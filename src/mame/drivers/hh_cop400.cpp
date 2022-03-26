// license:BSD-3-Clause
// copyright-holders:hap
// thanks-to:Sean Riddle
/***************************************************************************

National Semiconductor COPS(COP400 MCU series) handhelds or other simple
devices, mostly LED electronic games/toys.

TODO:
- why does h2hbaskbc(and clones) need a workaround on writing L pins?
- plus1: which sensor position is which colour?
- vidchal: Add screen and gun cursor with brightness detection callback,
  and softwarelist for the video tapes. We'd also need a VHS player device.
  The emulated lightgun itself appears to be working fine(eg. add a 30hz
  timer to IN.3 to score +100)
- solution release year, most chips on the PCB were from 1984, but this one
  was dumped from a licensed product branded for the VWR company, so it
  could be later than the initial release

***************************************************************************/

#include "emu.h"

#include "cpu/cop400/cop400.h"
#include "video/pwm.h"
#include "machine/timer.h"
#include "sound/spkrdev.h"
#include "sound/dac.h"

#include "screen.h"
#include "speaker.h"

// internal artwork
#include "bship82.lh" // clickable
#include "ctstein.lh" // clickable
#include "einvaderc.lh"
#include "funjacks.lh" // clickable
#include "funrlgl.lh" // clickable
#include "funtag.lh" // clickable
#include "h2hbaskbc.lh"
#include "h2hhockeyc.lh"
#include "h2hsoccerc.lh"
#include "lafootb.lh"
#include "lchicken.lh" // clickable
#include "lightfgt.lh" // clickable
#include "mbaskb2.lh"
#include "mdallas.lh"
#include "msoccer2.lh"
#include "qkracer.lh"
#include "scat.lh"
#include "unkeinv.lh"
#include "vidchal.lh"

//#include "hh_cop400_test.lh" // common test-layout - use external artwork


class hh_cop400_state : public driver_device
{
public:
	hh_cop400_state(const machine_config &mconfig, device_type type, const char *tag) :
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
	required_device<cop400_cpu_device> m_maincpu;
	optional_device<pwm_display_device> m_display;
	optional_device<speaker_sound_device> m_speaker;
	optional_ioport_array<6> m_inputs; // max 6

	// misc common
	u8 m_l = 0;                     // MCU port L write data
	u8 m_g = 0;                     // MCU port G write data
	u8 m_d = 0;                     // MCU port D write data
	int m_so = 0;                   // MCU SO line state
	int m_sk = 0;                   // MCU SK line state
	u16 m_inp_mux = ~0;             // multiplexed inputs mask

	u16 read_inputs(int columns, u16 colmask = ~0);
};


// machine start/reset

void hh_cop400_state::machine_start()
{
	// register for savestates
	save_item(NAME(m_l));
	save_item(NAME(m_g));
	save_item(NAME(m_d));
	save_item(NAME(m_so));
	save_item(NAME(m_sk));
	save_item(NAME(m_inp_mux));
}

void hh_cop400_state::machine_reset()
{
}



/***************************************************************************

  Helper Functions

***************************************************************************/

// generic input handlers

u16 hh_cop400_state::read_inputs(int columns, u16 colmask)
{
	// active low
	u16 ret = ~0 & colmask;

	// read selected input rows
	for (int i = 0; i < columns; i++)
		if (~m_inp_mux >> i & 1)
			ret &= m_inputs[i]->read();

	return ret;
}

INPUT_CHANGED_MEMBER(hh_cop400_state::reset_button)
{
	// when an input is directly wired to MCU reset pin
	m_maincpu->set_input_line(INPUT_LINE_RESET, newval ? ASSERT_LINE : CLEAR_LINE);
}



/***************************************************************************

  Minidrivers (subclass, I/O, Inputs, Machine Config, ROM Defs)

***************************************************************************/

namespace {

/***************************************************************************

  Castle Toy Einstein
  * COP421 MCU label ~/927 COP421-NEZ/N
  * 4 lamps, 1-bit sound

  This is a Simon clone, the tones are not harmonic. Two models exist, each
  with a different batteries setup, assume they're same otherwise.

***************************************************************************/

class ctstein_state : public hh_cop400_state
{
public:
	ctstein_state(const machine_config &mconfig, device_type type, const char *tag) :
		hh_cop400_state(mconfig, type, tag)
	{ }

	void ctstein(machine_config &config);

private:
	void write_g(u8 data);
	void write_l(u8 data);
	u8 read_l();
};

// handlers

void ctstein_state::write_g(u8 data)
{
	// G0-G2: input mux
	m_inp_mux = data & 7;
}

void ctstein_state::write_l(u8 data)
{
	// L0-L3: button lamps
	m_display->matrix(1, data & 0xf);
}

u8 ctstein_state::read_l()
{
	// L4-L7: multiplexed inputs
	return read_inputs(3, 0xf) << 4 | 0xf;
}

// config

static INPUT_PORTS_START( ctstein )
	PORT_START("IN.0") // G0 port L
	PORT_CONFNAME( 0x0f, 0x01^0x0f, DEF_STR( Difficulty ) )
	PORT_CONFSETTING(    0x01^0x0f, "1" )
	PORT_CONFSETTING(    0x02^0x0f, "2" )
	PORT_CONFSETTING(    0x04^0x0f, "3" )
	PORT_CONFSETTING(    0x08^0x0f, "4" )

	PORT_START("IN.1") // G1 port L
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_START )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_SELECT ) PORT_NAME("Best Score")
	PORT_BIT( 0x0c, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("IN.2") // G2 port L
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_NAME("Red Button")
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_BUTTON4 ) PORT_NAME("Yellow Button")
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_NAME("Green Button")
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_NAME("Blue Button")
INPUT_PORTS_END

void ctstein_state::ctstein(machine_config &config)
{
	// basic machine hardware
	COP421(config, m_maincpu, 850000); // approximation - RC osc. R=12K, C=100pF
	m_maincpu->set_config(COP400_CKI_DIVISOR_4, COP400_CKO_OSCILLATOR_OUTPUT, false); // guessed
	m_maincpu->write_g().set(FUNC(ctstein_state::write_g));
	m_maincpu->write_l().set(FUNC(ctstein_state::write_l));
	m_maincpu->write_sk().set(m_speaker, FUNC(speaker_sound_device::level_w));
	m_maincpu->read_l().set(FUNC(ctstein_state::read_l));

	// video hardware
	PWM_DISPLAY(config, m_display).set_size(1, 4);
	config.set_default_layout(layout_ctstein);

	// sound hardware
	SPEAKER(config, "mono").front_center();
	SPEAKER_SOUND(config, m_speaker).add_route(ALL_OUTPUTS, "mono", 0.25);
}

// roms

ROM_START( ctstein )
	ROM_REGION( 0x0400, "maincpu", 0 )
	ROM_LOAD( "cop421-nez_n", 0x0000, 0x0400, CRC(16148e03) SHA1(b2b74891d36813d9a1eefd56a925054997c4b7f7) ) // 2nd half empty
ROM_END





/***************************************************************************

  Coleco Head to Head: Electronic Basketball/Hockey/Soccer (model 2150/2160/2170)
  * COP420 MCU label COP420L-NEZ/N
  * 2-digit 7seg display, 41 other leds, 1-bit sound

  3 Head to Head games were released using this MCU/ROM. They play very much
  the same, only differing on game time. The PCB is pre-configured on G1+IN2
  and IN3 to select the game.

  An earlier revision of this runs on TMS1000, see hh_tms1k.cpp driver. Model
  numbers are the same. From the outside, an easy way to spot the difference is
  the Start/Display button: TMS1000 version button label is D, COP420 one is a *.
  The COP420 version also plays much slower.

***************************************************************************/

class h2hbaskbc_state : public hh_cop400_state
{
public:
	h2hbaskbc_state(const machine_config &mconfig, device_type type, const char *tag) :
		hh_cop400_state(mconfig, type, tag)
	{ }

	void h2hsoccerc(machine_config &config);
	void h2hbaskbc(machine_config &config);
	void h2hhockeyc(machine_config &config);

private:
	void write_d(u8 data);
	void write_g(u8 data);
	void write_l(u8 data);
	u8 read_in();
};

// handlers

void h2hbaskbc_state::write_d(u8 data)
{
	// D: led select
	m_d = data & 0xf;
}

void h2hbaskbc_state::write_g(u8 data)
{
	// G: led select, input mux
	m_inp_mux = data;
	m_g = data & 0xf;
}

void h2hbaskbc_state::write_l(u8 data)
{
	// D2,D3 double as multiplexer
	u16 mask = ((m_d >> 2 & 1) * 0x00ff) | ((m_d >> 3 & 1) * 0xff00);
	u16 sel = (m_g | m_d << 4 | m_g << 8 | m_d << 12) & mask;

	// D2+G0,G1 are 7segs
	// L0-L6: digit segments A-G, L0-L4: led data
	m_display->matrix(sel, data);
}

u8 h2hbaskbc_state::read_in()
{
	// IN: multiplexed inputs
	return read_inputs(4, 7) | (m_inputs[4]->read() & 8);
}

// config

static INPUT_PORTS_START( h2hbaskbc )
	PORT_START("IN.0") // G0 port IN
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_16WAY PORT_NAME("P1 Pass CW") // clockwise
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_16WAY PORT_NAME("P1 Pass CCW") // counter-clockwise
	PORT_CONFNAME( 0x04, 0x04, DEF_STR( Players ) )
	PORT_CONFSETTING(    0x04, "1" )
	PORT_CONFSETTING(    0x00, "2" )

	PORT_START("IN.1") // G1 port IN
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_NAME("P1 Shoot")
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_START ) PORT_NAME("Start/Display")
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_CUSTOM ) // factory set

	PORT_START("IN.2") // G2 port IN
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_COCKTAIL PORT_16WAY PORT_NAME("P2 Defense Right")
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_COCKTAIL PORT_16WAY PORT_NAME("P2 Defense Left")
	PORT_CONFNAME( 0x04, 0x04, DEF_STR( Difficulty ) )
	PORT_CONFSETTING(    0x04, "1" )
	PORT_CONFSETTING(    0x00, "2" )

	PORT_START("IN.3") // G3 port IN
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_CONFNAME( 0x04, 0x04, "Factory Test" )
	PORT_CONFSETTING(    0x04, DEF_STR( Off ) )
	PORT_CONFSETTING(    0x00, DEF_STR( On ) )

	PORT_START("IN.4") // IN3 (factory set)
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_CUSTOM )
INPUT_PORTS_END

static INPUT_PORTS_START( h2hhockeyc )
	PORT_INCLUDE( h2hbaskbc )

	PORT_MODIFY("IN.3")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_COCKTAIL PORT_NAME("P2 Goalie Right")
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_COCKTAIL PORT_NAME("P2 Goalie Left")

	PORT_MODIFY("IN.4")
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_CUSTOM )
INPUT_PORTS_END

