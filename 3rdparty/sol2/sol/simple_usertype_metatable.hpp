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

#ifndef SOL_SIMPLE_USERTYPE_METATABLE_HPP
#define SOL_SIMPLE_USERTYPE_METATABLE_HPP

#include "usertype_metatable.hpp"
#include "object.hpp"
#include <vector>
#include <unordered_map>
#include <utility>

namespace sol {

	namespace usertype_detail {
		const lua_Integer toplevel_magic = static_cast<lua_Integer>(0x00000001);

		struct variable_wrapper {
			virtual int index(lua_State* L) = 0;
			virtual int new_index(lua_State* L) = 0;
			virtual ~variable_wrapper() {};
		};

		template <typename T, typename F>
		struct callable_binding : variable_wrapper {
			F fx;

			template <typename Arg>
			callable_binding(Arg&& arg) : fx(std::forward<Arg>(arg)) {}

			virtual int index(lua_State* L) override {
				return call_detail::call_wrapped<T, true, true>(L, fx);
			}

			virtual int new_index(lua_State* L) override {
				return call_detail::call_wrapped<T, false, true>(L, fx);
			}
		};

		typedef std::unordered_map<std::string, std::unique_ptr<variable_wrapper>> variable_map;
		typedef std::unordered_map<std::string, object> function_map;

		struct simple_map {
			const char* metakey;
			variable_map variables;
			function_map functions;
			base_walk indexbaseclasspropogation;
			base_walk newindexbaseclasspropogation;

			simple_map(const char* mkey, base_walk index, base_walk newindex, variable_map&& vars, function_map&& funcs) : metakey(mkey), variables(std::move(vars)), functions(std::move(funcs)), indexbaseclasspropogation(index), newindexbaseclasspropogation(newindex) {}
		};

		template <typename T>
		inline int simple_metatable_newindex(lua_State* L) {
			int isnum = 0;
			lua_Integer magic = lua_tointegerx(L, lua_upvalueindex(4), &isnum);
			if (isnum != 0 && magic == toplevel_magic) {
				for (std::size_t i = 0; i < 3; lua_pop(L, 1), ++i) {
					// Pointer types, AKA "references" from C++
					const char* metakey = nullptr;
					switch (i) {
					case 0:
						metakey = &usertype_traits<T*>::metatable()[0];
						break;
					case 1:
						metakey = &usertype_traits<detail::unique_usertype<T>>::metatable()[0];
						break;
					case 2:
					default:
						metakey = &usertype_traits<T>::metatable()[0];
						break;
					}
					luaL_getmetatable(L, metakey);
					int tableindex = lua_gettop(L);
					if (type_of(L, tableindex) == type::lua_nil) {
						continue;
					}
					stack::set_field<false, true>(L, stack_reference(L, 2), stack_reference(L, 3), tableindex);
				}
				lua_settop(L, 0);
				return 0;
			}
			lua_pop(L, 1);
			return indexing_fail<false>(L);
		}

		template <bool is_index, bool toplevel = false>
		inline int simple_core_indexing_call(lua_State* L) {
			simple_map& sm = toplevel ? stack::get<user<simple_map>>(L, upvalue_index(1)) : stack::pop<user<simple_map>>(L);
			variable_map& variables = sm.variables;
			function_map& functions = sm.functions;
			static const int keyidx = -2 + static_cast<int>(is_index);
			if (toplevel) {
				if (stack::get<type>(L, keyidx) != type::string) {
					lua_CFunction indexingfunc = is_index ? stack::get<lua_CFunction>(L, upvalue_index(2)) : stack::get<lua_CFunction>(L, upvalue_index(3));
					return indexingfunc(L);
				}
			}
			string_detail::string_shim accessor = stack::get<string_detail::string_shim>(L, keyidx);
			std::string accessorkey = accessor.c_str();
			auto vit = variables.find(accessorkey);
			if (vit != variables.cend()) {
				auto& varwrap = *(vit->second);
				if (is_index) {
					return varwrap.index(L);
				}
				return varwrap.new_index(L);
			}
			auto fit = functions.find(accessorkey);
			if (fit != functions.cend()) {
				auto& func = (fit->second);
				return stack::push(L, func);
			}
			// Check table storage first for a method that works
			luaL_getmetatable(L, sm.metakey);
			if (type_of(L, -1) != type::lua_nil) {
				stack::get_field<false, true>(L, accessor.c_str(), lua_gettop(L));
				if (type_of(L, -1) != type::lua_nil) {
					// Woo, we found it?
					lua_remove(L, -2);
					return 1;
				}
				lua_pop(L, 1);
			}
			lua_pop(L, 1);
			
			int ret = 0;
			bool found = false;
			// Otherwise, we need to do propagating calls through the bases
			if (is_index) {
				sm.indexbaseclasspropogation(L, found, ret, accessor);
			}
			else {
				sm.newindexbaseclasspropogation(L, found, ret, accessor);
			}
			if (found) {
				return ret;
			}
			if (toplevel) {
				lua_CFunction indexingfunc = is_index ? stack::get<lua_CFunction>(L, upvalue_index(2)) : stack::get<lua_CFunction>(L, upvalue_index(3));
				return indexingfunc(L);
			}
			return -1;
		}

