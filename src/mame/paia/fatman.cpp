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
to generate voltages in the range 3-6 V, which the firmware uses for the top
octave of pitches. That voltage can be divided by 2, 4 or 8, using resistor
dividers and a MUX, to generate the voltages needed for lower octaves. There is
only a single DAC, and the voltages for pitch and velocity are sampled from it
by two Sample & Hold circuits controlled by the firmware.

This driver is based on the schematics, user manual and documentation provided
by the manufacturer on their website. This driver cannot replace the real
device. Specifically, there is no attempt to emulate audio. This is just an
educational tool.

The driver emulates the digital functionality of FatMan, the analog outputs (CV
and gate), and most of the analog functionality that has an effect on the
firmware (the two envelope generators with all their controls, and the "sustain"
switch. The "punch" switch is not yet emulated).

Usage:

This driver requires supplying a MIDI input to MAME.
Example:
./mame -listmidi  # List MIDI devices, physical or virtual (e.g. DAWs).
./mame -window fatman -midiin "{midi device}"

The layout will display some LEDs, and the value of the functional knob. But
to actually see control voltage and EG control signals, you need to look at
the error logs (run mame with `-log` and look at error.log).

The knobs for the two EGs can be controlled by the Slider Control menu. Other
knobs are not emulated.

Keep in mind that changing the MIDI channel via dipswitches won't take effect
until a restart or reset (F3).
*/

#include "emu.h"

#include "cpu/mcs51/mcs51.h"
#include "bus/midi/midiinport.h"
#include "bus/midi/midioutport.h"
#include "machine/rescap.h"
#include "video/pwm.h"

#include "attotime.h"

#include "paia_fatman.lh"

#define LOG_EG  (1U << 1)
#define LOG_CV  (1U << 2)
#define LOG_DSW (1U << 3)

#define VERBOSE (LOG_GENERAL | LOG_EG | LOG_CV)
//#define LOG_OUTPUT_FUNC osd_printf_info

#include "logmacro.h"

// Emulates a (DC) RC circuit with a variable resistance. Useful for emulating
// envelope generators. This is not a simulation, just uses the standard RC
// equations to return voltage at a specific time.
class fatman_rc_network_state_device : public device_t
{
public:
	fatman_rc_network_state_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock = 0) ATTR_COLD;

	void set_c(float c);
	void reset(float v_target, float r, const attotime &t);
	float get_v(const attotime &t) const;

protected:
	void device_start() override ATTR_COLD;

private:
	// Initialize to a slow (large RC) but valid charging state.
	float m_c = CAP_U(1000);
	float m_r = RES_M(100);
	float m_v_start = 0;
	float m_v_end = 1;
	attotime m_start_t;
};

DEFINE_DEVICE_TYPE(FATMAN_RC_NETWORK_STATE, fatman_rc_network_state_device, "fatman_rc_network_state", "Fatman EG RC network");

fatman_rc_network_state_device::fatman_rc_network_state_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, FATMAN_RC_NETWORK_STATE, tag, owner, clock)
{
}

void fatman_rc_network_state_device::device_start()
{
	save_item(NAME(m_c));
	save_item(NAME(m_r));
	save_item(NAME(m_v_start));
	save_item(NAME(m_v_end));
	save_item(NAME(m_start_t));
}

void fatman_rc_network_state_device::set_c(float c)
{
	m_c = c;
}

void fatman_rc_network_state_device::reset(float v_target, float r, const attotime &t)
{
	m_v_start = get_v(t);
	m_v_end = v_target;
	m_r = r;
	m_start_t = t;
}

float fatman_rc_network_state_device::get_v(const attotime &t) const
{
	assert(t >= m_start_t);
	const attotime delta_t = t - m_start_t;
	return m_v_start + (m_v_end - m_v_start) * (1.0F - expf(-delta_t.as_double() / (m_r * m_c)));
}


