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


#define LOG_UNKNOWN    (1 << 1)
#define LOG_HYPERVOICE (1 << 2)

#define LOG_ALL        (LOG_UNKNOWN | LOG_HYPERVOICE)

#define VERBOSE        (0)

#include "logmacro.h"

#define LOGUNKNOWN(...)    LOGMASKED(LOG_UNKNOWN, __VA_ARGS__)
#define LOGHYPERVOICE(...) LOGMASKED(LOG_HYPERVOICE, __VA_ARGS__)

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
		m_sweep_time(8192),
		m_sweep_count(0),
		m_noise_type(0),
		m_noise_enable(0),
		m_noise_output(0),
		m_sample_address(0),
		m_audio2_voice(0),
		m_audio3_sweep(0),
		m_audio4_noise(0),
		m_speaker_enable(0),
		m_speaker_volume(0),
		m_headphone_enable(0),
		m_headphone_connected(0),
		m_noise_shift(0),
		m_sample_volume(0),
		m_system_volume(0),
		m_loutput(0),
		m_routput(0)
{
}

static constexpr int clk_div = 128;

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
	save_item(NAME(m_noise_enable));
	save_item(NAME(m_sample_address));
	save_item(NAME(m_audio2_voice));
	save_item(NAME(m_audio3_sweep));
	save_item(NAME(m_audio4_noise));
	save_item(NAME(m_speaker_enable));
	save_item(NAME(m_speaker_volume));
	save_item(NAME(m_headphone_enable));
	save_item(NAME(m_headphone_connected));
	save_item(NAME(m_noise_shift));
	save_item(NAME(m_sample_volume));
	save_item(NAME(m_system_volume));
	save_item(NAME(m_loutput));
	save_item(NAME(m_routput));
	save_item(STRUCT_MEMBER(m_audio, freq));
	save_item(STRUCT_MEMBER(m_audio, period));
	save_item(STRUCT_MEMBER(m_audio, pos));
	save_item(STRUCT_MEMBER(m_audio, vol_left));
	save_item(STRUCT_MEMBER(m_audio, vol_right));
	save_item(STRUCT_MEMBER(m_audio, on));
	save_item(STRUCT_MEMBER(m_audio, offset));
	save_item(STRUCT_MEMBER(m_audio, signal));
	save_item(NAME(m_hypervoice.loutput));
	save_item(NAME(m_hypervoice.routput));
	save_item(NAME(m_hypervoice.linput));
	save_item(NAME(m_hypervoice.rinput));
	save_item(NAME(m_hypervoice.input_channel));
	save_item(NAME(m_hypervoice.volume));
	save_item(NAME(m_hypervoice.scale_mode));
	save_item(NAME(m_hypervoice.div));
	save_item(NAME(m_hypervoice.counter));
	save_item(NAME(m_hypervoice.enable));
	save_item(NAME(m_hypervoice.channel_mode));
}

void wswan_sound_device::device_clock_changed()
{
	m_channel->set_sample_rate(clock() / clk_div);
}

void wswan_sound_device::rom_bank_pre_change()
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
		m_audio[i].on = false;
		m_audio[i].signal = 0;
		m_audio[i].offset = 0;
		m_audio[i].pos = 0;
	}
	m_noise_output = 0;
	m_hypervoice.enable = false;
	m_hypervoice.counter = 0;
	m_hypervoice.loutput = m_hypervoice.routput = m_hypervoice.linput = m_hypervoice.rinput = 0;
	m_hypervoice.input_channel = false;
}

u8 wswan_sound_device::fetch_sample(int channel, int offset)
{
	u16 const w = read_word(m_sample_address + ((channel & 3) << 4) + ((offset >> 1) & 0x0e));

	return (w >> ((offset & 0x03) * 4)) & 0x0f;
}

//-------------------------------------------------
//  sound_stream_update - handle a stream update
//-------------------------------------------------

