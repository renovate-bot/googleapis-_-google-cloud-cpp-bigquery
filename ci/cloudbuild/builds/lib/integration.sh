#!/bin/bash
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

# This file defines helper functions for running integration tests.

# Make our include guard clean against set -o nounset.
test -n "${CI_CLOUDBUILD_BUILDS_LIB_INTEGRATION_SH__:-}" || declare -i CI_CLOUDBUILD_BUILDS_LIB_INTEGRATION_SH__=0
if ((CI_CLOUDBUILD_BUILDS_LIB_INTEGRATION_SH__++ != 0)); then
  return 0
fi # include guard

source module ci/etc/integration-tests-config.sh
source module ci/cloudbuild/builds/lib/ctest.sh
source module ci/cloudbuild/builds/lib/git.sh
source module ci/lib/io.sh

# Some of the tests will need a valid roots.pem file.
rm -f /dev/shm/roots.pem
ci/retry-command.sh 3 120 curl -fsSL -o /dev/shm/roots.pem https://pki.google.com/roots.pem

# Outputs a list of Bazel arguments that should be used when running
# integration tests. These do not include the common `bazel::common_args`.
#
# Example usage:
#
#   mapfile -t args < <(bazel::common_args)
#   mapfile -t integration_args < <(integration::bazel_args)
#   integration::bazel_with_emulators test "${args[@]}" "${integration_args[@]}"
#
function integration::bazel_args() {
  declare -a args

  # Integration tests are inherently flaky. Make up to three attempts to get the
  # test passing.
  args+=(--flaky_test_attempts=3)

  args+=(
    # Common settings
    "--test_env=GOOGLE_CLOUD_PROJECT=${GOOGLE_CLOUD_PROJECT}"
    "--test_env=GOOGLE_CLOUD_CPP_TEST_REGION=${GOOGLE_CLOUD_CPP_TEST_REGION}"
    "--test_env=GOOGLE_CLOUD_CPP_NON_US_TEST_REGION=${GOOGLE_CLOUD_CPP_NON_US_TEST_REGION}"
    "--test_env=GOOGLE_CLOUD_CPP_TEST_ZONE=${GOOGLE_CLOUD_CPP_TEST_ZONE}"
    "--test_env=GOOGLE_CLOUD_CPP_TEST_ORGANIZATION=${GOOGLE_CLOUD_CPP_TEST_ORGANIZATION}"
    "--test_env=GOOGLE_CLOUD_CPP_TEST_SERVICE_ACCOUNT_KEYFILE=${GOOGLE_CLOUD_CPP_TEST_SERVICE_ACCOUNT_KEYFILE}"
    "--test_env=GOOGLE_CLOUD_CPP_AUTO_RUN_EXAMPLES=${GOOGLE_CLOUD_CPP_AUTO_RUN_EXAMPLES}"
    "--test_env=GOOGLE_CLOUD_CPP_EXPERIMENTAL_LOG_CONFIG=${GOOGLE_CLOUD_CPP_EXPERIMENTAL_LOG_CONFIG}"
    "--test_env=GOOGLE_CLOUD_CPP_ENABLE_TRACING=${GOOGLE_CLOUD_CPP_ENABLE_TRACING}"
    "--test_env=GOOGLE_CLOUD_CPP_TRACING_OPTIONS=${GOOGLE_CLOUD_CPP_TRACING_OPTIONS}"
    "--test_env=HOME=${HOME}"
  )

  printf "%s\n" "${args[@]}"
}

# Runs integration tests with bazel using emulators when possible. This
# function requires a first argument that is the bazel verb to do, valid verbs
# are "test" and "coverage". Additional arguments are assumed to be bazel args.
# Almost certainly the caller should pass the arguments returned from the
# `integration::bazel_args` function defined above.
#
# Example usage:
#
#   mapfile -t args < <(bazel::common_args)
#   mapfile -t integration_args < <(integration::bazel_args)
#   integration::bazel_with_emulators test "${args[@]}" "${integration_args[@]}"
#
function integration::bazel_with_emulators() {
  readonly EMULATOR_SCRIPT="run_integration_tests_emulator_bazel.sh"
  if [[ $# == 0 ]]; then
    io::log_red "error: bazel verb required"
    return 1
  fi

  local verb="$1"
  local args=("${@:2}")

  production_integration_tests=(
    "google/cloud/bigquery_unified/..."
  )

  production_tests_tag_filters="integration-test,-ud-only"
  if echo "${args[@]}" | grep -w -q -- "--config=msan"; then
    production_tests_tag_filters="integration-test,-no-msan,-ud-only"
  fi

  if [[ "${BAZEL_TARGETS[*]}" != "..." ]]; then
    io::log_h2 "Skipping some integration tests because BAZEL_TARGETS is not the default"
    return 0
  fi

  io::log_h2 "Running integration tests that require production access"
  bazel "${verb}" "${args[@]}" \
    --test_tag_filters="${production_tests_tag_filters}" \
    "${production_integration_tests[@]}"
}

# Runs integration tests with CTest using emulators. This function requires a
# first argument that is the "cmake-out" directory where the tests live.
#
# Example usage:
#
#   integration::ctest_with_emulators "cmake-out"
#
function integration::ctest_with_emulators() {
  readonly EMULATOR_SCRIPT="run_integration_tests_emulator_cmake.sh"
  if [[ $# == 0 ]]; then
    io::log_red "error: build output directory required"
    return 1
  fi

  local cmake_out="$1"
  mapfile -t ctest_args < <(ctest::common_args)
  # Integration tests are inherently flaky. Make up to three attempts to get the
  # test passing.
  ctest_args+=(--repeat until-pass:3)
}
