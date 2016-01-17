// license:BSD-3-Clause
// copyright-holders:Paul Priest, David Haywood
/******************************************************************************

  'Face' LINDA board
 driver by Paul Priest + David Haywood

*******************************************************************************

 Games on this Hardware

 Magical Cat Adventure (c)1993 Wintechno
 Nostradamus (c)1993 Face

*******************************************************************************

 Hardware Overview:

Magical Cat (C) 1993 Wintechno
Board Name: LINDA5

 Main CPU: 68000-16
 Sound CPU: Z80
 Sound Chip: YMF286-K

 Custom: FACE FX1037 x1
         038 x2 (As in Cave)


Nostradamus (C) 1993 FACE
Board Name: LINDA25

  Main CPU: MC68000P12F 16MHz
 Sound CPU: Z8400B PS (Goldstar)
Sound chip: YMF286-K & Y3016-F

Graphics chips:
176 Pin PQFP 038 9330EX705
176 Pin PQFP 038 9320EX702
176 Pin PQFP FX1037 FACE FA01-2075 (Face Custom)

OSC 28.000 MHz - SEOAN SX0-T100
OSC 16.000 MHz - Sunny SC0-010T

8 Way DIP Switch x 2
Push Button Test Switch

Roms:
NOS-PO-U 2740000PC-15 (68k Program) U29 - Odd
NOS-PE-U 2740000PC-15 (68k Program) U30 - Even
NOS-PS     D27C020-15 (Z80 program) U9

As labelled on PCB, with location:
NOS-SO-00.U83-
NOS-SO-01.U85 \
NOS-SO-02.U87  | Sprites Odd/Even (These are 27C8001)
NOS-SE-00.U82  |
NOS-SE-01.U84 /
NOS-SE-02.U86-
U92 & U93 are unpopulated

NOS-SN-00.U53 Sound samples (Near the YMF286-K)

NOS-B0-00.U58-
NOS-B0-01.U59 \ Background (separate for each 038 chip?)
NOS-B1-00.U60 /
NOS-B1-01.U61-

YMF286-K is compatible to YM2610 - see psikyo.c driver
038 9320EX702 / 038 9330EX705    - see Cave.c driver

Note # = Pin #1    PCB Layout:

+----------------------------------------------------------------------------+
| ___________                                                                |_
|| NOS-B1-00 |                                                                J|
|#___________|                ________   ________                             A|
| ___________   __________   |NOS-PO-U| |NOS-PE-U|                            M|
|| NOS-B1-01 | |          |  #________| #________|                            M|
|#___________| | 038      |   ___________________    _______                  A|
|              | 9330EX705|  |   MC68000P12F     |  |NOS-PS |                  |
|              |__________#  |   16MHz           |  #_______|                 C|
|                            #___________________|  ___________               o|
| ___________   __________                         | Z8400B PS |              n|
|| NOS-B0-00 | |          |                        #___________|              n|
|#___________| | 038      |                        ______________             e|
| ___________  | 9320EX702|                   SW1 |   YMF286-K   |            c|
|| NOS-B0-01 | |__________#     _________         #______________|            t|
|#___________|                 |FX1037   #  SW2                    _______    i|
|                              |(C) Face |         ___________    |Y3016-F#   o|
|                              |FA01-2075|        | NOS-SN-00 |   |_______|   n|
|                              |_________|        #___________|               _|
| ______                   ___  ___  ___       ___  ___  ___                 |
||OSC 28|                 # N |# N |# N |  E  # N |# N |# N | E              |
|#______|                 | O || O || O |  m  | O || O || O | m              |
|                         | S || S || S |  p  | S || S || S | p              |
| Empty                   | | || | || | |  t  | | || | || | | t              |
|  OSC                    | S || S || S |  y  | S || S || S | y              |
| ______                  | E || E || E |     | O || O || O |                |
||OSC 16|                 | | || | || | |  S  | | || | || | | S              |
|#______|                 | 0 || 0 || 0 |  C  | 0 || 0 || 0 | C              |
|                         | 0 || 1 || 2 |  K  | 0 || 1 || 2 | K              |
|    PUSHBTN              |___||___||___|  T  |___||___||___| T              |
+----------------------------------------------------------------------------+

*******************************************************************************

Stephh's notes (based on the games M68000 code and some tests) :

1) "mcatadv*'

  - Player 1 Button 3 is only used in the "test" mode :
      * to select "OBJECT ROM CHECK"
      * in "BG ROM", to change the background number

  - Do NOT trust the "NORMAL TESTMODE" for the system inputs !

  - The Japan version has extra GFX/anims and it's harder than the other set.

2) 'nost*'

  - When entering the "test mode", you need to press SERVICE1 to cycle through
    the different screens.

*******************************************************************************

 todo:

 Flip Screen

*******************************************************************************

 trivia:

 Magical Cat Adventure tests for 'MASICAL CAT ADVENTURE' in RAM on start-up
 and will write it there if not found, expecting a reset, great engrish ;-)

******************************************************************************/

#include "emu.h"
#include "cpu/z80/z80.h"
#include "cpu/m68000/m68000.h"
#include "sound/2610intf.h"
#include "includes/mcatadv.h"


/*** Main CPU ***/

WRITE16_MEMBER(mcatadv_state::mcat_soundlatch_w)
{
	soundlatch_byte_w(space, 0, data);
	m_soundcpu->set_input_line(INPUT_LINE_NMI, PULSE_LINE);
}

#if 0 // mcat only.. install read handler?
WRITE16_MEMBER(mcatadv_state::mcat_coin_w)
{
	if(ACCESSING_BITS_8_15)
	{
		machine().bookkeeping().coin_counter_w(0, data & 0x1000);
		machine().bookkeeping().coin_counter_w(1, data & 0x2000);
		machine().bookkeeping().coin_lockout_w(0, ~data & 0x4000);
		machine().bookkeeping().coin_lockout_w(1, ~data & 0x8000);
	}
}
#endif

