/***************************************************************************

Omega Fighter
----------------------
driver by Yochizo

This driver is heavily dependent on the Raine source.
Very thanks to Richard Bush and the Raine team.


Supported games :
==================
 Omega Fighter     (C) 1989 UPL
 Atomic Robokid    (C) 1988 UPL

Known issues :
================
 - Cocktail mode has not been supported yet.
 - Omega Fighter has a input protection. Currently it is hacked instead
   of emulated.
 - I don't know if Omega Fighter uses sprite overdraw flag or not.
 - Sometimes sprites stays behind the screen in Atomic Robokid due to
   incomplete sprite overdraw emulation.
 - Currently it has not been implemented palette marking in sprite
   overdraw mode.
 - When RAM and ROM check and color test mode, the palette is overflows.
   16 bit color is needed ?

NOTE :
========
 - To skip dip setting display, press 1P + 2P start in Atomic Robokid.
 - To enter the level code in Atomic Robokid, in the title screen insert 1 coin
   and then press 1P or 2P start while keeping pressed 1P or 2P button1

***************************************************************************/

/*

    TODO:

    - "XXX Intturupt Hold ???" msg at post screen
    - coin counters/lockouts
    - sprites are invisible in flipscreen mode

*/

#include "driver.h"
#include "sound/2203intf.h"


/**************************************************************************
  Variables
**************************************************************************/

extern UINT8 *omegaf_fg_videoram;
extern size_t omegaf_fgvideoram_size;

extern UINT8 *omegaf_bg0_scroll_x;
extern UINT8 *omegaf_bg1_scroll_x;
extern UINT8 *omegaf_bg2_scroll_x;
extern UINT8 *omegaf_bg0_scroll_y;
extern UINT8 *omegaf_bg1_scroll_y;
extern UINT8 *omegaf_bg2_scroll_y;

extern WRITE8_HANDLER( omegaf_bg0_bank_w );
extern WRITE8_HANDLER( omegaf_bg1_bank_w );
extern WRITE8_HANDLER( omegaf_bg2_bank_w );
extern READ8_HANDLER( omegaf_bg0_videoram_r );
extern READ8_HANDLER( omegaf_bg1_videoram_r );
extern READ8_HANDLER( omegaf_bg2_videoram_r );
extern WRITE8_HANDLER( omegaf_bg0_videoram_w );
extern WRITE8_HANDLER( omegaf_bg1_videoram_w );
extern WRITE8_HANDLER( omegaf_bg2_videoram_w );
extern WRITE8_HANDLER( robokid_bg0_videoram_w );
extern WRITE8_HANDLER( robokid_bg1_videoram_w );
extern WRITE8_HANDLER( robokid_bg2_videoram_w );
extern WRITE8_HANDLER( omegaf_bg0_scrollx_w );
extern WRITE8_HANDLER( omegaf_bg1_scrollx_w );
extern WRITE8_HANDLER( omegaf_bg2_scrollx_w );
extern WRITE8_HANDLER( omegaf_bg0_scrolly_w );
extern WRITE8_HANDLER( omegaf_bg1_scrolly_w );
extern WRITE8_HANDLER( omegaf_bg2_scrolly_w );
extern WRITE8_HANDLER( omegaf_fgvideoram_w );
extern WRITE8_HANDLER( omegaf_bg0_enabled_w );
extern WRITE8_HANDLER( omegaf_bg1_enabled_w );
extern WRITE8_HANDLER( omegaf_bg2_enabled_w );
extern WRITE8_HANDLER( omegaf_sprite_overdraw_w );
extern WRITE8_HANDLER( omegaf_flipscreen_w );

extern VIDEO_START( omegaf );
extern VIDEO_START( robokid );
extern VIDEO_UPDATE( omegaf );
extern VIDEO_UPDATE( robokid );

static int omegaf_bank_latch = 2;


/**************************************************************************
  Initializers
**************************************************************************/

static DRIVER_INIT( omegaf )
{
	UINT8 *RAM = memory_region(REGION_CPU1);

	/* Hack the input protection. $00 and $01 code is written to $C005 */
	/* and $C006.                                                      */

	RAM[0x029a] = 0x00;
	RAM[0x029b] = 0x00;
	RAM[0x02a6] = 0x00;
	RAM[0x02a7] = 0x00;

	RAM[0x02b2] = 0xC9;
	RAM[0x02b5] = 0xC9;
	RAM[0x02c9] = 0xC9;
	RAM[0x02f6] = 0xC9;

	RAM[0x05f0] = 0x00;
	RAM[0x054c] = 0x04;
	RAM[0x0557] = 0x03;


	/* Fix ROM check */

	RAM[0x0b8d] = 0x00;
	RAM[0x0b8e] = 0x00;
	RAM[0x0b8f] = 0x00;
}


/**************************************************************************
  Interrupts
**************************************************************************/

static INTERRUPT_GEN( omegaf_interrupt )
{
	cpunum_set_input_line_and_vector(0, 0, HOLD_LINE, 0xd7);	/* RST 10h */
}


/**************************************************************************
  Inputs
**************************************************************************/

