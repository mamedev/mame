// license:BSD-3-Clause
// copyright-holders:hap
// thanks-to:Sean Riddle, Kevin Horton
/***************************************************************************

Rockwell PPS-4/1 MCU series handhelds

ROM source notes when dumped from another publisher, but confident it's the same:
- memoquiz: Mattel Mind Boggler

***************************************************************************/

#include "emu.h"

#include "cpu/pps41/mm75.h"
#include "cpu/pps41/mm76.h"
#include "cpu/pps41/mm78.h"
#include "cpu/pps41/mm78la.h"
#include "video/pwm.h"
#include "sound/beep.h"
#include "sound/spkrdev.h"

#include "screen.h"
#include "speaker.h"

// internal artwork
#include "brainbaf.lh"
#include "dunksunk.lh"
#include "ftri1.lh"
#include "horocomp.lh"
#include "mastmind.lh"
#include "memoquiz.lh"
#include "mfootb2.lh"
#include "mwcfootb.lh"
#include "rdqa.lh"
#include "scrabsen.lh"
#include "smastmind.lh"

//#include "hh_pps41_test.lh" // common test-layout - use external artwork


class hh_pps41_state : public driver_device
{
public:
	hh_pps41_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_display(*this, "display"),
		m_speaker(*this, "speaker"),
		m_inputs(*this, "IN.%u", 0)
	{ }

	virtual DECLARE_INPUT_CHANGED_MEMBER(reset_button);

protected:
	virtual void machine_start() override;
	virtual void machine_reset() override { update_int(); }
	virtual void device_post_load() override { update_int(); }

	// devices
	required_device<pps41_base_device> m_maincpu;
	optional_device<pwm_display_device> m_display;
	optional_device<speaker_sound_device> m_speaker;
	optional_ioport_array<11> m_inputs; // max 11

	u16 m_inp_mux = 0;
	u32 m_grid = 0;
	u32 m_plate = 0;

	// MCU output pin state
	u16 m_d = 0;
	u16 m_r = 0;

	u8 read_inputs(int columns);
	virtual void update_int() { ; }
};


// machine start

void hh_pps41_state::machine_start()
{
	// register for savestates
	save_item(NAME(m_inp_mux));
	save_item(NAME(m_grid));
	save_item(NAME(m_plate));
	save_item(NAME(m_d));
	save_item(NAME(m_r));
}



/***************************************************************************

  Helper Functions

***************************************************************************/

// generic input handlers

u8 hh_pps41_state::read_inputs(int columns)
{
	u8 ret = 0;

	// read selected input rows
	for (int i = 0; i < columns; i++)
		if (m_inp_mux >> i & 1)
			ret |= m_inputs[i]->read();

	return ret;
}

INPUT_CHANGED_MEMBER(hh_pps41_state::reset_button)
{
	// when an input is directly wired to MCU PO pin
	m_maincpu->set_input_line(INPUT_LINE_RESET, newval ? ASSERT_LINE : CLEAR_LINE);
}



/***************************************************************************

  Minidrivers (subclass, I/O, Inputs, Machine Config, ROM Defs)

***************************************************************************/

namespace {

/***************************************************************************

  Fonas Tri-1
  * PCB label: CASSIA CA010-F
  * MM78 MCU variant with 40 pins (no label, die label A7859)
  * 4 7seg leds, 41 other leds, 1-bit sound

  The game only uses 1.5KB ROM and seems it doesn't use all the RAM either,
  as if it was programmed for MM77L.

  Hold all 4 buttons at boot (not counting RESET) for a led test.
  Cassia was Eric White/Ken Cohen's company, later named CXG, known for
  their chess computers.

***************************************************************************/

class ftri1_state : public hh_pps41_state
{
public:
	ftri1_state(const machine_config &mconfig, device_type type, const char *tag) :
		hh_pps41_state(mconfig, type, tag)
	{ }

	void ftri1(machine_config &config);

private:
	void update_display();
	void write_d(u16 data);
	void write_r(u16 data);
};

// handlers

void ftri1_state::update_display()
{
	m_display->matrix(m_d, bitswap<8>(~m_r, 0,7,6,5,4,3,2,1));
}

void ftri1_state::write_d(u16 data)
{
	// DIO0-DIO8: digit/led select
	m_d = data;
	update_display();

	// DIO9: speaker out
	m_speaker->level_w(BIT(data, 9));
}

void ftri1_state::write_r(u16 data)
{
	// RIO1-RIO8: digit/led data
	m_r = data;
	update_display();
}

// config

static INPUT_PORTS_START( ftri1 )
	PORT_START("IN.0")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_CONFNAME( 0x0c, 0x04, "Game Select" )
	PORT_CONFSETTING(    0x08, "Star Chase" )
	PORT_CONFSETTING(    0x04, "All Star Baseball" )
	PORT_CONFSETTING(    0x00, "Batting Champs" )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON4 ) PORT_NAME("Score / S1 H")
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_NAME("Steal / S1 V")
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_NAME("Pitch / S2 H")
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_NAME("Swing / S2 V")

	PORT_START("RESET")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CHANGED_MEMBER(DEVICE_SELF, hh_pps41_state, reset_button, 0) PORT_CODE(KEYCODE_F1) PORT_NAME("Game Reset")
INPUT_PORTS_END

void ftri1_state::ftri1(machine_config &config)
{
	// basic machine hardware
	MM78(config, m_maincpu, 300000); // approximation - VC osc. R=68K
	m_maincpu->write_d().set(FUNC(ftri1_state::write_d));
	m_maincpu->write_r().set(FUNC(ftri1_state::write_r));
	m_maincpu->read_p().set_ioport("IN.0");

	// video hardware
	PWM_DISPLAY(config, m_display).set_size(9, 8);
	m_display->set_segmask(0x1e0, 0x7f);
	config.set_default_layout(layout_ftri1);

	// sound hardware
	SPEAKER(config, "mono").front_center();
	SPEAKER_SOUND(config, m_speaker);
	m_speaker->add_route(ALL_OUTPUTS, "mono", 0.25);
}

// roms

ROM_START( ftri1 )
	ROM_REGION( 0x0800, "maincpu", 0 )
	ROM_LOAD( "a7859", 0x0000, 0x0800, CRC(3c957f1d) SHA1(42db81a78bbef971a84e61a26d91f7411980d79c) )
ROM_END





/***************************************************************************

  Invicta Electronic Master Mind
  * MM75 MCU (label MM75 A7525-11, die label A7525)
  * 9-digit 7seg VFD display (Futaba 9-ST)

  Invicta Super-Sonic Electronic Master Mind
  * MM75 MCU (label A7539-12, die label A7539)
  * same base hardware, added beeper

  Invicta Plastics is the owner of the Mastermind game rights. The back of the
  Master Mind unit says (C) 1977, but this electronic handheld version came
  out in 1979. Or maybe there's an older revision.

***************************************************************************/

class mastmind_state : public hh_pps41_state
{
public:
	mastmind_state(const machine_config &mconfig, device_type type, const char *tag) :
		hh_pps41_state(mconfig, type, tag),
		m_beeper(*this, "beeper")
	{ }

	void mastmind(machine_config &config);
	void smastmind(machine_config &config);

private:
	optional_device<beep_device> m_beeper;

	void update_display();
	void write_d(u16 data);
	void write_r(u16 data);
	u8 read_p();
};

// handlers

void mastmind_state::update_display()
{
	m_display->matrix(m_inp_mux, ~m_r);
}

