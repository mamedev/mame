// license:BSD-3-Clause
// copyright-holders:Howie Cohen, Frank Palazzolo, Alex Pasadyn, David Haywood, Steph, Phil Stroffolino, Uki
/******************************************************************************

UPL "orthogonal palette" hardware

driver by Howie Cohen, Frank Palazzolo, Alex Pasadyn, David Haywood, Steph, Phil Stroffolino, Uki

The peculiar feature of this hardware is the palette layout. 16 colors are
arranged in a 16x16 matrix, with all columns containing a single color, except
for column 1, which is a copy of row 1. Essentially, the 4bpp graphics are
drawn using a fixed palette except for one color that changes.
Nova 2001 and Penguin Kun Wars have palette PROMs and the effect applies to both
tilemaps and sprites. Ninjakun and Raiders5 have palette RAM and the effect
applies only to sprites while tilemaps use a normal palette.


Another peculiar characteristic is that in the later scrolling games (that is
Ninjakun and Raiders5, but not Nova 2001) the bg scroll registers are added to
the videoram address not only when the video circuitry is accessing it, as
usually happens, but also when the CPU is accessing it.


Game                      Board
------------------------  ---------
Nova 2001                 UPL-83005
Ninjakun Majou no Bouken  UPL-84003
Penguin Kun Wars          UPL-?????
Raiders5                  UPL-85004

Hardware Overview:
------------------
1xZ80 @ 3MHz (2x in Ninjakun and Raiders5)
2xAY8910 @ various frequencies

Resolution: 256x192
64 sprites
Tilemaps: fg 256x256 (8x8) + bg 256x256 (8x8) (Penguin Kun Wars has only one)


Designers:
----------
Nova 2001:
* Staff : Ryuichi Nishizawa (Bucha), HAL, Fukushi, Hiromi

Penguin Kun Wars:
* Tsutomu Fuzisawa, Ogata, Nobuyuki Narita, Kosikawa

Ninjakun Majou no Bouken:
* Staff : Ryuichi Nishizawa (Bucha), Todo, Fukushi, Tsutomu Fuzisawa, Tateno

Raiders5:
* Staff : Tsutomu Fuzisawa, Suzuko, Naotsugu
* Game, software and character design by : Ryuichi Nishizawa
* Hardware and effect design by : Nobuyuki Narita


Notes:
------
- nova2001 is VERY sensitive to coin inputs, if the coin isn't held down long
  enough, or is held down too long the game will reset, likewise if coins are
  inserted too quickly. This only happens in nova2001 and not in nova2001u.
  (the nova2001h set seems to be an unofficial fix for this issue, presumably
   it's so sensitive it would reset sometimes in the original cabinet?)

- Nova 2001 draws black bars on the sides of the screen so the visible area becomes
  240x192, however the physical resolution is still 256x192, the game probably does
  that to avoid wraparound glitches in the background without having to care about
  the sprite X MSB.

- In Ninjakun, hold P1 Start after a reset to skip the startup memory tests.

- The production year of Penguin Kun Wars is uncertain since it's not shown on
  screen. The console/home computer ports were made by ASCII in 1985, and the
  Ninjakun character appears in the audience (just on the left of the timer) so
  this game must have been produced after Ninjakun. This restricts the possibilities
  to 1985 or 1984.

- Penguin Kun Wars has some tile/sprite priority issues that have been verified
  to happen on the real board (e.g. the blob thing that moves in the middle of
  the table is not obscured by the text box at the end of a set).

TODO:
-----
- The Nova 2001 schematics show a FG priority bit, which should control priority
  over sprites. However that bit is set by the game only during the screen fade
  effect at the end of a level, so it would make the score area appear below
  the sprites, which definitely doesn't look right. For this reason, the priority
  bit is implemented in the video driver but it is ignored and the FG is always
  drawn above sprites.

- In Ninjakun, some garbage is drawn behind the GAME OVER text at the end of a
  game. Correct behaviour?

- The IRQ source of Ninjakun and Raiders5 CPU 2 is unknown.

- Several unknown memory read accesses in Raiders5.

*******************************************************************************

Nova 2001 Memory Map:

Address Range:     R/W:     Function:
--------------------------------------------------------------------------
0000 - 7fff        R        Program ROM (7000-7fff mirror of 6000-6fff)
a000 - a3ff        R/W      Foreground Playfield character RAM
a400 - a7ff        R/W      Foreground Playfield color modifier RAM
a800 - abff        R/W      Scrolling Playfield character RAM
ac00 - a7ff        R/W      Scrolling Playfield color modifier RAM
b000 - b7ff        R/W      Sprite RAM
bfff               W        flip screen
c000               R/W      AY8910 #1 Data R/W
c001               R/W      AY8910 #2 Data R/W
c002               W        AY8910 #1 Control W
c003               W        AY8910 #2 Control W
c004               R        Interrupt acknowledge / Watchdog reset
c006               R        Player 1 Controls
c007               R        Player 2 Controls
c00e               R        Coin Inputs, etc.
e000 - e7ff        R/W      Work RAM

******************************************************************************/

#include "emu.h"
#include "cpu/z80/z80.h"
#include "sound/ay8910.h"
#include "includes/nova2001.h"

#define MAIN_CLOCK XTAL_12MHz


/*************************************
 *
 *  Ninjakun 0xA000 Read / Write Handlers
 *
 *************************************/


CUSTOM_INPUT_MEMBER(nova2001_state::ninjakun_io_A002_ctrl_r)
{
	return m_ninjakun_io_a002_ctrl;
}

WRITE8_MEMBER(nova2001_state::ninjakun_cpu1_io_A002_w)
{
	if( data == 0x80 ) m_ninjakun_io_a002_ctrl |= 0x01;
	if( data == 0x40 ) m_ninjakun_io_a002_ctrl &= ~0x02;
}

WRITE8_MEMBER(nova2001_state::ninjakun_cpu2_io_A002_w)
{
	if( data == 0x40 ) m_ninjakun_io_a002_ctrl |= 0x02;
	if( data == 0x80 ) m_ninjakun_io_a002_ctrl &= ~0x01;
}



/*************************************
 *
 *  Init
 *
 *************************************/

MACHINE_START_MEMBER(nova2001_state,ninjakun)
{
	/* Save State Stuff */
	save_item(NAME(m_ninjakun_io_a002_ctrl));
}



/*************************************
 *
 *  Memory maps
 *
 *************************************/

static ADDRESS_MAP_START( nova2001_map, AS_PROGRAM, 8, nova2001_state )
	AM_RANGE(0x0000, 0x7fff) AM_ROM
	AM_RANGE(0xa000, 0xa7ff) AM_RAM_WRITE(nova2001_fg_videoram_w) AM_SHARE("fg_videoram")
	AM_RANGE(0xa800, 0xafff) AM_RAM_WRITE(nova2001_bg_videoram_w) AM_SHARE("bg_videoram")
	AM_RANGE(0xb000, 0xb7ff) AM_RAM AM_SHARE("spriteram")
	AM_RANGE(0xb800, 0xbfff) AM_WRITE(nova2001_flipscreen_w)
	AM_RANGE(0xc000, 0xc000) AM_DEVREADWRITE("ay1", ay8910_device, data_r, data_w)
	AM_RANGE(0xc001, 0xc001) AM_DEVREADWRITE("ay2", ay8910_device, data_r, data_w)
	AM_RANGE(0xc002, 0xc002) AM_DEVWRITE("ay1", ay8910_device, address_w)
	AM_RANGE(0xc003, 0xc003) AM_DEVWRITE("ay2", ay8910_device, address_w)
	AM_RANGE(0xc004, 0xc004) AM_READ(watchdog_reset_r)
	AM_RANGE(0xc006, 0xc006) AM_READ_PORT("IN0")
	AM_RANGE(0xc007, 0xc007) AM_READ_PORT("IN1")
	AM_RANGE(0xc00e, 0xc00e) AM_READ_PORT("IN2")
	AM_RANGE(0xe000, 0xe7ff) AM_RAM
