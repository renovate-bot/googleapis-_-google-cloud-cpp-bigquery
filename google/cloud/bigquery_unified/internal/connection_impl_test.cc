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
#include "google/cloud/bigquery_unified/internal/default_options.h"
#include "google/cloud/bigquery_unified/job_options.h"
#include "google/cloud/bigquery_unified/testing_util/status_matchers.h"
#include "google/cloud/bigquerycontrol/v2/job_connection.h"
#include "google/cloud/bigquerycontrol/v2/job_options.h"
#include "google/cloud/internal/rest_background_threads_impl.h"
#include <gmock/gmock.h>

namespace google::cloud::bigquery_unified_internal {
GOOGLE_CLOUD_CPP_BIGQUERY_INLINE_NAMESPACE_BEGIN
namespace {

using ::google::cloud::bigquery_unified::testing_util::IsOk;
using ::google::cloud::bigquery_unified::testing_util::StatusIs;
using ::testing::Eq;
using ::testing::Return;

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

TEST(DetermineBillingProject, UseCorrectFieldDependingOnJobType) {
  google::cloud::bigquery::v2::Job job;

  google::cloud::bigquery::v2::JobConfigurationTableCopy copy_job;
  copy_job.mutable_destination_table()->set_project_id("my-copy-project");
  *(job.mutable_configuration()->mutable_copy()) = copy_job;
  job.mutable_configuration()->set_job_type("COPY");
  EXPECT_THAT(DetermineBillingProject(job), Eq("my-copy-project"));

  google::cloud::bigquery::v2::JobConfigurationExtract model_extract_job;
  model_extract_job.mutable_source_model()->set_project_id(
      "my-model-extract-project");
  *(job.mutable_configuration()->mutable_extract()) = model_extract_job;
  job.mutable_configuration()->set_job_type("EXTRACT");
  EXPECT_THAT(DetermineBillingProject(job), Eq("my-model-extract-project"));

  google::cloud::bigquery::v2::JobConfigurationExtract table_extract_job;
  table_extract_job.mutable_source_table()->set_project_id(
      "my-table-extract-project");
  *(job.mutable_configuration()->mutable_extract()) = table_extract_job;
  job.mutable_configuration()->set_job_type("EXTRACT");
  EXPECT_THAT(DetermineBillingProject(job), Eq("my-table-extract-project"));

  google::cloud::bigquery::v2::JobConfigurationLoad load_job;
  load_job.mutable_destination_table()->set_project_id("my-load-project");
  *(job.mutable_configuration()->mutable_load()) = load_job;
  job.mutable_configuration()->set_job_type("LOAD");
  EXPECT_THAT(DetermineBillingProject(job), Eq("my-load-project"));

  google::cloud::bigquery::v2::JobConfigurationQuery query_job;
  query_job.mutable_destination_table()->set_project_id("my-query-project");
  *(job.mutable_configuration()->mutable_query()) = query_job;
  job.mutable_configuration()->set_job_type("QUERY");
  EXPECT_THAT(DetermineBillingProject(job), Eq("my-query-project"));
}

class MockJobServiceRestStub
    : public google::cloud::bigquerycontrol_v2_internal::JobServiceRestStub {
 public:
  MOCK_METHOD(StatusOr<google::cloud::bigquery::v2::JobCancelResponse>,
              CancelJob,
              (google::cloud::rest_internal::RestContext & rest_context,
               Options const& options,
               google::cloud::bigquery::v2::CancelJobRequest const& request),
              (override));
  MOCK_METHOD(StatusOr<google::cloud::bigquery::v2::Job>, GetJob,
              (google::cloud::rest_internal::RestContext & rest_context,
               Options const& options,
               google::cloud::bigquery::v2::GetJobRequest const& request),
              (override));
  MOCK_METHOD(StatusOr<google::cloud::bigquery::v2::Job>, InsertJob,
              (google::cloud::rest_internal::RestContext & rest_context,
               Options const& options,
               google::cloud::bigquery::v2::InsertJobRequest const& request),
              (override));
  MOCK_METHOD(Status, DeleteJob,
              (google::cloud::rest_internal::RestContext & rest_context,
               Options const& options,
               google::cloud::bigquery::v2::DeleteJobRequest const& request),
              (override));
  MOCK_METHOD(StatusOr<google::cloud::bigquery::v2::JobList>, ListJobs,
              (google::cloud::rest_internal::RestContext & rest_context,
               Options const& options,
               google::cloud::bigquery::v2::ListJobsRequest const& request),
              (override));
  MOCK_METHOD(
      StatusOr<google::cloud::bigquery::v2::GetQueryResultsResponse>,
      GetQueryResults,
      (google::cloud::rest_internal::RestContext & rest_context,
       Options const& options,
       google::cloud::bigquery::v2::GetQueryResultsRequest const& request),
      (override));
  MOCK_METHOD(StatusOr<google::cloud::bigquery::v2::QueryResponse>, Query,
              (google::cloud::rest_internal::RestContext & rest_context,
               Options const& options,
               google::cloud::bigquery::v2::PostQueryRequest const& request),
              (override));
};

class MockJobServiceConnection
    : public bigquerycontrol_v2::JobServiceConnection {
 public:
  MOCK_METHOD(Options, options, (), (override));
  MOCK_METHOD(StatusOr<google::cloud::bigquery::v2::JobCancelResponse>,
              CancelJob,
              (google::cloud::bigquery::v2::CancelJobRequest const& request),
              (override));
  MOCK_METHOD(StatusOr<google::cloud::bigquery::v2::Job>, GetJob,
              (google::cloud::bigquery::v2::GetJobRequest const& request),
              (override));
  MOCK_METHOD(StatusOr<google::cloud::bigquery::v2::Job>, InsertJob,
              (google::cloud::bigquery::v2::InsertJobRequest const& request),
              (override));
  MOCK_METHOD(Status, DeleteJob,
              (google::cloud::bigquery::v2::DeleteJobRequest const& request),
              (override));
  MOCK_METHOD((StreamRange<google::cloud::bigquery::v2::ListFormatJob>),
              ListJobs, (google::cloud::bigquery::v2::ListJobsRequest request),
              (override));
  MOCK_METHOD(
      StatusOr<google::cloud::bigquery::v2::GetQueryResultsResponse>,
      GetQueryResults,
      (google::cloud::bigquery::v2::GetQueryResultsRequest const& request),
      (override));
  MOCK_METHOD(StatusOr<google::cloud::bigquery::v2::QueryResponse>, Query,
              (google::cloud::bigquery::v2::PostQueryRequest const& request),
              (override));
};

class MockBigQueryReadConnection
    : public bigquery_storage_v1::BigQueryReadConnection {
 public:
  MOCK_METHOD(Options, options, (), (override));

  MOCK_METHOD(
      StatusOr<google::cloud::bigquery::storage::v1::ReadSession>,
      CreateReadSession,
      (google::cloud::bigquery::storage::v1::CreateReadSessionRequest const&
           request),
      (override));

  MOCK_METHOD(
      StreamRange<google::cloud::bigquery::storage::v1::ReadRowsResponse>,
      ReadRows,
      (google::cloud::bigquery::storage::v1::ReadRowsRequest const& request),
      (override));

  MOCK_METHOD(
      StatusOr<google::cloud::bigquery::storage::v1::SplitReadStreamResponse>,
      SplitReadStream,
      (google::cloud::bigquery::storage::v1::SplitReadStreamRequest const&
           request),
      (override));
};

class MockBackgroundThreads : public BackgroundThreads {
 public:
  MOCK_METHOD(CompletionQueue, cq, (), (const, override));
};

class ConnectionImplTest : public ::testing::Test {
 protected:
  void SetUp() override {
    mock_read_connection_ = std::make_shared<MockBigQueryReadConnection>();
    mock_job_connection_ = std::make_shared<MockJobServiceConnection>();
    mock_job_stub_ = std::make_shared<MockJobServiceRestStub>();
    mock_background_ = std::make_unique<MockBackgroundThreads>();
  }

