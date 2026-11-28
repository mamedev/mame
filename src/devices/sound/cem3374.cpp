// license:BSD-3-Clause
// copyright-holders:m1macrophage

#include "emu.h"
#include "cem3374.h"

#include "machine/rescap.h"

#include <cmath>

#define LOG_FREQ (1U << 1)
#define LOG_PW   (1U << 2)

#define VERBOSE (0)
//#define LOG_OUTPUT_FUNC osd_printf_info

#include "logmacro.h"


cem3374_device::cem3374_device(const machine_config &mconfig, const char *tag, device_t *owner, osc osc_side, float vcc, float c_f)
	: va_vco_device(mconfig, CEM3374, tag, owner, 0)
	, m_vcc(vcc)
	, m_cf(c_f)
	, m_vthresh(0)  // Initialized by the call to configure_vthresh() below.
	, m_pulse_src_gain(-1)
{
	// A voltage divider inside the chip sets the triangle comparator reference,
	// when there is no external reference provided.
	configure_vthresh(m_vcc * RES_VOLTAGE_DIVIDER(RES_K(10), RES_K(5)));

	// The only difference between oscillators A and B is how sync works.
	if (osc_side == osc::A)
		configure_sync_type(va_vco_device::SYNC_TYPE_RESET);
	else if (osc_side == osc::B)
		configure_sync_type(va_vco_device::SYNC_TYPE_REVERSE);
}

cem3374_device::cem3374_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: cem3374_device(mconfig, tag, owner, osc::A, 15.0F, CAP_P(1000))
{
}

cem3374_device &cem3374_device::configure_vthresh(float vthresh)
{
	if (vthresh < 0.0F || vthresh > m_vcc)
		fatalerror("%s: Invalid Vthresh\n", tag());

	m_vthresh = vthresh;
	configure_triangle_range(0, m_vthresh);
	configure_ramp_range(0, 2.0F * m_vthresh);
	return *this;
}

cem3374_device &cem3374_device::configure_pulse_cmp(float src_gain)
{
	m_pulse_src_gain = src_gain;
	return *this;
}

float cem3374_device::v_tempco() const
{
	// The CEM3374 has an on-chip temperature sensor which generates a voltage
	// at pin 10. This can be used for temperature compensation of the
	// oscillator frequency CVs. For now, just returning the nominal value
	// according to the datasheet.
	return 2.5F;
}

void cem3374_device::device_start()
{
	if (std::isnan(freq_ref()) && !BIT(get_sound_requested_inputs_mask(), INPUT_FREQ_REF))
		fatalerror("%s: Frequency reference (freq_ref, aka Iref input) not set\n", tag());

	if (m_pulse_src_gain <= 0.0F && BIT(get_sound_requested_outputs_mask(), OUTPUT_PULSE))
		fatalerror("%s: Pulse output connected but not configured\n", tag());

	va_vco_device::device_start();
}

float cem3374_device::ctrl2freq(float freq_ctrl, float freq_ref) const
{
	// The datasheet does not describe the relationship between the CV,
	// reference current and frequency. Pieced this together from:
	// - The block diagram in the CEM3374 datasheet.
	// - The description in the CEM3340 datasheet.
	// - The schematic of a CEM3374 clone (https://github.com/alanbog/3374-VCO).

	constexpr float VT = 25.7E-3F;  // Thermal voltage constant at 25 deg C.
	const float ieg = freq_ref * expf(freq_ctrl / VT);  // Output current at the exponential converter.
	const float freq = ieg / (2.0F * m_vthresh * m_cf);  // Oscillation frequency.

	LOGMASKED(LOG_FREQ, "%s: FREQ - CV: %f, Iref: %e, freq: %f Hz\n", tag(), freq_ctrl, freq_ref, freq);
	return freq;
}

float cem3374_device::ctrl2pw(float pw_ctrl) const
{
	float max_pw_ctrl = m_vthresh * m_pulse_src_gain;
	if (!pulse_from_tri())
		max_pw_ctrl *= 2.0F;
	const float pw = std::clamp(pw_ctrl / max_pw_ctrl, 0.0F, 1.0F);

	LOGMASKED(LOG_PW, "%s: PW - CV: %f, pw: %f\n", tag(), pw_ctrl, pw);
	return pw;
}

DEFINE_DEVICE_TYPE(CEM3374, cem3374_device, "cem3374", "CEM3374 voltage-controlled oscillator")
