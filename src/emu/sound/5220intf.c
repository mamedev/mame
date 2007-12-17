/**********************************************************************************************

     TMS5220 interface

     Written for MAME by Frank Palazzolo
     With help from Neill Corlett
     Additional tweaking by Aaron Giles
     Speech ROM support and a few bug fixes by R Nabet

***********************************************************************************************/

#include <math.h>

#include "sndintrf.h"
#include "streams.h"
#include "tms5220.h"
#include "5220intf.h"


#define MAX_SAMPLE_CHUNK	10000


/* the state of the streamed output */
struct tms5220_info
{
	const struct TMS5220interface *intf;
	sound_stream *stream;
	int clock;
	void *chip;
};


/* static function prototypes */
static void tms5220_update(void *param, stream_sample_t **inputs, stream_sample_t **buffer, int length);



/**********************************************************************************************

     tms5220_start -- allocate buffers and reset the 5220

***********************************************************************************************/

static void *tms5220_start(int sndindex, int clock, const void *config)
{
	static const struct TMS5220interface dummy = { 0 };
	struct tms5220_info *info;

	info = auto_malloc(sizeof(*info));
	memset(info, 0, sizeof(*info));
	info->intf = config ? config : &dummy;

	info->chip = tms5220_create(sndindex);
	if (!info->chip)
		return NULL;
	sndintrf_register_token(info);

	/* initialize a info->stream */
	info->stream = stream_create(0, 1, clock / 80, info, tms5220_update);
	info->clock = clock;

    /* reset the 5220 */
    tms5220_reset_chip(info->chip);
    tms5220_set_irq(info->chip, info->intf->irq);

	/* init the speech ROM handlers */
	tms5220_set_read(info->chip, info->intf->read);
	tms5220_set_load_address(info->chip, info->intf->load_address);
	tms5220_set_read_and_branch(info->chip, info->intf->read_and_branch);

    /* request a sound channel */
    return info;
}



/**********************************************************************************************

     tms5220_stop -- free buffers

***********************************************************************************************/

static void tms5220_stop(void *chip)
{
	struct tms5220_info *info = chip;
	tms5220_destroy(info->chip);
}



static void tms5220_reset(void *chip)
{
	struct tms5220_info *info = chip;
	tms5220_reset_chip(info->chip);
}



/**********************************************************************************************

     tms5220_data_w -- write data to the sound chip

***********************************************************************************************/

WRITE8_HANDLER( tms5220_data_w )
{
	struct tms5220_info *info = sndti_token(SOUND_TMS5220, 0);
    /* bring up to date first */
    stream_update(info->stream);
    tms5220_data_write(info->chip, data);
}



/**********************************************************************************************

     tms5220_status_r -- read status or data from the sound chip

***********************************************************************************************/

READ8_HANDLER( tms5220_status_r )
{
	struct tms5220_info *info = sndti_token(SOUND_TMS5220, 0);
    /* bring up to date first */
    stream_update(info->stream);
    return tms5220_status_read(info->chip);
}



/**********************************************************************************************

     tms5220_ready_r -- return the not ready status from the sound chip

***********************************************************************************************/

int tms5220_ready_r(void)
{
	struct tms5220_info *info = sndti_token(SOUND_TMS5220, 0);
    /* bring up to date first */
    stream_update(info->stream);
    return tms5220_ready_read(info->chip);
}



/**********************************************************************************************

     tms5220_ready_r -- return the time in seconds until the ready line is asserted

***********************************************************************************************/

double tms5220_time_to_ready(void)
{
	struct tms5220_info *info = sndti_token(SOUND_TMS5220, 0);
	double cycles;

	/* bring up to date first */
	stream_update(info->stream);
	cycles = tms5220_cycles_to_ready(info->chip);
	return cycles * 80.0 / info->clock;
}



/**********************************************************************************************

     tms5220_int_r -- return the int status from the sound chip

***********************************************************************************************/

int tms5220_int_r(void)
{
	struct tms5220_info *info = sndti_token(SOUND_TMS5220, 0);
    /* bring up to date first */
    stream_update(info->stream);
    return tms5220_int_read(info->chip);
}



/**********************************************************************************************

     tms5220_update -- update the sound chip so that it is in sync with CPU execution

***********************************************************************************************/

static void tms5220_update(void *param, stream_sample_t **inputs, stream_sample_t **_buffer, int length)
{
	struct tms5220_info *info = param;
	INT16 sample_data[MAX_SAMPLE_CHUNK];
	stream_sample_t *buffer = _buffer[0];

	/* loop while we still have samples to generate */
	while (length)
	{
		int samples = (length > MAX_SAMPLE_CHUNK) ? MAX_SAMPLE_CHUNK : length;
		int index;

		/* generate the samples and copy to the target buffer */
		tms5220_process(info->chip, sample_data, samples);
		for (index = 0; index < samples; index++)
			*buffer++ = sample_data[index];

		/* account for the samples */
		length -= samples;
	}
}



/**********************************************************************************************

     tms5220_set_frequency -- adjusts the playback frequency

***********************************************************************************************/

void tms5220_set_frequency(int frequency)
{
	struct tms5220_info *info = sndti_token(SOUND_TMS5220, 0);
	stream_set_sample_rate(info->stream, frequency / 80);
	info->clock = frequency;
}



/**************************************************************************
 * Generic get_info
 **************************************************************************/

static void tms5220_set_info(void *token, UINT32 state, sndinfo *info)
{
	struct tms5220_info *ti = token;

	switch (state)
	{
		case SNDINFO_INT_TMS5220_VARIANT:				tms5220_set_variant(ti->chip, (tms5220_variant) info->i); break;
	}
}


void tms5220_get_info(void *token, UINT32 state, sndinfo *info)
{
	switch (state)
	{
		/* --- the following bits of info are returned as 64-bit signed integers --- */

		/* --- the following bits of info are returned as pointers to data or functions --- */
		case SNDINFO_PTR_SET_INFO:						info->set_info = tms5220_set_info;		break;
		case SNDINFO_PTR_START:							info->start = tms5220_start;			break;
		case SNDINFO_PTR_STOP:							info->stop = tms5220_stop;				break;
		case SNDINFO_PTR_RESET:							info->reset = tms5220_reset;			break;

		/* --- the following bits of info are returned as NULL-terminated strings --- */
		case SNDINFO_STR_NAME:							info->s = "TMS5220";					break;
		case SNDINFO_STR_CORE_FAMILY:					info->s = "TI Speech";					break;
		case SNDINFO_STR_CORE_VERSION:					info->s = "1.0";						break;
		case SNDINFO_STR_CORE_FILE:						info->s = __FILE__;						break;
		case SNDINFO_STR_CORE_CREDITS:					info->s = "Copyright (c) 2004, The MAME Team"; break;
	}
}

