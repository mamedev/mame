/*

Chihiro is an Xbox based arcade motherboard from SEGA
A Chihiro system consists of network board, media board, base board & Xbox board

The whole system is divided into 2 parts and each part has two boards.
The upper part contains a media board with a TSOP48 where there is an xbox .xbe loader
(this is the dashboard you see when you power the Chihiro) and for a network board (100% the same
as the one in the Triforce v3, same firmware also)

The bottom section consists of an Xbox board with 128MB of RAM and with a different MCPX than
a retail one and a base board that handles JVS and Video.

Network Board Dump : Ver1305.bin
Media Board dump   : FPR21042_M29W160ET.bin
Base Board Dumps   : ic10_g24lc64.bin ic11_24lc024.bin pc20_g24lc64.bin
Xbox Board Dump    : Not dumped

FPR21042_M29W160ET.bin :
As in Triforce, it consists of two versions in the same flash, the first MB of the flash has
an older version as backup, and the second MB has the current version, versions included are:
SegaBoot Ver.2.00.0 Build:Feb  7 2003 12:28:30
SegaBoot Ver.2.13.0 Build:Mar  3 2005 17:03:15

ic10_g24lc64.bin: This dump contains the firmware of the Base Board, serial number and REGION of the whole system
Region is located at Offset 0x00001F10 , 01 means JAP, 02 Means USA, 03 Means EXPORT, if you
want to change the region of your Chihiro Board, just change this byte.
pc20_g24lc64.bin: it seems a backup of the base board without region or serial, older version maybe?
ic11_24lc024.bin: this is the mysterious one, as the previous 2, its on the Base Board, and just contains some
strings the interesting thing is that it contains the string SBJE and if you go to the system info menu
on the Chihiro and you press service button 16 times in a row, a message will be displayed: GAME ID SBJE

Thanks to Alex, Mr Mudkips, and Philip Burke for this info.

*/

#include "emu.h"
#include "cpu/i386/i386.h"
#include "includes/naomibd.h"

static VIDEO_START(chihiro)
{

}

static VIDEO_UPDATE(chihiro)
{

	return 0;
}


/*
St.     Instr.       Comment
0x02    PEEK         ACC := MEM[OP1]
0x03    POKE         MEM[OP1] := OP2
0x04    POKEPCI      PCICONF[OP1] := OP2
0x05    PEEKPCI      ACC := PCICONF[OP1]
0x06    AND/OR       ACC := (ACC & OP1) | OP2
0x07    (prefix)     execute the instruction code in OP1 with OP1 := OP2, OP2 := ACC
0x08    BNE          IF ACC = OP1 THEN PC := PC + OP2
0x09    BRA          PC := PC + OP2
0x10    AND/OR ACC2  (unused/defunct) ACC2 := (ACC2 & OP1) | OP2
0x11    OUTB         PORT[OP1] := OP2
0x12    INB          ACC := PORT(OP1)
0xEE    END
*/
static READ32_HANDLER( chihiro_jamtable )
{
	return 0xEEEEEEEE;
}


static ADDRESS_MAP_START( xbox_map, ADDRESS_SPACE_PROGRAM, 32 )
	AM_RANGE(0x00000000, 0x004fffff) AM_RAM
	AM_RANGE(0x07fd0000, 0x07feffff) AM_RAM // a table of some sort?

	AM_RANGE(0xff000080, 0xff000083) AM_READ( chihiro_jamtable )
	AM_RANGE(0xfff00000, 0xfff7ffff) AM_ROM AM_SHARE("biosflash")
	AM_RANGE(0xfff80000, 0xffffffff) AM_ROM AM_REGION("bios", 0) AM_SHARE("biosflash")
ADDRESS_MAP_END

static INPUT_PORTS_START( chihiro )
INPUT_PORTS_END

