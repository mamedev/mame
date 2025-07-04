// license:BSD-3-Clause
// copyright-holders:Olivier Galibert
/*
   GI SP0250 digital LPC sound synthesizer

   By O. Galibert.

   Unimplemented:
   - Direct Data test mode (pin 7)
*/

#include "emu.h"
#include "sp0250.h"

//
// Input clock is divided by 2 to make ROMCLOCK.
// Output is via pulse-width modulation (PWM) over the course of 39 ROMCLOCKs.
// 4 PWM periods per frame.
//
static constexpr int PWM_CLOCKS = 39;


DEFINE_DEVICE_TYPE(SP0250, sp0250_device, "sp0250", "GI SP0250 LPC")

sp0250_device::sp0250_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	device_t(mconfig, SP0250, tag, owner, clock),
	device_sound_interface(mconfig, *this),
	m_timer(nullptr),
	m_pwm_mode(false),
	m_pwm_index(PWM_CLOCKS),
	m_pwm_count(0),
	m_pwm_counts(0),
	m_voiced(0),
	m_amp(0),
	m_lfsr(0x7fff),
	m_pitch(0),
	m_pcount(0),
	m_repeat(0),
	m_rcount(0),
	m_fifo_pos(0),
	m_stream(nullptr),
	m_drq(*this)
{
	for (auto & elem : m_fifo)
	{
		elem = 0;
	}

	for (auto & elem : m_filter)
	{
		elem.F = 0;
		elem.B = 0;
		elem.z1 = 0;
		elem.z2 = 0;
	}
}

//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void sp0250_device::device_start()
{
	// output PWM data at the ROMCLOCK frequency
	int sample_rate = clock() / 2;
	int frame_rate = sample_rate / (4 * PWM_CLOCKS);
	m_stream = stream_alloc(0, 1, m_pwm_mode ? sample_rate : frame_rate);

	// if a DRQ callback is offered, run a timer at the frame rate
	// to ensure the DRQ gets picked up in a timely manner
	if (!m_drq.isunset())
	{
		m_drq(ASSERT_LINE);
		attotime period = attotime::from_hz(frame_rate);
		m_timer = timer_alloc(FUNC(sp0250_device::delayed_stream_update), this);
		m_timer->adjust(period, 0, period);
	}

	// PWM state
	save_item(NAME(m_pwm_index));
	save_item(NAME(m_pwm_count));
	save_item(NAME(m_pwm_counts));

	// LPC state
	save_item(NAME(m_voiced));
	save_item(NAME(m_amp));
	save_item(NAME(m_lfsr));
	save_item(NAME(m_pitch));
	save_item(NAME(m_pcount));
	save_item(NAME(m_repeat));
	save_item(NAME(m_rcount));

	save_item(STRUCT_MEMBER(m_filter, F));
	save_item(STRUCT_MEMBER(m_filter, B));
	save_item(STRUCT_MEMBER(m_filter, z1));
	save_item(STRUCT_MEMBER(m_filter, z2));

	// FIFO state
	save_item(NAME(m_fifo));
	save_item(NAME(m_fifo_pos));
}

//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void sp0250_device::device_reset()
{
	load_values();
}

static uint16_t sp0250_ga(uint8_t v)
{
	return (v & 0x1f) << (v>>5);
}

static int16_t sp0250_gc(uint8_t v)
{
	// Internal ROM to the chip, cf. manual
	static const uint16_t coefs[128] =
	{
		  0,   9,  17,  25,  33,  41,  49,  57,  65,  73,  81,  89,  97, 105, 113, 121,
		129, 137, 145, 153, 161, 169, 177, 185, 193, 201, 203, 217, 225, 233, 241, 249,
		257, 265, 273, 281, 289, 297, 301, 305, 309, 313, 317, 321, 325, 329, 333, 337,
		341, 345, 349, 353, 357, 361, 365, 369, 373, 377, 381, 385, 389, 393, 397, 401,
		405, 409, 413, 417, 421, 425, 427, 429, 431, 433, 435, 437, 439, 441, 443, 445,
		447, 449, 451, 453, 455, 457, 459, 461, 463, 465, 467, 469, 471, 473, 475, 477,
		479, 481, 482, 483, 484, 485, 486, 487, 488, 489, 490, 491, 492, 493, 494, 495,
		496, 497, 498, 499, 500, 501, 502, 503, 504, 505, 506, 507, 508, 509, 510, 511
	};
	int16_t res = coefs[v & 0x7f];

	if (!(v & 0x80))
		res = -res;
	return res;
}

