/**********************************************************************

  Copyright (C) Antoine Mine' 2006

  Philips / Signetics MEA 8000 emulation.

  The MEA 8000 is a speech synthesis chip.
  The French company TMPI (Techni-musique & parole informatique) provided
  speech extensions for several 8-bit computers (Thomson, Amstrad, Oric).
  It was quite popular in France because of its ability to spell 'u'
  (unlike the more widespread SPO 296 chip).

  The synthesis is based on a 4-formant model.
  First, an initial sawtooth noise signal is generated.
  The signal passes through a cascade of 4 filters of increasing frequency.
  Each filter is a second order digital filter with a programmable
  frequency and bandwidth.
  All parameters, including filter parameters, are smoothly interpolated
  for the duration of a frame (8ms, 16ms, 32ms, or 64 ms).

  TODO:
  - REQ output pin
  - optimize mea8000_compute_sample
  - should we accept new frames in slow-stop mode ?

**********************************************************************/

#include <math.h>

#include "emu.h"
#include "mea8000.h"
#include "sound/dac.h"


#define VERBOSE 0

/* define to use double instead of int (slow but useful for debugging) */
#undef FLOAT_MODE



/******************* internal chip data structure ******************/



/* finite machine state controling frames */
typedef enum
{
	MEA8000_STOPPED,    /* nothing to do, timer disabled */
	MEA8000_WAIT_FIRST, /* received pitch, wait for first full trame, timer disabled */
	MEA8000_STARTED,    /* playing a frame, timer on */
	MEA8000_SLOWING,    /* repating last frame with decreasing amplitude, timer on */
} mea8000_state;

ALLOW_SAVE_TYPE( mea8000_state );


typedef struct
{
#ifdef FLOAT_MODE
	double fm, last_fm;         /* frequency, in Hz */
	double bw, last_bw;         /* band-width, in Hz */
	double output, last_output; /* filter state */
#else
	UINT16 fm, last_fm;
	UINT16 bw, last_bw;
	INT32  output, last_output;
#endif
} filter_t;



typedef struct
{

	/* configuration parameters */
	const mea8000_interface* iface;

	/* state */

	mea8000_state state; /* current state */

	UINT8 buf[4]; /* store 4 consecutive data to form a frame info */
	UINT8 bufpos; /* new byte to write in frame info buffer */

	UINT8 cont; /* if no data 0=stop 1=repeat last frame */
	UINT8 roe;  /* enable req output, now unimplemented */

	UINT16 framelength;  /* in samples */
	UINT16 framepos;     /* in samples */
	UINT16 framelog;     /* log2 of framelength */

	INT16 lastsample, sample; /* output samples are interpolated */

	UINT32 phi; /* absolute phase for frequency / noise generator */

	filter_t f[4]; /* filters */

	UINT16 last_ampl, ampl;    /* amplitude * 1000 */
	UINT16 last_pitch, pitch;  /* pitch of sawtooth signal, in Hz */
	UINT8  noise;

	emu_timer *timer;

} mea8000_t;



/******************* utilitiy function and macros ********************/



#define LOG(x) do { if (VERBOSE) logerror x; } while (0)

/* digital filters work at 8 kHz */
#define F0 8096

/* filtered output is supersampled x 8 */
#define SUPERSAMPLING 8

/* actual output pediod */
#define SAMPLING attotime::from_hz((SUPERSAMPLING*F0))


INLINE mea8000_t* get_safe_token( device_t *device )
{
	assert( device != NULL );
	assert( device->type() == MEA8000);
	return (mea8000_t*) downcast<mea8000_device *>(device)->token();
}


/************************* quantization tables ***********************/



/* frequency, in Hz */

static const int fm1_table[32] =
{
	150,  162,  174,  188,  202,  217,  233,  250,
	267,  286,  305,  325,  346,  368,  391,  415,
	440,  466,  494,  523,  554,  587,  622,  659,
	698,  740,  784,  830,  880,  932,  988, 1047
};

static const int fm2_table[32] =
{
	440,  466,  494,  523,  554,  587,  622,  659,
	698,  740,  784,  830,  880,  932,  988, 1047,
	1100, 1179, 1254, 1337, 1428, 1528, 1639, 1761,
	1897, 2047, 2214, 2400, 2609, 2842, 3105, 3400
};

static const int fm3_table[8] =
{
	1179, 1337, 1528, 1761, 2047, 2400, 2842, 3400
};

