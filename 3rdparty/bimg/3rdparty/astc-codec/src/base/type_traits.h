// Copyright 2018 Google LLC
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     https://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#ifndef ASTC_CODEC_BASE_TYPE_TRAITS_H_
#define ASTC_CODEC_BASE_TYPE_TRAITS_H_

#include <iterator>
#include <type_traits>

namespace astc_codec {
namespace base {

namespace details {

// a simple helper class for SFINAE below.
template<class X = void>
struct dummy {
  using type = X;
};

}  // namespace details

// add some convenience shortcuts for an overly complex std::enable_if syntax

// Use 'enable_if<Predicate,Type>' instead of
// 'typename std::enable_if<Predicate::value,Type>::type'
template<class Predicate, class Type = void*>
using enable_if = typename std::enable_if<Predicate::value, Type>::type;

// Use 'enable_if_c<BooleanFlag,Type>' instead of
// 'typename std::enable_if<BooleanFlag,Type>::type'
template<bool predicate, class Type = void*>
using enable_if_c = typename std::enable_if<predicate, Type>::type;

// Use 'enable_if_convertible<From,To,Type>' instead of
// 'typename std::enable_if<std::is_convertible<From,To>::value, Type>::type'
template<class From, class To, class Type = void*>
using enable_if_convertible = enable_if<std::is_convertible<From, To>>;

// -----------------------------------------------------------------------------
// A predicate for checking if some object is callable with a specific
// signature. Examples:
//
//     is_callable_as<int, void()>::value == false.
//     is_callable_as<strcmp, void()>::value == false.
//     is_callable_as<strcmp, int(const char*, const char*)>::value == true
//
template<class F, class Signature, class X = void>
struct is_callable_as : std::false_type {};

// This specialization is SFINAE-d out if template arguments can't be combined
// into a call expression F(), or if the result of that call is not |R|
template<class F, class R, class... Args>
struct is_callable_as<F, R(Args...),
                      typename std::enable_if<std::is_same<
                          typename details::dummy<decltype(std::declval<F>()(
                              std::declval<Args>()...))>::type,
                          R>::value>::type> : std::true_type {};

//
// A similar predicate to only check arguments of the function call and ignore
// the specified return type
//
//  is_callable_as<strcmp, int(const char*, const char*)>::value == true
//  is_callable_as<strcmp, void(const char*, const char*)>::value == false
//  is_callable_with_args<strcmp, void(const char*, const char*)>::value == true
//
template<class F, class Signature, class X = void>
struct is_callable_with_args : std::false_type {};

template<class F, class R, class... Args>
struct is_callable_with_args<
    F, R(Args...),
    typename std::enable_if<
        !std::is_same<typename details::dummy<decltype(
                          std::declval<F>()(std::declval<Args>()...))>::type,
                      F>::value>::type> : std::true_type {};

// -----------------------------------------------------------------------------
// Check if a type |T| is any instantiation of a template |U|. Examples:
//
//    is_template_instantiation_of<int, std::vector>::value == false
//    is_template_instantiation_of<
//         std::list<std::vector<int>>, std::vector>::value == false
//    is_template_instantiation_of<std::vector<int>, std::vector>::value == true
//    is_template_instantiation_of<
//         std::vector<std::vector<int>>, std::vector>::value == true
//
template<class T, template<class...> class U>
struct is_template_instantiation_of : std::false_type {};

template<template<class...> class U, class... Args>
struct is_template_instantiation_of<U<Args...>, U> : std::true_type {};
// -----------------------------------------------------------------------------

//
// is_range<T> - check if type |T| is a range-like type.
//
// It makes sure that expressions std::begin(t) and std::end(t) are well-formed
// and those return the same type.
//
// Note: with expression SFINAE from C++14 is_range_helper<> could be renamed to
//   is_range<> with no extra code. C++11 needs an extra level of enable_if<>
//   to make it work when the type isn't a range.
//

namespace details {

template<class T>
using is_range_helper = std::is_same<
    decltype(std::begin(
        std::declval<typename std::add_lvalue_reference<T>::type>())),
    decltype(
        std::end(std::declval<typename std::add_lvalue_reference<T>::type>()))>;

}  // namespace details

template<class T, class = void>
struct is_range : std::false_type {};

template<class T>
struct is_range<
    T, typename std::enable_if<details::is_range_helper<T>::value>::type>
    : std::true_type {};

////////////////////////////////////////////////////////////////////////////////
//
// A class to incapsulate integer sequence 0, 1, ..., <num_args>
//      Seq<int...>
// Useful to pass function parameters in an array/tuple to call it later.
//

template<int...>
struct Seq {};

// A 'maker' class to construct Seq<int...> given only <num_args>
//    value.
//      MakeSeq<N, S...> works this way, e.g.
//
//      MakeSeq<2> inherits MakeSeq<2 - 1, 2 - 1> == MakeSeq<1, 1>
//          MakeSeq<1, 1> : MakeSeq<1 - 1, 1 - 1, 1> == MakeSeq<0, 0, 1>
//          MakeSeq<0, 0, 1> == MakeSeq<0, S...> and defines |type| = Seq<0, 1>

template<int N, int... S>
struct MakeSeq : MakeSeq<N - 1, N - 1, S...> {};

template<int... S>
struct MakeSeq<0, S...> {
  using type = Seq<S...>;
};

//
// MakeSeqT alias to quickly create Seq<...>:
//      MakeSeqT<3> == Seq<0, 1, 2>
template<int... S>
using MakeSeqT = typename MakeSeq<S...>::type;

}  // namespace base
}  // namespace astc_codec

#endif  // ASTC_CODEC_BASE_TYPE_TRAITS_H_