ADDRESS_MAP_END


static ADDRESS_MAP_START( ninjakun_cpu1_map, AS_PROGRAM, 8, nova2001_state )
	AM_RANGE(0x0000, 0x1fff) AM_ROM
	AM_RANGE(0x2000, 0x7fff) AM_ROM
	AM_RANGE(0x8000, 0x8001) AM_DEVWRITE("ay1", ay8910_device, address_data_w)
	AM_RANGE(0x8001, 0x8001) AM_DEVREAD("ay1", ay8910_device, data_r)
	AM_RANGE(0x8002, 0x8003) AM_DEVWRITE("ay2", ay8910_device, address_data_w)
	AM_RANGE(0x8003, 0x8003) AM_DEVREAD("ay2", ay8910_device, data_r)
	AM_RANGE(0xa000, 0xa000) AM_READ_PORT("IN0")
	AM_RANGE(0xa001, 0xa001) AM_READ_PORT("IN1")
	AM_RANGE(0xa002, 0xa002) AM_READ_PORT("IN2") AM_WRITE(ninjakun_cpu1_io_A002_w)
	AM_RANGE(0xa003, 0xa003) AM_WRITE(pkunwar_flipscreen_w)
	AM_RANGE(0xc000, 0xc7ff) AM_RAM_WRITE(nova2001_fg_videoram_w) AM_SHARE("fg_videoram")
	AM_RANGE(0xc800, 0xcfff) AM_READWRITE(ninjakun_bg_videoram_r, ninjakun_bg_videoram_w) AM_SHARE("bg_videoram")
	AM_RANGE(0xd000, 0xd7ff) AM_RAM AM_SHARE("spriteram")
	AM_RANGE(0xd800, 0xd9ff) AM_RAM_WRITE(ninjakun_paletteram_w) AM_SHARE("palette")
	AM_RANGE(0xe000, 0xe3ff) AM_RAM AM_SHARE("share1")
	AM_RANGE(0xe400, 0xe7ff) AM_RAM AM_SHARE("share2")
ADDRESS_MAP_END

static ADDRESS_MAP_START( ninjakun_cpu2_map, AS_PROGRAM, 8, nova2001_state )
	AM_RANGE(0x0000, 0x1fff) AM_ROM
	AM_RANGE(0x2000, 0x7fff) AM_ROM AM_REGION("maincpu", 0x2000)
	AM_RANGE(0x8000, 0x8001) AM_DEVWRITE("ay1", ay8910_device, address_data_w)
	AM_RANGE(0x8001, 0x8001) AM_DEVREAD("ay1", ay8910_device, data_r)
	AM_RANGE(0x8002, 0x8003) AM_DEVWRITE("ay2", ay8910_device, address_data_w)
	AM_RANGE(0x8003, 0x8003) AM_DEVREAD("ay2", ay8910_device, data_r)
	AM_RANGE(0xa000, 0xa000) AM_READ_PORT("IN0")
	AM_RANGE(0xa001, 0xa001) AM_READ_PORT("IN1")
	AM_RANGE(0xa002, 0xa002) AM_READ_PORT("IN2") AM_WRITE(ninjakun_cpu2_io_A002_w)
	AM_RANGE(0xa003, 0xa003) AM_WRITE(nova2001_flipscreen_w)
	AM_RANGE(0xc000, 0xc7ff) AM_RAM_WRITE(nova2001_fg_videoram_w) AM_SHARE("fg_videoram")
	AM_RANGE(0xc800, 0xcfff) AM_READWRITE(ninjakun_bg_videoram_r, ninjakun_bg_videoram_w) AM_SHARE("bg_videoram")
	AM_RANGE(0xd000, 0xd7ff) AM_RAM AM_SHARE("spriteram")
	AM_RANGE(0xd800, 0xd9ff) AM_RAM_WRITE(ninjakun_paletteram_w) AM_SHARE("palette")
	AM_RANGE(0xe000, 0xe3ff) AM_RAM AM_SHARE("share2") /* swapped wrt CPU1 */
	AM_RANGE(0xe400, 0xe7ff) AM_RAM AM_SHARE("share1") /* swapped wrt CPU1 */
ADDRESS_MAP_END


static ADDRESS_MAP_START( pkunwar_map, AS_PROGRAM, 8, nova2001_state )
	AM_RANGE(0x0000, 0x7fff) AM_ROM
	AM_RANGE(0x8000, 0x87ff) AM_RAM AM_SHARE("spriteram")
	AM_RANGE(0x8800, 0x8fff) AM_RAM_WRITE(nova2001_bg_videoram_w) AM_SHARE("bg_videoram")
	AM_RANGE(0xa000, 0xa001) AM_DEVWRITE("ay1", ay8910_device, address_data_w)
	AM_RANGE(0xa001, 0xa001) AM_DEVREAD("ay1", ay8910_device, data_r)
	AM_RANGE(0xa002, 0xa003) AM_DEVWRITE("ay2", ay8910_device, address_data_w)
	AM_RANGE(0xa003, 0xa003) AM_DEVREAD("ay2", ay8910_device, data_r)
	AM_RANGE(0xc000, 0xc7ff) AM_RAM
	AM_RANGE(0xe000, 0xffff) AM_ROM
ADDRESS_MAP_END

static ADDRESS_MAP_START( pkunwar_io, AS_IO, 8, nova2001_state )
	ADDRESS_MAP_GLOBAL_MASK(0xff)
	AM_RANGE(0x00, 0x00) AM_WRITE(pkunwar_flipscreen_w)
ADDRESS_MAP_END


static ADDRESS_MAP_START( raiders5_cpu1_map, AS_PROGRAM, 8, nova2001_state )
	AM_RANGE(0x0000, 0x7fff) AM_ROM
	AM_RANGE(0x8000, 0x87ff) AM_RAM AM_SHARE("spriteram")
	AM_RANGE(0x8800, 0x8fff) AM_RAM_WRITE(nova2001_fg_videoram_w) AM_SHARE("fg_videoram")
	AM_RANGE(0x9000, 0x97ff) AM_READWRITE(ninjakun_bg_videoram_r, ninjakun_bg_videoram_w) AM_SHARE("bg_videoram")
	AM_RANGE(0xa000, 0xa000) AM_WRITE(nova2001_scroll_x_w)
	AM_RANGE(0xa001, 0xa001) AM_WRITE(nova2001_scroll_y_w)
	AM_RANGE(0xa002, 0xa002) AM_WRITE(pkunwar_flipscreen_w)
	AM_RANGE(0xc000, 0xc001) AM_DEVWRITE("ay1", ay8910_device, address_data_w)
	AM_RANGE(0xc001, 0xc001) AM_DEVREAD("ay1", ay8910_device, data_r)
	AM_RANGE(0xc002, 0xc003) AM_DEVWRITE("ay2", ay8910_device, address_data_w)
	AM_RANGE(0xc003, 0xc003) AM_DEVREAD("ay2", ay8910_device, data_r)
	AM_RANGE(0xd000, 0xd1ff) AM_RAM_WRITE(ninjakun_paletteram_w) AM_SHARE("palette")
	AM_RANGE(0xe000, 0xe7ff) AM_RAM AM_SHARE("share1")
ADDRESS_MAP_END

