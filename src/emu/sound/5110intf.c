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
typedef struct _tms5110_state tms5110_state;
struct _tms5110_state
{
	const tms5110_interface *intf;
	const UINT8 *table;
	sound_stream *stream;
	void *chip;
	INT32 speech_rom_bitnum;
};


INLINE tms5110_state *get_safe_token(const device_config *device)
{
	assert(device != NULL);
	assert(device->token != NULL);
	assert(device->type == SOUND);
	assert(sound_get_type(device) == SOUND_TMS5110 ||
		   sound_get_type(device) == SOUND_TMS5100 ||
		   sound_get_type(device) == SOUND_TMS5110A ||
		   sound_get_type(device) == SOUND_CD2801 ||
		   sound_get_type(device) == SOUND_TMC0281 ||
		   sound_get_type(device) == SOUND_CD2802 ||
		   sound_get_type(device) == SOUND_M58817);
	return (tms5110_state *)device->token;
}


/* static function prototypes */
static STREAM_UPDATE( tms5110_update );

static int speech_rom_read_bit(const device_config *device)
{
	tms5110_state *info = get_safe_token(device);

	int r;

	if (info->speech_rom_bitnum<0)
		r = 0;
	else
		r = (info->table[info->speech_rom_bitnum >> 3] >> (0x07 - (info->speech_rom_bitnum & 0x07))) & 1;

	info->speech_rom_bitnum++;

	return r;
}

static void speech_rom_set_addr(const device_config *device, int addr)
{
	tms5110_state *info = get_safe_token(device);

	info->speech_rom_bitnum = addr * 8 - 1;
}

/******************************************************************************

     DEVICE_START( tms5110 ) -- allocate buffers and reset the 5110

******************************************************************************/

static DEVICE_START( tms5110 )
{
	static const tms5110_interface dummy = { 0 };
	tms5110_state *info = get_safe_token(device);

	info->intf = device->static_config ? (const tms5110_interface *)device->static_config : &dummy;
	info->table = device->region;

	info->chip = tms5110_create(device, TMS5110_IS_5110A);
	assert_always(info->chip != NULL, "Error creating TMS5110 chip");

	/* initialize a stream */
	info->stream = stream_create(device, 0, 1, device->clock / 80, info, tms5110_update);

	if (info->table == NULL)
	{
		assert_always(info->intf->M0_callback != NULL, "Missing _mandatory_ 'M0_callback' function pointer in the TMS5110 interface\n  This function is used by TMS5110 to call for a single bits\n  needed to generate the speech\n  Aborting startup...\n");
	    tms5110_set_M0_callback(info->chip, info->intf->M0_callback );
	    tms5110_set_load_address(info->chip, info->intf->load_address );
	}
	else
	{
	    tms5110_set_M0_callback(info->chip, speech_rom_read_bit );
	    tms5110_set_load_address(info->chip, speech_rom_set_addr );
	}

    /* reset the 5110 */
    tms5110_reset_chip(info->chip);
}

static DEVICE_START( tms5100 )
{
	tms5110_state *info = get_safe_token(device);
	DEVICE_START_CALL( tms5110 );
	tms5110_set_variant(info->chip, TMS5110_IS_5100);
}

static DEVICE_START( tms5110a )
{
	tms5110_state *info = get_safe_token(device);
	DEVICE_START_CALL( tms5110 );
	tms5110_set_variant(info->chip, TMS5110_IS_5110A);
}

static DEVICE_START( cd2801 )
{
	tms5110_state *info = get_safe_token(device);
	DEVICE_START_CALL( tms5110 );
	tms5110_set_variant(info->chip, TMS5110_IS_CD2801);
}

static DEVICE_START( tmc0281 )
{
	tms5110_state *info = get_safe_token(device);
	DEVICE_START_CALL( tms5110 );
	tms5110_set_variant(info->chip, TMS5110_IS_TMC0281);
}

static DEVICE_START( cd2802 )
{
	tms5110_state *info = get_safe_token(device);
	DEVICE_START_CALL( tms5110 );
	tms5110_set_variant(info->chip, TMS5110_IS_CD2802);
}

static DEVICE_START( m58817 )
{
	tms5110_state *info = get_safe_token(device);
	DEVICE_START_CALL( tms5110 );
	tms5110_set_variant(info->chip, TMS5110_IS_M58817);
}


/******************************************************************************

     DEVICE_STOP( tms5110 ) -- free buffers

******************************************************************************/

static DEVICE_STOP( tms5110 )
{
	tms5110_state *info = get_safe_token(device);
	tms5110_destroy(info->chip);
}


static DEVICE_RESET( tms5110 )
{
	tms5110_state *info = get_safe_token(device);
	tms5110_reset_chip(info->chip);
}



/******************************************************************************

     tms5110_ctl_w -- write Control Command to the sound chip
commands like Speech, Reset, etc., are loaded into the chip via the CTL pins

******************************************************************************/

WRITE8_DEVICE_HANDLER( tms5110_ctl_w )
{
	tms5110_state *info = get_safe_token(device);

    /* bring up to date first */
    stream_update(info->stream);
    tms5110_CTL_set(info->chip, data);
}

/******************************************************************************

     tms5110_pdc_w -- write to PDC pin on the sound chip

******************************************************************************/

