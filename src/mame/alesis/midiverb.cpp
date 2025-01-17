// license:BSD-3-Clause
// copyright-holders:m1macrophage

/*
The Midiverb is a digital delay & reverb unit.

The computer portion of the device is very simple. The firmware runs on a
80C31 microcontroller. It reads the 4 buttons, drives the two 7-segment
displays, and listens to MIDI for program changes. It also controls which
program (effect) is running on the DSP.

An interesting aspect of the Midiverb is its DSP, which is built out of discrete
logic components. This runs custom microcode consisting of 4 instructions.

The UI is very simple. The user can choose one of 63 effects by using the
"up" and "down" buttons on the unit. The effect can also be set via MIDI
program changes, and the MIDI channel is configurable ("channel" button). The
"defeat" button will run the 64th effect, which is just a bypass. That same
bypass effect is also enabled temporarily, when switching between effects.
Finally, there is a wet/dry control knob.

This driver is intended as an educational tool.

TODO: Audio & DSP (coming soon).

Usage notes:

The driver comes with an interactive layout.
MIDI is optional, and can be configured as follows:
./mame -listmidi  # List MIDI devices, physical or virtual (e.g. DAWs).
./mame -window midiverb -midiin "{midi device}"
*/

#include "emu.h"

#include "cpu/mcs51/mcs51.h"
#include "bus/midi/midiinport.h"
#include "bus/midi/midioutport.h"
#include "video/pwm.h"

#include "alesis_midiverb.lh"

#define LOG_PROGRAM_CHANGE (1U << 1)

#define VERBOSE (LOG_GENERAL | LOG_PROGRAM_CHANGE)
//#define LOG_OUTPUT_FUNC osd_printf_info

#include "logmacro.h"

namespace {

constexpr const char MAINCPU_TAG[] = "80c31";

class midiverb_state : public driver_device
{
public:
	midiverb_state(const machine_config &mconfig, device_type type, const char *tag) ATTR_COLD
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, MAINCPU_TAG)
		, m_digit_device(*this, "pwm_digit_device")
		, m_digit_out(*this, "digit_%d", 1U)
	{
	}

	void midiverb(machine_config &config) ATTR_COLD;

protected:
	void machine_start() override ATTR_COLD;

private:
	u8 midi_rxd_r() const;
	void program_select_w(u8 data);
	void digit_select_w(u8 data);
	void digit_latch_w(u8 data);
	void digit_out_update_w(offs_t offset, u8 data);
	void midi_rxd_w(int state);

	void program_map(address_map &map) ATTR_COLD;
	void external_memory_map(address_map &map) ATTR_COLD;

	required_device<mcs51_cpu_device> m_maincpu;
	required_device<pwm_display_device> m_digit_device;
	output_finder<2> m_digit_out;  // 2 x MAN4710A (7-seg display), DS1 & DS2.

	bool m_midi_rxd_bit = true; // Start high for serial idle.
	u8 m_digit_latch_inv = 0x00;
	u8 m_digit_mask = 0x00;
	u8 m_program = 0;
};

u8 midiverb_state::midi_rxd_r() const
{
	return m_midi_rxd_bit ? 1 : 0;
}

void midiverb_state::program_select_w(u8 data)
{
	const u8 new_program = data & 0x3f;
	if (m_program == new_program)
		return;
	m_program = new_program;
	LOGMASKED(LOG_PROGRAM_CHANGE, "Program set to: %d\n", m_program);
}

void midiverb_state::digit_select_w(u8 data)
{
	// The digit select signals (bit 0 and 1) are active-low. They connect to
	// the base of PNP transistors (2N4403, Q4 and Q3 for DS1 and DS2
	// respectively). When low, power is connected to the MAN4710 anode inputs.
	m_digit_mask = ~data & 0x03;
	m_digit_device->matrix(m_digit_mask, m_digit_latch_inv);
}

