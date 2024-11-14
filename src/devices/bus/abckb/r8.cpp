// copyright-holders:Curt Coder
/**********************************************************************

    Luxor R8 mouse emulation

*********************************************************************/

#include "emu.h"
#include "r8.h"



//**************************************************************************
//  DEVICE DEFINITIONS
//**************************************************************************

DEFINE_DEVICE_TYPE(LUXOR_R8, luxor_r8_device, "luxor_r8", "Luxor R8")


//-------------------------------------------------
//  INPUT_PORTS( r8 )
//-------------------------------------------------

static INPUT_PORTS_START( r8 )
	PORT_START("MOUSEB")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_NAME("Left Mouse Button") PORT_CODE(MOUSECODE_BUTTON1)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_NAME("Middle Mouse Button") PORT_CODE(MOUSECODE_BUTTON3)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_NAME("Right Mouse Button") PORT_CODE(MOUSECODE_BUTTON2)

	PORT_START("MOUSEX")
	PORT_BIT( 0xfff, 0x000, IPT_MOUSE_X ) PORT_SENSITIVITY(50)

	PORT_START("MOUSEY")
	PORT_BIT( 0xfff, 0x000, IPT_MOUSE_Y ) PORT_SENSITIVITY(50)
INPUT_PORTS_END


//-------------------------------------------------
//  input_ports - device-specific input ports
//-------------------------------------------------

ioport_constructor luxor_r8_device::device_input_ports() const
{
	return INPUT_PORTS_NAME( r8 );
}


//-------------------------------------------------
//  scan_mouse -
//-------------------------------------------------

TIMER_CALLBACK_MEMBER(luxor_r8_device::scan_mouse)
{
	switch(m_phase)
	{
	case 0:
		m_xa = m_x > m_prev_x ? CLEAR_LINE : ASSERT_LINE;
		m_xb = m_x < m_prev_x ? CLEAR_LINE : ASSERT_LINE;
		m_ya = m_y > m_prev_y ? CLEAR_LINE : ASSERT_LINE;
		m_yb = m_y < m_prev_y ? CLEAR_LINE : ASSERT_LINE;
		break;
	case 1:
		m_xa = m_xb = m_x != m_prev_x ? CLEAR_LINE : ASSERT_LINE;
		m_ya = m_yb = m_y != m_prev_y ? CLEAR_LINE : ASSERT_LINE;
		break;
	case 2:
		m_xa = m_x < m_prev_x ? CLEAR_LINE : ASSERT_LINE;
		m_xb = m_x > m_prev_x ? CLEAR_LINE : ASSERT_LINE;
		m_ya = m_y < m_prev_y ? CLEAR_LINE : ASSERT_LINE;
		m_yb = m_y > m_prev_y ? CLEAR_LINE : ASSERT_LINE;
		break;
	case 3:
		m_xa = m_xb = ASSERT_LINE;
		m_ya = m_yb = ASSERT_LINE;
		m_prev_x = m_x;
		m_prev_y = m_y;
		m_x = m_mouse_x->read();
		m_y = m_mouse_y->read();
		break;
	}

	m_phase = (m_phase + 1) & 3;
}



//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  luxor_r8_device - constructor
//-------------------------------------------------

luxor_r8_device::luxor_r8_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	device_t(mconfig, LUXOR_R8, tag, owner, clock),
	m_mouse_timer(nullptr),
	m_mouse_b(*this, "MOUSEB"),
	m_mouse_x(*this, "MOUSEX"),
	m_mouse_y(*this, "MOUSEY")
{
}


//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void luxor_r8_device::device_start()
{
	// allocate timers
	m_mouse_timer = timer_alloc(FUNC(luxor_r8_device::scan_mouse), this);
	m_mouse_timer->adjust(attotime::from_hz(1000), 0, attotime::from_hz(1000));

	// state saving
	save_item(NAME(m_phase));
	save_item(NAME(m_x));
	save_item(NAME(m_y));
	save_item(NAME(m_prev_x));
	save_item(NAME(m_prev_y));
	save_item(NAME(m_xa));
	save_item(NAME(m_xb));
	save_item(NAME(m_ya));
	save_item(NAME(m_yb));
}


//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void luxor_r8_device::device_reset()
{
	m_phase = 0;
	m_xa = m_xb = ASSERT_LINE;
	m_ya = m_yb = ASSERT_LINE;
	m_x = m_y = 0;
	m_prev_x = m_prev_y = 0;
}


//-------------------------------------------------
//  read -
//-------------------------------------------------

u8 luxor_r8_device::read()
{
	u8 data = 0;

	// mouse movement
	data |= m_xa;
	data |= m_xb << 1;
	data |= m_ya << 2;
	data |= m_yb << 3;

	// mouse buttons
	data |= (m_mouse_b->read() & 0x07) << 4;

	return data;
}
