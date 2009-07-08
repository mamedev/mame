/***************************************************************************

    mamecore.h

    General core utilities and macros used throughout MAME.

    Copyright Nicola Salmoria and the MAME Team.
    Visit http://mamedev.org for licensing and usage restrictions.

***************************************************************************/

#pragma once

#ifndef __MAMECORE_H__
#define __MAMECORE_H__

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "osdcomm.h"
#include "bitmap.h"
#include "coreutil.h"
#include "corestr.h"


/***************************************************************************
    COMPILER-SPECIFIC NASTINESS
***************************************************************************/

/* Suppress warnings about redefining the macro 'PPC' on LinuxPPC. */
#ifdef PPC
#undef PPC
#endif

/* Suppress warnings about redefining the macro 'ARM' on ARM. */
#ifdef ARM
#undef ARM
#endif



/***************************************************************************
    COMMON TYPES
***************************************************************************/

/* genf is a type that can be used for function pointer casting in a way
   that doesn't confuse some compilers */
typedef void genf(void);


/* FPTR is a type that can be used to cast a pointer to a scalar */
/* 64-bit platforms should define PTR64 */
#ifdef PTR64
typedef UINT64 FPTR;
#else
typedef UINT32 FPTR;
#endif


/* These are forward struct declarations that are used to break
   circular dependencies in the code */
typedef struct _running_machine running_machine;
typedef struct _game_driver game_driver;
typedef struct _machine_config machine_config;
typedef struct _gfx_element gfx_element;
typedef struct _mame_file mame_file;
typedef struct _device_config device_config;


/* pen_t is used to represent pixel values in bitmaps */
typedef UINT32 pen_t;

/* stream_sample_t is used to represent a single sample in a sound stream */
typedef INT32 stream_sample_t;



/***************************************************************************
 * Union of UINT8, UINT16 and UINT32 in native endianess of the target
 * This is used to access bytes and words in a machine independent manner.
 * The upper bytes h2 and h3 normally contain zero (16 bit CPU cores)
 * thus PAIR.d can be used to pass arguments to the memory system
 * which expects 'int' really.
***************************************************************************/
typedef union
{
#ifdef LSB_FIRST
	struct { UINT8 l,h,h2,h3; } b;
	struct { UINT16 l,h; } w;
	struct { INT8 l,h,h2,h3; } sb;
	struct { INT16 l,h; } sw;
#else
	struct { UINT8 h3,h2,h,l; } b;
	struct { INT8 h3,h2,h,l; } sb;
	struct { UINT16 h,l; } w;
	struct { INT16 h,l; } sw;
#endif
	UINT32 d;
	INT32 sd;
} PAIR;


/***************************************************************************
 * Union of UINT8, UINT16, UINT32, and UINT64 in native endianess of
 * the target.  This is used to access bytes and words in a machine
 * independent manner.
***************************************************************************/
typedef union
{
#ifdef LSB_FIRST
	struct { UINT8 l,h,h2,h3,h4,h5,h6,h7; } b;
	struct { UINT16 l,h,h2,h3; } w;
	struct { UINT32 l,h; } d;
	struct { INT8 l,h,h2,h3,h4,h5,h6,h7; } sb;
	struct { INT16 l,h,h2,h3; } sw;
	struct { INT32 l,h; } sd;
#else
	struct { UINT8 h7,h6,h5,h4,h3,h2,h,l; } b;
	struct { UINT16 h3,h2,h,l; } w;
	struct { UINT32 h,l; } d;
	struct { INT8 h7,h6,h5,h4,h3,h2,h,l; } sb;
	struct { INT16 h3,h2,h,l; } sw;
	struct { INT32 h,l; } sd;
#endif
	UINT64 q;
	INT64 sq;
} PAIR64;



/***************************************************************************
    COMMON CONSTANTS
***************************************************************************/

/* this is not part of the C/C++ standards and is not present on */
/* strict ANSI compilers or when compiling under GCC with -ansi */
#ifndef M_PI
#define M_PI    3.14159265358979323846
#endif


/* Endianness constants */
enum
{
	ENDIANNESS_LITTLE = 0,
	ENDIANNESS_BIG
};


/* Native endianness */
#ifdef LSB_FIRST
#define ENDIANNESS_NATIVE				ENDIANNESS_LITTLE
#else
#define ENDIANNESS_NATIVE				ENDIANNESS_BIG
#endif


/* orientation of bitmaps */
#define	ORIENTATION_FLIP_X				0x0001	/* mirror everything in the X direction */
#define	ORIENTATION_FLIP_Y				0x0002	/* mirror everything in the Y direction */
#define ORIENTATION_SWAP_XY				0x0004	/* mirror along the top-left/bottom-right diagonal */

