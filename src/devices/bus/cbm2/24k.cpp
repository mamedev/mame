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
	m_ram(*this, "ram", 0x6000, ENDIANNESS_LITTLE)
{
}


//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void cbm2_24k_cartridge_device::device_start()
{
}


//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void cbm2_24k_cartridge_device::device_reset()
{
	m_slot->bank1().install_ram(0x0000, 0x1fff, &m_ram[0x0000]);
	m_slot->bank2().install_ram(0x0000, 0x1fff, &m_ram[0x2000]);
	m_slot->bank3().install_ram(0x0000, 0x1fff, &m_ram[0x4000]);
}
