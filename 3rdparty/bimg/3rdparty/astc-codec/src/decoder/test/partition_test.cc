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

#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include <array>
#include <random>
#include <string>
#include <vector>

namespace {

using ::testing::ElementsAreArray;
using ::testing::Eq;
using ::testing::Le;
using ::testing::Not;

using astc_codec::Footprint;
using astc_codec::Partition;
using astc_codec::PartitionMetric;
using astc_codec::GetASTCPartition;
using astc_codec::FindClosestASTCPartition;

// Test to make sure that a simple difference between two partitions where
// most of the values are the same returns what we expect.
TEST(PartitionTest, TestSimplePartitionMetric) {
  Partition a = {Footprint::Get6x6(), /* num_parts = */ 2,
                 /* partition_id = */ {}, /* assignment = */ {}};
  Partition b = a;

  a.assignment = {
      0, 0, 0, 0, 0, 0,
      0, 0, 0, 0, 0, 0,
      0, 0, 0, 0, 0, 0,
      0, 0, 0, 0, 0, 0,
      0, 0, 0, 0, 0, 0,
      0, 0, 0, 0, 0, 1,
    };

  b.assignment = {
      1, 0, 0, 0, 0, 0,
      0, 0, 0, 0, 0, 0,
      0, 0, 0, 0, 0, 0,
      0, 0, 0, 0, 0, 0,
      0, 0, 0, 0, 0, 0,
      0, 0, 0, 0, 0, 0,
    };

  const int dist = PartitionMetric(a, b);
  EXPECT_EQ(dist, 2);
}

// Test to make sure that if one partition is a subset of another that we still
// return the proper difference against the subset of the larger one.
TEST(PartitionDeathTest, TestPartitionMetric) {
  Partition a = {Footprint::Get4x4(), /* num_parts = */ 2,
                 /* partition_id = */ {}, /* assignment = */ {}};
  Partition b = {Footprint::Get6x6(), /* num_parts = */ 2,
                 /* partition_id = */ {}, /* assignment = */ {}};

  a.assignment = {{
      1, 1, 1, 1,
      0, 0, 0, 0,
      0, 0, 0, 0,
      0, 0, 0, 1,
    }};

  b.assignment = {{
      1, 0, 0, 0, 0, 0,
      0, 0, 0, 0, 0, 0,
      0, 0, 0, 0, 0, 1,
      0, 0, 0, 0, 0, 0,
      0, 1, 0, 0, 1, 0,
      0, 0, 1, 1, 0, 0,
    }};

  EXPECT_DEATH(PartitionMetric(a, b), "");
}

// Test to make sure that even if we have different numbers of subsets for each
// partition, that the returned value is what we'd expect.
TEST(PartitionTest, TestDiffPartsPartitionMetric) {
  Partition a = {Footprint::Get4x4(), /* num_parts = */ 2,
                 /* partition_id = */ {}, /* assignment = */ {}};
  Partition b = {Footprint::Get4x4(), /* num_parts = */ 3,
                 /* partition_id = */ {}, /* assignment = */ {}};

  a.assignment = {{
      2, 2, 2, 0,
      0, 0, 0, 0,
      0, 0, 0, 0,
      0, 0, 0, 1,
    }};

  b.assignment = {{
      1, 0, 0, 0,
      0, 0, 0, 0,
      0, 0, 0, 0,
      0, 0, 0, 0
    }};

  const int dist = PartitionMetric(a, b);
  EXPECT_EQ(dist, 3);
}

// An additional sanity check test that makes sure that we're not always mapping
// zero to zero in our tests.
TEST(PartitionTest, TestDiffMappingPartitionMetric) {
  Partition a = {Footprint::Get4x4(), /* num_parts = */ 2,
                 /* partition_id = */ {}, /* assignment = */ {}};
  Partition b = {Footprint::Get4x4(), /* num_parts = */ 3,
                 /* partition_id = */ {}, /* assignment = */ {}};

  a.assignment = {{
      0, 1, 2, 2,
      2, 2, 2, 2,
      2, 2, 2, 2,
      2, 2, 2, 2,
    }};

  b.assignment = {{
      1, 0, 0, 0,
      0, 0, 0, 0,
      0, 0, 0, 0,
      0, 0, 0, 0,
    }};

  const int dist = PartitionMetric(a, b);
  EXPECT_EQ(dist, 1);
}

// Finally, if we grab an ASTC partition and modify it a tad, the closest
// partition should still be the same ASTC partition.
TEST(PartitionTest, TestFindingASTCPartition) {
  const Partition astc = GetASTCPartition(Footprint::Get12x12(), 3, 0x3CB);
  Partition almost_astc = astc;
  almost_astc.assignment[0]++;

  const Partition& closest_astc = FindClosestASTCPartition(almost_astc);
  EXPECT_EQ(astc, closest_astc);
}

// Test a partition that was obtained from the reference ASTC encoder. We should
// be able to match it exactly
TEST(PartitionTest, TestSpecificPartition) {
  const Partition astc = GetASTCPartition(Footprint::Get10x6(), 3, 557);
  EXPECT_THAT(astc.assignment, ElementsAreArray(std::array<int, 60> {{
          0, 0, 0, 0, 1, 1, 1, 2, 2, 2,
          0, 0, 0, 0, 1, 1, 1, 2, 2, 2,
          0, 0, 0, 0, 1, 1, 1, 2, 2, 2,
          0, 0, 0, 0, 1, 1, 1, 2, 2, 2,
          0, 0, 0, 0, 1, 1, 1, 2, 2, 2,
          0, 0, 0, 0, 1, 1, 1, 2, 2, 2 }}));
}

// Make sure that when we match against this specific partition, it'll return a
// partition with the same number of subsets
TEST(PartitionTest, EstimatedPartitionSubsets) {
  Partition partition = {
    /* footprint = */ Footprint::Get6x6(),
    /* num_parts = */ 2,
    /* partition_id = */ {},
    /* assignment = */ {
      0, 0, 1, 1, 1, 0,
      0, 0, 0, 0, 0, 0,
      0, 0, 0, 0, 0, 0,
      0, 1, 1, 1, 1, 1,
      0, 0, 0, 0, 0, 0,
      1, 1, 1, 1, 1, 1
    }};

  const Partition astc = FindClosestASTCPartition(partition);
  EXPECT_THAT(astc.num_parts, Eq(partition.num_parts));
}

// Make sure that regardless of what partition we match against, it'll return a
// partition with at most a fewer number of subsets
TEST(PartitionTest, EstimatedPartitionFewerSubsets) {
  std::mt19937 random(0xdeadbeef);
  auto randUniform = [&random](int max) {
    std::uniform_int_distribution<> dist(0, max - 1);
    return dist(random);
  };

  constexpr int kNumFootprints = Footprint::NumValidFootprints();
  const auto kFootprints = std::array<Footprint, kNumFootprints> {{
      Footprint::Get4x4(),
      Footprint::Get5x4(),
      Footprint::Get5x5(),
      Footprint::Get6x5(),
      Footprint::Get6x6(),
      Footprint::Get8x5(),
      Footprint::Get8x6(),
      Footprint::Get8x8(),
      Footprint::Get10x5(),
      Footprint::Get10x6(),
      Footprint::Get10x8(),
      Footprint::Get10x10(),
      Footprint::Get12x10(),
      Footprint::Get12x12()
    }};

  constexpr int kNumTests = 200;
  for (int i = 0; i < kNumTests; ++i) {
    const auto& footprint = kFootprints[randUniform(kNumFootprints)];
    const int num_parts = 2 + randUniform(3);
    Partition partition = {
      footprint,
      num_parts,
      /* partition_id = */ {},
      /* assignment = */ std::vector<int>(footprint.NumPixels(), 0)};

    for (auto& p : partition.assignment) {
      p = randUniform(num_parts);
    }

    const Partition astc = FindClosestASTCPartition(partition);
    EXPECT_THAT(astc.num_parts, Le(partition.num_parts))
        << "Test #" << i << ": "
        << "Selected partition with ID " << astc.partition_id.value();
  }
}

// Make sure that we generate unique partitions that are close to the
// candidates.
TEST(PartitionTest, UniquePartitionResults) {
  Partition partition = {
    /* footprint = */ Footprint::Get6x6(),
    /* num_parts = */ 2,
    /* partition_id = */ {},
    /* assignment = */ {
      0, 1, 1, 1, 1, 1,
      0, 1, 1, 1, 1, 1,
      0, 1, 1, 1, 1, 1,
      0, 1, 1, 1, 1, 1,
      0, 1, 1, 1, 1, 1,
      0, 1, 1, 1, 1, 1
    }};

  const auto parts = FindKClosestASTCPartitions(partition, 2);
  EXPECT_THAT(*parts[0], Not(Eq(*parts[1])));
}

// TODO(google): Verify somehow that the assignment generated from
// GetASTCPartition actually matches what's in the spec. The selection
// function was more or less copy/pasted though so it's unclear how to
// measure that against e.g. the ASTC encoder.

}  // namespace
