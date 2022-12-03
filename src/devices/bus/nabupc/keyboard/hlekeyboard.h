// license:BSD-3-Clause
// copyright-holders:Brian Johnson
/***************************************************************************

    NABU PC HLE Keyboard Interface

***************************************************************************/


#ifndef MAME_BUS_NABU_KEYBOARD_HLE_H
#define MAME_BUS_NABU_KEYBOARD_HLE_H

#pragma once

#include "bus/rs232/rs232.h"
#include "machine/keyboard.h"
#include "diserial.h"

namespace bus::nabupc::keyboard {

//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

class hle_keyboard_device
	: public device_t
	, public device_buffered_serial_interface<16U>
	, public device_rs232_port_interface
	, protected device_matrix_keyboard_interface<16U>
{
public:
	// constructor/destructor
	hle_keyboard_device(machine_config const &mconfig, char const *tag, device_t *owner, u32 clock);

protected:
	// device overrides
	virtual void device_start() override;
	virtual void device_reset() override;

	virtual ioport_constructor device_input_ports() const override;

	// device_buffered_serial_interface overrides
	virtual void tra_callback() override;

	// device_matrix_keyboard_interface overrides
	virtual void key_make(uint8_t row, uint8_t column) override;
	virtual void key_break(uint8_t row, uint8_t column) override;
	virtual void scan_complete() override;

	uint8_t translate(uint8_t row, uint8_t column);

private:
	static constexpr int START_BIT_COUNT = 1;
	static constexpr int DATA_BIT_COUNT = 8;
	static constexpr device_serial_interface::parity_t PARITY = device_serial_interface::PARITY_NONE;
	static constexpr device_serial_interface::stop_bits_t STOP_BITS = device_serial_interface::STOP_BITS_1;
	static constexpr int BAUD = 6'993;

	TIMER_CALLBACK_MEMBER(watchdog_tick);

	virtual void received_byte(uint8_t byte) override {}

	required_ioport m_modifiers;
	required_ioport m_gameport1;
	required_ioport m_gameport2;

	emu_timer *m_watchdog_timer;
	uint8_t m_prev_gameport1;
	uint8_t m_prev_gameport2;
};

} // bus::nabupc::keyboard

DECLARE_DEVICE_TYPE_NS(NABUPC_HLE_KEYBOARD, bus::nabupc::keyboard, hle_keyboard_device)

#endif // MAME_BUS_NABU_KEYBOARD_HLE_H
