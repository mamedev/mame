// license:BSD-3-Clause
// copyright-holders:Olivier Galibert
/***************************************************************************

    h8_timer8.h

    H8 8 bits timer

***************************************************************************/

#ifndef MAME_CPU_H8_H8_TIMER8_H
#define MAME_CPU_H8_H8_TIMER8_H

#pragma once

#include "h8.h"
#include "h8_intc.h"

class h8_timer8_channel_device : public device_t {
public:
	enum {
		STOPPED,
		CHAIN_A,
		CHAIN_OVERFLOW,
		INPUT_UP,
		INPUT_DOWN,
		INPUT_UPDOWN,
		DIV
	};

	h8_timer8_channel_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);
	template<typename T, typename U> h8_timer8_channel_device(const machine_config &mconfig, const char *tag, device_t *owner, T &&cpu, U &&intc, int irq_ca, int irq_cb, int irq_v,
				int div1, int div2, int div3, int div4, int div5, int div6)
		: h8_timer8_channel_device(mconfig, tag, owner, 0)
	{
		m_cpu.set_tag(std::forward<T>(cpu));
		m_intc.set_tag(std::forward<U>(intc));
		m_irq_ca = irq_ca;
		m_irq_cb = irq_cb;
		m_irq_v = irq_v;
		m_div_tab[0] = div1;
		m_div_tab[1] = div2;
		m_div_tab[2] = div3;
		m_div_tab[3] = div4;
		m_div_tab[4] = div5;
		m_div_tab[5] = div6;
	}

	u8 tcr_r();
	void tcr_w(u8 data);
	u8 tcsr_r();
	void tcsr_w(u8 data);
	u8 tcor_r(offs_t offset);
	void tcor_w(offs_t offset, u8 data);
	u8 tcnt_r();
	void tcnt_w(u8 data);

	u64 internal_update(u64 current_time);
	void notify_standby(int state);
	void set_extra_clock_bit(bool bit);

	void chained_timer_overflow();
	void chained_timer_tcora();

protected:
	enum {
		TCR_CKS   = 0x07,
		TCR_CCLR  = 0x18,
		TCR_OVIE  = 0x20,
		TCR_CMIEA = 0x40,
		TCR_CMIEB = 0x80,

		TCSR_OS   = 0x0f,
		TCSR_ADTE = 0x10,
		TCSR_OVF  = 0x20,
		TCSR_CMFA = 0x40,
		TCSR_CMFB = 0x80
	};

	enum {
		CLEAR_NONE,
		CLEAR_A,
		CLEAR_B,
		CLEAR_EXTERNAL
	};

	required_device<h8_device> m_cpu;
	required_device<h8_intc_device> m_intc;
	optional_device<h8_timer8_channel_device> m_chained_timer;

	int m_irq_ca, m_irq_cb, m_irq_v, m_chain_type;
	int m_div_tab[6];
	u8 m_tcor[2];
	u8 m_tcr, m_tcsr, m_tcnt;
	bool m_extra_clock_bit, m_has_adte, m_has_ice;
	int m_clock_type, m_clock_divider, m_clear_type, m_counter_cycle;
	u64 m_last_clock_update, m_event_time;

	h8_timer8_channel_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, u32 clock);

	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

	void update_counter(u64 cur_time = 0, u64 delta = 0);
	void recalc_event(u64 cur_time = 0);

	void update_tcr();
};

class h8h_timer8_channel_device : public h8_timer8_channel_device {
public:
	h8h_timer8_channel_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);
	template<typename T, typename U, typename V> h8h_timer8_channel_device(const machine_config &mconfig, const char *tag, device_t *owner, T &&cpu, U &&intc, int irq_ca, int irq_cb, int irq_v,
				V &&chain, int chain_type, bool has_adte, bool has_ice)
		: h8h_timer8_channel_device(mconfig, tag, owner, 0)
	{
		m_cpu.set_tag(std::forward<T>(cpu));
		m_intc.set_tag(std::forward<U>(intc));
		m_irq_ca = irq_ca;
		m_irq_cb = irq_cb;
		m_irq_v = irq_v;
		m_chained_timer.set_tag(std::forward<V>(chain));
		m_chain_type = chain_type;
		m_has_adte = has_adte;
		m_has_ice = has_ice;
	}
};

DECLARE_DEVICE_TYPE(H8_TIMER8_CHANNEL,  h8_timer8_channel_device)
DECLARE_DEVICE_TYPE(H8H_TIMER8_CHANNEL, h8h_timer8_channel_device)

#endif // MAME_CPU_H8_H8_TIMER8_H
