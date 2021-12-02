/* SPDX-License-Identifier: MIT */
/* Copyright © 2020 Max Bachmann */

#pragma once
#include <rapidfuzz/details/types.hpp>

#include <functional>
#include <iterator>
#include <type_traits>
#include <utility>

namespace rapidfuzz {

template <bool B, class T = void>
using enable_if_t = typename std::enable_if<B, T>::type;

namespace detail {
template <typename T>
auto inner_type(T const*) -> T;

template <typename T>
auto inner_type(T const&) -> typename T::value_type;
} // namespace detail

template <typename T>
using char_type = decltype(detail::inner_type(std::declval<T const&>()));

template <typename... Conds>
struct satisfies_all : std::true_type {};

template <typename Cond, typename... Conds>
struct satisfies_all<Cond, Conds...>
    : std::conditional<Cond::value, satisfies_all<Conds...>, std::false_type>::type {};

template <typename... Conds>
struct satisfies_any : std::false_type {};

template <typename Cond, typename... Conds>
struct satisfies_any<Cond, Conds...>
    : std::conditional<Cond::value, std::true_type, satisfies_any<Conds...>>::type {};

// taken from
// https://stackoverflow.com/questions/16893992/check-if-type-can-be-explicitly-converted
template <typename From, typename To>
struct is_explicitly_convertible {
    template <typename T>
    static void f(T);

    template <typename F, typename T>
    static constexpr auto test(int /*unused*/)
        -> decltype(f(static_cast<T>(std::declval<F>())), true)
    {
        return true;
    }

    template <typename F, typename T>
    static constexpr auto test(...) -> bool
    {
        return false;
    }

    static bool const value = test<From, To>(0);
};

#define GENERATE_HAS_MEMBER(member)                                                                \
                                                                                                   \
    template <typename T>                                                                          \
    struct has_member_##member {                                                                   \
    private:                                                                                       \
        using yes = std::true_type;                                                                \
        using no = std::false_type;                                                                \
                                                                                                   \
        struct Fallback {                                                                          \
            int member;                                                                            \
        };                                                                                         \
        struct Derived : T, Fallback {};                                                           \
                                                                                                   \
        template <class U>                                                                         \
        static no test(decltype(U::member)*);                                                      \
        template <typename U>                                                                      \
        static yes test(U*);                                                                       \
                                                                                                   \
        template <typename U, typename = enable_if_t<std::is_class<U>::value>>                     \
        static constexpr bool class_test(U*)                                                       \
        {                                                                                          \
            return std::is_same<decltype(test<Derived>(nullptr)), yes>::value;                     \
        }                                                                                          \
                                                                                                   \
        template <typename U, typename = enable_if_t<!std::is_class<U>::value>>                    \
        static constexpr bool class_test(const U&)                                                 \
        {                                                                                          \
            return false;                                                                          \
        }                                                                                          \
                                                                                                   \
    public:                                                                                        \
        static constexpr bool value = class_test(static_cast<T*>(nullptr));                        \
    };

GENERATE_HAS_MEMBER(data) // Creates 'has_member_data'
GENERATE_HAS_MEMBER(size) // Creates 'has_member_size'

template <typename Sentence>
using has_data_and_size = satisfies_all<has_member_data<Sentence>, has_member_size<Sentence>>;

// This trait checks if a given type is a standard collection of hashable types
// SFINAE ftw
template <class T>
class is_hashable_sequence {
    is_hashable_sequence() = delete;
    using hashable = char;
    struct not_hashable {
        char t[2];
    }; // Ensured to work on any platform
    template <typename C>
    static hashable matcher(decltype(&std::hash<typename C::value_type>::operator()));
    template <typename C>
    static not_hashable matcher(...);

public:
    static bool const value = (sizeof(matcher<T>(nullptr)) == sizeof(hashable));
};

template <class T>
class is_standard_iterable {
    is_standard_iterable() = delete;
    using iterable = char;
    struct not_iterable {
        char t[2];
    }; // Ensured to work on any platform
    template <typename C>
    static iterable matcher(typename C::const_iterator*);
    template <typename C>
    static not_iterable matcher(...);

public:
    static bool const value = (sizeof(matcher<T>(nullptr)) == sizeof(iterable));
};

template <typename C>
void* sub_matcher(typename C::value_type const& (C::*)(size_t) const);

// TODO: Not a real SFINAE, because of the ambiguity between
// value_type const& operator[](size_t) const;
// and value_type& operator[](size_t);
// Not really important
template <class T>
class has_bracket_operator {
    has_bracket_operator() = delete;
    using has_op = char;
    struct hasnt_op {
        char t[2];
    }; // Ensured to work on any platform
    template <typename C>
    static has_op matcher(decltype(sub_matcher<T>(&T::at)));
    template <typename C>
    static hasnt_op matcher(...);

public:
    static bool const value = (sizeof(matcher<T>(nullptr)) == sizeof(has_op));
};

} // namespace rapidfuzz
