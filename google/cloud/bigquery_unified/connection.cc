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

#include "google/cloud/bigquery_unified/connection.h"
#include "google/cloud/bigquery_unified/internal/connection_impl.h"
#include "google/cloud/bigquery_unified/internal/default_options.h"
#include "google/cloud/internal/make_status.h"
#include "google/cloud/internal/pagination_range.h"

namespace google::cloud::bigquery_unified {
GOOGLE_CLOUD_CPP_BIGQUERY_INLINE_NAMESPACE_BEGIN

Connection::~Connection() = default;

// CancelJob
future<StatusOr<google::cloud::bigquery::v2::Job>> Connection::CancelJob(
    google::cloud::bigquery::v2::CancelJobRequest const& request,
    Options opts) {
  return google::cloud::make_ready_future<
      StatusOr<google::cloud::bigquery::v2::Job>>(
      Status(StatusCode::kUnimplemented, "not implemented"));
}

StatusOr<google::cloud::bigquery::v2::JobReference> Connection::CancelJob(
    google::cloud::NoAwaitTag,
    google::cloud::bigquery::v2::CancelJobRequest const& request,
    Options opts) {
  return internal::UnimplementedError("not implemented");
}

future<StatusOr<google::cloud::bigquery::v2::JobCancelResponse>>
Connection::CancelJob(
    google::cloud::bigquery::v2::JobReference const& job_reference,
    Options opts) {
  return google::cloud::make_ready_future<
      StatusOr<google::cloud::bigquery::v2::JobCancelResponse>>(
      Status(StatusCode::kUnimplemented, "not implemented"));
}

// GetJob
StatusOr<google::cloud::bigquery::v2::Job> Connection::GetJob(
    google::cloud::bigquery::v2::GetJobRequest const& request, Options opts) {
  return internal::UnimplementedError("not implemented");
}

// DeleteJob
Status Connection::DeleteJob(
    google::cloud::bigquery::v2::DeleteJobRequest const& request,
    Options opts) {
  return internal::UnimplementedError("not implemented");
}

// ListJobs
StreamRange<google::cloud::bigquery::v2::ListFormatJob> Connection::ListJobs(
    google::cloud::bigquery::v2::ListJobsRequest request, Options opts) {
  return internal::MakeUnimplementedPaginationRange<
      StreamRange<google::cloud::bigquery::v2::ListFormatJob>>();
}

// InsertJob
future<StatusOr<google::cloud::bigquery::v2::Job>> Connection::InsertJob(
    google::cloud::bigquery::v2::Job const& job, Options opts) {
  return google::cloud::make_ready_future<
      StatusOr<google::cloud::bigquery::v2::Job>>(
      Status(StatusCode::kUnimplemented, "not implemented"));
}

StatusOr<google::cloud::bigquery::v2::JobReference> Connection::InsertJob(
    google::cloud::NoAwaitTag, google::cloud::bigquery::v2::Job const& job,
    Options opts) {
  return internal::UnimplementedError("not implemented");
}

future<StatusOr<google::cloud::bigquery::v2::Job>> Connection::InsertJob(
    google::cloud::bigquery::v2::JobReference const& job_reference,
    Options opts) {
  return google::cloud::make_ready_future<
      StatusOr<google::cloud::bigquery::v2::Job>>(
      Status(StatusCode::kUnimplemented, "not implemented"));
}

StatusOr<ReadArrowResponse> Connection::ReadArrow(
    google::cloud::bigquery::storage::v1::CreateReadSessionRequest const&
        read_session,
    Options opts) {
  return internal::UnimplementedError("not implemented");
}

std::shared_ptr<Connection> MakeConnection(Options options) {
  return bigquery_unified_internal::MakeDefaultConnectionImpl(
      bigquery_unified_internal::DefaultOptions(std::move(options)));
}

GOOGLE_CLOUD_CPP_BIGQUERY_INLINE_NAMESPACE_END
}  // namespace google::cloud::bigquery_unified
