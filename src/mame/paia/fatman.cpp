// license:BSD-3-Clause
// copyright-holders:m1macrophage

/*
The PAiA FatMan is a monophonic analog synthesizer, sold as a kit. It consists
of 2 sawtooth (ramp) oscillators, a resonant 2-pole lowpass VCF (State Variable
topology), and a VCA.

The VCF's cutoff frequency is controlled by a knob, an A(S)R EG, pitch, and
velocity. The VCA is controlled by an ADSR EG and velocity. The depth of these
modulation sources is set using knobs.

The FatMan is controlled by MIDI. It listens to note on and off messages on a
channel specified by dipswitches. It converts those to pitch and velocity
control voltages, and a gate signal. Those are also exposed externally by
corresponding outputs on the unit.

The two control voltages are exponential ("V/Hz"). The 8-bit DAC is configured
to generate voltages in the range 3-6 V (approximately), which the firmware uses
for the top octave of pitches. That voltage can be divided by 2, 4 or 8, using
resistor dividers and a MUX, to generate the voltages needed for lower octaves.
There is only a single DAC, and the voltages for pitch and velocity are sampled
from it by two Sample & Hold circuits controlled by the firmware.

This driver is based on the schematics, user manual and documentation provided
by the manufacturer on their website. This driver cannot replace the real
device. Specifically, there is no attempt to emulate audio. This is just an
educational tool.

The driver emulates the digital functionality of the FatMan, its analog outputs
(Pitch CV, Velocity CV and gate), and all analog functionality that affects the
firmware (the two envelope generators with all their controls, and the "sustain"
and "punch" switces).

Usage:

This driver requires supplying a MIDI input to MAME.
Example:
./mame -listmidi  # List MIDI devices, physical or virtual (e.g. DAWs).
./mame -window fatman -midiin "{midi device}"

The layout will display some LEDs, and the value of the functional knob. But
to actually see control voltage and EG control signals, you need to look at
the error logs (run mame with `-log` and look at error.log).

The knobs for the two EGs can be controlled with the mouse if the layout plugin
is enabled, or the Slider Controls menu. Other knobs are not emulated.

Keep in mind that changing the MIDI channel via dipswitches won't take effect
until a restart or reset (F3).
*/

#include "emu.h"

#include "cpu/mcs51/mcs51.h"
#include "bus/midi/midiinport.h"
#include "bus/midi/midioutport.h"
#include "machine/rescap.h"
#include "sound/va_eg.h"
#include "video/pwm.h"

#include "attotime.h"

#include "paia_fatman.lh"

#define LOG_EG          (1U << 1)
#define LOG_CV          (1U << 2)
#define LOG_DSW         (1U << 3)
#define LOG_CALIBRATION (1U << 4)

#define VERBOSE (LOG_GENERAL | LOG_EG | LOG_CV | LOG_CALIBRATION)
//#define LOG_OUTPUT_FUNC osd_printf_info

#include "logmacro.h"

namespace {

constexpr const char MAINCPU_TAG[] = "8031";

class fatman_state : public driver_device
{
public:
	fatman_state(const machine_config &mconfig, device_type type, const char *tag) ATTR_COLD;

	void fatman(machine_config &config) ATTR_COLD;

	DECLARE_INPUT_CHANGED_MEMBER(vca_eg_knob_adjusted);
	DECLARE_INPUT_CHANGED_MEMBER(vcf_eg_knob_adjusted);
	DECLARE_INPUT_CHANGED_MEMBER(vplus_adjusted);
	DECLARE_INPUT_CHANGED_MEMBER(dac_trimmer_adjusted);
	DECLARE_INPUT_CHANGED_MEMBER(octave_trimmer_adjusted);

protected:
	void machine_start() override ATTR_COLD;
	void machine_reset() override ATTR_COLD;

private:
	static void update_cv_out(float cv, output_finder<2> &cv_out);

	float get_vplus() const;
	float get_eg_ref() const;

	u8 dsw_midi_channel_r() const;
	int midi_rxd_r() const;
	void midi_rxd_w(int state);

	int vca_eg_attack_r() const;
	void vca_eg_decay_w(int state);
	void vca_eg_release_w(int state);
	void update_vca_eg();
	TIMER_CALLBACK_MEMBER(vca_eg_comp_tripped);

	int vcf_eg_attack_r() const;
	void vcf_eg_release_w(int state);
	void update_vcf_eg();