namespace {

constexpr const char MAINCPU_TAG[] = "8031";

class fatman_state : public driver_device
{
public:
	fatman_state(const machine_config &mconfig, device_type type, const char *tag) ATTR_COLD
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, MAINCPU_TAG)
		, m_vca_adsr_state(*this, "vca_dsr_eg")
		, m_vcf_ar_state(*this, "vcf_ar_eg")
		, m_midi_led_pwm(*this, "midi_led_pwm_device")
		, m_gate_led(*this, "gate_led")
		, m_dsw_io(*this, "dsw")
		, m_vca_attack_pot(*this, "VCA_attack")
		, m_vca_decay_pot(*this, "VCA_decay")
		, m_vca_sustain_pot(*this, "VCA_sustain")
		, m_vca_release_pot(*this, "VCA_release")
		, m_vcf_attack_pot(*this, "VCF_attack")
		, m_vcf_sustain_switch(*this, "VCF_sustain")
		, m_vcf_release_pot(*this, "VCF_release")
	{
	}

	void fatman(machine_config &config) ATTR_COLD;

protected:
	void machine_start() override ATTR_COLD;

private:
	u8 dsw_midi_channel_r() const;
	int midi_rxd_r() const;
	void midi_rxd_w(int state);

	int vca_adsr_attack_r() const;
	void vca_adsr_decay_w(int state);
	void vca_adsr_release_w(int state);
	void update_vca_adsr_state();

	int vcf_ar_attack_r() const;
	void vcf_ar_release_w(int state);

	void cv_dac_w(offs_t offset, u8 data);
	void cv_mux_w(u8 data);
	void pitch_cv_sample_w(int state);
	void velocity_cv_sample_w(int state);

	float compute_cv() const;
	void update_pitch_cv();
	void update_velocity_cv();
	void update_cvs();

	void program_map(address_map &map) ATTR_COLD;
	void external_memory_map(address_map &map) ATTR_COLD;

	required_device<mcs51_cpu_device> m_maincpu;
	required_device<fatman_rc_network_state_device> m_vca_adsr_state;
	required_device<fatman_rc_network_state_device> m_vcf_ar_state;
	required_device<pwm_display_device> m_midi_led_pwm;  // D2
	output_finder<> m_gate_led;  // D13
	required_ioport m_dsw_io;
	required_ioport m_vca_attack_pot;
	required_ioport m_vca_decay_pot;
	required_ioport m_vca_sustain_pot;
	required_ioport m_vca_release_pot;
	required_ioport m_vcf_attack_pot;
	required_ioport m_vcf_sustain_switch;
	required_ioport m_vcf_release_pot;

	u8 m_midi_rxd_bit = 1; // Initial value is 1, for serial "idle".
	bool m_vca_adsr_decay = false;
	bool m_vca_adsr_release = false;
	bool m_vcf_ar_release = false;
	u8 m_cv_dac = 0;
	u8 m_cv_mux = 0;
	bool m_sampling_pitch = false;
	bool m_sampling_velocity = false;
	float m_pitch_cv = 0;
	float m_velocity_cv = 0;

	static constexpr const float V_PLUS = 9;
	static constexpr const float V_GND = 0;
	static constexpr const float V_MINUS = -12;

	// Voltage reference for both EG comparators.
	static constexpr const float R85 = RES_R(2200);
	static constexpr const float R86 = RES_K(10);
	static constexpr const float V_EG_COMP = V_PLUS * RES_VOLTAGE_DIVIDER(R85, R86);

	// VCA ADSR components.
	static constexpr const float POT_R90 = RES_K(1);  // Sustain potentiometer.
	static constexpr const float R91 = RES_R(100);
	static constexpr const float POT_R92 = RES_M(1);  // Decay rheostat.
	static constexpr const float R93 = RES_R(10);
	static constexpr const float POT_R94 = RES_M(1);  // Attack rheostat.
	static constexpr const float R95 = RES_R(100);
	static constexpr const float POT_R96 = RES_M(1);  // Release rheostat.
	static constexpr const float C19 = CAP_U(2.2);

	// VCF A(S)R components.
	static constexpr const float R80 = RES_K(1);
	static constexpr const float R81 = RES_R(100);
	static constexpr const float POT_R82 = RES_M(1);  // Release rheostat.
	static constexpr const float R83 = RES_R(100);
	static constexpr const float POT_R84 = RES_M(1);  // Attack rheostat.
	static constexpr const float C22 = CAP_U(2.2);
};

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

int fatman_state::vca_adsr_attack_r() const
{
	const float v = m_vca_adsr_state->get_v(machine().time());
	return (v >= V_EG_COMP) ? 1 : 0;
}

