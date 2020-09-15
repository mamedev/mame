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

#ifndef SOL_FUNCTION_TYPES_HPP
#define SOL_FUNCTION_TYPES_HPP

#include "function_types_core.hpp"
#include "function_types_templated.hpp"
#include "function_types_stateless.hpp"
#include "function_types_stateful.hpp"
#include "function_types_overloaded.hpp"
#include "resolve.hpp"
#include "call.hpp"

namespace sol {
	namespace stack {
		template<typename... Sigs>
		struct pusher<function_sig<Sigs...>> {
			template <typename... Sig, typename Fx, typename... Args>
			static void select_convertible(std::false_type, types<Sig...>, lua_State* L, Fx&& fx, Args&&... args) {
				typedef std::remove_pointer_t<std::decay_t<Fx>> clean_fx;
				typedef function_detail::functor_function<clean_fx> F;
				set_fx<F>(L, std::forward<Fx>(fx), std::forward<Args>(args)...);
			}

			template <typename R, typename... A, typename Fx, typename... Args>
			static void select_convertible(std::true_type, types<R(A...)>, lua_State* L, Fx&& fx, Args&&... args) {
				using fx_ptr_t = R(*)(A...);
				fx_ptr_t fxptr = detail::unwrap(std::forward<Fx>(fx));
				select_function(std::true_type(), L, fxptr, std::forward<Args>(args)...);
			}

			template <typename R, typename... A, typename Fx, typename... Args>
			static void select_convertible(types<R(A...)> t, lua_State* L, Fx&& fx, Args&&... args) {
				typedef std::decay_t<meta::unwrap_unqualified_t<Fx>> raw_fx_t;
				typedef R(*fx_ptr_t)(A...);
				typedef std::is_convertible<raw_fx_t, fx_ptr_t> is_convertible;
				select_convertible(is_convertible(), t, L, std::forward<Fx>(fx), std::forward<Args>(args)...);
			}

			template <typename Fx, typename... Args>
			static void select_convertible(types<>, lua_State* L, Fx&& fx, Args&&... args) {
				typedef meta::function_signature_t<meta::unwrap_unqualified_t<Fx>> Sig;
				select_convertible(types<Sig>(), L, std::forward<Fx>(fx), std::forward<Args>(args)...);
			}

			template <typename Fx, typename T, typename... Args>
			static void select_reference_member_variable(std::false_type, lua_State* L, Fx&& fx, T&& obj, Args&&... args) {
				typedef std::remove_pointer_t<std::decay_t<Fx>> clean_fx;
				typedef function_detail::member_variable<meta::unwrap_unqualified_t<T>, clean_fx> F;
				set_fx<F>(L, std::forward<Fx>(fx), std::forward<T>(obj), std::forward<Args>(args)...);
			}

			template <typename Fx, typename T, typename... Args>
			static void select_reference_member_variable(std::true_type, lua_State* L, Fx&& fx, T&& obj, Args&&... args) {
				typedef std::decay_t<Fx> dFx;
				dFx memfxptr(std::forward<Fx>(fx));
				auto userptr = detail::ptr(std::forward<T>(obj), std::forward<Args>(args)...);
				lua_CFunction freefunc = &function_detail::upvalue_member_variable<std::decay_t<decltype(*userptr)>, meta::unqualified_t<Fx>>::call;

				int upvalues = stack::stack_detail::push_as_upvalues(L, memfxptr);
				upvalues += stack::push(L, lightuserdata_value(static_cast<void*>(userptr)));
				stack::push(L, c_closure(freefunc, upvalues));
			}

			template <typename Fx, typename... Args>
			static void select_member_variable(std::false_type, lua_State* L, Fx&& fx, Args&&... args) {
				select_convertible(types<Sigs...>(), L, std::forward<Fx>(fx), std::forward<Args>(args)...);
			}

