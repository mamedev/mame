/**********************************************************************************************
 *
 *   OKI MSM6258 ADPCM
 *
 *   TODO:
 *   3-bit ADPCM support
 *   Recording?
 *
 **********************************************************************************************/


#include "emu.h"
#include "okim6258.h"

#define COMMAND_STOP		(1 << 0)
#define COMMAND_PLAY		(1 << 1)
#define	COMMAND_RECORD		(1 << 2)

#define STATUS_PLAYING		(1 << 1)
#define STATUS_RECORDING	(1 << 2)

static const int dividers[4] = { 1024, 768, 512, 512 };

typedef struct _okim6258_state okim6258_state;
struct _okim6258_state
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

INLINE okim6258_state *get_safe_token(device_t *device)
{
	assert(device != NULL);
	assert(device->type() == OKIM6258);
	return (okim6258_state *)downcast<okim6258_device *>(device)->token();
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


static INT16 clock_adpcm(okim6258_state *chip, UINT8 nibble)
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

static STREAM_UPDATE( okim6258_update )
{
	okim6258_state *chip = (okim6258_state *)param;
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

static void okim6258_state_save_register(okim6258_state *info, device_t *device)
{
	device->save_item(NAME(info->status));
	device->save_item(NAME(info->master_clock));
	device->save_item(NAME(info->divider));
	device->save_item(NAME(info->data_in));
	device->save_item(NAME(info->nibble_shift));
	device->save_item(NAME(info->signal));
	device->save_item(NAME(info->step));
}


/**********************************************************************************************

     OKIM6258_start -- start emulation of an OKIM6258-compatible chip

***********************************************************************************************/

static DEVICE_START( okim6258 )
{
	const okim6258_interface *intf = (const okim6258_interface *)device->static_config();
	okim6258_state *info = get_safe_token(device);

	compute_tables();

	info->master_clock = device->clock();
	info->adpcm_type = intf->adpcm_type;

	/* D/A precision is 10-bits but 12-bit data can be output serially to an external DAC */
	info->output_bits = intf->output_12bits ? 12 : 10;
	info->divider = dividers[intf->divider];

	info->stream = device->machine().sound().stream_alloc(*device, 0, 1, device->clock()/info->divider, info, okim6258_update);

	info->signal = -2;
	info->step = 0;

	okim6258_state_save_register(info, device);
}


/**********************************************************************************************

     OKIM6258_stop -- stop emulation of an OKIM6258-compatible chip

***********************************************************************************************/

static DEVICE_RESET( okim6258 )
{
	okim6258_state *info = get_safe_token(device);

	info->stream->update();

	info->signal = -2;
	info->step = 0;
	info->status = 0;
}


/**********************************************************************************************

     okim6258_set_divider -- set the master clock divider

***********************************************************************************************/

void okim6258_set_divider(device_t *device, int val)
{
	okim6258_state *info = get_safe_token(device);
	int divider = dividers[val];

	info->divider = dividers[val];
	info->stream->set_sample_rate(info->master_clock / divider);
}


/**********************************************************************************************

     okim6258_set_clock -- set the master clock

***********************************************************************************************/

void okim6258_set_clock(device_t *device, int val)
{
	okim6258_state *info = get_safe_token(device);

	info->master_clock = val;
	info->stream->set_sample_rate(info->master_clock / info->divider);
}


/**********************************************************************************************

     okim6258_get_vclk -- get the VCLK/sampling frequency

***********************************************************************************************/

int okim6258_get_vclk(device_t *device)
{
	okim6258_state *info = get_safe_token(device);

	return (info->master_clock / info->divider);
}


/**********************************************************************************************

     okim6258_status_r -- read the status port of an OKIM6258-compatible chip

***********************************************************************************************/

READ8_DEVICE_HANDLER( okim6258_status_r )
{
	okim6258_state *info = get_safe_token(device);

	info->stream->update();

	return (info->status & STATUS_PLAYING) ? 0x00 : 0x80;
}


/**********************************************************************************************

     okim6258_data_w -- write to the control port of an OKIM6258-compatible chip

***********************************************************************************************/
WRITE8_DEVICE_HANDLER( okim6258_data_w )
{
	okim6258_state *info = get_safe_token(device);

	/* update the stream */
	info->stream->update();

	info->data_in = data;
	info->nibble_shift = 0;
}


/**********************************************************************************************

     okim6258_ctrl_w -- write to the control port of an OKIM6258-compatible chip

***********************************************************************************************/

WRITE8_DEVICE_HANDLER( okim6258_ctrl_w )
{
	okim6258_state *info = get_safe_token(device);

	info->stream->update();

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


const device_type OKIM6258 = &device_creator<okim6258_device>;

okim6258_device::okim6258_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
	: device_t(mconfig, OKIM6258, "OKI6258", tag, owner, clock),
	  device_sound_interface(mconfig, *this)
{
	m_token = global_alloc_array_clear(UINT8, sizeof(okim6258_state));
}

//-------------------------------------------------
//  device_config_complete - perform any
//  operations now that the configuration is
//  complete
//-------------------------------------------------

void okim6258_device::device_config_complete()
{
}

//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void okim6258_device::device_start()
{
	DEVICE_START_NAME( okim6258 )(this);
}

//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void okim6258_device::device_reset()
{
	DEVICE_RESET_NAME( okim6258 )(this);
}

//-------------------------------------------------
//  sound_stream_update - handle a stream update
//-------------------------------------------------

void okim6258_device::sound_stream_update(sound_stream &stream, stream_sample_t **inputs, stream_sample_t **outputs, int samples)
{
	// should never get here
	fatalerror("sound_stream_update called; not applicable to legacy sound devices\n");
}


