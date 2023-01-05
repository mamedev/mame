// Copyright 2018 Google LLC
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     https://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

// ASTC fuzzing wrapper to help with fuzz testing.

#include "src/decoder/codec.h"

#include <benchmark/benchmark.h>

#include <vector>

extern "C" int LLVMFuzzerTestOneInput(const uint8_t* data, size_t size) {
  std::string error;
  std::unique_ptr<astc_codec::ASTCFile> file =
      astc_codec::ASTCFile::LoadFromMemory(reinterpret_cast<const char*>(data),
                                           size, &error);
  if (file) {
    std::vector<uint8_t> out_buffer(file->GetWidth() * file->GetHeight() * 4);
    bool result = astc_codec::DecompressToImage(
        *file, out_buffer.data(), out_buffer.size(), file->GetWidth() * 4);
    benchmark::DoNotOptimize(result);
  }

  return 0;
}
