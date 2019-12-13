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

#include "src/decoder/physical_astc_block.h"
#include "src/base/math_utils.h"
#include "src/base/optional.h"
#include "src/base/uint128.h"
#include "src/decoder/integer_sequence_codec.h"

#include <array>
#include <cmath>

namespace astc_codec {

namespace {

static_assert(static_cast<int>(ColorEndpointMode::kNumColorEndpointModes) == 16,
              "There are only sixteen color endpoint modes defined in the "
              "ASTC specification. If this is false, then the enum may be "
              "incorrect.");

constexpr int kASTCBlockSizeBits = 128;
//constexpr int kASTCBlockSizeBytes = kASTCBlockSizeBits / 8;
constexpr uint32_t kVoidExtentMaskBits = 9;
constexpr uint32_t kVoidExtentMask = 0x1FC;
constexpr int kWeightGridMinBitLength = 24;
constexpr int kWeightGridMaxBitLength = 96;
constexpr int kMaxNumPartitions = 4;
constexpr int kMaxNumWeights = 64;

// These are the overall block modes defined in table C.2.8.  There are 10
// weight grid encoding schemes + void extent.
enum class BlockMode {
  kB4_A2,
  kB8_A2,
  kA2_B8,
  kA2_B6,
  kB2_A2,
  k12_A2,
  kA2_12,
  k6_10,
  k10_6,
  kA6_B6,
  kVoidExtent,
};

struct WeightGridProperties {
  int width;
  int height;
  int range;
};

// Local function prototypes
base::Optional<BlockMode> DecodeBlockMode(const base::UInt128 astc_bits);
base::Optional<WeightGridProperties> DecodeWeightProps(
    const base::UInt128 astc_bits, std::string* error);
std::array<int, 4> DecodeVoidExtentCoords(const base::UInt128 astc_bits);
bool DecodeDualPlaneBit(const base::UInt128 astc_bits);
int DecodeNumPartitions(const base::UInt128 astc_bits);
int DecodeNumWeightBits(const base::UInt128 astc_bits);
int DecodeDualPlaneBitStartPos(const base::UInt128 astc_bits);
ColorEndpointMode DecodeEndpointMode(const base::UInt128 astc_bits,
                                     int partition);
int DecodeNumColorValues(const base::UInt128 astc_bits);

// Returns the block mode, if it's valid.
base::Optional<BlockMode> DecodeBlockMode(const base::UInt128 astc_bits) {
  using Result = base::Optional<BlockMode>;
  const uint64_t low_bits = astc_bits.LowBits();
  if (base::GetBits(low_bits, 0, kVoidExtentMaskBits) == kVoidExtentMask) {
    return Result(BlockMode::kVoidExtent);
  }

  if (base::GetBits(low_bits, 0, 2) != 0) {
    const uint64_t mode_bits = base::GetBits(low_bits, 2, 2);
    switch (mode_bits) {
      case 0: return Result(BlockMode::kB4_A2);
      case 1: return Result(BlockMode::kB8_A2);
      case 2: return Result(BlockMode::kA2_B8);
      case 3: return base::GetBits(low_bits, 8, 1) ?
          Result(BlockMode::kB2_A2) : Result(BlockMode::kA2_B6);
    }
  } else {
    const uint64_t mode_bits = base::GetBits(low_bits, 5, 4);
    if ((mode_bits & 0xC) == 0x0) {
      if (base::GetBits(low_bits, 0, 4) == 0) {
        // Reserved.
        return Result();
      } else {
        return Result(BlockMode::k12_A2);
      }
    } else if ((mode_bits & 0xC) == 0x4) {
      return Result(BlockMode::kA2_12);
    } else if (mode_bits == 0xC) {
      return Result(BlockMode::k6_10);
    } else if (mode_bits == 0xD) {
      return Result(BlockMode::k10_6);
    } else if ((mode_bits & 0xC) == 0x8) {
      return Result(BlockMode::kA6_B6);
    }
  }

  return Result();
}

base::Optional<WeightGridProperties> DecodeWeightProps(
    const base::UInt128 astc_bits, std::string* error) {
  auto block_mode = DecodeBlockMode(astc_bits);
  if (!block_mode) {
    *error = "Reserved block mode";
    return {};
  }

  // The dimensions of the weight grid and their range
  WeightGridProperties props;

  // Determine the weight extents based on the block mode
  const uint32_t low_bits =
      static_cast<uint32_t>(astc_bits.LowBits() & 0xFFFFFFFF);
  switch (block_mode.value()) {
    case BlockMode::kB4_A2: {
      int a = base::GetBits(low_bits, 5, 2);
      int b = base::GetBits(low_bits, 7, 2);
      props.width = b + 4;
      props.height = a + 2;
    }
    break;

    case BlockMode::kB8_A2: {
      int a = base::GetBits(low_bits, 5, 2);
      int b = base::GetBits(low_bits, 7, 2);
      props.width = b + 8;
      props.height = a + 2;
    }
    break;

    case BlockMode::kA2_B8: {
      int a = base::GetBits(low_bits, 5, 2);
      int b = base::GetBits(low_bits, 7, 2);
      props.width = a + 2;
      props.height = b + 8;
    }
    break;

    case BlockMode::kA2_B6: {
      int a = base::GetBits(low_bits, 5, 2);
      int b = base::GetBits(low_bits, 7, 1);
      props.width = a + 2;
      props.height = b + 6;
    }
    break;

    case BlockMode::kB2_A2: {
      int a = base::GetBits(low_bits, 5, 2);
      int b = base::GetBits(low_bits, 7, 1);
      props.width = b + 2;
      props.height = a + 2;
    }
    break;

    case BlockMode::k12_A2: {
      int a = base::GetBits(low_bits, 5, 2);
      props.width = 12;
      props.height = a + 2;
    }
    break;

    case BlockMode::kA2_12: {
      int a = base::GetBits(low_bits, 5, 2);
      props.width = a + 2;
      props.height = 12;
    }
    break;

    case BlockMode::k6_10: {
      props.width = 6;
      props.height = 10;
    }
    break;

    case BlockMode::k10_6: {
      props.width = 10;
      props.height = 6;
    }
    break;

    case BlockMode::kA6_B6: {
      int a = base::GetBits(low_bits, 5, 2);
      int b = base::GetBits(low_bits, 9, 2);
      props.width = a + 6;
      props.height = b + 6;
    }
    break;

    // Void extent blocks have no weight grid.
    case BlockMode::kVoidExtent:
      *error = "Void extent block has no weight grid";
      return {};

    // We have a valid block mode which isn't a void extent? We
    // should be able to decode the weight grid dimensions.
    default:
      assert(false && "Error decoding weight grid");
      *error = "Internal error";
      return {};
  }

  // Determine the weight range based on the block mode
  uint32_t r = base::GetBits(low_bits, 4, 1);
  switch (block_mode.value()) {
    case BlockMode::kB4_A2:
    case BlockMode::kB8_A2:
    case BlockMode::kA2_B8:
    case BlockMode::kA2_B6:
    case BlockMode::kB2_A2: {
      r |= base::GetBits(low_bits, 0, 2) << 1;
    }
    break;

    case BlockMode::k12_A2:
    case BlockMode::kA2_12:
    case BlockMode::k6_10:
    case BlockMode::k10_6:
    case BlockMode::kA6_B6:  {
      r |= base::GetBits(low_bits, 2, 2) << 1;
    }
    break;

    // We have a valid block mode which doesn't have weights? We
    // should have caught this earlier.
    case BlockMode::kVoidExtent:
    default:
      assert(false && "Error decoding weight grid");
      *error = "Internal error";
      return {};
  }

  // Decode the range...
  // High bit is in bit 9 unless we're using a particular block mode
  uint32_t h = base::GetBits(low_bits, 9, 1);
  if (block_mode == BlockMode::kA6_B6) {
    h = 0;
  }

  // Figure out the range of the weights (Table C.2.7)
  constexpr std::array<int, 16> kWeightRanges = {{
      -1, -1, 1, 2, 3, 4, 5, 7, -1, -1, 9, 11, 15, 19, 23, 31
    }};

  assert(((h << 3) | r) < kWeightRanges.size());

  props.range = kWeightRanges.at((h << 3) | r);
  if (props.range < 0) {
    *error = "Reserved range for weight bits";
    return {};
  }

  // Error checking -- do we have too many weights?
  int num_weights = props.width * props.height;
  if (DecodeDualPlaneBit(astc_bits)) {
    num_weights *= 2;
  }

  if (kMaxNumWeights < num_weights) {
    *error = "Too many weights specified";
    return {};
  }

  // Do we have too many weight bits?
  const int bit_count =
      IntegerSequenceCodec::GetBitCountForRange(num_weights, props.range);

  if (bit_count < kWeightGridMinBitLength) {
    *error = "Too few bits required for weight grid";
    return {};
  }

  if (kWeightGridMaxBitLength < bit_count) {
    *error = "Too many bits required for weight grid";
    return {};
  }

  return props;
}

// Returns the four 13-bit integers that define the range of texture
// coordinates present in a void extent block as defined in Section
// C.2.23 of the specification. The coordinates returned are of
// the form (min_s, max_s, min_t, max_t)
std::array<int, 4> DecodeVoidExtentCoords(const base::UInt128 astc_bits) {
  const uint64_t low_bits = astc_bits.LowBits();

  std::array<int, 4> coords;
  for (int i = 0; i < 4; ++i) {
    coords[i] = static_cast<int>(base::GetBits(low_bits, 12 + 13 * i, 13));
  }

  return coords;
}

bool DecodeDualPlaneBit(const base::UInt128 astc_bits) {
  base::Optional<BlockMode> block_mode = DecodeBlockMode(astc_bits);

  // Void extent blocks certainly aren't dual-plane.
  if (block_mode == BlockMode::kVoidExtent) {
    return false;
  }

  // One special block mode doesn't have any dual plane bit
  if (block_mode == BlockMode::kA6_B6) {
    return false;
  }

  // Otherwise, dual plane is determined by the 10th bit.
  constexpr int kDualPlaneBitPosition = 10;
  return base::GetBits(astc_bits, kDualPlaneBitPosition, 1) != 0;
}

int DecodeNumPartitions(const base::UInt128 astc_bits) {
  constexpr int kNumPartitionsBitPosition = 11;
  constexpr int kNumPartitionsBitLength = 2;

  // Non-void extent blocks
  const uint64_t low_bits = astc_bits.LowBits();
  const int num_partitions = 1 + static_cast<int>(
      base::GetBits(low_bits,
                    kNumPartitionsBitPosition,
                    kNumPartitionsBitLength));
  assert(num_partitions > 0);
  assert(num_partitions <= kMaxNumPartitions);

  return num_partitions;
}

int DecodeNumWeightBits(const base::UInt128 astc_bits) {
  std::string error;
  auto maybe_weight_props = DecodeWeightProps(astc_bits, &error);
  if (!maybe_weight_props.hasValue()) {
    return 0;  // No weights? No weight bits...
  }

  const auto weight_props = maybe_weight_props.value();

  // Figure out the number of weights
  int num_weights = weight_props.width * weight_props.height;
  if (DecodeDualPlaneBit(astc_bits)) {
    num_weights *= 2;
  }

  // The number of bits is determined by the number of values
  // that are going to be encoded using the given ise_counts.
  return IntegerSequenceCodec::GetBitCountForRange(
      num_weights, weight_props.range);
}

// Returns the number of bits after the weight data used to
// store additional CEM bits.
int DecodeNumExtraCEMBits(const base::UInt128 astc_bits) {
  const int num_partitions = DecodeNumPartitions(astc_bits);

  // Do we only have one partition?
  if (num_partitions == 1) {
    return 0;
  }

  // Do we have a shared CEM?
  constexpr int kSharedCEMBitPosition = 23;
  constexpr int kSharedCEMBitLength = 2;
  const base::UInt128 shared_cem =
      base::GetBits(astc_bits, kSharedCEMBitPosition, kSharedCEMBitLength);
  if (shared_cem == 0) {
    return 0;
  }

  const std::array<int, 4> extra_cem_bits_for_partition = {{ 0, 2, 5, 8 }};
  return extra_cem_bits_for_partition[num_partitions - 1];
}

// Returns the starting position of the dual plane channel. This comes
// before the weight data and extra CEM bits.
int DecodeDualPlaneBitStartPos(const base::UInt128 astc_bits) {
  const int start_pos = kASTCBlockSizeBits
      - DecodeNumWeightBits(astc_bits)
      - DecodeNumExtraCEMBits(astc_bits);

  if (DecodeDualPlaneBit(astc_bits)) {
    return start_pos - 2;
  } else {
    return start_pos;
  }
}

// Decodes a CEM mode based on the partition number.
ColorEndpointMode DecodeEndpointMode(const base::UInt128 astc_bits,
                                     int partition) {
  int num_partitions = DecodeNumPartitions(astc_bits);
  assert(partition >= 0);
  assert(partition < num_partitions);

  // Do we only have one partition?
  uint64_t low_bits = astc_bits.LowBits();
  if (num_partitions == 1) {
    uint64_t cem = base::GetBits(low_bits, 13, 4);
    return static_cast<ColorEndpointMode>(cem);
  }

  // More than one partition ... do we have a shared CEM?
  if (DecodeNumExtraCEMBits(astc_bits) == 0) {
    const uint64_t shared_cem = base::GetBits(low_bits, 25, 4);
    return static_cast<ColorEndpointMode>(shared_cem);
  }

  // More than one partition and no shared CEM...
  uint64_t cem = base::GetBits(low_bits, 23, 6);
  const int base_cem = static_cast<int>(((cem & 0x3) - 1) * 4);
  cem >>= 2;  // Skip the base CEM bits

  // The number of extra CEM bits at the end of the weight grid is
  // determined by the number of partitions and what the base cem mode is...
  const int num_extra_cem_bits = DecodeNumExtraCEMBits(astc_bits);
  const int extra_cem_start_pos = kASTCBlockSizeBits
      - num_extra_cem_bits
      - DecodeNumWeightBits(astc_bits);

  base::UInt128 extra_cem =
      base::GetBits(astc_bits, extra_cem_start_pos, num_extra_cem_bits);
  cem |= extra_cem.LowBits() << 4;

  // Decode C and M per Figure C.4
  int c = -1, m = -1;
  for (int i = 0; i < num_partitions; ++i) {
    if (i == partition) {
      c = cem & 0x1;
    }
    cem >>= 1;
  }

  for (int i = 0; i < num_partitions; ++i) {
    if (i == partition) {
      m = cem & 0x3;
    }
    cem >>= 2;
  }

  assert(c >= 0);
  assert(m >= 0);

  // Compute the mode based on C and M
  const int mode = base_cem + 4 * c + m;
  assert(mode < static_cast<int>(ColorEndpointMode::kNumColorEndpointModes));
  return static_cast<ColorEndpointMode>(mode);
}

int DecodeNumColorValues(const base::UInt128 astc_bits) {
  int num_color_values = 0;
  auto num_partitions = DecodeNumPartitions(astc_bits);
  for (int i = 0; i < num_partitions; ++i) {
    ColorEndpointMode endpoint_mode = DecodeEndpointMode(astc_bits, i);
    num_color_values += NumColorValuesForEndpointMode(endpoint_mode);
  }

  return num_color_values;
}

}  // namespace

////////////////////////////////////////////////////////////////////////////////

static_assert(sizeof(PhysicalASTCBlock) == PhysicalASTCBlock::kSizeInBytes,
              "The size of the struct should be the size of the block so that"
              "we can effectively use them contiguously in memory.");

PhysicalASTCBlock::PhysicalASTCBlock(const base::UInt128 astc_block)
    : astc_bits_(astc_block) {}

PhysicalASTCBlock::PhysicalASTCBlock(const std::string& encoded_block)
    : astc_bits_([&encoded_block]() {
        assert(encoded_block.size() == PhysicalASTCBlock::kSizeInBytes);
        base::UInt128 astc_bits = 0;
        int shift = 0;
        for (const unsigned char c : encoded_block) {
          astc_bits |= base::UInt128(static_cast<uint64_t>(c)) << shift;
          shift += 8;
        }
        return astc_bits;
      }())
{ }

base::Optional<std::string> PhysicalASTCBlock::IsIllegalEncoding() const {
  // If the block is not a void extent block, then it must have
  // weights specified. DecodeWeightProps will return the weight specifications
  // if they exist and are legal according to C.2.24, and will otherwise be
  // empty.
  base::Optional<BlockMode> block_mode = DecodeBlockMode(astc_bits_);
  if (block_mode != BlockMode::kVoidExtent) {
    std::string error;
    auto maybe_weight_props = DecodeWeightProps(astc_bits_, &error);
    if (!maybe_weight_props.hasValue()) {
      return error;
    }
  }

  // Check void extent blocks...
  if (block_mode == BlockMode::kVoidExtent) {
    // ... for reserved bits incorrectly set
    if (base::GetBits(astc_bits_, 10, 2) != 0x3) {
      return std::string("Reserved bits set for void extent block");
    }

    // ... for incorrectly defined texture coordinates
    std::array<int, 4> coords = DecodeVoidExtentCoords(astc_bits_);

    bool coords_all_1s = true;
    for (const auto coord : coords) {
      coords_all_1s &= coord == ((1 << 13) - 1);
    }

    if (!coords_all_1s && (coords[0] >= coords[1] || coords[2] >= coords[3])) {
      return std::string("Void extent texture coordinates are invalid");
    }
  }

  // If the number of color values exceeds a threshold and it isn't a void
  // extent block then we've run into an error
  if (block_mode != BlockMode::kVoidExtent) {
    int num_color_vals = DecodeNumColorValues(astc_bits_);
    if (num_color_vals > 18) {
      return std::string("Too many color values");
    }

    // The maximum number of available color bits is the number of
    // bits between the dual plane bits and the base CEM. This must
    // be larger than a threshold defined in C.2.24.

    // Dual plane bit starts after weight bits and CEM
    const int num_partitions = DecodeNumPartitions(astc_bits_);
    const int dual_plane_start_pos = DecodeDualPlaneBitStartPos(astc_bits_);
    const int color_start_bit = (num_partitions == 1) ? 17 : 29;

    const int required_color_bits = ((13 * num_color_vals) + 4) / 5;
    const int available_color_bits = dual_plane_start_pos - color_start_bit;
    if (available_color_bits < required_color_bits) {
      return std::string("Not enough color bits");
    }

    // If we have four partitions and a dual plane then we have a problem.
    if (num_partitions == 4 && DecodeDualPlaneBit(astc_bits_)) {
      return std::string("Both four partitions and dual plane specified");
    }
  }

  // Otherwise we're OK
  return { };
}

bool PhysicalASTCBlock::IsVoidExtent() const {
  // If it's an error block, it's not a void extent block.
  if (IsIllegalEncoding()) {
    return false;
  }

  return DecodeBlockMode(astc_bits_) == BlockMode::kVoidExtent;
}

base::Optional<std::array<int, 4>> PhysicalASTCBlock::VoidExtentCoords() const {
  if (IsIllegalEncoding() || !IsVoidExtent()) {
    return { };
  }

  // If void extent coords are all 1's then these are not valid void extent
  // coords
  const uint64_t ve_mask = 0xFFFFFFFFFFFFFDFFULL;
  const uint64_t const_blk_mode = 0xFFFFFFFFFFFFFDFCULL;
  if ((ve_mask & astc_bits_.LowBits()) == const_blk_mode) {
    return {};
  }

  return DecodeVoidExtentCoords(astc_bits_);
}

bool PhysicalASTCBlock::IsDualPlane() const {
  // If it's an error block, then we aren't a dual plane block
  if (IsIllegalEncoding()) {
    return false;
  }

  return DecodeDualPlaneBit(astc_bits_);
}

// Returns the number of weight bits present in this block
base::Optional<int> PhysicalASTCBlock::NumWeightBits() const {
  // If it's an error block, then we have no weight bits.
  if (IsIllegalEncoding()) return { };

  // If it's a void extent block, we have no weight bits
  if (IsVoidExtent()) return { };

  return DecodeNumWeightBits(astc_bits_);
}

base::Optional<int> PhysicalASTCBlock::WeightStartBit() const {
  if (IsIllegalEncoding()) return { };
  if (IsVoidExtent()) return { };

  return kASTCBlockSizeBits - DecodeNumWeightBits(astc_bits_);
}

base::Optional<std::array<int, 2>> PhysicalASTCBlock::WeightGridDims() const {
  std::string error;
  auto weight_props = DecodeWeightProps(astc_bits_, &error);

  if (!weight_props.hasValue()) return { };
  if (IsIllegalEncoding()) return { };

  const auto props = weight_props.value();
  return {{{ props.width, props.height }}};
}

base::Optional<int> PhysicalASTCBlock::WeightRange() const {
  std::string error;
  auto weight_props = DecodeWeightProps(astc_bits_, &error);

  if (!weight_props.hasValue()) return { };
  if (IsIllegalEncoding()) return { };

  return weight_props.value().range;
}

base::Optional<int> PhysicalASTCBlock::DualPlaneChannel() const {
  if (!IsDualPlane()) return { };

  int dual_plane_start_pos = DecodeDualPlaneBitStartPos(astc_bits_);
  auto plane_bits = base::GetBits(astc_bits_, dual_plane_start_pos, 2);
  return base::Optional<int>(static_cast<int>(plane_bits.LowBits()));
}

base::Optional<int> PhysicalASTCBlock::ColorStartBit() const {
  if (IsVoidExtent()) {
    return 64;
  }

  auto num_partitions = NumPartitions();
  if (!num_partitions) return { };

  return (num_partitions == 1) ? 17 : 29;
}

base::Optional<int> PhysicalASTCBlock::NumColorValues() const {
  // If we have a void extent block, then we have four color values
  if (IsVoidExtent()) {
    return 4;
  }

  // If we have an illegal encoding, then we have no color values
  if (IsIllegalEncoding()) return { };

  return DecodeNumColorValues(astc_bits_);
}

void PhysicalASTCBlock::GetColorValuesInfo(int* const color_bits,
                                           int* const color_range) const {
  // Figure out the range possible for the number of values we have...
  const int dual_plane_start_pos = DecodeDualPlaneBitStartPos(astc_bits_);
  const int max_color_bits = dual_plane_start_pos - ColorStartBit().value();
  const int num_color_values = NumColorValues().value();
  for (int range = 255; range > 0; --range) {
    const int bitcount =
        IntegerSequenceCodec::GetBitCountForRange(num_color_values, range);
    if (bitcount <= max_color_bits) {
      if (color_bits != nullptr) {
        *color_bits = bitcount;
      }

      if (color_range != nullptr) {
        *color_range = range;
      }
      return;
    }
  }

  assert(false &&
         "This means that even if we have a range of one there aren't "
         "enough bits to store the color values, and our encoding is "
         "illegal.");
}

base::Optional<int> PhysicalASTCBlock::NumColorBits() const {
  if (IsIllegalEncoding()) return { };

  if (IsVoidExtent()) {
    return 64;
  }

  int color_bits;
  GetColorValuesInfo(&color_bits, nullptr);
  return color_bits;
}

base::Optional<int> PhysicalASTCBlock::ColorValuesRange() const {
  if (IsIllegalEncoding()) return { };

  if (IsVoidExtent()) {
    return (1 << 16) - 1;
  }

  int color_range;
  GetColorValuesInfo(nullptr, &color_range);
  return color_range;
}

base::Optional<int> PhysicalASTCBlock::NumPartitions() const {
  // Error blocks have no partitions
  if (IsIllegalEncoding()) return { };

  // Void extent blocks have no partitions either
  if (DecodeBlockMode(astc_bits_) == BlockMode::kVoidExtent) {
    return { };
  }

  // All others have some number of partitions
  return DecodeNumPartitions(astc_bits_);
}

base::Optional<int> PhysicalASTCBlock::PartitionID() const {
  auto num_partitions = NumPartitions();
  if (!num_partitions || num_partitions == 1) return { };

  const uint64_t low_bits = astc_bits_.LowBits();
  return static_cast<int>(base::GetBits(low_bits, 13, 10));
}

base::Optional<ColorEndpointMode> PhysicalASTCBlock::GetEndpointMode(
    int partition) const {
  // Error block?
  if (IsIllegalEncoding()) return { };

  // Void extent blocks have no endpoint modes
  if (DecodeBlockMode(astc_bits_) == BlockMode::kVoidExtent) {
    return { };
  }

  // Do we even have a CEM for this partition?
  if (partition < 0 || DecodeNumPartitions(astc_bits_) <= partition) {
    return { };
  }

  return DecodeEndpointMode(astc_bits_, partition);
}

}  // namespace astc_codec
