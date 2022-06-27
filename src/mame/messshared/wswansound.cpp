// license:BSD-3-Clause
// copyright-holders:Wilbert Pol
/**************************************************************************************

  Wonderswan sound emulation

  Wilbert Pol

  Sound emulation is preliminary and not complete


The noise taps and behavior are the same as the Virtual Boy.

**************************************************************************************/

#include "emu.h"
#include "wswansound.h"


// device type definition
DEFINE_DEVICE_TYPE(WSWAN_SND, wswan_sound_device, "wswan_sound", "WonderSwan Custom Sound")


//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  wswan_sound_device - constructor
//-------------------------------------------------

wswan_sound_device::wswan_sound_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, WSWAN_SND, tag, owner, clock),
		device_sound_interface(mconfig, *this),
		device_rom_interface(mconfig, *this),
		m_channel(nullptr),
		m_sweep_step(0),
		m_sweep_time(0),
		m_sweep_count(0),
		m_noise_type(0),
		m_noise_reset(0),
		m_noise_enable(0),
		m_noise_output(0),
		m_sample_address(0),
		m_audio2_voice(0),
		m_audio3_sweep(0),
		m_audio4_noise(0),
		m_mono(0),
		m_output_volume(0),
		m_external_stereo(0),
		m_external_speaker(0),
		m_noise_shift(0),
		m_master_volume(0)
{
}

static constexpr int clk_div = 64;

//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void wswan_sound_device::device_start()
{
	m_channel = stream_alloc(0, 2, clock() / clk_div);

	save_item(NAME(m_sweep_step));
	save_item(NAME(m_sweep_time));
	save_item(NAME(m_sweep_count));
	save_item(NAME(m_noise_type));
	save_item(NAME(m_noise_reset));
	save_item(NAME(m_noise_enable));
	save_item(NAME(m_sample_address));
	save_item(NAME(m_audio2_voice));
	save_item(NAME(m_audio3_sweep));
	save_item(NAME(m_audio4_noise));
	save_item(NAME(m_mono));
	save_item(NAME(m_output_volume));
	save_item(NAME(m_external_stereo));
	save_item(NAME(m_external_speaker));
	save_item(NAME(m_noise_shift));
	save_item(NAME(m_master_volume));
	save_item(NAME(m_system_volume));
	save_item(STRUCT_MEMBER(m_audio, freq));
	save_item(STRUCT_MEMBER(m_audio, period));
	save_item(STRUCT_MEMBER(m_audio, pos));
	save_item(STRUCT_MEMBER(m_audio, vol_left));
	save_item(STRUCT_MEMBER(m_audio, vol_right));
	save_item(STRUCT_MEMBER(m_audio, on));
	save_item(STRUCT_MEMBER(m_audio, offset));
	save_item(STRUCT_MEMBER(m_audio, signal));
}

void wswan_sound_device::device_clock_changed()
{
	m_channel->set_sample_rate(clock() / clk_div);
}

void wswan_sound_device::rom_bank_updated()
{
	m_channel->update();
}

//-------------------------------------------------
//  device_reset
//-------------------------------------------------

void wswan_sound_device::device_reset()
{
	for (int i = 0; i < 4; i++)
	{
		m_audio[i].on = 0;
		m_audio[i].signal = 0;
		m_audio[i].offset = 0;
		m_audio[i].pos = 0;
	}
	m_noise_output = 0;
}

int wswan_sound_device::fetch_sample(int channel, int offset)
{
	u16 w = read_word(m_sample_address + ((channel & 3) << 4) + ((offset >> 1) & 0x0e));

	return (w >> ((offset & 0x03) * 4)) & 0x0f;
}

//-------------------------------------------------
//  sound_stream_update - handle a stream update
//-------------------------------------------------

