// license:BSD-3-Clause
// copyright-holders: Devin Acker
/***************************************************************************
    Casio GT913 timers
***************************************************************************/

#ifndef MAME_CPU_H8_GT913_TIMER_H
#define MAME_CPU_H8_GT913_TIMER_H

#pragma once

#include "h8_intc.h"

class h8_cpu_base;

//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

DECLARE_DEVICE_TYPE(GT913_TIMER8,  gt913_timer8_device)
DECLARE_DEVICE_TYPE(GT913_TIMER16, gt913_timer16_device)

class gt913_timer_device : public device_t
{
public:
	u64 internal_update(u64 current_time);
	void notify_standby(int state);

	u8 tcr_r() { return m_tcr; }
	void tcr_w(u8 data);

protected:
	template<typename T, typename U> void set_info(T &&cpu, U &&intc, int vect)
	{
		m_cpu.set_tag(std::forward<T>(cpu));
		m_intc.set_tag(std::forward<U>(intc));
		m_intc_vector = vect;
	}

	gt913_timer_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, u32 clock);

	// device_t overrides
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

	void update_counter(u64 cur_time = 0);
	void recalc_event(u64 cur_time = 0);

	void update_irq();
	void update_tcr();

	u8 m_tcr;
	u16 m_tcor, m_tcnt;
	u16 m_clock_divider;

	u64 m_last_clock_update, m_event_time;

private:
	required_device<h8_cpu_base> m_cpu;
	required_device<h8_intc_base> m_intc;

	int m_intc_vector;
};

class gt913_timer8_device : public gt913_timer_device
{
public:
	gt913_timer8_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
		: gt913_timer_device(mconfig, GT913_TIMER8, tag, owner, clock)
	{
	}

	template <typename T, typename U> gt913_timer8_device(const machine_config &mconfig, const char *tag, device_t *owner, T &&cpu, U &&intc, int vect)
		: gt913_timer8_device(mconfig, tag, owner, 0)
	{
		set_info(cpu, intc, vect);
	}

	u8 tcor_r() { return m_tcor; }
	void tcor_w(u8 data);
	u8 tcnt_r();
	void tcnt_w(u8 data);
};

class gt913_timer16_device : public gt913_timer_device
{
public:
	gt913_timer16_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
		: gt913_timer_device(mconfig, GT913_TIMER16, tag, owner, clock)
	{
	}

	template <typename T, typename U> gt913_timer16_device(const machine_config &mconfig, const char *tag, device_t *owner, T &&cpu, U &&intc, int vect)
		: gt913_timer16_device(mconfig, tag, owner, 0)
	{
		set_info(cpu, intc, vect);
	}

	u16 tcor_r() { return m_tcor; }
	void tcor_w(offs_t offset, u16 data, u16 mem_mask);
	u16 tcnt_r();
	void tcnt_w(offs_t offset, u16 data, u16 mem_mask);
};

#endif // MAME_CPU_H8_GT913_TIMER_H
