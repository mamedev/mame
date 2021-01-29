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
#include <cassert>
#include <cmath>
#include <cstdio>
#include <cstring>
#include <cstdlib>
#include <cstdarg>

// some cleanups for Solaris for things defined in stdlib.h
#if defined(__sun__) && defined(__svr4__)
#undef si_status
#undef WWORD
#endif

// standard C++ includes
#include <exception>
#include <string>
#include <type_traits>
#include <typeinfo>

// core system includes
#include "osdcomm.h"
#include "emualloc.h"
#include "coretmpl.h"
#include "bitmap.h"
#include "strformat.h"
#include "vecstream.h"

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
using util::make_bitmask;
using util::BIT;
using util::bitswap;
using util::iabs;
using util::string_format;


// genf is a generic function pointer; cast function pointers to this instead of void *
typedef void genf(void);

// pen_t is used to represent pixel values in bitmaps
typedef u32 pen_t;



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


// these are UTF-8 encoded strings for common characters
#define UTF8_NBSP               "\xc2\xa0"          /* non-breaking space */

#define UTF8_MULTIPLY           "\xc3\x97"          /* multiplication sign */
#define UTF8_DIVIDE             "\xc3\xb7"          /* division sign */
#define UTF8_SQUAREROOT         "\xe2\x88\x9a"      /* square root symbol */
#define UTF8_PLUSMINUS          "\xc2\xb1"          /* plusminus symbol */

#define UTF8_POW_2              "\xc2\xb2"          /* superscript 2 */
#define UTF8_POW_X              "\xcb\xa3"          /* superscript x */
#define UTF8_POW_Y              "\xca\xb8"          /* superscript y */
#define UTF8_PRIME              "\xca\xb9"          /* prime symbol */
#define UTF8_DEGREES            "\xc2\xb0"          /* degrees symbol */

#define UTF8_SMALL_PI           "\xcf\x80"          /* Greek small letter pi */
#define UTF8_CAPITAL_SIGMA      "\xce\xa3"          /* Greek capital letter sigma */
#define UTF8_CAPITAL_DELTA      "\xce\x94"          /* Greek capital letter delta */

#define UTF8_MACRON             "\xc2\xaf"          /* macron symbol */
#define UTF8_NONSPACE_MACRON    "\xcc\x84"          /* nonspace macron, use after another char */

#define a_RING                  "\xc3\xa5"          /* small a with a ring */
#define a_UMLAUT                "\xc3\xa4"          /* small a with an umlaut */
#define o_UMLAUT                "\xc3\xb6"          /* small o with an umlaut */
#define u_UMLAUT                "\xc3\xbc"          /* small u with an umlaut */
#define e_ACUTE                 "\xc3\xa9"          /* small e with an acute */
#define n_TILDE                 "\xc3\xb1"          /* small n with a tilde */

#define A_RING                  "\xc3\x85"          /* capital A with a ring */
#define A_UMLAUT                "\xc3\x84"          /* capital A with an umlaut */
#define O_UMLAUT                "\xc3\x96"          /* capital O with an umlaut */
#define U_UMLAUT                "\xc3\x9c"          /* capital U with an umlaut */
#define E_ACUTE                 "\xc3\x89"          /* capital E with an acute */
#define N_TILDE                 "\xc3\x91"          /* capital N with a tilde */

#define UTF8_LEFT               "\xe2\x86\x90"      /* cursor left */
#define UTF8_RIGHT              "\xe2\x86\x92"      /* cursor right */
#define UTF8_UP                 "\xe2\x86\x91"      /* cursor up */
#define UTF8_DOWN               "\xe2\x86\x93"      /* cursor down */



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
	emu_fatalerror(util::format_argument_pack<std::ostream> const &args);
	emu_fatalerror(int _exitcode, util::format_argument_pack<std::ostream> const &args);

	template <typename Format, typename... Params>
	emu_fatalerror(Format const &fmt, Params &&... args)
		: emu_fatalerror(static_cast<util::format_argument_pack<std::ostream> const &>(util::make_format_argument_pack(fmt, std::forward<Params>(args)...)))
	{
	}
	template <typename Format, typename... Params>
	emu_fatalerror(int _exitcode, Format const &fmt, Params &&... args)
		: emu_fatalerror(_exitcode, static_cast<util::format_argument_pack<std::ostream> const &>(util::make_format_argument_pack(fmt, std::forward<Params>(args)...)))
	{
	}

	virtual char const *what() const noexcept override { return m_text.c_str(); }
	int exitcode() const noexcept { return m_code; }

private:
	std::string m_text;
	int m_code;
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

[[noreturn]] void report_bad_cast(const std::type_info &src_type, const std::type_info &dst_type);
[[noreturn]] void report_bad_device_cast(const device_t *dev, const std::type_info &src_type, const std::type_info &dst_type);

template <typename Dest, typename Source>
inline void report_bad_cast(Source *src)
{
	if constexpr (std::is_base_of_v<device_t, Source>)
	{
		if (src) report_bad_device_cast(src, typeid(Source), typeid(Dest));
		else report_bad_cast(typeid(Source), typeid(Dest));
	}
	else
	{
		device_t const *dev(dynamic_cast<device_t const *>(src));
		if (dev) report_bad_device_cast(dev, typeid(Source), typeid(Dest));
		else report_bad_cast(typeid(Source), typeid(Dest));
	}
}

// template function for casting from a base class to a derived class that is checked
// in debug builds and fast in release builds
template <typename Dest, typename Source>
inline Dest downcast(Source *src)
{
#if defined(MAME_DEBUG)
	Dest const chk(dynamic_cast<Dest>(src));
	if (chk != src) report_bad_cast<std::remove_pointer_t<Dest>, Source>(src);
#endif
	return static_cast<Dest>(src);
}

template<class Dest, class Source>
inline Dest downcast(Source &src)
{
#if defined(MAME_DEBUG)
	std::remove_reference_t<Dest> *const chk(dynamic_cast<std::remove_reference_t<Dest> *>(&src));
	if (chk != &src) report_bad_cast<std::remove_reference_t<Dest>, Source>(&src);
#endif
	return static_cast<Dest>(src);
}



//**************************************************************************
//  INLINE FUNCTIONS
//**************************************************************************

template <typename... T>
[[noreturn]] inline void fatalerror(T &&... args)
{
	throw emu_fatalerror(std::forward<T>(args)...);
}


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

#endif // MAME_EMU_EMUCORE_H