static const int fm4_table[1] = { 3500 };



/* bandwidth, in Hz */
static const int bw_table[4] = { 726, 309, 125, 50 };



/* amplitude * 1000 */
static const int ampl_table[16] =
{
	0,   8,  11,  16,  22,  31,  44,   62,
	88, 125, 177, 250, 354, 500, 707, 1000
};



/* pitch increment, in Hz / 8 ms */
static const int pi_table[32] =
{
	0, 1,  2,  3,  4,  5,  6,  7,
	8, 9, 10, 11, 12, 13, 14, 15,
	0 /* noise */, -15, -14, -13, -12, -11, -10, -9,
	-8, -7, -6, -5, -4, -3, -2, -1
};



/***************************** REQ **********************************/



static int mea8000_accept_byte( mea8000_t* mea8000 )
{
	return
		mea8000->state == MEA8000_STOPPED ||
		mea8000->state == MEA8000_WAIT_FIRST ||
		(mea8000->state == MEA8000_STARTED && mea8000->bufpos < 4);
}

static void mea8000_update_req( device_t *device )
{
	mea8000_t* mea8000 = get_safe_token( device );
	/* actually, req pulses less than 3us for each new byte,
       it goes back up if there space left in the buffer, or stays low if the
       buffer contains a complete frame and the CPU nees to wait for the next
       frame end to compose a new frame.
    */
	if (mea8000->iface->req_out_func)
		mea8000->iface->req_out_func( device, 0, mea8000_accept_byte( mea8000 ) );
}



/*********************** sound generation ***************************/



/* table amplitude [-QUANT,QUANT] */
#define QUANT 512

/* filter coefficients from frequencies */
#define TABLE_LEN 3600
static int cos_table[TABLE_LEN];  /* fm => cos coefficient */
static int exp_table[TABLE_LEN];  /* bw => exp coefficient */
static int exp2_table[TABLE_LEN]; /* bw => 2*exp coefficient */

/* noise generator table */
#define NOISE_LEN 8192
static int noise_table[NOISE_LEN];



/* precompute tables */
static void mea8000_init_tables( running_machine &machine )
{
	int i;
	for (i=0; i<TABLE_LEN; i++)
	{
		double f = (double)i / F0;
		cos_table[i]  = 2. * cos(2.*M_PI*f) * QUANT;
		exp_table[i]  = exp(-M_PI*f) * QUANT;
		exp2_table[i] = exp(-2*M_PI*f) * QUANT;
	}
	for (i=0; i<NOISE_LEN; i++)
		noise_table[i] = (machine.rand() % (2*QUANT)) - QUANT;
}


#ifndef FLOAT_MODE /* UINT16 version */



/* linear interpolation */
static int mea8000_interp( mea8000_t* mea8000, UINT16 org, UINT16 dst )
{
	return org + (((dst-org) * mea8000->framepos) >> mea8000->framelog);
}



/* apply second order digital filter, sampling at F0 */
static int mea8000_filter_step( mea8000_t* mea8000, int i, int input )
{
	/* frequency */
	int fm = mea8000_interp(mea8000, mea8000->f[i].last_fm, mea8000->f[i].fm);
	/* bandwidth */
	int bw = mea8000_interp(mea8000, mea8000->f[i].last_bw, mea8000->f[i].bw);
	/* filter coefficients */
	int b = (cos_table[fm] * exp_table[bw]) / QUANT;
	int c = exp2_table[bw];
	/* transfer function */
	int next_output = input + (b * mea8000->f[i].output - c * mea8000->f[i].last_output) / QUANT;
	mea8000->f[i].last_output = mea8000->f[i].output;
	mea8000->f[i].output = next_output;
	return next_output;
}


/* random waveform, in [-QUANT,QUANT] */
static int mea8000_noise_gen( mea8000_t* mea8000 )
{
	mea8000->phi = (mea8000->phi + 1) % NOISE_LEN;
	return noise_table[mea8000->phi];
}



/* sawtooth waveform at F0, in [-QUANT,QUANT] */
static int mea8000_freq_gen( mea8000_t* mea8000 )
{
	int pitch = mea8000_interp(mea8000, mea8000->last_pitch, mea8000->pitch);
	mea8000->phi = (mea8000->phi + pitch) % F0;
	return ((mea8000->phi % F0) * QUANT * 2) / F0 - QUANT;
}



