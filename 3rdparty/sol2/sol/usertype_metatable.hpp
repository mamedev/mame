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

#ifndef SOL_USERTYPE_METATABLE_HPP
#define SOL_USERTYPE_METATABLE_HPP

#include "wrapper.hpp"
#include "call.hpp"
#include "stack.hpp"
#include "types.hpp"
#include "stack_reference.hpp"
#include "usertype_traits.hpp"
#include "inheritance.hpp"
#include "raii.hpp"
#include "deprecate.hpp"
#include <unordered_map>
#include <cstdio>

namespace sol {
	namespace usertype_detail {
		struct no_comp {
			template <typename A, typename B>
			bool operator()(A&&, B&&) const {
				return false;
			}
		};

		typedef void(*base_walk)(lua_State*, bool&, int&, string_detail::string_shim&);
		typedef int(*member_search)(lua_State*, void*);

		struct find_call_pair {
			member_search first;
			member_search second;

			find_call_pair(member_search first, member_search second) : first(first), second(second) {}
		};

		inline bool is_indexer(string_detail::string_shim s) {
			return s == name_of(meta_function::index) || s == name_of(meta_function::new_index);
		}

		inline bool is_indexer(meta_function mf) {
			return mf == meta_function::index || mf == meta_function::new_index;
		}

		inline bool is_indexer(call_construction) {
			return false;
		}

		inline bool is_indexer(base_classes_tag) {
			return false;
		}

		inline auto make_shim(string_detail::string_shim s) {
			return s;
		}

		inline auto make_shim(call_construction) {
			return string_detail::string_shim(name_of(meta_function::call_function));
		}

		inline auto make_shim(meta_function mf) {
			return string_detail::string_shim(name_of(mf));
		}

		inline auto make_shim(base_classes_tag) {
			return string_detail::string_shim(detail::base_class_cast_key());
		}

		template <typename Arg>
		inline std::string make_string(Arg&& arg) {
			string_detail::string_shim s = make_shim(arg);
			return std::string(s.c_str(), s.size());
		}

		template <typename N>
		inline luaL_Reg make_reg(N&& n, lua_CFunction f) {
			luaL_Reg l{ make_shim(std::forward<N>(n)).c_str(), f };
			return l;
		}

		struct registrar {
			virtual int push_um(lua_State* L) = 0;
			virtual ~registrar() {}
		};

		template <bool is_index>
		inline int indexing_fail(lua_State* L) {
			auto maybeaccessor = stack::get<optional<string_detail::string_shim>>(L, is_index ? -1 : -2);
			string_detail::string_shim accessor = maybeaccessor.value_or(string_detail::string_shim("(unknown)"));
			if (is_index)
				return luaL_error(L, "sol: attempt to index (get) lua_nil value \"%s\" on userdata (bad (misspelled?) key name or does not exist)", accessor.c_str());
			else
				return luaL_error(L, "sol: attempt to index (set) lua_nil value \"%s\" on userdata (bad (misspelled?) key name or does not exist)", accessor.c_str());
		}

		template <bool is_index, typename Base>
		static void walk_single_base(lua_State* L, bool& found, int& ret, string_detail::string_shim&) {
			if (found)
				return;
			const char* metakey = &usertype_traits<Base>::metatable()[0];
			const char* gcmetakey = &usertype_traits<Base>::gc_table()[0];
			const char* basewalkkey = is_index ? detail::base_class_index_propogation_key() : detail::base_class_new_index_propogation_key();

			luaL_getmetatable(L, metakey);
			if (type_of(L, -1) == type::lua_nil) {
				lua_pop(L, 1);
				return;
			}
			
			stack::get_field(L, basewalkkey);
			if (type_of(L, -1) == type::lua_nil) {
				lua_pop(L, 2);
				return;
			}
			lua_CFunction basewalkfunc = stack::pop<lua_CFunction>(L);
			lua_pop(L, 1);

			stack::get_field<true>(L, gcmetakey);
			int value = basewalkfunc(L);
			if (value > -1) {
				found = true;
				ret = value;
			}
		}