#define	ROT0							0
#define	ROT90							(ORIENTATION_SWAP_XY | ORIENTATION_FLIP_X)	/* rotate clockwise 90 degrees */
#define	ROT180							(ORIENTATION_FLIP_X | ORIENTATION_FLIP_Y)	/* rotate 180 degrees */
#define	ROT270							(ORIENTATION_SWAP_XY | ORIENTATION_FLIP_Y)	/* rotate counter-clockwise 90 degrees */


/* giant global string buffer */
#define GIANT_STRING_BUFFER_SIZE		65536



/***************************************************************************
    COMMON MACROS
***************************************************************************/

/* Macro for declaring enumerator operators for easier porting */
#ifdef __cplusplus
#define DECLARE_ENUM_OPERATORS(type) \
inline void operator++(type &value) { value = (type)((int)value + 1); } \
inline void operator++(type &value, int) { value = (type)((int)value + 1); } \
inline void operator--(type &value) { value = (type)((int)value - 1); } \
inline void operator--(type &value, int) { value = (type)((int)value - 1); }
#else
#define DECLARE_ENUM_OPERATORS(type)
#endif


/* Standard MAME assertion macros */
#undef assert
#undef assert_always

#ifdef MAME_DEBUG
#define assert(x)	do { if (!(x)) fatalerror("assert: %s:%d: %s", __FILE__, __LINE__, #x); } while (0)
#define assert_always(x, msg) do { if (!(x)) fatalerror("Fatal error: %s\nCaused by assert: %s:%d: %s", msg, __FILE__, __LINE__, #x); } while (0)
#else
#define assert(x)	do { } while (0)
#define assert_always(x, msg) do { if (!(x)) fatalerror("Fatal error: %s (%s:%d)", msg, __FILE__, __LINE__); } while (0)
#endif


/* map mame_* helpers to core_* helpers */
#define mame_stricmp		core_stricmp
#define mame_strnicmp		core_strnicmp
#define mame_strdup			core_strdup
#define mame_strwildcmp		core_strwildcmp


/* prevent the use of rand() -- use mame_rand() instead */
#define rand


/* macros to convert radians to degrees and degrees to radians */
#define RADIAN_TO_DEGREE(x)   ((180.0 / M_PI) * (x))
#define DEGREE_TO_RADIAN(x)   ((M_PI / 180.0) * (x))


/* endian-based value: first value is if 'endian' is little-endian, second is if 'endian' is big-endian */
#define ENDIAN_VALUE_LE_BE(endian,leval,beval)	(((endian) == ENDIANNESS_LITTLE) ? (leval) : (beval))

/* endian-based value: first value is if native endianness is little-endian, second is if native is big-endian */
#define NATIVE_ENDIAN_VALUE_LE_BE(leval,beval)	((ENDIANNESS_NATIVE == ENDIANNESS_LITTLE) ? (leval) : (beval))

/* endian-based value: first value is if 'endian' matches native, second is if 'endian' doesn't match native */
#define ENDIAN_VALUE_NE_NNE(endian,leval,beval)	(((endian) == ENDIANNESS_NATIVE) ? (neval) : (nneval))


/* Useful macros to deal with bit shuffling encryptions */
#define BIT(x,n) (((x)>>(n))&1)

#define BITSWAP8(val,B7,B6,B5,B4,B3,B2,B1,B0) \
		((BIT(val,B7) << 7) | \
		 (BIT(val,B6) << 6) | \
		 (BIT(val,B5) << 5) | \
		 (BIT(val,B4) << 4) | \
		 (BIT(val,B3) << 3) | \
		 (BIT(val,B2) << 2) | \
		 (BIT(val,B1) << 1) | \
		 (BIT(val,B0) << 0))

#define BITSWAP16(val,B15,B14,B13,B12,B11,B10,B9,B8,B7,B6,B5,B4,B3,B2,B1,B0) \
		((BIT(val,B15) << 15) | \
		 (BIT(val,B14) << 14) | \
		 (BIT(val,B13) << 13) | \
		 (BIT(val,B12) << 12) | \
		 (BIT(val,B11) << 11) | \
		 (BIT(val,B10) << 10) | \
		 (BIT(val, B9) <<  9) | \
		 (BIT(val, B8) <<  8) | \
		 (BIT(val, B7) <<  7) | \
		 (BIT(val, B6) <<  6) | \
		 (BIT(val, B5) <<  5) | \
		 (BIT(val, B4) <<  4) | \
		 (BIT(val, B3) <<  3) | \
		 (BIT(val, B2) <<  2) | \
		 (BIT(val, B1) <<  1) | \
		 (BIT(val, B0) <<  0))

