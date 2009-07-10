/**********************************************************************************************
 *
 *   streaming ADPCM driver
 *   by Aaron Giles
 *
 *   Library to transcode from an ADPCM source to raw PCM.
 *   Written by Buffoni Mirko in 08/06/97
 *   References: various sources and documents.
 *
 *   HJB 08/31/98
 *   modified to use an automatically selected oversampling factor
 *   for the current sample rate
 *
 *   Mish 21/7/99
 *   Updated to allow multiple OKI chips with different sample rates
 *
 *   R. Belmont 31/10/2003
 *   Updated to allow a driver to use both MSM6295s and "raw" ADPCM voices (gcpinbal)
 *   Also added some error trapping for MAME_DEBUG builds
 *
 **********************************************************************************************/


#include <math.h>

#include "sndintrf.h"
#include "streams.h"
#include "okim6295.h"

#define MAX_SAMPLE_CHUNK	10000


/* struct describing a single playing ADPCM voice */
struct ADPCMVoice
{
	UINT8 playing;			/* 1 if we are actively playing */

	UINT32 base_offset;		/* pointer to the base memory location */
	UINT32 sample;			/* current sample number */
	UINT32 count;			/* total samples to play */

	struct adpcm_state adpcm;/* current ADPCM state */
	UINT32 volume;			/* output volume */
};

typedef struct _okim6295_state okim6295_state;
struct _okim6295_state
{
	#define OKIM6295_VOICES		4
	struct ADPCMVoice voice[OKIM6295_VOICES];
	const device_config *device;
	INT32 command;
	INT32 bank_num;
	INT32 bank_offs;
	sound_stream *stream;	/* which stream are we playing on? */
	UINT32 master_clock;	/* master clock frequency */
};

/* step size index shift table */
static const int index_shift[8] = { -1, -1, -1, -1, 2, 4, 6, 8 };

/* lookup table for the precomputed difference */
static int diff_lookup[49*16];

/* volume lookup table. The manual lists only 9 steps, ~3dB per step. Given the dB values,
   that seems to map to a 5-bit volume control. Any volume parameter beyond the 9th index
   results in silent playback. */
static const int volume_table[16] =
{
	0x20,	//   0 dB
	0x16,	//  -3.2 dB
	0x10,	//  -6.0 dB
	0x0b,	//  -9.2 dB
	0x08,	// -12.0 dB
	0x06,	// -14.5 dB
	0x04,	// -18.0 dB
	0x03,	// -20.5 dB
	0x02,	// -24.0 dB
	0x00,
	0x00,
	0x00,
	0x00,
	0x00,
	0x00,
	0x00,
};

/* tables computed? */
static int tables_computed = 0;

/* useful interfaces */
const okim6295_interface okim6295_interface_pin7high = { 1 };
const okim6295_interface okim6295_interface_pin7low = { 0 };

/* default address map */
static ADDRESS_MAP_START( okim6295, 0, 8 )
	AM_RANGE(0x00000, 0x3ffff) AM_ROM
ADDRESS_MAP_END


INLINE okim6295_state *get_safe_token(const device_config *device)
{
	assert(device != NULL);
	assert(device->token != NULL);
	assert(device->type == SOUND);
	assert(sound_get_type(device) == SOUND_OKIM6295);
	return (okim6295_state *)device->token;
}


/**********************************************************************************************

     compute_tables -- compute the difference tables

***********************************************************************************************/

static void compute_tables(void)
{
	/* nibble to bit map */
	static const int nbl2bit[16][4] =
	{
		{ 1, 0, 0, 0}, { 1, 0, 0, 1}, { 1, 0, 1, 0}, { 1, 0, 1, 1},
		{ 1, 1, 0, 0}, { 1, 1, 0, 1}, { 1, 1, 1, 0}, { 1, 1, 1, 1},
		{-1, 0, 0, 0}, {-1, 0, 0, 1}, {-1, 0, 1, 0}, {-1, 0, 1, 1},
		{-1, 1, 0, 0}, {-1, 1, 0, 1}, {-1, 1, 1, 0}, {-1, 1, 1, 1}
	};

	int step, nib;

	/* loop over all possible steps */
	for (step = 0; step <= 48; step++)
	{
		/* compute the step value */
		int stepval = floor(16.0 * pow(11.0 / 10.0, (double)step));

		/* loop over all nibbles and compute the difference */
		for (nib = 0; nib < 16; nib++)
		{
			diff_lookup[step*16 + nib] = nbl2bit[nib][0] *
				(stepval   * nbl2bit[nib][1] +
				 stepval/2 * nbl2bit[nib][2] +
				 stepval/4 * nbl2bit[nib][3] +
				 stepval/8);
		}
	}

	tables_computed = 1;
}



