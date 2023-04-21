// license:BSD-3-Clause
// copyright-holders:SomeRandomGuyIdk
/**********************************************************************

	Electromechanical reels for slot machines

**********************************************************************/

#include "emu.h"
#include "em_reel.h"

DEFINE_DEVICE_TYPE(EM_REEL, em_reel_device, "em_reel", "Electromechanical Reel")

em_reel_device::em_reel_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	device_t(mconfig, EM_REEL, tag, owner, clock),
	m_reel_out(*this, tag),
	m_state_cb(*this)
{}

void em_reel_device::set_state(uint8_t state)
{
	if(m_state == REEL_STOPPED)
	{
		if(state)
		{
			m_state = REEL_SPINNING;
			m_move_timer->adjust(m_step_period);
			m_state_cb(1);
		}
	}
	else if(m_state == REEL_SPINNING)
	{
		if(!state)
		{
			if(m_pos % m_steps_per_detent == 0) // If reel is already on a detent, then stop it immediately
			{
				m_move_timer->adjust(attotime::never);
				m_state = REEL_STOPPED;
				m_state_cb(0);
			}
			else m_state = REEL_STOPPING;
		}
	}
}

TIMER_CALLBACK_MEMBER( em_reel_device::move )
{
	if(m_direction == REVERSE)
	{
		if(m_pos == 0)
			m_pos = m_max_pos - 1;
		else
			m_pos--;
	}
	else
	{
		if(m_pos == m_max_pos - 1)
			m_pos = 0;
		else
			m_pos++;
	}

	if(m_state == REEL_STOPPING && m_pos % m_steps_per_detent == 0) // Stop once a detent is reached
	{
		m_move_timer->adjust(attotime::never);
		m_state = REEL_STOPPED;
		m_state_cb(0);
	}
	else m_move_timer->adjust(m_step_period);

	m_reel_out = (m_pos * 0x10000) / m_max_pos; // Scrolling reel output from awpvid.cpp
}

void em_reel_device::set_steps_per_detent(uint16_t numstops)
{
	if(m_max_pos % numstops != 0)
	{
		fatalerror("em_reel_device: Reel step amount %u is not divisible by numstops %u\n", m_max_pos, numstops);
	}

	m_steps_per_detent = m_max_pos / numstops;
}

void em_reel_device::device_start()
{
	m_reel_out.resolve();
	m_state_cb.resolve_safe();

	save_item(NAME(m_state));
	save_item(NAME(m_pos));
	save_item(NAME(m_max_pos));
	save_item(NAME(m_steps_per_detent));
	save_item(NAME(m_step_period));
	save_item(NAME(m_direction));

	m_move_timer = timer_alloc(FUNC(em_reel_device::move), this);

	m_state = REEL_STOPPED;
	m_pos = 0;
}
