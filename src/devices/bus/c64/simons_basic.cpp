// license:BSD-3-Clause
// copyright-holders:Curt Coder
/**********************************************************************

    VizaWrite 64 cartridge emulation

**********************************************************************/

#include "emu.h"
#include "simons_basic.h"



//**************************************************************************
//  DEVICE DEFINITIONS
//**************************************************************************

DEFINE_DEVICE_TYPE(C64_SIMONS_BASIC, c64_simons_basic_cartridge_device, "c64_simons_basic", "C64 Simons' BASIC")



//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  c64_simons_basic_cartridge_device - constructor
//-------------------------------------------------

c64_simons_basic_cartridge_device::c64_simons_basic_cartridge_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	device_t(mconfig, C64_SIMONS_BASIC, tag, owner, clock),
	device_c64_expansion_card_interface(mconfig, *this)
{
}


//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void c64_simons_basic_cartridge_device::device_start()
{
}


//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void c64_simons_basic_cartridge_device::device_reset()
{
	m_game = 1;
}


//-------------------------------------------------
//  c64_cd_r - cartridge data read
//-------------------------------------------------

uint8_t c64_simons_basic_cartridge_device::c64_cd_r(offs_t offset, uint8_t data, int sphi2, int ba, int roml, int romh, int io1, int io2)
{
	if (!roml)
	{
		data = m_roml[offset & 0x1fff];
	}
	else if (!romh)
	{
		data = m_romh[offset & 0x1fff];
	}

	return data;
}


//-------------------------------------------------
//  c64_cd_w - cartridge data write
//-------------------------------------------------

void c64_simons_basic_cartridge_device::c64_cd_w(offs_t offset, uint8_t data, int sphi2, int ba, int roml, int romh, int io1, int io2)
{
	if (!io1)
	{
		m_game = !BIT(data, 0);
	}
}
