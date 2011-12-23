/***************************************************************************

 Vega by Olypmia?

 I don't know much about this, and I'm unsure of the dump quality.  There
 were several dumps, and all but one had ROM10 with identical halves, however
 the dump which was in ASCII format appears to have unique data in the 2nd
 half of ROM10 instead, so I'm using that.

 Surface of chips (CPU etc.) is scratched off.

***************************************************************************/

#include "emu.h"
#include "cpu/i8085/i8085.h"
#include "cpu/mcs48/mcs48.h"


class vega_state : public driver_device
{
public:
	vega_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag) { }

};


static ADDRESS_MAP_START( vega_map, AS_PROGRAM, 8 )
	AM_RANGE(0x0000, 0x3fff) AM_ROM
ADDRESS_MAP_END

static INPUT_PORTS_START( vega )
	PORT_START("IN0")
INPUT_PORTS_END



static PALETTE_INIT(vega)
{

}

static SCREEN_UPDATE(vega)
{
	return 0;
}

static MACHINE_CONFIG_START( vega, vega_state )
	MCFG_CPU_ADD("maincpu", I8035, 6000000) // what CPU? what speed?
	MCFG_CPU_PROGRAM_MAP(vega_map)

	/* video hardware */
	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_REFRESH_RATE(60)
	MCFG_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(2500) /* not accurate */)
	MCFG_SCREEN_FORMAT(BITMAP_FORMAT_INDEXED16)
	MCFG_SCREEN_SIZE(32*8, 32*8)
	MCFG_SCREEN_VISIBLE_AREA(0*8, 32*8-1, 0*8, 28*8-1)
	MCFG_SCREEN_UPDATE(vega)

	MCFG_PALETTE_LENGTH(0x100)

	MCFG_PALETTE_INIT(vega)
MACHINE_CONFIG_END

ROM_START( vega )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "rom1.bin",	      0x0000, 0x0800, CRC(a0c0e0af) SHA1(7ccbfe3c23cda4c3a639c89ff4b2f554e2876c98) ) // FIXED BITS (00xxxxxx) (tile attribs?)
	ROM_LOAD( "rom2.bin",	      0x0000, 0x0800, CRC(718da952) SHA1(1a0023be1ee3a48ed3ddb8daddbb49ca3f442d46) )
	ROM_LOAD( "rom3.bin",	      0x0000, 0x0800, CRC(37944311) SHA1(8b20be3d3ca5cb27bef78a73ee7e977fdf76c7f1) )
	ROM_LOAD( "rom4.bin",	      0x0000, 0x0800, CRC(09453d7a) SHA1(75fe96ae25467f82c0725834c6c04a197f50cce7) )
	ROM_LOAD( "rom5.bin",	      0x0000, 0x0800, CRC(be3df449) SHA1(acba1e07bdf9c0e971f47f2433d2760472c4326a) )
	ROM_LOAD( "rom6.bin",	      0x0000, 0x0800, CRC(dc46527c) SHA1(d10a54d8d3ce9ffd8a53bede3d089625aff445a2) )
	ROM_LOAD( "rom7.bin",	      0x0000, 0x0800, CRC(1de564cd) SHA1(7408cd29f1afc111aa695ecb00160d8f7fba7532) )
	ROM_LOAD( "rom8.bin",	      0x0000, 0x0800, CRC(ccb8598c) SHA1(8c4a702f0653bb189db7d8ac4c2a06aacecc0de0) )
	ROM_LOAD( "rom9.bin",	      0x0000, 0x0800, CRC(191c73cd) SHA1(17b1c3790f82b276e55d25ea8a38a3c9cf20bf12) )
	//ROM_LOAD( "rom10.bin",          0x0000, 0x1000, CRC(c7659222) SHA1(86df4f4afe5bfd0b67239353a344724405c32fed) ) // 1ST AND 2ND HALF IDENTICAL
	ROM_LOAD( "rom10a.bin",	      0x0000, 0x1000, CRC(fca9a570) SHA1(598772db11b32518ed6bf5155a19f4f1761a4831) ) // if you convert the ascii dump there is different data in the 2nd half unlike the above dump (it also appears in the ascii dumps of 11/12 too tho)
	ROM_LOAD( "rom11.bin",	      0x0000, 0x0800, CRC(d1896f77) SHA1(5b80bf7aa81508edfae4fa583b4b0077575a300c) )
	ROM_LOAD( "rom12.bin",	      0x0000, 0x0800, CRC(f5f1df2f) SHA1(5851b468702e5e4f085b64afbe7d8b797bb109b5) )

	ROM_LOAD( "r10.bin", 0x0000, 0x0100, CRC(ca5a3627) SHA1(8c632fa9174e336c588074f92f3519b0cf224852) ) // FIXED BITS (0000xxxx)
	ROM_LOAD( "r11.bin", 0x0000, 0x0100, CRC(d8aab14a) SHA1(798feaa929dd7b71266220b568826997acd2a93e) ) // FIXED BITS (000011xx)
	ROM_LOAD( "r8.bin",	 0x0000, 0x0100, CRC(40c9caad) SHA1(ddd427ff4df4cb2d217690efefdd5e53e3add118) ) // FIXED BITS (0000xxxx)
	ROM_LOAD( "r9.bin",	 0x0000, 0x0100, CRC(db0bcea5) SHA1(692bea2d9e28985fe7270a940e9f48ac64bdeaa8) ) // FIXED BITS (0000xxxx)
ROM_END

// code for converting the ASCII dump..
#if 0
UINT8 ascii_to_bin( UINT8 ascii )
{

	if (ascii>=0x30 && ascii <= 0x39)
	{
		return ascii-0x30;
	}

	if (ascii>=0x41 && ascii <= 0x46)
	{
		return ascii-0x37;
	}

	fatalerror("bad char\n");
	return 0;
}

DRIVER_INIT(vegaa)
{
	UINT8* buf = (UINT8*)malloc(0x10000);
	UINT8* rom = machine.region("maincpu")->base();
	int i;
	int count = 0;
	// last 0xc bytes of file are just some settings, ignore
	for (i=0;i<0x2e*0x1000;i+=0x2e)
	{
		// first 0x9 bytes are the offset details, ignore
		// bytes 0x2a-0x2e are checksum / newline, ignore
		int j;
		for (j=0xa; j<0x2a;j+=2)
		{
			UINT8 l=rom[i+j+0];
			UINT8 r=rom[i+j+1];
			UINT8 num;

			l = ascii_to_bin(l);
			r = ascii_to_bin(r);

			num = l << 4 | r;

			buf[count] = num;
			count++;
		}
	}

	{
		FILE *fp;
		char filename[256];
		sprintf(filename,"vega_%s", machine.system().name);
		fp=fopen(filename, "w+b");
		if (fp)
		{
			fwrite(buf, 0x10000, 1, fp);
			fclose(fp);
		}
	}
}
#endif

GAME( 19??, vega,   0, vega, vega, 0, ROT270, "Olympia?", "Vega", GAME_IS_SKELETON )
