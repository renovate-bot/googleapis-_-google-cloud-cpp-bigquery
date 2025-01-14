// Copyright 2025 Google LLC
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     https://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#ifndef GOOGLE_CLOUD_CPP_BIGQUERY_GOOGLE_CLOUD_BIGQUERY_UNIFIED_INTERNAL_ASYNC_REST_LONG_RUNNING_OPERATION_CUSTOM_H
#define GOOGLE_CLOUD_CPP_BIGQUERY_GOOGLE_CLOUD_BIGQUERY_UNIFIED_INTERNAL_ASYNC_REST_LONG_RUNNING_OPERATION_CUSTOM_H

#include "google/cloud/bigquery_unified/version.h"
#include "google/cloud/future.h"
#include "google/cloud/internal/async_rest_long_running_operation.h"
#include "google/cloud/polling_policy.h"
#include "google/cloud/status_or.h"
#include <functional>
#include <memory>
#include <string>

namespace google {
namespace cloud {
namespace bigquery_unified_internal {
GOOGLE_CLOUD_CPP_BIGQUERY_INLINE_NAMESPACE_BEGIN

// TODO: These types are mostly copy/paste from
// google/cloud/internal/async_rest_long_running_operation_custom.h,
// google/cloud/internal/async_rest_polling_loop.h, and
// google/cloud/internal/async_rest_polling_loop_impl.h.
// However, we need to provide a mechanism to get the rpc name from an
// OperationType that does not have a `name` method. We do so here by providing
// a std::function that takes the OperationTypes as a parameter and returns a
// std::string.
// Once we update the code in google-cloud-cpp to accept a operation_name
// function and default it to OperationType::name, all this code can go away.

template <typename OperationType, typename GetOperationRequestType>
using AsyncRestPollLongRunningOperation =
    std::function<future<StatusOr<OperationType>>(
        google::cloud::CompletionQueue&,
        std::unique_ptr<rest_internal::RestContext>, internal::ImmutableOptions,
        GetOperationRequestType const&)>;

template <typename CancelOperationRequestType>
using AsyncRestCancelLongRunningOperation = std::function<future<Status>(
    google::cloud::CompletionQueue&,
    std::unique_ptr<rest_internal::RestContext>, internal::ImmutableOptions,
    CancelOperationRequestType const&)>;

template <typename OperationType, typename GetOperationRequestType,
          typename CancelOperationRequestType>
class AsyncRestPollingLoopImpl
    : public std::enable_shared_from_this<AsyncRestPollingLoopImpl<
          OperationType, GetOperationRequestType, CancelOperationRequestType>> {
 public:
  AsyncRestPollingLoopImpl(
      google::cloud::CompletionQueue cq, internal::ImmutableOptions options,
      AsyncRestPollLongRunningOperation<OperationType, GetOperationRequestType>
          poll,
      AsyncRestCancelLongRunningOperation<CancelOperationRequestType> cancel,
      std::unique_ptr<PollingPolicy> polling_policy, std::string location,
      std::function<bool(OperationType const&)> is_operation_done,
      std::function<void(std::string const&, GetOperationRequestType&)>
          get_request_set_operation_name,
      std::function<void(std::string const&, CancelOperationRequestType&)>
          cancel_request_set_operation_name,
      std::function<std::string(StatusOr<OperationType> const&)> operation_name)
      : cq_(std::move(cq)),
        options_(std::move(options)),
        poll_(std::move(poll)),
        cancel_(std::move(cancel)),
        polling_policy_(std::move(polling_policy)),
        location_(std::move(location)),
        promise_(null_promise_t{}),
        is_operation_done_(std::move(is_operation_done)),
        get_request_set_operation_name_(
            std::move(get_request_set_operation_name)),
        cancel_request_set_operation_name_(
            std::move(cancel_request_set_operation_name)),
        operation_name_(std::move(operation_name)) {}

  future<StatusOr<OperationType>> Start(future<StatusOr<OperationType>> op) {
    auto self = this->shared_from_this();
    auto w = WeakFromThis();
    promise_ = promise<StatusOr<OperationType>>(
        [w, c = internal::CallContext{options_}]() mutable {
          if (auto self = w.lock()) {
            internal::ScopedCallContext scope(std::move(c));
            self->DoCancel();
          }
        });
    op.then(
        [self](future<StatusOr<OperationType>> f) { self->OnStart(f.get()); });
    return promise_.get_future();
  }

 private:
  using TimerResult = future<StatusOr<std::chrono::system_clock::time_point>>;

  std::weak_ptr<AsyncRestPollingLoopImpl> WeakFromThis() {
    return this->shared_from_this();
  }

  void DoCancel() {
    CancelOperationRequestType request;
    {
      std::unique_lock<std::mutex> lk(mu_);
      if (op_name_.empty()) {
        delayed_cancel_ = true;  // Wait for OnStart() to set `op_name_`.
        return;
      }
      cancel_request_set_operation_name_(op_name_, request);
    }
    // Cancels are best effort, so we use weak pointers.
    auto w = WeakFromThis();
    cancel_(cq_, std::make_unique<rest_internal::RestContext>(), options_,
            request)
        .then([w](future<Status> f) {
          if (auto self = w.lock()) self->OnCancel(f.get());
        });
  }

  void OnCancel(Status const& status) {
    GCP_LOG(DEBUG) << location_ << "() cancelled: " << status;
  }

  void OnStart(StatusOr<OperationType> op) {
    if (!op) return promise_.set_value(std::move(op));
    auto operation_name = operation_name_(op);
    internal::AddSpanAttribute(*options_, "gl-cpp.LRO_name", operation_name);
    if (is_operation_done_(*op)) return promise_.set_value(std::move(op));
    GCP_LOG(DEBUG) << location_ << "() polling loop starting for "
                   << operation_name;
    bool do_cancel = false;
    {
      std::unique_lock<std::mutex> lk(mu_);
      std::swap(delayed_cancel_, do_cancel);
      op_name_ = operation_name;
    }
    if (do_cancel) DoCancel();
    return Wait();
  }

  void Wait() {
    std::chrono::milliseconds duration = polling_policy_->WaitPeriod();
    GCP_LOG(DEBUG) << location_ << "() polling loop waiting "
                   << duration.count() << "ms";
    auto self = this->shared_from_this();
    internal::TracedAsyncBackoff(cq_, *options_, duration, "Async Backoff")
        .then([self](TimerResult f) { self->OnTimer(std::move(f)); });
  }

  void OnTimer(TimerResult f) {
    GCP_LOG(DEBUG) << location_ << "() polling loop awakened";
    auto t = f.get();
    if (!t) return promise_.set_value(std::move(t).status());
    GetOperationRequestType request;
    {
      std::unique_lock<std::mutex> lk(mu_);
      get_request_set_operation_name_(op_name_, request);
    }
    auto self = this->shared_from_this();
    poll_(cq_, std::make_unique<rest_internal::RestContext>(), options_,
          request)
        .then([self](future<StatusOr<OperationType>> g) {
          self->OnPoll(std::move(g));
        });
  }

  void OnPoll(future<StatusOr<OperationType>> f) {
    GCP_LOG(DEBUG) << location_ << "() polling loop result";
    auto op = f.get();
    if (op && is_operation_done_(*op)) {
      return promise_.set_value(*std::move(op));
    }
    // Update the polling policy even on successful requests, so we can stop
    // after too many polling attempts.
    if (!polling_policy_->OnFailure(op.status())) {
      if (op) {
        // We should not be fabricating a `Status` value here. Rather, we
        // should cancel the operation and wait for the next poll to return
        // an accurate status to the user, otherwise they will have no idea
        // how to react. But for now, we leave the operation running. It
        // may eventually complete.
        return promise_.set_value(internal::DeadlineExceededError(
            location_ + "() - polling loop terminated by "
                        "polling policy",
            GCP_ERROR_INFO()));
      }
      // This could be a transient error if the policy is exhausted.
      return promise_.set_value(std::move(op).status());
    }
    return Wait();
  }

  // These member variables are initialized in the constructor or from
  // `Start()`, and then only used from the `On*()` callbacks, which are
  // serialized, so they need no external synchronization.
  google::cloud::CompletionQueue cq_;
  internal::ImmutableOptions options_;
  AsyncRestPollLongRunningOperation<OperationType, GetOperationRequestType>
      poll_;
  AsyncRestCancelLongRunningOperation<CancelOperationRequestType> cancel_;
  std::unique_ptr<PollingPolicy> polling_policy_;
  std::string location_;
  promise<StatusOr<OperationType>> promise_;
  std::function<bool(OperationType const&)> is_operation_done_;
  std::function<void(std::string const&, GetOperationRequestType&)>
      get_request_set_operation_name_;
  std::function<void(std::string const&, CancelOperationRequestType&)>
      cancel_request_set_operation_name_;
  std::function<std::string(StatusOr<OperationType> const&)> operation_name_;

  // `delayed_cancel_` and `op_name_`, in contrast, are also used from
  // `DoCancel()`, which is called asynchronously, so they need locking.
  std::mutex mu_;
  bool delayed_cancel_ = false;  // GUARDED_BY(mu_)
  std::string op_name_;          // GUARDED_BY(mu_)
};

/**
 * Customizable polling loop for services that do not conform to AIP-151.
 */
template <typename OperationType, typename GetOperationRequestType,
          typename CancelOperationRequestType>
future<StatusOr<OperationType>> AsyncRestPollingLoop(
    google::cloud::CompletionQueue cq, internal::ImmutableOptions options,
    future<StatusOr<OperationType>> op,
    AsyncRestPollLongRunningOperation<OperationType, GetOperationRequestType>
        poll,
    AsyncRestCancelLongRunningOperation<CancelOperationRequestType> cancel,
    std::unique_ptr<PollingPolicy> polling_policy, std::string location,
    std::function<bool(OperationType const&)> is_operation_done,
    std::function<void(std::string const&, GetOperationRequestType&)>
        get_request_set_operation_name,
    std::function<void(std::string const&, CancelOperationRequestType&)>
        cancel_request_set_operation_name,
    std::function<std::string(StatusOr<OperationType> const&)> operation_name) {
  auto loop = std::make_shared<AsyncRestPollingLoopImpl<
      OperationType, GetOperationRequestType, CancelOperationRequestType>>(
      std::move(cq), options, std::move(poll), std::move(cancel),
      std::move(polling_policy), std::move(location), is_operation_done,
      get_request_set_operation_name, cancel_request_set_operation_name,
      operation_name);
  return loop->Start(std::move(op));
}

/*
 * AsyncAwaitRestLongRunningOperation for services that do not conform to
 * AIP-151.
 */
template <typename ReturnType, typename OperationType,
          typename GetOperationRequestType, typename CancelOperationRequestType,
          typename CompletionQueue>
future<StatusOr<ReturnType>> AsyncRestAwaitLongRunningOperation(
    CompletionQueue cq, internal::ImmutableOptions options,
    OperationType operation,
    AsyncRestPollLongRunningOperation<OperationType, GetOperationRequestType>
        poll,
    AsyncRestCancelLongRunningOperation<CancelOperationRequestType> cancel,
    rest_internal::LongRunningOperationValueExtractor<ReturnType, OperationType>
        value_extractor,
    std::unique_ptr<PollingPolicy> polling_policy, char const* location,
    std::function<bool(OperationType const&)> is_operation_done,
    std::function<void(std::string const&, GetOperationRequestType&)>
        get_request_set_operation_name,
    std::function<void(std::string const&, CancelOperationRequestType&)>
        cancel_request_set_operation_name,
    std::function<std::string(StatusOr<OperationType> const&)> operation_name) {
  auto loc = std::string{location};
  return AsyncRestPollingLoop<OperationType, GetOperationRequestType,
                              CancelOperationRequestType>(
             std::move(cq), std::move(options),
             make_ready_future(StatusOr<OperationType>(operation)),
             std::move(poll), std::move(cancel), std::move(polling_policy),
             std::move(location), is_operation_done,
             get_request_set_operation_name, cancel_request_set_operation_name,
             operation_name)
      .then([value_extractor, loc](future<StatusOr<OperationType>> g) {
        return value_extractor(g.get(), loc);
      });
}

GOOGLE_CLOUD_CPP_BIGQUERY_INLINE_NAMESPACE_END
}  // namespace bigquery_unified_internal
}  // namespace cloud
}  // namespace google

#endif  // GOOGLE_CLOUD_CPP_BIGQUERY_GOOGLE_CLOUD_BIGQUERY_UNIFIED_INTERNAL_ASYNC_REST_LONG_RUNNING_OPERATION_CUSTOM_H
