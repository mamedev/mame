// license:BSD-3-Clause
// copyright-holders:m1macrophage

#include "emu.h"
#include "va_vco.h"
#include "corefloat.h"

#include <limits>


va_vco_device::va_vco_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: va_vco_device(mconfig, VA_VCO, tag, owner, clock)
{
}

va_vco_device::va_vco_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, type, tag, owner, clock)
	, device_sound_interface(mconfig, *this)
	, m_stream(nullptr)
	, m_ramp_min(-1)
	, m_ramp_max(1)
	, m_pulse_min(-1)
	, m_pulse_max(1)
	, m_tri_min(-1)
	, m_tri_max(1)
	, m_pulse_dc_comp(false)
	, m_pulse_tri(false)
	, m_sync_type(SYNC_TYPE_NONE)
	, m_freq_ctrl(std::numeric_limits<float>::quiet_NaN())
	, m_freq(10.0F)
	, m_pw_ctrl(std::numeric_limits<float>::quiet_NaN())
	, m_pw(0.5F)
	, m_step(0)
	, m_phase(0)
	, m_ramp_correction(0)
	, m_pulse_correction(0)
	, m_tri_correction(0)
	, m_sync(false)
	, m_sync_phase(0)
	, m_sync_step(0)
	, m_last_sample_time(attotime::zero)
{
}

va_vco_device &va_vco_device::configure_ramp_range(float minval, float maxval)
{
	m_ramp_min = minval;
	m_ramp_max = maxval;
	return *this;
}

va_vco_device &va_vco_device::configure_pulse_range(float minval, float maxval)
{
	m_pulse_min = minval;
	m_pulse_max = maxval;
	return *this;
}

va_vco_device &va_vco_device::configure_triangle_range(float minval, float maxval)
{
	m_tri_min = minval;
	m_tri_max = maxval;
	return *this;
}

va_vco_device &va_vco_device::configure_pulse_dc_comp(bool enabled)
{
	m_pulse_dc_comp = enabled;
	return *this;
}

va_vco_device &va_vco_device::configure_pulse_from_tri(bool tri_derived)
{
	m_pulse_tri = tri_derived;
	return *this;
}

va_vco_device &va_vco_device::configure_sync_type(sync_type type)
{
	m_sync_type = type;
	return *this;
}

float va_vco_device::ctrl2freq(float freq_ctrl) const
{
	return freq_ctrl;
}

float va_vco_device::ctrl2pw(float pw_ctrl) const
{
	return pw_ctrl;
}

void va_vco_device::update_stream()
{
	if (m_stream)
		m_stream->update();
}

void va_vco_device::set_freq_ctrl_internal(float freq_ctrl)
{
	if (freq_ctrl == m_freq_ctrl)
		return;
	m_freq_ctrl = freq_ctrl;
	m_freq = ctrl2freq(freq_ctrl);
	m_step = m_freq / float(m_stream->sample_rate());
}

void va_vco_device::set_pw_ctrl_internal(float pw_ctrl)
{
	if (pw_ctrl == m_pw_ctrl)
		return;
	m_pw_ctrl = pw_ctrl;
	m_pw = ctrl2pw(pw_ctrl);
}

void va_vco_device::set_freq_ctrl(float freq_ctrl)
{
	if (!m_stream)  // Need to know the sample rate.
		fatalerror("%s: set_freq_ctrl() cannot be called before device_start().\n", tag());
	if (freq_ctrl == m_freq_ctrl)
		return;
	m_stream->update();
	set_freq_ctrl_internal(freq_ctrl);
}

void va_vco_device::set_pw_ctrl(float pw_ctrl)
{
	if (pw_ctrl == m_pw_ctrl)
		return;
	if (m_stream)
		m_stream->update();
	set_pw_ctrl_internal(pw_ctrl);
}

void va_vco_device::set_sync_enabled(bool enabled)
{
	const bool valid_sync_type = (m_sync_type == SYNC_TYPE_RESET || m_sync_type == SYNC_TYPE_REVERSE);
	assert(valid_sync_type);
	if (!valid_sync_type)
		logerror("set_sync_enabled() called even though no sync type has been configured.\n");

	if (enabled == m_sync)
		return;
	m_stream->update();
	m_sync = enabled;
}

