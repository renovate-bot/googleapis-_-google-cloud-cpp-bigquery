// Copyright 2025 Google LLC
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

#ifndef GOOGLE_CLOUD_CPP_BIGQUERY_GOOGLE_CLOUD_BIGQUERY_UNIFIED_IDEMPOTENCY_POLICY_H
#define GOOGLE_CLOUD_CPP_BIGQUERY_GOOGLE_CLOUD_BIGQUERY_UNIFIED_IDEMPOTENCY_POLICY_H

#include "google/cloud/bigquery/storage/v1/bigquery_read_connection.h"
#include "google/cloud/bigquery_unified/version.h"
#include "google/cloud/bigquerycontrol/v2/job_connection.h"
#include "google/cloud/idempotency.h"
#include "google/cloud/no_await_tag.h"

namespace google::cloud::bigquery_unified {
GOOGLE_CLOUD_CPP_BIGQUERY_INLINE_NAMESPACE_BEGIN

class IdempotencyPolicy {
 public:
  virtual ~IdempotencyPolicy() = default;

  /// Create a new copy of this object.
  virtual std::unique_ptr<IdempotencyPolicy> clone() const;

  virtual google::cloud::Idempotency CancelJob(
      google::cloud::bigquery::v2::CancelJobRequest const& request,
      Options opts);

  virtual google::cloud::Idempotency CancelJob(
      google::cloud::NoAwaitTag no_await_tag,
      google::cloud::bigquery::v2::CancelJobRequest const& request,
      Options opts);

  virtual google::cloud::Idempotency CancelJob(
      google::cloud::bigquery::v2::JobReference const& job_reference,
      Options opts);

  virtual google::cloud::Idempotency DeleteJob(
      google::cloud::bigquery::v2::DeleteJobRequest const& request,
      Options opts);

  virtual google::cloud::Idempotency GetJob(
      google::cloud::bigquery::v2::GetJobRequest const& request, Options opts);

  virtual google::cloud::Idempotency InsertJob(
      google::cloud::bigquery::v2::InsertJobRequest const& request,
      Options opts);

  virtual google::cloud::Idempotency InsertJob(
      google::cloud::NoAwaitTag,
      google::cloud::bigquery::v2::InsertJobRequest const& request,
      Options opts);

  virtual google::cloud::Idempotency InsertJob(
      google::cloud::bigquery::v2::JobReference const& job_reference,
      Options opts);

  virtual google::cloud::Idempotency ListJobs(
      google::cloud::bigquery::v2::ListJobsRequest request, Options opts);

  virtual google::cloud::Idempotency ReadArrow(
      google::cloud::bigquery::storage::v1::CreateReadSessionRequest const&
          read_session,
      Options opts);
};

std::unique_ptr<IdempotencyPolicy> MakeDefaultIdempotencyPolicy();

GOOGLE_CLOUD_CPP_BIGQUERY_INLINE_NAMESPACE_END
}  // namespace google::cloud::bigquery_unified

#endif  // GOOGLE_CLOUD_CPP_BIGQUERY_GOOGLE_CLOUD_BIGQUERY_UNIFIED_IDEMPOTENCY_POLICY_H
