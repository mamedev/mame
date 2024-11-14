// license:BSD-3-Clause
// copyright-holders:AJR
/**********************************************************************

    Micro Craft Dimension 68000 84-key keyboard

**********************************************************************/
#ifndef MAME_MICROCRAFT_DIM68K_KBD_H
#define MAME_MICROCRAFT_DIM68K_KBD_H

#pragma once

#include "cpu/mcs48/mcs48.h"


//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> dim68k_keyboard_device

class dim68k_keyboard_device : public device_t
{
public:
	// device type constructor
	dim68k_keyboard_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock = 0);

	// callback configuration
	auto txd_callback() { return m_txd_callback.bind(); }

protected:
	// device_t implementation
	virtual void device_resolve_objects() override ATTR_COLD;
	virtual void device_start() override ATTR_COLD;
	virtual ioport_constructor device_input_ports() const override ATTR_COLD;
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;
	virtual const tiny_rom_entry *device_rom_region() const override ATTR_COLD;

private:
	// MCU handlers
	u8 p1_r();
	void p2_w(u8 data);
	void prog_w(int state);
	u8 eprom_r();

	// address map
	void ext_map(address_map &map) ATTR_COLD;

	// object finders
	required_device<mcs48_cpu_device> m_mcu;
	required_region_ptr<u8> m_eprom;
	required_ioport_array<11> m_keys;
	output_finder<> m_capslock_led;
	output_finder<> m_numlock_led;

	// output callback
	devcb_write_line m_txd_callback;

	// internal state
	u8 m_p1_in;
};

// device type declarations
DECLARE_DEVICE_TYPE(DIM68K_KEYBOARD, dim68k_keyboard_device)

#endif // MAME_MICROCRAFT_DIM68K_KBD_H
