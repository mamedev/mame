// license:BSD-3-Clause
// copyright-holders:Antoine Mine
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


#define VERBOSE 0

/* define to use double instead of int (slow but useful for debugging) */
#undef FLOAT_MODE


/******************* utilitiy function and macros ********************/


#define LOG(x) do { if (VERBOSE) logerror x; } while (0)

/* digital filters work at 8 kHz */
#define F0 8096

/* filtered output is supersampled x 8 */
#define SUPERSAMPLING 8

/* actual output pediod */
#define SAMPLING attotime::from_hz((SUPERSAMPLING*F0))



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



const device_type MEA8000 = &device_creator<mea8000_device>;


mea8000_device::mea8000_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock) :
	device_t(mconfig, MEA8000, "Philips / Signetics MEA 8000 speech synthesizer", tag, owner, clock, "mea8000", __FILE__),
	m_write_req(*this),
	m_dac(*this)
{
}


//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void mea8000_device::device_start()
{
	m_write_req.resolve_safe();

	init_tables();

	m_timer = machine().scheduler().timer_alloc(timer_expired_delegate(FUNC(mea8000_device::timer_expire),this));

	save_item(NAME(m_state));
	save_item(NAME(m_buf));
	save_item(NAME(m_bufpos));
	save_item(NAME(m_cont));
	save_item(NAME(m_roe));
	save_item(NAME(m_framelength));
	save_item(NAME(m_framepos));
	save_item(NAME(m_framelog));
	save_item(NAME(m_lastsample));
	save_item(NAME(m_sample));
	save_item(NAME(m_phi));
	for (int i = 0; i < 4; i++)
	{
		save_item(NAME(m_f[i].fm), i);
		save_item(NAME(m_f[i].last_fm), i);
		save_item(NAME(m_f[i].bw), i);
		save_item(NAME(m_f[i].last_bw), i);
		save_item(NAME(m_f[i].output), i);
		save_item(NAME(m_f[i].last_output), i);
	}
	save_item(NAME(m_last_ampl));
	save_item(NAME(m_ampl));
	save_item(NAME(m_last_pitch));
	save_item(NAME(m_pitch));
	save_item(NAME(m_noise));
}

//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void mea8000_device::device_reset()
{
	LOG(("mea8000_reset\n"));
	m_timer->reset();
	m_phi = 0;
	m_cont = 0;
	m_roe = 0;
	m_state = MEA8000_STOPPED;
	update_req();
	for (int i = 0; i < 4; i++)
	{
		m_f[i].last_output = 0;
		m_f[i].output = 0;
	}
}



/***************************** REQ **********************************/


int mea8000_device::accept_byte()
{
	return m_state == MEA8000_STOPPED || m_state == MEA8000_WAIT_FIRST || (m_state == MEA8000_STARTED && m_bufpos < 4);
}

void mea8000_device::update_req()
{
	// actually, req pulses less than 3us for each new byte,
	// it goes back up if there space left in the buffer, or stays low if the
	// buffer contains a complete frame and the CPU nees to wait for the next
	// frame end to compose a new frame.
	m_write_req(accept_byte());
}



/*********************** sound generation ***************************/


/* precompute tables */
void mea8000_device::init_tables()
{
	for (int i = 0; i < TABLE_LEN; i++)
	{
		double f = (double)i / F0;
		m_cos_table[i]  = 2. * cos(2. * M_PI * f) * QUANT;
		m_exp_table[i]  = exp(-M_PI * f) * QUANT;
		m_exp2_table[i] = exp(-2 * M_PI * f) * QUANT;
	}
	for (int i = 0; i < NOISE_LEN; i++)
		m_noise_table[i] = (machine().rand() % (2 * QUANT)) - QUANT;
}


#ifndef FLOAT_MODE /* UINT16 version */



/* linear interpolation */
int mea8000_device::interp( UINT16 org, UINT16 dst )
{
	return org + (((dst - org) * m_framepos) >> m_framelog);
}


/* apply second order digital filter, sampling at F0 */
int mea8000_device::filter_step( int i, int input )
{
	/* frequency */
	int fm = interp(m_f[i].last_fm, m_f[i].fm);
	/* bandwidth */
	int bw = interp(m_f[i].last_bw, m_f[i].bw);
	/* filter coefficients */
	int b = (m_cos_table[fm] * m_exp_table[bw]) / QUANT;
	int c = m_exp2_table[bw];
	/* transfer function */
	int next_output = input + (b * m_f[i].output - c * m_f[i].last_output) / QUANT;
	m_f[i].last_output = m_f[i].output;
	m_f[i].output = next_output;
	return next_output;
}


/* random waveform, in [-QUANT,QUANT] */
int mea8000_device::noise_gen()
{
	m_phi = (m_phi + 1) % NOISE_LEN;
	return m_noise_table[m_phi];
}