	void cv_dac_w(offs_t offset, u8 data);
	void cv_mux_w(u8 data);
	void pitch_cv_sample_w(int state);
	void velocity_cv_sample_w(int state);

	float compute_cv() const;
	void update_pitch_cv();
	void update_velocity_cv();
	void update_cvs();
	void update_dac_reference();
	void update_octave_calibration();

	void program_map(address_map &map) ATTR_COLD;
	void external_memory_map(address_map &map) ATTR_COLD;

	required_device<mcs51_cpu_device> m_maincpu;
	required_device<va_rc_eg_device> m_vca_eg;
	required_device<va_rc_eg_device> m_vcf_eg;
	required_device<va_rc_eg_device> m_punch_rc;
	required_device<pwm_display_device> m_midi_led_pwm;  // D2
	output_finder<2> m_pitch_cv_out;
	output_finder<2> m_velocity_cv_out;
	output_finder<> m_gate_out;
	output_finder<> m_gate_led;  // D13
	required_ioport m_dsw_io;
	required_ioport m_vca_attack_pot;
	required_ioport m_vca_decay_pot;
	required_ioport m_vca_sustain_pot;
	required_ioport m_vca_release_pot;
	required_ioport m_vca_punch_switch;
	required_ioport m_vcf_attack_pot;
	required_ioport m_vcf_sustain_switch;
	required_ioport m_vcf_release_pot;
	required_ioport m_vplus_config;
	required_ioport m_dac_trimmer;
	required_ioport_array<3> m_octave_trimmers;

	emu_timer *m_vca_eg_comp_timer = nullptr;

	u8 m_midi_rxd_bit = 1; // Initial value is 1, for serial "idle".
	bool m_vca_eg_decay = false;
	bool m_vca_eg_release = false;
	bool m_vcf_eg_release = false;
	u8 m_cv_dac = 0;
	u8 m_cv_mux = 0;
	bool m_sampling_pitch = false;
	bool m_sampling_velocity = false;
	float m_pitch_cv = 0;
	float m_velocity_cv = 0;
	float m_dac_i_fs = 0;
	std::array<float, 4> m_octave_scalers = {1, 1, 1, 1};

