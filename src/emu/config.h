// license:BSD-3-Clause
// copyright-holders:Nicola Salmoria, Aaron Giles
/***************************************************************************

    config.h

    Wrappers for handling MAME configuration files
***************************************************************************/

#pragma once

#ifndef __CONFIG_H__
#define __CONFIG_H__

#include "xmlfile.h"

/*************************************
 *
 *  Constants
 *
 *************************************/

#define CONFIG_VERSION          10

enum class config_type
{
	CONFIG_TYPE_INIT = 0,                   /* opportunity to initialize things first */
	CONFIG_TYPE_CONTROLLER,                 /* loading from controller file */
	CONFIG_TYPE_DEFAULT,                    /* loading from default.cfg */
	CONFIG_TYPE_GAME,                   /* loading from game.cfg */
	CONFIG_TYPE_FINAL                   /* opportunity to finish initialization */
};

/*************************************
 *
 *  Type definitions
 *
 *************************************/

typedef delegate<void (config_type, xml_data_node *)> config_saveload_delegate;

// ======================> configuration_manager

class configuration_manager
{
	struct config_element
	{
		std::string			     name;              /* node name */
		config_saveload_delegate load;              /* load callback */
		config_saveload_delegate save;              /* save callback */
	};

public:
	// construction/destruction
	configuration_manager(running_machine &machine);

	void config_register(const char* nodename, config_saveload_delegate load, config_saveload_delegate save);
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

#endif  /* __CONFIG_H__ */
