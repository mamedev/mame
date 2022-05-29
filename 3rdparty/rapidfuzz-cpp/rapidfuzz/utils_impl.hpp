/* SPDX-License-Identifier: MIT */
/* Copyright © 2020 Max Bachmann */

#include <algorithm>
#include <array>
#include <cctype>
#include <cwctype>
#include <limits>

#include <rapidfuzz/details/common.hpp>
#include <rapidfuzz/details/unicode.hpp>

namespace rapidfuzz {

template <typename CharT>
std::size_t utils::default_process(CharT* str, std::size_t len)
{
    /* mapping converting
     * - non alphanumeric characters to whitespace (32)
     * - alphanumeric characters to lowercase
     *
     * generated using
     * `[ord(chr(x).lower()) if chr(x).isalnum() else 0x20 for x in range(256)]`
     * in Python3.9
     */
    static const int extended_ascii_mapping[256] = {
        32,  32,  32,  32,  32,  32,  32,  32,  32,  32,  32,  32,  32,  32,  32,  32,  32,  32,
        32,  32,  32,  32,  32,  32,  32,  32,  32,  32,  32,  32,  32,  32,  32,  32,  32,  32,
        32,  32,  32,  32,  32,  32,  32,  32,  32,  32,  32,  32,  48,  49,  50,  51,  52,  53,
        54,  55,  56,  57,  32,  32,  32,  32,  32,  32,  32,  97,  98,  99,  100, 101, 102, 103,
        104, 105, 106, 107, 108, 109, 110, 111, 112, 113, 114, 115, 116, 117, 118, 119, 120, 121,
        122, 32,  32,  32,  32,  32,  32,  97,  98,  99,  100, 101, 102, 103, 104, 105, 106, 107,
        108, 109, 110, 111, 112, 113, 114, 115, 116, 117, 118, 119, 120, 121, 122, 32,  32,  32,
        32,  32,  32,  32,  32,  32,  32,  32,  32,  32,  32,  32,  32,  32,  32,  32,  32,  32,
        32,  32,  32,  32,  32,  32,  32,  32,  32,  32,  32,  32,  32,  32,  32,  32,  32,  32,
        32,  32,  32,  32,  32,  32,  32,  32,  170, 32,  32,  32,  32,  32,  32,  32,  178, 179,
        32,  181, 32,  32,  32,  185, 186, 32,  188, 189, 190, 32,  224, 225, 226, 227, 228, 229,
        230, 231, 232, 233, 234, 235, 236, 237, 238, 239, 240, 241, 242, 243, 244, 245, 246, 32,
        248, 249, 250, 251, 252, 253, 254, 223, 224, 225, 226, 227, 228, 229, 230, 231, 232, 233,
        234, 235, 236, 237, 238, 239, 240, 241, 242, 243, 244, 245, 246, 32,  248, 249, 250, 251,
        252, 253, 254, 255};

    std::transform(str, str + len, str, [](CharT ch) {
        /* irrelevant cases for a given char type are removed at compile time by any decent compiler
         */
        if (ch < 0 || rapidfuzz::common::to_unsigned(ch) > std::numeric_limits<uint32_t>::max()) {
            return ch;
        }
        else if (ch < 256) {
            return static_cast<CharT>(extended_ascii_mapping[rapidfuzz::common::to_unsigned(ch)]);
        }
        else {
            // this requires sources to compiled, while the current version for C++ is header only
            // this will be added to the C++ version later on.
#ifdef RAPIDFUZZ_PYTHON
            return static_cast<CharT>(Unicode::UnicodeDefaultProcess(static_cast<uint32_t>(ch)));
#else
      return ch;
#endif
        }
    });

    while (len > 0 && str[len - 1] == ' ') {
        len--;
    }

    std::size_t prefix = 0;
    while (len > 0 && str[prefix] == ' ') {
        len--;
        prefix++;
    }

    if (prefix != 0) {
        std::copy(str + prefix, str + prefix + len, str);
    }

    return len;
}

template <typename Sentence, typename CharT, typename>
std::basic_string<CharT> utils::default_process(Sentence&& s)
{
    std::basic_string<CharT> str(std::forward<Sentence>(s));

    std::size_t len = default_process(&str[0], str.size());
    str.resize(len);
    return str;
}

template <typename Sentence, typename CharT, typename>
std::basic_string<CharT> utils::default_process(Sentence s)
{
    return default_process(std::basic_string<CharT>(s.data(), s.size()));
}

} // namespace rapidfuzz
