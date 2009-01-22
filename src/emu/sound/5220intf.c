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
	const tms5220_interface *intf;
	sound_stream *stream;
	int clock;
	void *chip;
};


/* static function prototypes */
static STREAM_UPDATE( tms5220_update );



/**********************************************************************************************

     SND_START( tms5220 ) -- allocate buffers and reset the 5220

***********************************************************************************************/

static SND_START( tms5220 )
{
	static const tms5220_interface dummy = { 0 };
	struct tms5220_info *info = device->token;

	info->intf = device->static_config ? device->static_config : &dummy;

	info->chip = tms5220_create(device);
	assert_always(info->chip != NULL, "Error creating TMS5220 chip");

	/* initialize a info->stream */
	info->stream = stream_create(device, 0, 1, clock / 80, info, tms5220_update);
	info->clock = clock;

    /* reset the 5220 */
    tms5220_reset_chip(info->chip);
    tms5220_set_irq(info->chip, info->intf->irq);

	/* init the speech ROM handlers */
	tms5220_set_read(info->chip, info->intf->read);
	tms5220_set_load_address(info->chip, info->intf->load_address);
	tms5220_set_read_and_branch(info->chip, info->intf->read_and_branch);
}


#if (HAS_TMC0285 || HAS_TMS5200)
static SND_START( tms5200 )
{
	struct tms5220_info *info = device->token;
	SND_START_CALL( tms5220 );
	tms5220_set_variant(info->chip, variant_tmc0285);
}
#endif /* (HAS_TMC0285) && (HAS_TMS5200) */



/**********************************************************************************************

     SND_STOP( tms5220 ) -- free buffers

***********************************************************************************************/

static SND_STOP( tms5220 )
{
	struct tms5220_info *info = device->token;
	tms5220_destroy(info->chip);
}



static SND_RESET( tms5220 )
{
	struct tms5220_info *info = device->token;
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

static STREAM_UPDATE( tms5220_update )
{
	struct tms5220_info *info = param;
	INT16 sample_data[MAX_SAMPLE_CHUNK];
	stream_sample_t *buffer = outputs[0];

	/* loop while we still have samples to generate */
	while (samples)
	{
		int length = (samples > MAX_SAMPLE_CHUNK) ? MAX_SAMPLE_CHUNK : samples;
		int index;

		/* generate the samples and copy to the target buffer */
		tms5220_process(info->chip, sample_data, length);
		for (index = 0; index < length; index++)
			*buffer++ = sample_data[index];

		/* account for the samples */
		samples -= length;
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

static SND_SET_INFO( tms5220 )
{
	struct tms5220_info *ti = device->token;

	switch (state)
	{
		case SNDINFO_INT_TMS5220_VARIANT:				tms5220_set_variant(ti->chip, (tms5220_variant) info->i); break;
	}
}


SND_GET_INFO( tms5220 )
{
	switch (state)
	{
		/* --- the following bits of info are returned as 64-bit signed integers --- */
		case SNDINFO_INT_TOKEN_BYTES:					info->i = sizeof(struct tms5220_info);			break;
		case SNDINFO_FCT_ALIAS:							info->type = SOUND_TMS5220;						break;

		/* --- the following bits of info are returned as pointers to data or functions --- */
		case SNDINFO_PTR_SET_INFO:						info->set_info = SND_SET_INFO_NAME( tms5220 );	break;
		case SNDINFO_PTR_START:							info->start = SND_START_NAME( tms5220 );		break;
		case SNDINFO_PTR_STOP:							info->stop = SND_STOP_NAME( tms5220 );			break;
		case SNDINFO_PTR_RESET:							info->reset = SND_RESET_NAME( tms5220 );		break;

		/* --- the following bits of info are returned as NULL-terminated strings --- */
		case SNDINFO_STR_NAME:							strcpy(info->s, "TMS5220");						break;
		case SNDINFO_STR_CORE_FAMILY:					strcpy(info->s, "TI Speech");					break;
		case SNDINFO_STR_CORE_VERSION:					strcpy(info->s, "1.0");							break;
		case SNDINFO_STR_CORE_FILE:						strcpy(info->s, __FILE__);						break;
		case SNDINFO_STR_CORE_CREDITS:					strcpy(info->s, "Copyright Nicola Salmoria and the MAME Team"); break;
	}
}

#if (HAS_TMC0285)
SND_GET_INFO( tmc0285 )
{
	switch (state)
	{
		case SNDINFO_PTR_START:							info->start = SND_START_NAME( tms5200 );		break;
		case SNDINFO_STR_NAME:							strcpy(info->s, "TMC0285");						break;
		default: 										SND_GET_INFO_CALL( tms5220 );					break;
	}
}
#endif

#if (HAS_TMS5200)
SND_GET_INFO( tms5200 )
{
	switch (state)
	{
		case SNDINFO_PTR_START:							info->start = SND_START_NAME( tms5200 );		break;
		case SNDINFO_STR_NAME:							strcpy(info->s, "TMS5200");						break;
		default: 										SND_GET_INFO_CALL( tms5220 );					break;
	}
}
#endif
