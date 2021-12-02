/* SPDX-License-Identifier: MIT */
/* Copyright © 2021 Max Bachmann */

#pragma once
#include <rapidfuzz/details/common.hpp>
#include <rapidfuzz/details/string_metrics/generic_levenshtein_impl.hpp>
#include <rapidfuzz/details/string_metrics/jaro_impl.hpp>
#include <rapidfuzz/details/string_metrics/levenshtein_editops_impl.hpp>
#include <rapidfuzz/details/string_metrics/levenshtein_impl.hpp>
#include <rapidfuzz/details/string_metrics/weighted_levenshtein_impl.hpp>

#include <cmath>
#include <numeric>
#include <stdexcept>
#include <tuple>
#include <vector>

namespace rapidfuzz {
namespace string_metric {

/**
 * @defgroup string_metric string_metric
 * Different useful string_metrics
 * @{
 */

/**
 * @brief Calculates the minimum number of insertions, deletions, and substitutions
 * required to change one sequence into the other according to Levenshtein with custom
 * costs for insertion, deletion and substitution
 *
 * @tparam Sentence1 This is a string that can be converted to
 * basic_string_view<char_type>
 * @tparam Sentence2 This is a string that can be converted to
 * basic_string_view<char_type>
 *
 * @param s1
 *   string to compare with s2 (for type info check Template parameters above)
 * @param s2
 *   string to compare with s1 (for type info check Template parameters above)
 * @param weights
 *   The weights for the three operations in the form
 *   (insertion, deletion, substitution). Default is {1, 1, 1},
 *   which gives all three operations a weight of 1.
 * @param max
 *   Maximum Levenshtein distance between s1 and s2, that is
 *   considered as a result. If the distance is bigger than max,
 *   -1 is returned instead. Default is std::numeric_limits<std::size_t>::max(),
 *   which deactivates this behaviour.
 *
 * @return returns the levenshtein distance between s1 and s2
 *
 * @remarks
 * @parblock
 * Depending on the input parameters different optimized implementation are used
 * to improve the performance. Worst-case performance is ``O(m * n)``.
 *
 * <b>Insertion = Deletion = Substitution:</b>
 *
 *    This is known as uniform Levenshtein distance and is the distance most commonly
 *    referred to as Levenshtein distance. The following implementation is used
 *    with a worst-case performance of ``O([N/64]M)``.
 *
 *    - if max is 0 the similarity can be calculated using a direct comparision,
 *      since no difference between the strings is allowed.  The time complexity of
 *      this algorithm is ``O(N)``.
 *
 *    - A common prefix/suffix of the two compared strings does not affect
 *      the Levenshtein distance, so the affix is removed before calculating the
 *      similarity.
 *
 *    - If max is ≤ 3 the mbleven algorithm is used. This algorithm
 *      checks all possible edit operations that are possible under
 *      the threshold `max`. The time complexity of this algorithm is ``O(N)``.
 *
 *    - If the length of the shorter string is ≤ 64 after removing the common affix
 *      Hyyrös' algorithm is used, which calculates the Levenshtein distance in
 *      parallel. The algorithm is described by @cite hyrro_2002. The time complexity of this
 *      algorithm is ``O(N)``.
 *
 *    - If the length of the shorter string is ≥ 64 after removing the common affix
 *      a blockwise implementation of Myers' algorithm is used, which calculates
 *      the Levenshtein distance in parallel (64 characters at a time).
 *      The algorithm is described by @cite myers_1999. The time complexity of this
 *      algorithm is ``O([N/64]M)``.
 *
 *
 * <b>Insertion = Deletion, Substitution >= Insertion + Deletion:</b>
 *
 *    Since every Substitution can be performed as Insertion + Deletion, this variant
 *    of the Levenshtein distance only uses Insertions and Deletions. Therefore this
 *    variant is often referred to as InDel-Distance.  The following implementation
 *    is used with a worst-case performance of ``O([N/64]M)``.
 *
 *    - if max is 0 the similarity can be calculated using a direct comparision,
 *      since no difference between the strings is allowed.  The time complexity of
 *      this algorithm is ``O(N)``.
 *
 *    - if max is 1 and the two strings have a similar length, the similarity can be
 *      calculated using a direct comparision aswell, since a substitution would cause
 *      a edit distance higher than max. The time complexity of this algorithm
 *      is ``O(N)``.
 *
 *    - A common prefix/suffix of the two compared strings does not affect
 *      the Levenshtein distance, so the affix is removed before calculating the
 *      similarity.
 *
 *    - If max is ≤ 4 the mbleven algorithm is used. This algorithm
 *      checks all possible edit operations that are possible under
 *      the threshold `max`. As a difference to the normal Levenshtein distance this
 *      algorithm can even be used up to a threshold of 4 here, since the higher weight
 *      of substitutions decreases the amount of possible edit operations.
 *      The time complexity of this algorithm is ``O(N)``.
 *
 *    - If the length of the shorter string is ≤ 64 after removing the common affix
 *      Hyyrös' lcs algorithm is used, which calculates the InDel distance in
 *      parallel. The algorithm is described by @cite hyrro_lcs_2004 and is extended with support
 *      for UTF32 in this implementation. The time complexity of this
 *      algorithm is ``O(N)``.
 *
 *    - If the length of the shorter string is ≥ 64 after removing the common affix
 *      a blockwise implementation of Hyyrös' lcs algorithm is used, which calculates
 *      the Levenshtein distance in parallel (64 characters at a time).
 *      The algorithm is described by @cite hyrro_lcs_2004. The time complexity of this
 *      algorithm is ``O([N/64]M)``.
 *
 * <b>Other weights:</b>
 *
 *   The implementation for other weights is based on Wagner-Fischer.
 *   It has a performance of ``O(N * M)`` and has a memory usage of ``O(N)``.
 *   Further details can be found in @cite wagner_fischer_1974.
 * @endparblock
 *
 * @par Examples
 * @parblock
 * Find the Levenshtein distance between two strings:
 * @code{.cpp}
 * // dist is 2
 * std::size_t dist = levenshtein("lewenstein", "levenshtein");
 * @endcode
 *
 * Setting a maximum distance allows the implementation to select
 * a more efficient implementation:
 * @code{.cpp}
 * // dist is -1
 * std::size_t dist = levenshtein("lewenstein", "levenshtein", {1, 1, 1}, 1);
 * @endcode
 *
 * It is possible to select different weights by passing a `weight` struct.
 * @code{.cpp}
 * // dist is 3
 * std::size_t dist = levenshtein("lewenstein", "levenshtein", {1, 1, 2});
 * @endcode
 * @endparblock
 */
template <typename Sentence1, typename Sentence2>
std::size_t levenshtein(const Sentence1& s1, const Sentence2& s2,
                        LevenshteinWeightTable weights = {1, 1, 1},
                        std::size_t max = std::numeric_limits<std::size_t>::max())
{
    auto sentence1 = common::to_string_view(s1);
    auto sentence2 = common::to_string_view(s2);

    if (weights.insert_cost == weights.delete_cost) {
        /* when insertions + deletions operations are free there can not be any edit distance */
        if (weights.insert_cost == 0) {
            return 0;
        }

        /* uniform Levenshtein multiplied with the common factor */
        if (weights.insert_cost == weights.replace_cost) {
            // max can make use of the common divisor of the three weights
            const std::size_t new_max =
                max / weights.insert_cost + (std::size_t)(max % weights.insert_cost != 0);
            const std::size_t distance =
                detail::levenshtein(sentence1, sentence2, new_max) * weights.insert_cost;
            return (distance <= max) ? distance : (std::size_t)-1;
        }
        /*
         * when replace_cost >= insert_cost + delete_cost no substitutions are performed
         * therefore this can be implemented as InDel distance multiplied with the common factor
         */
        else if (weights.replace_cost >= weights.insert_cost + weights.delete_cost) {
            // max can make use of the common divisor of the three weights
            const std::size_t new_max =
                max / weights.insert_cost + (std::size_t)(max % weights.insert_cost != 0);
            const std::size_t distance =
                detail::weighted_levenshtein(sentence1, sentence2, new_max) * weights.insert_cost;
            return (distance <= max) ? distance : (std::size_t)-1;
        }
    }

    return detail::generic_levenshtein(sentence1, sentence2, weights, max);
}

template <typename Sentence1>
struct CachedLevenshtein {
    using CharT1 = char_type<Sentence1>;

