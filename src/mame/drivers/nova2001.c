/*******************************************************************************

     Nova 2001 - by UPL - 1983

     driver by Howie Cohen, Frank Palazzolo, Alex Pasadyn

     Memory Map:

     Address Range:     R/W:     Function:
     --------------------------------------------------------------------------
     0000 - 7fff        R        Program ROM (7000-7fff mirror of 6000-6fff)
     a000 - a3ff        R/W      Foreground Playfield character RAM
     a400 - a7ff        R/W      Foreground Playfield color modifier RAM
     a800 - abff        R/W      Scrolling Playfield character RAM
     ac00 - a7ff        R/W      Scrolling Playfield color modifier RAM
     b000 - b7ff        R/W      Sprite RAM
     bfff               W        flip screen
     c000               R/W      AY8910 #1 Data R/W
     c001               R/W      AY8910 #2 Data R/W
     c002               W        AY8910 #1 Control W
     c003               W        AY8910 #2 Control W
     c004               R        Interrupt acknowledge / Watchdog reset
     c006               R        Player 1 Controls
     c007               R        Player 2 Controls
     c00e               R        Coin Inputs, etc.
     e000 - e7ff        R/W      Work RAM

 the parent set is VERY sensitive to coin inputs, if the coin isn't held down
 long enough, or is held down too long the game will reset, likewise if coins
 are inserted too quickly

*******************************************************************************/

#include "driver.h"
#include "sound/ay8910.h"

/* From video/nova2001.c */
extern UINT8 *nova2001_videoram2, *nova2001_colorram2;

extern WRITE8_HANDLER( nova2001_videoram_w );
extern WRITE8_HANDLER( nova2001_colorram_w );
extern WRITE8_HANDLER( nova2001_videoram2_w );
extern WRITE8_HANDLER( nova2001_colorram2_w );
extern WRITE8_HANDLER( nova2001_scroll_x_w );
extern WRITE8_HANDLER( nova2001_scroll_y_w );
extern WRITE8_HANDLER( nova2001_flipscreen_w );

extern PALETTE_INIT( nova2001 );
extern VIDEO_START( nova2001 );
extern VIDEO_UPDATE( nova2001 );



static ADDRESS_MAP_START( nova2001_map, ADDRESS_SPACE_PROGRAM, 8 )
	AM_RANGE(0x0000, 0x7fff) AM_ROM
	AM_RANGE(0xa000, 0xa3ff) AM_RAM AM_WRITE(nova2001_videoram2_w) AM_BASE(&nova2001_videoram2)
	AM_RANGE(0xa400, 0xa7ff) AM_RAM AM_WRITE(nova2001_colorram2_w) AM_BASE(&nova2001_colorram2)
	AM_RANGE(0xa800, 0xabff) AM_RAM AM_WRITE(nova2001_videoram_w) AM_BASE(&videoram)
	AM_RANGE(0xac00, 0xafff) AM_RAM AM_WRITE(nova2001_colorram_w) AM_BASE(&colorram)
	AM_RANGE(0xb000, 0xb7ff) AM_RAM AM_BASE(&spriteram) AM_SIZE(&spriteram_size)
	AM_RANGE(0xbfff, 0xbfff) AM_WRITE(nova2001_flipscreen_w)
	AM_RANGE(0xc000, 0xc000) AM_READWRITE(AY8910_read_port_0_r, AY8910_write_port_0_w)
	AM_RANGE(0xc001, 0xc001) AM_READWRITE(AY8910_read_port_1_r, AY8910_write_port_1_w)
	AM_RANGE(0xc002, 0xc002) AM_WRITE(AY8910_control_port_0_w)
	AM_RANGE(0xc003, 0xc003) AM_WRITE(AY8910_control_port_1_w)
	AM_RANGE(0xc004, 0xc004) AM_READ(watchdog_reset_r)
	AM_RANGE(0xc006, 0xc006) AM_READ(input_port_0_r)
	AM_RANGE(0xc007, 0xc007) AM_READ(input_port_1_r)
	AM_RANGE(0xc00e, 0xc00e) AM_READ(input_port_2_r)
	AM_RANGE(0xe000, 0xe7ff) AM_RAM
