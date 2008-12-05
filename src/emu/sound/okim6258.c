/**********************************************************************************************
 *
 *   OKI MSM6258 ADPCM
 *
 *   TODO:
 *   3-bit ADPCM support
 *   Recording?
 *
 **********************************************************************************************/


#include <math.h>

#include "sndintrf.h"
#include "streams.h"
#include "okim6258.h"

#define COMMAND_STOP		(1 << 0)
#define COMMAND_PLAY		(1 << 1)
#define	COMMAND_RECORD		(1 << 2)

#define STATUS_PLAYING		(1 << 1)
#define STATUS_RECORDING	(1 << 2)

static const int dividers[4] = { 1024, 768, 512, 512 };

struct okim6258
{
	UINT8  status;

	UINT32 master_clock;	/* master clock frequency */
	UINT32 divider;			/* master clock divider */
	UINT8 adpcm_type;		/* 3/4 bit ADPCM select */
	UINT8 data_in;			/* ADPCM data-in register */
	UINT8 nibble_shift;		/* nibble select */
	sound_stream *stream;	/* which stream are we playing on? */

	UINT8 output_bits;

	INT32 signal;
	INT32 step;
};

/* step size index shift table */
static const int index_shift[8] = { -1, -1, -1, -1, 2, 4, 6, 8 };

/* lookup table for the precomputed difference */
static int diff_lookup[49*16];

/* tables computed? */
static int tables_computed = 0;

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


static INT16 clock_adpcm(struct okim6258 *chip, UINT8 nibble)
{
	INT32 max = (1 << (chip->output_bits - 1)) - 1;
	INT32 min = -(1 << (chip->output_bits - 1));

	chip->signal += diff_lookup[chip->step * 16 + (nibble & 15)];

	/* clamp to the maximum */
	if (chip->signal > max)
		chip->signal = max;
	else if (chip->signal < min)
		chip->signal = min;

	/* adjust the step size and clamp */
	chip->step += index_shift[nibble & 7];
	if (chip->step > 48)
		chip->step = 48;
	else if (chip->step < 0)
		chip->step = 0;

	/* return the signal scaled up to 32767 */
	return chip->signal << 4;
}

/**********************************************************************************************

     okim6258_update -- update the sound chip so that it is in sync with CPU execution

***********************************************************************************************/

static void okim6258_update(void *param, stream_sample_t **inputs, stream_sample_t **outputs, int samples)
{
	struct okim6258 *chip = param;
	stream_sample_t *buffer = outputs[0];

	memset(outputs[0], 0, samples * sizeof(*outputs[0]));

	if (chip->status & STATUS_PLAYING)
	{
		int nibble_shift = chip->nibble_shift;

		while (samples)
		{
			/* Compute the new amplitude and update the current step */
			int nibble = (chip->data_in >> nibble_shift) & 0xf;

			/* Output to the buffer */
			INT16 sample = clock_adpcm(chip, nibble);

			nibble_shift ^= 4;

			*buffer++ = sample;
			samples--;
		}

		/* Update the parameters */
		chip->nibble_shift = nibble_shift;
	}
	else
	{
		/* Fill with 0 */
		while (samples--)
			*buffer++ = 0;
	}
}



/**********************************************************************************************

     state save support for MAME

***********************************************************************************************/

static void okim6258_state_save_register(struct okim6258 *info, const device_config *device)
{
	state_save_register_device_item(device, 0, info->status);
	state_save_register_device_item(device, 0, info->master_clock);
	state_save_register_device_item(device, 0, info->divider);
	state_save_register_device_item(device, 0, info->data_in);
	state_save_register_device_item(device, 0, info->nibble_shift);
	state_save_register_device_item(device, 0, info->signal);
	state_save_register_device_item(device, 0, info->step);
}


/**********************************************************************************************

     OKIM6258_start -- start emulation of an OKIM6258-compatible chip

***********************************************************************************************/

static SND_START( okim6258 )
{
	const okim6258_interface *intf = config;
	struct okim6258 *info;

	info = auto_malloc(sizeof(*info));
	memset(info, 0, sizeof(*info));

	compute_tables();

	info->master_clock = clock;
	info->adpcm_type = intf->adpcm_type;

	/* D/A precision is 10-bits but 12-bit data can be output serially to an external DAC */
	info->output_bits = intf->output_12bits ? 12 : 10;
	info->divider = dividers[intf->divider];

	info->stream = stream_create(0, 1, clock/info->divider, info, okim6258_update);

	info->signal = -2;
	info->step = 0;

	okim6258_state_save_register(info, device);

	return info;
}


/**********************************************************************************************

     OKIM6258_stop -- stop emulation of an OKIM6258-compatible chip

***********************************************************************************************/

static SND_RESET( okim6258 )
{
	struct okim6258 *info = device->token;

	stream_update(info->stream);

	info->signal = -2;
	info->step = 0;
	info->status = 0;
}


/**********************************************************************************************

     okim6258_set_divider -- set the master clock divider

***********************************************************************************************/

void okim6258_set_divider(int which, int val)
{
	struct okim6258 *info = sndti_token(SOUND_OKIM6258, which);
	int divider = dividers[val];

	info->divider = dividers[val];
	stream_set_sample_rate(info->stream, info->master_clock / divider);
}


/**********************************************************************************************

     okim6258_set_clock -- set the master clock

***********************************************************************************************/

void okim6258_set_clock(int which, int val)
{
	struct okim6258 *info = sndti_token(SOUND_OKIM6258, which);

	info->master_clock = val;
	stream_set_sample_rate(info->stream, info->master_clock / info->divider);
}


/**********************************************************************************************

     okim6258_get_vclk -- get the VCLK/sampling frequency

***********************************************************************************************/

