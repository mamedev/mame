// license:BSD-3-Clause
// copyright-holders:Curt Coder
/**********************************************************************

    Sinclair QL standard ROM cartridge emulation

**********************************************************************/

#include "emu.h"
#include "std.h"



//**************************************************************************
//  DEVICE DEFINITIONS
//**************************************************************************

DEFINE_DEVICE_TYPE(QL_STANDARD_ROM_CARTRIDGE, ql_standard_rom_cartridge_device, "ql_standard", "QL standard ROM cartridge")



//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  ql_standard_rom_cartridge_device - constructor
//-------------------------------------------------

ql_standard_rom_cartridge_device::ql_standard_rom_cartridge_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	device_t(mconfig, QL_STANDARD_ROM_CARTRIDGE, tag, owner, clock),
	device_ql_rom_cartridge_card_interface(mconfig, *this)
{
}


//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void ql_standard_rom_cartridge_device::device_start()
{
}


//-------------------------------------------------
//  read - cartridge data read
//-------------------------------------------------

uint8_t ql_standard_rom_cartridge_device::read(offs_t offset, uint8_t data)
{
	if (m_romoeh && m_rom.bytes())
	{
		data = m_rom[offset & m_rom.mask()];
	}

	return data;
}