void wswan_sound_device::sound_stream_update(sound_stream &stream)
{
	for (int sampindex = 0; sampindex < stream.samples(); sampindex++)
	{
		m_loutput = m_routput = 0;
		if (m_headphone_connected ? m_headphone_enable : m_speaker_enable)
		{
			if (m_audio[0].on)
			{
				u8 const sample = m_audio[0].signal;
				m_audio[0].pos += clk_div;
				while (m_audio[0].pos >= m_audio[0].period)
				{
					m_audio[0].pos -= m_audio[0].period;
					m_audio[0].signal = fetch_sample(0, m_audio[0].offset++);
				}
				m_loutput += m_audio[0].vol_left * sample;
				m_routput += m_audio[0].vol_right * sample;
			}

			if (m_audio[1].on)
			{
				if (m_audio2_voice)
				{
					u8 const voice_data = (m_audio[1].vol_left << 4) | m_audio[1].vol_right;
					if (m_sample_volume & 0xc)
						m_loutput += voice_data >> u8(BIT(m_sample_volume, 3) ? 0 : (BIT(m_sample_volume, 2) ? 1 : ~0));
					if (m_sample_volume & 0x3)
						m_routput += voice_data >> u8(BIT(m_sample_volume, 1) ? 0 : (BIT(m_sample_volume, 0) ? 1 : ~0));
				}
				else
				{
					u8 const sample = m_audio[1].signal;
					m_audio[1].pos += clk_div;
					while (m_audio[1].pos >= m_audio[1].period)
					{
						m_audio[1].pos -= m_audio[1].period;
						m_audio[1].signal = fetch_sample(1, m_audio[1].offset++);
					}
					m_loutput += m_audio[1].vol_left * sample;
					m_routput += m_audio[1].vol_right * sample;
				}
			}

			if (m_audio[2].on)
			{
				u8 const sample = m_audio[2].signal;
				m_audio[2].pos += clk_div;
				while (m_audio[2].pos >= m_audio[2].period)
				{
					m_audio[2].pos -= m_audio[2].period;
					m_audio[2].signal = fetch_sample(2, m_audio[2].offset++);
				}
				if (m_audio3_sweep && m_sweep_time)
				{
					m_sweep_count += clk_div;
					while (m_sweep_count >= m_sweep_time)
					{
						m_sweep_count -= m_sweep_time;
						m_audio[2].freq += m_sweep_step;
						m_audio[2].freq &= 0x7ff;
						m_audio[2].period = 2048 - m_audio[2].freq;
					}
				}
				m_loutput += m_audio[2].vol_left * sample;
				m_routput += m_audio[2].vol_right * sample;
			}

			if (m_audio[3].on)
			{
				u8 const sample = m_audio[3].signal;
				m_audio[3].pos += clk_div;
				while (m_audio[3].pos >= m_audio[3].period)
				{
					if (m_audio4_noise)
						m_audio[3].signal = m_noise_output ? 0xf : 0;
					else
						m_audio[3].signal = fetch_sample(3, m_audio[3].offset++);

					m_audio[3].pos -= m_audio[3].period;

					if (m_noise_enable)
					{
						static const int shift_bit[8] = { 14, 10, 13, 4, 8, 6, 9, 11 };

						m_noise_output = (1 ^ (m_noise_shift >> 7) ^ (m_noise_shift >> shift_bit[m_noise_type])) & 1;
						m_noise_shift = m_noise_shift << 1 | m_noise_output;
					}
				}
				m_loutput += m_audio[3].vol_left * sample;
				m_routput += m_audio[3].vol_right * sample;
			}
			if (m_headphone_connected)
			{
				s32 left = m_loutput << 5;
				s32 right = m_routput << 5;
				if (m_hypervoice.enable)
				{
					static const u8 hypervoice_div[8] = { 1, 2, 3, 4, 5, 6, 8, 12 };

					left += m_hypervoice.loutput;
					right += m_hypervoice.routput;
					if (++m_hypervoice.counter >= hypervoice_div[m_hypervoice.div])
					{
						m_hypervoice.loutput = m_hypervoice.scale(m_hypervoice.linput);
						m_hypervoice.routput = m_hypervoice.scale(m_hypervoice.rinput);
						m_hypervoice.counter = 0;
					}
				}
				// TODO: clamped?
				stream.put_int_clamp(0, sampindex, left, 32768);
				stream.put_int_clamp(1, sampindex, right, 32768);
			}
			else
			{
				u8 const mono = (((m_loutput & 0x3ff) + (m_routput & 0x3ff)) >> m_speaker_volume) & 0xff;
				stream.put_int(0, sampindex, mono, 256);
				stream.put_int(1, sampindex, mono, 256);
			}
		}
		else
		{
			stream.put(0, sampindex, 0.0);
			stream.put(1, sampindex, 0.0);
		}
	}
}


void wswan_sound_device::hypervoice_t::stereo_input(u8 input)
{
	if (input_channel) // Right input
		rinput = input;
	else // Left input
		linput = input;
	input_channel = !input_channel;
}


s32 wswan_sound_device::hypervoice_t::scale(u8 input)
{
	s32 output = 0;
	switch (scale_mode)
	{
		case 0x00: // Unsigned
			output = input << 8;
			break;
		case 0x01: // Unsigned negated
			output = 0xffff0000 | (input << 8);
			break;
		case 0x02: // Signed
			output = s32(s8(input)) << 8;
			break;
		case 0x03: // None
			output = s16(input << 8);
			break;
	}
	return (scale_mode == 0x03) ? output : (output >> volume);
}


void wswan_sound_device::hypervoice_dma_w(u8 data)
{
	switch (m_hypervoice.channel_mode)
	{
		case 0x00: // Stereo
			m_hypervoice.stereo_input(data);
			break;
		case 0x01: // Mono, Left
			m_hypervoice.linput = data;
			break;
		case 0x02: // Mono, Right
			m_hypervoice.rinput = data;
			break;
		case 0x03: // Mono, Both
			m_hypervoice.linput = m_hypervoice.rinput = data;
			break;
	}
}