ADDRESS_MAP_END



static INPUT_PORTS_START( nova2001 )
    PORT_START
    PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY
    PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY
    PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY
    PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY
    PORT_BIT( 0x30, IP_ACTIVE_LOW, IPT_UNUSED )
    PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_BUTTON2 )	// pause
    PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_BUTTON1 )	// fire

    PORT_START
    PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY PORT_COCKTAIL
    PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY PORT_COCKTAIL
    PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY PORT_COCKTAIL
    PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_COCKTAIL
    PORT_BIT( 0x30, IP_ACTIVE_LOW, IPT_UNUSED )
    PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_COCKTAIL
    PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_COCKTAIL

    PORT_START
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_COIN1 ) PORT_IMPULSE(4)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_START1 )
    PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_START2 )
    PORT_BIT( 0x78, IP_ACTIVE_LOW, IPT_UNUSED )
    PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_VBLANK )

    PORT_START  /* dsw0 */
    PORT_DIPNAME( 0x01, 0x00, DEF_STR( Cabinet ) )
    PORT_DIPSETTING(    0x00, DEF_STR( Upright ) )
    PORT_DIPSETTING(    0x01, DEF_STR( Cocktail ) )
    PORT_DIPNAME( 0x02, 0x02, DEF_STR( Lives ) )
    PORT_DIPSETTING(    0x02, "3" )
    PORT_DIPSETTING(    0x00, "4" )
    PORT_DIPNAME( 0x04, 0x04, "1st Bonus Life" )
    PORT_DIPSETTING(    0x04, "20K" )
    PORT_DIPSETTING(    0x00, "30K" )
    PORT_DIPNAME( 0x18, 0x18, "Extra Bonus Life" )
    PORT_DIPSETTING(    0x18, "60K" )
    PORT_DIPSETTING(    0x10, "70K" )
    PORT_DIPSETTING(    0x08, "90K" )
    PORT_DIPSETTING(    0x00, DEF_STR( None ) )
    PORT_DIPNAME( 0x60, 0x60, DEF_STR( Coinage ) )
    PORT_DIPSETTING(    0x40, DEF_STR( 2C_1C ) )
    PORT_DIPSETTING(    0x00, DEF_STR( 2C_2C ) )
    PORT_DIPSETTING(    0x60, DEF_STR( 1C_1C ) )
    PORT_DIPSETTING(    0x20, DEF_STR( 1C_2C ) )
    PORT_DIPNAME( 0x80, 0x80, DEF_STR( Free_Play ) )
    PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
    PORT_DIPSETTING(    0x00, DEF_STR( On ) )

    PORT_START  /* dsw1 */
    PORT_DIPNAME( 0x03, 0x00, DEF_STR( Difficulty ) )
    PORT_DIPSETTING(    0x00, DEF_STR( Easy ) )
    PORT_DIPSETTING(    0x03, DEF_STR( Medium ) )
    PORT_DIPSETTING(    0x02, DEF_STR( Hard ) )
    PORT_DIPSETTING(    0x01, DEF_STR( Hardest ) )
    PORT_DIPNAME( 0x04, 0x00, DEF_STR( Demo_Sounds ) )
    PORT_DIPSETTING(    0x04, DEF_STR( Off ) )
    PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x08, "High Score Names" )
	PORT_DIPSETTING(    0x00, "3 Letters" )
	PORT_DIPSETTING(    0x08, "8 Letters" )
    PORT_BIT( 0x70, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_SERVICE( 0x80, IP_ACTIVE_LOW )
INPUT_PORTS_END



static const gfx_layout charlayout =
{
	8,8,    /* 8*8 characters */
	256,    /* 256 characters */
	4,       /* 4 bits per pixel */
	{0,1,2,3 }, /* the bitplanes are packed in one nibble */
	{0, 4, 8192*8+0, 8192*8+4, 8, 12, 8192*8+8, 8192*8+12},
	{16*0, 16*1, 16*2, 16*3, 16*4, 16*5, 16*6, 16*7},
	8*16
};

static const gfx_layout spritelayout =
{
	16,16,    /* 16*16 characters */
	128,      /* 128 sprites */
	4,       /* 4 bits per pixel */
	{0,1,2,3}, /* the bitplanes are packed in one nibble */
	{0,  4,  8192*8+0,  8192*8+4,  8, 12,  8192*8+8, 8192*8+12,
			16*8+0, 16*8+4, 16*8+8192*8+0, 16*8+8192*8+4, 16*8+8, 16*8+12, 16*8+8192*8+8, 16*8+8192*8+12},
	{16*0, 16*1, 16*2, 16*3, 16*4, 16*5, 16*6, 16*7,
			32*8+16*0, 32*8+16*1, 32*8+16*2, 32*8+16*3, 32*8+16*4, 32*8+16*5, 32*8+16*6, 32*8+16*7},
	8*64
};

static GFXDECODE_START( nova2001 )
	GFXDECODE_ENTRY( REGION_GFX1, 0x0000, charlayout,       0, 16 )
	GFXDECODE_ENTRY( REGION_GFX2, 0x0000, charlayout,   16*16, 16 )
	GFXDECODE_ENTRY( REGION_GFX1, 0x0000, spritelayout,     0, 16 )
	GFXDECODE_ENTRY( REGION_GFX2, 0x0000, spritelayout,     0, 16 )
GFXDECODE_END



static struct AY8910interface ay8910_interface_1 =
{
	0,
	0,
	nova2001_scroll_x_w, /* writes are connected to pf scroll */
	nova2001_scroll_y_w
};

static struct AY8910interface ay8910_interface_2 =
{
	input_port_3_r,
	input_port_4_r
};

static MACHINE_DRIVER_START( nova2001 )

	/* basic machine hardware */
	MDRV_CPU_ADD(Z80, 12000000/4)	// 3 MHz
	MDRV_CPU_PROGRAM_MAP(nova2001_map,0)
	MDRV_CPU_VBLANK_INT(irq0_line_hold,1)

	MDRV_SCREEN_REFRESH_RATE(60)
	MDRV_SCREEN_VBLANK_TIME(DEFAULT_REAL_60HZ_VBLANK_DURATION)

	/* video hardware */
	MDRV_VIDEO_ATTRIBUTES(VIDEO_TYPE_RASTER)
	MDRV_SCREEN_FORMAT(BITMAP_FORMAT_INDEXED16)
	MDRV_SCREEN_SIZE(32*8, 32*8)
	MDRV_SCREEN_VISIBLE_AREA(1*8, 31*8-1, 4*8, 28*8-1)
	MDRV_GFXDECODE(nova2001)
	MDRV_PALETTE_LENGTH(32)
	MDRV_COLORTABLE_LENGTH(32*16)

	MDRV_PALETTE_INIT(nova2001)
	MDRV_VIDEO_START(nova2001)
	MDRV_VIDEO_UPDATE(nova2001)

	/* sound hardware */
	MDRV_SPEAKER_STANDARD_MONO("mono")

	MDRV_SOUND_ADD(AY8910, 6000000/3)
	MDRV_SOUND_CONFIG(ay8910_interface_1)
	MDRV_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.25)

	MDRV_SOUND_ADD(AY8910, 6000000/3)
	MDRV_SOUND_CONFIG(ay8910_interface_2)
	MDRV_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.25)
