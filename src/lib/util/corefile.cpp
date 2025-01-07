// license:BSD-3-Clause
// copyright-holders:Aaron Giles, Vas Crabb
/***************************************************************************

    corefile.cpp

    File access functions.

***************************************************************************/

#include "corefile.h"

#include "coretmpl.h"
#include "osdcore.h"
#include "unicode.h"
#include "vecstream.h"

#include <cassert>
#include <cstring>
#include <iterator>
#include <limits>


namespace util {

namespace {

/***************************************************************************
    VALIDATION
***************************************************************************/

#if !defined(CRLF) || (CRLF < 1) || (CRLF > 3)
#error CRLF undefined: must be 1 (CR), 2 (LF) or 3 (CR/LF)
#endif



/***************************************************************************
    TYPE DEFINITIONS
***************************************************************************/

class core_proxy_file : public core_file
{
public:
	core_proxy_file(core_file &file) noexcept : m_file(file) { }

	virtual std::error_condition seek(std::int64_t offset, int whence) noexcept override { return m_file.seek(offset, whence); }
	virtual std::error_condition tell(std::uint64_t &result) noexcept override { return m_file.tell(result); }
	virtual std::error_condition length(std::uint64_t &result) noexcept override { return m_file.length(result); }

	virtual std::error_condition read_some(void *buffer, std::size_t length, std::size_t &actual) noexcept override { return m_file.read_some(buffer, length, actual); }
	virtual std::error_condition read_some_at(std::uint64_t offset, void *buffer, std::size_t length, std::size_t &actual) noexcept override { return m_file.read_some_at(offset, buffer, length, actual); }

	virtual std::error_condition finalize() noexcept override { return m_file.finalize(); }
	virtual std::error_condition flush() noexcept override { return m_file.flush(); }
	virtual std::error_condition write_some(void const *buffer, std::size_t length, std::size_t &actual) noexcept override { return m_file.write_some(buffer, length, actual); }
	virtual std::error_condition write_some_at(std::uint64_t offset, void const *buffer, std::size_t length, std::size_t &actual) noexcept override { return m_file.write_some_at(offset, buffer, length, actual); }

	virtual bool eof() const override { return m_file.eof(); }

	virtual int getc() override { return m_file.getc(); }
	virtual int ungetc(int c) override { return m_file.ungetc(c); }
	virtual char *gets(char *s, int n) override { return m_file.gets(s, n); }

	virtual int puts(std::string_view s) override { return m_file.puts(s); }
	virtual int vprintf(util::format_argument_pack<char> const &args) override { return m_file.vprintf(args); }
	virtual std::error_condition truncate(std::uint64_t offset) override { return m_file.truncate(offset); }

private:
	core_file &m_file;
};


class core_text_file : public core_file
{
public:
	enum class text_file_type
	{
		OSD,        // OSD dependent encoding format used when BOMs missing
		UTF8,       // UTF-8
		UTF16BE,    // UTF-16 (big endian)
		UTF16LE,    // UTF-16 (little endian)
		UTF32BE,    // UTF-32 (UCS-4) (big endian)
		UTF32LE     // UTF-32 (UCS-4) (little endian)
	};

	virtual int getc() override;
	virtual int ungetc(int c) override;
	virtual char *gets(char *s, int n) override;
	virtual int puts(std::string_view s) override;
	virtual int vprintf(util::format_argument_pack<char> const &args) override;

protected:
	core_text_file(std::uint32_t openflags)
		: m_openflags(openflags)
		, m_text_type(text_file_type::OSD)
		, m_back_char_head(0)
		, m_back_char_tail(0)
		, m_printf_buffer()
	{
	}

	bool read_access() const noexcept { return 0U != (m_openflags & OPEN_FLAG_READ); }
	bool write_access() const noexcept { return 0U != (m_openflags & OPEN_FLAG_WRITE); }
	bool no_bom() const noexcept { return 0U != (m_openflags & OPEN_FLAG_NO_BOM); }

