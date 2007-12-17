/***************************************************************************

    osdcomm.h

    Common definitions shared by the OSD layer. This includes the most
    fundamental integral types as well as compiler-specific tweaks.

    Copyright (c) 1996-2007, Nicola Salmoria and the MAME Team.
    Visit http://mamedev.org for licensing and usage restrictions.

***************************************************************************/

#pragma once

#ifndef __OSDCOMM_H__
#define __OSDCOMM_H__

#include <stdio.h>
#include <string.h>
#include <stdlib.h>



/***************************************************************************
    COMPILER-SPECIFIC NASTINESS
***************************************************************************/

/* The Win32 port requires this constant for variable arg routines. */
#ifndef CLIB_DECL
#define CLIB_DECL
#endif


/* Some optimizations/warnings cleanups for GCC */
#if defined(__GNUC__) && (__GNUC__ >= 3)
#define ATTR_UNUSED				__attribute__((__unused__))
#define ATTR_NORETURN			__attribute__((noreturn))
#define ATTR_PRINTF(x,y)		__attribute__((format(printf, x, y)))
#define ATTR_MALLOC				__attribute__((malloc))
#define ATTR_PURE				__attribute__((pure))
#define ATTR_CONST				__attribute__((const))
#define UNEXPECTED(exp)			__builtin_expect((exp), 0)
#define TYPES_COMPATIBLE(a,b)	__builtin_types_compatible_p(a, b)
#define RESTRICT				__restrict__
#else
#define ATTR_UNUSED
#define ATTR_NORETURN
#define ATTR_PRINTF(x,y)
#define ATTR_MALLOC
#define ATTR_PURE
#define ATTR_CONST
#define UNEXPECTED(exp)			(exp)
#define TYPES_COMPATIBLE(a,b)	1
#define RESTRICT
#endif


/* And some MSVC optimizations/warnings */
#if defined(_MSC_VER) && (_MSC_VER >= 1200)
#define DECL_NORETURN			__declspec(noreturn)
#else
#define DECL_NORETURN
#endif



/***************************************************************************
    FUNDAMENTAL TYPES
***************************************************************************/

/* These types work on most modern compilers; however, OSD code can
   define their own by setting OSD_TYPES_DEFINED */

#ifndef OSD_TYPES_DEFINED

/* 8-bit values */
typedef unsigned char						UINT8;
typedef signed char 						INT8;

/* 16-bit values */
typedef unsigned short						UINT16;
typedef signed short						INT16;

/* 32-bit values */
#ifndef _WINDOWS_H
typedef unsigned int						UINT32;
typedef signed int							INT32;
#endif

/* 64-bit values */
#ifndef _WINDOWS_H
#ifdef _MSC_VER
typedef signed __int64						INT64;
typedef unsigned __int64					UINT64;
#else
__extension__ typedef unsigned long long	UINT64;
__extension__ typedef signed long long		INT64;
#endif
#endif

#endif



/***************************************************************************
    FUNDAMENTAL CONSTANTS
***************************************************************************/

/* Ensure that TRUE/FALSE are defined */
#ifndef TRUE
#define TRUE    			1
#endif

#ifndef FALSE
#define FALSE  				0
#endif



/***************************************************************************
    FUNDAMENTAL MACROS
***************************************************************************/

/* Standard MIN/MAX macros */
#ifndef MIN
#define MIN(x,y)			((x) < (y) ? (x) : (y))
#endif
#ifndef MAX
#define MAX(x,y)			((x) > (y) ? (x) : (y))
#endif


/* Highly useful macro for compile-time knowledge of an array size */
#define ARRAY_LENGTH(x)		(sizeof(x) / sizeof(x[0]))


/* Macros for normalizing data into big or little endian formats */
#define FLIPENDIAN_INT16(x)	(((((UINT16) (x)) >> 8) | ((x) << 8)) & 0xffff)
#define FLIPENDIAN_INT32(x)	((((UINT32) (x)) << 24) | (((UINT32) (x)) >> 24) | \
	(( ((UINT32) (x)) & 0x0000ff00) << 8) | (( ((UINT32) (x)) & 0x00ff0000) >> 8))
#define FLIPENDIAN_INT64(x)	\
	(												\
		(((((UINT64) (x)) >> 56) & ((UINT64) 0xFF)) <<  0)	|	\
		(((((UINT64) (x)) >> 48) & ((UINT64) 0xFF)) <<  8)	|	\
		(((((UINT64) (x)) >> 40) & ((UINT64) 0xFF)) << 16)	|	\
		(((((UINT64) (x)) >> 32) & ((UINT64) 0xFF)) << 24)	|	\
		(((((UINT64) (x)) >> 24) & ((UINT64) 0xFF)) << 32)	|	\
		(((((UINT64) (x)) >> 16) & ((UINT64) 0xFF)) << 40)	|	\
		(((((UINT64) (x)) >>  8) & ((UINT64) 0xFF)) << 48)	|	\
		(((((UINT64) (x)) >>  0) & ((UINT64) 0xFF)) << 56)		\
	)

#ifdef LSB_FIRST
#define BIG_ENDIANIZE_INT16(x)		(FLIPENDIAN_INT16(x))
#define BIG_ENDIANIZE_INT32(x)		(FLIPENDIAN_INT32(x))
#define BIG_ENDIANIZE_INT64(x)		(FLIPENDIAN_INT64(x))
#define LITTLE_ENDIANIZE_INT16(x)	(x)
#define LITTLE_ENDIANIZE_INT32(x)	(x)
#define LITTLE_ENDIANIZE_INT64(x)	(x)
#else
#define BIG_ENDIANIZE_INT16(x)		(x)
#define BIG_ENDIANIZE_INT32(x)		(x)
#define BIG_ENDIANIZE_INT64(x)		(x)
#define LITTLE_ENDIANIZE_INT16(x)	(FLIPENDIAN_INT16(x))
#define LITTLE_ENDIANIZE_INT32(x)	(FLIPENDIAN_INT32(x))
#define LITTLE_ENDIANIZE_INT64(x)	(FLIPENDIAN_INT64(x))
#endif /* LSB_FIRST */


#endif	/* __OSDCOMM_H__ */