static ADDRESS_MAP_START( raiders5_cpu2_map, AS_PROGRAM, 8, nova2001_state )
	AM_RANGE(0x0000, 0x3fff) AM_ROM
	AM_RANGE(0x8000, 0x8001) AM_DEVWRITE("ay1", ay8910_device, address_data_w)
	AM_RANGE(0x8001, 0x8001) AM_DEVREAD("ay1", ay8910_device, data_r)
	AM_RANGE(0x8002, 0x8003) AM_DEVWRITE("ay2", ay8910_device, address_data_w)
	AM_RANGE(0x8003, 0x8003) AM_DEVREAD("ay2", ay8910_device, data_r)
	AM_RANGE(0x9000, 0x9000) AM_READNOP /* unknown */
	AM_RANGE(0xa000, 0xa7ff) AM_RAM AM_SHARE("share1")
	AM_RANGE(0xc000, 0xc000) AM_READNOP /* unknown */
	AM_RANGE(0xc800, 0xc800) AM_READNOP /* unknown */
	AM_RANGE(0xd000, 0xd000) AM_READNOP /* unknown */
	AM_RANGE(0xe000, 0xe000) AM_WRITE(nova2001_scroll_x_w)
	AM_RANGE(0xe001, 0xe001) AM_WRITE(nova2001_scroll_y_w)
	AM_RANGE(0xe002, 0xe002) AM_WRITE(pkunwar_flipscreen_w)
ADDRESS_MAP_END

static ADDRESS_MAP_START( raiders5_io, AS_IO, 8, nova2001_state )
	AM_RANGE(0x00, 0x00) AM_READNOP /* unknown */
ADDRESS_MAP_END



/*************************************
 *
 *  Port definitions
 *
 *************************************/

static INPUT_PORTS_START( nova2001 )
	PORT_START("IN0")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY
	PORT_BIT( 0x30, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_BUTTON2 )    // pause
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_BUTTON1 )    // fire

	PORT_START("IN1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY PORT_COCKTAIL
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY PORT_COCKTAIL
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY PORT_COCKTAIL
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_COCKTAIL
	PORT_BIT( 0x30, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_COCKTAIL
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_COCKTAIL

	PORT_START("IN2")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_COIN1 ) PORT_IMPULSE(4)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_START2 )
	PORT_BIT( 0x78, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_CUSTOM ) PORT_VBLANK("screen")

	PORT_START("DSW1")
	PORT_DIPNAME( 0x01, 0x00, DEF_STR( Cabinet ) ) PORT_DIPLOCATION("SW1:1")
	PORT_DIPSETTING(    0x00, DEF_STR( Upright ) )
	PORT_DIPSETTING(    0x01, DEF_STR( Cocktail ) )
	PORT_DIPNAME( 0x02, 0x02, DEF_STR( Lives ) ) PORT_DIPLOCATION("SW1:2")
	PORT_DIPSETTING(    0x02, "3" )
	PORT_DIPSETTING(    0x00, "4" )
	PORT_DIPNAME( 0x04, 0x04, "1st Bonus Life" ) PORT_DIPLOCATION("SW1:3")
	PORT_DIPSETTING(    0x04, "20K" )
	PORT_DIPSETTING(    0x00, "30K" )
	PORT_DIPNAME( 0x18, 0x18, "Extra Bonus Life" ) PORT_DIPLOCATION("SW1:4,5")
	PORT_DIPSETTING(    0x18, "60K" )
	PORT_DIPSETTING(    0x10, "70K" )
	PORT_DIPSETTING(    0x08, "90K" )
	PORT_DIPSETTING(    0x00, DEF_STR( None ) )
	PORT_DIPNAME( 0x60, 0x60, DEF_STR( Coinage ) ) PORT_DIPLOCATION("SW1:6,7")
	PORT_DIPSETTING(    0x40, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x00, DEF_STR( 2C_2C ) )
	PORT_DIPSETTING(    0x60, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x20, DEF_STR( 1C_2C ) )
	PORT_DIPNAME( 0x80, 0x80, DEF_STR( Free_Play ) ) PORT_DIPLOCATION("SW1:8")
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

	PORT_START("DSW2")
	PORT_DIPNAME( 0x03, 0x00, DEF_STR( Difficulty ) ) PORT_DIPLOCATION("SW2:1,2")
	PORT_DIPSETTING(    0x00, DEF_STR( Easy ) )
	PORT_DIPSETTING(    0x03, DEF_STR( Medium ) )
	PORT_DIPSETTING(    0x02, DEF_STR( Hard ) )
	PORT_DIPSETTING(    0x01, DEF_STR( Hardest ) )
	PORT_DIPNAME( 0x04, 0x00, DEF_STR( Demo_Sounds ) ) PORT_DIPLOCATION("SW2:3")
	PORT_DIPSETTING(    0x04, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x08, "High Score Names" ) PORT_DIPLOCATION("SW2:4")
	PORT_DIPSETTING(    0x00, "3 Letters" )
	PORT_DIPSETTING(    0x08, "8 Letters" )
	PORT_DIPUNUSED_DIPLOC( 0x10, IP_ACTIVE_LOW, "SW2:5" )
	PORT_DIPUNUSED_DIPLOC( 0x20, IP_ACTIVE_LOW, "SW2:6" )
	PORT_DIPUNUSED_DIPLOC( 0x40, IP_ACTIVE_LOW, "SW2:7" )
	PORT_SERVICE_DIPLOC( 0x80, IP_ACTIVE_LOW, "SW2:8" )
INPUT_PORTS_END

static INPUT_PORTS_START( ninjakun )
	PORT_START("IN0")   /* 0xa000 */
	PORT_BIT( 0x01, IP_ACTIVE_LOW,  IPT_JOYSTICK_LEFT ) PORT_2WAY /* "XPOS1" */
	PORT_BIT( 0x02, IP_ACTIVE_LOW,  IPT_JOYSTICK_RIGHT ) PORT_2WAY
	PORT_BIT( 0x04, IP_ACTIVE_LOW,  IPT_BUTTON2 )
	PORT_BIT( 0x08, IP_ACTIVE_LOW,  IPT_BUTTON1 )
	PORT_BIT( 0x10, IP_ACTIVE_LOW,  IPT_UNKNOWN )
	PORT_BIT( 0x20, IP_ACTIVE_LOW,  IPT_START1  )
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_UNKNOWN )

	PORT_START("IN1")   /* 0xa001 */
	PORT_BIT( 0x01, IP_ACTIVE_LOW,  IPT_JOYSTICK_LEFT) PORT_2WAY PORT_COCKTAIL /* "YPOS1" */
	PORT_BIT( 0x02, IP_ACTIVE_LOW,  IPT_JOYSTICK_RIGHT) PORT_2WAY PORT_COCKTAIL
	PORT_BIT( 0x04, IP_ACTIVE_LOW,  IPT_BUTTON2 ) PORT_COCKTAIL
	PORT_BIT( 0x08, IP_ACTIVE_LOW,  IPT_BUTTON1 ) PORT_COCKTAIL
	PORT_BIT( 0x10, IP_ACTIVE_LOW,  IPT_UNKNOWN )
	PORT_BIT( 0x20, IP_ACTIVE_LOW,  IPT_START2  )
	PORT_SERVICE( 0x40, IP_ACTIVE_HIGH )
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_COIN1 )

	PORT_START("IN2")   /* 0xa002 */
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_CUSTOM ) PORT_VBLANK("screen")
	PORT_BIT( 0x0c, IP_ACTIVE_HIGH, IPT_SPECIAL ) PORT_CUSTOM_MEMBER(DEVICE_SELF, nova2001_state,ninjakun_io_A002_ctrl_r, NULL)

	PORT_START("DSW1") // printed "SW 2"
	PORT_DIPNAME( 0x01, 0x00, DEF_STR( Cabinet ) )  PORT_DIPLOCATION("SW2:1")
	PORT_DIPSETTING(    0x00, DEF_STR( Upright ) )
	PORT_DIPSETTING(    0x01, DEF_STR( Cocktail ) )
	PORT_DIPNAME( 0x06, 0x04, DEF_STR( Lives ) )  PORT_DIPLOCATION("SW2:2,3")
	PORT_DIPSETTING(    0x02, "2" )
	PORT_DIPSETTING(    0x04, "3" ) // factory default = "3"
	PORT_DIPSETTING(    0x06, "4" )
	PORT_DIPSETTING(    0x00, "5" )
	PORT_DIPNAME( 0x08, 0x08, "First Bonus" )  PORT_DIPLOCATION("SW2:4")
	PORT_DIPSETTING(    0x08, "30000" ) // factory default = "30000"
	PORT_DIPSETTING(    0x00, "40000" )
	PORT_DIPNAME( 0x30, 0x20, "Second Bonus" )  PORT_DIPLOCATION("SW2:5,6")
	PORT_DIPSETTING(    0x00, "No Bonus" )
	PORT_DIPSETTING(    0x10, "Every 30000" )
	PORT_DIPSETTING(    0x30, "Every 50000" )
	PORT_DIPSETTING(    0x20, "Every 70000" ) // factory default = "70000"
	PORT_DIPNAME( 0x40, 0x00, DEF_STR( Demo_Sounds ) )  PORT_DIPLOCATION("SW2:7")
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, DEF_STR( Difficulty ) )  PORT_DIPLOCATION("SW2:8")
	PORT_DIPSETTING(    0x80, DEF_STR( Normal ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Hard )   )

	PORT_START("DSW2") // printed "SW 1"
	PORT_DIPNAME( 0x07, 0x07, DEF_STR( Coinage ) )  PORT_DIPLOCATION("SW1:1,2,3")
	PORT_DIPSETTING(    0x04, DEF_STR( 4C_1C ) )
	PORT_DIPSETTING(    0x05, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(    0x00, DEF_STR( 4C_2C ) )
	PORT_DIPSETTING(    0x06, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x01, DEF_STR( 3C_2C ) )
	PORT_DIPSETTING(    0x02, DEF_STR( 2C_2C ) )
	PORT_DIPSETTING(    0x07, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x03, DEF_STR( 1C_2C ) )
	PORT_DIPNAME( 0x08, 0x08, "High Score Names" )  PORT_DIPLOCATION("SW1:4")
	PORT_DIPSETTING(    0x00, "3 Letters" )
	PORT_DIPSETTING(    0x08, "8 Letters" )
	PORT_DIPNAME( 0x10, 0x00, DEF_STR( Allow_Continue ) )  PORT_DIPLOCATION("SW1:5")
	PORT_DIPSETTING(    0x10, DEF_STR( No ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Yes ) )
	PORT_DIPUNUSED_DIPLOC( 0x20, IP_ACTIVE_LOW, "SW1:6" )
	PORT_DIPNAME( 0x40, 0x40, DEF_STR( Free_Play ) )  PORT_DIPLOCATION("SW1:7")
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, "Infinite Lives (If Free Play)" )  PORT_DIPLOCATION("SW1:8")
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
INPUT_PORTS_END

