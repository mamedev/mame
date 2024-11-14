// license:BSD-3-Clause
// copyright-holders:Patrick Mackinlay
#ifndef MAME_MG1_KBD_H
#define MAME_MG1_KBD_H

#pragma once

#include "cpu/mcs48/mcs48.h"

class mg1_kbd_device
	: public device_t
{
public:
	auto out_data() { return m_data_cb.bind(); }

	mg1_kbd_device(machine_config const &mconfig, char const *tag, device_t *owner, u32 clock = 0);

protected:
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;
	virtual void device_start() override ATTR_COLD;

	virtual tiny_rom_entry const *device_rom_region() const override ATTR_COLD;
	virtual ioport_constructor device_input_ports() const override ATTR_COLD;

private:
	virtual void map_mem(address_map &map) ATTR_COLD;
	virtual void map_pio(address_map &map) ATTR_COLD;

	devcb_write_line m_data_cb;

	required_device<i8039_device> m_mcu;
	required_ioport m_w;
	required_ioport_array<16> m_matrix;
	output_finder<2> m_led;
};

DECLARE_DEVICE_TYPE(MG1_KBD, mg1_kbd_device)

#endif // MAME_MG1_KBD_H
