/* SPDX-License-Identifier: MIT */
/* Copyright © 2021 Max Bachmann */

#include <algorithm>
#include <array>
#include <limits>
#include <numeric>
#include <rapidfuzz/details/common.hpp>
#include <rapidfuzz/details/intrinsics.hpp>

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
 *   01 = DELETE, 10 = INSERT, 11 = SUBSTITUTE
 *
 * For example, 3F -> 0b111111 means three substitutions
 */
static constexpr uint8_t levenshtein_mbleven2018_matrix[9][8] = {
    /* max edit distance 1 */
    {0x03}, /* len_diff 0 */
    {0x01}, /* len_diff 1 */
    /* max edit distance 2 */
    {0x0F, 0x09, 0x06}, /* len_diff 0 */
    {0x0D, 0x07},       /* len_diff 1 */
    {0x05},             /* len_diff 2 */
    /* max edit distance 3 */
    {0x3F, 0x27, 0x2D, 0x39, 0x36, 0x1E, 0x1B}, /* len_diff 0 */
    {0x3D, 0x37, 0x1F, 0x25, 0x19, 0x16},       /* len_diff 1 */
    {0x35, 0x1D, 0x17},                         /* len_diff 2 */
    {0x15},                                     /* len_diff 3 */
};

template <typename CharT1, typename CharT2>
std::size_t levenshtein_mbleven2018(basic_string_view<CharT1> s1, basic_string_view<CharT2> s2,
                                    std::size_t max)
{
    if (s1.size() < s2.size()) {
        return levenshtein_mbleven2018(s2, s1, max);
    }

    std::size_t len_diff = s1.size() - s2.size();
    auto possible_ops = levenshtein_mbleven2018_matrix[(max + max * max) / 2 + len_diff - 1];
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
                if (ops & 1) s1_pos++;
                if (ops & 2) s2_pos++;
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

/**
 * @brief Bitparallel implementation of the Levenshtein distance.
 *
 * This implementation requires the first string to have a length <= 64.
 * The algorithm used is described @cite hyrro_2002 and has a time complexity
 * of O(N). Comments and variable names in the implementation follow the
 * paper. This implementation is used internally when the strings are short enough
 *
 * @tparam CharT1 This is the char type of the first sentence
 * @tparam CharT2 This is the char type of the second sentence
 *
 * @param s1
 *   string to compare with s2 (for type info check Template parameters above)
 * @param s2
 *   string to compare with s1 (for type info check Template parameters above)
 *
 * @return returns the levenshtein distance between s1 and s2
 */
template <typename CharT1>
std::size_t levenshtein_hyrroe2003(basic_string_view<CharT1> s2,
                                   const common::PatternMatchVector& PM,
                                   std::size_t s1_len, std::size_t max)
{
    /* VP is set to 1^m. Shifting by bitwidth would be undefined behavior */
    uint64_t VP = (uint64_t)-1;
    uint64_t VN = 0;
    std::size_t currDist = s1_len;

    // saturated addition + subtraction to limit maxMisses to a range of 0 <-> (size_t)-1
    // make sure a wraparound can never occur
    std::size_t maxMisses = 0;
    if (s1_len > s2.size()) {
        if (s1_len - s2.size() < max) {
            maxMisses = max - (s1_len - s2.size());
        }
        else {
            // minimum is 0
            maxMisses = 0;
        }
    }
    else {
        maxMisses = s2.size() - s1_len;
        if (max <= std::numeric_limits<std::size_t>::max() - maxMisses) {
            maxMisses = max + maxMisses;
        }
        else {
            // max is (size_t)-1
            maxMisses = std::numeric_limits<std::size_t>::max();
        }
    }

    /* mask used when computing D[m,j] in the paper 10^(m-1) */
    uint64_t mask = (uint64_t)1 << (s1_len - 1);

    /* Searching */
    for (const auto& ch2 : s2) {
        /* Step 1: Computing D0 */
        uint64_t PM_j = PM.get(ch2);
        uint64_t X = PM_j | VN;
        uint64_t D0 = (((X & VP) + VP) ^ VP) | X;

        /* Step 2: Computing HP and HN */
        uint64_t HP = VN | ~(D0 | VP);
        uint64_t HN = D0 & VP;

        /* Step 3: Computing the value D[m,j] */
        // modification: early exit using maxMisses
        if (HP & mask) {
            currDist++;
            if (maxMisses < 2) {
                return (std::size_t)-1;
            }
            maxMisses -= 2;
        }
        else if (HN & mask) {
            currDist--;
        }
        else {
            if (maxMisses < 1) {
                return (std::size_t)-1;
            }
            --maxMisses;
        }

        /* Step 4: Computing Vp and VN */
        X = (HP << 1) | 1;
        VP = (HN << 1) | ~(D0 | X);
        VN = X & D0;
    }

    return currDist;
}

template <typename CharT1>
std::size_t levenshtein_hyrroe2003(basic_string_view<CharT1> s2,
                                   const common::PatternMatchVector& PM,
                                   std::size_t s1_len)
{
    /* VP is set to 1^m. Shifting by bitwidth would be undefined behavior */
    uint64_t VP = (uint64_t)-1;
    uint64_t VN = 0;
    std::size_t currDist = s1_len;

    /* mask used when computing D[m,j] in the paper 10^(m-1) */
    uint64_t mask = (uint64_t)1 << (s1_len - 1);

    /* Searching */
    for (const auto& ch2 : s2) {
        /* Step 1: Computing D0 */
        uint64_t PM_j = PM.get(ch2);
        uint64_t X = PM_j | VN;
        uint64_t D0 = (((X & VP) + VP) ^ VP) | X;

        /* Step 2: Computing HP and HN */
        uint64_t HP = VN | ~(D0 | VP);
        uint64_t HN = D0 & VP;

        /* Step 3: Computing the value D[m,j] */
        // modification: early exit using maxMisses
        currDist += !!(HP & mask);
        currDist -= !!(HN & mask);

        /* Step 4: Computing Vp and VN */
        X = (HP << 1) | 1;
        VP = (HN << 1) | ~(D0 | X);
        VN = X & D0;
    }

    return currDist;
}

template <typename CharT1>
std::size_t
levenshtein_myers1999_block(basic_string_view<CharT1> s2,
                            const common::BlockPatternMatchVector& PM,
                            std::size_t s1_len, std::size_t max)
{
    struct Vectors {
        uint64_t Mv;
        uint64_t Pv;

        Vectors() : Mv(0), Pv(~0x0ull)
        {}
    };

    const std::size_t words = PM.m_val.size();
    std::size_t currDist = s1_len;

    // saturated addition + subtraction to limit maxMisses to a range of 0 <-> (size_t)-1
    // make sure a wraparound can never occur
    std::size_t maxMisses = 0;
    if (s1_len > s2.size()) {
        if (s1_len - s2.size() < max) {
            maxMisses = max - (s1_len - s2.size());
        }
        else {
            // minimum is 0
            maxMisses = 0;
        }
    }
    else {
        maxMisses = s2.size() - s1_len;
        if (max <= std::numeric_limits<std::size_t>::max() - maxMisses) {
            maxMisses = max + maxMisses;
        }
        else {
            // max is (size_t)-1
            maxMisses = std::numeric_limits<std::size_t>::max();
        }
    }

    std::vector<Vectors> vecs(words);
    const uint64_t Last = (uint64_t)1 << ((s1_len - 1) % 64);

    for (std::size_t i = 0; i < s2.size(); i++) {
        uint64_t Pb = 1;
        uint64_t Mb = 0;

        for (std::size_t word = 0; word < words - 1; word++) {
            const uint64_t PM_j = PM.get(word, s2[i]);
            const uint64_t Mv = vecs[word].Mv;
            const uint64_t Pv = vecs[word].Pv;

            const uint64_t Xv = PM_j | Mv;
            const uint64_t Xh = ((((PM_j | Mb) & Pv) + Pv) ^ Pv) | PM_j | Mb;

            uint64_t Ph = Mv | ~(Xh | Pv);
            uint64_t Mh = Pv & Xh;

            const uint64_t PbTemp = Pb;
            Pb = Ph >> 63;
            Ph = (Ph << 1) | PbTemp;

            const uint64_t MbTemp = Mb;
            Mb = Mh >> 63;
            Mh = (Mh << 1) | MbTemp;

            vecs[word].Pv = Mh | ~(Xv | Ph);
            vecs[word].Mv = Ph & Xv;
        }

        // distance only has to be incremented/decremented in the last word
        {
            const uint64_t PM_j = PM.get(words - 1, s2[i]);
            const uint64_t Mv = vecs[words - 1].Mv;
            const uint64_t Pv = vecs[words - 1].Pv;

            const uint64_t Xv = PM_j | Mv;
            const uint64_t Xh = ((((PM_j | Mb) & Pv) + Pv) ^ Pv) | PM_j | Mb;

            uint64_t Ph = Mv | ~(Xh | Pv);
            uint64_t Mh = Pv & Xh;

            // modification: early exit using maxMisses
            if (Ph & Last) {
                currDist++;
                if (maxMisses < 2) {
                    return (std::size_t)-1;
                }
                maxMisses -= 2;
            }
            else if (Mh & Last) {
                currDist--;
            }
            else {
                if (maxMisses < 1) {
                    return (std::size_t)-1;
                }
                --maxMisses;
            }

            Ph = (Ph << 1) | Pb;
            Mh = (Mh << 1) | Mb;

            vecs[words - 1].Pv = Mh | ~(Xv | Ph);
            vecs[words - 1].Mv = Ph & Xv;
        }
    }

    return currDist;
}

template <typename CharT1, typename CharT2>
std::size_t levenshtein(basic_string_view<CharT1> s1,
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
    if (max >= 4) {
        std::size_t dist = 0;
        if (s2.size() < 65) {
            if (max == (size_t)-1) {
                dist = levenshtein_hyrroe2003(s1, block.m_val[0], s2.size());
            }
            else {
                dist = levenshtein_hyrroe2003(s1, block.m_val[0], s2.size(), max);
            }
        }
        else {
            dist = levenshtein_myers1999_block(s1, block, s2.size(), max);
        }

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

    return levenshtein_mbleven2018(s1, s2, max);
}

template <typename CharT1, typename CharT2>
std::size_t levenshtein(basic_string_view<CharT1> s1, basic_string_view<CharT2> s2, std::size_t max)
{
    /* Swapping the strings so the first string is shorter.
     * Swapping has no effect on the score since Insertion and Deletion have the
     * the same weight */
    if (s1.size() > s2.size()) {
        return levenshtein(s2, s1, max);
    }

    // when no differences are allowed a direct comparision is sufficient
    if (max == 0) {
        if (s1.size() != s2.size()) {
            return (std::size_t)-1;
        }
        return std::equal(s1.begin(), s1.end(), s2.begin()) ? 0 : (std::size_t)-1;
    }

    // at least length difference insertions/deletions required
    if (s2.size() - s1.size() > max) {
        return (std::size_t)-1;
    }

    /* The Levenshtein distance between
     * <prefix><string1><suffix> and <prefix><string2><suffix>
     * is similar to the distance between <string1> and <string2>,
     * so they can be removed in linear time */
    common::remove_common_affix(s1, s2);

    if (s1.empty()) {
        return s2.size();
    }

    if (max < 4) {
        return levenshtein_mbleven2018(s1, s2, max);
    }

    /* when the short strings has less then 65 elements Hyyrös' algorithm can be used */
    if (s2.size() < 65) {
        std::size_t dist;
        if (max == (size_t)-1) {
            dist = levenshtein_hyrroe2003(s1, common::PatternMatchVector(s2), s2.size());
        }
        else {
            dist = levenshtein_hyrroe2003(s1, common::PatternMatchVector(s2), s2.size(), max);
        }
        return (dist > max) ? (std::size_t)-1 : dist;
    }

    std::size_t dist = levenshtein_myers1999_block(s1, common::BlockPatternMatchVector(s2),
                                                   s2.size(), max);

    return (dist > max) ? (std::size_t)-1 : dist;
}

template <typename CharT1, typename CharT2>
double normalized_levenshtein(basic_string_view<CharT1> s1,
                              const common::BlockPatternMatchVector& block,
                              basic_string_view<CharT2> s2, const double score_cutoff)
{
    if (s1.empty() || s2.empty()) {
        return 100.0 * static_cast<double>(s1.empty() && s2.empty());
    }

    /* calculate the maximum possible edit distance with
     * Insertion/Deletion/Substitution = 1 */
    std::size_t max_dist = std::max(s1.size(), s2.size());

    auto cutoff_distance = common::score_cutoff_to_distance(score_cutoff, max_dist);

    std::size_t dist = levenshtein(s1, block, s2, cutoff_distance);
    return (dist != (std::size_t)-1) ? common::norm_distance(dist, max_dist, score_cutoff) : 0.0;
}

template <typename CharT1, typename CharT2>
double normalized_levenshtein(basic_string_view<CharT1> s1, basic_string_view<CharT2> s2,
                              const double score_cutoff)
{
    if (s1.empty() || s2.empty()) {
        return 100.0 * static_cast<double>(s1.empty() && s2.empty());
    }

    /* calculate the maximum possible edit distance with
     * Insertion/Deletion/Substitution = 1 */
    std::size_t max_dist = std::max(s1.size(), s2.size());

    auto cutoff_distance = common::score_cutoff_to_distance(score_cutoff, max_dist);

    std::size_t dist = levenshtein(s1, s2, cutoff_distance);
    return (dist != (std::size_t)-1) ? common::norm_distance(dist, max_dist, score_cutoff) : 0.0;
}

} // namespace detail
} // namespace string_metric
} // namespace rapidfuzz
