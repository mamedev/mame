/**********************************************************************************


    SNOOKER 10 / SANDII'

    Driver by Roberto Fresca.


    Games running on this hardware:

    * Snooker 10 (Ver 1.11), Sandii', 1998.
    * Apple 10 (Ver 1.21),   Sandii', 1998.
    * Ten Balls (Ver 1.05),  unknown, 1997.


***********************************************************************************


    The hardware is generally composed by:


    CPU:    1x 65SC02 at 2MHz.

    Sound:  1x OKI6295
            1x LM358N
            1x TDA2003

    HD-PLD: 2x AMD MACH231-15-JC/1-18JI/1
            (2x Lattice ispLSI1024-60LJ for earlier revisions)
 
    RAM:    1x 76C88AL-15, SRAM 8Kx8 
    NVRAM:  1x 76C88AL-15, SRAM 8Kx8 (battery backed)
    ROMs:   4x 27C256
            (3x 27C256 for earlier revisions)
            1x 27C020

    PROMs:  1x 82S147 or similar. (512 bytes)

    Clock:  1x Crystal: 16MHz.

    Other:  1x 28x2 edge connector.
            1x 15 legs connector.
            1x trimmer (volume).
            1x 8 DIP switches.
            1x 3.5 Volt, 55-80 mAh battery.


***************************************************************************************


    All the supported games have been coded using some Funworld games as point to start,
    changing hardware accesses, program logics, graphics, plus protection and some I/O
    through the 2x high density PLDs.


    Color palettes are normally stored in format RRRBBBGG inside a bipolar color PROM
    (old hardware), or repeated 64 times inside a regular 27c256 ROM (new hardware).

    - bits -
    7654 3210
    ---- -xxx   Red component.
    --xx x---   Blue component.
    xx-- ----   Green component.


    Same as Funworld video hardware, this one was designed to manage 4096 tiles with a
    size of 8x4 pixels each. Also support 4bpp graphics and the palette limitation is
    8 bits for color codes (256 x 16 colors). It means the hardware was designed for more
    elaborated graphics than these games...

    Color PROMs from current games are 512 bytes lenght, but they only use the first 256 bytes.

    Normal hardware capabilities:

    - bits -
    7654 3210
    xxxx xx--   tiles color (game tiles)    ;codes 0x00-0xdc
    xxx- x-xx   tiles color (title).        :codes 0xe9-0xeb
    xxxx -xxx   tiles color (background).   ;codes 0xf1-0xf7


    --- Issues / Protection ---

    * Apple 10

    - Tiles and color palette are totally scrambled.



***********************************************************************************


    Memory Map
    ----------


    $0000 - $0FFF   NVRAM       ;All registers and settings.

    $1000 - $1000   ???         ;R/W
    $3000 - $3004   Input Ports ;Reads
    $5000 - $5001   ???         ;Writes

    $6000 - $6FFF   VideoRAM
    $7000 - $7FFF   ColorRAM

    $8000 - $FFFF   ROM Space



***********************************************************************************


    *** Driver Updates ***


    [2008/04/18]
    - Initial release. Support for snookr10, apple10 and tenballs.
    - Added technical/general notes.


    *** TO DO ***

    - Inputs.
    - Figure out the high-density PLDs, or get a dump of them.
    - Figure out the sound.


***********************************************************************************/


#define MASTER_CLOCK	XTAL_16MHz

#include "driver.h"


/* from video */
WRITE8_HANDLER( snookr10_videoram_w );
WRITE8_HANDLER( snookr10_colorram_w );
PALETTE_INIT( snookr10 );
VIDEO_START( snookr10 );
VIDEO_UPDATE( snookr10 );


/*************************
* Memory map information *
*************************/

static ADDRESS_MAP_START( snookr10_map, ADDRESS_SPACE_PROGRAM, 8 )
	AM_RANGE(0x0000, 0x0fff) AM_RAM AM_BASE(&generic_nvram) AM_SIZE(&generic_nvram_size)
