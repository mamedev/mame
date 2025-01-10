// license:BSD-3-Clause
// copyright-holders:Patrick Mackinlay

#ifndef MAME_BUS_VME_MVME327A_H
#define MAME_BUS_VME_MVME327A_H

#pragma once

#include "cpu/m68000/m68010.h"

#include "machine/68230pit.h"
#include "machine/68153bim.h"
#include "machine/wd33c9x.h"
#include "machine/upd765.h"

#include "bus/vme/vme.h"

class vme_mvme327a_device
	: public device_t
	, public device_vme_card_interface
{
public:
	vme_mvme327a_device(machine_config const &mconfig, char const *tag, device_t *owner, u32 clock);

protected:
	virtual const tiny_rom_entry *device_rom_region() const override ATTR_COLD;
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;
	virtual ioport_constructor device_input_ports() const override ATTR_COLD;
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

private:
	void cpu_mem(address_map &map) ATTR_COLD;
	void cpu_int(address_map &map) ATTR_COLD;

	required_device<m68010_device> m_cpu;
	required_device<pit68230_device> m_pit;
	required_device<bim68153_device> m_bim;

	required_device<wd37c65c_device> m_fdc;
	required_device<wd33c93a_device> m_scsi;

	memory_view m_boot;
};

DECLARE_DEVICE_TYPE(VME_MVME327A, vme_mvme327a_device)

#endif // MAME_BUS_VME_MVME327A_H
