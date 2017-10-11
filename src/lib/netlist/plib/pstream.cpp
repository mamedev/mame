// license:GPL-2.0+
// copyright-holders:Couriersud
/*
 * pstream.c
 *
 */

#include "pstream.h"
#include "palloc.h"

#include <cstdio>
#include <cstdlib>
#include <algorithm>

// VS2015 prefers _dup
#ifdef _WIN32
#include <io.h>
#else
#include <unistd.h>
#endif

namespace plib {

pstream::~pstream()
{
}

// -----------------------------------------------------------------------------
// pistream: input stream
// -----------------------------------------------------------------------------

pistream::~pistream()
{
}

// -----------------------------------------------------------------------------
// postream: output stream
// -----------------------------------------------------------------------------

postream::~postream()
{
}

void postream::write(pistream &strm)
{
	char buf[1024];
	pos_type r;
	while ((r=strm.read(buf, 1024)) > 0)
		write(buf, r);
}

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

pifilestream::pos_type pifilestream::vread(void *buf, const pos_type n)
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

pifilestream::pos_type pifilestream::vtell()
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

pstdin::~pstdin()
{
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

void pofilestream::vwrite(const void *buf, const pos_type n)
{
	std::size_t r = fwrite(buf, 1, n, static_cast<FILE *>(m_file));
	if (r < n)
	{
		//printf("%ld %ld %s\n", r, n, strerror(errno));
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

pstream::pos_type pofilestream::vtell()
{
	std::ptrdiff_t ret = ftell(static_cast<FILE *>(m_file));
	if (ret < 0)
	{
		return m_pos;
	}
	else
		return static_cast<pos_type>(ret);
}

postringstream::~postringstream()
{
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

pstderr::~pstderr()
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

pstdout::~pstdout()
{
}

// -----------------------------------------------------------------------------
// Memory stream
// -----------------------------------------------------------------------------

pimemstream::pimemstream(const void *mem, const pos_type len)
	: pistream(FLAG_SEEKABLE), m_pos(0), m_len(len), m_mem(static_cast<const pstring::mem_t *>(mem))
{
}

pimemstream::pimemstream(const pomemstream &ostrm)
: pistream(FLAG_SEEKABLE), m_pos(0), m_len(ostrm.size()), m_mem(reinterpret_cast<pstring::mem_t *>(ostrm.memory()))
{
}

pimemstream::~pimemstream()
{
}

pimemstream::pos_type pimemstream::vread(void *buf, const pos_type n)
{
	pos_type ret = (m_pos + n <= m_len) ? n :  m_len - m_pos;

	if (ret > 0)
	{
		std::copy(m_mem + m_pos, m_mem + m_pos + ret, static_cast<char *>(buf));
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

pimemstream::pos_type pimemstream::vtell()
{
	return m_pos;
}

pistringstream::~pistringstream()
{
}

// -----------------------------------------------------------------------------
// Output memory stream
// -----------------------------------------------------------------------------

pomemstream::pomemstream()
: postream(FLAG_SEEKABLE), m_pos(0), m_capacity(1024), m_size(0)
{
	m_mem = palloc_array<char>(m_capacity);
}

pomemstream::~pomemstream()
{
	pfree_array(m_mem);
}

void pomemstream::vwrite(const void *buf, const pos_type n)
{
	if (m_pos + n >= m_capacity)
	{
		while (m_pos + n >= m_capacity)
			m_capacity *= 2;
		char *o = m_mem;
		m_mem = palloc_array<char>(m_capacity);
		if (m_mem == nullptr)
		{
			throw out_of_mem_e("pomemstream::vwrite");
		}
		std::copy(o, o + m_pos, m_mem);
		pfree_array(o);
	}

	std::copy(static_cast<const char *>(buf), static_cast<const char *>(buf) + n, m_mem + m_pos);
	m_pos += n;
	m_size = std::max(m_pos, m_size);
}

void pomemstream::vseek(const pos_type n)
{
	m_pos = n;
	m_size = std::max(m_pos, m_size);
	if (m_size >= m_capacity)
	{
		while (m_size >= m_capacity)
			m_capacity *= 2;
		char *o = m_mem;
		m_mem = palloc_array<char>(m_capacity);
		if (m_mem == nullptr)
		{
			throw out_of_mem_e("pomemstream::vseek");
		}
		std::copy(o, o + m_pos, m_mem);
		pfree_array(o);
	}
}

pstream::pos_type pomemstream::vtell()
{
	return m_pos;
}

bool putf8_reader::readline(pstring &line)
{
	pstring::code_t c = 0;
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
			m_linebuf += pstring(c);
		if (!this->readcode(c))
			break;
	}
	line = m_linebuf;
	return true;
}

putf8_fmt_writer::putf8_fmt_writer(postream &strm)
: pfmt_writer_t()
, putf8_writer(strm)
{
}

putf8_fmt_writer::~putf8_fmt_writer()
{
}

void putf8_fmt_writer::vdowrite(const pstring &ls) const
{
	write(ls);
}



}