/* sample in [-32768,32767], at F0 */
static int mea8000_compute_sample( mea8000_t* mea8000 )
{
	int i;
	int out;
	int ampl = mea8000_interp(mea8000, mea8000->last_ampl, mea8000->ampl);

	if (mea8000->noise)
		out = mea8000_noise_gen(mea8000);
	else
		out = mea8000_freq_gen(mea8000);

	out *= ampl / 32;

	for (i=0; i<4; i++)
	{
		out = mea8000_filter_step(mea8000, i, out);
	}

	if ( out > 32767 )
		out = 32767;
	if ( out < -32767)
		out = -32767;
	return out;
}



#else /* float version */



/* linear interpolation */
static double mea8000_interp( mea8000_t* mea8000, double org, double dst )
{
	return org + ((dst-org) * mea8000->framepos) / mea8000->framelength;
}



/* apply second order digital filter, sampling at F0 */
static double mea8000_filter_step( mea8000_t* mea8000, int i, double input )
{
	double fm = mea8000_interp(mea8000, mea8000->f[i].last_fm, mea8000->f[i].fm);
	double bw = mea8000_interp(mea8000, mea8000->f[i].last_bw, mea8000->f[i].bw);
	double b = 2.*cos(2.*M_PI*fm/F0);
	double c = -exp(-M_PI*bw/F0);
	double next_output =
		input -
		c * (b * mea8000->f[i].output + c * mea8000->f[i].last_output);
	mea8000->f[i].last_output = mea8000->f[i].output;
	mea8000->f[i].output = next_output;
	return next_output;
}



/* noise, in [-1,1] */
static double mea8000_noise_gen( mea8000_t* mea8000 )
{
	mea8000->phi++;
	return (double) noise_table[mea8000->phi % NOISE_LEN] / QUANT;
}



/* sawtooth waveform at F0, in [-1,1] */
static double mea8000_freq_gen( mea8000_t* mea8000 )
{
	int pitch = mea8000_interp(mea8000, mea8000->last_pitch, mea8000->pitch);
	mea8000->phi += pitch;
	return (double) (mea8000->phi % F0) / (F0/2.) - 1.;
}


/* sample in [-32767,32767], at F0 */
static int mea8000_compute_sample( mea8000_t* mea8000 )
{
	int i;
	double out;
	double ampl = mea8000_interp(mea8000, 8.*mea8000->last_ampl, 8.*mea8000->ampl);

	if (mea8000->noise)
		out = mea8000_noise_gen(mea8000);
	else
		out = mea8000_freq_gen(mea8000);

	out *= ampl;

	for (i=0; i<4; i++)
	{
		out = mea8000_filter_step(mea8000, i, out);
	}

	if ( out > 32767 )
		out = 32767;
	if ( out < -32767)
		out = -32767;
	return out;
}


#endif



/*********************** frame management ***************************/



/* shift frame parameters from current to last */
static void mea8000_shift_frame( mea8000_t* mea8000 )
{
	int i;
	mea8000->last_pitch = mea8000->pitch;
	for (i=0; i<4; i++)
	{
		mea8000->f[i].last_bw = mea8000->f[i].bw;
		mea8000->f[i].last_fm = mea8000->f[i].fm;
	}
	mea8000->last_ampl = mea8000->ampl;
}