		inline int simple_real_index_call(lua_State* L) {
			return simple_core_indexing_call<true, true>(L);
		}

		inline int simple_real_new_index_call(lua_State* L) {
			return simple_core_indexing_call<false, true>(L);
		}

		inline int simple_index_call(lua_State* L) {
			return detail::static_trampoline<(&simple_real_index_call)>(L);
		}

		inline int simple_new_index_call(lua_State* L) {
			return detail::static_trampoline<(&simple_real_new_index_call)>(L);
		}
	}

	struct simple_tag {} const simple{};

	template <typename T>
	struct simple_usertype_metatable : usertype_detail::registrar {
	public:
		usertype_detail::function_map registrations;
		usertype_detail::variable_map varmap;
		object callconstructfunc;
		lua_CFunction indexfunc;
		lua_CFunction newindexfunc;
		lua_CFunction indexbase;
		lua_CFunction newindexbase;
		usertype_detail::base_walk indexbaseclasspropogation;
		usertype_detail::base_walk newindexbaseclasspropogation;
		void* baseclasscheck;
		void* baseclasscast;
		bool mustindex;
		bool secondarymeta;

		template <typename N>
		void insert(N&& n, object&& o) {
			std::string key = usertype_detail::make_string(std::forward<N>(n));
			auto hint = registrations.find(key);
			if (hint == registrations.cend()) {
				registrations.emplace_hint(hint, std::move(key), std::move(o));
				return;
			}
			hint->second = std::move(o);
		}

		template <typename N, typename F, meta::enable<meta::is_callable<meta::unwrap_unqualified_t<F>>> = meta::enabler>
		void add_function(lua_State* L, N&& n, F&& f) {
			insert(std::forward<N>(n), make_object(L, as_function_reference(std::forward<F>(f))));
		}

		template <typename N, typename F, meta::disable<meta::is_callable<meta::unwrap_unqualified_t<F>>> = meta::enabler>
		void add_function(lua_State* L, N&& n, F&& f) {
			object o = make_object(L, std::forward<F>(f));
			if (std::is_same<meta::unqualified_t<N>, call_construction>::value) {
				callconstructfunc = std::move(o);
				return;
			}
			insert(std::forward<N>(n), std::move(o));
		}

		template <typename N, typename F, meta::disable<is_variable_binding<meta::unqualified_t<F>>> = meta::enabler>
		void add(lua_State* L, N&& n, F&& f) {
			add_function(L, std::forward<N>(n), std::forward<F>(f));
		}

		template <typename N, typename F, meta::enable<is_variable_binding<meta::unqualified_t<F>>> = meta::enabler>
		void add(lua_State*, N&& n, F&& f) {
			mustindex = true;
			secondarymeta = true;
			std::string key = usertype_detail::make_string(std::forward<N>(n));
			auto o = std::make_unique<usertype_detail::callable_binding<T, std::decay_t<F>>>(std::forward<F>(f));
			auto hint = varmap.find(key);
			if (hint == varmap.cend()) {
				varmap.emplace_hint(hint, std::move(key), std::move(o));
				return;
			}
			hint->second = std::move(o);
		}

		template <typename N, typename... Fxs>
		void add(lua_State* L, N&& n, constructor_wrapper<Fxs...> c) {
			object o(L, in_place<detail::tagged<T, constructor_wrapper<Fxs...>>>, std::move(c));
			if (std::is_same<meta::unqualified_t<N>, call_construction>::value) {
				callconstructfunc = std::move(o);
				return;
			}
			insert(std::forward<N>(n), std::move(o));
		}

		template <typename N, typename... Lists>
		void add(lua_State* L, N&& n, constructor_list<Lists...> c) {
			object o(L, in_place<detail::tagged<T, constructor_list<Lists...>>>, std::move(c));
			if (std::is_same<meta::unqualified_t<N>, call_construction>::value) {
				callconstructfunc = std::move(o);
				return;
			}
			insert(std::forward<N>(n), std::move(o));
		}

