// license:BSD-3-Clause
// copyright-holders:m1macrophage

/*
The LinnDrum (unofficially also known as LM-2) is a digital drum machine. It
stores and plays digital samples (recordings of acoustic dumps), some of which
are processed by analog filters or other analog circuits.

The firmware runs on a Z80. It controls the UI (reads buttons, drives displays
and LEDs), synchronization with other devices, cassette I/O, and voice
triggering.

The LinnDrum has 12-voice polyphony, not including the "click" (metronome).
It has a total of 15 voices, along with a "click" sound. Some of the voices have
variations, which brings the total to 24 different sounds.

The driver is based on the LinnDrum's service manual and schematics, and is
intended as an educational tool.

Most of the digital functionality is emulated. Audio is not yet emulated.

PCBoards:
* CPU board. 2 sections in schematics:
  * Computer.
  * Input/Output.
* DRM board. 4 sections in schematics:
  * MUX drums.
  * MUX drum output stage.
  * Snare/Sidestick.
  * Tom/Cga.
* MXR board.
* 5 VSR board (power supply).

Usage:

The driver includes an interactive layout.

Since there is no audio, the driver logs triggers and other info. To see those,
run the driver with `-log`:
./mame -window linndrum -log

Tail the log file:
(on linux): tail -f error.log
*/

#include "emu.h"
#include "cpu/z80/z80.h"
#include "machine/nvram.h"
#include "machine/output_latch.h"
#include "machine/rescap.h"
#include "machine/timer.h"

#include "linn_linndrum.lh"

#define LOG_KEYBOARD         (1U << 1)
#define LOG_DEBOUNCE         (1U << 2)
#define LOG_TEMPO            (1U << 3)
#define LOG_TEMPO_CHANGE     (1U << 4)
#define LOG_TRIGGERS         (1U << 5)
#define LOG_TAPE_SYNC_ENABLE (1U << 6)

#define VERBOSE (LOG_GENERAL | LOG_TEMPO_CHANGE | LOG_TAPE_SYNC_ENABLE | LOG_TRIGGERS)
//#define LOG_OUTPUT_FUNC osd_printf_info

#include "logmacro.h"

namespace {

constexpr const char MAINCPU_TAG[] = "z80";
constexpr const char NVRAM_TAG[] = "nvram";

class linndrum_state : public driver_device
{
public:
	linndrum_state(const machine_config &mconfig, device_type type, const char *tag) ATTR_COLD
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, MAINCPU_TAG)
		, m_tempo_timer(*this, "tempo_timer_556_u30a")
		, m_debounce_timer(*this, "debounce_timer_556_u30b")
		, m_keyboard(*this, "keyboard_col_%d", 0)
		, m_playstop(*this, "play_stop")
		, m_triggers(*this, "trigger_inputs")
		, m_tempo_pot(*this, "pot_tempo")
		, m_step_display(*this, "display_step_%d", 0U)
		, m_pattern_display(*this, "display_pattern_%d", 0U)
		, m_tape_sync_out(*this, "output_tape_sync")
	{
	}

	void linndrum(machine_config &config) ATTR_COLD;

	DECLARE_INPUT_CHANGED_MEMBER(tempo_pot_adjusted);

protected:
	void machine_start() override ATTR_COLD;
	void machine_reset() override ATTR_COLD;

private:
	u8 keyboard_r(offs_t offset);
	u8 inport_r();
	template<int DISPLAY> void display_w(u8 data);

	u8 start_debounce_r();
	void start_debounce_w(u8 data);
	TIMER_DEVICE_CALLBACK_MEMBER(debounce_timer_elapsed);

	void update_tempo_timer();
	TIMER_DEVICE_CALLBACK_MEMBER(tempo_timer_tick);

	void trigger_w(offs_t offset, u8 data);
	void trigger_beep_w(int state);
	void data_out_enable_w(int state);
	void tape_sync_enable_w(int state);
	void update_tape_sync_out();

	void memory_map(address_map &map) ATTR_COLD;
	void io_map(address_map &map) ATTR_COLD;

	required_device<z80_device> m_maincpu;
	required_device<timer_device> m_tempo_timer;  // 556, U30A.
	required_device<timer_device> m_debounce_timer;  // 556, U30B.
	required_ioport_array<6> m_keyboard;
	required_ioport m_playstop;
	required_ioport m_triggers;
	required_ioport m_tempo_pot;
	output_finder<2> m_step_display;  // 2 x MAN4710.
	output_finder<2> m_pattern_display;  // 2 x MAN4710.
	output_finder<> m_tape_sync_out;

