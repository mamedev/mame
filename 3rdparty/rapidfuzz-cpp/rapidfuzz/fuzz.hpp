/* SPDX-License-Identifier: MIT */
/* Copyright © 2021 Max Bachmann */
/* Copyright © 2011 Adam Cohen */

#pragma once
#include <rapidfuzz/details/common.hpp>

#include <type_traits>

namespace rapidfuzz {
namespace fuzz {

/**
 * @defgroup Fuzz Fuzz
 * A collection of string matching algorithms from FuzzyWuzzy
 * @{
 */

/**
 * @brief calculates a simple ratio between two strings
 *
 * @details
 * @code{.cpp}
 * // score is 96.55
 * double score = ratio("this is a test", "this is a test!")
 * @endcode
 *
 * @tparam Sentence1 This is a string that can be converted to
 * basic_string_view<char_type>
 * @tparam Sentence2 This is a string that can be converted to
 * basic_string_view<char_type>
 *
 * @param s1 string to compare with s2 (for type info check Template parameters
 * above)
 * @param s2 string to compare with s1 (for type info check Template parameters
 * above)
 * @param score_cutoff Optional argument for a score threshold between 0% and
 * 100%. Matches with a lower score than this number will not be returned.
 * Defaults to 0.
 *
 * @return returns the ratio between s1 and s2 or 0 when ratio < score_cutoff
 */
template <typename Sentence1, typename Sentence2>
percent ratio(const Sentence1& s1, const Sentence2& s2, percent score_cutoff = 0);

// TODO documentation
template <typename Sentence1>
struct CachedRatio {
    using CharT1 = char_type<Sentence1>;

    CachedRatio(const Sentence1& s1) : s1_view(common::to_string_view(s1)), blockmap_s1(s1_view)
    {}

    template <typename Sentence2>
    double ratio(const Sentence2& s2, percent score_cutoff = 0) const;

private:
    rapidfuzz::basic_string_view<CharT1> s1_view;
    common::BlockPatternMatchVector blockmap_s1;
};

/**
 * @brief calculates the fuzz::ratio of the optimal string alignment
 *
 * @details
 * test @cite hyrro_2004 @cite wagner_fischer_1974
 * @code{.cpp}
 * // score is 100
 * double score = partial_ratio("this is a test", "this is a test!")
 * @endcode
 *
 * @tparam Sentence1 This is a string that can be converted to
 * basic_string_view<char_type>
 * @tparam Sentence2 This is a string that can be converted to
 * basic_string_view<char_type>
 *
 * @param s1 string to compare with s2 (for type info check Template parameters
 * above)
 * @param s2 string to compare with s1 (for type info check Template parameters
 * above)
 * @param score_cutoff Optional argument for a score threshold between 0% and
 * 100%. Matches with a lower score than this number will not be returned.
 * Defaults to 0.
 *
 * @return returns the ratio between s1 and s2 or 0 when ratio < score_cutoff
 */
template <typename Sentence1, typename Sentence2, typename CharT1 = char_type<Sentence1>,
          typename CharT2 = char_type<Sentence2>>
percent partial_ratio(const Sentence1& s1, const Sentence2& s2, percent score_cutoff = 0);

// todo add real implementation
template <typename Sentence1>
struct CachedPartialRatio {
    template <typename>
    friend struct CachedWRatio;
    using CharT1 = char_type<Sentence1>;

    CachedPartialRatio(const Sentence1& s1);

