// license:BSD-3-Clause
// copyright-holders:R. Belmont
/***************************************************************************

    nds_sound.cpp
	Nintendo DS sound: 16 channels of PCM, ADPCM, PSG, or noise
	Emulation by R. Belmont

	TODO: 
	- bugs, lots of bugs
	- the capture units

***************************************************************************/

#include "emu.h"
#include "nds_sound.h"


DEFINE_DEVICE_TYPE(NDS_SOUND, nds_sound_device, "nds_sound", "Nintendo DS sound")

namespace {

// IMA-ADPCM magic numbers
const int16_t s_ima_step[89] =
{
	7,     8,     9,     10,    11,    12,    13,    14,    16,    17,
	19,    21,    23,    25,    28,    31,    34,    37,    41,    45,
	50,    55,    60,    66,    73,    80,    88,    97,    107,   118,
	130,   143,   157,   173,   190,   209,   230,   253,   279,   307,
	337,   371,   408,   449,   494,   544,   598,   658,   724,   796,
	876,   963,   1060,  1166,  1282,  1411,  1552,  1707,  1878,  2066,
	2272,  2499,  2749,  3024,  3327,  3660,  4026,  4428,  4871,  5358,
	5894,  6484,  7132,  7845,  8630,  9493,  10442, 11487, 12635, 13899,
	15289, 16818, 18500, 20350, 22385, 24623, 27086, 29794, 32767
};

const int s_ima_index[8] = { -1, -1, -1, -1, 2, 4, 6, 8 };

} // anonymous namespace


nds_sound_device::nds_sound_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, NDS_SOUND, tag, owner, clock)
	, device_sound_interface(mconfig, *this)
	, m_stream(nullptr)
	, m_space(nullptr)
{
}

void nds_sound_device::device_start()
{
	m_stream = stream_alloc(0, 2, OUTPUT_RATE);

	save_item(NAME(m_regs));
	save_item(STRUCT_MEMBER(m_chan, active));
	save_item(STRUCT_MEMBER(m_chan, format));
	save_item(STRUCT_MEMBER(m_chan, repeat));
	save_item(STRUCT_MEMBER(m_chan, volume));
	save_item(STRUCT_MEMBER(m_chan, vol_shift));
	save_item(STRUCT_MEMBER(m_chan, pan));
	save_item(STRUCT_MEMBER(m_chan, duty));
	save_item(STRUCT_MEMBER(m_chan, hold));
	save_item(STRUCT_MEMBER(m_chan, sad));
	save_item(STRUCT_MEMBER(m_chan, loopstart));
	save_item(STRUCT_MEMBER(m_chan, length));
	save_item(STRUCT_MEMBER(m_chan, addr));
	save_item(STRUCT_MEMBER(m_chan, frac));
	save_item(STRUCT_MEMBER(m_chan, step));
	save_item(STRUCT_MEMBER(m_chan, sample));
	save_item(STRUCT_MEMBER(m_chan, adpcm_val));
	save_item(STRUCT_MEMBER(m_chan, adpcm_index));
	save_item(STRUCT_MEMBER(m_chan, adpcm_loop_val));
	save_item(STRUCT_MEMBER(m_chan, adpcm_loop_index));
	save_item(STRUCT_MEMBER(m_chan, adpcm_loop_saved));
	save_item(STRUCT_MEMBER(m_chan, adpcm_low_nibble));
	save_item(STRUCT_MEMBER(m_chan, psg_phase));
	save_item(STRUCT_MEMBER(m_chan, noise_lfsr));
}

void nds_sound_device::device_reset()
{
	std::fill(std::begin(m_regs), std::end(m_regs), 0);
	for (auto &c : m_chan)
	{
		c = channel_t{};
	}
}

// the channel timer counts up at half the 33.513982 MHz clock
uint32_t nds_sound_device::chan_freq(int ch) const
{
	const uint32_t tmr = m_regs[(ch * 4) + 2] & 0xffff;
	const uint32_t period = 0x10000 - tmr;
	return (33513982 / 2) / period;
}

