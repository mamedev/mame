// license:BSD-3-Clause
// copyright-holders:Vas Crabb
/***************************************************************************

    ioprocsstream.cpp

    C++ I/O stream wrappers for I/O interfaces

***************************************************************************/

#include "ioprocsstream.h"

#include "ioprocs.h"

#include <algorithm>
#include <bit>
#include <cassert>
#include <cstdlib>
#include <system_error>
#include <type_traits>


namespace util {

namespace {

constexpr size_t BUFFER_SIZE = 1024;

} // anonymous namespace


class write_stream_streambuf::helper
{
public:
	static_assert(sizeof(char_type) == 1U);
	static_assert(sizeof(char16_t) == 2U);

	template <bool Swap>
	static constexpr uint16_t encode_char(char_type ch)
	{
		return uint16_t(std::make_unsigned_t<char_type>(ch)) << (Swap ? 8 : 0);
	}

	static void allocate_utf16(write_stream_streambuf &o, bool swap, bool bom, bool nl)
	{
		// worst case is a buffer full of LF doubling in size when CR are inserted
		std::size_t const extrepsize(BUFFER_SIZE * ((nl && (3 == CRLF)) ? 2 : 1) * sizeof(char16_t));
		o.m_extrep = std::malloc(extrepsize + BUFFER_SIZE);
		if (o.m_extrep)
			o.m_buffer = reinterpret_cast<char_type *>(o.m_extrep) + extrepsize;
		else
			o.m_buffer = nullptr;
		if (!o.m_buffer)
			throw std::bad_alloc();

		// implicitly insert encoded U+FEFF in buffer if requested
		o.setg(nullptr, nullptr, nullptr);
		if (bom)
		{
			auto const cnv(reinterpret_cast<char16_t *>(o.m_extrep));
			*cnv = swap ? 0xfffe : 0xfeff;
			o.setp(o.m_buffer, o.m_buffer + (BUFFER_SIZE - 2));
			o.m_aux = cnv + 1;
		}
		else
		{
			o.setp(o.m_buffer, o.m_buffer + (BUFFER_SIZE - 1));
			o.m_aux = o.m_extrep;
		}
	}

	template <bool Swap, bool Newline>
	static bool sync_utf16(write_stream_streambuf &o, bool flush, bool term)
	{
		auto const end(o.pptr());
		auto ptr(o.pbase());
		auto buffered(end - ptr);
		if (!buffered && (!flush || (o.m_aux == o.m_extrep)))
			return true;

		auto extrep(reinterpret_cast<char *>(o.m_extrep));
		auto unwritten(reinterpret_cast<char *>(o.m_aux) - extrep);
		if (unwritten & 1U)
			++extrep;
		auto cnv(reinterpret_cast<char16_t *>(extrep + unwritten));
		while (ptr != end)
		{
			auto const ch(*ptr);
			if (Newline && (1 == CRLF) && ('\n' == ch))
			{
				*cnv++ = encode_char<Swap>('\r');
			}
			else if (Newline && (1 == CRLF) && ('\r' == ch))
			{
				*cnv++ = encode_char<Swap>('\n');
			}
			else if (Newline && (3 == CRLF) && ('\n' == ch))
			{
				*cnv++ = encode_char<Swap>('\r');
				*cnv++ = encode_char<Swap>('\n');
			}
			else
			{
				auto const prefix(std::countl_one(std::make_unsigned_t<char_type>(ch)));
				if (!prefix)
				{
					*cnv++ = encode_char<Swap>(ch);
				}
				else if ((2 <= prefix) && (4 >= prefix))
				{
					unsigned cont = prefix - 1;
					unsigned offs = 1U;
					uint32_t acc = ch & ((unsigned(1) << (7 - prefix)) - 1);
					while (cont && ((ptr + offs) != end))
					{
						auto const trail(ptr[offs]);
						if ((trail & 0xc0U) == 0x80U)
						{
							++offs;
							acc = (acc << 6) | (trail & 0x3fU);
							if (!--cont)
							{
								if (0xffffU >= acc)
								{
									*cnv++ = uint16_t(Swap ? ((acc << 8) | (acc >> 8)) : acc);
								}
								else if (0x10ffffU >= acc)
								{
									auto const h(0xd800U | ((acc - 0x10000) >> 10));
									auto const l(0xdc00U | ((acc - 0x10000) & 0x3ffU));
									*cnv++ = Swap ? uint16_t((h << 8) | (h >> 8)) : h;
									*cnv++ = Swap ? uint16_t((l << 8) | (l >> 8)) : l;
								}
								else
								{
									*cnv++ = Swap ? 0xfdff : 0xfffd;
								}
								ptr += offs;
							}
						}
						else
						{
							*cnv++ = Swap ? 0xfdff : 0xfffd;
							ptr += offs;
							cont = 0;
						}
					}
					if (cont)
					{
						if (!term)
						{
							break;
						}
						else
						{
							*cnv++ = Swap ? 0xfdff : 0xfffd;
							ptr += offs;
						}
					}
					continue;
				}
				else
				{
					*cnv++ = Swap ? 0xfdff : 0xfffd;
				}
			}
			++ptr;
		}
		unwritten = reinterpret_cast<char *>(cnv) - extrep;

		std::error_condition err;
		auto pos(extrep);
		do
		{
			std::size_t written;
			err = o.m_stream.write_some(pos, unwritten, written);
			pos += written;
			unwritten -= written;
		}
		while (unwritten && ((flush && !err) || (std::errc::interrupted == err)));

		extrep = reinterpret_cast<char *>(o.m_extrep);
		if (pos != extrep)
		{
			// keep unwritten transcoded content aligned
			if (unwritten & 1U)
				std::copy_n(extrep, unwritten + 1, pos - 1);
			else
				std::copy_n(extrep, unwritten, pos);
		}
		o.m_aux = extrep + unwritten;
		if (Newline && (3 == CRLF))
			o.setp(o.m_buffer, o.m_buffer + (BUFFER_SIZE - ((unwritten + 3) >> 2) - 1));
		else
			o.setp(o.m_buffer, o.m_buffer + (BUFFER_SIZE - ((unwritten + 1) >> 1) - 1));
		if (ptr != end)
		{
			std::copy(ptr, end, o.m_buffer);
			o.pbump(end - ptr);
		}

		return !err;
	}

