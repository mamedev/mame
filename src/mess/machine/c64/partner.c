/**********************************************************************

    Timeworks PARTNER 64 cartridge emulation

    Copyright MESS Team.
    Visit http://mamedev.org for licensing and usage restrictions.

**********************************************************************/

/*

    PCB Layout
    ----------

    |===========================|
    |=|                         |
    |=|LS05 LS09 LS00     HC74  |
    |=|                         |
    |=|                         |
    |=|   ROM       RAM         |
    |=|       LS133             |
    |=|                   LS156 |
    |=|                         |
    |===========================|

    ROM     - General Instrument 27C128-25 16Kx8 EPROM "TIMEWORKS C-64 VER 2-16-87"
    RAM     - Sony CXK5864PN-15L 8Kx8 SRAM

*/

#include "partner.h"



//**************************************************************************
//  DEVICE DEFINITIONS
//**************************************************************************

const device_type C64_PARTNER = &device_creator<c64_partner_cartridge_device>;


//-------------------------------------------------
//  INPUT_PORTS( c64_partner )
//-------------------------------------------------

static INPUT_PORTS_START( c64_partner )
	PORT_START("RESET")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_NAME("Reset") PORT_CODE(KEYCODE_F11) PORT_WRITE_LINE_DEVICE_MEMBER(DEVICE_SELF_OWNER, c64_expansion_slot_device, reset_w)
INPUT_PORTS_END


//-------------------------------------------------
//  input_ports - device-specific input ports
//-------------------------------------------------

ioport_constructor c64_partner_cartridge_device::device_input_ports() const
{
	return INPUT_PORTS_NAME( c64_partner );
}



//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  c64_partner_cartridge_device - constructor
//-------------------------------------------------

c64_partner_cartridge_device::c64_partner_cartridge_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock) :
	device_t(mconfig, C64_PARTNER, "C64 PARTNER 64 cartridge", tag, owner, clock, "c64_partner", __FILE__),
	device_c64_expansion_card_interface(mconfig, *this)
{
}


//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void c64_partner_cartridge_device::device_start()
{
	// allocate memory
	c64_ram_pointer(machine(), 0x2000);
}


//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void c64_partner_cartridge_device::device_reset()
{
}


//-------------------------------------------------
//  c64_cd_r - cartridge data read
//-------------------------------------------------

UINT8 c64_partner_cartridge_device::c64_cd_r(address_space &space, offs_t offset, UINT8 data, int sphi2, int ba, int roml, int romh, int io1, int io2)
{
	return data;
}


//-------------------------------------------------
//  c64_cd_w - cartridge data write
//-------------------------------------------------

void c64_partner_cartridge_device::c64_cd_w(address_space &space, offs_t offset, UINT8 data, int sphi2, int ba, int roml, int romh, int io1, int io2)
{
}


//-------------------------------------------------
//  c64_exrom_r - EXROM read
//-------------------------------------------------

int c64_partner_cartridge_device::c64_exrom_r(offs_t offset, int sphi2, int ba, int rw)
{
	return m_exrom;
}


//-------------------------------------------------
//  c64_game_r - GAME read
//-------------------------------------------------

int c64_partner_cartridge_device::c64_game_r(offs_t offset, int sphi2, int ba, int rw)
{
	return m_game;
}
