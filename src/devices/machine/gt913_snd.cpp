// license:BSD-3-Clause
// copyright-holders:Devin Acker
/***************************************************************************
    Casio GT913 sound (HLE)

    This is the sound portion of the GT913.
    Up to 24 voices can be mixed into a 16-bit stereo serial bitstream,
    which is then input to either a serial DAC or a HG51B-based DSP,
    depending on the model of keyboard.

    The sample format, as well as other details such as the linear interpolation,
    are covered in these two Japanese patents:
    https://patents.google.com/patent/JP3603343B2/en
    https://patents.google.com/patent/JPH07199996A/en

    TODO: Volume envelope rates still need adjusting.
    (See comment in gt913_sound_device::command_w regarding command 6007)

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
	  0,   1,   2,   3,   4,   5,   6,   7,   8,   9,  10,  11,  12,  13,  14,  15,
	 16,  17,  18,  19,  20,  21,  22,  23,  24,  25,  26,  27,  28,  29,  30,  31,
	 32,  33,  34,  35,  36,  37,  38,  39,  40,  41,  42,  43,  44,  45,  46,  47,
	 48,  49,  50,  51,  52,  53,  54,  55,  56,  57,  58,  59,  60,  61,  62,  63,
	-64, -63, -62, -61, -60, -59, -58, -57, -56, -55, -54, -53, -52, -51, -50, -49,
	-48, -47, -46, -45, -44, -43, -42, -41, -40, -39, -38, -37, -36, -35, -34, -33,
	-32, -31, -30, -29, -28, -27, -26, -25, -24, -23, -22, -21, -20, -19, -18, -17,
	-16, -15, -14, -13, -12, -11, -10,  -9,  -8,  -7,  -6,  -5,  -4,  -3,  -2,  -1
};

// based on SW-10 softsynth
const u16 gt913_sound_device::volume_ramp[17] =
{
	0x0000, 0x00fa, 0x0231, 0x03b5, 0x0596, 0x07ee, 0x0ad8, 0x0e78,
	0x12fa, 0x1897, 0x1f93, 0x2843, 0x3313, 0x4087, 0x5143, 0x6617,
	0x8000
};

gt913_sound_device::gt913_sound_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: device_t(mconfig, GT913_SOUND, tag, owner, clock)
	, device_sound_interface(mconfig, *this)
	, device_rom_interface(mconfig, *this)
{
}

void gt913_sound_device::device_start()
{
	/*
	generate sound at 104 cycles per sample (~= 144.231 kHz sample clock to the DAC)
	on keyboard models that include a DSP, this also results in a multiple
	of the 36.058 kHz CPU->DSP sync signal shown in some schematics (WK-1200 and others)
	*/
	m_stream = stream_alloc(0, 2, clock() / 104);

	save_item(NAME(m_gain));
	save_item(NAME(m_data));

	save_item(STRUCT_MEMBER(m_voices, m_enable));

	save_item(STRUCT_MEMBER(m_voices, m_addr_start));
	save_item(STRUCT_MEMBER(m_voices, m_addr_end));
	save_item(STRUCT_MEMBER(m_voices, m_addr_loop));

	save_item(STRUCT_MEMBER(m_voices, m_addr_current));
	save_item(STRUCT_MEMBER(m_voices, m_addr_frac));
	save_item(STRUCT_MEMBER(m_voices, m_pitch));

	save_item(STRUCT_MEMBER(m_voices, m_sample));
	save_item(STRUCT_MEMBER(m_voices, m_sample_next));
	save_item(STRUCT_MEMBER(m_voices, m_exp));

	save_item(STRUCT_MEMBER(m_voices, m_volume_data));
	save_item(STRUCT_MEMBER(m_voices, m_volume_current));
	save_item(STRUCT_MEMBER(m_voices, m_volume_target));
	save_item(STRUCT_MEMBER(m_voices, m_volume_rate));

	save_item(STRUCT_MEMBER(m_voices, m_balance));
	save_item(STRUCT_MEMBER(m_voices, m_gain));
}