void mastmind_state::write_d(u16 data)
{
	// DIO0-DIO7: digit select (DIO7 N/C on mastmind)
	// DIO0-DIO3: input mux
	m_inp_mux = data;
	update_display();

	// DIO8: beeper on smastmind
	if (m_beeper)
		m_beeper->set_state(BIT(data, 8));
}

void mastmind_state::write_r(u16 data)
{
	// RIO1-RIO7: digit segment data
	m_r = data;
	update_display();
}

u8 mastmind_state::read_p()
{
	// PI1-PI4: multiplexed inputs
	return ~read_inputs(4);
}

// config

static INPUT_PORTS_START( mastmind )
	PORT_START("IN.0") // DIO0
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_ENTER) PORT_CODE(KEYCODE_ENTER_PAD) PORT_NAME("Try")
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_F) PORT_NAME("Fail")
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_UNUSED ) // display test on mastmind?
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("IN.1") // DIO1
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_8) PORT_CODE(KEYCODE_8_PAD) PORT_NAME("8")
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_9) PORT_CODE(KEYCODE_9_PAD) PORT_NAME("9")
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_S) PORT_NAME("Set")
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_DEL) PORT_CODE(KEYCODE_BACKSPACE) PORT_NAME("Clear")

	PORT_START("IN.2") // DIO2
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_4) PORT_CODE(KEYCODE_4_PAD) PORT_NAME("4")
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_5) PORT_CODE(KEYCODE_5_PAD) PORT_NAME("5")
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_6) PORT_CODE(KEYCODE_6_PAD) PORT_NAME("6")
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_7) PORT_CODE(KEYCODE_7_PAD) PORT_NAME("7")

	PORT_START("IN.3") // DIO3
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_0) PORT_CODE(KEYCODE_0_PAD) PORT_NAME("0")
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_1) PORT_CODE(KEYCODE_1_PAD) PORT_NAME("1")
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_2) PORT_CODE(KEYCODE_2_PAD) PORT_NAME("2")
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_3) PORT_CODE(KEYCODE_3_PAD) PORT_NAME("3")
INPUT_PORTS_END

void mastmind_state::mastmind(machine_config &config)
{
	// basic machine hardware
	MM75(config, m_maincpu, 360000); // approximation - VC osc. R=56K
	m_maincpu->write_d().set(FUNC(mastmind_state::write_d));
	m_maincpu->write_r().set(FUNC(mastmind_state::write_r));
	m_maincpu->read_p().set(FUNC(mastmind_state::read_p));

	// video hardware
	PWM_DISPLAY(config, m_display).set_size(8, 7);
	m_display->set_segmask(0xff, 0x7f);
	config.set_default_layout(layout_mastmind);

	// no sound!
}

void mastmind_state::smastmind(machine_config &config)
{
	mastmind(config);

	config.set_default_layout(layout_smastmind);

	// sound hardware
	SPEAKER(config, "mono").front_center();
	BEEP(config, m_beeper, 2400); // approximation
	m_beeper->add_route(ALL_OUTPUTS, "mono", 0.25);
}

// roms

ROM_START( mastmind )
	ROM_REGION( 0x0400, "maincpu", ROMREGION_ERASE00 )
	ROM_LOAD( "mm75_a7525-11", 0x0000, 0x0200, CRC(39dbdd50) SHA1(72fa5781e9df62d91d57437ded2931fab8253c3c) )
	ROM_CONTINUE(              0x0380, 0x0080 )

	ROM_REGION( 314, "maincpu:opla", 0 )
	ROM_LOAD( "mm76_mastmind_output.pla", 0, 314, CRC(c936aee7) SHA1(e9ec08a82493d6b63e936f82deeab3e4449b54c3) )
ROM_END

ROM_START( smastmind )
	ROM_REGION( 0x0400, "maincpu", ROMREGION_ERASE00 )
	ROM_LOAD( "a7539-12", 0x0000, 0x0200, CRC(b63c453f) SHA1(f47a540fd90eed7514ed03864be2121f641c1154) )
	ROM_CONTINUE(         0x0380, 0x0080 )

	ROM_REGION( 314, "maincpu:opla", 0 )
	ROM_LOAD( "mm76_smastmind_output.pla", 0, 314, CRC(c936aee7) SHA1(e9ec08a82493d6b63e936f82deeab3e4449b54c3) )
ROM_END





/***************************************************************************

  Kmart Dunk 'n Sunk (manufactured in Hong Kong)
  * MM76EL MCU (label GE-E 1V2280, die label B8617)
  * 4 7seg leds, 31 other leds, 1-bit sound

  It's by the same Hong Kong company that did Kmart Dr. Dunk/Tandy Electronic
  Basketball (PIC16 MCU), Grandstand Pocket Match of the Day (MOS MPS765x MCU)
  among others.

  known releases:
  - USA(1): Dunk 'n Sunk, published by Kmart
  - USA(2): Electronic Basketball / Submarine Warfare, published by U.S. Games

***************************************************************************/

class dunksunk_state : public hh_pps41_state
{
public:
	dunksunk_state(const machine_config &mconfig, device_type type, const char *tag) :
		hh_pps41_state(mconfig, type, tag)
	{ }

	void dunksunk(machine_config &config);

	DECLARE_INPUT_CHANGED_MEMBER(difficulty_switch) { update_int(); }
	DECLARE_INPUT_CHANGED_MEMBER(game_switch) { update_int(); }

private:
	virtual void update_int() override;
	void update_display();
	void write_d(u16 data);
	void write_r(u16 data);
};

// handlers

void dunksunk_state::update_int()
{
	// 2 of the switches are tied to MCU interrupt pins
	m_maincpu->set_input_line(0, (m_inputs[1]->read() & 1) ? ASSERT_LINE : CLEAR_LINE);
	m_maincpu->set_input_line(1, (m_inputs[2]->read() & 1) ? ASSERT_LINE : CLEAR_LINE);
}

void dunksunk_state::update_display()
{
	m_display->matrix(m_d >> 1, ~m_r);
}

void dunksunk_state::write_d(u16 data)
{
	// DIO0: speaker out
	m_speaker->level_w(data & 1);

	// DIO1-DIO5: led select
	// DIO6-DIO9: digit select
	m_d = data;
	update_display();
}

void dunksunk_state::write_r(u16 data)
{
	// RIO1-RIO7: led data
	m_r = data;
	update_display();
}

// config

static INPUT_PORTS_START( dunksunk )
	PORT_START("IN.0") // PI
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_BUTTON1 )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_CUSTOM ) PORT_CONDITION("IN.1", 0x03, EQUALS, 0x01)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_JOYSTICK_UP )

	PORT_START("IN.1")
	PORT_CONFNAME( 0x03, 0x00, DEF_STR( Difficulty ) ) PORT_CHANGED_MEMBER(DEVICE_SELF, dunksunk_state, difficulty_switch, 0)
	PORT_CONFSETTING(    0x00, "1" ) // INT0
	PORT_CONFSETTING(    0x01, "2" ) // PI4
	PORT_CONFSETTING(    0x03, "3" )

	PORT_START("IN.2") // INT1
	PORT_CONFNAME( 0x01, 0x01, "Game Select" ) PORT_CHANGED_MEMBER(DEVICE_SELF, dunksunk_state, game_switch, 0)
	PORT_CONFSETTING(    0x01, "Basketball" )
	PORT_CONFSETTING(    0x00, "Submarine Chase" )
INPUT_PORTS_END

