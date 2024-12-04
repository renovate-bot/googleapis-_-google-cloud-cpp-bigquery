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

#ifndef GOOGLE_CLOUD_CPP_BIGQUERY_GOOGLE_CLOUD_BIGQUERY_UNIFIED_READ_ARROW_RESPONSE_H
#define GOOGLE_CLOUD_CPP_BIGQUERY_GOOGLE_CLOUD_BIGQUERY_UNIFIED_READ_ARROW_RESPONSE_H

#include "google/cloud/bigquery_unified/version.h"
#include "google/cloud/stream_range.h"
#include <google/protobuf/timestamp.pb.h>
#include <arrow/record_batch.h>
#include <memory>
#include <vector>

namespace google::cloud::bigquery_unified {
GOOGLE_CLOUD_CPP_BIGQUERY_INLINE_NAMESPACE_BEGIN

// ReadRowsResponse.StreamStats, ReadRowsResponse.ThrottleState, and
// ReadRowsResponse.uncompressed_byte_size are not made available. If there is
// demand for these, we can look at adding mechanisms to retrieve those.
struct ReadArrowResponse {
  std::int64_t estimated_total_bytes;
  std::int64_t estimated_total_physical_file_size;
  std::int64_t estimated_row_count;
  google::protobuf::Timestamp expire_time;
  std::shared_ptr<arrow::Schema> schema;
  std::vector<StreamRange<std::shared_ptr<arrow::RecordBatch>>> readers;
};

GOOGLE_CLOUD_CPP_BIGQUERY_INLINE_NAMESPACE_END
}  // namespace google::cloud::bigquery_unified

#endif  // GOOGLE_CLOUD_CPP_BIGQUERY_GOOGLE_CLOUD_BIGQUERY_UNIFIED_READ_ARROW_RESPONSE_H
