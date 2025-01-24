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
#include "google/cloud/bigquery_unified/job_options.h"
#include "google/cloud/bigquery_unified/read_options.h"
#include "google/cloud/internal/make_status.h"
#include "google/cloud/internal/pagination_range.h"
#include "google/cloud/options.h"
#include "google/cloud/project.h"

namespace google::cloud::bigquery_unified {
GOOGLE_CLOUD_CPP_BIGQUERY_INLINE_NAMESPACE_BEGIN
namespace {
std::string TableReferenceFullName(
    google::cloud::bigquery::v2::TableReference const& table_reference) {
  return absl::StrCat("projects/", table_reference.project_id(), "/datasets/",
                      table_reference.dataset_id(), "/tables/",
                      table_reference.table_id());
}

}  // namespace

Client::Client(std::shared_ptr<Connection> connection, Options opts)
    : connection_(std::move(connection)),
      options_(
          internal::MergeOptions(std::move(opts), connection_->options())) {}

future<StatusOr<google::cloud::bigquery::v2::Job>> Client::CancelJob(
    google::cloud::bigquery::v2::CancelJobRequest const& request,
    Options opts) {
  return connection_->CancelJob(
      request, internal::MergeOptions(std::move(opts), options_));
}

StatusOr<google::cloud::bigquery::v2::JobReference> Client::CancelJob(
    google::cloud::NoAwaitTag,
    google::cloud::bigquery::v2::CancelJobRequest const& request,
    Options opts) {
  return connection_->CancelJob(
      google::cloud::NoAwaitTag{}, request,
      internal::MergeOptions(std::move(opts), options_));
}

future<StatusOr<google::cloud::bigquery::v2::Job>> Client::CancelJob(
    google::cloud::bigquery::v2::JobReference const& job_reference,
    Options opts) {
  return connection_->CancelJob(
      job_reference, internal::MergeOptions(std::move(opts), options_));
}

StatusOr<google::cloud::bigquery::v2::Job> Client::GetJob(
    google::cloud::bigquery::v2::GetJobRequest const& request, Options opts) {
  return connection_->GetJob(request,
                             internal::MergeOptions(std::move(opts), options_));
}

Status Client::DeleteJob(
    google::cloud::bigquery::v2::DeleteJobRequest const& request,
    Options opts) {
  return connection_->DeleteJob(
      request, internal::MergeOptions(std::move(opts), options_));
}

StreamRange<google::cloud::bigquery::v2::ListFormatJob> Client::ListJobs(
    google::cloud::bigquery::v2::ListJobsRequest request, Options opts) {
  return connection_->ListJobs(
      std::move(request), internal::MergeOptions(std::move(opts), options_));
}

future<StatusOr<google::cloud::bigquery::v2::Job>> Client::InsertJob(
    google::cloud::bigquery::v2::Job const& job, Options opts) {
  return connection_->InsertJob(
      job, internal::MergeOptions(std::move(opts), options_));
}

StatusOr<google::cloud::bigquery::v2::JobReference> Client::InsertJob(
    google::cloud::NoAwaitTag, google::cloud::bigquery::v2::Job const& job,
    Options opts) {
  return connection_->InsertJob(
      google::cloud::NoAwaitTag{}, job,
      internal::MergeOptions(std::move(opts), options_));
}

future<StatusOr<google::cloud::bigquery::v2::Job>> Client::InsertJob(
    google::cloud::bigquery::v2::JobReference const& job_reference,
    Options opts) {
  return connection_->InsertJob(
      job_reference, internal::MergeOptions(std::move(opts), options_));
}

StatusOr<ReadArrowResponse> Client::ReadArrow(
    google::cloud::bigquery::v2::Job const& job, Options opts) {
  return internal::UnimplementedError("not implemented");
}
StatusOr<ReadArrowResponse> Client::ReadArrow(
    google::cloud::bigquery::v2::JobReference const& job_reference,
    Options opts) {
  return internal::UnimplementedError("not implemented");
}

StatusOr<ReadArrowResponse> Client::ReadArrow(
    google::cloud::bigquery::v2::TableReference const& table_reference,
    Options opts) {
  auto current_options = internal::MergeOptions(std::move(opts), options_);
  google::cloud::bigquery::storage::v1::CreateReadSessionRequest
      read_session_request;
  auto const billing_project =
      current_options.has<bigquery_unified::BillingProjectOption>()
          ? current_options.get<bigquery_unified::BillingProjectOption>()
          : table_reference.project_id();

  read_session_request.set_parent(
      google::cloud::Project(billing_project).FullName());

  if (current_options.has<bigquery_unified::MaxReadStreamsOption>()) {
    read_session_request.set_max_stream_count(
        current_options.get<bigquery_unified::MaxReadStreamsOption>());
  }

  if (current_options
          .has<bigquery_unified::PreferredMinimumReadStreamsOption>()) {
    read_session_request.set_preferred_min_stream_count(
        current_options
            .get<bigquery_unified::PreferredMinimumReadStreamsOption>());
  }

  google::cloud::bigquery::storage::v1::ReadSession read_session;
  read_session.set_data_format(
      google::cloud::bigquery::storage::v1::DataFormat::ARROW);
  read_session.set_table(TableReferenceFullName(table_reference));
  *read_session_request.mutable_read_session() = read_session;

  return connection_->ReadArrow(read_session_request,
                                std::move(current_options));
}

StatusOr<ReadArrowResponse> Client::ReadArrow(
    google::cloud::bigquery::storage::v1::CreateReadSessionRequest const&
        read_session,
    Options opts) {
  return internal::UnimplementedError("not implemented");
}

GOOGLE_CLOUD_CPP_BIGQUERY_INLINE_NAMESPACE_END
}  // namespace google::cloud::bigquery_unified