void nds_sound_device::key_on(int ch)
{
	channel_t &c = m_chan[ch];
	const uint32_t cnt = m_regs[ch * 4];

	c.volume = cnt & 0x7f;
	static const uint8_t div_shift[4] = { 0, 1, 2, 4 };
	c.vol_shift = div_shift[(cnt >> 8) & 3];
	c.hold = BIT(cnt, 15);
	c.pan = (cnt >> 16) & 0x7f;
	c.duty = (cnt >> 24) & 7;
	c.repeat = (cnt >> 27) & 3;
	c.format = (cnt >> 29) & 3;

	c.sad = m_regs[(ch * 4) + 1] & 0x07ffffff;
	const uint32_t pnt = (m_regs[(ch * 4) + 2] >> 16) & 0xffff;
	const uint32_t len = m_regs[(ch * 4) + 3] & 0x3fffff;
	c.loopstart = pnt * 4;
	c.length = (pnt + len) * 4;

	c.frac = 0;
	c.step = uint32_t((uint64_t(chan_freq(ch)) << 16) / OUTPUT_RATE);
	c.adpcm_loop_saved = false;

	// load the first sample and leave addr pointing at the second
	c.addr = c.sad;
	switch (c.format)
	{
		case FORMAT_PCM8:
			c.sample = m_space ? (int16_t(int8_t(m_space->read_byte(c.addr))) << 8) : 0;
			c.addr += 1;
			break;

		case FORMAT_PCM16:
			c.sample = m_space ? int16_t(m_space->read_word(c.addr)) : 0;
			c.addr += 2;
			break;

		case FORMAT_ADPCM:
		{
			const uint32_t header = m_space ? m_space->read_dword(c.addr) : 0;
			c.adpcm_val = int16_t(header & 0xffff);
			c.adpcm_index = std::clamp<int32_t>((header >> 16) & 0x7f, 0, 88);
			c.adpcm_low_nibble = true;
			c.sample = c.adpcm_val;
			c.addr += 4;
			break;
		}

		case FORMAT_PSG:
			c.psg_phase = 0;
			c.noise_lfsr = 0x7fff;
			c.sample = 0;
			break;
	}

	c.active = true;
}

