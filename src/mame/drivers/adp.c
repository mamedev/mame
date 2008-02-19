/*
ADP (Merkur?) games from '90 running on similar hardware.
(68k + HD63484 + YM2149)

Skeleton driver by TS -  analog at op.pl

TODO:
(almost everything)
 - add emulation of HD63484 (like shanghai.c but 4bpp mode and much more commands)
 - add sound and i/o
 - protection in Fashion Gambler (NVRam based?)

Supported games :
- Quick Jack      ("COPYRIGHT BY ADP LUEBBECKE GERMANY 1993")
- Skat TV           ("COPYRIGHT BY ADP LUEBBECKE GERMANY 1994")
- Skat TV v. TS3  ("COPYRIGHT BY ADP LUEBBECKE GERMANY 1995")
- Fashion Gambler ("COPYRIGHT BY ADP LUEBBECKE GERMANY 1997")
- Backgammon        ("COPYRIGHT BY ADP LUEBBECKE GERMANY 1994")



Skat TV (Version TS3)
Three board stack.

CPU Board:
----------
 ____________________________________________________________
 |           ______________  ______________     ___________ |
 | 74HC245N  | t1 i       |  |KM681000ALP7|     |+        | |
 | 74HC573   |____________|  |____________|     |  3V Bat | |
 |                                              |         | |
 |           ______________  ______________     |        -| |
 |           | t1 ii      |  |KM681000ALP7|     |_________| |
 |     |||   |____________|  |____________| |||             |
 |     |||   ___________                    |||  M62X42B    |
 | X   |||   |         |                    |||             |
 |     |||   |68EC000 8|  74HC32   74HC245  |||  MAX691CPE  |
 |     |||   |         |  74AC138  74HC573  |||    74HC32   |
 |           |         |                                    |
 | 74HC573   |_________|  74HC08   74HC10  74HC32  74HC21   |
 |__________________________________________________________|

Parts:

 68EC000FN8         - Motorola 68k CPU
 KM681000ALP7       - 128K X 8 Bit Low Power CMOS Static RAM
 OKIM62X42B         - Real-time Clock ic With Built-in Crystal
 MAX691CPE          - P Reset ic With Watchdog And Battery Switchover
 X                    - 8MHz xtal
 3V Bat             - Lithium 3V power module

Video Board:
------------
 ____________________________________________________________
 |           ______________  ______________                 |
 |           | t2 i       |  |KM681000ALP7|     74HC573     |
 |           |____________|  |____________|                *|
 |                                              74HC573    *|
 |           ______________  ______________                *|
 |           | t2 ii      |  |KM681000ALP7|               P3|
 |       ||| |____________|  |____________|   |||          *|
 |       ||| ___________                      |||          *|
 |       ||| |         |                      |||          *|
 |       ||| | HD63484 |  74HC04   74HC00     |||         P6|
 |       ||| |         |  74HC74   74HC08     |||  74HC245  |
 |           |         |                                    |
 | 74HC573   |_________|  74HC166  74HC166 74HC166 74HC166  |
 |__________________________________________________________|

Parts:

 HD63484CP8         - Advanced CRT Controller
 KM681000ALP7       - 128K X 8 Bit Low Power CMOS Static RAM

Connectors:

 Two connectors to link with CPU Board
 Two connectors to link with Sound and I/O Board
 P3  - Monitor
 P6  - Lightpen

Sound  and I/O board:
---------------------
 _________________________________________________________________________________
 |                        TS271CN    74HC02                        ****  ****    |
 |*                      ________________                          P1    P2     *|
 |*         74HC574      | YM2149F      |                                       *|
 |*                  ||| |______________|   74HC393  74HC4015 |||               *|
 |P3        74HC245  |||                                      |||              P6|
 |*                  ||| ________________          X          ||| TL7705ACP     *|
 |*                  ||| |SCN68681C1N40 |                     |||               *|
 |*                  ||| |______________|   74HC32   74AC138  |||               *|
 |P7                 |||                                      |||              P8|
 |*                        TC428CPA                                             *|
 |*                                                                             *|
 |*    P11  P12    P13    P14       P15   P16   P17      P18   P19   P20  P21   *|
 |P9   **** *****  *****  ****  OO  ****  ****  *******  ****  ****  ***  *** P10|
 |_______________________________________________________________________________|

Parts:

 YM2149F         - Yamaha PSG
 SCN68681C1N40   - Dual Asynchronous Receiver/transmitter (DUART);
 TS271CN         - Programmable Low Power CMOS Single Op-amp
 TL7705ACP       - Supply Voltage Supervisor
 TC428CPA        - Dual CMOS High-speed Driver
 OO              - LEDs (red)
 X               - 3.6864MHz xtal

Connectors:

 Two connectors to link with Video Board
 P1  - Tueroeffn
 P2  - PSG In/Out
 P3  - Lautsprecher
 P6  - Service - Tast.
 P7  - Maschine (barely readable)
 P8  - Muenzeinheit
 P9  - Atzepter
 P10 - Reset Fadenfoul
 P11 - Netzteil
 P12 - Serienplan
 P13 - Serienplan 2
 P14 - Muenzeinheit 2
 P15 - I2C Bus
 P16 - Kodierg.
 P17 - TTL Ein-Aueg.
 P18 - Out
 P19 - In
 P20 - Serielle-S.
 P21 - Tuerschalter

There's also (external) JAMMA adapter - 4th board filled with resistors and diodes.

*/

