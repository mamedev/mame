// license:BSD-3-Clause
// copyright-holders:smf
/***************************************************************************

    machine/ser_mouse.c

    Code for emulating PC-style serial mouses

***************************************************************************/

#include "ser_mouse.h"


serial_mouse_device::serial_mouse_device(const machine_config &mconfig, device_type type, const char *name, const char *tag, device_t *owner, UINT32 clock, const char *shortname, const char *source)
	: device_t(mconfig, type, name, tag, owner, clock, shortname, source),
	device_rs232_port_interface(mconfig, *this),
	device_serial_interface(mconfig, *this),
	m_dtr(1),
	m_rts(1),
	m_x(*this, "ser_mouse_x"),
	m_y(*this, "ser_mouse_y"),
	m_btn(*this, "ser_mouse_btn")
{
}

const device_type MSFT_SERIAL_MOUSE = &device_creator<microsoft_mouse_device>;

microsoft_mouse_device::microsoft_mouse_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
	: serial_mouse_device(mconfig, MSFT_SERIAL_MOUSE, "Microsoft Serial Mouse", tag, owner, clock, "microsoft_mouse", __FILE__)
{
}

const device_type MSYSTEM_SERIAL_MOUSE = &device_creator<mouse_systems_mouse_device>;

mouse_systems_mouse_device::mouse_systems_mouse_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
	: serial_mouse_device(mconfig, MSYSTEM_SERIAL_MOUSE, "Mouse Systems Serial Mouse", tag, owner, clock, "mouse_systems_mouse", __FILE__)
{
}

void serial_mouse_device::device_start()
{
	m_timer = timer_alloc();
	m_enabled = false;
	set_frame();
	set_tra_rate(1200);
	reset_mouse();

	save_item(NAME(m_dtr));
	save_item(NAME(m_rts));
	save_item(NAME(m_queue));
	save_item(NAME(m_head));
	save_item(NAME(m_tail));
	save_item(NAME(m_mb));
	save_item(NAME(m_enabled));
}

void serial_mouse_device::reset_mouse()
{
	m_head = m_tail = 0;
	output_rxd(1);
	output_dcd(0);
	output_dsr(0);
	output_ri(0);
	output_cts(0);
}

void serial_mouse_device::tra_complete()
{
	if(m_tail != m_head)
		transmit_register_setup(unqueue_data());
}

void serial_mouse_device::tra_callback()
{
	output_rxd(transmit_register_get_data_bit());
}

/**************************************************************************
 *  Check for mouse moves and buttons. Build delta x/y packets
 **************************************************************************/
void serial_mouse_device::device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr)
{
	if (id)
	{
		device_serial_interface::device_timer(timer, id, param, ptr);
		return;
	}

	static int ox = 0, oy = 0;
	int nx,ny;
	int dx, dy, nb;
	int mbc;

	/* Do not get deltas or send packets if queue is not empty (Prevents drifting) */
	if (m_head==m_tail)
	{
		nx = m_x->read();

		dx = nx - ox;
		if (dx<=-0x800) dx = nx + 0x1000 - ox; /* Prevent jumping */
		if (dx>=0x800) dx = nx - 0x1000 - ox;
		ox = nx;

		ny = m_y->read();

		dy = ny - oy;
		if (dy<=-0x800) dy = ny + 0x1000 - oy;
		if (dy>=0x800) dy = ny - 0x1000 - oy;
		oy = ny;

		nb = m_btn->read();
		mbc = nb^m_mb;
		m_mb = nb;

		/* check if there is any delta or mouse buttons changed */
		if ( (dx!=0) || (dy!=0) || (mbc!=0) )
			mouse_trans(dx, dy, nb, mbc);
	}


	if(m_tail != m_head && is_transmit_register_empty())
		transmit_register_setup(unqueue_data());
}

void microsoft_mouse_device::mouse_trans(int dx, int dy, int nb, int mbc)
{
	/* split deltas into packets of -128..+127 max */
	do
	{
		UINT8 m0, m1, m2;
		int ddx = (dx < -128) ? -128 : (dx > 127) ? 127 : dx;
		int ddy = (dy < -128) ? -128 : (dy > 127) ? 127 : dy;
		m0 = 0x40 | ((nb << 4) & 0x30) | ((ddx >> 6) & 0x03) | ((ddy >> 4) & 0x0c);
		m1 = ddx & 0x3f;
		m2 = ddy & 0x3f;

		/* KT - changed to use a function */
		queue_data(m0 | 0x40);
		queue_data(m1 & 0x03f);
		queue_data(m2 & 0x03f);
		if ((mbc & 0x04) != 0)  /* If button 3 changed send extra byte */
			queue_data( (nb & 0x04) << 3);

		dx -= ddx;
		dy -= ddy;
	} while( dx || dy );
}

