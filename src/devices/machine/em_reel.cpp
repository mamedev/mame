// license:BSD-3-Clause
// copyright-holders:SomeRandomGuyIdk
/**********************************************************************

    Electromechanical reels for slot machines

**********************************************************************/

#include "emu.h"
#include "em_reel.h"

#include <algorithm>


ALLOW_SAVE_TYPE(em_reel_device::dir);
ALLOW_SAVE_TYPE(em_reel_device::reel_state);

DEFINE_DEVICE_TYPE(EM_REEL, em_reel_device, "em_reel", "Electromechanical Reel")

em_reel_device::em_reel_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	device_t(mconfig, EM_REEL, tag, owner, clock),
	m_reel_out(*this, tag),
	m_state_cb(*this)
{
}

void em_reel_device::set_state(uint8_t state)
{
	if(m_state == reel_state::STOPPED)
	{
		if(state)
		{
			m_state = reel_state::SPINNING;
			m_move_timer->adjust(m_step_period);
			m_state_cb(1);
		}
	}
	else if(m_state == reel_state::SPINNING)
	{
		if(!state)
		{
			if(is_at_detent()) // If reel is already on a detent, then stop it immediately
			{
				m_move_timer->adjust(attotime::never);
				m_state = reel_state::STOPPED;
				m_state_cb(0);
			}
			else
			{
				m_state = reel_state::STOPPING;
			}
		}
	}
	else if(m_state == reel_state::STOPPING)
	{
		if(state)
			m_state = reel_state::SPINNING;
	}
}

inline bool em_reel_device::is_at_detent() const
{
	auto const found = std::lower_bound(m_detents.begin(), m_detents.end(), m_pos);
	return (m_detents.end() != found) && (*found == m_pos);
}

TIMER_CALLBACK_MEMBER( em_reel_device::move )
{
	if(m_direction == dir::REVERSE)
	{
		if(m_pos == 0)
			m_pos = m_max_pos;
		else
			m_pos--;
	}
	else
	{
		if(m_pos == m_max_pos)
			m_pos = 0;
		else
			m_pos++;
	}

	if(m_state == reel_state::STOPPING && is_at_detent()) // Stop once a detent is reached
	{
		m_state = reel_state::STOPPED;
		m_state_cb(0);
	}
	else
	{
		m_move_timer->adjust(m_step_period);
	}

	m_reel_out = m_pos;
}

void em_reel_device::device_start()
{
	m_reel_out.resolve();

	save_item(NAME(m_state));
	save_item(NAME(m_pos));
	save_item(NAME(m_direction));

	m_move_timer = timer_alloc(FUNC(em_reel_device::move), this);

	m_state = reel_state::STOPPED;
	m_pos = 0;
}

void em_reel_device::device_validity_check(validity_checker &valid) const
{
	auto detent = m_detents.begin();
	while(m_detents.end() != detent)
	{
		if(*detent > m_max_pos)
			fatalerror("Detent on step %u is out of range, maximum is %u\n", *detent, m_max_pos);

		auto const next = std::next(detent);
		if((m_detents.end() != next) && (*next <= *detent))
			fatalerror("Detents %u and %u are not in ascending order\n", *detent, *next);

		detent = next;
	}
}
