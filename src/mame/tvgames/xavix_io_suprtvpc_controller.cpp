// license:BSD-3-Clause
// copyright-holders:MagikalUnicorn
/***************************************************************************

    Super TV-PC and Piano PC keyboard, gamepad and clock controller. Queues
    input changes and packed calendar reports over the host-clocked serial
    connection, and accepts clock-setting words sent by the console.

    The controller keeps its own six-bit wrapping year and sends clock updates
    as four ordered reports. Keyboard and player reports share the same queue.

***************************************************************************/

#include "emu.h"
#include "xavix_io_suprtvpc_controller.h"

#include <bit>

#define VERBOSE 0
#include "logmacro.h"

DEFINE_DEVICE_TYPE(XAVIX_TVPC_CONTROLLER, xavix_tvpc_controller_device, "xavix_tvpc", "XaviX TV-PC Controller")

xavix_tvpc_controller_device::xavix_tvpc_controller_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: device_t(mconfig, XAVIX_TVPC_CONTROLLER, tag, owner, clock)
	, device_rtc_interface(mconfig, *this)
	, m_event_out_cb(*this)
	, m_keys_cb(*this, 0)
	, m_right_shift_cb(*this, 0)
	, m_players_cb(*this, 0)
	, m_queue{}
{
}

void xavix_tvpc_controller_device::device_start()
{
	m_event_timer = timer_alloc(FUNC(xavix_tvpc_controller_device::event_timer), this);
	m_clock_timer = timer_alloc(FUNC(xavix_tvpc_controller_device::clock_timer), this);
	m_clock_timer->adjust(attotime::from_seconds(1), 0, attotime::from_seconds(1));

	save_item(NAME(m_register));
	save_item(NAME(m_queue));
	save_item(NAME(m_queue_head));
	save_item(NAME(m_queue_count));
	save_item(NAME(m_keyboard_state));
	save_item(NAME(m_player_state));
	save_item(NAME(m_clock_state));
	save_item(NAME(m_shift));
	save_item(NAME(m_bits_remaining));
	save_item(NAME(m_data));
	save_item(NAME(m_clock));
	save_item(NAME(m_link_active));
	save_item(NAME(m_sending));
	save_item(NAME(m_host_sending));
	save_item(NAME(m_host_bits));
	save_item(NAME(m_host_word));
}

void xavix_tvpc_controller_device::device_reset()
{
	m_event_timer->adjust(attotime::never);
	m_queue_head = 0;
	m_queue_count = 0;
	m_keyboard_state = read_keyboard();
	m_player_state.fill(0xff);
	m_clock_state = ~u32(0);
	m_shift = 0;
	m_bits_remaining = 0;
	m_data = 0;
	m_clock = false;
	m_link_active = false;
	m_sending = false;
	m_host_sending = false;
	m_host_bits = 0;
	m_host_word = 0;
}

void xavix_tvpc_controller_device::rtc_clock_updated(int year, int month, int day, int day_of_week, int hour, int minute, int second)
{
}

u8 xavix_tvpc_controller_device::days_in_month(u8 year, u8 month)
{
	return month >= 1 && month <= 12 ? ((month == 2 && !(year & 3)) ? 29 : DAYS_PER_MONTH[month - 1]) : 0;
}

u8 xavix_tvpc_controller_device::day_of_week(u8 year, u8 month, u8 day)
{
	unsigned full_year = 2000 + year;
	if (month < 3)
		full_year--;
	return (full_year + full_year / 4 - full_year / 100 + full_year / 400 + MONTH_OFFSETS[month - 1] + day) % 7 + 1;
}

u64 xavix_tvpc_controller_device::read_keyboard()
{
	u64 result = 0;
	for (unsigned row = 0; row < m_keys_cb.size(); row++)
		result |= u64(m_keys_cb[row]()) << (row * 8);

	// Both physical Shift keys generate the same controller key code.
	if (m_right_shift_cb())
		result |= u64(1) << 2;

	return result;
}

