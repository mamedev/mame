// license:BSD-3-Clause
// copyright-holders:Nicola Salmoria, Aaron Giles
/***************************************************************************

    emucore.cpp

    Simple core functions that are defined in emucore.h and which may
    need to be accessed by other MAME-related tools.

****************************************************************************/

#include "emu.h"
#include "emucore.h"
#include "osdcore.h"

emu_fatalerror::emu_fatalerror(util::format_argument_pack<std::ostream> const &args)
	: emu_fatalerror(0, args)
{
	osd_break_into_debugger(m_text.c_str());
}

emu_fatalerror::emu_fatalerror(int _exitcode, util::format_argument_pack<std::ostream> const &args)
	: m_text(util::string_format(args))
	, m_code(_exitcode)
{
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
