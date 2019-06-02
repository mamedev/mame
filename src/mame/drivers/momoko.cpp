// license:BSD-3-Clause
// copyright-holders:Uki
/*****************************************************************************

    Momoko 120% (c) 1986 Jaleco

    Driver by Uki

    02/Mar/2001 -

******************************************************************************

Notes

Real machine has some bugs.(escalator bug, sprite garbage)
It is not emulation bug.
Flipped screen looks wrong, but it is correct.

Note that the game-breaking escalator bug only happens on an 8-way joystick,
it's safe to assume that this game dedicated cpanel was 4-way.


Stephh's notes (based on the game Z80 code and some tests) :

  - Accoding to the "initialisation" routine (code at 0x2a23),
    the "Bonus Life" Dip Switches shall be coded that way :

      PORT_DIPNAME( 0x03, 0x03, "1st Bonus Life" )
      PORT_DIPSETTING(    0x01, "20k" )
      PORT_DIPSETTING(    0x03, "30k" )
      PORT_DIPSETTING(    0x02, "50k" )
      PORT_DIPSETTING(    0x00, "100k" )
      PORT_DIPNAME( 0x0c, 0x0c, "2nd Bonus Life" )
      PORT_DIPSETTING(    0x04, "100k" )
      PORT_DIPSETTING(    0x08, "200k" )
      PORT_DIPSETTING(    0x00, "500k" )
      PORT_DIPSETTING(    0x0c, DEF_STR( None ) )

    But in the "check score for life" routine (code at 29c3),
    there is a 'ret' instruction (0xc9) instead of 'retc' (0xd8)
    that doesn't allow the player to get a 2nd bonus life !
  - DSW1 is read each frame (code at 0x2197), but result is
    completely discarded due to 'xor' instruction at 0x219a.
    I can't tell what this read is supposed to do.

*****************************************************************************/

#include "emu.h"
#include "includes/momoko.h"

#include "cpu/z80/z80.h"
#include "machine/gen_latch.h"
#include "machine/watchdog.h"
#include "sound/2203intf.h"
#include "screen.h"
#include "speaker.h"


WRITE8_MEMBER(momoko_state::momoko_bg_read_bank_w)
{
	membank("bank1")->set_entry(data & 0x1f);
}

/****************************************************************************/

void momoko_state::momoko_map(address_map &map)
{
	map(0x0000, 0xbfff).rom();
	map(0xc000, 0xcfff).ram();
	map(0xd064, 0xd0ff).ram().share("spriteram");
	map(0xd400, 0xd400).portr("IN0").nopw(); /* interrupt ack? */
	map(0xd402, 0xd402).portr("IN1").w(FUNC(momoko_state::momoko_flipscreen_w));
	map(0xd404, 0xd404).w("watchdog", FUNC(watchdog_timer_device::reset_w));
	map(0xd406, 0xd406).portr("DSW0").w("soundlatch", FUNC(generic_latch_8_device::write));
	map(0xd407, 0xd407).portr("DSW1");
	map(0xd800, 0xdbff).ram().w(m_palette, FUNC(palette_device::write8)).share("palette");
	map(0xdc00, 0xdc00).w(FUNC(momoko_state::momoko_fg_scrolly_w));
	map(0xdc01, 0xdc01).w(FUNC(momoko_state::momoko_fg_scrollx_w));
	map(0xdc02, 0xdc02).w(FUNC(momoko_state::momoko_fg_select_w));
	map(0xe000, 0xe3ff).ram().share("videoram");
	map(0xe800, 0xe800).w(FUNC(momoko_state::momoko_text_scrolly_w));
	map(0xe801, 0xe801).w(FUNC(momoko_state::momoko_text_mode_w));
	map(0xf000, 0xffff).bankr("bank1");
	map(0xf000, 0xf001).w(FUNC(momoko_state::momoko_bg_scrolly_w)).share("bg_scrolly");
	map(0xf002, 0xf003).w(FUNC(momoko_state::momoko_bg_scrollx_w)).share("bg_scrollx");
	map(0xf004, 0xf004).w(FUNC(momoko_state::momoko_bg_read_bank_w));
	map(0xf006, 0xf006).w(FUNC(momoko_state::momoko_bg_select_w));
	map(0xf007, 0xf007).w(FUNC(momoko_state::momoko_bg_priority_w));
}

