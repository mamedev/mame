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

#include "src/decoder/intermediate_astc_block.h"
#include "src/decoder/integer_sequence_codec.h"
#include "src/base/bit_stream.h"
#include "src/base/math_utils.h"
#include "src/base/optional.h"
#include "src/base/uint128.h"

#include <algorithm>
#include <numeric>
#include <sstream>

namespace astc_codec {

namespace {

constexpr int kEndpointRange_ReturnInvalidWeightDims = -1;
constexpr int kEndpointRange_ReturnNotEnoughColorBits = -2;

base::UInt128 PackVoidExtentBlock(uint16_t r, uint16_t g, uint16_t b,
                                  uint16_t a, std::array<uint16_t, 4> coords) {
  base::BitStream<base::UInt128> bit_sink;

  // Put void extent mode...
  bit_sink.PutBits(0xDFC, 12);

  // Each of the coordinates goes in 13 bits at a time.
  for (auto coord : coords) {
    assert(coord < 1 << 13);
    bit_sink.PutBits(coord, 13);
  }
  assert(bit_sink.Bits() == 64);

  // Then we add R, G, B, and A in order
  bit_sink.PutBits(r, 16);
  bit_sink.PutBits(g, 16);
  bit_sink.PutBits(b, 16);
  bit_sink.PutBits(a, 16);

  assert(bit_sink.Bits() == 128);

  base::UInt128 result;
  bit_sink.GetBits(128, &result);
  return result;
}

base::Optional<std::string> GetEncodedWeightRange(int range,
                                                  std::array<int, 3>* const r) {
  const std::array<std::array<int, 3>, 12> kValidRangeEncodings =
      {{ {{ 0, 1, 0 }}, {{ 1, 1, 0 }}, {{ 0, 0, 1 }},
         {{ 1, 0, 1 }}, {{ 0, 1, 1 }}, {{ 1, 1, 1 }},
         {{ 0, 1, 0 }}, {{ 1, 1, 0 }}, {{ 0, 0, 1 }},
         {{ 1, 0, 1 }}, {{ 0, 1, 1 }}, {{ 1, 1, 1 }} }};

  // If our range is larger than all available ranges, this is an error.
  const int smallest_range = kValidWeightRanges.front();
  const int largest_range = kValidWeightRanges.back();
  if (range < smallest_range || largest_range < range) {
    std::stringstream strm;
    strm << "Could not find block mode. Invalid weight range: "
         << range << " not in [" << smallest_range << ", "
         << largest_range << std::endl;
    return strm.str();
  }

  // Find the upper bound on the range, otherwise.
  const auto range_iter = std::lower_bound(
      kValidWeightRanges.cbegin(), kValidWeightRanges.cend(), range);
  auto enc_iter = kValidRangeEncodings.cbegin();
  enc_iter += std::distance(kValidWeightRanges.cbegin(), range_iter);
  *r = *enc_iter;
  return {};
}

struct BlockModeInfo {
  int min_weight_grid_dim_x;
  int max_weight_grid_dim_x;
  int min_weight_grid_dim_y;
  int max_weight_grid_dim_y;
  int r0_bit_pos;
  int r1_bit_pos;
  int r2_bit_pos;
  int weight_grid_x_offset_bit_pos;
  int weight_grid_y_offset_bit_pos;
  bool require_single_plane_low_prec;
};

constexpr int kNumBlockModes = 10;
const std::array<BlockModeInfo, kNumBlockModes> kBlockModeInfo {{
  { 4, 7, 2, 5, 4, 0, 1, 7, 5, false },      // B+4 A+2
  { 8, 11, 2, 5, 4, 0, 1, 7, 5, false },     // B+8 A+2
  { 2, 5, 8, 11, 4, 0, 1, 5, 7, false },     // A+2 B+8
  { 2, 5, 6, 7, 4, 0, 1, 5, 7, false },      // A+2 B+6
  { 2, 3, 2, 5, 4, 0, 1, 7, 5, false },      // B+2 A+2
  { 12, 12, 2, 5, 4, 2, 3, -1, 5, false },   // 12  A+2
  { 2, 5, 12, 12, 4, 2, 3, 5, -1, false },   // A+2 12
  { 6, 6, 10, 10, 4, 2, 3, -1, -1, false },  // 6   10
  { 10, 10, 6, 6, 4, 2, 3, -1, -1, false },  // 10  6
  { 6, 9, 6, 9, 4, 2, 3, 5, 9, true }        // A+6 B+6
}};

// These are the bits that must be set for ASTC to recognize a given
// block mode. They are the 1's set in table C.2.8 of the spec.
const std::array<int, kNumBlockModes> kBlockModeMask = {{
  0x0, 0x4, 0x8, 0xC, 0x10C, 0x0, 0x80, 0x180, 0x1A0, 0x100
}};

static base::Optional<std::string> PackBlockMode(int dim_x, int dim_y, int range,
                                          bool dual_plane,
                                          base::BitStream<base::UInt128>* const bit_sink) {
  // We need to set the high precision bit if our range is too high...
  bool high_prec = range > 7;

  std::array<int, 3> r = {};
  const auto result = GetEncodedWeightRange(range, &r);
  if (result) {
    return result;
  }

  // The high two bits of R must not be zero. If this happens then it's
  // an illegal encoding according to Table C.2.7 that should have gotten
  // caught in GetEncodedWeightRange
  assert((r[1] | r[2]) > 0);

  // Just go through the table and see if any of the modes can handle
  // the given dimensions.
  for (int mode = 0; mode < kNumBlockModes; ++mode) {
    const BlockModeInfo& block_mode = kBlockModeInfo[mode];

    bool is_valid_mode = true;
    is_valid_mode &= block_mode.min_weight_grid_dim_x <= dim_x;
    is_valid_mode &= dim_x <= block_mode.max_weight_grid_dim_x;
    is_valid_mode &= block_mode.min_weight_grid_dim_y <= dim_y;
    is_valid_mode &= dim_y <= block_mode.max_weight_grid_dim_y;
    is_valid_mode &= !(block_mode.require_single_plane_low_prec && dual_plane);
    is_valid_mode &= !(block_mode.require_single_plane_low_prec && high_prec);

    if (!is_valid_mode) {
      continue;
    }

    // Initialize to the bits we must set.
    uint32_t encoded_mode = kBlockModeMask[mode];
    auto setBit = [&encoded_mode](const uint32_t value, const uint32_t offset) {
      encoded_mode = (encoded_mode & ~(1 << offset)) | ((value & 1) << offset);
    };

    // Set all the bits we need to set
    setBit(r[0], block_mode.r0_bit_pos);
    setBit(r[1], block_mode.r1_bit_pos);
    setBit(r[2], block_mode.r2_bit_pos);

    // Find our width and height offset from the base width and height weight
    // grid dimension for the given block mode. These are the 1-2 bits that
    // get encoded in the block mode used to calculate the final weight grid
    // width and height.
    const int offset_x = dim_x - block_mode.min_weight_grid_dim_x;
    const int offset_y = dim_y - block_mode.min_weight_grid_dim_y;

    // If we don't have an offset position then our offset better be zero.
    // If this isn't the case, then this isn't a viable block mode and we
    // should have caught this sooner.
    assert(block_mode.weight_grid_x_offset_bit_pos >= 0 || offset_x == 0);
    assert(block_mode.weight_grid_y_offset_bit_pos >= 0 || offset_y == 0);

    encoded_mode |= offset_x << block_mode.weight_grid_x_offset_bit_pos;
    encoded_mode |= offset_y << block_mode.weight_grid_y_offset_bit_pos;

    if (!block_mode.require_single_plane_low_prec) {
      setBit(high_prec, 9);
      setBit(dual_plane, 10);
    }

    // Make sure that the mode is the first thing the bit sink is writing to
    assert(bit_sink->Bits() == 0);
    bit_sink->PutBits(encoded_mode, 11);

    return {};
  }

  return std::string("Could not find viable block mode");
}

// Returns true if all endpoint modes are equal.
bool SharedEndpointModes(const IntermediateBlockData& data) {
  return std::accumulate(
      data.endpoints.begin(), data.endpoints.end(), true,
      [&data](const bool& a, const IntermediateEndpointData& b) {
        return a && b.mode == data.endpoints[0].mode;
      });
}

// Returns the starting bit (between 0 and 128) where the extra CEM and
// dual plane info is stored in the ASTC block.
int ExtraConfigBitPosition(const IntermediateBlockData& data) {
  const bool has_dual_channel = data.dual_plane_channel.hasValue();
  const int num_weights = data.weight_grid_dim_x * data.weight_grid_dim_y *
      (has_dual_channel ? 2 : 1);
  const int num_weight_bits =
      IntegerSequenceCodec::GetBitCountForRange(num_weights, data.weight_range);

  int extra_config_bits = 0;
  if (!SharedEndpointModes(data)) {
    const int num_encoded_cem_bits = 2 + int(data.endpoints.size()) * 3;
    extra_config_bits = num_encoded_cem_bits - 6;
  }

  if (has_dual_channel) {
    extra_config_bits += 2;
  }

  return 128 - num_weight_bits - extra_config_bits;
}

}  // namespace

////////////////////////////////////////////////////////////////////////////////

base::Optional<IntermediateBlockData> UnpackIntermediateBlock(
    const PhysicalASTCBlock& pb) {
  if (pb.IsIllegalEncoding()) {
    return {};
  }

  if (pb.IsVoidExtent()) {
    return {};
  }

  // Non void extent? Then let's try to decode everything else.
  IntermediateBlockData data;

  // All blocks have color values...
  const base::UInt128 color_bits_mask =
      (base::UInt128(1) << pb.NumColorBits().value()) - 1;
  const base::UInt128 color_bits =
      (pb.GetBlockBits() >> pb.ColorStartBit().value()) & color_bits_mask;
  base::BitStream<base::UInt128> bit_src(color_bits, 128);

  IntegerSequenceDecoder color_decoder(pb.ColorValuesRange().value());
  const int num_colors_in_block = pb.NumColorValues().value();
  std::vector<int> colors = color_decoder.Decode(num_colors_in_block, &bit_src);

  // Decode simple info
  const auto weight_dims = pb.WeightGridDims();
  data.weight_grid_dim_x = weight_dims->at(0);
  data.weight_grid_dim_y = weight_dims->at(1);
  data.weight_range = pb.WeightRange().value();

  data.partition_id = pb.PartitionID();
  data.dual_plane_channel = pb.DualPlaneChannel();

  auto colors_iter = colors.begin();
  for (int i = 0; i < pb.NumPartitions().value(); ++i) {
    IntermediateEndpointData ep_data;
    ep_data.mode = pb.GetEndpointMode(i).value();

    const int num_colors = NumColorValuesForEndpointMode(ep_data.mode);
    ep_data.colors.insert(ep_data.colors.end(), colors_iter,
                          colors_iter + num_colors);
    colors_iter += num_colors;

    data.endpoints.push_back(ep_data);
  }
  assert(colors_iter == colors.end());
  data.endpoint_range = pb.ColorValuesRange().value();

  // Finally decode the weights
  const base::UInt128 weight_bits_mask =
      (base::UInt128(1) << pb.NumWeightBits().value()) - 1;
  const base::UInt128 weight_bits =
      base::ReverseBits(pb.GetBlockBits()) & weight_bits_mask;
  bit_src = base::BitStream<base::UInt128>(weight_bits, 128);

  IntegerSequenceDecoder weight_decoder(data.weight_range);
  int num_weights = data.weight_grid_dim_x * data.weight_grid_dim_y;
  num_weights *= pb.IsDualPlane() ? 2 : 1;
  data.weights = weight_decoder.Decode(num_weights, &bit_src);

  return data;
}

int EndpointRangeForBlock(const IntermediateBlockData& data) {
  // First check to see if we exceed the number of bits allotted for weights, as
  // specified in C.2.24. If so, then the endpoint range is meaningless, but not
  // because we had an overzealous color endpoint mode, so return a different
  // error code.
  if (IntegerSequenceCodec::GetBitCountForRange(
          data.weight_grid_dim_x * data.weight_grid_dim_y *
          (data.dual_plane_channel.hasValue() ? 2 : 1),
          data.weight_range) > 96) {
    return kEndpointRange_ReturnInvalidWeightDims;
  }

  const int num_partitions = int(data.endpoints.size());

  // Calculate the number of bits that we would write prior to getting to the
  // color endpoint data
  const int bits_written =
      11   // Block mode
      + 2  // Num partitions
      + ((num_partitions > 1) ? 10 : 0)  // Partition ID
      + ((num_partitions == 1) ? 4 : 6);  // Shared CEM bits

  // We can determine the range based on how many bits we have between the start
  // of the color endpoint data and the next section, which is the extra config
  // bit position
  const int color_bits_available = ExtraConfigBitPosition(data) - bits_written;

  int num_color_values = 0;
  for (const auto& ep_data : data.endpoints) {
    num_color_values += NumColorValuesForEndpointMode(ep_data.mode);
  }

  // There's no way any valid ASTC encoding has no room left for any color
  // values. If we hit this then something is wrong in the caller -- abort.
  // According to section C.2.24, the smallest number of bits available is
  // ceil(13*C/5), where C is the number of color endpoint integers needed.
  const int bits_needed = (13 * num_color_values + 4) / 5;
  if (color_bits_available < bits_needed) {
    return kEndpointRange_ReturnNotEnoughColorBits;
  }

  int color_value_range = 255;
  for (; color_value_range > 1; --color_value_range) {
    const int bits_for_range = IntegerSequenceCodec::GetBitCountForRange(
        num_color_values, color_value_range);
    if (bits_for_range <= color_bits_available) {
      break;
    }
  }

  return color_value_range;
}

base::Optional<VoidExtentData> UnpackVoidExtent(const PhysicalASTCBlock& pb) {
  if (pb.IsIllegalEncoding()) {
    return {};
  }

  if (!pb.IsVoidExtent()) {
    return {};
  }

  // All blocks have color values...
  const base::UInt128 color_bits_mask =
      (base::UInt128(1) << pb.NumColorBits().value()) - 1;
  const uint64_t color_bits = (
      (pb.GetBlockBits() >> pb.ColorStartBit().value()) & color_bits_mask).LowBits();

  assert(pb.NumColorValues().value() == 4);
  VoidExtentData data;
  data.r = static_cast<uint16_t>((color_bits >>  0) & 0xFFFF);
  data.g = static_cast<uint16_t>((color_bits >> 16) & 0xFFFF);
  data.b = static_cast<uint16_t>((color_bits >> 32) & 0xFFFF);
  data.a = static_cast<uint16_t>((color_bits >> 48) & 0xFFFF);

  const auto void_extent_coords = pb.VoidExtentCoords();
  if (void_extent_coords) {
    data.coords[0] = uint16_t(void_extent_coords->at(0));
    data.coords[1] = uint16_t(void_extent_coords->at(1));
    data.coords[2] = uint16_t(void_extent_coords->at(2));
    data.coords[3] = uint16_t(void_extent_coords->at(3));
  } else {
    uint16_t all_ones = (1 << 13) - 1;
    for (auto& coord : data.coords) {
      coord = all_ones;
    }
  }

  return data;
}

// Packs the given intermediate block into a physical block. Returns false if
// the provided values in the intermediate block emit an illegal ASTC
// encoding.
base::Optional<std::string> Pack(const IntermediateBlockData& data,
                                 base::UInt128* pb) {
  if (data.weights.size() !=
      size_t(data.weight_grid_dim_x * data.weight_grid_dim_y *
      (data.dual_plane_channel.hasValue() ? 2 : 1))) {
    return std::string("Incorrect number of weights!");
  }

  // If it's not a void extent block, then it gets a bit more tricky...
  base::BitStream<base::UInt128> bit_sink;

  // First we need to encode the block mode.
  const auto error_string = PackBlockMode(
      data.weight_grid_dim_x, data.weight_grid_dim_y, data.weight_range,
      data.dual_plane_channel.hasValue(), &bit_sink);
  if (error_string) {
    return error_string;
  }

  // Next, we place the number of partitions minus one.
  const int num_partitions = int(data.endpoints.size());
  bit_sink.PutBits(num_partitions - 1, 2);

  // If we have more than one partition, then we also have a partition ID.
  if (num_partitions > 1) {
    const int id = data.partition_id.value();
    assert(id >= 0);
    bit_sink.PutBits(id, 10);
  }

  // Take a detour, let's encode the weights so that we know how many bits they
  // consume.
  base::BitStream<base::UInt128> weight_sink;

  IntegerSequenceEncoder weight_enc(data.weight_range);
  for (auto weight : data.weights) {
    weight_enc.AddValue(weight);
  }
  weight_enc.Encode(&weight_sink);

  const int num_weight_bits = weight_sink.Bits();
  assert(num_weight_bits ==
           IntegerSequenceCodec::GetBitCountForRange(
               int(data.weights.size()), data.weight_range));

  // Let's continue... how much after the color data do we need to write?
  int extra_config = 0;

  // Determine if all endpoint pairs share the same endpoint mode
  assert(data.endpoints.size() > 0);
  bool shared_endpoint_mode = SharedEndpointModes(data);

  // The first part of the endpoint mode (CEM) comes directly after the
  // partition info, if it exists. If there is no partition info, the CEM comes
  // right after the block mode. In the single-partition case, we just write out
  // the entire singular CEM, but in the multi-partition case, if all CEMs are
  // the same then their shared CEM is specified directly here, too. In both
  // cases, shared_endpoint_mode is true (in the singular case,
  // shared_endpoint_mode is trivially true).
  if (shared_endpoint_mode) {
    if (num_partitions > 1) {
      bit_sink.PutBits(0, 2);
    }
    bit_sink.PutBits(static_cast<int>(data.endpoints[0].mode), 4);
  } else {
    // Here, the CEM is not shared across all endpoint pairs, and we need to
    // figure out what to place here, and what to place in the extra config
    // bits before the weight data...

    // Non-shared config modes must all be within the same class (out of four)
    // See Section C.2.11
    int min_class = 2;  // We start with 2 here instead of three because it's
                        // the highest that can be encoded -- even if all modes
                        // are class 3.
    int max_class = 0;
    for (const auto& ep_data : data.endpoints) {
      const int ep_mode_class = static_cast<int>(ep_data.mode) >> 2;
      min_class = std::min(min_class, ep_mode_class);
      max_class = std::max(max_class, ep_mode_class);
    }

    assert(max_class >= min_class);

    if (max_class - min_class > 1) {
      return std::string("Endpoint modes are invalid");
    }

    // Construct the CEM mode -- six of its bits will fit here, but otherwise
    // the rest will go in the extra configuration bits.
    base::BitStream<uint32_t> cem_encoder;

    // First encode the base class
    assert(min_class >= 0);
    assert(min_class < 3);
    cem_encoder.PutBits(min_class + 1, 2);

    // Next, encode the class selector bits -- this is simply the offset
    // from the base class
    for (const auto& ep_data : data.endpoints) {
      const int ep_mode_class = static_cast<int>(ep_data.mode) >> 2;
      const int class_selector_bit = ep_mode_class - min_class;
      assert(class_selector_bit == 0 || class_selector_bit == 1);
      cem_encoder.PutBits(class_selector_bit, 1);
    }

    // Finally, we need to choose from each class which actual mode
    // we belong to and encode those.
    for (const auto& ep_data : data.endpoints) {
      const int ep_mode = static_cast<int>(ep_data.mode) & 3;
      assert(ep_mode < 4);
      cem_encoder.PutBits(ep_mode, 2);
    }
    assert(cem_encoder.Bits() == uint32_t(2 + num_partitions * 3));

    uint32_t encoded_cem;
    cem_encoder.GetBits(2 + num_partitions * 3, &encoded_cem);

    // Since only six bits fit here before the color endpoint data, the rest
    // need to go in the extra config data.
    extra_config = encoded_cem >> 6;

    // Write out the six bits we had
    bit_sink.PutBits(encoded_cem, 6);
  }

  // If we have a dual-plane channel, we can tack that onto our extra config
  // data
  if (data.dual_plane_channel.hasValue()) {
    const int channel = data.dual_plane_channel.value();
    assert(channel < 4);
    extra_config <<= 2;
    extra_config |= channel;
  }

  // Get the range of endpoint values. It can't be -1 because we should have
  // checked for that much earlier.
  const int color_value_range = data.endpoint_range
      ? data.endpoint_range.value()
      : EndpointRangeForBlock(data);

  assert(color_value_range != kEndpointRange_ReturnInvalidWeightDims);
  if (color_value_range == kEndpointRange_ReturnNotEnoughColorBits) {
    return { "Intermediate block emits illegal color range" };
  }

  IntegerSequenceEncoder color_enc(color_value_range);
  for (const auto& ep_data : data.endpoints) {
    for (int color : ep_data.colors) {
      if (color > color_value_range) {
        return { "Color outside available color range!" };
      }

      color_enc.AddValue(color);
    }
  }
  color_enc.Encode(&bit_sink);

  // Now we need to skip some bits to get to the extra configuration bits. The
  // number of bits we need to skip depends on where we are in the stream and
  // where we need to get to.
  const int extra_config_bit_position = ExtraConfigBitPosition(data);
  const int extra_config_bits =
      128 - num_weight_bits - extra_config_bit_position;
  assert(extra_config_bits >= 0);
  assert(extra_config < 1 << extra_config_bits);

  // Make sure the color encoder didn't write more than we thought it would.
  int bits_to_skip = extra_config_bit_position - bit_sink.Bits();
  assert(bits_to_skip >= 0);

  while (bits_to_skip > 0) {
    const int skipping = std::min(32, bits_to_skip);
    bit_sink.PutBits(0, skipping);
    bits_to_skip -= skipping;
  }

  // Finally, write out the rest of the config bits.
  bit_sink.PutBits(extra_config, extra_config_bits);

  // We should be right up to the weight bits...
  assert(bit_sink.Bits() == uint32_t(128 - num_weight_bits));

  // Flush out our bit writer and write out the weight bits
  base::UInt128 astc_bits;
  bit_sink.GetBits(128 - num_weight_bits, &astc_bits);

  base::UInt128 rev_weight_bits;
  weight_sink.GetBits(weight_sink.Bits(), &rev_weight_bits);

  astc_bits |= base::ReverseBits(rev_weight_bits);

  // And we're done! Whew!
  *pb = astc_bits;
  return PhysicalASTCBlock(*pb).IsIllegalEncoding();
}

base::Optional<std::string> Pack(const VoidExtentData& data,
                                 base::UInt128* pb) {
  *pb = PackVoidExtentBlock(data.r, data.g, data.b, data.a, data.coords);
  return PhysicalASTCBlock(*pb).IsIllegalEncoding();
}

}  // namespace astc_codec
