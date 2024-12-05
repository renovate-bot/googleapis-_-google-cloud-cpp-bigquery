#!/usr/bin/env bash
#
# Copyright 2024 Google LLC
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#     https://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.

# Make our include guard clean against set -o nounset.
test -n "${CI_ETC_INTEGRATION_TESTS_CONFIG_SH__:-}" || declare -i CI_ETC_INTEGRATION_TESTS_CONFIG_SH__=0
if ((CI_ETC_INTEGRATION_TESTS_CONFIG_SH__++ != 0)); then
  return 0
fi # include guard

# The name of the project used to run the integration tests and examples.
export GOOGLE_CLOUD_PROJECT="cloud-cpp-testing-resources"
# Some services seemingly require project numbers. If you ever need to generate
# this again use:
#   gcloud projects describe "${GOOGLE_CLOUD_PROJECT}"
export GOOGLE_CLOUD_CPP_TEST_PROJECT_NUMBER="936212892354"
# Some quickstarts require a x-goog-user-project header, either when using
# our own user account in local builds, or when using the GCB service
# account
export GOOGLE_CLOUD_CPP_USER_PROJECT="${GOOGLE_CLOUD_PROJECT}"
# Many tests and quickstarts need a location, this is typically a region.
export GOOGLE_CLOUD_CPP_TEST_REGION="us-central1"
# Some quickstart programs require a zone.
export GOOGLE_CLOUD_CPP_TEST_ZONE="us-central1-a"
# Some tests and quickstarts benefit from a region outside the US.
export GOOGLE_CLOUD_CPP_NON_US_TEST_REGION="asia-southeast1"
# Some tests and quickstarts need to specify a location as "global".
export GOOGLE_CLOUD_CPP_TEST_GLOBAL="global"
# Some quickstart programs require an organization.
export GOOGLE_CLOUD_CPP_TEST_ORGANIZATION="1006341795026"

# This file contains an invalidated service account key.  That is, the file is
# in the right format for a service account, but it is not associated with a
# valid service account or service account key.
export GOOGLE_CLOUD_CPP_TEST_SERVICE_ACCOUNT_KEYFILE="${PROJECT_ROOT}/ci/etc/invalidated-keyfile.json"

# Enable the self-test for the sample programs. Normally the example drivers
# require the name of the example to run as a command-line argument, with this
# environment variable the sample drivers run all the examples.
export GOOGLE_CLOUD_CPP_AUTO_RUN_EXAMPLES="yes"

# A number of options to improve logging during the CI builds. They are useful
# when troubleshooting problems.
export GOOGLE_CLOUD_CPP_EXPERIMENTAL_LOG_CONFIG="lastN,1024,WARNING"
export GOOGLE_CLOUD_CPP_ENABLE_TRACING="rpc,rpc-streams"
export GOOGLE_CLOUD_CPP_TRACING_OPTIONS="single_line_mode=off,truncate_string_field_longer_than=512"

# Cloud BigQuery configuration parameters
export GOOGLE_CLOUD_CPP_BIGQUERY_TEST_QUICKSTART_TABLE="projects/bigquery-public-data/datasets/usa_names/tables/usa_1910_current"


