// license:BSD-3-Clause
// copyright-holders:Devin Acker
/***************************************************************************
    Casio GT913 sound (HLE)

    This is the sound portion of the GT913.
    Up to 24 voices can be mixed into a 16-bit stereo serial bitstream,
    which is then input to either a serial DAC or a HG51B-based DSP,
    depending on the model of keyboard.

    Currently, the actual sample format in ROM is unknown.
    The serial output is twos-complement 16-bit PCM, but the data in ROM
    doesn't seem to be - reading it as such produces sounds that are
    somewhat recognizable, but highly distorted.

    For now, all known (and unknown) register writes are just logged
    without generating any sound.

***************************************************************************/

#include "emu.h"
#include "gt913_snd.h"


//**************************************************************************
//  DEVICE DEFINITIONS
//**************************************************************************

DEFINE_DEVICE_TYPE(GT913_SOUND, gt913_sound_device, "gt913_sound_hle", "Casio GT913F sound")

// expand 2-bit exponent deltas
const u8 gt913_sound_device::exp_2_to_3[4] = { 0, 1, 2, 7 };

// sign-extend 7-bit sample deltas
const s8 gt913_sound_device::sample_7_to_8[128] =
{
	0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15,
	16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 26, 27, 28, 29, 30, 31,
	32, 33, 34, 35, 36, 37, 38, 39, 40, 41, 42, 43, 44, 45, 46, 47,
	48, 49, 50, 51, 52, 53, 54, 55, 56, 57, 58, 59, 60, 61, 62, 63,
	-64, -63, -62, -61, -60, -59, -58, -57, -56, -55, -54, -53, -52, -51, -50, -49,
	-48, -47, -46, -45, -44, -43, -42, -41, -40, -39, -38, -37, -36, -35, -34, -33,
	-32, -31, -30, -29, -28, -27, -26, -25, -24, -23, -22, -21, -20, -19, -18, -17,
	-16, -15, -14, -13, -12, -11, -10, -9, -8, -7, -6, -5, -4, -3, -2, -1
};

gt913_sound_device::gt913_sound_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: device_t(mconfig, GT913_SOUND, tag, owner, clock)
	, device_sound_interface(mconfig, *this)
	, device_memory_interface(mconfig, *this)
	, m_data_config("data", ENDIANNESS_BIG, 16, 21)
	, m_rom(*this, DEVICE_SELF_OWNER)
{
}

device_memory_interface::space_config_vector gt913_sound_device::memory_space_config() const
{
	return space_config_vector{ std::make_pair(0, &m_data_config) };
}

void gt913_sound_device::device_start()
{
	space(0).install_rom(0x000000, m_rom.bytes() - 1, m_rom.target());
	space(0).cache(m_cache);

	m_stream = stream_alloc(0, 2, std::round(clock() / 832.0f));

	save_item(NAME(m_gain));
	save_item(NAME(m_data));

	for (int i = 0; i < 24; i++)
	{
		save_item(NAME(m_voices[i].m_enable), i);

		save_item(NAME(m_voices[i].m_addr_start), i);
		save_item(NAME(m_voices[i].m_addr_end), i);
		save_item(NAME(m_voices[i].m_addr_loop), i);

		save_item(NAME(m_voices[i].m_addr_current), i);
		save_item(NAME(m_voices[i].m_addr_frac), i);
		save_item(NAME(m_voices[i].m_pitch), i);

		save_item(NAME(m_voices[i].m_sample), i);
		save_item(NAME(m_voices[i].m_sample_at_loop), i);
		save_item(NAME(m_voices[i].m_exp), i);
		save_item(NAME(m_voices[i].m_exp_at_loop), i);

		save_item(NAME(m_voices[i].m_volume_current), i);
		save_item(NAME(m_voices[i].m_volume_target), i);
		save_item(NAME(m_voices[i].m_volume_rate), i);

		save_item(NAME(m_voices[i].m_balance), i);
		save_item(NAME(m_voices[i].m_gain), i);
	}
}

void gt913_sound_device::device_reset()
{
	m_gain = 0;
	std::memset(m_data, 0, sizeof(m_data));

	std::memset(m_voices, 0, sizeof(m_voices));
}

void gt913_sound_device::sound_stream_update(sound_stream& stream, std::vector<read_stream_view> const& inputs, std::vector<write_stream_view>& outputs)
{
	for (int i = 0; i < outputs[0].samples(); i++)
	{
		s32 left = 0, right = 0;

		for (auto& voice : m_voices)
		{
			if (voice.m_enable)
				mix_sample(voice, left, right);
		}

	//	logerror("left/right %d/%d, with gain %d/%d\n", left, right, left * m_gain, right * m_gain);

		outputs[0].put_int(i, left * m_gain, INT_MAX);
		outputs[1].put_int(i, right * m_gain, INT_MAX);
	}
}