    CachedLevenshtein(const Sentence1& s1, LevenshteinWeightTable aWeights = {1, 1, 1})
        : s1_view(common::to_string_view(s1)), blockmap_s1(s1_view), weights(aWeights)
    {}

    template <typename Sentence2>
    std::size_t distance(const Sentence2& s2,
                         std::size_t max = std::numeric_limits<std::size_t>::max()) const
    {
        auto s2_view = common::to_string_view(s2);

        if (weights.insert_cost == weights.delete_cost) {
            /* when insertions + deletions operations are free there can not be any edit distance */
            if (weights.insert_cost == 0) {
                return 0;
            }

            /* uniform Levenshtein multiplied with the common factor */
            if (weights.insert_cost == weights.replace_cost) {
                // max can make use of the common divisor of the three weights
                const std::size_t new_max =
                    max / weights.insert_cost + (std::size_t)(max % weights.insert_cost != 0);
                const std::size_t distance =
                    detail::levenshtein(s2_view, blockmap_s1, s1_view, new_max) *
                    weights.insert_cost;
                return (distance <= max) ? distance : (std::size_t)-1;
            }
            /*
             * when replace_cost >= insert_cost + delete_cost no substitutions are performed
             * therefore this can be implemented as InDel distance multiplied with the common factor
             */
            else if (weights.replace_cost >= weights.insert_cost + weights.delete_cost) {
                // max can make use of the common divisor of the three weights
                const std::size_t new_max =
                    max / weights.insert_cost + (std::size_t)(max % weights.insert_cost != 0);
                const std::size_t distance =
                    detail::weighted_levenshtein(s2_view, blockmap_s1, s1_view, new_max) *
                    weights.insert_cost;
                return (distance <= max) ? distance : (std::size_t)-1;
            }
        }

        return detail::generic_levenshtein(s1_view, s2_view, weights, max);
    }

private:
    rapidfuzz::basic_string_view<CharT1> s1_view;
    common::BlockPatternMatchVector blockmap_s1;
    LevenshteinWeightTable weights;
};

/**
 * @brief Calculates a normalized levenshtein distance using custom
 * costs for insertion, deletion and substitution.
 *
 * @tparam Sentence1 This is a string that can be converted to
 * basic_string_view<char_type>
 * @tparam Sentence2 This is a string that can be converted to
 * basic_string_view<char_type>
 *
 * @param s1
 *   string to compare with s2 (for type info check Template parameters above)
 * @param s2
 *   string to compare with s1 (for type info check Template parameters above)
 * @param weights
 *   The weights for the three operations in the form
 *   (insertion, deletion, substitution). Default is {1, 1, 1},
 *   which gives all three operations a weight of 1.
 * @param score_cutoff
 *   Optional argument for a score threshold as a float between 0 and 100.
 *   For ratio < score_cutoff 0 is returned instead. Default is 0,
 *   which deactivates this behaviour.
 *
 * @return Normalized weighted levenshtein distance between s1 and s2
 *   as a double between 0 and 100
 *
 * @see levenshtein()
 *
 * @remarks
 * @parblock
 * The normalization of the Levenshtein distance is performed in the following way:
 *
 * \f{align*}{
 *   \\
 *   dist_{max} &= \begin{cases}
 *     min(len(s1), len(s2)) \cdot sub,       & \text{if } sub \leq ins + del \\
 *     len(s1) \cdot del + len(s2) \cdot ins, & \text{otherwise}
 *   \end{cases}\\[10pt]
 *
 *   dist_{max} &= \begin{cases}
 *     dist_{max} + (len(s1) - len(s2)) \cdot del, & \text{if } len(s1) > len(s2) \\
 *     dist_{max} + (len(s2) - len(s1)) \cdot ins, & \text{if } len(s1) < len(s2) \\
 *     dist_{max},                                 & \text{if } len(s1) = len(s2)
 *   \end{cases}\\[10pt]
 *
 *   ratio &= 100 \cdot \frac{distance(s1, s2)}{dist_{max}}
 * \f}
 * @endparblock
 *
 *
 * @par Examples
 * @parblock
 * Find the normalized Levenshtein distance between two strings:
 * @code{.cpp}
 * // ratio is 81.81818181818181
 * double ratio = normalized_levenshtein("lewenstein", "levenshtein");
 * @endcode
 *
 * Setting a score_cutoff allows the implementation to select
 * a more efficient implementation:
 * @code{.cpp}
 * // ratio is 0.0
 * double ratio = normalized_levenshtein("lewenstein", "levenshtein", {1, 1, 1}, 85.0);
 * @endcode
 *
 * It is possible to select different weights by passing a `weight` struct
 * @code{.cpp}
 * // ratio is 85.71428571428571
 * double ratio = normalized_levenshtein("lewenstein", "levenshtein", {1, 1, 2});
 * @endcode
 * @endparblock
 */
template <typename Sentence1, typename Sentence2>
double normalized_levenshtein(const Sentence1& s1, const Sentence2& s2,
                              LevenshteinWeightTable weights = {1, 1, 1},
                              percent score_cutoff = 0.0)
{
    auto sentence1 = common::to_string_view(s1);
    auto sentence2 = common::to_string_view(s2);

    if (weights.insert_cost == weights.delete_cost) {
        /* uniform Levenshtein */
        if (weights.insert_cost == weights.replace_cost) {
            return detail::normalized_levenshtein(sentence1, sentence2, score_cutoff);
        }
        /*
         * when replace_cost >= insert_cost + delete_cost no substitutions are performed
         * therefore this can be implemented as InDel distance
         */
        else if (weights.replace_cost >= weights.insert_cost + weights.delete_cost) {
            return detail::normalized_weighted_levenshtein(sentence1, sentence2, score_cutoff);
        }
    }

    return detail::normalized_generic_levenshtein(sentence1, sentence2, weights, score_cutoff);
}

template <typename Sentence1>
struct CachedNormalizedLevenshtein {
    using CharT1 = char_type<Sentence1>;

