// license:BSD-3-Clause
// copyright-holders:Patrick Mackinlay

#ifndef MAME_BUS_VME_MVME187_H
#define MAME_BUS_VME_MVME187_H

#pragma once

#include "cpu/m88000/m88000.h"

#include "machine/53c7xx.h"
//#include "machine/cd2400.h"
#include "machine/i82586.h"
#include "machine/mc88200.h"
#include "machine/timekpr.h"

#include "bus/vme/vme.h"
#include "bus/rs232/rs232.h"

class vme_mvme187_card_device
	: public device_t
	, public device_vme_card_interface
{
public:
	vme_mvme187_card_device(machine_config const &mconfig, char const *tag, device_t *owner, u32 clock);

protected:
	virtual const tiny_rom_entry *device_rom_region() const override ATTR_COLD;
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;
	virtual ioport_constructor device_input_ports() const override ATTR_COLD;
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

private:
	void cpu_mem(address_map &map) ATTR_COLD;

	required_device<mc88100_device> m_cpu;
	required_device_array<mc88200_device, 2> m_mmu;

	required_device<ds1643_device> m_rtc;
	//required_device_array<cd2401, 2> m_uart;
	required_device_array<rs232_port_device, 6> m_serial;
	//required_device<ncr53c7xx_device> m_scsi;
	required_device<i82596_device> m_lan;

	memory_view m_boot;
};

DECLARE_DEVICE_TYPE(VME_MVME187, vme_mvme187_card_device)

#endif // MAME_BUS_VME_MVME187_H
