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
	, m_fixed_inv_input(0.0F)
{
}

void va_vca_device::update()
{
	if (m_stream)
		m_stream->update();
}

void va_vca_device::set_fixed_gain_cv(float gain_cv)
{
	if (BIT(get_sound_requested_inputs_mask(), INPUT_GAIN))
		fatalerror("%s: Cannot set a fixed gain CV when streaming it.\n", tag());

	const float gain = cv_to_gain(gain_cv);
	if (gain == m_fixed_gain)
		return;

	update();
	m_fixed_gain = gain;
}

void va_vca_device::set_fixed_inv_input(float x)
{
	if (BIT(get_sound_requested_inputs_mask(), INPUT_AUDIO_INV))
		fatalerror("%s: Cannot set the inverting input when streaming it.\n", tag());

	if (x == m_fixed_inv_input)
		return;

	update();
	m_fixed_inv_input = x;
}

float va_vca_device::diff(float p, float m) const
{
	return p - m;
}

float va_vca_device::cv_to_gain(float cv) const
{
	return cv;
}

u32 va_vca_device::preferred_sample_rate() const
{
	return SAMPLE_RATE_OUTPUT_ADAPTIVE;
}

void va_vca_device::device_start()
{
	if (!BIT(get_sound_requested_inputs_mask() , INPUT_AUDIO))
		fatalerror("%s: Input 0 must be connected.", tag());
	if (get_sound_requested_inputs_mask() & ~u64(0x07))
		fatalerror("%s: Only inputs 0-2 can be connected.", tag());

	m_stream = stream_alloc(get_sound_requested_inputs(), 1, preferred_sample_rate());

	save_item(NAME(m_fixed_gain));
	save_item(NAME(m_fixed_inv_input));
}

void va_vca_device::sound_stream_update(sound_stream &stream)
{
	const int n = stream.samples();
	const bool streaming_gain = BIT(get_sound_requested_inputs_mask(), INPUT_GAIN);
	const bool streaming_inv = BIT(get_sound_requested_inputs_mask(), INPUT_AUDIO_INV);

	for (int i = 0; i < n; ++i)
	{
		const float plus = stream.get(INPUT_AUDIO, i);
		const float minus = streaming_inv ? stream.get(INPUT_AUDIO_INV, i) : m_fixed_inv_input;
		const float gain = streaming_gain ? cv_to_gain(stream.get(INPUT_GAIN, i)) : m_fixed_gain;
		stream.put(0, i, gain * diff(plus, minus));
	}
}


ca3280_vca_device::ca3280_vca_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: va_vca_device(mconfig, CA3280_VCA, tag, owner, 0)
	, m_plus_scale(1)
	, m_minus_scale(1)
	, m_output_scale(1)
{
}

ca3280_vca_device &ca3280_vca_device::configure_plus_divider(float r_in, float r_gnd)
{
	m_plus_scale = r_gnd / (r_in + r_gnd);
	return *this;
}

