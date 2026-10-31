// license:BSD-3-Clause
// copyright-holders:Chiaki Tachibana
/***************************************************************************

    Pachi-Slot Medal Selector and Hopper Payout Device (Simulator)

    Initial implementation by Chiaki Tachibana.

    Development started: 2024-12-08

***************************************************************************/

#include "emu.h"
#include "pachislotdev.h"


static const attotime MEDAL_TICK = attotime::from_usec(100);	// 100us tick


// -------------------------------------------------
//  Helper: count sensors from TIMECHART table
// -------------------------------------------------
template <size_t Max>
static u8 count_sensors_from_timechart(std::vector<pachislot_device::TIMECHART> const &v)
{
	u8 n = 0;
	const u8 limit = std::min<u8>((u8)Max, (u8)v.size());

	for (u8 i = 0; i < limit; i++)
	{
		if (v[i].tc_signal == pachislot_device::DEVSIGNAL_NONE)
			break;
		n++;
	}
	return n;
}


// device type definition
DEFINE_DEVICE_TYPE(PACHISLOT, pachislot_device, "pachislot", "Pachi-Slot Medal Simulator Device")


pachislot_device::pachislot_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, PACHISLOT, tag, owner, clock)
	, m_hopper_cb(*this)
{
	m_selector_type_s = DEVSELECTOR_NONE;
	m_hopper_type_s   = DEVHOPPER_NONE;
}

void pachislot_device::device_start()
{
	// selector timer
	m_selector_timer = timer_alloc(FUNC(pachislot_device::selector_timer_tick), this);
	m_selector_timer->adjust(attotime::never);

	// hopper timer
	m_hopper_timer = timer_alloc(FUNC(pachislot_device::hopper_timer_tick), this);
	m_hopper_timer->adjust(attotime::never);

	// hopper motor timer
	m_hopper_motor_timer = timer_alloc(FUNC(pachislot_device::hopper_motor_tick), this);
	m_hopper_motor_timer->adjust(attotime::never);

	// Persist which timing table is currently selected
	save_item(NAME(m_selector_type_s));
	save_item(NAME(m_hopper_type_s));

	// hopper motor state
	save_item(NAME(m_hopper_motor_on));
	save_item(NAME(m_hopper_period));

	// selector work
	save_item(NAME(m_selector_work.maxnum));
	save_item(NAME(m_selector_work.signal));
	save_item(NAME(m_selector_work.event_active));
	save_item(NAME(m_selector_work.event_counter));
	save_item(NAME(m_selector_work.active_events));
	save_item(NAME(m_selector_work.signal_old));
	save_item(NAME(m_selector_work.input_on));
	save_item(NAME(m_selector_work.lock));

	// hopper work
	save_item(NAME(m_hopper_work.maxnum));
	save_item(NAME(m_hopper_work.signal));
	save_item(NAME(m_hopper_work.event_active));
	save_item(NAME(m_hopper_work.event_counter));
	save_item(NAME(m_hopper_work.active_events));
	save_item(NAME(m_hopper_work.signal_old));
	save_item(NAME(m_hopper_work.input_on));

	// ----- for debug test
	m_selector_repeat_timer = timer_alloc(FUNC(pachislot_device::selector_repeat_tick), this);
	m_selector_repeat_timer->adjust(attotime::never);
	m_selector_repeat_off_timer = timer_alloc(FUNC(pachislot_device::selector_repeat_off_tick), this);
	m_selector_repeat_off_timer->adjust(attotime::never);

	save_item(NAME(m_selector_repeat_enable));
	save_item(NAME(m_selector_repeat_period));
	save_item(NAME(m_selector_repeat_pulse));
	// -----
}

void pachislot_device::device_reset()
{
	reset_selector();
	reset_hopper();
}

bool pachislot_device::has_free_medal_event(DEVICE_MEDAL_PATH const &work) const
{
	return work.active_events < MEDAL_EVENT_MAX;
}

bool pachislot_device::start_medal_event(DEVICE_MEDAL_PATH &work)
{
	if (!work.maxnum || !has_free_medal_event(work))
		return false;

	for (u8 i = 0; i < MEDAL_EVENT_MAX; i++)
	{
		if (!work.event_active[i])
		{
			work.event_active[i] = 1;
			work.event_counter[i] = 0;
			work.active_events++;
			return true;
		}
	}

	return false;
}

bool pachislot_device::medal_timer_tick_common(DEVICE_MEDAL_PATH &work)
{
	for (u8 num = 0; num < work.maxnum; num++)
		work.signal[num] = (u8)work.ptr[num].tc_signal;

	for (u8 event = 0; event < MEDAL_EVENT_MAX; event++)
	{
		if (!work.event_active[event])
			continue;

		const int c = work.event_counter[event];
		int event_end = 0;

		for (u8 num = 0; num < work.maxnum; num++)
		{
			DEVICE_SIGNAL const sensor_off = work.ptr[num].tc_signal;
			DEVICE_SIGNAL const sensor_on  = invert_signal(sensor_off);

			int counter_start = work.ptr[num].tc_start;
			int counter_len   = work.ptr[num].tc_length;

			if (counter_start < 0) counter_start = 0;
			if (counter_len   < 0) counter_len   = 0;

			const int counter_end = counter_start + counter_len;
			if (event_end < counter_end)
				event_end = counter_end;

			if (counter_len > 0 && counter_start <= c && c < counter_end)
				work.signal[num] = (u8)sensor_on;
		}

		work.event_counter[event]++;

		if (event_end <= c)
		{
			work.event_active[event] = 0;
			work.event_counter[event] = 0;
			if (work.active_events)
				work.active_events--;
		}
	}

	return work.active_events != 0;
}

