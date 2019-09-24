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
//#include <cstdlib>

// VS2015 prefers _dup
#ifdef _WIN32
#include <io.h>
#else
#include <unistd.h>
#endif

namespace plib {


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
			m_linebuf += putf8string(1, c);
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