	static inline constexpr float VCC = 5;
	static inline constexpr float V_GND = 0;
	static inline constexpr float V_MINUS = -12;
};

fatman_state::fatman_state(const machine_config &mconfig, device_type type, const char *tag)
	: driver_device(mconfig, type, tag)
	, m_maincpu(*this, MAINCPU_TAG)
	, m_vca_eg(*this, "vca_eg_eg")
	, m_vcf_eg(*this, "vcf_eg_eg")
	, m_punch_rc(*this, "vca_punch_rc")
	, m_midi_led_pwm(*this, "midi_led_pwm_device")
	, m_pitch_cv_out(*this, "pitch_cv_out_%u", 0U)
	, m_velocity_cv_out(*this, "velocity_cv_out_%u", 0U)
	, m_gate_out(*this, "gate_out")
	, m_gate_led(*this, "gate_led")
	, m_dsw_io(*this, "dsw")
	, m_vca_attack_pot(*this, "VCA_attack")
	, m_vca_decay_pot(*this, "VCA_decay")
	, m_vca_sustain_pot(*this, "VCA_sustain")
	, m_vca_release_pot(*this, "VCA_release")
	, m_vca_punch_switch(*this, "VCA_punch")
	, m_vcf_attack_pot(*this, "VCF_attack")
	, m_vcf_sustain_switch(*this, "VCF_sustain")
	, m_vcf_release_pot(*this, "VCF_release")
	, m_vplus_config(*this, "VPLUS")
	, m_dac_trimmer(*this, "dac_trimmer")
	, m_octave_trimmers(*this, "octave_%u_trimmer", 1U)
{
}

void fatman_state::update_cv_out(float cv, output_finder<2> &cv_out)
{
	const s32 cv_millis = s32(roundf(1000 * cv));
	cv_out[0] = cv_millis / 1000;
	cv_out[1] = cv_millis % 1000;
}

float fatman_state::get_vplus() const
{
	// See documentation for the "VPLUS" CONFSETTING.
	return BIT(m_vplus_config->read(), 0) ? 9 : 8;
}

float fatman_state::get_eg_ref() const
{
	// Reference voltage for both envelope generator comparators. Those detect
	// the end of the Attack phase.
	constexpr float R85 = RES_R(2200);
	constexpr float R86 = RES_K(10);
	return get_vplus() * RES_VOLTAGE_DIVIDER(R85, R86);
}

u8 fatman_state::dsw_midi_channel_r() const
{
	// Lower 4 bits in reverse order.
	const u8 channel = bitswap<4>(m_dsw_io->read(), 0, 1, 2, 3);
	LOGMASKED(LOG_DSW, "DSW MIDI channel: %d\n", channel + 1);
	return channel;
}

int fatman_state::midi_rxd_r() const
{
	return m_midi_rxd_bit;
}

void fatman_state::midi_rxd_w(int state)
{
	m_midi_rxd_bit = state ? 1 : 0;

	// MIDI IN state is inverted twice (IC7:A and IC7:C) and connected to the
	// cathode of LED D2. So the LED will be on when MIDI IN is low.
	m_midi_led_pwm->write_element(0, 0, state ? 0 : 1);
}

int fatman_state::vca_eg_attack_r() const
{
	// Typical positive- and negative-going thresholds for the 74HC14 Schmitt-
	// trigger inverter with a 5V supply.
	constexpr float HC14_THRESH_POS = 3.0F;
	constexpr float HC14_THRESH_NEG = 1.5F;

	// The heuristic for determining hysteresis is not accurate in the general
	// case (would need to track the voltage stream for that). But it works
	// well in this particular case.
	const bool punch_rc_charging = m_vca_eg->get_v() < get_eg_ref();
	const float threshold = punch_rc_charging ? HC14_THRESH_POS : HC14_THRESH_NEG;

	return (m_punch_rc->get_v() >= threshold) ? 0 : 1;  // 74HC14 is an inverter.
}

void fatman_state::vca_eg_decay_w(int state)
{
	const bool decay = state > 0;
	if (decay == m_vca_eg_decay)
		return;
	m_vca_eg_decay = decay;
	update_vca_eg();
}

void fatman_state::vca_eg_release_w(int state)
{
	const bool release = state > 0;
	if (release == m_vca_eg_release)
		return;
	m_vca_eg_release = release;
	update_vca_eg();

	// LED cathode connected to ground, anode connected to IC7:F inverter,
	// which inverts the 'release' signal. IC7:F output is also connected to the
	// Gate Out socket.
	if (m_vca_eg_release)
	{
		m_gate_led = 0;
		m_gate_out = V_GND;
	}
	else
	{
		m_gate_led = 1;
		m_gate_out = VCC;
	}
}

void fatman_state::update_vca_eg()
{
	// This function only needs to be called on a state change. For instance, if
	// either m_vca_eg_decay or m_vca_eg_release has changed.

	constexpr float POT_R90 = RES_K(1);  // sustain
	constexpr float R91 = RES_R(100);
	constexpr float POT_R92 = RES_M(1);  // decay
	constexpr float R93 = RES_R(10);
	constexpr float POT_R94 = RES_M(1);  // attack
	constexpr float R95 = RES_R(100);
	constexpr float POT_R96 = RES_M(1);  // release

	if (m_vca_eg_decay && m_vca_eg_release)  // release
	{
		const float r96 = m_vca_release_pot->read() * POT_R96 / 100.0F;
		m_vca_eg->set_r(r96 + R95).set_target_v(V_GND);
	}
	else if (m_vca_eg_decay)  // decay
	{
		const float sustain_v = get_vplus() * m_vca_sustain_pot->read() / 100.0F;
		const float current_v = m_vca_eg->get_v();
		const float r92 = m_vca_decay_pot->read() * POT_R92 / 100.0F;
		m_vca_eg->set_r(r92 + R91).set_target_v(std::min(sustain_v, current_v));
	}
	else if (m_vca_eg_release)  // "invalid" state
	{
		// release == 1 and decay == 0 is not a useful EG state. But it is
		// emulated for completeness. The firmware enters it transiently when
		// switching from Attack to Release mode.
		const float r94 = m_vca_attack_pot->read() * POT_R94 / 100.0F;
		const float r_attack = POT_R90 + R93 + r94;
		const float r96 = m_vca_release_pot->read() * POT_R96 / 100.0F;
		const float r_release = r96 + R95;
		const float target_v = get_vplus() * RES_VOLTAGE_DIVIDER(r_attack, r_release);
		const float effective_r = RES_2_PARALLEL(r_attack, r_release);
		m_vca_eg->set_r(effective_r).set_target_v(target_v);
	}
	else  // attack
	{
		const float r94 = m_vca_attack_pot->read() * POT_R94 / 100.0F;
		m_vca_eg->set_r(POT_R90 + R93 + r94).set_target_v(get_vplus());
	}

	// Determine when the EG comparator will trip. The comparator detects when
	// the attack phase has completed, by comparing the EG voltage with a fixed
	// reference.
	const float comp_ref = get_eg_ref();
	const bool attacked = (m_vca_eg->get_v() >= comp_ref);
	const attotime comp_trip_dt = m_vca_eg->get_dt(comp_ref);
	if (comp_trip_dt.is_never())
	{
		m_vca_eg_comp_timer->reset();
		vca_eg_comp_tripped(attacked ? 1 : 0);
	}
	else
	{
		// Schedule a flip of the "attacked" state (see vca_eg_comp_tripped()).
		m_vca_eg_comp_timer->adjust(comp_trip_dt, attacked ? 0 : 1);
	}

	LOGMASKED(LOG_EG, "VCA EG decay: %d, release: %d, attacked: %d, trip dt: %g\n",
	          m_vca_eg_decay, m_vca_eg_release, attacked, comp_trip_dt.as_double());
}

// `param` specifies whether the VCA EG voltage has exceeded the EG reference.
// A non-zero value signifies that the Attack phase completed. `param` is the
// negated output of comparator IC8:D (LM339).
TIMER_CALLBACK_MEMBER(fatman_state::vca_eg_comp_tripped)
{
	const bool punch_enabled = !BIT(m_vca_punch_switch->read(), 0);
	if (punch_enabled)
	{
		// Enabling the "punch" switch activates an RC circuit between the VCA
		// EG Attack detection comparator, and the 74HC14 inverter that buffers
		// the comparator output for the MCU. This results in an additional
		// delay before the MCU is informed that the Attack phase is complete.
		if (param)
			m_punch_rc->set_target_v(V_GND).set_r(RES_K(330));  // R98
		else
			m_punch_rc->set_target_v(VCC).set_r(RES_K(10));  // R101
	}
	else
	{
		// When "punch" is disabled, the capacitor (C34) of the RC circuit is
		// disconnected from ground. So voltage changes on the other side of
		// the capacitor are instant.
		if (param)
			m_punch_rc->set_instant_v(V_GND);
		else
			m_punch_rc->set_instant_v(VCC);
	}
	LOGMASKED(LOG_EG, "VCA EG comparator tripped: %d, punch: %d\n", param, punch_enabled);
}

int fatman_state::vcf_eg_attack_r() const
{
	const bool sustain_enabled = !BIT(m_vcf_sustain_switch->read(), 0);
	if (sustain_enabled)
	{
		// When the sustain switch (S3) is "on" (i.e. grounded), the MCU never
		// receives a 1, so the firmware doesn't know that the EG attack
		// completed, and it doesn't start the EG release.
		return 0;
	}
	return (m_vcf_eg->get_v() >= get_eg_ref()) ? 1 : 0;
}

void fatman_state::vcf_eg_release_w(int state)
{
	const bool release = state > 0;
	if (release == m_vcf_eg_release)
		return;
	m_vcf_eg_release = release;
	update_vcf_eg();
}

void fatman_state::update_vcf_eg()
{
	constexpr float R80 = RES_K(1);
	constexpr float R81 = RES_R(100);
	constexpr float POT_R82 = RES_M(1);  // release
	constexpr float R83 = RES_R(100);
	constexpr float POT_R84 = RES_M(1);  // attack

	if (m_vcf_eg_release)  // release
	{
		const float r82 = m_vcf_release_pot->read() * POT_R82 / 100.0;
		m_vcf_eg->set_r(r82 + R81).set_target_v(V_GND);
	}
	else  // attack
	{
		const float r84 = m_vcf_attack_pot->read() * POT_R84 / 100.0;
		m_vcf_eg->set_r(R80 + R83 + r84).set_target_v(get_vplus());
	}
	LOGMASKED(LOG_EG, "VCF EG release: %d\n", m_vcf_eg_release);
}

void fatman_state::cv_dac_w(offs_t offset, u8 data)
{
	if (m_cv_dac == data)
		return;
	m_cv_dac = data;
	update_cvs();
}

void fatman_state::cv_mux_w(u8 data)
{
	// P3.4 and P3.5 are level-shifted and inverted by Q1 and Q2, and used
	// as the control inputs (high and low order bits, respectively) of the
	// 4052 MUX (IC9).
	const u8 mux = bitswap<2>(~data, 4, 5);
	if (m_cv_mux == mux)
		return;
	m_cv_mux = mux;
	update_cvs();
}

void fatman_state::pitch_cv_sample_w(int state)
{
	const bool old_state = m_sampling_pitch;
	// P1.0 is level-shifted with a comparator (without inverting), and
	// connected to 4016 'control A' (active high).
	m_sampling_pitch = state > 0;
	if (!old_state && state)
		update_pitch_cv();
}

void fatman_state::velocity_cv_sample_w(int state)
{
	const bool old_state = m_sampling_velocity;
	// Same as pitch_cv_sample_w above, but for P1.1 and 4016 'control B'
	m_sampling_velocity = state > 0;
	if (!old_state && state)
		update_velocity_cv();
}

float fatman_state::compute_cv() const
{
	// See file comments at the top for more info on the CV generation strategy.

	// Compute the offset current flowing away from the inverting input of the
	// current-to-voltage (I2V) converter (IC10:B, LM324).
	constexpr float R12 = RES_R(4700);
	constexpr float R14 = RES_K(15);
	constexpr float I_OFFSET_ABS = -V_MINUS / (R12 + R14);

	// Compute DAC Iout. This flows into the DAC and away from the inverting
	// input of the I2V.
	const float dac_i_out_abs = m_dac_i_fs * m_cv_dac / 255.0F;

	// Convert current to voltage.
	constexpr float R15 = RES_R(4700);
	const float v = (dac_i_out_abs + I_OFFSET_ABS) * R15;

	// Scale voltage according to the selected octave. The scaler for the top
	// octave is 1.
	assert(m_cv_mux >= 0 && m_cv_mux < 4);
	return v * m_octave_scalers[3 - m_cv_mux];
}

void fatman_state::update_pitch_cv()
{
	if (!m_sampling_pitch)
		return;

	const float new_cv = compute_cv();
	if (new_cv == m_pitch_cv)
		return;

	m_pitch_cv = new_cv;
	update_cv_out(m_pitch_cv, m_pitch_cv_out);
	LOGMASKED(LOG_CV, "Pitch CV %f\n", m_pitch_cv);
}

void fatman_state::update_velocity_cv()
{
	if (!m_sampling_velocity)
		return;

	const float new_cv = compute_cv();
	if (new_cv == m_velocity_cv)
		return;

	m_velocity_cv = new_cv;
	update_cv_out(m_velocity_cv, m_velocity_cv_out);
	LOGMASKED(LOG_CV, "Velocity CV %f\n", m_velocity_cv);
}

void fatman_state::update_cvs()
{
	update_pitch_cv();
	update_velocity_cv();
}

void fatman_state::update_dac_reference()
{
	constexpr float R13_POT_MAX = RES_K(10);
	constexpr float R11 = RES_R(6800);

	const float r13 = R13_POT_MAX * (100 - m_dac_trimmer->read()) / 100.0F;
	const float i_ref = get_vplus() / (r13 + R11);

	// Compute the absolute value of the max ("full scale") output current using
	// the formula in the DAC08 datasheet.
	m_dac_i_fs = i_ref * 255.0 / 256.0;
	update_cvs();

	LOGMASKED(LOG_CALIBRATION, "DAC iFS updated: %f\n", m_dac_i_fs);
}

// Returns the voltage proportion at different points on the resistive divider
// ladder. This ladder is used for scaling down the CV to lower octaves.
constexpr std::array<float, 6> octave_resistor_ladder_v()
{
	constexpr float RESISTOR_LADDER[7] =
	{
		RES_R(100),  // R26
		RES_2_PARALLEL(RES_R(56), RES_K(1)),  // R25, R24 pot
		RES_R(47),  // R23
		RES_2_PARALLEL(RES_R(120), RES_K(1)),  // R22, R21 pot
		RES_R(100),  // R20
		RES_2_PARALLEL(RES_R(270), RES_K(1)),  // R19, R18 pot
		RES_R(390),  // R17
	};

	// Compute cumulative resistance in both directions.
	std::array<float, 6> r_lower{};
	std::array<float, 6> r_upper{};
	r_lower[0] = RESISTOR_LADDER[0];
	r_upper[5] = RESISTOR_LADDER[6];
	for (int i = 1; i < 6; ++i)
	{
		r_lower[i] = r_lower[i - 1] + RESISTOR_LADDER[i];
		r_upper[5 - i] = r_upper[6 - i] + RESISTOR_LADDER[6 - i];
	}

	std::array<float, 6> ladder_v{};
	for (int i = 0; i < 6; ++i)
		ladder_v[i] = RES_VOLTAGE_DIVIDER(r_upper[i], r_lower[i]);
	return ladder_v;
}

void fatman_state::update_octave_calibration()
{
	constexpr std::array<float, 6> ladder_v = octave_resistor_ladder_v();
	for (int octave = 0; octave < 3; ++octave)
	{
		const float v_low = ladder_v[2 * octave];
		const float v_high = ladder_v[2 * octave + 1];
		const float trimmer = m_octave_trimmers[octave]->read() / 100.0F;
		m_octave_scalers[octave] = v_low + trimmer * (v_high - v_low);
	}
	m_octave_scalers[3] = 1.0F;
	update_cvs();

	LOGMASKED(LOG_CALIBRATION, "Octave calibration updated: ");
	for (int i = 0; i < m_octave_scalers.size(); ++i)
		LOGMASKED(LOG_CALIBRATION, "%d: %f  ", i, m_octave_scalers[i]);
	LOGMASKED(LOG_CALIBRATION, "\n");
}

void fatman_state::program_map(address_map &map)
{
	// A13-A15 are not connected.
	map(0x0000, 0x1fff).mirror(0xe000).rom().region(MAINCPU_TAG, 0);
}

void fatman_state::external_memory_map(address_map &map)
{
	// Address lines ignored on external memory writes.
	map(0x0000, 0x0000).mirror(0xffff).w(FUNC(fatman_state::cv_dac_w));
}

void fatman_state::machine_start()
{
	save_item(NAME(m_midi_rxd_bit));
	save_item(NAME(m_vca_eg_decay));
	save_item(NAME(m_vca_eg_release));
	save_item(NAME(m_vcf_eg_release));
	save_item(NAME(m_cv_dac));
	save_item(NAME(m_cv_mux));
	save_item(NAME(m_sampling_pitch));
	save_item(NAME(m_sampling_velocity));
	save_item(NAME(m_pitch_cv));
	save_item(NAME(m_velocity_cv));
	save_item(NAME(m_dac_i_fs));
	save_item(NAME(m_octave_scalers));

	m_pitch_cv_out.resolve();
	m_velocity_cv_out.resolve();
	m_gate_out.resolve();
	m_gate_led.resolve();

	m_vca_eg_comp_timer = timer_alloc(FUNC(fatman_state::vca_eg_comp_tripped), this);
}

void fatman_state::machine_reset()
{
	update_dac_reference();
	update_octave_calibration();
	update_vca_eg();
	update_vcf_eg();
}

void fatman_state::fatman(machine_config &config)
{
	I8031(config, m_maincpu, 12_MHz_XTAL);
	m_maincpu->set_addrmap(AS_PROGRAM, &fatman_state::program_map);
	m_maincpu->set_addrmap(AS_IO, &fatman_state::external_memory_map);

	// The `set_constant()`s on inputs 1 and 3 ensure that those inputs, when
	// read, will return whatever was written to them (the input is
	// essentially ANDed to the value that was written to the port. See mcs51
	// implementation). The firmware depends on that functionality for at least
	// some of these inputs.

	m_maincpu->port_in_cb<1>().set_constant(0x0f).mask(0x0f);
	m_maincpu->port_in_cb<1>().append(FUNC(fatman_state::dsw_midi_channel_r)).lshift(4).mask(0xf0);

	m_maincpu->port_out_cb<1>().set(FUNC(fatman_state::pitch_cv_sample_w)).bit(0);
	m_maincpu->port_out_cb<1>().append(FUNC(fatman_state::velocity_cv_sample_w)).bit(1);
	m_maincpu->port_out_cb<1>().append(FUNC(fatman_state::vca_eg_decay_w)).bit(2);
	m_maincpu->port_out_cb<1>().append(FUNC(fatman_state::vca_eg_release_w)).bit(3);

	m_maincpu->port_in_cb<3>().set_constant(0x72).mask(0x72);  // Bits 1, 4, 5, 6.
	m_maincpu->port_in_cb<3>().append(FUNC(fatman_state::midi_rxd_r)).mask(0x01);
	m_maincpu->port_in_cb<3>().append(FUNC(fatman_state::vca_eg_attack_r)).lshift(2).mask(0x04);
	m_maincpu->port_in_cb<3>().append(FUNC(fatman_state::vcf_eg_attack_r)).lshift(3).mask(0x08);
	m_maincpu->port_in_cb<3>().append_ioport("dsw").mask(0x80);

	m_maincpu->port_out_cb<3>().set(FUNC(fatman_state::vcf_eg_release_w)).bit(1);
	m_maincpu->port_out_cb<3>().append(FUNC(fatman_state::cv_mux_w)).mask(0x30);  // Bits 4, 5.

	VA_RC_EG(config, m_vca_eg).set_c(CAP_U(2.2));  // C19
	VA_RC_EG(config, m_vcf_eg).set_c(CAP_U(2.2));  // C22
	VA_RC_EG(config, m_punch_rc).set_c(CAP_U(0.05));  // C34

	midi_port_device &midi_in(MIDI_PORT(config, "mdin", midiin_slot, "midiin"));
	MIDI_PORT(config, "mdthru", midiout_slot, "midiout");
	midi_in.rxd_handler().set(FUNC(fatman_state::midi_rxd_w));
	midi_in.rxd_handler().append("mdthru", FUNC(midi_port_device::write_txd));

	PWM_DISPLAY(config, m_midi_led_pwm).set_size(1, 1);
	m_midi_led_pwm->output_x().set_output("midi_led");
	m_midi_led_pwm->set_interpolation(0.2);
	m_midi_led_pwm->set_bri_levels(0.0001);
	m_midi_led_pwm->set_refresh(attotime::from_hz(30));

	config.set_default_layout(layout_paia_fatman);
}

DECLARE_INPUT_CHANGED_MEMBER(fatman_state::vca_eg_knob_adjusted)
{
	update_vca_eg();
}

DECLARE_INPUT_CHANGED_MEMBER(fatman_state::vcf_eg_knob_adjusted)
{
	update_vcf_eg();
}

DECLARE_INPUT_CHANGED_MEMBER(fatman_state::vplus_adjusted)
{
	update_vca_eg();
	update_vcf_eg();
	update_dac_reference();
}

DECLARE_INPUT_CHANGED_MEMBER(fatman_state::dac_trimmer_adjusted)
{
	update_dac_reference();
}

DECLARE_INPUT_CHANGED_MEMBER(fatman_state::octave_trimmer_adjusted)
{
	update_octave_calibration();
}

INPUT_PORTS_START(fatman)
	PORT_START("dsw")
	PORT_DIPNAME(0x0f, 0x00, "MIDI Channel") PORT_DIPLOCATION("SW1:1,2,3,4")
	PORT_DIPSETTING(   0x00, "1")
	PORT_DIPSETTING(   0x08, "2")
	PORT_DIPSETTING(   0x04, "3")
	PORT_DIPSETTING(   0x0c, "4")
	PORT_DIPSETTING(   0x02, "5")
	PORT_DIPSETTING(   0x0a, "6")
	PORT_DIPSETTING(   0x06, "7")
	PORT_DIPSETTING(   0x0e, "8")
	PORT_DIPSETTING(   0x01, "9")
	PORT_DIPSETTING(   0x09, "10")
	PORT_DIPSETTING(   0x05, "11")
	PORT_DIPSETTING(   0x0d, "12")
	PORT_DIPSETTING(   0x03, "13")
	PORT_DIPSETTING(   0x0b, "14")
	PORT_DIPSETTING(   0x07, "15")
	PORT_DIPSETTING(   0x0f, "16")
	PORT_DIPNAME(0x70, 0x00, "Not Connected") PORT_DIPLOCATION("SW1:5,6,7")
	PORT_DIPNAME(0x80, 0x00, DEF_STR(Unused)) PORT_DIPLOCATION("SW1:8")
	PORT_DIPSETTING(   0x00, DEF_STR(On))
	PORT_DIPSETTING(   0x80, DEF_STR(Off))

