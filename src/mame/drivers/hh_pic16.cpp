// license:BSD-3-Clause
// copyright-holders:hap
// thanks-to:Sean Riddle, Kevin Horton
/***************************************************************************

GI PIC 16xx-driven dedicated handhelds or other simple devices.

known chips:

  serial  device  etc.
-----------------------------------------------------------
 *020     1650    19??, GI Economega IV TV PPL Tuning System Control
 *021     1650    1978, GI AY-3-8910 demo board
 @024     1655    1979, Toytronic? Football
 @033     1655A   1979, Toytronic Football (newer)
 @036     1655A   1979, Ideal Maniac
 @043     1655A   1979, Caprice Pro-Action Baseball
 @049     1655A   1980, Kingsford Match Me(?)/Mini Match Me
 @051     1655A   1979, Kmart Dr. Dunk/Tandy Electronic Basketball
 @053     1655A   1979, Atari Touch Me
 @0??     1655A   1979, Tiger Half Court Computer Basketball/Sears Electronic Basketball (custom label)
 @061     1655A   1980, Lakeside Le Boom
 @078     1655A   1980, Ideal Flash
 *081     1655A   1981, Ramtex Space Invaders/Block Buster
 *085     1655A   1980, VTech Soccer 2/Grandstand Match of the Day Soccer
 @094     1655A   1980, GAF Melody Madness
 @110     1650A   1979, Tiger/Tandy Rocket Pinball
 *123     1655A?  1980, Kingsford Match Me/Mini Match Me
 @133     1650A   1981, U.S. Games Programmable Baseball/Tandy 2-Player Baseball
 @144     1650A   1981, U.S. Games/Tandy 2-Player Football
 *192     1650    19??, <unknown> phone dialer (have dump)
 *255     1655    19??, <unknown> talking clock (have dump)
 *518     1650A   19??, GI Teleview Control Chip (features differ per program)
 *519     1650A   19??, "
 *532     1650A   19??, "
 *533     1650A   19??, "
 *536     1650    1982, GI Teleview Autodialer/Terminal Identifier

  (* means undumped unless noted, @ denotes it's in this driver)

ROM source notes when dumped from another publisher, but confident it's the same:
- drdunk: Tandy Electronic Basketball
- flash: Radio Shack Sound Effects Chassis
- hccbaskb: Sears Electronic Basketball
- us2pfball: Tandy 2-Player Football
- uspbball: Tandy 2-Player Baseball

TODO:
- tweak MCU frequency for games when video/audio recording surfaces(YouTube etc.)
- us2pfball player led is brighter, but I can't get a stable picture
- ttfball: discrete sound part, for volume gating?
- what's the relation between drdunk and hccbaskb? Probably made by the same
  Hong Kong subcontractor? I presume Toytronic.
- uspbball and pabball internal artwork

***************************************************************************/

#include "emu.h"

#include "cpu/pic16c5x/pic16c5x.h"
#include "video/pwm.h"
#include "machine/clock.h"
#include "machine/timer.h"
#include "sound/spkrdev.h"

#include "speaker.h"

// internal artwork
#include "drdunk.lh"
#include "flash.lh" // clickable
#include "hccbaskb.lh"
#include "leboom.lh" // clickable
#include "maniac.lh" // clickable
#include "melodym.lh" // clickable
#include "matchme.lh" // clickable
#include "rockpin.lh"
#include "touchme.lh" // clickable
#include "ttfball.lh"
#include "us2pfball.lh"

#include "hh_pic16_test.lh" // common test-layout - use external artwork


class hh_pic16_state : public driver_device
{
public:
	hh_pic16_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_display(*this, "display"),
		m_speaker(*this, "speaker"),
		m_inputs(*this, "IN.%u", 0)
	{ }

	virtual DECLARE_INPUT_CHANGED_MEMBER(reset_button);

protected:
	virtual void machine_start() override;
	virtual void machine_reset() override;

	// devices
	required_device<pic16c5x_device> m_maincpu;
	optional_device<pwm_display_device> m_display;
	optional_device<speaker_sound_device> m_speaker;
	optional_ioport_array<6> m_inputs; // max 6

	// misc common
	u8 m_a = 0;                     // MCU port A write data
	u8 m_b = 0;                     // " B
	u8 m_c = 0;                     // " C
	u8 m_d = 0;                     // " D
	u16 m_inp_mux = ~0;             // multiplexed inputs mask

	u16 read_inputs(int columns, u16 colmask = ~0);
	u8 read_rotated_inputs(int columns, u8 rowmask = ~0);
};


// machine start/reset

void hh_pic16_state::machine_start()
{
	// register for savestates
	save_item(NAME(m_a));
	save_item(NAME(m_b));
	save_item(NAME(m_c));
	save_item(NAME(m_d));
	save_item(NAME(m_inp_mux));
}

void hh_pic16_state::machine_reset()
{
}



/***************************************************************************

  Helper Functions

***************************************************************************/

// generic input handlers

u16 hh_pic16_state::read_inputs(int columns, u16 colmask)
{
	// active low
	u16 ret = ~0 & colmask;

	// read selected input rows
	for (int i = 0; i < columns; i++)
		if (~m_inp_mux >> i & 1)
			ret &= m_inputs[i]->read();

	return ret;
}

u8 hh_pic16_state::read_rotated_inputs(int columns, u8 rowmask)
{
	u8 ret = 0;
	u16 colmask = (1 << columns) - 1;

	// read selected input columns
	for (int i = 0; i < 8; i++)
		if (1 << i & rowmask && ~m_inputs[i]->read() & ~m_inp_mux & colmask)
			ret |= 1 << i;

	// active low
	return ~ret & rowmask;
}

INPUT_CHANGED_MEMBER(hh_pic16_state::reset_button)
{
	// when an input is directly wired to MCU MCLR pin
	m_maincpu->set_input_line(INPUT_LINE_RESET, newval ? ASSERT_LINE : CLEAR_LINE);
}



/***************************************************************************

  Minidrivers (subclass, I/O, Inputs, Machine Config, ROM Defs)

***************************************************************************/

namespace {

/***************************************************************************

  Atari Touch Me
  * PIC 1655A-053
  * 2 7seg LEDs + 4 other LEDs, 1-bit sound

  This is the handheld version of the 1974 arcade game.

  known revisions:
  - Model BH-100 GI C013233 Rev 2 Atari W 1979: PIC 1655A-053
  - Model BH-100 C013150 Rev 6 Atari 1979: AMI C10745 (custom ASIC)

***************************************************************************/

class touchme_state : public hh_pic16_state
{
public:
	touchme_state(const machine_config &mconfig, device_type type, const char *tag) :
		hh_pic16_state(mconfig, type, tag)
	{ }

	void touchme(machine_config &config);

private:
	void update_display();
	void update_speaker();
	u8 read_a();
	void write_b(u8 data);
	void write_c(u8 data);
};

// handlers

void touchme_state::update_display()
{
	m_display->matrix(~m_b & 0x7b, m_c);
}

void touchme_state::update_speaker()
{
	m_speaker->level_w((m_b >> 7 & 1) | (m_c >> 6 & 2));
}

u8 touchme_state::read_a()
{
	// A: multiplexed inputs
	return read_inputs(3, 0xf);
}

void touchme_state::write_b(u8 data)
{
	// B0-B2: input mux
	m_inp_mux = data & 7;

	// B0,B1: digit select
	// B3-B6: leds
	m_b = data;
	update_display();

	// B7: speaker lead 1
	update_speaker();
}

void touchme_state::write_c(u8 data)
{
	// C0-C6: digit segments
	m_c = data;
	update_display();

	// C7: speaker lead 2
	update_speaker();
}

// config

static INPUT_PORTS_START( touchme )
	PORT_START("IN.0") // B0 port A
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_START )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_BUTTON5 ) PORT_NAME("Last")
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_BUTTON6 ) PORT_NAME("High")
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_BUTTON7 ) PORT_NAME("Skill")

	PORT_START("IN.1") // B1 port A
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_NAME("Blue Button")
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_NAME("Yellow Button")
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_NAME("Red Button")
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_BUTTON4 ) PORT_NAME("Green Button")

	PORT_START("IN.2") // B2 port A
	PORT_CONFNAME( 0x07, 0x01^0x07, "Game Select" )
	PORT_CONFSETTING(    0x01^0x07, "1" )
	PORT_CONFSETTING(    0x02^0x07, "2" )
	PORT_CONFSETTING(    0x04^0x07, "3" )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNUSED )
