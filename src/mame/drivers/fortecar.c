/* Forte Card */

/*

Forte Card (POKER GAME)

CPU  SGS Z8400AB1 (Z80ACPU)
VIDEO CM607P
PIO M5L8255AP
snd ay38910/P (microchip)
+ 8251A

RAM 6116 + GOLDSTAR GM76C256ALL-70
dip 1X8

*/




#include "driver.h"
#include "deprecat.h"

static UINT8 *fortecar_ram;


static ADDRESS_MAP_START( fortecar_map, ADDRESS_SPACE_PROGRAM, 8 )
	AM_RANGE(0x0000, 0xc7ff) AM_READ(MRA8_ROM)
	AM_RANGE(0xd000, 0xd1ff) AM_RAM

	AM_RANGE(0xd800, 0xffff) AM_RAM AM_BASE(&fortecar_ram)
ADDRESS_MAP_END

#ifdef UNUSED_FUNCTION
READ8_HANDLER( fortecar_read62 )
{
	return mame_rand(Machine);
}
#endif

static ADDRESS_MAP_START( fortecar_ports, ADDRESS_SPACE_IO, 8 )
	ADDRESS_MAP_FLAGS( AMEF_ABITS(8) )
	AM_RANGE(0x40, 0x40) AM_WRITE(MWA8_NOP)
	AM_RANGE(0x41, 0x41) AM_WRITE(MWA8_NOP)
//  AM_RANGE(0x62, 0x62) AM_READ(fortecar_read62) // nvram eeprom?
ADDRESS_MAP_END


static INPUT_PORTS_START( fortecar )
	PORT_START	/* 8bit */
	PORT_DIPNAME( 0x01, 0x01, DEF_STR( Unknown ) )
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

	PORT_START	/* 8bit */
	PORT_DIPNAME( 0x01, 0x01, DEF_STR( Unknown ) )
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

static const gfx_layout tiles8x8_layout =
{
	8,8,
	RGN_FRAC(1,1),
	2,
	{ 0,4 },
	{ 8,9,10,11,0, 1, 2, 3 },
	{ 0*16, 1*16, 2*16, 3*16, 4*16, 5*16, 6*16, 7*16 },
	16*8
};

static GFXDECODE_START( fortecar )
	GFXDECODE_ENTRY( REGION_GFX1, 0, tiles8x8_layout, 0, 16 )
GFXDECODE_END

static VIDEO_START(fortecar)
{
}

static VIDEO_UPDATE(fortecar)
{
	int x,y,count;
	count = 0;
	fillbitmap(bitmap,0,cliprect);
	for (y=0;y<32;y++)
	{
		for(x=0;x<64;x++)
		{
			int tile;

			tile = fortecar_ram[0x800+(count*4)+1];

			drawgfx(bitmap,machine->gfx[0],tile,0,0,0,x*8,y*8,cliprect,TRANSPARENCY_PEN,0);
			count++;

		}
	}
	return 0;
}


static MACHINE_DRIVER_START( fortecar )
	/* basic machine hardware */
	MDRV_CPU_ADD(Z80,8000000)		 /* ? MHz */
	MDRV_CPU_PROGRAM_MAP(fortecar_map,0)
	MDRV_CPU_IO_MAP(fortecar_ports,0)

	MDRV_CPU_VBLANK_INT(nmi_line_pulse,1)

	/* video hardware */
	MDRV_SCREEN_ADD("main", RASTER)
	MDRV_SCREEN_REFRESH_RATE(60)
	MDRV_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(0))
	MDRV_SCREEN_FORMAT(BITMAP_FORMAT_INDEXED16)
	MDRV_SCREEN_SIZE(512, 256)
	MDRV_SCREEN_VISIBLE_AREA(0, 512-1, 0, 256-1)

	MDRV_GFXDECODE(fortecar)
	MDRV_PALETTE_LENGTH(0x100)

	MDRV_VIDEO_START(fortecar)
	MDRV_VIDEO_UPDATE(fortecar)
MACHINE_DRIVER_END

ROM_START( fortecar )
	ROM_REGION( 0x10000, REGION_CPU1, 0 )
	ROM_LOAD( "fortecar.u7", 0x00000, 0x10000, CRC(2a4b3429) SHA1(8fa630dac949e758678a1a36b05b3412abe8ae16)  )

	ROM_REGION( 0x30000, REGION_GFX1, 0 )
	ROM_LOAD( "fortecar.u38", 0x00000, 0x10000, CRC(c2090690) SHA1(f0aa8935b90a2ab6043555ece69f926372246648) )
	ROM_LOAD( "fortecar.u39", 0x10000, 0x10000, CRC(fc3ddf4f) SHA1(4a95b24c4edb67f6d59f795f86dfbd12899e01b0) )
	ROM_LOAD( "fortecar.u40", 0x20000, 0x10000, CRC(9693bb83) SHA1(e3e3bc750c89a1edd1072ce3890b2ce498dec633) )
ROM_END


GAME( 19??, fortecar, 0, fortecar, fortecar, 0, ROT0, "unknown", "Forte Card",GAME_NO_SOUND|GAME_NOT_WORKING )
