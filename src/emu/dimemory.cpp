// license:BSD-3-Clause
// copyright-holders:Aaron Giles
/***************************************************************************

    dimemory.c

    Device memory interfaces.

***************************************************************************/

#include "emu.h"
#include "validity.h"


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
		m_databus_width(0),
		m_addrbus_width(0),
		m_addrbus_shift(0),
		m_logaddr_width(0),
		m_page_shift(0),
		m_internal_map(nullptr),
		m_default_map(nullptr)
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
address_space_config::address_space_config(const char *name, endianness_t endian, UINT8 datawidth, UINT8 addrwidth, INT8 addrshift, address_map_constructor internal, address_map_constructor defmap)
	: m_name(name),
		m_endianness(endian),
		m_databus_width(datawidth),
		m_addrbus_width(addrwidth),
		m_addrbus_shift(addrshift),
		m_logaddr_width(addrwidth),
		m_page_shift(0),
		m_internal_map(internal),
		m_default_map(defmap)
{
}

address_space_config::address_space_config(const char *name, endianness_t endian, UINT8 datawidth, UINT8 addrwidth, INT8 addrshift, UINT8 logwidth, UINT8 pageshift, address_map_constructor internal, address_map_constructor defmap)
	: m_name(name),
		m_endianness(endian),
		m_databus_width(datawidth),
		m_addrbus_width(addrwidth),
		m_addrbus_shift(addrshift),
		m_logaddr_width(logwidth),
		m_page_shift(pageshift),
		m_internal_map(internal),
		m_default_map(defmap)
{
}

address_space_config::address_space_config(const char *name, endianness_t endian, UINT8 datawidth, UINT8 addrwidth, INT8 addrshift, address_map_delegate internal, address_map_delegate defmap)
	: m_name(name),
		m_endianness(endian),
		m_databus_width(datawidth),
		m_addrbus_width(addrwidth),
		m_addrbus_shift(addrshift),
		m_logaddr_width(addrwidth),
		m_page_shift(0),
		m_internal_map(nullptr),
		m_default_map(nullptr),
		m_internal_map_delegate(std::move(internal)),
		m_default_map_delegate(std::move(defmap))
{
}

address_space_config::address_space_config(const char *name, endianness_t endian, UINT8 datawidth, UINT8 addrwidth, INT8 addrshift, UINT8 logwidth, UINT8 pageshift, address_map_delegate internal, address_map_delegate defmap)
	: m_name(name),
		m_endianness(endian),
		m_databus_width(datawidth),
		m_addrbus_width(addrwidth),
		m_addrbus_shift(addrshift),
		m_logaddr_width(logwidth),
		m_page_shift(pageshift),
		m_internal_map(nullptr),
		m_default_map(nullptr),
		m_internal_map_delegate(std::move(internal)),
		m_default_map_delegate(std::move(defmap))
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
	memset(m_address_map, 0, sizeof(m_address_map));
	memset(m_addrspace, 0, sizeof(m_addrspace));

	// configure the fast accessor
	device.m_memory = this;
}


//-------------------------------------------------
//  ~device_memory_interface - destructor
//-------------------------------------------------

device_memory_interface::~device_memory_interface()
{
}


//-------------------------------------------------
//  static_set_addrmap - configuration helper
//  to connect an address map to a device
//-------------------------------------------------

void device_memory_interface::static_set_addrmap(device_t &device, address_spacenum spacenum, address_map_constructor map)
{
	device_memory_interface *memory;
	if (!device.interface(memory))
		throw emu_fatalerror("MCFG_DEVICE_ADDRESS_MAP called on device '%s' with no memory interface", device.tag());
	if (spacenum >= ARRAY_LENGTH(memory->m_address_map))
		throw emu_fatalerror("MCFG_DEVICE_ADDRESS_MAP called on device '%s' with out-of-range space number %d", device.tag(), spacenum);
	memory->m_address_map[spacenum] = map;
}


//-------------------------------------------------
//  set_address_space - connect an address space
//  to a device
//-------------------------------------------------

void device_memory_interface::set_address_space(address_spacenum spacenum, address_space &space)
{
	assert(spacenum < ARRAY_LENGTH(m_addrspace));
	m_addrspace[spacenum] = &space;
}


//-------------------------------------------------
//  memory_translate - translate from logical to
//  phyiscal addresses; designed to be overridden
//  by the actual device implementation if address
//  translation is supported
//-------------------------------------------------

bool device_memory_interface::memory_translate(address_spacenum spacenum, int intention, offs_t &address)
{
	// by default it maps directly
	return true;
}


//-------------------------------------------------
//  memory_read - perform internal memory
//  operations that bypass the memory system;
//  designed to be overridden by the actual device
//  implementation if internal read operations are
//  handled by bypassing the memory system
//-------------------------------------------------

bool device_memory_interface::memory_read(address_spacenum spacenum, offs_t offset, int size, UINT64 &value)
{
	// by default, we don't do anything
	return false;
}


//-------------------------------------------------
//  memory_write - perform internal memory
//  operations that bypass the memory system;
//  designed to be overridden by the actual device
//  implementation if internal write operations are
//  handled by bypassing the memory system
//-------------------------------------------------

bool device_memory_interface::memory_write(address_spacenum spacenum, offs_t offset, int size, UINT64 value)
{
	// by default, we don't do anything
	return false;
}


//-------------------------------------------------
//  memory_readop - perform internal memory
//  operations that bypass the memory system;
//  designed to be overridden by the actual device
//  implementation if internal opcode fetching
//  operations are handled by bypassing the memory
//  system
//-------------------------------------------------

bool device_memory_interface::memory_readop(offs_t offset, int size, UINT64 &value)
{
	// by default, we don't do anything
	return false;
}


//-------------------------------------------------
//  interface_validity_check - perform validity
//  checks on the memory configuration
//-------------------------------------------------

void device_memory_interface::interface_validity_check(validity_checker &valid) const
{
	// loop over all address spaces
	for (address_spacenum spacenum = AS_0; spacenum < ADDRESS_SPACES; ++spacenum)
	{
		if (space_config(spacenum) != nullptr)
		{
			// construct the map
			::address_map addrmap(const_cast<device_t &>(device()), spacenum);

			// let the map check itself
			addrmap.map_validity_check(valid, device(), spacenum);
		}
	}
}