INPUT_PORTS_END

void touchme_state::touchme(machine_config &config)
{
	// basic machine hardware
	PIC1655(config, m_maincpu, 300000); // approximation - RC osc. R=100K, C=47pF
	m_maincpu->read_a().set(FUNC(touchme_state::read_a));
	m_maincpu->write_b().set(FUNC(touchme_state::write_b));
	m_maincpu->read_c().set_constant(0xff);
	m_maincpu->write_c().set(FUNC(touchme_state::write_c));

	// PIC CLKOUT, tied to RTCC
	CLOCK(config, "clock", 300000/4).signal_handler().set_inputline("maincpu", PIC16C5x_RTCC);

	// video hardware
	PWM_DISPLAY(config, m_display).set_size(7, 7);
	m_display->set_segmask(3, 0x7f);
	config.set_default_layout(layout_touchme);

	// sound hardware
	SPEAKER(config, "mono").front_center();
	SPEAKER_SOUND(config, m_speaker);
	static const double speaker_levels[] = { 0.0, 1.0, -1.0, 0.0 };
	m_speaker->set_levels(4, speaker_levels);
	m_speaker->add_route(ALL_OUTPUTS, "mono", 0.25);
}

// roms

ROM_START( touchme )
	ROM_REGION( 0x0400, "maincpu", 0 )
	ROM_LOAD( "pic_1655a-053", 0x0000, 0x0400, CRC(f0858f0a) SHA1(53ffe111d43db1c110847590350ef62f02ed5e0e) )
ROM_END





/***************************************************************************

  Caprice Pro-Action Baseball (manufactured by Calfax)
  * PIC 1655A-043
  * 1 7seg LED + 36 other LEDs, CD4028, 1-bit sound

***************************************************************************/

class pabball_state : public hh_pic16_state
{
public:
	pabball_state(const machine_config &mconfig, device_type type, const char *tag) :
		hh_pic16_state(mconfig, type, tag)
	{ }

	void pabball(machine_config &config);

private:
	void update_display();
	void write_b(u8 data);
	void write_c(u8 data);
};

// handlers

void pabball_state::update_display()
{
	// CD4028 BCD to decimal decoder
	// CD4028 0-8: led select, 9: 7seg
	u16 sel = m_c & 0xf;
	if (sel & 8) sel &= 9;
	sel = 1 << sel;

	m_display->matrix(sel, m_b);
}

void pabball_state::write_b(u8 data)
{
	// B: led data
	m_b = ~data;
	update_display();
}

void pabball_state::write_c(u8 data)
{
	// C2: RTCC pin
	m_maincpu->set_input_line(PIC16C5x_RTCC, data >> 2 & 1);

	// C7: speaker out
	m_speaker->level_w(data >> 7 & 1);

	// C0-C3: CD4028 A-D
	m_c = data;
	update_display();
}

// config

static INPUT_PORTS_START( pabball )
	PORT_START("IN.0") // port A
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_COCKTAIL PORT_NAME("P2 Curve Left")
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_COCKTAIL PORT_NAME("P2 Curve Right")
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_COCKTAIL PORT_NAME("P2 Straight")
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("IN.1") // port C
	PORT_BIT( 0xcf, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_NAME("P1 Hit")
	PORT_CONFNAME( 0x20, 0x00, DEF_STR( Players ) )
	PORT_CONFSETTING(    0x00, "1" )
	PORT_CONFSETTING(    0x20, "2" )

	PORT_START("RESET")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_BUTTON2 ) PORT_CHANGED_MEMBER(DEVICE_SELF, hh_pic16_state, reset_button, 0) PORT_NAME("P1 Reset")
INPUT_PORTS_END

void pabball_state::pabball(machine_config &config)
{
	// basic machine hardware
	PIC1655(config, m_maincpu, 1200000); // approximation - RC osc. R=18K, C=27pF
	m_maincpu->read_a().set_ioport("IN.0");
	m_maincpu->write_b().set(FUNC(pabball_state::write_b));
	m_maincpu->read_c().set_ioport("IN.1");
	m_maincpu->write_c().set(FUNC(pabball_state::write_c));

	// video hardware
	PWM_DISPLAY(config, m_display).set_size(10, 8);
	m_display->set_segmask(0x200, 0xff);
	config.set_default_layout(layout_hh_pic16_test);

	// sound hardware
	SPEAKER(config, "mono").front_center();
	SPEAKER_SOUND(config, m_speaker).add_route(ALL_OUTPUTS, "mono", 0.25);
}

// roms

ROM_START( pabball )
	ROM_REGION( 0x0400, "maincpu", 0 )
	ROM_LOAD( "pic_1655a-043", 0x0000, 0x0400, CRC(43c9b765) SHA1(888a431bab9bcb241c14f33f70863fa2ad89c96b) )
ROM_END





/***************************************************************************

  GAF Melody Madness
  * PIC 1655A-094
  * 2 lamps under tube, 1-bit sound

  Melody Madness is a tabletop music memory game, shaped like a jukebox.
  It can also be played as a simple electronic piano.

***************************************************************************/

class melodym_state : public hh_pic16_state
{
public:
	melodym_state(const machine_config &mconfig, device_type type, const char *tag) :
		hh_pic16_state(mconfig, type, tag)
	{ }

	void melodym(machine_config &config);

private:
	void write_b(u8 data);
	u8 read_c();
	void write_c(u8 data);
};

// handlers

void melodym_state::write_b(u8 data)
{
	// B2-B6: input mux
	m_inp_mux = data >> 2 & 0x1f;
}

u8 melodym_state::read_c()
{
	// C0-C4: multiplexed inputs
	return read_inputs(5, 0x1f) | 0xe0;
}

void melodym_state::write_c(u8 data)
{
	// C6: both lamps
	m_display->matrix(1, ~data >> 6 & 1);

	// C7: speaker out
	m_speaker->level_w(~data >> 7 & 1);
}

// config

static INPUT_PORTS_START( melodym )
	PORT_START("IN.0") // B2 port C
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_1) PORT_NAME("Button 1")
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_2) PORT_NAME("Button 2")
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_3) PORT_NAME("Button 3")
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_4) PORT_NAME("Button 4")
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_5) PORT_NAME("Button 5")

	PORT_START("IN.1") // B3 port C
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_Q) PORT_NAME("Button 6")
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_W) PORT_NAME("Button 7")
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_E) PORT_NAME("Button 8")
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_R) PORT_NAME("Button 9")
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_T) PORT_NAME("Button 10")

	PORT_START("IN.2") // B4 port C
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_A) PORT_NAME("Button 11")
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_S) PORT_NAME("Button 12")
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNUSED ) // there is no button 13
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_F) PORT_NAME("Button 14")
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_G) PORT_NAME("Button 15")

	PORT_START("IN.3") // B5 port C
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_Z) PORT_NAME("Button 16")
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_X) PORT_NAME("Button 17")
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_C) PORT_NAME("Button 18")
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_V) PORT_NAME("Button 19")
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_B) PORT_NAME("Button 20")

	PORT_START("IN.4") // B6 port C
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_N) PORT_NAME("Button 21")
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_M) PORT_NAME("Button 22")
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_COMMA) PORT_NAME("Button 23")
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_STOP) PORT_NAME("Button 24")
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_SLASH) PORT_NAME("Button 25")

	PORT_START("IN.5") // port A
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_7) PORT_NAME("Novice")
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_8) PORT_NAME("Whiz")
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_9) PORT_NAME("Pro")
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_0) PORT_NAME("Note")
INPUT_PORTS_END

