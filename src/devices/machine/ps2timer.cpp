// license:BSD-3-Clause
// copyright-holders:Ryan Holtz
/******************************************************************************
*
*   Sony PlayStation 2 EE timer device skeleton
*
*   To Do:
*     Everything
*
*/

#include "emu.h"
#include "ps2timer.h"

DEFINE_DEVICE_TYPE(SONYPS2_TIMER, ps2_timer_device, "ps2timer", "PlayStation 2 EE Core Timer")

ps2_timer_device::ps2_timer_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, SONYPS2_TIMER, tag, owner, clock)
	, m_compare_timer(nullptr)
	, m_overflow_timer(nullptr)
	, m_mode(0)
	, m_last_enable_time(attotime::zero)
	, m_last_update_time(attotime::zero)
	, m_elapsed_time(attotime::zero)
	, m_enabled(false)
	, m_zero_return(false)
	, m_count(0)
	, m_compare(0)
	, m_can_hold(false)
	, m_hold(0)
	, m_clk_select(CLKS_BUSCLK1)
	, m_gate_enable(false)
	, m_gate_select(GATS_HBLNK)
	, m_gate_mode(GATM_LOW)
	, m_cmp_int_enabled(false)
	, m_cmp_int(false)
	, m_ovf_int_enabled(false)
	, m_ovf_int(false)
{
}

void ps2_timer_device::device_start()
{
	if (!m_compare_timer)
		m_compare_timer = machine().scheduler().timer_alloc(timer_expired_delegate(FUNC(ps2_timer_device::compare), this));

	if (!m_overflow_timer)
		m_overflow_timer = machine().scheduler().timer_alloc(timer_expired_delegate(FUNC(ps2_timer_device::overflow), this));
}

void ps2_timer_device::device_reset()
{
	m_mode = 0;

	m_last_enable_time = attotime::zero;
	m_last_update_time = attotime::zero;
	m_elapsed_time = attotime::zero;

	m_enabled = false;
	m_zero_return = false;

	m_count = 0;
	m_compare = 0;

	m_can_hold = false;
	m_hold = 0;

	m_clk_select = CLKS_BUSCLK1;

	m_gate_enable = false;
	m_gate_select = GATS_HBLNK;
	m_gate_mode = GATM_LOW;

	m_cmp_int_enabled = false;
	m_cmp_int = false;

	m_ovf_int_enabled = false;
	m_ovf_int = false;

	m_compare_timer->adjust(attotime::never);
	m_overflow_timer->adjust(attotime::never);
}

void ps2_timer_device::update_gate()
{
	// TODO
}

void ps2_timer_device::update_interrupts()
{
	// TODO
}

void ps2_timer_device::update_compare_timer()
{
	// TODO
}

void ps2_timer_device::update_overflow_timer()
{
	// TODO
}

void ps2_timer_device::update_count()
{
	if (!m_enabled) return;
	uint32_t frequency = 0;

	switch (m_clk_select)
	{
		case CLKS_BUSCLK1:
			frequency = clock();
			break;
		case CLKS_BUSCLK16:
			frequency = clock() / 16;
			break;
		case CLKS_BUSCLK256:
			frequency = clock() / 256;
			break;
		case CLKS_HBLNK:
			frequency = clock() / 18700;
			break;
	}

	attotime curr_time = machine().scheduler().time();
	attotime time_delta = curr_time - m_last_update_time;
	m_last_update_time = curr_time;
	m_elapsed_time += time_delta;
	uint32_t ticks = (uint32_t)m_elapsed_time.as_ticks(frequency);
	if (ticks > 0)
	{
		m_elapsed_time -= attotime::from_ticks(ticks, frequency);
		m_count += ticks;
		m_count &= 0xffff;
	}
}

void ps2_timer_device::update_hold()
{
	// TODO
}

