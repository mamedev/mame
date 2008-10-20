/*

 Skeleton Driver

Magic the Gathereing: Armageddon by Acclaim

TOP Board
.20u	27c4001	stickered	U20
				#1537 V1.0	a 1 was hadwritten over the 0

.u7		stamped		(c) 1997
				ACCLAIM COINOP
				ARMAGEDDON
				SND3 P/N 1605
				9806 D


.u8		stamped		(c) 1997 11/25/97
				ACCLAIM COINOP
				ARMAGEDDON
				1534 SND0
				9752 D

.u13		stamped		(c) 1997 11/25/97
				ACCLAIM COINOP
				ARMAGEDDON
				1536 SND2
				9752 D

.u14		stamped		(c) 1997 11/25/97
				ACCLAIM COINOP
				ARMAGEDDON
				1535 SND1
				9752 D

Analog devices 	ADSP 2181
Xilinx 		XC5202
dt71256 x2
Analog Devices	AD1866R

Bottom board
.u32	27c801		stickered	4
.u33	27c801		stickered	3
.u34	27c801		stickered	2
.u35	27c801		stickered	1
.u58	AT17C128	stickered	CC3E
.u66	GAL16V8D

Xilinx	XC4005E
Xilinx	XC95108	stickered	ACCLAIM COIN-OP
				CSC2_10.JED
				B710/0B84
				U40 p/N 1611

3dFX	500-0003-03		x2
	BF1684.1
TI	TVP3409
	V53C16258HK40		x24
	V53C511816500K60	x4
2 big chips with heat sinks on them, one by each 3dFX part
2 big chips with heat sinks on them, by the EPROMS
14.31818 Oscillator by the TI part
50.0000 Oscillator by EPROMS
33.0000 Oscillator by the V53C511816500K60
KM622560LG-7 by Battery

Bottom daughter board
All read as 29F800B
.u9		stamped		(c) 1997
				ACCLAIM COINOP
				ARMAGEDDON S0
				1514 11/25/97
				9803 D


.u10		stamped		(c) 1997
				ACCLAIM COINOP
				ARMAGEDDON S1
				1515 11/25/97
				9803 D

.u11		stamped		(c) 1997
				ACCLAIM COINOP
				ARMAGEDDON S3
				1517 11/25/97
				9803 D

.u12		stamped		(c) 1997
				ACCLAIM COINOP
				ARMAGEDDON S2
				1516 11/25/97
				9803 D

.u20		stamped		(c) 1997
				ACCLAIM COINOP
				ARMAGEDDON K0
				1543 11/25/97
				9752 D

Xilinx 	XC4010E
Zoran	ZR36120PQC
Zoran	ZR36016PQC
Xilinx	XC3120A
	DT72811
	DT71256	x2
	DT72271
29.500000 osciallator by ZR36120PQC
Medium size chip with heat sink on it


*/

#include "driver.h"
#include "cpu/mips/mips3.h"


VIDEO_START(magictg)
{

}

VIDEO_UPDATE(magictg)
{
	return 0;
}

/* ?? */
static const mips3_config config =
{
	16384,				/* code cache size */
	16384				/* data cache size */
};


static ADDRESS_MAP_START( magictg_map, ADDRESS_SPACE_PROGRAM, 32 )
	AM_RANGE(0x00000000, 0x00ffffff) AM_RAM
	AM_RANGE(0x1fc00000, 0x1fc7ffff) AM_WRITENOP AM_ROM AM_REGION("user1", 0)
ADDRESS_MAP_END



static MACHINE_DRIVER_START( magictg )
	/* basic machine hardware */
	MDRV_CPU_ADD("main", R4600BE, 10000000)  // ?? what cpu?
	MDRV_CPU_CONFIG(config)
	MDRV_CPU_PROGRAM_MAP(magictg_map, 0)

	MDRV_SCREEN_ADD("main", RASTER)
	MDRV_SCREEN_REFRESH_RATE(60)
	MDRV_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(0))
	MDRV_SCREEN_FORMAT(BITMAP_FORMAT_RGB32)
	MDRV_SCREEN_SIZE(1024, 1024)
	MDRV_SCREEN_VISIBLE_AREA(0, 511, 16, 447)

	MDRV_PALETTE_LENGTH(0x1000)

	MDRV_VIDEO_START(magictg)
	MDRV_VIDEO_UPDATE(magictg)
