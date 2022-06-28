// license:BSD-3-Clause
// copyright-holders:Curt Coder, Phill Harvey-Smith
/**********************************************************************

    QJump/Quanta QL Internal Mouse Interface emulation

**********************************************************************/

#include "emu.h"
#include "qimi.h"



//**************************************************************************
//  DEVICE DEFINITIONS
//**************************************************************************

DEFINE_DEVICE_TYPE(QIMI, qimi_device, "qimi", "QL Internal Mouse Interface")


//-------------------------------------------------
//  INPUT_CHANGED_MEMBER( mouse_x_changed )
//-------------------------------------------------

INPUT_CHANGED_MEMBER( qimi_device::mouse_x_changed )
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

INPUT_CHANGED_MEMBER( qimi_device::mouse_y_changed )
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
	PORT_BIT( 0xff, 0x00, IPT_MOUSE_X ) PORT_SENSITIVITY(50) PORT_KEYDELTA(5) PORT_MINMAX(0, 255) PORT_CHANGED_MEMBER(DEVICE_SELF, qimi_device, mouse_x_changed, 0)

	PORT_START("mouse_y")
	PORT_BIT( 0xff, 0x00, IPT_MOUSE_Y ) PORT_SENSITIVITY(50) PORT_KEYDELTA(5) PORT_MINMAX(0, 255) PORT_CHANGED_MEMBER(DEVICE_SELF, qimi_device, mouse_y_changed, 0)

	PORT_START("mouse_buttons")
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_NAME("Right Mouse Button") PORT_CODE(MOUSECODE_BUTTON2)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_NAME("Left Mouse Button") PORT_CODE(MOUSECODE_BUTTON1)
	PORT_BIT( 0xcf, IP_ACTIVE_LOW, IPT_UNUSED )
INPUT_PORTS_END


//-------------------------------------------------
//  input_ports - device-specific input ports
//-------------------------------------------------

ioport_constructor qimi_device::device_input_ports() const
{
	return INPUT_PORTS_NAME( qimi );
}



//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  qimi_device - constructor
//-------------------------------------------------

qimi_device::qimi_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	device_t(mconfig, QIMI, tag, owner, clock),
	m_write_extint(*this),
	m_buttons(*this, "mouse_buttons"),
	m_status(0),
	m_extint_en(false)
{
}


//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void qimi_device::device_start()
{
	// resolve callbacks
	m_write_extint.resolve_safe();
}


//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void qimi_device::device_reset()
{
	m_status = 0;
	m_extint_en = false;
}


//-------------------------------------------------
//  read -
//-------------------------------------------------

uint8_t qimi_device::read(offs_t offset, uint8_t data)
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

void qimi_device::write(offs_t offset, uint8_t data)
{
	(void)data;

	// write to 0x1bfbe resets int status
	if (offset == 0x1bfbe)
	{
		m_status &= ~(ST_Y_INT | ST_X_INT);
		m_extint_en = true;

		m_write_extint(CLEAR_LINE);
	}
}
