/* Some IGS Mahjong games with a 68k

IGS based games, some of which use

Video: IGS031
I/O: IGS022
Protection: IGS025

Others use

IGS017
IGS029

*/

#include "driver.h"
#include "cpu/m68000/m68000.h"
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
	AM_RANGE(0x100000, 0x103fff) AM_RAM //to test the decryption
	AM_RANGE(0x300000, 0x303fff) AM_RAM //to test the decryption
	AM_RANGE(0x500000, 0x503fff) AM_RAM //to test the decryption
ADDRESS_MAP_END

static INPUT_PORTS_START( igs_m68 )
INPUT_PORTS_END




static MACHINE_DRIVER_START( igs_m68 )
	MDRV_CPU_ADD("maincpu", M68000, 22000000 /2)	 // 11mhz
	MDRV_CPU_PROGRAM_MAP(igs_m68_map,0)
	//MDRV_CPU_VBLANK_INT("screen", irq1_line_hold)

	//MDRV_GFXDECODE(igs_m68)


	MDRV_SCREEN_ADD("screen", RASTER)
	MDRV_SCREEN_REFRESH_RATE(60)
	MDRV_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(0))
	MDRV_SCREEN_FORMAT(BITMAP_FORMAT_INDEXED16)
	MDRV_SCREEN_SIZE(64*8, 32*8)
	MDRV_SCREEN_VISIBLE_AREA(0*8, 64*8-1, 0*8, 32*8-1)

	MDRV_PALETTE_LENGTH(0x200)

	MDRV_VIDEO_START(igs_m68)
	MDRV_VIDEO_UPDATE(igs_m68)

	MDRV_SPEAKER_STANDARD_STEREO("lspeaker", "rspeaker")

	MDRV_SOUND_ADD("oki", OKIM6295, 8000000/8)
	MDRV_SOUND_CONFIG(okim6295_interface_pin7high)
	MDRV_SOUND_ROUTE(ALL_OUTPUTS, "lspeaker", 0.47)
	MDRV_SOUND_ROUTE(ALL_OUTPUTS, "rspeaker", 0.47)
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
	ROM_REGION( 0x80000, "maincpu", 0 ) /* 68000 Code */
	ROM_LOAD16_WORD_SWAP( "p1100.u30", 0x00000, 0x80000, CRC(68102b25) SHA1(6c1e8d204be0efda0e9b6c2f49b5c6760712475f) )

	ROM_REGION( 0x80000, "oki", 0 ) /* Samples */
	ROM_LOAD( "s1102.u23", 0x00000, 0x80000, CRC(51ffe245) SHA1(849011b186096add657ab20d49d260ec23363ef3) )

	ROM_REGION( 0x200000, "gfx1", 0 )
	ROM_LOAD( "m1101.u6", 0x000000, 0x200000, CRC(fed09cd6) SHA1(0658a97983f8ba408126e79889cc58323f2d99ba) )
	ROM_LOAD( "m1103.u8", 0x000000, 0x040000, CRC(89d0b81c) SHA1(b8d294a143e5cc9466b544cb70e43a7ce3450ace) )
	ROM_LOAD( "m1104.u11",0x000000, 0x010000, CRC(794d0276) SHA1(ac903d2faa3fb315438dc8da22c5337611a8790d) )
ROM_END

/* alt hardware, no IGS025 (protection) chip */

