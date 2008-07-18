/************************************************************************

  Italian Gambling games based on Mitsubishi (Renesas) M16C MCU family.

  Written by Roberto Fresca.


  All these games use MCUs with internal ROM for their programs.

  They have 256KB of internal flash ROM that can't be dumped easily,
  and thus we can't emulate them at the moment because there is
  nothing to emulate.

  This driver is just a placeholder for the graphics/sound ROM loading


*************************************************************************

  --- Hardware Notes ---

  The hardware is normally composed by:


  CPU:   1x M30624FGAFP.
           (256KB ROM; 20KB RAM)

  Sound: 1x OKI M6295.
         1x TDA2003 (audio amplifier).
         1x LM358M (audio amplifier).

  PLDs:  2x ispLSI1032E-70JL.

  Clock: 1x Xtal 16.000 MHz.

  ROMs:  1x (up to) 27C4001 or similar (sound).
         4x 27C4001 or similar (graphics).


************************************************************************/

#define MAIN_CLOCK	XTAL_16MHz

#include "driver.h"
#include "cpu/h83002/h83002.h"
#include "sound/okim6295.h"


/*************************
*     Video Hardware     *
*************************/

VIDEO_START( itgambl3 )
{
}


VIDEO_UPDATE( itgambl3 )
{
	return 0;
}


/*************************
* Memory map information *
*************************/

static ADDRESS_MAP_START( itgambl3_map, ADDRESS_SPACE_PROGRAM, 16 )
	ADDRESS_MAP_GLOBAL_MASK(0xffffff)
	AM_RANGE(0x000000, 0xffffff) AM_ROM
ADDRESS_MAP_END


/*************************
*      Input ports       *
*************************/

static INPUT_PORTS_START( itgambl3 )
    PORT_START
    PORT_DIPNAME( 0x0001, 0x0001, DEF_STR( Unknown ) )
    PORT_DIPSETTING(      0x0001, DEF_STR( Off ) )
    PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
    PORT_DIPNAME( 0x0002, 0x0002, DEF_STR( Unknown ) )
    PORT_DIPSETTING(      0x0002, DEF_STR( Off ) )
    PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
    PORT_DIPNAME( 0x0004, 0x0004, DEF_STR( Unknown ) )
    PORT_DIPSETTING(      0x0004, DEF_STR( Off ) )
    PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
    PORT_DIPNAME( 0x0008, 0x0008, DEF_STR( Unknown ) )
    PORT_DIPSETTING(      0x0008, DEF_STR( Off ) )
    PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
    PORT_DIPNAME( 0x0010, 0x0010, DEF_STR( Unknown ) )
    PORT_DIPSETTING(      0x0010, DEF_STR( Off ) )
    PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
    PORT_DIPNAME( 0x0020, 0x0020, DEF_STR( Unknown ) )
    PORT_DIPSETTING(      0x0020, DEF_STR( Off ) )
    PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
    PORT_DIPNAME( 0x0040, 0x0040, DEF_STR( Unknown ) )
    PORT_DIPSETTING(      0x0040, DEF_STR( Off ) )
    PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
    PORT_DIPNAME( 0x0080, 0x0080, DEF_STR( Unknown ) )
    PORT_DIPSETTING(      0x0080, DEF_STR( Off ) )
    PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
    PORT_DIPNAME( 0x0100, 0x0100, DEF_STR( Unknown ) )
    PORT_DIPSETTING(      0x0100, DEF_STR( Off ) )
    PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
    PORT_DIPNAME( 0x0200, 0x0200, DEF_STR( Unknown ) )
    PORT_DIPSETTING(      0x0200, DEF_STR( Off ) )
    PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
    PORT_DIPNAME( 0x0400, 0x0400, DEF_STR( Unknown ) )
    PORT_DIPSETTING(      0x0400, DEF_STR( Off ) )
    PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
    PORT_DIPNAME( 0x0800, 0x0800, DEF_STR( Unknown ) )
    PORT_DIPSETTING(      0x0800, DEF_STR( Off ) )
    PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
    PORT_DIPNAME( 0x1000, 0x1000, DEF_STR( Unknown ) )
    PORT_DIPSETTING(      0x1000, DEF_STR( Off ) )
    PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
    PORT_DIPNAME( 0x2000, 0x2000, DEF_STR( Unknown ) )
    PORT_DIPSETTING(      0x2000, DEF_STR( Off ) )
    PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
    PORT_DIPNAME( 0x4000, 0x4000, DEF_STR( Unknown ) )
    PORT_DIPSETTING(      0x4000, DEF_STR( Off ) )
    PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
    PORT_DIPNAME( 0x8000, 0x8000, DEF_STR( Unknown ) )
    PORT_DIPSETTING(      0x8000, DEF_STR( Off ) )
    PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
