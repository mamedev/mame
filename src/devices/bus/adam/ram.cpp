// license:BSD-3-Clause
// copyright-holders:Curt Coder
/**********************************************************************

    Coleco Adam Internal 64KB RAM Expansion emulation

**********************************************************************/

#include "emu.h"
#include "ram.h"



//**************************************************************************
//  DEVICE DEFINITIONS
//**************************************************************************

DEFINE_DEVICE_TYPE(ADAM_RAM, adam_ram_expansion_device, "adam_ram", "Adam 64KB RAM expansion")



//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  adam_ram_expansion_device - constructor
//-------------------------------------------------

adam_ram_expansion_device::adam_ram_expansion_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	device_t(mconfig, ADAM_RAM, tag, owner, clock),
	device_adam_expansion_slot_card_interface(mconfig, *this),
	m_ram(*this, "ram")
{
}


//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void adam_ram_expansion_device::device_start()
{
	m_ram.allocate(0x10000);
}


//-------------------------------------------------
//  adam_bd_r - buffered data read
//-------------------------------------------------

uint8_t adam_ram_expansion_device::adam_bd_r(offs_t offset, uint8_t data, int bmreq, int biorq, int aux_rom_cs, int cas1, int cas2)
{
	if (!cas2)
	{
		data = m_ram[offset];
	}

	return data;
}


//-------------------------------------------------
//  adam_bd_w - buffered data write
//-------------------------------------------------

void adam_ram_expansion_device::adam_bd_w(offs_t offset, uint8_t data, int bmreq, int biorq, int aux_rom_cs, int cas1, int cas2)
{
	if (!cas2)
	{
		m_ram[offset] = data;
	}
}
