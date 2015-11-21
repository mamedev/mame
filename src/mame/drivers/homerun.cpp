// license:LGPL-2.1+
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
#include "cpu/z80/z80.h"
#include "machine/i8255.h"
#include "sound/2203intf.h"
#include "sound/samples.h"
#include "includes/homerun.h"


/***************************************************************************

  I/O / Memory

***************************************************************************/

WRITE8_MEMBER(homerun_state::homerun_control_w)
{
	// d0, d1: somehow related to port $40?

	// d4: d7756 start pin
	// d5: d7756 reset pin(?)
	if (m_d7756 != NULL)
	{
		m_d7756->reset_w(~data & 0x20);
		m_d7756->start_w(~data & 0x10);
	}
	if (m_samples != NULL)
	{
		// play MAME sample if a dump of the internal rom does not exist
		if (data & 0x20 & ~m_control)
			m_samples->stop(0);

		if (~data & 0x10 & m_control && !m_samples->playing(0))
		{
			samples_iterator iter(m_samples);
			if (m_sample < iter.count())
				m_samples->start(0, m_sample);
		}
	}

	// other bits: ?
	m_control = data;
}

WRITE8_MEMBER(homerun_state::homerun_d7756_sample_w)
{
	m_sample = data;

	if (m_d7756 != NULL)
		m_d7756->port_w(space, 0, data);
}

static ADDRESS_MAP_START( homerun_memmap, AS_PROGRAM, 8, homerun_state )
	AM_RANGE(0x0000, 0x3fff) AM_ROM
	AM_RANGE(0x4000, 0x7fff) AM_ROMBANK("bank1")
	AM_RANGE(0x8000, 0x9fff) AM_RAM_WRITE(homerun_videoram_w) AM_SHARE("videoram")
	AM_RANGE(0xa000, 0xa0ff) AM_RAM AM_SHARE("spriteram")
	AM_RANGE(0xb000, 0xb03f) AM_RAM_WRITE(homerun_color_w) AM_SHARE("colorram")
	AM_RANGE(0xc000, 0xdfff) AM_RAM
ADDRESS_MAP_END

static ADDRESS_MAP_START( homerun_iomap, AS_IO, 8, homerun_state )
	ADDRESS_MAP_GLOBAL_MASK(0xff)
	AM_RANGE(0x10, 0x10) AM_WRITE(homerun_d7756_sample_w)
	AM_RANGE(0x20, 0x20) AM_WRITE(homerun_control_w)
	AM_RANGE(0x30, 0x33) AM_DEVREADWRITE("ppi8255", i8255_device, read, write)
	AM_RANGE(0x40, 0x40) AM_READ_PORT("IN0")
	AM_RANGE(0x50, 0x50) AM_READ_PORT("IN2")
	AM_RANGE(0x60, 0x60) AM_READ_PORT("IN1")
	AM_RANGE(0x70, 0x71) AM_DEVREADWRITE("ymsnd", ym2203_device, read, write)
ADDRESS_MAP_END


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
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_SPECIAL ) PORT_CUSTOM_MEMBER(DEVICE_SELF, homerun_state, homerun_sprite0_r, NULL)
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_SPECIAL ) PORT_CUSTOM_MEMBER(DEVICE_SELF, homerun_state, homerun_d7756_busy_r, NULL)
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
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_SPECIAL ) PORT_CUSTOM_MEMBER(DEVICE_SELF, homerun_state, homerun_sprite0_r, NULL)
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
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_SPECIAL ) PORT_CUSTOM_MEMBER(DEVICE_SELF, homerun_state, homerun_sprite0_r, NULL)
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_SPECIAL ) PORT_CUSTOM_MEMBER(DEVICE_SELF, homerun_state, ganjaja_d7756_busy_r, NULL)
	PORT_BIT( 0x36, IP_ACTIVE_HIGH, IPT_UNKNOWN )

	PORT_START("IN1")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_JOYSTICK_UP    ) PORT_NAME("P1 Up / Rock")
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_JOYSTICK_DOWN  ) PORT_NAME("P1 Down / Paper")
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT ) PORT_NAME("P1 Right / Scissors")
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT  ) // unused?
	PORT_BIT( 0x30, IP_ACTIVE_HIGH, IPT_SPECIAL ) PORT_CUSTOM_MEMBER(DEVICE_SELF, homerun_state, ganjaja_hopper_status_r, NULL)
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_UNKNOWN )

	PORT_START("IN2")
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_UNKNOWN ) // bit masked with IN0 IPT_COIN1, maybe coin lockout?
	PORT_BIT( 0x20, IP_ACTIVE_LOW,  IPT_COIN2 )
	PORT_BIT( 0xcf, IP_ACTIVE_HIGH, IPT_UNKNOWN )

	PORT_START("DSW")
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
	0
};

