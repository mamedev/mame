// license:BSD-3-Clause
// copyright-holders:David Haywood
// thanks-to:azya

/*

The following should fit here

HT1132A Space War
HT1134A Pin Ball
HT1136A Football
HT1137A Motorcycle
HT113AA Streetfighters
HT113FA Submarine War
HT113JA Baseball
HT113RA Poker and Black Jack
HT113SA Casino Game 5-in-1
HT113LA Original "Tea" Brick Game
HTG1395 3-in-1 (Car racing, Soccer, The eagle preys on the chicken)

(and likely many more)

Other Gametech keychain games possibly using this HW:
- Pikorin Bros. (ピコリンブロス)
- Tetris Mini

TODO:
- add LCD deflicker like hh_sm510?

*/

#include "emu.h"

#include "cpu/ht1130/ht1130.h"

#include "screen.h"
#include "speaker.h"

// internal artwork
#include "hh_ht11xx_lcd.lh"


namespace {

// base class

class hh_ht11xx_state : public driver_device
{
public:
	virtual DECLARE_INPUT_CHANGED_MEMBER(input_wakeup);

protected:
	hh_ht11xx_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu")
	{ }

	void mcfg_svg_screen(machine_config &config, u16 width, u16 height, const char *tag = "screen");

	required_device<ht1130_device> m_maincpu;
};

INPUT_CHANGED_MEMBER(hh_ht11xx_state::input_wakeup)
{
	m_maincpu->set_input_line(HT1130_EXT_WAKEUP_LINE, newval ? CLEAR_LINE : ASSERT_LINE);
}

void hh_ht11xx_state::mcfg_svg_screen(machine_config &config, u16 width, u16 height, const char *tag)
{
	screen_device &screen(SCREEN(config, tag, SCREEN_TYPE_SVG));
	screen.set_refresh_hz(60);
	screen.set_size(width, height);
	screen.set_visarea_full();

	config.set_default_layout(layout_hh_ht11xx_lcd);
}


// HT1130 class

class hh_ht1130_state : public hh_ht11xx_state
{
public:
	hh_ht1130_state(const machine_config &mconfig, device_type type, const char *tag) :
		hh_ht11xx_state(mconfig, type, tag),
		m_out_x(*this, "%u.%u", 0U, 0U),
		m_in_pm(*this, "PM"),
		m_in_pp(*this, "PP"),
		m_in_ps(*this, "PS")
	{ }

	void ht1130_common(machine_config &config);
	void ga888(machine_config &config);
	void piko55(machine_config &config);

protected:
	virtual void machine_start() override ATTR_COLD;

	void segment_w(offs_t offset, u64 data);

private:
	output_finder<4, 32> m_out_x;
	required_ioport m_in_pm;
	required_ioport m_in_pp;
	required_ioport m_in_ps;
};

void hh_ht1130_state::machine_start()
{
	m_out_x.resolve();
}

void hh_ht1130_state::segment_w(offs_t offset, u64 data)
{
	// output to x.y where x = COM# and y = SEG#
	for (int i = 0; i < 32; i++)
		m_out_x[offset][i] = BIT(data, i);
}

void hh_ht1130_state::ht1130_common(machine_config &config)
{
	HT1130(config, m_maincpu, 1000000/8); // frequency?
	m_maincpu->segment_out_cb().set(FUNC(hh_ht1130_state::segment_w));

	m_maincpu->ps_in_cb().set_ioport(m_in_ps);
	m_maincpu->pp_in_cb().set_ioport(m_in_pp);
	m_maincpu->pm_in_cb().set_ioport(m_in_pm);

	SPEAKER(config, "speaker").front_center();

	mcfg_svg_screen(config, 698, 1080);
}


// HT1190 class

class hh_ht1190_state : public hh_ht11xx_state
{
public:
	hh_ht1190_state(const machine_config &mconfig, device_type type, const char *tag) :
		hh_ht11xx_state(mconfig, type, tag),
		m_out_x(*this, "%u.%u", 0U, 0U),
		m_in_pp(*this, "PP"),
		m_in_ps(*this, "PS")
	{ }

	void brke23p2(machine_config &config);

protected:
	virtual void machine_start() override ATTR_COLD;

	void segment_w(offs_t offset, u64 data);

private:
	output_finder<8, 40> m_out_x;
	required_ioport m_in_pp;
	required_ioport m_in_ps;
};

void hh_ht1190_state::machine_start()
{
	m_out_x.resolve();
}

void hh_ht1190_state::segment_w(offs_t offset, u64 data)
{
	// output to x.y where x = COM# and y = SEG#
	for (int i = 0; i < 40; i++)
		m_out_x[offset][i] = BIT(data, i);
}



/*******************************************************************************

  Minidrivers (optional subclass, I/O, Inputs, Machine Config, ROM Defs)

*******************************************************************************/

/*******************************************************************************

  E-Star Brick Game 96 in 1 (E-23 Plus Mark II)
  * Holtek HT1190
  * 10*20 LCD screen + custom segments, 1-bit sound

*******************************************************************************/

static INPUT_PORTS_START( brke23p2 )
	PORT_START("PS")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_START ) PORT_NAME("Start / Pause")
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_VOLUME_DOWN ) PORT_NAME("Mute")
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_POWER_ON ) PORT_NAME("On / Off") PORT_CHANGED_MEMBER(DEVICE_SELF, FUNC(hh_ht11xx_state::input_wakeup), 0)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("PP") // not a joystick, but buttons are used for directional inputs in the snake game etc.
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_NAME("Rotate / Direction")
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_NAME("Down / Mode")
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_NAME("Right / Speed")
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_BUTTON4 ) PORT_NAME("Left / Level")
INPUT_PORTS_END


