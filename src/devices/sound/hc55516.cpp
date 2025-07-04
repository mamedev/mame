// license:BSD-3-Clause
// copyright-holders:Aaron Giles, Jonathan Gevaryahu
// thanks-to:Zonn Moore
/*****************************************************************************

    Continuously Variable Slope Demodulator standalone chip emulator:
    Harris HC-55516 (sometimes labeled HCI-55516 or HC1-55516)
    Harris HC-55532 (sometimes labeled HCI-55532 or HC1-55532) [preliminary]
    Motorola MC-3417/MC-34115
    Motorola MC-3418

    TODO:
    - see .h file
    - research HC-55536 and HC-55564 differences vs HC-55516 (better auto-zeroing,
      and removal of the encoder offset compensation DAC?)
    - /src/mame/exidy/exidy440_a.cpp has its own internal implementation of the
      MC3417 and MC3418, it should be using this file instead
    - MC3417 interpolation slope being determined by the number of samples in
      the current stream slice doesn't make sense

    BTANB:
    - outputs a low-volume but very high-pitched background tone when /FZ is
      active and the hardware doesn't have a low-pass filter

*****************************************************************************/

#include "emu.h"
#include "hc55516.h"

// fixed samplerate of 192khz
#define SAMPLE_RATE             (48000 * 4)

#define INTEGRATOR_LEAK_TC      0.001
#define FILTER_DECAY_TC         0.004
#define FILTER_CHARGE_TC        0.004
#define FILTER_MIN              0.0416
#define FILTER_MAX              1.0954
#define SAMPLE_GAIN             (10000.0 / 32768.0)


//#####################################
//                 COMMON
//#####################################
cvsd_device_base::cvsd_device_base(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock, bool active_clock_edge, uint8_t shiftreg_mask)
	: device_t(mconfig, type, tag, owner, clock)
	, device_sound_interface(mconfig, *this)
	, m_clock_state_push_cb(*this)
	, m_digin_pull_cb(*this, 1)
	, m_digout_push_cb(*this)
	, m_active_clock_edge(active_clock_edge)
	, m_shiftreg_mask(shiftreg_mask)
	, m_last_clock_state(false)
	, m_buffered_bit(false)
	, m_shiftreg(0)
	, m_samples_generated(0)
{
}

//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void cvsd_device_base::device_start()
{
	// create the stream
	m_stream = stream_alloc(0, 1, SAMPLE_RATE);

	save_item(NAME(m_last_clock_state));
	save_item(NAME(m_buffered_bit));
	save_item(NAME(m_shiftreg));
	save_item(NAME(m_samples_generated));
}

//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void cvsd_device_base::device_reset()
{
	m_last_clock_state = 0;
}

//-------------------------------------------------
//  device_clock_changed - device-specific samplerate change
//-------------------------------------------------
/*void cvsd_device_base::device_clock_changed()
{
    // do nothing.
    //m_stream->set_sample_rate(clock());
}*/

int cvsd_device_base::clock_r()
{
	// prevent debugger from changing the internal state
	if (!machine().side_effects_disabled())
		m_stream->update(); // bring up to date first
	return clock_state_r();
}

void cvsd_device_base::mclock_w(int state)
{
	clock_w(state);
}

void cvsd_device_base::digin_w(int state)
{
	digit_w(state);
}

// the following encode related functions don't do anything yet, don't call them.
/*void cvsd_device_base::audio_in_w(int16_t data)
{
    assert(0);
}*/

void cvsd_device_base::dec_encq_w(int state)
{
	assert(0);
}

int cvsd_device_base::digout_r()
{
	return 0;
}

// default and stub implementations

inline bool cvsd_device_base::is_external_oscillator()
{
	return clock() != 0;
}

inline bool cvsd_device_base::is_clock_changed(bool clock_state)
{
	return ((!m_last_clock_state && clock_state) || (m_last_clock_state && !clock_state));
}

inline bool cvsd_device_base::is_active_clock_transition(bool clock_state)
{
	return ((clock_state != m_last_clock_state) && (clock_state == m_active_clock_edge));
}

