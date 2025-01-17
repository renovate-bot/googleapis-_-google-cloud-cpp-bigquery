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
#include "google/cloud/bigquery_unified/job_options.h"
#include "google/cloud/bigquerycontrol/v2/job_connection.h"
#include "google/cloud/bigquerycontrol/v2/job_options.h"
#include <gmock/gmock.h>

namespace google::cloud::bigquery_unified_internal {
GOOGLE_CLOUD_CPP_BIGQUERY_INLINE_NAMESPACE_BEGIN
namespace {

using ::testing::Eq;

class MockBackoffPolicy : public google::cloud::BackoffPolicy {
 public:
  MOCK_METHOD(std::unique_ptr<google::cloud::BackoffPolicy>, clone, (),
              (const, override));
  MOCK_METHOD(std::chrono::milliseconds, OnCompletion, (), (override));
};

TEST(ApplyUnifiedPolicyOptionsToJobServicePolicyOptions,
     JobServiceBackoffPolicyNotSpecified) {
  std::chrono::milliseconds expected_on_completion =
      std::chrono::milliseconds(42);
  auto backoff_policy = std::make_shared<MockBackoffPolicy>();
  EXPECT_CALL(*backoff_policy, OnCompletion)
      .WillRepeatedly(::testing::Return(expected_on_completion));
  EXPECT_CALL(*backoff_policy, clone).WillOnce([=]() {
    auto backoff_policy = std::make_unique<MockBackoffPolicy>();
    EXPECT_CALL(*backoff_policy, OnCompletion)
        .WillRepeatedly(::testing::Return(expected_on_completion));
    return backoff_policy;
  });

  auto const initial_options =
      google::cloud::Options{}.set<bigquery_unified::BackoffPolicyOption>(
          backoff_policy);
  auto result_options =
      ApplyUnifiedPolicyOptionsToJobServicePolicyOptions(initial_options);
  EXPECT_THAT(result_options.get<bigquery_unified::BackoffPolicyOption>()
                  ->OnCompletion(),
              Eq(result_options
                     .get<bigquerycontrol_v2::JobServiceBackoffPolicyOption>()
                     ->OnCompletion()));
}

TEST(ApplyUnifiedPolicyOptionsToJobServicePolicyOptions,
     JobServiceBackoffPolicySpecified) {
  std::chrono::milliseconds unified_expected_on_completion =
      std::chrono::milliseconds(42);
  auto unified_backoff_policy = std::make_shared<MockBackoffPolicy>();
  EXPECT_CALL(*unified_backoff_policy, OnCompletion)
      .WillRepeatedly(::testing::Return(unified_expected_on_completion));

  std::chrono::milliseconds job_expected_on_completion =
      std::chrono::milliseconds(8675309);
  auto job_backoff_policy = std::make_shared<MockBackoffPolicy>();
  EXPECT_CALL(*job_backoff_policy, OnCompletion)
      .WillRepeatedly(::testing::Return(job_expected_on_completion));

  auto const initial_options =
      google::cloud::Options{}
          .set<bigquery_unified::BackoffPolicyOption>(unified_backoff_policy)
          .set<bigquerycontrol_v2::JobServiceBackoffPolicyOption>(
              job_backoff_policy);
  auto result_options =
      ApplyUnifiedPolicyOptionsToJobServicePolicyOptions(initial_options);
  EXPECT_THAT(result_options.get<bigquery_unified::BackoffPolicyOption>()
                  ->OnCompletion(),
              Eq(unified_expected_on_completion));
  EXPECT_THAT(
      result_options.get<bigquerycontrol_v2::JobServiceBackoffPolicyOption>()
          ->OnCompletion(),
      Eq(job_expected_on_completion));
}

TEST(ApplyUnifiedPolicyOptionsToJobServicePolicyOptions,
     JobServiceRetryPolicyNotSpecifiedLimitedError) {
  auto unified_retry_policy =
      std::make_shared<bigquery_unified::LimitedErrorCountRetryPolicy>(42);
  auto const initial_options =
      google::cloud::Options{}
          .set<bigquery_unified::BackoffPolicyOption>(
              ExponentialBackoffPolicy(std::chrono::seconds(0),
                                       std::chrono::seconds(1),
                                       std::chrono::minutes(5), 2.0, 2.0)
                  .clone())
          .set<bigquery_unified::RetryPolicyOption>(unified_retry_policy);

  auto result_options =
      ApplyUnifiedPolicyOptionsToJobServicePolicyOptions(initial_options);
  auto const& job_retry_policy =
      result_options.get<bigquerycontrol_v2::JobServiceRetryPolicyOption>();
  auto* job_limited_error_count_retry_policy =
      dynamic_cast<bigquerycontrol_v2::JobServiceLimitedErrorCountRetryPolicy*>(
          job_retry_policy.get());
  ASSERT_TRUE(job_limited_error_count_retry_policy != nullptr);
  EXPECT_THAT(unified_retry_policy->maximum_failures(),
              Eq(job_limited_error_count_retry_policy->maximum_failures()));
}

TEST(ApplyUnifiedPolicyOptionsToJobServicePolicyOptions,
     JobServiceRetryPolicyNotSpecifiedLimitedTime) {
  auto unified_retry_policy =
      std::make_shared<bigquery_unified::LimitedTimeRetryPolicy>(
          std::chrono::milliseconds(42));
  auto const initial_options =
      google::cloud::Options{}
          .set<bigquery_unified::BackoffPolicyOption>(
              ExponentialBackoffPolicy(std::chrono::seconds(0),
                                       std::chrono::seconds(1),
                                       std::chrono::minutes(5), 2.0, 2.0)
                  .clone())
          .set<bigquery_unified::RetryPolicyOption>(unified_retry_policy);

  auto result_options =
      ApplyUnifiedPolicyOptionsToJobServicePolicyOptions(initial_options);
  auto const& job_retry_policy =
      result_options.get<bigquerycontrol_v2::JobServiceRetryPolicyOption>();
  auto* job_limited_error_count_retry_policy =
      dynamic_cast<bigquerycontrol_v2::JobServiceLimitedTimeRetryPolicy*>(
          job_retry_policy.get());
  ASSERT_TRUE(job_limited_error_count_retry_policy != nullptr);
  EXPECT_THAT(unified_retry_policy->maximum_duration(),
              Eq(job_limited_error_count_retry_policy->maximum_duration()));
}

TEST(ApplyUnifiedPolicyOptionsToJobServicePolicyOptions,
     JobServiceRetryPolicySpecified) {
  auto unified_retry_policy =
      std::make_shared<bigquery_unified::LimitedErrorCountRetryPolicy>(42);
  auto job_retry_policy = std::make_shared<
      bigquerycontrol_v2::JobServiceLimitedErrorCountRetryPolicy>(8675309);
  auto const initial_options =
      google::cloud::Options{}
          .set<bigquery_unified::BackoffPolicyOption>(
              ExponentialBackoffPolicy(std::chrono::seconds(0),
                                       std::chrono::seconds(1),
                                       std::chrono::minutes(5), 2.0, 2.0)
                  .clone())
          .set<bigquery_unified::RetryPolicyOption>(unified_retry_policy)
          .set<bigquerycontrol_v2::JobServiceRetryPolicyOption>(
              job_retry_policy);

  auto result_options =
      ApplyUnifiedPolicyOptionsToJobServicePolicyOptions(initial_options);

  auto const& result_job_retry_policy =
      result_options.get<bigquerycontrol_v2::JobServiceRetryPolicyOption>();
  auto* job_limited_error_count_retry_policy =
      dynamic_cast<bigquerycontrol_v2::JobServiceLimitedErrorCountRetryPolicy*>(
          result_job_retry_policy.get());
  ASSERT_TRUE(job_limited_error_count_retry_policy != nullptr);
  EXPECT_THAT(job_limited_error_count_retry_policy->maximum_failures(),
              Eq(8675309));
}

}  // namespace
GOOGLE_CLOUD_CPP_BIGQUERY_INLINE_NAMESPACE_END
}  // namespace google::cloud::bigquery_unified_internal
