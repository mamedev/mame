// license:BSD-3-Clause
// copyright-holders:Aaron Giles
/***************************************************************************

    info.h

    Dumps the MAME internal data as an XML file.

***************************************************************************/

#ifndef MAME_FRONTEND_MAME_INFOXML_H
#define MAME_FRONTEND_MAME_INFOXML_H

#pragma once

#include "emuopts.h"

#include <functional>
#include <vector>


//**************************************************************************
//  FUNCTION PROTOTYPES
//**************************************************************************

// helper class to putput
class info_xml_creator
{
public:
	// construction/destruction
	info_xml_creator(emu_options const &options, bool dtd = true);

	// output
	void output(std::ostream &out, const std::vector<std::string> &patterns);
	void output(std::ostream &out, const std::function<bool(const char *shortname, bool &done)> &filter = { }, bool include_devices = true);

private:
	// internal state
	emu_options     m_lookup_options;
	bool            m_dtd;
};

#endif // MAME_FRONTEND_MAME_INFOXML_H
