// license:BSD-3-Clause
// copyright-holders:Carl
#ifndef MAME_MACHINE_MIDIKBD_H
#define MAME_MACHINE_MIDIKBD_H

#pragma once

#include "diserial.h"


class midi_keyboard_device : public device_t, public device_serial_interface
{
public:
	midi_keyboard_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);
	ioport_constructor device_input_ports() const override;

	auto tx_callback() { return m_out_tx_func.bind(); }

protected:
	void device_start() override;
	void tra_callback() override;
	void tra_complete() override;
	virtual void device_timer(emu_timer &timer, device_timer_id id, int param) override;

private:
	void push_tx(uint8_t data) { ++m_head %= 16; m_buffer[m_head] = data; }

	devcb_write_line m_out_tx_func;
	emu_timer *m_keyboard_timer;
	required_ioport m_keyboard;
	uint32_t m_keyboard_state;
	uint8_t m_buffer[16];
	uint8_t m_head, m_tail;
};

DECLARE_DEVICE_TYPE(MIDI_KBD, midi_keyboard_device)

#endif // MAME_MACHINE_MIDIKBD_H
