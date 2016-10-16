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

#ifndef SOL_CONTAINER_USERTYPE_HPP
#define SOL_CONTAINER_USERTYPE_HPP

#include "stack.hpp"

namespace sol {
	
	namespace detail {

		template <typename T>
		struct has_find {
		private:
			typedef std::array<char, 1> one;
			typedef std::array<char, 2> two;

			template <typename C> static one test(decltype(&C::find));
			template <typename C> static two test(...);

		public:
			static const bool value = sizeof(test<T>(0)) == sizeof(char);
		};

		template <typename T>
		T& get_first(const T& t) {
			return std::forward<T>(t);
		}

		template <typename A, typename B>
		decltype(auto) get_first(const std::pair<A, B>& t) {
			return t.first;
		}

		template <typename C, typename I, meta::enable<has_find<meta::unqualified_t<C>>> = meta::enabler>
		auto find(C& c, I&& i) {
			return c.find(std::forward<I>(i));
		}

		template <typename C, typename I, meta::disable<has_find<meta::unqualified_t<C>>> = meta::enabler>
		auto find(C& c, I&& i) {
			using std::begin;
			using std::end;
			return std::find_if(begin(c), end(c), [&i](auto&& x) {
				return i == get_first(x);
			});
		}

	}

	template <typename Raw, typename C = void>
	struct container_usertype_metatable {
		typedef meta::unqualified_t<Raw> T;
		typedef std::size_t K;
		typedef typename T::value_type V;
		typedef typename T::iterator I;
		typedef std::remove_reference_t<decltype(*std::declval<I&>())> IR;

		struct iter {
			T& source;
			I it;

			iter(T& source, I it) : source(source), it(std::move(it)) {}
		};

		static auto& get_src(lua_State* L) {
#ifdef SOL_SAFE_USERTYPE
			auto p = stack::get<T*>(L, 1);
			if (p == nullptr) {
				luaL_error(L, "sol: 'self' argument is nil (pass 'self' as first argument or call on proper type)");
			}
			return *p;
#else
			return stack::get<T>(L, 1);
#endif
		}

		static int real_index_call(lua_State* L) {
			auto& src = get_src(L);
#ifdef SOL_SAFE_USERTYPE
			auto maybek = stack::check_get<K>(L, 2);
			if (maybek) {
				using std::begin;
				auto it = begin(src);
				K k = *maybek;
				if (k <= src.size() && k > 0) {
					--k;
					std::advance(it, k);
					return stack::push_reference(L, *it);
				}
			}
			return stack::push(L, nil);
#else
			using std::begin;
			auto it = begin(src);
			K k = stack::get<K>(L, 2);
			--k;
			std::advance(it, k);
			return stack::push_reference(L, *it);
#endif // Safety
		}

		template <bool b, meta::disable<meta::boolean<b>> = meta::enabler>
		static int real_new_index_call_const(std::integral_constant<bool, b>, lua_State* L) {
			luaL_error(L, "sol: cannot write to a const value type or an immutable iterator (e.g., std::set)");
			return 0;
		}

		template <bool b, meta::enable<meta::boolean<b>> = meta::enabler>
		static int real_new_index_call_const(std::integral_constant<bool, b>, lua_State* L) {
			auto& src = get_src(L);
#ifdef SOL_SAFE_USERTYPE
			auto maybek = stack::check_get<K>(L, 2);
			if (maybek) {
				K k = *maybek;
				if (k <= src.size() && k > 0) {
					--k;
					using std::begin;
					auto it = begin(src);
					std::advance(it, k);
					*it = stack::get<V>(L, 3);
				}
			}
#else
			using std::begin;
			auto it = begin(src);
			K k = stack::get<K>(L, 2);
			--k;
			std::advance(it, k);
			*it = stack::get<V>(L, 3);
#endif
			return 0;
		}

		static int real_new_index_call(lua_State* L) {
			return real_new_index_call_const(meta::neg<meta::any<std::is_const<V>, std::is_const<IR>>>(), L);
		}