void momoko_state::momoko_sound_map(address_map &map)
{
	map(0x0000, 0x7fff).rom();
	map(0x8000, 0x87ff).ram();
	map(0x9000, 0x9000).nopw(); /* unknown */
	map(0xa000, 0xa001).rw("ym1", FUNC(ym2203_device::read), FUNC(ym2203_device::write));
	map(0xb000, 0xb000).nopw(); /* unknown */
	map(0xc000, 0xc001).rw("ym2", FUNC(ym2203_device::read), FUNC(ym2203_device::write));
}


/****************************************************************************/

static INPUT_PORTS_START( momoko )
	PORT_START("IN0")
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_START2 )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON2 )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON1 )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT )  PORT_4WAY
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_4WAY
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN )  PORT_4WAY
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_UP )    PORT_4WAY

	PORT_START("IN1")
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_COCKTAIL
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_COCKTAIL
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT )  PORT_4WAY PORT_COCKTAIL
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_4WAY PORT_COCKTAIL
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN )  PORT_4WAY PORT_COCKTAIL
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_UP )    PORT_4WAY PORT_COCKTAIL

	PORT_START("DSW0")
	PORT_DIPNAME( 0x03, 0x03, DEF_STR( Lives ) )
	PORT_DIPSETTING(    0x03, "3" )
	PORT_DIPSETTING(    0x02, "4" )
	PORT_DIPSETTING(    0x01, "5" )
	PORT_DIPSETTING(    0x00, "255 (Cheat)" )
	PORT_DIPNAME( 0x1c, 0x1c, DEF_STR( Coinage ) )
	PORT_DIPSETTING(    0x10, DEF_STR( 5C_1C ) )
	PORT_DIPSETTING(    0x14, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(    0x18, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x1c, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x0c, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0x04, DEF_STR( 2C_5C ) )
	PORT_DIPSETTING(    0x08, DEF_STR( 1C_5C ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Free_Play ) )
	PORT_DIPNAME( 0x60, 0x60, DEF_STR( Difficulty ) )
	PORT_DIPSETTING(    0x40, DEF_STR( Easy ) )
	PORT_DIPSETTING(    0x60, DEF_STR( Normal ) )
	PORT_DIPSETTING(    0x20, DEF_STR( Hard ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Hardest ) )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_COIN1 )

	PORT_START("DSW1")
	PORT_DIPNAME( 0x03, 0x03, DEF_STR( Bonus_Life ) )       /* see notes */
	PORT_DIPSETTING(    0x01, "20k" )
	PORT_DIPSETTING(    0x03, "30k" )
	PORT_DIPSETTING(    0x02, "50k" )
	PORT_DIPSETTING(    0x00, "100k" )
	PORT_DIPUNUSED( 0x04, IP_ACTIVE_LOW )                   /* see notes */
	PORT_DIPUNUSED( 0x08, IP_ACTIVE_LOW )                   /* see notes */
	PORT_DIPNAME( 0x10, 0x00, DEF_STR( Cabinet ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Upright ) )
	PORT_DIPSETTING(    0x10, DEF_STR( Cocktail ) )
	PORT_DIPNAME( 0x20, 0x20, DEF_STR( Demo_Sounds ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x20, DEF_STR( On ) )
	PORT_DIPUNUSED( 0x40, IP_ACTIVE_LOW )
	PORT_DIPUNUSED( 0x80, IP_ACTIVE_LOW )

	PORT_START("FAKE")
	PORT_DIPNAME( 0x01, 0x00, DEF_STR( Flip_Screen ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x01, DEF_STR( On ) )
INPUT_PORTS_END

/****************************************************************************/

static const gfx_layout charlayout =
{
	8,8,    /* 8*8 characters */
	256,    /* 256 characters */
	2,      /* 2 bits per pixel */
	{4, 0},
	{0, 1, 2, 3, 256*8*8+0, 256*8*8+1, 256*8*8+2, 256*8*8+3},
	{8*0, 8*1, 8*2, 8*3, 8*4, 8*5, 8*6, 8*7},
	8*8
};

static const gfx_layout spritelayout =
{
	8,16,     /* 8*16 characters */
	2048-128, /* 1024 sprites ( ccc 0ccccccc ) */
	4,        /* 4 bits per pixel */
	{12,8,4,0},
	{0, 1, 2, 3, 4096*8+0, 4096*8+1, 4096*8+2, 4096*8+3},
	{0, 1*16, 2*16, 3*16, 4*16, 5*16, 6*16, 7*16,
		8*16, 9*16, 10*16, 11*16, 12*16, 13*16, 14*16, 15*16},
	8*32
};

static const gfx_layout tilelayout =
{
	8,8,      /* 8*8 characters */
	8192-256, /* 4096 tiles ( cccc0 cccccccc ) */
	4,        /* 4 bits per pixel */
	{4,0,12,8},
	{0, 1, 2, 3, 4096*8+0, 4096*8+1, 4096*8+2, 4096*8+3},
	{0, 1*16, 2*16, 3*16, 4*16, 5*16, 6*16, 7*16},
	8*16
};

static const gfx_layout charlayout1 =
{
	8,1,    /* 8*1 characters */
	256*8,  /* 2048 characters */
	2,      /* 2 bits per pixel */
	{4, 0},
	{0, 1, 2, 3, 256*8*8+0, 256*8*8+1, 256*8*8+2, 256*8*8+3},
	{8*0},
	8*1
};

static GFXDECODE_START( gfx_momoko )
	GFXDECODE_ENTRY( "gfx1", 0x0000, charlayout1,      0,  24 ) /* TEXT */
	GFXDECODE_ENTRY( "gfx2", 0x0000, tilelayout,     256,  16 ) /* BG */
	GFXDECODE_ENTRY( "gfx3", 0x0000, charlayout,       0,   1 ) /* FG */
	GFXDECODE_ENTRY( "gfx4", 0x0000, spritelayout,   128,   8 ) /* sprite */
GFXDECODE_END

/****************************************************************************/

void momoko_state::machine_start()
{
	uint8_t *BG_MAP = memregion("user1")->base();

	membank("bank1")->configure_entries(0, 32, &BG_MAP[0x0000], 0x1000);

	save_item(NAME(m_fg_scrollx));
	save_item(NAME(m_fg_scrolly));
	save_item(NAME(m_fg_select));
	save_item(NAME(m_text_scrolly));
	save_item(NAME(m_text_mode));
	save_item(NAME(m_bg_select));
	save_item(NAME(m_bg_priority));
	save_item(NAME(m_bg_mask));
	save_item(NAME(m_fg_mask));
	save_item(NAME(m_flipscreen));
}

void momoko_state::machine_reset()
{
	m_fg_scrollx = 0;
	m_fg_scrolly = 0;
	m_fg_select = 0;
	m_text_scrolly = 0;
	m_text_mode = 0;
	m_bg_select = 0;
	m_bg_priority = 0;
	m_bg_mask = 0;
	m_fg_mask = 0;
	m_flipscreen = 0;
}

void momoko_state::momoko(machine_config &config)
{
	/* basic machine hardware */
	Z80(config, m_maincpu, XTAL(10'000'000)/2);   /* 5.0MHz */
	m_maincpu->set_addrmap(AS_PROGRAM, &momoko_state::momoko_map);
	m_maincpu->set_vblank_int("screen", FUNC(momoko_state::irq0_line_hold));

	z80_device &audiocpu(Z80(config, "audiocpu", XTAL(10'000'000)/4));  /* 2.5MHz */
	audiocpu.set_addrmap(AS_PROGRAM, &momoko_state::momoko_sound_map);

	WATCHDOG_TIMER(config, "watchdog");

	/* video hardware */
	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_RASTER));
	screen.set_refresh_hz(60);
	screen.set_vblank_time(ATTOSECONDS_IN_USEC(2500) /* not accurate */);
	screen.set_size(32*8, 32*8);
	screen.set_visarea(1*8, 31*8-1, 2*8, 29*8-1);
	screen.set_screen_update(FUNC(momoko_state::screen_update_momoko));
	screen.set_palette(m_palette);

	GFXDECODE(config, m_gfxdecode, m_palette, gfx_momoko);
	PALETTE(config, m_palette).set_format(palette_device::xRGB_444, 512);
	m_palette->set_endianness(ENDIANNESS_BIG);

	/* sound hardware */
	SPEAKER(config, "mono").front_center();

	GENERIC_LATCH_8(config, "soundlatch");

	ym2203_device &ym1(YM2203(config, "ym1", XTAL(10'000'000)/8));
	ym1.add_route(0, "mono", 0.15);
	ym1.add_route(1, "mono", 0.15);
	ym1.add_route(2, "mono", 0.15);
	ym1.add_route(3, "mono", 0.40);

	ym2203_device &ym2(YM2203(config, "ym2", XTAL(10'000'000)/8));
	ym2.port_a_read_callback().set("soundlatch", FUNC(generic_latch_8_device::read));
	ym2.add_route(0, "mono", 0.15);
	ym2.add_route(1, "mono", 0.15);
	ym2.add_route(2, "mono", 0.15);
	ym2.add_route(3, "mono", 0.40);
}

