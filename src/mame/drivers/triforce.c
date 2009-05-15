/* Triforce Skeleton
 -- split from Naomi.c

 Triforce uses

a stock Gamecube motherboard with custom Bios
a 'media board' which acts as a CD/DVD emulator
the Naomi 'DIMM' board, which connects to the GD-ROM drive

*/

#include "driver.h"
#include "cpu/powerpc/ppc.h"
#include "video/generic.h"
#include "naomibd.h"

static ADDRESS_MAP_START( gc_map, ADDRESS_SPACE_PROGRAM, 32 )
	AM_RANGE(0x7fe00000, 0x7fffffff) AM_ROM AM_REGION("maincpu", 0) AM_SHARE(2)	/* Program ROM */
ADDRESS_MAP_END


static VIDEO_START(triforce)
{

}

static VIDEO_UPDATE(triforce)
{
	return 0;
}

static INPUT_PORTS_START( triforce )
INPUT_PORTS_END

static MACHINE_DRIVER_START( triforce_base )

	/* basic machine hardware */
	MDRV_CPU_ADD("maincpu", PPC403GA, 64000000) /* Wrong! */
	MDRV_CPU_PROGRAM_MAP(gc_map)

	MDRV_QUANTUM_TIME(HZ(6000))

 	/* video hardware */
	MDRV_SCREEN_ADD("screen", RASTER)
	MDRV_SCREEN_REFRESH_RATE(60)
	MDRV_SCREEN_FORMAT(BITMAP_FORMAT_RGB32)
	MDRV_SCREEN_SIZE(640, 480)
	MDRV_SCREEN_VISIBLE_AREA(0, 639, 0, 479)

	MDRV_PALETTE_LENGTH(65536)

	MDRV_VIDEO_START(triforce)
	MDRV_VIDEO_UPDATE(triforce)
MACHINE_DRIVER_END

static MACHINE_DRIVER_START( triforcegd )
	MDRV_IMPORT_FROM(triforce_base)
	MDRV_NAOMI_DIMM_BOARD_ADD("rom_board", "gdrom", "user1", "picreturn")
MACHINE_DRIVER_END

#define ROM_LOAD16_WORD_SWAP_BIOS(bios,name,offset,length,hash) \
		ROMX_LOAD(name, offset, length, hash, ROM_GROUPWORD | ROM_BIOS(bios+1)) /* Note '+1' */

#define TRIFORCE_BIOS \
	ROM_REGION( 0x200000, "maincpu", 0) \
	ROM_SYSTEM_BIOS( 0, "bios0", "Triforce Bios" ) \
	ROM_LOAD16_WORD_SWAP_BIOS( 0,  "triforce_bootrom.bin", 0x000000, 0x200000, CRC(d1883221) SHA1(c3cb7227e4dbc2af861e76d00cb59726105a2e4c) ) \

ROM_START( triforce )
	TRIFORCE_BIOS

	ROM_REGION( 0x8400000, "user1", ROMREGION_ERASE)
ROM_END

ROM_START( vs2002j )
	TRIFORCE_BIOS

	ROM_REGION( 0x20000000, "user1", ROMREGION_ERASE) // allocate max size in init instead?

	DISK_REGION( "gdrom" )
	DISK_IMAGE_READONLY( "gdt-0001", 0, SHA1(1b4b16b0715fa5717904f0b3141cc48cca99b7a4) )

	ROM_REGION( 0x50, "picreturn", ROMREGION_ERASE)
	ROM_LOAD("gdt-0001.data", 0x00, 0x50, CRC(4a1fca38) SHA1(3bc6dca4f8faba44bf5c5a8012cffc69dbb6aea2) )
ROM_END

/*
Title   VIRTUA_STRIKER_2002
Media ID    0DD8
Media Config    GD-ROM1/1
Regions J
Peripheral String   0000000
Product Number  GDT-0002
Version V1.005
Release Date    20020730


PIC

253-5508-337E
317-0337-EXP
*/

ROM_START( vs2002ex )
	TRIFORCE_BIOS


	ROM_REGION( 0x20000000, "user1", ROMREGION_ERASE) // allocate max size in init instead?

	DISK_REGION( "gdrom" )
	DISK_IMAGE_READONLY( "gdt-0002", 0, SHA1(471e896d43167c93cc229cfc94ff7ac6de7cf9a4) )

	ROM_REGION( 0x50, "picreturn", ROMREGION_ERASE)
	ROM_LOAD("317-0337-exp.data", 0x00, 0x50, CRC(aa6be604) SHA1(fabc43ecfb7ddf1d5a87f10884852027d6f4773b) )
ROM_END