    template <typename Sentence2>
    double ratio(const Sentence2& s2, percent score_cutoff = 0) const;

private:
    rapidfuzz::basic_string_view<CharT1> s1_view;
    common::CharHashTable<CharT1, bool> s1_char_map;
    CachedRatio<Sentence1> cached_ratio;
};

/**
 * @brief Sorts the words in the strings and calculates the fuzz::ratio between
 * them
 *
 * @details
 * @code{.cpp}
 * // score is 100
 * double score = token_sort_ratio("fuzzy wuzzy was a bear", "wuzzy fuzzy was a
 * bear")
 * @endcode
 *
 * @tparam Sentence1 This is a string that can be converted to
 * basic_string_view<char_type>
 * @tparam Sentence2 This is a string that can be converted to
 * basic_string_view<char_type>
 *
 * @param s1 string to compare with s2 (for type info check Template parameters
 * above)
 * @param s2 string to compare with s1 (for type info check Template parameters
 * above)
 * @param score_cutoff Optional argument for a score threshold between 0% and
 * 100%. Matches with a lower score than this number will not be returned.
 * Defaults to 0.
 *
 * @return returns the ratio between s1 and s2 or 0 when ratio < score_cutoff
 */
template <typename Sentence1, typename Sentence2, typename CharT1 = char_type<Sentence1>,
          typename CharT2 = char_type<Sentence2>>
percent token_sort_ratio(const Sentence1& s1, const Sentence2& s2, percent score_cutoff = 0);
// todo CachedRatio speed for equal strings vs original implementation
// TODO documentation
template <typename Sentence1>
struct CachedTokenSortRatio {
    using CharT1 = char_type<Sentence1>;

    CachedTokenSortRatio(const Sentence1& s1)
        : s1_sorted(common::sorted_split(s1).join()), cached_ratio(s1_sorted)
    {}

    template <typename Sentence2>
    double ratio(const Sentence2& s2, percent score_cutoff = 0) const;

private:
    std::basic_string<CharT1> s1_sorted;
    CachedRatio<Sentence1> cached_ratio;
};

/**
 * @brief Sorts the words in the strings and calculates the fuzz::partial_ratio
 * between them
 *
 *
 * @tparam Sentence1 This is a string that can be converted to
 * basic_string_view<char_type>
 * @tparam Sentence2 This is a string that can be converted to
 * basic_string_view<char_type>
 *
 * @param s1 string to compare with s2 (for type info check Template parameters
 * above)
 * @param s2 string to compare with s1 (for type info check Template parameters
 * above)
 * @param score_cutoff Optional argument for a score threshold between 0% and
 * 100%. Matches with a lower score than this number will not be returned.
 * Defaults to 0.
 *
 * @return returns the ratio between s1 and s2 or 0 when ratio < score_cutoff
 */
template <typename Sentence1, typename Sentence2, typename CharT1 = char_type<Sentence1>,
          typename CharT2 = char_type<Sentence2>>
percent partial_token_sort_ratio(const Sentence1& s1, const Sentence2& s2,
                                 percent score_cutoff = 0);

// TODO documentation
template <typename Sentence1>
struct CachedPartialTokenSortRatio {
    using CharT1 = char_type<Sentence1>;

    CachedPartialTokenSortRatio(const Sentence1& s1)
        : s1_sorted(common::sorted_split(s1).join()), cached_partial_ratio(s1_sorted)
    {}

    template <typename Sentence2>
    double ratio(const Sentence2& s2, percent score_cutoff = 0) const;

private:
    std::basic_string<CharT1> s1_sorted;
    CachedPartialRatio<Sentence1> cached_partial_ratio;
};

/**
 * @brief Compares the words in the strings based on unique and common words
 * between them using fuzz::ratio
 *
 * @details
 * @code{.cpp}
 * // score1 is 83.87
 * double score1 = token_sort_ratio("fuzzy was a bear", "fuzzy fuzzy was a
 * bear")
 * // score2 is 100
 * double score2 = token_set_ratio("fuzzy was a bear", "fuzzy fuzzy was a bear")
 * @endcode
 *
 * @tparam Sentence1 This is a string that can be converted to
 * basic_string_view<char_type>
 * @tparam Sentence2 This is a string that can be converted to
 * basic_string_view<char_type>
 *
 * @param s1 string to compare with s2 (for type info check Template parameters
 * above)
 * @param s2 string to compare with s1 (for type info check Template parameters
 * above)
 * @param score_cutoff Optional argument for a score threshold between 0% and
 * 100%. Matches with a lower score than this number will not be returned.
 * Defaults to 0.
 *
 * @return returns the ratio between s1 and s2 or 0 when ratio < score_cutoff
 */
template <typename Sentence1, typename Sentence2>
percent token_set_ratio(const Sentence1& s1, const Sentence2& s2, percent score_cutoff = 0);

// TODO documentation
template <typename Sentence1>
struct CachedTokenSetRatio {
    using CharT1 = char_type<Sentence1>;

