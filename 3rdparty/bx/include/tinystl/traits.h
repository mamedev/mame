/*-
 * Copyright 2012 Matthew Endsley
 * All rights reserved
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted providing that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY
 * DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT,
 * STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING
 * IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 */

#ifndef TINYSTL_TRAITS_H
#define TINYSTL_TRAITS_H

#include "new.h"

#if defined(__GNUC__) && defined(__is_pod)
#	define TINYSTL_TRY_POD_OPTIMIZATION(t) __is_pod(t)
#elif defined(_MSC_VER)
#	define TINYSTL_TRY_POD_OPTIMIZATION(t) (!__is_class(t) || __is_pod(t))
#else
#	define TINYSTL_TRY_POD_OPTIMIZATION(t) false
#endif

namespace tinystl {
	template<typename T, bool pod = TINYSTL_TRY_POD_OPTIMIZATION(T)> struct pod_traits {};

	template<typename T, T t> struct swap_holder;

	template<typename T>
	static inline void move_impl(T& a, T& b, ...) {
		a = b;
	}

	template<typename T>
	static inline void move_impl(T& a, T& b, T*, swap_holder<void (T::*)(T&), &T::swap>* = 0) {
		a.swap(b);
	}

	template<typename T>
	static inline void move(T& a, T&b) {
		move_impl(a, b, (T*)0);
	}

	template<typename T>
	static inline void move_construct_impl(T* a, T& b, ...) {
		new(placeholder(), a) T(b);
	}

	template<typename T>
	static inline void move_construct_impl(T* a, T& b, void*, swap_holder<void (T::*)(T&), &T::swap>* = 0) {
		// If your type T does not have a default constructor, simply insert:
		// struct tinystl_nomove_construct;
		// in the class definition
		new(placeholder(), a) T;
		a->swap(b);
	}

	template<typename T>
	static inline void move_construct_impl(T* a, T& b, T*, typename T::tinystl_nomove_construct* = 0) {
		new(placeholder(), a) T(b);
	}

	template<typename T>
	static inline void move_construct(T* a, T& b) {
		move_construct_impl(a, b, (T*)0);
	}
}

#endif
