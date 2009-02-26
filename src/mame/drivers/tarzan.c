
/* Old IGS game called Tarzan, don't know much about it, could be gambling */
/* CPU might not be z80 (maybe z180)

  decryption is incomplete?, first part of code doesn't seem right?

  graphics area also encrypted

*/

#include "driver.h"
#include "cpu/z80/z80.h"


static VIDEO_START(tarzan)
{
}

static VIDEO_UPDATE(tarzan)
{

	return 0;
}

static ADDRESS_MAP_START( tarzan_map, ADDRESS_SPACE_PROGRAM, 8 )
	AM_RANGE(0x0000, 0x7fff) AM_ROM
ADDRESS_MAP_END


static INPUT_PORTS_START( tarzan )
INPUT_PORTS_END


static const gfx_layout tarzan_layout =
{
	8,8,
	RGN_FRAC(1,1),
	4,
	{ 0*8,1*8,2*8,3*8 },
	{ 0,1,2,3,4,5,6,7 },
	{ 0*8, 4*8, 8*8,12*8,16*8,20*8,24*8,28*8},
	32*8
};


static GFXDECODE_START( tarzan )
	GFXDECODE_ENTRY( "gfx1", 0, tarzan_layout,   0x0, 2  )
	GFXDECODE_ENTRY( "gfx2", 0, tarzan_layout,  0x0, 2  )
GFXDECODE_END




static MACHINE_DRIVER_START( tarzan )
	/* basic machine hardware */
	MDRV_CPU_ADD("maincpu", Z80,8000000)		 /* ? */
	MDRV_CPU_PROGRAM_MAP(tarzan_map,0)

	/* video hardware */
	MDRV_SCREEN_ADD("screen", RASTER)
	MDRV_SCREEN_REFRESH_RATE(60)
	MDRV_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(0))
	MDRV_SCREEN_FORMAT(BITMAP_FORMAT_INDEXED16)
	MDRV_SCREEN_SIZE(256, 256)
	MDRV_SCREEN_VISIBLE_AREA(0, 256-1, 0, 256-1)

	MDRV_PALETTE_LENGTH(0x100)
	MDRV_GFXDECODE(tarzan)

	MDRV_VIDEO_START(tarzan)
	MDRV_VIDEO_UPDATE(tarzan)
MACHINE_DRIVER_END

static DRIVER_INIT( tarzan )
{
	UINT16* ROM = (UINT16*)memory_region(machine,"maincpu");
	int i;
	int size = 0x040000;

	for(i=0; i<size/2; i++)
	{
		UINT16 x = ROM[i];

		if((i & 0x10c0) == 0x0000)
			x ^= 0x0001;

		if((i & 0x0010) == 0x0010 || (i & 0x0130) == 0x0020)
			x ^= 0x0404;

		if((i & 0x00d0) != 0x0010)
			x ^= 0x1010;

		if(((i & 0x0008) == 0x0008)^((i & 0x10c0) == 0x0000))
			x ^= 0x0100;

		ROM[i] = x;
	}

	#if 0
	{
		FILE *fp;
		char filename[256];
		sprintf(filename,"decrypted_%s", machine->gamedrv->name);
		fp=fopen(filename, "w+b");
		if (fp)
		{
			fwrite(ROM, 0x40000, 1, fp);
			fclose(fp);
		}
	}
	#endif
}

ROM_START( tarzan )
	ROM_REGION( 0x040000, "maincpu", 0 )
	ROM_LOAD( "0228-u16.bin", 0x00000, 0x040000, CRC(e6c552a5) SHA1(f156de9459833474c85a1f5b35917881b390d34c)  )

	ROM_REGION( 0x080000, "gfx1", 0 )
	ROM_LOAD( "0228-u21.bin", 0x00000, 0x080000, CRC(80aaece4) SHA1(07cad92492c5de36c3915867ed4c6544b1a30c07)  )

	ROM_REGION( 0x080000, "gfx2", 0 )
	ROM_LOAD( "0228-u6.bin", 0x00000, 0x080000, CRC(55e94832) SHA1(b15409f4f1264b6d1218d5dc51c5bd1de2e40284)  )

ROM_END


GAME( 199?, tarzan, 0, tarzan, tarzan, tarzan, ROT0, "IGS", "Tarzan",GAME_NOT_WORKING|GAME_NO_SOUND )
