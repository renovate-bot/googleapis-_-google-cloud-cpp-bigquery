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

#include "google/cloud/bigquery_unified/testing_util/opentelemetry_matchers.h"
#include "google/cloud/bigquery_unified/internal/tracing_connection.h"
#include "google/cloud/bigquery_unified/mocks/mock_connection.h"
#include "google/cloud/bigquery_unified/testing_util/status_matchers.h"
#include <gmock/gmock.h>
#include "google/cloud/internal/make_status.h"

namespace google::cloud::bigquery_unified_internal {
GOOGLE_CLOUD_CPP_BIGQUERY_INLINE_NAMESPACE_BEGIN
namespace {

#ifdef GOOGLE_CLOUD_CPP_BIGQUERY_HAVE_OPENTELEMETRY
using ::google::cloud::testing_util::DisableTracing;
using ::google::cloud::testing_util::EnableTracing;
using ::google::cloud::testing_util::OTelAttribute;
using ::google::cloud::testing_util::OTelContextCaptured;
using ::google::cloud::testing_util::InstallSpanCatcher;
using ::google::cloud::testing_util::SpanHasAttributes;
using ::google::cloud::testing_util::SpanHasInstrumentationScope;
using ::google::cloud::testing_util::SpanKindIsClient;
using ::google::cloud::testing_util::SpanNamed;
using ::google::cloud::testing_util::SpanWithStatus;
using ::google::cloud::testing_util::ThereIsAnActiveSpan;
using ::testing::AllOf;
using ::testing::ByMove;
using ::testing::ElementsAre;
using ::testing::IsEmpty;
using ::testing::Not;
using ::google::cloud::bigquery_unified::testing_util::StatusIs;
using ::testing::Return;

auto constexpr kErrorCode = "ABORTED";

TEST(TracingConnectionTest, GetJob) {
  auto span_catcher = InstallSpanCatcher();

  auto mock = std::make_shared<bigquery_unified::MockConnection>();
  EXPECT_CALL(*mock, GetJob).WillOnce([](google::cloud::bigquery::v2::GetJobRequest const&,
                    Options) {
    EXPECT_TRUE(ThereIsAnActiveSpan());
    return internal::AbortedError("fail");
  });

  auto under_test = TracingConnection(mock);
  google::cloud::bigquery::v2::GetJobRequest request;
  auto result = under_test.GetJob(request, Options{});
  EXPECT_THAT(result, StatusIs(StatusCode::kAborted));

  auto spans = span_catcher->GetSpans();
  EXPECT_THAT(
      spans,
      ElementsAre(AllOf(
          SpanHasInstrumentationScope(), SpanKindIsClient(),
          SpanNamed(
              "bigquery_unified::Connection::GetJob"),
          SpanWithStatus(opentelemetry::trace::StatusCode::kError, "fail"),
          SpanHasAttributes(
              OTelAttribute<std::string>("gl-cpp.status_code", kErrorCode)))));
}

#endif  // GOOGLE_CLOUD_CPP_BIGQUERY_HAVE_OPENTELEMETRY

}  // namespace
GOOGLE_CLOUD_CPP_BIGQUERY_INLINE_NAMESPACE_END
}  // namespace google::cloud::bigquery_unified_internal
