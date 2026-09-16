// license:BSD-3-Clause
// copyright-holders:hap
// thanks-to:Sean Riddle
/*******************************************************************************

Matsushita (Panasonic) MN1400 handhelds. Matsushita used this MCU in their
audio/video equipment, and it's used in some handheld toys too.

*******************************************************************************/

#include "emu.h"

#include "cpu/mn1400/mn1400.h"
#include "sound/spkrdev.h"
#include "video/pwm.h"

#include "screen.h"
#include "speaker.h"

// internal artwork
#include "compperf.lh"
#include "scrablexa.lh"
#include "tmbaskb.lh"

//#include "hh_mn1400_test.lh" // common test-layout - use external artwork


namespace {

class hh_mn1400_state : public driver_device
{
public:
	hh_mn1400_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_display(*this, "display"),
		m_speaker(*this, "speaker"),
		m_inputs(*this, "IN.%u", 0)
	{ }

protected:
	virtual void machine_start() override ATTR_COLD;
	virtual void machine_reset() override ATTR_COLD;

	// devices
	required_device<mn1400_base_device> m_maincpu;
	optional_device<pwm_display_device> m_display;
	optional_device<speaker_sound_device> m_speaker;
	optional_ioport_array<5> m_inputs; // max 5

	u16 m_inp_mux = 0; // multiplexed inputs mask

	// MCU output pins state
	u16 m_c = 0;       // C pins
	u8 m_d = 0;        // D pins
	u8 m_e = 0;        // E pins

	u16 read_inputs(int columns);
};


// machine start/reset

void hh_mn1400_state::machine_start()
{
	// register for savestates
	save_item(NAME(m_inp_mux));
	save_item(NAME(m_c));
	save_item(NAME(m_d));
	save_item(NAME(m_e));
}

void hh_mn1400_state::machine_reset()
{
}



/*******************************************************************************

  Helper Functions

*******************************************************************************/

// generic input handlers

u16 hh_mn1400_state::read_inputs(int columns)
{
	u16 ret = 0;

	// read selected input rows
	for (int i = 0; i < columns; i++)
		if (BIT(m_inp_mux, i))
			ret |= m_inputs[i]->read();

	return ret;
}



/*******************************************************************************

  Minidrivers (subclass, I/O, Inputs, Machine Config, ROM Defs)

*******************************************************************************/

/*******************************************************************************

  Lakeside Computer Perfection
  * PCB label: Lakeside, PANASONIC, TCI-A4H94HB
  * MN1400ML (28 pins, die label: 1400 ML-0)
  * 10 LEDs, 2-bit sound

  known releases:
  - USA: Computer Perfection, published by Lakeside
  - UK: Computer Perfection, published by Action GT

*******************************************************************************/

class compperf_state : public hh_mn1400_state
{
public:
	compperf_state(const machine_config &mconfig, device_type type, const char *tag) :
		hh_mn1400_state(mconfig, type, tag)
	{ }