	bool m_debouncing = false;
	bool m_tempo_state = false;
	bool m_tape_sync_enabled = false;
	bool m_data_out_enabled = false;

	static constexpr const int NUM_VOICE_TRIGGERS = 11;
	static constexpr const char *VOICE_TRIGGER_NAMES[NUM_VOICE_TRIGGERS] =
	{
		"BASS", "SNARE", "HI-HAT", "TOM / CGA", "RIDE", "CRASH", "CABASA",
		"TAMB", "COWBELL", "CLAP", "CLICK"
	};

	static constexpr const int DISPLAY_STEP = 0;
	static constexpr const int DISPLAY_PATTERN = 1;
};

u8 linndrum_state::keyboard_r(offs_t offset)
{
	// Columns in the keyboard matrix are selected by A0-A5. Each of these
	// address lines are inverted by U29 (74LS05). A high at Ax enables column
	// x. If more than one of A0-A5 are set, multiple columns will be selected.
	// Rows are pulled up by a resistor array (RP8), buffered by U28 (74LS244)
	// and connected to D0-D5.
	u8 selected = 0x3f;  // D0-D5.
	for (int i = 0; i < 6; ++i)
		if (BIT(offset, i))
			selected &= m_keyboard[i]->read();

	const u8 d6 = m_debouncing ? 1 : 0;

	// The play/stop button is not part of the keyboard matrix. It is always
	// read at D7, regardless of the selected column. The play/stop button and
	// footswitch are wired such that if any of them is grounded (activated) a
	// 0 will be returned in D7.
	const bool play_button_pressed = !(m_playstop->read() & 0x01);
	const bool play_footswitch_pressed = !(m_playstop->read() & 0x02);
	const u8 d7 = (play_button_pressed || play_footswitch_pressed) ? 0 : 1;

	if (selected != 0x3f || d7 == 0)
	{
		LOGMASKED(LOG_KEYBOARD,
				"Offset: %02x, keys: %02x, debounce: %d, play: %d\n",
				offset, selected, d6, d7);
	}

	return (d7 << 7) | (d6 << 6) | selected;
}

u8 linndrum_state::inport_r()
{
	const u8 d0 = m_tempo_state ? 1 : 0;  // Tempo.

	// The cassette input and sync input circuits are identical.
	// TODO: Determine value at rest.
	const u8 d1 = 0;  // Sync input. TODO: implement.
	const u8 d2 = 0;  // Cassette input. TODO: implement.

	const u8 triggers = m_triggers->read() & 0x1f;

	return (triggers << 3) | (d2 << 2) | (d1 << 1) | d0;
}

template<int DISPLAY> void linndrum_state::display_w(u8 data)
{
	static constexpr const u8 PATTERNS[16] = // 4 x 74LS47 (U24-U27).
	{
		0x3f, 0x06, 0x5b, 0x4f, 0x66, 0x6d, 0x7c, 0x07,
		0x7f, 0x67, 0x58, 0x4c, 0x62, 0x69, 0x78, 0x00,
	};

	const u8 ms_digit = PATTERNS[(data >> 4) & 0x0f];
	const u8 ls_digit = PATTERNS[data & 0x0f];

	static_assert(DISPLAY == DISPLAY_STEP || DISPLAY == DISPLAY_PATTERN);
	if (DISPLAY == DISPLAY_STEP)
	{
		m_step_display[0] = ms_digit;
		m_step_display[1] = ls_digit;
	}
	else
	{
		m_pattern_display[0] = ms_digit;
		m_pattern_display[1] = ls_digit;
	}
}

u8 linndrum_state::start_debounce_r()
{
	if (!machine().side_effects_disabled())
		start_debounce_w(0);
	return 0x00;  // D0-D7 pulled low by RP10.
}

void linndrum_state::start_debounce_w(u8 /*data*/)
{
	static constexpr const float R3 = RES_K(30);
	static constexpr const float C3 = CAP_U(0.1);
	static constexpr attotime INTERVAL = PERIOD_OF_555_MONOSTABLE(R3, C3);

	m_debouncing = true;
	m_debounce_timer->adjust(INTERVAL);
	LOGMASKED(LOG_DEBOUNCE, "Debounce timer: start\n");
}

TIMER_DEVICE_CALLBACK_MEMBER(linndrum_state::debounce_timer_elapsed)
{
	m_debouncing = false;
	LOGMASKED(LOG_DEBOUNCE, "Debounce timer: elapsed\n");
}

