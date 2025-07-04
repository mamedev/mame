// license:BSD-3-Clause
// copyright-holders:Patrick Mackinlay

/*
 * While not physically compatible, the RT PC keyboard protocol is logically
 * and electrically identical to the PC/AT. This implementation is copied from
 * pc_kbdc and implements the same bi-directional clock and data lines.
 */

#include "emu.h"

#include "rtpc_kbdc.h"

//#define VERBOSE (LOG_GENERAL)
#include "logmacro.h"

DEFINE_DEVICE_TYPE(RTPC_KBDC, rtpc_kbdc_device, "rtpc_kbdc", "RT PC keyboard connector")

rtpc_kbdc_device::rtpc_kbdc_device(machine_config const &mconfig, char const *tag, device_t *owner, u32 clock)
	: device_t(mconfig, RTPC_KBDC, tag, owner, clock)
	, device_single_card_slot_interface<device_rtpc_kbd_interface>(mconfig, *this)
	, m_out_clock_cb(*this)
	, m_out_data_cb(*this)
	, m_clock_state(-1)
	, m_data_state(-1)
	, m_mb_clock_state(0)
	, m_mb_data_state(0)
	, m_kb_clock_state(1)
	, m_kb_data_state(1)
	, m_keyboard(nullptr)
{
}

void rtpc_kbdc_device::device_config_complete()
{
	m_keyboard = get_card_device();
}

void rtpc_kbdc_device::device_start()
{
	save_item(NAME(m_clock_state));
	save_item(NAME(m_data_state));

	save_item(NAME(m_mb_clock_state));
	save_item(NAME(m_mb_data_state));
	save_item(NAME(m_kb_clock_state));
	save_item(NAME(m_kb_data_state));

	m_clock_state = -1;
	m_data_state = -1;

	m_mb_clock_state = 1;
	m_mb_data_state = 1;
	m_kb_clock_state = 1;
	m_kb_data_state = 1;
}

void rtpc_kbdc_device::update_clock_state(bool fromkb)
{
	int new_clock_state = m_mb_clock_state & m_kb_clock_state;

	if (new_clock_state != m_clock_state)
	{
		m_clock_state = new_clock_state;
		LOG("%s clock: %d\n", fromkb? "<-" : "->", m_clock_state);

		// send state to host
		m_out_clock_cb(m_clock_state);

		// send state to keyboard
		if (m_keyboard)
			m_keyboard->clock_w(m_clock_state);
	}
}

void rtpc_kbdc_device::update_data_state(bool fromkb)
{
	int new_data_state = m_mb_data_state & m_kb_data_state;

	if (new_data_state != m_data_state)
	{
		m_data_state = new_data_state;
		LOG("%s data:  %d\n", fromkb? "<-" : "->", m_data_state);

		// send state to host
		m_out_data_cb(m_data_state);

		// send state to keyboard
		if (m_keyboard)
			m_keyboard->data_w(m_data_state);
	}
}

void rtpc_kbdc_device::clock_write_from_mb(int state)
{
	m_mb_clock_state = state;
	update_clock_state(false);
}

void rtpc_kbdc_device::data_write_from_mb(int state)
{
	m_mb_data_state = state;
	update_data_state(false);
}

void rtpc_kbdc_device::clock_write_from_kb(int state)
{
	m_kb_clock_state = state;
	update_clock_state(true);
}

void rtpc_kbdc_device::data_write_from_kb(int state)
{
	m_kb_data_state = state;
	update_data_state(true);
}

device_rtpc_kbd_interface::device_rtpc_kbd_interface(machine_config const &mconfig, device_t &device)
	: device_interface(device, "rtpc_kbd_if")
	, m_port(dynamic_cast<rtpc_kbdc_device *>(device.owner()))
{
}
