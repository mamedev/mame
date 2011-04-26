/***************************************************************************

Speed Ball map

driver by Joseba Epalza

Z80 MAIN CPU
0000-7fff ROM 1 32k
8000-dbff ROM 2 data
dc00-dfff for both CPU&SOUND (Shared)
e000-e1ff Video RAM background tiles 16*16 (2 bytes: 1 for tile char + 1 for color)
e800-efff Video RAM characters 32*32 (2 bytes: 1 for char and 1 for color)
f000-feff RAM
ff00-ffff Sprites (64 total: four bytes to one)

in:
00  DSW ONE
10  DSW TWO
20  IN1  JOY 1 STATUS & COIN & START
30  IN2  JOY 2 STATUS & TILT

out:
40  coin counters
50  ????  maybe VSYNC ????

 ======================================================================

Z80 SOUND
0000-7fff ROM
dc00-dfff shared with MAIN CPU
f000-dfff RAM

out:
00  YM3812 control
01  YM3812 data
40  ??
80  ??
82  ??
c1  ??

 ======================================================================

  Colors : 2 bits for foreground characters =  4 colors * 16 palettes
           4 bits for background tiles      = 16 colors * 16 palettes
           4 bits for sprites               = 16 colors * 16 palettes

 Note:
 - To enter test mode, keep pressed COIN1 and COIN2 during boot,
   until the RAM / ROM tests are finished

***************************************************************************/

#include "emu.h"
#include "cpu/z80/z80.h"
#include "sound/3812intf.h"
#include "includes/speedbal.h"
#include "machine/nvram.h"

static WRITE8_HANDLER( speedbal_coincounter_w )
{
	coin_counter_w(space->machine(), 0, data & 0x80);
	coin_counter_w(space->machine(), 1, data & 0x40);
	flip_screen_set(space->machine(), data & 8); // also changes data & 0x10 at the same time too (flipx and flipy?)
	/* unknown: (data & 0x10) and (data & 4) */
}

static ADDRESS_MAP_START( main_cpu_map, AS_PROGRAM, 8 )
	AM_RANGE(0x0000, 0xdbff) AM_ROM
	AM_RANGE(0xdc00, 0xdfff) AM_RAM AM_SHARE("share1") // shared with SOUND
	AM_RANGE(0xe000, 0xe1ff) AM_RAM_WRITE(speedbal_background_videoram_w) AM_BASE_MEMBER(speedbal_state, m_background_videoram)
	AM_RANGE(0xe800, 0xefff) AM_RAM_WRITE(speedbal_foreground_videoram_w) AM_BASE_MEMBER(speedbal_state, m_foreground_videoram)
	AM_RANGE(0xf000, 0xf5ff) AM_RAM_WRITE(paletteram_RRRRGGGGBBBBxxxx_be_w) AM_BASE_GENERIC(paletteram)
	AM_RANGE(0xf600, 0xfeff) AM_RAM AM_SHARE("nvram")
	AM_RANGE(0xff00, 0xffff) AM_RAM AM_BASE_SIZE_MEMBER(speedbal_state, m_spriteram, m_spriteram_size)
ADDRESS_MAP_END

static ADDRESS_MAP_START( main_cpu_io_map, AS_IO, 8 )
	ADDRESS_MAP_GLOBAL_MASK(0xff)
	AM_RANGE(0x00, 0x00) AM_READ_PORT("DSW2")
	AM_RANGE(0x10, 0x10) AM_READ_PORT("DSW1")
	AM_RANGE(0x20, 0x20) AM_READ_PORT("P1")
	AM_RANGE(0x30, 0x30) AM_READ_PORT("P2")
	AM_RANGE(0x40, 0x40) AM_WRITE(speedbal_coincounter_w)
	AM_RANGE(0x50, 0x50) AM_WRITENOP
ADDRESS_MAP_END

