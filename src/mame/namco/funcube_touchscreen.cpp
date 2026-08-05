// license:BSD-3-Clause
// copyright-holders:Luca Elia,Olivier Galibert
/***************************************************************************

    Namco Touchscreen device for Funcube series

***************************************************************************/

#include "emu.h"
#include "funcube_touchscreen.h"

#define VERBOSE ( 0 )
#include "logmacro.h"


static INPUT_PORTS_START( funcube_touchscreen )
	PORT_START("TOUCH_BTN")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_NAME( "Touch Screen" )

	PORT_START("TOUCH_X")
	PORT_BIT( 0xff, 0x00, IPT_LIGHTGUN_X ) PORT_MINMAX(0,0x5c+1) PORT_CROSSHAIR(X, -(1.0 * 0x05d/0x5c), -1.0/0x5c, 0) PORT_SENSITIVITY(45) PORT_KEYDELTA(5) PORT_REVERSE

	PORT_START("TOUCH_Y")
	PORT_BIT( 0xff, 0x00, IPT_LIGHTGUN_Y ) PORT_MINMAX(0,0x46+1) PORT_CROSSHAIR(Y, -(0xf0-8.0)/0xf0*0x047/0x46, -1.0/0x46, 0) PORT_SENSITIVITY(45) PORT_KEYDELTA(5) PORT_REVERSE
INPUT_PORTS_END


DEFINE_DEVICE_TYPE(FUNCUBE_TOUCHSCREEN, funcube_touchscreen_device, "funcube_touchscreen", "Funcube Touchscreen")

funcube_touchscreen_device::funcube_touchscreen_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	device_t(mconfig, FUNCUBE_TOUCHSCREEN, tag, owner, clock),
	device_serial_interface(mconfig, *this),
	m_tx_cb(*this),
	m_x(*this, "TOUCH_X"),
	m_y(*this, "TOUCH_Y"),
	m_btn(*this, "TOUCH_BTN"),
	m_button_state(0),
	m_serial_pos(0),
	m_serial{0}
{
}

ioport_constructor funcube_touchscreen_device::device_input_ports() const
{
	return INPUT_PORTS_NAME(funcube_touchscreen);
}

void funcube_touchscreen_device::device_start()
{
	set_data_frame(1, 8, PARITY_NONE, STOP_BITS_1);
	set_tra_rate(9600);
	m_button_state = 0x00;
	emu_timer *tm = timer_alloc(FUNC(funcube_touchscreen_device::read_buttons), this);
	tm->adjust(attotime::from_ticks(1, clock()), 0, attotime::from_ticks(1, clock()));

	save_item(NAME(m_button_state));
	save_item(NAME(m_serial_pos));
	save_item(NAME(m_serial));
}

void funcube_touchscreen_device::device_reset()
{
	m_serial_pos = 0;
	memset(m_serial, 0, sizeof(m_serial));
	m_tx_cb(1);
}

TIMER_CALLBACK_MEMBER(funcube_touchscreen_device::read_buttons)
{
	const uint8_t button_state = m_btn->read();
	if (m_button_state != button_state)
	{
		m_button_state = button_state;
		m_serial[0] = button_state ? 0xfe : 0xfd;
		m_serial[1] = m_x->read();
		m_serial[2] = m_y->read();
		m_serial[3] = 0xff;
		m_serial_pos = 0;
		transmit_register_setup(m_serial[m_serial_pos++]);
	}
}

void funcube_touchscreen_device::tra_complete()
{
	if (m_serial_pos != 4)
		transmit_register_setup(m_serial[m_serial_pos++]);
}

void funcube_touchscreen_device::tra_callback()
{
	m_tx_cb(transmit_register_get_data_bit());
}