		template <bool is_index, typename... Bases>
		static void walk_all_bases(lua_State* L, bool& found, int& ret, string_detail::string_shim& accessor) {
			(void)L;
			(void)found;
			(void)ret;
			(void)accessor;
			(void)detail::swallow{ 0, (walk_single_base<is_index, Bases>(L, found, ret, accessor), 0)... };
		}

		template <typename T, typename Op>
		inline int operator_wrap(lua_State* L) {
			auto maybel = stack::check_get<T>(L, 1);
			if (maybel) {
				auto mayber = stack::check_get<T>(L, 2);
				if (mayber) {
					auto& l = *maybel;
					auto& r = *mayber;
					if (std::is_same<no_comp, Op>::value) {
						return stack::push(L, detail::ptr(l) == detail::ptr(r));
					}
					else {
						Op op;
						return stack::push(L, (detail::ptr(l) == detail::ptr(r)) || op(detail::deref(l), detail::deref(r)));
					}
				}
			}
			return stack::push(L, false);
		}

		template <typename T, typename Op, typename Supports, typename Regs, meta::enable<Supports> = meta::enabler>
		inline void make_reg_op(Regs& l, int& index, const char* name) {
			l[index] = { name, &operator_wrap<T, Op> };
			++index;
		}

		template <typename T, typename Op, typename Supports, typename Regs, meta::disable<Supports> = meta::enabler>
		inline void make_reg_op(Regs&, int&, const char*) {
			// Do nothing if there's no support
		}

		struct add_destructor_tag {};
		struct check_destructor_tag {};
		struct verified_tag {} const verified{};

		template <typename T>
		struct is_non_factory_constructor : std::false_type {};
		
		template <typename... Args>
		struct is_non_factory_constructor<constructors<Args...>> : std::true_type {};

		template <typename... Args>
		struct is_non_factory_constructor<constructor_wrapper<Args...>> : std::true_type {};

		template <>
		struct is_non_factory_constructor<no_construction> : std::true_type {};

		template <typename T>
		struct is_constructor : is_non_factory_constructor<T> {};

		template <typename... Args>
		struct is_constructor<factory_wrapper<Args...>> : std::true_type {};

		template <typename... Args>
		using has_constructor = meta::any<is_constructor<meta::unqualified_t<Args>>...>;

		template <typename T>
		struct is_destructor : std::false_type {};

		template <typename Fx>
		struct is_destructor<destructor_wrapper<Fx>> : std::true_type {};

		template <typename... Args>
		using has_destructor = meta::any<is_destructor<meta::unqualified_t<Args>>...>;

	} // usertype_detail

	template <typename T>
	struct clean_type {
		typedef std::conditional_t<std::is_array<meta::unqualified_t<T>>::value, T&, std::decay_t<T>> type;
	};

	template <typename T>
	using clean_type_t = typename clean_type<T>::type;

	template <typename T, typename IndexSequence, typename... Tn>
	struct usertype_metatable : usertype_detail::registrar {};

	template <typename T, std::size_t... I, typename... Tn>
	struct usertype_metatable<T, std::index_sequence<I...>, Tn...> : usertype_detail::registrar {
		typedef std::make_index_sequence<sizeof...(I) * 2> indices;
		typedef std::index_sequence<I...> half_indices;
		typedef std::array<luaL_Reg, sizeof...(Tn) / 2 + 1 + 3> regs_t;
		typedef std::tuple<Tn...> RawTuple;
		typedef std::tuple<clean_type_t<Tn> ...> Tuple;
		template <std::size_t Idx>
		struct check_binding : is_variable_binding<meta::unqualified_tuple_element_t<Idx, Tuple>> {};
		typedef std::unordered_map<std::string, usertype_detail::find_call_pair> mapping_t;
		Tuple functions;
		mapping_t mapping;
		lua_CFunction indexfunc;
		lua_CFunction newindexfunc;
		lua_CFunction destructfunc;
		lua_CFunction callconstructfunc;
		lua_CFunction indexbase;
		lua_CFunction newindexbase;
		usertype_detail::base_walk indexbaseclasspropogation;
		usertype_detail::base_walk newindexbaseclasspropogation;
		void* baseclasscheck;
		void* baseclasscast;
		bool mustindex;
		bool secondarymeta;
		bool hasequals;
		bool hasless;
		bool haslessequals;

