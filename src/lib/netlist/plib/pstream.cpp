// license:GPL-2.0+
// copyright-holders:Couriersud
/*
 * pstream.c
 *
 */

#include "pstream.h"
#include "palloc.h"

#include <algorithm>
#include <cstdio>
#include <cstdlib>

// VS2015 prefers _dup
#ifdef _WIN32
#include <io.h>
#else
#include <unistd.h>
#endif

namespace plib {

// -----------------------------------------------------------------------------
// pistream: input stream
// -----------------------------------------------------------------------------

// -----------------------------------------------------------------------------
// postream: output stream
// -----------------------------------------------------------------------------

// -----------------------------------------------------------------------------
// Input file stream
// -----------------------------------------------------------------------------

pifilestream::pifilestream(const pstring &fname)
: pistream(0)
, m_file(fopen(fname.c_str(), "rb"))
, m_pos(0)
, m_actually_close(true)
, m_filename(fname)
{
	if (m_file == nullptr)
		throw file_open_e(fname);
	else
		init();
}

pifilestream::pifilestream(void *file, const pstring &name, const bool do_close)
: pistream(0), m_file(file), m_pos(0), m_actually_close(do_close), m_filename(name)
{
	if (m_file == nullptr)
		throw null_argument_e(m_filename);
	init();
}

void pifilestream::init()
{
	if (ftell(static_cast<FILE *>(m_file)) >= 0)
	{
		if (fseek(static_cast<FILE *>(m_file), 0, SEEK_SET) >= 0)
			set_flag(FLAG_SEEKABLE);
	}
}

pifilestream::~pifilestream()
{
	if (m_actually_close)
	{
		fclose(static_cast<FILE *>(m_file));
	}
}

pifilestream::pos_type pifilestream::vread(value_type *buf, const pos_type n)
{
	pos_type r = fread(buf, 1, n, static_cast<FILE *>(m_file));
	if (r < n)
	{
		if (feof(static_cast<FILE *>(m_file)))
			set_flag(FLAG_EOF);
		if (ferror(static_cast<FILE *>(m_file)))
			throw file_read_e(m_filename);
	}
	m_pos += r;
	return r;
}

void pifilestream::vseek(const pos_type n)
{
	if (fseek(static_cast<FILE *>(m_file), static_cast<long>(n), SEEK_SET) < 0)
		throw file_e("File seek failed: {}", m_filename);
	else
		m_pos = n;
	if (feof(static_cast<FILE *>(m_file)))
		set_flag(FLAG_EOF);
	else
		clear_flag(FLAG_EOF);
	if (ferror(static_cast<FILE *>(m_file)))
		throw file_e("Generic file operation failed: {}", m_filename);
}

pifilestream::pos_type pifilestream::vtell() const
{
	long ret = ftell(static_cast<FILE *>(m_file));
	if (ret < 0)
	{
		return m_pos;
	}
	else
		return static_cast<pos_type>(ret);
}

// -----------------------------------------------------------------------------
// pstdin: reads from stdin
// -----------------------------------------------------------------------------

pstdin::pstdin()
: pifilestream(stdin, "<stdin>", false)
{
	/* nothing to do */
}

// -----------------------------------------------------------------------------
// Output file stream
// -----------------------------------------------------------------------------

pofilestream::pofilestream(const pstring &fname)
: postream(0), m_file(fopen(fname.c_str(), "wb")), m_pos(0), m_actually_close(true), m_filename(fname)
{
	if (m_file == nullptr)
		throw file_open_e(m_filename);
	init();
}

pofilestream::pofilestream(void *file, const pstring &name, const bool do_close)
: postream(0), m_file(file), m_pos(0), m_actually_close(do_close), m_filename(name)
{
	if (m_file == nullptr)
		throw null_argument_e(m_filename);
	init();
}

void pofilestream::init()
{
	if (ftell(static_cast<FILE *>(m_file)) >= 0)
		if (fseek(static_cast<FILE *>(m_file), 0, SEEK_SET) >= 0)
			set_flag(FLAG_SEEKABLE);
}

pofilestream::~pofilestream()
{
	if (m_actually_close)
	{
		fflush(static_cast<FILE *>(m_file));
		fclose(static_cast<FILE *>(m_file));
	}
}

void pofilestream::vwrite(const value_type *buf, const pos_type n)
{
	std::size_t r = fwrite(buf, 1, n, static_cast<FILE *>(m_file));
	if (r < n)
	{
		if (ferror(static_cast<FILE *>(m_file)))
			throw file_write_e(m_filename);
	}
	m_pos += r;
}

void pofilestream::vseek(const pos_type n)
{
	if (fseek(static_cast<FILE *>(m_file), static_cast<long>(n), SEEK_SET) < 0)
		throw file_e("File seek failed: {}", m_filename);
	else
	{
		m_pos = n;
		if (ferror(static_cast<FILE *>(m_file)))
			throw file_e("Generic file operation failed: {}", m_filename);
	}
}

pstream::pos_type pofilestream::vtell() const
{
	std::ptrdiff_t ret = ftell(static_cast<FILE *>(m_file));
	if (ret < 0)
	{
		return m_pos;
	}
	else
		return static_cast<pos_type>(ret);
}

// -----------------------------------------------------------------------------
// pstderr: write to stderr
// -----------------------------------------------------------------------------

pstderr::pstderr()
#ifdef _WIN32
: pofilestream(fdopen(_dup(fileno(stderr)), "wb"), "<stderr>", true)
#else
: pofilestream(fdopen(dup(fileno(stderr)), "wb"), "<stderr>", true)
#endif
{
}

// -----------------------------------------------------------------------------
// pstdout: write to stdout
// -----------------------------------------------------------------------------

pstdout::pstdout()
#ifdef _WIN32
: pofilestream(fdopen(_dup(fileno(stdout)), "wb"), "<stdout>", true)
#else
: pofilestream(fdopen(dup(fileno(stdout)), "wb"), "<stdout>", true)
#endif
{
}

// -----------------------------------------------------------------------------
// Memory stream
// -----------------------------------------------------------------------------

pimemstream::pimemstream(const void *mem, const pos_type len)
	: pistream(FLAG_SEEKABLE), m_pos(0), m_len(len), m_mem(static_cast<const char *>(mem))
{
}

pimemstream::pimemstream()
	: pistream(FLAG_SEEKABLE), m_pos(0), m_len(0), m_mem(static_cast<const char *>(nullptr))
{
}

pimemstream::pimemstream(const pomemstream &ostrm)
: pistream(FLAG_SEEKABLE), m_pos(0), m_len(ostrm.size()), m_mem(reinterpret_cast<const char *>(ostrm.memory()))
{
}

pimemstream::pos_type pimemstream::vread(value_type *buf, const pos_type n)
{
	pos_type ret = (m_pos + n <= m_len) ? n :  m_len - m_pos;

	if (ret > 0)
	{
		std::copy(m_mem + m_pos, m_mem + m_pos + ret, reinterpret_cast<char *>(buf));
		m_pos += ret;
	}

	if (ret < n)
		set_flag(FLAG_EOF);

	return ret;
}

void pimemstream::vseek(const pos_type n)
{
	m_pos = (n>=m_len) ? m_len : n;
	clear_flag(FLAG_EOF);

}

pimemstream::pos_type pimemstream::vtell() const
{
	return m_pos;
}

// -----------------------------------------------------------------------------
// Output memory stream
// -----------------------------------------------------------------------------

pomemstream::pomemstream()
: postream(FLAG_SEEKABLE), m_pos(0), m_mem(1024)
{
	m_mem.clear();
}

void pomemstream::vwrite(const value_type *buf, const pos_type n)
{
	if (m_pos + n >= m_mem.size())
		m_mem.resize(m_pos + n);

	std::copy(buf, buf + n, &m_mem[0] + m_pos);
	m_pos += n;
}

void pomemstream::vseek(const pos_type n)
{
	m_pos = n;
	if (m_pos>=m_mem.size())
		m_mem.resize(m_pos);
}

pstream::pos_type pomemstream::vtell() const
{
	return m_pos;
}

bool putf8_reader::readline(pstring &line)
{
	putf8string::code_t c = 0;
	m_linebuf = "";
	if (!this->readcode(c))
	{
		line = "";
		return false;
	}
	while (true)
	{
		if (c == 10)
			break;
		else if (c != 13) /* ignore CR */
			m_linebuf += putf8string(c);
		if (!this->readcode(c))
			break;
	}
	line = m_linebuf.c_str();
	return true;
}


void putf8_fmt_writer::vdowrite(const pstring &ls) const
{
	write(ls);
}



} // namespace plib
