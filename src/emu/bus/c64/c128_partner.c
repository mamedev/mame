// license:BSD-3-Clause
// copyright-holders:Curt Coder
/**********************************************************************

    Timeworks PARTNER 128 cartridge emulation

**********************************************************************/

/*

    PCB Layout
    ----------

	|---------------|
	|LS74  SW     CN|
	|LS09      LS273|
	|LS139   RAM    |
	|LS133          |
	|     LS240     |
	|LS33    ROM    |
	|LS09           |
	 |||||||||||||||

    ROM     - Toshiba TMM24128AP 16Kx8 EPROM (blank label)
    RAM     - Sony CXK5864PN-15L 8Kx8 SRAM
    SW      - push button switch
    CN      - lead out to joystick port dongle

*/

#include "c128_partner.h"



//**************************************************************************
//  DEVICE DEFINITIONS
//**************************************************************************

const device_type C128_PARTNER = &device_creator<partner128_t>;


//-------------------------------------------------
//  INPUT_PORTS( c128_partner )
//-------------------------------------------------

WRITE_LINE_MEMBER( partner128_t::nmi_w )
{
}

static INPUT_PORTS_START( c128_partner )
	PORT_START("NMI")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_NAME("Menu") PORT_CODE(KEYCODE_F11) PORT_WRITE_LINE_DEVICE_MEMBER(DEVICE_SELF, partner128_t, nmi_w)
INPUT_PORTS_END


//-------------------------------------------------
//  input_ports - device-specific input ports
//-------------------------------------------------

ioport_constructor partner128_t::device_input_ports() const
{
	return INPUT_PORTS_NAME( c128_partner );
}



//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  partner128_t - constructor
//-------------------------------------------------

partner128_t::partner128_t(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock) :
	device_t(mconfig, C128_PARTNER, "PARTNER 128", tag, owner, clock, "c128_partner", __FILE__),
	device_c64_expansion_card_interface(mconfig, *this),
	device_vcs_control_port_interface(mconfig, *this),
	m_ram(*this, "ram")
{
}


//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void partner128_t::device_start()
{
	// allocate memory
	m_ram.allocate(0x2000);
}


//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void partner128_t::device_reset()
{
}


//-------------------------------------------------
//  c64_cd_r - cartridge data read
//-------------------------------------------------

UINT8 partner128_t::c64_cd_r(address_space &space, offs_t offset, UINT8 data, int sphi2, int ba, int roml, int romh, int io1, int io2)
{
	return data;
}


//-------------------------------------------------
//  c64_cd_w - cartridge data write
//-------------------------------------------------

void partner128_t::c64_cd_w(address_space &space, offs_t offset, UINT8 data, int sphi2, int ba, int roml, int romh, int io1, int io2)
{
}


//-------------------------------------------------
//  c64_game_r - GAME read
//-------------------------------------------------

int partner128_t::c64_game_r(offs_t offset, int sphi2, int ba, int rw)
{
	return 1;
}
