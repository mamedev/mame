// license:BSD-3-Clause
// copyright-holders:Vas Crabb
/***************************************************************************

    strformat.h

    type-safe printf substitutes

    This header provides type-safe printf substitutes that output to
    std::ostream- or std::string-like objects.  Most format strings
    supported by C99, SUS, glibc and MSVCRT are accepted.  Not all
    features are implemented, and semantics differ in some cases.  Any
    object with an appropriate stream output operator can be used as
    a format argument with the %s conversion.

    Since the funcitons are implemented using C++ iostream, some
    behaviour more closely resembles iostream output operator behaviour
    than printf behaviour.  You are also exposed to bugs in your C++
    iostream implementation (e.g. hexadecimal scientific format doesn't
    work properly on MinGW).

    These functions are designed to be forgiving - using an
    inappropriate conversion for an argument's type just results in the
    default conversion for the type being used.  Inappropriate types or
    out-of-range positions for parameterised field width and precision
    are treated as if no width/precision was specified.  Out-of-range
    argument positions result in the format specification being printed.

    Position specifiers for arguments (%123$), field width (*456$) and
    precision (.*789$) are supported.  Mixing explicit and implied
    positions for arugments/widths/precisions is discouraged, although
    it does produce deterministic behaviour.

    The following format flags are recognised:
    - "#": alternate format - sets showbase/showpoint, and also
           boolalpha for bool with s conversion
    - "0": pad with zeroes rather than spaces, ignored if '-' flag is
           specified or if precision is specified for d/i/u/o/x/X
           conversion
    - "-": left-align output, overrides '0'
    - " ": recognised but not implemented, ignored for u/o/x/X
           conversion
    - "+": show sign for positive numbers, overrides ' ', ignored for
           u/o/x/X conversions
    - "'": recognised for SUS compatibility but ignored (digit grouping
           is controlled by stream locale)
    - "I": recognised for glibc compatibility but ignored (digits are
           controlled by stream locale)

    Precision is supported for conversions by setting precision on the
    stream.  This works as expected for a/A/e/E/f/F/g/G conversions on
    floating-point types, and may work for objects with user-defined
    stream output operators.  Precision for d/i/u/o/x/X conversions
    (minimum digits to print) is not supported.  Precision for s
    conversions (maximum characters to print) is only honoured for
    string-like types (output character pointer/array and
    std::basic_string).

    Length specifiers are supported but not required for d/i/u/o/x/X
    conversions with integer/char/bool arguments.  They result in the
    value being cast to the desired type before being printed.  Length
    specifiers are ignored for other conversions.

    The following length specifiers are recognised:
    - hh:  cast to char/unsigned char for d/i/u/o/x/X
    - h:   cast to short/unsigned short for d/i/u/o/x/X
    - l:   cast to long/unsigned long for d/i/u/o/x/X
    - ll:  cast to long long/unsigned long long for d/i/u/o/x/X
    - L:   always ignored
    - j:   cast to intmax_t/uintmax_t for d/i/u/o/x/X
    - z:   cast to ssize_t/size_t for d/i/u/o/x/X
    - t:   cast to ptrdiff_t for d/i/u/o/x/X
    - I:   cast to ssize_t/size_t for d/i/u/o/x/X
    - I32: cast to int32_t/uint32_t for d/i/u/o/x/X
    - I64: cast to int64_t/uint64_t for d/i/u/o/x/X
    - w:   always ignored

    The following conversions are recognised:
    - d/i: signed decimal for integer/char/bool types
    - u:   unsigned decimal for integer/char/bool types
    - o:   unsigned octal for integer/char/bool types
    - x/X: lower/upppercase unsigned hexadecimal for integer/char/bool
           types or scientific hexadecimal for floating-point types
    - e/E: lower/uppercase scientific decimal for floating-point types
    - f/F: lower/uppercase fixed-point decimal for floating-point types
    - g/G: default stream output format for floating-point types (may
           differ from printf behaviour)
    - a/A: lower/upppercase scientific hexadecimal for floating-point
           types or hexadecimal for integer types
    - c/C: cast integer types to stream's character type, no automatic
           widening or narrowing
    - s/S: default stream output behaviour for argument
    - p/P: cast any integer/char/bool/pointer/array to void const *
    - n:   store characters printed so far, produces no output, argument
           must be pointer to type that std::streamoff is convertible to
    - m:   output of std::strerror(errno), no automatic widening or
           narrowing, does not consume an argument
    - %:   literal %, field width applied, does not consume an argument

    The output stream type for stream_format must be equivalent to a
    std::basic_ostream for duck-typing purposes.  The output string for
    string type for string_format must provide value_type, traits_type
    and allocator_type declarations, and must be constructible from a
    std::basic_string using the same value, traits and allocator types.

    The format string type can be a pointer to a NUL-terminated string,
    an array containing a NUL-terminated or non-terminated string, or a
    STL contiguous container holding a string (e.g. std::string,
    std::vector or std::array).  Note that NUL characters characters are
    only treated as terminators for pointers and arrays, they are
    treated as normal characters for other containers.  A non-contiguous
    container (e.g. std::list or std::deque) will result in undesirable
    behaviour likely culminating in a crash.

    The value type of the format string and the character type of the
    output stream/string need to match.  You can't use a wchar_t format
    to format char output and vice versa.

    The format string encoding must have contiguous decimal digits.  The
    character encoding must not use shift states or multi-byte sequences
    that could result in a format character codepoint appearing as part
    of a multi-byte sequence or being interpreted differently.  ASCII,
    ISO Latin, KOI-8, UTF-8, EUC, EBCDIC, and UTF-EBCDIC encodings meet
    these requirements, while ISO-2022, UTF-7, KOI-7 and Shift-JIS
    encodings do not.  For character types other than char and wchar_t,
    the encoding must be a strict superset of the char encoding.

    The following conditions cause assertion failures in debug builds:
    - Unsupported conversion specifier
    - Out-of-range argument/width/precision position
    - Inappropriate type for parameterised width/precision
    - Positional width/precision specifier not terminated with $
    - Inappropriate type for n conversion

    Some limitations have been described in passing.  Major limitations
    and bugs include:
    - No automatic widening/narrowing support, so no simple way to
      output wide characters/strings to narrow streams/strings and vice
      versa.
    - Precision ignored for d/i/u/o/x/X conversions (should set minimum
      digits to print).
    - Precisoin for s/S conversion is only honoured for string-like
      types (output character pointer/array and std::basic_string).
    - If the output character type is not char, signed char or unsgined
      char, printing the a value of this type with d/i/u/o/x/X
      conversion and no length specifier causes it to be printed as a
      character.  Can be worked around by casting to another integer
      type or using length specifier.
    - Printing with d/i/u/o/x/X conversions may not behave as expected
      if the output character type is an integer type other than char,
      signed char, unsigned char, or wchar_t.
    - Only output character pointer/array is treated as a C string, any
      other pointer/array will be printed as a pointer value.  The
      signed/unsigned/default char are not handled equivalently.
    - There is no length specifier to force cast to int/unsigned.  Can
      be worked around by casting the argument.
    - MSVCRT length specifiers I/I32/I64 will not be recognised if no
      width or precision is specified, as they will be mistaken for
      glibc alternate digits flag.
    - The " " flag to prefix positive numbers with a space is not
      implemented.
    - The "'" and "I" flags are not implemented, as digit grouping and
      characters are controlled by the output stream's locale.
    - The characters used for space- and zero-padding are not locale-
      aware.

***************************************************************************/

