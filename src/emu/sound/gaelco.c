/***************************************************************************
                    Gaelco Sound Hardware

                By Manuel Abadia <manu@teleline.es>

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

#define VERBOSE_SOUND 0
#define VERBOSE_READ_WRITES 0
#define LOG_SOUND(x) do { if (VERBOSE_SOUND) logerror x; } while (0)
#define LOG_READ_WRITES(x) do { if (VERBOSE_READ_WRITES) logerror x; } while (0)

#define LOG_WAVE  0
//#define ALT_MIX

#define GAELCO_NUM_CHANNELS 	0x07
#define VOLUME_LEVELS			0x10

/* this structure defines a channel */
typedef struct _gaelco_sound_channel gaelco_sound_channel;
struct _gaelco_sound_channel
{
	int active;			/* is it playing? */
	int loop;			/* = 0 no looping, = 1 looping */
	int chunkNum;		/* current chunk if looping */
};

/* this structure defines the Gaelco custom sound chip */
typedef struct _gaelco_sound_state gaelco_sound_state;
struct _gaelco_sound_state
{
	sound_stream *stream;									/* our stream */
	UINT8 *snd_data;										/* PCM data */
	int banks[4];											/* start of each ROM bank */
	gaelco_sound_channel channel[GAELCO_NUM_CHANNELS];	/* 7 stereo channels */

	UINT16 sndregs[0x38];

	/* table for converting from 8 to 16 bits with volume control */
	INT16 volume_table[VOLUME_LEVELS][256];
};

static wav_file *	wavraw;					/* raw waveform */

INLINE gaelco_sound_state *get_safe_token(device_t *device)
{
	assert(device != NULL);
	assert(device->type() == GAELCO_GAE1 || device->type() == GAELCO_CG1V);
	return (gaelco_sound_state *)downcast<gaelco_gae1_device *>(device)->token();
}

/*============================================================================
                        CG-1V/GAE1 Sound Update

            Writes length bytes to the sound buffer
  ============================================================================*/