	void compperf(machine_config &config);

private:
	void update_display();
	void write_c(u16 data);
	void write_d(u8 data);
	void write_e(u8 data);
	u8 read_sns();
};

// handlers

void compperf_state::update_display()
{
	m_display->matrix(1, ~m_inp_mux);
}

void compperf_state::write_c(u16 data)
{
	// CO5-CO9: leds/input mux part
	m_inp_mux = (m_inp_mux & ~0x3e0) | (data & 0x3e0);
	update_display();
}

void compperf_state::write_d(u8 data)
{
	// DO0-DO3: leds/input mux part
	m_inp_mux = (m_inp_mux & ~0x1e) | (data << 1 & 0x1e);
	update_display();
}

void compperf_state::write_e(u8 data)
{
	// EO0: leds/input mux part
	m_inp_mux = (m_inp_mux & ~1) | (data & 1);
	update_display();

	// EO2,EO3: speaker out
	m_speaker->level_w(data >> 2 & 3);
}

u8 compperf_state::read_sns()
{
	// SNS0: multiplexed inputs, SNS1: set button
	u8 sns0 = (m_inputs[2]->read() & m_inp_mux) ? 1 : 0;
	return (m_inputs[3]->read() & 2) | sns0;
}

// inputs

static INPUT_PORTS_START( compperf )
	PORT_START("IN.0") // AI
	PORT_CONFNAME( 0x07, 0x01, "Game" )
	PORT_CONFSETTING(    0x01, "1" )
	PORT_CONFSETTING(    0x02, "2" )
	PORT_CONFSETTING(    0x04, "3" )
	PORT_CONFSETTING(    0x00, "4" )
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_SELECT ) PORT_NAME("Score")

	PORT_START("IN.1") // BI
	PORT_CONFNAME( 0x03, 0x00, "Mode" )
	PORT_CONFSETTING(    0x01, "T" ) // Test
	PORT_CONFSETTING(    0x00, "N" ) // New
	PORT_CONFSETTING(    0x02, "R" ) // Repeat
	PORT_CONFNAME( 0x0c, 0x00, DEF_STR( Difficulty ) )
	PORT_CONFSETTING(    0x00, "1" )
	PORT_CONFSETTING(    0x08, "2" )
	PORT_CONFSETTING(    0x04, "3" )

	PORT_START("IN.2") // EO/DO/CO SNS0
	PORT_BIT( 0x001, IP_ACTIVE_HIGH, IPT_BUTTON1 )
	PORT_BIT( 0x002, IP_ACTIVE_HIGH, IPT_BUTTON2 )
	PORT_BIT( 0x004, IP_ACTIVE_HIGH, IPT_BUTTON3 )
	PORT_BIT( 0x008, IP_ACTIVE_HIGH, IPT_BUTTON4 )
	PORT_BIT( 0x010, IP_ACTIVE_HIGH, IPT_BUTTON5 )
	PORT_BIT( 0x020, IP_ACTIVE_HIGH, IPT_BUTTON6 )
	PORT_BIT( 0x040, IP_ACTIVE_HIGH, IPT_BUTTON7 )
	PORT_BIT( 0x080, IP_ACTIVE_HIGH, IPT_BUTTON8 )
	PORT_BIT( 0x100, IP_ACTIVE_HIGH, IPT_BUTTON9 )
	PORT_BIT( 0x200, IP_ACTIVE_HIGH, IPT_BUTTON10 ) // 0

	PORT_START("IN.3") // SNS1
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_START ) PORT_NAME("Set")
INPUT_PORTS_END

// config

void compperf_state::compperf(machine_config &config)
{
	// basic machine hardware
	MN1400_28PINS(config, m_maincpu, 290000); // approximation - RC osc. R=18K, C=100pF
	m_maincpu->write_c().set(FUNC(compperf_state::write_c));
	m_maincpu->write_d().set(FUNC(compperf_state::write_d));
	m_maincpu->write_e().set(FUNC(compperf_state::write_e));
	m_maincpu->read_a().set_ioport("IN.0");
	m_maincpu->read_b().set_ioport("IN.1");
	m_maincpu->read_sns().set(FUNC(compperf_state::read_sns));

	// video hardware
	PWM_DISPLAY(config, m_display).set_size(1, 10);
	m_display->set_bri_levels(0.25);
	config.set_default_layout(layout_compperf);

	// sound hardware
	SPEAKER(config, "mono").front_center();
	SPEAKER_SOUND(config, m_speaker);
	static const double speaker_levels[4] = { 0.0, 1.0, -1.0, 0.0 };
	m_speaker->set_levels(4, speaker_levels);
	m_speaker->add_route(ALL_OUTPUTS, "mono", 0.125);
}

// roms

ROM_START( compperf )
	ROM_REGION( 0x0400, "maincpu", 0 )
	ROM_LOAD( "mn1400ml", 0x0000, 0x0400, CRC(8d5ab2af) SHA1(f6281bb5a5a7ffeead107681d68b66a4844e93ad) )

	ROM_REGION( 428, "maincpu:opla", 0 ) // 4-bit
	ROM_LOAD( "mn1400_common1_output.pla", 0, 428, CRC(07489d8b) SHA1(4fe65af8ee798490ed0bbe6a77d61713a2fb28b4) )
ROM_END





/*******************************************************************************

  Selchow & Righter Scrabble Lexor
  * PCB label: 2294HB
  * MN1405MS (die label: 1405 MS-0)
  * 8-digit 14-seg LEDs, 2-bit sound

  This is the MN1405 version, see scrablex.cpp for the MB8841 version.

*******************************************************************************/

