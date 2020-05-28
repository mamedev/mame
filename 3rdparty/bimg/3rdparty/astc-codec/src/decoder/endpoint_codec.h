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

#ifndef ASTC_CODEC_DECODER_ENDPOINT_CODEC_H_
#define ASTC_CODEC_DECODER_ENDPOINT_CODEC_H_

#include "src/decoder/physical_astc_block.h"
#include "src/decoder/types.h"

#include <array>
#include <vector>

namespace astc_codec {

// We use a special distinction for encode modes used to pass to the
// EncodeColorsForMode function below. The reason is that some of the color
// modes have sub-modes (like blue-contract) that change whether or not it is
// useful to encode an endpoint pair using one mode versus another. To avoid
// this problem, we approach the problem of encoding by specifying some
// high-level encoding modes. These eventually choose one of the low level
// ColorEndpointModes from Section C.2.14 when used in EncodeColorsForMode.
enum class EndpointEncodingMode {
  kDirectLuma,
  kDirectLumaAlpha,
  kBaseScaleRGB,
  kBaseScaleRGBA,
  kDirectRGB,
  kDirectRGBA
};

// Returns the number of values in the encoded endpoint pair after encoding
// to a specific high-level encoding mode.
constexpr int NumValuesForEncodingMode(EndpointEncodingMode mode) {
  return
      mode == EndpointEncodingMode::kDirectLuma ? 2 :
      mode == EndpointEncodingMode::kDirectLumaAlpha ? 4 :
      mode == EndpointEncodingMode::kBaseScaleRGB ? 4 :
      mode == EndpointEncodingMode::kBaseScaleRGBA ? 6 :
      mode == EndpointEncodingMode::kDirectRGB ? 6 : 8;
}

// Fills |vals| with the quantized endpoint colors values defined in the ASTC
// specification. The values are quantized to the range [0, max_value]. These
// quantization limits can be obtained by querying the associated functions in
// integer_sequence_codec. The returned |astc_mode| will be the ASTC mode used
// to encode the resulting sequence.
//
// The |encoding_mode| is used to determine the way that we encode the values.
// Each encoding mode is used to determine which ASTC mode best corresponds
// to the pair of endpoints. It is a necessary hint to the encoding function
// in order to process the endpoints. Each encoding mode gurantees a certain
// number of values generated per endpoints.
//
// The return value will be true if the endpoints have been switched in order to
// reap the most benefit from the way the hardware decodes the given mode. In
// this case, the associated weights that interpolate this color must also be
// switched. In other words, for each w, it should change to 64 - w.
bool EncodeColorsForMode(
    const RgbaColor& endpoint_low_rgba, const RgbaColor& endpoint_high_rgba,
    int max_value, EndpointEncodingMode encoding_mode,
    ColorEndpointMode* astc_mode, std::vector<int>* vals);

// Decodes the color values quantized to the range [0, max_value] into RGBA
// endpoints for the given mode. This function is the inverse of
// EncodeColorsForMode -- see that function for details. This function should
// work on all LDR endpoint modes, but no HDR modes.
void DecodeColorsForMode(const std::vector<int>& vals,
                         int max_value, ColorEndpointMode mode,
                         RgbaColor* endpoint_low_rgba,
                         RgbaColor* endpoint_high_rgba);

// Returns true if the quantized |vals| in the range [0, max_value] use the
// 'blue_contract' modification during decoding for the given |mode|.
bool UsesBlueContract(int max_value, ColorEndpointMode mode,
                      const std::vector<int>& vals);

}  // namespace astc_codec

#endif  // ASTC_CODEC_DECODER_ENDPOINT_CODEC_H_