	// VCA EG Controls.
	PORT_START("VCA_attack")   // POT_R94
	PORT_ADJUSTER(2, "VCA EG Attack") PORT_CHANGED_MEMBER(DEVICE_SELF, FUNC(fatman_state::vca_eg_knob_adjusted), 0)
	PORT_START("VCA_decay")    // POT_R92
	PORT_ADJUSTER(1, "VCA EG Decay") PORT_CHANGED_MEMBER(DEVICE_SELF, FUNC(fatman_state::vca_eg_knob_adjusted), 0)
	PORT_START("VCA_sustain")  // POT_R90
	PORT_ADJUSTER(50, "VCA EG Sustain") PORT_CHANGED_MEMBER(DEVICE_SELF, FUNC(fatman_state::vca_eg_knob_adjusted), 0)
	PORT_START("VCA_release")  // POT_R96
	PORT_ADJUSTER(3, "VCA EG Release") PORT_CHANGED_MEMBER(DEVICE_SELF, FUNC(fatman_state::vca_eg_knob_adjusted), 0)
	PORT_START("VCA_punch")  // S1
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("VCA Punch") PORT_CODE(KEYCODE_P) PORT_TOGGLE
		PORT_CHANGED_MEMBER(DEVICE_SELF, FUNC(fatman_state::vca_eg_knob_adjusted), 0)

