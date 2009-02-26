/* Taito
 'Parent Jack'
 (Medal Game)
*/



#include "driver.h"
#include "cpu/m68000/m68000.h"

static UINT16* parentj_video;

static VIDEO_START(parentj)
{


}

static VIDEO_UPDATE(parentj)
{
	const gfx_element *gfx = screen->machine->gfx[0];
	int count = 0x0c000/2;

	int y,x;


	for (y=0;y<64;y++)
	{
		for (x=0;x<64;x++)
		{
			int tile = parentj_video[count];
			//int colour = tile>>12;
			drawgfx(bitmap,gfx,tile,9,0,0,x*16,y*16,cliprect,TRANSPARENCY_NONE,0);

			count++;
		}
	}


	return 0;
}

static ADDRESS_MAP_START( parentj_map, ADDRESS_SPACE_PROGRAM, 16 )
	AM_RANGE(0x000000, 0x01ffff) AM_ROM
	AM_RANGE(0x100000, 0x103fff) AM_RAM


	AM_RANGE(0x200004, 0x200005) AM_WRITE(SMH_NOP)

	AM_RANGE(0x400000, 0x41ffff) AM_RAM AM_BASE(&parentj_video)

	AM_RANGE(0x500800, 0x500fff) AM_RAM AM_WRITE(paletteram16_xBBBBBGGGGGRRRRR_word_w) AM_BASE(&paletteram16)


ADDRESS_MAP_END



static const gfx_layout parentj_layout =
{
	16,16,
	RGN_FRAC(1,8),
	4,
	{ 0,1,2,3 },
	{ RGN_FRAC(7,8)+4, RGN_FRAC(7,8)+0,
	  RGN_FRAC(6,8)+4, RGN_FRAC(6,8)+0,
	  RGN_FRAC(5,8)+4, RGN_FRAC(5,8)+0,
	  RGN_FRAC(4,8)+4, RGN_FRAC(4,8)+0,
	  RGN_FRAC(3,8)+4, RGN_FRAC(3,8)+0,
	  RGN_FRAC(2,8)+4, RGN_FRAC(2,8)+0,
	  RGN_FRAC(1,8)+4, RGN_FRAC(1,8)+0,
	  RGN_FRAC(0,8)+4, RGN_FRAC(0,8)+0
	  },
	{ 0,8,16,24,32,40,48,56,64,72,80,88,96,104,112,120},

	1*128
};

static GFXDECODE_START( parentj )
	GFXDECODE_ENTRY( "gfx1", 0, parentj_layout,  0x0, 0x400/16  )
GFXDECODE_END


static INPUT_PORTS_START( parentj )
INPUT_PORTS_END

static MACHINE_DRIVER_START( parentj )
	MDRV_CPU_ADD("maincpu", M68000, 16000000)	// ?
	MDRV_CPU_PROGRAM_MAP(parentj_map,0)
	MDRV_CPU_VBLANK_INT("screen", irq4_line_hold) // 4/5/6 valid?

	MDRV_GFXDECODE(parentj)

	MDRV_SCREEN_ADD("screen", RASTER)
	MDRV_SCREEN_REFRESH_RATE(60)
	MDRV_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(0))
	MDRV_SCREEN_FORMAT(BITMAP_FORMAT_INDEXED16)
	MDRV_SCREEN_SIZE(64*16, 64*16)
	MDRV_SCREEN_VISIBLE_AREA(32*16, 64*16-1, 0*16, 32*16-1)

	MDRV_PALETTE_LENGTH(0x400)

	MDRV_VIDEO_START(parentj)
	MDRV_VIDEO_UPDATE(parentj)
MACHINE_DRIVER_END


ROM_START( parentj )
	ROM_REGION( 0x20000, "maincpu", 0 ) /* 68000 Code */
	ROM_LOAD16_BYTE( "c42-13.21", 0x00000, 0x10000, CRC(823623eb) SHA1(7302cc0ac532f6190ae35218ea05bf8cf11fd687) )
	ROM_LOAD16_BYTE( "c42-12.20", 0x00001, 0x10000, CRC(8654b0ab) SHA1(edd23a731c1c60cab353e51ef5e66d33bc3fde61) )

	ROM_REGION( 0x100000, "gfx1", 0 )
	ROM_LOAD( "c42-05.06", 0x00000, 0x20000, CRC(7af0d45d) SHA1(bc527b74185596e4e77b34d08eb3e1678614b451) )
	ROM_LOAD( "c42-04.05", 0x20000, 0x20000, CRC(133009a1) SHA1(fae5dd600384790225c24a62d1f8a00f0366dae9) )
	ROM_LOAD( "c42-09.13", 0x40000, 0x20000, CRC(ba35fb03) SHA1(b76e50d298ccc0f230c865b563cd8e02866a4ffb) )
	ROM_LOAD( "c42-08.12", 0x60000, 0x20000, CRC(7fae35a7) SHA1(f4bc6c6fd4afc167eb36b8f16589e1bfd729085e) )
	ROM_LOAD( "c42-07.10", 0x80000, 0x20000, CRC(f92c6f03) SHA1(ff42318ee425b423b67e2cec1fe3ef9d9785ebf6) )
	ROM_LOAD( "c42-06.09", 0xa0000, 0x20000, CRC(3685febd) SHA1(637946377f6d934f791d52e9790c91f60a5b2c65) )
	ROM_LOAD( "c42-11.17", 0xc0000, 0x20000, CRC(5d8d3c59) SHA1(c8a8a957ac9f2f1c346b4504495893c71fbfe14b) )
	ROM_LOAD( "c42-10.16", 0xe0000, 0x20000, CRC(e85e536e) SHA1(9ed9e316869333338e39cb0d1293e3380861a3ca) )

	ROM_REGION( 0x2dd, "misc", 0 )
	ROM_LOAD( "ampal22v10a-0233.c42", 0x000, 0x2dd, CRC(0c030a81) SHA1(0f8198df2cb046683d2db9ac8e609cdff53083ed) )
ROM_END

GAME( 1989, parentj,    0,        parentj,   parentj,    0, ROT0,  "Taito", "Parent Jack", GAME_NOT_WORKING|GAME_NO_SOUND )