static MACHINE_DRIVER_START( chihiro_base )

	/* basic machine hardware */
	MDRV_CPU_ADD("maincpu", PENTIUM, 733333333) /* Wrong! */
	MDRV_CPU_PROGRAM_MAP(xbox_map)

	MDRV_QUANTUM_TIME(HZ(6000))

	/* video hardware */
	MDRV_SCREEN_ADD("screen", RASTER)
	MDRV_SCREEN_REFRESH_RATE(60)
	MDRV_SCREEN_FORMAT(BITMAP_FORMAT_RGB32)
	MDRV_SCREEN_SIZE(640, 480)
	MDRV_SCREEN_VISIBLE_AREA(0, 639, 0, 479)

	MDRV_PALETTE_LENGTH(65536)

	MDRV_VIDEO_START(chihiro)
	MDRV_VIDEO_UPDATE(chihiro)
MACHINE_DRIVER_END

static MACHINE_DRIVER_START( chihirogd )
	MDRV_IMPORT_FROM(chihiro_base)
	MDRV_NAOMI_DIMM_BOARD_ADD("rom_board", "gdrom", "user1", "picreturn")
MACHINE_DRIVER_END

#define ROM_LOAD16_WORD_SWAP_BIOS(bios,name,offset,length,hash) \
		ROMX_LOAD(name, offset, length, hash, ROM_GROUPWORD | ROM_BIOS(bios+1)) /* Note '+1' */

#define ROM_LOAD_BIOS(bios,name,offset,length,hash) \
		ROMX_LOAD(name, offset, length, hash, ROM_BIOS(bios+1)) /* Note '+1' */

#define CHIHIRO_BIOS \
	ROM_REGION( 0x200000, "bios", 0) \
	ROM_SYSTEM_BIOS( 0, "bios0", "Chihiro Bios" ) \
	ROM_LOAD_BIOS( 0,  "chihiro_xbox_bios.bin", 0x000000, 0x80000, CRC(66232714) SHA1(b700b0041af8f84835e45d1d1250247bf7077188) ) \
	ROM_REGION( 0x200000, "others", 0) \
	ROM_LOAD16_WORD_SWAP_BIOS( 0,  "fpr21042_m29w160et.bin", 0x000000, 0x200000, CRC(a4fcab0b) SHA1(a13cf9c5cdfe8605d82150b7573652f419b30197) ) \
	ROM_LOAD16_WORD_SWAP_BIOS( 0,  "ic10_g24lc64.bin", 0x000000, 0x2000, CRC(cfc5e06f) SHA1(3ababd4334d8d57abb22dd98bd2d347df39648d9) ) \
	ROM_LOAD16_WORD_SWAP_BIOS( 0,  "ic11_24lc024.bin", 0x000000, 0x80, CRC(8dc8374e) SHA1(cc03a0650bfac4bf6cb66e414bbef121cba53efe) ) \
	ROM_LOAD16_WORD_SWAP_BIOS( 0,  "pc20_g24lc64.bin", 0x000000, 0x2000, CRC(7742ab62) SHA1(82dad6e2a75bab4a4840dc6939462f1fb9b95101) ) \
	ROM_LOAD16_WORD_SWAP_BIOS( 0,  "ver1305.bin", 0x000000, 0x200000, CRC(a738ea1c) SHA1(45d94d0c39be1cb3db9fab6610a88a550adda4e9) ) \

ROM_START( chihiro )
	CHIHIRO_BIOS

	ROM_REGION( 0x20000000, "user1", ROMREGION_ERASE) // allocate max size in init instead?

	DISK_REGION( "gdrom" )

	ROM_REGION( 0x50, "picreturn", ROMREGION_ERASE)
ROM_END



ROM_START( hotd3 )
	CHIHIRO_BIOS

	ROM_REGION( 0x20000000, "user1", ROMREGION_ERASE) // allocate max size in init instead?

	DISK_REGION( "gdrom" )
	DISK_IMAGE_READONLY( "gdx-0001", 0,  SHA1(174c72f851d0c97e8993227467f16b0781ed2f5c) )

	ROM_REGION( 0x50, "picreturn", ROMREGION_ERASE)
	ROM_LOAD("317-0348-com.data", 0x00, 0x50, CRC(d28219ef) SHA1(40dbbc092bc9f99b8d2ae67fbefacd62184f90ec) )
