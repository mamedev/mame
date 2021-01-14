/*
 * Copyright 2010-2021 Branimir Karadzic. All rights reserved.
 * License: https://github.com/bkaradzic/bx#license-bsd-2-clause
 */

#ifndef BX_H_HEADER_GUARD
#	error "Do not include macros.h directly #include <bx/bx.h> instead."
#endif // BX_H_HEADER_GUARD

#ifndef BX_MACROS_H_HEADER_GUARD
#define BX_MACROS_H_HEADER_GUARD

///
#if BX_COMPILER_MSVC
// Workaround MSVS bug...
#	define BX_VA_ARGS_PASS(...) BX_VA_ARGS_PASS_1_ __VA_ARGS__ BX_VA_ARGS_PASS_2_
#	define BX_VA_ARGS_PASS_1_ (
#	define BX_VA_ARGS_PASS_2_ )
#else
#	define BX_VA_ARGS_PASS(...) (__VA_ARGS__)
#endif // BX_COMPILER_MSVC

#define BX_VA_ARGS_COUNT(...) BX_VA_ARGS_COUNT_ BX_VA_ARGS_PASS(__VA_ARGS__, 16, 15, 14, 13, 12, 11, 10, 9, 8, 7, 6, 5, 4, 3, 2, 1, 0)
#define BX_VA_ARGS_COUNT_(_a1, _a2, _a3, _a4, _a5, _a6, _a7, _a8, _a9, _a10, _a11, _a12, _a13, _a14, _a15, _a16, _last, ...) _last

///
#define BX_MACRO_DISPATCHER(_func, ...) BX_MACRO_DISPATCHER_1_(_func, BX_VA_ARGS_COUNT(__VA_ARGS__) )
#define BX_MACRO_DISPATCHER_1_(_func, _argCount) BX_MACRO_DISPATCHER_2_(_func, _argCount)
#define BX_MACRO_DISPATCHER_2_(_func, _argCount) BX_CONCATENATE(_func, _argCount)

///
#define BX_MAKEFOURCC(_a, _b, _c, _d) ( ( (uint32_t)(_a) | ( (uint32_t)(_b) << 8) | ( (uint32_t)(_c) << 16) | ( (uint32_t)(_d) << 24) ) )

///
#define BX_STRINGIZE(_x) BX_STRINGIZE_(_x)
#define BX_STRINGIZE_(_x) #_x

///
#define BX_CONCATENATE(_x, _y) BX_CONCATENATE_(_x, _y)
#define BX_CONCATENATE_(_x, _y) _x ## _y

///
#define BX_FILE_LINE_LITERAL "" __FILE__ "(" BX_STRINGIZE(__LINE__) "): "

///
#define BX_ALIGNOF(_type) __alignof(_type)

#if defined(__has_feature)
#	define BX_CLANG_HAS_FEATURE(_x) __has_feature(_x)
#else
#	define BX_CLANG_HAS_FEATURE(_x) 0
#endif // defined(__has_feature)

#if defined(__has_extension)
#	define BX_CLANG_HAS_EXTENSION(_x) __has_extension(_x)
#else
#	define BX_CLANG_HAS_EXTENSION(_x) 0
#endif // defined(__has_extension)

#if BX_COMPILER_GCC || BX_COMPILER_CLANG
#	define BX_ALIGN_DECL(_align, _decl) _decl __attribute__( (aligned(_align) ) )
#	define BX_ALLOW_UNUSED __attribute__( (unused) )
#	define BX_FORCE_INLINE inline __attribute__( (__always_inline__) )
#	define BX_FUNCTION __PRETTY_FUNCTION__
#	define BX_LIKELY(_x)   __builtin_expect(!!(_x), 1)
#	define BX_UNLIKELY(_x) __builtin_expect(!!(_x), 0)
#	define BX_NO_INLINE   __attribute__( (noinline) )
#	define BX_NO_RETURN   __attribute__( (noreturn) )
#	define BX_CONST_FUNC  __attribute__( (const) )

#	if BX_COMPILER_GCC >= 70000
#		define BX_FALLTHROUGH __attribute__( (fallthrough) )
#	else
#		define BX_FALLTHROUGH BX_NOOP()
#	endif // BX_COMPILER_GCC >= 70000

