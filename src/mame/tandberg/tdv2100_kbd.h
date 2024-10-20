// license:BSD-3-Clause
// copyright-holders:Frode van der Meeren
/***************************************************************************

    Tandberg TDV-2100 Series Keyboard

****************************************************************************/

#ifndef MAME_TANDBERG_TDV2100KBD_H
#define MAME_TANDBERG_TDV2100KBD_H

#pragma once

#include "machine/clock.h"

class tandberg_tdv2100_keyboard_device : public device_t
{
public:
	tandberg_tdv2100_keyboard_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock = 0);

	auto write_kstr_callback() { return m_write_kstr_cb.bind(); }
	auto write_cleark_callback() { return m_write_cleark_cb.bind(); }
	auto write_linek_callback() { return m_write_linek_cb.bind(); }
	auto write_transk_callback() { return m_write_transk_cb.bind(); }
	auto write_break_callback() { return m_write_break_cb.bind(); }

	void waitl_w(int state);
	void onlil_w(int state);
	void carl_w(int state);
	void errorl_w(int state);
	void enql_w(int state);
	void ackl_w(int state);
	void nakl_w(int state);

protected:
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;
	virtual void device_start() override ATTR_COLD;
	virtual ioport_constructor device_input_ports() const override ATTR_COLD;
	virtual const tiny_rom_entry *device_rom_region() const override ATTR_COLD;

private:
	void scan_next_column(int state);
	void new_keystroke(uint8_t key_nr, bool shift, bool control);
	void key_trigger();
	TIMER_CALLBACK_MEMBER(key_repeat);

	required_region_ptr<uint8_t>    m_keymap;
	required_region_ptr<uint8_t>    m_keyparams;
	required_ioport_array<15>       m_matrix;
	required_device<clock_device>   m_scan_clock;
	required_ioport                 m_sw_all_cap;
	required_ioport                 m_key_repeat_delay;
	required_ioport                 m_key_repeat_rate;

	output_finder<>                 m_online_led;
	output_finder<>                 m_carrier_led;
	output_finder<>                 m_error_led;
	output_finder<>                 m_enquiry_led;
	output_finder<>                 m_ack_led;
	output_finder<>                 m_nak_led;
	output_finder<>                 m_wait_led;
	output_finder<>                 m_shiftlock_led;

	devcb_write8                    m_write_kstr_cb;
	devcb_write_line                m_write_cleark_cb;
	devcb_write_line                m_write_linek_cb;
	devcb_write_line                m_write_transk_cb;
	devcb_write_line                m_write_break_cb;

	uint8_t m_column_counter;
	uint8_t m_keystate[15];

	bool m_shift;
	bool m_shift_lock;
	bool m_control;
	uint8_t m_char_buffer;
	uint8_t m_key_nr_in_buffer;
	emu_timer *m_key_repeat_trigger;

	bool m_8_bit_output;
};

// device type definition
DECLARE_DEVICE_TYPE(TANDBERG_TDV2100_KEYBOARD, tandberg_tdv2100_keyboard_device)

#endif // MAME_TANDBERG_TDV2100KBD_H