float va_vco_device::freq()
{
	if (BIT(get_sound_requested_inputs_mask(), INPUT_FREQ_CTRL))
		m_stream->update();
	return m_freq;
}

attotime va_vco_device::ramp_time_to_thresh(float threshold)
{
	m_stream->update();

	const float dt = float((machine().time() - m_last_sample_time).as_double());
	assert(dt >= 0.0F);

	const float osc_phase = fpmod1(m_phase + dt * m_freq);
	const float thresh_phase = (threshold - m_ramp_min) / (m_ramp_max - m_ramp_min);

	float remaining = 0.0F;
	if (osc_phase < thresh_phase)
		remaining = thresh_phase - osc_phase;
	else
		remaining = 1.0F - osc_phase + thresh_phase;

	return attotime::from_double(remaining / m_freq);
}

void va_vco_device::device_start()
{
	m_stream = stream_alloc(get_sound_requested_inputs(), get_sound_requested_outputs(), machine().sample_rate());
	save_item(NAME(m_freq_ctrl));
	save_item(NAME(m_freq));
	save_item(NAME(m_pw_ctrl));
	save_item(NAME(m_pw));
	save_item(NAME(m_step));
	save_item(NAME(m_phase));
	save_item(NAME(m_ramp_correction));
	save_item(NAME(m_pulse_correction));
	save_item(NAME(m_tri_correction));
	save_item(NAME(m_sync));
	save_item(NAME(m_sync_phase));
	save_item(NAME(m_sync_step));
	save_item(NAME(m_last_sample_time));
}

// Implementation is based on:
// https://www.martin-finke.de/articles/audio-plugins-018-polyblep-oscillator/
// but a scaling of 2 is not baked into the calculations.
float va_vco_device::poly_blep(float phase) const
{
	float val = 0;
	if (phase < m_step)
	{
		const float t = phase / m_step;
		val = t - 0.5F * t * t - 0.5F;
	}
	else if (phase > 1.0F - m_step)
	{
		const float t = (phase - 1.0F) / m_step;
		val = t + 0.5F * t * t + 0.5F;
	}
	return val;
}

std::pair<float, float> va_vco_device::poly_blep_corrections(
	const std::array<std::pair<float, float>, 5> &disc, int n) const
{
	assert(n <= disc.size());
	float current = 0;
	float next = 0;

	for (int i = 0; i < n; ++i)
	{
		const auto [phase, jump] = disc[i];
		current += jump * poly_blep(phase);
		next += jump * poly_blep(fpmod1(phase + m_step));
	}

	return std::make_pair(current, next);
}

// Implementation is based on:
// https://dsp.stackexchange.com/questions/54790/polyblamp-anti-aliasing-in-c
float va_vco_device::poly_blamp(float phase) const
{
	float y = 0.0F;
	if (0.0F <= phase && phase < 2.0F * m_step)
	{
		const float x = phase / m_step;
		const float u = 2.0F - x;
		const float u2 = u * u;
		y -= u * u2 * u2;
		if (phase < m_step)
		{
			const float v = 1.0F - x;
			const float v2 = v * v;
			y += 4.0F * v * v2 * v2;
		}
	}
	return y * m_step / 15.0F;
}

std::pair<float, float> va_vco_device::poly_blamp_corrections(
	const std::array<std::pair<float, float>, 5> &disc, int n) const
{
	// NOTE: Only applying a 1-sample correction: before and after the
	// discontinuity. The reference PolyBLAMP implementation uses 2 samples.
	// Doing that would further complicate the implementation, for a fairly
	// small difference in quality (barely visible in spectra).

	assert(n <= disc.size());
	float current = 0;
	float next = 0;

	for (int i = 0; i < n; ++i)
	{
		const auto [phase, corner] = disc[i];

		// top: corner = 1, bottom: corner = -1.
		// Note that the signs are opposite of those in the reference (see
		// poly_blamp()), because the triangle wave in the reference is inverted.
		current += corner * poly_blamp(1.0F - phase);
		next += corner * poly_blamp(fpmod1(phase + m_step));
	}

	return std::make_pair(current, next);
}

