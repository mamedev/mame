// license:BSD-3-Clause
// copyright-holders:Carl
/***************************************************************************

    network.c

    Core network functions and definitions.
***************************************************************************/
#include <ctype.h>

#include "emu.h"
#include "network.h"
#include "config.h"
#include "xmlfile.h"

//**************************************************************************
//  NETWORK MANAGER
//**************************************************************************

//-------------------------------------------------
//  network_manager - constructor
//-------------------------------------------------

network_manager::network_manager(running_machine &machine)
	: m_machine(machine)
{
	machine.configuration().config_register("network", config_saveload_delegate(FUNC(network_manager::config_load), this), config_saveload_delegate(FUNC(network_manager::config_save), this));
}

//-------------------------------------------------
//  config_load - read and apply data from the
//  configuration file
//-------------------------------------------------

void network_manager::config_load(config_type cfg_type, xml_data_node *parentnode)
{
	xml_data_node *node;
	if ((cfg_type == config_type::CONFIG_TYPE_GAME) && (parentnode != nullptr))
	{
		for (node = xml_get_sibling(parentnode->child, "device"); node; node = xml_get_sibling(node->next, "device"))
		{
			const char *tag = xml_get_attribute_string(node, "tag", nullptr);

			if ((tag != nullptr) && (tag[0] != '\0'))
			{
				network_interface_iterator iter(machine().root_device());
				for (device_network_interface *network = iter.first(); network != nullptr; network = iter.next())
				{
					if (!strcmp(tag, network->device().tag().c_str())) {
						int interface = xml_get_attribute_int(node, "interface", 0);
						network->set_interface(interface);
						const char *mac_addr = xml_get_attribute_string(node, "mac", nullptr);
						if (mac_addr != nullptr && strlen(mac_addr) == 17) {
							char mac[7];
							unsigned int mac_num[6];
							sscanf(mac_addr, "%02x:%02x:%02x:%02x:%02x:%02x", &mac_num[0], &mac_num[1], &mac_num[2], &mac_num[3], &mac_num[4], &mac_num[5]);
							for (int i = 0; i<6; i++) mac[i] = mac_num[i];
							network->set_mac(mac);
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

void network_manager::config_save(config_type cfg_type, xml_data_node *parentnode)
{
	xml_data_node *node;

	/* only care about game-specific data */
	if (cfg_type == config_type::CONFIG_TYPE_GAME)
	{
		network_interface_iterator iter(machine().root_device());
		for (device_network_interface *network = iter.first(); network != nullptr; network = iter.next())
		{
			node = xml_add_child(parentnode, "device", nullptr);
			if (node != nullptr)
			{
				xml_set_attribute(node, "tag", network->device().tag().c_str());
				xml_set_attribute_int(node, "interface", network->get_interface());
				const char *mac = network->get_mac();
				char mac_addr[6 * 3];
				sprintf(mac_addr, "%02x:%02x:%02x:%02x:%02x:%02x", (UINT8)mac[0], (UINT8)mac[1], (UINT8)mac[2], (UINT8)mac[3], (UINT8)mac[4], (UINT8)mac[5]);
				xml_set_attribute(node, "mac", mac_addr);
			}
		}
	}
}
