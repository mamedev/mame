// license:BSD-3-Clause
// copyright-holders:Nicola Salmoria
/***************************************************************************

    SNK Wave sound driver.

    This is a very simple single-voice generator with a programmable waveform.

***************************************************************************/

#include "emu.h"
#include "snkwave.h"


#define CLOCK_SHIFT 8


const device_type SNKWAVE = &device_creator<snkwave_device>;

//-------------------------------------------------
//  snkwave_device - constructor
//-------------------------------------------------

snkwave_device::snkwave_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
	: device_t(mconfig, SNKWAVE, "SNK Wave", tag, owner, clock, "snkwave", __FILE__),
		device_sound_interface(mconfig, *this),
		m_stream(nullptr),
		m_external_clock(0),
		m_sample_rate(0),
		m_frequency(0),
		m_counter(0),
		m_waveform_position(0)
{
	memset(m_waveform, 0, sizeof(m_waveform));
}

//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void snkwave_device::device_start()
{
	/* adjust internal clock */
	m_external_clock = clock();

	/* adjust output clock */
	m_sample_rate = m_external_clock >> CLOCK_SHIFT;

	/* get stream channels */
	m_stream = stream_alloc(0, 1, m_sample_rate);

	/* reset all the voices */
	m_frequency = 0;
	m_counter = 0;
	m_waveform_position = 0;

	/* register with the save state system */
	save_item(NAME(m_frequency));
	save_item(NAME(m_counter));
	save_item(NAME(m_waveform_position));
	save_pointer(NAME(m_waveform), SNKWAVE_WAVEFORM_LENGTH);
}


//-------------------------------------------------
//  sound_stream_update - handle update requests
//  for our sound stream
//-------------------------------------------------

void snkwave_device::sound_stream_update(sound_stream &stream, stream_sample_t **inputs, stream_sample_t **outputs, int samples)
{
	stream_sample_t *buffer = outputs[0];

	/* zap the contents of the buffer */
	memset(buffer, 0, samples * sizeof(*buffer));

	assert(m_counter < 0x1000);
	assert(m_frequency < 0x1000);

	/* if no sound, we're done */
	if (m_frequency == 0xfff)
		return;

	/* generate sound into buffer while updating the counter */
	while (samples-- > 0)
	{
		int loops;
		INT16 out = 0;

		loops = 1 << CLOCK_SHIFT;
		while (loops > 0)
		{
			int steps = 0x1000 - m_counter;

			if (steps <= loops)
			{
				out += m_waveform[m_waveform_position] * steps;
				m_counter = m_frequency;
				m_waveform_position = (m_waveform_position + 1) & (SNKWAVE_WAVEFORM_LENGTH-1);
				loops -= steps;
			}
			else
			{
				out += m_waveform[m_waveform_position] * loops;
				m_counter += loops;
				loops = 0;
			}
		}

		*buffer++ = out;
	}
}


/* SNK wave register map
    all registers are 6-bit
    0-1         frequency (12-bit)
    2-5         waveform (8 3-bit nibbles)
*/

WRITE8_MEMBER( snkwave_device::snkwave_w )
{
	m_stream->update();

	// all registers are 6-bit
	data &= 0x3f;

	if (offset == 0)
		m_frequency = (m_frequency & 0x03f) | (data << 6);
	else if (offset == 1)
		m_frequency = (m_frequency & 0xfc0) | data;
	else if (offset <= 5)
		update_waveform(offset - 2, data);
}


/* update the decoded waveform data */
/* The programmable waveform consists of 8 3-bit nibbles.
   The waveform goes to a 4-bit DAC and is played alternatingly forwards and
   backwards.
   When going forwards, bit 3 is 1. When going backwards, it's 0.
   So the sequence 01234567 will play as
   89ABCDEF76543210
*/
void snkwave_device::update_waveform(unsigned int offset, UINT8 data)
{
	assert(offset < SNKWAVE_WAVEFORM_LENGTH/4);

	m_waveform[offset * 2]     = ((data & 0x38) >> 3) << (12-CLOCK_SHIFT);
	m_waveform[offset * 2 + 1] = ((data & 0x07) >> 0) << (12-CLOCK_SHIFT);
	m_waveform[SNKWAVE_WAVEFORM_LENGTH-2 - offset * 2] = ~m_waveform[offset * 2 + 1];
	m_waveform[SNKWAVE_WAVEFORM_LENGTH-1 - offset * 2] = ~m_waveform[offset * 2];
}
