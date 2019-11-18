// license:BSD-3-Clause
// copyright-holders:Curt Coder
/**********************************************************************

    Commodore VIC-10 Standard 8K/16K ROM Cartridge emulation

**********************************************************************/

#include "emu.h"
#include "std.h"



//**************************************************************************
//  DEVICE DEFINITIONS
//**************************************************************************

DEFINE_DEVICE_TYPE(VIC10_STD, vic10_standard_cartridge_device, "vic10_standard", "VIC-10 Standard Cartridge")



//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  vic10_standard_cartridge_device - constructor
//-------------------------------------------------

vic10_standard_cartridge_device::vic10_standard_cartridge_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, VIC10_STD, tag, owner, clock), device_vic10_expansion_card_interface(mconfig, *this)
{
}


//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void vic10_standard_cartridge_device::device_start()
{
}


//-------------------------------------------------
//  vic10_cd_r - cartridge data read
//-------------------------------------------------

uint8_t vic10_standard_cartridge_device::vic10_cd_r(offs_t offset, uint8_t data, int lorom, int uprom, int exram)
{
	if (!lorom && m_lorom.bytes())
	{
		data = m_lorom[offset & m_lorom.mask()];
	}
	else if (!exram && m_exram.bytes())
	{
		data = m_exram[offset & m_exram.mask()];
	}
	else if (!uprom && m_uprom.bytes())
	{
		data = m_uprom[offset & m_uprom.mask()];
	}

	return data;
}


//-------------------------------------------------
//  vic10_cd_w - cartridge data write
//-------------------------------------------------

void vic10_standard_cartridge_device::vic10_cd_w(offs_t offset, uint8_t data, int lorom, int uprom, int exram)
{
	if (!exram && m_exram.bytes())
	{
		m_exram[offset & m_exram.mask()] = data;
	}
}
