// license:BSD-3-Clause
// copyright-holders:Devin Acker

/***************************************************************************

    Casio GT155 (HG51B155FD)

    This is the sound generator and DSP used in various higher-end
    "A-Squared Sound Source" keyboards and pianos between roughly 1994-2001.

    TODO:
    - verify per-voice lowpass filter behavior
    - DSP (architecture/instruction set seems to be the same as the standalone
      "GD277" DSP used in other contemporary keyboards)

***************************************************************************/

#include "emu.h"
#include "gt155.h"

#include <algorithm>

//**************************************************************************
//  DEVICE DEFINITIONS
//**************************************************************************

DEFINE_DEVICE_TYPE(GT155, gt155_device, "gt155", "Casio GT155")

gt155_device::gt155_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: device_t(mconfig, GT155, tag, owner, clock)
	, device_sound_interface(mconfig, *this)
	, device_rom_interface(mconfig, *this)
{
}

/**************************************************************************/
void gt155_device::device_start()
{
	m_stream = stream_alloc(0, 2, clock() / CLOCKS_PER_SAMPLE); // 16.384 MHz -> 32 kHz, or 24.576 MHz -> 48 kHz

	for (int i = 0; i < 0x800; i++)
	{
		const double frac = 1.0 - ((double)i / 0x7ff);
		m_volume[i] = 0x800 * pow(10, -2.15 * frac) * cos(0.5 * M_PI * frac * frac * frac);
	}

	save_item(NAME(m_data));
	save_item(NAME(m_dsp_data));
	save_item(NAME(m_rom_addr));

	save_item(STRUCT_MEMBER(m_voices, m_enable));
	save_item(STRUCT_MEMBER(m_voices, m_format));

	save_item(STRUCT_MEMBER(m_voices, m_addr));
	save_item(STRUCT_MEMBER(m_voices, m_addr_frac));
	save_item(STRUCT_MEMBER(m_voices, m_addr_end));
	save_item(STRUCT_MEMBER(m_voices, m_addr_loop));
	save_item(STRUCT_MEMBER(m_voices, m_addr_loop_frac));

	save_item(STRUCT_MEMBER(m_voices, m_pitch));

	save_item(STRUCT_MEMBER(m_voices, m_filter_gain));
	save_item(STRUCT_MEMBER(m_voices, m_filter));
	save_item(STRUCT_MEMBER(m_voices, m_filter_out));
	save_item(STRUCT_MEMBER(m_voices, m_filter_unk));

	save_item(STRUCT_MEMBER(m_voices, m_sample_last));
	save_item(STRUCT_MEMBER(m_voices, m_sample));

	save_item(STRUCT_MEMBER(m_voices, m_env_current));
	save_item(STRUCT_MEMBER(m_voices, m_env_target));
	save_item(STRUCT_MEMBER(m_voices, m_env_level));
	save_item(STRUCT_MEMBER(m_voices, m_env_scale));
	save_item(STRUCT_MEMBER(m_voices, m_env_rate));

	save_item(STRUCT_MEMBER(m_voices, m_balance));
	save_item(STRUCT_MEMBER(m_voices, m_dsp_send));

}

/**************************************************************************/
void gt155_device::device_reset()
{
	std::fill(std::begin(m_data), std::end(m_data), 0);
	std::fill(std::begin(m_dsp_data), std::end(m_dsp_data), 0);
	std::fill(std::begin(m_voices), std::end(m_voices), voice_t());
}

/**************************************************************************/
void gt155_device::device_clock_changed()
{
	m_stream->set_sample_rate(clock() / CLOCKS_PER_SAMPLE);
}

/**************************************************************************/
void gt155_device::sound_stream_update(sound_stream& stream)
{
	for (int i = 0; i < stream.samples(); i++)
	{
		s64 left = 0, right = 0;

		for (auto &voice : m_voices)
		{
			if (voice.m_enable)
			{
				mix_sample(voice, left, right);
				voice.update_envelope();
			}
		}

		stream.put_int_clamp(0, i, left >> 11, 32678);
		stream.put_int_clamp(1, i, right >> 11, 32768);
	}
}

/**************************************************************************/
void gt155_device::rom_bank_pre_change()
{
	m_stream->update();
}

