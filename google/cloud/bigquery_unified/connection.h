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

#ifndef GOOGLE_CLOUD_CPP_BIGQUERY_GOOGLE_CLOUD_BIGQUERY_UNIFIED_CONNECTION_H
#define GOOGLE_CLOUD_CPP_BIGQUERY_GOOGLE_CLOUD_BIGQUERY_UNIFIED_CONNECTION_H

#include "google/cloud/bigquery_unified/read_arrow_response.h"
#include "google/cloud/bigquery_unified/version.h"
#include "google/cloud/future.h"
#include "google/cloud/no_await_tag.h"
#include "google/cloud/options.h"
#include "google/cloud/status_or.h"
#include "google/cloud/stream_range.h"
#include <google/cloud/bigquery/storage/v1/storage.pb.h>
#include <google/cloud/bigquery/v2/job.pb.h>
#include <arrow/record_batch.h>
#include <memory>

namespace google::cloud::bigquery_unified {
GOOGLE_CLOUD_CPP_BIGQUERY_INLINE_NAMESPACE_BEGIN

/**
 * The `Connection` object for `Client`.
 *
 * This interface defines virtual methods for each of the user-facing overload
 * sets in `Client`. This allows users to inject custom behavior
 * (e.g., with a Google Mock object) when writing tests that use objects of type
 * `Client`.
 *
 * To create a concrete instance, see `MakeConnection()`.
 *
 * For mocking, see `bigquery_unified_mocks::MockConnection`.
 */
class Connection {
 public:
  virtual ~Connection() = 0;

  virtual Options options() { return Options{}; }

  // CancelJob
  virtual future<StatusOr<google::cloud::bigquery::v2::Job>> CancelJob(
      google::cloud::bigquery::v2::CancelJobRequest const& request,
      Options opts);

  virtual StatusOr<google::cloud::bigquery::v2::JobReference> CancelJob(
      google::cloud::NoAwaitTag,
      google::cloud::bigquery::v2::CancelJobRequest const& request,
      Options opts);

  virtual future<StatusOr<google::cloud::bigquery::v2::Job>> CancelJob(
      google::cloud::bigquery::v2::JobReference const& job_reference,
      Options opts);

  // GetJob
  virtual StatusOr<google::cloud::bigquery::v2::Job> GetJob(
      google::cloud::bigquery::v2::GetJobRequest const& request, Options opts);

  // DeleteJob
  virtual Status DeleteJob(
      google::cloud::bigquery::v2::DeleteJobRequest const& request,
      Options opts);

  // ListJobs
  virtual StreamRange<google::cloud::bigquery::v2::ListFormatJob> ListJobs(
      google::cloud::bigquery::v2::ListJobsRequest request, Options opts);

  // InsertJob
  virtual future<StatusOr<google::cloud::bigquery::v2::Job>> InsertJob(
      google::cloud::bigquery::v2::Job const& job, Options opts);

  virtual StatusOr<google::cloud::bigquery::v2::JobReference> InsertJob(
      google::cloud::NoAwaitTag, google::cloud::bigquery::v2::Job const& job,
      Options opts);

  virtual future<StatusOr<google::cloud::bigquery::v2::Job>> InsertJob(
      google::cloud::bigquery::v2::JobReference const& job_reference,
      Options opts);

  virtual StatusOr<ReadArrowResponse> ReadArrow(
      google::cloud::bigquery::storage::v1::CreateReadSessionRequest const&
          read_session,
      Options opts);
};

/**
 * A factory function to construct an object of type `Connection`.
 *
 * The returned connection object should not be used directly; instead it
 * should be passed as an argument to the constructor of Client.
 *
 * The optional @p options argument may be used to configure aspects of the
 * returned `Connection`. Expected options are any of the types in
 * the following option lists:
 *
 * - `google::cloud::CommonOptionList`
 * - `google::cloud::GrpcOptionList`
 * - `google::cloud::UnifiedCredentialsOptionList`
 * // TODO: figure out other acceptable OptionLists
 *
 * @note Unexpected options will be ignored. To log unexpected options instead,
 *     set `GOOGLE_CLOUD_CPP_ENABLE_CLOG=yes` in the environment.
 *
 * @param options (optional) Configure the `Connection` created by
 * this function.
 */
std::shared_ptr<Connection> MakeConnection(Options options = {});

GOOGLE_CLOUD_CPP_BIGQUERY_INLINE_NAMESPACE_END
}  // namespace google::cloud::bigquery_unified

#endif  // GOOGLE_CLOUD_CPP_BIGQUERY_GOOGLE_CLOUD_BIGQUERY_UNIFIED_CONNECTION_H
