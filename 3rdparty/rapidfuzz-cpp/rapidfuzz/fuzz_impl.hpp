/* SPDX-License-Identifier: MIT */
/* Copyright © 2021 Max Bachmann */
/* Copyright © 2011 Adam Cohen */

#include <rapidfuzz/details/matching_blocks.hpp>
#include <rapidfuzz/string_metric.hpp>

#include <algorithm>
#include <cmath>
#include <iterator>
#include <unordered_map>
#include <vector>

namespace rapidfuzz {
namespace fuzz {

/**********************************************
 *                  ratio
 *********************************************/

template <typename Sentence1, typename Sentence2>
percent ratio(const Sentence1& s1, const Sentence2& s2, const percent score_cutoff)
{
    return string_metric::normalized_levenshtein(s1, s2, {1, 1, 2}, score_cutoff);
}

template <typename Sentence1>
template <typename Sentence2>
double CachedRatio<Sentence1>::ratio(const Sentence2& s2, percent score_cutoff) const
{
    auto s2_view = common::to_string_view(s2);

    return string_metric::detail::normalized_weighted_levenshtein(s2_view, blockmap_s1, s1_view,
                                                                  score_cutoff);
}

/**********************************************
 *              partial_ratio
 *********************************************/

namespace detail {

template <typename Sentence1, typename CachedSentence1, typename Sentence2>
percent
partial_ratio_short_needle(const Sentence1& s1, const CachedRatio<CachedSentence1>& cached_ratio,
                           const common::CharHashTable<char_type<Sentence1>, bool>& s1_char_map,
                           const Sentence2& s2, percent score_cutoff)
{
    double max_ratio = 0;
    auto s1_view = common::to_string_view(s1);
    auto s2_view = common::to_string_view(s2);

    for (std::size_t i = 1; i < s1_view.size(); ++i) {
        auto long_substr = s2_view.substr(0, i);

        if (!s1_char_map[long_substr.back()]) {
            continue;
        }

        double ls_ratio = cached_ratio.ratio(long_substr, score_cutoff);
        if (ls_ratio > max_ratio) {
            score_cutoff = max_ratio = ls_ratio;
            if (ls_ratio == 100.0) {
                return 100.0;
            }
        }
    }

    for (std::size_t i = 0; i < s2_view.size() - s1_view.size(); ++i) {
        auto long_substr = s2_view.substr(i, s1_view.size());

        if (!s1_char_map[long_substr.back()]) {
            continue;
        }

        double ls_ratio = cached_ratio.ratio(long_substr, score_cutoff);
        if (ls_ratio > max_ratio) {
            score_cutoff = max_ratio = ls_ratio;
            if (ls_ratio == 100.0) {
                return 100.0;
            }
        }
    }

    for (std::size_t i = s2_view.size() - s1_view.size(); i < s2_view.size(); ++i) {
        auto long_substr = s2_view.substr(i, s1_view.size());

        if (!s1_char_map[long_substr[0]]) {
            continue;
        }

        double ls_ratio = cached_ratio.ratio(long_substr, score_cutoff);
        if (ls_ratio > max_ratio) {
            score_cutoff = max_ratio = ls_ratio;
            if (ls_ratio == 100.0) {
                return 100.0;
            }
        }
    }

    return max_ratio;
}

template <typename Sentence1, typename Sentence2, typename CharT1 = char_type<Sentence1>>
percent partial_ratio_short_needle(const Sentence1& s1, const Sentence2& s2, percent score_cutoff)
{
    auto s1_view = common::to_string_view(s1);
    CachedRatio<decltype(s1_view)> cached_ratio(s1_view);

    common::CharHashTable<CharT1, bool> s1_char_map;
    for (const CharT1& ch : s1_view) {
        s1_char_map.create(ch) = true;
    }

    return partial_ratio_short_needle(s1_view, cached_ratio, s1_char_map, s2, score_cutoff);
}

template <typename Sentence1, typename CachedSentence1, typename Sentence2>
percent partial_ratio_long_needle(const Sentence1& s1,
                                  const CachedRatio<CachedSentence1>& cached_ratio,
                                  const Sentence2& s2, percent score_cutoff)
{
    double max_ratio = 0;
    if (score_cutoff > 100) {
        return 0;
    }

    auto s1_view = common::to_string_view(s1);
    auto s2_view = common::to_string_view(s2);

    if (s1_view.empty() || s2_view.empty()) {
        return static_cast<double>(s1_view.empty() && s2_view.empty()) * 100.0;
    }

    auto blocks = rapidfuzz::detail::get_matching_blocks(s1_view, s2_view);

    // when there is a full match exit early
    for (const auto& block : blocks) {
        if (block.length == s1_view.length()) {
            return 100;
        }
    }

    for (const auto& block : blocks) {
        std::size_t long_start = (block.dpos > block.spos) ? block.dpos - block.spos : 0;
        auto long_substr = s2_view.substr(long_start, s1_view.length());

        double ls_ratio = cached_ratio.ratio(long_substr, score_cutoff);
        if (ls_ratio > max_ratio) {
            score_cutoff = max_ratio = ls_ratio;
        }
    }

    return max_ratio;
}

template <typename Sentence1, typename Sentence2>
percent partial_ratio_long_needle(const Sentence1& s1, const Sentence2& s2, percent score_cutoff)
{
    auto s1_view = common::to_string_view(s1);
    CachedRatio<decltype(s1_view)> cached_ratio(s1_view);

    return partial_ratio_long_needle(s1_view, cached_ratio, s2, score_cutoff);
}

} // namespace detail

template <typename Sentence1, typename Sentence2, typename CharT1, typename CharT2>
percent partial_ratio(const Sentence1& s1, const Sentence2& s2, percent score_cutoff)
{
    if (score_cutoff > 100) {
        return 0;
    }

    auto s1_view = common::to_string_view(s1);
    auto s2_view = common::to_string_view(s2);

    if (s1_view.empty() || s2_view.empty()) {
        return static_cast<double>(s1_view.empty() && s2_view.empty()) * 100.0;
    }

    if (s1_view.length() > s2_view.length()) {
        return partial_ratio(s2_view, s1_view, score_cutoff);
    }

    if (s1_view.length() <= 64) {
        return detail::partial_ratio_short_needle(s1_view, s2_view, score_cutoff);
    }
    else {
        return detail::partial_ratio_long_needle(s1_view, s2_view, score_cutoff);
    }
}

template <typename Sentence1>
CachedPartialRatio<Sentence1>::CachedPartialRatio(const Sentence1& s1)
    : s1_view(common::to_string_view(s1)), cached_ratio(s1)
{
    for (const CharT1& ch : s1_view) {
        s1_char_map.create(ch) = true;
    }
}

template <typename Sentence1>
template <typename Sentence2>
double CachedPartialRatio<Sentence1>::ratio(const Sentence2& s2, percent score_cutoff) const
{
    auto s2_view = common::to_string_view(s2);

    if (s1_view.size() > s2_view.size()) {
        return partial_ratio(s1_view, s2_view, score_cutoff);
    }

    if (s1_view.empty() || s2_view.empty()) {
        return static_cast<double>(s1_view.empty() && s2_view.empty()) * 100.0;
    }

    if (s1_view.length() <= 64) {
        return detail::partial_ratio_short_needle(s1_view, cached_ratio, s1_char_map, s2_view,
                                                  score_cutoff);
    }
    else {
        return detail::partial_ratio_long_needle(s1_view, cached_ratio, s2_view, score_cutoff);
    }
}

/**********************************************
 *             token_sort_ratio
 *********************************************/

template <typename Sentence1, typename Sentence2, typename CharT1, typename CharT2>
percent token_sort_ratio(const Sentence1& s1, const Sentence2& s2, percent score_cutoff)
{
    if (score_cutoff > 100) return 0;

    return ratio(common::sorted_split(s1).join(), common::sorted_split(s2).join(), score_cutoff);
}

template <typename Sentence1>
template <typename Sentence2>
double CachedTokenSortRatio<Sentence1>::ratio(const Sentence2& s2, percent score_cutoff) const
{
    if (score_cutoff > 100) return 0;

    return cached_ratio.ratio(common::sorted_split(s2).join(), score_cutoff);
}

/**********************************************
 *          partial_token_sort_ratio
 *********************************************/

template <typename Sentence1, typename Sentence2, typename CharT1, typename CharT2>
percent partial_token_sort_ratio(const Sentence1& s1, const Sentence2& s2, percent score_cutoff)
{
    if (score_cutoff > 100) return 0;

    return partial_ratio(common::sorted_split(s1).join(), common::sorted_split(s2).join(),
                         score_cutoff);
}

template <typename Sentence1>
template <typename Sentence2>
double CachedPartialTokenSortRatio<Sentence1>::ratio(const Sentence2& s2,
                                                     percent score_cutoff) const
{
    if (score_cutoff > 100) return 0;

    return cached_partial_ratio.ratio(common::sorted_split(s2).join(), score_cutoff);
}

/**********************************************
 *               token_set_ratio
 *********************************************/

namespace detail {
template <typename CharT1, typename CharT2>
percent token_set_ratio(const SplittedSentenceView<CharT1>& tokens_a,
                        const SplittedSentenceView<CharT2>& tokens_b, const percent score_cutoff)
{
    /* in FuzzyWuzzy this returns 0. For sake of compatibility return 0 here as well
     * see https://github.com/maxbachmann/RapidFuzz/issues/110 */
    if (tokens_a.empty() || tokens_a.empty()) {
        return 0;
    }

    auto decomposition = common::set_decomposition(tokens_a, tokens_b);
    auto intersect = decomposition.intersection;
    auto diff_ab = decomposition.difference_ab;
    auto diff_ba = decomposition.difference_ba;

    // one sentence is part of the other one
    if (!intersect.empty() && (diff_ab.empty() || diff_ba.empty())) {
        return 100;
    }

    auto diff_ab_joined = diff_ab.join();
    auto diff_ba_joined = diff_ba.join();

    size_t ab_len = diff_ab_joined.length();
    size_t ba_len = diff_ba_joined.length();
    size_t sect_len = intersect.length();

    // string length sect+ab <-> sect and sect+ba <-> sect
    size_t sect_ab_len = sect_len + !!sect_len + ab_len;
    size_t sect_ba_len = sect_len + !!sect_len + ba_len;

    percent result = 0;
    auto cutoff_distance = common::score_cutoff_to_distance(score_cutoff, ab_len + ba_len);
    size_t dist =
        string_metric::levenshtein(diff_ab_joined, diff_ba_joined, {1, 1, 2}, cutoff_distance);

    if (dist != (std::size_t)-1) {
        result = common::norm_distance(dist, sect_ab_len + sect_ba_len, score_cutoff);
    }

    // exit early since the other ratios are 0
    if (!sect_len) {
        return result;
    }

    // levenshtein distance sect+ab <-> sect and sect+ba <-> sect
    // since only sect is similar in them the distance can be calculated based on
    // the length difference
    std::size_t sect_ab_dist = !!sect_len + ab_len;
    percent sect_ab_ratio =
        common::norm_distance(sect_ab_dist, sect_len + sect_ab_len, score_cutoff);

    std::size_t sect_ba_dist = !!sect_len + ba_len;
    percent sect_ba_ratio =
        common::norm_distance(sect_ba_dist, sect_len + sect_ba_len, score_cutoff);

    return std::max({result, sect_ab_ratio, sect_ba_ratio});
}
} // namespace detail

template <typename Sentence1, typename Sentence2>
percent token_set_ratio(const Sentence1& s1, const Sentence2& s2, const percent score_cutoff)
{
    if (score_cutoff > 100) return 0;

    return detail::token_set_ratio(common::sorted_split(s1), common::sorted_split(s2),
                                   score_cutoff);
}

template <typename Sentence1>
CachedTokenSetRatio<Sentence1>::CachedTokenSetRatio(const Sentence1& s1)
    : tokens_s1(common::sorted_split(s1))
{}

template <typename Sentence1>
template <typename Sentence2>
double CachedTokenSetRatio<Sentence1>::ratio(const Sentence2& s2, percent score_cutoff) const
{
    if (score_cutoff > 100) return 0;

    return detail::token_set_ratio(tokens_s1, common::sorted_split(s2), score_cutoff);
}

/**********************************************
 *          partial_token_set_ratio
 *********************************************/

namespace detail {
template <typename CharT1, typename CharT2>
percent partial_token_set_ratio(const SplittedSentenceView<CharT1>& tokens_a,
                                const SplittedSentenceView<CharT2>& tokens_b,
                                const percent score_cutoff)
{
    /* in FuzzyWuzzy this returns 0. For sake of compatibility return 0 here as well
     * see https://github.com/maxbachmann/RapidFuzz/issues/110 */
    if (tokens_a.empty() || tokens_a.empty()) {
        return 0;
    }

    auto decomposition = common::set_decomposition(tokens_a, tokens_b);

    // exit early when there is a common word in both sequences
    if (!decomposition.intersection.empty()) return 100;

    return partial_ratio(decomposition.difference_ab.join(), decomposition.difference_ba.join(),
                         score_cutoff);
}
} // namespace detail

template <typename Sentence1, typename Sentence2, typename CharT1, typename CharT2>
percent partial_token_set_ratio(const Sentence1& s1, const Sentence2& s2, percent score_cutoff)
{
    if (score_cutoff > 100) return 0;

    return detail::partial_token_set_ratio(common::sorted_split(s1), common::sorted_split(s2),
                                           score_cutoff);
}

template <typename Sentence1>
CachedPartialTokenSetRatio<Sentence1>::CachedPartialTokenSetRatio(const Sentence1& s1)
    : tokens_s1(common::sorted_split(s1))
{}

template <typename Sentence1>
template <typename Sentence2>
double CachedPartialTokenSetRatio<Sentence1>::ratio(const Sentence2& s2, percent score_cutoff) const
{
    if (score_cutoff > 100) return 0;

    return detail::partial_token_set_ratio(tokens_s1, common::sorted_split(s2), score_cutoff);
}

/**********************************************
 *                token_ratio
 *********************************************/

template <typename Sentence1, typename Sentence2>
percent token_ratio(const Sentence1& s1, const Sentence2& s2, percent score_cutoff)
{
    if (score_cutoff > 100) return 0;

    auto tokens_a = common::sorted_split(s1);
    auto tokens_b = common::sorted_split(s2);

    auto decomposition = common::set_decomposition(tokens_a, tokens_b);
    auto intersect = decomposition.intersection;
    auto diff_ab = decomposition.difference_ab;
    auto diff_ba = decomposition.difference_ba;

    if (!intersect.empty() && (diff_ab.empty() || diff_ba.empty())) {
        return 100;
    }

    auto diff_ab_joined = diff_ab.join();
    auto diff_ba_joined = diff_ba.join();

    size_t ab_len = diff_ab_joined.length();
    size_t ba_len = diff_ba_joined.length();
    size_t sect_len = intersect.length();

    percent result = ratio(tokens_a.join(), tokens_b.join(), score_cutoff);

    // string length sect+ab <-> sect and sect+ba <-> sect
    size_t sect_ab_len = sect_len + !!sect_len + ab_len;
    size_t sect_ba_len = sect_len + !!sect_len + ba_len;

    auto cutoff_distance = common::score_cutoff_to_distance(score_cutoff, ab_len + ba_len);
    size_t dist =
        string_metric::levenshtein(diff_ab_joined, diff_ba_joined, {1, 1, 2}, cutoff_distance);
    if (dist != (std::size_t)-1) {
        result =
            std::max(result, common::norm_distance(dist, sect_ab_len + sect_ba_len, score_cutoff));
    }

    // exit early since the other ratios are 0
    if (!sect_len) {
        return result;
    }

    // levenshtein distance sect+ab <-> sect and sect+ba <-> sect
    // since only sect is similar in them the distance can be calculated based on
    // the length difference
    std::size_t sect_ab_dist = !!sect_len + ab_len;
    percent sect_ab_ratio =
        common::norm_distance(sect_ab_dist, sect_len + sect_ab_len, score_cutoff);

    std::size_t sect_ba_dist = !!sect_len + ba_len;
    percent sect_ba_ratio =
        common::norm_distance(sect_ba_dist, sect_len + sect_ba_len, score_cutoff);

    return std::max({result, sect_ab_ratio, sect_ba_ratio});
}

namespace detail {
template <typename CharT1, typename CachedSentence1, typename Sentence2>
percent token_ratio(const SplittedSentenceView<CharT1>& s1_tokens,
                    const CachedRatio<CachedSentence1>& cached_ratio_s1_sorted, const Sentence2& s2,
                    percent score_cutoff)
{
    if (score_cutoff > 100) return 0;

    auto s2_tokens = common::sorted_split(s2);

    auto decomposition = common::set_decomposition(s1_tokens, s2_tokens);
    auto intersect = decomposition.intersection;
    auto diff_ab = decomposition.difference_ab;
    auto diff_ba = decomposition.difference_ba;

    if (!intersect.empty() && (diff_ab.empty() || diff_ba.empty())) {
        return 100;
    }

    auto diff_ab_joined = diff_ab.join();
    auto diff_ba_joined = diff_ba.join();

    size_t ab_len = diff_ab_joined.length();
    size_t ba_len = diff_ba_joined.length();
    size_t sect_len = intersect.length();

    percent result = cached_ratio_s1_sorted.ratio(s2_tokens.join(), score_cutoff);

    // string length sect+ab <-> sect and sect+ba <-> sect
    size_t sect_ab_len = sect_len + !!sect_len + ab_len;
    size_t sect_ba_len = sect_len + !!sect_len + ba_len;

    auto cutoff_distance = common::score_cutoff_to_distance(score_cutoff, ab_len + ba_len);
    size_t dist =
        string_metric::levenshtein(diff_ab_joined, diff_ba_joined, {1, 1, 2}, cutoff_distance);
    if (dist != (std::size_t)-1) {
        result =
            std::max(result, common::norm_distance(dist, sect_ab_len + sect_ba_len, score_cutoff));
    }

    // exit early since the other ratios are 0
    if (!sect_len) {
        return result;
    }

    // levenshtein distance sect+ab <-> sect and sect+ba <-> sect
    // since only sect is similar in them the distance can be calculated based on
    // the length difference
    std::size_t sect_ab_dist = !!sect_len + ab_len;
    percent sect_ab_ratio =
        common::norm_distance(sect_ab_dist, sect_len + sect_ab_len, score_cutoff);

    std::size_t sect_ba_dist = !!sect_len + ba_len;
    percent sect_ba_ratio =
        common::norm_distance(sect_ba_dist, sect_len + sect_ba_len, score_cutoff);

    return std::max({result, sect_ab_ratio, sect_ba_ratio});
}

// todo this is a temporary solution until WRatio is properly implemented using other scorers
template <typename CharT1, typename Sentence2>
percent token_ratio(const std::basic_string<CharT1>& s1_sorted,
                    const SplittedSentenceView<CharT1>& tokens_s1,
                    const common::BlockPatternMatchVector& blockmap_s1_sorted,
                    const Sentence2& s2, percent score_cutoff)
{
    if (score_cutoff > 100) return 0;

    auto tokens_b = common::sorted_split(s2);

    auto decomposition = common::set_decomposition(tokens_s1, tokens_b);
    auto intersect = decomposition.intersection;
    auto diff_ab = decomposition.difference_ab;
    auto diff_ba = decomposition.difference_ba;

    if (!intersect.empty() && (diff_ab.empty() || diff_ba.empty())) {
        return 100;
    }

    auto diff_ab_joined = diff_ab.join();
    auto diff_ba_joined = diff_ba.join();

    size_t ab_len = diff_ab_joined.length();
    size_t ba_len = diff_ba_joined.length();
    size_t sect_len = intersect.length();

    percent result = 0;
    auto s2_sorted = tokens_b.join();
    if (s1_sorted.size() < 65) {
        result = string_metric::detail::normalized_weighted_levenshtein(
            common::to_string_view(s2_sorted), blockmap_s1_sorted,
            common::to_string_view(s1_sorted), score_cutoff);
    }
    else {
        result = fuzz::ratio(s1_sorted, s2_sorted, score_cutoff);
    }

    // string length sect+ab <-> sect and sect+ba <-> sect
    size_t sect_ab_len = sect_len + !!sect_len + ab_len;
    size_t sect_ba_len = sect_len + !!sect_len + ba_len;

    auto cutoff_distance = common::score_cutoff_to_distance(score_cutoff, ab_len + ba_len);
    size_t dist =
        string_metric::levenshtein(diff_ab_joined, diff_ba_joined, {1, 1, 2}, cutoff_distance);
    if (dist != (std::size_t)-1) {
        result =
            std::max(result, common::norm_distance(dist, sect_ab_len + sect_ba_len, score_cutoff));
    }

    // exit early since the other ratios are 0
    if (!sect_len) {
        return result;
    }

    // levenshtein distance sect+ab <-> sect and sect+ba <-> sect
    // since only sect is similar in them the distance can be calculated based on
    // the length difference
    std::size_t sect_ab_dist = !!sect_len + ab_len;
    percent sect_ab_ratio =
        common::norm_distance(sect_ab_dist, sect_len + sect_ab_len, score_cutoff);

    std::size_t sect_ba_dist = !!sect_len + ba_len;
    percent sect_ba_ratio =
        common::norm_distance(sect_ba_dist, sect_len + sect_ba_len, score_cutoff);

    return std::max({result, sect_ab_ratio, sect_ba_ratio});
}
} // namespace detail

template <typename Sentence1>
template <typename Sentence2>
double CachedTokenRatio<Sentence1>::ratio(const Sentence2& s2, percent score_cutoff) const
{
    return detail::token_ratio(s1_tokens, cached_ratio_s1_sorted, s2, score_cutoff);
}

/**********************************************
 *            partial_token_ratio
 *********************************************/

template <typename Sentence1, typename Sentence2, typename CharT1, typename CharT2>
percent partial_token_ratio(const Sentence1& s1, const Sentence2& s2, percent score_cutoff)
{
    if (score_cutoff > 100) return 0;

    auto tokens_a = common::sorted_split(s1);
    auto tokens_b = common::sorted_split(s2);

    auto decomposition = common::set_decomposition(tokens_a, tokens_b);

    // exit early when there is a common word in both sequences
    if (!decomposition.intersection.empty()) return 100;

    auto diff_ab = decomposition.difference_ab;
    auto diff_ba = decomposition.difference_ba;

    percent result = partial_ratio(tokens_a.join(), tokens_b.join(), score_cutoff);

    // do not calculate the same partial_ratio twice
    if (tokens_a.word_count() == diff_ab.word_count() &&
        tokens_b.word_count() == diff_ba.word_count()) {
        return result;
    }

    score_cutoff = std::max(score_cutoff, result);
    return std::max(result, partial_ratio(diff_ab.join(), diff_ba.join(), score_cutoff));
}

namespace detail {
template <typename CharT1, typename Sentence2>
percent partial_token_ratio(const std::basic_string<CharT1>& s1_sorted,
                            const SplittedSentenceView<CharT1>& tokens_s1, const Sentence2& s2,
                            percent score_cutoff)
{
    if (score_cutoff > 100) return 0;

    auto tokens_b = common::sorted_split(s2);

    auto decomposition = common::set_decomposition(tokens_s1, tokens_b);

    // exit early when there is a common word in both sequences
    if (!decomposition.intersection.empty()) return 100;

    auto diff_ab = decomposition.difference_ab;
    auto diff_ba = decomposition.difference_ba;

    percent result = partial_ratio(s1_sorted, tokens_b.join(), score_cutoff);

    // do not calculate the same partial_ratio twice
    if (tokens_s1.word_count() == diff_ab.word_count() &&
        tokens_b.word_count() == diff_ba.word_count()) {
        return result;
    }

    score_cutoff = std::max(score_cutoff, result);
    return std::max(result, partial_ratio(diff_ab.join(), diff_ba.join(), score_cutoff));
}

} // namespace detail

template <typename Sentence1>
CachedPartialTokenRatio<Sentence1>::CachedPartialTokenRatio(const Sentence1& s1)
    : tokens_s1(common::sorted_split(s1))
{
    s1_sorted = tokens_s1.join();
}

template <typename Sentence1>
template <typename Sentence2>
double CachedPartialTokenRatio<Sentence1>::ratio(const Sentence2& s2, percent score_cutoff) const
{
    return detail::partial_token_ratio(s1_sorted, tokens_s1, s2, score_cutoff);
}

/**********************************************
 *                  WRatio
 *********************************************/

template <typename Sentence1, typename Sentence2>
percent WRatio(const Sentence1& s1, const Sentence2& s2, percent score_cutoff)
{
    if (score_cutoff > 100) return 0;

    constexpr double UNBASE_SCALE = 0.95;

    auto s1_view = common::to_string_view(s1);
    auto s2_view = common::to_string_view(s2);

    /* in FuzzyWuzzy this returns 0. For sake of compatibility return 0 here as well
     * see https://github.com/maxbachmann/RapidFuzz/issues/110 */
    if (s1_view.empty() || s2_view.empty()) {
        return 0;
    }

    size_t len_a = s1_view.length();
    size_t len_b = s2_view.length();
    double len_ratio = (len_a > len_b) ? static_cast<double>(len_a) / static_cast<double>(len_b)
                                       : static_cast<double>(len_b) / static_cast<double>(len_a);

    percent end_ratio = ratio(s1, s2, score_cutoff);

    if (len_ratio < 1.5) {
        score_cutoff = std::max(score_cutoff, end_ratio) / UNBASE_SCALE;
        return std::max(end_ratio, token_ratio(s1, s2, score_cutoff) * UNBASE_SCALE);
    }

    const double PARTIAL_SCALE = (len_ratio < 8.0) ? 0.9 : 0.6;

    score_cutoff = std::max(score_cutoff, end_ratio) / PARTIAL_SCALE;
    end_ratio = std::max(end_ratio, partial_ratio(s1, s2, score_cutoff) * PARTIAL_SCALE);

    score_cutoff = std::max(score_cutoff, end_ratio) / UNBASE_SCALE;
    return std::max(end_ratio,
                    partial_token_ratio(s1, s2, score_cutoff) * UNBASE_SCALE * PARTIAL_SCALE);
}

template <typename Sentence1>
CachedWRatio<Sentence1>::CachedWRatio(const Sentence1& s1)
    : cached_partial_ratio(s1), tokens_s1(common::sorted_split(s1))
{
    s1_view = common::to_string_view(s1);
    s1_sorted = tokens_s1.join();

    blockmap_s1_sorted.insert(common::to_string_view(s1_sorted));
}

template <typename Sentence1>
template <typename Sentence2>
double CachedWRatio<Sentence1>::ratio(const Sentence2& s2, percent score_cutoff) const
{
    if (score_cutoff > 100) return 0;

    constexpr double UNBASE_SCALE = 0.95;

    auto s2_view = common::to_string_view(s2);

    /* in FuzzyWuzzy this returns 0. For sake of compatibility return 0 here as well
     * see https://github.com/maxbachmann/RapidFuzz/issues/110 */
    if (s1_view.empty() || s2_view.empty()) {
        return 0;
    }

    size_t len_a = s1_view.length();
    size_t len_b = s2_view.length();
    double len_ratio = (len_a > len_b) ? static_cast<double>(len_a) / static_cast<double>(len_b)
                                       : static_cast<double>(len_b) / static_cast<double>(len_a);

    percent end_ratio = cached_partial_ratio.cached_ratio.ratio(s2_view, score_cutoff);

    if (len_ratio < 1.5) {
        score_cutoff = std::max(score_cutoff, end_ratio) / UNBASE_SCALE;
        // use pre calculated values
        auto r =
            detail::token_ratio(s1_sorted, tokens_s1, blockmap_s1_sorted, s2_view, score_cutoff);
        return std::max(end_ratio, r * UNBASE_SCALE);
    }

    const double PARTIAL_SCALE = (len_ratio < 8.0) ? 0.9 : 0.6;

    score_cutoff = std::max(score_cutoff, end_ratio) / PARTIAL_SCALE;
    end_ratio =
        std::max(end_ratio, cached_partial_ratio.ratio(s2_view, score_cutoff) * PARTIAL_SCALE);

    score_cutoff = std::max(score_cutoff, end_ratio) / UNBASE_SCALE;
    auto r = detail::partial_token_ratio(s1_sorted, tokens_s1, s2_view, score_cutoff);
    return std::max(end_ratio, r * UNBASE_SCALE * PARTIAL_SCALE);
}

/**********************************************
 *                QRatio
 *********************************************/

template <typename Sentence1, typename Sentence2>
percent QRatio(const Sentence1& s1, const Sentence2& s2, percent score_cutoff)
{
    auto s1_view = common::to_string_view(s1);
    auto s2_view = common::to_string_view(s2);

    /* in FuzzyWuzzy this returns 0. For sake of compatibility return 0 here as well
     * see https://github.com/maxbachmann/RapidFuzz/issues/110 */
    if (s1_view.empty() || s2_view.empty()) {
        return 0;
    }

    return ratio(s1_view, s2_view, score_cutoff);
}

template <typename Sentence1>
template <typename Sentence2>
double CachedQRatio<Sentence1>::ratio(const Sentence2& s2, percent score_cutoff) const
{
    auto s2_view = common::to_string_view(s2);

    /* in FuzzyWuzzy this returns 0. For sake of compatibility return 0 here as well
     * see https://github.com/maxbachmann/RapidFuzz/issues/110 */
    if (s1_view.empty() || s2_view.empty()) {
        return 0;
    }

    return cached_ratio.ratio(s2_view, score_cutoff);
}

} // namespace fuzz
} // namespace rapidfuzz
