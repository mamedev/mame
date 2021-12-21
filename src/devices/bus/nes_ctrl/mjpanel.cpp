// license:BSD-3-Clause
// copyright-holders:Fabio Priuli
/**********************************************************************

    Nintendo Family Computer Capcom Mahjong Controller

    There are two versions of this controller, one packed with the 1st
    and 2nd Ide Yousoku Meijin no Jissen Mahjong, respectively. Both
    have the original game's serial "CAP-IM" on back. Controller serials
    HC-01 or HC-02 appear on revisions of the 1st game's controller.
    HC-01 also appears on the 2nd game's controller. At least one
    version of the original controller has a PCB marked 0827-06 HORI.

**********************************************************************/

#include "emu.h"
#include "mjpanel.h"

//**************************************************************************
//  DEVICE DEFINITIONS
//**************************************************************************

DEFINE_DEVICE_TYPE(NES_MJPANEL, nes_mjpanel_device, "nes_mjpanel", "Capcom Mahjong Controller")


static INPUT_PORTS_START( nes_mjpanel )
	PORT_START("MJPANEL.0")
	PORT_BIT( 0xff, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("MJPANEL.1")
	PORT_BIT( 0x03, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_MAHJONG_N ) PORT_NAME("%p Mahjong N / Circle")
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_MAHJONG_M ) PORT_NAME("%p Mahjong M / Square")
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_MAHJONG_L )
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_MAHJONG_K )
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_MAHJONG_J )
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_MAHJONG_I )

	PORT_START("MJPANEL.2")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_MAHJONG_H ) PORT_NAME("%p Mahjong H / Right")
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_MAHJONG_G ) PORT_NAME("%p Mahjong G / Up")
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_MAHJONG_F ) PORT_NAME("%p Mahjong F / Left")
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_MAHJONG_E )
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_MAHJONG_D )
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_MAHJONG_C )
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_MAHJONG_B )
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_MAHJONG_A )

	PORT_START("MJPANEL.3")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_MAHJONG_RON )
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_MAHJONG_REACH )
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_MAHJONG_CHI )
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_MAHJONG_PON )
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_MAHJONG_KAN ) PORT_NAME("%p Mahjong Kan / Right")
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_START ) PORT_NAME("%p Start / Down")
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_SELECT ) PORT_NAME("%p Select / Left")
INPUT_PORTS_END


//-------------------------------------------------
//  input_ports - device-specific input ports
//-------------------------------------------------

ioport_constructor nes_mjpanel_device::device_input_ports() const
{
	return INPUT_PORTS_NAME( nes_mjpanel );
}



//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  nes_mjpanel_device - constructor
//-------------------------------------------------

nes_mjpanel_device::nes_mjpanel_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: device_t(mconfig, NES_MJPANEL, tag, owner, clock)
	, device_nes_control_port_interface(mconfig, *this)
	, m_panel(*this, "MJPANEL.%u", 0)
	, m_latch(0)
	, m_row(0)
{
}


//-------------------------------------------------
//  device_start
//-------------------------------------------------

void nes_mjpanel_device::device_start()
{
	save_item(NAME(m_latch));
	save_item(NAME(m_row));
	save_item(NAME(m_strobe));
}


//-------------------------------------------------
//  read
//-------------------------------------------------

u8 nes_mjpanel_device::read_exp(offs_t offset)
{
	u8 ret = 0;
	if (offset)    // $4017
	{
		if (m_strobe)
			set_latch();
		ret = (m_latch & 1) << 1;
		m_latch >>= 1;
	}
	return ret;
}

//-------------------------------------------------
//  write
//-------------------------------------------------

void nes_mjpanel_device::write(u8 data)
{
	m_row = (data >> 1) & 0x03;
	if (write_strobe(data))
		set_latch();
}

void nes_mjpanel_device::set_latch()
{
	if (m_row)
		m_latch = m_panel[m_row]->read();
	else  // hardware behavior of the apparently unused input row
		m_latch = m_panel[1]->read() | m_panel[2]->read();
}