void melodym_state::melodym(machine_config &config)
{
	// basic machine hardware
	PIC1655(config, m_maincpu, 1000000); // approximation
	m_maincpu->read_a().set_ioport("IN.5");
	m_maincpu->write_b().set(FUNC(melodym_state::write_b));
	m_maincpu->read_c().set(FUNC(melodym_state::read_c));
	m_maincpu->write_c().set(FUNC(melodym_state::write_c));

	// video hardware
	PWM_DISPLAY(config, m_display).set_size(1, 1);
	m_display->set_bri_levels(0.9);
	config.set_default_layout(layout_melodym);

	// sound hardware
	SPEAKER(config, "mono").front_center();
	SPEAKER_SOUND(config, m_speaker).add_route(ALL_OUTPUTS, "mono", 0.25);
}

// roms

ROM_START( melodym )
	ROM_REGION( 0x0400, "maincpu", 0 )
	ROM_LOAD( "pic_1655a-094", 0x0000, 0x0400, CRC(6d35bd7b) SHA1(20e326085878f69a9d4ef1651ef4443f27188567) )
ROM_END





/***************************************************************************

  Ideal Maniac, by Ralph Baer
  * PIC 1655A-036
  * 2 7seg LEDs, 1-bit sound

  Maniac is a reflex game for 2-4 players. There are 4 challenges:
  1: Musical Maniac: Press the button as soon as the music stops.
  2: Sounds Abound: Count the number of tones in the song, then press the button
     after the same amount of beeps.
  3: Look Twice: Press the button after the game repeats the first pattern.
  4: Your Time Is Up: Press the button after estimating the duration of the tone.

***************************************************************************/

class maniac_state : public hh_pic16_state
{
public:
	maniac_state(const machine_config &mconfig, device_type type, const char *tag) :
		hh_pic16_state(mconfig, type, tag)
	{ }

	void maniac(machine_config &config);

private:
	void update_display();
	void update_speaker();
	void write_b(u8 data);
	void write_c(u8 data);
};

// handlers

void maniac_state::update_display()
{
	m_display->write_row(0, ~m_b & 0x7f);
	m_display->write_row(1, ~m_c & 0x7f);
}

void maniac_state::update_speaker()
{
	m_speaker->level_w((m_b >> 7 & 1) | (m_c >> 6 & 2));
}

void maniac_state::write_b(u8 data)
{
	// B0-B6: left 7seg
	m_b = data;
	update_display();

	// B7: speaker lead 1
	update_speaker();
}

void maniac_state::write_c(u8 data)
{
	// C0-C6: right 7seg
	m_c = data;
	update_display();

	// C7: speaker lead 2
	update_speaker();
}

// config

static INPUT_PORTS_START( maniac )
	PORT_START("IN.0") // port A
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(1) // top button, increment clockwise
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(2)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(3)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(4)
INPUT_PORTS_END

void maniac_state::maniac(machine_config &config)
{
	// basic machine hardware
	PIC1655(config, m_maincpu, 1000000); // approximation - RC osc. R=~13.4K, C=470pF
	m_maincpu->read_a().set_ioport("IN.0");
	m_maincpu->write_b().set(FUNC(maniac_state::write_b));
	m_maincpu->write_c().set(FUNC(maniac_state::write_c));

	// video hardware
	PWM_DISPLAY(config, m_display).set_size(2, 7);
	m_display->set_segmask(3, 0x7f);
	config.set_default_layout(layout_maniac);

	// sound hardware
	SPEAKER(config, "mono").front_center();
	SPEAKER_SOUND(config, m_speaker);
	static const double speaker_levels[] = { 0.0, 1.0, -1.0, 0.0 };
	m_speaker->set_levels(4, speaker_levels);
	m_speaker->add_route(ALL_OUTPUTS, "mono", 0.25);
}

// roms

ROM_START( maniac )
	ROM_REGION( 0x0400, "maincpu", 0 )
	ROM_LOAD( "pic_1655a-036", 0x0000, 0x0400, CRC(a96f7011) SHA1(e97ae44d3c1e74c7e1024bb0bdab03eecdc9f827) )
ROM_END





/***************************************************************************

  Ideal Flash
  * PCB label 25-600321, REV C, TCI-A3H / 94HB
  * PIC 1655A-078
  * 2 7seg LEDs + 8 other LEDs, 1-bit sound with volume decay

  Flash is a wall-mounted game, players throw beanbags to activate the buttons.
  It's described in patent US4333657 as an electronic dart game.

  BTANB: In games 4 and 5 it's easy to lock up the program by pressing the
  buttons repeatedly and causing a score overflow. Although that wouldn't be
  possible by properly throwing beanbags at it. This bug is warned about in
  the manual.

  This could also be purchased as a bare PCB from Radio Shack under the Archer
  brand, catalog number 277-1013. It was named "Sound Effects Chassis" but
  clearly it's nothing like that. The instruction leaflet that came with the
  PCB says to attach a speaker and a 9V power source. It actually takes 5V,
  9V would break it. The only thing it has to say about the game itself is
  "Your module will produce blinking lights and several different sounds."

***************************************************************************/

class flash_state : public hh_pic16_state
{
public:
	flash_state(const machine_config &mconfig, device_type type, const char *tag) :
		hh_pic16_state(mconfig, type, tag)
	{ }

	void flash(machine_config &config);

protected:
	virtual void machine_start() override;

private:
	void update_display();
	void write_b(u8 data);
	u8 read_c();
	void write_c(u8 data);

	void speaker_decay_reset();
	TIMER_DEVICE_CALLBACK_MEMBER(speaker_decay_sim);
	double m_speaker_volume = 0.0;
};

void flash_state::machine_start()
{
	hh_pic16_state::machine_start();
	save_item(NAME(m_speaker_volume));
}

// handlers

void flash_state::speaker_decay_reset()
{
	if (~m_b & 0x40)
		m_speaker_volume = 20.0;

	// it takes a bit before it actually starts fading
	double vol = (m_speaker_volume > 1.0) ? 1.0 : m_speaker_volume;
	m_speaker->set_output_gain(0, vol);
}

TIMER_DEVICE_CALLBACK_MEMBER(flash_state::speaker_decay_sim)
{
	// volume decays when speaker is off (divisor and timer period determine duration)
	speaker_decay_reset();
	m_speaker_volume /= 1.15;
}

void flash_state::update_display()
{
	m_display->matrix(~m_b >> 4 & 3, (~m_c >> 1 & 0x7f) | (~m_b << 7 & 0x780));
}

void flash_state::write_b(u8 data)
{
	// B0-B3: led data
	// B4,B5: led select
	m_b = data;
	update_display();

	// B6: speaker on
	// B7: speaker out
	speaker_decay_reset();
	m_speaker->level_w(data >> 7 & 1);
}

u8 flash_state::read_c()
{
	// C1-C7: buttons
	return (m_c & 1) ? 0xff : m_inputs[1]->read();
}

void flash_state::write_c(u8 data)
{
	// C0: enable buttons
	// C1-C7: digit segments
	m_c = data;
	update_display();
}

// config

static INPUT_PORTS_START( flash )
	PORT_START("IN.0") // port A
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_BUTTON1 ) // top button, increment clockwise
	PORT_BIT( 0x0e, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("IN.1") // port C
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_BUTTON2 )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_BUTTON3 )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_BUTTON4 )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON5 )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON6 )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_BUTTON7 )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_BUTTON8 )
INPUT_PORTS_END

void flash_state::flash(machine_config &config)
{
	// basic machine hardware
	PIC1655(config, m_maincpu, 1050000); // approximation
	m_maincpu->read_a().set_ioport("IN.0");
	m_maincpu->write_b().set(FUNC(flash_state::write_b));
	m_maincpu->read_c().set(FUNC(flash_state::read_c));
	m_maincpu->write_c().set(FUNC(flash_state::write_c));

	// video hardware
	PWM_DISPLAY(config, m_display).set_size(2, 7+4);
	m_display->set_segmask(3, 0x7f);
	config.set_default_layout(layout_flash);

	// sound hardware
	SPEAKER(config, "mono").front_center();
	SPEAKER_SOUND(config, m_speaker).add_route(ALL_OUTPUTS, "mono", 0.25);
	TIMER(config, "speaker_decay").configure_periodic(FUNC(flash_state::speaker_decay_sim), attotime::from_msec(25));
}

