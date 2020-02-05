// license:BSD-3-Clause
// copyright-holders:Charles MacDonald
/*
    Hudson PSG emulator
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

    - Verify LFO frequency from real hardware.

    - Noise generating algorithm is shared in all channels?

    For some background on Hudson Soft's C62 chipset:

    - http://www.hudsonsoft.net/ww/about/about.html
    - http://www.hudson.co.jp/corp/eng/coinfo/history.html

    Integrated on:
    HuC6280 CPU (PC Engine/TurboGrafx 16)
    HuC6230 Sound Chip (PC-FX, with OKI ADPCM)

*/

#include "emu.h"
#include "c6280.h"

#include <algorithm>

void c6280_device::sound_stream_update(sound_stream &stream, stream_sample_t **inputs, stream_sample_t **outputs, int samples)
{
	const u8 lmal = (m_balance >> 4) & 0x0f;
	const u8 rmal = (m_balance >> 0) & 0x0f;

	/* Clear buffer */
	std::fill_n(&outputs[0][0], samples, 0);
	std::fill_n(&outputs[1][0], samples, 0);

	for (int ch = 0; ch < 6; ch++)
	{
		channel *chan = &m_channel[ch];
		/* Only look at enabled channels */
		if (chan->control & 0x80)
		{
			const u8 lal = (chan->balance >> 4) & 0x0f;
			const u8 ral = (chan->balance >> 0) & 0x0f;
			const u8 al  = (chan->control >> 1) & 0x0f; // only high 4 bit is affected to calculating volume, low 1 bit is independent

			// verified from both patent and manual
			int vll = (0xf - lmal) + (0xf - al) + (0xf - lal);
			if (vll > 0xf) vll = 0xf;

			int vlr = (0xf - rmal) + (0xf - al) + (0xf - ral);
			if (vlr > 0xf) vlr = 0xf;

			vll = m_volume_table[(vll << 1) | (chan->control & 1)];
			vlr = m_volume_table[(vlr << 1) | (chan->control & 1)];

			/* Check channel mode */
			if ((ch >= 4) && (chan->noise_control & 0x80))
			{
				/* Noise mode */
				const u32 step = (chan->noise_control & 0x1f) ^ 0x1f;
				for (int i = 0; i < samples; i += 1)
				{
					s16 data = BIT(chan->noise_seed, 0) ? 0x1f : 0;
					chan->noise_counter--;
					if (chan->noise_counter <= 0)
					{
						chan->noise_counter = step << 6; // 32 * 2
						const u32 seed = chan->noise_seed;
						// based on Charles MacDonald's research
						chan->noise_seed = (seed >> 1) | ((BIT(seed, 0) ^ BIT(seed, 1) ^ BIT(seed, 11) ^ BIT(seed, 12) ^ BIT(seed, 17)) << 17);
					}
					outputs[0][i] += (s16)(vll * (data - 16));
					outputs[1][i] += (s16)(vlr * (data - 16));
				}
			}
			else
			if (chan->control & 0x40)
			{
				/* DDA mode */
				for (int i = 0; i < samples; i++)
				{
					outputs[0][i] += (s16)(vll * (chan->dda - 16));
					outputs[1][i] += (s16)(vlr * (chan->dda - 16));
				}
			}
			else
			{
				if ((m_lfo_control & 3) && (ch < 2))
				{
					if (ch == 0) // CH 0 only, CH 1 is muted
					{
						/* Waveform mode with LFO */
						channel *lfo_srcchan = &m_channel[1];
						channel *lfo_dstchan = &m_channel[0];
						const u16 lfo_step = lfo_srcchan->frequency ? lfo_srcchan->frequency : 0x1000;
						for (int i = 0; i < samples; i += 1)
						{
							s32 step = lfo_dstchan->frequency ? lfo_dstchan->frequency : 0x1000;
							if (m_lfo_control & 0x80) // reset LFO
							{
								lfo_srcchan->tick = lfo_step * m_lfo_frequency;
								lfo_srcchan->index = 0;
							}
							else
							{
								const s16 lfo_data = lfo_srcchan->waveform[lfo_srcchan->index];
								lfo_srcchan->tick--;
								if (lfo_srcchan->tick <= 0)
								{
									lfo_srcchan->tick = lfo_step * m_lfo_frequency; // verified from manual
									lfo_srcchan->index = (lfo_srcchan->index + 1) & 0x1f;
								}
								step += ((lfo_data - 16) << (((m_lfo_control & 3)-1)<<1)); // verified from manual
							}
							const s16 data = lfo_dstchan->waveform[lfo_dstchan->index];
							lfo_dstchan->tick--;
							if (lfo_dstchan->tick <= 0)
							{
								lfo_dstchan->tick = step;
								lfo_dstchan->index = (lfo_dstchan->index + 1) & 0x1f;
							}
							outputs[0][i] += (s16)(vll * (data - 16));
							outputs[1][i] += (s16)(vlr * (data - 16));
						}
					}
				}
				else
				{
					/* Waveform mode */
					const u32 step = chan->frequency ? chan->frequency : 0x1000;
					for (int i = 0; i < samples; i += 1)
					{
						const s16 data = chan->waveform[chan->index];
						chan->tick--;
						if (chan->tick <= 0)
						{
							chan->tick = step;
							chan->index = (chan->index + 1) & 0x1f;
						}
						outputs[0][i] += (s16)(vll * (data - 16));
						outputs[1][i] += (s16)(vlr * (data - 16));
					}
				}
			}
		}
	}
}


