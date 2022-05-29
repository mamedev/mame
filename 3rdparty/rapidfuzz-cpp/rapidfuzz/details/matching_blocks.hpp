// The MIT License (MIT)
//
// Copyright (c) 2020 Max Bachmann
// Copyright (c) 2014 Jean-Bernard Jansen
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in
// all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
// THE SOFTWARE.

#pragma once

#include <rapidfuzz/details/common.hpp>

#include <algorithm>
#include <tuple>
#include <unordered_map>
#include <vector>

namespace rapidfuzz {
namespace detail {
struct MatchingBlock {
    std::size_t spos;
    std::size_t dpos;
    std::size_t length;
    MatchingBlock(std::size_t aSPos, std::size_t aDPos, std::size_t aLength)
        : spos(aSPos), dpos(aDPos), length(aLength)
    {}
};

namespace difflib {

template <typename CharT1, typename CharT2>
class SequenceMatcher {
public:
    using match_t = std::tuple<std::size_t, std::size_t, std::size_t>;

    SequenceMatcher(basic_string_view<CharT1> a, basic_string_view<CharT2> b) : a_(a), b_(b)
    {
        j2len_.resize(b.size() + 1);
        for (std::size_t i = 0; i < b.size(); ++i) {
            b2j_.create(b[i]).push_back(i);
        }
    }

    match_t find_longest_match(std::size_t a_low, std::size_t a_high, std::size_t b_low, std::size_t b_high)
    {
        std::size_t best_i = a_low;
        std::size_t best_j = b_low;
        std::size_t best_size = 0;

        // Find longest junk free match
        {
            for (std::size_t i = a_low; i < a_high; ++i) {
                const auto& indexes = b2j_[a_[i]];
                std::size_t pos = 0;
                std::size_t next_val = 0;
                for (; pos < indexes.size(); pos++) {
                    std::size_t j = indexes[pos];
                    if (j < b_low) continue;

                    next_val = j2len_[j];
                    break;
                }

                for (; pos < indexes.size(); pos++) {
                    std::size_t j = indexes[pos];
                    if (j >= b_high) break;

                    std::size_t k = next_val + 1;

                    /* the next value might be overwritten below
                     * so cache it */
                    if (pos + 1 < indexes.size()) {
                        next_val = j2len_[indexes[pos + 1]];
                    }

                    j2len_[j + 1] = k;
                    if (k > best_size) {
                        best_i = i - k + 1;
                        best_j = j - k + 1;
                        best_size = k;
                    }
                }
            }

            std::fill(j2len_.begin() + static_cast<std::vector<std::size_t>::difference_type>(b_low),
                      j2len_.begin() + static_cast<std::vector<std::size_t>::difference_type>(b_high), 0);
        }

        while (best_i > a_low && best_j > b_low &&
               common::mixed_sign_equal(a_[best_i - 1], b_[best_j - 1])) {
            --best_i;
            --best_j;
            ++best_size;
        }

        while ((best_i + best_size) < a_high && (best_j + best_size) < b_high &&
               common::mixed_sign_equal(a_[best_i + best_size], b_[best_j + best_size]))
        {
            ++best_size;
        }

        return std::make_tuple(best_i, best_j, best_size);
    }

    std::vector<MatchingBlock> get_matching_blocks()
    {
        // The following are tuple extracting aliases
        std::vector<std::tuple<std::size_t, std::size_t, std::size_t, std::size_t>> queue;
        std::vector<match_t> matching_blocks_pass1;

        std::size_t queue_head = 0;
        queue.reserve(std::min(a_.size(), b_.size()));
        queue.emplace_back(0, a_.size(), 0, b_.size());

        while (queue_head < queue.size()) {
            std::size_t a_low, a_high, b_low, b_high;
            std::tie(a_low, a_high, b_low, b_high) = queue[queue_head++];
            std::size_t spos, dpos, length;
            std::tie(spos, dpos, length) = find_longest_match(a_low, a_high, b_low, b_high);
            if (length) {
                if (a_low < spos && b_low < dpos) {
                    queue.emplace_back(a_low, spos, b_low, dpos);
                }
                if ((spos + length) < a_high && (dpos + length) < b_high) {
                    queue.emplace_back(spos + length, a_high, dpos + length, b_high);
                }
                matching_blocks_pass1.emplace_back(spos, dpos, length);
            }
        }
        std::sort(std::begin(matching_blocks_pass1), std::end(matching_blocks_pass1));

        std::vector<MatchingBlock> matching_blocks;

        matching_blocks.reserve(matching_blocks_pass1.size());

        std::size_t i1, j1, k1;
        i1 = j1 = k1 = 0;

        for (match_t const& m : matching_blocks_pass1) {
            if (i1 + k1 == std::get<0>(m) && j1 + k1 == std::get<1>(m)) {
                k1 += std::get<2>(m);
            }
            else {
                if (k1) {
                    matching_blocks.emplace_back(i1, j1, k1);
                }
                std::tie(i1, j1, k1) = m;
            }
        }
        if (k1) {
            matching_blocks.emplace_back(i1, j1, k1);
        }
        matching_blocks.emplace_back(a_.size(), b_.size(), 0);

        return matching_blocks;
    }

protected:
    basic_string_view<CharT1> a_;
    basic_string_view<CharT2> b_;

private:
    // Cache to avoid reallocations
    std::vector<std::size_t> j2len_;
    common::CharHashTable<CharT2, std::vector<std::size_t>> b2j_;
};
} // namespace difflib

template <typename CharT1, typename CharT2>
std::vector<MatchingBlock> get_matching_blocks(basic_string_view<CharT1> s1,
                                               basic_string_view<CharT2> s2)
{
    return difflib::SequenceMatcher<CharT1, CharT2>(s1, s2).get_matching_blocks();
}

} /* namespace detail */
} /* namespace rapidfuzz */