class scrablexa_state : public hh_mn1400_state
{
public:
	scrablexa_state(const machine_config &mconfig, device_type type, const char *tag) :
		hh_mn1400_state(mconfig, type, tag)
	{ }

	void scrablexa(machine_config &config);

private:
	void update_display();
	void write_c(u16 data);
	void write_d(u8 data);
	void write_e(u8 data);
	template<int N> u8 read_abs();
};

// handlers

void scrablexa_state::update_display()
{
	u16 data = bitswap<14>(m_e << 12 | m_c,10,8,6,12,11,7,9,13,5,4,3,2,1,0);
	m_display->matrix(m_d, ~data);
}

void scrablexa_state::write_c(u16 data)
{
	// CO0-CO11: digit segments
	m_c = data;
	update_display();
}

void scrablexa_state::write_d(u8 data)
{
	// DO0-DO4: input mux
	// DO0-DO7: digit select
	m_inp_mux = data & 0x1f;
	m_d = data;
	update_display();
}

void scrablexa_state::write_e(u8 data)
{
	// EO0,EO1: digit segments
	m_e = data;
	update_display();

	// EO2,EO3: speaker out
	m_speaker->level_w(data >> 2 & 3);
}

template<int N>
u8 scrablexa_state::read_abs()
{
	// AI/BI/SNS: multiplexed inputs
	return read_inputs(5) >> (N * 4);
}

// inputs

