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

#ifndef GOOGLE_CLOUD_CPP_BIGQUERY_GOOGLE_CLOUD_BIGQUERY_UNIFIED_READ_OPTIONS_H
#define GOOGLE_CLOUD_CPP_BIGQUERY_GOOGLE_CLOUD_BIGQUERY_UNIFIED_READ_OPTIONS_H

#include "google/cloud/bigquery_unified/version.h"
#include "google/cloud/options.h"
#include <memory>

namespace google::cloud::bigquery_unified {
GOOGLE_CLOUD_CPP_BIGQUERY_INLINE_NAMESPACE_BEGIN

/**
 *  Use with `google::cloud::Options` to configure the maximum read streams.
 *
 *  Max initial number of streams. If unset or zero, the server will
 *  provide a value of streams as to produce reasonable throughput. Must be
 *  non-negative. The number of streams may be lower than the requested number,
 *  depending on the amount parallelism that is reasonable for the table.
 *  There is a default system max limit of 1,000.
 *
 *  This must be greater than or equal to preferred_min_stream_count.
 *  Typically, clients should either leave this unset to let the system
 *  determine an upper bound OR set this a size for the maximum "units of work"
 *  it can gracefully handle.
 *
 *  @ingroup google-cloud-bigquery-unified-options
 */
struct MaxReadStreamsOption {
  using Type = int32_t;
};

/**
 *  Use with `google::cloud::Options` to configure the preferred minimum number
 *  of read streams.
 *
 *  The minimum preferred stream count. This parameter can be used to inform
 *  the service that there is a desired lower bound on the number of streams.
 *  This is typically a target parallelism of the client (e.g. a Spark
 *  cluster with N-workers would set this to a low multiple of N to ensure
 *  good cluster utilization).
 *
 *  The system will make a best effort to provide at least this number of
 *  streams, but in some cases might provide less.
 *
 *  @ingroup google-cloud-bigquery-unified-options
 */
struct PreferredMinimumReadStreamsOption {
  using Type = int32_t;
};

using BigQueryReadOptionList =
    OptionList<MaxReadStreamsOption, PreferredMinimumReadStreamsOption>;

GOOGLE_CLOUD_CPP_BIGQUERY_INLINE_NAMESPACE_END
}  // namespace google::cloud::bigquery_unified

#endif  // GOOGLE_CLOUD_CPP_BIGQUERY_GOOGLE_CLOUD_BIGQUERY_UNIFIED_READ_OPTIONS_H