void dunksunk_state::dunksunk(machine_config &config)
{
	// basic machine hardware
	MM76EL(config, m_maincpu, 390000); // approximation - VC osc. R=56K
	m_maincpu->write_d().set(FUNC(dunksunk_state::write_d));
	m_maincpu->write_r().set(FUNC(dunksunk_state::write_r));
	m_maincpu->read_p().set_ioport("IN.0");

	// video hardware
	PWM_DISPLAY(config, m_display).set_size(9, 7);
	m_display->set_segmask(0x1e0, 0x7f);
	m_display->set_bri_levels(0.015, 0.2); // player led is brighter
	config.set_default_layout(layout_dunksunk);

	// sound hardware
	SPEAKER(config, "mono").front_center();
	SPEAKER_SOUND(config, m_speaker);
	m_speaker->add_route(ALL_OUTPUTS, "mono", 0.25);
}

// roms

ROM_START( dunksunk )
	ROM_REGION( 0x0400, "maincpu", 0 )
	ROM_LOAD( "ge-e_1v2280", 0x0000, 0x0400, CRC(90f17191) SHA1(80c3708af99c9db7afe17254fa4df2080aa9f145) )

	ROM_REGION( 314, "maincpu:opla", 0 )
	ROM_LOAD( "mm76_dunksunk_output.pla", 0, 314, CRC(410fa6d7) SHA1(d46aaf1ec2c942083cba7dbd59d4261dc238d4c8) )
ROM_END





/***************************************************************************

  M.E.M. Belgium Memoquiz
  * PCB label: MEMOQUIZ MO3
  * MM75 MCU (label M7505 A7505-12, die label A7505)
  * 9-digit 7seg VFD display, no sound

  It's a Mastermind game, not as straightforward as Invicta's version.
  To start, press the "?" button to generate a new code, then try to guess it,
  confirming with the "=" button. CD reveals the answer, PE is for player entry.

  known releases:
  - Europe: Memoquiz, published by M.E.M. Belgium
  - UK: Memoquiz, published by Polymark
  - USA: Mind Boggler (model 2626), published by Mattel

***************************************************************************/

class memoquiz_state : public hh_pps41_state
{
public:
	memoquiz_state(const machine_config &mconfig, device_type type, const char *tag) :
		hh_pps41_state(mconfig, type, tag)
	{ }

	void memoquiz(machine_config &config);

	DECLARE_INPUT_CHANGED_MEMBER(digits_switch) { update_int(); }

private:
	virtual void update_int() override;
	void update_display();
	void write_d(u16 data);
	void write_r(u16 data);
	u8 read_p();
};

// handlers

void memoquiz_state::update_int()
{
	// digits switch is tied to MCU interrupt pins
	u8 inp = m_inputs[4]->read();
	m_maincpu->set_input_line(0, (inp & 1) ? CLEAR_LINE : ASSERT_LINE);
	m_maincpu->set_input_line(1, (inp & 2) ? ASSERT_LINE : CLEAR_LINE);
}

void memoquiz_state::update_display()
{
	m_display->matrix(m_inp_mux, (m_inp_mux << 2 & 0x80) | (~m_r & 0x7f));
}

void memoquiz_state::write_d(u16 data)
{
	// DIO0-DIO7: digit select, DIO5 is also DP segment
	// DIO0-DIO3: input mux
	m_inp_mux = data;
	update_display();

	// DIO8: N/C, looks like they planned to add sound, but didn't
}

void memoquiz_state::write_r(u16 data)
{
	// RIO1-RIO7: digit segment data
	m_r = data;
	update_display();
}

u8 memoquiz_state::read_p()
{
	// PI1-PI4: multiplexed inputs
	return ~read_inputs(4);
}

// config

static INPUT_PORTS_START( memoquiz )
	PORT_START("IN.0") // DIO0
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_3) PORT_CODE(KEYCODE_3_PAD) PORT_NAME("3")
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_2) PORT_CODE(KEYCODE_2_PAD) PORT_NAME("2")
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_1) PORT_CODE(KEYCODE_1_PAD) PORT_NAME("1")
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_0) PORT_CODE(KEYCODE_0_PAD) PORT_NAME("0")

	PORT_START("IN.1") // DIO1
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_7) PORT_CODE(KEYCODE_7_PAD) PORT_NAME("7")
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_6) PORT_CODE(KEYCODE_6_PAD) PORT_NAME("6")
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_5) PORT_CODE(KEYCODE_5_PAD) PORT_NAME("5")
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_4) PORT_CODE(KEYCODE_4_PAD) PORT_NAME("4")

	PORT_START("IN.2") // DIO2
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_A) PORT_NAME("AC")
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_DEL) PORT_CODE(KEYCODE_BACKSPACE) PORT_NAME("CE")
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_9) PORT_CODE(KEYCODE_9_PAD) PORT_NAME("9")
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_8) PORT_CODE(KEYCODE_8_PAD) PORT_NAME("8")

	PORT_START("IN.3") // DIO3
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_ENTER) PORT_CODE(KEYCODE_ENTER_PAD) PORT_NAME("=")
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_SLASH) PORT_NAME("?")
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_E) PORT_NAME("PE")
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_C) PORT_NAME("CD")

	PORT_START("IN.4")
	PORT_CONFNAME( 0x03, 0x01, "Digits" ) PORT_CHANGED_MEMBER(DEVICE_SELF, memoquiz_state, digits_switch, 0)
	PORT_CONFSETTING(    0x01, "3" ) // INT0, Vdd when closed, pulled to GND when open
	PORT_CONFSETTING(    0x02, "4" ) // INT1, GND when closed, pulled to Vdd when open
	PORT_CONFSETTING(    0x00, "5" )
INPUT_PORTS_END

void memoquiz_state::memoquiz(machine_config &config)
{
	// basic machine hardware
	MM75(config, m_maincpu, 360000); // approximation - VC osc. R=56K
	m_maincpu->write_d().set(FUNC(memoquiz_state::write_d));
	m_maincpu->write_r().set(FUNC(memoquiz_state::write_r));
	m_maincpu->read_p().set(FUNC(memoquiz_state::read_p));

	// video hardware
	PWM_DISPLAY(config, m_display).set_size(8, 8);
	m_display->set_segmask(0xff, 0xff);
	config.set_default_layout(layout_memoquiz);

	// no sound!
}

// roms

ROM_START( memoquiz )
	ROM_REGION( 0x0400, "maincpu", ROMREGION_ERASE00 )
	ROM_LOAD( "m7505_a7505-12", 0x0000, 0x0200, CRC(47223508) SHA1(97b62e0c453ae2e65d48e039ad65857dae2d4d76) )
	ROM_CONTINUE(               0x0380, 0x0080 )

	ROM_REGION( 314, "maincpu:opla", 0 )
	ROM_LOAD( "mm76_memoquiz_output.pla", 0, 314, CRC(a5799b50) SHA1(9b4923b37c9ba8221ecece5a3370c605a880a453) )
ROM_END





/***************************************************************************

  Mattel Football 2 (model 1050)
  * PCB label: MATTEL, 1050-4369D
  * MM77LA MCU (label B8000-12, die label B8000)
  * 7 7seg leds, 30 other leds, 2-bit sound

  Through its production run, it was released as "Football 2" and "Football II".

***************************************************************************/

class mfootb2_state : public hh_pps41_state
{
public:
	mfootb2_state(const machine_config &mconfig, device_type type, const char *tag) :
		hh_pps41_state(mconfig, type, tag)
	{ }

	void mfootb2(machine_config &config);

private:
	void update_display();
	void write_d(u16 data);
	void write_r(u16 data);
	void write_spk(u8 data);
};

// handlers

void mfootb2_state::update_display()
{
	m_display->matrix(m_d, (m_r & 0x7f) | (m_d >> 4 & 0x80) | (m_r << 1 & 0x700));
}

