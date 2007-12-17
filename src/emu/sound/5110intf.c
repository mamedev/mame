/******************************************************************************

     TMS5110 interface

     slightly modified from 5220intf by Jarek Burczynski

     Written for MAME by Frank Palazzolo
     With help from Neill Corlett
     Additional tweaking by Aaron Giles

******************************************************************************/

#include <math.h>

#include "sndintrf.h"
#include "streams.h"
#include "tms5110.h"
#include "5110intf.h"


#define MAX_SAMPLE_CHUNK	10000


/* the state of the streamed output */
struct tms5110_info
{
	const struct TMS5110interface *intf;
	sound_stream *stream;
	void *chip;
};


/* static function prototypes */
static void tms5110_update(void *param, stream_sample_t **inputs, stream_sample_t **buffer, int length);



/******************************************************************************

     tms5110_start -- allocate buffers and reset the 5110

******************************************************************************/

static void *tms5110_start(int sndindex, int clock, const void *config)
{
	static const struct TMS5110interface dummy = { 0 };
	struct tms5110_info *info;

	info = auto_malloc(sizeof(*info));
	memset(info, 0, sizeof(*info));
	info->intf = config ? config : &dummy;

	info->chip = tms5110_create(sndindex);
	if (!info->chip)
		return NULL;
	sndintrf_register_token(info);

	/* initialize a stream */
	info->stream = stream_create(0, 1, clock / 80, info, tms5110_update);

    if (info->intf->M0_callback==NULL)
    {
		logerror("\n file: 5110intf.c, tms5110_start(), line 53:\n  Missing _mandatory_ 'M0_callback' function pointer in the TMS5110 interface\n  This function is used by TMS5110 to call for a single bits\n  needed to generate the speech\n  Aborting startup...\n");
		return NULL;
    }
    tms5110_set_M0_callback(info->chip, info->intf->M0_callback );

    /* reset the 5110 */
    tms5110_reset_chip(info->chip);

    /* request a sound channel */
    return info;
}



/******************************************************************************

     tms5110_stop -- free buffers

******************************************************************************/

static void tms5110_stop(void *chip)
{
	struct tms5110_info *info = chip;
	tms5110_destroy(info->chip);
}


static void tms5110_reset(void *chip)
{
	struct tms5110_info *info = chip;
	tms5110_reset_chip(info->chip);
}



/******************************************************************************

     tms5110_CTL_w -- write Control Command to the sound chip
commands like Speech, Reset, etc., are loaded into the chip via the CTL pins

******************************************************************************/

WRITE8_HANDLER( tms5110_CTL_w )
{
	struct tms5110_info *info = sndti_token(SOUND_TMS5110, 0);

    /* bring up to date first */
    stream_update(info->stream);
    tms5110_CTL_set(info->chip, data);
}

/******************************************************************************

     tms5110_PDC_w -- write to PDC pin on the sound chip

******************************************************************************/

WRITE8_HANDLER( tms5110_PDC_w )
{
	struct tms5110_info *info = sndti_token(SOUND_TMS5110, 0);

    /* bring up to date first */
    stream_update(info->stream);
    tms5110_PDC_set(info->chip, data);
}



/******************************************************************************

     tms5110_status_r -- read status from the sound chip

******************************************************************************/

READ8_HANDLER( tms5110_status_r )
{
	struct tms5110_info *info = sndti_token(SOUND_TMS5110, 0);

    /* bring up to date first */
    stream_update(info->stream);
    return tms5110_status_read(info->chip);
}



/******************************************************************************

     tms5110_ready_r -- return the not ready status from the sound chip

******************************************************************************/

int tms5110_ready_r(void)
{
	struct tms5110_info *info = sndti_token(SOUND_TMS5110, 0);

    /* bring up to date first */
    stream_update(info->stream);
    return tms5110_ready_read(info->chip);
}



/******************************************************************************

     tms5110_update -- update the sound chip so that it is in sync with CPU execution

******************************************************************************/

static void tms5110_update(void *param, stream_sample_t **inputs, stream_sample_t **_buffer, int length)
{
	struct tms5110_info *info = param;
	INT16 sample_data[MAX_SAMPLE_CHUNK];
	stream_sample_t *buffer = _buffer[0];

	/* loop while we still have samples to generate */
	while (length)
	{
		int samples = (length > MAX_SAMPLE_CHUNK) ? MAX_SAMPLE_CHUNK : length;
		int index;

		/* generate the samples and copy to the target buffer */
		tms5110_process(info->chip, sample_data, samples);
		for (index = 0; index < samples; index++)
			*buffer++ = sample_data[index];

		/* account for the samples */
		length -= samples;
	}
}



/******************************************************************************

     tms5110_set_frequency -- adjusts the playback frequency

******************************************************************************/

void tms5110_set_frequency(int frequency)
{
	struct tms5110_info *info = sndti_token(SOUND_TMS5110, 0);
	stream_set_sample_rate(info->stream, frequency / 80);
}




/**************************************************************************
 * Generic get_info
 **************************************************************************/

static void tms5110_set_info(void *token, UINT32 state, sndinfo *info)
{
	switch (state)
	{
		/* no parameters to set */
	}
}


void tms5110_get_info(void *token, UINT32 state, sndinfo *info)
{
	switch (state)
	{
		/* --- the following bits of info are returned as 64-bit signed integers --- */

		/* --- the following bits of info are returned as pointers to data or functions --- */
		case SNDINFO_PTR_SET_INFO:						info->set_info = tms5110_set_info;		break;
		case SNDINFO_PTR_START:							info->start = tms5110_start;			break;
		case SNDINFO_PTR_STOP:							info->stop = tms5110_stop;				break;
		case SNDINFO_PTR_RESET:							info->reset = tms5110_reset;			break;

		/* --- the following bits of info are returned as NULL-terminated strings --- */
		case SNDINFO_STR_NAME:							info->s = "TMS5110";					break;
		case SNDINFO_STR_CORE_FAMILY:					info->s = "TI Speech";					break;
		case SNDINFO_STR_CORE_VERSION:					info->s = "1.0";						break;
		case SNDINFO_STR_CORE_FILE:						info->s = __FILE__;						break;
		case SNDINFO_STR_CORE_CREDITS:					info->s = "Copyright (c) 2004, The MAME Team"; break;
	}
}