    CachedTokenSetRatio(const Sentence1& s1);

    template <typename Sentence2>
    double ratio(const Sentence2& s2, percent score_cutoff = 0) const;

private:
    SplittedSentenceView<CharT1> tokens_s1;
};

/**
 * @brief Compares the words in the strings based on unique and common words
 * between them using fuzz::partial_ratio
 *
 * @tparam Sentence1 This is a string that can be converted to
 * basic_string_view<char_type>
 * @tparam Sentence2 This is a string that can be converted to
 * basic_string_view<char_type>
 *
 * @param s1 string to compare with s2 (for type info check Template parameters
 * above)
 * @param s2 string to compare with s1 (for type info check Template parameters
 * above)
 * @param score_cutoff Optional argument for a score threshold between 0% and
 * 100%. Matches with a lower score than this number will not be returned.
 * Defaults to 0.
 *
 * @return returns the ratio between s1 and s2 or 0 when ratio < score_cutoff
 */
template <typename Sentence1, typename Sentence2, typename CharT1 = char_type<Sentence1>,
          typename CharT2 = char_type<Sentence2>>
percent partial_token_set_ratio(const Sentence1& s1, const Sentence2& s2, percent score_cutoff = 0);

// TODO documentation
template <typename Sentence1>
struct CachedPartialTokenSetRatio {
    using CharT1 = char_type<Sentence1>;

    CachedPartialTokenSetRatio(const Sentence1& s1);

    template <typename Sentence2>
    double ratio(const Sentence2& s2, percent score_cutoff = 0) const;

private:
    SplittedSentenceView<CharT1> tokens_s1;
};

/**
 * @brief Helper method that returns the maximum of fuzz::token_set_ratio and
 * fuzz::token_sort_ratio (faster than manually executing the two functions)
 *
 * @tparam Sentence1 This is a string that can be converted to
 * basic_string_view<char_type>
 * @tparam Sentence2 This is a string that can be converted to
 * basic_string_view<char_type>
 *
 * @param s1 string to compare with s2 (for type info check Template parameters
 * above)
 * @param s2 string to compare with s1 (for type info check Template parameters
 * above)
 * @param score_cutoff Optional argument for a score threshold between 0% and
 * 100%. Matches with a lower score than this number will not be returned.
 * Defaults to 0.
 *
 * @return returns the ratio between s1 and s2 or 0 when ratio < score_cutoff
 */
template <typename Sentence1, typename Sentence2>
percent token_ratio(const Sentence1& s1, const Sentence2& s2, percent score_cutoff = 0);

// todo add real implementation
template <typename Sentence1>
struct CachedTokenRatio {
    using CharT1 = char_type<Sentence1>;

    CachedTokenRatio(const Sentence1& s1)
        : s1_tokens(common::sorted_split(s1)),
          s1_sorted(s1_tokens.join()),
          cached_ratio_s1_sorted(s1_sorted)
    {}

    template <typename Sentence2>
    double ratio(const Sentence2& s2, percent score_cutoff = 0) const;

private:
    SplittedSentenceView<CharT1> s1_tokens;
    std::basic_string<CharT1> s1_sorted;
    CachedRatio<Sentence1> cached_ratio_s1_sorted;
};

/**
 * @brief Helper method that returns the maximum of
 * fuzz::partial_token_set_ratio and fuzz::partial_token_sort_ratio (faster than
 * manually executing the two functions)
 *
 * @tparam Sentence1 This is a string that can be converted to
 * basic_string_view<char_type>
 * @tparam Sentence2 This is a string that can be converted to
 * basic_string_view<char_type>
 *
 * @param s1 string to compare with s2 (for type info check Template parameters
 * above)
 * @param s2 string to compare with s1 (for type info check Template parameters
 * above)
 * @param score_cutoff Optional argument for a score threshold between 0% and
 * 100%. Matches with a lower score than this number will not be returned.
 * Defaults to 0.
 *
 * @return returns the ratio between s1 and s2 or 0 when ratio < score_cutoff
 */
template <typename Sentence1, typename Sentence2, typename CharT1 = char_type<Sentence1>,
          typename CharT2 = char_type<Sentence2>>
percent partial_token_ratio(const Sentence1& s1, const Sentence2& s2, percent score_cutoff = 0);

// todo add real implementation
template <typename Sentence1>
struct CachedPartialTokenRatio {
    using CharT1 = char_type<Sentence1>;

