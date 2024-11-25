// license:BSD-3-Clause
// copyright-holders:AJR

#ifndef MAME_BUS_PC_KBD_CHERRY_MX1500_H
#define MAME_BUS_PC_KBD_CHERRY_MX1500_H

#pragma once

#include "pc_kbdc.h"

class cherry_g80_1500_device : public device_t, public device_pc_kbd_interface
{
public:
	cherry_g80_1500_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);

protected:
	virtual void data_write(int state) override;

	virtual ioport_constructor device_input_ports() const override ATTR_COLD;
	virtual void device_start() override ATTR_COLD;
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;
	virtual const tiny_rom_entry *device_rom_region() const override ATTR_COLD;

private:
	void mcu_p1_w(u8 data);
	u8 mcu_p3_r();
	void mcu_p3_w(u8 data);
	u8 matrix_r(offs_t offset);
	void outputs_w(offs_t offset, u8 data);

	void prog_map(address_map &map) ATTR_COLD;
	void ext_map(address_map &map) ATTR_COLD;

	required_device<cpu_device> m_mcu;
	required_ioport_array<13> m_keys;
	output_finder<3> m_leds;

	u8 m_p1;
};

// device type declaration
DECLARE_DEVICE_TYPE(CHERRY_G80_1500, cherry_g80_1500_device)

#endif // MAME_BUS_PC_KBD_CHERRY_MX1500_H