		template <typename N>
		void add(lua_State* L, N&& n, destructor_wrapper<void> c) {
			object o(L, in_place<detail::tagged<T, destructor_wrapper<void>>>, std::move(c));
			if (std::is_same<meta::unqualified_t<N>, call_construction>::value) {
				callconstructfunc = std::move(o);
				return;
			}
			insert(std::forward<N>(n), std::move(o));
		}

		template <typename N, typename Fx>
		void add(lua_State* L, N&& n, destructor_wrapper<Fx> c) {
			object o(L, in_place<detail::tagged<T, destructor_wrapper<Fx>>>, std::move(c));
			if (std::is_same<meta::unqualified_t<N>, call_construction>::value) {
				callconstructfunc = std::move(o);
				return;
			}
			insert(std::forward<N>(n), std::move(o));
		}

		template <typename... Bases>
		void add(lua_State*, base_classes_tag, bases<Bases...>) {
			static_assert(sizeof(usertype_detail::base_walk) <= sizeof(void*), "size of function pointer is greater than sizeof(void*); cannot work on this platform. Please file a bug report.");
			if (sizeof...(Bases) < 1) {
				return;
			}
			mustindex = true;
			(void)detail::swallow{ 0, ((detail::has_derived<Bases>::value = true), 0)... };

			static_assert(sizeof(void*) <= sizeof(detail::inheritance_check_function), "The size of this data pointer is too small to fit the inheritance checking function: Please file a bug report.");
			static_assert(sizeof(void*) <= sizeof(detail::inheritance_cast_function), "The size of this data pointer is too small to fit the inheritance checking function: Please file a bug report.");
			baseclasscheck = (void*)&detail::inheritance<T, Bases...>::type_check;
			baseclasscast = (void*)&detail::inheritance<T, Bases...>::type_cast;
			indexbaseclasspropogation = usertype_detail::walk_all_bases<true, Bases...>;
			newindexbaseclasspropogation = usertype_detail::walk_all_bases<false, Bases...>;
		}

	private:
		template<std::size_t... I, typename Tuple>
		simple_usertype_metatable(usertype_detail::verified_tag, std::index_sequence<I...>, lua_State* L, Tuple&& args)
			: callconstructfunc(lua_nil),
			indexfunc(&usertype_detail::indexing_fail<true>), newindexfunc(&usertype_detail::indexing_fail<false>),
			indexbase(&usertype_detail::simple_core_indexing_call<true>), newindexbase(&usertype_detail::simple_core_indexing_call<false>),
			indexbaseclasspropogation(usertype_detail::walk_all_bases<true>), newindexbaseclasspropogation(&usertype_detail::walk_all_bases<false>),
			baseclasscheck(nullptr), baseclasscast(nullptr),
			mustindex(false), secondarymeta(false) {
			(void)detail::swallow{ 0,
				(add(L, detail::forward_get<I * 2>(args), detail::forward_get<I * 2 + 1>(args)),0)...
			};
		}

		template<typename... Args>
		simple_usertype_metatable(lua_State* L, usertype_detail::verified_tag v, Args&&... args) : simple_usertype_metatable(v, std::make_index_sequence<sizeof...(Args) / 2>(), L, std::forward_as_tuple(std::forward<Args>(args)...)) {}

		template<typename... Args>
		simple_usertype_metatable(lua_State* L, usertype_detail::add_destructor_tag, Args&&... args) : simple_usertype_metatable(L, usertype_detail::verified, std::forward<Args>(args)..., "__gc", default_destructor) {}

		template<typename... Args>
		simple_usertype_metatable(lua_State* L, usertype_detail::check_destructor_tag, Args&&... args) : simple_usertype_metatable(L, meta::condition<meta::all<std::is_destructible<T>, meta::neg<usertype_detail::has_destructor<Args...>>>, usertype_detail::add_destructor_tag, usertype_detail::verified_tag>(), std::forward<Args>(args)...) {}

	public:
		simple_usertype_metatable(lua_State* L) : simple_usertype_metatable(L, meta::condition<meta::all<std::is_default_constructible<T>>, decltype(default_constructor), usertype_detail::check_destructor_tag>()) {}