static INPUT_PORTS_START( pkunwar )
	PORT_START("IN0")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_2WAY
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_2WAY
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_BUTTON1 )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_CUSTOM ) PORT_VBLANK("screen")

	PORT_START("IN1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_2WAY PORT_COCKTAIL
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_2WAY PORT_COCKTAIL
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_COCKTAIL
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_START2 )
	PORT_SERVICE( 0x40, IP_ACTIVE_LOW )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_COIN1 )

	PORT_START("IN2")
	PORT_DIPUNUSED_DIPLOC( 0x01, IP_ACTIVE_LOW, "SW1:1" )
	PORT_DIPUNUSED_DIPLOC( 0x02, IP_ACTIVE_LOW, "SW1:2" )
	PORT_DIPUNUSED_DIPLOC( 0x04, IP_ACTIVE_LOW, "SW1:3" )
	PORT_DIPUNUSED_DIPLOC( 0x08, IP_ACTIVE_LOW, "SW1:4" )
	PORT_DIPUNUSED_DIPLOC( 0x10, IP_ACTIVE_LOW, "SW1:5" )
	PORT_DIPUNUSED_DIPLOC( 0x20, IP_ACTIVE_LOW, "SW1:6" )
	PORT_DIPUNUSED_DIPLOC( 0x40, IP_ACTIVE_LOW, "SW1:7" )
	PORT_DIPUNUSED_DIPLOC( 0x80, IP_ACTIVE_LOW, "SW1:8" )

	PORT_START("DSW1")
	PORT_DIPNAME( 0x03, 0x03, DEF_STR( Coinage ) )  PORT_DIPLOCATION("SW2:1,2")
	PORT_DIPSETTING(    0x00, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(    0x02, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x03, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x01, DEF_STR( 1C_2C ) )
	PORT_DIPNAME( 0x04, 0x00, DEF_STR( Cabinet ) )  PORT_DIPLOCATION("SW2:3")
	PORT_DIPSETTING(    0x00, DEF_STR( Upright ) )
	PORT_DIPSETTING(    0x04, DEF_STR( Cocktail ) )
	PORT_DIPNAME( 0x08, 0x08, DEF_STR( Demo_Sounds ) )  PORT_DIPLOCATION("SW2:4")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x08, DEF_STR( On ) )
	PORT_DIPNAME( 0x30, 0x30, DEF_STR( Difficulty ) )  PORT_DIPLOCATION("SW2:5,6")
	PORT_DIPSETTING(    0x10, DEF_STR( Easy ) )
	PORT_DIPSETTING(    0x30, DEF_STR( Medium ) )
	PORT_DIPSETTING(    0x20, DEF_STR( Hard ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Hardest ) )
	PORT_DIPNAME( 0x40, 0x40, DEF_STR( Flip_Screen ) )  PORT_DIPLOCATION("SW2:7")
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, DEF_STR( Free_Play ) )  PORT_DIPLOCATION("SW2:8")
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
INPUT_PORTS_END

static INPUT_PORTS_START( raiders5 )
	PORT_START("IN0")
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_CUSTOM ) PORT_VBLANK("screen")
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON1 )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_4WAY
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_4WAY
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_4WAY
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_4WAY

	PORT_START("IN1")
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_COIN1 ) PORT_IMPULSE(5)
	PORT_SERVICE( 0x40, IP_ACTIVE_LOW )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_START2 )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_COCKTAIL
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_4WAY PORT_COCKTAIL
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_4WAY PORT_COCKTAIL
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_4WAY PORT_COCKTAIL
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_4WAY PORT_COCKTAIL

	PORT_START("IN2")
	PORT_DIPNAME( 0x01, 0x00, DEF_STR( Cabinet ) )  PORT_DIPLOCATION("SW1:1")
	PORT_DIPSETTING(    0x00, DEF_STR( Upright ) )
	PORT_DIPSETTING(    0x01, DEF_STR( Cocktail ) )
	PORT_DIPNAME( 0x06, 0x06, DEF_STR( Lives ) )  PORT_DIPLOCATION("SW1:2,3")
	PORT_DIPSETTING(    0x00, "2" )
	PORT_DIPSETTING(    0x06, "3" )
	PORT_DIPSETTING(    0x04, "4" )
	PORT_DIPSETTING(    0x02, "5" )
	PORT_DIPNAME( 0x08, 0x08, "1st Bonus" )  PORT_DIPLOCATION("SW1:4")
	PORT_DIPSETTING(    0x08, "30000" )
	PORT_DIPSETTING(    0x00, "40000" )
	PORT_DIPNAME( 0x30, 0x30, "2nd Bonus" )  PORT_DIPLOCATION("SW1:5,6")
	PORT_DIPSETTING(    0x30, "Every 50000" )
	PORT_DIPSETTING(    0x20, "Every 70000" )
	PORT_DIPSETTING(    0x10, "Every 90000" )
	PORT_DIPSETTING(    0x00, DEF_STR( None ) )
	PORT_DIPNAME( 0x40, 0x40, "Exercise" )  PORT_DIPLOCATION("SW1:7")  // Unused in manual
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x40, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, DEF_STR( Difficulty ) )  PORT_DIPLOCATION("SW1:8")
	PORT_DIPSETTING(    0x80, DEF_STR( Normal ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Hard ) )

	PORT_START("DSW1")
	PORT_DIPNAME( 0x07, 0x07, DEF_STR( Coinage ) )  PORT_DIPLOCATION("SW2:1,2,3")
	PORT_DIPSETTING(    0x04, DEF_STR( 4C_1C ) )
	PORT_DIPSETTING(    0x05, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(    0x00, DEF_STR( 4C_2C ) )
	PORT_DIPSETTING(    0x06, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x01, DEF_STR( 3C_2C ) )
	PORT_DIPSETTING(    0x02, DEF_STR( 2C_2C ) )
	PORT_DIPSETTING(    0x07, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x03, DEF_STR( 1C_2C ) )
	PORT_DIPNAME( 0x08, 0x08, "High Score Names" )  PORT_DIPLOCATION("SW2:4")
	PORT_DIPSETTING(    0x00, "3 Letters" )
	PORT_DIPSETTING(    0x08, "8 Letters" )
	PORT_DIPNAME( 0x10, 0x10, DEF_STR( Allow_Continue ) )  PORT_DIPLOCATION("SW2:5")
	PORT_DIPSETTING(    0x00, DEF_STR( No ) )
	PORT_DIPSETTING(    0x10, DEF_STR( Yes ) )
	PORT_DIPUNUSED_DIPLOC( 0x20, IP_ACTIVE_LOW, "SW2:6" )
	PORT_DIPNAME( 0x40, 0x40, DEF_STR( Free_Play ) )  PORT_DIPLOCATION("SW2:7") // Unused in manual
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, "Unlimited Lives (If Free Play)" )  PORT_DIPLOCATION("SW2:8")  // Unused in manual
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
INPUT_PORTS_END