#pragma once

#ifndef __MAME_UTIL_STRFORMAT_H__
#define __MAME_UTIL_STRFORMAT_H__

#include <algorithm>
#include <array>
#include <cassert>
#include <cstddef>
#include <cstdint>
#include <cstring>
#include <iterator>
#include <limits>
#include <locale>
#include <sstream>
#include <string>
#include <type_traits>
#include <utility>

#if defined(__GLIBCXX__) && (__GLIBCXX__ < 20150413)
namespace std
{
template<class _Container>
	inline constexpr auto
	cbegin(const _Container& __cont) noexcept(noexcept(std::begin(__cont)))-> decltype(std::begin(__cont))
	{ return std::begin(__cont); }

template<class _Container>
	inline constexpr auto
	cend(const _Container& __cont) noexcept(noexcept(std::end(__cont)))-> decltype(std::end(__cont))
	{ return std::end(__cont); }
}
#endif

namespace util {
namespace detail {
//**************************************************************************
//  FORMAT CHARACTER DEFINITIONS
//**************************************************************************

template <typename Character>
class format_chars
{
public:
	typedef Character char_type;
	enum : Character {
			nul         = Character('\0'),
			space       = Character(' '),
			point       = Character('.'),
			percent     = Character('%'),
			dollar      = Character('$'),
			hash        = Character('#'),
			minus       = Character('-'),
			plus        = Character('+'),
			asterisk    = Character('*'),
			quote       = Character('\''),
			zero        = Character('0'),
			nine        = Character('9'),
			a           = Character('a'),
			A           = Character('A'),
			c           = Character('c'),
			C           = Character('C'),
			d           = Character('d'),
			e           = Character('e'),
			E           = Character('E'),
			f           = Character('f'),
			F           = Character('F'),
			g           = Character('g'),
			G           = Character('G'),
			h           = Character('h'),
			i           = Character('i'),
			I           = Character('I'),
			j           = Character('j'),
			l           = Character('l'),
			L           = Character('L'),
			m           = Character('m'),
			n           = Character('n'),
			o           = Character('o'),
			p           = Character('p'),
			s           = Character('s'),
			S           = Character('S'),
			t           = Character('t'),
			u           = Character('u'),
			w           = Character('w'),
			x           = Character('x'),
			X           = Character('X'),
			z           = Character('z')
		};
};

template <>
class format_chars<wchar_t>
{
public:
	typedef wchar_t char_type;
	enum : wchar_t {
			nul         = L'\0',
			space       = L' ',
			point       = L'.',
			percent     = L'%',
			dollar      = L'$',
			hash        = L'#',
			minus       = L'-',
			plus        = L'+',
			asterisk    = L'*',
			quote       = L'\'',
			zero        = L'0',
			nine        = L'9',
			a           = L'a',
			A           = L'A',
			c           = L'c',
			C           = L'C',
			d           = L'd',
			e           = L'e',
			E           = L'E',
			f           = L'f',
			F           = L'F',
			g           = L'g',
			G           = L'G',
			h           = L'h',
			i           = L'i',
			I           = L'I',
			j           = L'j',
			l           = L'l',
			L           = L'L',
			m           = L'm',
			n           = L'n',
			o           = L'o',
			p           = L'p',
			s           = L's',
			S           = L'S',
			t           = L't',
			u           = L'u',
			w           = L'w',
			x           = L'x',
			X           = L'X',
			z           = L'z'
		};
};


//**************************************************************************
//  FORMAT SPECIFIER ENCAPSULATION
//**************************************************************************

class format_flags
{
public:
	enum class positive_sign {
			none,
			space,                      // ' '
			plus                        // +
		};

	enum class length {
			unspecified,
			character,                  // hh
			short_integer,              // h
			long_integer,               // l
			long_long_integer,          // ll
			long_double,                // L
			integer_maximum,            // j
			size_type,                  // z, I
			pointer_difference,         // t
			integer_32,                 // I32
			integer_64,                 // I64
			wide_character              // w
		};

	enum class conversion {
			unspecified,
			signed_decimal,             // i, d
			unsigned_decimal,           // u
			octal,                      // o
			hexadecimal,                // x, X
			scientific_decimal,         // e, E
			fixed_decimal,              // f, F
			floating_decimal,           // g, G
			scientific_hexadecimal,     // a, A
			character,                  // c, C
			string,                     // s, S
			pointer,                    // p
			tell,                       // n
			strerror,                   // m
			percent                     // %
		};

	format_flags()
		: m_alternate_format(false)
		, m_zero_pad(false)
		, m_left_align(false)
		, m_positive_sign(positive_sign::none)
		, m_digit_grouping(false)
		, m_alternate_digits(false)
		, m_field_width(0)
		, m_precision(-1)
		, m_length(length::unspecified)
		, m_uppercase(false)
		, m_conversion(conversion::unspecified)
	{
	}

	template <typename Stream> void apply(Stream &stream) const
	{
		typedef format_chars<typename Stream::char_type> chars;

		stream.unsetf(
				Stream::basefield |
				Stream::adjustfield |
				Stream::floatfield |
				Stream::boolalpha |
				Stream::showbase |
				Stream::showpoint |
				Stream::showpos |
				Stream::uppercase);

		if (get_alternate_format()) stream.setf(Stream::showbase | Stream::showpoint);
		stream.fill(get_zero_pad() ? chars::zero : chars::space);
		stream.setf(get_left_align() ? Stream::left : get_zero_pad() ? Stream::internal : Stream::right);
		if (positive_sign::plus == get_positive_sign()) stream.setf(Stream::showpos);
		stream.precision((get_precision() < 0) ? 6 : get_precision());
		stream.width(get_field_width());
		if (get_uppercase()) stream.setf(Stream::uppercase);
		switch (get_conversion())
		{
		case conversion::unspecified:
			break;
		case conversion::signed_decimal:
		case conversion::unsigned_decimal:
			stream.setf(Stream::dec);
			break;
		case conversion::octal:
			stream.setf(Stream::oct);
			break;
		case conversion::hexadecimal:
			stream.setf(Stream::hex | Stream::scientific | Stream::fixed);
			break;
		case conversion::scientific_decimal:
			stream.setf(Stream::dec | Stream::scientific);
			break;
		case conversion::fixed_decimal:
			stream.setf(Stream::dec | Stream::fixed);
			break;
		case conversion::floating_decimal:
			stream.setf(Stream::dec);
			break;
		case conversion::scientific_hexadecimal:
			stream.setf(Stream::hex | Stream::scientific | Stream::fixed);
			break;
		case conversion::character:
		case conversion::string:
		case conversion::pointer:
		case conversion::tell:
		case conversion::strerror:
		case conversion::percent:
			break;
		}
	}

