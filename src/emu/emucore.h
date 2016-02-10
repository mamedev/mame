// license:BSD-3-Clause
// copyright-holders:Nicola Salmoria, Aaron Giles
/***************************************************************************

    emucore.h

    General core utilities and macros used throughout the emulator.
***************************************************************************/

#pragma once

#ifndef __EMUCORE_H__
#define __EMUCORE_H__

// standard C includes
#include <assert.h>
#include <math.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdarg.h>

// some cleanups for Solaris for things defined in stdlib.h
#if defined(__sun__) && defined(__svr4__)
#undef si_status
#undef WWORD
#endif

// standard C++ includes
#include <exception>
#include <typeinfo>

// core system includes
#include "osdcomm.h"
#include "emualloc.h"
#include "corestr.h"
#include "bitmap.h"
#include "tagmap.h"



//**************************************************************************
//  COMPILER-SPECIFIC NASTINESS
//**************************************************************************

// Suppress warnings about redefining the macro 'PPC' on LinuxPPC.
#undef PPC

// Suppress warnings about redefining the macro 'ARM' on ARM.
#undef ARM



//**************************************************************************
//  FUNDAMENTAL TYPES
//**************************************************************************

// genf is a generic function pointer; cast function pointers to this instead of void *
typedef void genf(void);

// pen_t is used to represent pixel values in bitmaps
typedef UINT32 pen_t;

// stream_sample_t is used to represent a single sample in a sound stream
typedef INT32 stream_sample_t;

// running_machine is core to pretty much everything
class running_machine;



//**************************************************************************
//  USEFUL COMPOSITE TYPES
//**************************************************************************

// generic_ptr is a union of pointers to various sizes
union generic_ptr
{
	generic_ptr(void *value) { v = value; }
	void *      v;
	INT8 *      i8;
	UINT8 *     u8;
	INT16 *     i16;
	UINT16 *    u16;
	INT32 *     i32;
	UINT32 *    u32;
	INT64 *     i64;
	UINT64 *    u64;
};


// PAIR is an endian-safe union useful for representing 32-bit CPU registers
union PAIR
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
};


// PAIR16 is a 16-bit extension of a PAIR
union PAIR16
{
#ifdef LSB_FIRST
	struct { UINT8 l,h; } b;
	struct { INT8 l,h; } sb;
#else
	struct { UINT8 h,l; } b;
	struct { INT8 h,l; } sb;
#endif
	UINT16 w;
	INT16 sw;
};


// PAIR64 is a 64-bit extension of a PAIR
union PAIR64
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
};



//**************************************************************************
//  COMMON CONSTANTS
//**************************************************************************

// constants for expression endianness
enum endianness_t
{
	ENDIANNESS_LITTLE,
	ENDIANNESS_BIG
};


// declare native endianness to be one or the other
#ifdef LSB_FIRST
const endianness_t ENDIANNESS_NATIVE = ENDIANNESS_LITTLE;
#else
const endianness_t ENDIANNESS_NATIVE = ENDIANNESS_BIG;
#endif


// M_PI is not part of the C/C++ standards and is not present on
// strict ANSI compilers or when compiling under GCC with -ansi
#ifndef M_PI
#define M_PI                            3.14159265358979323846
#endif


// orientation of bitmaps
#define ORIENTATION_FLIP_X              0x0001  /* mirror everything in the X direction */
#define ORIENTATION_FLIP_Y              0x0002  /* mirror everything in the Y direction */
#define ORIENTATION_SWAP_XY             0x0004  /* mirror along the top-left/bottom-right diagonal */

#define ROT0                            0
#define ROT90                           (ORIENTATION_SWAP_XY | ORIENTATION_FLIP_X)  /* rotate clockwise 90 degrees */
#define ROT180                          (ORIENTATION_FLIP_X | ORIENTATION_FLIP_Y)   /* rotate 180 degrees */
#define ROT270                          (ORIENTATION_SWAP_XY | ORIENTATION_FLIP_Y)  /* rotate counter-clockwise 90 degrees */



//**************************************************************************
//  COMMON MACROS
//**************************************************************************

// macro for defining a copy constructor and assignment operator to prevent copying
#define DISABLE_COPYING(_Type) \
private: \
	_Type(const _Type &) = delete; \
	_Type &operator=(const _Type &) = delete

// macro for declaring enumerator operators that increment/decrement like plain old C
#define DECLARE_ENUM_OPERATORS(_Type) \
inline void operator++(_Type &value) { value = (_Type)((int)value + 1); } \
inline void operator++(_Type &value, int) { value = (_Type)((int)value + 1); } \
inline void operator--(_Type &value) { value = (_Type)((int)value - 1); } \
inline void operator--(_Type &value, int) { value = (_Type)((int)value - 1); }


// this macro passes an item followed by a string version of itself as two consecutive parameters
#define NAME(x) x, #x

// this macro wraps a function 'x' and can be used to pass a function followed by its name
#define FUNC(x) &x, #x
#define FUNC_NULL NULL, "(null)"


