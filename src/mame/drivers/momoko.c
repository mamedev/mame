/*****************************************************************************

Momoko 120% (c) 1986 Jaleco

    Driver by Uki

    02/Mar/2001 -

******************************************************************************

Notes

Real machine has some bugs.(escalator bug, sprite garbage)
It is not emulation bug.
Flipped screen looks wrong, but it is correct.

*****************************************************************************/

#include "driver.h"
#include "sound/2203intf.h"

extern UINT8 *momoko_bg_scrollx;
extern UINT8 *momoko_bg_scrolly;

VIDEO_UPDATE( momoko );

WRITE8_HANDLER( momoko_fg_scrollx_w );
WRITE8_HANDLER( momoko_fg_scrolly_w );
WRITE8_HANDLER( momoko_text_scrolly_w );
WRITE8_HANDLER( momoko_text_mode_w );
WRITE8_HANDLER( momoko_bg_scrollx_w );
WRITE8_HANDLER( momoko_bg_scrolly_w );
WRITE8_HANDLER( momoko_flipscreen_w );
WRITE8_HANDLER( momoko_fg_select_w);
WRITE8_HANDLER( momoko_bg_select_w);
WRITE8_HANDLER( momoko_bg_priority_w);

static WRITE8_HANDLER( momoko_bg_read_bank_w )
{
	UINT8 *BG_MAP = memory_region(machine, "user1");
	int bank_address = (data & 0x1f) * 0x1000;
	memory_set_bankptr(1, &BG_MAP[bank_address]);
}

/****************************************************************************/

static ADDRESS_MAP_START( readmem, ADDRESS_SPACE_PROGRAM, 8 )
	AM_RANGE(0x0000, 0xbfff) AM_READ(SMH_ROM)
	AM_RANGE(0xc000, 0xcfff) AM_READ(SMH_RAM)

	AM_RANGE(0xd064, 0xd0ff) AM_READ(SMH_RAM) /* sprite ram */

	AM_RANGE(0xd400, 0xd400) AM_READ_PORT("IN0")
	AM_RANGE(0xd402, 0xd402) AM_READ_PORT("IN1")
	AM_RANGE(0xd406, 0xd406) AM_READ_PORT("DSW0")
	AM_RANGE(0xd407, 0xd407) AM_READ_PORT("DSW1")

	AM_RANGE(0xd800, 0xdbff) AM_READ(SMH_RAM)
	AM_RANGE(0xe000, 0xe3ff) AM_READ(SMH_RAM) /* text */

	AM_RANGE(0xf000, 0xffff) AM_READ(SMH_BANK1)
ADDRESS_MAP_END

static ADDRESS_MAP_START( writemem, ADDRESS_SPACE_PROGRAM, 8 )
	AM_RANGE(0x0000, 0xbfff) AM_WRITE(SMH_ROM)
	AM_RANGE(0xc000, 0xcfff) AM_WRITE(SMH_RAM)

	AM_RANGE(0xd064, 0xd0ff) AM_WRITE(SMH_RAM) AM_BASE(&spriteram) AM_SIZE(&spriteram_size)

	AM_RANGE(0xd400, 0xd400) AM_WRITE(SMH_NOP) /* interrupt ack? */
	AM_RANGE(0xd402, 0xd402) AM_WRITE(momoko_flipscreen_w)
	AM_RANGE(0xd404, 0xd404) AM_WRITE(watchdog_reset_w)
	AM_RANGE(0xd406, 0xd406) AM_WRITE(soundlatch_w)

	AM_RANGE(0xd800, 0xdbff) AM_WRITE(paletteram_xxxxRRRRGGGGBBBB_be_w) AM_BASE(&paletteram)

	AM_RANGE(0xdc00, 0xdc00) AM_WRITE(momoko_fg_scrolly_w)
	AM_RANGE(0xdc01, 0xdc01) AM_WRITE(momoko_fg_scrollx_w)
	AM_RANGE(0xdc02, 0xdc02) AM_WRITE(momoko_fg_select_w)

	AM_RANGE(0xe000, 0xe3ff) AM_WRITE(SMH_RAM) AM_BASE(&videoram) AM_SIZE(&videoram_size)

	AM_RANGE(0xe800, 0xe800) AM_WRITE(momoko_text_scrolly_w)
	AM_RANGE(0xe801, 0xe801) AM_WRITE(momoko_text_mode_w)

	AM_RANGE(0xf000, 0xf001) AM_WRITE(momoko_bg_scrolly_w) AM_BASE(&momoko_bg_scrolly)
	AM_RANGE(0xf002, 0xf003) AM_WRITE(momoko_bg_scrollx_w) AM_BASE(&momoko_bg_scrollx)
	AM_RANGE(0xf004, 0xf004) AM_WRITE(momoko_bg_read_bank_w)
	AM_RANGE(0xf006, 0xf006) AM_WRITE(momoko_bg_select_w)
	AM_RANGE(0xf007, 0xf007) AM_WRITE(momoko_bg_priority_w)
