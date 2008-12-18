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
	const tms5110_interface *intf;
	const UINT8 *table;
	sound_stream *stream;
	void *chip;
	INT32 speech_rom_bitnum;
};


/* static function prototypes */
static STREAM_UPDATE( tms5110_update );

static int speech_rom_read_bit(void)
{
	struct tms5110_info *info = sndti_token(SOUND_TMS5110, 0);

	int r;

	if (info->speech_rom_bitnum<0)
		r = 0;
	else
		r = (info->table[info->speech_rom_bitnum >> 3] >> (0x07 - (info->speech_rom_bitnum & 0x07))) & 1;

	info->speech_rom_bitnum++;

	return r;
}

static void speech_rom_set_addr(int addr)
{
	struct tms5110_info *info = sndti_token(SOUND_TMS5110, 0);

	info->speech_rom_bitnum = addr * 8 - 1;
}

/******************************************************************************

     SND_START( tms5110 ) -- allocate buffers and reset the 5110

******************************************************************************/

static SND_START( tms5110 )
{
	static const tms5110_interface dummy = { 0 };
	struct tms5110_info *info;

	info = auto_malloc(sizeof(*info));
	memset(info, 0, sizeof(*info));
	info->intf = config ? config : &dummy;
	info->table = device->region;

	info->chip = tms5110_create(device, TMS5110_IS_5110A);
	if (!info->chip)
		return NULL;
	sndintrf_register_token(info);

	/* initialize a stream */
	info->stream = stream_create(device, 0, 1, clock / 80, info, tms5110_update);

	if (info->table == NULL)
	{
	    if (info->intf->M0_callback==NULL)
	    {
			logerror("\n file: 5110intf.c, SND_START( tms5110 ):\n  Missing _mandatory_ 'M0_callback' function pointer in the TMS5110 interface\n  This function is used by TMS5110 to call for a single bits\n  needed to generate the speech\n  Aborting startup...\n");
			return NULL;
	    }
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

    /* request a sound channel */
    return info;
}

static SND_START( tms5100 )
{
	struct tms5110_info *info = SND_START_CALL( tms5110 );
	tms5110_set_variant(info->chip, TMS5110_IS_5100);
	return info;
}

static SND_START( tms5110a )
{
	struct tms5110_info *info = SND_START_CALL( tms5110 );
	tms5110_set_variant(info->chip, TMS5110_IS_5110A);
	return info;
}

static SND_START( cd2801 )
{
	struct tms5110_info *info = SND_START_CALL( tms5110 );
	tms5110_set_variant(info->chip, TMS5110_IS_CD2801);
	return info;
}

static SND_START( tmc0281 )
{
	struct tms5110_info *info = SND_START_CALL( tms5110 );
	tms5110_set_variant(info->chip, TMS5110_IS_TMC0281);
	return info;
}

static SND_START( cd2802 )
{
	struct tms5110_info *info = SND_START_CALL( tms5110 );
	tms5110_set_variant(info->chip, TMS5110_IS_CD2802);
	return info;
}

static SND_START( m58817 )
{
	struct tms5110_info *info = SND_START_CALL( tms5110 );
	tms5110_set_variant(info->chip, TMS5110_IS_M58817);
	return info;
}


/******************************************************************************

     SND_STOP( tms5110 ) -- free buffers

******************************************************************************/

static SND_STOP( tms5110 )
{
	struct tms5110_info *info = device->token;
	tms5110_destroy(info->chip);
}


static SND_RESET( tms5110 )
{
	struct tms5110_info *info = device->token;
	tms5110_reset_chip(info->chip);
}



/******************************************************************************

     tms5110_ctl_w -- write Control Command to the sound chip
commands like Speech, Reset, etc., are loaded into the chip via the CTL pins

******************************************************************************/

WRITE8_HANDLER( tms5110_ctl_w )
{
	struct tms5110_info *info = sndti_token(SOUND_TMS5110, 0);

    /* bring up to date first */
    stream_update(info->stream);
    tms5110_CTL_set(info->chip, data);
}

/******************************************************************************

     tms5110_pdc_w -- write to PDC pin on the sound chip

******************************************************************************/

WRITE8_HANDLER( tms5110_pdc_w )
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

static STREAM_UPDATE( tms5110_update )
{
	struct tms5110_info *info = param;
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

void tms5110_set_frequency(int frequency)
{
	struct tms5110_info *info = sndti_token(SOUND_TMS5110, 0);
	stream_set_sample_rate(info->stream, frequency / 80);
}




/**************************************************************************
 * Generic get_info
 **************************************************************************/

static SND_SET_INFO( tms5110 )
{
	switch (state)
	{
		/* no parameters to set */
	}
}

SND_GET_INFO( tms5110 )
{
	switch (state)
	{
		/* --- the following bits of info are returned as 64-bit signed integers --- */
		case SNDINFO_INT_ALIAS:							info->i = SOUND_TMS5110;				break;

		/* --- the following bits of info are returned as pointers to data or functions --- */
		case SNDINFO_PTR_SET_INFO:						info->set_info = SND_SET_INFO_NAME( tms5110 );		break;
		case SNDINFO_PTR_START:							info->start = SND_START_NAME( tms5110 );			break;
		case SNDINFO_PTR_STOP:							info->stop = SND_STOP_NAME( tms5110 );				break;
		case SNDINFO_PTR_RESET:							info->reset = SND_RESET_NAME( tms5110 );			break;

		/* --- the following bits of info are returned as NULL-terminated strings --- */
		case SNDINFO_STR_NAME:							info->s = "TMS5110";					break;
		case SNDINFO_STR_CORE_FAMILY:					info->s = "TI Speech";					break;
		case SNDINFO_STR_CORE_VERSION:					info->s = "1.0";						break;
		case SNDINFO_STR_CORE_FILE:						info->s = __FILE__;						break;
		case SNDINFO_STR_CORE_CREDITS:					info->s = "Copyright Nicola Salmoria and the MAME Team"; break;
	}
}

SND_GET_INFO( tms5100 )
{
	switch (state)
	{
		case SNDINFO_PTR_START:							info->start = SND_START_NAME( tms5100 );			break;
		case SNDINFO_STR_NAME:							info->s = "TMS5100";					break;
		default: 										SND_GET_INFO_CALL(tms5110);	break;
	}
}

SND_GET_INFO( tms5110a )
{
	switch (state)
	{
		case SNDINFO_PTR_START:							info->start = SND_START_NAME( tms5110a );			break;
		case SNDINFO_STR_NAME:							info->s = "TMS5100A";					break;
		default: 										SND_GET_INFO_CALL(tms5110);	break;
	}
}

SND_GET_INFO( cd2801 )
{
	switch (state)
	{
		case SNDINFO_PTR_START:							info->start = SND_START_NAME( cd2801 );				break;
		case SNDINFO_STR_NAME:							info->s = "CD2801";						break;
		default: 										SND_GET_INFO_CALL(tms5110);	break;
	}
}

SND_GET_INFO( tmc0281 )
{
	switch (state)
	{
		case SNDINFO_PTR_START:							info->start = SND_START_NAME( tmc0281 );			break;
		case SNDINFO_STR_NAME:							info->s = "TMS5100";					break;
		default: 										SND_GET_INFO_CALL(tms5110);	break;
	}
}

SND_GET_INFO( cd2802 )
{
	switch (state)
	{
		case SNDINFO_PTR_START:							info->start = SND_START_NAME( cd2802 );				break;
		case SNDINFO_STR_NAME:							info->s = "CD2802";						break;
		default: 										SND_GET_INFO_CALL(tms5110);	break;
	}
}

SND_GET_INFO( m58817 )
{
	switch (state)
	{
		case SNDINFO_PTR_START:							info->start = SND_START_NAME( m58817 );				break;
		case SNDINFO_STR_NAME:							info->s = "M58817";						break;
		default: 										SND_GET_INFO_CALL(tms5110);	break;
	}
}