static INPUT_PORTS_START( h2hsoccerc )
	PORT_INCLUDE( h2hhockeyc )

	PORT_MODIFY("IN.1")
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_CUSTOM )
INPUT_PORTS_END

void h2hbaskbc_state::h2hbaskbc(machine_config &config)
{
	// basic machine hardware
	COP420(config, m_maincpu, 1000000); // approximation - RC osc. R=43K, C=101pF
	m_maincpu->set_config(COP400_CKI_DIVISOR_16, COP400_CKO_OSCILLATOR_OUTPUT, false); // guessed
	m_maincpu->write_d().set(FUNC(h2hbaskbc_state::write_d));
	m_maincpu->write_g().set(FUNC(h2hbaskbc_state::write_g));
	m_maincpu->write_l().set(FUNC(h2hbaskbc_state::write_l));
	m_maincpu->read_in().set(FUNC(h2hbaskbc_state::read_in));
	m_maincpu->write_so().set(m_speaker, FUNC(speaker_sound_device::level_w));

	// video hardware
	PWM_DISPLAY(config, m_display).set_size(16, 7);
	m_display->set_segmask(3, 0x7f);
	config.set_default_layout(layout_h2hbaskbc);

	// sound hardware
	SPEAKER(config, "mono").front_center();
	SPEAKER_SOUND(config, m_speaker).add_route(ALL_OUTPUTS, "mono", 0.25);
}

void h2hbaskbc_state::h2hhockeyc(machine_config &config)
{
	h2hbaskbc(config);
	config.set_default_layout(layout_h2hhockeyc);
}

void h2hbaskbc_state::h2hsoccerc(machine_config &config)
{
	h2hbaskbc(config);
	config.set_default_layout(layout_h2hsoccerc);
}

// roms

ROM_START( h2hbaskbc )
	ROM_REGION( 0x0400, "maincpu", 0 )
	ROM_LOAD( "cop420l-nmy", 0x0000, 0x0400, CRC(87152509) SHA1(acdb869b65d49b3b9855a557ed671cbbb0f61e2c) )
ROM_END

#define rom_h2hhockeyc rom_h2hbaskbc // dumped from Basketball
#define rom_h2hsoccerc rom_h2hbaskbc // "





/***************************************************************************

  Entex Space Invader
  * COP444L MCU label /B138 COPL444-HRZ/N INV II (die label HRZ COP 444L/A)
  * 3 7seg LEDs, LED matrix and overlay mask, 1-bit sound

  The first version was on TMS1100 (see hh_tms1k.c), this is the reprogrammed
  second release with a gray case instead of black.

***************************************************************************/

class einvaderc_state : public hh_cop400_state
{
public:
	einvaderc_state(const machine_config &mconfig, device_type type, const char *tag) :
		hh_cop400_state(mconfig, type, tag)
	{ }

	void einvaderc(machine_config &config);

private:
	void update_display();
	void write_d(u8 data);
	void write_g(u8 data);
	DECLARE_WRITE_LINE_MEMBER(write_sk);
	DECLARE_WRITE_LINE_MEMBER(write_so);
	void write_l(u8 data);
};

// handlers

void einvaderc_state::update_display()
{
	u8 l = bitswap<8>(m_l,7,6,0,1,2,3,4,5);
	u16 grid = (m_d | m_g << 4 | m_sk << 8 | m_so << 9) ^ 0x0ff;

	m_display->matrix(grid, l);
}

void einvaderc_state::write_d(u8 data)
{
	// D: led grid 0-3 (D0-D2 are 7segs)
	m_d = data;
	update_display();
}

void einvaderc_state::write_g(u8 data)
{
	// G: led grid 4-7
	m_g = data;
	update_display();
}

WRITE_LINE_MEMBER(einvaderc_state::write_sk)
{
	// SK: speaker out + led grid 8
	m_speaker->level_w(state);
	m_sk = state;
	update_display();
}

WRITE_LINE_MEMBER(einvaderc_state::write_so)
{
	// SO: led grid 9
	m_so = state;
	update_display();
}

void einvaderc_state::write_l(u8 data)
{
	// L: led state/segment
	m_l = data;
	update_display();
}

// config

static INPUT_PORTS_START( einvaderc )
	PORT_START("IN.0") // port IN
	PORT_CONFNAME( 0x01, 0x01, DEF_STR( Difficulty ) )
	PORT_CONFSETTING(    0x01, "Amateur" )
	PORT_CONFSETTING(    0x00, "Professional" )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_16WAY
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_16WAY
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_BUTTON1 )
INPUT_PORTS_END

void einvaderc_state::einvaderc(machine_config &config)
{
	// basic machine hardware
	COP444L(config, m_maincpu, 850000); // approximation - RC osc. R=47K, C=100pF
	m_maincpu->set_config(COP400_CKI_DIVISOR_16, COP400_CKO_OSCILLATOR_OUTPUT, false); // guessed
	m_maincpu->read_in().set_ioport("IN.0");
	m_maincpu->write_d().set(FUNC(einvaderc_state::write_d));
	m_maincpu->write_g().set(FUNC(einvaderc_state::write_g));
	m_maincpu->write_sk().set(FUNC(einvaderc_state::write_sk));
	m_maincpu->write_so().set(FUNC(einvaderc_state::write_so));
	m_maincpu->write_l().set(FUNC(einvaderc_state::write_l));

	// video hardware
	screen_device &mask(SCREEN(config, "mask", SCREEN_TYPE_SVG));
	mask.set_refresh_hz(60);
	mask.set_size(919, 1080);
	mask.set_visarea_full();

	PWM_DISPLAY(config, m_display).set_size(10, 8);
	m_display->set_segmask(7, 0x7f);
	config.set_default_layout(layout_einvaderc);

	// sound hardware
	SPEAKER(config, "mono").front_center();
	SPEAKER_SOUND(config, m_speaker).add_route(ALL_OUTPUTS, "mono", 0.25);
}

// roms

ROM_START( einvaderc )
	ROM_REGION( 0x0800, "maincpu", 0 )
	ROM_LOAD( "copl444-hrz_n_inv_ii", 0x0000, 0x0800, CRC(76400f38) SHA1(0e92ab0517f7b7687293b189d30d57110df20fe0) )

	ROM_REGION( 82104, "mask", 0)
	ROM_LOAD( "einvaderc.svg", 0, 82104, CRC(0013227f) SHA1(44a3ac48c947369231f010559331ad16fcbef7be) )
ROM_END





/***************************************************************************

  Gordon Barlow Design electronic Space Invaders game (unreleased, from patent US4345764)
  * COP421 (likely a development chip)
  * 36+9 LEDs, 1-bit sound

  This game is presumedly unreleased. The title is unknown, the patent simply names
  it "Hand-held electronic game". There is no mass-manufacture company assigned
  to it either. The game seems unfinished(no scorekeeping, some bugs), and the design
  is very complex. Player ship and bullets are on a moving "wand", a 2-way mirror
  makes it appear on the same plane as the enemies and barriers.

***************************************************************************/

class unkeinv_state : public hh_cop400_state
{
public:
	unkeinv_state(const machine_config &mconfig, device_type type, const char *tag) :
		hh_cop400_state(mconfig, type, tag)
	{ }

	void unkeinv(machine_config &config);

private:
	void update_display();
	void write_g(u8 data);
	void write_d(u8 data);
	void write_l(u8 data);
	u8 read_l();
};

// handlers

void unkeinv_state::update_display()
{
	m_display->matrix(m_g << 4 | m_d, m_l);
}

void unkeinv_state::write_g(u8 data)
{
	// G0,G1: led select part
	// G2,G3: input mux
	m_g = ~data & 0xf;
	update_display();
}

void unkeinv_state::write_d(u8 data)
{
	// D0-D3: led select part
	m_d = ~data & 0xf;
	update_display();
}

void unkeinv_state::write_l(u8 data)
{
	// L0-L7: led data
	m_l = ~data & 0xff;
	update_display();
}

u8 unkeinv_state::read_l()
{
	u8 ret = 0xff;

	// L0-L5+G2: positional odd
	// L0-L5+G3: positional even
	u8 pos = m_inputs[1]->read();
	if (m_g & 4 && pos & 1)
		ret ^= (1 << (pos >> 1));
	if (m_g & 8 && ~pos & 1)
		ret ^= (1 << (pos >> 1));

	// L7+G3: fire button
	if (m_g & 8 && m_inputs[0]->read())
		ret ^= 0x80;

	return ret & ~m_l;
}

// config

static INPUT_PORTS_START( unkeinv )
	PORT_START("IN.0")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_BUTTON1 )

	PORT_START("IN.1")
	PORT_BIT( 0x0f, 0x00, IPT_POSITIONAL ) PORT_POSITIONS(12) PORT_SENSITIVITY(10) PORT_KEYDELTA(1) PORT_CENTERDELTA(0)
INPUT_PORTS_END

void unkeinv_state::unkeinv(machine_config &config)
{
	// basic machine hardware
	COP421(config, m_maincpu, 850000); // frequency guessed
	m_maincpu->set_config(COP400_CKI_DIVISOR_4, COP400_CKO_OSCILLATOR_OUTPUT, false); // guessed
	m_maincpu->write_g().set(FUNC(unkeinv_state::write_g));
	m_maincpu->write_d().set(FUNC(unkeinv_state::write_d));
	m_maincpu->write_l().set(FUNC(unkeinv_state::write_l));
	m_maincpu->read_l().set(FUNC(unkeinv_state::read_l));
	m_maincpu->read_l_tristate().set_constant(0xff);
	m_maincpu->write_so().set(m_speaker, FUNC(speaker_sound_device::level_w));

	// video hardware
	PWM_DISPLAY(config, m_display).set_size(6, 8);
	config.set_default_layout(layout_unkeinv);

	// sound hardware
	SPEAKER(config, "mono").front_center();
	SPEAKER_SOUND(config, m_speaker).add_route(ALL_OUTPUTS, "mono", 0.25);
}

// roms

ROM_START( unkeinv )
	ROM_REGION( 0x0400, "maincpu", 0 )
	ROM_LOAD( "cop421_us4345764", 0x0000, 0x0400, CRC(0068c3a3) SHA1(4e5fd566a5a26c066cc14623a9bd01e109ebf797) ) // typed in from patent US4345764, good print quality
ROM_END





/***************************************************************************

  LJN I Took a Lickin' From a Chicken
  * COP421 MCU label ~/005 COP421-NJC/N
  * 11 leds, 1-bit sound, motor to a chicken on a spring

  This toy includes 4 games: Tic Tac Toe, Chicken Sez, and Total Recall I/II.

  known releases:
  - USA: I Took a Lickin' From a Chicken, published by LJN
  - Japan: Professor Chicken's Genius Classroom 「にわとり博士の天才教室」, published by Bandai
    (not sure if it's the same ROM, or just licensed the outer shell)

***************************************************************************/

