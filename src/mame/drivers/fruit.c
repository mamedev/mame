/*

Fruit?

Video Fruit Machine

- Doesn't make much sense,anything (including rom load/rom dump etc.) might be bad or heavily protected.
*/

#include "driver.h"
#include "cpu/z80/z80.h"

static UINT8 *fruit_vram;

static VIDEO_START(fruit)
{

}

static VIDEO_UPDATE(fruit)
{
	const gfx_element *gfx = screen->machine->gfx[1];
	int count = 0x0000;

	int y,x;


	for (y=0;y<64;y++)
	{
		for (x=0;x<64;x++)
		{
			int tile = (fruit_vram[count]);
			//int colour = tile>>12;
			drawgfx(bitmap,gfx,tile,0,0,0,x*8,y*8,cliprect,TRANSPARENCY_NONE,0);

			count++;
		}
	}

	return 0;
}

static ADDRESS_MAP_START( mainmap, ADDRESS_SPACE_PROGRAM, 8 )
	AM_RANGE(0x0000, 0x2fff) AM_ROM
	AM_RANGE(0xa000, 0xafff) AM_RAM
	AM_RANGE(0xb000, 0xbfff) AM_RAM AM_BASE(&fruit_vram)
	AM_RANGE(0xd000, 0xdfff) AM_ROM //second rom mirrors there
	AM_RANGE(0xe000, 0xefff) AM_ROM //first rom mirrors there
	AM_RANGE(0xf000, 0xffff) AM_RAM
ADDRESS_MAP_END



static INPUT_PORTS_START( fruit )
INPUT_PORTS_END

static const gfx_layout tiles32x32_layout =
{
	32,32,
	RGN_FRAC(1,3),
	3,
	{ RGN_FRAC(0,3),RGN_FRAC(1,3),RGN_FRAC(2,3) },
	{ 0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16,17,18,19,20,21,22,23,24,25,26,27,28,29,30,31 },
	{ 0*32, 1*32, 2*32, 3*32,4*32,5*32,6*32,7*32,8*32,9*32,10*32,11*32,12*32,13*32,14*32,15*32,16*32,17*32,18*32,19*32,20*32,21*32,22*32,23*32,24*32,25*32,26*32,27*32,28*32,29*32,30*32,31*32 },
	32*32
};

static const gfx_layout tiles8x8_layout =
{
	8,8,
	RGN_FRAC(1,1),
	1,
	{ 0 },
	{ 0,1,2,3,4,5,6,7 },
	{ 0*8,1*8,2*8,3*8,4*8,5*8,6*8,7*8},
	8*8
};
static GFXDECODE_START( fruit )
	GFXDECODE_ENTRY( "gfx1", 0, tiles32x32_layout, 0, 16 )
	GFXDECODE_ENTRY( "gfx2", 0, tiles8x8_layout, 0, 16 )
GFXDECODE_END

static MACHINE_DRIVER_START( fruit )
	/* basic machine hardware */
	MDRV_CPU_ADD("main", Z80,8000000)		 /* ? MHz */
	MDRV_CPU_PROGRAM_MAP(mainmap,0)
	MDRV_CPU_VBLANK_INT("main", irq0_line_hold)

	/* video hardware */
	MDRV_SCREEN_ADD("main", RASTER)
	MDRV_SCREEN_REFRESH_RATE(60)
	MDRV_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(0))
	MDRV_SCREEN_FORMAT(BITMAP_FORMAT_INDEXED16)
	MDRV_SCREEN_SIZE(512, 512)
	MDRV_SCREEN_VISIBLE_AREA(0, 512-1, 0, 512-1)

	MDRV_GFXDECODE(fruit)
	MDRV_PALETTE_LENGTH(0x100)

	MDRV_VIDEO_START(fruit)
	MDRV_VIDEO_UPDATE(fruit)
MACHINE_DRIVER_END



ROM_START( fruit )
	ROM_REGION( 0x10000, "main", 0 )
	ROM_LOAD( "vp-h9.bin",  0x00000, 0x1000, CRC(f595daf7) SHA1(be5abd34fd06f73cd80f5b15902d158e33705c8f) )
	ROM_RELOAD(             0x0e000, 0x1000 )
	ROM_LOAD( "vp-h10.bin", 0x01000, 0x1000, CRC(b0539100) SHA1(763f31f72f55c3322b24e127b37130d37daa5216) )
	ROM_RELOAD(             0x0d000, 0x1000 )
	ROM_LOAD( "vp-h11.bin", 0x02000, 0x1000, CRC(fa176072) SHA1(18203278bb9c505f07390f7b95ecf9ab6d7b7122) )

	ROM_REGION( 0x1800, "gfx1", 0 ) // fruits - 3bpp
	ROM_LOAD( "vp-h5.bin", 0x00000, 0x0800, CRC(dfffe063) SHA1(1b860323fe93b7d010fa35167769555a6bd4a49c) )
	ROM_LOAD( "vp-h6.bin", 0x00800, 0x0800, CRC(355203b8) SHA1(959f3599a24293f392e8b10061c39d3244f34c05) )
	ROM_LOAD( "vp-h7.bin", 0x01000, 0x0800, CRC(7784de8a) SHA1(40851724c9b7ef26964462b5e97ad943df4d56e2) )

	ROM_REGION( 0x800, "gfx2", 0 ) // screen layout stuff - 1bpp
	ROM_LOAD( "vp-h8.bin", 0x00000, 0x0800, CRC(d587e541) SHA1(902b6c4673b8b989d034d60d3c47f2499f100ba2) )
ROM_END

GAME( 1987, fruit,  0,    fruit, fruit,  0, ROT270, "unknown", "Unknown Fruits Game", GAME_NOT_WORKING|GAME_NO_SOUND )
