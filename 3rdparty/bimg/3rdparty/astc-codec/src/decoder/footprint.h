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

#ifndef ASTC_CODEC_DECODER_FOOTPRINT_H_
#define ASTC_CODEC_DECODER_FOOTPRINT_H_

#include "include/astc-codec/astc-codec.h"
#include "src/base/optional.h"

#include <cstddef>

namespace astc_codec {

// An ASTC texture can be encoded with varying choices in block size. A set of
// predefined block sizes are specified in the ASTC specification. These are
// referred to in the literature as "footprints" available to an encoder when
// constructing an ASTC bitstream. This class provides various utility functions
// for interacting with these footprints.
class Footprint {
 public:
  Footprint() = delete;
  Footprint(const Footprint& footprint) = default;

  // Return the footprint type.
  FootprintType Type() const { return footprint_; }

  // Return logical descriptions of the dimensions.
  int Width() const { return width_; }
  int Height() const { return height_; }

  // Returns the number of pixels for a block with this footprint.
  int NumPixels() const { return width_ * height_; }

  // Returns the number of bytes needed to store an ASTC encoded image with the
  // given width and height.
  size_t StorageRequirements(int width, int height) const;

  // Returns the number of bits used per pixel.
  float Bitrate() const;

  static constexpr int NumValidFootprints() {
    return static_cast<int>(FootprintType::kCount);
  }

  bool operator==(const Footprint& other) const {
    return footprint_ == other.footprint_;
  }

  // These are the valid and available ASTC footprints.
  static Footprint Get4x4() { return Footprint(FootprintType::k4x4); }
  static Footprint Get5x4() { return Footprint(FootprintType::k5x4); }
  static Footprint Get5x5() { return Footprint(FootprintType::k5x5); }
  static Footprint Get6x5() { return Footprint(FootprintType::k6x5); }
  static Footprint Get6x6() { return Footprint(FootprintType::k6x6); }
  static Footprint Get8x5() { return Footprint(FootprintType::k8x5); }
  static Footprint Get8x6() { return Footprint(FootprintType::k8x6); }
  static Footprint Get8x8() { return Footprint(FootprintType::k8x8); }
  static Footprint Get10x5() { return Footprint(FootprintType::k10x5); }
  static Footprint Get10x6() { return Footprint(FootprintType::k10x6); }
  static Footprint Get10x8() { return Footprint(FootprintType::k10x8); }
  static Footprint Get10x10() { return Footprint(FootprintType::k10x10); }
  static Footprint Get12x10() { return Footprint(FootprintType::k12x10); }
  static Footprint Get12x12() { return Footprint(FootprintType::k12x12); }

  // Constructs a footprint from a string of the form "NxM", or no value if
  // width and height are not a valid footprint.
  static base::Optional<Footprint> Parse(const char* footprint_string);

  // Returns a footprint corresponding to a block of the given width and height,
  // or no value if it does not.
  static base::Optional<Footprint> FromDimensions(int width, int height);

  // Returns a Footprint for the given FootprintType.
  static base::Optional<Footprint> FromFootprintType(FootprintType type);

 private:
  // The only constructor.
  explicit Footprint(FootprintType footprint);

  // Returns the valid footprint for the width and height if possible.
  static base::Optional<FootprintType> GetValidFootprintForDimensions(
      int width, int height);

  // Returns the associated dimension for the given valid footprint.
  static int GetWidthForFootprint(FootprintType footprint);
  static int GetHeightForFootprint(FootprintType footprint);

  FootprintType footprint_;
  int width_;
  int height_;
};

}  // namespace astc_codec

#endif  // ASTC_CODEC_DECODER_FOOTPRINT_H_
