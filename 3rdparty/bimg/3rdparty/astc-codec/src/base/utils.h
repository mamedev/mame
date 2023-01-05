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

#ifndef ASTC_CODEC_BASE_UTILS_H_
#define ASTC_CODEC_BASE_UTILS_H_

#include <cassert>
#include <cstdio>
#include <cstdlib>

#ifdef NDEBUG
#define UTILS_RELEASE_ASSERT(x)                                        \
  do {                                                                 \
    const bool result = (x);                                           \
    if (!result) {                                                     \
      fprintf(stderr, "Error: UTILS_RELEASE_ASSERT failed: %s\n", #x); \
      abort();                                                         \
    }                                                                  \
  } while (false)
#else
#define UTILS_RELEASE_ASSERT(x) assert(x)
#endif

#endif  // ASTC_CODEC_BASE_UTILS_H_
