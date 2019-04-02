// license:BSD-3-Clause
// copyright-holders:Tomasz Slanina
/*
 Moero!! Pro Yakyuu Homerun Kyousou - (c) 1988 Jaleco
 Dynamic Shoot Kyousou - (c) 1988 Jaleco
 Ganbare Jajamaru Saisho wa Goo / Ganbare Jajamaru Hop Step & Jump - (c) 1990 Jaleco
 Driver by Tomasz Slanina

 They're gambling games (seems to be aimed at kids), with a little skill involved.
 ganjaja has a coin hopper, I don't know how the other games reward the player.

 *weird* hardware - based on NES version
 (gfx bank changed in the middle of screen,
  sprites in NES format etc)

 homerun and ganjaja use an extra soundchip for playing voice/samples

Todo :
 - dump homerun sample rom
 - improve controls/dips
 - fix sprite glitches in ganjaja Hop Step & Jump

-----------------------------------
Moero!! Pro Yakyuu Homerun Kyousou
Jaleco, 1988

PCB Layout
----------

HR-8847
-----------------------------------
| YM2203    Z80B         6264     |
|YM3014 DSW(8)     HOMERUN.43     |
|    D7756C   6264                |
|                                 |
|J  640KHz   HOMERUN.60           |
|A 2018                           |
|M      2018    2018          8255|
|M          2018                  |
|A                                |
|                                 |
|                                 |
| HOMERUN.120                20MHz|
-----------------------------------

Notes:
      Z80 clock: 5.000MHz
          VSync: 60Hz
          HSync: 15.21kHz

*/

#include "emu.h"
#include "includes/homerun.h"

#include "cpu/z80/z80.h"
#include "machine/i8255.h"
#include "sound/2203intf.h"
#include "sound/samples.h"
#include "speaker.h"


/***************************************************************************

  I/O / Memory

***************************************************************************/

void homerun_state::control_w(u8 data)
{
	// d0, d1: somehow related to port $40?

	// d4: d7756 start pin
	// d5: d7756 reset pin(?)
	if (m_d7756 != nullptr)
	{
		m_d7756->reset_w(!BIT(data, 5));
		m_d7756->start_w(!BIT(data, 4));
	}
	if (m_samples != nullptr)
	{
		// play MAME sample if a dump of the internal rom does not exist
		if (data & 0x20 & ~m_control)
			m_samples->stop(0);

		if (~data & 0x10 & m_control && !m_samples->playing(0))
		{
			samples_iterator iter(*m_samples);
			if (m_sample < iter.count())
				m_samples->start(0, m_sample);
		}
	}

	// other bits: ?
	m_control = data;
}

void homerun_state::d7756_sample_w(u8 data)
{
	m_sample = data;

	if (m_d7756 != nullptr)
		m_d7756->port_w(data);
}

void homerun_state::mem_map(address_map &map)
{
	map(0x0000, 0x3fff).rom();
	map(0x4000, 0x7fff).bankr("mainbank");
	map(0x8000, 0x9fff).ram().w(FUNC(homerun_state::videoram_w)).share("videoram");
	map(0xa000, 0xa0ff).ram().share("spriteram");
	map(0xb000, 0xb03f).ram().w(m_palette, FUNC(palette_device::write8)).share("palette");
	map(0xc000, 0xdfff).ram();
}

void homerun_state::io_map(address_map &map)
{
	map.global_mask(0xff);
	map(0x10, 0x10).w(FUNC(homerun_state::d7756_sample_w));
	map(0x20, 0x20).w(FUNC(homerun_state::control_w));
	map(0x30, 0x33).rw("ppi8255", FUNC(i8255_device::read), FUNC(i8255_device::write));
	map(0x40, 0x40).portr("IN0");
	map(0x50, 0x50).portr("IN2");
	map(0x60, 0x60).portr("IN1");
	map(0x70, 0x71).rw("ymsnd", FUNC(ym2203_device::read), FUNC(ym2203_device::write));
}


CUSTOM_INPUT_MEMBER(homerun_state::homerun_d7756_busy_r)
{
	return m_samples->playing(0) ? 0 : 1;
}

CUSTOM_INPUT_MEMBER(homerun_state::ganjaja_d7756_busy_r)
{
	return m_d7756->busy_r();
}

CUSTOM_INPUT_MEMBER(homerun_state::ganjaja_hopper_status_r)
{
	// gives hopper error if not 0
	return 0;
}