void mfootb2_state::write_d(u16 data)
{
	// DIO0-DIO2, DIO6-DIO9: digit select
	// DIO3-DIO5: led select
	// DIO10: 4th digit DP
	m_d = data;
	update_display();
}

void mfootb2_state::write_r(u16 data)
{
	// RO01-RO10: led data
	m_r = data;
	update_display();
}

void mfootb2_state::write_spk(u8 data)
{
	// SPK: speaker out
	m_speaker->level_w(data);
}

// config

static INPUT_PORTS_START( mfootb2 )
	PORT_START("IN.0") // PI
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_START2 ) PORT_NAME("Score")
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_START1 ) PORT_NAME("Status")
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_JOYSTICK_UP ) PORT_16WAY
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT ) PORT_16WAY
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_BUTTON2 ) PORT_NAME("Kick")
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_NAME("Pass")
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_JOYSTICK_DOWN ) PORT_16WAY
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT ) PORT_16WAY

	PORT_START("IN.1") // DIO11
	PORT_CONFNAME( 0x400, 0x000, DEF_STR( Difficulty ) )
	PORT_CONFSETTING(     0x000, "1" )
	PORT_CONFSETTING(     0x400, "2" )
INPUT_PORTS_END

void mfootb2_state::mfootb2(machine_config &config)
{
	// basic machine hardware
	MM77LA(config, m_maincpu, 380000); // approximation - VC osc. R=56K
	m_maincpu->write_d().set(FUNC(mfootb2_state::write_d));
	m_maincpu->read_d().set_ioport("IN.1");
	m_maincpu->write_r().set(FUNC(mfootb2_state::write_r));
	m_maincpu->read_p().set_ioport("IN.0");
	m_maincpu->write_spk().set(FUNC(mfootb2_state::write_spk));

	// video hardware
	PWM_DISPLAY(config, m_display).set_size(10, 11);
	m_display->set_segmask(0x3c7, 0x7f);
	m_display->set_segmask(0x002, 0xff); // only one digit has DP
	m_display->set_bri_levels(0.015, 0.2); // ball led is brighter
	config.set_default_layout(layout_mfootb2);

	// sound hardware
	SPEAKER(config, "mono").front_center();
	SPEAKER_SOUND(config, m_speaker);
	static const double speaker_levels[4] = { 0.0, 1.0, -1.0, 0.0 };
	m_speaker->set_levels(4, speaker_levels);
	m_speaker->add_route(ALL_OUTPUTS, "mono", 0.25);
}

// roms

ROM_START( mfootb2 )
	ROM_REGION( 0x0800, "maincpu", ROMREGION_ERASE00 )
	ROM_LOAD( "b8000-12", 0x0000, 0x0600, CRC(5b65fc38) SHA1(4fafc9deb5609b16f09b18b7346ea96ffe8bf9e0) )

	ROM_REGION( 317, "maincpu:opla", 0 )
	ROM_LOAD( "mm77la_mfootb2_output.pla", 0, 317, CRC(11c0bbfa) SHA1(939a0a6adeace8ca0f9e17290306a2e7ced21db3) )
ROM_END





/***************************************************************************

  Mattel Brain Baffler (model 1080)
  * PCB label: OLYMPOS KOREA, CM04-D102-001 REV D
  * MM78LA MCU (label MM95 B9000-12, die label B9000)
  * 8-digit 14seg LED display, 2-bit sound

  It includes 8 word games, most of them are meant for 2 players.

***************************************************************************/

class brainbaf_state : public hh_pps41_state
{
public:
	brainbaf_state(const machine_config &mconfig, device_type type, const char *tag) :
		hh_pps41_state(mconfig, type, tag)
	{ }

	void brainbaf(machine_config &config);

private:
	void update_display();
	void write_d(u16 data);
	void write_r(u16 data);
	u8 read_p();
	void write_spk(u8 data);
};

// handlers

void brainbaf_state::update_display()
{
	m_display->matrix(m_inp_mux, bitswap<14>(m_r, 6,5,13,12,11,4,3,10,9,8,7,2,1,0));
}

void brainbaf_state::write_d(u16 data)
{
	// DIO0-DIO9: input mux
	// DIO0-DIO7: digit select
	m_inp_mux = data;
	update_display();
}

void brainbaf_state::write_r(u16 data)
{
	// RO01-RO14: digit segment data
	m_r = data;
	update_display();
}

u8 brainbaf_state::read_p()
{
	// PI5-PI8: multiplexed inputs
	return read_inputs(10) << 4;
}

void brainbaf_state::write_spk(u8 data)
{
	// SPK: speaker out
	m_speaker->level_w(data);
}

// config

static INPUT_PORTS_START( brainbaf )
	PORT_START("IN.0") // DIO0
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_J) PORT_CODE(KEYCODE_0) PORT_CODE(KEYCODE_0_PAD) PORT_CHAR('J') PORT_NAME("J-0")
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_T) PORT_CHAR('T')
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_F12) PORT_NAME("Player / Bonus R")

	PORT_START("IN.1") // DIO1
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_A) PORT_CODE(KEYCODE_1) PORT_CODE(KEYCODE_1_PAD) PORT_CHAR('A') PORT_NAME("A-1")
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_K) PORT_CHAR('K')
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_F1) PORT_NAME("Player / Bonus L")

	PORT_START("IN.2") // DIO2
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_B) PORT_CODE(KEYCODE_2) PORT_CODE(KEYCODE_2_PAD) PORT_CHAR('B') PORT_NAME("B-2")
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_L) PORT_CHAR('L')
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_F3) PORT_NAME("Game")

	PORT_START("IN.3") // DIO3
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_C) PORT_CODE(KEYCODE_3) PORT_CODE(KEYCODE_3_PAD) PORT_CHAR('C') PORT_NAME("C-3")
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_M) PORT_CHAR('M')
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_U) PORT_CHAR('U')
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_F4) PORT_NAME("Go")

	PORT_START("IN.4") // DIO4
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_D) PORT_CODE(KEYCODE_4) PORT_CODE(KEYCODE_4_PAD) PORT_CHAR('D') PORT_NAME("D-4")
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_N) PORT_CHAR('N')
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_V) PORT_CHAR('V')
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_F5) PORT_CODE(KEYCODE_ENTER) PORT_CODE(KEYCODE_ENTER_PAD) PORT_CHAR(13) PORT_NAME("Entry")

	PORT_START("IN.5") // DIO5
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_E) PORT_CODE(KEYCODE_5) PORT_CODE(KEYCODE_5_PAD) PORT_CHAR('E') PORT_NAME("E-5")
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_O) PORT_CHAR('O')
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_W) PORT_CHAR('W')
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_F6) PORT_CODE(KEYCODE_DEL) PORT_CODE(KEYCODE_BACKSPACE) PORT_CHAR(8) PORT_NAME("Clear")

	PORT_START("IN.6") // DIO6
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_F) PORT_CODE(KEYCODE_6) PORT_CODE(KEYCODE_6_PAD) PORT_CHAR('F') PORT_NAME("F-6")
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_P) PORT_CHAR('P')
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_X) PORT_CHAR('X')
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_F7) PORT_NAME("Refresh")

	PORT_START("IN.7") // DIO7
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_G) PORT_CODE(KEYCODE_7) PORT_CODE(KEYCODE_7_PAD) PORT_CHAR('G') PORT_NAME("G-7")
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_Q) PORT_CHAR('Q')
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_Y) PORT_CHAR('Y')
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_F8) PORT_NAME("Buy")

	PORT_START("IN.8") // DIO8
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_H) PORT_CODE(KEYCODE_8) PORT_CODE(KEYCODE_8_PAD) PORT_CHAR('H') PORT_NAME("H-8")
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_R) PORT_CHAR('R')
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_Z) PORT_CHAR('Z')
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_F9) PORT_NAME("Give-Up")

	PORT_START("IN.9") // DIO9
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_I) PORT_CODE(KEYCODE_9) PORT_CODE(KEYCODE_9_PAD) PORT_CHAR('I') PORT_NAME("I-9")
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_S) PORT_CHAR('S')
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_F2) PORT_CODE(KEYCODE_F11) PORT_NAME("Stop") // both STOP keys
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_F10) PORT_NAME("Score")
INPUT_PORTS_END

