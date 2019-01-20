// license:BSD-3-Clause
// copyright-holders:Ryan Holtz

#include "emu.h"
#include "pad.h"


//**************************************************************************
//  GLOBAL VARIABLES
//**************************************************************************

DEFINE_DEVICE_TYPE(VSMILE_PAD, vsmile_pad_device, "vsmile_pad", "V.Smile Control Pad")


//**************************************************************************
//    V.Smile control pad
//**************************************************************************

vsmile_pad_device::vsmile_pad_device(machine_config const &mconfig, char const *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, VSMILE_PAD, tag, owner, clock)
	, device_vsmile_ctrl_interface(mconfig, *this)
	, m_io_joy(*this, "JOY")
	, m_io_colors(*this, "COLORS")
	, m_io_buttons(*this, "BUTTONS")
	, m_uart_tx_timer(nullptr)
	, m_pad_timer(nullptr)
{
}

vsmile_pad_device::~vsmile_pad_device()
{
}

void vsmile_pad_device::device_start()
{
	m_pad_timer = timer_alloc(TIMER_PAD);
	m_pad_timer->adjust(attotime::never);

	m_uart_tx_timer = timer_alloc(TIMER_UART_TX);
	m_uart_tx_timer->adjust(attotime::never);

	save_item(NAME(m_ctrl_cts));
	save_item(NAME(m_ctrl_probe_history));
	save_item(NAME(m_ctrl_probe_count));
	save_item(NAME(m_uart_tx_fifo));
	save_item(NAME(m_uart_tx_fifo_start));
	save_item(NAME(m_uart_tx_fifo_end));
	save_item(NAME(m_uart_tx_fifo_count));
}

void vsmile_pad_device::device_reset()
{
	m_pad_timer->adjust(attotime::from_hz(1), 0, attotime::from_hz(1));
	m_uart_tx_timer->adjust(attotime::from_hz(9600/10), 0, attotime::from_hz(9600/10));

	m_ctrl_cts = false;
	memset(m_ctrl_probe_history, 0, 2);
	m_ctrl_probe_count = 0;
	memset(m_uart_tx_fifo, 0, 32);
	m_uart_tx_fifo_start = 0;
	m_uart_tx_fifo_end = 0;
	m_uart_tx_fifo_count = 0;
	m_uart_tx_state = XMIT_STATE_IDLE;
}

void vsmile_pad_device::cts_w(int state)
{
	//printf("%s CTS: %d\n", tag(), state);
	m_ctrl_cts = state;
}

void vsmile_pad_device::data_w(uint8_t data)
{
	//printf("%s Receiving: %02x\n", tag(), data);
	if ((data >> 4) == 7 || (data >> 4) == 11)
	{
		m_ctrl_probe_history[0] = m_ctrl_probe_history[1];
		m_ctrl_probe_history[1] = data;
		const uint8_t response = ((m_ctrl_probe_history[0] + m_ctrl_probe_history[1] + 0x0f) & 0x0f) ^ 0x05;
		uart_tx_fifo_push(0xb0 | response);
	}
}

void vsmile_pad_device::uart_tx_fifo_push(uint8_t data)
{
	if (m_uart_tx_fifo_count == ARRAY_LENGTH(m_uart_tx_fifo))
	{
		logerror("Warning: Trying to push more than %d bytes onto the controller Tx FIFO, data will be lost\n", ARRAY_LENGTH(m_uart_tx_fifo));
	}

	//printf("%s Pushing: %02x\n", tag(), data);
	m_uart_tx_fifo[m_uart_tx_fifo_end] = data;
	m_uart_tx_fifo_count++;
	m_uart_tx_fifo_end = (m_uart_tx_fifo_end + 1) % ARRAY_LENGTH(m_uart_tx_fifo);
}

void vsmile_pad_device::handle_uart_tx()
{
	if (m_uart_tx_fifo_count == 0)
		return;

	if (m_uart_tx_state == XMIT_STATE_IDLE)
	{
		//printf("%s RTS: 1\n", tag());
		m_uart_tx_state = XMIT_STATE_RTS;
		rts_out(1);
		return;
	}
	else if (m_uart_tx_state == XMIT_STATE_RTS)
	{
		// HACK: This should work. It doesn't.
		//if (m_ctrl_cts)
		//	m_uart_tx_state = XMIT_STATE_CTS;
		//return;
	}

	//printf("%s Transmitting: %02x\n", tag(), m_uart_tx_fifo[m_uart_tx_fifo_start]);
	data_out(m_uart_tx_fifo[m_uart_tx_fifo_start]);
	m_uart_tx_fifo_start = (m_uart_tx_fifo_start + 1) % ARRAY_LENGTH(m_uart_tx_fifo);
	m_uart_tx_fifo_count--;
	if (m_uart_tx_fifo_count == 0)
	{
		m_uart_tx_state = XMIT_STATE_IDLE;
		//printf("%s RTS: 0\n", tag());
		rts_out(0);
	}
}