int okim6258_get_vclk(int which)
{
	struct okim6258 *info = sndti_token(SOUND_OKIM6258, which);

	return (info->master_clock / info->divider);
}


/**********************************************************************************************

     okim6258_status_r -- read the status port of an OKIM6258-compatible chip

***********************************************************************************************/

static int okim6258_status_r(int num)
{
	struct okim6258 *info = sndti_token(SOUND_OKIM6258, num);

	stream_update(info->stream);

	return (info->status & STATUS_PLAYING) ? 0x00 : 0x80;
}


/**********************************************************************************************

     okim6258_data_w -- write to the control port of an OKIM6258-compatible chip

***********************************************************************************************/
static void okim6258_data_w(int num, int data)
{
	struct okim6258 *info = sndti_token(SOUND_OKIM6258, num);

	/* update the stream */
	stream_update(info->stream);

	info->data_in = data;
	info->nibble_shift = 0;
}


/**********************************************************************************************

     okim6258_ctrl_w -- write to the control port of an OKIM6258-compatible chip

***********************************************************************************************/

static void okim6258_ctrl_w(int num, int data)
{
	struct okim6258 *info = sndti_token(SOUND_OKIM6258, num);

	stream_update(info->stream);

	if (data & COMMAND_STOP)
	{
		info->status &= ~(STATUS_PLAYING | STATUS_RECORDING);
		return;
	}

	if (data & COMMAND_PLAY)
	{
		if (!(info->status & STATUS_PLAYING))
		{
			info->status |= STATUS_PLAYING;

			/* Also reset the ADPCM parameters */
			info->signal = -2;
			info->step = 0;
			info->nibble_shift = 0;
		}
	}
	else
	{
		info->status &= ~STATUS_PLAYING;
	}

	if (data & COMMAND_RECORD)
	{
		logerror("M6258: Record enabled\n");
		info->status |= STATUS_RECORDING;
	}
	else
	{
		info->status &= ~STATUS_RECORDING;
	}
}


/**********************************************************************************************

     okim6258_status_0_r -- generic status read functions

***********************************************************************************************/

READ8_HANDLER( okim6258_status_0_r )
{
	return okim6258_status_r(0);
}

READ16_HANDLER( okim6258_status_0_lsb_r )
{
	return okim6258_status_r(0);
}

READ16_HANDLER( okim6258_status_0_msb_r )
{
	return okim6258_status_r(0) << 8;
}

READ8_HANDLER( okim6258_data_0_r )
{
	// TODO
	return 0;
}

READ16_HANDLER( okim6258_data_0_lsb_r )
{
	// TODO
	return 0;
}

READ16_HANDLER( okim6258_data_0_msb_r )
{
	// TODO
	return 0;
}

/**********************************************************************************************

     okim6258_data_0_w -- generic data write functions
     okim6258_data_1_w

***********************************************************************************************/

WRITE8_HANDLER( okim6258_data_0_w )
{
	okim6258_data_w(0, data);
}

WRITE8_HANDLER( okim6258_data_1_w )
{
	okim6258_data_w(1, data);
}

WRITE8_HANDLER( okim6258_data_2_w )
{
	okim6258_data_w(2, data);
}

WRITE16_HANDLER( okim6258_data_0_lsb_w )
{
	if (ACCESSING_BITS_0_7)
		okim6258_data_w(0, data & 0xff);
}

WRITE16_HANDLER( okim6258_data_1_lsb_w )
{
	if (ACCESSING_BITS_0_7)
		okim6258_data_w(1, data & 0xff);
}

WRITE16_HANDLER( okim6258_data_2_lsb_w )
{
	if (ACCESSING_BITS_0_7)
		okim6258_data_w(2, data & 0xff);
}

WRITE16_HANDLER( okim6258_data_0_msb_w )
{
	if (ACCESSING_BITS_8_15)
		okim6258_data_w(0, data >> 8);
}

WRITE16_HANDLER( okim6258_data_1_msb_w )
{
	if (ACCESSING_BITS_8_15)
		okim6258_data_w(1, data >> 8);
}

WRITE16_HANDLER( okim6258_data_2_msb_w )
{
	if (ACCESSING_BITS_8_15)
		okim6258_data_w(2, data >> 8);
}

WRITE16_HANDLER( okim6258_ctrl_0_lsb_w )
{
	if (ACCESSING_BITS_0_7)
		okim6258_ctrl_w(0, data & 0xff);
}


/**************************************************************************
 * Generic get_info
 **************************************************************************/

static SND_SET_INFO( okim6258 )
{
	switch (state)
	{
		/* no parameters to set */
	}
}


SND_GET_INFO( okim6258 )
{
	switch (state)
	{
		/* --- the following bits of info are returned as 64-bit signed integers --- */

		/* --- the following bits of info are returned as pointers to data or functions --- */
		case SNDINFO_PTR_SET_INFO:						info->set_info = SND_SET_INFO_NAME(okim6258); break;
		case SNDINFO_PTR_START:							info->start = SND_START_NAME(okim6258);	break;
		case SNDINFO_PTR_STOP:							/* nothing */							break;
		case SNDINFO_PTR_RESET:							info->reset = SND_RESET_NAME(okim6258);	break;

		/* --- the following bits of info are returned as NULL-terminated strings --- */
		case SNDINFO_STR_NAME:							info->s = "OKI6258";					break;
		case SNDINFO_STR_CORE_FAMILY:					info->s = "OKI ADPCM";					break;
		case SNDINFO_STR_CORE_VERSION:					info->s = "1.0";						break;
		case SNDINFO_STR_CORE_FILE:						info->s = __FILE__;						break;
		case SNDINFO_STR_CORE_CREDITS:					info->s = "Copyright Nicola Salmoria and the MAME Team"; break;
	}
}

