// license:BSD-3-Clause
// copyright-holders:Olivier Galibert

// Emulation of the MKS3 keyboard scanning PCB used in the psr340 and
// psr540 among others.

#ifndef MAME_YAMAHA_MKS3_H
#define MAME_YAMAHA_MKS3_H

#pragma once

#include "cpu/m6805/m6805.h"

DECLARE_DEVICE_TYPE(MKS3, mks3_device)

class mks3_device : public device_t
{
public:
	mks3_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock = 0);

	void ic_w(int state);
	void req_w(int state);
	auto write_da() { return m_write_da.bind(); }
	auto write_clk() { return m_write_clk.bind(); }

protected:
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;
	virtual ioport_constructor device_input_ports() const override ATTR_COLD;
	virtual const tiny_rom_entry *device_rom_region() const override ATTR_COLD;
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;

private:
	required_device<hd6305v0_device> m_cpu;
	required_ioport_array<11> m_port_a, m_port_b;
	devcb_write_line m_write_da;
	devcb_write_line m_write_clk;

	u8 m_port_c, m_port_d;
	int m_ic, m_req;

	void da_w(int state);
	void clk_w(int state);

	u8 pa_r();
	u8 pb_r();
	void pc_w(u8 data);
	void pd_w(u8 data);
};

#endif
