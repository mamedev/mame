/*   Miracle Derby - Ascot

   - has the same GX61A01 custom (blitter?) as homedata.c and a 'similar' CPU setup (this has more CPUs)
     and similar board / rom numbering (X**-)

     The drivers can probably be merged later, although the current per-game handling of the blitter in
     homedata.c should be looked at.



        Notes from Stefan Lindberg:

        Eprom "x70_a04.5g" had wires attached to it, pin 2 and 16 was joined and pin 1,32,31,30 was joined, i
        removed them and read the eprom as the type it was (D27c1000D).

        Measured frequencies:
        MBL68B09E = 2mhz
        MBL68B09E = 2mhz
        z80 = 4mhz
        YM2203 = 2mhz

        See included PCB pics.



        Roms:

        Name              Size     CRC32         Chip Type
        ---------------------------------------------------------------------------------
        x70a07.8l         256      0x7d4c9712    82s129
        x70a08.7l         256      0xc4e77174    82s129
        x70a09.6l         256      0xd0187957    82s129
        x70_a03.8g        32768    0x4e298b2d    27c256
        x70_a04.5g        131072   0x14392fdb    D27c1000D
        x70_a11.1g        32768    0xb394eef7    27c256
        x70_b02.12e       32768    0x76c9bb6f    27c256
        x70_c01.14e       65536    0xd79d072d    27c512



*/

/* clocks are 16mhz and 9mhz */

#include "driver.h"
#include "cpu/z80/z80.h"
#include "cpu/m6809/m6809.h"
#include "sound/2203intf.h"

static VIDEO_START(mirderby)
{

}

static VIDEO_UPDATE(mirderby)
{
	return 0;
}

static ADDRESS_MAP_START( cpu0_map, ADDRESS_SPACE_PROGRAM, 8 )
	AM_RANGE(0x0000, 0x7fff) AM_ROM
ADDRESS_MAP_END

static ADDRESS_MAP_START( cpu1_map, ADDRESS_SPACE_PROGRAM, 8 )
	AM_RANGE(0x0000, 0xffff) AM_ROM
ADDRESS_MAP_END

static ADDRESS_MAP_START( cpu2_map, ADDRESS_SPACE_PROGRAM, 8 )
	AM_RANGE(0x8000, 0xffff) AM_ROM
ADDRESS_MAP_END



static PALETTE_INIT( mirderby )
{
	int i;

	for (i = 0; i < 0x100; i++)
	{
		int r,g,b;
		r = color_prom[0x000+i];
		g = color_prom[0x100+i];
		b = color_prom[0x200+i];

		palette_set_color_rgb(machine,i,pal4bit(r),pal4bit(g),pal4bit(b));
	}
}

static const gfx_layout char_layout =
{
	8,8,
	RGN_FRAC(1,1),
	4,
	{ 0, 1, 2, 3 },
	{ 0*4, 1*4, 2*4, 3*4, 4*4, 5*4, 6*4, 7*4 },
	{ 0*32, 1*32, 2*32, 3*32, 4*32, 5*32, 6*32, 7*32 },
	32*8
};

static GFXDECODE_START( mirderby )
	GFXDECODE_ENTRY( "gfx1", 0, char_layout, 0x0000, 0x10 )
	GFXDECODE_ENTRY( "gfx2", 0, char_layout, 0x0000, 0x10 )
GFXDECODE_END

