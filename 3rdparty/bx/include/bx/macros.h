/*
 * Copyright 2010-2024 Branimir Karadzic. All rights reserved.
 * License: https://github.com/bkaradzic/bx/blob/master/LICENSE
 */

#ifndef BX_H_HEADER_GUARD
#	error "Do not include macros.h directly #include <bx/bx.h> instead."
#endif // BX_H_HEADER_GUARD

#ifndef BX_MACROS_H_HEADER_GUARD
#define BX_MACROS_H_HEADER_GUARD

#define BX_VA_ARGS_COUNT(...) BX_VA_ARGS_COUNT_(__VA_ARGS__, 16, 15, 14, 13, 12, 11, 10, 9, 8, 7, 6, 5, 4, 3, 2, 1, 0)
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
#	define BX_ASSUME(_condition) BX_MACRO_BLOCK_BEGIN if (!(_condition) ) { __builtin_unreachable(); } BX_MACRO_BLOCK_END
#	define BX_ALIGN_DECL(_align, _decl) _decl __attribute__( (aligned(_align) ) )
#	define BX_FORCE_INLINE inline __attribute__( (__always_inline__) )
#	define BX_FUNCTION __PRETTY_FUNCTION__
#	define BX_LIKELY(_x)   __builtin_expect(!!(_x), 1)
#	define BX_UNLIKELY(_x) __builtin_expect(!!(_x), 0)
#	define BX_NO_INLINE   __attribute__( (noinline) )
#	define BX_CONST_FUNC  __attribute__( (pure) )
#	define BX_UNREACHABLE __builtin_unreachable()
#	define BX_NO_VTABLE
#	define BX_PRINTF_ARGS(_format, _args) __attribute__( (format(__printf__, _format, _args) ) )
#	define BX_THREAD_LOCAL __thread
#	define BX_ATTRIBUTE(_x) __attribute__( (_x) )

#	if BX_CRT_MSVC
#		define __stdcall
#	endif // BX_CRT_MSVC

#elif BX_COMPILER_MSVC
#	define BX_ASSUME(_condition) __assume(_condition)
#	define BX_ALIGN_DECL(_align, _decl) __declspec(align(_align) ) _decl
#	define BX_FORCE_INLINE __forceinline
#	define BX_FUNCTION __FUNCTION__
#	define BX_LIKELY(_x)   (_x)
#	define BX_UNLIKELY(_x) (_x)
#	define BX_NO_INLINE __declspec(noinline)
#	define BX_CONST_FUNC  __declspec(noalias)
#	define BX_UNREACHABLE __assume(false)
#	define BX_NO_VTABLE __declspec(novtable)
#	define BX_PRINTF_ARGS(_format, _args)
#	define BX_THREAD_LOCAL __declspec(thread)
#	define BX_ATTRIBUTE(_x)
#else
#	error "Unknown BX_COMPILER_?"
#endif

/// The return value of the function is solely a function of the arguments.
///
#define BX_CONSTEXPR_FUNC constexpr BX_CONST_FUNC

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

#define BX_UNUSED(...) BX_MACRO_DISPATCHER(BX_UNUSED_, __VA_ARGS__)(__VA_ARGS__)

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

#if BX_COMPILER_GCC
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

/// No default constructor.
#define BX_CLASS_NO_DEFAULT_CTOR(_class) \
	_class() = delete

/// No copy constructor.
#define BX_CLASS_NO_COPY_CTOR(_class) \
	_class(const _class& _rhs) = delete

/// No copy assignment operator.
#define BX_CLASS_NO_COPY_ASSIGNMENT(_class) \
	_class& operator=(const _class& _rhs) = delete

/// No copy construcor, and copy assignment operator.
#define BX_CLASS_NO_COPY(_class)   \
	BX_CLASS_NO_COPY_CTOR(_class); \
	BX_CLASS_NO_COPY_ASSIGNMENT(_class)

///
#define BX_CLASS_ALLOCATOR(_class)              \
	public: void* operator new(size_t _size);   \
	public: void  operator delete(void* _ptr);  \
	public: void* operator new[](size_t _size); \
	public: void  operator delete[](void* _ptr)

#define BX_CLASS_1(_class, _a1) BX_CONCATENATE(BX_CLASS_, _a1)(_class)
#define BX_CLASS_2(_class, _a1, _a2) BX_CLASS_1(_class, _a1); BX_CLASS_1(_class, _a2)
#define BX_CLASS_3(_class, _a1, _a2, _a3) BX_CLASS_2(_class, _a1, _a2); BX_CLASS_1(_class, _a3)
#define BX_CLASS_4(_class, _a1, _a2, _a3, _a4) BX_CLASS_3(_class, _a1, _a2, _a3); BX_CLASS_1(_class, _a4)
#define BX_CLASS(_class, ...) BX_MACRO_DISPATCHER(BX_CLASS_, __VA_ARGS__)(_class, __VA_ARGS__)

