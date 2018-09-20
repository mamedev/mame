// license:BSD-3-Clause
// copyright-holders:hap
/*

  TMS1000 family - TMS0270

*/

#ifndef MAME_CPU_TMS1000_TMS0270_H
#define MAME_CPU_TMS1000_TMS0270_H

#pragma once

#include "tms0980.h"


// TMS0270 was designed to interface with TMS5100, set it up at driver level
#define MCFG_TMS0270_READ_CTL_CB(_devcb) \
	downcast<tms0270_cpu_device &>(*device).set_read_ctl_callback(DEVCB_##_devcb);

#define MCFG_TMS0270_WRITE_CTL_CB(_devcb) \
	downcast<tms0270_cpu_device &>(*device).set_write_ctl_callback(DEVCB_##_devcb);

#define MCFG_TMS0270_WRITE_PDC_CB(_devcb) \
	downcast<tms0270_cpu_device &>(*device).set_write_pdc_callback(DEVCB_##_devcb);


class tms0270_cpu_device : public tms0980_cpu_device
{
public:
	tms0270_cpu_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);

	// configuration helpers
	template <class Object> devcb_base &set_read_ctl_callback(Object &&cb) { return m_read_ctl.set_callback(std::forward<Object>(cb)); }
	template <class Object> devcb_base &set_write_ctl_callback(Object &&cb) { return m_write_ctl.set_callback(std::forward<Object>(cb)); }
	template <class Object> devcb_base &set_write_pdc_callback(Object &&cb) { return m_write_pdc.set_callback(std::forward<Object>(cb)); }

protected:
	// overrides
	virtual void device_start() override;
	virtual void device_reset() override;

	virtual void device_add_mconfig(machine_config &config) override;

	virtual void write_o_output(u8 index) override { tms1k_base_device::write_o_output(index); }
	virtual u8 read_k_input() override;
	virtual void dynamic_output() override;

	virtual void op_setr() override;
	virtual void op_rstr() override;
	virtual void op_tdo() override;

private:
	// state specific to interface with TMS5100
	u16  m_r_prev;
	u8   m_chipsel;
	u8   m_ctl_out;
	u8   m_ctl_dir;
	int  m_pdc;

	u8   m_o_latch_low;
	u8   m_o_latch;
	u8   m_o_latch_prev;

	devcb_read8 m_read_ctl;
	devcb_write8 m_write_ctl;
	devcb_write_line m_write_pdc;
};


DECLARE_DEVICE_TYPE(TMS0270, tms0270_cpu_device)

#endif // MAME_CPU_TMS1000_TMS0270_H