void vsmile_pad_device::device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr)
{
	switch (id)
	{
	case TIMER_UART_TX:
		handle_uart_tx();
		break;
	case TIMER_PAD:
		uart_tx_fifo_push(0x55);
		break;
	default:
		logerror("Unknown timer ID: %d\n", id);
		break;
	}
}

INPUT_CHANGED_MEMBER(vsmile_pad_device::pad_joy_changed)
{
	const uint8_t value = m_io_joy->read();

	if (BIT(value, 2))
		uart_tx_fifo_push(0xcf);
	else if (BIT(value, 3))
		uart_tx_fifo_push(0xc7);
	else
		uart_tx_fifo_push(0xc0);

	if (BIT(value, 0))
		uart_tx_fifo_push(0x87);
	else if (BIT(value, 1))
		uart_tx_fifo_push(0x8f);
	else
		uart_tx_fifo_push(0x80);
}

INPUT_CHANGED_MEMBER(vsmile_pad_device::pad_color_changed)
{
	uart_tx_fifo_push(0x90 | m_io_colors->read());
}

INPUT_CHANGED_MEMBER(vsmile_pad_device::pad_button_changed)
{
	const uint8_t value = m_io_buttons->read();
	const size_t bit = reinterpret_cast<size_t>(param);
	if (BIT(value, bit))
	{
		uart_tx_fifo_push(0xa1 + (uint8_t)bit);
	}
	else
	{
		uart_tx_fifo_push(0xa0);
	}
}

static INPUT_PORTS_START( vsmile_pad )
	PORT_START("JOY")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_JOYSTICK_UP )    PORT_CHANGED_MEMBER(DEVICE_SELF, vsmile_pad_device, pad_joy_changed, 0) PORT_NAME("Joypad Up")
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_JOYSTICK_DOWN )  PORT_CHANGED_MEMBER(DEVICE_SELF, vsmile_pad_device, pad_joy_changed, 0) PORT_NAME("Joypad Down")
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT )  PORT_CHANGED_MEMBER(DEVICE_SELF, vsmile_pad_device, pad_joy_changed, 0) PORT_NAME("Joypad Left")
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT ) PORT_CHANGED_MEMBER(DEVICE_SELF, vsmile_pad_device, pad_joy_changed, 0) PORT_NAME("Joypad Right")
	PORT_BIT( 0xf0, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("COLORS")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_CHANGED_MEMBER(DEVICE_SELF, vsmile_pad_device, pad_color_changed, 0) PORT_NAME("Green")
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_BUTTON2 ) PORT_CHANGED_MEMBER(DEVICE_SELF, vsmile_pad_device, pad_color_changed, 0) PORT_NAME("Blue")
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_BUTTON3 ) PORT_CHANGED_MEMBER(DEVICE_SELF, vsmile_pad_device, pad_color_changed, 0) PORT_NAME("Yellow")
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_BUTTON4 ) PORT_CHANGED_MEMBER(DEVICE_SELF, vsmile_pad_device, pad_color_changed, 0) PORT_NAME("Red")
	PORT_BIT( 0xf0, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("BUTTONS")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_BUTTON5 ) PORT_CHANGED_MEMBER(DEVICE_SELF, vsmile_pad_device, pad_button_changed, 0) PORT_NAME("OK")
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_BUTTON6 ) PORT_CHANGED_MEMBER(DEVICE_SELF, vsmile_pad_device, pad_button_changed, 1) PORT_NAME("Quit")
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_BUTTON7 ) PORT_CHANGED_MEMBER(DEVICE_SELF, vsmile_pad_device, pad_button_changed, 2) PORT_NAME("Help")
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_BUTTON8 ) PORT_CHANGED_MEMBER(DEVICE_SELF, vsmile_pad_device, pad_button_changed, 3) PORT_NAME("ABC")
	PORT_BIT( 0xf0, IP_ACTIVE_HIGH, IPT_UNUSED )
INPUT_PORTS_END

ioport_constructor vsmile_pad_device::device_input_ports() const
{
	return INPUT_PORTS_NAME( vsmile_pad );
}
