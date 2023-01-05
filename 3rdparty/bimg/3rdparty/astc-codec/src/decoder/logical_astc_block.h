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

#ifndef ASTC_CODEC_DECODER_LOGICAL_ASTC_BLOCK_H_
#define ASTC_CODEC_DECODER_LOGICAL_ASTC_BLOCK_H_

#include "src/base/optional.h"
#include "src/decoder/footprint.h"
#include "src/decoder/intermediate_astc_block.h"
#include "src/decoder/partition.h"
#include "src/decoder/physical_astc_block.h"

#include <array>
#include <utility>
#include <vector>

namespace astc_codec {

// A logical ASTC block holds the endpoints, indices, and partition information
// of a compressed block. These values generally do not adhere to any
// quality-for-bitrate-imposed limits and are solely logical entities for
// determining the best representation of a given block.
class LogicalASTCBlock {
 public:
  LogicalASTCBlock(const LogicalASTCBlock&) = default;
  LogicalASTCBlock(LogicalASTCBlock&&) = default;

  // Unpack an intermediate block into a logical one.
  LogicalASTCBlock(const Footprint& footprint,
                   const IntermediateBlockData& block);

  // Unpack a void extent intermediate block into a logical one.
  LogicalASTCBlock(const Footprint& footprint, const VoidExtentData& block);

  // Create a new, empty ASTC block
  explicit LogicalASTCBlock(const Footprint& footprint);

  // Returns the footprint associated with this block. The footprint is defined
  // via the partition, because the partition definition is dependent on the
  // footprint.
  const Footprint& GetFootprint() const { return partition_.footprint; }

  // Returns the unquantized and infilled weight in the range [0, 64] for the
  // given texel location. Assumes that the block is a single-plane block,
  // meaning that weights are used equally across all channels.
  void SetWeightAt(int x, int y, int weight);
  int WeightAt(int x, int y) const;

  // Returns the unquantized and infilled weight in the range [0, 64] for the
  // given channel at the given texel location. If the block does not have a
  // dual-plane channel then the reference-returning version will fail, as it
  // cannot return a reference to a value that (potentially) doesn't exist.
  void SetDualPlaneWeightAt(int channel, int x, int y, int weight);
  int DualPlaneWeightAt(int channel, int x, int y) const;

  // Returns the color as it would be in the given pixel coordinates of the
  // block. Fails if the coordinates are outside of the range of the block
  // footprint
  RgbaColor ColorAt(int x, int y) const;

  // Sets the current partition for the block. |p|'s footprint must match the
  // return value of GetFootprint() or else this call will fail.
  void SetPartition(const Partition& p);

  // Sets the endpoints for the given subset.
  void SetEndpoints(const EndpointPair& eps, int subset);
  void SetEndpoints(const Endpoint& ep1, const Endpoint& ep2, int subset) {
    SetEndpoints(std::make_pair(ep1, ep2), subset);
  }

  // Sets the dual plane channel for the block. Value must be within the range
  // [0, 3]. If a negative value is passed, then the dual-plane data for the
  // block is removed, and the block is treated as a single-plane block.
  void SetDualPlaneChannel(int channel);
  bool IsDualPlane() const { return dual_plane_.hasValue(); }

 private:
  // A block may have up to four endpoint pairs.
  std::vector<EndpointPair> endpoints_;

  // Weights are stored as values in the interval [0, 64].
  std::vector<int> weights_;

  // The partition information for this block. This determines the
  // appropriate subsets that each pixel should belong to.
  Partition partition_;

  // Dual plane data holds both the channel and the weights that describe
  // the dual plane data for the given block. If a block has a dual plane, then
  // we need to know both the channel and the weights associated with it.
  struct DualPlaneData {
    int channel;
    std::vector<int> weights;
  };

  // The dual-plane data is optional from a logical representation of the block.
  base::Optional<DualPlaneData> dual_plane_;

  // Calculates the unquantized and interpolated weights from the encoded weight
  // values and possibly dual-plane weights specified in the passed ASTC block.
  void CalculateWeights(const Footprint& footprint,
                        const IntermediateBlockData& block);

  // Calculates the weights for a VoidExtentBlock.
  void CalculateWeights(const Footprint& footprint,
                        const VoidExtentData& block);
};

// Unpacks the physical ASTC block into a logical block. Returns false if the
// physical block is an error encoded block.
base::Optional<LogicalASTCBlock> UnpackLogicalBlock(
    const Footprint& footprint, const PhysicalASTCBlock& pb);

}  // namespace astc_codec

#endif  // ASTC_CODEC_DECODER_LOGICAL_ASTC_BLOCK_H_
