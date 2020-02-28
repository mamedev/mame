// license:BSD-3-Clause
// copyright-holders:Curt Coder
/**********************************************************************

    PROPHET64 cartridge emulation

**********************************************************************/

#include "emu.h"
#include "prophet64.h"



//**************************************************************************
//  DEVICE DEFINITIONS
//**************************************************************************

DEFINE_DEVICE_TYPE(C64_PROPHET64, c64_prophet64_cartridge_device, "c64_prophet64", "PROPHET64")



//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  c64_prophet64_cartridge_device - constructor
//-------------------------------------------------

c64_prophet64_cartridge_device::c64_prophet64_cartridge_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	device_t(mconfig, C64_PROPHET64, tag, owner, clock),
	device_c64_expansion_card_interface(mconfig, *this),
	m_bank(0)
{
}


//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void c64_prophet64_cartridge_device::device_start()
{
	// state saving
	save_item(NAME(m_bank));
}


//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void c64_prophet64_cartridge_device::device_reset()
{
	m_bank = 0;
	m_exrom = 0;
}


//-------------------------------------------------
//  c64_cd_r - cartridge data read
//-------------------------------------------------

uint8_t c64_prophet64_cartridge_device::c64_cd_r(offs_t offset, uint8_t data, int sphi2, int ba, int roml, int romh, int io1, int io2)
{
	if (!roml)
	{
		offs_t addr = (m_bank << 13) | (offset & 0x1fff);
		data = m_roml[addr];
	}

	return data;
}


//-------------------------------------------------
//  c64_cd_w - cartridge data write
//-------------------------------------------------

void c64_prophet64_cartridge_device::c64_cd_w(offs_t offset, uint8_t data, int sphi2, int ba, int roml, int romh, int io1, int io2)
{
	if (!io2)
	{
		m_bank = data & 0x1f;
		m_exrom = BIT(data, 5);
	}
}
