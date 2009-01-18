/***************************************************************************

    SNK Wave sound driver.

    This is a very simple single-voice generator with a programmable waveform.

***************************************************************************/

#include "sndintrf.h"
#include "streams.h"
#include "snkwave.h"


#define WAVEFORM_LENGTH 16

#define CLOCK_SHIFT 8


struct snkwave_sound
{
	/* global sound parameters */
	sound_stream * stream;
	int external_clock;
	int sample_rate;

	/* data about the sound system */
	UINT32 frequency;
	UINT32 counter;
	int waveform_position;

	/* decoded waveform table */
	INT16 waveform[WAVEFORM_LENGTH];
};


/* update the decoded waveform data */
/* The programmable waveform consists of 8 3-bit nibbles.
   The waveform goes to a 4-bit DAC and is played alternatingly forwards and
   backwards.
   When going forwards, bit 3 is 1. When going backwards, it's 0.
   So the sequence 01234567 will play as
   89ABCDEF76543210
*/
static void update_waveform(struct snkwave_sound *chip, unsigned int offset, UINT8 data)
{
	assert(offset < WAVEFORM_LENGTH/4);

	chip->waveform[offset * 2]     = ((data & 0x38) >> 3) << (12-CLOCK_SHIFT);
	chip->waveform[offset * 2 + 1] = ((data & 0x07) >> 0) << (12-CLOCK_SHIFT);
	chip->waveform[WAVEFORM_LENGTH-2 - offset * 2] = ~chip->waveform[offset * 2 + 1];
	chip->waveform[WAVEFORM_LENGTH-1 - offset * 2] = ~chip->waveform[offset * 2];
}


/* generate sound to the mix buffer */
static STREAM_UPDATE( snkwave_update )
{
	struct snkwave_sound *chip = param;
	stream_sample_t *buffer = outputs[0];

	/* zap the contents of the buffer */
	memset(buffer, 0, samples * sizeof(*buffer));

	assert(chip->counter < 0x1000);
	assert(chip->frequency < 0x1000);

	/* if no sound, we're done */
	if (chip->frequency == 0xfff)
		return;

	/* generate sound into buffer while updating the counter */
	while (samples-- > 0)
	{
		int loops;
		INT16 out = 0;

		loops = 1 << CLOCK_SHIFT;
		while (loops > 0)
		{
			int steps = 0x1000 - chip->counter;

			if (steps <= loops)
			{
				out += chip->waveform[chip->waveform_position] * steps;
				chip->counter = chip->frequency;
				chip->waveform_position = (chip->waveform_position + 1) & (WAVEFORM_LENGTH-1);
				loops -= steps;
			}
			else
			{
				out += chip->waveform[chip->waveform_position] * loops;
				chip->counter += loops;
				loops = 0;
			}
		}

		*buffer++ = out;
	}
}


static SND_START( snkwave )
{
	struct snkwave_sound *chip = device->token;

	assert(device->static_config == 0);

	/* adjust internal clock */
	chip->external_clock = clock;

	/* adjust output clock */
	chip->sample_rate = chip->external_clock >> CLOCK_SHIFT;

	/* get stream channels */
	chip->stream = stream_create(device, 0, 1, chip->sample_rate, chip, snkwave_update);

	/* reset all the voices */
	chip->frequency = 0;
	chip->counter = 0;
	chip->waveform_position = 0;

	/* register with the save state system */
	state_save_register_device_item(device, 0, chip->frequency);
	state_save_register_device_item(device, 0, chip->counter);
	state_save_register_device_item(device, 0, chip->waveform_position);
	state_save_register_device_item_pointer(device, 0, chip->waveform, WAVEFORM_LENGTH);

	return DEVICE_START_OK;
}


/********************************************************************************/

/* SNK wave register map
    all registers are 6-bit
    0-1         frequency (12-bit)
    2-5         waveform (8 3-bit nibbles)
*/

WRITE8_HANDLER( snkwave_w )
{
	struct snkwave_sound *chip = sndti_token(SOUND_SNKWAVE, 0);

	stream_update(chip->stream);

	// all registers are 6-bit
	data &= 0x3f;

	if (offset == 0)
		chip->frequency = (chip->frequency & 0x03f) | (data << 6);
	else if (offset == 1)
		chip->frequency = (chip->frequency & 0xfc0) | data;
	else if (offset <= 5)
		update_waveform(chip, offset - 2, data);
}



/**************************************************************************
 * Generic get_info
 **************************************************************************/

static SND_SET_INFO( snkwave )
{
	switch (state)
	{
		/* no parameters to set */
	}
}


SND_GET_INFO( snkwave )
{
	switch (state)
	{
		/* --- the following bits of info are returned as 64-bit signed integers --- */
		case SNDINFO_INT_TOKEN_BYTES:					info->i = sizeof(struct snkwave_sound); 		break;

		/* --- the following bits of info are returned as pointers to data or functions --- */
		case SNDINFO_PTR_SET_INFO:						info->set_info = SND_SET_INFO_NAME( snkwave );	break;
		case SNDINFO_PTR_START:							info->start = SND_START_NAME( snkwave );		break;
		case SNDINFO_PTR_STOP:							/* Nothing */									break;
		case SNDINFO_PTR_RESET:							/* Nothing */									break;

		/* --- the following bits of info are returned as NULL-terminated strings --- */
		case SNDINFO_STR_NAME:							strcpy(info->s, "SNK Wave");					break;
		case SNDINFO_STR_CORE_FAMILY:					strcpy(info->s, "SNK Wave");					break;
		case SNDINFO_STR_CORE_VERSION:					strcpy(info->s, "1.0");							break;
		case SNDINFO_STR_CORE_FILE:						strcpy(info->s, __FILE__);						break;
		case SNDINFO_STR_CORE_CREDITS:					strcpy(info->s, "Copyright Nicola Salmoria and the MAME Team"); break;
	}
}
