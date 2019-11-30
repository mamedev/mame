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

// astc_inspector_cli collects the various statistics of a stream of ASTC data
// stored in an ASTC file.
//
// Example usage:
//   To dump statistics about an ASTC file, use:
//     astc_inspector_cli <filename>
//
//   To dump statistics on a specific block in an ASTC file, use:
//     astc_inspector_cli <filename> <number>

#include <algorithm>
#include <array>
#include <fstream>
#include <functional>
#include <iomanip>
#include <iostream>
#include <memory>
#include <numeric>
#include <sstream>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <vector>

#include "src/base/string_utils.h"
#include "src/decoder/astc_file.h"
#include "src/decoder/endpoint_codec.h"
#include "src/decoder/intermediate_astc_block.h"
#include "src/decoder/partition.h"
#include "src/decoder/quantization.h"
#include "src/decoder/weight_infill.h"

using astc_codec::ASTCFile;
using astc_codec::ColorEndpointMode;
using astc_codec::IntermediateBlockData;
using astc_codec::PhysicalASTCBlock;
using astc_codec::RgbaColor;
using astc_codec::VoidExtentData;
using astc_codec::base::Optional;

namespace {

constexpr int kNumEndpointModes =
    static_cast<int>(ColorEndpointMode::kNumColorEndpointModes);
constexpr std::array<const char*, kNumEndpointModes> kModeStrings {{
    "kLDRLumaDirect", "kLDRLumaBaseOffset", "kHDRLumaLargeRange",
    "kHDRLumaSmallRange", "kLDRLumaAlphaDirect", "kLDRLumaAlphaBaseOffset",
    "kLDRRGBBaseScale", "kHDRRGBBaseScale", "kLDRRGBDirect",
    "kLDRRGBBaseOffset", "kLDRRGBBaseScaleTwoA", "kHDRRGBDirect",
    "kLDRRGBADirect", "kLDRRGBABaseOffset", "kHDRRGBDirectLDRAlpha",
    "kHDRRGBDirectHDRAlpha" }};

////////////////////////////////////////////////////////////////////////////////
//
// A generic stat that should be tracked via an instance of ASTCFileStats.
class Stat {
 public:
  explicit Stat(const std::vector<IntermediateBlockData>* blocks, size_t total)
      : blocks_(blocks), total_(total) { }
  virtual ~Stat() { }

  virtual std::ostream& PrintToStream(std::ostream& out) const = 0;

 protected:
  // Utility function to iterate over all of the blocks that are not void-extent
  // blocks. FoldFn optionally allows a value to accumulate. It should be of the
  // type:
  //   (const IntermediateBlockData&, T x) -> T
  template<typename T, typename FoldFn>
  T IterateBlocks(T initial, FoldFn f) const {
    T result = initial;
    for (const auto& block : *blocks_) {
      result = f(block, std::move(result));
    }
    return result;
  }

  size_t NumBlocks() const { return total_; }

 private:
  const std::vector<IntermediateBlockData>* const blocks_;
  const size_t total_;
};

////////////////////////////////////////////////////////////////////////////////
//
// Computes the number of void extent blocks.
class VoidExtentCount : public Stat {
 public:
  VoidExtentCount(const std::vector<IntermediateBlockData>* blocks,
                  size_t total, std::string description)
      : Stat(blocks, total), description_(std::move(description)),
        count_(total - blocks->size()) { }

  std::ostream& PrintToStream(std::ostream& out) const override {
    return out << description_ << ": " << count_
               << " (" << (count_ * 100 / NumBlocks())  << "%)" << std::endl;
  };

 private:
  const std::string description_;
  const size_t count_;
};

//////////////////////////////////////////////////////////////////////////////
//
// Computes a per-block stat and reports it as an average over all blocks.
class PerBlockAverage : public Stat {
 public:
  PerBlockAverage(const std::vector<IntermediateBlockData>* blocks,
                  size_t total, std::string description,
                  const std::function<int(const IntermediateBlockData&)> &fn)
      : Stat(blocks, total),
        description_(std::move(description)) {
    int sum = 0;
    size_t count = 0;
    for (const auto& block : *blocks) {
      sum += fn(block);
      ++count;
    }
    average_ = sum / count;
  }

