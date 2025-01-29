# ~~~
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
# ~~~

include(CMakeDependentOption)
include(CreateBazelConfig)

set(GOOGLE_CLOUD_CPP_BIGQUERY_EXPERIMENTAL_LIBRARIES # cmake-format: sort
)

set(GOOGLE_CLOUD_CPP_BIGQUERY_TRANSITION_LIBRARIES # cmake-format: sort
)

set(GOOGLE_CLOUD_CPP_BIGQUERY_GA_LIBRARIES # cmake-format: sort
                                           "bigquery_unified")

function (export_libraries_bzl)
    foreach (stage IN ITEMS EXPERIMENTAL TRANSITION GA)
        set(var "GOOGLE_CLOUD_CPP_BIGQUERY_${stage}_LIBRARIES")
        list(SORT ${var})
    endforeach ()

    export_list_to_bazel(
        "libraries.bzl"
        YEAR
        2024
        GOOGLE_CLOUD_CPP_BIGQUERY_EXPERIMENTAL_LIBRARIES
        GOOGLE_CLOUD_CPP_BIGQUERY_TRANSITION_LIBRARIES
        GOOGLE_CLOUD_CPP_BIGQUERY_GA_LIBRARIES)
endfunction ()
export_libraries_bzl()
