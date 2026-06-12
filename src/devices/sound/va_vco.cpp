// license:BSD-3-Clause
// copyright-holders:m1macrophage

#include "emu.h"
#include "va_vco.h"

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

float va_vco_device::ctrl2freq(float freq_ctrl) const
{
	return freq_ctrl;
}

float va_vco_device::ctrl2pw(float pw_ctrl) const
{
	return pw_ctrl;
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

	float remaining = 0.0F;
	const float thresh_phase = (threshold - m_ramp_min) / (m_ramp_max - m_ramp_min);
	if (m_phase < thresh_phase)
		remaining = thresh_phase - m_phase;
	else
		remaining = 1.0F - m_phase + thresh_phase;

	const float t = remaining / m_freq;
	return attotime::from_double(t);
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
}

// A faster way to do fmod(x, 1.0F).
static inline float fmod1f(float x)
{
	if (x < 0.0F || x >= 1.0F)
		x -= floorf(x);
	return x;
}

// Converts from [-1, 1] to [min_value, max_value]
static inline float transform(float x, float min_value, float max_value)
{
	return (max_value - min_value) * (x + 1.0F) / 2.0F + min_value;
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
	const std::array<std::pair<float, float>, 5>& disc, int n) const
{
	assert(n <= disc.size());
	float current = 0;
	float next = 0;

	for (int i = 0; i < n; ++i)
	{
		const auto [phase, jump] = disc[i];
		current += jump * poly_blep(phase);
		next += jump * poly_blep(fmod1f(phase + m_step));
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
	const std::array<std::pair<float, float>, 5>& disc, int n) const
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
		next += corner * poly_blamp(fmod1f(phase + m_step));
	}

	return std::make_pair(current, next);
}

float va_vco_device::ramp_wave(float phase)
{
	return 2.0F * phase - 1.0F;  // [-1, 1]
}

