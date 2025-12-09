// license:BSD-3-Clause
// copyright-holders:Manuel Abadia, David Haywood
/***************************************************************************
                    Gaelco Sound Hardware

                By Manuel Abadia <emumanu+mame@gmail.com>

CG-1V/GAE1 (Gaelco custom GFX & Sound chip):
    The CG-1V/GAE1 can handle up to 7 stereo channels.
    The chip output is connected to a TDA1543 (16 bit DAC).

Registers per channel:
======================
    Word | Bit(s)            | Description
    -----+-FEDCBA98-76543210-+--------------------------
      0  | xxxxxxxx xxxxxxxx | not used?
      1  | xxxx---- -------- | left channel volume (0x00..0x0f)
      1  | ----xxxx -------- | right channel volume (0x00..0x0f)
      1  | -------- xxxx---- | sample type (0x0c = PCM 8 bits mono, 0x08 = PCM 8 bits stereo)
      1  | -------- ----xxxx | ROM Bank
      2  | xxxxxxxx xxxxxxxx | sample end position
      3  | xxxxxxxx xxxxxxxx | remaining bytes to play

      the following are used only when looping (usually used for music)

      4  | xxxxxxxx xxxxxxxx | not used?
      5  | xxxx---- -------- | left channel volume (0x00..0x0f)
      5  | ----xxxx -------- | right channel volume (0x00..0x0f)
      5  | -------- xxxx---- | sample type (0x0c = PCM 8 bits mono, 0x08 = PCM 8 bits stereo)
      5  | -------- ----xxxx | ROM Bank
      6  | xxxxxxxx xxxxxxxx | sample end position
      7  | xxxxxxxx xxxxxxxx | remaining bytes to play

    The samples are played from (end position + length) to (end position)!

***************************************************************************/

#include "emu.h"
#include "gaelco.h"

#include "wavwrite.h"

#define LOG_SOUND       (1U << 1)
#define LOG_READ_WRITES (1U << 2)
#define VERBOSE (0)
#include "logmacro.h"

//#define ALT_MIX

#define LOG_WAVE  0
static util::wav_file_ptr wavraw; // Raw waveform


/*============================================================================
                        Gaelco GAE1 sound device
  ============================================================================*/

DEFINE_DEVICE_TYPE(GAELCO_GAE1, gaelco_gae1_device, "gaelco_gae1", "Gaelco GAE1")

gaelco_gae1_device::gaelco_gae1_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: gaelco_gae1_device(mconfig, GAELCO_GAE1, tag, owner, clock)
{
}

gaelco_gae1_device::gaelco_gae1_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, type, tag, owner, clock)
	, device_sound_interface(mconfig, *this)
	, device_rom_interface(mconfig, *this)
	, m_stream(nullptr)
{
}


/*============================================================================
                        CG-1V/GAE1 Sound Update

            Writes length bytes to the sound buffer
  ============================================================================*/

