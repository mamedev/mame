// license:BSD-3-Clause
// copyright-holders:David Broman
/*********************************************************************

    srcdbg_provider.cpp

    Factory to create format-specific implementation of
    format-agnostic interface to source-debugging info

***************************************************************************/


#include "emu.h"
#include "srcdbg_provider_simple.h"
#include "srcdbg_provider.h"
#include "srcdbg_format_reader.h"
#include "srcdbg_api.h"

#include "emuopts.h"
#include "fileio.h"


// Create and initialize debug info provider of the correct type by reading
// the debug info file
//
// static
srcdbg_provider_base * srcdbg_provider_base::create_debug_info(running_machine &machine, const std::string & di_path)
{
	std::string error;
	srcdbg_format format;
	if (!srcdbg_format_header_read(di_path.c_str(), format, error))
	{
		throw emu_fatalerror("Error reading source-level debugging information file\n%s\n\n%s", di_path, error.c_str());
	}

	switch (format)
	{
	case SRCDBG_FORMAT_SIMPLE:
	{
		srcdbg_provider_simple * ret = new srcdbg_provider_simple(machine);
		srcdbg_import importer(*ret);
		if (!srcdbg_format_simp_read(di_path.c_str(), importer, error))
		{
			if (!error.empty())
			{
				throw emu_fatalerror("Error importing source-level debugging information\n\n%s", error.c_str());
			}
			return nullptr;
		}
		return ret;
	}

	// FUTURE: If more file formats are invented, add cases for them here to read them

	default:
		assert(!"Unexpected source-level debugging information file format");
		return nullptr;
	}

	assert (!"This code should be unreachable");
	return nullptr;
}

srcdbg_provider_base::srcdbg_provider_base()
	: m_global_fixed_symbols()
	, m_local_fixed_symbols()
	, m_local_relative_symbols()
{
}