void hh_ht1190_state::brke23p2(machine_config &config)
{
	HT1190(config, m_maincpu, 1000000/8); // frequency?
	m_maincpu->segment_out_cb().set(FUNC(hh_ht1190_state::segment_w));

	m_maincpu->ps_in_cb().set_ioport(m_in_ps);
	m_maincpu->pp_in_cb().set_ioport(m_in_pp);

	SPEAKER(config, "speaker").front_center();

	mcfg_svg_screen(config, 755, 1080);
}


ROM_START( brke23p2 )
	ROM_REGION( 0x1000, "maincpu", 0 )
	ROM_LOAD( "brke23p2.bin", 0x0000, 0x1000, CRC(8045fac4) SHA1(a36213309e6add31f31e4248f02f17de9914a5c1) ) // visual decap

	ROM_REGION( 0x280, "melody", 0 )
	ROM_LOAD( "e23plusmarkii96in1.srom", 0x000, 0x280, CRC(591a8a21) SHA1(f039359e8e1d1bf75581a4c852b263c8c140e072) )

	ROM_REGION( 160500, "screen", 0)
	ROM_LOAD( "brke23p2.svg", 0, 160500, CRC(9edf8aab) SHA1(f2ab907d23517612196648f1b5b0cb9b4a1ab3bd) )
ROM_END



/*******************************************************************************

  Block Game & Echo Key GA888
  * Holtek HT1130
  * 8*12 LCD screen + 8 custom segments, 1-bit sound

*******************************************************************************/

static INPUT_PORTS_START( ga888 ) // the unit also has an up button, and a reset button, is 'up' connected to anything?
	PORT_START("PS")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_VOLUME_DOWN ) PORT_NAME("Pause / Power") PORT_CHANGED_MEMBER(DEVICE_SELF, FUNC(hh_ht11xx_state::input_wakeup), 0)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("PP")
	PORT_BIT( 0x0f, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("PM")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_START ) PORT_NAME("Start / Rotate")
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_NAME("Down / Drop")
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_NAME("Right / Sound")
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_BUTTON4 ) PORT_NAME("Left")
INPUT_PORTS_END


void hh_ht1130_state::ga888(machine_config &config)
{
	ht1130_common(config);
}


ROM_START( ga888 )
	ROM_REGION( 0x1000, "maincpu", 0 )
	ROM_LOAD( "ga888.bin", 0x0000, 0x1000, CRC(cb3f8ff4) SHA1(00b08773e8ee7577377f4d52cd1e6bb657b6f242) ) // visual decap

	ROM_REGION( 0x280, "melody", 0 )
	ROM_LOAD( "ga888.srom", 0x000, 0x280, CRC(a8495ac7) SHA1(f6c24fc9622bff73ab09b5ee77eb338f27f7a6b1) )

	ROM_REGION( 85508, "screen", 0)
	ROM_LOAD( "ga888.svg", 0, 85508, CRC(9ab6dd67) SHA1(a4365a00204bf4e376f28600c0b87289bda0cbb0) )
ROM_END



/*******************************************************************************

  Pikorin 55 (ピコリン55) - keychain game
  * Holtek HT1130
  * 8*12 LCD screen + 8 custom segments, 1-bit sound

*******************************************************************************/

static INPUT_PORTS_START( piko55 )
	PORT_START("PS")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_VOLUME_DOWN ) PORT_NAME("On / Pause") PORT_CHANGED_MEMBER(DEVICE_SELF, FUNC(hh_ht11xx_state::input_wakeup), 0)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("PP")
	PORT_BIT( 0x0f, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("PM")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_START ) PORT_NAME("Start / Rotate")
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_NAME("Down / Game")
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_NAME("Right")
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_BUTTON4 ) PORT_NAME("Left / Select")
INPUT_PORTS_END


void hh_ht1130_state::piko55(machine_config &config)
{
	ht1130_common(config);
}


ROM_START( piko55 )
	ROM_REGION( 0x1000, "maincpu", 0 )
	ROM_LOAD( "keychain55in1.bin", 0x0000, 0x1000, CRC(c8623cf2) SHA1(27fe405a8a866bfc6a857af886ed00f64083c2cc) ) // visual decap

	ROM_REGION( 0x280, "melody", 0 )
	ROM_LOAD( "keychain55in1.srom", 0x000, 0x280, CRC(5667eb80) SHA1(4aee372f87a988ae46790538f4435c4a249e3686) )

	ROM_REGION( 85508, "screen", 0)
	ROM_LOAD( "keychain55in1.svg", 0, 85508, CRC(9ab6dd67) SHA1(a4365a00204bf4e376f28600c0b87289bda0cbb0) )
ROM_END


} // anonymous namespace


/*******************************************************************************

  Game driver(s)

*******************************************************************************/

//    YEAR  NAME        PARENT  COMPAT  MACHINE   INPUT     CLASS            INIT        COMPANY,       FULLNAME,                                   FLAGS
CONS( 1993, brke23p2,   0,      0,      brke23p2, brke23p2, hh_ht1190_state, empty_init, "E-Star",      "Brick Game 96 in 1 (E-23 Plus Mark II)",   MACHINE_IMPERFECT_TIMING | MACHINE_NO_SOUND ) // some other dieshots have 1996 on them, it is also possible the software is from Holtek
CONS( 199?, ga888,      0,      0,      ga888,    ga888,    hh_ht1130_state, empty_init, "<unknown>",   "Block Game & Echo Key GA888",              MACHINE_IMPERFECT_TIMING | MACHINE_NO_SOUND ) // clone of Tetris Jr?
CONS( 199?, piko55,     0,      0,      piko55,   piko55,   hh_ht1130_state, empty_init, "Gametech",    "Pikorin 55",                               MACHINE_IMPERFECT_TIMING | MACHINE_NO_SOUND )