    CachedNormalizedLevenshtein(const Sentence1& s1, LevenshteinWeightTable aWeights = {1, 1, 1})
        : s1_view(common::to_string_view(s1)), blockmap_s1(s1_view), weights(aWeights)
    {}

    template <typename Sentence2>
    double ratio(const Sentence2& s2, percent score_cutoff = 0) const
    {
        auto s2_view = common::to_string_view(s2);

        if (weights.insert_cost == weights.delete_cost) {
            /* uniform Levenshtein */
            if (weights.insert_cost == weights.replace_cost) {
                return detail::normalized_levenshtein(s2_view, blockmap_s1, s1_view, score_cutoff);
            }
            /*
             * when replace_cost >= insert_cost + delete_cost no substitutions are performed
             * therefore this can be implemented as InDel distance
             */
            else if (weights.replace_cost >= weights.insert_cost + weights.delete_cost) {
                return detail::normalized_weighted_levenshtein(s2_view, blockmap_s1, s1_view,
                                                               score_cutoff);
            }
        }

        return detail::normalized_generic_levenshtein(s1_view, s2_view, weights, score_cutoff);
    }

private:
    rapidfuzz::basic_string_view<CharT1> s1_view;
    common::BlockPatternMatchVector blockmap_s1;
    LevenshteinWeightTable weights;
};

/**
 * @brief Return list of LevenshteinEditOp describing how to turn s1 into s2.
 *
 * @tparam Sentence1 This is a string that can be converted to
 * basic_string_view<char_type>
 * @tparam Sentence2 This is a string that can be converted to
 * basic_string_view<char_type>
 *
 * @param s1
 *   string to compare with s2 (for type info check Template parameters above)
 * @param s2
 *   string to compare with s1 (for type info check Template parameters above)
 *
 * @return Edit operations required to turn s1 into s2
 */
template <typename Sentence1, typename Sentence2>
std::vector<LevenshteinEditOp> levenshtein_editops(const Sentence1& s1, const Sentence2& s2)
{
    auto sentence1 = common::to_string_view(s1);
    auto sentence2 = common::to_string_view(s2);

    return detail::levenshtein_editops(sentence1, sentence2);
}

/**
 * @brief Calculates the Hamming distance between two strings.
 *
 * @details
 * Both strings require a similar length
 *
 *
 * @tparam Sentence1 This is a string that can be converted to
 * basic_string_view<char_type>
 * @tparam Sentence2 This is a string that can be converted to
 * basic_string_view<char_type>
 *
 * @param s1
 *   string to compare with s2 (for type info check Template parameters above)
 * @param s2
 *   string to compare with s1 (for type info check Template parameters above)
 * @param max
 *   Maximum Hamming distance between s1 and s2, that is
 *   considered as a result. If the distance is bigger than max,
 *   -1 is returned instead. Default is std::numeric_limits<std::size_t>::max(),
 *   which deactivates this behaviour.
 *
 * @return Hamming distance between s1 and s2
 */
template <typename Sentence1, typename Sentence2>
std::size_t hamming(const Sentence1& s1, const Sentence2& s2,
                    std::size_t max = std::numeric_limits<std::size_t>::max())
{
    auto sentence1 = common::to_string_view(s1);
    auto sentence2 = common::to_string_view(s2);

    if (sentence1.size() != sentence2.size()) {
        throw std::invalid_argument("s1 and s2 are not the same length.");
    }

    std::size_t hamm = 0;

    for (std::size_t i = 0; i < sentence1.length(); i++) {
        if (common::mixed_sign_unequal(sentence1[i], sentence2[i])) {
            hamm++;
        }
    }

    return hamm > max ? (std::size_t)-1 : hamm;
}

template <typename Sentence1>
struct CachedHamming {
    using CharT1 = char_type<Sentence1>;

