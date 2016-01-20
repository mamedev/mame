// license:BSD-3-Clause
// copyright-holders:Charles MacDonald
/*
    HuC6280 sound chip emulator
    by Charles MacDonald
    E-mail: cgfm2@hotmail.com
    WWW: http://cgfm2.emuviews.com

    Thanks to:

    - Paul Clifford for his PSG documentation.
    - Richard Bannister for the TGEmu-specific sound updating code.
    - http://www.uspto.gov for the PSG patents.
    - All contributors to the tghack-list.

    Changes:

    (03/30/2003)
    - Removed TGEmu specific code and added support functions for MAME.
    - Modified setup code to handle multiple chips with different clock and
      volume settings.

    Missing features / things to do:

    - Add LFO support. But do any games actually use it?

    - Add shared index for waveform playback and sample writes. Almost every
      game will reset the index prior to playback so this isn't an issue.

    - While the noise emulation is complete, the data for the pseudo-random
      bitstream is calculated by machine.rand() and is not a representation of what
      the actual hardware does.

    For some background on Hudson Soft's C62 chipset:

    - http://www.hudsonsoft.net/ww/about/about.html
    - http://www.hudson.co.jp/corp/eng/coinfo/history.html

*/

#include "emu.h"
#include "c6280.h"

/* only needed for io_buffer */
#include "cpu/h6280/h6280.h"


void c6280_device::sound_stream_update(sound_stream &stream, stream_sample_t **inputs, stream_sample_t **outputs, int samples)
{
	static const int scale_tab[] = {
		0x00, 0x03, 0x05, 0x07, 0x09, 0x0B, 0x0D, 0x0F,
		0x10, 0x13, 0x15, 0x17, 0x19, 0x1B, 0x1D, 0x1F
	};

	int lmal = (m_balance >> 4) & 0x0F;
	int rmal = (m_balance >> 0) & 0x0F;

	lmal = scale_tab[lmal];
	rmal = scale_tab[rmal];

	/* Clear buffer */
	for (int i = 0; i < samples; i++)
	{
		outputs[0][i] = 0;
		outputs[1][i] = 0;
	}

	for (int ch = 0; ch < 6; ch++)
	{
		/* Only look at enabled channels */
		if(m_channel[ch].m_control & 0x80)
		{
			int lal = (m_channel[ch].m_balance >> 4) & 0x0F;
			int ral = (m_channel[ch].m_balance >> 0) & 0x0F;
			int al  = m_channel[ch].m_control & 0x1F;

			lal = scale_tab[lal];
			ral = scale_tab[ral];

			/* Calculate volume just as the patent says */
			int vll = (0x1F - lal) + (0x1F - al) + (0x1F - lmal);
			if(vll > 0x1F) vll = 0x1F;

			int vlr = (0x1F - ral) + (0x1F - al) + (0x1F - rmal);
			if(vlr > 0x1F) vlr = 0x1F;

			vll = m_volume_table[vll];
			vlr = m_volume_table[vlr];

			/* Check channel mode */
			if((ch >= 4) && (m_channel[ch].m_noise_control & 0x80))
			{
				/* Noise mode */
				UINT32 step = m_noise_freq_tab[(m_channel[ch].m_noise_control & 0x1F) ^ 0x1F];
				for (int i = 0; i < samples; i += 1)
				{
					static int data = 0;
					m_channel[ch].m_noise_counter += step;
					if(m_channel[ch].m_noise_counter >= 0x800)
					{
						data = (machine().rand() & 1) ? 0x1F : 0;
					}
					m_channel[ch].m_noise_counter &= 0x7FF;
					outputs[0][i] += (INT16)(vll * (data - 16));
					outputs[1][i] += (INT16)(vlr * (data - 16));
				}
			}
			else
			if(m_channel[ch].m_control & 0x40)
			{
				/* DDA mode */
				for (int i = 0; i < samples; i++)
				{
					outputs[0][i] += (INT16)(vll * (m_channel[ch].m_dda - 16));
					outputs[1][i] += (INT16)(vlr * (m_channel[ch].m_dda - 16));
				}
			}
			else
			{
				/* Waveform mode */
				UINT32 step = m_wave_freq_tab[m_channel[ch].m_frequency];
				for (int i = 0; i < samples; i += 1)
				{
					int offset;
					INT16 data;
					offset = (m_channel[ch].m_counter >> 12) & 0x1F;
					m_channel[ch].m_counter += step;
					m_channel[ch].m_counter &= 0x1FFFF;
					data = m_channel[ch].m_waveform[offset];
					outputs[0][i] += (INT16)(vll * (data - 16));
					outputs[1][i] += (INT16)(vlr * (data - 16));
				}
			}
		}
	}
}


