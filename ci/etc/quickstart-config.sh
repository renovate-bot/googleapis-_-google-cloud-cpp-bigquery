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
test -n "${CI_ETC_QUICKSTART_CONFIG_SH__:-}" || declare -i CI_ETC_QUICKSTART_CONFIG_SH__=0
if ((CI_ETC_QUICKSTART_CONFIG_SH__++ != 0)); then
  return 0
fi # include guard

source module ci/etc/integration-tests-config.sh

function quickstart::libraries() {
  echo "bigquery_unified"
}

function quickstart::arguments() {
  local -r library="$1"
  case "${library}" in
    "bigquery_unified")
      echo "${GOOGLE_CLOUD_PROJECT}"
      echo "${GOOGLE_CLOUD_CPP_BIGQUERY_TEST_QUICKSTART_TABLE}"
      return 0
      ;;
    *)
      echo "Unknown argument list for ${library}'s quickstart"
      ;;
  esac
  return 1
}
