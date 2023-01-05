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

#ifndef ASTC_CODEC_DECODER_ASTC_TYPES_H_
#define ASTC_CODEC_DECODER_ASTC_TYPES_H_

#include <array>
#include <string>
#include <utility>

namespace astc_codec {

// The color endpoint mode determines how the values encoded in the ASTC block
// are interpreted in order to create the RGBA values for the given endpoint
// pair. The order of this enum is required to match the ASTC specification in
// Section C.2.14.
enum class ColorEndpointMode {
  kLDRLumaDirect = 0,
  kLDRLumaBaseOffset,
  kHDRLumaLargeRange,
  kHDRLumaSmallRange,
  kLDRLumaAlphaDirect,
  kLDRLumaAlphaBaseOffset,
  kLDRRGBBaseScale,
  kHDRRGBBaseScale,
  kLDRRGBDirect,
  kLDRRGBBaseOffset,
  kLDRRGBBaseScaleTwoA,
  kHDRRGBDirect,
  kLDRRGBADirect,
  kLDRRGBABaseOffset,
  kHDRRGBDirectLDRAlpha,
  kHDRRGBDirectHDRAlpha,

  // The total number of color endpoints defined by the ASTC specification.
  // This isn't a specific endpoint mode and its sole purpose is to be used
  // as a constant number.
  kNumColorEndpointModes
};

// Returns the class for the given mode as defined in Section C.2.11.
constexpr int EndpointModeClass(ColorEndpointMode mode) {
  return static_cast<int>(mode) / 4;
}

// Returns the number of encoded color values for the given endpoint mode. The
// number of encoded color values and their range determines the size of the
// color data in a physical ASTC block. This information is taken from
// Section C.2.17 of the ASTC specification.
constexpr int NumColorValuesForEndpointMode(ColorEndpointMode mode) {
  return (EndpointModeClass(mode) + 1) * 2;
}

// We define a number of convenience types here that give more logical meaning
// throughout the ASTC utilities.
using RgbColor = std::array<int, 3>;
using RgbaColor = std::array<int, 4>;
using Endpoint = RgbaColor;
using EndpointPair = std::pair<Endpoint, Endpoint>;

}  // namespace astc_codec

#endif  // ASTC_CODEC_DECODER_ASTC_TYPES_H_
