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

find_package(google_cloud_cpp_bigquery REQUIRED)
find_package(google_cloud_cpp_bigquerycontrol REQUIRED)
find_package(Arrow REQUIRED)

add_library(
    google_cloud_cpp_bigquery_unified # cmake-format: sort
    client.cc
    client.h
    connection.h
    internal/version_info.h
    read_arrow_response.h
    version.cc
    version.h)

target_include_directories(
    google_cloud_cpp_bigquery_unified
    PUBLIC $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}>
           $<BUILD_INTERFACE:${PROJECT_BINARY_DIR}>
           $<INSTALL_INTERFACE:include>)
target_link_libraries(
    google_cloud_cpp_bigquery_unified
    PUBLIC google-cloud-cpp::bigquery
           google-cloud-cpp::experimental-bigquerycontrol
           google-cloud-cpp::grpc_utils
           google-cloud-cpp::common
           Arrow::arrow_shared)
set_target_properties(
    google_cloud_cpp_bigquery_unified
    PROPERTIES EXPORT_NAME "google-cloud-cpp-bigquery::bigquery_unified"
               VERSION "${PROJECT_VERSION}"
               SOVERSION "${PROJECT_VERSION_MAJOR}")
target_compile_options(google_cloud_cpp_bigquery_unified
                       PUBLIC ${GOOGLE_CLOUD_CPP_EXCEPTIONS_FLAG})