// Checks if an inherent (i.e. not sync) discontinuity will occur between the
// current and next sample, taking into account interference by a sync event.
template<auto DiscPhaseFn>
va_vco_device::disc_event_t va_vco_device::check_disc(float osc_phase, const sync_event_t &sync_info) const
{
	disc_event_t disc;

	disc.disc_phase = (this->*DiscPhaseFn)(osc_phase);
	disc.disc = will_wrap(disc.disc_phase);

	if (sync_info.synced)
	{
		// If a discontinuity (e.g. a reset) is also scheduled, it should be
		// ignored if the sync occured first.
		if (disc.disc && sync_info.sync_ttr < time_to_reset(disc.disc_phase))
			disc.disc = false;

		// Did the sync place the oscillator's phase right before the discontinuity?
		// This is likely for SYNC_TYPE_REVERSE, since a sync places the oscillator
		// at an arbitrary phase. Whereas it can only occur in extreme settings
		// (e.g. small PW, high frequency) for SYNC_TYPE_RESET, since this sync
		// type always places the phase at the start.
		disc.post_sync_disc_phase = (this->*DiscPhaseFn)(sync_info.synced_osc_phase);
		if (will_wrap(disc.post_sync_disc_phase) && will_wrap((this->*DiscPhaseFn)(sync_info.new_phase_at_sync)))
			disc.post_sync_disc = true;
	}

	return disc;
}

float va_vco_device::ramp_wave(float phase)
{
	return 2.0F * phase - 1.0F;  // [-1, 1]
}

float va_vco_device::tri_wave(float phase)
{
	return 1.0F - 2.0F * fabsf(ramp_wave(phase));  // [-1, 1]
}

float va_vco_device::pulse_wave(float phase) const
{
	float wave = (phase < m_pw) ? 1.0F : -1.0F;  // [-1, 1]
	if (m_pulse_dc_comp)
		wave -= m_pw + m_pw - 1.0F;  // [-2, 2], see configure_pulse_dc_comp() usage doc.
	return wave;
}

float va_vco_device::tripulse_wave(float phase) const
{
	// Either [-1, 1] or [-2, 2], depending on m_pulse_dc_comp.
	return pulse_wave(tripulse_reset_phase(phase));
}

float va_vco_device::will_wrap(float phase, float step)
{
	return phase > 1.0F - step;
}

float va_vco_device::will_wrap(float phase) const
{
	return will_wrap(phase, m_step);
}

float va_vco_device::time_to_reset(float phase, float step)  // In number of samples.
{
	return (1.0F - phase) / step;
}

float va_vco_device::time_to_reset(float phase) const
{
	return time_to_reset(phase, m_step);
}

// A note on the term "phase": this often refers to the phase of the oscillator,
// which is in the range [0, 1). But more generally, phase can be thought of as
// the distance from a discontinuity (the discontinuity occurs at 0). A lot of
// the code here, including the poly_blep() and poly_blamp() functions, use the
// term in that way.

// Phase distance from the oscillator's reset.
// The identity function. Useful as a template argument to check_disc().
float va_vco_device::reset_phase(float osc_phase) const
{
	return osc_phase;
}

// Phase distance from the top of the triangle.
float va_vco_device::tritop_phase(float osc_phase) const
{
	return fpmod1(osc_phase + 0.5F);
}

// Phase distance from the mid-pulse flip (downwards jump), for a ramp-derived pulse.
float va_vco_device::pulse_flip_phase(float osc_phase) const
{
	return fpmod1(osc_phase + (1.0F - m_pw));
}

// Phase distance from the reset (upward jump), for a triangle-derived pulse.
float va_vco_device::tripulse_reset_phase(float osc_phase) const
{
	return fpmod1(osc_phase + 0.5F * m_pw);
}

// Phase distance from a flip (downwards jump), for a triangle-derived pulse.
float va_vco_device::tripulse_flip_phase(float osc_phase) const
{
	return fpmod1(osc_phase + 1.0F - 0.5F * m_pw);
}