static INPUT_PORTS_START( scrablexa )
	PORT_START("IN.0") // DO0
	PORT_BIT( 0x001, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_A) PORT_CHAR('A')
	PORT_BIT( 0x002, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_B) PORT_CHAR('B')
	PORT_BIT( 0x004, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_C) PORT_CHAR('C')
	PORT_BIT( 0x008, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_D) PORT_CHAR('D')
	PORT_BIT( 0x010, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_E) PORT_CHAR('E')
	PORT_BIT( 0x020, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_F) PORT_CHAR('F')
	PORT_BIT( 0x040, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_G) PORT_CHAR('G')
	PORT_BIT( 0x080, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_H) PORT_CHAR('H')
	PORT_BIT( 0x100, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_I) PORT_CHAR('I')

	PORT_START("IN.1") // DO1
	PORT_BIT( 0x001, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_J) PORT_CHAR('J')
	PORT_BIT( 0x002, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_K) PORT_CHAR('K')
	PORT_BIT( 0x004, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_L) PORT_CHAR('L')
	PORT_BIT( 0x008, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_M) PORT_CHAR('M')
	PORT_BIT( 0x010, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_N) PORT_CHAR('N')
	PORT_BIT( 0x020, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_O) PORT_CHAR('O')
	PORT_BIT( 0x040, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_P) PORT_CHAR('P')
	PORT_BIT( 0x080, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_Q) PORT_CHAR('Q')
	PORT_BIT( 0x100, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_R) PORT_CHAR('R')

	PORT_START("IN.2") // DO2
	PORT_BIT( 0x001, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_S) PORT_CHAR('S')
	PORT_BIT( 0x002, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_T) PORT_CHAR('T')
	PORT_BIT( 0x004, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_U) PORT_CHAR('U')
	PORT_BIT( 0x008, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_V) PORT_CHAR('V')
	PORT_BIT( 0x010, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_W) PORT_CHAR('W')
	PORT_BIT( 0x020, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_X) PORT_CHAR('X')
	PORT_BIT( 0x040, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_Y) PORT_CHAR('Y')
	PORT_BIT( 0x080, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_Z) PORT_CHAR('Z')
	PORT_BIT( 0x100, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_SPACE) PORT_CHAR('*')

	PORT_START("IN.3") // DO4
	PORT_BIT( 0x001, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x002, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_COMMA) PORT_NAME("Double")
	PORT_BIT( 0x004, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_STOP) PORT_NAME("Triple")
	PORT_BIT( 0x008, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_SLASH) PORT_NAME("Word")
	PORT_BIT( 0x010, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_EQUALS) PORT_NAME("Bonus")
	PORT_BIT( 0x020, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_MINUS) PORT_CHAR('-') PORT_NAME("Minus")
	PORT_BIT( 0x040, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_DEL) PORT_CODE(KEYCODE_BACKSPACE) PORT_CHAR(8) PORT_NAME("Clear")
	PORT_BIT( 0x080, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_ENTER) PORT_CHAR(13) PORT_NAME("Enter")
	PORT_BIT( 0x100, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("IN.4") // DO5
	PORT_BIT( 0x001, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_F1) PORT_NAME("Flash")
	PORT_BIT( 0x002, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_F2) PORT_NAME("Solo")
	PORT_BIT( 0x004, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_F3) PORT_NAME("Score")
	PORT_BIT( 0x008, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_1) PORT_CHAR('1')
	PORT_BIT( 0x010, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_2) PORT_CHAR('2')
	PORT_BIT( 0x020, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_3) PORT_CHAR('3')
	PORT_BIT( 0x040, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_4) PORT_CHAR('4')
	PORT_BIT( 0x080, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_5) PORT_NAME("Timer")
	PORT_BIT( 0x100, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_6) PORT_NAME("Review")
INPUT_PORTS_END

// config

void scrablexa_state::scrablexa(machine_config &config)
{
	// basic machine hardware
	MN1405(config, m_maincpu, 310000); // approximation - RC osc. R=15K, C=100pF
	m_maincpu->write_c().set(FUNC(scrablexa_state::write_c));
	m_maincpu->write_d().set(FUNC(scrablexa_state::write_d));
	m_maincpu->write_e().set(FUNC(scrablexa_state::write_e));
	m_maincpu->read_a().set(FUNC(scrablexa_state::read_abs<0>));
	m_maincpu->read_b().set(FUNC(scrablexa_state::read_abs<1>));
	m_maincpu->read_sns().set(FUNC(scrablexa_state::read_abs<2>));

	// video hardware
	PWM_DISPLAY(config, m_display).set_size(8, 14);
	m_display->set_segmask(0xff, 0x3fff);
	config.set_default_layout(layout_scrablexa);

	// sound hardware
	SPEAKER(config, "mono").front_center();
	SPEAKER_SOUND(config, m_speaker);
	static const double speaker_levels[4] = { 0.0, 1.0, -1.0, 0.0 };
	m_speaker->set_levels(4, speaker_levels);
	m_speaker->add_route(ALL_OUTPUTS, "mono", 0.125);
}

// roms

ROM_START( scrablexa )
	ROM_REGION( 0x0800, "maincpu", 0 )
	ROM_LOAD( "mn1405ms", 0x0000, 0x0800, CRC(d6d61d03) SHA1(b319cb0f0539e7516bb28f1182665b05b7dd16b1) )

	ROM_REGION( 428, "maincpu:opla", 0 )
	ROM_LOAD( "mn1400_scrablexa_output.pla", 0, 428, CRC(555fe168) SHA1(cf19be34391dae0a8aacc7be53f2a3415fed3108) )
ROM_END





/*******************************************************************************

  Tomy Basketball (model 7600)
  * PCB label: TOMY, BASKET, 2E018E01
  * MN9008 (28-pin MN1400, AI0-AI3 replaced with CO0-CO3, die label: 1400 BM-0)
  * 2 7seg LEDs, 29 other LEDs, 1-bit sound

  Two versions are known: one with a black bezel and one with a brown bezel,
  the internal hardware is the same. The other 2 games in this series (Soccer,
  Volleyball) use a TMS1000 MCU instead.

*******************************************************************************/

class tmbaskb_state : public hh_mn1400_state
{
public:
	tmbaskb_state(const machine_config &mconfig, device_type type, const char *tag) :
		hh_mn1400_state(mconfig, type, tag)
	{ }

	void tmbaskb(machine_config &config);

private:
	void update_display();
	void write_c(u16 data);
	void write_d(u8 data);
	void write_e(u8 data);
	u8 read_b();
};

// handlers

void tmbaskb_state::update_display()
{
	u8 data = bitswap<8>(m_e << 4 | m_d,7,5,1,4,3,2,6,0);
	m_display->matrix((m_c >> 1 & 0x30) | (m_c & 0xf), data);
}

void tmbaskb_state::write_c(u16 data)
{
	// CO0,CO1: digit select
	// CO2,CO3,CO5,CO6: led select
	m_c = data;
	update_display();

	// CO3,CO5: input mux
	m_inp_mux = bitswap<2>(data,5,3);

	// CO7: speaker out
	m_speaker->level_w(BIT(data, 7));
}

void tmbaskb_state::write_d(u8 data)
{
	// DO0-DO3: led data
	m_d = data;
	update_display();
}

void tmbaskb_state::write_e(u8 data)
{
	// EO0-EO3: led data
	m_e = data;
	update_display();
}

u8 tmbaskb_state::read_b()
{
	// BI1-BI3: multiplexed inputs, BI0: score button
	return (read_inputs(2) & 0xe) | (m_inputs[2]->read() & 1);
}

// inputs

static INPUT_PORTS_START( tmbaskb )
	PORT_START("IN.0") // CO3 BI (left)
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_BUTTON2 ) PORT_NAME("P1 Offense P")
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_BUTTON3 ) PORT_NAME("P1 Offense S")
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_NAME("P1 Defense")

	PORT_START("IN.1") // CO5 BI (right)
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_BUTTON2 ) PORT_COCKTAIL PORT_NAME("P2 Offense P")
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_BUTTON3 ) PORT_COCKTAIL PORT_NAME("P2 Offense S")
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_COCKTAIL PORT_NAME("P2 Defense")

	PORT_START("IN.2") // BI0
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_SELECT ) PORT_NAME("Score")

	PORT_START("IN.3") // SNS
	PORT_CONFNAME( 0x01, 0x01, DEF_STR( Players ) )
	PORT_CONFSETTING(    0x01, "1" )
	PORT_CONFSETTING(    0x00, "2" )
	PORT_CONFNAME( 0x02, 0x00, DEF_STR( Difficulty ) )
	PORT_CONFSETTING(    0x00, "1" ) // PRO1
	PORT_CONFSETTING(    0x02, "2" ) // PRO2
