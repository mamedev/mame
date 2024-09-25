// license:BSD-3-Clause
// copyright-holders:Patrick Mackinlay
#ifndef MAME_DEVICES_INTERPRO_KEYBOARD_HLE_H
#define MAME_DEVICES_INTERPRO_KEYBOARD_HLE_H

#pragma once

#include "keyboard.h"
#include "machine/keyboard.h"
#include "sound/beep.h"
#include "diserial.h"

namespace bus::interpro::keyboard {

class hle_device_base
	: public device_t
	, public device_buffered_serial_interface<16U>
	, public device_interpro_keyboard_port_interface
	, protected device_matrix_keyboard_interface<5U>
{
public:
	virtual ~hle_device_base() override;

	virtual void input_txd(int state) override;

protected:
	// constructor/destructor
	hle_device_base(machine_config const &mconfig, device_type type, char const *tag, device_t *owner, u32 clock);

	// device overrides
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

	// device_buffered_serial_interface overrides
	virtual void tra_callback() override;
	virtual void tra_complete() override;

	// device_matrix_keyboard_interface overrides
	virtual void key_make(u8 row, u8 column) override;
	virtual void key_break(u8 row, u8 column) override;

	// customised transmit_byte method
	void transmit_byte(u8 byte);
	virtual u8 translate(u8 row, u8 column) = 0;

private:
	static constexpr int START_BIT_COUNT = 1;
	static constexpr int DATA_BIT_COUNT = 8;
	static constexpr device_serial_interface::parity_t PARITY = device_serial_interface::PARITY_EVEN;
	static constexpr device_serial_interface::stop_bits_t STOP_BITS = device_serial_interface::STOP_BITS_1;
	static constexpr int BAUD = 1'200;

	enum {
		CLICK_TIMER_ID = 30'000
	};

	enum : int {
		LED_DISK = 0,
		LED_LOCK,
		LED_UNKNOWN,
		LED_L1,
		LED_L2,
		LED_L3,
		LED_L4
	};

	enum : u8 {
		BEEPER_BELL = 0x01U,
		BEEPER_CLICK = 0x02U
	};

	enum : u8 {
		RX_IDLE,
		RX_COMMAND,
		RX_FLAGS
	};

	enum : u8 {
		COMMAND_RESET = 0x01U,
		COMMAND_BELL_ON = 0x02U,
		COMMAND_BELL_OFF = 0x03U,
		COMMAND_CLICK_ON = 0x0aU,
		COMMAND_CLICK_OFF = 0x0bU,
		COMMAND_LED = 0x0eU,
		COMMAND_LAYOUT = 0x0fU
	};

	TIMER_CALLBACK_MEMBER(click_tick);

	// device_buffered_serial_interface overrides
	virtual void received_byte(u8 byte) override;

	emu_timer *m_click_timer;
	required_device<beep_device> m_beeper;

	u8 m_make_count;
	u8 m_rx_state;

	u8 m_keyclick;
	u8 m_beeper_state;
};


class hle_en_us_device : public hle_device_base
{
public:
	hle_en_us_device(machine_config const &mconfig, char const *tag, device_t *owner, u32 clock);

	virtual ioport_constructor device_input_ports() const override ATTR_COLD;

	virtual u8 translate(u8 row, u8 column) override;

	required_ioport m_modifiers;
};

} // namespace bus::interpro::keyboard

DECLARE_DEVICE_TYPE_NS(INTERPRO_HLE_EN_US_KEYBOARD, bus::interpro::keyboard, hle_en_us_device)

#endif // MAME_DEVICES_INTERPRO_KEYBOARD_HLE_H
