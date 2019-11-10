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

#ifndef ASTC_CODEC_BASE_STRING_UTILS_H_
#define ASTC_CODEC_BASE_STRING_UTILS_H_

#include <limits>
#include <string>
#include <stdlib.h>

namespace astc_codec {
namespace base {

// Iterates over a string's parts using |splitBy| as a delimiter.
// |splitBy| must be a nonempty string well, or it's a no-op.
// Otherwise, |func| is called on each of the splits, excluding the
// characters that are part of |splitBy|.  If two |splitBy|'s occur in a row,
// |func| will be called on a StringView("") in between. See
// StringUtils_unittest.cpp for the full story.
template<class Func>
void Split(const std::string& str, const std::string& splitBy, Func func) {
  if (splitBy.empty()) {
    return;
  }

  size_t splitSize = splitBy.size();
  size_t begin = 0;
  size_t end = str.find(splitBy);

  while (true) {
    func(str.substr(begin, end - begin));
    if (end == std::string::npos) {
      return;
    }

    begin = end + splitSize;
    end = str.find(splitBy, begin);
  }
}

static int32_t ParseInt32(const char* str, int32_t deflt) {
  using std::numeric_limits;

  char* error = nullptr;
  int64_t value = strtol(str, &error, 0);
  // Limit long values to int32 min/max.  Needed for lp64; no-op on 32 bits.
  if (value > std::numeric_limits<int32_t>::max()) {
    value = std::numeric_limits<int32_t>::max();
  } else if (value < std::numeric_limits<int32_t>::min()) {
    value = std::numeric_limits<int32_t>::min();
  }
  return (error == str) ? deflt : static_cast<int32_t>(value);
}

}  // namespace base
}  // namespace astc_codec

#endif  // ASTC_CODEC_BASE_STRING_UTILS_H_
