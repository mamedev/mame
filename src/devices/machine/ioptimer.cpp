// license:BSD-3-Clause
// copyright-holders:Ryan Holtz
/******************************************************************************
*
*   Sony PlayStation 2 IOP timer device skeleton
*
*   To Do:
*     Everything
*
*/

#include "emu.h"
#include "ioptimer.h"

DEFINE_DEVICE_TYPE(SONYIOP_TIMER, iop_timer_device, "ioptimer", "PlayStation 2 IOP timer")

iop_timer_device::iop_timer_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, SONYIOP_TIMER, tag, owner, clock)
	, m_compare_timer(nullptr)
	, m_overflow_timer(nullptr)
	, m_last_update_time(attotime::zero)
	, m_elapsed_time(attotime::zero)
	, m_zero_return(false)
	, m_count(0)
	, m_compare(0)
	, m_gate_enable(false)
	, m_gate_mode(GATM_LOW)
	, m_cmp_int_enabled(false)
	, m_cmp_int(false)
	, m_ovf_int_enabled(false)
	, m_ovf_int(false)
	, m_ienable(false)
	, m_int_cb(*this)
{
}

void iop_timer_device::device_start()
{
	m_int_cb.resolve_safe();

	if (!m_compare_timer)
		m_compare_timer = machine().scheduler().timer_alloc(timer_expired_delegate(FUNC(iop_timer_device::compare), this));

	if (!m_overflow_timer)
		m_overflow_timer = machine().scheduler().timer_alloc(timer_expired_delegate(FUNC(iop_timer_device::overflow), this));
}

void iop_timer_device::device_reset()
{
	m_ctrl = 0;

	m_last_update_time = attotime::zero;
	m_elapsed_time = attotime::zero;

	m_zero_return = false;

	m_count = 0;
	m_compare = 0;

	m_gate_enable = false;
	m_gate_mode = GATM_LOW;

	m_cmp_int_enabled = false;
	m_cmp_int = false;

	m_ovf_int_enabled = false;
	m_ovf_int = false;

	m_ienable = false;

	m_compare_timer->adjust(attotime::never);
	m_overflow_timer->adjust(attotime::never);
}

void iop_timer_device::update_gate()
{
	// TODO
}

void iop_timer_device::update_interrupts()
{
	if (!m_int_cb.isnull())
	{
		bool interrupt = m_ienable && ((m_ovf_int && m_ovf_int_enabled) || (m_cmp_int && m_cmp_int_enabled));
		m_int_cb(interrupt);
	}
}

void iop_timer_device::update_compare_timer()
{
	// TODO: Compare timer functionality is a total guess
	if (!m_ienable || !m_cmp_int_enabled)
		m_compare_timer->adjust(attotime::never);
	m_compare_timer->adjust(attotime::from_ticks(m_compare - m_count, clock()));
}

void iop_timer_device::update_overflow_timer()
{
	// TODO: Overflow timer functionality is not used yet, total guess
	/*
	uint32_t ticks = 0;
	if (m_zero_return)
	{
	    if (m_compare > m_count)
	        ticks = m_compare - m_count;
	    else
	        ticks = (uint32_t)(0x100000000ULL - (m_count - m_compare));
	}
	else
	{
	    ticks = (uint32_t)(0x100000000ULL - m_count);
	}
	*/
	m_overflow_timer->adjust(attotime::never);
}

void iop_timer_device::update_count()
{
	attotime curr_time = machine().scheduler().time();
	attotime time_delta = curr_time - m_last_update_time;
	m_last_update_time = curr_time;
	m_elapsed_time += time_delta;
	uint32_t ticks = (uint32_t)m_elapsed_time.as_ticks(clock());
	if (ticks > 0)
	{
		m_elapsed_time -= attotime::from_ticks(ticks, clock());
		m_count += ticks;
	}
}

