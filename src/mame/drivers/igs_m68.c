/* Some IGS Mahjong games with a 68k

IGS based games, some of which use

Video: IGS031
I/O: IGS022
Protection: IGS025

Others use

IGS017
IGS029

All of these have encrypted program roms, which will need decrypting

*/

#include "driver.h"
#include "sound/okim6295.h"

static VIDEO_START(igs_m68)
{
}

static VIDEO_UPDATE(igs_m68)
{
	return 0;
}


static ADDRESS_MAP_START( igs_m68_map, ADDRESS_SPACE_PROGRAM, 16 )
	AM_RANGE(0x000000, 0x07ffff) AM_ROM
ADDRESS_MAP_END

static INPUT_PORTS_START( igs_m68 )
INPUT_PORTS_END




static MACHINE_DRIVER_START( igs_m68 )
	MDRV_CPU_ADD_TAG("main", M68000, 22000000 /2)	 // 11mhz
	MDRV_CPU_PROGRAM_MAP(igs_m68_map,0)
	//MDRV_CPU_VBLANK_INT("main", irq1_line_hold)

	//MDRV_GFXDECODE(igs_m68)


	MDRV_SCREEN_ADD("main", RASTER)
	MDRV_SCREEN_REFRESH_RATE(60)
	MDRV_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(0))
	MDRV_SCREEN_FORMAT(BITMAP_FORMAT_INDEXED16)
	MDRV_SCREEN_SIZE(64*8, 32*8)
	MDRV_SCREEN_VISIBLE_AREA(0*8, 64*8-1, 0*8, 32*8-1)

	MDRV_PALETTE_LENGTH(0x200)

	MDRV_VIDEO_START(igs_m68)
	MDRV_VIDEO_UPDATE(igs_m68)

	MDRV_SPEAKER_STANDARD_STEREO("left", "right")

	MDRV_SOUND_ADD(OKIM6295, 8000000/8)
	MDRV_SOUND_CONFIG(okim6295_interface_region_1_pin7high)
	MDRV_SOUND_ROUTE(ALL_OUTPUTS, "left", 0.47)
	MDRV_SOUND_ROUTE(ALL_OUTPUTS, "right", 0.47)
MACHINE_DRIVER_END

/*

Mahjong Long Hu Zheng Ba 2
IGS, 1998


PCB Layout
----------

IGS PCB NO-0206
|---------------------------------------|
|    6264             |-------|         |
|    6264      |----| |       |         |
|              |IGS | |IGS025 |         |
|              |022 | |       |  PAL    |
|              |----| |-------|         |
|                       PAL             |
|    M1104.U11          PAL    68000    |
|1                                      |
|8                                      |
|W                                      |
|A   M1101.U6  8MHz          P1100.U30  |
|Y                                      |
|                                  6264 |
|                                       |
|              |-------|                |
|              |       |                |
|              |IGS031 |           61256|
|1             |       |                |
|0   M1103.U8  |-------|                |
|W      22MHz                           |
|A             DSW1   DSW2              |
|Y            K668     BATTERY          |
|TDA1020 VOL          S1102.U23  SPDT_SW|
|---------------------------------------|
Notes:
      Uses common 10-way/18-way Mahjong pinout
      68000 clock 11.000MHz [22/2]
      K668  == Oki M6295 (QFP44). Clock 1MHz [8/8]. pin7 = High
      VSync - 60Hz
      HSync - 15.3kHz

*/

ROM_START( lhzb2 )
	ROM_REGION( 0x80000, REGION_CPU1, 0 ) /* 68000 Code */
	ROM_LOAD( "p1100.u30", 0x00000, 0x80000, CRC(68102b25) SHA1(6c1e8d204be0efda0e9b6c2f49b5c6760712475f) )

	ROM_REGION( 0x80000, REGION_SOUND1, 0 ) /* Samples */
	ROM_LOAD( "s1102.u23", 0x00000, 0x80000, CRC(51ffe245) SHA1(849011b186096add657ab20d49d260ec23363ef3) )

	ROM_REGION( 0x200000, REGION_GFX1, 0 )
	ROM_LOAD( "m1101.u6", 0x000000, 0x200000, CRC(fed09cd6) SHA1(0658a97983f8ba408126e79889cc58323f2d99ba) )
	ROM_LOAD( "m1103.u8", 0x000000, 0x040000, CRC(89d0b81c) SHA1(b8d294a143e5cc9466b544cb70e43a7ce3450ace) )
	ROM_LOAD( "m1104.u11",0x000000, 0x010000, CRC(794d0276) SHA1(ac903d2faa3fb315438dc8da22c5337611a8790d) )