/**************************************************************************/

static const gfx_layout gfxlayout =
{
	8,8,
	RGN_FRAC(1,1),
	2,
	{ 8*8,0},
	{ 0, 1, 2, 3, 4, 5, 6, 7},
	{ 0*8, 1*8, 2*8, 3*8, 4*8, 5*8, 6*8, 7*8},
	8*8*2
};

static const gfx_layout spritelayout =
{
	16,16,
	RGN_FRAC(1,1),
	2,
	{ 8*8,0},
	{ 0, 1, 2, 3, 4, 5, 6, 7,0+8*8*2,1+8*8*2,2+8*8*2,3+8*8*2,4+8*8*2,5+8*8*2,6+8*8*2,7+8*8*2},
	{ 0*8, 1*8, 2*8, 3*8, 4*8, 5*8, 6*8, 7*8, 0*8+2*8*8*2,1*8+2*8*8*2,2*8+2*8*8*2,3*8+2*8*8*2,4*8+2*8*8*2,5*8+2*8*8*2,6*8+2*8*8*2,7*8+2*8*8*2},
	8*8*2*4
};

static GFXDECODE_START( homerun )
	GFXDECODE_ENTRY( "gfx1", 0, gfxlayout,   0, 16 )
	GFXDECODE_ENTRY( "gfx2", 0, spritelayout,   0, 16 )
GFXDECODE_END


/**************************************************************************/

void homerun_state::machine_start()
{
	UINT8 *ROM = memregion("maincpu")->base();

	membank("bank1")->configure_entry(0, &ROM[0x00000]);
	membank("bank1")->configure_entries(1, 7, &ROM[0x10000], 0x4000);

	save_item(NAME(m_control));
	save_item(NAME(m_sample));
	save_item(NAME(m_gfx_ctrl));
	save_item(NAME(m_scrolly));
	save_item(NAME(m_scrollx));
}

void homerun_state::machine_reset()
{
	m_control = 0;
	m_sample = 0;
	m_gfx_ctrl = 0;
	m_scrolly = 0;
	m_scrollx = 0;
}

/**************************************************************************/

static MACHINE_CONFIG_START( dynashot, homerun_state )

	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", Z80, XTAL_20MHz/4)
	MCFG_CPU_PROGRAM_MAP(homerun_memmap)
	MCFG_CPU_IO_MAP(homerun_iomap)
	MCFG_CPU_VBLANK_INT_DRIVER("screen", homerun_state,  irq0_line_hold)

	MCFG_DEVICE_ADD("ppi8255", I8255A, 0)
	MCFG_I8255_OUT_PORTA_CB(WRITE8(homerun_state, homerun_scrollhi_w))
	MCFG_I8255_OUT_PORTB_CB(WRITE8(homerun_state, homerun_scrolly_w))
	MCFG_I8255_OUT_PORTC_CB(WRITE8(homerun_state, homerun_scrollx_w))

	/* video hardware */
	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_REFRESH_RATE(60)
	MCFG_SCREEN_SIZE(256, 256)
	MCFG_SCREEN_VISIBLE_AREA(0*8, 32*8-1, 0*8, 30*8-1)
	MCFG_SCREEN_UPDATE_DRIVER(homerun_state, screen_update_homerun)
	MCFG_SCREEN_PALETTE("palette")

	MCFG_GFXDECODE_ADD("gfxdecode", "palette", homerun)
	MCFG_PALETTE_ADD("palette", 16*4)

	/* sound hardware */
	MCFG_SPEAKER_STANDARD_MONO("mono")

	MCFG_SOUND_ADD("ymsnd", YM2203, XTAL_20MHz/8)
	MCFG_AY8910_PORT_A_READ_CB(IOPORT("DSW"))
	MCFG_AY8910_PORT_B_WRITE_CB(WRITE8(homerun_state, homerun_banking_w))
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.50)
MACHINE_CONFIG_END

static MACHINE_CONFIG_DERIVED( homerun, dynashot )

	/* sound hardware */
	MCFG_SOUND_ADD("d7756", UPD7756, UPD7759_STANDARD_CLOCK)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.75)

	MCFG_SOUND_ADD("samples", SAMPLES, 0)
	MCFG_SAMPLES_CHANNELS(1)
	MCFG_SAMPLES_NAMES(homerun_sample_names)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.50)
MACHINE_CONFIG_END

