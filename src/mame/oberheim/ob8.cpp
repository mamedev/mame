// license:BSD-3-Clause
// copyright-holders:m1macrophage
/*
The OB-8 is an 8-voice digitally-controlled analog synthesizer.

The firmware runs on a Z80.

The driver is based on the OB8 srevice manual and schematics, and is intended
as an educational tool.

This driver is very much an early-stage skeleton.

Board prefixes in component designations (e.g. PB:U54)
PB - Processor Board.
BB - Bend Board.
TB - Pot Board.
*/

#include "emu.h"

#include "cpu/z80/z80.h"
#include "machine/nvram.h"
#include "machine/output_latch.h"
#include "machine/pit8253.h"
#include "video/pwm.h"

namespace {

constexpr const char MAINCPU_TAG[] = "z80";
constexpr const char NVRAM_TAG[] = "nvram";

class ob8_state : public driver_device
{
public:
	ob8_state(const machine_config &mconfig, device_type type, const char *tag) ATTR_COLD
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, MAINCPU_TAG)
		, m_pit(*this, "pit_8253")
		, m_led_matrix_device(*this, "led_matrix_device")
		, m_switch_io(*this, "switch_column_%d", 0U)
		, m_b_switch_io(*this, "b_switch_%d", 0U)
	{
	}

	void ob8(machine_config &config) ATTR_COLD;

private:
	u8 switches_r(offs_t offset);
	u8 b_switches_r(offs_t offset);
	void leds_w(offs_t offset, u8 data);

	void memory_map(address_map &map) ATTR_COLD;
	void io_map(address_map &map) ATTR_COLD;

	required_device<z80_device> m_maincpu;
	required_device<pit8253_device> m_pit;
	required_device<pwm_display_device> m_led_matrix_device;
	required_ioport_array<16> m_switch_io;
	required_ioport_array<2> m_b_switch_io;

	u8 m_selected_pot = 0;
	bool m_hold_pedal_enabled = false;
};

u8 ob8_state::switches_r(offs_t offset)
{
	const u8 selected_pot = offset & 0x1f;
	const u8 selected_column = offset & 0x0f;

	if (!machine().side_effects_disabled())
	{
		// A3-A4 select the MUX device, and A0-A2 select the MUX input.
		m_selected_pot = selected_pot;
		const u8 selected_mux_device = m_selected_pot >> 3;
		if (selected_mux_device == 3)
		{
			// MUX 3 (BB:U1) has its "C" input grounded, so A2 has no effect.
			m_selected_pot &= 0xfb;
		}

		m_hold_pedal_enabled = (selected_column == 9);
	}

	// Inverted by buffers PB:U53 and PB:U54.
	return ~m_switch_io[selected_column]->read();
}

u8 ob8_state::b_switches_r(offs_t offset)
{
	// BSWEN* further decoded by U56.
	// A0 == 0: BSW0*, A0 == 1: BSW1*.
	// Inverted by BB:U3 (80C98).
	// Only D0-D5 are relevant, D6-D7 are pulled high.
	return 0xc0 | (~m_b_switch_io[offset]->read() & 0x3f);
}

void ob8_state::leds_w(offs_t offset, u8 data)
{
	// The LED sources (rows) are controlled by D2-D7 (schematic signals LR2-7.
	// LR = Led Row). They are active low. Inverting 'data' because matrix()
	// expects an active-high mask.
	const u8 source_mask = (~data >> 2) & 0x3f;

	// The LED sink (column) is controlled by A0-A2, which are decoded by U1
	// (4028) and inverted by TB:U2. This creates a single-bit, active-low mask.
	// But matrix() requires an active-high mask.
	const u8 sink_mask = 1 << (offset & 0x07);

	m_led_matrix_device->matrix(source_mask, sink_mask);
}