	template <bool MacNewline>
	static bool sync_utf8(write_stream_streambuf &o, bool flush, bool term)
	{
		auto const end(o.pptr());
		auto ptr(o.pbase());
		auto buffered(end - ptr);
		if (!buffered)
			return true;
		if (MacNewline)
		{
			auto cnv(reinterpret_cast<char_type *>(o.m_aux));
			while (cnv < end)
			{
				if ('\n' == *cnv)
					*cnv = '\r';
				else if ('\r' == *cnv)
					*cnv = '\n';
				++cnv;
			}
			o.m_aux = cnv;
		}
		std::error_condition err;
		do
		{
			std::size_t written;
			err = o.m_stream.write_some(ptr, buffered, written);
			ptr += written;
			buffered -= written;
		}
		while (buffered && ((flush && !err) || (std::errc::interrupted == err)));
		if (o.pbase() != ptr)
		{
			o.setp(o.m_buffer, o.m_buffer + (BUFFER_SIZE - 1));
			if (buffered)
			{
				std::copy_n(ptr, buffered, o.pbase());
				o.pbump(buffered);
			}
			if (MacNewline)
				o.m_aux = o.pptr();
		}
		return !err;
	}

	static bool sync_utf8_crlf(write_stream_streambuf &o, bool flush, bool term)
	{
		auto const end(o.pptr());
		auto ptr(o.pbase());
		auto buffered(end - ptr);
		if (!buffered && (!flush || (o.m_aux == o.m_extrep)))
			return true;

		std::error_condition err;
		auto lf(std::find(ptr, end, '\n'));
		if ((lf == end) && (o.m_aux == o.m_extrep))
		{
			do
			{
				std::size_t written;
				err = o.m_stream.write_some(ptr, buffered, written);
				ptr += written;
				buffered -= written;
			}
			while (buffered && ((flush && !err) || (std::errc::interrupted == err)));
			if (o.pbase() != ptr)
			{
				o.setp(o.m_buffer, o.m_buffer + (BUFFER_SIZE - 1));
				if (buffered)
				{
					std::copy_n(ptr, buffered, o.pbase());
					o.pbump(buffered);
				}
			}
		}
		else
		{
			auto const extrep(reinterpret_cast<char_type *>(o.m_extrep));
			auto cnv(reinterpret_cast<char_type *>(o.m_aux));
			while (lf != end)
			{
				cnv = std::copy(ptr, lf, cnv);
				*cnv++ = '\r';
				ptr = lf;
				lf = std::find(ptr + 1, end, '\n');
			}
			cnv = std::copy(ptr, lf, cnv);
			ptr = extrep;
			auto translated(cnv - ptr);
			do
			{
				std::size_t written;
				err = o.m_stream.write_some(ptr, translated, written);
				ptr += written;
				translated -= written;
			}
			while (translated && ((flush && !err) || (std::errc::interrupted == err)));
			std::copy_n(ptr, translated, extrep);
			o.m_aux = extrep + translated;
			o.setp(o.m_buffer, o.m_buffer + (BUFFER_SIZE - ((translated + 1) >> 1) - 1));
		}
		return !err;
	}
};


write_stream_streambuf::write_stream_streambuf(write_stream &stream, utf8_id_t, bool bom, bool nl) : m_stream(stream)
{
	if (nl && (3 == CRLF))
	{
		// worst case is a buffer full of LF doubling in size when CR are inserted
		m_extrep = std::malloc(BUFFER_SIZE * 3);
		if (m_extrep)
			m_buffer = reinterpret_cast<char_type *>(m_extrep) + (BUFFER_SIZE * 2);
		else
			m_buffer = nullptr;
		m_sync_func = &helper::sync_utf8_crlf;
		m_aux = m_extrep;
	}
	else
	{
		// newline translation won't change size
		m_buffer = reinterpret_cast<char_type *>(std::malloc(BUFFER_SIZE));
		m_extrep = nullptr;
		if (nl && (1 == CRLF))
		{
			m_sync_func = &helper::sync_utf8<true>;
			m_aux = m_buffer;
		}
		else
		{
			m_sync_func = &helper::sync_utf8<false>;
			m_aux = nullptr;
		}
	}
	if (!m_buffer)
		throw std::bad_alloc();

	// implicitly insert encoded U+FEFF in buffer if requested
	setg(nullptr, nullptr, nullptr);
	setp(m_buffer, m_buffer + (BUFFER_SIZE - 1));
	if (bom)
	{
		m_buffer[0] = 0xef;
		m_buffer[1] = 0xbb;
		m_buffer[2] = 0xbf;
		pbump(3);
		if (nl && (1 == CRLF))
			m_aux = m_buffer + 3;
	}
}

write_stream_streambuf::write_stream_streambuf(write_stream &stream, utf16native_id_t, bool bom, bool nl) : m_stream(stream)
{
	helper::allocate_utf16(*this, false, bom, nl);
	if (nl)
		m_sync_func = &helper::sync_utf16<false, true>;
	else
		m_sync_func = &helper::sync_utf16<false, false>;
}

write_stream_streambuf::write_stream_streambuf(write_stream &stream, utf16swapped_id_t, bool bom, bool nl) : m_stream(stream)
{
	helper::allocate_utf16(*this, true, bom, nl);
	if (nl)
		m_sync_func = &helper::sync_utf16<true, true>;
	else
		m_sync_func = &helper::sync_utf16<true, false>;
}

write_stream_streambuf::~write_stream_streambuf()
{
	m_sync_func(*this, true, true);
	if (m_extrep)
		std::free(m_extrep);
	else if (m_buffer)
		std::free(m_buffer);
}

int write_stream_streambuf::sync()
{
	return m_sync_func(*this, true, false) ? 0 : -1;
}

write_stream_streambuf::int_type write_stream_streambuf::overflow(int_type ch)
{
	bool unconsumed(false);
	if (!traits_type::eq_int_type(ch, traits_type::eof()))
	{
		if (pptr() <= epptr())
		{
			*pptr() = traits_type::to_char_type(ch);
			pbump(1);
		}
		else
		{
			unconsumed = true;
		}
	}
	bool const result(m_sync_func(*this, false, false));
	if (unconsumed && (pptr() <= epptr()))
	{
		*pptr() = traits_type::to_char_type(ch);
		pbump(1);
	}
	return result ? traits_type::not_eof(ch) : traits_type::eof();
}

} // namespace util
