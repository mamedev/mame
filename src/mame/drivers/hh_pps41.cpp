// license:BSD-3-Clause
// copyright-holders:hap
// thanks-to:Sean Riddle, Kevin Horton
/***************************************************************************

  Rockwell PPS-4/1 MCU series handhelds

***************************************************************************/

#include "emu.h"

#include "cpu/pps41/mm75.h"
#include "cpu/pps41/mm76.h"
#include "cpu/pps41/mm78.h"
#include "video/pwm.h"
#include "sound/spkrdev.h"
#include "speaker.h"

// internal artwork
#include "mastmind.lh"
#include "memoquiz.lh"
#include "scrabsen.lh"

#include "hh_pps41_test.lh" // common test-layout - use external artwork


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

	// devices
	required_device<pps41_base_device> m_maincpu;
	optional_device<pwm_display_device> m_display;
	optional_device<speaker_sound_device> m_speaker;
	optional_ioport_array<6> m_inputs; // max 6

	u16 m_inp_mux = 0;

	// MCU output pin state
	u16 m_d = 0;
	u8 m_r = ~0;

	u8 read_inputs(int columns);

protected:
	virtual void update_int() { ; }

	virtual void machine_start() override;
	virtual void machine_reset() override { update_int(); }
	virtual void device_post_load() override { update_int(); }
};


// machine start

void hh_pps41_state::machine_start()
{
	// register for savestates
	save_item(NAME(m_inp_mux));
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

	// active low by default
	return ~ret;
}



/***************************************************************************

  Minidrivers (subclass, I/O, Inputs, Machine Config, ROM Defs)

***************************************************************************/

namespace {

/***************************************************************************

  Fonas Tri-1
  * MM78 MCU variant with 40 pins (label ?, die label A7859)
  * 4 7seg leds, 41 other leds, 1-bit sound

***************************************************************************/

class ftri1_state : public hh_pps41_state
{
public:
	ftri1_state(const machine_config &mconfig, device_type type, const char *tag) :
		hh_pps41_state(mconfig, type, tag)
	{ }

	void update_display();
	void write_d(u16 data);
	void write_r(u8 data);
	void ftri1(machine_config &config);
};

// handlers

void ftri1_state::update_display()
{
	m_display->matrix((m_d << 4 & 0x100) | (~m_r & 0xff), bitswap<8>(m_d, 2,5,8,0,1,3,7,6));
}

void ftri1_state::write_d(u16 data)
{
	// DIO0-DIO3, DIO5-DIO8: digit/led data
	// DIO4: another led select
	m_d = data;
	update_display();

	// DIO9: speaker out
	m_speaker->level_w(BIT(data, 9));
}

void ftri1_state::write_r(u8 data)
{
	// RIO1-RIO8: digit/led select
	m_r = data;
	update_display();
}

// config

static INPUT_PORTS_START( ftri1 )
	PORT_START("IN.0")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_1)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_2)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_3)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_4)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_5)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_6)
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_7)
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_8)
INPUT_PORTS_END

void ftri1_state::ftri1(machine_config &config)
{
	/* basic machine hardware */
	MM78(config, m_maincpu, 600000); // approximation
	m_maincpu->write_d().set(FUNC(ftri1_state::write_d));
	m_maincpu->write_r().set(FUNC(ftri1_state::write_r));
	m_maincpu->read_p().set_ioport("IN.0");

	/* video hardware */
	PWM_DISPLAY(config, m_display).set_size(9, 8);
	m_display->set_segmask(0xff, 0x7f);
	config.set_default_layout(layout_hh_pps41_test);

	/* sound hardware */
	SPEAKER(config, "mono").front_center();
	SPEAKER_SOUND(config, m_speaker);
	m_speaker->add_route(ALL_OUTPUTS, "mono", 0.25);
}

// roms

ROM_START( ftri1 )
	ROM_REGION( 0x0800, "maincpu", 0 )
	ROM_LOAD( "ftri1.bin", 0x0000, 0x0800, CRC(3c957f1d) SHA1(42db81a78bbef971a84e61a26d91f7411980d79c) )