class lchicken_state : public hh_cop400_state
{
public:
	lchicken_state(const machine_config &mconfig, device_type type, const char *tag) :
		hh_cop400_state(mconfig, type, tag),
		m_motor_pos_out(*this, "motor_pos"),
		m_motor_on_out(*this, "motor_on")
	{ }

	void lchicken(machine_config &config);

	DECLARE_READ_LINE_MEMBER(motor_switch_r);

protected:
	virtual void machine_start() override;

private:
	output_finder<> m_motor_pos_out;
	output_finder<> m_motor_on_out;

	u8 m_motor_pos = 0;
	TIMER_DEVICE_CALLBACK_MEMBER(motor_sim_tick);

	void write_l(u8 data);
	void write_d(u8 data);
	void write_g(u8 data);
	u8 read_g();
};

void lchicken_state::machine_start()
{
	hh_cop400_state::machine_start();

	m_motor_pos_out.resolve();
	m_motor_on_out.resolve();

	// register for savestates
	save_item(NAME(m_motor_pos));
}

// handlers

READ_LINE_MEMBER(lchicken_state::motor_switch_r)
{
	return m_motor_pos > 0xe8; // approximation
}

TIMER_DEVICE_CALLBACK_MEMBER(lchicken_state::motor_sim_tick)
{
	if (~m_inp_mux & 8)
	{
		m_motor_pos++;
		m_motor_pos_out = 100 * (m_motor_pos / (float)0x100);
	}
}

void lchicken_state::write_l(u8 data)
{
	// L0-L3: led data
	// L4-L6: led select
	// L7: N/C
	m_display->matrix(data >> 4 & 7, ~data & 0xf);
}

void lchicken_state::write_d(u8 data)
{
	// D0-D3: input mux
	// D3: motor on
	m_inp_mux = data & 0xf;
	m_motor_on_out = ~data >> 3 & 1;
}

void lchicken_state::write_g(u8 data)
{
	m_g = data;
}

u8 lchicken_state::read_g()
{
	// G0-G3: multiplexed inputs
	return read_inputs(4, m_g);
}

// config

static INPUT_PORTS_START( lchicken )
	PORT_START("IN.0") // D0 port G
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_3) PORT_CODE(KEYCODE_3_PAD) PORT_NAME("3")
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_6) PORT_CODE(KEYCODE_6_PAD) PORT_NAME("6")
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_9) PORT_CODE(KEYCODE_9_PAD) PORT_NAME("9")
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("IN.1") // D1 port G
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_2) PORT_CODE(KEYCODE_2_PAD) PORT_NAME("2")
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_5) PORT_CODE(KEYCODE_5_PAD) PORT_NAME("5")
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_8) PORT_CODE(KEYCODE_8_PAD) PORT_NAME("8")
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("IN.2") // D2 port G
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_1) PORT_CODE(KEYCODE_1_PAD) PORT_NAME("1")
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_4) PORT_CODE(KEYCODE_4_PAD) PORT_NAME("4")
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_7) PORT_CODE(KEYCODE_7_PAD) PORT_NAME("7")
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("IN.3") // D3 port G
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_CUSTOM ) PORT_READ_LINE_MEMBER(lchicken_state, motor_switch_r)
INPUT_PORTS_END

void lchicken_state::lchicken(machine_config &config)
{
	// basic machine hardware
	COP421(config, m_maincpu, 850000); // approximation - RC osc. R=12K, C=100pF
	m_maincpu->set_config(COP400_CKI_DIVISOR_4, COP400_CKO_OSCILLATOR_OUTPUT, false); // guessed
	m_maincpu->write_l().set(FUNC(lchicken_state::write_l));
	m_maincpu->write_d().set(FUNC(lchicken_state::write_d));
	m_maincpu->write_g().set(FUNC(lchicken_state::write_g));
	m_maincpu->read_g().set(FUNC(lchicken_state::read_g));
	m_maincpu->write_so().set(m_speaker, FUNC(speaker_sound_device::level_w));
	m_maincpu->read_si().set(m_maincpu, FUNC(cop400_cpu_device::so_r));

	TIMER(config, "chicken_motor").configure_periodic(FUNC(lchicken_state::motor_sim_tick), attotime::from_msec(6000/0x100)); // ~6sec for a full rotation

	// video hardware
	PWM_DISPLAY(config, m_display).set_size(3, 4);
	config.set_default_layout(layout_lchicken);

	// sound hardware
	SPEAKER(config, "mono").front_center();
	SPEAKER_SOUND(config, m_speaker).add_route(ALL_OUTPUTS, "mono", 0.25);
}

// roms

ROM_START( lchicken )
	ROM_REGION( 0x0400, "maincpu", 0 )
	ROM_LOAD( "cop421-njc_n", 0x0000, 0x0400, CRC(319e7985) SHA1(9714327518f65ebefe38ac7911bed2b9b9c77307) )
ROM_END





/***************************************************************************

  Mattel Funtronics: Jacks (model 1603)
  * COP410L MCU die bonded directly to PCB (die label COP410L/B NGS)
  * 8 LEDs, 1-bit sound

***************************************************************************/

class funjacks_state : public hh_cop400_state
{
public:
	funjacks_state(const machine_config &mconfig, device_type type, const char *tag) :
		hh_cop400_state(mconfig, type, tag)
	{ }

	void funjacks(machine_config &config);

private:
	void update_display();
	void write_d(u8 data);
	void write_l(u8 data);
	void write_g(u8 data);
	u8 read_l();
	u8 read_g();
};

// handlers

void funjacks_state::update_display()
{
	m_display->matrix(m_d, m_l);
}

void funjacks_state::write_d(u8 data)
{
	// D: led grid + input mux
	m_inp_mux = data;
	m_d = ~data & 0xf;
	update_display();
}

void funjacks_state::write_l(u8 data)
{
	// L0,L1: led state
	m_l = data & 3;
	update_display();
}

void funjacks_state::write_g(u8 data)
{
	// G1: speaker out
	m_speaker->level_w(data >> 1 & 1);
	m_g = data;
}

u8 funjacks_state::read_l()
{
	// L4,L5: multiplexed inputs
	return read_inputs(3, 0x30) | m_l;
}

u8 funjacks_state::read_g()
{
	// G1: speaker out state
	// G2,G3: inputs
	return m_inputs[3]->read() | (m_g & 2);
}

// config

static INPUT_PORTS_START( funjacks )
	PORT_START("IN.0") // D0 port L
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON2 )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON3 )

	PORT_START("IN.1") // D1 port L
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON5 )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON4 )

	PORT_START("IN.2") // D2 port L
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON1 ) // positioned at 1 o'clock on panel, increment clockwise
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON6 )

	PORT_START("IN.3") // port G
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_CUSTOM ) // speaker
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_START )
	PORT_CONFNAME( 0x08, 0x08, DEF_STR( Difficulty ) )
	PORT_CONFSETTING(    0x08, "1" )
	PORT_CONFSETTING(    0x00, "2" )
INPUT_PORTS_END

void funjacks_state::funjacks(machine_config &config)
{
	// basic machine hardware
	COP410(config, m_maincpu, 850000); // approximation - RC osc. R=47K, C=56pF
	m_maincpu->set_config(COP400_CKI_DIVISOR_16, COP400_CKO_OSCILLATOR_OUTPUT, false); // guessed
	m_maincpu->write_d().set(FUNC(funjacks_state::write_d));
	m_maincpu->write_l().set(FUNC(funjacks_state::write_l));
	m_maincpu->write_g().set(FUNC(funjacks_state::write_g));
	m_maincpu->read_l().set(FUNC(funjacks_state::read_l));
	m_maincpu->read_g().set(FUNC(funjacks_state::read_g));

	// video hardware
	PWM_DISPLAY(config, m_display).set_size(4, 2);
	config.set_default_layout(layout_funjacks);

	// sound hardware
	SPEAKER(config, "mono").front_center();
	SPEAKER_SOUND(config, m_speaker).add_route(ALL_OUTPUTS, "mono", 0.25);
}

// roms

ROM_START( funjacks )
	ROM_REGION( 0x0200, "maincpu", 0 )
	ROM_LOAD( "cop410l_b_ngs", 0x0000, 0x0200, CRC(863368ea) SHA1(f116cc27ae721b3a3e178fa13765808bdc275663) )
ROM_END





/***************************************************************************

  Mattel Funtronics: Red Light Green Light (model 1604)
  * COP410L MCU die bonded directly to PCB (die label COP410L/B NHZ)
  * 14 LEDs, 1-bit sound

  known releases:
  - USA: Funtronics: Red Light Green Light, published by Mattel
  - USA(rerelease): Funtronics: Hot Wheels Drag Race, published by Mattel

***************************************************************************/

class funrlgl_state : public hh_cop400_state
{
public:
	funrlgl_state(const machine_config &mconfig, device_type type, const char *tag) :
		hh_cop400_state(mconfig, type, tag)
	{ }

	void funrlgl(machine_config &config);

private:
	void update_display();
	void write_d(u8 data);
	void write_l(u8 data);
	void write_g(u8 data);
};

// handlers

void funrlgl_state::update_display()
{
	m_display->matrix(m_d, m_l);
}

void funrlgl_state::write_d(u8 data)
{
	// D: led grid
	m_d = ~data & 0xf;
	update_display();
}

void funrlgl_state::write_l(u8 data)
{
	// L0-L3: led state
	// L4-L7: N/C
	m_l = ~data & 0xf;
	update_display();
}

void funrlgl_state::write_g(u8 data)
{
	// G3: speaker out
	m_speaker->level_w(data >> 3 & 1);
}

// config

static INPUT_PORTS_START( funrlgl )
	PORT_START("IN.0") // port G
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(1)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(2)
	PORT_CONFNAME( 0x04, 0x04, DEF_STR( Difficulty ) )
	PORT_CONFSETTING(    0x04, "1" )
	PORT_CONFSETTING(    0x00, "2" )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("RESET")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_START ) PORT_CHANGED_MEMBER(DEVICE_SELF, hh_cop400_state, reset_button, 0)
INPUT_PORTS_END

void funrlgl_state::funrlgl(machine_config &config)
{
	// basic machine hardware
	COP410(config, m_maincpu, 800000); // approximation - RC osc. R=51K, C=91pF
	m_maincpu->set_config(COP400_CKI_DIVISOR_16, COP400_CKO_OSCILLATOR_OUTPUT, false); // guessed
	m_maincpu->write_d().set(FUNC(funrlgl_state::write_d));
	m_maincpu->write_l().set(FUNC(funrlgl_state::write_l));
	m_maincpu->read_l_tristate().set_constant(0xff);
	m_maincpu->write_g().set(FUNC(funrlgl_state::write_g));
	m_maincpu->read_g().set_ioport("IN.0");

	// video hardware
	PWM_DISPLAY(config, m_display).set_size(4, 4);
	m_display->set_bri_levels(0.005, 0.1); // top led is brighter
	config.set_default_layout(layout_funrlgl);

	// sound hardware
	SPEAKER(config, "mono").front_center();
	SPEAKER_SOUND(config, m_speaker).add_route(ALL_OUTPUTS, "mono", 0.25);
}

