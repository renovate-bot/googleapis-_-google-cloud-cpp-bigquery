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

include(FindGMockWithTargets)
include(GoogleCloudCppCommon)
include(CreateBazelConfig)

find_package(google_cloud_cpp_bigquery REQUIRED)
find_package(google_cloud_cpp_bigquerycontrol REQUIRED)
find_package(Arrow REQUIRED)

add_library(
    google_cloud_cpp_bigquery_bigquery_unified # cmake-format: sort
    client.cc
    client.h
    connection.cc
    connection.h
    internal/version_info.h
    read_arrow_response.h
    version.cc
    version.h)

target_include_directories(
    google_cloud_cpp_bigquery_bigquery_unified
    PUBLIC $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}>
           $<BUILD_INTERFACE:${PROJECT_BINARY_DIR}>
           $<INSTALL_INTERFACE:include>)
target_link_libraries(
    google_cloud_cpp_bigquery_bigquery_unified
    PUBLIC google-cloud-cpp::bigquery
           google-cloud-cpp::experimental-bigquerycontrol
           google-cloud-cpp::grpc_utils
           google-cloud-cpp::common
           Arrow::arrow_static)
set_target_properties(
    google_cloud_cpp_bigquery_bigquery_unified
    PROPERTIES EXPORT_NAME "google-cloud-cpp-bigquery::bigquery_unified"
               VERSION "${PROJECT_VERSION}"
               SOVERSION "${PROJECT_VERSION_MAJOR}")
target_compile_options(google_cloud_cpp_bigquery_bigquery_unified
                       PUBLIC ${GOOGLE_CLOUD_CPP_EXCEPTIONS_FLAG})
add_library(google-cloud-cpp-bigquery::bigquery_unified ALIAS
            google_cloud_cpp_bigquery_bigquery_unified)

create_bazel_config(google_cloud_cpp_bigquery_bigquery_unified YEAR "2024")

# Get the destination directories based on the GNU recommendations.
include(GNUInstallDirs)

# Export the CMake targets to make it easy to create configuration files.
install(
    EXPORT google_cloud_cpp_bigquery_bigquery_unified-targets
    DESTINATION
        "${CMAKE_INSTALL_LIBDIR}/cmake/google_cloud_cpp_bigquery_bigquery_unified"
    COMPONENT google_cloud_cpp_bigquery_development)

# Install the libraries and headers in the locations determined by
# GNUInstallDirs
install(
    TARGETS google_cloud_cpp_bigquery_bigquery_unified
    EXPORT google_cloud_cpp_bigquery_bigquery_unified-targets
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
            COMPONENT google_cloud_cpp_bigquery_runtime
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
            COMPONENT google_cloud_cpp_bigquery_runtime
            NAMELINK_COMPONENT google_cloud_cpp_bigquery_development
    ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
            COMPONENT google_cloud_cpp_bigquery_development)

google_cloud_cpp_bigquery_install_headers(
    google_cloud_cpp_bigquery_bigquery_unified
    "include/google/cloud/bigquery_unified")

google_cloud_cpp_bigquery_add_pkgconfig(
    bigquery_unified "The BigQuery Unified C++ Client Library"
    "Provides C++ APIs to use the BigQuery Unified API."
    google_cloud_cpp_bigquery google_cloud_cpp_bigquerycontrol arrow)

# Create and install the CMake configuration files.
include(CMakePackageConfigHelpers)
set(GOOGLE_CLOUD_CPP_CONFIG_LIBRARY
    "google_cloud_cpp_bigquery_bigquery_unified")
list(APPEND find_dependencies "find_dependency(google_cloud_cpp_bigquery)")
list(APPEND find_dependencies
     "find_dependency(google_cloud_cpp_bigquerycontrol)")
list(APPEND find_dependencies "find_dependency(Arrow)")
string(JOIN "\n" GOOGLE_CLOUD_CPP_ADDITIONAL_FIND_DEPENDENCIES
       ${find_dependencies})
configure_file("${PROJECT_SOURCE_DIR}/cmake/templates/config.cmake.in"
               "google_cloud_cpp_bigquery_bigquery_unified-config.cmake" @ONLY)
write_basic_package_version_file(
    "google_cloud_cpp_bigquery_bigquery_unified-config-version.cmake"
    VERSION ${PROJECT_VERSION}
    COMPATIBILITY ExactVersion)

