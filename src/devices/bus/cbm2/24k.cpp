// license:BSD-3-Clause
// copyright-holders:Curt Coder
/**********************************************************************

    GLA 24K RAM cartridge emulation

**********************************************************************/

#include "emu.h"
#include "24k.h"



//**************************************************************************
//  DEVICE DEFINITIONS
//**************************************************************************

DEFINE_DEVICE_TYPE(CBM2_24K, cbm2_24k_cartridge_device, "cbm2_24k", "CBM-II 24K RAM/ROM cartridge")



//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  cbm2_24k_cartridge_device - constructor
//-------------------------------------------------

cbm2_24k_cartridge_device::cbm2_24k_cartridge_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	device_t(mconfig, CBM2_24K, tag, owner, clock),
	device_cbm2_expansion_card_interface(mconfig, *this),
	m_ram(*this, "ram")
{
}


//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void cbm2_24k_cartridge_device::device_start()
{
	m_ram.allocate(0x6000);
}


//-------------------------------------------------
//  cbm2_bd_r - cartridge data read
//-------------------------------------------------

uint8_t cbm2_24k_cartridge_device::cbm2_bd_r(offs_t offset, uint8_t data, int csbank1, int csbank2, int csbank3)
{
	if (!csbank1)
	{
		data = m_ram[offset];
	}
	else if (!csbank2)
	{
		data = m_ram[0x2000 | offset];
	}
	else if (!csbank3)
	{
		data = m_ram[0x4000 | offset];
	}

	return data;
}


//-------------------------------------------------
//  cbm2_bd_w - cartridge data write
//-------------------------------------------------

void cbm2_24k_cartridge_device::cbm2_bd_w(offs_t offset, uint8_t data, int csbank1, int csbank2, int csbank3)
{
	if (!csbank1)
	{
		m_ram[offset] = data;
	}
	else if (!csbank2)
	{
		m_ram[0x2000 | offset] = data;
	}
	else if (!csbank3)
	{
		m_ram[0x4000 | offset] = data;
	}
}
