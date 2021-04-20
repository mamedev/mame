// license:BSD-3-Clause
// copyright-holders:Aaron Giles
/***************************************************************************

    infoprotobuf.h

    Dumps the MAME internal data as an Protobuf file.

***************************************************************************/

#ifndef MAME_FRONTEND_MAME_INFOPROTOBUF_H
#define MAME_FRONTEND_MAME_INFOPROTOBUF_H

#pragma once

#include "emuopts.h"

#include <functional>
#include <vector>


//**************************************************************************
//  FUNCTION PROTOTYPES
//**************************************************************************

// helper class to putput
class info_protobuf_creator
{
public:
	// construction/destruction
	info_protobuf_creator(emu_options const &options, bool dtd = true);

	// output
	void output(std::ostream &out, const std::vector<std::string> &patterns);
	void output(std::ostream &out, const std::function<bool(const char *shortname, bool &done)> &filter = { }, bool include_devices = true);

	static char const *feature_name(device_t::feature_type feature);

private:
	// internal state
	emu_options     m_lookup_options;
};

#endif // MAME_FRONTEND_MAME_INFOPROTOBUF_H
