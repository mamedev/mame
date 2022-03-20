// license:BSD-3-Clause
// copyright-holders:Aaron Giles
/***************************************************************************

    dimemory.cpp

    Device memory interfaces.

***************************************************************************/

#include "emu.h"

#include <map>


//**************************************************************************
//  CONSTANTS
//**************************************************************************

//const int TRIGGER_SUSPENDTIME = -4000;



//**************************************************************************
//  MEMORY DEVICE MANAGEMENT
//**************************************************************************

//-------------------------------------------------
//  device_memory_interface - constructor
//-------------------------------------------------

device_memory_interface::device_memory_interface(const machine_config &mconfig, device_t &device)
	: device_interface(device, "memory")
{
	// configure the fast accessor
	device.interfaces().m_memory = this;
}


//-------------------------------------------------
//  ~device_memory_interface - destructor
//-------------------------------------------------

device_memory_interface::~device_memory_interface()
{
}


//-------------------------------------------------
//  set_addrmap - connect an address map to a device
//-------------------------------------------------

void device_memory_interface::set_addrmap(int spacenum, address_map_constructor map)
{
	assert(0 <= spacenum);
	if (spacenum >= int(m_address_map.size()))
		m_address_map.resize(spacenum + 1);
	m_address_map[spacenum] = std::move(map);
}


//-------------------------------------------------
//  memory_translate - translate from logical to
//  physical addresses; designed to be overridden
//  by the actual device implementation if address
//  translation is supported
//-------------------------------------------------

bool device_memory_interface::memory_translate(int spacenum, int intention, offs_t &address)
{
	// by default it maps directly
	return true;
}


//-------------------------------------------------
//  interface_config_complete - perform final
//  memory configuration setup
//-------------------------------------------------

void device_memory_interface::interface_config_complete()
{
	const space_config_vector r = memory_space_config();
	for (const auto &entry : r) {
		if (entry.first >= int(m_address_config.size()))
			m_address_config.resize(entry.first + 1);
		m_address_config[entry.first] = entry.second;
	}
}


//-------------------------------------------------
//  interface_validity_check - perform validity
//  checks on the memory configuration
//-------------------------------------------------

void device_memory_interface::interface_validity_check(validity_checker &valid) const
{
	// loop over all address spaces
	std::map<std::string, int> space_name_map;
	const int max_spaces = std::max(m_address_map.size(), m_address_config.size());
	for (int spacenum = 0; spacenum < max_spaces; ++spacenum)
	{
		const address_space_config *config = space_config(spacenum);
		if (config)
		{
			// validate name
			if (!config->name() || !*config->name())
			{
				osd_printf_error("Name is empty for address space %d\n",  config->name(), spacenum);
			}
			else
			{
				static char const *const validchars = "abcdefghijklmnopqrstuvwxyz0123456789_";
				for (char const *p = config->name(); *p; ++p)
				{
					if (*p == ' ')
					{
						osd_printf_error("Name for address space %s (%d) contains spaces\n",  config->name(), spacenum);
						break;
					}
					if (!strchr(validchars, *p))
					{
						osd_printf_error("Name for address space %s (%d) contains invalid character '%c'\n",  config->name(), spacenum, *p);
						break;
					}
				}
				auto const ins = space_name_map.emplace(config->name(), spacenum);
				if (!ins.second)
					osd_printf_error("Name for address space %s (%d) already used for space %d\n",  config->name(), spacenum, ins.first->second);
			}

			// validate data width
			int width = config->data_width();
			if (width != 8 && width != 16 && width != 32 && width != 64)
				osd_printf_error("Invalid data width %d specified for address space %s (%d)\n", width, config->name(), spacenum);

			// validate address shift
			int shift = config->addr_shift();
			if (shift < 0 && (width >> -shift) < 8)
				osd_printf_error("Invalid shift %d specified for address space %s (%d)\n", shift, config->name(), spacenum);

			// construct the map
			::address_map addrmap(const_cast<device_t &>(device()), spacenum);

			// let the map check itself
			addrmap.map_validity_check(valid, spacenum);
		}
		else if (spacenum < int(m_address_map.size()) && !m_address_map[spacenum].isnull())
			osd_printf_warning("Map configured for nonexistent memory space %d\n", spacenum);
	}
}