void brainbaf_state::brainbaf(machine_config &config)
{
	// basic machine hardware
	MM78LA(config, m_maincpu, 440000); // approximation - VC osc. R=10K
	m_maincpu->write_d().set(FUNC(brainbaf_state::write_d));
	m_maincpu->write_r().set(FUNC(brainbaf_state::write_r));
	m_maincpu->read_p().set(FUNC(brainbaf_state::read_p));
	m_maincpu->write_spk().set(FUNC(brainbaf_state::write_spk));

	// video hardware
	PWM_DISPLAY(config, m_display).set_size(8, 14);
	m_display->set_segmask(0xff, 0x3fff);
	config.set_default_layout(layout_brainbaf);

	// sound hardware
	SPEAKER(config, "mono").front_center();
	SPEAKER_SOUND(config, m_speaker);
	static const double speaker_levels[4] = { 0.0, 1.0, -1.0, 0.0 };
	m_speaker->set_levels(4, speaker_levels);
	m_speaker->add_route(ALL_OUTPUTS, "mono", 0.25);
}

// roms

ROM_START( brainbaf )
	ROM_REGION( 0x0800, "maincpu", 0 )
	ROM_LOAD( "mm95_b9000-12", 0x0000, 0x0800, CRC(f7a4829c) SHA1(12f789d8264a969777764d31ea67067cc330a73c) )

	ROM_REGION( 605, "maincpu:opla", 0 )
	ROM_LOAD( "mm78la_brainbaf_output.pla", 0, 605, CRC(4fae532f) SHA1(3a08c0fa3ce476c014280b3cfeb6aa37824ae503) )
ROM_END





/***************************************************************************

  Mattel Horoscope Computer (model 1081)
  * PCB label: DET. CM05-D102-001 REV D
  * MM78LA MCU (label MM95 B9001-13, die label B9001)
  * 8-digit 14seg LED display, 2-bit sound

  This is not a toy, but a fortune forecast. Date format is mm-dd-yy, it is
  valid only from June 1 1979 until December 31 1987

***************************************************************************/

class horocomp_state : public hh_pps41_state
{
public:
	horocomp_state(const machine_config &mconfig, device_type type, const char *tag) :
		hh_pps41_state(mconfig, type, tag)
	{ }

	void horocomp(machine_config &config);

private:
	void update_display();
	void write_d(u16 data);
	void write_r(u16 data);
	u8 read_p();
	void write_spk(u8 data);
};

// handlers

void horocomp_state::update_display()
{
	// 14seg display is upside-down
	u16 flip = m_r << 7 | m_r >> 7;
	m_display->matrix(m_inp_mux, bitswap<14>(flip, 6,5,13,12,11,4,3,10,9,8,7,2,1,0));
}

void horocomp_state::write_d(u16 data)
{
	// DIO0-DIO9: input mux
	// DIO0-DIO7: digit select
	m_inp_mux = data;
	update_display();
}

void horocomp_state::write_r(u16 data)
{
	// RO01-RO14: digit segment data
	m_r = data;
	update_display();
}

u8 horocomp_state::read_p()
{
	// PI4: mode switch
	// PI5-PI8: multiplexed inputs
	return read_inputs(10) << 4 | (m_inputs[10]->read() & 8);
}

void horocomp_state::write_spk(u8 data)
{
	// SPK: speaker out
	m_speaker->level_w(data);
}

// config

/* physical button layout and labels are like this:

              CAP.
       AQU.   [ ]   SAG.
         [ ]   9   [ ]
  PIS.             8     SCO.
     [ ]              7[ ]

ARI.[ ]0               6[ ]LIB.

     [ ]1             5[ ]          [   ]   [   ]   [   ]   [   ]
  TAU.     2       4     VIR.       LOVE    CAREER  TRAVEL  MONEY
         [ ]   3   [ ]
       GEM.   [ ]   LEO             [   ]   [   ]   [   ]   [   ]
              CAN.                  FRIENDS SPIRIT  FAMILY  CREATIVE
*/

static INPUT_PORTS_START( horocomp )
	PORT_START("IN.0") // DIO0
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_L) PORT_NAME("Love")
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_F) PORT_NAME("Friends")
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_0) PORT_CODE(KEYCODE_0_PAD) PORT_NAME("0 / Aries")
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("IN.1") // DIO1
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_C) PORT_NAME("Career")
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_S) PORT_NAME("Spirit")
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_1) PORT_CODE(KEYCODE_1_PAD) PORT_NAME("1 / Taurus")
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("IN.2") // DIO2
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_T) PORT_NAME("Travel")
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_A) PORT_NAME("Family")
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_2) PORT_CODE(KEYCODE_2_PAD) PORT_NAME("2 / Gemini")
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("IN.3") // DIO3
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_M) PORT_NAME("Money")
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_R) PORT_NAME("Creative")
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_3) PORT_CODE(KEYCODE_3_PAD) PORT_NAME("3 / Cancer")
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("IN.4") // DIO4
	PORT_BIT( 0x03, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_4) PORT_CODE(KEYCODE_4_PAD) PORT_NAME("4 / Leo")
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_MINUS) PORT_CODE(KEYCODE_MINUS_PAD) PORT_NAME("Aquarius / Reset")

	PORT_START("IN.5") // DIO5
	PORT_BIT( 0x03, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_5) PORT_CODE(KEYCODE_5_PAD) PORT_NAME("5 / Virgo")
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_EQUALS) PORT_CODE(KEYCODE_PLUS_PAD) PORT_NAME("Pisces")

	PORT_START("IN.6") // DIO6
	PORT_BIT( 0x0b, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_6) PORT_CODE(KEYCODE_6_PAD) PORT_NAME("6 / Libra")

	PORT_START("IN.7") // DIO7
	PORT_BIT( 0x0b, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_7) PORT_CODE(KEYCODE_7_PAD) PORT_NAME("7 / Scorpio")

	PORT_START("IN.8") // DIO8
	PORT_BIT( 0x0b, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_8) PORT_CODE(KEYCODE_8_PAD) PORT_NAME("8 / Sagittarius")

	PORT_START("IN.9") // DIO9
	PORT_BIT( 0x0b, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_9) PORT_CODE(KEYCODE_9_PAD) PORT_NAME("9 / Capricorn")

	PORT_START("IN.10") // PI4
	PORT_CONFNAME( 0x08, 0x00, "Mode" )
	PORT_CONFSETTING(    0x08, "C" ) // compatibility
	PORT_CONFSETTING(    0x00, "P" ) // personal
INPUT_PORTS_END

