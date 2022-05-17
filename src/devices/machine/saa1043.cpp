// license:BSD-3-Clause
// copyright-holders:Ryan Holtz
/***********************************************************************

    Philips SAA1043 Universal Sync Generator

    TOOD:
    - Everything.

***********************************************************************/

#include "emu.h"
#include "saa1043.h"

#include <algorithm>


DEFINE_DEVICE_TYPE(SAA1043, saa1043_device, "saa1043", "Philips SAA1043")

/*static*/ const uint32_t saa1043_device::s_line_counts[4] = { 624, 624, 524, 524 };

saa1043_device::saa1043_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, SAA1043, tag, owner, clock)
	, m_outputs(*this)
	, m_type(PAL)
{
}

void saa1043_device::device_start()
{
	m_h = attotime::from_ticks(320, clock() * 2);
	m_line_count = s_line_counts[m_type];

	m_outputs.resolve_all_safe();

	m_timers[OUT_V2] = timer_alloc(FUNC(saa1043_device::toggle_v2), this);
	m_timers[OUT_V2]->adjust(m_h * 6, 1);
}

void saa1043_device::device_reset()
{
	for (uint32_t i = 0; i < OUT_COUNT; i++)
	{
		m_outputs[i](CLEAR_LINE);
	}
	m_outputs[OUT_V2](ASSERT_LINE);
}

TIMER_CALLBACK_MEMBER(saa1043_device::toggle_v2)
{
	m_outputs[OUT_V2](1 - param);
	if (param)
		m_timers[OUT_V2]->adjust(m_h * (m_line_count - 9), 0);
	else
		m_timers[OUT_V2]->adjust(m_h * 9, 1);
}
