// license:BSD-3-Clause
// copyright-holders:Aaron Giles
/***************************************************************************

    osdcomm.h

    Common definitions shared by the OSD layer. This includes the most
    fundamental integral types as well as compiler-specific tweaks.

***************************************************************************/

#pragma once

#ifndef __OSDCOMM_H__
#define __OSDCOMM_H__

#include <stdio.h>
#include <string.h>


/***************************************************************************
    COMPILER-SPECIFIC NASTINESS
***************************************************************************/

/* The Win32 port requires this constant for variable arg routines. */
#ifndef CLIB_DECL
#define CLIB_DECL
#endif


/* Some optimizations/warnings cleanups for GCC */
#if defined(__GNUC__) && (__GNUC__ >= 3)
#define ATTR_UNUSED             __attribute__((__unused__))
#define ATTR_NORETURN           __attribute__((noreturn))
#define ATTR_PRINTF(x,y)        __attribute__((format(printf, x, y)))
#define ATTR_MALLOC             __attribute__((malloc))
#define ATTR_PURE               __attribute__((pure))
#define ATTR_CONST              __attribute__((const))
#define ATTR_FORCE_INLINE       __attribute__((always_inline))
#define ATTR_NONNULL(...)       __attribute__((nonnull(__VA_ARGS__)))
#define ATTR_DEPRECATED         __attribute__((deprecated))
/* not supported in GCC prior to 4.4.x */
#if ((__GNUC__ == 4) && (__GNUC_MINOR__ >= 4)) || (__GNUC__ > 4)
#define ATTR_HOT                __attribute__((hot))
#define ATTR_COLD               __attribute__((cold))
#else
#define ATTR_HOT
#define ATTR_COLD
#endif
#define UNEXPECTED(exp)         __builtin_expect(!!(exp), 0)
#define EXPECTED(exp)           __builtin_expect(!!(exp), 1)
#define RESTRICT                __restrict__
#define SETJMP_GNUC_PROTECT()   (void)__builtin_return_address(1)
#else
#define ATTR_UNUSED
#if defined(_MSC_VER) && (_MSC_VER >= 1200)
#define ATTR_NORETURN           __declspec(noreturn)
#else
#define ATTR_NORETURN
#endif
#define ATTR_PRINTF(x,y)
#define ATTR_MALLOC
#define ATTR_PURE
#define ATTR_CONST
#define ATTR_FORCE_INLINE       __forceinline
#define ATTR_NONNULL(...)
#define ATTR_DEPRECATED         __declspec(deprecated)
#define ATTR_HOT
#define ATTR_COLD
#define UNEXPECTED(exp)         (exp)
#define EXPECTED(exp)           (exp)
#define RESTRICT
#define SETJMP_GNUC_PROTECT()   do {} while (0)
#endif



/***************************************************************************
    FUNDAMENTAL TYPES
***************************************************************************/

/* These types work on most modern compilers; however, OSD code can
   define their own by setting OSD_TYPES_DEFINED */

#ifndef OSD_TYPES_DEFINED

/* 8-bit values */
typedef unsigned char                       UINT8;
typedef signed char                         INT8;

/* 16-bit values */
typedef unsigned short                      UINT16;
typedef signed short                        INT16;

/* 32-bit values */
#ifndef _WINDOWS_H
typedef unsigned int                        UINT32;
typedef signed int                          INT32;
#endif

/* 64-bit values */
#ifndef _WINDOWS_H
#ifdef _MSC_VER
typedef signed __int64                      INT64;
typedef unsigned __int64                    UINT64;
#else
__extension__ typedef unsigned long long    UINT64;
__extension__ typedef signed long long      INT64;
#endif
#endif

#endif

/* pointer-sized values */
#ifdef PTR64
typedef UINT64                              FPTR;
#else
typedef UINT32                              FPTR;
#endif



/***************************************************************************
    FUNDAMENTAL CONSTANTS
***************************************************************************/

/* Ensure that TRUE/FALSE are defined */
#ifndef TRUE
#define TRUE                1
#endif

#ifndef FALSE
#define FALSE               0
#endif



/***************************************************************************
    FUNDAMENTAL MACROS
***************************************************************************/

/* Standard MIN/MAX macros */
#ifndef MIN
#define MIN(x,y)            ((x) < (y) ? (x) : (y))
#endif
#ifndef MAX
#define MAX(x,y)            ((x) > (y) ? (x) : (y))
#endif


/* U64 and S64 are used to wrap long integer constants. */
#if defined(__GNUC__) || defined(_MSC_VER)
#define U64(val) val##ULL
#define S64(val) val##LL
#else
#define U64(val) val
#define S64(val) val
#endif


