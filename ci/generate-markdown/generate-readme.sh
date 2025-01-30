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

set -euo pipefail

if ! type sponge >/dev/null 2>&1; then
  echo "This script requires sponge(1) to produce its output."
  echo "Please install this tool and try again."
  exit 1
fi

file="README.md"
(
  sed '/<!-- inject-quickstart-start -->/q' "${file}"
  echo '```cc'
  # Dumps the contents of quickstart.cc starting at the first #include, so we
  # skip the license header comment.
  sed -n -e '/END .*quickstart/,$d' -e '\:^//!:d' -e '/^#/,$p' "google/cloud/bigquery_unified/quickstart/quickstart.cc"
  echo '```'
  sed -n '/<!-- inject-quickstart-end -->/,$p' "${file}"
) | sponge "${file}"