void gaelco_gae1_device::sound_stream_update(sound_stream &stream)
{
	/* fill all data needed */
	for (int j = 0; j < stream.samples(); j++)
	{
		int output_l = 0, output_r = 0;

		/* for each channel */
		for (int ch = 0; ch < NUM_CHANNELS; ch++)
		{
			int ch_data_l = 0, ch_data_r = 0;
			sound_channel *channel = &m_channel[ch];

			/* if the channel is playing */
			if (channel->active == 1)
			{
				int data, chunkNum = 0;
				int base_offset, type, bank, vol_r, vol_l, end_pos;

				/* if the channel is looping, get current chunk to play */
				if (channel->loop == 1)
					chunkNum = channel->chunkNum;

				base_offset = ch*8 + chunkNum*4;

				/* get channel parameters */
				type = ((m_sndregs[base_offset + 1] >> 4) & 0x0f);
				bank = m_banks[((m_sndregs[base_offset + 1] >> 0) & 0x03)];
				vol_l = ((m_sndregs[base_offset + 1] >> 12) & 0x0f);
				vol_r = ((m_sndregs[base_offset + 1] >> 8) & 0x0f);
				end_pos = (m_sndregs[base_offset + 2] << 8) - 1;

				/* generates output data (range 0x00000..0xffff) */
				if (type == 0x08) /* PCM, 8 bits mono */
				{
					data = read_byte(bank + end_pos + m_sndregs[base_offset + 3]);
					ch_data_l = m_volume_table[vol_l][data];
					ch_data_r = m_volume_table[vol_r][data];

					m_sndregs[base_offset + 3]--;
				}
				else if (type == 0x0c) /* PCM, 8 bits stereo */
				{
					data = read_byte(bank + end_pos + m_sndregs[base_offset + 3]);
					ch_data_l = m_volume_table[vol_l][data];

					m_sndregs[base_offset + 3]--;

					if (m_sndregs[base_offset + 3] > 0)
					{
						data = read_byte(bank + end_pos + m_sndregs[base_offset + 3]);
						ch_data_r = m_volume_table[vol_r][data];

						m_sndregs[base_offset + 3]--;
					}
				}
				else
				{
					LOGMASKED(LOG_SOUND, "(GAE1) Playing unknown sample format in channel: %02d, type: %02x, bank: %02x, end: %08x, Length: %04x\n", ch, type, bank, end_pos, m_sndregs[base_offset + 3]);
					//channel->active = 0;
					// play2000 expects these to expire, are they valid? this is unrelated to the missing sounds in touchgo which never hits here
					m_sndregs[base_offset + 3]--;
				}

				/* check if the current sample has finished playing */
				if (m_sndregs[base_offset + 3] == 0)
				{
					if (channel->loop == 0)    /* if no looping, we're done */
					{
						channel->active = 0;
					}
					else                    /* if we're looping, swap chunks */
					{
						channel->chunkNum = (channel->chunkNum + 1) & 0x01;

						/* if the length of the next chunk is 0, we're done */
						if (m_sndregs[ch*8 + channel->chunkNum*4 + 3] == 0)
							channel->active = 0;

					}
				}
			}

			/* add the contribution of this channel to the current data output */
			output_l += ch_data_l;
			output_r += ch_data_r;
		}

#ifndef ALT_MIX
		/* clip to max or min value */
		if (output_l > 32767) output_l = 32767;
		if (output_r > 32767) output_r = 32767;
		if (output_l < -32768) output_l = -32768;
		if (output_r < -32768) output_r = -32768;
#else
		/* ponderate channels */
		output_l /= NUM_CHANNELS;
		output_r /= NUM_CHANNELS;
#endif

		/* now that we have computed all channels, save current data to the output buffer */
		stream.put_int(0, j, output_l, 32768);
		stream.put_int(1, j, output_r, 32768);
	}

//  if (wavraw)
//      util::wav_add_data_buffer(*wavraw, outputs[0], outputs[1]);
}

/*============================================================================
                        CG-1V/GAE1 Read Handler
  ============================================================================*/

uint16_t gaelco_gae1_device::gaelcosnd_r(offs_t offset)
{
	LOGMASKED(LOG_READ_WRITES, "%s: (GAE1): read from %04x\n", machine().describe_context(), offset);

	/* first update the stream to this point in time */
	m_stream->update();

	return m_sndregs[offset];
}

/*============================================================================
                        CG-1V/GAE1 Write Handler
  ============================================================================*/