static MACHINE_CONFIG_DERIVED( ganjaja, dynashot )

	/* basic machine hardware */
	MCFG_CPU_MODIFY("maincpu")
	MCFG_CPU_PERIODIC_INT_DRIVER(homerun_state, irq0_line_hold,  4*60) // ?

	/* sound hardware */
	MCFG_SOUND_ADD("d7756", UPD7756, UPD7759_STANDARD_CLOCK)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.75)
MACHINE_CONFIG_END



/**************************************************************************/

ROM_START( homerun )
	ROM_REGION( 0x30000, "maincpu", 0 )
	ROM_LOAD( "homerun.ic43",   0x00000, 0x04000, CRC(e759e476) SHA1(ad4f356ff26209033320a3e6353e4d4d9beb59c1) )
	ROM_CONTINUE(               0x10000, 0x1c000)

	ROM_REGION( 0x10000, "gfx1", 0 )
	ROM_LOAD( "homerun.ic60",   0x00000, 0x10000, CRC(69a720d1) SHA1(0f0a4877578f358e9e829ece8c31e23f01adcf83) )

	ROM_REGION( 0x20000, "gfx2", 0 )
	ROM_LOAD( "homerun.ic120",  0x00000, 0x20000, CRC(52f0709b) SHA1(19e675bcccadb774f60ec5929fc1fb5cf0d3f617) )

	ROM_REGION( 0x08000, "d7756", ROMREGION_ERASE00 )
	ROM_LOAD( "d7756c.ic98",    0x00000, 0x08000, NO_DUMP ) /* D7756C built-in rom - very likely the same rom as [Moero!! Pro Yakyuu (Black/Red)] on Famicom, and [Moero!! Nettou Yakyuu '88] on MSX2 */
ROM_END


ROM_START( dynashot )
	ROM_REGION( 0x30000, "maincpu", 0 )
	ROM_LOAD( "1.ic43",         0x00000, 0x04000, CRC(bf3c9586) SHA1(439effbda305f5fa265e5897c81dc1447e5d867d) )
	ROM_CONTINUE(               0x10000, 0x1c000)

	ROM_REGION( 0x10000, "gfx1", 0 )
	ROM_LOAD( "3.ic60",         0x00000, 0x10000, CRC(77d6a608) SHA1(a31ff343a5d4d6f20301c030ecc2e252149bcf9d) )

	ROM_REGION( 0x20000, "gfx2", 0 )
	ROM_LOAD( "2.ic120",        0x00000, 0x20000, CRC(bedf7b98) SHA1(cb6c5fcaf8df5f5c7636c3c8f79b9dda78e30c2e) )
ROM_END


ROM_START( ganjaja )
	ROM_REGION( 0x30000, "maincpu", 0 )
	ROM_LOAD( "1.ic43",         0x00000, 0x04000, CRC(dad57543) SHA1(dbd8b5cee33756ee5e3c41bf84c0f7141d3466dc) )
	ROM_CONTINUE(               0x10000, 0x1c000)

	ROM_REGION( 0x10000, "gfx1", 0 )
	ROM_LOAD( "ic60",           0x00000, 0x10000, CRC(855f6b28) SHA1(386411e88cf9bed54fe2073f0828d579cb1d04ee) )

	ROM_REGION( 0x20000, "gfx2", 0 )
	ROM_LOAD( "2.ic120",        0x00000, 0x20000, CRC(e65d4d57) SHA1(2ec9e5bdaa94b808573313b6eca657d798004b53) )

	ROM_REGION( 0x08000, "d7756", 0 )
	ROM_LOAD( "d77p56cr.ic98",  0x00000, 0x08000, CRC(06a234ac) SHA1(b4ceff3f9f78551cf4a085642e162e33b266f067) ) /* D77P56CR OTP rom (One-Time Programmable, note the extra P) */
ROM_END


GAME( 1988, homerun,  0, homerun,  homerun,  driver_device, 0, ROT0, "Jaleco", "Moero!! Pro Yakyuu Homerun Kyousou", MACHINE_IMPERFECT_GRAPHICS | MACHINE_IMPERFECT_SOUND | MACHINE_SUPPORTS_SAVE )
GAME( 1988, dynashot, 0, dynashot, dynashot, driver_device, 0, ROT0, "Jaleco", "Dynamic Shoot Kyousou", MACHINE_IMPERFECT_GRAPHICS | MACHINE_SUPPORTS_SAVE )
GAME( 1990, ganjaja,  0, ganjaja,  ganjaja,  driver_device, 0, ROT0, "Jaleco", "Ganbare Jajamaru Saisho wa Goo / Ganbare Jajamaru Hop Step & Jump", MACHINE_IMPERFECT_GRAPHICS | MACHINE_SUPPORTS_SAVE )