  std::ostream& PrintToStream(std::ostream& out) const override {
    return out << description_ << ": " << average_ << std::endl;
  }

 private:
  size_t average_;
  std::string description_;
};

//////////////////////////////////////////////////////////////////////////////
//
// Computes a per-block true or false value and reports how many blocks return
// true with a percentage of total blocks.
class PerBlockPredicate : public Stat {
 public:
  PerBlockPredicate(const std::vector<IntermediateBlockData>* blocks,
                    size_t total, std::string description,
                    const std::function<bool(const IntermediateBlockData&)> &fn)
      : Stat(blocks, total),
        description_(std::move(description)),
        count_(std::count_if(blocks->begin(), blocks->end(), fn)) { }

  std::ostream& PrintToStream(std::ostream& out) const override {
    return out << description_ << ": " << count_
               << " (" << (count_ * 100 / NumBlocks())  << "%)" << std::endl;
  };

 private:
  const std::string description_;
  const size_t count_;
};

//////////////////////////////////////////////////////////////////////////////
//
// Returns a histogram of the number of occurrences of each endpoint mode in
// the list of blocks. Note, due to multi-subset blocks, the sum of these
// values will not match the total number of blocks.
class ModeCountsStat : public Stat {
 public:
  explicit ModeCountsStat(const std::vector<IntermediateBlockData>* blocks,
                          size_t total)
      : Stat(blocks, total),
        mode_counts_(IterateBlocks<ModeArray>(
            {}, [](const IntermediateBlockData& data, ModeArray&& m) {
              auto result = m;
              for (const auto& ep : data.endpoints) {
                result[static_cast<int>(ep.mode)]++;
              }
              return result;
            })) { }

  std::ostream& PrintToStream(std::ostream& out) const override {
    const size_t total_modes_used =
        std::accumulate(mode_counts_.begin(), mode_counts_.end(), 0);

    out << "Endpoint modes used: " << std::endl;
    for (size_t i = 0; i < kNumEndpointModes; ++i) {
      out << "  ";
      out << std::setw(30) << std::left << std::setfill('.') << kModeStrings[i];
      out << std::setw(8) << std::right << std::setfill('.') << mode_counts_[i];

      std::stringstream pct;
      pct << " (" << (mode_counts_[i] * 100 / total_modes_used) << "%)";

      out << std::setw(6) << std::right << std::setfill(' ') << pct.str();
      out << std::endl;
    }

    return out;
  }

 private:
  using ModeArray = std::array<int, kNumEndpointModes>;
  const ModeArray mode_counts_;
};

//////////////////////////////////////////////////////////////////////////////
//
// Counts the number of unique endpoints used across all blocks.
class UniqueEndpointsCount : public Stat {
 public:
  explicit UniqueEndpointsCount(
      const std::vector<IntermediateBlockData>* blocks, size_t total)
      : Stat(blocks, total),
        unique_endpoints_(IterateBlocks<UniqueEndpointSet>(
            UniqueEndpointSet(),
            [](const IntermediateBlockData& data, UniqueEndpointSet&& eps) {
              UniqueEndpointSet result(eps);
              for (const auto& ep : data.endpoints) {
                RgbaColor ep_one, ep_two;
                DecodeColorsForMode(ep.colors, data.endpoint_range.value(),
                                    ep.mode, &ep_one, &ep_two);
                result.insert(PackEndpoint(ep_one));
                result.insert(PackEndpoint(ep_two));
              }
              return result;
            })) { }

  std::ostream& PrintToStream(std::ostream& out) const override {
    out << "Num unique endpoints: " << unique_endpoints_.size() << std::endl;
    return out;
  }

