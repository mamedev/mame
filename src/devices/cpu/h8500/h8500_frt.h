// license:BSD-3-Clause
// copyright-holders:R. Belmont
/***************************************************************************

    h8500_frt.h

    H8/500 family 16-bit free-running timer (FRT).

	Based on Olivier Galibert's H8/300 family timer implementation,
	particularly H8/325's which is almost identical to the H8/500 version.

***************************************************************************/

#ifndef MAME_CPU_H8500_H8500_FRT_H
#define MAME_CPU_H8500_H8500_FRT_H

#pragma once

#include "cpu/h8/h8_timer16.h"

class h8500_frt_device : public h8_timer16_channel_device
{
public:
	h8500_frt_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock = 0);

	template <typename T, typename U>
	h8500_frt_device(const machine_config &mconfig, const char *tag, device_t *owner, T &&cpu, U &&intc, int irq_base)
		: h8500_frt_device(mconfig, tag, owner)
	{
		m_cpu.set_tag(std::forward<T>(cpu));
		m_intc.set_tag(std::forward<U>(intc));
		m_tgr_count = 2;               // OCRA and OCRB; ICR is capture-only

		m_interrupt[0] = irq_base + 1; // OCIA
		m_interrupt[1] = irq_base + 2; // OCIB
		m_interrupt[2] = irq_base;     // ICI
		m_interrupt[3] = -1;
		m_interrupt[4] = irq_base + 3; // FOVI
		m_interrupt[5] = -1;
	}

	u16  ocra_r() { return tgr_r(0); }
	void ocra_w(offs_t offset, u16 data, u16 mem_mask = ~0) { tgr_w(0, data, mem_mask); }
	u16  ocrb_r() { return tgr_r(1); }
	void ocrb_w(offs_t offset, u16 data, u16 mem_mask = ~0) { tgr_w(1, data, mem_mask); }
	u16  icr_r() { return tgr_r(2); }

	// byte accessors for parts with an 8-bit internal bus, modeling the
	// TEMP register semantics of section 10.3 (a high-byte access latches;
	// the OCR registers are read directly)
	u8   frc8_r(offs_t offset);
	void frc8_w(offs_t offset, u8 data);
	u8   ocra8_r(offs_t offset) { return offset ? (ocra_r() & 0xff) : (ocra_r() >> 8); }
	void ocra8_w(offs_t offset, u8 data);
	u8   ocrb8_r(offs_t offset) { return offset ? (ocrb_r() & 0xff) : (ocrb_r() >> 8); }
	void ocrb8_w(offs_t offset, u8 data);
	u8   icr8_r(offs_t offset);

protected:
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

	virtual void tcr_update() override;
	virtual void isr_update(u8 value) override;
	virtual u8 isr_to_sr() const override;

private:
	u8 m_tcsr;
	u8 m_temp;
};

DECLARE_DEVICE_TYPE(H8500_FRT, h8500_frt_device)

#endif // MAME_CPU_H8500_H8500_FRT_H
