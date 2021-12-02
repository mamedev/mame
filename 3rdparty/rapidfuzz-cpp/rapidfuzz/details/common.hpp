/* SPDX-License-Identifier: MIT */
/* Copyright © 2021 Max Bachmann */

#pragma once
#include <array>
#include <cmath>
#include <cstring>
#include <rapidfuzz/details/SplittedSentenceView.hpp>
#include <rapidfuzz/details/type_traits.hpp>
#include <rapidfuzz/details/types.hpp>
#include <tuple>
#include <unordered_map>
#include <vector>

namespace rapidfuzz {

template <typename CharT1, typename CharT2, typename CharT3>
struct DecomposedSet {
    SplittedSentenceView<CharT1> difference_ab;
    SplittedSentenceView<CharT2> difference_ba;
    SplittedSentenceView<CharT3> intersection;
    DecomposedSet(SplittedSentenceView<CharT1> diff_ab, SplittedSentenceView<CharT2> diff_ba,
                  SplittedSentenceView<CharT3> intersect)
        : difference_ab(std::move(diff_ab)),
          difference_ba(std::move(diff_ba)),
          intersection(std::move(intersect))
    {}
};

namespace common {

/**
 * @defgroup Common Common
 * Common utilities shared among multiple functions
 * @{
 */

template <typename CharT1, typename CharT2>
DecomposedSet<CharT1, CharT2, CharT1> set_decomposition(SplittedSentenceView<CharT1> a,
                                                        SplittedSentenceView<CharT2> b);

constexpr percent result_cutoff(double result, percent score_cutoff);

constexpr percent norm_distance(std::size_t dist, std::size_t lensum, percent score_cutoff = 0);

static inline std::size_t score_cutoff_to_distance(percent score_cutoff, std::size_t lensum);

template <typename T>
constexpr bool is_zero(T a, T tolerance = std::numeric_limits<T>::epsilon());

/**
 * @brief Get a string view to the object passed as parameter
 *
 * @tparam Sentence This is a string that can be explicitly converted to
 * basic_string_view<char_type>
 * @tparam CharT This is the char_type of Sentence
 *
 * @param str string that should be converted to string_view (for type info check Template
 * parameters above)
 *
 * @return basic_string_view<CharT> of str
 */
template <
    typename Sentence, typename CharT = char_type<Sentence>,
    typename = enable_if_t<is_explicitly_convertible<Sentence, basic_string_view<CharT>>::value>>
basic_string_view<CharT> to_string_view(Sentence&& str);

/**
 * @brief Get a string view to the object passed as parameter
 *
 * @tparam Sentence This is a string that can not be explicitly converted to
 * basic_string_view<char_type>, but stores a sequence in a similar way (e.g. boost::string_view or
 * std::vector)
 * @tparam CharT This is the char_type of Sentence
 *
 * @param str container that should be converted to string_view (for type info check Template
 * parameters above)
 *
 * @return basic_string_view<CharT> of str
 */
template <
    typename Sentence, typename CharT = char_type<Sentence>,
    typename = enable_if_t<!is_explicitly_convertible<Sentence, basic_string_view<CharT>>::value &&
                           has_data_and_size<Sentence>::value>>
basic_string_view<CharT> to_string_view(const Sentence& str);

template <
    typename Sentence, typename CharT = char_type<Sentence>,
    typename = enable_if_t<is_explicitly_convertible<Sentence, std::basic_string<CharT>>::value>>
std::basic_string<CharT> to_string(Sentence&& str);

template <
    typename Sentence, typename CharT = char_type<Sentence>,
    typename = enable_if_t<!is_explicitly_convertible<Sentence, std::basic_string<CharT>>::value &&
                           has_data_and_size<Sentence>::value>>
std::basic_string<CharT> to_string(const Sentence& str);

/**
 * @brief Finds the first mismatching pair of elements from two ranges:
 * one defined by [first1, last1) and another defined by [first2,last2).
 * Similar implementation to std::mismatch from C++14
 *
 * @param first1, last1	-	the first range of the elements
 * @param first2, last2	-	the second range of the elements
 *
 * @return std::pair with iterators to the first two non-equal elements.
 */
template <typename InputIterator1, typename InputIterator2>
std::pair<InputIterator1, InputIterator2> mismatch(InputIterator1 first1, InputIterator1 last1,
                                                   InputIterator2 first2, InputIterator2 last2);

template <typename CharT1, typename CharT2>
StringAffix remove_common_affix(basic_string_view<CharT1>& a, basic_string_view<CharT2>& b);

template <typename CharT1, typename CharT2>
std::size_t remove_common_prefix(basic_string_view<CharT1>& a, basic_string_view<CharT2>& b);

template <typename CharT1, typename CharT2>
std::size_t remove_common_suffix(basic_string_view<CharT1>& a, basic_string_view<CharT2>& b);

template <typename Sentence, typename CharT = char_type<Sentence>>
SplittedSentenceView<CharT> sorted_split(Sentence&& sentence);

template <typename T>
constexpr auto to_unsigned(T value) -> typename std::make_unsigned<T>::type
{
    return typename std::make_unsigned<T>::type(value);
}

template <typename T>
constexpr auto to_signed(T value) -> typename std::make_unsigned<T>::type
{
    return typename std::make_signed<T>::type(value);
}

template <typename T, typename U>
bool mixed_sign_equal(const T a, const U b)
{
    // prevent compiler warnings by casting
    static constexpr bool both_signed = std::is_signed<T>::value && std::is_signed<U>::value;
    static constexpr bool both_unsigned = std::is_unsigned<T>::value && std::is_unsigned<U>::value;
    if (both_signed) {
        return to_signed(a) == to_signed(b);
    }
    else if (both_unsigned) {
        return to_unsigned(a) == to_unsigned(b);
    }
    else {
        // They can't be equal if 'a' or 'b' is negative.
        return a >= 0 && b >= 0 && to_unsigned(a) == to_unsigned(b);
    }
}

template <typename T, typename U>
bool mixed_sign_unequal(const T a, const U b)
{
    return !mixed_sign_equal(a, b);
}

/*
 * taken from https://stackoverflow.com/a/17251989/11335032
 */
template <typename T, typename U>
bool CanTypeFitValue(const U value)
{
    const intmax_t botT = intmax_t(std::numeric_limits<T>::min());
    const intmax_t botU = intmax_t(std::numeric_limits<U>::min());
    const uintmax_t topT = uintmax_t(std::numeric_limits<T>::max());
    const uintmax_t topU = uintmax_t(std::numeric_limits<U>::max());
    return !((botT > botU && value < static_cast<U>(botT)) ||
             (topT < topU && value > static_cast<U>(topT)));
}

struct PatternMatchVector {
    struct MapElem {
        uint64_t key   = 0;
        uint64_t value = 0;
    };
    std::array<MapElem, 128> m_map;
    std::array<uint64_t, 256> m_extendedAscii;