    CachedPartialTokenRatio(const Sentence1& s1);

    template <typename Sentence2>
    double ratio(const Sentence2& s2, percent score_cutoff = 0) const;

private:
    SplittedSentenceView<CharT1> tokens_s1;
    std::basic_string<CharT1> s1_sorted;
};

/**
 * @brief Calculates a weighted ratio based on the other ratio algorithms
 *
 * @details
 * @todo add a detailed description
 *
 * @tparam Sentence1 This is a string that can be converted to
 * basic_string_view<char_type>
 * @tparam Sentence2 This is a string that can be converted to
 * basic_string_view<char_type>
 *
 * @param s1 string to compare with s2 (for type info check Template parameters
 * above)
 * @param s2 string to compare with s1 (for type info check Template parameters
 * above)
 * @param score_cutoff Optional argument for a score threshold between 0% and
 * 100%. Matches with a lower score than this number will not be returned.
 * Defaults to 0.
 *
 * @return returns the ratio between s1 and s2 or 0 when ratio < score_cutoff
 */
template <typename Sentence1, typename Sentence2>
percent WRatio(const Sentence1& s1, const Sentence2& s2, percent score_cutoff = 0);

// todo add real implementation
template <typename Sentence1>
struct CachedWRatio {
    using CharT1 = char_type<Sentence1>;

    CachedWRatio(const Sentence1& s1);

    template <typename Sentence2>
    double ratio(const Sentence2& s2, percent score_cutoff = 0) const;

private:
    // todo somehow implement this using other ratios with creating PatternMatchVector
    // multiple times
    CachedPartialRatio<Sentence1> cached_partial_ratio;
    rapidfuzz::basic_string_view<CharT1> s1_view;
    SplittedSentenceView<CharT1> tokens_s1;
    std::basic_string<CharT1> s1_sorted;
    common::BlockPatternMatchVector blockmap_s1_sorted;
};

/**
 * @brief Calculates a quick ratio between two strings using fuzz.ratio
 *
 * @details
 * @todo add a detailed description
 *
 * @tparam Sentence1 This is a string that can be converted to
 * basic_string_view<char_type>
 * @tparam Sentence2 This is a string that can be converted to
 * basic_string_view<char_type>
 *
 * @param s1 string to compare with s2 (for type info check Template parameters
 * above)
 * @param s2 string to compare with s1 (for type info check Template parameters
 * above)
 * @param score_cutoff Optional argument for a score threshold between 0% and
 * 100%. Matches with a lower score than this number will not be returned.
 * Defaults to 0.
 *
 * @return returns the ratio between s1 and s2 or 0 when ratio < score_cutoff
 */
template <typename Sentence1, typename Sentence2>
percent QRatio(const Sentence1& s1, const Sentence2& s2, percent score_cutoff = 0);

template <typename Sentence1>
struct CachedQRatio {
    using CharT1 = char_type<Sentence1>;

    CachedQRatio(const Sentence1& s1) : s1_view(common::to_string_view(s1)), cached_ratio(s1)
    {}

    template <typename Sentence2>
    double ratio(const Sentence2& s2, percent score_cutoff = 0) const;

private:
    rapidfuzz::basic_string_view<CharT1> s1_view;
    CachedRatio<Sentence1> cached_ratio;
};

/**@}*/

} // namespace fuzz
} // namespace rapidfuzz

#include <rapidfuzz/fuzz_impl.hpp>
