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
	std::fill(std::begin(m_outputs_hooked), std::end(m_outputs_hooked), false);
}

void saa1043_device::device_start()
{
	m_h = attotime::from_ticks(320, clock() * 2);
	m_line_count = s_line_counts[m_type];

	// resolve callbacks
	for (uint32_t i = 0; i < OUT_COUNT; i++)
	{
		m_outputs[i].resolve_safe();
		if (m_outputs_hooked[i])
		{
			m_timers[i] = timer_alloc(i);
			switch(i)
			{
				case V2:
					m_timers[V2]->adjust(m_h * 6, 1);
					break;
				default:
					// Not yet implemented
					break;
			}
		}
	}
}

void saa1043_device::device_reset()
{
	// Clear any existing clock states
	for (uint32_t i = 0; i < OUT_COUNT; i++)
	{
		m_outputs[i](CLEAR_LINE);
	}
	m_outputs[V2](ASSERT_LINE);
}

void saa1043_device::device_timer(emu_timer &timer, device_timer_id id, int param)
{
	switch (id)
	{
		case V2:
			m_outputs[V2](1 - param);
			if (param)
				m_timers[V2]->adjust(m_h * (m_line_count - 9), 0);
			else
				m_timers[V2]->adjust(m_h * 9, 1);
			break;

		default:
			// Not yet implemented
			break;
	}
}