#	define BX_NO_VTABLE
#	define BX_PRINTF_ARGS(_format, _args) __attribute__( (format(__printf__, _format, _args) ) )

#	if BX_CLANG_HAS_FEATURE(cxx_thread_local) \
	|| (!BX_PLATFORM_OSX && (BX_COMPILER_GCC >= 40200) ) \
	|| (BX_COMPILER_GCC >= 40500)
#		define BX_THREAD_LOCAL __thread
#	endif // BX_COMPILER_GCC

#	define BX_ATTRIBUTE(_x) __attribute__( (_x) )

#	if BX_CRT_MSVC
#		define __stdcall
#	endif // BX_CRT_MSVC
#elif BX_COMPILER_MSVC
#	define BX_ALIGN_DECL(_align, _decl) __declspec(align(_align) ) _decl
#	define BX_ALLOW_UNUSED
#	define BX_FORCE_INLINE __forceinline
#	define BX_FUNCTION __FUNCTION__
#	define BX_LIKELY(_x)   (_x)
#	define BX_UNLIKELY(_x) (_x)
#	define BX_NO_INLINE __declspec(noinline)
#	define BX_NO_RETURN
#	define BX_CONST_FUNC  __declspec(noalias)
#	define BX_FALLTHROUGH BX_NOOP()
#	define BX_NO_VTABLE __declspec(novtable)
#	define BX_PRINTF_ARGS(_format, _args)
#	define BX_THREAD_LOCAL __declspec(thread)
#	define BX_ATTRIBUTE(_x)
#else
#	error "Unknown BX_COMPILER_?"
#endif

/// The return value of the function is solely a function of the arguments.
///
#if __cplusplus < 201402
#	define BX_CONSTEXPR_FUNC BX_CONST_FUNC
#else
#	define BX_CONSTEXPR_FUNC constexpr BX_CONST_FUNC
#endif // __cplusplus < 201402

///
#define BX_STATIC_ASSERT(_condition, ...) static_assert(_condition, "" __VA_ARGS__)

///
#define BX_ALIGN_DECL_16(_decl) BX_ALIGN_DECL(16, _decl)
#define BX_ALIGN_DECL_256(_decl) BX_ALIGN_DECL(256, _decl)
#define BX_ALIGN_DECL_CACHE_LINE(_decl) BX_ALIGN_DECL(BX_CACHE_LINE_SIZE, _decl)

///
#define BX_MACRO_BLOCK_BEGIN for(;;) {
#define BX_MACRO_BLOCK_END break; }
#define BX_NOOP(...) BX_MACRO_BLOCK_BEGIN BX_MACRO_BLOCK_END

///
#define BX_UNUSED_1(_a1)                                              \
	BX_MACRO_BLOCK_BEGIN                                              \
		BX_PRAGMA_DIAGNOSTIC_PUSH();                                  \
		/*BX_PRAGMA_DIAGNOSTIC_IGNORED_CLANG_GCC("-Wuseless-cast");*/ \
		(void)(true ? (void)0 : ( (void)(_a1) ) );                    \
		BX_PRAGMA_DIAGNOSTIC_POP();                                   \
	BX_MACRO_BLOCK_END

