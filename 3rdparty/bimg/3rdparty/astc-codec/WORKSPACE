# Copyright 2018 Google LLC
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

load("@bazel_tools//tools/build_defs/repo:git.bzl", "git_repository")
load("@bazel_tools//tools/build_defs/repo:http.bzl", "http_archive")

git_repository(
    name = "gtest",
    remote = "https://github.com/google/googletest.git",
    commit = "ba96d0b1161f540656efdaed035b3c062b60e006",
)

http_archive(
    name = "honggfuzz",
    url = "https://github.com/google/honggfuzz/archive/1.7.zip",
    sha256 = "9d420326979fed4a065fa6176d5e09bd513cd2820fe216ae8b684aa6780d72b2",
    build_file = "//third_party:honggfuzz.BUILD",
    strip_prefix = "honggfuzz-1.7",
)

http_archive(
    name = "benchmark",
    url = "https://github.com/google/benchmark/archive/v1.4.1.zip",
    sha256 = "61ae07eb5d4a0b02753419eb17a82b7d322786bb36ab62bd3df331a4d47c00a7",
    strip_prefix = "benchmark-1.4.1",
)
