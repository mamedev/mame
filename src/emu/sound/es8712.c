/**********************************************************************************************
 *
 *  Streaming single channel ADPCM core for the ES8712 chip
 *  Chip is branded by Excellent Systems, probably OEM'd.
 *
 *  Samples are currently looped, but whether they should and how, is unknown.
 *  Interface to the chip is also not 100% clear.
 *  Should there be any status signals signifying busy, end of sample - etc?
 *
 *  Heavily borrowed from the OKI M6295 source
 *
 **********************************************************************************************/


#include "emu.h"
#include "es8712.h"

#define MAX_SAMPLE_CHUNK	10000


/* struct describing a playing ADPCM chip */
struct es8712_state
{
	UINT8 playing;			/* 1 if we're actively playing */

	UINT32 base_offset;		/* pointer to the base memory location */
	UINT32 sample;			/* current sample number */
	UINT32 count;			/* total samples to play */

	UINT32 signal;			/* current ADPCM signal */
	UINT32 step;			/* current ADPCM step */

	UINT32 start;			/* starting address for the next loop */
	UINT32 end;				/* ending address for the next loop */
	UINT8  repeat;			/* Repeat current sample when 1 */

	INT32 bank_offset;
	UINT8 *region_base;		/* pointer to the base of the region */
	sound_stream *stream;	/* which stream are we playing on? */
};

/* step size index shift table */
static const int index_shift[8] = { -1, -1, -1, -1, 2, 4, 6, 8 };

/* lookup table for the precomputed difference */
static int diff_lookup[49*16];


INLINE es8712_state *get_safe_token(device_t *device)
{
	assert(device != NULL);
	assert(device->type() == ES8712);
	return (es8712_state *)downcast<es8712_device *>(device)->token();
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
}



/**********************************************************************************************

    generate_adpcm -- general ADPCM decoding routine

***********************************************************************************************/

static void generate_adpcm(es8712_state *chip, stream_sample_t *buffer, int samples)
{
	/* if this chip is active */
	if (chip->playing)
	{
		UINT8 *base = chip->region_base + chip->bank_offset + chip->base_offset;
		int sample = chip->sample;
		int signal = chip->signal;
		int count = chip->count;
		int step = chip->step;
		int val;

		/* loop while we still have samples to generate */
		while (samples)
		{
			/* compute the new amplitude and update the current step */
			val = base[sample / 2] >> (((sample & 1) << 2) ^ 4);
			signal += diff_lookup[step * 16 + (val & 15)];

			/* clamp to the maximum */
			if (signal > 2047)
				signal = 2047;
			else if (signal < -2048)
				signal = -2048;

			/* adjust the step size and clamp */
			step += index_shift[val & 7];
			if (step > 48)
				step = 48;
			else if (step < 0)
				step = 0;

			/* output to the buffer */
			*buffer++ = signal * 16;
			samples--;

			/* next! */
			if (++sample >= count)
			{
				if (chip->repeat)
				{
					sample = 0;
					signal = -2;
					step = 0;
				}
				else
				{
					chip->playing = 0;
					break;
				}
			}
		}

		/* update the parameters */
		chip->sample = sample;
		chip->signal = signal;
		chip->step = step;
	}

	/* fill the rest with silence */
	while (samples--)
		*buffer++ = 0;
}


/**********************************************************************************************

    es8712_update -- update the sound chip so that it is in sync with CPU execution

***********************************************************************************************/

static STREAM_UPDATE( es8712_update )
{
	stream_sample_t *buffer = outputs[0];
	es8712_state *chip = (es8712_state *)param;

	/* generate them into our buffer */
	generate_adpcm(chip, buffer, samples);
}



/**********************************************************************************************

     state save support for MAME

***********************************************************************************************/

static void es8712_state_save_register(es8712_state *chip, device_t *device)
{
	device->save_item(NAME(chip->bank_offset));

	device->save_item(NAME(chip->playing));
	device->save_item(NAME(chip->sample));
	device->save_item(NAME(chip->count));
	device->save_item(NAME(chip->signal));
	device->save_item(NAME(chip->step));

	device->save_item(NAME(chip->base_offset));

	device->save_item(NAME(chip->start));
	device->save_item(NAME(chip->end));
	device->save_item(NAME(chip->repeat));
}



/**********************************************************************************************

    DEVICE_START( es8712 ) -- start emulation of an ES8712 chip

***********************************************************************************************/

static DEVICE_START( es8712 )
{
	es8712_state *chip = get_safe_token(device);

	compute_tables();

	chip->start = 0;
	chip->end = 0;
	chip->repeat = 0;

	chip->bank_offset = 0;
	chip->region_base = *device->region();

	/* generate the name and create the stream */
	chip->stream = device->machine().sound().stream_alloc(*device, 0, 1, device->clock(), chip, es8712_update);

	/* initialize the rest of the structure */
	chip->signal = -2;

	es8712_state_save_register(chip, device);
}



