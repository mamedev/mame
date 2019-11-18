// license:BSD-3-Clause
// copyright-holders:Curt Coder
/**********************************************************************

    VizaStar 64 XL4 cartridge emulation

**********************************************************************/

/*

    PCB Layout
    ----------
    RB84 (C) MICROPORT
    |===========================|
    |=|                         |
    |=|                         |
    |=|                         |
    |=|                         |
    |=|                   ROM   |
    |=|                         |
    |=|                         |
    |=|                         |
    |===========================|

    ROM     - Hitachi HN462732G 4Kx8 EPROM "V"

*/

#include "emu.h"
#include "vizastar.h"



//**************************************************************************
//  MACROS/CONSTANTS
//**************************************************************************

#define UNSCRAMBLE_ADDRESS(_offset) \
	bitswap<16>(_offset,15,14,13,12,5,0,7,10,11,9,8,6,4,3,2,1)

#define UNSCRAMBLE_DATA(_data) \
	bitswap<8>(_data,7,6,0,5,1,4,2,3)



//**************************************************************************
//  DEVICE DEFINITIONS
//**************************************************************************

DEFINE_DEVICE_TYPE(C64_VIZASTAR, c64_vizastar_cartridge_device, "c64_vizastar", "VizaStar 64 XL4")



//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  c64_vizastar_cartridge_device - constructor
//-------------------------------------------------

c64_vizastar_cartridge_device::c64_vizastar_cartridge_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	device_t(mconfig, C64_VIZASTAR, tag, owner, clock),
	device_c64_expansion_card_interface(mconfig, *this)
{
}


//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void c64_vizastar_cartridge_device::device_start()
{
}


//-------------------------------------------------
//  c64_cd_r - cartridge data read
//-------------------------------------------------

uint8_t c64_vizastar_cartridge_device::c64_cd_r(offs_t offset, uint8_t data, int sphi2, int ba, int roml, int romh, int io1, int io2)
{
	if (!roml)
	{
		data = UNSCRAMBLE_DATA(m_roml[UNSCRAMBLE_ADDRESS(offset & 0xfff)]);
	}

	return data;
}