	bool            get_alternate_format() const    { return m_alternate_format; }
	bool            get_zero_pad() const            { return m_zero_pad; }
	bool            get_left_align() const          { return m_left_align; }
	positive_sign   get_positive_sign() const       { return m_positive_sign; }
	bool            get_digit_grouping() const      { return m_digit_grouping; }
	bool            get_alternate_digits() const    { return m_alternate_digits; }
	unsigned        get_field_width() const         { return m_field_width; }
	int             get_precision() const           { return m_precision; }
	length          get_length() const              { return m_length; }
	bool            get_uppercase() const           { return m_uppercase; }
	conversion      get_conversion() const          { return m_conversion; }

	void set_alternate_format()
	{
		m_alternate_format = true;
	}

	void set_zero_pad()
	{
		if (!m_left_align)
		{
			switch (m_conversion)
			{
			case conversion::signed_decimal:
			case conversion::unsigned_decimal:
			case conversion::octal:
			case conversion::hexadecimal:
				m_zero_pad = (0 > m_precision);
				break;
			default:
				m_zero_pad = true;
			}
		}
	}

	void set_left_align()
	{
		m_zero_pad = false;
		m_left_align = true;
	}

	void set_positive_sign_space()
	{
		switch (m_conversion)
		{
		case conversion::unsigned_decimal:
		case conversion::octal:
		case conversion::hexadecimal:
			break;
		default:
			if (positive_sign::plus != m_positive_sign)
				m_positive_sign = positive_sign::space;
		}
	}

	void set_positive_sign_plus()
	{
		switch (m_conversion)
		{
		case conversion::unsigned_decimal:
		case conversion::octal:
		case conversion::hexadecimal:
			break;
		default:
			m_positive_sign = positive_sign::plus;
		}
	}

	void set_digit_grouping()
	{
		m_digit_grouping = true;
	}

	void set_alternate_digits()
	{
		m_alternate_digits = true;
	}

	void set_field_width(int value)
	{
		if (0 > value)
		{
			set_left_align();
			m_field_width = unsigned(-value);
		}
		else
		{
			m_field_width = unsigned(value);
		}
	}

	void set_precision(int value)
	{
		m_precision = value;
		if (0 <= value)
		{
			switch (m_conversion)
			{
			case conversion::signed_decimal:
			case conversion::unsigned_decimal:
			case conversion::octal:
			case conversion::hexadecimal:
				m_zero_pad = false;
				break;
			default:
				break;
			}
		}
	}

	void set_length(length value)
	{
		m_length = value;
	}

	void set_uppercase()
	{
		m_uppercase = true;
	}