/*************************************
 *
 *  Graphics layouts
 *
 *************************************/

static const gfx_layout layout8x8 =
{
	8,8,
	RGN_FRAC(1,1),
	4,
	{ STEP4(0,1) },
	{ STEP8(0,4) },
	{ STEP8(0,32) },
	32*8
};

static const gfx_layout layout8x8_part =
{
	8,8,
	RGN_FRAC(1,4),
	4,
	{ STEP4(0,1) },
	{ STEP8(0,4) },
	{ STEP8(0,32) },
	32*8
};

static const gfx_layout layout16x16 =
{
	16,16,
	RGN_FRAC(1,1),
	4,
	{ STEP4(0,1) },
	{ STEP8(0,4), STEP8(32*8,4) },
	{ STEP8(0,32), STEP8(64*8,32) },
	128*8
};

static GFXDECODE_START( nova2001 )
	GFXDECODE_ENTRY( "gfx1", 0x0000, layout16x16,    0x000, 16 )    // sprites
	GFXDECODE_ENTRY( "gfx1", 0x0000, layout8x8_part, 0x000, 16 )    // fg tiles (using only 1/4th of the ROM space)
	GFXDECODE_ENTRY( "gfx1", 0x4000, layout8x8_part, 0x100, 16 )    // bg tiles (using only 1/4th of the ROM space)
GFXDECODE_END

static GFXDECODE_START( ninjakun )
	GFXDECODE_ENTRY( "gfx1", 0, layout16x16, 0x200, 16 )    // sprites
	GFXDECODE_ENTRY( "gfx1", 0, layout8x8,   0x000, 16 )    // fg tiles
	GFXDECODE_ENTRY( "gfx2", 0, layout8x8,   0x100, 16 )    // bg tiles
GFXDECODE_END

static GFXDECODE_START( pkunwar )
	GFXDECODE_ENTRY( "gfx1", 0, layout16x16, 0x000, 16 )    // sprites
	GFXDECODE_ENTRY( "gfx1", 0, layout8x8,   0x100, 16 )    // bg tiles
GFXDECODE_END

static GFXDECODE_START( raiders5 )
	GFXDECODE_ENTRY( "gfx1", 0, layout16x16,    0x200, 16 ) // sprites
	GFXDECODE_ENTRY( "gfx1", 0, layout8x8_part, 0x000, 16 ) // fg tiles (using only 1/4th of the ROM space)
	GFXDECODE_ENTRY( "gfx2", 0, layout8x8,      0x100, 16 ) // bg tiles
GFXDECODE_END


/*************************************
 *
 *  Machine drivers
 *
 *************************************/

static MACHINE_CONFIG_START( nova2001, nova2001_state )

	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", Z80, MAIN_CLOCK/4)  // 3 MHz verified on schematics
	MCFG_CPU_PROGRAM_MAP(nova2001_map)
	MCFG_CPU_VBLANK_INT_DRIVER("screen", nova2001_state,  irq0_line_hold)

	/* video hardware */
	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_REFRESH_RATE(60)
	MCFG_SCREEN_SIZE(32*8, 32*8)
	MCFG_SCREEN_VISIBLE_AREA(0*8, 32*8-1, 4*8, 28*8-1)
	MCFG_SCREEN_UPDATE_DRIVER(nova2001_state, screen_update_nova2001)
	MCFG_SCREEN_PALETTE("palette")

	MCFG_GFXDECODE_ADD("gfxdecode", "palette", nova2001)
	MCFG_PALETTE_ADD("palette", 512)
	MCFG_PALETTE_FORMAT_CLASS(1, nova2001_state, BBGGRRII)

	MCFG_PALETTE_INIT_OWNER(nova2001_state,nova2001)
	MCFG_VIDEO_START_OVERRIDE(nova2001_state,nova2001)

	/* sound hardware */
	MCFG_SPEAKER_STANDARD_MONO("mono")

	MCFG_SOUND_ADD("ay1", AY8910, MAIN_CLOCK/6) // 2 MHz verified on schematics
	MCFG_AY8910_PORT_A_WRITE_CB(WRITE8(nova2001_state, nova2001_scroll_x_w))
	MCFG_AY8910_PORT_B_WRITE_CB(WRITE8(nova2001_state, nova2001_scroll_y_w))
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.25)

	MCFG_SOUND_ADD("ay2", AY8910, MAIN_CLOCK/6)
	MCFG_AY8910_PORT_A_READ_CB(IOPORT("DSW1"))
	MCFG_AY8910_PORT_B_READ_CB(IOPORT("DSW2"))
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.25)
MACHINE_CONFIG_END

static MACHINE_CONFIG_START( ninjakun, nova2001_state )

	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", Z80, MAIN_CLOCK/4)  // 3 MHz
	MCFG_CPU_PROGRAM_MAP(ninjakun_cpu1_map)
	MCFG_CPU_VBLANK_INT_DRIVER("screen", nova2001_state,  irq0_line_hold)

	MCFG_CPU_ADD("sub", Z80, MAIN_CLOCK/4)  // 3 MHz
	MCFG_CPU_PROGRAM_MAP(ninjakun_cpu2_map)
	MCFG_CPU_PERIODIC_INT_DRIVER(nova2001_state, irq0_line_hold, 4*60) /* ? */

	MCFG_QUANTUM_TIME(attotime::from_hz(6000))  /* 100 CPU slices per frame */

	MCFG_MACHINE_START_OVERRIDE(nova2001_state,ninjakun)

	/* video hardware */

	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_REFRESH_RATE(60)
	MCFG_SCREEN_SIZE(32*8, 32*8)
	MCFG_SCREEN_VISIBLE_AREA(0*8, 32*8-1, 4*8, 28*8-1 )
	MCFG_SCREEN_UPDATE_DRIVER(nova2001_state, screen_update_ninjakun)
	MCFG_SCREEN_PALETTE("palette")

	MCFG_GFXDECODE_ADD("gfxdecode", "palette", ninjakun)
	MCFG_PALETTE_ADD("palette", 768)
	MCFG_PALETTE_FORMAT_CLASS(1, nova2001_state, BBGGRRII)

	MCFG_VIDEO_START_OVERRIDE(nova2001_state,ninjakun)

	/* sound hardware */
	MCFG_SPEAKER_STANDARD_MONO("mono")

	MCFG_SOUND_ADD("ay1", AY8910, MAIN_CLOCK/4) // 3 MHz
	MCFG_AY8910_PORT_A_READ_CB(IOPORT("DSW1"))
	MCFG_AY8910_PORT_B_READ_CB(IOPORT("DSW2"))
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.20)

	MCFG_SOUND_ADD("ay2", AY8910, MAIN_CLOCK/4) // 3 MHz
	MCFG_AY8910_PORT_A_WRITE_CB(WRITE8(nova2001_state, nova2001_scroll_x_w))
	MCFG_AY8910_PORT_B_WRITE_CB(WRITE8(nova2001_state, nova2001_scroll_y_w))
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.20)
MACHINE_CONFIG_END

