// license:BSD-3-Clause
// copyright-holders:Curt Coder
/**********************************************************************

    C64 switchable 8K cartridge emulation

**********************************************************************/

/*

    4040 + Fast Hack'em

    PCB Layout
    ----------

    |===========================|
    |=|                         |
    |=|                     SW1 |
    |=|         ROM0            |
    |=|                         |
    |=|                         |
    |=|         ROM1            |
    |=|                         |
    |=|                         |
    |===========================|

    ROM0,1  - National Semiconductor NMC27C64Q 8Kx8 EPROM
    SW1     - ROM selection switch

*/

#include "sw8k.h"



//**************************************************************************
//  DEVICE DEFINITIONS
//**************************************************************************

const device_type C64_SW8K = &device_creator<c64_switchable_8k_cartridge_device>;


//-------------------------------------------------
//  INPUT_PORTS( c64_easyflash )
//-------------------------------------------------

static INPUT_PORTS_START( c64_switchable_8k )
	PORT_START("SW")
	PORT_DIPNAME( 0x01, 0x00, "ROM Select" )
	PORT_DIPSETTING(    0x00, "ROM 0" )
	PORT_DIPSETTING(    0x01, "ROM 1" )
INPUT_PORTS_END


//-------------------------------------------------
//  input_ports - device-specific input ports
//-------------------------------------------------

ioport_constructor c64_switchable_8k_cartridge_device::device_input_ports() const
{
	return INPUT_PORTS_NAME( c64_switchable_8k );
}



//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  c64_switchable_8k_cartridge_device - constructor
//-------------------------------------------------

c64_switchable_8k_cartridge_device::c64_switchable_8k_cartridge_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock) :
	device_t(mconfig, C64_SW8K, "C64 Switchable 8K cartridge", tag, owner, clock, "c64_sw8k", __FILE__),
	device_c64_expansion_card_interface(mconfig, *this),
	m_sw(*this, "SW")
{
}


//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void c64_switchable_8k_cartridge_device::device_start()
{
}


//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void c64_switchable_8k_cartridge_device::device_reset()
{
	m_bank = m_sw->read();
}


//-------------------------------------------------
//  c64_cd_r - cartridge data read
//-------------------------------------------------

UINT8 c64_switchable_8k_cartridge_device::c64_cd_r(address_space &space, offs_t offset, UINT8 data, int sphi2, int ba, int roml, int romh, int io1, int io2)
{
	if (!roml)
	{
		offs_t addr = (m_bank << 13) | (offset & 0x1fff);
		data = m_roml[addr];
	}

	return data;
}
