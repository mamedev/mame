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

#include "src/decoder/footprint.h"
#include "src/base/string_utils.h"

#include <map>
#include <string>
#include <utility>
#include <vector>

namespace astc_codec {

namespace {

// Encodes the width and height into an integer so that we can use a switch
// statement instead of a costly lookup map.
constexpr int EncodeDims(int width, int height) {
  return (width << 16) | height;
}

}  // namespace

base::Optional<FootprintType>
Footprint::GetValidFootprintForDimensions(int width, int height) {
  switch (EncodeDims(width, height)) {
    case EncodeDims(4, 4): return FootprintType::k4x4;
    case EncodeDims(5, 4): return FootprintType::k5x4;
    case EncodeDims(5, 5): return FootprintType::k5x5;
    case EncodeDims(6, 5): return FootprintType::k6x5;
    case EncodeDims(6, 6): return FootprintType::k6x6;
    case EncodeDims(8, 5): return FootprintType::k8x5;
    case EncodeDims(8, 6): return FootprintType::k8x6;
    case EncodeDims(8, 8): return FootprintType::k8x8;
    case EncodeDims(10, 5): return FootprintType::k10x5;
    case EncodeDims(10, 6): return FootprintType::k10x6;
    case EncodeDims(10, 8): return FootprintType::k10x8;
    case EncodeDims(10, 10): return FootprintType::k10x10;
    case EncodeDims(12, 10): return FootprintType::k12x10;
    case EncodeDims(12, 12): return FootprintType::k12x12;
    default:                 return {};
  }
}

int Footprint::GetWidthForFootprint(FootprintType footprint) {
  switch (footprint) {
    case FootprintType::k4x4: return 4;
    case FootprintType::k5x4: return 5;
    case FootprintType::k5x5: return 5;
    case FootprintType::k6x5: return 6;
    case FootprintType::k6x6: return 6;
    case FootprintType::k8x5: return 8;
    case FootprintType::k8x6: return 8;
    case FootprintType::k10x5: return 10;
    case FootprintType::k10x6: return 10;
    case FootprintType::k8x8: return 8;
    case FootprintType::k10x8: return 10;
    case FootprintType::k10x10: return 10;
    case FootprintType::k12x10: return 12;
    case FootprintType::k12x12: return 12;
    default:
      assert(false);
      return -1;
  }
}

int Footprint::GetHeightForFootprint(FootprintType footprint) {
  switch (footprint) {
    case FootprintType::k4x4: return 4;
    case FootprintType::k5x4: return 4;
    case FootprintType::k5x5: return 5;
    case FootprintType::k6x5: return 5;
    case FootprintType::k6x6: return 6;
    case FootprintType::k8x5: return 5;
    case FootprintType::k8x6: return 6;
    case FootprintType::k10x5: return 5;
    case FootprintType::k10x6: return 6;
    case FootprintType::k8x8: return 8;
    case FootprintType::k10x8: return 8;
    case FootprintType::k10x10: return 10;
    case FootprintType::k12x10: return 10;
    case FootprintType::k12x12: return 12;
    default:
      assert(false);
      return -1;
  }
}

Footprint::Footprint(FootprintType footprint)
    : footprint_(footprint), width_(GetWidthForFootprint(footprint)),
      height_(GetHeightForFootprint(footprint)) { }

////////////////////////////////////////////////////////////////////////////////

base::Optional<Footprint> Footprint::Parse(const char* footprint_string) {
  assert(footprint_string && footprint_string[0] != '\0');

  std::vector<std::string> dimension_strings;
  base::Split(footprint_string, "x", [&dimension_strings](std::string&& s) {
    dimension_strings.push_back(std::move(s));
  });

  if (dimension_strings.size() != 2) {
    assert(false && "Invalid format for footprint");
    return {};
  }

  const int width = base::ParseInt32(dimension_strings[0].c_str(), 0);
  const int height = base::ParseInt32(dimension_strings[1].c_str(), 0);

  assert(width > 0 && height > 0 && "Invalid width or height.");

  return FromDimensions(width, height);
}

base::Optional<Footprint> Footprint::FromDimensions(int width, int height) {
  base::Optional<FootprintType> valid_footprint =
      GetValidFootprintForDimensions(width, height);
  if (valid_footprint) {
    return Footprint(valid_footprint.value());
  } else {
    return {};
  }
}

// Returns a Footprint for the given FootprintType.
base::Optional<Footprint> Footprint::FromFootprintType(FootprintType type) {
  if (type >= FootprintType::k4x4 && type < FootprintType::kCount) {
    return Footprint(type);
  } else {
    return {};
  }
}

size_t Footprint::StorageRequirements(int width, int height) const {
  const int blocks_wide = (width + width_ - 1) / width_;
  const int blocks_high = (height + height_ - 1) / height_;

  constexpr size_t kASTCBlockSizeInBytes = 16;
  return blocks_wide * blocks_high * kASTCBlockSizeInBytes;
}

// Returns bits/pixel for a given footprint.
float Footprint::Bitrate() const {
  const int kASTCBlockBitCount = 128;
  const int footprint_pixel_count = width_ * height_;
  return static_cast<float>(kASTCBlockBitCount) /
         static_cast<float>(footprint_pixel_count);
}

}  // namespace astc_codec