/* decode fields from buffer to current frame */
static void mea8000_decode_frame( mea8000_t* mea8000 )
{
	int fd = (mea8000->buf[3] >> 5) & 3; /* 0=8ms, 1=16ms, 2=32ms, 3=64ms */
	int pi = pi_table[ mea8000->buf[3] & 0x1f ] << fd;
	mea8000->noise = (mea8000->buf[3] & 0x1f) == 16;
	mea8000->pitch = mea8000->last_pitch + pi;
	mea8000->f[0].bw = bw_table[ mea8000->buf[0] >> 6 ];
	mea8000->f[1].bw = bw_table[ (mea8000->buf[0] >> 4) & 3 ];
	mea8000->f[2].bw = bw_table[ (mea8000->buf[0] >> 2) & 3 ];
	mea8000->f[3].bw = bw_table[ mea8000->buf[0] & 3 ];
	mea8000->f[3].fm = fm4_table[ 0 ];
	mea8000->f[2].fm = fm3_table[ mea8000->buf[1] >> 5 ];
	mea8000->f[1].fm = fm2_table[ mea8000->buf[1] & 0x1f ];
	mea8000->f[0].fm = fm1_table[ mea8000->buf[2] >> 3 ];
	mea8000->ampl = ampl_table[ ((mea8000->buf[2] & 7) << 1) |
				   (mea8000->buf[3] >> 7) ];
	mea8000->framelog = fd + 6 /* 64 samples / ms */ + 3;
	mea8000->framelength = 1 <<  mea8000->framelog;
	mea8000->bufpos = 0;
#ifdef FLOAT_MODE
	LOG(( "mea800_decode_frame: pitch=%i noise=%i  fm1=%gHz bw1=%gHz  fm2=%gHz bw2=%gHz  fm3=%gHz bw3=%gHz  fm4=%gHz bw4=%gHz  ampl=%g fd=%ims\n",
	      mea8000->pitch, mea8000->noise,
	      mea8000->f[0].fm, mea8000->f[0].bw, mea8000->f[1].fm, mea8000->f[1].bw,
	      mea8000->f[2].fm, mea8000->f[2].bw, mea8000->f[3].fm, mea8000->f[3].bw,
	      mea8000->ampl/1000., 8 << fd ));
#else
	LOG(( "mea800_decode_frame: pitch=%i noise=%i  fm1=%iHz bw1=%iHz  fm2=%iHz bw2=%iHz  fm3=%iHz bw3=%iHz  fm4=%iHz bw4=%iHz  ampl=%g fd=%ims\n",
	      mea8000->pitch, mea8000->noise,
	      mea8000->f[0].fm, mea8000->f[0].bw, mea8000->f[1].fm, mea8000->f[1].bw,
	      mea8000->f[2].fm, mea8000->f[2].bw, mea8000->f[3].fm, mea8000->f[3].bw,
	      mea8000->ampl/1000., 8 << fd ));
#endif
}



static void mea8000_start_frame( mea8000_t* mea8000 )
{
	/* enter or stay in active mode */
	mea8000->timer->reset( SAMPLING );
	mea8000->framepos = 0;
}



static void mea8000_stop_frame( running_machine &machine, mea8000_t* mea8000 )
{
	/* enter stop mode */
	mea8000->timer->reset(  );
	mea8000->state = MEA8000_STOPPED;
	machine.device<dac_device>(mea8000->iface->channel)->write_signed16(0x8000);
}



/* next sample in frame, sampling at 64 kHz */
static TIMER_CALLBACK( mea8000_timer_expire )
{
	device_t* device = (device_t*) ptr;
	mea8000_t* mea8000 = get_safe_token( device );
	int pos = mea8000->framepos % SUPERSAMPLING;

	if (!pos)
	{
		/* sample is really computed only every 8-th time */
		mea8000->lastsample = mea8000->sample;
		mea8000->sample = mea8000_compute_sample(mea8000);
		machine.device<dac_device>(mea8000->iface->channel)->write_signed16(0x8000+mea8000->lastsample);
	}
	else
	{
		/* other samples are simply interpolated */
		int sample =
			mea8000->lastsample +
			((pos*(mea8000->sample-mea8000->lastsample)) / SUPERSAMPLING);
		machine.device<dac_device>(mea8000->iface->channel)->write_signed16(0x8000+sample);
	}

	mea8000->framepos++;
	if (mea8000->framepos >= mea8000->framelength)
	{
		mea8000_shift_frame(mea8000);
		/* end of frame */
		if (mea8000->bufpos == 4)
		{
			/* we have a successor */
			LOG(( "%f mea8000_timer_expire: new frame\n", machine.time().as_double() ));
			mea8000_decode_frame(mea8000);
			mea8000_start_frame(mea8000);
		}
		else if (mea8000->cont)
		{
			/* repeat mode */
			LOG(( "%f mea8000_timer_expire: repeat frame\n", machine.time().as_double() ));
			mea8000_start_frame(mea8000);
		}
		/* slow stop */
		else if (mea8000->state == MEA8000_STARTED)
		{
			mea8000->ampl = 0;
			LOG(( "%f mea8000_timer_expire: fade frame\n", machine.time().as_double() ));
			mea8000_start_frame(mea8000);
			mea8000->state = MEA8000_SLOWING;
		}
		else if (mea8000->state == MEA8000_SLOWING)
		{
			LOG(( "%f mea8000_timer_expire: stop frame\n", machine.time().as_double() ));
			mea8000_stop_frame(machine, mea8000);
		}
		mea8000_update_req(device);
	}
	else
	{
		/* continue frame */
		mea8000->timer->reset( SAMPLING );
	}
}



