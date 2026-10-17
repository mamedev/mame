// license:BSD-3-Clause
// copyright-holders:Barry Rodewald
// Convergent NGEN keyboard

#ifndef MAME_SKELETON_NGEN_KB_H
#define MAME_SKELETON_NGEN_KB_H

#pragma once

#include "bus/rs232/keyboard.h"

class ngen_keyboard_device : public serial_keyboard_device
{
public:
	ngen_keyboard_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);
	virtual ioport_constructor device_input_ports() const override ATTR_COLD;

protected:
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;
	virtual void rcv_complete() override;
	virtual void key_make(uint8_t row, uint8_t column) override;
	virtual void key_repeat(uint8_t row, uint8_t column) override;
	virtual void key_break(uint8_t row, uint8_t column) override;
	virtual void send_key(uint8_t code) override;
	TIMER_CALLBACK_MEMBER(id_sequence_tick);

private:
	void write(uint8_t data);
	void send_key_scan();

	uint64_t m_keys_down;
	uint8_t m_last_reset;
	emu_timer *m_id_timer;
	uint8_t m_id_bytes_remaining;
	bool m_id_sequence_complete;
	bool m_scan_requested;
	bool m_last_shifted;
	bool m_echo = false;
};

DECLARE_DEVICE_TYPE(NGEN_KEYBOARD, ngen_keyboard_device)

#endif // MAME_SKELETON_NGEN_KB_H