inline bool cvsd_device_base::current_clock_state()
{
	// keep track of the clock state given its previous state and the number of samples produced
	// i.e. if we generated m_samples_generated samples, at a sample rate of SAMPLE_RATE, then are we on a
	// positive or negative level of a squarewave at clock() hz? SAMPLE_RATE may not be an integer multiple of clock()
	//uint64_t fractions_of_second = (((uint64_t)m_samples_generated) << 32) / SAMPLE_RATE; // 32.32 bits of seconds passed so far
	//uint32_t clock_edges_passed = (fractions_of_second * clock() * 2) >> 32
	//return bool(((((uint64_t)m_samples_generated << 32) * clock() * 2 / SAMPLE_RATE) >> 32) & 0x1);
	return bool(((uint64_t)m_samples_generated * clock() * 2 / SAMPLE_RATE) & 0x01);
}

void cvsd_device_base::digit_w(int digit)
{
	m_stream->update();
	m_buffered_bit = bool(digit);
}

void cvsd_device_base::clock_w(int state)
{
	// update the output buffer first
	m_stream->update();
	bool clock_state = bool(state);

	// only makes sense for setups with a software driven clock
	assert(!is_external_oscillator());

	// speech clock changing?
	if (is_clock_changed(clock_state))
	{
		// clear the update count
		m_samples_generated = 0;
		process_bit(m_buffered_bit, clock_state);
	}

	// update the clock
	m_last_clock_state = clock_state;
}

int cvsd_device_base::clock_state_r()
{
	// only makes sense for setups with an external oscillator
	assert(is_external_oscillator());

	m_stream->update();

	return current_clock_state();
}

void cvsd_device_base::process_bit(bool bit, bool clock_state)
{
	// stub
}

//-------------------------------------------------
//  sound_stream_update - handle a stream update
//-------------------------------------------------

void cvsd_device_base::sound_stream_update(sound_stream &stream)
{
	// Stub, just return silence
	m_samples_generated += stream.samples();
	if (m_samples_generated >= SAMPLE_RATE)
		m_samples_generated -= SAMPLE_RATE;
}


//#########################################
//                 HC55516
//#########################################
DEFINE_DEVICE_TYPE(HC55516, hc55516_device, "hc55516", "HC-55516")

hc55516_device::hc55516_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: hc55516_device(mconfig, HC55516, tag, owner, clock, 0xfc0, 6, 0xfc1, 4)
{
}

// overridable type for hc55532 etc
hc55516_device::hc55516_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock, uint32_t sylmask, int32_t sylshift, int32_t syladd, int32_t intshift)
	: cvsd_device_base(mconfig, type, tag, owner, clock, RISING, 0x7)
	, m_agc_push_cb(*this)
	, m_fzq_pull_cb(*this, 1)
	, m_sylmask(sylmask)
	, m_sylshift(sylshift)
	, m_syladd(syladd)
	, m_intshift(intshift)
	, m_sylfilter(0)
	, m_intfilter(0)
	, m_next_sample(0)
	, m_agc(true)
	, m_buffered_fzq(true)
{
}

//-------------------------------------------------
//  device_start - device-specific start
//-------------------------------------------------

void hc55516_device::device_start()
{
	cvsd_device_base::device_start();

	save_item(NAME(m_sylfilter));
	save_item(NAME(m_intfilter));
	save_item(NAME(m_next_sample));
	save_item(NAME(m_agc));
	save_item(NAME(m_buffered_fzq));
}

//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void hc55516_device::device_reset()
{
	cvsd_device_base::device_reset();

	// simulate /FZ having been held for a while
	m_sylfilter = 0x3f;
	m_intfilter = 0;
	m_agc = true;
	m_buffered_fzq = true; // assuming /FZ was just released and is now high/inactive
}

// device specific functions

void hc55516_device::fzq_w(int state)
{
	m_buffered_fzq = state;
}

int hc55516_device::agc_r()
{
	// prevent debugger from changing the internal state
	if (!machine().side_effects_disabled())
		m_stream->update(); // bring up to date first
	return m_agc;
}