/*--------------------------------------------------------------------------*/
/* MAME specific code                                                       */
/*--------------------------------------------------------------------------*/

/* Write Register Layout

       76543210
    00 -----xxx Channel Select
       -----000 Channel 0
       -----001 Channel 1
       -----010 Channel 2
       -----011 Channel 3
       -----100 Channel 4
       -----101 Channel 5

    01 xxxx---- Overall Left Volume
       ----xxxx Overall Right Volume

    02 xxxxxxxx Frequency (Low 8 bits)

    03 ----xxxx Frequency (High 4 bits)

    04 x------- Channel Enable
       -x------ Direct D/A
       00------ Write Data
       01------ Reset Counter
       10------ Mixing (Sound Output)
       11------ Direct D/A
       ---xxxxx Channel Master Volume

    05 xxxx---- Channel Left Volume
       ----xxxx Channel Right Volume

    06 ---xxxxx Waveform Data

    07 x------- Noise Enable (channel 5, 6 only)
       ---xxxxx Noise Frequency ^ 0x1f

    08 xxxxxxxx LFO Frequency

    09 x------- LFO Reset
       ------xx LFO Control
       ------00 LFO off
       ------01 Direct Addition
       ------10 2 bit Shift Addition
       ------11 4 bit Shift Addition
*/

WRITE8_MEMBER( c6280_device::c6280_w )
{
	channel *chan = &m_channel[m_select];

	/* Update stream */
	m_stream->update();

	switch (offset & 0x0f)
	{
		case 0x00: /* Channel select */
			m_select = data & 0x07;
			break;

		case 0x01: /* Global balance */
			m_balance  = data;
			break;

		case 0x02: /* Channel frequency (LSB) */
			chan->frequency = (chan->frequency & 0x0f00) | data;
			break;

		case 0x03: /* Channel frequency (MSB) */
			chan->frequency = (chan->frequency & 0x00ff) | ((data << 8) & 0x0f00);
			break;

		case 0x04: /* Channel control (key-on, DDA mode, volume) */

			/* 1-to-0 transition of DDA bit resets waveform index */
			if ((chan->control & 0x40) && ((data & 0x40) == 0))
			{
				chan->index = 0;
			}
			if (((chan->control & 0x80) == 0) && (data & 0x80))
			{
				chan->tick = chan->frequency;
			}
			chan->control = data;
			break;

		case 0x05: /* Channel balance */
			chan->balance = data;
			break;

		case 0x06: /* Channel waveform data */

			switch (chan->control & 0x40)
			{
				case 0x00: // Waveform
					chan->waveform[chan->index & 0x1f] = data & 0x1f;
					if (!(chan->control & 0x80)) // TODO : wave pointer is increased at writing data when sound playback is off??
						chan->index = (chan->index + 1) & 0x1f;
					break;

				case 0x40: // Direct D/A
					chan->dda = data & 0x1f;
					break;
			}

			break;

		case 0x07: /* Noise control (enable, frequency) */
			chan->noise_control = data;
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

DEFINE_DEVICE_TYPE(C6280, c6280_device, "c6280", "Hudson Soft HuC6280 PSG")

c6280_device::c6280_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: device_t(mconfig, C6280, tag, owner, clock)
	, device_sound_interface(mconfig, *this)
{
}

void c6280_device::device_clock_changed()
{
	m_stream->set_sample_rate(clock());
}

//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void c6280_device::device_start()
{
	/* Loudest volume level for table */
	double level = 65535.0 / 6.0 / 32.0;

	/* Clear context */
	m_select = 0;
	m_balance = 0;
	m_lfo_frequency = 0;
	m_lfo_control = 0;
	memset(m_channel, 0, sizeof(channel) * 8);

	m_stream = machine().sound().stream_alloc(*this, 0, 2, clock());

	/* Make volume table */
	/* PSG has 48dB volume range spread over 32 steps */
	double step = 48.0 / 32.0;
	for (int i = 0; i < 30; i++)
	{
		m_volume_table[i] = (u16)level;
		level /= pow(10.0, step / 20.0);
	}
	m_volume_table[30] = m_volume_table[31] = 0;

	save_item(NAME(m_select));
	save_item(NAME(m_balance));
	save_item(NAME(m_lfo_frequency));
	save_item(NAME(m_lfo_control));

	save_item(STRUCT_MEMBER(m_channel, frequency));
	save_item(STRUCT_MEMBER(m_channel, control));
	save_item(STRUCT_MEMBER(m_channel, balance));
	save_item(STRUCT_MEMBER(m_channel, waveform));
	save_item(STRUCT_MEMBER(m_channel, index));
	save_item(STRUCT_MEMBER(m_channel, dda));
	save_item(STRUCT_MEMBER(m_channel, noise_control));
	save_item(STRUCT_MEMBER(m_channel, noise_counter));
	save_item(STRUCT_MEMBER(m_channel, noise_seed));
	save_item(STRUCT_MEMBER(m_channel, tick));
}

void c6280_device::device_reset()
{
	for (int ch = 0; ch < 6; ch++)
	{
		channel *chan = &m_channel[ch];
		chan->index = 0;
		if (ch >= 4)
			chan->noise_seed = 1;
	}
}
