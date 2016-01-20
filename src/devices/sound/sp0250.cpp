// license:BSD-3-Clause
// copyright-holders:Olivier Galibert
/*
   GI SP0250 digital LPC sound synthesizer

   By O. Galibert.

   Unknown:
   - Exact clock divider
   - Exact noise algorithm
   - Exact noise pitch (probably ok)
   - 7 bits output mapping
   - Whether the pitch starts counting from 0 or 1

   Unimplemented:
   - Direct Data test mode (pin 7)

   Sound quite reasonably already though.
*/

#include "emu.h"
#include "sp0250.h"

/*
standard external clock is 3.12MHz
the chip provides a 445.7kHz output clock, which is = 3.12MHz / 7
therefore I expect the clock divider to be a multiple of 7
Also there are 6 cascading filter stages so I expect the divider to be a multiple of 6.

The SP0250 manual states that the original speech is sampled at 10kHz, so the divider
should be 312, but 312 = 39*8 so it doesn't look right because a divider by 39 is unlikely.

7*6*8 = 336 gives a 9.286kHz sample rate and matches the samples from the Sega boards.
*/
#define CLOCK_DIVIDER (7*6*8)

const device_type SP0250 = &device_creator<sp0250_device>;

sp0250_device::sp0250_device(const machine_config &mconfig, std::string tag, device_t *owner, UINT32 clock)
	: device_t(mconfig, SP0250, "SP0250", tag, owner, clock, "sp0250", __FILE__),
		device_sound_interface(mconfig, *this),
		m_amp(0),
		m_pitch(0),
		m_repeat(0),
		m_pcount(0),
		m_rcount(0),
		m_playing(0),
		m_RNG(0),
		m_stream(nullptr),
		m_voiced(0),
		m_fifo_pos(0),
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
	m_RNG = 1;
	m_drq.resolve_safe();
	if (!m_drq.isnull())
	{
		m_drq( ASSERT_LINE);
		machine().scheduler().timer_pulse(attotime::from_hz(clock()) * CLOCK_DIVIDER, timer_expired_delegate(FUNC(sp0250_device::timer_tick), this));
	}

	m_stream = machine().sound().stream_alloc(*this, 0, 1, clock() / CLOCK_DIVIDER);

	save_item(NAME(m_amp));
	save_item(NAME(m_pitch));
	save_item(NAME(m_repeat));
	save_item(NAME(m_pcount));
	save_item(NAME(m_rcount));
	save_item(NAME(m_playing));
	save_item(NAME(m_RNG));
	save_item(NAME(m_voiced));
	save_item(NAME(m_fifo));
	save_item(NAME(m_fifo_pos));
}

static UINT16 sp0250_ga(UINT8 v)
{
	return (v & 0x1f) << (v>>5);
}

static INT16 sp0250_gc(UINT8 v)
{
	// Internal ROM to the chip, cf. manual
	static const UINT16 coefs[128] =
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
	INT16 res = coefs[v & 0x7f];

	if (!(v & 0x80))
		res = -res;
	return res;
}

void sp0250_device::load_values()
{
	int f;


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

	for (f = 0; f < 6; f++)
		m_filter[f].z1 = m_filter[f].z2 = 0;

	m_playing = 1;
}

TIMER_CALLBACK_MEMBER( sp0250_device::timer_tick )
{
	m_stream->update();
}

WRITE8_MEMBER( sp0250_device::write )
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


UINT8 sp0250_device::drq_r()
{
	m_stream->update();
	return (m_fifo_pos == 15) ? CLEAR_LINE : ASSERT_LINE;
}

//-------------------------------------------------
//  sound_stream_update - handle a stream update
//-------------------------------------------------

void sp0250_device::sound_stream_update(sound_stream &stream, stream_sample_t **inputs, stream_sample_t **outputs, int samples)
{
	stream_sample_t *output = outputs[0];
	int i;
	for (i = 0; i < samples; i++)
	{
		if (m_playing)
		{
			INT16 z0;
			int f;

			if (m_voiced)
			{
				if(!m_pcount)
					z0 = m_amp;
				else
					z0 = 0;
			}
			else
			{
				// Borrowing the ay noise generation LFSR
				if(m_RNG & 1)
				{
					z0 = m_amp;
					m_RNG ^= 0x24000;
				}
				else
					z0 = -m_amp;

				m_RNG >>= 1;
			}

			for (f = 0; f < 6; f++)
			{
				z0 += ((m_filter[f].z1 * m_filter[f].F) >> 8)
					+ ((m_filter[f].z2 * m_filter[f].B) >> 9);
				m_filter[f].z2 = m_filter[f].z1;
				m_filter[f].z1 = z0;
			}

			// Physical resolution is only 7 bits, but heh

			// max amplitude is 0x0f80 so we have margin to push up the output
			output[i] = z0 << 3;

			m_pcount++;
			if (m_pcount >= m_pitch)
			{
				m_pcount = 0;
				m_rcount++;
				if (m_rcount >= m_repeat)
					m_playing = 0;
			}
		}
		else
			output[i] = 0;

		if (!m_playing)
		{
			if(m_fifo_pos == 15)
				load_values();
		}
	}
}
