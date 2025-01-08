// license:BSD-3-Clause
// copyright-holders:m1macrophage

/*
The Oberheim DMX is a digital drum machine.

The firmware, running on a Z80, implements the drum machine functionality (e.g.
sequencer), controls the UI (scans for button presses, drives the display),
triggers voices, and controls voice variations. All harware functionality is
mapped to I/O ports. Only ROM and RAM are mapped to memory.

There are 8 voice cards (though the CYMBAL voice consists of 2 physical cards),
each of which can produce 3 different variations, for a total of 24 sounds. The
8 voice cards can play back simultaneously, but only 1 variation per card can be
active at a time, for a total of 8 voices of polyphony.

The drum sounds are samples stored in ROMs. During playback, they are
post-processed by an analog VCA, which is modulated by a Release envelope
genereator. Pitch and volume variations for each voice are also controlled by
analog circuitry.

Currently, this driver emulates the early version of the DMX. A later hardware
revision added additional memory, while the final hardware revision added MIDI.
These 2 subsequent revisions are not yet emulated.

The driver is based on the DMX's service manual and schematics. It is intended
as an educational tool.

PCBoards:
- Processor Board (includes the power supply and connectors)
- Switch Board (buttons, faders and display)
- Voice Card - Bass
- Voice Card - Snare
- Voice Card - Hi-hat
- Voice Card - Tom 1
- Voice Card - Tom 2
- Voice Card - Perc 1
- Voice Card - Perc 2
- Voice Card - Cymbal #1
- Voice Card - Cymbal #2
Cymbal #1 and #2 implement a single voice, but occupy two voice card slots.

Usage notes:
- No audio support (coming very soon).
- An interactive, clickable layout is included.
- This driver is marked as "not working", due to the lack of audio support. But
  the UI is otherwise functional.
- The mixer faders can be controlled with the mouse, or from the "Sliders" menu.
- The drum keys are mapped to the keyboard, starting at "Q". Specifically:
  Q - Bass 1, W - Snare 1, ...
  A - Bass 2, S - Snare 2, ...
  Z - Bass 3, X - Snare 3, ...
- The number buttons on the layout are mapped to the numeric keypad.
*/

#include "emu.h"

#include "attotime.h"
#include "cpu/z80/z80.h"
#include "machine/nvram.h"
#include "machine/output_latch.h"
#include "machine/timer.h"
#include "video/dl1416.h"

#include "oberheim_dmx.lh"

#define LOG_TRIGGERS  (1U << 1)
#define LOG_INT_TIMER (1U << 2)

#define VERBOSE (LOG_GENERAL | LOG_TRIGGERS | LOG_INT_TIMER)
//#define LOG_OUTPUT_FUNC osd_printf_info

#include "logmacro.h"

namespace {

constexpr const char MAINCPU_TAG[] = "z80";
constexpr const char NVRAM_TAG[] = "nvram";

class dmx_state : public driver_device
{
public:
	dmx_state(const machine_config &mconfig, device_type type, const char *tag) ATTR_COLD
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, MAINCPU_TAG)
		, m_digit_device(*this, "dl1414_%d", 0)
		, m_digit_output(*this, "digit_%d", 0U)
		, m_metronome_timer(*this, "metronome_timer")
		, m_buttons(*this, "buttons_%d", 0)
		, m_switches(*this, "switches")
		, m_external_triggers(*this, "external_triggers")
		, m_clk_in(*this, "clk_in")
		, m_clk_out_tip(*this, "CLK_OUT_tip")
		, m_metronome_mix(*this, "METRONOME_MIX")
		, m_metronome(*this, "METRONOME")
	{
	}

	void dmx(machine_config &config) ATTR_COLD;

	DECLARE_INPUT_CHANGED_MEMBER(clk_in_changed);

protected:
	void machine_start() override ATTR_COLD;

private:
	void refresh_int_flipflop();
	void int_timer_preset_w(u8 data);
	void int_timer_enable_w(int state);
	TIMER_DEVICE_CALLBACK_MEMBER(int_timer_tick);

