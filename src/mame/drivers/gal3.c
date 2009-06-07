/* Galaxian 3

  skeleton driver

  the hardware sits somewhere between Namco S21 and Namco S22
  multiple boards are used to drive multiple screens and multiple laserdiscs

  the version here is for a 2 screen system

 ------- info from Andy -------------


Namco 'Galaxian 3 - Theater 6 : Project Dragoon'
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

Dumped by:
Andrew Welburn
http://www.andys-arcade.com
on a cold evening 21/11/08


This romset was obtained from a 'spares' set of pcbs, the pcbs were never
used in a game, and the set included one of every type of pcb in the machine.
This means that there were 11 pcbs of 17 needed for a full set. The
additional 6 pcbs are all duplicates of pcbs in the set (see, what i got
was a 'spares' set ;)

Anyway...

from looking at Darth_nuno's photos, i can see that a full PCB set comprises :

2x backplane pcbs
2x CPU pcbs (one with master MST roms, one with slave SLV roms)
1x CRAM pcb
1x RS (RS0) pcb
1x SOUND pcb
2x OBJ pcb
2x PGN pcb
2x DSP pcb
1x VMIX pcb
3x PERSONAL pcbs

This make 17 pcbs in total needed for a complete set running Zolgear.

I have 11 pcbs (one of each type) one assumes it might be possible to run
the game with what i have, but who knows.

Darth_nuno's machine (and pcbs) are actually 'Attack of the Zolgear',
the update 'kit' for 'Namco Galaxian 3 theater 6'.

The pcb set i have dumped here is for a plain Galaxian 3 : Project Dragoon,
all my rom labels differ from his, and they are different games.

Of the 11 pcbs, 7 of them contain unique game roms. In order to keep
things clear, i have kept the roms apart in seperate folders, each
folder contains a photo of the pcb itself and another text file
containing location descriptions.

enjoy.

Andrew Welburn


--------------
better notes (complete chip lists) for each board still needed
--------------

*/

#include "driver.h"
#include "cpu/m68000/m68000.h"

static VIDEO_START(gal3)
{

}

static VIDEO_UPDATE(gal3)
{
	return 0;
}

static INPUT_PORTS_START( gal3 )
INPUT_PORTS_END

static ADDRESS_MAP_START( cpu_mst_map, ADDRESS_SPACE_PROGRAM, 32 )
	AM_RANGE(0x000000, 0x1fffff) AM_ROM
ADDRESS_MAP_END

static ADDRESS_MAP_START( cpu_slv_map, ADDRESS_SPACE_PROGRAM, 32 )
	AM_RANGE(0x000000, 0x07ffff) AM_ROM
ADDRESS_MAP_END

static ADDRESS_MAP_START( psn_b1_cpu_map, ADDRESS_SPACE_PROGRAM, 16 )
	AM_RANGE(0x000000, 0x03ffff) AM_ROM
ADDRESS_MAP_END

static ADDRESS_MAP_START( rs_cpu_map, ADDRESS_SPACE_PROGRAM, 16 )
	AM_RANGE(0x000000, 0x03ffff) AM_ROM
ADDRESS_MAP_END

static ADDRESS_MAP_START( sound_cpu_map, ADDRESS_SPACE_PROGRAM, 16 )
	AM_RANGE(0x000000, 0x03ffff) AM_ROM
ADDRESS_MAP_END


