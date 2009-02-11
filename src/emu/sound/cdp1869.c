/***************************************************************************

    cdp1869.c

    Sound handler

    TODO:

    - white noise

****************************************************************************/

#include "sndintrf.h"
#include "streams.h"
#include "sound/cdp1869.h"

typedef struct _cdp1869_state cdp1869_state;
struct _cdp1869_state
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

INLINE cdp1869_state *get_safe_token(const device_config *device)
{
	assert(device != NULL);
	assert(device->token != NULL);
	assert(device->type == SOUND);
	assert(sound_get_type(device) == SOUND_CDP1869);
	return (cdp1869_state *)device->token;
}

/*************************************
 *
 *  Stream update
 *
 *************************************/

static STREAM_UPDATE( cdp1869_update )
{
	cdp1869_state *info = param;
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

static DEVICE_START( cdp1869 )
{
	cdp1869_state *info = get_safe_token(device);

	info->device = device;
	info->stream = stream_create(device, 0, 1, device->machine->sample_rate, info, cdp1869_update );
	info->incr = 0;
	info->signal = 0x07fff;

	info->clock = device->clock;
	info->toneoff = 1;
	info->wnoff = 1;
	info->tonediv = 0;
	info->tonefreq = 0;
	info->toneamp = 0;
	info->wnfreq = 0;
	info->wnamp = 0;
}

void cdp1869_set_toneamp(const device_config *device, int value)
{
	cdp1869_state *info = get_safe_token(device);
	info->toneamp = value & 0x0f;
}

void cdp1869_set_tonefreq(const device_config *device, int value)
{
	cdp1869_state *info = get_safe_token(device);
	info->tonefreq = value & 0x07;
}

void cdp1869_set_toneoff(const device_config *device, int value)
{
	cdp1869_state *info = get_safe_token(device);
	info->toneoff = value & 0x01;
}

void cdp1869_set_tonediv(const device_config *device, int value)
{
	cdp1869_state *info = get_safe_token(device);
	info->tonediv = value & 0x7f;
}

void cdp1869_set_wnamp(const device_config *device, int value)
{
	cdp1869_state *info = get_safe_token(device);
	info->wnamp = value & 0x0f;
}

void cdp1869_set_wnfreq(const device_config *device, int value)
{
	cdp1869_state *info = get_safe_token(device);
	info->wnfreq = value & 0x07;
}

void cdp1869_set_wnoff(const device_config *device, int value)
{
	cdp1869_state *info = get_safe_token(device);
	info->wnoff = value & 0x01;
}

/**************************************************************************
 * Generic get_info
 **************************************************************************/

DEVICE_GET_INFO( cdp1869 )
{
	switch (state)
	{
		/* --- the following bits of info are returned as 64-bit signed integers --- */
		case DEVINFO_INT_TOKEN_BYTES:					info->i = sizeof(cdp1869_state);				break;

		/* --- the following bits of info are returned as pointers to data or functions --- */
		case DEVINFO_FCT_START:							info->start = DEVICE_START_NAME( cdp1869 );		break;
		case DEVINFO_FCT_STOP:							/* nothing */									break;
		case DEVINFO_FCT_RESET:							/* nothing */									break;

		/* --- the following bits of info are returned as NULL-terminated strings --- */
		case DEVINFO_STR_NAME:							strcpy(info->s, "CDP1869");						break;
		case DEVINFO_STR_FAMILY:					strcpy(info->s, "RCA CDP1869");					break;
		case DEVINFO_STR_VERSION:					strcpy(info->s, "1.0");							break;
		case DEVINFO_STR_SOURCE_FILE:						strcpy(info->s, __FILE__);						break;
		case DEVINFO_STR_CREDITS:					strcpy(info->s, "Copyright Nicola Salmoria and the MAME Team"); break;
	}
}