void wswan_sound_device::sound_stream_update(sound_stream &stream, std::vector<read_stream_view> const &inputs, std::vector<write_stream_view> &outputs)
{
	auto &outputl = outputs[0];
	auto &outputr = outputs[1];
	for (int sampindex = 0; sampindex < outputl.samples(); sampindex++)
	{
		s32 left = 0, right = 0;

		if (m_audio[0].on)
		{
			s32 sample = m_audio[0].signal;
			m_audio[0].pos += clk_div;
			if (m_audio[0].pos >= m_audio[0].period)
			{
				m_audio[0].pos -= m_audio[0].period;
				m_audio[0].signal = fetch_sample(0, m_audio[0].offset++);
			}
			left += m_audio[0].vol_left * sample;
			right += m_audio[0].vol_right * sample;
		}

		if (m_audio[1].on)
		{
			if (m_audio2_voice)
			{
				u8 voice_data = (m_audio[1].vol_left << 4) | m_audio[1].vol_right;
				left += voice_data * (m_master_volume & 0x0f);
				right += voice_data * (m_master_volume & 0x0f);
			}
			else
			{
				s32 sample = m_audio[1].signal;
				m_audio[1].pos += clk_div;
				if (m_audio[1].pos >= m_audio[1].period)
				{
					m_audio[1].pos -= m_audio[1].period;
					m_audio[1].signal = fetch_sample(1, m_audio[1].offset++);
				}
				left += m_audio[1].vol_left * sample;
				right += m_audio[1].vol_right * sample;
			}
		}

		if (m_audio[2].on)
		{
			s32 sample = m_audio[2].signal;
			m_audio[2].pos += clk_div;
			if (m_audio[2].pos >= m_audio[2].period)
			{
				m_audio[2].pos -= m_audio[2].period;
				m_audio[2].signal = fetch_sample(2, m_audio[2].offset++);
			}
			if (m_audio3_sweep && m_sweep_time)
			{
				m_sweep_count += clk_div;
				if (m_sweep_count >= m_sweep_time)
				{
					m_sweep_count -= m_sweep_time;
					m_audio[2].freq += m_sweep_step;
					m_audio[2].freq &= 0x7ff;
					m_audio[2].period = 2048 - m_audio[2].freq;
				}
			}
			left += m_audio[2].vol_left * sample;
			right += m_audio[2].vol_right * sample;
		}

		if (m_audio[3].on)
		{
			s32 sample = m_audio[3].signal;
			m_audio[3].pos += clk_div;
			if (m_audio[3].pos >= m_audio[3].period)
			{
				if (m_audio4_noise)
					m_audio[3].signal = m_noise_output ? 0xf : 0;
				else
					m_audio[3].signal = fetch_sample(3, m_audio[3].offset++);

				m_audio[3].pos -= m_audio[3].period;

				if (m_noise_reset)
				{
					m_noise_reset = 0;
					m_noise_shift = 0;
					m_noise_output = 0;
				}

				if (m_noise_enable)
				{
					static int shift_bit[] = { 14, 10, 13, 4, 8, 6, 9, 11 };

					m_noise_output = (1 ^ (m_noise_shift >> 7) ^ (m_noise_shift >> shift_bit[m_noise_type])) & 1;
					m_noise_shift = m_noise_shift << 1 | m_noise_output;
				}
			}
			left += m_audio[3].vol_left * sample;
			right += m_audio[3].vol_right * sample;
		}

		outputl.put_int(sampindex, left, 32768 >> 5);
		outputr.put_int(sampindex, right, 32768 >> 5);
	}
}


u16 wswan_sound_device::port_r(offs_t offset, u16 mem_mask)
{
	m_channel->update();
	switch (offset) {
		case 0x80 / 2:
		case 0x82 / 2:
		case 0x84 / 2:
		case 0x86 / 2:
			return m_audio[offset & 0x03].freq;
		case 0x88 / 2:
			return (m_audio[0].vol_left << 4) | m_audio[0].vol_right |
				(m_audio[1].vol_left << 12) | (m_audio[1].vol_right << 8);
		case 0x8a / 2:
			return (m_audio[2].vol_left << 4) | m_audio[2].vol_right |
				(m_audio[3].vol_left << 12) | (m_audio[3].vol_right << 8);
		case 0x8c / 2:
			return m_sweep_step | (((m_sweep_time / 8192) - 1) << 8);
		case 0x8e / 2:
			return m_noise_type | (m_noise_reset ? 0x08 : 0x00) | (m_noise_enable ? 0x10 : 0x00) |
				((m_sample_address << 2) & 0xff00);
		case 0x90 / 2:
			return (m_audio[0].on ? 0x01 : 0x00) |
				(m_audio[1].on ? 0x02 : 0x00) |
				(m_audio[2].on ? 0x04 : 0x00) |
				(m_audio[3].on ? 0x08 : 0x00) |
				(m_audio2_voice ? 0x20 : 0x00) |
				(m_audio3_sweep ? 0x40 : 0x00) |
				(m_audio4_noise ? 0x80 : 0x00) |
				(m_mono ? 0x0100 : 0x00) | (m_output_volume << 9) |
				(m_external_stereo ? 0x0800 : 0x00) |
				(m_external_speaker ? 0x00 : 0x00); // TODO 0x80 is set when external speaker is connected
		case 0x92 / 2:
			return m_noise_shift;
		case 0x94 / 2:
			return m_master_volume;
		case 0x9e / 2:
			return m_system_volume;
	}
	return 0;
}

