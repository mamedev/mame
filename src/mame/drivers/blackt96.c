/*

    Black Touch '96


Black Touch 96
D.G.R.M. of Korea, 1996

This game is a beat'em-up like Double Dragon

PCB Layout
----------

D.G.R.M. NO 1947
|---------------------------------------------|
| M6295    1     8MHz                         |
| M6295    2              2018  2018          |
|         16C57           2018  2018          |
|HA13001                  2018  2018          |
|                         2018  2018          |
|                       PAL     PAL           |
|    6116                 5      6            |
|J   6116                 7      8            |
|A                                            |
|M                                            |
|M                                            |
|A                                            |
|                               9  10         |
|    DSW1           24MHz               PAL   |
|    DSW2                                     |
|   PAL PAL           ACTEL     6116    11    |
|   62256    62256    A1020B            12    |
|   3        4        PL84C     6264    13    |
|                               6264    14    |
|18MHz 68000                    6264          |
|---------------------------------------------|
Notes:
      68000 clock 9.000MHz [18/2]
      M6295 clocks 1.000MHz [8/8] pin 7 high


*/

#include "driver.h"
#include "sound/okim6295.h"

VIDEO_START( blackt96 )
{

}

VIDEO_UPDATE( blackt96 )
{
	return 0;
}

static READ16_HANDLER( blackt96_80000_r )
{
	return 0x0000;
}

static READ16_HANDLER( blackt96_c0000_r )
{
	return 0x0000;
}

static READ16_HANDLER( blackt96_e0000_r )
{
	return 0x0000;
}

static READ16_HANDLER( blackt96_e8000_r )
{
	return 0x0000;
}

static READ16_HANDLER( blackt96_f0000_r )
{
	return 0x0000;
}

static READ16_HANDLER( blackt96_f0008_r )
{
	return 0x0000;
}

static WRITE16_HANDLER( blackt96_c0000_w )
{
	printf("blackt96_c0000_w %04x %04x\n",data,mem_mask);
}

static ADDRESS_MAP_START( blackt96_map, ADDRESS_SPACE_PROGRAM, 16 )
	AM_RANGE(0x000000, 0x07ffff) AM_ROM
	AM_RANGE(0x080000, 0x080001) AM_READ(blackt96_80000_r) AM_WRITE(SMH_NOP)
	AM_RANGE(0x0c0000, 0x0c0001) AM_READ(blackt96_c0000_r) AM_WRITE(blackt96_c0000_w)
	AM_RANGE(0x0e0000, 0x0e0001) AM_READ(blackt96_e0000_r)
	AM_RANGE(0x0e8000, 0x0e8001) AM_READ(blackt96_e8000_r)
	AM_RANGE(0x0f0000, 0x0f0001) AM_READ(blackt96_f0000_r)
	AM_RANGE(0x0f0008, 0x0f0009) AM_READ(blackt96_f0008_r)

	AM_RANGE(0x100000, 0x100fff) AM_RAM
	AM_RANGE(0x200000, 0x207fff) AM_RAM
	AM_RANGE(0x400000, 0x400fff) AM_RAM
	AM_RANGE(0xc00000, 0xc03fff) AM_RAM // main ram

ADDRESS_MAP_END



static INPUT_PORTS_START( blackt96 )
	PORT_START
	PORT_BIT( 0xffff, IP_ACTIVE_LOW, IPT_UNKNOWN )
INPUT_PORTS_END



static const gfx_layout blackt96_layout =
{
	16,16,
	RGN_FRAC(1,1),
	8,
	{ 0,1,2,3,4,5,6,7 },
	{  1024+32, 1024+40, 1024+48, 1024+56, 1024+0, 1024+8, 1024+16, 1024+24,
		32,40,48,56,0,8,16,24 },
	{ 0*64, 1*64, 2*64, 3*64, 4*64, 5*64, 6*64, 7*64, 8*64,9*64,10*64,11*64,12*64,13*64,14*64,15*64 },
	32*64
};


static GFXDECODE_START( blackt96 )
	GFXDECODE_ENTRY( REGION_GFX1, 0, blackt96_layout,   0x0, 2  )
	GFXDECODE_ENTRY( REGION_GFX2, 0, blackt96_layout,   0x0, 2  )
	GFXDECODE_ENTRY( REGION_GFX3, 0, blackt96_layout,   0x0, 2  )
GFXDECODE_END


