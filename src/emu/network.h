// license:BSD-3-Clause
// copyright-holders:Carl
/***************************************************************************

    network.h

    Core network interface functions and definitions.
***************************************************************************/

#pragma once

#ifndef __NETWORK_H__
#define __NETWORK_H__

// ======================> network_manager

class network_manager
{
public:
	// construction/destruction
	network_manager(running_machine &machine);

	// getters
	running_machine &machine() const { return m_machine; }
private:
	void config_load(config_type cfg_type, xml_data_node *parentnode);
	void config_save(config_type cfg_type, xml_data_node *parentnode);

	// internal state
	running_machine &   m_machine;                  // reference to our machine
};

#endif /* __NETWORK_H__ */