static MACHINE_DRIVER_START( gal3 )
	MDRV_CPU_ADD("cpumst", M68020, 24000000) // ??
	MDRV_CPU_PROGRAM_MAP(cpu_mst_map)

	MDRV_CPU_ADD("cpuslv", M68020, 24000000) // ??
	MDRV_CPU_PROGRAM_MAP(cpu_slv_map)

	MDRV_CPU_ADD("psn_b1_cpu", M68000, 12000000) // ??
	MDRV_CPU_PROGRAM_MAP(psn_b1_cpu_map)

	MDRV_CPU_ADD("psn_b2_cpu", M68000, 12000000) // ??
	MDRV_CPU_PROGRAM_MAP(psn_b1_cpu_map)

	MDRV_CPU_ADD("psn_b3_cpu", M68000, 12000000) // ??
	MDRV_CPU_PROGRAM_MAP(psn_b1_cpu_map)

	MDRV_CPU_ADD("rs_cpu", M68000, 12000000) // ??
	MDRV_CPU_PROGRAM_MAP(rs_cpu_map)

	MDRV_CPU_ADD("sound_cpu", M68000, 12000000) // ??
	MDRV_CPU_PROGRAM_MAP(sound_cpu_map)



	MDRV_SCREEN_ADD("screen", RASTER)
	MDRV_SCREEN_REFRESH_RATE(60)
	MDRV_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(0))
	MDRV_SCREEN_FORMAT(BITMAP_FORMAT_INDEXED16)
	MDRV_SCREEN_SIZE(64*8, 32*8)
	MDRV_SCREEN_VISIBLE_AREA(0*8, 320-1, 0*8, 224-1)

	MDRV_PALETTE_LENGTH(0x200)

	MDRV_VIDEO_START(gal3)
	MDRV_VIDEO_UPDATE(gal3)
MACHINE_DRIVER_END

/*

**************************************************************************************************
MASTER CPU PCB
**************************************************************************************************

PCB markings:
screened : 8623961202 CPU020
Etched   : (8623963202)


label       loc.    Device      Filename
--------------------------------------------------------
GLC1-MST-PRG0E  6B  27c4001     PRG0E
GLC1-MST-PRG1E  10B 27c4001     PRG1E
GLC1-MST-PRG2E  14B 27c4001     PRG2E
GLC1-MST-PRG3E  18B 27c4001     PRG3E

**************************************************************************************************
SLAVE CPU PCB
**************************************************************************************************

PCB markings:
screened : 8623961202 CPU020
Etched   : (8623963202)


label       loc.    Device      Filename
--------------------------------------------------------
GLC-SLV-PRG0    6B  27c010A     PRG0.bin
GLC-SLV-PRG1    10B 27c010A     PRG1.bin
GLC-SLV-PRG2    14B 27c010A     PRG2.bin
GLC-SLV-PRG3    18B 27c010A     PRG3.bin

**************************************************************************************************
DSP PCB
**************************************************************************************************


PCB markings:
screened : 8623961703 DSP
Etched   : (8623963703) TSK-A


label       loc.    Device      Filename
--------------------------------------------------------
GLC1-DSP-PTOH   2F  27c040      PTOH.BIN
GLC1-DSP-PTOU   2K  27c040      PTOU.BIN
GLC1-DSP-PTOL   2N  27c040      PTOL.BIN

**************************************************************************************************
OBJ PCB
**************************************************************************************************

PCB markings:
screened : 8623962002
Etched   : (8623964002)


label       loc.    Device      Filename
--------------------------------------------------------
GLC1-OBJ-OBJ0   9T  27c040      OBJ0.BIN
GLC1-OBJ-OBJ2   9W  27c4000     OBJ2.BIN
GLC1-OBJ-OBJ1   9Y  27c040      OBJ1.BIN
GLC1-OBJ-OBJ3   9Z  27c4000     OBJ0.BIN

**************************************************************************************************
PSN PCB
**************************************************************************************************


PCB markings:
screened : V1079603
Etched   : (V1079703)


label       loc.    Device      Filename
--------------------------------------------------------
GLC-PSN-VOL IC100   27c010A     VOL.bin
GLC-PSN-PRG0B   IC22    27c010A     PRG0B.bin
GLC-PSN-PRG0B   IC23    27c010A     PRG1B.bin


PCB markings:
screened : V107960701
Etched   : (V107970701) TSK-A

**************************************************************************************************
RS PCB
**************************************************************************************************

label       loc.    Device      Filename
--------------------------------------------------------
GLC-RS-PRGLB    18B 27c010      PRGLB.BIN
GLC-RS-PRGUB    19B 27c010      PRGUB.BIN

**************************************************************************************************
SOUND PCB
**************************************************************************************************


PCB markings:
screened : V107965101
Etched   : (V107975101)


label       loc.    Device      Filename
--------------------------------------------------------
GLC1-SND-VOI0   13A 27c040      VOI0.BIN
GLC1-SND-VOI2   13C 27c040      VOI2.BIN
GLC1-SND-VOI8   10G 27c040      VOI8.BIN
GLC1-SND-VOI9   11/12G  27c040      VOI9.BIN
GLC1-SND-VOI10  13G 27c040      VOI10.BIN
GLC1-SND-VOI11  14G 27c040      VOI11.BIN

GLC1-SND-PRG0   1H  27c1000     PRG0.BIN
GLC1-SND-PRG1   2H  27c1000     PRG1.BIN
GLC1-SND-DATA1  4/5H    27c1000     DATA1.BIN


*/