void sp0250_device::load_values()
{
	m_filter[0].B = sp0250_gc(m_fifo[ 0]);
	m_filter[0].F = sp0250_gc(m_fifo[ 1]);
	m_amp         = sp0250_ga(m_fifo[ 2]);
	m_filter[1].B = sp0250_gc(m_fifo[ 3]);
	m_filter[1].F = sp0250_gc(m_fifo[ 4]);
	m_pitch       =           m_fifo[ 5];
	m_filter[2].B = sp0250_gc(m_fifo[ 6]);
	m_filter[2].F = sp0250_gc(m_fifo[ 7]);
	m_repeat      =           m_fifo[ 8] & 0x3f;
	m_voiced      =           m_fifo[ 8] & 0x40;
	m_filter[3].B = sp0250_gc(m_fifo[ 9]);
	m_filter[3].F = sp0250_gc(m_fifo[10]);
	m_filter[4].B = sp0250_gc(m_fifo[11]);
	m_filter[4].F = sp0250_gc(m_fifo[12]);
	m_filter[5].B = sp0250_gc(m_fifo[13]);
	m_filter[5].F = sp0250_gc(m_fifo[14]);
	m_fifo_pos = 0;
	m_drq(ASSERT_LINE);
	m_pcount = 0;
	m_rcount = 0;

	for (int f = 0; f < 6; f++)
		m_filter[f].reset();
}

void sp0250_device::write(uint8_t data)
{
	m_stream->update();
	if (m_fifo_pos != 15)
	{
		m_fifo[m_fifo_pos++] = data;
		if (m_fifo_pos == 15)
			m_drq(CLEAR_LINE);
	}
	else
		logerror("%s: overflow SP0250 FIFO\n", machine().describe_context());
}


int sp0250_device::drq_r()
{
	m_stream->update();
	return (m_fifo_pos == 15) ? CLEAR_LINE : ASSERT_LINE;
}

int8_t sp0250_device::next()
{
	if (m_rcount >= m_repeat)
	{
		if (m_fifo_pos == 15)
			load_values();
		else
		{
			// According to http://www.cpcwiki.eu/index.php/SP0256_Measured_Timings
			// the SP0250 executes "NOPs" with a repeat count of 1 and unchanged
			// pitch while waiting for input
			m_repeat = 1;
			m_pcount = 0;
			m_rcount = 0;
		}
	}

	// 15-bit LFSR algorithm verified by dump from actual hardware
	// clocks every cycle regardless of voiced/unvoiced setting
	m_lfsr ^= (m_lfsr ^ (m_lfsr >> 1)) << 15;
	m_lfsr >>= 1;

	int16_t z0;
	if (m_voiced)
		z0 = (m_pcount == 0) ? m_amp : 0;
	else
		z0 = (m_lfsr & 1) ? m_amp : -m_amp;

	for (int f = 0; f < 6; f++)
		z0 = m_filter[f].apply(z0);

	// maximum amp value is effectively 13 bits
	// reduce to 7 bits; due to filter effects it
	// may occasionally clip
	int dac = z0 >> 6;
	if (dac < -64)
		dac = -64;
	if (dac > 63)
		dac = 63;

	// PWM is divided into 4x 5-bit sections; the lower
	// bits of the original 7-bit value are added to only
	// some of the pulses in the following pattern:
	//
	//    DAC -64 -> 1,1,1,1
	//    DAC -63 -> 2,1,1,1
	//    DAC -62 -> 2,1,2,1
	//    DAC -61 -> 2,2,2,1
	//    DAC -60 -> 2,2,2,2
	//    ...
	//    DAC  -1 -> 17,17,17,16
	//    DAC   0 -> 17,17,17,17
	//    DAC   1 -> 18,17,17,17
	//    ...
	//    DAC  60 -> 32,32,32,32
	//    DAC  61 -> 33,32,32,32
	//    DAC  62 -> 33,32,33,32
	//    DAC  63 -> 33,33,33,32
	m_pwm_counts = (((dac + 68 + 3) >> 2) << 0) +
				   (((dac + 68 + 1) >> 2) << 8) +
				   (((dac + 68 + 2) >> 2) << 16) +
				   (((dac + 68 + 0) >> 2) << 24);

	if (m_pcount++ == m_pitch)
	{
		m_pcount = 0;
		m_rcount++;
	}
	return dac;
}

//-------------------------------------------------
//  sound_stream_update - handle a stream update
//-------------------------------------------------

void sp0250_device::sound_stream_update(sound_stream &stream)
{
	if (!m_pwm_mode)
	{
		for (int sampindex = 0; sampindex < stream.samples(); sampindex++)
			stream.put_int(0, sampindex, next(), 128);
	}
	else
	{
		for (int sampindex = 0; sampindex < stream.samples(); )
		{
			// see where we're at in the current PWM cycle
			if (m_pwm_index >= PWM_CLOCKS)
			{
				m_pwm_index = 0;
				if (m_pwm_counts == 0)
					next();
				m_pwm_count = m_pwm_counts & 0xff;
				m_pwm_counts >>= 8;
			}

			// determine the value to fill and the number of samples remaining
			// until it changes
			sound_stream::sample_t value;
			int remaining;
			if (m_pwm_index < m_pwm_count)
			{
				value = 1.0;
				remaining = m_pwm_count - m_pwm_index;
			}
			else
			{
				value = 0.0;
				remaining = PWM_CLOCKS - m_pwm_index;
			}

			// clamp to the number of samples requested and advance the counters
			if (remaining > stream.samples() - sampindex)
				remaining = stream.samples() - sampindex;
			m_pwm_index += remaining;

			// fill the output
			while (remaining-- != 0)
				stream.put(0, sampindex++, value);
		}
	}
}