  static Options SetQuickPollingOptions(Options options) {
    options
        .set<bigquery_unified::RetryPolicyOption>(
            std::make_shared<bigquery_unified::LimitedErrorCountRetryPolicy>(3))
        .set<bigquery_unified::BackoffPolicyOption>(
            std::make_shared<ExponentialBackoffPolicy>(
                std::chrono::milliseconds(0), std::chrono::milliseconds(0),
                2.0));
    options.set<bigquery_unified::PollingPolicyOption>(
        std::make_shared<
            GenericPollingPolicy<bigquery_unified::RetryPolicyOption::Type,
                                 bigquery_unified::BackoffPolicyOption::Type>>(
            options.get<bigquery_unified::RetryPolicyOption>()->clone(),
            options.get<bigquery_unified::BackoffPolicyOption>()->clone()));
    return options;
  }

  std::shared_ptr<MockBigQueryReadConnection> mock_read_connection_;
  std::shared_ptr<MockJobServiceConnection> mock_job_connection_;
  std::shared_ptr<MockJobServiceRestStub> mock_job_stub_;
  std::unique_ptr<MockBackgroundThreads> mock_background_;
};

TEST_F(ConnectionImplTest, InsertJobNoAwait) {
  std::string const project_id = "my-project";
  std::string const job_id = "my_job";

  EXPECT_CALL(*mock_job_connection_, InsertJob)
      .WillOnce(
          [&](google::cloud::bigquery::v2::InsertJobRequest const& request) {
            EXPECT_THAT(request.job().job_reference().project_id(),
                        Eq(project_id));
            EXPECT_THAT(request.job().job_reference().job_id(), Eq(job_id));
            google::cloud::bigquery::v2::Job job;
            job.mutable_job_reference()->set_project_id(project_id);
            job.mutable_job_reference()->set_job_id(job_id);
            return job;
          });

  auto connection_impl =
      ConnectionImpl(mock_read_connection_, mock_job_connection_, {}, {},
                     mock_job_stub_, std::move(mock_background_), {});

  google::cloud::bigquery::v2::Job job;
  job.mutable_job_reference()->set_project_id(project_id);
  job.mutable_job_reference()->set_job_id(job_id);
  auto result = connection_impl.InsertJob(NoAwaitTag{}, job, {});
  ASSERT_STATUS_OK(result);
  EXPECT_THAT(result->project_id(), Eq(project_id));
  EXPECT_THAT(result->job_id(), Eq(job_id));
}

TEST_F(ConnectionImplTest, InsertJobAwaitImmediatelyDone) {
  std::string const project_id = "my-project";
  std::string const job_id = "my_job";

  EXPECT_CALL(*mock_background_, cq).WillOnce(Return(CompletionQueue{}));
  EXPECT_CALL(*mock_job_connection_, GetJob)
      .WillOnce([&](google::cloud::bigquery::v2::GetJobRequest const& request) {
        EXPECT_THAT(request.project_id(), Eq(project_id));
        EXPECT_THAT(request.job_id(), Eq(job_id));
        google::cloud::bigquery::v2::Job job;
        job.mutable_job_reference()->set_project_id(project_id);
        job.mutable_job_reference()->set_job_id(job_id);
        job.mutable_status()->set_state("DONE");
        return job;
      });

  auto bigquery_unified_options =
      Options{}.set<bigquery_unified::RetryPolicyOption>(
          std::make_unique<bigquery_unified::LimitedErrorCountRetryPolicy>(3));

  auto connection_impl = ConnectionImpl(
      mock_read_connection_, mock_job_connection_, {}, {}, mock_job_stub_,
      std::move(mock_background_), DefaultOptions(bigquery_unified_options));

  google::cloud::bigquery::v2::JobReference job_reference;
  job_reference.set_project_id(project_id);
  job_reference.set_job_id(job_id);
  auto result = connection_impl.InsertJob(job_reference, {}).get();
  EXPECT_THAT(result, IsOk());
}

TEST_F(ConnectionImplTest, InsertJobAwaitPollForDone) {
  std::string const project_id = "my-project";
  std::string const job_id = "my_job";

  EXPECT_CALL(*mock_job_connection_, GetJob)
      .WillOnce([&](google::cloud::bigquery::v2::GetJobRequest const& request) {
        EXPECT_THAT(request.project_id(), Eq(project_id));
        EXPECT_THAT(request.job_id(), Eq(job_id));
        google::cloud::bigquery::v2::Job job;
        job.mutable_job_reference()->set_project_id(project_id);
        job.mutable_job_reference()->set_job_id(job_id);
        job.mutable_status()->set_state("PENDING");
        return job;
      });

  EXPECT_CALL(*mock_job_stub_, GetJob)
      .WillOnce([&](rest_internal::RestContext&, google::cloud::Options const&,
                    google::cloud::bigquery::v2::GetJobRequest const& request)
                    -> StatusOr<google::cloud::bigquery::v2::Job> {
        EXPECT_THAT(request.project_id(), Eq(project_id));
        EXPECT_THAT(request.job_id(), Eq(job_id));
        google::cloud::bigquery::v2::Job job;
        job.mutable_job_reference()->set_project_id(project_id);
        job.mutable_job_reference()->set_job_id(job_id);
        job.mutable_status()->set_state("PENDING");
        return job;
      })
      .WillOnce([&](rest_internal::RestContext&, google::cloud::Options const&,
                    google::cloud::bigquery::v2::GetJobRequest const& request)
                    -> StatusOr<google::cloud::bigquery::v2::Job> {
        EXPECT_THAT(request.project_id(), Eq(project_id));
        EXPECT_THAT(request.job_id(), Eq(job_id));
        google::cloud::bigquery::v2::Job job;
        job.mutable_job_reference()->set_project_id(project_id);
        job.mutable_job_reference()->set_job_id(job_id);
        job.mutable_status()->set_state("PENDING");
        return job;
      })
      .WillOnce([&](rest_internal::RestContext&, google::cloud::Options const&,
                    google::cloud::bigquery::v2::GetJobRequest const& request)
                    -> StatusOr<google::cloud::bigquery::v2::Job> {
        EXPECT_THAT(request.project_id(), Eq(project_id));
        EXPECT_THAT(request.job_id(), Eq(job_id));
        google::cloud::bigquery::v2::Job job;
        job.mutable_job_reference()->set_project_id(project_id);
        job.mutable_job_reference()->set_job_id(job_id);
        job.mutable_status()->set_state("DONE");
        return job;
      });

  auto bigquery_unified_options = SetQuickPollingOptions({});

  auto unified_background = std::make_unique<
      rest_internal::AutomaticallyCreatedRestBackgroundThreads>();
  auto connection_impl = ConnectionImpl(
      mock_read_connection_, mock_job_connection_, {}, {}, mock_job_stub_,
      std::move(unified_background), DefaultOptions(bigquery_unified_options));

  google::cloud::bigquery::v2::JobReference job_reference;
  job_reference.set_project_id(project_id);
  job_reference.set_job_id(job_id);
  auto result = connection_impl.InsertJob(job_reference, {}).get();
  EXPECT_THAT(result, IsOk());
}

TEST_F(ConnectionImplTest, InsertJobAwaitPollDeadlineExceeded) {
  std::string const project_id = "my-project";
  std::string const job_id = "my_job";

  EXPECT_CALL(*mock_job_connection_, GetJob)
      .WillOnce([&](google::cloud::bigquery::v2::GetJobRequest const& request) {
        EXPECT_THAT(request.project_id(), Eq(project_id));
        EXPECT_THAT(request.job_id(), Eq(job_id));
        google::cloud::bigquery::v2::Job job;
        job.mutable_job_reference()->set_project_id(project_id);
        job.mutable_job_reference()->set_job_id(job_id);
        job.mutable_status()->set_state("PENDING");
        return job;
      });

  EXPECT_CALL(*mock_job_stub_, GetJob)
      .WillRepeatedly(
          [&](rest_internal::RestContext&, google::cloud::Options const&,
              google::cloud::bigquery::v2::GetJobRequest const& request)
              -> StatusOr<google::cloud::bigquery::v2::Job> {
            EXPECT_THAT(request.project_id(), Eq(project_id));
            EXPECT_THAT(request.job_id(), Eq(job_id));
            google::cloud::bigquery::v2::Job job;
            job.mutable_job_reference()->set_project_id(project_id);
            job.mutable_job_reference()->set_job_id(job_id);
            job.mutable_status()->set_state("PENDING");
            return job;
          });

  auto bigquery_unified_options = SetQuickPollingOptions({});

  auto unified_background = std::make_unique<
      rest_internal::AutomaticallyCreatedRestBackgroundThreads>();
  auto connection_impl = ConnectionImpl(
      mock_read_connection_, mock_job_connection_, {}, {}, mock_job_stub_,
      std::move(unified_background), DefaultOptions(bigquery_unified_options));

  google::cloud::bigquery::v2::JobReference job_reference;
  job_reference.set_project_id(project_id);
  job_reference.set_job_id(job_id);
  auto result = connection_impl.InsertJob(job_reference, {}).get();
  EXPECT_THAT(result, StatusIs(StatusCode::kDeadlineExceeded));
}

TEST_F(ConnectionImplTest, CancelJobNoAwait) {
  std::string const project_id = "my-project";
  std::string const job_id = "my_job";

  EXPECT_CALL(*mock_job_connection_, CancelJob)
      .WillOnce(
          [&](google::cloud::bigquery::v2::CancelJobRequest const& request) {
            EXPECT_THAT(request.project_id(), Eq(project_id));
            EXPECT_THAT(request.job_id(), Eq(job_id));
            google::cloud::bigquery::v2::JobCancelResponse response;
            google::cloud::bigquery::v2::Job job;
            job.mutable_job_reference()->set_project_id(project_id);
            job.mutable_job_reference()->set_job_id(job_id);
            *response.mutable_job() = job;
            return response;
          });

  auto connection_impl =
      ConnectionImpl(mock_read_connection_, mock_job_connection_, {}, {},
                     mock_job_stub_, std::move(mock_background_), {});

  google::cloud::bigquery::v2::CancelJobRequest request;
  request.set_job_id(job_id);
  request.set_project_id(project_id);
  auto result = connection_impl.CancelJob(NoAwaitTag{}, request, {});
  ASSERT_STATUS_OK(result);
  EXPECT_THAT(result->project_id(), Eq(project_id));
  EXPECT_THAT(result->job_id(), Eq(job_id));
}

TEST_F(ConnectionImplTest, CancelJobAwaitImmediatelyDone) {
  std::string const project_id = "my-project";
  std::string const job_id = "my_job";

  EXPECT_CALL(*mock_background_, cq).WillOnce(Return(CompletionQueue{}));
  EXPECT_CALL(*mock_job_connection_, GetJob)
      .WillOnce([&](google::cloud::bigquery::v2::GetJobRequest const& request) {
        EXPECT_THAT(request.project_id(), Eq(project_id));
        EXPECT_THAT(request.job_id(), Eq(job_id));
        google::cloud::bigquery::v2::Job job;
        job.mutable_job_reference()->set_project_id(project_id);
        job.mutable_job_reference()->set_job_id(job_id);
        job.mutable_status()->set_state("DONE");
        return job;
      });

  auto bigquery_unified_options =
      Options{}.set<bigquery_unified::RetryPolicyOption>(
          std::make_unique<bigquery_unified::LimitedErrorCountRetryPolicy>(3));

  auto connection_impl = ConnectionImpl(
      mock_read_connection_, mock_job_connection_, {}, {}, mock_job_stub_,
      std::move(mock_background_), DefaultOptions(bigquery_unified_options));

  google::cloud::bigquery::v2::JobReference job_reference;
  job_reference.set_project_id(project_id);
  job_reference.set_job_id(job_id);
  auto result = connection_impl.CancelJob(job_reference, {}).get();
  EXPECT_THAT(result, IsOk());
}

TEST_F(ConnectionImplTest, CancelJobAwaitPollForDone) {
  std::string const project_id = "my-project";
  std::string const job_id = "my_job";

  EXPECT_CALL(*mock_job_connection_, GetJob)
      .WillOnce([&](google::cloud::bigquery::v2::GetJobRequest const& request) {
        EXPECT_THAT(request.project_id(), Eq(project_id));
        EXPECT_THAT(request.job_id(), Eq(job_id));
        google::cloud::bigquery::v2::Job job;
        job.mutable_job_reference()->set_project_id(project_id);
        job.mutable_job_reference()->set_job_id(job_id);
        job.mutable_status()->set_state("PENDING");
        return job;
      });

  EXPECT_CALL(*mock_job_stub_, GetJob)
      .WillOnce([&](rest_internal::RestContext&, google::cloud::Options const&,
                    google::cloud::bigquery::v2::GetJobRequest const& request)
                    -> StatusOr<google::cloud::bigquery::v2::Job> {
        EXPECT_THAT(request.project_id(), Eq(project_id));
        EXPECT_THAT(request.job_id(), Eq(job_id));
        google::cloud::bigquery::v2::Job job;
        job.mutable_job_reference()->set_project_id(project_id);
        job.mutable_job_reference()->set_job_id(job_id);
        job.mutable_status()->set_state("PENDING");
        return job;
      })
      .WillOnce([&](rest_internal::RestContext&, google::cloud::Options const&,
                    google::cloud::bigquery::v2::GetJobRequest const& request)
                    -> StatusOr<google::cloud::bigquery::v2::Job> {
        EXPECT_THAT(request.project_id(), Eq(project_id));
        EXPECT_THAT(request.job_id(), Eq(job_id));
        google::cloud::bigquery::v2::Job job;
        job.mutable_job_reference()->set_project_id(project_id);
        job.mutable_job_reference()->set_job_id(job_id);
        job.mutable_status()->set_state("PENDING");
        return job;
      })
      .WillOnce([&](rest_internal::RestContext&, google::cloud::Options const&,
                    google::cloud::bigquery::v2::GetJobRequest const& request)
                    -> StatusOr<google::cloud::bigquery::v2::Job> {
        EXPECT_THAT(request.project_id(), Eq(project_id));
        EXPECT_THAT(request.job_id(), Eq(job_id));
        google::cloud::bigquery::v2::Job job;
        job.mutable_job_reference()->set_project_id(project_id);
        job.mutable_job_reference()->set_job_id(job_id);
        job.mutable_status()->set_state("DONE");
        return job;
      });

  auto bigquery_unified_options = SetQuickPollingOptions({});

  auto unified_background = std::make_unique<
      rest_internal::AutomaticallyCreatedRestBackgroundThreads>();
  auto connection_impl = ConnectionImpl(
      mock_read_connection_, mock_job_connection_, {}, {}, mock_job_stub_,
      std::move(unified_background), DefaultOptions(bigquery_unified_options));

  google::cloud::bigquery::v2::JobReference job_reference;
  job_reference.set_project_id(project_id);
  job_reference.set_job_id(job_id);
  auto result = connection_impl.CancelJob(job_reference, {}).get();
  EXPECT_THAT(result, IsOk());
}

TEST_F(ConnectionImplTest, CancelJobAwaitPollDeadlineExceeded) {
  std::string const project_id = "my-project";
  std::string const job_id = "my_job";

  EXPECT_CALL(*mock_job_connection_, GetJob)
      .WillOnce([&](google::cloud::bigquery::v2::GetJobRequest const& request) {
        EXPECT_THAT(request.project_id(), Eq(project_id));
        EXPECT_THAT(request.job_id(), Eq(job_id));
        google::cloud::bigquery::v2::Job job;
        job.mutable_job_reference()->set_project_id(project_id);
        job.mutable_job_reference()->set_job_id(job_id);
        job.mutable_status()->set_state("PENDING");
        return job;
      });

  EXPECT_CALL(*mock_job_stub_, GetJob)
      .WillRepeatedly(
          [&](rest_internal::RestContext&, google::cloud::Options const&,
              google::cloud::bigquery::v2::GetJobRequest const& request)
              -> StatusOr<google::cloud::bigquery::v2::Job> {
            EXPECT_THAT(request.project_id(), Eq(project_id));
            EXPECT_THAT(request.job_id(), Eq(job_id));
            google::cloud::bigquery::v2::Job job;
            job.mutable_job_reference()->set_project_id(project_id);
            job.mutable_job_reference()->set_job_id(job_id);
            job.mutable_status()->set_state("PENDING");
            return job;
          });

  auto bigquery_unified_options = SetQuickPollingOptions({});

  auto unified_background = std::make_unique<
      rest_internal::AutomaticallyCreatedRestBackgroundThreads>();
  auto connection_impl = ConnectionImpl(
      mock_read_connection_, mock_job_connection_, {}, {}, mock_job_stub_,
      std::move(unified_background), DefaultOptions(bigquery_unified_options));

  google::cloud::bigquery::v2::JobReference job_reference;
  job_reference.set_project_id(project_id);
  job_reference.set_job_id(job_id);
  auto result = connection_impl.CancelJob(job_reference, {}).get();
  EXPECT_THAT(result, StatusIs(StatusCode::kDeadlineExceeded));
}

}  // namespace
GOOGLE_CLOUD_CPP_BIGQUERY_INLINE_NAMESPACE_END
}  // namespace google::cloud::bigquery_unified_internal