void gt913_sound_device::device_reset()
{
	m_gain = 0;
	std::memset(m_data, 0, sizeof(m_data));

	std::memset(m_voices, 0, sizeof(m_voices));
}

void gt913_sound_device::sound_stream_update(sound_stream& stream)
{
	for (int i = 0; i < stream.samples(); i++)
	{
		s64 left = 0, right = 0;

		for (auto& voice : m_voices)
		{
			update_envelope(voice);
			if (voice.m_enable)
				mix_sample(voice, left, right);
		}

		stream.put_int_clamp(0, i, (left * m_gain) >> 27, 32678);
		stream.put_int_clamp(1, i, (right * m_gain) >> 27, 32768);
	}
}

void gt913_sound_device::rom_bank_pre_change()
{
	m_stream->update();
}

void gt913_sound_device::mix_sample(voice_t& voice, s64& left, s64& right)
{
	// update sample position
	voice.m_addr_frac += voice.m_pitch;
	while (voice.m_enable && voice.m_addr_frac >= (1 << 25))
	{
		voice.m_addr_frac -= (1 << 25);
		update_sample(voice);
	}

	// interpolate, apply envelope + channel gain, and mix into output
	const u8 step = (voice.m_addr_frac >> 22) & 7;

	const u8 env = (voice.m_volume_current >> 27);
	const u16 env_step = (voice.m_volume_current >> 16) & 0x7ff;
	const u32 env_level = (u32)volume_ramp[env] + (((volume_ramp[env + 1] - volume_ramp[env]) * env_step) >> 11);

	const s64 sample = ((s64)voice.m_sample + (voice.m_sample_next * step / 8)) * voice.m_gain * env_level;

	left  += sample * voice.m_balance[0];
	right += sample * voice.m_balance[1];
}

void gt913_sound_device::update_envelope(voice_t& voice)
{
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
}

