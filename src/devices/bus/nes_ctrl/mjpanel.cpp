// license:BSD-3-Clause
// copyright-holders:Fabio Priuli
/**********************************************************************

    Nintendo Family Computer Mahjong Panel

**********************************************************************/

#include "mjpanel.h"

//**************************************************************************
//  DEVICE DEFINITIONS
//**************************************************************************

const device_type NES_MJPANEL = &device_creator<nes_mjpanel_device>;


static INPUT_PORTS_START( nes_mjpanel )
	PORT_START("MJPANEL.0")
	PORT_BIT( 0xff, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("MJPANEL.1")
	PORT_BIT( 0x03, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_MAHJONG_N )
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_MAHJONG_M )
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_MAHJONG_L )
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_MAHJONG_K )
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_MAHJONG_J )
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_MAHJONG_I )

	PORT_START("MJPANEL.2")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_MAHJONG_H )
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_MAHJONG_G )
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_MAHJONG_F )
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
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_MAHJONG_KAN )
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_SELECT ) PORT_NAME("Mahjong Select")
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_START ) PORT_NAME("Mahjong Start")
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

nes_mjpanel_device::nes_mjpanel_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock) :
					device_t(mconfig, NES_MJPANEL, "Famicom Mahjong Panel", tag, owner, clock, "nes_mjpanel", __FILE__),
					device_nes_control_port_interface(mconfig, *this),
					m_panel(*this, "MJPANEL"), m_latch(0)
{
}


//-------------------------------------------------
//  device_start
//-------------------------------------------------

void nes_mjpanel_device::device_start()
{
	save_item(NAME(m_latch));
}


//-------------------------------------------------
//  device_reset
//-------------------------------------------------

void nes_mjpanel_device::device_reset()
{
	m_latch = 0;
}


//-------------------------------------------------
//  read
//-------------------------------------------------

UINT8 nes_mjpanel_device::read_exp(offs_t offset)
{
	UINT8 ret = 0;
	if (offset)
	{
		ret = (m_latch & 1) << 1;
		m_latch >>= 1;
	}
	else
		logerror("Error: Mahjong panel read from $4016\n");

	return ret;
}

//-------------------------------------------------
//  write
//-------------------------------------------------

void nes_mjpanel_device::write(UINT8 data)
{
	if (data & 0x01)
		return;

	if (data & 0xf8)
		logerror("Error: Mahjong panel read with mux data %02x\n", (data & 0xfe));
	else
		m_latch = m_panel[(data & 0xfe) >> 1]->read();
}
