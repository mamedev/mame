// license:BSD-3-Clause
// copyright-holders:AJR
/**********************************************************************

    Altos II 105-key keyboard

**********************************************************************/

#ifndef MAME_ALTOS_ALTOS2_KEYBOARD_H
#define MAME_ALTOS_ALTOS2_KEYBOARD_H

#pragma once

#include "cpu/mcs48/mcs48.h"


//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> altos2_keyboard_device

class altos2_keyboard_device : public device_t
{
public:
	// device type constructor
	altos2_keyboard_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock = 0);

	// callback configuration
	auto txd_callback() { return m_txd_callback.bind(); }

	// serial line input
	void rxd_w(int state);

protected:
	// device-level overrides
	virtual void device_resolve_objects() override ATTR_COLD;
	virtual void device_start() override ATTR_COLD;
	virtual ioport_constructor device_input_ports() const override ATTR_COLD;
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;
	virtual const tiny_rom_entry *device_rom_region() const override ATTR_COLD;

private:
	// MCU handlers
	void leds_w(u8 data);
	u8 p1_r();
	void p2_w(u8 data);
	void prog_w(int state);

	// address maps
	void ext_map(address_map &map) ATTR_COLD;

	// object finders
	required_device<mcs48_cpu_device> m_mcu;
	required_ioport_array<16> m_keys;
	output_finder<> m_caps_led;
	output_finder<> m_insline_led;
	output_finder<> m_inschar_led;
	output_finder<> m_calc_led;

	// output callback
	devcb_write_line m_txd_callback;

	// internal state
	u8 m_key_row;
};

// device type declarations
DECLARE_DEVICE_TYPE(ALTOS2_KEYBOARD, altos2_keyboard_device)

#endif // MAME_ALTOS_ALTOS2_KEYBOARD_H