void horocomp_state::horocomp(machine_config &config)
{
	// basic machine hardware
	MM78LA(config, m_maincpu, 440000); // approximation - VC osc. R=10K
	m_maincpu->write_d().set(FUNC(horocomp_state::write_d));
	m_maincpu->write_r().set(FUNC(horocomp_state::write_r));
	m_maincpu->read_p().set(FUNC(horocomp_state::read_p));
	m_maincpu->write_spk().set(FUNC(horocomp_state::write_spk));

	// video hardware
	PWM_DISPLAY(config, m_display).set_size(8, 14);
	m_display->set_segmask(0xff, 0x3fff);
	config.set_default_layout(layout_horocomp);

	// sound hardware
	SPEAKER(config, "mono").front_center();
	SPEAKER_SOUND(config, m_speaker);
	static const double speaker_levels[4] = { 0.0, 1.0, -1.0, 0.0 };
	m_speaker->set_levels(4, speaker_levels);
	m_speaker->add_route(ALL_OUTPUTS, "mono", 0.25);
}

// roms

ROM_START( horocomp )
	ROM_REGION( 0x0800, "maincpu", 0 )
	ROM_LOAD( "mm95_b9001-13", 0x0000, 0x0800, CRC(d284a837) SHA1(02092db2a29b4bb7e9f86286601b67b2e9556476) )

	ROM_REGION( 605, "maincpu:opla", 0 )
	ROM_LOAD( "mm78la_horocomp_output.pla", 0, 605, CRC(4fae532f) SHA1(3a08c0fa3ce476c014280b3cfeb6aa37824ae503) )
ROM_END





/***************************************************************************

  Mattel World Championship Football (model 3202)
  * MM78 MCU (MM78L pinout) (label MM78 A78C6-12, die label A78C6)
  * MM78 MCU (MM78L pinout) (label MM78 A78C7-12, die label A78C7)
  * 8-digit 7seg VFD, cyan/red/green VFD Itron CP5023, 1-bit sound

  It was patented under US4422639. Like the Baseball counterpart (mwcbaseb in
  hh_hmcs40.cpp), this handheld is a complex game.

***************************************************************************/

class mwcfootb_state : public hh_pps41_state
{
public:
	mwcfootb_state(const machine_config &mconfig, device_type type, const char *tag) :
		hh_pps41_state(mconfig, type, tag),
		m_subcpu(*this, "subcpu")
	{ }

	void mwcfootb(machine_config &config);

private:
	required_device<pps41_base_device> m_subcpu;

	void update_display();

	void main_write_d(u16 data);
	u16 main_read_d();
	void main_write_r(u16 data);
	u8 main_read_p();

	void sub_write_d(u16 data);
	void sub_write_r(u16 data);
};

// handlers

void mwcfootb_state::update_display()
{
	m_display->matrix(m_grid, bitswap<19>(m_plate, 19,18,17,16, 11,10,9,8,15,14,13,12, 2,3,1,0,6,5,4));
}

// maincpu side

void mwcfootb_state::main_write_d(u16 data)
{
	// DIO0-DIO7: vfd grid
	// DIO0-DIO2: input mux
	m_grid = m_inp_mux = data;
	update_display();

	// DIO8: subcpu INT0
	m_subcpu->set_input_line(0, (data & 0x100) ? ASSERT_LINE : CLEAR_LINE);
}

u16 mwcfootb_state::main_read_d()
{
	// DIO9: subcpu DIO9
	return m_subcpu->d_output_r() & 0x200;
}

void mwcfootb_state::main_write_r(u16 data)
{
	// RIO1-RIO7: vfd plate 0-6
	m_plate = (m_plate & 0xfff00) | (~data & 0x7f);
	update_display();

	// RIO8: speaker out
	m_speaker->level_w(BIT(~data, 7));
}

u8 mwcfootb_state::main_read_p()
{
	// PI1-PI8: multiplexed inputs
	return ~read_inputs(3);
}

// subcpu side

void mwcfootb_state::sub_write_d(u16 data)
{
	// DIO0-DIO3: vfd plate 15-18
	m_plate = (m_plate & 0x0ffff) | (data << 16 & 0xf0000);
	update_display();

	// DIO9: maincpu INT0 (+DIO9)
	m_maincpu->set_input_line(0, (data & 0x200) ? ASSERT_LINE : CLEAR_LINE);
}

void mwcfootb_state::sub_write_r(u16 data)
{
	// RIO1-RIO8: vfd plate 7-14
	m_plate = (m_plate & 0xf00ff) | (~data << 8 & 0xff00);
	update_display();
}

// config

/* physical button layout and labels are like this:

     (home team side)                                                      (visitor team side)
    [1] RECEIVERS [2]                                                       [1] RECEIVERS [2]

           [1]                                                                     [1]
    [4]  [PAUSE]  [2]                                                       [4]  [PAUSE]  [2]
           [3]                                                                     [3]
                           DOWN      QUARTER
         [ENTER]        YDS. TO GO  TIME LEFT        POSITION    SCORE           [ENTER]
    [KICK]     [TIME]     [    ]     [    ]           [    ]     [    ]     [TIME]     [KICK]

           [^]                                                                     [^]
    [<]   [P/C]   [>]                                                       [<]   [P/C]   [>]
           [v]                                                                     [v]
*/

static INPUT_PORTS_START( mwcfootb ) // P1 = left/home, P2 = right/visitor
	PORT_START("IN.0") // DIO0
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_START4 ) PORT_NAME("Score")
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_START3 ) PORT_NAME("Position")
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_BUTTON2 ) PORT_PLAYER(2) PORT_NAME("P2 Kick")
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_PLAYER(2) PORT_NAME("P2 P/C/Pause")
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_START2 ) PORT_NAME("Quarter Time Left")
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_START1 ) PORT_NAME("Down / Yards To Go")
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_BUTTON2 ) PORT_PLAYER(1) PORT_NAME("P1 Kick")
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_PLAYER(1) PORT_NAME("P1 P/C/Pause")

	PORT_START("IN.1") // DIO1
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_BUTTON6 ) PORT_PLAYER(2) PORT_NAME("P2 Receiver 2")
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_BUTTON3 ) PORT_PLAYER(2) PORT_NAME("P2 Time")
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_BUTTON4 ) PORT_PLAYER(2) PORT_NAME("P2 Enter")
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_BUTTON5 ) PORT_PLAYER(2) PORT_NAME("P2 Receiver 1")
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_BUTTON6 ) PORT_PLAYER(1) PORT_NAME("P1 Receiver 2")
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_BUTTON3 ) PORT_PLAYER(1) PORT_NAME("P1 Time")
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_BUTTON4 ) PORT_PLAYER(1) PORT_NAME("P1 Enter")
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_BUTTON5 ) PORT_PLAYER(1) PORT_NAME("P1 Receiver 1")

	PORT_START("IN.2") // DIO2
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT ) PORT_PLAYER(2) PORT_16WAY PORT_NAME("P2 Left/4")
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT ) PORT_PLAYER(2) PORT_16WAY PORT_NAME("P2 Right/2")
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_JOYSTICK_DOWN ) PORT_PLAYER(2) PORT_16WAY PORT_NAME("P2 Down/3")
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_JOYSTICK_UP ) PORT_PLAYER(2) PORT_16WAY PORT_NAME("P2 Up/1")
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT ) PORT_PLAYER(1) PORT_16WAY PORT_NAME("P1 Left/4")
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT ) PORT_PLAYER(1) PORT_16WAY PORT_NAME("P1 Right/2")
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_JOYSTICK_DOWN ) PORT_PLAYER(1) PORT_16WAY PORT_NAME("P1 Down/3")
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_JOYSTICK_UP ) PORT_PLAYER(1) PORT_16WAY PORT_NAME("P1 Up/1")
INPUT_PORTS_END