	void metronome_trigger_w(u8 data);
	void metronome_mix_w(u8 data);
	TIMER_DEVICE_CALLBACK_MEMBER(metronome_timer_tick);

	void display_w(offs_t offset, u8 data);
	template<int DISPLAY> void display_output_w(offs_t offset, u16 data);

	u8 buttons_r(offs_t offset);
	u8 external_triggers_r();
	u8 cassette_r();
	template<int GROUP> void gen_trigger_w(u8 data);

	void memory_map(address_map &map) ATTR_COLD;
	void io_map(address_map &map) ATTR_COLD;

	required_device<z80_device> m_maincpu;
	required_device_array<dl1414_device, 4> m_digit_device;
	output_finder<16> m_digit_output;
	required_device<timer_device> m_metronome_timer;
	required_ioport_array<6> m_buttons;
	required_ioport m_switches;  // Includes foot switches.
	required_ioport m_external_triggers;
	required_ioport m_clk_in;
	output_finder<> m_clk_out_tip;  // Tip conncetion of "CLK OUT" TRS jack.
	output_finder<> m_metronome_mix;
	output_finder<> m_metronome;

	// 40103 timer (U11 in Processor Board)
	u8 m_int_timer_preset = 0xff;
	bool m_int_timer_enabled = true;
	s16 m_int_timer_value = 0xff;
	u8 m_int_timer_out = 1;
	u8 m_int_flipflop_clock = 0;

	// The service manual states that the crystal is 4.912MHz, in the section
	// that describes the clock circuitry. However, the parts list specifies
	// this as a 4.9152 MHz crystal. The parts list value is likely the correct
	// one.
	// Divided by U35, 74LS74.
	static constexpr const XTAL CPU_CLOCK = 4.9152_MHz_XTAL / 2;
	// Divided by U34, 74LS393.
	static constexpr const XTAL INT_TIMER_CLOCK = CPU_CLOCK / 256;
};

void dmx_state::refresh_int_flipflop()
{
	// All component designations refer to the Processor Board.

	// Interrups are used for maintaining tempo. They are generated by an SR
	// flipflop (U23B, 4013), with /Q connected to /INT. D is always 1.
	// If CLK IN is not connected, the flipflop is clocked by the timer's output
	// (U11, 40103).
	// If CLK IN is connected, it will clock the flipflop, and the timer's
	// output will be ignored.
	// The Z80's interrupt acknowledge cycle clears the flipfop (flipflop
	// "clear" connected to NOR(/M1, /IORQ) (U32, 4001)).
	// CSYN (cassette sync) will also trigger interrupts, via the flipflop's
	// "set" input.
	// The circuit ensures that either the timer is enabled, or CSYN signals
	// are generated, but not both. This prevents potentially conflicting
	// interrupts.

	u8 new_int_flipflop_clock = 0;
	if (m_clk_in->read() & 0x01)  // Is CLK IN plugged?
	{
		new_int_flipflop_clock = (m_clk_in->read() & 0x02) ? 1 : 0;
	}
	else
	{
		// 40103 (U11) timer output is inverted by Q1, R20 and R43.
		new_int_flipflop_clock = m_int_timer_out ? 0 : 1;
	}

	if (!m_int_flipflop_clock && new_int_flipflop_clock)  // 0->1 transition.
		m_maincpu->set_input_line(INPUT_LINE_IRQ0, HOLD_LINE);

	m_int_flipflop_clock = new_int_flipflop_clock;
	m_clk_out_tip = m_int_flipflop_clock;
}

void dmx_state::int_timer_preset_w(u8 data)
{
	if (m_int_timer_preset == data)
		return;
	m_int_timer_preset = data;
	LOGMASKED(LOG_INT_TIMER, "INT timer preset: %02x\n", m_int_timer_preset);
}

