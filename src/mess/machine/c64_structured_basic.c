/**********************************************************************

    Structured Basic cartridge emulation

    Copyright MESS Team.
    Visit http://mamedev.org for licensing and usage restrictions.

**********************************************************************/

#include "c64_structured_basic.h"



//**************************************************************************
//  DEVICE DEFINITIONS
//**************************************************************************

const device_type C64_STRUCTURED_BASIC = &device_creator<c64_structured_basic_cartridge_device>;



//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  c64_structured_basic_cartridge_device - constructor
//-------------------------------------------------

c64_structured_basic_cartridge_device::c64_structured_basic_cartridge_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock) :
	device_t(mconfig, C64_STRUCTURED_BASIC, "C64 Structured Basic cartridge", tag, owner, clock),
	device_c64_expansion_card_interface(mconfig, *this)
{
}


//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void c64_structured_basic_cartridge_device::device_start()
{
	// state saving
	save_item(NAME(m_bank));
}


//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void c64_structured_basic_cartridge_device::device_reset()
{
	m_exrom = 0;

	m_bank = 0;
}


//-------------------------------------------------
//  c64_cd_r - cartridge data read
//-------------------------------------------------

UINT8 c64_structured_basic_cartridge_device::c64_cd_r(address_space &space, offs_t offset, int ba, int roml, int romh, int io1, int io2)
{
	UINT8 data = 0;

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

void c64_structured_basic_cartridge_device::c64_cd_w(address_space &space, offs_t offset, UINT8 data, int ba, int roml, int romh, int io1, int io2)
{
	if (!io1)
	{
		switch (data & 0x03)
		{
		case 0:
		case 1:
			m_exrom = 0;
			m_bank = 0;
			break;

		case 2:
			m_exrom = 0;
			m_bank = 1;
			break;

		case 3:
			m_exrom = 1;
			m_bank = 0;
			break;
		}
	}
}
