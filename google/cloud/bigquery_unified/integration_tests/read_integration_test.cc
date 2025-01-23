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

#include "google/cloud/bigquery_unified/client.h"
#include "google/cloud/bigquery_unified/job_options.h"
#include "google/cloud/bigquery_unified/read_options.h"
#include "google/cloud/bigquery_unified/testing_util/status_matchers.h"
#include "google/cloud/bigquery_unified/version.h"
#include "google/cloud/internal/getenv.h"
#include <gmock/gmock.h>

namespace google::cloud::bigquery_unified {
GOOGLE_CLOUD_CPP_BIGQUERY_INLINE_NAMESPACE_BEGIN
namespace {

using ::google::cloud::bigquery_unified::testing_util::IsOk;
using ::testing::Eq;
using ::testing::Gt;
namespace bigquery_proto = ::google::cloud::bigquery::v2;

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

class ReadIntegrationTest : public ::testing::Test {
 protected:
  void SetUp() override {
    project_id_ =
        google::cloud::internal::GetEnv("GOOGLE_CLOUD_PROJECT").value_or("");
    ASSERT_FALSE(project_id_.empty());
  }
  std::string project_id_;
};

TEST_F(ReadIntegrationTest, ReadArrowTableReference) {
  bigquery_proto::TableReference table_reference;
  table_reference.set_project_id("bigquery-public-data");
  table_reference.set_dataset_id("usa_names");
  table_reference.set_table_id("usa_1910_2013");

  auto options =
      google::cloud::Options{}.set<BillingProjectOption>(project_id_);
  std::shared_ptr<Connection> connection =
      google::cloud::bigquery_unified::MakeConnection();
  auto client = google::cloud::bigquery_unified::Client(connection);

  options.set<bigquery_unified::PreferredMinimumReadStreamsOption>(2);
  auto read_response = client.ReadArrow(table_reference, options);
  ASSERT_STATUS_OK(read_response);

  EXPECT_THAT(read_response->readers.size(), Gt(2));
  for (auto& reader : read_response->readers) {
    for (StatusOr<std::shared_ptr<arrow::RecordBatch>>& batch : reader) {
      ASSERT_STATUS_OK(batch);
      auto validate_full = (*batch)->ValidateFull();
      EXPECT_TRUE(validate_full == arrow::Status::OK());
    }
  }
}

TEST_F(ReadIntegrationTest, ReadArrowJobReference) {
  std::shared_ptr<Connection> connection = MakeConnection();
  auto client = Client(connection);

  auto job = MakeQueryJob(
      "SELECT name, state, year, sum(number) as total "
      "FROM `bigquery-public-data.usa_names.usa_1910_2013` "
      "WHERE year >= 2000 "
      "GROUP BY name, state, year "
      "LIMIT 100");
  auto options = Options{}.set<BillingProjectOption>(project_id_);
  auto query_job = client.InsertJob(job, options).get();
  ASSERT_STATUS_OK(query_job);

  options.set<MaxReadStreamsOption>(1);
  auto read_response = client.ReadArrow(query_job->job_reference(), options);
  ASSERT_STATUS_OK(read_response);
  EXPECT_THAT(read_response->estimated_row_count, Eq(100));

  ASSERT_THAT(read_response->readers.size(), Eq(1));
  for (StatusOr<std::shared_ptr<arrow::RecordBatch>>& batch :
       read_response->readers[0]) {
    ASSERT_STATUS_OK(batch);
    auto validate_full = (*batch)->ValidateFull();
    EXPECT_TRUE(validate_full == arrow::Status::OK());
  }
}

TEST_F(ReadIntegrationTest, ReadArrowJob) {
  std::shared_ptr<Connection> connection = MakeConnection();
  auto client = Client(connection);

  auto job = MakeQueryJob(
      "SELECT name, state, year, sum(number) as total "
      "FROM `bigquery-public-data.usa_names.usa_1910_2013` "
      "WHERE year >= 2000 "
      "GROUP BY name, state, year "
      "LIMIT 100");
  auto options = Options{}
                     .set<BillingProjectOption>(project_id_)
                     .set<MaxReadStreamsOption>(1);
  client.InsertJob(job, options).then([client, options](auto f) mutable {
    auto job = f.get();
    ASSERT_STATUS_OK(job);
    auto read_response = client.ReadArrow(*job, options);
    ASSERT_STATUS_OK(read_response);
    EXPECT_THAT(read_response->estimated_row_count, Eq(100));

    ASSERT_THAT(read_response->readers.size(), Eq(1));
    for (StatusOr<std::shared_ptr<arrow::RecordBatch>>& batch :
         read_response->readers[0]) {
      ASSERT_STATUS_OK(batch);
      auto validate_full = (*batch)->ValidateFull();
      EXPECT_TRUE(validate_full == arrow::Status::OK());
    }
  });
}

}  // namespace
GOOGLE_CLOUD_CPP_BIGQUERY_INLINE_NAMESPACE_END
}  // namespace google::cloud::bigquery_unified
