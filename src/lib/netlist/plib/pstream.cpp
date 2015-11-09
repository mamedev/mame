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

// -----------------------------------------------------------------------------
// pistream: input stream
// -----------------------------------------------------------------------------

bool pistream::readline(pstring &line)
{
	UINT8 c = 0;
	pstringbuffer buf;
	if (!this->read(c))
	{
		line = "";
		return false;
	}
	while (true)
	{
		if (c == 10)
			break;
		else if (c != 13) /* ignore CR */
			buf += c;
		if (!this->read(c))
			break;
	}
	line = buf;
	return true;
}

// -----------------------------------------------------------------------------
// postream: output stream
// -----------------------------------------------------------------------------

void postream::write(pistream &strm)
{
	char buf[1024];
	unsigned r;
	while ( !bad() && ((r=strm.read(buf, 1024)) > 0))
		write(buf, r);
}

// -----------------------------------------------------------------------------
// Input file stream
// -----------------------------------------------------------------------------

pifilestream::pifilestream(const pstring &fname)
: pistream(0), m_pos(0), m_actually_close(true)
{
	init(fopen(fname.cstr(), "rb"));
}

pifilestream::pifilestream(void *file, const bool do_close)
: pistream(0), m_pos(0), m_actually_close(do_close)
{
	init(file);
}

void pifilestream::init(void *file)
{
	m_file = file;
	if (m_file == NULL)
	{
		set_flag(FLAG_ERROR);
		set_flag(FLAG_EOF);
		set_flag(FLAG_CLOSED);
	}
	else
	{
		if (ftell((FILE *) m_file) >= 0)
		{
			if (fseek((FILE *) m_file, 0, SEEK_SET) >= 0)
				set_flag(FLAG_SEEKABLE);
		}
	}
}

pifilestream::~pifilestream()
{
	if (!closed())
		close();
}

void pifilestream::close()
{
	if (m_actually_close)
	{
		fclose((FILE *) m_file);
		set_flag(FLAG_CLOSED);
	}
}

unsigned pifilestream::vread(void *buf, unsigned n)
{
	std::size_t r = fread(buf, 1, n, (FILE *) m_file);
	if (r < n)
	{
		if (feof((FILE *) m_file))
			set_flag(FLAG_EOF);
		if (ferror((FILE *) m_file))
			set_flag(FLAG_ERROR);
	}
	m_pos += r;
	return r;
}

void pifilestream::vseek(pos_type n)
{
	check_seekable();
	if (fseek((FILE *) m_file, SEEK_SET, n) < 0)
		set_flag(FLAG_ERROR);
	else
		m_pos = n;
	if (feof((FILE *) m_file))
		set_flag(FLAG_EOF);
	else
		clear_flag(FLAG_EOF);
	if (ferror((FILE *) m_file))
		set_flag(FLAG_ERROR);
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
: pifilestream(stdin, false)
{
	/* nothing to do */
}

// -----------------------------------------------------------------------------
// Output file stream
// -----------------------------------------------------------------------------

pofilestream::pofilestream(const pstring &fname)
: postream(0), m_pos(0), m_actually_close(true)
{
	init(fopen(fname.cstr(), "wb"));
}

pofilestream::pofilestream(void *file, const bool do_close)
: postream(0), m_pos(0), m_actually_close(do_close)
{
	init(file);
}

void pofilestream::init(void *file)
{
	m_file = file;
	if (m_file == NULL)
	{
		set_flag(FLAG_ERROR);
		set_flag(FLAG_CLOSED);
	}
	else
	{
		if (ftell((FILE *) m_file) >= 0)
		{
			if (fseek((FILE *) m_file, 0, SEEK_SET) >= 0)
				set_flag(FLAG_SEEKABLE);
		}
	}
}

pofilestream::~pofilestream()
{
	if (!closed())
		close();
}

void pofilestream::close()
{
	if (m_actually_close)
	{
		fclose((FILE *) m_file);
		set_flag(FLAG_CLOSED);
	}
}

void pofilestream::vwrite(const void *buf, unsigned n)
{
	std::size_t r = fwrite(buf, 1, n, (FILE *) m_file);
	if (r < n)
	{
		if (ferror((FILE *) m_file))
			set_flag(FLAG_ERROR);
	}
	m_pos += r;
}

void pofilestream::vseek(pos_type n)
{
	check_seekable();
	if (fseek((FILE *) m_file, SEEK_SET, n) < 0)
		set_flag(FLAG_ERROR);
	else
	{
		m_pos = n;
		if (ferror((FILE *) m_file))
			set_flag(FLAG_ERROR);
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
: pofilestream(stderr, false)
{
}

// -----------------------------------------------------------------------------
// pstdout: write to stdout
// -----------------------------------------------------------------------------

pstdout::pstdout()
: pofilestream(stdout, false)
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

unsigned pimemstream::vread(void *buf, unsigned n)
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

void pimemstream::vseek(pos_type n)
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
	m_mem = palloc_array(char, m_capacity);
}

pomemstream::~pomemstream()
{
	pfree_array(m_mem);
}

void pomemstream::vwrite(const void *buf, unsigned n)
{
	if (m_pos + n >= m_capacity)
	{
		while (m_pos + n >= m_capacity)
			m_capacity *= 2;
		char *o = m_mem;
		m_mem = palloc_array(char, m_capacity);
		if (m_mem == NULL)
		{
			set_flag(FLAG_ERROR);
			return;
		}
		memcpy(m_mem, o, m_pos);
		pfree_array(o);
	}

	memcpy(m_mem + m_pos, buf, n);
	m_pos += n;
	m_size = std::max(m_pos, m_size);
}

void pomemstream::vseek(pos_type n)
{
	m_pos = n;
	m_size = std::max(m_pos, m_size);
	if (m_size >= m_capacity)
	{
		while (m_size >= m_capacity)
			m_capacity *= 2;
		char *o = m_mem;
		m_mem = palloc_array(char, m_capacity);
		if (m_mem == NULL)
		{
			set_flag(FLAG_ERROR);
			return;
		}
		memcpy(m_mem, o, m_pos);
		pfree_array(o);
	}
}

pstream::pos_type pomemstream::vtell()
{
	return m_pos;
}
