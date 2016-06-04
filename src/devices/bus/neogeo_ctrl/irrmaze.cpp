// license:BSD-3-Clause
// copyright-holders:Fabio Priuli
/**********************************************************************

    SNK Neo Geo Irritation Maze Analog Controller emulation

**********************************************************************/

#include "irrmaze.h"

//**************************************************************************
//  DEVICE DEFINITIONS
//**************************************************************************

const device_type NEOGEO_IRRMAZE = &device_creator<neogeo_irrmaze_device>;


static INPUT_PORTS_START( neogeo_irrmaze )
	PORT_START("BUTTONS")
	PORT_BIT( 0x0f, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(1)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(1)
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(2)
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(2)

	PORT_START("TRACK_X")
	PORT_BIT( 0xff, 0x00, IPT_TRACKBALL_X ) PORT_SENSITIVITY(10) PORT_KEYDELTA(20) PORT_REVERSE

	PORT_START("TRACK_Y")
	PORT_BIT( 0xff, 0x00, IPT_TRACKBALL_Y ) PORT_SENSITIVITY(10) PORT_KEYDELTA(20) PORT_REVERSE
INPUT_PORTS_END


//-------------------------------------------------
//  input_ports - device-specific input ports
//-------------------------------------------------

ioport_constructor neogeo_irrmaze_device::device_input_ports() const
{
	return INPUT_PORTS_NAME( neogeo_irrmaze );
}


//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  neogeo_irrmaze_device - constructor
//-------------------------------------------------

neogeo_irrmaze_device::neogeo_irrmaze_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock) :
					device_t(mconfig, NEOGEO_IRRMAZE, "SNK Neo Geo Irritating Maze Analog Controller", tag, owner, clock, "neogeo_irrmaze", __FILE__),
					device_neogeo_ctrl_edge_interface(mconfig, *this),
					m_tx(*this, "TRACK_X"),
					m_ty(*this, "TRACK_Y"),
					m_buttons(*this, "BUTTONS")
{
}


//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void neogeo_irrmaze_device::device_start()
{
	save_item(NAME(m_ctrl_sel));
}


//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void neogeo_irrmaze_device::device_reset()
{
	m_ctrl_sel = 0;
}


//-------------------------------------------------
//  in0_r
//-------------------------------------------------

READ8_MEMBER(neogeo_irrmaze_device::in0_r)
{
	UINT8 res = 0;
	if (m_ctrl_sel & 0x01)
		res = m_ty->read();
	else
		res = m_tx->read();

	return res;
}

//-------------------------------------------------
//  in1_r
//-------------------------------------------------

READ8_MEMBER(neogeo_irrmaze_device::in1_r)
{
	return m_buttons->read();
}

//-------------------------------------------------
//  write_ctrlsel
//-------------------------------------------------

void neogeo_irrmaze_device::write_ctrlsel(UINT8 data)
{
	m_ctrl_sel = data;
}
