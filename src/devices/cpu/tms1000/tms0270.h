// license:BSD-3-Clause
// copyright-holders:hap
/*

  TMS1000 family - TMS0270

*/

#ifndef MAME_CPU_TMS1000_TMS0270_H
#define MAME_CPU_TMS1000_TMS0270_H

#pragma once

#include "tms0980.h"


class tms0270_cpu_device : public tms0980_cpu_device
{
public:
	tms0270_cpu_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);

protected:
	// overrides
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;

	virtual void write_o_reg(u8 index) override { tms1k_base_device::write_o_reg(index); }
	virtual u8 read_k_input() override;
	virtual void dynamic_output() override;

	virtual void op_setr() override;
	virtual void op_rstr() override;
	virtual void op_tdo() override;

private:
	// state specific to interface with TMS5100
	u32  m_r_prev;
	u8   m_chipsel;
	u8   m_ctl_out;
	u8   m_ctl_dir;
	int  m_pdc;

	u8   m_o_latch_low;
	u8   m_o_latch;
	u8   m_o_latch_prev;
};


DECLARE_DEVICE_TYPE(TMS0270, tms0270_cpu_device)

#endif // MAME_CPU_TMS1000_TMS0270_H