		template<typename Arg, typename... Args, meta::disable_any<
			meta::any_same<meta::unqualified_t<Arg>, 
				usertype_detail::verified_tag, 
				usertype_detail::add_destructor_tag,
				usertype_detail::check_destructor_tag
			>,
			meta::is_specialization_of<constructors, meta::unqualified_t<Arg>>,
			meta::is_specialization_of<constructor_wrapper, meta::unqualified_t<Arg>>
		> = meta::enabler>
		simple_usertype_metatable(lua_State* L, Arg&& arg, Args&&... args) : simple_usertype_metatable(L, meta::condition<meta::all<std::is_default_constructible<T>, meta::neg<usertype_detail::has_constructor<Args...>>>, decltype(default_constructor), usertype_detail::check_destructor_tag>(), std::forward<Arg>(arg), std::forward<Args>(args)...) {}

		template<typename... Args, typename... CArgs>
		simple_usertype_metatable(lua_State* L, constructors<CArgs...> constructorlist, Args&&... args) : simple_usertype_metatable(L, usertype_detail::check_destructor_tag(), std::forward<Args>(args)..., "new", constructorlist) {}

		template<typename... Args, typename... Fxs>
		simple_usertype_metatable(lua_State* L, constructor_wrapper<Fxs...> constructorlist, Args&&... args) : simple_usertype_metatable(L, usertype_detail::check_destructor_tag(), std::forward<Args>(args)..., "new", constructorlist) {}

		virtual int push_um(lua_State* L) override {
			return stack::push(L, std::move(*this));
		}
	};

	namespace stack {
		template <typename T>
		struct pusher<simple_usertype_metatable<T>> {
			typedef simple_usertype_metatable<T> umt_t;
			
			static usertype_detail::simple_map& make_cleanup(lua_State* L, umt_t& umx) {
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
				stack::push_specific<user<usertype_detail::simple_map>>(L, metatable_key, uniquegcmetakey, &usertype_traits<T>::metatable()[0], 
					umx.indexbaseclasspropogation, umx.newindexbaseclasspropogation, 
					std::move(umx.varmap), std::move(umx.registrations)
				);
				stack_reference stackvarmap(L, -1);
				stack::set_field<true>(L, gcmetakey, stackvarmap);
				stackvarmap.pop();

				stack::get_field<true>(L, gcmetakey);
				usertype_detail::simple_map& varmap = stack::pop<light<usertype_detail::simple_map>>(L);
				return varmap;
			}