TIMER_CALLBACK_MEMBER(pachislot_device::selector_timer_tick)
{
	if (!medal_timer_tick_common(m_selector_work))
		m_selector_timer->adjust(attotime::never);
}

TIMER_CALLBACK_MEMBER(pachislot_device::hopper_timer_tick)
{
	if (!medal_timer_tick_common(m_hopper_work))
		m_hopper_timer->adjust(attotime::never);
}

TIMER_CALLBACK_MEMBER(pachislot_device::hopper_motor_tick)
{
	if (!m_hopper_motor_on)
		return;

	if (!m_hopper_work.lock && start_medal_event(m_hopper_work))
	{
		m_hopper_timer->adjust(MEDAL_TICK, 0, MEDAL_TICK);
		m_hopper_cb(1);
	}
}

pachislot_device::DEVICE_SIGNAL pachislot_device::selector_r(DEVSENSOR_NUM num)
{
	if (num < m_selector_work.maxnum)
		return (DEVICE_SIGNAL)m_selector_work.signal[num];

	return DEVSIGNAL_LOW;
}

pachislot_device::DEVICE_SIGNAL pachislot_device::hopper_r(DEVSENSOR_NUM num)
{
	if (num < m_hopper_work.maxnum)
		return (DEVICE_SIGNAL)m_hopper_work.signal[num];

	return DEVSIGNAL_LOW;
}

void pachislot_device::selector_w(u8 signal)
{
	const u8 on = (signal & 0x01) ? 1 : 0;

	m_selector_work.input_on = on;

	if (!on)
	{
		m_selector_work.signal_old = 0;
		return;
	}

	// Start a new medal event on each rising edge. Existing events continue
	// independently, allowing overlapping selector waveforms when medals are
	// inserted faster than a previous sensor sequence has completed.
	if (!m_selector_work.lock && !m_selector_work.signal_old && start_medal_event(m_selector_work))
		m_selector_timer->adjust(MEDAL_TICK, 0, MEDAL_TICK);

	m_selector_work.signal_old = 1;
}

void pachislot_device::hopper_w(u8 signal)
{
	const u8 on = (signal & 0x01) ? 1 : 0;

	m_hopper_work.input_on = on;

	if (!on)
	{
		m_hopper_work.signal_old = 0;
		return;
	}

	// Start a new medal payout event on each rising edge. Existing events
	// continue independently, allowing overlapping hopper sensor waveforms.
	if (!m_hopper_work.signal_old && start_medal_event(m_hopper_work))
		m_hopper_timer->adjust(MEDAL_TICK, 0, MEDAL_TICK);

	m_hopper_work.signal_old = 1;
}

void pachislot_device::set_hopper_period(attotime period)
{
	if (period > attotime::zero)
		m_hopper_period = period;
}

void pachislot_device::hopper_motor_w(u8 state)
{
	const u8 on = (state & 0x01) ? 1 : 0;

	if (m_hopper_motor_on == on)
		return;

	m_hopper_motor_on = on;

	if (on)
		m_hopper_motor_timer->adjust(m_hopper_period, 0, m_hopper_period);
	else
		m_hopper_motor_timer->adjust(attotime::never);
}

void pachislot_device::selector_lock_w(u8 signal)
{
	m_selector_work.lock = signal;
}

void pachislot_device::set_selector_table(DEVSELECTOR_TYPE type)
{
	switch (type)
	{
		case DEVSELECTOR_SINGLE:     m_selector_work.ptr = selector_single; break;
		case DEVSELECTOR_UNIVERSAL1: m_selector_work.ptr = selector_universal1; break;
		case DEVSELECTOR_YAMASA1:    m_selector_work.ptr = selector_yamasa1; break;
		case DEVSELECTOR_YAMASA2:    m_selector_work.ptr = selector_yamasa2; break;
		case DEVSELECTOR_SAMMY1:     m_selector_work.ptr = selector_sammy1; break;
		default:                     m_selector_work.ptr = selector_none; break;
	}
}

void pachislot_device::set_hopper_table(DEVHOPPER_TYPE type)
{
	switch (type)
	{
		case DEVHOPPER_SINGLE:     m_hopper_work.ptr = hopper_single; break;
		case DEVHOPPER_UNIVERSAL1: m_hopper_work.ptr = hopper_universal1; break;
		case DEVHOPPER_UNIVERSAL2: m_hopper_work.ptr = hopper_universal2; break;
		case DEVHOPPER_ELECO1:     m_hopper_work.ptr = hopper_eleco1; break;
		case DEVHOPPER_YAMASA1:    m_hopper_work.ptr = hopper_yamasa1; break;
		default:                   m_hopper_work.ptr = hopper_none; break;
	}
}