// roms

ROM_START( flash )
	ROM_REGION( 0x0400, "maincpu", 0 )
	ROM_LOAD( "pic_1655a-078", 0x0000, 0x0400, CRC(bf780733) SHA1(57ac4620d87492280ab8cf69c148f98e38ecedc4) )
ROM_END





/***************************************************************************

  Kingsford Match Me
  * PIC 1655A-049
  * 8 lamps, 1-bit sound

  Known releases:
  - USA(1): Match Me/Mini Match Me, published by Kingsford
  - USA(2): Me Too, published by Talbot
  - Hong Kong: Gotcha!/Encore/Follow Me, published by Toytronic

  Match Me is the tabletop version, Mini Match Me is the handheld.
  The original is probably by Toytronic, Kingsford's version being licensed from them.

  Known revisions:
  - PIC 1655A-049 (this one, dumped from a Mini Match Me)
  - PIC 1655A-123 (seen in Match Me and Mini Match Me)

***************************************************************************/

class matchme_state : public hh_pic16_state
{
public:
	matchme_state(const machine_config &mconfig, device_type type, const char *tag) :
		hh_pic16_state(mconfig, type, tag)
	{ }

	void matchme(machine_config &config);

	DECLARE_INPUT_CHANGED_MEMBER(speed_switch) { set_clock(); }

protected:
	virtual void machine_reset() override;

private:
	void set_clock();
	void write_b(u8 data);
	void write_c(u8 data);
	u8 read_c();
};

void matchme_state::machine_reset()
{
	hh_pic16_state::machine_reset();
	set_clock();
}

// handlers

void matchme_state::set_clock()
{
	// MCU clock is ~1.2MHz by default (R=18K, C=15pF), high speed setting adds a
	// 10pF cap to speed it up by about 7.5%.
	m_maincpu->set_unscaled_clock((m_inputs[4]->read() & 1) ? 1300000 : 1200000);
}

void matchme_state::write_b(u8 data)
{
	// B0-B7: lamps
	m_display->matrix(1, data);
}

u8 matchme_state::read_c()
{
	// C0-C3: multiplexed inputs from C4-C6
	m_inp_mux = m_c >> 4 & 7;
	u8 lo = read_inputs(3, 0xf);

	// C4-C6: multiplexed inputs from C0-C3
	m_inp_mux = m_c & 0xf;
	u8 hi = read_rotated_inputs(4, 7);

	return lo | hi << 4 | 0x80;
}

void matchme_state::write_c(u8 data)
{
	// C0-C6: input mux
	m_c = data;

	// C7: speaker out + RTCC pin
	m_speaker->level_w(data >> 7 & 1);
	m_maincpu->set_input_line(PIC16C5x_RTCC, data >> 7 & 1);
}

// config

static INPUT_PORTS_START( matchme )
	PORT_START("IN.0") // C4 port C
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_BUTTON1 ) // purple
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_BUTTON2 ) // pink
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_BUTTON3 ) // yellow
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_BUTTON4 ) // blue

	PORT_START("IN.1") // C5 port C
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_BUTTON5 ) // red
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_BUTTON6 ) // cyan
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_BUTTON7 ) // orange
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_BUTTON8 ) // green

	PORT_START("IN.2") // C6 port C
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_START )
	PORT_BIT( 0x02, 0x02, IPT_CUSTOM ) PORT_CONDITION("FAKE", 0x03, EQUALS, 0x03) // Last/Auto
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_BUTTON10 ) PORT_NAME("Long")
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("IN.3") // port A
	PORT_CONFNAME( 0x07, 0x00^0x07, "Game" )
	PORT_CONFSETTING(    0x00^0x07, "1" )
	PORT_CONFSETTING(    0x01^0x07, "2" )
	PORT_CONFSETTING(    0x02^0x07, "3" )
	PORT_CONFSETTING(    0x04^0x07, "4" )
	PORT_CONFNAME( 0x08, 0x08, DEF_STR( Difficulty ) )
	PORT_CONFSETTING(    0x08, "Amateur" ) // AM
	PORT_CONFSETTING(    0x00, "Professional" ) // PRO

	PORT_START("IN.4") // another fake
	PORT_CONFNAME( 0x01, 0x00, "Speed" ) PORT_CHANGED_MEMBER(DEVICE_SELF, matchme_state, speed_switch, 0)
	PORT_CONFSETTING(    0x00, DEF_STR( Low ) )
	PORT_CONFSETTING(    0x01, DEF_STR( High ) )

	PORT_START("FAKE") // Last/Auto are electronically the same button
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_BUTTON9 ) PORT_NAME("Last")
	PORT_CONFNAME( 0x02, 0x02, "Music" )
	PORT_CONFSETTING(    0x02, "Manual" )
	PORT_CONFSETTING(    0x00, "Auto" )
INPUT_PORTS_END

void matchme_state::matchme(machine_config &config)
{
	// basic machine hardware
	PIC1655(config, m_maincpu, 1200000); // see set_clock
	m_maincpu->read_a().set_ioport("IN.3");
	m_maincpu->write_b().set(FUNC(matchme_state::write_b));
	m_maincpu->read_c().set(FUNC(matchme_state::read_c));
	m_maincpu->write_c().set(FUNC(matchme_state::write_c));

	// video hardware
	PWM_DISPLAY(config, m_display).set_size(1, 8);
	config.set_default_layout(layout_matchme);

	// sound hardware
	SPEAKER(config, "mono").front_center();
	SPEAKER_SOUND(config, m_speaker).add_route(ALL_OUTPUTS, "mono", 0.25);
}

// roms

ROM_START( matchme )
	ROM_REGION( 0x0400, "maincpu", 0 )
	ROM_LOAD( "pic_1655a-049", 0x0000, 0x0400, CRC(fa3f4805) SHA1(57cbac18baa201927e99cd69cc2ffda4d2e642bb) )
ROM_END





/***************************************************************************

  Kmart Dr. Dunk (manufactured in Hong Kong)
  * PIC 1655A-51
  * 2 7seg LEDs + 21 other LEDs, 1-bit sound

  It is a clone of Mattel Basketball, but at lower speed.
  The ROM is nearly identical to hccbaskb, the housing/overlay is similar to
  U.S. Games/Tandy Trick Shot Basketball.

  known releases:
  - USA(1): Dr. Dunk, published by Kmart
  - USA(2): Electronic Basketball (model 60-2146), published by Tandy

***************************************************************************/

class drdunk_state : public hh_pic16_state
{
public:
	drdunk_state(const machine_config &mconfig, device_type type, const char *tag) :
		hh_pic16_state(mconfig, type, tag)
	{ }

	void drdunk(machine_config &config);

private:
	void update_display();
	u8 read_a();
	void write_b(u8 data);
	void write_c(u8 data);
};

// handlers

void drdunk_state::update_display()
{
	m_display->matrix(m_b, m_c);
}

u8 drdunk_state::read_a()
{
	// A2: skill switch, A3: multiplexed inputs
	return m_inputs[5]->read() | read_inputs(5, 8) | 3;
}

void drdunk_state::write_b(u8 data)
{
	// B0: RTCC pin
	m_maincpu->set_input_line(PIC16C5x_RTCC, data & 1);

	// B0-B4: input mux
	m_inp_mux = ~data & 0x1f;

	// B0-B3: led select
	// B4,B5: digit select
	m_b = data;
	update_display();
}

void drdunk_state::write_c(u8 data)
{
	// C7: speaker out
	m_speaker->level_w(data >> 7 & 1);

	// C0-C6: led data
	m_c = ~data;
	update_display();
}

// config