/************************** CPU interface ****************************/



READ8_DEVICE_HANDLER ( mea8000_r )
{
	mea8000_t* mea8000 = get_safe_token( device );
	switch ( offset )
	{

	case 0: /* status register */
	case 1:
		/* ready to accept next frame */
#if 0
		LOG(( "$%04x %f: mea8000_r ready=%i\n", cpu_get_previouspc( device->machine().firstcpu ), machine.time().as_double(), mea8000_accept_byte( mea8000 ) ));
#endif
		return mea8000_accept_byte(mea8000) << 7;

	default:
		logerror( "$%04x mea8000_r invalid read offset %i\n",  cpu_get_previouspc( device->machine().firstcpu ), offset );
	}
	return 0;
}

WRITE8_DEVICE_HANDLER ( mea8000_w )
{
	mea8000_t* mea8000 = get_safe_token( device );
	switch ( offset )
	{

	case 0: /* data register */
		if (mea8000->state == MEA8000_STOPPED)
		{
			/* got pitch byte before first frame */
			mea8000->pitch = 2 * data;
			LOG(( "$%04x %f: mea8000_w pitch %i\n", cpu_get_previouspc( device->machine().firstcpu ), device->machine().time().as_double(), mea8000->pitch ));
			mea8000->state = MEA8000_WAIT_FIRST;
			mea8000->bufpos = 0;
		}
		else if (mea8000->bufpos == 4)
		{
			/* overflow */
			LOG(( "$%04x %f: mea8000_w data overflow %02X\n", cpu_get_previouspc( device->machine().firstcpu ), device->machine().time().as_double(), data ));
		}
		else
		{
			/* enqueue frame byte */
			LOG(( "$%04x %f: mea8000_w data %02X in frame pos %i\n", cpu_get_previouspc( device->machine().firstcpu ), device->machine().time().as_double(),
			      data, mea8000->bufpos ));
			mea8000->buf[mea8000->bufpos] = data;
			mea8000->bufpos++;
			if (mea8000->bufpos == 4 && mea8000->state == MEA8000_WAIT_FIRST)
			{
				/* fade-in first frame */
				int old_pitch = mea8000->pitch;
				mea8000->last_pitch = old_pitch;
				mea8000_decode_frame(mea8000);
				mea8000_shift_frame(mea8000);
				mea8000->last_pitch = old_pitch;
				mea8000->ampl = 0;
				mea8000_start_frame(mea8000);
				mea8000->state = MEA8000_STARTED;
			}
		}
		mea8000_update_req(device);
		break;

	case 1: /* command register */
	{
		int stop = (data >> 4) & 1;

		if (data & 8)
			mea8000->cont = (data >> 2) & 1;

		if (data & 2)
			mea8000->roe = data & 1;

		if (stop)
			mea8000_stop_frame(device->machine(), mea8000);

		LOG(( "$%04x %f: mea8000_w command %02X stop=%i cont=%i roe=%i\n",
		      cpu_get_previouspc(device->machine().firstcpu), device->machine().time().as_double(), data,
		      stop, mea8000->cont, mea8000->roe ));

		mea8000_update_req(device);
		break;
	}

	default:
		logerror( "$%04x mea8000_w invalid write offset %i\n", cpu_get_previouspc( device->machine().firstcpu ), offset );
	}
}



/************************ reset *****************************/

static DEVICE_RESET( mea8000 )
{
	mea8000_t* mea8000 = get_safe_token( device );
	int i;
	LOG (( "mea8000_reset\n" ));
	mea8000->timer->reset(  );
	mea8000->phi = 0;
	mea8000->cont = 0;
	mea8000->roe = 0;
	mea8000->state = MEA8000_STOPPED;
	mea8000_update_req(device);
	for (i=0; i<4; i++)
	{
		mea8000->f[i].last_output = 0;
		mea8000->f[i].output = 0;
	}

}



/****************************** start ********************************/