MACHINE_DRIVER_END



ROM_START( nova2001 )
	ROM_REGION( 0x10000, REGION_CPU1, 0 )
	ROM_LOAD( "1.6c",         0x0000, 0x2000, CRC(368cffc0) SHA1(b756c0542d5b86640af62639bdd0d32f6e364dd3) )
	ROM_LOAD( "2.6d",         0x2000, 0x2000, CRC(bc4e442b) SHA1(6e1dca5dde442db95403377bf49aaad2a337813e) )
	ROM_LOAD( "3.6f",         0x4000, 0x2000, CRC(b2849038) SHA1(b56c7c03ef7c677cc6df0280a485f9cda3435b23) )
	ROM_LOAD( "4.6g",         0x6000, 0x1000, CRC(6b5bb12d) SHA1(74aee3d08a7ee1f98eaec4a4b3062aa9d17948ec) )
	ROM_RELOAD(               0x7000, 0x1000 )

	ROM_REGION( 0x4000, REGION_GFX1, ROMREGION_DISPOSE )
	ROM_LOAD( "5.12s",        0x0000, 0x2000, CRC(54198941) SHA1(fe762a0bbcf10b13ece87ded2ea730257cfbe7d3) )
	ROM_LOAD( "6.12p",        0x2000, 0x2000, CRC(cbd90dca) SHA1(7eacde832f5783f4389fb98d6bf6b26dd494665d) )

	ROM_REGION( 0x4000, REGION_GFX2, ROMREGION_DISPOSE )
	ROM_LOAD( "7.12n",        0x0000, 0x2000, CRC(9ebd8806) SHA1(26b6caa0d0a7ae52a182070ecc7bc696c12038b3) )
	ROM_LOAD( "8.12l",        0x2000, 0x2000, CRC(d1b18389) SHA1(2d808fee774f1bb4cec42e23cfef36b54eee0efa) )

	ROM_REGION( 0x0020, REGION_PROMS, 0 )
	ROM_LOAD( "nova2001.clr", 0x0000, 0x0020, CRC(a2fac5cd) SHA1(ad14aa2be57722d1f48b47171fe72f96091423b6) )
