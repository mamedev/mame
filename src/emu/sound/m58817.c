/******************************************************************************

     M58817 interface

     Written for MAME by couriersud

     - structure from TMS5110 interface
     - this is a wrapper around the TMS5110 interface
     - M58817 & TMS5110 seem to be similar, however it is very probable
       that they use different "coding" tables for energy, pitch
     - Speech is understandable, but off
     - This driver supports to use a "sample" interface instead as well

******************************************************************************/

#include <math.h>

#include "sndintrf.h"
#include "streams.h"
#include "tms5110.h"
#include "m58817.h"
#include "sound/samples.h"


#define MAX_SAMPLE_CHUNK	10000

enum {
	WAIT_CMD,
	WAIT_WRITE,
	WAIT_DONE1,
	WAIT_DONE2
} m58817_states;

/* the state of the streamed output */
struct m58817_info
{
	const struct M58817interface *intf;
	sound_stream *stream;
	void *chip;
	UINT8 state;
	UINT8 drq;
	UINT8 nibbles[4];
	UINT8 command_latch;
	INT32 count;
	INT32 address;
	INT32 speech_rom_bitnum;
};


/* static function prototypes */
static void m58817_update(void *param, stream_sample_t **inputs, stream_sample_t **buffer, int length);

static int speech_rom_read_bit(void)
{
	struct m58817_info *info = sndti_token(SOUND_M58817, 0);
	const UINT8 *table = memory_region(info->intf->rom_region);

	int r;

	if (info->speech_rom_bitnum<0)
		r = 0;
	else
		r = (table[info->speech_rom_bitnum >> 3] >> (0x07 - (info->speech_rom_bitnum & 0x07))) & 1;
	//r = (table[speech_rom_bitnum >> 3] >> ((speech_rom_bitnum & 0x07))) & 1;
	info->speech_rom_bitnum++;
	//printf("Byte: 0x%02x\n", speech_rom_bitnum>>3);
	return r;
}

/******************************************************************************

     m58817_state_loop -- process commands

******************************************************************************/

static void m58817_state_loop(void *chip, int data)
{
	struct m58817_info *info = chip;
	int i;
	switch (info->state)
	{
		case WAIT_CMD:
			switch (data)
			{
				case 0x00: // reset ????
					info->count=0;
					/*To be extremely accurate there should be a delays between each of
                      the function calls below. In real they happen with the frequency of 160 kHz.
                    */

					if (info->intf->rom_region != -1)
					{
						tms5110_CTL_set(info->chip, TMS5110_CMD_RESET);
						tms5110_PDC_set(info->chip, 0);
						tms5110_PDC_set(info->chip, 1);
						tms5110_PDC_set(info->chip, 0);

						tms5110_PDC_set(info->chip, 0);
						tms5110_PDC_set(info->chip, 1);
						tms5110_PDC_set(info->chip, 0);

						tms5110_PDC_set(info->chip, 0);
						tms5110_PDC_set(info->chip, 1);
						tms5110_PDC_set(info->chip, 0);

						info->speech_rom_bitnum = 0x0;
					}
					break;
				case 0x02: // latch next nibbel
					info->state=WAIT_WRITE;
					break;
				case 0x08: // play ????
					info->state=WAIT_DONE1;
					break;
				default:
					logerror("m58817: unknown cmd : 0x%02x\n", data);
			}
			break;
		case WAIT_WRITE:
			info->nibbles[info->count++] = data & 0x0f;
			info->state=WAIT_CMD;
			break;
		case WAIT_DONE1:
			if (data != 0x0A)
				logerror("m58817: expected 0x0A got 0x%02x\n", data);
			info->address = 0;
			for (i=0;i<info->count;i++)
			{
				info->address |= (info->nibbles[i] << (i*4));
			}
			logerror("m58817: address: 0x%04x\n", info->address);


			if (info->intf->rom_region != -1)
			{
				info->speech_rom_bitnum = info->address * 8 - 1;
				tms5110_CTL_set(info->chip, TMS5110_CMD_SPEAK);
				tms5110_PDC_set(info->chip, 0);
				tms5110_PDC_set(info->chip, 1);
				tms5110_PDC_set(info->chip, 0);
			}
			else
			{
				for (i=0;i<M58817_MAX_SAMPLES;i++)
					if (info->intf->sample_addr[i] == info->address)
					{
						sample_start(0,i,0);
						break;
					}
			}

			info->state=WAIT_CMD;
			break;
	}
}


/******************************************************************************

     m58817_start -- allocate buffers and reset the 5110

******************************************************************************/