void ps2_timer_device::set_mode(uint32_t data)
{
	static char const *const clks_names[4] = { "BUSCLK", "BUSCLK/16", "BUSCLK/256", "HBLNK" };
	static char const *const gatm_names[4] = { "low", "reset+rising", "reset+falling", "reset+both" };
	logerror("%s:          CLKS=%s, GATE=%d, GATS=%cBLNK\n", machine().describe_context(), clks_names[data & 3], BIT(data, 2), BIT(data, 3) ? 'V' : 'H');
	logerror("%s:          GATM=%s, ZRET=%d, CUE=%d\n", machine().describe_context(), gatm_names[(data >> 4) & 3], BIT(data, 6), BIT(data, 7));
	logerror("%s:          CMPE=%d, OVFE=%d, %s, %s\n", machine().describe_context(), BIT(data, 8), BIT(data, 9), BIT(data, 10) ? "Clear Equal" : "", BIT(data, 11) ? "Clear Overflow" : "");

	if (!(data & (MODE_EQUF | MODE_OVFF)) && data == m_mode) return;

	const uint32_t old = m_mode;
	m_mode = data;
	const uint32_t changed = old ^ data;

	m_clk_select = (timer_clock_select)(data & 3);
	m_gate_enable = BIT(data, 2);
	m_gate_select = BIT(data, 3) ? GATS_VBLNK : GATS_HBLNK;
	m_gate_mode = (timer_gate_mode)((data >> 4) & 3);
	m_zero_return = BIT(data, 6);
	m_enabled = BIT(data, 7);

	if (changed & (MODE_GATE | MODE_GATS | MODE_GATM))
	{
		update_gate();
	}

	if (changed & (MODE_CUE | MODE_CLKS | MODE_ZRET))
	{
		update_compare_timer();
		update_overflow_timer();
	}

	if (changed & (MODE_CMPE | MODE_OVFE | MODE_EQUF | MODE_OVFF))
	{
		m_cmp_int_enabled = BIT(data, 8);
		m_ovf_int_enabled = BIT(data, 9);
		m_cmp_int = BIT(data, 10) ? 0 : m_cmp_int;
		m_ovf_int = BIT(data, 11) ? 0 : m_ovf_int;
		update_interrupts();
	}
}

TIMER_CALLBACK_MEMBER(ps2_timer_device::compare)
{
}

TIMER_CALLBACK_MEMBER(ps2_timer_device::overflow)
{
}

READ32_MEMBER(ps2_timer_device::read)
{
	uint32_t ret = 0;
	switch (offset)
	{
		case 0x00:
		{
			const uint32_t old = m_count;
			update_count();
			ret = m_count;
			if (old != m_count)
				logerror("%s: PS2 timer read: COUNT (%08x & %08x)\n", machine().describe_context(), ret, mem_mask);
			break;
		}

		case 0x02:
			ret = m_mode;
			logerror("%s: PS2 timer read: MODE (%08x & %08x)\n", machine().describe_context(), ret, mem_mask);
			break;

		case 0x04:
			ret = m_compare;
			logerror("%s: PS2 timer read: COMP (%08x & %08x)\n", machine().describe_context(), ret, mem_mask);
			break;

		case 0x06:
			ret = m_hold;
			logerror("%s: PS2 timer read: HOLD (%08x & %08x)\n", machine().describe_context(), ret, mem_mask);
			break;

		default:
			break;
	}
	return ret;
}

WRITE32_MEMBER(ps2_timer_device::write)
{
	switch (offset)
	{
		case 0x00:
			m_count = data;
			logerror("%s: PS2 timer write: COUNT = %08x & %08x\n", machine().describe_context(), data, mem_mask);
			update_compare_timer();
			update_overflow_timer();
			break;

		case 0x02:
			set_mode(data);
			break;

		case 0x04:
		{
			logerror("%s: PS2 timer write: COMP = %08x & %08x\n", machine().describe_context(), data, mem_mask);
			if (m_compare == data)
				break;

			m_compare = data;
			update_compare_timer();
			break;
		}

		case 0x06:
		{
			if (!m_can_hold)
				break;

			logerror("%s: PS2 timer write: HOLD = %08x & %08x\n", machine().describe_context(), data, mem_mask);
			if (m_hold == data)
				break;

			m_hold = data;
			update_hold();
			break;
		}

		default:
			break;
	}
}