/**************************************************************************/
void gt155_device::mix_sample(voice_t &voice, s64 &left, s64 &right)
{
	// update sample position
	voice.m_addr_frac += voice.m_pitch;
	if (voice.m_addr_frac >= (1 << 15))
		update_sample(voice);

	// interpolate, apply envelope + channel gain and lowpass, and mix into output
	s64 sample = voice.m_sample_last + (s64(voice.m_sample - voice.m_sample_last) * voice.m_addr_frac >> 15);
	// TODO: does this produce accurate filter output?
	sample = sample * voice.m_filter_gain;
	sample += s64(voice.m_filter_out) * (voice.m_filter ^ 0xffff);
	sample >>= 16;
	voice.m_filter_out = sample;

	const u16 env_level = voice.m_env_current >> (ENV_SHIFT + 4);
	sample *= m_volume[env_level];

	left  += (sample * voice.m_balance[0]) / 0x1f;
	right += (sample * voice.m_balance[1]) / 0x1f;
}

/**************************************************************************/
void gt155_device::voice_t::update_envelope()
{
	if (m_env_target > m_env_current
		&& (m_env_target - m_env_current) > m_env_rate)
	{
		m_env_current += m_env_rate;
	}
	else if (m_env_target < m_env_current
		&& (m_env_current - m_env_target) > m_env_rate)
	{
		m_env_current -= m_env_rate;
	}
	else
	{
		m_env_current = m_env_target;
	}
}

/**************************************************************************/
void gt155_device::update_sample(voice_t &voice)
{
	voice.m_sample_last = voice.m_sample;

	while (voice.m_addr_frac >= (1 << 15))
	{
		voice.m_addr += (voice.m_addr_frac >> 15) * (voice.m_format ? 2 : 1);
		voice.m_addr_frac &= 0x7fff;

		if (voice.m_addr >= voice.m_addr_end)
		{
			if (voice.m_addr_loop == voice.m_addr_end)
			{
				// if this is a one-shot sample, just disable it now
				voice.m_enable = 0;
				voice.m_env_current = voice.m_env_target = 0;
				return;
			}

			// apply the fractional component of the loop
			if (voice.m_format)
			{
				voice.m_addr -= (voice.m_addr_end - (voice.m_addr_loop & ~1));
				voice.m_addr_frac += (voice.m_addr_loop_frac >> 1);
				if (BIT(voice.m_addr_loop, 0))
					voice.m_addr_frac += 1 << 14;
			}
			else
			{
				voice.m_addr -= (voice.m_addr_end - voice.m_addr_loop);
				voice.m_addr_frac += voice.m_addr_loop_frac;
			}
		}
	}

	if (voice.m_format)
	{
		voice.m_sample = read_word(voice.m_addr & ~1);
	}
	else
	{
		// wk1800 apparently expects 8bit samples to only be expanded to 9 bits
		// (with m_filter_gain then boosted to compensate)
		voice.m_sample = s16(s8(read_byte(voice.m_addr))) << 1;
	}
}

/**************************************************************************/
void gt155_device::write(offs_t offset, u8 data)
{
	offset &= 0xf;

	if (offset < 6)
	{
		m_data[offset] = data;
	}
	else if (offset == 6)
	{
		switch (data)
		{
		case 0x00:
			m_rom_addr = (m_data[0] << 1) | (m_data[1] << 9) | (m_data[2] << 17);
			break;

		case 0x06:
			m_dsp_data[m_data[5] & 0x7f] &= 0xffff;
			m_dsp_data[m_data[5] & 0x7f] |= ((m_data[0] << 16) | (m_data[1] << 24));
			break;

		case 0x07:
			m_dsp_data[m_data[5] & 0x7f] &= 0xffff0000;
			m_dsp_data[m_data[5] & 0x7f] |= (m_data[0] | (m_data[1] << 8));
			return;

		default:
			voice_command(data);
			break;
		}

	}
	else
	{
		logerror("%s: write offset %u = %02x\n", machine().describe_context(), offset, data);
	}
}

/**************************************************************************/
u8 gt155_device::read(offs_t offset)
{
	u8 data = 0;
	offset &= 0xf;

	if (offset < 0x6)
	{
		data = m_data[offset];
	}
	else if (offset == 0xe)
	{
		data = read_byte(m_rom_addr);
	}
	else if (offset == 0xf)
	{
		data = read_byte(m_rom_addr + 1);
		if (!machine().side_effects_disabled())
			m_rom_addr += 2;
	}
	else if (!machine().side_effects_disabled())
	{
		logerror("%s: read offset %u\n", machine().describe_context(), offset);
	}

	return data;
}