		template <std::size_t Idx, meta::enable<std::is_same<lua_CFunction, meta::unqualified_tuple_element<Idx + 1, RawTuple>>> = meta::enabler>
		inline lua_CFunction make_func() {
			return std::get<Idx + 1>(functions);
		}

		template <std::size_t Idx, meta::disable<std::is_same<lua_CFunction, meta::unqualified_tuple_element<Idx + 1, RawTuple>>> = meta::enabler>
		inline lua_CFunction make_func() {
			return call<Idx + 1>;
		}

		static bool contains_variable() {
			typedef meta::any<check_binding<(I * 2 + 1)>...> has_variables;
			return has_variables::value;
		}

		bool contains_index() const {
			bool idx = false;
			(void)detail::swallow{ 0, ((idx |= usertype_detail::is_indexer(std::get<I * 2>(functions))), 0) ... };
			return idx;
		}

		int finish_regs(regs_t& l, int& index) {
			if (!hasless) {
				const char* name = name_of(meta_function::less_than).c_str();
				usertype_detail::make_reg_op<T, std::less<>, meta::supports_op_less<T>>(l, index, name);
			}
			if (!haslessequals) {
				const char* name = name_of(meta_function::less_than_or_equal_to).c_str();
				usertype_detail::make_reg_op<T, std::less_equal<>, meta::supports_op_less_equal<T>>(l, index, name);
			}
			if (!hasequals) {
				const char* name = name_of(meta_function::equal_to).c_str();
				usertype_detail::make_reg_op<T, std::conditional_t<meta::supports_op_equal<T>::value, std::equal_to<>, usertype_detail::no_comp>, std::true_type>(l, index, name);
			}
			if (destructfunc != nullptr) {
				l[index] = { name_of(meta_function::garbage_collect).c_str(), destructfunc };
				++index;
			}
			return index;
		}

		template <std::size_t Idx, typename F>
		void make_regs(regs_t&, int&, call_construction, F&&) {
			callconstructfunc = call<Idx + 1>;
			secondarymeta = true;
		}

		template <std::size_t, typename... Bases>
		void make_regs(regs_t&, int&, base_classes_tag, bases<Bases...>) {
			if (sizeof...(Bases) < 1) {
				return;
			}
			mustindex = true;
			(void)detail::swallow{ 0, ((detail::has_derived<Bases>::value = true), 0)... };

			static_assert(sizeof(void*) <= sizeof(detail::inheritance_check_function), "The size of this data pointer is too small to fit the inheritance checking function: file a bug report.");
			static_assert(sizeof(void*) <= sizeof(detail::inheritance_cast_function), "The size of this data pointer is too small to fit the inheritance checking function: file a bug report.");
			baseclasscheck = (void*)&detail::inheritance<T, Bases...>::type_check;
			baseclasscast = (void*)&detail::inheritance<T, Bases...>::type_cast;
			indexbaseclasspropogation = usertype_detail::walk_all_bases<true, Bases...>;
			newindexbaseclasspropogation = usertype_detail::walk_all_bases<false, Bases...>;
		}