			template <typename Fx, typename T, typename... Args>
			static void select_member_variable(std::true_type, lua_State* L, Fx&& fx, T&& obj, Args&&... args) {
				typedef meta::boolean<meta::is_specialization_of<std::reference_wrapper, meta::unqualified_t<T>>::value || std::is_pointer<T>::value> is_reference;
				select_reference_member_variable(is_reference(), L, std::forward<Fx>(fx), std::forward<T>(obj), std::forward<Args>(args)...);
			}

			template <typename Fx>
			static void select_member_variable(std::true_type, lua_State* L, Fx&& fx) {
				typedef typename meta::bind_traits<meta::unqualified_t<Fx>>::object_type C;
				lua_CFunction freefunc = &function_detail::upvalue_this_member_variable<C, Fx>::call;
				int upvalues = stack::stack_detail::push_as_upvalues(L, fx);
				stack::push(L, c_closure(freefunc, upvalues));
			}

			template <typename Fx, typename T, typename... Args>
			static void select_reference_member_function(std::false_type, lua_State* L, Fx&& fx, T&& obj, Args&&... args) {
				typedef std::decay_t<Fx> clean_fx;
				typedef function_detail::member_function<meta::unwrap_unqualified_t<T>, clean_fx> F;
				set_fx<F>(L, std::forward<Fx>(fx), std::forward<T>(obj), std::forward<Args>(args)...);
			}

			template <typename Fx, typename T, typename... Args>
			static void select_reference_member_function(std::true_type, lua_State* L, Fx&& fx, T&& obj, Args&&... args) {
				typedef std::decay_t<Fx> dFx;
				dFx memfxptr(std::forward<Fx>(fx));
				auto userptr = detail::ptr(std::forward<T>(obj), std::forward<Args>(args)...);
				lua_CFunction freefunc = &function_detail::upvalue_member_function<std::decay_t<decltype(*userptr)>, meta::unqualified_t<Fx>>::call;

				int upvalues = stack::stack_detail::push_as_upvalues(L, memfxptr);
				upvalues += stack::push(L, lightuserdata_value(static_cast<void*>(userptr)));
				stack::push(L, c_closure(freefunc, upvalues));
			}

			template <typename Fx, typename... Args>
			static void select_member_function(std::false_type, lua_State* L, Fx&& fx, Args&&... args) {
				select_member_variable(std::is_member_object_pointer<meta::unqualified_t<Fx>>(), L, std::forward<Fx>(fx), std::forward<Args>(args)...);
			}

			template <typename Fx, typename T, typename... Args>
			static void select_member_function(std::true_type, lua_State* L, Fx&& fx, T&& obj, Args&&... args) {
				typedef meta::boolean<meta::is_specialization_of<std::reference_wrapper, meta::unqualified_t<T>>::value || std::is_pointer<T>::value> is_reference;
				select_reference_member_function(is_reference(), L, std::forward<Fx>(fx), std::forward<T>(obj), std::forward<Args>(args)...);
			}

			template <typename Fx>
			static void select_member_function(std::true_type, lua_State* L, Fx&& fx) {
				typedef typename meta::bind_traits<meta::unqualified_t<Fx>>::object_type C;
				lua_CFunction freefunc = &function_detail::upvalue_this_member_function<C, Fx>::call;
				int upvalues = stack::stack_detail::push_as_upvalues(L, fx);
				stack::push(L, c_closure(freefunc, upvalues));
			}

			template <typename Fx, typename... Args>
			static void select_function(std::false_type, lua_State* L, Fx&& fx, Args&&... args) {
				select_member_function(std::is_member_function_pointer<meta::unqualified_t<Fx>>(), L, std::forward<Fx>(fx), std::forward<Args>(args)...);
			}

