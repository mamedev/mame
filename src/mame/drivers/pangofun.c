/* It's a standard 486 PC motherboard, gfx card etc. with expansion ROM board

 probably impossible to emulate right now due to the bad / missing (blank when read) rom
 although it would be a good idea if somebody checked for sure

*/

/*

readme by f205v

Game: Pango Fun
Anno: 1995
Produttore: InfoCube (Pisa-Italy)
N.revisione: rl00rv00

CPUs:-----------

Main PCB (it's a standard 486 motherboard):

1x 80486
1x oscillator 14.31818 MHz

Video PCB (it's a standard VGA EISA board):

1x CIRRUS CLGD5401-42QC-B-31063-198AC

Sound PCB (missing, it's a standard ISA 16bit sound card):
?

ROMs PCB (it's a custom PCB, with EISA connector to motherboard on one side and JAMMA connetcor on the other side):
1x NE555P


ROMs:-----------

Main PCB (it's a standard 486 motherboard):
1x 27C512 (bios)

Video PCB (it's a standard VGA EISA board):
1x maskrom (28pin) (VGAbios)(not dumped)

Sound PCB (missing, it's a standard ISA 16bit sound card):

ROMs PCB (it's a custom PCB, with EISA connector to motherboard on one side and JAMMA connetcor on the other side):
5x AM27C040 (u11,u12,u31,u32,u33)
1x TMS27C040 (u13)(probably corrupted)
1x INTEL27C010A (u39)
4x PALCE16V8H (u5,u25,u26,u28)(not dumped yet)
1x PALCE20V8H (u42)
1x PALCE20V8H (u44)(not dumped yet)
2x PALCE22V10H (u45,u49)(not dumped yet)


Notes:----------

Main PCB (it's a standard 486 motherboard):
1x Keyboard DIN connector (not used)

Video PCB (it's a standard VGA EISA board):
1x VGA connetctor (to ROMs PCB)
1x red/black cable (to ROMs PCB)

Sound PCB (missing, it's a standard ISA 16bit sound card):
1x stereo audio out (to ROMs PCB)

ROMs PCB (it's a custom PCB, with EISA connector to motherboard on one side and JAMMA connetcor on the other side):
1x EISA connector (into motherboard)
1x JAMMA edge connector
1x VGA in connector (from Video PCB)
1x stereo audio jack (from sound card)
1x 13x2 legs connector(only 2 legs used for red/black cable from Video PCB)
1x trimmer (volume)
2x 8 switches dip

Info:----------

------------------------------------------
I have:
3 Main PCBs (all of them without 80486)
11 Video PCBs
4 ROMs PCBs (only one has EPROMs)
------------------------------------------

Game was programmed by Giovanni Tummarello and Roberto Molinelli in 1990 for Amiga;
it was an action/strategy videogame published by Proxxima Software (Rome, Italy) and
later ported to PC machines and published in 1994 by AIM Games software (US) and in
Arcade Version (Coin-Op) by InfoCube (Pisa, Italy)

*/

#include "driver.h"
#include "cpu/i386/i386.h"

static VIDEO_START(pangofun)
{
}

static VIDEO_UPDATE(pangofun)
{
	return 0;
}

static ADDRESS_MAP_START( pangofun_map, ADDRESS_SPACE_PROGRAM, 32 )
	AM_RANGE(0x00000000, 0x0001ffff) AM_ROM
ADDRESS_MAP_END


static INPUT_PORTS_START( pangofun )
INPUT_PORTS_END


static MACHINE_DRIVER_START( pangofun )
	/* basic machine hardware */
	MDRV_CPU_ADD("maincpu", I486, 14318180*2)	/* I486 ?? Mhz */
	MDRV_CPU_PROGRAM_MAP(pangofun_map)

	/* video hardware */
	MDRV_SCREEN_ADD("screen", RASTER)
	MDRV_SCREEN_FORMAT(BITMAP_FORMAT_INDEXED16)
	MDRV_SCREEN_REFRESH_RATE(60)
	MDRV_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(0))
	MDRV_SCREEN_SIZE(64*8, 32*8)
	MDRV_SCREEN_VISIBLE_AREA(0*8, 64*8-1, 0*8, 32*8-1)

	MDRV_PALETTE_LENGTH(0x100)

	MDRV_VIDEO_START(pangofun)
	MDRV_VIDEO_UPDATE(pangofun)
MACHINE_DRIVER_END


ROM_START(pangofun)
	ROM_REGION32_LE(0x20000, "maincpu", 0)	/* motherboard bios */
	ROM_LOAD("bios.bin", 0x000000, 0x10000, CRC(e70168ff) SHA1(4a0d985c218209b7db2b2d33f606068aae539020) )

	ROM_REGION32_LE(0x20000, "user1", 0)	/* gfx card bios */
	ROM_LOAD("vgabios.bin", 0x000000, 0x20000, NO_DUMP ) // 1x maskrom (28pin)

	/* this is what was on the rom board, mapping unknown */
	ROM_REGION32_LE(0xa00000, "user2", 0)	/* rom board */
	ROM_LOAD32_WORD("bank0.u11", 0x000000, 0x80000, CRC(6ce951d7) SHA1(1dd09491c651920a8a507bdc6584400367e5a292) )
	ROM_LOAD32_WORD("bank0.u31", 0x000002, 0x80000, CRC(b6c06baf) SHA1(79074b086d24737d629272d98f17de6e1e650485) )
	ROM_LOAD32_WORD("bank1.u12", 0x100000, 0x80000, CRC(5adc1f2e) SHA1(17abde7a2836d042a698661339eefe242dd9af0d) )
	ROM_LOAD32_WORD("bank1.u32", 0x100002, 0x80000, CRC(5647cbf6) SHA1(2e53a74b5939b297fa1a77441017cadc8a19ddef) )
	ROM_LOAD32_WORD("bank2.u13", 0x200000, 0x80000, BAD_DUMP CRC(504bf849) SHA1(13a184ec9e176371808938015111f8918cb4df7d) ) // EMPTY! (BAD?)
	ROM_LOAD32_WORD("bank2.u33", 0x200002, 0x80000, CRC(272ecfb6) SHA1(6e1b6bdef62d953de102784ba0148fb20182fa87) )
	               /*bank3.u14 , NOT POPULATED */
				   /*bank3.u34 , NOT POPULATED */
				   /*bank4.u15 , NOT POPULATED */
				   /*bank4.u35 , NOT POPULATED */
				   /*bank5.u16 , NOT POPULATED */
				   /*bank5.u36 , NOT POPULATED */
				   /*bank6.u17 , NOT POPULATED */
				   /*bank6.u37 , NOT POPULATED */
				   /*bank7.u18 , NOT POPULATED */
				   /*bank7.u37 , NOT POPULATED */
				   /*bank8.u19 , NOT POPULATED */
	ROM_LOAD32_WORD("bank8.u39", 0x900002, 0x20000, CRC(72422c66) SHA1(40b8cca3f99925cf019053921165f6a4a30d784d) )
ROM_END


GAME( 1995, pangofun,  0,   pangofun, pangofun, 0, ROT0, "InfoCube", "Pango Fun (Italy)", GAME_NOT_WORKING|GAME_NO_SOUND )