//	AM_RANGE(0x1000, 0x1000) AM_READNOP	/* R/W */
//	AM_RANGE(0x3000, 0x3004) AM_READNOP	/* reads (input port) */
//	AM_RANGE(0x5000, 0x5001) AM_READNOP	/* writes */
	AM_RANGE(0x6000, 0x6fff) AM_RAM_WRITE(snookr10_videoram_w) AM_BASE(&videoram)
	AM_RANGE(0x7000, 0x7fff) AM_RAM	AM_WRITE(snookr10_colorram_w) AM_BASE(&colorram)
	AM_RANGE(0x8000, 0xffff) AM_ROM
ADDRESS_MAP_END


/*************************
*      Input ports       *
*************************/

static INPUT_PORTS_START( snookr10 )
INPUT_PORTS_END


/*************************
*    Graphics Layouts    *
*************************/

static const gfx_layout charlayout =
{
	4,8,
	RGN_FRAC(1,2),
	4,
	{ RGN_FRAC(0,2), RGN_FRAC(0,2) + 4, RGN_FRAC(1,2), RGN_FRAC(1,2) + 4 },
	{ 0, 1, 2, 3 },
	{ 0*8, 1*8, 2*8, 3*8, 4*8, 5*8, 6*8, 7*8 },
	8*4*2
};


/******************************
* Graphics Decode Information *
******************************/

static GFXDECODE_START( snookr10 )
	GFXDECODE_ENTRY( REGION_GFX1, 0x0000, charlayout, 0, 16 )
GFXDECODE_END


/**************************
*     Machine Drivers     *
**************************/

static MACHINE_DRIVER_START( snookr10 )
    /* basic machine hardware */
	MDRV_CPU_ADD_TAG("main", M65SC02, MASTER_CLOCK/8)	/* 2MHz */
	MDRV_CPU_PROGRAM_MAP(snookr10_map, 0)
	MDRV_CPU_VBLANK_INT("main", nmi_line_pulse)

	MDRV_NVRAM_HANDLER(generic_0fill)

    /* video hardware */

	MDRV_SCREEN_ADD("main", RASTER)
	MDRV_SCREEN_REFRESH_RATE(60)
	MDRV_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(0))
	MDRV_SCREEN_FORMAT(BITMAP_FORMAT_INDEXED16)
	MDRV_SCREEN_SIZE((128+1)*4, (30+1)*8)
	MDRV_SCREEN_VISIBLE_AREA(0*4, 96*4-1, 0*8, 30*8-1)

	MDRV_GFXDECODE(snookr10)

	MDRV_PALETTE_LENGTH(0x200)
	MDRV_PALETTE_INIT(snookr10)
	MDRV_VIDEO_START(snookr10)
	MDRV_VIDEO_UPDATE(snookr10)

MACHINE_DRIVER_END


/*************************
*        Rom Load        *
*************************/

ROM_START( snookr10 )
	ROM_REGION( 0x10000, REGION_CPU1, 0 )
	ROM_LOAD( "1.u2", 0x8000, 0x8000, CRC(216ccb2d) SHA1(d86270cd03a08f6fd3e7b327b8173f66da28e5e8) )

    /* the first 256 bytes looks as a samples table */
	ROM_REGION( 0x40000, REGION_CPU2, 0 )
	ROM_LOAD( "4.u18", 0x00000, 0x40000 , CRC(17090d56) SHA1(3a4c247f96c80f8cf4c1389b273880c5ea6fc39d) )

	ROM_REGION( 0x10000, REGION_GFX1, ROMREGION_DISPOSE )
	ROM_LOAD( "2.u22", 0x0000, 0x8000, CRC(a70d9c48) SHA1(3fa90190323526553866662afda4dbe1c94abeff) )
	ROM_LOAD( "3.u25", 0x8000, 0x8000, CRC(3009faaa) SHA1(d1cda455b270cb9afa65b9701735a3a1f2a48df2) )

    /* this should be changed because the palette is stored in a normal ROM instead of a color PROM */
	ROM_REGION( 0x8000, REGION_PROMS, 0 )
	ROM_LOAD( "5.u27", 0x0000, 0x8000, CRC(f3d7d640) SHA1(f78060f4603e316fa3c2ec4ba6d7edf261cf6d8a) )