/***************************************************************************

  Inputs

***************************************************************************/

static INPUT_PORTS_START( homerun )
	PORT_START("IN0")
	PORT_BIT( 0x08, IP_ACTIVE_LOW,  IPT_COIN1 )
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_CUSTOM_MEMBER(DEVICE_SELF, homerun_state, sprite0_r, nullptr)
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_CUSTOM_MEMBER(DEVICE_SELF, homerun_state, homerun_d7756_busy_r, nullptr)
	PORT_BIT( 0x37, IP_ACTIVE_HIGH, IPT_UNKNOWN )

	PORT_START("IN1")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_JOYSTICK_UP )
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_JOYSTICK_DOWN )
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT )
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT )
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_START1 )
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_BUTTON1 )

	PORT_START("IN2")
	PORT_BIT( 0x20, IP_ACTIVE_LOW,  IPT_COIN2 )
	PORT_BIT( 0xdf, IP_ACTIVE_HIGH, IPT_UNKNOWN )

	PORT_START("DSW")
	PORT_DIPNAME( 0x03, 0x02, DEF_STR( Coin_B ) )       PORT_DIPLOCATION("DIPSW:1,2")
	PORT_DIPSETTING(    0x00, DEF_STR( 5C_1C ) )
	PORT_DIPSETTING(    0x01, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(    0x02, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x03, DEF_STR( Free_Play ) )
	PORT_DIPUNKNOWN_DIPLOC( 0x04, 0x04, "DIPSW:3" )
	PORT_DIPUNKNOWN_DIPLOC( 0x08, 0x08, "DIPSW:4" )
	PORT_DIPUNKNOWN_DIPLOC( 0x10, 0x10, "DIPSW:5" )
	PORT_DIPUNKNOWN_DIPLOC( 0x20, 0x20, "DIPSW:6" )
	PORT_DIPUNKNOWN_DIPLOC( 0x40, 0x40, "DIPSW:7" )
	PORT_DIPNAME( 0x80, 0x80, DEF_STR( Coin_A ) )       PORT_DIPLOCATION("DIPSW:8")
	PORT_DIPSETTING(    0x80, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x00, DEF_STR( 1C_2C ) )
INPUT_PORTS_END

static INPUT_PORTS_START( dynashot )
	PORT_START("IN0")
	PORT_BIT( 0x08, IP_ACTIVE_LOW,  IPT_COIN1 )
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_CUSTOM_MEMBER(DEVICE_SELF, homerun_state, sprite0_r, nullptr)
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_UNUSED ) // doesn't have d7756
	PORT_BIT( 0x37, IP_ACTIVE_HIGH, IPT_UNKNOWN )

	PORT_START("IN1")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_JOYSTICK_UP )
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_JOYSTICK_DOWN )
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT )
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT )
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_START1 )
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_BUTTON1 )

	PORT_START("IN2")
	PORT_BIT( 0x20, IP_ACTIVE_LOW,  IPT_COIN2 )
	PORT_BIT( 0xdf, IP_ACTIVE_HIGH, IPT_UNKNOWN )

	PORT_START("DSW")
	PORT_DIPNAME( 0x03, 0x02, DEF_STR( Coin_B ) )       PORT_DIPLOCATION("DIPSW:1,2")
	PORT_DIPSETTING(    0x00, DEF_STR( 5C_1C ) )
	PORT_DIPSETTING(    0x01, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(    0x02, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x03, DEF_STR( Free_Play ) )
	PORT_DIPUNKNOWN_DIPLOC( 0x04, 0x04, "DIPSW:3" ) // collisions? (not all bits)
	PORT_DIPUNKNOWN_DIPLOC( 0x08, 0x08, "DIPSW:4" ) // "
	PORT_DIPUNKNOWN_DIPLOC( 0x10, 0x10, "DIPSW:5" ) // "
	PORT_DIPUNKNOWN_DIPLOC( 0x20, 0x20, "DIPSW:6" ) // "
	PORT_DIPUNKNOWN_DIPLOC( 0x40, 0x40, "DIPSW:7" ) // "
	PORT_DIPNAME( 0x80, 0x80, DEF_STR( Coin_A ) )       PORT_DIPLOCATION("DIPSW:8")
	PORT_DIPSETTING(    0x80, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x00, DEF_STR( 1C_2C ) )
INPUT_PORTS_END

static INPUT_PORTS_START( ganjaja )
	PORT_START("IN0")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_UNKNOWN ) // ?
	PORT_BIT( 0x08, IP_ACTIVE_LOW,  IPT_COIN1 )
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_CUSTOM_MEMBER(DEVICE_SELF, homerun_state, sprite0_r, nullptr)
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_CUSTOM_MEMBER(DEVICE_SELF, homerun_state, ganjaja_d7756_busy_r, nullptr)
	PORT_BIT( 0x36, IP_ACTIVE_HIGH, IPT_UNKNOWN )

	PORT_START("IN1")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_JOYSTICK_UP    ) PORT_NAME("P1 Up / Rock")
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_JOYSTICK_DOWN  ) PORT_NAME("P1 Down / Paper")
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT ) PORT_NAME("P1 Right / Scissors")
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT  ) // unused?
	PORT_BIT( 0x30, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_CUSTOM_MEMBER(DEVICE_SELF, homerun_state, ganjaja_hopper_status_r, nullptr)
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_UNKNOWN )

	PORT_START("IN2")
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_UNKNOWN ) // bit masked with IN0 IPT_COIN1, maybe coin lockout?
	PORT_BIT( 0x20, IP_ACTIVE_LOW,  IPT_COIN2 )
	PORT_BIT( 0xcf, IP_ACTIVE_HIGH, IPT_UNKNOWN )

	PORT_START("DSW")
	// starts game with coin in if 1c_1c?
	PORT_DIPNAME( 0x01, 0x00, DEF_STR( Coin_B ) )       PORT_DIPLOCATION("DIPSW:1")
	PORT_DIPSETTING(    0x00, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x01, DEF_STR( 1C_1C ) )
	PORT_DIPNAME( 0x02, 0x02, DEF_STR( Coin_A ) )       PORT_DIPLOCATION("DIPSW:2")
	PORT_DIPSETTING(    0x00, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x02, DEF_STR( 1C_1C ) ) // game will boot with 1 credit inserted
	PORT_DIPUNKNOWN_DIPLOC( 0x04, 0x04, "DIPSW:3" )
	PORT_DIPUNKNOWN_DIPLOC( 0x08, 0x08, "DIPSW:4" )
	PORT_DIPUNKNOWN_DIPLOC( 0x10, 0x10, "DIPSW:5" )
	PORT_DIPUNKNOWN_DIPLOC( 0x20, 0x20, "DIPSW:6" ) // chance to win?
	PORT_DIPUNKNOWN_DIPLOC( 0x40, 0x40, "DIPSW:7" ) // "
	PORT_DIPNAME( 0x80, 0x80, "Game" )                  PORT_DIPLOCATION("DIPSW:8")
	PORT_DIPSETTING(    0x80, "Saisho wa Goo" )
	PORT_DIPSETTING(    0x00, "Hop Step & Jump" )
