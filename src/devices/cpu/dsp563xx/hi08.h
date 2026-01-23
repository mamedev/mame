// license:BSD-3-Clause
// copyright-holders:Olivier Galibert

// DSP 563xx HI08 host interface

#ifndef MAME_CPU_DSP563XX_HI08_H
#define MAME_CPU_DSP563XX_HI08_H

#pragma once

class dsp563xx_device;

class hi08_device : public device_t
{
public:
	hi08_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock = 0);

	void write(offs_t offset, u8 data);
	u8 read(offs_t offset);

	void map(address_map &map);

protected:
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

private:
	enum {
		ICR_INIT  = 0x80,
		ICR_HLEND = 0x20,
		ICR_HF1   = 0x10,
		ICR_HF0   = 0x08,
		ICR_HDRQ  = 0x04,
		ICR_TREQ  = 0x02,
		ICR_RREQ  = 0x01,

		CVR_HC    = 0x80,

		ISR_HREQ  = 0x80,
		ISR_HF3   = 0x10,
		ISR_HF2   = 0x08,
		ISR_TRDY  = 0x04,
		ISR_TXDE  = 0x02,
		ISR_RXDF  = 0x01,

		HSR_HF1   = 0x10,
		HSR_HF0   = 0x08,
		HSR_HCP   = 0x04,
		HSR_HTDE  = 0x02,
		HSR_HRDF  = 0x01,

	};

	required_device<dsp563xx_device> m_cpu;

	u32 m_htx, m_hrx, m_tx;
	u16 m_hcr, m_hpcr;
	u8 m_hbar;
	u8 m_icr, m_cvr, m_isr, m_ivr;

	void hcr_w(u32 data);
	u32 hcr_r();
	u32 hsr_r();
	void hpcr_w(u32 data);
	u32 hpcr_r();
	void hbar_w(u32 data);
	u32 hbar_r();
	void hrx_w(u32 data);
	u32 hrx_r();
	void htx_w(u32 data);
	u32 htx_r();
};

DECLARE_DEVICE_TYPE(HI08, hi08_device)

#endif
