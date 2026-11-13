// license:BSD-3-Clause
// copyright-holders:AJR

#ifndef MAME_APPLE_MPF3KBD_H
#define MAME_APPLE_MPF3KBD_H

#pragma once

#include "cpu/mcs48/mcs48.h"

class mpf3kbd_device : public device_t
{
public:
	// device type constructor
	mpf3kbd_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock = 0U);

	// callback configuration
	auto kbdout_callback() { return m_kbdout_callback.bind(); }
	auto reset_callback() { return m_reset_callback.bind(); }

	int pb0_r() { return BIT(m_modifiers->read(), 5); }
	int pb1_r() { return BIT(m_modifiers->read(), 6); }

	DECLARE_INPUT_CHANGED_MEMBER(reset_changed);
	void int_w(int state);

protected:
	// device_t implementation
	virtual void device_start() override ATTR_COLD;
	virtual ioport_constructor device_input_ports() const override ATTR_COLD;
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;
	virtual const tiny_rom_entry *device_rom_region() const override ATTR_COLD;

private:
	// I/O port handlers
	void p1_w(u8 data);
	u8 key_matrix_r();
	int shift_r();

	// memory map
	void prog_map(address_map &map);

	// callback objects
	devcb_write8 m_kbdout_callback;
	devcb_write_line m_reset_callback;

	// object finders
	required_device<mcs48_cpu_device> m_kbdmcu;
	required_ioport_array<11> m_keys;
	required_ioport m_modifiers;
	output_finder<> m_caps_led;
	output_finder<> m_num_led;
};

// device type declaration
DECLARE_DEVICE_TYPE(MPF3_KEYBOARD, mpf3kbd_device)

#endif // MAME_APPLE_MPF3KBD_H