    CachedHamming(const Sentence1& s1) : s1_view(common::to_string_view(s1))
    {}

    template <typename Sentence2>
    std::size_t distance(const Sentence2& s2,
                         std::size_t max = std::numeric_limits<std::size_t>::max()) const
    {
        return hamming(s1_view, s2, max);
    }

private:
    rapidfuzz::basic_string_view<CharT1> s1_view;
};

/**
 * @brief Calculates a normalized hamming distance
 *
 * @details
 * Both string require a similar length
 *
 *
 * @tparam Sentence1 This is a string that can be converted to
 * basic_string_view<char_type>
 * @tparam Sentence2 This is a string that can be converted to
 * basic_string_view<char_type>
 *
 * @param s1
 *   string to compare with s2 (for type info check Template parameters above)
 * @param s2
 *   string to compare with s1 (for type info check Template parameters above)
 * @param score_cutoff
 *   Optional argument for a score threshold as a float between 0 and 100.
 *   For ratio < score_cutoff 0 is returned instead. Default is 0,
 *   which deactivates this behaviour.
 *
 * @return Normalized hamming distance between s1 and s2
 *   as a float between 0 and 100
 */
template <typename Sentence1, typename Sentence2>
double normalized_hamming(const Sentence1& s1, const Sentence2& s2, percent score_cutoff = 0.0)
{
    auto sentence1 = common::to_string_view(s1);
    auto sentence2 = common::to_string_view(s2);
    return common::norm_distance(hamming(sentence1, sentence2), sentence1.size(), score_cutoff);
}

template <typename Sentence1>
struct CachedNormalizedHamming {
    using CharT1 = char_type<Sentence1>;

