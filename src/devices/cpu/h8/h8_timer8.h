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

	h8_timer8_channel_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);
	h8_timer8_channel_device(const machine_config &mconfig, const char *tag, device_t *owner, const char *intc, int irq_ca, int irq_cb, int irq_v,
				int div1, int div2, int div3, int div4, int div5, int div6)
		: h8_timer8_channel_device(mconfig, tag, owner, 0)
	{
		set_info(intc, irq_ca, irq_cb, irq_v, div1, div2, div3, div4, div5, div6);
	}

	void set_info(const char *intc, int irq_ca, int irq_cb, int irq_v, int div1, int div2, int div3, int div4, int div5, int div6);

	DECLARE_READ8_MEMBER(tcr_r);
	DECLARE_WRITE8_MEMBER(tcr_w);
	DECLARE_READ8_MEMBER(tcsr_r);
	DECLARE_WRITE8_MEMBER(tcsr_w);
	DECLARE_READ8_MEMBER(tcor_r);
	DECLARE_WRITE8_MEMBER(tcor_w);
	DECLARE_READ8_MEMBER(tcnt_r);
	DECLARE_WRITE8_MEMBER(tcnt_w);

	uint64_t internal_update(uint64_t current_time);
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

	required_device<h8_device> cpu;
	h8_timer8_channel_device *chained_timer;
	h8_intc_device *intc;
	const char *chain_tag, *intc_tag;
	int irq_ca, irq_cb, irq_v, chain_type;
	int div_tab[6];
	uint8_t tcor[2];
	uint8_t tcr, tcsr, tcnt;
	bool extra_clock_bit, has_adte, has_ice;
	int clock_type, clock_divider, clear_type, counter_cycle;
	uint64_t last_clock_update, event_time;

	h8_timer8_channel_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock);

	virtual void device_start() override;
	virtual void device_reset() override;

	void update_counter(uint64_t cur_time = 0);
	void recalc_event(uint64_t cur_time = 0);

	void timer_tick();
	void update_tcr();
};

class h8h_timer8_channel_device : public h8_timer8_channel_device {
public:
	h8h_timer8_channel_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);
	h8h_timer8_channel_device(const machine_config &mconfig, const char *tag, device_t *owner, const char *intc, int irq_ca, int irq_cb, int irq_v,
				const char *chain_tag, int chain_type, bool has_adte, bool has_ice)
		: h8h_timer8_channel_device(mconfig, tag, owner, 0)
	{
		set_info(intc, irq_ca, irq_cb, irq_v, chain_tag, chain_type, has_adte, has_ice);
	}
	virtual ~h8h_timer8_channel_device();

	void set_info(const char *intc, int irq_ca, int irq_cb, int irq_v, const char *chain_tag, int chain_type, bool has_adte, bool has_ice);
};

DECLARE_DEVICE_TYPE(H8_TIMER8_CHANNEL,  h8_timer8_channel_device)
DECLARE_DEVICE_TYPE(H8H_TIMER8_CHANNEL, h8h_timer8_channel_device)

#endif // MAME_CPU_H8_H8_TIMER8_H
