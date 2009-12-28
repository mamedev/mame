/***************************************************************************

    beep.c

    This is used for computers/systems which can only output a constant tone.
    This tone can be turned on and off.
    e.g. PCW and PCW16 computer systems
    KT - 25-Jun-2000

    Sound handler

****************************************************************************/

#include "sndintrf.h"
#include "sound/beep.h"
#include "streams.h"


#define BEEP_RATE			48000

typedef struct _beep_state beep_state;
struct _beep_state
{
	sound_stream *stream;	/* stream number */
	int enable; 			/* enable beep */
	int frequency;			/* set frequency - this can be changed using the appropiate function */
	int incr;				/* initial wave state */
	INT16 signal;			/* current signal */
};


INLINE beep_state *get_safe_token(const device_config *device)
{
	assert(device != NULL);
	assert(device->token != NULL);
	assert(device->type == SOUND);
	assert(sound_get_type(device) == SOUND_BEEP);
	return (beep_state *)device->token;
}



/*************************************
 *
 *  Stream updater
 *
 *************************************/

static STREAM_UPDATE( beep_sound_update )
{
	beep_state *bs = (beep_state *) param;
	stream_sample_t *buffer = outputs[0];
	INT16 signal = bs->signal;
	int clock = 0, rate = BEEP_RATE / 2;

    /* get progress through wave */
	int incr = bs->incr;

	if (bs->frequency > 0)
		clock = bs->frequency;

	/* if we're not enabled, just fill with 0 */
	if ( !bs->enable || clock == 0 )
	{
		memset( buffer, 0, samples * sizeof(*buffer) );
		return;
	}

	/* fill in the sample */
	while( samples-- > 0 )
	{
		*buffer++ = signal;
		incr -= clock;
		while( incr < 0 )
		{
			incr += rate;
			signal = -signal;
		}
	}

	/* store progress through wave */
	bs->incr = incr;
	bs->signal = signal;
}



/*************************************
 *
 *  Sound handler start
 *
 *************************************/

static DEVICE_START( beep )
{
	beep_state *pBeep = get_safe_token(device);

	pBeep->stream = stream_create(device, 0, 1, BEEP_RATE, pBeep, beep_sound_update );
	pBeep->enable = 0;
	pBeep->frequency = 3250;
	pBeep->incr = 0;
	pBeep->signal = 0x07fff;
}



/*************************************
 *
 *  changing state to on from off will restart tone
 *
 *************************************/

void beep_set_state(const device_config *device, int on)
{
	beep_state *info = get_safe_token(device);

	/* only update if new state is not the same as old state */
	if (info->enable == on)
		return;

	stream_update(info->stream);

	info->enable = on;
	/* restart wave from beginning */
	info->incr = 0;
	info->signal = 0x07fff;
}



/*************************************
 *
 *  setting new frequency starts from beginning
 *
 *************************************/

void beep_set_frequency(const device_config *device,int frequency)
{
	beep_state *info = get_safe_token(device);

	if (info->frequency == frequency)
		return;

	stream_update(info->stream);
	info->frequency = frequency;
	info->signal = 0x07fff;
	info->incr = 0;
}



/*************************************
 *
 *  change a channel volume
 *
 *************************************/

void beep_set_volume(const device_config *device, int volume)
{
	beep_state *info = get_safe_token(device);

	stream_update(info->stream);

	volume = 100 * volume / 7;

	sound_set_output_gain(device, 0, volume);
}



/**************************************************************************
 * Generic get_info
 **************************************************************************/

DEVICE_GET_INFO( beep )
{
	switch (state)
	{
		/* --- the following bits of info are returned as 64-bit signed integers --- */
		case DEVINFO_INT_TOKEN_BYTES:					info->i = sizeof(beep_state);				break;

		/* --- the following bits of info are returned as pointers to data or functions --- */
		case DEVINFO_FCT_START:							info->start = DEVICE_START_NAME( beep );	break;

		/* --- the following bits of info are returned as NULL-terminated strings --- */
		case DEVINFO_STR_NAME:							strcpy(info->s, "Beep");					break;
		case DEVINFO_STR_FAMILY:					strcpy(info->s, "Beep");					break;
		case DEVINFO_STR_VERSION:					strcpy(info->s, "1.0");						break;
		case DEVINFO_STR_SOURCE_FILE:						strcpy(info->s, __FILE__);					break;
		case DEVINFO_STR_CREDITS:					strcpy(info->s, "Copyright The MESS Team"); break;
	}
}