// roms

ROM_START( funrlgl )
	ROM_REGION( 0x0200, "maincpu", 0 )
	ROM_LOAD( "cop410l_b_nhz", 0x0000, 0x0200, CRC(4065c3ce) SHA1(f0bc8125d922949e0d7ab1ba89c805a836d20e09) )
ROM_END





/***************************************************************************

  Mattel Funtronics: Tag (model 1497)
  * COP410L MCU die bonded directly to PCB (die label COP410L/B GTJ)
  * 7 LEDs, 7 buttons, 1-bit sound

***************************************************************************/

class funtag_state : public hh_cop400_state
{
public:
	funtag_state(const machine_config &mconfig, device_type type, const char *tag) :
		hh_cop400_state(mconfig, type, tag)
	{ }

	void funtag(machine_config &config);

private:
	void update_display();
	void write_d(u8 data);
	void write_l(u8 data);
	void write_g(u8 data);
	u8 read_l();
	u8 read_g();
};

// handlers

void funtag_state::update_display()
{
	m_display->matrix(m_d, m_l);
}

void funtag_state::write_d(u8 data)
{
	// D: led grid + input mux
	m_inp_mux = data;
	m_d = ~data & 0xf;
	update_display();
}

void funtag_state::write_l(u8 data)
{
	// L0,L1: led state
	m_l = data & 3;
	update_display();
}

void funtag_state::write_g(u8 data)
{
	// G2: speaker out
	m_speaker->level_w(data >> 2 & 1);
}

u8 funtag_state::read_l()
{
	// L2: difficulty switch
	return m_inputs[4]->read() | 8;
}

u8 funtag_state::read_g()
{
	// G0,G1: multiplexed inputs
	// G3: start button
	return read_inputs(3, 3) | m_inputs[3]->read() | 4;
}

// config

static INPUT_PORTS_START( funtag )
	PORT_START("IN.0") // D0 port G
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_BUTTON1 )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_BUTTON4 )

	PORT_START("IN.1") // D1 port G
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_BUTTON2 )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_BUTTON5 )

	PORT_START("IN.2") // D2 port G
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_BUTTON3 )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_BUTTON6 )

	PORT_START("IN.3") // port G
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_START )

	PORT_START("IN.4") // port L
	PORT_CONFNAME( 0x04, 0x04, DEF_STR( Difficulty ) )
	PORT_CONFSETTING(    0x04, "1" )
	PORT_CONFSETTING(    0x00, "2" )
INPUT_PORTS_END

void funtag_state::funtag(machine_config &config)
{
	// basic machine hardware
	COP410(config, m_maincpu, 1000000); // approximation - RC osc. R=47K, C=91pF
	m_maincpu->set_config(COP400_CKI_DIVISOR_16, COP400_CKO_OSCILLATOR_OUTPUT, false); // guessed
	m_maincpu->write_d().set(FUNC(funtag_state::write_d));
	m_maincpu->write_l().set(FUNC(funtag_state::write_l));
	m_maincpu->write_g().set(FUNC(funtag_state::write_g));
	m_maincpu->read_l().set(FUNC(funtag_state::read_l));
	m_maincpu->read_g().set(FUNC(funtag_state::read_g));

	// video hardware
	PWM_DISPLAY(config, m_display).set_size(4, 2);
	config.set_default_layout(layout_funtag);

	// sound hardware
	SPEAKER(config, "mono").front_center();
	SPEAKER_SOUND(config, m_speaker).add_route(ALL_OUTPUTS, "mono", 0.25);
}

// roms

ROM_START( funtag )
	ROM_REGION( 0x0200, "maincpu", 0 )
	ROM_LOAD( "cop410l_b_gtj", 0x0000, 0x0200, CRC(ce565da6) SHA1(34e5f39e32f220007d353c93787c1a6d117592c1) )
ROM_END





/***************************************************************************

  Mattel Basketball 2 (model 1645), Soccer 2 (model 1642)
  * PCB label: MA6037/38
  * dual COP420L MCUs, dies bonded to PCB (see romdefs for rom serials)
  * 4001, die also bonded to PCB
  * 2-digit 7seg display, 36 other leds, 1-bit sound

  The clock generator was measured ~527kHz for mbaskb2, ~483kHz for msoccer2,
  meaning that the internal divider is 8. Main MCU SK connects to the other
  MCU CKO pin, probably for syncing serial I/O.

  These two are on the same hardware, see patents US4341383 and US4372556
  for detailed descriptions of the games.

***************************************************************************/

class mbaskb2_state : public hh_cop400_state
{
public:
	mbaskb2_state(const machine_config &mconfig, device_type type, const char *tag) :
		hh_cop400_state(mconfig, type, tag),
		m_subcpu(*this, "subcpu"),
		m_on_timer(*this, "on_timer")
	{ }

	void mbaskb2(machine_config &config);
	void msoccer2(machine_config &config);

	DECLARE_CUSTOM_INPUT_MEMBER(switch_r);

protected:
	virtual void machine_reset() override;

private:
	required_device<cop400_cpu_device> m_subcpu;
	required_device<timer_device> m_on_timer;

	void update_display();
	void shared_write_l(u8 data);
	void main_write_g(u8 data);
	void sub_write_g(u8 data);
	void sub_write_d(u8 data);
	u8 sub_read_in();
};

void mbaskb2_state::machine_reset()
{
	hh_cop400_state::machine_reset();
	m_on_timer->adjust(attotime::from_msec(5));
}

// handlers

void mbaskb2_state::update_display()
{
	m_display->matrix(~(m_d << 4 | m_g), m_l);
}

void mbaskb2_state::shared_write_l(u8 data)
{
	// L: led data (though it's unlikely that maincpu will write valid led data to it)
	m_l = m_maincpu->l_r() | m_subcpu->l_r();
	update_display();
}

void mbaskb2_state::main_write_g(u8 data)
{
	// G1: speaker out
	m_speaker->level_w(data >> 1 & 1);
}

void mbaskb2_state::sub_write_g(u8 data)
{
	// G: led select (low), input mux
	m_g = m_inp_mux = data & 0xf;
	update_display();
}

void mbaskb2_state::sub_write_d(u8 data)
{
	// D: led select (high)
	m_d = data & 0xf;
	update_display();
}

u8 mbaskb2_state::sub_read_in()
{
	// IN: multiplexed inputs
	return read_inputs(3, 0xf);
}

// config

CUSTOM_INPUT_MEMBER(mbaskb2_state::switch_r)
{
	// The power switch is off-1-2, and the game relies on power-on starting at 1,
	// otherwise msoccer2 boots up to what looks like a factory test mode.
	return (m_inputs[3]->read() & 1) | (m_on_timer->enabled() ? 1 : 0);
}