#define BX_UNUSED_2(_a1, _a2) BX_UNUSED_1(_a1); BX_UNUSED_1(_a2)
#define BX_UNUSED_3(_a1, _a2, _a3) BX_UNUSED_2(_a1, _a2); BX_UNUSED_1(_a3)
#define BX_UNUSED_4(_a1, _a2, _a3, _a4) BX_UNUSED_3(_a1, _a2, _a3); BX_UNUSED_1(_a4)
#define BX_UNUSED_5(_a1, _a2, _a3, _a4, _a5) BX_UNUSED_4(_a1, _a2, _a3, _a4); BX_UNUSED_1(_a5)
#define BX_UNUSED_6(_a1, _a2, _a3, _a4, _a5, _a6) BX_UNUSED_5(_a1, _a2, _a3, _a4, _a5); BX_UNUSED_1(_a6)
#define BX_UNUSED_7(_a1, _a2, _a3, _a4, _a5, _a6, _a7) BX_UNUSED_6(_a1, _a2, _a3, _a4, _a5, _a6); BX_UNUSED_1(_a7)
#define BX_UNUSED_8(_a1, _a2, _a3, _a4, _a5, _a6, _a7, _a8) BX_UNUSED_7(_a1, _a2, _a3, _a4, _a5, _a6, _a7); BX_UNUSED_1(_a8)
#define BX_UNUSED_9(_a1, _a2, _a3, _a4, _a5, _a6, _a7, _a8, _a9) BX_UNUSED_8(_a1, _a2, _a3, _a4, _a5, _a6, _a7, _a8); BX_UNUSED_1(_a9)
#define BX_UNUSED_10(_a1, _a2, _a3, _a4, _a5, _a6, _a7, _a8, _a9, _a10) BX_UNUSED_9(_a1, _a2, _a3, _a4, _a5, _a6, _a7, _a8, _a9); BX_UNUSED_1(_a10)
#define BX_UNUSED_11(_a1, _a2, _a3, _a4, _a5, _a6, _a7, _a8, _a9, _a10, _a11) BX_UNUSED_10(_a1, _a2, _a3, _a4, _a5, _a6, _a7, _a8, _a9, _a10); BX_UNUSED_1(_a11)
#define BX_UNUSED_12(_a1, _a2, _a3, _a4, _a5, _a6, _a7, _a8, _a9, _a10, _a11, _a12) BX_UNUSED_11(_a1, _a2, _a3, _a4, _a5, _a6, _a7, _a8, _a9, _a10, _a11); BX_UNUSED_1(_a12)

#if BX_COMPILER_MSVC
// Workaround MSVS bug...
#	define BX_UNUSED(...) BX_MACRO_DISPATCHER(BX_UNUSED_, __VA_ARGS__) BX_VA_ARGS_PASS(__VA_ARGS__)
#else
#	define BX_UNUSED(...) BX_MACRO_DISPATCHER(BX_UNUSED_, __VA_ARGS__)(__VA_ARGS__)
#endif // BX_COMPILER_MSVC

///
#if BX_COMPILER_CLANG
#	define BX_PRAGMA_DIAGNOSTIC_PUSH_CLANG_()     _Pragma("clang diagnostic push")
#	define BX_PRAGMA_DIAGNOSTIC_POP_CLANG_()      _Pragma("clang diagnostic pop")
#	define BX_PRAGMA_DIAGNOSTIC_IGNORED_CLANG(_x) _Pragma(BX_STRINGIZE(clang diagnostic ignored _x) )
#else
#	define BX_PRAGMA_DIAGNOSTIC_PUSH_CLANG_()
#	define BX_PRAGMA_DIAGNOSTIC_POP_CLANG_()
#	define BX_PRAGMA_DIAGNOSTIC_IGNORED_CLANG(_x)
#endif // BX_COMPILER_CLANG

#if BX_COMPILER_GCC && BX_COMPILER_GCC >= 40600
#	define BX_PRAGMA_DIAGNOSTIC_PUSH_GCC_()       _Pragma("GCC diagnostic push")
#	define BX_PRAGMA_DIAGNOSTIC_POP_GCC_()        _Pragma("GCC diagnostic pop")
#	define BX_PRAGMA_DIAGNOSTIC_IGNORED_GCC(_x)   _Pragma(BX_STRINGIZE(GCC diagnostic ignored _x) )
#else
#	define BX_PRAGMA_DIAGNOSTIC_PUSH_GCC_()
#	define BX_PRAGMA_DIAGNOSTIC_POP_GCC_()
#	define BX_PRAGMA_DIAGNOSTIC_IGNORED_GCC(_x)
#endif // BX_COMPILER_GCC

