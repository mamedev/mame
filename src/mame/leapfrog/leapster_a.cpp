// license:BSD-3-Clause
// copyright-holders:Alice Shelton
#include "emu.h"
#include "leapster_a.h"

#define LOG_SOUND    (1U << 1)

#define VERBOSE (LOG_SOUND)

#include "logmacro.h"


/*
    Note: Everything here probably also applies to the Leappad and earlier Leapfrog systems,
          they seem to have reused their sound hardware. This would also explain why audio
          registers are all 16-bit.

    The Leapster has 8 sound channels:
      0-4: Pitched audio: Take an ALAW waveform, with a register providing the pitch
      5-6: Raw audio: Simply take an 8 KHz ALAW waveform
        7: Speech: Takes CELP-compressed input and a codebook to do speech playback


*/

DEFINE_DEVICE_TYPE(LEAPSTER_SOUND, leapster_snd_device, "leapster_snd", "Leapster Sound")

leapster_snd_device::leapster_snd_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	device_t(mconfig, LEAPSTER_SOUND, tag, owner, clock),
	device_sound_interface(mconfig, *this)
{

}

void leapster_snd_device::do_voice_command(uint8_t command, uint8_t voice)
{
	LOGMASKED(LOG_SOUND, "Received command %i.%i\n", command, voice);

	m_stream->update();

	if(command == 0x10)
	{
		if(voice < 5)
		{
			m_channel_dx[voice] =  ((float) m_pitch[voice]) / 0x8000;
			m_channel_index[voice] = (float) BIT(m_data_source_start[voice], 0, 16);
			m_channel_triggered[voice] = true;
			LOGMASKED(LOG_SOUND, "Triggering channel %i, Diff: %i, Pitch register: %04hX (%f)\n",
				   voice, (BIT(m_data_source_end[voice], 16, 16) - BIT(m_data_source_end[voice], 0, 16)),
				   m_pitch[voice], m_channel_dx[voice]);
		}
		else if(voice < 7)
		{
			m_channel_dx[voice] = 1.0f;
			m_channel_index[voice] = 0.0f;
			m_channel_triggered[voice] = true;
			LOGMASKED(LOG_SOUND, "Triggering channel %i\n", voice);
		}
	}
	else
	{
		m_channel_triggered[voice] = false;
	}
}

// Codebook is 20224 (0x4f00) bytes long and is constant across leapster and leappad revisions
//   The Leapster BIOS asserts that the lower 15 bits of the pointer are 0x0900 so that might
//   be a hardware requirement?
void leapster_snd_device::celp_codebook_w(uint32_t off, uint16_t value) {
	switch(BIT(off, 0, 2))
	{
		case 0:
			m_celp_codebook = (((uint32_t) value) << 16) | BIT(m_celp_codebook, 0, 16);
			break;
		case 2:
			m_celp_codebook = (BIT(m_celp_codebook, 16, 16) << 16) | value;
			break;
		default:
			LOGMASKED(LOG_SOUND, "%s: celp_codebook_w write to %i\n", machine().describe_context(), BIT(off, 0, 2));
	}
}

void leapster_snd_device::voice_generic_w(uint32_t off, uint16_t value)
{
	LOGMASKED(LOG_SOUND, "Wrote %04X to %03X\n", value, off << 1);
}

void leapster_snd_device::voice_start_w(uint32_t off, uint16_t value)
{
	auto channel = off >> 2;

	switch(BIT(off, 0, 2))
	{
		case 0:
			m_data_source_start[channel] = (((uint32_t) value) << 16) | BIT(m_data_source_start[channel], 0, 16);
			break;
		case 2:
			m_data_source_start[channel] = (BIT(m_data_source_start[channel], 16, 16) << 16) | value;
			break;
		case 3:
		default:
			LOGMASKED(LOG_SOUND, "%s: voice_start_w write to %i\n", machine().describe_context(), BIT(off, 0, 2));
	}
}