static MACHINE_DRIVER_START( mirderby )

	/* basic machine hardware */
	MDRV_CPU_ADD("cpu0", Z80, 16000000/4)	/* 4 Mhz */
	MDRV_CPU_PROGRAM_MAP(cpu0_map)

	MDRV_CPU_ADD("cpu1", M6809, 16000000/8)	/* 2 Mhz */
	MDRV_CPU_PROGRAM_MAP(cpu1_map)

	MDRV_CPU_ADD("cpu2", M6809, 16000000/8)	/* 2 Mhz */
	MDRV_CPU_PROGRAM_MAP(cpu2_map)

	MDRV_QUANTUM_TIME(HZ(6000))

	/* video hardware */
	MDRV_SCREEN_ADD("screen", RASTER)
	MDRV_SCREEN_REFRESH_RATE(59)
	MDRV_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(0))
	MDRV_SCREEN_FORMAT(BITMAP_FORMAT_INDEXED16)
	MDRV_SCREEN_SIZE(64*8, 32*8)
	MDRV_SCREEN_VISIBLE_AREA(0*8, 54*8-1, 2*8, 30*8-1)

	MDRV_GFXDECODE(mirderby)
	MDRV_PALETTE_LENGTH(0x100)

	MDRV_PALETTE_INIT(mirderby)
	MDRV_VIDEO_START(mirderby)
	MDRV_VIDEO_UPDATE(mirderby)

	/* sound hardware */
	MDRV_SPEAKER_STANDARD_MONO("mono")

	MDRV_SOUND_ADD("ym", YM2203, 2000000)
	MDRV_SOUND_ROUTE(0, "mono", 0.25)
	MDRV_SOUND_ROUTE(1, "mono", 0.25)
	MDRV_SOUND_ROUTE(2, "mono", 0.25)
	MDRV_SOUND_ROUTE(3, "mono", 1.0)
MACHINE_DRIVER_END


static INPUT_PORTS_START( mirderby )
INPUT_PORTS_END


ROM_START( mirderby )
	ROM_REGION( 0x8000, "cpu0", 0 ) /* Z80 Code */
	ROM_LOAD( "x70_a11.1g", 0x2000, 0x6000, CRC(b394eef7) SHA1(a646596d09b90eda44aaf8ccbf8f3fccfd3d5dad) ) // first 0x6000 bytes are blank!
	ROM_CONTINUE(0x0000, 0x2000) // main z80 code is here

	ROM_REGION( 0x10000, "cpu1", 0 ) /* M6809 code */
	ROM_LOAD( "x70_c01.14e", 0x00000, 0x10000, CRC(d79d072d) SHA1(8e189931de9c4eb520c1ec2d0898d8eaba0f01b5) )

	ROM_REGION( 0x10000, "cpu2", 0 ) /* M6809 code */
	ROM_LOAD( "x70_b02.12e", 0x8000, 0x8000, CRC(76c9bb6f) SHA1(dd8893f3082d33d366247295e9531f8879c219c5) )

	ROM_REGION( 0x8000, "gfx1", 0 ) // horse gfx
	ROM_LOAD( "x70_a03.8g", 0x0000, 0x8000, CRC(4e298b2d) SHA1(ae78327d1f30c8d19ef772b82803dab4d6b7b919))

	ROM_REGION( 0x20000, "gfx2", 0 ) // fonts etc.
	ROM_LOAD( "x70_a04.5g", 0x0000, 0x20000, CRC(14392fdb) SHA1(dafdce473b2d2ebbdbf49fbd12f85c1ad69b2877) )

	ROM_REGION( 0x300, "proms", 0 ) /* colours */
	ROM_LOAD( "x70a07.8l", 0x000, 0x100, CRC(7d4c9712) SHA1(fe2a89841fdf5e4fd6cd41478ad2f29d28bed54d) )
	ROM_LOAD( "x70a08.7l", 0x100, 0x100, CRC(c4e77174) SHA1(ada238ded69f01b4daeb0159a2c5c422977bb95e) )
	ROM_LOAD( "x70a09.6l", 0x200, 0x100, CRC(d0187957) SHA1(6b36c1bccad24708cfa2fc78da08313f9bcfdbc0) )
ROM_END

static DRIVER_INIT( mirderby )
{

}

GAME( 1988, mirderby, 0, mirderby, mirderby, mirderby,   ROT0, "Home Data?", "Miracle Derby - Ascot", GAME_NO_SOUND|GAME_NOT_WORKING )

