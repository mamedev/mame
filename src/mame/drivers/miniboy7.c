/******************************************************************************


    MINI BOY 7

    Driver by Roberto Fresca.


    Games running on this hardware:

    - Mini Boy 7.    1983, Bonanza Enterprises, Ltd.


*******************************************************************************


    Preliminary Notes:

    This driver was made reverse-engineering the program ROMs.
    The Mini Boy 7 dump found lacks of PCB pics, technical notes or hardware list.
    Only one text file inside telling that ROMs mb7511, mb7311 and mb7111 are rotten,
    typical for M5L2764K parts. The color PROM was not dumped.


    Game Notes:

    Mini Boy 7. Seven games in one, plus Ad message support.
    http://www.arcadeflyers.com/?page=thumbs&db=videodb&id=4275

    - Draw Poker.
    - 7-Stud Poker.
    - Black Jack.
    - Baccarat.
    - Hi-Lo.
    - Double-Up.
    - Craps.


*******************************************************************************


    Hardware Notes:

    - CPU:            1x M6502.
    - Video:          1x MC6845.
    - RAM:            (unknown).
    - I/O             At least 1x 6821 PIA.
    - prg ROMs:       6x 2764 (8Kb) or similar.
    - gfx ROMs:       1x 2732 (4Kb) or similar for text layer.
                      4x M5L2764K (8Kb) for gfx tiles.
    - sound:          (unknown)
    - battery backup: (unknown)


    Still guessing...


*******************************************************************************


    --------------------
    ***  Memory Map  ***
    --------------------

    $0000 - $00FF   RAM     ; Zero Page (pointers and registers)
    $0100 - $01FF   RAM     ; 6502 Stack Pointer.
    $0200 - $07FF   RAM     ; R/W. (settings)

    $0800 - $0FFF   Video RAM
    $1000 - $17FF   Color RAM

    $2800 - $2801   MC6845  ; MC6845 use $2800 for register addressing and $2801 for register values.

    $3000 - $3001   ?????   ; R/W. AY8910?
    $3080 - $3083   ?????   ; R/W. PIA?
    $3800 - $3800   ?????   ; R.

    $4000 - $FFFF   ROM     ; ROM space.


    *** mc6845 init ***
    register:   00    01    02    03    04    05    06    07    08    09    10    11    12    13    14    15    16    17
    value:     0x2F  0x25  0x28  0x44  0x27  0x06  0x25  0x25  0x00  0x07  0x00  0x00  0x00  0x00  0x00  0x00  0x00  0x00.


*******************************************************************************


    DRIVER UPDATES:


    [2007-06-19]

    - Initial release. Just a skeleton driver.


    [2007-06-20]

    - Confirmed the CPU as 6502.
    - Confirmed the CRT controller as 6845.
    - Corrected the total & visible area analyzing the 6845 registers.
    - Crystal documented via #define.
    - CPU clock derived from #defined crystal value.
    - Decoded all gfx properly.
    - Partially worked the GFX banks:
        - 2 bank (1bpp) for text layers and minor graphics.
        - 1 bank (3bpp) for cards, jokers, dices and big text graphics.


    TODO:

    - Inputs.
    - DIP Switches.
    - NVRAM support if applicable.
    - Support for bottom scroll (big user message).
    - Figure out the colors (need a color PROM dump).
    - Figure out the sound.
    - Final cleanup and split the driver.


*******************************************************************************/


#define MASTER_CLOCK	10000000	/* 10MHz */

#include "driver.h"
#include "cpu/m6502/m6502.h"
#include "video/mc6845.h"


/*************************
*     Video Hardware     *
*************************/

static tilemap *bg_tilemap;

static WRITE8_HANDLER( miniboy7_videoram_w )
{
	videoram[offset] = data;
	tilemap_mark_tile_dirty(bg_tilemap, offset);
}

static WRITE8_HANDLER( miniboy7_colorram_w )
{
	colorram[offset] = data;
	tilemap_mark_tile_dirty(bg_tilemap, offset);
}

