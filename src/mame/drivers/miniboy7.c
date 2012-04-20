/******************************************************************************

  MINI BOY 7

  Driver by Roberto Fresca.


  Games running on this hardware:

  * Mini Boy 7 (set 1).    1983, Bonanza Enterprises, Ltd.
  * Mini Boy 7 (set 2).    1983, Bonanza Enterprises, Ltd.


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
  --------------

  Board silkscreened on top:
  be MVX-001-01  ('be' is a Bonanza Enterprises logo).

  - CPU:            1x R6502P.
  - Sound:          1x AY-3-8910.
  - Video:          1x HD46505 HD6845SP.
  - RAM:            (unknown).
  - I/O             1x MC6821 PIA.
  - PRG ROMs:       6x 2764 (8Kb).
  - GFX ROMs:       1x 2732 (4Kb) for text layer.
                    4x 2764 (8Kb) for gfx tiles.

  - Clock:          1x 12.4725 MHz. Crystal.

  - Battery backup: (unknown)

  - 1x normal switch (SW1)
  - 1x 8 DIP switches bank (SW2)
  - 1x 4 DIP switches bank (SW3)

  - 1x 2x28 pins edge connector.
  - 1x 2x20 pins female connector.


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


  [2010-07-30]

  - Added a new complete set. Now set as parent.
  - Corrected Xtal frequency.
  - Mapped the PIA MC6821 (not wired since is not totally understood).
  - Preliminary attempt to decode the color PROM.
  - Mapped the AY-3-8910, but still needs ports and some checks.
  - Added debug and technical notes.


  TODO:

  - Inputs.
  - DIP Switches.
  - NVRAM support.
  - Support for bottom scroll (big user message).
  - Figure out the colors.
  - Figure out the sound.
  - Final cleanup and split the driver.


*******************************************************************************/


#define MASTER_CLOCK	XTAL_12_4725MHz    /* 12.4725 MHz */

#include "emu.h"
#include "cpu/m6502/m6502.h"
#include "video/mc6845.h"
#include "machine/6821pia.h"
#include "sound/ay8910.h"
#include "machine/nvram.h"


class miniboy7_state : public driver_device
{
public:
	miniboy7_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag) ,
		m_videoram(*this, "videoram"),
		m_colorram(*this, "colorram"){ }

	required_shared_ptr<UINT8> m_videoram;
	required_shared_ptr<UINT8> m_colorram;
	tilemap_t *m_bg_tilemap;
	DECLARE_WRITE8_MEMBER(miniboy7_videoram_w);
	DECLARE_WRITE8_MEMBER(miniboy7_colorram_w);
};


/***********************************
*          Video Hardware          *
***********************************/

WRITE8_MEMBER(miniboy7_state::miniboy7_videoram_w)
{
	m_videoram[offset] = data;
	m_bg_tilemap->mark_tile_dirty(offset);
}

WRITE8_MEMBER(miniboy7_state::miniboy7_colorram_w)
{
	m_colorram[offset] = data;
	m_bg_tilemap->mark_tile_dirty(offset);
}

static TILE_GET_INFO( get_bg_tile_info )
{
	miniboy7_state *state = machine.driver_data<miniboy7_state>();
/*  - bits -
    7654 3210
    --xx xx--   tiles color?.
    ---- --x-   tiles bank.
    xx-- ---x   seems unused. */

	int attr = state->m_colorram[tile_index];
	int code = state->m_videoram[tile_index];
	int bank = (attr & 0x02) >> 1;	/* bit 1 switch the gfx banks */
	int color = (attr & 0x3c);	/* bits 2-3-4-5 for color? */

	if (bank == 1)	/* temporary hack to point to the 3rd gfx bank */
		bank = 2;

	SET_TILE_INFO(bank, code, color, 0);
}

static VIDEO_START( miniboy7 )
{
	miniboy7_state *state = machine.driver_data<miniboy7_state>();
	state->m_bg_tilemap = tilemap_create(machine, get_bg_tile_info, tilemap_scan_rows, 8, 8, 37, 37);
}

static SCREEN_UPDATE_IND16( miniboy7 )
{
	miniboy7_state *state = screen.machine().driver_data<miniboy7_state>();
	state->m_bg_tilemap->draw(bitmap, cliprect, 0, 0);
	return 0;
}

