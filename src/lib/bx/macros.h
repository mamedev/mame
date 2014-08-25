/*
 * Copyright 2010-2013 Branimir Karadzic. All rights reserved.
 * License: http://www.opensource.org/licenses/BSD-2-Clause
 */

#ifndef BX_MACROS_H_HEADER_GUARD
#define BX_MACROS_H_HEADER_GUARD

#include "bx.h"

#if BX_COMPILER_MSVC
// Workaround MSVS bug...
#	define BX_VA_ARGS_PASS(...) BX_VA_ARGS_PASS_1_ __VA_ARGS__ BX_VA_ARGS_PASS_2_
#	define BX_VA_ARGS_PASS_1_ (
#	define BX_VA_ARGS_PASS_2_ )
#else
#	define BX_VA_ARGS_PASS(...) (__VA_ARGS__)
#endif // BX_COMPILER_MSVC

#define BX_VA_ARGS_COUNT(...) BX_VA_ARGS_COUNT_ BX_VA_ARGS_PASS(__VA_ARGS__, 16, 15, 14, 13, 12, 11, 10, 9, 8, 7, 6, 5, 4, 3, 2, 1)
#define BX_VA_ARGS_COUNT_(_a1, _a2, _a3, _a4, _a5, _a6, _a7, _a8, _a9, _a10, _a11, _a12, _a13, _a14, _a15, _a16, _last, ...) _last

#define BX_MACRO_DISPATCHER(_func, ...) BX_MACRO_DISPATCHER_1_(_func, BX_VA_ARGS_COUNT(__VA_ARGS__) )
#define BX_MACRO_DISPATCHER_1_(_func, _argCount) BX_MACRO_DISPATCHER_2_(_func, _argCount)
#define BX_MACRO_DISPATCHER_2_(_func, _argCount) BX_CONCATENATE(_func, _argCount)

#define BX_MAKEFOURCC(_a, _b, _c, _d) ( ( (uint32_t)(_a) | ( (uint32_t)(_b) << 8) | ( (uint32_t)(_c) << 16) | ( (uint32_t)(_d) << 24) ) )

#define BX_STRINGIZE(_x) BX_STRINGIZE_(_x)
#define BX_STRINGIZE_(_x) #_x

#define BX_CONCATENATE(_x, _y) BX_CONCATENATE_(_x, _y)
#define BX_CONCATENATE_(_x, _y) _x ## _y

#define BX_FILE_LINE_LITERAL "" __FILE__ "(" BX_STRINGIZE(__LINE__) "): "

#define BX_ALIGN_MASK(_value, _mask) ( ( (_value)+(_mask) ) & ( (~0)&(~(_mask) ) ) )
#define BX_ALIGN_16(_value) BX_ALIGN_MASK(_value, 0xf)
#define BX_ALIGN_256(_value) BX_ALIGN_MASK(_value, 0xff)
#define BX_ALIGN_4096(_value) BX_ALIGN_MASK(_value, 0xfff)

#define BX_ALIGNOF(_type) __alignof(_type)

#if BX_COMPILER_GCC || BX_COMPILER_CLANG
#	define BX_ALIGN_STRUCT(_align, _struct) _struct __attribute__( (aligned(_align) ) )
#	define BX_ALLOW_UNUSED __attribute__( (unused) )
#	define BX_FORCE_INLINE __extension__ static __inline __attribute__( (__always_inline__) )
#	define BX_FUNCTION __PRETTY_FUNCTION__
#	define BX_NO_INLINE __attribute__( (noinline) )
#	define BX_NO_RETURN __attribute__( (noreturn) )
#	define BX_NO_VTABLE
#	define BX_OVERRIDE
#	define BX_PRINTF_ARGS(_format, _args) __attribute__ ( (format(__printf__, _format, _args) ) )
#	if BX_COMPILER_CLANG || BX_PLATFORM_OSX || BX_PLATFORM_IOS
#		define BX_THREAD /* not supported right now */
#	else
#		define BX_THREAD __thread
#	endif // BX_COMPILER_CLANG
#	define BX_ATTRIBUTE(_x) __attribute__( (_x) )
#elif BX_COMPILER_MSVC
#	define BX_ALIGN_STRUCT(_align, _struct) __declspec(align(_align) ) _struct
#	define BX_ALLOW_UNUSED
#	define BX_FORCE_INLINE __forceinline
#	define BX_FUNCTION __FUNCTION__
#	define BX_NO_INLINE __declspec(noinline)
#	define BX_NO_RETURN
#	define BX_NO_VTABLE __declspec(novtable)
#	define BX_OVERRIDE override
#	define BX_PRINTF_ARGS(_format, _args)
#	define BX_THREAD __declspec(thread)
#	define BX_ATTRIBUTE(_x)
#else
#	error "Unknown BX_COMPILER_?"
#endif

// #define BX_STATIC_ASSERT(_condition, ...) static_assert(_condition, "" __VA_ARGS__)
#define BX_STATIC_ASSERT(_condition, ...) typedef char BX_CONCATENATE(BX_STATIC_ASSERT_, __LINE__)[1][(_condition)] BX_ATTRIBUTE(unused)

#define BX_CACHE_LINE_ALIGN_MARKER() BX_ALIGN_STRUCT(BX_CACHE_LINE_SIZE, struct) BX_CONCATENATE(bx_cache_line_marker_compiler_stfu, __COUNTER__) {}
#define BX_CACHE_LINE_ALIGN(_def) BX_CACHE_LINE_ALIGN_MARKER(); _def; BX_CACHE_LINE_ALIGN_MARKER()

#define BX_ALIGN_STRUCT_16(_struct) BX_ALIGN_STRUCT(16, _struct)
#define BX_ALIGN_STRUCT_256(_struct) BX_ALIGN_STRUCT(256, _struct)

#define BX_MACRO_BLOCK_BEGIN for(;;) {
#define BX_MACRO_BLOCK_END break; }
#define BX_NOOP(...) BX_MACRO_BLOCK_BEGIN BX_MACRO_BLOCK_END

#define BX_UNUSED_1(_a1) BX_MACRO_BLOCK_BEGIN (void)(true ? (void)0 : ( (void)(_a1) ) ); BX_MACRO_BLOCK_END
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

#define BX_TYPE_IS_POD(_type) (!__is_class(_type) || __is_pod(_type) )

#define BX_CLASS_NO_DEFAULT_CTOR(_class) \
			private: _class()

#define BX_CLASS_NO_COPY(_class) \
			private: _class(const _class& _rhs)

#define BX_CLASS_NO_ASSIGNMENT(_class) \
			private: _class& operator=(const _class& _rhs)

#define BX_CLASS_ALLOCATOR(_class) \
			public: void* operator new(size_t _size); \
			public: void  operator delete(void* _ptr); \
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

#ifndef BX_CHECK
#	define BX_CHECK(_condition, ...) BX_NOOP()
#endif // BX_CHECK

#ifndef BX_TRACE
#	define BX_TRACE(...) BX_NOOP()
#endif // BX_TRACE

#ifndef BX_WARN
#	define BX_WARN(_condition, ...) BX_NOOP()
#endif // BX_CHECK

#endif // BX_MACROS_H_HEADER_GUARD
