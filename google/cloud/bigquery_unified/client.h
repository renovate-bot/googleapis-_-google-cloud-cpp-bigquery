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

#ifndef GOOGLE_CLOUD_CPP_BIGQUERY_GOOGLE_CLOUD_BIGQUERY_UNIFIED_CLIENT_H
#define GOOGLE_CLOUD_CPP_BIGQUERY_GOOGLE_CLOUD_BIGQUERY_UNIFIED_CLIENT_H

#include "google/cloud/bigquery_unified/connection.h"
#include "google/cloud/future.h"
#include "google/cloud/no_await_tag.h"
#include "google/cloud/options.h"
#include "google/cloud/bigquery_unified/read_arrow_response.h"
#include "google/cloud/bigquery_unified/version.h"
#include "google/cloud/status_or.h"
#include <google/cloud/bigquery/storage/v1/storage.pb.h>
#include <google/cloud/bigquery/v2/job.pb.h>
#include <memory>

namespace google::cloud::bigquery_unified {
GOOGLE_CLOUD_CPP_BIGQUERY_INLINE_NAMESPACE_BEGIN

///
/// BigQuery Unified API.
///
/// TODO: description here
///
/// @par Equality
///
/// Instances of this class created via copy-construction or copy-assignment
/// always compare equal. Instances created with equal
/// `std::shared_ptr<*Connection>` objects compare equal. Objects that compare
/// equal share the same underlying resources.
///
/// @par Performance
///
/// Creating a new instance of this class is a relatively expensive operation,
/// new objects establish new connections to the service. In contrast,
/// copy-construction, move-construction, and the corresponding assignment
/// operations are relatively efficient as the copies share all underlying
/// resources.
///
/// @par Thread Safety
///
/// Concurrent access to different instances of this class, even if they compare
/// equal, is guaranteed to work. Two or more threads operating on the same
/// instance of this class is not guaranteed to work. Since copy-construction
/// and move-construction is a relatively efficient operation, consider using
/// such a copy when using this class from multiple threads.
///
class Client {
 public:
  explicit Client(std::shared_ptr<Connection> connection, Options opts = {});
  ~Client() = default;

  ///@{
  /// @name Copy and move support
  Client(Client const&) = default;
  Client& operator=(Client const&) = default;
  Client(Client&&) = default;
  Client& operator=(Client&&) = default;
  ///@}

  ///@{
  /// @name Equality
  friend bool operator==(Client const& a, Client const& b) {
    return a.connection_ == b.connection_;
  }
  friend bool operator!=(Client const& a, Client const& b) { return !(a == b); }
  ///@}

  // CancelJob
  future<StatusOr<google::cloud::bigquery::v2::JobCancelResponse>> CancelJob(
      google::cloud::bigquery::v2::CancelJobRequest const& request,
      Options opts = {});

  StatusOr<google::cloud::bigquery::v2::JobReference> CancelJob(
      google::cloud::NoAwaitTag,
      google::cloud::bigquery::v2::CancelJobRequest const& request,
      Options opts = {});

  future<StatusOr<google::cloud::bigquery::v2::JobCancelResponse>> CancelJob(
      google::cloud::bigquery::v2::JobReference const& job_reference,
      Options opts = {});

  // GetJob
  StatusOr<google::cloud::bigquery::v2::Job> GetJob(
      google::cloud::bigquery::v2::GetJobRequest const& request,
      Options opts = {});

  // DeleteJob
  Status DeleteJob(google::cloud::bigquery::v2::DeleteJobRequest const& request,
                   Options opts = {});

  // ListJobs
  StreamRange<google::cloud::bigquery::v2::ListFormatJob> ListJobs(
      google::cloud::bigquery::v2::ListJobsRequest request, Options opts = {});

  // InsertJob
  future<StatusOr<google::cloud::bigquery::v2::Job>> InsertJob(
      google::cloud::bigquery::v2::InsertJobRequest const& request,
      Options opts = {});

  StatusOr<google::cloud::bigquery::v2::JobReference> InsertJob(
      google::cloud::NoAwaitTag,
      google::cloud::bigquery::v2::InsertJobRequest const& request,
      Options opts = {});

  future<StatusOr<google::cloud::bigquery::v2::Job>> InsertJob(
      google::cloud::bigquery::v2::JobReference const& job_reference,
      Options opts = {});

  // ReadArrow
  //  defined in read_arrow_response.h
  //  struct ReadArrowResponse {
  //    std::int64_t estimated_total_bytes;
  //    std::int64_t estimated_total_physical_file_size;
  //    std::int64_t estimated_row_count;
  //    google::protobuf::Timestamp expire_time;
  //    std::shared_ptr<arrow::Schema> schema;
  //    std::vector<StreamRange<std::shared_ptr<arrow::RecordBatch>>> readers;
  //  };
  //
  // Unless `bigquery_unified::BillingProjectOption is set, the billing project
  // is assumed to be the same as the project owning the table.
  // Unless `bigquery_unified::SingleReaderOption` is set, the service
  // suggested number of readers will be present in the response.
  // Setting `bigquery_unified::SingleReaderOption` is required to guarantee
  // ordering when reading results from ordered queries.
  StatusOr<ReadArrowResponse> ReadArrow(
      google::cloud::bigquery::v2::Job const& job,
      Options opts = {});
  StatusOr<ReadArrowResponse> ReadArrow(
      google::cloud::bigquery::v2::JobReference const& job_reference,
      Options opts = {});
  StatusOr<ReadArrowResponse> ReadArrow(
      google::cloud::bigquery::v2::TableReference const& table_reference,
      Options opts = {});
  // This ReadArrow overload allows for full customization of the read session,
  // except for AVRO format or AVRO serialization options which are ignored.
  StatusOr<ReadArrowResponse> ReadArrow(
      google::cloud::bigquery::storage::v1::CreateReadSessionRequest const& read_session,
      Options opts = {});

 private:
  std::shared_ptr<Connection> connection_;
  Options options_;
};

GOOGLE_CLOUD_CPP_BIGQUERY_INLINE_NAMESPACE_END
}  // namespace google::cloud::bigquery_unified

#endif  // GOOGLE_CLOUD_CPP_BIGQUERY_GOOGLE_CLOUD_BIGQUERY_UNIFIED_CLIENT_H