void xavix_tvpc_controller_device::scan_clock()
{
	if (!m_link_active)
		return;

	u32 const clock = packed_clock();
	if (clock != m_clock_state && QUEUE_SIZE - m_queue_count >= 4)
	{
		// The receiver commits the clock only after the descending type 3, 2, 1, 0 sequence.
		for (unsigned byte = 4; byte != 0; byte--)
			queue_report(byte - 1, clock >> ((byte - 1) * 8));
		m_clock_state = clock;
	}
}

u32 xavix_tvpc_controller_device::packed_clock()
{
	u8 const month = get_clock_register(RTC_MONTH);
	u8 const year = get_clock_register(RTC_YEAR);
	// The controller packs six fields continuously across four type-numbered bytes.
	u8 const byte0 = get_clock_register(RTC_SECOND) | ((get_clock_register(RTC_MINUTE) & 0x03) << 6);
	u8 const byte1 = ((get_clock_register(RTC_MINUTE) >> 2) & 0x0f) | ((get_clock_register(RTC_HOUR) & 0x0f) << 4);
	u8 const byte2 = ((get_clock_register(RTC_HOUR) >> 4) & 0x01) | ((get_clock_register(RTC_DAY) & 0x1f) << 1) | ((month & 0x03) << 6);
	u8 const byte3 = ((month >> 2) & 0x03) | ((year & 0x3f) << 2);
	return u32(byte0) | (u32(byte1) << 8) | (u32(byte2) << 16) | (u32(byte3) << 24);
}

void xavix_tvpc_controller_device::set_packed_clock(u32 packed)
{
	u8 const second = packed & 0x3f;
	u8 const minute = (packed >> 6) & 0x3f;
	u8 const hour = (packed >> 12) & 0x1f;
	u8 const day = (packed >> 17) & 0x1f;
	u8 const month = (packed >> 22) & 0x0f;
	u8 const year = (packed >> 26) & 0x3f;
	u8 const days = days_in_month(year, month);

	if (second >= 60 || minute >= 60 || hour >= 24 || !day || day > days)
	{
		LOG("%s: ignored invalid packed clock %08x\n", machine().describe_context(), packed);
		return;
	}

	set_time(true, year, month, day, day_of_week(year, month, day), hour, minute, second);
	m_clock_state = ~u32(0);
}

bool xavix_tvpc_controller_device::queue_report(u8 type, u8 data)
{
	if (m_queue_count >= QUEUE_SIZE)
	{
		LOG("%s: input report queue overflow\n", machine().describe_context());
		return false;
	}

	m_queue[(m_queue_head + m_queue_count) % QUEUE_SIZE] = (u16(type & 7) << 8) | data;
	m_queue_count++;
	return true;
}

void xavix_tvpc_controller_device::scan_inputs()
{
	scan_clock();

	u64 const keyboard = read_keyboard();
	u64 changed = keyboard ^ m_keyboard_state;
	while (changed)
	{
		unsigned const code = std::countr_zero(changed);
		if (!queue_report(6, code | (BIT(keyboard, code) ? 0x40 : 0x00)))
			break;
		m_keyboard_state ^= u64(1) << code;
		changed &= changed - 1;
	}

	for (unsigned player = 0; player < m_players_cb.size(); player++)
	{
		if (m_players_cb[player].isunset())
			continue;

		u8 const controls = m_players_cb[player]() & 0x1f;
		if (controls != m_player_state[player] && queue_report(4 + player, controls))
			m_player_state[player] = controls;
	}
}

void xavix_tvpc_controller_device::schedule_event()
{
	// Keep the next edge outside the PIO write callback that acknowledges the previous bit.
	m_event_timer->adjust(attotime::from_usec(10));
}