static MACHINE_CONFIG_START( pkunwar, nova2001_state )

	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", Z80, MAIN_CLOCK/4)  // 3 MHz
	MCFG_CPU_PROGRAM_MAP(pkunwar_map)
	MCFG_CPU_IO_MAP(pkunwar_io)
	MCFG_CPU_VBLANK_INT_DRIVER("screen", nova2001_state,  irq0_line_hold)

	/* video hardware */
	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_REFRESH_RATE(60)
	MCFG_SCREEN_SIZE(32*8, 32*8)
	MCFG_SCREEN_VISIBLE_AREA(0*8, 32*8-1, 4*8, 28*8-1)
	MCFG_SCREEN_UPDATE_DRIVER(nova2001_state, screen_update_pkunwar)
	MCFG_SCREEN_PALETTE("palette")

	MCFG_GFXDECODE_ADD("gfxdecode", "palette", pkunwar)
	MCFG_PALETTE_ADD("palette", 512)
	MCFG_PALETTE_FORMAT_CLASS(1, nova2001_state, BBGGRRII)

	MCFG_PALETTE_INIT_OWNER(nova2001_state,nova2001)
	MCFG_VIDEO_START_OVERRIDE(nova2001_state,pkunwar)

	/* sound hardware */
	MCFG_SPEAKER_STANDARD_MONO("mono")

	MCFG_SOUND_ADD("ay1", AY8910, MAIN_CLOCK/8) // 1.5MHz (correct?)
	MCFG_AY8910_PORT_A_READ_CB(IOPORT("IN0"))
	MCFG_AY8910_PORT_B_READ_CB(IOPORT("IN1"))
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.25)

	MCFG_SOUND_ADD("ay2", AY8910, MAIN_CLOCK/8)
	MCFG_AY8910_PORT_A_READ_CB(IOPORT("IN2"))
	MCFG_AY8910_PORT_B_READ_CB(IOPORT("DSW1"))
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.25)
MACHINE_CONFIG_END

static MACHINE_CONFIG_START( raiders5, nova2001_state )

	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", Z80, MAIN_CLOCK/4)  // 3 MHz
	MCFG_CPU_PROGRAM_MAP(raiders5_cpu1_map)
	MCFG_CPU_IO_MAP(raiders5_io)
	MCFG_CPU_VBLANK_INT_DRIVER("screen", nova2001_state,  irq0_line_hold)

	MCFG_CPU_ADD("sub", Z80, MAIN_CLOCK/4)  // 3 MHz
	MCFG_CPU_PROGRAM_MAP(raiders5_cpu2_map)
	MCFG_CPU_PERIODIC_INT_DRIVER(nova2001_state, irq0_line_hold, 4*60)  /* ? */

	MCFG_QUANTUM_TIME(attotime::from_hz(24000))

	/* video hardware */
	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_REFRESH_RATE(60)
	MCFG_SCREEN_SIZE(32*8, 32*8)
	MCFG_SCREEN_VISIBLE_AREA(0*8, 32*8-1, 4*8, 28*8-1)
	MCFG_SCREEN_UPDATE_DRIVER(nova2001_state, screen_update_raiders5)
	MCFG_SCREEN_PALETTE("palette")

	MCFG_GFXDECODE_ADD("gfxdecode", "palette", raiders5)
	MCFG_PALETTE_ADD("palette", 768)
	MCFG_PALETTE_FORMAT_CLASS(1, nova2001_state, BBGGRRII)

	MCFG_VIDEO_START_OVERRIDE(nova2001_state,raiders5)

	/* sound hardware */
	MCFG_SPEAKER_STANDARD_MONO("mono")

	MCFG_SOUND_ADD("ay1", AY8910, MAIN_CLOCK/8) // 1.5MHz
	MCFG_AY8910_PORT_A_READ_CB(IOPORT("IN0"))
	MCFG_AY8910_PORT_B_READ_CB(IOPORT("IN1"))
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.25)

	MCFG_SOUND_ADD("ay2", AY8910, MAIN_CLOCK/8)
	MCFG_AY8910_PORT_A_READ_CB(IOPORT("IN2"))
	MCFG_AY8910_PORT_B_READ_CB(IOPORT("DSW1"))
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.25)
MACHINE_CONFIG_END



/*************************************
 *
 *  ROM definitions
 *
 *************************************/

ROM_START( nova2001 )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "1.6c",         0x0000, 0x2000, CRC(368cffc0) SHA1(b756c0542d5b86640af62639bdd0d32f6e364dd3) )
	ROM_LOAD( "2.6d",         0x2000, 0x2000, CRC(bc4e442b) SHA1(6e1dca5dde442db95403377bf49aaad2a337813e) )
	ROM_LOAD( "3.6f",         0x4000, 0x2000, CRC(b2849038) SHA1(b56c7c03ef7c677cc6df0280a485f9cda3435b23) )
	ROM_LOAD( "4.6g",         0x6000, 0x1000, CRC(6b5bb12d) SHA1(74aee3d08a7ee1f98eaec4a4b3062aa9d17948ec) )
	ROM_RELOAD(               0x7000, 0x1000 )  // half size ROM, mirrored

	ROM_REGION( 0x8000, "gfx1", 0 )
	ROM_LOAD16_BYTE( "5.12s",        0x0000, 0x2000, CRC(54198941) SHA1(fe762a0bbcf10b13ece87ded2ea730257cfbe7d3) )
	ROM_LOAD16_BYTE( "6.12p",        0x0001, 0x2000, CRC(cbd90dca) SHA1(7eacde832f5783f4389fb98d6bf6b26dd494665d) )
	ROM_LOAD16_BYTE( "7.12n",        0x4000, 0x2000, CRC(9ebd8806) SHA1(26b6caa0d0a7ae52a182070ecc7bc696c12038b3) )
	ROM_LOAD16_BYTE( "8.12l",        0x4001, 0x2000, CRC(d1b18389) SHA1(2d808fee774f1bb4cec42e23cfef36b54eee0efa) )

	ROM_REGION( 0x0020, "proms", 0 )
	ROM_LOAD( "nova2001.clr", 0x0000, 0x0020, CRC(a2fac5cd) SHA1(ad14aa2be57722d1f48b47171fe72f96091423b6) )
ROM_END