/****************************************************************************/

ROM_START( momoko )
	ROM_REGION( 0x10000, "maincpu", 0 ) /* main CPU */
	ROM_LOAD( "momoko03.m6", 0x0000,  0x8000, CRC(386e26ed) SHA1(ad746ed1b87bafc5b4df9a28aade58cf894f4e7b) ) // age progression text in Japanese
	ROM_LOAD( "momoko02.m5", 0x8000,  0x4000, CRC(4255e351) SHA1(27a0e8d8aea223d2128139582e3b66106f3608ef) )

	ROM_REGION( 0x10000, "audiocpu", 0 ) /* sound CPU */
	ROM_LOAD( "momoko01.u4", 0x0000,  0x8000, CRC(e8a6673c) SHA1(f8984b063929305c9058801202405e6d45254b5b) )

	ROM_REGION( 0x2000, "gfx1", 0 ) /* text */
	ROM_LOAD( "momoko13.u4", 0x0000,  0x2000, CRC(2745cf5a) SHA1(3db7c6319cac63df1620ef25508c5c45eaa4b141) ) // On the FP-8631 PCB

	ROM_REGION( 0x2000, "gfx3", 0 ) /* FG */
	ROM_LOAD( "momoko14.p2", 0x0000,  0x2000, CRC(cfccca05) SHA1(4ecff488a37ac76ecb9ecf8980bea30dcc9c9951) )

	ROM_REGION( 0x10000, "gfx4", 0 ) /* sprite */
	ROM_LOAD16_BYTE( "momoko16.e5", 0x0000,  0x8000, CRC(fc6876fc) SHA1(b2d06bc01ef9f4db9bf8902d67f31ccbb0fea61a) ) // On the FP-8631 PCB
	ROM_LOAD16_BYTE( "momoko17.e6", 0x0001,  0x8000, CRC(45dc0247) SHA1(1b2bd4197ab7d237966e037c249b5bd623646c0b) ) // On the FP-8631 PCB

	ROM_REGION( 0x20000, "gfx2", 0 ) /* BG */
	ROM_LOAD16_BYTE( "momoko09.e8", 0x00000, 0x8000, CRC(9f5847c7) SHA1(6bc9a00622d8a23446294a8d5d467375c5719125) )
	ROM_LOAD16_BYTE( "momoko11.c8", 0x00001, 0x8000, CRC(9c9fbd43) SHA1(7adfd7ea3dd6745c14e719883f1a86e0a3b3c0ff) )
	ROM_LOAD16_BYTE( "momoko10.d8", 0x10000, 0x8000, CRC(ae17e74b) SHA1(f52657ea6b6ac518b70fd7b811d9699da27f67d9) )
	ROM_LOAD16_BYTE( "momoko12.a8", 0x10001, 0x8000, CRC(1e29c9c4) SHA1(d78f102cefc9852b529dd317a76c7003ec2ad3d5) )

	ROM_REGION( 0x20000, "user1", 0 ) /* BG map */
	ROM_LOAD( "momoko04.r8", 0x0000,  0x8000, CRC(3ab3c2c3) SHA1(d4a0d7f83bf64769e90a2c264c6114ac308cb8b5) )
	ROM_LOAD( "momoko05.p8", 0x8000,  0x8000, CRC(757cdd2b) SHA1(3471b42dc6458a18894dbd0638f4fe43c86dd70d) )
	ROM_LOAD( "momoko06.n8", 0x10000, 0x8000, CRC(20cacf8b) SHA1(e2b39abfc960e1c472e2bcf0cf06825c39941c03) )
	ROM_LOAD( "momoko07.l8", 0x18000, 0x8000, CRC(b94b38db) SHA1(9c9e45bbeca7b6b8b0051b144fb31fceaf5d6906) )

	ROM_REGION( 0x2000, "user2", 0 ) /* BG color/priority table */
	ROM_LOAD( "momoko08.h8", 0x0000,  0x2000, CRC(69b41702) SHA1(21b33b243dd6eaec8d41d9fd4d9e7faf2bd7f4d2) )

	ROM_REGION( 0x4000, "user3", 0 ) /* FG map */
	ROM_LOAD( "momoko15.k2", 0x0000,  0x4000, CRC(8028f806) SHA1(c7450d48803082f64af67fe752b6f49b71b6ff48) ) // On the FP-8631 PCB

	ROM_REGION( 0x0120, "proms", 0 ) /* TEXT color */
	ROM_LOAD( "momoko-c.bin", 0x0000,  0x0100, CRC(f35ccae0) SHA1(60b99dd3c96637dacba7e96a143b1a2d6ffd28b9) )
	ROM_LOAD( "momoko-b.bin", 0x0100,  0x0020, CRC(427b0e5c) SHA1(aa2797b899571527cc96013fd3420b841954ee67) )