READ16_MEMBER(mcatadv_state::mcat_wd_r)
{
	watchdog_reset_r(space, 0);
	return 0xc00;
}


static ADDRESS_MAP_START( mcatadv_map, AS_PROGRAM, 16, mcatadv_state )
	AM_RANGE(0x000000, 0x0fffff) AM_ROM
	AM_RANGE(0x100000, 0x10ffff) AM_RAM

//  AM_RANGE(0x180018, 0x18001f) AM_READNOP // ?

	AM_RANGE(0x200000, 0x200005) AM_RAM AM_SHARE("scroll1")
	AM_RANGE(0x300000, 0x300005) AM_RAM AM_SHARE("scroll2")

	AM_RANGE(0x400000, 0x401fff) AM_RAM_WRITE(mcatadv_videoram1_w) AM_SHARE("videoram1") // Tilemap 0
	AM_RANGE(0x500000, 0x501fff) AM_RAM_WRITE(mcatadv_videoram2_w) AM_SHARE("videoram2") // Tilemap 1

	AM_RANGE(0x600000, 0x601fff) AM_RAM_DEVWRITE("palette", palette_device, write) AM_SHARE("palette")
	AM_RANGE(0x602000, 0x602fff) AM_RAM // Bigger than needs to be?

	AM_RANGE(0x700000, 0x707fff) AM_RAM AM_SHARE("spriteram") // Sprites, two halves for double buffering
	AM_RANGE(0x708000, 0x70ffff) AM_RAM // Tests more than is needed?

	AM_RANGE(0x800000, 0x800001) AM_READ_PORT("P1")
	AM_RANGE(0x800002, 0x800003) AM_READ_PORT("P2")
//  AM_RANGE(0x900000, 0x900001) AM_WRITE(mcat_coin_w) // Lockout / Counter MCAT Only
	AM_RANGE(0xa00000, 0xa00001) AM_READ_PORT("DSW1")
	AM_RANGE(0xa00002, 0xa00003) AM_READ_PORT("DSW2")

	AM_RANGE(0xb00000, 0xb0000f) AM_RAM AM_SHARE("vidregs")

	AM_RANGE(0xb00018, 0xb00019) AM_WRITE(watchdog_reset16_w) // NOST Only
	AM_RANGE(0xb0001e, 0xb0001f) AM_READ(mcat_wd_r) // MCAT Only
	AM_RANGE(0xc00000, 0xc00001) AM_READ(soundlatch2_word_r) AM_WRITE(mcat_soundlatch_w)
ADDRESS_MAP_END

/*** Sound ***/

WRITE8_MEMBER(mcatadv_state::mcatadv_sound_bw_w)
{
	membank("bank1")->set_entry(data);
}


static ADDRESS_MAP_START( mcatadv_sound_map, AS_PROGRAM, 8, mcatadv_state )
	AM_RANGE(0x0000, 0x3fff) AM_ROM                     // ROM
	AM_RANGE(0x4000, 0xbfff) AM_ROMBANK("bank1")                // ROM
	AM_RANGE(0xc000, 0xdfff) AM_RAM                     // RAM
	AM_RANGE(0xe000, 0xe003) AM_DEVREADWRITE("ymsnd", ym2610_device, read, write)
	AM_RANGE(0xf000, 0xf000) AM_WRITE(mcatadv_sound_bw_w)
ADDRESS_MAP_END

static ADDRESS_MAP_START( mcatadv_sound_io_map, AS_IO, 8, mcatadv_state )
	ADDRESS_MAP_GLOBAL_MASK(0xff)
	AM_RANGE(0x80, 0x80) AM_READWRITE(soundlatch_byte_r, soundlatch2_byte_w)
ADDRESS_MAP_END


static ADDRESS_MAP_START( nost_sound_map, AS_PROGRAM, 8, mcatadv_state )
	AM_RANGE(0x0000, 0x7fff) AM_ROM                     // ROM
	AM_RANGE(0x8000, 0xbfff) AM_ROMBANK("bank1")                // ROM
	AM_RANGE(0xc000, 0xdfff) AM_RAM                     // RAM
ADDRESS_MAP_END

static ADDRESS_MAP_START( nost_sound_io_map, AS_IO, 8, mcatadv_state )
	ADDRESS_MAP_GLOBAL_MASK(0xff)
	AM_RANGE(0x00, 0x03) AM_DEVWRITE("ymsnd", ym2610_device, write)
	AM_RANGE(0x04, 0x07) AM_DEVREAD("ymsnd", ym2610_device, read)
	AM_RANGE(0x40, 0x40) AM_WRITE(mcatadv_sound_bw_w)
	AM_RANGE(0x80, 0x80) AM_READWRITE(soundlatch_byte_r, soundlatch2_byte_w)
ADDRESS_MAP_END

/*** Inputs ***/