/* sawtooth waveform at F0, in [-QUANT,QUANT] */
int mea8000_device::freq_gen()
{
	int pitch = interp(m_last_pitch, m_pitch);
	m_phi = (m_phi + pitch) % F0;
	return ((m_phi % F0) * QUANT * 2) / F0 - QUANT;
}


/* sample in [-32768,32767], at F0 */
int mea8000_device::compute_sample()
{
	int out;
	int ampl = interp(m_last_ampl, m_ampl);

	if (m_noise)
		out = noise_gen();
	else
		out = freq_gen();

	out *= ampl / 32;

	for (int i = 0; i < 4; i++)
		out = filter_step(i, out);

	if (out > 32767)
		out = 32767;
	if (out < -32767)
		out = -32767;
	return out;
}



#else /* float version */



/* linear interpolation */
double mea8000_device::interp(double org, double dst)
{
	return org + ((dst - org) * m_framepos) / m_framelength;
}


/* apply second order digital filter, sampling at F0 */
double mea8000_device::filter_step(int i, double input)
{
	double fm = interp(m_f[i].last_fm, m_f[i].fm);
	double bw = interp(m_f[i].last_bw, m_f[i].bw);
	double b = 2. * cos(2. * M_PI * fm / F0);
	double c = -exp(-M_PI * bw / F0);
	double next_output = input - c * (b * m_f[i].output + c * m_f[i].last_output);
	m_f[i].last_output = m_f[i].output;
	m_f[i].output = next_output;
	return next_output;
}


/* noise, in [-1,1] */
double mea8000_device::noise_gen()
{
	m_phi++;
	return (double) m_noise_table[m_phi % NOISE_LEN] / QUANT;
}



/* sawtooth waveform at F0, in [-1,1] */
double mea8000_device::freq_gen()
{
	int pitch = interp(m_last_pitch, m_pitch);
	m_phi += pitch;
	return (double) (m_phi % F0) / (F0 / 2.) - 1.;
}


/* sample in [-32767,32767], at F0 */
int mea8000_device::compute_sample()
{
	double out;
	double ampl = interp(8. * m_last_ampl, 8. * m_ampl);

	if (m_noise)
		out = noise_gen();
	else
		out = freq_gen();

	out *= ampl;

	for (int i = 0; i < 4; i++)
	{
		out = filter_step(i, out);
	}

	if (out > 32767)
		out = 32767;
	if (out < -32767)
		out = -32767;
	return out;
}


#endif


/*********************** frame management ***************************/



/* shift frame parameters from current to last */
void mea8000_device::shift_frame()
{
	m_last_pitch = m_pitch;
	for (int i = 0; i < 4; i++)
	{
		m_f[i].last_bw = m_f[i].bw;
		m_f[i].last_fm = m_f[i].fm;
	}
	m_last_ampl = m_ampl;
}



/* decode fields from buffer to current frame */
void mea8000_device::decode_frame()
{
	int fd = (m_buf[3] >> 5) & 3; /* 0=8ms, 1=16ms, 2=32ms, 3=64ms */
	int pi = pi_table[m_buf[3] & 0x1f] << fd;
	m_noise = (m_buf[3] & 0x1f) == 16;
	m_pitch = m_last_pitch + pi;
	m_f[0].bw = bw_table[m_buf[0] >> 6];
	m_f[1].bw = bw_table[(m_buf[0] >> 4) & 3];
	m_f[2].bw = bw_table[(m_buf[0] >> 2) & 3];
	m_f[3].bw = bw_table[m_buf[0] & 3];
	m_f[3].fm = fm4_table[0];
	m_f[2].fm = fm3_table[m_buf[1] >> 5];
	m_f[1].fm = fm2_table[m_buf[1] & 0x1f];
	m_f[0].fm = fm1_table[m_buf[2] >> 3];
	m_ampl = ampl_table[((m_buf[2] & 7) << 1) | (m_buf[3] >> 7)];
	m_framelog = fd + 6 /* 64 samples / ms */ + 3;
	m_framelength = 1 << m_framelog;
	m_bufpos = 0;
#ifdef FLOAT_MODE
	LOG(("mea800_decode_frame: pitch=%i noise=%i  fm1=%gHz bw1=%gHz  fm2=%gHz bw2=%gHz  fm3=%gHz bw3=%gHz  fm4=%gHz bw4=%gHz  ampl=%g fd=%ims\n",
			m_pitch, m_noise,
			m_f[0].fm, m_f[0].bw, m_f[1].fm, m_f[1].bw,
			m_f[2].fm, m_f[2].bw, m_f[3].fm, m_f[3].bw,
			m_ampl/1000., 8 << fd));
#else
	LOG(("mea800_decode_frame: pitch=%i noise=%i  fm1=%iHz bw1=%iHz  fm2=%iHz bw2=%iHz  fm3=%iHz bw3=%iHz  fm4=%iHz bw4=%iHz  ampl=%g fd=%ims\n",
			m_pitch, m_noise,
			m_f[0].fm, m_f[0].bw, m_f[1].fm, m_f[1].bw,
			m_f[2].fm, m_f[2].bw, m_f[3].fm, m_f[3].bw,
			m_ampl/1000., 8 << fd));
#endif
}