ROM_END





/***************************************************************************

  Invicta Electronic Master Mind
  * MM75 MCU (label MM75 A7525-11, die label A7525)
  * 9-digit 7seg VFD display (Futaba 9-ST)

  Invicta is the owner of the Mastermind game rights. The back of the unit
  says (C) 1977, but this electronic handheld version came out in 1979.
  Or maybe there's an older revision.

***************************************************************************/

class mastmind_state : public hh_pps41_state
{
public:
	mastmind_state(const machine_config &mconfig, device_type type, const char *tag) :
		hh_pps41_state(mconfig, type, tag)
	{ }

	void update_display();
	void write_d(u16 data);
	void write_r(u8 data);
	u8 read_p();
	void mastmind(machine_config &config);
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
}

void mastmind_state::write_r(u8 data)
{
	// RIO1-RIO7: digit segment data
	m_r = data;
	update_display();
}

u8 mastmind_state::read_p()
{
	// PI1-PI4: multiplexed inputs
	return read_inputs(4);
}

// config

static INPUT_PORTS_START( mastmind )
	PORT_START("IN.0") // DIO0
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_ENTER) PORT_CODE(KEYCODE_ENTER_PAD) PORT_NAME("Try")
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_F) PORT_NAME("Fail")
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_UNUSED ) // display test?
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
	/* basic machine hardware */
	MM75(config, m_maincpu, 700000); // approximation
	m_maincpu->write_d().set(FUNC(mastmind_state::write_d));
	m_maincpu->write_r().set(FUNC(mastmind_state::write_r));
	m_maincpu->read_p().set(FUNC(mastmind_state::read_p));

	/* video hardware */
	PWM_DISPLAY(config, m_display).set_size(8, 7);
	m_display->set_segmask(0xff, 0x7f);
	config.set_default_layout(layout_mastmind);

	/* no sound! */
}

// roms

ROM_START( mastmind )
	ROM_REGION( 0x0400, "maincpu", ROMREGION_ERASE00 )
	ROM_LOAD( "mm75_a7525-11", 0x0000, 0x0200, CRC(39dbdd50) SHA1(72fa5781e9df62d91d57437ded2931fab8253c3c) )
	ROM_CONTINUE(              0x0380, 0x0080 )

	ROM_REGION( 314, "maincpu:opla", 0 )
	ROM_LOAD( "mm76_mastmind_output.pla", 0, 314, CRC(c936aee7) SHA1(e9ec08a82493d6b63e936f82deeab3e4449b54c3) )
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
  - Europe: Memoquiz
  - UK: Memoquiz, published by Polymark
  - USA: Mind Boggler (model 2626), published by Mattel

***************************************************************************/

class memoquiz_state : public hh_pps41_state
{
public:
	memoquiz_state(const machine_config &mconfig, device_type type, const char *tag) :
		hh_pps41_state(mconfig, type, tag)
	{ }

	DECLARE_INPUT_CHANGED_MEMBER(digits_switch) { update_int(); }
	virtual void update_int() override;