		template <std::size_t Idx, typename N, typename F, typename = std::enable_if_t<!meta::any_same<meta::unqualified_t<N>, base_classes_tag, call_construction>::value>>
		void make_regs(regs_t& l, int& index, N&& n, F&&) {
			if (is_variable_binding<meta::unqualified_t<F>>::value) {
				return;
			}
			luaL_Reg reg = usertype_detail::make_reg(std::forward<N>(n), make_func<Idx>());
			// Returnable scope
			// That would be a neat keyword for C++
			// returnable { ... };
			if (reg.name == name_of(meta_function::equal_to)) {
				hasequals = true;
			}
			if (reg.name == name_of(meta_function::less_than)) {
				hasless = true;
			}
			if (reg.name == name_of(meta_function::less_than_or_equal_to)) {
				haslessequals = true;
			}
			if (reg.name == name_of(meta_function::garbage_collect)) {
				destructfunc = reg.func;
				return;
			}
			else if (reg.name == name_of(meta_function::index)) {
				indexfunc = reg.func;
				mustindex = true;
				return;
			}
			else if (reg.name == name_of(meta_function::new_index)) {
				newindexfunc = reg.func;
				mustindex = true;
				return;
			}
			l[index] = reg;
			++index;
		}

		template <typename... Args, typename = std::enable_if_t<sizeof...(Args) == sizeof...(Tn)>>
		usertype_metatable(Args&&... args) : functions(std::forward<Args>(args)...),
		mapping(),
		indexfunc(usertype_detail::indexing_fail<true>), newindexfunc(usertype_detail::indexing_fail<false>),
		destructfunc(nullptr), callconstructfunc(nullptr),
		indexbase(&core_indexing_call<true>), newindexbase(&core_indexing_call<false>),
		indexbaseclasspropogation(usertype_detail::walk_all_bases<true>), newindexbaseclasspropogation(usertype_detail::walk_all_bases<false>),
		baseclasscheck(nullptr), baseclasscast(nullptr),
		mustindex(contains_variable() || contains_index()), secondarymeta(contains_variable()),
		hasequals(false), hasless(false), haslessequals(false) {
			std::initializer_list<typename mapping_t::value_type> ilist{ {
				std::pair<std::string, usertype_detail::find_call_pair>(
					usertype_detail::make_string(std::get<I * 2>(functions)),
					usertype_detail::find_call_pair(&usertype_metatable::real_find_call<I * 2, I * 2 + 1, false>,
						&usertype_metatable::real_find_call<I * 2, I * 2 + 1, true>)
					)
			}... };
			mapping.insert(ilist);
		}

		template <std::size_t I0, std::size_t I1, bool is_index>
		static int real_find_call(lua_State* L, void* um) {
			auto& f = *static_cast<usertype_metatable*>(um);
			if (is_variable_binding<decltype(std::get<I1>(f.functions))>::value) {
				return real_call_with<I1, is_index, true>(L, f);
			}
			return stack::push(L, c_closure(call<I1, is_index>, stack::push(L, light<usertype_metatable>(f))));
		}

		template <bool is_index, bool toplevel = false>
		static int core_indexing_call(lua_State* L) {
			usertype_metatable& f = toplevel ? stack::get<light<usertype_metatable>>(L, upvalue_index(1)) : stack::pop<light<usertype_metatable>>(L);
			static const int keyidx = -2 + static_cast<int>(is_index);
			if (toplevel && stack::get<type>(L, keyidx) != type::string) {
				return is_index ? f.indexfunc(L) : f.newindexfunc(L);
			}
			std::string name = stack::get<std::string>(L, keyidx);
			auto memberit = f.mapping.find(name);
			if (memberit != f.mapping.cend()) {
				auto& member = is_index ? memberit->second.second : memberit->second.first;
				return (member)(L, static_cast<void*>(&f));
			}
			string_detail::string_shim accessor = name;
			int ret = 0;
			bool found = false;
			// Otherwise, we need to do propagating calls through the bases
			if (is_index)
				f.indexbaseclasspropogation(L, found, ret, accessor);
			else
				f.newindexbaseclasspropogation(L, found, ret, accessor);
			if (found) {
				return ret;
			}
			return toplevel ? (is_index ? f.indexfunc(L) : f.newindexfunc(L)) : -1;
		}

		static int real_index_call(lua_State* L) {
			return core_indexing_call<true, true>(L);
		}

		static int real_new_index_call(lua_State* L) {
			return core_indexing_call<false, true>(L);
		}

