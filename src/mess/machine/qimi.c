// license:BSD-3-Clause
// copyright-holders:Curt Coder
/**********************************************************************

    QL Internal Mouse Interface emulation

    Copyright MESS Team.
    Visit http://mamedev.org for licensing and usage restrictions.

**********************************************************************/

#include "qimi.h"



//**************************************************************************
//  MACROS / CONSTANTS
//**************************************************************************

#define MOUSEX_TAG              "MOUSEX"
#define MOUSEY_TAG              "MOUSEY"
#define MOUSEB_TAG              "MOUSEB"

// Mouse bits in Sandy port order
#define MOUSE_MIDDLE            0x02
#define MOUSE_RIGHT             0x04
#define MOUSE_LEFT              0x08
#define MOUSE_DIRY              0x10
#define MOUSE_DIRX              0x20
#define MOUSE_INTY              0x40
#define MOUSE_INTX              0x80
#define MOUSE_INT_MASK          (MOUSE_INTX | MOUSE_INTY)

#define QIMI_INTX               0x04
#define QIMI_INTY               0x20
#define QIMI_DIRX               0x10
#define QIMI_DIRY               0x01
#define QIMI_LEFT               0x20
#define QIMI_RIGHT              0x10
#define QIMI_INT_MASK           (QIMI_INTX | QIMI_INTY)



//**************************************************************************
//  DEVICE DEFINITIONS
//**************************************************************************

const device_type QIMI = &device_creator<qimi_t>;


//-------------------------------------------------
//  INPUT_PORTS( qimi )
//-------------------------------------------------

INPUT_PORTS_START( qimi )
	PORT_START(MOUSEX_TAG)
	PORT_BIT( 0xff, 0x00, IPT_MOUSE_X ) PORT_SENSITIVITY(50) PORT_KEYDELTA(5) PORT_MINMAX(0, 255) PORT_PLAYER(1)

	PORT_START(MOUSEY_TAG)
	PORT_BIT( 0xff, 0x00, IPT_MOUSE_Y ) PORT_SENSITIVITY(50) PORT_KEYDELTA(5) PORT_MINMAX(0, 255) PORT_PLAYER(1)

	PORT_START(MOUSEB_TAG)  /* Mouse buttons */
	PORT_BIT( MOUSE_RIGHT, IP_ACTIVE_LOW, IPT_BUTTON1) PORT_NAME("Mouse Button 1") PORT_CODE(MOUSECODE_BUTTON1)
	PORT_BIT( MOUSE_LEFT,  IP_ACTIVE_LOW, IPT_BUTTON2) PORT_NAME("Mouse Button 2") PORT_CODE(MOUSECODE_BUTTON2)
	PORT_BIT( MOUSE_MIDDLE, IP_ACTIVE_LOW, IPT_BUTTON3) PORT_NAME("Mouse Button 3") PORT_CODE(MOUSECODE_BUTTON3)
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
	m_mousex(*this, MOUSEX_TAG),
	m_mousey(*this, MOUSEY_TAG),
	m_mouseb(*this, MOUSEB_TAG)
{
}


//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void qimi_t::device_start()
{
	// resolve callbacks
	m_write_extint.resolve_safe();

	// allocate timer
	m_mouse_timer = timer_alloc();
	m_mouse_timer->adjust(attotime::zero, 0, attotime::from_hz(500));
}


//-------------------------------------------------
//  device_timer - handler timer events
//-------------------------------------------------

void qimi_t::device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr)
{
	UINT8 x         = m_mousex->read();
	UINT8 y         = m_mousey->read();
	UINT8 do_int    = 0;

	//m_mouse_int = 0;

	// Set X interupt flag and direction if x has changed
	if (x > m_ql_mouse_x)
	{
		m_mouse_int |= MOUSE_INTX;
		m_mouse_int |= MOUSE_DIRX;
	}
	else if (x < m_ql_mouse_x)
	{
		m_mouse_int |= MOUSE_INTX;
		m_mouse_int &= ~MOUSE_DIRX;
	}

	// Set Y interupt flag and direction if y has changed
	if (y > m_ql_mouse_y)
	{
		m_mouse_int |= MOUSE_INTY;
		m_mouse_int &= ~MOUSE_DIRY;
	}
	else if (y < m_ql_mouse_y)
	{
		m_mouse_int |= MOUSE_INTY;
		m_mouse_int |= MOUSE_DIRY;
	}

	// Update saved location
	m_ql_mouse_x = x;
	m_ql_mouse_y = y;

	// if it is a QIMI, then always do int if triggered.
	// if this is a Sandy mouse, only trigger an int if it is enabled in the mask register
	do_int = 1;

	//logerror("m_mouse_int=%02X, MOUSE_INT_MASK=%02X, m_disk_io_byte=%02X, (m_disk_io_byte & SANDY_MOUSE_INTMASK)=%02x\n",m_mouse_int,MOUSE_INT_MASK,m_disk_io_byte,(m_disk_io_byte & SANDY_MOUSE_INTMASK));

	// if mouse moved trigger external int
	if((m_mouse_int & MOUSE_INT_MASK) && do_int)
	{
		m_write_extint(ASSERT_LINE);
	}
}


//-------------------------------------------------
//  read -
//-------------------------------------------------

READ8_MEMBER( qimi_t::read )
{
	UINT8 result = 0;
	UINT8 buttons;

	switch (offset)
	{
		// 0x1bf9c, button status
		case 0x00   :
			buttons = m_mouseb->read();
			result = ((buttons & MOUSE_RIGHT) << 2) | ((buttons & MOUSE_LEFT) << 2);
			break;

		// 0x1bfbc, direction status
		case 0x20   :
			result = ((m_mouse_int & MOUSE_INTX) >> 5) | ((m_mouse_int & MOUSE_INTY) >> 1) |
						((m_mouse_int & MOUSE_DIRX) >> 1) | ((m_mouse_int & MOUSE_DIRY) >> 4);
			break;
		case 0x22   :
			m_mouse_int &= ~MOUSE_INT_MASK;
			break;
	}

	return result;
}


//-------------------------------------------------
//  write -
//-------------------------------------------------

WRITE8_MEMBER( qimi_t::write )
{
	// write to 0x1bfbe resets int status
	if (offset == 0x22)
	{
		m_mouse_int = 0;
	}
}