ca3280_vca_device &ca3280_vca_device::configure_minus_divider(float r_in, float r_gnd)
{
	m_minus_scale = r_gnd / (r_in + r_gnd);
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

float ca3280_vca_device::diff(float p, float m) const
{
	constexpr float VT = 25.7E-3;  // Thermal voltage at 25 degrees C.
	return tanhf((m_plus_scale * p - m_minus_scale * m) / (2 * VT));
}

float ca3280_vca_device::cv_to_gain(float cv) const
{
	// cv = Iabc.
	return CA3280_IOUT_MAX_SCALE * cv * m_output_scale;
}

u32 ca3280_vca_device::preferred_sample_rate() const
{
	// Upsample to reduce aliasing due to tanh() distortion.
	return std::max(96000, machine().sample_rate());
}

ca3280_vca_lin_device::ca3280_vca_lin_device(const machine_config &mconfig, const char *tag, device_t *owner, float i_d)
	: va_vca_device(mconfig, CA3280_VCA_LIN, tag, owner, 0)
	, m_id(i_d)
	, m_output_scale(1)
	, m_cv_scale(1)
	, m_rp(-1)
	, m_rm(-1)
	, m_r_inv(-1)
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

ca3280_vca_lin_device &ca3280_vca_lin_device::configure_voltage_output(float r_out)
{
	// output voltage = output_current * R, where R is the resistor to ground
	// at the output.
	m_output_scale = r_out;
	update_cv_scale();
	return *this;
}

ca3280_vca_lin_device &ca3280_vca_lin_device::set_rplus(float r)
{
	assert(r > 0);
	if (r == m_rp)
		return *this;
	update();
	m_rp = r;
	m_r_inv = 1.0F / (m_rp + m_rm);
	return *this;
}

ca3280_vca_lin_device &ca3280_vca_lin_device::set_rminus(float r)
{
	assert(r > 0);
	if (r == m_rm)
		return *this;
	update();
	m_rm = r;
	m_r_inv = 1.0F / (m_rp + m_rm);
	return *this;
}

// The output current for a linearized OTA is:
//   Iout = Idiff * s * Iabc / Id
// Where:
//   s ~ an OTA-specific scaling factor (e.g. CA3280_IOUT_MAX_SCALE)
//   Iabc ~ transconductance (gain) control current.
//   Id ~ diode current. The current supplied to the Id pin.
//   Idiff ~ differential input current. See diff() below.

// Returns (s * Iabc / Id), possibly scaled to account for the current-to-voltage
// conversion, if so configured.
float ca3280_vca_lin_device::cv_to_gain(float cv) const
{
	// cv = Iabc.
	return cv * m_cv_scale;
}

void ca3280_vca_lin_device::update_cv_scale()
{
	m_cv_scale = CA3280_IOUT_MAX_SCALE * m_output_scale / m_id;
}

// Idiff = Im - Ip
// Where:
//   Im ~ current flowing out of the "-" input.
//   Ip ~ current flowing out of the "+" input.
//
// See also page 3 (Background section) of
// http://www.openmusiclabs.com/files/otadist.pdf
//
// This diagram shows the part of the OTA that generates Im and Ip. While this
// circuit is typical of OTAs with linearizing diodes (e.g. LM13700), the
// CA3280's relevant circuit is more complex (see datasheet). But it is
// functionally similar.
//
//            Id
//    OTA     |  <- Vtop
//    -----------------
//    |       |       |
//    |   +---+---+   |
//    |   |       |   |
//    |   D       D   |
//    |   v       v   |
//    |   |       |   |
//    -----------------
//       +|       |-
//        |       |
//  Ip |  Rp     Rm  | Im
//     v  |       |  v
//        Vp     Vm
//
// [1] Id = Ip + Im  => Im = Id - Ip
// [2] VDp = Vt * log(Ip/Is)
// [3] VDm = Vt * log(Im/Is)
// [4] Vtop = Vp + Ip * Rp + VDp
// [5] Vtop = Vm + Im * Rm + VDm
// [6] Is = Diode saturation current.
//
// Use [4] to solve for Ip and [5] to solve for Im, using the other equations to
// substitue variables:
//   Ip = (Id * Rm - (Vp - Vm) + Vt * log(Im/Ip)) / (Rp + Rm)
//   Im = (Id * Rp + (vp - Vm) + Vt * log(Ip/Im)) / (Rp + Rm)
//
// For arbitrary Rp and Rm, the above do not have closed form solutions.
// However, in typical applications, Id * Rm >> Vt * log(Im/Ip) and
// Id * Rp >> Vt * log(Ip/Im), so we can pretend the diodes are not there to get
// the approximations:
//   Ip ~= (Id * Rm - (Vp - Vm)) / (Rp + Rm)
//   Im ~= (Id * Rp + (vp - Vm)) / (Rp + Rm)
//   Idiff ~= Im - Ip
//
// If Rp = Rm = R, then the log terms cancel out and we get:
//   Idiff = Im - Ip = (Vp - Vm) / R
//
// Importantly, when Rp = Rm (which is typical), we don't need the
// "Id * Rp >> ..." assumptions to hold, so emulation accuracy is higher.
//
float ca3280_vca_lin_device::diff(float p, float m) const
{
	// p = Vp, m = Vm
	assert(m_rp > 0 && m_rm > 0 && m_r_inv > 0);
	const float dv = p - m;
	const float ip = (m_id * m_rm - dv) * m_r_inv;
	const float im = (m_id * m_rp + dv) * m_r_inv;
	return im - ip;
}

void ca3280_vca_lin_device::device_start()
{
	va_vca_device::device_start();
	save_item(NAME(m_rp));
	save_item(NAME(m_rm));
	save_item(NAME(m_r_inv));
}

void ca3280_vca_lin_device::device_reset()
{
	va_vca_device::device_reset();
	if (m_rp <= 0 || m_rm <= 0 || m_r_inv <= 0)
		fatalerror("%s requires both set_rplus() and set_rminus() to have been called.\n", tag());
}

float ca3280_vca_lin_device::i_d(float r, float v_r, float v_minus)
{
	// Voltage at the Id input changes with current. So VD is just an
	// approximation. Not sure if this is 1 or 2 diode drops for the CA3280.
	// Going with 2 for now.
	constexpr float VD = 2 * 0.6;
	return (v_r - (v_minus + VD)) / r;
}


DEFINE_DEVICE_TYPE(VA_VCA,         va_vca_device,         "va_vca",         "Voltage-controlled amplifier")
DEFINE_DEVICE_TYPE(CA3280_VCA,     ca3280_vca_device,     "ca3280_vca",     "CA3280-based VCA")
DEFINE_DEVICE_TYPE(CA3280_VCA_LIN, ca3280_vca_lin_device, "ca3280_vca_lin", "Linearized CA3280-based VCA")