static ADDRESS_MAP_START( sound_cpu_map, AS_PROGRAM, 8 )
	AM_RANGE(0x0000, 0x7fff) AM_ROM
	AM_RANGE(0xd000, 0xdbff) AM_RAM
	AM_RANGE(0xdc00, 0xdfff) AM_RAM AM_SHARE("share1") // shared with MAIN CPU
	AM_RANGE(0xf000, 0xffff) AM_RAM
ADDRESS_MAP_END

static ADDRESS_MAP_START( sound_cpu_io_map, AS_IO, 8 )
	ADDRESS_MAP_GLOBAL_MASK(0xff)
	AM_RANGE(0x00, 0x01) AM_DEVREADWRITE("ymsnd", ym3812_r, ym3812_w)
	AM_RANGE(0x40, 0x40) AM_WRITENOP
	AM_RANGE(0x80, 0x80) AM_WRITENOP
	AM_RANGE(0x82, 0x82) AM_WRITENOP
	AM_RANGE(0xc1, 0xc1) AM_WRITENOP
ADDRESS_MAP_END


static INPUT_PORTS_START( speedbal )
	PORT_START("DSW1")
	PORT_DIPNAME( 0x03, 0x03, DEF_STR( Coin_B ) )
	PORT_DIPSETTING(    0x03, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0x02, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(    0x01, DEF_STR( 1C_4C ) )
	PORT_DIPSETTING(    0x00, DEF_STR( 1C_5C ) )
	PORT_DIPNAME( 0x0c, 0x0c, DEF_STR( Coin_A ) )
	PORT_DIPSETTING(    0x00, DEF_STR( 4C_1C ) )
	PORT_DIPSETTING(    0x04, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(    0x08, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x0c, DEF_STR( 1C_1C ) )
	PORT_DIPNAME( 0x30, 0x30, DEF_STR( Lives ) )
	PORT_DIPSETTING(    0x00, "2" )
	PORT_DIPSETTING(    0x30, "3" )
	PORT_DIPSETTING(    0x20, "4" )
	PORT_DIPSETTING(    0x10, "5" )
	PORT_DIPNAME( 0x40, 0x00, DEF_STR( Cabinet ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Upright ) )
	PORT_DIPSETTING(    0x40, DEF_STR( Cocktail ) )
	PORT_DIPNAME( 0x80, 0x80, DEF_STR( Demo_Sounds ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x80, DEF_STR( On ) )

	PORT_START("DSW2")
	PORT_DIPNAME( 0x07, 0x07, DEF_STR( Bonus_Life ) )
	PORT_DIPSETTING(    0x06, "70000 200000 1M" )
	PORT_DIPSETTING(    0x07, "70000 200000" )
	PORT_DIPSETTING(    0x03, "100000 300000 1M" )
	PORT_DIPSETTING(    0x04, "100000 300000" )
	PORT_DIPSETTING(    0x01, "200000 1M" )
	PORT_DIPSETTING(    0x05, "200000" )
/*  PORT_DIPSETTING(    0x02, "200000" ) */
	PORT_DIPSETTING(    0x00, DEF_STR( None ) )
	PORT_DIPNAME( 0x08, 0x08, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x08, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x30, 0x30, "Difficulty 1" )
	PORT_DIPSETTING(    0x30, DEF_STR( Very_Easy) )
	PORT_DIPSETTING(    0x20, DEF_STR( Easy ) )
	PORT_DIPSETTING(    0x10, DEF_STR( Difficult ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Very_Difficult ) )
	PORT_DIPNAME( 0xc0, 0xc0, "Difficulty 2" )
	PORT_DIPSETTING(    0xc0, DEF_STR( Very_Easy) )
	PORT_DIPSETTING(    0x80, DEF_STR( Easy ) )
	PORT_DIPSETTING(    0x40, DEF_STR( Difficult ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Very_Difficult ) )

	PORT_START("P1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW , IPT_BUTTON1 )
	PORT_BIT( 0x02, IP_ACTIVE_LOW , IPT_BUTTON2 )
	PORT_BIT( 0x04, IP_ACTIVE_LOW , IPT_BUTTON4 )
	PORT_BIT( 0x08, IP_ACTIVE_LOW , IPT_BUTTON3 )
	PORT_BIT( 0x10, IP_ACTIVE_LOW , IPT_START2 )
	PORT_BIT( 0x20, IP_ACTIVE_LOW , IPT_START1 )
	PORT_BIT( 0x40, IP_ACTIVE_LOW , IPT_COIN2 )
	PORT_BIT( 0x80, IP_ACTIVE_LOW , IPT_COIN1 )

	PORT_START("P2")
	PORT_BIT( 0x01, IP_ACTIVE_LOW , IPT_BUTTON1 ) PORT_COCKTAIL
	PORT_BIT( 0x02, IP_ACTIVE_LOW , IPT_BUTTON2 ) PORT_COCKTAIL
	PORT_BIT( 0x04, IP_ACTIVE_LOW , IPT_BUTTON4 ) PORT_COCKTAIL
	PORT_BIT( 0x08, IP_ACTIVE_LOW , IPT_BUTTON3 ) PORT_COCKTAIL
	PORT_BIT( 0x10, IP_ACTIVE_LOW , IPT_TILT    )
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_UNKNOWN )
INPUT_PORTS_END



static const gfx_layout charlayout =
{
	8,8,    /* 8*8 characters */
	1024,   /* 1024 characters */
	4,      /* actually 2 bits per pixel - two of the planes are empty */
	{ 1024*16*8+4, 1024*16*8+0, 4, 0 },
	{ 8+3, 8+2, 8+1, 8+0, 3, 2, 1, 0 },
	{ 0*16, 1*16, 2*16, 3*16, 4*16, 5*16, 6*16, 7*16 },   /* characters are rotated 90 degrees */
	16*8	   /* every char takes 16 bytes */
};

static const gfx_layout tilelayout =
{
	16,16,  /* 16*16 tiles */
	1024,   /* 1024 tiles */
	4,      /* 4 bits per pixel */
	{ 0, 2, 4, 6 }, /* the bitplanes are packed in one nibble */
	{ 0*8+0, 0*8+1, 7*8+0, 7*8+1, 6*8+0, 6*8+1, 5*8+0, 5*8+1,
			4*8+0, 4*8+1, 3*8+0, 3*8+1, 2*8+0, 2*8+1, 1*8+0, 1*8+1 },
	{ 0*64, 1*64, 2*64, 3*64, 4*64, 5*64, 6*64, 7*64,
			8*64, 9*64, 10*64, 11*64, 12*64, 13*64, 14*64, 15*64 },
	128*8  /* every sprite takes 128 consecutive bytes */
};

static const gfx_layout spritelayout =
{
	16,16,  /* 16*16 sprites */
	512,    /* 512 sprites */
	4,      /* 4 bits per pixel */
	{ 0, 2, 4, 6 }, /* the bitplanes are packed in one nibble */
	{ 7*8+1, 7*8+0, 6*8+1, 6*8+0, 5*8+1, 5*8+0, 4*8+1, 4*8+0,
			3*8+1, 3*8+0, 2*8+1, 2*8+0, 1*8+1, 1*8+0, 0*8+1, 0*8+0 },
	{ 0*64, 1*64, 2*64, 3*64, 4*64, 5*64, 6*64, 7*64,
			8*64, 9*64, 10*64, 11*64, 12*64, 13*64, 14*64, 15*64 },
	128*8  /* every sprite takes 128 consecutive bytes */
};

static GFXDECODE_START( speedbal )
	GFXDECODE_ENTRY( "gfx1", 0, charlayout,	 256, 16 )
	GFXDECODE_ENTRY( "gfx2", 0, tilelayout,	 512, 16 )
	GFXDECODE_ENTRY( "gfx3", 0, spritelayout,   0, 16 )
GFXDECODE_END



static MACHINE_CONFIG_START( speedbal, speedbal_state )

	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", Z80, 4000000)	/* 4 MHz ??? */
	MCFG_CPU_PROGRAM_MAP(main_cpu_map)
	MCFG_CPU_IO_MAP(main_cpu_io_map)
	MCFG_CPU_VBLANK_INT("screen", irq0_line_hold)

	MCFG_CPU_ADD("audiocpu", Z80, 2660000)	/* 2.66 MHz ???  Maybe yes */
	MCFG_CPU_PROGRAM_MAP(sound_cpu_map)
	MCFG_CPU_IO_MAP(sound_cpu_io_map)
	MCFG_CPU_PERIODIC_INT(irq0_line_hold,8*60)

	MCFG_NVRAM_ADD_1FILL("nvram")

	/* video hardware */
	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_REFRESH_RATE(60)
	MCFG_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(2500) /* not accurate */)
	MCFG_SCREEN_FORMAT(BITMAP_FORMAT_INDEXED16)
	MCFG_SCREEN_SIZE(32*8, 32*8)
	MCFG_SCREEN_VISIBLE_AREA(0*8, 32*8-1, 2*8, 30*8-1)
	MCFG_SCREEN_UPDATE(speedbal)

	MCFG_GFXDECODE(speedbal)
	MCFG_PALETTE_LENGTH(768)

	MCFG_VIDEO_START(speedbal)

	/* sound hardware */
	MCFG_SPEAKER_STANDARD_MONO("mono")

	MCFG_SOUND_ADD("ymsnd", YM3812, 3600000)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 1.0)
MACHINE_CONFIG_END



/***************************************************************************

  Game driver(s)

***************************************************************************/

ROM_START( speedbal )
	ROM_REGION( 0x10000, "maincpu", 0 )     /* 64K for code: main */
	ROM_LOAD( "sb1.bin",  0x0000,  0x8000, CRC(1c242e34) SHA1(8b2e8983e0834c99761ce2b5ea765dba56e77964) )
	ROM_LOAD( "sb3.bin",  0x8000,  0x8000, CRC(7682326a) SHA1(15a72bf088a9adfaa50c11202b4970e07c309a21) )

	ROM_REGION( 0x10000, "audiocpu", 0 )     /* 64K for second CPU: sound */
	ROM_LOAD( "sb2.bin",  0x0000, 0x8000, CRC(e6a6d9b7) SHA1(35d228d13d4305f606fdd84adad1d6e435f4b7ce) )

	ROM_REGION( 0x08000, "gfx1", 0 )
	ROM_LOAD("sb10.bin",  0x00000, 0x08000, CRC(36dea4bf) SHA1(60095f482af4595a39be5ae6def8cd30298c1ef8) )    /* chars */

	ROM_REGION( 0x20000, "gfx2", 0 )
	ROM_LOAD( "sb9.bin",  0x00000, 0x08000, CRC(b567e85e) SHA1(7036792ea70ad48384f348399ed9b136272fedb6) )    /* bg tiles */
	ROM_LOAD( "sb5.bin",  0x08000, 0x08000, CRC(b0eae4ba) SHA1(baee3fcb1399c56efaa5f97912de324d7b38f286) )
	ROM_LOAD( "sb8.bin",  0x10000, 0x08000, CRC(d2bfbdb6) SHA1(b552b055450f438729c83337f561d05b6518ae75) )
	ROM_LOAD( "sb4.bin",  0x18000, 0x08000, CRC(1d23a130) SHA1(aabf7c46f9299ffb8b8ca92839622d000a470a0b) )

	ROM_REGION( 0x10000, "gfx3", ROMREGION_INVERT )
	ROM_LOAD( "sb7.bin",  0x00000, 0x08000, CRC(9f1b33d1) SHA1(1f8be8f8e6a2ee99a7dafeead142ccc629fa792d) )   /* sprites */
	ROM_LOAD( "sb6.bin",  0x08000, 0x08000, CRC(0e2506eb) SHA1(56f779266b977819063c475b84ca246fc6d8d6a7) )
ROM_END


GAME( 1987, speedbal, 0, speedbal, speedbal, 0, ROT270, "Tecfri", "Speed Ball", 0 )
