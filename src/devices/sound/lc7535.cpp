// license: BSD-3-Clause
// copyright-holders: Dirk Best
/***************************************************************************

    Sanyo LC7535

    Electronic Volume/Loudness Control with Serial Data Control

    It uses the Sanyo CCB (Computer Control Bus) to communicate. The
    device address is 0x08 or 0x09 (depending on the select pin).

***************************************************************************/

#include "emu.h"
#include "lc7535.h"

// disable to use a logarithmic scale, but volume is really low then
#define USE_LINEAR_SCALE 1


//**************************************************************************
//  DEVICE DEFINITIONS
//**************************************************************************

DEFINE_DEVICE_TYPE(LC7535, lc7535_device, "lc7535", "Sanyo LC7535")

lc7535_device::lc7535_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	device_t(mconfig, LC7535, tag, owner, clock),
	device_sound_interface(mconfig, *this),
	m_select_cb(*this, 1),
	m_addr(0), m_data(0),
	m_count(0),
	m_ce(false), m_di(false), m_clk(false)
{
}


//**************************************************************************
//  MACHINE EMULATION
//**************************************************************************

void lc7535_device::device_start()
{
	// allocate stream for 2 input channels and 2 output channels
	m_stream = stream_alloc(2, 2, SAMPLE_RATE_OUTPUT_ADAPTIVE);

	// register for save states
	save_item(NAME(m_addr));
	save_item(NAME(m_data));
	save_item(NAME(m_count));
	save_item(NAME(m_ce));
	save_item(NAME(m_di));
	save_item(NAME(m_clk));
	save_item(NAME(m_loudness));
	save_item(NAME(m_volume));
}

void lc7535_device::sound_stream_update(sound_stream &stream)
{
	for (int channel = 0; channel < 2 && channel < stream.output_count(); channel++)
	{
		for (int sampindex = 0; sampindex < stream.samples(); sampindex++)
			stream.put(channel, sampindex, stream.get(channel, sampindex) * m_volume[channel]);
	}
}

float lc7535_device::attenuation_to_gain(int attenuation)
{
#if USE_LINEAR_SCALE
	return (attenuation + (MAX * (-1.0))) / (MAX * (-1.0));
#else
	if (attenuation <= MAX)
		return 0.0f; // mute

	return powf(10.0f, attenuation / 20.0f);
#endif
}


//**************************************************************************
//  INTERFACE
//**************************************************************************

void lc7535_device::ce_w(int state)
{
	m_ce = bool(state);
}

void lc7535_device::di_w(int state)
{
	m_di = bool(state);
}

void lc7535_device::clk_w(int state)
{
	if (!m_clk && state == 1)
	{
		if (!m_ce)
		{
			if (m_count == 0)
				m_addr = 0;

			m_addr >>= 1;
			m_addr |= ((m_di ? 1 : 0) << 3);

			if (++m_count == 4)
				m_count = 0;
		}
		else
		{
			// our address is either 8 or 9
			if (m_addr == (8 | (m_select_cb() ? 0 : 1)))
			{
				m_data >>= 1;
				m_data |= ((m_di ? 1 : 0) << 15);

				if (++m_count == 16)
				{
					// left channel
					int att_l = m_1db[(m_data >> 12) & 0x07];
					if (att_l != MAX)
						att_l += m_5db[(m_data >> 8) & 0x0f];

					// right channel
					int att_r = m_1db[(m_data >> 4) & 0x07];
					if (att_r != MAX)
						att_r += m_5db[(m_data >> 0) & 0x0f];

					m_loudness = bool(BIT(m_data, 7));
					m_volume[0] = attenuation_to_gain(att_l);
					m_volume[1] = attenuation_to_gain(att_r);

					logerror("Volume: left = %d dB, right %d dB, loudness = %s\n", att_l, att_r, m_loudness ? "on" :"off");

					m_addr = 0;
					m_count = 0;
				}
			}
		}
	}

	m_clk = bool(state);
}
