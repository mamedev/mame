// license:BSD-3-Clause
// copyright-holders:Aaron Giles
/***************************************************************************

    dimemory.c

    Device memory interfaces.

***************************************************************************/

#include "emu.h"


//**************************************************************************
//  CONSTANTS
//**************************************************************************

//const int TRIGGER_SUSPENDTIME = -4000;



//**************************************************************************
//  ADDRESS SPACE CONFIG
//**************************************************************************

//-------------------------------------------------
//  address_space_config - constructors
//-------------------------------------------------

address_space_config::address_space_config()
	: m_name("unknown"),
		m_endianness(ENDIANNESS_NATIVE),
		m_data_width(0),
		m_addr_width(0),
		m_addr_shift(0),
		m_logaddr_width(0),
		m_page_shift(0),
		m_is_octal(false),
		m_internal_map(address_map_constructor()),
		m_default_map(address_map_constructor())
{
}

/*!
 @param name
 @param endian CPU endianness
 @param datawidth CPU parallelism bits
 @param addrwidth address bits
 @param addrshift
 @param internal
 @param defmap
 */
address_space_config::address_space_config(const char *name, endianness_t endian, u8 datawidth, u8 addrwidth, s8 addrshift, address_map_constructor internal, address_map_constructor defmap)
	: m_name(name),
		m_endianness(endian),
		m_data_width(datawidth),
		m_addr_width(addrwidth),
		m_addr_shift(addrshift),
		m_logaddr_width(addrwidth),
		m_page_shift(0),
		m_is_octal(false),
		m_internal_map(internal),
		m_default_map(defmap)
{
}

address_space_config::address_space_config(const char *name, endianness_t endian, u8 datawidth, u8 addrwidth, s8 addrshift, u8 logwidth, u8 pageshift, address_map_constructor internal, address_map_constructor defmap)
	: m_name(name),
		m_endianness(endian),
		m_data_width(datawidth),
		m_addr_width(addrwidth),
		m_addr_shift(addrshift),
		m_logaddr_width(logwidth),
		m_page_shift(pageshift),
		m_is_octal(false),
		m_internal_map(internal),
		m_default_map(defmap)
{
}


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
	if (spacenum >= int(m_address_map.size()))
		m_address_map.resize(spacenum+1);
	m_address_map[spacenum] = map;
}


//-------------------------------------------------
//  dump - dump memory tables to the given file in
//  human-readable format
//-------------------------------------------------

void device_memory_interface::dump(FILE *file) const
{
	for (auto const &space : m_addrspace)
		if (space) {
			fprintf(file,
					"\n\n"
					"====================================================\n"
					"Device '%s' %s address space read handler dump\n"
					"====================================================\n",
					device().tag(), space->name());
			space->dump_map(file, read_or_write::READ);

			fprintf(file,
					"\n\n"
					"====================================================\n"
					"Device '%s' %s address space write handler dump\n"
					"====================================================\n",
					device().tag(), space->name());
			space->dump_map(file, read_or_write::WRITE);
		}
}


//-------------------------------------------------
//  memory_translate - translate from logical to
//  phyiscal addresses; designed to be overridden
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
	for (int spacenum = 0; spacenum < int(m_address_config.size()); ++spacenum)
	{
		if (space_config(spacenum))
		{
			// construct the map
			::address_map addrmap(const_cast<device_t &>(device()), spacenum);

			// let the map check itself
			addrmap.map_validity_check(valid, spacenum);
		}
	}
}
