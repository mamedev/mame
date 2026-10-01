// license:BSD-3-Clause
// copyright-holders:Curt Coder
/**********************************************************************

    Commodore VIC-1210 3K RAM Expansion Cartridge emulation
    Commodore VIC-1211A Super Expander with 3K RAM Cartridge emulation

**********************************************************************/

#include "emu.h"
#include "vic1210.h"



//**************************************************************************
//  DEVICE DEFINITIONS
//**************************************************************************

DEFINE_DEVICE_TYPE(VIC1210, vic1210_device, "vic1210", "VIC-1210 3K RAM Expansion")



//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  vic1210_device - constructor
//-------------------------------------------------

vic1210_device::vic1210_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, VIC1210, tag, owner, clock)
	, device_vic20_expansion_card_interface(mconfig, *this)
	, m_ram(*this, "ram", 0xc00, ENDIANNESS_LITTLE)
{
}


//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void vic1210_device::device_start()
{
	m_slot->ram1().install_ram(0x000, 0x3ff, &m_ram[0x000]);
	m_slot->ram2().install_ram(0x000, 0x3ff, &m_ram[0x400]);
	m_slot->ram3().install_ram(0x000, 0x3ff, &m_ram[0x800]);
}


//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void vic1210_device::device_reset()
{
	memory_region *const blk5 = m_slot->memregion("blk5");

	if (blk5)
		m_slot->blk5().install_rom(0x0000, 0x0fff, 0x1000, blk5->base());
}
