// license:BSD-3-Clause
// copyright-holders:Angelo Salese

#ifndef MAME_NEC_PC6001_KBD_H
#define MAME_NEC_PC6001_KBD_H

#pragma once

#include "machine/keyboard.h"

#include <tuple>


class pc6001_kbd_device : public device_t
						, protected device_matrix_keyboard_interface<10>
{
public:
	pc6001_kbd_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock = 0);

	auto key_irq_cb()    { return m_key_irq_cb.bind(); }
	auto keyfn_irq_cb()  { return m_keyfn_irq_cb.bind(); }
	auto joy_irq_cb()    { return m_joy_irq_cb.bind(); }

	u8 read_key_press() { return m_scan_code; }
	u8 read_joy_press() { return m_joy_code; }

	void joy_cmd_w(int state);

protected:
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

	virtual ioport_constructor device_input_ports() const override ATTR_COLD;

	virtual void key_make(uint8_t row, uint8_t column) override;
	virtual void key_break(uint8_t row, uint8_t column) override;
	virtual void key_repeat(uint8_t row, uint8_t column) override;
	std::tuple<uint8_t, bool> translate(uint8_t row, uint8_t column);

private:
	devcb_write_line m_key_irq_cb;
	devcb_write_line m_keyfn_irq_cb;
	devcb_write_line m_joy_irq_cb;

	emu_timer *m_joy_trigger_timer;

	u8 m_scan_code;
	u8 m_joy_code;
	bool m_fn_key;

	TIMER_CALLBACK_MEMBER(joy_trigger_cb);

	uint8_t convert_key_to_joy_map();
};

DECLARE_DEVICE_TYPE(PC6001_KBD, pc6001_kbd_device)

#endif // MAME_NEC_PC6001_KBD_H
