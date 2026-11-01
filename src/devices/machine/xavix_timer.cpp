// license:BSD-3-Clause
// copyright-holders:David Haywood, MagikalUnicorn
/***************************************************************************

    XaviX general-purpose timer. Implements an 8-bit counter with a programmable
    prescaler and reload value, selectable up/down counting, latched terminal-
    count status and a separately enabled interrupt. Timer events and register
    accesses advance the same clock-based counter state.

***************************************************************************/

#include "emu.h"
#include "xavix_timer.h"

#define LOG_TIMER (1U << 1)
#define VERBOSE 0
#include "logmacro.h"

DEFINE_DEVICE_TYPE(XAVIX_TIMER, xavix_timer_device, "xavix_timer", "XaviX General Timer")

xavix_timer_device::xavix_timer_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: device_t(mconfig, XAVIX_TIMER, tag, owner, clock)
	, m_irq_cb(*this)
	, m_restart_on_run(false)
{
}

void xavix_timer_device::map(address_map &map)
{
	map(0x00, 0x00).rw(FUNC(xavix_timer_device::status_r), FUNC(xavix_timer_device::ctrl_w));
	map(0x01, 0x01).rw(FUNC(xavix_timer_device::reload_r), FUNC(xavix_timer_device::reload_w));
	map(0x02, 0x02).rw(FUNC(xavix_timer_device::prescaler_r), FUNC(xavix_timer_device::prescaler_w));
	map(0x03, 0x03).r(FUNC(xavix_timer_device::count_r));
	map(0x04, 0x04).rw(FUNC(xavix_timer_device::mode_r), FUNC(xavix_timer_device::mode_w));
}

void xavix_timer_device::device_start()
{
	m_timer = timer_alloc(FUNC(xavix_timer_device::timer_expired), this);

	save_item(NAME(m_ctrl));
	save_item(NAME(m_reload));
	save_item(NAME(m_prescaler));
	save_item(NAME(m_count));
	save_item(NAME(m_mode));
	save_item(NAME(m_prescaler_count));
	save_item(NAME(m_last_clock));
}

void xavix_timer_device::device_reset()
{
	m_ctrl = 0;
	m_reload = 0;
	m_prescaler = 0;
	m_count = 0;
	m_mode = 0;
	m_prescaler_count = 0;
	m_last_clock = current_clock();
	m_timer->enable(false);
	update_irq();
}

u32 xavix_timer_device::divisor() const
{
	return 1U << (m_prescaler + 2);
}

u32 xavix_timer_device::terminal_ticks(u8 count) const
{
	// Down-counting modes terminate at zero or one; up-counting modes
	// terminate at the preset. Include the terminal count itself.
	return u8(m_mode < 2 ? count - m_mode : m_reload - count) + 1;
}

u64 xavix_timer_device::current_clock() const
{
	// The counter runs at twice the peripheral clock. Use the same
	// attosecond quantisation as from_ticks when locating clock edges.
	u32 const frequency = clock() * 2;
	attotime const now = machine().time();
	return u64(now.seconds()) * frequency + now.attoseconds() / HZ_TO_ATTOSECONDS(frequency);
}

auto xavix_timer_device::current_counter() const -> counter_state
{
	counter_state state{m_ctrl, m_count, m_prescaler_count};
	u64 const now = current_clock();
	// A timer callback can run a few clocks ahead of the next CPU access.
	// Retain its processed counter state until CPU time catches up.
	if (now <= m_last_clock)
		return state;
	u64 clocks = now - m_last_clock;

	if (!BIT(state.control, 0))
	{
		// An empty prescaler reloads even while the counter is stopped.
		if (!state.prescaler_count)
			state.prescaler_count = divisor() - 1;
		return state;
	}

	u32 const first = state.prescaler_count + 1;
	if (clocks < first)
	{
		state.prescaler_count -= clocks;
		return state;
	}

	clocks -= first;
	u64 ticks = 1 + clocks / divisor();
	state.prescaler_count = divisor() - 1 - clocks % divisor();
	u32 const terminal = terminal_ticks(state.count);
	if (ticks >= terminal)
	{
		state.control |= 0x80;
		ticks -= terminal;
		state.count = m_mode < 2 ? m_reload : 1;
		ticks %= terminal_ticks(state.count);
	}
	state.count = m_mode < 2 ? state.count - ticks : state.count + ticks;
	return state;
}