static INPUT_PORTS_START( mbaskb2 )
	PORT_START("IN.0") // G0 port IN
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_16WAY
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_16WAY
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_16WAY
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_16WAY

	PORT_START("IN.1") // G1 port IN
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_NAME("Pass")
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_NAME("Shoot")
	PORT_BIT( 0x0c, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("IN.2") // G2 port IN
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(2) PORT_NAME("Defense: Man")
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(2) PORT_NAME("Defense: Zone")
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_PLAYER(2) PORT_NAME("Defense: Press")
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_CUSTOM_MEMBER(mbaskb2_state, switch_r)

	PORT_START("IN.3")
	PORT_CONFNAME( 0x01, 0x01, DEF_STR( Difficulty ) )
	PORT_CONFSETTING(    0x01, "1" )
	PORT_CONFSETTING(    0x00, "2" )
INPUT_PORTS_END

static INPUT_PORTS_START( msoccer2 )
	PORT_INCLUDE( mbaskb2 )

	PORT_MODIFY("IN.2")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(2) PORT_NAME("Low/High Kick")
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(2) PORT_NAME("Score")
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_PLAYER(2) PORT_NAME("Teammate")
INPUT_PORTS_END

void mbaskb2_state::mbaskb2(machine_config &config)
{
	// basic machine hardware
	COP420(config, m_maincpu, 500000); // approximation
	m_maincpu->set_config(COP400_CKI_DIVISOR_8, COP400_CKO_SYNC_INPUT, false); // guessed
	m_maincpu->write_g().set(FUNC(mbaskb2_state::main_write_g));
	m_maincpu->write_l().set(FUNC(mbaskb2_state::shared_write_l));
	m_maincpu->read_l().set(m_subcpu, FUNC(cop400_cpu_device::l_r));
	m_maincpu->read_l_tristate().set_constant(0x80);
	m_maincpu->read_si().set(m_subcpu, FUNC(cop400_cpu_device::so_r));

	COP420(config, m_subcpu, 500000); // same as maincpu
	m_subcpu->set_config(COP400_CKI_DIVISOR_8, COP400_CKO_SYNC_INPUT, false); // guessed
	m_subcpu->write_d().set(FUNC(mbaskb2_state::sub_write_d));
	m_subcpu->write_g().set(FUNC(mbaskb2_state::sub_write_g));
	m_subcpu->write_l().set(FUNC(mbaskb2_state::shared_write_l));
	m_subcpu->read_l().set(m_maincpu, FUNC(cop400_cpu_device::l_r));
	m_subcpu->read_l_tristate().set_constant(0x80);
	m_subcpu->read_in().set(FUNC(mbaskb2_state::sub_read_in));
	m_subcpu->read_si().set(m_maincpu, FUNC(cop400_cpu_device::so_r));
	m_subcpu->read_cko().set(m_maincpu, FUNC(cop400_cpu_device::sk_r));

	config.set_perfect_quantum(m_maincpu);

	TIMER(config, "on_timer").configure_generic(nullptr);

	// video hardware
	PWM_DISPLAY(config, m_display).set_size(8, 7);
	m_display->set_segmask(0xc0, 0x7f);
	m_display->set_bri_levels(0.008, 0.04); // offense is brighter
	config.set_default_layout(layout_mbaskb2);

	// sound hardware
	SPEAKER(config, "mono").front_center();
	SPEAKER_SOUND(config, m_speaker).add_route(ALL_OUTPUTS, "mono", 0.25);
}

void mbaskb2_state::msoccer2(machine_config &config)
{
	mbaskb2(config);
	config.set_default_layout(layout_msoccer2);
}

// roms

ROM_START( mbaskb2 )
	ROM_REGION( 0x0400, "maincpu", 0 )
	ROM_LOAD( "cop420l_nmp", 0x0000, 0x0400, CRC(afc44378) SHA1(e96435bd1d0b2bea5140efdfe21f4684f2525075) )

	ROM_REGION( 0x0400, "subcpu", 0 )
	ROM_LOAD( "cop420l_nmq", 0x0000, 0x0400, CRC(70943f6f) SHA1(3f711d8b7c7c5dd13c68bf1ef980d2f784b748f4) )
ROM_END

ROM_START( msoccer2 )
	ROM_REGION( 0x0400, "maincpu", 0 )
	ROM_LOAD( "cop420l_nnm", 0x0000, 0x0400, CRC(c9169aca) SHA1(525486e8a18ec6132e53f9be582b2667172230a9) )

	ROM_REGION( 0x0400, "subcpu", 0 )
	ROM_LOAD( "cop420l_nnk", 0x0000, 0x0400, CRC(a84dd5f4) SHA1(5d269816248319a2bca1708d5022af455d52682d) )
ROM_END





/***************************************************************************

  Mattel Look Alive! Football (model 1998)
  * COP421L MCU die bonded directly to PCB (rom serial HCJ)
  * 2 7seg LEDs, LED matrix and overlay mask, 1-bit sound

  For a detailed description, see patent US4582323. 1st-person view versions
  for Baseball and Basketball were also announced, but not released.

***************************************************************************/

class lafootb_state : public hh_cop400_state
{
public:
	lafootb_state(const machine_config &mconfig, device_type type, const char *tag) :
		hh_cop400_state(mconfig, type, tag)
	{ }

	void lafootb(machine_config &config);

private:
	void update_display();
	void write_l(u8 data);
	void write_d(u8 data);
	u8 read_g();
};

// handlers

void lafootb_state::update_display()
{
	m_display->matrix(~m_d, m_l);
}

void lafootb_state::write_l(u8 data)
{
	// L: led data
	m_l = data;
	update_display();
}

void lafootb_state::write_d(u8 data)
{
	// D: led select, D2,D3: input mux
	m_d = data & 0xf;
	m_inp_mux = data >> 2 & 3;
	update_display();
}

u8 lafootb_state::read_g()
{
	// G: multiplexed inputs
	return read_inputs(2, 7) | (m_inputs[2]->read() & 8);
}

// config

static INPUT_PORTS_START( lafootb )
	PORT_START("IN.0") // D2 port G
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_16WAY PORT_NAME("Right / Home")
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_NAME("Kick / Yards to go")

	PORT_START("IN.1") // D3 port G
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_16WAY PORT_NAME("Left / Visitors")
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_16WAY PORT_NAME("Up / Time")
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_NAME("Pass / Status")

	PORT_START("IN.2") // G3
	PORT_CONFNAME( 0x08, 0x08, DEF_STR( Difficulty ) )
	PORT_CONFSETTING(    0x08, "1" )
	PORT_CONFSETTING(    0x00, "2" )
INPUT_PORTS_END

void lafootb_state::lafootb(machine_config &config)
{
	// basic machine hardware
	COP421(config, m_maincpu, 900000); // approximation - RC osc. R=51K, C=100pF
	m_maincpu->set_config(COP400_CKI_DIVISOR_16, COP400_CKO_OSCILLATOR_OUTPUT, false); // guessed
	m_maincpu->write_l().set(FUNC(lafootb_state::write_l));
	m_maincpu->write_d().set(FUNC(lafootb_state::write_d));
	m_maincpu->read_g().set(FUNC(lafootb_state::read_g));
	m_maincpu->write_sk().set(m_speaker, FUNC(speaker_sound_device::level_w));

	// video hardware
	screen_device &mask(SCREEN(config, "mask", SCREEN_TYPE_SVG));
	mask.set_refresh_hz(60);
	mask.set_size(1920, 864);
	mask.set_visarea_full();

	PWM_DISPLAY(config, m_display).set_size(4, 8);
	m_display->set_segmask(0x4, 0x7f);
	m_display->set_segmask(0x8, 0xff); // right digit has dp
	m_display->set_bri_levels(0.005);
	config.set_default_layout(layout_lafootb);

	// sound hardware
	SPEAKER(config, "mono").front_center();
	SPEAKER_SOUND(config, m_speaker).add_route(ALL_OUTPUTS, "mono", 0.25);
}

// roms

ROM_START( lafootb )
	ROM_REGION( 0x0400, "maincpu", 0 )
	ROM_LOAD( "cop421l_hcj", 0x0000, 0x0400, CRC(a9cc1e94) SHA1(7a39f5a5f10b8a2bd72da3ff3f3fcfaad35ead5f) )

	ROM_REGION( 38608, "mask", 0)
	ROM_LOAD( "lafootb.svg", 0, 38608, CRC(35387445) SHA1(7cd9db170820fc84d47545c3db8d991b2c5f4f7f) )
ROM_END





/***************************************************************************

  Mattel Dalla$ (J.R. handheld)
  * COP444 MCU label COP444L-HYN/N
  * 8-digit 7seg display, 1-bit sound

  This is a board game, only the handheld device is emulated here.

***************************************************************************/

class mdallas_state : public hh_cop400_state
{
public:
	mdallas_state(const machine_config &mconfig, device_type type, const char *tag) :
		hh_cop400_state(mconfig, type, tag)
	{ }

	void mdallas(machine_config &config);

private:
	void update_display();
	void write_l(u8 data);
	void write_d(u8 data);
	void write_g(u8 data);
	u8 read_in();
};

// handlers

void mdallas_state::update_display()
{
	m_display->matrix(~(m_d << 4 | m_g), m_l);
}

void mdallas_state::write_l(u8 data)
{
	// L: digit segment data
	m_l = data;
	update_display();
}

void mdallas_state::write_d(u8 data)
{
	// D: select digit, input mux high
	m_inp_mux = (m_inp_mux & 0xf) | (data << 4 & 0x30);
	m_d = data & 0xf;
	update_display();
}

void mdallas_state::write_g(u8 data)
{
	// G: select digit, input mux low
	m_inp_mux = (m_inp_mux & 0x30) | (data & 0xf);
	m_g = data & 0xf;
	update_display();
}

u8 mdallas_state::read_in()
{
	// IN: multiplexed inputs
	return read_inputs(6, 0xf);
}

// config

/* physical button layout and labels are like this:

    <  ON>  [YES]   [NO]   [NEXT]
    [<W]    [^N]    [Sv]   [E>]
    [7]     [8]     [9]    [STATUS]
    [4]     [5]     [6]    [ASSETS]
    [1]     [2]     [3]    [START]
    [CLEAR] [0]     [MOVE] [ENTER]
*/

static INPUT_PORTS_START( mdallas )
	PORT_START("IN.0") // G0 port IN
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_C) PORT_CODE(KEYCODE_DEL) PORT_NAME("Clear")
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_0) PORT_CODE(KEYCODE_0_PAD) PORT_NAME("0")
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_M) PORT_NAME("Move")
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_ENTER) PORT_CODE(KEYCODE_ENTER_PAD) PORT_NAME("Enter")

	PORT_START("IN.1") // G1 port IN
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_1) PORT_CODE(KEYCODE_1_PAD) PORT_NAME("1")
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_2) PORT_CODE(KEYCODE_2_PAD) PORT_NAME("2")
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_3) PORT_CODE(KEYCODE_3_PAD) PORT_NAME("3")
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_S) PORT_NAME("Start")

	PORT_START("IN.2") // G2 port IN
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_4) PORT_CODE(KEYCODE_4_PAD) PORT_NAME("4")
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_5) PORT_CODE(KEYCODE_5_PAD) PORT_NAME("5")
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_6) PORT_CODE(KEYCODE_6_PAD) PORT_NAME("6")
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_A) PORT_NAME("Assets")

	PORT_START("IN.3") // G3 port IN
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_7) PORT_CODE(KEYCODE_7_PAD) PORT_NAME("7")
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_8) PORT_CODE(KEYCODE_8_PAD) PORT_NAME("8")
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_9) PORT_CODE(KEYCODE_9_PAD) PORT_NAME("9")
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_T) PORT_NAME("Status")

	PORT_START("IN.4") // D0 port IN
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_Q) PORT_NAME("West") // W
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_X) PORT_NAME("Next")
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_R) PORT_NAME("East") // E

	PORT_START("IN.5") // D1 port IN
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_N) PORT_NAME("No")
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_Y) PORT_NAME("Yes")
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_E) PORT_NAME("South") // S
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_W) PORT_NAME("North") // N
INPUT_PORTS_END

void mdallas_state::mdallas(machine_config &config)
{
	// basic machine hardware
	COP444L(config, m_maincpu, 1000000); // approximation - RC osc. R=57K, C=101pF
	m_maincpu->set_config(COP400_CKI_DIVISOR_16, COP400_CKO_OSCILLATOR_OUTPUT, false); // guessed
	m_maincpu->write_l().set(FUNC(mdallas_state::write_l));
	m_maincpu->write_d().set(FUNC(mdallas_state::write_d));
	m_maincpu->write_g().set(FUNC(mdallas_state::write_g));
	m_maincpu->read_in().set(FUNC(mdallas_state::read_in));
	m_maincpu->write_so().set(m_speaker, FUNC(speaker_sound_device::level_w));

	// video hardware
	PWM_DISPLAY(config, m_display).set_size(8, 8);
	m_display->set_segmask(0xff, 0xff);
	config.set_default_layout(layout_mdallas);

	// sound hardware
	SPEAKER(config, "mono").front_center();
	SPEAKER_SOUND(config, m_speaker).add_route(ALL_OUTPUTS, "mono", 0.25);
}

// roms

ROM_START( mdallas )
	ROM_REGION( 0x0800, "maincpu", 0 )
	ROM_LOAD( "cop444l-hyn_n", 0x0000, 0x0800, CRC(7848b78c) SHA1(778d24512180892f58c49df3c72ca77b2618d63b) )
ROM_END





/***************************************************************************

  Milton Bradley Plus One
  * COP410L MCU in 8-pin DIP, label ~/029 MM 57405 (die label COP410L/B NNE)
  * orientation sensor(4 directions), 1-bit sound

  This is a board game, each player needs to rotate a triangular pyramid
  shaped piece the same as the previous player, plus 1.

***************************************************************************/

class plus1_state : public hh_cop400_state
{
public:
	plus1_state(const machine_config &mconfig, device_type type, const char *tag) :
		hh_cop400_state(mconfig, type, tag)
	{ }

	void plus1(machine_config &config);

private:
	void write_d(u8 data);
	void write_l(u8 data);
	u8 read_l();
};

// handlers

void plus1_state::write_d(u8 data)
{
	// D0?: speaker out
	m_speaker->level_w(data & 1);
}

void plus1_state::write_l(u8 data)
{
	m_l = data;
}

