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
	m_streaming_iref(false),
	m_voltage_output(false),
	m_r_pos(1.0F),
	m_r_neg(1.0F),
	m_chord(0),
	m_step(0),
	m_sb(false),
	m_fixed_iref(1.0F)
{
}

void dac76_device::configure_streaming_iref(bool streaming_iref)
{
	m_streaming_iref = streaming_iref;
}

void dac76_device::configure_voltage_output(float i2v_r_pos, float i2v_r_neg)
{
	m_voltage_output = true;
	m_r_pos = i2v_r_pos;
	m_r_neg = i2v_r_neg;
}

void dac76_device::set_fixed_iref(float iref)
{
	if (m_fixed_iref == iref)
		return;
	if (m_stream != nullptr)
		m_stream->update();
	m_fixed_iref = iref;
}

//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void dac76_device::device_start()
{
	// create sound stream
	const int input_count = m_streaming_iref ? 1 : 0;
	m_stream = stream_alloc(input_count, 1, machine().sample_rate() * 8);

	// register for save states
	save_item(NAME(m_chord));
	save_item(NAME(m_step));
	save_item(NAME(m_sb));
	save_item(NAME(m_fixed_iref));
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

void dac76_device::sound_stream_update(sound_stream &stream)
{
	// get current output level
	int step_size = (2 << m_chord);
	s32 vout = m_level[m_chord] + m_step * step_size;

	// apply sign bit
	vout *= (m_sb ? +1 : -1);

	// range is 0-8031, normalize to 0-1 range
	sound_stream::sample_t y = sound_stream::sample_t(vout) * (1.0 / 8031.0);

	if (m_voltage_output)
	{
		static constexpr const float FULL_SCALE_MULT = 3.8F;  // From datasheet.
		y *= ((y >= 0) ? m_r_pos : m_r_neg) * FULL_SCALE_MULT;
	}

	if (m_streaming_iref)
	{
		const int n = stream.samples();
		for (int i = 0; i < n; ++i)
			stream.put(0, i, stream.get(0, i) * y);
	}
	else
	{
		stream.fill(0, m_fixed_iref * y);
	}
}