INPUT_PORTS_END


/***************************************************************************

  Machine Config

***************************************************************************/

// homerun samples, note that this is the complete rom contents; not all samples are used in this game
static const char *const homerun_sample_names[] =
{
	"*homerun",
	"00", // strike
	"01", // ball
	"02", // time (ask for time out)
	"03", // out
	"04", // safe
	"05", // foul
	"06", // yah (field player catching a fast ball)
	"07", // batter out (batter out after 3 strikes)
	"08", // play ball
	"09", // ball four
	"10", // home run
	"11", // new pitcher (choosing new pitcher in time out)
	"12", // ouch (batter gets hit by pitcher)
	"13", // aho (be called a fool by supervisor)
	"14", // bat hits ball
	"15", // crowd cheers
	nullptr
};

/**************************************************************************/

static const gfx_layout gfxlayout =
{
	8,8,
	RGN_FRAC(1,1),
	2,
	{ 8*8,0},
	{ STEP8(0,1) },
	{ STEP8(0,8) },
	8*8*2
};

static const gfx_layout spritelayout =
{
	16,16,
	RGN_FRAC(1,1),
	2,
	{ 8*8,0 },
	{ STEP8(0,1), STEP8(8*8*2,1) },
	{ STEP8(0,8), STEP8(8*8*2*2,8) },
	8*8*2*4
};

static GFXDECODE_START( gfx_homerun )
	GFXDECODE_ENTRY( "gfx1", 0, gfxlayout,    0, 16 )
	GFXDECODE_ENTRY( "gfx2", 0, spritelayout, 0, 16 )