static TILE_GET_INFO( get_bg_tile_info )
{
/*  - bits -
    7654 3210
    --xx xx--   tiles color?.
    ---- --x-   tiles bank.
    xx-- ---x   seems unused. */

	int attr = colorram[tile_index];
	int code = videoram[tile_index];
	int bank = (attr & 0x02) >> 1;	/* bit 1 switch the gfx banks */
	int color = (attr & 0x3c);	/* bits 2-3-4-5 for color? */

	if (bank == 1)	/* temporary hack to point to the 3rd gfx bank */
		bank = 2;

	SET_TILE_INFO(bank, code, color, 0);
}

static VIDEO_START( miniboy7 )
{
	bg_tilemap = tilemap_create(machine, get_bg_tile_info, tilemap_scan_rows, 8, 8, 37, 37);
}

static VIDEO_UPDATE( miniboy7 )
{
	tilemap_draw(bitmap, cliprect, bg_tilemap, 0, 0);
	return 0;
}


/*************************
* Memory Map Information *
*************************/

static ADDRESS_MAP_START( miniboy7_map, ADDRESS_SPACE_PROGRAM, 8 )
	AM_RANGE(0x0000, 0x07ff) AM_RAM	/* battery backed RAM? */
	AM_RANGE(0x0800, 0x0fff) AM_RAM_WRITE(miniboy7_videoram_w) AM_BASE(&videoram)
	AM_RANGE(0x1000, 0x17ff) AM_RAM_WRITE(miniboy7_colorram_w) AM_BASE(&colorram)
	AM_RANGE(0x1800, 0x25ff) AM_RAM	/* looks like videoram */
	AM_RANGE(0x2600, 0x27ff) AM_RAM
	AM_RANGE(0x2800, 0x2800) AM_DEVWRITE("crtc", mc6845_address_w)
	AM_RANGE(0x2801, 0x2801) AM_DEVREADWRITE("crtc", mc6845_register_r, mc6845_register_w)
//  AM_RANGE(0x3000, 0x3001) ????? R/W
//  AM_RANGE(0x3080, 0x3083) AM_READWRITE(pia_0_r, pia_0_w)
//  AM_RANGE(0x3800, 0x3800) ????? R
	AM_RANGE(0x4000, 0xffff) AM_ROM
ADDRESS_MAP_END


/*************************
*      Input Ports       *
*************************/

static INPUT_PORTS_START( miniboy7 )
INPUT_PORTS_END


/*************************
*    Graphics Layouts    *
*************************/

static const gfx_layout charlayout =
{
	8, 8,
	256,
	1,
	{ 0 },
	{ 0, 1, 2, 3, 4, 5, 6, 7 },
	{ 0*8, 1*8, 2*8, 3*8, 4*8, 5*8, 6*8, 7*8 },
	8*8
};

static const gfx_layout tilelayout =
{
	8, 8,
	RGN_FRAC(1,3),
	3,
	{ 0, RGN_FRAC(1,3), RGN_FRAC(2,3) },    /* bitplanes are separated */
	{ 0, 1, 2, 3, 4, 5, 6, 7 },
	{ 0*8, 1*8, 2*8, 3*8, 4*8, 5*8, 6*8, 7*8 },
	8*8
};


/******************************
* Graphics Decode Information *
******************************/

static GFXDECODE_START( miniboy7 )
	GFXDECODE_ENTRY( "gfx1", 0x0800,	charlayout, 0, 16 )	/* text layer 1 */
	GFXDECODE_ENTRY( "gfx1", 0x0000,	charlayout, 0, 16 )	/* text layer 2 */

    /* 0x000 cards
       0x100 joker
       0x200 dices
       0x300 bigtxt */
	GFXDECODE_ENTRY( "gfx2", 0,	tilelayout, 0, 16 )

GFXDECODE_END


/************************
*    CRTC Interface    *
************************/

static const mc6845_interface mc6845_intf =
{
	"screen",	/* screen we are acting on */
	8,			/* number of pixels per video memory address */
	NULL,		/* before pixel update callback */
	NULL,		/* row update callback */
	NULL,		/* after pixel update callback */
	DEVCB_NULL,	/* callback for display state changes */
	DEVCB_NULL,	/* callback for cursor state changes */
	DEVCB_NULL,	/* HSYNC callback */
	DEVCB_NULL,	/* VSYNC callback */
	NULL		/* update address callback */
};


/*************************
*    Machine Drivers     *
*************************/