static MACHINE_DRIVER_START( blackt96 )
	MDRV_CPU_ADD_TAG("main", M68000, 18000000 /2)
	MDRV_CPU_PROGRAM_MAP(blackt96_map,0)
	MDRV_CPU_VBLANK_INT("main", irq1_line_hold)

	MDRV_GFXDECODE(blackt96)

	MDRV_SCREEN_ADD("main", RASTER)
	MDRV_SCREEN_REFRESH_RATE(60)
	MDRV_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(0))
	MDRV_SCREEN_FORMAT(BITMAP_FORMAT_INDEXED16)
	MDRV_SCREEN_SIZE(64*8, 32*8)
	MDRV_SCREEN_VISIBLE_AREA(8*8, 48*8-1, 2*8, 30*8-1)

	MDRV_PALETTE_LENGTH(0x200)

	MDRV_VIDEO_START(blackt96)
	MDRV_VIDEO_UPDATE(blackt96)

	MDRV_SPEAKER_STANDARD_STEREO("left", "right")

	MDRV_SOUND_ADD(OKIM6295, 8000000/8)
	MDRV_SOUND_CONFIG(okim6295_interface_region_1_pin7high)
	MDRV_SOUND_ROUTE(ALL_OUTPUTS, "left", 0.47)
	MDRV_SOUND_ROUTE(ALL_OUTPUTS, "right", 0.47)

	MDRV_SOUND_ADD(OKIM6295, 8000000/8)
	MDRV_SOUND_CONFIG(okim6295_interface_region_1_pin7high)
	MDRV_SOUND_ROUTE(ALL_OUTPUTS, "left", 0.47)
	MDRV_SOUND_ROUTE(ALL_OUTPUTS, "right", 0.47)
MACHINE_DRIVER_END


ROM_START( blackt96 )
	ROM_REGION( 0x80000, REGION_CPU1, 0 ) /* 68000 Code */
	ROM_LOAD16_BYTE( "3", 0x00001, 0x40000, CRC(fc2c1d79) SHA1(742478237819af16d3fd66039283202b3c07eedd) )
	ROM_LOAD16_BYTE( "4", 0x00000, 0x40000, CRC(caff5b4a) SHA1(9a388cbb07211fa66f27082a8a5b847168c86a4f) )

	ROM_REGION( 0x080000, REGION_SOUND1, 0 ) /* Samples */
	ROM_LOAD( "1", 0x00000, 0x80000, CRC(6a934174) SHA1(087f5fa226dc68ee217f99c64d16cdf14372d44c) )

	ROM_REGION( 0x040000, REGION_SOUND2, 0 ) /* Samples */
	ROM_LOAD( "2", 0x00000, 0x40000, CRC(94009cd4) SHA1(aa36298e280c20bf86d70f3eb3fb33aca4df07e3) )

	ROM_REGION( 0x180000, REGION_GFX1, 0 ) // tiles, 16x16x8
	ROM_LOAD16_BYTE( "5", 0x00000, 0x40000, CRC(6e52c331) SHA1(31ef1d352d4ee5f7b3ef336b1f052c3a1468f22e) )
	ROM_LOAD16_BYTE( "6", 0x00001, 0x40000, CRC(69637a5a) SHA1(a5731478856d8bb91d34b747838b2b47772864ef) )
	ROM_LOAD16_BYTE( "7", 0x80000, 0x80000, CRC(6b04e8a8) SHA1(309ba1efd60600a30e1ae8f6e8b92939c23cd9c6) )
	ROM_LOAD16_BYTE( "8", 0x80001, 0x80000, CRC(16c656be) SHA1(06c40c16080a97b01a638776d28f36594ce4fb3b) )

	ROM_REGION( 0x100000, REGION_GFX2, 0 ) // not tiles?
	ROM_LOAD( "11", 0x00000, 0x40000, CRC(9eb773a3) SHA1(9c91ee938438a61f5fa650ced6249e34aa5321bd) )
	ROM_LOAD( "12", 0x40000, 0x40000, CRC(8894e608) SHA1(389974a0b208b7cbf7d5f83641ddc058ad5ebe87) )
	ROM_LOAD( "13", 0x80000, 0x40000, CRC(0acceb9d) SHA1(e8a85c7eab45d84613ac37a9b7ffbc45b44eb2e5) )
	ROM_LOAD( "14", 0xc0000, 0x40000, CRC(b5e3de25) SHA1(33ac5602ab6bcadc8b0d1aa805a3bdce0b67c215) )

	ROM_REGION( 0x100000, REGION_GFX3, 0 ) // not tiles?
	ROM_LOAD( "9",  0x00000, 0x10000, CRC(81a4cf4c) SHA1(94b2bbcbc8327d9babbc3b222bd88954c7e7b80e) )
	ROM_LOAD( "10", 0x10000, 0x10000, CRC(b78232a2) SHA1(36a4f01011faf64e46b73f0082ab04843ac8b0e2) )
ROM_END

GAME( 1996, blackt96,    0,        blackt96,    blackt96,    0, ROT0,  "D.G.R.M.", "Black Touch '96", GAME_NOT_WORKING )