	void set_conversion(conversion value)
	{
		m_conversion = value;
		switch (value)
		{
		case conversion::unsigned_decimal:
		case conversion::octal:
		case conversion::hexadecimal:
			m_positive_sign = positive_sign::none;
		case conversion::signed_decimal:
			if (0 <= m_precision)
				m_zero_pad = false;
			break;
		default:
			break;
		}
	}

private:
	bool            m_alternate_format; // #
	bool            m_zero_pad;         // 0
	bool            m_left_align;       // -
	positive_sign   m_positive_sign;    // ' ', +
	bool            m_digit_grouping;   // '
	bool            m_alternate_digits; // I
	unsigned        m_field_width;
	int             m_precision;        // .
	length          m_length;           // hh, h, l, ll, L, j, z, I, t, w
	bool            m_uppercase;        // X, E, F, G, A
	conversion      m_conversion;       // i, d, u, o, x, X, e, E, f, F, g, G, a, A, c, C, s, S, p, m, %
};


//**************************************************************************
//  FORMAT OUTPUT HELPERS
//**************************************************************************

template <typename Stream, typename T>
class format_output
{
private:
	template <typename U> struct signed_integer_semantics
	{ static constexpr bool value = std::is_integral<U>::value && std::is_signed<U>::value; };
	template <typename U> struct unsigned_integer_semantics
	{ static constexpr bool value = std::is_integral<U>::value && !std::is_signed<U>::value; };
	template <typename U> struct default_semantics
	{ static constexpr bool value = !signed_integer_semantics<U>::value && !unsigned_integer_semantics<U>::value; };

public:
	template <typename U>
	static void apply(std::enable_if_t<signed_integer_semantics<U>::value, Stream> &str, format_flags const &flags, U const &value)
	{
		switch (flags.get_conversion())
		{
		case format_flags::conversion::signed_decimal:
			switch (flags.get_length())
			{
			case format_flags::length::character:
				str << int(static_cast<signed char>(value));
				break;
			case format_flags::length::short_integer:
				str << short(value);
				break;
			case format_flags::length::long_integer:
				str << long(value);
				break;
			case format_flags::length::long_long_integer:
				str << static_cast<long long>(value);
				break;
			case format_flags::length::integer_maximum:
				str << std::intmax_t(value);
				break;
			case format_flags::length::size_type:
				str << std::make_signed_t<std::size_t>(value);
				break;
			case format_flags::length::pointer_difference:
				str << std::make_signed_t<std::ptrdiff_t>(value);
				break;
			case format_flags::length::integer_32:
				str << std::uint32_t(std::int32_t(value));
				break;
			case format_flags::length::integer_64:
				str << std::int64_t(value);
				break;
			default:
				if (std::is_same<std::make_signed_t<U>, std::make_signed_t<char> >::value)
					str << int(value);
				else
					str << value;
			}
			break;
		case format_flags::conversion::unsigned_decimal:
		case format_flags::conversion::octal:
		case format_flags::conversion::hexadecimal:
			switch (flags.get_length())
			{
			case format_flags::length::character:
				str << unsigned(static_cast<unsigned char>(static_cast<signed char>(value)));
				break;
			case format_flags::length::short_integer:
				str << static_cast<unsigned short>(short(value));
				break;
			case format_flags::length::long_integer:
				str << static_cast<unsigned long>(long(value));
				break;
			case format_flags::length::long_long_integer:
				str << static_cast<unsigned long long>(static_cast<long long>(value));
				break;
			case format_flags::length::integer_maximum:
				str << std::uintmax_t(std::intmax_t(value));
				break;
			case format_flags::length::size_type:
				str << std::make_unsigned_t<std::size_t>(std::make_signed_t<std::size_t>(value));
				break;
			case format_flags::length::pointer_difference:
				str << std::make_unsigned_t<std::ptrdiff_t>(std::make_signed_t<std::ptrdiff_t>(value));
				break;
			case format_flags::length::integer_32:
				str << std::uint32_t(std::int32_t(value));
				break;
			case format_flags::length::integer_64:
				str << std::uint64_t(std::int64_t(value));
				break;
			default:
				if (std::is_same<std::make_unsigned_t<U>, std::make_unsigned_t<char> >::value)
					str << unsigned(std::make_unsigned_t<U>(value));
				else
					str << std::make_unsigned_t<U>(value);
			}
			break;
		case format_flags::conversion::character:
			if (std::is_signed<typename Stream::char_type>::value)
				str << typename Stream::char_type(value);
			else
				str << typename Stream::char_type(std::make_signed_t<typename Stream::char_type>(value));
			break;
		case format_flags::conversion::pointer:
			str << reinterpret_cast<void const *>(std::uintptr_t(std::intptr_t(value)));
			break;
		default:
			str << value;
		}
	}
	template <typename U>
	static void apply(std::enable_if_t<unsigned_integer_semantics<U>::value, Stream> &str, format_flags const &flags, U const &value)
	{
		switch (flags.get_conversion())
		{
		case format_flags::conversion::signed_decimal:
			switch (flags.get_length())
			{
			case format_flags::length::character:
				str << int(static_cast<signed char>(static_cast<unsigned char>(value)));
				break;
			case format_flags::length::short_integer:
				str << short(static_cast<unsigned short>(value));
				break;
			case format_flags::length::long_integer:
				str << long(static_cast<unsigned long>(value));
				break;
			case format_flags::length::long_long_integer:
				str << static_cast<long long>(static_cast<unsigned long long>(value));
				break;
			case format_flags::length::integer_maximum:
				str << std::intmax_t(std::uintmax_t(value));
				break;
			case format_flags::length::size_type:
				str << std::make_signed_t<std::size_t>(std::make_unsigned_t<std::size_t>(value));
				break;
			case format_flags::length::pointer_difference:
				str << std::make_signed_t<std::ptrdiff_t>(std::make_unsigned_t<std::ptrdiff_t>(value));
				break;
			case format_flags::length::integer_32:
				str << std::int32_t(std::uint32_t(value));
				break;
			case format_flags::length::integer_64:
				str << std::int64_t(std::uint64_t(value));
				break;
			default:
				if (std::is_same<std::make_signed_t<U>, std::make_signed_t<char> >::value)
					str << int(std::make_signed_t<U>(value));
				else
					str << std::make_signed_t<U>(value);
			}
			break;
		case format_flags::conversion::unsigned_decimal:
		case format_flags::conversion::octal:
		case format_flags::conversion::hexadecimal:
			switch (flags.get_length())
			{
			case format_flags::length::character:
				str << unsigned(static_cast<unsigned char>(value));
				break;
			case format_flags::length::short_integer:
				str << static_cast<unsigned short>(value);
				break;
			case format_flags::length::long_integer:
				str << static_cast<unsigned long>(value);
				break;
			case format_flags::length::long_long_integer:
				str << static_cast<unsigned long long>(value);
				break;
			case format_flags::length::integer_maximum:
				str << std::uintmax_t(value);
				break;
			case format_flags::length::size_type:
				str << std::make_unsigned_t<std::size_t>(value);
				break;
			case format_flags::length::pointer_difference:
				str << std::make_unsigned_t<std::ptrdiff_t>(value);
				break;
			case format_flags::length::integer_32:
				str << std::uint32_t(std::int32_t(value));
				break;
			case format_flags::length::integer_64:
				str << std::int64_t(value);
				break;
			default:
				if (std::is_same<std::make_unsigned_t<U>, std::make_unsigned_t<char> >::value)
					str << unsigned(value);
				else
					str << value;
			}
			break;
		case format_flags::conversion::character:
			if (std::is_signed<typename Stream::char_type>::value)
				str << typename Stream::char_type(value);
			else
				str << typename Stream::char_type(std::make_signed_t<typename Stream::char_type>(value));
			break;
		case format_flags::conversion::pointer:
			str << reinterpret_cast<void const *>(std::uintptr_t(value));
			break;
		default:
			str << value;
		}
	}
	template <typename U>
	static void apply(std::enable_if_t<default_semantics<U>::value, Stream> &str, format_flags const &flags, U const &value)
	{
		str << value;
	}
	static void apply(Stream &str, format_flags const &flags, bool value)
	{
		switch (flags.get_conversion())
		{
		case format_flags::conversion::signed_decimal:
		case format_flags::conversion::unsigned_decimal:
		case format_flags::conversion::octal:
		case format_flags::conversion::hexadecimal:
		case format_flags::conversion::scientific_decimal:
		case format_flags::conversion::fixed_decimal:
		case format_flags::conversion::floating_decimal:
		case format_flags::conversion::scientific_hexadecimal:
		case format_flags::conversion::character:
		case format_flags::conversion::pointer:
			apply(str, flags, unsigned(value));
			break;
		default:
			if (flags.get_alternate_format()) str.setf(Stream::boolalpha);
			str << value;
		}
	}
	template <typename CharT, typename Traits, typename Allocator>
	static void apply(std::enable_if_t<std::is_same<CharT, typename Stream::char_type>::value, Stream> &str, format_flags const &flags, std::basic_string<CharT, Traits, Allocator> const &value)
	{
		int const precision(flags.get_precision());
		if ((0 <= precision) && (value.size() > unsigned(precision)))
		{
			unsigned width(flags.get_field_width());
			bool const pad(unsigned(precision) < width);
			typename Stream::fmtflags const adjust(str.flags() & Stream::adjustfield);
			if (!pad || (Stream::left == adjust)) str.write(&*value.begin(), unsigned(precision));
			if (pad)
			{
				for (width -= precision; 0U < width; --width) str.put(str.fill());
				if (Stream::left != adjust) str.write(&*value.begin(), unsigned(precision));
			}
			str.width(0);
		}
		else
		{
			str << value;
		}
	}
	template <typename CharT, typename Traits, typename Allocator>
	static void apply(std::enable_if_t<!std::is_same<CharT, typename Stream::char_type>::value, Stream> &str, format_flags const &flags, std::basic_string<CharT, Traits, Allocator> const &value)
	{
		int const precision(flags.get_precision());
		if ((0 <= precision) && (value.size() > unsigned(precision)))
			str << value.substr(0, unsigned(precision));
		else
			str << value;
	}
};

template <typename Stream, typename T>
class format_output<Stream, T *>
{
protected:
	template <typename U> struct string_semantics
	{ static constexpr bool value = std::is_same<std::remove_const_t<U>, typename Stream::char_type>::value; };

public:
	template <typename U>
	static void apply(std::enable_if_t<string_semantics<U>::value, Stream> &str, format_flags const &flags, U const *value)
	{
		switch (flags.get_conversion())
		{
		case format_flags::conversion::string:
			{
				int precision(flags.get_precision());
				if (0 <= flags.get_precision())
				{
					std::streamsize cnt(0);
					for ( ; (0 < precision) && (U(format_chars<U>::nul) != value[cnt]); --precision, ++cnt) { }
					unsigned width(flags.get_field_width());
					bool const pad(std::make_unsigned_t<std::streamsize>(cnt) < width);
					typename Stream::fmtflags const adjust(str.flags() & Stream::adjustfield);
					if (!pad || (Stream::left == adjust)) str.write(value, cnt);
					if (pad)
					{
						for (width -= cnt; 0U < width; --width) str.put(str.fill());
						if (Stream::left != adjust) str.write(value, cnt);
					}
					str.width(0);
				}
				else
				{
					str << value;
				}
			}
			break;
		case format_flags::conversion::pointer:
			str << reinterpret_cast<void const *>(const_cast<std::remove_volatile_t<U> *>(value));
			break;
		default:
			str << value;
		}
	}
	template <typename U>
	static void apply(std::enable_if_t<!string_semantics<U>::value, Stream> &str, format_flags const &flags, U const *value)
	{
		str << reinterpret_cast<void const *>(const_cast<std::remove_volatile_t<U> *>(value));
	}
};

template <typename Stream, typename T, std::size_t N>
class format_output<Stream, T[N]> : protected format_output<Stream, T *>
{
public:
	template <typename U>
	static void apply(Stream &str, format_flags const &flags, U const *value)
	{
		static_assert(
				!format_output::template string_semantics<U>::value || (N <= size_t(unsigned((std::numeric_limits<int>::max)()))),
				"C string array length must not exceed maximum integer value");
		format_flags f(flags);
		if (format_output::template string_semantics<U>::value && ((0 > f.get_precision()) || (N < unsigned(f.get_precision()))))
			f.set_precision(int(unsigned(N)));
		format_output<Stream, T *>::apply(str, f, value);
	}
};


//**************************************************************************
//  INTEGER INPUT HELPERS
//**************************************************************************

template <typename T>
class format_make_integer
{
private:
	template <typename U> struct use_unsigned_cast
	{ static constexpr bool value = std::is_convertible<U const, unsigned>::value && std::is_unsigned<U>::value; };
	template <typename U> struct use_signed_cast
	{ static constexpr bool value = !use_unsigned_cast<U>::value && std::is_convertible<U const, int>::value; };
	template <typename U> struct disable
	{ static constexpr bool value = !use_unsigned_cast<U>::value && !use_signed_cast<U>::value; };

public:
	template <typename U> static std::enable_if_t<use_unsigned_cast<U>::value, bool> apply(U const &value, int &result)
	{
		result = int(unsigned(value));
		return true;
	}
	template <typename U> static std::enable_if_t<use_signed_cast<U>::value, bool> apply(U const &value, int &result)
	{
		result = int(value);
		return true;
	}
	template <typename U> static std::enable_if_t<disable<U>::value, bool> apply(U const &value, int &result)
	{
		return false;
	}
};


//**************************************************************************
//  INTEGER OUTPUT HELPERS
//**************************************************************************

template <typename T>
class format_store_integer
{
private:
	template <typename U> struct is_non_const_ptr
	{ static constexpr bool value = std::is_pointer<U>::value && !std::is_const<std::remove_pointer_t<U> >::value; };
	template <typename U> struct is_unsigned_ptr
	{ static constexpr bool value = std::is_pointer<U>::value && std::is_unsigned<std::remove_pointer_t<U> >::value; };
	template <typename U> struct use_unsigned_cast
	{ static constexpr bool value = is_non_const_ptr<U>::value && is_unsigned_ptr<U>::value && std::is_convertible<std::make_unsigned_t<std::streamoff>, std::remove_pointer_t<U> >::value; };
	template <typename U> struct use_signed_cast
	{ static constexpr bool value = is_non_const_ptr<U>::value && !use_unsigned_cast<U>::value && std::is_convertible<std::streamoff, std::remove_pointer_t<U> >::value; };
	template <typename U> struct disable
	{ static constexpr bool value = !use_unsigned_cast<U>::value && !use_signed_cast<U>::value; };

public:
	template <typename U> static std::enable_if_t<use_unsigned_cast<U>::value, bool> apply(U const &value, std::streamoff data)
	{
		*value = std::remove_pointer_t<U>(std::make_unsigned_t<std::streamoff>(data));
		return true;
	}
	template <typename U> static std::enable_if_t<use_signed_cast<U>::value, bool> apply(U const &value, std::streamoff data)
	{
		*value = std::remove_pointer_t<U>(std::make_signed_t<std::streamoff>(data));
		return true;
	}
	template <typename U> static std::enable_if_t<disable<U>::value, bool> apply(U const &value, std::streamoff data)
	{
		assert(false); // inappropriate type for storing characters written so far
		return false;
	}
};


//**************************************************************************
//  NON-POLYMORPHIC ARGUMENT WRAPPER
//**************************************************************************

template <typename Stream>
class format_argument
{
public:
	format_argument()
		: m_value(nullptr)
		, m_output_function(nullptr)
		, m_make_integer_function(nullptr)
		, m_store_integer_function(nullptr)
	{
	}

