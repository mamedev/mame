// license:BSD-3-Clause
// copyright-holders:smf
#ifndef MAME_BUS_RS232_KEYBOARD_H
#define MAME_BUS_RS232_KEYBOARD_H

#pragma once

#include "rs232.h"
#include "machine/keyboard.h"

class serial_keyboard_device
	: public generic_keyboard_device
	, public device_buffered_serial_interface<16U>
	, public device_rs232_port_interface
{
public:
	serial_keyboard_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);
	serial_keyboard_device(const machine_config &mconfig, device_type type, const char *name, const char *tag, device_t *owner, uint32_t clock, const char *shortname, const char *source);

	virtual ioport_constructor device_input_ports() const override;

	virtual void input_txd(int state) override;

	void update_serial(int state);

protected:
	virtual void device_start() override;
	virtual void device_reset() override;
	virtual void device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr) override;
	virtual void tra_callback() override;
	virtual void send_key(uint8_t code) override;

private:
	virtual void received_byte(uint8_t byte) override;

	required_ioport m_rs232_txbaud;
	required_ioport m_rs232_startbits;
	required_ioport m_rs232_databits;
	required_ioport m_rs232_parity;
	required_ioport m_rs232_stopbits;
};

extern const device_type SERIAL_KEYBOARD;

#endif // MAME_BUS_RS232_KEYBOARD_H