MACHINE_DRIVER_END

static INPUT_PORTS_START( magictg )
	PORT_START("IPT_TEST")
INPUT_PORTS_END

/* the SMT rom are almost certainly dumped too small */

ROM_START( magictg )
	ROM_REGION32_BE( 0x400000, "user1", 0 ) /* MIPs code? */
	ROM_LOAD16_BYTE( "magic.u34", 0x000000, 0x100000, CRC(2e8971e2) SHA1(9bdf433a7c7257389ebdf131317ef26a7d4e1ba2) )
	ROM_LOAD16_BYTE( "magic.u35", 0x000001, 0x100000, CRC(e2202143) SHA1(f07b7da81508cd4594f66e34dabd904a21eb03f0) )
	ROM_LOAD16_BYTE( "magic.u32", 0x200000, 0x100000, CRC(f1d530e3) SHA1(fcc392804cd6b98917a869cc5d3826278b7ba90b) )
	ROM_LOAD16_BYTE( "magic.u33", 0x200001, 0x100000, CRC(b2330cfc) SHA1(559c35426588b349ef31bf8b296b950912f6fcc7) )

	ROM_REGION32_BE( 0x80000, "user2", 0 ) /* ADSP 2181 code? */
	ROM_LOAD( "magic.20u", 0x00000, 0x80000, CRC(50968301) SHA1(e9bdd0c942f0c66e18aa8de5a04edb51cdf1fee8) )

	ROM_REGION32_BE( 0x400000, "user3", 0 ) /* ADSP samples */
	ROM_LOAD( "magic.snd0.u8", 0x000000, 0x100000, BAD_DUMP CRC(ef3384ed) SHA1(0b2fa03e3da691590c705ff9d4e26ffbe374cd0f) )
	ROM_LOAD( "magic.snd1.u14",0x100000, 0x100000, BAD_DUMP CRC(d591dc93) SHA1(6a7627659de8b9e4cde7c314504f0ee087ecab4d) )
	ROM_LOAD( "magic.snd2.u13",0x200000, 0x100000, BAD_DUMP CRC(d81cb2e1) SHA1(f62f85ce094cfe48373d0929bfdb76522315da81) )
	ROM_LOAD( "magic.snd3.u7", 0x300000, 0x100000, BAD_DUMP CRC(6c8e0475) SHA1(9cafbc85d20390e9cd5d55e3c48a049e40a833ca) ) // basically empty

	ROM_REGION( 0x400000, "user4", 0 ) /* ? */
	ROM_LOAD( "magic.s0.u9",  0x000000, 0x100000, BAD_DUMP CRC(b1d9e54e) SHA1(c4964fcab4aa3d37cbfaeac0c35be31597fdd0aa) )
	ROM_LOAD( "magic.s1.u10", 0x100000, 0x100000, BAD_DUMP CRC(ccce2af7) SHA1(23d6e1bcf1ae9787f20b9bb14c2553b9a6d6b12c) )
	ROM_LOAD( "magic.s2.u12", 0x200000, 0x100000, BAD_DUMP CRC(eb8d1607) SHA1(353410b20d121b0bbecef4735a692f0c1d09ce9b) )
	ROM_LOAD( "magic.s3.u11", 0x300000, 0x100000, BAD_DUMP CRC(58c33f8e) SHA1(af09105b17b95882f7ff90ec64081e8b1d429e0b) )

	ROM_REGION( 0x100000, "user5", 0 ) /* ? */
	ROM_LOAD( "magic.k0.u20", 0x000000, 0x100000, BAD_DUMP CRC(aeb617fa) SHA1(4853c1c3631b798312d9146a90158490f2252119) )