void linndrum_state::update_tempo_timer()
{
	static constexpr const float R1 = RES_K(5.1);
	static constexpr const float R2 = RES_K(18);
	static constexpr const float C2 = CAP_U(0.1);
	static constexpr const float P1_MAX = RES_K(100);

	// Using `100 - pot value` because the higher (the more clockwise) the pot
	// is turned, the lower the resistance and the fastest the tempo.
	const float tempo_r = (100 - m_tempo_pot->read()) * P1_MAX / 100.0f;
	const attotime period = PERIOD_OF_555_ASTABLE(R1, R2 + tempo_r, C2);
	m_tempo_timer->adjust(period, 0, period);
	LOGMASKED(LOG_TEMPO_CHANGE, "Tempo adjusted: %f\n", period.as_double());
}

TIMER_DEVICE_CALLBACK_MEMBER(linndrum_state::tempo_timer_tick)
{
	// The output of the 556 clocks an 74LS74 (U8B) that is wired in a
	// divide-by-2 configuration. Each time the timer elapses, m_tempo_state
	// will get negated.
	m_tempo_state = !m_tempo_state;
	update_tape_sync_out();
	LOGMASKED(LOG_TEMPO, "Tempo timer elapsed: %d\n", m_tempo_state);
}

void linndrum_state::trigger_w(offs_t offset, u8 data)
{
	assert(offset >= 0 && offset < NUM_VOICE_TRIGGERS);
	LOGMASKED(LOG_TRIGGERS, "Trigger %s (%02x), data: %02x, data enabled: %d\n",
			VOICE_TRIGGER_NAMES[offset], offset, data, m_data_out_enabled);
	// TODO: Implement.
}

void linndrum_state::trigger_beep_w(int state)
{
	LOGMASKED(LOG_TRIGGERS, "Trigger BEEP: %d, data enabled: %d\n",
			state, m_data_out_enabled);
	// TODO: Implement.
}

void linndrum_state::data_out_enable_w(int state)
{
	// Controls whether data (D0-D3) is transmitted to the voice circuits. This
	// is done by U19 (74LS365 hex buffer. Enable inputs are active-low).
	// This is usually disabled to prevent interference from the "noisy" data
	// bus to the voice circuits.
	m_data_out_enabled = !state;
}

void linndrum_state::tape_sync_enable_w(int state)
{
	LOGMASKED(LOG_TAPE_SYNC_ENABLE, "Tape sync enable: %d\n", state);
	m_tape_sync_enabled = bool(state);
	update_tape_sync_out();
}

void linndrum_state::update_tape_sync_out()
{
	// tape_sync_enable (U16 latch, O5) is NANDed with the current tempo
	// state (U34, 74LS00). The output is then inverted by Q3/R20/R21.
	if (m_tape_sync_enabled)
		m_tape_sync_out = m_tempo_state ? 1 : 0;
	else
		m_tape_sync_out = 0;
}

void linndrum_state::memory_map(address_map &map)
{
	// Signal names (such as "/READ INPORT") are taken from the schematics.

	map.global_mask(0x3fff);  // A14 and A15 are not used.

	map(0x0000, 0x1f7f).rom();  // 2 x 2732. U14-U13.
	map(0x1f80, 0x1f80).mirror(0x003f).r(FUNC(linndrum_state::inport_r));  // /READ INPORT.

	// Writes to: 0x1f80 - 0x1fbf: /OUT STROBE EN.
	// Outputs are selected by two 74LS138 (U20, U21). U20 is enabled when A3 is
	// 0, U21 when it is 1.
	map(0x1f80, 0x1f80).mirror(0x0030).w(FUNC(linndrum_state::display_w<DISPLAY_PATTERN>));  // U23 latch.
	map(0x1f81, 0x1f81).mirror(0x0030).w(FUNC(linndrum_state::display_w<DISPLAY_STEP>));  // U22 latch.
	map(0x1f82, 0x1f82).mirror(0x0030).w("latch_u18", FUNC(output_latch_device::write));  // LEDs.
	map(0x1f83, 0x1f83).mirror(0x0030).w("latch_u17", FUNC(output_latch_device::write));  // LEDs.
	map(0x1f84, 0x1f84).mirror(0x0030).w("latch_u16", FUNC(output_latch_device::write));  // LEDs & outputs.
	map(0x1f85, 0x1f8f).mirror(0x0030).w(FUNC(linndrum_state::trigger_w));  // Data out.

	map(0x1fc0, 0x1fff).r(FUNC(linndrum_state::keyboard_r));  // /READ KEYBD.
	map(0x2000, 0x3fff).ram().share(NVRAM_TAG); // 4 x HM6116LP4. U12-U9.
}

