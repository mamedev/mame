// license:BSD-3-Clause
// copyright-holders:m1macrophage

#ifndef MAME_SOUND_VA_VCO_H
#define MAME_SOUND_VA_VCO_H

#pragma once

DECLARE_DEVICE_TYPE(VA_VCO, va_vco_device)

// A virtual analog oscillator. Can be configured to emulate a variety of
// oscillator architectures.
//
// The frequency and pulse width control signals are provided in Hz and a
// fraction, respectively. The control signals can either be set by functions,
// or they can be provided as input streams.
//
// Subclasses can override the "ctrl2*()" methods to match the control inputs
// (e.g. voltage, current) of real oscillator devices, and they can use the
// configure_*() methods to tailor the oscillator's behavior.
//
// This class can also be used directly in drivers that emulate audio at the
// block-diagram-level, rather than at the circuit-level. No need to worry about
// voltages and currents in that case.
//
// A note on sync control: To avoid aliasing, we need to know the exact time the
// sync event occurred. It is not enough to know the sample it occurred at.
// Therefore the sync signal itself cannot be provided as a streaming input.
// Instead, we use the frequency of the sync oscillator as the input. The
// implementation will then keep track of when the sync events occur, based on
// that provided frequency.
//
class va_vco_device : public device_t, public device_sound_interface
{
public:
	enum input
	{
		// Frequency in Hz. Subclasses can change the control type by overriding ctrl2freq().
		INPUT_FREQ_CTRL = 0,
		// Pulse width fraction (0-1). Subclasses can change the control type by overriding ctrl2pw().
		INPUT_PW_CTRL,
		// Frequency of the sync oscillator.
		INPUT_SYNC_FREQ,
	};

	// The default output range is [-1, 1]. The min and max of the range can be
	// changed using the configure_{shape}_range() functions.
	enum output
	{
		// Starts at {min}, ramps towards and peaks at {max}, and ramps back
		// towards {min}.
		OUTPUT_TRIANGLE = 0,
		// Upwards ramp. Starts at {min}, ramps towards {max}, and resets to {min}.
		// Can be inverted to a downwards ramp with configure_ramp_range().
		OUTPUT_RAMP,  // AKA sawtooth
		// Starts at {max}, flips to {min} once the pulse width is reached, and
		// resets to {max} at the end of the cycle. Using configure_pulse_dc_comp()
		// might change the range output range. See its documentation.
		OUTPUT_PULSE,
		// Typically connected to INPUT_SYNC_FREQ of the synchronized oscillator.
		OUTPUT_FREQ,
	};

	enum sync_type
	{
		SYNC_TYPE_NONE = 0,
		// Resets the phase of the oscillator. Found in both: sawtooth- and
		// triangle-core oscillators.
		SYNC_TYPE_RESET,
		// Reverses or flips the slope of the source triangle in triangle-core
		// oscillators.
		SYNC_TYPE_REVERSE,
	};

	va_vco_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock = 0) ATTR_COLD;

	// The default output range for all waveforms is [-1, 1]. These methods
	// can change that, for example to voltages or currents, in order to match
	// real devices. The waveforms can be inverted by passing the max and min
	// values in "minval" and "maxval" respectively.
	va_vco_device &configure_ramp_range(float minval, float maxval) ATTR_COLD;
	va_vco_device &configure_pulse_range(float minval, float maxval) ATTR_COLD;
	va_vco_device &configure_triangle_range(float minval, float maxval) ATTR_COLD;

	// Some oscillator architectures adjust the level of the pulse waveform when
	// pulse width changes, to maintain a constant average DC. This removes
	// control signal feedthrough during pulse width modulation.
	//
	// IMPORTANT: enabling this will shift the output range of the oscillator
	// when PW != 50%. For example, if the range is configured as [-1, 1],
	// the average output (DC) will be 0 for all PW settings, but the output
	// range for different PW settings will differ. For example:
	//
	//  PW       output range
	//  50%      [-1, 1]
	//  10%      [-0.2, 1.8]
	//  90%      [-1.8, 0.2]
	va_vco_device &configure_pulse_dc_comp(bool enabled) ATTR_COLD;

	// Some oscillator architectures derive the pulse wave from a ramp wave
	// (the default configuration), others from a triangle wave.
	//
	// The phase of the pulse differs between these configurations. The center
	// of a triangle-derived pulse will match the middle of the oscillator phase.
	// This difference in phase has the following effects:
	//
	// * Modulating the pulse width of a ramp-derived pulse will affect its
	//   pitch to some extent, whereas pitch remains stable in the triangle-
	//   derived pulse.
	// * The two configurations will produce different loudness and timbre when
	//   mixed with other waveforms of the same oscillator.
	va_vco_device &configure_pulse_from_tri(bool tri_derived) ATTR_COLD;

	// See sync_type enum.
	va_vco_device &configure_sync_type(sync_type type) ATTR_COLD;

	// Sets the frequency in Hz. Subclasses can change the input type (e.g. to
	// a control current) by overriding ctrl2freq(). Cannot be used when
	// INPUT_FREQ_CTRL is connected.
	void set_freq_ctrl(float freq_ctrl);

	// Sets the pulse width fraction (0-1). Subclasses can change the input type
	// (e.g. to a control voltage) by overriding ctrl2pw(). Cannot be used when
	// INPUT_PW_CTRL is connected.
	void set_pw_ctrl(float pw_ctrl);

	// Can only be used if a sync type has been configured.
	void set_sync_enabled(bool enabled);

	float freq();  // in Hz
	u32 sample_rate() const { return m_stream->sample_rate(); }

	// Returns the time remaining for the ramp waveform to cross the specified
	// threshold, in an upwards direction. The threshold should lie within the
	// configured range of the ramp wave. This function also works for the
	// minimum value (the point at which a reset occurs).
	attotime ramp_time_to_thresh(float threshold);