ROM_START( avalons )
	TRIFORCE_BIOS


	ROM_REGION( 0x20000000, "user1", ROMREGION_ERASE) // allocate max size in init instead?

	DISK_REGION( "gdrom" )
	DISK_IMAGE_READONLY( "gdt-0005c", 0, SHA1(9edb3d9ff492d2207d57bfdb6859e796f76c5e0c) )

	ROM_REGION( 0x50, "picreturn", ROMREGION_ERASE)
	ROM_LOAD("gdt-0005.data", 0x00, 0x50, CRC(6c51e5d6) SHA1(84afef983f1f855fe8722f55baa8ea5121da9369) )
ROM_END


ROM_START( gekpurya )
	TRIFORCE_BIOS


	ROM_REGION( 0x20000000, "user1", ROMREGION_ERASE) // allocate max size in init instead?

	DISK_REGION( "gdrom" )
	DISK_IMAGE_READONLY( "gdt-0008c", 0, SHA1(2c1bdb8324efc216edd771fe45c680ac726111a0) )

	ROM_REGION( 0x50, "picreturn", ROMREGION_ERASE)
	ROM_LOAD("317-0371-jpn.data", 0x00, 0x50, CRC(08434e5e) SHA1(2121999e851f6f62ab845e6de40849d850ac9d1c) )
ROM_END


ROM_START( tfupdate )
	TRIFORCE_BIOS


	ROM_REGION( 0x20000000, "user1", ROMREGION_ERASE) // allocate max size in init instead?

	DISK_REGION( "gdrom" )
	DISK_IMAGE_READONLY( "gdt-0011", 0, SHA1(71bfa8f53d211085c020d54f55eeeabf85212a0b) )

	ROM_REGION( 0x50, "picreturn", ROMREGION_ERASE)
	ROM_LOAD("gdt-0011.data", 0x00, 0x50, CRC(08434e5e) SHA1(2121999e851f6f62ab845e6de40849d850ac9d1c) )
ROM_END


/*

Title   VIRTUA STRIKER 4
Media ID    93B2
Media Config    GD-ROM1/1
Regions J
Peripheral String   0000000
Product Number  GDT-0015
Version V1.001
Release Date    20041202
Manufacturer ID
TOC DISC
Track   Start Sector    End Sector  Track Size
track01.bin 150 449 705600
track02.raw 600 1951    3179904
track03.bin 45150   549299  1185760800


PIC
255-5508-393E
317-0393-EXP

*/

ROM_START( vs4 )
	TRIFORCE_BIOS


	ROM_REGION( 0x20000000, "user1", ROMREGION_ERASE) // allocate max size in init instead?

	DISK_REGION( "gdrom" )
	DISK_IMAGE_READONLY( "gdt-0015", 0, SHA1(1f83712b2b170d6edf4a27c15b6f763cc3cc4b71) )

	ROM_REGION( 0x50, "picreturn", ROMREGION_ERASE)
	ROM_LOAD("317-0393-exp.data", 0x00, 0x50, CRC(2dcfecd7) SHA1(d805168e1564051ae5c47876ade2c9843253c6b4) )
ROM_END


/*

Title   VIRTUA STRIKER 4
Media ID    7BC9
Media Config    GD-ROM1/1
Regions J
Peripheral String   0000000
Product Number  GDT-0013E
Version V6.000
Release Date    20050217


PIC
255-5508-391J
317-0391-JPN

*/

ROM_START( vs4j )
	TRIFORCE_BIOS


	ROM_REGION( 0x20000000, "user1", ROMREGION_ERASE) // allocate max size in init instead?

	DISK_REGION( "gdrom" )
	DISK_IMAGE_READONLY( "gdt-0013e", 0, SHA1(b69cc5cab889114eda5c6e9ddcca42de9bc235b3) )

	ROM_REGION( 0x50, "picreturn", ROMREGION_ERASE)
	ROM_LOAD("317-0391-jpn.data", 0x00, 0x50, CRC(0f2dbb73) SHA1(7b9d66abe85303b3e26b442a3a63feca1a0edbdb) )
ROM_END


/*

Title   TRF GDROM TBA EX SATL
Media ID    92C5
Media Config    GD-ROM1/1
Regions J
Peripheral String   0000000
Product Number  GDT-0010C
Version V4.000
Release Date    20040608
Manufacturer ID

TOC DISC
Track   Start Sector    End Sector  Track Size
track01.bin 150 599 1058400
track02.raw 750 2101    3179904
track03.bin 45150   549299  1185760800
*/

ROM_START( avalon13 )
	TRIFORCE_BIOS


	ROM_REGION( 0x20000000, "user1", ROMREGION_ERASE) // allocate max size in init instead?

	DISK_REGION( "gdrom" )
	DISK_IMAGE_READONLY( "gdt-0010c", 0, SHA1(716c441d8dc9036a13c66ef0048cd6d32ac63c4e) )

	ROM_REGION( 0x50, "picreturn", ROMREGION_ERASE)
	ROM_LOAD("gdt-0010c.data", 0x00, 0x50, CRC(6c51e5d6) SHA1(84afef983f1f855fe8722f55baa8ea5121da9369) )