 private:
  static uint32_t PackEndpoint(const RgbaColor& color) {
    uint32_t result = 0;
    for (const int& c : color) {
      constexpr int kSaturatedChannelValue = 0xFF;
      assert(c >= 0);
      assert(c <= kSaturatedChannelValue);
      result <<= 8;
      result |= c;
    }
    return result;
  }

  using UniqueEndpointSet = std::unordered_set<uint32_t>;
  const UniqueEndpointSet unique_endpoints_;
};

//////////////////////////////////////////////////////////////////////////////
//
// Computes a histogram of the number of occurrences of 1-4 subset partitions.
class PartitionCountStat : public Stat {
 public:
  explicit PartitionCountStat(const std::vector<IntermediateBlockData>* blocks,
                              size_t total)
      : Stat(blocks, total)
      , part_counts_(IterateBlocks<PartCount>(
          {}, [](const IntermediateBlockData& data, PartCount&& m) {
            PartCount result = m;
            result[data.endpoints.size() - 1]++;
            return result;
          })) { }

  std::ostream& PrintToStream(std::ostream& out) const override {
    out << "Num partitions used: " << std::endl;
    for (size_t i = 0; i < part_counts_.size(); ++i) {
      out << "  " << i + 1 << ": " << part_counts_[i] << std::endl;
    }
    return out;
  }

 private:
  using PartCount = std::array<int, 4>;
  const PartCount part_counts_;
};

//////////////////////////////////////////////////////////////////////////////
//
// For each block that uses dual-plane mode, computes and stores the dual-plane
// channels in a vector. Outputs the number of each channel used across all
// blocks
class DualChannelStat : public Stat {
 private:
  static constexpr auto kNumDualPlaneChannels =
    std::tuple_size<astc_codec::Endpoint>::value;
  using CountsArray = std::array<int, kNumDualPlaneChannels>;

 public:
  explicit DualChannelStat(const std::vector<IntermediateBlockData>* blocks,
                           size_t total)
      : Stat(blocks, total),
        dual_channels_(IterateBlocks(
            std::vector<int>(),
            [](const IntermediateBlockData& data, std::vector<int>&& input) {
              auto result = input;
              if (data.dual_plane_channel) {
                result.push_back(data.dual_plane_channel.value());
              }
              return result;
            })) { }

  std::ostream& PrintToStream(std::ostream& out) const override {
    // Similar to the number of partitions, the number of dual plane blocks
    // can be determined by parsing the next four fields and summing them.
    const int num_dual_plane_blocks = dual_channels_.size();
    out << "Number of dual-plane blocks: " << num_dual_plane_blocks
        << " (" << (num_dual_plane_blocks * 100) / NumBlocks() << "%)"
        << std::endl;

    CountsArray counts = GetCounts();
    assert(counts.size() == kNumDualPlaneChannels);

    for (size_t i = 0; i < counts.size(); ++i) {
      out << "  " << i << ": " << counts[i] << std::endl;
    }
    return out;
  }

 private:
  CountsArray GetCounts() const {
    CountsArray counts;
    for (size_t i = 0; i < kNumDualPlaneChannels; ++i) {
      counts[i] =
          std::count_if(dual_channels_.begin(), dual_channels_.end(),
                        [i](int channel) { return i == channel; });
    }
    return counts;
  }