	bool has_putback() const noexcept { return m_back_char_head != m_back_char_tail; }
	void clear_putback() noexcept { m_back_char_head = m_back_char_tail = 0; }

private:
	std::uint32_t const m_openflags;                    // flags we were opened with
	text_file_type      m_text_type;                    // text output format
	char                m_back_chars[UTF8_CHAR_MAX];    // buffer to hold characters for ungetc
	int                 m_back_char_head;               // head of ungetc buffer
	int                 m_back_char_tail;               // tail of ungetc buffer
	ovectorstream       m_printf_buffer;                // persistent buffer for formatted output
};


class core_basic_file : public core_text_file
{
public:
	virtual std::error_condition seek(std::int64_t offset, int whence) noexcept override;
	virtual std::error_condition tell(std::uint64_t &result) noexcept override { result = m_index; return std::error_condition(); }
	virtual std::error_condition length(std::uint64_t &result) noexcept override { result = m_size; return std::error_condition(); }

	virtual bool eof() const override;

protected:
	core_basic_file(std::uint32_t openflags, std::uint64_t length) noexcept
		: core_text_file(openflags)
		, m_index(0)
		, m_size(length)
	{
	}

	std::uint64_t index() const noexcept { return m_index; }
	void add_offset(std::size_t increment) noexcept { m_index += increment; m_size = (std::max)(m_size, m_index); }
	std::uint64_t size() const noexcept { return m_size; }
	void set_size(std::uint64_t value) noexcept { m_size = value; }

	static std::size_t safe_buffer_copy(
			void const *source, std::size_t sourceoffs, std::size_t sourcelen,
			void *dest, std::size_t destoffs, std::size_t destlen) noexcept;


private:
	std::uint64_t   m_index;            // current file offset
	std::uint64_t   m_size;             // total file length
};

class core_in_memory_file final : public core_basic_file
{
public:
	core_in_memory_file(std::uint32_t openflags, void const *data, std::size_t length, bool copy) noexcept
		: core_basic_file(openflags, length)
		, m_data_allocated(false)
		, m_data(copy ? nullptr : data)
	{
		if (copy)
		{
			void *const buf = allocate();
			if (buf)
				std::memcpy(buf, data, length);
		}
	}

	~core_in_memory_file() override { purge(); }

	virtual std::error_condition read_some(void *buffer, std::size_t length, std::size_t &actual) noexcept override;
	virtual std::error_condition read_some_at(std::uint64_t offset, void *buffer, std::size_t length, std::size_t &actual) noexcept override;

	virtual std::error_condition finalize() noexcept override { return std::error_condition(); }
	virtual std::error_condition flush() noexcept override { clear_putback(); return std::error_condition(); }
	virtual std::error_condition write_some(void const *buffer, std::size_t length, std::size_t &actual) noexcept override { actual = 0; return std::errc::bad_file_descriptor; }
	virtual std::error_condition write_some_at(std::uint64_t offset, void const *buffer, std::size_t length, std::size_t &actual) noexcept override { actual = 0; return std::errc::bad_file_descriptor; }

	void const *buffer() const { return m_data; }

	virtual std::error_condition truncate(std::uint64_t offset) override;

protected:
	void *allocate() noexcept
	{
		if (m_data || (std::numeric_limits<std::size_t>::max() < size()))
			return nullptr;
		void *data = malloc(size());
		if (data)
		{
			m_data_allocated = true;
			m_data = data;
		}
		return data;
	}

	void purge() noexcept
	{
		if (m_data && m_data_allocated)
			free(const_cast<void *>(m_data));
		m_data_allocated = false;
		m_data = nullptr;
	}

private:
	bool            m_data_allocated;   // was the data allocated by us?
	void const *    m_data;             // file data, if RAM-based
};


class core_osd_file final : public core_basic_file
{
public:
	core_osd_file(std::uint32_t openmode, osd_file::ptr &&file, std::uint64_t length)
		: core_basic_file(openmode, length)
		, m_file(std::move(file))
	{
	}
	~core_osd_file() override;

