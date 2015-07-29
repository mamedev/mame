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
#include "cpu/z80/z80.h"
#include "sound/2203intf.h"
#include "includes/momoko.h"


WRITE8_MEMBER(momoko_state::momoko_bg_read_bank_w)
{
	membank("bank1")->set_entry(data & 0x1f);
}

/****************************************************************************/

static ADDRESS_MAP_START( momoko_map, AS_PROGRAM, 8, momoko_state )
	AM_RANGE(0x0000, 0xbfff) AM_ROM
	AM_RANGE(0xc000, 0xcfff) AM_RAM
	AM_RANGE(0xd064, 0xd0ff) AM_RAM AM_SHARE("spriteram")
	AM_RANGE(0xd400, 0xd400) AM_READ_PORT("IN0") AM_WRITENOP /* interrupt ack? */
	AM_RANGE(0xd402, 0xd402) AM_READ_PORT("IN1") AM_WRITE(momoko_flipscreen_w)
	AM_RANGE(0xd404, 0xd404) AM_WRITE(watchdog_reset_w)
	AM_RANGE(0xd406, 0xd406) AM_READ_PORT("DSW0") AM_WRITE(soundlatch_byte_w)
	AM_RANGE(0xd407, 0xd407) AM_READ_PORT("DSW1")
	AM_RANGE(0xd800, 0xdbff) AM_RAM_DEVWRITE("palette", palette_device, write) AM_SHARE("palette")
	AM_RANGE(0xdc00, 0xdc00) AM_WRITE(momoko_fg_scrolly_w)
	AM_RANGE(0xdc01, 0xdc01) AM_WRITE(momoko_fg_scrollx_w)
	AM_RANGE(0xdc02, 0xdc02) AM_WRITE(momoko_fg_select_w)
	AM_RANGE(0xe000, 0xe3ff) AM_RAM AM_SHARE("videoram")
	AM_RANGE(0xe800, 0xe800) AM_WRITE(momoko_text_scrolly_w)
	AM_RANGE(0xe801, 0xe801) AM_WRITE(momoko_text_mode_w)
	AM_RANGE(0xf000, 0xffff) AM_ROMBANK("bank1")
	AM_RANGE(0xf000, 0xf001) AM_WRITE(momoko_bg_scrolly_w) AM_SHARE("bg_scrolly")
	AM_RANGE(0xf002, 0xf003) AM_WRITE(momoko_bg_scrollx_w) AM_SHARE("bg_scrollx")
	AM_RANGE(0xf004, 0xf004) AM_WRITE(momoko_bg_read_bank_w)
	AM_RANGE(0xf006, 0xf006) AM_WRITE(momoko_bg_select_w)
	AM_RANGE(0xf007, 0xf007) AM_WRITE(momoko_bg_priority_w)
ADDRESS_MAP_END

static ADDRESS_MAP_START( momoko_sound_map, AS_PROGRAM, 8, momoko_state )
	AM_RANGE(0x0000, 0x7fff) AM_ROM
	AM_RANGE(0x8000, 0x87ff) AM_RAM
	AM_RANGE(0x9000, 0x9000) AM_WRITENOP /* unknown */
	AM_RANGE(0xa000, 0xa001) AM_DEVREADWRITE("ym1", ym2203_device, read, write)
	AM_RANGE(0xb000, 0xb000) AM_WRITENOP /* unknown */
	AM_RANGE(0xc000, 0xc001) AM_DEVREADWRITE("ym2", ym2203_device, read, write)
ADDRESS_MAP_END


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

static GFXDECODE_START( momoko )
	GFXDECODE_ENTRY( "gfx1", 0x0000, charlayout1,      0,  24 ) /* TEXT */
	GFXDECODE_ENTRY( "gfx2", 0x0000, tilelayout,     256,  16 ) /* BG */
	GFXDECODE_ENTRY( "gfx3", 0x0000, charlayout,       0,   1 ) /* FG */
	GFXDECODE_ENTRY( "gfx4", 0x0000, spritelayout,   128,   8 ) /* sprite */
GFXDECODE_END

/****************************************************************************/

void momoko_state::machine_start()
{
	UINT8 *BG_MAP = memregion("user1")->base();

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

static MACHINE_CONFIG_START( momoko, momoko_state )

	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", Z80, 5000000)   /* 5.0MHz */
	MCFG_CPU_PROGRAM_MAP(momoko_map)
	MCFG_CPU_VBLANK_INT_DRIVER("screen", momoko_state,  irq0_line_hold)

	MCFG_CPU_ADD("audiocpu", Z80, 2500000)  /* 2.5MHz */
	MCFG_CPU_PROGRAM_MAP(momoko_sound_map)

	/* video hardware */
	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_REFRESH_RATE(60)
	MCFG_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(2500) /* not accurate */)
	MCFG_SCREEN_SIZE(32*8, 32*8)
	MCFG_SCREEN_VISIBLE_AREA(1*8, 31*8-1, 2*8, 29*8-1)
	MCFG_SCREEN_UPDATE_DRIVER(momoko_state, screen_update_momoko)
	MCFG_SCREEN_PALETTE("palette")

	MCFG_GFXDECODE_ADD("gfxdecode", "palette", momoko)
	MCFG_PALETTE_ADD("palette", 512)
	MCFG_PALETTE_FORMAT(xxxxRRRRGGGGBBBB)
	MCFG_PALETTE_ENDIANNESS(ENDIANNESS_BIG)

	/* sound hardware */
	MCFG_SPEAKER_STANDARD_MONO("mono")

	MCFG_SOUND_ADD("ym1", YM2203, 1250000)
	MCFG_SOUND_ROUTE(0, "mono", 0.15)
	MCFG_SOUND_ROUTE(1, "mono", 0.15)
	MCFG_SOUND_ROUTE(2, "mono", 0.15)
	MCFG_SOUND_ROUTE(3, "mono", 0.40)

	MCFG_SOUND_ADD("ym2", YM2203, 1250000)
	MCFG_AY8910_PORT_A_READ_CB(READ8(driver_device, soundlatch_byte_r))
	MCFG_SOUND_ROUTE(0, "mono", 0.15)
	MCFG_SOUND_ROUTE(1, "mono", 0.15)
	MCFG_SOUND_ROUTE(2, "mono", 0.15)
	MCFG_SOUND_ROUTE(3, "mono", 0.40)
