// license:BSD-3-Clause
// copyright-holders:Nicola Salmoria, Aaron Giles
/***************************************************************************

    config.h

    Wrappers for handling MAME configuration files

***************************************************************************/

#ifndef MAME_EMU_CONFIG_H
#define MAME_EMU_CONFIG_H

#pragma once

#include "xmlfile.h"


enum class config_type : int
{
	INIT,           // opportunity to initialize things first
	CONTROLLER,     // loading from controller file
	DEFAULT,        // loading from default.cfg
	SYSTEM,         // loading from system.cfg
	FINAL           // opportunity to finish initialization
};

enum class config_level : int
{
	DEFAULT,
	SOURCE,
	BIOS,
	PARENT,
	SYSTEM
};


class configuration_manager
{
public:
	typedef delegate<void (config_type, config_level, util::xml::data_node const *)> load_delegate;
	typedef delegate<void (config_type, util::xml::data_node *)> save_delegate;

	static inline constexpr int CONFIG_VERSION = 10;

	// construction/destruction
	configuration_manager(running_machine &machine);

	void config_register(const char *nodename, load_delegate load, save_delegate save);
	bool load_settings();
	void save_settings();

	// getters
	running_machine &machine() const { return m_machine; }

private:
	struct config_element
	{
		std::string     name;              // node name
		load_delegate   load;              // load callback
		save_delegate   save;              // save callback
	};

	bool load_xml(emu_file &file, config_type which_type);
	bool save_xml(emu_file &file, config_type which_type);

	// internal state
	running_machine &   m_machine;                  // reference to our machine
	std::vector<config_element> m_typelist;
};

#endif // MAME_EMU_CONFIG_H
