// license:BSD-3-Clause
// copyright-holders:Carlos A. Lozano
/**************************************************************************

Blood Bros, West Story & Sky Smasher
TAD Corporation 1990
68000 + Z80 + YM3931 + YM3812

driver by Carlos A. Lozano Baides

TODO:
West Story:
- sound (program rom is close to the original)
- some bad sprites (2 bad sprite roms, should the actual decoded data match?, can they be reconstructed from the original?)
- tilemap scroll
- runs too fast? (vblank flag somewhere?)


Blood Bros  (c) 1990 Nihon System [Seibu hardware]
-----------
The manual states:

 At Power On, if you touch the joystick, you will have DIP-SW setting on
 the screen

This works for all sets and the bootleg.

Sky Smasher  (c) 1990 Nihon System [Seibu hardware]
-----------

Like some other Seibu hardware games, hold P1 right at boot to
view DIP descriptions. The manual states:

 At Power On, if you touch the joystick or SHOOT or MISSILE button, you will
 have DIP-SW setting on the screen

Game does not appear to have cocktail mode. The screen hardware
is undoubtedly capable of flipscreen and layer priority flipping
however.(which is why we have MACHINE_NO_COCKTAIL despite the games
being upright)

Dumpers Notes
=============

PCB is made by Seibu

Sound           - Z80
                - YM3931 SEI0100BU    (64 pin DIP)
                - YM3812
                - M6295

GFX             - SEI0210   custom    (128 pin PQFP)
                - SEI0220BP custom    (80 pin PQFP)
                - SEI0200   custom    (100 pin PQFP)
                - SEI0160   custom    (60 pin PQFP)


Stephh's notes (based on the games M68000 code and some tests) :

1) 'bloodbro'

  - When "Starting Coin" Dip Switch is set to "x2", you need 2 coins to start
    a game (but 1 coin to join), then 1 coin to continue.
    However, it you insert 2 coins, and press START2, you start a 2 players game.
    Also note that when "Starting Coin" Dip Switch is set to "x2", SERVICE1
    adds 2 credits instead of 1.

  - Bits 6 and 7 of DSW are told to be unused, but they are tested before
    entering the initials (code at 0x004a1e, 0x014bb0 and 0x014c84)


2) 'bloodbroa'

  - When "Starting Coin" Dip Switch is set to "x2", you need 2 coins to start
    a game (but 1 coin to join), then 1 coin to continue.
    However, it you insert 2 coins, and press START2, you start a 2 players game.
    Also note that when "Starting Coin" Dip Switch is set to "x2", SERVICE1
    adds 2 credits instead of 1.

  - Bits 6 and 7 of DSW are told to be unused, but they are tested before
    entering the initials (code at 0x004a46, 0x014bd8 and 0x014cac)


3) 'weststry'

  - This bootleg has been released by Datsu in 1991. This "company" also
    bootlegged "Toki" (another TAD game) in 1990, and several Sega titles.

  - When "Starting Coin" Dip Switch is set to "x2", you need 2 coins to start
    a game (but 1 coin to join), then 1 coin to continue.
    However, it you insert 2 coins, and press START2, you start a 2 players game.

  - Bits 6 and 7 of DSW are told to be unused, but bit 7 is tested when
    entering the initials (code at 0x00497e, 0x014cf2 and 0x014dc4
    Note that bit 8 (!) is tested at 0x0049a2, so this has no effect.
  - Bit 7 of DSW is also "merged" with bit 6 (code at 0x0002f6 and 0x000326).


4) 'skysmash'

  - This game only has 2 buttons : bits 6 and 14 of players inputs are NEVER tested !

  - When "Starting Coin" Dip Switch is set to "x2", you need 2 coins to start
    a game (but 1 coin to join), then 1 coin to continue.
    However, it you insert 2 coins, and press START2, you start a 2 players game.
    Also note that when "Starting Coin" Dip Switch is set to "x2", SERVICE1
    adds 2 credits instead of 1.

  - Bit 6 of DSW was previouly used as a "Cabinet" Dip Switch (OFF = Upright
    and ON = Cocktail), but it isn't tested outside of the "test mode".
    Check code from 0x021abe to 0x021afc (and the "rts" instruction at 0x021adc)
  - Bit 7 of DSW is only tested at 0x02035e and writes a value to 0x0c0100.
    I don't know what it is supposed to do, but it could be a "Flip Screen"
    Dip Switch as in 'toki' (same manufacturer and similar hardware).


DIP locations verified for Blood Bros. & Sky Smasher via manual & DIP-SW setting
    screens at power up.

**************************************************************************/

#include "emu.h"
#include "cpu/m68000/m68000.h"
#include "audio/seibu.h"
#include "cpu/z80/z80.h"
#include "sound/3812intf.h"
#include "includes/bloodbro.h"
#include "video/seibu_crtc.h"


/* Memory Maps */
static ADDRESS_MAP_START( common_map, AS_PROGRAM, 16, bloodbro_state )
	AM_RANGE(0x000000, 0x07ffff) AM_ROM
	AM_RANGE(0x080000, 0x08afff) AM_RAM
	AM_RANGE(0x08b000, 0x08bfff) AM_RAM AM_SHARE("spriteram")
	AM_RANGE(0x08c000, 0x08c3ff) AM_RAM_WRITE(bgvideoram_w) AM_SHARE("bgvideoram")
	AM_RANGE(0x08c400, 0x08cfff) AM_RAM
	AM_RANGE(0x08d000, 0x08d3ff) AM_RAM_WRITE(fgvideoram_w) AM_SHARE("fgvideoram")
	AM_RANGE(0x08d400, 0x08d7ff) AM_RAM
	AM_RANGE(0x08d800, 0x08dfff) AM_RAM_WRITE(txvideoram_w) AM_SHARE("txvideoram")
	AM_RANGE(0x08e000, 0x08e7ff) AM_RAM
	AM_RANGE(0x08e800, 0x08f7ff) AM_RAM_DEVWRITE("palette", palette_device, write) AM_SHARE("palette")
	AM_RANGE(0x08f800, 0x08ffff) AM_RAM
	AM_RANGE(0x0a0000, 0x0a000d) AM_DEVREADWRITE("seibu_sound", seibu_sound_device, main_word_r, main_word_w)
