// license:BSD-3-Clause
// copyright-holders:AJR

#ifndef MAME_MACHINE_TEK410X_KEYBOARD_H
#define MAME_MACHINE_TEK410X_KEYBOARD_H

#pragma once

#include "cpu/mcs48/mcs48.h"


//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> tek410x_keyboard_device

class tek410x_keyboard_device : public device_t
{
public:
	// device type constructor
	tek410x_keyboard_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock = 0);

	// callback configuration
	auto tdata_callback() { return m_tdata_callback.bind(); }
	auto rdata_callback() { return m_rdata_callback.bind(); }

	// line inputs
	DECLARE_WRITE_LINE_MEMBER(kdi_w);
	DECLARE_WRITE_LINE_MEMBER(kdo_w);
	DECLARE_WRITE_LINE_MEMBER(reset_w);

protected:
	// device-level overrides
	virtual void device_resolve_objects() override;
	virtual void device_start() override;
	virtual ioport_constructor device_input_ports() const override;
	virtual void device_add_mconfig(machine_config &config) override;
	virtual const tiny_rom_entry *device_rom_region() const override;

private:
	// MCU handlers
	u8 p1_r();
	u8 p2_r();
	void p2_w(u8 data);

	// address maps
	void ext_map(address_map &map);

	// object finders
	required_device<mcs48_cpu_device> m_mcu;
	required_ioport_array<12> m_key_matrix;
	required_ioport m_config;

	// output callbacks
	devcb_write_line m_tdata_callback;
	devcb_write_line m_rdata_callback;

	// internal state
	u8 m_select;
	u8 m_p2_out;
	bool m_kdi;
	bool m_kdo;
};

// device type declaration
DECLARE_DEVICE_TYPE(TEK410X_KEYBOARD, tek410x_keyboard_device)

#endif // MAME_MACHINE_TEK410X_KEYBOARD_H
