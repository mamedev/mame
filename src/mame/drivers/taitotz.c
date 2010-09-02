/*
   Taito Type-Zero hardware

   Skeleton driver. Requires TLCS-900 CPU core to make progress.

Landing High Japan PCB info
===========================

Taito Landing High

Top board
    silkscreened        TYPE-ZERO MOTHER PCB

    stickered       298100308
                K11X0886A
                JC101

.5  29LV400BC-90        stamped E68-05-1
.6  29LV400BC-90        stamped E68-04-1
.24 PALV 18V8-10JC      stamped E68-06


IC30    Taito TCG020AGP
IC26    IDT7024 S35J V9928P
IC10    IBM EMPPC603eBG-100
IC53    ADV7120KP30 9926 F101764.1
IC16,17         M54256V32A-10
IC15,25,31,39,45,49,44  48D4811650GF-A10-9BT
IC27,36         D4564163G5-A10n-9JF
IC 41,46,42,47      D4516161AG5-A10B-9F
IC43            QSV991-7JRI
66.6667 Oscillator near IC43

Bottom board
    silkscreened        JC101 DAUGHTER PCB

    stickered       K91J0775A
                LANDING H.JAPAN

                299100308

                M43J0741A
                LANDING H.JAPAN

.14 27c1001     stickered   E82
                    03*

.15 27c1001     stickered   E82
                    04*

.44 PALCE16V8H  stamped     E82-01

.45 PALCE22V10H stamped     E82-02

IC40    Toshiba TMP95C063F
IC55    Panasonic MN89306
EPSON 9X5C pscillator near IC55
IC56    HY57V161610D TC-10
IC22    ID7133 SA70J
25.000 oscillator near IC22
IC11    Xilinx XC9572
IC5 HY5118164C JC-60
IC10    ZOOM ZSG-2
IC20    ZOOM ZFX 2 HD 96NE2VJ
IC26    TM TECH  UA4464V T224162B-28J
IC7 Panasonic MN1020819DA E68-01
20.000 oscillator near IC7

*/

#include "emu.h"
#include "cpu/powerpc/ppc.h"


static VIDEO_START( taitotz )
{
}

static VIDEO_UPDATE( taitotz )
{
	return 0;
}

static ADDRESS_MAP_START( ppc603e_mem, ADDRESS_SPACE_PROGRAM, 64)
	AM_RANGE(0x00000000, 0x00000007) AM_RAM   // Register/RAM access port? - Written 128k+256k times on boot
	AM_RANGE(0x00000008, 0x0000000f) AM_RAM   // Register/RAM address port?
	AM_RANGE(0x40000000, 0x400fffff) AM_RAM   // Work RAM
	AM_RANGE(0xa8000000, 0xa8001fff) AM_RAM   // Common RAM (with TLCS-900)
	AM_RANGE(0xa8003ff8, 0xa8003fff) AM_RAM   // TLCS-900 related?
	AM_RANGE(0xac000000, 0xac0fffff) AM_ROM AM_REGION("user1", 0)
	AM_RANGE(0xfff00000, 0xffffffff) AM_ROM AM_REGION("user1", 0)
ADDRESS_MAP_END


static INPUT_PORTS_START( taitotz )
INPUT_PORTS_END


static const powerpc_config ppc603e_config =
{
	XTAL_66_6667MHz		/* Multiplier 1.5, Bus = 66MHz, Core = 100MHz */
};


static MACHINE_CONFIG_START( taitotz, driver_device )
	MDRV_CPU_ADD("maincpu", PPC603E, 100000000)
	MDRV_CPU_CONFIG(ppc603e_config)
	MDRV_CPU_PROGRAM_MAP(ppc603e_mem)

	/* TMP95C063F I/O CPU */
	/* MN1020819DA sound CPU */

	MDRV_SCREEN_ADD("screen", RASTER)
	MDRV_SCREEN_FORMAT(BITMAP_FORMAT_RGB32)
	MDRV_SCREEN_REFRESH_RATE(60)
	MDRV_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(0))
	MDRV_SCREEN_SIZE(512, 384)
	MDRV_SCREEN_VISIBLE_AREA(0, 511, 0, 383)

	MDRV_VIDEO_START(taitotz)
	MDRV_VIDEO_UPDATE(taitotz)
MACHINE_CONFIG_END

#define TAITOTZ_BIOS	\
	ROM_LOAD32_WORD_SWAP( "e68-05-1.ic6", 0x000000, 0x080000, CRC(6ad9b006) SHA1(f05a0ae26b6abaeda9c7944aee96c72b08fff7a5) )	\
	ROM_LOAD32_WORD_SWAP( "e68-04-1.ic5", 0x000002, 0x080000, CRC(c7c2dc6b) SHA1(bf88c818166c285130c5c73d6982f009da26e143) )


