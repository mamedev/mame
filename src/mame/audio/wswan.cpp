// license:BSD-3-Clause
// copyright-holders:Wilbert Pol
/**************************************************************************************

  Wonderswan sound emulation

  Wilbert Pol

  Sound emulation is preliminary and not complete


The noise taps and behavior are the same as the Virtual Boy.

**************************************************************************************/

#include "emu.h"
#include "wswan.h"


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
		device_rom_interface(mconfig, *this, 14),
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

constexpr int clk_div = 64;

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

	save_item(NAME(m_audio1.freq));
	save_item(NAME(m_audio1.period));
	save_item(NAME(m_audio1.pos));
	save_item(NAME(m_audio1.vol_left));
	save_item(NAME(m_audio1.vol_right));
	save_item(NAME(m_audio1.on));
	save_item(NAME(m_audio1.offset));
	save_item(NAME(m_audio1.signal));

	save_item(NAME(m_audio2.freq));
	save_item(NAME(m_audio2.period));
	save_item(NAME(m_audio2.pos));
	save_item(NAME(m_audio2.vol_left));
	save_item(NAME(m_audio2.vol_right));
	save_item(NAME(m_audio2.on));
	save_item(NAME(m_audio2.offset));
	save_item(NAME(m_audio2.signal));

	save_item(NAME(m_audio3.freq));
	save_item(NAME(m_audio3.period));
	save_item(NAME(m_audio3.pos));
	save_item(NAME(m_audio3.vol_left));
	save_item(NAME(m_audio3.vol_right));
	save_item(NAME(m_audio3.on));
	save_item(NAME(m_audio3.offset));
	save_item(NAME(m_audio3.signal));

	save_item(NAME(m_audio4.freq));
	save_item(NAME(m_audio4.period));
	save_item(NAME(m_audio4.pos));
	save_item(NAME(m_audio4.vol_left));
	save_item(NAME(m_audio4.vol_right));
	save_item(NAME(m_audio4.on));
	save_item(NAME(m_audio4.offset));
	save_item(NAME(m_audio4.signal));
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
	m_audio1.on = 0;
	m_audio1.signal = 0;
	m_audio1.offset = 0;
	m_audio1.pos = 0;
	m_audio2.on = 0;
	m_audio2.signal = 0;
	m_audio2.offset = 0;
	m_audio2.pos = 0;
	m_audio3.on = 0;
	m_audio3.signal = 0;
	m_audio3.offset = 0;
	m_audio3.pos = 0;
	m_audio4.on = 0;
	m_audio4.signal = 0;
	m_audio4.offset = 0;
	m_audio4.pos = 0;
	m_noise_output = 0;
}

int wswan_sound_device::fetch_sample(int channel, int offset)
{
	uint8_t b = read_byte(m_sample_address + ((channel & 3) << 4) + ((offset & 0x1f) >> 1));

	if (offset & 1)
		return b >> 4;
	else
		return b & 0xf;
}

//-------------------------------------------------
//  sound_stream_update - handle a stream update
//-------------------------------------------------

