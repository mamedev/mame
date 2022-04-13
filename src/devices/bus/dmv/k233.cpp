// license:BSD-3-Clause
// copyright-holders:Sandro Ronco
// thanks-to:rfka01
/***************************************************************************

    K233 16K Shared RAM

***************************************************************************/

#include "emu.h"
#include "k233.h"


/***************************************************************************
    IMPLEMENTATION
***************************************************************************/

//**************************************************************************
//  GLOBAL VARIABLES
//**************************************************************************

DEFINE_DEVICE_TYPE(DMV_K233, dmv_k233_device, "dmv_k233", "K233 16K Shared RAM")

//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  dmv_k233_device - constructor
//-------------------------------------------------

dmv_k233_device::dmv_k233_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, DMV_K233, tag, owner, clock)
	, device_dmvslot_interface(mconfig, *this)
	, m_enabled(false)
	, m_ram(nullptr)
{
}

//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void dmv_k233_device::device_start()
{
	m_ram = machine().memory().region_alloc( "sharedram", 0x4000, 1, ENDIANNESS_LITTLE )->base();

	// register for state saving
	save_item(NAME(m_enabled));
	save_pointer(NAME(m_ram), 0x4000);
}

//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void dmv_k233_device::device_reset()
{
	m_enabled = false;
}

void dmv_k233_device::io_write(int ifsel, offs_t offset, uint8_t data)
{
	if (ifsel == 1)
		m_enabled = !m_enabled;
}

//-------------------------------------------------
//  read
//-------------------------------------------------

bool dmv_k233_device::read(offs_t offset, uint8_t &data)
{
	if (m_enabled && offset >= 0xc000 && offset < 0x10000)
	{
		data = m_ram[offset & 0x3fff];
		return true;
	}

	return false;
}

//-------------------------------------------------
//  write
//-------------------------------------------------

bool dmv_k233_device::write(offs_t offset, uint8_t data)
{
	if (m_enabled && offset >= 0xc000 && offset < 0x10000)
	{
		m_ram[offset & 0x3fff] = data;
		return true;
	}

	return false;
}