ROM_END

ROM_START( apple10 )
	ROM_REGION( 0x10000, REGION_CPU1, 0 )
	ROM_LOAD( "1.u2", 0x8000, 0x8000, CRC(7d538566) SHA1(2e805157010c366ab1f2313a2bedb071c1dde733) )

    /* the first 256 bytes looks as a samples table */
	ROM_REGION( 0x40000, REGION_CPU2, 0 )
	ROM_LOAD( "4.u18", 0x00000, 0x40000 , CRC(17090d56) SHA1(3a4c247f96c80f8cf4c1389b273880c5ea6fc39d) )

	ROM_REGION( 0x10000, REGION_GFX1, ROMREGION_DISPOSE )
	ROM_LOAD( "2.u22", 0x0000, 0x8000, CRC(42b016f4) SHA1(59d1b77f8cb706a3878813111c6a71514c413784) )
	ROM_LOAD( "3.u25", 0x8000, 0x8000, CRC(afc535dc) SHA1(ed2d65f3154c6d80b7b22bfef1f30232e4496128) )

    /* this should be changed because the palette is stored in a normal ROM instead of a color PROM */
	ROM_REGION( 0x8000, REGION_PROMS, 0 )
	ROM_LOAD( "5.u27", 0x0000, 0x8000, CRC(3510d705) SHA1(2190c8199d29bf89e3007eb771cc6b0e2b58f6cd) )
ROM_END

ROM_START( tenballs )
	ROM_REGION( 0x10000, REGION_CPU1, 0 )
	ROM_LOAD( "4.u2", 0x8000, 0x8000, CRC(2f334862) SHA1(61d57995451b6bc7de23900c460c3e073993899c) )

    /* the first 256 bytes looks as a samples table */
	ROM_REGION( 0x40000, REGION_CPU2, 0 )
	ROM_LOAD( "1.u28", 0x00000, 0x40000 , CRC(17090d56) SHA1(3a4c247f96c80f8cf4c1389b273880c5ea6fc39d) )

	ROM_REGION( 0x10000, REGION_GFX1, ROMREGION_DISPOSE )
	ROM_LOAD( "3.u16", 0x0000, 0x8000, CRC(9eb88a08) SHA1(ab52924103e2b14c598a21c3d77b053da37a0212) )
	ROM_LOAD( "2.u15", 0x8000, 0x8000, CRC(a5091583) SHA1(c0775d9b77cb634d3702b6c08cdf73c867b6169a) )

	ROM_REGION( 0x0200, REGION_PROMS, 0 )
	ROM_LOAD( "82s147.u17", 0x0000, 0x0200, CRC(20234dcc) SHA1(197937bbec0201888467e250bdba49e39aa4204a) )
ROM_END


/*************************
*      Game Drivers      *
*************************/

/*    YEAR  NAME      PARENT  MACHINE   INPUT     INIT      ROT    COMPANY     FULLNAME                FLAGS  */
GAME( 1998, snookr10, 0,      snookr10, snookr10, 0,        ROT0, "Sandii'",  "Snooker 10 (Ver 1.11)", GAME_NO_SOUND | GAME_UNEMULATED_PROTECTION | GAME_NOT_WORKING )
GAME( 1998, apple10,  0,      snookr10, snookr10, 0,        ROT0, "Sandii'",  "Apple 10 (Ver 1.21)",   GAME_NO_SOUND | GAME_UNEMULATED_PROTECTION | GAME_IMPERFECT_GRAPHICS | GAME_WRONG_COLORS | GAME_NOT_WORKING )
GAME( 1997, tenballs, 0,      snookr10, snookr10, 0,        ROT0, "unknown",  "Ten Balls (Ver 1.05)",  GAME_NO_SOUND | GAME_UNEMULATED_PROTECTION | GAME_NOT_WORKING )
