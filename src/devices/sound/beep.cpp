// license:BSD-3-Clause
// copyright-holders:Kevin Thacker
/***************************************************************************

    Simple beeper sound driver

    This is used for computers/systems which can only output a constant tone.
    This tone can be turned on and off.
    e.g. PCW and PCW16 computer systems

****************************************************************************/

#include "emu.h"
#include "beep.h"


// device type definition
DEFINE_DEVICE_TYPE(BEEP, beep_device, "beep", "Beep")

beep_device::beep_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, BEEP, tag, owner, clock)
	, device_sound_interface(mconfig, *this)
	, m_stream(nullptr)
	, m_enable(false)
	, m_frequency(clock)
{
}


//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void beep_device::device_start()
{
	// large stream buffer to favour emu/sound.cpp resample quality
	m_stream = stream_alloc(0, 1, 48000 * 32);

	m_enable = false;
	m_incr = 0;
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

	// if we're not enabled, just fill with 0
	if (!m_enable || m_frequency == 0)
	{
		buffer.fill(0);
		return;
	}

	// fill in the sample
	for (int sampindex = 0; sampindex < buffer.samples(); sampindex++)
	{
		m_incr -= m_frequency;
		while (m_incr < 0)
		{
			m_incr += stream.sample_rate() / 2;
			m_signal = -m_signal;
		}

		buffer.put(sampindex, m_signal);
	}
}


//-------------------------------------------------
//  changing state to on from off will restart tone
//-------------------------------------------------

void beep_device::set_state(int state)
{
	// only update if new state is not the same as old state
	if (m_enable == bool(state))
		return;

	m_stream->update();
	m_enable = bool(state);

	// restart wave from beginning
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

	// restart wave from beginning
	m_incr = 0;
	m_signal = 1.0;
}