		template <std::size_t Idx, bool is_index = true, bool is_variable = false>
		static int real_call(lua_State* L) {
			usertype_metatable& f = stack::get<light<usertype_metatable>>(L, upvalue_index(1));
			return real_call_with<Idx, is_index, is_variable>(L, f);
		}

		template <std::size_t Idx, bool is_index = true, bool is_variable = false>
		static int real_call_with(lua_State* L, usertype_metatable& um) {
			typedef meta::unqualified_tuple_element_t<Idx - 1, Tuple> K;
			typedef meta::unqualified_tuple_element_t<Idx, Tuple> F;
			static const int boost = 
				!usertype_detail::is_non_factory_constructor<F>::value
				&& std::is_same<K, call_construction>::value ? 
				1 : 0;
			auto& f = std::get<Idx>(um.functions);
			return call_detail::call_wrapped<T, is_index, is_variable, boost>(L, f);
		}

		template <std::size_t Idx, bool is_index = true, bool is_variable = false>
		static int call(lua_State* L) {
			return detail::static_trampoline<(&real_call<Idx, is_index, is_variable>)>(L);
		}

		template <std::size_t Idx, bool is_index = true, bool is_variable = false>
		static int call_with(lua_State* L) {
			return detail::static_trampoline<(&real_call_with<Idx, is_index, is_variable>)>(L);
		}

		static int index_call(lua_State* L) {
			return detail::static_trampoline<(&real_index_call)>(L);
		}

		static int new_index_call(lua_State* L) {
			return detail::static_trampoline<(&real_new_index_call)>(L);
		}

		virtual int push_um(lua_State* L) override {
			return stack::push(L, std::move(*this));
		}

		~usertype_metatable() override {

		}
	};

	namespace stack {

		template <typename T, std::size_t... I, typename... Args>
		struct pusher<usertype_metatable<T, std::index_sequence<I...>, Args...>> {
			typedef usertype_metatable<T, std::index_sequence<I...>, Args...> umt_t;
			typedef typename umt_t::regs_t regs_t;

			static umt_t& make_cleanup(lua_State* L, umt_t&& umx) {
				// ensure some sort of uniqueness
				static int uniqueness = 0;
				std::string uniquegcmetakey = usertype_traits<T>::user_gc_metatable();
				// std::to_string doesn't exist in android still, with NDK, so this bullshit
				// is necessary
				// thanks, Android :v
				int appended = snprintf(nullptr, 0, "%d", uniqueness);
				std::size_t insertionpoint = uniquegcmetakey.length() - 1;
				uniquegcmetakey.append(appended, '\0');
				char* uniquetarget = &uniquegcmetakey[insertionpoint];
				snprintf(uniquetarget, uniquegcmetakey.length(), "%d", uniqueness);
				++uniqueness;

				const char* gcmetakey = &usertype_traits<T>::gc_table()[0];
				// Make sure userdata's memory is properly in lua first,
				// otherwise all the light userdata we make later will become invalid
				stack::push_specific<user<umt_t>>(L, metatable_key, uniquegcmetakey, std::move(umx));
				// Create the top level thing that will act as our deleter later on
				stack_reference umt(L, -1);
				stack::set_field<true>(L, gcmetakey, umt);
				umt.pop();

				stack::get_field<true>(L, gcmetakey);
				return stack::pop<light<umt_t>>(L);
			}