/**************************************************************************/
u16 gt155_device::reg16(u8 num) const
{
	return m_data[num] | (m_data[num+1] << 8);
}

/**************************************************************************/
u32 gt155_device::reg24(u8 num) const
{
	return m_data[num] | (m_data[num+1] << 8) | (m_data[num+2] << 16);
}

/**************************************************************************/
u32 gt155_device::reg32(u8 num) const
{
	return m_data[num] | (m_data[num+1] << 8) | (m_data[num+2] << 16) | (m_data[num+3] << 24);
}

/**************************************************************************/
void gt155_device::voice_command(u8 data)
{
	m_stream->update();

	voice_t &voice = m_voices[m_data[5] & 0x1f];
	const u16 cmd = data | ((m_data[5] & 0xe0) << 8);

	switch (cmd)
	{
	case 0x0001: // sample start address
		voice.m_addr = reg32(1) >> 7;
		voice.m_addr_frac = reg16(0) & 0x7fff;
		break;

	case 0x2001: // sample loop address
		voice.m_addr_loop = reg32(1) >> 7;
		voice.m_addr_loop_frac = reg16(0) & 0x7fff;
		break;

	case 0x0002: // sample envelope step
	case 0x2002: // sample envelope step (double rate?) used when forcing a voice off
		voice.m_env_rate = reg16(0) & 0x7fff;
		if (cmd == 0x2002)
			voice.m_env_rate <<= 1;
		voice.m_env_level = reg16(2);
		voice.m_env_target = u32(voice.m_env_level) * voice.m_env_scale;
		break;

	case 0x0003: // sample end address and envelope scale
		voice.m_addr_end = reg24(0) << 1;
		voice.m_env_scale = m_data[3];
		voice.m_env_target = u32(voice.m_env_level) * voice.m_env_scale;
		break;

	case 0x0004:
		/*
		* params used by wk1800:
		* ff ff 00 00 (on boot)
		* fe 07 0c 00 (before note on)
		* f5 0d 0a 02 (starting 8-bit sample)
		* f5 05 0a 0a (starting 16-bit sample)
		* 00 20 00 00 (before note off)
		* 14 00 08 02 (after envelope update)
		*
		* TODO: is any of this related to how 8bit samples are expanded?
		* (see comment at bottom of update_sample)
		*/
		if (!voice.m_enable && !BIT(m_data[0], 1))
		{
			voice.m_format = BIT(m_data[3], 3);
			voice.m_sample_last = voice.m_sample = 0;
		}
		voice.m_enable = BIT(~m_data[0], 1);
		break;

	case 0x0005: // sample pitch
		voice.m_pitch = reg16(0);
		break;

	case 0x2005:
		{
			// this is actually gain premultiplied by filter level, so the sound HW does one fewer mult when applying the filter.
			const u16 gain = reg16(0);
			voice.m_filter_gain = (gain & 0x1fff) << (3 + (gain >> 13));
		}
		break;

	case 0x4005:
		/*
		filter coefficient (bit 15 inverted) - this is also premultiplied into the value written to m_filter_gain,
		with this value probably only being used as a coefficient for the previous filter output?
		*/
		voice.m_filter = reg16(0) ^ 0x8000;
		break;

	case 0x6005:
		voice.m_filter_unk = reg16(0);
		break;

	case 0x000e:
		voice.m_balance[0] = m_data[0] & 0x1f;
		voice.m_balance[1] = m_data[1] & 0x1f;
		break;

	case 0x200e:
		voice.m_dsp_send[0] = m_data[0] & 0x1f;
		voice.m_dsp_send[1] = m_data[1] & 0x1f;
		break;

	case 0x0014: // read envelope ready status
		if (voice.m_env_current == voice.m_env_target)
			m_data[2] = 0x00;
		else
			m_data[2] = 0x08;
		break;

	case 0x2018: // read envelope output & current sample output
		m_data[1] = voice.m_env_current >> ENV_SHIFT;
		m_data[2] = voice.m_env_current >> (ENV_SHIFT + 8);
		m_data[3] = voice.m_sample;
		m_data[4] = voice.m_sample >> 8;
		break;

	default:
		logerror("%s: sound cmd %02x%02x, param = %02x %02x %02x %02x %02x\n", machine().describe_context(),
				m_data[5], data,
				m_data[0], m_data[1], m_data[2], m_data[3], m_data[4]);
		break;
	}
}