    CachedNormalizedHamming(const Sentence1& s1) : s1_view(common::to_string_view(s1))
    {}

    template <typename Sentence2>
    double ratio(const Sentence2& s2, percent score_cutoff = 0) const
    {
        return normalized_hamming(s1_view, s2, score_cutoff);
    }

private:
    rapidfuzz::basic_string_view<CharT1> s1_view;
};

/**
 * @brief Calculates the jaro winkler similarity
 *
 * @tparam Sentence1 This is a string that can be converted to
 * basic_string_view<char_type>
 * @tparam Sentence2 This is a string that can be converted to
 * basic_string_view<char_type>
 *
 * @param s1
 *   string to compare with s2 (for type info check Template parameters above)
 * @param s2
 *   string to compare with s1 (for type info check Template parameters above)
 * @param prefix_weight
 *   Weight used for the common prefix of the two strings.
 *   Has to be between 0 and 0.25. Default is 0.1.
 * @param score_cutoff
 *   Optional argument for a score threshold as a float between 0 and 100.
 *   For ratio < score_cutoff 0 is returned instead. Default is 0,
 *   which deactivates this behaviour.
 *
 * @return jaro winkler similarity between s1 and s2
 *   as a float between 0 and 100
 */
template <typename Sentence1, typename Sentence2>
double jaro_winkler_similarity(const Sentence1& s1, const Sentence2& s2, double prefix_weight = 0.1,
                               percent score_cutoff = 0.0)
{
    auto sentence1 = common::to_string_view(s1);
    auto sentence2 = common::to_string_view(s2);

    if (prefix_weight < 0.0 || prefix_weight > 0.25) {
        throw std::invalid_argument("prefix_weight has to be between 0.0 - 0.25");
    }

    return detail::jaro_winkler_similarity(sentence1, sentence2, prefix_weight, score_cutoff);
}

template <typename Sentence1>
struct CachedJaroWinklerSimilarity {
    using CharT1 = char_type<Sentence1>;