WRITE8_DEVICE_HANDLER( tms5110_pdc_w )
{
	tms5110_state *info = get_safe_token(device);

    /* bring up to date first */
    stream_update(info->stream);
    tms5110_PDC_set(info->chip, data);
}



/******************************************************************************

     tms5110_status_r -- read status from the sound chip

******************************************************************************/

READ8_DEVICE_HANDLER( tms5110_status_r )
{
	tms5110_state *info = get_safe_token(device);

    /* bring up to date first */
    stream_update(info->stream);
    return tms5110_status_read(info->chip);
}



/******************************************************************************

     tms5110_ready_r -- return the not ready status from the sound chip

******************************************************************************/

int tms5110_ready_r(const device_config *device)
{
	tms5110_state *info = get_safe_token(device);

    /* bring up to date first */
    stream_update(info->stream);
    return tms5110_ready_read(info->chip);
}



/******************************************************************************

     tms5110_update -- update the sound chip so that it is in sync with CPU execution

******************************************************************************/

static STREAM_UPDATE( tms5110_update )
{
	tms5110_state *info = (tms5110_state *)param;
	INT16 sample_data[MAX_SAMPLE_CHUNK];
	stream_sample_t *buffer = outputs[0];

	/* loop while we still have samples to generate */
	while (samples)
	{
		int length = (samples > MAX_SAMPLE_CHUNK) ? MAX_SAMPLE_CHUNK : samples;
		int index;

		/* generate the samples and copy to the target buffer */
		tms5110_process(info->chip, sample_data, length);
		for (index = 0; index < length; index++)
			*buffer++ = sample_data[index];

		/* account for the samples */
		samples -= length;
	}
}



/******************************************************************************

     tms5110_set_frequency -- adjusts the playback frequency

******************************************************************************/

void tms5110_set_frequency(const device_config *device, int frequency)
{
	tms5110_state *info = get_safe_token(device);
	stream_set_sample_rate(info->stream, frequency / 80);
}




/**************************************************************************
 * Generic get_info
 **************************************************************************/

DEVICE_GET_INFO( tms5110 )
{
	switch (state)
	{
		/* --- the following bits of info are returned as 64-bit signed integers --- */
		case DEVINFO_INT_TOKEN_BYTES:					info->i = sizeof(tms5110_state);			break;

		/* --- the following bits of info are returned as pointers to data or functions --- */
		case DEVINFO_FCT_START:							info->start = DEVICE_START_NAME( tms5110 );		break;
		case DEVINFO_FCT_STOP:							info->stop = DEVICE_STOP_NAME( tms5110 );			break;
		case DEVINFO_FCT_RESET:							info->reset = DEVICE_RESET_NAME( tms5110 );		break;

		/* --- the following bits of info are returned as NULL-terminated strings --- */
		case DEVINFO_STR_NAME:							strcpy(info->s, "TMS5110");						break;
		case DEVINFO_STR_FAMILY:					strcpy(info->s, "TI Speech");					break;
		case DEVINFO_STR_VERSION:					strcpy(info->s, "1.0");							break;
		case DEVINFO_STR_SOURCE_FILE:						strcpy(info->s, __FILE__);						break;
		case DEVINFO_STR_CREDITS:					strcpy(info->s, "Copyright Nicola Salmoria and the MAME Team"); break;
	}
}

DEVICE_GET_INFO( tms5100 )
{
	switch (state)
	{
		case DEVINFO_FCT_START:							info->start = DEVICE_START_NAME( tms5100 );		break;
		case DEVINFO_STR_NAME:							strcpy(info->s, "TMS5100");						break;
		default: 										DEVICE_GET_INFO_CALL(tms5110);						break;
	}
}

DEVICE_GET_INFO( tms5110a )
{
	switch (state)
	{
		case DEVINFO_FCT_START:							info->start = DEVICE_START_NAME( tms5110a );		break;
		case DEVINFO_STR_NAME:							strcpy(info->s, "TMS5100A");					break;
		default: 										DEVICE_GET_INFO_CALL(tms5110);						break;
	}
}

DEVICE_GET_INFO( cd2801 )
{
	switch (state)
	{
		case DEVINFO_FCT_START:							info->start = DEVICE_START_NAME( cd2801 );			break;
		case DEVINFO_STR_NAME:							strcpy(info->s, "CD2801");						break;
		default: 										DEVICE_GET_INFO_CALL(tms5110);						break;
	}
}

DEVICE_GET_INFO( tmc0281 )
{
	switch (state)
	{
		case DEVINFO_FCT_START:							info->start = DEVICE_START_NAME( tmc0281 );		break;
		case DEVINFO_STR_NAME:							strcpy(info->s, "TMS5100");						break;
		default: 										DEVICE_GET_INFO_CALL(tms5110);						break;
	}
}

DEVICE_GET_INFO( cd2802 )
{
	switch (state)
	{
		case DEVINFO_FCT_START:							info->start = DEVICE_START_NAME( cd2802 );			break;
		case DEVINFO_STR_NAME:							strcpy(info->s, "CD2802");						break;
		default: 										DEVICE_GET_INFO_CALL(tms5110);						break;
	}
}

DEVICE_GET_INFO( m58817 )
{
	switch (state)
	{
		case DEVINFO_FCT_START:							info->start = DEVICE_START_NAME( m58817 );			break;
		case DEVINFO_STR_NAME:							strcpy(info->s, "M58817");						break;
		default: 										DEVICE_GET_INFO_CALL(tms5110);						break;
	}
}
