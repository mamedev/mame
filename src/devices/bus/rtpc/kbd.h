// license:BSD-3-Clause
// copyright-holders:Patrick Mackinlay

#ifndef MAME_BUS_RTPC_KBD_H
#define MAME_BUS_RTPC_KBD_H

#pragma once

#include "kbd_con.h"
#include "cpu/mcs48/mcs48.h"

class rtpc_kbd_device
	: public device_t
	, public device_rtpc_kbd_interface
{
public:
	rtpc_kbd_device(machine_config const &mconfig, char const *tag, device_t *owner, u32 clock);

protected:
	// device_t overrides
	virtual const tiny_rom_entry *device_rom_region() const override ATTR_COLD;
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;
	virtual ioport_constructor device_input_ports() const override ATTR_COLD;
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

	// device_rtpc_kbd_interface overrides
	virtual void data_w(int state) override;

private:
	void bus_w(u8 data);
	void p1_w(u8 data);
	void p2_w(u8 data);

	required_device<i8749_device> m_mcu;
	required_ioport_array<8> m_matrix[4];
	output_finder<3> m_leds;

	bool m_t1;
	u8 m_bus;
	u8 m_p1;
	u8 m_p2;
};

DECLARE_DEVICE_TYPE(RTPC_KBD, rtpc_kbd_device)

#endif // MAME_BUS_RTPC_KBD_H
