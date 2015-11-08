// license:BSD-3-Clause
// copyright-holders:Curt Coder, Phill Harvey-Smith
/**********************************************************************

    QJump/Quanta QL Internal Mouse Interface emulation

**********************************************************************/

#include "qimi.h"



//**************************************************************************
//  DEVICE DEFINITIONS
//**************************************************************************

const device_type QIMI = &device_creator<qimi_t>;


//-------------------------------------------------
//  INPUT_CHANGED_MEMBER( mouse_x_changed )
//-------------------------------------------------

INPUT_CHANGED_MEMBER( qimi_t::mouse_x_changed )
{
	if (newval > oldval)
	{
		m_status |= ST_X_DIR;
	}
	else
	{
		m_status &= ~ST_X_DIR;
	}

	m_status |= ST_X_INT;

	if (m_extint_en)
	{
		m_write_extint(ASSERT_LINE);
	}
}


//-------------------------------------------------
//  INPUT_CHANGED_MEMBER( mouse_y_changed )
//-------------------------------------------------

INPUT_CHANGED_MEMBER( qimi_t::mouse_y_changed )
{
	if (newval < oldval)
	{
		m_status |= ST_Y_DIR;
	}
	else
	{
		m_status &= ~ST_Y_DIR;
	}

	m_status |= ST_Y_INT;

	if (m_extint_en)
	{
		m_write_extint(ASSERT_LINE);
	}
}


//-------------------------------------------------
//  INPUT_PORTS( qimi )
//-------------------------------------------------

INPUT_PORTS_START( qimi )
	PORT_START("mouse_x")
	PORT_BIT( 0xff, 0x00, IPT_MOUSE_X ) PORT_SENSITIVITY(50) PORT_KEYDELTA(5) PORT_MINMAX(0, 255) PORT_CHANGED_MEMBER(DEVICE_SELF, qimi_t, mouse_x_changed, 0)

	PORT_START("mouse_y")
	PORT_BIT( 0xff, 0x00, IPT_MOUSE_Y ) PORT_SENSITIVITY(50) PORT_KEYDELTA(5) PORT_MINMAX(0, 255) PORT_CHANGED_MEMBER(DEVICE_SELF, qimi_t, mouse_y_changed, 0)

	PORT_START("mouse_buttons")
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_NAME("Right Mouse Button") PORT_CODE(MOUSECODE_BUTTON2)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_NAME("Left Mouse Button") PORT_CODE(MOUSECODE_BUTTON1)
	PORT_BIT( 0xcf, IP_ACTIVE_LOW, IPT_UNUSED )
INPUT_PORTS_END


//-------------------------------------------------
//  input_ports - device-specific input ports
//-------------------------------------------------

ioport_constructor qimi_t::device_input_ports() const
{
	return INPUT_PORTS_NAME( qimi );
}



//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  qimi_t - constructor
//-------------------------------------------------

qimi_t::qimi_t(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock) :
	device_t(mconfig, QIMI, "QL Internal Mouse Interface", tag, owner, clock, "qimi", __FILE__),
	m_write_extint(*this),
	m_buttons(*this, "mouse_buttons"),
	m_status(0),
	m_extint_en(false)
{
}


//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void qimi_t::device_start()
{
	// resolve callbacks
	m_write_extint.resolve_safe();
}


//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void qimi_t::device_reset()
{
	m_status = 0;
	m_extint_en = false;
}


//-------------------------------------------------
//  read -
//-------------------------------------------------

UINT8 qimi_t::read(address_space &space, offs_t offset, UINT8 data)
{
	switch (offset)
	{
	// button status
	case 0x1bf9c:
		data = m_buttons->read();
		break;

	// direction status
	case 0x1bfbc:
		data = m_status;
		break;

	case 0x1bfbe:
		m_status &= ~(ST_Y_INT | ST_X_INT);
		m_extint_en = true;

		m_write_extint(CLEAR_LINE);
		break;
	}

	return data;
}


//-------------------------------------------------
//  write -
//-------------------------------------------------

WRITE8_MEMBER( qimi_t::write )
{
	// write to 0x1bfbe resets int status
	if (offset == 0x1bfbe)
	{
		m_status &= ~(ST_Y_INT | ST_X_INT);
		m_extint_en = true;

		m_write_extint(CLEAR_LINE);
	}
}