static INPUT_PORTS_START( drdunk )
	PORT_START("IN.0") // B0 port A3
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_16WAY

	PORT_START("IN.1") // B1 port A3
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_16WAY

	PORT_START("IN.2") // B2 port A3
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_16WAY

	PORT_START("IN.3") // B3 port A3
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_16WAY

	PORT_START("IN.4") // B4 port A3
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_BUTTON1 )

	PORT_START("IN.5") // port A2
	PORT_CONFNAME( 0x04, 0x04, DEF_STR( Difficulty ) )
	PORT_CONFSETTING(    0x04, "1" )
	PORT_CONFSETTING(    0x00, "2" )
INPUT_PORTS_END

void drdunk_state::drdunk(machine_config &config)
{
	// basic machine hardware
	PIC1655(config, m_maincpu, 800000); // approximation - RC osc. R=18K, C=47pF
	m_maincpu->read_a().set(FUNC(drdunk_state::read_a));
	m_maincpu->write_b().set(FUNC(drdunk_state::write_b));
	m_maincpu->read_c().set_constant(0xff);
	m_maincpu->write_c().set(FUNC(drdunk_state::write_c));

	// video hardware
	PWM_DISPLAY(config, m_display).set_size(6, 7);
	m_display->set_segmask(0x30, 0x7f);
	m_display->set_bri_levels(0.01, 0.2); // player led is brighter
	config.set_default_layout(layout_drdunk);

	// sound hardware
	SPEAKER(config, "mono").front_center();
	SPEAKER_SOUND(config, m_speaker).add_route(ALL_OUTPUTS, "mono", 0.25);
}

// roms

ROM_START( drdunk )
	ROM_REGION( 0x0400, "maincpu", 0 )
	ROM_LOAD( "pic_1655a-051", 0x0000, 0x0400, CRC(92534b40) SHA1(7055e32846c913e68f7d35f279cd537f6325f4f2) )
ROM_END





/***************************************************************************

  Lakeside Le Boom
  * PIC 1655A-061
  * 1 led, 1-bit sound with RC circuit for volume decay

  This is a tabletop timebomb defusion game. It's shaped like an aerial bomb,
  colored black on USA version, yellow on dual-language Canadian version.
  The game starts 'ticking' when the player opens the keypad door. To begin,
  select the game mode, rows(keypad size), and fuse duration.

  Game modes as described on the box:
  1: Eliminate the buttons one by one in the order set out by the computer. Press
     one twice and you'll be sorry!
  2: For 2 or more players. Take turns pressing the buttons, remember which ones.
     Press a button a second time and watch out, it's all over.
  3: The computer picks one secret button that stops the fuse. You must press it
     on your 5th turn. Listen to the clues and you'll do fine.
  4: The computer picks a secret combination. Find it first by listening to the
     clues. Find the right order and you'll get it to fizzle out.

***************************************************************************/

class leboom_state : public hh_pic16_state
{
public:
	leboom_state(const machine_config &mconfig, device_type type, const char *tag) :
		hh_pic16_state(mconfig, type, tag)
	{ }

	void leboom(machine_config &config);

protected:
	virtual void machine_start() override;

private:
	u8 read_a();
	void write_b(u8 data);
	void write_c(u8 data);

	void speaker_decay_reset();
	TIMER_DEVICE_CALLBACK_MEMBER(speaker_decay_sim);
	double m_speaker_volume = 0.0;
};

void leboom_state::machine_start()
{
	hh_pic16_state::machine_start();
	save_item(NAME(m_speaker_volume));
}

// handlers

void leboom_state::speaker_decay_reset()
{
	if (~m_c & 0x80)
		m_speaker_volume = 1.0;

	m_speaker->set_output_gain(0, m_speaker_volume);
}

TIMER_DEVICE_CALLBACK_MEMBER(leboom_state::speaker_decay_sim)
{
	// volume decays when speaker is off (divisor and timer period determine duration)
	speaker_decay_reset();
	m_speaker_volume /= 1.015;
}

u8 leboom_state::read_a()
{
	// A: multiplexed inputs
	return read_inputs(6, 0xf);
}

void leboom_state::write_b(u8 data)
{
	// B0-B5: input mux
	m_inp_mux = data & 0x3f;
}

void leboom_state::write_c(u8 data)
{
	// C4: single led
	m_display->matrix(1, data >> 4 & 1);

	// C7: speaker on
	m_c = data;
	speaker_decay_reset();

	// C6: speaker out
	m_speaker->level_w(data >> 6 & 1);
}

// config

static INPUT_PORTS_START( leboom )
	PORT_START("IN.0") // B0 port A
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_1) PORT_NAME("Red Button 1")
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_Q) PORT_NAME("Red Button 2")
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_A) PORT_NAME("Red Button 3")
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_Z) PORT_NAME("Red Button 4")

	PORT_START("IN.1") // B1 port A
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_2) PORT_NAME("Red-Red Button")
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_W) PORT_NAME("Red-Green Button")
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_S) PORT_NAME("Red-Yellow Button")
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_X) PORT_NAME("Red-Blue Button")

	PORT_START("IN.2") // B2 port A
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_3) PORT_NAME("Shortest")
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_E) PORT_NAME("Short")
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_D) PORT_NAME("Long")
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_C) PORT_NAME("Longest")

	PORT_START("IN.3") // B3 port A
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_4) PORT_NAME("Yellow Button 1")
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_R) PORT_NAME("Yellow Button 2")
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_F) PORT_NAME("Yellow Button 3")
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_V) PORT_NAME("Yellow Button 4")

	PORT_START("IN.4") // B4 port A
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_5) PORT_NAME("Blue Button 1")
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_T) PORT_NAME("Blue Button 2")
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_G) PORT_NAME("Blue Button 3")
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_B) PORT_NAME("Blue Button 4")

	PORT_START("IN.5") // B5 port A
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_6) PORT_NAME("Blue Button 5")
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_Y) PORT_NAME("Blue Button 6")
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_H) PORT_NAME("Blue Button 7")
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_N) PORT_NAME("Blue Button 8")
INPUT_PORTS_END

void leboom_state::leboom(machine_config &config)
{
	// basic machine hardware
	PIC1655(config, m_maincpu, 1000000); // approximation
	m_maincpu->read_a().set(FUNC(leboom_state::read_a));
	m_maincpu->write_b().set(FUNC(leboom_state::write_b));
	m_maincpu->read_c().set_constant(0xff);
	m_maincpu->write_c().set(FUNC(leboom_state::write_c));

	// video hardware
	PWM_DISPLAY(config, m_display).set_size(1, 1);
	config.set_default_layout(layout_leboom);

	// sound hardware
	SPEAKER(config, "mono").front_center();
	SPEAKER_SOUND(config, m_speaker).add_route(ALL_OUTPUTS, "mono", 0.25);
	TIMER(config, "speaker_decay").configure_periodic(FUNC(leboom_state::speaker_decay_sim), attotime::from_msec(25));
}

// roms

ROM_START( leboom )
	ROM_REGION( 0x0400, "maincpu", 0 )
	ROM_LOAD( "pic_1655a-061", 0x0000, 0x0400, CRC(5880eea1) SHA1(e3795b347fd5df9de084da36e33f6b70fbc0b0ae) )
ROM_END





/***************************************************************************

  Tiger Electronics Rocket Pinball (model 7-460)
  * PIC 1650A-110, 69-11397
  * 3 7seg LEDs + 44 other LEDs, 1-bit sound

  known releases:
  - Hong Kong(1): Rocket Pinball, published by Tiger
  - Hong Kong(2): Spaceship Pinball, published by Toytronic
  - USA(1): Rocket Pinball (model 60-2140), published by Tandy
  - USA(2): Cosmic Pinball (model 49-65456), published by Sears

***************************************************************************/

class rockpin_state : public hh_pic16_state
{
public:
	rockpin_state(const machine_config &mconfig, device_type type, const char *tag) :
		hh_pic16_state(mconfig, type, tag)
	{ }

	void rockpin(machine_config &config);

private:
	void update_display();
	void write_a(u8 data);
	void write_b(u8 data);
	void write_c(u8 data);
	void write_d(u8 data);
};

// handlers

