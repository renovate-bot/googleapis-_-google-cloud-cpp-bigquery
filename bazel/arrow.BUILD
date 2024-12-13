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

package(default_visibility = ["//visibility:public"])

# Our CI builds expect the libraries to be in
# /usr/local/lib64 and the headers in /usr/include/arrow.
# You may need to create a symbolic link on other systems, e.g.:
# mkdir -p /usr/local/lib64
# ln -s $(find /usr -name libarrow.a) /usr/local/lib64/libarrow.a
# ln -s $(find /usr -name libarrow.so) /usr/local/lib64/libarrow.so
# ln -s $(find /usr/include -name arrow -type d) /usr/local/include/arrow
cc_import(
    name = "libarrow",
    hdrs = glob(["include/arrow/*.h"]),
    interface_library = "lib64/libarrow.so",
    static_library = "lib64/libarrow.a",
    system_provided = True,
    visibility = ["//visibility:public"],
)
