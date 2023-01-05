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

#include "src/decoder/partition.h"
#include "src/base/bottom_n.h"
#include "src/base/utils.h"
#include "src/decoder/footprint.h"

#include <algorithm>
#include <array>
#include <limits>
#include <memory>
#include <numeric>
#include <queue>
#include <set>
#include <unordered_set>
#include <utility>

namespace astc_codec {

namespace {

// The maximum number of partitions supported by ASTC is four.
constexpr int kMaxNumSubsets = 4;

// Partition selection function based on the ASTC specification.
// See section C.2.21
int SelectASTCPartition(int seed, int x, int y, int z, int partitioncount,
                        int num_pixels) {
  if (partitioncount <= 1) {
    return 0;
  }

  if (num_pixels < 31) {
    x <<= 1;
    y <<= 1;
    z <<= 1;
  }

  seed += (partitioncount - 1) * 1024;

  uint32_t rnum = seed;
  rnum ^= rnum >> 15;
  rnum -= rnum << 17;
  rnum += rnum << 7;
  rnum += rnum << 4;
  rnum ^= rnum >> 5;
  rnum += rnum << 16;
  rnum ^= rnum >> 7;
  rnum ^= rnum >> 3;
  rnum ^= rnum << 6;
  rnum ^= rnum >> 17;

  uint8_t seed1 = rnum & 0xF;
  uint8_t seed2 = (rnum >> 4) & 0xF;
  uint8_t seed3 = (rnum >> 8) & 0xF;
  uint8_t seed4 = (rnum >> 12) & 0xF;
  uint8_t seed5 = (rnum >> 16) & 0xF;
  uint8_t seed6 = (rnum >> 20) & 0xF;
  uint8_t seed7 = (rnum >> 24) & 0xF;
  uint8_t seed8 = (rnum >> 28) & 0xF;
  uint8_t seed9 = (rnum >> 18) & 0xF;
  uint8_t seed10 = (rnum >> 22) & 0xF;
  uint8_t seed11 = (rnum >> 26) & 0xF;
  uint8_t seed12 = ((rnum >> 30) | (rnum << 2)) & 0xF;

  seed1 *= seed1;
  seed2 *= seed2;
  seed3 *= seed3;
  seed4 *= seed4;
  seed5 *= seed5;
  seed6 *= seed6;
  seed7 *= seed7;
  seed8 *= seed8;
  seed9 *= seed9;
  seed10 *= seed10;
  seed11 *= seed11;
  seed12 *= seed12;

  int sh1, sh2, sh3;
  if (seed & 1) {
    sh1 = (seed & 2 ? 4 : 5);
    sh2 = (partitioncount == 3 ? 6 : 5);
  } else {
    sh1 = (partitioncount == 3 ? 6 : 5);
    sh2 = (seed & 2 ? 4 : 5);
  }
  sh3 = (seed & 0x10) ? sh1 : sh2;

  seed1 >>= sh1;
  seed2 >>= sh2;
  seed3 >>= sh1;
  seed4 >>= sh2;
  seed5 >>= sh1;
  seed6 >>= sh2;
  seed7 >>= sh1;
  seed8 >>= sh2;

  seed9 >>= sh3;
  seed10 >>= sh3;
  seed11 >>= sh3;
  seed12 >>= sh3;

  int a = seed1 * x + seed2 * y + seed11 * z + (rnum >> 14);
  int b = seed3 * x + seed4 * y + seed12 * z + (rnum >> 10);
  int c = seed5 * x + seed6 * y + seed9  * z + (rnum >> 06);
  int d = seed7 * x + seed8 * y + seed10 * z + (rnum >> 02);

  a &= 0x3F;
  b &= 0x3F;
  c &= 0x3F;
  d &= 0x3F;

  if (partitioncount <= 3) {
    d = 0;
  }
  if (partitioncount <= 2) {
    c = 0;
  }

  if (a >= b && a >= c && a >= d) {
    return 0;
  } else if (b >= c && b >= d) {
    return 1;
  } else if (c >= d) {
    return 2;
  } else {
    return 3;
  }
}

// A partition hash that we can pass to containers like std::unordered_set
struct PartitionHasher {
  size_t operator()(const Partition& part) const {
    // The issue here is that if we have two different partitions, A and B, then
    // their hash should be equal if A and B are equal. We define the distance
    // between A and B using PartitionMetric, but internally that finds a 1-1
    // mapping from labels in A to labels in B.
    //
    // With that in mind, when we define a hash for partitions, we need to find
    // a 1-1 mapping to a 'universal' labeling scheme. Here we define that as
    // the heuristic: the first encountered label will be 0, the second will be
    // 1, etc. This creates a unique 1-1 mapping scheme from any partition.
    //
    // Note, we can't use this heuristic for the PartitionMetric, as it will
    // generate very large discrepancies between similar labellings (for example
    // 000...001 vs 011...111). We are just looking for a boolean distinction
    // whether or not two partitions are different and don't care how different
    // they are.
    std::array<int, kMaxNumSubsets> mapping {{ -1, -1, -1, -1 }};
    int next_subset = 0;
    for (int subset : part.assignment) {
      if (mapping[subset] < 0) {
        mapping[subset] = next_subset++;
      }
    }
    assert(next_subset <= kMaxNumSubsets);

    // The return value will be the hash of the assignment according to this
    // mapping
    const size_t seed0 = 0;
    return std::accumulate(part.assignment.begin(), part.assignment.end(), seed0,
                           [&mapping](size_t seed, const int& subset) {
                             std::hash<size_t> hasher;
                             const int s = mapping[subset];
                             return hasher(seed) ^ hasher(static_cast<size_t>(s));
                           });
  }
};

// Construct a VP-Tree of partitions. Since our PartitionMetric satisfies
// the triangle inequality, we can use this general higher-dimensional space
// partitioning tree to organize our partitions.
//
// TODO(google): !SPEED! Right now this tree stores an actual linked
// structure of pointers which is likely very slow during construction and
// very not cache-coherent during traversal, so it'd probably be good to
// switch to a flattened binary tree structure if performance becomes an
// issue.
class PartitionTree {
 public:
  // Unclear what it means to have an uninitialized tree, so delete default
  // constructors, but allow the tree to be moved
  PartitionTree() = delete;
  PartitionTree(const PartitionTree&) = delete;
  PartitionTree(PartitionTree&& t) = default;

