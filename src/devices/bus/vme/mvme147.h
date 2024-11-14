// license:BSD-3-Clause
// copyright-holders:Joakim Larsson Edstrom

#ifndef MAME_BUS_VME_MVME147_H
#define MAME_BUS_VME_MVME147_H

#pragma once

#include "bus/vme/vme.h"
#include "machine/z80scc.h"

class vme_mvme147_card_device
	: public device_t
	, public device_vme_card_interface
{
public:
	vme_mvme147_card_device(machine_config const &mconfig, char const *tag, device_t *owner, u32 clock);

protected:
	virtual const tiny_rom_entry *device_rom_region() const override ATTR_COLD;
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;
	virtual ioport_constructor device_input_ports() const override ATTR_COLD;
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

private:
	/* PCC - Peripheral Channel Controller */
	//uint32_t pcc32_r(offs_t offset);
	//void pcc32_w(offs_t offset, uint32_t data);
	uint16_t pcc16_r(offs_t offset);
	void pcc16_w(offs_t offset, uint16_t data);
	uint8_t pcc8_r(offs_t offset);
	void pcc8_w(offs_t offset, uint8_t data);
	uint8_t vmechip_r(offs_t offset);
	void vmechip_w(offs_t offset, uint8_t data);
	//uint16_t vme_a24_r();
	//void vme_a24_w(uint16_t data);
	//uint16_t vme_a16_r();
	//void vme_a16_w(uint16_t data);
	void mvme147_mem(address_map &map) ATTR_COLD;

	required_device<cpu_device> m_maincpu;
	required_device<scc85c30_device> m_sccterm;
	required_device<scc85c30_device> m_sccterm2;

	required_memory_region m_system;
	required_shared_ptr<uint32_t> m_ram;
	memory_passthrough_handler m_boot_mph;

	// PCC registers
	uint8_t   m_genpurp_stat = 0;

	// VME chip registers
	uint8_t   m_vc_cntl_conf = 0;
};

DECLARE_DEVICE_TYPE(VME_MVME147, vme_mvme147_card_device)

#endif // MAME_BUS_VME_MVME147_H
