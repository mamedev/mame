// copyright-holders:Curt Coder
/**********************************************************************

    Luxor R8 mouse emulation

*********************************************************************/

#include "emu.h"
#include "r8.h"



//**************************************************************************
//  DEVICE DEFINITIONS
//**************************************************************************

DEFINE_DEVICE_TYPE(R8, r8_device, "r8", "Luxor R8")


//-------------------------------------------------
//  INPUT_PORTS( r8 )
//-------------------------------------------------

INPUT_PORTS_START( r8 )
	PORT_START("MOUSEB")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("Left Mouse Button") PORT_CODE(MOUSECODE_BUTTON1)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("Middle Mouse Button") PORT_CODE(MOUSECODE_BUTTON3)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("Right Mouse Button") PORT_CODE(MOUSECODE_BUTTON2)

	PORT_START("MOUSEX")
	PORT_BIT( 0xfff, 0x000, IPT_MOUSE_X ) PORT_SENSITIVITY(50) PORT_KEYDELTA(0)

	PORT_START("MOUSEY")
	PORT_BIT( 0xfff, 0x000, IPT_MOUSE_Y ) PORT_SENSITIVITY(50) PORT_KEYDELTA(0)
INPUT_PORTS_END


//-------------------------------------------------
//  input_ports - device-specific input ports
//-------------------------------------------------

ioport_constructor r8_device::device_input_ports() const
{
	return INPUT_PORTS_NAME( r8 );
}


//-------------------------------------------------
//  scan_mouse -
//-------------------------------------------------

TIMER_CALLBACK_MEMBER(r8_device::scan_mouse)
{
	switch(m_mouse.phase)
	{
	case 0:
		m_mouse.xa = m_mouse.x > m_mouse.prev_x ? CLEAR_LINE : ASSERT_LINE;
		m_mouse.xb = m_mouse.x < m_mouse.prev_x ? CLEAR_LINE : ASSERT_LINE;
		m_mouse.ya = m_mouse.y > m_mouse.prev_y ? CLEAR_LINE : ASSERT_LINE;
		m_mouse.yb = m_mouse.y < m_mouse.prev_y ? CLEAR_LINE : ASSERT_LINE;
		break;
	case 1:
		m_mouse.xa = m_mouse.xb = m_mouse.x != m_mouse.prev_x ? CLEAR_LINE : ASSERT_LINE;
		m_mouse.ya = m_mouse.yb = m_mouse.y != m_mouse.prev_y ? CLEAR_LINE : ASSERT_LINE;
		break;
	case 2:
		m_mouse.xa = m_mouse.x < m_mouse.prev_x ? CLEAR_LINE : ASSERT_LINE;
		m_mouse.xb = m_mouse.x > m_mouse.prev_x ? CLEAR_LINE : ASSERT_LINE;
		m_mouse.ya = m_mouse.y < m_mouse.prev_y ? CLEAR_LINE : ASSERT_LINE;
		m_mouse.yb = m_mouse.y > m_mouse.prev_y ? CLEAR_LINE : ASSERT_LINE;
		break;
	case 3:
		m_mouse.xa = m_mouse.xb = ASSERT_LINE;
		m_mouse.ya = m_mouse.yb = ASSERT_LINE;
		m_mouse.prev_x = m_mouse.x;
		m_mouse.prev_y = m_mouse.y;
		m_mouse.x = m_mouse_x->read();
		m_mouse.y = m_mouse_y->read();
		break;
	}

	m_mouse.phase = (m_mouse.phase + 1) & 3;
}



//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  r8_device - constructor
//-------------------------------------------------

r8_device::r8_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	device_t(mconfig, R8, tag, owner, clock),
	m_mouse_timer(nullptr),
	m_mouse_b(*this, "MOUSEB"),
	m_mouse_x(*this, "MOUSEX"),
	m_mouse_y(*this, "MOUSEY")
{
}


//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void r8_device::device_start()
{
	// allocate timers
	m_mouse_timer = timer_alloc(FUNC(r8_device::scan_mouse), this);
	m_mouse_timer->adjust(attotime::from_hz(1000), 0, attotime::from_hz(1000));

	// state saving
	save_item(NAME(m_mouse.phase));
	save_item(NAME(m_mouse.x));
	save_item(NAME(m_mouse.y));
	save_item(NAME(m_mouse.prev_x));
	save_item(NAME(m_mouse.prev_y));
	save_item(NAME(m_mouse.xa));
	save_item(NAME(m_mouse.xb));
	save_item(NAME(m_mouse.ya));
	save_item(NAME(m_mouse.yb));
}


//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void r8_device::device_reset()
{
	m_mouse.phase = 0;
	m_mouse.xa = m_mouse.xb = ASSERT_LINE;
	m_mouse.ya = m_mouse.yb = ASSERT_LINE;
	m_mouse.x = m_mouse.y = 0;
	m_mouse.prev_x = m_mouse.prev_y = 0;
}


//-------------------------------------------------
//  read -
//-------------------------------------------------

u8 r8_device::read()
{
	u8 data = 0;

	// mouse movement
	data |= m_mouse.xa;
	data |= m_mouse.xb << 1;
	data |= m_mouse.ya << 2;
	data |= m_mouse.yb << 3;

	// mouse buttons
	data |= (m_mouse_b->read() & 0x07) << 4;

	return data;
}