void dmx_state::int_timer_enable_w(int state)
{
	// When the INT timer is enabled, the cassette sync INT generator is
	// disabled, and vice versa. Cassette functionality (including sync) is
	// not currently emulated.
	if (m_int_timer_enabled == bool(state))
		return;

	// The CLEN signal is inverted by U28 (4011 NAND), but it is active low
	// since it is connected on the /CE input of a 40103. So, logically,
	// "1" == "enabled".
	m_int_timer_enabled = bool(state);
	LOGMASKED(LOG_INT_TIMER, "INT timer enabled: %d\n", m_int_timer_enabled);
}

TIMER_DEVICE_CALLBACK_MEMBER(dmx_state::int_timer_tick)
{
	if (!m_int_timer_enabled)
		return;

	if (m_int_timer_value <= 0)  // We reached 0 in the previous clock cycle.
	{
		// The 40103 timer is configured for synchronous loading of the preset
		// value. So it starts over in the next clock cycle after reaching 0.
		// TODO: Actually verify that a preset of 0 means 256. Not clear in
		// the datasheet.
		if (m_int_timer_preset == 0)
			m_int_timer_value = 256;
		else
			m_int_timer_value = m_int_timer_preset;

		// The timer's output transitions to 1 in the clock cycle after reaching
		// 0 (the one we are handling here).
		m_int_timer_out = 1;
		refresh_int_flipflop();
		return;
	}

	--m_int_timer_value;

	if (m_int_timer_value <= 0)
	{
		// The timer's output goes low when its counter reaches 0.
		m_int_timer_out = 0;
		refresh_int_flipflop();
	}
}

void dmx_state::metronome_trigger_w(u8 /*data*/)
{
	// Writing to this port clocks U33A (D-flipflop). R62 and C32 form an RC
	// network that resets the flipflop after ~1ms. This becomes a 1ms, 10-12V
	// pulse (via Q8, R55, R54) on the "metronome out" connection.
	// (component designations refer to the Processor Board)
	m_metronome_timer->adjust(attotime::from_msec(1));
	m_metronome = 1;
}

void dmx_state::metronome_mix_w(u8 data)
{
	// D0 connected to D of U35 74LS74 (D-flipflop), which is clocked by
	// CLICK* from the address decoder (U7, 74LS42).

	// The service manual states that this bit enables mixing the metronome into
	// the main mix, and that the LVI signal controls the volume. But that seems
	// to describe how the firmware uses these. Looking at the schematic, it
	// seems that both this signal and LVI would enable mixing on their own, and
	// setting both to 1 would increase the volume.
	m_metronome_mix = BIT(data, 0);
}

TIMER_DEVICE_CALLBACK_MEMBER(dmx_state::metronome_timer_tick)
{
	m_metronome = 0;
}

void dmx_state::display_w(offs_t offset, u8 data)
{
	// Decoding between different displays is done by U2 (74LS42). The
	// m_digit_device array is ordered from right to left (U6-U3). All
	// components are on the Switch Board.
	m_digit_device[offset >> 2]->bus_w(offset & 0x03, data);
}

template<int DISPLAY> void dmx_state::display_output_w(offs_t offset, u16 data)
{
	// The displays and the digits within each display are addressed from right
	// to left with increassing offset. Reversing that order in m_digit_output,
	// since left-to-right is more convenient for building the layout.
	static_assert(DISPLAY >= 0 && DISPLAY < 4);
	const int display_in_layout = 3 - DISPLAY;
	const int digit_in_layout = 3 - (offset & 0x03);
	m_digit_output[4 * display_in_layout + digit_in_layout] = data;
}

u8 dmx_state::buttons_r(offs_t offset)
{
	// Switches are inverted and buffered by 2 x 80C98 (inverting 3-state
	// buffers), U7 and U8 on Switch Board.
	return ~m_buttons[offset]->read();
}

