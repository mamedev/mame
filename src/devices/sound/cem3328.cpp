// license:BSD-3-Clause
// copyright-holders:tim lindner

#include "emu.h"
#include "cem3328.h"
#include "machine/rescap.h"

#include <cmath>

// Datasheet conditions: V_EE = -12V, R_REF = 100K, C_A = C_B = C_C = 0.03uF,
// C_D = 300pF give 350Hz (typ.) at V_FREQ = 0. Pin 3 sits about 0.85V above
// V_EE at -12V and about 0.58V above at -5V.

namespace {

constexpr float DS_F0 = 350.0F;
constexpr float DS_CAP = 0.03e-6F;
constexpr float DS_IREF = (12.0F - 0.85F) / 100e3F;
constexpr float SCALE_V_PER_OCT = 0.020F;  // +20mV/octave (typ.)

float iref_pin_offset(float v_ee)
{
	// The datasheet gives only two points (-12V and -5V). Interpolating
	// linearly between them is an assumption.
	const float mag = std::fabs(v_ee);
	return 0.58F + (mag - 5.0F) * (0.85F - 0.58F) / (12.0F - 5.0F);
}

float base_freq(double c_p, double r_ref, double v_ee)
{
	// Pole frequency is proportional to I_REF and inversely proportional to
	// the pole capacitors.
	const float iref = (std::fabs(float(v_ee)) - iref_pin_offset(float(v_ee))) / float(r_ref);
	return DS_F0 * (iref / DS_IREF) * (DS_CAP / float(c_p));
}

} // anonymous namespace

cem3328_lpf4_device::cem3328_lpf4_device(const machine_config &mconfig, const char *tag, device_t *owner, double c_p, double r_ref, double v_ee)
	: va_lpf4_device(mconfig, CEM3328_LPF4, tag, owner, 0)
	, m_f0(base_freq(c_p, r_ref, v_ee))
{
	// Estimate, not measured: uses the starting point suggested by
	// va_lpf4_device::configure_drive(), 1 / (PP / 2), with PP = 6.5V, the
	// datasheet's nominal output swing for 1% THD.
	configure_drive(1.0F / 3.25F);
}

cem3328_lpf4_device::cem3328_lpf4_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: cem3328_lpf4_device(mconfig, tag, owner, CAP_U(0.033), RES_K(100), -12.0)
{
}

float cem3328_lpf4_device::cv_to_freq(float freq_cv) const
{
	// Exponential: positive voltage raises the cutoff.
	return m_f0 * exp2f(freq_cv / SCALE_V_PER_OCT);
}

float cem3328_lpf4_device::cv_to_res(float res_cv) const
{
	// "Reverse audio taper": fast rise at low CV, fine resolution near the
	// oscillation point. Feedback gain of 4 is the oscillation threshold.
	if (res_cv <= 0)
		return 0;
	if (res_cv < RES_OSC_V)
	{
		const float x = 1.0F - res_cv / RES_OSC_V;
		return 4.0F * (1.0F - x * x);
	}
	// Beyond the threshold the feedback gain keeps rising slowly, so the
	// oscillation gets stronger. The slope is an estimate; the datasheet only
	// says the range ends in oscillation.
	return 4.0F + 0.2F * (res_cv - RES_OSC_V);
}

DEFINE_DEVICE_TYPE(CEM3328_LPF4, cem3328_lpf4_device, "cem3328_lpf4", "CEM3328 4th order LPF")
