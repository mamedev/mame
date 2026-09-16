// license:BSD-3-Clause
// copyright-holders:AJR

#ifndef MAME_BUS_UTS_KBD_400KBD_H
#define MAME_BUS_UTS_KBD_400KBD_H

#pragma once

#include "bus/uts_kbd/uts_kbd.h"

class uts_400_keyboard_device : public device_t, public device_uts_keyboard_interface
{
public:
	uts_400_keyboard_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);

protected:
	virtual void ready_w(int state) override;

	virtual ioport_constructor device_input_ports() const override ATTR_COLD;
	virtual void device_start() override ATTR_COLD;
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;
	virtual const tiny_rom_entry *device_rom_region() const override ATTR_COLD;

private:
	u8 bus_r();
	void p1_w(u8 data);
	void p2_w(u8 data);
	int t1_r();
	void prog_w(int state);

	void ext_map(address_map &map) ATTR_COLD;

	required_ioport_array<16> m_keys;
	output_finder<> m_shift_led;

	u8 m_p2_output;
	u8 m_input_select;
	bool m_ready_line;
	bool m_prog_line;
};

DECLARE_DEVICE_TYPE(UTS_400_KEYBOARD, uts_400_keyboard_device)

#endif // MAME_BUS_UTS_KBD_400KBD_H
