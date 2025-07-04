// license:BSD-3-Clause
// copyright-holders:windyfairy
/**********************************************************************

    Sanyo LC82310 MP3 decoder

**********************************************************************/

#include "emu.h"
#include "lc82310.h"
#include "mp3_audio.h"

DEFINE_DEVICE_TYPE(LC82310, lc82310_device, "lc82310", "Sanyo LC82310 MP3 Decoder")

lc82310_device::lc82310_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, LC82310, tag, owner, clock)
	, device_sound_interface(mconfig, *this)
{
}

void lc82310_device::device_start()
{
	stream = stream_alloc(0, 2, 44100);
	mp3dec = std::make_unique<mp3_audio>(reinterpret_cast<const uint8_t *>(&mp3data[0]));

	save_item(NAME(mp3data));
	save_item(NAME(samples));
	save_item(NAME(m_mp3data_count));
	save_item(NAME(m_sample_count));
	save_item(NAME(m_samples_idx));
	save_item(NAME(m_frame_channels));
	save_item(NAME(m_output_gain));

	save_item(NAME(m_csctl));
	save_item(NAME(m_ckctl));
	save_item(NAME(m_dictl));
	save_item(NAME(m_doctl));
	save_item(NAME(m_ctl_state));
	save_item(NAME(m_ctl_cmd));
	save_item(NAME(m_ctl_bits));
	save_item(NAME(m_ctl_byte));
	save_item(NAME(m_ctl_out_byte));

	mp3dec->register_save(*this);
}

void lc82310_device::device_reset()
{
	std::fill(std::begin(m_output_gain), std::end(m_output_gain), 0);

	m_csctl = 0;
	m_ckctl = 0;
	m_dictl = 0;
	m_doctl = 0;
	m_ctl_bits = 0;
	m_ctl_byte = 0;
	m_ctl_out_byte = 0;
	m_ctl_state = ACCEPTING_CMD;

	reset_playback();
}

void lc82310_device::reset_playback()
{
	std::fill(mp3data.begin(), mp3data.end(), 0);
	std::fill(samples.begin(), samples.end(), 0);

	mp3dec->clear();
	m_mp3data_count = 0;
	m_sample_count = 0;
	m_samples_idx = 0;
	m_frame_channels = 2;
}

void lc82310_device::zcsctl_w(int state)
{
	m_csctl = state;
}

void lc82310_device::ckctl_w(int state)
{
	if (m_csctl == 0 && m_ckctl == 0 && state == 1)
	{
		m_ctl_byte |= m_dictl << m_ctl_bits;
		m_ctl_bits++;

		if (m_ctl_bits > 7)
		{
			if (m_ctl_state == ACCEPTING_CMD)
			{
				// Expected to be able to read the return value while sending the second byte
				// Everything in the 0x80 range of commands seems to respond with a value
				// 0x80, 0x81, 0x82 return separate 8-bit values
				// 0x83 returns an 8-bit error status, bits 0 and 1 are used to signal errors, any non-0 bit is considered an error
				// 0x84 and 0x85 are used together to form a 16-bit value
				m_ctl_out_byte = 0;

				m_ctl_cmd = m_ctl_byte;
				m_ctl_state = ACCEPTING_PARAM;
			}
			else if (m_ctl_state == ACCEPTING_PARAM)
			{
				handle_command(m_ctl_cmd, m_ctl_byte);
				m_ctl_state = ACCEPTING_CMD;
			}

			m_ctl_byte = 0;
			m_ctl_bits = 0;
		}

		m_doctl = m_ctl_out_byte & 1;
		m_ctl_out_byte >>= 1;
	}

	m_ckctl = state;
}

void lc82310_device::dictl_w(int state)
{
	m_dictl = state;
}

int lc82310_device::doctl_r()
{
	return m_doctl;
}

int lc82310_device::demand_r()
{
	return m_mp3data_count < mp3data.size();
}

void lc82310_device::dimpg_w(uint8_t data)
{
	if (m_mp3data_count >= mp3data.size())
	{
		// Drop a byte if the buffer is full and it's still trying to send data
		std::copy(mp3data.begin() + 1, mp3data.end(), mp3data.begin());
		m_mp3data_count--;
	}

	mp3data[m_mp3data_count++] = data;
}