static MACHINE_DRIVER_START( miniboy7 )

	/* basic machine hardware */
	MDRV_CPU_ADD("maincpu", M6502, MASTER_CLOCK/16)	/* guess */
	MDRV_CPU_PROGRAM_MAP(miniboy7_map)
	MDRV_CPU_VBLANK_INT("screen", nmi_line_pulse)

	/* video hardware */
	MDRV_SCREEN_ADD("screen", RASTER)
	MDRV_SCREEN_REFRESH_RATE(60)
	MDRV_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(0))
	MDRV_SCREEN_FORMAT(BITMAP_FORMAT_INDEXED16)
	MDRV_SCREEN_SIZE((47+1)*8, (39+1)*8)                  /* Taken from MC6845, registers 00 & 04. Normally programmed with (value-1) */
	MDRV_SCREEN_VISIBLE_AREA(0*8, 37*8-1, 0*8, 37*8-1)    /* Taken from MC6845, registers 01 & 06 */

	MDRV_GFXDECODE(miniboy7)

	MDRV_PALETTE_LENGTH(256)

	MDRV_VIDEO_START(miniboy7)
	MDRV_VIDEO_UPDATE(miniboy7)

	MDRV_MC6845_ADD("crtc", MC6845, MASTER_CLOCK/11, mc6845_intf) /* guess */
MACHINE_DRIVER_END


/*************************
*        Rom Load        *
*************************/

ROM_START( miniboy7 )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "mb7111.8a",	0x4000, 0x2000,  BAD_DUMP CRC(1b7ac5f0) SHA1(a52052771fcce688afccf9f0c3e3c2b5e7cec4e4) )    /* marked as BAD for the dumper but seems OK */
	ROM_LOAD( "mb7211.7a",	0x6000, 0x2000, CRC(ac9b66a6) SHA1(66a33e475de4fb3ffdd9a68a24932574e7d78116) )
	ROM_LOAD( "mb7311.6a",	0x8000, 0x2000,  BAD_DUMP CRC(99f2a063) SHA1(94108cdc574c7e9400fe8a249b78ba190d10502b) )    /* marked as BAD for the dumper */
	ROM_LOAD( "mb7411.5a",	0xa000, 0x2000, CRC(99f8268f) SHA1(a4ca98dfb5df86fe45f33e291bf0c40d1f43ae7c) )
	ROM_LOAD( "mb7511.4a",	0xc000, 0x2000,  BAD_DUMP CRC(2820ae91) SHA1(70f9b3823733ae39d153948a4006a5972204f482) )    /* marked as BAD for the dumper */
	ROM_LOAD( "mb7611.3a",	0xe000, 0x2000, CRC(ca9b9b20) SHA1(c6cd793a15948601faa051a4643b14fd3d8bda0b) )

	ROM_REGION( 0x1000, "gfx1", 0 )
	ROM_LOAD( "mb70.11d",	0x0000, 0x1000, CRC(84f78ee2) SHA1(c434e8a9b19ef1394b1dac67455f859eef299f95) )    /* text layer */

	ROM_REGION( 0x6000, "gfx2", 0 )
	ROM_LOAD( "mb71.12d",	0x0000, 0x2000, CRC(5f3e3b93) SHA1(41ab6a42a41ddeb8b6b76f4d790bf9fb9e7c32a3) )
	ROM_LOAD( "mb72.13d",	0x2000, 0x2000, CRC(b3362650) SHA1(603907fd3a0049c0a3e1858c4329bf9fd58137f6) )
	ROM_LOAD( "mb73.14d",	0x4000, 0x2000, CRC(10c2bf71) SHA1(23a01625b0fc0b772054ee4bc026d2257df46a03) )

	ROM_REGION( 0x0100, "proms", 0 )
	ROM_LOAD( "mb7_24s10n.bin",	0x0000, 0x0100, NO_DUMP) /* PROM dump needed */
ROM_END


/*************************
*      Game Drivers      *
*************************/

/*    YEAR  NAME      PARENT  MACHINE   INPUT     INIT   ROT    COMPANY                     FULLNAME     FLAGS  */
GAME( 1983, miniboy7, 0,      miniboy7, miniboy7, 0,     ROT0, "Bonanza Enterprises, Ltd", "Mini Boy 7", GAME_NO_SOUND | GAME_WRONG_COLORS | GAME_NOT_WORKING )

