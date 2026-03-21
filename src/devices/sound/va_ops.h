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
// resistor network circuits.
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


#endif  // MAME_SOUND_VA_OPS_H
