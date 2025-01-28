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

/**
 *  Contains data and metadata from a successful `ReadArrow` call.
 */
struct ReadArrowResponse {
  /// An estimate on the number of bytes this session will scan when
  /// all streams are completely consumed. This estimate is based on
  /// metadata from the table which might be incomplete or stale.
  std::int64_t estimated_total_bytes_scanned;

  /// A pre-projected estimate of the total physical size of files
  /// (in bytes) that this session will scan when all streams are consumed. This
  /// estimate is independent of the selected columns and can be based on
  /// incomplete or stale metadata from the table.  This field is only set for
  /// BigLake tables.
  std::int64_t estimated_total_physical_file_size;

  /// An estimate on the number of rows present in this session's
  /// streams. This estimate is based on metadata from the table which might be
  /// incomplete or stale.
  std::int64_t estimated_row_count;

  /// Time at which the session becomes invalid. After this time,
  /// subsequent requests to read from this session will return errors. The
  /// expire_time is automatically assigned and currently cannot be specified or
  /// updated.
  google::protobuf::Timestamp expire_time;

  /// The schema for the read. If read_options.selected_fields is set, the
  /// schema may be different from the table schema as it will only contain
  /// the selected fields.
  std::shared_ptr<arrow::Schema> schema;

  /// Contains one or more StreamRanges from which the data can be read.
  std::vector<StreamRange<std::shared_ptr<arrow::RecordBatch>>> readers;
};

GOOGLE_CLOUD_CPP_BIGQUERY_INLINE_NAMESPACE_END
}  // namespace google::cloud::bigquery_unified

#endif  // GOOGLE_CLOUD_CPP_BIGQUERY_GOOGLE_CLOUD_BIGQUERY_UNIFIED_READ_ARROW_RESPONSE_H
