// license:GPL-2.0+
// copyright-holders:Couriersud
/*
 * pstream.c
 *
 */

#include <cstdio>
#include <cstring>
#include <cstdlib>
#include <algorithm>

#include "pstream.h"
#include "palloc.h"

namespace plib {
// -----------------------------------------------------------------------------
// pistream: input stream
// -----------------------------------------------------------------------------

bool pistream::readline(pstring &line)
{
	char c = 0;
	m_linebuf.clear();
	if (!this->readbyte(c))
	{
		line = "";
		return false;
	}
	while (true)
	{
		if (c == 10)
			break;
		else if (c != 13) /* ignore CR */
			m_linebuf += c;
		if (!this->readbyte(c))
			break;
	}
	line = m_linebuf;
	return true;
}

// -----------------------------------------------------------------------------
// postream: output stream
// -----------------------------------------------------------------------------

void postream::write(pistream &strm)
{
	char buf[1024];
	unsigned r;
	while ((r=strm.read(buf, 1024)) > 0)
		write(buf, r);
}

// -----------------------------------------------------------------------------
// Input file stream
// -----------------------------------------------------------------------------

pifilestream::pifilestream(const pstring &fname)
: pistream(0)
, m_file(fopen(fname.cstr(), "rb"))
, m_pos(0)
, m_actually_close(true)
, m_filename(fname)
{
	if (m_file == nullptr)
		throw file_open_e(fname);
	init();
}

pifilestream::pifilestream(void *file, const pstring name, const bool do_close)
: pistream(0), m_file(file), m_pos(0), m_actually_close(do_close), m_filename(name)
{
	if (m_file == nullptr)
		throw null_argument_e(m_filename);
	init();
}

void pifilestream::init()
{
	if (ftell((FILE *) m_file) >= 0)
	{
		if (fseek((FILE *) m_file, 0, SEEK_SET) >= 0)
			set_flag(FLAG_SEEKABLE);
	}
}

pifilestream::~pifilestream()
{
	if (m_actually_close)
	{
		fclose((FILE *) m_file);
	}
}

unsigned pifilestream::vread(void *buf, const unsigned n)
{
	std::size_t r = fread(buf, 1, n, (FILE *) m_file);
	if (r < n)
	{
		if (feof((FILE *) m_file))
			set_flag(FLAG_EOF);
		if (ferror((FILE *) m_file))
			throw file_read_e(m_filename);
	}
	m_pos += r;
	return r;
}

void pifilestream::vseek(const pos_type n)
{
	if (fseek((FILE *) m_file, SEEK_SET, n) < 0)
		throw file_e("File seek failed: {}", m_filename);
	else
		m_pos = n;
	if (feof((FILE *) m_file))
		set_flag(FLAG_EOF);
	else
		clear_flag(FLAG_EOF);
	if (ferror((FILE *) m_file))
		throw file_e("Generic file operation failed: {}", m_filename);
}

pifilestream::pos_type pifilestream::vtell()
{
	long ret = ftell((FILE *) m_file);
	if (ret < 0)
	{
		return m_pos;
	}
	else
		return ret;
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
: postream(0), m_file(fopen(fname.cstr(), "wb")), m_pos(0), m_actually_close(true), m_filename(fname)
{
	if (m_file == nullptr)
		throw file_open_e(m_filename);
	init();
}

pofilestream::pofilestream(void *file, const pstring name, const bool do_close)
: postream(0), m_file(file), m_pos(0), m_actually_close(do_close), m_filename(name)
{
	if (m_file == nullptr)
		throw null_argument_e(m_filename);
	init();
}

void pofilestream::init()
{
	if (ftell((FILE *) m_file) >= 0)
		if (fseek((FILE *) m_file, 0, SEEK_SET) >= 0)
			set_flag(FLAG_SEEKABLE);
}

pofilestream::~pofilestream()
{
	if (m_actually_close)
		fclose((FILE *) m_file);
}

void pofilestream::vwrite(const void *buf, const unsigned n)
{
	std::size_t r = fwrite(buf, 1, n, (FILE *) m_file);
	if (r < n)
	{
		if (ferror((FILE *) m_file))
			throw file_write_e(m_filename);
	}
	m_pos += r;
}

void pofilestream::vseek(const pos_type n)
{
	if (fseek((FILE *) m_file, SEEK_SET, n) < 0)
		throw file_e("File seek failed: {}", m_filename);
	else
	{
		m_pos = n;
		if (ferror((FILE *) m_file))
			throw file_e("Generic file operation failed: {}", m_filename);
	}
}

pstream::pos_type pofilestream::vtell()
{
	long ret = ftell((FILE *) m_file);
	if (ret < 0)
	{
		return m_pos;
	}
	else
		return ret;
}

// -----------------------------------------------------------------------------
// pstderr: write to stderr
// -----------------------------------------------------------------------------

pstderr::pstderr()
: pofilestream(stderr, "<stderr>", false)
{
}

// -----------------------------------------------------------------------------
// pstdout: write to stdout
// -----------------------------------------------------------------------------

pstdout::pstdout()
: pofilestream(stdout, "<stdout>", false)
{
}

// -----------------------------------------------------------------------------
// Memory stream
// -----------------------------------------------------------------------------

pimemstream::pimemstream(const void *mem, const pos_type len)
	: pistream(FLAG_SEEKABLE), m_pos(0), m_len(len), m_mem((char *) mem)
{
}

pimemstream::pimemstream(const pomemstream &ostrm)
: pistream(FLAG_SEEKABLE), m_pos(0), m_len(ostrm.size()), m_mem((char *) ostrm.memory())
{
}

pimemstream::~pimemstream()
{
}

unsigned pimemstream::vread(void *buf, const unsigned n)
{
	unsigned ret = (m_pos + n <= m_len) ? n :  m_len - m_pos;

	if (ret > 0)
	{
		memcpy(buf, m_mem + m_pos, ret);
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

void pomemstream::vwrite(const void *buf, const unsigned n)
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
		memcpy(m_mem, o, m_pos);
		pfree_array(o);
	}

	memcpy(m_mem + m_pos, buf, n);
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
		memcpy(m_mem, o, m_pos);
		pfree_array(o);
	}
}

pstream::pos_type pomemstream::vtell()
{
	return m_pos;
}

}