ROM_START( gal3 )
	/********* CPU-MST board x1 *********/
	ROM_REGION( 0x200000, "cpumst", 0 ) /* 68020 Code */
	ROM_LOAD32_BYTE( "glc1-mst-prg0e.6b", 0x00003, 0x80000, CRC(5deccd72) SHA1(8d50779221538cc171469a691fabb17b62a8e664) )
	ROM_LOAD32_BYTE( "glc1-mst-prg1e.10b", 0x00002, 0x80000, CRC(b6144e3b) SHA1(33f63d881e7012db7f971b074bc5f876a66198b7) )
	ROM_LOAD32_BYTE( "glc1-mst-prg2e.14b", 0x00001, 0x80000, CRC(13381084) SHA1(486c1e136e6594ba68858e40246c5fb9bef1c0d2) )
	ROM_LOAD32_BYTE( "glc1-mst-prg3e.18b", 0x00000, 0x80000, CRC(7917584a) SHA1(ec22de8a3751099d37e14cd05c736c33baa1ee1d) )

	/********* CPU-SLV board x1 *********/
	ROM_REGION( 0x080000, "cpuslv", 0 ) /* 68020 Code */
	ROM_LOAD32_BYTE( "glc-slv-prg0.6b",  0x00003, 0x20000, CRC(75b2341a) SHA1(73616f5633f583b9ebfba2380fde3e7b08743e9f) )
	ROM_LOAD32_BYTE( "glc-slv-prg1.10b", 0x00002, 0x20000, CRC(f37ba6c0) SHA1(f8ee29ee4d341bfd6595e92c090865b8e5d9578c) )
	ROM_LOAD32_BYTE( "glc-slv-prg2.14b", 0x00001, 0x20000, CRC(c38a933e) SHA1(96c85db607d8527e927ef23fc53324172ddb861a) )
	ROM_LOAD32_BYTE( "glc-slv-prg3.18b", 0x00000, 0x20000, CRC(deae86d2) SHA1(1898955423b8da585b6319406566aad02db20d64) )

	/********* DSP board x2 *********/
	ROM_REGION32_BE( 0x400000, "dsp_board1", ROMREGION_ERASE ) /* 24bit signed point data */
	ROM_LOAD32_BYTE( "glc1-dsp-ptoh.2f", 0x000001, 0x80000, CRC(b4213c83) SHA1(9d036b73149656fdc13eed38946a70f532bff3f1) )	/* most significant */
	ROM_LOAD32_BYTE( "glc1-dsp-ptou.2k", 0x000002, 0x80000, CRC(14877cef) SHA1(5ebdccd6db837ceb9473bd219eb211431944cbf0) )
	ROM_LOAD32_BYTE( "glc1-dsp-ptol.2n", 0x000003, 0x80000, CRC(b318534a) SHA1(6fcf2ead6dd0d5a6f22438520588ba4e33ca39a8) )	/* least significant */
	/* and 5x C67 (TMS320C25) */

	ROM_REGION32_BE( 0x400000, "dsp_board2", ROMREGION_ERASE ) /* 24bit signed point data */
	ROM_LOAD32_BYTE( "glc1-dsp-ptoh.2f", 0x000001, 0x80000, CRC(b4213c83) SHA1(9d036b73149656fdc13eed38946a70f532bff3f1) )	/* most significant */
	ROM_LOAD32_BYTE( "glc1-dsp-ptou.2k", 0x000002, 0x80000, CRC(14877cef) SHA1(5ebdccd6db837ceb9473bd219eb211431944cbf0) )
	ROM_LOAD32_BYTE( "glc1-dsp-ptol.2n", 0x000003, 0x80000, CRC(b318534a) SHA1(6fcf2ead6dd0d5a6f22438520588ba4e33ca39a8) )	/* least significant */
	/* and 5x C67 (TMS320C25) */

	/********* OBJ board x2 *********/
	ROM_REGION( 0x200000, "obj_board1", 0 )
	ROM_LOAD( "glc1-obj-obj0.9t", 0x000000, 0x80000, CRC(0fe98d33) SHA1(5cfefa342fe2fa278d010927d761cb51105a4a60) )
	ROM_LOAD( "glc1-obj-obj1.9w", 0x080000, 0x80000, CRC(660a4f6d) SHA1(c3c3525f51280e71f2d607649a6b5434cbd862c8) )
	ROM_LOAD( "glc1-obj-obj2.9y", 0x100000, 0x80000, CRC(90bcc5a3) SHA1(76cb23e295bb15279e046e83f8e4ab9f85f68243) )
	ROM_LOAD( "glc1-obj-obj3.9z", 0x180000, 0x80000, CRC(65244f07) SHA1(fd876ca5f198914f15864397b358e56fcaa41e90) )

	ROM_REGION( 0x200000, "obj_board2", 0 )
	ROM_LOAD( "glc1-obj-obj0.9t", 0x000000, 0x80000, CRC(0fe98d33) SHA1(5cfefa342fe2fa278d010927d761cb51105a4a60) )
	ROM_LOAD( "glc1-obj-obj1.9w", 0x080000, 0x80000, CRC(660a4f6d) SHA1(c3c3525f51280e71f2d607649a6b5434cbd862c8) )
	ROM_LOAD( "glc1-obj-obj2.9y", 0x100000, 0x80000, CRC(90bcc5a3) SHA1(76cb23e295bb15279e046e83f8e4ab9f85f68243) )
	ROM_LOAD( "glc1-obj-obj3.9z", 0x180000, 0x80000, CRC(65244f07) SHA1(fd876ca5f198914f15864397b358e56fcaa41e90) )

	/********* PSN board x3 *********/
	ROM_REGION( 0x040000, "psn_b1_cpu", 0 )
	ROM_LOAD16_BYTE( "glc-psn-prg0b.ic22", 0x000001, 0x20000, CRC(da8a74f8) SHA1(2826a55a4a0acec07ff760c7857da10c4ffaf7d0) )
	ROM_LOAD16_BYTE( "glc-psn-prg1b.ic23", 0x000000, 0x20000, CRC(978431c9) SHA1(7d631444f5844c55bea820507e34a17199f5da2e) )
	ROM_REGION( 0x020000, "psn_b1_vol", 0 )
	ROM_LOAD( "glc-psn-vol.ic100", 0x000000, 0x20000, CRC(9d49576c) SHA1(25c02d2cc171468711c71d8f2da0ea7d9b5f0c23) )

	ROM_REGION( 0x040000, "psn_b2_cpu", 0 )
	ROM_LOAD16_BYTE( "glc-psn-prg0b.ic22", 0x000001, 0x20000, CRC(da8a74f8) SHA1(2826a55a4a0acec07ff760c7857da10c4ffaf7d0) )
	ROM_LOAD16_BYTE( "glc-psn-prg1b.ic23", 0x000000, 0x20000, CRC(978431c9) SHA1(7d631444f5844c55bea820507e34a17199f5da2e) )
	ROM_REGION( 0x020000, "psn_b2_vol", 0 )
	ROM_LOAD( "glc-psn-vol.ic100", 0x000000, 0x20000, CRC(9d49576c) SHA1(25c02d2cc171468711c71d8f2da0ea7d9b5f0c23) )

	ROM_REGION( 0x040000, "psn_b3_cpu", 0 )
	ROM_LOAD16_BYTE( "glc-psn-prg0b.ic22", 0x000001, 0x20000, CRC(da8a74f8) SHA1(2826a55a4a0acec07ff760c7857da10c4ffaf7d0) )
	ROM_LOAD16_BYTE( "glc-psn-prg1b.ic23", 0x000000, 0x20000, CRC(978431c9) SHA1(7d631444f5844c55bea820507e34a17199f5da2e) )
	ROM_REGION( 0x020000, "psn_b3_vol", 0 )
	ROM_LOAD( "glc-psn-vol.ic100", 0x000000, 0x20000, CRC(9d49576c) SHA1(25c02d2cc171468711c71d8f2da0ea7d9b5f0c23) )

	/********* RS board x1 *********/
	ROM_REGION( 0x040000, "rs_cpu", 0 )
	ROM_LOAD16_BYTE( "glc-rs-prglb.18b", 0x000001, 0x20000, CRC(9d0c8d03) SHA1(8fffef622cd4440ea9f17882cd54a8a49fbbc148) )
	ROM_LOAD16_BYTE( "glc-rs-prgub.19b", 0x000000, 0x20000, CRC(125ad94c) SHA1(4e2e316b639e9a3a78ecd5c827f3309efa3bc78c) )

	/********* SOUND board x1 *********/
	ROM_REGION( 0x080000, "sound_cpu", ROMREGION_ERASE00 )
	ROM_LOAD16_BYTE( "glc1-snd-prg0.1h", 0x000000, 0x20000, CRC(4845481c) SHA1(3cf90b8b2351b2bc015bf273552e19e09f84ee70) )
	ROM_LOAD16_BYTE( "glc1-snd-prg1.2h", 0x000001, 0x20000, CRC(3b98c175) SHA1(26e59700347bab7fa10f029e781f993f3a97d257) )
	ROM_LOAD16_BYTE( "glc1-snd-data1.4h",0x040001, 0x20000, CRC(8c7135f5) SHA1(b8c3866c70ac1c431140d6cfe50d9273db7d9b68) )

	ROM_REGION( 0x080000, "samples", ROMREGION_ERASE00 )
	ROM_LOAD( "glc1-snd-voi0.13a", 0x000000, 0x80000, CRC(ef3bda56) SHA1(2cdfec1860a6d2bd645d83b42cc232643818a699) )
	ROM_LOAD( "glc1-snd-voi2.13c", 0x000000, 0x80000, CRC(ef3bda56) SHA1(2cdfec1860a6d2bd645d83b42cc232643818a699) ) // == voi0
	ROM_LOAD( "glc1-snd-voi8.10g", 0x000000, 0x80000, CRC(bba0c15b) SHA1(b0abc22fd1ae8a9970ad45d9ebdb38e6b06033a7) )
	ROM_LOAD( "glc1-snd-voi9.11g", 0x000000, 0x80000, CRC(dd1b1ee4) SHA1(b69af15acaa9c3d79d7758adc8722ff5c1129b76) )
	ROM_LOAD( "glc1-snd-voi10.13g",0x000000, 0x80000, CRC(1c1dedf4) SHA1(b6b9dac68103ff2206d731d409a557a71afd98f7) )
	ROM_LOAD( "glc1-snd-voi11.14g",0x000000, 0x80000, CRC(559e2a8a) SHA1(9a2f28305c6073a0b9b80a5d9617cc25a921e9d0))

	/********* Laserdiscs *********/
	/* used 2 apparently, no idea what they connect to */

	DISK_REGION( "laserdisc1" )
	DISK_IMAGE_READONLY( "gal3_ld1", 0, NO_DUMP )

	DISK_REGION( "laserdisc2" )
	DISK_IMAGE_READONLY( "gal3_ld2", 0, NO_DUMP )
ROM_END

GAME( 1992, gal3,    0,        gal3,    gal3,    0, ROT0,  "Namco", "Galaxian 3 - Theater 6 : Project Dragoon", GAME_NOT_WORKING | GAME_NO_SOUND )