	virtual std::error_condition read_some(void *buffer, std::size_t length, std::size_t &actual) noexcept override;
	virtual std::error_condition read_some_at(std::uint64_t offset, void *buffer, std::size_t length, std::size_t &actual) noexcept override;

	virtual std::error_condition finalize() noexcept override;
	virtual std::error_condition flush() noexcept override;
	virtual std::error_condition write_some(void const *buffer, std::size_t length, std::size_t &actual) noexcept override;
	virtual std::error_condition write_some_at(std::uint64_t offset, void const *buffer, std::size_t length, std::size_t &actual) noexcept override;

	virtual std::error_condition truncate(std::uint64_t offset) override;

protected:
	bool is_buffered(std::uint64_t offset) const noexcept { return (offset >= m_bufferbase) && (offset < (m_bufferbase + m_bufferbytes)); }

private:
	static constexpr std::size_t FILE_BUFFER_SIZE = 512;

	osd_file::ptr   m_file;                     // OSD file handle
	std::uint64_t   m_bufferbase = 0U;          // base offset of internal buffer
	std::uint32_t   m_bufferbytes = 0U;         // bytes currently loaded into buffer
	std::uint8_t    m_buffer[FILE_BUFFER_SIZE]; // buffer data
};



//**************************************************************************
//  core_text_file
//**************************************************************************

//-------------------------------------------------
//  getc - read a character from a file
//-------------------------------------------------

int core_text_file::getc()
{
	// refresh buffer, if necessary
	if (m_back_char_head == m_back_char_tail)
	{
		// do we need to check the byte order marks?
		std::uint64_t pos;
		if (!tell(pos))
		{
			if (!pos)
			{
				std::uint8_t bom[4];
				auto const [err, readlen] = read(*this, bom, 4); // FIXME: check for errors
				if (readlen == 4)
				{
					if (bom[0] == 0xef && bom[1] == 0xbb && bom[2] == 0xbf)
					{
						m_text_type = text_file_type::UTF8;
						pos = 3;
					}
					else if (bom[0] == 0x00 && bom[1] == 0x00 && bom[2] == 0xfe && bom[3] == 0xff)
					{
						m_text_type = text_file_type::UTF32BE;
						pos = 4;
					}
					else if (bom[0] == 0xff && bom[1] == 0xfe && bom[2] == 0x00 && bom[3] == 0x00)
					{
						m_text_type = text_file_type::UTF32LE;
						pos = 4;
					}
					else if (bom[0] == 0xfe && bom[1] == 0xff)
					{
						m_text_type = text_file_type::UTF16BE;
						pos = 2;
					}
					else if (bom[0] == 0xff && bom[1] == 0xfe)
					{
						m_text_type = text_file_type::UTF16LE;
						pos = 2;
					}
					else
					{
						m_text_type = text_file_type::OSD;
						pos = 0;
					}
				}
				seek(pos, SEEK_SET); // FIXME: don't assume seeking is possible, check for errors
			}
		}

		// fetch the next character
		// FIXME: all of this plays fast and loose with error checking and seeks backwards far too frequently
		char16_t utf16_buffer[UTF16_CHAR_MAX];
		char32_t uchar = ~char32_t(0);
		switch (m_text_type)
		{
		default:
		case text_file_type::OSD:
			{
				char default_buffer[16];
				auto const [err, readlen] = read(*this, default_buffer, sizeof(default_buffer));
				if (readlen > 0)
				{
					auto const charlen = osd_uchar_from_osdchar(&uchar, default_buffer, readlen / sizeof(default_buffer[0]));
					if( charlen == 0 ) // terminating null character ('\0') has been read. Corrupted file.
						uchar = ~char32_t(0);
					else
						seek(std::int64_t(charlen * sizeof(default_buffer[0])) - readlen, SEEK_CUR);
				}
			}
			break;

		case text_file_type::UTF8:
			{
				char utf8_buffer[UTF8_CHAR_MAX];
				auto const [err, readlen] = read(*this, utf8_buffer, sizeof(utf8_buffer));
				if (readlen > 0)
				{
					auto const charlen = uchar_from_utf8(&uchar, utf8_buffer, readlen / sizeof(utf8_buffer[0]));
					seek(std::int64_t(charlen * sizeof(utf8_buffer[0])) - readlen, SEEK_CUR);
				}
			}
			break;

		case text_file_type::UTF16BE:
			{
				auto const [err, readlen] = read(*this, utf16_buffer, sizeof(utf16_buffer));
				if (readlen > 0)
				{
					auto const charlen = uchar_from_utf16be(&uchar, utf16_buffer, readlen / sizeof(utf16_buffer[0]));
					seek(std::int64_t(charlen * sizeof(utf16_buffer[0])) - readlen, SEEK_CUR);
				}
			}
			break;

		case text_file_type::UTF16LE:
			{
				auto const [err, readlen] = read(*this, utf16_buffer, sizeof(utf16_buffer));
				if (readlen > 0)
				{
					auto const charlen = uchar_from_utf16le(&uchar, utf16_buffer, readlen / sizeof(utf16_buffer[0]));
					seek(std::int64_t(charlen * sizeof(utf16_buffer[0])) - readlen, SEEK_CUR);
				}
			}
			break;

		case text_file_type::UTF32BE:
			{
				// FIXME: deal with read returning short
				auto const [err, readlen] = read(*this, &uchar, sizeof(uchar));
				if (sizeof(uchar) == readlen)
					uchar = big_endianize_int32(uchar);
			}
			break;

		case text_file_type::UTF32LE:
			{
				// FIXME: deal with read returning short
				auto const [err, readlen] = read(*this, &uchar, sizeof(uchar));
				if (sizeof(uchar) == readlen)
					uchar = little_endianize_int32(uchar);
			}
			break;
		}

		if (uchar != ~0)
		{
			// place the new character in the ring buffer
			m_back_char_head = 0;
			m_back_char_tail = utf8_from_uchar(m_back_chars, std::size(m_back_chars), uchar);
			//assert(file->back_char_tail != -1);
		}
	}

	// now read from the ring buffer
	int result;
	if (m_back_char_head == m_back_char_tail)
		result = EOF;
	else
	{
		result = m_back_chars[m_back_char_head++];
		m_back_char_head %= std::size(m_back_chars);
	}

	return result;
}


//-------------------------------------------------
//  ungetc - put back a character read from a
//  file
//-------------------------------------------------

int core_text_file::ungetc(int c)
{
	m_back_chars[m_back_char_tail++] = char(c);
	m_back_char_tail %= std::size(m_back_chars);
	return c;
}


//-------------------------------------------------
//  gets - read a line from a text file
//-------------------------------------------------

char *core_text_file::gets(char *s, int n)
{
	char *cur = s;

	// loop while we have characters
	while (n > 0)
	{
		int const c = getc();
		if (c == EOF)
			break;
		else if (c == 0x0d) // if there's a CR, look for an LF afterwards
		{
			int const c2 = getc();
			if (c2 != 0x0a)
				ungetc(c2);
			*cur++ = 0x0d;
			n--;
			break;
		}
		else if (c == 0x0a) // if there's an LF, reinterp as a CR for consistency
		{
			*cur++ = 0x0d;
			n--;
			break;
		}
		else // otherwise, pop the character in and continue
		{
			*cur++ = c;
			n--;
		}
	}

	// if we put nothing in, return nullptr
	if (cur == s)
		return nullptr;

	/* otherwise, terminate */
	if (n > 0)
		*cur++ = 0;
	return s;
}


//-------------------------------------------------
//  puts - write a string to a text file
//-------------------------------------------------

int core_text_file::puts(std::string_view s)
{
	// TODO: what to do about write errors?
	// The API doesn't lend itself to reporting the error as the return
	// value includes extra bytes inserted like the UTF-8 marker and
	// carriage returns.
	char convbuf[1024];
	char *pconvbuf = convbuf;
	int count = 0;

	// is this the beginning of the file?  if so, write a byte order mark
	if (!no_bom())
	{
		std::uint64_t offset;
		if (!tell(offset))
		{
			if (!offset)
			{
				*pconvbuf++ = char(0xef);
				*pconvbuf++ = char(0xbb);
				*pconvbuf++ = char(0xbf);
			}
		}
	}

	// convert '\n' to platform dependant line endings
	for (char ch : s)
	{
		if (ch == '\n')
		{
			if (CRLF == 1)      // CR only
				*pconvbuf++ = 13;
			else if (CRLF == 2) // LF only
				*pconvbuf++ = 10;
			else if (CRLF == 3) // CR+LF
			{
				*pconvbuf++ = 13;
				*pconvbuf++ = 10;
			}
		}
		else
			*pconvbuf++ = ch;

		// if we overflow, break into chunks
		if (pconvbuf >= convbuf + std::size(convbuf) - 10)
		{
			auto const [err, written] = write(*this, convbuf, pconvbuf - convbuf); // FIXME: error ignored here
			count += written;
			pconvbuf = convbuf;
		}
	}

	// final flush
	if (pconvbuf != convbuf)
	{
		auto const [err, written] = write(*this, convbuf, pconvbuf - convbuf); // FIXME: error ignored here
		count += written;
	}

	return count;
}


//-------------------------------------------------
//  vprintf - vfprintf to a text file
//-------------------------------------------------

int core_text_file::vprintf(util::format_argument_pack<char> const &args)
{
	m_printf_buffer.clear();
	m_printf_buffer.reserve(1024);
	m_printf_buffer.seekp(0, ovectorstream::beg);
	util::stream_format(m_printf_buffer, args);
	return puts(buf_to_string_view(m_printf_buffer));
}



//**************************************************************************
//  core_basic_file
//**************************************************************************

//-------------------------------------------------
//  seek - seek within a file
//-------------------------------------------------

std::error_condition core_basic_file::seek(std::int64_t offset, int whence) noexcept
{
	// flush any buffered char
	clear_putback(); // TODO: report errors; also, should the argument check happen before this?

	// switch off the relative location
	switch (whence)
	{
	case SEEK_SET:
		if (0 > offset)
			return std::errc::invalid_argument;
		m_index = offset;
		return std::error_condition();

	case SEEK_CUR:
		if (0 > offset)
		{
			if (std::uint64_t(-offset) > m_index)
				return std::errc::invalid_argument;
		}
		else if ((std::numeric_limits<std::uint64_t>::max() - offset) < m_index)
		{
			return std::errc::invalid_argument;
		}
		m_index += offset;
		return std::error_condition();

	case SEEK_END:
		if (0 > offset)
		{
			if (std::uint64_t(-offset) > m_size)
				return std::errc::invalid_argument;
		}
		else if ((std::numeric_limits<std::uint64_t>::max() - offset) < m_size)
		{
			return std::errc::invalid_argument;
		}
		m_index = m_size + offset;
		return std::error_condition();

	default:
		return std::errc::invalid_argument;
	}
}


//-------------------------------------------------
//  eof - return true if we're at the end of file
//-------------------------------------------------

bool core_basic_file::eof() const
{
	// check for buffered chars
	if (has_putback())
		return false;

	// if the offset == length, we're at EOF
	return (m_index >= m_size);
}


//-------------------------------------------------
//  safe_buffer_copy - copy safely from one
//  bounded buffer to another
//-------------------------------------------------

std::size_t core_basic_file::safe_buffer_copy(
		void const *source, std::size_t sourceoffs, std::size_t sourcelen,
		void *dest, std::size_t destoffs, std::size_t destlen) noexcept
{
	auto const sourceavail = sourcelen - sourceoffs;
	auto const destavail = destlen - destoffs;
	auto const bytes_to_copy = (std::min)(sourceavail, destavail);
	if (bytes_to_copy > 0)
	{
		std::memcpy(
				reinterpret_cast<std::uint8_t *>(dest) + destoffs,
				reinterpret_cast<std::uint8_t const *>(source) + sourceoffs,
				bytes_to_copy);
	}
	return bytes_to_copy;
}



//**************************************************************************
//  core_in_memory_file
//**************************************************************************

//-------------------------------------------------
//  read - read from a file
//-------------------------------------------------

std::error_condition core_in_memory_file::read_some(void *buffer, std::size_t length, std::size_t &actual) noexcept
{
	clear_putback();

	// handle RAM-based files
	if (index() < size())
		actual = safe_buffer_copy(m_data, std::size_t(index()), std::size_t(size()), buffer, 0, length);
	else
		actual = 0U;
	add_offset(actual);
	return std::error_condition();
}

std::error_condition core_in_memory_file::read_some_at(std::uint64_t offset, void *buffer, std::size_t length, std::size_t &actual) noexcept
{
	clear_putback();

	// handle RAM-based files
	if (offset < size())
		actual = safe_buffer_copy(m_data, std::size_t(offset), std::size_t(size()), buffer, 0, length);
	else
		actual = 0U;
	return std::error_condition();
}


//-------------------------------------------------
//  truncate - truncate a file
//-------------------------------------------------

std::error_condition core_in_memory_file::truncate(std::uint64_t offset)
{
	if (size() < offset)
		return std::errc::io_error; // TODO: revisit this error code

	// adjust to new length and offset
	set_size(offset);
	return std::error_condition();
}


//**************************************************************************
//  core_osd_file
//**************************************************************************

//-------------------------------------------------
//  closes a file
//-------------------------------------------------

core_osd_file::~core_osd_file()
{
	// close files and free memory
}


//-------------------------------------------------
//  read - read from a file
//-------------------------------------------------

std::error_condition core_osd_file::read_some(void *buffer, std::size_t length, std::size_t &actual) noexcept
{
	// since osd_file works like pread/pwrite, implement in terms of read_at
	// core_osd_file is declared final, so a derived class can't interfere
	std::error_condition err = read_some_at(index(), buffer, length, actual);
	add_offset(actual);
	return err;
}


std::error_condition core_osd_file::read_some_at(std::uint64_t offset, void *buffer, std::size_t length, std::size_t &actual) noexcept
{
	if (!m_file)
	{
		actual = 0U;
		return std::errc::bad_file_descriptor;
	}

	// flush any buffered char
	clear_putback();

	actual = 0U;
	std::error_condition err;

	// if we're within the buffer, consume that first
	if (is_buffered(offset))
		actual += safe_buffer_copy(m_buffer, offset - m_bufferbase, m_bufferbytes, buffer, actual, length);

	// if we've got a small amount left, read it into the buffer first
	if (actual < length)
	{
		if ((length - actual) < (sizeof(m_buffer) / 2))
		{
			// read as much as makes sense into the buffer
			m_bufferbase = offset + actual;
			err = m_file->read(m_buffer, m_bufferbase, sizeof(m_buffer), m_bufferbytes);

			// do a bounded copy from the buffer to the destination if it succeeded
			if (!err)
				actual += safe_buffer_copy(m_buffer, 0, m_bufferbytes, buffer, actual, length);
			else
				m_bufferbytes = 0U;
		}
		else
		{
			// read the remainder directly from the file - may need to return short if size_t is larger than 32 bits
			std::uint32_t const chunk = std::min<std::common_type_t<std::uint32_t, std::size_t> >(std::numeric_limits<std::uint32_t>::max(), length - actual);
			std::uint32_t bytes_read;
			err = m_file->read(reinterpret_cast<std::uint8_t *>(buffer) + actual, offset + actual, chunk, bytes_read);
			if (!err)
				actual += bytes_read;
		}
	}

	// return any errors
	return err;
}


//-------------------------------------------------
//  write - write to a file
//-------------------------------------------------

std::error_condition core_osd_file::write_some(void const *buffer, std::size_t length, std::size_t &actual) noexcept
{
	// since osd_file works like pread/pwrite, implement in terms of write_at
	// core_osd_file is declared final, so a derived class can't interfere
	std::error_condition err = write_some_at(index(), buffer, length, actual);
	add_offset(actual);
	return err;
}


std::error_condition core_osd_file::write_some_at(std::uint64_t offset, void const *buffer, std::size_t length, std::size_t &actual) noexcept
{
	// flush any buffered char
	clear_putback();

	// invalidate any buffered data
	m_bufferbytes = 0U;

	// do the write - may need to return short if size_t is larger than 32 bits
	std::uint32_t const chunk = std::min<std::common_type_t<std::uint32_t, std::size_t> >(std::numeric_limits<std::uint32_t>::max(), length);
	std::uint32_t bytes_written;
	std::error_condition err = m_file->write(buffer, offset, chunk, bytes_written);
	if (err)
	{
		// bytes written not valid on error
		actual = 0U;
	}
	else
	{
		assert(chunk >= bytes_written);
		offset += bytes_written;
		actual = bytes_written;
		set_size((std::max)(size(), offset));
	}
	return err;
}


//-------------------------------------------------
//  truncate - truncate a file
//-------------------------------------------------

std::error_condition core_osd_file::truncate(std::uint64_t offset)
{
	// truncate file
	std::error_condition err = m_file->truncate(offset);
	if (err)
		return err;

	// and adjust to new length and offset
	set_size(offset);
	return std::error_condition();
}


//-------------------------------------------------
//  flush - flush file buffers
//-------------------------------------------------

std::error_condition core_osd_file::finalize() noexcept
{
	return std::error_condition();
}


std::error_condition core_osd_file::flush() noexcept
{
	// flush any buffered char
	clear_putback();

	// invalidate any buffered data
	m_bufferbytes = 0U;

	return m_file->flush();
}

} // anonymous namespace



//**************************************************************************
//  core_file
//**************************************************************************

//-------------------------------------------------
//  open - open a file for access and
//  return an error code
//-------------------------------------------------

std::error_condition core_file::open(std::string_view filename, std::uint32_t openflags, ptr &file) noexcept
{
	// attempt to open the file
	osd_file::ptr f;
	std::uint64_t length = 0;
	auto const filerr = osd_file::open(std::string(filename), openflags, f, length); // FIXME: allow osd_file to accept std::string_view
	if (filerr)
		return filerr;

	try { file = std::make_unique<core_osd_file>(openflags, std::move(f), length); }
	catch (...) { return std::errc::not_enough_memory; }

	return std::error_condition();
}


//-------------------------------------------------
//  open_ram - open a RAM-based buffer for file-
//  like access and return an error code
//-------------------------------------------------

std::error_condition core_file::open_ram(void const *data, std::size_t length, std::uint32_t openflags, ptr &file) noexcept
{
	// can only do this for read access
	if ((openflags & OPEN_FLAG_WRITE) || (openflags & OPEN_FLAG_CREATE))
		return std::errc::invalid_argument;

	// if length is non-zero, data must be non-null
	if (length && !data)
		return std::errc::invalid_argument;

	// platforms where size_t is larger than 64 bits are theoretically possible
	if (std::uint64_t(length) != length)
		return std::errc::file_too_large;

	ptr result(new (std::nothrow) core_in_memory_file(openflags, data, length, false));
	if (!result)
		return std::errc::not_enough_memory;

	file = std::move(result);
	return std::error_condition();
}


//-------------------------------------------------
//  open_ram_copy - open a copy of a RAM-based
//  buffer for file-like access and return an
//  error code
//-------------------------------------------------

std::error_condition core_file::open_ram_copy(void const *data, std::size_t length, std::uint32_t openflags, ptr &file) noexcept
{
	// can only do this for read access
	if ((openflags & OPEN_FLAG_WRITE) || (openflags & OPEN_FLAG_CREATE))
		return std::errc::invalid_argument;

	// if length is non-zero, data must be non-null
	if (length && !data)
		return std::errc::invalid_argument;

	// platforms where size_t is larger than 64 bits are theoretically possible
	if (std::uint64_t(length) != length)
		return std::errc::file_too_large;

	std::unique_ptr<core_in_memory_file> result(new (std::nothrow) core_in_memory_file(openflags, data, length, true));
	if (!result || !result->buffer())
		return std::errc::not_enough_memory;

	file = std::move(result);
	return std::error_condition();
}


//-------------------------------------------------
//  open_proxy - open a proxy to an existing file
//  object and return an error code
//-------------------------------------------------

std::error_condition core_file::open_proxy(core_file &file, ptr &proxy) noexcept
{
	ptr result(new (std::nothrow) core_proxy_file(file));
	if (!result)
		return std::errc::not_enough_memory;

	proxy = std::move(result);
	return std::error_condition();
}


//-------------------------------------------------
//  closes a file
//-------------------------------------------------

core_file::~core_file()
{
}


//-------------------------------------------------
//  load - open a file with the specified
//  filename, read it into memory, and return a
//  pointer
//-------------------------------------------------

std::error_condition core_file::load(std::string_view filename, void **data, std::size_t &length) noexcept
{
	std::error_condition err;

	// attempt to open the file
	ptr file;
	err = open(filename, OPEN_FLAG_READ, file);
	if (err)
		return err;

	// get the size
	std::uint64_t size;
	err = file->length(size);
	if (err)
		return err;
	else if (std::size_t(size) != size)
		return std::errc::file_too_large;

	// allocate memory
	*data = std::malloc(std::size_t(size));
	if (!*data)
		return std::errc::not_enough_memory;
	length = std::size_t(size);

	// read the data
	if (size)
	{
		std::size_t actual;
		std::tie(err, actual) = read(*file, *data, std::size_t(size));
		if (err || (size != actual))
		{
			std::free(*data);
			*data = nullptr;
			if (err)
				return err;
			else
				return std::errc::io_error; // TODO: revisit this error code - file truncated out from under us
		}
	}

	// close the file and return data
	return std::error_condition();
}

std::error_condition core_file::load(std::string_view filename, std::vector<uint8_t> &data) noexcept
{
	std::error_condition err;

	// attempt to open the file
	ptr file;
	err = open(filename, OPEN_FLAG_READ, file);
	if (err)
		return err;

	// get the size
	std::uint64_t size;
	err = file->length(size);
	if (err)
		return err;
	else if (std::size_t(size) != size)
		return std::errc::file_too_large;

	// allocate memory
	try { data.resize(std::size_t(size)); }
	catch (...) { return std::errc::not_enough_memory; }

	// read the data
	if (size)
	{
		std::size_t actual;
		std::tie(err, actual) = read(*file, &data[0], std::size_t(size));
		if (err || (size != actual))
		{
			data.clear();
			if (err)
				return err;
			else
				return std::errc::io_error; // TODO: revisit this error code - file truncated out from under us
		}
	}

	// close the file and return data
	return std::error_condition();
}

} // namespace util