float va_vco_device::pulse_wave(float phase) const
{
	// The pulse waveform is generated by a comparator on the ramp waveform,
	// wired such that it goes high when the PW CV is larger than the (scaled)
	// ramp waveform.
	return (phase < m_pw) ? 1.0F : -1.0F;
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

float va_vco_device::midpulse_phase(float phase) const
{
	return fmod1f(phase + (1.0F - m_pw));
}

float va_vco_device::tritop_phase(float phase) const
{
	return fmod1f(phase + 0.5F);
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
// A note on the term "phase": this often refers to the phase of the oscillator,
// which is in the range [0, 1). But more generally, phase can be thought of as
// the distance from a discontinuity (the discontinuity occurs at 0). A lot of
// the code here, including the poly_blep() and poly_blamp() functions, use the
// term in that way.
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

		// Will a reset occur before the next sample?
		bool before_reset = will_wrap(old_phase);

		// Will a sync occur before the next sample?
		const bool before_sync = m_sync && will_wrap(m_sync_phase, m_sync_step);

		// These variables will only be initialized if a sync occurs.
		float sync_ttr = 0;  // time-to-reset (in samples) for the sync oscillator.
		float sync_phase = 0;  // Distance from sync discontinuity.
		// When a sync occurs, the oscillator's phase jumps from phase_at_sync
		// to new_phase_at_sync.
		float phase_at_sync = 0;
		float new_phase_at_sync = 0;

		bool post_sync_reset = false;
		if (before_sync)
		{
			// If a reset is also scheduled, it should be ignored if the sync
			// occurs first.
			sync_ttr = time_to_reset(m_sync_phase, m_sync_step);
			if (before_reset && sync_ttr < time_to_reset(old_phase))
					before_reset = false;

			phase_at_sync = fmod1f(old_phase + sync_ttr * m_step);
			new_phase_at_sync = 1.0F - phase_at_sync;  // Triangle core switched direction.

			const float delta = fmod1f(phase_at_sync - old_phase);
			sync_phase = 1.0F - delta;  // Distance from the sync discontinuity.
			m_phase = fmod1f(new_phase_at_sync - delta);  // Oscillator phase adjusted for sync.

			// Did the sync place the oscillator's phase right before a reset?
			if (will_wrap(m_phase) && will_wrap(new_phase_at_sync))
				post_sync_reset = true;
		}

		// Needed for both the ramp and triangle waveforms.
		const float naive_ramp = ramp_wave(old_phase);

		if (ramp_out)
		{
			int nd = 0;

			if (before_sync)
			{
				// Account for sync discontinuity.
				const float sync_jump = ramp_wave(new_phase_at_sync) - ramp_wave(phase_at_sync);
				disc[nd++] = std::make_pair(sync_phase, sync_jump);

				// Account for post-sync reset.
				if (post_sync_reset)
					disc[nd++] = std::make_pair(m_phase, RAMP_RESET_JUMP);
			}

			// Account for the reset discontinuity.
			if (before_reset)
				disc[nd++] = std::make_pair(old_phase, RAMP_RESET_JUMP);

			// Apply corrections for any discontinuities to the current and next sample.
			const auto [current_corr, next_corr] = poly_blep_corrections(disc, nd);
			const float ramp = naive_ramp + m_ramp_correction + current_corr;
			m_ramp_correction = next_corr;

			stream.put(OUTPUT_RAMP, i, transform(ramp, m_ramp_min, m_ramp_max));
		}

		if (pulse_out)
		{
			int nd = 0;

			// Will there be a flip before the next sample?
			// Note: this refers to the flip in the middle (or, more accurately,
			// the pulse width) of the waveform.
			const float flip_phase = midpulse_phase(old_phase);
			bool before_flip = will_wrap(flip_phase);

			if (before_sync)
			{
				// If the sync is scheduled to occur right before the scheduled
				// flip, ignore the flip since it won't actually happen.
				if (before_flip && sync_ttr < time_to_reset(flip_phase))
					before_flip = false;

				// Account for the sync discontinuity.
				const float sync_jump = pulse_wave(new_phase_at_sync) - pulse_wave(phase_at_sync);
				disc[nd++] = std::make_pair(sync_phase, sync_jump);

				// Account for a reset happening after the sync and before the
				// next sample.
				if (post_sync_reset)
					disc[nd++] = std::make_pair(m_phase, PULSE_RESET_JUMP);

				// Check and account for a flip happening after the sync and
				// before the next sample.
				const float post_sync_flip_phase = midpulse_phase(m_phase);
				if (will_wrap(post_sync_flip_phase) && will_wrap(midpulse_phase(new_phase_at_sync)))
					disc[nd++] = std::make_pair(post_sync_flip_phase, PULSE_FLIP_JUMP);
			}

			// Account for oscillator reset.
			if (before_reset)
				disc[nd++] = std::make_pair(old_phase, PULSE_RESET_JUMP);

			// Account for pulse flip.
			if (before_flip)
				disc[nd++] = std::make_pair(flip_phase, PULSE_FLIP_JUMP);

			// Apply corrections for any discontinuities to the current and next sample.
			const auto [current_corr, next_corr] = poly_blep_corrections(disc, nd);
			const float pulse = pulse_wave(old_phase) + m_pulse_correction + current_corr;
			m_pulse_correction = next_corr;

			stream.put(OUTPUT_PULSE, i, transform(pulse, m_pulse_min, m_pulse_max));
		}

		if (tri_out)
		{
			constexpr float TOP = 1.0F;
			constexpr float BOTTOM = -1.0F;
			int nd = 0;

			// Will the triangle flip direction? Note: this refers to the top
			// corner. The bottom corner is tracked in `before_reset`.
			const float flip_phase = tritop_phase(old_phase);
			bool before_flip = will_wrap(flip_phase);

			if (before_sync)
			{
				// If the sync is scheduled to occur right before the scheduled
				// flip, ignore the flip since it won't actually happen.
				if (before_flip && sync_ttr < time_to_reset(flip_phase))
					before_flip = false;

				// Determine if the sync will create a top or bottom corner,
				// and account for that discontinuity.
				assert(!before_reset || !before_flip);
				float corner = 0;
				if (before_reset)
				{
					// The reset (bottom corner) occurs before the sync. So the
					// sync will cause a top corner.
					corner = TOP;
				}
				else if (before_flip)
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
				disc[nd++] = std::make_pair(sync_phase, corner);

				// Account for a reset happening after the sync and before the
				// next sample.
				if (post_sync_reset)
					disc[nd++] = std::make_pair(m_phase, BOTTOM);

				// Check and account for a flip happening after the sync and
				// before the next sample.
				const float post_sync_flip_phase = tritop_phase(m_phase);
				if (will_wrap(post_sync_flip_phase) && will_wrap(tritop_phase(new_phase_at_sync)))
					disc[nd++] = std::make_pair(post_sync_flip_phase, TOP);
			}

			// Account for oscillator reset.
			if (before_reset)
				disc[nd++] = std::make_pair(old_phase, BOTTOM);

			// Account for the direction flip at the middle of the triangle's phase.
			if (before_flip)
				disc[nd++] = std::make_pair(flip_phase, TOP);

			// Apply corrections for any discontinuities to the current and next
			// sample.
			const auto [current_corr, next_corr] = poly_blamp_corrections(disc, nd);
			float triangle = 1.0F - 2.0F * fabsf(naive_ramp);  // Naive triangle waveform.
			triangle += m_tri_correction + current_corr;
			m_tri_correction = next_corr;

			stream.put(OUTPUT_TRIANGLE, i, transform(triangle, m_tri_min, m_tri_max));
		}

		m_phase = fmod1f(m_phase + m_step);
		m_sync_phase = fmod1f(m_sync_phase + m_sync_step);
	}
}

DEFINE_DEVICE_TYPE(VA_VCO, va_vco_device, "va_vco", "Virtual Analog Oscillator")
