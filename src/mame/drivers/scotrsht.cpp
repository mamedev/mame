// license:BSD-3-Clause
// copyright-holders:David Haywood, Pierpaolo Prazzoli
/***************************************************************************

 GX545 Scooter Shooter - (c) 1985 Konami

 It uses a mixed hardware based on Jailbreak and Iron Horse


Stephh's notes (based on the game M6502 code and some tests) :

  - There is a leftover from an unknown previous Konami game
    when you enter your initials (code at 0x43f2) :
    if DSW2 bit 2 is ON and DSW3 bit 1 is OFF, you ALWAYS
    have to use player 1 controls !
    Here is what you have for example in jailbrek.c driver :

      PORT_START("DSW2")
      ...
      PORT_DIPNAME( 0x04, 0x00, DEF_STR( Cabinet ) )
      PORT_DIPSETTING(    0x00, DEF_STR( Upright ) )
      PORT_DIPSETTING(    0x04, DEF_STR( Cocktail ) )
      ...
      PORT_START("DSW3")
      ...
      PORT_DIPNAME( 0x02, 0x02, "Upright Controls" )
      PORT_DIPSETTING(    0x02, DEF_STR( Single ) )
      PORT_DIPSETTING(    0x00, DEF_STR( Dual ) )

    This looks the same, doesn't it ? That's why I've named them
    "Dip must be OFF !" to avoid confusion and people changing them.
  - The "Free Play" is correct (press a START button if you aren't
    convinced), but no text is displayed to tell that to players.

***************************************************************************/

#include "emu.h"
#include "includes/scotrsht.h"
#include "includes/konamipt.h"

#include "cpu/z80/z80.h"
#include "cpu/m6809/m6809.h"
#include "machine/watchdog.h"
#include "sound/2203intf.h"
#include "screen.h"
#include "speaker.h"


WRITE8_MEMBER(scotrsht_state::ctrl_w)
{
	m_irq_enable = data & 0x02;
	flip_screen_set(data & 0x08);
}

WRITE_LINE_MEMBER(scotrsht_state::vblank_irq)
{
	if (state && m_irq_enable)
		m_maincpu->set_input_line(0, HOLD_LINE);
}

WRITE8_MEMBER(scotrsht_state::soundlatch_w)
{
	m_soundlatch->write(space, 0, data);
	m_audiocpu->set_input_line(0, HOLD_LINE);
}

void scotrsht_state::scotrsht_map(address_map &map)
{
	map(0x0000, 0x07ff).ram().w(FUNC(scotrsht_state::colorram_w)).share("colorram");
	map(0x0800, 0x0fff).ram().w(FUNC(scotrsht_state::videoram_w)).share("videoram");
	map(0x1000, 0x10bf).ram().share("spriteram"); /* sprites */
	map(0x10c0, 0x1fff).ram(); /* work ram */
	map(0x2000, 0x201f).ram().share("scroll"); /* scroll registers */
	map(0x2040, 0x2040).nopw();
	map(0x2041, 0x2041).nopw();
	map(0x2042, 0x2042).nopw();  /* it should be -> bit 2 = scroll direction like in jailbrek, but it's not used */
	map(0x2043, 0x2043).w(FUNC(scotrsht_state::charbank_w));
	map(0x2044, 0x2044).w(FUNC(scotrsht_state::ctrl_w));
	map(0x3000, 0x3000).w(FUNC(scotrsht_state::palettebank_w));
	map(0x3100, 0x3100).w(FUNC(scotrsht_state::soundlatch_w));
	map(0x3200, 0x3200).nopw(); /* it writes 0, 1 */
	map(0x3100, 0x3100).portr("DSW2");
	map(0x3200, 0x3200).portr("DSW3");
	map(0x3300, 0x3300).portr("SYSTEM");
	map(0x3301, 0x3301).portr("P1");
	map(0x3302, 0x3302).portr("P2");
	map(0x3303, 0x3303).portr("DSW1");
	map(0x3300, 0x3300).w("watchdog", FUNC(watchdog_timer_device::reset_w));
	map(0x4000, 0xffff).rom();
}

void scotrsht_state::scotrsht_sound_map(address_map &map)
{
	map(0x0000, 0x3fff).rom();
	map(0x4000, 0x43ff).ram();
	map(0x8000, 0x8000).r(m_soundlatch, FUNC(generic_latch_8_device::read));
}

void scotrsht_state::scotrsht_sound_port(address_map &map)
{
	map.global_mask(0xff);
	map(0x00, 0x01).rw("ymsnd", FUNC(ym2203_device::read), FUNC(ym2203_device::write));
}


