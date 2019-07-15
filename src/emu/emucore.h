// license:BSD-3-Clause
// copyright-holders:Nicola Salmoria, Aaron Giles
/***************************************************************************

    emucore.h

    General core utilities and macros used throughout the emulator.
***************************************************************************/

#ifndef MAME_EMU_EMUCORE_H
#define MAME_EMU_EMUCORE_H

#pragma once

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
#include <cassert>
#include <exception>
#include <type_traits>
#include <typeinfo>

// core system includes
#include "osdcomm.h"
#include "emualloc.h"
#include "corestr.h"
#include "bitmap.h"

#include "emufwd.h"


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

// explicitly sized integers
using osd::u8;
using osd::u16;
using osd::u32;
using osd::u64;
using osd::s8;
using osd::s16;
using osd::s32;
using osd::s64;

// useful utility functions
using util::underlying_value;
using util::enum_value;
using util::BIT;
using util::bitswap;
using util::iabs;


// genf is a generic function pointer; cast function pointers to this instead of void *
typedef void genf(void);

// pen_t is used to represent pixel values in bitmaps
typedef u32 pen_t;

// stream_sample_t is used to represent a single sample in a sound stream
typedef s32 stream_sample_t;



//**************************************************************************
//  USEFUL COMPOSITE TYPES
//**************************************************************************

// PAIR is an endian-safe union useful for representing 32-bit CPU registers
union PAIR
{
#ifdef LSB_FIRST
	struct { u8 l,h,h2,h3; } b;
	struct { u16 l,h; } w;
	struct { s8 l,h,h2,h3; } sb;
	struct { s16 l,h; } sw;
#else
	struct { u8 h3,h2,h,l; } b;
	struct { s8 h3,h2,h,l; } sb;
	struct { u16 h,l; } w;
	struct { s16 h,l; } sw;
#endif
	u32 d;
	s32 sd;
};


// PAIR16 is a 16-bit extension of a PAIR
union PAIR16
{
#ifdef LSB_FIRST
	struct { u8 l,h; } b;
	struct { s8 l,h; } sb;
#else
	struct { u8 h,l; } b;
	struct { s8 h,l; } sb;
#endif
	u16 w;
	s16 sw;
};