void hc55516_device::process_bit(bool bit, bool clock_state)
{
	const bool frozen = (((m_intfilter >= 0x180) && !bit) || ((m_intfilter <= -0x180) && bit));
	int32_t sum;

	if (is_active_clock_transition(clock_state))
	{
		// grab the /FZ state; if the callback is present, use that, otherwise use the buffered state
		bool fzq_state = false;
		if (!m_fzq_pull_cb.isunset())
			fzq_state = m_fzq_pull_cb();
		else
			fzq_state = m_buffered_fzq;

		// if /FZ is active, the input bit is ignored and the inverse of the previous bit in the shifter is used instead
		if (!fzq_state)
			bit = !(m_shiftreg & 1);

		// shift the new bit into the shift register
		m_shiftreg = (m_shiftreg << 1) | (bit ? 1 : 0);

		// if we got all 0's or all 1's in the last n bits...
		if (((m_shiftreg & m_shiftreg_mask) == 0) || ((m_shiftreg & m_shiftreg_mask) == m_shiftreg_mask))
		{
			// coincidence is true
			if (!frozen) m_sylfilter += (((~m_sylfilter) & m_sylmask) >> m_sylshift);
		}
		else
		{
			// coincidence is false
			if (!frozen) m_sylfilter += (((~m_sylfilter) & m_sylmask) >> m_sylshift) + m_syladd;
		}
		m_sylfilter &= 0xfff;

		sum = util::sext(((~m_intfilter) >> m_intshift) + 1, 10);
	}
	else
	{
		// inactive clock transition
		if (m_shiftreg & 1)
		{
			sum = util::sext((~std::max(2, m_sylfilter >> 6)) + 1, 10);
		}
		else
		{
			sum = util::sext(std::max(2, m_sylfilter >> 6), 10);
		}
	}

	if (!frozen)
	{
		m_intfilter = util::sext(m_intfilter + sum, 10);
	}

	/* scale the result (-512 to 511) to -32768 thru 32767 */
	/*
	F E D C B A 9 8 7 6 5 4 3 2 1 0
	9 8 7 6 5 4 3 2 1 0/9 8 7 6 5 4
	*/
	m_next_sample = ((m_intfilter << 6) | (((m_intfilter & 0x3ff) ^ 0x200) >> 4));

	// update agc state
	if ((m_intfilter >= 0x100) || (m_intfilter <= -0x100))
		m_agc = false;
	else
		m_agc = true;

	// push agc state if a callback is present
	m_agc_push_cb(m_agc);
}

//-------------------------------------------------
//  sound_stream_update_legacy - handle a stream update
//-------------------------------------------------

void hc55516_device::sound_stream_update(sound_stream &stream)
{
	if (is_external_oscillator())
	{
		// external oscillator
		for (int i = 0; i < stream.samples(); i++)
		{
			stream.put_int(0, i, m_next_sample, 32768);

			m_samples_generated++;

			uint8_t clock_state = current_clock_state();

			// pull in next digit on the appropriate edge of the clock
			if (is_clock_changed(clock_state))
			{
				process_bit(m_buffered_bit, clock_state);
			}

			m_last_clock_state = clock_state;
		}
	}

	// software driven clock
	else
	{
		for (int i = 0; i < stream.samples(); i++)
			stream.put_int(0, i, m_next_sample, 32768);
	}
}



//#########################################
//                 HC55532
//#########################################
DEFINE_DEVICE_TYPE(HC55532, hc55532_device, "hc55532", "HC-55532")

hc55532_device::hc55532_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: hc55516_device(mconfig, HC55532, tag, owner, clock, 0xf80, 7, 0xfe1, 5)
{
}

//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void hc55532_device::device_reset()
{
	cvsd_device_base::device_reset();

	// simulate /FZ having been held for a while
	m_sylfilter = 0x7f;
	m_intfilter = 0;
	m_agc = true;
	m_buffered_fzq = true; // assuming /FZ was just released and is now high/inactive
}