ADDRESS_MAP_END

static ADDRESS_MAP_START( readmem_sound, ADDRESS_SPACE_PROGRAM, 8 )
	AM_RANGE(0x0000, 0x7fff) AM_READ(SMH_ROM)
	AM_RANGE(0x8000, 0x87ff) AM_READ(SMH_RAM)
	AM_RANGE(0xa000, 0xa000) AM_READ(YM2203_status_port_0_r)
	AM_RANGE(0xa001, 0xa001) AM_READ(YM2203_read_port_0_r)
	AM_RANGE(0xc000, 0xc000) AM_READ(YM2203_status_port_1_r)
	AM_RANGE(0xc001, 0xc001) AM_READ(YM2203_read_port_1_r)
ADDRESS_MAP_END

static ADDRESS_MAP_START( writemem_sound, ADDRESS_SPACE_PROGRAM, 8 )
	AM_RANGE(0x0000, 0x7fff) AM_WRITE(SMH_ROM)
	AM_RANGE(0x8000, 0x87ff) AM_WRITE(SMH_RAM)
	AM_RANGE(0x9000, 0x9000) AM_WRITE(SMH_NOP) /* unknown */
	AM_RANGE(0xa000, 0xa000) AM_WRITE(YM2203_control_port_0_w)
	AM_RANGE(0xa001, 0xa001) AM_WRITE(YM2203_write_port_0_w)
	AM_RANGE(0xb000, 0xb000) AM_WRITE(SMH_NOP) /* unknown */
	AM_RANGE(0xc000, 0xc000) AM_WRITE(YM2203_control_port_1_w)
	AM_RANGE(0xc001, 0xc001) AM_WRITE(YM2203_write_port_1_w)
ADDRESS_MAP_END

/****************************************************************************/

static INPUT_PORTS_START( momoko )
    PORT_START_TAG("IN0")
    PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_START2 )
    PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_START1 )
    PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON2 )
    PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON1 )
    PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY
    PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY
    PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY
    PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY

    PORT_START_TAG("IN1")
    PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )
    PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
    PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_COCKTAIL
    PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_COCKTAIL
    PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY PORT_COCKTAIL
    PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_COCKTAIL
    PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY PORT_COCKTAIL
    PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY PORT_COCKTAIL

    PORT_START_TAG("DSW0")
    PORT_DIPNAME( 0x03, 0x03, DEF_STR( Lives ) )
    PORT_DIPSETTING(    0x03, "3" )
    PORT_DIPSETTING(    0x02, "4" )
    PORT_DIPSETTING(    0x01, "5" )
    PORT_DIPSETTING(    0x00, "256")
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
    PORT_DIPSETTING(    0x20, "Difficult" )
    PORT_DIPSETTING(    0x00, "Very difficult" )
    PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_COIN1 )

    PORT_START_TAG("DSW1")
    PORT_DIPNAME( 0x03, 0x03, DEF_STR( Bonus_Life) )
    PORT_DIPSETTING(    0x01, "20000" )
    PORT_DIPSETTING(    0x03, "30000" )
    PORT_DIPSETTING(    0x02, "50000" )
    PORT_DIPSETTING(    0x00, "100000" )
    PORT_DIPNAME( 0x04, 0x04, DEF_STR( Unknown) )
    PORT_DIPSETTING(    0x04, DEF_STR( Off ) )
    PORT_DIPSETTING(    0x00, DEF_STR( On ) )
    PORT_DIPNAME( 0x08, 0x08, DEF_STR( Unknown) )
    PORT_DIPSETTING(    0x08, DEF_STR( Off ) )
    PORT_DIPSETTING(    0x00, DEF_STR( On ) )
    PORT_DIPNAME( 0x10, 0x00, DEF_STR( Cabinet ) )
    PORT_DIPSETTING(    0x00, DEF_STR( Upright ) )
    PORT_DIPSETTING(    0x10, DEF_STR( Cocktail ) )
    PORT_DIPNAME( 0x20, 0x20, DEF_STR( Demo_Sounds ) )
    PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
    PORT_DIPSETTING(    0x20, DEF_STR( On ) )
    PORT_DIPNAME( 0x40, 0x40, DEF_STR( Unknown ) )
    PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
    PORT_DIPSETTING(    0x00, DEF_STR( On ) )
    PORT_DIPNAME( 0x80, 0x80, DEF_STR( Unknown ) )
    PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
    PORT_DIPSETTING(    0x00, DEF_STR( On ) )

    PORT_START_TAG("FAKE")
    PORT_DIPNAME( 0x01, 0x00, DEF_STR( Flip_Screen ) )
    PORT_DIPSETTING(	0x00, DEF_STR( Off ) )
    PORT_DIPSETTING(	0x01, DEF_STR( On ) )

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