#include "driver.h"
#include "sound/ay8910.h"

static ADDRESS_MAP_START( skattv_mem, ADDRESS_SPACE_PROGRAM, 16 )
	AM_RANGE(0x000000, 0x0fffff) AM_ROM
	AM_RANGE(0xfc0000, 0xffffff) AM_RAM
ADDRESS_MAP_END

static VIDEO_START(skattv)
{
}

static VIDEO_UPDATE(skattv)
{
	return 0;
}

static INPUT_PORTS_START( skattv )

INPUT_PORTS_END

static MACHINE_DRIVER_START( skattv )
	MDRV_CPU_ADD_TAG("main", M68000, 8000000)
	MDRV_CPU_PROGRAM_MAP(skattv_mem, 0)
	MDRV_SCREEN_REFRESH_RATE(60)
	MDRV_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(0))

	MDRV_VIDEO_ATTRIBUTES(VIDEO_TYPE_RASTER)
	MDRV_SCREEN_FORMAT(BITMAP_FORMAT_INDEXED16)
	MDRV_SCREEN_SIZE(640, 480)
	MDRV_SCREEN_VISIBLE_AREA(0, 640-1, 0, 480-1)
	MDRV_PALETTE_LENGTH(0x100)

	MDRV_VIDEO_START(skattv)
	MDRV_VIDEO_UPDATE(skattv)

	MDRV_SPEAKER_STANDARD_MONO("mono")
	MDRV_SOUND_ADD(AY8910, 3686400/2)
	MDRV_SOUND_ROUTE(ALL_OUTPUTS, "mono", 1.0)

MACHINE_DRIVER_END

ROM_START( quickjac )
	ROM_REGION( 0x100000, REGION_CPU1, 0 )
	ROM_LOAD16_BYTE( "quick_jack_index_a.1.u2.bin", 0x00000, 0x10000, CRC(c2fba6fe) SHA1(f79e5913f9ded1e370cc54dd55860263b9c51d61) )
	ROM_LOAD16_BYTE( "quick_jack_index_a.2.u6.bin", 0x00001, 0x10000, CRC(210cb89b) SHA1(8eac60d40b60e845f9c02fee6c447f125ba5d1ab) )

	ROM_REGION( 0x40000, REGION_GFX1, 0 )
	ROM_LOAD16_BYTE( "quick_jack_video_inde_a.1.u2.bin", 0x00000, 0x20000, CRC(73c27fc6) SHA1(12429bc0009b7754e08d2b6a5e1cd8251ab66e2d) )
	ROM_LOAD16_BYTE( "quick_jack_video_inde_a.2.u6.bin", 0x00001, 0x20000, CRC(61d55be2) SHA1(bc17dc91fd1ef0f862eb0d7dbbbfa354a8403eb8) )
ROM_END

ROM_START( skattv )
	ROM_REGION( 0x100000, REGION_CPU1, 0 )
	ROM_LOAD16_BYTE( "f2_i.bin", 0x00000, 0x20000, CRC(3cb8b431) SHA1(e7930876b6cd4cba837c3da05d6948ef9167daea) )
	ROM_LOAD16_BYTE( "f2_ii.bin", 0x00001, 0x20000, CRC(0db1d2d5) SHA1(a29b0299352e0b2b713caf02aa7978f2a4b34e37) )

	ROM_REGION( 0x40000, REGION_GFX1, 0 )
	ROM_LOAD16_BYTE( "f1_i.bin", 0x00000, 0x20000, CRC(4869a889) SHA1(ad9f3fcdfd3630f9ad5b93a9d2738de9fc3514d3) )
	ROM_LOAD16_BYTE( "f1_ii.bin", 0x00001, 0x20000, CRC(17681537) SHA1(133685854b2080aaa3d0cced0287bc454d1f3bfc) )