ROM_END


/*

Title   TRF GDROM TBT SATL
Media ID    1348
Media Config    GD-ROM1/1
Regions J
Peripheral String   0000000
Product Number  GDT-0017B
Version V3.001
Release Date    20041102
Manufacturer ID

TOC DISC
Track   Start Sector    End Sector  Track Size
track01.bin 150 599 1058400
track02.raw 750 2101    3179904
track03.bin 45150   549299  1185760800

*/

ROM_START( avalon20 )
	TRIFORCE_BIOS


	ROM_REGION( 0x20000000, "user1", ROMREGION_ERASE) // allocate max size in init instead?

	DISK_REGION( "gdrom" )
	DISK_IMAGE_READONLY( "gdt-0017b", 0, SHA1(e2dd32c322ffcaf38b82275d2721b71bb3dfc1f2) )

	ROM_REGION( 0x50, "picreturn", ROMREGION_ERASE)
	ROM_LOAD("gdt-0017b.data", 0x00, 0x50, CRC(32cb46d4) SHA1(a58b9e03d57b317133d9b6c29e42852af8e77559) )
ROM_END


ROM_START( vs42006 )
	TRIFORCE_BIOS


	ROM_REGION( 0x20000000, "user1", ROMREGION_ERASE) // allocate max size in init instead?

	DISK_REGION( "gdrom" )
	DISK_IMAGE_READONLY( "gdt-0020d", 0, SHA1(db256d094b9754d452d7a2b8a370699d21141c1f) )

	ROM_REGION( 0x50, "picreturn", ROMREGION_ERASE)
	ROM_LOAD("gdt-0020.data", 0x00, 0x50, CRC(e3d13191) SHA1(4255c09aad06eb38c16bdec881897404a3a68b37) )
ROM_END



GAME( 2002, triforce, 0,        triforcegd,    triforce,    0, ROT0, "Sega",           "Triforce Bios", GAME_NO_SOUND|GAME_NOT_WORKING|GAME_IS_BIOS_ROOT )
GAME( 2002, vs2002j,  triforce, triforcegd,    triforce,    0, ROT0, "Sega",           "Virtua Striker 2002 (GDT-0001)", GAME_NO_SOUND|GAME_NOT_WORKING )
GAME( 2002, vs2002ex, triforce, triforcegd,    triforce,    0, ROT0, "Sega",           "Virtua Striker 2002 (GDT-0002)", GAME_NO_SOUND|GAME_NOT_WORKING )
GAME( 2003, avalons,  triforce, triforcegd,    triforce,    0, ROT0, "Sega",           "The Key Of Avalon - The Wizard Master - Server (GDT-0005C) (V4.001)", GAME_NO_SOUND|GAME_NOT_WORKING )
GAME( 2003, gekpurya, triforce, triforcegd,    triforce,    0, ROT0, "Sega",           "Gekitou Pro Yakyuu Mizushima Shinji All Stars vs. Pro Yakyuu (Rev C) (GDT-0008C)", GAME_NO_SOUND|GAME_NOT_WORKING )
GAME( 2004, avalon13, triforce, triforcegd,    triforce,    0, ROT0, "Sega",           "The Key Of Avalon 1.3 - Chaotic Sabbat - Client (GDT-0010C) (V4.000)", GAME_NO_SOUND|GAME_NOT_WORKING )
GAME( 200?, tfupdate, triforce, triforcegd,    triforce,    0, ROT0, "Sega",           "Triforce DIMM Updater (GDT-0011)", GAME_NO_SOUND|GAME_NOT_WORKING )
GAME( 2004, vs4j,     triforce, triforcegd,    triforce,    0, ROT0, "Sega",           "Virtua Striker 4 (Japan) (GDT-0013E)", GAME_NO_SOUND|GAME_NOT_WORKING )
GAME( 2004, vs4,      triforce, triforcegd,    triforce,    0, ROT0, "Sega",           "Virtua Striker 4 (Export) (GDT-0015)", GAME_NO_SOUND|GAME_NOT_WORKING )
GAME( 2004, avalon20, triforce, triforcegd,    triforce,    0, ROT0, "Sega",           "The Key Of Avalon 2.0 - Eutaxy and Commandment - Client (GDT-0017B) (V3.001)", GAME_NO_SOUND|GAME_NOT_WORKING )
GAME( 2006, vs42006,  triforce, triforcegd,    triforce,    0, ROT0, "Sega",           "Virtua Striker 4 ver. 2006 (Rev D) (Japan) (GDT-0020D)", GAME_NO_SOUND|GAME_NOT_WORKING )


