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


struct beep_sound
{
	sound_stream *stream; 	/* stream number */
	int enable; 			/* enable beep */
	int frequency;			/* set frequency - this can be changed using the appropiate function */
	int incr;				/* initial wave state */
	INT16 signal;			/* current signal */
};



/*************************************
 *
 *  Stream updater
 *
 *************************************/

static void beep_sound_update(void *param,stream_sample_t **inputs, stream_sample_t **_buffer,int length)
{
	struct beep_sound *bs = (struct beep_sound *) param;
	stream_sample_t *buffer = _buffer[0];
	INT16 signal = bs->signal;
	int clock = 0, rate = BEEP_RATE / 2;

    /* get progress through wave */
	int incr = bs->incr;

	if (bs->frequency > 0)
		clock = bs->frequency;

	/* if we're not enabled, just fill with 0 */
	if ( !bs->enable || clock == 0 )
	{
		memset( buffer, 0, length * sizeof(*buffer) );
		return;
	}

	/* fill in the sample */
	while( length-- > 0 )
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

static void *beep_start(int sndindex, int clock, const void *config)
{
	struct beep_sound *pBeep;

	pBeep = auto_malloc(sizeof(*pBeep));
	memset(pBeep, 0, sizeof(*pBeep));

	pBeep->stream = stream_create(0, 1, BEEP_RATE, pBeep, beep_sound_update );
	pBeep->enable = 0;
	pBeep->frequency = 3250;
	pBeep->incr = 0;
	pBeep->signal = 0x07fff;
	return pBeep;
}



/*************************************
 *
 *  changing state to on from off will restart tone
 *
 *************************************/

void beep_set_state( int num, int on )
{
	struct beep_sound *info = sndti_token(SOUND_BEEP, num);

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

void beep_set_frequency(int num,int frequency)
{
	struct beep_sound *info = sndti_token(SOUND_BEEP, num);

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

void beep_set_volume(int num, int volume)
{
	struct beep_sound *info = sndti_token(SOUND_BEEP, num);

	stream_update(info->stream);

	volume = 100 * volume / 7;

	sndti_set_output_gain(SOUND_BEEP, num, 0, volume );
}



/**************************************************************************
 * Generic get_info
 **************************************************************************/

static void beep_set_info(void *token, UINT32 state, sndinfo *info)
{
	switch (state)
	{
		/* no parameters to set */
	}
}


void beep_get_info(void *token, UINT32 state, sndinfo *info)
{
	switch (state)
	{
		/* --- the following bits of info are returned as 64-bit signed integers --- */

		/* --- the following bits of info are returned as pointers to data or functions --- */
		case SNDINFO_PTR_SET_INFO:						info->set_info = beep_set_info;			break;
		case SNDINFO_PTR_START:							info->start = beep_start;				break;
		case SNDINFO_PTR_STOP:							/* nothing */							break;
		case SNDINFO_PTR_RESET:							/* nothing */							break;

		/* --- the following bits of info are returned as NULL-terminated strings --- */
		case SNDINFO_STR_NAME:							info->s = "Beep";						break;
		case SNDINFO_STR_CORE_FAMILY:					info->s = "Beep";						break;
		case SNDINFO_STR_CORE_VERSION:					info->s = "1.0";						break;
		case SNDINFO_STR_CORE_FILE:						info->s = __FILE__;						break;
		case SNDINFO_STR_CORE_CREDITS:					info->s = "Copyright (c) 2005, The MESS Team"; break;
	}
}