void gt913_sound_device::update_sample(voice_t& voice)
{
	voice.m_sample += voice.m_sample_next;

	if (voice.m_addr_current >= voice.m_addr_end)
	{
		if (voice.m_addr_loop == voice.m_addr_end)
		{
			voice.m_enable = false;
			return;
		}

		voice.m_addr_current = voice.m_addr_loop;
		/*
		The last 12 bytes of each sample are a table containing five sample and exponent value pairs
		for the data words immediately after the loop point. The first pair corresponds to what the
		sample and exponent value will be _after_ processing the first 16-bit word after the loop,
		so once we've reached that point, use those values to reload the current sample and exponent
		*/
		const u32 addr_loop_data = (voice.m_addr_end + 1) & ~1;

		voice.m_sample_next = read_word(addr_loop_data) - voice.m_sample;
		voice.m_exp = read_word(addr_loop_data + 10) & 7;

		if (!BIT(voice.m_addr_current, 0))
		{
			/*
			the loop data represents the state after applying both samples in a 16-bit word,
			so if we're looping to the first of the two samples, compensate for the second one
			*/
			const u16 word = read_word(voice.m_addr_current);
			const s16 delta = sample_7_to_8[word >> 9];
			voice.m_sample_next -= delta * (1 << voice.m_exp);
		}
	}
	else
	{
		/*
		For all other samples, just get the next sample delta value.
		For even-numbered samples, also update the exponent/shift value.
		*/
		const u16 word = read_word(voice.m_addr_current & ~1);
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
		voice.m_sample_next = delta * (1 << voice.m_exp);
	}

	voice.m_addr_current++;
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
	m_stream->update();

	const uint8_t voicenum = (data & 0x1f00) >> 8;
	const uint16_t voicecmd = data & 0x60ff;

	if (data == 0x0012)
	{
		m_gain = m_data[0] & 0x3f;
		return;
	}
	else if (voicenum >= 24)
	{
		return;
	}

	auto& voice = m_voices[voicenum];
	if (voicecmd == 0x0008) // voice data write commands
	{
		/*
		sample start addresses seem to need to be word-aligned to decode properly
		(see: ctk551 "Trumpet" patch, which will have a bad exponent value otherwise)
		this apparently doesn't apply to end/loop addresses, though, or else samples
		may loop badly or even become noticeably detuned
		TODO: is the LSB of start addresses supposed to indicate something else, then?
		*/
		voice.m_addr_start = (m_data[1] | (m_data[2] << 16)) & 0x3ffffe;
	}
	else if (voicecmd == 0x0000)
	{
		voice.m_addr_end = (m_data[0] | (m_data[1] << 16)) & 0x3fffff;
	}
	else if (voicecmd == 0x2000)
	{
		voice.m_addr_loop = (m_data[0] | (m_data[1] << 16)) & 0x3fffff;
	}
	else if (voicecmd == 0x200a)
	{
		/* TODO: what does bit 3 of data[2] do? ctk551 sets it unconditionally */
		voice.m_exp = m_data[2] & 7;
	}
	else if (voicecmd == 0x200b)
	{
		bool enable = BIT(m_data[2], 7);
		if (enable && !m_voices[voicenum].m_enable)
		{
			voice.m_addr_current = voice.m_addr_start;
			voice.m_addr_frac = 0;
			voice.m_volume_current = 0;
			voice.m_sample = voice.m_sample_next = 0;
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
		/*
		for pitch, data[1] apparently contains both the most and least significant of 4 bytes,
		with data0 in the middle. strange, but apparently correct (see higher octaves of ctk551 E.Piano2)
		*/
		voice.m_pitch = (m_data[1] << 24) | (m_data[0] << 8) | (m_data[1] >> 8);
	}
	else if (voicecmd == 0x6006)
	{
		/*
		per-voice gain used for normalizing samples
		currently treated such that the lower 3 bits are fractional
		*/
		voice.m_gain = m_data[1] & 0xff;
	}
	else if (voicecmd == 0x6007)
	{
	//  logerror("voice %u volume %u rate %u\n", voicenum, (m_data[0] >> 8), m_data[0] & 0xff);

		voice.m_volume_data = m_data[0];
		voice.m_volume_target = (m_data[0] & 0x7f00) << 16;

		// referenced from the SW-10 softsynth
		u8 base = m_data[0] & 0xff;
		u8 shift = base >> 5;
		switch (shift)
		{
		case 0:
			shift = base >> 2;
			base &= 3;
			break;

		case 1:
			shift = 8;
			base &= 0x1f;
			break;

		default:
			shift += 6;
			base = (base & 0x1f) | 0x20;
			break;
		}
		/*
		this part is less certain - the overall rate needs adjusting based on
		the sample rate difference between this and the softsynth.
		it's probably not exact, but it sounds okay
		*/
		voice.m_volume_rate = (base * 3) << (shift + 5);
	}
	else if (voicecmd == 0x2028) // voice data read commands
	{
		/*
		data0 is used to determine if it's time to start the next part of the volume envelope or not
		*/
		m_data[0] = voice.m_volume_current >> 16;
		/*
		data1 is used to read consecutive output samples and detect zero crossings when
		applying volume or expression changes to a MIDI channel
		*/
		m_data[1] = voice.m_sample;
	}
	else if (voicecmd == 0x6020)
	{
		/*
		AP-10 sometimes issues this command, then clears the low byte of data0,
		and then issues command 0x6007 with the result (to pause an envelope?)
		*/
		m_data[0] = voice.m_volume_data;
	}
	else
	{
		logerror("unknown sound write %04x (data: %04x %04x %04x)\n", data, m_data[0], m_data[1], m_data[2]);
	}
}

u16 gt913_sound_device::status_r()
{
	/*
	ctk551 reads the current gain level out of the lower 6 bits and ignores the rest
	it's unknown what, if anything, the other bits are supposed to contain
	*/
	return m_gain & 0x3f;
}
