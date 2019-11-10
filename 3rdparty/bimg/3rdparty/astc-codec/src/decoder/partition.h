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

#ifndef ASTC_CODEC_DECODER_PARTITION_H_
#define ASTC_CODEC_DECODER_PARTITION_H_

#include "src/base/optional.h"
#include "src/decoder/footprint.h"

#include <vector>

namespace astc_codec {

struct Partition;

// Determines the "difference" between any two partitions of the same size.
// This metric attempts to find the best one to one mapping from the labels in
// partition a against the labels in partition b. Once that mapping is found, it
// returns the number of pixels that are mismatched between the two. Each
// partition is expected to start in the upper left corner of the block and
// proceed in raster-scan order. Two partitions are equal if the mapping is
// bijective. This metric is a metric in the mathematical sense. In other words
// it has the following properties:
//
// 1) PartitionMetric(a, b) >= 0
// 2) PartitionMetric(a, b) == PartitionMetric(b, a)
// 3) PartitionMetric(a, b) == 0 iff a == b
// 4) PartitionMetric(a, b) + PartitionMetric(b, c) >= PartitionMetric(a, c)
//
// Throws an error if one partition's footprint is not equal to the other.
int PartitionMetric(const Partition& a, const Partition& b);

// A partition is a way to divide up an ASTC block into disjoint subsets such
// that each subset uses a different set of endpoints. This is used to increase
// the compression quality of blocks. One way to store such a partition is to
// assign an ID to use with a predetermined decoding method. Here we store the
// logical representation of partitions by keeping a per-pixel label. All pixels
// that share a label belong to the same subset.
struct Partition {
  // The footprint width and height of this partition. This determines the size
  // of the assignment array.
  Footprint footprint;

  // The number of subsets in this partition. The values in the partition
  // assignment fall within the range [0, num_parts). The maximum number of
  // parts supported is four.
  int num_parts;

  // The 10-bit partition ID as stored in bits 13-22 of multi-part ASTC blocks.
  // (See Section C.2.9) If there is no guarantee that this partition is a valid
  // ASTC partition, this should be set to absl::nullopt.
  base::Optional<int> partition_id;

  // A value in the range [0, num_parts) corresponding to the label for
  // the given texel (x, y) in [0, footprint_width) x [0, footprint_height)
  // using a raster-order layout.
  std::vector<int> assignment;

  // Returns true only if their "distance" is zero, i.e. if they have compatible
  // subset assignments.
  bool operator==(const Partition& other) const {
    return PartitionMetric(*this, other) == 0;
  }
};

// Generates the ASTC partition assignment for the given block attributes.
Partition GetASTCPartition(const Footprint& footprint, int num_parts,
                           int partition_id);

// Returns the |k| valid ASTC partitions that are closest to the candidate based
// on the PartitionMetric defined above.
const std::vector<const Partition*> FindKClosestASTCPartitions(
    const Partition& candidate, int k);

// Returns the valid ASTC partition closest to the candidate with at most as
// many subsets as the |candidate|. Note: this is not a deterministic function,
// as the underlying valid partitions are sorted using a hash map and a distance
// function whose range is the natural numbers. The chances that two or more
// partitions are equally 'closest' is possible, in which case this function
// makes no guarantees about which one it will return. For more control, use
// FindKClosestASTCPartitions above.
const Partition& FindClosestASTCPartition(const Partition& candidate);

}  // namespace astc_codec

#endif  // ASTC_CODEC_DECODER_PARTITION_H_
