// license:BSD-3-Clause
// copyright-holders:Patrick Mackinlay

#ifndef MAME_BUS_VME_TP880V_H
#define MAME_BUS_VME_TP880V_H

#pragma once

#include "cpu/m68000/m68000.h"
#include "cpu/m88000/m88000.h"
#include "machine/hd63450.h"
#include "machine/mc68681.h"
#include "machine/mc88200.h"
#include "machine/ncr53c90.h"
#include "machine/timekpr.h"
#include "machine/z8536.h"

#include "bus/vme/vme.h"
#include "bus/rs232/rs232.h"

class vme_tp880v_card_device
	: public device_t
	, public device_vme_card_interface
{
public:
	vme_tp880v_card_device(machine_config const &mconfig, char const *tag, device_t *owner, u32 clock);

protected:
	virtual const tiny_rom_entry *device_rom_region() const override ATTR_COLD;
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;
	virtual ioport_constructor device_input_ports() const override ATTR_COLD;
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

private:
	void cpu_mem(address_map &map) ATTR_COLD;
	void ios_mem(address_map &map) ATTR_COLD;

	required_device<mc88100_device> m_cpu;
	required_device_array<mc88200_device, 2> m_mmu;
	required_device<m68000_device> m_ios;

	required_device_array<z8036_device, 2> m_cio;
	required_device<hd63450_device> m_dma;
	required_device<m48t02_device> m_rtc;
	required_device<ncr53c90_device> m_scsi;
	required_device<scn2681_device> m_duart;

	required_device_array<rs232_port_device, 2> m_serial;

	required_shared_ptr<u32> m_ram;
	util::endian_cast<u32, u16, util::endianness::big> m_ram_68k;
};

DECLARE_DEVICE_TYPE(VME_TP880V, vme_tp880v_card_device)

#endif // MAME_BUS_VME_TP880V_H
