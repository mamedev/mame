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

#ifndef SOL_WRAPPER_HPP
#define SOL_WRAPPER_HPP

#include "types.hpp"

namespace sol {

	template <typename F, typename = void>
	struct wrapper {
		typedef lua_bind_traits<F> traits_type;
		typedef typename traits_type::args_list args_list;
		typedef typename traits_type::args_list free_args_list;
		typedef typename traits_type::returns_list returns_list;

		template <typename... Args>
		static decltype(auto) call(F& f, Args&&... args) {
			return f(std::forward<Args>(args)...);
		}

		struct caller {
			template <typename... Args>
			decltype(auto) operator()(F& fx, Args&&... args) const {
				return call(fx, std::forward<Args>(args)...);
			}
		};
	};

	template <typename F>
	struct wrapper<F, std::enable_if_t<std::is_function<meta::unqualified_t<std::remove_pointer_t<F>>>::value>> {
		typedef lua_bind_traits<F> traits_type;
		typedef typename traits_type::args_list args_list;
		typedef typename traits_type::args_list free_args_list;
		typedef typename traits_type::returns_list returns_list;

		template <F fx, typename... Args>
		static decltype(auto) invoke(Args&&... args) {
			return fx(std::forward<Args>(args)...);
		}

		template <typename... Args>
		static decltype(auto) call(F& fx, Args&&... args) {
			return fx(std::forward<Args>(args)...);
		}

		struct caller {
			template <typename... Args>
			decltype(auto) operator()(F& fx, Args&&... args) const {
				return call(fx, std::forward<Args>(args)...);
			}
		};

		template <F fx>
		struct invoker {
			template <typename... Args>
			decltype(auto) operator()(Args&&... args) const {
				return invoke<fx>(std::forward<Args>(args)...);
			}
		};
	};

	template <typename F>
	struct wrapper<F, std::enable_if_t<std::is_member_object_pointer<meta::unqualified_t<F>>::value>> {
		typedef lua_bind_traits<F> traits_type;
		typedef typename traits_type::object_type object_type;
		typedef typename traits_type::return_type return_type;
		typedef typename traits_type::args_list args_list;
		typedef types<object_type&, return_type> free_args_list;
		typedef typename traits_type::returns_list returns_list;

		template <F fx, typename... Args>
		static decltype(auto) invoke(object_type& mem, Args&&... args) {
			return (mem.*fx)(std::forward<Args>(args)...);
		}

		template <typename Fx>
		static decltype(auto) call(Fx&& fx, object_type& mem) {
			return (mem.*fx);
		}

		template <typename Fx, typename Arg, typename... Args>
		static void call(Fx&& fx, object_type& mem, Arg&& arg, Args&&...) {
			(mem.*fx) = std::forward<Arg>(arg);
		}

		struct caller {
			template <typename Fx, typename... Args>
			decltype(auto) operator()(Fx&& fx, object_type& mem, Args&&... args) const {
				return call(std::forward<Fx>(fx), mem, std::forward<Args>(args)...);
			}
		};

		template <F fx>
		struct invoker {
			template <typename... Args>
			decltype(auto) operator()(Args&&... args) const {
				return invoke<fx>(std::forward<Args>(args)...);
			}
		};
	};

	template <typename F, typename R, typename O, typename... FArgs>
	struct member_function_wrapper {
		typedef O object_type;
		typedef lua_bind_traits<F> traits_type;
		typedef typename traits_type::args_list args_list;
		typedef types<object_type&, FArgs...> free_args_list;
		typedef meta::tuple_types<R> returns_list;

		template <F fx, typename... Args>
		static R invoke(O& mem, Args&&... args) {
			return (mem.*fx)(std::forward<Args>(args)...);
		}

		template <typename Fx, typename... Args>
		static R call(Fx&& fx, O& mem, Args&&... args) {
			return (mem.*fx)(std::forward<Args>(args)...);
		}

		struct caller {
			template <typename Fx, typename... Args>
			decltype(auto) operator()(Fx&& fx, O& mem, Args&&... args) const {
				return call(std::forward<Fx>(fx), mem, std::forward<Args>(args)...);
			}
		};

		template <F fx>
		struct invoker {
			template <typename... Args>
			decltype(auto) operator()(O& mem, Args&&... args) const {
				return invoke<fx>(mem, std::forward<Args>(args)...);
			}
		};
	};

	template <typename R, typename O, typename... Args>
	struct wrapper<R(O:: *)(Args...)> : public member_function_wrapper<R(O:: *)(Args...), R, O, Args...> {

	};

	template <typename R, typename O, typename... Args>
	struct wrapper<R(O:: *)(Args...) const> : public member_function_wrapper<R(O:: *)(Args...) const, R, O, Args...> {

	};

	template <typename R, typename O, typename... Args>
	struct wrapper<R(O:: *)(Args...) const volatile> : public member_function_wrapper<R(O:: *)(Args...) const volatile, R, O, Args...> {

	};

	template <typename R, typename O, typename... Args>
	struct wrapper<R(O:: *)(Args...) &> : public member_function_wrapper<R(O:: *)(Args...) &, R, O, Args...> {

	};

	template <typename R, typename O, typename... Args>
	struct wrapper<R(O:: *)(Args...) const &> : public member_function_wrapper<R(O:: *)(Args...) const &, R, O, Args...> {

	};

	template <typename R, typename O, typename... Args>
	struct wrapper<R(O:: *)(Args...) const volatile &> : public member_function_wrapper<R(O:: *)(Args...) const volatile &, R, O, Args...> {

	};

	template <typename R, typename O, typename... Args>
	struct wrapper<R(O:: *)(Args..., ...) &> : public member_function_wrapper<R(O:: *)(Args..., ...) &, R, O, Args...> {

	};

	template <typename R, typename O, typename... Args>
	struct wrapper<R(O:: *)(Args..., ...) const &> : public member_function_wrapper<R(O:: *)(Args..., ...) const &, R, O, Args...> {

	};

	template <typename R, typename O, typename... Args>
	struct wrapper<R(O:: *)(Args..., ...) const volatile &> : public member_function_wrapper<R(O:: *)(Args..., ...) const volatile &, R, O, Args...> {

	};

	template <typename R, typename O, typename... Args>
	struct wrapper<R(O:: *)(Args...) && > : public member_function_wrapper<R(O:: *)(Args...) &, R, O, Args...> {

	};

	template <typename R, typename O, typename... Args>
	struct wrapper<R(O:: *)(Args...) const &&> : public member_function_wrapper<R(O:: *)(Args...) const &, R, O, Args...> {

	};

	template <typename R, typename O, typename... Args>
	struct wrapper<R(O:: *)(Args...) const volatile &&> : public member_function_wrapper<R(O:: *)(Args...) const volatile &, R, O, Args...> {

	};

	template <typename R, typename O, typename... Args>
	struct wrapper<R(O:: *)(Args..., ...) && > : public member_function_wrapper<R(O:: *)(Args..., ...) &, R, O, Args...> {

	};

	template <typename R, typename O, typename... Args>
	struct wrapper<R(O:: *)(Args..., ...) const &&> : public member_function_wrapper<R(O:: *)(Args..., ...) const &, R, O, Args...> {

	};

	template <typename R, typename O, typename... Args>
	struct wrapper<R(O:: *)(Args..., ...) const volatile &&> : public member_function_wrapper<R(O:: *)(Args..., ...) const volatile &, R, O, Args...> {

	};

} // sol

#endif // SOL_WRAPPER_HPP
