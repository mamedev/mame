// license:BSD-3-Clause
// copyright-holders:Miodrag Milanovic
/***************************************************************************

    pluginopts.cpp

    Plugin options manager.

***************************************************************************/
#ifndef MAME_FRONTEND_PLUGINOPTS_H
#define MAME_FRONTEND_PLUGINOPTS_H

#pragma once

#include "options.h"

#include <list>
#include <string>


class plugin_options : public core_options
{
public:
	plugin_options();

	void parse_json(std::string path);

private:
	static const options_entry s_option_entries[];
	std::list<std::string> m_descriptions;
};

#endif // MAME_FRONTEND_PLUGINOPTS_H
