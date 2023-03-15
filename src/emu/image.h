// license:BSD-3-Clause
// copyright-holders:Nathan Woods, Miodrag Milanovic
/***************************************************************************

    image.h

    Core image interface functions and definitions.

***************************************************************************/

#ifndef MAME_EMU_IMAGE_H
#define MAME_EMU_IMAGE_H

#pragma once

// ======================> image_manager

class image_manager
{
public:
	// construction/destruction
	image_manager(running_machine &machine);

	void unload_all();
	void postdevice_init();

	// getters
	running_machine &machine() const { return m_machine; }

	std::string setup_working_directory();

private:
	void config_load(config_type cfg_type, config_level cfg_level, util::xml::data_node const *parentnode);
	void config_save(config_type cfg_type, util::xml::data_node *parentnode);

	void options_extract();
	int write_config(emu_options &options, const char *filename, const game_driver *gamedrv);

	bool try_change_working_directory(std::string &working_directory, const std::string &subdir);

	// internal state
	running_machine &   m_machine;                  // reference to our machine
};

#endif /* MAME_EMU_IMAGE_H */