#define BITSWAP24(val,B23,B22,B21,B20,B19,B18,B17,B16,B15,B14,B13,B12,B11,B10,B9,B8,B7,B6,B5,B4,B3,B2,B1,B0) \
		((BIT(val,B23) << 23) | \
		 (BIT(val,B22) << 22) | \
		 (BIT(val,B21) << 21) | \
		 (BIT(val,B20) << 20) | \
		 (BIT(val,B19) << 19) | \
		 (BIT(val,B18) << 18) | \
		 (BIT(val,B17) << 17) | \
		 (BIT(val,B16) << 16) | \
		 (BIT(val,B15) << 15) | \
		 (BIT(val,B14) << 14) | \
		 (BIT(val,B13) << 13) | \
		 (BIT(val,B12) << 12) | \
		 (BIT(val,B11) << 11) | \
		 (BIT(val,B10) << 10) | \
		 (BIT(val, B9) <<  9) | \
		 (BIT(val, B8) <<  8) | \
		 (BIT(val, B7) <<  7) | \
		 (BIT(val, B6) <<  6) | \
		 (BIT(val, B5) <<  5) | \
		 (BIT(val, B4) <<  4) | \
		 (BIT(val, B3) <<  3) | \
		 (BIT(val, B2) <<  2) | \
		 (BIT(val, B1) <<  1) | \
		 (BIT(val, B0) <<  0))

#define BITSWAP32(val,B31,B30,B29,B28,B27,B26,B25,B24,B23,B22,B21,B20,B19,B18,B17,B16,B15,B14,B13,B12,B11,B10,B9,B8,B7,B6,B5,B4,B3,B2,B1,B0) \
		((BIT(val,B31) << 31) | \
		 (BIT(val,B30) << 30) | \
		 (BIT(val,B29) << 29) | \
		 (BIT(val,B28) << 28) | \
		 (BIT(val,B27) << 27) | \
		 (BIT(val,B26) << 26) | \
		 (BIT(val,B25) << 25) | \
		 (BIT(val,B24) << 24) | \
		 (BIT(val,B23) << 23) | \
		 (BIT(val,B22) << 22) | \
		 (BIT(val,B21) << 21) | \
		 (BIT(val,B20) << 20) | \
		 (BIT(val,B19) << 19) | \
		 (BIT(val,B18) << 18) | \
		 (BIT(val,B17) << 17) | \
		 (BIT(val,B16) << 16) | \
		 (BIT(val,B15) << 15) | \
		 (BIT(val,B14) << 14) | \
		 (BIT(val,B13) << 13) | \
		 (BIT(val,B12) << 12) | \
		 (BIT(val,B11) << 11) | \
		 (BIT(val,B10) << 10) | \
		 (BIT(val, B9) <<  9) | \
		 (BIT(val, B8) <<  8) | \
		 (BIT(val, B7) <<  7) | \
		 (BIT(val, B6) <<  6) | \
		 (BIT(val, B5) <<  5) | \
		 (BIT(val, B4) <<  4) | \
		 (BIT(val, B3) <<  3) | \
		 (BIT(val, B2) <<  2) | \
		 (BIT(val, B1) <<  1) | \
		 (BIT(val, B0) <<  0))



/***************************************************************************
    FUNCTION PROTOTYPES
***************************************************************************/

/* Used by assert(), so definition here instead of mame.h */
DECL_NORETURN void CLIB_DECL fatalerror(const char *text, ...) ATTR_PRINTF(1,2) ATTR_NORETURN;
DECL_NORETURN void CLIB_DECL fatalerror_exitcode(running_machine *machine, int exitcode, const char *text, ...) ATTR_PRINTF(3,4) ATTR_NORETURN;



/***************************************************************************
    INLINE FUNCTIONS
***************************************************************************/

/* population count */
INLINE int popcount(UINT32 val)
{
	int count;

	for (count = 0; val != 0; count++)
		val &= val - 1;
	return count;
}


/* convert a series of 32 bits into a float */
INLINE float u2f(UINT32 v)
{
	union {
		float ff;
		UINT32 vv;
	} u;
	u.vv = v;
	return u.ff;
}


/* convert a float into a series of 32 bits */
INLINE UINT32 f2u(float f)
{
	union {
		float ff;
		UINT32 vv;
	} u;
	u.ff = f;
	return u.vv;
}


/* convert a series of 64 bits into a double */
INLINE double u2d(UINT64 v)
{
	union {
		double dd;
		UINT64 vv;
	} u;
	u.vv = v;
	return u.dd;
}


/* convert a double into a series of 64 bits */
INLINE UINT64 d2u(double d)
{
	union {
		double dd;
		UINT64 vv;
	} u;
	u.dd = d;
	return u.vv;
}

#endif	/* __MAMECORE_H__ */
