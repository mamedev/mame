// license:BSD-3-Clause
// copyright-holders:Patrick Mackinlay

#ifndef MAME_BUS_VME_TP881V_H
#define MAME_BUS_VME_TP881V_H

#pragma once

#include "cpu/m88000/m88000.h"
#include "machine/i82586.h"
#include "machine/hd63450.h"
#include "machine/mc88200.h"
#include "machine/ncr53c90.h"
#include "machine/nmc9306.h"
#include "machine/timekpr.h"
#include "machine/z80scc.h"
#include "machine/z8536.h"

#include "bus/vme/vme.h"
#include "bus/rs232/rs232.h"

class vme_tp881v_card_device
	: public device_t
	, public device_vme_card_interface
{
public:
	vme_tp881v_card_device(machine_config const &mconfig, char const *tag, device_t *owner, u32 clock);

protected:
	virtual const tiny_rom_entry *device_rom_region() const override ATTR_COLD;
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;
	virtual ioport_constructor device_input_ports() const override ATTR_COLD;
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

private:
	void cpu_mem(address_map &map) ATTR_COLD;
	void net_mem(address_map &map) ATTR_COLD;

	required_device<mc88100_device> m_cpu;
	required_device_array<mc88200_device, 2> m_mmu;
	required_device_array<z8036_device, 8> m_cio;

	required_device<m48t02_device> m_rtc;
	required_device_array<ncr53c90a_device, 2> m_scsi;
	required_device<i82596_device> m_net;
	required_device_array<scc8030_device, 2> m_scc;
	required_device<hd63450_device> m_scc_dma;

	required_device<z8036_device> m_vcs; // vme control and status cio
	required_device_array<z8036_device, 2> m_gcs; // global control and status cio
	required_device<nmc9306_device> m_eeprom;

	required_device_array<rs232_port_device, 4> m_serial;
};

DECLARE_DEVICE_TYPE(VME_TP881V, vme_tp881v_card_device)

#endif // MAME_BUS_VME_TP881V_H