	// VCF EG Controls.
	PORT_START("VCF_attack")   // POT_R82
	PORT_ADJUSTER(1, "VCF EG Attack") PORT_CHANGED_MEMBER(DEVICE_SELF, FUNC(fatman_state::vcf_eg_knob_adjusted), 0)
	PORT_START("VCF_release")  // POT_R84
	PORT_ADJUSTER(30, "VCF EG Release") PORT_CHANGED_MEMBER(DEVICE_SELF, FUNC(fatman_state::vcf_eg_knob_adjusted), 0)
	PORT_START("VCF_sustain")  // S3
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("VCF EG Sustain") PORT_CODE(KEYCODE_S) PORT_TOGGLE

	// Early devices had an 8V positive supply. This was changed to 9V at some
	// point. The negative supply is -12V. Interestingly, using a 9V positive
	// supply exceeds the MUX's (IC9, CD4052) Absolute Maximum Rating for supply
	// voltage (20V).
	PORT_START("VPLUS")
	PORT_CONFNAME(0x01, 0x01, "V+") PORT_CHANGED_MEMBER(DEVICE_SELF, FUNC(fatman_state::vplus_adjusted), 0)
	PORT_CONFSETTING(0x00, "8V (older devices)")
	PORT_CONFSETTING(0x01, "9V (newer devices)")

