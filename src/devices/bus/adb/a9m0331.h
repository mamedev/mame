// license:BSD-3-Clause
// copyright-holders: R. Belmont

// a9m0331.h - Apple ADB mouse

#ifndef MAME_BUS_ADB_A9M0331_H
#define MAME_BUS_ADB_A9M0331_H

#pragma once

#include "adb.h"
#include "cpu/m6805/m68705.h"

class a9m0331_device : public adb_device, public adb_slot_card_interface
{
public:
	a9m0331_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

protected:
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

	virtual const tiny_rom_entry *device_rom_region() const override ATTR_COLD;
	virtual ioport_constructor device_input_ports() const override ATTR_COLD;
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;

	virtual void adb_w(int state) override;

	required_device<m68705p_device> m_mcu;

private:
	void mcu_port_a_w(u8 data);
	void mcu_port_b_w(u8 data);
	void mcu_port_c_w(u8 data);
	u8 mcu_port_a_r();
	u8 mcu_port_b_r();
	u8 mcu_port_c_r();
};

DECLARE_DEVICE_TYPE(ADB_A9M0331, a9m0331_device)

#endif
