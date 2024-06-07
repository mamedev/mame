// license:BSD-3-Clause
// copyright-holders:Carl
/***************************************************************************

    network.cpp

    Core network functions and definitions.

***************************************************************************/

#include "emu.h"
#include "network.h"

#include "config.h"
#include "dinetwork.h"

#include "xmlfile.h"

#include <cctype>

//**************************************************************************
//  NETWORK MANAGER
//**************************************************************************

//-------------------------------------------------
//  network_manager - constructor
//-------------------------------------------------

network_manager::network_manager(running_machine &machine)
	: m_machine(machine)
{
	machine.configuration().config_register(
			"network",
			configuration_manager::load_delegate(&network_manager::config_load, this),
			configuration_manager::save_delegate(&network_manager::config_save, this));
}

//-------------------------------------------------
//  config_load - read and apply data from the
//  configuration file
//-------------------------------------------------

void network_manager::config_load(config_type cfg_type, config_level cfg_level, util::xml::data_node const *parentnode)
{
	if ((cfg_type == config_type::SYSTEM) && parentnode)
	{
		for (util::xml::data_node const *node = parentnode->get_child("device"); node; node = node->get_next_sibling("device"))
		{
			const char *tag = node->get_attribute_string("tag", nullptr);

			if ((tag != nullptr) && (tag[0] != '\0'))
			{
				for (device_network_interface &network : network_interface_enumerator(machine().root_device()))
				{
					if (!strcmp(tag, network.device().tag())) {
						int interface = node->get_attribute_int("interface", 0);
						network.set_interface(interface);
						const char *mac_addr = node->get_attribute_string("mac", nullptr);
						if (mac_addr != nullptr && strlen(mac_addr) == 17) {
							uint8_t mac[6];
							unsigned int mac_num[6];
							sscanf(mac_addr, "%02x:%02x:%02x:%02x:%02x:%02x", &mac_num[0], &mac_num[1], &mac_num[2], &mac_num[3], &mac_num[4], &mac_num[5]);
							for (int i = 0; i<6; i++) mac[i] = mac_num[i];
							network.set_mac(mac);
						}

					}
				}
			}
		}
	}
}
//-------------------------------------------------
//  config_save - save data to the configuration
//  file
//-------------------------------------------------

void network_manager::config_save(config_type cfg_type, util::xml::data_node *parentnode)
{
	// only save about system-specific data
	if (cfg_type == config_type::SYSTEM)
	{
		for (device_network_interface &network : network_interface_enumerator(machine().root_device()))
		{
			util::xml::data_node *const node = parentnode->add_child("device", nullptr);
			if (node)
			{
				node->set_attribute("tag", network.device().tag());
				node->set_attribute_int("interface", network.get_interface());
				const std::array<u8, 6> &mac = network.get_mac();
				const std::string mac_addr = util::string_format(
						"%02x:%02x:%02x:%02x:%02x:%02x",
						mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);
				node->set_attribute("mac", mac_addr.c_str());
			}
		}
	}
}