void pachislot_device::reset_selector()
{
	m_selector_work.maxnum = count_sensors_from_timechart<SELECTOR_SENSOR_MAX>(m_selector_work.ptr);
	m_selector_work.active_events = 0;

	for (u8 num = 0; num < m_selector_work.maxnum; num++)
		m_selector_work.signal[num] = (u8)m_selector_work.ptr[num].tc_signal;

	for (u8 event = 0; event < MEDAL_EVENT_MAX; event++)
	{
		m_selector_work.event_active[event] = 0;
		m_selector_work.event_counter[event] = 0;
	}

	m_selector_work.signal_old = 0;
	m_selector_work.input_on = 0;
	m_selector_work.lock = 0;
}

void pachislot_device::reset_hopper()
{
	m_hopper_work.maxnum = count_sensors_from_timechart<HOPPER_SENSOR_MAX>(m_hopper_work.ptr);
	m_hopper_work.active_events = 0;

	for (u8 num = 0; num < m_hopper_work.maxnum; num++)
		m_hopper_work.signal[num] = (u8)m_hopper_work.ptr[num].tc_signal;

	for (u8 event = 0; event < MEDAL_EVENT_MAX; event++)
	{
		m_hopper_work.event_active[event] = 0;
		m_hopper_work.event_counter[event] = 0;
	}

	m_hopper_work.signal_old = 0;
	m_hopper_work.input_on = 0;
	m_hopper_work.lock = 0;

	m_hopper_motor_on = 0;
	if (m_hopper_motor_timer)
		m_hopper_motor_timer->adjust(attotime::never);
}

void pachislot_device::init_selector(DEVSELECTOR_TYPE type)
{
	m_selector_work.capacity = SELECTOR_SENSOR_MAX;

	m_selector_type_s = (u8)type;
	set_selector_table(type);
	reset_selector();
}

void pachislot_device::init_hopper(DEVHOPPER_TYPE type)
{
	m_hopper_work.capacity = HOPPER_SENSOR_MAX;

	m_hopper_type_s = (u8)type;
	set_hopper_table(type);
	reset_hopper();
}

void pachislot_device::device_post_load()
{
	set_selector_table((DEVSELECTOR_TYPE)m_selector_type_s);
	set_hopper_table((DEVHOPPER_TYPE)m_hopper_type_s);

	m_selector_work.maxnum = count_sensors_from_timechart<SELECTOR_SENSOR_MAX>(m_selector_work.ptr);
	m_hopper_work.maxnum   = count_sensors_from_timechart<HOPPER_SENSOR_MAX>(m_hopper_work.ptr);

	m_selector_work.signal_old = m_selector_work.input_on ? 1 : 0;
	m_hopper_work.signal_old   = m_hopper_work.input_on   ? 1 : 0;
}

pachislot_device::DEVICE_SIGNAL pachislot_device::invert_signal(pachislot_device::DEVICE_SIGNAL signal)
{
	switch (signal)
	{
		case DEVSIGNAL_LOW:
			return DEVSIGNAL_HIGH;
		case DEVSIGNAL_HIGH:
			return DEVSIGNAL_LOW;
		default:
			return DEVSIGNAL_NONE;
	}
}

// ----- for debug test
void pachislot_device::selector_repeat_period_w(attotime period)
{
	if (period <= attotime::zero)
		return;

	m_selector_repeat_period = period;

	if (m_selector_repeat_enable)
		m_selector_repeat_timer->adjust(m_selector_repeat_period);
}

void pachislot_device::selector_repeat_w(u8 sw)
{
	const bool on = (sw & 1) != 0;

	if (!on)
	{
		m_selector_repeat_enable = 0;
		if (m_selector_repeat_timer)     m_selector_repeat_timer->adjust(attotime::never);
		if (m_selector_repeat_off_timer) m_selector_repeat_off_timer->adjust(attotime::never);

		selector_w(0);
		return;
	}

	if (m_selector_repeat_enable)
		return;

	m_selector_repeat_enable = 1;

	if (!m_selector_work.lock && has_free_medal_event(m_selector_work))
	{
		selector_w(1);
		m_selector_repeat_off_timer->adjust(m_selector_repeat_pulse);
	}

	m_selector_repeat_timer->adjust(m_selector_repeat_period);
}

TIMER_CALLBACK_MEMBER(pachislot_device::selector_repeat_tick)
{
	if (!m_selector_repeat_enable)
		return;

	if (m_selector_work.lock || !has_free_medal_event(m_selector_work))
	{
		m_selector_repeat_timer->adjust(m_selector_repeat_period);
		return;
	}

	selector_w(1);
	m_selector_repeat_off_timer->adjust(m_selector_repeat_pulse);

	m_selector_repeat_timer->adjust(m_selector_repeat_period);
}

TIMER_CALLBACK_MEMBER(pachislot_device::selector_repeat_off_tick)
{
	selector_w(0);
}
// -----
