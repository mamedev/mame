// license:BSD-3-Clause
// copyright-holders:m1macrophage

/*
The PAiA midi2cv8 is an 8-channel MIDI-to-CV converter. It is a module for
the PAiA 9700 Series modular system, but can be used as a standalone MIDI-to-CV
converter.

DIP switches control the MIDI channel to listen to, and the operating mode.
Depending on that mode, each of the 8 outputs produces control voltage (CV),
trigger, or gate signals in response to MIDI messages.

The firmware is running on an 80C31. It listens for MIDI messages and sets the
voltage for the 8 outputs by time-multiplexing 8 sample & hold (S&H) circuits.
Specifically, the firmware will write a value to the 8-bit DAC, and after some
delay for the DAC to settle, it will enable the S&H for one output. After some
additional delay for the S&H capacitor to (dis)charge to the target value, the
firmware will disable that S&H and move on to the next one.

The midi2cv8 comes in two configurations: Volt-per-octave (V/Oct) and
Volt-per-Hertz (V/Hz).

In the V/Oct configuration, the output of the DAC (a current) is converted to a
voltage in the range 0-10V.

The V/Hz configuration adds a daughterboard and changes some of the components.
R32 (2.7Kohm) is added, and R28 is changed from 5.6Kohm to 2.7Kohm. These change
the DAC output range to 5-10V. The daughterboard is then used to divide that by
1, 2, 4, 8 or 16 (controlled by the MCU). This is a neat trick to get
exponential voltage output for 5 octaves, while only using an 8-bit DAC.

The firmware for the two configurations is the same. P1.7 of the MCU is read at
startup to detect whether the daughterboard is installed. If it is, the firmware
will switch to V/Hz mode. See the two variants of is_volts_per_hz_r().

This driver is based on the schematics, user manual and documentation provided
by the manufacturer on their website. Both the V/Oct and V/Hz configurations are
emulated. This driver cannot replace the real device. MAME does not output
physical voltages. This is just an educational tool.

Usage:

The provided layout will display the generated CVs/triggers/gates next to the
outputs.

The default dipswitch setting for "Mode" ("Mode 8 - self tests") does not
require MIDI. Just run the driver and see voltages come to life on the screen.

The other modes require supplying a MIDI input source to MAME.
Example:
./mame -listmidi  # List MIDI devices, physical or virtual (e.g. DAWs).
./mame -window midi2cv8 -midiin "{midi device}"

Keep in mind that dipswitch changes don't take effect until a restart or
reset (F3).
*/

#include "emu.h"

#include "cpu/mcs51/mcs51.h"
#include "bus/midi/midiinport.h"
#include "bus/midi/midioutport.h"
#include "video/pwm.h"

#include "paia_midi2cv8.lh"

#define LOG_DAC (1U << 1)
#define LOG_CVS (1U << 2)

#define VERBOSE (LOG_GENERAL)
//#define LOG_OUTPUT_FUNC osd_printf_info

#include "logmacro.h"

namespace {

constexpr const char MAINCPU_TAG[] = "80c31";

class midi2cv8_state : public driver_device
{
public:
	midi2cv8_state(const machine_config &mconfig, device_type type, const char *tag) ATTR_COLD
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, MAINCPU_TAG)
		, m_midi_pwm_led(*this, "midi_pwm_led")
		, m_cv_display_integer(*this, "cv_%d_integer", 1U)
		, m_cv_display_fractional(*this, "cv_%d_fractional", 1U)
		, m_cv(8, -1)
	{
	}

	void midi2cv8(machine_config &config) ATTR_COLD;

protected:
	static constexpr const float DAC_V_MAX = 10;

	void machine_start() override ATTR_COLD;

	u8 get_dac_value() const;
	void update_active_cv();
	virtual bool compute_cv(float *cv) const;
	virtual int is_volts_per_hz_r() const;

	mcs51_cpu_device &get_maincpu() { return *m_maincpu; }

private:
	void midi_rxd_w(int state);
	int midi_rxd_r() const;

	void dac_w(u8 data);
	void output_mux_select_w(u8 data);