void fatman_state::vca_adsr_decay_w(int state)
{
	const bool decay = state > 0;
	if (m_vca_adsr_decay == decay)
		return;

	m_vca_adsr_decay = decay;
	LOGMASKED(LOG_EG, "vca decay: %d\n", m_vca_adsr_decay);
	update_vca_adsr_state();
}

void fatman_state::vca_adsr_release_w(int state)
{
	const bool release = state > 0;
	if (m_vca_adsr_release == release)
		return;

	m_vca_adsr_release = release;
	LOGMASKED(LOG_EG, "vca release: %d\n", m_vca_adsr_release);

	// LED cathode connected to ground, anode connected to IC7:F inverter,
	// which inverts the 'release' signal.
	m_gate_led = m_vca_adsr_release ? 0 : 1;
	update_vca_adsr_state();
}

void fatman_state::update_vca_adsr_state()
{
	// This function only needs to be called on a state change. For instance, if
	// either m_vca_adsr_decay or m_vca_adsr_release has changed.

	if (m_vca_adsr_decay && m_vca_adsr_release)  // Release.
	{
		const float r96 = m_vca_release_pot->read() * POT_R96 / 100.0F;
		m_vca_adsr_state->reset(V_GND, r96 + R95, machine().time());
	}
	else if (m_vca_adsr_decay)  // Decay.
	{
		const attotime now = machine().time();
		const float sustain_v = V_PLUS * m_vca_sustain_pot->read() / 100.0F;
		const float current_v = m_vca_adsr_state->get_v(now);
		const float r92 = m_vca_decay_pot->read() * POT_R92 / 100.0F;
		m_vca_adsr_state->reset(std::min(sustain_v, current_v), r92 + R91, now);
	}
	else if (m_vca_adsr_release)  // "Invalid" state.
	{
		// release == 1 and decay == 0 is not a useful EG state. But it is
		// emulated for completeness. The firmware enters it transiently when
		// switching from Attack to Release mode.
		const float r94 = m_vca_attack_pot->read() * POT_R94 / 100.0F;
		const float r_attack = POT_R90 + R93 + r94;
		const float r96 = m_vca_release_pot->read() * POT_R96 / 100.0F;
		const float r_release = r96 + R95;
		const float target_v = V_PLUS * RES_VOLTAGE_DIVIDER(r_attack, r_release);
		const float effective_r = RES_2_PARALLEL(r_attack, r_release);
		m_vca_adsr_state->reset(target_v, effective_r, machine().time());
	}
	else // Attack.
	{
		const float r94 = m_vca_attack_pot->read() * POT_R94 / 100.0F;
		m_vca_adsr_state->reset(V_PLUS, POT_R90 + R93 + r94, machine().time());
	}
}

int fatman_state::vcf_ar_attack_r() const
{
	const bool sustain_enabled = !(m_vcf_sustain_switch->read() & 0x01);
	if (sustain_enabled)
	{
		// When the sustain switch (S3) is "on" (i.e. grounded), the MCU never
		// receives a 1, so the firmware doesn't know that the EG attack
		// completed, and it doesn't start the EG release.
		return 0;
	}
	const float v = m_vcf_ar_state->get_v(machine().time());
	return (v >= V_EG_COMP) ? 1 : 0;
}

void fatman_state::vcf_ar_release_w(int state)
{
	const bool release = state > 0;
	if (m_vcf_ar_release == release)
		return;

	m_vcf_ar_release = release;
	LOGMASKED(LOG_EG, "vcf release: %d\n", m_vcf_ar_release);

	if (m_vcf_ar_release)  // Start release.
	{
		const float r82 = m_vcf_release_pot->read() * POT_R82 / 100.0;
		m_vcf_ar_state->reset(V_GND, r82 + R81, machine().time());
	}
	else // Start attack.
	{
		const float r84 = m_vcf_attack_pot->read() * POT_R84 / 100.0;
		m_vcf_ar_state->reset(V_PLUS, R80 + R83 + r84, machine().time());
	}
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
	static constexpr float MAX_CV = 6;
	static constexpr float HALF_CV = MAX_CV / 2;
	static constexpr float MUX_DIVISOR[4] = {1, 2, 4, 8};

	assert(m_cv_mux >= 0 && m_cv_mux < 4);
	return (HALF_CV + m_cv_dac * HALF_CV / 255) / MUX_DIVISOR[m_cv_mux];
}

