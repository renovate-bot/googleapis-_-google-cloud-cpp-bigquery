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

#ifndef GOOGLE_CLOUD_CPP_BIGQUERY_GOOGLE_CLOUD_BIGQUERY_UNIFIED_INTERNAL_ARROW_READER_H
#define GOOGLE_CLOUD_CPP_BIGQUERY_GOOGLE_CLOUD_BIGQUERY_UNIFIED_INTERNAL_ARROW_READER_H

#include "google/cloud/bigquery_unified/version.h"
#include "google/cloud/options.h"
#include "google/cloud/stream_range.h"
#include <google/cloud/bigquery/storage/v1/storage.grpc.pb.h>
#include <arrow/ipc/dictionary.h>
#include <arrow/record_batch.h>

namespace google::cloud::bigquery_unified_internal {
GOOGLE_CLOUD_CPP_BIGQUERY_INLINE_NAMESPACE_BEGIN

StatusOr<std::pair<std::shared_ptr<arrow::Schema>,
                   std::shared_ptr<arrow::ipc::DictionaryMemo>>>
GetArrowSchema(
    ::google::cloud::bigquery::storage::v1::ArrowSchema const& schema_in);

StatusOr<std::shared_ptr<arrow::RecordBatch>> GetArrowRecordBatch(
    ::google::cloud::bigquery::storage::v1::ArrowRecordBatch const&
        record_batch_in,
    std::shared_ptr<arrow::Schema> schema,
    std::shared_ptr<arrow::ipc::DictionaryMemo> const& dictionary);

class ArrowRecordBatchReader {
 public:
  ArrowRecordBatchReader(
      std::string stream_name, std::shared_ptr<arrow::Schema> schema,
      std::shared_ptr<arrow::ipc::DictionaryMemo> dictionary,
      std::function<std::shared_ptr<google::cloud::StreamRange<
          google::cloud::bigquery::storage::v1::ReadRowsResponse>>(
          google::cloud::bigquery::storage::v1::ReadRowsRequest const&)>
          factory);

  absl::variant<Status, std::shared_ptr<arrow::RecordBatch>> operator()(
      Options const&);

 private:
  std::string stream_name_;
  std::shared_ptr<arrow::Schema> schema_;
  std::shared_ptr<arrow::ipc::DictionaryMemo> dictionary_;
  google::cloud::bigquery::storage::v1::ReadRowsRequest request_;
  std::function<std::shared_ptr<google::cloud::StreamRange<
      google::cloud::bigquery::storage::v1::ReadRowsResponse>>(
      google::cloud::bigquery::storage::v1::ReadRowsRequest const&)>
      factory_;
  bool begun_ = false;
  std::shared_ptr<google::cloud::StreamRange<
      google::cloud::bigquery::storage::v1::ReadRowsResponse>>
      read_rows_stream_;
  google::cloud::StreamRange<
      google::cloud::bigquery::storage::v1::ReadRowsResponse>::iterator iter_;
  google::cloud::bigquery::storage::v1::ReadRowsResponse current_rows_;
};

GOOGLE_CLOUD_CPP_BIGQUERY_INLINE_NAMESPACE_END
}  // namespace google::cloud::bigquery_unified_internal

#endif  // GOOGLE_CLOUD_CPP_BIGQUERY_GOOGLE_CLOUD_BIGQUERY_UNIFIED_INTERNAL_ARROW_READER_H