void ob8_state::memory_map(address_map &map)
{
	// Signal names below (e.g. PROT*, IOR*) match those in the schematic.

	// ROM decoding done by PB:U32A (74LS139).
	map(0x0000, 0x3fff).rom();  // 4 x 2732, 4Kbyte ROMs (PB:U24-U21).

	// RAM decoding done by PB:U32B (74LS139).
	// RAMs powered by battery when there is no power.
	map(0x4000, 0x5fff).ram().share(NVRAM_TAG); // 4 x 6116 2KB RAMs (PB:U20-U17).
	// TODO: Is U17 populated?
	// TODO: Implement write protection for PB:U18-19.

	// TODO: map(0x6000, 0x7fff) // CHEN* // Decoded by PB:U41A (74LS139).

	map(0x7c00, 0x7c1f).mirror(0x0060).r(FUNC(ob8_state::switches_r));  // IOR*
	map(0x7c80, 0x7c87).mirror(0x0078).w(FUNC(ob8_state::leds_w));  // LEDS*

	// 0x7d00- 0x7d7f: SWTCH*. Further decoding done by PB:U55 (LS139)
	map(0x7d00, 0x7d00).mirror(0x001f).w("latch_bled0", FUNC(output_latch_device::write));  // BLED0*
	map(0x7d20, 0x7d20).mirror(0x001f).w("latch_bled1", FUNC(output_latch_device::write));  // BLED1*
	//TODO: map(0x7d40, 0x7d5f);  // MISC*
	map(0x7d60, 0x7d61).mirror(0x001e).r(FUNC(ob8_state::b_switches_r));  // BSWEN*

	map(0x7d80, 0x7d83).mirror(0x007c).rw(m_pit, FUNC(pit8253_device::read), FUNC(pit8253_device::write));  // TIMER*
	// TODO: map(0x7e00, 0x7e7f);  // STAT*

	map(0x7e80, 0x7e80).mirror(0x007f).w("latch_pb_u4", FUNC(output_latch_device::write));  // LATCH*

	// TODO: map(0x7f00, 0x7f7f);  // MISC2*
	// 0x7f80 - 0x7fff: unused. Decoder output not connected.
}

void ob8_state::io_map(address_map &map)
{
}

void ob8_state::ob8(machine_config &config)
{
	Z80(config, m_maincpu, 8_MHz_XTAL / 2);
	m_maincpu->set_addrmap(AS_PROGRAM, &ob8_state::memory_map);
	m_maincpu->set_addrmap(AS_IO, &ob8_state::io_map);

	NVRAM(config, NVRAM_TAG, nvram_device::DEFAULT_ALL_0);

	PIT8253(config, m_pit);
	// TODO: clock 0 connected to OSC.
	m_pit->set_clk<1>(8_MHz_XTAL / 2);
	m_pit->set_clk<2>(8_MHz_XTAL / 2);
	// TODO: The rest of the connections.

	PWM_DISPLAY(config, m_led_matrix_device).set_size(6, 8);
	// TODO: Set up LED outputs.

	// 74HC174, 6-bit latch, PB:U4.
	output_latch_device &u4_pb(OUTPUT_LATCH(config, "latch_pb_u4"));
	u4_pb.bit_handler<0>().set_output("ARM");
	u4_pb.bit_handler<1>().set_output("AUTOST");
	// Bits 2-5 not connected.

	// 74LS174, 6-bit latch, BB:U5. Controls LEDs, active low.
	output_latch_device &bled0(OUTPUT_LATCH(config, "latch_bled0"));
	bled0.bit_handler<0>().set_output("led_osc1").invert();
	bled0.bit_handler<1>().set_output("led_osc2").invert();
	bled0.bit_handler<2>().set_output("led_osc2only").invert();
	bled0.bit_handler<3>().set_output("led_amount").invert();
	bled0.bit_handler<4>().set_output("led_down").invert();
	bled0.bit_handler<5>().set_output("led_up").invert();

	// 74LS174, 6-bit latch, BB:U4. Controls LEDs, active low.
	output_latch_device &bled1(OUTPUT_LATCH(config, "latch_bled1"));
	bled1.bit_handler<0>().set_output("led_rate").invert();
	bled1.bit_handler<1>().set_output("led_mode").invert();
	bled1.bit_handler<2>().set_output("led_lower").invert();
	bled1.bit_handler<3>().set_output("led_upper").invert();
	bled1.bit_handler<4>().set_output("led_arpegiate").invert();
	// Bit 5 is unused.
}