static DEVICE_START( mea8000 )
{
	mea8000_t* mea8000 = get_safe_token( device );
	int i;
	mea8000->iface = (const mea8000_interface*)device->static_config();

	mea8000_init_tables(device->machine());

	mea8000->timer = device->machine().scheduler().timer_alloc(FUNC(mea8000_timer_expire), (void*)device );

	state_save_register_item( device->machine(), "mea8000", device->tag(), 0, mea8000->state );
	state_save_register_item_array( device->machine(), "mea8000", device->tag(), 0, mea8000->buf );
	state_save_register_item( device->machine(), "mea8000", device->tag(), 0, mea8000->bufpos );
	state_save_register_item( device->machine(), "mea8000", device->tag(), 0, mea8000->cont );
	state_save_register_item( device->machine(), "mea8000", device->tag(), 0, mea8000->roe );
	state_save_register_item( device->machine(), "mea8000", device->tag(), 0, mea8000->framelength );
	state_save_register_item( device->machine(), "mea8000", device->tag(), 0, mea8000->framepos );
	state_save_register_item( device->machine(), "mea8000", device->tag(), 0, mea8000->framelog );
	state_save_register_item( device->machine(), "mea8000", device->tag(), 0, mea8000->lastsample );
	state_save_register_item( device->machine(), "mea8000", device->tag(), 0, mea8000->sample );
	state_save_register_item( device->machine(), "mea8000", device->tag(), 0, mea8000->phi );
	for (i=0; i<4; i++)
	{
		state_save_register_item( device->machine(), "mea8000", device->tag(), i, mea8000->f[i].fm );
		state_save_register_item( device->machine(), "mea8000", device->tag(), i, mea8000->f[i].last_fm );
		state_save_register_item( device->machine(), "mea8000", device->tag(), i, mea8000->f[i].bw );
		state_save_register_item( device->machine(), "mea8000", device->tag(), i, mea8000->f[i].last_bw );
		state_save_register_item( device->machine(), "mea8000", device->tag(), i, mea8000->f[i].output );
		state_save_register_item( device->machine(), "mea8000", device->tag(), i, mea8000->f[i].last_output );
	}
	state_save_register_item( device->machine(), "mea8000", device->tag(), 0, mea8000->last_ampl );
	state_save_register_item( device->machine(), "mea8000", device->tag(), 0, mea8000->ampl );
	state_save_register_item( device->machine(), "mea8000", device->tag(), 0, mea8000->last_pitch );
	state_save_register_item( device->machine(), "mea8000", device->tag(), 0, mea8000->pitch );
	state_save_register_item( device->machine(), "mea8000", device->tag(), 0, mea8000->noise );
}


/************************** configuration ****************************/

DEVICE_GET_INFO( mea8000 ) {
	switch ( state ) {
		/* --- the following bits of info are returned as 64-bit signed integers --- */
		case DEVINFO_INT_TOKEN_BYTES:			info->i = sizeof(mea8000_t);			break;

		/* --- the following bits of info are returned as pointers to data or functions --- */
		case DEVINFO_FCT_START:				info->start = DEVICE_START_NAME(mea8000);	break;
		case DEVINFO_FCT_STOP:				/* nothing */					break;
		case DEVINFO_FCT_RESET:				info->reset = DEVICE_RESET_NAME(mea8000);	break;

		/* --- the following bits of info are returned as NULL-terminated strings --- */
		case DEVINFO_STR_NAME:				strcpy(info->s, "Philips / Signetics MEA 8000 speech synthesizer");		break;
		case DEVINFO_STR_FAMILY:			strcpy(info->s, "MEA8000");				break;
		case DEVINFO_STR_VERSION:			strcpy(info->s, "1.00");				break;
		case DEVINFO_STR_SOURCE_FILE:		strcpy(info->s, __FILE__);				break;
		case DEVINFO_STR_CREDITS:			strcpy(info->s, "Copyright the MAME and MESS Teams");  break;
	}
}

const device_type MEA8000 = &device_creator<mea8000_device>;

mea8000_device::mea8000_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
	: device_t(mconfig, MEA8000, "Philips / Signetics MEA 8000 speech synthesizer", tag, owner, clock)
{
	m_token = global_alloc_array_clear(UINT8, sizeof(mea8000_t));
}

//-------------------------------------------------
//  device_config_complete - perform any
//  operations now that the configuration is
//  complete
//-------------------------------------------------

void mea8000_device::device_config_complete()
{
}

//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void mea8000_device::device_start()
{
	DEVICE_START_NAME( mea8000 )(this);
}

//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void mea8000_device::device_reset()
{
	DEVICE_RESET_NAME( mea8000 )(this);
}