		static int real_pairs_next_call(lua_State* L) {
			using std::end;
			iter& i = stack::get<user<iter>>(L, 1);
			auto& source = i.source;
			auto& it = i.it;
			K k = stack::get<K>(L, 2);
			if (it == end(source)) {
				return 0;
			}
			int p = stack::multi_push_reference(L, k + 1, *it);
			std::advance(it, 1);
			return p;
		}

		static int real_pairs_call(lua_State* L) {
			auto& src = get_src(L);
			using std::begin;
			stack::push(L, pairs_next_call);
			stack::push<user<iter>>(L, src, begin(src));
			stack::push(L, 0);
			return 3;
		}

		static int real_length_call(lua_State*L) {
			auto& src = get_src(L);
			return stack::push(L, src.size());
		}

#if 0
		static int real_push_back_call(lua_State*L) {
			auto& src = get_src(L);
			src.push_back(stack::get<V>(L, 2));
			return 0;
		}

		static int real_insert_call(lua_State*L) {
			using std::begin;
			auto& src = get_src(L);
			src.insert(std::next(begin(src), stack::get<K>(L, 2)), stack::get<V>(L, 3));
			return 0;
		}

		static int push_back_call(lua_State*L) {
			return detail::static_trampoline<(&real_length_call)>(L);
		}

		static int insert_call(lua_State*L) {
			return detail::static_trampoline<(&real_insert_call)>(L);
		}
#endif // Sometime later, in a distant universe...

		static int length_call(lua_State*L) {
			return detail::static_trampoline<(&real_length_call)>(L);
		}

		static int pairs_next_call(lua_State*L) {
			return detail::static_trampoline<(&real_pairs_next_call)>(L);
		}

		static int pairs_call(lua_State*L) {
			return detail::static_trampoline<(&real_pairs_call)>(L);
		}

		static int index_call(lua_State*L) {
			return detail::static_trampoline<(&real_index_call)>(L);
		}

		static int new_index_call(lua_State*L) {
			return detail::static_trampoline<(&real_new_index_call)>(L);
		}
	};

	template <typename Raw>
	struct container_usertype_metatable<Raw, std::enable_if_t<meta::has_key_value_pair<meta::unqualified_t<Raw>>::value>> {
		typedef meta::unqualified_t<Raw> T;
		typedef typename T::value_type KV;
		typedef typename KV::first_type K;
		typedef typename KV::second_type V;
		typedef typename T::iterator I;
		struct iter {
			T& source;
			I it;

			iter(T& source, I it) : source(source), it(std::move(it)) {}
		};

		static auto& get_src(lua_State* L) {
#ifdef SOL_SAFE_USERTYPE
			auto p = stack::get<T*>(L, 1);
			if (p == nullptr) {
				luaL_error(L, "sol: 'self' argument is nil (pass 'self' as first argument or call on proper type)");
			}
			return *p;
#else
			return stack::get<T>(L, 1);
#endif
		}

		static int real_index_call(lua_State* L) {
			auto& src = get_src(L);
			auto k = stack::check_get<K>(L, 2);
			if (k) {
				using std::end;
				auto it = detail::find(src, *k);
				if (it != end(src)) {
					auto& v = *it;
					return stack::push_reference(L, v.second);
				}
			}
			return stack::push(L, nil);
		}

		static int real_new_index_call_const(std::false_type, lua_State* L) {
			luaL_error(L, "sol: cannot write to a const value type");
			return 0;
		}

		static int real_new_index_call_const(std::true_type, lua_State* L) {
			auto& src = get_src(L);
			auto k = stack::check_get<K>(L, 2);
			if (k) {
				using std::end;
				auto it = detail::find(src, *k);
				if (it != end(src)) {
					auto& v = *it;
					v.second = stack::get<V>(L, 3);
				}
				else {
					src.insert(it, { std::move(*k), stack::get<V>(L, 3) });
				}
			}
			return 0;
		}

		static int real_new_index_call(lua_State* L) {
			return real_new_index_call_const(meta::neg<std::is_const<V>>(), L);
		}

		static int real_pairs_next_call(lua_State* L) {
			using std::end;
			iter& i = stack::get<user<iter>>(L, 1);
			auto& source = i.source;
			auto& it = i.it;
			std::advance(it, 1);
			if (it == end(source)) {
				return 0;
			}
			return stack::multi_push_reference(L, it->first, it->second);
		}