ROM_START( lhzb2a )
	ROM_REGION( 0x80000, "maincpu", 0 ) /* 68000 Code */
	ROM_LOAD16_WORD_SWAP( "p-4096", 0x00000, 0x80000, CRC(41293f32) SHA1(df4e993f4a458729ade13981e58f32d8116c0082) )

	ROM_REGION( 0x80000, "oki", 0 ) /* Samples */
	ROM_LOAD( "s1102.u23", 0x00000, 0x80000, CRC(51ffe245) SHA1(849011b186096add657ab20d49d260ec23363ef3) )

	ROM_REGION( 0x200000, "gfx1", 0 )
	ROM_LOAD( "m1101.u6", 0x000000, 0x200000, CRC(fed09cd6) SHA1(0658a97983f8ba408126e79889cc58323f2d99ba) )
	ROM_LOAD( "m1103",    0x000000, 0x080000, CRC(4d3776b4) SHA1(fa9b311b1a6ad56e136b66d090bc62ed5003b2f2) )
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
	ROM_REGION( 0x80000, "maincpu", 0 ) /* 68000 Code */
	ROM_LOAD16_WORD_SWAP( "p1100.u28", 0x00000, 0x80000, CRC(0b8e5c9e) SHA1(16572bd1163bba4da8a76b10649d2f71e50ad369) )

	ROM_REGION( 0x80000, "oki", 0 ) /* Samples */
	ROM_LOAD( "s1102.u20", 0x00000, 0x80000, CRC(51ffe245) SHA1(849011b186096add657ab20d49d260ec23363ef3) ) // = s1102.u23 Mahjong Long Hu Zheng Ba 2

	ROM_REGION( 0x200000, "gfx1", 0 )
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
| UPC1242H          S0903.U15   BATTERY |
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
	ROM_REGION( 0x80000, "maincpu", 0 ) /* 68000 Code */
	ROM_LOAD16_WORD_SWAP( "p0900.u25", 0x00000, 0x80000,CRC(43366f51) SHA1(48dd965dceff7de15b43c2140226a8b17a792dbc) )

	ROM_REGION( 0x80000, "oki", 0 ) /* Samples */
	ROM_LOAD( "s0903.u15", 0x00000, 0x80000, CRC(ae5a441c) SHA1(923774ef73ab0f70e0db1738a4292dcbd70d2384) )

	ROM_REGION( 0x200000, "gfx1", 0 )
	ROM_LOAD( "m0901.u5",  0x000000, 0x200000, CRC(9699db24) SHA1(50fc2f173c20b48d10595f01f1e9545f1b13a61b) )
	ROM_LOAD( "text.u6",   0x000000, 0x020000, CRC(cb34cbc0) SHA1(ceedbdda085fd1acc9a575502bdf7cf998f54f05) )
	ROM_LOAD( "m0902.u4",  0x000000, 0x080000, CRC(3298b13b) SHA1(13b21ddeed368b7f4fea1408c8fc511244342faf) )
ROM_END


static DRIVER_INIT( lhzb2 )
{
	int i;
	UINT16 *src = (UINT16 *) (memory_region(machine, "maincpu"));

	int rom_size = 0x80000;

	for (i=0; i<rom_size/2; i++)
	{
		UINT16 x = src[i];

		/* bit 0 xor layer */

		if( i & 0x20/2 )
		{
			if( i & 0x02/2 )
			{
				x ^= 0x0001;
			}
		}

		if( !(i & 0x4000/2) )
		{
			if( !(i & 0x300/2) )
			{
				x ^= 0x0001;
			}
		}

		/* bit 13 xor layer */

		if( !(i & 0x1000/2) )
		{
			if( i & 0x2000/2 )
			{
				if( i & 0x8000/2 )
				{
					if( !(i & 0x100/2) )
					{
						if( i & 0x200/2 )
						{
							if( !(i & 0x40/2) )
							{
								x ^= 0x2000;
							}
						}
						else
						{
							x ^= 0x2000;
						}
					}
				}
				else
				{
					if( !(i & 0x100/2) )
					{
						x ^= 0x2000;
					}
				}
			}
			else
			{
				if( i & 0x8000/2 )
				{
					if( i & 0x200/2 )
					{
						if( !(i & 0x40/2) )
						{
							x ^= 0x2000;
						}
					}
					else
					{
						x ^= 0x2000;
					}
				}
				else
				{
					x ^= 0x2000;
				}
			}
		}

		src[i] = x;
	}


}