ROM_START( nova2001h )
	ROM_REGION( 0x10000, "maincpu", 0 )
	// roms 1 and 2 had green stickers, but looks like an unofficial mod, bytes have been added in empty space to fix game checksum after mods were made to code.
	// one of the mods fixes the game resetting if the coin input is held down for too short / long of a period, the purpose of the other is unknown.
	ROM_LOAD( "1(green).6c",         0x0000, 0x2000, CRC(1a8731b3) SHA1(a865d1cb070686dfa19e0da887c599455692a860) )
	ROM_LOAD( "2(green).6d",         0x2000, 0x2000, CRC(bc4e442b) SHA1(6e1dca5dde442db95403377bf49aaad2a337813e) ) // not actually modified?
	ROM_LOAD( "3.6f",         0x4000, 0x2000, CRC(b2849038) SHA1(b56c7c03ef7c677cc6df0280a485f9cda3435b23) )
	ROM_LOAD( "4.6g",         0x6000, 0x1000, CRC(6b5bb12d) SHA1(74aee3d08a7ee1f98eaec4a4b3062aa9d17948ec) )
	ROM_RELOAD(               0x7000, 0x1000 )  // half size ROM, mirrored

	ROM_REGION( 0x8000, "gfx1", 0 )
	ROM_LOAD16_BYTE( "5.12s",        0x0000, 0x2000, CRC(54198941) SHA1(fe762a0bbcf10b13ece87ded2ea730257cfbe7d3) )
	ROM_LOAD16_BYTE( "6.12p",        0x0001, 0x2000, CRC(cbd90dca) SHA1(7eacde832f5783f4389fb98d6bf6b26dd494665d) )
	ROM_LOAD16_BYTE( "7.12n",        0x4000, 0x2000, CRC(9ebd8806) SHA1(26b6caa0d0a7ae52a182070ecc7bc696c12038b3) )
	ROM_LOAD16_BYTE( "8.12l",        0x4001, 0x2000, CRC(d1b18389) SHA1(2d808fee774f1bb4cec42e23cfef36b54eee0efa) )

	ROM_REGION( 0x0020, "proms", 0 )
	ROM_LOAD( "nova2001.clr", 0x0000, 0x0020, CRC(a2fac5cd) SHA1(ad14aa2be57722d1f48b47171fe72f96091423b6) )
ROM_END

ROM_START( nova2001u )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "nova2001.1",   0x0000, 0x2000, CRC(b79461bd) SHA1(7fac3313bc76612f66a6518450d0fed32fe70c45) )
	ROM_LOAD( "nova2001.2",   0x2000, 0x2000, CRC(fab87144) SHA1(506703f9d96443839f864ef5bde1a71120f54384) )
	ROM_LOAD( "3.6f",         0x4000, 0x2000, CRC(b2849038) SHA1(b56c7c03ef7c677cc6df0280a485f9cda3435b23) )
	ROM_LOAD( "4.6g",         0x6000, 0x1000, CRC(6b5bb12d) SHA1(74aee3d08a7ee1f98eaec4a4b3062aa9d17948ec) )
	ROM_RELOAD(               0x7000, 0x1000 )  // half size ROM, mirrored

	ROM_REGION( 0x8000, "gfx1", 0 )
	ROM_LOAD16_BYTE( "nova2001.5",   0x0000, 0x2000, CRC(8ea576e8) SHA1(d8dbcfd43aafe25afad7f947a80737cdc55b23d7) )
	ROM_LOAD16_BYTE( "nova2001.6",   0x0001, 0x2000, CRC(0c61656c) SHA1(41c480799798c95543b5a805694e68282b9f563a) )
	ROM_LOAD16_BYTE( "7.12n",        0x4000, 0x2000, CRC(9ebd8806) SHA1(26b6caa0d0a7ae52a182070ecc7bc696c12038b3) )
	ROM_LOAD16_BYTE( "8.12l",        0x4001, 0x2000, CRC(d1b18389) SHA1(2d808fee774f1bb4cec42e23cfef36b54eee0efa) )

	ROM_REGION( 0x0020, "proms", 0 )
	ROM_LOAD( "nova2001.clr", 0x0000, 0x0020, CRC(a2fac5cd) SHA1(ad14aa2be57722d1f48b47171fe72f96091423b6) )
ROM_END

ROM_START( ninjakun ) /* Original Board? */
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "ninja-1.7a",  0x0000, 0x02000, CRC(1c1dc141) SHA1(423d3ed35e73a8d5bfce075a889b0322b207bd0d) )
	ROM_LOAD( "ninja-2.7b",  0x2000, 0x02000, CRC(39cc7d37) SHA1(7f0d0e1e92cb6a57f15eb7fc51a67112f1c5fc8e) )
	ROM_LOAD( "ninja-3.7d",  0x4000, 0x02000, CRC(d542bfe3) SHA1(3814d8f5b1acda21438fff4f71670fa653dc7b30) )
	ROM_LOAD( "ninja-4.7e",  0x6000, 0x02000, CRC(a57385c6) SHA1(77925a281e64889bfe967c3d42a388529aaf7eb6) )

	ROM_REGION( 0x10000, "sub", 0 )
	ROM_LOAD( "ninja-5.7h",  0x0000, 0x02000, CRC(164a42c4) SHA1(16b434b33b76b878514f67c23315d4c6da7bfc9e) )

	ROM_REGION( 0x08000, "gfx1", 0 )
	ROM_LOAD16_BYTE( "ninja-6.7n",  0x0000, 0x02000, CRC(a74c4297) SHA1(87184d14c67331f2c8a2412e28f31427eddae799) )
	ROM_LOAD16_BYTE( "ninja-7.7p",  0x0001, 0x02000, CRC(53a72039) SHA1(d77d608ce9388a8956831369badd88a8eda8e102) )
	ROM_LOAD16_BYTE( "ninja-8.7s",  0x4000, 0x02000, CRC(4a99d857) SHA1(6aadb6a5c721a161a5c1bef5569c1e323e380cff) )
	ROM_LOAD16_BYTE( "ninja-9.7t",  0x4001, 0x02000, CRC(dede49e4) SHA1(8ce4bc02ec583b3885ca63fb5e2d5dad185fe192) )

	ROM_REGION( 0x08000, "gfx2", 0 )
	ROM_LOAD16_BYTE( "ninja-10.2c", 0x0000, 0x02000, CRC(0d55664a) SHA1(955a607b4401ce9f3f807d53833a766152b0ef9b) )
	ROM_LOAD16_BYTE( "ninja-11.2d", 0x0001, 0x02000, CRC(12ff9597) SHA1(10b572844ab32e3ae54abe3600fecc1a811ac713) )
	ROM_LOAD16_BYTE( "ninja-12.4c", 0x4000, 0x02000, CRC(e9b75807) SHA1(cf4c8ac962f785e9de5502df58eab9b3725aaa28) )
	ROM_LOAD16_BYTE( "ninja-13.4d", 0x4001, 0x02000, CRC(1760ed2c) SHA1(ee4c8efcce483c8051873714856824a1a1e14b61) )
ROM_END

ROM_START( pkunwar )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "pkwar.01r",    0x0000, 0x4000, CRC(ce2d2c7b) SHA1(2ffe2eb339fd668ec4fe90eff66124a334db0693) )
	ROM_LOAD( "pkwar.02r",    0x4000, 0x4000, CRC(abc1f661) SHA1(c4bf4a345efd4271617de9f334303d81c6885aa5) )
	ROM_LOAD( "pkwar.03r",    0xe000, 0x2000, CRC(56faebea) SHA1(dd0406c723a08f5d1120655857a115ab8c2d2a11) )

	ROM_REGION( 0x10000, "gfx1", 0 )    // (need lineswapping)
	ROM_LOAD( "pkwar.01y",    0x0000, 0x4000, CRC(428d3b92) SHA1(7fe11e8d785fe829d34e512f233bb9ccc70cd431) )
	ROM_LOAD( "pkwar.02y",    0x4000, 0x4000, CRC(ce1da7bc) SHA1(a2357b61703a689ce63aec7dd44702b119894f8e) )
	ROM_LOAD( "pkwar.03y",    0x8000, 0x4000, CRC(63204400) SHA1(1ba87ad3425c51150cb65408f04ee0147ef332d3) )
	ROM_LOAD( "pkwar.04y",    0xc000, 0x4000, CRC(061dfca8) SHA1(0a2dd8fc790d607195ca18dfc55575c2b9ddc58a) )

	ROM_REGION( 0x0020, "proms", 0 )
	ROM_LOAD( "pkwar.col",    0x0000, 0x0020, CRC(af0fc5e2) SHA1(480908bf893211b580ae19cfb40dc35ad1bbc343) )
ROM_END