ROM_END

ROM_START( nov2001u )
	ROM_REGION( 0x10000, REGION_CPU1, 0 )
	ROM_LOAD( "nova2001.1",   0x0000, 0x2000, CRC(b79461bd) SHA1(7fac3313bc76612f66a6518450d0fed32fe70c45) )
	ROM_LOAD( "nova2001.2",   0x2000, 0x2000, CRC(fab87144) SHA1(506703f9d96443839f864ef5bde1a71120f54384) )
	ROM_LOAD( "3.6f",         0x4000, 0x2000, CRC(b2849038) SHA1(b56c7c03ef7c677cc6df0280a485f9cda3435b23) )
	ROM_LOAD( "4.6g",         0x6000, 0x1000, CRC(6b5bb12d) SHA1(74aee3d08a7ee1f98eaec4a4b3062aa9d17948ec) )
	ROM_RELOAD(               0x7000, 0x1000 )

	ROM_REGION( 0x4000, REGION_GFX1, ROMREGION_DISPOSE )
	ROM_LOAD( "nova2001.5",   0x0000, 0x2000, CRC(8ea576e8) SHA1(d8dbcfd43aafe25afad7f947a80737cdc55b23d7) )
	ROM_LOAD( "nova2001.6",   0x2000, 0x2000, CRC(0c61656c) SHA1(41c480799798c95543b5a805694e68282b9f563a) )

	ROM_REGION( 0x4000, REGION_GFX2, ROMREGION_DISPOSE )
	ROM_LOAD( "7.12n",        0x0000, 0x2000, CRC(9ebd8806) SHA1(26b6caa0d0a7ae52a182070ecc7bc696c12038b3) )
	ROM_LOAD( "8.12l",        0x2000, 0x2000, CRC(d1b18389) SHA1(2d808fee774f1bb4cec42e23cfef36b54eee0efa) )

	ROM_REGION( 0x0020, REGION_PROMS, 0 )
	ROM_LOAD( "nova2001.clr", 0x0000, 0x0020, CRC(a2fac5cd) SHA1(ad14aa2be57722d1f48b47171fe72f96091423b6) )
ROM_END



GAME( 1983, nova2001, 0,        nova2001, nova2001, 0, ROT0, "UPL", "Nova 2001 (Japan)", 0 )
GAME( 1983, nov2001u, nova2001, nova2001, nova2001, 0, ROT0, "UPL (Universal license)", "Nova 2001 (US)", 0 )
