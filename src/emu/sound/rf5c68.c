/*********************************************************/
/*    ricoh RF5C68(or clone) PCM controller              */
/*********************************************************/

#include "emu.h"
#include "rf5c68.h"


#define  NUM_CHANNELS    (8)


typedef struct _pcm_channel pcm_channel;
struct _pcm_channel
{
	UINT8		enable;
	UINT8		env;
	UINT8		pan;
	UINT8		start;
	UINT32		addr;
	UINT16		step;
	UINT16		loopst;
};


typedef struct _rf5c68_state rf5c68_state;
struct _rf5c68_state
{
	sound_stream *		stream;
	pcm_channel			chan[NUM_CHANNELS];
	UINT8				cbank;
	UINT8				wbank;
	UINT8				enable;
	UINT8				data[0x10000];
	void				(*sample_callback)(device_t* device,int channel);
	device_t* device;
};


INLINE rf5c68_state *get_safe_token(device_t *device)
{
	assert(device != NULL);
	assert(device->type() == RF5C68);
	return (rf5c68_state *)downcast<rf5c68_device *>(device)->token();
}


/************************************************/
/*    RF5C68 stream update                      */
/************************************************/

static STREAM_UPDATE( rf5c68_update )
{
	rf5c68_state *chip = (rf5c68_state *)param;
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
		pcm_channel *chan = &chip->chan[i];

		/* if this channel is active, accumulate samples */
		if (chan->enable)
		{
			int lv = (chan->pan & 0x0f) * chan->env;
			int rv = ((chan->pan >> 4) & 0x0f) * chan->env;

			/* loop over the sample buffer */
			for (j = 0; j < samples; j++)
			{
				int sample;

				/* trigger sample callback */
				if(chip->sample_callback)
				{
					if(((chan->addr >> 11) & 0xfff) == 0xfff)
						chip->sample_callback(chip->device,((chan->addr >> 11)/0x2000));
				}

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

static DEVICE_START( rf5c68 )
{
	const rf5c68_interface* intf = (const rf5c68_interface*)device->static_config();

	/* allocate memory for the chip */
	rf5c68_state *chip = get_safe_token(device);
	memset(chip->data, 0xff, sizeof(chip->data));

	/* allocate the stream */
	chip->stream = device->machine().sound().stream_alloc(*device, 0, 2, device->clock() / 384, chip, rf5c68_update);

	chip->device = device;

	/* set up callback */
	if(intf != NULL)
		chip->sample_callback = intf->sample_end_callback;
	else
		chip->sample_callback = NULL;
}


/************************************************/
/*    RF5C68 write register                     */
/************************************************/

READ8_DEVICE_HANDLER( rf5c68_r )
{
	rf5c68_state *chip = get_safe_token(device);
	UINT8 shift;

	chip->stream->update();
	shift = (offset & 1) ? 11 + 8 : 11;

//  printf("%08x\n",(chip->chan[(offset & 0x0e) >> 1].addr));

	return (chip->chan[(offset & 0x0e) >> 1].addr) >> (shift);
}

WRITE8_DEVICE_HANDLER( rf5c68_w )
{
	rf5c68_state *chip = get_safe_token(device);
	pcm_channel *chan = &chip->chan[chip->cbank];
	int i;

	/* force the stream to update first */
	chip->stream->update();

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

READ8_DEVICE_HANDLER( rf5c68_mem_r )
{
	rf5c68_state *chip = get_safe_token(device);
	return chip->data[chip->wbank * 0x1000 + offset];
}


/************************************************/
/*    RF5C68 write memory                       */
/************************************************/

WRITE8_DEVICE_HANDLER( rf5c68_mem_w )
{
	rf5c68_state *chip = get_safe_token(device);
	chip->data[chip->wbank * 0x1000 + offset] = data;
}



/**************************************************************************
 * Generic get_info
 **************************************************************************/

DEVICE_GET_INFO( rf5c68 )
{
	switch (state)
	{
		/* --- the following bits of info are returned as 64-bit signed integers --- */
		case DEVINFO_INT_TOKEN_BYTES:					info->i = sizeof(rf5c68_state);				break;

		/* --- the following bits of info are returned as pointers to data or functions --- */
		case DEVINFO_FCT_START:							info->start = DEVICE_START_NAME( rf5c68 );			break;
		case DEVINFO_FCT_STOP:							/* Nothing */									break;
		case DEVINFO_FCT_RESET:							/* Nothing */									break;

		/* --- the following bits of info are returned as NULL-terminated strings --- */
		case DEVINFO_STR_NAME:							strcpy(info->s, "RF5C68");						break;
		case DEVINFO_STR_FAMILY:					strcpy(info->s, "Ricoh PCM");					break;
		case DEVINFO_STR_VERSION:					strcpy(info->s, "1.0");							break;
		case DEVINFO_STR_SOURCE_FILE:						strcpy(info->s, __FILE__);						break;
		case DEVINFO_STR_CREDITS:					strcpy(info->s, "Copyright Nicola Salmoria and the MAME Team"); break;
	}
}

/**************** end of file ****************/

const device_type RF5C68 = &device_creator<rf5c68_device>;

rf5c68_device::rf5c68_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
	: device_t(mconfig, RF5C68, "RF5C68", tag, owner, clock),
	  device_sound_interface(mconfig, *this)
{
	m_token = global_alloc_array_clear(UINT8, sizeof(rf5c68_state));
}

//-------------------------------------------------
//  device_config_complete - perform any
//  operations now that the configuration is
//  complete
//-------------------------------------------------

void rf5c68_device::device_config_complete()
{
}

//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void rf5c68_device::device_start()
{
	DEVICE_START_NAME( rf5c68 )(this);
}

//-------------------------------------------------
//  sound_stream_update - handle a stream update
//-------------------------------------------------

void rf5c68_device::sound_stream_update(sound_stream &stream, stream_sample_t **inputs, stream_sample_t **outputs, int samples)
{
	// should never get here
	fatalerror("sound_stream_update called; not applicable to legacy sound devices\n");
}