void rockpin_state::update_display()
{
	// 3 7seg leds from ports A and B
	m_display->matrix_partial(0, 3, m_a, m_b);

	// 44 leds from ports C and D
	m_display->matrix_partial(3, 6, m_d, m_c);
}

void rockpin_state::write_a(u8 data)
{
	// A3,A4: speaker out
	m_speaker->level_w(data >> 3 & 3);

	// A0-A2: select digit
	m_a = ~data & 7;
	update_display();
}

void rockpin_state::write_b(u8 data)
{
	// B0-B6: digit segments
	m_b = data & 0x7f;
	update_display();
}

void rockpin_state::write_c(u8 data)
{
	// C0-C7: led data
	m_c = ~data;
	update_display();
}

void rockpin_state::write_d(u8 data)
{
	// D0-D5: led select
	m_d = ~data;
	update_display();
}

// config

static INPUT_PORTS_START( rockpin )
	PORT_START("IN.0") // port A
	PORT_BIT( 0x1f, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_NAME("Right Flipper")
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_NAME("Left Flipper")
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_NAME("Ball")
INPUT_PORTS_END

void rockpin_state::rockpin(machine_config &config)
{
	// basic machine hardware
	PIC1650(config, m_maincpu, 450000); // approximation - RC osc. R=47K, C=47pF
	m_maincpu->read_a().set_ioport("IN.0");
	m_maincpu->write_a().set(FUNC(rockpin_state::write_a));
	m_maincpu->read_b().set_constant(0xff);
	m_maincpu->write_b().set(FUNC(rockpin_state::write_b));
	m_maincpu->read_c().set_constant(0xff);
	m_maincpu->write_c().set(FUNC(rockpin_state::write_c));
	m_maincpu->read_d().set_constant(0xff);
	m_maincpu->write_d().set(FUNC(rockpin_state::write_d));

	// PIC CLKOUT, tied to RTCC
	CLOCK(config, "clock", 450000/4).signal_handler().set_inputline(m_maincpu, PIC16C5x_RTCC);

	// video hardware
	PWM_DISPLAY(config, m_display).set_size(3+6, 8);
	m_display->set_segmask(7, 0x7f);
	config.set_default_layout(layout_rockpin);

	// sound hardware
	SPEAKER(config, "mono").front_center();
	SPEAKER_SOUND(config, m_speaker);
	static const double speaker_levels[] = { 0.0, 1.0, -1.0, 0.0 };
	m_speaker->set_levels(4, speaker_levels);
	m_speaker->add_route(ALL_OUTPUTS, "mono", 0.25);
}

// roms

ROM_START( rockpin )
	ROM_REGION( 0x0400, "maincpu", 0 )
	ROM_LOAD( "pic_1650a-110_69-11397", 0x0000, 0x0400, CRC(d5396e77) SHA1(952feaff70fde53a9eda84c54704520d50749e78) )
ROM_END





/***************************************************************************

  Tiger Electronics Half Court Computer Basketball (model 7-470)
  * PIC 1655A(no serial), 69-11557
  * 2 7seg LEDs + 26 other LEDs, 1-bit sound

  known releases:
  - Hong Kong: Half Court Computer Basketball, published by Tiger
  - USA: Electronic Basketball (model 49-65453), published by Sears

***************************************************************************/

class hccbaskb_state : public hh_pic16_state
{
public:
	hccbaskb_state(const machine_config &mconfig, device_type type, const char *tag) :
		hh_pic16_state(mconfig, type, tag)
	{ }

	void hccbaskb(machine_config &config);

private:
	void update_display();
	u8 read_a();
	void write_b(u8 data);
	void write_c(u8 data);
};

// handlers

void hccbaskb_state::update_display()
{
	m_display->matrix(m_b, m_c);
}

u8 hccbaskb_state::read_a()
{
	// A2: skill switch, A3: multiplexed inputs
	return m_inputs[5]->read() | read_inputs(5, 8) | 3;
}

void hccbaskb_state::write_b(u8 data)
{
	// B0: RTCC pin
	m_maincpu->set_input_line(PIC16C5x_RTCC, data & 1);

	// B0-B4: input mux
	m_inp_mux = ~data & 0x1f;

	// B7: speaker out
	m_speaker->level_w(data >> 7 & 1);

	// B0-B4: led select
	// B5,B6: digit select
	m_b = data;
	update_display();
}

void hccbaskb_state::write_c(u8 data)
{
	// C0-C6: led data
	m_c = ~data;
	update_display();
}

// config

static INPUT_PORTS_START( hccbaskb )
	PORT_START("IN.0") // B0 port A3
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_16WAY

	PORT_START("IN.1") // B1 port A3
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_16WAY

	PORT_START("IN.2") // B2 port A3
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_16WAY

	PORT_START("IN.3") // B3 port A3
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_16WAY

	PORT_START("IN.4") // B4 port A3
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_BUTTON1 )

	PORT_START("IN.5") // port A2
	PORT_CONFNAME( 0x04, 0x04, DEF_STR( Difficulty ) )
	PORT_CONFSETTING(    0x04, "1" )
	PORT_CONFSETTING(    0x00, "2" )
INPUT_PORTS_END

void hccbaskb_state::hccbaskb(machine_config &config)
{
	// basic machine hardware
	PIC1655(config, m_maincpu, 800000); // approximation - RC osc. R=15K, C=47pF
	m_maincpu->read_a().set(FUNC(hccbaskb_state::read_a));
	m_maincpu->write_b().set(FUNC(hccbaskb_state::write_b));
	m_maincpu->read_c().set_constant(0xff);
	m_maincpu->write_c().set(FUNC(hccbaskb_state::write_c));

	// video hardware
	PWM_DISPLAY(config, m_display).set_size(7, 7);
	m_display->set_segmask(0x60, 0x7f);
	m_display->set_bri_levels(0.01, 0.2); // player led is brighter
	config.set_default_layout(layout_hccbaskb);

	// sound hardware
	SPEAKER(config, "mono").front_center();
	SPEAKER_SOUND(config, m_speaker).add_route(ALL_OUTPUTS, "mono", 0.25);
}

// roms

ROM_START( hccbaskb )
	ROM_REGION( 0x0400, "maincpu", 0 )
	ROM_LOAD( "69-11557", 0x0000, 0x0400, CRC(56e81079) SHA1(1933f87f82c4c53f953534dba7757c9afc52d5bc) )
ROM_END





/***************************************************************************

  Toytronic Football (set 1)
  * PIC 1655A-033
  * 4511 7seg BCD decoder, 7 7seg LEDs + 27 other LEDs, 1-bit sound

  (no brand) Football (set 2)
  * PIC 1655-024
  * rest same as above, 1 less button

  Hello and welcome to another Mattel Football clone, there are so many of these.
  The 1655-024 one came from an unbranded handheld, but comparison suggests that
  it's the 'prequel' of 1655A-033.

  The 1655-024 version looks and sounds the same as Conic "Electronic Football".

***************************************************************************/

class ttfball_state : public hh_pic16_state
{
public:
	ttfball_state(const machine_config &mconfig, device_type type, const char *tag) :
		hh_pic16_state(mconfig, type, tag)
	{ }

	void ttfball(machine_config &config);

private:
	void update_display();
	u8 read_a();
	void write_b(u8 data);
	void write_c(u8 data);
};

// handlers

void ttfball_state::update_display()
{
	// C0-C2: led data
	// C0-C3: 4511 A-D, C4: digit segment DP
	// C5: select digits or led matrix
	const u8 _4511_map[16] = { 0x3f,0x06,0x5b,0x4f,0x66,0x6d,0x7c,0x07,0x7f,0x67,0,0,0,0,0,0 };
	u16 led_data = (m_c & 0x20) ? (_4511_map[m_c & 0xf] | (~m_c << 3 & 0x80)) : (~m_c << 8 & 0x700);

	m_display->matrix(m_b | (m_c << 1 & 0x100), led_data);
}

u8 ttfball_state::read_a()
{
	// A3: multiplexed inputs, A0-A2: other inputs
	return m_inputs[5]->read() | read_inputs(5, 8);
}

