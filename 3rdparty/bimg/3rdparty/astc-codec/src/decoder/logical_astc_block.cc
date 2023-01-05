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

#include "src/decoder/logical_astc_block.h"
#include "src/decoder/endpoint_codec.h"
#include "src/decoder/footprint.h"
#include "src/decoder/integer_sequence_codec.h"
#include "src/decoder/quantization.h"
#include "src/decoder/weight_infill.h"

namespace astc_codec {

namespace {

Partition GenerateSinglePartition(Footprint footprint) {
  return Partition { footprint, /* num_parts = */ 1, /* partition_id = */ 0,
        std::vector<int>(footprint.NumPixels(), 0) };
}

static std::vector<EndpointPair> DecodeEndpoints(const IntermediateBlockData& block) {
  const int endpoint_range = block.endpoint_range
      ? block.endpoint_range.value()
      : EndpointRangeForBlock(block);
  assert(endpoint_range > 0);

  std::vector<EndpointPair> endpoints;
  for (const auto& eps : block.endpoints) {
    RgbaColor decmp_one_rgba, decmp_two_rgba;
    DecodeColorsForMode(eps.colors, endpoint_range, eps.mode,
                        &decmp_one_rgba, &decmp_two_rgba);
    endpoints.emplace_back(decmp_one_rgba, decmp_two_rgba);
  }
  return endpoints;
}

static std::vector<EndpointPair> DecodeEndpoints(const VoidExtentData& block) {
  EndpointPair eps;
  eps.first[0] = eps.second[0] = (block.r * 255) / 65535;
  eps.first[1] = eps.second[1] = (block.g * 255) / 65535;
  eps.first[2] = eps.second[2] = (block.b * 255) / 65535;
  eps.first[3] = eps.second[3] = (block.a * 255) / 65535;

  std::vector<EndpointPair> endpoints;
  endpoints.emplace_back(eps);
  return endpoints;
}

Partition ComputePartition(const Footprint& footprint,
                           const IntermediateBlockData& block) {
  if (block.partition_id) {
    const int part_id = block.partition_id.value();
    const int num_parts = int(block.endpoints.size());
    return GetASTCPartition(footprint, num_parts, part_id);
  } else {
    return GenerateSinglePartition(footprint);
  }
}

Partition ComputePartition(const Footprint& footprint, const VoidExtentData&) {
  return GenerateSinglePartition(footprint);
}

}  // namespace

////////////////////////////////////////////////////////////////////////////////

LogicalASTCBlock::LogicalASTCBlock(const Footprint& footprint)
    : endpoints_(1),
      weights_(footprint.NumPixels(), 0),
      partition_(GenerateSinglePartition(footprint)) { }

LogicalASTCBlock::LogicalASTCBlock(const Footprint& footprint,
                                   const IntermediateBlockData& block)
    : endpoints_(DecodeEndpoints(block)),
      partition_(ComputePartition(footprint, block)) {
  CalculateWeights(footprint, block);
}

LogicalASTCBlock::LogicalASTCBlock(const Footprint& footprint,
                                   const VoidExtentData& block)
    : endpoints_(DecodeEndpoints(block)),
      partition_(ComputePartition(footprint, block)) {
  CalculateWeights(footprint, block);
}

void LogicalASTCBlock::CalculateWeights(const Footprint& footprint,
                                        const IntermediateBlockData& block) {
  const int grid_size_x = block.weight_grid_dim_x;
  const int grid_size_y = block.weight_grid_dim_y;
  const int weight_grid_size = grid_size_x * grid_size_y;

  // Either we have a dual plane and we have twice as many weights as
  // specified or we don't
  assert(block.dual_plane_channel
        ? block.weights.size() == size_t(weight_grid_size * 2)
        : block.weights.size() == size_t(weight_grid_size));

  std::vector<int> unquantized;
  unquantized.reserve(weight_grid_size);

  // According to C.2.16, if we have dual-plane weights, then we have two
  // weights per texel -- one adjacent to the other. Hence, we have to skip
  // some when we decode the separate weight values.
  const int weight_frequency = (block.dual_plane_channel) ? 2 : 1;
  const int weight_range = block.weight_range;

  for (int i = 0; i < weight_grid_size; ++i) {
    const int weight = block.weights[i * weight_frequency];
    unquantized.push_back(UnquantizeWeightFromRange(weight, weight_range));
  }
  weights_ = InfillWeights(unquantized, footprint, grid_size_x, grid_size_y);

  if (block.dual_plane_channel) {
    SetDualPlaneChannel(block.dual_plane_channel.value());
    for (int i = 0; i < weight_grid_size; ++i) {
      const int weight = block.weights[i * weight_frequency + 1];
      unquantized[i] = UnquantizeWeightFromRange(weight, weight_range);
    }
    dual_plane_->weights =
        InfillWeights(unquantized, footprint, grid_size_x, grid_size_y);
  }
}

void LogicalASTCBlock::CalculateWeights(const Footprint& footprint,
                                        const VoidExtentData&) {
  weights_ = std::vector<int>(footprint.NumPixels(), 0);
}

void LogicalASTCBlock::SetWeightAt(int x, int y, int weight) {
  assert(weight >= 0);
  assert(weight <= 64);
  weights_.at(y * GetFootprint().Width() + x) = weight;
}

int LogicalASTCBlock::WeightAt(int x, int y) const {
  return weights_.at(y * GetFootprint().Width() + x);
}

void LogicalASTCBlock::SetDualPlaneWeightAt(int channel, int x, int y,
                                            int weight) {
  assert(weight >= 0);
  assert(weight <= 64);

  // If it's not a dual plane, then this has no logical meaning
  assert(IsDualPlane());

  // If it is a dual plane and the passed channel matches the query, then we
  // return the specialized weights
  if (dual_plane_->channel == channel) {
    dual_plane_->weights.at(y * GetFootprint().Width() + x) = weight;
  } else {
    // If the channel is not the special channel, then return the general weight
    SetWeightAt(x, y, weight);
  }
}

int LogicalASTCBlock::DualPlaneWeightAt(int channel, int x, int y) const {
  // If it's not a dual plane, then we just return the weight for all channels
  if (!IsDualPlane()) {
    // TODO(google): Log warning, Requesting dual-plane channel for a non
    // dual-plane block!
    return WeightAt(x, y);
  }

  // If it is a dual plane and the passed channel matches the query, then we
  // return the specialized weights
  if (dual_plane_->channel == channel) {
    return dual_plane_->weights.at(y * GetFootprint().Width() + x);
  }

  // If the channel is not the special channel, then return the general weight
  return WeightAt(x, y);
}

void LogicalASTCBlock::SetDualPlaneChannel(int channel) {
  if (channel < 0) {
    dual_plane_.clear();
  } else if (dual_plane_) {
    dual_plane_->channel = channel;
  } else {
    dual_plane_ = DualPlaneData {channel, weights_};
  }
}

RgbaColor LogicalASTCBlock::ColorAt(int x, int y) const {
  const auto footprint = GetFootprint();
  assert(x >= 0);  assert(x < footprint.Width());
  assert(y >= 0);  assert(y < footprint.Height());

  const int texel_idx = y * footprint.Width() + x;
  const int part = partition_.assignment[texel_idx];
  const auto& endpoints = endpoints_[part];

  RgbaColor result;
  for (int channel = 0; channel < 4; ++channel) {
    const int weight = (dual_plane_ && dual_plane_->channel == channel)
        ? dual_plane_->weights[texel_idx]
        : weights_[texel_idx];
    const int p0 = endpoints.first[channel];
    const int p1 = endpoints.second[channel];

    assert(p0 >= 0); assert(p0 < 256);
    assert(p1 >= 0); assert(p1 < 256);

    // According to C.2.19
    const int c0 = (p0 << 8) | p0;
    const int c1 = (p1 << 8) | p1;
    const int c = (c0 * (64 - weight) + c1 * weight + 32) / 64;
    // TODO(google): Handle conversion to sRGB or FP16 per C.2.19.
    const int quantized = ((c * 255) + 32767) / 65536;
    assert(quantized < 256);
    result[channel] = quantized;
  }

  return result;
}

void LogicalASTCBlock::SetPartition(const Partition& p) {
  assert(p.footprint == partition_.footprint &&
         "New partitions may not be for a different footprint");
  partition_ = p;
  endpoints_.resize(p.num_parts);
}

void LogicalASTCBlock::SetEndpoints(const EndpointPair& eps, int subset) {
  assert(subset < partition_.num_parts);
  assert(size_t(subset) < endpoints_.size());

  endpoints_[subset] = eps;
}

base::Optional<LogicalASTCBlock> UnpackLogicalBlock(
    const Footprint& footprint, const PhysicalASTCBlock& pb) {
  if (pb.IsVoidExtent()) {
    base::Optional<VoidExtentData> ve = UnpackVoidExtent(pb);
    if (!ve) {
      return {};
    }

    return LogicalASTCBlock(footprint, ve.value());
  } else {
    base::Optional<IntermediateBlockData> ib = UnpackIntermediateBlock(pb);
    if (!ib) {
      return {};
    }

    return LogicalASTCBlock(footprint, ib.value());
  }
}

}  // namespace astc_codec