// standard assertion macros
#undef assert
#undef assert_always

#if defined(MAME_DEBUG_FAST)
#define assert(x)               do { } while (0)
#define assert_always(x, msg)   do { if (!(x)) throw emu_fatalerror("Fatal error: %s\nCaused by assert: %s:%d: %s", msg, __FILE__, __LINE__, #x); } while (0)
#elif defined(MAME_DEBUG)
#define assert(x)               do { if (!(x)) throw emu_fatalerror("assert: %s:%d: %s", __FILE__, __LINE__, #x); } while (0)
#define assert_always(x, msg)   do { if (!(x)) throw emu_fatalerror("Fatal error: %s\nCaused by assert: %s:%d: %s", msg, __FILE__, __LINE__, #x); } while (0)
#else
#define assert(x)               do { } while (0)
#define assert_always(x, msg)   do { if (!(x)) throw emu_fatalerror("Fatal error: %s (%s:%d)", msg, __FILE__, __LINE__); } while (0)
#endif


// macros to convert radians to degrees and degrees to radians
#define RADIAN_TO_DEGREE(x)   ((180.0 / M_PI) * (x))
#define DEGREE_TO_RADIAN(x)   ((M_PI / 180.0) * (x))


// endian-based value: first value is if 'endian' is little-endian, second is if 'endian' is big-endian
#define ENDIAN_VALUE_LE_BE(endian,leval,beval)  (((endian) == ENDIANNESS_LITTLE) ? (leval) : (beval))

// endian-based value: first value is if native endianness is little-endian, second is if native is big-endian
#define NATIVE_ENDIAN_VALUE_LE_BE(leval,beval)  ENDIAN_VALUE_LE_BE(ENDIANNESS_NATIVE, leval, beval)

// endian-based value: first value is if 'endian' matches native, second is if 'endian' doesn't match native
#define ENDIAN_VALUE_NE_NNE(endian,neval,nneval) (((endian) == ENDIANNESS_NATIVE) ? (neval) : (nneval))


// useful macros to deal with bit shuffling encryptions
#define BIT(x,n) (((x)>>(n))&1)

#define BITSWAP8(val,B7,B6,B5,B4,B3,B2,B1,B0) \
	((BIT(val,B7) << 7) | (BIT(val,B6) << 6) | (BIT(val,B5) << 5) | (BIT(val,B4) << 4) | \
		(BIT(val,B3) << 3) | (BIT(val,B2) << 2) | (BIT(val,B1) << 1) | (BIT(val,B0) << 0))

#define BITSWAP16(val,B15,B14,B13,B12,B11,B10,B9,B8,B7,B6,B5,B4,B3,B2,B1,B0) \
	((BIT(val,B15) << 15) | (BIT(val,B14) << 14) | (BIT(val,B13) << 13) | (BIT(val,B12) << 12) | \
		(BIT(val,B11) << 11) | (BIT(val,B10) << 10) | (BIT(val, B9) <<  9) | (BIT(val, B8) <<  8) | \
		(BIT(val, B7) <<  7) | (BIT(val, B6) <<  6) | (BIT(val, B5) <<  5) | (BIT(val, B4) <<  4) | \
		(BIT(val, B3) <<  3) | (BIT(val, B2) <<  2) | (BIT(val, B1) <<  1) | (BIT(val, B0) <<  0))

#define BITSWAP24(val,B23,B22,B21,B20,B19,B18,B17,B16,B15,B14,B13,B12,B11,B10,B9,B8,B7,B6,B5,B4,B3,B2,B1,B0) \
	((BIT(val,B23) << 23) | (BIT(val,B22) << 22) | (BIT(val,B21) << 21) | (BIT(val,B20) << 20) | \
		(BIT(val,B19) << 19) | (BIT(val,B18) << 18) | (BIT(val,B17) << 17) | (BIT(val,B16) << 16) | \
		(BIT(val,B15) << 15) | (BIT(val,B14) << 14) | (BIT(val,B13) << 13) | (BIT(val,B12) << 12) | \
		(BIT(val,B11) << 11) | (BIT(val,B10) << 10) | (BIT(val, B9) <<  9) | (BIT(val, B8) <<  8) | \
		(BIT(val, B7) <<  7) | (BIT(val, B6) <<  6) | (BIT(val, B5) <<  5) | (BIT(val, B4) <<  4) | \
		(BIT(val, B3) <<  3) | (BIT(val, B2) <<  2) | (BIT(val, B1) <<  1) | (BIT(val, B0) <<  0))