void xavix_tvpc_controller_device::start_report()
{
	if (!m_queue_count || m_sending || m_host_sending)
		return;

	u16 const report = m_queue[m_queue_head];
	m_queue_head = (m_queue_head + 1) % QUEUE_SIZE;
	m_queue_count--;

	u8 const type = report >> 8;
	u8 const payload_bits = PAYLOAD_BITS[type];
	u8 const payload_mask = (u16(1) << payload_bits) - 1;
	m_shift = (u16(type) << payload_bits) | (report & payload_mask);
	m_bits_remaining = 3 + payload_bits;
	m_data = BIT(m_shift, m_bits_remaining - 1);
	m_sending = true;
	schedule_event();
}

void xavix_tvpc_controller_device::host_w(u8 data, u8 direction)
{
	bool const clock = BIT(data, 6);
	bool const host_data_output = BIT(direction, 7);
	bool const link_active = BIT(direction, 6);
	bool const link_starting = !m_link_active && link_active;

	if (m_link_active && !link_active)
	{
		// Discard incomplete reports when the host releases the serial clock during boot or shutdown.
		m_event_timer->adjust(attotime::never);
		m_queue_head = 0;
		m_queue_count = 0;
		m_bits_remaining = 0;
		m_data = 0;
		m_sending = false;
		m_host_sending = false;
		m_host_bits = 0;
		m_host_word = 0;
	}
	m_link_active = link_active;
	if (link_starting)
	{
		// Start reporting on the next RTC tick; an immediate set can overlap the receiver's setup.
		m_clock_state = packed_clock();
	}

	if (!m_link_active)
	{
		m_clock = clock;
		return;
	}

	if (host_data_output)
	{
		if (!m_host_sending && clock)
		{
			m_sending = false;
			m_bits_remaining = 0;
			m_data = 0;
			m_host_sending = true;
			m_host_bits = 0;
			m_host_word = 0;
			schedule_event();
		}
		else if (m_host_sending && (clock != m_clock) && (m_host_bits < 32))
		{
			m_host_word = (m_host_word << 1) | BIT(data, 7);
			m_host_bits++;
			schedule_event();
		}
	}
	else if (m_host_sending)
	{
		LOG("%s: received host word %08x (%u bits)\n", machine().describe_context(), m_host_word, m_host_bits);
		if (m_host_bits == 32)
			set_packed_clock(m_host_word);
		m_host_sending = false;
		m_event_timer->adjust(attotime::never);
	}
	else if (m_sending && (clock != m_clock))
	{
		if (--m_bits_remaining)
		{
			m_data = BIT(m_shift, m_bits_remaining - 1);
			schedule_event();
		}
		else
		{
			m_sending = false;
			m_data = 0;
		}
	}
	else if (!m_sending && clock && (clock != m_clock))
	{
		start_report();
	}

	m_clock = clock;
}

void xavix_tvpc_controller_device::vblank_w(int state)
{
	if (!state || !m_link_active)
		return;

	scan_inputs();
}

TIMER_CALLBACK_MEMBER(xavix_tvpc_controller_device::event_timer)
{
	m_event_out_cb(1);
}

TIMER_CALLBACK_MEMBER(xavix_tvpc_controller_device::clock_timer)
{
	int second = get_clock_register(RTC_SECOND) + 1;
	int minute = get_clock_register(RTC_MINUTE);
	int hour = get_clock_register(RTC_HOUR);
	int day = get_clock_register(RTC_DAY);
	int month = get_clock_register(RTC_MONTH);
	int year = get_clock_register(RTC_YEAR) & 0x3f;
	int day_of_week = get_clock_register(RTC_DAY_OF_WEEK);

	if (second >= 60)
	{
		second = 0;
		if (++minute >= 60)
		{
			minute = 0;
			if (++hour >= 24)
			{
				hour = 0;
				day++;
				day_of_week = day_of_week % 7 + 1;

				int const days = days_in_month(year, month);
				if (day > days)
				{
					day = 1;
					if (++month > 12)
					{
						month = 1;
						year = (year + 1) & 0x3f;
					}
				}
			}
		}
	}

	// The controller stores a wrapping six-bit Gregorian year.
	set_time(true, year, month, day, day_of_week, hour, minute, second);
}
