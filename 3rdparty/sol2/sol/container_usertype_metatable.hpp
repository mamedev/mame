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
		struct has_push_back {
		private:
			typedef std::array<char, 1> one;
			typedef std::array<char, 2> two;

			template <typename C> static one test(decltype(std::declval<C>().push_back(std::declval<std::add_rvalue_reference_t<typename C::value_type>>()))*);
			template <typename C> static two test(...);

		public:
			static const bool value = sizeof(test<T>(0)) == sizeof(char);
		};

		template <typename T>
		struct has_clear {
		private:
			typedef std::array<char, 1> one;
			typedef std::array<char, 2> two;

			template <typename C> static one test(decltype(&C::clear));
			template <typename C> static two test(...);

		public:
			static const bool value = sizeof(test<T>(0)) == sizeof(char);
		};

		template <typename T>
		struct has_insert {
		private:
			typedef std::array<char, 1> one;
			typedef std::array<char, 2> two;

			template <typename C> static one test(decltype(std::declval<C>().insert(std::declval<std::add_rvalue_reference_t<typename C::const_iterator>>(), std::declval<std::add_rvalue_reference_t<typename C::value_type>>()))*);
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
		typedef meta::has_key_value_pair<meta::unqualified_t<Raw>> is_associative;
		typedef meta::unqualified_t<Raw> T;
		typedef typename T::iterator I;
		typedef std::conditional_t<is_associative::value, typename T::value_type, std::pair<std::size_t, typename T::value_type>> KV;
		typedef typename KV::first_type K;
		typedef typename KV::second_type V;
		typedef std::remove_reference_t<decltype(*std::declval<I&>())> IR;

		struct iter {
			T& source;
			I it;

			iter(T& source, I it) : source(source), it(std::move(it)) {}
		};

		static auto& get_src(lua_State* L) {
#ifdef SOL_SAFE_USERTYPE
			auto p = stack::check_get<T*>(L, 1);
			if (!p || p.value() == nullptr) {
				luaL_error(L, "sol: 'self' argument is not the proper type (pass 'self' as first argument with ':' or call on proper type)");
			}
			return *p.value();
#else
			return stack::get<T>(L, 1);
#endif
		}

		static int real_index_call_associative(std::true_type, lua_State* L) {
			auto k = stack::check_get<K>(L, 2);
			if (k) {
				auto& src = get_src(L);
				using std::end;
				auto it = detail::find(src, *k);
				if (it != end(src)) {
					auto& v = *it;
					return stack::push_reference(L, v.second);
				}
			}
			else {
				auto maybename = stack::check_get<string_detail::string_shim>(L, 2);
				if (maybename) {
					auto& name = *maybename;
					if (name == "add") {
						return stack::push(L, &add_call);
					}
					else if (name == "insert") {
						return stack::push(L, &insert_call);
					}
					else if (name == "clear") {
						return stack::push(L, &clear_call);
					}
				}
			}
			return stack::push(L, lua_nil);
		}

		static int real_index_call_associative(std::false_type, lua_State* L) {
			auto& src = get_src(L);
			auto maybek = stack::check_get<K>(L, 2);
			if (maybek) {
				using std::begin;
				auto it = begin(src);
				K k = *maybek;
#ifdef SOL_SAFE_USERTYPE
				if (k > src.size() || k < 1) {
					return stack::push(L, lua_nil);
				}
#else
#endif // Safety
				--k;
				std::advance(it, k);
				return stack::push_reference(L, *it);
			}
			else {
				auto maybename = stack::check_get<string_detail::string_shim>(L, 2);
				if (maybename) {
					auto& name = *maybename;
					if (name == "add") {
						return stack::push(L, &add_call);
					}
					else if (name == "insert") {
						return stack::push(L, &insert_call);
					}
					else if (name == "clear") {
						return stack::push(L, &clear_call);
					}
				}
			}

			return stack::push(L, lua_nil);
		}

		static int real_index_call(lua_State* L) {
			return real_index_call_associative(is_associative(), L);
		}

		static int real_new_index_call_const(std::false_type, std::false_type, lua_State* L) {
			return luaL_error(L, "sol: cannot write to a const value type or an immutable iterator (e.g., std::set)");
		}

		static int real_new_index_call_const(std::false_type, std::true_type, lua_State* L) {
			return luaL_error(L, "sol: cannot write to a const value type or an immutable iterator (e.g., std::set)");
		}

		static int real_new_index_call_const(std::true_type, std::true_type, lua_State* L) {
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

		static int real_new_index_call_const(std::true_type, std::false_type, lua_State* L) {
			auto& src = get_src(L);
#ifdef SOL_SAFE_USERTYPE
			auto maybek = stack::check_get<K>(L, 2);
			if (!maybek) {
				return stack::push(L, lua_nil);
			}
			K k = *maybek;
#else
			K k = stack::get<K>(L, 2);
#endif
			using std::begin;
			auto it = begin(src);
			if (k == src.size()) {
				real_add_call_push(std::integral_constant<bool, detail::has_push_back<T>::value>(), L, src, 1);
				return 0;
			}
			--k;
			std::advance(it, k);
			*it = stack::get<V>(L, 3);
			return 0;
		}

		static int real_new_index_call(lua_State* L) {
			return real_new_index_call_const(meta::neg<meta::any<std::is_const<V>, std::is_const<IR>>>(), is_associative(), L);
		}

		static int real_pairs_next_call_assoc(std::true_type, lua_State* L) {
			using std::end;
			iter& i = stack::get<user<iter>>(L, 1);
			auto& source = i.source;
			auto& it = i.it;
			if (it == end(source)) {
				return 0;
			}
			int p = stack::multi_push_reference(L, it->first, it->second);
			std::advance(it, 1);
			return p;
		}

		static int real_pairs_call_assoc(std::true_type, lua_State* L) {
			auto& src = get_src(L);
			using std::begin;
			stack::push(L, pairs_next_call);
			stack::push_specific<user<iter>>(L, src, begin(src));
			stack::push(L, 1);
			return 3;
		}

		static int real_pairs_next_call_assoc(std::false_type, lua_State* L) {
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

		static int real_pairs_call_assoc(std::false_type, lua_State* L) {
			auto& src = get_src(L);
			using std::begin;
			stack::push(L, pairs_next_call);
			stack::push_specific<user<iter>>(L, src, begin(src));
			stack::push(L, 0);
			return 3;
		}

		static int real_pairs_next_call(lua_State* L) {
			return real_pairs_next_call_assoc(is_associative(), L);
		}

		static int real_pairs_call(lua_State* L) {
			return real_pairs_call_assoc(is_associative(), L);
		}

		static int real_length_call(lua_State*L) {
			auto& src = get_src(L);
			return stack::push(L, src.size());
		}

		static int real_add_call_insert(std::true_type, lua_State*L, T& src, int boost = 0) {
			using std::end;
			src.insert(end(src), stack::get<V>(L, 2 + boost));
			return 0;
		}

		static int real_add_call_insert(std::false_type, lua_State*L, T&, int = 0) {
			static const std::string& s = detail::demangle<T>();
			return luaL_error(L, "sol: cannot call insert on type %s", s.c_str());
		}

		static int real_add_call_push(std::true_type, lua_State*L, T& src, int boost = 0) {
			src.push_back(stack::get<V>(L, 2 + boost));
			return 0;
		}

		static int real_add_call_push(std::false_type, lua_State*L, T& src, int boost = 0) {
			return real_add_call_insert(std::integral_constant<bool, detail::has_insert<T>::value>(), L, src, boost);
		}

		static int real_add_call_associative(std::true_type, lua_State* L) {
			return real_insert_call(L);
		}

		static int real_add_call_associative(std::false_type, lua_State* L) {
			auto& src = get_src(L);
			return real_add_call_push(std::integral_constant<bool, detail::has_push_back<T>::value>(), L, src);
		}

		static int real_add_call_capable(std::true_type, lua_State* L) {
			return real_add_call_associative(is_associative(), L);
		}

		static int real_add_call_capable(std::false_type, lua_State* L) {
			static const std::string& s = detail::demangle<T>();
			return luaL_error(L, "sol: cannot call add on type %s", s.c_str());
		}

		static int real_add_call(lua_State* L) {
			return real_add_call_capable(std::integral_constant<bool, detail::has_push_back<T>::value || detail::has_insert<T>::value>(), L);
		}

		static int real_insert_call_capable(std::false_type, std::false_type, lua_State*L) {
			static const std::string& s = detail::demangle<T>();
			return luaL_error(L, "sol: cannot call insert on type %s", s.c_str());
		}

		static int real_insert_call_capable(std::false_type, std::true_type, lua_State*L) {
			return real_insert_call_capable(std::false_type(), std::false_type(), L);
		}

		static int real_insert_call_capable(std::true_type, std::false_type, lua_State* L) {
			using std::begin;
			auto& src = get_src(L);
			src.insert(std::next(begin(src), stack::get<K>(L, 2)), stack::get<V>(L, 3));
			return 0;
		}

		static int real_insert_call_capable(std::true_type, std::true_type, lua_State* L) {
			return real_new_index_call(L);
		}

		static int real_insert_call(lua_State*L) {
			return real_insert_call_capable(std::integral_constant<bool, detail::has_insert<T>::value>(), is_associative(), L);
		}

		static int real_clear_call_capable(std::false_type, lua_State* L) {
			static const std::string& s = detail::demangle<T>();
			return luaL_error(L, "sol: cannot call clear on type %s", s.c_str());
		}

		static int real_clear_call_capable(std::true_type, lua_State* L) {
			auto& src = get_src(L);
			src.clear();
			return 0;
		}

		static int real_clear_call(lua_State*L) {
			return real_clear_call_capable(std::integral_constant<bool, detail::has_clear<T>::value>(), L);
		}

		static int add_call(lua_State*L) {
			return detail::static_trampoline<(&real_add_call)>(L);
		}

		static int insert_call(lua_State*L) {
			return detail::static_trampoline<(&real_insert_call)>(L);
		}

		static int clear_call(lua_State*L) {
			return detail::static_trampoline<(&real_clear_call)>(L);
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
		namespace stack_detail {
			template <typename T>
			inline auto container_metatable() {
				typedef container_usertype_metatable<std::remove_pointer_t<T>> meta_cumt;
				std::array<luaL_Reg, 10> reg = { {
					{ "__index", &meta_cumt::index_call },
					{ "__newindex", &meta_cumt::new_index_call },
					{ "__pairs", &meta_cumt::pairs_call },
					{ "__ipairs", &meta_cumt::pairs_call },
					{ "__len", &meta_cumt::length_call },
					{ "clear", &meta_cumt::clear_call },
					{ "insert", &meta_cumt::insert_call },
					{ "add", &meta_cumt::add_call },
					std::is_pointer<T>::value ? luaL_Reg{ nullptr, nullptr } : luaL_Reg{ "__gc", &detail::usertype_alloc_destroy<T> },
					{ nullptr, nullptr }
				} };
				return reg;
			}

			template <typename T>
			inline auto container_metatable_behind() {
				typedef container_usertype_metatable<std::remove_pointer_t<T>> meta_cumt;
				std::array<luaL_Reg, 3> reg = { {
					{ "__index", &meta_cumt::index_call },
					{ "__newindex", &meta_cumt::new_index_call },
					{ nullptr, nullptr }
				} };
				return reg;
			}

			template <typename T>
			struct metatable_setup {
				lua_State* L;

				metatable_setup(lua_State* L) : L(L) {}

				void operator()() {
					static const auto reg = container_metatable<T>();
					static const auto containerreg = container_metatable_behind<T>();
					static const char* metakey = &usertype_traits<T>::metatable()[0];
					
					if (luaL_newmetatable(L, metakey) == 1) {
						stack_reference metatable(L, -1);
						luaL_setfuncs(L, reg.data(), 0);

						lua_createtable(L, 0, static_cast<int>(containerreg.size()));
						stack_reference metabehind(L, -1);
						luaL_setfuncs(L, containerreg.data(), 0);
						
						stack::set_field(L, metatable_key, metabehind, metatable.stack_index());
						metabehind.pop();
					}
					lua_setmetatable(L, -2);
				}
			};
		}
		
		template<typename T>
		struct pusher<T, std::enable_if_t<meta::all<is_container<meta::unqualified_t<T>>, meta::neg<meta::any<std::is_base_of<reference, meta::unqualified_t<T>>, std::is_base_of<stack_reference, meta::unqualified_t<T>>>>>::value>> {
			static int push(lua_State* L, const T& cont) {
				stack_detail::metatable_setup<T> fx(L);
				return pusher<detail::as_value_tag<T>>{}.push_fx(L, fx, cont);
			}

			static int push(lua_State* L, T&& cont) {
				stack_detail::metatable_setup<T> fx(L);
				return pusher<detail::as_value_tag<T>>{}.push_fx(L, fx, std::move(cont));
			}
		};

		template<typename T>
		struct pusher<T*, std::enable_if_t<meta::all<is_container<meta::unqualified_t<T>>, meta::neg<meta::any<std::is_base_of<reference, meta::unqualified_t<T>>, std::is_base_of<stack_reference, meta::unqualified_t<T>>>>>::value>> {
			static int push(lua_State* L, T* cont) {
				stack_detail::metatable_setup<meta::unqualified_t<std::remove_pointer_t<T>>*> fx(L);
				return pusher<detail::as_pointer_tag<T>>{}.push_fx(L, fx, cont);
			}
		};
	} // stack

} // sol

#endif // SOL_CONTAINER_USERTYPE_HPP
