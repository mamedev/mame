/**********************************************************************

    Commodore 128 COMAL 80 cartridge emulation

    Copyright MESS Team.
    Visit http://mamedev.org for licensing and usage restrictions.

**********************************************************************/

#include "c128_comal80.h"



//**************************************************************************
//  DEVICE DEFINITIONS
//**************************************************************************

const device_type C128_COMAL80 = &device_creator<c128_comal80_cartridge_device>;



//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  c128_comal80_cartridge_device - constructor
//-------------------------------------------------

c128_comal80_cartridge_device::c128_comal80_cartridge_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock) :
	device_t(mconfig, C128_COMAL80, "C128 COMAL 80 cartridge", tag, owner, clock),
	device_c64_expansion_card_interface(mconfig, *this)
{
}


//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void c128_comal80_cartridge_device::device_start()
{
	// state saving
	save_item(NAME(m_bank));
}


//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void c128_comal80_cartridge_device::device_reset()
{
	m_bank = 0;
}


//-------------------------------------------------
//  c64_cd_r - cartridge data read
//-------------------------------------------------

UINT8 c128_comal80_cartridge_device::c64_cd_r(address_space &space, offs_t offset, UINT8 data, int ba, int roml, int romh, int io1, int io2)
{
	if (!romh)
	{
		offs_t addr = (m_bank << 14) | (offset & 0x3fff);
		data = m_roml[addr];
	}

	return data;
}


//-------------------------------------------------
//  c64_cd_w - cartridge data write
//-------------------------------------------------

void c128_comal80_cartridge_device::c64_cd_w(address_space &space, offs_t offset, UINT8 data, int ba, int roml, int romh, int io1, int io2)
{
	if (!io1)
	{
		/*

            bit     description

            0
            1
            2
            3
            4       A14
            5       ROM select bit 0
            6       ROM select bit 1
            7

        */

		m_bank = (data >> 4) & 0x07;
	}
}