void midiverb_state::digit_latch_w(u8 data)
{
	// The Data bus is connected to the latch in an unintuitive way. Same goes
	// for the connections from the latch to the 7seg display. Presumably done
	// to save board space, which was limited.
	const u8 descrambled = bitswap<8>(data, 3, 1, 6, 7, 4, 0, 2, 5);

	// Inverting because segment LEDs are active-low, but pwm_display_device
	// expects active-high.
	m_digit_latch_inv = ~descrambled & 0x7f;
	m_digit_device->matrix(m_digit_mask, m_digit_latch_inv);
}

void midiverb_state::digit_out_update_w(offs_t offset, u8 data)
{
	// Digits are ordered from left to right. So offset 0 (corresponding to
	// digit_1) is the most significant digit.
	m_digit_out[offset] = data;
}

void midiverb_state::midi_rxd_w(int state)
{
	m_midi_rxd_bit = state;
}

void midiverb_state::program_map(address_map &map)
{
	// 2764 ROM has A0-A11 connected to the MCU, and A12 tied high. ROM /OE
	// is tied to MCU A15, so it is only active when A15 is 0.
	map(0x0000, 0x0fff).mirror(0x7000).rom().region(MAINCPU_TAG, 0x1000);
}

void midiverb_state::external_memory_map(address_map &map)
{
	// Address lines ignored when writing to external memory.
	map(0x0000, 0x0000).mirror(0xffff).w(FUNC(midiverb_state::digit_latch_w));
}

void midiverb_state::machine_start()
{
	m_digit_out.resolve();
	save_item(NAME(m_midi_rxd_bit));
	save_item(NAME(m_digit_latch_inv));
	save_item(NAME(m_digit_mask));
	save_item(NAME(m_program));
}

void midiverb_state::midiverb(machine_config &config)
{
	I80C31(config, m_maincpu, 6_MHz_XTAL);  // U55.
	m_maincpu->set_addrmap(AS_PROGRAM, &midiverb_state::program_map);
	m_maincpu->set_addrmap(AS_IO, &midiverb_state::external_memory_map);

	m_maincpu->port_out_cb<1>().set(FUNC(midiverb_state::digit_select_w)).mask(0x03);  // P1.0-P1.1
	m_maincpu->port_out_cb<1>().append(FUNC(midiverb_state::program_select_w)).rshift(2);  // P1.2-P1.7
	m_maincpu->port_in_cb<3>().set(FUNC(midiverb_state::midi_rxd_r)).mask(0x01);  // P3.0
	m_maincpu->port_in_cb<3>().append_ioport("buttons").lshift(2).mask(0x3c);  // P3.2-P3.5

	midi_port_device &midi_in = MIDI_PORT(config, "mdin", midiin_slot, "midiin");
	MIDI_PORT(config, "mdthru", midiout_slot, "midiout");
	midi_in.rxd_handler().set(FUNC(midiverb_state::midi_rxd_w));
	midi_in.rxd_handler().append("mdthru", FUNC(midi_port_device::write_txd));

	PWM_DISPLAY(config, m_digit_device).set_size(2, 7);  // 2 x MAN4710.
	m_digit_device->set_segmask(0x03, 0x7f);
	m_digit_device->output_digit().set(FUNC(midiverb_state::digit_out_update_w));

	config.set_default_layout(layout_alesis_midiverb);
}

INPUT_PORTS_START(midiverb)
	PORT_START("buttons")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("MIDI CHANNEL") PORT_CODE(KEYCODE_C)
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("UP") PORT_CODE(KEYCODE_UP)
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("DOWN") PORT_CODE(KEYCODE_DOWN)
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("DEFEAT") PORT_CODE(KEYCODE_D)
INPUT_PORTS_END

ROM_START(midiverb)
	ROM_REGION(0x2000, MAINCPU_TAG, 0)
	// U54. 2764 ROM.
	ROM_LOAD("mvop_4-7-86.u54", 0x000000, 0x002000, CRC(14d6596d) SHA1(c6dc579d8086556b2dd4909c8deb3c7006293816))
ROM_END

}  // anonymous namespace

SYST(1986, midiverb, 0, 0, midiverb, midiverb, midiverb_state, empty_init, "Alesis", "MIDIverb", MACHINE_SUPPORTS_SAVE | MACHINE_NOT_WORKING | MACHINE_NO_SOUND)

