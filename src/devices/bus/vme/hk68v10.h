// license:BSD-3-Clause
// copyright-holders:Joakim Larsson Edstrom

#ifndef MAME_BUS_VME_HK68V10_H
#define MAME_BUS_VME_HK68V10_H

#pragma once

#include "bus/vme/vme.h"
#include "machine/z80scc.h"

class vme_hk68v10_card_device
	: public device_t
	, public device_vme_card_interface
{
public:
	vme_hk68v10_card_device(machine_config const &mconfig, char const *tag, device_t *owner, u32 clock);

protected:
	virtual const tiny_rom_entry *device_rom_region() const override ATTR_COLD;
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;
	virtual ioport_constructor device_input_ports() const override ATTR_COLD;
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

private:
	//uint16_t vme_a24_r();
	//void vme_a24_w(uint16_t data);
	//uint16_t vme_a16_r();
	//void vme_a16_w(uint16_t data);

	void hk68v10_mem(address_map &map) ATTR_COLD;

	required_device<cpu_device> m_maincpu;
	required_device<scc8530_device> m_sccterm;

	required_memory_region m_eprom;
	required_shared_ptr<uint16_t> m_ram;
	memory_passthrough_handler m_boot_mph;
};

DECLARE_DEVICE_TYPE(VME_HK68V10, vme_hk68v10_card_device)

#endif // MAME_BUS_VME_HK68V10_H
