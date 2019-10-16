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

#ifndef ASTC_CODEC_DECODER_CODEC_H_
#define ASTC_CODEC_DECODER_CODEC_H_

#include "src/decoder/astc_file.h"
#include "src/decoder/footprint.h"

#include <string>

namespace astc_codec {

// Decompresses ASTC blocks to an image buffer.
// Returns true if the decompression succeeded and the out buffer has been
// filled.
bool DecompressToImage(const uint8_t* astc_data, size_t astc_data_size,
                       size_t width, size_t height, Footprint footprint,
                       uint8_t* out_buffer, size_t out_buffer_size,
                       size_t out_buffer_stride);

// Decompresses an ASTC file to an image buffer.
// Returns true if the decompression succeeded and the out buffer has been
// filled.
bool DecompressToImage(const ASTCFile& file, uint8_t* out_buffer,
                       size_t out_buffer_size, size_t out_buffer_stride);

}  // namespace astc_codec

#endif  // ASTC_CODEC_DECODER_CODEC_H_
