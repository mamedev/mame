// license:BSD-3-Clause
// copyright-holders:Angelo Salese
/***************************************************************************

Template for skeleton device

***************************************************************************/

#include "emu.h"
#include "machine/segacdblock.h"



//**************************************************************************
//  GLOBAL VARIABLES
//**************************************************************************

// device type definition
const device_type SEGACDBLOCK = &device_creator<segacdblock_device>;

static ADDRESS_MAP_START( map, AS_0, 32, segacdblock_device )
ADDRESS_MAP_END

//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  segacdblock_device - constructor
//-------------------------------------------------

segacdblock_device::segacdblock_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
	: device_t(mconfig, SEGACDBLOCK, "Sega Saturn CD-Block (HLE)", tag, owner, clock, "segacdblock", __FILE__),
		device_memory_interface(mconfig, *this),
		m_space_config("segacdblock", ENDIANNESS_BIG, 32,32, 0, NULL, *ADDRESS_MAP_NAME(map))
{
}


const address_space_config *segacdblock_device::memory_space_config(address_spacenum spacenum) const
{
	return (spacenum == 0) ? &m_space_config : NULL;
}

//-------------------------------------------------
//  device_validity_check - perform validity checks
//  on this device
//-------------------------------------------------

void segacdblock_device::device_validity_check(validity_checker &valid) const
{
}


//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void segacdblock_device::device_start()
{
	m_space = &space(AS_0);
}


//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void segacdblock_device::device_reset()
{
}


//**************************************************************************
//  READ/WRITE HANDLERS
//**************************************************************************

READ32_MEMBER( segacdblock_device::read )
{
	return m_space->read_dword(offset);
}

WRITE32_MEMBER( segacdblock_device::write )
{
	m_space->write_dword(offset,data);
}
