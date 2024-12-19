// Copyright 2024 Google LLC
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//      https://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#include "google/cloud/bigquery_unified/client.h"
#include "google/cloud/bigquery_unified/mocks/mock_connection.h"
#include "google/cloud/bigquery_unified/testing_util/status_matchers.h"

namespace google::cloud::bigquery_unified {
GOOGLE_CLOUD_CPP_BIGQUERY_INLINE_NAMESPACE_BEGIN
namespace {

using ::google::cloud::bigquery_unified::testing_util::IsOkAndHolds;
using ::testing::AllOf;
using ::testing::Eq;
using ::testing::ResultOf;
using ::testing::Return;

struct TestOption {
  using Type = std::string;
};

TEST(BigQueryUnifiedClientTest, GetJobSuccess) {
  auto mock_connection = std::make_shared<MockConnection>();

  auto call_options = Options{}.set<TestOption>("call-test-option");

  EXPECT_CALL(*mock_connection, GetJob)
      .WillOnce([&](google::cloud::bigquery::v2::GetJobRequest const& request,
                    Options opts) {
        EXPECT_THAT(request.project_id(), Eq("my-project-id"));
        EXPECT_THAT(opts.get<TestOption>(), Eq(call_options.get<TestOption>()));
        google::cloud::bigquery::v2::Job job;
        job.mutable_job_reference()->set_project_id("my-project-id");
        job.mutable_job_reference()->set_job_id("my-job-id");
        return job;
      });

  auto client =
      Client(mock_connection, Options{}.set<TestOption>("client-test-option"));
  google::cloud::bigquery::v2::GetJobRequest request;
  request.set_project_id("my-project-id");
  auto get_result = client.GetJob(request, call_options);
  EXPECT_THAT(
      get_result,
      IsOkAndHolds(AllOf(
          ResultOf(
              "project id",
              [](auto const& x) { return x.job_reference().project_id(); },
              "my-project-id"),
          ResultOf(
              "job id",
              [](auto const& x) { return x.job_reference().job_id(); },
              "my-job-id"))));
}

}  // namespace
GOOGLE_CLOUD_CPP_BIGQUERY_INLINE_NAMESPACE_END
}  // namespace google::cloud::bigquery_unified
