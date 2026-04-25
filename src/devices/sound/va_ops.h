// license:BSD-3-Clause
// copyright-holders:m1macrophage

// A collection of sound devices for performing operations on streams. These
// can naturally be inserted into stream processing pipelines.
// Useful for emulating circuits that process control or modulation signals in
// synthesizers, among other things.

#ifndef MAME_SOUND_VA_OPS_H
#define MAME_SOUND_VA_OPS_H

#pragma once

DECLARE_DEVICE_TYPE(VA_CONST, va_const_device)
DECLARE_DEVICE_TYPE(VA_SCALE_OFFSET, va_scale_offset_device)
DECLARE_DEVICE_TYPE(VA_COMPARATOR, va_comparator_device)


// Outputs a constant value to a stream. This is meant for things like
// firmware-controlled control voltages and other slow-changing signals. This is
// not meant for audio-rate signals, as there is no attempt at antialiasing.
class va_const_device : public device_t, public device_sound_interface
{
public:
	va_const_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock = 0) ATTR_COLD;

	va_const_device &set_value(float value);

protected:
	void device_start() override ATTR_COLD;
	void sound_stream_update(sound_stream &stream) override;

private:
	sound_stream *m_stream;
	float m_value;
};


// Scales and offsets the input stream. Useful for emulating certain op-amp and
// resistor network circuits. A common use is as a MIXER with an offset or
// constant input.
class va_scale_offset_device :  public device_t, public device_sound_interface
{
public:
	va_scale_offset_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock = 0) ATTR_COLD;

	va_scale_offset_device &set_scale(float scale);
	va_scale_offset_device &set_offset(float offset);

protected:
	void device_start() override ATTR_COLD;
	void sound_stream_update(sound_stream &stream) override;

private:
	sound_stream *m_stream;
	float m_scale;
	float m_offset;
};


// Emulates various comparator devices and circuits.
//
// The positive- and negative-going thresholds and output type can either be
// configured directly, or by using one of the helpers for specific circuits.
//
// Configuration helpers:
//
// * comp_oc_hyst_config: Configuration for an open-collector / open-drain
//   comparator (LM393 or similar), with hysteresis via positive feedback. Acts
//   like a Schmitt trigger inverter with configurable thresholds and output levels.
//
//                                     ______         Vpullup
//                                    |      \           |
//                   Input ---------- |-      \       Rpullup
//                                    |        \         |
//                                    |  COMP   > -------+------ Output
//                                    |        /         |
//                           +------- |+      /          |
//                           |        |______/           |
//                           |                           |
//    Vthresh --- Rthresh ---+--------- Rfeedback--------+
//
class va_comparator_device : public device_t, public device_sound_interface
{
public:
	struct comp_oc_hyst_config
	{
		const float v_minus;   // Negative supply voltage (often 0V).
		const float v_pullup;
		const float r_pullup;
		const float v_thresh;
		const float r_thresh;
		const float r_feedback;
	};

	va_comparator_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock = 0) ATTR_COLD;

	va_comparator_device &configure(float thresh_pos, float thresh_neg, bool inverted) ATTR_COLD;
	va_comparator_device &configure(const comp_oc_hyst_config &c) ATTR_COLD;

	int state();

protected:
	void device_start() override ATTR_COLD;
	void sound_stream_update(sound_stream &stream) override;

private:
	// configuration
	float m_thresh_pos;
	float m_thresh_neg;
	bool m_inverted;
	sound_stream *m_stream;

	// state
	bool m_state;
};


#endif  // MAME_SOUND_VA_OPS_H