void fatman_state::update_pitch_cv()
{
	if (!m_sampling_pitch)
		return;

	const float new_cv = compute_cv();
	if (new_cv == m_pitch_cv)
		return;

	m_pitch_cv = new_cv;
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
	LOGMASKED(LOG_CV, "Velocity CV %f\n", m_velocity_cv);
}

void fatman_state::update_cvs()
{
	update_pitch_cv();
	update_velocity_cv();
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
	m_gate_led.resolve();
	save_item(NAME(m_midi_rxd_bit));
	save_item(NAME(m_vca_adsr_decay));
	save_item(NAME(m_vca_adsr_release));
	save_item(NAME(m_vcf_ar_release));
	save_item(NAME(m_cv_dac));
	save_item(NAME(m_cv_mux));
	save_item(NAME(m_sampling_pitch));
	save_item(NAME(m_sampling_velocity));
	save_item(NAME(m_pitch_cv));
	save_item(NAME(m_velocity_cv));
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
	m_maincpu->port_out_cb<1>().append(FUNC(fatman_state::vca_adsr_decay_w)).bit(2);
	m_maincpu->port_out_cb<1>().append(FUNC(fatman_state::vca_adsr_release_w)).bit(3);

	m_maincpu->port_in_cb<3>().set_constant(0x72).mask(0x72);  // Bits 1, 4, 5, 6.
	m_maincpu->port_in_cb<3>().append(FUNC(fatman_state::midi_rxd_r)).mask(0x01);
	m_maincpu->port_in_cb<3>().append(FUNC(fatman_state::vca_adsr_attack_r)).lshift(2).mask(0x04);
	m_maincpu->port_in_cb<3>().append(FUNC(fatman_state::vcf_ar_attack_r)).lshift(3).mask(0x08);
	m_maincpu->port_in_cb<3>().append_ioport("dsw").mask(0x80);

	m_maincpu->port_out_cb<3>().set(FUNC(fatman_state::vcf_ar_release_w)).bit(1);
	m_maincpu->port_out_cb<3>().append(FUNC(fatman_state::cv_mux_w)).mask(0x30);  // Bits 4, 5.

	FATMAN_RC_NETWORK_STATE(config, m_vca_adsr_state).set_c(C19);
	FATMAN_RC_NETWORK_STATE(config, m_vcf_ar_state).set_c(C22);

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
	PORT_START("VCA_attack")                // POT_R94
	PORT_ADJUSTER(2, "VCA EG Attack")
	PORT_START("VCA_decay")                 // POT_R92
	PORT_ADJUSTER(1, "VCA EG Decay")
	PORT_START("VCA_sustain")               // POT_R90
	PORT_ADJUSTER(50, "VCA EG Sustain")
	PORT_START("VCA_release")               // POT_R96
	PORT_ADJUSTER(3, "VCA EG Release")

	// VCF EG Controls.
	PORT_START("VCF_attack")                // POT_R82
	PORT_ADJUSTER(1, "VCF EG Attack")
	PORT_START("VCF_release")               // POT_R84
	PORT_ADJUSTER(30, "VCF EG Release")
	PORT_START("VCF_sustain")               // S3
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("VCF EG Sustain") PORT_CODE(KEYCODE_S) PORT_TOGGLE
INPUT_PORTS_END

ROM_START(fatman)
	ROM_REGION(0x2000, MAINCPU_TAG, 0)
	ROM_DEFAULT_BIOS("fatman9")
	ROM_SYSTEM_BIOS(0, "fatman9", "FatMan Operating System 1.9 (C) 1995")
	ROMX_LOAD("fatmopv1.9.ic3", 0x000000, 0x002000, CRC(e2a4b9a9) SHA1(3738f0c9b3b158884fddb103b19314899d7e60c0), ROM_BIOS(0))
ROM_END

}  // anonymous namespace

SYST(1992, fatman, 0, 0, fatman, fatman, fatman_state, empty_init, "PAiA Electronics", "FatMan", MACHINE_SUPPORTS_SAVE | MACHINE_NOT_WORKING | MACHINE_NO_SOUND)
