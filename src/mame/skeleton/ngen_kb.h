// license:BSD-3-Clause
// copyright-holders:Barry Rodewald
// Convergent NGEN keyboard

#ifndef MAME_SKELETON_NGEN_KB_H
#define MAME_SKELETON_NGEN_KB_H

#pragma once

#include "machine/keyboard.h"
#include "bus/rs232/rs232.h"
#include "diserial.h"

class ngen_keyboard_device : public device_t,
							public device_buffered_serial_interface<16U>,
							public device_rs232_port_interface,
							protected device_matrix_keyboard_interface<4U>
{
public:
	ngen_keyboard_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);
	virtual ioport_constructor device_input_ports() const override ATTR_COLD;
	virtual void input_txd(int state) override;
	void update_serial(int state);

protected:
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;
	virtual void tra_callback() override;
	virtual void received_byte(uint8_t byte) override;
	virtual void key_make(uint8_t row, uint8_t column) override;
	virtual void key_repeat(uint8_t row, uint8_t column) override;
	virtual void key_break(uint8_t row, uint8_t column) override;
	virtual void will_scan_row(uint8_t row) override;
	TIMER_CALLBACK_MEMBER(id_sequence_tick);

private:
	void write(uint8_t data);
	void send_key(uint8_t code);
	void send_key_scan();

	required_ioport m_modifiers;
	required_ioport m_rs232_txbaud;
	required_ioport m_rs232_databits;
	required_ioport m_rs232_parity;
	required_ioport m_rs232_stopbits;
	uint64_t m_keys_down;
	emu_timer *m_id_timer;
	uint8_t m_id_bytes_remaining;
	bool m_id_sequence_complete;
	bool m_scan_requested;
	bool m_last_shifted;
	bool m_echo = false;
};

DECLARE_DEVICE_TYPE(NGEN_KEYBOARD, ngen_keyboard_device)

#endif // MAME_SKELETON_NGEN_KB_H
