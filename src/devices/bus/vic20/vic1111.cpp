// license:BSD-3-Clause
// copyright-holders:Curt Coder
/**********************************************************************

    Commodore VIC-1111 16K RAM Expansion Cartridge emulation

**********************************************************************/

#include "emu.h"
#include "vic1111.h"



//**************************************************************************
//  DEVICE DEFINITIONS
//**************************************************************************

DEFINE_DEVICE_TYPE(VIC1111, vic1111_device, "vic1111", "VIC-1111 16K RAM Expansion")



//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  vic1111_device - constructor
//-------------------------------------------------

vic1111_device::vic1111_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, VIC1111, tag, owner, clock)
	, device_vic20_expansion_card_interface(mconfig, *this)
	, m_ram(*this, "ram", 0x4000, ENDIANNESS_LITTLE)
{
}


//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void vic1111_device::device_start()
{
	m_slot->blk1().install_ram(0x0000, 0x1fff, &m_ram[0x0000]);
	m_slot->blk2().install_ram(0x0000, 0x1fff, &m_ram[0x2000]);
}