  const std::vector<int> dual_channels_;
};


// Stores the intermediate block representations of the blocks associated with
// an ASTCFile. Also provides various facilities for extracting aggregate data
// from these blocks.
class ASTCFileStats {
 public:
  explicit ASTCFileStats(const std::unique_ptr<ASTCFile>& astc_file) {
    const size_t total = astc_file->NumBlocks();

    for (size_t block_idx = 0; block_idx < astc_file->NumBlocks(); ++block_idx) {
      const PhysicalASTCBlock pb = astc_file->GetBlock(block_idx);
      assert(!pb.IsIllegalEncoding());
      if (pb.IsIllegalEncoding()) {
        std::cerr << "WARNING: Block " << block_idx << " has illegal encoding." << std::endl;
        continue;
      }

      if (!pb.IsVoidExtent()) {
        Optional<IntermediateBlockData> block = UnpackIntermediateBlock(pb);
        if (!block) {
          std::cerr << "WARNING: Block " << block_idx << " failed to unpack." << std::endl;
          continue;
        }

        blocks_.push_back(block.value());
      }
    }

    stats_.emplace_back(new UniqueEndpointsCount(&blocks_, total));
    stats_.emplace_back(new VoidExtentCount(
        &blocks_, total, "Num void extent blocks"));

    stats_.emplace_back(new PerBlockAverage(
        &blocks_, total, "Average weight range",
        [](const IntermediateBlockData& b) { return b.weight_range; }));

    stats_.emplace_back(new PerBlockAverage(
        &blocks_, total, "Average number of weights",
        [](const IntermediateBlockData& b) { return b.weights.size(); }));

    stats_.emplace_back(new PerBlockPredicate(
        &blocks_, total, "Num blocks that use blue contract mode",
        [](const IntermediateBlockData& block) {
          for (const auto& ep : block.endpoints) {
            if (UsesBlueContract(
                    block.endpoint_range.valueOr(255), ep.mode, ep.colors)) {
              return true;
            }
          }

          return false;
        }));

    stats_.emplace_back(new ModeCountsStat(&blocks_, total));

    stats_.emplace_back(new PerBlockPredicate(
        &blocks_, total, "Num multi-part blocks",
        [](const IntermediateBlockData& block) {
          return block.endpoints.size() > 1;
        }));
    stats_.emplace_back(new PartitionCountStat(&blocks_, total));

    stats_.emplace_back(new DualChannelStat(&blocks_, total));
  }

  // Returns a sorted list of pairs of the form (part_id, count) where the
  // |part_id| is the partition ID used for 2-subset blocks, and |count| is the
  // number of times that particular ID was used.
  std::vector<std::pair<int, int>> ComputePartIDHistogram() const {
    std::vector<int> part_ids(1 << 11, 0);
    std::iota(part_ids.begin(), part_ids.end(), 0);

    // The histogram will then pair IDs with counts so that we can sort by
    // the number of instances later on.
    std::vector<std::pair<int, int>> part_id_histogram;
    std::transform(part_ids.begin(), part_ids.end(),
                   std::back_inserter(part_id_histogram),
                   [](const int& x) { return std::make_pair(x, 0); });

    // Actually count the IDs in the list of blocks.
    for (const auto& block : blocks_) {
      if (block.endpoints.size() == 2) {
        const int id = block.partition_id.value();
        assert(part_id_histogram[id].first == id);
        part_id_histogram[id].second++;
      }
    }

    struct OrderBySecondGreater {
      typedef std::pair<int, int> PairType;
        bool operator()(const PairType& lhs, const PairType& rhs) {
          return lhs.second > rhs.second;
        }
    };

    // Sort by descending numbers of occurrence for each partition ID
    std::sort(part_id_histogram.begin(), part_id_histogram.end(),
              OrderBySecondGreater());

    return part_id_histogram;
  }

  // Weights range from 2x2 - 12x12. For simplicity define buckets for every
  // pair in [0, 12]^2.
  constexpr static int kResolutionBuckets = 13;
  // Returns a linear array of buckets over all pairs of grid resolutions,
  // x-major in memory.
  std::vector<int> ComputeWeightResolutionHistogram() const {
    // Allocate one bucket for every grid resolution.
    std::vector<int> resolution_histogram(
        kResolutionBuckets * kResolutionBuckets, 0);

    // Count the weight resolutions in the list of blocks.
    for (const auto& block : blocks_) {
      const int dim_x = block.weight_grid_dim_x;
      const int dim_y = block.weight_grid_dim_y;
      assert(dim_x > 0);
      assert(dim_x < kResolutionBuckets);
      assert(dim_y > 0);
      assert(dim_y < kResolutionBuckets);
      ++resolution_histogram[dim_x + dim_y * kResolutionBuckets];
    }

    return resolution_histogram;
  }