static void *m58817_start(int sndindex, int clock, const void *config)
{
	static const struct M58817interface dummy = { 0 };
	struct m58817_info *info;

	info = auto_malloc(sizeof(*info));
	memset(info, 0, sizeof(*info));
	info->intf = config ? config : &dummy;

	if (info->intf->rom_region != -1)
	{
		info->chip = tms5110_create(sndindex);
		if (!info->chip)
			return NULL;
	}
	sndintrf_register_token(info);

	/* initialize a stream */
	if (info->intf->rom_region != -1)
	{
		info->stream = stream_create(0, 1, clock / 80, info, m58817_update);
		tms5110_set_M0_callback(info->chip, speech_rom_read_bit );
	    /* reset the 58817 */
	    tms5110_reset_chip(info->chip);
	}

	state_save_register_item("m58817", sndindex, info->state);
	state_save_register_item("m58817", sndindex, info->drq);
	state_save_register_item_array("m58817", sndindex, info->nibbles);
	state_save_register_item("m58817", sndindex, info->count);
	state_save_register_item("m58817", sndindex, info->address);
	state_save_register_item("m58817", sndindex, info->speech_rom_bitnum);

    /* request a sound channel */
    return info;
}



/******************************************************************************

     m58817_stop -- free buffers

******************************************************************************/

static void m58817_stop(void *chip)
{
	struct m58817_info *info = chip;
	if (info->intf->rom_region != -1)
		tms5110_destroy(info->chip);
}


static void m58817_reset(void *chip)
{
	struct m58817_info *info = chip;
	if (info->intf->rom_region != -1)
		tms5110_reset_chip(info->chip);
	info->state = WAIT_CMD;
	info->drq = 0;
	info->command_latch = 0;
	info->count=0;
}



/******************************************************************************

     m58817_CTL_w -- write Control Command to the sound chip
     commands like Speech, Reset, etc., are loaded into the chip via the CTL pins

******************************************************************************/

WRITE8_HANDLER( m58817_CTL_w )
{
	struct m58817_info *info = sndti_token(SOUND_M58817, 0);

    /* bring up to date first */
    //stream_update(info->stream);
    info->command_latch = data & 0x0f;
}

/******************************************************************************

     m58817_DRQ_w -- write to DRQ pin on the sound chip

******************************************************************************/

WRITE8_HANDLER( m58817_DRQ_w )
{
	struct m58817_info *info = sndti_token(SOUND_M58817, 0);

    /* bring up to date first */
	if (info->intf->rom_region != -1)
	    stream_update(info->stream);
	if (!data & info->drq)
		m58817_state_loop(info, info->command_latch);
	info->drq = data;
}



/******************************************************************************

     m58817_status_r -- read status from the sound chip

******************************************************************************/

READ8_HANDLER( m58817_status_r )
{
	struct m58817_info *info = sndti_token(SOUND_M58817, 0);

    /* bring up to date first */
	if (info->intf->rom_region != -1)
	    stream_update(info->stream);
	if (info->intf->rom_region != -1)
	    return tms5110_status_read(info->chip);
	else
		return sample_playing(0);
}



/******************************************************************************

     m58817_update -- update the sound chip so that it is in sync with CPU execution

******************************************************************************/

static void m58817_update(void *param, stream_sample_t **inputs, stream_sample_t **_buffer, int length)
{
	struct m58817_info *info = param;
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

     m58817_set_frequency -- adjusts the playback frequency

******************************************************************************/

void m58817_set_frequency(int frequency)
{
	struct m58817_info *info = sndti_token(SOUND_M58817, 0);
	stream_set_sample_rate(info->stream, frequency / 80);
}




/**************************************************************************
 * Generic get_info
 **************************************************************************/

static void m58817_set_info(void *token, UINT32 state, sndinfo *info)
{
	switch (state)
	{
		/* no parameters to set */
	}
}


void m58817_get_info(void *token, UINT32 state, sndinfo *info)
{
	switch (state)
	{
		/* --- the following bits of info are returned as 64-bit signed integers --- */

		/* --- the following bits of info are returned as pointers to data or functions --- */
		case SNDINFO_PTR_SET_INFO:						info->set_info = m58817_set_info;	break;
		case SNDINFO_PTR_START:							info->start = m58817_start;			break;
		case SNDINFO_PTR_STOP:							info->stop = m58817_stop;			break;
		case SNDINFO_PTR_RESET:							info->reset = m58817_reset;			break;

		/* --- the following bits of info are returned as NULL-terminated strings --- */
		case SNDINFO_STR_NAME:							info->s = "M58817";					break;
		case SNDINFO_STR_CORE_FAMILY:					info->s = "Mitsubishi Speech";		break;
		case SNDINFO_STR_CORE_VERSION:					info->s = "1.0";					break;
		case SNDINFO_STR_CORE_FILE:						info->s = __FILE__;					break;
		case SNDINFO_STR_CORE_CREDITS:					info->s = "Copyright (c) 2007, The MAME Team"; break;
	}
}

