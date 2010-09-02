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
 *  separate MSM5205 emulator form adpcm.c and some fix
 */

#include "emu.h"
#include "streams.h"
#include "msm5205.h"

/*
 *
 *  MSM 5205 ADPCM chip:
 *
 *  Data is streamed from a CPU by means of a clock generated on the chip.
 *
 *  A reset signal is set high or low to determine whether playback (and interrupts) are occuring
 *
 */

typedef struct _msm5205_state msm5205_state;
struct _msm5205_state
{
	const msm5205_interface *intf;
	running_device *device;
	sound_stream * stream;  /* number of stream system      */
	INT32 clock;				/* clock rate */
	emu_timer *timer;        /* VCLK callback timer          */
	INT32 data;               /* next adpcm data              */
	INT32 vclk;               /* vclk signal (external mode)  */
	INT32 reset;              /* reset pin signal             */
	INT32 prescaler;          /* prescaler selector S1 and S2 */
	INT32 bitwidth;           /* bit width selector -3B/4B    */
	INT32 signal;             /* current ADPCM signal         */
	INT32 step;               /* current ADPCM step           */
	int diff_lookup[49*16];
};

INLINE msm5205_state *get_safe_token(running_device *device)
{
	assert(device != NULL);
	assert(device->type() == MSM5205);
	return (msm5205_state *)downcast<legacy_device_base *>(device)->token();
}


static void msm5205_playmode(msm5205_state *voice,int select);

/*
 * ADPCM lockup tabe
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

/* timer callback at VCLK low eddge */
static TIMER_CALLBACK( MSM5205_vclk_callback )
{
	msm5205_state *voice = (msm5205_state *)ptr;
	int val;
	int new_signal;
	/* callback user handler and latch next data */
	if(voice->intf->vclk_callback) (*voice->intf->vclk_callback)(voice->device);

	/* reset check at last hieddge of VCLK */
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
		stream_update(voice->stream);
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
	voice->intf = (const msm5205_interface *)device->baseconfig().static_config();
	voice->device = device;
	voice->clock = device->clock();

	/* compute the difference tables */
	ComputeTables (voice);

	/* stream system initialize */
	voice->stream = stream_create(device,0,1,device->clock(),voice,MSM5205_update);
	voice->timer = timer_alloc(device->machine, MSM5205_vclk_callback, voice);

	/* initialize */
	DEVICE_RESET_CALL(msm5205);

	/* register for save states */
	state_save_register_device_item(device, 0, voice->clock);
	state_save_register_device_item(device, 0, voice->data);
	state_save_register_device_item(device, 0, voice->vclk);
	state_save_register_device_item(device, 0, voice->reset);
	state_save_register_device_item(device, 0, voice->prescaler);
	state_save_register_device_item(device, 0, voice->bitwidth);
	state_save_register_device_item(device, 0, voice->signal);
	state_save_register_device_item(device, 0, voice->step);
}

/*
 *    Handle an update of the vclk status of a chip (1 is reset ON, 0 is reset OFF)
 *    This function can use selector = MSM5205_SEX only
 */
void msm5205_vclk_w (running_device *device, int vclk)
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
			if( !vclk ) MSM5205_vclk_callback(voice->device->machine, voice, 0);
		}
	}
}

/*
 *    Handle an update of the reset status of a chip (1 is reset ON, 0 is reset OFF)
 */

void msm5205_reset_w (running_device *device, int reset)
{
	msm5205_state *voice = get_safe_token(device);
	voice->reset = reset;
}

/*
 *    Handle an update of the data to the chip
 */

void msm5205_data_w (running_device *device, int data)
{
	msm5205_state *voice = get_safe_token(device);
	if( voice->bitwidth == 4)
		voice->data = data & 0x0f;
	else
		voice->data = (data & 0x07)<<1; /* unknown */
}

/*
 *    Handle an change of the selector
 */

void msm5205_playmode_w(running_device *device, int select)
{
	msm5205_state *voice = get_safe_token(device);
	msm5205_playmode(voice,select);
}

static void msm5205_playmode(msm5205_state *voice,int select)
{
	static const int prescaler_table[4] = {96,48,64,0};
	int prescaler = prescaler_table[select & 3];
	int bitwidth = (select & 4) ? 4 : 3;


	if( voice->prescaler != prescaler )
	{
		stream_update(voice->stream);

		voice->prescaler = prescaler;
		/* timer set */
		if( prescaler )
		{
			attotime period = attotime_mul(ATTOTIME_IN_HZ(voice->clock), prescaler);
			timer_adjust_periodic(voice->timer, period, 0, period);
		}
		else
			timer_adjust_oneshot(voice->timer, attotime_never, 0);
	}

	if( voice->bitwidth != bitwidth )
	{
		stream_update(voice->stream);

		voice->bitwidth = bitwidth;
	}
}


void msm5205_set_volume(running_device *device,int volume)
{
	msm5205_state *voice = get_safe_token(device);

	stream_set_output_gain(voice->stream,0,volume / 100.0);
}




/**************************************************************************
 * Generic get_info
 **************************************************************************/

DEVICE_GET_INFO( msm5205 )
{
	switch (state)
	{
		/* --- the following bits of info are returned as 64-bit signed integers --- */
		case DEVINFO_INT_TOKEN_BYTES:					info->i = sizeof(msm5205_state);				break;

		/* --- the following bits of info are returned as pointers to data or functions --- */
		case DEVINFO_FCT_START:							info->start = DEVICE_START_NAME( msm5205 );		break;
		case DEVINFO_FCT_STOP:							/* nothing */									break;
		case DEVINFO_FCT_RESET:							info->reset = DEVICE_RESET_NAME( msm5205 );		break;

		/* --- the following bits of info are returned as NULL-terminated strings --- */
		case DEVINFO_STR_NAME:							strcpy(info->s, "MSM5205");						break;
		case DEVINFO_STR_FAMILY:					strcpy(info->s, "ADPCM");						break;
		case DEVINFO_STR_VERSION:					strcpy(info->s, "1.0");							break;
		case DEVINFO_STR_SOURCE_FILE:						strcpy(info->s, __FILE__);						break;
		case DEVINFO_STR_CREDITS:					strcpy(info->s, "Copyright Nicola Salmoria and the MAME Team"); break;
	}
}


DEFINE_LEGACY_SOUND_DEVICE(MSM5205, msm5205);