  // Runs through each defined statistic and prints it out to stdout. Also
  // prints a histogram of partition ids used for the given blocks.
  void PrintStats() const {
    for (const auto& stat : stats_) {
      stat->PrintToStream(std::cout);
    }

    // We also want to find if there are any 2-subset partition IDs that are
    // used disproportionately often. Since partition IDs are 11 bits long, we
    // can have as many as (1 << 11) used IDs in a given sequence of blocks.
    const auto part_id_histogram = ComputePartIDHistogram();
    const int total_part_ids = std::accumulate(
        part_id_histogram.begin(), part_id_histogram.end(), 0,
        [](const int& x, const std::pair<int, int>& hist) {
          return x + hist.second;
        });

    if (total_part_ids > 0) {
      // Display numbers until we either:
      //   A. Display the top 90% of used partitions
      //   B. Reach a point where the remaining partition IDs constitute < 1% of
      //      the total number of IDs used.
      const auto prepare_part_entry = []() -> std::ostream& {
        return std::cout << std::setw(6) << std::left << std::setfill('.');
      };
      int part_accum = 0;
      std::cout << "Two subset partition ID histogram: " << std::endl;
      std::cout << "  ";
      prepare_part_entry() << "ID" << "Count" << std::endl;
      for (const auto& hist : part_id_histogram) {
        part_accum += hist.second;
        if ((hist.second * 100 / total_part_ids) < 1 ||
            (100 * (total_part_ids - part_accum)) / total_part_ids < 10) {
          const int num_to_display = (total_part_ids - part_accum);
          std::cout << "  rest: " << num_to_display
                    << " (" << (num_to_display * 100 / total_part_ids)
                    << "%)" << std::endl;
          break;
        } else {
          std::cout << "  ";
          prepare_part_entry() << hist.first << hist.second
                               << " (" << (hist.second * 100 / total_part_ids)
                               << "%)" << std::endl;
        }
      }
    }

    // Build the 2D histogram of resolutions.
    std::vector<int> weight_histogram = ComputeWeightResolutionHistogram();
    // Labels the weight resolution table.
    std::cout << "Weight resolutions:" << std::endl;
    const auto prepare_weight_entry = []() -> std::ostream& {
      return std::cout << std::setw(6) << std::left << std::setfill(' ');
    };
    prepare_weight_entry() << "H W";
    for (int resolution_x = 2; resolution_x < kResolutionBuckets;
         ++resolution_x) {
      prepare_weight_entry() << resolution_x;
    }
    std::cout << std::endl;

    // Displays table; skips rows/cols {0, 1} since they will always be empty.
    for (int resolution_y = 2; resolution_y < kResolutionBuckets;
         ++resolution_y) {
      prepare_weight_entry() << resolution_y;
      for (int resolution_x = 2; resolution_x < kResolutionBuckets;
           ++resolution_x) {
        const int count =
            weight_histogram[resolution_x + resolution_y * kResolutionBuckets];
        prepare_weight_entry();
        if (!count) {
          std::cout << "*";
        } else {
          std::cout << count;
        }
      }
      std::cout << std::endl;
    }
  }

  size_t NumBlocks() const { return blocks_.size(); }