void gaelco_gae1_device::gaelcosnd_w(offs_t offset, uint16_t data, uint16_t mem_mask)
{
	sound_channel *channel = &m_channel[offset >> 3];

	LOGMASKED(LOG_READ_WRITES, "%s: (GAE1): write %04x to %04x\n", machine().describe_context(), data, offset);

	/* first update the stream to this point in time */
	m_stream->update();

	COMBINE_DATA(&m_sndregs[offset]);

	switch (offset & 0x07)
	{
	case 0x03:
		// if sample end position isn't 0, and length isn't 0
		if ((m_sndregs[offset - 1] != 0) && (data != 0))
		{
			LOGMASKED(LOG_SOUND, "(GAE1) Playing or Queuing 1st chunk in channel: %02d, type: %02x, bank: %02x, end: %08x, Length: %04x\n", offset >> 3, (m_sndregs[offset - 2] >> 4) & 0x0f, m_sndregs[offset - 2] & 0x03, m_sndregs[offset - 1] << 8, data);

			channel->loop = 1;

			if (!channel->active)
			{
				channel->chunkNum = 0;
			}

			channel->active = 1;
		}
		else
		{
			//channel->loop = 0;
			channel->active = 0;
		}

		break;

	case 0x07:
		// if sample end position isn't 0, and length isn't 0
		if ((m_sndregs[offset - 1] != 0) && (data != 0))
		{
			LOGMASKED(LOG_SOUND, "(GAE1) Playing or Queuing 2nd chunk in channel: %02d, type: %02x, bank: %02x, end: %08x, Length: %04x\n", offset >> 3, (m_sndregs[offset - 2] >> 4) & 0x0f, m_sndregs[offset - 2] & 0x03, m_sndregs[offset - 1] << 8, data);

			channel->loop = 1;

			if (!channel->active)
			{
				channel->chunkNum = 1;
			}

			channel->active = 1;
		}
		else
		{
			channel->loop = 0;
			// channel->active = 0;
		}

		break;
	}
}

/*============================================================================
                        CG-1V/GAE1 Init / Close
  ============================================================================*/

void gaelco_gae1_device::device_start()
{
	u32 rate = clock() / 128;
	m_stream = stream_alloc(0, 2, rate);

	/* init volume table */
	for (int vol = 0; vol < VOLUME_LEVELS; vol++)
		for (int j = -128; j <= 127; j++)
			m_volume_table[vol][(j ^ 0x80) & 0xff] = (vol*j*256)/(VOLUME_LEVELS - 1);

	if (LOG_WAVE)
		wavraw = util::wav_open("gae1_snd.wav", rate, 2);

	for (int ch = 0; ch < NUM_CHANNELS; ch++)
	{
		save_item(NAME(m_channel[ch].active), ch);
		save_item(NAME(m_channel[ch].loop), ch);
		save_item(NAME(m_channel[ch].chunkNum), ch);
	}

	save_item(NAME(m_sndregs));
}

void gaelco_gae1_device::device_reset()
{
	for (int ch = 0; ch < NUM_CHANNELS; ch++)
	{
		m_channel[ch].active = 0;
		m_channel[ch].loop = 0;
		m_channel[ch].chunkNum = 0;
	}

	std::fill(std::begin(m_sndregs), std::end(m_sndregs), 0.0);
}

void gaelco_gae1_device::device_stop()
{
	wavraw.reset();
}


void gaelco_gae1_device::device_post_load()
{
	device_clock_changed();
}


void gaelco_gae1_device::device_clock_changed()
{
	u32 rate = clock() / 128;
	m_stream->set_sample_rate(rate);
	wavraw.reset();

	if (LOG_WAVE)
		wavraw = util::wav_open("gae1_snd.wav", rate, 2);
}


void gaelco_gae1_device::rom_bank_pre_change()
{
	m_stream->update();
}


/*============================================================================
                        Gaelco CG-1V sound device
  ============================================================================*/

DEFINE_DEVICE_TYPE(GAELCO_CG1V, gaelco_cg1v_device, "gaelco_cg1v", "Gaelco CG1V")

gaelco_cg1v_device::gaelco_cg1v_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: gaelco_gae1_device(mconfig, GAELCO_CG1V, tag, owner, clock)
{
}
