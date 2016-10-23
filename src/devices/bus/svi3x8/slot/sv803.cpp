// license:GPL-2.0+
// copyright-holders:Dirk Best
/***************************************************************************

    SV-803 16k memory expansion for SVI-318

***************************************************************************/

#include "sv803.h"


//**************************************************************************
//  DEVICE DEFINITIONS
//**************************************************************************

const device_type SV803 = &device_creator<sv803_device>;


//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  sv803_device - constructor
//-------------------------------------------------

sv803_device::sv803_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	device_t(mconfig, SV803, "SV-803 16k RAM Cartridge", tag, owner, clock, "sv803", __FILE__),
	device_svi_slot_interface(mconfig, *this)
{
	m_ram = std::make_unique<uint8_t[]>(0x4000);
	memset(m_ram.get(), 0xff, 0x4000);
}

//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void sv803_device::device_start()
{
	// register for savestates
	save_pointer(NAME(m_ram.get()), 0x4000);
}

//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void sv803_device::device_reset()
{
}


//**************************************************************************
//  IMPLEMENTATION
//**************************************************************************

uint8_t sv803_device::mreq_r(address_space &space, offs_t offset, uint8_t mem_mask)
{
	if (offset >= 0x8000 && offset <= 0xbfff)
		return m_ram[offset - 0x8000];

	return 0xff;
}

void sv803_device::mreq_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask)
{
	if (offset >= 0x8000 && offset <= 0xbfff)
		m_ram[offset - 0x8000] = data;
}