u8 plus1_state::read_l()
{
	// L: IN.1, mask with output
	return m_inputs[1]->read() & m_l;
}

// config

static INPUT_PORTS_START( plus1 )
	PORT_START("IN.0") // port G
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_NAME("Sensor Position 3")
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_NAME("Sensor Position 1")
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("IN.1") // port L
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_BUTTON4 ) PORT_NAME("Sensor Position 4")
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_NAME("Sensor Position 2")
	PORT_BIT( 0xf0, IP_ACTIVE_LOW, IPT_UNUSED )
INPUT_PORTS_END

void plus1_state::plus1(machine_config &config)
{
	// basic machine hardware
	COP410(config, m_maincpu, 1000000); // approximation - RC osc. R=51K, C=100pF
	m_maincpu->set_config(COP400_CKI_DIVISOR_16, COP400_CKO_OSCILLATOR_OUTPUT, false); // guessed
	m_maincpu->write_d().set(FUNC(plus1_state::write_d));
	m_maincpu->read_g().set_ioport("IN.0");
	m_maincpu->write_l().set(FUNC(plus1_state::write_l));
	m_maincpu->read_l().set(FUNC(plus1_state::read_l));

	// no visual feedback!

	// sound hardware
	SPEAKER(config, "mono").front_center();
	SPEAKER_SOUND(config, m_speaker).add_route(ALL_OUTPUTS, "mono", 0.25);
}

// roms

ROM_START( plus1 )
	ROM_REGION( 0x0200, "maincpu", 0 )
	ROM_LOAD( "cop410l_b_nne", 0x0000, 0x0200, CRC(d861b80c) SHA1(4652f8ee0dd4c3c48b625285bb4f094d96434071) )
ROM_END





/***************************************************************************

  Milton Bradley Electronic Lightfight
  * COP421L MCU label /B119 COP421L-HLA/N
  * LED matrix, 1-bit sound

  Xbox-shaped electronic game for 2 or more players, with long diagonal buttons
  next to each outer LED. The main object of the game is to pinpoint a light
  by pressing 2 buttons. To start, press a skill-level button(P2 button 7/8/9)
  after selecting a game mode(P1 button 6-10).

  The game variations are:
  1: LightFight
  2: NightFight
  3: RiteSite
  4: QuiteBrite
  5: RightLight

***************************************************************************/

class lightfgt_state : public hh_cop400_state
{
public:
	lightfgt_state(const machine_config &mconfig, device_type type, const char *tag) :
		hh_cop400_state(mconfig, type, tag)
	{ }

	void lightfgt(machine_config &config);

private:
	void update_display();
	DECLARE_WRITE_LINE_MEMBER(write_so);
	void write_d(u8 data);
	void write_l(u8 data);
	u8 read_g();
};

// handlers

void lightfgt_state::update_display()
{
	u8 grid = (m_so | m_d << 1) ^ 0x1f;
	m_display->matrix(grid, m_l);
}

WRITE_LINE_MEMBER(lightfgt_state::write_so)
{
	// SO: led grid 0 (and input mux)
	m_so = state;
	update_display();
}

void lightfgt_state::write_d(u8 data)
{
	// D: led grid 1-4 (and input mux)
	m_d = data;
	update_display();
}

void lightfgt_state::write_l(u8 data)
{
	// L0-L4: led state
	// L5-L7: N/C
	m_l = data & 0x1f;
	update_display();
}

u8 lightfgt_state::read_g()
{
	// G: multiplexed inputs
	m_inp_mux = m_d << 1 | m_so;
	return read_inputs(5, 0xf);
}

// config

static INPUT_PORTS_START( lightfgt )
	PORT_START("IN.0") // SO port G
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_BUTTON6 )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_BUTTON1 ) // note: button 1 is on the left side from player perspective
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_BUTTON5 ) PORT_COCKTAIL
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_BUTTON10 ) PORT_COCKTAIL

	PORT_START("IN.1") // D0 port G
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_BUTTON7 )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_BUTTON2 )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_BUTTON4 ) PORT_COCKTAIL
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_BUTTON9 ) PORT_COCKTAIL

	PORT_START("IN.2") // D1 port G
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_BUTTON8 )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_BUTTON3 )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_COCKTAIL
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_BUTTON8 ) PORT_COCKTAIL

	PORT_START("IN.3") // D2 port G
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_BUTTON9 )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_BUTTON4 )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_COCKTAIL
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_BUTTON7 ) PORT_COCKTAIL

	PORT_START("IN.4") // D3 port G
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_BUTTON10 )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_BUTTON5 )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_COCKTAIL
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_BUTTON6 ) PORT_COCKTAIL
INPUT_PORTS_END

void lightfgt_state::lightfgt(machine_config &config)
{
	// basic machine hardware
	COP421(config, m_maincpu, 950000); // approximation - RC osc. R=82K, C=56pF
	m_maincpu->set_config(COP400_CKI_DIVISOR_16, COP400_CKO_OSCILLATOR_OUTPUT, false); // guessed
	m_maincpu->write_so().set(FUNC(lightfgt_state::write_so));
	m_maincpu->write_d().set(FUNC(lightfgt_state::write_d));
	m_maincpu->write_l().set(FUNC(lightfgt_state::write_l));
	m_maincpu->write_sk().set(m_speaker, FUNC(speaker_sound_device::level_w));
	m_maincpu->read_g().set(FUNC(lightfgt_state::read_g));

	// video hardware
	PWM_DISPLAY(config, m_display).set_size(5, 5);
	config.set_default_layout(layout_lightfgt);

	// sound hardware
	SPEAKER(config, "mono").front_center();
	SPEAKER_SOUND(config, m_speaker).add_route(ALL_OUTPUTS, "mono", 0.25);
}

// roms

ROM_START( lightfgt )
	ROM_REGION( 0x0400, "maincpu", 0 )
	ROM_LOAD( "cop421l-hla_n", 0x0000, 0x0400, CRC(aceb2d65) SHA1(2328cbb195faf93c575f3afa3a1fe0079180edd7) )
ROM_END





/***************************************************************************

  Milton Bradley Electronic Battleship (1982 version)
  * COP420 MCU label COP420-JWE/N

  see hh_tms1k.cpp bship driver for more information

***************************************************************************/

class bship82_state : public hh_cop400_state
{
public:
	bship82_state(const machine_config &mconfig, device_type type, const char *tag) :
		hh_cop400_state(mconfig, type, tag)
	{ }

	void bship82(machine_config &config);

private:
	void write_d(u8 data);
	void write_g(u8 data);
	u8 read_l();
	u8 read_in();
	DECLARE_WRITE_LINE_MEMBER(write_so);
};

// handlers

void bship82_state::write_d(u8 data)
{
	// D: input mux
	m_inp_mux = data;
}

void bship82_state::write_g(u8 data)
{
	// G0-G2: speaker out via 3.9K, 2.2K, 1.0K resistor
	// G3: enable speaker
	m_speaker->level_w((data & 8) ? (data & 7) : 0);
}

u8 bship82_state::read_l()
{
	// L: multiplexed inputs
	return read_inputs(4, 0xff);
}

u8 bship82_state::read_in()
{
	// IN: multiplexed inputs
	return read_inputs(4, 0xf00) >> 8;
}

WRITE_LINE_MEMBER(bship82_state::write_so)
{
	// SO: led
	m_display->matrix(1, state);
}

// config