INPUT_PORTS_END


/*************************
*    Graphics Layouts    *
*************************/

static const gfx_layout gfxlayout_8x8x8 =
{
/* this is wrong and need to be fixed */

	8,8,
	RGN_FRAC(1,1),
	8,
	{ 0, 1, 2, 3, 4, 5, 6, 7 },
	{ 0*8, 1*8, 2*8, 3*8, 4*8, 5*8, 6*8, 7*8 },
	{ 0*64, 1*64, 2*64, 3*64, 4*64, 5*64, 6*64, 7*64 },
	8*64
};


/******************************
* Graphics Decode Information *
******************************/

static GFXDECODE_START( itgambl3 )
    GFXDECODE_ENTRY( REGION_GFX1, 0, gfxlayout_8x8x8,   0, 16  )
GFXDECODE_END


/**************************
*      Machine Reset      *
**************************/

MACHINE_RESET( itgambl3 )
{
	/* stop the CPU, we have no code for it anyway */
	cpunum_set_input_line(machine, 0, INPUT_LINE_HALT, ASSERT_LINE);
}


/**************************
*     Machine Drivers     *
**************************/

static MACHINE_DRIVER_START( itgambl3 )

    /* basic machine hardware */
	MDRV_CPU_ADD_TAG("main", H83044, MAIN_CLOCK)	/* wrong CPU, but we have not a M16C core ATM */
	MDRV_CPU_PROGRAM_MAP(itgambl3_map,0)

    /* video hardware */
	MDRV_SCREEN_ADD("main", RASTER)
	MDRV_SCREEN_REFRESH_RATE(60)
	MDRV_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(0))
	MDRV_SCREEN_FORMAT(BITMAP_FORMAT_INDEXED16)
	MDRV_SCREEN_SIZE(512, 256)
	MDRV_SCREEN_VISIBLE_AREA(0, 512-1, 0, 256-1)
	MDRV_MACHINE_RESET( itgambl3 )

	MDRV_GFXDECODE(itgambl3)
	MDRV_PALETTE_LENGTH(0x200)
	MDRV_VIDEO_START( itgambl3 )
	MDRV_VIDEO_UPDATE( itgambl3 )

    /* sound hardware */
	MDRV_SPEAKER_STANDARD_MONO("mono")
	MDRV_SOUND_ADD("oki", OKIM6295, MAIN_CLOCK/16)	/* 1MHz */
	MDRV_SOUND_CONFIG(okim6295_interface_region_1_pin7high)
	MDRV_SOUND_ROUTE(ALL_OUTPUTS, "mono", 1.0)
MACHINE_DRIVER_END


/*************************
*        Rom Load        *
*************************/

/* Euro Jolly X5

CPU:

1x M30624FGAFP-03001A4 (u1)(main)

This one is a Mitsubishi (Renesas) M16/62A 16bit microcomputer.
It has 256 KB of internal flash ROM + 20 KB of RAM.

1x OKI M6295 (u22)(sound)
1x TDA2003 (u25)(sound)
1x LM358M (u23)(sound)
1x oscillator 16.000MHz (u20)


ROMs:

4x M27C4001 (u21, u15, u16, u17)


PLDs:

1x ST93C46 (u18)
(1K 64 x 16 or 128 x 8 serial microwirw EEPROM)

2x ispLSI1032E-70LJ


Note:

1x JAMMA style edge connector
1x RS232 connector (P1) (along with an ST232C controller (u12)
1x 6 legs jumper (jp1)
1x 8 legs jumper (jp_1)
1x 7 legs jumper (jp2)
1x 4 legs jumper (jp3)
1x red led (d7)
1x battery (bt1)
2x trimmer (r6,r33) (volume)
1x pushbutton (s1)

*/

