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

#include "google/cloud/bigquery_unified/internal/connection_impl.h"
#include "google/cloud/bigquery_unified/idempotency_policy.h"
#include "google/cloud/bigquery_unified/internal/async_rest_long_running_operation_custom.h"
#include "google/cloud/bigquery_unified/internal/default_options.h"
#include "google/cloud/bigquery_unified/job_options.h"
#include "google/cloud/bigquery_unified/retry_policy.h"
#include "google/cloud/bigquerycontrol/v2/internal/job_option_defaults.h"
#include "google/cloud/bigquerycontrol/v2/internal/job_rest_connection_impl.h"
#include "google/cloud/bigquerycontrol/v2/internal/job_rest_stub_factory.h"
#include "google/cloud/bigquerycontrol/v2/internal/job_tracing_connection.h"
#include "google/cloud/background_threads.h"
#include "google/cloud/internal/rest_background_threads_impl.h"
#include "google/cloud/internal/rest_retry_loop.h"

namespace google::cloud::bigquery_unified_internal {
GOOGLE_CLOUD_CPP_BIGQUERY_INLINE_NAMESPACE_BEGIN
namespace {

std::unique_ptr<bigquery_unified::RetryPolicy> retry_policy(
    Options const& options) {
  return options.get<bigquery_unified::RetryPolicyOption>()->clone();
}

std::unique_ptr<BackoffPolicy> backoff_policy(Options const& options) {
  return options.get<bigquery_unified::BackoffPolicyOption>()->clone();
}

std::unique_ptr<bigquery_unified::IdempotencyPolicy> idempotency_policy(
    Options const& options) {
  return options.get<bigquery_unified::IdempotencyPolicyOption>()->clone();
}

std::unique_ptr<PollingPolicy> polling_policy(Options const& options) {
  return options.get<bigquery_unified::PollingPolicyOption>()->clone();
}

}  // namespace

ConnectionImpl::ConnectionImpl(
    std::shared_ptr<google::cloud::bigquerycontrol_v2::JobServiceConnection>
        job_connection,
    google::cloud::Options job_options,
    std::shared_ptr<bigquerycontrol_v2_internal::JobServiceRestStub> job_stub,
    std::unique_ptr<google::cloud::BackgroundThreads> background,
    google::cloud::Options options)
    : job_connection_(std::move(job_connection)),
      job_stub_(std::move(job_stub)),
      job_options_(std::move(job_options)),
      background_(std::move(background)),
      options_(std::move(options)) {}

Status ConnectionImpl::DeleteJob(
    google::cloud::bigquery::v2::DeleteJobRequest const& request,
    Options opts) {
  // TODO: Instead of creating an OptionsSpan, pass opts when job_connection_
  // supports it.
  internal::OptionsSpan span(internal::MergeOptions(
      std::move(opts), internal::MergeOptions(options_, job_options_)));
  return job_connection_->DeleteJob(request);
}

StatusOr<google::cloud::bigquery::v2::Job> ConnectionImpl::GetJob(
    google::cloud::bigquery::v2::GetJobRequest const& request, Options opts) {
  // TODO: Instead of creating an OptionsSpan, pass opts when job_connection_
  // supports it.
  internal::OptionsSpan span(internal::MergeOptions(
      std::move(opts), internal::MergeOptions(options_, job_options_)));
  return job_connection_->GetJob(request);
}

future<StatusOr<google::cloud::bigquery::v2::Job>> ConnectionImpl::InsertJob(
    google::cloud::bigquery::v2::Job const& job, Options opts) {
  // TODO: Instead of creating an OptionsSpan, pass opts when job_connection_
  // supports it.
  internal::OptionsSpan span(internal::MergeOptions(
      std::move(opts), internal::MergeOptions(options_, job_options_)));
  auto current_options = google::cloud::internal::SaveCurrentOptions();
  google::cloud::bigquery::v2::InsertJobRequest insert_request;
  auto const billing_project =
      current_options->has<bigquery_unified::BillingProjectOption>()
          ? current_options->get<bigquery_unified::BillingProjectOption>()
          : "";

  insert_request.set_project_id(billing_project);
  *insert_request.mutable_job() = job;
  auto idempotency = idempotency_policy(*current_options)
                         ->InsertJob(insert_request, *current_options);
  auto insert_response = rest_internal::RestRetryLoop(
      retry_policy(*current_options), backoff_policy(*current_options),
      std::move(idempotency),
      [stub = job_stub_](
          rest_internal::RestContext& context, google::cloud::Options options,
          google::cloud::bigquery::v2::InsertJobRequest const& request)
          -> StatusOr<google::cloud::bigquery::v2::Job> {
        auto x=  stub->InsertJob(context, options, request);
        std::cout << "@@@@@@@@@@@@@@@@@@@@@@@@@@" << std::endl;
        if (!x) {
          std::cout << x.status() << std::endl;
        }
        return x;
      },
      *current_options, insert_request, __func__);

  return bigquery_unified_internal::AsyncRestAwaitLongRunningOperation<
      google::cloud::bigquery::v2::Job, google::cloud::bigquery::v2::Job,
      google::cloud::bigquery::v2::GetJobRequest,
      google::cloud::bigquery::v2::CancelJobRequest>(
      background_->cq(), current_options, *insert_response,
      [stub = job_stub_](
          CompletionQueue& cq,
          std::unique_ptr<rest_internal::RestContext> context,
          google::cloud::internal::ImmutableOptions options,
          google::cloud::bigquery::v2::GetJobRequest const& request)
          -> future<StatusOr<google::cloud::bigquery::v2::Job>> {
        return make_ready_future(
            stub->GetJob(*std::move(context), *std::move(options), request));
      },
      [stub = job_stub_](
          CompletionQueue& cq,
          std::unique_ptr<rest_internal::RestContext> context,
          google::cloud::internal::ImmutableOptions options,
          google::cloud::bigquery::v2::CancelJobRequest const& request)
          -> future<Status> {
        auto cancel_response =
            stub->CancelJob(*std::move(context), *std::move(options), request);
        if (!cancel_response) {
          return make_ready_future(std::move(cancel_response).status());
        }
        return make_ready_future(Status{});
      },
      [](StatusOr<google::cloud::bigquery::v2::Job> op, std::string const&) {
        return op;
      },
      polling_policy(*current_options), __func__,
      [](google::cloud::bigquery::v2::Job const& op) {
        return op.status().state() == "DONE";
      },
      [ref = insert_response->job_reference()](
          std::string const&, google::cloud::bigquery::v2::GetJobRequest& r) {
        r.set_project_id(ref.project_id());
        r.set_job_id(ref.job_id());
        r.set_location(ref.location().value());
      },
      [ref = insert_response->job_reference()](
          std::string const&,
          google::cloud::bigquery::v2::CancelJobRequest& r) {
        r.set_project_id(ref.project_id());
        r.set_job_id(ref.job_id());
        r.set_location(ref.location().value());
      },
      [](StatusOr<google::cloud::bigquery::v2::Job> const&) {
        return std::string{"InsertJob"};
      });
}

StatusOr<google::cloud::bigquery::v2::JobReference> ConnectionImpl::InsertJob(
    google::cloud::NoAwaitTag, google::cloud::bigquery::v2::Job const& job,
    Options opts) {
  // TODO: Instead of creating an OptionsSpan, pass opts when job_connection_
  // supports it.
  internal::OptionsSpan span(internal::MergeOptions(
      std::move(opts), internal::MergeOptions(options_, job_options_)));
  auto current_options = google::cloud::internal::SaveCurrentOptions();
  google::cloud::bigquery::v2::InsertJobRequest insert_request;
  auto const billing_project =
      current_options->has<bigquery_unified::BillingProjectOption>()
          ? current_options->get<bigquery_unified::BillingProjectOption>()
          : "";

  insert_request.set_project_id(billing_project);
  *insert_request.mutable_job() = job;
  auto insert_response = job_connection_->InsertJob(insert_request);
  if (!insert_response) {
    return insert_response.status();
  }
  return insert_response->job_reference();
}

future<StatusOr<google::cloud::bigquery::v2::Job>> ConnectionImpl::InsertJob(
    google::cloud::bigquery::v2::JobReference const& job_reference,
    Options opts) {
  bigquery::v2::GetJobRequest get_job_request;
  get_job_request.set_project_id(job_reference.project_id());
  get_job_request.set_job_id(job_reference.job_id());
  get_job_request.set_location(job_reference.location().value());
  auto job = GetJob(get_job_request, opts);
  std::cout << "------------------!!!!!!!!---------------" << std::endl;
  std::cout << job->job_reference().job_id() << std::endl;
  std::cout << job->job_reference().project_id() << std::endl;
  std::cout << job->job_reference().location().value() << std::endl;
  std::cout << "------------------!!!!!!!!---------------" << std::endl;
  if (!job) {
    return make_ready_future(StatusOr<google::cloud::bigquery::v2::Job>(job.status()));
  }
  return InsertJob(std::move(*job), opts);
}

StreamRange<google::cloud::bigquery::v2::ListFormatJob>
ConnectionImpl::ListJobs(google::cloud::bigquery::v2::ListJobsRequest request,
                         Options opts) {
  // TODO: Instead of creating an OptionsSpan, pass opts when job_connection_
  // supports it.
  internal::OptionsSpan span(internal::MergeOptions(
      std::move(opts), internal::MergeOptions(options_, job_options_)));
  return job_connection_->ListJobs(request);
}

Options ApplyUnifiedPolicyOptionsToJobServicePolicyOptions(Options options) {
  if (!options.has<bigquerycontrol_v2::JobServiceBackoffPolicyOption>()) {
    options.set<bigquerycontrol_v2::JobServiceBackoffPolicyOption>(
        options.get<bigquery_unified::BackoffPolicyOption>()->clone());
  }

  if (!options.has<bigquerycontrol_v2::JobServiceRetryPolicyOption>()) {
    // Dynamic casting is required here because of the current class hierarchy
    // present in RetryPolicy types found in google-cloud-cpp which prevent
    // handling them generically like BackoffPolicy types can be. If someday we
    // compress the RetryPolicy hierarchy, this code can be simplified similar
    // to how BackOffPolicy types are handled.
    auto const& unified_retry_policy =
        options.get<bigquery_unified::RetryPolicyOption>();
    if (auto* p = dynamic_cast<bigquery_unified::LimitedErrorCountRetryPolicy*>(
            unified_retry_policy.get());
        p != nullptr) {
      options.set<bigquerycontrol_v2::JobServiceRetryPolicyOption>(
          std::make_shared<
              bigquerycontrol_v2::JobServiceLimitedErrorCountRetryPolicy>(
              p->maximum_failures()));
    }
    if (auto* p = dynamic_cast<bigquery_unified::LimitedTimeRetryPolicy*>(
            unified_retry_policy.get());
        p != nullptr) {
      options.set<bigquerycontrol_v2::JobServiceRetryPolicyOption>(
          std::make_shared<
              bigquerycontrol_v2::JobServiceLimitedTimeRetryPolicy>(
              p->maximum_duration()));
    }
  }

  return options;
}

std::shared_ptr<bigquery_unified::Connection> MakeDefaultConnectionImpl(
    Options options) {
  auto background = std::make_unique<
      rest_internal::AutomaticallyCreatedRestBackgroundThreads>();

  options =
      ApplyUnifiedPolicyOptionsToJobServicePolicyOptions(std::move(options));

  auto job_options =
      bigquerycontrol_v2_internal::JobServiceDefaultOptions(options);
  // TODO: JobServiceRestConnectionImpl requires background threads, but does it
  // actually use them? Needs further investigation.
  auto job_background = std::make_unique<
      rest_internal::AutomaticallyCreatedRestBackgroundThreads>();
  auto job_stub =
      bigquerycontrol_v2_internal::CreateDefaultJobServiceRestStub(job_options);
  std::shared_ptr<bigquerycontrol_v2::JobServiceConnection> job_connection =
      bigquerycontrol_v2_internal::MakeJobServiceTracingConnection(
          std::make_shared<
              bigquerycontrol_v2_internal::JobServiceRestConnectionImpl>(
              std::move(job_background), job_stub, job_options));

  // TODO: We should probably add a TracingConnection decorator in order to
  //  associate all the various rpcs that are called as part of these
  //  operations.
  return std::make_shared<bigquery_unified_internal::ConnectionImpl>(
      std::move(job_connection), std::move(job_options), std::move(job_stub),
      std::move(background), std::move(options));
}

GOOGLE_CLOUD_CPP_BIGQUERY_INLINE_NAMESPACE_END
}  // namespace google::cloud::bigquery_unified_internal
