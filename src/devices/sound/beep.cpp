// license:BSD-3-Clause
// copyright-holders:Kevin Thacker
/***************************************************************************

    beep.c

    This is used for computers/systems which can only output a constant tone.
    This tone can be turned on and off.
    e.g. PCW and PCW16 computer systems
    KT - 25-Jun-2000

    Sound handler

****************************************************************************/

#include "emu.h"
#include "sound/beep.h"

#define BEEP_RATE (48000)


// device type definition
const device_type BEEP = &device_creator<beep_device>;


//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  beep_device - constructor
//-------------------------------------------------

beep_device::beep_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
	: device_t(mconfig, BEEP, "Beep", tag, owner, clock, "beep", __FILE__),
		device_sound_interface(mconfig, *this),
		m_stream(nullptr),
		m_enable(0),
		m_frequency(0),
		m_incr(0),
		m_signal(0)
{
}


//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void beep_device::device_start()
{
	m_stream = stream_alloc(0, 1, BEEP_RATE);
	m_enable = 0;
	m_frequency = 3250;
	m_incr = 0;
	m_signal = 0x07fff;
}


//-------------------------------------------------
//  sound_stream_update - handle a stream update
//-------------------------------------------------

void beep_device::sound_stream_update(sound_stream &stream, stream_sample_t **inputs, stream_sample_t **outputs, int samples)
{
	stream_sample_t *buffer = outputs[0];
	INT16 signal = m_signal;
	int clock = 0, rate = BEEP_RATE / 2;

	/* get progress through wave */
	int incr = m_incr;

	if (m_frequency > 0)
		clock = m_frequency;

	/* if we're not enabled, just fill with 0 */
	if ( !m_enable || clock == 0 )
	{
		memset( buffer, 0, samples * sizeof(*buffer) );
		return;
	}

	/* fill in the sample */
	while( samples-- > 0 )
	{
		*buffer++ = signal;
		incr -= clock;
		while( incr < 0 )
		{
			incr += rate;
			signal = -signal;
		}
	}

	/* store progress through wave */
	m_incr = incr;
	m_signal = signal;
}


//-------------------------------------------------
//  changing state to on from off will restart tone
//-------------------------------------------------

void beep_device::set_state(int on)
{
	/* only update if new state is not the same as old state */
	if (m_enable == on)
		return;

	m_stream->update();
	m_enable = on;

	/* restart wave from beginning */
	m_incr = 0;
	m_signal = 0x07fff;
}



//-------------------------------------------------
//  setting new frequency starts from beginning
//-------------------------------------------------

void beep_device::set_frequency(int frequency)
{
	if (m_frequency == frequency)
		return;

	m_stream->update();
	m_frequency = frequency;
	m_signal = 0x07fff;
	m_incr = 0;
}



//-------------------------------------------------
//  change a channel volume
//-------------------------------------------------

void beep_device::set_volume(int volume)
{
	m_stream->update();
	volume = 100 * volume / 7;
	set_output_gain(0, volume);
}
