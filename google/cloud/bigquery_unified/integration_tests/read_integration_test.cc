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

class ReadIntegrationTest : public ::testing::Test {
 protected:
  void SetUp() override {
    project_id_ =
        google::cloud::internal::GetEnv("GOOGLE_CLOUD_PROJECT").value_or("");
    ASSERT_FALSE(project_id_.empty());
  }
  std::string project_id_;
};

TEST_F(ReadIntegrationTest, ReadTable) {
  google::cloud::bigquery::v2::TableReference table_reference;
  table_reference.set_project_id("bigquery-public-data");
  table_reference.set_dataset_id("usa_names");
  table_reference.set_table_id("usa_1910_2013");

  auto options =
      google::cloud::Options{}.set<BillingProjectOption>(project_id_);
  std::shared_ptr<Connection> connection =
      google::cloud::bigquery_unified::MakeConnection();
  auto client = google::cloud::bigquery_unified::Client(connection);

  options.set<MaxReadStreamsOption>(1);
  auto read_response = client.ReadArrow(table_reference, options);
  ASSERT_STATUS_OK(read_response);

  for (StatusOr<std::shared_ptr<arrow::RecordBatch>>& batch :
       read_response->readers[0]) {
    ASSERT_STATUS_OK(batch);
    auto validate_full = (*batch)->ValidateFull();
    EXPECT_TRUE(validate_full == arrow::Status::OK());
  }
}

}  // namespace
GOOGLE_CLOUD_CPP_BIGQUERY_INLINE_NAMESPACE_END
}  // namespace google::cloud::bigquery_unified