protected:
	va_vco_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock) ATTR_COLD;

	void device_start() override ATTR_COLD;
	void sound_stream_update(sound_stream &stream) override;

	// Subclasses can override these, to support voltage or current control of
	// the oscillator frequency and pulse width.
	virtual float ctrl2freq(float freq_ctrl) const;
	virtual float ctrl2pw(float pw_ctrl) const;

private:
	// Information about an oscillator sync event.
	struct sync_event_t
	{
		bool synced = false;            // Did a sync event occur?

		// The variables below are only relevant when synced == true.
		float sync_ttr = 0.0F;          // time-to-reset (in samples) for the sync oscillator.
		float sync_phase = 0.0F;        // Phase distance from sync discontinuity.
		float synced_osc_phase = 0.0F;  // Post-sync oscillator phase at current sample.
		float phase_at_sync = 0.0F;     // Oscillator phase at the instant the sync occurred.
		float new_phase_at_sync = 0.0F; // Post-sync oscillator phase at the instant the sync occurred.
	};

	// Information about a waveform discontinuity, including interference by a sync.
	struct disc_event_t
	{
		bool disc = false;
		bool post_sync_disc = false;
		float disc_phase = 0.0F;
		float post_sync_disc_phase = 0.0F;
	};

	void set_freq_ctrl_internal(float freq_cc);
	void set_pw_ctrl_internal(float pw_cv);

	float poly_blep(float phase) const;
	std::pair<float, float> poly_blep_corrections(
		const std::array<std::pair<float, float>, 5> &disc, int n) const;

	float poly_blamp(float phase) const;
	std::pair<float, float> poly_blamp_corrections(
		const std::array<std::pair<float, float>, 5> &disc, int n) const;

	template<auto DiscPhaseFn>
	disc_event_t check_disc(float osc_phase, const sync_event_t &sync_info) const;

	static float ramp_wave(float phase);
	static float tri_wave(float phase);
	float pulse_wave(float phase) const;
	float tripulse_wave(float phase) const;

	static float will_wrap(float phase, float step);
	float will_wrap(float phase) const;

	static float time_to_reset(float phase, float step);
	float time_to_reset(float phase) const;

	float reset_phase(float osc_phase) const;
	float tritop_phase(float osc_phase) const;
	float pulse_flip_phase(float osc_phase) const;
	float tripulse_reset_phase(float osc_phase) const;
	float tripulse_flip_phase(float osc_phase) const;

	// configuration
	sound_stream *m_stream;
	float m_ramp_min;
	float m_ramp_max;
	float m_pulse_min;
	float m_pulse_max;
	float m_tri_min;
	float m_tri_max;
	bool m_pulse_dc_comp;
	bool m_pulse_tri;
	sync_type m_sync_type;

	// state
	float m_freq_ctrl;
	float m_freq;
	float m_pw_ctrl;
	float m_pw;
	float m_step;
	float m_phase;  // 0-1
	float m_ramp_correction;
	float m_pulse_correction;
	float m_tri_correction;
	bool m_sync;
	float m_sync_phase;  // 0-1
	float m_sync_step;
	attotime m_last_sample_time;
};

#endif  // MAME_SOUND_VA_VCO_H