void xavix_timer_device::synchronize()
{
	u64 const now = current_clock();
	if (now <= m_last_clock)
		return;

	u8 const old_control = m_ctrl;
	counter_state const state = current_counter();
	m_ctrl = state.control;
	m_count = state.count;
	m_prescaler_count = state.prescaler_count;
	m_last_clock = now;
	if (m_ctrl != old_control)
		update_irq();
}

void xavix_timer_device::update_irq()
{
	m_irq_cb((m_ctrl & 0xc0) == 0xc0);
}

void xavix_timer_device::schedule_timer()
{
	// Masked and already-pending periods can be advanced arithmetically
	// on the next register access, without scheduling every counter tick.
	if ((m_ctrl & 0xc1) == 0x41)
	{
		u64 const target = m_last_clock + m_prescaler_count + 1 + u64(terminal_ticks(m_count) - 1) * divisor();
		m_timer->adjust(attotime::from_ticks(target, clock() * 2) - machine().time());
	}
	else
		m_timer->enable(false);
}

u8 xavix_timer_device::status_r()
{
	if (machine().side_effects_disabled())
		return current_counter().control;
	synchronize();
	LOGMASKED(LOG_TIMER, "%s: status_r %02x\n", machine().describe_context(), m_ctrl);
	return m_ctrl;
}

void xavix_timer_device::ctrl_w(u8 data)
{
	LOGMASKED(LOG_TIMER, "%s: ctrl_w %02x\n", machine().describe_context(), data);
	synchronize();

	bool const start = !BIT(m_ctrl, 0) && BIT(data, 0);
	bool const retrigger = m_restart_on_run && m_mode == 0 && BIT(data, 0);
	bool const reload = m_mode == 1 ? BIT(data, 0) : (start || retrigger || BIT(data, 1));
	bool const reset_prescaler = m_mode == 1 ? BIT(data, 0) : (BIT(data, 1) || (start && m_mode >= 2));
	if (reload)
		m_count = m_mode < 2 ? m_reload : 1;
	if (reset_prescaler)
		m_prescaler_count = divisor() - 1;

	// Bit 1 is a reload command, not a stored mode. Pending status is
	// independent of interrupt enable and is cleared only by writing bit 7.
	m_ctrl = (BIT(data, 7) ? 0 : (m_ctrl & 0x80)) | (data & 0x41);
	update_irq();
	schedule_timer();
}

u8 xavix_timer_device::reload_r()
{
	return m_reload;
}

void xavix_timer_device::reload_w(u8 data)
{
	LOGMASKED(LOG_TIMER, "%s: reload_w %02x\n", machine().describe_context(), data);
	synchronize();
	// A new preset takes effect at the next reload, preserving this interval.
	m_reload = data;
	schedule_timer();
}

u8 xavix_timer_device::prescaler_r()
{
	return m_prescaler;
}

void xavix_timer_device::prescaler_w(u8 data)
{
	LOGMASKED(LOG_TIMER, "%s: prescaler_w %02x\n", machine().describe_context(), data);
	synchronize();
	m_prescaler = data & 0x0f;
	schedule_timer();
}

u8 xavix_timer_device::count_r()
{
	if (machine().side_effects_disabled())
		return current_counter().count;
	synchronize();
	return m_count;
}

u8 xavix_timer_device::mode_r()
{
	return m_mode;
}

void xavix_timer_device::mode_w(u8 data)
{
	synchronize();
	m_mode = data & 0x03;
	schedule_timer();
}

TIMER_CALLBACK_MEMBER(xavix_timer_device::timer_expired)
{
	synchronize();
	schedule_timer();
}
