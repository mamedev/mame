/**********************************************************************

    Cinemaware Warp Speed cartridge emulation

    Copyright MESS Team.
    Visit http://mamedev.org for licensing and usage restrictions.

**********************************************************************/

/*

    PCB Layout
    ----------

    |===================|
    |=|             SW1 |
    |=|                 |
    |=|             SW2 |
    |=|                 |
    |=|                 |
    |=|    ROM          |
    |=|            LS109|
    |=|                 |
    |===================|

    ROM     - "DEI-356NR-WRPIIDA1 (C) NCR A8847 609-2415038 F833071" 16Kx8 ROM
    SW1     - mode switch (C64/C128)
    SW2     - reset button

*/

#include "c64_warp_speed.h"



//**************************************************************************
//  MACROS/CONSTANTS
//**************************************************************************

#define UNSCRAMBLE_ADDRESS(_offset) \
	BITSWAP16(_offset,15,14,12,13,5,2,7,9,11,10,8,6,4,3,1,0)

#define UNSCRAMBLE_DATA(_data) \
	BITSWAP8(_data,7,6,5,0,1,4,2,3)



//**************************************************************************
//  DEVICE DEFINITIONS
//**************************************************************************

const device_type C64_WARP_SPEED = &device_creator<c64_warp_speed_cartridge_device>;


//-------------------------------------------------
//  INPUT_PORTS( c64_warp_speed )
//-------------------------------------------------

INPUT_CHANGED_MEMBER( c64_warp_speed_cartridge_device::reset )
{
	if (!newval)
	{
		device_reset();
	}

	m_slot->reset_w(newval ? CLEAR_LINE : ASSERT_LINE);
}

static INPUT_PORTS_START( c64_warp_speed )
	PORT_START("SW1")
	PORT_DIPNAME( 0x01, 0x01, "Mode" )
	PORT_DIPSETTING(    0x01, "C64" )
	PORT_DIPSETTING(    0x00, "C128" )

	PORT_START("SW2")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("Reset") PORT_CODE(KEYCODE_F11) PORT_CHANGED_MEMBER(DEVICE_SELF, c64_warp_speed_cartridge_device, reset, 0)
INPUT_PORTS_END


//-------------------------------------------------
//  input_ports - device-specific input ports
//-------------------------------------------------

ioport_constructor c64_warp_speed_cartridge_device::device_input_ports() const
{
	return INPUT_PORTS_NAME( c64_warp_speed );
}



//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  c64_warp_speed_cartridge_device - constructor
//-------------------------------------------------

c64_warp_speed_cartridge_device::c64_warp_speed_cartridge_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock) :
	device_t(mconfig, C64_WARP_SPEED, "C64 Warp Speed cartridge", tag, owner, clock),
	device_c64_expansion_card_interface(mconfig, *this)
{
}


//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void c64_warp_speed_cartridge_device::device_start()
{
}


//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void c64_warp_speed_cartridge_device::device_reset()
{
	m_exrom = 0;
	m_game = 0;
}


//-------------------------------------------------
//  c64_cd_r - cartridge data read
//-------------------------------------------------

UINT8 c64_warp_speed_cartridge_device::c64_cd_r(address_space &space, offs_t offset, UINT8 data, int ba, int roml, int romh, int io1, int io2)
{
	if (!roml || !romh || !io1 || !io2)
	{
		offs_t addr = UNSCRAMBLE_ADDRESS(offset & 0x3fff);
		data = UNSCRAMBLE_DATA(m_roml[addr]);
	}

	return data;
}


//-------------------------------------------------
//  c64_cd_w - cartridge data write
//-------------------------------------------------

void c64_warp_speed_cartridge_device::c64_cd_w(address_space &space, offs_t offset, UINT8 data, int ba, int roml, int romh, int io1, int io2)
{
	if (!io1)
	{
		m_exrom = 0;
		m_game = 0;
	}
	else if (!io2)
	{
		m_exrom = 1;
		m_game = 1;
	}
}