static PALETTE_INIT( miniboy7 )
{
	const UINT8 *color_prom = machine.root_device().memregion("proms")->base();
/*  FIXME... Can't get the correct palette.
    sometimes RGB bits are inverted, disregarding the 4th bit.

    prom bits
    7654 3210
    ---- ---x   red component?.
    ---- --x-   green component?.
    ---- -x--   blue component?.
    ---- x---   intensity?.
    xxxx ----   unused.
*/
	int i;

	/* 0000IBGR */
	if (color_prom == 0) return;

	for (i = 0;i < machine.total_colors();i++)
	{
		int bit0, bit1, bit2, r, g, b, inten, intenmin, intenmax;

		intenmin = 0xe0;
//      intenmin = 0xc2;
		intenmax = 0xff;

		/* intensity component */
		inten = (color_prom[i] >> 3) & 0x01;

		/* red component */
		bit0 = (color_prom[i] >> 0) & 0x01;
		r = (bit0 * intenmin) + (inten * (bit0 * (intenmax - intenmin)));

		/* green component */
		bit1 = (color_prom[i] >> 1) & 0x01;
		g = (bit1 * intenmin) + (inten * (bit1 * (intenmax - intenmin)));

		/* blue component */
		bit2 = (color_prom[i] >> 2) & 0x01;
		b = (bit2 * intenmin) + (inten * (bit2 * (intenmax - intenmin)));


		palette_set_color(machine, i, MAKE_RGB(r, g, b));
	}
}


/***********************************
*      Memory Map Information      *
***********************************/

static ADDRESS_MAP_START( miniboy7_map, AS_PROGRAM, 8, miniboy7_state )
	AM_RANGE(0x0000, 0x07ff) AM_RAM	AM_SHARE("nvram") /* battery backed RAM? */
	AM_RANGE(0x0800, 0x0fff) AM_RAM_WRITE(miniboy7_videoram_w) AM_SHARE("videoram")
	AM_RANGE(0x1000, 0x17ff) AM_RAM_WRITE(miniboy7_colorram_w) AM_SHARE("colorram")
	AM_RANGE(0x1800, 0x25ff) AM_RAM	/* looks like videoram */
	AM_RANGE(0x2600, 0x27ff) AM_RAM
	AM_RANGE(0x2800, 0x2800) AM_DEVWRITE("crtc", mc6845_device, address_w)
	AM_RANGE(0x2801, 0x2801) AM_DEVREADWRITE("crtc", mc6845_device, register_r, register_w)
	AM_RANGE(0x3000, 0x3001) AM_DEVREADWRITE_LEGACY("ay8910", ay8910_r, ay8910_address_data_w)	// FIXME
	AM_RANGE(0x3080, 0x3083) AM_DEVREADWRITE("pia0", pia6821_device, read, write)
	AM_RANGE(0x3800, 0x3800) AM_READNOP	// R (right after each read, another value is loaded to the ACCU, so it lacks of sense)
	AM_RANGE(0x4000, 0xffff) AM_ROM
ADDRESS_MAP_END

/*

'maincpu' (E190): unmapped program memory byte read from 3800
'maincpu' (E190): unmapped program memory byte read from 3800
'maincpu' (E190): unmapped program memory byte read from 3800
'maincpu' (E190): unmapped program memory byte read from 3800

'maincpu' (CF41): unmapped program memory byte read from 3081
'maincpu' (CF41): unmapped program memory byte write to 3081 = 00
'maincpu' (CF41): unmapped program memory byte read from 3083
'maincpu' (CF41): unmapped program memory byte write to 3083 = 00
'maincpu' (CF41): unmapped program memory byte read from 3080
'maincpu' (CF41): unmapped program memory byte write to 3080 = 00
'maincpu' (CF41): unmapped program memory byte read from 3082
'maincpu' (CF41): unmapped program memory byte write to 3082 = 00
'maincpu' (CF41): unmapped program memory byte read from 3081
'maincpu' (CF41): unmapped program memory byte write to 3081 = 3F
'maincpu' (CF41): unmapped program memory byte read from 3083
'maincpu' (CF41): unmapped program memory byte write to 3083 = 34

'maincpu' (CF5A): unmapped program memory byte write to 3000 = 0E
'maincpu' (CF61): unmapped program memory byte write to 3001 = FF
'maincpu' (CF5A): unmapped program memory byte write to 3000 = 0F
'maincpu' (CF61): unmapped program memory byte write to 3001 = FF
'maincpu' (CF5A): unmapped program memory byte write to 3000 = 07
'maincpu' (CF61): unmapped program memory byte write to 3001 = FF
'maincpu' (CF5A): unmapped program memory byte write to 3000 = 0E
'maincpu' (CF61): unmapped program memory byte write to 3001 = FF
'maincpu' (CF5A): unmapped program memory byte write to 3000 = 0F
'maincpu' (CF61): unmapped program memory byte write to 3001 = FF

  ... CRTC init (snap) --> $CF2D: JSR $CF76

'maincpu' (E189): unmapped program memory byte read from 3800

'maincpu' (E1A0): unmapped program memory byte write to 3000 = 0E
'maincpu' (E1A3): unmapped program memory byte read from 3000
'maincpu' (E1A8): unmapped program memory byte write to 3001 = 00
'maincpu' (E1BF): unmapped program memory byte write to 3000 = 0E
'maincpu' (E1C2): unmapped program memory byte read from 3000
'maincpu' (E1CA): unmapped program memory byte write to 3001 = 1F

'maincpu' (E189): unmapped program memory byte read from 3800
'maincpu' (E189): unmapped program memory byte read from 3800
'maincpu' (E189): unmapped program memory byte read from 3800
'maincpu' (E189): unmapped program memory byte read from 3800
'maincpu' (E189): unmapped program memory byte read from 3800
'maincpu' (E189): unmapped program memory byte read from 3800

*/