ROM_END

/* alt hardware, no IGS025 (protection) chip */

ROM_START( lhzb2a )
	ROM_REGION( 0x80000, REGION_CPU1, 0 ) /* 68000 Code */
	ROM_LOAD( "p-4096", 0x00000, 0x80000, CRC(41293f32) SHA1(df4e993f4a458729ade13981e58f32d8116c0082) )

	ROM_REGION( 0x80000, REGION_SOUND1, 0 ) /* Samples */
	ROM_LOAD( "s1102.u23", 0x00000, 0x80000, CRC(51ffe245) SHA1(849011b186096add657ab20d49d260ec23363ef3) )

	ROM_REGION( 0x200000, REGION_GFX1, 0 )
	ROM_LOAD( "m1101.u6", 0x000000, 0x200000, CRC(fed09cd6) SHA1(0658a97983f8ba408126e79889cc58323f2d99ba) )
	ROM_LOAD( "m1103",    0x000000, 0x080000, CRC(4d3776b4) SHA1(fa9b311b1a6ad56e136b66d090bc62ed5003b2f2) )
ROM_END

/*

Mahjong Man Guan Cai Shen
IGS, 1998


PCB Layout
----------

IGS PCB NO-0192-1
|---------------------------------------|
|              JAMMA            UPC1242 |
|                                       |
|               S1502.U10               |
|                          K668    VOL  |
|                                       |
|                                       |
|                       22MHz           |
|1     61256                            |
|8              |-------|      TEXT.U25 |
|W     PAL      |       |               |
|A              |IGS017 |               |
|Y              |       |      M1501.U23|
|               |-------|               |
|   |-------|                           |
|   |       |                           |
|   |IGS025 |   P1500.U8                |
|   |       |              PAL    6264  |
|1  |-------|                           |
|0  |----|                 PAL    6264  |
|W  |IGS |                 PAL          |
|A  |029 |  8MHz                 SPDT_SW|
|Y  |----|                 68000        |
|T DSW1  DSW2                   BATTERY |
|---------------------------------------|
Notes:
      Uses JAMMA & common 10-way/18-way Mahjong pinout
      68000 clock 11.000MHz [22/2]
      K668  == Oki M6295 (QFP44). Clock 1MHz [8/8]. pin7 = High
      VSync - 60Hz
      HSync - 15.3kHz

*/


ROM_START( mgcs )
	ROM_REGION( 0x80000, REGION_CPU1, 0 ) /* 68000 Code */
	ROM_LOAD( "p1500.u8", 0x00000, 0x80000, CRC(a8cb5905) SHA1(37be7d926a1352869632d43943763accd4dec4b7) )

	ROM_REGION( 0x80000, REGION_SOUND1, 0 ) /* Samples */
	ROM_LOAD( "s1502.u10", 0x00000, 0x80000, CRC(a8a6ba58) SHA1(59276a8ab4a31812600816c2a43b74bd71394419) )

	ROM_REGION( 0x200000, REGION_GFX1, 0 )
	ROM_LOAD( "m1501.u23", 0x000000, 0x200000, CRC(7a876bcc) SHA1(581740f39cfac56324f483a24aece6957645378a) )
	ROM_LOAD( "text.u25",    0x000000, 0x080000, CRC(a37f9613) SHA1(812f060ca98a34540c48a180c359c3d0f1c0b5bb) )
ROM_END

/*

Mahjong Shuang Long Qiang Zhu 2
IGS, 1998


PCB Layout
----------

IGS PCB NO-0207
|---------------------------------------|
|                   K668  S1102.U20     |
|     PAL                               |
| 8MHz                     6264         |
|                                       |
|    |----|                6264         |
|    |IGS |                             |
|    |022 |  M1103.U12       PAL        |
|J   |----|                    PAL      |
|A                                      |
|M                                      |
|M      |-------|                       |
|A      |       |                       |
|       |IGS025 |   68000               |
|       |       |                       |
|       |-------|                       |
|                            P1100.U28  |
|                 PAL                   |
|  M1101.U4       |-------|             |
|                 |       |             |
|                 |IGS031 |      6264   |
|  TEXT.U6        |       |             |
|                 |-------|      62256  |
|SPDT_SW   22MHz   DSW1  DSW2  BATTERY  |
|---------------------------------------|
Notes:
      68000 clock 11.000MHz [22/2]
      K668  == Oki M6295 (QFP44). Clock 1.000MHz [8/8]. pin7 = High
      VSync - 60Hz
      HSync - 15.3kHz

*/

