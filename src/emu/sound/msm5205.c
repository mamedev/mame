/*
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
 *   01/06/99
 *    separate MSM5205 emulator form adpcm.c and some fix
 *
 *   07/29/12
 *    added basic support for the MSM6585
 */

#include "emu.h"
#include "msm5205.h"

/*

    MSM 5205 ADPCM chip:

    Data is streamed from a CPU by means of a clock generated on the chip.

    A reset signal is set high or low to determine whether playback (and interrupts) are occuring.

  MSM6585: is an upgraded MSM5205 voice synth IC.
   Improvements:
    More precise internal DA converter
    Built in low-pass filter
    Expanded sampling frequency

   Differences between MSM6585 & MSM5205:

                              MSM6586          MSM5205
    Master clock frequency    640kHz           384kHz
    Sampling frequency        4k/8k/16k/32kHz  4k/6k/8kHz
    ADPCM bit length          4-bit            3-bit/4-bit
    DA converter              12-bit           10-bit
    Low-pass filter           -40dB/oct        N/A
    Overflow prevent circuit  Included         N/A

    Timer callback at VCLK low edge on MSM5205 (at rising edge on MSM6585)

   TODO:
   - convert to modern
   - lowpass filter for MSM6585

 */

struct msm5205_state
{
	const msm5205_interface *intf;
	device_t *device;
	sound_stream * stream;    /* number of stream system      */
	INT32 clock;              /* clock rate                   */
	emu_timer *timer;         /* VCLK callback timer          */
	INT32 data;               /* next adpcm data              */
	INT32 vclk;               /* vclk signal (external mode)  */
	INT32 reset;              /* reset pin signal             */
	INT32 prescaler;          /* prescaler selector S1 and S2 */
	INT32 bitwidth;           /* bit width selector -3B/4B    */
	INT32 signal;             /* current ADPCM signal         */
	INT32 step;               /* current ADPCM step           */
	int diff_lookup[49*16];
};

INLINE msm5205_state *get_safe_token(device_t *device)
{
	assert(device != NULL);
	assert(device->type() == MSM5205 || device->type() == MSM6585);
	return (msm5205_state *)downcast<msm5205_device *>(device)->token();
}


static void msm5205_playmode(msm5205_state *voice,int select);

/*
 * ADPCM lookup table
 */

/* step size index shift table */
static const int index_shift[8] = { -1, -1, -1, -1, 2, 4, 6, 8 };

/*
 *   Compute the difference table
 */

static void ComputeTables (msm5205_state *voice)
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
		int stepval = floor (16.0 * pow (11.0 / 10.0, (double)step));

		/* loop over all nibbles and compute the difference */
		for (nib = 0; nib < 16; nib++)
		{
			voice->diff_lookup[step*16 + nib] = nbl2bit[nib][0] *
				(stepval   * nbl2bit[nib][1] +
				 stepval/2 * nbl2bit[nib][2] +
				 stepval/4 * nbl2bit[nib][3] +
				 stepval/8);
		}
	}
}

/* stream update callbacks */
static STREAM_UPDATE( MSM5205_update )
{
	msm5205_state *voice = (msm5205_state *)param;
	stream_sample_t *buffer = outputs[0];

	/* if this voice is active */
	if(voice->signal)
	{
		short val = voice->signal * 16;
		while (samples)
		{
			*buffer++ = val;
			samples--;
		}
	}
	else
		memset (buffer,0,samples*sizeof(*buffer));
}

/* timer callback at VCLK low edge on MSM5205 (at rising edge on MSM6585) */
static TIMER_CALLBACK( MSM5205_vclk_callback )
{
	msm5205_state *voice = (msm5205_state *)ptr;
	int val;
	int new_signal;

	/* callback user handler and latch next data */
	if(voice->intf->vclk_callback) (*voice->intf->vclk_callback)(voice->device);

	/* reset check at last hiedge of VCLK */
	if(voice->reset)
	{
		new_signal = 0;
		voice->step = 0;
	}
	else
	{
		/* update signal */
		/* !! MSM5205 has internal 12bit decoding, signal width is 0 to 8191 !! */
		val = voice->data;
		new_signal = voice->signal + voice->diff_lookup[voice->step * 16 + (val & 15)];
		if (new_signal > 2047) new_signal = 2047;
		else if (new_signal < -2048) new_signal = -2048;
		voice->step += index_shift[val & 7];
		if (voice->step > 48) voice->step = 48;
		else if (voice->step < 0) voice->step = 0;
	}

	/* update when signal changed */
	if( voice->signal != new_signal)
	{
		voice->stream->update();
		voice->signal = new_signal;
	}
}

/*
 *    Reset emulation of an MSM5205-compatible chip
 */
static DEVICE_RESET( msm5205 )
{
	msm5205_state *voice = get_safe_token(device);

	/* initialize work */
	voice->data    = 0;
	voice->vclk    = 0;
	voice->reset   = 0;
	voice->signal  = 0;
	voice->step    = 0;

	/* timer and bitwidth set */
	msm5205_playmode(voice,voice->intf->select);
}


/*
 *    Start emulation of an MSM5205-compatible chip
 */

