// license:BSD-3-Clause
// copyright-holders:SomeRandomGuyIdk
/**********************************************************************

	Electromechanical reels for slot machines

**********************************************************************/

#include "emu.h"
#include "em_reel.h"

#include "speaker.h"

namespace {

const char *const sample_names[4] =
{
	"*em_reel",
	"em_reel_start",
	"em_reel_stop",
	nullptr
};

} // anonymous namespace

DEFINE_DEVICE_TYPE(EM_REEL, em_reel_device, "em_reel", "Electromechanical Reel")

em_reel_device::em_reel_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	device_t(mconfig, EM_REEL, tag, owner, clock),
	m_reel_out(*this, tag),
	m_samples(*this, "samples")
{}

void em_reel_device::set_state(uint8_t state)
{
	if(m_state == REEL_STOPPED)
	{
		if(state)
		{
			m_state = REEL_SPINNING;
			m_move_timer->adjust(m_speed_period);
			m_samples->start(0, SAMPLE_START);
		}
	}
	else if(m_state == REEL_SPINNING)
	{
		if(!state)
		{
			if(m_pos % STEPS_PER_SYMBOL == 0) // If reel is already on a symbol, then stop it immediately
			{
				m_move_timer->adjust(attotime::never);
				m_state = REEL_STOPPED;
				m_samples->start(0, SAMPLE_STOP);
			}
			else m_state = REEL_STOPPING;
		}
	}
}

uint8_t em_reel_device::read_pos()
{
	if(m_pos % STEPS_PER_SYMBOL == 0)
		return (m_pos / STEPS_PER_SYMBOL) + 1;
	else 
		return 0;
}

bool em_reel_device::read_bfm_symbol_opto()
{
	uint8_t sym_pos = m_pos % STEPS_PER_SYMBOL;
	return (sym_pos >= 12 && sym_pos <= 15); // Symbol tab is on every symbol
}

bool em_reel_device::read_bfm_reel_opto()
{
	return (m_pos >= 10 && m_pos <= 13); // Reel tab is only on the first symbol
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

	if(m_state == REEL_STOPPING && m_pos % STEPS_PER_SYMBOL == 0) // Stop once a symbol is reached
	{
		m_move_timer->adjust(attotime::never);
		m_state = REEL_STOPPED;
		m_samples->start(0, SAMPLE_STOP);
	}
	else m_move_timer->adjust(m_speed_period);

	m_reel_out = (m_pos * 0x10000) / m_max_pos; // Scrolling reel output from awpvid.cpp
}

void em_reel_device::device_start()
{
	m_reel_out.resolve();

	save_item(NAME(m_state));
	save_item(NAME(m_pos));
	save_item(NAME(m_max_pos));
	save_item(NAME(m_direction));
	save_item(NAME(m_speed_period));

	m_move_timer = timer_alloc(FUNC(em_reel_device::move), this);

	m_state = REEL_STOPPED;
	m_pos = 0;
}

void em_reel_device::device_add_mconfig(machine_config &config)
{
	SPEAKER(config, "reelsnd").front_center();

	SAMPLES(config, m_samples);
	m_samples->set_channels(1);
	m_samples->set_samples_names(sample_names);
	m_samples->add_route(ALL_OUTPUTS, "reelsnd", 1.0);
}
