// license:BSD-3-Clause
// copyright-holders:Erwin Jansen
/**********************************************************************

    Philips P2000T Mini Digital Cassette Recorder  emulation

**********************************************************************/

#include "p2000t_mdcr.h"
#include "emu.h"
#include "formats/p2000t_cas.h"

DEFINE_DEVICE_TYPE(MDCR, mdcr_device, "mdcr", "Philips Mini DCR")

READ_LINE_MEMBER(mdcr_device::rdc)
{
	// According to mdcr spec there is cross talk on the wires when writing,
	// hence the clock signal is always false when writing.
	if (m_recording)
		return false;

	return m_fwd ? m_rdc : m_rda;
}

READ_LINE_MEMBER(mdcr_device::rda)
{
	return m_fwd ? m_rda : m_rdc;
}

READ_LINE_MEMBER(mdcr_device::bet)
{
	return tape_start_or_end();
}

READ_LINE_MEMBER(mdcr_device::cip)
{
	return true;
}

READ_LINE_MEMBER(mdcr_device::wen)
{
	return true;
}

WRITE_LINE_MEMBER(mdcr_device::rev)
{
	m_rev = state;
	if (m_rev)
	{
		rewind();
	}

	if (!m_rev && !m_fwd)
	{
		stop();
	}
}

WRITE_LINE_MEMBER(mdcr_device::fwd)
{
	m_fwd = state;
	if (m_fwd)
	{
		forward();
	}

	if (!m_rev && !m_fwd)
	{
		stop();
	}
}

WRITE_LINE_MEMBER(mdcr_device::wda)
{
	m_wda = state;
}

WRITE_LINE_MEMBER(mdcr_device::wdc)
{
	if (state)
	{
		write_bit(m_wda);
	};
}

void mdcr_device::device_add_mconfig(machine_config &config)
{
	CASSETTE(config, m_cassette);
	m_cassette->set_default_state(CASSETTE_STOPPED | CASSETTE_MOTOR_DISABLED |
	                              CASSETTE_SPEAKER_MUTED);
	m_cassette->set_interface("p2000_cass");
	m_cassette->set_formats(p2000t_cassette_formats);
}

mdcr_device::mdcr_device(machine_config const &mconfig, char const *tag, device_t *owner, uint32_t clock)
: device_t(mconfig, MDCR, tag, owner, clock)
, m_cassette(*this, "cassette")
, m_read_timer(nullptr)
{
}

void mdcr_device::device_start()
{
	m_read_timer = timer_alloc();
	m_read_timer->adjust(attotime::from_hz(44100), 0, attotime::from_hz(44100));
}

void mdcr_device::device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr)
{
	if (!m_recording && m_cassette->motor_on())
	{
		// Account for moving backwards.
		auto delay = abs(m_cassette->get_position() - m_last_tape_time);

		// Decode the signal using the fake phase decode circuit
		bool newBit = m_phase_decoder.signal((m_cassette->input() > +0.04), delay);
		if (newBit)
		{
			// Flip rdc
			m_rdc = !m_rdc;
			m_rda = m_phase_decoder.pull_bit();
		}
	}
	m_last_tape_time = m_cassette->get_position();
}

void mdcr_device::write_bit(bool bit)
{
	m_recording = true;
	m_cassette->change_state(CASSETTE_RECORD, CASSETTE_MASK_UISTATE);
	m_cassette->output(bit ? +1.0 : -1.0);
	m_phase_decoder.reset();
}

void mdcr_device::rewind()
{
	m_fwd       = false;
	m_recording = false;
	m_cassette->set_motor(true);
	m_cassette->change_state(CASSETTE_PLAY, CASSETTE_MASK_UISTATE);
	m_cassette->go_reverse();
}

void mdcr_device::forward()
{
	// A pulse of 1us < T < 20 usec should reset the phase decoder.
	// See mdcr spec for details.
	constexpr double RESET_PULSE_TIMING = 2.00e-05;
	auto now                            = machine().time().as_double();
	auto pulse_delay                    = now - m_fwd_pulse_time;
	m_fwd_pulse_time                    = now;

	if (pulse_delay < RESET_PULSE_TIMING)
	{
		m_phase_decoder.reset();
	}

	m_fwd = true;
	m_cassette->set_motor(true);
	m_cassette->change_state(m_recording ? CASSETTE_RECORD : CASSETTE_PLAY,
	CASSETTE_MASK_UISTATE);
	m_cassette->go_forward();
}