void linndrum_state::io_map(address_map &map)
{
	map.global_mask(0xff);

	// The debouncing timer is started by the Z80's /IORQ signal itself. So
	// there is no difference between read and write, and the address and data
	// bits don't matter.
	map(0x00, 0x00).mirror(0xff).r(FUNC(linndrum_state::start_debounce_r)).w(FUNC(linndrum_state::start_debounce_w));
}

void linndrum_state::machine_start()
{
	m_step_display.resolve();
	m_pattern_display.resolve();
	m_tape_sync_out.resolve();

	save_item(NAME(m_debouncing));
	save_item(NAME(m_tempo_state));
	save_item(NAME(m_tape_sync_enabled));
	save_item(NAME(m_data_out_enabled));
}

void linndrum_state::machine_reset()
{
	update_tempo_timer();
}

void linndrum_state::linndrum(machine_config &config)
{
	// D0-D7 and A0-A11 are pulled low by RP10 and RP9.
	Z80(config, m_maincpu, 4_MHz_XTAL / 2);  // Divided by 74LS74, U8A.
	m_maincpu->set_addrmap(AS_PROGRAM, &linndrum_state::memory_map);
	m_maincpu->set_addrmap(AS_IO, &linndrum_state::io_map);

	NVRAM(config, NVRAM_TAG, nvram_device::DEFAULT_ALL_0);

	TIMER(config, m_tempo_timer).configure_generic(  // 556, U30A.
		FUNC(linndrum_state::tempo_timer_tick));
	TIMER(config, m_debounce_timer).configure_generic(  // 556, U30B.
		FUNC(linndrum_state::debounce_timer_elapsed));

	config.set_default_layout(layout_linn_linndrum);

	// Latches connected to cathodes of LEDs (through resistors), so they are
	// active-low.

	output_latch_device &u18(OUTPUT_LATCH(config, "latch_u18"));
	u18.bit_handler<0>().set_output("led_1_8").invert();
	u18.bit_handler<1>().set_output("led_1_8_t").invert();
	u18.bit_handler<2>().set_output("led_1_16").invert();
	u18.bit_handler<3>().set_output("led_1_16_t").invert();
	u18.bit_handler<4>().set_output("led_1_32").invert();
	u18.bit_handler<5>().set_output("led_1_32_t").invert();
	u18.bit_handler<6>().set_output("led_hi").invert();
	u18.bit_handler<7>().set_output("led_play_stop").invert();

	output_latch_device &u17(OUTPUT_LATCH(config, "latch_u17"));
	u17.bit_handler<0>().set_output("led_a").invert();
	u17.bit_handler<1>().set_output("led_b").invert();
	u17.bit_handler<2>().set_output("led_c").invert();
	u17.bit_handler<3>().set_output("led_d").invert();
	u17.bit_handler<4>().set_output("led_e").invert();
	u17.bit_handler<5>().set_output("led_f").invert();
	u17.bit_handler<6>().set_output("led_load").invert();
	u17.bit_handler<7>().set_output("led_store").invert();

	output_latch_device &u16(OUTPUT_LATCH(config, "latch_u16"));
	u16.bit_handler<0>().set_output("led_percussion").invert();
	u16.bit_handler<1>().set_output("led_ext_sync").invert();
	u16.bit_handler<2>().set_output("led_pattern").invert();
	u16.bit_handler<2>().append_output("led_song");  // Inverted by U31A.
	u16.bit_handler<3>().set(FUNC(linndrum_state::data_out_enable_w));
	u16.bit_handler<4>().set(FUNC(linndrum_state::trigger_beep_w));
	u16.bit_handler<5>().set(FUNC(linndrum_state::tape_sync_enable_w));
	// Output voltage divided by R24/R25 to 0V - 2.5V.
	u16.bit_handler<6>().set_output("output_cassette");
	// Inverted by Q4/R22/R23.
	u16.bit_handler<7>().set_output("output_trigger").invert();
}

DECLARE_INPUT_CHANGED_MEMBER(linndrum_state::tempo_pot_adjusted)
{
	update_tempo_timer();
}