GFXDECODE_END


/**************************************************************************/

void homerun_state::machine_start()
{
	u8 *ROM = memregion("maincpu")->base();

	m_mainbank->configure_entries(0, 8, &ROM[0x00000], 0x4000);

	save_item(NAME(m_control));
	save_item(NAME(m_sample));
}

void homerun_state::machine_reset()
{
	control_w(0);
	d7756_sample_w(0);
	banking_w(0);
	m_scrolly = 0;
	m_scrollx = 0;
}

/**************************************************************************/

void homerun_state::dynashot(machine_config &config)
{
	/* basic machine hardware */
	Z80(config, m_maincpu, XTAL(20'000'000)/4);
	m_maincpu->set_addrmap(AS_PROGRAM, &homerun_state::mem_map);
	m_maincpu->set_addrmap(AS_IO, &homerun_state::io_map);
	m_maincpu->set_vblank_int("screen", FUNC(homerun_state::irq0_line_hold));

	i8255_device &ppi(I8255A(config, "ppi8255"));
	ppi.out_pa_callback().set(FUNC(homerun_state::scrollhi_w));
	ppi.out_pb_callback().set(FUNC(homerun_state::scrolly_w));
	ppi.out_pc_callback().set(FUNC(homerun_state::scrollx_w));

	/* video hardware */
	SCREEN(config, m_screen, SCREEN_TYPE_RASTER);
	m_screen->set_raw(XTAL(20'000'000)/4,328,0,256,253,0,240);
	m_screen->set_screen_update(FUNC(homerun_state::screen_update));
	m_screen->set_palette(m_palette);

	GFXDECODE(config, m_gfxdecode, m_palette, gfx_homerun);
	PALETTE(config, m_palette).set_format(1, &homerun_state::homerun_RGB332, 16*4);

	/* sound hardware */
	SPEAKER(config, "mono").front_center();

	ym2203_device &ymsnd(YM2203(config, "ymsnd", XTAL(20'000'000)/8));
	ymsnd.port_a_read_callback().set_ioport("DSW");
	ymsnd.port_b_write_callback().set(FUNC(homerun_state::banking_w));
	ymsnd.add_route(ALL_OUTPUTS, "mono", 0.50);
}

void homerun_state::homerun(machine_config &config)
{
	dynashot(config);

	/* sound hardware */
	UPD7756(config, m_d7756);
	m_d7756->add_route(ALL_OUTPUTS, "mono", 0.75);

	SAMPLES(config, m_samples);
	m_samples->set_channels(1);
	m_samples->set_samples_names(homerun_sample_names);
	m_samples->add_route(ALL_OUTPUTS, "mono", 0.50);
}

void homerun_state::ganjaja(machine_config &config)
{
	dynashot(config);

	/* basic machine hardware */
	m_maincpu->set_periodic_int(FUNC(homerun_state::irq0_line_hold), attotime::from_hz(4*60)); // ?

	/* sound hardware */
	UPD7756(config, m_d7756);
	m_d7756->add_route(ALL_OUTPUTS, "mono", 0.75);
}



/**************************************************************************/

ROM_START( homerun )
	ROM_REGION( 0x20000, "maincpu", 0 )
	ROM_LOAD( "homerun.ic43",   0x00000, 0x20000, CRC(e759e476) SHA1(ad4f356ff26209033320a3e6353e4d4d9beb59c1) )

	ROM_REGION( 0x10000, "gfx1", 0 )
	ROM_LOAD( "homerun.ic60",   0x00000, 0x10000, CRC(69a720d1) SHA1(0f0a4877578f358e9e829ece8c31e23f01adcf83) )

	ROM_REGION( 0x20000, "gfx2", 0 )
	ROM_LOAD( "homerun.ic120",  0x00000, 0x20000, CRC(52f0709b) SHA1(19e675bcccadb774f60ec5929fc1fb5cf0d3f617) )

	ROM_REGION( 0x08000, "d7756", ROMREGION_ERASE00 )
	ROM_LOAD( "d7756c.ic98",    0x00000, 0x08000, NO_DUMP ) /* D7756C built-in rom - very likely the same rom as [Moero!! Pro Yakyuu (Black/Red)] on Famicom, and [Moero!! Nettou Yakyuu '88] on MSX2 */
ROM_END

