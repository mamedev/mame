// license:BSD-3-Clause
// copyright-holders:Fabio Priuli
/**********************************************************************

    SNK Neo Geo Irritating Maze Trackball Controller emulation

**********************************************************************/

#include "emu.h"
#include "irrmaze.h"


//**************************************************************************
//  DEVICE DEFINITIONS
//**************************************************************************

DEFINE_DEVICE_TYPE(NEOGEO_IRRMAZE, neogeo_irrmaze_device, "neogeo_irrmaze", "SNK Neo Geo Irritating Maze Trackball controller")


static INPUT_PORTS_START( neogeo_irrmaze )
	PORT_START("BUTTONS")
	PORT_BIT( 0x0f, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(1)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(1)
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(2)
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(2)

	PORT_START("START")
	PORT_BIT( 0xfe, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_START1 )

	PORT_START("TRACK_X")
	PORT_BIT( 0xff, 0x00, IPT_TRACKBALL_X ) PORT_SENSITIVITY(10) PORT_KEYDELTA(20) PORT_REVERSE

	PORT_START("TRACK_Y")
	PORT_BIT( 0xff, 0x00, IPT_TRACKBALL_Y ) PORT_SENSITIVITY(10) PORT_KEYDELTA(20) PORT_REVERSE
INPUT_PORTS_END


//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  neogeo_irrmaze_device - constructor
//-------------------------------------------------

neogeo_irrmaze_device::neogeo_irrmaze_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	device_t(mconfig, NEOGEO_IRRMAZE, tag, owner, clock),
	device_neogeo_ctrl_edge_interface(mconfig, *this),
	m_tx(*this, "TRACK_X"),
	m_ty(*this, "TRACK_Y"),
	m_buttons(*this, "BUTTONS"),
	m_ss(*this, "START"),
	m_spi_outputs(*this, "sit%u", 0U),
	m_spi_sr(0U),
	m_ctrl_sel(0U)
{
}


//-------------------------------------------------
//  input_ports - device-specific input ports
//-------------------------------------------------

ioport_constructor neogeo_irrmaze_device::device_input_ports() const
{
	return INPUT_PORTS_NAME( neogeo_irrmaze );
}


//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void neogeo_irrmaze_device::device_start()
{
	m_spi_outputs.resolve();

	save_item(NAME(m_spi_sr));
	save_item(NAME(m_ctrl_sel));
}


//-------------------------------------------------
//  in0_r
//-------------------------------------------------

READ8_MEMBER(neogeo_irrmaze_device::in0_r)
{
	uint8_t res = 0;
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
//  read_start_sel
//-------------------------------------------------

uint8_t neogeo_irrmaze_device::read_start_sel()
{
	return m_ss->read();
}

//-------------------------------------------------
//  write_ctrlsel
//-------------------------------------------------

void neogeo_irrmaze_device::write_ctrlsel(uint8_t data)
{
	if (BIT(data & ~m_ctrl_sel, 3) && BIT(m_ctrl_sel, 1))
	{
		for (unsigned i = 0; 16U > i; ++i)
			m_spi_outputs[i] = BIT(m_spi_sr, i);
	}

	if (BIT(data & ~m_ctrl_sel, 2))
		m_spi_sr = (m_spi_sr >> 1) | (uint16_t(BIT(m_ctrl_sel, 1)) << 15);

	m_ctrl_sel = data;
}