	template <typename T>
	format_argument(T const &value)
		: m_value(reinterpret_cast<void const *>(&value))
		, m_output_function(&static_output<T>)
		, m_make_integer_function(&static_make_integer<T>)
		, m_store_integer_function(&static_store_integer<T>)
	{
	}

	void output(Stream &str, format_flags const &flags) const { m_output_function(str, flags, m_value); }
	bool make_integer(int &result) const { return m_make_integer_function(m_value, result); }
	void store_integer(std::streamoff data) const { m_store_integer_function(m_value, data); }

private:
	typedef void (*output_function)(Stream &str, format_flags const &flags, void const *value);
	typedef bool (*make_integer_function)(void const *value, int &result);
	typedef void (*store_integer_function)(void const *value, std::streamoff data);

	template <typename T> static void static_output(Stream &str, format_flags const &flags, void const *value)
	{
		format_output<Stream, T>::apply(str, flags, *reinterpret_cast<T const *>(value));
	}

	template <typename T> static bool static_make_integer(void const *value, int &result)
	{
		return format_make_integer<T>::apply(*reinterpret_cast<T const *>(value), result);
	}

	template <typename T> static void static_store_integer(void const *value, std::streamoff data)
	{
		format_store_integer<T>::apply(*reinterpret_cast<T const *>(value), data);
	}

	void const              *m_value;
	output_function         m_output_function;
	make_integer_function   m_make_integer_function;
	store_integer_function  m_store_integer_function;
};


//**************************************************************************
//  NON-POLYMORPHIC ARGUMENT PACK WRAPPER BASE
//**************************************************************************

template <typename Stream = std::ostream>
class format_argument_pack
{
public:
	typedef typename Stream::char_type char_type;
	typedef char_type const *iterator;
	iterator format_begin() const
	{
		return m_begin;
	}
	bool format_at_end(iterator it) const
	{
		return (m_end && (m_end == it)) || (m_check_nul && (format_chars<char_type>::nul == *it));
	}
	std::size_t argument_count() const
	{
		return m_argument_count;
	}
	format_argument<Stream> const &operator[](std::size_t index) const
	{
		assert(m_argument_count > index);
		return m_arguments[index];
	}

protected:
	template <typename T>
	struct handle_char_ptr { static constexpr bool value = std::is_pointer<T>::value && std::is_same<std::remove_cv_t<std::remove_pointer_t<T> >, char_type>::value; };
	template <typename T>
	struct handle_char_array { static constexpr bool value = std::is_array<T>::value && std::is_same<std::remove_cv_t<std::remove_extent_t<T> >, char_type>::value; };
	template <typename T>
	struct handle_container { static constexpr bool value = !handle_char_ptr<T>::value && !handle_char_array<T>::value; };