    PatternMatchVector()
        : m_map(), m_extendedAscii()
    {}

    template <typename CharT>
    PatternMatchVector(basic_string_view<CharT> s)
        : m_map(), m_extendedAscii()
    {
        insert(s);
    }

    template <typename CharT>
    void insert(basic_string_view<CharT> s)
    {
        uint64_t mask = 1;
        for (std::size_t i = 0; i < s.size(); ++i) {
            insert_mask(s[i], mask);
            mask <<= 1;
        }
    }

    template <typename CharT>
    void insert(CharT key, std::size_t pos)
    {
        insert_mask(key, 1ull << pos);
    }

    template <typename CharT>
    uint64_t get(CharT key) const
    {
        if (key >= 0 && key <= 255) {
            return m_extendedAscii[key];
        } else {
            return m_map[lookup((uint64_t)key)].value;
        }
    }

private:
    template <typename CharT>
    void insert_mask(CharT key, uint64_t mask)
    {
        if (key >= 0 && key <= 255) {
            m_extendedAscii[key] |= mask;
        } else {
            size_t i = lookup((uint64_t)key);
            m_map[i].key = key;
            m_map[i].value |= mask;
        }
    }

    /**
     * lookup key inside the hasmap using a similar collision resolution
     * strategy to CPython and Ruby
     */
    std::size_t lookup(uint64_t key) const
    {
        std::size_t i = key % 128;

        if (!m_map[i].value || m_map[i].key == key) {
            return i;
        }

        size_t perturb = key;
        while (true) {
            i = ((i * 5) + perturb + 1) % 128;
            if (!m_map[i].value || m_map[i].key == key) {
                return i;
            }

            perturb >>= 5;
        }
    }
};

struct BlockPatternMatchVector {
    std::vector<PatternMatchVector> m_val;

    BlockPatternMatchVector()
    {}

    template <typename CharT>
    BlockPatternMatchVector(basic_string_view<CharT> s)
    {
        insert(s);
    }

    template <typename CharT>
    void insert(std::size_t block, CharT ch, int pos)
    {
        auto* be = &m_val[block];
        be->insert(ch, pos);
    }

    template <typename CharT>
    void insert(basic_string_view<CharT> s)
    {
        std::size_t block_count = (s.size() / 64) + (std::size_t)((s.size() % 64) != 0);
        m_val.resize(block_count);

        for (std::size_t block = 0; block < block_count; ++block)
        {
            m_val[block].insert(s.substr(block * 64, 64));
        }
    }

    template <typename CharT>
    uint64_t get(std::size_t block, CharT ch) const
    {
        auto* be = &m_val[block];
        return be->get(ch);
    }
};

template <typename CharT1, typename ValueType, std::size_t size = sizeof(CharT1)>
struct CharHashTable;

template <typename CharT1, typename ValueType>
struct CharHashTable<CharT1, ValueType, 1> {
    using UCharT1 = typename std::make_unsigned<CharT1>::type;

    std::array<ValueType, std::numeric_limits<UCharT1>::max() + 1> m_val;
    ValueType m_default;

    CharHashTable() : m_val{}, m_default{}
    {}

    ValueType& create(CharT1 ch)
    {
        return m_val[UCharT1(ch)];
    }

    template <typename CharT2>
    const ValueType& operator[](CharT2 ch) const
    {
        if (!CanTypeFitValue<CharT1>(ch)) {
            return m_default;
        }

        return m_val[UCharT1(ch)];
    }
};

template <typename CharT1, typename ValueType, std::size_t size>
struct CharHashTable {
    std::unordered_map<CharT1, ValueType> m_val;
    ValueType m_default;

    CharHashTable() : m_val{}, m_default{}
    {}

    ValueType& create(CharT1 ch)
    {
        return m_val[ch];
    }

    template <typename CharT2>
    const ValueType& operator[](CharT2 ch) const
    {
        if (!CanTypeFitValue<CharT1>(ch)) {
            return m_default;
        }

        auto search = m_val.find(CharT1(ch));
        if (search == m_val.end()) {
            return m_default;
        }

        return search->second;
    }
};

/**@}*/

} // namespace common
} // namespace rapidfuzz

#include <rapidfuzz/details/common_impl.hpp>