#define BITSWAP32(val,B31,B30,B29,B28,B27,B26,B25,B24,B23,B22,B21,B20,B19,B18,B17,B16,B15,B14,B13,B12,B11,B10,B9,B8,B7,B6,B5,B4,B3,B2,B1,B0) \
	((BIT(val,B31) << 31) | (BIT(val,B30) << 30) | (BIT(val,B29) << 29) | (BIT(val,B28) << 28) | \
		(BIT(val,B27) << 27) | (BIT(val,B26) << 26) | (BIT(val,B25) << 25) | (BIT(val,B24) << 24) | \
		(BIT(val,B23) << 23) | (BIT(val,B22) << 22) | (BIT(val,B21) << 21) | (BIT(val,B20) << 20) | \
		(BIT(val,B19) << 19) | (BIT(val,B18) << 18) | (BIT(val,B17) << 17) | (BIT(val,B16) << 16) | \
		(BIT(val,B15) << 15) | (BIT(val,B14) << 14) | (BIT(val,B13) << 13) | (BIT(val,B12) << 12) | \
		(BIT(val,B11) << 11) | (BIT(val,B10) << 10) | (BIT(val, B9) <<  9) | (BIT(val, B8) <<  8) | \
		(BIT(val, B7) <<  7) | (BIT(val, B6) <<  6) | (BIT(val, B5) <<  5) | (BIT(val, B4) <<  4) | \
		(BIT(val, B3) <<  3) | (BIT(val, B2) <<  2) | (BIT(val, B1) <<  1) | (BIT(val, B0) <<  0))



//**************************************************************************
//  EXCEPTION CLASSES
//**************************************************************************

// emu_exception is the base class for all emu-related exceptions
class emu_exception : public std::exception { };


// emu_fatalerror is a generic fatal exception that provides an error string
class emu_fatalerror : public emu_exception
{
public:
	emu_fatalerror(const char *format, ...) ATTR_PRINTF(2,3);
	emu_fatalerror(const char *format, va_list ap);
	emu_fatalerror(int _exitcode, const char *format, ...) ATTR_PRINTF(3,4);
	emu_fatalerror(int _exitcode, const char *format, va_list ap);

	const char *string() const { return text; }
	int exitcode() const { return code; }

private:
	char text[1024];
	int code;
};



//**************************************************************************
//  CASTING TEMPLATES
//**************************************************************************

class device_t;

void report_bad_cast(const std::type_info &src_type, const std::type_info &dst_type);
void report_bad_device_cast(const device_t *dev, const std::type_info &src_type, const std::type_info &dst_type);

// template function for casting from a base class to a derived class that is checked
// in debug builds and fast in release builds
template<class _Dest, class _Source>
inline _Dest downcast(_Source *src)
{
#if defined(MAME_DEBUG) && !defined(MAME_DEBUG_FAST)
	try {
		if (dynamic_cast<_Dest>(src) != src)
		{
			if (dynamic_cast<const device_t *>(src) != nullptr)
				report_bad_device_cast(dynamic_cast<const device_t *>(src), typeid(src), typeid(_Dest));
			else
				report_bad_cast(typeid(src), typeid(_Dest));
		}
	}
	catch (std::bad_cast &)
	{
		report_bad_cast(typeid(src), typeid(_Dest));
	}
#endif
	return static_cast<_Dest>(src);
}

template<class _Dest, class _Source>
inline _Dest downcast(_Source &src)
{
#if defined(MAME_DEBUG) && !defined(MAME_DEBUG_FAST)
	try {
		if (&dynamic_cast<_Dest>(src) != &src)
		{
			if (dynamic_cast<const device_t *>(&src) != nullptr)
				report_bad_device_cast(dynamic_cast<const device_t *>(&src), typeid(src), typeid(_Dest));
			else
				report_bad_cast(typeid(src), typeid(_Dest));
		}
	}
	catch (std::bad_cast &)
	{
		report_bad_cast(typeid(src), typeid(_Dest));
	}
#endif
	return static_cast<_Dest>(src);
}



//**************************************************************************
//  FUNCTION PROTOTYPES
//**************************************************************************

ATTR_NORETURN void fatalerror(const char *format, ...) ATTR_PRINTF(1,2);
ATTR_NORETURN void fatalerror_exitcode(running_machine &machine, int exitcode, const char *format, ...) ATTR_PRINTF(3,4);

//**************************************************************************
//  INLINE FUNCTIONS
//**************************************************************************

// population count
#if !defined(__NetBSD__)
inline int popcount(UINT32 val)
{
	int count;

	for (count = 0; val != 0; count++)
		val &= val - 1;
	return count;
}
#endif


// convert a series of 32 bits into a float
inline float u2f(UINT32 v)
{
	union {
		float ff;
		UINT32 vv;
	} u;
	u.vv = v;
	return u.ff;
}


// convert a float into a series of 32 bits
inline UINT32 f2u(float f)
{
	union {
		float ff;
		UINT32 vv;
	} u;
	u.ff = f;
	return u.vv;
}


// convert a series of 64 bits into a double
inline double u2d(UINT64 v)
{
	union {
		double dd;
		UINT64 vv;
	} u;
	u.vv = v;
	return u.dd;
}


// convert a double into a series of 64 bits
inline UINT64 d2u(double d)
{
	union {
		double dd;
		UINT64 vv;
	} u;
	u.dd = d;
	return u.vv;
}

#endif  /* __EMUCORE_H__ */