static INPUT_PORTS_START( scotrsht )
	PORT_START("SYSTEM")    /* $3300 */
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_SERVICE1 )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_START2 )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("P1")        /* $3301 */
	KONAMI8_B1_UNK(1)

	PORT_START("P2")        /* $3302 */
	KONAMI8_B1_UNK(2)

	PORT_START("DSW1")      /* $3303 -> $196e */
	KONAMI_COINAGE_LOC(DEF_STR( Free_Play ), "Invalid", SW1)
	/* "Invalid" = both coin slots disabled */

	PORT_START("DSW2")      /* $3100 -> $196f */
	PORT_DIPNAME( 0x03, 0x02, DEF_STR( Lives ) )        PORT_DIPLOCATION("SW2:1,2")
	PORT_DIPSETTING(    0x03, "2" )
	PORT_DIPSETTING(    0x02, "3" )
	PORT_DIPSETTING(    0x01, "4" )
	PORT_DIPSETTING(    0x00, "5" )
	PORT_DIPNAME( 0x04, 0x04, "Dip MUST be OFF !" )     PORT_DIPLOCATION("SW2:3")   /* see notes */
	PORT_DIPSETTING(    0x04, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x08, DEF_STR( Bonus_Life ) )   PORT_DIPLOCATION("SW2:4")   /* code at 0x40f4 */
	PORT_DIPSETTING(    0x08, "30k 110k 80k+" )
	PORT_DIPSETTING(    0x00, "40k 120k 90k+" )
	PORT_DIPNAME( 0x30, 0x30, DEF_STR( Difficulty ) )   PORT_DIPLOCATION("SW2:5,6")
	PORT_DIPSETTING(    0x30, DEF_STR( Easy ) )
	PORT_DIPSETTING(    0x20, DEF_STR( Normal ) )
	PORT_DIPSETTING(    0x10, DEF_STR( Hard ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Hardest ) )
	PORT_DIPUNUSED_DIPLOC( 0x40, IP_ACTIVE_LOW, "SW2:7" )
	PORT_DIPNAME( 0x80, 0x00, DEF_STR( Demo_Sounds ) )  PORT_DIPLOCATION("SW2:8")
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

	PORT_START("DSW3")      /* $3200 -> $1970 */
	PORT_DIPNAME( 0x01, 0x01, DEF_STR( Flip_Screen ) )  PORT_DIPLOCATION("SW3:1")
	PORT_DIPSETTING(    0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x02, "Dip MUST be OFF !" )     PORT_DIPLOCATION("SW3:2")   /* see notes */
	PORT_DIPSETTING(    0x02, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPUNUSED_DIPLOC( 0x04, IP_ACTIVE_LOW, "SW3:3" )
	PORT_DIPUNUSED_DIPLOC( 0x08, IP_ACTIVE_LOW, "SW3:4" )
INPUT_PORTS_END


static const gfx_layout charlayout =
{
	8,8,    /* 8*8 characters */
	RGN_FRAC(1,1),  /* 1024 characters */
	4,  /* 4 bits per pixel */
	{ 0, 1, 2, 3 }, /* the four bitplanes are packed in one nibble */
	{ 0*4, 1*4, 2*4, 3*4, 4*4, 5*4, 6*4, 7*4 },
	{ 0*32, 1*32, 2*32, 3*32, 4*32, 5*32, 6*32, 7*32 },
	32*8    /* every char takes 32 consecutive bytes */
};

static const gfx_layout spritelayout =
{
	16,16,  /* 16*16 sprites */
	RGN_FRAC(1,1),  /* 512 sprites */
	4,  /* 4 bits per pixel */
	{ 0, 1, 2, 3 }, /* the bitplanes are packed in one nibble */
	{ 0*4, 1*4, 2*4, 3*4, 4*4, 5*4, 6*4, 7*4,
			32*8+0*4, 32*8+1*4, 32*8+2*4, 32*8+3*4, 32*8+4*4, 32*8+5*4, 32*8+6*4, 32*8+7*4 },
	{ 0*32, 1*32, 2*32, 3*32, 4*32, 5*32, 6*32, 7*32,
			16*32, 17*32, 18*32, 19*32, 20*32, 21*32, 22*32, 23*32 },
	128*8   /* every sprite takes 128 consecutive bytes */
};

static GFXDECODE_START( gfx_scotrsht )
	GFXDECODE_ENTRY( "gfx1", 0, charlayout,         0, 16*8 ) /* characters */
	GFXDECODE_ENTRY( "gfx2", 0, spritelayout, 16*16*8, 16*8 ) /* sprites */
GFXDECODE_END

void scotrsht_state::scotrsht(machine_config &config)
{
	/* basic machine hardware */
	MC6809E(config, m_maincpu, 18432000/6);        /* 3.072 MHz */
	m_maincpu->set_addrmap(AS_PROGRAM, &scotrsht_state::scotrsht_map);

	Z80(config, m_audiocpu, 18432000/6);        /* 3.072 MHz */
	m_audiocpu->set_addrmap(AS_PROGRAM, &scotrsht_state::scotrsht_sound_map);
	m_audiocpu->set_addrmap(AS_IO, &scotrsht_state::scotrsht_sound_port);

	WATCHDOG_TIMER(config, "watchdog");

	/* video hardware */
	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_RASTER));
	screen.set_refresh_hz(60);
	screen.set_vblank_time(ATTOSECONDS_IN_USEC(0));
	screen.set_size(32*8, 32*8);
	screen.set_visarea(0*8, 32*8-1, 2*8, 30*8-1);
	screen.set_screen_update(FUNC(scotrsht_state::screen_update));
	screen.set_palette(m_palette);
	screen.screen_vblank().set(FUNC(scotrsht_state::vblank_irq));

	GFXDECODE(config, m_gfxdecode, m_palette, gfx_scotrsht);
	PALETTE(config, m_palette, FUNC(scotrsht_state::scotrsht_palette), 16*8*16+16*8*16, 256);

	/* sound hardware */
	SPEAKER(config, "mono").front_center();

	GENERIC_LATCH_8(config, m_soundlatch);

	YM2203(config, "ymsnd", 18432000/6).add_route(ALL_OUTPUTS, "mono", 0.40);
}


