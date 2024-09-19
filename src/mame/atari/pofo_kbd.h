// license:BSD-3-Clause
// copyright-holders:Jaen Saul
#ifndef MAME_ATARI_POFO_KBD_H
#define MAME_ATARI_POFO_KBD_H

#pragma once

#include "machine/keyboard.h"

class pofo_keyboard_device : public device_t, protected device_matrix_keyboard_interface<8U>
{
public:
	pofo_keyboard_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock = 0);

	// callbacks
	auto int_handler() { return m_int_handler.bind(); }

	// from host
	uint8_t read() { return m_data; }

protected:
	void key_make(u8 row, u8 column) override;
	void key_break(u8 row, u8 column) override;

	void device_start() override ATTR_COLD;
	void device_reset() override ATTR_COLD;

	ioport_constructor device_input_ports() const override;

private:
	devcb_write_line m_int_handler;
	uint8_t m_data;

	void trigger();
};

DECLARE_DEVICE_TYPE(POFO_KEYBOARD, pofo_keyboard_device);

#endif // MAME_ATARI_POFO_KBD_H
