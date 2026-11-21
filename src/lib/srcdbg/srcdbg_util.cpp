// license:BSD-3-Clause
// copyright-holders:David Broman
/*********************************************************************

    srcdbg_util.cpp

    Internal helpers for srcdbg library.  Since srcdbg cannot rely
    on other MAME libraries, this file may include functionality
    found elsewhere in MAME

    WARNING: Tools external to MAME should only use functionality
    declared in srcdg_format.h and srcdbg_api.h.

***************************************************************************/

#include "srcdbg_util.h"

#include <stdarg.h>

// Like sprintf, but for std::string
void srcdbg_sprintf(std::string & out, const char * format, ...)
{
	char buf[1024];
	va_list args;
	va_start(args, format);
	vsnprintf(buf, sizeof(buf), format, args);
	va_end(args);

	out = std::string(buf);
}

