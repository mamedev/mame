// license:BSD-3-Clause
// copyright-holders:Miodrag Milanovic
/***************************************************************************

    pluginopts.cpp

    Plugin options manager.

***************************************************************************/

#pragma once

#ifndef __PLUGIN_OPTS_H__
#define __PLUGIN_OPTS_H__

#include "options.h"

class plugin_options : public core_options
{
public:
	// construction/destruction
	plugin_options();

	void parse_json(std::string path);
private:
	static const options_entry s_option_entries[];
};

#endif /* __PLUGIN_OPTS_H__ */