//  AM_RANGE(0x0c0000, 0x0c007f) AM_RAM AM_SHARE("scroll")
	AM_RANGE(0x0c0080, 0x0c0081) AM_WRITENOP // ??? IRQ Ack VBL?
	AM_RANGE(0x0c00c0, 0x0c00c1) AM_WRITENOP // ??? watchdog?
	AM_RANGE(0x0c0100, 0x0c0101) AM_WRITENOP // ??? written once
	AM_RANGE(0x0e0000, 0x0e0001) AM_READ_PORT("DSW")
	AM_RANGE(0x0e0002, 0x0e0003) AM_READ_PORT("IN0")
	AM_RANGE(0x0e0004, 0x0e0005) AM_READ_PORT("IN1")
ADDRESS_MAP_END

static ADDRESS_MAP_START( bloodbro_map, AS_PROGRAM, 16, bloodbro_state )
	AM_IMPORT_FROM(common_map)
	AM_RANGE(0xc0000, 0xc004f) AM_DEVREADWRITE("crtc", seibu_crtc_device, read, write)
ADDRESS_MAP_END

static ADDRESS_MAP_START( skysmash_map, AS_PROGRAM, 16, bloodbro_state )
	AM_IMPORT_FROM(common_map)
	AM_RANGE(0xc0000, 0xc004f) AM_DEVREADWRITE("crtc", seibu_crtc_device, read_alt, write_alt)
ADDRESS_MAP_END

static ADDRESS_MAP_START( weststry_map, AS_PROGRAM, 16, bloodbro_state )
	AM_RANGE(0x000000, 0x07ffff) AM_ROM
	AM_RANGE(0x080000, 0x08afff) AM_RAM
	AM_RANGE(0x08b000, 0x08bfff) AM_RAM AM_SHARE("spriteram")
	AM_RANGE(0x08c000, 0x08c3ff) AM_RAM_WRITE(bgvideoram_w) AM_SHARE("bgvideoram")
	AM_RANGE(0x08c400, 0x08cfff) AM_RAM
	AM_RANGE(0x08d000, 0x08d3ff) AM_RAM_WRITE(fgvideoram_w) AM_SHARE("fgvideoram")
	AM_RANGE(0x08d400, 0x08d7ff) AM_RAM
	AM_RANGE(0x08d800, 0x08dfff) AM_RAM_WRITE(txvideoram_w) AM_SHARE("txvideoram")
	AM_RANGE(0x08e000, 0x08ffff) AM_RAM
	AM_RANGE(0x0c1000, 0x0c1001) AM_READ_PORT("DSW")
	AM_RANGE(0x0c1002, 0x0c1003) AM_READ_PORT("IN0")
	AM_RANGE(0x0c1004, 0x0c1005) AM_READ_PORT("IN1")
	AM_RANGE(0x0c1000, 0x0c17ff) AM_RAM
	AM_RANGE(0x128000, 0x1287ff) AM_RAM_DEVWRITE("palette", palette_device, write) AM_SHARE("palette")
	AM_RANGE(0x120000, 0x128fff) AM_RAM
ADDRESS_MAP_END

/* Input Ports */

#define BLOODBRO_COINAGE \
	PORT_DIPNAME( 0x0001, 0x0001, "Coin Mode" )                  PORT_DIPLOCATION("SW1:1") \
	PORT_DIPSETTING(      0x0001, "Mode 1" ) \
	PORT_DIPSETTING(      0x0000, "Mode 2" ) \
	/* Coin Mode 1 */ \
	PORT_DIPNAME( 0x001e, 0x001e, DEF_STR( Coinage ) )           PORT_DIPLOCATION("SW1:2,3,4,5") PORT_CONDITION("DSW",0x0001,EQUALS,0x0001) \
	PORT_DIPSETTING(      0x0014, DEF_STR( 6C_1C ) ) \
	PORT_DIPSETTING(      0x0016, DEF_STR( 5C_1C ) ) \
	PORT_DIPSETTING(      0x0018, DEF_STR( 4C_1C ) ) \
	PORT_DIPSETTING(      0x001a, DEF_STR( 3C_1C ) ) \
	PORT_DIPSETTING(      0x0002, DEF_STR( 8C_3C ) ) \
	PORT_DIPSETTING(      0x001c, DEF_STR( 2C_1C ) ) \
	PORT_DIPSETTING(      0x0004, DEF_STR( 5C_3C ) ) \
	PORT_DIPSETTING(      0x0006, DEF_STR( 3C_2C ) ) \
	PORT_DIPSETTING(      0x001e, DEF_STR( 1C_1C ) ) \
	PORT_DIPSETTING(      0x0008, DEF_STR( 2C_3C ) ) \
	PORT_DIPSETTING(      0x0012, DEF_STR( 1C_2C ) ) \
	PORT_DIPSETTING(      0x0010, DEF_STR( 1C_3C ) ) \
	PORT_DIPSETTING(      0x000e, DEF_STR( 1C_4C ) ) \
	PORT_DIPSETTING(      0x000c, DEF_STR( 1C_5C ) ) \
	PORT_DIPSETTING(      0x000a, DEF_STR( 1C_6C ) ) \
	PORT_DIPSETTING(      0x0000, DEF_STR( Free_Play ) ) \
	/* Coin Mode 2 */ \
	PORT_DIPNAME( 0x0006, 0x0006, DEF_STR( Coin_A ) )            PORT_DIPLOCATION("SW1:2,3") PORT_CONDITION("DSW",0x0001,EQUALS,0x0000) \
	PORT_DIPSETTING(      0x0000, DEF_STR( 5C_1C ) ) \
	PORT_DIPSETTING(      0x0002, DEF_STR( 3C_1C ) ) \
	PORT_DIPSETTING(      0x0004, DEF_STR( 2C_1C ) ) \
	PORT_DIPSETTING(      0x0006, DEF_STR( 1C_1C ) ) \
	PORT_DIPNAME( 0x0018, 0x0018, DEF_STR( Coin_B ) )            PORT_DIPLOCATION("SW1:4,5") PORT_CONDITION("DSW",0x0001,EQUALS,0x0000) \
	PORT_DIPSETTING(      0x0018, DEF_STR( 1C_2C ) ) \
	PORT_DIPSETTING(      0x0010, DEF_STR( 1C_3C ) ) \
	PORT_DIPSETTING(      0x0008, DEF_STR( 1C_5C ) ) \
	PORT_DIPSETTING(      0x0000, DEF_STR( 1C_6C ) ) \
	PORT_DIPNAME( 0x0020, 0x0020, "Starting Coin" )              PORT_DIPLOCATION("SW1:6") \
	PORT_DIPSETTING(      0x0020, DEF_STR( Normal ) ) \
	PORT_DIPSETTING(      0x0000, "x2" )