void ttfball_state::write_b(u8 data)
{
	// B0: RTCC pin
	m_maincpu->set_input_line(PIC16C5x_RTCC, data & 1);

	// B0,B1,B3,B7: input mux low
	m_inp_mux = (m_inp_mux & 0x10) | (~data & 3) | (~data >> 1 & 4) | (~data >> 4 & 8);

	// B0-B7: led select (see above)
	m_b = data;
	update_display();
}

void ttfball_state::write_c(u8 data)
{
	// C6: speaker out
	m_speaker->level_w(data >> 6 & 1);

	// C7: input mux high
	m_inp_mux = (m_inp_mux & 0xf) | (data >> 3 & 0x10);

	// C0-C7: led data/select (see above)
	m_c = data;
	update_display();
}

// config

static INPUT_PORTS_START( ttfball )
	PORT_START("IN.0") // B0 port A3
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_16WAY

	PORT_START("IN.1") // B1 port A3
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_16WAY

	PORT_START("IN.2") // B3 port A3
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_16WAY

	PORT_START("IN.3") // B7 port A3
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_16WAY

	PORT_START("IN.4") // C7 port A3
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_NAME("Kick")

	PORT_START("IN.5") // port A
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_START1 ) PORT_NAME("Status")
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_START2 ) PORT_NAME("Score")
	PORT_CONFNAME( 0x04, 0x04, DEF_STR( Difficulty ) )
	PORT_CONFSETTING(    0x04, "1" )
	PORT_CONFSETTING(    0x00, "2" )
INPUT_PORTS_END

static INPUT_PORTS_START( ttfballa )
	PORT_START("IN.0") // B0 port A3
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_NAME("Kick")

	PORT_START("IN.1") // B1 port A3
	PORT_BIT( 0x08, 0x08, IPT_CUSTOM ) PORT_CONDITION("FAKE", 0x03, EQUALS, 0x00) // left/right

	PORT_START("IN.2") // B3 port A3
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_16WAY

	PORT_START("IN.3") // B7 port A3
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_16WAY

	PORT_START("IN.4") // C7 port A3
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("IN.5") // port A
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_START1 ) PORT_NAME("Status")
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_START2 ) PORT_NAME("Score")
	PORT_CONFNAME( 0x04, 0x04, DEF_STR( Difficulty ) )
	PORT_CONFSETTING(    0x04, "1" )
	PORT_CONFSETTING(    0x00, "2" )

	PORT_START("FAKE") // fake port for left/right combination
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT ) PORT_16WAY PORT_NAME("P1 Left/Right")
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT ) PORT_16WAY PORT_NAME("P1 Left/Right")
INPUT_PORTS_END

void ttfball_state::ttfball(machine_config &config)
{
	// basic machine hardware
	PIC1655(config, m_maincpu, 500000); // approximation - RC osc. R=27K(set 1) or 33K(set 2), C=68pF
	m_maincpu->read_a().set(FUNC(ttfball_state::read_a));
	m_maincpu->write_b().set(FUNC(ttfball_state::write_b));
	m_maincpu->read_c().set_constant(0xff);
	m_maincpu->write_c().set(FUNC(ttfball_state::write_c));

	// video hardware
	PWM_DISPLAY(config, m_display).set_size(9, 11);
	m_display->set_segmask(0x7f, 0xff);
	m_display->set_bri_levels(0.002, 0.02); // player led is brighter
	config.set_default_layout(layout_ttfball);

	// sound hardware
	SPEAKER(config, "mono").front_center();
	SPEAKER_SOUND(config, m_speaker).add_route(ALL_OUTPUTS, "mono", 0.25);
}

// roms

ROM_START( ttfball )
	ROM_REGION( 0x0400, "maincpu", 0 )
	ROM_LOAD( "pic_1655a-033", 0x0000, 0x0400, CRC(2b500501) SHA1(f7fe464663c56e2181a31a1dc5f1f5239df57bed) )
ROM_END

ROM_START( ttfballa )
	ROM_REGION( 0x0400, "maincpu", 0 )
	ROM_LOAD( "pic_1655-024", 0x0000, 0x0400, CRC(9091102f) SHA1(ef72759f20b5a99e0366863caad1e26be114263f) )
ROM_END





/***************************************************************************

  U.S. Games Programmable Baseball
  * PIC 1650A-133
  * 3 7seg LEDs + 36 other LEDs, 1-bit sound

  known releases:
  - USA(1): Programmable Baseball, published by U.S. Games
  - USA(2): Electronic 2-Player Baseball (model 60-2157), published by Tandy

***************************************************************************/

class uspbball_state : public hh_pic16_state
{
public:
	uspbball_state(const machine_config &mconfig, device_type type, const char *tag) :
		hh_pic16_state(mconfig, type, tag)
	{ }

	void uspbball(machine_config &config);

private:
	void update_display();
	void write_a(u8 data);
	void write_b(u8 data);
	void write_c(u8 data);
	void write_d(u8 data);
};

// handlers

void uspbball_state::update_display()
{
	m_display->matrix(m_d, m_c << 8 | m_b);
}

void uspbball_state::write_a(u8 data)
{
	// A0: speaker out
	m_speaker->level_w(data & 1);
}

void uspbball_state::write_b(u8 data)
{
	// B: digit segment data
	m_b = bitswap<8>(data,0,1,2,3,4,5,6,7);
	update_display();
}

void uspbball_state::write_c(u8 data)
{
	// C: led data
	m_c = ~data;
	update_display();
}

void uspbball_state::write_d(u8 data)
{
	// D0-D2: digit select
	// D3-D5: led select
	m_d = ~data;
	update_display();
}

// config

