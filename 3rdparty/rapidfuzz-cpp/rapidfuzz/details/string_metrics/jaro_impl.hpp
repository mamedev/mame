/* SPDX-License-Identifier: MIT */
/* Copyright © 2021 Max Bachmann */

#include <rapidfuzz/details/common.hpp>
#include <rapidfuzz/details/intrinsics.hpp>

namespace rapidfuzz {
namespace string_metric {
namespace detail {

template<typename CharT>
bool isnum(CharT val)
{
    return (val >= '0') && (val <= '9');
}

template <typename CharT1, typename CharT2>
static inline percent jaro_calculate_similarity(
    basic_string_view<CharT1> P, basic_string_view<CharT2> T,
    size_t CommonChars, size_t Transpositions
)
{
    Transpositions /= 2;
    double Sim = (double)CommonChars / (double)P.size() +
                 (double)CommonChars / (double)T.size() +
                 (double)(CommonChars - Transpositions) / (double)CommonChars;
    Sim /= 3.0;
    return Sim * 100;
}

template <typename CharT1, typename CharT2>
static inline bool jaro_length_filter(
    basic_string_view<CharT1> P, basic_string_view<CharT2> T, percent score_cutoff
)
{
    if (!T.size() || !P.size()) return false;

    double min_len = (double)std::min(P.size(), T.size());
    double Sim = (double)min_len / (double)P.size() +
                 (double)min_len / (double)T.size() +
                 1.0;
    Sim /= 3.0;
    return Sim * 100 >= score_cutoff;
}

template <typename CharT1, typename CharT2>
static inline bool jaro_common_char_filter(
    basic_string_view<CharT1> P, basic_string_view<CharT2> T, size_t CommonChars, percent score_cutoff
)
{
    if (!CommonChars) return false;

    double Sim = (double)CommonChars / (double)P.size() +
                 (double)CommonChars / (double)T.size() +
                 1.0;
    Sim /= 3.0;
    return Sim * 100 >= score_cutoff;
}

struct FlaggedCharsOriginal {
  std::vector<int> P_flag;
  std::vector<int> T_flag;
  std::size_t CommonChars;
};

template <typename CharT1, typename CharT2>
static inline FlaggedCharsOriginal flag_similar_characters_original(
    basic_string_view<CharT1> P, basic_string_view<CharT2> T
)
{
    std::vector<int> P_flag(P.size() + 1);
    std::vector<int> T_flag(T.size() + 1);

    std::size_t Bound = std::max(P.size(), T.size()) / 2;
    if (Bound > 0) Bound--;

    std::size_t CommonChars = 0;
    for (std::size_t i = 0; i < T.size(); i++) {
        std::size_t lowlim = (i >= Bound) ? i - Bound : 0;
        std::size_t hilim = (i + Bound <= P.size() - 1) ? (i + Bound) : P.size() - 1;
        for (std::size_t j = lowlim; j <= hilim; j++) {
            if (!P_flag[j] && common::mixed_sign_equal(P[j], T[i])) {
                T_flag[i] = 1;
                P_flag[j] = 1;
                CommonChars++;
                break;
            }
        }
    }

    return {P_flag, T_flag, CommonChars};
}

struct FlaggedCharsWord {
  uint64_t P_flag;
  uint64_t T_flag;
  std::size_t CommonChars;
};

template <typename CharT1, typename CharT2>
static inline FlaggedCharsWord flag_similar_characters_word(
  const common::PatternMatchVector& PM, basic_string_view<CharT1> P, basic_string_view<CharT2> T
)
{
    using namespace intrinsics;
    assert(P.size() <= 64);
    assert(T.size() <= 64);

    uint64_t P_flag = 0;
    uint64_t T_flag = 0;
    uint64_t Bound = std::max(P.size(), T.size()) / 2 - 1;
    uint64_t BoundMask = (1ull << 1 << Bound) - 1;

    std::size_t j = 0;
    for (; j < std::min(Bound, (uint64_t)T.size()); ++j)
    {
        uint64_t PM_j = PM.get(T[j]) & BoundMask & (~P_flag);

        P_flag |= blsi(PM_j);
        T_flag |= (uint64_t)(PM_j != 0) << j;

        BoundMask = (BoundMask << 1) | 1;
    }

    for (; j < std::min((uint64_t)T.size(), (uint64_t)P.size() + Bound); ++j)
    {
        uint64_t PM_j = PM.get(T[j]) & BoundMask & (~P_flag);

        P_flag |= blsi(PM_j);
        T_flag |= (uint64_t)(PM_j != 0) << j;

        BoundMask <<= 1;
    }

    return {P_flag, T_flag, popcount64(P_flag)};
}

template <typename CharT>
static inline size_t count_transpositions_word(
  const common::PatternMatchVector& PM, uint64_t P_mapped,
  basic_string_view<CharT> T, uint64_t T_mapped
)
{
    using namespace intrinsics;
    size_t Transpositions = 0;
    while (T_mapped)
    {
        uint64_t PatternFlagMask = blsi(P_mapped);
        

        Transpositions += !(PM.get(T[tzcnt(T_mapped)]) & PatternFlagMask);

        T_mapped = blsr(T_mapped);
        P_mapped ^= PatternFlagMask;
    }

    return Transpositions;
}


template <typename CharT1, typename CharT2>
double jaro_similarity_word(basic_string_view<CharT1> P, basic_string_view<CharT2> T, percent score_cutoff)
{
    if (!jaro_length_filter(P, T, score_cutoff))
    {
        return 0.0;
    }

    common::PatternMatchVector PM(P);

    auto flagged = flag_similar_characters_word(PM, P, T);

    if (!jaro_common_char_filter(P, T, flagged.CommonChars, score_cutoff))
    {
        return 0.0;
    }

    size_t Transpositions = count_transpositions_word(PM, flagged.P_flag, T, flagged.T_flag);

    return common::result_cutoff(jaro_calculate_similarity(P, T, flagged.CommonChars, Transpositions), score_cutoff);
}

template <typename CharT1, typename CharT2>
percent jaro_similarity_original(basic_string_view<CharT2> P, basic_string_view<CharT1> T, percent score_cutoff)
{
    if (!jaro_length_filter(P, T, score_cutoff))
    {
        return 0.0;
    }

    auto flagged = flag_similar_characters_original(P, T);

    if (!jaro_common_char_filter(P, T, flagged.CommonChars, score_cutoff))
    {
        return 0.0;
    }
    // Count the number of transpositions
    std::size_t Transpositions = 0;
    std::size_t k = 0;
    for (std::size_t i = 0; i < T.size(); i++) {
        if (flagged.T_flag[i]) {
            std::size_t j = k;
            for (; j < P.size(); j++) {
                if (flagged.P_flag[j]) {
                    k = j + 1;
                    break;
                }
            }
            if (common::mixed_sign_unequal(T[i], P[j])) {
                Transpositions++;
            }
        }
    }

    return common::result_cutoff(jaro_calculate_similarity(P, T, flagged.CommonChars, Transpositions), score_cutoff);
}

template <typename CharT1, typename CharT2>
percent jaro_similarity(basic_string_view<CharT2> P, basic_string_view<CharT1> T, percent score_cutoff)
{
    if (P.size() <= 64 && P.size() <= 64)
    {
        return jaro_similarity_word(P, T, score_cutoff);
    }
    else
    {
        return jaro_similarity_original(P, T, score_cutoff);
    }
}

template <typename CharT1, typename CharT2>
percent jaro_winkler_similarity(basic_string_view<CharT2> P, basic_string_view<CharT1> T, double prefix_weight, percent score_cutoff)
{
    std::size_t min_len = std::min(P.size(), T.size());
    std::size_t prefix = 0;
    std::size_t max_prefix = (min_len >= 4) ? 4 : min_len;

    for (; prefix < max_prefix; ++prefix)
    {
        if (!common::mixed_sign_equal(T[prefix], P[prefix]) || isnum(T[prefix]))
        {
            break;
        }
    }

    double jaro_score_cutoff = score_cutoff;
    if (jaro_score_cutoff > 70)
    {
        double prefix_sim = prefix * prefix_weight * 100;

        if (prefix_sim == 100)
        {
            jaro_score_cutoff = 70;
        }
        else
        {
            jaro_score_cutoff = std::max(70.0, (prefix_sim - jaro_score_cutoff) / (prefix_sim - 100.0));
        }
    }

    double Sim = jaro_similarity(P, T, jaro_score_cutoff);
    if (Sim > 70)
    {
        Sim += prefix * prefix_weight * (100 - Sim);
    }

    return common::result_cutoff(Sim, score_cutoff);;
}

} // namespace detail
} // namespace string_metric
} // namespace rapidfuzz
