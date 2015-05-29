// license:BSD-3-Clause
// copyright-holders:Nicola Salmoria, Aaron Giles
/***************************************************************************

    emucore.c

    Simple core functions that are defined in emucore.h and which may
    need to be accessed by other MAME-related tools.
****************************************************************************/

#include "emu.h"
#include "emucore.h"
#include "osdcore.h"

emu_fatalerror::emu_fatalerror(const char *format, ...)
: code(0)
{
	if (format == NULL)
	{
		text[0] = '\0';
	}
	else
	{
		va_list ap;
		va_start(ap, format);
		vsnprintf(text, sizeof(text), format, ap);
		va_end(ap);
	}
	osd_break_into_debugger(text);
}

emu_fatalerror::emu_fatalerror(const char *format, va_list ap)
: code(0)
{
	if (format == NULL)
	{
		text[0] = '\0';
	}
	else
	{
		vsnprintf(text, sizeof(text), format, ap);
	}
	osd_break_into_debugger(text);
}

emu_fatalerror::emu_fatalerror(int _exitcode, const char *format, ...)
: code(_exitcode)
{
	if (format == NULL)
	{
		text[0] = '\0';
	}
	else
	{
		va_list ap;
		va_start(ap, format);
		vsnprintf(text, sizeof(text), format, ap);
		va_end(ap);
	}
}

emu_fatalerror::emu_fatalerror(int _exitcode, const char *format, va_list ap)
: code(_exitcode)
{
	if (format == NULL)
	{
		text[0] = '\0';
	}
	else
	{
		vsnprintf(text, sizeof(text), format, ap);
	}
}


void report_bad_cast(const std::type_info &src_type, const std::type_info &dst_type)
{
	throw emu_fatalerror("Error: bad downcast<> or device<>.  Tried to convert a %s to a %s, which are incompatible.\n",
			src_type.name(), dst_type.name());
}

void report_bad_device_cast(const device_t *dev, const std::type_info &src_type, const std::type_info &dst_type)
{
	throw emu_fatalerror("Error: bad downcast<> or device<>.  Tried to convert the device %s (%s) of type %s to a %s, which are incompatible.\n",
			dev->tag(), dev->name(), src_type.name(), dst_type.name());
}

void fatalerror(const char *format, ...)
{
	va_list ap;
	va_start(ap, format);
	emu_fatalerror error(format, ap);
	va_end(ap);
	throw error;
}

void fatalerror_exitcode(running_machine &machine, int exitcode, const char *format, ...)
{
	va_list ap;
	va_start(ap, format);
	emu_fatalerror error(exitcode, format, ap);
	va_end(ap);
	throw error;
}
