// license:BSD-3-Clause
// copyright-holders:Curt Coder
/**********************************************************************

    32K RAM Expansion Cartridge emulation

**********************************************************************/

#include "emu.h"
#include "32k.h"



//**************************************************************************
//  DEVICE DEFINITIONS
//**************************************************************************

DEFINE_DEVICE_TYPE(VIC20_32K, vic20_32k_device, "vic20_32k", "VIC-20 32K RAM Expansion")



//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  vic20_32k_device - constructor
//-------------------------------------------------

vic20_32k_device::vic20_32k_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, VIC20_32K, tag, owner, clock)
	, device_vic20_expansion_card_interface(mconfig, *this)
	, m_ram(*this, "ram", 0x8000, ENDIANNESS_LITTLE)
{
}


//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void vic20_32k_device::device_start()
{
	m_slot->blk1().install_ram(0x0000, 0x1fff, &m_ram[0x0000]);
	m_slot->blk2().install_ram(0x0000, 0x1fff, &m_ram[0x2000]);
	m_slot->blk3().install_ram(0x0000, 0x1fff, &m_ram[0x4000]);
	m_slot->blk5().install_ram(0x0000, 0x1fff, &m_ram[0x6000]);
}
