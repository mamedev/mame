/* Galxaia */

/*

Galaxia by Zaccaria (1980)

Taken from an untested board.

1K byte files were 2708 or equivalent.
512 byte file is the 82S130 colour PROM.

This is not a direct pirate of Galaxians as you might think from the name.
The game uses a Signetics 2650A CPU with three 40-pin 2636 chips. I have
no idea what 2636's are but I am hoping they are something to do with the
sound since the board has no apparent sound circuitry. The video hardware
looks like it's similar to Galaxians (2 x 2114, 2 x 2101, 2 x EPROM) but
there is no attack RAM and the graphics EPROMS are 2708. The graphics EPROMS
do contain Galaxian-like graphics...

---

rom mapping is still wrong, correct this before anything else

*/

#include "driver.h"

static UINT8 *galaxia_video;

static VIDEO_START( galaxia )
{
}

static VIDEO_UPDATE( galaxia )
{
	int x,y, count;

	fillbitmap(bitmap,0,cliprect);
	count = 0;

	for (y=0;y<256/8;y++)
	{
		for (x=0;x<256/8;x++)
		{
			int tile = galaxia_video[count];
			drawgfx(bitmap,screen->machine->gfx[0],tile,0,0,0,x*8,y*8,cliprect,TRANSPARENCY_NONE,0);
			count++;
		}

	}



	return 0;
}

static ADDRESS_MAP_START( readmem, ADDRESS_SPACE_PROGRAM, 8 )
	AM_RANGE(0x0000, 0x13ff) AM_READ(SMH_ROM)
	AM_RANGE(0x1400, 0x1fff) AM_READ(SMH_RAM)
	AM_RANGE(0x5000, 0x6fff) AM_READ(SMH_ROM)
ADDRESS_MAP_END

static ADDRESS_MAP_START( writemem, ADDRESS_SPACE_PROGRAM, 8 )
	AM_RANGE(0x0000, 0x13ff) AM_WRITE(SMH_ROM)
	AM_RANGE(0x1400, 0x14ff) AM_WRITE(SMH_RAM)
	AM_RANGE(0x1500, 0x15ff) AM_WRITE(SMH_RAM)
	AM_RANGE(0x1600, 0x16ff) AM_WRITE(SMH_RAM)
	AM_RANGE(0x1700, 0x17ff) AM_WRITE(SMH_RAM)
	AM_RANGE(0x1800, 0x18ff) AM_WRITE(SMH_RAM) AM_BASE(&galaxia_video)
	AM_RANGE(0x1900, 0x19ff) AM_WRITE(SMH_RAM)
	AM_RANGE(0x1a00, 0x1aff) AM_WRITE(SMH_RAM)
	AM_RANGE(0x1b00, 0x1bff) AM_WRITE(SMH_RAM)
	AM_RANGE(0x1c00, 0x1cff) AM_WRITE(SMH_RAM)
	AM_RANGE(0x1d00, 0x1dff) AM_WRITE(SMH_RAM)
	AM_RANGE(0x1e00, 0x1eff) AM_WRITE(SMH_RAM)
	AM_RANGE(0x1f00, 0x1fff) AM_WRITE(SMH_RAM)
ADDRESS_MAP_END

static ADDRESS_MAP_START( readport, ADDRESS_SPACE_IO, 8 )
	ADDRESS_MAP_GLOBAL_MASK(0xff)
	AM_RANGE(0x02, 0x02) AM_READ(input_port_0_r)
ADDRESS_MAP_END

static ADDRESS_MAP_START( writeport, ADDRESS_SPACE_IO, 8 )
	ADDRESS_MAP_GLOBAL_MASK(0xff)
ADDRESS_MAP_END


