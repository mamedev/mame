// license:BSD-3-Clause
// copyright-holders:m1macrophage

#include "emu.h"
#include "va_vca.h"

#include <cfloat>

va_vca_device::va_vca_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: va_vca_device(mconfig, VA_VCA, tag, owner, clock)
{
}

va_vca_device::va_vca_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, type, tag, owner, clock)
	, device_sound_interface(mconfig, *this)
	, m_stream(nullptr)
	, m_fixed_gain(1.0F)
{
}

void va_vca_device::set_fixed_gain_cv(float gain_cv)
{
	if (!m_stream)
		fatalerror("%s: set_fixed_gain_cv() cannot be called before device_start()\n", tag());
	if (BIT(get_sound_requested_inputs_mask(), INPUT_GAIN))
		fatalerror("%s: Cannot set a fixed gain CV when streaming it.\n", tag());

	const float gain = cv_to_gain(gain_cv);
	if (gain == m_fixed_gain)
		return;

	m_stream->update();
	m_fixed_gain = gain;
}

float va_vca_device::cv_to_gain(float cv) const
{
	return cv;
}

void va_vca_device::device_start()
{
	if (get_sound_requested_inputs_mask() != 0x01 && get_sound_requested_inputs_mask() != 0x03)
		fatalerror("%s: Input 0 must be connected, input 1 can optionally be connected. No other inputs allowed.\n", tag());

	m_stream = stream_alloc(get_sound_requested_inputs(), 1, SAMPLE_RATE_OUTPUT_ADAPTIVE);
	save_item(NAME(m_fixed_gain));
}

void va_vca_device::sound_stream_update(sound_stream &stream)
{
	if (BIT(get_sound_requested_inputs_mask(), INPUT_GAIN))
	{
		for (int i = 0; i < stream.samples(); i++)
			stream.put(0, i, stream.get(INPUT_AUDIO, i) * cv_to_gain(stream.get(INPUT_GAIN, i)));
	}
	else
	{
		for (int i = 0; i < stream.samples(); i++)
			stream.put(0, i, stream.get(INPUT_AUDIO, i) * m_fixed_gain);
	}
}


cem3360_vca_device::cem3360_vca_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: va_vca_device(mconfig, CEM3360_VCA, tag, owner, clock)
{
}

float cem3360_vca_device::cv_to_gain(float cv) const
{
	// Typical linear CV for max gain, as reported on the CEM3360 datasheet.
	constexpr float CEM3360_MAX_GAIN_CV = 1.93F;
	return std::clamp(cv, 0.0F, CEM3360_MAX_GAIN_CV) / CEM3360_MAX_GAIN_CV;
}


DEFINE_DEVICE_TYPE(VA_VCA, va_vca_device, "va_vca", "Voltage-controlled amplifier")
DEFINE_DEVICE_TYPE(CEM3360_VCA, cem3360_vca_device, "cem3360_vca", "CEM3360-based VCA")
