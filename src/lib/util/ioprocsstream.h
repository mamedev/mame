// license:BSD-3-Clause
// copyright-holders:Vas Crabb
/***************************************************************************

    ioprocsstream.h

    C++ I/O stream wrappers for I/O interfaces

    These classes are preliminary and may change substantially.

***************************************************************************/
#ifndef MAME_LIB_UTIL_IOPROCSSTREAM_H
#define MAME_LIB_UTIL_IOPROCSSTREAM_H

#pragma once

#include "utilfwd.h"

#include <ostream>
#include <streambuf>
#include <utility>


namespace util {

class write_stream_streambuf : public std::streambuf
{
public:
	enum class utf8_id_t { tag };
	enum class utf16native_id_t { tag };
	enum class utf16swapped_id_t { tag };

	static inline constexpr auto UTF_8 = utf8_id_t::tag;
#ifdef LSB_FIRST
	static inline constexpr auto UTF_16BE = utf16swapped_id_t::tag;
	static inline constexpr auto UTF_16LE = utf16native_id_t::tag;
#else
	static inline constexpr auto UTF_16BE = utf16native_id_t::tag;
	static inline constexpr auto UTF_16LE = utf16swapped_id_t::tag;
#endif

	write_stream_streambuf(write_stream &stream, utf8_id_t = UTF_8, bool bom = true, bool nl = true);
	write_stream_streambuf(write_stream &stream, utf16native_id_t, bool bom = true, bool nl = true);
	write_stream_streambuf(write_stream &stream, utf16swapped_id_t, bool bom = true, bool nl = true);
	~write_stream_streambuf();

protected:
	virtual int sync() override;
	virtual int_type overflow(int_type ch = traits_type::eof()) override;

private:
	using sync_func = bool (*)(write_stream_streambuf &, bool, bool);
	class helper;

	write_stream &m_stream;
	sync_func m_sync_func;
	char_type *m_buffer;
	void *m_extrep;
	void *m_aux;
};


class owritestream : public std::ostream
{
public:
	static inline constexpr auto UTF_8 = write_stream_streambuf::UTF_8;
	static inline constexpr auto UTF_16BE = write_stream_streambuf::UTF_16BE;
	static inline constexpr auto UTF_16LE = write_stream_streambuf::UTF_16LE;

	template <typename... T> owritestream(T &&... args) : std::ostream(&m_streambuf), m_streambuf(std::forward<T>(args)...) { }

private:
	write_stream_streambuf m_streambuf;
};

} // namespace util

#endif // MAME_LIB_UTIL_IOPROCSSTREAM_H
