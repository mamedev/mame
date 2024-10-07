// license:BSD-3-Clause
// copyright-holders:AJR

#ifndef MAME_VISUAL_V102_KBD_H
#define MAME_VISUAL_V102_KBD_H

#pragma once

#include "cpu/mcs48/mcs48.h"

class visual_mcs48_keyboard_device : public device_t
{
public:
	auto txd_callback() { return m_txd_callback.bind(); }

	void write_rxd(int state);

protected:
	visual_mcs48_keyboard_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner);

	virtual void device_start() override ATTR_COLD;

	u8 p1_r();
	void p2_w(u8 data);
	void prog_w(int state);
	void prog_map(address_map &map) ATTR_COLD;
	void ext_map(address_map &map) ATTR_COLD;

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
	virtual ioport_constructor device_input_ports() const override ATTR_COLD;
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;
	virtual const tiny_rom_entry *device_rom_region() const override ATTR_COLD;
};

class v550_keyboard_device : public visual_mcs48_keyboard_device
{
public:
	v550_keyboard_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock = 0);

protected:
	virtual ioport_constructor device_input_ports() const override ATTR_COLD;
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;
	virtual const tiny_rom_entry *device_rom_region() const override ATTR_COLD;
};

DECLARE_DEVICE_TYPE(V102_KEYBOARD, v102_keyboard_device)
DECLARE_DEVICE_TYPE(V550_KEYBOARD, v550_keyboard_device)

#endif // MAME_VISUAL_V102_KBD_H