	void program_map(address_map &map) ATTR_COLD;
	void external_memory_map(address_map &map) ATTR_COLD;

	required_device<mcs51_cpu_device> m_maincpu;
	required_device<pwm_display_device> m_midi_pwm_led;
	output_finder<8> m_cv_display_integer;
	output_finder<8> m_cv_display_fractional;

	bool m_inhibit_output_mux = false;
	u8 m_selected_output_mux = 0;
	u8 m_dac_value = 0;
	u8 m_midi_rxd_bit = 1; // Initial value needs to be 1, for serial "idle".
	std::vector<float> m_cv;
};

// MIDI2CV8 with the V/Hz daughterboard.
class midi2cv8_vhz_state : public midi2cv8_state
{
public:
	midi2cv8_vhz_state(const machine_config &mconfig, device_type type, const char *tag) ATTR_COLD
		: midi2cv8_state(mconfig, type, tag)
	{
	}

	void midi2cv8_vhz(machine_config &config) ATTR_COLD;

protected:
	void machine_start() override ATTR_COLD;

	bool compute_cv(float *cv) const override;
	int is_volts_per_hz_r() const override;

private:
	void octave_mux_select_w(u8 data);

	u8 m_selected_octave_mux = 0;
};

// The implementations of midi2cv8_state and midi2cv8_vhz_state below are
// interleaved, to better demonstrate the difference in behavior between them.

u8 midi2cv8_state::get_dac_value() const
{
	return m_dac_value;
}

void midi2cv8_state::update_active_cv()
{
	// Mapping from CV MUX output to the output on the panel from top to bottom.
	// 0 is at the top, 7 at the bottom.
	static constexpr const int OUTPUT_MAPPING[8] = {0, 4, 1, 5, 2, 6, 3, 7};

	if (m_inhibit_output_mux)
		return;

	float cv = 0;
	if (!compute_cv(&cv))
		return;

	const int physical_output = OUTPUT_MAPPING[m_selected_output_mux];
	if (cv == m_cv[physical_output])
		return;

	m_cv[physical_output] = cv;
	const s32 cv_millis = s32(round(1000 * cv));
	m_cv_display_integer[physical_output] = cv_millis / 1000;
	m_cv_display_fractional[physical_output] = cv_millis % 1000;

	LOGMASKED(LOG_CVS, "CV %d - %d: %f - %d @ %f\n",
			physical_output + 1, m_selected_output_mux, cv, cv_millis,
			machine().time().as_double());
}

bool midi2cv8_state::compute_cv(float *cv) const
{
	*cv = DAC_V_MAX * get_dac_value() / 255.0F;
	return true;
}

bool midi2cv8_vhz_state::compute_cv(float *cv) const
{
	// -1 means the MUX input is not connected.
	static constexpr const float DIVIDE_BY[8] = {-1, 1, 2, 4, 8, 16, -1, -1};
	static constexpr const float V_HALF = DAC_V_MAX / 2;

	assert(m_selected_octave_mux >= 0 && m_selected_octave_mux < 8);
	const float divisor = DIVIDE_BY[m_selected_octave_mux];
	if (divisor <= 0)
		return false;

	*cv = (V_HALF + V_HALF * get_dac_value() / 255.0F) / divisor;
	return true;
}

int midi2cv8_state::is_volts_per_hz_r() const
{
	// P1.7 pulled up by R54, but connected to GND when the V/Hz option is not
	// installed. This results in P1.7 reading as 0.
	return 0;
}

int midi2cv8_vhz_state::is_volts_per_hz_r() const
{
	// P1.7 pulled up by R54, and connected to R5 in the V/Hz board, when
	// that board is installed. This results in P1.7 reading as 1.
	return 1;
}

void midi2cv8_state::midi_rxd_w(int state)
{
	m_midi_rxd_bit = state;

	// MIDI IN state is inverted twice (IC6:A and IC6:C) and connected to the
	// cathode of LED D2. So the LED will be on when MIDI IN is low.
	m_midi_pwm_led->write_element(0, 0, state ? 0 : 1);
}

int midi2cv8_state::midi_rxd_r() const
{
	return m_midi_rxd_bit;
}

