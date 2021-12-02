/* SPDX-License-Identifier: MIT */
/* Copyright © 2021 Max Bachmann */

#include <rapidfuzz/string_metric.hpp>
#include <stdexcept>

extern "C" int LLVMFuzzerTestOneInput(const uint8_t* data, size_t size) {
  if (size <= 4) {
    return 0;
  }
  // Extract Bytecode length:
  uint32_t codeLen = 0;
  memcpy(&codeLen, data, sizeof(codeLen));

  if (codeLen+4 > size || codeLen+4 < 4){
     return 0;
  }
  const uint8_t *code = &data[4];
  const uint8_t *input = &data[4+codeLen];

  rapidfuzz::basic_string_view<uint8_t> s1(code, codeLen);
  rapidfuzz::basic_string_view<uint8_t> s2(input,size-codeLen-4);

  {
    auto dist = rapidfuzz::string_metric::levenshtein(s1, s2, {1,1,1}, -1);
    auto reference_dist = rapidfuzz::string_metric::detail::generic_levenshtein(s1, s2, {1,1,1}, -1);
    if (dist != reference_dist)
    {
        throw std::logic_error("levenshtein distance failed");
    }
  }

  {
    auto dist = rapidfuzz::string_metric::levenshtein(s1, s2, {1,1,2}, -1);
    auto reference_dist = rapidfuzz::string_metric::detail::generic_levenshtein(s1, s2, {1,1,2}, -1);
    if (dist != reference_dist)
    {
        throw std::logic_error("InDel Distance failed");
    }
  }

  return 0;
}