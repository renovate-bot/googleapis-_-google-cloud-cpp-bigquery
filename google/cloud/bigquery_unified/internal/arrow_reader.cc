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

#include "google/cloud/bigquery_unified/internal/arrow_reader.h"
#include "google/cloud/internal/make_status.h"
#include <arrow/api.h>
#include <arrow/array/data.h>
#include <arrow/io/memory.h>
#include <arrow/ipc/api.h>
#include <arrow/status.h>

namespace google::cloud::bigquery_unified_internal {
GOOGLE_CLOUD_CPP_BIGQUERY_INLINE_NAMESPACE_BEGIN

StatusOr<std::pair<std::shared_ptr<arrow::Schema>,
                   std::shared_ptr<arrow::ipc::DictionaryMemo>>>
GetArrowSchema(
    ::google::cloud::bigquery::storage::v1::ArrowSchema const& schema_in) {
  std::shared_ptr<arrow::Buffer> buffer =
      std::make_shared<arrow::Buffer>(schema_in.serialized_schema());
  arrow::io::BufferReader buffer_reader(buffer);
  auto dictionary = std::make_shared<arrow::ipc::DictionaryMemo>();
  auto result = arrow::ipc::ReadSchema(&buffer_reader, dictionary.get());
  if (!result.ok()) {
    return google::cloud::internal::InternalError("Unable to parse schema",
                                                  GCP_ERROR_INFO());
  }
  std::shared_ptr<arrow::Schema> schema = result.ValueOrDie();
  return std::make_pair(std::move(schema), std::move(dictionary));
}

StatusOr<std::shared_ptr<arrow::RecordBatch>> GetArrowRecordBatch(
    ::google::cloud::bigquery::storage::v1::ArrowRecordBatch const&
        record_batch_in,
    std::shared_ptr<arrow::Schema> schema,
    std::shared_ptr<arrow::ipc::DictionaryMemo> const& dictionary) {
  std::shared_ptr<arrow::Buffer> buffer = std::make_shared<arrow::Buffer>(
      record_batch_in.serialized_record_batch());
  arrow::io::BufferReader buffer_reader(buffer);
  arrow::ipc::IpcReadOptions read_options;
  auto result = arrow::ipc::ReadRecordBatch(schema, dictionary.get(),
                                            read_options, &buffer_reader);
  if (!result.ok()) {
    return google::cloud::internal::InternalError(
        "Unable to parse record batch", GCP_ERROR_INFO());
  }
  std::shared_ptr<arrow::RecordBatch> record_batch = result.ValueOrDie();
  return record_batch;
}

ArrowRecordBatchReader::ArrowRecordBatchReader(
    std::string stream_name, std::shared_ptr<arrow::Schema> schema,
    std::shared_ptr<arrow::ipc::DictionaryMemo> dictionary,
    std::function<std::shared_ptr<google::cloud::StreamRange<
        google::cloud::bigquery::storage::v1::ReadRowsResponse>>(
        google::cloud::bigquery::storage::v1::ReadRowsRequest const&)>
        factory)
    : stream_name_(stream_name),
      schema_(std::move(schema)),
      dictionary_(std::move(dictionary)),
      factory_(std::move(factory)) {}

absl::variant<Status, std::shared_ptr<arrow::RecordBatch>>
ArrowRecordBatchReader::operator()(Options const&) {
  // We could possibly remove this if block if we initialize read_row_stream_
  // and iter_ during construction. However, that would mean that we make the
  // rpc call on construction rather than delaying the rpc call to the first
  // invocation of operator(). But, if StreamRange calls operator() during its
  // construction, then it's a moot point. Further investigation is required.
  if (!begun_) {
    begun_ = true;
    request_.set_read_stream(stream_name_);
    read_rows_stream_ = factory_(request_);
    iter_ = read_rows_stream_->begin();
  } else {
    ++iter_;
  }

  if (iter_ == read_rows_stream_->end()) {
    return Status{};
  }

  StatusOr<google::cloud::bigquery::storage::v1::ReadRowsResponse> e = *iter_;
  if (!e) return std::move(e).status();
  current_rows_ = *std::move(e);

  StatusOr<std::shared_ptr<arrow::RecordBatch>> record_batch =
      GetArrowRecordBatch(current_rows_.arrow_record_batch(), schema_,
                          dictionary_);
  if (!record_batch) return std::move(record_batch).status();
  return *record_batch;
}

GOOGLE_CLOUD_CPP_BIGQUERY_INLINE_NAMESPACE_END
}  // namespace google::cloud::bigquery_unified_internal
