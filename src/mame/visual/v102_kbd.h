// license:BSD-3-Clause
// copyright-holders:AJR

#ifndef MAME_MACHINE_V102_KBD_H
#define MAME_MACHINE_V102_KBD_H

#pragma once

#include "cpu/mcs48/mcs48.h"

class visual_mcs48_keyboard_device : public device_t
{
public:
	auto txd_callback() { return m_txd_callback.bind(); }

	DECLARE_WRITE_LINE_MEMBER(write_rxd);

protected:
	visual_mcs48_keyboard_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner);

	virtual void device_resolve_objects() override;
	virtual void device_start() override;

	u8 p1_r();
	void p2_w(u8 data);
	DECLARE_WRITE_LINE_MEMBER(prog_w);
	void prog_map(address_map &map);
	void ext_map(address_map &map);

	devcb_write_line m_txd_callback;
	required_device<mcs48_cpu_device> m_kbdc;
	required_ioport_array<12> m_keys;

	u8 m_key_select;
	bool m_prog_line;
};

class v102_keyboard_device : public visual_mcs48_keyboard_device
{
public:
	v102_keyboard_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock = 0);

protected:
	virtual ioport_constructor device_input_ports() const override;
	virtual void device_add_mconfig(machine_config &config) override;
	virtual const tiny_rom_entry *device_rom_region() const override;
};

class v550_keyboard_device : public visual_mcs48_keyboard_device
{
public:
	v550_keyboard_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock = 0);

protected:
	virtual ioport_constructor device_input_ports() const override;
	virtual void device_add_mconfig(machine_config &config) override;
	virtual const tiny_rom_entry *device_rom_region() const override;
};

DECLARE_DEVICE_TYPE(V102_KEYBOARD, v102_keyboard_device)
DECLARE_DEVICE_TYPE(V550_KEYBOARD, v550_keyboard_device)

#endif // MAME_MACHINE_V102_KBD_H