/*--------------------------------------------------------------------------*/
/* MAME specific code                                                       */
/*--------------------------------------------------------------------------*/

READ8_MEMBER( c6280_device::c6280_r )
{
	return m_cpudevice->io_get_buffer();
}

WRITE8_MEMBER( c6280_device::c6280_w )
{
	m_cpudevice->io_set_buffer(data);

	channel *chan = &m_channel[m_select];

	/* Update stream */
	m_stream->update();

	switch(offset & 0x0F)
	{
		case 0x00: /* Channel select */
			m_select = data & 0x07;
			break;

		case 0x01: /* Global balance */
			m_balance  = data;
			break;

		case 0x02: /* Channel frequency (LSB) */
			chan->m_frequency = (chan->m_frequency & 0x0F00) | data;
			chan->m_frequency &= 0x0FFF;
			break;

		case 0x03: /* Channel frequency (MSB) */
			chan->m_frequency = (chan->m_frequency & 0x00FF) | (data << 8);
			chan->m_frequency &= 0x0FFF;
			break;

		case 0x04: /* Channel control (key-on, DDA mode, volume) */

			/* 1-to-0 transition of DDA bit resets waveform index */
			if((chan->m_control & 0x40) && ((data & 0x40) == 0))
			{
				chan->m_index = 0;
			}
			chan->m_control = data;
			break;

		case 0x05: /* Channel balance */
			chan->m_balance = data;
			break;

		case 0x06: /* Channel waveform data */

			switch(chan->m_control & 0xC0)
			{
				case 0x00:
					chan->m_waveform[chan->m_index & 0x1F] = data & 0x1F;
					chan->m_index = (chan->m_index + 1) & 0x1F;
					break;

				case 0x40:
					break;

				case 0x80:
					chan->m_waveform[chan->m_index & 0x1F] = data & 0x1F;
					chan->m_index = (chan->m_index + 1) & 0x1F;
					break;

				case 0xC0:
					chan->m_dda = data & 0x1F;
					break;
			}

			break;

		case 0x07: /* Noise control (enable, frequency) */
			chan->m_noise_control = data;
			break;

		case 0x08: /* LFO frequency */
			m_lfo_frequency = data;
			break;

		case 0x09: /* LFO control (enable, mode) */
			m_lfo_control = data;
			break;

		default:
			break;
	}
}

const device_type C6280 = &device_creator<c6280_device>;

c6280_device::c6280_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
	: device_t(mconfig, C6280, "HuC6280", tag, owner, clock, "c6280", __FILE__),
		device_sound_interface(mconfig, *this),
		m_cpudevice(*this)
{
}

//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void c6280_device::device_start()
{
	int rate = clock() / 16;

	/* Create stereo stream */
	m_stream = machine().sound().stream_alloc(*this, 0, 2, rate);

	/* Loudest volume level for table */
	double level = 65535.0 / 6.0 / 32.0;

	/* Clear context */
	m_select = 0;
	m_balance = 0;
	m_lfo_frequency = 0;
	m_lfo_control = 0;
	memset(m_channel, 0, sizeof(channel) * 8);

	/* Make waveform frequency table */
	for (int i = 0; i < 4096; i += 1)
	{
		double step = ((clock() / rate) * 4096) / (i + 1);
		m_wave_freq_tab[(1 + i) & 0xFFF] = (UINT32)step;
	}

	/* Make noise frequency table */
	for (int i = 0; i < 32; i += 1)
	{
		double step = ((clock() / rate) * 32) / (i+1);
		m_noise_freq_tab[i] = (UINT32)step;
	}

	/* Make volume table */
	/* PSG has 48dB volume range spread over 32 steps */
	double step = 48.0 / 32.0;
	for (int i = 0; i < 31; i++)
	{
		m_volume_table[i] = (UINT16)level;
		level /= pow(10.0, step / 20.0);
	}
	m_volume_table[31] = 0;

	save_item(NAME(m_select));
	save_item(NAME(m_balance));
	save_item(NAME(m_lfo_frequency));
	save_item(NAME(m_lfo_control));
	for (int chan = 0; chan < 8; chan++)
	{
		save_item(NAME(m_channel[chan].m_frequency), chan);
		save_item(NAME(m_channel[chan].m_control), chan);
		save_item(NAME(m_channel[chan].m_balance), chan);
		save_item(NAME(m_channel[chan].m_waveform), chan);
		save_item(NAME(m_channel[chan].m_index), chan);
		save_item(NAME(m_channel[chan].m_dda), chan);
		save_item(NAME(m_channel[chan].m_noise_control), chan);
		save_item(NAME(m_channel[chan].m_noise_counter), chan);
		save_item(NAME(m_channel[chan].m_counter), chan);
	}
}