MACHINE_CONFIG_END

/****************************************************************************/

ROM_START( momoko )
	ROM_REGION( 0x10000, "maincpu", 0 ) /* main CPU */
	ROM_LOAD( "momoko03.bin", 0x0000,  0x8000, CRC(386e26ed) SHA1(ad746ed1b87bafc5b4df9a28aade58cf894f4e7b) )
	ROM_LOAD( "momoko02.bin", 0x8000,  0x4000, CRC(4255e351) SHA1(27a0e8d8aea223d2128139582e3b66106f3608ef) )

	ROM_REGION( 0x10000, "audiocpu", 0 ) /* sound CPU */
	ROM_LOAD( "momoko01.bin", 0x0000,  0x8000, CRC(e8a6673c) SHA1(f8984b063929305c9058801202405e6d45254b5b) )

	ROM_REGION( 0x2000, "gfx1", 0 ) /* text */
	ROM_LOAD( "momoko13.bin", 0x0000,  0x2000, CRC(2745cf5a) SHA1(3db7c6319cac63df1620ef25508c5c45eaa4b141) )

	ROM_REGION( 0x2000, "gfx3", 0 ) /* FG */
	ROM_LOAD( "momoko14.bin", 0x0000,  0x2000, CRC(cfccca05) SHA1(4ecff488a37ac76ecb9ecf8980bea30dcc9c9951) )

	ROM_REGION( 0x10000, "gfx4", 0 ) /* sprite */
	ROM_LOAD16_BYTE( "momoko16.bin", 0x0000,  0x8000, CRC(fc6876fc) SHA1(b2d06bc01ef9f4db9bf8902d67f31ccbb0fea61a) )
	ROM_LOAD16_BYTE( "momoko17.bin", 0x0001,  0x8000, CRC(45dc0247) SHA1(1b2bd4197ab7d237966e037c249b5bd623646c0b) )

	ROM_REGION( 0x20000, "gfx2", 0 ) /* BG */
	ROM_LOAD16_BYTE( "momoko09.bin", 0x00000, 0x8000, CRC(9f5847c7) SHA1(6bc9a00622d8a23446294a8d5d467375c5719125) )
	ROM_LOAD16_BYTE( "momoko11.bin", 0x00001, 0x8000, CRC(9c9fbd43) SHA1(7adfd7ea3dd6745c14e719883f1a86e0a3b3c0ff) )
	ROM_LOAD16_BYTE( "momoko10.bin", 0x10000, 0x8000, CRC(ae17e74b) SHA1(f52657ea6b6ac518b70fd7b811d9699da27f67d9) )
	ROM_LOAD16_BYTE( "momoko12.bin", 0x10001, 0x8000, CRC(1e29c9c4) SHA1(d78f102cefc9852b529dd317a76c7003ec2ad3d5) )

	ROM_REGION( 0x20000, "user1", 0 ) /* BG map */
	ROM_LOAD( "momoko04.bin", 0x0000,  0x8000, CRC(3ab3c2c3) SHA1(d4a0d7f83bf64769e90a2c264c6114ac308cb8b5) )
	ROM_LOAD( "momoko05.bin", 0x8000,  0x8000, CRC(757cdd2b) SHA1(3471b42dc6458a18894dbd0638f4fe43c86dd70d) )
	ROM_LOAD( "momoko06.bin", 0x10000, 0x8000, CRC(20cacf8b) SHA1(e2b39abfc960e1c472e2bcf0cf06825c39941c03) )
	ROM_LOAD( "momoko07.bin", 0x18000, 0x8000, CRC(b94b38db) SHA1(9c9e45bbeca7b6b8b0051b144fb31fceaf5d6906) )

	ROM_REGION( 0x2000, "user2", 0 ) /* BG color/priority table */
	ROM_LOAD( "momoko08.bin", 0x0000,  0x2000, CRC(69b41702) SHA1(21b33b243dd6eaec8d41d9fd4d9e7faf2bd7f4d2) )

	ROM_REGION( 0x4000, "user3", 0 ) /* FG map */
	ROM_LOAD( "momoko15.bin", 0x0000,  0x4000, CRC(8028f806) SHA1(c7450d48803082f64af67fe752b6f49b71b6ff48) )

	ROM_REGION( 0x0120, "proms", 0 ) /* TEXT color */
	ROM_LOAD( "momoko-c.bin", 0x0000,  0x0100, CRC(f35ccae0) SHA1(60b99dd3c96637dacba7e96a143b1a2d6ffd28b9) )
	ROM_LOAD( "momoko-b.bin", 0x0100,  0x0020, CRC(427b0e5c) SHA1(aa2797b899571527cc96013fd3420b841954ee67) )
ROM_END

GAME( 1986, momoko, 0, momoko, momoko, driver_device, 0, ROT0, "Jaleco", "Momoko 120%", MACHINE_SUPPORTS_SAVE )
