// license:BSD-3-Clause
// copyright-holders:Nicola Salmoria, Aaron Giles
/***************************************************************************

    emucore.h

    General core utilities and macros used throughout the emulator.

***************************************************************************/

#ifndef MAME_EMU_EMUCORE_H
#define MAME_EMU_EMUCORE_H

#pragma once

// some cleanups for Solaris for things defined in stdlib.h
#if defined(__sun__) && defined(__svr4__)
#undef si_status
#undef WWORD
#endif

// centralised forward declarations
#include "emufwd.h"

// common stuff from lib/util
#include "corealloc.h"
#include "coretmpl.h"
#include "bitmap.h"
#include "endianness.h"
#include "strformat.h"
#include "vecstream.h"

// common stuff from osd
#include "osdcomm.h"

// standard C++ includes
#include <exception>
#include <string>
#include <type_traits>
#include <typeinfo>

// standard C includes
#include <cassert>
#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <cstring>


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

using endianness_t = util::endianness;

using util::BYTE_XOR_BE;
using util::BYTE_XOR_LE;
using util::BYTE4_XOR_BE;
using util::BYTE4_XOR_LE;
using util::WORD_XOR_BE;
using util::WORD_XOR_LE;
using util::BYTE8_XOR_BE;
using util::BYTE8_XOR_LE;
using util::WORD2_XOR_BE;
using util::WORD2_XOR_LE;
using util::DWORD_XOR_BE;
using util::DWORD_XOR_LE;


// input ports support up to 32 bits each
typedef u32 ioport_value;

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

constexpr endianness_t ENDIANNESS_LITTLE = util::endianness::little;
constexpr endianness_t ENDIANNESS_BIG    = util::endianness::big;
constexpr endianness_t ENDIANNESS_NATIVE = util::endianness::native;


// M_PI is not part of the C/C++ standards and is not present on
// strict ANSI compilers or when compiling under GCC with -ansi
#ifndef M_PI
#define M_PI                            3.14159265358979323846
#endif


/// \name Image orientation flags
/// \{

constexpr int ORIENTATION_FLIP_X   = 0x0001;  ///< Mirror horizontally (in the X direction)
constexpr int ORIENTATION_FLIP_Y   = 0x0002;  ///< Mirror vertically (in the Y direction)
constexpr int ORIENTATION_SWAP_XY  = 0x0004;  ///< Mirror along the top-left/bottom-right diagonal

constexpr int ROT0                 = 0;
constexpr int ROT90                = ORIENTATION_SWAP_XY | ORIENTATION_FLIP_X;  ///< Rotate 90 degrees clockwise
constexpr int ROT180               = ORIENTATION_FLIP_X | ORIENTATION_FLIP_Y;   ///< Rotate 180 degrees
constexpr int ROT270               = ORIENTATION_SWAP_XY | ORIENTATION_FLIP_Y;  ///< Rotate 90 degrees anti-clockwise (270 degrees clockwise)

/// \}



//**************************************************************************
//  COMMON MACROS
//**************************************************************************

// this macro passes an item followed by a string version of itself as two consecutive parameters
#define NAME(x) x, #x

// this macro wraps a function 'x' and can be used to pass a function followed by its name
#define FUNC(x) &x, #x


// macros to convert radians to degrees and degrees to radians
template <typename T> constexpr auto RADIAN_TO_DEGREE(T const &x) { return (180.0 / M_PI) * x; }
template <typename T> constexpr auto DEGREE_TO_RADIAN(T const &x) { return (M_PI / 180.0) * x; }


//**************************************************************************
//  EXCEPTION CLASSES
//**************************************************************************

// emu_exception is the base class for all emu-related exceptions
class emu_exception : public std::exception { };


// emu_fatalerror is a generic fatal exception that provides an error string
class emu_fatalerror : public emu_exception
{
public:
	emu_fatalerror(emu_fatalerror const &) = default;
	emu_fatalerror(emu_fatalerror &&) = default;
	emu_fatalerror(util::format_argument_pack<char> const &args);
	emu_fatalerror(int _exitcode, util::format_argument_pack<char> const &args);

	template <typename Format, typename... Params, typename = std::enable_if_t<!std::is_base_of_v<emu_fatalerror, std::remove_reference_t<Format> > > >
	emu_fatalerror(Format &&fmt, Params &&... args)
		: emu_fatalerror(static_cast<util::format_argument_pack<char> const &>(util::make_format_argument_pack(std::forward<Format>(fmt), std::forward<Params>(args)...)))
	{
	}
	template <typename Format, typename... Params>
	emu_fatalerror(int _exitcode, Format &&fmt, Params &&... args)
		: emu_fatalerror(_exitcode, static_cast<util::format_argument_pack<char> const &>(util::make_format_argument_pack(std::forward<Format>(fmt), std::forward<Params>(args)...)))
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


//**************************************************************************
//  USEFUL UTILITIES
//**************************************************************************

using util::make_unique_clear;

#endif // MAME_EMU_EMUCORE_H