#ifndef BX_ASSERT
#	if BX_CONFIG_DEBUG
#		define BX_ASSERT _BX_ASSERT
#	else
#		define BX_ASSERT(...) BX_NOOP()
#	endif // BX_CONFIG_DEBUG
#endif // BX_ASSERT

#ifndef BX_ASSERT_LOC
#	if BX_CONFIG_DEBUG
#		define BX_ASSERT_LOC _BX_ASSERT_LOC
#	else
#		define BX_ASSERT_LOC(_location, ...) BX_MACRO_BLOCK_BEGIN BX_UNUSED(_location) BX_MACRO_BLOCK_END
#	endif // BX_CONFIG_DEBUG
#endif // BX_ASSERT_LOC

#ifndef BX_TRACE
#	if BX_CONFIG_DEBUG
#		define BX_TRACE _BX_TRACE
#	else
#		define BX_TRACE(...) BX_NOOP()
#	endif // BX_CONFIG_DEBUG
#endif // BX_TRACE

#ifndef BX_TRACE_LOC
#	if BX_CONFIG_DEBUG
#		define BX_TRACE_LOC _BX_TRACE_LOC
#	else
#		define BX_TRACE_LOC(_location, ...) BX_MACRO_BLOCK_BEGIN BX_UNUSED(_location) BX_MACRO_BLOCK_END
#	endif // BX_CONFIG_DEBUG
#endif // BX_TRACE_LOC

#ifndef BX_WARN
#	if BX_CONFIG_DEBUG
#		define BX_WARN _BX_WARN
#	else
#		define BX_WARN(...) BX_NOOP()
#	endif // BX_CONFIG_DEBUG
#endif // BX_ASSERT

#ifndef BX_WARN_LOC
#	if BX_CONFIG_DEBUG
#		define BX_WARN_LOC _BX_WARN_LOC
#	else
#		define BX_WARN_LOC(_location, ...) BX_MACRO_BLOCK_BEGIN BX_UNUSED(_location) BX_MACRO_BLOCK_END
#	endif // BX_CONFIG_DEBUG
#endif // BX_WARN_LOC

#define _BX_TRACE(_format, ...)                                                                    \
	BX_MACRO_BLOCK_BEGIN                                                                           \
		bx::debugPrintf(__FILE__ "(" BX_STRINGIZE(__LINE__) "): BX " _format "\n", ##__VA_ARGS__); \
	BX_MACRO_BLOCK_END

#define _BX_TRACE_LOC(_location, _format, ...)                                                          \
	BX_MACRO_BLOCK_BEGIN                                                                                \
		bx::debugPrintf("%s(%d): BX " _format "\n", _location.filePath, _location.line, ##__VA_ARGS__); \
	BX_MACRO_BLOCK_END

#define _BX_ASSERT(_condition, _format, ...)                                                                   \
	BX_MACRO_BLOCK_BEGIN                                                                                       \
		if (!BX_IGNORE_C4127(_condition)                                                                       \
		&&  bx::assertFunction(bx::Location::current(), "ASSERT %s -> " _format, #_condition, ##__VA_ARGS__) ) \
		{                                                                                                      \
			bx::debugBreak();                                                                                  \
		}                                                                                                      \
	BX_MACRO_BLOCK_END

#define _BX_ASSERT_LOC(_location, _condition, _format, ...)                                       \
	BX_MACRO_BLOCK_BEGIN                                                                          \
		if  (!BX_IGNORE_C4127(_condition)                                                         \
		&&   bx::assertFunction(_location, "ASSERT %s -> " _format, #_condition, ##__VA_ARGS__) ) \
		{                                                                                         \
			bx::debugBreak();                                                                     \
		}                                                                                         \
	BX_MACRO_BLOCK_END

#define _BX_WARN(_condition, _format, ...)            \
	BX_MACRO_BLOCK_BEGIN                              \
		if (!BX_IGNORE_C4127(_condition) )            \
		{                                             \
			BX_TRACE("WARN " _format, ##__VA_ARGS__); \
		}                                             \
	BX_MACRO_BLOCK_END

#define _BX_WARN_LOC(_location, _condition, _format, ...)             \
	BX_MACRO_BLOCK_BEGIN                                              \
		if (!BX_IGNORE_C4127(_condition) )                            \
		{                                                             \
			_BX_TRACE_LOC(_location, "WARN " _format, ##__VA_ARGS__); \
		}                                                             \
	BX_MACRO_BLOCK_END

// static_assert sometimes causes unused-local-typedef...
BX_PRAGMA_DIAGNOSTIC_IGNORED_CLANG("-Wunused-local-typedef")

#endif // BX_MACROS_H_HEADER_GUARD