			static int push(lua_State* L, umt_t&& umx) {
				auto& varmap = make_cleanup(L, umx);
				bool hasequals = false;
				bool hasless = false;
				bool haslessequals = false;
				auto register_kvp = [&](std::size_t i, stack_reference& t, const std::string& first, object& second) {
					if (first == name_of(meta_function::equal_to)) {
						hasequals = true;
					}
					else if (first == name_of(meta_function::less_than)) {
						hasless = true;
					}
					else if (first == name_of(meta_function::less_than_or_equal_to)) {
						haslessequals = true;
					}
					else if (first == name_of(meta_function::index)) {
						umx.indexfunc = second.template as<lua_CFunction>();
					}
					else if (first == name_of(meta_function::new_index)) {
						umx.newindexfunc = second.template as<lua_CFunction>();
					}
					switch (i) {
					case 0:
						if (first == name_of(meta_function::garbage_collect)) {
							return;
						}
						break;
					case 1:
						if (first == name_of(meta_function::garbage_collect)) {
							stack::set_field(L, first, detail::unique_destruct<T>, t.stack_index());
							return;
						}
						break;
					case 2:
					default:
						break;
					}
					stack::set_field(L, first, second, t.stack_index());
				};
				for (std::size_t i = 0; i < 3; ++i) {
					// Pointer types, AKA "references" from C++
					const char* metakey = nullptr;
					switch (i) {
					case 0:
						metakey = &usertype_traits<T*>::metatable()[0];
						break;
					case 1:
						metakey = &usertype_traits<detail::unique_usertype<T>>::metatable()[0];
						break;
					case 2:
					default:
						metakey = &usertype_traits<T>::metatable()[0];
						break;
					}
					luaL_newmetatable(L, metakey);
					stack_reference t(L, -1);
					for (auto& kvp : varmap.functions) {
						auto& first = std::get<0>(kvp);
						auto& second = std::get<1>(kvp);
						register_kvp(i, t, first, second);
					}
					luaL_Reg opregs[4]{};
					int opregsindex = 0;
					if (!hasless) {
						const char* name = name_of(meta_function::less_than).c_str();
						usertype_detail::make_reg_op<T, std::less<>, meta::supports_op_less<T>>(opregs, opregsindex, name);
					}
					if (!haslessequals) {
						const char* name = name_of(meta_function::less_than_or_equal_to).c_str();
						usertype_detail::make_reg_op<T, std::less_equal<>, meta::supports_op_less_equal<T>>(opregs, opregsindex, name);
					}
					if (!hasequals) {
						const char* name = name_of(meta_function::equal_to).c_str();
						usertype_detail::make_reg_op<T, std::conditional_t<meta::supports_op_equal<T>::value, std::equal_to<>, usertype_detail::no_comp>, std::true_type>(opregs, opregsindex, name);
					}
					t.push();
					luaL_setfuncs(L, opregs, 0);
					t.pop();

					if (umx.baseclasscheck != nullptr) {
						stack::set_field(L, detail::base_class_check_key(), umx.baseclasscheck, t.stack_index());
					}
					if (umx.baseclasscast != nullptr) {
						stack::set_field(L, detail::base_class_cast_key(), umx.baseclasscast, t.stack_index());
					}

					// Base class propagation features
					stack::set_field(L, detail::base_class_index_propogation_key(), umx.indexbase, t.stack_index());
					stack::set_field(L, detail::base_class_new_index_propogation_key(), umx.newindexbase, t.stack_index());

					if (umx.mustindex) {
						// use indexing function
						stack::set_field(L, meta_function::index,
							make_closure(&usertype_detail::simple_index_call,
								make_light(varmap),
								umx.indexfunc,
								umx.newindexfunc
							), t.stack_index());
						stack::set_field(L, meta_function::new_index,
							make_closure(&usertype_detail::simple_new_index_call,
								make_light(varmap),
								umx.indexfunc,
								umx.newindexfunc
							), t.stack_index());
					}
					else {
						// Metatable indexes itself
						stack::set_field(L, meta_function::index, t, t.stack_index());
					}
					// metatable on the metatable
					// for call constructor purposes and such
					lua_createtable(L, 0, 2 * static_cast<int>(umx.secondarymeta) + static_cast<int>(umx.callconstructfunc.valid()));
					stack_reference metabehind(L, -1);
					if (umx.callconstructfunc.valid()) {
						stack::set_field(L, sol::meta_function::call_function, umx.callconstructfunc, metabehind.stack_index());
					}
					if (umx.secondarymeta) {
						stack::set_field(L, meta_function::index, 
							make_closure(&usertype_detail::simple_index_call,
								make_light(varmap),
								umx.indexfunc,
								umx.newindexfunc
							), metabehind.stack_index());
						stack::set_field(L, meta_function::new_index, 
							make_closure(&usertype_detail::simple_new_index_call,
								make_light(varmap),
								umx.indexfunc,
								umx.newindexfunc
							), metabehind.stack_index());
					}
					stack::set_field(L, metatable_key, metabehind, t.stack_index());
					metabehind.pop();

					t.pop();
				}

				// Now for the shim-table that actually gets pushed
				luaL_newmetatable(L, &usertype_traits<T>::user_metatable()[0]);
				stack_reference t(L, -1);
				for (auto& kvp : varmap.functions) {
					auto& first = std::get<0>(kvp);
					auto& second = std::get<1>(kvp);
					register_kvp(2, t, first, second);
				}
				{
					lua_createtable(L, 0, 2 + static_cast<int>(umx.callconstructfunc.valid()));
					stack_reference metabehind(L, -1);
					if (umx.callconstructfunc.valid()) {
						stack::set_field(L, sol::meta_function::call_function, umx.callconstructfunc, metabehind.stack_index());
					}
					// use indexing function
					stack::set_field(L, meta_function::index,
						make_closure(&usertype_detail::simple_index_call,
							make_light(varmap),
							&usertype_detail::simple_index_call,
							&usertype_detail::simple_metatable_newindex<T>,
							usertype_detail::toplevel_magic
						), metabehind.stack_index());
					stack::set_field(L, meta_function::new_index,
						make_closure(&usertype_detail::simple_new_index_call,
							make_light(varmap),
							&usertype_detail::simple_index_call,
							&usertype_detail::simple_metatable_newindex<T>,
							usertype_detail::toplevel_magic
						), metabehind.stack_index());
					stack::set_field(L, metatable_key, metabehind, t.stack_index());
					metabehind.pop();
				}

				// Don't pop the table when we're done;
				// return it
				return 1;
			}
		};
	} // stack
} // sol

#endif // SOL_SIMPLE_USERTYPE_METATABLE_HPP