static INPUT_PORTS_START( uspbball )
	PORT_START("IN.0") // port A
	PORT_BIT( 0x03, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_COCKTAIL PORT_NAME("P2 Curve Right")
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_COCKTAIL PORT_NAME("P2 Slow")
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_COCKTAIL PORT_NAME("P2 Fast")
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON4 ) PORT_COCKTAIL PORT_NAME("P2 Curve Left")
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_BUTTON5 ) PORT_COCKTAIL PORT_NAME("P2 Change Up/Fielder")
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_NAME("P1 Batter")

	PORT_START("IN.1") // port D
	PORT_BIT( 0x7f, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_CONFNAME( 0x80, 0x80, DEF_STR( Players ) )
	PORT_CONFSETTING(    0x80, "1" )
	PORT_CONFSETTING(    0x00, "2" )
INPUT_PORTS_END

void uspbball_state::uspbball(machine_config &config)
{
	// basic machine hardware
	PIC1650(config, m_maincpu, 900000); // approximation - RC osc. R=22K, C=47pF
	m_maincpu->read_a().set_ioport("IN.0");
	m_maincpu->write_a().set(FUNC(uspbball_state::write_a));
	m_maincpu->read_b().set_constant(0xff);
	m_maincpu->write_b().set(FUNC(uspbball_state::write_b));
	m_maincpu->read_c().set_constant(0xff);
	m_maincpu->write_c().set(FUNC(uspbball_state::write_c));
	m_maincpu->read_d().set_constant(0xff);
	m_maincpu->write_d().set(FUNC(uspbball_state::write_d));

	// PIC CLKOUT, tied to RTCC
	CLOCK(config, "clock", 900000/4).signal_handler().set_inputline("maincpu", PIC16C5x_RTCC);

	// video hardware
	PWM_DISPLAY(config, m_display).set_size(6, 16);
	m_display->set_segmask(7, 0x7f);
	config.set_default_layout(layout_hh_pic16_test);

	// sound hardware
	SPEAKER(config, "mono").front_center();
	SPEAKER_SOUND(config, m_speaker).add_route(ALL_OUTPUTS, "mono", 0.25);
}

// roms

ROM_START( uspbball )
	ROM_REGION( 0x0400, "maincpu", 0 )
	ROM_LOAD( "pic_1650a-133", 0x0000, 0x0400, CRC(479e98be) SHA1(67437177b059dfa6e01940da26daf997cec96ead) )
ROM_END





/***************************************************************************

  U.S. Games Electronic 2-Player Football
  * PIC 1650A-144
  * 8 7seg LEDs + 2 other LEDs, 1-bit sound

  known releases:
  - USA(1): Electronic 2-Player Football, published by U.S. Games
  - USA(2): Electronic 2-Player Football (model 60-2156), published by Tandy

***************************************************************************/

class us2pfball_state : public hh_pic16_state
{
public:
	us2pfball_state(const machine_config &mconfig, device_type type, const char *tag) :
		hh_pic16_state(mconfig, type, tag)
	{ }

	void us2pfball(machine_config &config);

private:
	void update_display();
	u8 read_a();
	void write_a(u8 data);
	void write_b(u8 data);
	void write_c(u8 data);
	void write_d(u8 data);
};

// handlers

void us2pfball_state::update_display()
{
	m_display->matrix(m_d | (m_a << 6 & 0x300), m_c);
}

u8 us2pfball_state::read_a()
{
	// A0,A1: multiplexed inputs, A4-A7: other inputs
	return read_inputs(4, 3) | (m_inputs[4]->read() & 0xf0) | 0x0c;
}

void us2pfball_state::write_a(u8 data)
{
	// A2,A3: leds
	m_a = data;
	update_display();
}

void us2pfball_state::write_b(u8 data)
{
	// B0-B3: input mux
	m_inp_mux = data & 0xf;
}

void us2pfball_state::write_c(u8 data)
{
	// C7: speaker out
	m_speaker->level_w(data >> 7 & 1);

	// C0-C6: digit segments
	m_c = data;
	update_display();
}

void us2pfball_state::write_d(u8 data)
{
	// D0-D7: digit select
	m_d = ~data;
	update_display();
}

// config

static INPUT_PORTS_START( us2pfball )
	PORT_START("IN.0") // B0 port A low
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_16WAY
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_PLAYER(2) PORT_16WAY

	PORT_START("IN.1") // B1 port A low
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_16WAY
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_PLAYER(2) PORT_16WAY

	PORT_START("IN.2") // B2 port A low
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_16WAY
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_PLAYER(2) PORT_16WAY

	PORT_START("IN.3") // B3 port A low
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_16WAY
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_PLAYER(2) PORT_16WAY

	PORT_START("IN.4") // port A high
	PORT_CONFNAME( 0x10, 0x10, DEF_STR( Players ) )
	PORT_CONFSETTING(    0x10, "1" )
	PORT_CONFSETTING(    0x00, "2" )
	PORT_CONFNAME( 0x20, 0x20, DEF_STR( Difficulty ) )
	PORT_CONFSETTING(    0x20, "1" ) // college
	PORT_CONFSETTING(    0x00, "2" ) // pro
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_SELECT ) PORT_TOGGLE PORT_NAME("Play Selector") // pass
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_NAME("Kick/Pass") // K/P

	PORT_START("IN.5") // port B
	PORT_BIT( 0x7f, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_START ) PORT_NAME("Status/Score") // S
INPUT_PORTS_END

void us2pfball_state::us2pfball(machine_config &config)
{
	// basic machine hardware
	PIC1650(config, m_maincpu, 800000); // approximation - RC osc. R=39K, C=75pF
	m_maincpu->read_a().set(FUNC(us2pfball_state::read_a));
	m_maincpu->write_a().set(FUNC(us2pfball_state::write_a));
	m_maincpu->read_b().set_ioport("IN.5");
	m_maincpu->write_b().set(FUNC(us2pfball_state::write_b));
	m_maincpu->read_c().set_constant(0xff);
	m_maincpu->write_c().set(FUNC(us2pfball_state::write_c));
	m_maincpu->read_d().set_constant(0xff);
	m_maincpu->write_d().set(FUNC(us2pfball_state::write_d));

	// PIC CLKOUT, tied to RTCC
	CLOCK(config, "clock", 800000/4).signal_handler().set_inputline("maincpu", PIC16C5x_RTCC);

	// video hardware
	PWM_DISPLAY(config, m_display).set_size(10, 7);
	m_display->set_segmask(0xff, 0x7f);
	config.set_default_layout(layout_us2pfball);

	// sound hardware
	SPEAKER(config, "mono").front_center();
	SPEAKER_SOUND(config, m_speaker).add_route(ALL_OUTPUTS, "mono", 0.25);
}

// roms

ROM_START( us2pfball )
	ROM_REGION( 0x0400, "maincpu", 0 )
	ROM_LOAD( "pic_1650a-144", 0x0000, 0x0400, CRC(ef3677c9) SHA1(33f89c79e7e090710681dffe09eddaf66b5cb794) )
ROM_END



} // anonymous namespace

/***************************************************************************

  Game driver(s)

***************************************************************************/

//    YEAR  NAME       PARENT  CMP MACHINE    INPUT      CLASS            INIT        COMPANY, FULLNAME, FLAGS
CONS( 1979, touchme,   0,       0, touchme,   touchme,   touchme_state,   empty_init, "Atari", "Touch Me (handheld, Rev 2)", MACHINE_SUPPORTS_SAVE | MACHINE_CLICKABLE_ARTWORK )

CONS( 1979, pabball,   0,       0, pabball,   pabball,   pabball_state,   empty_init, "Caprice / Calfax", "Pro-Action Baseball", MACHINE_SUPPORTS_SAVE | MACHINE_NOT_WORKING )

CONS( 1980, melodym,   0,       0, melodym,   melodym,   melodym_state,   empty_init, "GAF", "Melody Madness", MACHINE_SUPPORTS_SAVE | MACHINE_CLICKABLE_ARTWORK )

CONS( 1979, maniac,    0,       0, maniac,    maniac,    maniac_state,    empty_init, "Ideal Toy Corporation", "Maniac", MACHINE_SUPPORTS_SAVE | MACHINE_CLICKABLE_ARTWORK )
CONS( 1980, flash,     0,       0, flash,     flash,     flash_state,     empty_init, "Ideal Toy Corporation", "Flash (Ideal)", MACHINE_SUPPORTS_SAVE | MACHINE_CLICKABLE_ARTWORK )

CONS( 1980, matchme,   0,       0, matchme,   matchme,   matchme_state,   empty_init, "Kingsford", "Match Me", MACHINE_SUPPORTS_SAVE | MACHINE_CLICKABLE_ARTWORK )

CONS( 1979, drdunk,    0,       0, drdunk,    drdunk,    drdunk_state,    empty_init, "Kmart", "Dr. Dunk", MACHINE_SUPPORTS_SAVE )

CONS( 1980, leboom,    0,       0, leboom,    leboom,    leboom_state,    empty_init, "Lakeside", "Le Boom", MACHINE_SUPPORTS_SAVE | MACHINE_CLICKABLE_ARTWORK )

CONS( 1979, rockpin,   0,       0, rockpin,   rockpin,   rockpin_state,   empty_init, "Tiger Electronics", "Rocket Pinball", MACHINE_SUPPORTS_SAVE )
CONS( 1979, hccbaskb,  0,       0, hccbaskb,  hccbaskb,  hccbaskb_state,  empty_init, "Tiger Electronics", "Half Court Computer Basketball", MACHINE_SUPPORTS_SAVE )

CONS( 1979, ttfball,   0,       0, ttfball,   ttfball,   ttfball_state,   empty_init, "Toytronic", "Football (Toytronic, set 1)", MACHINE_SUPPORTS_SAVE | MACHINE_IMPERFECT_SOUND )
CONS( 1979, ttfballa,  ttfball, 0, ttfball,   ttfballa,  ttfball_state,   empty_init, "Toytronic", "Football (Toytronic, set 2)", MACHINE_SUPPORTS_SAVE )

CONS( 1981, uspbball,  0,       0, uspbball,  uspbball,  uspbball_state,  empty_init, "U.S. Games", "Programmable Baseball", MACHINE_SUPPORTS_SAVE | MACHINE_NOT_WORKING )
CONS( 1981, us2pfball, 0,       0, us2pfball, us2pfball, us2pfball_state, empty_init, "U.S. Games", "Electronic 2-Player Football", MACHINE_SUPPORTS_SAVE )
