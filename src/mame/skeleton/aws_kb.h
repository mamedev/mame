// license:BSD-3-Clause
// copyright-holders:Barry Rodewald
// Convergent AWS keyboard

#ifndef MAME_SKELETON_AWS_KB_H
#define MAME_SKELETON_AWS_KB_H

#pragma once

#include "bus/rs232/rs232.h"

class aws_keyboard_device : public device_t,
							public device_buffered_serial_interface<8U>,
							public device_rs232_port_interface
{
public:
	aws_keyboard_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);
	virtual ioport_constructor device_input_ports() const override ATTR_COLD;

	virtual void received_byte(uint8_t byte) override;

protected:
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;
	virtual void rcv_complete() override;
	virtual void input_txd(int state) override;
	virtual void tra_callback() override;

private:
	required_ioport_array<16> m_rows;

	void write(uint8_t data);
	TIMER_CALLBACK_MEMBER(scan_keys);
	void send_key(uint8_t code);

	emu_timer* m_scan_timer;
	uint8_t m_keys_down[16][8];
	uint8_t m_last_reset;
};

DECLARE_DEVICE_TYPE(AWS_KEYBOARD, aws_keyboard_device)

#endif // MAME_SKELETON_AWS_KB_H