	template <typename Format>
	format_argument_pack(
			Format &&fmt,
			format_argument<Stream> const *arguments,
			std::enable_if_t<handle_char_ptr<std::remove_reference_t<Format> >::value, std::size_t> argument_count)
		: m_begin(fmt)
		, m_end(nullptr)
		, m_check_nul(true)
		, m_arguments(arguments)
		, m_argument_count(argument_count)
	{
		assert(m_begin);
		assert(m_end || m_check_nul);
		assert(!m_end || (m_end > m_begin));
		assert(m_arguments || !m_argument_count);
	}
	template <typename Format>
	format_argument_pack(
			Format &&fmt,
			format_argument<Stream> const *arguments,
			std::enable_if_t<handle_char_array<std::remove_reference_t<Format> >::value, std::size_t> argument_count)
		: m_begin(std::cbegin(fmt))
		, m_end(std::cend(fmt))
		, m_check_nul(true)
		, m_arguments(arguments)
		, m_argument_count(argument_count)
	{
		assert(m_begin);
		assert(m_end || m_check_nul);
		assert(!m_end || (m_end > m_begin));
		assert(m_arguments || !m_argument_count);
	}
	template <typename Format>
	format_argument_pack(
			Format &&fmt,
			format_argument<Stream> const *arguments,
			std::enable_if_t<handle_container<std::remove_reference_t<Format> >::value, std::size_t> argument_count)
		: m_begin(fmt.empty() ? nullptr : &*std::cbegin(fmt))
		, m_end(fmt.empty() ? nullptr : (m_begin + std::distance(std::cbegin(fmt), std::cend(fmt))))
		, m_check_nul(true)
		, m_arguments(arguments)
		, m_argument_count(argument_count)
	{
		assert(m_begin);
		assert(m_end || m_check_nul);
		assert(!m_end || (m_end > m_begin));
		assert(m_arguments || !m_argument_count);
	}

