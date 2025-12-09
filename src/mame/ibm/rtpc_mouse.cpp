// license:BSD-3-Clause
// copyright-holders:Patrick Mackinlay

/*
 * IBM RT PC Mouse (#8426,00F2384) also known as a 6100 pointing device.
 *
 * TODO:
 *  - resolution and exponential scaling
 */

#include "emu.h"
#include "rtpc_mouse.h"

#include <algorithm>

//#define VERBOSE (LOG_GENERAL)
#include "logmacro.h"

enum mouse_command : u8
{
	CMD_RESET          = 0x01,
	CMD_READ_CONFIG    = 0x06,
	CMD_ENABLE         = 0x08,
	CMD_DISABLE        = 0x09,
	CMD_READ_DATA      = 0x0b,
	CMD_SET_WRAP       = 0x0e,
	CMD_CLR_WRAP       = 0x0f,
	CMD_SET_LINEAR     = 0x6c,
	CMD_QUERY_STATUS   = 0x73,
	CMD_SET_SCALED     = 0x78,
	CMD_SET_RESOLUTION = 0x89, // 2-byte command
	CMD_SET_RATE       = 0x8a, // 2-byte command
	CMD_SET_DATA_MODE  = 0x8d, // 2-byte command
};

DEFINE_DEVICE_TYPE(RTPC_MOUSE, rtpc_mouse_device, "rtpc_mouse", "IBM RT PC Mouse")

rtpc_mouse_device::rtpc_mouse_device(machine_config const &mconfig, char const *tag, device_t *owner, u32 clock)
	: buffered_rs232_device<4>(mconfig, RTPC_MOUSE, tag, owner, clock)
	, m_buttons(*this, "BTN")
	, m_x_axis(*this, "X")
	, m_y_axis(*this, "Y")
{
}

void rtpc_mouse_device::device_start()
{
	buffered_rs232_device<4>::device_start();

	save_item(NAME(m_linear));
	save_item(NAME(m_disable));
	save_item(NAME(m_remote));
	save_item(NAME(m_wrap));
	save_item(NAME(m_resolution));
	save_item(NAME(m_sample_rate));

	save_item(NAME(m_b));
	save_item(NAME(m_x));
	save_item(NAME(m_y));

	set_data_frame(1, 8, PARITY_ODD, STOP_BITS_1);
	set_rate(9600);

	transmit_register_reset();

	m_timer = timer_alloc(FUNC(rtpc_mouse_device::data_report), this);
}

void rtpc_mouse_device::device_reset()
{
	buffered_rs232_device<4>::device_reset();

	m_linear = true;
	m_disable = true;
	m_remote = false;
	m_wrap = false;

	m_resolution = 1;
	m_sample_rate = 100;

	m_b = 0;
	m_x = 0;
	m_y = 0;

	m_cmd_param = nullptr;

	// power on report
	pon_report(false);

	m_timer->reset();
}

void rtpc_mouse_device::received_byte(u8 data)
{
	if (m_wrap)
	{
		if (data != CMD_RESET && data != CMD_CLR_WRAP)
		{
			transmit_byte(data);
			return;
		}
	}

	if (m_cmd_param)
	{
		(this->*m_cmd_param)(data);

		return;
	}

	switch (data)
	{
	case CMD_RESET:
		LOG("reset\n");
		reset();
		break;
	case CMD_READ_CONFIG:
		LOG("read configuration\n");
		transmit_byte(0x20);
		break;
	case CMD_ENABLE:
		LOG("enable\n");
		m_disable = false;
		if (!m_remote)
			m_timer->adjust(attotime::from_hz(m_sample_rate), 0, attotime::from_hz(m_sample_rate));
		break;
	case CMD_DISABLE:
		LOG("disable\n");
		m_disable = true;
		m_timer->reset();
		break;
	case CMD_READ_DATA:
		LOG("read data\n");
		data_report(1);
		break;
	case CMD_SET_WRAP:
		LOG("set wrap mode\n");
		m_wrap = true;
		break;
	case CMD_CLR_WRAP:
		LOG("reset wrap mode\n");
		m_wrap = false;
		break;
	case CMD_SET_LINEAR:
		LOG("set linear scaling\n");
		m_linear = true;
		break;
	case CMD_QUERY_STATUS:
		LOG("query status\n");
		status_report();
		break;
	case CMD_SET_SCALED:
		LOG("set exponential scaling\n");
		m_linear = false;
		break;
	case CMD_SET_RESOLUTION:
		m_cmd_param = &rtpc_mouse_device::set_resolution;
		break;
	case CMD_SET_RATE:
		m_cmd_param = &rtpc_mouse_device::set_sample_rate;
		break;
	case CMD_SET_DATA_MODE:
		m_cmd_param = &rtpc_mouse_device::set_data_mode;
		break;
	default:
		LOG("unknown command 0x%02x\n", data);
		pon_report(true);
		break;
	}
}