void mwcfootb_state::mwcfootb(machine_config &config)
{
	// basic machine hardware
	MM78(config, m_maincpu, 360000); // approximation - VC osc. R=56K
	m_maincpu->write_d().set(FUNC(mwcfootb_state::main_write_d));
	m_maincpu->read_d().set(FUNC(mwcfootb_state::main_read_d));
	m_maincpu->write_r().set(FUNC(mwcfootb_state::main_write_r));
	m_maincpu->read_p().set(FUNC(mwcfootb_state::main_read_p));
	m_maincpu->read_sdi().set(m_subcpu, FUNC(pps41_base_device::sdo_r));
	m_maincpu->write_ssc().set(m_subcpu, FUNC(pps41_base_device::ssc_w));

	MM78(config, m_subcpu, 360000); // osc. from maincpu
	m_subcpu->write_d().set(FUNC(mwcfootb_state::sub_write_d));
	m_subcpu->write_r().set(FUNC(mwcfootb_state::sub_write_r));
	m_subcpu->read_sdi().set(m_maincpu, FUNC(pps41_base_device::sdo_r));

	config.set_perfect_quantum(m_maincpu);

	// video hardware
	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_SVG));
	screen.set_refresh_hz(60);
	screen.set_size(1920, 571);
	screen.set_visarea_full();

	PWM_DISPLAY(config, m_display).set_size(8, 19);
	m_display->set_segmask(0x7f, 0x7f);
	config.set_default_layout(layout_mwcfootb);

	// sound hardware
	SPEAKER(config, "mono").front_center();
	SPEAKER_SOUND(config, m_speaker);
	m_speaker->add_route(ALL_OUTPUTS, "mono", 0.25);
}

// roms

ROM_START( mwcfootb )
	ROM_REGION( 0x0800, "maincpu", 0 )
	ROM_LOAD( "mm78_a78c6-12", 0x0000, 0x0800, CRC(91cf0d9b) SHA1(8d778b441eb26fcff50e8532c142f368c0dd5818) )

	ROM_REGION( 0x0800, "subcpu", 0 )
	ROM_LOAD( "mm78_a78c7-12", 0x0000, 0x0800, CRC(b991d06e) SHA1(1f801b5cd7214f7378ae3f19799b84a9dc5bba4e) )

	ROM_REGION( 248486, "screen", 0)
	ROM_LOAD( "mwcfootb.svg", 0, 248486, CRC(03d17b85) SHA1(c877316c0c7923432235655d810fea8d714a4b31) )
ROM_END





/***************************************************************************

  Selchow & Righter Scrabble Sensor
  * MM76EL MCU (label B8610-11, die label B8610)
  * 16 leds, 1-bit sound

  The game concept is similar to Mastermind. Enter a word (or press AUTO.)
  to start the game, then try to guess it.

***************************************************************************/

class scrabsen_state : public hh_pps41_state
{
public:
	scrabsen_state(const machine_config &mconfig, device_type type, const char *tag) :
		hh_pps41_state(mconfig, type, tag)
	{ }

	void scrabsen(machine_config &config);

	DECLARE_INPUT_CHANGED_MEMBER(players_switch) { update_int(); }

private:
	virtual void update_int() override;
	void update_display();
	void write_d(u16 data);
	void write_r(u16 data);
	u8 read_p();
};

// handlers

void scrabsen_state::update_int()
{
	// players switch is tied to MCU INT0
	m_maincpu->set_input_line(0, (m_inputs[5]->read() & 1) ? ASSERT_LINE : CLEAR_LINE);
}

void scrabsen_state::update_display()
{
	m_display->matrix(m_inp_mux >> 6 & 3, ~m_r);
}

void scrabsen_state::write_d(u16 data)
{
	// DIO0-DIO4: input mux
	// DIO6,DIO7: led select
	m_inp_mux = data;
	update_display();

	// DIO8: speaker out
	m_speaker->level_w(BIT(data, 8));
}

void scrabsen_state::write_r(u16 data)
{
	// RIO1-RIO8: led data
	m_r = data;
	update_display();
}

u8 scrabsen_state::read_p()
{
	// PI1-PI7: multiplexed inputs
	return ~read_inputs(5);
}

// config

static INPUT_PORTS_START( scrabsen )
	PORT_START("IN.0") // DIO0
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_D) PORT_CHAR('D')
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_S) PORT_CHAR('S')
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_N) PORT_CHAR('N')
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_DEL) PORT_CODE(KEYCODE_BACKSPACE) PORT_CHAR(8) PORT_NAME("Clear")
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_I) PORT_CHAR('I')

	PORT_START("IN.1") // DIO1
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_A) PORT_CHAR('A')
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_E) PORT_CHAR('E')
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_T) PORT_CHAR('T')
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_X) PORT_CHAR('X')
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_O) PORT_CHAR('O')
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_SPACE) PORT_CHAR(' ') PORT_NAME("Space")
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_J) PORT_CHAR('J')

	PORT_START("IN.2") // DIO2
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_H) PORT_CHAR('H')
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_W) PORT_CHAR('W')
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_R) PORT_CHAR('R')
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_ENTER) PORT_CODE(KEYCODE_ENTER_PAD) PORT_CHAR(13) PORT_NAME("Enter")
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_M) PORT_CHAR('M')

	PORT_START("IN.3") // DIO3
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_C) PORT_CHAR('C')
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_G) PORT_CHAR('G')
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_V) PORT_CHAR('V')
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_Z) PORT_CHAR('Z')
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_Q) PORT_CHAR('Q')
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_STOP) PORT_CHAR('.') PORT_NAME("Auto.")
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_L) PORT_CHAR('L')

	PORT_START("IN.4") // DIO4
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_B) PORT_CHAR('B')
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_F) PORT_CHAR('F')
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_U) PORT_CHAR('U')
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_Y) PORT_CHAR('Y')
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_P) PORT_CHAR('P')
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_K) PORT_CHAR('K')

	PORT_START("IN.5") // INT0
	PORT_CONFNAME( 0x01, 0x01, "Players" ) PORT_CHANGED_MEMBER(DEVICE_SELF, scrabsen_state, players_switch, 0)
	PORT_CONFSETTING(    0x01, "1" ) // single
	PORT_CONFSETTING(    0x00, "2" ) // double
INPUT_PORTS_END

void scrabsen_state::scrabsen(machine_config &config)
{
	// basic machine hardware
	MM76EL(config, m_maincpu, 380000); // approximation - VC osc. R=56K
	m_maincpu->write_d().set(FUNC(scrabsen_state::write_d));
	m_maincpu->write_r().set(FUNC(scrabsen_state::write_r));
	m_maincpu->read_p().set(FUNC(scrabsen_state::read_p));

	// video hardware
	PWM_DISPLAY(config, m_display).set_size(2, 8);
	config.set_default_layout(layout_scrabsen);

	// sound hardware
	SPEAKER(config, "mono").front_center();
	SPEAKER_SOUND(config, m_speaker);
	m_speaker->add_route(ALL_OUTPUTS, "mono", 0.25);
}

// roms

ROM_START( scrabsen )
	ROM_REGION( 0x0400, "maincpu", 0 )
	ROM_LOAD( "b8610-11", 0x0000, 0x0400, CRC(97c8a466) SHA1(ed5d2cddd2761ed6e3ddc47d97b2ed19a2aaeee9) )

	ROM_REGION( 314, "maincpu:opla", 0 ) // unused
	ROM_LOAD( "mm76_scrabsen_output.pla", 0, 314, CRC(410fa6d7) SHA1(d46aaf1ec2c942083cba7dbd59d4261dc238d4c8) )
ROM_END





