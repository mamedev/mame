/*

Merit Poker?

*/

#include "driver.h"
#include "cpu/z80/z80.h"

static UINT8* mpoker_video;


static VIDEO_START(mpoker)
{

}

static VIDEO_UPDATE(mpoker)
{
	int y,x;
	int count;
	const gfx_element *gfx = screen->machine->gfx[0];

	count = 0;
	for (y=0;y<32;y++)
	{
		for (x=0;x<32;x++)
		{
			UINT16 dat = mpoker_video[count];
			UINT16 col = mpoker_video[count+0x400];
			drawgfx_opaque(bitmap,cliprect,gfx,dat,col,0,0,x*16,y*16);
			count++;
		}

	}
	return 0;
}

static READ8_HANDLER( test_r )
{
	return mame_rand(space->machine);
}

static ADDRESS_MAP_START( main_map, ADDRESS_SPACE_PROGRAM, 8 )
	AM_RANGE(0x0000, 0x2fff) AM_ROM
	AM_RANGE(0x3800, 0x3fff) AM_RAM
	AM_RANGE(0x4000, 0x47ff) AM_RAM AM_BASE(&mpoker_video)
	AM_RANGE(0x8000, 0x8000) AM_READ_PORT("IN0")
	AM_RANGE(0x8001, 0x8001) AM_READ(test_r)
	AM_RANGE(0x8002, 0x8002) AM_READ_PORT("IN1")
	AM_RANGE(0x8003, 0x8003) AM_READ_PORT("IN2")
//  AM_RANGE(0x8004, 0x8004) AM_READ(test_r)
//  AM_RANGE(0x8005, 0x8005) AM_READ(test_r)
//  AM_RANGE(0x8006, 0x8006) AM_READ(test_r)
//  AM_RANGE(0x8007, 0x8007) AM_READ(test_r)
ADDRESS_MAP_END

static INPUT_PORTS_START( mpoker )
	PORT_START("IN0")
	PORT_DIPNAME( 0x01, 0x01, "IN0" )
	PORT_DIPSETTING(    0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x02, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x02, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x04, 0x04, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x04, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x08, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x08, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x10, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x20, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x40, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_START("IN1")
	PORT_DIPNAME( 0x01, 0x01, "IN1" )
	PORT_DIPSETTING(    0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x02, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x02, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x04, 0x04, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x04, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x08, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x08, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x10, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x20, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x40, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_START("IN2")
	PORT_DIPNAME( 0x01, 0x01, "IN2" )
	PORT_DIPSETTING(    0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x02, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x02, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x04, 0x04, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x04, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x08, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x08, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x10, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x20, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x40, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
INPUT_PORTS_END


static const gfx_layout tiles16x16_layout =
{
	16,16,
	RGN_FRAC(1,1),
	1,
	{ 0 },
	{ 0, 1, 2, 3, 4, 5, 6, 7,8,9,10,11,12,13,14,15 },
	{ 0*16, 1*16, 2*16, 3*16, 4*16, 5*16, 6*16, 7*16,
	  8*16, 9*16,10*16,11*16,12*16,13*16,14*16,15*16},
	16*16
};

static GFXDECODE_START( mpoker )
	GFXDECODE_ENTRY( "gfx1", 0, tiles16x16_layout, 0, 16 )
GFXDECODE_END

static MACHINE_DRIVER_START( mpoker )
	/* basic machine hardware */
	MDRV_CPU_ADD("maincpu", Z80,4000000)		 /* ? MHz */
	MDRV_CPU_PROGRAM_MAP(main_map)
	MDRV_CPU_VBLANK_INT("screen", irq0_line_hold)

	/* video hardware */
	MDRV_SCREEN_ADD("screen", RASTER)
	MDRV_SCREEN_REFRESH_RATE(60)
	MDRV_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(0))
	MDRV_SCREEN_FORMAT(BITMAP_FORMAT_INDEXED16)
	MDRV_SCREEN_SIZE(512, 256)
	MDRV_SCREEN_VISIBLE_AREA(0, 512-1, 0, 256-1)

	MDRV_GFXDECODE(mpoker)
	MDRV_PALETTE_LENGTH(0x100)

	MDRV_VIDEO_START(mpoker)
	MDRV_VIDEO_UPDATE(mpoker)
MACHINE_DRIVER_END

ROM_START( mpoker )
	ROM_REGION( 0x3000, "maincpu", 0 )
	ROM_LOAD( "u13_mlt10_mk2716j.bin", 0x0000, 0x0800, CRC(ce2da863) SHA1(ddb921ac2fdd965138a91757843d3035144a7007) )
	ROM_LOAD( "u14_mlt11_mk2716j.bin", 0x0800, 0x0800, CRC(1382d166) SHA1(a8e7339f94d65b9540a8c16190a28ff0af48ccb4) )
	ROM_LOAD( "u15_mlt12_mk2716j.bin", 0x1000, 0x0800, CRC(eb12716c) SHA1(1589cb0aa180a3bfa5cb3da200b71f77c2191272) )
	ROM_LOAD( "u16_mlt13_mk2716j.bin", 0x1800, 0x0800, CRC(e2d80ff0) SHA1(f8aaa513f57da458ca89f999e30ea6b2d2ef41d3) )
	ROM_LOAD( "u17_mlt14_mk2716j.bin", 0x2000, 0x0800, CRC(36efcdf1) SHA1(d6fdc6abb1dbb5f812dc1c1ecb8d369bfcbf2b8a) )
	ROM_LOAD( "u18_mlt15_mk2716j.bin", 0x2800, 0x0800, CRC(7701c7df) SHA1(abf19f75367f926e49031e2fb4021172ebf176e1) )

	ROM_REGION( 0x2000, "gfx1", 0 )
	ROM_LOAD( "u68_cgm0_mk2716j.bin", 0x0000, 0x0800, CRC(3e4148e3) SHA1(bc5d173cc6ff17e0a7f06d36790e20d254be0377) )
	ROM_LOAD( "u67_cgm1_mk2716j.bin", 0x0800, 0x0800, CRC(c66c30ed) SHA1(9fb8fc669da37ff2e0379b023349ed314c8dcba4) )
	ROM_LOAD( "u66_cgm2_mk2716j.bin", 0x1000, 0x0800, CRC(3b6b98d3) SHA1(9fc1d9d61d67ad696750c21ef9a19968ddb0a9e1) )
	ROM_LOAD( "u65_cgm3_mk2716j.bin", 0x1800, 0x0800, CRC(d61ae9d1) SHA1(219123518999fc925397db4f442ac444dfddffbe) )
ROM_END

GAME( 1983, mpoker,  0,    mpoker, mpoker,  0, ROT0, "Merit", "Merit Poker", GAME_NOT_WORKING|GAME_NO_SOUND )