ROM_START( nhomerun )
	ROM_REGION( 0x20000, "maincpu", 0 )
	ROM_LOAD( "1.ic43",   0x00000, 0x20000, CRC(aed96d6d) SHA1(5cb3932f4cfa3f6c0134ac20a1747c562db31a65) )

	ROM_REGION( 0x10000, "gfx1", 0 )
	ROM_LOAD( "3.ic60",   0x00000, 0x10000, CRC(69a720d1) SHA1(0f0a4877578f358e9e829ece8c31e23f01adcf83) )

	ROM_REGION( 0x20000, "gfx2", 0 )
	ROM_LOAD( "2.ic120",  0x00000, 0x20000, CRC(57e9b757) SHA1(8190d690721005407a5b06d13d64e70301d1e925) )

	ROM_REGION( 0x08000, "d7756", ROMREGION_ERASE00 )
	ROM_LOAD( "d7756c.ic98",    0x00000, 0x08000, NO_DUMP )
ROM_END

ROM_START( dynashot )
	ROM_REGION( 0x20000, "maincpu", 0 )
	ROM_LOAD( "1.ic43",         0x00000, 0x20000, CRC(bf3c9586) SHA1(439effbda305f5fa265e5897c81dc1447e5d867d) )

	ROM_REGION( 0x10000, "gfx1", 0 )
	ROM_LOAD( "3.ic60",         0x00000, 0x10000, CRC(77d6a608) SHA1(a31ff343a5d4d6f20301c030ecc2e252149bcf9d) )

	ROM_REGION( 0x20000, "gfx2", 0 )
	ROM_LOAD( "2.ic120",        0x00000, 0x20000, CRC(bedf7b98) SHA1(cb6c5fcaf8df5f5c7636c3c8f79b9dda78e30c2e) )
ROM_END


ROM_START( ganjaja )
	ROM_REGION( 0x20000, "maincpu", 0 )
	ROM_LOAD( "1.ic43",         0x00000, 0x20000, CRC(dad57543) SHA1(dbd8b5cee33756ee5e3c41bf84c0f7141d3466dc) )

	ROM_REGION( 0x10000, "gfx1", 0 )
	ROM_LOAD( "ic60",           0x00000, 0x10000, CRC(855f6b28) SHA1(386411e88cf9bed54fe2073f0828d579cb1d04ee) )

	ROM_REGION( 0x20000, "gfx2", 0 )
	ROM_LOAD( "2.ic120",        0x00000, 0x20000, CRC(e65d4d57) SHA1(2ec9e5bdaa94b808573313b6eca657d798004b53) )

	ROM_REGION( 0x08000, "d7756", 0 )
	ROM_LOAD( "d77p56cr.ic98",  0x00000, 0x08000, CRC(06a234ac) SHA1(b4ceff3f9f78551cf4a085642e162e33b266f067) ) /* D77P56CR OTP rom (One-Time Programmable, note the extra P) */
ROM_END


//    YEAR  NAME      PARENT    MACHINE   INPUT     STATE          INIT        ROT    COMPANY   FULLNAME                                                            FLAGS
GAME( 1988, nhomerun, 0,        homerun,  homerun,  homerun_state, empty_init, ROT0, "Jaleco", "NEW Moero!! Pro Yakyuu Homerun Kyousou",                            MACHINE_IMPERFECT_GRAPHICS | MACHINE_IMPERFECT_SOUND | MACHINE_SUPPORTS_SAVE ) // same as below but harder?
GAME( 1988, homerun,  nhomerun, homerun,  homerun,  homerun_state, empty_init, ROT0, "Jaleco", "Moero!! Pro Yakyuu Homerun Kyousou",                                MACHINE_IMPERFECT_GRAPHICS | MACHINE_IMPERFECT_SOUND | MACHINE_SUPPORTS_SAVE )
GAME( 1988, dynashot, 0,        dynashot, dynashot, homerun_state, empty_init, ROT0, "Jaleco", "Dynamic Shoot Kyousou",                                             MACHINE_IMPERFECT_GRAPHICS | MACHINE_SUPPORTS_SAVE )
GAME( 1990, ganjaja,  0,        ganjaja,  ganjaja,  homerun_state, empty_init, ROT0, "Jaleco", "Ganbare Jajamaru Saisho wa Goo / Ganbare Jajamaru Hop Step & Jump", MACHINE_IMPERFECT_GRAPHICS | MACHINE_SUPPORTS_SAVE )
