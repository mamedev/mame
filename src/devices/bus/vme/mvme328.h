// license:BSD-3-Clause
// copyright-holders:Patrick Mackinlay

#ifndef MAME_BUS_VME_MVME328_H
#define MAME_BUS_VME_MVME328_H

#pragma once

#include "cpu/m68000/m68000.h"
#include "machine/mb87030.h"

#include "bus/vme/vme.h"

class vme_mvme328_device
	: public device_t
	, public device_vme_card_interface
{
public:
	vme_mvme328_device(machine_config const &mconfig, char const *tag, device_t *owner, u32 clock);

protected:
	virtual const tiny_rom_entry *device_rom_region() const override ATTR_COLD;
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;
	virtual ioport_constructor device_input_ports() const override ATTR_COLD;
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

private:
	void cpu_mem(address_map &map) ATTR_COLD;
	void cpu_int(address_map &map) ATTR_COLD;

	void timer(int param);

	u16 sts_r();
	u16 ctl_r();
	void ctl_w(u16 data);

	required_device<m68000_device> m_cpu;
	required_device_array<mb87030_device, 2> m_scsi;

	required_shared_ptr<u16> m_bram;

	required_ioport m_sw1;
	output_finder<> m_led1;
	output_finder<4> m_led2;

	emu_timer *m_timer;
	memory_view m_boot;

	u16 m_sts;
	u16 m_ctl;
};

DECLARE_DEVICE_TYPE(VME_MVME328, vme_mvme328_device)

#endif // MAME_BUS_VME_MVME328_H