/***************************************************************************

  Selchow & Righter Reader's Digest Q&A
  * MM76EL MCU (label MM76EL B8654-11, die label B8654)
  * 9-digit 7seg LED display(4 unused), 2-bit sound

  The game requires question books. The player inputs a 3-digit code and
  answers 20 multiple-choice questions from the page.

***************************************************************************/

class rdqa_state : public hh_pps41_state
{
public:
	rdqa_state(const machine_config &mconfig, device_type type, const char *tag) :
		hh_pps41_state(mconfig, type, tag)
	{ }

	void rdqa(machine_config &config);

	DECLARE_INPUT_CHANGED_MEMBER(players_switch) { update_int(); }

private:
	virtual void update_int() override;
	void update_display();
	void write_d(u16 data);
	void write_r(u16 data);
	u8 read_p();
};

// handlers

void rdqa_state::update_int()
{
	// players switch is tied to MCU INT0
	m_maincpu->set_input_line(0, (m_inputs[4]->read() & 1) ? ASSERT_LINE : CLEAR_LINE);
}

void rdqa_state::update_display()
{
	m_display->matrix(m_inp_mux, ~m_r);
}

void rdqa_state::write_d(u16 data)
{
	// DIO0-DIO4: digit select
	// DIO0-DIO3: input mux
	m_inp_mux = data;
	update_display();

	// DIO8,DIO9: speaker out
	m_speaker->level_w(data >> 8 & 3);
}

void rdqa_state::write_r(u16 data)
{
	// RIO1-RIO7: digit segment data
	m_r = data;
	update_display();
}

u8 rdqa_state::read_p()
{
	// PI1-PI5: multiplexed inputs
	return ~read_inputs(4);
}

// config

static INPUT_PORTS_START( rdqa )
	PORT_START("IN.0") // DIO0
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_1) PORT_NAME("1")
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_2) PORT_NAME("2")
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_3) PORT_NAME("3")
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_4) PORT_NAME("4")
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_5) PORT_NAME("5")

	PORT_START("IN.1") // DIO1
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_6) PORT_NAME("6")
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_7) PORT_NAME("7")
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_8) PORT_NAME("8")
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_9) PORT_NAME("9")
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_0) PORT_NAME("0")

	PORT_START("IN.2") // DIO2
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_A) PORT_NAME("A")
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_B) PORT_NAME("B")
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_C) PORT_NAME("C")
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_D) PORT_NAME("D")
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_S) PORT_NAME("Score")

	PORT_START("IN.3") // DIO3
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_R) PORT_NAME("Right")
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_W) PORT_NAME("Wrong")
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_X) PORT_NAME("Pass")
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_DEL) PORT_CODE(KEYCODE_BACKSPACE) PORT_NAME("Clear")
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_ENTER) PORT_CODE(KEYCODE_ENTER_PAD) PORT_NAME("Enter")

	PORT_START("IN.4") // INT0
	PORT_CONFNAME( 0x01, 0x01, "Players" ) PORT_CHANGED_MEMBER(DEVICE_SELF, rdqa_state, players_switch, 0)
	PORT_CONFSETTING(    0x01, "1" ) // single
	PORT_CONFSETTING(    0x00, "2" ) // double
INPUT_PORTS_END

void rdqa_state::rdqa(machine_config &config)
{
	// basic machine hardware
	MM76EL(config, m_maincpu, 400000); // approximation - VC osc. R=56K
	m_maincpu->write_d().set(FUNC(rdqa_state::write_d));
	m_maincpu->write_r().set(FUNC(rdqa_state::write_r));
	m_maincpu->read_p().set(FUNC(rdqa_state::read_p));

	// video hardware
	PWM_DISPLAY(config, m_display).set_size(5, 7);
	m_display->set_segmask(0x1f, 0x7f);
	config.set_default_layout(layout_rdqa);

	// sound hardware
	SPEAKER(config, "mono").front_center();
	SPEAKER_SOUND(config, m_speaker);
	static const double speaker_levels[4] = { 0.0, 1.0, -1.0, 0.0 };
	m_speaker->set_levels(4, speaker_levels);
	m_speaker->add_route(ALL_OUTPUTS, "mono", 0.25);
}

// roms

ROM_START( rdqa )
	ROM_REGION( 0x0400, "maincpu", 0 )
	ROM_LOAD( "mm76el_b8654-11", 0x0000, 0x0400, CRC(95c00eee) SHA1(1537626df03a7131d83a555e557a4528e093a22a) )

	ROM_REGION( 314, "maincpu:opla", 0 )
	ROM_LOAD( "mm76_rdqa_output.pla", 0, 314, CRC(e024b2d3) SHA1(fc3121e70f22151cf8f3411f9fcbac88002ae330) )
ROM_END



} // anonymous namespace

/***************************************************************************

  Game driver(s)

***************************************************************************/

//    YEAR  NAME       PARENT  CMP MACHINE    INPUT     CLASS           INIT        COMPANY, FULLNAME, FLAGS
CONS( 1979, ftri1,     0,       0, ftri1,     ftri1,    ftri1_state,    empty_init, "Fonas", "Tri-1 (Fonas)", MACHINE_SUPPORTS_SAVE )

CONS( 1979, mastmind,  0,       0, mastmind,  mastmind, mastmind_state, empty_init, "Invicta", "Electronic Master Mind (Invicta)", MACHINE_SUPPORTS_SAVE | MACHINE_NO_SOUND_HW )
CONS( 1979, smastmind, 0,       0, smastmind, mastmind, mastmind_state, empty_init, "Invicta", "Super-Sonic Electronic Master Mind", MACHINE_SUPPORTS_SAVE )

CONS( 1979, dunksunk,  0,       0, dunksunk,  dunksunk, dunksunk_state, empty_init, "Kmart", "Dunk 'n Sunk", MACHINE_SUPPORTS_SAVE )

CONS( 1978, memoquiz,  0,       0, memoquiz,  memoquiz, memoquiz_state, empty_init, "M.E.M. Belgium", "Memoquiz", MACHINE_SUPPORTS_SAVE | MACHINE_NO_SOUND_HW )

CONS( 1978, mfootb2,   0,       0, mfootb2,   mfootb2,  mfootb2_state,  empty_init, "Mattel Electronics", "Football 2 (Mattel)", MACHINE_SUPPORTS_SAVE )
CONS( 1979, brainbaf,  0,       0, brainbaf,  brainbaf, brainbaf_state, empty_init, "Mattel Electronics", "Brain Baffler", MACHINE_SUPPORTS_SAVE )
COMP( 1979, horocomp,  0,       0, horocomp,  horocomp, horocomp_state, empty_init, "Mattel Electronics", "Horoscope Computer", MACHINE_SUPPORTS_SAVE )
CONS( 1980, mwcfootb,  0,       0, mwcfootb,  mwcfootb, mwcfootb_state, empty_init, "Mattel Electronics", "World Championship Football", MACHINE_SUPPORTS_SAVE )

CONS( 1978, scrabsen,  0,       0, scrabsen,  scrabsen, scrabsen_state, empty_init, "Selchow & Righter", "Scrabble Sensor - Electronic Word Game", MACHINE_SUPPORTS_SAVE )
CONS( 1980, rdqa,      0,       0, rdqa,      rdqa,     rdqa_state,     empty_init, "Selchow & Righter", "Reader's Digest Q&A - Computer Question & Answer Game", MACHINE_SUPPORTS_SAVE ) // ***

// ***: As far as MAME is concerned, the game is emulated fine. But for it to be playable, it requires interaction
// with other, unemulatable, things eg. game board/pieces, book, playing cards, pen & paper, etc.
