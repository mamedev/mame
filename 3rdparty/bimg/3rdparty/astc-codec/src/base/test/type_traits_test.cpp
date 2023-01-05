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

#include "src/base/type_traits.h"

#include <gtest/gtest.h>

#include <array>
#include <functional>
#include <list>
#include <vector>

namespace astc_codec {
namespace base {

TEST(TypeTraits, IsCallable) {
  class C;
  C* c = nullptr;

  auto lambda = [c](bool) -> C* { return nullptr; };

  static_assert(is_callable_as<void(), void()>::value, "simple function");
  static_assert(is_callable_as<void (&)(), void()>::value,
                "function reference");
  static_assert(is_callable_as<void (*)(), void()>::value, "function pointer");
  static_assert(is_callable_as<int(C&, C*), int(C&, C*)>::value,
                "function with arguments and return type");
  static_assert(is_callable_as<decltype(lambda), C*(bool)>::value, "lambda");
  static_assert(is_callable_as<std::function<bool(int)>, bool(int)>::value,
                "std::function");

  static_assert(!is_callable_as<int, void()>::value,
                "int should not be callable");
  static_assert(!is_callable_as<C, void()>::value, "incomplete type");
  static_assert(!is_callable_as<void(), void(int)>::value,
                "different arguments");
  static_assert(!is_callable_as<int(), void()>::value,
                "different return types");
  static_assert(!is_callable_as<int(), short()>::value,
                "slightly different return types");
  static_assert(!is_callable_as<int(int), int(int, int)>::value,
                "more arguments");
  static_assert(!is_callable_as<int(int, int), int(int)>::value,
                "less arguments");

  static_assert(!is_callable_as<int(int), int>::value,
                "bad required signature");

  static_assert(is_callable_with_args<void(), void()>::value,
                "simple function");
  static_assert(is_callable_with_args<void (&)(), void()>::value,
                "function reference");
  static_assert(is_callable_with_args<void (*)(), void()>::value,
                "function pointer");
  static_assert(is_callable_with_args<int(C&, C*), int(C&, C*)>::value,
                "function with arguments and return type");
  static_assert(is_callable_with_args<decltype(lambda), C*(bool)>::value,
                "lambda");
  static_assert(
      is_callable_with_args<std::function<bool(int)>, bool(int)>::value,
      "std::function");

  static_assert(!is_callable_with_args<int, void()>::value,
                "int should not be callable");
  static_assert(!is_callable_with_args<C, void()>::value, "incomplete type");
  static_assert(!is_callable_with_args<void(), void(int)>::value,
                "different arguments");
  static_assert(is_callable_with_args<int(), void()>::value,
                "different return types are ignored");
  static_assert(is_callable_with_args<int(), short()>::value,
                "slightly different return types are ignored");
  static_assert(!is_callable_with_args<int(int), int(int, int)>::value,
                "more arguments");
  static_assert(!is_callable_with_args<int(int, int), int(int)>::value,
                "less arguments");

  static_assert(!is_callable_with_args<int(int), int>::value,
                "bad required signature");
}

TEST(TypeTraits, IsTemplateInstantiation) {
  static_assert(!is_template_instantiation_of<int, std::vector>::value,
                "int is not an instance of vector");
  static_assert(!is_template_instantiation_of<std::list<std::vector<int>>,
                                              std::vector>::value,
                "list is not an instance of vector");

  static_assert(
      is_template_instantiation_of<std::vector<int>, std::vector>::value,
      "std::vector<int> is an instance of vector");
  static_assert(
      is_template_instantiation_of<std::vector<std::vector<std::vector<int>>>,
                                   std::vector>::value,
      "nested std::vector<> is an instance of vector");
}

#ifndef _MSC_VER
TEST(TypeTraits, IsRange) {
  static_assert(is_range<std::vector<int>>::value,
                "vector<> should be detected as a range");
  static_assert(is_range<const std::list<std::function<void()>>>::value,
                "const list<> should be detected as a range");
  static_assert(is_range<std::array<std::vector<int>, 10>>::value,
                "array<> should be detected as a range");
  char arr[100];
  static_assert(is_range<decltype(arr)>::value,
                "C array should be detected as a range");
  static_assert(is_range<decltype("string")>::value,
                "String literal should be detected as a range");

  static_assert(!is_range<int>::value, "int shouldn't be a range");
  static_assert(!is_range<int*>::value, "int* shouldn't be a range");
  static_assert(!is_range<const int*>::value,
                "even const int* shouldn't be a range");
}
#endif

}  // namespace base
}  // namespace astc_codec
