// license:BSD-3-Clause
// copyright-holders:Vas Crabb
/***************************************************************************

    language.cpp

    Multi-language support.

***************************************************************************/

#include "emu.h"
#include "language.h"

#include "emuopts.h"
#include "fileio.h"

#include "corestr.h"

#include <string>


void load_translation(emu_options &m_options)
{
	util::unload_translation();

	std::string name = m_options.language();
	if (name.empty())
		return;

	strreplace(name, " ", "_");
	strreplace(name, "(", "");
	strreplace(name, ")", "");
	emu_file file(m_options.language_path(), OPEN_FLAG_READ);
	if (file.open(name + PATH_SEPARATOR "strings.mo"))
	{
		osd_printf_error("Error opening translation file %s\n", name);
		return;
	}

	osd_printf_verbose("Loading translation file %s\n", file.fullpath());
	util::load_translation(file);
}