	format_argument_pack(format_argument_pack<Stream> const &) = default;
	format_argument_pack(format_argument_pack<Stream> &&) = default;
	format_argument_pack &operator=(format_argument_pack<Stream> const &) = default;
	format_argument_pack &operator=(format_argument_pack<Stream> &&) = default;

private:
	iterator                        m_begin;
	iterator                        m_end;
	bool                            m_check_nul;
	format_argument<Stream> const   *m_arguments;
	std::size_t                     m_argument_count;
};


//**************************************************************************
//  ARGUMENT PACK WRAPPER IMPLEMENTATION
//**************************************************************************

template <typename Stream, std::size_t Count>
class format_argument_pack_impl
	: private std::array<format_argument<Stream>, Count>
	, public format_argument_pack<Stream>
{
public:
	using typename format_argument_pack<Stream>::iterator;
	using format_argument_pack<Stream>::operator[];

	template <typename Format, typename... Params>
	format_argument_pack_impl(Format &&fmt, Params &&... args)
		: std::array<format_argument<Stream>, Count>({ { format_argument<Stream>(std::forward<Params>(args))... } })
		, format_argument_pack<Stream>(std::forward<Format>(fmt), Count ? &*this->cbegin() : nullptr, Count)
	{
		static_assert(sizeof...(Params) == Count, "Wrong number of constructor arguments");
	}

	format_argument_pack_impl(format_argument_pack_impl<Stream, Count> const &) = default;
	format_argument_pack_impl(format_argument_pack_impl<Stream, Count> &&) = default;
	format_argument_pack_impl &operator=(format_argument_pack_impl<Stream, Count> const &) = default;
	format_argument_pack_impl &operator=(format_argument_pack_impl<Stream, Count> &&) = default;
};


//**************************************************************************
//  ARGUMENT PACK CREATOR FUNCTION
//**************************************************************************

template <typename Stream = std::ostream, typename Format, typename... Params>
inline format_argument_pack_impl<Stream, sizeof...(Params)> make_format_argument_pack(Format &&fmt, Params &&... args)
{
	return format_argument_pack_impl<Stream, sizeof...(Params)>(std::forward<Format>(fmt), std::forward<Params>(args)...);
}


//**************************************************************************
//  FORMAT STRING PARSING HELPER
//**************************************************************************

template <typename Format>
class format_helper : public format_chars<typename Format::char_type>
{
public:
	static bool parse_format(
			Format const &fmt,
			typename Format::iterator &it,
			format_flags &flags,
			int &next_position,
			int &argument_position,
			int &width_position,
			int &precision_position)
	{
		static_assert((format_helper::nine - format_helper::zero) == 9, "Digits must be contiguous");
		assert(!fmt.format_at_end(it));
		assert(format_helper::percent == *it);

		int num;
		int nxt(next_position);
		++it;
		flags = format_flags();
		argument_position = -1;
		width_position = -1;
		precision_position = -1;

		// Leading zeroes are tricky - they could be a zero-pad flag or part of a position specifier
		bool const leading_zero(!fmt.format_at_end(it) && (format_helper::zero == *it));
		while (!fmt.format_at_end(it) && (format_helper::zero == *it)) ++it;

		// Digits encountered at this point could be a field width or a position specifier
		num = 0;
		bool leading_num(have_digit(fmt, it));
		while (have_digit(fmt, it)) add_digit(num, *it++);
		if (leading_num && !have_dollar(fmt, it))
		{
			// No dollar sign, leading number is field width
			if (leading_zero) flags.set_zero_pad();
			flags.set_field_width(num);
		}
		else
		{
			// If we hit a dollar sign after a number, that's a position specifier
			if ((leading_zero || leading_num) && have_dollar(fmt, it))
			{
				argument_position = num;
				++it;
			}
			else if (leading_zero)
			{
				flags.set_zero_pad();
			}

			// Parse flag characters
			while (!fmt.format_at_end(it))
			{
				switch (*it)
				{
				case format_helper::hash:   ++it;   flags.set_alternate_format();       continue;
				case format_helper::zero:   ++it;   flags.set_zero_pad();               continue;
				case format_helper::minus:  ++it;   flags.set_left_align();             continue;
				case format_helper::space:  ++it;   flags.set_positive_sign_space();    continue;
				case format_helper::plus:   ++it;   flags.set_positive_sign_plus();     continue;
				case format_helper::quote:  ++it;   flags.set_digit_grouping();         continue;
				case format_helper::I:      ++it;   flags.set_alternate_digits();       continue;
				default: break;
				}
				break;
			}

			// Check for literal or parameterised field width
			if (!fmt.format_at_end(it))
			{
				if (is_digit(*it))
				{
					flags.set_field_width(read_number(fmt, it));
				}
				else if (format_helper::asterisk == *it)
				{
					++it;
					if (have_digit(fmt, it))
					{
						num = read_number(fmt, it);
						assert(have_dollar(fmt, it)); // invalid positional width
						if (!have_dollar(fmt, it)) return false;
						width_position = num;
						nxt = width_position + 1;
						++it;
					}
					else
					{
						width_position = nxt++;
					}
				}
			}
		}

		// Check for literal or parameterised precision
		if (!fmt.format_at_end(it) && (*it == format_helper::point))
		{
			++it;
			if (have_digit(fmt, it))
			{
				flags.set_precision(read_number(fmt, it));
			}
			else if (!fmt.format_at_end(it) && (format_helper::asterisk == *it))
			{
				++it;
				if (have_digit(fmt, it))
				{
					num = read_number(fmt, it);
					assert(have_dollar(fmt, it)); // invalid positional precision
					if (!have_dollar(fmt, it)) return false;
					precision_position = num;
					nxt = precision_position + 1;
					++it;
				}
				else
				{
					precision_position = nxt++;
				}
			}
			else
			{
				flags.set_precision(0);
			}
		}

		// Check for length modifiers
		if (!fmt.format_at_end(it)) switch (*it)
		{
		case format_helper::h:
			++it;
			if (!fmt.format_at_end(it) && (format_helper::h == *it))
			{
				++it;
				flags.set_length(format_flags::length::character);
			}
			else
			{
				flags.set_length(format_flags::length::short_integer);
			}
			break;
		case format_helper::l:
			++it;
			if (!fmt.format_at_end(it) && (format_helper::l == *it))
			{
				++it;
				flags.set_length(format_flags::length::long_long_integer);
			}
			else
			{
				flags.set_length(format_flags::length::long_integer);
			}
			break;
		case format_helper::L:
			++it;
			flags.set_length(format_flags::length::long_double);
			break;
		case format_helper::j:
			++it;
			flags.set_length(format_flags::length::integer_maximum);
			break;
		case format_helper::z:
			++it;
			flags.set_length(format_flags::length::size_type);
			break;
		case format_helper::t:
			++it;
			flags.set_length(format_flags::length::pointer_difference);
			break;
		case format_helper::I:
			{
				++it;
				format_flags::length length = format_flags::length::size_type;
				if (!fmt.format_at_end(it))
				{
					if ((typename format_helper::char_type(format_helper::zero) + 3) == *it)
					{
						typename Format::iterator tmp(it);
						++tmp;
						if (!fmt.format_at_end(tmp) && ((typename format_helper::char_type(format_helper::zero) + 2) == *tmp))
						{
							length = format_flags::length::integer_32;
							it = ++tmp;
						}
					}
					else if ((typename format_helper::char_type(format_helper::zero) + 6) == *it)
					{
						typename Format::iterator tmp(it);
						++tmp;
						if (!fmt.format_at_end(tmp) && ((typename format_helper::char_type(format_helper::zero) + 4) == *tmp))
						{
							length = format_flags::length::integer_64;
							it = ++tmp;
						}
					}
				}
				flags.set_length(length);
			}
			break;
		case format_helper::w:
			++it;
			flags.set_length(format_flags::length::wide_character);
			break;
		default:
			break;
		}

		// Now we should find a conversion specifier
		assert(!fmt.format_at_end(it)); // missing conversion
		if (fmt.format_at_end(it)) return false;
		switch (*it)
		{
		case format_helper::d:
		case format_helper::i:
			flags.set_conversion(format_flags::conversion::signed_decimal);
			break;
		case format_helper::o:
			flags.set_conversion(format_flags::conversion::octal);
			break;
		case format_helper::u:
			flags.set_conversion(format_flags::conversion::unsigned_decimal);
			break;
		case format_helper::X:
			flags.set_uppercase();
		case format_helper::x:
			flags.set_conversion(format_flags::conversion::hexadecimal);
			break;
		case format_helper::E:
			flags.set_uppercase();
		case format_helper::e:
			flags.set_conversion(format_flags::conversion::scientific_decimal);
			break;
		case format_helper::F:
			flags.set_uppercase();
		case format_helper::f:
			flags.set_conversion(format_flags::conversion::fixed_decimal);
			break;
		case format_helper::G:
			flags.set_uppercase();
		case format_helper::g:
			flags.set_conversion(format_flags::conversion::floating_decimal);
			break;
		case format_helper::A:
			flags.set_uppercase();
		case format_helper::a:
			flags.set_conversion(format_flags::conversion::scientific_hexadecimal);
			break;
		case format_helper::C:
			if (format_flags::length::unspecified == flags.get_length())
				flags.set_length(format_flags::length::long_integer);
		case format_helper::c:
			flags.set_conversion(format_flags::conversion::character);
			break;
		case format_helper::S:
			if (format_flags::length::unspecified == flags.get_length())
				flags.set_length(format_flags::length::long_integer);
		case format_helper::s:
			flags.set_conversion(format_flags::conversion::string);
			break;
		case format_helper::p:
			flags.set_conversion(format_flags::conversion::pointer);
			break;
		case format_helper::n:
			flags.set_conversion(format_flags::conversion::tell);
			break;
		case format_helper::m:
			flags.set_conversion(format_flags::conversion::strerror);
			break;
		case format_helper::percent:
			flags.set_conversion(format_flags::conversion::percent);
			break;
		default:
			assert(false); // unsupported conversion
			return false;
		}
		++it;

		// Finalise argument position
		if (argument_position < 0) argument_position = nxt;
		next_position = argument_position;
		switch (flags.get_conversion())
		{
		case format_flags::conversion::strerror:
		case format_flags::conversion::percent:
			break;
		default:
			++next_position;
		}
		return true;
	}

private:
	static bool have_dollar(Format const &fmt, typename Format::iterator const &it)
	{
		return !fmt.format_at_end(it) && (*it == format_helper::dollar);
	}

	static bool have_digit(Format const &fmt, typename Format::iterator const &it)
	{
		return !fmt.format_at_end(it) && is_digit(*it);
	}

	static bool is_digit(typename format_helper::char_type value)
	{
		return (format_helper::zero <= value) && (format_helper::nine >= value);
	}

	static int digit_value(typename format_helper::char_type value)
	{
		assert(is_digit(value));
		return int(std::make_signed_t<decltype(value)>(value - format_helper::zero));
	}

	static void add_digit(int &num, typename format_helper::char_type digit)
	{
		num = (num * 10) + digit_value(digit);
	}

