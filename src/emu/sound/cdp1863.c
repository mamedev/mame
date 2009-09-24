/**********************************************************************

    RCA CDP1863 CMOS 8-Bit Programmable Frequency Generator emulation

    Copyright MESS Team.
    Visit http://mamedev.org for licensing and usage restrictions.

**********************************************************************/

/*

    TODO:

    - what happens if you connect both clocks?

*/

#include "driver.h"
#include "sndintrf.h"
#include "streams.h"
#include "cpu/cdp1802/cdp1802.h"
#include "cdp1863.h"

/***************************************************************************
    PARAMETERS
***************************************************************************/

#define CDP1863_DEFAULT_LATCH	0x35

/***************************************************************************
    TYPE DEFINITIONS
***************************************************************************/

typedef struct _cdp1863_t cdp1863_t;
struct _cdp1863_t
{
	int clock1;						/* clock 1 */
	int clock2;						/* clock 2 */

	sound_stream *stream;			/* sound output */

	/* sound state */
	int oe;							/* output enable */
	int latch;						/* sound latch */
	INT16 signal;					/* current signal */
	int incr;						/* initial wave state */
};

/***************************************************************************
    INLINE FUNCTIONS
***************************************************************************/

INLINE cdp1863_t *get_safe_token(const device_config *device)
{
	assert(device != NULL);
	assert(device->token != NULL);
	return (cdp1863_t *)device->token;
}

INLINE cdp1863_config *get_safe_config(const device_config *device)
{
	assert(device != NULL);
	return (cdp1863_config *)device->inline_config;
}

/***************************************************************************
    IMPLEMENTATION
***************************************************************************/

/*-------------------------------------------------
    cdp1863_str_w - tone latch write
-------------------------------------------------*/

WRITE8_DEVICE_HANDLER( cdp1863_str_w )
{
	cdp1863_t *cdp1863 = get_safe_token(device);

	cdp1863->latch = data;
}

/*-------------------------------------------------
    cdp1863_oe_w - output enable write
------------------------------------------------*/

WRITE_LINE_DEVICE_HANDLER( cdp1863_oe_w )
{
	cdp1863_t *cdp1863 = get_safe_token(device);

	cdp1863->oe = state;
}

/*-------------------------------------------------
    cdp1863_set_clk1 - set clock 1 frequency
------------------------------------------------*/

void cdp1863_set_clk1(const device_config *device, int frequency)
{
	cdp1863_t *cdp1863 = get_safe_token(device);

	cdp1863->clock1 = frequency;
}

/*-------------------------------------------------
    cdp1863_set_clk2 - set clock 2 frequency
------------------------------------------------*/

void cdp1863_set_clk2(const device_config *device, int frequency)
{
	cdp1863_t *cdp1863 = get_safe_token(device);

	cdp1863->clock2 = frequency;
}

/*-------------------------------------------------
    STREAM_UPDATE( cdp1863_stream_update )
-------------------------------------------------*/

static STREAM_UPDATE( cdp1863_stream_update )
{
	cdp1863_t *cdp1863 = get_safe_token(device);

	INT16 signal = cdp1863->signal;
	stream_sample_t *buffer = outputs[0];

	memset( buffer, 0, samples * sizeof(*buffer) );

	if (cdp1863->oe)
	{
		double frequency;
		int rate = device->machine->sample_rate / 2;

		/* get progress through wave */
		int incr = cdp1863->incr;

		if (cdp1863->clock1 > 0)
		{
			/* CLK1 is pre-divided by 4 */
			frequency = cdp1863->clock1 / 4 / (cdp1863->latch + 1) / 2;
		}
		else
		{
			/* CLK2 is pre-divided by 8 */
			frequency = cdp1863->clock2 / 8 / (cdp1863->latch + 1) / 2;
		}

		if (signal < 0)
		{
			signal = -0x7fff;
		}
		else
		{
			signal = 0x7fff;
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
		cdp1863->incr = incr;
		cdp1863->signal = signal;
	}
}

/*-------------------------------------------------
    DEVICE_START( cdp1863 )
-------------------------------------------------*/

static DEVICE_START( cdp1863 )
{
	cdp1863_t *cdp1863 = get_safe_token(device);
	const cdp1863_config *config = get_safe_config(device);

	/* set initial values */
	cdp1863->stream = stream_create(device, 0, 1, device->machine->sample_rate, cdp1863, cdp1863_stream_update);
	cdp1863->clock1 = device->clock;
	cdp1863->clock2 = config->clock2;
	cdp1863->oe = 1;

	/* register for state saving */
	state_save_register_device_item(device, 0, cdp1863->clock1);
	state_save_register_device_item(device, 0, cdp1863->clock2);
	state_save_register_device_item(device, 0, cdp1863->oe);
	state_save_register_device_item(device, 0, cdp1863->latch);
	state_save_register_device_item(device, 0, cdp1863->signal);
	state_save_register_device_item(device, 0, cdp1863->incr);
}

/*-------------------------------------------------
    DEVICE_RESET( cdp1863 )
-------------------------------------------------*/

static DEVICE_RESET( cdp1863 )
{
	cdp1863_t *cdp1863 = get_safe_token(device);

	cdp1863->latch = CDP1863_DEFAULT_LATCH;
}

/*-------------------------------------------------
    DEVICE_GET_INFO( cdp1863 )
-------------------------------------------------*/

DEVICE_GET_INFO( cdp1863 )
{
	switch (state)
	{
		/* --- the following bits of info are returned as 64-bit signed integers --- */
		case DEVINFO_INT_TOKEN_BYTES:					info->i = sizeof(cdp1863_t);				break;
		case DEVINFO_INT_INLINE_CONFIG_BYTES:			info->i = sizeof(cdp1863_config);			break;

		/* --- the following bits of info are returned as pointers to data or functions --- */
		case DEVINFO_FCT_START:							info->start = DEVICE_START_NAME(cdp1863);	break;
		case DEVINFO_FCT_STOP:							/* Nothing */								break;
		case DEVINFO_FCT_RESET:							info->reset = DEVICE_RESET_NAME(cdp1863);	break;

		/* --- the following bits of info are returned as NULL-terminated strings --- */
		case DEVINFO_STR_NAME:							strcpy(info->s, "RCA CDP1863");				break;
		case DEVINFO_STR_FAMILY:						strcpy(info->s, "RCA CDP1800");				break;
		case DEVINFO_STR_VERSION:						strcpy(info->s, "1.0");						break;
		case DEVINFO_STR_SOURCE_FILE:					strcpy(info->s, __FILE__);					break;
		case DEVINFO_STR_CREDITS:						strcpy(info->s, "Copyright MESS Team");		break;
	}
}