ROM_END

ROM_START( outr2 )
	CHIHIRO_BIOS

	ROM_REGION( 0x20000000, "user1", ROMREGION_ERASE) // allocate max size in init instead?

	DISK_REGION( "gdrom" )
	DISK_IMAGE_READONLY( "gdx-0004a", 0, SHA1(27acd2d0680e6bafa0d052f60b4372adc37224fd) )

	ROM_REGION( 0x50, "picreturn", ROMREGION_ERASE)
	ROM_LOAD("317-0372-com.data", 0x00, 0x50, CRC(a15c9666) SHA1(fd36c524744acb33e579ccb257c71375a5d3af67) )
ROM_END

/*

Title   GHOST SQUAD
Media ID    004F
Media Config    GD-ROM1/1
Regions J
Peripheral String   0000000
Product Number  GDX-0012A
Version V2.000
Release Date    20041209
Manufacturer ID

PIC
253-5508-0398
317-0398-COM

*/

ROM_START( ghostsqu )
	CHIHIRO_BIOS

	ROM_REGION( 0x20000000, "user1", ROMREGION_ERASE) // allocate max size in init instead?

	DISK_REGION( "gdrom" )
	DISK_IMAGE_READONLY( "gdx-0012a", 0,  SHA1(d7d78ce4992cb16ee5b4ac6ca7a37c46b07e8c14) )

	ROM_REGION( 0x50, "picreturn", ROMREGION_ERASE)
	ROM_LOAD("317-0398-com.data", 0x00, 0x50, CRC(8c5391a2) SHA1(e64cadeb30c94c3cd4002630cd79cc76c7bde2ed) )
ROM_END

/*

Title   VIRTUA COP 3
Media ID    C4AD
Media Config    GD-ROM1/1
Regions J
Peripheral String   0000000
Product Number  GDX-0003A
Version V2.004
Release Date    20030226
Manufacturer ID
TOC DISC
Track   Start Sector    End Sector  Track Size
track01.bin 150 599 1058400
track02.raw 750 2101    3179904
track03.bin 45150   549299  1185760800


PIC
255-5508-354
317-054-COM

*/

ROM_START( vcop3 )
	CHIHIRO_BIOS

	ROM_REGION( 0x20000000, "user1", ROMREGION_ERASE) // allocate max size in init instead?

	DISK_REGION( "gdrom" )
	DISK_IMAGE_READONLY( "gdx-0003a", 0,  SHA1(cdfec1d2ef02ae9e29cb1462f08904177bc4c9ea) )

	ROM_REGION( 0x50, "picreturn", ROMREGION_ERASE)
	ROM_LOAD("317-0354-com.data", 0x00, 0x50,  CRC(df7e3217) SHA1(9f0f4bf6b15f3b6eeea81eaa27b3d25bd94110da) )
ROM_END


ROM_START( mj2 )
	CHIHIRO_BIOS

	ROM_REGION( 0x20000000, "user1", ROMREGION_ERASE) // allocate max size in init instead?

	DISK_REGION( "gdrom" )
	DISK_IMAGE_READONLY( "gdx-0006c", 0, SHA1(505653117a73ed8b256ccf19450e7573a4dc57e9) )

	ROM_REGION( 0x50, "picreturn", ROMREGION_ERASE) // key was missing
	ROM_LOAD("gdx-0006c.pic_data", 0x00, 0x50, NO_DUMP )
ROM_END


