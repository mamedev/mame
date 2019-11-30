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

#ifndef ASTC_CODEC_DECODER_QUANTIZATION_H_
#define ASTC_CODEC_DECODER_QUANTIZATION_H_

////////////////////////////////////////////////////////////////////////////////
//
// ASTC Quantization procedures.
//
// The values stored in ASTC blocks tend to be stored in a range much more
// restricted than the logical range used. For example, sometimes weights are
// stored in the range from [0, 3] but are used in the range [0, 64]. The
// process of translating a value to or from this range is known as quantization
// and dequantization. The ranges to which these values can be (de)quantized
// are defined by ISERange[Begin|End]() in integer_sequence_codec.h

namespace astc_codec {

// The minimum possible range for a pair of endpoints. If endpoints are
// quantized to something smaller than this, then it would constitute an
// illegal ASTC encoding.
constexpr int kEndpointRangeMinValue = 5;

// The maximum possible range for a weight value. If weights are quantized to
// something larger than this, then it would constitute an illegal ASTC
// encoding.
constexpr int kWeightRangeMaxValue = 31;

// Quantizes a value in the range [0, 255] to [0, |range|]. The quantized values
// have no correlation to the input values, and there should be no implicit
// assumptions made about their ordering. Valid values of |range_max_value| are
// in the interval [5, 255]
int QuantizeCEValueToRange(int value, int range_max_value);

// Unquantizes a value in the range [0, |range|] to [0, 255]. Performs the
// inverse procedure of QuantizeValueToRange. Valid values of |range_max_value|
// are in the interval [5, 255]
int UnquantizeCEValueFromRange(int value, int range_max_value);

// Quantizes a weight in the range [0, 64] to [0, |range_max_value|]. The
// quantized values have no correlation to the input values, and there should
// be no implicit assumptions made about their ordering. Valid values of
// |range_max_value| are in the interval [1, 31]
int QuantizeWeightToRange(int weight, int range_max_value);

// Unquantizes a weight in the range [0, |range_max_value|] to [0, 64]. Performs
// the inverse procedure of QuantizeWeightToRange. Valid values of
// |range_max_value| are in the interval [1, 31]
int UnquantizeWeightFromRange(int weight, int range_max_value);

}  // namespace astc_codec

#endif  // ASTC_CODEC_DECODER_QUANTIZATION_H_