/***********************************
*           Input Ports            *
***********************************/

static INPUT_PORTS_START( miniboy7 )
INPUT_PORTS_END


/***********************************
*         Graphics Layouts         *
***********************************/

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


/****************************************
*      Graphics Decode Information      *
****************************************/

static GFXDECODE_START( miniboy7 )
	GFXDECODE_ENTRY( "gfx1", 0x0800,	charlayout, 0, 16 )	/* text layer 1 */
	GFXDECODE_ENTRY( "gfx1", 0x0000,	charlayout, 0, 16 )	/* text layer 2 */

    /* 0x000 cards
       0x100 joker
       0x200 dices
       0x300 bigtxt */
	GFXDECODE_ENTRY( "gfx2", 0,	tilelayout, 0, 16 )

GFXDECODE_END


/**********************************
*         CRTC Interface          *
**********************************/

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


/*********************************
*         PIA Interface          *
*********************************/

static const pia6821_interface miniboy7_pia0_intf =
{
	DEVCB_NULL,		/* port A in */
	DEVCB_NULL,		/* port B in */
	DEVCB_NULL,		/* line CA1 in */
	DEVCB_NULL,		/* line CB1 in */
	DEVCB_NULL,		/* line CA2 in */
	DEVCB_NULL,		/* line CB2 in */
	DEVCB_NULL,		/* port A out */
	DEVCB_NULL,		/* port B out */
	DEVCB_NULL,		/* line CA2 out */
	DEVCB_NULL,		/* port CB2 out */
	DEVCB_NULL,		/* IRQA */
	DEVCB_NULL		/* IRQB */
};


/***********************************
*         Sound Interface          *
***********************************/

static const ay8910_interface miniboy7_ay8910_intf =
{
	AY8910_LEGACY_OUTPUT,
	AY8910_DEFAULT_LOADS,
	DEVCB_NULL,
	DEVCB_NULL,
	DEVCB_NULL,
	DEVCB_NULL
};


/***********************************
*         Machine Drivers          *
***********************************/

static MACHINE_CONFIG_START( miniboy7, miniboy7_state )

	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", M6502, MASTER_CLOCK/16)	/* guess */
	MCFG_CPU_PROGRAM_MAP(miniboy7_map)
	MCFG_CPU_VBLANK_INT("screen", nmi_line_pulse)

	MCFG_NVRAM_ADD_0FILL("nvram")
	MCFG_PIA6821_ADD("pia0", miniboy7_pia0_intf)

	/* video hardware */
	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_REFRESH_RATE(60)
	MCFG_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(0))
	MCFG_SCREEN_SIZE((47+1)*8, (39+1)*8)                  /* Taken from MC6845, registers 00 & 04. Normally programmed with (value-1) */
	MCFG_SCREEN_VISIBLE_AREA(0*8, 37*8-1, 0*8, 37*8-1)    /* Taken from MC6845, registers 01 & 06 */
	MCFG_SCREEN_UPDATE_STATIC(miniboy7)

	MCFG_GFXDECODE(miniboy7)

	MCFG_PALETTE_INIT(miniboy7)
	MCFG_PALETTE_LENGTH(256)
	MCFG_VIDEO_START(miniboy7)

	MCFG_MC6845_ADD("crtc", MC6845, MASTER_CLOCK/12, mc6845_intf) /* guess */

	/* sound hardware */
	MCFG_SPEAKER_STANDARD_MONO("mono")
	MCFG_SOUND_ADD("ay8910", AY8910, MASTER_CLOCK/8)	/* guess */
	MCFG_SOUND_CONFIG(miniboy7_ay8910_intf)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.75)

MACHINE_CONFIG_END


/***********************************
*             Rom Load             *
***********************************/

