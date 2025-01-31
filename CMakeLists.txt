# ~~~
# Copyright 2024 Google LLC
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#     http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.
# ~~~

cmake_minimum_required(VERSION 3.16...3.31)
set(CMAKE_CXX_STANDARD 17)

# Define the project name and where to report bugs.
set(PACKAGE_BUGREPORT
    "https://github.com/googleapis/google-cloud-cpp-bigquery/issues")
project(
    google-cloud-cpp-bigquery
    VERSION 0.0.0
    LANGUAGES CXX)
set(PROJECT_VERSION_PRE_RELEASE "")

list(APPEND CMAKE_MODULE_PATH ${PROJECT_SOURCE_DIR}/cmake)

include(GoogleCloudCppFeatures)

include(CMakeDependentOption)
cmake_dependent_option(
    GOOGLE_CLOUD_CPP_BIGQUERY_WITH_MOCKS
    [==[Build the google-cloud-cpp-bigquery mocking libraries.

google-cloud-cpp-bigquery offers mocking libraries with mock classes, to
facilitate unit testing of Cloud C++ clients. Consumers of this library that do
not use the provided mocks to test code involving the Cloud C++ clients may wish
to turn this flag off.]==]
    ON
    "NOT BUILD_TESTING"
    ON)
mark_as_advanced(GOOGLE_CLOUD_CPP_BIGQUERY_WITH_MOCKS)

include(CTest)

add_subdirectory(google/cloud/bigquery_unified)