ROM_END

ROM_START( momokoe )
	ROM_REGION( 0x10000, "maincpu", 0 ) /* main CPU */
	ROM_LOAD( "3.m6", 0x0000,  0x8000, CRC(84053a7d) SHA1(6e8fb22bb48954f4fed2530991ebe5b872c9c089) ) // age progression text in English
	ROM_LOAD( "2.m5", 0x8000,  0x4000, CRC(98ad397b) SHA1(b7ae218d0d397b1e258ec6d1f836cb998f984092) )

	ROM_REGION( 0x10000, "audiocpu", 0 ) /* sound CPU */
	ROM_LOAD( "momoko01.u4", 0x0000,  0x8000, CRC(e8a6673c) SHA1(f8984b063929305c9058801202405e6d45254b5b) )

	ROM_REGION( 0x2000, "gfx1", 0 ) /* text */
	ROM_LOAD( "momoko13.u4", 0x0000,  0x2000, CRC(2745cf5a) SHA1(3db7c6319cac63df1620ef25508c5c45eaa4b141) ) // On the FP-8631 PCB

	ROM_REGION( 0x2000, "gfx3", 0 ) /* FG */
	ROM_LOAD( "momoko14.p2", 0x0000,  0x2000, CRC(cfccca05) SHA1(4ecff488a37ac76ecb9ecf8980bea30dcc9c9951) )

	ROM_REGION( 0x10000, "gfx4", 0 ) /* sprite */
	ROM_LOAD16_BYTE( "momoko16.e5", 0x0000,  0x8000, CRC(fc6876fc) SHA1(b2d06bc01ef9f4db9bf8902d67f31ccbb0fea61a) ) // On the FP-8631 PCB
	ROM_LOAD16_BYTE( "momoko17.e6", 0x0001,  0x8000, CRC(45dc0247) SHA1(1b2bd4197ab7d237966e037c249b5bd623646c0b) ) // On the FP-8631 PCB

	ROM_REGION( 0x20000, "gfx2", 0 ) /* BG */
	ROM_LOAD16_BYTE( "momoko09.e8", 0x00000, 0x8000, CRC(9f5847c7) SHA1(6bc9a00622d8a23446294a8d5d467375c5719125) )
	ROM_LOAD16_BYTE( "momoko11.c8", 0x00001, 0x8000, CRC(9c9fbd43) SHA1(7adfd7ea3dd6745c14e719883f1a86e0a3b3c0ff) )
	ROM_LOAD16_BYTE( "momoko10.d8", 0x10000, 0x8000, CRC(ae17e74b) SHA1(f52657ea6b6ac518b70fd7b811d9699da27f67d9) )
	ROM_LOAD16_BYTE( "momoko12.a8", 0x10001, 0x8000, CRC(1e29c9c4) SHA1(d78f102cefc9852b529dd317a76c7003ec2ad3d5) )

	ROM_REGION( 0x20000, "user1", 0 ) /* BG map */
	ROM_LOAD( "momoko04.r8", 0x0000,  0x8000, CRC(3ab3c2c3) SHA1(d4a0d7f83bf64769e90a2c264c6114ac308cb8b5) )
	ROM_LOAD( "momoko05.p8", 0x8000,  0x8000, CRC(757cdd2b) SHA1(3471b42dc6458a18894dbd0638f4fe43c86dd70d) )
	ROM_LOAD( "momoko06.n8", 0x10000, 0x8000, CRC(20cacf8b) SHA1(e2b39abfc960e1c472e2bcf0cf06825c39941c03) )
	ROM_LOAD( "momoko07.l8", 0x18000, 0x8000, CRC(b94b38db) SHA1(9c9e45bbeca7b6b8b0051b144fb31fceaf5d6906) )

	ROM_REGION( 0x2000, "user2", 0 ) /* BG color/priority table */
	ROM_LOAD( "momoko08.h8", 0x0000,  0x2000, CRC(69b41702) SHA1(21b33b243dd6eaec8d41d9fd4d9e7faf2bd7f4d2) )

	ROM_REGION( 0x4000, "user3", 0 ) /* FG map */
	ROM_LOAD( "momoko15.k2", 0x0000,  0x4000, CRC(8028f806) SHA1(c7450d48803082f64af67fe752b6f49b71b6ff48) ) // On the FP-8631 PCB

	ROM_REGION( 0x0120, "proms", 0 ) /* TEXT color */
	ROM_LOAD( "momoko-c.bin", 0x0000,  0x0100, CRC(f35ccae0) SHA1(60b99dd3c96637dacba7e96a143b1a2d6ffd28b9) )
	ROM_LOAD( "momoko-b.bin", 0x0100,  0x0020, CRC(427b0e5c) SHA1(aa2797b899571527cc96013fd3420b841954ee67) )