static INPUT_PORTS_START( mcatadv )
	PORT_START("P1")
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_PLAYER(1)
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_PLAYER(1)
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_PLAYER(1)
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_PLAYER(1)
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(1)   // "Fire"
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(1)   // "Jump"
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_PLAYER(1)   // See notes
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x0100, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0xfe00, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("P2")
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_PLAYER(2)
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_PLAYER(2)
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_PLAYER(2)
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_PLAYER(2)
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(2)   // "Fire"
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(2)   // "Jump"
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_START2 )
	PORT_BIT( 0x0100, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_BIT( 0x0200, IP_ACTIVE_LOW, IPT_SERVICE1 )
	PORT_BIT( 0xfc00, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("DSW1")
	PORT_BIT( 0x00ff, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_DIPNAME( 0x0100, 0x0100, DEF_STR( Demo_Sounds ) )      PORT_DIPLOCATION("SW1:1")
	PORT_DIPSETTING(      0x0000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0100, DEF_STR( On ) )
	PORT_DIPNAME( 0x0200, 0x0200, DEF_STR( Flip_Screen ) )      PORT_DIPLOCATION("SW1:2")
	PORT_DIPSETTING(      0x0200, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_SERVICE_DIPLOC(   0x0400, IP_ACTIVE_LOW, "SW1:3" )
	PORT_DIPNAME( 0x0800, 0x0800, "Coin Mode" )         PORT_DIPLOCATION("SW1:4")
	PORT_DIPSETTING(      0x0800, "Mode 1" )
	PORT_DIPSETTING(      0x0000, "Mode 2" )
	PORT_DIPNAME( 0x3000, 0x3000, DEF_STR( Coin_A ) )       PORT_DIPLOCATION("SW1:5,6")
	PORT_DIPSETTING(      0x0000, DEF_STR( 4C_1C ) )    PORT_CONDITION("DSW1", 0x0800, EQUALS, 0x0000)
	PORT_DIPSETTING(      0x1000, DEF_STR( 3C_1C ) )    PORT_CONDITION("DSW1", 0x0800, EQUALS, 0x0000)
	PORT_DIPSETTING(      0x1000, DEF_STR( 2C_1C ) )    PORT_CONDITION("DSW1", 0x0800, EQUALS, 0x0800)
	PORT_DIPSETTING(      0x3000, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( 2C_3C ) )    PORT_CONDITION("DSW1", 0x0800, EQUALS, 0x0800)
	PORT_DIPSETTING(      0x2000, DEF_STR( 1C_2C ) )    PORT_CONDITION("DSW1", 0x0800, EQUALS, 0x0800)
	PORT_DIPSETTING(      0x2000, DEF_STR( 1C_4C ) )    PORT_CONDITION("DSW1", 0x0800, EQUALS, 0x0000)
	PORT_DIPNAME( 0xc000, 0xc000, DEF_STR( Coin_B ) )       PORT_DIPLOCATION("SW1:7,8")
	PORT_DIPSETTING(      0x0000, DEF_STR( 4C_1C ) )    PORT_CONDITION("DSW1", 0x0800, EQUALS, 0x0000)
	PORT_DIPSETTING(      0x4000, DEF_STR( 3C_1C ) )    PORT_CONDITION("DSW1", 0x0800, EQUALS, 0x0000)
	PORT_DIPSETTING(      0x4000, DEF_STR( 2C_1C ) )    PORT_CONDITION("DSW1", 0x0800, EQUALS, 0x0800)
	PORT_DIPSETTING(      0xc000, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( 2C_3C ) )    PORT_CONDITION("DSW1", 0x0800, EQUALS, 0x0800)
	PORT_DIPSETTING(      0x8000, DEF_STR( 1C_2C ) )    PORT_CONDITION("DSW1", 0x0800, EQUALS, 0x0800)
	PORT_DIPSETTING(      0x8000, DEF_STR( 1C_4C ) )    PORT_CONDITION("DSW1", 0x0800, EQUALS, 0x0000)

	PORT_START("DSW2")
	PORT_BIT( 0x00ff, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_DIPNAME( 0x0300, 0x0300, DEF_STR( Difficulty ) )       PORT_DIPLOCATION("SW2:1,2")
	PORT_DIPSETTING(      0x0200, DEF_STR( Easy ) )
	PORT_DIPSETTING(      0x0300, DEF_STR( Normal ) )
	PORT_DIPSETTING(      0x0100, DEF_STR( Hard ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( Hardest ) )
	PORT_DIPNAME( 0x0c00, 0x0c00, DEF_STR( Lives ) )        PORT_DIPLOCATION("SW2:3,4")
	PORT_DIPSETTING(      0x0400, "2" )
	PORT_DIPSETTING(      0x0c00, "3" )
	PORT_DIPSETTING(      0x0800, "4" )
	PORT_DIPSETTING(      0x0000, "5" )
	PORT_DIPNAME( 0x3000, 0x3000, "Energy" )            PORT_DIPLOCATION("SW2:5,6")
	PORT_DIPSETTING(      0x3000, "3" )
	PORT_DIPSETTING(      0x2000, "4" )
	PORT_DIPSETTING(      0x1000, "5" )
	PORT_DIPSETTING(      0x0000, "8" )
	PORT_DIPNAME( 0xc000, 0xc000, DEF_STR( Cabinet ) )      PORT_DIPLOCATION("SW2:7,8")
	PORT_DIPSETTING(      0x4000, "Upright 1 Player" )
	PORT_DIPSETTING(      0xc000, "Upright 2 Players" )
	PORT_DIPSETTING(      0x8000, DEF_STR( Cocktail ) )
//  PORT_DIPSETTING(      0x0000, "Upright 2 Players" )       // duplicated setting (NEVER tested)
INPUT_PORTS_END

static INPUT_PORTS_START( nost )
	PORT_START("P1")
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_PLAYER(1)
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_PLAYER(1)
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_PLAYER(1)
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_PLAYER(1)
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(1)
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_UNKNOWN )                  // Button 2 in "test mode"
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_UNKNOWN )                  // Button 3 in "test mode"
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x0100, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x0200, IP_ACTIVE_LOW, IPT_UNKNOWN )                  // "test" 3 in "test mode"
	PORT_BIT( 0x0800, IP_ACTIVE_HIGH, IPT_UNKNOWN )                 // Must be LOW or startup freezes !
	PORT_BIT( 0xf400, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("P2")
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_PLAYER(2)
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_PLAYER(2)
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_PLAYER(2)
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_PLAYER(2)
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(2)
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_UNKNOWN )                  // Button 2 in "test mode"
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_UNKNOWN )                  // Button 3 in "test mode"
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_START2 )
	PORT_BIT( 0x0100, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_BIT( 0x0200, IP_ACTIVE_LOW, IPT_SERVICE1 )
	PORT_BIT( 0xfc00, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("DSW1")
	PORT_DIPNAME( 0x0300, 0x0300, DEF_STR( Lives ) )        PORT_DIPLOCATION("SW1:1,2")
	PORT_DIPSETTING(      0x0200, "2" )
	PORT_DIPSETTING(      0x0300, "3" )
	PORT_DIPSETTING(      0x0100, "4" )
	PORT_DIPSETTING(      0x0000, "5" )
	PORT_DIPNAME( 0x0c00, 0x0c00, DEF_STR( Difficulty ) )       PORT_DIPLOCATION("SW1:3,4")
	PORT_DIPSETTING(      0x0800, DEF_STR( Easy ) )
	PORT_DIPSETTING(      0x0c00, DEF_STR( Normal ) )
	PORT_DIPSETTING(      0x0400, DEF_STR( Hard ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( Hardest ) )
	PORT_DIPNAME( 0x1000, 0x1000, DEF_STR( Flip_Screen ) )      PORT_DIPLOCATION("SW1:5")
	PORT_DIPSETTING(      0x1000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x2000, 0x2000, DEF_STR( Demo_Sounds ) )      PORT_DIPLOCATION("SW1:6")
	PORT_DIPSETTING(      0x0000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x2000, DEF_STR( On ) )
	PORT_DIPNAME( 0xc000, 0xc000, DEF_STR( Bonus_Life ) )       PORT_DIPLOCATION("SW1:7,8")
	PORT_DIPSETTING(      0x8000, "500k 1000k" )
	PORT_DIPSETTING(      0xc000, "800k 1500k" )
	PORT_DIPSETTING(      0x4000, "1000k 2000k" )
	PORT_DIPSETTING(      0x0000, DEF_STR( None ) )

	PORT_START("DSW2")
	PORT_DIPNAME( 0x0700, 0x0700, DEF_STR( Coin_A ) )       PORT_DIPLOCATION("SW2:1,2,3")
	PORT_DIPSETTING(      0x0200, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(      0x0400, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(      0x0100, DEF_STR( 3C_2C ) )
	PORT_DIPSETTING(      0x0700, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(      0x0300, DEF_STR( 2C_3C ) )
	PORT_DIPSETTING(      0x0600, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(      0x0500, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( Free_Play ) )
	PORT_DIPNAME( 0x3800, 0x3800, DEF_STR( Coin_B ) )       PORT_DIPLOCATION("SW2:4,5,6")
	PORT_DIPSETTING(      0x0000, DEF_STR( 4C_1C ) )
	PORT_DIPSETTING(      0x1000, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(      0x2000, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(      0x0800, DEF_STR( 3C_2C ) )
	PORT_DIPSETTING(      0x3800, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(      0x1800, DEF_STR( 2C_3C ) )
	PORT_DIPSETTING(      0x3000, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(      0x2800, DEF_STR( 1C_3C ) )
	PORT_DIPUNUSED_DIPLOC( 0x4000, 0x4000, "SW2:7" )        /* Listed as "Unused" */
	PORT_SERVICE_DIPLOC(   0x8000, IP_ACTIVE_LOW, "SW2:8" )
INPUT_PORTS_END

/*** GFX Decode ***/

static const gfx_layout mcatadv_tiles16x16x4_layout =
{
	16,16,
	RGN_FRAC(1,1),
	4,
	{ STEP4(0,1) },
	{ STEP8(0,4), STEP8(32*8,4) },
	{ STEP8(0,32), STEP8(64*8,32) },
	128*8
};

static GFXDECODE_START( mcatadv )
	GFXDECODE_ENTRY( "gfx2", 0, mcatadv_tiles16x16x4_layout, 0, 0x200 )
	GFXDECODE_ENTRY( "gfx3", 0, mcatadv_tiles16x16x4_layout, 0, 0x200 )
GFXDECODE_END


/* Stolen from Psikyo.c */
WRITE_LINE_MEMBER(mcatadv_state::sound_irq)
{
	m_soundcpu->set_input_line(0, state ? ASSERT_LINE : CLEAR_LINE);
}


void mcatadv_state::machine_start()
{
	UINT8 *ROM = memregion("soundcpu")->base();

	membank("bank1")->configure_entries(0, 8, &ROM[0x10000], 0x4000);
	membank("bank1")->set_entry(1);


	save_item(NAME(m_palette_bank1));
	save_item(NAME(m_palette_bank2));
}

static MACHINE_CONFIG_START( mcatadv, mcatadv_state )

	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", M68000, XTAL_16MHz) /* verified on pcb */
	MCFG_CPU_PROGRAM_MAP(mcatadv_map)
	MCFG_CPU_VBLANK_INT_DRIVER("screen", mcatadv_state,  irq1_line_hold)

	MCFG_CPU_ADD("soundcpu", Z80, XTAL_16MHz/4) /* verified on pcb */
	MCFG_CPU_PROGRAM_MAP(mcatadv_sound_map)
	MCFG_CPU_IO_MAP(mcatadv_sound_io_map)


	/* video hardware */
	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_REFRESH_RATE(60)
	MCFG_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(0))
	MCFG_SCREEN_SIZE(320, 256)
	MCFG_SCREEN_VISIBLE_AREA(0, 320-1, 0, 224-1)
	MCFG_SCREEN_UPDATE_DRIVER(mcatadv_state, screen_update_mcatadv)
	MCFG_SCREEN_VBLANK_DRIVER(mcatadv_state, screen_eof_mcatadv)
	MCFG_SCREEN_PALETTE("palette")

	MCFG_GFXDECODE_ADD("gfxdecode", "palette", mcatadv)
	MCFG_PALETTE_ADD("palette", 0x2000/2)
	MCFG_PALETTE_FORMAT(xGGGGGRRRRRBBBBB)

	MCFG_WATCHDOG_TIME_INIT(attotime::from_seconds(3))  /* a guess, and certainly wrong */


	/* sound hardware */
	MCFG_SPEAKER_STANDARD_STEREO("lspeaker", "rspeaker")

	MCFG_SOUND_ADD("ymsnd", YM2610, XTAL_16MHz/2) /* verified on pcb */
	MCFG_YM2610_IRQ_HANDLER(WRITELINE(mcatadv_state, sound_irq))
	MCFG_SOUND_ROUTE(0, "lspeaker",  0.32)
	MCFG_SOUND_ROUTE(0, "rspeaker", 0.32)
	MCFG_SOUND_ROUTE(1, "lspeaker",  0.5)
	MCFG_SOUND_ROUTE(2, "rspeaker", 0.5)
MACHINE_CONFIG_END

static MACHINE_CONFIG_DERIVED( nost, mcatadv )

	MCFG_CPU_MODIFY("soundcpu")
	MCFG_CPU_PROGRAM_MAP(nost_sound_map)
	MCFG_CPU_IO_MAP(nost_sound_io_map)

	MCFG_DEVICE_REMOVE("lspeaker")
	MCFG_DEVICE_REMOVE("rspeaker")
	MCFG_SPEAKER_STANDARD_MONO("mono")
	MCFG_SOUND_REPLACE("ymsnd", YM2610, XTAL_16MHz/2) /* verified on pcb */
		MCFG_YM2610_IRQ_HANDLER(WRITELINE(mcatadv_state, sound_irq))
		MCFG_SOUND_ROUTE(0, "mono", 0.2)
		MCFG_SOUND_ROUTE(1, "mono", 0.5)
		MCFG_SOUND_ROUTE(2, "mono", 0.5)
MACHINE_CONFIG_END


ROM_START( mcatadv )
	ROM_REGION( 0x100000, "maincpu", 0 ) /* M68000 */
	ROM_LOAD16_BYTE( "mca-u30e", 0x00000, 0x80000, CRC(c62fbb65) SHA1(39a30a165d4811141db8687a4849626bef8e778e) )
	ROM_LOAD16_BYTE( "mca-u29e", 0x00001, 0x80000, CRC(cf21227c) SHA1(4012811ebfe3c709ab49946f8138bc4bad881ef7) )

	ROM_REGION( 0x030000, "soundcpu", 0 ) /* Z80-A */
	ROM_LOAD( "u9.bin", 0x00000, 0x20000, CRC(fda05171) SHA1(2c69292573ec35034572fa824c0cae2839d23919) )
	ROM_RELOAD( 0x10000, 0x20000 )

	ROM_REGION( 0x800000, "gfx1", ROMREGION_ERASEFF ) /* Sprites */
	ROM_LOAD16_BYTE( "mca-u82.bin", 0x000000, 0x100000, CRC(5f01d746) SHA1(11b241456e15299912ee365eedb8f9d5e5ca875d) )
	ROM_LOAD16_BYTE( "mca-u83.bin", 0x000001, 0x100000, CRC(4e1be5a6) SHA1(cb19aad42dba54d6a4a33859f27254c2a3271e8c) )
	ROM_LOAD16_BYTE( "mca-u84.bin", 0x200000, 0x080000, CRC(df202790) SHA1(f6ae54e799af195860ed0ab3c85138cf2f10efa6) )
	ROM_LOAD16_BYTE( "mca-u85.bin", 0x200001, 0x080000, CRC(a85771d2) SHA1(a1817cd72f5bf0a4f24a37c782dc63ecec3b8e68) )
	ROM_LOAD16_BYTE( "mca-u86e",    0x400000, 0x080000, CRC(017bf1da) SHA1(f6446a7219275c0eff62129f59fdfa3a6a3e06c8) )
	ROM_LOAD16_BYTE( "mca-u87e",    0x400001, 0x080000, CRC(bc9dc9b9) SHA1(f525c9f994d5107752aa4d3a499ee376ec75f42b) )

	ROM_REGION( 0x080000, "gfx2", 0 ) /* BG0 */
	ROM_LOAD( "mca-u58.bin", 0x000000, 0x080000, CRC(3a8186e2) SHA1(129c220d72608a8839f779ce1a6cfec8646dbf23) )

	ROM_REGION( 0x280000, "gfx3", 0 ) /* BG1 */
	ROM_LOAD( "mca-u60.bin", 0x000000, 0x100000, CRC(c8942614) SHA1(244fccb9abbb04e33839dd2cd0e2de430819a18c) )
	ROM_LOAD( "mca-u61.bin", 0x100000, 0x100000, CRC(51af66c9) SHA1(1055cf78ea286f02003b0d1bf08c2d7829b36f90) )
	ROM_LOAD( "mca-u100",    0x200000, 0x080000, CRC(b273f1b0) SHA1(39318fe2aaf2792b85426ec6791b3360ac964de3) )

	ROM_REGION( 0x80000, "ymsnd", 0 ) /* Samples - ADPCM or Delta-T? */
	ROM_LOAD( "mca-u53.bin", 0x00000, 0x80000, CRC(64c76e05) SHA1(379cef5e0cba78d0e886c9cede41985850a3afb7) )
ROM_END

ROM_START( mcatadvj )
	ROM_REGION( 0x100000, "maincpu", 0 ) /* M68000 */
	ROM_LOAD16_BYTE( "u30.bin", 0x00000, 0x80000, CRC(05762f42) SHA1(3675fb606bf9d7be9462324e68263f4a6c2fea1c) )
	ROM_LOAD16_BYTE( "u29.bin", 0x00001, 0x80000, CRC(4c59d648) SHA1(2ab77ea254f2c11fc016078cedcab2fffbe5ee1b) )

	ROM_REGION( 0x030000, "soundcpu", 0 ) /* Z80-A */
	ROM_LOAD( "u9.bin", 0x00000, 0x20000, CRC(fda05171) SHA1(2c69292573ec35034572fa824c0cae2839d23919) )
	ROM_RELOAD( 0x10000, 0x20000 )

	ROM_REGION( 0x800000, "gfx1", ROMREGION_ERASEFF ) /* Sprites */
	ROM_LOAD16_BYTE( "mca-u82.bin", 0x000000, 0x100000, CRC(5f01d746) SHA1(11b241456e15299912ee365eedb8f9d5e5ca875d) )
	ROM_LOAD16_BYTE( "mca-u83.bin", 0x000001, 0x100000, CRC(4e1be5a6) SHA1(cb19aad42dba54d6a4a33859f27254c2a3271e8c) )
	ROM_LOAD16_BYTE( "mca-u84.bin", 0x200000, 0x080000, CRC(df202790) SHA1(f6ae54e799af195860ed0ab3c85138cf2f10efa6) )
	ROM_LOAD16_BYTE( "mca-u85.bin", 0x200001, 0x080000, CRC(a85771d2) SHA1(a1817cd72f5bf0a4f24a37c782dc63ecec3b8e68) )
	ROM_LOAD16_BYTE( "u86.bin",     0x400000, 0x080000, CRC(2d3725ed) SHA1(8b4c0f280eb901113d842848ffc26371be7b6067) )
	ROM_LOAD16_BYTE( "u87.bin",     0x400001, 0x080000, CRC(4ddefe08) SHA1(5ade0a694d73f4f3891c1ab7757e37a88afcbf54) )

	ROM_REGION( 0x080000, "gfx2", 0 ) /* BG0 */
	ROM_LOAD( "mca-u58.bin", 0x000000, 0x080000, CRC(3a8186e2) SHA1(129c220d72608a8839f779ce1a6cfec8646dbf23) )

	ROM_REGION( 0x280000, "gfx3", 0 ) /* BG1 */
	ROM_LOAD( "mca-u60.bin", 0x000000, 0x100000, CRC(c8942614) SHA1(244fccb9abbb04e33839dd2cd0e2de430819a18c) )
	ROM_LOAD( "mca-u61.bin", 0x100000, 0x100000, CRC(51af66c9) SHA1(1055cf78ea286f02003b0d1bf08c2d7829b36f90) )
	ROM_LOAD( "u100.bin",    0x200000, 0x080000, CRC(e2c311da) SHA1(cc3217484524de94704869eaa9ce1b90393039d8) )

	ROM_REGION( 0x80000, "ymsnd", 0 ) /* Samples - ADPCM or Delta-T? */
	ROM_LOAD( "mca-u53.bin", 0x00000, 0x80000, CRC(64c76e05) SHA1(379cef5e0cba78d0e886c9cede41985850a3afb7) )
ROM_END

ROM_START( catt )
	ROM_REGION( 0x100000, "maincpu", 0 ) /* M68000 */
	ROM_LOAD16_BYTE( "catt-u30.bin",  0x00000, 0x80000, CRC(8c921e1e) SHA1(2fdaa9b743e1731f3cfe9d8334f1b759cf46855d) )
	ROM_LOAD16_BYTE( "catt-u29.bin",  0x00001, 0x80000, CRC(e725af6d) SHA1(78c08fa5744a6a953e13c0ff39736ccd4875fb72) )

	ROM_REGION( 0x030000, "soundcpu", 0 ) /* Z80-A */
	ROM_LOAD( "u9.bin", 0x00000, 0x20000, CRC(fda05171) SHA1(2c69292573ec35034572fa824c0cae2839d23919) )
	ROM_RELOAD( 0x10000, 0x20000 )

	ROM_REGION( 0x800000, "gfx1", ROMREGION_ERASEFF ) /* Sprites */
	ROM_LOAD16_BYTE( "mca-u82.bin", 0x000000, 0x100000, CRC(5f01d746) SHA1(11b241456e15299912ee365eedb8f9d5e5ca875d) )
	ROM_LOAD16_BYTE( "mca-u83.bin", 0x000001, 0x100000, CRC(4e1be5a6) SHA1(cb19aad42dba54d6a4a33859f27254c2a3271e8c) )
	ROM_LOAD16_BYTE( "u84.bin",     0x200000, 0x100000, CRC(843fd624) SHA1(2e16d8a909fe9447da37a87428bff0734af59a00) )
	ROM_LOAD16_BYTE( "u85.bin",     0x200001, 0x100000, CRC(5ee7b628) SHA1(feedc212ed4893d784dc6b3361930b9199c6876d) )
	ROM_LOAD16_BYTE( "mca-u86e",    0x400000, 0x080000, CRC(017bf1da) SHA1(f6446a7219275c0eff62129f59fdfa3a6a3e06c8) )
	ROM_LOAD16_BYTE( "mca-u87e",    0x400001, 0x080000, CRC(bc9dc9b9) SHA1(f525c9f994d5107752aa4d3a499ee376ec75f42b) )

	ROM_REGION( 0x100000, "gfx2", 0 ) /* BG0 */
	ROM_LOAD( "u58.bin",     0x00000, 0x100000, CRC(73c9343a) SHA1(9efdddbad6244c1ed267bd954563ab43a1017c96) )

	ROM_REGION( 0x280000, "gfx3", 0 ) /* BG1 */
	ROM_LOAD( "mca-u60.bin", 0x000000, 0x100000, CRC(c8942614) SHA1(244fccb9abbb04e33839dd2cd0e2de430819a18c) )
	ROM_LOAD( "mca-u61.bin", 0x100000, 0x100000, CRC(51af66c9) SHA1(1055cf78ea286f02003b0d1bf08c2d7829b36f90) )
	ROM_LOAD( "mca-u100",    0x200000, 0x080000, CRC(b273f1b0) SHA1(39318fe2aaf2792b85426ec6791b3360ac964de3) )

	ROM_REGION( 0x100000, "ymsnd", 0 ) /* Samples - ADPCM or Delta-T? */
	ROM_LOAD( "u53.bin",     0x00000, 0x100000, CRC(99f2a624) SHA1(799e8e40e8bdcc8fa4cd763a366cc32473038a49) )

	ROM_REGION( 0x0400, "plds", 0 )
	ROM_LOAD( "peel18cv8.u1", 0x0000, 0x0155, NO_DUMP )
	ROM_LOAD( "gal16v8a.u10", 0x0200, 0x0117, NO_DUMP )
	ROM_END

ROM_START( nost )
	ROM_REGION( 0x100000, "maincpu", 0 ) /* M68000 */
	ROM_LOAD16_BYTE( "nos-pe-u.bin", 0x00000, 0x80000, CRC(4b080149) SHA1(e1dbbe5bf554c7c5731cc3079850f257417e3caa) )
	ROM_LOAD16_BYTE( "nos-po-u.bin", 0x00001, 0x80000, CRC(9e3cd6d9) SHA1(db5351ff9a05f602eceae62c0051c16ae0e4ead9) )

	ROM_REGION( 0x050000, "soundcpu", 0 ) /* Z80-A */
	ROM_LOAD( "nos-ps.u9", 0x00000, 0x40000, CRC(832551e9) SHA1(86fc481b1849f378c88593594129197c69ea1359) )
	ROM_RELOAD( 0x10000, 0x40000 )

	ROM_REGION( 0x800000, "gfx1", ROMREGION_ERASEFF ) /* Sprites */
	ROM_LOAD16_BYTE( "nos-se-0.u82", 0x000000, 0x100000, CRC(9d99108d) SHA1(466540989d7b1b7f6dc7acbae74f6a8201973d45) )
	ROM_LOAD16_BYTE( "nos-so-0.u83", 0x000001, 0x100000, CRC(7df0fc7e) SHA1(2e064cb5367b2839d736d339c4f1a44785b4eedf) )
	ROM_LOAD16_BYTE( "nos-se-1.u84", 0x200000, 0x100000, CRC(aad07607) SHA1(89c51a9cb6b8d8ed3a357f5d8ac8399ff1c7ad46) )
	ROM_LOAD16_BYTE( "nos-so-1.u85", 0x200001, 0x100000, CRC(83d0012c) SHA1(831d36521693891f44e7adcc2ba63fef5d493821) )
	ROM_LOAD16_BYTE( "nos-se-2.u86", 0x400000, 0x080000, CRC(d99e6005) SHA1(49aae72111334ff5cd0fd86500882f559ff921f9) )
	ROM_LOAD16_BYTE( "nos-so-2.u87", 0x400001, 0x080000, CRC(f60e8ef3) SHA1(4f7472b5a465e6cc6a5df520ebfe6a544739dd28) )

	ROM_REGION( 0x180000, "gfx2", 0 ) /* BG0 */
	ROM_LOAD( "nos-b0-0.u58", 0x000000, 0x100000, CRC(0214b0f2) SHA1(678fa3dc739323bda6d7bbb1c7a573c976d69356) )
	ROM_LOAD( "nos-b0-1.u59", 0x100000, 0x080000, CRC(3f8b6b34) SHA1(94c48614782ce6405965bcf6029e3bcc24a6d84f) )

	ROM_REGION( 0x180000, "gfx3", 0 ) /* BG1 */
	ROM_LOAD( "nos-b1-0.u60", 0x000000, 0x100000, CRC(ba6fd0c7) SHA1(516d6e0c4dc6fb12ec9f30877ea1c582e7440a19) )
	ROM_LOAD( "nos-b1-1.u61", 0x100000, 0x080000, CRC(dabd8009) SHA1(1862645b8d6216c3ec2b8dbf74816b8e29dea14f) )

	ROM_REGION( 0x100000, "ymsnd", 0 ) /* Samples - ADPCM or Delta-T? */
	ROM_LOAD( "nossn-00.u53", 0x00000, 0x100000, CRC(3bd1bcbc) SHA1(1bcad43792e985402db4eca122676c2c555f3313) )
ROM_END

ROM_START( nostj )
	ROM_REGION( 0x100000, "maincpu", 0 ) /* M68000 */
	ROM_LOAD16_BYTE( "nos-pe-j.u30", 0x00000, 0x80000, CRC(4b080149) SHA1(e1dbbe5bf554c7c5731cc3079850f257417e3caa) )
	ROM_LOAD16_BYTE( "nos-po-j.u29", 0x00001, 0x80000, CRC(7fe241de) SHA1(aa4ffd81cb73efc59690c2038ae9375021a775a4) )

	ROM_REGION( 0x050000, "soundcpu", 0 ) /* Z80-A */
	ROM_LOAD( "nos-ps.u9", 0x00000, 0x40000, CRC(832551e9) SHA1(86fc481b1849f378c88593594129197c69ea1359) )
	ROM_RELOAD( 0x10000, 0x40000 )

	ROM_REGION( 0x800000, "gfx1", ROMREGION_ERASEFF ) /* Sprites */
	ROM_LOAD16_BYTE( "nos-se-0.u82", 0x000000, 0x100000, CRC(9d99108d) SHA1(466540989d7b1b7f6dc7acbae74f6a8201973d45) )
	ROM_LOAD16_BYTE( "nos-so-0.u83", 0x000001, 0x100000, CRC(7df0fc7e) SHA1(2e064cb5367b2839d736d339c4f1a44785b4eedf) )
	ROM_LOAD16_BYTE( "nos-se-1.u84", 0x200000, 0x100000, CRC(aad07607) SHA1(89c51a9cb6b8d8ed3a357f5d8ac8399ff1c7ad46) )
	ROM_LOAD16_BYTE( "nos-so-1.u85", 0x200001, 0x100000, CRC(83d0012c) SHA1(831d36521693891f44e7adcc2ba63fef5d493821) )
	ROM_LOAD16_BYTE( "nos-se-2.u86", 0x400000, 0x080000, CRC(d99e6005) SHA1(49aae72111334ff5cd0fd86500882f559ff921f9) )
	ROM_LOAD16_BYTE( "nos-so-2.u87", 0x400001, 0x080000, CRC(f60e8ef3) SHA1(4f7472b5a465e6cc6a5df520ebfe6a544739dd28) )

	ROM_REGION( 0x180000, "gfx2", 0 ) /* BG0 */
	ROM_LOAD( "nos-b0-0.u58", 0x000000, 0x100000, CRC(0214b0f2) SHA1(678fa3dc739323bda6d7bbb1c7a573c976d69356) )
	ROM_LOAD( "nos-b0-1.u59", 0x100000, 0x080000, CRC(3f8b6b34) SHA1(94c48614782ce6405965bcf6029e3bcc24a6d84f) )

	ROM_REGION( 0x180000, "gfx3", 0 ) /* BG1 */
	ROM_LOAD( "nos-b1-0.u60", 0x000000, 0x100000, CRC(ba6fd0c7) SHA1(516d6e0c4dc6fb12ec9f30877ea1c582e7440a19) )
	ROM_LOAD( "nos-b1-1.u61", 0x100000, 0x080000, CRC(dabd8009) SHA1(1862645b8d6216c3ec2b8dbf74816b8e29dea14f) )

	ROM_REGION( 0x100000, "ymsnd", 0 ) /* Samples - ADPCM or Delta-T? */
	ROM_LOAD( "nossn-00.u53", 0x00000, 0x100000, CRC(3bd1bcbc) SHA1(1bcad43792e985402db4eca122676c2c555f3313) )
ROM_END

ROM_START( nostk )
	ROM_REGION( 0x100000, "maincpu", 0 ) /* M68000 */
	ROM_LOAD16_BYTE( "nos-pe-t.u30", 0x00000, 0x80000, CRC(bee5fbc8) SHA1(a8361fa004bb31471f973ece51a9a87b9f3438ab) )
	ROM_LOAD16_BYTE( "nos-po-t.u29", 0x00001, 0x80000, CRC(f4736331) SHA1(7a6db2db1a4dbf105c22e15deff6f6032e04609c) )

	ROM_REGION( 0x050000, "soundcpu", 0 ) /* Z80-A */
	ROM_LOAD( "nos-ps.u9", 0x00000, 0x40000, CRC(832551e9) SHA1(86fc481b1849f378c88593594129197c69ea1359) )
	ROM_RELOAD( 0x10000, 0x40000 )

	ROM_REGION( 0x800000, "gfx1", ROMREGION_ERASEFF ) /* Sprites */
	ROM_LOAD16_BYTE( "nos-se-0.u82", 0x000000, 0x100000, CRC(9d99108d) SHA1(466540989d7b1b7f6dc7acbae74f6a8201973d45) )
	ROM_LOAD16_BYTE( "nos-so-0.u83", 0x000001, 0x100000, CRC(7df0fc7e) SHA1(2e064cb5367b2839d736d339c4f1a44785b4eedf) )
	ROM_LOAD16_BYTE( "nos-se-1.u84", 0x200000, 0x100000, CRC(aad07607) SHA1(89c51a9cb6b8d8ed3a357f5d8ac8399ff1c7ad46) )
	ROM_LOAD16_BYTE( "nos-so-1.u85", 0x200001, 0x100000, CRC(83d0012c) SHA1(831d36521693891f44e7adcc2ba63fef5d493821) )
	ROM_LOAD16_BYTE( "nos-se-2.u86", 0x400000, 0x080000, CRC(d99e6005) SHA1(49aae72111334ff5cd0fd86500882f559ff921f9) )
	ROM_LOAD16_BYTE( "nos-so-2.u87", 0x400001, 0x080000, CRC(f60e8ef3) SHA1(4f7472b5a465e6cc6a5df520ebfe6a544739dd28) )

	ROM_REGION( 0x180000, "gfx2", 0 ) /* BG0 */
	ROM_LOAD( "nos-b0-0.u58", 0x000000, 0x100000, CRC(0214b0f2) SHA1(678fa3dc739323bda6d7bbb1c7a573c976d69356) )
	ROM_LOAD( "nos-b0-1.u59", 0x100000, 0x080000, CRC(3f8b6b34) SHA1(94c48614782ce6405965bcf6029e3bcc24a6d84f) )

	ROM_REGION( 0x180000, "gfx3", 0 ) /* BG1 */
	ROM_LOAD( "nos-b1-0.u60", 0x000000, 0x100000, CRC(ba6fd0c7) SHA1(516d6e0c4dc6fb12ec9f30877ea1c582e7440a19) )
	ROM_LOAD( "nos-b1-1.u61", 0x100000, 0x080000, CRC(dabd8009) SHA1(1862645b8d6216c3ec2b8dbf74816b8e29dea14f) )

	ROM_REGION( 0x100000, "ymsnd", 0 ) /* Samples - ADPCM or Delta-T? */
	ROM_LOAD( "nossn-00.u53", 0x00000, 0x100000, CRC(3bd1bcbc) SHA1(1bcad43792e985402db4eca122676c2c555f3313) )
ROM_END


GAME( 1993, mcatadv,  0,       mcatadv, mcatadv, driver_device, 0, ROT0,   "Wintechno", "Magical Cat Adventure", MACHINE_NO_COCKTAIL | MACHINE_SUPPORTS_SAVE )
GAME( 1993, mcatadvj, mcatadv, mcatadv, mcatadv, driver_device, 0, ROT0,   "Wintechno", "Magical Cat Adventure (Japan)", MACHINE_NO_COCKTAIL | MACHINE_SUPPORTS_SAVE )
GAME( 1993, catt,     mcatadv, mcatadv, mcatadv, driver_device, 0, ROT0,   "Wintechno", "Catt (Japan)", MACHINE_NO_COCKTAIL | MACHINE_SUPPORTS_SAVE )
GAME( 1993, nost,     0,       nost,    nost, driver_device,    0, ROT270, "Face",      "Nostradamus", MACHINE_NO_COCKTAIL | MACHINE_SUPPORTS_SAVE )
GAME( 1993, nostj,    nost,    nost,    nost, driver_device,    0, ROT270, "Face",      "Nostradamus (Japan)", MACHINE_NO_COCKTAIL | MACHINE_SUPPORTS_SAVE )
GAME( 1993, nostk,    nost,    nost,    nost, driver_device,    0, ROT270, "Face",      "Nostradamus (Korea)", MACHINE_NO_COCKTAIL | MACHINE_SUPPORTS_SAVE )
