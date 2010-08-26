/*

 Skeleton Driver

Magic the Gathereing: Armageddon by Acclaim

TOP Board
.20u    27c4001 stickered   U20
                #1537 V1.0  a 1 was hadwritten over the 0

.u7     stamped     (c) 1997
                ACCLAIM COINOP
                ARMAGEDDON
                SND3 P/N 1605
                9806 D


.u8     stamped     (c) 1997 11/25/97
                ACCLAIM COINOP
                ARMAGEDDON
                1534 SND0
                9752 D

.u13        stamped     (c) 1997 11/25/97
                ACCLAIM COINOP
                ARMAGEDDON
                1536 SND2
                9752 D

.u14        stamped     (c) 1997 11/25/97
                ACCLAIM COINOP
                ARMAGEDDON
                1535 SND1
                9752 D

Analog devices  ADSP 2181
Xilinx      XC5202
dt71256 x2
Analog Devices  AD1866R

Bottom board
.u32    27c801      stickered   4
.u33    27c801      stickered   3
.u34    27c801      stickered   2
.u35    27c801      stickered   1
.u58    AT17C128    stickered   CC3E
.u66    GAL16V8D

Xilinx  XC4005E
Xilinx  XC95108 stickered   ACCLAIM COIN-OP
                CSC2_10.JED
                B710/0B84
                U40 p/N 1611

3dFX    500-0003-03     x2
    BF1684.1
TI  TVP3409
    V53C16258HK40       x24
    V53C511816500K60    x4
2 big chips with heat sinks on them, one by each 3dFX part
2 big chips with heat sinks on them, by the EPROMS
14.31818 Oscillator by the TI part
50.0000 Oscillator by EPROMS
33.0000 Oscillator by the V53C511816500K60
KM622560LG-7 by Battery

Bottom daughter board
All read as 29F800B
.u9     stamped     (c) 1997
                ACCLAIM COINOP
                ARMAGEDDON S0
                1514 11/25/97
                9803 D


.u10        stamped     (c) 1997
                ACCLAIM COINOP
                ARMAGEDDON S1
                1515 11/25/97
                9803 D

.u11        stamped     (c) 1997
                ACCLAIM COINOP
                ARMAGEDDON S3
                1517 11/25/97
                9803 D

.u12        stamped     (c) 1997
                ACCLAIM COINOP
                ARMAGEDDON S2
                1516 11/25/97
                9803 D

.u20        stamped     (c) 1997
                ACCLAIM COINOP
                ARMAGEDDON K0
                1543 11/25/97
                9752 D

Xilinx  XC4010E
Zoran   ZR36120PQC
Zoran   ZR36016PQC
Xilinx  XC3120A
    DT72811
    DT71256 x2
    DT72271
29.500000 osciallator by ZR36120PQC
Medium size chip with heat sink on it


*/

#include "emu.h"
#include "cpu/mips/mips3.h"


static VIDEO_START(magictg)
{

}

static VIDEO_UPDATE(magictg)
{
	return 0;
}

/* ?? */
static const mips3_config r4600_config =
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
	MDRV_CPU_ADD("maincpu", R4600BE, 10000000)  // ?? what cpu?
	MDRV_CPU_CONFIG(r4600_config)
	MDRV_CPU_PROGRAM_MAP(magictg_map)

	MDRV_SCREEN_ADD("screen", RASTER)
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