void wswan_sound_device::sound_stream_update(sound_stream &stream, stream_sample_t **inputs, stream_sample_t **outputs, int samples)
{
	stream_sample_t sample, left, right;

	while( samples-- > 0 )
	{
		left = right = 0;

		if ( m_audio1.on )
		{
			sample = m_audio1.signal;
			m_audio1.pos += clk_div;
			if (m_audio1.pos >= m_audio1.period)
			{
				m_audio1.pos -= m_audio1.period;
				m_audio1.signal = fetch_sample(0, m_audio1.offset++);
			}
			left += m_audio1.vol_left * sample;
			right += m_audio1.vol_right * sample;
		}

		if ( m_audio2.on )
		{
			if ( m_audio2_voice )
			{
				uint8_t voice_data = m_audio2.vol_left << 4 | m_audio2.vol_right;
				left += voice_data * (m_master_volume & 0x0f);
				right += voice_data * (m_master_volume & 0x0f);
			}
			else
			{
				sample = m_audio2.signal;
				m_audio2.pos += clk_div;
				if (m_audio2.pos >= m_audio2.period)
				{
					m_audio2.pos -= m_audio2.period;
					m_audio2.signal = fetch_sample(1, m_audio2.offset++);
				}
				left += m_audio2.vol_left * sample;
				right += m_audio2.vol_right * sample;
			}
		}

		if ( m_audio3.on )
		{
			sample = m_audio3.signal;
			m_audio3.pos += clk_div;
			if (m_audio3.pos >= m_audio3.period)
			{
				m_audio3.pos -= m_audio3.period;
				m_audio3.signal = fetch_sample(2, m_audio3.offset++);
			}
			if ( m_audio3_sweep && m_sweep_time )
			{
				m_sweep_count += clk_div;
				if ( m_sweep_count >= m_sweep_time )
				{
					m_sweep_count -= m_sweep_time;
					m_audio3.freq += m_sweep_step;
					m_audio3.freq &= 0x7ff;
					m_audio3.period = 2048 - m_audio3.freq;
				}
			}
			left += m_audio3.vol_left * sample;
			right += m_audio3.vol_right * sample;
		}

		if ( m_audio4.on )
		{
			sample = m_audio4.signal;
			m_audio4.pos += clk_div;
			if (m_audio4.pos >= m_audio4.period)
			{
				if (m_audio4_noise)
					m_audio4.signal = m_noise_output ? 0xf : 0;
				else
					m_audio4.signal = fetch_sample(3, m_audio4.offset++);

				m_audio4.pos -= m_audio4.period;

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
			left += m_audio4.vol_left * sample;
			right += m_audio4.vol_right * sample;
		}

		left <<= 5;
		right <<= 5;

		*(outputs[0]++) = left;
		*(outputs[1]++) = right;
	}
}


void wswan_sound_device::wswan_ch_set_freq( CHAN *ch, uint16_t freq )
{
	freq &= 0x7ff;  // docs say freq is 11bits and a few games (Morita Shougi, World Stadium + others) write 0x800 causing a divide by 0 crash
	ch->freq = freq;
	ch->period = 2048 - freq;
}

void wswan_sound_device::port_w(offs_t offset, uint8_t data)
{
	m_channel->update();

	switch( offset )
	{
		case 0x80:              /* Audio 1 freq (lo) */
			wswan_ch_set_freq(&m_audio1, (m_audio1.freq & 0xff00) | data);
			break;

		case 0x81:              /* Audio 1 freq (hi) */
			wswan_ch_set_freq(&m_audio1, (data << 8 ) | (m_audio1.freq & 0x00ff));
			break;

		case 0x82:              /* Audio 2 freq (lo) */
			wswan_ch_set_freq(&m_audio2, (m_audio2.freq & 0xff00) | data);
			break;

		case 0x83:              /* Audio 2 freq (hi) */
			wswan_ch_set_freq(&m_audio2, (data << 8 ) | (m_audio2.freq & 0x00ff));
			break;

		case 0x84:              /* Audio 3 freq (lo) */
			wswan_ch_set_freq(&m_audio3, (m_audio3.freq & 0xff00) | data);
			break;

		case 0x85:              /* Audio 3 freq (hi) */
			wswan_ch_set_freq(&m_audio3, (data << 8) | (m_audio3.freq & 0x00ff));
			break;

		case 0x86:              /* Audio 4 freq (lo) */
			wswan_ch_set_freq(&m_audio4, (m_audio4.freq & 0xff00) | data);
			break;

		case 0x87:              /* Audio 4 freq (hi) */
			wswan_ch_set_freq(&m_audio4, (data << 8) | (m_audio4.freq & 0x00ff));
			break;

		case 0x88:              /* Audio 1 volume */
			m_audio1.vol_left = ( data & 0xF0 ) >> 4;
			m_audio1.vol_right = data & 0x0F;
			break;

		case 0x89:              /* Audio 2 volume */
			m_audio2.vol_left = ( data & 0xF0 ) >> 4;
			m_audio2.vol_right = data & 0x0F;
			break;

		case 0x8A:              /* Audio 3 volume */
			m_audio3.vol_left = ( data & 0xF0 ) >> 4;
			m_audio3.vol_right = data & 0x0F;
			break;

		case 0x8B:              /* Audio 4 volume */
			m_audio4.vol_left = ( data & 0xF0 ) >> 4;
			m_audio4.vol_right = data & 0x0F;
			break;

		case 0x8C:              /* Sweep step */
			m_sweep_step = (int8_t)data;
			break;

		case 0x8D:              /* Sweep time */
			m_sweep_time = 8192 * (data + 1);
			break;

		case 0x8E:              /* Noise control */
			m_noise_type = data & 0x07;
			m_noise_reset = ( data & 0x08 ) >> 3;
			m_noise_enable = ( data & 0x10 ) >> 4;
			break;

		case 0x8F:              /* Sample location */
			m_sample_address = data << 6;
			break;

		case 0x90:              /* Audio control */
			m_audio1.on = data & 0x01;
			m_audio2.on = ( data & 0x02 ) >> 1;
			m_audio3.on = ( data & 0x04 ) >> 2;
			m_audio4.on = ( data & 0x08 ) >> 3;
			m_audio2_voice = ( data & 0x20 ) >> 5;
			m_audio3_sweep = ( data & 0x40 ) >> 6;
			m_audio4_noise = ( data & 0x80 ) >> 7;
			break;

		case 0x91:              /* Audio output */
			m_mono = data & 0x01;
			m_output_volume = ( data & 0x06 ) >> 1;
			m_external_stereo = ( data & 0x08 ) >> 3;
			m_external_speaker = 1;
			break;

		case 0x92:              /* Noise counter shift register (lo) */
			m_noise_shift = ( m_noise_shift & 0xFF00 ) | data;
			break;

		case 0x93:              /* Noise counter shift register (hi) */
			m_noise_shift = ( ( data & 0x7f ) << 8 ) | ( m_noise_shift & 0x00FF );
			break;

		case 0x94:              /* Master volume */
			m_master_volume = data;
			break;
	}
}
