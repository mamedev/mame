// license:BSD-3-Clause
// copyright-holders:Dirk Best
/***************************************************************************

    PMI DAC-76 COMDAC

    Companding D/A Converter

***************************************************************************/

#include "emu.h"
#include "dac76.h"


//**************************************************************************
//  CONSTEXPR DEFINITIONS
//**************************************************************************

constexpr int dac76_device::m_level[];


//**************************************************************************
//  DEVICE DEFINITIONS
//**************************************************************************

DEFINE_DEVICE_TYPE(DAC76, dac76_device, "dac76", "PMI DAC-76 COMDAC")


//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  dac76_device - constructor
//-------------------------------------------------

dac76_device::dac76_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	device_t(mconfig, DAC76, tag, owner, clock),
	device_sound_interface(mconfig, *this),
	m_stream(nullptr),
	m_chord(0),
	m_step(0),
	m_sb(false)
{
}

//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void dac76_device::device_start()
{
	// create sound stream
	m_stream = stream_alloc(0, 1, machine().sample_rate() * 8);

	// register for save states
	save_item(NAME(m_chord));
	save_item(NAME(m_step));
	save_item(NAME(m_sb));
}

//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void dac76_device::device_reset()
{
	m_chord = 0;
	m_step = 0;
	m_sb = false;
}

//-------------------------------------------------
//  sound_stream_update - handle update requests for
//  our sound stream
//-------------------------------------------------

void dac76_device::sound_stream_update(sound_stream &stream, std::vector<read_stream_view> const &inputs, std::vector<write_stream_view> &outputs)
{
	// get current output level
	int step_size = (2 << m_chord);
	s32 vout = m_level[m_chord] + m_step * step_size;

	// apply sign bit
	vout *= (m_sb ? +1 : -1);

	// range is 0-8031, normalize to 0-1 range
	outputs[0].fill(stream_buffer::sample_t(vout) * (1.0 / 8031.0));
}