install(
    FILES
        "${CMAKE_CURRENT_BINARY_DIR}/google_cloud_cpp_bigquery_bigquery_unified-config.cmake"
        "${CMAKE_CURRENT_BINARY_DIR}/google_cloud_cpp_bigquery_bigquery_unified-config-version.cmake"
    DESTINATION
        "${CMAKE_INSTALL_LIBDIR}/cmake/google_cloud_cpp_bigquery_bigquery_unified"
    COMPONENT google_cloud_cpp_bigquery_development)

# TODO: Change this to use the MOCKS define
if (GOOGLE_CLOUD_CPP_BIGQUERY_WITH_MOCKS)
    add_library(google_cloud_cpp_bigquery_bigquery_unified_mocks INTERFACE)
    set(google_cloud_cpp_bigquery_bigquery_unified_mocks_hdrs
        # cmake-format: sort
        mocks/mock_connection.h)
    export_list_to_bazel(
        "google_cloud_cpp_bigquery_bigquery_unified_mocks.bzl"
        "google_cloud_cpp_bigquery_bigquery_unified_mocks_hdrs" YEAR "2024")

    # Add the prefix to each file name for cmake
    set(cmake_google_cloud_cpp_bigquery_bigquery_unified_mocks_hdrs)
    foreach (fname ${google_cloud_cpp_bigquery_bigquery_unified_mocks_hdrs})
        list(APPEND cmake_google_cloud_cpp_bigquery_bigquery_unified_mocks_hdrs
             "$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/${fname}>")
    endforeach ()
    target_sources(
        google_cloud_cpp_bigquery_bigquery_unified_mocks
        INTERFACE ${cmake_google_cloud_cpp_bigquery_bigquery_unified_mocks_hdrs}
    )
    target_link_libraries(
        google_cloud_cpp_bigquery_bigquery_unified_mocks
        INTERFACE google-cloud-cpp-bigquery::bigquery_unified GTest::gmock
                  GTest::gtest)
    set_target_properties(
        google_cloud_cpp_bigquery_bigquery_unified_mocks
        PROPERTIES EXPORT_NAME
                   google-cloud-cpp-bigquery::bigquery_unified_mocks)
    target_include_directories(
        google_cloud_cpp_bigquery_bigquery_unified_mocks
        INTERFACE $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}>
                  $<BUILD_INTERFACE:${PROJECT_BINARY_DIR}>
                  $<INSTALL_INTERFACE:include>)
    set_target_properties(
        google_cloud_cpp_bigquery_bigquery_unified_mocks
        PROPERTIES EXPORT_NAME
                   "google-cloud-cpp-bigquery::bigquery_unified_mocks")
    target_compile_options(google_cloud_cpp_bigquery_bigquery_unified_mocks
                           INTERFACE ${GOOGLE_CLOUD_CPP_EXCEPTIONS_FLAG})
    add_library(google-cloud-cpp-bigquery::bigquery_unified_mocks ALIAS
                google_cloud_cpp_bigquery_bigquery_unified_mocks)

    google_cloud_cpp_bigquery_install_mocks(bigquery_unified
                                            "Google Cloud BigQuery Unified")
endif ()

# Define the tests in a function so we have a new scope for variable names.
function (bigquery_unified_client_define_tests)
    # The tests require googletest to be installed. Force CMake to use the
    # config file for googletest (that is, the CMake file installed by
    # googletest itself), because the generic `FindGTest` module does not define
    # the GTest::gmock target, and the target names are also weird.
    find_package(GTest CONFIG REQUIRED)
    set(bigquery_unified_client_unit_tests # cmake-format: sort
                                           client_test.cc)

    # Export the list of unit tests to a .bzl file so we do not need to maintain
    # the list in two places.
    export_list_to_bazel("bigquery_unified_client_unit_tests.bzl"
                         "bigquery_unified_client_unit_tests" YEAR "2024")

    # Create a custom target so we can say "build all the tests"
    add_custom_target(bigquery_unified-client-tests)

    # Generate a target for each unit test.
    foreach (fname ${bigquery_unified_client_unit_tests})
        google_cloud_cpp_add_executable(target "bigquery_unified" "${fname}")
        target_link_libraries(
            ${target} PRIVATE google-cloud-cpp-bigquery::bigquery_unified
                              GTest::gmock_main GTest::gmock GTest::gtest)
        google_cloud_cpp_add_common_options(${target})
        add_test(NAME ${target} COMMAND ${target})
        add_dependencies(bigquery_unified-client-tests ${target})
    endforeach ()
endfunction ()

if (BUILD_TESTING)
    bigquery_unified_client_define_tests()
endif ()