u8 dmx_state::external_triggers_r()
{
	// External triggers are inverted by 2 X CA3086 NPN arrays (U4 for D0-D3 and
	// U5 for D4-D7) and buffered by U6 (74LS244). All components are on the
	// Processor Board.
	return ~m_external_triggers->read();
}

u8 dmx_state::cassette_r()
{
	// All components are on the Processor Board.
	// Inputs buffered by U19 (75LS244). This is a mix of cassette-related
	// inputs and the states of foot-switches and toggle-switches.

	const u8 data = m_switches->read();

	const u8 d0 = 0;  // CD0: not yet implemented.
	const u8 d1 = 0;  // CD1: not yet implemented.
	const u8 d2 = 0;  // CD2: not yet implemented.

	const u8 d3 = BIT(data, 3);  // CASEN* (cassette enable switch. Active low).

	// START footswitch input is pulled high (R41) and ANDed with 1 (U22,
	// 74LS08, with the other input tied high via R42). So U22 is logically a
	// no-op. However, the NEXT footswitch is tied directly to U19.
	const u8 d4 = BIT(data, 4);  // START* (output of U22).
	const u8 d5 = BIT(data, 5);  // NEXT (pulled high by R58).
	const u8 d6 = BIT(data, 6);  // TSYN* (pulled high by R23).
	const u8 d7 = BIT(data, 7);  // PROT* (memory protect switch. Active low).

	return (d7 << 7) | (d6 << 6) | (d5 << 5) | (d4 << 4) |
	       (d3 << 3) | (d2 << 2) | (d1 << 1) | d0;
}

template<int GROUP> void dmx_state::gen_trigger_w(u8 data)
{
	// Triggers are active-high and only last for the duration of the write.
	// They are implemented by enabling an 74LS244 (U8 and U9) whose outputs are
	// normally pulled low.

	static_assert(GROUP >= 0 && GROUP <= 1);
	static constexpr const int NUM_VOICE_CARDS = 8;
	static constexpr const int TRIGGER_MAPPINGS[2][NUM_VOICE_CARDS] =
	{
		{0, 2, 4, 6, 1, 3, 5, 7},
		{8, 14, 10, 12, 9, 15, 11, 13},
	};

	if (data == 0)
		return;

	std::vector<bool> triggers(2 * NUM_VOICE_CARDS, false);
	for (int i = 0; i < NUM_VOICE_CARDS; ++i)
		if (BIT(data, i))
			triggers[TRIGGER_MAPPINGS[GROUP][i]] = true;

	for (int i = 0; i < NUM_VOICE_CARDS; ++i)
	{
		const bool tr0 = triggers[2 * i];
		const bool tr1 = triggers[2 * i + 1];
		if (tr0 || tr1)
		{
			LOGMASKED(LOG_TRIGGERS, "Triggered Voice: %d, tr0: %d, tr1: %d\n",
			          i, tr0, tr1);
		}
	}
}

void dmx_state::memory_map(address_map &map)
{
	// Component designations refer to the Processor Board.
	// Memory decoding done by U2 (74LS42) and U22 (74LS08).
	map.global_mask(0x3fff);  // A14 and A15 not used.
	map(0x0000, 0x1fff).rom();  // 2 x 2732 (4K x 8bit) ROMs, U18-U17.
	// 4 x 6116 (2K x 8bit) RAMs, U16-U13. U3 (4071, OR-gate) will only allow
	// RAM select signals to go through if fully powered up.
	map(0x2000, 0x3fff).ram().share(NVRAM_TAG);
}