ROM_END

ROM_START( magictga )
	ROM_REGION32_BE( 0x400000, "user1", 0 ) /* MIPs code? */
	ROM_LOAD16_BYTE( "magic.u63", 0x000000, 0x100000, CRC(a10d45f1) SHA1(0ede10f19cf70baf7b43e3f672532b4be1a179f8) )
	ROM_LOAD16_BYTE( "magic.u64", 0x000001, 0x100000, CRC(8fdb6060) SHA1(b638244cad86dc60435a4a9150a5b639f5d61a3f) )
	ROM_LOAD16_BYTE( "magic.u61", 0x200000, 0x100000, CRC(968891d6) SHA1(67ab87039864bb148d20795333ffa7a23e3b84f2) )
	ROM_LOAD16_BYTE( "magic.u62", 0x200001, 0x100000, CRC(690946eb) SHA1(6c9b02367704309f4fde5cbd9d195a45c32c3861) )

	// this set was incomplete, none of these roms were dumped for it, are they the same?
	#if 0
	ROM_REGION32_BE( 0x80000, "user2", 0 ) /* ADSP 2181 code? */
	ROM_LOAD( "magic.20u", 0x00000, 0x80000, CRC(50968301) SHA1(e9bdd0c942f0c66e18aa8de5a04edb51cdf1fee8) )

	ROM_REGION32_BE( 0x400000, "user3", 0 ) /* ADSP samples */
	ROM_LOAD( "magic.snd0.u8", 0x000000, 0x100000, BAD_DUMP CRC(ef3384ed) SHA1(0b2fa03e3da691590c705ff9d4e26ffbe374cd0f) )
	ROM_LOAD( "magic.snd1.u14",0x100000, 0x100000, BAD_DUMP CRC(d591dc93) SHA1(6a7627659de8b9e4cde7c314504f0ee087ecab4d) )
	ROM_LOAD( "magic.snd2.u13",0x200000, 0x100000, BAD_DUMP CRC(d81cb2e1) SHA1(f62f85ce094cfe48373d0929bfdb76522315da81) )
	ROM_LOAD( "magic.snd3.u7", 0x300000, 0x100000, BAD_DUMP CRC(6c8e0475) SHA1(9cafbc85d20390e9cd5d55e3c48a049e40a833ca) ) // basically empty

	ROM_REGION( 0x400000, "user4", 0 ) /* ? */
	ROM_LOAD( "magic.s0.u9",  0x000000, 0x100000, BAD_DUMP CRC(b1d9e54e) SHA1(c4964fcab4aa3d37cbfaeac0c35be31597fdd0aa) )
	ROM_LOAD( "magic.s1.u10", 0x100000, 0x100000, BAD_DUMP CRC(ccce2af7) SHA1(23d6e1bcf1ae9787f20b9bb14c2553b9a6d6b12c) )
	ROM_LOAD( "magic.s2.u12", 0x200000, 0x100000, BAD_DUMP CRC(eb8d1607) SHA1(353410b20d121b0bbecef4735a692f0c1d09ce9b) )
	ROM_LOAD( "magic.s3.u11", 0x300000, 0x100000, BAD_DUMP CRC(58c33f8e) SHA1(af09105b17b95882f7ff90ec64081e8b1d429e0b) )

	ROM_REGION( 0x100000, "user5", 0 ) /* ? */
	ROM_LOAD( "magic.k0.u20", 0x000000, 0x100000, BAD_DUMP CRC(aeb617fa) SHA1(4853c1c3631b798312d9146a90158490f2252119) )
	#endif
ROM_END

GAME( 199?, magictg,  0,        magictg, magictg, 0, ROT0, "Acclaim", "Magic the Gathering: Armageddon (set 1)", GAME_NOT_WORKING|GAME_NO_SOUND )
GAME( 199?, magictga, magictg,  magictg, magictg, 0, ROT0, "Acclaim", "Magic the Gathering: Armageddon (set 2)", GAME_NOT_WORKING|GAME_NO_SOUND )