ROM_START( pkunwarj )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "pgunwar.6",    0x0000, 0x4000, CRC(357f3ef3) SHA1(bc651fb7701b395ae8cda1888814af5c5aa325a6) )
	ROM_LOAD( "pgunwar.5",    0x4000, 0x4000, CRC(0092e49e) SHA1(7945361036f7679e4f4bb6b94f60f3ca09c077dc) )
	ROM_LOAD( "pkwar.03r",    0xe000, 0x2000, CRC(56faebea) SHA1(dd0406c723a08f5d1120655857a115ab8c2d2a11) )

	ROM_REGION( 0x10000, "gfx1", 0 )    // (need lineswapping)
	ROM_LOAD( "pkwar.01y",    0x0000, 0x4000, CRC(428d3b92) SHA1(7fe11e8d785fe829d34e512f233bb9ccc70cd431) )
	ROM_LOAD( "pkwar.02y",    0x4000, 0x4000, CRC(ce1da7bc) SHA1(a2357b61703a689ce63aec7dd44702b119894f8e) )
	ROM_LOAD( "pgunwar.2",    0x8000, 0x4000, CRC(a2a43443) SHA1(4e10569886d364eb2539928ea81dc1565b60b590) )
	ROM_LOAD( "pkwar.04y",    0xc000, 0x4000, CRC(061dfca8) SHA1(0a2dd8fc790d607195ca18dfc55575c2b9ddc58a) )

	ROM_REGION( 0x0020, "proms", 0 )
	ROM_LOAD( "pkwar.col",    0x0000, 0x0020, CRC(af0fc5e2) SHA1(480908bf893211b580ae19cfb40dc35ad1bbc343) )
ROM_END

ROM_START( raiders5 )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "raiders5.1", 0x0000,  0x4000, CRC(47cea11f) SHA1(0499e6627ad9c16775fdc59f2ff56dfdfc23490a) )
	ROM_LOAD( "raiders5.2", 0x4000,  0x4000, CRC(eb2ff410) SHA1(5c995b66b6301cd3cd58efd173481deaa036f842) )

	ROM_REGION( 0x10000, "sub", 0 )
	ROM_LOAD( "raiders5.2", 0x0000,  0x4000, CRC(eb2ff410) SHA1(5c995b66b6301cd3cd58efd173481deaa036f842) )

	ROM_REGION( 0x8000, "gfx1", 0 ) // (need lineswapping)
	ROM_LOAD( "raiders3.11f", 0x0000,  0x4000, CRC(30041d58) SHA1(a33087de7afb276925879898a96f418128a5a38c) )
	ROM_LOAD( "raiders4.11g", 0x4000,  0x4000, CRC(e441931c) SHA1(f39b4c25de779c671a6e2b02df64e7fed726f4da) )

	ROM_REGION( 0x4000, "gfx2", 0 ) // (need lineswapping)
	ROM_LOAD( "raiders5.11n", 0x0000,  0x4000, CRC(c0895090) SHA1(a3a1ae57ed66bc095ea9bfb26470290f67aab1fe) )
ROM_END

ROM_START( raiders5t )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "raiders1.4c", 0x0000,  0x4000, CRC(4e2d5679) SHA1(a1c1603ba98814a83b92ad024ca4422aea872111) )
	ROM_LOAD( "raiders2.4d", 0x4000,  0x4000, CRC(c8604be1) SHA1(6d23f26174bb9b2f7db3a5fa6b39674fe237135b) )

	ROM_REGION( 0x10000, "sub", 0 )
	ROM_LOAD( "raiders2.4d", 0x0000,  0x4000, CRC(c8604be1) SHA1(6d23f26174bb9b2f7db3a5fa6b39674fe237135b) )

	ROM_REGION( 0x8000, "gfx1", 0 ) // (need lineswapping)
	ROM_LOAD( "raiders3.11f", 0x0000,  0x4000, CRC(30041d58) SHA1(a33087de7afb276925879898a96f418128a5a38c) )
	ROM_LOAD( "raiders4.11g", 0x4000,  0x4000, CRC(e441931c) SHA1(f39b4c25de779c671a6e2b02df64e7fed726f4da) )

	ROM_REGION( 0x4000, "gfx2", 0 ) // (need lineswapping)
	ROM_LOAD( "raiders5.11n", 0x0000,  0x4000, CRC(c0895090) SHA1(a3a1ae57ed66bc095ea9bfb26470290f67aab1fe) )
ROM_END



/*************************************
 *
 *  Gfx ROM swizzling
 *
 *************************************/

/******************************************************************************

Gfx ROMs in pkunwar have an unusual layout, where a high address bit
(which is not the top bit) separates parts of the same tile.

This all originates from Nova2001 apparently, which uses 0x2000 bytes ROMs for
the graphics. When the number of tiles was increased, the same 0x2000 blocks
were maintained even if the ROMs got larger.

To make it possible to decode graphics without resorting to ROM_CONTINUE
trickery, this function makes an address line rotation, bringing bit "bit" to
bit 0 and shifting left by one place all the intervening bits.

This code is overly generic because it is used for several games in ninjakd2.c

******************************************************************************/

void nova2001_state::lineswap_gfx_roms(const char *region, const int bit)
{
	const int length = memregion(region)->bytes();

	UINT8* const src = memregion(region)->base();

	dynamic_buffer temp(length);

	const int mask = (1 << (bit + 1)) - 1;

	int sa;

	for (sa = 0; sa < length; sa++)
	{
		const int da = (sa & ~mask) | ((sa << 1) & mask) | ((sa >> bit) & 1);

		temp[da] = src[sa];
	}

	memcpy(src, &temp[0], length);
}



/*************************************
 *
 *  Driver initialization
 *
 *************************************/

DRIVER_INIT_MEMBER(nova2001_state,pkunwar)
{
	lineswap_gfx_roms("gfx1", 13);
}

DRIVER_INIT_MEMBER(nova2001_state,raiders5)
{
	lineswap_gfx_roms("gfx1", 13);
	lineswap_gfx_roms("gfx2", 13);
}



/*************************************
 *
 *  Game drivers
 *
 *************************************/

// many of these don't explicitly state Japan, eg. Nova 2001 could easily be used anywhere.

//    YEAR, NAME,      PARENT,   MACHINE,  INPUT,    INIT,     MONITOR,COMPANY,FULLNAME,FLAGS
GAME( 1983, nova2001,  0,        nova2001, nova2001, driver_device, 0,        ROT0,   "UPL", "Nova 2001 (Japan)", MACHINE_SUPPORTS_SAVE )
GAME( 1983, nova2001h, nova2001, nova2001, nova2001, driver_device, 0,        ROT0,   "UPL", "Nova 2001 (Japan, hack?)", MACHINE_SUPPORTS_SAVE )
GAME( 1983, nova2001u, nova2001, nova2001, nova2001, driver_device, 0,        ROT0,   "UPL (Universal license)", "Nova 2001 (US)", MACHINE_SUPPORTS_SAVE )
GAME( 1984, ninjakun,  0,        ninjakun, ninjakun, driver_device, 0,        ROT0,   "UPL (Taito license)", "Ninjakun Majou no Bouken", MACHINE_SUPPORTS_SAVE )
GAME( 1985, pkunwar,   0,        pkunwar,  pkunwar, nova2001_state,  pkunwar,  ROT0,   "UPL", "Penguin-Kun Wars (US)", MACHINE_SUPPORTS_SAVE )
GAME( 1985, pkunwarj,  pkunwar,  pkunwar,  pkunwar, nova2001_state,  pkunwar,  ROT0,   "UPL", "Penguin-Kun Wars (Japan)", MACHINE_SUPPORTS_SAVE )
GAME( 1985, raiders5,  0,        raiders5, raiders5, nova2001_state, raiders5, ROT0,   "UPL", "Raiders5", MACHINE_SUPPORTS_SAVE )
GAME( 1985, raiders5t, raiders5, raiders5, raiders5, nova2001_state, raiders5, ROT0,   "UPL (Taito license)", "Raiders5 (Japan)", MACHINE_SUPPORTS_SAVE )