void dmx_state::io_map(address_map &map)
{
	// Signal names (e.g. GEN0*) match those in the schematics.

	map.global_mask(0x7f);  // A7-A15 not used for port decoding or I/O.

	// Write decoding done by U7 (74LS42) on the Processor Board.
	map(0x00, 0x0f).w(FUNC(dmx_state::display_w));  // ID*.
	map(0x10, 0x10).mirror(0x0f).w(FUNC(dmx_state::gen_trigger_w<0>));  // GEN0*
	map(0x20, 0x20).mirror(0x0f).w(FUNC(dmx_state::gen_trigger_w<1>));  // GEN1*
	map(0x30, 0x30).mirror(0x0f).w(FUNC(dmx_state::int_timer_preset_w));  // STIM*
	map(0x40, 0x40).mirror(0x0f).w(FUNC(dmx_state::metronome_trigger_w));  // METRONOME OUT
	map(0x50, 0x50).mirror(0x0f).w("cas_latch", FUNC(output_latch_device::write));  // CAS0*
	map(0x60, 0x60).mirror(0x0f).w(FUNC(dmx_state::metronome_mix_w));  // CLICK*

	// Read decoding done by U9 (74LS42) on the Switch Board.
	map(0x00, 0x05).mirror(0x78).r(FUNC(dmx_state::buttons_r));  // IORD*
	map(0x06, 0x06).mirror(0x78).r(FUNC(dmx_state::external_triggers_r));  // SW6*
	map(0x07, 0x07).mirror(0x78).r(FUNC(dmx_state::cassette_r));  // CASI*
}

void dmx_state::machine_start()
{
	m_clk_out_tip.resolve();
	m_metronome_mix.resolve();
	m_metronome.resolve();
	m_digit_output.resolve();

	save_item(NAME(m_int_timer_preset));
	save_item(NAME(m_int_timer_enabled));
	save_item(NAME(m_int_timer_value));
	save_item(NAME(m_int_timer_out));
	save_item(NAME(m_int_flipflop_clock));
}

void dmx_state::dmx(machine_config &config)
{
	Z80(config, m_maincpu, CPU_CLOCK);
	m_maincpu->set_addrmap(AS_PROGRAM, &dmx_state::memory_map);
	m_maincpu->set_addrmap(AS_IO, &dmx_state::io_map);

	NVRAM(config, NVRAM_TAG, nvram_device::DEFAULT_ALL_0);

	// 40103 timer (U11 on Processor Board). See refresh_int_flipflop() and
	// int_timer_tick().
	TIMER(config, "int_timer_u11").configure_periodic(
		FUNC(dmx_state::int_timer_tick), attotime::from_hz(INT_TIMER_CLOCK));

	// A one-shot timer based on an RC-cicruit. See metronome_trigger_w().
	TIMER(config, m_metronome_timer).configure_generic(
		FUNC(dmx_state::metronome_timer_tick));

	DL1414T(config, m_digit_device[0], 0U).update().set(FUNC(dmx_state::display_output_w<0>));
	DL1414T(config, m_digit_device[1], 0U).update().set(FUNC(dmx_state::display_output_w<1>));
	DL1414T(config, m_digit_device[2], 0U).update().set(FUNC(dmx_state::display_output_w<2>));
	DL1414T(config, m_digit_device[3], 0U).update().set(FUNC(dmx_state::display_output_w<3>));

	config.set_default_layout(layout_oberheim_dmx);

	// 74C174 (U20, on Processor Board), 6-bit latch.
	output_latch_device &cas_latch(OUTPUT_LATCH(config, "cas_latch"));
	cas_latch.bit_handler<0>().set_output("CDATO");
	cas_latch.bit_handler<1>().set_output("ARM");
	cas_latch.bit_handler<2>().set(FUNC(dmx_state::int_timer_enable_w));
	// Bit 3 is an open-collector connection to the Ring of the "CLK OUT" TRS jack.
	cas_latch.bit_handler<3>().set_output("CLK_OUT_ring").invert();
	cas_latch.bit_handler<4>().set_output("LVI");
	// Bit 5 not connected.
}

DECLARE_INPUT_CHANGED_MEMBER(dmx_state::clk_in_changed)
{
	refresh_int_flipflop();
}

