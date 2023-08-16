// license:BSD-3-Clause
// copyright-holders:Patrick Mackinlay

#ifndef MAME_BUS_VME_ENP10_H
#define MAME_BUS_VME_ENP10_H

#pragma once

#include "cpu/m68000/m68000.h"
#include "machine/am79c90.h"
#include "bus/vme/vme.h"

class cmc_enp10_device
	: public device_t
	, public device_vme_card_interface
{
public:
	cmc_enp10_device(machine_config const &mconfig, char const *tag, device_t *owner, u32 clock);

protected:
	virtual const tiny_rom_entry *device_rom_region() const override;
	virtual void device_add_mconfig(machine_config &config) override;
	virtual ioport_constructor device_input_ports() const override;
	virtual void device_start() override;
	virtual void device_reset() override;

private:
	void cpu_mem(address_map &map);

	void interrupt();

	required_device<m68000_device> m_cpu;
	required_device<am7990_device> m_net;

	output_finder<2> m_led;

	u8 m_ier; // interrupt enable register
	u8 m_tir; // transmit interrupt register
	u8 m_rir; // receive interrupt register
	u8 m_uir; // utility interrupt register
	u8 m_exr; // exception register
	u8 m_csr; // control/status register
	u8 m_rer; // ram/rom enable register
	u8 m_hir; // host interrupt register

	memory_view m_boot;
};

DECLARE_DEVICE_TYPE(CMC_ENP10, cmc_enp10_device)

#endif // MAME_BUS_VME_ENP10_H