// PAIR64 is a 64-bit extension of a PAIR
union PAIR64
{
#ifdef LSB_FIRST
	struct { u8 l,h,h2,h3,h4,h5,h6,h7; } b;
	struct { u16 l,h,h2,h3; } w;
	struct { u32 l,h; } d;
	struct { s8 l,h,h2,h3,h4,h5,h6,h7; } sb;
	struct { s16 l,h,h2,h3; } sw;
	struct { s32 l,h; } sd;
#else
	struct { u8 h7,h6,h5,h4,h3,h2,h,l; } b;
	struct { u16 h3,h2,h,l; } w;
	struct { u32 h,l; } d;
	struct { s8 h7,h6,h5,h4,h3,h2,h,l; } sb;
	struct { s16 h3,h2,h,l; } sw;
	struct { s32 h,l; } sd;
#endif
	u64 q;
	s64 sq;
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

extern const char *const endianness_names[2];

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
constexpr int ORIENTATION_FLIP_X   = 0x0001;  // mirror everything in the X direction
constexpr int ORIENTATION_FLIP_Y   = 0x0002;  // mirror everything in the Y direction
constexpr int ORIENTATION_SWAP_XY  = 0x0004;  // mirror along the top-left/bottom-right diagonal

constexpr int ROT0                 = 0;
constexpr int ROT90                = ORIENTATION_SWAP_XY | ORIENTATION_FLIP_X;  // rotate clockwise 90 degrees
constexpr int ROT180               = ORIENTATION_FLIP_X | ORIENTATION_FLIP_Y;   // rotate 180 degrees
constexpr int ROT270               = ORIENTATION_SWAP_XY | ORIENTATION_FLIP_Y;  // rotate counter-clockwise 90 degrees



//**************************************************************************
//  COMMON MACROS
//**************************************************************************

// macro for defining a copy constructor and assignment operator to prevent copying
#define DISABLE_COPYING(TYPE) \
	TYPE(const TYPE &) = delete; \
	TYPE &operator=(const TYPE &) = delete

// macro for declaring enumeration operators that increment/decrement like plain old C
#define DECLARE_ENUM_INCDEC_OPERATORS(TYPE) \
inline TYPE &operator++(TYPE &value) { return value = TYPE(std::underlying_type_t<TYPE>(value) + 1); } \
inline TYPE &operator--(TYPE &value) { return value = TYPE(std::underlying_type_t<TYPE>(value) - 1); } \
inline TYPE operator++(TYPE &value, int) { TYPE const old(value); ++value; return old; } \
inline TYPE operator--(TYPE &value, int) { TYPE const old(value); --value; return old; }

// macro for declaring bitwise operators for an enumerated type
#define DECLARE_ENUM_BITWISE_OPERATORS(TYPE) \
constexpr TYPE operator~(TYPE value) { return TYPE(~std::underlying_type_t<TYPE>(value)); } \
constexpr TYPE operator&(TYPE a, TYPE b) { return TYPE(std::underlying_type_t<TYPE>(a) & std::underlying_type_t<TYPE>(b)); } \
constexpr TYPE operator|(TYPE a, TYPE b) { return TYPE(std::underlying_type_t<TYPE>(a) | std::underlying_type_t<TYPE>(b)); } \
inline TYPE &operator&=(TYPE &a, TYPE b) { return a = a & b; } \
inline TYPE &operator|=(TYPE &a, TYPE b) { return a = a | b; }


// this macro passes an item followed by a string version of itself as two consecutive parameters
#define NAME(x) x, #x

// this macro wraps a function 'x' and can be used to pass a function followed by its name
#define FUNC(x) &x, #x


// standard assertion macros
#undef assert_always

#if defined(MAME_DEBUG_FAST)
#define assert_always(x, msg)   do { if (!(x)) throw emu_fatalerror("%s\nCaused by assert: %s:%d: %s", msg, __FILE__, __LINE__, #x); } while (0)
#elif defined(MAME_DEBUG)
#define assert_always(x, msg)   do { if (!(x)) throw emu_fatalerror("%s\nCaused by assert: %s:%d: %s", msg, __FILE__, __LINE__, #x); } while (0)
#else
#define assert_always(x, msg)   do { if (!(x)) throw emu_fatalerror("%s (%s:%d)", msg, __FILE__, __LINE__); } while (0)
#endif


// macros to convert radians to degrees and degrees to radians
template <typename T> constexpr auto RADIAN_TO_DEGREE(T const &x) { return (180.0 / M_PI) * x; }
template <typename T> constexpr auto DEGREE_TO_RADIAN(T const &x) { return (M_PI / 180.0) * x; }


// endian-based value: first value is if 'endian' is little-endian, second is if 'endian' is big-endian
#define ENDIAN_VALUE_LE_BE(endian,leval,beval)  (((endian) == ENDIANNESS_LITTLE) ? (leval) : (beval))

// endian-based value: first value is if native endianness is little-endian, second is if native is big-endian
#define NATIVE_ENDIAN_VALUE_LE_BE(leval,beval)  ENDIAN_VALUE_LE_BE(ENDIANNESS_NATIVE, leval, beval)

// endian-based value: first value is if 'endian' matches native, second is if 'endian' doesn't match native
#define ENDIAN_VALUE_NE_NNE(endian,neval,nneval) (((endian) == ENDIANNESS_NATIVE) ? (neval) : (nneval))


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

class tag_add_exception
{
public:
	tag_add_exception(const char *tag) : m_tag(tag) { }
	const char *tag() const { return m_tag.c_str(); }
private:
	std::string m_tag;
};

//**************************************************************************
//  CASTING TEMPLATES
//**************************************************************************

void report_bad_cast(const std::type_info &src_type, const std::type_info &dst_type);
void report_bad_device_cast(const device_t *dev, const std::type_info &src_type, const std::type_info &dst_type);

template <typename Dest, typename Source>
inline std::enable_if_t<std::is_base_of<device_t, Source>::value> report_bad_cast(Source *const src)
{
	if (src) report_bad_device_cast(src, typeid(Source), typeid(Dest));
	else report_bad_cast(typeid(Source), typeid(Dest));
}

template <typename Dest, typename Source>
inline std::enable_if_t<!std::is_base_of<device_t, Source>::value> report_bad_cast(Source *const src)
{
	device_t const *dev(dynamic_cast<device_t const *>(src));
	if (dev) report_bad_device_cast(dev, typeid(Source), typeid(Dest));
	else report_bad_cast(typeid(Source), typeid(Dest));
}

// template function for casting from a base class to a derived class that is checked
// in debug builds and fast in release builds
template <typename Dest, typename Source>
inline Dest downcast(Source *src)
{
#if defined(MAME_DEBUG) && !defined(MAME_DEBUG_FAST)
	Dest const chk(dynamic_cast<Dest>(src));
	if (chk != src) report_bad_cast<std::remove_pointer_t<Dest>, Source>(src);
#endif
	return static_cast<Dest>(src);
}

template<class Dest, class Source>
inline Dest downcast(Source &src)
{
#if defined(MAME_DEBUG) && !defined(MAME_DEBUG_FAST)
	std::remove_reference_t<Dest> *const chk(dynamic_cast<std::remove_reference_t<Dest> *>(&src));
	if (chk != &src) report_bad_cast<std::remove_reference_t<Dest>, Source>(&src);
#endif
	return static_cast<Dest>(src);
}



//**************************************************************************
//  FUNCTION PROTOTYPES
//**************************************************************************

[[noreturn]] void fatalerror(const char *format, ...) ATTR_PRINTF(1,2);
[[noreturn]] void fatalerror_exitcode(running_machine &machine, int exitcode, const char *format, ...) ATTR_PRINTF(3,4);

//**************************************************************************
//  INLINE FUNCTIONS
//**************************************************************************

// convert a series of 32 bits into a float
inline float u2f(u32 v)
{
	union {
		float ff;
		u32 vv;
	} u;
	u.vv = v;
	return u.ff;
}


// convert a float into a series of 32 bits
inline u32 f2u(float f)
{
	union {
		float ff;
		u32 vv;
	} u;
	u.ff = f;
	return u.vv;
}


// convert a series of 64 bits into a double
inline double u2d(u64 v)
{
	union {
		double dd;
		u64 vv;
	} u;
	u.vv = v;
	return u.dd;
}


// convert a double into a series of 64 bits
inline u64 d2u(double d)
{
	union {
		double dd;
		u64 vv;
	} u;
	u.dd = d;
	return u.vv;
}

#endif  /* MAME_EMU_EMUCORE_H */
