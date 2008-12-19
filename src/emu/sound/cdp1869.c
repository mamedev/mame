/***************************************************************************

    cdp1869.c

    Sound handler

    TODO:

    - white noise

****************************************************************************/

#include "sndintrf.h"
#include "streams.h"
#include "sound/cdp1869.h"

struct CDP1869
{
	const device_config *device;
	sound_stream *stream;	/* returned by stream_create() */
	int clock;			/* chip's base frequency */

	int incr;				/* initial wave state */
	INT16 signal;			/* current signal */

	int toneoff;
	int wnoff;

	UINT8 tonediv;
	UINT8 tonefreq;
	UINT8 toneamp;

	UINT8 wnfreq;
	UINT8 wnamp;
};

/*************************************
 *
 *  Stream update
 *
 *************************************/

static STREAM_UPDATE( cdp1869_update )
{
	struct CDP1869 *info = param;
	INT16 signal = info->signal;
	stream_sample_t *buffer = outputs[0];

	memset( buffer, 0, samples * sizeof(*buffer) );

	if (!info->toneoff && info->toneamp)
	{
		double frequency = (info->clock / 2) / (512 >> info->tonefreq) / (info->tonediv + 1);
//      double amplitude = info->toneamp * ((0.78*5) / 15);

		int rate = info->device->machine->sample_rate / 2;

		/* get progress through wave */
		int incr = info->incr;

		if (signal < 0)
		{
			signal = -(info->toneamp * (0x07fff / 15));
		}
		else
		{
			signal = info->toneamp * (0x07fff / 15);
		}

		while( samples-- > 0 )
		{
			*buffer++ = signal;
			incr -= frequency;
			while( incr < 0 )
			{
				incr += rate;
				signal = -signal;
			}
		}

		/* store progress through wave */
		info->incr = incr;
		info->signal = signal;
	}
/*
    if (!info->wnoff)
    {
        double amplitude = info->wnamp * ((0.78*5) / 15);

        for (int wndiv = 0; wndiv < 128; wndiv++)
        {
            double frequency = (info->clock / 2) / (4096 >> info->wnfreq) / (wndiv + 1):

            cdp1869_sum_square_wave(buffer, frequency, amplitude);
        }
    }
*/
}

/*************************************
 *
 *  Sound handler start
 *
 *************************************/

static SND_START( cdp1869 )
{
	struct CDP1869 *info;

	info = auto_malloc(sizeof(*info));
	memset(info, 0, sizeof(*info));

	info->device = device;
	info->stream = stream_create(device, 0, 1, device->machine->sample_rate, info, cdp1869_update );
	info->incr = 0;
	info->signal = 0x07fff;

	info->clock = clock;
	info->toneoff = 1;
	info->wnoff = 1;
	info->tonediv = 0;
	info->tonefreq = 0;
	info->toneamp = 0;
	info->wnfreq = 0;
	info->wnamp = 0;

	return info;
}

void cdp1869_set_toneamp(int which, int value)
{
	struct CDP1869 *info = sndti_token(SOUND_CDP1869, which);
	info->toneamp = value & 0x0f;
}

void cdp1869_set_tonefreq(int which, int value)
{
	struct CDP1869 *info = sndti_token(SOUND_CDP1869, which);
	info->tonefreq = value & 0x07;
}

void cdp1869_set_toneoff(int which, int value)
{
	struct CDP1869 *info = sndti_token(SOUND_CDP1869, which);
	info->toneoff = value & 0x01;
}

void cdp1869_set_tonediv(int which, int value)
{
	struct CDP1869 *info = sndti_token(SOUND_CDP1869, which);
	info->tonediv = value & 0x7f;
}

void cdp1869_set_wnamp(int which, int value)
{
	struct CDP1869 *info = sndti_token(SOUND_CDP1869, which);
	info->wnamp = value & 0x0f;
}

void cdp1869_set_wnfreq(int which, int value)
{
	struct CDP1869 *info = sndti_token(SOUND_CDP1869, which);
	info->wnfreq = value & 0x07;
}

void cdp1869_set_wnoff(int which, int value)
{
	struct CDP1869 *info = sndti_token(SOUND_CDP1869, which);
	info->wnoff = value & 0x01;
}

/**************************************************************************
 * Generic get_info
 **************************************************************************/

static SND_SET_INFO( cdp1869 )
{
	switch (state)
	{
		/* no parameters to set */
	}
}

SND_GET_INFO( cdp1869 )
{
	switch (state)
	{
		/* --- the following bits of info are returned as 64-bit signed integers --- */

		/* --- the following bits of info are returned as pointers to data or functions --- */
		case SNDINFO_PTR_SET_INFO:						info->set_info = SND_SET_INFO_NAME( cdp1869 );	break;
		case SNDINFO_PTR_START:							info->start = SND_START_NAME( cdp1869 );		break;
		case SNDINFO_PTR_STOP:							/* nothing */									break;
		case SNDINFO_PTR_RESET:							/* nothing */									break;

		/* --- the following bits of info are returned as NULL-terminated strings --- */
		case SNDINFO_STR_NAME:							strcpy(info->s, "CDP1869");						break;
		case SNDINFO_STR_CORE_FAMILY:					strcpy(info->s, "RCA CDP1869");					break;
		case SNDINFO_STR_CORE_VERSION:					strcpy(info->s, "1.0");							break;
		case SNDINFO_STR_CORE_FILE:						strcpy(info->s, __FILE__);						break;
		case SNDINFO_STR_CORE_CREDITS:					strcpy(info->s, "Copyright Nicola Salmoria and the MAME Team"); break;
	}
}