ROM_END

ROM_START( momokob ) // bootleg board, almost exact copy of an original one
	ROM_REGION( 0x10000, "maincpu", 0 ) /* main CPU */
	ROM_LOAD( "3.bin", 0x0000,  0x8000, CRC(a18d7e78) SHA1(5d2dd498be3e22b5e8fc5ffe17e1ef463c1e9a02) ) // age progression text in Engrish, title screen in English
	ROM_LOAD( "2.bin", 0x8000,  0x4000, CRC(2dcf50ed) SHA1(6d02cb86fce031859bc0a5a26ecf7a8c8b89dea3) )

	ROM_REGION( 0x10000, "audiocpu", 0 ) /* sound CPU */
	ROM_LOAD( "momoko01.u4", 0x0000,  0x8000, CRC(e8a6673c) SHA1(f8984b063929305c9058801202405e6d45254b5b) )

	ROM_REGION( 0x2000, "gfx1", 0 ) /* text */
	ROM_LOAD( "momoko13.u4", 0x0000,  0x2000, CRC(2745cf5a) SHA1(3db7c6319cac63df1620ef25508c5c45eaa4b141) )

	ROM_REGION( 0x2000, "gfx3", 0 ) /* FG */
	ROM_LOAD( "momoko14.p2", 0x0000,  0x2000, CRC(cfccca05) SHA1(4ecff488a37ac76ecb9ecf8980bea30dcc9c9951) )

	ROM_REGION( 0x10000, "gfx4", 0 ) /* sprite */
	ROM_LOAD16_BYTE( "16.bin", 0x0000,  0x8000, CRC(49de49a1) SHA1(b4954286cba50332d4366a8160e9fbfd574c60ed) )
	ROM_LOAD16_BYTE( "17.bin", 0x0001,  0x8000, CRC(f06a3d1a) SHA1(f377ffad958fdc9cff2baee70ce4ba9080b5fe0d) )

	ROM_REGION( 0x20000, "gfx2", 0 ) /* BG */
	ROM_LOAD16_BYTE( "momoko09.e8", 0x00000, 0x8000, CRC(9f5847c7) SHA1(6bc9a00622d8a23446294a8d5d467375c5719125) )
	ROM_LOAD16_BYTE( "momoko11.c8", 0x00001, 0x8000, CRC(9c9fbd43) SHA1(7adfd7ea3dd6745c14e719883f1a86e0a3b3c0ff) )
	ROM_LOAD16_BYTE( "10.bin",      0x10000, 0x8000, CRC(68b9156d) SHA1(e157434d7ee33837ba35e720d221bf1eb21b7020) )
	ROM_LOAD16_BYTE( "12.bin",      0x10001, 0x8000, CRC(c32f5e19) SHA1(488da565e20bf002ff3dffca1efedbdf29e6e559) )

	ROM_REGION( 0x20000, "user1", 0 ) /* BG map */
	ROM_LOAD( "4.bin",       0x0000,  0x8000, CRC(1f0226d5) SHA1(6411e85c51e23dfe6c643692987dc7eeef37538f) )
	ROM_LOAD( "momoko05.p8", 0x8000,  0x8000, CRC(757cdd2b) SHA1(3471b42dc6458a18894dbd0638f4fe43c86dd70d) )
	ROM_LOAD( "momoko06.n8", 0x10000, 0x8000, CRC(20cacf8b) SHA1(e2b39abfc960e1c472e2bcf0cf06825c39941c03) )
	ROM_LOAD( "momoko07.l8", 0x18000, 0x8000, CRC(b94b38db) SHA1(9c9e45bbeca7b6b8b0051b144fb31fceaf5d6906) )

	ROM_REGION( 0x2000, "user2", 0 ) /* BG color/priority table */
	ROM_LOAD( "momoko08.h8", 0x0000,  0x2000, CRC(69b41702) SHA1(21b33b243dd6eaec8d41d9fd4d9e7faf2bd7f4d2) )

	ROM_REGION( 0x4000, "user3", 0 ) /* FG map */
	ROM_LOAD( "momoko15.k2", 0x0000,  0x4000, CRC(8028f806) SHA1(c7450d48803082f64af67fe752b6f49b71b6ff48) )

	ROM_REGION( 0x0120, "proms", 0 ) /* TEXT color */
	ROM_LOAD( "momoko-c.bin", 0x0000,  0x0100, CRC(f35ccae0) SHA1(60b99dd3c96637dacba7e96a143b1a2d6ffd28b9) )
	ROM_LOAD( "momoko-b.bin", 0x0100,  0x0020, CRC(427b0e5c) SHA1(aa2797b899571527cc96013fd3420b841954ee67) )
ROM_END

GAME( 1986, momoko,       0, momoko, momoko, momoko_state, empty_init, ROT0, "Jaleco",  "Momoko 120% (Japanese text)", MACHINE_SUPPORTS_SAVE )
GAME( 1986, momokoe, momoko, momoko, momoko, momoko_state, empty_init, ROT0, "Jaleco",  "Momoko 120% (English text)",  MACHINE_SUPPORTS_SAVE )
GAME( 1986, momokob, momoko, momoko, momoko, momoko_state, empty_init, ROT0, "bootleg", "Momoko 120% (bootleg)",       MACHINE_SUPPORTS_SAVE )
