/*

Buster

Video Fruit Machine

*/

#include "emu.h"
#include "cpu/z80/z80.h"
#include "sound/ay8910.h"

static UINT8 *buster_rom;
static UINT8 *buster_vram;

static VIDEO_START(buster)
{

}

static VIDEO_UPDATE(buster)
{
	const gfx_element *gfx = screen->machine->gfx[0];
	int count = 0x0000;

	int y,x;


	for (y=0;y<64;y++)
	{
		for (x=0;x<32;x++)
		{
			int tile = (buster_vram[count+1])|(buster_vram[count]<<8);
			//int colour = tile>>12;
			drawgfx_opaque(bitmap,cliprect,gfx,tile,0,0,0,x*8,y*4);

			count+=2;
		}
	}
	return 0;
}

static ADDRESS_MAP_START( mainmap, ADDRESS_SPACE_PROGRAM, 8 )
	AM_RANGE(0x0000, 0x3fff) AM_RAM AM_BASE(&buster_rom)
	AM_RANGE(0x4000, 0x47ff) AM_RAM
	AM_RANGE(0x5000, 0x5fff) AM_RAM AM_BASE(&buster_vram)
//  AM_RANGE(0x6000, 0x6000) MC6845 address
//  AM_RANGE(0x6001, 0x6001) MC6845 data
	AM_RANGE(0x7000, 0xafff) AM_ROM
ADDRESS_MAP_END



static INPUT_PORTS_START( buster )
INPUT_PORTS_END

static const gfx_layout tiles8x8_layout =
{
	8,4,
	RGN_FRAC(1,3),
	3,
	{ RGN_FRAC(0,3),RGN_FRAC(1,3),RGN_FRAC(2,3) },
	{ 0,1,2,3,4,5,6,7 },
	{ 0*8, 1*8, 2*8, 3*8 },
	8*4
};

static GFXDECODE_START( buster )
	GFXDECODE_ENTRY( "gfx1", 0, tiles8x8_layout, 0, 16 )
GFXDECODE_END

static MACHINE_DRIVER_START( buster )
	/* basic machine hardware */
	MDRV_CPU_ADD("maincpu", Z80,8000000)		 /* ? MHz */
	MDRV_CPU_PROGRAM_MAP(mainmap)
	MDRV_CPU_VBLANK_INT("screen", irq0_line_hold)

	/* video hardware */
	MDRV_SCREEN_ADD("screen", RASTER)
	MDRV_SCREEN_REFRESH_RATE(60)
	MDRV_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(0))
	MDRV_SCREEN_FORMAT(BITMAP_FORMAT_INDEXED16)
	MDRV_SCREEN_SIZE(256, 256)
	MDRV_SCREEN_VISIBLE_AREA(0, 256-1, 16, 256-16-1)

	MDRV_GFXDECODE(buster)
	MDRV_PALETTE_LENGTH(0x100)

	MDRV_VIDEO_START(buster)
	MDRV_VIDEO_UPDATE(buster)

	MDRV_SPEAKER_STANDARD_MONO("mono")

	MDRV_SOUND_ADD("aysnd", AY8910, 1500000/2)
//  MDRV_SOUND_CONFIG(ay8910_config)
	MDRV_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.25)
MACHINE_DRIVER_END


ROM_START( buster )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "vf-1.bin", 0x00000, 0x1000, CRC(571c32fe) SHA1(a61a052a4caf4430be5c5377934295bb90bb903a) )
	ROM_LOAD( "vf-2.bin", 0x01000, 0x1000, CRC(fefe8783) SHA1(aed2bae4cf531dc994b50be9968fa5e10b61f2b8) )
	ROM_LOAD( "vf-3.bin", 0x02000, 0x1000, CRC(9dd9be43) SHA1(5319ef3b43236abd38138adef87a7701f91afd1d))
	ROM_LOAD( "vf-4.bin", 0x03000, 0x1000, CRC(90dd550b) SHA1(a0a26031aada35f6d6c4fa5af9b75e594d8039d0) )
	ROM_COPY( "maincpu",     0x00000, 0x7000, 0x4000 )

	ROM_REGION( 0x6000, "gfx1", 0 )
	ROM_LOAD( "b-red.bin", 0x00000, 0x2000, CRC(6e3ea232) SHA1(dcf76a1ee12517bd00c7b10aaeda0fa2fcec941e) )
	ROM_LOAD( "b-grn.bin", 0x02000, 0x2000, CRC(acdbb44f) SHA1(41f6a2d4a6b12f506588379f2ed3df48fbc8184e) )
	ROM_LOAD( "b-blu.bin", 0x04000, 0x2000, CRC(3b6bfe7b) SHA1(1888149a2ef85db59845d7e6e9227449f80c8f22) )
ROM_END

static DRIVER_INIT( buster )
{
	UINT8 *ROM = memory_region(machine, "maincpu");
//  vram = auto_alloc_array(machine, UINT8, 0x2000);
	memcpy(buster_rom, ROM, 0x4000);
}

GAME( 1987, buster,  0,    buster, buster,  buster, ROT0, "Marian Electronics Ltd.", "Buster", GAME_NOT_WORKING|GAME_NO_SOUND )
