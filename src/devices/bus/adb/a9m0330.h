// license:BSD-3-Clause
// copyright-holders: R. Belmont

// a9m0330.h - Apple IIgs ADB keyboard

#ifndef MAME_BUS_ADB_A9M0330_H
#define MAME_BUS_ADB_A9M0330_H

#pragma once

#include "adb.h"
#include "cpu/mcs48/mcs48.h"

class a9m0330_device : public adb_device, public adb_slot_card_interface
{
public:
	a9m0330_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);

protected:
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

	virtual const tiny_rom_entry *device_rom_region() const override ATTR_COLD;
	virtual ioport_constructor device_input_ports() const override ATTR_COLD;
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;

	virtual void adb_w(int state) override;

	required_device<i8048_device> m_mcu;
	required_ioport_array<10> m_rows;

	void program_map(address_map &map) ATTR_COLD;

private:
	u8 bus_r();
	void p1_w(u8 data);
	void p2_w(u8 data);
	int t0_r();
	int t1_r();

	int m_adb_state;
	int m_kbd_row;
	int m_our_last_adb_state;
};

DECLARE_DEVICE_TYPE(ADB_A9M0330, a9m0330_device)

#endif
