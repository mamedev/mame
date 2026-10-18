// license:BSD-3-Clause
// copyright-holders:cam900
/*
    Hudson HuC6230 SoundBox
    PSG part of HuC6280 with ADPCM

    TODO:
    - Volume changes are instant, hardware smooths them
    - Click suppression on channel reset
    - Verify the PSG/ADPCM/CD-DA balance
    - Implement/Correct VCA (Voltage Controlled Amplifier) behavior

    Related patent:
    - https://patents.google.com/patent/US5692099
    - https://patents.google.com/patent/US6453286
    - https://patents.google.com/patent/US5548655A
*/

#include "emu.h"
#include "huc6230.h"

#include <algorithm>
#include <cmath>


void huc6230_device::sound_stream_update(sound_stream &stream)
{
	for (int i = 0; i < stream.samples(); i++)
	{
		s32 samp0 = stream.get(0, i) * 32768.0;
		s32 samp1 = stream.get(1, i) * 32768.0;

		for (int adpcm = 0; adpcm < 2; adpcm++)
		{
			adpcm_channel *channel = &m_adpcm_channel[adpcm];
			samp0 += s32(channel->m_predictor * m_volume[channel->m_lvol]);
			samp1 += s32(channel->m_predictor * m_volume[channel->m_rvol]);
		}

		stream.put_int_clamp(0, i, samp0, 32768);
		stream.put_int_clamp(1, i, samp1, 32768);
	}
}

/*--------------------------------------------------------------------------*/
/* MAME specific code                                                       */
/*--------------------------------------------------------------------------*/

void huc6230_device::write(offs_t offset, uint8_t data)
{
	/* Update stream */
	m_stream->update();
	if (offset < 0x10)
	{
		m_psg->c6280_w(offset, data);
	}
	else if (offset < 0x11)
	{
		// ---- -xx- channel 1/0 linear interpolation
		// --xx ---- channel 1/0 reset (decoder state cleared, then held while set)
		for (int i = 0; i < 2; i++)
		{
			if (!BIT(m_adpcm_control, 4 + i) && BIT(data, 4 + i))
			{
				m_adpcm_channel[i].m_predictor = 0;
				m_adpcm_channel[i].m_step_index = 0;
			}
		}
		m_adpcm_control = data;
	}
	else if (offset < 0x15)
	{
		offset -= 0x11;
		if ((offset & 1) == 0)
			m_adpcm_channel[offset >> 1].m_lvol = data & 0x3f;
		else
			m_adpcm_channel[offset >> 1].m_rvol = data & 0x3f;
	}
	else if (offset < 0x16)
	{
		m_pcm_lvol = data & 0x3f;
		m_vca_cb(0, m_pcm_lvol);
	}
	else if (offset < 0x17)
	{
		m_pcm_rvol = data & 0x3f;
		m_vca_cb(1, m_pcm_rvol);
	}
}

/*
    ADPCM decoding, one step per sample: the HuC6272 supplies a nibble every 2^rate samples.
    4-bit IMA-like codes: delta = step * (magnitude + 1), 15-bit predictor.
    With interpolation on, each delta is spread over the 2^rate samples.
*/
TIMER_CALLBACK_MEMBER(huc6230_device::adpcm_timer)
{
	static const int step_sizes[49] =
	{
		16, 17, 19, 21, 23, 25, 28, 31, 34, 37, 41, 45, 50,
		55, 60, 66, 73, 80, 88, 97, 107, 118, 130, 143, 157,
		173, 190, 209, 230, 253, 279, 307, 337, 371, 408, 449,
		494, 544, 598, 658, 724, 796, 876, 963, 1060, 1166, 1282, 1411, 1552
	};
	static const int step_index_deltas[8] = { -1, -1, -1, -1, 2, 4, 6, 8 };

	for (int adpcm = 0; adpcm < 2; adpcm++)
	{
		adpcm_channel *channel = &m_adpcm_channel[adpcm];
		const u8 input = m_adpcm_update_cb[adpcm]();
		const int rate = (input >> 5) & 3;
		const bool interpolate = BIT(m_adpcm_control, 2 + adpcm);

		if (BIT(input, 7))
		{
			if (BIT(m_adpcm_control, 4 + adpcm))
				channel->m_delta = 0;
			else
			{
				const u8 nibble = input & 0xf;
				channel->m_delta = step_sizes[channel->m_step_index] * ((nibble & 7) + 1);
				if (interpolate)
					channel->m_delta >>= rate;
				if (BIT(nibble, 3))
					channel->m_delta = -channel->m_delta;
				channel->m_step_index = std::clamp(channel->m_step_index + step_index_deltas[nibble & 7], 0, 48);
			}
			channel->m_delta_count = interpolate ? (1 << rate) : 1;
		}

		if (channel->m_delta_count)
		{
			m_stream->update();
			channel->m_predictor = std::clamp(channel->m_predictor + channel->m_delta, -0x4000, 0x3fff);
			channel->m_delta_count--;
		}
	}
}

DEFINE_DEVICE_TYPE(HuC6230, huc6230_device, "huc6230", "Hudson Soft HuC6230 SoundBox")

huc6230_device::huc6230_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, HuC6230, tag, owner, clock)
	, device_sound_interface(mconfig, *this)
	, m_stream(nullptr)
	, m_psg(*this, "psg")
	, m_adpcm_control(0)
	, m_pcm_lvol(0)
	, m_pcm_rvol(0)
	, m_adpcm_update_cb(*this, 0)
	, m_vca_cb(*this)
{
}

//-------------------------------------------------
//  device_add_mconfig - add machine configuration
//-------------------------------------------------
void huc6230_device::device_add_mconfig(machine_config &config)
{
	C6280(config, m_psg, DERIVED_CLOCK(1,6));
	m_psg->add_route(0, DEVICE_SELF, 1.0, 0);
	m_psg->add_route(1, DEVICE_SELF, 1.0, 1);
}

//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void huc6230_device::device_start()
{
	m_stream = stream_alloc(2, 2, clock() / 6);
	m_adpcm_timer = timer_alloc(FUNC(huc6230_device::adpcm_timer), this);

	// 1.5dB per step, settings 0x00-0x1b are silent
	for (int i = 0; i < 0x40; i++)
		m_volume[i] = (i <= 0x1b) ? 0.0f : float(std::pow(2.0, -(0x3f - i) / 4.0));

	for (int i = 0; i < 2; i++)
	{
		m_adpcm_channel[i] = adpcm_channel{ 0, 0, 0, 0, 0, 0 };

		save_item(NAME(m_adpcm_channel[i].m_predictor), i);
		save_item(NAME(m_adpcm_channel[i].m_step_index), i);
		save_item(NAME(m_adpcm_channel[i].m_delta), i);
		save_item(NAME(m_adpcm_channel[i].m_delta_count), i);
		save_item(NAME(m_adpcm_channel[i].m_lvol), i);
		save_item(NAME(m_adpcm_channel[i].m_rvol), i);
	}
	save_item(NAME(m_adpcm_control));
	save_item(NAME(m_pcm_lvol));
	save_item(NAME(m_pcm_rvol));
}

void huc6230_device::device_clock_changed()
{
	m_stream->set_sample_rate(clock() / 6);
	// 2 samples per 1365 clock line
	const attotime adpcm_period = clocks_to_attotime(1365) / 2;
	m_adpcm_timer->adjust(adpcm_period, 0, adpcm_period);
}
