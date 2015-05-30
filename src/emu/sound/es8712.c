// license:BSD-3-Clause
// copyright-holders:Quench
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

#define MAX_SAMPLE_CHUNK    10000


/* step size index shift table */
static const int index_shift[8] = { -1, -1, -1, -1, 2, 4, 6, 8 };

/* lookup table for the precomputed difference */
static int diff_lookup[49*16];


// device type definition
const device_type ES8712 = &device_creator<es8712_device>;


//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  es8712_device - constructor
//-------------------------------------------------

es8712_device::es8712_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
	: device_t(mconfig, ES8712, "ES8712", tag, owner, clock, "es8712", __FILE__),
		device_sound_interface(mconfig, *this),
		m_rom(*this, DEVICE_SELF),
		m_playing(0),
		m_base_offset(0),
		m_sample(0),
		m_count(0),
		m_signal(0),
		m_step(0),
		m_start(0),
		m_end(0),
		m_repeat(0),
		m_bank_offset(0),
		m_stream(NULL)
{
}


//-------------------------------------------------
//  device_start - start emulation of an ES8712 chip
//-------------------------------------------------

void es8712_device::device_start()
{
	compute_tables();

	m_start = 0;
	m_end = 0;
	m_repeat = 0;

	m_bank_offset = 0;

	/* generate the name and create the stream */
	m_stream = stream_alloc(0, 1, clock());

	/* initialize the rest of the structure */
	m_signal = -2;

	es8712_state_save_register();
}


//-------------------------------------------------
//  device_reset - stop emulation of an ES8712-compatible chip
//-------------------------------------------------

void es8712_device::device_reset()
{
	if (m_playing)
	{
		/* update the stream, then turn it off */
		m_stream->update();
		m_playing = 0;
		m_repeat = 0;
	}
}


//-------------------------------------------------
//  sound_stream_update - update the sound chip so that it is in sync with CPU execution
//-------------------------------------------------

void es8712_device::sound_stream_update(sound_stream &stream, stream_sample_t **inputs, stream_sample_t **outputs, int samples)
{
	stream_sample_t *buffer = outputs[0];

	/* generate them into our buffer */
	generate_adpcm(buffer, samples);
}


//-------------------------------------------------
//   compute_tables -- compute the difference tables
//-------------------------------------------------

void es8712_device::compute_tables()
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


//-------------------------------------------------
//  generate_adpcm -- general ADPCM decoding routine
//-------------------------------------------------

void es8712_device::generate_adpcm(stream_sample_t *buffer, int samples)
{
	/* if this chip is active */
	if (m_playing)
	{
		UINT8 *base = &m_rom[m_bank_offset + m_base_offset];
		int sample = m_sample;
		int signal = m_signal;
		int count = m_count;
		int step = m_step;
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
				if (m_repeat)
				{
					sample = 0;
					signal = -2;
					step = 0;
				}
				else
				{
					m_playing = 0;
					break;
				}
			}
		}

		/* update the parameters */
		m_sample = sample;
		m_signal = signal;
		m_step = step;
	}

	/* fill the rest with silence */
	while (samples--)
		*buffer++ = 0;
}



//-------------------------------------------------
//   state save support for MAME
//-------------------------------------------------

void es8712_device::es8712_state_save_register()
{
	save_item(NAME(m_bank_offset));

	save_item(NAME(m_playing));
	save_item(NAME(m_sample));
	save_item(NAME(m_count));
	save_item(NAME(m_signal));
	save_item(NAME(m_step));

	save_item(NAME(m_base_offset));

	save_item(NAME(m_start));
	save_item(NAME(m_end));
	save_item(NAME(m_repeat));
}




//-------------------------------------------------
//   es8712_set_bank_base -- set the base of the bank on a given chip
//-------------------------------------------------

void es8712_device::set_bank_base(int base)
{
	m_stream->update();
	m_bank_offset = base;
}


//-------------------------------------------------
//  es8712_set_frequency -- dynamically adjusts the frequency of a given ADPCM chip
//-------------------------------------------------

void es8712_device::set_frequency(int frequency)
{
	/* update the stream and set the new base */
	m_stream->update();
	m_stream->set_sample_rate(frequency);
}


//-------------------------------------------------
//  play -- Begin playing the addressed sample
//-------------------------------------------------

void es8712_device::play()
{
	if (m_start < m_end)
	{
		if (!m_playing)
		{
			m_playing = 1;
			m_base_offset = m_start;
			m_sample = 0;
			m_count = 2 * (m_end - m_start + 1);
			m_repeat = 0;//1;

			/* also reset the ADPCM parameters */
			m_signal = -2;
			m_step = 0;
		}
	}
	/* invalid samples go here */
	else
	{
		logerror("ES871295:'%s' requested to play invalid sample range %06x-%06x\n", tag(), m_start, m_end);

		if (m_playing)
		{
			/* update the stream */
			m_stream->update();
			m_playing = 0;
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

WRITE8_MEMBER( es8712_device::es8712_w )
{
	switch (offset)
	{
		case 00:    m_start &= 0x000fff00;
					m_start |= ((data & 0xff) <<  0); break;
		case 01:    m_start &= 0x000f00ff;
					m_start |= ((data & 0xff) <<  8); break;
		case 02:    m_start &= 0x0000ffff;
					m_start |= ((data & 0x0f) << 16); break;
		case 03:    m_end   &= 0x000fff00;
					m_end   |= ((data & 0xff) <<  0); break;
		case 04:    m_end   &= 0x000f00ff;
					m_end   |= ((data & 0xff) <<  8); break;
		case 05:    m_end   &= 0x0000ffff;
					m_end   |= ((data & 0x0f) << 16); break;
		case 06:
					play(); break;
		default:    break;
	}
	m_start &= 0xfffff; m_end &= 0xfffff;
}