			template <typename Fx, typename... Args>
			static void select_function(std::true_type, lua_State* L, Fx&& fx, Args&&... args) {
				std::decay_t<Fx> target(std::forward<Fx>(fx), std::forward<Args>(args)...);
				lua_CFunction freefunc = &function_detail::upvalue_free_function<Fx>::call;

				int upvalues = stack::stack_detail::push_as_upvalues(L, target);
				stack::push(L, c_closure(freefunc, upvalues));
			}

			static void select_function(std::true_type, lua_State* L, lua_CFunction f) {
				stack::push(L, f);
			}

			template <typename Fx, typename... Args>
			static void select(lua_State* L, Fx&& fx, Args&&... args) {
				select_function(std::is_function<meta::unqualified_t<Fx>>(), L, std::forward<Fx>(fx), std::forward<Args>(args)...);
			}

			template <typename Fx, typename... Args>
			static void set_fx(lua_State* L, Args&&... args) {
				lua_CFunction freefunc = function_detail::call<meta::unqualified_t<Fx>>;

				stack::push_specific<user<Fx>>(L, std::forward<Args>(args)...);
				stack::push(L, c_closure(freefunc, 1));
			}

			template<typename... Args>
			static int push(lua_State* L, Args&&... args) {
				// Set will always place one thing (function) on the stack
				select(L, std::forward<Args>(args)...);
				return 1;
			}
		};

		template<typename T, typename... Args>
		struct pusher<function_arguments<T, Args...>> {
			template <std::size_t... I, typename FP>
			static int push_func(std::index_sequence<I...>, lua_State* L, FP&& fp) {
				return stack::push_specific<T>(L, detail::forward_get<I>(fp.arguments)...);
			}

			static int push(lua_State* L, const function_arguments<T, Args...>& fp) {
				return push_func(std::make_index_sequence<sizeof...(Args)>(), L, fp);
			}

			static int push(lua_State* L, function_arguments<T, Args...>&& fp) {
				return push_func(std::make_index_sequence<sizeof...(Args)>(), L, std::move(fp));
			}
		};

		template<typename Signature>
		struct pusher<std::function<Signature>> {
			static int push(lua_State* L, std::function<Signature> fx) {
				return pusher<function_sig<Signature>>{}.push(L, std::move(fx));
			}
		};

		template<typename Signature>
		struct pusher<Signature, std::enable_if_t<std::is_member_pointer<Signature>::value>> {
			template <typename F>
			static int push(lua_State* L, F&& f) {
				return pusher<function_sig<>>{}.push(L, std::forward<F>(f));
			}
		};

		template<typename Signature>
		struct pusher<Signature, std::enable_if_t<meta::all<std::is_function<Signature>, meta::neg<std::is_same<Signature, lua_CFunction>>, meta::neg<std::is_same<Signature, std::remove_pointer_t<lua_CFunction>>>>::value>> {
			template <typename F>
			static int push(lua_State* L, F&& f) {
				return pusher<function_sig<>>{}.push(L, std::forward<F>(f));
			}
		};

		template<typename... Functions>
		struct pusher<overload_set<Functions...>> {
			static int push(lua_State* L, overload_set<Functions...>&& set) {
				typedef function_detail::overloaded_function<Functions...> F;
				pusher<function_sig<>>{}.set_fx<F>(L, std::move(set.functions));
				return 1;
			}

			static int push(lua_State* L, const overload_set<Functions...>& set) {
				typedef function_detail::overloaded_function<Functions...> F;
				pusher<function_sig<>>{}.set_fx<F>(L, set.functions);
				return 1;
			}
		};

		template <typename T>
		struct pusher<protect_t<T>> {
			static int push(lua_State* L, protect_t<T>&& pw) {
				lua_CFunction cf = call_detail::call_user<void, false, false, protect_t<T>>;
				int closures = stack::push_specific<user<protect_t<T>>>(L, std::move(pw.value));
				return stack::push(L, c_closure(cf, closures));
			}