  // Generate a PartitionTree from iterators over |Partition|s
  template<typename Itr>
  PartitionTree(Itr begin, Itr end) : parts_(begin, end) {
    std::vector<int> part_indices(parts_.size());
    std::iota(part_indices.begin(), part_indices.end(), 0);
    root_ = std::unique_ptr<PartitionTreeNode>(
        new PartitionTreeNode(parts_, part_indices));
  }

  // Search for the k-nearest partitions that are closest to part based on
  // the result of PartitionMetric
  void Search(const Partition& part, int k,
              std::vector<const Partition*>* const results,
              std::vector<int>* const distances) const {
    ResultHeap heap(k);
    SearchNode(root_, part, &heap);

    results->clear();
    if (nullptr != distances) {
      distances->clear();
    }

    std::vector<ResultNode> search_results = heap.Pop();
    for (const auto& result : search_results) {
      results->push_back(&parts_[result.part_idx]);
      if (nullptr != distances) {
        distances->push_back(result.distance);
      }
    }

    assert(results->size() == size_t(k));
  }

 private:
  // Heap elements to be stored while searching the tree. The two relevant
  // pieces of information are the partition index and it's distance from the
  // queried partition.
  struct ResultNode {
    int part_idx;
    int distance;

    // Heap based on distance from query point.
    bool operator<(const ResultNode& other) const {
      return distance < other.distance;
    }
  };

  using ResultHeap = base::BottomN<ResultNode>;

  struct PartitionTreeNode {
    int part_idx;
    int split_dist;

    std::unique_ptr<PartitionTreeNode> left;
    std::unique_ptr<PartitionTreeNode> right;