static INPUT_PORTS_START( galaxia )
	PORT_START	/*  */

	PORT_DIPNAME( 0x01, 0x01, DEF_STR( Unused ) )
	PORT_DIPSETTING(    0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

	PORT_DIPNAME( 0x02, 0x02, DEF_STR( Unused ) )
	PORT_DIPSETTING(    0x02, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

	PORT_DIPNAME( 0x04, 0x04, DEF_STR( Unused ) )
	PORT_DIPSETTING(    0x04, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

	PORT_DIPNAME( 0x08, 0x08, DEF_STR( Unused ) )
	PORT_DIPSETTING(    0x08, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

	PORT_DIPNAME( 0x10, 0x10, DEF_STR( Unused ) )
	PORT_DIPSETTING(    0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

	PORT_DIPNAME( 0x20, 0x20, DEF_STR( Unused ) )
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

	PORT_DIPNAME( 0x40, 0x40, DEF_STR( Unused ) )
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

	PORT_DIPNAME( 0x80, 0x80, DEF_STR( Unused ) )
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
INPUT_PORTS_END

static const gfx_layout tiles8x8_layout =
{
	8,8,
	RGN_FRAC(1,2),
	2,
	{ RGN_FRAC(0,2), RGN_FRAC(1,2) },
	{ 0, 1, 2, 3, 4, 5, 6, 7 },
	{ 0*8, 1*8, 2*8, 3*8, 4*8, 5*8, 6*8, 7*8 },
	8*8
};

static GFXDECODE_START( galaxia )
	GFXDECODE_ENTRY( REGION_GFX1, 0, tiles8x8_layout, 0, 16 )
GFXDECODE_END





static MACHINE_DRIVER_START( galaxia )
	/* basic machine hardware */
	MDRV_CPU_ADD(S2650,2000000)		 /* ? MHz */
	MDRV_CPU_PROGRAM_MAP(readmem,writemem)
	MDRV_CPU_IO_MAP(readport,writeport)
	MDRV_CPU_VBLANK_INT("main", irq0_line_hold)
//  MDRV_CPU_VBLANK_INT("main", nmi_line_pulse)

	/* video hardware */
	MDRV_SCREEN_ADD("main", RASTER)
	MDRV_SCREEN_REFRESH_RATE(60)
	MDRV_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(0))
	MDRV_SCREEN_FORMAT(BITMAP_FORMAT_INDEXED16)
	MDRV_SCREEN_SIZE(256, 256)
	MDRV_SCREEN_VISIBLE_AREA(0, 256-1, 0, 256-1)

	MDRV_GFXDECODE(galaxia)
	MDRV_PALETTE_LENGTH(0x100)

	MDRV_VIDEO_START(galaxia)
	MDRV_VIDEO_UPDATE(galaxia)
MACHINE_DRIVER_END


ROM_START( galaxia )
	ROM_REGION( 0x10000, REGION_CPU1, 0 )
	ROM_LOAD( "08h.bin", 0x00000, 0x0400, CRC(f3b4ffde) SHA1(15b004e7821bfc145158b1e9435f061c524f6b86) )
	ROM_LOAD( "10h.bin", 0x00400, 0x0400, CRC(6d07fdd4) SHA1(d7d4b345a055275d59951788569db370bccd5195) )
	ROM_LOAD( "08i.bin", 0x00800, 0x0400, CRC(45b88599) SHA1(3b79c21db1aa9d80fac81ac5a554e438805febd1) )
	ROM_LOAD( "10i.bin", 0x00c00, 0x0400, CRC(c0baa654) SHA1(80e0880c32ad285fbce0f7f552268b964b97cab3) )

	ROM_LOAD( "11h.bin", 0x05000, 0x0400, CRC(1520eb3d) SHA1(3683174da701e1124af0f9c2ee4a9a84f3fea33a) )
	ROM_LOAD( "13h.bin", 0x05400, 0x0400, CRC(c4482770) SHA1(aee983cc3d80989f49aea4138961bb623039484a) )
	ROM_LOAD( "11i.bin", 0x05800, 0x0400, CRC(4456808a) SHA1(f9e8cfdde0e17f13f1be297b2b4503ccc959b33c) )
	ROM_LOAD( "13i.bin", 0x05c00, 0x0400, CRC(cf653b9a) SHA1(fef5943de60cb5ba2459fc6ae7419e29c96a76cd) )
	ROM_LOAD( "11l.bin", 0x06000, 0x0400, CRC(50c6a645) SHA1(46638907bc393df6be25fc7461d73047d1746ffc) )
	ROM_LOAD( "13l.bin", 0x06400, 0x0400, CRC(3a9c38c7) SHA1(d1e934092b69c0f3f9636eba05a1d8a6d9588e6b) )

	ROM_REGION( 0x0800, REGION_GFX1, 0 )
	ROM_LOAD( "01d.bin", 0x00000, 0x0400, CRC(2dd50aab) SHA1(758d7a5383c9a1ee134d99e3f7025819cfbe0e0f) )
	ROM_LOAD( "03d.bin", 0x00400, 0x0400, CRC(1dc30185) SHA1(e3c75eecb80b376ece98f602e1b9587487841824) )

	ROM_REGION( 0x80000, REGION_PROMS, 0 )
	ROM_LOAD( "11o", 0x00000, 0x0200, CRC(ae816417) SHA1(9497857d13c943a2735c3b85798199054e613b2c) )
ROM_END

GAME( 1979, galaxia, 0, galaxia, galaxia, 0, ROT90, "Zaccaria", "Galaxia", GAME_NOT_WORKING|GAME_NO_SOUND )