ROM_START( wangmid )
	CHIHIRO_BIOS

	ROM_REGION( 0x20000000, "user1", ROMREGION_ERASE) // allocate max size in init instead?

	DISK_REGION( "gdrom" )
	DISK_IMAGE_READONLY( "gdx-0009b", 0, SHA1(e032b9fd8d5d09255592f02f7531a608e8179c9c) )

	ROM_REGION( 0x50, "picreturn", ROMREGION_ERASE)
	ROM_LOAD("gdx-0009b.data", 0x00, 0x50, CRC(3af801f3) SHA1(e9a2558930f3f1f55d5b3c2cadad69329d931f26) )
ROM_END


ROM_START( wangmid2 )
	CHIHIRO_BIOS

	ROM_REGION( 0x20000000, "user1", ROMREGION_ERASE) // allocate max size in init instead?

	DISK_REGION( "gdrom" )
	DISK_IMAGE_READONLY( "gdx-0015", 0, SHA1(259483fd211a70c23205ffd852316d616c5a2740) )

	ROM_REGION( 0x50, "picreturn", ROMREGION_ERASE)
	ROM_LOAD("317-5106-com.data", 0x00, 0x50, CRC(75c716aa) SHA1(5c2bcf3d28a80b336c6882d5aeb010d04327f8c1) )
ROM_END


ROM_START( mj3 )
	CHIHIRO_BIOS

	ROM_REGION( 0x20000000, "user1", ROMREGION_ERASE) // allocate max size in init instead?

	DISK_REGION( "gdrom" )
	DISK_IMAGE_READONLY( "gdx-0017d", 0, SHA1(cfbbd452c8f4efe0e99f398f5521fc3574b913bb) )

	ROM_REGION( 0x50, "picreturn", ROMREGION_ERASE) // key was missing
	ROM_LOAD("gdx-0017d.pic_data", 0x00, 0x50, NO_DUMP )
ROM_END


GAME( 200?, chihiro,  0,       chihiro_base, chihiro,    0, ROT0, "Sega",           "Chihiro Bios", GAME_NO_SOUND|GAME_NOT_WORKING|GAME_IS_BIOS_ROOT )
GAME( 2002, hotd3,    chihiro, chihirogd,    chihiro,    0, ROT0, "Sega",           "The House of the Dead III [GDX-0001]", GAME_NO_SOUND|GAME_NOT_WORKING )
GAME( 2002, outr2,    chihiro, chihirogd,    chihiro,    0, ROT0, "Sega",           "Out Run 2 (Rev. A) [GDX-0004A]", GAME_NO_SOUND|GAME_NOT_WORKING )
GAME( 2005, mj2,      chihiro, chihirogd,    chihiro,    0, ROT0, "Sega",           "Sega Network Taisen Mahjong MJ 2 (Rev C) [GDX-0006C]", GAME_NO_SOUND|GAME_NOT_WORKING )
GAME( 2005, wangmid,  chihiro, chihirogd,    chihiro,    0, ROT0, "Sega",           "Wangan Midnight Maximum Tune (Rev. B) (Export) (GDX-0009B)", GAME_NO_SOUND|GAME_NOT_WORKING )
GAME( 2005, wangmid2, chihiro, chihirogd,    chihiro,    0, ROT0, "Sega",           "Wangan Midnight Maximum Tune 2 (Export) (GDX-0015)", GAME_NO_SOUND|GAME_NOT_WORKING )
GAME( 2005, ghostsqu, chihiro, chihirogd,    chihiro,    0, ROT0, "Sega",           "Ghost Squad (Ver. A?) (GDX-0012A)", GAME_NO_SOUND|GAME_NOT_WORKING )
GAME( 2003, vcop3,    chihiro, chihirogd,    chihiro,    0, ROT0, "Sega",           "Virtua Cop 3 (GDX-0003A)", GAME_NO_SOUND|GAME_NOT_WORKING )
GAME( 2005, mj3,      chihiro, chihirogd,    chihiro,    0, ROT0, "Sega",           "Sega Network Taisen Mahjong MJ 3 (Rev D) [GDX-0017D]", GAME_NO_SOUND|GAME_NOT_WORKING )

