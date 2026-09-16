// license:BSD-3-Clause
// copyright-holders:AJR

#ifndef MAME_APPLE_SPECTRED_KBD_H
#define MAME_APPLE_SPECTRED_KBD_H

#pragma once

class spectred_kbd_device : public device_t
{
public:
	// device type constructor
	spectred_kbd_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock = 0U);

	// output callback configuration
	auto kbdout_callback() { return m_kbdout_callback.bind(); }

protected:
	// device_t implementation
	virtual void device_start() override ATTR_COLD;
	virtual ioport_constructor device_input_ports() const override ATTR_COLD;
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;
	virtual const tiny_rom_entry *device_rom_region() const override ATTR_COLD;

private:
	// memory map
	void prog_map(address_map &map);

	// port handlers
	void p1_w(u8 data);
	void p2_w(u8 data);
	int key_r();

	// callback objects
	devcb_write8 m_kbdout_callback;

	// object finders
	required_ioport_array<8> m_key_matrix;
	output_finder<> m_caps_led;

	// internal state
	u8 m_p1;
};

// device type declaration
DECLARE_DEVICE_TYPE(SPECTRED_KEYBOARD, spectred_kbd_device)

#endif // MAME_APPLE_SPECTRED_KBD_H