void iop_timer_device::set_ctrl(uint32_t data)
{
	static char const *const gatm_names[4] = { "low?", "reset+rising?", "reset+falling?", "reset+both?" };
	logerror("%s: set_ctrl: GATE=%d, GATM=%s, ZRET=%d\n", machine().describe_context(), data & CTRL_GATE, gatm_names[(data & CTRL_GATM) >> 1], (data & CTRL_ZRET) >> 3);
	logerror("%s:           CMPE=%d, OVFE=%d, REP_INT=%d, TOG_INT=%d\n", machine().describe_context(), (data & CTRL_CMPE) >> 4, (data & CTRL_OVFE) >> 5, (data & CTRL_REPI) >> 6, (data & CTRL_TOGI) >> 7);

	const uint32_t old = m_ctrl;
	m_ctrl = data | CTRL_INTE;
	const uint32_t changed = old ^ data;

	if (!changed) return;

	m_gate_enable = data & CTRL_GATE;
	m_gate_mode = (timer_gate_mode)((data & CTRL_GATM) >> 1);
	m_zero_return = (data & CTRL_ZRET) >> 3;
	m_cmp_int_enabled = (data & CTRL_CMPE) >> 4;
	m_ovf_int_enabled = (data & CTRL_OVFE) >> 5;
	m_repeat_int = (data & CTRL_REPI) >> 6;
	m_toggle_int = (data & CTRL_TOGI) >> 7;
	m_ienable = 1;

	m_count = 0;
	if (changed & CTRL_CMPE)
	{
		if (data & CTRL_CMPE)
			update_compare_timer();
		else
			m_compare_timer->adjust(attotime::never);
	}

	if (changed & CTRL_OVFE)
	{
		if (data & CTRL_OVFE)
			update_overflow_timer();
		else
			m_overflow_timer->adjust(attotime::never);
	}

	if (changed & CTRL_INTE)
	{
		update_interrupts();
	}
}

TIMER_CALLBACK_MEMBER(iop_timer_device::compare)
{
	if (m_zero_return)
	{
		m_count = 0; // Guess
		update_compare_timer();
	}
	else
	{
		m_compare_timer->adjust(attotime::never);
	}

	if (m_cmp_int_enabled)
		m_cmp_int = 1;

	update_interrupts();
}

TIMER_CALLBACK_MEMBER(iop_timer_device::overflow)
{
	if (m_ovf_int_enabled)
		m_ovf_int = 1;

	update_interrupts();
}

READ32_MEMBER(iop_timer_device::read)
{
	uint32_t ret = 0;
	switch (offset)
	{
		case 0x00:
		{
			//const uint32_t old = m_count;
			update_count();
			ret = m_count;
			//if (old != m_count)
				//logerror("%s: IOP timer read: COUNT (%08x & %08x)\n", machine().describe_context(), ret, mem_mask);
			break;
		}

		case 0x01:
			ret = m_ctrl | (m_ovf_int ? CTRL_OVFF : 0) | (m_cmp_int ? CTRL_CMPF : 0);
			logerror("%s: IOP timer read: MODE (%08x & %08x)\n", machine().describe_context(), ret, mem_mask);
			break;

		case 0x02:
			ret = m_compare;
			logerror("%s: IOP timer read: COMPARE (%08x & %08x)\n", machine().describe_context(), ret, mem_mask);
			break;

		default:
			break;
	}
	return ret;
}

WRITE32_MEMBER(iop_timer_device::write)
{
	switch (offset)
	{
		case 0x00:
			m_count = data;
			logerror("%s: IOP timer write: COUNT = %08x & %08x\n", machine().describe_context(), data, mem_mask);
			update_compare_timer();
			update_overflow_timer();
			break;

		case 0x01:
			set_ctrl(data);
			break;

		case 0x02:
		{
			logerror("%s: IOP timer write: COMPARE = %08x & %08x\n", machine().describe_context(), data, mem_mask);
			if (m_compare == data)
				break;

			m_compare = data;
			if (!m_toggle_int)
				m_ienable = true;
			update_compare_timer();
			break;
		}

		default:
			break;
	}
}