// Using the PolyBLEP (for ramp and pulse) and PolyBLAMP (for triangle)
// algorithms to generate anti-aliased waveforms.
//
// Those algorithms start with the "naive" versions of the waveforms, and only
// apply corrections at waveform discontinuities: one on the sample before the
// discontinuity, and one on the sample after.
//
// The ramp waveform has one discontinuity (referred to as "reset" here), and
// the pulse and triangle waveforms have two each ("reset" and "flip"). Enabling
// sync adds one more possible discontinuity ("sync").
//
// The sync discontinuity could occur between the same two samples as the
// inherent ones. We need to correct for each of the discontinuities that
// occurred in a sample interval. For example:
//
// - If a reset is scheduled to happen before a sync, then we need to correct
//   for both the reset and sync discontinuities.
// - If a sync is scheduled to happen before a reset, then the scheduled reset
//   needs to be ignored, since it won't actually occur.
// - A sync might place the oscillator right before a reset. We need to correct
//   for both the sync and the post-sync reset in that case.
//
// The above means there could be up to 3 discontinuities in a single sample
// interval (reset, sync, second reset). The same applies to the flip
// discontinuities of the triangle and pulse. Also, if the pulse width of the
// pulse wave is very small or large, the reset, flip and sync could occur in
// the same sample interval, and the oscillator could be placed right before
// the reset and flip, for a total of 5 discontinuities to correct for.
//
void va_vco_device::sound_stream_update(sound_stream &stream)
{
	constexpr float RAMP_RESET_JUMP = -2.0F;  // Resets from 1 to -1.
	constexpr float PULSE_RESET_JUMP = 2.0F;  // Resets from -1 to 1.
	constexpr float PULSE_FLIP_JUMP = -2.0F;  // Flips from 1 to -1.

	const bool streaming_freq = BIT(get_sound_requested_inputs_mask(), INPUT_FREQ_CTRL);
	const bool streaming_pw = BIT(get_sound_requested_inputs_mask(), INPUT_PW_CTRL);
	const bool streaming_sync = BIT(get_sound_requested_inputs_mask(), INPUT_SYNC_FREQ);
	assert(!m_sync || streaming_sync);

	const bool tri_out = BIT(get_sound_requested_outputs_mask(), OUTPUT_TRIANGLE);
	const bool ramp_out = BIT(get_sound_requested_outputs_mask(), OUTPUT_RAMP);
	const bool pulse_out = BIT(get_sound_requested_outputs_mask(), OUTPUT_PULSE);
	const bool freq_out = BIT(get_sound_requested_outputs_mask(), OUTPUT_FREQ);

	const int n = stream.samples();
	const float sample_rate_inv = 1.0F / float(m_stream->sample_rate());
	std::array<std::pair<float, float>, 5> disc;  // Keeps track of discontinuities.

	for (int i = 0; i < n; ++i)
	{
		if (streaming_freq)
			set_freq_ctrl_internal(stream.get(INPUT_FREQ_CTRL, i));
		if (streaming_pw)
			set_pw_ctrl_internal(stream.get(INPUT_PW_CTRL, i));
		if (streaming_sync)
			m_sync_step = sample_rate_inv * stream.get(INPUT_SYNC_FREQ, i);
		if (freq_out)
			stream.put(OUTPUT_FREQ, i, m_freq);

		// While there is a lot of logic here, the approach is simple at a high
		// level. For each waveform:
		// - Find all discontinuities occurring between the current and next
		//   sample, and append them to the `disc` array.
		// - Compute "naive" waveform.
		// - Apply corrections to the current sample (if applicable).
		// - Store corrections to be applied to the next sample (if applicable).
		//
		// The bulk of the code deals with tracking all possible discontinuities,
		// which gets complicated with oscillator sync.

		// The code below assumes that discontinuities of the same type can
		// occur up to once every 2 samples.
		assert(m_step <= 0.5F && m_sync_step <= 0.5F);

		const float old_phase = m_phase;

		sync_event_t sync_event;
		sync_event.synced = m_sync && will_wrap(m_sync_phase, m_sync_step);

		if (sync_event.synced)
		{
			sync_event.sync_ttr = time_to_reset(m_sync_phase, m_sync_step);
			sync_event.phase_at_sync = fpmod1(old_phase + sync_event.sync_ttr * m_step);

			if (m_sync_type == SYNC_TYPE_REVERSE)
				sync_event.new_phase_at_sync = 1.0F - sync_event.phase_at_sync;  // Triangle core reversed slope.
			else  // m_sync_type == SYNC_TYPE_RESET
				sync_event.new_phase_at_sync = 0.0F;  // Saw or triangle core reset.

			const float delta = fpmod1(sync_event.phase_at_sync - old_phase);
			sync_event.sync_phase = 1.0F - delta;  // Distance from the sync discontinuity.
			m_phase = fpmod1(sync_event.new_phase_at_sync - delta);  // Oscillator phase adjusted for sync.
			sync_event.synced_osc_phase = m_phase;
		}

		// Applies to multiple waveforms.
		const disc_event_t reset = check_disc<&va_vco_device::reset_phase>(old_phase, sync_event);

		if (ramp_out)
		{
			// The ramp waveform has a single inherent discontinuity ("reset").

			int nd = 0;

			if (reset.disc)
				disc[nd++] = std::make_pair(reset.disc_phase, RAMP_RESET_JUMP);

			if (sync_event.synced)
			{
				const float sync_jump = ramp_wave(sync_event.new_phase_at_sync) - ramp_wave(sync_event.phase_at_sync);
				disc[nd++] = std::make_pair(sync_event.sync_phase, sync_jump);
			}

			if (reset.post_sync_disc)
				disc[nd++] = std::make_pair(reset.post_sync_disc_phase, RAMP_RESET_JUMP);

			const auto [current_corr, next_corr] = poly_blep_corrections(disc, nd);
			const float ramp = ramp_wave(old_phase) + m_ramp_correction + current_corr;
			m_ramp_correction = next_corr;

			stream.put(OUTPUT_RAMP, i, fmaprange(ramp, m_ramp_min, m_ramp_max));
		}

		if (pulse_out)
		{
			int nd = 0;
			float pulse = 0;

			if (m_pulse_tri)
			{
				// A triangle-derived pulse waveform has two inherent discontinuities.
				// One when the triangle passes the PW-dependent threshold in an
				// upwards direction ("reset"), and one when it passes it in the
				// downwards direction ("flip").

				const disc_event_t reset = check_disc<&va_vco_device::tripulse_reset_phase>(old_phase, sync_event);
				const disc_event_t flip = check_disc<&va_vco_device::tripulse_flip_phase>(old_phase, sync_event);

				if (reset.disc)
					disc[nd++] = std::make_pair(reset.disc_phase, PULSE_RESET_JUMP);

				if (flip.disc)
					disc[nd++] = std::make_pair(flip.disc_phase, PULSE_FLIP_JUMP);

				// We only need to correct for a "reset" sync. A "reverse" sync
				// reflects the phase around 0.5, which is where a
				// triangle-derived pulse is centered. This means the pulse's
				// value won't change when a "reverse" sync occurs.
				if (sync_event.synced && m_sync_type == SYNC_TYPE_RESET)
				{
					const float sync_jump = tripulse_wave(sync_event.new_phase_at_sync) - tripulse_wave(sync_event.phase_at_sync);
					disc[nd++] = std::make_pair(sync_event.sync_phase, sync_jump);
				}

				if (reset.post_sync_disc)
					disc[nd++] = std::make_pair(reset.post_sync_disc_phase, PULSE_RESET_JUMP);

				if (flip.post_sync_disc)
					disc[nd++] = std::make_pair(flip.post_sync_disc_phase, PULSE_FLIP_JUMP);

				pulse = tripulse_wave(old_phase);
			}
			else
			{
				// A ramp-derived pulse waveform has two inherent discontinuities.
				// One when the underlying ramp resets ("reset"), and one when
				// the PW-dependent threshold is exceeded ("flip").

				const disc_event_t flip = check_disc<&va_vco_device::pulse_flip_phase>(old_phase, sync_event);

				if (reset.disc)
					disc[nd++] = std::make_pair(reset.disc_phase, PULSE_RESET_JUMP);

				if (flip.disc)
					disc[nd++] = std::make_pair(flip.disc_phase, PULSE_FLIP_JUMP);

				if (sync_event.synced)
				{
					const float sync_jump = pulse_wave(sync_event.new_phase_at_sync) - pulse_wave(sync_event.phase_at_sync);
					disc[nd++] = std::make_pair(sync_event.sync_phase, sync_jump);
				}

				if (reset.post_sync_disc)
					disc[nd++] = std::make_pair(reset.post_sync_disc_phase, PULSE_RESET_JUMP);

				if (flip.post_sync_disc)
					disc[nd++] = std::make_pair(flip.post_sync_disc_phase, PULSE_FLIP_JUMP);

				pulse = pulse_wave(old_phase);
			}

			const auto [current_corr, next_corr] = poly_blep_corrections(disc, nd);
			pulse += m_pulse_correction + current_corr;
			m_pulse_correction = next_corr;

			stream.put(OUTPUT_PULSE, i, fmaprange(pulse, m_pulse_min, m_pulse_max));
		}

		if (tri_out)
		{
			// A triangle waveform has two inherent discontinuities. One at its
			// min value ("reset") and one at its max value ("flip").

			constexpr float TOP = 1.0F;
			constexpr float BOTTOM = -1.0F;

			const disc_event_t flip = check_disc<&va_vco_device::tritop_phase>(old_phase, sync_event);
			int nd = 0;

			if (reset.disc)
				disc[nd++] = std::make_pair(reset.disc_phase, BOTTOM);

			if (flip.disc)
				disc[nd++] = std::make_pair(flip.disc_phase, TOP);

			if (reset.post_sync_disc)
				disc[nd++] = std::make_pair(reset.post_sync_disc_phase, BOTTOM);

			if (flip.post_sync_disc)
				disc[nd++] = std::make_pair(flip.post_sync_disc_phase, TOP);

			const auto [current_corr, next_corr] = poly_blamp_corrections(disc, nd);
			float triangle = tri_wave(old_phase) + m_tri_correction + current_corr;
			m_tri_correction = next_corr;

			if (sync_event.synced)
			{
				if (m_sync_type == SYNC_TYPE_REVERSE)
				{
					// Determine if the sync will create a top or bottom corner.
					assert(!reset.disc || !flip.disc);
					float corner = 0;
					if (reset.disc)
					{
						// The reset (bottom corner) occurs before the sync. So the
						// sync will cause a top corner.
						corner = TOP;
					}
					else if (flip.disc)
					{
						// The flip (top corner) occurs before the sync. So the sync
						// will cause a bottom corner.
						corner = BOTTOM;
					}
					else
					{
						// If the triangle is in the "moving upwards" phase, the
						// sync will cause a top corner. Otherwise, a bottom one.
						corner = (old_phase < 0.5F) ? TOP : BOTTOM;
					}

					// The sync caused the triangle to change direction.
					// Need to apply a polyBLAMP correction.
					triangle += corner * poly_blamp(1.0F - sync_event.sync_phase);
					m_tri_correction += corner * poly_blamp(fpmod1(sync_event.sync_phase + m_step));
				}
				else  // m_sync_type == SYNC_TYPE_RESET
				{
					// The sync caused the triangle to reset to its min value.
					// Need to apply a polyBLEP correction.
					const float sync_jump = tri_wave(sync_event.new_phase_at_sync) - tri_wave(sync_event.phase_at_sync);
					triangle += sync_jump * poly_blep(sync_event.sync_phase);
					m_tri_correction += sync_jump * poly_blep(fpmod1(sync_event.sync_phase + m_step));
				}
			}

			stream.put(OUTPUT_TRIANGLE, i, fmaprange(triangle, m_tri_min, m_tri_max));
		}

		m_phase = fpmod1(m_phase + m_step);
		m_sync_phase = fpmod1(m_sync_phase + m_sync_step);
	}

	m_last_sample_time = stream.end_time() - stream.sample_period();
}

DEFINE_DEVICE_TYPE(VA_VCO, va_vco_device, "va_vco", "Virtual Analog Oscillator")