static STREAM_UPDATE( gaelco_update )
{
	gaelco_sound_state *info = (gaelco_sound_state *)param;
	int j, ch;

    /* fill all data needed */
	for(j = 0; j < samples; j++){
		int output_l = 0, output_r = 0;

		/* for each channel */
		for (ch = 0; ch < GAELCO_NUM_CHANNELS; ch ++){
			int ch_data_l = 0, ch_data_r = 0;
			gaelco_sound_channel *channel = &info->channel[ch];

			/* if the channel is playing */
			if (channel->active == 1){
				int data, chunkNum = 0;
				int base_offset, type, bank, vol_r, vol_l, end_pos;

				/* if the channel is looping, get current chunk to play */
				if (channel->loop == 1){
					chunkNum = channel->chunkNum;
				}

				base_offset = ch*8 + chunkNum*4;

				/* get channel parameters */
				type = ((info->sndregs[base_offset + 1] >> 4) & 0x0f);
				bank = info->banks[((info->sndregs[base_offset + 1] >> 0) & 0x03)];
				vol_l = ((info->sndregs[base_offset + 1] >> 12) & 0x0f);
				vol_r = ((info->sndregs[base_offset + 1] >> 8) & 0x0f);
				end_pos = info->sndregs[base_offset + 2] << 8;

				/* generates output data (range 0x00000..0xffff) */
				if (type == 0x08){
					/* PCM, 8 bits mono */
					data = info->snd_data[bank + end_pos + info->sndregs[base_offset + 3]];
					ch_data_l = info->volume_table[vol_l][data];
					ch_data_r = info->volume_table[vol_r][data];

					info->sndregs[base_offset + 3]--;
				} else if (type == 0x0c){
					/* PCM, 8 bits stereo */
					data = info->snd_data[bank + end_pos + info->sndregs[base_offset + 3]];
					ch_data_l = info->volume_table[vol_l][data];

					info->sndregs[base_offset + 3]--;

					if (info->sndregs[base_offset + 3] > 0){
						data = info->snd_data[bank + end_pos + info->sndregs[base_offset + 3]];
						ch_data_r = info->volume_table[vol_r][data];

						info->sndregs[base_offset + 3]--;
					}
				} else {
					LOG_SOUND(("(GAE1) Playing unknown sample format in channel: %02d, type: %02x, bank: %02x, end: %08x, Length: %04x\n", ch, type, bank, end_pos, info->sndregs[base_offset + 3]));
					channel->active = 0;
				}

				/* check if the current sample has finished playing */
				if (info->sndregs[base_offset + 3] == 0){
					if (channel->loop == 0){	/* if no looping, we're done */
						channel->active = 0;
					} else {					/* if we're looping, swap chunks */
						channel->chunkNum = (channel->chunkNum + 1) & 0x01;

						/* if the length of the next chunk is 0, we're done */
						if (info->sndregs[ch*8 + channel->chunkNum*4 + 3] == 0){
							channel->active = 0;
						}
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
		output_l /= GAELCO_NUM_CHANNELS;
		output_r /= GAELCO_NUM_CHANNELS;
#endif

		/* now that we have computed all channels, save current data to the output buffer */
		outputs[0][j] = output_l;
		outputs[1][j] = output_r;
	}

	if (wavraw)
		wav_add_data_32lr(wavraw, outputs[0], outputs[1], samples, 0);
}

/*============================================================================
                        CG-1V/GAE1 Read Handler
  ============================================================================*/

READ16_DEVICE_HANDLER( gaelcosnd_r )
{
	gaelco_sound_state *info = get_safe_token(device);

	LOG_READ_WRITES(("%s: (GAE1): read from %04x\n", device->machine().describe_context(), offset));

	return info->sndregs[offset];
}

/*============================================================================
                        CG-1V/GAE1 Write Handler
  ============================================================================*/

WRITE16_DEVICE_HANDLER( gaelcosnd_w )
{
	gaelco_sound_state *info = get_safe_token(device);
	gaelco_sound_channel *channel = &info->channel[offset >> 3];

	LOG_READ_WRITES(("%s: (GAE1): write %04x to %04x\n", device->machine().describe_context(), data, offset));

	/* first update the stream to this point in time */
	info->stream->update();

	COMBINE_DATA(&info->sndregs[offset]);

	switch(offset & 0x07){
		case 0x03:
			/* trigger sound */
			if ((info->sndregs[offset - 1] != 0) && (data != 0)){
				if (!channel->active){
					channel->active = 1;
					channel->chunkNum = 0;
					channel->loop = 0;
					LOG_SOUND(("(GAE1) Playing sample channel: %02d, type: %02x, bank: %02x, end: %08x, Length: %04x\n", offset >> 3, (info->sndregs[offset - 2] >> 4) & 0x0f, info->sndregs[offset - 2] & 0x03, info->sndregs[offset - 1] << 8, data));
				}
			} else {
				channel->active = 0;
			}

			break;

		case 0x07: /* enable/disable looping */
			if ((info->sndregs[offset - 1] != 0) && (data != 0)){
				LOG_SOUND(("(GAE1) Looping in channel: %02d, type: %02x, bank: %02x, end: %08x, Length: %04x\n", offset >> 3, (info->sndregs[offset - 2] >> 4) & 0x0f, info->sndregs[offset - 2] & 0x03, info->sndregs[offset - 1] << 8, data));
				channel->loop = 1;
			} else {
				channel->loop = 0;
			}

			break;
	}
}

/*============================================================================
                        CG-1V/GAE1 Init
  ============================================================================*/

static DEVICE_START( gaelco )
{
	int j, vol;
	const gaelcosnd_interface *intf = (const gaelcosnd_interface *)device->static_config();

	gaelco_sound_state *info = get_safe_token(device);

	/* copy rom banks */
	for (j = 0; j < 4; j++){
		info->banks[j] = intf->banks[j];
	}
	info->stream = device->machine().sound().stream_alloc(*device, 0, 2, 8000, info, gaelco_update);
	info->snd_data = (UINT8 *)device->machine().root_device().memregion(intf->gfxregion)->base();
	if (info->snd_data == NULL)
		info->snd_data = *device->region();

	/* init volume table */
	for (vol = 0; vol < VOLUME_LEVELS; vol++){
		for (j = -128; j <= 127; j++){
			info->volume_table[vol][(j ^ 0x80) & 0xff] = (vol*j*256)/(VOLUME_LEVELS - 1);
		}
	}

	if (LOG_WAVE)
		wavraw = wav_open("gae1_snd.wav", 8000, 2);
}


static DEVICE_STOP( gaelco )
{
	if (wavraw)
		wav_close(wavraw);
	wavraw = NULL;
}


const device_type GAELCO_GAE1 = &device_creator<gaelco_gae1_device>;

gaelco_gae1_device::gaelco_gae1_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
	: device_t(mconfig, GAELCO_GAE1, "Gaelco GAE1", tag, owner, clock),
	  device_sound_interface(mconfig, *this)
{
	m_token = global_alloc_array_clear(UINT8, sizeof(gaelco_sound_state));
}

gaelco_gae1_device::gaelco_gae1_device(const machine_config &mconfig, device_type type, const char *name, const char *tag, device_t *owner, UINT32 clock)
	: device_t(mconfig, type, name, tag, owner, clock),
	  device_sound_interface(mconfig, *this)
{
	m_token = global_alloc_array_clear(UINT8, sizeof(gaelco_sound_state));
}

//-------------------------------------------------
//  device_config_complete - perform any
//  operations now that the configuration is
//  complete
//-------------------------------------------------

void gaelco_gae1_device::device_config_complete()
{
}

//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void gaelco_gae1_device::device_start()
{
	DEVICE_START_NAME( gaelco )(this);
}

//-------------------------------------------------
//  device_stop - device-specific stop
//-------------------------------------------------

void gaelco_gae1_device::device_stop()
{
	DEVICE_STOP_NAME( gaelco )(this);
}

//-------------------------------------------------
//  sound_stream_update - handle a stream update
//-------------------------------------------------

void gaelco_gae1_device::sound_stream_update(sound_stream &stream, stream_sample_t **inputs, stream_sample_t **outputs, int samples)
{
	// should never get here
	fatalerror("sound_stream_update called; not applicable to legacy sound devices\n");
}


const device_type GAELCO_CG1V = &device_creator<gaelco_cg1v_device>;

gaelco_cg1v_device::gaelco_cg1v_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
	: gaelco_gae1_device(mconfig, GAELCO_CG1V, "Gaelco CG1V", tag, owner, clock)
{
}