/*

  Board silkscreened on top:
  be MVX-001-01  ('be' is a Bonanza Enterprises logo).

  1x 6502.
  1x AY-3-8910.
  1x MC6821.
  1x HD46505 HD6845SP (Handwritten sticker '107040').
  1x 12.4725 Crystal.

  .a1    2764    No sticker.
  .a3    2764    Stickered 'MB7 5-4'
  .a4    2764    Stickered 'MB7 4-4'
  .a6    2764    Stickered 'MB7 3-4'
  .a7    2764    Stickered 'MB7 2-4'
  .a8    2764    Stickered 'MB7 6-4'
  .d11   2732    Stickered 'MB7 ASC CG'
  .d12   2764    Stickered 'MB7 CG1'
  .d13   2764    Stickered 'MB7 CG2'
  .d14   2764    Stickered 'MB7 CG3'

  .e7    82s10 read as 82s129, stickered 'J'
  .f10   82s10 read as 82s129, stickered 'J'

*/
ROM_START( miniboy7 )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "mb7_6-4.a8",   0x4000, 0x2000, CRC(a3fdea08) SHA1(2f1a74274005b8c77eb4254d0220206ae4175834) )
	ROM_LOAD( "mb7_2-4.a7",	  0x6000, 0x2000, CRC(396e7250) SHA1(8f8c86cc412269157b16ad883638b38bb21345d7) )
	ROM_LOAD( "mb7_3-4.a6",	  0x8000, 0x2000, CRC(360a7f7c) SHA1(d98bcfd320680e88b07182d78b4e56fc5579874d) )
	ROM_LOAD( "mb7_4-4.a4",	  0xa000, 0x2000, CRC(bff8e334) SHA1(1d09a86b4dbfec6522b326683febaf7426f723e0) )
	ROM_LOAD( "mb7_5-4.a3",	  0xc000, 0x2000, CRC(d610bed3) SHA1(67e44ce2345d5429d6ccf4833de207ff6518c534) )
	ROM_LOAD( "nosticker.a1", 0xe000, 0x2000, CRC(5f715a12) SHA1(eabe0e4ee2e110c6ce4fd58c9d36ba80a612d4b5) )	/* ROM 1-4? */

	ROM_REGION( 0x1000, "gfx1", 0 )
	ROM_LOAD( "mb7_asc_cg.d11",	0x0000, 0x1000, CRC(84f78ee2) SHA1(c434e8a9b19ef1394b1dac67455f859eef299f95) )	/* text layer */

	ROM_REGION( 0x6000, "gfx2", 0 )
	ROM_LOAD( "mb7_cg1.d12",	0x0000, 0x2000, CRC(5f3e3b93) SHA1(41ab6a42a41ddeb8b6b76f4d790bf9fb9e7c32a3) )	/* bitplane 1 */
	ROM_LOAD( "mb7_cg2.d13",	0x2000, 0x2000, CRC(b3362650) SHA1(603907fd3a0049c0a3e1858c4329bf9fd58137f6) )	/* bitplane 2 */
	ROM_LOAD( "mb7_cg3.d14",	0x4000, 0x2000, CRC(10c2bf71) SHA1(23a01625b0fc0b772054ee4bc026d2257df46a03) )	/* bitplane 3 */

	ROM_REGION( 0x0200, "proms", 0 )	/* both bipolar PROMs are identical */
	ROM_LOAD( "j.e7",	0x0000, 0x0100, CRC(4b66215e) SHA1(de4a8f1ee7b9bea02f3a5fc962358d19c7a871a0) )
	ROM_LOAD( "j.f10",	0x0100, 0x0100, CRC(4b66215e) SHA1(de4a8f1ee7b9bea02f3a5fc962358d19c7a871a0) )
ROM_END

/*
   Incomplete set with some bad dumps.
   Seems to be a different version/revision.
*/
ROM_START( miniboy7a )
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


/***********************************
*           Game Drivers           *
***********************************/

/*    YEAR  NAME       PARENT    MACHINE   INPUT     INIT   ROT    COMPANY                     FULLNAME             FLAGS  */
GAME( 1983, miniboy7,  0,        miniboy7, miniboy7, 0,     ROT0, "Bonanza Enterprises, Ltd", "Mini Boy 7 (set 1)", GAME_NO_SOUND | GAME_WRONG_COLORS | GAME_NOT_WORKING )
GAME( 1983, miniboy7a, miniboy7, miniboy7, miniboy7, 0,     ROT0, "Bonanza Enterprises, Ltd", "Mini Boy 7 (set 2)", GAME_NO_SOUND | GAME_WRONG_COLORS | GAME_NOT_WORKING )