 private:
  std::vector<std::unique_ptr<Stat>> stats_;
  std::vector<IntermediateBlockData> blocks_;
};

std::ostream& operator<<(std::ostream& stream, const RgbaColor& color) {
  stream << "{";
  constexpr int kNumChannels = std::tuple_size<RgbaColor>::value;
  for (int i = 0; i < kNumChannels; ++i) {
    stream << color[i];
    if (i < (kNumChannels - 1)) {
      stream << ", ";
    }
  }
  return stream << "}";
}

void PrintStatsForBlock(const PhysicalASTCBlock& pb,
                        astc_codec::Footprint footprint) {
  const auto print_void_extent = [&pb](const VoidExtentData& void_extent_data) {
    std::cout << "Void extent block:" << std::endl;
    std::cout << "  16-bit RGBA: {"
              << void_extent_data.r << ", "
              << void_extent_data.g << ", "
              << void_extent_data.b << ", "
              << void_extent_data.a << "}" << std::endl;
    if (pb.VoidExtentCoords()) {
      std::cout << "  Extent (S): {"
                << void_extent_data.coords[0] << ", "
                << void_extent_data.coords[1] << "}" << std::endl;
      std::cout << "  Extent (T): {"
                << void_extent_data.coords[2] << ", "
                << void_extent_data.coords[3] << "}" << std::endl;
    } else {
      std::cout << "  No valid extent data" << std::endl;
    }
  };

  const auto print_endpoint_data =
      [](ColorEndpointMode mode, int endpoint_range,
         const std::vector<int>& encoded_vals) {
    std::cout << "  Endpoint mode: "
              << kModeStrings[static_cast<int>(mode)] << std::endl;
    std::cout << "  Uses blue-contract mode: "
              << (UsesBlueContract(endpoint_range, mode, encoded_vals)
                  ? "true" : "false")
              << std::endl;

    RgbaColor endpoint_low, endpoint_high;
    DecodeColorsForMode(encoded_vals, endpoint_range, mode,
                        &endpoint_low, &endpoint_high);

    std::cout << "  Low endpoint: " << endpoint_low << std::endl;
    std::cout << "  High endpoint: " << endpoint_high << std::endl;
  };

  const auto print_color_data =
      [&print_endpoint_data, &footprint](const IntermediateBlockData& ib_data) {
    const int endpoint_range = ib_data.endpoint_range.value();
    std::cout << "Endpoint range: " << endpoint_range << std::endl;

    const int num_parts = ib_data.endpoints.size();
    if (ib_data.partition_id.hasValue()) {
      const int part_id = ib_data.partition_id.value();
      std::cout << "Parititon ID: " << part_id << std::endl;

      const auto part = GetASTCPartition(footprint, num_parts, part_id);
      assert(part.assignment.size() == footprint.Height() * footprint.Width());

      std::cout << "Assignment:" << std::endl;
      for (int y = 0; y < footprint.Height(); ++y) {
        std::cout << " ";
        for (int x = 0; x < footprint.Width(); ++x) {
          const int texel_index = y * footprint.Width() + x;
          std::cout << " " << part.assignment[texel_index];
        }
        std::cout << std::endl;
      }
    } else {
      std::cout << "Single partition" << std::endl;
    }

    int endpoint_index = 0;
    for (const auto& ep_data : ib_data.endpoints) {
      if (num_parts == 1) {
        std::cout << "Endpoints:" << std::endl;
      } else {
        std::cout << "Endpoint " << (endpoint_index++) << ": " << std::endl;
      }
      print_endpoint_data(ep_data.mode, endpoint_range, ep_data.colors);
    }

    if (ib_data.dual_plane_channel) {
      std::cout << "Dual plane channel: "
                << ib_data.dual_plane_channel.value() << std::endl;
    } else {
      std::cout << "Single plane" << std::endl;
    }
  };

  const auto print_weight_data =
      [&footprint](const IntermediateBlockData& ib_data) {
    std::cout << "Weight grid dimensions: "
              << ib_data.weight_grid_dim_x << "x" << ib_data.weight_grid_dim_y
              << std::endl;
    std::cout << "Weight range: " << ib_data.weight_range << std::endl;

    std::cout << "Encoded weight grid: " << std::endl;
    int weight_idx = 0;
    for (int j = 0; j < ib_data.weight_grid_dim_y; ++j) {
      std::cout << "  ";
      for (int i = 0; i < ib_data.weight_grid_dim_x; ++i) {
        std::cout << std::setw(3) << std::left << std::setfill(' ')
                  << ib_data.weights[weight_idx++];
      }
      std::cout << std::endl;
    }

    std::cout << "Actual weight grid: " << std::endl;
    std::vector<int> actual_weights = ib_data.weights;
    for (auto& weight : actual_weights) {
      weight = astc_codec::UnquantizeWeightFromRange(
          weight, ib_data.weight_range);
    }

    actual_weights = astc_codec::InfillWeights(
        actual_weights, footprint, ib_data.weight_grid_dim_x,
        ib_data.weight_grid_dim_y);

    weight_idx = 0;
    for (int j = 0; j < footprint.Height(); ++j) {
      std::cout << "  ";
      for (int i = 0; i < footprint.Width(); ++i) {
        std::cout << std::setw(3) << std::left << std::setfill(' ')
                  << actual_weights[weight_idx++];
      }
      std::cout << std::endl;
    }
  };

  if (pb.IsVoidExtent()) {
    Optional<VoidExtentData> ve = astc_codec::UnpackVoidExtent(pb);
    if (!ve) {
      std::cerr << "ERROR: Failed to unpack void extent block." << std::endl;
    } else {
      print_void_extent(ve.value());
    }
  } else {
    Optional<IntermediateBlockData> ib =
        astc_codec::UnpackIntermediateBlock(pb);
    if (!ib) {
      std::cerr << "ERROR: Failed to unpack intermediate block." << std::endl;
    } else {
      const auto& ib_data = ib.value();
      print_color_data(ib_data);
      print_weight_data(ib_data);
    }
  }
}

}  // namespace

