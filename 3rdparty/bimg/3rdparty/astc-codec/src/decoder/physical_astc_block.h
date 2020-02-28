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

#ifndef ASTC_CODEC_DECODER_PHYSICAL_ASTC_BLOCK_H_
#define ASTC_CODEC_DECODER_PHYSICAL_ASTC_BLOCK_H_

// The logic in this file is based on the ASTC specification, which can be
// found here:
// https://www.opengl.org/registry/specs/KHR/texture_compression_astc_hdr.txt

#include "src/base/optional.h"
#include "src/base/uint128.h"
#include "src/decoder/types.h"

#include <string>

namespace astc_codec {

// A PhysicalASTCBlock contains all 128 bits and the logic for decoding the
// various internals of an ASTC block.
class PhysicalASTCBlock {
 public:
  // The physical size in bytes of an ASTC block
  static const size_t kSizeInBytes = 16;

  // Initializes an ASTC block based on the encoded string.
  explicit PhysicalASTCBlock(const std::string& encoded_block);
  explicit PhysicalASTCBlock(const base::UInt128 astc_block);

  // Returns the 128 bits of this ASTC block.
  base::UInt128 GetBlockBits() const { return astc_bits_; }

  // Weights are stored in a grid that may not have the same dimensions
  // as the block dimensions. This allows us to see what the physical
  // dimensions are of the grid.
  base::Optional<std::array<int, 2>> WeightGridDims() const;

  // The weight range is the maximum value a weight can take in the
  // weight grid.
  base::Optional<int> WeightRange() const;

  // Returns true if the block encoding specifies a void-extent block. This
  // kind of block stores a single color to be used for every pixel in the
  // block.
  bool IsVoidExtent() const;

  // Returns the values (min_s, max_s, min_t, max_t) as defined in the void
  // extent block as the range of texture coordinates for which this block is
  // defined. (See Section C.2.23)
  base::Optional<std::array<int, 4>> VoidExtentCoords() const;

  // Returns true if the block contains two separate weight grids. One used
  // for the channel returned by DualPlaneChannel() and one used by the other
  // channels.
  bool IsDualPlane() const;

  // Returns the channel used as the "dual plane". The return value is only
  // meaningful if IsDualPlane() returns true...
  base::Optional<int> DualPlaneChannel() const;

  // Returns a reason that the encoding doesn't adhere to the specification.
  // If the encoding is legal, then this returns a nullptr. This allows us to
  // still use code of the form:
  //
  //     if (IsIllegalEncoding()) {
  //       ... error ...
  //     }
  //     ... no error ...
  //
  // However, it also helps with debugging since we can find problems with
  // encodings a lot faster.
  base::Optional<std::string> IsIllegalEncoding() const;

  // Returns the number of weight bits present in this block.
  base::Optional<int> NumWeightBits() const;

  // Returns the starting position within the range [0, 127] of the
  // weight data within the block.
  base::Optional<int> WeightStartBit() const;

  // Returns the number of endpoint pairs used in this block.
  base::Optional<int> NumPartitions() const;

  // Returns the seed used to determine the partition for a given
  // (x, y) coordinate within the block. Determined using the
  // block size and the function as described in the specification.
  base::Optional<int> PartitionID() const;

  // Returns the color endpoint mode for the given partition index.
  base::Optional<ColorEndpointMode> GetEndpointMode(int partition) const;

  // Returns the starting position within the range [0, 127] of the
  // color data within the block.
  base::Optional<int> ColorStartBit() const;

  // Returns the number of integers used to represent the color endpoints.
  base::Optional<int> NumColorValues() const;

  // Returns the number of bits used to represent the color endpoints.
  base::Optional<int> NumColorBits() const;

  // Returns the maximum value that each of the encoded integers used to
  // represent the color endpoints can take.
  base::Optional<int> ColorValuesRange() const;

 private:
  const base::UInt128 astc_bits_;

  // The logic to return the number of color bits and the color values range
  // is very similar, so it's probably best to abstract it away into its own
  // function.
  void GetColorValuesInfo(int* color_bits, int* color_range) const;
};

}  // namespace astc_codec

#endif  // ASTC_CODEC_DECODER_PHYSICAL_ASTC_BLOCK_H_