void gt913_sound_device::mix_sample(voice_t& voice, s32& left, s32& right)
{
	// update sample position
	voice.m_addr_frac += voice.m_pitch;
	while (voice.m_addr_frac >= (1 << 23))
	{
		voice.m_addr_frac -= (1 << 23);
		update_sample(voice);
	}

	// update volume envelope
	if (voice.m_volume_target > voice.m_volume_current
		&& (voice.m_volume_target - voice.m_volume_current) > voice.m_volume_rate)
	{
		voice.m_volume_current += voice.m_volume_rate;
	}
	else if (voice.m_volume_target < voice.m_volume_current
		&& (voice.m_volume_current - voice.m_volume_target) > voice.m_volume_rate)
	{
		voice.m_volume_current -= voice.m_volume_rate;
	}
	else
	{
		voice.m_volume_current = voice.m_volume_target;
	}

	// mix sample into output
	left += (s32)voice.m_sample * (voice.m_volume_current >> 8) * voice.m_balance[0];
	right += (s32)voice.m_sample * (voice.m_volume_current >> 8) * voice.m_balance[1];
}

void gt913_sound_device::update_sample(voice_t& voice)
{
	u16 word = m_cache.read_word(voice.m_addr_current & ~1U);
	s16 delta = 0;
	
	if (!BIT(voice.m_addr_current, 0))
	{
		voice.m_exp += exp_2_to_3[word & 3];
		voice.m_exp &= 7;
		delta = sample_7_to_8[(word >> 2) & 0x7f];
	}
	else
	{
		delta = sample_7_to_8[word >> 9];
	}

	voice.m_sample += delta * (1 << voice.m_exp); // *voice.m_gain; // TODO: make sure this is actually the gain value and not something else

	voice.m_addr_current++;
	if (voice.m_addr_current == voice.m_addr_end)
	{
		if (voice.m_addr_loop == voice.m_addr_end)
		{
			voice.m_enable = false;
			voice.m_sample = 0;
		}
		else
		{
			voice.m_sample = voice.m_sample_at_loop;
			voice.m_exp = voice.m_exp_at_loop;
			voice.m_addr_current = voice.m_addr_loop;
		}
	}
	else if (voice.m_addr_current == voice.m_addr_loop)
	{
	//	logerror("at loop start - sample %d exp %u\n", voice.m_sample, voice.m_exp);
		voice.m_sample_at_loop = voice.m_sample;
		voice.m_exp_at_loop = voice.m_exp;
	}
}

void gt913_sound_device::data_w(offs_t offset, u16 data)
{
	assert(offset < 3);
	m_data[offset] = data;
}

u16 gt913_sound_device::data_r(offs_t offset)
{
	assert(offset < 3);
	return m_data[offset];
}

void gt913_sound_device::command_w(u16 data)
{
	uint8_t voicenum = (data & 0x1f00) >> 8;
	uint16_t voicecmd = data & 0x60ff;

	if (data == 0x0012)
	{
		uint8_t gain = m_data[0] & 0x3f;
		if (gain != m_gain)
			logerror("gain %u\n", gain);
		m_gain = gain;
		return;
	}
	else if (voicenum >= 24)
	{
		return;
	}

	auto& voice = m_voices[voicenum];
	if (voicecmd == 0x0008)
	{
		voice.m_addr_start = (m_data[1] | (m_data[2] << 16)) & 0x1fffff;
	}
	else if (voicecmd == 0x0000)
	{
		voice.m_addr_end = (m_data[0] | (m_data[1] << 16)) & 0x1fffff;
	}
	else if (voicecmd == 0x2000)
	{
		voice.m_addr_loop = (m_data[0] | (m_data[1] << 16)) & 0x1fffff;
	}
	else if (voicecmd == 0x200a)
	{
		/* TODO: what does bit 4 of data[2] do? ctk551 sets it unconditionally */
		voice.m_exp = m_data[2] & 7;
	}
	else if (voicecmd == 0x200b)
	{
		bool enable = BIT(m_data[2], 7);
		if (enable && !m_voices[voicenum].m_enable)
		{
			voice.m_addr_current = voice.m_addr_start;
			voice.m_addr_frac = 0;
			voice.m_sample = 0;
			voice.m_volume_current = 0;
		}

		voice.m_enable = enable;
	}
	else if (voicecmd == 0x4004)
	{
		voice.m_balance[0] = (m_data[1] & 0xe0) >> 5;
		voice.m_balance[1] = (m_data[1] & 0x1c) >> 2;
	}
	else if (voicecmd == 0x4005)
	{
		voice.m_pitch = (m_data[0] << 8) | (m_data[1] >> 8);
	}
	else if (voicecmd == 0x6006)
	{
		voice.m_gain = m_data[1] & 0x7f; // TODO: verify this
	}
	else if (voicecmd == 0x6007)
	{
		voice.m_volume_target = m_data[0] & 0x7f00;
		voice.m_volume_rate = m_data[0] & 0xff;
	}
	else if (voicecmd == 0x2028)
	{
		/*
		ctk551 issues this command and then reads the voice's current volume from data0
		to determine if it's time to start the next part of the volume envelope or not.
		(TODO: also figure out what it expects to be returned in data1)
		*/
		m_data[0] = voice.m_enable ? voice.m_volume_current : 0;
		m_data[1] = 0;
	}
	else
	{
		logerror("unknown sound write %04x (data: %04x %04x %04x)\n", data, m_data[0], m_data[1], m_data[2]);
	}
}

u16 gt913_sound_device::status_r()
{
	/* ctk551 reads the current gain level out of the lower 6 bits and ignores the rest
	it's unknown what, if anything, the other bits are supposed to contain */
	return m_gain & 0x3f;
}
