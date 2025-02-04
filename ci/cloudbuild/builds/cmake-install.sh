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

set -euo pipefail

source "$(dirname "$0")/../../lib/init.sh"
source module ci/cloudbuild/builds/lib/cmake.sh
source module ci/cloudbuild/builds/lib/ctest.sh
source module ci/cloudbuild/builds/lib/features.sh
source module ci/cloudbuild/builds/lib/quickstart.sh
source module ci/lib/io.sh

export CC=clang
export CXX=clang++

mapfile -t cmake_args < <(cmake::common_args)
INSTALL_PREFIX="$(mktemp -d)"
readonly INSTALL_PREFIX
read -r ENABLED_FEATURES < <(features::list_full_cmake)
readonly ENABLED_FEATURES

# Compiles and installs all libraries and headers.
io::run cmake "${cmake_args[@]}" \
  -DCMAKE_INSTALL_PREFIX="${INSTALL_PREFIX}" \
  -DCMAKE_INSTALL_MESSAGE=NEVER \
  -DGOOGLE_CLOUD_CPP_ENABLE_CLANG_ABI_COMPAT_17=ON \
  -DBUILD_TESTING=OFF \
  -DGOOGLE_CLOUD_CPP_ENABLE_EXAMPLES=OFF \
  -DGOOGLE_CLOUD_CPP_ENABLE="${ENABLED_FEATURES}" \
  -DGOOGLE_CLOUD_CPP_BIGQUERY_ENABLE_OPENTELEMETRY=ON
io::run cmake --build cmake-out
io::run cmake --install cmake-out --component google_cloud_cpp_bigquery_development

io::log_h2 "Verifying installed directories"
# Finds all the installed leaf directories (i.e., directories with exactly two
# links: . and ..). We only look at leaf directories, because obviously all
# parent directories must also exist.
mapfile -t actual_dirs < <(env -C "${INSTALL_PREFIX}" find -type d)
mapfile -t expected_dirs < <(cat ci/etc/expected_install_directories)

# Fails on any difference between the expected vs actually installed dirs.
discrepancies="$(comm -3 \
  <(printf "%s\n" "${expected_dirs[@]}" | sort) \
  <(printf "%s\n" "${actual_dirs[@]}" | sort))"
if [[ -n "${discrepancies}" ]]; then
  io::log "Found install discrepancies: expected vs actual"
  echo "${discrepancies}"
  exit 1
fi

io::log_h2 "Validating installed pkg-config files"
export PKG_CONFIG_PATH="${INSTALL_PREFIX}/lib64/pkgconfig:${PKG_CONFIG_PATH:-}"
while IFS= read -r -d '' pc; do
  # Ignores the warning noise from system .pc files, but we redo the validation
  # with warnings enabled if the validation of our .pc fails.
  if ! pkg-config --validate "${pc}" >/dev/null 2>&1; then
    echo "Invalid ${pc}"
    pkg-config --validate "${pc}" || true
    echo "---"
    cat "${pc}"
    echo "---"
    exit 1
  fi
done < <(find "${INSTALL_PREFIX}" -name '*.pc' -print0)

io::log_h2 "Validating installed file extensions"
# All installed libraries have the same version, so pick one.
version=$(pkg-config google_cloud_cpp_bigquery_bigquery_unified --modversion)
version_major=$(cut -d. -f1 <<<"${version}")
allow_re="\.(h|inc|proto|cmake|pc|a|so|so\.${version}|so\.${version_major})\$"
while IFS= read -r -d '' f; do
  if ! grep -qP "${allow_re}" <<<"${f}"; then
    echo "File with unexpected suffix installed: ${f}"
    exit 1
  fi
done < <(find "${INSTALL_PREFIX}" -type f -print0)

mapfile -t ctest_args < <(ctest::common_args)
out_dir="cmake-out/verify_current_targets-out"
rm -f "${out_dir}/CMakeCache.txt"
io::log_h2 "Verifying CMake targets in repo root: ci/verify_current_targets"
io::run cmake --log-level WARNING -GNinja -DCMAKE_PREFIX_PATH="${INSTALL_PREFIX}" \
  -S ci/verify_current_targets -B "${out_dir}" -Wno-dev
io::run cmake --build "${out_dir}"
env -C "${out_dir}" ctest "${ctest_args[@]}"

io::log_h1 "Build and run quickstarts"
# Tests the installed artifacts by building and running the quickstarts.
# shellcheck disable=SC2046
feature_list="$(printf "%s" $(features::libraries))"
io::run cmake -G Ninja \
  -S "${PROJECT_ROOT}/ci/verify_quickstart" \
  -B "${PROJECT_ROOT}/cmake-out/quickstart" \
  "-DCMAKE_PREFIX_PATH=${INSTALL_PREFIX}" \
  "-DFEATURES=${feature_list}"
io::run cmake --build "${PROJECT_ROOT}/cmake-out/quickstart"

io::log_h2 "Delete installed artifacts and run compiled quickstarts"
# Deletes all the installed artifacts, and installs only the runtime components
# to verify that we can still execute the compiled quickstart programs.
rm -rf "${INSTALL_PREFIX:?}"/{include,lib64}
io::run cmake --install cmake-out --component google_cloud_cpp_bigquery_bigquery_unified_runtime
quickstart::run_cmake_and_make "${INSTALL_PREFIX}"

# Be a little more explicit because we often run this manually
io::log_h1 "SUCCESS"
