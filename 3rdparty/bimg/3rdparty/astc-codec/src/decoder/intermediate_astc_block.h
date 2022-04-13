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

#ifndef ASTC_CODEC_DECODER_INTERMEDIATE_ASTC_BLOCK_H_
#define ASTC_CODEC_DECODER_INTERMEDIATE_ASTC_BLOCK_H_

#include "src/base/optional.h"
#include "src/base/uint128.h"
#include "src/decoder/physical_astc_block.h"

#include <array>
#include <vector>

namespace astc_codec {

// From Table C.2.7 -- These are the only valid ranges that weight
// values can take.
constexpr std::array<int, 12> kValidWeightRanges =
{{ 1, 2, 3, 4, 5, 7, 9, 11, 15, 19, 23, 31 }};

// Void extent data are all the ASTC blocks that are labeled for having a
// constant color. In the ASTC spec, some of these blocks may optionally
// have "void extent coordinates" that describe how far in texture space
// the constant color should span. If these coordinates are not valid,
// then the coordinates are all set to a fully saturated bit mask
// ((1 << 13) - 1) and the block is treated as a singular constant color.
// We call both types of these blocks "void extent" to remove confusion
// in our code.
struct VoidExtentData {
  uint16_t r;
  uint16_t g;
  uint16_t b;
  uint16_t a;
  std::array<uint16_t, 4> coords;
};

// Intermediate endpoint data. Really this is just an endpoint mode
// and a couple of values that represent the data used to decode the
// RGB values from the color endpoint mode.
struct IntermediateEndpointData {
  ColorEndpointMode mode;
  std::vector<int> colors;
};

// This is an unpacked physical ASTC block, but it does not have enough
// information to be worked with logically. It is simply a container of
// all of the unpacked ASTC information. It is used as a staging area
// for the information that is later Pack()'d into a PhysicalASTCBlock.
struct IntermediateBlockData {
  int weight_grid_dim_x;
  int weight_grid_dim_y;
  int weight_range;

  // Quantized, non-interpolated weights
  std::vector<int> weights;

  // The 10-bit partition ID if we need one
  base::Optional<int> partition_id;

  // The dual-plane channel in [0, 3] if it exists.
  base::Optional<int> dual_plane_channel;

  // The quantized/encoded endpoint values for this block. The range of each
  // endpoint value is specified by |endpoint_range|, if it exists. If not, the
  // range can be queried by calling EndpointRangeForBlock
  std::vector<IntermediateEndpointData> endpoints;

  // The range [0, endpoint_range] that any one endpoint value can take. Users
  // should not write to this value themselves. If it is empty at the time
  // someone calls Pack(), it will be automatically inferred. Otherwise, it is
  // set by Unpack() based on what the underlying encoding specified.
  base::Optional<int> endpoint_range;
};

// Returns the maximum value that a given endpoint value can take according to
// the other settings in the block. Ignores the |endpoint_range| member
// variable. Returns negative values on error:
//  -1 : Too many bits required to store weight grid
//  -2 : There are too few bits allocated for color endpoint data according to
//       C.2.24 in the ASTC spec
int EndpointRangeForBlock(const IntermediateBlockData& data);
inline int EndpointRangeForBlock(const VoidExtentData& data);

// Unpacks the physical ASTC block into the intermediate block. Returns false
// if the physical block is an error encoded block, or if the physical block
// is a void extent block. On error the contents of ib are undefined.
base::Optional<IntermediateBlockData> UnpackIntermediateBlock(
    const PhysicalASTCBlock& pb);

// Unpacks the physical ASTC block into a void extent block. Returns false
// if the physical block is an error encoded block, or if the physical block
// is an intermediate block. On error the contents of ib are undefined.
base::Optional<VoidExtentData> UnpackVoidExtent(const PhysicalASTCBlock& pb);

// Packs the given intermediate block into a physical block. Returns an error
// string if the provided values in the intermediate block emit an illegal ASTC
// encoding. In this case the results in the physical block are undefined.
base::Optional<std::string> Pack(const IntermediateBlockData& data,
                                 base::UInt128* pb);

// Packs the given void extent block into a physical block. Returns an error
// string if the provided values in the void extent block emit an illegal ASTC
// encoding. In this case the results in the physical block are undefined.
base::Optional<std::string> Pack(const VoidExtentData& data, base::UInt128* pb);

////////////////////////////////////////////////////////////////////////////////
//
// Impl

inline int EndpointRangeForBlock(const VoidExtentData&) {
  // Void extent blocks use 16-bit ARGB definitions
  return (1 << 16) - 1;
}

}  // namespace astc_codec

#endif  // ASTC_CODEC_DECODER_INTERMEDIATE_ASTC_BLOCK_H_