int main(int argc, char* argv[]) {
  bool error = false;

  std::string filename;
  size_t block_index = 0;
  bool has_block_index = false;

  if (argc >= 2) {
    filename = argv[1];

    if (argc == 3) {
      int32_t param = astc_codec::base::ParseInt32(argv[2], -1);
      if (param < 0) {
        std::cerr << "ERROR: Invalid block index." << std::endl;
        error = true;
      } else {
        block_index = static_cast<size_t>(param);
        has_block_index = true;
      }
    } else if (argc != 2) {
      std::cerr << "ERROR: Too many parameters." << std::endl;
      error = true;
    }
  } else {
    error = true;
  }

  if (error) {
    std::cout << ((argc >= 0) ? argv[0] : "astc_inspector_cli")
              << " <filename> [<block index>]" << std::endl
              << std::endl
              << "Collects the various statistics of a stream of ASTC data "
              << "stored in an ASTC file." << std::endl
              << std::endl
              << "    filename        ASTC file path." << std::endl
              << "    block index     If specified, show detailed information about a block"
              << std::endl;
    return 1;
  }

  std::string error_string;
  std::unique_ptr<ASTCFile> astc_file = ASTCFile::LoadFile(argv[1], &error_string);
  if (!astc_file) {
    std::cerr << "ERROR: " << error_string << std::endl;
    return 2;
  }

  if (has_block_index) {
    Optional<astc_codec::Footprint> footprint =
        astc_codec::Footprint::Parse(astc_file->GetFootprintString().c_str());
    if (!footprint) {
      std::cerr << "ERROR: Invalid footprint \"" << astc_file->GetFootprintString() << "\"" << std::endl;
      return 3;
    }

    PrintStatsForBlock(astc_file->GetBlock(block_index), footprint.value());
  } else {
    std::cout << "Dimensions: " << astc_file->GetWidth() << "x"
              << astc_file->GetHeight() << ", depth " << astc_file->GetDepth()
              << std::endl;

    ASTCFileStats stats(astc_file);

    std::cout << std::endl
              << "Total bits used: " << 128 * astc_file->NumBlocks()
              << " (" << astc_file->NumBlocks() << " blocks, "
              << (astc_file->NumBlocks() * 16) << " bytes)"
              << std::endl << std::endl;

    stats.PrintStats();
  }

  return 0;
}
