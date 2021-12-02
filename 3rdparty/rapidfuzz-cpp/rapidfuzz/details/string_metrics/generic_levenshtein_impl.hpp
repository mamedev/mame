/* SPDX-License-Identifier: MIT */
/* Copyright © 2020 Max Bachmann */

#include <algorithm>
#include <array>
#include <limits>
#include <numeric>
#include <rapidfuzz/details/common.hpp>

namespace rapidfuzz {
namespace string_metric {
namespace detail {

template <typename CharT1, typename CharT2>
std::size_t generic_levenshtein_wagner_fischer(basic_string_view<CharT1> s1,
                                               basic_string_view<CharT2> s2,
                                               LevenshteinWeightTable weights, std::size_t max)
{
    std::vector<std::size_t> cache(s1.size() + 1);

    cache[0] = 0;
    for (std::size_t i = 1; i < cache.size(); ++i) {
        cache[i] = cache[i - 1] + weights.delete_cost;
    }

    for (const auto& char2 : s2) {
        auto cache_iter = cache.begin();
        std::size_t temp = *cache_iter;
        *cache_iter += weights.insert_cost;

        for (const auto& char1 : s1) {
            if (common::mixed_sign_unequal(char1, char2)) {
                temp = std::min({*cache_iter + weights.delete_cost,
                                 *(cache_iter + 1) + weights.insert_cost,
                                 temp + weights.replace_cost});
            }
            ++cache_iter;
            std::swap(*cache_iter, temp);
        }
    }

    return (cache.back() <= max) ? cache.back() : (std::size_t)-1;
}

template <typename CharT1, typename CharT2>
std::size_t generic_levenshtein(basic_string_view<CharT1> s1, basic_string_view<CharT2> s2,
                                LevenshteinWeightTable weights, std::size_t max)
{
    // do not swap the strings, since insertion/deletion cost can be different
    if (s1.size() >= s2.size()) {
        // at least length difference deletions required
        if ((s1.size() - s2.size()) * weights.delete_cost > max) {
            return (std::size_t)-1;
        }
    }
    else {
        // at least length difference insertions required
        if ((s2.size() - s1.size()) * weights.insert_cost > max) {
            return (std::size_t)-1;
        }
    }

    // The Levenshtein distance between <prefix><string1><suffix> and <prefix><string2><suffix>
    // is similar to the distance between <string1> and <string2>, so they can be removed in linear
    // time
    common::remove_common_affix(s1, s2);

    return generic_levenshtein_wagner_fischer(s1, s2, weights, max);
}

template <typename CharT1, typename CharT2>
double normalized_generic_levenshtein(basic_string_view<CharT1> s1, basic_string_view<CharT2> s2,
                                      LevenshteinWeightTable weights, const double score_cutoff)
{
    if (s1.empty() || s2.empty()) {
        return 100.0 * static_cast<double>(s1.empty() && s2.empty());
    }

    // calculate the maximum possible edit distance from the weights
    std::size_t max_dist = 0;
    if (s1.size() >= s2.size()) {
        max_dist = std::min(
            // delete all characters from s1 and insert all characters from s2
            s1.size() * weights.delete_cost + s2.size() * weights.insert_cost,
            // replace all characters and delete the remaining characters from s1
            s2.size() * weights.replace_cost + (s1.size() - s2.size()) * weights.delete_cost);
    }
    else {
        max_dist = std::min(
            // delete all characters from s1 and insert all characters from s2
            s1.size() * weights.delete_cost + s2.size() * weights.insert_cost,
            // replace all characters and insert the remaining characters into s1
            s1.size() * weights.replace_cost + (s2.size() - s1.size()) * weights.insert_cost);
    }

    auto cutoff_distance = common::score_cutoff_to_distance(score_cutoff, max_dist);

    std::size_t dist = generic_levenshtein(s1, s2, weights, cutoff_distance);
    return (dist != (std::size_t)-1) ? common::norm_distance(dist, max_dist, score_cutoff) : 0.0;
}

} // namespace detail
} // namespace string_metric
} // namespace rapidfuzz
