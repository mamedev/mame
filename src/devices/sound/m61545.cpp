// license:BSD-3-Clause
// copyright-holders:MagikalUnicorn
/***************************************************************************

    Mitsubishi M61545AFP dual electronic volume control. The two-wire
    interface shifts an 18-bit word MSB first and independently attenuates
    two analogue channels in 1 dB steps from 0 to -95 dB, with a mute code.
    DATA is sampled on rising CLOCK edges; DATA high on a falling edge latches.

    Outputs initially mute pending a documented volume setting. There is no
    reset pin, so a host soft reset does not clear the serial or volume state.
    Supply-range selection is retained without modelling analogue headroom.

    TODO:
    - Establish power-on output and undocumented attenuation codes 96-126.
    - Model supply-dependent headroom and power-up settling/transients.

***************************************************************************/

#include "emu.h"
#include "m61545.h"

#include <cmath>

#define LOG_VOLUME (1U << 1)

#define VERBOSE (0)
#include "logmacro.h"

DEFINE_DEVICE_TYPE(M61545, m61545_device, "m61545", "Mitsubishi M61545AFP Dual Electronic Volume Control")

m61545_device::m61545_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: device_t(mconfig, M61545, tag, owner, clock)
	, device_sound_interface(mconfig, *this)
{
}

void m61545_device::device_start()
{
	m_stream = stream_alloc(2, 2, SAMPLE_RATE_OUTPUT_ADAPTIVE);
	m_shift = 0;
	m_supply = 0;
	m_data = false;
	m_clock = false;
	m_gain[0] = m_gain[1] = 0.0f;

	save_item(NAME(m_shift));
	save_item(NAME(m_supply));
	save_item(NAME(m_data));
	save_item(NAME(m_clock));
	save_item(NAME(m_gain));
}

void m61545_device::data_w(int state)
{
	m_data = bool(state);
}

void m61545_device::clock_w(int state)
{
	bool const clock = bool(state);
	if (m_clock == clock)
		return;

	if (clock)
		m_shift = ((m_shift << 1) | (m_data ? 1 : 0)) & 0x3ffff;
	else if (m_data && (BIT(m_shift, 0, 2) == 3))
		latch_data();

	m_clock = clock;
}

void m61545_device::latch_data()
{
	m_stream->update();
	m_supply = BIT(m_shift, 2, 2);

	for (unsigned channel = 0; channel < 2; ++channel)
	{
		u8 const attenuation = BIT(m_shift, channel ? 4 : 11, 7);
		if (attenuation <= 95)
			m_gain[channel] = std::pow(10.0f, -float(attenuation) / 20.0f);
		else if (attenuation == 127)
			m_gain[channel] = 0.0f;
		else
			LOGMASKED(LOG_VOLUME, "%s: channel %u undocumented attenuation code %02x\n", machine().describe_context(), channel, attenuation);
	}

	LOGMASKED(LOG_VOLUME, "%s: volume word %05x, gains %f/%f, supply range %u\n", machine().describe_context(), m_shift, m_gain[0], m_gain[1], m_supply);
}

void m61545_device::sound_stream_update(sound_stream &stream)
{
	for (int channel = 0; channel < 2; ++channel)
	{
		for (int sample = 0; sample < stream.samples(); ++sample)
			stream.put(channel, sample, stream.get(channel, sample) * m_gain[channel]);
	}
}
