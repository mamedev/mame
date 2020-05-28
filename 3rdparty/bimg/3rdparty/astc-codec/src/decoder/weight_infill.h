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

#ifndef ASTC_CODEC_DECODER_WEIGHT_INFILL_H_
#define ASTC_CODEC_DECODER_WEIGHT_INFILL_H_

#include "src/decoder/footprint.h"

#include <vector>

namespace astc_codec {

// Returns the number of bits used to represent the weight grid at the target
// dimensions and weight range.
int CountBitsForWeights(int weight_dim_x, int weight_dim_y,
                        int target_weight_range);

// Performs weight infill of a grid of weights of size |dim_x * dim_y|. The
// weights are fit using the algorithm laid out in Section C.2.18 of the ASTC
// specification. Weights are expected to be passed unquantized and the returned
// grid will be unquantized as well (i.e. each weight within the range [0, 64]).
std::vector<int> InfillWeights(const std::vector<int>& weights,
                               Footprint footprint, int dim_x, int dim_y);

}  // namespace astc_codec

#endif  // ASTC_CODEC_DECODER_WEIGHT_INFILL_H_
