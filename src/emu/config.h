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

/*************************************
 *
 *  Constants
 *
 *************************************/

#define CONFIG_VERSION          10

enum class config_type
{
	INIT = 0,       // opportunity to initialize things first
	CONTROLLER,     // loading from controller file
	DEFAULT,        // loading from default.cfg
	GAME,           // loading from game.cfg
	FINAL           // opportunity to finish initialization
};

/*************************************
 *
 *  Type definitions
 *
 *************************************/

typedef delegate<void (config_type, util::xml::data_node const *)> config_load_delegate;
typedef delegate<void (config_type, util::xml::data_node *)> config_save_delegate;

// ======================> configuration_manager

class configuration_manager
{
	struct config_element
	{
		std::string          name;              // node name
		config_load_delegate load;              // load callback
		config_save_delegate save;              // save callback
	};

public:
	// construction/destruction
	configuration_manager(running_machine &machine);

	void config_register(const char* nodename, config_load_delegate load, config_save_delegate save);
	int load_settings();
	void save_settings();

	// getters
	running_machine &machine() const { return m_machine; }
private:
	int load_xml(emu_file &file, config_type which_type);
	int save_xml(emu_file &file, config_type which_type);

	// internal state
	running_machine &   m_machine;                  // reference to our machine
	std::vector<config_element> m_typelist;
};

#endif  /* MAME_EMU_CONFIG_H */