#if BX_COMPILER_MSVC
#	define BX_PRAGMA_DIAGNOSTIC_PUSH_MSVC_()     __pragma(warning(push) )
#	define BX_PRAGMA_DIAGNOSTIC_POP_MSVC_()      __pragma(warning(pop) )
#	define BX_PRAGMA_DIAGNOSTIC_IGNORED_MSVC(_x) __pragma(warning(disable:_x) )
#else
#	define BX_PRAGMA_DIAGNOSTIC_PUSH_MSVC_()
#	define BX_PRAGMA_DIAGNOSTIC_POP_MSVC_()
#	define BX_PRAGMA_DIAGNOSTIC_IGNORED_MSVC(_x)
#endif // BX_COMPILER_CLANG

#if BX_COMPILER_CLANG
#	define BX_PRAGMA_DIAGNOSTIC_PUSH              BX_PRAGMA_DIAGNOSTIC_PUSH_CLANG_
#	define BX_PRAGMA_DIAGNOSTIC_POP               BX_PRAGMA_DIAGNOSTIC_POP_CLANG_
#	define BX_PRAGMA_DIAGNOSTIC_IGNORED_CLANG_GCC BX_PRAGMA_DIAGNOSTIC_IGNORED_CLANG
#elif BX_COMPILER_GCC
#	define BX_PRAGMA_DIAGNOSTIC_PUSH              BX_PRAGMA_DIAGNOSTIC_PUSH_GCC_
#	define BX_PRAGMA_DIAGNOSTIC_POP               BX_PRAGMA_DIAGNOSTIC_POP_GCC_
#	define BX_PRAGMA_DIAGNOSTIC_IGNORED_CLANG_GCC BX_PRAGMA_DIAGNOSTIC_IGNORED_GCC
#elif BX_COMPILER_MSVC
#	define BX_PRAGMA_DIAGNOSTIC_PUSH              BX_PRAGMA_DIAGNOSTIC_PUSH_MSVC_
#	define BX_PRAGMA_DIAGNOSTIC_POP               BX_PRAGMA_DIAGNOSTIC_POP_MSVC_
#	define BX_PRAGMA_DIAGNOSTIC_IGNORED_CLANG_GCC(_x)
#endif // BX_COMPILER_

///
#define BX_CLASS_NO_DEFAULT_CTOR(_class) \
	private: _class()

#define BX_CLASS_NO_COPY(_class) \
	private: _class(const _class& _rhs)

#define BX_CLASS_NO_ASSIGNMENT(_class) \
	private: _class& operator=(const _class& _rhs)

#define BX_CLASS_ALLOCATOR(_class)              \
	public: void* operator new(size_t _size);   \
	public: void  operator delete(void* _ptr);  \
	public: void* operator new[](size_t _size); \
	public: void  operator delete[](void* _ptr)

#define BX_CLASS_1(_class, _a1) BX_CONCATENATE(BX_CLASS_, _a1)(_class)
#define BX_CLASS_2(_class, _a1, _a2) BX_CLASS_1(_class, _a1); BX_CLASS_1(_class, _a2)
#define BX_CLASS_3(_class, _a1, _a2, _a3) BX_CLASS_2(_class, _a1, _a2); BX_CLASS_1(_class, _a3)
#define BX_CLASS_4(_class, _a1, _a2, _a3, _a4) BX_CLASS_3(_class, _a1, _a2, _a3); BX_CLASS_1(_class, _a4)

#if BX_COMPILER_MSVC
#	define BX_CLASS(_class, ...) BX_MACRO_DISPATCHER(BX_CLASS_, __VA_ARGS__) BX_VA_ARGS_PASS(_class, __VA_ARGS__)
#else
#	define BX_CLASS(_class, ...) BX_MACRO_DISPATCHER(BX_CLASS_, __VA_ARGS__)(_class, __VA_ARGS__)
#endif // BX_COMPILER_MSVC

#ifndef BX_ASSERT
#	define BX_ASSERT(_condition, ...) BX_NOOP()
#endif // BX_ASSERT

#ifndef BX_TRACE
#	define BX_TRACE(...) BX_NOOP()
#endif // BX_TRACE

#ifndef BX_WARN
#	define BX_WARN(_condition, ...) BX_NOOP()
#endif // BX_ASSERT

// static_assert sometimes causes unused-local-typedef...
BX_PRAGMA_DIAGNOSTIC_IGNORED_CLANG("-Wunused-local-typedef")

#endif // BX_MACROS_H_HEADER_GUARD
