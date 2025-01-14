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

#include "google/cloud/bigquery_unified/idempotency_policy.h"
#include "google/cloud/bigquery_unified/version.h"

namespace google::cloud::bigquery_unified {
GOOGLE_CLOUD_CPP_BIGQUERY_INLINE_NAMESPACE_BEGIN
using ::google::cloud::Idempotency;

/// Create a new copy of this object.
std::unique_ptr<IdempotencyPolicy> IdempotencyPolicy::clone() const {
  return std::make_unique<IdempotencyPolicy>(*this);
}

Idempotency IdempotencyPolicy::CancelJob(
    google::cloud::bigquery::v2::CancelJobRequest const&, Options) {
  return Idempotency::kIdempotent;
}

Idempotency IdempotencyPolicy::CancelJob(
    google::cloud::NoAwaitTag,
    google::cloud::bigquery::v2::CancelJobRequest const&, Options) {
  return Idempotency::kIdempotent;
}

Idempotency IdempotencyPolicy::CancelJob(
    google::cloud::bigquery::v2::JobReference const&, Options) {
  return Idempotency::kIdempotent;
}

Idempotency IdempotencyPolicy::GetJob(
    google::cloud::bigquery::v2::GetJobRequest const&, Options) {
  return Idempotency::kIdempotent;
}

Idempotency IdempotencyPolicy::DeleteJob(
    google::cloud::bigquery::v2::DeleteJobRequest const&, Options) {
  return Idempotency::kIdempotent;
}

Idempotency IdempotencyPolicy::InsertJob(
    google::cloud::bigquery::v2::InsertJobRequest const&, Options) {
  return Idempotency::kNonIdempotent;
}

Idempotency IdempotencyPolicy::InsertJob(
    google::cloud::NoAwaitTag,
    google::cloud::bigquery::v2::InsertJobRequest const&, Options) {
  return Idempotency::kNonIdempotent;
}

Idempotency IdempotencyPolicy::InsertJob(
    google::cloud::bigquery::v2::JobReference const&, Options) {
  return Idempotency::kNonIdempotent;
}

Idempotency IdempotencyPolicy::ListJobs(
    google::cloud::bigquery::v2::ListJobsRequest, Options) {
  return Idempotency::kIdempotent;
}

Idempotency IdempotencyPolicy::ReadArrow(
    google::cloud::bigquery::storage::v1::CreateReadSessionRequest const&,
    Options) {
  return Idempotency::kIdempotent;
}

std::unique_ptr<IdempotencyPolicy> MakeDefaultIdempotencyPolicy() {
  return std::make_unique<IdempotencyPolicy>();
}

GOOGLE_CLOUD_CPP_BIGQUERY_INLINE_NAMESPACE_END
}  // namespace google::cloud::bigquery_unified
