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

#include "vizastar.h"



//**************************************************************************
//  MACROS/CONSTANTS
//**************************************************************************

#define UNSCRAMBLE_ADDRESS(_offset) \
	BITSWAP16(_offset,15,14,13,12,5,0,7,10,11,9,8,6,4,3,2,1)

#define UNSCRAMBLE_DATA(_data) \
	BITSWAP8(_data,7,6,0,5,1,4,2,3)



//**************************************************************************
//  DEVICE DEFINITIONS
//**************************************************************************

const device_type C64_VIZASTAR = &device_creator<c64_vizastar_cartridge_device>;



//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  c64_vizastar_cartridge_device - constructor
//-------------------------------------------------

c64_vizastar_cartridge_device::c64_vizastar_cartridge_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock) :
	device_t(mconfig, C64_VIZASTAR, "VizaStar 64 XL4", tag, owner, clock, "c64_vizastar", __FILE__),
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

UINT8 c64_vizastar_cartridge_device::c64_cd_r(address_space &space, offs_t offset, UINT8 data, int sphi2, int ba, int roml, int romh, int io1, int io2)
{
	if (!roml)
	{
		data = UNSCRAMBLE_DATA(m_roml[UNSCRAMBLE_ADDRESS(offset & 0xfff)]);
	}

	return data;
}
