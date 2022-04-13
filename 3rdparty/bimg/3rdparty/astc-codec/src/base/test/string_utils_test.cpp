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

#include "src/base/string_utils.h"

#include <gtest/gtest.h>

#include <list>
#include <string>
#include <vector>

namespace astc_codec {
namespace base {

TEST(StringUtils, Split) {
  std::vector<std::string> results;

  auto testFunc = [&results](std::string&& s) {
    results.push_back(std::move(s));
  };

  Split("", "abc", testFunc);
  EXPECT_EQ(results.size(), 1);

  Split("abc", "", testFunc);
  EXPECT_EQ(results.size(), 1);

  results.clear();
  Split("abc", "a", testFunc);
  EXPECT_EQ(results.size(), 2);
  EXPECT_EQ(results[0], "");
  EXPECT_EQ(results[1], "bc");

  results.clear();
  Split("aaa", "a", testFunc);
  EXPECT_EQ(4, results.size());
  EXPECT_EQ("", results[0]);
  EXPECT_EQ("", results[1]);
  EXPECT_EQ("", results[2]);
  EXPECT_EQ("", results[3]);

  results.clear();
  Split("1a2a3a4", "a", testFunc);
  EXPECT_EQ(4, results.size());
  EXPECT_EQ("1", results[0]);
  EXPECT_EQ("2", results[1]);
  EXPECT_EQ("3", results[2]);
  EXPECT_EQ("4", results[3]);

  results.clear();
  Split("1a2aa3a4", "a", testFunc);
  EXPECT_EQ(5, results.size());
  EXPECT_EQ("1", results[0]);
  EXPECT_EQ("2", results[1]);
  EXPECT_EQ("", results[2]);
  EXPECT_EQ("3", results[3]);
  EXPECT_EQ("4", results[4]);

  results.clear();
  Split("The quick brown fox jumped over the lazy dog",
        " ", testFunc);
  EXPECT_EQ(9, results.size());
  EXPECT_EQ("The", results[0]);
  EXPECT_EQ("quick", results[1]);
  EXPECT_EQ("brown", results[2]);
  EXPECT_EQ("fox", results[3]);
  EXPECT_EQ("jumped", results[4]);
  EXPECT_EQ("over", results[5]);
  EXPECT_EQ("the", results[6]);
  EXPECT_EQ("lazy", results[7]);
  EXPECT_EQ("dog", results[8]);

  results.clear();
  Split("a; b; c; d", "; ", testFunc);
  EXPECT_EQ(4, results.size());
  EXPECT_EQ("a", results[0]);
  EXPECT_EQ("b", results[1]);
  EXPECT_EQ("c", results[2]);
  EXPECT_EQ("d", results[3]);
}

TEST(StringUtils, ParseInt32) {
  EXPECT_EQ(ParseInt32("0", -1), 0);
  EXPECT_EQ(ParseInt32("100", -1), 100);
  EXPECT_EQ(ParseInt32("-100", -1), -100);

  EXPECT_EQ(ParseInt32("", -1), -1);
  EXPECT_EQ(ParseInt32("a", -1), -1);
  EXPECT_EQ(ParseInt32("10x1", -1), 10);

  EXPECT_EQ(ParseInt32("2147483647", -1), 2147483647);
  EXPECT_EQ(ParseInt32("2147483648", -1), 2147483647);

  EXPECT_EQ(ParseInt32("-2147483648", -1), -2147483648);
  EXPECT_EQ(ParseInt32("-2147483649", -1), -2147483648);
}

}  // namespace base
}  // namespace astc_codec
