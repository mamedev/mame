// license:BSD-3-Clause
// copyright-holders: BlueRain
#ifndef MAME_HITACHI_B16_KBD_H
#define MAME_HITACHI_B16_KBD_H

#pragma once

#include "machine/keyboard.h"

class b16_kbd_device : public device_t, protected device_matrix_keyboard_interface<16>
{
public:
	b16_kbd_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock = 0);

	auto irq_callback() { return m_irq_cb.bind(); }
	u8 data_r();
	void data_w(u8 data);
	u8 status_r();

protected:
	virtual ioport_constructor device_input_ports() const override ATTR_COLD;
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

	virtual void will_scan_row(u8 row) override;
	virtual void key_make(u8 row, u8 column) override;
	virtual void key_repeat(u8 row, u8 column) override;

private:
	void update_modifiers();
	void push(u8 data);

	required_ioport m_modifiers;
	required_ioport m_lock_keys;
	devcb_write_line m_irq_cb;
	std::array<u8, 32> m_fifo{};
	u8 m_last_code = 0;
	u8 m_head = 0, m_count = 0;
	u8 m_modifier_state = 0, m_lock_pressed = 0, m_lock_state = 0;
};

DECLARE_DEVICE_TYPE(B16_KBD, b16_kbd_device)

#endif // MAME_HITACHI_B16_KBD_H
