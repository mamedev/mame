// license:BSD-3-Clause
// copyright-holders:Carl
/***************************************************************************

    network.h

    Core network interface functions and definitions.
***************************************************************************/

#ifndef MAME_EMU_NETWORK_H
#define MAME_EMU_NETWORK_H

#pragma once

// ======================> network_manager

class network_manager
{
public:
	// construction/destruction
	network_manager(running_machine &machine);

	// getters
	running_machine &machine() const { return m_machine; }
private:
	void config_load(config_type cfg_type, config_level cfg_lvl, util::xml::data_node const *parentnode);
	void config_save(config_type cfg_type, util::xml::data_node *parentnode);

	// internal state
	running_machine &   m_machine;                  // reference to our machine
};

#endif // MAME_EMU_NETWORK_H
