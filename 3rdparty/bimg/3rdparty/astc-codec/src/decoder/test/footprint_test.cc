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

#include "src/decoder/footprint.h"

#include <array>
#include <tuple>
#include <vector>

#include <gtest/gtest.h>

namespace astc_codec {

namespace {

TEST(FootprintTest, ParseAstcFootprintString) {
  using ASTCTestPair = std::pair<std::string, Footprint>;
  const std::array<ASTCTestPair, Footprint::NumValidFootprints()>
      valid_footprints {{
      std::make_pair("4x4", Footprint::Get4x4()),
      std::make_pair("5x4", Footprint::Get5x4()),
      std::make_pair("5x5", Footprint::Get5x5()),
      std::make_pair("6x5", Footprint::Get6x5()),
      std::make_pair("6x6", Footprint::Get6x6()),
      std::make_pair("8x5", Footprint::Get8x5()),
      std::make_pair("8x6", Footprint::Get8x6()),
      std::make_pair("8x8", Footprint::Get8x8()),
      std::make_pair("10x5", Footprint::Get10x5()),
      std::make_pair("10x6", Footprint::Get10x6()),
      std::make_pair("10x8", Footprint::Get10x8()),
      std::make_pair("10x10", Footprint::Get10x10()),
      std::make_pair("12x10", Footprint::Get12x10()),
      std::make_pair("12x12", Footprint::Get12x12())
  }};

  for (const auto& test : valid_footprints) {
    base::Optional<Footprint> footprint = Footprint::Parse(test.first.c_str());
    EXPECT_TRUE(footprint);
    EXPECT_EQ(test.second, footprint.value());
  }

  EXPECT_DEBUG_DEATH(EXPECT_FALSE(Footprint::Parse("")), "");
  EXPECT_DEBUG_DEATH(EXPECT_FALSE(Footprint::Parse("3")), "");
  EXPECT_DEBUG_DEATH(EXPECT_FALSE(Footprint::Parse("x")), "");
  // Validly formed but out-of-bounds dimensions do not assert, otherwise
  // malformed ASTC files could crash the decoder in debug builds.
  EXPECT_FALSE(Footprint::Parse("9999999999x10"));
  EXPECT_DEBUG_DEATH(EXPECT_FALSE(Footprint::Parse("ax8")), "");
  EXPECT_DEBUG_DEATH(EXPECT_FALSE(Footprint::Parse("2x3x4")), "");
  EXPECT_DEBUG_DEATH(EXPECT_FALSE(Footprint::Parse("-3x4")), "");
  EXPECT_FALSE(Footprint::Parse("10x4"));
}

TEST(FootprintTest, Bitrates) {
  EXPECT_NEAR(Footprint::Get4x4().Bitrate(), 8.f, 0.01f);
  EXPECT_NEAR(Footprint::Get5x4().Bitrate(), 6.4f, 0.01f);
  EXPECT_NEAR(Footprint::Get5x5().Bitrate(), 5.12f, 0.01f);
  EXPECT_NEAR(Footprint::Get6x5().Bitrate(), 4.27f, 0.01f);
  EXPECT_NEAR(Footprint::Get6x6().Bitrate(), 3.56f, 0.01f);
  EXPECT_NEAR(Footprint::Get8x5().Bitrate(), 3.20f, 0.01f);
  EXPECT_NEAR(Footprint::Get8x6().Bitrate(), 2.67f, 0.01f);
  EXPECT_NEAR(Footprint::Get8x8().Bitrate(), 2.00f, 0.01f);
  EXPECT_NEAR(Footprint::Get10x5().Bitrate(), 2.56f, 0.01f);
  EXPECT_NEAR(Footprint::Get10x6().Bitrate(), 2.13f, 0.01f);
  EXPECT_NEAR(Footprint::Get10x8().Bitrate(), 1.60f, 0.01f);
  EXPECT_NEAR(Footprint::Get10x10().Bitrate(), 1.28f, 0.01f);
  EXPECT_NEAR(Footprint::Get12x10().Bitrate(), 1.07f, 0.01f);
  EXPECT_NEAR(Footprint::Get12x12().Bitrate(), 0.89f, 0.01f);
}

TEST(FootprintTest, StorageRequirements) {
  auto footprint = Footprint::Get10x8();
  EXPECT_EQ(footprint.Width(), 10);
  EXPECT_EQ(footprint.Height(), 8);

  // If we have 8x8 blocks, then we have 64*16 = 1024 bytes.
  EXPECT_EQ(footprint.StorageRequirements(80, 64), 1024);

  // If our block is a little smaller this still counts because we need to
  // cover a partial block with a fully encoded one.
  EXPECT_EQ(footprint.StorageRequirements(79, 63), 1024);
}

}  // namespace

}  // namespace astc_codec