    PartitionTreeNode(const std::vector<Partition> &parts,
                      const std::vector<int> &part_indices)
        : split_dist(-1) {
      assert(part_indices.size() > 0);

      right.reset(nullptr);
      left.reset(nullptr);

      // Store the first node as our vantage point
      part_idx = part_indices[0];
      const Partition& vantage_point = parts[part_indices[0]];

      // Calculate the distances of the remaining nodes against the vantage
      // point.
      std::vector<std::pair<int, int>> part_dists;
      for (size_t i = 1; i < part_indices.size(); ++i) {
        const int idx = part_indices[i];
        const int dist = PartitionMetric(vantage_point, parts[idx]);
        if (dist > 0) {
          part_dists.push_back(std::make_pair(idx, dist));
        }
      }

      // If there are no more different parts, then this is a leaf node
      if (part_dists.empty()) {
        return;
      }

      struct OrderBySecond {
        typedef std::pair<int, int> PairType;
          bool operator()(const PairType& lhs, const PairType& rhs) {
            return lhs.second < rhs.second;
          }
      };

      // We want to partition the set such that the points are ordered
      // based on their distances from the vantage point. We can do this
      // using the partial sort of nth element.
      std::nth_element(
          part_dists.begin(), part_dists.begin() + part_dists.size() / 2,
          part_dists.end(), OrderBySecond());

      // Once that's done, our split position is in the middle
      const auto split_iter = part_dists.begin() + part_dists.size() / 2;
      split_dist = split_iter->second;

      // Recurse down the right and left sub-trees with the indices of the
      // parts that are farther and closer respectively
      std::vector<int> right_indices;
      for (auto itr = split_iter; itr != part_dists.end(); ++itr) {
        right_indices.push_back(itr->first);
      }

      if (!right_indices.empty()) {
        right.reset(new PartitionTreeNode(parts, right_indices));
      }

      std::vector<int> left_indices;
      for (auto itr = part_dists.begin(); itr != split_iter; ++itr) {
        left_indices.push_back(itr->first);
      }

      if (!left_indices.empty()) {
        left.reset(new PartitionTreeNode(parts, left_indices));
      }
    }
  };

  void SearchNode(const std::unique_ptr<PartitionTreeNode>& node,
                  const Partition& p, ResultHeap* const heap) const {
    if (nullptr == node) {
      return;
    }

    // Calculate distance against current node
    const int dist = PartitionMetric(parts_[node->part_idx], p);

    // Push it onto the heap and remove the top-most nodes to maintain
    // closest k indices.
    ResultNode result;
    result.part_idx = node->part_idx;
    result.distance = dist;
    heap->Push(result);

    // If the split distance is uninitialized, it means we have no children.
    if (node->split_dist < 0) {
      assert(nullptr == node->left);
      assert(nullptr == node->right);
      return;
    }

    // Next we need to check the left and right trees if their distance
    // is closer/farther than the farthest element on the heap
    const int tau = heap->Top().distance;
    if (dist + tau < node->split_dist || dist - tau < node->split_dist) {
      SearchNode(node->left, p, heap);
    }

    if (dist + tau > node->split_dist || dist - tau > node->split_dist) {
      SearchNode(node->right, p, heap);
    }
  }