ROM_START( magictg )
	ROM_REGION32_BE( 0x400000, "user1", 0 ) /* MIPs code? */
	ROM_LOAD16_BYTE( "magic.u34", 0x000000, 0x100000, CRC(2e8971e2) SHA1(9bdf433a7c7257389ebdf131317ef26a7d4e1ba2) )
	ROM_LOAD16_BYTE( "magic.u35", 0x000001, 0x100000, CRC(e2202143) SHA1(f07b7da81508cd4594f66e34dabd904a21eb03f0) )
	ROM_LOAD16_BYTE( "magic.u32", 0x200000, 0x100000, CRC(f1d530e3) SHA1(fcc392804cd6b98917a869cc5d3826278b7ba90b) )
	ROM_LOAD16_BYTE( "magic.u33", 0x200001, 0x100000, CRC(b2330cfc) SHA1(559c35426588b349ef31bf8b296b950912f6fcc7) )

	ROM_REGION32_BE( 0x80000, "user2", 0 ) /* ADSP 2181 code? */
	ROM_LOAD( "magic.20u", 0x00000, 0x80000, CRC(50968301) SHA1(e9bdd0c942f0c66e18aa8de5a04edb51cdf1fee8) )

	ROM_REGION32_BE( 0x1000000, "user3", 0 ) /* ADSP samples */
	ROM_LOAD( "magic.snd0.u8", 0x000000, 0x400000, CRC(3cb81717) SHA1(9d35796381ca57e9782e0338c456e63c31d11266) )
	ROM_LOAD( "magic.snd1.u14",0x400000, 0x400000, CRC(b4ef9977) SHA1(dedc79e5d506bb0d1649a41b9912dcc999e1da72) )
	ROM_LOAD( "magic.snd2.u13",0x800000, 0x400000, CRC(3728f16e) SHA1(6b7da30b100d053e95aa96edf74a0474f1493dfb) )
	ROM_LOAD( "magic.snd3.u7", 0xc00000, 0x400000, CRC(11a1cb63) SHA1(a1048d3cd580747c20eb0b4e816e7e4e0f5c8c2b) )

	ROM_REGION( 0x2000000, "user4", 0 ) /* Jpeg GFX data */
	/* each of these contains 2x 0x400000 blocks */
	ROM_LOAD( "magic_s0.u9",  0x0000000, 0x800000, CRC(a01b5b99) SHA1(e77f2e9b08a97d6118e1e307b38ea79d0177e9b8) )
	ROM_LOAD( "magic_s1.u10", 0x0800000, 0x800000, CRC(d5a1a557) SHA1(2511ee8d08da765a2fa2d42fb504793f9e8b615c) )
	ROM_LOAD( "magic_s2.u12", 0x1000000, 0x800000, CRC(06ed6770) SHA1(884a3e4c97a50fa926546eb6def2c11c5732ba88) )
	ROM_LOAD( "magic_s3.u11", 0x1800000, 0x800000, CRC(71d4c252) SHA1(aeab2542b9d5fb63f4d60b808010a657a895c1d7) )

	ROM_REGION( 0x400000, "user5", 0 ) /* 'key' data */
	ROM_LOAD( "magic.k0.u20", 0x000000, 0x400000, CRC(63ab0e9e) SHA1(c4f0b009860ee499496ed7fc1f14ef1e221c1085) )
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

	ROM_REGION32_BE( 0x1000000, "user3", 0 ) /* ADSP samples */
	ROM_LOAD( "magic.snd0.u8", 0x000000, 0x400000, CRC(3cb81717) SHA1(9d35796381ca57e9782e0338c456e63c31d11266) )
	ROM_LOAD( "magic.snd1.u14",0x400000, 0x400000, CRC(b4ef9977) SHA1(dedc79e5d506bb0d1649a41b9912dcc999e1da72) )
	ROM_LOAD( "magic.snd2.u13",0x800000, 0x400000, CRC(3728f16e) SHA1(6b7da30b100d053e95aa96edf74a0474f1493dfb) )
	ROM_LOAD( "magic.snd3.u7", 0xc00000, 0x400000, CRC(11a1cb63) SHA1(a1048d3cd580747c20eb0b4e816e7e4e0f5c8c2b) )

	ROM_REGION( 0x2000000, "user4", 0 ) /* Jpeg GFX data */
	/* each of these contains 2x 0x400000 blocks */
	ROM_LOAD( "magic_s0.u9",  0x0000000, 0x800000, CRC(a01b5b99) SHA1(e77f2e9b08a97d6118e1e307b38ea79d0177e9b8) )
	ROM_LOAD( "magic_s1.u10", 0x0800000, 0x800000, CRC(d5a1a557) SHA1(2511ee8d08da765a2fa2d42fb504793f9e8b615c) )
	ROM_LOAD( "magic_s2.u12", 0x1000000, 0x800000, CRC(06ed6770) SHA1(884a3e4c97a50fa926546eb6def2c11c5732ba88) )
	ROM_LOAD( "magic_s3.u11", 0x1800000, 0x800000, CRC(71d4c252) SHA1(aeab2542b9d5fb63f4d60b808010a657a895c1d7) )

	ROM_REGION( 0x400000, "user5", 0 ) /* 'key' data */
	ROM_LOAD( "magic.k0.u20", 0x000000, 0x400000, CRC(63ab0e9e) SHA1(c4f0b009860ee499496ed7fc1f14ef1e221c1085) )
	#endif
ROM_END

GAME( 1997, magictg,  0,        magictg, magictg, 0, ROT0, "Acclaim", "Magic the Gathering: Armageddon (set 1)", GAME_NOT_WORKING|GAME_NO_SOUND )
GAME( 1997, magictga, magictg,  magictg, magictg, 0, ROT0, "Acclaim", "Magic the Gathering: Armageddon (set 2)", GAME_NOT_WORKING|GAME_NO_SOUND )

