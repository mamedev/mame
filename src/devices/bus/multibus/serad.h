// license:BSD-3-Clause
// copyright-holders:Patrick Mackinlay

#ifndef MAME_BUS_MULTIBUS_SERAD_H
#define MAME_BUS_MULTIBUS_SERAD_H

#pragma once

#include "multibus.h"

#include "cpu/i8085/i8085.h"
#include "machine/mc68681.h"
#include "machine/input_merger.h"
#include "bus/rs232/rs232.h"

class serad_device
	: public device_t
	, public device_multibus_interface
{
public:
	serad_device(machine_config const &mconfig, char const *tag, device_t *owner, u32 clock);

protected:
	// device_t overrides
	virtual const tiny_rom_entry *device_rom_region() const override ATTR_COLD;
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;
	virtual ioport_constructor device_input_ports() const override ATTR_COLD;
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

private:
	void mem_map(address_map &map) ATTR_COLD;
	void pio_map(address_map &map) ATTR_COLD;

	template <unsigned S> void rst55_w(u8 data) { m_cpu->set_input_line(I8085_RST55_LINE, S); }
	void apzint_w(u8 data) { int_w(2, 0); }

	required_device<i8085a_cpu_device> m_cpu;
	required_shared_ptr<u8> m_mbx;
	required_device<input_merger_any_high_device> m_rst65;
	required_device_array<scn2681_device, 3> m_duart;
	required_device_array<rs232_port_device, 6> m_port;

	bool m_installed;
};

DECLARE_DEVICE_TYPE(SERAD, serad_device)

#endif // MAME_BUS_MULTIBUS_SERAD_H