static const struct YM2203interface ym2203_interface =
{
	{
		AY8910_LEGACY_OUTPUT,
		AY8910_DEFAULT_LOADS,
		soundlatch_r,
		NULL,
		NULL,
		NULL
	},
	NULL
};

static MACHINE_DRIVER_START( momoko )

	/* basic machine hardware */
	MDRV_CPU_ADD("main", Z80, 5000000)	/* 5.0MHz */
	MDRV_CPU_PROGRAM_MAP(readmem,writemem)
	MDRV_CPU_VBLANK_INT("main", irq0_line_hold)

	MDRV_CPU_ADD("audio", Z80, 2500000)	/* 2.5MHz */
	MDRV_CPU_PROGRAM_MAP(readmem_sound,writemem_sound)

	/* video hardware */
	MDRV_SCREEN_ADD("main", RASTER)
	MDRV_SCREEN_REFRESH_RATE(60)
	MDRV_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(2500) /* not accurate */)
	MDRV_SCREEN_FORMAT(BITMAP_FORMAT_INDEXED16)
	MDRV_SCREEN_SIZE(32*8, 32*8)
	MDRV_SCREEN_VISIBLE_AREA(1*8, 31*8-1, 2*8, 29*8-1)

	MDRV_GFXDECODE(momoko)
	MDRV_PALETTE_LENGTH(512)

	MDRV_VIDEO_UPDATE(momoko)

	/* sound hardware */
	MDRV_SPEAKER_STANDARD_MONO("mono")

	MDRV_SOUND_ADD("ym1", YM2203, 1250000)
	MDRV_SOUND_ROUTE(0, "mono", 0.15)
	MDRV_SOUND_ROUTE(1, "mono", 0.15)
	MDRV_SOUND_ROUTE(2, "mono", 0.15)
	MDRV_SOUND_ROUTE(3, "mono", 0.40)

	MDRV_SOUND_ADD("ym2", YM2203, 1250000)
	MDRV_SOUND_CONFIG(ym2203_interface)
	MDRV_SOUND_ROUTE(0, "mono", 0.15)
	MDRV_SOUND_ROUTE(1, "mono", 0.15)
	MDRV_SOUND_ROUTE(2, "mono", 0.15)
	MDRV_SOUND_ROUTE(3, "mono", 0.40)
MACHINE_DRIVER_END

/****************************************************************************/

ROM_START( momoko )
	ROM_REGION( 0x10000, "main", 0 ) /* main CPU */
	ROM_LOAD( "momoko03.bin", 0x0000,  0x8000, CRC(386e26ed) SHA1(ad746ed1b87bafc5b4df9a28aade58cf894f4e7b) )
	ROM_LOAD( "momoko02.bin", 0x8000,  0x4000, CRC(4255e351) SHA1(27a0e8d8aea223d2128139582e3b66106f3608ef) )

	ROM_REGION( 0x10000, "audio", 0 ) /* sound CPU */
	ROM_LOAD( "momoko01.bin", 0x0000,  0x8000, CRC(e8a6673c) SHA1(f8984b063929305c9058801202405e6d45254b5b) )

	ROM_REGION( 0x2000, "gfx1", ROMREGION_DISPOSE ) /* text */
	ROM_LOAD( "momoko13.bin", 0x0000,  0x2000, CRC(2745cf5a) SHA1(3db7c6319cac63df1620ef25508c5c45eaa4b141) )

	ROM_REGION( 0x2000, "gfx3", ROMREGION_DISPOSE ) /* FG */
	ROM_LOAD( "momoko14.bin", 0x0000,  0x2000, CRC(cfccca05) SHA1(4ecff488a37ac76ecb9ecf8980bea30dcc9c9951) )

	ROM_REGION( 0x10000, "gfx4", ROMREGION_DISPOSE ) /* sprite */
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

GAME( 1986, momoko, 0, momoko, momoko, 0, ROT0, "Jaleco", "Momoko 120%", 0 )