	static int read_number(Format const &fmt, typename Format::iterator &it)
	{
		assert(have_digit(fmt, it));
		int value = 0;
		do add_digit(value, *it++); while (have_digit(fmt, it));
		return value;
	}
};


//**************************************************************************
//  CORE FORMATTING FUNCTION
//**************************************************************************

template <typename Stream, typename Base>
typename Stream::off_type stream_format(Stream &str, format_argument_pack<Base> const &args)
{
	typedef format_helper<format_argument_pack<Base> > format_helper;
	typedef typename format_argument_pack<Base>::iterator iterator;
	class stream_preserver
	{
	public:
		stream_preserver(Stream &stream)
			: m_stream(stream)
			, m_fill(stream.fill())
			, m_flags(stream.flags())
			, m_precision(stream.precision())
			, m_width(stream.width())
		{
		}
		~stream_preserver()
		{
			m_stream.width(m_width);
			m_stream.precision(m_precision);
			m_stream.flags(m_flags);
			m_stream.fill(m_fill);
		}
	private:
		Stream                      &m_stream;
		typename Stream::char_type  m_fill;
		typename Stream::fmtflags   m_flags;
		std::streamsize             m_precision;
		std::streamsize             m_width;
	};

	typename Stream::pos_type const begin(str.tellp());
	stream_preserver const preserver(str);
	int next_pos(1);
	iterator start = args.format_begin();
	for (iterator it = start; !args.format_at_end(start); )
	{
		while (!args.format_at_end(it) && (format_helper::percent != *it)) ++it;
		if (start != it)
		{
			str.write(&*start, it - start);
			start = it;
		}
		if (!args.format_at_end(it))
		{
			// Try to parse a percent format specification
			format_flags flags;
			int arg_pos, width_pos, prec_pos;
			if (!format_helper::parse_format(args, it, flags, next_pos, arg_pos, width_pos, prec_pos))
				continue;

			// Handle parameterised width
			if (0 <= width_pos)
			{
				assert(flags.get_field_width() == 0U);
				assert(0 < width_pos);
				assert(args.argument_count() >= unsigned(width_pos));
				if ((0 < width_pos) && (args.argument_count() >= unsigned(width_pos)))
				{
					int width;
					if (args[width_pos - 1].make_integer(width))
					{
						if (0 > width)
						{
							flags.set_left_align();
							flags.set_field_width(unsigned(-width));
						}
						else
						{
							flags.set_field_width(unsigned(width));
						}
					}
					else
					{
						assert(false); // inappropriate type passed as width argument
					}
				}
			}

			// Handle parameterised precision
			if (0 <= prec_pos)
			{
				assert(flags.get_precision() < 0);
				assert(0 < prec_pos);
				assert(args.argument_count() >= unsigned(prec_pos));
				if ((0 < prec_pos) && (args.argument_count() >= unsigned(prec_pos)))
				{
					int precision;
					if (args[prec_pos - 1].make_integer(precision))
						flags.set_precision(precision);
					else
						assert(false); // inappropriate type passed as precision argument
				}
			}

			// Some conversions don't actually take an argument - get them out of the way
			flags.apply(str);
			if (format_flags::conversion::strerror == flags.get_conversion())
			{
				str << std::strerror(errno);
				start = it;
			}
			else if (format_flags::conversion::percent == flags.get_conversion())
			{
				str << typename Stream::char_type(format_chars<typename Stream::char_type>::percent);
				start = it;
			}
			else
			{
				assert(0 < arg_pos);
				assert(args.argument_count() >= unsigned(arg_pos));
				if ((0 >= arg_pos) || (args.argument_count() < unsigned(arg_pos)))
					continue;
				if (format_flags::conversion::tell == flags.get_conversion())
				{
					typename Stream::pos_type const current(str.tellp());
					args[arg_pos - 1].store_integer(
							((typename Stream::pos_type(-1) == begin) || (typename Stream::pos_type(-1) == current))
								? typename Stream::off_type(-1)
								: (current - begin));
				}
				else
				{
					args[arg_pos - 1].output(str, flags);
				}
				start = it;
			}
		}
	}
	typename Stream::pos_type const end(str.tellp());
	return ((typename Stream::pos_type(-1) == begin) || (typename Stream::pos_type(-1) == end))
			? typename Stream::off_type(-1)
			: (end - begin);
}

} // namespace detail


//**************************************************************************
//  FORMAT TO STREAM FUNCTIONS
//**************************************************************************

template <typename Stream, typename Format, typename... Params>
inline typename Stream::off_type stream_format(Stream &str, Format const &fmt, Params &&... args)
{
	return detail::stream_format(str, detail::make_format_argument_pack<Stream>(fmt, std::forward<Params>(args)...));
}

template <typename Stream, typename Base>
inline typename Stream::off_type stream_format(Stream &str, detail::format_argument_pack<Base> const &args)
{
	return detail::stream_format(str, args);
}

template <typename Stream, typename Base>
inline typename Stream::off_type stream_format(Stream &str, detail::format_argument_pack<Base> &&args)
{
	return detail::stream_format(str, args);
}


//**************************************************************************
//  FORMAT TO NEW STRING FUNCTIONS
//**************************************************************************

template <typename String = std::string, typename Format, typename... Params>
inline String string_format(Format &&fmt, Params &&... args)
{
	typedef std::basic_ostringstream<typename String::value_type, typename String::traits_type, typename String::allocator_type> ostream;
	ostream str;
	stream_format(str, fmt, std::forward<Params>(args)...);
	return str.str();
};

template <typename String = std::string, typename Format, typename... Params>
inline String string_format(std::locale const &locale, Format &&fmt, Params &&... args)
{
	typedef std::basic_ostringstream<typename String::value_type, typename String::traits_type, typename String::allocator_type> ostream;
	ostream str;
	str.imbue(locale);
	stream_format(str, fmt, std::forward<Params>(args)...);
	return str.str();
};

template <typename String = std::string, typename Stream>
inline String string_format(detail::format_argument_pack<Stream> const &args)
{
	typedef std::basic_ostringstream<typename String::value_type, typename String::traits_type, typename String::allocator_type> ostream;
	ostream str;
	detail::stream_format(str, args);
	return str.str();
};

template <typename String = std::string, typename Stream>
inline String string_format(detail::format_argument_pack<Stream> &&args)
{
	typedef std::basic_ostringstream<typename String::value_type, typename String::traits_type, typename String::allocator_type> ostream;
	ostream str;
	detail::stream_format(str, std::move(args));
	return str.str();
};

template <typename String = std::string, typename Stream>
inline String string_format(std::locale const &locale, detail::format_argument_pack<Stream> const &args)
{
	typedef std::basic_ostringstream<typename String::value_type, typename String::traits_type, typename String::allocator_type> ostream;
	ostream str;
	str.imbue(locale);
	detail::stream_format(str, args);
	return str.str();
};

template <typename String = std::string, typename Stream>
inline String string_format(std::locale const &locale, detail::format_argument_pack<Stream> &&args)
{
	typedef std::basic_ostringstream<typename String::value_type, typename String::traits_type, typename String::allocator_type> ostream;
	ostream str;
	str.imbue(locale);
	detail::stream_format(str, std::move(args));
	return str.str();
};


//**************************************************************************
//  CREATING ARGUMENT PACKS
//**************************************************************************

using detail::format_argument_pack;
using detail::make_format_argument_pack;

} // namespace util

using util::string_format;

#endif // __MAME_UTIL_STRFORMAT_H__
