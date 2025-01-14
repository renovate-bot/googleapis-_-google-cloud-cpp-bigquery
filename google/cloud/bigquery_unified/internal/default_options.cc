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

#include "google/cloud/bigquery_unified/internal/default_options.h"
#include "google/cloud/bigquery_unified/idempotency_policy.h"
#include "google/cloud/bigquery_unified/job_options.h"
#include "google/cloud/bigquery_unified/retry_policy.h"
#include "google/cloud/backoff_policy.h"
#include "google/cloud/polling_policy.h"

namespace google::cloud::bigquery_unified_internal {
GOOGLE_CLOUD_CPP_BIGQUERY_INLINE_NAMESPACE_BEGIN

namespace {
auto constexpr kBackoffScaling = 2.0;
}  // namespace

google::cloud::Options DefaultOptions(google::cloud::Options options) {
  if (!options.has<bigquery_unified::RetryPolicyOption>()) {
    options.set<bigquery_unified::RetryPolicyOption>(
        bigquery_unified::LimitedTimeRetryPolicy(std::chrono::minutes(30))
            .clone());
  }
  if (!options.has<bigquery_unified::BackoffPolicyOption>()) {
    options.set<bigquery_unified::BackoffPolicyOption>(
        ExponentialBackoffPolicy(
            std::chrono::seconds(0), std::chrono::seconds(1),
            std::chrono::minutes(5), kBackoffScaling, kBackoffScaling)
            .clone());
  }
  if (!options.has<bigquery_unified::PollingPolicyOption>()) {
    options.set<bigquery_unified::PollingPolicyOption>(
        GenericPollingPolicy<bigquery_unified::RetryPolicyOption::Type,
                             bigquery_unified::BackoffPolicyOption::Type>(
            options.get<bigquery_unified::RetryPolicyOption>()->clone(),
            ExponentialBackoffPolicy(std::chrono::seconds(1),
                                     std::chrono::minutes(5), kBackoffScaling)
                .clone())
            .clone());
  }
  if (!options.has<bigquery_unified::IdempotencyPolicyOption>()) {
    options.set<bigquery_unified::IdempotencyPolicyOption>(
        bigquery_unified::MakeDefaultIdempotencyPolicy());
  }

  return options;
}

GOOGLE_CLOUD_CPP_BIGQUERY_INLINE_NAMESPACE_END
}  // namespace google::cloud::bigquery_unified_internal