static DEVICE_START( msm5205 )
{
	msm5205_state *voice = get_safe_token(device);

	/* save a global pointer to our interface */
	voice->intf = (const msm5205_interface *)device->static_config();
	voice->device = device;
	voice->clock = device->clock();

	/* compute the difference tables */
	ComputeTables (voice);

	/* stream system initialize */
	voice->stream = device->machine().sound().stream_alloc(*device,0,1,device->clock(),voice,MSM5205_update);
	voice->timer = device->machine().scheduler().timer_alloc(FUNC(MSM5205_vclk_callback), voice);

	/* initialize */
	DEVICE_RESET_CALL(msm5205);

	/* register for save states */
	device->save_item(NAME(voice->clock));
	device->save_item(NAME(voice->data));
	device->save_item(NAME(voice->vclk));
	device->save_item(NAME(voice->reset));
	device->save_item(NAME(voice->prescaler));
	device->save_item(NAME(voice->bitwidth));
	device->save_item(NAME(voice->signal));
	device->save_item(NAME(voice->step));
}

/*
 *    Handle an update of the vclk status of a chip (1 is reset ON, 0 is reset OFF)
 *    This function can use selector = MSM5205_SEX only
 */
void msm5205_vclk_w (device_t *device, int vclk)
{
	msm5205_state *voice = get_safe_token(device);

	if( voice->prescaler != 0 )
	{
		logerror("error: msm5205_vclk_w() called with chip = '%s', but VCLK selected master mode\n", device->tag());
	}
	else
	{
		if( voice->vclk != vclk)
		{
			voice->vclk = vclk;
			if( !vclk ) MSM5205_vclk_callback(voice->device->machine(), voice, 0);
		}
	}
}

/*
 *    Handle an update of the reset status of a chip (1 is reset ON, 0 is reset OFF)
 */

void msm5205_reset_w (device_t *device, int reset)
{
	msm5205_state *voice = get_safe_token(device);
	voice->reset = reset;
}

/*
 *    Handle an update of the data to the chip
 */

void msm5205_data_w (device_t *device, int data)
{
	msm5205_state *voice = get_safe_token(device);
	if( voice->bitwidth == 4)
		voice->data = data & 0x0f;
	else
		voice->data = (data & 0x07)<<1; /* unknown */
}

/*
 *    Handle a change of the selector
 */

void msm5205_playmode_w(device_t *device, int select)
{
	msm5205_state *voice = get_safe_token(device);
	msm5205_playmode(voice, select);
}

static void msm5205_playmode(msm5205_state *voice, int select)
{
	static const int prescaler_table[2][4] =
	{
		{ 96, 48, 64,  0},
		{160, 40, 80, 20}
	};
	int prescaler = prescaler_table[select >> 3 & 1][select & 3];
	int bitwidth = (select & 4) ? 4 : 3;


	if( voice->prescaler != prescaler )
	{
		voice->stream->update();

		voice->prescaler = prescaler;

		/* timer set */
		if( prescaler )
		{
			attotime period = attotime::from_hz(voice->clock) * prescaler;
			voice->timer->adjust(period, 0, period);
		}
		else
			voice->timer->adjust(attotime::never);
	}

	if( voice->bitwidth != bitwidth )
	{
		voice->stream->update();

		voice->bitwidth = bitwidth;
	}
}


void msm5205_set_volume(device_t *device,int volume)
{
	msm5205_state *voice = get_safe_token(device);

	voice->stream->set_output_gain(0,volume / 100.0);
}

void msm5205_change_clock_w(device_t *device, INT32 clock)
{
	msm5205_state *voice = get_safe_token(device);
	attotime period;

	voice->clock = clock;

	period = attotime::from_hz(voice->clock) * voice->prescaler;
	voice->timer->adjust(period, 0, period);
}

const device_type MSM5205 = &device_creator<msm5205_device>;

msm5205_device::msm5205_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
	: device_t(mconfig, MSM5205, "MSM5205", tag, owner, clock),
	  device_sound_interface(mconfig, *this)
{
	m_token = global_alloc_array_clear(UINT8, sizeof(msm5205_state));
}
msm5205_device::msm5205_device(const machine_config &mconfig, device_type type, const char *name, const char *tag, device_t *owner, UINT32 clock)
	: device_t(mconfig, type, name, tag, owner, clock),
	  device_sound_interface(mconfig, *this)
{
	m_token = global_alloc_array_clear(UINT8, sizeof(msm5205_state));
}

//-------------------------------------------------
//  device_config_complete - perform any
//  operations now that the configuration is
//  complete
//-------------------------------------------------

void msm5205_device::device_config_complete()
{
}

//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void msm5205_device::device_start()
{
	DEVICE_START_NAME( msm5205 )(this);
}

//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void msm5205_device::device_reset()
{
	DEVICE_RESET_NAME( msm5205 )(this);
}

//-------------------------------------------------
//  sound_stream_update - handle a stream update
//-------------------------------------------------

void msm5205_device::sound_stream_update(sound_stream &stream, stream_sample_t **inputs, stream_sample_t **outputs, int samples)
{
	// should never get here
	fatalerror("sound_stream_update called; not applicable to legacy sound devices\n");
}


const device_type MSM6585 = &device_creator<msm6585_device>;

msm6585_device::msm6585_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
	: msm5205_device(mconfig, MSM6585, "MSM6585", tag, owner, clock)
{
}

//-------------------------------------------------
//  sound_stream_update - handle a stream update
//-------------------------------------------------

void msm6585_device::sound_stream_update(sound_stream &stream, stream_sample_t **inputs, stream_sample_t **outputs, int samples)
{
	// should never get here
	fatalerror("sound_stream_update called; not applicable to legacy sound devices\n");
}