			static int push(lua_State* L, umt_t&& umx) {

				umt_t& um = make_cleanup(L, std::move(umx));
				regs_t value_table{ {} };
				int lastreg = 0;
				(void)detail::swallow{ 0, (um.template make_regs<(I * 2)>(value_table, lastreg, std::get<(I * 2)>(um.functions), std::get<(I * 2 + 1)>(um.functions)), 0)... };
				um.finish_regs(value_table, lastreg);
				value_table[lastreg] = { nullptr, nullptr };
				regs_t ref_table = value_table;
				regs_t unique_table = value_table;
				bool hasdestructor = !value_table.empty() && name_of(meta_function::garbage_collect) == value_table[lastreg - 1].name;
				if (hasdestructor) {
					ref_table[lastreg - 1] = { nullptr, nullptr };
					unique_table[lastreg - 1] = { value_table[lastreg - 1].name, detail::unique_destruct<T> };
				}

				// Now use um
				const bool& mustindex = um.mustindex;
				for (std::size_t i = 0; i < 3; ++i) {
					// Pointer types, AKA "references" from C++
					const char* metakey = nullptr;
					luaL_Reg* metaregs = nullptr;
					switch (i) {
					case 0:
						metakey = &usertype_traits<T*>::metatable()[0];
						metaregs = ref_table.data();
						break;
					case 1:
						metakey = &usertype_traits<detail::unique_usertype<T>>::metatable()[0];
						metaregs = unique_table.data();
						break;
					case 2:
					default:
						metakey = &usertype_traits<T>::metatable()[0];
						metaregs = value_table.data();
						break;
					}
					luaL_newmetatable(L, metakey);
					stack_reference t(L, -1);
					stack::push(L, make_light(um));
					luaL_setfuncs(L, metaregs, 1);

					if (um.baseclasscheck != nullptr) {
						stack::set_field(L, detail::base_class_check_key(), um.baseclasscheck, t.stack_index());
					}
					if (um.baseclasscast != nullptr) {
						stack::set_field(L, detail::base_class_cast_key(), um.baseclasscast, t.stack_index());
					}

					stack::set_field(L, detail::base_class_index_propogation_key(), make_closure(um.indexbase, make_light(um)), t.stack_index());
					stack::set_field(L, detail::base_class_new_index_propogation_key(), make_closure(um.newindexbase, make_light(um)), t.stack_index());

					if (mustindex) {
						// Basic index pushing: specialize
						// index and newindex to give variables and stuff
						stack::set_field(L, meta_function::index, make_closure(umt_t::index_call, make_light(um)), t.stack_index());
						stack::set_field(L, meta_function::new_index, make_closure(umt_t::new_index_call, make_light(um)), t.stack_index());
					}
					else {
						// If there's only functions, we can use the fast index version
						stack::set_field(L, meta_function::index, t, t.stack_index());
					}
					// metatable on the metatable
					// for call constructor purposes and such
					lua_createtable(L, 0, 3);
					stack_reference metabehind(L, -1);
					if (um.callconstructfunc != nullptr) {
						stack::set_field(L, meta_function::call_function, make_closure(um.callconstructfunc, make_light(um)), metabehind.stack_index());
					}
					if (um.secondarymeta) {
						stack::set_field(L, meta_function::index, make_closure(umt_t::index_call, make_light(um)), metabehind.stack_index());
						stack::set_field(L, meta_function::new_index, make_closure(umt_t::new_index_call, make_light(um)), metabehind.stack_index());
					}
					stack::set_field(L, metatable_key, metabehind, t.stack_index());
					metabehind.pop();
					// We want to just leave the table
					// in the registry only, otherwise we return it
					t.pop();
				}

				// Now for the shim-table that actually gets assigned to the name
				luaL_newmetatable(L, &usertype_traits<T>::user_metatable()[0]);
				stack_reference t(L, -1);
				stack::push(L, make_light(um));
				luaL_setfuncs(L, value_table.data(), 1);
				{
					lua_createtable(L, 0, 3);
					stack_reference metabehind(L, -1);
					if (um.callconstructfunc != nullptr) {
						stack::set_field(L, meta_function::call_function, make_closure(um.callconstructfunc, make_light(um)), metabehind.stack_index());
					}
					if (um.secondarymeta) {
						stack::set_field(L, meta_function::index, make_closure(umt_t::index_call, make_light(um)), metabehind.stack_index());
						stack::set_field(L, meta_function::new_index, make_closure(umt_t::new_index_call, make_light(um)), metabehind.stack_index());
					}
					stack::set_field(L, metatable_key, metabehind, t.stack_index());
					metabehind.pop();
				}

				return 1;
			}
		};

	} // stack

} // sol

#endif // SOL_USERTYPE_METATABLE_HPP