void rtpc_mouse_device::set_sample_rate(u8 data)
{
	LOG("set sample rate 0x%02x\n", data);

	switch (data)
	{
	case 0x0a: case 0x14: case 0x28:
	case 0x3c: case 0x50: case 0x64:
		m_sample_rate = data;
		if (!m_disable && !m_remote)
			m_timer->adjust(attotime::from_hz(m_sample_rate), 0, attotime::from_hz(m_sample_rate));
		break;
	default:
		pon_report(true);
		break;
	}

	m_cmd_param = nullptr;
}

void rtpc_mouse_device::set_data_mode(u8 data)
{
	switch (data)
	{
	case 0x00:
		LOG("set stream mode\n");
		m_remote = false;
		if (!m_disable)
			m_timer->adjust(attotime::from_hz(m_sample_rate), 0, attotime::from_hz(m_sample_rate));
		break;
	case 0x03:
		LOG("set remote mode\n");
		m_remote = true;
		m_timer->reset();
		break;
	default:
		LOG("set_data_mode unknown 0x%02x\n", data);
		pon_report(true);
		break;
	}

	m_cmd_param = nullptr;
}

void rtpc_mouse_device::set_resolution(u8 data)
{
	LOG("set resolution 0x%02x\n", data);

	switch (data)
	{
	case 0x00: // 200/inch
	case 0x01: // 100/inch
	case 0x02: //  50/inch
	case 0x03: //  25/inch
		m_resolution = data;
		break;
	default:
		pon_report(true);
		break;
	}

	m_cmd_param = nullptr;
}

void rtpc_mouse_device::status_report()
{
	u8 status = 0xe1;

	if (m_linear)
		status |= 0x08;
	if (m_disable)
		status |= 0x04;
	if (m_remote)
		status |= 0x02;

	transmit_byte(status);
	transmit_byte(m_buttons->read());
	transmit_byte(m_resolution);
	transmit_byte(m_sample_rate);
}

static s16 read_axis(ioport_port &port, u16 &old_val)
{
	u16 const new_val = port.read();
	s16 const delta = new_val - old_val;

	old_val = new_val;

	return delta;
}

void rtpc_mouse_device::data_report(s32 param)
{
	if (fifo_empty())
	{
		u8 const btn = m_buttons->read();
		s16 const dx = read_axis(*m_x_axis, m_x);
		s16 const dy = -read_axis(*m_y_axis, m_y);

		if (dx || dy || m_b != btn || param)
		{
			u8 status = btn;

			if (dx < 0)
				status |= 0x04;
			if (dy < 0)
				status |= 0x02;

			transmit_byte(0x0b);
			transmit_byte(status);
			transmit_byte(s8(std::clamp<s16>(dx, -127, 127)));
			transmit_byte(s8(std::clamp<s16>(dy, -127, 127)));

			m_b = btn;
		}
	}
}

void rtpc_mouse_device::pon_report(bool error)
{
	transmit_byte(0xff);
	if (error)
		transmit_byte(0x0a);
	else
		transmit_byte(0x08);
	transmit_byte(0x00);
	transmit_byte(0x00);
}

INPUT_PORTS_START(rtpc_mouse)
	PORT_START("BTN")
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_BUTTON1) PORT_CODE(MOUSECODE_BUTTON1)
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_BUTTON2) PORT_CODE(MOUSECODE_BUTTON2)

	PORT_START("X")
	PORT_BIT(0x0fff, 0x0000, IPT_MOUSE_X) PORT_SENSITIVITY(100) PORT_KEYDELTA(0)

	PORT_START("Y")
	PORT_BIT(0x0fff, 0x0000, IPT_MOUSE_Y) PORT_SENSITIVITY(100) PORT_KEYDELTA(0)
INPUT_PORTS_END

ioport_constructor rtpc_mouse_device::device_input_ports() const
{
	return INPUT_PORTS_NAME(rtpc_mouse);
}
