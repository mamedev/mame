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

	virtual ioport_constructor device_input_ports() const override ATTR_COLD;

	virtual void input_txd(int state) override;

	void update_serial(int state);

protected:
	serial_keyboard_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock);

	virtual void device_reset() override ATTR_COLD;
	virtual void tra_callback() override;
	virtual void send_key(uint8_t code) override;

private:
	virtual void received_byte(uint8_t byte) override;

	required_ioport m_rs232_txbaud;
	required_ioport m_rs232_databits;
	required_ioport m_rs232_parity;
	required_ioport m_rs232_stopbits;
};

DECLARE_DEVICE_TYPE(SERIAL_KEYBOARD, serial_keyboard_device)

#endif // MAME_BUS_RS232_KEYBOARD_H