/**********************************************************************************************

     reset_adpcm -- reset the ADPCM stream

***********************************************************************************************/

void reset_adpcm(struct adpcm_state *state)
{
	/* make sure we have our tables */
	if (!tables_computed)
		compute_tables();

	/* reset the signal/step */
	state->signal = -2;
	state->step = 0;
}



/**********************************************************************************************

     clock_adpcm -- clock the next ADPCM byte

***********************************************************************************************/

INT16 clock_adpcm(struct adpcm_state *state, UINT8 nibble)
{
	state->signal += diff_lookup[state->step * 16 + (nibble & 15)];

	/* clamp to the maximum */
	if (state->signal > 2047)
		state->signal = 2047;
	else if (state->signal < -2048)
		state->signal = -2048;

	/* adjust the step size and clamp */
	state->step += index_shift[nibble & 7];
	if (state->step > 48)
		state->step = 48;
	else if (state->step < 0)
		state->step = 0;

	/* return the signal */
	return state->signal;
}



/**********************************************************************************************

     generate_adpcm -- general ADPCM decoding routine

***********************************************************************************************/

static void generate_adpcm(okim6295_state *chip, struct ADPCMVoice *voice, INT16 *buffer, int samples)
{
	/* if this voice is active */
	if (voice->playing)
	{
		offs_t base = voice->base_offset;
		int sample = voice->sample;
		int count = voice->count;

		/* loop while we still have samples to generate */
		while (samples)
		{
			/* compute the new amplitude and update the current step */
			int nibble = memory_raw_read_byte(chip->device->space[0], base + sample / 2) >> (((sample & 1) << 2) ^ 4);

			/* output to the buffer, scaling by the volume */
			/* signal in range -2048..2047, volume in range 2..32 => signal * volume / 2 in range -32768..32767 */
			*buffer++ = clock_adpcm(&voice->adpcm, nibble) * voice->volume / 2;
			samples--;

			/* next! */
			if (++sample >= count)
			{
				voice->playing = 0;
				break;
			}
		}

		/* update the parameters */
		voice->sample = sample;
	}

	/* fill the rest with silence */
	while (samples--)
		*buffer++ = 0;
}



/**********************************************************************************************
 *
 *  OKIM 6295 ADPCM chip:
 *
 *  Command bytes are sent:
 *
 *      1xxx xxxx = start of 2-byte command sequence, xxxxxxx is the sample number to trigger
 *      abcd vvvv = second half of command; one of the abcd bits is set to indicate which voice
 *                  the v bits seem to be volumed
 *
 *      0abc d000 = stop playing; one or more of the abcd bits is set to indicate which voice(s)
 *
 *  Status is read:
 *
 *      ???? abcd = one bit per voice, set to 0 if nothing is playing, or 1 if it is active
 *
***********************************************************************************************/


/**********************************************************************************************

     okim6295_update -- update the sound chip so that it is in sync with CPU execution

***********************************************************************************************/

static STREAM_UPDATE( okim6295_update )
{
	okim6295_state *chip = (okim6295_state *)param;
	int i;

	memset(outputs[0], 0, samples * sizeof(*outputs[0]));

	for (i = 0; i < OKIM6295_VOICES; i++)
	{
		struct ADPCMVoice *voice = &chip->voice[i];
		stream_sample_t *buffer = outputs[0];
		INT16 sample_data[MAX_SAMPLE_CHUNK];
		int remaining = samples;

		/* loop while we have samples remaining */
		while (remaining)
		{
			int samples = (remaining > MAX_SAMPLE_CHUNK) ? MAX_SAMPLE_CHUNK : remaining;
			int samp;

			generate_adpcm(chip, voice, sample_data, samples);
			for (samp = 0; samp < samples; samp++)
				*buffer++ += sample_data[samp];

			remaining -= samples;
		}
	}
}



/**********************************************************************************************

     state save support for MAME

***********************************************************************************************/

static void adpcm_state_save_register(struct ADPCMVoice *voice, const device_config *device, int index)
{
	state_save_register_device_item(device, index, voice->playing);
	state_save_register_device_item(device, index, voice->sample);
	state_save_register_device_item(device, index, voice->count);
	state_save_register_device_item(device, index, voice->adpcm.signal);
	state_save_register_device_item(device, index, voice->adpcm.step);
	state_save_register_device_item(device, index, voice->volume);
	state_save_register_device_item(device, index, voice->base_offset);
}