    CachedJaroWinklerSimilarity(const Sentence1& s1, double prefix_weight_ = 0.1)
        : s1_view(common::to_string_view(s1)), prefix_weight(prefix_weight_)
    {}

    template <typename Sentence2>
    double ratio(const Sentence2& s2, percent score_cutoff = 0) const
    {
        return jaro_winkler_similarity(s1_view, s2, prefix_weight, score_cutoff);
    }

private:
    rapidfuzz::basic_string_view<CharT1> s1_view;
    double prefix_weight;
};

/**
 * @brief Calculates the jaro similarity
 *
 * @tparam Sentence1 This is a string that can be converted to
 * basic_string_view<char_type>
 * @tparam Sentence2 This is a string that can be converted to
 * basic_string_view<char_type>
 *
 * @param s1
 *   string to compare with s2 (for type info check Template parameters above)
 * @param s2
 *   string to compare with s1 (for type info check Template parameters above)
 * @param score_cutoff
 *   Optional argument for a score threshold as a float between 0 and 100.
 *   For ratio < score_cutoff 0 is returned instead. Default is 0,
 *   which deactivates this behaviour.
 *
 * @return jaro similarity between s1 and s2
 *   as a float between 0 and 100
 */
template <typename Sentence1, typename Sentence2>
double jaro_similarity(const Sentence1& s1, const Sentence2& s2, percent score_cutoff = 0.0)
{
    auto sentence1 = common::to_string_view(s1);
    auto sentence2 = common::to_string_view(s2);

    return detail::jaro_similarity(sentence1, sentence2, score_cutoff);
}

template <typename Sentence1>
struct CachedJaroSimilarity {
    using CharT1 = char_type<Sentence1>;

    CachedJaroSimilarity(const Sentence1& s1) : s1_view(common::to_string_view(s1))
    {}

    template <typename Sentence2>
    double ratio(const Sentence2& s2, percent score_cutoff = 0) const
    {
        return jaro_similarity(s1_view, s2, score_cutoff);
    }

private:
    rapidfuzz::basic_string_view<CharT1> s1_view;
};

/**@}*/

} // namespace string_metric
} // namespace rapidfuzz