/***************************************************************************

  Game driver(s)

***************************************************************************/

ROM_START( scotrsht )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "gx545_g03_12c.bin", 0x8000, 0x4000, CRC(b808e0d3) SHA1(d42b6979ade705a7522bd0bbc3eaa6d661580902) )
	ROM_CONTINUE(                  0x4000, 0x4000 )
	ROM_LOAD( "gx545_g02_10c.bin", 0xc000, 0x4000, CRC(b22c0586) SHA1(07c21609c6cdfe2b8dd734d21086c5236ff8197b) )

	ROM_REGION( 0x10000, "audiocpu", 0 )    /* 64k for sound code */
	ROM_LOAD( "gx545_g01_8c.bin",  0x0000, 0x4000, CRC(46a7cc65) SHA1(73389fe04ce40da124d630dc3f8e58600d9556fc) )

	ROM_REGION( 0x08000, "gfx1", 0 )
	ROM_LOAD( "gx545_g05_5f.bin",  0x0000, 0x8000, CRC(856c349c) SHA1(ba45e6d18e56cc7fc49c8fda190ec152ce6bd15c) )   /* characters */

	ROM_REGION( 0x10000, "gfx2", 0 )
	ROM_LOAD( "gx545_g06_6f.bin",  0x0000, 0x8000, CRC(14ad7601) SHA1(6dfcf2abfa2ea056c948d82d35c55f033f3e4678) )   /* sprites */
	ROM_LOAD( "gx545_h04_4f.bin",  0x8000, 0x8000, CRC(c06c11a3) SHA1(6e89c738498d716fd43d9cc7b71b23438bd3c4b8) )

	ROM_REGION( 0x0500, "proms", 0 )
	ROM_LOAD( "gx545_6301_1f.bin", 0x0000, 0x0100, CRC(f584586f) SHA1(0576cd0a738737c18143af887efd5ce76cdfc7cb) ) /* red */
	ROM_LOAD( "gx545_6301_2f.bin", 0x0100, 0x0100, CRC(ad464db1) SHA1(24937f2c9143e925c9becb488e11aa6daa807817) ) /* green */
	ROM_LOAD( "gx545_6301_3f.bin", 0x0200, 0x0100, CRC(bd475d23) SHA1(4ae6dfbb5c40a5ff97d7d80d0a441c1dc6dc5705) ) /* blue */
	ROM_LOAD( "gx545_6301_7f.bin", 0x0300, 0x0100, CRC(2b0cd233) SHA1(a2ccf693bf378ce8dd311c4224ad20de59418f88) ) /* char lookup */
	ROM_LOAD( "gx545_6301_8f.bin", 0x0400, 0x0100, CRC(c1c7cf58) SHA1(08452228bf13e43ce4a05806f79e9cd1542416f1) ) /* sprites lookup */
ROM_END

GAME( 1985, scotrsht, 0, scotrsht, scotrsht, scotrsht_state, empty_init, ROT90,"Konami", "Scooter Shooter", MACHINE_SUPPORTS_SAVE )
