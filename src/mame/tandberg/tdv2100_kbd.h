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

	void w_waitl(int state);
	void w_onlil(int state);
	void w_carl(int state);
	void w_errorl(int state);
	void w_enql(int state);
	void w_ackl(int state);
	void w_nakl(int state);

protected:
	required_memory_region          m_keymap;
	required_memory_region          m_keyparams;
	required_ioport_array<15>       m_matrix;
	required_device<clock_device>   m_scan_clock;

	output_finder<>                 m_online_led;
	output_finder<>                 m_carrier_led;
	output_finder<>                 m_error_led;
	output_finder<>                 m_enquiry_led;
	output_finder<>                 m_ack_led;
	output_finder<>                 m_nak_led;
	output_finder<>                 m_wait_led;
	output_finder<>                 m_shiftlock_led;

	virtual void device_add_mconfig(machine_config &config) override;
	virtual void device_start() override;
	virtual void device_reset() override;
	virtual const tiny_rom_entry *device_rom_region() const override;
	virtual ioport_constructor device_input_ports() const override;

	void scan_next_column(int state);
	void new_keystroke(uint8_t key_nr, bool shift = false, bool ctrl = false);

	devcb_write8 m_write_kstr_cb;
	devcb_write_line m_write_cleark_cb;
	devcb_write_line m_write_linek_cb;
	devcb_write_line m_write_transk_cb;
	devcb_write_line m_write_break_cb;

	bool inhibit_key_from_params;
	bool all_caps;
	int column_counter;
	int keystate[15];
	bool shift;
	bool shift_lock;
	bool control;
};

// device type definition
DECLARE_DEVICE_TYPE(TANDBERG_TDV2100_KEYBOARD, tandberg_tdv2100_keyboard_device)

#endif // MAME_TANDBERG_TDV2100KBD_H
