// license:BSD-3-Clause
// copyright-holders:m1macrophage

#include "emu.h"
#include "va_vca.h"

#include <cfloat>

// Default max peak-to-peak voltage for the CV input.
static constexpr const float DEFAULT_MAX_VPP = 100;

DEFINE_DEVICE_TYPE(VA_VCA, va_vca_device, "va_vca", "Voltage Controlled Amplifier")

va_vca_device::va_vca_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, VA_VCA, tag, owner, clock)
	, device_sound_interface(mconfig, *this)
	, m_stream(nullptr)
	, m_has_cv_stream(false)
	, m_min_cv(-DEFAULT_MAX_VPP)
	, m_max_cv(DEFAULT_MAX_VPP)
	, m_cv_scale(1.0F)
	, m_fixed_gain(1.0F)
{
}

va_vca_device &va_vca_device::configure_streaming_cv(bool use_streaming_cv)
{
	m_has_cv_stream = use_streaming_cv;
	return *this;
}

va_vca_device &va_vca_device::configure_cem3360_linear_cv()
{
	// TODO: For now, the CEM3360 is treated as a linear device. But since it
	// is OTA-based, it likely has a tanh response. This requires more research.

	// Typical linear CV for max gain, as reported on the CEM3360 datasheet.
	static constexpr const float CEM3360_MAX_GAIN_CV = 1.93F;
	m_min_cv = 0;
	m_max_cv = CEM3360_MAX_GAIN_CV;
	m_cv_scale = 1.0F / CEM3360_MAX_GAIN_CV;
	return *this;
}

void va_vca_device::set_fixed_cv(float cv)
{
	m_stream->update();
	m_fixed_gain = cv_to_gain(cv);
}

void va_vca_device::device_start()
{
	const int input_count = m_has_cv_stream ? 2 : 1;
	m_stream = stream_alloc(input_count, 1, SAMPLE_RATE_OUTPUT_ADAPTIVE);
	save_item(NAME(m_fixed_gain));
}

void va_vca_device::sound_stream_update(sound_stream &stream)
{
	if (m_has_cv_stream)
	{
		for (int i = 0; i < stream.samples(); i++)
			stream.put(0, i, stream.get(0, i) * cv_to_gain(stream.get(1, i)));
	}
	else
	{
		for (int i = 0; i < stream.samples(); i++)
			stream.put(0, i, stream.get(0, i) * m_fixed_gain);
	}
}

float va_vca_device::cv_to_gain(float cv) const
{
	return std::clamp(cv, m_min_cv, m_max_cv) * m_cv_scale;
}