			static int push(lua_State* L, const protect_t<T>& pw) {
				lua_CFunction cf = call_detail::call_user<void, false, false, protect_t<T>>;
				int closures = stack::push_specific<user<protect_t<T>>>(L, pw.value);
				return stack::push(L, c_closure(cf, closures));
			}
		};

		template <typename F, typename G>
		struct pusher<property_wrapper<F, G>, std::enable_if_t<!std::is_void<F>::value && !std::is_void<G>::value>> {
			static int push(lua_State* L, property_wrapper<F, G>&& pw) {
				return stack::push(L, sol::overload(std::move(pw.read), std::move(pw.write)));
			}
			static int push(lua_State* L, const property_wrapper<F, G>& pw) {
				return stack::push(L, sol::overload(pw.read, pw.write));
			}
		};

		template <typename F>
		struct pusher<property_wrapper<F, void>> {
			static int push(lua_State* L, property_wrapper<F, void>&& pw) {
				return stack::push(L, std::move(pw.read));
			}
			static int push(lua_State* L, const property_wrapper<F, void>& pw) {
				return stack::push(L, pw.read);
			}
		};

		template <typename F>
		struct pusher<property_wrapper<void, F>> {
			static int push(lua_State* L, property_wrapper<void, F>&& pw) {
				return stack::push(L, std::move(pw.write));
			}
			static int push(lua_State* L, const property_wrapper<void, F>& pw) {
				return stack::push(L, pw.write);
			}
		};

		template <typename T>
		struct pusher<var_wrapper<T>> {
			static int push(lua_State* L, var_wrapper<T>&& vw) {
				return stack::push(L, std::move(vw.value));
			}
			static int push(lua_State* L, const var_wrapper<T>& vw) {
				return stack::push(L, vw.value);
			}
		};

		template <typename... Functions>
		struct pusher<factory_wrapper<Functions...>> {
			static int push(lua_State* L, const factory_wrapper<Functions...>& fw) {
				typedef function_detail::overloaded_function<Functions...> F;
				pusher<function_sig<>>{}.set_fx<F>(L, fw.functions);
				return 1;
			}

			static int push(lua_State* L, factory_wrapper<Functions...>&& fw) {
				typedef function_detail::overloaded_function<Functions...> F;
				pusher<function_sig<>>{}.set_fx<F>(L, std::move(fw.functions));
				return 1;
			}
		};

		template <typename T, typename... Lists>
		struct pusher<detail::tagged<T, constructor_list<Lists...>>> {
			static int push(lua_State* L, detail::tagged<T, constructor_list<Lists...>>) {
				lua_CFunction cf = call_detail::construct<T, Lists...>;
				return stack::push(L, cf);
			}
		};

		template <typename T, typename... Fxs>
		struct pusher<detail::tagged<T, constructor_wrapper<Fxs...>>> {
			template <typename C>
			static int push(lua_State* L, C&& c) {
				lua_CFunction cf = call_detail::call_user<T, false, false, constructor_wrapper<Fxs...>>;
				int closures = stack::push_specific<user<constructor_wrapper<Fxs...>>>(L, std::forward<C>(c));
				return stack::push(L, c_closure(cf, closures));
			}
		};

		template <typename T>
		struct pusher<detail::tagged<T, destructor_wrapper<void>>> {
			static int push(lua_State* L, destructor_wrapper<void>) {
				lua_CFunction cf = detail::usertype_alloc_destroy<T>;
				return stack::push(L, cf);
			}
		};

		template <typename T, typename Fx>
		struct pusher<detail::tagged<T, destructor_wrapper<Fx>>> {
			static int push(lua_State* L, destructor_wrapper<Fx> c) {
				lua_CFunction cf = call_detail::call_user<T, false, false, destructor_wrapper<Fx>>;
				int closures = stack::push_specific<user<T>>(L, std::move(c));
				return stack::push(L, c_closure(cf, closures));
			}
		};

	} // stack
} // sol

#endif // SOL_FUNCTION_TYPES_HPP