INPUT_PORTS_START(dmx)
	PORT_START("buttons_0")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("2") PORT_CODE(KEYCODE_2_PAD)
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("3") PORT_CODE(KEYCODE_3_PAD)
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("4") PORT_CODE(KEYCODE_4_PAD)
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("5") PORT_CODE(KEYCODE_5_PAD)
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("6") PORT_CODE(KEYCODE_6_PAD)
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("7") PORT_CODE(KEYCODE_7_PAD)
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("8") PORT_CODE(KEYCODE_8_PAD)
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("9") PORT_CODE(KEYCODE_9_PAD)

	PORT_START("buttons_1")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("0") PORT_CODE(KEYCODE_0_PAD)
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("1") PORT_CODE(KEYCODE_1_PAD)
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("<") PORT_CODE(KEYCODE_LEFT)
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME(">") PORT_CODE(KEYCODE_RIGHT)
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("SWING") PORT_CODE(KEYCODE_BACKSPACE)
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("SONG") PORT_CODE(KEYCODE_INSERT)
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("EDIT") PORT_CODE(KEYCODE_HOME)
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("STEP") PORT_CODE(KEYCODE_PGUP)

	PORT_START("buttons_2")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("RECORD") PORT_CODE(KEYCODE_ENTER)
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("ERASE") PORT_CODE(KEYCODE_DEL)
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("COPY") PORT_CODE(KEYCODE_END)
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("LENGTH") PORT_CODE(KEYCODE_L)
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("PLAY") PORT_CODE(KEYCODE_SPACE)
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("TEMPO") PORT_CODE(KEYCODE_OPENBRACE)
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("QUANT") PORT_CODE(KEYCODE_BACKSLASH)
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("SIGN") PORT_CODE(KEYCODE_CLOSEBRACE)

	PORT_START("buttons_3")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("BASS 3") PORT_CODE(KEYCODE_Z)
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("SNARE 3") PORT_CODE(KEYCODE_X)
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("HI-HAT OPEN") PORT_CODE(KEYCODE_C)
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("TOM 3") PORT_CODE(KEYCODE_V)
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("TOM 6") PORT_CODE(KEYCODE_B)
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("CYMB 3") PORT_CODE(KEYCODE_N)
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("RIMSHOT") PORT_CODE(KEYCODE_M)
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("CLAPS") PORT_CODE(KEYCODE_COMMA)

	PORT_START("buttons_4")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("BASS 2") PORT_CODE(KEYCODE_A)
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("SNARE 2") PORT_CODE(KEYCODE_S)
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("HI-HAT ACC") PORT_CODE(KEYCODE_D)
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("TOM 2") PORT_CODE(KEYCODE_F)
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("TOM 5") PORT_CODE(KEYCODE_G)
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("CYMB 2") PORT_CODE(KEYCODE_H)
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("TAMP 2") PORT_CODE(KEYCODE_J)
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("SHAKE 2") PORT_CODE(KEYCODE_K)

	PORT_START("buttons_5")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("BASS 1") PORT_CODE(KEYCODE_Q)
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("SNARE 1") PORT_CODE(KEYCODE_W)
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("HI-HAT CLOSED") PORT_CODE(KEYCODE_E)
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("TOM 1") PORT_CODE(KEYCODE_R)
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("TOM 4") PORT_CODE(KEYCODE_T)
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("CYMB 1") PORT_CODE(KEYCODE_Y)
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("TAMB 1") PORT_CODE(KEYCODE_U)
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("SHAKE 1") PORT_CODE(KEYCODE_I)

	PORT_START("switches")
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("CASSETTE ENABLE") PORT_TOGGLE
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("START")  // Footswitch.
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("NEXT")  // Footswitch.
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("SYNC IN middle")  // Plug input.
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("MEMORY PROTECT") PORT_TOGGLE

	PORT_START("external_triggers")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_OTHER) PORT_NAME("BASS TRIG")
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_OTHER) PORT_NAME("PERC1 TRIG")
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_OTHER) PORT_NAME("CYMBAL TRIG")
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_OTHER) PORT_NAME("TOM2 TRIG")
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_OTHER) PORT_NAME("TOM1 TRIG")
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_OTHER) PORT_NAME("HI-HAT TRIG")
	PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_OTHER) PORT_NAME("SNARE TRIG")
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_OTHER) PORT_NAME("PERC2 TRIG")

	PORT_START("clk_in")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_OTHER) PORT_NAME("CLK IN plugged") PORT_CODE(KEYCODE_QUOTE) PORT_TOGGLE
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_OTHER) PORT_NAME("CLK IN") PORT_CODE(KEYCODE_SLASH)
		PORT_CHANGED_MEMBER(DEVICE_SELF, FUNC(dmx_state::clk_in_changed), 0)

	// Fader potentiometers. P1-P10 on the Switch Board.
	PORT_START("fader_p1")
	PORT_ADJUSTER(100, "BASS")
	PORT_START("fader_p2")
	PORT_ADJUSTER(100, "SNARE")
	PORT_START("fader_p3")
	PORT_ADJUSTER(100, "HI-HAT")
	PORT_START("fader_p4")
	PORT_ADJUSTER(100, "TOM1")
	PORT_START("fader_p5")
	PORT_ADJUSTER(100, "TOM2")
	PORT_START("fader_p6")
	PORT_ADJUSTER(100, "CYMBAL")
	PORT_START("fader_p7")
	PORT_ADJUSTER(100, "PERC1")
	PORT_START("fader_p8")
	PORT_ADJUSTER(100, "PERC2")
	PORT_START("fader_p9")
	PORT_ADJUSTER(100, "MET")
	PORT_START("fader_p10")
	PORT_ADJUSTER(100, "VOLUME")