void mea8000_device::start_frame()
{
	/* enter or stay in active mode */
	m_timer->reset(SAMPLING);
	m_framepos = 0;
}



void mea8000_device::stop_frame()
{
	/* enter stop mode */
	m_timer->reset();
	m_state = MEA8000_STOPPED;
	m_dac->write_signed16(0x8000);
}



/* next sample in frame, sampling at 64 kHz */
TIMER_CALLBACK_MEMBER( mea8000_device::timer_expire )
{
	int pos = m_framepos % SUPERSAMPLING;

	if (!pos)
	{
		/* sample is really computed only every 8-th time */
		m_lastsample = m_sample;
		m_sample = compute_sample();
		m_dac->write_signed16(0x8000 + m_lastsample);
	}
	else
	{
		/* other samples are simply interpolated */
		int sample = m_lastsample + ((pos * (m_sample-m_lastsample)) / SUPERSAMPLING);
		m_dac->write_signed16(0x8000 + sample);
	}

	m_framepos++;
	if (m_framepos >= m_framelength)
	{
		shift_frame();
		/* end of frame */
		if (m_bufpos == 4)
		{
			/* we have a successor */
			LOG(("%f mea8000_timer_expire: new frame\n", machine().time().as_double()));
			decode_frame();
			start_frame();
		}
		else if (m_cont)
		{
			/* repeat mode */
			LOG(("%f mea8000_timer_expire: repeat frame\n", machine().time().as_double()));
			start_frame();
		}
		/* slow stop */
		else if (m_state == MEA8000_STARTED)
		{
			m_ampl = 0;
			LOG(("%f mea8000_timer_expire: fade frame\n", machine().time().as_double()));
			start_frame();
			m_state = MEA8000_SLOWING;
		}
		else if (m_state == MEA8000_SLOWING)
		{
			LOG(("%f mea8000_timer_expire: stop frame\n", machine().time().as_double()));
			stop_frame();
		}
		update_req();
	}
	else
	{
		/* continue frame */
		m_timer->reset(SAMPLING);
	}
}


/************************** CPU interface ****************************/


READ8_MEMBER( mea8000_device::read )
{
	switch (offset)
	{
	case 0: /* status register */
	case 1:
		/* ready to accept next frame */
#if 0
		LOG(("%s %f: mea8000_r ready=%i\n", machine().describe_context(), machine().time().as_double(), accept_byte()));
#endif
		return accept_byte() << 7;

	default:
		logerror("%s mea8000_r invalid read offset %i\n", machine().describe_context(), offset);
	}
	return 0;
}

WRITE8_MEMBER( mea8000_device::write )
{
	switch (offset)
	{
	case 0: /* data register */
		if (m_state == MEA8000_STOPPED)
		{
			/* got pitch byte before first frame */
			m_pitch = 2 * data;
			LOG(("%s %f: mea8000_w pitch %i\n", machine().describe_context(), machine().time().as_double(), m_pitch));
			m_state = MEA8000_WAIT_FIRST;
			m_bufpos = 0;
		}
		else if (m_bufpos == 4)
		{
			/* overflow */
			LOG(("%s %f: mea8000_w data overflow %02X\n", machine().describe_context(), machine().time().as_double(), data));
		}
		else
		{
			/* enqueue frame byte */
			LOG(("%s %f: mea8000_w data %02X in frame pos %i\n", machine().describe_context(), machine().time().as_double(),
					data, m_bufpos));
			m_buf[m_bufpos] = data;
			m_bufpos++;
			if (m_bufpos == 4 && m_state == MEA8000_WAIT_FIRST)
			{
				/* fade-in first frame */
				int old_pitch = m_pitch;
				m_last_pitch = old_pitch;
				decode_frame();
				shift_frame();
				m_last_pitch = old_pitch;
				m_ampl = 0;
				start_frame();
				m_state = MEA8000_STARTED;
			}
		}
		update_req();
		break;

	case 1: /* command register */
	{
		int stop = BIT(data, 4);

		if (data & 8)
			m_cont = BIT(data, 2);

		if (data & 2)
			m_roe = BIT(data, 0);

		if (stop)
			stop_frame();

		LOG(( "%s %f: mea8000_w command %02X stop=%i cont=%i roe=%i\n",
				machine().describe_context(), machine().time().as_double(), data,
				stop, m_cont, m_roe));

		update_req();
		break;
	}

	default:
		logerror( "%s mea8000_w invalid write offset %i\n", machine().describe_context(), offset);
	}
}