INPUT_PORTS_END

// config

void tmbaskb_state::tmbaskb(machine_config &config)
{
	// basic machine hardware
	MN1400_28PINS(config, m_maincpu, 290000); // approximation - RC osc. R=18K, C=100pF
	m_maincpu->write_c().set(FUNC(tmbaskb_state::write_c));
	m_maincpu->set_c_mask(0x3ef);
	m_maincpu->write_d().set(FUNC(tmbaskb_state::write_d));
	m_maincpu->write_e().set(FUNC(tmbaskb_state::write_e));
	m_maincpu->read_b().set(FUNC(tmbaskb_state::read_b));
	m_maincpu->read_sns().set_ioport("IN.3");

	// video hardware
	PWM_DISPLAY(config, m_display).set_size(6, 8);
	m_display->set_segmask(3, 0x7f);
	config.set_default_layout(layout_tmbaskb);

	// sound hardware
	SPEAKER(config, "mono").front_center();
	SPEAKER_SOUND(config, m_speaker);
	m_speaker->add_route(ALL_OUTPUTS, "mono", 0.25);
}

// roms

ROM_START( tmbaskb )
	ROM_REGION( 0x0400, "maincpu", 0 )
	ROM_LOAD( "tomy_basket_mn9008", 0x0000, 0x0400, CRC(25be3560) SHA1(17855397cf05963c1381191cd4731860b8e180a8) )

	ROM_REGION( 428, "maincpu:opla", 0 ) // 4-bit
	ROM_LOAD( "mn1400_common1_output.pla", 0, 428, CRC(07489d8b) SHA1(4fe65af8ee798490ed0bbe6a77d61713a2fb28b4) )
ROM_END



} // anonymous namespace

/*******************************************************************************

  Game driver(s)

*******************************************************************************/

//    YEAR  NAME       PARENT    COMPAT  MACHINE    INPUT      CLASS            INIT        COMPANY, FULLNAME, FLAGS
SYST( 1979, compperf,  0,        0,      compperf,  compperf,  compperf_state,  empty_init, "Lakeside", "Computer Perfection", MACHINE_SUPPORTS_SAVE )

SYST( 1980, scrablexa, scrablex, 0,      scrablexa, scrablexa, scrablexa_state, empty_init, "Selchow & Righter", "Scrabble Lexor: Computer Word Game (MN1405 version)", MACHINE_SUPPORTS_SAVE )

SYST( 1980, tmbaskb,   0,        0,      tmbaskb,   tmbaskb,   tmbaskb_state,   empty_init, "Tomy", "Basketball (Tomy)", MACHINE_SUPPORTS_SAVE )