INPUT_PORTS_END

ROM_START(obdmx)
	ROM_REGION(0x2000, MAINCPU_TAG, 0)  // 2 x 2732 (4K x 8bit) ROMs, U18 and U17.
	ROM_DEFAULT_BIOS("rev_f")

	ROM_SYSTEM_BIOS(0, "debug", "DMX debug firmware")
	ROMX_LOAD("dmx_dbg.u18", 0x000000, 0x001000, CRC(b3a24d64) SHA1(99a64b92f1520aaf2e7117b2fc75bdbd847f3a7f), ROM_BIOS(0))
	ROMX_FILL(0x001000, 0x001000, 0xff, ROM_BIOS(0))  // No ROM needed in U17.

	ROM_SYSTEM_BIOS(1, "rev_b", "DMX 1.00 (Revision B), 1981")
	ROMX_LOAD("dmx_b0.u18", 0x000000, 0x001000, CRC(69116c5b) SHA1(29448b60d7f864b0a9df2a786b877feb7b019c05), ROM_BIOS(1))
	ROMX_LOAD("dmx_b1.u17", 0x001000, 0x001000, CRC(49084dee) SHA1(fffe4e6592dc586c0a0f03bed18308df38b991da), ROM_BIOS(1))

	ROM_SYSTEM_BIOS(2, "rev_f", "DMX 2.21 (Revision F), 1982")
	ROMX_LOAD("dmx_f0.u18", 0x000000, 0x001000, CRC(a01e5e7d) SHA1(74faf9c056c7e3eb89ed7657d8b64b1d3fe0ec22), ROM_BIOS(2))
	ROMX_LOAD("dmx_f1.u17", 0x001000, 0x001000, CRC(b24d4040) SHA1(74e7ae87cdcf9442cf76b696970c0efe52a30262), ROM_BIOS(2))
ROM_END

}  // anonymous namespace

SYST(1980, obdmx, 0, 0, dmx, dmx, dmx_state, empty_init, "Oberheim", "DMX", MACHINE_SUPPORTS_SAVE | MACHINE_NOT_WORKING | MACHINE_NO_SOUND)

