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
typedef struct _tms5220_state tms5220_state;
struct _tms5220_state
{
	const tms5220_interface *intf;
	sound_stream *stream;
	int clock;
	void *chip;
};


INLINE tms5220_state *get_safe_token(const device_config *device)
{
	assert(device != NULL);
	assert(device->token != NULL);
	assert(device->type == SOUND);
	assert(sound_get_type(device) == SOUND_TMS5220 ||
		   sound_get_type(device) == SOUND_TMC0285 ||
		   sound_get_type(device) == SOUND_TMS5200);
	return (tms5220_state *)device->token;
}


/* static function prototypes */
static STREAM_UPDATE( tms5220_update );



/**********************************************************************************************

     DEVICE_START( tms5220 ) -- allocate buffers and reset the 5220

***********************************************************************************************/

static DEVICE_START( tms5220 )
{
	static const tms5220_interface dummy = { 0 };
	tms5220_state *info = get_safe_token(device);

	info->intf = device->static_config ? (const tms5220_interface *)device->static_config : &dummy;

	info->chip = tms5220_create(device);
	assert_always(info->chip != NULL, "Error creating TMS5220 chip");

	/* initialize a info->stream */
	info->stream = stream_create(device, 0, 1, device->clock / 80, info, tms5220_update);
	info->clock = device->clock;

    /* reset the 5220 */
    tms5220_reset_chip(info->chip);
    tms5220_set_irq(info->chip, info->intf->irq);

	/* init the speech ROM handlers */
	tms5220_set_read(info->chip, info->intf->read);
	tms5220_set_load_address(info->chip, info->intf->load_address);
	tms5220_set_read_and_branch(info->chip, info->intf->read_and_branch);
}


#if (HAS_TMC0285 || HAS_TMS5200)
static DEVICE_START( tms5200 )
{
	tms5220_state *info = get_safe_token(device);
	DEVICE_START_CALL( tms5220 );
	tms5220_set_variant(info->chip, variant_tmc0285);
}
#endif /* (HAS_TMC0285) && (HAS_TMS5200) */



/**********************************************************************************************

     DEVICE_STOP( tms5220 ) -- free buffers

***********************************************************************************************/

static DEVICE_STOP( tms5220 )
{
	tms5220_state *info = get_safe_token(device);
	tms5220_destroy(info->chip);
}



static DEVICE_RESET( tms5220 )
{
	tms5220_state *info = get_safe_token(device);
	tms5220_reset_chip(info->chip);
}



/**********************************************************************************************

     tms5220_data_w -- write data to the sound chip

***********************************************************************************************/

WRITE8_DEVICE_HANDLER( tms5220_data_w )
{
	tms5220_state *info = get_safe_token(device);
    /* bring up to date first */
    stream_update(info->stream);
    tms5220_data_write(info->chip, data);
}



/**********************************************************************************************

     tms5220_status_r -- read status or data from the sound chip

***********************************************************************************************/

READ8_DEVICE_HANDLER( tms5220_status_r )
{
	tms5220_state *info = get_safe_token(device);
    /* bring up to date first */
    stream_update(info->stream);
    return tms5220_status_read(info->chip);
}



/**********************************************************************************************

     tms5220_ready_r -- return the not ready status from the sound chip

***********************************************************************************************/

int tms5220_ready_r(const device_config *device)
{
	tms5220_state *info = get_safe_token(device);
    /* bring up to date first */
    stream_update(info->stream);
    return tms5220_ready_read(info->chip);
}



/**********************************************************************************************

     tms5220_time_to_ready -- return the time in seconds until the ready line is asserted

***********************************************************************************************/

double tms5220_time_to_ready(const device_config *device)
{
	tms5220_state *info = get_safe_token(device);
	double cycles;

	/* bring up to date first */
	stream_update(info->stream);
	cycles = tms5220_cycles_to_ready(info->chip);
	return cycles * 80.0 / info->clock;
}



/**********************************************************************************************

     tms5220_int_r -- return the int status from the sound chip

***********************************************************************************************/

int tms5220_int_r(const device_config *device)
{
	tms5220_state *info = get_safe_token(device);
    /* bring up to date first */
    stream_update(info->stream);
    return tms5220_int_read(info->chip);
}



/**********************************************************************************************

     tms5220_update -- update the sound chip so that it is in sync with CPU execution

***********************************************************************************************/

static STREAM_UPDATE( tms5220_update )
{
	tms5220_state *info = (tms5220_state *)param;
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

void tms5220_set_frequency(const device_config *device, int frequency)
{
	tms5220_state *info = get_safe_token(device);
	stream_set_sample_rate(info->stream, frequency / 80);
	info->clock = frequency;
}



/**************************************************************************
 * Generic get_info
 **************************************************************************/

DEVICE_GET_INFO( tms5220 )
{
	switch (state)
	{
		/* --- the following bits of info are returned as 64-bit signed integers --- */
		case DEVINFO_INT_TOKEN_BYTES:					info->i = sizeof(tms5220_state);			break;

		/* --- the following bits of info are returned as pointers to data or functions --- */
		case DEVINFO_FCT_START:							info->start = DEVICE_START_NAME( tms5220 );		break;
		case DEVINFO_FCT_STOP:							info->stop = DEVICE_STOP_NAME( tms5220 );			break;
		case DEVINFO_FCT_RESET:							info->reset = DEVICE_RESET_NAME( tms5220 );		break;

		/* --- the following bits of info are returned as NULL-terminated strings --- */
		case DEVINFO_STR_NAME:							strcpy(info->s, "TMS5220");						break;
		case DEVINFO_STR_FAMILY:					strcpy(info->s, "TI Speech");					break;
		case DEVINFO_STR_VERSION:					strcpy(info->s, "1.0");							break;
		case DEVINFO_STR_SOURCE_FILE:						strcpy(info->s, __FILE__);						break;
		case DEVINFO_STR_CREDITS:					strcpy(info->s, "Copyright Nicola Salmoria and the MAME Team"); break;
	}
}

#if (HAS_TMC0285)
DEVICE_GET_INFO( tmc0285 )
{
	switch (state)
	{
		case DEVINFO_FCT_START:							info->start = DEVICE_START_NAME( tms5200 );		break;
		case DEVINFO_STR_NAME:							strcpy(info->s, "TMC0285");						break;
		default: 										DEVICE_GET_INFO_CALL( tms5220 );					break;
	}
}
#endif

#if (HAS_TMS5200)
DEVICE_GET_INFO( tms5200 )
{
	switch (state)
	{
		case DEVINFO_FCT_START:							info->start = DEVICE_START_NAME( tms5200 );		break;
		case DEVINFO_STR_NAME:							strcpy(info->s, "TMS5200");						break;
		default: 										DEVICE_GET_INFO_CALL( tms5220 );					break;
	}
}
#endif