void nds_sound_device::advance_sample(channel_t &c, int ch)
{
	if (c.format == FORMAT_PSG)
	{
		if (ch >= 14)
		{
			// LFSR noise
			if (c.noise_lfsr & 1)
			{
				c.noise_lfsr = (c.noise_lfsr >> 1) ^ 0x6000;
				c.sample = -0x7fff;
			}
			else
			{
				c.noise_lfsr >>= 1;
				c.sample = 0x7fff;
			}
		}
		else if (ch >= 8)
		{
			// square wave
			c.psg_phase = (c.psg_phase + 1) & 7;
			const int high = (c.duty == 7) ? 0 : (c.duty + 1);
			c.sample = (int(c.psg_phase) < high) ? 0x7fff : -0x7fff;
		}
		return;
	}

	// end of the sample?
	if (c.addr >= (c.sad + c.length))
	{
		if (c.repeat == 1)
		{
			c.addr = c.sad + c.loopstart;
			if ((c.format == FORMAT_ADPCM) && c.adpcm_loop_saved)
			{
				c.adpcm_val = c.adpcm_loop_val;
				c.adpcm_index = c.adpcm_loop_index;
				c.adpcm_low_nibble = true;
			}
		}
		else
		{
			c.active = false;
			if (!c.hold)
			{
				c.sample = 0;
			}
			return;
		}
	}

	// save the ADPCM decoder state at loop start
	if ((c.format == FORMAT_ADPCM) && !c.adpcm_loop_saved && (c.addr >= (c.sad + c.loopstart)) && (c.loopstart >= 4))
	{
		c.adpcm_loop_val = c.adpcm_val;
		c.adpcm_loop_index = c.adpcm_index;
		c.adpcm_loop_saved = true;
	}

	if (!m_space)
	{
		return;
	}

	switch (c.format)
	{
		case FORMAT_PCM8:
			c.sample = int16_t(int8_t(m_space->read_byte(c.addr))) << 8;
			c.addr += 1;
			break;

		case FORMAT_PCM16:
			c.sample = int16_t(m_space->read_word(c.addr));
			c.addr += 2;
			break;

		case FORMAT_ADPCM:
		{
			const uint8_t byte = m_space->read_byte(c.addr);
			const int nibble = c.adpcm_low_nibble ? (byte & 0xf) : (byte >> 4);

			const int step = s_ima_step[c.adpcm_index];
			int diff = step >> 3;
			if (nibble & 1)
			{
				diff += step >> 2;
			}
			if (nibble & 2)
			{
				diff += step >> 1;
			}
			if (nibble & 4)
			{
				diff += step;
			}
			if (nibble & 8)
			{
				c.adpcm_val = std::max<int32_t>(c.adpcm_val - diff, -0x7fff);
			}
			else
			{
				c.adpcm_val = std::min<int32_t>(c.adpcm_val + diff, 0x7fff);
			}
			c.adpcm_index = std::clamp<int32_t>(c.adpcm_index + s_ima_index[nibble & 7], 0, 88);
			c.sample = c.adpcm_val;

			if (c.adpcm_low_nibble)
			{
				c.adpcm_low_nibble = false;
			}
			else
			{
				c.adpcm_low_nibble = true;
				c.addr += 1;
			}
			break;
		}
	}
}

void nds_sound_device::mute_all()
{
	m_stream->update();
	for (auto &c : m_chan)
	{
		c.active = false;
	}
}

uint32_t nds_sound_device::read(offs_t offset, uint32_t mem_mask)
{
	if (offset >= std::size(m_regs))
	{
		return 0;
	}
	return m_regs[offset];
}

void nds_sound_device::write(offs_t offset, uint32_t data, uint32_t mem_mask)
{
	if (offset >= std::size(m_regs))
	{
		return;
	}

	m_stream->update();

	if ((offset < (NUM_CHANNELS * 4)) && ((offset & 3) == 0))
	{
		const uint32_t old = m_regs[offset];
		COMBINE_DATA(&m_regs[offset]);
		const int ch = offset / 4;
		if (BIT(m_regs[offset], 31) && !BIT(old, 31))
		{
			key_on(ch);
		}
		else if (!BIT(m_regs[offset], 31))
		{
			m_chan[ch].active = false;
		}
		return;
	}

	COMBINE_DATA(&m_regs[offset]);
}

void nds_sound_device::sound_stream_update(sound_stream &stream)
{
	const uint32_t soundcnt = m_regs[0x500 / 4 - 0x100];
	const bool master_enable = BIT(soundcnt, 15);
	const int master_vol = soundcnt & 0x7f;

	for (int i = 0; i < stream.samples(); i++)
	{
		int32_t left = 0, right = 0;

		if (master_enable)
		{
			for (int ch = 0; ch < NUM_CHANNELS; ch++)
			{
				channel_t &c = m_chan[ch];
				if (!c.active)
				{
					continue;
				}

				int32_t v = (int32_t(c.sample) * c.volume) >> 7;
				v >>= c.vol_shift;
				left += (v * (128 - c.pan)) >> 7;
				right += (v * c.pan) >> 7;

				c.frac += c.step;
				for (int guard = 0; (c.frac >= 0x10000) && (guard < 64); guard++)
				{
					c.frac -= 0x10000;
					advance_sample(c, ch);
				}
			}

			left = (left * master_vol) >> 7;
			right = (right * master_vol) >> 7;
		}

		stream.put_int_clamp(0, i, left, 32768);
		stream.put_int_clamp(1, i, right, 32768);
	}
}
