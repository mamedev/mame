/*

3 Super 8

this dump is bad


Produttore  ?Italy?
N.revisione
CPU

1x 24mhz osc
2x fpga
1x z840006
1x PIC16c65a-20/p
1x 6295 oki

ROMs

Note

4x 8 dipswitch
1x 4 dispwitch

*/

#include "emu.h"
#include "cpu/z80/z80.h"
#include "sound/okim6295.h"

static VIDEO_START(3super8)
{

}

static VIDEO_UPDATE(3super8)
{
	return 0;
}

static ADDRESS_MAP_START( map, ADDRESS_SPACE_PROGRAM, 8 )
	AM_RANGE(0x0000, 0x7fff) AM_ROM
ADDRESS_MAP_END


static INPUT_PORTS_START( 3super8 )
INPUT_PORTS_END


static const gfx_layout tiles8x8_layout =
{
	8,8,
	RGN_FRAC(1,3),
	6,
	{ RGN_FRAC(2,3)+0, RGN_FRAC(2,3)+2*8, RGN_FRAC(1,3)+0, RGN_FRAC(1,3)+2*8,RGN_FRAC(0,3)+0, RGN_FRAC(0,3)+2*8 },
	{ 0, 1,2,3,4,5,6,7 },
	{ 0*8, 1*8, 4*8, 5*8, 8*8, 9*8,  12*8, 13*8,},
	16*8
};

static GFXDECODE_START( 3super8 )
	GFXDECODE_ENTRY( "gfx", 0, tiles8x8_layout, 0, 16 )
GFXDECODE_END



static MACHINE_DRIVER_START( 3super8 )
	/* basic machine hardware */
	MDRV_CPU_ADD("maincpu", Z80,24000000/4)		 /* 6 MHz */
	MDRV_CPU_PROGRAM_MAP(map)
	//MDRV_CPU_VBLANK_INT("screen", irq0_line_hold)

	/* video hardware */
	MDRV_SCREEN_ADD("screen", RASTER)
	MDRV_SCREEN_REFRESH_RATE(60)
	MDRV_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(0))
	MDRV_SCREEN_FORMAT(BITMAP_FORMAT_INDEXED16)
	MDRV_SCREEN_SIZE(512, 256)
	MDRV_SCREEN_VISIBLE_AREA(0, 512-1, 0, 256-0-1)

	MDRV_GFXDECODE(3super8)
	MDRV_PALETTE_LENGTH(0x100)

	MDRV_VIDEO_START(3super8)
	MDRV_VIDEO_UPDATE(3super8)

	/* sound hardware */
	MDRV_SPEAKER_STANDARD_MONO("mono")

	MDRV_OKIM6295_ADD("oki", 1056000, OKIM6295_PIN7_HIGH) // clock frequency & pin 7 not verified
	MDRV_SOUND_ROUTE(ALL_OUTPUTS, "mono", 1.0)
MACHINE_DRIVER_END



// all gfx / sound roms are bad.  they're definitely meant to have different data
//  in each half, and maybe even be twice the size.
//  in all cases the first half is missing (the sample table in the samples rom for example)
//1.bin                                           1ST AND 2ND HALF IDENTICAL
//2.bin                                           1ST AND 2ND HALF IDENTICAL
//3.bin                                           1ST AND 2ND HALF IDENTICAL
//sound.bin                                       1ST AND 2ND HALF IDENTICAL


ROM_START( 3super8 )
	ROM_REGION( 0x20000, "maincpu", 0 )
	ROM_LOAD( "prgrom.bin", 0x00000, 0x20000, CRC(37c85dfe) SHA1(56bd2fb859b17dda1e675a385b6bcd6867ecceb0)  )

	ROM_REGION( 0x1000, "pic", 0 )
	ROM_LOAD( "pic16c65a-20-p", 0x0000, 0x1000, NO_DUMP )

	ROM_REGION( 0xc0000, "gfx", 0 )
	ROM_LOAD( "1.bin", 0x00000, 0x40000, BAD_DUMP CRC(d9d3e21e) SHA1(2f3f07ca427d9f56f0ff143d15d95cbf15255e33) )
	ROM_LOAD( "2.bin", 0x40000, 0x40000, BAD_DUMP CRC(fbb50ab1) SHA1(50a7ef9219c38d59117c510fe6d53fb3ba1fa456) )
	ROM_LOAD( "3.bin", 0x80000, 0x40000, BAD_DUMP CRC(545aa4e6) SHA1(3348d4b692900c9e9cd4a52b20922a84e596cd35) )

	ROM_REGION( 0x40000, "oki", 0 )
	ROM_LOAD( "sound.bin", 0x00000, 0x40000, BAD_DUMP CRC(230b31c3) SHA1(38c107325d3a4e9781912078b1317dc9ba3e1ced) )
ROM_END

GAME( 199?, 3super8,  0,    3super8, 3super8,  0, ROT0, "<unknown>", "3 Super 8 (Italy)", GAME_NOT_WORKING|GAME_NO_SOUND )
