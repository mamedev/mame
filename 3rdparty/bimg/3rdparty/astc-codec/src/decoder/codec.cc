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

#include "src/decoder/codec.h"
#include "src/base/uint128.h"
#include "src/decoder/logical_astc_block.h"
#include "src/decoder/physical_astc_block.h"

#include <cstring>

namespace astc_codec {

namespace {
static constexpr size_t kBytesPerPixelUNORM8 = 4;
}

bool DecompressToImage(const uint8_t* astc_data, size_t astc_data_size,
                       size_t width, size_t height, Footprint footprint,
                       uint8_t* out_buffer, size_t out_buffer_size,
                       size_t out_buffer_stride) {
  const size_t block_width = footprint.Width();
  const size_t block_height = footprint.Height();
  assert(block_width != 0);
  assert(block_height != 0);

  if (width == 0 || height == 0) {
    return false;
  }

  const size_t blocks_wide = (width + block_width - 1) / block_width;
  assert(blocks_wide != 0);

  // Check that this buffer has the expected number of blocks.
  const size_t expected_block_count =
      ((width + block_width - 1) / block_width) *
      ((height + block_height - 1) / block_height);

  if (astc_data_size % PhysicalASTCBlock::kSizeInBytes != 0 ||
      astc_data_size / PhysicalASTCBlock::kSizeInBytes !=
          expected_block_count) {
    // TODO(google): Expose error?
    return false;
  }

  if (kBytesPerPixelUNORM8 * width > out_buffer_stride ||
      out_buffer_stride * height < out_buffer_size) {
    // Output buffer too small.
    return false;
  }

  base::UInt128 block;
  static_assert(sizeof(block) == PhysicalASTCBlock::kSizeInBytes,
                "Block size mismatch");

  for (size_t i0 = 0; i0 < astc_data_size; i0 += PhysicalASTCBlock::kSizeInBytes) {
    const size_t block_index = i0 / PhysicalASTCBlock::kSizeInBytes;
    const size_t block_x = block_index % blocks_wide;
    const size_t block_y = block_index / blocks_wide;
    block = *(base::UInt128*)(astc_data + i0);

    PhysicalASTCBlock physical_block(block);
    auto lb = UnpackLogicalBlock(footprint, physical_block);
    if (!lb) {
      return false;
    }

    LogicalASTCBlock logical_block = lb.value();

    for (size_t y = 0; y < block_height; ++y) {
      const size_t py = block_height * block_y + y;
      uint8_t* out_row = out_buffer + py * out_buffer_stride;

      for (size_t x = 0; x < block_width; ++x) {
        const size_t px = block_width * block_x + x;

        // Skip out of bounds.
        if (px >= width || py >= height) {
          continue;
        }

        uint8_t* pixel = out_row + px * kBytesPerPixelUNORM8;
        const RgbaColor decoded_color = logical_block.ColorAt(int(x), int(y));
        for (size_t i = 0; i < kBytesPerPixelUNORM8; ++i) {
          pixel[i] = static_cast<uint8_t>(decoded_color[i]);
        }
      }
    }
  }

  return true;
}

bool DecompressToImage(const ASTCFile& file, uint8_t* out_buffer,
                       size_t out_buffer_size, size_t out_buffer_stride) {
  base::Optional<Footprint> footprint = file.GetFootprint();
  if (!footprint) {
    return false;
  }

  return DecompressToImage(
      reinterpret_cast<const uint8_t*>(file.GetRawBlockData().c_str()),
      file.GetRawBlockData().size(), file.GetWidth(), file.GetHeight(),
      footprint.value(), out_buffer, out_buffer_size, out_buffer_stride);
}

bool ASTCDecompressToRGBA(const uint8_t* astc_data, size_t astc_data_size,
                          size_t width, size_t height, FootprintType footprint,
                          uint8_t* out_buffer, size_t out_buffer_size,
                          size_t out_buffer_stride) {
  base::Optional<Footprint> footprint_opt =
      Footprint::FromFootprintType(footprint);
  if (!footprint_opt) {
    return false;
  }

  return DecompressToImage(astc_data, astc_data_size, width, height,
                           footprint_opt.value(), out_buffer, out_buffer_size,
                           out_buffer_stride);
}

}  // namespace astc_codec
