// license:BSD-3-Clause
// copyright-holders:AJR

#ifndef MAME_CPU_M68HC16_M68HC16Z_H
#define MAME_CPU_M68HC16_M68HC16Z_H

#pragma once

#include "cpu16.h"


class m68hc16z_device : public cpu16_device
{
protected:
	m68hc16z_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, u32 clock, address_map_constructor map);

	// device-level overrides
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

	void sim_map(address_map &map) ATTR_COLD;
	void sram_map(address_map &map) ATTR_COLD;
	void adc_map(address_map &map) ATTR_COLD;
	void qsm_map(address_map &map) ATTR_COLD;
	void gpt_map(address_map &map) ATTR_COLD;

private:
	void simcr_w(offs_t offset, u16 data, u16 mem_mask = 0xffff);
	void syncr_w(offs_t offset, u16 data, u16 mem_mask = 0xffff);
	u8 rsr_r();
	void porte_w(u8 data);
	void ddre_w(u8 data);
	void pepar_w(u8 data);
	void portf_w(u8 data);
	void ddrf_w(u8 data);
	void pfpar_w(u8 data);
	void sypcr_w(u8 data);
	void picr_w(offs_t offset, u16 data, u16 mem_mask = 0xffff);
	u16 pitr_r();
	void pitr_w(offs_t offset, u16 data, u16 mem_mask = 0xffff);
	void swsr_w(u8 data);
	void cspar_w(offs_t offset, u16 data, u16 mem_mask = 0xffff);
	void csbar_csor_w(offs_t offset, u16 data, u16 mem_mask = 0xffff);

	void rammcr_w(offs_t offset, u16 data, u16 mem_mask = 0xffff);
	void ramba_w(offs_t offset, u16 data, u16 mem_mask = 0xffff);

	void adcmcr_w(offs_t offset, u16 data, u16 mem_mask = 0xffff);
	void adctl0_w(u8 data);
	void adctl1_w(u8 data);
	u16 adcstat_r();
	u16 rjurr_r(offs_t offset);
	u16 ljsrr_r(offs_t offset);
	u16 ljurr_r(offs_t offset);

	void qsmcr_w(offs_t offset, u16 data, u16 mem_mask = 0xffff);
	void qsilr_w(u8 data);
	void qsivr_w(u8 data);
	u16 sccr_r(offs_t offset);
	void sccr_w(offs_t offset, u16 data, u16 mem_mask = 0xffff);
	u16 scsr_r();
	void scdr_w(u16 data);
	void portqs_w(u8 data);
	void pqspar_w(u8 data);
	void ddrqs_w(u8 data);
	u16 spcr_r(offs_t offset);
	void spcr_w(offs_t offset, u16 data, u16 mem_mask = 0xffff);

	void gptmcr_w(offs_t offset, u16 data, u16 mem_mask = 0xffff);
	void gpticr_w(offs_t offset, u16 data, u16 mem_mask = 0xffff);
	void ddrgp_w(u8 data);
	void portgp_w(u8 data);
	void oc1m_w(u8 data);
	u16 tcnt_r();
	void pactl_w(u8 data);
	void toc_w(offs_t offset, u16 data, u16 mem_mask);
	void tctl1_w(u8 data);
	void tctl2_w(u8 data);
	u8 tmsk1_r();
	void tmsk1_w(u8 data);
	void tmsk2_w(u8 data);
	void tflg1_w(u8 data);
	void tflg2_w(u8 data);

	u16 m_pitr;
	u16 m_sccr[4];
	u16 m_spcr[4];
	u8 m_tmsk1;
};

class mc68hc16z1_device : public m68hc16z_device
{
public:
	// device type constructor
	mc68hc16z1_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);

private:
	void internal_map(address_map &map) ATTR_COLD;
};

// device type declaration
DECLARE_DEVICE_TYPE(MC68HC16Z1, mc68hc16z1_device)


#endif // MAME_CPU_M68HC16_M68HC16Z_H