void leapster_snd_device::voice_end_w(uint32_t off, uint16_t value)
{
	auto channel = off >> 2;

	switch(BIT(off, 0, 2))
	{
		case 0:
			m_data_source_end[channel] = (((uint32_t) value) << 16) | BIT(m_data_source_end[channel], 0, 16);
			break;
		case 2:
			m_data_source_end[channel] = (BIT(m_data_source_end[channel], 16, 16) << 16) | value;
			break;
		default:
			LOGMASKED(LOG_SOUND, "%s: voice_start_w write to %i\n", machine().describe_context(), BIT(off, 0, 2));
	}
}

void leapster_snd_device::voice_volume_w(uint32_t off, uint16_t value)
{
	if(off % 1)
	{
		LOGMASKED(LOG_SOUND, "%s: voice_volume_w write odd halfword\n", machine().describe_context());
		return;
	}

	LOGMASKED(LOG_SOUND, "Set volume %i\n", value);

	m_volume[off >> 1] = value;
}

// 1.15 fixed point number giving "dx" value in bytes for the wave source each sampling tick
void leapster_snd_device::voice_pitch_w(uint32_t off, uint16_t value)
{
	if(off & 1)
	{
		LOGMASKED(LOG_SOUND, "%s: voice_pitch_w write odd halfword\n", machine().describe_context());
		return;
	}

	m_pitch[off >> 1] = value;
}

void leapster_snd_device::map(address_map &map)
{
	map(0x000, 0xfff).w(FUNC(leapster_snd_device::voice_generic_w));
	map(0x0c4, 0x103).w(FUNC(leapster_snd_device::voice_start_w));
	map(0x104, 0x13b).w(FUNC(leapster_snd_device::voice_end_w));
	map(0x13c, 0x15b).w(FUNC(leapster_snd_device::voice_volume_w));
	map(0x15c, 0x16f).w(FUNC(leapster_snd_device::voice_pitch_w));
}

void leapster_snd_device::device_start()
{
	m_stream = stream_alloc(0, 1, 8000);
}

void leapster_snd_device::sound_stream_update(sound_stream &stream)
{
	int mix_buf[8000];
	memset(mix_buf, 0, sizeof(mix_buf));

	for(int i = 0; i < 5; i++)
	{
		if(!m_channel_triggered[i])
		{
			continue;
		}

		float loop_point = BIT(m_data_source_end[i], 16, 16);
		float loop_target = BIT(m_data_source_end[i], 0, 16);
		auto curr = m_channel_index[i];
		auto dx = m_channel_dx[i];
		auto volume_scale = 0.125f * (((float) m_volume[i]) / 0x4000);

		for(int j = 0; j < stream.samples(); j++)
		{
			// This might be better converted to fixed point...
			uint32_t addr = (BIT(m_data_source_start[i], 16, 16) << 16) | ((uint16_t) curr);
			mix_buf[j] += conv_alaw_sample(m_space->read_byte(addr)) * volume_scale ;
			curr += dx;

			if(curr >= loop_point)
			{
				if(loop_point - loop_target < 2)
				{
					m_channel_triggered[i] = false;
					break;
				}
				else
				{
					curr = curr - loop_point + loop_target;
				}
			}
		}

		m_channel_index[i] = curr;
	}

	for(int i = 5; i < 7; i++)
	{
		if(!m_channel_triggered[i])
		{
			continue;
		}

		auto curr = m_channel_index[i];
		const auto dx = m_channel_dx[i];
		auto volume_scale = 0.125f * (((float) m_volume[i]) / 0x4000);

		for(int j = 0; j < stream.samples(); j++)
		{
			auto sample = conv_alaw_sample(m_space->read_byte(m_data_source_start[i] + ((uint32_t) curr)));
			mix_buf[j] += sample * volume_scale;
			curr += dx;

			if(m_data_source_start[i] + ((uint32_t) curr) >= m_data_source_end[i])
			{
				m_channel_triggered[i] = false;
				break;
			}
		}

		m_channel_index[i] = curr;
	}

	for(int i = 0; i < stream.samples(); i++)
	{
		stream.put_int_clamp(0, i, mix_buf[i], 32768);
	}
}