// PORT_NAMEs are based on the annotations in the schematic.
INPUT_PORTS_START(linndrum)
	PORT_START("keyboard_col_0")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("1") PORT_CODE(KEYCODE_1)
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("2") PORT_CODE(KEYCODE_2)
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("3") PORT_CODE(KEYCODE_3)
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("4") PORT_CODE(KEYCODE_4)
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("5") PORT_CODE(KEYCODE_5)
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("6") PORT_CODE(KEYCODE_6)

	PORT_START("keyboard_col_1")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("7") PORT_CODE(KEYCODE_7)
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("SONG/PAT.") PORT_CODE(KEYCODE_Q)
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("SONG#") PORT_CODE(KEYCODE_W)
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("END") PORT_CODE(KEYCODE_E)
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("DELETE") PORT_CODE(KEYCODE_R)
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("INSERT") PORT_CODE(KEYCODE_T)

	PORT_START("keyboard_col_2")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("<-") PORT_CODE(KEYCODE_Y)
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("->") PORT_CODE(KEYCODE_U)
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("ENTER") PORT_CODE(KEYCODE_I)
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("STORE") PORT_CODE(KEYCODE_P)
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("LOAD") PORT_CODE(KEYCODE_COLON)
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("EXT.SYNC") PORT_CODE(KEYCODE_L)

	PORT_START("keyboard_col_3")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("BPM/TRIG.") PORT_CODE(KEYCODE_O)
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("SIDESTICK") PORT_CODE(KEYCODE_Z)
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("SNARE 1") PORT_CODE(KEYCODE_X)
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("SNARE 2") PORT_CODE(KEYCODE_C)
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("SNARE 3") PORT_CODE(KEYCODE_V)
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("BASS 1") PORT_CODE(KEYCODE_B)

	PORT_START("keyboard_col_4")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("BASS 2") PORT_CODE(KEYCODE_N)
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("CRASH") PORT_CODE(KEYCODE_M)
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("PERC.") PORT_CODE(KEYCODE_COMMA)
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("CABASA1 / HIHAT1") PORT_CODE(KEYCODE_A)
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("CABASA2 / HIHAT2") PORT_CODE(KEYCODE_S)
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("TAMP 1 / HIHAT O") PORT_CODE(KEYCODE_D)

	PORT_START("keyboard_col_5")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("TAMP 2 / HI TOM") PORT_CODE(KEYCODE_F)
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("HI CONGA / MID TOM") PORT_CODE(KEYCODE_G)
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("LO CONGA / LO TOM") PORT_CODE(KEYCODE_H)
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("COWBELL / RIDE 1") PORT_CODE(KEYCODE_J)
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("CLAPS / RIDE 2") PORT_CODE(KEYCODE_K)
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_UNUSED)

	// The play-stop button and footswitch input are both connected together
	// in a way that if any of them is low, D6 of the keyboard port will read
	// low.
	PORT_START("play_stop")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("PLAY/STOP") PORT_CODE(KEYCODE_SPACE)
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("PLAY/STOP FOOTSWITCH")

	// The trigger circuit will detect fast changes in the "trigger" inputs, and
	// output a pulse. Slow changes won't trigger a pulse. So there is some
	// flexibility in the types of input accepted. For now, trigger inputs
	// are implemented as digital inputs (traditional trigger signals).
	// TODO: Determine if active high or active low.
	PORT_START("trigger_inputs")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_OTHER) PORT_NAME("TRIGGER 5")
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_OTHER) PORT_NAME("TRIGGER 4")
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_OTHER) PORT_NAME("TRIGGER 3")
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_OTHER) PORT_NAME("TRIGGER 2")
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_OTHER) PORT_NAME("TRIGGER 1")

	PORT_START("pot_tempo")
	PORT_ADJUSTER(50, "TEMPO")
		PORT_CHANGED_MEMBER(DEVICE_SELF, FUNC(linndrum_state::tempo_pot_adjusted), 0)

	PORT_START("pot_volume")
	PORT_ADJUSTER(100, "MASTER VOLUME")

	PORT_START("pot_tuning_1")
	PORT_ADJUSTER(50, "SNARE TUNING");
	PORT_START("pot_tuning_2")
	PORT_ADJUSTER(50, "HI TOM TUNING");
	PORT_START("pot_tuning_3")
	PORT_ADJUSTER(50, "MID TOM TUNING");
	PORT_START("pot_tuning_4")
	PORT_ADJUSTER(50, "LO TOM TUNING");
	PORT_START("pot_tuning_5")
	PORT_ADJUSTER(50, "HI CONGAS TUNING");
	PORT_START("pot_tuning_6")
	PORT_ADJUSTER(50, "LO CONGAS TUNING");
	PORT_START("pot_tuning_7")
	PORT_ADJUSTER(50, "HIHAT DECAY");

	PORT_START("pot_pan_1")
	PORT_ADJUSTER(50, "BASS PAN")
	PORT_START("pot_pan_2")
	PORT_ADJUSTER(50, "SNARE PAN")
	PORT_START("pot_pan_3")
	PORT_ADJUSTER(50, "SIDESTICK PAN")
	PORT_START("pot_pan_4")
	PORT_ADJUSTER(50, "HIHAT PAN")
	PORT_START("pot_pan_5")
	PORT_ADJUSTER(50, "HI TOM PAN")
	PORT_START("pot_pan_6")
	PORT_ADJUSTER(50, "MID TOM PAN")
	PORT_START("pot_pan_7")
	PORT_ADJUSTER(50, "LO TOM PAN")
	PORT_START("pot_pan_8")
	PORT_ADJUSTER(50, "RIDE PAN")
	PORT_START("pot_pan_9")
	PORT_ADJUSTER(50, "CRASH PAN")
	PORT_START("pot_pan_10")
	PORT_ADJUSTER(50, "CABASA PAN")
	PORT_START("pot_pan_11")
	PORT_ADJUSTER(50, "TAMB PAN")
	PORT_START("pot_pan_12")
	PORT_ADJUSTER(50, "HI CONGA PAN")
	PORT_START("pot_pan_13")
	PORT_ADJUSTER(50, "LO CONGA PAN")
	PORT_START("pot_pan_14")
	PORT_ADJUSTER(50, "COWBELL PAN")
	PORT_START("pot_pan_15")
	PORT_ADJUSTER(50, "CLAPS PAN")
	PORT_START("pot_pan_16")
	PORT_ADJUSTER(50, "CLICK PAN")

	PORT_START("pot_gain_1")
	PORT_ADJUSTER(100, "BASS GAIN")
	PORT_START("pot_gain_2")
	PORT_ADJUSTER(100, "SNARE GAIN")
	PORT_START("pot_gain_3")
	PORT_ADJUSTER(100, "SIDESTICK GAIN")
	PORT_START("pot_gain_4")
	PORT_ADJUSTER(100, "HIHAT GAIN")
	PORT_START("pot_gain_5")
	PORT_ADJUSTER(100, "HI TOM GAIN")
	PORT_START("pot_gain_6")
	PORT_ADJUSTER(100, "MID TOM GAIN")
	PORT_START("pot_gain_7")
	PORT_ADJUSTER(100, "LO TOM GAIN")
	PORT_START("pot_gain_8")
	PORT_ADJUSTER(100, "RIDE GAIN")
	PORT_START("pot_gain_9")
	PORT_ADJUSTER(100, "CRASH GAIN")
	PORT_START("pot_gain_10")
	PORT_ADJUSTER(100, "CABASA GAIN")
	PORT_START("pot_gain_11")
	PORT_ADJUSTER(100, "TAMB GAIN")
	PORT_START("pot_gain_12")
	PORT_ADJUSTER(100, "HI CONGA GAIN")
	PORT_START("pot_gain_13")
	PORT_ADJUSTER(100, "LO CONGA GAIN")
	PORT_START("pot_gain_14")
	PORT_ADJUSTER(100, "COWBELL GAIN")
	PORT_START("pot_gain_15")
	PORT_ADJUSTER(100, "CLAPS GAIN")
	PORT_START("pot_gain_16")
	PORT_ADJUSTER(100, "CLICK GAIN")
INPUT_PORTS_END

ROM_START(linndrum)
	ROM_REGION(0x2000, MAINCPU_TAG, 0)
	ROM_LOAD("ld_2.1_a.u14", 0x000000, 0x001000, CRC(566d720e) SHA1(91b7a515e3d18a28b7f5428765ed79114a5a00fb))
	ROM_LOAD("ld_2.1_b.u13", 0x001000, 0x001000, CRC(9c9a5520) SHA1(6e4573051254051c75f9071fd8dfcd5a9184f9cc))
ROM_END

}  // anonymous namespace

SYST(1982, linndrum, 0, 0, linndrum, linndrum, linndrum_state, empty_init, "Linn Electronics", "LinnDrum", MACHINE_SUPPORTS_SAVE | MACHINE_NOT_WORKING | MACHINE_NO_SOUND)