static STATE_POSTLOAD( okim6295_postload )
{
	const device_config *device = (const device_config *)param;
	okim6295_state *info = get_safe_token(device);
	okim6295_set_bank_base(device, info->bank_offs);
}

static void okim6295_state_save_register(okim6295_state *info, const device_config *device)
{
	int j;

	state_save_register_device_item(device, 0, info->command);
	state_save_register_device_item(device, 0, info->bank_offs);
	for (j = 0; j < OKIM6295_VOICES; j++)
		adpcm_state_save_register(&info->voice[j], device, j);

	state_save_register_postload(device->machine, okim6295_postload, (void *)device);
}



/**********************************************************************************************

     DEVICE_START( okim6295 ) -- start emulation of an OKIM6295-compatible chip

***********************************************************************************************/

static DEVICE_START( okim6295 )
{
	const okim6295_interface *intf = (const okim6295_interface *)device->static_config;
	okim6295_state *info = get_safe_token(device);
	int divisor = intf->pin7 ? 132 : 165;
	int voice;

	compute_tables();

	info->command = -1;
	info->bank_num = -1;
	info->bank_offs = 0;
	info->device = device;

	info->master_clock = device->clock;

	/* generate the name and create the stream */
	info->stream = stream_create(device, 0, 1, device->clock/divisor, info, okim6295_update);

	/* initialize the voices */
	for (voice = 0; voice < OKIM6295_VOICES; voice++)
	{
		/* initialize the rest of the structure */
		info->voice[voice].volume = 0;
		reset_adpcm(&info->voice[voice].adpcm);
	}

	okim6295_state_save_register(info, device);
}



/**********************************************************************************************

     DEVICE_RESET( okim6295 ) -- stop emulation of an OKIM6295-compatible chip

***********************************************************************************************/

static DEVICE_RESET( okim6295 )
{
	okim6295_state *info = get_safe_token(device);
	int i;

	stream_update(info->stream);
	for (i = 0; i < OKIM6295_VOICES; i++)
		info->voice[i].playing = 0;
}



/**********************************************************************************************

     okim6295_set_bank_base -- set the base of the bank for a given voice on a given chip

***********************************************************************************************/

void okim6295_set_bank_base(const device_config *device, int base)
{
	okim6295_state *info = get_safe_token(device);
	stream_update(info->stream);

	/* if we are setting a non-zero base, and we have no bank, allocate one */
	if (info->bank_num == -1 && base != 0)
	{
		info->bank_num = memory_find_unused_bank(device->machine);
		if (info->bank_num == -1)
			fatalerror("Unable to allocate bank for oki6295 device '%s'", device->tag);

		/* override our memory map with a bank */
		memory_install_read8_handler(device->space[0], 0x00000, 0x3ffff, 0, 0, SMH_BANK(info->bank_num));
	}

	/* if we have a bank number, set the base pointer */
	if (info->bank_num != -1)
	{
		info->bank_offs = base;
		memory_set_bankptr(device->machine, info->bank_num, device->region + base);
	}
}



/**********************************************************************************************

     okim6295_set_pin7 -- adjust pin 7, which controls the internal clock division

***********************************************************************************************/

void okim6295_set_pin7(const device_config *device, int pin7)
{
	okim6295_state *info = get_safe_token(device);
	int divisor = pin7 ? 132 : 165;

	stream_set_sample_rate(info->stream, info->master_clock/divisor);
}


/**********************************************************************************************

     okim6295_status_r -- read the status port of an OKIM6295-compatible chip

***********************************************************************************************/

READ8_DEVICE_HANDLER( okim6295_r )
{
	okim6295_state *info = get_safe_token(device);
	int i, result;

	result = 0xf0;	/* naname expects bits 4-7 to be 1 */

	/* set the bit to 1 if something is playing on a given channel */
	stream_update(info->stream);
	for (i = 0; i < OKIM6295_VOICES; i++)
	{
		struct ADPCMVoice *voice = &info->voice[i];

		/* set the bit if it's playing */
		if (voice->playing)
			result |= 1 << i;
	}

	return result;
}



/**********************************************************************************************

     okim6295_data_w -- write to the data port of an OKIM6295-compatible chip

***********************************************************************************************/

