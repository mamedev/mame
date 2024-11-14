// license:BSD-3-Clause
// copyright-holders:Olivier Galibert
/***************************************************************************

    h8_watchdog.h

    H8 watchdog/timer

***************************************************************************/

#ifndef MAME_CPU_H8_H8_WATCHDOG_H
#define MAME_CPU_H8_H8_WATCHDOG_H

#pragma once

#include "h8.h"
#include "h8_intc.h"

class h8_watchdog_device : public device_t {
public:
	enum { B, H, S };

	h8_watchdog_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);
	template<typename T, typename U> h8_watchdog_device(const machine_config &mconfig, const char *tag, device_t *owner, T &&cpu, U &&intc, int irq, int type)
		: h8_watchdog_device(mconfig, tag, owner, 0)
	{
		m_cpu.set_tag(std::forward<T>(cpu));
		m_intc.set_tag(std::forward<U>(intc));
		m_irq = irq;
		m_type = type;
	}

	u64 internal_update(u64 current_time);
	void notify_standby(int state);

	u16 wd_r();
	void wd_w(offs_t offset, u16 data, u16 mem_mask = ~0);
	u16 rst_r();
	void rst_w(offs_t offset, u16 data, u16 mem_mask = ~0);

protected:
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

private:
	enum {
		TCSR_CKS  = 0x07,
		TCSR_NMI  = 0x08,
		TCSR_TME  = 0x20,
		TCSR_WT   = 0x40,
		TCSR_OVF  = 0x80,

		RST_RSTS  = 0x20,
		RST_RSTE  = 0x40,
		RST_RSTEO = 0x40,
		RST_WRST  = 0x80
	};

	static const int div_bh[8];
	static const int div_s[8];

	required_device<h8_device> m_cpu;
	required_device<h8_intc_device> m_intc;
	int m_irq;
	int m_type;
	u8 m_tcnt, m_tcsr, m_rst;
	u64 m_tcnt_cycle_base;

	void tcnt_update(u64 current_time = 0);
};

DECLARE_DEVICE_TYPE(H8_WATCHDOG, h8_watchdog_device)

#endif // MAME_CPU_H8_H8_WATCHDOG_H