ROM_START( ejollyx5 )	/* CPU and clock should be changed for this game */
	ROM_REGION( 0x1000000, REGION_CPU1, 0 )	/* all the program code is in here */
	ROM_LOAD( "ejollyx5_m30624fgafp.mcu", 0x00000, 0x4000, NO_DUMP )

	ROM_REGION( 0x180000, REGION_GFX1, 0 )	/* different encoded gfx */
	ROM_LOAD( "eurojolly5-ep01.u15", 0x000000, 0x80000, CRC(feb4ef88) SHA1(5a86e92326096e4e0619a8aa6b491553eb46839d) )
	ROM_LOAD( "eurojolly5-ep02.u17", 0x080000, 0x80000, CRC(83b2dab0) SHA1(a65cae227a444fe7474f8f821dbb6a8b506e4ae6) )
	ROM_LOAD( "eurojolly5-ep03.u16", 0x100000, 0x80000, CRC(a0599d3c) SHA1(f52928cd75b4374a45fad37b7a7c1d39ea31b5f2) )

	ROM_REGION( 0x80000, REGION_SOUND1, 0 ) /* M6295 samples, identical halves */
	ROM_LOAD( "eurojolly5-msg0.u21", 0x00000, 0x80000, CRC(edc157bc) SHA1(8400251ca7a74a4a0f2d443ae2c0254f1de955ac) )
ROM_END


/* Grand Prix

CPU:

1x M30624FGAFP-251G108 (u21)(main)

This one is a Mitsubishi (Renesas) M16/62A 16bit microcomputer.
It has 256 KB of internal flash ROM + 20 KB of RAM.

1x OKI M6295 (u2)(sound)
1x TDA2003 (u1)(sound)
1x LM358M (u23)(sound)
1x oscillator 16.000MHz (u9)


ROMs:

1x AM27C040 (u3)
1x MX29F1610 (u22)


PLDs:

2x ispLSI1032E-70LJ-E011S03 (u10, U11)


Note:

1x JAMMA edge connector (jp9)
1x 4 legs connector (jp3)
1x 22x2 legs connector (jp5)
1x 7x2 legs connector (jp17)
1x 9 pins (serial?) connector (jp4)

1x battery
1x trimmer (volume) (rv1)
1x red led (d1)
1x pushbutton (test mode) (RDP2)

PCB N? KGS0243-DF070283/03 made in Italy

*/

ROM_START( grandprx )	/* CPU and clock should be changed for this game */
	ROM_REGION( 0x1000000, REGION_CPU1, 0 )	/* all the program code is in here */
	ROM_LOAD( "grandprx_m30624fgafp.mcu", 0x00000, 0x4000, NO_DUMP )

	ROM_REGION( 0x200000, REGION_GFX1, 0 )	/* different encoded gfx */
	ROM_LOAD( "u22.bin", 0x000000, 0x200000, CRC(e8ec804f) SHA1(84e647f693e0273b9b09d7726b814516496121a9) )

	ROM_REGION( 0x80000, REGION_SOUND1, 0 ) /* M6295 samples, identical halves */
	ROM_LOAD( "u3.bin", 0x00000, 0x80000, CRC(c9bb5858) SHA1(c154df7c7dfe394fc1963dc0c73f1d909f5b62ee) )
ROM_END


/*************************
*      Game Drivers      *
*************************/

/*    YEAR  NAME      PARENT  MACHINE   INPUT     INIT ROT    COMPANY        FULLNAME        FLAGS  */
GAME( 200?, ejollyx5, 0,      itgambl3, itgambl3, 0,   ROT0, "Solar Games", "Euro Jolly X5", GAME_NOT_WORKING )
GAME( 200?, grandprx, 0,      itgambl3, itgambl3, 0,   ROT0, "4fun",        "Grand Prix",    GAME_NOT_WORKING )
