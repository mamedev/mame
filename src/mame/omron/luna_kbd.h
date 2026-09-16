// license:BSD-3-Clause
// copyright-holders:Patrick Mackinlay
#ifndef MAME_OMRON_LUNA_KBD_H
#define MAME_OMRON_LUNA_KBD_H

#pragma once

#include "bus/rs232/rs232.h"
#include "machine/keyboard.h"
#include "sound/beep.h"

#include "diserial.h"

class luna_keyboard_device
	: public device_t
	, public device_buffered_serial_interface<16U>
	, public device_rs232_port_interface
	, protected device_matrix_keyboard_interface<8U>
{
public:
	luna_keyboard_device(machine_config const &mconfig, char const *tag, device_t *owner, u32 clock);

	virtual void input_txd(int state) override { device_buffered_serial_interface::rx_w(state); }

protected:
	// device overrides
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;
	virtual ioport_constructor device_input_ports() const override ATTR_COLD;
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

	// device_buffered_serial_interface overrides
	virtual void tra_callback() override;
	virtual void tra_complete() override;
	virtual void received_byte(u8 data) override;

	// device_matrix_keyboard_interface overrides
	virtual void key_make(u8 row, u8 column) override;
	virtual void key_break(u8 row, u8 column) override;

private:
	TIMER_CALLBACK_MEMBER(beep_timer) { m_beep->set_state(0); }

	static constexpr int START_BIT_COUNT = 1;
	static constexpr int DATA_BIT_COUNT = 8;
	static constexpr device_serial_interface::parity_t PARITY = device_serial_interface::PARITY_NONE;
	static constexpr device_serial_interface::stop_bits_t STOP_BITS = device_serial_interface::STOP_BITS_1;
	static constexpr int BAUD = 9'600;

	required_device<beep_device> m_beep;
	output_finder<2> m_leds;

	emu_timer *m_beep_timer;
};

DECLARE_DEVICE_TYPE(LUNA_KEYBOARD, luna_keyboard_device)

#endif // MAME_OMRON_LUNA_KBD_H
