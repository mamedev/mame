/*********************************************************/
/*    ricoh RF5C68(or clone) PCM controller              */
/*********************************************************/

#include "sndintrf.h"
#include "streams.h"
#include "rf5c68.h"
#include <math.h>


#define  NUM_CHANNELS    (8)


struct pcm_channel
{
	UINT8		enable;
	UINT8		env;
	UINT8		pan;
	UINT8		start;
	UINT32		addr;
	UINT16		step;
	UINT16		loopst;
};


struct rf5c68pcm
{
	sound_stream *		stream;
	struct pcm_channel	chan[NUM_CHANNELS];
	UINT8				cbank;
	UINT8				wbank;
	UINT8				enable;
	UINT8				data[0x10000];
};



/************************************************/
/*    RF5C68 stream update                      */
/************************************************/

static STREAM_UPDATE( rf5c68_update )
{
	struct rf5c68pcm *chip = param;
	stream_sample_t *left = outputs[0];
	stream_sample_t *right = outputs[1];
	int i, j;

	/* start with clean buffers */
	memset(left, 0, samples * sizeof(*left));
	memset(right, 0, samples * sizeof(*right));

	/* bail if not enabled */
	if (!chip->enable)
		return;

	/* loop over channels */
	for (i = 0; i < NUM_CHANNELS; i++)
	{
		struct pcm_channel *chan = &chip->chan[i];

		/* if this channel is active, accumulate samples */
		if (chan->enable)
		{
			int lv = (chan->pan & 0x0f) * chan->env;
			int rv = ((chan->pan >> 4) & 0x0f) * chan->env;

			/* loop over the sample buffer */
			for (j = 0; j < samples; j++)
			{
				int sample;

				/* fetch the sample and handle looping */
				sample = chip->data[(chan->addr >> 11) & 0xffff];
				if (sample == 0xff)
				{
					chan->addr = chan->loopst << 11;
					sample = chip->data[(chan->addr >> 11) & 0xffff];

					/* if we loop to a loop point, we're effectively dead */
					if (sample == 0xff)
						break;
				}
				chan->addr += chan->step;

				/* add to the buffer */
				if (sample & 0x80)
				{
					sample &= 0x7f;
					left[j] += (sample * lv) >> 5;
					right[j] += (sample * rv) >> 5;
				}
				else
				{
					left[j] -= (sample * lv) >> 5;
					right[j] -= (sample * rv) >> 5;
				}
			}
		}
	}

	/* now clamp and shift the result (output is only 10 bits) */
	for (j = 0; j < samples; j++)
	{
		stream_sample_t temp;

		temp = left[j];
		if (temp > 32767) temp = 32767;
		else if (temp < -32768) temp = -32768;
		left[j] = temp & ~0x3f;

		temp = right[j];
		if (temp > 32767) temp = 32767;
		else if (temp < -32768) temp = -32768;
		right[j] = temp & ~0x3f;
	}
}


/************************************************/
/*    RF5C68 start                              */
/************************************************/

static SND_START( rf5c68 )
{
	/* allocate memory for the chip */
	struct rf5c68pcm *chip = device->token;

	/* allocate the stream */
	chip->stream = stream_create(device, 0, 2, clock / 384, chip, rf5c68_update);
	return DEVICE_START_OK;
}


/************************************************/
/*    RF5C68 write register                     */
/************************************************/

WRITE8_HANDLER( rf5c68_reg_w )
{
	struct rf5c68pcm *chip = sndti_token(SOUND_RF5C68, 0);
	struct pcm_channel *chan = &chip->chan[chip->cbank];
	int i;

	/* force the stream to update first */
	stream_update(chip->stream);

	/* switch off the address */
	switch (offset)
	{
		case 0x00:	/* envelope */
			chan->env = data;
			break;

		case 0x01:	/* pan */
			chan->pan = data;
			break;

		case 0x02:	/* FDL */
			chan->step = (chan->step & 0xff00) | (data & 0x00ff);
			break;

		case 0x03:	/* FDH */
			chan->step = (chan->step & 0x00ff) | ((data << 8) & 0xff00);
			break;

		case 0x04:	/* LSL */
			chan->loopst = (chan->loopst & 0xff00) | (data & 0x00ff);
			break;

		case 0x05:	/* LSH */
			chan->loopst = (chan->loopst & 0x00ff) | ((data << 8) & 0xff00);
			break;

		case 0x06:	/* ST */
			chan->start = data;
			if (!chan->enable)
				chan->addr = chan->start << (8 + 11);
			break;

		case 0x07:	/* control reg */
			chip->enable = (data >> 7) & 1;
			if (data & 0x40)
				chip->cbank = data & 7;
			else
				chip->wbank = data & 15;
			break;

		case 0x08:	/* channel on/off reg */
			for (i = 0; i < 8; i++)
			{
				chip->chan[i].enable = (~data >> i) & 1;
				if (!chip->chan[i].enable)
					chip->chan[i].addr = chip->chan[i].start << (8 + 11);
			}
			break;
	}
}


/************************************************/
/*    RF5C68 read memory                        */
/************************************************/

READ8_HANDLER( rf5c68_r )
{
	struct rf5c68pcm *chip = sndti_token(SOUND_RF5C68, 0);
	return chip->data[chip->wbank * 0x1000 + offset];
}


/************************************************/
/*    RF5C68 write memory                       */
/************************************************/

WRITE8_HANDLER( rf5c68_w )
{
	struct rf5c68pcm *chip = sndti_token(SOUND_RF5C68, 0);
	chip->data[chip->wbank * 0x1000 + offset] = data;
}



/**************************************************************************
 * Generic get_info
 **************************************************************************/

static SND_SET_INFO( rf5c68 )
{
	switch (state)
	{
		/* no parameters to set */
	}
}


SND_GET_INFO( rf5c68 )
{
	switch (state)
	{
		/* --- the following bits of info are returned as 64-bit signed integers --- */
		case SNDINFO_INT_TOKEN_BYTES:					info->i = sizeof(struct rf5c68pcm);				break;

		/* --- the following bits of info are returned as pointers to data or functions --- */
		case SNDINFO_PTR_SET_INFO:						info->set_info = SND_SET_INFO_NAME( rf5c68 );	break;
		case SNDINFO_PTR_START:							info->start = SND_START_NAME( rf5c68 );			break;
		case SNDINFO_PTR_STOP:							/* Nothing */									break;
		case SNDINFO_PTR_RESET:							/* Nothing */									break;

		/* --- the following bits of info are returned as NULL-terminated strings --- */
		case SNDINFO_STR_NAME:							strcpy(info->s, "RF5C68");						break;
		case SNDINFO_STR_CORE_FAMILY:					strcpy(info->s, "Ricoh PCM");					break;
		case SNDINFO_STR_CORE_VERSION:					strcpy(info->s, "1.0");							break;
		case SNDINFO_STR_CORE_FILE:						strcpy(info->s, __FILE__);						break;
		case SNDINFO_STR_CORE_CREDITS:					strcpy(info->s, "Copyright Nicola Salmoria and the MAME Team"); break;
	}
}

/**************** end of file ****************/