		static int real_pairs_call(lua_State* L) {
			auto& src = get_src(L);
			using std::begin;
			stack::push(L, pairs_next_call);
			stack::push<user<iter>>(L, src, begin(src));
			stack::push(L, 1);
			return 3;
		}

		static int real_length_call(lua_State*L) {
			auto& src = get_src(L);
			return stack::push(L, src.size());
		}

		static int length_call(lua_State*L) {
			return detail::static_trampoline<(&real_length_call)>(L);
		}

		static int pairs_next_call(lua_State*L) {
			return detail::static_trampoline<(&real_pairs_next_call)>(L);
		}

		static int pairs_call(lua_State*L) {
			return detail::static_trampoline<(&real_pairs_call)>(L);
		}

		static int index_call(lua_State*L) {
			return detail::static_trampoline<(&real_index_call)>(L);
		}

		static int new_index_call(lua_State*L) {
			return detail::static_trampoline<(&real_new_index_call)>(L);
		}
	};

	namespace stack {

		template<typename T>
		struct pusher<T, std::enable_if_t<meta::all<meta::has_begin_end<T>, meta::neg<meta::any<std::is_base_of<reference, T>, std::is_base_of<stack_reference, T>>>>::value>> {
			typedef container_usertype_metatable<T> cumt;
			static int push(lua_State* L, const T& cont) {
				auto fx = [&L]() {
					const char* metakey = &usertype_traits<T>::metatable[0];
					if (luaL_newmetatable(L, metakey) == 1) {
						luaL_Reg reg[] = {
							{ "__index", &cumt::index_call },
							{ "__newindex", &cumt::new_index_call },
							{ "__pairs", &cumt::pairs_call },
							{ "__len", &cumt::length_call },
							{ "__gc", &detail::usertype_alloc_destroy<T> },
							{ nullptr, nullptr }
						};
						luaL_setfuncs(L, reg, 0);
					}
					lua_setmetatable(L, -2);
				};
				return pusher<detail::as_value_tag<T>>{}.push_fx(L, fx, cont);
			}

			static int push(lua_State* L, T&& cont) {
				auto fx = [&L]() {
					const char* metakey = &usertype_traits<T>::metatable[0];
					if (luaL_newmetatable(L, metakey) == 1) {
						luaL_Reg reg[] = {
							{ "__index", &cumt::index_call },
							{ "__newindex", &cumt::new_index_call },
							{ "__pairs", &cumt::pairs_call },
							{ "__len", &cumt::length_call },
							{ "__gc", &detail::usertype_alloc_destroy<T> },
							{ nullptr, nullptr }
						};
						luaL_setfuncs(L, reg, 0);
					}
					lua_setmetatable(L, -2);
				};
				return pusher<detail::as_value_tag<T>>{}.push_fx(L, fx, std::move(cont));
			}
		};

		template<typename T>
		struct pusher<T*, std::enable_if_t<meta::all<meta::has_begin_end<meta::unqualified_t<T>>, meta::neg<meta::any<std::is_base_of<reference, meta::unqualified_t<T>>, std::is_base_of<stack_reference, meta::unqualified_t<T>>>>>::value>> {
			typedef container_usertype_metatable<T> cumt;
			static int push(lua_State* L, T* cont) {
				auto fx = [&L]() {
					const char* metakey = &usertype_traits<meta::unqualified_t<T>*>::metatable[0];
					if (luaL_newmetatable(L, metakey) == 1) {
						luaL_Reg reg[] = {
							{ "__index", &cumt::index_call },
							{ "__newindex", &cumt::new_index_call },
							{ "__pairs", &cumt::pairs_call },
							{ "__len", &cumt::length_call },
							{ nullptr, nullptr }
						};
						luaL_setfuncs(L, reg, 0);
					}
					lua_setmetatable(L, -2);
				};
				return pusher<detail::as_pointer_tag<T>>{}.push_fx(L, fx, cont);
			}
		};
	} // stack

} // sol

#endif // SOL_CONTAINER_USERTYPE_HPP
