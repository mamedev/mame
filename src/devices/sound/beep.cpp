// license:BSD-3-Clause
// copyright-holders:Kevin Thacker
/***************************************************************************

    Simple beeper sound driver

    This is used for computers/systems which can only output a constant tone.
    This tone can be turned on and off.
    e.g. PCW and PCW16 computer systems
    KT - 25-Jun-2000

****************************************************************************/

#include "emu.h"
#include "beep.h"

#define BEEP_RATE (384000)


// device type definition
DEFINE_DEVICE_TYPE(BEEP, beep_device, "beep", "Beep")

beep_device::beep_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, BEEP, tag, owner, clock)
	, device_sound_interface(mconfig, *this)
	, m_stream(nullptr)
	, m_enable(0)
	, m_frequency(clock)
{
}


//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void beep_device::device_start()
{
	m_stream = stream_alloc(0, 1, BEEP_RATE);
	m_enable = 0;
	m_signal = 1.0;

	// register for savestates
	save_item(NAME(m_enable));
	save_item(NAME(m_frequency));
	save_item(NAME(m_incr));
	save_item(NAME(m_signal));
}


//-------------------------------------------------
//  sound_stream_update - handle a stream update
//-------------------------------------------------

void beep_device::sound_stream_update(sound_stream &stream, std::vector<read_stream_view> const &inputs, std::vector<write_stream_view> &outputs)
{
	auto &buffer = outputs[0];
	int16_t signal = m_signal;
	int clock = 0, rate = BEEP_RATE / 2;

	/* get progress through wave */
	int incr = m_incr;

	if (m_frequency > 0)
		clock = m_frequency;

	/* if we're not enabled, just fill with 0 */
	if (!m_enable || clock == 0)
	{
		buffer.fill(0);
		return;
	}

	/* fill in the sample */
	for (int sampindex = 0; sampindex < buffer.samples(); sampindex++)
	{
		buffer.put(sampindex, signal);
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

void beep_device::set_state(int state)
{
	/* only update if new state is not the same as old state */
	int on = (state) ? 1 : 0;
	if (m_enable == on)
		return;

	m_stream->update();
	m_enable = on;

	/* restart wave from beginning */
	m_incr = 0;
	m_signal = 1.0;
}


//-------------------------------------------------
//  setting new frequency starts from beginning
//-------------------------------------------------

void beep_device::set_clock(uint32_t frequency)
{
	if (m_frequency == frequency)
		return;

	m_stream->update();
	m_frequency = frequency;
	m_signal = 1.0;
	m_incr = 0;
}