	// The default value is based on the calibration instructions in the user
	// manual. A DAC value of 0xff should generate 2x the CV compared to a DAC
	// value of 0x00, with a small error due to adjuster resolution. Calibration
	// was done with a 9V supply. The value will be different for an 8V supply.
	PORT_START("dac_trimmer")
	PORT_ADJUSTER(21, "DAC tune") PORT_CHANGED_MEMBER(DEVICE_SELF, FUNC(fatman_state::dac_trimmer_adjusted), 0)

	// All octave trimmers are 1K potentiometers.
	// Default values are for a correct tuning relative to octave 4, with a
	// small error due to adjuster resolution.
	PORT_START("octave_1_trimmer")
	PORT_ADJUSTER(49, "Octave 1 trimmer (R24)") PORT_CHANGED_MEMBER(DEVICE_SELF, FUNC(fatman_state::octave_trimmer_adjusted), 0)
	PORT_START("octave_2_trimmer")
	PORT_ADJUSTER(49, "Octave 2 trimmer (R21)") PORT_CHANGED_MEMBER(DEVICE_SELF, FUNC(fatman_state::octave_trimmer_adjusted), 0)
	PORT_START("octave_3_trimmer")
	PORT_ADJUSTER(46, "Octave 3 trimmer (R18)") PORT_CHANGED_MEMBER(DEVICE_SELF, FUNC(fatman_state::octave_trimmer_adjusted), 0)
INPUT_PORTS_END

ROM_START(fatman)
	ROM_REGION(0x2000, MAINCPU_TAG, 0)
	ROM_DEFAULT_BIOS("fatman9")
	ROM_SYSTEM_BIOS(0, "fatman9", "FatMan Operating System 1.9 (C) 1995")
	ROMX_LOAD("fatmopv1.9.ic3", 0x000000, 0x002000, CRC(e2a4b9a9) SHA1(3738f0c9b3b158884fddb103b19314899d7e60c0), ROM_BIOS(0))
ROM_END

}  // anonymous namespace

SYST(1992, fatman, 0, 0, fatman, fatman, fatman_state, empty_init, "PAiA Electronics", "FatMan", MACHINE_SUPPORTS_SAVE | MACHINE_NOT_WORKING | MACHINE_NO_SOUND)
