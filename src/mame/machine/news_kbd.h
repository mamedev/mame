// license:BSD-3-Clause
// copyright-holders:Patrick Mackinlay

#ifndef MAME_MACHINE_NEWS_KBD_H
#define MAME_MACHINE_NEWS_KBD_H

#pragma once

#include "machine/keyboard.h"

class news_hle_kbd_device
	: public device_t
	, public device_matrix_keyboard_interface<8U>
{
public:
	news_hle_kbd_device(machine_config const &mconfig, char const *tag, device_t *owner, u32 clock = 0);

	auto irq_out() { return m_irq_out_cb.bind(); }
	u8 data_r();

protected:
	// device_t overrides
	virtual void device_start() override;
	virtual void device_reset() override;
	virtual ioport_constructor device_input_ports() const override;

	// device_matrix_keyboard_interface overrides
	virtual void key_make(u8 row, u8 column) override;
	virtual void key_break(u8 row, u8 column) override;

private:
	void push_key(u8 code);
	void out_irq(bool state);

	devcb_write_line m_irq_out_cb;

	util::fifo<u8, 8> m_fifo;
	bool m_irq_out_state;
};

DECLARE_DEVICE_TYPE(NEWS_HLE_KBD, news_hle_kbd_device)

#endif // MAME_MACHINE_NEWS_KBD_H