void mdcr_device::stop()
{
	m_cassette->change_state(CASSETTE_PLAY, CASSETTE_MASK_UISTATE);
	m_cassette->set_motor(false);
}

bool mdcr_device::tape_start_or_end()
{
	auto pos = m_cassette->get_position();
	return m_cassette->motor_on() &&
	       (pos <= 0 || pos >= m_cassette->get_length());
}

void p2000_mdcr_devices(device_slot_interface &device)
{
	device.option_add("mdcr", MDCR);
}

//
// phase_decoder
//

phase_decoder::phase_decoder(double tolerance)
: m_tolerance(tolerance)
{
	reset();
}

bool phase_decoder::read_clock()
{
	if (m_bit_queue.empty())
		return false;

	// Note that this can be 0 when we read the first ever bit.
	timed_bit bit = m_bit_queue.front();
	return m_current_clock < (bit.second * 0.75);
}

bool phase_decoder::read_data()
{
	if (m_bit_queue.empty())
		return false;

	return m_bit_queue.front().first;
};

bool phase_decoder::pull_bit()
{
	if (m_bit_queue.empty())
	{
		return false;
	}
	auto bit = m_bit_queue.front().first;
	m_bit_queue.pop();
	return bit;
}

bool phase_decoder::signal(bool state, double delay)
{
	m_current_clock += delay;
	if (state == m_last_signal)
	{
		if (m_needs_sync == 0 && m_current_clock > m_clock_period &&
		    !within_tolerance(m_current_clock, m_clock_period))
		{
			// We might be at the last bit in a sequence, meaning we
			// are only getting the reference signal for a while.
			// so we produce one last clock signal.
			reset();
			return true;
		}
		return false;
	}

	// A transition happened!
	m_last_signal = state;
	if (m_needs_sync > 0)
	{
		// We have not yet determined our clock period.
		return sync_signal(state);
	}

	// We are within bounds of the current clock
	if (within_tolerance(m_current_clock, m_clock_period))
	{
		add_bit(state);
		return true;
	};

	// We went out of sync, our clock is wayyy out of bounds.
	if (m_current_clock > m_clock_period)
	{
		reset();
	}

	// We are likely halfway in our clock signal..
	return false;
};

void phase_decoder::reset()
{
	m_last_signal      = false;
	m_last_signal_time = {};
	m_current_clock    = {};
	m_clock_period     = {};
	m_needs_sync       = SYNCBITS;
}

void phase_decoder::add_bit(bool bit)
{
	m_bit_queue.push(std::make_pair(bit, m_clock_period));
	if (m_bit_queue.size() > QUEUE_DELAY)
	{
		m_bit_queue.pop();
	}
	m_current_clock = {};
}

bool phase_decoder::sync_signal(bool state)
{
	m_needs_sync--;
	if (m_needs_sync == SYNCBITS - 1)
	{
		// First bit!
		if (state)
		{
			// We can only synchronize when we go up
			// on the first bit.
			add_bit(true);
		}
		return false;
	}
	if (m_needs_sync == SYNCBITS - 2)
	{
		static_assert(SYNCBITS >= 2, "Need at least 2 bits to synchronize!");
		// Update the clock period of the bit we just added to the queue.
		m_clock_period            = m_current_clock;
		m_bit_queue.back().second = m_clock_period;
	}
	if (!within_tolerance(m_current_clock, m_clock_period))
	{
		// Clock is way off!
		reset();
		return false;
	}

	// We've derived a clock period, we will use the average.
	auto div       = SYNCBITS - m_needs_sync;
	m_clock_period = ((div - 1) * m_clock_period + m_current_clock) / div;
	add_bit(state);
	return true;
}

// y * (1 - tolerance) < x < y * (1 + tolerance)
bool phase_decoder::within_tolerance(double x, double y)
{
	assert(m_tolerance > 0 && m_tolerance < 1);
	return (y * (1 - m_tolerance)) < x && x < (y * (1 + m_tolerance));
}
