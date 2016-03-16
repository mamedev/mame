// license:BSD-3-Clause
// copyright-holders:hap
/*

  TMS1000 family - TMS0270

*/

#ifndef _TMS0270_H_
#define _TMS0270_H_

#include "tms0980.h"


// TMS0270 was designed to interface with TMS5100, set it up at driver level
#define MCFG_TMS0270_READ_CTL_CB(_devcb) \
	tms0270_cpu_device::set_read_ctl_callback(*device, DEVCB_##_devcb);

#define MCFG_TMS0270_WRITE_CTL_CB(_devcb) \
	tms0270_cpu_device::set_write_ctl_callback(*device, DEVCB_##_devcb);

#define MCFG_TMS0270_WRITE_PDC_CB(_devcb) \
	tms0270_cpu_device::set_write_pdc_callback(*device, DEVCB_##_devcb);


class tms0270_cpu_device : public tms0980_cpu_device
{
public:
	tms0270_cpu_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);

	// static configuration helpers
	template<class _Object> static devcb_base &set_read_ctl_callback(device_t &device, _Object object) { return downcast<tms0270_cpu_device &>(device).m_read_ctl.set_callback(object); }
	template<class _Object> static devcb_base &set_write_ctl_callback(device_t &device, _Object object) { return downcast<tms0270_cpu_device &>(device).m_write_ctl.set_callback(object); }
	template<class _Object> static devcb_base &set_write_pdc_callback(device_t &device, _Object object) { return downcast<tms0270_cpu_device &>(device).m_write_pdc.set_callback(object); }

protected:
	// overrides
	virtual void device_start() override;
	virtual void device_reset() override;

	virtual machine_config_constructor device_mconfig_additions() const override;

	virtual void write_o_output(UINT8 index) override { tms1k_base_device::write_o_output(index); }
	virtual UINT8 read_k_input() override;
	virtual void dynamic_output() override;

	virtual void op_setr() override;
	virtual void op_rstr() override;
	virtual void op_tdo() override;

private:
	// state specific to interface with TMS5100
	UINT16  m_r_prev;
	UINT8   m_chipsel;
	UINT8   m_ctl_out;
	UINT8   m_ctl_dir;
	int     m_pdc;

	UINT8   m_o_latch_low;
	UINT8   m_o_latch;
	UINT8   m_o_latch_prev;

	devcb_read8 m_read_ctl;
	devcb_write8 m_write_ctl;
	devcb_write_line m_write_pdc;
};


extern const device_type TMS0270;

#endif /* _TMS0270_H_ */