/* mouse systems mouse
   from "PC Mouse information" by Tomi Engdahl */

/*
   The data is sent in 5 byte packets in following format:
   D7      D6      D5      D4      D3      D2      D1      D0

   1.      1       0       0       0       0       LB      CB      RB
   2.      X7      X6      X5      X4      X3      X2      X1      X0
   3.      Y7      Y6      Y5      Y4      Y3      Y4      Y1      Y0
   4.      X7'     X6'     X5'     X4'     X3'     X2'     X1'     X0'
   5.      Y7'     Y6'     Y5'     Y4'     Y3'     Y4'     Y1'     Y0'

   LB is left button state (0=pressed, 1=released)
   CB is center button state (0=pressed, 1=released)
   RB is right button state (0=pressed, 1=released)
   X7-X0 movement in X direction since last packet in signed byte
   format (-128..+127), positive direction right
   Y7-Y0 movement in Y direction since last packet in signed byte
   format (-128..+127), positive direction up
   X7'-X0' movement in X direction since sending of X7-X0 packet in signed byte
   format (-128..+127), positive direction right
   Y7'-Y0' movement in Y direction since sending of Y7-Y0 in signed byte
   format (-128..+127), positive direction up

   The last two bytes in the packet (bytes 4 and 5) contains information
   about movement data changes which have occurred after data bytes 2 and 3 have been sent. */

void mouse_systems_mouse_device::mouse_trans(int dx, int dy, int nb, int mbc)
{
	dy =-dy;

	do
	{
		int ddx = (dx < -128) ? -128 : (dx > 127) ? 127 : dx;
		int ddy = (dy < -128) ? -128 : (dy > 127) ? 127 : dy;

		/* KT - changed to use a function */
		queue_data(0x080 | ((((nb & 0x04) >> 1) + ((nb & 0x02) << 1) + (nb & 0x01)) ^ 0x07));
		queue_data(ddx);
		queue_data(ddy);
		/* for now... */
		queue_data(0);
		queue_data(0);
		dx -= ddx;
		dy -= ddy;
	} while( dx || dy );
}

/**************************************************************************
 *  Check for mouse control line changes and (de-)install timer
 **************************************************************************/

void serial_mouse_device::set_mouse_enable(bool state)
{
	if(state && !m_enabled)
	{
		m_timer->adjust(attotime::zero, 0, attotime::from_hz(240));
	}
	else if(!state && m_enabled)
	{
		m_timer->adjust(attotime::never);
		m_head = m_tail = 0;
	}
	m_enabled = state;

}


WRITE_LINE_MEMBER(serial_mouse_device::input_dtr)
{
	m_dtr = state;
	check_state();
}

WRITE_LINE_MEMBER(serial_mouse_device::input_rts)
{
	m_rts = state;
	check_state();
}

WRITE_LINE_MEMBER(microsoft_mouse_device::input_rts)
{
	if (!m_dtr && m_rts && !state)
	{
		reset_mouse();
		/* Identify as Microsoft 3 Button Mouse */
		queue_data('M');
		queue_data('3');
	}

	serial_mouse_device::input_rts(state);
}



/**************************************************************************
 *  Mouse INPUT_PORT declarations
 **************************************************************************/

static INPUT_PORTS_START( ser_mouse )
	PORT_START( "ser_mouse_btn" )
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_BUTTON1) PORT_NAME("Mouse Left Button") PORT_CODE(MOUSECODE_BUTTON1)
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_BUTTON2) PORT_NAME("Mouse Middle Button") PORT_CODE(MOUSECODE_BUTTON3)
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_BUTTON3) PORT_NAME("Mouse Right Button") PORT_CODE(MOUSECODE_BUTTON2)

	PORT_START( "ser_mouse_x" ) /* Mouse - X AXIS */
	PORT_BIT( 0xfff, 0x00, IPT_MOUSE_X) PORT_SENSITIVITY(100) PORT_KEYDELTA(0) PORT_PLAYER(1)

	PORT_START( "ser_mouse_y" ) /* Mouse - Y AXIS */
	PORT_BIT( 0xfff, 0x00, IPT_MOUSE_Y) PORT_SENSITIVITY(100) PORT_KEYDELTA(0) PORT_PLAYER(1)
INPUT_PORTS_END

ioport_constructor serial_mouse_device::device_input_ports() const
{
	return INPUT_PORTS_NAME(ser_mouse);
}