  std::vector<Partition> parts_;
  std::unique_ptr<PartitionTreeNode> root_;
};

// A helper function that generates all of the partitions for each number of
// subsets in ASTC blocks and stores them in a PartitionTree for fast retrieval.
const int kNumASTCPartitionIDBits = 10;
PartitionTree GenerateASTCPartitionTree(Footprint footprint) {
  std::unordered_set<Partition, PartitionHasher> parts;
  for (int num_parts = 2; num_parts <= kMaxNumSubsets; ++num_parts) {
    for (int id = 0; id < (1 << kNumASTCPartitionIDBits); ++id) {
      Partition part = GetASTCPartition(footprint, num_parts, id);

      // Make sure we're not using a degenerate partition assignment that wastes
      // an endpoint pair...
      bool valid_part = true;
      for (int i = 0; i < num_parts; ++i) {
        if (std::find(part.assignment.begin(), part.assignment.end(), i) ==
            part.assignment.end()) {
          valid_part = false;
          break;
        }
      }

      if (valid_part) {
        parts.insert(std::move(part));
      }
    }
  }

  return PartitionTree(parts.begin(), parts.end());
}

// To avoid needing any fancy boilerplate for mapping from a width, height
// tuple, we can define a simple encoding for the block mode:
constexpr int EncodeDims(int width, int height) {
  return (width << 16) | height;
}

}  // namespace

////////////////////////////////////////////////////////////////////////////////

int PartitionMetric(const Partition& a, const Partition& b) {
  // Make sure that one partition is at least a subset of the other...
  UTILS_RELEASE_ASSERT(a.footprint == b.footprint);

  // Make sure that the number of parts is within our limits. ASTC has a maximum
  // of four subsets per block according to the specification.
  UTILS_RELEASE_ASSERT(a.num_parts <= kMaxNumSubsets);
  UTILS_RELEASE_ASSERT(b.num_parts <= kMaxNumSubsets);

  const int w = a.footprint.Width();
  const int h = b.footprint.Height();

  struct PairCount {
    int a;
    int b;
    int count;

    // Comparison needed for sort below.
    bool operator>(const PairCount& other) const {
      return count > other.count;
    }
  };

  // Since we need to find the smallest mapping from labels in A to labels in B,
  // we need to store each label pair in a structure that can later be sorted.
  // The maximum number of subsets in an ASTC block is four, meaning that
  // between the two partitions, we can have up to sixteen different pairs.
  std::array<PairCount, 16> pair_counts;
  for (int y = 0; y < 4; ++y) {
    for (int x = 0; x < 4; ++x) {
      const int idx = y * 4 + x;
      pair_counts[idx].a = x;
      pair_counts[idx].b = y;
      pair_counts[idx].count = 0;
    }
  }

  // Count how many times we see each pair of assigned values (order matters!)
  for (int y = 0; y < h; ++y) {
    for (int x = 0; x < w; ++x) {
      const int idx = y * w + x;

      const int a_val = a.assignment[idx];
      const int b_val = b.assignment[idx];

      assert(a_val >= 0);
      assert(b_val >= 0);

      assert(a_val < 4);
      assert(b_val < 4);

      ++(pair_counts[b_val * 4 + a_val].count);
    }
  }

  // Sort the pairs in descending order based on their count
  std::sort(pair_counts.begin(), pair_counts.end(), std::greater<PairCount>());

  // Now assign pairs one by one until we have no more pairs to assign. Once
  // a value from A is assigned to a value in B, it can no longer be reassigned,
  // so we can keep track of this in a matrix. Similarly, to keep the assignment
  // one-to-one, once a value in B has been assigned to, it cannot be assigned
  // to again.
  std::array<std::array<bool, kMaxNumSubsets>, kMaxNumSubsets> assigned { };

  int pixels_matched = 0;
  for (const auto& pair_count : pair_counts) {
    bool is_assigned = false;
    for (int i = 0; i < kMaxNumSubsets; ++i) {
      is_assigned |= assigned.at(pair_count.a).at(i);
      is_assigned |= assigned.at(i).at(pair_count.b);
    }

    if (!is_assigned) {
      assigned.at(pair_count.a).at(pair_count.b) = true;
      pixels_matched += pair_count.count;
    }
  }

  // The difference is the number of pixels that had an assignment versus the
  // total number of pixels.
  return w * h - pixels_matched;
}

// Generates the partition assignment for the given block attributes.
Partition GetASTCPartition(const Footprint& footprint, int num_parts,
                           int partition_id) {
  // Partitions must have at least one subset but may have at most four
  assert(num_parts >= 0);
  assert(num_parts <= kMaxNumSubsets);

  // Partition ID can be no more than 10 bits.
  assert(partition_id >= 0);
  assert(partition_id < 1 << 10);

  Partition part = {footprint, num_parts, partition_id, /* assignment = */ {}};
  part.assignment.reserve(footprint.NumPixels());

  // Maintain column-major order so that we match all of the image processing
  // algorithms that depend on this class.
  for (int y = 0; y < footprint.Height(); ++y) {
    for (int x = 0; x < footprint.Width(); ++x) {
      const int p = SelectASTCPartition(partition_id, x, y, 0, num_parts,
                                        footprint.NumPixels());
      part.assignment.push_back(p);
    }
  }

  return part;
}

const std::vector<const Partition*> FindKClosestASTCPartitions(
    const Partition& candidate, int k) {
  const int encoded_dims = EncodeDims(candidate.footprint.Width(),
                                      candidate.footprint.Height());

  int index = 0;
  switch (encoded_dims) {
    case EncodeDims(4, 4): index = 0; break;
    case EncodeDims(5, 4): index = 1; break;
    case EncodeDims(5, 5): index = 2; break;
    case EncodeDims(6, 5): index = 3; break;
    case EncodeDims(6, 6): index = 4; break;
    case EncodeDims(8, 5): index = 5; break;
    case EncodeDims(8, 6): index = 6; break;
    case EncodeDims(8, 8): index = 7; break;
    case EncodeDims(10, 5): index = 8; break;
    case EncodeDims(10, 6): index = 9; break;
    case EncodeDims(10, 8): index = 10; break;
    case EncodeDims(10, 10): index = 11; break;
    case EncodeDims(12, 10): index = 12; break;
    case EncodeDims(12, 12): index = 13; break;
    default:
      assert(false && "Unknown footprint dimensions. This should have been caught sooner.");
      break;
  }

  static const auto* const kASTCPartitionTrees =
      new std::array<PartitionTree, Footprint::NumValidFootprints()> {{
      GenerateASTCPartitionTree(Footprint::Get4x4()),
      GenerateASTCPartitionTree(Footprint::Get5x4()),
      GenerateASTCPartitionTree(Footprint::Get5x5()),
      GenerateASTCPartitionTree(Footprint::Get6x5()),
      GenerateASTCPartitionTree(Footprint::Get6x6()),
      GenerateASTCPartitionTree(Footprint::Get8x5()),
      GenerateASTCPartitionTree(Footprint::Get8x6()),
      GenerateASTCPartitionTree(Footprint::Get8x8()),
      GenerateASTCPartitionTree(Footprint::Get10x5()),
      GenerateASTCPartitionTree(Footprint::Get10x6()),
      GenerateASTCPartitionTree(Footprint::Get10x8()),
      GenerateASTCPartitionTree(Footprint::Get10x10()),
      GenerateASTCPartitionTree(Footprint::Get12x10()),
      GenerateASTCPartitionTree(Footprint::Get12x12()),
    }};

  const PartitionTree& parts_vptree = kASTCPartitionTrees->at(index);
  std::vector<const Partition*> results;
  parts_vptree.Search(candidate, k, &results, nullptr);
  return results;
}

// Returns the valid ASTC partition that is closest to the candidate based on
// the PartitionMetric defined above.
const Partition& FindClosestASTCPartition(const Partition& candidate) {
  // Given a candidate, the closest valid partition will likely not be an exact
  // match. Consider all of the texels for which this valid partition differs
  // with the candidate.
  //
  // If the valid partition has more subsets than the candidate, then all of the
  // highest subset will be included in the mismatched texels. Since the number
  // of possible partitions with increasing subsets grows exponentially, the
  // chance that a valid partition with fewer subsets appears within the first
  // few closest partitions is relatively high. Empirically, we can usually find
  // a partition with at most |candidate.num_parts| number of subsets within the
  // first four closest partitions.
  constexpr int kSearchItems = 4;

  const std::vector<const Partition*> results =
      FindKClosestASTCPartitions(candidate, kSearchItems);

  // Optimistically, look for result with the same number of subsets.
  for (const auto& result : results) {
    if (result->num_parts == candidate.num_parts) {
      return *result;
    }
  }

  // If all else fails, then at least find the result with fewer subsets than
  // we asked for.
  for (const auto& result : results) {
    if (result->num_parts < candidate.num_parts) {
      return *result;
    }
  }

  assert(false &&
         "Could not find partition with acceptable number of subsets!");
  return *(results[0]);
}

}  // namespace astc_codec