INPUT_PORTS_START(ob8)
	PORT_START("switch_column_0")  // C0 - G0 in schematic.
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_OTHER) PORT_GM_C2
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_OTHER) PORT_GM_CS2
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_OTHER) PORT_GM_D2
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_OTHER) PORT_GM_DS2
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_OTHER) PORT_GM_E2
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_OTHER) PORT_GM_F2
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_OTHER) PORT_GM_FS2
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_OTHER) PORT_GM_G2

	PORT_START("switch_column_1")  // G0# - D1#
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_OTHER) PORT_GM_GS2
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_OTHER) PORT_GM_A2
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_OTHER) PORT_GM_AS2
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_OTHER) PORT_GM_B2
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_OTHER) PORT_GM_C3
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_OTHER) PORT_GM_CS3
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_OTHER) PORT_GM_D3
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_OTHER) PORT_GM_DS3

	PORT_START("switch_column_2")  // E1 - B1
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_OTHER) PORT_GM_E3
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_OTHER) PORT_GM_F3
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_OTHER) PORT_GM_FS3
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_OTHER) PORT_GM_G3
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_OTHER) PORT_GM_GS3
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_OTHER) PORT_GM_A3
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_OTHER) PORT_GM_AS3
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_OTHER) PORT_GM_B3

	PORT_START("switch_column_3")  // C2 - G2
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_OTHER) PORT_GM_C4
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_OTHER) PORT_GM_CS4
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_OTHER) PORT_GM_D4
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_OTHER) PORT_GM_DS4
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_OTHER) PORT_GM_E4
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_OTHER) PORT_GM_F4
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_OTHER) PORT_GM_FS4
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_OTHER) PORT_GM_G4

	PORT_START("switch_column_4")  // G2# - D3#
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_OTHER) PORT_GM_GS4
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_OTHER) PORT_GM_A4
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_OTHER) PORT_GM_AS4
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_OTHER) PORT_GM_B4
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_OTHER) PORT_GM_C5
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_OTHER) PORT_GM_CS5
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_OTHER) PORT_GM_D5
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_OTHER) PORT_GM_DS5

	PORT_START("switch_column_5")  // E3 - B3
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_OTHER) PORT_GM_E5
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_OTHER) PORT_GM_F5
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_OTHER) PORT_GM_FS5
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_OTHER) PORT_GM_G5
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_OTHER) PORT_GM_GS5
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_OTHER) PORT_GM_A5
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_OTHER) PORT_GM_AS5
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_OTHER) PORT_GM_B5

	PORT_START("switch_column_6")  // C4 - G4
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_OTHER) PORT_GM_C6
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_OTHER) PORT_GM_CS6
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_OTHER) PORT_GM_D6
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_OTHER) PORT_GM_DS6
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_OTHER) PORT_GM_E6
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_OTHER) PORT_GM_F6
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_OTHER) PORT_GM_FS6
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_OTHER) PORT_GM_G6

	PORT_START("switch_column_7")  // G4# - C5
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_OTHER) PORT_GM_GS6
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_OTHER) PORT_GM_A6
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_OTHER) PORT_GM_AS6
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_OTHER) PORT_GM_B6
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_OTHER) PORT_GM_C7
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_UNUSED)

	PORT_START("switch_column_8")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("PRG1")
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("PRG2")
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("PRG3")
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("PRG4")
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("PRG5")
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("PRG6")
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("PRG7")
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("PRG8")

	PORT_START("switch_column_9")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("HOLD")
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("OSC2 FM")
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("FILTER FM")
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("UNISON")
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("LFO SIN")
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("LFO SQR")
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("LFO S/H")

	PORT_START("switch_column_10")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("OSC1 PWM")
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("OSC2 PWM")
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("OSC1 SAW")
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("OSC1 PULSE")
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("OSC2 SAW")
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("OSC2 PULSE")

	PORT_START("switch_column_11")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("TRACK")
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("OSC1 FULL")
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("OSC1 HALF")
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("OSC2 FULL")
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("4 POLE")
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("NOISE")

	PORT_START("switch_column_12")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("TEST1")
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("TEST2")
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("OSC1 FM")
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("SYNC")
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("F-ENV")
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("VCA")
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("AUTO")
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("CHORD")

	PORT_START("switch_column_13")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("LOWER")
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("UPPER")
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("SPLIT")
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("GRP A")
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("GRP B")
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("GRP C")
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("GRP D")

	PORT_START("switch_column_14")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("DOUBLE")
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("MANUAL")
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("WRITE")
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_UNUSED)

	// TODO: Find actual usage. See SWENF* in PB sheet 2/4.
	// (SWENF* is coming from TB).
	PORT_START("switch_column_15")
	PORT_BIT(0xff, IP_ACTIVE_LOW, IPT_UNUSED)

	PORT_START("b_switch_0")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("OSC1")
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("OSC2")
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("OSC2 ONLY")
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("AMNT")
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("DOWN")
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("UP")

	PORT_START("b_switch_1")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("WAVE")
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("MODE")
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("LOWER")
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("UPPER")
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("ARPEG")
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("DEPTH ON/OFF")

	// 4051 MUX TB:U7 (Pot Board), enabled by POT0*.
	PORT_START("pot_0")
	PORT_ADJUSTER(50, "VCF REL")
	PORT_START("pot_1")
	PORT_ADJUSTER(50, "VCA REL")
	PORT_START("pot_2")
	PORT_ADJUSTER(50, "VCF DCY")
	PORT_START("pot_3")
	PORT_ADJUSTER(50, "VCA DCY")
	PORT_START("pot_4")
	PORT_ADJUSTER(50, "VCF ATK")
	PORT_START("pot_5")
	PORT_ADJUSTER(50, "VCA ATK")
	PORT_START("pot_6")
	PORT_ADJUSTER(50, "VCF SUS")
	PORT_START("pot_7")
	PORT_ADJUSTER(50, "VCA SUS")

	// 4051 MUX TB:U5 (Pot Board), enabled by POT1*.
	PORT_START("pot_8")
	PORT_ADJUSTER(50, "VCF MOD")
	PORT_START("pot_9")
	PORT_ADJUSTER(50, "VCF RES")
	PORT_START("pot_10")
	PORT_ADJUSTER(50, "VCO PW")
	PORT_START("pot_11")
	PORT_ADJUSTER(50, "LFO FREQ")
	PORT_START("pot_12")
	PORT_ADJUSTER(50, "FM AMNT")
	PORT_START("pot_13")
	PORT_ADJUSTER(50, "PWM AMNT")
	PORT_START("pot_14")
	PORT_ADJUSTER(50, "PORT AMNT")
	PORT_START("pot_15")
	PORT_ADJUSTER(50, "VCO2 DETUNE")

	// 4051 MUX TB:U6 (Pot Board), enabled by POT2*.
	PORT_START("pot_16")
	PORT_ADJUSTER(50, "VCF FREQ")
	PORT_START("pot_17")
	PORT_ADJUSTER(50, "VCO2 FREQ")
	PORT_START("pot_18")
	PORT_ADJUSTER(50, "VCO1 FREQ")
	PORT_START("pot_19")
	PORT_ADJUSTER(50, "BALANCER PROG VOL")
	PORT_START("pot_20")
	PORT_ADJUSTER(50, "MASTER TUNE")
	// The rest of the mux inputs are grounded.
	PORT_START("pot_21")
	PORT_BIT(0xff, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_START("pot_22");
	PORT_BIT(0xff, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_START("pot_23")
	PORT_BIT(0xff, IP_ACTIVE_HIGH, IPT_UNUSED)

	// 4051 MUX BB:U1 (Bend Board), enabled by POT3*.
	PORT_START("pot_24")
	PORT_ADJUSTER(50, "SPEED")
	PORT_START("pot_25")
	PORT_ADJUSTER(50, "DEPTH")
	PORT_START("pot_26")
	PORT_ADJUSTER(50, "BEND LEVER")
	PORT_START("pot_27")
	PORT_ADJUSTER(50, "VIBRATO LEVER")
	// The rest of the mux inputs are grounded, but they can never be addressed
	// Mux address "C" tied to ground, essentially mirroring the above 4 inputs.
INPUT_PORTS_END

ROM_START(ob8)
	ROM_REGION(0x4000, MAINCPU_TAG, 0)
	ROM_DEFAULT_BIOS("a8")

	ROM_SYSTEM_BIOS(0, "a8", "OB-8 A8 OS")
	ROMX_LOAD("ob8a80.u24", 0x000000, 0x001000, CRC(3d141a93) SHA1(4d9866687f5dfe09133da9a4feedd9af0862cfbe), ROM_BIOS(0))
	ROMX_LOAD("ob8a81.u23", 0x001000, 0x001000, CRC(fba31703) SHA1(487258baac9d5bb399c5ad1630249e41302305ba), ROM_BIOS(0))
	ROMX_LOAD("ob8a82.u22", 0x002000, 0x001000, CRC(e6e99305) SHA1(f2c4c28cf3feb77fb8e401e191b0f22af6b09e90), ROM_BIOS(0))
	ROMX_LOAD("ob8a83.u21", 0x003000, 0x001000, CRC(6912415d) SHA1(77108a9540e4d84833dc0fa8066025d812bb6e7c), ROM_BIOS(0))
ROM_END

}  // anonymous namespace

// 1983 - 1985.
SYST(1983, ob8, 0, 0, ob8, ob8, ob8_state, empty_init, "Oberheim", "OB8", MACHINE_NOT_WORKING | MACHINE_NO_SOUND)

