// license:BSD-3-Clause
// copyright-holders:AJR

#ifndef MAME_CONCEPT_CONCEPT_KBD_H
#define MAME_CONCEPT_CONCEPT_KBD_H

#pragma once

#include "cpu/mcs48/mcs48.h"


//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> concept_keyboard_device

class concept_keyboard_device : public device_t
{
public:
	// device type constructor
	concept_keyboard_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock = 0);

	// callback configuration
	auto txd_callback() { return m_txd_callback.bind(); }

	// serial line input
	void write_rxd(int state);

protected:
	// device-level overrides
	virtual void device_resolve_objects() override ATTR_COLD;
	virtual void device_start() override ATTR_COLD;
	virtual ioport_constructor device_input_ports() const override ATTR_COLD;
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;
	virtual const tiny_rom_entry *device_rom_region() const override ATTR_COLD;

private:
	// MCU handlers
	u8 p1_r();
	void p2_w(u8 data);

	// address maps
	void prog_map(address_map &map) ATTR_COLD;

	// object finders
	required_device<mcs48_cpu_device> m_mcu;
	required_ioport_array<12> m_keys;
	output_finder<> m_led;

	// output callback
	devcb_write_line m_txd_callback;

	// internal state
	u8 m_p2_output;
	u8 m_key_input;
};

// device type declaration
DECLARE_DEVICE_TYPE(CONCEPT_KEYBOARD, concept_keyboard_device)

#endif // MAME_CONCEPT_CONCEPT_KBD_H