static INPUT_PORTS_START( bloodbro_base )
	PORT_START("DSW")
	BLOODBRO_COINAGE
	/*  SW1:7,8 is listed as "ROM change option", "optional"
	                     A  B  C  D
	    Coin Up          0  1  1  1
	    Screen Complete  0  0  0  1
	    Stage Complete   1  1  3  1
	    Top 10           1  3  3  3
	    High Score       1  4  5  4
	    Complete Game    5  5 10 15

	    D limits 8 tickets/coin except completion

	    The sets handled in MAME don't seem to have this "feature"
	PORT_DIPNAME( 0x00c0, 0x00c0, "Ticket Dispenser Payout Mode" ) PORT_DIPLOCATION("SW1:7,8")
	PORT_DIPSETTING(      0x00c0, "A" )
	PORT_DIPSETTING(      0x0080, "B" )
	PORT_DIPSETTING(      0x0040, "C" )
	PORT_DIPSETTING(      0x0000, "D" )
	*/
	PORT_DIPUNKNOWN_DIPLOC( 0x0040, IP_ACTIVE_LOW, "SW1:7" )
	PORT_DIPUNKNOWN_DIPLOC( 0x0080, IP_ACTIVE_LOW, "SW1:8" )
	PORT_DIPNAME( 0x0300, 0x0300, DEF_STR( Lives ) )             PORT_DIPLOCATION("SW2:1,2")
	PORT_DIPSETTING(      0x0000, "1" )
	PORT_DIPSETTING(      0x0200, "2" )
	PORT_DIPSETTING(      0x0300, "3" )
	PORT_DIPSETTING(      0x0100, "5" )
	PORT_DIPNAME( 0x0c00, 0x0c00, DEF_STR( Bonus_Life ) )        PORT_DIPLOCATION("SW2:3,4")
	PORT_DIPSETTING(      0x0c00, "300K 500K+" )
	PORT_DIPSETTING(      0x0800, "500K+" )
	PORT_DIPSETTING(      0x0400, "500K" )
	PORT_DIPSETTING(      0x0000, DEF_STR( None ) )
	PORT_DIPNAME( 0x3000, 0x3000, DEF_STR( Difficulty ) )        PORT_DIPLOCATION("SW2:5,6")
	PORT_DIPSETTING(      0x2000, DEF_STR( Easy ) )
	PORT_DIPSETTING(      0x3000, DEF_STR( Normal ) )
	PORT_DIPSETTING(      0x1000, DEF_STR( Hard ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( Very_Hard ) )
	PORT_DIPNAME( 0x4000, 0x4000, DEF_STR( Allow_Continue ) )    PORT_DIPLOCATION("SW2:7")
	PORT_DIPSETTING(      0x0000, DEF_STR( No ) )
	PORT_DIPSETTING(      0x4000, DEF_STR( Yes ) )
	PORT_DIPNAME( 0x8000, 0x8000, DEF_STR( Demo_Sounds ) )       PORT_DIPLOCATION("SW2:8")
	PORT_DIPSETTING(      0x0000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x8000, DEF_STR( On ) )

	PORT_START("IN0")
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(1)   // "Fire"
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(1)   // "Roll"
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_PLAYER(1)   // "Dynamite"
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_UNKNOWN )  /* tested when "continue" - check code at 0x000598 */
	PORT_BIT( 0x0100, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x0200, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x0400, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x0800, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x1000, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(2)   // "Fire"
	PORT_BIT( 0x2000, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(2)   // "Roll"
	PORT_BIT( 0x4000, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_PLAYER(2)   // "Dynamite"
	PORT_BIT( 0x8000, IP_ACTIVE_LOW, IPT_UNKNOWN )  /* tested - check code at 0x0005fe - VBLANK ? (probably not) */

	PORT_START("IN1")
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_START2 )
	PORT_BIT( 0x00e0, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0xff00, IP_ACTIVE_LOW, IPT_UNKNOWN )
INPUT_PORTS_END


static INPUT_PORTS_START( weststry )
	PORT_INCLUDE( bloodbro_base )

	PORT_START("COIN")  /* referenced by seibu sound board */
	PORT_BIT( 0xff, IP_ACTIVE_LOW, IPT_UNKNOWN )
INPUT_PORTS_END


static INPUT_PORTS_START( bloodbro )
	PORT_INCLUDE( bloodbro_base )

	PORT_MODIFY("IN1")
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x000e, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_START2 )
	PORT_BIT( 0x00e0, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0100, IP_ACTIVE_LOW, IPT_UNKNOWN )  /* tested - check code at 0x000800 */
	PORT_BIT( 0x0e00, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x1000, IP_ACTIVE_LOW, IPT_SERVICE1 )
	PORT_BIT( 0xe000, IP_ACTIVE_LOW, IPT_UNKNOWN )

	SEIBU_COIN_INPUTS   /* coin inputs read through sound cpu */
INPUT_PORTS_END

static INPUT_PORTS_START( skysmash )
	PORT_START("DSW")
	BLOODBRO_COINAGE
	PORT_DIPUNUSED_DIPLOC( 0x0040, IP_ACTIVE_LOW, "SW1:7" )      /* see notes */
	PORT_DIPUNUSED_DIPLOC( 0x0080, IP_ACTIVE_LOW, "SW1:8" )      /* see notes */
	PORT_DIPNAME( 0x0300, 0x0300, DEF_STR( Lives ) )             PORT_DIPLOCATION("SW2:1,2")
	PORT_DIPSETTING(      0x0200, "2" )
	PORT_DIPSETTING(      0x0300, "3" )
	PORT_DIPSETTING(      0x0100, "5" )
	PORT_DIPSETTING(      0x0000, DEF_STR( Infinite ) )
	PORT_DIPNAME( 0x0c00, 0x0c00, DEF_STR( Bonus_Life ) )        PORT_DIPLOCATION("SW2:3,4")
	PORT_DIPSETTING(      0x0c00, "120K 200K+" )
	PORT_DIPSETTING(      0x0800, "200K+" )
	PORT_DIPSETTING(      0x0400, "250K+" )
	PORT_DIPSETTING(      0x0000, "200K" )
	PORT_DIPNAME( 0x3000, 0x3000, DEF_STR( Difficulty ) )        PORT_DIPLOCATION("SW2:5,6")
	PORT_DIPSETTING(      0x0000, DEF_STR( Easy ) )
	PORT_DIPSETTING(      0x3000, DEF_STR( Normal ) )
	PORT_DIPSETTING(      0x2000, DEF_STR( Hard ) )
	PORT_DIPSETTING(      0x1000, DEF_STR( Very_Hard ) )
	PORT_DIPNAME( 0x4000, 0x4000, DEF_STR( Allow_Continue ) )    PORT_DIPLOCATION("SW2:7")
	PORT_DIPSETTING(      0x0000, DEF_STR( No ) )
	PORT_DIPSETTING(      0x4000, DEF_STR( Yes ) )
	PORT_DIPNAME( 0x8000, 0x8000, DEF_STR( Demo_Sounds ) )       PORT_DIPLOCATION("SW2:8")
	PORT_DIPSETTING(      0x0000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x8000, DEF_STR( On ) )

	PORT_START("IN0")
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(1)   // "Fire"
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(1)   // "Bomb"
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0100, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x0200, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x0400, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x0800, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x1000, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(2)   // "Fire"
	PORT_BIT( 0x2000, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(2)   // "Bomb"
	PORT_BIT( 0x4000, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x8000, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("IN1")
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x000e, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_START2 )
	PORT_BIT( 0x00e0, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0100, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0e00, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x1000, IP_ACTIVE_LOW, IPT_SERVICE1 )
	PORT_BIT( 0xe000, IP_ACTIVE_LOW, IPT_UNKNOWN )

	SEIBU_COIN_INPUTS   /* coin inputs read through sound cpu */
INPUT_PORTS_END


/* Graphics Layouts */

static const gfx_layout textlayout =
{
	8,8,    /* 8*8 characters */
	RGN_FRAC(1,2),  /* 4096 characters */
	4,  /* 4 bits per pixel */
	{ 0, 4, RGN_FRAC(1,2)+0, RGN_FRAC(1,2)+4 },
	{ 3, 2, 1, 0, 8+3, 8+2, 8+1, 8+0},
	{ 0*16, 1*16, 2*16, 3*16, 4*16, 5*16, 6*16, 7*16 },
	16*8    /* every char takes 16 consecutive bytes */
};

static const gfx_layout spritelayout =
{
	16,16,  /* 16*16 sprites  */
	RGN_FRAC(1,1),
	4,  /* 4 bits per pixel */
	{ 8, 12, 0, 4 },
	{ 3, 2, 1, 0, 16+3, 16+2, 16+1, 16+0,
				3+32*16, 2+32*16, 1+32*16, 0+32*16, 16+3+32*16, 16+2+32*16, 16+1+32*16, 16+0+32*16 },
	{ 0*16, 2*16, 4*16, 6*16, 8*16, 10*16, 12*16, 14*16,
			16*16, 18*16, 20*16, 22*16, 24*16, 26*16, 28*16, 30*16 },
	128*8   /* every sprite takes 128 consecutive bytes */
};

static const gfx_layout weststry_textlayout =
{
	8,8,    /* 8*8 sprites */
	RGN_FRAC(1,4),  /* 4096 sprites */
	4,  /* 4 bits per pixel */
	{ RGN_FRAC(0,4), RGN_FRAC(1,4), RGN_FRAC(2,4), RGN_FRAC(3,4) },
		{ 0, 1, 2, 3, 4, 5, 6, 7 },
		{ 0*8, 1*8, 2*8, 3*8, 4*8, 5*8, 6*8, 7*8 },
	8*8 /* every sprite takes 8 consecutive bytes */
};

static const gfx_layout weststry_spritelayout =
{
	16,16,  /* 16*16 sprites */
	RGN_FRAC(1,4),  /* 8192 sprites */
	4,  /* 4 bits per pixel */
	{ RGN_FRAC(0,4), RGN_FRAC(1,4), RGN_FRAC(2,4), RGN_FRAC(3,4) },
	{ 0, 1, 2, 3, 4, 5, 6, 7,
			16*8+0, 16*8+1, 16*8+2, 16*8+3, 16*8+4, 16*8+5, 16*8+6, 16*8+7 },
	{ 0*8, 1*8, 2*8, 3*8, 4*8, 5*8, 6*8, 7*8,
			8*8, 9*8, 10*8, 11*8, 12*8, 13*8, 14*8, 15*8 },
	32*8    /* every sprite takes 32 consecutive bytes */
};

/* Graphics Decode Info */

static GFXDECODE_START( bloodbro )
	GFXDECODE_ENTRY( "gfx1", 0x00000, textlayout,   0x70*16,  0x10 ) /* Text */
	GFXDECODE_ENTRY( "gfx2", 0x00000, spritelayout, 0x40*16,  0x10 ) /* Background */
	GFXDECODE_ENTRY( "gfx2", 0x00000, spritelayout, 0x50*16,  0x10 ) /* Foreground */
	GFXDECODE_ENTRY( "gfx3", 0x00000, spritelayout, 0x00*16,  0x10 ) /* Sprites */
GFXDECODE_END

static GFXDECODE_START( weststry )
	GFXDECODE_ENTRY( "gfx1", 0x00000, weststry_textlayout,     0x10*16,  0x10 )
	GFXDECODE_ENTRY( "gfx2", 0x00000, weststry_spritelayout,   0x30*16,  0x10 )
	GFXDECODE_ENTRY( "gfx2", 0x00000, weststry_spritelayout,   0x20*16,  0x10 )
	GFXDECODE_ENTRY( "gfx3", 0x00000, weststry_spritelayout,   0x00*16,  0x10 )
GFXDECODE_END

WRITE16_MEMBER( bloodbro_state::layer_en_w )
{
	m_layer_en = data;
}

WRITE16_MEMBER( bloodbro_state::layer_scroll_w )
{
	COMBINE_DATA(&m_scrollram[offset]);
}

/* Machine Drivers */

static MACHINE_CONFIG_START( bloodbro, bloodbro_state )
	// basic machine hardware
	MCFG_CPU_ADD("maincpu", M68000, XTAL_20MHz/2) /* verified on pcb */
	MCFG_CPU_PROGRAM_MAP(bloodbro_map)
	MCFG_CPU_VBLANK_INT_DRIVER("screen", bloodbro_state,  irq4_line_hold)

	SEIBU_SOUND_SYSTEM_CPU(XTAL_7_15909MHz/2) /* verified on pcb */

	// video hardware

	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_REFRESH_RATE(59.39)    /* verified on pcb */
	MCFG_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(2500) /* not accurate */)
	MCFG_SCREEN_SIZE(32*8, 32*8)
	MCFG_SCREEN_VISIBLE_AREA(0*8, 32*8-1, 2*8, 30*8-1)
	MCFG_SCREEN_UPDATE_DRIVER(bloodbro_state, screen_update_bloodbro)
	MCFG_SCREEN_PALETTE("palette")

	MCFG_DEVICE_ADD("crtc", SEIBU_CRTC, 0)
	MCFG_SEIBU_CRTC_LAYER_EN_CB(WRITE16(bloodbro_state, layer_en_w))
	MCFG_SEIBU_CRTC_LAYER_SCROLL_CB(WRITE16(bloodbro_state, layer_scroll_w))


	MCFG_GFXDECODE_ADD("gfxdecode", "palette", bloodbro)
	MCFG_PALETTE_ADD("palette", 2048)
	MCFG_PALETTE_FORMAT(xxxxBBBBGGGGRRRR)

	// sound hardware
	SEIBU_SOUND_SYSTEM_YM3812_RAIDEN_INTERFACE(XTAL_7_15909MHz/2, XTAL_12MHz/12)
MACHINE_CONFIG_END

static MACHINE_CONFIG_DERIVED( weststry, bloodbro )

	MCFG_CPU_MODIFY("maincpu")
	MCFG_CPU_PROGRAM_MAP(weststry_map)
	MCFG_CPU_VBLANK_INT_DRIVER("screen", bloodbro_state,  irq6_line_hold)

	MCFG_GFXDECODE_MODIFY("gfxdecode", weststry)
	MCFG_PALETTE_MODIFY("palette")
	MCFG_PALETTE_ENTRIES(1024)
	MCFG_PALETTE_FORMAT(xxxxBBBBGGGGRRRR)

	MCFG_SCREEN_MODIFY("screen")
	MCFG_SCREEN_UPDATE_DRIVER(bloodbro_state, screen_update_weststry)
MACHINE_CONFIG_END

static MACHINE_CONFIG_DERIVED( skysmash, bloodbro )

	MCFG_CPU_MODIFY("maincpu")
	MCFG_CPU_PROGRAM_MAP(skysmash_map)
	MCFG_CPU_VBLANK_INT_DRIVER("screen", bloodbro_state,  irq2_line_hold)

	MCFG_SCREEN_MODIFY("screen")
	MCFG_SCREEN_UPDATE_DRIVER(bloodbro_state, screen_update_skysmash)
MACHINE_CONFIG_END

/* ROMs */

ROM_START( bloodbro )
	ROM_REGION( 0x80000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "2j.u021.7n",   0x00001, 0x20000, CRC(c0fdc3e4) SHA1(31968f693de2054a0c8ba50a8d44a371dd9c2848) )
	ROM_LOAD16_BYTE( "1j.u022.8n",   0x00000, 0x20000, CRC(2d7e0fdf) SHA1(8fe22d8a1ef7d562a475a5b6c98303b0cb1af561) )
	ROM_LOAD16_BYTE( "bb_04.u023.7l", 0x40001, 0x20000, CRC(fd951c2c) SHA1(f4031bf303c67c82f2f78f7456f78382d8c1ac85) )
	ROM_LOAD16_BYTE( "bb_03.u024.8l", 0x40000, 0x20000, CRC(18d3c460) SHA1(93b86af1199f0fedeaf1fe64d27ffede4b819e42) )

	ROM_REGION( 0x20000, "audiocpu", 0 )
	ROM_LOAD( "bb_07.u1016.6a",   0x000000, 0x08000, CRC(411b94e8) SHA1(6968441f64212c0935afeca68f07deaadf86d614) )
	ROM_CONTINUE(            0x010000, 0x08000 )
	ROM_COPY( "audiocpu", 0x000000, 0x018000, 0x08000 )

	ROM_REGION( 0x20000, "gfx1", 0 )
	ROM_LOAD( "bb_05.u061.6f", 0x00000, 0x10000, CRC(04ba6d19) SHA1(7333075c3323756d51917418b5234d785a9bee00) ) /* characters */
	ROM_LOAD( "bb_06.u063.6d", 0x10000, 0x10000, CRC(7092e35b) SHA1(659d30b2e2fd9ffa34a47e98193c8f0a87ac1315) )

	ROM_REGION( 0x100000, "gfx2", 0 )
	ROM_LOAD( "blood_bros_bk__(c)1990_tad_corp.u064.4d", 0x00000, 0x100000, CRC(1aa87ee6) SHA1(e7843c1e8a0f3a685f0b5d6e3a2eb3176c410847) )  /* Background+Foreground */

	ROM_REGION( 0x100000, "gfx3", 0 )
	ROM_LOAD( "blood_bros_obj__(c)1990_tad_corp.u078.2n", 0x00000, 0x100000, CRC(d27c3952) SHA1(de7306432b682f238b911507ad7aa2fa8acbee80) ) /* sprites */

	ROM_REGION( 0x40000, "oki", 0 ) /* ADPCM samples */
	ROM_LOAD( "bb_08.u095.5a",  0x00000, 0x20000, CRC(deb1b975) SHA1(08f2e9a0a23171201b71d381d091edcd3787c287) )
ROM_END

ROM_START( bloodbroa )
	ROM_REGION( 0x80000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "2j_a.u021.7n",     0x00001, 0x20000, CRC(e8ca21b8) SHA1(65330368c29ec64ca03124d28a7f8b49d7a15566) )
	ROM_LOAD16_BYTE( "1j_a.u022.8n",     0x00000, 0x20000, CRC(6b28cfc7) SHA1(dedd06cab61381f3e27dc14315f283d707d06bcb) )
	ROM_LOAD16_BYTE( "bb_04.u023.7l", 0x40001, 0x20000, CRC(fd951c2c) SHA1(f4031bf303c67c82f2f78f7456f78382d8c1ac85) )
	ROM_LOAD16_BYTE( "bb_03.u024.8l", 0x40000, 0x20000, CRC(18d3c460) SHA1(93b86af1199f0fedeaf1fe64d27ffede4b819e42) )

	ROM_REGION( 0x20000, "audiocpu", 0 )
	ROM_LOAD( "bb_07.u1016.6a",   0x000000, 0x08000, CRC(411b94e8) SHA1(6968441f64212c0935afeca68f07deaadf86d614) )
	ROM_CONTINUE(            0x010000, 0x08000 )
	ROM_COPY( "audiocpu", 0x000000, 0x018000, 0x08000 )

	ROM_REGION( 0x20000, "gfx1", 0 )
	ROM_LOAD( "bb_05.u061.6f", 0x00000, 0x10000, CRC(04ba6d19) SHA1(7333075c3323756d51917418b5234d785a9bee00) ) /* characters */
	ROM_LOAD( "bb_06.u063.6d", 0x10000, 0x10000, CRC(7092e35b) SHA1(659d30b2e2fd9ffa34a47e98193c8f0a87ac1315) )

	ROM_REGION( 0x100000, "gfx2", 0 )
	ROM_LOAD( "blood_bros_bk__(c)1990_tad_corp.u064.4d", 0x00000, 0x100000, CRC(1aa87ee6) SHA1(e7843c1e8a0f3a685f0b5d6e3a2eb3176c410847) )  /* Background+Foreground */

	ROM_REGION( 0x100000, "gfx3", 0 )
	ROM_LOAD( "blood_bros_obj__(c)1990_tad_corp.u078.2n", 0x00000, 0x100000, CRC(d27c3952) SHA1(de7306432b682f238b911507ad7aa2fa8acbee80) ) /* sprites */

	ROM_REGION( 0x40000, "oki", 0 ) /* ADPCM samples */
	ROM_LOAD( "bb_08.u095.5a",  0x00000, 0x20000, CRC(deb1b975) SHA1(08f2e9a0a23171201b71d381d091edcd3787c287) )
ROM_END

ROM_START( bloodbrob )
	ROM_REGION( 0x80000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "bloodbros02.u021.7n", 0x00001, 0x20000, CRC(204dca6e) SHA1(f04cb2116d699ac2deec2d55597beb5d7b3dc793) )
	ROM_LOAD16_BYTE( "bloodbros01.u022.8n", 0x00000, 0x20000, CRC(ac6719e7) SHA1(bcefd61ed5d2f5eeb4424243f03037c7d123b9f6) )
	ROM_LOAD16_BYTE( "bb_04.u023.7l",       0x40001, 0x20000, CRC(fd951c2c) SHA1(f4031bf303c67c82f2f78f7456f78382d8c1ac85) )
	ROM_LOAD16_BYTE( "bb_03.u024.8l",       0x40000, 0x20000, CRC(18d3c460) SHA1(93b86af1199f0fedeaf1fe64d27ffede4b819e42) )

	ROM_REGION( 0x20000, "audiocpu", 0 )
	ROM_LOAD( "bb_07.u1016.6a",   0x000000, 0x08000, CRC(411b94e8) SHA1(6968441f64212c0935afeca68f07deaadf86d614) )
	ROM_CONTINUE(            0x010000, 0x08000 )
	ROM_COPY( "audiocpu", 0x000000, 0x018000, 0x08000 )

	ROM_REGION( 0x20000, "gfx1", 0 )
	ROM_LOAD( "bb_05.u061.6f", 0x00000, 0x10000, CRC(04ba6d19) SHA1(7333075c3323756d51917418b5234d785a9bee00) ) /* characters */
	ROM_LOAD( "bb_06.u063.6d", 0x10000, 0x10000, CRC(7092e35b) SHA1(659d30b2e2fd9ffa34a47e98193c8f0a87ac1315) )

	ROM_REGION( 0x100000, "gfx2", 0 )
	ROM_LOAD( "blood_bros_bk__(c)1990_tad_corp.u064.4d", 0x00000, 0x100000, CRC(1aa87ee6) SHA1(e7843c1e8a0f3a685f0b5d6e3a2eb3176c410847) )  /* Background+Foreground */

	ROM_REGION( 0x100000, "gfx3", 0 )
	ROM_LOAD( "blood_bros_obj__(c)1990_tad_corp.u078.2n", 0x00000, 0x100000, CRC(d27c3952) SHA1(de7306432b682f238b911507ad7aa2fa8acbee80) ) /* sprites */

	ROM_REGION( 0x40000, "oki", 0 ) /* ADPCM samples */
	ROM_LOAD( "bb_08.u095.5a",  0x00000, 0x20000, CRC(deb1b975) SHA1(08f2e9a0a23171201b71d381d091edcd3787c287) )
ROM_END


ROM_START( weststry )
	ROM_REGION( 0x80000, "maincpu", 0 ) /* 64k for cpu code */
	ROM_LOAD16_BYTE( "ws13.bin",  0x00001, 0x20000, CRC(158e302a) SHA1(52cc1bf526424ff025a6b79f3fc7bba4b9bbfcbb) )
	ROM_LOAD16_BYTE( "ws15.bin",  0x00000, 0x20000, CRC(672e9027) SHA1(71cb9fcef04edb972ba88de45d605dcff539ea2d) )
	ROM_LOAD16_BYTE( "bb_04.bin", 0x40001, 0x20000, CRC(fd951c2c) SHA1(f4031bf303c67c82f2f78f7456f78382d8c1ac85) )
	ROM_LOAD16_BYTE( "bb_03.bin", 0x40000, 0x20000, CRC(18d3c460) SHA1(93b86af1199f0fedeaf1fe64d27ffede4b819e42) )

	ROM_REGION( 0x20000, "audiocpu", 0 )    /* 64k for sound cpu code */
	ROM_LOAD( "ws17.bin",    0x000000, 0x08000, CRC(e00a8f09) SHA1(e7247ce0ab99d0726f31dee5de5ba33f4ebd183e) )
	ROM_CONTINUE(            0x010000, 0x08000 )
	ROM_COPY( "audiocpu", 0x000000, 0x018000, 0x08000 )

	ROM_REGION( 0x20000, "gfx1", 0 ) // first half of these is blank
	ROM_LOAD( "ws09.bin", 0x00000, 0x08000, CRC(f05b2b3e) SHA1(6570d795d68655ace9668f32dc0bf5c2d2372411) )  /* characters */
	ROM_CONTINUE(         0x00000, 0x08000 )
	ROM_LOAD( "ws11.bin", 0x08000, 0x08000, CRC(2b10e3d2) SHA1(0f5045615b44e2300745fd3afac7f1441352cca5) )
	ROM_CONTINUE(         0x08000, 0x08000 )
	ROM_LOAD( "ws10.bin", 0x10000, 0x08000, CRC(efdf7c82) SHA1(65392697f56473cfe90d9733b9c49f2da6f9b7e6) )
	ROM_CONTINUE(         0x10000, 0x08000 )
	ROM_LOAD( "ws12.bin", 0x18000, 0x08000, CRC(af993578) SHA1(b250b562deeab3bb2c79002e5e1f0b6e17986848) )
	ROM_CONTINUE(         0x18000, 0x08000 )

	ROM_REGION( 0x100000, "gfx2", 0 )
	ROM_LOAD( "ws01.bin", 0x20000, 0x20000, CRC(32bda4bc) SHA1(ed0c0740c7af513b341b2b7ff3e0bf6045e930e9) )  /* Foreground */
	ROM_LOAD( "ws03.bin", 0x60000, 0x20000, CRC(046b51f8) SHA1(25af752caebdec762582fc0130cf14546110bb54) )
	ROM_LOAD( "ws02.bin", 0xa0000, 0x20000, CRC(ed9d682e) SHA1(0f79ea09a7af367d175081f72f2bc94f6caad463) )
	ROM_LOAD( "ws04.bin", 0xe0000, 0x20000, CRC(75f082e5) SHA1(b29f09a3cc9a0ac3f982be3981f5e895050c49e8) )
	ROM_LOAD( "ws05.bin", 0x00000, 0x20000, CRC(007c8dc0) SHA1(f44576da3b89d6a889fdb564825ac6ce3bb4cffe) )  /* Background */
	ROM_LOAD( "ws07.bin", 0x40000, 0x20000, CRC(0f0c8d9a) SHA1(f5fe9b5ee4c8ffd7caf5313d13fb5f6e181ed9b6) )
	ROM_LOAD( "ws06.bin", 0x80000, 0x20000, CRC(459d075e) SHA1(24cd0bffe7c5bbccf653ced0b73579059603d187) )
	ROM_LOAD( "ws08.bin", 0xc0000, 0x20000, CRC(4d6783b3) SHA1(9870fe9570afeff179b6080581fd6bb187898ff0) )

	ROM_REGION( 0x100000, "gfx3", ROMREGION_INVERT )
	ROM_LOAD( "ws25.bin", 0x00000, 0x20000, BAD_DUMP CRC(8092e8e9) SHA1(eabe58ac0f88234b0dddf361f56aad509a83012e) ) /* sprites */
	ROM_LOAD( "ws26.bin", 0x20000, 0x20000, BAD_DUMP CRC(f6a1f42c) SHA1(6d5503e1a9b00104970292d22301ed28893c5223) )
	ROM_LOAD( "ws23.bin", 0x40000, 0x20000, CRC(43d58e24) SHA1(99e255faa9716d9102a1223419084fc209ab4024) )
	ROM_LOAD( "ws24.bin", 0x60000, 0x20000, CRC(20a867ea) SHA1(d3985002931fd4180fc541d61a94371871f3709d) )
	ROM_LOAD( "ws21.bin", 0x80000, 0x20000, CRC(e23d7296) SHA1(33bbced960be22efc7d2681e06a27feba09e0fc0) )
	ROM_LOAD( "ws22.bin", 0xa0000, 0x20000, CRC(7150a060) SHA1(73bdd7d6752f7fe9e23073d835dbc468d57865fa) )
	ROM_LOAD( "ws19.bin", 0xc0000, 0x20000, CRC(c5dd0a96) SHA1(4696ab1b02d40c54a7dacf0bdf90b624b7d6812e) )
	ROM_LOAD( "ws20.bin", 0xe0000, 0x20000, CRC(f1245c16) SHA1(f3941bf5830995f65a5378326fdb72687fbbddcf) )

	ROM_REGION( 0x40000, "oki", 0 ) /* ADPCM samples */
	ROM_LOAD( "bb_08.bin", 0x00000, 0x20000, CRC(deb1b975) SHA1(08f2e9a0a23171201b71d381d091edcd3787c287) )
ROM_END

ROM_START( skysmash )
	ROM_REGION( 0x80000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "rom5", 0x00000, 0x20000, CRC(867f9897) SHA1(7751f9d03d71bd5db0b82bda6e4d5231a30c1ad0) )
	ROM_LOAD16_BYTE( "rom6", 0x00001, 0x20000, CRC(e9c1d308) SHA1(d7032345b91f87de64ad09ffea49e39b755cac44) )
	ROM_LOAD16_BYTE( "rom7", 0x40000, 0x20000, CRC(d209db4d) SHA1(1cf85d39d12e92c1b97f7e5a148f3ad56cdca963) )
	ROM_LOAD16_BYTE( "rom8", 0x40001, 0x20000, CRC(d3646728) SHA1(898606be662214d2ba99e9a3e3cc0c7e7609a719) )

	ROM_REGION( 0x20000, "audiocpu", 0 )
	ROM_LOAD( "rom2",        0x000000, 0x08000, CRC(75b194cf) SHA1(6aaf36cdab06c0aa5328f5176557387a5d3f7d26) )
	ROM_CONTINUE(            0x010000, 0x08000 )
	ROM_COPY( "audiocpu", 0x000000, 0x018000, 0x08000 )

	ROM_REGION( 0x20000, "gfx1", 0 )
	ROM_LOAD( "rom3", 0x00000, 0x10000, CRC(fbb241be) SHA1(cd94c328891538bbd8c062d90a47ddf3d7d05bb0) )  /* characters */
	ROM_LOAD( "rom4", 0x10000, 0x10000, CRC(ad3cde81) SHA1(2bd0c707e5b67d3699a743d989cb5384cbe37ff7) )

	ROM_REGION( 0x100000, "gfx2", 0 )
	ROM_LOAD( "rom9", 0x00000, 0x100000, CRC(b0a5eecf) SHA1(9e8191c7ae4a32dc16aebc37fa942afc531eddd4) ) /* Background + Foreground */

	ROM_REGION( 0x80000, "gfx3", 0 )
	ROM_LOAD( "rom10", 0x00000, 0x080000, CRC(1bbcda5d) SHA1(63915221f70a7dfda6a4d8ac7f5c663c9316610a) )    /* sprites */

	ROM_REGION( 0x40000, "oki", 0 ) /* ADPCM samples */
	ROM_LOAD( "rom1", 0x00000, 0x20000, CRC(e69986f6) SHA1(de38bf2d5638cb40740882e1abccf7928e43a5a6) )
ROM_END


/* Game Drivers */

GAME( 1990, bloodbro, 0,        bloodbro, bloodbro, driver_device, 0, ROT0,   "TAD Corporation", "Blood Bros. (set 1)", MACHINE_NO_COCKTAIL | MACHINE_SUPPORTS_SAVE )
GAME( 1990, bloodbroa,bloodbro, bloodbro, bloodbro, driver_device, 0, ROT0,   "TAD Corporation", "Blood Bros. (set 2)", MACHINE_NO_COCKTAIL | MACHINE_SUPPORTS_SAVE )
GAME( 1990, bloodbrob,bloodbro, bloodbro, bloodbro, driver_device, 0, ROT0,   "TAD Corporation", "Blood Bros. (set 3)", MACHINE_NO_COCKTAIL | MACHINE_SUPPORTS_SAVE )
GAME( 1990, weststry, bloodbro, weststry, weststry, driver_device, 0, ROT0,   "bootleg (Datsu)", "West Story (bootleg of Blood Bros.)", MACHINE_NO_COCKTAIL | MACHINE_NO_SOUND | MACHINE_SUPPORTS_SAVE )
GAME( 1990, skysmash, 0,        skysmash, skysmash, driver_device, 0, ROT270, "Nihon System",    "Sky Smasher", MACHINE_SUPPORTS_SAVE )