static INPUT_PORTS_START( bship82 )
	PORT_START("IN.0") // D0 ports L,IN
	PORT_BIT( 0x001, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_BACKSPACE) PORT_NAME("P1 Clear Last Entry") // CLE
	PORT_BIT( 0x002, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_A) PORT_NAME("P1 A")
	PORT_BIT( 0x004, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_B) PORT_NAME("P1 B")
	PORT_BIT( 0x008, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_C) PORT_NAME("P1 C")
	PORT_BIT( 0x010, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_D) PORT_NAME("P1 D")
	PORT_BIT( 0x020, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_E) PORT_NAME("P1 E")
	PORT_BIT( 0x040, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_F) PORT_NAME("P1 F")
	PORT_BIT( 0x080, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_G) PORT_NAME("P1 G")
	PORT_BIT( 0x100, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_H) PORT_NAME("P1 H")
	PORT_BIT( 0x200, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_I) PORT_NAME("P1 I")
	PORT_BIT( 0x400, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_J) PORT_NAME("P1 J")
	PORT_BIT( 0x800, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("IN.1") // D1 ports L,IN
	PORT_BIT( 0x001, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_DEL) PORT_NAME("P1 Clear Memory") // CM
	PORT_BIT( 0x002, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_1) PORT_CODE(KEYCODE_1_PAD) PORT_NAME("P1 1")
	PORT_BIT( 0x004, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_2) PORT_CODE(KEYCODE_2_PAD) PORT_NAME("P1 2")
	PORT_BIT( 0x008, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_3) PORT_CODE(KEYCODE_3_PAD) PORT_NAME("P1 3")
	PORT_BIT( 0x010, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_4) PORT_CODE(KEYCODE_4_PAD) PORT_NAME("P1 4")
	PORT_BIT( 0x020, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_5) PORT_CODE(KEYCODE_5_PAD) PORT_NAME("P1 5")
	PORT_BIT( 0x040, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_6) PORT_CODE(KEYCODE_6_PAD) PORT_NAME("P1 6")
	PORT_BIT( 0x080, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_7) PORT_CODE(KEYCODE_7_PAD) PORT_NAME("P1 7")
	PORT_BIT( 0x100, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_8) PORT_CODE(KEYCODE_8_PAD) PORT_NAME("P1 8")
	PORT_BIT( 0x200, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_9) PORT_CODE(KEYCODE_9_PAD) PORT_NAME("P1 9")
	PORT_BIT( 0x400, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_0) PORT_CODE(KEYCODE_0_PAD) PORT_NAME("P1 10")
	PORT_BIT( 0x800, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_ENTER) PORT_CODE(KEYCODE_ENTER_PAD) PORT_NAME("P1 Fire")

	PORT_START("IN.2") // D2 ports L,IN
	PORT_BIT( 0x001, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_NAME("P2 Clear Last Entry") // CLE
	PORT_BIT( 0x002, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_NAME("P2 A")
	PORT_BIT( 0x004, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_NAME("P2 B")
	PORT_BIT( 0x008, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_NAME("P2 C")
	PORT_BIT( 0x010, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_NAME("P2 D")
	PORT_BIT( 0x020, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_NAME("P2 E")
	PORT_BIT( 0x040, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_NAME("P2 F")
	PORT_BIT( 0x080, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_NAME("P2 G")
	PORT_BIT( 0x100, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_NAME("P2 H")
	PORT_BIT( 0x200, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_NAME("P2 I")
	PORT_BIT( 0x400, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_NAME("P2 J")
	PORT_BIT( 0x800, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("IN.3") // D3 ports L,IN
	PORT_BIT( 0x001, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_NAME("P2 Clear Memory") // CM
	PORT_BIT( 0x002, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_NAME("P2 1")
	PORT_BIT( 0x004, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_NAME("P2 2")
	PORT_BIT( 0x008, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_NAME("P2 3")
	PORT_BIT( 0x010, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_NAME("P2 4")
	PORT_BIT( 0x020, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_NAME("P2 5")
	PORT_BIT( 0x040, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_NAME("P2 6")
	PORT_BIT( 0x080, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_NAME("P2 7")
	PORT_BIT( 0x100, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_NAME("P2 8")
	PORT_BIT( 0x200, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_NAME("P2 9")
	PORT_BIT( 0x400, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_NAME("P2 10")
	PORT_BIT( 0x800, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_NAME("P2 Fire")

	PORT_START("IN.4") // SI
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_TOGGLE PORT_CODE(KEYCODE_F1) PORT_NAME("Load/Go") // switch
INPUT_PORTS_END

void bship82_state::bship82(machine_config &config)
{
	// basic machine hardware
	COP420(config, m_maincpu, 750000); // approximation - RC osc. R=14K, C=100pF
	m_maincpu->set_config(COP400_CKI_DIVISOR_4, COP400_CKO_OSCILLATOR_OUTPUT, false); // guessed
	m_maincpu->write_d().set(FUNC(bship82_state::write_d));
	m_maincpu->write_g().set(FUNC(bship82_state::write_g));
	m_maincpu->read_l().set(FUNC(bship82_state::read_l));
	m_maincpu->read_in().set(FUNC(bship82_state::read_in));
	m_maincpu->write_so().set(FUNC(bship82_state::write_so));
	m_maincpu->read_si().set_ioport("IN.4");

	// video hardware
	PWM_DISPLAY(config, m_display).set_size(1, 1);
	config.set_default_layout(layout_bship82);

	// sound hardware
	SPEAKER(config, "mono").front_center();
	SPEAKER_SOUND(config, m_speaker);
	static const double speaker_levels[8] = { 0.0, 1.0/7.0, 2.0/7.0, 3.0/7.0, 4.0/7.0, 5.0/7.0, 6.0/7.0, 1.0 };
	m_speaker->set_levels(8, speaker_levels);
	m_speaker->add_route(ALL_OUTPUTS, "mono", 0.25);
}

// roms

ROM_START( bship82 )
	ROM_REGION( 0x0400, "maincpu", 0 )
	ROM_LOAD( "cop420-jwe_n", 0x0000, 0x0400, CRC(5ea8111a) SHA1(34931463b806b48dce4f8ae2361512510bae0ebf) )
ROM_END





/***************************************************************************

  National Semiconductor QuizKid Racer (COP420 version)
  * COP420 MCU label COP420-NPG/N
  * 8-digit 7seg led display(1 custom digit), 1 green led, no sound

  This is the COP420 version, they removed support for the link cable.
  The first release was on a MM5799 MCU, see hh_cops1.cpp.

***************************************************************************/

class qkracer_state : public hh_cop400_state
{
public:
	qkracer_state(const machine_config &mconfig, device_type type, const char *tag) :
		hh_cop400_state(mconfig, type, tag)
	{ }

	void qkracer(machine_config &config);

private:
	void update_display();
	void write_d(u8 data);
	void write_g(u8 data);
	void write_l(u8 data);
	u8 read_in();
	DECLARE_WRITE_LINE_MEMBER(write_sk);
};

// handlers

void qkracer_state::update_display()
{
	m_display->matrix(~(m_d | m_g << 4 | m_sk << 8), m_l);
}

void qkracer_state::write_d(u8 data)
{
	// D: select digit, D3: input mux low bit
	m_inp_mux = (m_inp_mux & ~1) | (data >> 3 & 1);
	m_d = data & 0xf;
	update_display();
}

void qkracer_state::write_g(u8 data)
{
	// G: select digit, input mux
	m_inp_mux = (m_inp_mux & 1) | (data << 1 & 0x1e);
	m_g = data & 0xf;
	update_display();
}

void qkracer_state::write_l(u8 data)
{
	// L0-L6: digit segment data
	m_l = data & 0x7f;
	update_display();
}

u8 qkracer_state::read_in()
{
	// IN: multiplexed inputs
	return read_inputs(5, 0xf);
}

WRITE_LINE_MEMBER(qkracer_state::write_sk)
{
	// SK: green led
	m_sk = state;
	update_display();
}

// config

static INPUT_PORTS_START( qkracer )
	PORT_START("IN.0") // D3 port IN
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_Q) PORT_NAME("Amateur")
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_W) PORT_NAME("Pro")
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_E) PORT_NAME("Complex")
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_R) PORT_NAME("Tables")

	PORT_START("IN.1") // G0 port IN
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_7) PORT_CODE(KEYCODE_7_PAD) PORT_NAME("7")
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_8) PORT_CODE(KEYCODE_8_PAD) PORT_NAME("8")
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_9) PORT_CODE(KEYCODE_9_PAD) PORT_NAME("9")
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_SLASH_PAD) PORT_NAME(u8"÷")

	PORT_START("IN.2") // G1 port IN
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_4) PORT_CODE(KEYCODE_4_PAD) PORT_NAME("4")
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_5) PORT_CODE(KEYCODE_5_PAD) PORT_NAME("5")
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_6) PORT_CODE(KEYCODE_6_PAD) PORT_NAME("6")
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_ASTERISK) PORT_NAME(u8"×")

	PORT_START("IN.3") // G2 port IN
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_1) PORT_CODE(KEYCODE_1_PAD) PORT_NAME("1")
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_2) PORT_CODE(KEYCODE_2_PAD) PORT_NAME("2")
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_3) PORT_CODE(KEYCODE_3_PAD) PORT_NAME("3")
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_MINUS_PAD) PORT_NAME("-")

	PORT_START("IN.4") // G3 port IN
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_MINUS) PORT_NAME("Slow")
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_EQUALS) PORT_NAME("Fast")
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_0) PORT_CODE(KEYCODE_0_PAD) PORT_NAME("0")
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_PLUS_PAD) PORT_NAME("+")
INPUT_PORTS_END

void qkracer_state::qkracer(machine_config &config)
{
	// basic machine hardware
	COP420(config, m_maincpu, 950000); // approximation - RC osc. R=47K, C=100pF
	m_maincpu->set_config(COP400_CKI_DIVISOR_16, COP400_CKO_OSCILLATOR_OUTPUT, false); // guessed
	m_maincpu->write_d().set(FUNC(qkracer_state::write_d));
	m_maincpu->write_g().set(FUNC(qkracer_state::write_g));
	m_maincpu->write_l().set(FUNC(qkracer_state::write_l));
	m_maincpu->read_in().set(FUNC(qkracer_state::read_in));
	m_maincpu->write_sk().set(FUNC(qkracer_state::write_sk));

	// video hardware
	PWM_DISPLAY(config, m_display).set_size(9, 7);
	m_display->set_segmask(0xdf, 0x7f);
	m_display->set_segmask(0x20, 0x41); // equals sign
	config.set_default_layout(layout_qkracer);

	// no sound!
}

// roms

ROM_START( qkracer )
	ROM_REGION( 0x0400, "maincpu", 0 )
	ROM_LOAD( "cop420-npg_n", 0x0000, 0x0400, CRC(17f8e538) SHA1(23d1a1819e6ba552d8da83da2948af1cf5b13d5b) )
ROM_END





/***************************************************************************

  SCAT specialist calculators
  * COP404LSN-5 MCU (no internal ROM)
  * 2KB EPROM (ETC2716Q)
  * 8-digit 7seg led display

  SCAT = aka South Carolina(SC) Applied Technology, Inc.

  Known products, assumed to be all on the same hardware:
  - The Dimension (aka Feet & Inch Calculator)
  - The Solution
  - Metalmate

  CKI was measured ~1.469MHz, but D0 was measured ~77.44Hz so that means real
  clock speed is a bit higher than CKI measurement, and the clock divider is 32.

***************************************************************************/

class scat_state : public hh_cop400_state
{
public:
	scat_state(const machine_config &mconfig, device_type type, const char *tag) :
		hh_cop400_state(mconfig, type, tag)
	{ }

	void scat(machine_config &config);

private:
	void main_map(address_map &map);

	void update_display();
	void write_d(u8 data);
	void write_g(u8 data);
	void write_l(u8 data);
	u8 read_in();
};

// handlers

void scat_state::update_display()
{
	m_display->matrix(~(m_d | m_g << 4), bitswap<8>(m_l,0,1,2,3,4,5,6,7));
}

void scat_state::write_d(u8 data)
{
	// D: select digit, input mux (low)
	m_inp_mux = (m_inp_mux & 0x30) | (data & 0xf);
	m_d = data & 0xf;
	update_display();
}

void scat_state::write_g(u8 data)
{
	// G: select digit, input mux (high)
	m_inp_mux = (m_inp_mux & 0xf) | (data << 4 & 0x30);
	m_g = data & 0xf;
	update_display();
}

void scat_state::write_l(u8 data)
{
	// L: digit segment data
	m_l = data;
	update_display();
}

u8 scat_state::read_in()
{
	// IN: multiplexed inputs
	return read_inputs(6, 0xf);
}

void scat_state::main_map(address_map &map)
{
	map(0x0000, 0x07ff).rom();
}

// config

static INPUT_PORTS_START( solution )
	PORT_START("IN.0") // D0 port IN
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_0) PORT_CODE(KEYCODE_0_PAD) PORT_NAME("0")
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_STOP) PORT_CODE(KEYCODE_DEL_PAD) PORT_NAME(".")
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_ENTER) PORT_CODE(KEYCODE_ENTER_PAD) PORT_NAME("= / Enter")
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_PLUS_PAD) PORT_NAME("+ / ppm")

	PORT_START("IN.1") // D1 port IN
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_1) PORT_CODE(KEYCODE_1_PAD) PORT_NAME("1")
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_2) PORT_CODE(KEYCODE_2_PAD) PORT_NAME("2 / Atoms / Mole")
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_3) PORT_CODE(KEYCODE_3_PAD) PORT_NAME("3 / Atomic Wt.")
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_MINUS_PAD) PORT_NAME("- / %")

	PORT_START("IN.2") // D2 port IN
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_4) PORT_CODE(KEYCODE_4_PAD) PORT_NAME("4 / Density Wntd.")
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_5) PORT_CODE(KEYCODE_5_PAD) PORT_NAME("5 / Orig. Density")
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_6) PORT_CODE(KEYCODE_6_PAD) PORT_NAME("6 / Eq. Wt.")
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_ASTERISK) PORT_NAME(u8"× / Normal")

	PORT_START("IN.3") // D3 port IN
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_7) PORT_CODE(KEYCODE_7_PAD) PORT_NAME("7 / Conc. Wntd.")
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_8) PORT_CODE(KEYCODE_8_PAD) PORT_NAME("8 / Orig. Conc.")
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_9) PORT_CODE(KEYCODE_9_PAD) PORT_NAME("9 / Fmla. Wt.")
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_SLASH_PAD) PORT_NAME(u8"÷ / Molar")

	PORT_START("IN.4") // G0 port IN
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_MINUS) PORT_NAME("Milli.")
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_EQUALS) PORT_NAME("Micro.")
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_G) PORT_NAME("Gram")
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_L) PORT_NAME("Liter")

	PORT_START("IN.5") // G1 port IN
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_V) PORT_NAME("Known Vol.")
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_W) PORT_NAME("Known Wt.")
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_D) PORT_NAME("Dil. Wntd.")
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_BACKSPACE) PORT_CODE(KEYCODE_DEL) PORT_NAME("CE/C")
INPUT_PORTS_END

