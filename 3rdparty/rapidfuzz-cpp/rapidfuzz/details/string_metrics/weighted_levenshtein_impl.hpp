/* SPDX-License-Identifier: MIT */
/* Copyright © 2021 Max Bachmann */

#include <rapidfuzz/details/common.hpp>
#include <rapidfuzz/details/intrinsics.hpp>

#include <algorithm>
#include <array>
#include <limits>
#include <stdexcept>
#include <string>

namespace rapidfuzz {
namespace string_metric {
namespace detail {

/*
 * An encoded mbleven model table.
 *
 * Each 8-bit integer represents an edit sequence, with using two
 * bits for a single operation.
 *
 * Each Row of 8 integers represent all possible combinations
 * of edit sequences for a gived maximum edit distance and length
 * difference between the two strings, that is below the maximum
 * edit distance
 *
 *   0x1 = 01 = DELETE,
 *   0x2 = 10 = INSERT
 *
 * 0x5 -> DEL + DEL
 * 0x6 -> DEL + INS
 * 0x9 -> INS + DEL
 * 0xA -> INS + INS
 */
static constexpr uint8_t weighted_levenshtein_mbleven2018_matrix[14][7] = {
    /* max edit distance 1 */
    {0},
    /* case does not occur */ /* len_diff 0 */
    {0x01},                   /* len_diff 1 */
    /* max edit distance 2 */
    {0x09, 0x06}, /* len_diff 0 */
    {0x01},       /* len_diff 1 */
    {0x05},       /* len_diff 2 */
    /* max edit distance 3 */
    {0x09, 0x06},       /* len_diff 0 */
    {0x25, 0x19, 0x16}, /* len_diff 1 */
    {0x05},             /* len_diff 2 */
    {0x15},             /* len_diff 3 */
    /* max edit distance 4 */
    {0x96, 0x66, 0x5A, 0x99, 0x69, 0xA5}, /* len_diff 0 */
    {0x25, 0x19, 0x16},                   /* len_diff 1 */
    {0x65, 0x56, 0x95, 0x59},             /* len_diff 2 */
    {0x15},                               /* len_diff 3 */
    {0x55},                               /* len_diff 4 */
};

template <typename CharT1, typename CharT2>
std::size_t weighted_levenshtein_mbleven2018(basic_string_view<CharT1> s1,
                                             basic_string_view<CharT2> s2, std::size_t max)
{
    if (s1.size() < s2.size()) {
        return weighted_levenshtein_mbleven2018(s2, s1, max);
    }

    std::size_t len_diff = s1.size() - s2.size();
    auto possible_ops =
        weighted_levenshtein_mbleven2018_matrix[(max + max * max) / 2 + len_diff - 1];
    std::size_t dist = max + 1;

    for (int pos = 0; possible_ops[pos] != 0; ++pos) {
        int ops = possible_ops[pos];
        std::size_t s1_pos = 0;
        std::size_t s2_pos = 0;
        std::size_t cur_dist = 0;

        while (s1_pos < s1.size() && s2_pos < s2.size()) {
            if (common::mixed_sign_unequal(s1[s1_pos], s2[s2_pos])) {
                cur_dist++;

                if (!ops) break;
                if (ops & 1)
                    s1_pos++;
                else if (ops & 2)
                    s2_pos++;
                ops >>= 2;
            }
            else {
                s1_pos++;
                s2_pos++;
            }
        }

        cur_dist += (s1.size() - s1_pos) + (s2.size() - s2_pos);
        dist = std::min(dist, cur_dist);
    }

    return (dist > max) ? (std::size_t)-1 : dist;
}

template <std::size_t N, typename CharT1>
static inline std::size_t
longest_common_subsequence_unroll(basic_string_view<CharT1> s1,
                            const common::PatternMatchVector* block,
                            std::size_t s2_len)
{
    std::uint64_t S[N];
    for (std::size_t i = 0; i < N; ++i)
    {
        S[i] = ~0x0ull;
    }

    for (const auto& ch1 : s1) {

        uint64_t carry = 0;
        std::uint64_t Matches[N];
        std::uint64_t u[N];
        std::uint64_t x[N];
        for (std::size_t i = 0; i < N; ++i)
        {
            Matches[i] = block[i].get(ch1);
            u[i] = S[i] & Matches[i];
            x[i] = intrinsics::addc64(S[i], u[i], carry, &carry);
            S[i] = x[i] | (S[i] - u[i]);
        }
    }

    std::size_t res = 0;
    for (std::size_t i = 0; i < N; ++i)
    {
        res += intrinsics::popcount64(~S[i]);
    }
    return s1.size() + s2_len - 2 * res;
}

template <typename CharT1>
static inline std::size_t
longest_common_subsequence_blockwise(basic_string_view<CharT1> s1,
                            const common::BlockPatternMatchVector& block,
                            std::size_t s2_len)
{
    std::size_t words = block.m_val.size();
    std::vector<std::uint64_t> S(words, ~0x0ull);

    for (const auto& ch1 : s1) {
        uint64_t carry = 0;
        for (std::size_t word = 0; word < words; ++word) {
            const uint64_t Matches = block.get(word, ch1);
            uint64_t Stemp = S[word];

            uint64_t u = Stemp & Matches;

            uint64_t x = intrinsics::addc64(Stemp, u, carry, &carry);
            S[word] = x | (Stemp - u);
        }
    }

    std::size_t res = 0;
    for (uint64_t Stemp : S) {
        res += intrinsics::popcount64(~Stemp);
    }

    return s1.size() + s2_len - 2 * res;
}

template <typename CharT1>
std::size_t longest_common_subsequence(basic_string_view<CharT1> s1,
                            const common::BlockPatternMatchVector& block,
                            std::size_t s2_len)
{
    switch(block.m_val.size())
    {
    case 1:  return longest_common_subsequence_unroll<1>(s1, &block.m_val[0], s2_len);
    case 2:  return longest_common_subsequence_unroll<2>(s1, &block.m_val[0], s2_len);
    case 3:  return longest_common_subsequence_unroll<3>(s1, &block.m_val[0], s2_len);
    case 4:  return longest_common_subsequence_unroll<4>(s1, &block.m_val[0], s2_len);
    case 5:  return longest_common_subsequence_unroll<5>(s1, &block.m_val[0], s2_len);
    case 6:  return longest_common_subsequence_unroll<6>(s1, &block.m_val[0], s2_len);
    case 7:  return longest_common_subsequence_unroll<7>(s1, &block.m_val[0], s2_len);
    case 8:  return longest_common_subsequence_unroll<8>(s1, &block.m_val[0], s2_len);
    default: return longest_common_subsequence_blockwise(s1, block, s2_len);
    }
}

template <typename CharT1, typename CharT2>
std::size_t longest_common_subsequence(basic_string_view<CharT1> s1, basic_string_view<CharT2> s2)
{
    std::size_t nr = (s2.size() / 64) + (std::size_t)((s2.size() % 64) > 0);
    switch(nr)
    {
    case 1:
    {
        auto block = common::PatternMatchVector(s2);
        return longest_common_subsequence_unroll<1>(s1, &block, s2.size());
    }
    case 2:
    {
        auto block = common::BlockPatternMatchVector(s2);
        return longest_common_subsequence_unroll<2>(s1, &block.m_val[0], s2.size());
    }
    case 3:
    {
        auto block = common::BlockPatternMatchVector(s2);
        return longest_common_subsequence_unroll<3>(s1, &block.m_val[0], s2.size());
    }
    case 4:
    {
        auto block = common::BlockPatternMatchVector(s2);
        return longest_common_subsequence_unroll<4>(s1, &block.m_val[0], s2.size());
    }
    case 5:
    {
        auto block = common::BlockPatternMatchVector(s2);
        return longest_common_subsequence_unroll<5>(s1, &block.m_val[0], s2.size());
    }
    case 6:
    {
        auto block = common::BlockPatternMatchVector(s2);
        return longest_common_subsequence_unroll<6>(s1, &block.m_val[0], s2.size());
    }
    case 7:
    {
        auto block = common::BlockPatternMatchVector(s2);
        return longest_common_subsequence_unroll<7>(s1, &block.m_val[0], s2.size());
    }
    case 8:
    {
        auto block = common::BlockPatternMatchVector(s2);
        return longest_common_subsequence_unroll<8>(s1, &block.m_val[0], s2.size());
    }
    default:
    {
        auto block = common::BlockPatternMatchVector(s2);
        return longest_common_subsequence_blockwise(s1, block, s2.size());
    }
    }
}


// TODO this implementation needs some cleanup
template <typename CharT1, typename CharT2>
std::size_t weighted_levenshtein(basic_string_view<CharT1> s1,
                                 const common::BlockPatternMatchVector& block,
                                 basic_string_view<CharT2> s2, std::size_t max)
{
    // when no differences are allowed a direct comparision is sufficient
    if (max == 0) {
        if (s1.size() != s2.size()) {
            return (std::size_t)-1;
        }
        return std::equal(s1.begin(), s1.end(), s2.begin()) ? 0 : (std::size_t)-1;
    }

    // when the strings have a similar length each difference causes
    // at least a edit distance of 2, so a direct comparision is sufficient
    if (max == 1) {
        if (s1.size() == s2.size()) {
            return std::equal(s1.begin(), s1.end(), s2.begin()) ? 0 : (std::size_t)-1;
        }
    }

    // at least length difference insertions/deletions required
    std::size_t len_diff = (s1.size() < s2.size()) ? s2.size() - s1.size() : s1.size() - s2.size();
    if (len_diff > max) {
        return (std::size_t)-1;
    }

    // important to catch, since this causes block.m_val to be empty -> raises exception on access
    if (s2.empty()) {
        return s1.size();
    }

    // do this first, since we can not remove any affix in encoded form
    if (max >= 5) {
        std::size_t dist = longest_common_subsequence(s1, block, s2.size());
        return (dist > max) ? (std::size_t)-1 : dist;
    }

    // The Levenshtein distance between <prefix><string1><suffix> and <prefix><string2><suffix>
    // is similar to the distance between <string1> and <string2>, so they can be removed in linear
    // time
    common::remove_common_affix(s1, s2);

    if (s2.empty()) {
        return s1.size();
    }

    if (s1.empty()) {
        return s2.size();
    }

    return weighted_levenshtein_mbleven2018(s1, s2, max);
}

template <typename CharT1, typename CharT2>
std::size_t weighted_levenshtein(basic_string_view<CharT1> s1, basic_string_view<CharT2> s2,
                                 std::size_t max)
{
    // Swapping the strings so the second string is shorter
    if (s1.size() < s2.size()) {
        return weighted_levenshtein(s2, s1, max);
    }

    // when no differences are allowed a direct comparision is sufficient
    if (max == 0) {
        if (s1.size() != s2.size()) {
            return (std::size_t)-1;
        }
        return std::equal(s1.begin(), s1.end(), s2.begin()) ? 0 : (std::size_t)-1;
    }

    // when the strings have a similar length each difference causes
    // at least a edit distance of 2, so a direct comparision is sufficient
    if (max == 1) {
        if (s1.size() == s2.size()) {
            return std::equal(s1.begin(), s1.end(), s2.begin()) ? 0 : (std::size_t)-1;
        }
    }

    // at least length difference insertions/deletions required
    if (s1.size() - s2.size() > max) {
        return (std::size_t)-1;
    }

    // The Levenshtein distance between <prefix><string1><suffix> and <prefix><string2><suffix>
    // is similar to the distance between <string1> and <string2>, so they can be removed in linear
    // time
    common::remove_common_affix(s1, s2);

    if (s2.empty()) {
        return s1.size();
    }

    if (max < 5) {
        return weighted_levenshtein_mbleven2018(s1, s2, max);
    }

    std::size_t dist = longest_common_subsequence(s1, s2);
    return (dist > max) ? (std::size_t)-1 : dist;
}

template <typename CharT1, typename CharT2>
double
normalized_weighted_levenshtein(basic_string_view<CharT1> s1,
                                const common::BlockPatternMatchVector& block,
                                basic_string_view<CharT2> s2, const double score_cutoff)
{
    if (s1.empty() || s2.empty()) {
        return 100.0 * static_cast<double>(s1.empty() && s2.empty());
    }

    std::size_t lensum = s1.size() + s2.size();

    auto cutoff_distance = common::score_cutoff_to_distance(score_cutoff, lensum);

    std::size_t dist = weighted_levenshtein(s1, block, s2, cutoff_distance);
    return (dist != (std::size_t)-1) ? common::norm_distance(dist, lensum, score_cutoff) : 0.0;
}

template <typename CharT1, typename CharT2>
double normalized_weighted_levenshtein(basic_string_view<CharT1> s1, basic_string_view<CharT2> s2,
                                       const double score_cutoff)
{
    if (s1.empty() || s2.empty()) {
        return 100.0 * static_cast<double>(s1.empty() && s2.empty());
    }

    std::size_t lensum = s1.size() + s2.size();

    auto cutoff_distance = common::score_cutoff_to_distance(score_cutoff, lensum);

    std::size_t dist = weighted_levenshtein(s1, s2, cutoff_distance);
    return (dist != (std::size_t)-1) ? common::norm_distance(dist, lensum, score_cutoff) : 0.0;
}

} // namespace detail
} // namespace string_metric
} // namespace rapidfuzz