void midi2cv8_state::dac_w(u8 data)
{
	if (m_dac_value == data)
		return;
	m_dac_value = data;
	update_active_cv();
	LOGMASKED(LOG_DAC, "DAC value: %02x\n", m_dac_value);
}

void midi2cv8_vhz_state::octave_mux_select_w(u8 data)
{
	// Octave MUX (IC4) is a 4051. X0, X6 and X7 are not connected.
	// MUX INH is tied low, so it is always enabled.
	// P1.5 -> OCT A.
	// P1.6 -> OCT B.
	// P1.7 -> OCT C.
	// All signals above are inverted and level-shifted by Q1-Q3.

	const u8 selection = (~data & 0xe0) >> 5;  // Bits 5-7.
	if (m_selected_octave_mux == selection)
		return;
	m_selected_octave_mux = selection;
	update_active_cv();
}

void midi2cv8_state::output_mux_select_w(u8 data)
{
	// MUX (IC13) is a 4051.
	// P3.1 -> MUX INH
	// P3.2 -> MUX B
	// P3.3 -> MUX C
	// P3.4 -> MUX A
	// All signals above are inverted and level-shifted by Q4-Q7.

	data = ~data & 0x1e;
	const bool inhibit = BIT(data, 1);
	const u8 selection = bitswap<3>(data, 3, 2, 4);
	if (inhibit == m_inhibit_output_mux && selection == m_selected_output_mux)
		return;
	m_inhibit_output_mux = inhibit;
	m_selected_output_mux = selection;
	update_active_cv();
}

void midi2cv8_state::program_map(address_map &map)
{
	// A13-A15 are not connected.
	map(0x0000, 0x1fff).mirror(0xe000).rom();
}

void midi2cv8_state::external_memory_map(address_map &map)
{
	// Address lines ignored on external memory writes.
	map(0x0000, 0x0000).mirror(0xffff).w(FUNC(midi2cv8_state::dac_w));
}

void midi2cv8_state::machine_start()
{
	m_cv_display_integer.resolve();
	m_cv_display_fractional.resolve();

	save_item(NAME(m_inhibit_output_mux));
	save_item(NAME(m_selected_output_mux));
	save_item(NAME(m_dac_value));
	save_item(NAME(m_midi_rxd_bit));
	save_item(NAME(m_cv));
}

void midi2cv8_vhz_state::machine_start()
{
	midi2cv8_state::machine_start();
	save_item(NAME(m_selected_octave_mux));
}

void midi2cv8_state::midi2cv8(machine_config &config)
{
	I80C31(config, m_maincpu, 12_MHz_XTAL);
	m_maincpu->set_addrmap(AS_PROGRAM, &midi2cv8_state::program_map);
	m_maincpu->set_addrmap(AS_IO, &midi2cv8_state::external_memory_map);

	m_maincpu->port_in_cb<1>().set_ioport("dsw").mask(0x1f);  // P1.0-P1.4
	m_maincpu->port_in_cb<1>().append(FUNC(midi2cv8_state::is_volts_per_hz_r)).lshift(7).mask(0x80);  // P1.7

	m_maincpu->port_in_cb<3>().set(FUNC(midi2cv8_state::midi_rxd_r)).mask(0x01);  // P3.0
	m_maincpu->port_in_cb<3>().append_ioport("dsw").mask(0x20);  // P3.5 <- DSW BIT 5
	m_maincpu->port_in_cb<3>().append_ioport("dsw").lshift(1).mask(0x80);  //  P3.7 <- DSW BIT 6
	m_maincpu->port_out_cb<3>().set(FUNC(midi2cv8_state::output_mux_select_w));

	midi_port_device &midi_in(MIDI_PORT(config, "mdin", midiin_slot, "midiin"));
	MIDI_PORT(config, "mdthru", midiout_slot, "midiout");
	midi_in.rxd_handler().set(FUNC(midi2cv8_state::midi_rxd_w));
	midi_in.rxd_handler().append("mdthru", FUNC(midi_port_device::write_txd));

	PWM_DISPLAY(config, m_midi_pwm_led).set_size(1, 1);
	m_midi_pwm_led->output_x().set_output("midi_led");
	// These values make the MIDI LED in the default layout functional,
	// without being too annoying.
	m_midi_pwm_led->set_interpolation(0.2);
	m_midi_pwm_led->set_bri_levels(0.0001);
	m_midi_pwm_led->set_refresh(attotime::from_hz(30));

	config.set_default_layout(layout_paia_midi2cv8);
}

