// license:BSD-3-Clause
// copyright-holders:Curt Coder
/**********************************************************************

    Sinclair QL standard ROM cartridge emulation

**********************************************************************/

#include "std.h"



//**************************************************************************
//  DEVICE DEFINITIONS
//**************************************************************************

const device_type QL_STANDARD_ROM_CARTRIDGE = &device_creator<ql_standard_rom_cartridge_t>;



//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  ql_standard_rom_cartridge_t - constructor
//-------------------------------------------------

ql_standard_rom_cartridge_t::ql_standard_rom_cartridge_t(const machine_config &mconfig, std::string tag, device_t *owner, UINT32 clock) :
	device_t(mconfig, QL_STANDARD_ROM_CARTRIDGE, "QL standard ROM cartridge", tag, owner, clock, "ql_standard", __FILE__),
	device_ql_rom_cartridge_card_interface(mconfig, *this)
{
}


//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void ql_standard_rom_cartridge_t::device_start()
{
}


//-------------------------------------------------
//  read - cartridge data read
//-------------------------------------------------

UINT8 ql_standard_rom_cartridge_t::read(address_space &space, offs_t offset, UINT8 data)
{
	if (m_romoeh && m_rom.bytes())
	{
		data = m_rom[offset & m_rom.mask()];
	}

	return data;
}