/*************************************************************************************

     DEVICE_RESET( es8712 ) -- stop emulation of an ES8712-compatible chip

**************************************************************************************/

static DEVICE_RESET( es8712 )
{
	es8712_state *chip = get_safe_token(device);

	if (chip->playing)
	{
		/* update the stream, then turn it off */
		chip->stream->update();
		chip->playing = 0;
		chip->repeat = 0;
	}
}


/****************************************************************************

    es8712_set_bank_base -- set the base of the bank on a given chip

*****************************************************************************/

void es8712_set_bank_base(device_t *device, int base)
{
	es8712_state *chip = get_safe_token(device);
	chip->stream->update();
	chip->bank_offset = base;
}


/****************************************************************************

    es8712_set_frequency -- dynamically adjusts the frequency of a given ADPCM chip

*****************************************************************************/

void es8712_set_frequency(device_t *device, int frequency)
{
	es8712_state *chip = get_safe_token(device);

	/* update the stream and set the new base */
	chip->stream->update();
	chip->stream->set_sample_rate(frequency);
}



/**********************************************************************************************

    es8712_play -- Begin playing the addressed sample

***********************************************************************************************/

void es8712_play(device_t *device)
{
	es8712_state *chip = get_safe_token(device);


	if (chip->start < chip->end)
	{
		if (!chip->playing)
		{
			chip->playing = 1;
			chip->base_offset = chip->start;
			chip->sample = 0;
			chip->count = 2 * (chip->end - chip->start + 1);
			chip->repeat = 0;//1;

			/* also reset the ADPCM parameters */
			chip->signal = -2;
			chip->step = 0;
		}
	}
	/* invalid samples go here */
	else
	{
		logerror("ES871295:'%s' requested to play invalid sample range %06x-%06x\n",device->tag(),chip->start,chip->end);

		if (chip->playing)
		{
			/* update the stream */
			chip->stream->update();
			chip->playing = 0;
		}
	}
}



/**********************************************************************************************

     es8712_data_0_w -- generic data write functions
     es8712_data_1_w

***********************************************************************************************/

/**********************************************************************************************
 *
 *  offset  Start       End
 *          0hmmll  -  0HMMLL
 *    00    ----ll
 *    01    --mm--
 *    02    0h----
 *    03               ----LL
 *    04               --MM--
 *    05               0H----
 *    06           Go!
 *
 * Offsets are written in the order -> 00, 02, 01, 03, 05, 04, 06
 * Offset 06 is written with the same value as offset 04.
 *
***********************************************************************************************/

WRITE8_DEVICE_HANDLER( es8712_w )
{
	es8712_state *chip = get_safe_token(device);
	switch (offset)
	{
		case 00:	chip->start &= 0x000fff00;
					chip->start |= ((data & 0xff) <<  0); break;
		case 01:	chip->start &= 0x000f00ff;
					chip->start |= ((data & 0xff) <<  8); break;
		case 02:	chip->start &= 0x0000ffff;
					chip->start |= ((data & 0x0f) << 16); break;
		case 03:	chip->end   &= 0x000fff00;
					chip->end   |= ((data & 0xff) <<  0); break;
		case 04:	chip->end   &= 0x000f00ff;
					chip->end   |= ((data & 0xff) <<  8); break;
		case 05:	chip->end   &= 0x0000ffff;
					chip->end   |= ((data & 0x0f) << 16); break;
		case 06:
				es8712_play(device);
				break;
		default:	break;
	}
	chip->start &= 0xfffff; chip->end &= 0xfffff;
}

const device_type ES8712 = &device_creator<es8712_device>;

es8712_device::es8712_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
	: device_t(mconfig, ES8712, "ES8712", tag, owner, clock),
	  device_sound_interface(mconfig, *this)
{
	m_token = global_alloc_array_clear(UINT8, sizeof(es8712_state));
}

//-------------------------------------------------
//  device_config_complete - perform any
//  operations now that the configuration is
//  complete
//-------------------------------------------------

void es8712_device::device_config_complete()
{
}

//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void es8712_device::device_start()
{
	DEVICE_START_NAME( es8712 )(this);
}

//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void es8712_device::device_reset()
{
	DEVICE_RESET_NAME( es8712 )(this);
}

//-------------------------------------------------
//  sound_stream_update - handle a stream update
//-------------------------------------------------

void es8712_device::sound_stream_update(sound_stream &stream, stream_sample_t **inputs, stream_sample_t **outputs, int samples)
{
	// should never get here
	fatalerror("sound_stream_update called; not applicable to legacy sound devices\n");
}


