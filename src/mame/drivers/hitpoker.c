/***************************************************************************

'Hit Poker'?

cpu hd46505SP (HD6845SP) <- ha, ha, ha... --"

other : ZC407615CFN (infralink)

chrystal : no idea

ram km6264BL X3
TMM 2018 X2
DALLAS REAL TIME CLK DS17487-5
SOUND YM2149F
DIP 1X4

============================================================================

Skeleton driver, just the main CPU has been identified (a MC68HC11). This one
requires some mods to the cpu core in order to start to work...
Many thanks to Olivier Galibert for the identify effort ;-)

How to initialize this:
- let it run then soft reset (it wants that ram at 0-0xff is equal to 0xff,
  nvram maybe?);
- set a bp 10c5 then pc=10c8, it currently fails the rom checksum for
  whatever reason (should be 0 and it returns 0x89), note that area
  0xf00-0xfff isn't tested at all, maybe that belongs to somewhere else?

***************************************************************************/


#include "driver.h"
#include "cpu/mc68hc11/mc68hc11.h"

static UINT8 *work_ram;

VIDEO_START(hitpoker)
{

}

VIDEO_UPDATE(hitpoker)
{
	UINT8 *VRAM = memory_region(screen->machine, "vram");
	const gfx_element *gfx = screen->machine->gfx[0];
	int count = 0;
	int y,x;

	bitmap_fill(bitmap, cliprect, 0);

	for (x=0;x<64;x++)
	{
		for (y=0;y<42;y+=2)
		{
			int tile;
			tile = ((VRAM[count]<<8)|(VRAM[count+1])) & 0x1fff;
			drawgfx(bitmap,gfx,tile,0,0,0,x*16,(y+0)*8,cliprect,TRANSPARENCY_NONE,0);

			tile = ((VRAM[count+2]<<8)|(VRAM[count+3])) & 0x1fff;
			drawgfx(bitmap,gfx,tile,0,0,0,x*16,(y+1)*8,cliprect,TRANSPARENCY_NONE,0);

			count+=4;
		}
	}

	return 0;
}

/* It wants that the even/odd memory is equal for this, 8-bit vram on a 16-bit wide bus? */
static READ8_HANDLER( hitpoker_work_ram_r )
{
	return work_ram[offset & ~1];
}

static WRITE8_HANDLER( hitpoker_work_ram_w )
{
	work_ram[offset & ~1] = data;
}

static WRITE8_HANDLER( hitpoker_vram_w )
{
	UINT8 *VRAM = memory_region(space->machine, "vram");

	VRAM[offset] = data;
}

static WRITE8_HANDLER( hitpoker_cram_w )
{
	UINT8 *CRAM = memory_region(space->machine, "cram");

	CRAM[offset] = data;
}

/* overlap empty rom addresses */
static ADDRESS_MAP_START( main_map, ADDRESS_SPACE_PROGRAM, 8 )
	AM_RANGE(0x0000, 0x00ff) AM_RAM // stack ram
	AM_RANGE(0x1000, 0x103f) AM_RAM // hw regs?
	AM_RANGE(0xb600, 0xbeff) AM_READWRITE(hitpoker_work_ram_r,hitpoker_work_ram_w) AM_BASE(&work_ram)
	AM_RANGE(0x8000, 0x8fff) AM_WRITE(hitpoker_vram_w)
	AM_RANGE(0xc000, 0xcfff) AM_WRITE(hitpoker_cram_w)
	AM_RANGE(0x0000, 0xffff) AM_ROM
ADDRESS_MAP_END

static INPUT_PORTS_START( hitpoker )
INPUT_PORTS_END



static const gfx_layout hitpoker_layout =
{
	16,8,
	RGN_FRAC(1,2),
	8,
	{ RGN_FRAC(1,2)+0,RGN_FRAC(1,2)+4,RGN_FRAC(1,2)+8,RGN_FRAC(1,2)+12,0,4,8,12 },
	{ 0,1,2,3,
	  16,17,18,19,
	8*32+0,8*32+1,8*32+2,8*32+3,
	8*32+16,8*32+17,8*32+18,8*32+19 },
	{ 0*32, 1*32, 2*32, 3*32,4*32,5*32,6*32,7*32 },

	8*64
};

static PALETTE_INIT( hitpoker )
{
	#if 0
	int x,r,g,b;

	for(x=0;x<0x100;x++)
	{
		r = (x & 0xf)*0x10;
		g = ((x & 0x3c)>>2)*0x10;
		b = ((x & 0xf0)>>4)*0x10;
		palette_set_color(machine,x,MAKE_RGB(r,g,b));
	}
	#endif
}

static GFXDECODE_START( hitpoker )
	GFXDECODE_ENTRY( "gfx1", 0, hitpoker_layout,   0x0, 2  )
GFXDECODE_END

static MACHINE_DRIVER_START( hitpoker )
	MDRV_CPU_ADD("maincpu", MC68HC11,2000000)
	MDRV_CPU_PROGRAM_MAP(main_map)
	MDRV_CPU_VBLANK_INT("screen", irq0_line_hold)

	/* video hardware */
	MDRV_SCREEN_ADD("screen", RASTER)
	MDRV_SCREEN_REFRESH_RATE(60)
	MDRV_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(0))
	MDRV_SCREEN_FORMAT(BITMAP_FORMAT_INDEXED16)
	MDRV_SCREEN_SIZE(1024, 512)
	MDRV_SCREEN_VISIBLE_AREA(0, 1024-1, 0, 512-1)

	MDRV_GFXDECODE(hitpoker)
	MDRV_PALETTE_LENGTH(0x100)
	MDRV_PALETTE_INIT(hitpoker)

	MDRV_VIDEO_START(hitpoker)
	MDRV_VIDEO_UPDATE(hitpoker)
MACHINE_DRIVER_END


ROM_START( hitpoker )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "u4.bin",         0x0000, 0x10000, CRC(0016497a) SHA1(017320bfe05fea8a48e26a66c0412415846cee7c) )

	ROM_REGION( 0x1000, "vram", ROMREGION_ERASE00 )

	ROM_REGION( 0x1000, "cram", ROMREGION_ERASE00 )


	ROM_REGION( 0x100000, "gfx1", 0 ) // tile 0x4c8 seems to contain something non-gfx related, could be tilemap / colour data, check!
	ROM_LOAD16_BYTE( "u42.bin",         0x00001, 0x40000, CRC(cbe56fec) SHA1(129bfd10243eaa7fb6a087f96de90228e6030353) )
	ROM_LOAD16_BYTE( "u43.bin",         0x00000, 0x40000, CRC(6c0d4283) SHA1(04a4fd82f5cc0ed9f548e490ac67d287227073c3) )
	ROM_LOAD16_BYTE( "u44.bin",         0x80001, 0x40000, CRC(e23d5f30) SHA1(ca8855301528aa4eeff40cb820943b4268f8596e) ) // the 'adult images' are 8bpp
	ROM_LOAD16_BYTE( "u45.bin",         0x80000, 0x40000, CRC(e65b3e52) SHA1(c0c1a360a4a1823bf71c0a4105ff41f4102862e8) ) //  the first part of these 2 is almost empty as the standard gfx are 4bpp
ROM_END

GAME( 1997, hitpoker,  0,    hitpoker, hitpoker,  0, ROT0, "Accept Ltd.", "Hit Poker? (Bulgaria)", GAME_NOT_WORKING|GAME_NO_SOUND )