//##########################################
//                 MC3417
//##########################################
DEFINE_DEVICE_TYPE(MC3417, mc3417_device, "mc3417", "MC3417")

mc3417_device::mc3417_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: mc3417_device(mconfig, MC3417, tag, owner, clock, 0x7)
{
}

// overridable type for mc3418 etc
mc3417_device::mc3417_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock, uint8_t shiftreg_mask)
	: cvsd_device_base(mconfig, type, tag, owner, clock, FALLING, shiftreg_mask)
	, m_charge(pow(exp(-1.0), 1.0 / (FILTER_CHARGE_TC * 16000.0)))
	, m_decay(pow(exp(-1.0), 1.0 / (FILTER_DECAY_TC * 16000.0)))
	, m_leak(pow(exp(-1.0), 1.0 / (INTEGRATOR_LEAK_TC * 16000.0)))
	, m_sylfilter(0.0)
	, m_intfilter(0.0)
	, m_curr_sample(0.0)
	, m_next_sample(0.0)
{
}

//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void mc3417_device::device_start()
{
	cvsd_device_base::device_start();

	save_item(NAME(m_sylfilter));
	save_item(NAME(m_intfilter));
	save_item(NAME(m_curr_sample));
	save_item(NAME(m_next_sample));
}

void mc3417_device::process_bit(bool bit, bool clock_state)
{
	if (is_active_clock_transition(clock_state))
	{
		// shift the new bit into the shift register
		m_shiftreg = (m_shiftreg << 1) | (bit ? 1 : 0);

		// move the estimator up or down a step based on the bit
		if (!bit)
			m_intfilter += m_sylfilter;
		else
			m_intfilter -= m_sylfilter;

		// simulate leakage
		m_intfilter *= m_leak;

		// if we got all 0's or all 1's in the last n bits, bump the step up
		if (((m_shiftreg & m_shiftreg_mask) == 0) || ((m_shiftreg & m_shiftreg_mask) == m_shiftreg_mask))
		{
			// coincidence is true
			m_sylfilter = FILTER_MAX - ((FILTER_MAX - m_sylfilter) * m_charge);

			if (m_sylfilter > FILTER_MAX)
				m_sylfilter = FILTER_MAX;
		}
		else
		{
			m_sylfilter *= m_decay;

			if (m_sylfilter < FILTER_MIN)
				m_sylfilter = FILTER_MIN;
		}

		// compute the sample as a 32-bit word
		m_next_sample = m_intfilter * SAMPLE_GAIN;
	}
}

void mc3417_device::sound_stream_update(sound_stream &stream)
{
	if (!is_external_oscillator())
	{
		// track how many samples we've updated without a clock; if it's been more than 1/32 of a second, output silence
		m_samples_generated += stream.samples();
		if (m_samples_generated > SAMPLE_RATE / 32)
		{
			m_samples_generated = SAMPLE_RATE;
			m_next_sample = 0;
		}
	}

	// compute the interpolation slope
	sound_stream::sample_t sample = m_curr_sample;
	sound_stream::sample_t slope = (m_next_sample - sample) / stream.samples();
	m_curr_sample = m_next_sample;

	if (is_external_oscillator())
	{
		// external oscillator
		for (int i = 0; i < stream.samples(); i++, sample += slope)
		{
			stream.put(0, i, sample);

			m_samples_generated++;

			uint8_t clock_state = current_clock_state();

			// pull in next digit on the appropriate edge of the clock
			if (is_clock_changed(clock_state))
			{
				process_bit(m_buffered_bit, clock_state);
			}

			m_last_clock_state = clock_state;
		}
	}

	// software driven clock
	else
	{
		for (int i = 0; i < stream.samples(); i++, sample += slope)
			stream.put(0, i, sample);
	}
}



//##########################################
//                 MC3418
//##########################################
DEFINE_DEVICE_TYPE(MC3418, mc3418_device, "mc3418", "MC3418")

mc3418_device::mc3418_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: mc3417_device(mconfig, MC3418, tag, owner, clock, 0xf)
{
}