WRITE8_DEVICE_HANDLER( okim6295_w )
{
	okim6295_state *info = get_safe_token(device);

	/* if a command is pending, process the second half */
	if (info->command != -1)
	{
		int temp = data >> 4, i, start, stop;
		offs_t base;

		/* the manual explicitly says that it's not possible to start multiple voices at the same time */
		if (temp != 0 && temp != 1 && temp != 2 && temp != 4 && temp != 8)
			popmessage("OKI6295 start %x contact MAMEDEV", temp);

		/* update the stream */
		stream_update(info->stream);

		/* determine which voice(s) (voice is set by a 1 bit in the upper 4 bits of the second byte) */
		for (i = 0; i < OKIM6295_VOICES; i++, temp >>= 1)
		{
			if (temp & 1)
			{
				struct ADPCMVoice *voice = &info->voice[i];

				/* determine the start/stop positions */
				base = info->command * 8;

				start  = memory_raw_read_byte(device->space[0], base + 0) << 16;
				start |= memory_raw_read_byte(device->space[0], base + 1) << 8;
				start |= memory_raw_read_byte(device->space[0], base + 2) << 0;
				start &= 0x3ffff;

				stop  = memory_raw_read_byte(device->space[0], base + 3) << 16;
				stop |= memory_raw_read_byte(device->space[0], base + 4) << 8;
				stop |= memory_raw_read_byte(device->space[0], base + 5) << 0;
				stop &= 0x3ffff;

				/* set up the voice to play this sample */
				if (start < stop)
				{
					if (!voice->playing) /* fixes Got-cha and Steel Force */
					{
						voice->playing = 1;
						voice->base_offset = start;
						voice->sample = 0;
						voice->count = 2 * (stop - start + 1);

						/* also reset the ADPCM parameters */
						reset_adpcm(&voice->adpcm);
						voice->volume = volume_table[data & 0x0f];
					}
					else
					{
						logerror("OKIM6295:'%s' requested to play sample %02x on non-stopped voice\n",device->tag,info->command);
					}
				}
				/* invalid samples go here */
				else
				{
					logerror("OKIM6295:'%s' requested to play invalid sample %02x\n",device->tag,info->command);
					voice->playing = 0;
				}
			}
		}

		/* reset the command */
		info->command = -1;
	}

	/* if this is the start of a command, remember the sample number for next time */
	else if (data & 0x80)
	{
		info->command = data & 0x7f;
	}

	/* otherwise, see if this is a silence command */
	else
	{
		int temp = data >> 3, i;

		/* update the stream, then turn it off */
		stream_update(info->stream);

		/* determine which voice(s) (voice is set by a 1 bit in bits 3-6 of the command */
		for (i = 0; i < OKIM6295_VOICES; i++, temp >>= 1)
		{
			if (temp & 1)
			{
				struct ADPCMVoice *voice = &info->voice[i];

				voice->playing = 0;
			}
		}
	}
}



/**************************************************************************
 * Generic get_info
 **************************************************************************/

DEVICE_GET_INFO( okim6295 )
{
	switch (state)
	{
		/* --- the following bits of info are returned as 64-bit signed integers --- */
		case DEVINFO_INT_TOKEN_BYTES:				info->i = sizeof(okim6295_state);				break;
		case DEVINFO_INT_DATABUS_WIDTH_0:			info->i = 8;									break;
		case DEVINFO_INT_ADDRBUS_WIDTH_0:			info->i = 18;									break;
		case DEVINFO_INT_ADDRBUS_SHIFT_0:			info->i = 0;									break;

		/* --- the following bits of info are returned as pointers to data --- */
		case DEVINFO_PTR_DEFAULT_MEMORY_MAP_0:		info->default_map8 = ADDRESS_MAP_NAME(okim6295);break;

		/* --- the following bits of info are returned as pointers to functions --- */
		case DEVINFO_FCT_START:						info->start = DEVICE_START_NAME( okim6295 );	break;
		case DEVINFO_FCT_RESET:						info->reset = DEVICE_RESET_NAME( okim6295 );	break;

		/* --- the following bits of info are returned as NULL-terminated strings --- */
		case DEVINFO_STR_NAME:						strcpy(info->s, "OKI6295");						break;
		case DEVINFO_STR_FAMILY:					strcpy(info->s, "OKI ADPCM");					break;
		case DEVINFO_STR_VERSION:					strcpy(info->s, "1.0");							break;
		case DEVINFO_STR_SOURCE_FILE:				strcpy(info->s, __FILE__);						break;
		case DEVINFO_STR_CREDITS:					strcpy(info->s, "Copyright Nicola Salmoria and the MAME Team"); break;
	}
}
