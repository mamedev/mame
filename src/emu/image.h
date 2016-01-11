// license:BSD-3-Clause
// copyright-holders:Nathan Woods, Miodrag Milanovic
/***************************************************************************

    image.h

    Core image interface functions and definitions.

***************************************************************************/

#pragma once

#ifndef __IMAGE_H__
#define __IMAGE_H__

// ======================> image_manager

class image_manager
{
public:
	// construction/destruction
	image_manager(running_machine &machine);
	~image_manager();

	void postdevice_init();
	std::string &mandatory_scan(std::string &mandatory);

	// getters
	running_machine &machine() const { return m_machine; }
private:
	void config_load(config_type cfg_type, xml_data_node *parentnode);
	void config_save(config_type cfg_type, xml_data_node *parentnode);

	void options_extract();
	int write_config(emu_options &options, const char *filename, const game_driver *gamedrv);

	// internal state
	running_machine &   m_machine;                  // reference to our machine
};

#endif /* __IMAGE_H__ */