static DRIVER_INIT( lhzb2a )
{
	int i;
	UINT16 *src = (UINT16 *) (memory_region(machine, "maincpu"));

	int rom_size = 0x80000;

	for (i=0; i<rom_size/2; i++)
	{
		UINT16 x = src[i];

		/* bit 0 xor layer */

		if( i & 0x20/2 )
		{
			if( i & 0x02/2 )
			{
				x ^= 0x0001;
			}
		}

		if( !(i & 0x4000/2) )
		{
			if( !(i & 0x300/2) )
			{
				x ^= 0x0001;
			}
		}

		/* bit 5 xor layer */

		if( i & 0x4000/2 )
		{
			if( i & 0x8000/2 )
			{
				if( i & 0x2000/2 )
				{
					if( i & 0x200/2 )
					{
						if( !(i & 0x40/2) || (i & 0x800/2) )
						{
							x ^= 0x0020;
						}
					}
				}
			}
			else
			{
				if( !(i & 0x40/2) || (i & 0x800/2) )
				{
					x ^= 0x0020;
				}
			}
		}

		src[i] = x;
	}

}

static DRIVER_INIT( slqz2 )
{
	int i;
	UINT16 *src = (UINT16 *) (memory_region(machine, "maincpu"));

	int rom_size = 0x80000;

	for (i=0; i<rom_size/2; i++)
	{
		UINT16 x = src[i];

		/* bit 0 xor layer */

		if( i & 0x20/2 )
		{
			if( i & 0x02/2 )
			{
				x ^= 0x0001;
			}
		}

		if( !(i & 0x4000/2) )
		{
			if( !(i & 0x300/2) )
			{
				x ^= 0x0001;
			}
		}

		/* bit 14 xor layer */

		if( i & 0x1000/2 )
		{
			if( i & 0x800/2 )
			{
				x ^= 0x4000;
			}
			else
			{
				if( i & 0x200/2 )
				{
					if( !(i & 0x100/2) )
					{
						if( i & 0x40/2 )
						{
							x ^= 0x4000;
						}
					}
				}
				else
				{
					x ^= 0x4000;
				}
			}
		}
		else
		{
			if( i & 0x800/2 )
			{
				x ^= 0x4000;
			}
			else
			{
				if( !(i & 0x100/2) )
				{
					if( i & 0x40/2 )
					{
						x ^= 0x4000;
					}
				}
			}
		}

		src[i] = x;
	}
}

static DRIVER_INIT( sdmg2 )
{
	int i;
	UINT16 *src = (UINT16 *) (memory_region(machine, "maincpu"));

	int rom_size = 0x80000;

	for (i=0; i<rom_size/2; i++)
	{
		UINT16 x = src[i];

		/* bit 0 xor layer */

		if( i & 0x20/2 )
		{
			if( i & 0x02/2 )
			{
				x ^= 0x0001;
			}
		}

		if( !(i & 0x4000/2) )
		{
			if( !(i & 0x300/2) )
			{
				x ^= 0x0001;
			}
		}

		/* bit 9 xor layer */

		if( i & 0x20000/2 )
		{
			x ^= 0x0200;
		}
		else
		{
			if( !(i & 0x400/2) )
			{
				x ^= 0x0200;
			}
		}

		/* bit 12 xor layer */

		if( i & 0x20000/2 )
		{
			x ^= 0x1000;
		}

		src[i] = x;
	}
}

GAME( 1998, lhzb2,    0,        igs_m68,    igs_m68,    lhzb2,  ROT0,  "IGS", "Mahjong Long Hu Zheng Ba 2 (set 1)", GAME_NOT_WORKING )
GAME( 1998, lhzb2a,   lhzb2,    igs_m68,    igs_m68,    lhzb2a, ROT0,  "IGS", "Mahjong Long Hu Zheng Ba 2 (set 2)", GAME_NOT_WORKING )
GAME( 1998, slqz2,    0,        igs_m68,    igs_m68,    slqz2,  ROT0,  "IGS", "Mahjong Shuang Long Qiang Zhu 2", GAME_NOT_WORKING )
GAME( 1997, sdmg2,    0,        igs_m68,    igs_m68,    sdmg2,  ROT0,  "IGS", "Mahjong Super Da Man Guan 2", GAME_NOT_WORKING )