ROM_START( taitotz )
	ROM_REGION64_BE( 0x100000, "user1", 0 )
	TAITOTZ_BIOS

	ROM_REGION( 0x10000, "sound_cpu", ROMREGION_ERASE00 ) /* Internal ROM :( */
	ROM_REGION( 0x500, "plds", ROMREGION_ERASE00 )
	DISK_REGION( "ide" )
ROM_END

ROM_START( landhigh )
	ROM_REGION64_BE( 0x100000, "user1", 0 )
	TAITOTZ_BIOS

	ROM_REGION( 0x40000, "io_cpu", 0 )
	ROM_LOAD16_BYTE( "e82-03.ic14", 0x000000, 0x020000, CRC(0de65b4d) SHA1(932316f7435259b723a29843d58b2e3dca92e7b7) )
	ROM_LOAD16_BYTE( "e82-04.ic15", 0x000001, 0x020000, CRC(b3cb0f3d) SHA1(80414f50a1593c6b849d9f37e94a32168699a5c1) )

	ROM_REGION( 0x10000, "sound_cpu", 0 ) /* Internal ROM :( */
	ROM_LOAD( "e68-01.ic7", 0x000000, 0x010000, NO_DUMP )

	ROM_REGION( 0x500, "plds", 0 )
	ROM_LOAD( "e82-01.ic44", 0x000, 0x117, CRC(49eea30f) SHA1(ef97c792358f05b9214a2f58ee1e97e8208806c4) )
	ROM_LOAD( "e82-02.ic45", 0x117, 0x2dd, CRC(f581cff5) SHA1(468e0e6a3828f2dcda35c6d523154510f9c99db7) )
	ROM_LOAD( "e68-06.ic24", 0x3f4, 0x100, NO_DUMP )

	DISK_REGION( "ide" )
	DISK_IMAGE( "landhigh", 0, SHA1(7cea4ea5c3899e6ac774a4eb12821f44541d9c9c) )
ROM_END

ROM_START( batlgear )
	ROM_REGION64_BE( 0x100000, "user1", 0 )
	TAITOTZ_BIOS

	ROM_REGION( 0x40000, "io_cpu", 0 )
        ROM_LOAD16_BYTE( "e68-07.ic14",  0x000000, 0x020000, CRC(554c6fd7) SHA1(9f203dead81c7ccf73d7fd462cab147cd17f890f) )
        ROM_LOAD16_BYTE( "e68-08.ic15",  0x000001, 0x020000, CRC(f1932380) SHA1(64d12e858af15a9ba8254917da13863ac7c9c050) )

	ROM_REGION( 0x10000, "sound_cpu", 0 ) /* Internal ROM :( */
	ROM_LOAD( "e68-01.ic7", 0x000000, 0x010000, NO_DUMP )

	DISK_REGION( "ide" )
	DISK_IMAGE( "batlgear", 0, SHA1(eab283839ad3e0a3e6be11f6482570db334eacca) )
ROM_END

ROM_START( batlgr2 )
	ROM_REGION64_BE( 0x100000, "user1", 0 )
	TAITOTZ_BIOS

	ROM_REGION( 0x40000, "io_cpu", 0 )
        ROM_LOAD16_BYTE( "e87-03.ic14",  0x000000, 0x020000, CRC(49ae7cd0) SHA1(15f07a6bb2044a85a2139481f1dc95a44520c929) )
        ROM_LOAD16_BYTE( "e87-04.ic15",  0x000001, 0x020000, CRC(59f8f75f) SHA1(f5595751b10c0033f460114c43f5e2c192fe61f1) )

	ROM_REGION( 0x10000, "sound_cpu", 0 ) /* Internal ROM :( */
	ROM_LOAD( "e68-01.ic7", 0x000000, 0x010000, NO_DUMP )

	DISK_REGION( "ide" )
	DISK_IMAGE( "bg2_204j", 0, SHA1(7ac100fba39ae0b93980c0af2f0212a731106912) )
ROM_END

GAME( 1999, taitotz,  0, taitotz, taitotz, 0, ROT0, "Taito", "Type Zero BIOS", GAME_NO_SOUND|GAME_NOT_WORKING|GAME_IS_BIOS_ROOT)
GAME( 1999, landhigh, taitotz, taitotz, taitotz, 0, ROT0, "Taito", "Landing High Japan", GAME_NOT_WORKING | GAME_NO_SOUND )
GAME( 1999, batlgear, taitotz, taitotz, taitotz, 0, ROT0, "Taito", "Battle Gear", GAME_NOT_WORKING | GAME_NO_SOUND )
GAME( 2000, batlgr2,  taitotz, taitotz, taitotz, 0, ROT0, "Taito", "Battle Gear 2 (v2.04J)", GAME_NOT_WORKING | GAME_NO_SOUND )

