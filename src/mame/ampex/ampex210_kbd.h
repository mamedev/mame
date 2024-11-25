// license:BSD-3-Clause
// copyright-holders:AJR
#ifndef MAME_AMPEX_AMPEX210_KBD_H
#define MAME_AMPEX_AMPEX210_KBD_H

#pragma once

#include "cpu/mcs48/mcs48.h"


class ampex230_keyboard_device : public device_t
{
public:
	// construction/destruction
	ampex230_keyboard_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock = 0);

	// callback configuration
	auto data_out_callback() { return m_data_out_callback.bind(); }
	auto clock_out_callback() { return m_clock_out_callback.bind(); }

	// external control input
	void strobe_w(int state);

protected:
	// device_t implementation
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;
	virtual const tiny_rom_entry *device_rom_region() const override ATTR_COLD;
	virtual ioport_constructor device_input_ports() const override ATTR_COLD;
	virtual void device_start() override ATTR_COLD;

private:
	// MCU port handlers
	u8 bus_r();
	void p1_w(u8 data);
	void p2_w(u8 data);

	// callback objects
	devcb_write_line m_data_out_callback;
	devcb_write_line m_clock_out_callback;

	// device finders
	required_device<mcs48_cpu_device> m_mcu;
	required_ioport_array<14> m_key_row;

	u16 m_key_scan;
};

// device type declaration
DECLARE_DEVICE_TYPE(AMPEX230_KEYBOARD, ampex230_keyboard_device)

#endif // MAME_AMPEX_AMPEX210_KBD_H
