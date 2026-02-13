// license:BSD-3-Clause
// copyright-holders:m1macrophage

#include "emu.h"
#include "va_vca.h"

#include <cfloat>

namespace {

// Ioutmax = (4.1 / 5.0) * Iabc = 0.82 * Iabc.
// Look for "Peak Output Current" in the CA3280 datasheet.
constexpr float CA3280_IOUT_MAX_SCALE = 4.1F / 5.0F;

}  // anonymous namespace


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

float va_vca_device::distorted(float s) const
{
	fatalerror("A va_vca_device subclass enabled distortion without implementing the distorted() function.\n");
	return s;
}

void va_vca_device::device_start()
{
	if (get_sound_requested_inputs_mask() != 0x01 && get_sound_requested_inputs_mask() != 0x03)
		fatalerror("%s: Input 0 must be connected, input 1 can optionally be connected. No other inputs allowed.\n", tag());

	// Upsample when using distortion, to reduce aliasing.
	const u32 sample_rate = has_distortion() ? std::max(96000, machine().sample_rate()) : SAMPLE_RATE_OUTPUT_ADAPTIVE;
	m_stream = stream_alloc(get_sound_requested_inputs(), 1, sample_rate);
	save_item(NAME(m_fixed_gain));
}

void va_vca_device::sound_stream_update(sound_stream &stream)
{
	if (has_distortion())
	{
		if (BIT(get_sound_requested_inputs_mask(), INPUT_GAIN))
		{
			for (int i = 0; i < stream.samples(); i++)
				stream.put(0, i, distorted(stream.get(INPUT_AUDIO, i)) * cv_to_gain(stream.get(INPUT_GAIN, i)));
		}
		else
		{
			for (int i = 0; i < stream.samples(); i++)
				stream.put(0, i, distorted(stream.get(INPUT_AUDIO, i)) * m_fixed_gain);
		}
	}
	else
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
}


ca3280_vca_device::ca3280_vca_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: va_vca_device(mconfig, CA3280_VCA, tag, owner, 0)
	, m_input_scale(1)
	, m_output_scale(1)
{
}

ca3280_vca_device &ca3280_vca_device::configure_input_divider(float r_in, float r_gnd)
{
	m_input_scale = r_gnd / (r_in + r_gnd);
	return *this;
}

ca3280_vca_device &ca3280_vca_device::configure_voltage_output(float r_out)
{
	// output voltage = output_current * Rout, where Rout is the resistor to
	// ground at the output.
	m_output_scale = r_out;
	return *this;
}

// The output current for a non-linearized OTA can be determined by:
//   Iout = Ioutmax * tanh(DVin / (2 * VT))
// For the CA3280:
//   Ioutmax = CA3280_IOUT_MAX_SCALE * Iabc
// Where:
//   Iabc ~ transconductance (gain) control current.
//   DVIn ~ (Vin+ - Vin-). Vin- is typically 0V.
//   VT ~ Thermal voltage. A temperature-dependent constant.

float ca3280_vca_device::cv_to_gain(float cv) const
{
	// cv = Iabc.
	return CA3280_IOUT_MAX_SCALE * cv * m_output_scale;
}

float ca3280_vca_device::distorted(float s) const
{
	constexpr float VT = 25.7E-3;  // Thermal voltage at 25 degrees C.
	return tanhf((s * m_input_scale) / (2 * VT));
}


ca3280_vca_lin_device::ca3280_vca_lin_device(const machine_config &mconfig, const char *tag, device_t *owner, float i_d)
	: va_vca_device(mconfig, CA3280_VCA_LIN, tag, owner, 0)
	, m_i_d_inv(1.0F / i_d)
	, m_input_scale(1)
	, m_output_scale(1)
	, m_cv_scale(1)
{
	update_cv_scale();
}

ca3280_vca_lin_device::ca3280_vca_lin_device(const machine_config &mconfig, const char *tag, device_t *owner, float r, float v_r, float v_minus)
	: ca3280_vca_lin_device(mconfig, tag, owner, i_d(r, v_r, v_minus))
{
}

ca3280_vca_lin_device::ca3280_vca_lin_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: ca3280_vca_lin_device(mconfig, tag, owner, 1.0F)
{
}

ca3280_vca_lin_device &ca3280_vca_lin_device::configure_voltage_input(float r_in)
{
	m_input_scale = 1.0F / r_in;
	update_cv_scale();
	return *this;
}

ca3280_vca_lin_device &ca3280_vca_lin_device::configure_voltage_output(float r_out)
{
	// output voltage = output_current * R, where R is the resistor to ground
	// at the output.
	m_output_scale = r_out;
	update_cv_scale();
	return *this;
}

// The output current for a linearized OTA can be determined by:
//   Iout = Ioutmax * Iin / Id
// For the CA3280:
//   Ioutmax = CA3280_IOUT_MAX_SCALE * Iabc
// Where:
//   Iabc ~ transconductance (gain) control current.
//   Iin ~ current at in+, assuming in- is connected to ground (via a
//         resistor).
//   Id ~ diode current. The current supplied to the Id pin.

float ca3280_vca_lin_device::cv_to_gain(float cv) const
{
	// cv = Iabc.
	// m_cv_scale incorporates 1 / Id, along with the optional factors to
	// convert the input voltage to a current, and the output current to a
	// a voltage (see update_cv_scale() and the various configure_*() methods).
	return cv * m_cv_scale;
}

void ca3280_vca_lin_device::update_cv_scale()
{
	m_cv_scale = CA3280_IOUT_MAX_SCALE * m_input_scale * m_i_d_inv * m_output_scale;
}

float ca3280_vca_lin_device::i_d(float r, float v_r, float v_minus)
{
	// Voltage at the Id input changes with current. So VD is just an
	// approximation. Not sure if this is 1 or 2 diode drops for the CA3280.
	// Going with 2 for now.
	constexpr float VD = 2 * 0.6;
	return (v_r - (v_minus + VD)) / r;
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


DEFINE_DEVICE_TYPE(VA_VCA,         va_vca_device,         "va_vca",         "Voltage-controlled amplifier")
DEFINE_DEVICE_TYPE(CA3280_VCA,     ca3280_vca_device,     "ca3280_vca",     "CA3280-based VCA")
DEFINE_DEVICE_TYPE(CA3280_VCA_LIN, ca3280_vca_lin_device, "ca3280_vca_lin", "Linearized CA3280-based VCA")
DEFINE_DEVICE_TYPE(CEM3360_VCA,    cem3360_vca_device,    "cem3360_vca",    "CEM3360-based VCA")