void midi2cv8_vhz_state::midi2cv8_vhz(machine_config &config)
{
	midi2cv8(config);
	get_maincpu().port_out_cb<1>().set(FUNC(midi2cv8_vhz_state::octave_mux_select_w));
}

INPUT_PORTS_START(midi2cv8)
	PORT_START("dsw")
	PORT_DIPNAME(0x0f, 0x00, "MIDI Channel") PORT_DIPLOCATION("SW1:1,2,3,4")
	PORT_DIPSETTING(   0x00, "1")
	PORT_DIPSETTING(   0x01, "2")
	PORT_DIPSETTING(   0x02, "3")
	PORT_DIPSETTING(   0x03, "4")
	PORT_DIPSETTING(   0x04, "5")
	PORT_DIPSETTING(   0x05, "6")
	PORT_DIPSETTING(   0x06, "7")
	PORT_DIPSETTING(   0x07, "8")
	PORT_DIPSETTING(   0x08, "9")
	PORT_DIPSETTING(   0x09, "10")
	PORT_DIPSETTING(   0x0a, "11")
	PORT_DIPSETTING(   0x0b, "12")
	PORT_DIPSETTING(   0x0c, "13")
	PORT_DIPSETTING(   0x0d, "14")
	PORT_DIPSETTING(   0x0e, "15")
	PORT_DIPSETTING(   0x0f, "16")
	PORT_DIPNAME(0x70, 0x70, "Mode") PORT_DIPLOCATION("SW1:5,6,7")
	PORT_DIPSETTING(   0x00, "Mode 1 - 1 voice")
	PORT_DIPSETTING(   0x10, "Mode 2 - 2 voice")
	PORT_DIPSETTING(   0x20, "Mode 3 - 4 voice")
	PORT_DIPSETTING(   0x30, "Mode 4 - control change")
	PORT_DIPSETTING(   0x40, "Mode 5 - analog drum")
	PORT_DIPSETTING(   0x50, "Mode 6 - din sync")
	PORT_DIPSETTING(   0x60, "Mode 7 - [unused]")
	PORT_DIPSETTING(   0x70, "Mode 8 - self-tests")
	PORT_DIPNAME(0x80, 0x00, "Not Connected") PORT_DIPLOCATION("SW1:8")
	PORT_DIPSETTING(   0x00, DEF_STR(On))
	PORT_DIPSETTING(   0x80, DEF_STR(Off))
INPUT_PORTS_END

#define ROMS_MIDI2CV8 \
	ROM_REGION(0x2000, MAINCPU_TAG, 0) \
	ROM_DEFAULT_BIOS("v201") \
	ROM_SYSTEM_BIOS(0, "v201", "v2.01 - December 1997") \
	ROMX_LOAD("midi2cv_v2.01.ic2", 0x000000, 0x002000, CRC(bae8c045) SHA1(a5db57e53831b73903a0fb171e0444e6956febc3), ROM_BIOS(0))

ROM_START(midi2cv8)
	ROMS_MIDI2CV8
ROM_END

ROM_START(midi2cv8_vhz)
	ROMS_MIDI2CV8
ROM_END

} // anonymous namespace

SYST(1997, midi2cv8, 0, 0, midi2cv8, midi2cv8, midi2cv8_state, empty_init, "PAiA Electronics", "midi2cv8", MACHINE_NO_SOUND_HW | MACHINE_SUPPORTS_SAVE)
SYST(1997, midi2cv8_vhz, 0, 0, midi2cv8_vhz, midi2cv8, midi2cv8_vhz_state, empty_init, "PAiA Electronics", "midi2cv8 V/Hz", MACHINE_NO_SOUND_HW | MACHINE_SUPPORTS_SAVE)
