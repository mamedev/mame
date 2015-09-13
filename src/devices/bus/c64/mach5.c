// license:BSD-3-Clause
// copyright-holders:Curt Coder
/**********************************************************************

    Access Software MACH 5 cartridge emulation

**********************************************************************/

#include "mach5.h"



//**************************************************************************
//  DEVICE DEFINITIONS
//**************************************************************************

const device_type C64_MACH5 = &device_creator<c64_mach5_cartridge_device>;


//-------------------------------------------------
//  INPUT_PORTS( c64_mach5 )
//-------------------------------------------------

static INPUT_PORTS_START( c64_mach5 )
	PORT_START("S1")
	PORT_DIPNAME( 0x01, 0x00, "Mode" )
	PORT_DIPSETTING(    0x00, "C64" )
	PORT_DIPSETTING(    0x01, "C128" )

	PORT_START("S2")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("Reset") PORT_CODE(KEYCODE_F11) PORT_WRITE_LINE_DEVICE_MEMBER(DEVICE_SELF_OWNER, c64_expansion_slot_device, reset_w)
INPUT_PORTS_END


//-------------------------------------------------
//  input_ports - device-specific input ports
//-------------------------------------------------

ioport_constructor c64_mach5_cartridge_device::device_input_ports() const
{
	return INPUT_PORTS_NAME( c64_mach5 );
}



//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  c64_mach5_cartridge_device - constructor
//-------------------------------------------------

c64_mach5_cartridge_device::c64_mach5_cartridge_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock) :
	device_t(mconfig, C64_MACH5, "C64 MACH5 cartridge", tag, owner, clock, "c64_mach5", __FILE__),
	device_c64_expansion_card_interface(mconfig, *this),
	m_s1(*this, "S1")
{
}


//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void c64_mach5_cartridge_device::device_start()
{
}


//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void c64_mach5_cartridge_device::device_reset()
{
	m_c128 = m_s1->read();

	if (!m_c128)
	{
		m_exrom = 0;
	}
}


//-------------------------------------------------
//  c64_cd_r - cartridge data read
//-------------------------------------------------

UINT8 c64_mach5_cartridge_device::c64_cd_r(address_space &space, offs_t offset, UINT8 data, int sphi2, int ba, int roml, int romh, int io1, int io2)
{
	if (!roml || !romh || !io1 || !io2)
	{
		data = m_roml[offset & 0x1fff];
	}

	return data;
}


//-------------------------------------------------
//  c64_cd_w - cartridge data write
//-------------------------------------------------

void c64_mach5_cartridge_device::c64_cd_w(address_space &space, offs_t offset, UINT8 data, int sphi2, int ba, int roml, int romh, int io1, int io2)
{
	if (!m_c128)
	{
		if (!io1)
		{
			m_exrom = 0;
		}
		else if (!io2)
		{
			m_exrom = 1;
		}
	}
}