ROM_END

ROM_START( skattva )
	ROM_REGION( 0x100000, REGION_CPU1, 0 )
	ROM_LOAD16_BYTE( "skat_tv_version_ts3.1.u2.bin", 0x00000, 0x20000, CRC(68f82fe8) SHA1(d5f9cb600531cdd748616d8c042b6a151ebe205a) )
	ROM_LOAD16_BYTE( "skat_tv_version_ts3.2.u6.bin", 0x00001, 0x20000, CRC(4f927832) SHA1(bbe013005fd00dd42d12939eab5c80ec44a54b71) )

	ROM_REGION( 0x40000, REGION_GFX1, 0 )
	ROM_LOAD16_BYTE( "skat_tv_videoprom_t2.1.u2.bin", 0x00000, 0x20000, CRC(de6f275b) SHA1(0c396fa4d1975c8ccc4967d330b368c0697d2124) )
	ROM_LOAD16_BYTE( "skat_tv_videoprom_t2.2.u5.bin", 0x00001, 0x20000, CRC(af3e60f9) SHA1(c88976ea42cf29a092fdee18377b32ffe91e9f33) )
ROM_END

ROM_START( backgamn )
	ROM_REGION( 0x100000, REGION_CPU1, 0 )
	ROM_LOAD16_BYTE( "b_f2_i.bin", 0x00000, 0x10000, CRC(9e42937c) SHA1(85d462a560b85b03ee9d341e18815b7c396118ac) )
	ROM_LOAD16_BYTE( "b_f2_ii.bin", 0x00001, 0x10000, CRC(8e0ee50c) SHA1(2a05c337db1131b873646aa4109593636ebaa356) )

	ROM_REGION( 0x40000, REGION_GFX1, 0 )
	ROM_LOAD16_BYTE( "b_f1_i.bin", 0x00000, 0x20000, NO_DUMP )
	ROM_LOAD16_BYTE( "b_f1_ii.bin", 0x00001, 0x20000, NO_DUMP )
ROM_END

ROM_START( fashiong )
	ROM_REGION( 0x100000, REGION_CPU1, 0 )
	ROM_LOAD16_BYTE( "fashion_gambler_s6_i.bin", 0x00000, 0x80000, CRC(827a164d) SHA1(dc16380226cabdefbfd893cb50cbfca9e134be40) )
	ROM_LOAD16_BYTE( "fashion_gambler_s6_ii.bin", 0x00001, 0x80000, CRC(5a2466d1) SHA1(c113a2295beed2011c70887a1f2fcdec00b055cb) )

	ROM_REGION( 0x100000, REGION_GFX1, 0 )
	ROM_LOAD16_BYTE( "fashion_gambler_video_s2_i.bin", 0x00000, 0x80000, CRC(d1ee9133) SHA1(e5fdfa303a3317f8f5fbdc03438ee97415afff4b) )
	ROM_LOAD16_BYTE( "fashion_gambler_video_s2_ii.bin", 0x00001, 0x80000, CRC(07b1e722) SHA1(594cbe9edfea6b04a4e49d1c1594f1c3afeadef5) )

	ROM_REGION( 0x4000, REGION_USER1, 0 )
	//nvram - 16 bit
	ROM_LOAD16_BYTE( "m48z08post.bin", 0x0000, 0x2000, CRC(2d317a04) SHA1(c690c0d4b2259231d642ab5a30fcf389ba987b70) )
	ROM_LOAD16_BYTE( "m48z08posz.bin", 0x0001, 0x2000, CRC(7c5a4b78) SHA1(262d0d7f5b24e356ab54eb2450bbaa90e3fb5464) )
ROM_END

GAME( 1990, backgamn,        0, skattv,    skattv,    0, ROT0,  "ADP", "Backgammon", GAME_NOT_WORKING )
GAME( 1993, quickjac,        0, skattv,    skattv,    0, ROT0,  "ADP", "Quick Jack", GAME_NOT_WORKING )
GAME( 1994, skattv,          0, skattv,    skattv,    0, ROT0,  "ADP", "Skat TV", GAME_NOT_WORKING )
GAME( 1995, skattva,    skattv, skattv,    skattv,    0, ROT0,  "ADP", "Skat TV (version TS3)", GAME_NOT_WORKING )
GAME( 1997, fashiong,        0, skattv,    skattv,    0, ROT0,  "ADP", "Fashion Gambler", GAME_NOT_WORKING )

