// license:BSD-3-Clause
// copyright-holders:AJR

#ifndef MAME_CITOH_CIT220_KBD_H
#define MAME_CITOH_CIT220_KBD_H

#pragma once

#include "cpu/mcs48/mcs48.h"
#include "sound/beep.h"


//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> cit220p_keyboard_device

class cit220p_keyboard_device : public device_t
{
public:
	// device type constructor
	cit220p_keyboard_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock = 0);

	// callback configuration
	auto txd_callback() { return m_txd_callback.bind(); }

	// serial line input
	void write_rxd(int state);

protected:
	// device_t implementation
	virtual void device_resolve_objects() override ATTR_COLD;
	virtual void device_start() override ATTR_COLD;
	virtual ioport_constructor device_input_ports() const override ATTR_COLD;
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;
	virtual const tiny_rom_entry *device_rom_region() const override ATTR_COLD;

private:
	// MCU handlers
	u8 mcu_bus_r();
	u8 mcu_p2_r();
	void mcu_p2_w(u8 data);
	void mcu_movx_w(u8 data);

	// address maps
	void prog_map(address_map &map) ATTR_COLD;
	void ext_map(address_map &map) ATTR_COLD;

	// object finders
	required_device<mcs48_cpu_device> m_mcu;
	required_device<beep_device> m_beeper;
	required_ioport_array<13> m_rows;
	required_ioport m_modifiers;
	output_finder<8> m_leds;

	// output callback
	devcb_write_line m_txd_callback;
};

// device type declaration
DECLARE_DEVICE_TYPE(CIT220P_KEYBOARD, cit220p_keyboard_device)

#endif // MAME_CITOH_CIT220_KBD_H
