// The MIT License (MIT) 

// Copyright (c) 2013-2016 Rapptz, ThePhD and contributors

// Permission is hereby granted, free of charge, to any person obtaining a copy of
// this software and associated documentation files (the "Software"), to deal in
// the Software without restriction, including without limitation the rights to
// use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of
// the Software, and to permit persons to whom the Software is furnished to do so,
// subject to the following conditions:

// The above copyright notice and this permission notice shall be included in all
// copies or substantial portions of the Software.

// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS
// FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR
// COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER
// IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
// CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

#ifndef SOL_STACK_PROBE_HPP
#define SOL_STACK_PROBE_HPP

#include "stack_core.hpp"
#include "stack_field.hpp"
#include "stack_check.hpp"

namespace sol {
	namespace stack {
		template <typename T, bool b, bool raw, typename>
		struct probe_field_getter {
			template <typename Key>
			probe get(lua_State* L, Key&& key, int tableindex = -2) {
				if (!b && !maybe_indexable(L, tableindex)) {
					return probe(false, 0);
				}
				get_field<b, raw>(L, std::forward<Key>(key), tableindex);
				return probe(!check<nil_t>(L), 1);
			}
		};

		template <typename A, typename B, bool b, bool raw, typename C>
		struct probe_field_getter<std::pair<A, B>, b, raw, C> {
			template <typename Keys>
			probe get(lua_State* L, Keys&& keys, int tableindex = -2) {
				if (!b && !maybe_indexable(L, tableindex)) {
					return probe(false, 0);
				}
				get_field<b, raw>(L, std::get<0>(keys), tableindex);
				if (!maybe_indexable(L)) {
					return probe(false, 1);
				}
				get_field<false, raw>(L, std::get<1>(keys), tableindex);
				return probe(!check<nil_t>(L), 2);
			}
		};

		template <typename... Args, bool b, bool raw, typename C>
		struct probe_field_getter<std::tuple<Args...>, b, raw, C> {
			template <std::size_t I, typename Keys>
			probe apply(std::index_sequence<I>, int sofar, lua_State* L, Keys&& keys, int tableindex) {
				get_field < I < 1 && b, raw>(L, std::get<I>(keys), tableindex);
				return probe(!check<nil_t>(L), sofar);
			}

			template <std::size_t I, std::size_t I1, std::size_t... In, typename Keys>
			probe apply(std::index_sequence<I, I1, In...>, int sofar, lua_State* L, Keys&& keys, int tableindex) {
				get_field < I < 1 && b, raw>(L, std::get<I>(keys), tableindex);
				if (!maybe_indexable(L)) {
					return probe(false, sofar);
				}
				return apply(std::index_sequence<I1, In...>(), sofar + 1, L, std::forward<Keys>(keys), -1);
			}

			template <typename Keys>
			probe get(lua_State* L, Keys&& keys, int tableindex = -2) {
				if (!b && !maybe_indexable(L, tableindex)) {
					return probe(false, 0);
				}
				return apply(std::index_sequence_for<Args...>(), 1, L, std::forward<Keys>(keys), tableindex);
			}
		};
	} // stack
} // sol

#endif // SOL_STACK_PROBE_HPP