void scat_state::scat(machine_config &config)
{
	// basic machine hardware
	COP404L(config, m_maincpu, 1500000); // R/C OSC via MM74C14N
	m_maincpu->set_config(COP400_CKI_DIVISOR_32, COP400_CKO_OSCILLATOR_OUTPUT, false); // guessed
	m_maincpu->set_addrmap(AS_PROGRAM, &scat_state::main_map);
	m_maincpu->write_d().set(FUNC(scat_state::write_d));
	m_maincpu->write_g().set(FUNC(scat_state::write_g));
	m_maincpu->write_l().set(FUNC(scat_state::write_l));
	m_maincpu->read_in().set(FUNC(scat_state::read_in));

	// video hardware
	PWM_DISPLAY(config, m_display).set_size(8, 8);
	m_display->set_segmask(0xff, 0xff);
	config.set_default_layout(layout_scat);

	// no sound!
}

// roms

ROM_START( solution )
	ROM_REGION( 0x0800, "maincpu", 0 )
	ROM_LOAD( "etc2716q", 0x0000, 0x0800, CRC(cf990f88) SHA1(6d505fdc94028cbdf6445df9e9451156a9d5f372) ) // no custom label
ROM_END





/***************************************************************************

  Select Merchandise Video Challenger
  * COP420 MCU label COP420-TDX/N
  * 6-digit 7seg led display, 3 other leds, 4-bit sound

  This is a lightgun with scorekeeping. The "games" themselves were released
  on VHS tapes. To determine scoring, the lightgun detects strobe lighting
  from objects in the video.

  known releases:
  - Japan: Video Challenger, published by Takara
  - UK: Video Challenger, published by Bandai
  - Canada: Video Challenger, published by Irwin

***************************************************************************/

class vidchal_state : public hh_cop400_state
{
public:
	vidchal_state(const machine_config &mconfig, device_type type, const char *tag) :
		hh_cop400_state(mconfig, type, tag)
	{ }

	void vidchal(machine_config &config);

private:
	void update_display();
	void write_d(u8 data);
	void write_l(u8 data);
	DECLARE_WRITE_LINE_MEMBER(write_sk);
};

// handlers

void vidchal_state::update_display()
{
	m_display->matrix(m_d | m_sk << 6, m_l);
}

void vidchal_state::write_d(u8 data)
{
	// D: CD4028BE to digit select
	m_d = 1 << data & 0x3f;
	update_display();
}

void vidchal_state::write_l(u8 data)
{
	// L: digit segment data
	m_l = bitswap<8>(data,0,3,1,5,4,7,2,6);
	update_display();
}

WRITE_LINE_MEMBER(vidchal_state::write_sk)
{
	// SK: hit led
	m_sk = state;
	update_display();
}

// config

static INPUT_PORTS_START( vidchal )
	PORT_START("IN.0") // port IN
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_BUTTON1 )
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_OTHER) PORT_CODE(KEYCODE_F1) PORT_NAME("Light Sensor")
INPUT_PORTS_END

void vidchal_state::vidchal(machine_config &config)
{
	// basic machine hardware
	COP420(config, m_maincpu, 900000); // approximation
	m_maincpu->set_config(COP400_CKI_DIVISOR_4, COP400_CKO_OSCILLATOR_OUTPUT, false); // guessed
	m_maincpu->write_d().set(FUNC(vidchal_state::write_d));
	m_maincpu->write_g().set("dac", FUNC(dac_byte_interface::data_w));
	m_maincpu->write_l().set(FUNC(vidchal_state::write_l));
	m_maincpu->read_in().set_ioport("IN.0");
	m_maincpu->write_sk().set(FUNC(vidchal_state::write_sk));

	// video hardware
	PWM_DISPLAY(config, m_display).set_size(6+1, 8);
	m_display->set_segmask(0x3f, 0xff);
	config.set_default_layout(layout_vidchal);

	// sound hardware
	SPEAKER(config, "mono").front_center();
	DAC_4BIT_BINARY_WEIGHTED_SIGN_MAGNITUDE(config, "dac").add_route(ALL_OUTPUTS, "mono", 0.125); // unknown DAC
}

// roms

ROM_START( vidchal )
	ROM_REGION( 0x0400, "maincpu", 0 )
	ROM_LOAD( "cop420-tdx_n", 0x0000, 0x0400, CRC(c9bd041c) SHA1(ab0dcaf4741620fa4c28ab75337a23d646af7626) )
ROM_END



} // anonymous namespace

/***************************************************************************

  Game driver(s)

***************************************************************************/

//    YEAR  NAME        PARENT    CMP MACHINE     INPUT       CLASS            INIT        COMPANY, FULLNAME, FLAGS
CONS( 1979, ctstein,    0,         0, ctstein,    ctstein,    ctstein_state,   empty_init, "Castle Toy", "Einstein (Castle Toy)", MACHINE_SUPPORTS_SAVE | MACHINE_CLICKABLE_ARTWORK )

CONS( 1980, h2hbaskbc,  h2hbaskb,  0, h2hbaskbc,  h2hbaskbc,  h2hbaskbc_state, empty_init, "Coleco", "Head to Head: Electronic Basketball (COP420L version)", MACHINE_SUPPORTS_SAVE )
CONS( 1980, h2hhockeyc, h2hhockey, 0, h2hhockeyc, h2hhockeyc, h2hbaskbc_state, empty_init, "Coleco", "Head to Head: Electronic Hockey (COP420L version)", MACHINE_SUPPORTS_SAVE )
CONS( 1980, h2hsoccerc, 0,         0, h2hsoccerc, h2hsoccerc, h2hbaskbc_state, empty_init, "Coleco", "Head to Head: Electronic Soccer (COP420L version)", MACHINE_SUPPORTS_SAVE )

CONS( 1981, einvaderc,  einvader,  0, einvaderc,  einvaderc,  einvaderc_state, empty_init, "Entex", "Space Invader (Entex, COP444L version)", MACHINE_SUPPORTS_SAVE )

CONS( 1980, unkeinv,    0,         0, unkeinv,    unkeinv,    unkeinv_state,   empty_init, "Gordon Barlow Design", "unknown electronic Space Invaders game (patent)", MACHINE_SUPPORTS_SAVE )

CONS( 1980, lchicken,   0,         0, lchicken,   lchicken,   lchicken_state,  empty_init, "LJN", "I Took a Lickin' From a Chicken", MACHINE_SUPPORTS_SAVE | MACHINE_CLICKABLE_ARTWORK | MACHINE_MECHANICAL )

CONS( 1979, funjacks,   0,         0, funjacks,   funjacks,   funjacks_state,  empty_init, "Mattel Electronics", "Funtronics: Jacks", MACHINE_SUPPORTS_SAVE | MACHINE_CLICKABLE_ARTWORK )
CONS( 1979, funrlgl,    0,         0, funrlgl,    funrlgl,    funrlgl_state,   empty_init, "Mattel Electronics", "Funtronics: Red Light Green Light", MACHINE_SUPPORTS_SAVE | MACHINE_CLICKABLE_ARTWORK )
CONS( 1980, funtag,     0,         0, funtag,     funtag,     funtag_state,    empty_init, "Mattel Electronics", "Funtronics: Tag", MACHINE_SUPPORTS_SAVE | MACHINE_CLICKABLE_ARTWORK )
CONS( 1979, mbaskb2,    0,         0, mbaskb2,    mbaskb2,    mbaskb2_state,   empty_init, "Mattel Electronics", "Basketball 2 (Mattel)", MACHINE_SUPPORTS_SAVE )
CONS( 1979, msoccer2,   0,         0, msoccer2,   msoccer2,   mbaskb2_state,   empty_init, "Mattel Electronics", "Soccer 2 (Mattel)", MACHINE_SUPPORTS_SAVE )
CONS( 1980, lafootb,    0,         0, lafootb,    lafootb,    lafootb_state,   empty_init, "Mattel Electronics", "Look Alive! Football", MACHINE_SUPPORTS_SAVE )
CONS( 1981, mdallas,    0,         0, mdallas,    mdallas,    mdallas_state,   empty_init, "Mattel Electronics", "Dalla$ (J.R. handheld)", MACHINE_SUPPORTS_SAVE ) // ***

CONS( 1980, plus1,      0,         0, plus1,      plus1,      plus1_state,     empty_init, "Milton Bradley", "Plus One", MACHINE_SUPPORTS_SAVE | MACHINE_IMPERFECT_CONTROLS ) // ***
CONS( 1981, lightfgt,   0,         0, lightfgt,   lightfgt,   lightfgt_state,  empty_init, "Milton Bradley", "Electronic Lightfight - The Games of Dueling Lights", MACHINE_SUPPORTS_SAVE | MACHINE_CLICKABLE_ARTWORK )
CONS( 1982, bship82,    bship,     0, bship82,    bship82,    bship82_state,   empty_init, "Milton Bradley", "Electronic Battleship (1982 version)", MACHINE_SUPPORTS_SAVE | MACHINE_CLICKABLE_ARTWORK ) // ***

CONS( 1979, qkracer,    0,         0, qkracer,    qkracer,    qkracer_state,   empty_init, "National Semiconductor", "QuizKid Racer (COP420 version)", MACHINE_SUPPORTS_SAVE | MACHINE_NO_SOUND_HW )

COMP( 1984, solution,   0,         0, scat,       solution,   scat_state,      empty_init, "SCAT", "The Solution", MACHINE_SUPPORTS_SAVE | MACHINE_NO_SOUND_HW )

CONS( 1987, vidchal,    0,         0, vidchal,    vidchal,    vidchal_state,   empty_init, "Select Merchandise", "Video Challenger", MACHINE_SUPPORTS_SAVE | MACHINE_NOT_WORKING )

// ***: As far as MAME is concerned, the game is emulated fine. But for it to be playable, it requires interaction
// with other, unemulatable, things eg. game board/pieces, book, playing cards, pen & paper, etc.