/* Concatenate/extract 32-bit halves of 64-bit values */
#define CONCAT_64(hi,lo)    (((UINT64)(hi) << 32) | (UINT32)(lo))
#define EXTRACT_64HI(val)   ((UINT32)((val) >> 32))
#define EXTRACT_64LO(val)   ((UINT32)(val))


/* MINGW has adopted the MSVC formatting for 64-bit ints as of gcc 4.4 */
#if (defined(__MINGW32__) && (__GNUC__ > 4 || (__GNUC__ == 4 && __GNUC_MINOR__ >= 4))) || defined(_MSC_VER)
#define I64FMT   "I64"
#else
#define I64FMT   "ll"
#endif

#if defined(_MSC_VER) || defined(__MINGW32__)
#ifdef PTR64
#define SIZETFMT   "I64u"
#else
#define SIZETFMT   "u"
#endif
#else
#define SIZETFMT   "zu"
#endif


/* Highly useful macro for compile-time knowledge of an array size */
#define ARRAY_LENGTH(x)     (sizeof(x) / sizeof(x[0]))


/* Macros for normalizing data into big or little endian formats */
#define FLIPENDIAN_INT16(x) (((((UINT16) (x)) >> 8) | ((x) << 8)) & 0xffff)
#define FLIPENDIAN_INT32(x) ((((UINT32) (x)) << 24) | (((UINT32) (x)) >> 24) | \
	(( ((UINT32) (x)) & 0x0000ff00) << 8) | (( ((UINT32) (x)) & 0x00ff0000) >> 8))
#define FLIPENDIAN_INT64(x) \
	(                                               \
		(((((UINT64) (x)) >> 56) & ((UINT64) 0xFF)) <<  0)  |   \
		(((((UINT64) (x)) >> 48) & ((UINT64) 0xFF)) <<  8)  |   \
		(((((UINT64) (x)) >> 40) & ((UINT64) 0xFF)) << 16)  |   \
		(((((UINT64) (x)) >> 32) & ((UINT64) 0xFF)) << 24)  |   \
		(((((UINT64) (x)) >> 24) & ((UINT64) 0xFF)) << 32)  |   \
		(((((UINT64) (x)) >> 16) & ((UINT64) 0xFF)) << 40)  |   \
		(((((UINT64) (x)) >>  8) & ((UINT64) 0xFF)) << 48)  |   \
		(((((UINT64) (x)) >>  0) & ((UINT64) 0xFF)) << 56)      \
	)

#ifdef LSB_FIRST
#define BIG_ENDIANIZE_INT16(x)      (FLIPENDIAN_INT16(x))
#define BIG_ENDIANIZE_INT32(x)      (FLIPENDIAN_INT32(x))
#define BIG_ENDIANIZE_INT64(x)      (FLIPENDIAN_INT64(x))
#define LITTLE_ENDIANIZE_INT16(x)   (x)
#define LITTLE_ENDIANIZE_INT32(x)   (x)
#define LITTLE_ENDIANIZE_INT64(x)   (x)
#else
#define BIG_ENDIANIZE_INT16(x)      (x)
#define BIG_ENDIANIZE_INT32(x)      (x)
#define BIG_ENDIANIZE_INT64(x)      (x)
#define LITTLE_ENDIANIZE_INT16(x)   (FLIPENDIAN_INT16(x))
#define LITTLE_ENDIANIZE_INT32(x)   (FLIPENDIAN_INT32(x))
#define LITTLE_ENDIANIZE_INT64(x)   (FLIPENDIAN_INT64(x))
#endif /* LSB_FIRST */

// compatibility with non-clang compilers
#ifndef __has_feature
	#define __has_feature(x) 0
#endif

#ifdef _MSC_VER
#include <malloc.h>
typedef ptrdiff_t ssize_t;
#if _MSC_VER == 1900 // VS2015
#define __LINE__Var 0
#endif // VS2015
#if _MSC_VER < 1900 // VS2013 or earlier
#define snprintf _snprintf
#define __func__ __FUNCTION__
#if _MSC_VER < 1800 // VS2012 or earlier
#define alloca _alloca
#define round(x) floor((x) + 0.5)
#define strtoll _strtoi64
#define _USE_MATH_DEFINES
#include <math.h>
static __inline double fmin(double x, double y){ return (x < y) ? x : y; }
static __inline double fmax(double x, double y){ return (x > y) ? x : y; }
static __inline double log2(double x) { return log(x) * M_LOG2E; }
#define __func__ __FUNCTION__
#endif // VS2012 or earlier
#else // VS2015
#define _CRT_STDIO_LEGACY_WIDE_SPECIFIERS
#endif
#endif

#ifdef __GNUC__
#ifndef alloca
#define alloca(size)  __builtin_alloca(size)
#endif
#endif

#endif  /* __OSDCOMM_H__ */
