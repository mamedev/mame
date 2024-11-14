// license:BSD-3-Clause
// copyright-holders:Patrick Mackinlay

#ifndef MAME_SONY_NEWS_HID_H
#define MAME_SONY_NEWS_HID_H

#pragma once

#include "machine/keyboard.h"

class news_hid_hle_device
	: public device_t
	, public device_matrix_keyboard_interface<8U>
{
public:
	news_hid_hle_device(machine_config const &mconfig, char const *tag, device_t *owner, u32 clock = 0);

	enum news_hid_device : unsigned
	{
		KEYBOARD = 0,
		MOUSE    = 1,
	};
	template <news_hid_device Device> auto irq_out() { return m_irq_out_cb[Device].bind(); }

	void map(address_map &map) ATTR_COLD;
	void map_68k(address_map &map) ATTR_COLD;
	void map_apbus(address_map &map) ATTR_COLD;

protected:
	// device_t overrides
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;
	virtual ioport_constructor device_input_ports() const override ATTR_COLD;

	// device_matrix_keyboard_interface overrides
	virtual void key_make(u8 row, u8 column) override;
	virtual void key_break(u8 row, u8 column) override;
	virtual void scan_complete() override;

private:
	void push_key(u8 code);

	template <news_hid_device Device> void out_irq(bool state);

	template <news_hid_device Device> u8 status_r() { return (!m_fifo[Device].empty() ? 2 : 0) | (m_fifo[Device].full() ? 1 : 0); }
	template <news_hid_device Device> u8 data_r();
	template <news_hid_device Device> void reset_w(u8 data);
	template <news_hid_device Device> void init_w(u8 data);
	template <news_hid_device Device> void ien_w(u8 data);

	u8 status_68k_r();

	required_ioport m_mouse_x_axis;
	required_ioport m_mouse_y_axis;
	required_ioport m_mouse_buttons;

	devcb_write_line::array<2> m_irq_out_cb;

	util::fifo<u8, 8> m_fifo[2];
	bool m_irq_enabled[2];
	bool m_irq_out_state[2];

	// mouse state
	s16 m_mouse_x;
	s16 m_mouse_y;
	u8 m_mouse_b;
};

DECLARE_DEVICE_TYPE(NEWS_HID_HLE, news_hid_hle_device)

#endif // MAME_SONY_NEWS_HID_H
