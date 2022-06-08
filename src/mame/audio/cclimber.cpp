// license:BSD-3-Clause
// copyright-holders:Nicola Salmoria, hap
/***************************************************************************

Crazy Climber sound hardware

It has an AY-3-8910.
And 8KB ROM for voice samples, done with TTL chips.

***************************************************************************/

#include "emu.h"
#include "audio/cclimber.h"

#include "sound/ay8910.h"


//**************************************************************************
//  DEVICE DEFINITIONS
//**************************************************************************

DEFINE_DEVICE_TYPE(CCLIMBER_AUDIO, cclimber_audio_device, "cclimber_audio", "Crazy Climber Sound Board")


//-------------------------------------------------
//  cclimber_audio_device: Constructor
//-------------------------------------------------

cclimber_audio_device::cclimber_audio_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock) :
	device_t(mconfig, CCLIMBER_AUDIO, tag, owner, clock),
	m_dac(*this, "dac"),
	m_volume(*this, "volume"),
	m_rom(*this, "samples"),
	m_sample_clockdiv(2)
{ }


//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void cclimber_audio_device::device_start()
{
	assert(m_rom.bytes() == 0x2000);

	m_address = 0;
	m_start_address = 0;
	m_loop_address = 0;
	m_sample_rate = 0;
	m_sample_trigger = 0;

	m_sample_timer = timer_alloc(FUNC(cclimber_audio_device::sample_tick), this);
	m_sample_timer->adjust(attotime::zero);

	// register for savestates
	save_item(NAME(m_address));
	save_item(NAME(m_start_address));
	save_item(NAME(m_loop_address));
	save_item(NAME(m_sample_rate));
	save_item(NAME(m_sample_trigger));
}


//-------------------------------------------------
//  device_add_mconfig - add device configuration
//-------------------------------------------------

void cclimber_audio_device::device_add_mconfig(machine_config &config)
{
	ay8910_device &aysnd(AY8910(config, "aysnd", DERIVED_CLOCK(1, 1)));
	aysnd.port_a_write_callback().set(FUNC(cclimber_audio_device::start_address_w));
	aysnd.port_b_write_callback().set(FUNC(cclimber_audio_device::loop_address_w));
	aysnd.add_route(ALL_OUTPUTS, ":speaker", 0.5);

	DAC_4BIT_R2R(config, m_dac).add_route(ALL_OUTPUTS, "volume", 0.5);

	FILTER_VOLUME(config, m_volume).add_route(ALL_OUTPUTS, ":speaker", 1.0);
}


//-------------------------------------------------
//  handlers
//-------------------------------------------------

void cclimber_audio_device::sample_rate_w(u8 data)
{
	m_sample_rate = data;
}

void cclimber_audio_device::sample_volume_w(u8 data)
{
	m_volume->flt_volume_set_volume(double(data & 0x1f) / 31.0); // range 0-31
}

void cclimber_audio_device::sample_trigger(int state)
{
	// start playing on rising edge
	if (state && !m_sample_trigger)
		m_address = m_start_address * 64;

	m_sample_trigger = state;
}

void cclimber_audio_device::sample_trigger_w(u8 data)
{
	sample_trigger(data != 0);
}

TIMER_CALLBACK_MEMBER(cclimber_audio_device::sample_tick)
{
	u8 data = m_rom[m_address >> 1 & 0x1fff];

	// sample end marker, continue from loop point
	if (data == 0x70)
		m_address = m_loop_address * 64;

	m_dac->write((m_address & 1) ? (data & 0xf) : (data >> 4));

	if (m_sample_trigger)
		m_address++;

	m_sample_timer->adjust(attotime::from_ticks(256 - m_sample_rate, clock() / m_sample_clockdiv));
}
