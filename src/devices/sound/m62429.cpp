// license:BSD-3-Clause
// copyright-holders:MagikalUnicorn
/***************************************************************************

    Mitsubishi M62429P/FP dual electronic volume control. The two-wire
    interface shifts an 11-bit word LSB first, selecting one or both channels
    and combining 4 dB and 1 dB attenuation stages from 0 to -83 dB, or mute.
    DATA is sampled on rising CLOCK edges; DATA high on a falling edge latches.

    Outputs initially mute pending a documented volume setting. There is no
    reset pin, so a host soft reset does not clear the serial or volume state.

    TODO:
    - Establish power-on output and undocumented coarse attenuation codes 22-31.

***************************************************************************/

#include "emu.h"
#include "m62429.h"

#include <cmath>

#define LOG_VOLUME (1U << 1)

#define VERBOSE (0)
#include "logmacro.h"

DEFINE_DEVICE_TYPE(M62429, m62429_device, "m62429", "Mitsubishi M62429 Dual Electronic Volume Control")

m62429_device::m62429_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: device_t(mconfig, M62429, tag, owner, clock)
	, device_sound_interface(mconfig, *this)
{
}

void m62429_device::device_start()
{
	m_stream = stream_alloc(2, 2, SAMPLE_RATE_OUTPUT_ADAPTIVE);
	m_shift = 0;
	m_data = false;
	m_clock = false;
	m_gain[0] = m_gain[1] = 0.0f;

	save_item(NAME(m_shift));
	save_item(NAME(m_data));
	save_item(NAME(m_clock));
	save_item(NAME(m_gain));
}

void m62429_device::data_w(int state)
{
	m_data = bool(state);
}

void m62429_device::clock_w(int state)
{
	bool const clock = bool(state);
	if (m_clock == clock)
		return;

	if (clock)
		m_shift = (m_shift >> 1) | (m_data ? 0x400 : 0);
	else if (m_data && (BIT(m_shift, 9, 2) == 3))
		latch_data();

	m_clock = clock;
}

void m62429_device::latch_data()
{
	u8 const coarse = BIT(m_shift, 2, 5);
	if (coarse > 21)
	{
		LOGMASKED(LOG_VOLUME, "%s: undocumented coarse attenuation code %02x\n", machine().describe_context(), coarse);
		return;
	}

	u8 const attenuation = (21 - coarse) * 4 + BIT(~m_shift, 7, 2);
	float const gain = coarse ? std::pow(10.0f, -float(attenuation) / 20.0f) : 0.0f;
	m_stream->update();

	if (!BIT(m_shift, 1))
		m_gain[0] = m_gain[1] = gain;
	else
		m_gain[BIT(m_shift, 0)] = gain;

	LOGMASKED(LOG_VOLUME, "%s: volume word %03x, gains %f/%f\n", machine().describe_context(), m_shift, m_gain[0], m_gain[1]);
}

void m62429_device::sound_stream_update(sound_stream &stream)
{
	for (int channel = 0; channel < 2; ++channel)
	{
		for (int sample = 0; sample < stream.samples(); ++sample)
			stream.put(channel, sample, stream.get(channel, sample) * m_gain[channel]);
	}
}
