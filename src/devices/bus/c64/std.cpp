// license:BSD-3-Clause
// copyright-holders:Curt Coder
/**********************************************************************

    Commodore 64 Standard 8K/16K cartridge emulation

**********************************************************************/

#include "emu.h"
#include "std.h"



//**************************************************************************
//  DEVICE DEFINITIONS
//**************************************************************************

DEFINE_DEVICE_TYPE(C64_STD, c64_standard_cartridge_device, "c64_standard", "C64 standard cartridge")



//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  c64_standard_cartridge_device - constructor
//-------------------------------------------------

c64_standard_cartridge_device::c64_standard_cartridge_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	device_t(mconfig, C64_STD, tag, owner, clock),
	device_c64_expansion_card_interface(mconfig, *this)
{
}


//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void c64_standard_cartridge_device::device_start()
{
}


//-------------------------------------------------
//  c64_cd_r - cartridge data read
//-------------------------------------------------

uint8_t c64_standard_cartridge_device::c64_cd_r(offs_t offset, uint8_t data, int sphi2, int ba, int roml, int romh, int io1, int io2)
{
	if (!roml && m_roml.bytes())
	{
		data = m_roml[offset & m_roml.mask()];
	}
	else if (!romh)
	{
		if (m_romh.bytes())
		{
			data = m_romh[offset & m_romh.mask()];
		}
		else if (m_roml.mask() == 0x3fff)
		{
			data = m_roml[offset & m_roml.mask()];
		}
	}

	return data;
}