	void update_display();
	void write_d(u16 data);
	void write_r(u8 data);
	u8 read_p();
	void memoquiz(machine_config &config);
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

void memoquiz_state::write_r(u8 data)
{
	// RIO1-RIO7: digit segment data
	m_r = data;
	update_display();
}

u8 memoquiz_state::read_p()
{
	// PI1-PI4: multiplexed inputs
	return read_inputs(4);
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
	/* basic machine hardware */
	MM75(config, m_maincpu, 700000); // approximation
	m_maincpu->write_d().set(FUNC(memoquiz_state::write_d));
	m_maincpu->write_r().set(FUNC(memoquiz_state::write_r));
	m_maincpu->read_p().set(FUNC(memoquiz_state::read_p));

	/* video hardware */
	PWM_DISPLAY(config, m_display).set_size(8, 8);
	m_display->set_segmask(0xff, 0xff);
	config.set_default_layout(layout_memoquiz);

	/* no sound! */
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

	DECLARE_INPUT_CHANGED_MEMBER(players_switch) { update_int(); }
	virtual void update_int() override;

	void update_display();
	void write_d(u16 data);
	void write_r(u8 data);
	u8 read_p();
	void scrabsen(machine_config &config);
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

void scrabsen_state::write_r(u8 data)
{
	// RIO1-RIO8: led data
	m_r = data;
	update_display();
}

u8 scrabsen_state::read_p()
{
	// PI1-PI7: multiplexed inputs
	return read_inputs(5);
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
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("IN.1") // DIO1
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_A) PORT_CHAR('A')
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_E) PORT_CHAR('E')
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_T) PORT_CHAR('T')
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_X) PORT_CHAR('X')
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_O) PORT_CHAR('O')
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_SPACE) PORT_CHAR(' ') PORT_NAME("Space")
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_J) PORT_CHAR('J')
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("IN.4") // DIO4
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_B) PORT_CHAR('B')
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_F) PORT_CHAR('F')
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_U) PORT_CHAR('U')
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_Y) PORT_CHAR('Y')
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_P) PORT_CHAR('P')
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_K) PORT_CHAR('K')
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("IN.3") // DIO3
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_C) PORT_CHAR('C')
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_G) PORT_CHAR('G')
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_V) PORT_CHAR('V')
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_Z) PORT_CHAR('Z')
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_Q) PORT_CHAR('Q')
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_STOP) PORT_CHAR('.') PORT_NAME("Auto.")
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_L) PORT_CHAR('L')
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("IN.2") // DIO2
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_H) PORT_CHAR('H')
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_W) PORT_CHAR('W')
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_R) PORT_CHAR('R')
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_ENTER) PORT_CODE(KEYCODE_ENTER_PAD) PORT_CHAR(13) PORT_NAME("Enter")
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_M) PORT_CHAR('M')
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("IN.5") // INT0
	PORT_CONFNAME( 0x01, 0x01, "Players" ) PORT_CHANGED_MEMBER(DEVICE_SELF, scrabsen_state, players_switch, 0)
	PORT_CONFSETTING(    0x01, "1" ) // single
	PORT_CONFSETTING(    0x00, "2" ) // double
INPUT_PORTS_END

void scrabsen_state::scrabsen(machine_config &config)
{
	/* basic machine hardware */
	MM76EL(config, m_maincpu, 750000); // approximation
	m_maincpu->write_d().set(FUNC(scrabsen_state::write_d));
	m_maincpu->write_r().set(FUNC(scrabsen_state::write_r));
	m_maincpu->read_p().set(FUNC(scrabsen_state::read_p));

	/* video hardware */
	PWM_DISPLAY(config, m_display).set_size(2, 8);
	config.set_default_layout(layout_scrabsen);

	/* sound hardware */
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



} // anonymous namespace

/***************************************************************************

  Game driver(s)

***************************************************************************/

//    YEAR  NAME      PARENT  CMP MACHINE   INPUT     CLASS           INIT        COMPANY, FULLNAME, FLAGS
CONS( 1979, ftri1,    0,       0, ftri1,    ftri1,    ftri1_state,    empty_init, "Fonas", "Tri-1 (Fonas)", MACHINE_SUPPORTS_SAVE | MACHINE_NOT_WORKING )

CONS( 1979, mastmind, 0,       0, mastmind, mastmind, mastmind_state, empty_init, "Invicta Plastics", "Electronic Master Mind (Invicta)", MACHINE_SUPPORTS_SAVE | MACHINE_NO_SOUND_HW )

CONS( 1978, memoquiz, 0,       0, memoquiz, memoquiz, memoquiz_state, empty_init, "M.E.M. Belgium", "Memoquiz", MACHINE_SUPPORTS_SAVE | MACHINE_NO_SOUND_HW )

CONS( 1980, scrabsen, 0,       0, scrabsen, scrabsen, scrabsen_state, empty_init, "Selchow & Righter", "Scrabble Sensor - Electronic Word Game", MACHINE_SUPPORTS_SAVE )