static INPUT_PORTS_START( omegaf )
	PORT_START			/* Player 1 inputs */
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON1 )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON2 )
	PORT_BIT( 0xc0, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START			/* Player 2 inputs */
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_COCKTAIL
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY PORT_COCKTAIL
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY PORT_COCKTAIL
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY PORT_COCKTAIL
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_COCKTAIL
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_COCKTAIL
	PORT_BIT( 0xc0, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START			/* System inputs */
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_START2 )
	PORT_BIT( 0x0c, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_SERVICE )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_SERVICE1 )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_COIN2 )

	PORT_START			/* DSW 0 */
	PORT_DIPNAME( 0x01, 0x01, DEF_STR( Flip_Screen ) )
	PORT_DIPSETTING(    0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x06, 0x06, DEF_STR( Difficulty ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Easy ) )
	PORT_DIPSETTING(    0x06, DEF_STR( Normal ) )
	PORT_DIPSETTING(    0x02, DEF_STR( Hard ) )
	PORT_DIPSETTING(    0x04, DEF_STR( Hardest ) )
	PORT_SERVICE( 0x08, IP_ACTIVE_LOW )
	PORT_DIPNAME( 0x10, 0x00, DEF_STR( Cabinet) )
	PORT_DIPSETTING(    0x00, DEF_STR( Upright ) )
	PORT_DIPSETTING(    0x10, DEF_STR( Cocktail ) )
	PORT_DIPNAME( 0x20, 0x20, DEF_STR( Demo_Sounds ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x20, DEF_STR( On ) )
	PORT_DIPNAME( 0xc0, 0xc0, DEF_STR( Lives ) )
	PORT_DIPSETTING(    0x00, "2" )
	PORT_DIPSETTING(    0xc0, "3" )
	PORT_DIPSETTING(    0x40, "4" )
	PORT_DIPSETTING(    0x80, "5" )

	PORT_START 			/* DSW 1 */
	PORT_DIPNAME( 0x03, 0x03, DEF_STR( Bonus_Life ) )
	PORT_DIPSETTING(    0x00, "20000" )
	PORT_DIPSETTING(    0x03, "30000" )
	PORT_DIPSETTING(    0x01, "50000" )
	PORT_DIPSETTING(    0x02, "100000" )
	PORT_DIPNAME( 0x1c, 0x1c, DEF_STR( Coin_B ) )
	PORT_DIPSETTING(    0x00, DEF_STR( 5C_1C ) )
	PORT_DIPSETTING(    0x10, DEF_STR( 4C_1C ) )
	PORT_DIPSETTING(    0x08, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(    0x18, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x1c, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x0c, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0x14, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(    0x04, DEF_STR( 1C_4C ) )
	PORT_DIPNAME( 0xe0, 0xe0, DEF_STR( Coin_A ) )
	PORT_DIPSETTING(    0x00, DEF_STR( 5C_1C ) )
	PORT_DIPSETTING(    0x80, DEF_STR( 4C_1C ) )
	PORT_DIPSETTING(    0x40, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(    0xc0, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0xe0, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x60, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0xa0, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(    0x20, DEF_STR( 1C_4C ) )
INPUT_PORTS_END

static INPUT_PORTS_START( robokid )
	PORT_START			/* Player 1 inputs */
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON1 )	// fire
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON2 )	// jump
	PORT_BIT( 0xc0, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START			/* Player 2 inputs */
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_COCKTAIL
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY PORT_COCKTAIL
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY PORT_COCKTAIL
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY PORT_COCKTAIL
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_COCKTAIL
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_COCKTAIL
	PORT_BIT( 0xc0, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START			/* System inputs */
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_START1 )

	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_START2 )
	PORT_BIT( 0x0c, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_SERVICE )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_COIN2 )

	PORT_START			/* DSW 0 */
	PORT_DIPNAME( 0x01, 0x01, DEF_STR( Flip_Screen ) )
	PORT_DIPSETTING(    0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x02, DEF_STR( Bonus_Life ) )
	PORT_DIPSETTING(    0x02, "50K 100K+" )
	PORT_DIPSETTING(	0x00, DEF_STR( Off ) )
	PORT_DIPNAME( 0x04, 0x04, DEF_STR( Difficulty ) )
	PORT_DIPSETTING(    0x04, DEF_STR( Normal ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Hard ) )
	PORT_DIPNAME( 0x08, 0x08, DEF_STR( Free_Play ) )
	PORT_DIPSETTING(    0x08, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x00, DEF_STR( Cabinet ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Upright ) )
	PORT_DIPSETTING(    0x10, DEF_STR( Cocktail ) )
	PORT_DIPNAME( 0x20, 0x00, DEF_STR( Demo_Sounds ) )
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0xc0, 0xc0, DEF_STR( Lives ) )
	PORT_DIPSETTING(    0x80, "2" )
	PORT_DIPSETTING(    0xc0, "3" )
	PORT_DIPSETTING(    0x40, "4" )
	PORT_DIPSETTING(    0x00, "5" )

	PORT_START 			/* DSW 1 */
	PORT_SERVICE( 0x01, IP_ACTIVE_LOW )
	PORT_BIT( 0x1e, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_DIPNAME( 0xe0, 0xe0, DEF_STR( Coinage ) )
	PORT_DIPSETTING(    0x00, DEF_STR( 5C_1C ) )
	PORT_DIPSETTING(    0x20, DEF_STR( 4C_1C ) )
	PORT_DIPSETTING(    0x40, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(    0x60, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0xe0, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0xc0, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0xa0, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(    0x80, DEF_STR( 1C_4C ) )
INPUT_PORTS_END


static INPUT_PORTS_START( robokidj )
	PORT_START			/* Player 1 inputs */
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON1 )	// fire
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON2 )	// jump
	PORT_BIT( 0xc0, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START			/* Player 2 inputs */
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_COCKTAIL
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY PORT_COCKTAIL
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY PORT_COCKTAIL
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY PORT_COCKTAIL
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_COCKTAIL
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_COCKTAIL
	PORT_BIT( 0xc0, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START			/* System inputs */
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_START2 )
	PORT_BIT( 0x0c, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_SERVICE )	/* keep pressed during boot to enter service mode */
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_COIN2 )

	PORT_START			/* DSW 0 */
	PORT_DIPNAME( 0x01, 0x01, DEF_STR( Flip_Screen ) )
	PORT_DIPSETTING(    0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x02, DEF_STR( Bonus_Life ) )
	PORT_DIPSETTING(    0x02, "30K 50K+" )
	PORT_DIPSETTING(	0x00, "50K 80K+" )
	PORT_DIPNAME( 0x04, 0x04, DEF_STR( Difficulty ) )
	PORT_DIPSETTING(    0x04, DEF_STR( Normal ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Hard ) )
	PORT_DIPNAME( 0x08, 0x08, DEF_STR( Free_Play ) )
	PORT_DIPSETTING(    0x08, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x00, DEF_STR( Cabinet ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Upright ) )
	PORT_DIPSETTING(    0x10, DEF_STR( Cocktail ) )
	PORT_DIPNAME( 0x20, 0x00, DEF_STR( Demo_Sounds ) )
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0xc0, 0xc0, DEF_STR( Lives ) )
	PORT_DIPSETTING(    0x80, "2" )
	PORT_DIPSETTING(    0xc0, "3" )
	PORT_DIPSETTING(    0x40, "4" )
	PORT_DIPSETTING(    0x00, "5" )

	PORT_START 			/* DSW 1 */
	PORT_SERVICE( 0x01, IP_ACTIVE_LOW )
	PORT_BIT( 0x1e, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_DIPNAME( 0xe0, 0xe0, DEF_STR( Coinage ) )
	PORT_DIPSETTING(    0x00, DEF_STR( 5C_1C ) )
	PORT_DIPSETTING(    0x20, DEF_STR( 4C_1C ) )
	PORT_DIPSETTING(    0x40, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(    0x60, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0xe0, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0xc0, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0xa0, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(    0x80, DEF_STR( 1C_4C ) )
INPUT_PORTS_END


/**************************************************************************
  Memory handlers
**************************************************************************/

static WRITE8_HANDLER( omegaf_bankselect_w )
{
	UINT8 *RAM = memory_region(REGION_CPU1);
	int bankaddress;

	if ( (data & 0x0f) != omegaf_bank_latch )
	{
		omegaf_bank_latch = data & 0x0f;

		if (omegaf_bank_latch < 2)
			bankaddress = omegaf_bank_latch * 0x4000;
		else
			bankaddress = 0x10000 + ( (omegaf_bank_latch - 2) * 0x4000);
		memory_set_bankptr( 1, &RAM[bankaddress] );	 /* Select 16 banks of 16k */
	}
}


/**************************************************************************
  Memory maps
**************************************************************************/

static ADDRESS_MAP_START( omegaf_readmem, ADDRESS_SPACE_PROGRAM, 8 )
	AM_RANGE(0x0000, 0x7fff) AM_READ(MRA8_ROM)
	AM_RANGE(0x8000, 0xbfff) AM_READ(MRA8_BANK1)
	AM_RANGE(0xc000, 0xc000) AM_READ(input_port_2_r)			/* system input */
	AM_RANGE(0xc001, 0xc001) AM_READ(input_port_0_r)			/* player 1 input */
	AM_RANGE(0xc002, 0xc002) AM_READ(input_port_1_r)			/* player 2 input */
	AM_RANGE(0xc003, 0xc003) AM_READ(input_port_3_r)			/* DSW 1 input */
	AM_RANGE(0xc004, 0xc004) AM_READ(input_port_4_r)			/* DSW 2 input */
	AM_RANGE(0xc005, 0xc005) AM_READ(MRA8_NOP)
	AM_RANGE(0xc006, 0xc006) AM_READ(MRA8_NOP)
	AM_RANGE(0xc100, 0xc105) AM_READ(MRA8_RAM)
	AM_RANGE(0xc200, 0xc205) AM_READ(MRA8_RAM)
	AM_RANGE(0xc300, 0xc305) AM_READ(MRA8_RAM)
	AM_RANGE(0xc400, 0xc7ff) AM_READ(omegaf_bg0_videoram_r)	/* BG0 video RAM */
	AM_RANGE(0xc800, 0xcbff) AM_READ(omegaf_bg1_videoram_r)	/* BG1 video RAM */
	AM_RANGE(0xcc00, 0xcfff) AM_READ(omegaf_bg2_videoram_r)	/* BG2 video RAM */
	AM_RANGE(0xd000, 0xd7ff) AM_READ(MRA8_RAM)				/* FG RAM */
	AM_RANGE(0xd800, 0xdfff) AM_READ(MRA8_RAM)				/* palette RAM */
	AM_RANGE(0xe000, 0xf9ff) AM_READ(MRA8_RAM)				/* RAM */
	AM_RANGE(0xfa00, 0xffff) AM_READ(MRA8_RAM)				/* sprite RAM */
ADDRESS_MAP_END

static ADDRESS_MAP_START( omegaf_writemem, ADDRESS_SPACE_PROGRAM, 8 )
	AM_RANGE(0x0000, 0xbfff) AM_WRITE(MWA8_ROM)
	AM_RANGE(0xc000, 0xc000) AM_WRITE(soundlatch_w)
	AM_RANGE(0xc001, 0xc001) AM_WRITE(omegaf_flipscreen_w)
	AM_RANGE(0xc002, 0xc002) AM_WRITE(omegaf_bankselect_w)
	AM_RANGE(0xc003, 0xc003) AM_WRITE(omegaf_sprite_overdraw_w)
	AM_RANGE(0xc004, 0xc004) AM_WRITE(MWA8_NOP)							/* input protection */
	AM_RANGE(0xc005, 0xc005) AM_WRITE(MWA8_NOP)							/* input protection */
	AM_RANGE(0xc006, 0xc006) AM_WRITE(MWA8_NOP)							/* input protection */
	AM_RANGE(0xc100, 0xc101) AM_WRITE(omegaf_bg0_scrollx_w) AM_BASE(&omegaf_bg0_scroll_x)
	AM_RANGE(0xc102, 0xc103) AM_WRITE(omegaf_bg0_scrolly_w) AM_BASE(&omegaf_bg0_scroll_y)
	AM_RANGE(0xc104, 0xc104) AM_WRITE(omegaf_bg0_enabled_w)				/* BG0 enabled */
	AM_RANGE(0xc105, 0xc105) AM_WRITE(omegaf_bg0_bank_w)					/* BG0 bank select */
	AM_RANGE(0xc200, 0xc201) AM_WRITE(omegaf_bg1_scrollx_w) AM_BASE(&omegaf_bg1_scroll_x)
	AM_RANGE(0xc202, 0xc203) AM_WRITE(omegaf_bg1_scrolly_w) AM_BASE(&omegaf_bg1_scroll_y)
	AM_RANGE(0xc204, 0xc204) AM_WRITE(omegaf_bg1_enabled_w)				/* BG1 enabled */
	AM_RANGE(0xc205, 0xc205) AM_WRITE(omegaf_bg1_bank_w)					/* BG1 bank select */
	AM_RANGE(0xc300, 0xc301) AM_WRITE(omegaf_bg2_scrollx_w) AM_BASE(&omegaf_bg2_scroll_x)
	AM_RANGE(0xc302, 0xc303) AM_WRITE(omegaf_bg2_scrolly_w) AM_BASE(&omegaf_bg2_scroll_y)
	AM_RANGE(0xc304, 0xc304) AM_WRITE(omegaf_bg2_enabled_w)				/* BG2 enabled */
	AM_RANGE(0xc305, 0xc305) AM_WRITE(omegaf_bg2_bank_w)					/* BG2 bank select */
	AM_RANGE(0xc400, 0xc7ff) AM_WRITE(omegaf_bg0_videoram_w)				/* BG0 video RAM */
	AM_RANGE(0xc800, 0xcbff) AM_WRITE(omegaf_bg1_videoram_w)				/* BG1 video RAM */
	AM_RANGE(0xcc00, 0xcfff) AM_WRITE(omegaf_bg2_videoram_w)				/* BG2 video RAM */
	AM_RANGE(0xd000, 0xd7ff) AM_WRITE(omegaf_fgvideoram_w) AM_BASE(&omegaf_fg_videoram)
	AM_RANGE(0xd800, 0xdfff) AM_WRITE(paletteram_RRRRGGGGBBBBxxxx_be_w) AM_BASE(&paletteram)
	AM_RANGE(0xe000, 0xf9ff) AM_WRITE(MWA8_RAM)							/* RAM */
	AM_RANGE(0xfa00, 0xffff) AM_WRITE(MWA8_RAM) AM_BASE(&spriteram) AM_SIZE(&spriteram_size)
ADDRESS_MAP_END

static ADDRESS_MAP_START( robokid_readmem, ADDRESS_SPACE_PROGRAM, 8 )
	AM_RANGE(0x0000, 0x7fff) AM_READ(MRA8_ROM)
	AM_RANGE(0x8000, 0xbfff) AM_READ(MRA8_BANK1)
	AM_RANGE(0xc000, 0xc7ff) AM_READ(MRA8_RAM)				/* palette RAM */
	AM_RANGE(0xc800, 0xcfff) AM_READ(MRA8_RAM)				/* FG RAM */
	AM_RANGE(0xd000, 0xd3ff) AM_READ(omegaf_bg2_videoram_r)
	AM_RANGE(0xd400, 0xd7ff) AM_READ(omegaf_bg1_videoram_r)
	AM_RANGE(0xd800, 0xdbff) AM_READ(omegaf_bg0_videoram_r)
	AM_RANGE(0xdc00, 0xdc00) AM_READ(input_port_2_r)			/* system input */
	AM_RANGE(0xdc01, 0xdc01) AM_READ(input_port_0_r)			/* player 1 input */
	AM_RANGE(0xdc02, 0xdc02) AM_READ(input_port_1_r)			/* player 2 input */
	AM_RANGE(0xdc03, 0xdc03) AM_READ(input_port_3_r)			/* DSW 1 input */
	AM_RANGE(0xdc04, 0xdc04) AM_READ(input_port_4_r)			/* DSW 2 input */
	AM_RANGE(0xdd00, 0xdd05) AM_READ(MRA8_RAM)
	AM_RANGE(0xde00, 0xde05) AM_READ(MRA8_RAM)
	AM_RANGE(0xdf00, 0xdf05) AM_READ(MRA8_RAM)
	AM_RANGE(0xe000, 0xf9ff) AM_READ(MRA8_RAM)				/* RAM */
	AM_RANGE(0xfa00, 0xffff) AM_READ(MRA8_RAM)				/* sprite RAM */
ADDRESS_MAP_END

static ADDRESS_MAP_START( robokid_writemem, ADDRESS_SPACE_PROGRAM, 8 )
	AM_RANGE(0x0000, 0xbfff) AM_WRITE(MWA8_ROM)
	AM_RANGE(0xc000, 0xc7ff) AM_WRITE(paletteram_RRRRGGGGBBBBxxxx_be_w) AM_BASE(&paletteram)
	AM_RANGE(0xc800, 0xcfff) AM_WRITE(omegaf_fgvideoram_w) AM_BASE(&omegaf_fg_videoram)
	AM_RANGE(0xd000, 0xd3ff) AM_WRITE(robokid_bg2_videoram_w)				/* BG2 video RAM */
	AM_RANGE(0xd400, 0xd7ff) AM_WRITE(robokid_bg1_videoram_w)				/* BG1 video RAM */
	AM_RANGE(0xd800, 0xdbff) AM_WRITE(robokid_bg0_videoram_w)				/* BG0 video RAM */
	AM_RANGE(0xdc00, 0xdc00) AM_WRITE(soundlatch_w)
	AM_RANGE(0xdc01, 0xdc01) AM_WRITE(omegaf_flipscreen_w)
	AM_RANGE(0xdc02, 0xdc02) AM_WRITE(omegaf_bankselect_w)
	AM_RANGE(0xdc03, 0xdc03) AM_WRITE(omegaf_sprite_overdraw_w)
	AM_RANGE(0xdd00, 0xdd01) AM_WRITE(omegaf_bg0_scrollx_w) AM_BASE(&omegaf_bg0_scroll_x)
	AM_RANGE(0xdd02, 0xdd03) AM_WRITE(omegaf_bg0_scrolly_w) AM_BASE(&omegaf_bg0_scroll_y)
	AM_RANGE(0xdd04, 0xdd04) AM_WRITE(omegaf_bg0_enabled_w)				/* BG0 enabled */
	AM_RANGE(0xdd05, 0xdd05) AM_WRITE(omegaf_bg0_bank_w)					/* BG0 bank select */
	AM_RANGE(0xde00, 0xde01) AM_WRITE(omegaf_bg1_scrollx_w) AM_BASE(&omegaf_bg1_scroll_x)
	AM_RANGE(0xde02, 0xde03) AM_WRITE(omegaf_bg1_scrolly_w) AM_BASE(&omegaf_bg1_scroll_y)
	AM_RANGE(0xde04, 0xde04) AM_WRITE(omegaf_bg1_enabled_w)				/* BG1 enabled */
	AM_RANGE(0xde05, 0xde05) AM_WRITE(omegaf_bg1_bank_w)					/* BG1 bank select */
	AM_RANGE(0xdf00, 0xdf01) AM_WRITE(omegaf_bg2_scrollx_w) AM_BASE(&omegaf_bg2_scroll_x)
	AM_RANGE(0xdf02, 0xdf03) AM_WRITE(omegaf_bg2_scrolly_w) AM_BASE(&omegaf_bg2_scroll_y)
	AM_RANGE(0xdf04, 0xdf04) AM_WRITE(omegaf_bg2_enabled_w)				/* BG2 enabled */
	AM_RANGE(0xdf05, 0xdf05) AM_WRITE(omegaf_bg2_bank_w)					/* BG2 bank select */
	AM_RANGE(0xe000, 0xf9ff) AM_WRITE(MWA8_RAM)							/* RAM */
	AM_RANGE(0xfa00, 0xffff) AM_WRITE(MWA8_RAM) AM_BASE(&spriteram) AM_SIZE(&spriteram_size)
ADDRESS_MAP_END

static ADDRESS_MAP_START( sound_readmem, ADDRESS_SPACE_PROGRAM, 8 )
	AM_RANGE(0x0000, 0xbfff) AM_READ(MRA8_ROM)
	AM_RANGE(0xc000, 0xc7ff) AM_READ(MRA8_RAM)
	AM_RANGE(0xe000, 0xe000) AM_READ(soundlatch_r)
	AM_RANGE(0xefee, 0xefee) AM_READ(MRA8_NOP)
ADDRESS_MAP_END

static ADDRESS_MAP_START( sound_writemem, ADDRESS_SPACE_PROGRAM, 8 )
	AM_RANGE(0x0000, 0xbfff) AM_WRITE(MWA8_ROM)
	AM_RANGE(0xc000, 0xc7ff) AM_WRITE(MWA8_RAM)
	AM_RANGE(0xe000, 0xe000) AM_WRITE(MWA8_NOP)
	AM_RANGE(0xeff5, 0xeff6) AM_WRITE(MWA8_NOP)	/* sample frequency ??? */
	AM_RANGE(0xefee, 0xefee) AM_WRITE(MWA8_NOP)	/* chip command ?? */
	AM_RANGE(0xf000, 0xf000) AM_WRITE(MWA8_NOP)	// ???
ADDRESS_MAP_END

static ADDRESS_MAP_START( sound_readport, ADDRESS_SPACE_IO, 8 )
	ADDRESS_MAP_FLAGS( AMEF_ABITS(8) )
	AM_RANGE(0x00, 0x00) AM_READ(YM2203_status_port_0_r)
	AM_RANGE(0x01, 0x01) AM_READ(YM2203_read_port_0_r)
	AM_RANGE(0x80, 0x80) AM_READ(YM2203_status_port_1_r)
	AM_RANGE(0x81, 0x81) AM_READ(YM2203_read_port_1_r)
ADDRESS_MAP_END

static ADDRESS_MAP_START( sound_writeport, ADDRESS_SPACE_IO, 8 )
	ADDRESS_MAP_FLAGS( AMEF_ABITS(8) )
	AM_RANGE(0x00, 0x00) AM_WRITE(YM2203_control_port_0_w)
	AM_RANGE(0x01, 0x01) AM_WRITE(YM2203_write_port_0_w)
	AM_RANGE(0x80, 0x80) AM_WRITE(YM2203_control_port_1_w)
	AM_RANGE(0x81, 0x81) AM_WRITE(YM2203_write_port_1_w)
ADDRESS_MAP_END


/**************************************************************************
  GFX decoding
**************************************************************************/

static const gfx_layout omegaf_charlayout =
{
	8, 8,	/* 8x8 characters */
	1024,	/* 1024 characters */
	4,		/* 4 bits per pixel */
	{ 0, 1, 2, 3 },
	{ 0, 4, 8, 12, 16, 20, 24, 28 },
	{ 32*0, 32*1, 32*2, 32*3, 32*4, 32*5, 32*6, 32*7 },
	8*32
};

static const gfx_layout omegaf_spritelayout =
{
	16, 16,	/* 16x16 characters */
	1024,
	4,
	{ 0, 1, 2, 3 },
	{ 0, 4, 8, 12, 16, 20, 24, 28,
		64*8+0, 64*8+4, 64*8+8, 64*8+12, 64*8+16, 64*8+20, 64*8+24, 64*8+28 },
	{ 32*0, 32*1, 32*2, 32*3, 32*4, 32*5, 32*6, 32*7,
		32*8, 32*9, 32*10, 32*11, 32*12, 32*13, 32*14, 32*15 },
	16*64
};

static const gfx_layout omegaf_bigspritelayout =
{
	32, 32,	/* 32x32 characters */
	256,
	4,
	{ 0, 1, 2, 3 },
	{	0, 4, 8, 12, 16, 20, 24, 28,
		64*8+0, 64*8+4, 64*8+8, 64*8+12, 64*8+16, 64*8+20, 64*8+24, 64*8+28,
		128*16+0, 128*16+4, 128*16+8, 128*16+12, 128*16+16, 128*16+20, 128*16+24, 128*16+28,
		128*16+64*8+0, 128*16+64*8+4, 128*16+64*8+8, 128*16+64*8+12, 128*16+64*8+16, 128*16+64*8+20, 128*16+64*8+24, 128*16+64*8+28 },
	{ 32*0, 32*1, 32*2, 32*3, 32*4, 32*5, 32*6, 32*7,
		32*8, 32*9, 32*10, 32*11, 32*12, 32*13, 32*14, 32*15,
		64*16+32*0, 64*16+32*1, 64*16+32*2, 64*16+32*3, 64*16+32*4, 64*16+32*5, 64*16+32*6, 64*16+32*7,
		64*16+32*8, 64*16+32*9, 64*16+32*10, 64*16+32*11, 64*16+32*12, 64*16+32*13, 64*16+32*14, 64*16+32*15 },
	16*64*4
};

static const gfx_layout omegaf_bglayout =
{
	16, 16,	/* 16x16 characters */
	4096,
	4,
	{ 0, 1, 2, 3 },
	{ 0, 4, 8, 12, 16, 20, 24, 28,
		64*8+0, 64*8+4, 64*8+8, 64*8+12, 64*8+16, 64*8+20, 64*8+24, 64*8+28 },
	{ 32*0, 32*1, 32*2, 32*3, 32*4, 32*5, 32*6, 32*7,
		32*8, 32*9, 32*10, 32*11, 32*12, 32*13, 32*14, 32*15 },
	16*64
};

static const gfx_layout robokid_spritelayout =
{
	16, 16,	/* 16x16 characters */
	2048,
	4,
	{ 0, 1, 2, 3 },
	{ 0, 4, 8, 12, 16, 20, 24, 28,
		64*8+0, 64*8+4, 64*8+8, 64*8+12, 64*8+16, 64*8+20, 64*8+24, 64*8+28 },
	{ 32*0, 32*1, 32*2, 32*3, 32*4, 32*5, 32*6, 32*7,
		32*8, 32*9, 32*10, 32*11, 32*12, 32*13, 32*14, 32*15 },
	16*64
};

static const gfx_layout robokid_bigspritelayout =
{
	32, 32,	/* 32x32 characters */
	512,
	4,
	{ 0, 1, 2, 3 },
	{	0, 4, 8, 12, 16, 20, 24, 28,
		64*8+0, 64*8+4, 64*8+8, 64*8+12, 64*8+16, 64*8+20, 64*8+24, 64*8+28,
		128*16+0, 128*16+4, 128*16+8, 128*16+12, 128*16+16, 128*16+20, 128*16+24, 128*16+28,
		128*16+64*8+0, 128*16+64*8+4, 128*16+64*8+8, 128*16+64*8+12, 128*16+64*8+16, 128*16+64*8+20, 128*16+64*8+24, 128*16+64*8+28 },
	{ 32*0, 32*1, 32*2, 32*3, 32*4, 32*5, 32*6, 32*7,
		32*8, 32*9, 32*10, 32*11, 32*12, 32*13, 32*14, 32*15,
		64*16+32*0, 64*16+32*1, 64*16+32*2, 64*16+32*3, 64*16+32*4, 64*16+32*5, 64*16+32*6, 64*16+32*7,
		64*16+32*8, 64*16+32*9, 64*16+32*10, 64*16+32*11, 64*16+32*12, 64*16+32*13, 64*16+32*14, 64*16+32*15 },
	16*64*4
};

static GFXDECODE_START( omegaf )
	GFXDECODE_ENTRY( REGION_GFX1, 0, omegaf_bglayout,         0*16, 16)
	GFXDECODE_ENTRY( REGION_GFX2, 0, omegaf_bglayout,         0*16, 16)
	GFXDECODE_ENTRY( REGION_GFX3, 0, omegaf_bglayout,         0*16, 16)
	GFXDECODE_ENTRY( REGION_GFX4, 0, omegaf_spritelayout,    32*16, 16)
	GFXDECODE_ENTRY( REGION_GFX4, 0, omegaf_bigspritelayout, 32*16, 16)
	GFXDECODE_ENTRY( REGION_GFX5, 0, omegaf_charlayout,      48*16, 16)
GFXDECODE_END

static GFXDECODE_START( robokid )
	GFXDECODE_ENTRY( REGION_GFX1, 0, omegaf_bglayout,          0*16, 16)
	GFXDECODE_ENTRY( REGION_GFX2, 0, omegaf_bglayout,          0*16, 16)
	GFXDECODE_ENTRY( REGION_GFX3, 0, omegaf_bglayout,          0*16, 16)
	GFXDECODE_ENTRY( REGION_GFX4, 0, robokid_spritelayout,    32*16, 16)
	GFXDECODE_ENTRY( REGION_GFX4, 0, robokid_bigspritelayout, 32*16, 16)
	GFXDECODE_ENTRY( REGION_GFX5, 0, omegaf_charlayout,       48*16, 16)
GFXDECODE_END


/**************************************************************************
  Machine drivers
**************************************************************************/

static MACHINE_DRIVER_START( omegaf )

	/* basic machine hardware */
	MDRV_CPU_ADD_TAG("main", Z80, 12000000/2)		/* 12000000/2 (verified on pcb) */
	MDRV_CPU_PROGRAM_MAP(omegaf_readmem,omegaf_writemem)	/* very sensitive to these settings */
	MDRV_CPU_VBLANK_INT(omegaf_interrupt,1)

	MDRV_CPU_ADD(Z80, 5000000)
	/* audio CPU */		/* 5 mhz crystal (verified on pcb) */
	MDRV_CPU_PROGRAM_MAP(sound_readmem,sound_writemem)
	MDRV_CPU_IO_MAP(sound_readport,sound_writeport)
	MDRV_CPU_VBLANK_INT(irq0_line_hold,2)

	MDRV_SCREEN_REFRESH_RATE(60)
	MDRV_SCREEN_VBLANK_TIME(DEFAULT_60HZ_VBLANK_DURATION)
	MDRV_INTERLEAVE(10)					/* number of slices per frame */

	/* video hardware */
	MDRV_VIDEO_ATTRIBUTES(VIDEO_TYPE_RASTER)
	MDRV_SCREEN_FORMAT(BITMAP_FORMAT_INDEXED16)
	MDRV_SCREEN_SIZE(128*16, 32*16)
	MDRV_SCREEN_VISIBLE_AREA(0*8, 32*8-1, 4*8, 28*8-1)
	MDRV_GFXDECODE(omegaf)
	MDRV_PALETTE_LENGTH(1024)

	MDRV_VIDEO_START(omegaf)
	MDRV_VIDEO_UPDATE(omegaf)

	/* sound hardware */
	MDRV_SPEAKER_STANDARD_MONO("mono")

	MDRV_SOUND_ADD(YM2203, 12000000/8) /* (verified on pcb) */
	MDRV_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.35)

	MDRV_SOUND_ADD(YM2203, 12000000/8) /* (verified on pcb) */
	MDRV_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.35)
MACHINE_DRIVER_END


static MACHINE_DRIVER_START( robokid )
	MDRV_IMPORT_FROM(omegaf)

	MDRV_CPU_MODIFY("main")
	MDRV_CPU_PROGRAM_MAP(robokid_readmem,robokid_writemem)

	MDRV_GFXDECODE(robokid)
	MDRV_VIDEO_START(robokid)
	MDRV_VIDEO_UPDATE(robokid)
MACHINE_DRIVER_END


/**************************************************************************
  ROM loaders
**************************************************************************/

ROM_START( omegaf )
	ROM_REGION( 0x48000, REGION_CPU1, 0 )						/* main CPU */
	ROM_LOAD( "1.5",          0x00000, 0x08000, CRC(57a7fd96) SHA1(65ca290b48f8579fcce00db5b3b3f8694667a136) )
	ROM_CONTINUE(             0x10000, 0x18000 )
	ROM_LOAD( "6.4l",         0x28000, 0x20000, CRC(6277735c) SHA1(b0f91f0cc51d424a1a7834c126736f24c2e23c17) )

	ROM_REGION( 0x10000, REGION_CPU2, 0 )						/* sound CPU */
	ROM_LOAD( "7.7m",         0x00000, 0x10000, CRC(d40fc8d5) SHA1(4f615a0fb786cafc20f82f0b5fa112a9c356378f) )

	ROM_REGION( 0x80000, REGION_GFX1, ROMREGION_DISPOSE )		/* BG0 */
	ROM_LOAD( "2back1.27b",   0x00000, 0x80000, CRC(21f8a32e) SHA1(26582e06e7381e09443fa99f24ca9edd0b4a2937) )

	ROM_REGION( 0x80000, REGION_GFX2, ROMREGION_DISPOSE )		/* BG1 */
	ROM_LOAD( "1back2.15b",   0x00000, 0x80000, CRC(6210ddcc) SHA1(89c091eeafcc92750d0ea303fcde8a8dc3eeba89) )

	ROM_REGION( 0x80000, REGION_GFX3, ROMREGION_DISPOSE )		/* BG2 */
	ROM_LOAD( "3back3.5f",    0x00000, 0x80000, CRC(c31cae56) SHA1(4cc2d0d70990ca04b0e3abd15e5afe183e98e4ab) )

	ROM_REGION( 0x20000, REGION_GFX4, ROMREGION_DISPOSE )		/* sprite */
	ROM_LOAD( "8.23m",        0x00000, 0x20000, CRC(0bd2a5d1) SHA1(ef84f1a5554e891fc38d17314e3952ea5c9d2731) )

	ROM_REGION( 0x08000, REGION_GFX5, ROMREGION_DISPOSE )		/* FG */
	ROM_LOAD( "4.18h",        0x00000, 0x08000, CRC(9e2d8152) SHA1(4b50557d171d1b03a870db5891ae67d70858ad37) )
ROM_END

ROM_START( omegafs )
	ROM_REGION( 0x48000, REGION_CPU1, 0 )						/* main CPU */
	ROM_LOAD( "5.3l",         0x00000, 0x08000, CRC(503a3e63) SHA1(73420aecb653cd4fd3b6afe67d6f5726f01411dd) )
	ROM_CONTINUE(             0x10000, 0x18000 )
	ROM_LOAD( "6.4l",         0x28000, 0x20000, CRC(6277735c) SHA1(b0f91f0cc51d424a1a7834c126736f24c2e23c17) )

	ROM_REGION( 0x10000, REGION_CPU2, 0 )						/* sound CPU */
	ROM_LOAD( "7.7m",         0x00000, 0x10000, CRC(d40fc8d5) SHA1(4f615a0fb786cafc20f82f0b5fa112a9c356378f) )

	ROM_REGION( 0x80000, REGION_GFX1, ROMREGION_DISPOSE )		/* BG0 */
	ROM_LOAD( "2back1.27b",   0x00000, 0x80000, CRC(21f8a32e) SHA1(26582e06e7381e09443fa99f24ca9edd0b4a2937) )

	ROM_REGION( 0x80000, REGION_GFX2, ROMREGION_DISPOSE )		/* BG1 */
	ROM_LOAD( "1back2.15b",   0x00000, 0x80000, CRC(6210ddcc) SHA1(89c091eeafcc92750d0ea303fcde8a8dc3eeba89) )

	ROM_REGION( 0x80000, REGION_GFX3, ROMREGION_DISPOSE )		/* BG2 */
	ROM_LOAD( "3back3.5f",    0x00000, 0x80000, CRC(c31cae56) SHA1(4cc2d0d70990ca04b0e3abd15e5afe183e98e4ab) )

	ROM_REGION( 0x20000, REGION_GFX4, ROMREGION_DISPOSE )		/* sprite */
	ROM_LOAD( "8.23m",        0x00000, 0x20000, CRC(0bd2a5d1) SHA1(ef84f1a5554e891fc38d17314e3952ea5c9d2731) )

	ROM_REGION( 0x08000, REGION_GFX5, ROMREGION_DISPOSE )		/* FG */
	ROM_LOAD( "4.18h",        0x00000, 0x08000, CRC(9e2d8152) SHA1(4b50557d171d1b03a870db5891ae67d70858ad37) )
ROM_END

ROM_START( robokid )
	ROM_REGION( 0x48000, REGION_CPU1, 0 )						/* main CPU */
	ROM_LOAD( "robokid1.18j", 0x00000, 0x08000, CRC(378c21fc) SHA1(58163bd6fbfa8385b1bd648cfde3d75bf81ac07d) )
	ROM_CONTINUE(             0x10000, 0x08000 )
	ROM_LOAD( "robokid2.18k", 0x18000, 0x10000, CRC(ddef8c5a) SHA1(a1dd2f51205863c3d5d3527991d538ca8adf7587) )
	ROM_LOAD( "robokid3.15k", 0x28000, 0x10000, CRC(05295ec3) SHA1(33dd0853a2064cb4301cfbdc7856def81f6e1223) )
	ROM_LOAD( "robokid4.12k", 0x38000, 0x10000, CRC(3bc3977f) SHA1(da394e12d197b0e109b03c854da06b1267bd9d59) )
	ROM_COPY( REGION_CPU1 ,   0x10000, 0x8000, 0x4000 ) /* to avoid crash */

	ROM_REGION( 0x10000, REGION_CPU2, 0 )						/* sound CPU */
	ROM_LOAD( "robokid.k7",   0x00000, 0x10000, CRC(f490a2e9) SHA1(861d1256c090ce3d1f45f95cc894affbbc3f1466) )

	ROM_REGION( 0x80000, REGION_GFX1, ROMREGION_DISPOSE )		/* BG0 */
	ROM_LOAD( "robokid.19c",  0x00000, 0x10000, CRC(02220421) SHA1(f533e9c6cea1dccbb60e0528c470f3cb5e8fc44e) )
	ROM_LOAD( "robokid.20c",  0x10000, 0x10000, CRC(02d59bc2) SHA1(031acbb14145f9f4623de8868c6207fb9f8e8207) )
	ROM_LOAD( "robokid.17d",  0x20000, 0x10000, CRC(2fa29b99) SHA1(13dce7932e2e9c03a139a4293584838aa3d9f1c3) )
	ROM_LOAD( "robokid.18d",  0x30000, 0x10000, CRC(ae15ce02) SHA1(175e4eebdf12f1f373e01a4b1c933053ddd09abf) )
	ROM_LOAD( "robokid.19d",  0x40000, 0x10000, CRC(784b089e) SHA1(1ae3346b4afa3da9484ebc59c8a530cb95f7d277) )
	ROM_LOAD( "robokid.20d",  0x50000, 0x10000, CRC(b0b395ed) SHA1(31ec07634053793a701bbfd601b029f7da66e9d7) )
	ROM_LOAD( "robokid.19f",  0x60000, 0x10000, CRC(0f9071c6) SHA1(8bf0c35189eda98a9bc150788890e136870cb5b2) )

	ROM_REGION( 0x80000, REGION_GFX2, ROMREGION_DISPOSE )		/* BG1 */
	ROM_LOAD( "robokid.12c",  0x00000, 0x10000, CRC(0ab45f94) SHA1(d8274263068d998c89a1b247dde7f814037cc15b) )
	ROM_LOAD( "robokid.14c",  0x10000, 0x10000, CRC(029bbd4a) SHA1(8e078cdafe608fc6cde827be85c5267ade4ecca6) )
	ROM_LOAD( "robokid.15c",  0x20000, 0x10000, CRC(7de67ebb) SHA1(2fe92e50e2894dd363e69b053db96bdb66a273eb) )
	ROM_LOAD( "robokid.16c",  0x30000, 0x10000, CRC(53c0e582) SHA1(763e6127532d022a707bf9ddf1a832413745f248) )
	ROM_LOAD( "robokid.17c",  0x40000, 0x10000, CRC(0cae5a1e) SHA1(a183a33516c81ea2c029b72ee6261c4519e095ab) )
	ROM_LOAD( "robokid.18c",  0x50000, 0x10000, CRC(56ac7c8a) SHA1(66ed5646a2e8563caeb4ff96fa7d34fde27e9899) )
	ROM_LOAD( "robokid.15d",  0x60000, 0x10000, CRC(cd632a4d) SHA1(a537d9ced45fdac490097e9162ac4d09a470be79) )
	ROM_LOAD( "robokid.16d",  0x70000, 0x10000, CRC(18d92b2b) SHA1(e6d20ea8f0fac8bd4824a3b279a0fd8a1d6c26f5) )

	ROM_REGION( 0x80000, REGION_GFX3, ROMREGION_DISPOSE )		/* BG2 */
	ROM_LOAD( "robokid.12a",  0x00000, 0x10000, CRC(e64d1c10) SHA1(d1073c80c9788aba65410f88691747a37b2a9d4a) )
	ROM_LOAD( "robokid.14a",  0x10000, 0x10000, CRC(8f9371e4) SHA1(0ea06d62bf4673ebda49a849cead832a24e5b886) )
	ROM_LOAD( "robokid.15a",  0x20000, 0x10000, CRC(469204e7) SHA1(8c2e94635b2b304e7dfa2e6ad58ba526dcf02453) )
	ROM_LOAD( "robokid.16a",  0x30000, 0x10000, CRC(4e340815) SHA1(d204b830c5809f25f7dfa451bbcbeda8b81ced54) )
	ROM_LOAD( "robokid.17a",  0x40000, 0x10000, CRC(f0863106) SHA1(ff7e44d0aa5a07ec9a7eddef1a55181bd2e867b1) )
	ROM_LOAD( "robokid.18a",  0x50000, 0x10000, CRC(fdff7441) SHA1(843b2c92bbc6f7319568677d50cbd9b03475b34a) )

	ROM_REGION( 0x40000, REGION_GFX4, ROMREGION_DISPOSE )		/* sprite */
	ROM_LOAD( "robokid.15f",  0x00000, 0x10000, CRC(ba61f5ab) SHA1(8433ddd55f0184cd5e8bb4a94a1c2336b2f8ff05) )
	ROM_LOAD( "robokid.16f",  0x10000, 0x10000, CRC(d9b399ce) SHA1(70755c9cae27187f183ae6d61bedb95c420756f4) )
	ROM_LOAD( "robokid.17f",  0x20000, 0x10000, CRC(afe432b9) SHA1(1ec7954ccf112eddf0ffcb8b5aec6cbc5cba7a7a) )
	ROM_LOAD( "robokid.18f",  0x30000, 0x10000, CRC(a0aa2a84) SHA1(4d46c169429cd285644336c7d47e393b33bd8770) )

	ROM_REGION( 0x08000, REGION_GFX5, ROMREGION_DISPOSE )		/* FG */
	ROM_LOAD( "robokid.b9",   0x00000, 0x08000, CRC(fac59c3f) SHA1(1b202ad5c12982512129d9e097267dd31b984ae8) )
ROM_END

ROM_START( robokidj )
	ROM_REGION( 0x48000, REGION_CPU1, 0 )						/* main CPU */
	ROM_LOAD( "1.29",         0x00000, 0x08000, CRC(59a1e2ec) SHA1(71f9d28dd8d2cf77a27fab163ce9562e3e75a540) )
	ROM_CONTINUE(             0x10000, 0x08000 )
	ROM_LOAD( "2.30",         0x18000, 0x10000, CRC(e3f73476) SHA1(bd1c8946d637df21432bd52ae9324255251570b9) )
	ROM_LOAD( "robokid3.15k", 0x28000, 0x10000, CRC(05295ec3) SHA1(33dd0853a2064cb4301cfbdc7856def81f6e1223) )
	ROM_LOAD( "robokid4.12k", 0x38000, 0x10000, CRC(3bc3977f) SHA1(da394e12d197b0e109b03c854da06b1267bd9d59) )

	ROM_REGION( 0x10000, REGION_CPU2, 0 )						/* sound CPU */
	ROM_LOAD( "robokid.k7",   0x00000, 0x10000, CRC(f490a2e9) SHA1(861d1256c090ce3d1f45f95cc894affbbc3f1466) )

	ROM_REGION( 0x80000, REGION_GFX1, ROMREGION_DISPOSE )		/* BG0 */
	ROM_LOAD( "robokid.19c",  0x00000, 0x10000, CRC(02220421) SHA1(f533e9c6cea1dccbb60e0528c470f3cb5e8fc44e) )
	ROM_LOAD( "robokid.20c",  0x10000, 0x10000, CRC(02d59bc2) SHA1(031acbb14145f9f4623de8868c6207fb9f8e8207) )
	ROM_LOAD( "robokid.17d",  0x20000, 0x10000, CRC(2fa29b99) SHA1(13dce7932e2e9c03a139a4293584838aa3d9f1c3) )
	ROM_LOAD( "robokid.18d",  0x30000, 0x10000, CRC(ae15ce02) SHA1(175e4eebdf12f1f373e01a4b1c933053ddd09abf) )
	ROM_LOAD( "robokid.19d",  0x40000, 0x10000, CRC(784b089e) SHA1(1ae3346b4afa3da9484ebc59c8a530cb95f7d277) )
	ROM_LOAD( "robokid.20d",  0x50000, 0x10000, CRC(b0b395ed) SHA1(31ec07634053793a701bbfd601b029f7da66e9d7) )
	ROM_LOAD( "robokid.19f",  0x60000, 0x10000, CRC(0f9071c6) SHA1(8bf0c35189eda98a9bc150788890e136870cb5b2) )

	ROM_REGION( 0x80000, REGION_GFX2, ROMREGION_DISPOSE )		/* BG1 */
	ROM_LOAD( "robokid.12c",  0x00000, 0x10000, CRC(0ab45f94) SHA1(d8274263068d998c89a1b247dde7f814037cc15b) )
	ROM_LOAD( "robokid.14c",  0x10000, 0x10000, CRC(029bbd4a) SHA1(8e078cdafe608fc6cde827be85c5267ade4ecca6) )
	ROM_LOAD( "robokid.15c",  0x20000, 0x10000, CRC(7de67ebb) SHA1(2fe92e50e2894dd363e69b053db96bdb66a273eb) )
	ROM_LOAD( "robokid.16c",  0x30000, 0x10000, CRC(53c0e582) SHA1(763e6127532d022a707bf9ddf1a832413745f248) )
	ROM_LOAD( "robokid.17c",  0x40000, 0x10000, CRC(0cae5a1e) SHA1(a183a33516c81ea2c029b72ee6261c4519e095ab) )
	ROM_LOAD( "robokid.18c",  0x50000, 0x10000, CRC(56ac7c8a) SHA1(66ed5646a2e8563caeb4ff96fa7d34fde27e9899) )
	ROM_LOAD( "robokid.15d",  0x60000, 0x10000, CRC(cd632a4d) SHA1(a537d9ced45fdac490097e9162ac4d09a470be79) )
	ROM_LOAD( "robokid.16d",  0x70000, 0x10000, CRC(18d92b2b) SHA1(e6d20ea8f0fac8bd4824a3b279a0fd8a1d6c26f5) )

	ROM_REGION( 0x80000, REGION_GFX3, ROMREGION_DISPOSE )		/* BG2 */
	ROM_LOAD( "robokid.12a",  0x00000, 0x10000, CRC(e64d1c10) SHA1(d1073c80c9788aba65410f88691747a37b2a9d4a) )
	ROM_LOAD( "robokid.14a",  0x10000, 0x10000, CRC(8f9371e4) SHA1(0ea06d62bf4673ebda49a849cead832a24e5b886) )
	ROM_LOAD( "robokid.15a",  0x20000, 0x10000, CRC(469204e7) SHA1(8c2e94635b2b304e7dfa2e6ad58ba526dcf02453) )
	ROM_LOAD( "robokid.16a",  0x30000, 0x10000, CRC(4e340815) SHA1(d204b830c5809f25f7dfa451bbcbeda8b81ced54) )
	ROM_LOAD( "robokid.17a",  0x40000, 0x10000, CRC(f0863106) SHA1(ff7e44d0aa5a07ec9a7eddef1a55181bd2e867b1) )
	ROM_LOAD( "robokid.18a",  0x50000, 0x10000, CRC(fdff7441) SHA1(843b2c92bbc6f7319568677d50cbd9b03475b34a) )

	ROM_REGION( 0x40000, REGION_GFX4, ROMREGION_DISPOSE )		/* sprite */
	ROM_LOAD( "robokid.15f",  0x00000, 0x10000, CRC(ba61f5ab) SHA1(8433ddd55f0184cd5e8bb4a94a1c2336b2f8ff05) )
	ROM_LOAD( "robokid.16f",  0x10000, 0x10000, CRC(d9b399ce) SHA1(70755c9cae27187f183ae6d61bedb95c420756f4) )
	ROM_LOAD( "robokid.17f",  0x20000, 0x10000, CRC(afe432b9) SHA1(1ec7954ccf112eddf0ffcb8b5aec6cbc5cba7a7a) )
	ROM_LOAD( "robokid.18f",  0x30000, 0x10000, CRC(a0aa2a84) SHA1(4d46c169429cd285644336c7d47e393b33bd8770) )

	ROM_REGION( 0x08000, REGION_GFX5, ROMREGION_DISPOSE )		/* FG */
	ROM_LOAD( "robokid.b9",   0x00000, 0x08000, CRC(fac59c3f) SHA1(1b202ad5c12982512129d9e097267dd31b984ae8) )
ROM_END

ROM_START( robokdj2 )
	ROM_REGION( 0x48000, REGION_CPU1, 0 )						/* main CPU */
	ROM_LOAD( "1_rom29.18j",  0x00000, 0x08000, CRC(969fb951) SHA1(aa32f0cb33ba2ccbb933dab5444a7e0dbbb84b3d) )
	ROM_CONTINUE(             0x10000, 0x08000 )
	ROM_LOAD( "2_rom30.18k",  0x18000, 0x10000, CRC(c0228b63) SHA1(8f7e3a29a35723abc8b10bf511fc8611e31a2961) )
	ROM_LOAD( "robokid3.15k", 0x28000, 0x10000, CRC(05295ec3) SHA1(33dd0853a2064cb4301cfbdc7856def81f6e1223) )
	ROM_LOAD( "robokid4.12k", 0x38000, 0x10000, CRC(3bc3977f) SHA1(da394e12d197b0e109b03c854da06b1267bd9d59) )
	ROM_COPY( REGION_CPU1 ,   0x10000, 0x8000, 0x4000 ) /* to avoid crash */

	ROM_REGION( 0x10000, REGION_CPU2, 0 )						/* sound CPU */
	ROM_LOAD( "robokid.k7",   0x00000, 0x10000, CRC(f490a2e9) SHA1(861d1256c090ce3d1f45f95cc894affbbc3f1466) )

	ROM_REGION( 0x80000, REGION_GFX1, ROMREGION_DISPOSE )		/* BG0 */
	ROM_LOAD( "robokid.19c",  0x00000, 0x10000, CRC(02220421) SHA1(f533e9c6cea1dccbb60e0528c470f3cb5e8fc44e) )
	ROM_LOAD( "robokid.20c",  0x10000, 0x10000, CRC(02d59bc2) SHA1(031acbb14145f9f4623de8868c6207fb9f8e8207) )
	ROM_LOAD( "robokid.17d",  0x20000, 0x10000, CRC(2fa29b99) SHA1(13dce7932e2e9c03a139a4293584838aa3d9f1c3) )
	ROM_LOAD( "robokid.18d",  0x30000, 0x10000, CRC(ae15ce02) SHA1(175e4eebdf12f1f373e01a4b1c933053ddd09abf) )
	ROM_LOAD( "robokid.19d",  0x40000, 0x10000, CRC(784b089e) SHA1(1ae3346b4afa3da9484ebc59c8a530cb95f7d277) )
	ROM_LOAD( "robokid.20d",  0x50000, 0x10000, CRC(b0b395ed) SHA1(31ec07634053793a701bbfd601b029f7da66e9d7) )
	ROM_LOAD( "robokid.19f",  0x60000, 0x10000, CRC(0f9071c6) SHA1(8bf0c35189eda98a9bc150788890e136870cb5b2) )

	ROM_REGION( 0x80000, REGION_GFX2, ROMREGION_DISPOSE )		/* BG1 */
	ROM_LOAD( "robokid.12c",  0x00000, 0x10000, CRC(0ab45f94) SHA1(d8274263068d998c89a1b247dde7f814037cc15b) )
	ROM_LOAD( "robokid.14c",  0x10000, 0x10000, CRC(029bbd4a) SHA1(8e078cdafe608fc6cde827be85c5267ade4ecca6) )
	ROM_LOAD( "robokid.15c",  0x20000, 0x10000, CRC(7de67ebb) SHA1(2fe92e50e2894dd363e69b053db96bdb66a273eb) )
	ROM_LOAD( "robokid.16c",  0x30000, 0x10000, CRC(53c0e582) SHA1(763e6127532d022a707bf9ddf1a832413745f248) )
	ROM_LOAD( "robokid.17c",  0x40000, 0x10000, CRC(0cae5a1e) SHA1(a183a33516c81ea2c029b72ee6261c4519e095ab) )
	ROM_LOAD( "robokid.18c",  0x50000, 0x10000, CRC(56ac7c8a) SHA1(66ed5646a2e8563caeb4ff96fa7d34fde27e9899) )
	ROM_LOAD( "robokid.15d",  0x60000, 0x10000, CRC(cd632a4d) SHA1(a537d9ced45fdac490097e9162ac4d09a470be79) )
	ROM_LOAD( "robokid.16d",  0x70000, 0x10000, CRC(18d92b2b) SHA1(e6d20ea8f0fac8bd4824a3b279a0fd8a1d6c26f5) )

	ROM_REGION( 0x80000, REGION_GFX3, ROMREGION_DISPOSE )		/* BG2 */
	ROM_LOAD( "robokid.12a",  0x00000, 0x10000, CRC(e64d1c10) SHA1(d1073c80c9788aba65410f88691747a37b2a9d4a) )
	ROM_LOAD( "robokid.14a",  0x10000, 0x10000, CRC(8f9371e4) SHA1(0ea06d62bf4673ebda49a849cead832a24e5b886) )
	ROM_LOAD( "robokid.15a",  0x20000, 0x10000, CRC(469204e7) SHA1(8c2e94635b2b304e7dfa2e6ad58ba526dcf02453) )
	ROM_LOAD( "robokid.16a",  0x30000, 0x10000, CRC(4e340815) SHA1(d204b830c5809f25f7dfa451bbcbeda8b81ced54) )
	ROM_LOAD( "robokid.17a",  0x40000, 0x10000, CRC(f0863106) SHA1(ff7e44d0aa5a07ec9a7eddef1a55181bd2e867b1) )
	ROM_LOAD( "robokid.18a",  0x50000, 0x10000, CRC(fdff7441) SHA1(843b2c92bbc6f7319568677d50cbd9b03475b34a) )

	ROM_REGION( 0x40000, REGION_GFX4, ROMREGION_DISPOSE )		/* sprite */
	ROM_LOAD( "robokid.15f",  0x00000, 0x10000, CRC(ba61f5ab) SHA1(8433ddd55f0184cd5e8bb4a94a1c2336b2f8ff05) )
	ROM_LOAD( "robokid.16f",  0x10000, 0x10000, CRC(d9b399ce) SHA1(70755c9cae27187f183ae6d61bedb95c420756f4) )
	ROM_LOAD( "robokid.17f",  0x20000, 0x10000, CRC(afe432b9) SHA1(1ec7954ccf112eddf0ffcb8b5aec6cbc5cba7a7a) )
	ROM_LOAD( "robokid.18f",  0x30000, 0x10000, CRC(a0aa2a84) SHA1(4d46c169429cd285644336c7d47e393b33bd8770) )

	ROM_REGION( 0x08000, REGION_GFX5, ROMREGION_DISPOSE )		/* FG */
	ROM_LOAD( "robokid.b9",   0x00000, 0x08000, CRC(fac59c3f) SHA1(1b202ad5c12982512129d9e097267dd31b984ae8) )
ROM_END


/*   ( YEAR  NAME      PARENT   MACHINE  INPUT    INIT      MONITOR COMPANY  FULLNAME                 FLAGS ) */
GAME( 1988, robokid,  0,       robokid, robokid, 0,        ROT0,   "UPL",  "Atomic Robo-kid",                GAME_NO_COCKTAIL )
GAME( 1988, robokidj, robokid, robokid, robokidj,0,        ROT0,   "UPL",  "Atomic Robo-kid (Japan, Set 1)", GAME_NO_COCKTAIL )
GAME( 1988, robokdj2, robokid, robokid, robokidj,0,        ROT0,   "UPL",  "Atomic Robo-kid (Japan, Set 2)", GAME_NO_COCKTAIL )
GAME( 1989, omegaf,   0,       omegaf,  omegaf,  omegaf,   ROT270, "UPL",  "Omega Fighter",          GAME_NO_COCKTAIL )
GAME( 1989, omegafs,  omegaf,  omegaf,  omegaf,  omegaf,   ROT270, "UPL",  "Omega Fighter Special",  GAME_NO_COCKTAIL )