void lc82310_device::handle_command(uint8_t cmd, uint8_t param)
{
	if (cmd == CMD_UNK13_VOL || cmd == CMD_UNK15_VOL)
	{
		// These are calculated based on the configurable values in-game vs what is sent to the MP3 decoder
		constexpr float gain_table[] = {
			1.0,                          // 0
			30.0 / 31.0,                  // 1
			29.0 / 31.0,                  // 2
			28.0 / 31.0,                  // 3
			27.0 / 31.0,                  // 4
			26.0 / 31.0,                  // 5
			25.0 / 31.0,                  // 6
			24.0 / 31.0,                  // 7
			23.0 / 31.0,                  // 8
			22.0 / 31.0,                  // 9
			21.0 / 31.0,                  // 10
			20.0 / 31.0,                  // 11
			19.0 / 31.0,                  // 12
			18.0 / 31.0,                  // 13
			17.0 / 31.0,                  // 14
			16.0 / 31.0,                  // 15
			15.0 / 31.0,                  // 16
			14.0 / 31.0,                  // 17
			13.0 / 31.0,                  // 18
			12.0 / 31.0,                  // 19
			11.0 / 31.0,                  // 20
			10.0 / 31.0,                  // 21
			9.0 / 31.0,                   // 22
			8.5 / 31.0,                   // 23
			8.0 / 31.0,                   // 24
			7.0 / 31.0,                   // 25
			6.0 / 31.0,                   // 25
			5.5 / 31.0,                   // 27
			5.0 / 31.0,                   // 28
			4.5 / 31.0,                   // 29
			4.0 / 31.0,                   // 30
			3.5 / 31.0,                   // 31
			3.0 / 31.0,                   // 32
			2.75 / 31.0,                  // 33
			2.5 / 31.0,                   // 34
			2.25 / 31.0,                  // 35
			2.0 / 31.0,                   // 36
			(1.0 + (1.0 / 6) * 5) / 31.0, // 37
			(1.0 + (1.0 / 6) * 4) / 31.0, // 38
			(1.0 + (1.0 / 6) * 3) / 31.0, // 39
			(1.0 + (1.0 / 6) * 2) / 31.0, // 40
			(1.0 + (1.0 / 6) * 1) / 31.0, // 41
			1.0 / 31.0,                   // 42
			((1.0 / 34) * 33) / 31.0,     // 43
			((1.0 / 34) * 32) / 31.0,     // 44
			((1.0 / 34) * 31) / 31.0,     // 45
			((1.0 / 34) * 30) / 31.0,     // 46
			((1.0 / 34) * 29) / 31.0,     // 47
			((1.0 / 34) * 28) / 31.0,     // 48
			((1.0 / 34) * 27) / 31.0,     // 49
			((1.0 / 34) * 26) / 31.0,     // 50
			((1.0 / 34) * 25) / 31.0,     // 51
			((1.0 / 34) * 24) / 31.0,     // 52
			((1.0 / 34) * 23) / 31.0,     // 53
			((1.0 / 34) * 22) / 31.0,     // 54
			((1.0 / 34) * 21) / 31.0,     // 55
			((1.0 / 34) * 20) / 31.0,     // 56
			((1.0 / 34) * 19) / 31.0,     // 57
			((1.0 / 34) * 18) / 31.0,     // 58
			((1.0 / 34) * 17) / 31.0,     // 59
			((1.0 / 34) * 16) / 31.0,     // 60
			((1.0 / 34) * 15) / 31.0,     // 61
			((1.0 / 34) * 14) / 31.0,     // 62
			((1.0 / 34) * 13) / 31.0,     // 63
			((1.0 / 34) * 12) / 31.0,     // 64
			((1.0 / 34) * 11) / 31.0,     // 65
			((1.0 / 34) * 10) / 31.0,     // 66
			((1.0 / 34) * 9) / 31.0,      // 67
			((1.0 / 34) * 8) / 31.0,      // 68
			((1.0 / 34) * 7) / 31.0,      // 69
			((1.0 / 34) * 6) / 31.0,      // 70
			((1.0 / 34) * 5) / 31.0,      // 71
			((1.0 / 34) * 4) / 31.0,      // 72
			((1.0 / 34) * 3) / 31.0,      // 73
			((1.0 / 34) * 2) / 31.0,      // 74
			((1.0 / 34) * 1) / 31.0,      // 75
			0.0,                          // 76
		};

		int speaker_idx = cmd == CMD_UNK15_VOL ? 1 : 0; // guessed, both are set at the same time in current use cases
		m_output_gain[speaker_idx] = gain_table[std::min<uint8_t>(param, 0x4c)];

		set_output_gain(speaker_idx, m_output_gain[speaker_idx]);
	}
}

void lc82310_device::fill_buffer()
{
	int pos = 0, frame_sample_rate = 0;
	bool decoded_frame = mp3dec->decode_buffer(pos, m_mp3data_count, &samples[0], m_sample_count, frame_sample_rate, m_frame_channels);
	m_samples_idx = 0;

	if (!decoded_frame || m_sample_count <= 0)
	{
		// Frame decode failed
		if (m_mp3data_count >= mp3data.size())
		{
			std::copy(mp3data.begin() + 1, mp3data.end(), mp3data.begin());
			m_mp3data_count--;
		}

		return;
	}

	std::copy(mp3data.begin() + pos, mp3data.end(), mp3data.begin());
	m_mp3data_count -= pos;

	stream->set_sample_rate(frame_sample_rate);
}

void lc82310_device::append_buffer(sound_stream &stream, int &pos, int scount)
{
	int s1 = std::min(scount - pos, m_sample_count);
	int words_per_sample = std::min(m_frame_channels, 2);

	for (int i = 0; i < s1; i++)
	{
		stream.put_int(0, pos, samples[m_samples_idx * words_per_sample], 32768);
		stream.put_int(1, pos, samples[m_samples_idx * words_per_sample + (words_per_sample >> 1)], 32768);

		m_samples_idx++;
		pos++;

		if (m_samples_idx >= m_sample_count)
		{
			m_sample_count = 0;
			return;
		}
	}
}

void lc82310_device::sound_stream_update(sound_stream &stream)
{
	int csamples = stream.samples();
	int pos = 0;

	while (pos < csamples)
	{
		if (m_sample_count == 0)
			fill_buffer();

		if (m_sample_count <= 0)
			return;

		append_buffer(stream, pos, csamples);
	}
}
