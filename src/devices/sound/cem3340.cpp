// license:BSD-3-Clause
// copyright-holders:m1macrophage

#include "emu.h"
#include "cem3340.h"

namespace {

constexpr float VT = 25.7E-3F;  // Thermal voltage constant at 25 deg C.

// Recommended voltage supply and component values in the datasheet. If a
// design deviates meaningfully from these, they can be made configurable.

constexpr float VCC = 15.0F;  // Positive supply voltage.
constexpr float RS = 1.8E3F;  // Resistor between pin 18 and GND.

constexpr float RT_DEFAULT = 5.6E3F;  // Resistor between pin 2 and pin 3.

constexpr float PW_MAX = VCC / 3.0F;

// See "supplies" section in the datasheet. The minimum for both triangle and
// ramp is always 0.
constexpr float TRIANGLE_MAX = VCC / 3.0F;
constexpr float TRIANGLE_MIN = 0.0F;
constexpr float RAMP_MAX = 2.0F * VCC / 3.0F;
constexpr float RAMP_MIN = 0.0F;

// Computing the exact PULSE max is somewhat involved, and depends on both:
// the pulldown resistor at the PULSE output, and the pulldown voltage (see
// "waveform outputs" section). Using the nominal value provided in the
// "supplies" section instead.
constexpr float PULSE_MAX = VCC - 1.5F;

// Typical configurations pull down to GND. See info in the "waveform outputs"
// section of the datasheet, if you need to make this configurable.
constexpr float PULSE_MIN = 0;

}  // anonymous namespace


cem3340_device::cem3340_device(const machine_config &mconfig, const char *tag, device_t *owner, float cf, float rr)
	: va_vco_device(mconfig, CEM3340, tag, owner, 0)
	, m_cf(cf)
	, m_rr(rr)
	, m_rz(rz_optimal(RT_DEFAULT))
	, m_rt(RT_DEFAULT)
{
	configure_ramp_range(RAMP_MIN, RAMP_MAX);
	configure_pulse_range(PULSE_MIN, PULSE_MAX);
	configure_triangle_range(TRIANGLE_MIN, TRIANGLE_MAX);
	configure_sync_type(SYNC_TYPE_REVERSE);
}

cem3340_device::cem3340_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: cem3340_device(mconfig, tag, owner, 1000E-12F, 1.5E6F)  // Values from the datasheet.
{
}

cem3340_device &cem3340_device::set_tempco_gen_res(float rz, float rt)
{
	if (rz == m_rz && rt == m_rt)
		return *this;
	update_stream();
	m_rz = rz;
	m_rt = rt;
	return *this;
}

float cem3340_device::rz_optimal(float rt)
{
	// According to the datasheet, RZ should be trimmed so that:
	// pin 2 current = pin 1 current. Or: 22 * VT / RT = 3 / RZ
	return 3.0F * rt / (22.0F * VT);
}

float cem3340_device::ctrl2freq(float freq_ctrl) const
{
	// Equations shown and/or described in the datasheet.
	// freq_ctrl is the current into pin 15.
	const float iom = (22.0F * VT / m_rt) * (1.0F - freq_ctrl * m_rz / 3.0F);  // Output current of the multiplier.
	const float vb = iom * RS;  // Voltage at the base of the exponential converter.
	const float iref = VCC / m_rr;  // Reference input current at pin 13.
	const float ieg = iref * expf(-vb / VT);  // Output current of the exponential converter.
	return 3.0F * ieg / (2.0F * VCC * m_cf);  // Oscillation frequency.
}

float cem3340_device::ctrl2pw(float pw_ctrl) const
{
	// pw_ctrl is the control voltage at pin 5.
	return std::clamp(pw_ctrl, 0.0F, PW_MAX) / PW_MAX;
}

void cem3340_device::device_start()
{
	va_vco_device::device_start();

	constexpr u64 SUPPORTED_INPUTS = (1 << INPUT_FREQ_CTRL) | (1 << INPUT_PW_CTRL) | (1 << INPUT_SYNC_FREQ);
	if (get_sound_requested_inputs_mask() & ~SUPPORTED_INPUTS)
	{
		fatalerror("%s: Unsupported inputs connected. Unsupported input mask: %x\n",
				   tag(), get_sound_requested_inputs_mask() & ~SUPPORTED_INPUTS);
	}

	constexpr u64 SUPPORTED_OUTPUTS = (1 << OUTPUT_TRIANGLE) | (1 << OUTPUT_RAMP) | (1 << OUTPUT_PULSE) | (1 << OUTPUT_FREQ);
	if (get_sound_requested_outputs_mask() & ~SUPPORTED_OUTPUTS)
	{
		fatalerror("%s: Unsupported outputs connected. Unsupported output mask: %x\n",
				   tag(), get_sound_requested_outputs_mask() & ~SUPPORTED_OUTPUTS);
	}

	save_item(NAME(m_rz));
	save_item(NAME(m_rt));
}

DEFINE_DEVICE_TYPE(CEM3340, cem3340_device, "cem3340", "CEM3340 Voltage Controlled Oscillator")