void wswan_sound_device::port_w(offs_t offset, u16 data, u16 mem_mask)
{
	m_channel->update();

	switch (offset)
	{
		case 0x80 / 2:              // Audio 1 freq
		case 0x82 / 2:              // Audio 2 freq
		case 0x84 / 2:              // Audio 3 freq
		case 0x86 / 2:              // Audio 4 freq
			COMBINE_DATA(&m_audio[offset & 0x03].freq);
			m_audio[offset & 0x03].freq &= 0x7ff;
			m_audio[offset & 0x03].period = 2048 - m_audio[offset & 0x03].freq;
			break;

		case 0x88 / 2:
			// Audio 1 volume
			if (ACCESSING_BITS_0_7)
			{
				m_audio[0].vol_left = (data >> 4) & 0x0f;
				m_audio[0].vol_right = data & 0x0f;
			}
			// Audio 2 volume
			if (ACCESSING_BITS_8_15)
			{
				m_audio[1].vol_left = (data >> 12) & 0x0f;
				m_audio[1].vol_right = (data >> 8) & 0x0f;
			}
			break;

		case 0x8a / 2:
		  // Audio 3 volume
			if (ACCESSING_BITS_0_7)
			{
				m_audio[2].vol_left = (data >> 4) & 0x0f;
				m_audio[2].vol_right = data & 0x0f;
			}
			// Audio 4 volume
			if (ACCESSING_BITS_8_15)
			{
				m_audio[3].vol_left = (data >> 12) & 0x0f;
				m_audio[3].vol_right = (data >> 8) & 0x0f;
			}
			break;

		case 0x8c / 2:
			// Sweep step
			if (ACCESSING_BITS_0_7)
			{
				m_sweep_step = (int8_t)(data & 0xff);
			}
			// Sweep time
			if (ACCESSING_BITS_8_15)
			{
				m_sweep_time = 8192 * ((data >> 8) + 1);
			}
			break;

		case 0x8e / 2:
			// Noise control
			if (ACCESSING_BITS_0_7)
			{
				m_noise_type = data & 0x07;
				m_noise_reset = BIT(data, 3);
				m_noise_enable = BIT(data, 4);
			}
			// Sample location
			if (ACCESSING_BITS_8_15)
			{
				m_sample_address = (data & 0xff00) >> 2;
			}
			break;

		case 0x90 / 2:
			// Audio control
			if (ACCESSING_BITS_0_7)
			{
				m_audio[0].on = BIT(data, 0);
				m_audio[1].on = BIT(data, 1);
				m_audio[2].on = BIT(data, 2);
				m_audio[3].on = BIT(data, 3);
				m_audio2_voice = BIT(data, 5);
				m_audio3_sweep = BIT(data, 6);
				m_audio4_noise = BIT(data, 7);
			}
			// Audio output
			if (ACCESSING_BITS_8_15)
			{
				m_mono = BIT(data, 8);
				m_output_volume = ((data >> 9) & 0x03);
				m_external_stereo = BIT(data, 11);
				m_external_speaker = 1;
			}
			break;

		case 0x92 / 2:              // Noise counter shift register
			COMBINE_DATA(&m_noise_shift);
			m_noise_shift &= 0x7fff;
			break;

		case 0x94 / 2:              // Master volume
			if (ACCESSING_BITS_0_7)
				m_master_volume = data & 0xff;
			break;

		case 0x9e / 2:              // WSC volume setting (0, 1, 2, 3) (TODO)
			if (ACCESSING_BITS_0_7)
				m_system_volume = data & 0x03;
			break;
	}
}
