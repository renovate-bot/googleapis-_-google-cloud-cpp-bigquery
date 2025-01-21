// Copyright 2024 Google LLC
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

#include "google/cloud/bigquery_unified/client.h"
#include "google/cloud/bigquery_unified/job_options.h"
#include "google/cloud/bigquery_unified/testing_util/status_matchers.h"
#include "google/cloud/bigquery_unified/version.h"
#include "google/cloud/internal/getenv.h"
#include <gmock/gmock.h>
#include <thread>

namespace google::cloud::bigquery_unified {
GOOGLE_CLOUD_CPP_BIGQUERY_INLINE_NAMESPACE_BEGIN
namespace {

using ::google::cloud::bigquery_unified::testing_util::IsOk;
using ::testing::Eq;
namespace bigquery_proto = google::cloud::bigquery::v2;

class JobIntegrationTest : public ::testing::Test {
 protected:
  void SetUp() override {
    project_id_ = internal::GetEnv("GOOGLE_CLOUD_PROJECT").value_or("");
    ASSERT_FALSE(project_id_.empty());
  }
  std::string project_id_;
};

bigquery_proto::Job MakeQueryJob(std::string query_text) {
  bigquery_proto::JobConfigurationQuery query;
  query.mutable_use_legacy_sql()->set_value(false);
  query.set_query(std::move(query_text));

  bigquery_proto::JobConfiguration config;
  *config.mutable_query() = query;
  config.mutable_labels()->insert({"test_suite", "job_integration_test"});
  config.mutable_labels()->insert({"test_case", "insert_job"});

  bigquery_proto::Job job;
  *job.mutable_configuration() = config;
  return job;
}

TEST_F(JobIntegrationTest, InsertJobWithJobInputTest) {
  std::shared_ptr<Connection> connection = MakeConnection();
  auto client = Client(connection);

  // insert a new job by making the query
  auto job = MakeQueryJob(
      "SELECT name, state, year, sum(number) as total "
      "FROM `bigquery-public-data.usa_names.usa_1910_2013` "
      "WHERE year >= 2000 "
      "GROUP BY name, state, year "
      "LIMIT 100");
  auto options = Options{}.set<BillingProjectOption>(project_id_);
  auto query_job = client.InsertJob(job, options).get();
  ASSERT_STATUS_OK(query_job);
  auto job_id = query_job->job_reference().job_id();

  // get the inserted job
  bigquery_proto::GetJobRequest get_request;
  get_request.set_project_id(project_id_);
  get_request.set_job_id(job_id);
  auto get_job = client.GetJob(get_request);
  EXPECT_THAT(get_job, IsOk());
  EXPECT_THAT(get_job->status().state(), Eq("DONE"));

  // list all jobs of the project, find the inserted job
  bigquery_proto::ListJobsRequest list_request;
  list_request.set_project_id(project_id_);
  auto list_jobs = client.ListJobs(list_request);
  bool find_job = false;
  for (auto const& job : list_jobs) {
    EXPECT_THAT(job, IsOk());
    if (job->job_reference().job_id() == job_id) {
      find_job = true;
      break;
    }
  }
  EXPECT_TRUE(find_job);

  // delete the inserted job
  bigquery_proto::DeleteJobRequest delete_request;
  delete_request.set_project_id(project_id_);
  delete_request.set_job_id(job_id);
  auto delete_job = client.DeleteJob(delete_request);
  ASSERT_STATUS_OK(delete_job);
}

TEST_F(JobIntegrationTest, InsertJobNoAwaitTest) {
  std::shared_ptr<Connection> connection = MakeConnection();
  auto client = Client(connection);

  // insert a new job by making the query
  auto job = MakeQueryJob(
      "SELECT name, state, year, sum(number) as total "
      "FROM `bigquery-public-data.usa_names.usa_1910_2013` "
      "WHERE year >= 2000 "
      "GROUP BY name, state, year "
      "LIMIT 100");
  auto options = Options{}.set<BillingProjectOption>(project_id_);

  auto job_ref = client.InsertJob(NoAwaitTag{}, job, options);
  ASSERT_STATUS_OK(job_ref);
  auto job_id = job_ref->job_id();

  std::this_thread::sleep_for(std::chrono::seconds(2));

  // get the inserted job
  bigquery_proto::GetJobRequest get_request;
  get_request.set_project_id(project_id_);
  get_request.set_job_id(job_id);
  auto get_job = client.GetJob(get_request);
  EXPECT_THAT(get_job, IsOk());
  EXPECT_THAT(get_job->status().state(), Eq("DONE"));

  // delete the inserted job
  bigquery_proto::DeleteJobRequest delete_request;
  delete_request.set_project_id(project_id_);
  delete_request.set_job_id(job_id);
  auto delete_job = client.DeleteJob(delete_request);
  ASSERT_STATUS_OK(delete_job);
}

}  // namespace
GOOGLE_CLOUD_CPP_BIGQUERY_INLINE_NAMESPACE_END
}  // namespace google::cloud::bigquery_unified