ROM_START( slqz2 )
	ROM_REGION( 0x80000, REGION_CPU1, 0 ) /* 68000 Code */
	ROM_LOAD( "p1100.u28", 0x00000, 0x80000, CRC(0b8e5c9e) SHA1(16572bd1163bba4da8a76b10649d2f71e50ad369) )

	ROM_REGION( 0x80000, REGION_SOUND1, 0 ) /* Samples */
	ROM_LOAD( "s1102.u20", 0x00000, 0x80000, CRC(51ffe245) SHA1(849011b186096add657ab20d49d260ec23363ef3) ) // = s1102.u23 Mahjong Long Hu Zheng Ba 2

	ROM_REGION( 0x200000, REGION_GFX1, 0 )
	ROM_LOAD( "m1101.u4",  0x000000, 0x200000, CRC(fed09cd6) SHA1(0658a97983f8ba408126e79889cc58323f2d99ba) ) // = m1101.u6 Mahjong Long Hu Zheng Ba 2
	ROM_LOAD( "text.u6",   0x000000, 0x080000, CRC(40d21adf) SHA1(18b202d6330ac89026bec2c9c8224b52540dd48d) )
	ROM_LOAD( "m1103.u12", 0x000000, 0x010000, CRC(9f3b8d65) SHA1(5ee1ad025474399c2826f21d970e76f25d0fa1fd) )
ROM_END

/*

Mahjong Super Da Man Guan 2
IGS, 1997


PCB Layout
----------

IGS PCB NO-0147-6
|---------------------------------------|
| UPC1242H          S0903.U15   BATTERY|
|          VOL               SPDT_SW    |
|                                       |
|        K668                    6264   |
|                                       |
|                                6264   |
|                   PAL                 |
|1   8255                               |
|8                            P0900.U25 |
|W                                      |
|A                                      |
|Y                                      |
|                                68000  |
|                                       |
|    M0902.U4       PAL                 |
|                                       |
|                                 PAL   |
|1   M0901.U5       |-------|           |
|0                  |       |     PAL   |
|W                  |IGS031 |           |
|A   TEXT.U6        |       |           |
|Y                  |-------|     62256 |
|T         22MHz  DSW1 DSW2             |
|---------------------------------------|
Notes:
      Uses common 10-way/18-way Mahjong pinout
      68000 clock 11.000MHz [22/2]
      K668 = M6295. clock 1.000MHz [22/22]. pin7 = High
      VSync - 60Hz
      HSync - 15.3kHz

*/


ROM_START( sdmg2 )
	ROM_REGION( 0x80000, REGION_CPU1, 0 ) /* 68000 Code */
	ROM_LOAD( "p0900.u25", 0x00000, 0x80000,CRC(43366f51) SHA1(48dd965dceff7de15b43c2140226a8b17a792dbc) )

	ROM_REGION( 0x80000, REGION_SOUND1, 0 ) /* Samples */
	ROM_LOAD( "s0903.u15", 0x00000, 0x80000, CRC(ae5a441c) SHA1(923774ef73ab0f70e0db1738a4292dcbd70d2384) )

	ROM_REGION( 0x200000, REGION_GFX1, 0 )
	ROM_LOAD( "m0901.u5",  0x000000, 0x200000, CRC(9699db24) SHA1(50fc2f173c20b48d10595f01f1e9545f1b13a61b) )
	ROM_LOAD( "text.u6",   0x000000, 0x020000, CRC(cb34cbc0) SHA1(ceedbdda085fd1acc9a575502bdf7cf998f54f05) )
	ROM_LOAD( "m0902.u4",  0x000000, 0x080000, CRC(3298b13b) SHA1(13b21ddeed368b7f4fea1408c8fc511244342faf) )
ROM_END


GAME( 1998, lhzb2,    0,        igs_m68,    igs_m68,    0, ROT0,  "IGS", "Mahjong Long Hu Zheng Ba 2 (set 1)", GAME_NOT_WORKING )
GAME( 1998, lhzb2a,   lhzb2,    igs_m68,    igs_m68,    0, ROT0,  "IGS", "Mahjong Long Hu Zheng Ba 2 (set 2)", GAME_NOT_WORKING )
GAME( 1998, mgcs,     0,        igs_m68,    igs_m68,    0, ROT0,  "IGS", "Mahjong Man Guan Cai Shen", GAME_NOT_WORKING )
GAME( 1998, slqz2,    0,        igs_m68,    igs_m68,    0, ROT0,  "IGS", "Mahjong Shuang Long Qiang Zhu 2", GAME_NOT_WORKING )
GAME( 1997, sdmg2,    0,        igs_m68,    igs_m68,    0, ROT0,  "IGS", "Mahjong Super Da Man Guan 2", GAME_NOT_WORKING )