u16 wswan_sound_device::hypervoice_r(offs_t offset, u16 mem_mask)
{
	offset += 0x64 / 2;
	m_channel->update();
	switch (offset)
	{
		case 0x6a / 2: // Control
			return (m_hypervoice.volume) |
				(m_hypervoice.scale_mode << 2) |
				(m_hypervoice.div << 4) |
				(m_hypervoice.enable ? 0x0080 : 0x0000) |
				(m_hypervoice.channel_mode << 13);
		default:
			if (!machine().side_effects_disabled())
				LOGHYPERVOICE("%s: Unknown hypervoice port read %02x & %04x", machine().describe_context(), offset << 1, mem_mask);
			break;
	}
	return 0;
}

void wswan_sound_device::hypervoice_w(offs_t offset, u16 data, u16 mem_mask)
{
	offset += 0x64 / 2;
	m_channel->update();

	switch (offset)
	{
		case 0x64 / 2: // Left output
			COMBINE_DATA(&m_hypervoice.loutput);
			break;
		case 0x66 / 2: // Right output
			COMBINE_DATA(&m_hypervoice.routput);
			break;
		case 0x68 / 2: // Input
			if (ACCESSING_BITS_8_15)
			{
				m_hypervoice.stereo_input((data >> 8) & 0xff);
			}
			break;
		case 0x6a / 2: // Control
			if (ACCESSING_BITS_0_7)
			{
				m_hypervoice.volume = data & 3;
				m_hypervoice.scale_mode = (data >> 2) & 3;
				m_hypervoice.div = (data >> 4) & 7;
				m_hypervoice.enable = BIT(data, 7);
			}
			if (ACCESSING_BITS_8_15)
			{
				m_hypervoice.channel_mode = (data >> 13) & 3;
				if (BIT(data, 12))
					m_hypervoice.input_channel = false;
			}
			break;
		default:
			LOGHYPERVOICE("%s: Unknown hypervoice port write %02x - %04x & %04x", machine().describe_context(), offset << 1, data, mem_mask);
			break;
	}
}


u16 wswan_sound_device::port_r(offs_t offset, u16 mem_mask)
{
	m_channel->update();
	switch (offset)
	{
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
			return m_noise_type | (m_noise_enable ? 0x10 : 0x00) |
				((m_sample_address << 2) & 0xff00);
		case 0x90 / 2:
			return (m_audio[0].on ? 0x01 : 0x00) |
				(m_audio[1].on ? 0x02 : 0x00) |
				(m_audio[2].on ? 0x04 : 0x00) |
				(m_audio[3].on ? 0x08 : 0x00) |
				(m_audio2_voice ? 0x20 : 0x00) |
				(m_audio3_sweep ? 0x40 : 0x00) |
				(m_audio4_noise ? 0x80 : 0x00) |
				(m_speaker_enable ? 0x0100 : 0x0000) | (m_speaker_volume << 9) |
				(m_headphone_enable ? 0x0800 : 0x0000) |
				(m_headphone_connected ? 0x8000 : 0x0000);
		case 0x92 / 2:
			return m_noise_shift;
		case 0x94 / 2:
			return m_sample_volume;
		case 0x96 / 2:
			return m_loutput & 0x3ff;
		case 0x98 / 2:
			return m_routput & 0x3ff;
		case 0x9a / 2:
			return (m_loutput & 0x3ff) + (m_routput & 0x3ff);
		case 0x9e / 2:
			return m_system_volume;
		default:
			if (!machine().side_effects_disabled())
				LOGUNKNOWN("%s: Unknown sound port read %02x & %04x", machine().describe_context(), offset << 1, mem_mask);
			break;
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
				m_sweep_step = s8(data & 0xff);
			}
			// Sweep time
			if (ACCESSING_BITS_8_15)
			{
				m_sweep_time = 8192 * (((data >> 8) & 0x1f) + 1);
			}
			break;

		case 0x8e / 2:
			// Noise control
			if (ACCESSING_BITS_0_7)
			{
				m_noise_type = data & 0x07;
				m_noise_enable = BIT(data, 4);
				if (BIT(data, 3))
					m_noise_shift = m_noise_output = 0;
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
				m_speaker_enable = BIT(data, 8);
				m_speaker_volume = ((data >> 9) & 0x03);
				m_headphone_enable = BIT(data, 11);
			}
			break;

		case 0x92 / 2:              // Noise counter shift register
			COMBINE_DATA(&m_noise_shift);
			m_noise_shift &= 0x7fff;
			break;

		case 0x94 / 2:              // Sample volume
			if (ACCESSING_BITS_0_7)
				m_sample_volume = data & 0x0f;
			if (ACCESSING_BITS_8_15)
				LOGUNKNOWN("%s: Sound Test bit set %02x\n", machine().describe_context(), (data >> 8) & 0xff);
			break;

		case 0x9e / 2:              // WSC volume setting (0, 1, 2, 3) (TODO)
			if (ACCESSING_BITS_0_7)
				m_system_volume = data & 0x03;
			break;
		default:
			LOGUNKNOWN("%s: Unknown hypervoice port write %02x - %04x & %04x", machine().describe_context(), offset << 1, data, mem_mask);
			break;
	}
}
