// license:BSD-3-Clause
// copyright-holders:Patrick Mackinlay

#ifndef MAME_SGI_GM1_H
#define MAME_SGI_GM1_H

#pragma once

#include "cpu/m68000/m68020.h"

#include "machine/mc68681.h"
#include "machine/timer.h"

#include "bus/rs232/rs232.h"
#include "bus/vme/vme.h"

class sgi_gm1_device
	: public device_t
	, public device_vme_card_interface
{
public:
	sgi_gm1_device(machine_config const &mconfig, char const *tag, device_t *owner, u32 clock);

protected:
	virtual tiny_rom_entry const *device_rom_region() const override ATTR_COLD;
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;
	virtual ioport_constructor device_input_ports() const override ATTR_COLD;
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

	void cpu_map(address_map &map) ATTR_COLD;
	void vme_a16_map(address_map &map) ATTR_COLD;
	void vme_a32_map(address_map &map) ATTR_COLD;

	u16 status_r();
	void reset_w(u16 data);
	void interrupt_w(u16 data);
	void interrupt_disable_w(u16 data);
	void interrupt_vector_w(u16 data);

private:
	required_device<m68020_device> m_cpu;
	required_device<scn2681_device> m_duart;
	required_device_array<rs232_port_device, 2> m_serial;

	u16 m_pp_wc; // pp sram word count
};

DECLARE_DEVICE_TYPE(SGI_GM1, sgi_gm1_device)

#endif // MAME_SGI_GM1_H
