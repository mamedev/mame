// license:BSD-3-Clause
// copyright-holders:AJR

#ifndef MAME_BUS_UTS_KBD_EXTW_H
#define MAME_BUS_UTS_KBD_EXTW_H

#pragma once

#include "bus/uts_kbd/uts_kbd.h"

class uts_extw_keyboard_device : public device_t, public device_uts_keyboard_interface
{
public:
	uts_extw_keyboard_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);

protected:
	virtual void ready_w(int state) override;

	virtual ioport_constructor device_input_ports() const override ATTR_COLD;
	virtual void device_start() override ATTR_COLD;
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;
	virtual const tiny_rom_entry *device_rom_region() const override ATTR_COLD;

private:
	u8 p1_r();
	void p1_w(u8 data);
	void p2_w(u8 data);
	void t0_clock(u32 clk);
	int t1_r();
	void prog_w(int state);

	void prog_map(address_map &map) ATTR_COLD;
	void ext_map(address_map &map) ATTR_COLD;

	required_ioport_array<16> m_keys;
	output_finder<> m_shift_led;

	u8 m_p1_output;
	u8 m_p2_output;
	u8 m_shift_register;
	bool m_ready_line;
	bool m_prog_line;
};

DECLARE_DEVICE_TYPE(UTS_EXTW_KEYBOARD, uts_extw_keyboard_device)

#endif // MAME_BUS_UTS_KBD_EXTW_H
