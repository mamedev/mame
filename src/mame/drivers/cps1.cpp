// license:???
// copyright-holders:Paul Leaman
/***************************************************************************

Capcom System 1
===============

Driver provided by:
Paul Leaman (paul@vortexcomputing.demon.co.uk)

68000 for game, Z80, YM-2151 and OKIM6295 for sound.

68000 clock speeds are unknown for all games (except where commented)

todo: move the bootleg sets with modified hardware into their own
      drivers, like fcrash.c


Notes
-----
- All original PALs are protected. The images loaded in the ROM definitions are
  logically equivalent replacements with a GAL16V8 target. The actual pld types
  found on the boards can also be PAL16P8, PAL16L8 etc.

- There are five PALs on the A-Board, they are the same across all A-Board
  revisions, but their location varies.
  The A-Boards are interchangeable, so the IC locations are not specified in
  the ROM definitions and are listed here instead:

  PAL ID  88617A  89626A
  ------  ------  ------
  PRG1    12H     9K
  IOA1    12F     4A
  BUF1    16H     13K
  ROM1    15H     14K
  SOU1    13E     10F

  For Q-Sound games, the A-Board misses the Z80, Oki and amp, which are replaced
  by the D-Board; the SOU1 PAL is missing as well, while PRG1 is replaced by
  PRG2. The other PALs are the same.

- The B-board usually has two PALs (later revisions have three). The first PAL
  is used to map tile codes to the graphics ROMs, and changes from game to game.
  The other doesn't change from game to game and there are only two versions,
  called LWIO and IOB1, which are almost identical, the only difference being
  that outputs o16 and o17 are R/W in IOB1 and Write only in LWIO. Since those
  outputs are not used anyway except that in Forgotten Worlds, one wonders why
  they were changed.

  SF2 revision "E" World and USA 910228 maps CPS-B at 8001C0 instead of 800140
  and seems to be the only one that uses the IOB2 and C632B PALs instead of the
  IOB1 and C632, while STF29 PAL is confirmed to be the same as the other SF2 sets.

  pin 1  CN2 D9  = /IOCS (i.e. address 8xxxxx)
  pin 2  CN2 D7  = /RDB
  pin 3  CN2 C6  = /UDSWR
  pin 4  CN2 D6  = /LDSWR
  pin 5  CN2 C19 = A8
  pin 6  CN2 C20 = A7
  pin 7  CN2 C21 = A6
  pin 8  CN2 C22 = A5
  pin 9  CN2 C23 = A4
  pin 10 GND
  pin 11 CN2 C24 = A3
  pin 12 CN2 C25 = A2

  LWIO:
  /o19 = /i1 * /i2 * /i5 * /i6 *  i7 *  i8 * /i9            800060-80006f R    not used
  /o18 = /i1 * /i2 * /i5 * /i6 *  i7 * /i8 *  i9            800050-80005f R    forgotten worlds dial (on 88618B & 88621B B-boards)
   o17 = /i1 * /i4 * /i5 * /i6 *  i7 * /i8 * /i9 * /i11     800040-800047 W    forgotten worlds dial (on 88618B & 88621B B-boards)
   o16 = /i1 * /i4 * /i5 * /i6 *  i7 * /i8 * /i9 *  i11     800048-80004f W    forgotten worlds dial (on 88618B & 88621B B-boards)
  /o15 = /i1 *  i5 * /i6 *  i7                              800140-80017f R/W  CPS-B
  /o14 = /i1 * /i4 * /i5 * /i6 * /i7 *  i8 *  i9 * /i11     800030-800037 W    not used (overlaps coinctrl on A-board)
  /o13 = /i1 * /i2 * /i5 * /i6 * /i7 * /i8 * /i9 *  i11     800008-80000f R    not used
  /o12 = /i1 * /i2 * /i5 * /i6 * /i7 * /i8 *  i9 *  i11     800018-80001f R    not used (overlaps system/dsw inputs on A-board)

  IOB1:
  /o19 = /i1 * /i2 * /i5 * /i6 *  i7 *  i8 * /i9            800060-80006f R    not used
  /o18 = /i1 * /i2 * /i5 * /i6 *  i7 * /i8 *  i9            800050-80005f R    not used
   o17 = /i1 * /i5 * /i6 *  i7 * /i8 * /i9 * /i11           800040-800047 R/W  not used
   o16 = /i1 * /i5 * /i6 *  i7 * /i8 * /i9 *  i11           800048-80004f R/W  not used
  /o15 = /i1 *  i5 * /i6 *  i7                              800140-80017f R/W  CPS-B
  /o14 = /i1 * /i4 * /i5 * /i6 * /i7 *  i8 *  i9 * /i11     800030-800037 W    not used (overlaps coinctrl on A-board)
  /o13 = /i1 * /i2 * /i5 * /i6 * /i7 * /i8 * /i9 *  i11     800008-80000f R    not used
  /o12 = /i1 * /i2 * /i5 * /i6 * /i7 * /i8 *  i9 *  i11     800018-80001f R    not used (overlaps system/dsw inputs on A-board)



Stephh's notes (based on the games M68000 code and some tests) :

1) 'forgottn' and clones

  - Keys in the "scroll 1 test" screen :
      * P1 UP    : char code += 0x0200
      * P1 DOWN  : char code -= 0x0200
      * P1 RIGHT : attribute ^= 0x20 ; also affects Hflip
      * P1 LEFT  : attribute ^= 0x40 ; also affects Vflip
      * P1 button 1 : (attribute++) & 0x1f ; also affects color
      * P1 button 2 : (attribute--) & 0x1f ; also affects color

2) 'ghouls' and clones

2a) 'ghouls'

  - NO debug features

2b) 'ghoulsu'

  - How to activate the debug features :
      * set "Game Mode" Dip Switch to "Game"
      * set both "Coin A" and "Coin B" Dip Switches to 1C_1C
      * reset the game (F3 key)
      * insert a coin
      * set "Game Mode" Dip Switch to "Test"
      * set the debug Dip Switches to what you want
      * start a 1 player game
  - Some debug features :
      * "Armor on New Life" is effective at the beginning of a new life
        Note that even when you start without armor, you need to be hit twice
      * "Starting Weapon" is effective only when you start a new game
        or when you continue play
      * "Starting Level" is effective only when you start a new game
        (you must NOT continue play !)
      * "Slow Motion" and "Invulnerability" can be changed at any time

2c) 'daimakai'

  - NO debug features
  - Different Dip Switches than in 'ghouls'

3) 'strider' and clones

  - TO DO !

4) 'dynwar' and clones

4a) 'dynwar'

  - According to code at 0x0125fa and 0x012634, bits 0 and 1 of DSWC also affect the energy cost
    (table at 0x012662) when you press button 3 ("tactics" in the manual) :

    bit 0  bit 1    cost         possible BCD values (based on kind of "tactics")
     OFF    OFF     normal       0x10 (rockslide) 0x08 (ambush) 0x14 (flame) 0x12 (explosion)
     ON     OFF     very low     0x03             0x02          0x05         0x04
     OFF    ON      low          0x06             0x04          0x10         0x08
     ON     ON      high         0x13             0x10          0x19         0x16

    So IMO, there might be an ingame bug at 0x000c7e which should read bits 2 and 3 of DSWC
    (instead of having them unused) :
    000C7E: 0240 0003                andi.w  #$3, D0
    should be
    000C7E: 0240 000c                andi.w  #$c, D0
    However, only bits 4 to 7 are tested while you are in the "test mode".

    Note that code at 0x000c7e is only executed once when you reset the machine;
    this means that you can change the "Freeze" and "Turbo Mode" Dip Switches before
    reseting the game to change the energy cost settings, then put them back to "Off"
    after the startup routine (when screen is black) so you can play the game.
  - Both "Coin A" and "Coin B" Dip Switches must be set to "2 Coins/1 Credit" to make
    this mode available. If only one Dip Switch is set to it, it is the same as 2C_1C.

4b) 'dynwarj'

  - The "energy cost" bug also exists in this version as well as code at 0x000c7e;
    in fact, only code that reads these settings is at a different address :
    check code at 0x01267a and 0x0126b4 and table at 0x0126e2.
  - Both "Coin A" and "Coin B" Dip Switches must be set to "2 Coins/1 Credit" to make
    this mode available. If only one Dip Switch is set to it, it is the same as 2C_1C.

5) 'willow' and clones

  - How to activate the debug features :
      * set "Game Mode" Dip Switch to "Game"
      * set both "Coin A" and "Coin B" Dip Switches to 1C_1C
      * reset the game (F3 key)
      * insert a coin
      * set "Game Mode" Dip Switch to "Test"
      * set the debug Dip Switches to what you want
      * start a 1 player game
  - Some debug features :
      * "Starting Level" is effective only when you start a new game
        (you must NOT continue play !)
      * Once you set "Maximum magic/sword power" to "On", setting it to "Off" won't have
        any effect until you start a new game (you must NOT continue play !)
      * "Slow Motion Delay" is effective only when "Slow Motion" is set to "On"
      * Even if "Freeze" Dip Switch is set to "On" a message will be displayed to its end.
      * I can't tell what kind of infos are displayed when "Display Debug Infos"
        Dip Switch is sry to "On" :( Any hint about them are welcome !
  - Both "Coin A" and "Coin B" Dip Switches must be set to "2 Coins/1 Credit" to make
    this mode available. If only one Dip Switch is set to it, it is the same as 2C_1C.
  - When the "Stage Magic Continue" Dip Switch is set to "On", your magic and sword power
    will be increased at the end of the level if you haven't bought the magic/sword item.
    But you won't notice this before you use the character again.
    For example, magic power will be increased at the end of level 1 but you won't notice
    it before level 3, and sword power will be increased at the end of level 2 but you
    won't notice it before level 4.

6) 'ganbare'

  - Using the payout setting dip switch results in some occasional hopper errors, if this
    happens, then the clear ram dip switch needs to be used. This needs to be resolved.

TO DO (2006.09.20) :

  - Check 'strider' and its clones and add debug features
  - Check other games to see if there are some debug or hidden features
  - Add addresses from routines with debug features in the notes
  - Look at what IN2 and IN3 do for the following sets :
      * 'cworld2j' (IN2 only)
      * 'qad' and 'qadjr'
      * 'qtono2j'
  - Check daimakar dip switches. E.g. changing the number of lives also changes the
    starting level.

Stephh's log (2006.09.20) :

  - Changed the "readinputport" reads by "readinputportbytag" reads.
    This way, inputs and Dip Switch can be in any order
    (and I don't have the nasty conditional Dip Switch bug).
    BTW, I've slightly changed the read memory map :
      * one handler for the 3 Dip Switches banks : cps1_dsw_r
      * one handler for the system inputs (IN0) : cps1_in0_r
      * one handler for the player inputs (IN1) : cps1_in1_r
      * renamed cps1_input2_r to cps_in2_r
      * renamed cps1_input3_r to cps_in3_r
  - Applied these changes to src/drivers/fcrash.c as well.
  - Added debug features in the following sets :
      * 'ghoulsu'
      * 'willowu', 'willowou' and 'willowj'
  - Checked sets with no debug features :
      * 'forgottnua' and 'lostwrld'
      * 'ghouls' and 'daimakai'
      * 'dynwar' and 'dynwarj'

2008-07:
  - replaced input read handler with direct AM_READ_PORT where suitable

***************************************************************************/

#include "emu.h"
#include "cpu/z80/z80.h"
#include "cpu/pic16c5x/pic16c5x.h"
#include "cpu/m68000/m68000.h"
#include "machine/eepromser.h"
#include "sound/2151intf.h"
#include "sound/okim6295.h"
#include "sound/qsound.h"
#include "machine/kabuki.h"
#include "includes/cps1.h"



READ16_MEMBER(cps_state::cps1_dsw_r)
{
	static const char *const dswname[] = { "IN0", "DSWA", "DSWB", "DSWC" };
	int in = ioport(dswname[offset])->read();
	return (in << 8) | 0xff;
}

READ16_MEMBER(cps_state::cps1_hack_dsw_r)
{
	static const char *const dswname[] = { "IN0", "DSWA", "DSWB", "DSWC" };
	int in = ioport(dswname[offset])->read();
	return (in << 8) | in;
}

READ16_MEMBER(cps_state::cps1_in1_r)
{
	int in = ioport("IN1")->read();
	return (in << 8) | in;
}

READ16_MEMBER(cps_state::cps1_in2_r)
{
	int in = ioport("IN2")->read();
	return (in << 8) | in;
}

READ16_MEMBER(cps_state::cps1_in3_r)
{
	int in = ioport("IN3")->read();
	return (in << 8) | in;
}

READ16_MEMBER(cps_state::forgottn_dial_0_r)
{
	return ((ioport("DIAL0")->read() - m_dial[0]) >> (8 * offset)) & 0xff;
}

READ16_MEMBER(cps_state::forgottn_dial_1_r)
{
	return ((ioport("DIAL1")->read() - m_dial[1]) >> (8 * offset)) & 0xff;
}

WRITE16_MEMBER(cps_state::forgottn_dial_0_reset_w)
{
	m_dial[0] = ioport("DIAL0")->read();
}

WRITE16_MEMBER(cps_state::forgottn_dial_1_reset_w)
{
	m_dial[1] = ioport("DIAL1")->read();
}


WRITE8_MEMBER(cps_state::cps1_snd_bankswitch_w)
{
	membank("bank1")->set_entry(data & 0x01);
}

WRITE8_MEMBER(cps_state::cps1_oki_pin7_w)
{
	m_oki->set_pin7(data & 1);
}

WRITE16_MEMBER(cps_state::cps1_soundlatch_w)
{
	if (ACCESSING_BITS_0_7)
		soundlatch_byte_w(space, 0, data & 0xff);
	else
		soundlatch_byte_w(space, 0, data >> 8);
}

WRITE16_MEMBER(cps_state::cps1_soundlatch2_w)
{
	if (ACCESSING_BITS_0_7)
		soundlatch2_byte_w(space, 0, data & 0xff);
}

WRITE16_MEMBER(cps_state::cps1_coinctrl_w)
{
	if (ACCESSING_BITS_8_15)
	{
		coin_counter_w(machine(), 0, data & 0x0100);
		coin_counter_w(machine(), 1, data & 0x0200);
		coin_lockout_w(machine(), 0, ~data & 0x0400);
		coin_lockout_w(machine(), 1, ~data & 0x0800);

		// bit 15 = CPS-A custom reset?
	}
}

WRITE16_MEMBER(cps_state::cpsq_coinctrl2_w)
{
	if (ACCESSING_BITS_0_7)
	{
		coin_counter_w(machine(), 2, data & 0x01);
		coin_lockout_w(machine(), 2, ~data & 0x02);
		coin_counter_w(machine(), 3, data & 0x04);
		coin_lockout_w(machine(), 3, ~data & 0x08);
	}
}

INTERRUPT_GEN_MEMBER(cps_state::cps1_interrupt)
{
	/* Strider also has a IRQ4 handler. It is input port related, but the game */
	/* works without it. It is the *only* CPS1 game to have that. */
	/* ...until we found out that ganbare relies on it, see below */
	device.execute().set_input_line(2, HOLD_LINE);
}

TIMER_DEVICE_CALLBACK_MEMBER(cps_state::ganbare_interrupt)
{
	/* not sure on the timing or source of this - the game needs it once per frame, */
	/* otherwise you get a "HARD ERROR" after boot */
	if (param == 0)
		m_maincpu->set_input_line(4, HOLD_LINE);
}


/********************************************************************
*
*  Q Sound
*  =======
*
********************************************************************/

READ16_MEMBER(cps_state::qsound_rom_r)
{
	UINT8 *rom = memregion("user1")->base();

	if (rom)
		return rom[offset] | 0xff00;
	else
	{
		popmessage("%06x: read sound ROM byte %04x", space.device().safe_pc(), offset);
		return 0;
	}
}

READ16_MEMBER(cps_state::qsound_sharedram1_r)
{
	return m_qsound_sharedram1[offset] | 0xff00;
}

WRITE16_MEMBER(cps_state::qsound_sharedram1_w)
{
	if (ACCESSING_BITS_0_7)
		m_qsound_sharedram1[offset] = data;
}

READ16_MEMBER(cps_state::qsound_sharedram2_r)
{
	return m_qsound_sharedram2[offset] | 0xff00;
}

WRITE16_MEMBER(cps_state::qsound_sharedram2_w)
{
	if (ACCESSING_BITS_0_7)
		m_qsound_sharedram2[offset] = data;
}

WRITE8_MEMBER(cps_state::qsound_banksw_w)
{
	/* Z80 bank register for music note data. It's odd that it isn't encrypted though. */
	int bank = data & 0x0f;
	if ((0x10000 + (bank * 0x4000)) >= memregion("audiocpu")->bytes())
	{
		logerror("WARNING: Q sound bank overflow (%02x)\n", data);
		bank = 0;
	}

	membank("bank1")->set_entry(bank);
}


/********************************************************************
*
*  EEPROM
*  ======
*
*   The EEPROM is accessed by a serial protocol using the register
*   0xf1c006
*
********************************************************************/

/*
PAL PRG1 (16P8B @ 12H):

I0 = AS
I1 = /BGACKB
I2 = A23
I3 = A22
I4 = A21
I5 = A20
I6 = A19
I7 = A18
I8 = A17
I9 = A16

n.c.      = pin19 =   ( !I0 &  I1 )
n.c.      = pin18 =   ( !I0 &  I1 )
n.c.      = pin17 = ! (  I0 &  I1 & (!I2 | !I3 | !I4 | !I5 | !I6 | !I7) )
n.c.      = pin16 = ! (  I2 &  I3 &  I4 &  I5 &  I6 &  I7 )
/IOCS     = pin15 = ! (  I0 &  I1 &  I2 & !I3 & !I4 & !I5 )
/ONE WAIT = pin14 = ! (  I0 &  I1 & ( I2 | !I3) )
/databus  = pin13 = ! (  I0 &  I1 & (!I2 | !I3 | !I4 | !I5 | !I6 | !I7) )
/workram  = pin12 = ! (  I0 &  I1 &  I2 &  I3 &  I4 &  I5 &  I6 &  I7 )


In Q-Sound games, PRG1 is replaced by PRG2:

/IOCS     = pin 15 = ! (  I0 &  I1 &  I2 & !I3 & !I4 & !I5 )
/ONE WAIT = pin 14 = ! (  I0 &  I1 & (!I2 | !I3 | !I4 | !I5 | ( I6 &  I7 &  I8 &  I9)) )
/databus  = pin 13 = ! (  I0 &  I1 & (!I2 | !I3 | !I4 | !I5 | (!I6 & !I7 & !I8)) )
/workram  = pin 12 = ! (  I0 &  I1 &  I2 &  I3 &  I4 &  I5 &  I6 &  I7 &  I8 &  I9 )





PAL IOA1 (16P8B @ 12F):

I0 = /IOCS
I1 = /RDB
I2 = /UDSWR
I3 = /LDSWR
I4 = AB8
I5 = AB7
I6 = AB6
I7 = AB5
I8 = AB4
I9 = AB3

player input      = pin19 = ! ( !I0 & !I1 & !I4 & !I5 & !I6 & !I7 & !I8 & !I9 )
system input/dips = pin18 = ! ( !I0 & !I1 & !I4 & !I5 & !I6 & !I7 &  I8 &  I9 )
outputs           = pin17 = ! ( !I0 & !I2 & !I4 & !I5 & !I6 &  I7 &  I8 & !I9 )
sound 1B          = pin16 = ! ( !I0 & !I3 &  I4 &  I5 & !I6 & !I7 & !I8 &  I9 )
sound 0B          = pin15 = ! ( !I0 & !I3 &  I4 &  I5 & !I6 & !I7 & !I8 & !I9 )
n.c.              = pin14 =   ( !I1 & !I2 )
/PPU1             = pin13 = ! ( !I0 &  I4 & !I5 & !I6 )
n.c.              = pin12 =   ( !I1 & !I2 )


PAL BUF1 (16P8B @ 16H):

I0 = A23 (all address lines can come both from 68000 and CPS-A custom)
I1 = A22
I2 = A21
I3 = A20
I4 = A19
I5 = A18
I6 = A17
I7 = A16
I8 = ASB

BUF0 = pin19 = ! ( I0 & !I1 & !I2 &  I3 & !I4 & !I5 & !I6 & !I7 )
BUF1 = pin18 = ! ( I0 & !I1 & !I2 &  I3 & !I4 & !I5 & !I6 &  I7 )
BUF2 = pin17 = ! ( I0 & !I1 & !I2 &  I3 & !I4 & !I5 &  I6 & !I7 )
BUF3 = pin16 = ! ( I0 & !I1 & !I2 &  I3 & !I4 & !I5 &  I6 &  I7 )
BUF4 = pin15 = ! ( I0 & !I1 & !I2 &  I3 & !I4 &  I5 & !I6 & !I7 )
BUF5 = pin14 = ! ( I0 & !I1 & !I2 &  I3 & !I4 &  I5 & !I6 &  I7 )
BUF6 = pin13 = ! ( I0 & !I1 & !I2 &  I3 & !I4 &  I5 &  I6 & !I7 )
/RDB = pin12 =   ( I0 & !I1 & !I2 &  I3 & !I8 )

BUF0-BUF2 are gfxram on A-board. BUF3-BUF6 go to B-board (provision for expansion, never used)


PAL ROM1 (16P8B @ 15H):

I0 = A23 (all address lines can come both from 68000 and CPS-A custom)
I1 = A22
I2 = A21
I3 = A20
I4 = A19
I5 = A18
I6 = A17
I7 = A16
I8 = ASB

PRG0 = pin17 = ! ( !I8 & !I0 & !I1 & !I2 & !I3 & !I4 & !I5 )
PRG1 = pin16 = ! ( !I8 & !I0 & !I1 & !I2 & !I3 & !I4 &  I5 )
PRG2 = pin15 = ! ( !I8 & !I0 & !I1 & !I2 & !I3 &  I4 & !I5 )
PRG3 = pin14 = ! ( !I8 & !I0 & !I1 & !I2 & !I3 &  I4 &  I5 )
PRG4 = pin19 = ! ( !I8 & !I0 & !I1 & !I2 &  I3 & !I4 )
PRG5 = pin18 = ! ( !I8 & !I0 & !I1 & !I2 &  I3 &  I4 )
PRG6 = pin13 = ! ( !I8 & !I0 & !I1 &  I2 & !I3 & !I4 )
/RDB = pin12 =   ( !I8 & !I0 & !I1 )

All PRGx go to B-board. Provision for up to 4MB of ROM space, which was never used in full.

*/

static ADDRESS_MAP_START( main_map, AS_PROGRAM, 16, cps_state )
	AM_RANGE(0x000000, 0x3fffff) AM_ROM
	AM_RANGE(0x800000, 0x800007) AM_READ_PORT("IN1")            /* Player input ports */
	/* forgottn, willow, cawing, nemo, varth read from 800010. Probably debug input leftover from development */
	AM_RANGE(0x800018, 0x80001f) AM_READ(cps1_dsw_r)            /* System input ports / Dip Switches */
	AM_RANGE(0x800020, 0x800021) AM_READNOP                     /* ? Used by Rockman ? not mapped according to PAL */
	AM_RANGE(0x800030, 0x800037) AM_WRITE(cps1_coinctrl_w)
	/* Forgotten Worlds has dial controls on B-board mapped at 800040-80005f. See DRIVER_INIT */
	AM_RANGE(0x800100, 0x80013f) AM_WRITE(cps1_cps_a_w) AM_SHARE("cps_a_regs")  /* CPS-A custom */
	/* CPS-B custom is mapped by the PAL IOB2 on the B-board. SF2 revision "E" World and USA 910228 has it at a different
	   address, see DRIVER_INIT */
	AM_RANGE(0x800140, 0x80017f) AM_READWRITE(cps1_cps_b_r, cps1_cps_b_w) AM_SHARE("cps_b_regs")
	AM_RANGE(0x800180, 0x800187) AM_WRITE(cps1_soundlatch_w)    /* Sound command */
	AM_RANGE(0x800188, 0x80018f) AM_WRITE(cps1_soundlatch2_w)   /* Sound timer fade */
	AM_RANGE(0x900000, 0x92ffff) AM_RAM_WRITE(cps1_gfxram_w) AM_SHARE("gfxram") /* SF2CE executes code from here */
	AM_RANGE(0xff0000, 0xffffff) AM_RAM AM_SHARE("mainram")
ADDRESS_MAP_END

/*
PAL SOU1 (16P8 @ 13E):

I0 = /MREQ
I1 = A15
I2 = A14
I3 = A13
I4 = A12
I5 = bank latch
I6 = /RD
I7 = /WR
I8 = /BANK

bank latch = pin19 = ! ( !I0 & !I7 & !I8 )
/SWR       = pin18 = ! ( !I0 & !I7 )
/SRD       = pin17 = ! ( !I0 & !I6 )
ls138      = pin16 = ! ( !I0 &  I1 &  I2 &  I3 &  I4 )
workram    = pin15 = ! ( !I0 &  I1 &  I2 & !I3 &  I4 )
SOUNDA14   = pin14 = ! ( !I0 & ((!I1 & !I2) | ( I1 & !I2 & !I5)) )
SOUNDA15   = pin13 =   (  I1 )
/SOUNDCE   = pin12 = ! ( !I0 & (!I1 | ( I1 & !I2)) )
*/

static ADDRESS_MAP_START( sub_map, AS_PROGRAM, 8, cps_state )
	AM_RANGE(0x0000, 0x7fff) AM_ROM
	AM_RANGE(0x8000, 0xbfff) AM_ROMBANK("bank1")
	AM_RANGE(0xd000, 0xd7ff) AM_RAM
	AM_RANGE(0xf000, 0xf001) AM_DEVREADWRITE("2151", ym2151_device, read, write)
	AM_RANGE(0xf002, 0xf002) AM_DEVREADWRITE("oki", okim6295_device, read, write)
	AM_RANGE(0xf004, 0xf004) AM_WRITE(cps1_snd_bankswitch_w)
	AM_RANGE(0xf006, 0xf006) AM_WRITE(cps1_oki_pin7_w) /* controls pin 7 of OKI chip */
	AM_RANGE(0xf008, 0xf008) AM_READ(soundlatch_byte_r) /* Sound command */
	AM_RANGE(0xf00a, 0xf00a) AM_READ(soundlatch2_byte_r) /* Sound timer fade */
ADDRESS_MAP_END

static ADDRESS_MAP_START( qsound_main_map, AS_PROGRAM, 16, cps_state )
	AM_RANGE(0x000000, 0x1fffff) AM_ROM
	AM_RANGE(0x800000, 0x800007) AM_READ_PORT("IN1")            /* Player input ports */
	AM_RANGE(0x800018, 0x80001f) AM_READ(cps1_dsw_r)            /* System input ports / Dip Switches */
	AM_RANGE(0x800030, 0x800037) AM_WRITE(cps1_coinctrl_w)
	AM_RANGE(0x800100, 0x80013f) AM_WRITE(cps1_cps_a_w) AM_SHARE("cps_a_regs")  /* CPS-A custom */
	AM_RANGE(0x800140, 0x80017f) AM_READWRITE(cps1_cps_b_r, cps1_cps_b_w) AM_SHARE("cps_b_regs")    /* CPS-B custom (mapped by LWIO/IOB1 PAL on B-board) */
	AM_RANGE(0x900000, 0x92ffff) AM_RAM_WRITE(cps1_gfxram_w) AM_SHARE("gfxram") /* SF2CE executes code from here */
	AM_RANGE(0xf00000, 0xf0ffff) AM_READ(qsound_rom_r)          /* Slammasters protection */
	AM_RANGE(0xf18000, 0xf19fff) AM_READWRITE(qsound_sharedram1_r, qsound_sharedram1_w)  /* Q RAM */
	AM_RANGE(0xf1c000, 0xf1c001) AM_READ_PORT("IN2")            /* Player 3 controls (later games) */
	AM_RANGE(0xf1c002, 0xf1c003) AM_READ_PORT("IN3")            /* Player 4 controls ("Muscle Bombers") */
	AM_RANGE(0xf1c004, 0xf1c005) AM_WRITE(cpsq_coinctrl2_w)     /* Coin control2 (later games) */
	AM_RANGE(0xf1c006, 0xf1c007) AM_READ_PORT("EEPROMIN") AM_WRITE_PORT("EEPROMOUT")
	AM_RANGE(0xf1e000, 0xf1ffff) AM_READWRITE(qsound_sharedram2_r, qsound_sharedram2_w)  /* Q RAM */
	AM_RANGE(0xff0000, 0xffffff) AM_RAM AM_SHARE("mainram")
ADDRESS_MAP_END

ADDRESS_MAP_START( qsound_sub_map, AS_PROGRAM, 8, cps_state )   // used by cps2.c too
	AM_RANGE(0x0000, 0x7fff) AM_ROM
	AM_RANGE(0x8000, 0xbfff) AM_ROMBANK("bank1")    /* banked (contains music data) */
	AM_RANGE(0xc000, 0xcfff) AM_RAM AM_SHARE("qsound_ram1")
	AM_RANGE(0xd000, 0xd002) AM_DEVWRITE("qsound", qsound_device, qsound_w)
	AM_RANGE(0xd003, 0xd003) AM_WRITE(qsound_banksw_w)
	AM_RANGE(0xd007, 0xd007) AM_DEVREAD("qsound", qsound_device, qsound_r)
	AM_RANGE(0xf000, 0xffff) AM_RAM AM_SHARE("qsound_ram2")
ADDRESS_MAP_END

ADDRESS_MAP_START( qsound_decrypted_opcodes_map, AS_DECRYPTED_OPCODES, 8, cps_state )
	AM_RANGE(0x0000, 0x7fff) AM_ROMBANK("decrypted")
ADDRESS_MAP_END

static ADDRESS_MAP_START( sf2m3_map, AS_PROGRAM, 16, cps_state )
	AM_RANGE(0x000000, 0x3fffff) AM_ROM
	AM_RANGE(0x800010, 0x800011) AM_READ_PORT("IN1")            /* Player input ports */
	AM_RANGE(0x800028, 0x80002f) AM_READ(cps1_hack_dsw_r)            /* System input ports / Dip Switches */
	AM_RANGE(0x800030, 0x800037) AM_WRITE(cps1_coinctrl_w)
	AM_RANGE(0x800100, 0x80013f) AM_WRITE(cps1_cps_a_w) AM_SHARE("cps_a_regs")  /* CPS-A custom */
	AM_RANGE(0x800140, 0x80017f) AM_READWRITE(cps1_cps_b_r, cps1_cps_b_w) AM_SHARE("cps_b_regs")   /* CPS-B custom */
	AM_RANGE(0x800186, 0x800187) AM_READ(cps1_in2_r)            /* Buttons 4,5,6 for both players */
	AM_RANGE(0x800190, 0x800197) AM_WRITE(cps1_soundlatch_w)    /* Sound command */
	AM_RANGE(0x800198, 0x80019f) AM_WRITE(cps1_soundlatch2_w)   /* Sound timer fade */
	AM_RANGE(0x8001a0, 0x8001c3) AM_WRITE(cps1_cps_a_w)
	AM_RANGE(0x8001c4, 0x8001c5) AM_WRITE(sf2m3_layer_w)
	AM_RANGE(0x900000, 0x92ffff) AM_RAM_WRITE(cps1_gfxram_w) AM_SHARE("gfxram")
	AM_RANGE(0xff0000, 0xffffff) AM_RAM
ADDRESS_MAP_END

static ADDRESS_MAP_START( sf2m10_map, AS_PROGRAM, 16, cps_state )
	AM_RANGE(0x000000, 0x3fffff) AM_ROM
	AM_RANGE(0x800000, 0x800007) AM_READ_PORT("IN1")
	AM_RANGE(0x800018, 0x80001f) AM_READ(cps1_hack_dsw_r)
	AM_RANGE(0x800020, 0x800021) AM_READNOP
	AM_RANGE(0x800030, 0x800037) AM_WRITE(cps1_coinctrl_w)
	AM_RANGE(0x800100, 0x80013f) AM_WRITE(cps1_cps_a_w) AM_SHARE("cps_a_regs")
	AM_RANGE(0x800140, 0x80017f) AM_READWRITE(cps1_cps_b_r, cps1_cps_b_w) AM_SHARE("cps_b_regs")
	AM_RANGE(0x800180, 0x800187) AM_WRITE(cps1_soundlatch_w)
	AM_RANGE(0x800188, 0x80018f) AM_WRITE(cps1_soundlatch2_w)
	AM_RANGE(0x8001a2, 0x8001b3) AM_WRITE(cps1_cps_a_w) // make 8001b2 point at 800110
	AM_RANGE(0x8001fe, 0x8001ff) AM_WRITENOP // writes FFFF here a lot
	AM_RANGE(0x900000, 0x92ffff) AM_RAM_WRITE(cps1_gfxram_w) AM_SHARE("gfxram")
	AM_RANGE(0xe00000, 0xefffff) AM_RAM // it writes to the whole range at start
	AM_RANGE(0xf1c000, 0xf1c001) AM_READ(cps1_in2_r)
	AM_RANGE(0xfeff00, 0xfeffff) AM_RAM // fix stack crash at start
	AM_RANGE(0xff0000, 0xffffff) AM_RAM AM_SHARE("mainram")
ADDRESS_MAP_END

/***********************************************************
             INPUT PORTS, DIPs
***********************************************************/

#define CPS1_COINAGE_1(diploc) \
	PORT_DIPNAME( 0x07, 0x07, DEF_STR( Coin_A ) ) PORT_DIPLOCATION(diploc ":1,2,3") \
	PORT_DIPSETTING(    0x00, DEF_STR( 4C_1C ) ) \
	PORT_DIPSETTING(    0x01, DEF_STR( 3C_1C ) ) \
	PORT_DIPSETTING(    0x02, DEF_STR( 2C_1C ) ) \
	PORT_DIPSETTING(    0x07, DEF_STR( 1C_1C ) ) \
	PORT_DIPSETTING(    0x06, DEF_STR( 1C_2C ) ) \
	PORT_DIPSETTING(    0x05, DEF_STR( 1C_3C ) ) \
	PORT_DIPSETTING(    0x04, DEF_STR( 1C_4C ) ) \
	PORT_DIPSETTING(    0x03, DEF_STR( 1C_6C ) ) \
	PORT_DIPNAME( 0x38, 0x38, DEF_STR( Coin_B ) ) PORT_DIPLOCATION(diploc ":4,5,6") \
	PORT_DIPSETTING(    0x00, DEF_STR( 4C_1C ) ) \
	PORT_DIPSETTING(    0x08, DEF_STR( 3C_1C ) ) \
	PORT_DIPSETTING(    0x10, DEF_STR( 2C_1C ) ) \
	PORT_DIPSETTING(    0x38, DEF_STR( 1C_1C ) ) \
	PORT_DIPSETTING(    0x30, DEF_STR( 1C_2C ) ) \
	PORT_DIPSETTING(    0x28, DEF_STR( 1C_3C ) ) \
	PORT_DIPSETTING(    0x20, DEF_STR( 1C_4C ) ) \
	PORT_DIPSETTING(    0x18, DEF_STR( 1C_6C ) )

#define CPS1_COINAGE_2(diploc) \
	PORT_DIPNAME( 0x07, 0x07, DEF_STR( Coinage ) ) PORT_DIPLOCATION(diploc ":1,2,3") \
	PORT_DIPSETTING(    0x00, DEF_STR( 4C_1C ) ) \
	PORT_DIPSETTING(    0x01, DEF_STR( 3C_1C ) ) \
	PORT_DIPSETTING(    0x02, DEF_STR( 2C_1C ) ) \
	PORT_DIPSETTING(    0x07, DEF_STR( 1C_1C ) ) \
	PORT_DIPSETTING(    0x06, DEF_STR( 1C_2C ) ) \
	PORT_DIPSETTING(    0x05, DEF_STR( 1C_3C ) ) \
	PORT_DIPSETTING(    0x04, DEF_STR( 1C_4C ) ) \
	PORT_DIPSETTING(    0x03, DEF_STR( 1C_6C ) )

#define CPS1_COINAGE_3(diploc) \
	PORT_DIPNAME( 0x07, 0x07, DEF_STR( Coin_A ) ) PORT_DIPLOCATION(diploc ":1,2,3") \
	PORT_DIPSETTING(    0x01, DEF_STR( 4C_1C ) ) \
	PORT_DIPSETTING(    0x02, DEF_STR( 3C_1C ) ) \
	PORT_DIPSETTING(    0x03, DEF_STR( 2C_1C ) ) \
	PORT_DIPSETTING(    0x00, "2 Coins/1 Credit (1 to continue)" ) \
	PORT_DIPSETTING(    0x07, DEF_STR( 1C_1C ) ) \
	PORT_DIPSETTING(    0x06, DEF_STR( 1C_2C ) ) \
	PORT_DIPSETTING(    0x05, DEF_STR( 1C_3C ) ) \
	PORT_DIPSETTING(    0x04, DEF_STR( 1C_4C ) ) \
	PORT_DIPNAME( 0x38, 0x38, DEF_STR( Coin_B ) ) PORT_DIPLOCATION(diploc ":4,5,6") \
	PORT_DIPSETTING(    0x08, DEF_STR( 4C_1C ) ) \
	PORT_DIPSETTING(    0x10, DEF_STR( 3C_1C ) ) \
	PORT_DIPSETTING(    0x18, DEF_STR( 2C_1C ) ) \
	PORT_DIPSETTING(    0x00, "2 Coins/1 Credit (1 to continue)" ) \
	PORT_DIPSETTING(    0x38, DEF_STR( 1C_1C ) ) \
	PORT_DIPSETTING(    0x30, DEF_STR( 1C_2C ) ) \
	PORT_DIPSETTING(    0x28, DEF_STR( 1C_3C ) ) \
	PORT_DIPSETTING(    0x20, DEF_STR( 1C_4C ) )

#define CPS1_DIFFICULTY_1(diploc) \
	PORT_DIPNAME( 0x07, 0x04, DEF_STR( Difficulty ) ) PORT_DIPLOCATION(diploc ":1,2,3") \
	PORT_DIPSETTING(    0x07, "0 (Easiest)" ) \
	PORT_DIPSETTING(    0x06, "1" ) \
	PORT_DIPSETTING(    0x05, "2" ) \
	PORT_DIPSETTING(    0x04, "3 (Normal)" ) \
	PORT_DIPSETTING(    0x03, "4" ) \
	PORT_DIPSETTING(    0x02, "5" ) \
	PORT_DIPSETTING(    0x01, "6" ) \
	PORT_DIPSETTING(    0x00, "7 (Hardest)" )

#define CPS1_DIFFICULTY_2(diploc) \
	PORT_DIPNAME( 0x07, 0x07, DEF_STR( Difficulty ) ) PORT_DIPLOCATION(diploc ":1,2,3") \
	PORT_DIPSETTING(    0x04, "1 (Easiest)" ) \
	PORT_DIPSETTING(    0x05, "2" ) \
	PORT_DIPSETTING(    0x06, "3" ) \
	PORT_DIPSETTING(    0x07, "4 (Normal)" ) \
	PORT_DIPSETTING(    0x03, "5" ) \
	PORT_DIPSETTING(    0x02, "6" ) \
	PORT_DIPSETTING(    0x01, "7" ) \
	PORT_DIPSETTING(    0x00, "8 (Hardest)" )

/* CPS1 games with 2 players and 3 buttons each */
static INPUT_PORTS_START( cps1_3b )
	PORT_START("IN0")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_SERVICE1 )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_START2 )
	PORT_SERVICE( 0x40, IP_ACTIVE_LOW )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("IN1")
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(1)
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(1)
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_PLAYER(1)
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0100, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x0200, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x0400, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x0800, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x1000, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(2)
	PORT_BIT( 0x2000, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(2)
	PORT_BIT( 0x4000, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_PLAYER(2)
	PORT_BIT( 0x8000, IP_ACTIVE_LOW, IPT_UNKNOWN )
INPUT_PORTS_END

/* CPS1 games with 2 players and 2 buttons each */
static INPUT_PORTS_START( cps1_2b )
	PORT_INCLUDE( cps1_3b )

	PORT_MODIFY("IN1")
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_UNKNOWN )  // no button 3
	PORT_BIT( 0x4000, IP_ACTIVE_LOW, IPT_UNKNOWN )  // no button 3
INPUT_PORTS_END

/* CPS1 games with 2 players and 1 button each */
static INPUT_PORTS_START( cps1_1b )
	PORT_INCLUDE( cps1_2b )

	PORT_MODIFY("IN1")
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_UNKNOWN )  // no button 2
	PORT_BIT( 0x2000, IP_ACTIVE_LOW, IPT_UNKNOWN )  // no button 2
INPUT_PORTS_END

/* CPS1 games with 3 players and 2 buttons each */
static INPUT_PORTS_START( cps1_3players )
	PORT_INCLUDE( cps1_2b )

	PORT_START("IN2")      /* Player 3 */
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_PLAYER(3)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY PORT_PLAYER(3)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY PORT_PLAYER(3)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY PORT_PLAYER(3)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(3)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(3)
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_COIN3 )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_START3 )
INPUT_PORTS_END

/* CPS1 games with 4 players and 2 buttons each */
static INPUT_PORTS_START( cps1_4players )
	PORT_INCLUDE( cps1_3players )

	PORT_START("IN3")      /* Player 4 */
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_PLAYER(4)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY PORT_PLAYER(4)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY PORT_PLAYER(4)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY PORT_PLAYER(4)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(4)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(4)
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_COIN4 )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_START4 )
INPUT_PORTS_END

/* CPS1 games with 2 players and 6 buttons each */
static INPUT_PORTS_START( cps1_6b)
	PORT_INCLUDE( cps1_3b)

	PORT_MODIFY("IN1")
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_NAME("P1 Jab Punch") PORT_PLAYER(1)
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_NAME("P1 Strong Punch") PORT_PLAYER(1)
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_NAME("P1 Fierce Punch") PORT_PLAYER(1)
	PORT_BIT( 0x1000, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_NAME("P2 Jab Punch") PORT_PLAYER(2)
	PORT_BIT( 0x2000, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_NAME("P2 Strong Punch") PORT_PLAYER(2)
	PORT_BIT( 0x4000, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_NAME("P2 Fierce Punch") PORT_PLAYER(2)

	PORT_START("IN2")      /* Extra buttons */
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_BUTTON4 ) PORT_NAME("P1 Short Kick") PORT_PLAYER(1)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_BUTTON5 ) PORT_NAME("P1 Forward Kick") PORT_PLAYER(1)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_BUTTON6 ) PORT_NAME("P1 Roundhouse Kick") PORT_PLAYER(1)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON4 ) PORT_NAME("P2 Short Kick") PORT_PLAYER(2)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON5 ) PORT_NAME("P2 Forward Kick") PORT_PLAYER(2)
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_BUTTON6 ) PORT_NAME("P2 Roundhouse Kick") PORT_PLAYER(2)
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )
INPUT_PORTS_END

/* CPS1 quiz games */
static INPUT_PORTS_START( cps1_quiz )
	PORT_INCLUDE( cps1_3b )

	PORT_MODIFY("IN1")  /* no joystick and 4th button */
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_BUTTON4 ) PORT_PLAYER(1)
	PORT_BIT( 0x0100, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0200, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0400, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0800, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x8000, IP_ACTIVE_LOW, IPT_BUTTON4 ) PORT_PLAYER(2)
INPUT_PORTS_END


static INPUT_PORTS_START( forgottn )
	PORT_INCLUDE( cps1_1b )

	PORT_MODIFY("IN0")
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("DSWC")                                  // The manual only mentions two DIP switch banks.
	PORT_DIPUNUSED( 0x80, 0x80 )                        // Is this port brought out to DIP switches or not?
	PORT_DIPUNKNOWN( 0x40, 0x40 )                       // Check code at 0x013c78 (0x013690 in 'lostwrld')
	PORT_DIPUNUSED( 0x20, 0x20 )
	PORT_DIPUNUSED( 0x10, 0x10 )
	PORT_DIPUNUSED( 0x08, 0x08 )
	PORT_DIPUNUSED( 0x04, 0x04 )
	PORT_DIPUNUSED( 0x02, 0x02 )
	PORT_DIPUNUSED( 0x01, 0x01 )

	PORT_START("DSWB")
	CPS1_DIFFICULTY_1( "DIP-B" )
	PORT_DIPUNUSED_DIPLOC( 0x20, 0x20, "DIP-B:4" )
	PORT_DIPUNUSED_DIPLOC( 0x10, 0x10, "DIP-B:5" )
	PORT_DIPUNKNOWN_DIPLOC( 0x08, 0x08, "DIP-B:6" )         // Check code at 0x00111c (0x00112c in 'lostwrld')
	PORT_SERVICE_DIPLOC( 0x40, IP_ACTIVE_LOW, "DIP-B:7" )
	PORT_DIPNAME( 0x80, 0x80, "Freeze" )                    PORT_DIPLOCATION("DIP-B:8")
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

	PORT_START("DSWA")
	CPS1_COINAGE_1( "DIP-A" )
	PORT_DIPNAME( 0x40, 0x40, DEF_STR( Demo_Sounds ) )      PORT_DIPLOCATION("DIP-A:7")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x40, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, DEF_STR( Flip_Screen ) )      PORT_DIPLOCATION("DIP-A:8")
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

	PORT_START("DIAL0")
	PORT_BIT( 0x0fff, 0x0000, IPT_DIAL ) PORT_SENSITIVITY(100) PORT_KEYDELTA(20) PORT_CODE_DEC(KEYCODE_Z) PORT_CODE_INC(KEYCODE_X) PORT_PLAYER(1)

	PORT_START("DIAL1")
	PORT_BIT( 0x0fff, 0x0000, IPT_DIAL ) PORT_SENSITIVITY(100) PORT_KEYDELTA(20) PORT_CODE_DEC(KEYCODE_N) PORT_CODE_INC(KEYCODE_M) PORT_PLAYER(2)
INPUT_PORTS_END

static INPUT_PORTS_START( ghouls )
	PORT_INCLUDE( cps1_2b )
	/* Service1 doesn't give any credit */

	PORT_START("DSWC")
	PORT_DIPNAME( 0x03, 0x03, DEF_STR( Lives ) )            PORT_DIPLOCATION("SW(C):1,2")
	PORT_DIPSETTING(    0x03, "3" )
	PORT_DIPSETTING(    0x02, "4" )
	PORT_DIPSETTING(    0x01, "5" )
	PORT_DIPSETTING(    0x00, "6" )
	PORT_DIPUNUSED_DIPLOC( 0x04, 0x04, "SW(C):3" )
	PORT_DIPUNUSED_DIPLOC( 0x08, 0x08, "SW(C):4" )
	PORT_DIPNAME( 0x10, 0x10, DEF_STR( Flip_Screen ) )      PORT_DIPLOCATION("SW(C):5")
	PORT_DIPSETTING(    0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x20, DEF_STR( Unused ) )           PORT_DIPLOCATION("SW(C):6")
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )              // "Demo Sounds" in manual; doesn't work
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x40, DEF_STR( Allow_Continue ) )   PORT_DIPLOCATION("SW(C):7")
	PORT_DIPSETTING(    0x00, DEF_STR( No ) )
	PORT_DIPSETTING(    0x40, DEF_STR( Yes ) )
	PORT_DIPNAME( 0x80, 0x80, "Game Mode")                  PORT_DIPLOCATION("SW(C):8")
	PORT_DIPSETTING(    0x80, "Game" )
	PORT_DIPSETTING(    0x00, DEF_STR( Test ) )

	PORT_START("DSWB") /* Manual states default difficulty "B" (2) which differs from the normal macro */
	PORT_DIPNAME( 0x07, 0x05, DEF_STR( Difficulty ) ) PORT_CONDITION("DSWC", 0x80, EQUALS, 0x80) PORT_DIPLOCATION("SW(B):1,2,3")
	PORT_DIPSETTING(    0x04, "1 (Easiest)" )
	PORT_DIPSETTING(    0x05, "2" )
	PORT_DIPSETTING(    0x06, "3" )
	PORT_DIPSETTING(    0x07, "4 (Normal)" )
	PORT_DIPSETTING(    0x03, "5" )
	PORT_DIPSETTING(    0x02, "6" )
	PORT_DIPSETTING(    0x01, "7" )
	PORT_DIPSETTING(    0x00, "8 (Hardest)" )
	PORT_DIPUNUSED_DIPLOC( 0x08, 0x08, "SW(B):4" )
	PORT_DIPNAME( 0x30, 0x30, DEF_STR( Bonus_Life ) )       PORT_DIPLOCATION("SW(B):5,6")
	PORT_DIPSETTING(    0x20, "10K, 30K and every 30K" )
	PORT_DIPSETTING(    0x10, "20K, 50K and every 70K" )
	PORT_DIPSETTING(    0x30, "30K, 60K and every 70K" )
	PORT_DIPSETTING(    0x00, "40K, 70K and every 80K" )
	PORT_DIPUNUSED_DIPLOC( 0x40, 0x40, "SW(B):7" )
	PORT_DIPUNUSED_DIPLOC( 0x80, 0x80, "SW(B):8" )

	PORT_START("DSWA")
	CPS1_COINAGE_1( "SW(A)" )
	PORT_DIPNAME( 0xc0, 0xc0, DEF_STR( Cabinet ) )          PORT_DIPLOCATION("SW(A):7,8")
	PORT_DIPSETTING(    0xc0, "Upright 1 Player" )
	PORT_DIPSETTING(    0x80, "Upright 2 Players" )
//  PORT_DIPSETTING(    0x40, DEF_STR( Cocktail ) )         // Manual says these are both valid settings
	PORT_DIPSETTING(    0x00, DEF_STR( Cocktail ) )         // for 2-player cocktail cabinet
INPUT_PORTS_END

/* Same as 'ghouls' but additional "Freeze" Dip Switch, different "Lives" Dip Switch,
   and LOTS of "debug" features (read the notes to know how to activate them) */
static INPUT_PORTS_START( ghoulsu )
	PORT_INCLUDE( ghouls )

	PORT_MODIFY("DSWC")
	PORT_DIPNAME( 0x03, 0x03, DEF_STR( Lives ) ) PORT_DIPLOCATION("SW(C):1,2")
	PORT_DIPSETTING(    0x00, "2" )
	PORT_DIPSETTING(    0x03, "3" )
	PORT_DIPSETTING(    0x02, "4" )
	PORT_DIPSETTING(    0x01, "5" )

	PORT_MODIFY("DSWB")
	/* Standard Dip Switches */
	PORT_DIPNAME( 0x08, 0x08, DEF_STR( Unused ) ) PORT_CONDITION("DSWC", 0x80, EQUALS, 0x80) PORT_DIPLOCATION("SW(B):4")
	PORT_DIPSETTING(    0x08, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x30, 0x00, DEF_STR( Bonus_Life ) )  PORT_CONDITION("DSWC", 0x80, EQUALS, 0x80) PORT_DIPLOCATION("SW(B):5,6")
	PORT_DIPSETTING(    0x20, "10K, 30K and every 30K" )
	PORT_DIPSETTING(    0x10, "20K, 50K and every 70K" )
	PORT_DIPSETTING(    0x30, "30K, 60K and every 70K" )
	PORT_DIPSETTING(    0x00, "40K, 70K and every 80K" )
/* Manuals states the following bonus settings
    PORT_DIPSETTING(    0x20, "20K, 50K and every 70K" )
    PORT_DIPSETTING(    0x10, "10K, 30K and every 30K" )
    PORT_DIPSETTING(    0x30, "40K, 70K and every 80K" )
    PORT_DIPSETTING(    0x00, "30K, 60K and every 70K" )
*/
	/* Debug Dip Switches */
	PORT_DIPNAME( 0x07, 0x07, "Starting Weapon" ) PORT_CONDITION("DSWC", 0x80, EQUALS, 0x00) PORT_DIPLOCATION("SW(B):1,2,3")
	PORT_DIPSETTING(    0x07, "Spear" )
	PORT_DIPSETTING(    0x06, "Knife" )
	PORT_DIPSETTING(    0x05, "Torch" )
	PORT_DIPSETTING(    0x04, "Sword" )
	PORT_DIPSETTING(    0x03, "Axe" )
	PORT_DIPSETTING(    0x02, "Shield" )
	PORT_DIPSETTING(    0x01, "Super Weapon" )
//  PORT_DIPSETTING(    0x00, "INVALID !" )
	PORT_DIPNAME( 0x38, 0x30, "Armor on New Life" ) PORT_CONDITION("DSWC", 0x80, EQUALS, 0x00) PORT_DIPLOCATION("SW(B):4,5,6")
//  PORT_DIPSETTING(    0x38, "Silver Armor" )
	PORT_DIPSETTING(    0x18, "Golden Armor" )
	PORT_DIPSETTING(    0x30, "Silver Armor" )
	PORT_DIPSETTING(    0x28, "None (young man)" )
	PORT_DIPSETTING(    0x20, "None (old man)" )
//  PORT_DIPSETTING(    0x10, "INVALID !" )
//  PORT_DIPSETTING(    0x08, "INVALID !" )
//  PORT_DIPSETTING(    0x00, "INVALID !" )

	PORT_DIPUNUSED_DIPLOC( 0x40, 0x40, "SW(B):7" )
	PORT_DIPNAME( 0x80, 0x80, "Freeze" ) PORT_DIPLOCATION("SW(B):8")
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

	PORT_MODIFY("DSWA")
	/* Standard Dip Switches */
	PORT_DIPNAME( 0x07, 0x07, DEF_STR( Coin_A ) ) PORT_CONDITION("DSWC", 0x80, EQUALS, 0x80) PORT_DIPLOCATION("SW(A):1,2,3")
	PORT_DIPSETTING(    0x00, DEF_STR( 4C_1C ) )
	PORT_DIPSETTING(    0x01, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(    0x02, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x07, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x06, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0x05, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(    0x04, DEF_STR( 1C_4C ) )
	PORT_DIPSETTING(    0x03, DEF_STR( 1C_6C ) )
	PORT_DIPNAME( 0x38, 0x38, DEF_STR( Coin_B ) ) PORT_CONDITION("DSWC", 0x80, EQUALS, 0x80) PORT_DIPLOCATION("SW(A):4,5,6")
	PORT_DIPSETTING(    0x00, DEF_STR( 4C_1C ) )
	PORT_DIPSETTING(    0x08, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(    0x10, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x38, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x30, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0x28, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(    0x20, DEF_STR( 1C_4C ) )
	PORT_DIPSETTING(    0x18, DEF_STR( 1C_6C ) )
	/* Debug Dip Switches */
	PORT_DIPNAME( 0x0f, 0x0f, "Starting Level" ) PORT_CONDITION("DSWC", 0x80, EQUALS, 0x00) PORT_DIPLOCATION("SW(A):1,2,3,4")
	PORT_DIPSETTING(    0x0f, "Level 1 (1st half)" )
	PORT_DIPSETTING(    0x0e, "Level 1 (2nd half)" )
	PORT_DIPSETTING(    0x0d, "Level 2 (1st half)" )
	PORT_DIPSETTING(    0x0c, "Level 2 (2nd half)" )
	PORT_DIPSETTING(    0x0b, "Level 3 (1st half)" )
	PORT_DIPSETTING(    0x0a, "Level 3 (2nd half)" )
	PORT_DIPSETTING(    0x09, "Level 4 (1st half)" )
	PORT_DIPSETTING(    0x08, "Level 4 (2nd half)" )
	PORT_DIPSETTING(    0x07, "Level 5 (1st half)" )
	PORT_DIPSETTING(    0x06, "Level 5 (2nd half)" )
	PORT_DIPSETTING(    0x05, "Level 6" )
//  PORT_DIPSETTING(    0x04, "INVALID !" )
//  PORT_DIPSETTING(    0x03, "INVALID !" )
//  PORT_DIPSETTING(    0x02, "INVALID !" )
//  PORT_DIPSETTING(    0x01, "INVALID !" )
//  PORT_DIPSETTING(    0x00, "INVALID !" )
	PORT_DIPNAME( 0x10, 0x10, "Invulnerability" ) PORT_CONDITION("DSWC", 0x80, EQUALS, 0x00) PORT_DIPLOCATION("SW(A):5")
	PORT_DIPSETTING(    0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x20, "Slow Motion" ) PORT_CONDITION("DSWC", 0x80, EQUALS, 0x00) PORT_DIPLOCATION("SW(A):6")
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

	PORT_DIPNAME( 0xc0, 0xc0, DEF_STR( Cabinet ) ) PORT_DIPLOCATION("SW(A):7,8")
	PORT_DIPSETTING(    0xc0, "Upright 1 Player" )
	PORT_DIPSETTING(    0x80, "Upright 2 Players" )
//  PORT_DIPSETTING(    0x40, DEF_STR( Cocktail ) )             // Manual says these are both valid settings
	PORT_DIPSETTING(    0x00, DEF_STR( Cocktail ) )             // for 2-player cocktail cabinet
INPUT_PORTS_END

/* Same as 'ghouls' but additional "Freeze" Dip Switch */
static INPUT_PORTS_START( daimakai )
	PORT_INCLUDE(ghouls)

	PORT_MODIFY("DSWB")
	PORT_DIPNAME( 0x80, 0x80, "Freeze" )        PORT_DIPLOCATION("SW(B):8")
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )  // This switch isn't documented in the manual
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
INPUT_PORTS_END

/* "Debug" features to be implemented */
static INPUT_PORTS_START( strider )
	PORT_INCLUDE( cps1_3b )

	PORT_START("DSWA")
	CPS1_COINAGE_1( "SW(A)" )
	PORT_DIPNAME( 0xc0, 0xc0, DEF_STR( Cabinet ) )              PORT_DIPLOCATION("SW(A):7,8")
	PORT_DIPSETTING(    0xc0, "Upright 1 Player" )              // These switches are not documented in the manual
	PORT_DIPSETTING(    0x80, "Upright 2 Players" )
//  PORT_DIPSETTING(    0x40, DEF_STR( Cocktail ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Cocktail ) )

	PORT_START("DSWB") /* Like Ghouls, Strider manual states "B" (or 2) as the recommended difficulty level. */
	PORT_DIPNAME( 0x07, 0x05, DEF_STR( Difficulty ) ) PORT_DIPLOCATION("SW(B):1,2,3")
	PORT_DIPSETTING(    0x04, "1 (Easiest)" )
	PORT_DIPSETTING(    0x05, "2" )
	PORT_DIPSETTING(    0x06, "3" )
	PORT_DIPSETTING(    0x07, "4 (Normal)" )
	PORT_DIPSETTING(    0x03, "5" )
	PORT_DIPSETTING(    0x02, "6" )
	PORT_DIPSETTING(    0x01, "7" )
	PORT_DIPSETTING(    0x00, "8 (Hardest)" )
	/* In 'striderj', bit 3 is stored at 0xff8e77 ($e77,A5) via code at 0x000a2a,
	   but this address is never checked again.
	   In 'strider' and 'stridrjr', this code even doesn't exist ! */
	PORT_DIPNAME( 0x08, 0x08, DEF_STR( Unused ) )               PORT_DIPLOCATION("SW(B):4")
	PORT_DIPSETTING(    0x08, DEF_STR( Off ) )                  // Manual says this is 2c start/1c continue but it
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )                   // doesn't work (see comment above)
	PORT_DIPNAME( 0x30, 0x00, DEF_STR( Bonus_Life ) )           PORT_DIPLOCATION("SW(B):5,6")
/* These show in test mode */
	PORT_DIPSETTING(    0x30, "20K, 40K then every 60K" )
	PORT_DIPSETTING(    0x20, "30K, 50K then every 70K" )
	PORT_DIPSETTING(    0x10, "20K & 60K only" )
	PORT_DIPSETTING(    0x00, "30K & 60K only" )
/* According to manual, these are the proper settings
    PORT_DIPSETTING(    0x30, "40K, 70K then every 80K" )
    PORT_DIPSETTING(    0x20, "20K, 50K then every 70K" )
    PORT_DIPSETTING(    0x10, "10k, 30k then every 30k" )
    PORT_DIPSETTING(    0x00, "30K, 60k then every 70k" )
*/

	PORT_DIPNAME( 0xc0, 0x80, "Internal Diff. on Life Loss" )   PORT_DIPLOCATION("SW(B):7,8")
	PORT_DIPSETTING(    0xc0, "-3" )                            // Check code at 0x00d15a
//  PORT_DIPSETTING(    0x40, "-1" )                            // These switches are not documented in the manual
	PORT_DIPSETTING(    0x00, "-1" )
	PORT_DIPSETTING(    0x80, "Default" )

	PORT_START("DSWC")
	PORT_DIPNAME( 0x03, 0x03, DEF_STR( Lives ) )                PORT_DIPLOCATION("SW(C):1,2")
	PORT_DIPSETTING(    0x00, "2" )                             // "6" in the "test mode" and manual
	PORT_DIPSETTING(    0x03, "3" )
	PORT_DIPSETTING(    0x02, "4" )
	PORT_DIPSETTING(    0x01, "5" )
	PORT_DIPNAME( 0x04, 0x04, "Freeze" )                        PORT_DIPLOCATION("SW(C):3")
	PORT_DIPSETTING(    0x04, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x08, DEF_STR( Free_Play ) )            PORT_DIPLOCATION("SW(C):4")
	PORT_DIPSETTING(    0x08, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x10, DEF_STR( Flip_Screen ) )          PORT_DIPLOCATION("SW(C):5")
	PORT_DIPSETTING(    0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x20, DEF_STR( Demo_Sounds ) )          PORT_DIPLOCATION("SW(C):6")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x20, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x40, DEF_STR( Allow_Continue ) )       PORT_DIPLOCATION("SW(C):7")
	PORT_DIPSETTING(    0x00, DEF_STR( No ) )
	PORT_DIPSETTING(    0x40, DEF_STR( Yes ) )
	PORT_DIPNAME( 0x80, 0x80, "Game Mode")                      PORT_DIPLOCATION("SW(C):8")
	PORT_DIPSETTING(    0x80, "Game" )
	PORT_DIPSETTING(    0x00, DEF_STR( Test ) )                 // To enable the "debug" features
INPUT_PORTS_END

/* Same as 'strider' but additional "2 Coins to Start, 1 to Continue" Dip Switch */
/* "Debug" features to be implemented */
static INPUT_PORTS_START( stridrua )
	PORT_INCLUDE( strider )

	PORT_MODIFY("DSWB")
	/* In 'striderj', bit 3 is stored at 0xff8e77 ($e77,A5) via code at 0x000a2a,
	   but this address is never checked again.
	   In 'strider' and 'stridrjr', this code even doesn't exist ! */
	PORT_DIPNAME( 0x08, 0x08, "2 Coins to Start, 1 to Continue" )   PORT_DIPLOCATION("SW(B):4")
	PORT_DIPSETTING(    0x08, DEF_STR( Off ) )                      // This works in this revision
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
INPUT_PORTS_END

static INPUT_PORTS_START( dynwar )
	PORT_INCLUDE( cps1_3b )

	PORT_START("DSWC")
	PORT_DIPNAME( 0x01, 0x01, "Freeze" )                        PORT_DIPLOCATION("SW(C):1")
	PORT_DIPSETTING(    0x01, DEF_STR( Off ) )                  // Also affects energy cost - read notes
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )                   // This switch is not documented in the manual
	PORT_DIPNAME( 0x02, 0x02, "Turbo Mode" )                    PORT_DIPLOCATION("SW(C):2")
	PORT_DIPSETTING(    0x02, DEF_STR( Off ) )                  // Also affects energy cost - read notes
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )                   // This switch is not documented in the manual
	PORT_DIPUNUSED_DIPLOC( 0x04, 0x04, "SW(C):3" )              // This switch is not documented in the manual
	PORT_DIPUNUSED_DIPLOC( 0x08, 0x08, "SW(C):4" )              // This switch is not documented in the manual
	PORT_DIPNAME( 0x10, 0x10, DEF_STR( Flip_Screen ) )          PORT_DIPLOCATION("SW(C):5")
	PORT_DIPSETTING(    0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x20, DEF_STR( Demo_Sounds ) )          PORT_DIPLOCATION("SW(C):6")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )                  // "ON"  in the "test mode"
	PORT_DIPSETTING(    0x20, DEF_STR( On ) )                   // "OFF" in the "test mode"
	PORT_DIPNAME( 0x40, 0x40, DEF_STR( Allow_Continue ) )       PORT_DIPLOCATION("SW(C):7")
	PORT_DIPSETTING(    0x00, DEF_STR( No ) )                   // "ON"  in the "test mode"
	PORT_DIPSETTING(    0x40, DEF_STR( Yes ) )                  // "OFF" in the "test mode"
	PORT_DIPNAME( 0x80, 0x80, "Game Mode")                      PORT_DIPLOCATION("SW(C):8")
	PORT_DIPSETTING(    0x80, "Game" )
	PORT_DIPSETTING(    0x00, DEF_STR( Test ) )

	PORT_START("DSWB")
	CPS1_DIFFICULTY_2( "SW(B)" )
	PORT_DIPUNUSED_DIPLOC( 0x08, 0x08, "SW(B):4" )              // These five switches are not documented in the
	PORT_DIPUNUSED_DIPLOC( 0x10, 0x10, "SW(B):5" )              // manual
	PORT_DIPUNUSED_DIPLOC( 0x20, 0x20, "SW(B):6" )
	PORT_DIPUNUSED_DIPLOC( 0x40, 0x40, "SW(B):7" )
	PORT_DIPUNUSED_DIPLOC( 0x80, 0x80, "SW(B):8" )

	PORT_START("DSWA")
	/* According to the manual, ALL switches 1 to 6 must be ON to have
	   "2 Coins/1 Credit (1 to continue)" for both coin slots */
	CPS1_COINAGE_3( "SW(A)" )
	PORT_DIPUNUSED_DIPLOC( 0x40, 0x40, "SW(A):7" )              // This switch is not documented in the manual
	PORT_DIPNAME( 0x80, 0x80, DEF_STR( Free_Play ) )            PORT_DIPLOCATION("SW(A):8")
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )                  // This switch is not documented in the manual
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
INPUT_PORTS_END

/* Read the notes to know how to activate the "debug" features */
static INPUT_PORTS_START( willow )
	PORT_INCLUDE( cps1_3b )

	PORT_MODIFY("IN0")
	PORT_SERVICE_NO_TOGGLE( 0x40, IP_ACTIVE_LOW )

	PORT_START("DSWC")
	/* Standard Dip Switches */
	PORT_DIPNAME( 0x03, 0x02, DEF_STR( Lives ) ) PORT_CONDITION("DSWC", 0x80, EQUALS, 0x80) PORT_DIPLOCATION("SW(C):1,2")
	PORT_DIPSETTING(    0x02, "1" )
	PORT_DIPSETTING(    0x03, "2" )
	PORT_DIPSETTING(    0x01, "3" )
	PORT_DIPSETTING(    0x00, "4" )
	PORT_DIPNAME( 0x0c, 0x08, "Vitality" ) PORT_CONDITION("DSWC", 0x80, EQUALS, 0x80) PORT_DIPLOCATION("SW(C):3,4")
	PORT_DIPSETTING(    0x00, "2" )
	PORT_DIPSETTING(    0x0c, "3" )
	PORT_DIPSETTING(    0x08, "4" )
	PORT_DIPSETTING(    0x04, "5" )
	/* Debug Dip Switches */
	PORT_DIPNAME( 0x01, 0x01, "Turbo Mode" ) PORT_CONDITION("DSWC", 0x80, EQUALS, 0x00) PORT_DIPLOCATION("SW(C):1")
	PORT_DIPSETTING(    0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x02, "Freeze" ) PORT_CONDITION("DSWC", 0x80, EQUALS, 0x00) PORT_DIPLOCATION("SW(C):2")
	PORT_DIPSETTING(    0x02, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x04, 0x04, "Slow Motion" ) PORT_CONDITION("DSWC", 0x80, EQUALS, 0x00) PORT_DIPLOCATION("SW(C):3")
	PORT_DIPSETTING(    0x04, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x08, "Invulnerability" ) PORT_CONDITION("DSWC", 0x80, EQUALS, 0x00) PORT_DIPLOCATION("SW(C):4")
	PORT_DIPSETTING(    0x08, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

	PORT_DIPNAME( 0x10, 0x10, DEF_STR( Flip_Screen ) )          PORT_DIPLOCATION("SW(C):5")
	PORT_DIPSETTING(    0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

	/* Standard Dip Switches */
	PORT_DIPNAME( 0x20, 0x20, DEF_STR( Demo_Sounds ) ) PORT_CONDITION("DSWC", 0x80, EQUALS, 0x80) PORT_DIPLOCATION("SW(C):6")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x20, DEF_STR( On ) )
	/* Debug Dip Switches */
	PORT_DIPNAME( 0x20, 0x20, "Display Debug Infos" ) PORT_CONDITION("DSWC", 0x80, EQUALS, 0x00) PORT_DIPLOCATION("SW(C):6")
	PORT_DIPSETTING(    0x20, DEF_STR( No ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Yes ) )

	PORT_DIPNAME( 0x40, 0x40, DEF_STR( Allow_Continue ) )       PORT_DIPLOCATION("SW(C):7")
	PORT_DIPSETTING(    0x00, DEF_STR( No ) )
	PORT_DIPSETTING(    0x40, DEF_STR( Yes ) )
	PORT_DIPNAME( 0x80, 0x80, "Game Mode")                      PORT_DIPLOCATION("SW(C):8")
	PORT_DIPSETTING(    0x80, "Game" )
	PORT_DIPSETTING(    0x00, DEF_STR( Test ) )                 // To enable the "debug" features

	PORT_START("DSWB")
	/* Standard Dip Switches */
	PORT_DIPNAME( 0x07, 0x07, DEF_STR( Difficulty ) ) PORT_CONDITION("DSWC", 0x80, EQUALS, 0x80) PORT_DIPLOCATION("SW(B):1,2,3")
	PORT_DIPSETTING(    0x04, "1 (Easiest)" )
	PORT_DIPSETTING(    0x05, "2" )
	PORT_DIPSETTING(    0x06, "3" )
	PORT_DIPSETTING(    0x07, "4 (Normal)" )
	PORT_DIPSETTING(    0x03, "5" )
	PORT_DIPSETTING(    0x02, "6" )
	PORT_DIPSETTING(    0x01, "7" )
	PORT_DIPSETTING(    0x00, "8 (Hardest)" )
	PORT_DIPNAME( 0x18, 0x18, "Nando Speed" ) PORT_CONDITION("DSWC", 0x80, EQUALS, 0x80) PORT_DIPLOCATION("SW(B):4,5")
	PORT_DIPSETTING(    0x10, "Slow" )
	PORT_DIPSETTING(    0x18, DEF_STR( Normal ) )
	PORT_DIPSETTING(    0x08, "Fast" )
	PORT_DIPSETTING(    0x00, "Very Fast" )
	PORT_DIPNAME( 0x20, 0x20, DEF_STR( Unused ) ) PORT_CONDITION("DSWC", 0x80, EQUALS, 0x80) PORT_DIPLOCATION("SW(B):6")
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x40, DEF_STR( Unused ) ) PORT_CONDITION("DSWC", 0x80, EQUALS, 0x80) PORT_DIPLOCATION("SW(B):7")
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, "Stage Magic Continue" ) PORT_CONDITION("DSWC", 0x80, EQUALS, 0x80) PORT_DIPLOCATION("SW(B):8")
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	/* Debug Dip Switches */
	PORT_DIPNAME( 0x01, 0x01, DEF_STR( Unused ) ) PORT_CONDITION("DSWC", 0x80, EQUALS, 0x00) PORT_DIPLOCATION("SW(B):1")
	PORT_DIPSETTING(    0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x1e, 0x1e, "Slow Motion Delay" ) PORT_CONDITION("DSWC", 0x80, EQUALS, 0x00) PORT_DIPLOCATION("SW(B):2,3,4,5")
	PORT_DIPSETTING(    0x1e, "2 Frames" )
	PORT_DIPSETTING(    0x1c, "3 Frames" )
	PORT_DIPSETTING(    0x1a, "4 Frames" )
	PORT_DIPSETTING(    0x18, "5 Frames" )
	PORT_DIPSETTING(    0x16, "6 Frames" )
	PORT_DIPSETTING(    0x14, "7 Frames" )
	PORT_DIPSETTING(    0x12, "8 Frames" )
	PORT_DIPSETTING(    0x10, "9 Frames" )
	PORT_DIPSETTING(    0x0e, "10 Frames" )
	PORT_DIPSETTING(    0x0c, "11 Frames" )
	PORT_DIPSETTING(    0x0a, "12 Frames" )
	PORT_DIPSETTING(    0x08, "13 Frames" )
	PORT_DIPSETTING(    0x06, "14 Frames" )
	PORT_DIPSETTING(    0x04, "15 Frames" )
	PORT_DIPSETTING(    0x02, "16 Frames" )
	PORT_DIPSETTING(    0x00, "17 Frames" )
	PORT_DIPNAME( 0xe0, 0xe0, "Starting Level" ) PORT_CONDITION("DSWC", 0x80, EQUALS, 0x00) PORT_DIPLOCATION("SW(B):6,7,8")
	PORT_DIPSETTING(    0xe0, "Level 1" )
	PORT_DIPSETTING(    0xc0, "Level 2" )
	PORT_DIPSETTING(    0xa0, "Level 3" )
	PORT_DIPSETTING(    0x80, "Level 4" )
	PORT_DIPSETTING(    0x60, "Level 5" )
	PORT_DIPSETTING(    0x40, "Level 6" )
//  PORT_DIPSETTING(    0x20, "INVALID !" )
//  PORT_DIPSETTING(    0x00, "INVALID !" )

	PORT_START("DSWA")
	/* Standard Dip Switches */
	PORT_DIPNAME( 0x07, 0x07, DEF_STR( Coin_A ) ) PORT_CONDITION("DSWC", 0x80, EQUALS, 0x80) PORT_DIPLOCATION("SW(A):1,2,3")
	PORT_DIPSETTING(    0x01, DEF_STR( 4C_1C ) )
	PORT_DIPSETTING(    0x02, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(    0x03, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x00, "2 Coins/1 Credit (1 to continue)" )
	PORT_DIPSETTING(    0x07, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x06, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0x05, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(    0x04, DEF_STR( 1C_4C ) )
	PORT_DIPNAME( 0x38, 0x38, DEF_STR( Coin_B ) ) PORT_CONDITION("DSWC", 0x80, EQUALS, 0x80) PORT_DIPLOCATION("SW(A):4,5,6")
	PORT_DIPSETTING(    0x08, DEF_STR( 4C_1C ) )
	PORT_DIPSETTING(    0x10, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(    0x18, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x00, "2 Coins/1 Credit (1 to continue)" )
	PORT_DIPSETTING(    0x38, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x30, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0x28, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(    0x20, DEF_STR( 1C_4C ) )
	PORT_DIPNAME( 0xc0, 0xc0, DEF_STR( Cabinet ) ) PORT_CONDITION("DSWC", 0x80, EQUALS, 0x80) PORT_DIPLOCATION("SW(A):7,8")
	PORT_DIPSETTING(    0xc0, "Upright 1 Player" )
	PORT_DIPSETTING(    0x80, "Upright 2 Players" )
//  PORT_DIPSETTING(    0x40, DEF_STR( Cocktail ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Cocktail ) )
	/* Debug Dip Switches */
	PORT_DIPNAME( 0x3f, 0x3f, DEF_STR( Free_Play ) ) PORT_CONDITION("DSWC", 0x80, EQUALS, 0x00) PORT_DIPLOCATION("SW(A):1,2,3,4,5,6")
	PORT_DIPSETTING(    0x3f, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x38, DEF_STR( On ) )
	/* Other values don't give free play */
	PORT_DIPNAME( 0x40, 0x40, DEF_STR( Cabinet ) ) PORT_CONDITION("DSWC", 0x80, EQUALS, 0x00) PORT_DIPLOCATION("SW(A):7")
	PORT_DIPSETTING(    0x40, "Upright 1 Player" )
	PORT_DIPSETTING(    0x00, "Upright 2 Players" )
	PORT_DIPNAME( 0x80, 0x80, "Maximum magic/sword power" ) PORT_CONDITION("DSWC", 0x80, EQUALS, 0x00) PORT_DIPLOCATION("SW(A):8")
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
INPUT_PORTS_END

/* To enable extra choices in the "test mode", you must press "Coin 1" ('5') AND "Service Mode" ('F2') */
static INPUT_PORTS_START( unsquad )
	PORT_INCLUDE( cps1_3b )

	PORT_MODIFY("IN0")
	PORT_SERVICE_NO_TOGGLE( 0x40, IP_ACTIVE_LOW )

	PORT_START("DSWA")
	CPS1_COINAGE_3( "SW(A)" )
	/* According to the manual, ALL bits 0 to 5 must be ON to have
	   "2 Coins/1 Credit (1 to continue)" for both coin slots */
	PORT_DIPUNUSED_DIPLOC( 0x40, 0x40, "SW(A):7" )
	PORT_DIPUNUSED_DIPLOC( 0x80, 0x80, "SW(A):8" )

	PORT_START("DSWB")
	CPS1_DIFFICULTY_1( "SW(B)" )
	PORT_DIPNAME( 0x18, 0x18, "Damage" )                    PORT_DIPLOCATION("SW(B):4,5")
	PORT_DIPSETTING(    0x10, "Small" )                 // Check code at 0x006f4e
	PORT_DIPSETTING(    0x18, DEF_STR( Normal ) )
	PORT_DIPSETTING(    0x08, "Big" )
	PORT_DIPSETTING(    0x00, "Biggest" )
	PORT_DIPUNUSED_DIPLOC( 0x20, 0x20, "SW(B):6" )
	PORT_DIPUNUSED_DIPLOC( 0x40, 0x40, "SW(B):7" )
	PORT_DIPUNUSED_DIPLOC( 0x80, 0x80, "SW(B):8" )

	PORT_START("DSWC")
	PORT_DIPUNUSED_DIPLOC( 0x01, 0x01, "SW(C):1" )
	PORT_DIPUNUSED_DIPLOC( 0x02, 0x02, "SW(C):2" )
	PORT_DIPNAME( 0x04, 0x04, DEF_STR( Free_Play ) )        PORT_DIPLOCATION("SW(C):3")
	PORT_DIPSETTING(    0x04, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x08, "Freeze" )                    PORT_DIPLOCATION("SW(C):4")
	PORT_DIPSETTING(    0x08, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x10, DEF_STR( Flip_Screen ) )      PORT_DIPLOCATION("SW(C):5")
	PORT_DIPSETTING(    0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x00, DEF_STR( Demo_Sounds ) )      PORT_DIPLOCATION("SW(C):6")
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x00, DEF_STR( Allow_Continue ) )   PORT_DIPLOCATION("SW(C):7")
	PORT_DIPSETTING(    0x40, DEF_STR( No ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Yes ) )
	PORT_DIPNAME( 0x80, 0x80, "Game Mode")                  PORT_DIPLOCATION("SW(C):8")
	PORT_DIPSETTING(    0x80, "Game" )
	PORT_DIPSETTING(    0x00, DEF_STR( Test ) )
INPUT_PORTS_END

/* To enable other choices in the "test mode", you must press ("P1 Button 1" ('Ctrl')
   or "P1 Button 2" ('Alt')) when "Service Mode" is ON */
static INPUT_PORTS_START( ffight )
	PORT_INCLUDE( cps1_3b )

	PORT_MODIFY("IN1")
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_PLAYER(1) PORT_NAME ("P1 Button 3 (Cheat)")
	PORT_BIT( 0x4000, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_PLAYER(2) PORT_NAME ("P2 Button 3 (Cheat)")

	PORT_START("DSWA")
	CPS1_COINAGE_1( "SW(A)" )
	PORT_DIPNAME( 0x40, 0x40, "2 Coins to Start, 1 to Continue" )   PORT_DIPLOCATION("SW(A):7")
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPUNUSED_DIPLOC( 0x80, 0x80, "SW(A):8" )

	PORT_START("DSWB")
	PORT_DIPNAME( 0x07, 0x04, "Difficulty Level 1" )                PORT_DIPLOCATION("SW(B):1,2,3")
	PORT_DIPSETTING(    0x07, DEF_STR( Easiest ) )      // "01"
	PORT_DIPSETTING(    0x06, DEF_STR( Easier ) )       // "02"
	PORT_DIPSETTING(    0x05, DEF_STR( Easy ) )         // "03"
	PORT_DIPSETTING(    0x04, DEF_STR( Normal ) )       // "04"
	PORT_DIPSETTING(    0x03, DEF_STR( Medium ) )       // "05"
	PORT_DIPSETTING(    0x02, DEF_STR( Hard ) )         // "06"
	PORT_DIPSETTING(    0x01, DEF_STR( Harder ) )       // "07"
	PORT_DIPSETTING(    0x00, DEF_STR( Hardest ) )      // "08"
	PORT_DIPNAME( 0x18, 0x10, "Difficulty Level 2" )                PORT_DIPLOCATION("SW(B):4,5")
	PORT_DIPSETTING(    0x18, DEF_STR( Easy ) )         // "01"
	PORT_DIPSETTING(    0x10, DEF_STR( Normal ) )       // "02"
	PORT_DIPSETTING(    0x08, DEF_STR( Hard ) )         // "03"
	PORT_DIPSETTING(    0x00, DEF_STR( Hardest ) )      // "04"
	PORT_DIPNAME( 0x60, 0x60, DEF_STR( Bonus_Life ) )               PORT_DIPLOCATION("SW(B):6,7")
	PORT_DIPSETTING(    0x60, "100k" )
	PORT_DIPSETTING(    0x40, "200k" )
	PORT_DIPSETTING(    0x20, "100k and every 200k" )
	PORT_DIPSETTING(    0x00, DEF_STR( None ) )
	PORT_DIPUNUSED_DIPLOC( 0x80, 0x80, "SW(B):8" )

	PORT_START("DSWC")
	PORT_DIPNAME( 0x03, 0x03, DEF_STR( Lives ) )                    PORT_DIPLOCATION("SW(C):1,2")
	PORT_DIPSETTING(    0x00, "1" )
	PORT_DIPSETTING(    0x03, "2" )
	PORT_DIPSETTING(    0x02, "3" )
	PORT_DIPSETTING(    0x01, "4" )
	PORT_DIPNAME( 0x04, 0x04, DEF_STR( Free_Play ) )                PORT_DIPLOCATION("SW(C):3")
	PORT_DIPSETTING(    0x04, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x08, "Freeze" )                            PORT_DIPLOCATION("SW(C):4")
	PORT_DIPSETTING(    0x08, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x10, DEF_STR( Flip_Screen ) )              PORT_DIPLOCATION("SW(C):5")
	PORT_DIPSETTING(    0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x00, DEF_STR( Demo_Sounds ) )              PORT_DIPLOCATION("SW(C):6")
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x00, DEF_STR( Allow_Continue ) )           PORT_DIPLOCATION("SW(C):7")
	PORT_DIPSETTING(    0x40, DEF_STR( No ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Yes ) )
	PORT_DIPNAME( 0x80, 0x80, "Game Mode")                          PORT_DIPLOCATION("SW(C):8")
	PORT_DIPSETTING(    0x80, "Game" )
	PORT_DIPSETTING(    0x00, DEF_STR( Test ) )
INPUT_PORTS_END

static INPUT_PORTS_START( 1941 )
	PORT_INCLUDE( cps1_2b )

	PORT_START("DSWA")
	CPS1_COINAGE_1( "SW(A)" )
	PORT_DIPNAME( 0x40, 0x40, "2 Coins to Start, 1 to Continue" )   PORT_DIPLOCATION("SW(A):7")
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPUNUSED_DIPLOC( 0x80, 0x80, "SW(A):8" )

	PORT_START("DSWB")
	CPS1_DIFFICULTY_1( "SW(B)" )
	PORT_DIPNAME( 0x18, 0x18, "Level Up Timer" )                    PORT_DIPLOCATION("SW(B):4,5")
	PORT_DIPSETTING(    0x18, "More Slowly" )
	PORT_DIPSETTING(    0x10, "Slowly" )
	PORT_DIPSETTING(    0x08, "Quickly" )
	PORT_DIPSETTING(    0x00, "More Quickly" )
	PORT_DIPNAME( 0x60, 0x60, "Bullet's Speed" )                    PORT_DIPLOCATION("SW(B):6,7")
	PORT_DIPSETTING(    0x60, "Very Slow" )
	PORT_DIPSETTING(    0x40, "Slow" )
	PORT_DIPSETTING(    0x20, "Fast" )
	PORT_DIPSETTING(    0x00, "Very Fast" )
	PORT_DIPNAME( 0x80, 0x80, "Initial Vitality" )                  PORT_DIPLOCATION("SW(B):8")
	PORT_DIPSETTING(    0x80, "3 Bars" )
	PORT_DIPSETTING(    0x00, "4 Bars" )

	PORT_START("DSWC")
	PORT_DIPNAME( 0x01, 0x01, "Throttle Game Speed" )               PORT_DIPLOCATION("SW(C):1")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )                      // turning this off will break the game
	PORT_DIPSETTING(    0x01, DEF_STR( On ) )
	PORT_DIPUNUSED_DIPLOC( 0x02, 0x02, "SW(C):2" )
	PORT_DIPNAME( 0x04, 0x04, DEF_STR( Free_Play ) )                PORT_DIPLOCATION("SW(C):3")
	PORT_DIPSETTING(    0x04, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x08, "Freeze" )                            PORT_DIPLOCATION("SW(C):4")
	PORT_DIPSETTING(    0x08, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x10, DEF_STR( Flip_Screen ) )              PORT_DIPLOCATION("SW(C):5")
	PORT_DIPSETTING(    0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x00, DEF_STR( Demo_Sounds ) )              PORT_DIPLOCATION("SW(C):6")
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x00, DEF_STR( Allow_Continue ) )           PORT_DIPLOCATION("SW(C):7")
	PORT_DIPSETTING(    0x40, DEF_STR( No ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Yes ) )
	PORT_DIPNAME( 0x80, 0x80, "Game Mode")                          PORT_DIPLOCATION("SW(C):8")
	PORT_DIPSETTING(    0x80, "Game" )
	PORT_DIPSETTING(    0x00, DEF_STR( Test ) )
INPUT_PORTS_END

static INPUT_PORTS_START( mercs )
	PORT_INCLUDE( cps1_3players )

	PORT_MODIFY("IN0")
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("DSWA")
	CPS1_COINAGE_2( "SW(A)" )
	PORT_DIPUNUSED_DIPLOC( 0x08, 0x08, "SW(A):4" )                  // These three switches are not documented in
	PORT_DIPUNUSED_DIPLOC( 0x10, 0x10, "SW(A):5" )                  // the manual
	PORT_DIPUNUSED_DIPLOC( 0x20, 0x20, "SW(A):6" )
	PORT_DIPNAME( 0x40, 0x40, "2 Coins to Start, 1 to Continue" )   PORT_DIPLOCATION("SW(A):7")
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPUNUSED_DIPLOC( 0x80, 0x80, "SW(A):8" )                  // This switch is not documented in the manual

	PORT_START("DSWB")
	CPS1_DIFFICULTY_1( "SW(B)" )
	PORT_DIPNAME( 0x08, 0x08, "Coin Slots" )                        PORT_DIPLOCATION("SW(B):4")
	PORT_DIPSETTING(    0x00, "1" )
	PORT_DIPSETTING(    0x08, "3" )                                 // This setting can't be used in two-player mode
	PORT_DIPNAME( 0x10, 0x10, "Play Mode" )                         PORT_DIPLOCATION("SW(B):5")
	PORT_DIPSETTING(    0x00, "2 Players" )
	PORT_DIPSETTING(    0x10, "3 Players" )
	PORT_DIPUNUSED_DIPLOC( 0x20, 0x20, "SW(B):6" )                  // These three switches are not documented in
	PORT_DIPUNUSED_DIPLOC( 0x40, 0x40, "SW(B):7" )                  // the manual
	PORT_DIPUNUSED_DIPLOC( 0x80, 0x80, "SW(B):8" )

	PORT_START("DSWC")
	PORT_DIPUNUSED_DIPLOC( 0x01, 0x01, "SW(C):1" )                  // These three switches are not documented in
	PORT_DIPUNUSED_DIPLOC( 0x02, 0x02, "SW(C):2" )                  // the manual
	PORT_DIPUNUSED_DIPLOC( 0x04, 0x04, "SW(C):3" )
	PORT_DIPNAME( 0x08, 0x08, "Freeze" )                            PORT_DIPLOCATION("SW(C):4")
	PORT_DIPSETTING(    0x08, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x10, DEF_STR( Flip_Screen ) )              PORT_DIPLOCATION("SW(C):5")
	PORT_DIPSETTING(    0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x00, DEF_STR( Demo_Sounds ) )              PORT_DIPLOCATION("SW(C):6")
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x00, DEF_STR( Allow_Continue ) )           PORT_DIPLOCATION("SW(C):7")
	PORT_DIPSETTING(    0x40, DEF_STR( No ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Yes ) )
	PORT_SERVICE_DIPLOC( 0x80, IP_ACTIVE_LOW, "SW(C):8" )
INPUT_PORTS_END

/* According to code at 0x001c4e ('mtwins') or ('chikij') , ALL bits 0 to 5 of DSWA
   must be ON to have "2 Coins/1 Credit (1 to continue)" for both coin slots.
   But according to routine starting at 0x06b27c ('mtwins') or 0x06b4fa ('chikij'),
   bit 6 of DSWA is tested to have the same "feature" in the "test mode".

   Bits 3 and 4 of DSWB affect the number of lives AND the level of damage when you get hit.
   When bit 5 of DSWB is ON you ALWAYS have 1 life but more energy (0x38 instead of 0x20).
   Useful addresses to know :
     - 0xff147b.b : lives  (player 1)
     - 0xff153b.b : lives  (player 2)
     - 0xff14ab.w : energy (player 1)
     - 0xff156b.w : energy (player 2)
*/
static INPUT_PORTS_START( mtwins )
	PORT_INCLUDE( cps1_3b )

	PORT_MODIFY("IN0")
	PORT_SERVICE_NO_TOGGLE( 0x40, IP_ACTIVE_LOW )

	PORT_START("DSWA")
	CPS1_COINAGE_1( "SW(A)" )
	PORT_DIPUNUSED_DIPLOC( 0x40, 0x40, "SW(A):7" )
	PORT_DIPUNUSED_DIPLOC( 0x80, 0x80, "SW(A):8" )

	PORT_START("DSWB")
	CPS1_DIFFICULTY_1( "SW(B)" )
	PORT_DIPNAME( 0x38, 0x18, DEF_STR( Lives ) )            PORT_DIPLOCATION("SW(B):4,5,6")
//  PORT_DIPSETTING(    0x30, "1" )                         // 0x38 energy, smallest damage
//  PORT_DIPSETTING(    0x38, "1" )                         // 0x38 energy, small damage
//  PORT_DIPSETTING(    0x28, "1" )                         // 0x38 energy, big damage
//  PORT_DIPSETTING(    0x20, "1" )                         // 0x38 energy, biggest damage
	PORT_DIPSETTING(    0x10, "1" )                         // 0x20 energy, smallest damage
	PORT_DIPSETTING(    0x18, "2" )                         // 0x20 energy, small damage
	PORT_DIPSETTING(    0x08, "3" )                         // 0x20 energy, big damage
	PORT_DIPSETTING(    0x00, "4" )                         // 0x20 energy, biggest damage
	PORT_DIPUNUSED_DIPLOC( 0x40, 0x40, "SW(B):7" )
	PORT_DIPUNUSED_DIPLOC( 0x80, 0x80, "SW(B):8" )

	PORT_START("DSWC")
	PORT_DIPUNUSED_DIPLOC( 0x01, 0x01, "SW(C):1" )
	PORT_DIPUNUSED_DIPLOC( 0x02, 0x02, "SW(C):2" )
	PORT_DIPNAME( 0x04, 0x04, DEF_STR( Free_Play ) )        PORT_DIPLOCATION("SW(C):3")
	PORT_DIPSETTING(    0x04, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x08, "Freeze" )                    PORT_DIPLOCATION("SW(C):4")
	PORT_DIPSETTING(    0x08, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x10, DEF_STR( Flip_Screen ) )      PORT_DIPLOCATION("SW(C):5")
	PORT_DIPSETTING(    0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x00, DEF_STR( Demo_Sounds ) )      PORT_DIPLOCATION("SW(C):6")
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x00, DEF_STR( Allow_Continue ) )   PORT_DIPLOCATION("SW(C):7")
	PORT_DIPSETTING(    0x40, DEF_STR( No ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Yes ) )
	PORT_DIPNAME( 0x80, 0x80, "Game Mode")                  PORT_DIPLOCATION("SW(C):8")
	PORT_DIPSETTING(    0x80, "Game" )
	PORT_DIPSETTING(    0x00, DEF_STR( Test ) )
INPUT_PORTS_END

/* I guess that bit 8 of DSWB was used for debug purpose :
     - code at 0x001094 : move players during "attract mode"
     - code at 0x019b62 ('msword' and 'mswordr1'), 0x019bde ('mswordu') or 0x019c26 ('mswordj') : unknown effect
     - code at 0x01c322 ('msword' and 'mswordr1'), 0x01c39e ('mswordu') or 0x01c3e0 ('mswordj') : unknown effect
   These features are not available because of the 'bra' instruction after the test of bit 7. */
static INPUT_PORTS_START( msword )
	PORT_INCLUDE( cps1_3b )

	PORT_MODIFY("IN0")
	PORT_SERVICE_NO_TOGGLE( 0x40, IP_ACTIVE_LOW )

	PORT_START("DSWA")
	CPS1_COINAGE_1( "SW(A)" )
	PORT_DIPNAME( 0x40, 0x40, "2 Coins to Start, 1 to Continue" )       PORT_DIPLOCATION("SW(A):7")
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPUNUSED_DIPLOC( 0x80, 0x80, "SW(A):8" )

	PORT_START("DSWB")
	PORT_DIPNAME( 0x07, 0x04, "Player's vitality consumption" )         PORT_DIPLOCATION("SW(B):1,2,3") // "Level 1"
	PORT_DIPSETTING(    0x07, "1 (Easiest)" )                   // "Easy 3"             (-1 every 28 frames)
	PORT_DIPSETTING(    0x06, "2" )                             // "Easy 2"             (-1 every 24 frames)
	PORT_DIPSETTING(    0x05, "3" )                             // "Easy 1"             (-1 every 20 frames)
	PORT_DIPSETTING(    0x04, "4 (Normal)" )                    // DEF_STR( Normal )    (-1 every 18 frames)
	PORT_DIPSETTING(    0x03, "5" )                             // "Difficult 1"        (-1 every 16 frames)
	PORT_DIPSETTING(    0x02, "6" )                             // "Difficult 2"        (-1 every 14 frames)
	PORT_DIPSETTING(    0x01, "7" )                             // "Difficult 3"        (-1 every 12 frames)
	PORT_DIPSETTING(    0x00, "8 (Hardest)" )                   // "Difficult 4"        (-1 every 8 frames)
	PORT_DIPNAME( 0x38, 0x38, "Enemy's vitality and attacking power" )  PORT_DIPLOCATION("SW(B):4,5,6") // "Level 2"
	PORT_DIPSETTING(    0x20, "1 (Easiest)" )                   // "Easy 3"
	PORT_DIPSETTING(    0x28, "2" )                             // "Easy 2"
	PORT_DIPSETTING(    0x30, "3" )                             // "Easy 1"
	PORT_DIPSETTING(    0x38, "4 (Normal)" )                    // DEF_STR( Normal )
	PORT_DIPSETTING(    0x18, "5" )                             // "Difficult 1"
	PORT_DIPSETTING(    0x10, "6" )                             // "Difficult 2"
	PORT_DIPSETTING(    0x08, "7" )                             // "Difficult 3"
	PORT_DIPSETTING(    0x00, "8 (Hardest)" )                   // "Difficult 4"
	PORT_DIPNAME( 0x40, 0x00, "Stage Select" )                          PORT_DIPLOCATION("SW(B):7")
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPUNUSED_DIPLOC( 0x80, 0x80, "SW(B):8" )

	PORT_START("DSWC")
	PORT_DIPNAME( 0x03, 0x03, "Vitality Packs" )                        PORT_DIPLOCATION("SW(C):1,2")
	PORT_DIPSETTING(    0x00, "1" )                             // 0x0320
	PORT_DIPSETTING(    0x03, "2" )                             // 0x0640
	PORT_DIPSETTING(    0x02, "3 (2 when continue)" )           // 0x0960 (0x0640 when continue)
	PORT_DIPSETTING(    0x01, "4 (3 when continue)" )           // 0x0c80 (0x0960 when continue)
	PORT_DIPNAME( 0x04, 0x04, DEF_STR( Free_Play ) )                    PORT_DIPLOCATION("SW(C):3")
	PORT_DIPSETTING(    0x04, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x08, "Freeze" )                                PORT_DIPLOCATION("SW(C):4")
	PORT_DIPSETTING(    0x08, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x10, DEF_STR( Flip_Screen ) )                  PORT_DIPLOCATION("SW(C):5")
	PORT_DIPSETTING(    0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x00, DEF_STR( Demo_Sounds ) )                  PORT_DIPLOCATION("SW(C):6")
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x00, DEF_STR( Allow_Continue ) )               PORT_DIPLOCATION("SW(C):7")
	PORT_DIPSETTING(    0x40, DEF_STR( No ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Yes ) )
	PORT_DIPNAME( 0x80, 0x80, "Game Mode")                              PORT_DIPLOCATION("SW(C):8")
	PORT_DIPSETTING(    0x80, "Game" )
	PORT_DIPSETTING(    0x00, DEF_STR( Test ) )
INPUT_PORTS_END

static INPUT_PORTS_START( cawing )
	PORT_INCLUDE( cps1_3b )

	PORT_MODIFY("IN0")
	PORT_SERVICE_NO_TOGGLE( 0x40, IP_ACTIVE_LOW )

	PORT_START("DSWA")
	CPS1_COINAGE_1( "SW(A)" )
	PORT_DIPNAME( 0x40, 0x40, "2 Coins to Start, 1 to Continue" )       PORT_DIPLOCATION("SW(A):7")
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )                          // Overrides all other coinage settings
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )                           // according to manual
	PORT_DIPUNUSED_DIPLOC( 0x80, 0x80, "SW(A):8" )                      // This switch is not documented

	PORT_START("DSWB")
	PORT_DIPNAME( 0x07, 0x04, "Difficulty Level (Enemy's Strength)" )   PORT_DIPLOCATION("SW(B):1,2,3")
	PORT_DIPSETTING(    0x07, "1 (Easiest)" )
	PORT_DIPSETTING(    0x06, "2" )
	PORT_DIPSETTING(    0x05, "3" )
	PORT_DIPSETTING(    0x04, "4 (Normal)" )
	PORT_DIPSETTING(    0x03, "5" )
	PORT_DIPSETTING(    0x02, "6" )
	PORT_DIPSETTING(    0x01, "7" )
	PORT_DIPSETTING(    0x00, "8 (Hardest)" )
	PORT_DIPNAME( 0x18, 0x18, "Difficulty Level (Player's Strength)" )  PORT_DIPLOCATION("SW(B):4,5")
	PORT_DIPSETTING(    0x10, DEF_STR( Easy ) )
	PORT_DIPSETTING(    0x18, DEF_STR( Normal ) )
	PORT_DIPSETTING(    0x08, DEF_STR( Hard ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Hardest ) )
	PORT_DIPUNUSED_DIPLOC( 0x20, 0x20, "SW(B):6" )                      // This switch is not documented
	PORT_DIPUNUSED_DIPLOC( 0x40, 0x40, "SW(B):7" )                      // This switch is not documented
	PORT_DIPUNUSED_DIPLOC( 0x80, 0x80, "SW(B):8" )                      // This switch is not documented

	PORT_START("DSWC")
	PORT_DIPUNUSED_DIPLOC( 0x01, 0x01, "SW(C):1" )                      // This switch is not documented
	PORT_DIPUNUSED_DIPLOC( 0x02, 0x02, "SW(C):2" )                      // This switch is not documented
	PORT_DIPNAME( 0x04, 0x04, DEF_STR( Free_Play ) )                    PORT_DIPLOCATION("SW(C):3")
	PORT_DIPSETTING(    0x04, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x08, "Freeze" )                                PORT_DIPLOCATION("SW(C):4")
	PORT_DIPSETTING(    0x08, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x10, DEF_STR( Flip_Screen ) )                  PORT_DIPLOCATION("SW(C):5")
	PORT_DIPSETTING(    0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x00, DEF_STR( Demo_Sounds ) )                  PORT_DIPLOCATION("SW(C):6")
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x00, DEF_STR( Allow_Continue ) )               PORT_DIPLOCATION("SW(C):7")
	PORT_DIPSETTING(    0x40, DEF_STR( No ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Yes ) )
	PORT_DIPNAME( 0x80, 0x80, "Game Mode")                              PORT_DIPLOCATION("SW(C):8")
	PORT_DIPSETTING(    0x80, "Game" )
	PORT_DIPSETTING(    0x00, DEF_STR( Test ) )
INPUT_PORTS_END

/* "Debug" features to be implemented */
static INPUT_PORTS_START( nemo )
	PORT_INCLUDE( cps1_3b )

	PORT_MODIFY("IN0")
	PORT_SERVICE_NO_TOGGLE( 0x40, IP_ACTIVE_LOW )

	PORT_START("DSWA")
	CPS1_COINAGE_1( "SW(A)" )
	PORT_DIPNAME( 0x40, 0x40, "2 Coins to Start, 1 to Continue" )   PORT_DIPLOCATION("SW(A):7")
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPUNUSED_DIPLOC( 0x80, 0x80, "SW(A):8" )

	PORT_START("DSWB")
	CPS1_DIFFICULTY_1( "SW(B)" )
	PORT_DIPNAME( 0x18, 0x18, "Life Bar" )                          PORT_DIPLOCATION("SW(B):4,5")
	PORT_DIPSETTING(    0x00, "Minimun" )
	PORT_DIPSETTING(    0x18, DEF_STR( Medium ) )
//  PORT_DIPSETTING(    0x10, DEF_STR( Medium ) )
	PORT_DIPSETTING(    0x08, "Maximum" )
	PORT_DIPUNUSED_DIPLOC( 0x20, 0x20, "SW(B):6" )
	PORT_DIPUNUSED_DIPLOC( 0x40, 0x40, "SW(B):7" )
	PORT_DIPUNUSED_DIPLOC( 0x80, 0x80, "SW(B):8" )

	PORT_START("DSWC")
	PORT_DIPNAME( 0x03, 0x03, DEF_STR( Lives ) )                    PORT_DIPLOCATION("SW(C):1,2")
	PORT_DIPSETTING(    0x02, "1" )
	PORT_DIPSETTING(    0x03, "2" )
	PORT_DIPSETTING(    0x01, "3" )
	PORT_DIPSETTING(    0x00, "4" )
	PORT_DIPNAME( 0x04, 0x04, DEF_STR( Free_Play ) )                PORT_DIPLOCATION("SW(C):3")
	PORT_DIPSETTING(    0x04, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x08, "Freeze" )                            PORT_DIPLOCATION("SW(C):4")
	PORT_DIPSETTING(    0x08, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x10, DEF_STR( Flip_Screen ) )              PORT_DIPLOCATION("SW(C):5")
	PORT_DIPSETTING(    0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x00, DEF_STR( Demo_Sounds ) )              PORT_DIPLOCATION("SW(C):6")
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x00, DEF_STR( Allow_Continue ) )           PORT_DIPLOCATION("SW(C):7")
	PORT_DIPSETTING(    0x40, DEF_STR( No ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Yes ) )
	PORT_DIPNAME( 0x80, 0x80, "Game Mode")                          PORT_DIPLOCATION("SW(C):8")
	PORT_DIPSETTING(    0x80, "Game" )
	PORT_DIPSETTING(    0x00, DEF_STR( Test ) )                 // To enable the "debug" features
INPUT_PORTS_END

INPUT_PORTS_START( sf2 )
	PORT_INCLUDE( cps1_6b )

	PORT_MODIFY("IN0")
	PORT_SERVICE_NO_TOGGLE( 0x40, IP_ACTIVE_LOW )

	PORT_START("DSWA")
	CPS1_COINAGE_1( "SW(A)" )
	PORT_DIPNAME( 0x40, 0x40, "2 Coins to Start, 1 to Continue" )   PORT_DIPLOCATION("SW(A):7")
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPUNUSED_DIPLOC( 0x80, 0x80, "SW(A):8" )

	PORT_START("DSWB")
	CPS1_DIFFICULTY_1( "SW(B)" )
	PORT_DIPUNUSED_DIPLOC( 0x08, 0x08, "SW(B):4" )
	PORT_DIPUNUSED_DIPLOC( 0x10, 0x10, "SW(B):5" )
	PORT_DIPUNUSED_DIPLOC( 0x20, 0x20, "SW(B):6" )
	PORT_DIPUNUSED_DIPLOC( 0x40, 0x40, "SW(B):7" )
	PORT_DIPUNUSED_DIPLOC( 0x80, 0x80, "SW(B):8" )

	PORT_START("DSWC")
	PORT_DIPUNUSED_DIPLOC( 0x01, 0x01, "SW(C):1" )
	PORT_DIPUNUSED_DIPLOC( 0x02, 0x02, "SW(C):2" )
	PORT_DIPNAME( 0x04, 0x04, DEF_STR( Free_Play ) )                PORT_DIPLOCATION("SW(C):3")
	PORT_DIPSETTING(    0x04, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x08, "Freeze" )                            PORT_DIPLOCATION("SW(C):4")
	PORT_DIPSETTING(    0x08, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x10, DEF_STR( Flip_Screen ) )              PORT_DIPLOCATION("SW(C):5")
	PORT_DIPSETTING(    0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x00, DEF_STR( Demo_Sounds ) )              PORT_DIPLOCATION("SW(C):6")
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x00, DEF_STR( Allow_Continue ) )           PORT_DIPLOCATION("SW(C):7")
	PORT_DIPSETTING(    0x40, DEF_STR( No ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Yes ) )
	PORT_DIPNAME( 0x80, 0x80, "Game Mode")                          PORT_DIPLOCATION("SW(C):8")
	PORT_DIPSETTING(    0x80, "Game" )
	PORT_DIPSETTING(    0x00, DEF_STR( Test ) )

INPUT_PORTS_END

/* Needs further checking */
static INPUT_PORTS_START( sf2j )
	PORT_INCLUDE( sf2 )

	PORT_MODIFY("DSWB")
	PORT_DIPNAME( 0x08, 0x00, "2 Players Game" )                    PORT_DIPLOCATION("SW(B):4")
	PORT_DIPSETTING(    0x08, "1 Credit/No Continue" )
	PORT_DIPSETTING(    0x00, "2 Credits/Winner Continue" ) //Winner stays, loser pays, in other words.
INPUT_PORTS_END

static INPUT_PORTS_START( sf2hack )
	PORT_INCLUDE( sf2 )

	PORT_MODIFY("IN2")      /* Extra buttons */
	PORT_BIT( 0x00ff, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0100, IP_ACTIVE_LOW, IPT_BUTTON4 ) PORT_NAME("P1 Short Kick") PORT_PLAYER(1)
	PORT_BIT( 0x0200, IP_ACTIVE_LOW, IPT_BUTTON5 ) PORT_NAME("P1 Forward Kick") PORT_PLAYER(1)
	PORT_BIT( 0x0400, IP_ACTIVE_LOW, IPT_BUTTON6 ) PORT_NAME("P1 Roundhouse Kick") PORT_PLAYER(1)
	PORT_BIT( 0x0800, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x1000, IP_ACTIVE_LOW, IPT_BUTTON4 ) PORT_NAME("P2 Short Kick") PORT_PLAYER(2)
	PORT_BIT( 0x2000, IP_ACTIVE_LOW, IPT_BUTTON5 ) PORT_NAME("P2 Forward Kick") PORT_PLAYER(2)
	PORT_BIT( 0x4000, IP_ACTIVE_LOW, IPT_BUTTON6 ) PORT_NAME("P2 Roundhouse Kick") PORT_PLAYER(2)
	PORT_BIT( 0x8000, IP_ACTIVE_LOW, IPT_UNKNOWN )
INPUT_PORTS_END

static INPUT_PORTS_START( sf2m2 )
	PORT_INCLUDE( sf2hack )

	PORT_MODIFY("DSWB")
	PORT_DIPNAME( 0x10, 0x00, "It needs to be High" )                       PORT_DIPLOCATION("SW(B):5")
	PORT_DIPSETTING(    0x10, DEF_STR ( Low ) )
	PORT_DIPSETTING(    0x00, DEF_STR ( High ) )
INPUT_PORTS_END

static INPUT_PORTS_START( sf2m4 )
	PORT_INCLUDE( sf2hack )

	PORT_MODIFY("DSWB")
	PORT_DIPNAME( 0x08, 0x00, "2 Players Game" )                    PORT_DIPLOCATION("SW(B):4")
	PORT_DIPSETTING(    0x08, "1 Credit/No Continue" )
	PORT_DIPSETTING(    0x00, "2 Credits/Winner Continue" ) //Winner stays, loser pays, in other words.
INPUT_PORTS_END

/* SWB.4, SWB.5 and SWB.6 need to be enabled simultaneously for turbo mode */
static INPUT_PORTS_START( sf2amf )
	PORT_INCLUDE( sf2hack )

	PORT_MODIFY("DSWB")
	PORT_DIPNAME( 0x08, 0x08, "Turbo Mode Switch 1 of 3" )   PORT_DIPLOCATION("SW(B):4")
	PORT_DIPSETTING(    0x08, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x10, "Turbo Mode Switch 2 of 3" )   PORT_DIPLOCATION("SW(B):5")
	PORT_DIPSETTING(    0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x20, "Turbo Mode Switch 3 of 3" )   PORT_DIPLOCATION("SW(B):6")
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
INPUT_PORTS_END

static INPUT_PORTS_START( 3wonders )
	PORT_INCLUDE( cps1_3b )

	PORT_MODIFY("IN0")
	PORT_SERVICE_NO_TOGGLE( 0x40, IP_ACTIVE_LOW )

	PORT_START("DSWA")
	CPS1_COINAGE_1( "SW(A)" )
	PORT_DIPNAME( 0x40, 0x40, "2 Coins to Start, 1 to Continue" ) PORT_CONDITION("DSWA", 0x3f,NOTEQUALS,0x00) PORT_DIPLOCATION("SW(A):7")
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x40, DEF_STR( Free_Play ) ) PORT_CONDITION("DSWA", 0x3f, EQUALS, 0x00) PORT_DIPLOCATION("SW(A):7")
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	/* Free Play: ALL bits 0 to 7 must be ON ; 4C_1C, 4C_1C, 2 Coins to Start, 1 to Continue ON */
	PORT_DIPNAME( 0x80, 0x80, "Freeze" )                            PORT_DIPLOCATION("SW(A):8")
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

	PORT_START("DSWB")
	PORT_DIPNAME( 0x03, 0x02, "Lives (Midnight Wanderers)" )        PORT_DIPLOCATION("SW(B):1,2")
	PORT_DIPSETTING(    0x03, "1" )
	PORT_DIPSETTING(    0x02, "2" )
	PORT_DIPSETTING(    0x01, "3" )
	PORT_DIPSETTING(    0x00, "5" )
	PORT_DIPNAME( 0x0c, 0x08, "Difficulty (Midnight Wanderers)" )   PORT_DIPLOCATION("SW(B):3,4")
	PORT_DIPSETTING(    0x0c, DEF_STR( Easy ) )
	PORT_DIPSETTING(    0x08, DEF_STR( Normal ) )
	PORT_DIPSETTING(    0x04, DEF_STR( Hard ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Hardest ) )
	PORT_DIPNAME( 0x30, 0x10, "Lives (Chariot)" )                   PORT_DIPLOCATION("SW(B):5,6")
	PORT_DIPSETTING(    0x30, "1" )
	PORT_DIPSETTING(    0x20, "2" )
	PORT_DIPSETTING(    0x10, "3" )
	PORT_DIPSETTING(    0x00, "5" )
	PORT_DIPNAME( 0xc0, 0x80, "Difficulty (Chariot)" )              PORT_DIPLOCATION("SW(B):7,8")
	PORT_DIPSETTING(    0xc0, DEF_STR( Easy ) )
	PORT_DIPSETTING(    0x80, DEF_STR( Normal ) )
	PORT_DIPSETTING(    0x40, DEF_STR( Hard ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Hardest ) )

	PORT_START("DSWC")
	PORT_DIPNAME( 0x03, 0x01, "Lives (Don't Pull)" )                PORT_DIPLOCATION("SW(C):1,2")
	PORT_DIPSETTING(    0x03, "1" )
	PORT_DIPSETTING(    0x02, "2" )
	PORT_DIPSETTING(    0x01, "3" )
	PORT_DIPSETTING(    0x00, "5" )
	PORT_DIPNAME( 0x0c, 0x08, "Difficulty (Don't Pull)" )           PORT_DIPLOCATION("SW(C):3,4")
	PORT_DIPSETTING(    0x0c, DEF_STR( Easy ) )
	PORT_DIPSETTING(    0x08, DEF_STR( Normal ) )
	PORT_DIPSETTING(    0x04, DEF_STR( Hard ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Hardest ) )
	PORT_DIPNAME( 0x10, 0x10, DEF_STR( Flip_Screen ) )              PORT_DIPLOCATION("SW(C):5")
	PORT_DIPSETTING(    0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x00, DEF_STR( Demo_Sounds ) )              PORT_DIPLOCATION("SW(C):6")
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x00, DEF_STR( Allow_Continue ) )           PORT_DIPLOCATION("SW(C):7")
	PORT_DIPSETTING(    0x40, DEF_STR( No ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Yes ) )
	PORT_DIPNAME( 0x80, 0x80, "Game Mode")                          PORT_DIPLOCATION("SW(C):8")
	PORT_DIPSETTING(    0x80, "Game" )
	PORT_DIPSETTING(    0x00, DEF_STR( Test ) )
INPUT_PORTS_END

static INPUT_PORTS_START( kod )
	PORT_INCLUDE( cps1_3players )

	PORT_MODIFY("IN0")
	PORT_SERVICE_NO_TOGGLE( 0x40, IP_ACTIVE_LOW )

	PORT_START("DSWA")
	CPS1_COINAGE_2( "SW(A)" )
	PORT_DIPNAME( 0x08, 0x08, "Coin Slots" )                        PORT_DIPLOCATION("SW(A):4")
	PORT_DIPSETTING(    0x00, "1" )
	PORT_DIPSETTING(    0x08, "3" )
	PORT_DIPNAME( 0x10, 0x10, "Play Mode" )                         PORT_DIPLOCATION("SW(A):5")
	PORT_DIPSETTING(    0x00, "2 Players" )
	PORT_DIPSETTING(    0x10, "3 Players" )
	PORT_DIPUNUSED_DIPLOC( 0x20, 0x20, "SW(A):6" )
	PORT_DIPNAME( 0x40, 0x40, "2 Coins to Start, 1 to Continue" )   PORT_DIPLOCATION("SW(A):7")
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPUNUSED_DIPLOC( 0x80, 0x80, "SW(A):8" )

	PORT_START("DSWB")
	CPS1_DIFFICULTY_1( "SW(B)" )
	PORT_DIPNAME( 0x38, 0x38, DEF_STR( Lives ) )                    PORT_DIPLOCATION("SW(B):4,5,6")
	PORT_DIPSETTING(    0x30, "1" )
	PORT_DIPSETTING(    0x38, "2" )
	PORT_DIPSETTING(    0x28, "3" )
	PORT_DIPSETTING(    0x20, "4" )
	PORT_DIPSETTING(    0x18, "5" )
	PORT_DIPSETTING(    0x10, "6" )
	PORT_DIPSETTING(    0x08, "7" )
	PORT_DIPSETTING(    0x00, "8" )
	PORT_DIPNAME( 0xc0, 0xc0, DEF_STR( Bonus_Life ) )               PORT_DIPLOCATION("SW(B):7,8")
	PORT_DIPSETTING(    0x80, "80k and every 400k" )
	PORT_DIPSETTING(    0x40, "160k and every 450k" )
	PORT_DIPSETTING(    0xc0, "200k and every 450k" )
	PORT_DIPSETTING(    0x00, DEF_STR( None ) )

	PORT_START("DSWC")
	PORT_DIPUNUSED_DIPLOC( 0x01, 0x01, "SW(C):1" )
	PORT_DIPUNUSED_DIPLOC( 0x02, 0x02, "SW(C):2" )
	PORT_DIPNAME( 0x04, 0x04, DEF_STR( Free_Play ) )                PORT_DIPLOCATION("SW(C):3")
	PORT_DIPSETTING(    0x04, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x08, "Freeze" )                            PORT_DIPLOCATION("SW(C):4")
	PORT_DIPSETTING(    0x08, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x10, DEF_STR( Flip_Screen ) )              PORT_DIPLOCATION("SW(C):5")
	PORT_DIPSETTING(    0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x00, DEF_STR( Demo_Sounds ) )              PORT_DIPLOCATION("SW(C):6")
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x00, DEF_STR( Allow_Continue ) )           PORT_DIPLOCATION("SW(C):7")
	PORT_DIPSETTING(    0x40, DEF_STR( No ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Yes ) )
	PORT_DIPNAME( 0x80, 0x80, "Game Mode")                          PORT_DIPLOCATION("SW(C):8")
	PORT_DIPSETTING(    0x80, "Game" )
	PORT_DIPSETTING(    0x00, DEF_STR( Test ) )
INPUT_PORTS_END

/* Needs further checking
   Same as kod but different "Bonus_life" values */
static INPUT_PORTS_START( kodr1 )
	PORT_INCLUDE( kod )

	PORT_MODIFY("DSWB")
	PORT_DIPNAME( 0xc0, 0xc0, DEF_STR( Bonus_Life ) )               PORT_DIPLOCATION("SW(B):7,8")
	PORT_DIPSETTING(    0x80, "80k and every 400k" )
	PORT_DIPSETTING(    0xc0, "100k and every 450k" )
	PORT_DIPSETTING(    0x40, "160k and every 450k" )
	PORT_DIPSETTING(    0x00, DEF_STR( None ) )
INPUT_PORTS_END


static INPUT_PORTS_START( captcomm )
	PORT_INCLUDE( cps1_4players )

	PORT_MODIFY("IN0")
	PORT_SERVICE_NO_TOGGLE( 0x40, IP_ACTIVE_LOW )

	PORT_START("DSWA")
	CPS1_COINAGE_2( "SW(A)" )
	PORT_DIPUNUSED_DIPLOC( 0x08, 0x08, "SW(A):4" )                  // The manual says to leave these three
	PORT_DIPUNUSED_DIPLOC( 0x10, 0x10, "SW(A):5" )                  // switches off.  Does turning them on cause
	PORT_DIPUNUSED_DIPLOC( 0x20, 0x20, "SW(A):6" )                  // any "undesirable" behaviour?
	PORT_DIPNAME( 0x40, 0x40, "2 Coins to Start, 1 to Continue" )   PORT_DIPLOCATION("SW(A):7")
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPUNUSED_DIPLOC( 0x80, 0x80, "SW(A):8" )                  // Unused according to manual

	PORT_START("DSWB")
	PORT_DIPNAME( 0x07, 0x04, "Difficulty 1" )                      PORT_DIPLOCATION("SW(B):1,2,3")
	PORT_DIPSETTING(    0x07, "1 (Easiest)" )
	PORT_DIPSETTING(    0x06, "2" )
	PORT_DIPSETTING(    0x05, "3" )
	PORT_DIPSETTING(    0x04, "4 (Normal)" )
	PORT_DIPSETTING(    0x03, "5" )
	PORT_DIPSETTING(    0x02, "6" )
	PORT_DIPSETTING(    0x01, "7" )
	PORT_DIPSETTING(    0x00, "8 (Hardest)" )
	PORT_DIPNAME( 0x18, 0x10, "Difficulty 2" )                      PORT_DIPLOCATION("SW(B):4,5")
	PORT_DIPSETTING(    0x18, DEF_STR( Easy ) )
	PORT_DIPSETTING(    0x10, DEF_STR( Normal ) )
	PORT_DIPSETTING(    0x08, DEF_STR( Hard ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Hardest ) )
	PORT_DIPUNUSED_DIPLOC( 0x20, 0x20, "SW(B):6" )                  // Manual says to leave this switch off.
	PORT_DIPNAME( 0xc0, 0xc0, "Play Mode" )                         PORT_DIPLOCATION("SW(B):7,8")
	PORT_DIPSETTING(    0x40, "1 Players" ) // Actual setting is 4 players
	PORT_DIPSETTING(    0xc0, "2 Players" )
	PORT_DIPSETTING(    0x80, "3 Players" )
	PORT_DIPSETTING(    0x00, "4 Players" )

	PORT_START("DSWC")
	PORT_DIPNAME( 0x03, 0x03, DEF_STR( Lives ) )                    PORT_DIPLOCATION("SW(C):1,2")
	PORT_DIPSETTING(    0x00, "1" )
	PORT_DIPSETTING(    0x03, "2" )
	PORT_DIPSETTING(    0x02, "3" )
	PORT_DIPSETTING(    0x01, "4" )
	PORT_DIPNAME( 0x04, 0x04, DEF_STR( Free_Play ) )                PORT_DIPLOCATION("SW(C):3")
	PORT_DIPSETTING(    0x04, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x08, "Freeze" )                            PORT_DIPLOCATION("SW(C):4")
	PORT_DIPSETTING(    0x08, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x10, DEF_STR( Flip_Screen ) )              PORT_DIPLOCATION("SW(C):5")
	PORT_DIPSETTING(    0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x00, DEF_STR( Demo_Sounds ) )              PORT_DIPLOCATION("SW(C):6")
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x00, DEF_STR( Allow_Continue ) )           PORT_DIPLOCATION("SW(C):7")
	PORT_DIPSETTING(    0x40, DEF_STR( No ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Yes ) )
	PORT_DIPNAME( 0x80, 0x80, "Game Mode")                          PORT_DIPLOCATION("SW(C):8")
	PORT_DIPSETTING(    0x80, "Game" )
	PORT_DIPSETTING(    0x00, DEF_STR( Test ) )
INPUT_PORTS_END

INPUT_PORTS_START( knights )
	PORT_INCLUDE( cps1_3players )

	PORT_MODIFY("IN0")
	PORT_SERVICE_NO_TOGGLE( 0x40, IP_ACTIVE_LOW )

	PORT_START("DSWA")
	CPS1_COINAGE_2( "SW(A)" )
	PORT_DIPUNUSED_DIPLOC( 0x08, 0x08, "SW(A):4" )
	PORT_DIPUNUSED_DIPLOC( 0x10, 0x10, "SW(A):5" )
	PORT_DIPUNUSED_DIPLOC( 0x20, 0x20, "SW(A):6" )
	PORT_DIPNAME( 0x40, 0x40, "2 Coins to Start, 1 to Continue" )   PORT_DIPLOCATION("SW(A):7")
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPUNUSED_DIPLOC( 0x80, 0x80, "SW(A):8" )

	PORT_START("DSWB")
	PORT_DIPNAME( 0x07, 0x04, "Enemy's attack frequency" )          PORT_DIPLOCATION("SW(B):1,2,3")
	PORT_DIPSETTING(    0x07, "1 (Easiest)" )
	PORT_DIPSETTING(    0x06, "2" )
	PORT_DIPSETTING(    0x05, "3" )
	PORT_DIPSETTING(    0x04, "4 (Normal)" )
	PORT_DIPSETTING(    0x03, "5" )
	PORT_DIPSETTING(    0x02, "6" )
	PORT_DIPSETTING(    0x01, "7" )
	PORT_DIPSETTING(    0x00, "8 (Hardest)" )
	PORT_DIPNAME( 0x38, 0x38, "Enemy's attack power" )              PORT_DIPLOCATION("SW(B):4,5,6")
	PORT_DIPSETTING(    0x00, "1 (Easiest)" )
	PORT_DIPSETTING(    0x08, "2" )
	PORT_DIPSETTING(    0x10, "3" )
	PORT_DIPSETTING(    0x38, "4 (Normal)" )
	PORT_DIPSETTING(    0x30, "5" )
	PORT_DIPSETTING(    0x28, "6" )
	PORT_DIPSETTING(    0x20, "7" )
	PORT_DIPSETTING(    0x18, "8 (Hardest)" )
	PORT_DIPNAME( 0x40, 0x40, "Coin Slots" )                        PORT_DIPLOCATION("SW(B):7")
	PORT_DIPSETTING(    0x00, "1" )
	PORT_DIPSETTING(    0x40, "3" )
	PORT_DIPNAME( 0x80, 0x80, "Play Mode" )                         PORT_DIPLOCATION("SW(B):8")
	PORT_DIPSETTING(    0x00, "2 Players" )
	PORT_DIPSETTING(    0x80, "3 Players" )

	PORT_START("DSWC")
	PORT_DIPNAME( 0x03, 0x03, DEF_STR( Lives ) )                    PORT_DIPLOCATION("SW(C):1,2")
	PORT_DIPSETTING(    0x00, "1" )
	PORT_DIPSETTING(    0x03, "2" )
	PORT_DIPSETTING(    0x02, "3" )
	PORT_DIPSETTING(    0x01, "4" )
	PORT_DIPNAME( 0x04, 0x04, DEF_STR( Free_Play ) )                PORT_DIPLOCATION("SW(C):3")
	PORT_DIPSETTING(    0x04, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x08, "Freeze" )                            PORT_DIPLOCATION("SW(C):4")
	PORT_DIPSETTING(    0x08, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x10, DEF_STR( Flip_Screen ) )              PORT_DIPLOCATION("SW(C):5")
	PORT_DIPSETTING(    0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x00, DEF_STR( Demo_Sounds ) )              PORT_DIPLOCATION("SW(C):6")
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x00, DEF_STR( Allow_Continue ) )           PORT_DIPLOCATION("SW(C):7")
	PORT_DIPSETTING(    0x40, DEF_STR( No ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Yes ) )
	PORT_DIPNAME( 0x80, 0x80, "Game Mode")                          PORT_DIPLOCATION("SW(C):8")
	PORT_DIPSETTING(    0x80, "Game" )
	PORT_DIPSETTING(    0x00, DEF_STR( Test ) )
INPUT_PORTS_END

static INPUT_PORTS_START( varth )
	PORT_INCLUDE( cps1_3b )

	PORT_MODIFY("IN0")
	PORT_SERVICE_NO_TOGGLE( 0x40, IP_ACTIVE_LOW )

	PORT_START("DSWA")
	CPS1_COINAGE_1( "SW(A)" )
	PORT_DIPNAME( 0x40, 0x40, "2 Coins to Start, 1 to Continue" )   PORT_DIPLOCATION("SW(A):7")
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPUNUSED_DIPLOC( 0x80, 0x80, "SW(A):8" )

	PORT_START("DSWB")
	CPS1_DIFFICULTY_1( "SW(B)" )
	PORT_DIPNAME( 0x18, 0x10, DEF_STR( Bonus_Life ) )               PORT_DIPLOCATION("SW(B):4,5")
	PORT_DIPSETTING(    0x18, "600k and every 1.400k" )
	PORT_DIPSETTING(    0x10, "600k 2.000k and 4500k" )
	PORT_DIPSETTING(    0x08, "1.200k 3.500k" )
	PORT_DIPSETTING(    0x00, "2000k only" )
	PORT_DIPUNUSED_DIPLOC( 0x20, 0x20, "SW(B):6" )
	PORT_DIPUNUSED_DIPLOC( 0x40, 0x40, "SW(B):7" )
	PORT_DIPUNUSED_DIPLOC( 0x80, 0x80, "SW(B):8" )

	PORT_START("DSWC")
	PORT_DIPNAME( 0x03, 0x03, DEF_STR( Lives ) )                    PORT_DIPLOCATION("SW(C):1,2")
	PORT_DIPSETTING(    0x02, "1" )
	PORT_DIPSETTING(    0x01, "2" )
	PORT_DIPSETTING(    0x03, "3" )
	PORT_DIPSETTING(    0x00, "4" )
	PORT_DIPNAME( 0x04, 0x04, DEF_STR( Free_Play ) )                PORT_DIPLOCATION("SW(C):3")
	PORT_DIPSETTING(    0x04, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x08, "Freeze" )                            PORT_DIPLOCATION("SW(C):4")
	PORT_DIPSETTING(    0x08, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x10, DEF_STR( Flip_Screen ) )              PORT_DIPLOCATION("SW(C):5")
	PORT_DIPSETTING(    0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x00, DEF_STR( Demo_Sounds ) )              PORT_DIPLOCATION("SW(C):6")
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x00, DEF_STR( Allow_Continue ) )           PORT_DIPLOCATION("SW(C):7")
	PORT_DIPSETTING(    0x40, DEF_STR( No ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Yes ) )
	PORT_DIPNAME( 0x80, 0x80, "Game Mode")                          PORT_DIPLOCATION("SW(C):8")
	PORT_DIPSETTING(    0x80, "Game" )
	PORT_DIPSETTING(    0x00, DEF_STR( Test ) )
INPUT_PORTS_END

/* Needs further checking */
static INPUT_PORTS_START( cworld2j )
	PORT_INCLUDE( cps1_quiz )

	PORT_MODIFY("IN0")
	PORT_SERVICE_NO_TOGGLE( 0x40, IP_ACTIVE_LOW )

	PORT_START("DSWA")
	CPS1_COINAGE_2( "SW(A)" )
	PORT_DIPUNKNOWN_DIPLOC( 0x08, 0x08, "SW(A):4" )
	PORT_DIPUNKNOWN_DIPLOC( 0x10, 0x10, "SW(A):5" )
	PORT_DIPUNKNOWN_DIPLOC( 0x20, 0x20, "SW(A):6" )
	PORT_DIPNAME( 0x40, 0x40, "2 Coins to Start, 1 to Continue" )   PORT_DIPLOCATION("SW(A):7")
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, "Extended Test Mode" )                PORT_DIPLOCATION("SW(A):8")
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

	PORT_START("DSWB")
	PORT_DIPNAME( 0x07, 0x06, DEF_STR( Difficulty ) )               PORT_DIPLOCATION("SW(B):1,2,3")
	PORT_DIPSETTING(    0x06, "0" )
	PORT_DIPSETTING(    0x05, "1" )
	PORT_DIPSETTING(    0x04, "2" )
	PORT_DIPSETTING(    0x03, "3" )
	PORT_DIPSETTING(    0x02, "4" )
	PORT_DIPNAME( 0x18, 0x18, "Extend" )                            PORT_DIPLOCATION("SW(B):4,5")
	PORT_DIPSETTING(    0x18, "N" )
	PORT_DIPSETTING(    0x10, "E" )
	PORT_DIPSETTING(    0x00, "D" )
	PORT_DIPNAME( 0xe0, 0xe0, DEF_STR( Lives ) )                    PORT_DIPLOCATION("SW(B):6,7,8")
	PORT_DIPSETTING(    0x00, "1" )
	PORT_DIPSETTING(    0x80, "2" )
	PORT_DIPSETTING(    0xe0, "3" )
	PORT_DIPSETTING(    0xa0, "4" )
	PORT_DIPSETTING(    0xc0, "5" )

	PORT_START("DSWC")
	PORT_DIPUNKNOWN_DIPLOC( 0x01, 0x01, "SW(C):1" )
	PORT_DIPUNKNOWN_DIPLOC( 0x02, 0x02, "SW(C):2" )
	PORT_DIPNAME( 0x04, 0x04, DEF_STR( Free_Play ) )                PORT_DIPLOCATION("SW(C):3")
	PORT_DIPSETTING(    0x04, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x08, "Freeze" )                            PORT_DIPLOCATION("SW(C):4")
	PORT_DIPSETTING(    0x08, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x10, DEF_STR( Flip_Screen ) )              PORT_DIPLOCATION("SW(C):5")
	PORT_DIPSETTING(    0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x00, DEF_STR( Demo_Sounds ) )              PORT_DIPLOCATION("SW(C):6")
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x40, DEF_STR( Allow_Continue ) )           PORT_DIPLOCATION("SW(C):7")
	PORT_DIPSETTING(    0x00, DEF_STR( No ) )
	PORT_DIPSETTING(    0x40, DEF_STR( Yes ) )
	PORT_DIPNAME( 0x80, 0x80, "Game Mode")                          PORT_DIPLOCATION("SW(C):8")
	PORT_DIPSETTING(    0x80, "Game" )
	PORT_DIPSETTING(    0x00, DEF_STR( Test ) )

	PORT_START("IN2")  /* check code at 0x000614, 0x0008ac and 0x000e36 */
	PORT_BIT( 0xff, IP_ACTIVE_LOW, IPT_UNKNOWN )
INPUT_PORTS_END

/* Needs further checking */
static INPUT_PORTS_START( wof )
	PORT_INCLUDE( cps1_3players )

	PORT_MODIFY("IN0")
	PORT_SERVICE_NO_TOGGLE( 0x40, IP_ACTIVE_LOW )

	PORT_START("DSWA")      /* (not used, EEPROM) */
	PORT_BIT( 0xff, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("DSWB")      /* (not used, EEPROM) */
	PORT_BIT( 0xff, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("DSWC")
	PORT_DIPNAME( 0x08, 0x08, "Freeze" )
	PORT_DIPSETTING(    0x08, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_BIT( 0xf7, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("IN3")      /* Player 4 - not used */
	PORT_BIT( 0xff, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START( "EEPROMIN" )
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_SPECIAL ) PORT_READ_LINE_DEVICE_MEMBER("eeprom", eeprom_serial_93cxx_device, do_read)

	PORT_START( "EEPROMOUT" )
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_OUTPUT ) PORT_WRITE_LINE_DEVICE_MEMBER("eeprom", eeprom_serial_93cxx_device, di_write)
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_OUTPUT ) PORT_WRITE_LINE_DEVICE_MEMBER("eeprom", eeprom_serial_93cxx_device, clk_write)
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_OUTPUT ) PORT_WRITE_LINE_DEVICE_MEMBER("eeprom", eeprom_serial_93cxx_device, cs_write)
INPUT_PORTS_END

INPUT_PORTS_START( dino )
	PORT_INCLUDE( cps1_3players )

	PORT_MODIFY("IN0")
	PORT_SERVICE_NO_TOGGLE( 0x40, IP_ACTIVE_LOW )

	PORT_START("DSWA")      /* (not used, EEPROM) */
	PORT_BIT( 0xff, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("DSWB")      /* (not used, EEPROM) */
	PORT_BIT( 0xff, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("DSWC")
	PORT_DIPNAME( 0x08, 0x08, "Freeze" )
	PORT_DIPSETTING(    0x08, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_BIT( 0xf7, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("IN3")      /* Player 4 - not used */
	PORT_BIT( 0xff, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START( "EEPROMIN" )
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_SPECIAL ) PORT_READ_LINE_DEVICE_MEMBER("eeprom", eeprom_serial_93cxx_device, do_read)

	PORT_START( "EEPROMOUT" )
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_OUTPUT ) PORT_WRITE_LINE_DEVICE_MEMBER("eeprom", eeprom_serial_93cxx_device, di_write)
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_OUTPUT ) PORT_WRITE_LINE_DEVICE_MEMBER("eeprom", eeprom_serial_93cxx_device, clk_write)
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_OUTPUT ) PORT_WRITE_LINE_DEVICE_MEMBER("eeprom", eeprom_serial_93cxx_device, cs_write)
INPUT_PORTS_END


static INPUT_PORTS_START( dinoh )
	PORT_INCLUDE( dino )

	PORT_MODIFY("DSWA")
	CPS1_COINAGE_2( "SW(A)" )
	PORT_DIPNAME( 0x08, 0x08, "Coin Slots" )                PORT_DIPLOCATION("SW(B):4")
	PORT_DIPSETTING(    0x00, "1" )
	PORT_DIPSETTING(    0x08, "3" )                 // This setting can't be used in two-player mode
	PORT_DIPNAME( 0x10, 0x10, "Play Mode" )                 PORT_DIPLOCATION("SW(B):5")
	PORT_DIPSETTING(    0x00, "2 Players" )
	PORT_DIPSETTING(    0x10, "3 Players" )
	PORT_DIPUNUSED_DIPLOC( 0x20, 0x20, "SW(A):6" )          // This switch is not documented in the manual
	PORT_DIPNAME( 0x40, 0x40, "2 Coins to Start, 1 to Continue" )       PORT_DIPLOCATION("SW(A):7")
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPUNUSED_DIPLOC( 0x80, 0x80, "SW(A):8" )          // This switch is not documented in the manual

	PORT_MODIFY("DSWB")
	PORT_DIPNAME( 0x07, 0x04, "Difficulty Level 1" )            PORT_DIPLOCATION("SW(B):1,2,3")
	PORT_DIPSETTING(    0x07, DEF_STR( Easiest ) )          // "01"
	PORT_DIPSETTING(    0x06, DEF_STR( Easier ) )           // "02"
	PORT_DIPSETTING(    0x05, DEF_STR( Easy ) )         // "03"
	PORT_DIPSETTING(    0x04, DEF_STR( Normal ) )           // "04"
	PORT_DIPSETTING(    0x03, DEF_STR( Medium ) )           // "05"
	PORT_DIPSETTING(    0x02, DEF_STR( Hard ) )         // "06"
	PORT_DIPSETTING(    0x01, DEF_STR( Harder ) )           // "07"
	PORT_DIPSETTING(    0x00, DEF_STR( Hardest ) )          // "08"
	PORT_DIPNAME( 0x18, 0x10, "Difficulty Level 2" )            PORT_DIPLOCATION("SW(B):4,5")
	PORT_DIPSETTING(    0x18, DEF_STR( Easy ) )         // "01"
	PORT_DIPSETTING(    0x10, DEF_STR( Normal ) )           // "02"
	PORT_DIPSETTING(    0x08, DEF_STR( Hard ) )         // "03"
	PORT_DIPSETTING(    0x00, DEF_STR( Hardest ) )          // "04"
	PORT_DIPNAME( 0x60, 0x40, DEF_STR( Bonus_Life ) )           PORT_DIPLOCATION("SW(B):6,7")
	PORT_DIPSETTING(    0x60, "300k and 700k" )
	PORT_DIPSETTING(    0x40, "500k and 1000k" )
	PORT_DIPSETTING(    0x20, "1000k" )
	PORT_DIPSETTING(    0x00, DEF_STR( None ) )
	PORT_DIPUNUSED_DIPLOC( 0x80, 0x80, "SW(B):8" )

	PORT_MODIFY("DSWC")
	PORT_DIPNAME( 0x03, 0x02, DEF_STR( Lives ) )                PORT_DIPLOCATION("SW(C):1,2")
	PORT_DIPSETTING(    0x00, "4" )
	PORT_DIPSETTING(    0x01, "3" )
	PORT_DIPSETTING(    0x02, "2" )
	PORT_DIPSETTING(    0x03, "1" )
	PORT_DIPNAME( 0x04, 0x04, DEF_STR( Free_Play ) )            PORT_DIPLOCATION("SW(C):3")
	PORT_DIPSETTING(    0x04, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x08, "Freeze" )                    PORT_DIPLOCATION("SW(C):4")
	PORT_DIPSETTING(    0x08, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x10, DEF_STR( Flip_Screen ) )          PORT_DIPLOCATION("SW(C):5")
	PORT_DIPSETTING(    0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x00, DEF_STR( Demo_Sounds ) )          PORT_DIPLOCATION("SW(C):6")
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x00, DEF_STR( Allow_Continue ) )           PORT_DIPLOCATION("SW(C):7")
	PORT_DIPSETTING(    0x40, DEF_STR( No ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Yes ) )
	PORT_DIPNAME( 0x80, 0x80, "Game Mode")                  PORT_DIPLOCATION("SW(C):8")
	PORT_DIPSETTING(    0x80, "Game" )
	PORT_DIPSETTING(    0x00, DEF_STR( Test ) )

	PORT_MODIFY("IN1")
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT  ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN  ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_JOYSTICK_UP    ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(1)
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(1)
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_PLAYER(1)
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0100, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x0200, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT  ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x0400, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN  ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x0800, IP_ACTIVE_LOW, IPT_JOYSTICK_UP    ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x1000, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(2)
	PORT_BIT( 0x2000, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(2)
	PORT_BIT( 0x4000, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_PLAYER(2)
	PORT_BIT( 0x8000, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_MODIFY("IN2")      /* Player 3 */
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_PLAYER(3)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY PORT_PLAYER(3)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY PORT_PLAYER(3)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY PORT_PLAYER(3)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(3)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(3)
//  PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_PLAYER(3)
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_COIN3 )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_START3 )
INPUT_PORTS_END

INPUT_PORTS_START( punisher )
	PORT_INCLUDE( cps1_2b )

	PORT_MODIFY("IN0")
	PORT_SERVICE_NO_TOGGLE( 0x40, IP_ACTIVE_LOW )

	PORT_START("DSWA")      /* (not used, EEPROM) */
	PORT_BIT( 0xff, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("DSWB")      /* (not used, EEPROM) */
	PORT_BIT( 0xff, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("DSWC")
	PORT_DIPNAME( 0x08, 0x08, "Freeze" )
	PORT_DIPSETTING(    0x08, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_BIT( 0xf7, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("IN2")      /* Player 3 - not used */
	PORT_BIT( 0xff, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("IN3")      /* Player 4 - not used */
	PORT_BIT( 0xff, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START( "EEPROMIN" )
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_SPECIAL ) PORT_READ_LINE_DEVICE_MEMBER("eeprom", eeprom_serial_93cxx_device, do_read)

	PORT_START( "EEPROMOUT" )
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_OUTPUT ) PORT_WRITE_LINE_DEVICE_MEMBER("eeprom", eeprom_serial_93cxx_device, di_write)
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_OUTPUT ) PORT_WRITE_LINE_DEVICE_MEMBER("eeprom", eeprom_serial_93cxx_device, clk_write)
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_OUTPUT ) PORT_WRITE_LINE_DEVICE_MEMBER("eeprom", eeprom_serial_93cxx_device, cs_write)
INPUT_PORTS_END


static INPUT_PORTS_START( punisherbz )
	PORT_INCLUDE( punisher )

	PORT_MODIFY("DSWA")
	CPS1_COINAGE_2( "SW(A)" )
	PORT_DIPNAME( 0x08, 0x08, "2 Coins to Start, 1 to Continue" )   PORT_DIPLOCATION("SW(A):4")
	PORT_DIPSETTING(    0x08, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x30, 0x20, DEF_STR( Lives ) )            PORT_DIPLOCATION("SW(A):5,6")
	PORT_DIPSETTING(    0x30, "1" )
	PORT_DIPSETTING(    0x20, "2" )
	PORT_DIPSETTING(    0x10, "3" )
	PORT_DIPSETTING(    0x00, "4" )
	PORT_DIPNAME( 0x40, 0x40, "Sound" )             PORT_DIPLOCATION("SW(A):7")
	PORT_DIPSETTING(    0x40, "Q Sound" )
	PORT_DIPSETTING(    0x00, "Monaural" )
	PORT_DIPNAME( 0x80, 0x80, DEF_STR( Flip_Screen ) )      PORT_DIPLOCATION("SW(A):8")
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

	PORT_MODIFY("DSWB")
	PORT_DIPNAME( 0x07, 0x04, DEF_STR( Difficulty ) )       PORT_DIPLOCATION("SW(B):1,2,3")
	PORT_DIPSETTING(    0x07, "Extra Easy" )
	PORT_DIPSETTING(    0x06, DEF_STR( Very_Easy) )
	PORT_DIPSETTING(    0x05, DEF_STR( Easy) )
	PORT_DIPSETTING(    0x04, DEF_STR( Normal) )
	PORT_DIPSETTING(    0x03, DEF_STR( Hard) )
	PORT_DIPSETTING(    0x02, DEF_STR( Very_Hard) )
	PORT_DIPSETTING(    0x01, "Extra Hard" )
	PORT_DIPSETTING(    0x00, DEF_STR( Hardest) )
	PORT_DIPNAME( 0x18, 0x10, "Extend" )                PORT_DIPLOCATION("SW(B):4,5")
	PORT_DIPSETTING(    0x18, "800000" )
	PORT_DIPSETTING(    0x10, "1800000" )
	PORT_DIPSETTING(    0x08, "2800000" )
	PORT_DIPSETTING(    0x00, "No Extend" )
	PORT_DIPNAME( 0x20, 0x00, DEF_STR( Allow_Continue ) )       PORT_DIPLOCATION("SW(B):6")
	PORT_DIPSETTING(    0x20, DEF_STR( No ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Yes ) )
	PORT_DIPNAME( 0x40, 0x00, DEF_STR( Demo_Sounds ) )      PORT_DIPLOCATION("SW(B):7")
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPUNKNOWN_DIPLOC( 0x80, 0x80, "SW(B):8" )

	PORT_MODIFY("DSWC")
	PORT_BIT( 0xff, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_MODIFY("IN1")
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT  ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN  ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_JOYSTICK_UP    ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(1)
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(1)
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_PLAYER(1)
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0100, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x0200, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT  ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x0400, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN  ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x0800, IP_ACTIVE_LOW, IPT_JOYSTICK_UP    ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x1000, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(2)
	PORT_BIT( 0x2000, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(2)
	PORT_BIT( 0x4000, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_PLAYER(2)
	PORT_BIT( 0x8000, IP_ACTIVE_LOW, IPT_UNKNOWN )
INPUT_PORTS_END

/* Needs further checking */
INPUT_PORTS_START( slammast )
	PORT_INCLUDE( cps1_4players )

	PORT_MODIFY("IN0")
	PORT_SERVICE_NO_TOGGLE( 0x40, IP_ACTIVE_LOW )

	PORT_MODIFY("IN1")
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_PLAYER(1)
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_PLAYER(3)
	PORT_BIT( 0x4000, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_PLAYER(2)
	PORT_BIT( 0x8000, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_PLAYER(4)

	PORT_START("DSWA")      /* (not used, EEPROM) */
	PORT_BIT( 0xff, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("DSWB")      /* (not used, EEPROM) */
	PORT_BIT( 0xff, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("DSWC")
	PORT_DIPNAME( 0x08, 0x08, "Freeze" )
	PORT_DIPSETTING(    0x08, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_BIT( 0xf7, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START( "EEPROMIN" )
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_SPECIAL ) PORT_READ_LINE_DEVICE_MEMBER("eeprom", eeprom_serial_93cxx_device, do_read)

	PORT_START( "EEPROMOUT" )
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_OUTPUT ) PORT_WRITE_LINE_DEVICE_MEMBER("eeprom", eeprom_serial_93cxx_device, di_write)
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_OUTPUT ) PORT_WRITE_LINE_DEVICE_MEMBER("eeprom", eeprom_serial_93cxx_device, clk_write)
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_OUTPUT ) PORT_WRITE_LINE_DEVICE_MEMBER("eeprom", eeprom_serial_93cxx_device, cs_write)
INPUT_PORTS_END

/* Needs further checking */
static INPUT_PORTS_START( pnickj )
	PORT_INCLUDE( cps1_3b )

	PORT_MODIFY("IN0")
	PORT_SERVICE_NO_TOGGLE( 0x40, IP_ACTIVE_LOW )

	PORT_START("DSWA")
	CPS1_COINAGE_2( "SW(A)" )
	PORT_DIPNAME( 0x08, 0x08, "Coin Slots" )                PORT_DIPLOCATION("SW(A):4")
	PORT_DIPSETTING(    0x08, "1" )
	PORT_DIPSETTING(    0x00, "2" )
	PORT_DIPUNKNOWN_DIPLOC( 0x10, 0x10, "SW(A):5" )
	PORT_DIPUNKNOWN_DIPLOC( 0x20, 0x20, "SW(A):6" )
	PORT_DIPUNKNOWN_DIPLOC( 0x40, 0x40, "SW(A):7" )
	PORT_DIPUNKNOWN_DIPLOC( 0x80, 0x80, "SW(A):8" )

	PORT_START("DSWB")
	CPS1_DIFFICULTY_1( "SW(B)" )
	PORT_DIPUNKNOWN_DIPLOC( 0x08, 0x08, "SW(B):4" )
	PORT_DIPUNKNOWN_DIPLOC( 0x10, 0x10, "SW(B):5" )
	PORT_DIPUNKNOWN_DIPLOC( 0x20, 0x20, "SW(B):6" )
	PORT_DIPNAME( 0xc0, 0xc0, "Vs Play Mode" )              PORT_DIPLOCATION("SW(B):7,8")
	PORT_DIPSETTING(    0xc0, "1 Game Match" )
	PORT_DIPSETTING(    0x80, "3 Games Match" )
	PORT_DIPSETTING(    0x40, "5 Games Match" )
	PORT_DIPSETTING(    0x00, "7 Games Match" )

	PORT_START("DSWC")
	PORT_DIPUNKNOWN_DIPLOC( 0x01, 0x01, "SW(C):1" )
	PORT_DIPUNKNOWN_DIPLOC( 0x02, 0x02, "SW(C):2" )
	PORT_DIPUNKNOWN_DIPLOC( 0x04, 0x04, "SW(C):3" )
	PORT_DIPNAME( 0x08, 0x08, "Freeze" )                    PORT_DIPLOCATION("SW(C):4")
	PORT_DIPSETTING(    0x08, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x10, DEF_STR( Flip_Screen ) )      PORT_DIPLOCATION("SW(C):5")
	PORT_DIPSETTING(    0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x00, DEF_STR( Demo_Sounds ) )      PORT_DIPLOCATION("SW(C):6")
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPUNKNOWN_DIPLOC( 0x40, 0x40, "SW(C):7" )
	PORT_DIPNAME( 0x80, 0x80, "Game Mode")                  PORT_DIPLOCATION("SW(C):8")
	PORT_DIPSETTING(    0x80, "Game" )
	PORT_DIPSETTING(    0x00, DEF_STR( Test ) )
INPUT_PORTS_END

/* Needs further checking */
static INPUT_PORTS_START( qad )
	PORT_INCLUDE( cps1_quiz )

	PORT_MODIFY("IN0")
	PORT_SERVICE_NO_TOGGLE( 0x40, IP_ACTIVE_LOW )

	PORT_START("DSWA")
	CPS1_COINAGE_2( "SW(A)" )
	PORT_DIPUNKNOWN_DIPLOC( 0x08, 0x08, "SW(A):4" )                 // Manual says these are for coin 2, but they
	PORT_DIPUNKNOWN_DIPLOC( 0x10, 0x10, "SW(A):5" )                 // coin to setting, but they clearly don't do
	PORT_DIPUNKNOWN_DIPLOC( 0x20, 0x20, "SW(A):6" )                 // that.
	PORT_DIPNAME( 0x40, 0x40, "2 Coins to Start, 1 to Continue" )   PORT_DIPLOCATION("SW(A):7")
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPUNUSED_DIPLOC( 0x80, 0x80, "SW(A):8" )

	PORT_START("DSWB")
	PORT_DIPNAME( 0x07, 0x04, DEF_STR( Difficulty ) )               PORT_DIPLOCATION("SW(B):1,2,3")
//  PORT_DIPSETTING(    0x07, DEF_STR( Easiest ) )                  // Controls overall difficulty
	PORT_DIPSETTING(    0x06, DEF_STR( Easiest ) )                  // Manual documents duplicate settings
	PORT_DIPSETTING(    0x05, DEF_STR( Easy ) )
	PORT_DIPSETTING(    0x04, DEF_STR( Normal ) )
	PORT_DIPSETTING(    0x03, DEF_STR( Hard ) )
	PORT_DIPSETTING(    0x02, DEF_STR( Hardest ) )
//  PORT_DIPSETTING(    0x01, DEF_STR( Hardest ) )
//  PORT_DIPSETTING(    0x00, DEF_STR( Hardest ) )
	PORT_DIPNAME( 0x18, 0x10, "Wisdom (questions to win game)" )    PORT_DIPLOCATION("SW(B):4,5")
	PORT_DIPSETTING(    0x18, DEF_STR( Easy ) )                     // Controls number of needed questions to finish
	PORT_DIPSETTING(    0x10, DEF_STR( Normal ) )
	PORT_DIPSETTING(    0x08, DEF_STR( Hard ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Hardest ) )
	PORT_DIPNAME( 0xe0, 0xe0, DEF_STR( Lives ) )                    PORT_DIPLOCATION("SW(B):6,7,8")
	PORT_DIPSETTING(    0x60, "1" )
	PORT_DIPSETTING(    0x80, "2" )
	PORT_DIPSETTING(    0xa0, "3" )
	PORT_DIPSETTING(    0xc0, "4" )
	PORT_DIPSETTING(    0xe0, "5" )
//  PORT_DIPSETTING(    0x40, "1" )                                 // These three settings are not documented
//  PORT_DIPSETTING(    0x20, "1" )
//  PORT_DIPSETTING(    0x00, "1" )

	PORT_START("DSWC")
	PORT_DIPUNUSED_DIPLOC( 0x01, 0x01, "SW(C):1" )
	PORT_DIPUNUSED_DIPLOC( 0x02, 0x02, "SW(C):2" )
	PORT_DIPNAME( 0x04, 0x04, DEF_STR( Free_Play ) )                PORT_DIPLOCATION("SW(C):3")
	PORT_DIPSETTING(    0x04, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x08, "Freeze" )                            PORT_DIPLOCATION("SW(C):4")
	PORT_DIPSETTING(    0x08, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x10, DEF_STR( Flip_Screen ) )              PORT_DIPLOCATION("SW(C):5")
	PORT_DIPSETTING(    0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x20, DEF_STR( Demo_Sounds ) )              PORT_DIPLOCATION("SW(C):6")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x20, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x40, DEF_STR( Allow_Continue ) )           PORT_DIPLOCATION("SW(C):7")
	PORT_DIPSETTING(    0x00, DEF_STR( No ) )
	PORT_DIPSETTING(    0x40, DEF_STR( Yes ) )
	PORT_DIPNAME( 0x80, 0x80, "Game Mode")                          PORT_DIPLOCATION("SW(C):8")
	PORT_DIPSETTING(    0x80, "Game" )
	PORT_DIPSETTING(    0x00, DEF_STR( Test ) )

	PORT_START("IN2")  /* check code at 0x01d2d2 */
	PORT_BIT( 0xff, IP_ACTIVE_LOW, IPT_UNKNOWN )
INPUT_PORTS_END

/* Needs further checking */
static INPUT_PORTS_START( qadjr )
	PORT_INCLUDE( qad )

	PORT_MODIFY("DSWB")
	PORT_DIPNAME( 0x07, 0x07, DEF_STR( Difficulty ) )               PORT_DIPLOCATION("SW(B):1,2,3")
	PORT_DIPSETTING(    0x07, "0" )
	PORT_DIPSETTING(    0x06, "1" )
	PORT_DIPSETTING(    0x05, "2" )
	PORT_DIPSETTING(    0x04, "3" )
	PORT_DIPSETTING(    0x03, "4" )
//  PORT_DIPSETTING(    0x02, "4" )
//  PORT_DIPSETTING(    0x01, "4" )
//  PORT_DIPSETTING(    0x00, "4" )
	PORT_DIPUNKNOWN_DIPLOC( 0x08, 0x08, "SW(B):4" )
	PORT_DIPUNKNOWN_DIPLOC( 0x10, 0x10, "SW(B):5" )
	PORT_DIPNAME( 0xe0, 0xe0, DEF_STR( Lives ) )                    PORT_DIPLOCATION("SW(B):6,7,8")
	PORT_DIPSETTING(    0xa0, "1" )
	PORT_DIPSETTING(    0xc0, "2" )
	PORT_DIPSETTING(    0xe0, "3" )
//  PORT_DIPSETTING(    0x00, "1" )
//  PORT_DIPSETTING(    0x20, "1" )
//  PORT_DIPSETTING(    0x80, "1" )
//  PORT_DIPSETTING(    0x40, "2" )
//  PORT_DIPSETTING(    0x60, "3" )

	PORT_MODIFY("IN2")  /* check code at 0x000c48 */
	PORT_BIT( 0xff, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("IN3")  /* check code at 0x000c3e */
	PORT_BIT( 0xff, IP_ACTIVE_LOW, IPT_UNKNOWN )
INPUT_PORTS_END

/* Needs further checking */
static INPUT_PORTS_START( qtono2j )
	PORT_INCLUDE( cps1_quiz )

	PORT_MODIFY("IN0")
	PORT_SERVICE_NO_TOGGLE( 0x40, IP_ACTIVE_LOW )

	PORT_START("DSWA")
	CPS1_COINAGE_2( "SW(A)" )
	PORT_DIPUNKNOWN_DIPLOC( 0x08, 0x08, "SW(A):4" )
	PORT_DIPUNKNOWN_DIPLOC( 0x10, 0x10, "SW(A):5" )
	PORT_DIPUNKNOWN_DIPLOC( 0x20, 0x20, "SW(A):6" )
	PORT_DIPNAME( 0x40, 0x40, "2 Coins to Start, 1 to Continue" )   PORT_DIPLOCATION("SW(A):7")
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPUNKNOWN_DIPLOC( 0x80, 0x80, "SW(A):8" )

	PORT_START("DSWB")
	CPS1_DIFFICULTY_1( "SW(B)" )
	PORT_DIPUNKNOWN_DIPLOC( 0x08, 0x08, "SW(B):4" )
	PORT_DIPUNKNOWN_DIPLOC( 0x10, 0x10, "SW(B):5" )
	PORT_DIPNAME( 0xe0, 0xe0, DEF_STR( Lives ) )                    PORT_DIPLOCATION("SW(B):6,7,8")
	PORT_DIPSETTING(    0x60, "1" )
	PORT_DIPSETTING(    0x80, "2" )
	PORT_DIPSETTING(    0xe0, "3" )
	PORT_DIPSETTING(    0xa0, "4" )
	PORT_DIPSETTING(    0xc0, "5" )
//  PORT_DIPSETTING(    0x40, "?" )
//  PORT_DIPSETTING(    0x20, "?" )
//  PORT_DIPSETTING(    0x00, "?" )

	PORT_START("DSWC")
	PORT_DIPUNKNOWN_DIPLOC( 0x01, 0x01, "SW(C):1" )
	PORT_DIPNAME( 0x02, 0x02, "Infinite Lives (Cheat)")             PORT_DIPLOCATION("SW(C):2")
	PORT_DIPSETTING(    0x02, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x04, 0x04, DEF_STR( Free_Play ) )                PORT_DIPLOCATION("SW(C):3")
	PORT_DIPSETTING(    0x04, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x08, "Freeze" )                            PORT_DIPLOCATION("SW(C):4")
	PORT_DIPSETTING(    0x08, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x10, DEF_STR( Flip_Screen ) )              PORT_DIPLOCATION("SW(C):5")
	PORT_DIPSETTING(    0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x00, DEF_STR( Demo_Sounds ) )              PORT_DIPLOCATION("SW(C):6")
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x40, DEF_STR( Allow_Continue ) )           PORT_DIPLOCATION("SW(C):7")
	PORT_DIPSETTING(    0x00, DEF_STR( No ) )
	PORT_DIPSETTING(    0x40, DEF_STR( Yes ) )
	PORT_DIPNAME( 0x80, 0x80, "Game Mode")                          PORT_DIPLOCATION("SW(C):8")
	PORT_DIPSETTING(    0x80, "Game" )
	PORT_DIPSETTING(    0x00, DEF_STR( Test ) )

	PORT_START("IN2")  /* check code at 0x000f80 */
	PORT_BIT( 0xff, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("IN3")  /* check code at 0x000f76 */
	PORT_BIT( 0xff, IP_ACTIVE_LOW, IPT_UNKNOWN )
INPUT_PORTS_END

/* Needs further checking */
static INPUT_PORTS_START( pang3 )
	PORT_INCLUDE( cps1_3b )

	PORT_MODIFY("IN0")
	PORT_SERVICE_NO_TOGGLE( 0x40, IP_ACTIVE_LOW )

	PORT_START("DSWA")      /* (not used, EEPROM) */
	PORT_BIT( 0xff, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("DSWB")      /* (not used, EEPROM) */
	PORT_BIT( 0xff, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("DSWC")
	PORT_DIPUNUSED( 0x01, 0x01 )
	PORT_DIPUNUSED( 0x02, 0x02 )
	PORT_DIPUNUSED( 0x04, 0x04 )
	PORT_DIPUNUSED( 0x08, 0x08 )
	PORT_DIPUNUSED( 0x10, 0x10 )
	PORT_DIPUNUSED( 0x20, 0x20 )
	PORT_DIPUNUSED( 0x40, 0x40 )
	PORT_DIPUNUSED( 0x80, 0x80 ) /* doubles up as an extra service switch */

	PORT_START( "EEPROMIN" )
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_SPECIAL ) PORT_READ_LINE_DEVICE_MEMBER("eeprom", eeprom_serial_93cxx_device, do_read)

	PORT_START( "EEPROMOUT" )
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_OUTPUT ) PORT_WRITE_LINE_DEVICE_MEMBER("eeprom", eeprom_serial_93cxx_device, di_write)
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_OUTPUT ) PORT_WRITE_LINE_DEVICE_MEMBER("eeprom", eeprom_serial_93cxx_device, clk_write)
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_OUTPUT ) PORT_WRITE_LINE_DEVICE_MEMBER("eeprom", eeprom_serial_93cxx_device, cs_write)
INPUT_PORTS_END

/* Needs further checking */
static INPUT_PORTS_START( pang3b )
	PORT_INCLUDE( pang3 )

	PORT_MODIFY("DSWC")
	PORT_DIPUNUSED( 0x01, 0x01 )
	PORT_DIPUNUSED( 0x02, 0x02 )
	PORT_DIPUNUSED( 0x04, 0x04 )
	PORT_DIPNAME( 0x08, 0x08, "Freeze" )
	PORT_DIPSETTING(    0x08, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPUNUSED( 0x10, 0x10 )
	PORT_DIPUNUSED( 0x20, 0x20 )
	PORT_DIPUNUSED( 0x40, 0x40 )
	PORT_DIPUNUSED( 0x80, 0x80 )
INPUT_PORTS_END

/* Needs further checking */
static INPUT_PORTS_START( megaman )
	PORT_INCLUDE( cps1_3b )

	PORT_MODIFY("IN0")
	PORT_SERVICE_NO_TOGGLE( 0x40, IP_ACTIVE_LOW )

	PORT_START("DSWA")
	PORT_DIPNAME( 0x1f, 0x1f, DEF_STR( Coinage ) )                  PORT_DIPLOCATION("SW(A):1,2,3,4,5")
	PORT_DIPSETTING(    0x0f, DEF_STR( 9C_1C ) )
	PORT_DIPSETTING(    0x10, DEF_STR( 8C_1C ) )
	PORT_DIPSETTING(    0x11, DEF_STR( 7C_1C ) )
	PORT_DIPSETTING(    0x12, DEF_STR( 6C_1C ) )
	PORT_DIPSETTING(    0x13, DEF_STR( 5C_1C ) )
	PORT_DIPSETTING(    0x14, DEF_STR( 4C_1C ) )
	PORT_DIPSETTING(    0x15, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(    0x16, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x0e, "2 Coins to Start, 1 to Continue" )
	PORT_DIPSETTING(    0x1f, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x1e, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0x1d, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(    0x1c, DEF_STR( 1C_4C ) )
	PORT_DIPSETTING(    0x1b, DEF_STR( 1C_5C ) )
	PORT_DIPSETTING(    0x1a, DEF_STR( 1C_6C ) )
	PORT_DIPSETTING(    0x19, DEF_STR( 1C_7C ) )
	PORT_DIPSETTING(    0x18, DEF_STR( 1C_8C ) )
	PORT_DIPSETTING(    0x17, DEF_STR( 1C_9C ) )
	PORT_DIPSETTING(    0x0d, DEF_STR( Free_Play ) )
	/* 0x00 to 0x0c 1 Coin/1 Credit */
	PORT_DIPNAME( 0x60, 0x60, "Coin slots" )                        PORT_DIPLOCATION("SW(A):6,7")
//  PORT_DIPSETTING(    0x00, "Invalid" )
	PORT_DIPSETTING(    0x20, "1, Common" )
	PORT_DIPSETTING(    0x60, "2, Common" )
	PORT_DIPSETTING(    0x40, "2, Individual" )
	PORT_DIPUNKNOWN_DIPLOC( 0x80, 0x80, "SW(A):8" )

	PORT_START("DSWB")
	PORT_DIPNAME( 0x03, 0x02, DEF_STR( Difficulty ) )               PORT_DIPLOCATION("SW(B):1,2")
	PORT_DIPSETTING(    0x03, DEF_STR( Easy ) )
	PORT_DIPSETTING(    0x02, DEF_STR( Normal ) )
	PORT_DIPSETTING(    0x01, DEF_STR( Hard ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Hardest ) )
	PORT_DIPNAME( 0x0c, 0x0c, "Time" )                              PORT_DIPLOCATION("SW(B):3,4")
	PORT_DIPSETTING(    0x0c, "100" )
	PORT_DIPSETTING(    0x08, "90" )
	PORT_DIPSETTING(    0x04, "70" )
	PORT_DIPSETTING(    0x00, "60" )
	PORT_DIPUNKNOWN_DIPLOC( 0x10, 0x10, "SW(B):5" )
	PORT_DIPUNKNOWN_DIPLOC( 0x20, 0x20, "SW(B):6" )
	PORT_DIPNAME( 0x40, 0x40, "Voice" )                             PORT_DIPLOCATION("SW(B):7")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x40, DEF_STR( On ) )
	PORT_DIPUNKNOWN_DIPLOC( 0x80, 0x80, "SW(B):8" )

	PORT_START("DSWC")
	PORT_DIPNAME( 0x01, 0x01, DEF_STR( Flip_Screen ) )              PORT_DIPLOCATION("SW(C):1")
	PORT_DIPSETTING(    0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x02, DEF_STR( Demo_Sounds ) )              PORT_DIPLOCATION("SW(C):2")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x02, DEF_STR( On ) )
	PORT_DIPNAME( 0x04, 0x04, DEF_STR( Allow_Continue ) )           PORT_DIPLOCATION("SW(C):3")
	PORT_DIPSETTING(    0x00, DEF_STR( No ) )
	PORT_DIPSETTING(    0x04, DEF_STR( Yes ) )
	PORT_DIPUNKNOWN_DIPLOC( 0x08, 0x08, "SW(C):4" )
	PORT_DIPUNKNOWN_DIPLOC( 0x10, 0x10, "SW(C):5" )
	PORT_DIPUNKNOWN_DIPLOC( 0x20, 0x20, "SW(C):6" )
	PORT_DIPUNKNOWN_DIPLOC( 0x40, 0x40, "SW(C):7" )
	PORT_DIPNAME( 0x80, 0x80, "Game Mode")                          PORT_DIPLOCATION("SW(C):8")
	PORT_DIPSETTING(    0x80, "Game" )
	PORT_DIPSETTING(    0x00, DEF_STR( Test ) )
INPUT_PORTS_END

/* Needs further checking */
/* Same as 'megaman' but no "Voice" Dip Switch */
static INPUT_PORTS_START( rockmanj )
	PORT_INCLUDE(megaman)

	PORT_MODIFY("DSWB")
	PORT_DIPUNKNOWN_DIPLOC( 0x40, 0x40, "SW(B):7" )
INPUT_PORTS_END

static INPUT_PORTS_START( wofhfh )
	PORT_INCLUDE( wof )

	PORT_MODIFY("DSWA")
	PORT_DIPNAME( 0x03, 0x03, DEF_STR( Coin_A ) )           PORT_DIPLOCATION("SW(A):1,2")
	PORT_DIPSETTING(    0x03, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x02, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0x01, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(    0x00, DEF_STR( 1C_4C ) )
	PORT_BIT( 0xfc, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_MODIFY("DSWB")
	PORT_DIPNAME( 0x07, 0x04, DEF_STR( Difficulty ) )       PORT_DIPLOCATION("SW(B):1,2,3")
	PORT_DIPSETTING(    0x07, "Extra Easy" )
	PORT_DIPSETTING(    0x06, DEF_STR( Very_Easy) )
	PORT_DIPSETTING(    0x05, DEF_STR( Easy) )
	PORT_DIPSETTING(    0x04, DEF_STR( Normal) )
	PORT_DIPSETTING(    0x03, DEF_STR( Hard) )
	PORT_DIPSETTING(    0x02, DEF_STR( Very_Hard) )
	PORT_DIPSETTING(    0x01, "Extra Hard" )
	PORT_DIPSETTING(    0x00, DEF_STR( Hardest) )
	PORT_DIPNAME( 0x70, 0x60, DEF_STR( Lives ) )            PORT_DIPLOCATION("SW(B):4,5,6")
	PORT_DIPSETTING(    0x00, "Start 4 Continue 5" )
	PORT_DIPSETTING(    0x10, "Start 3 Continue 4" )
	PORT_DIPSETTING(    0x20, "Start 2 Continue 3" )
	PORT_DIPSETTING(    0x30, "Start 1 Continue 2" )
	PORT_DIPSETTING(    0x40, "Start 4 Continue 4" )
	PORT_DIPSETTING(    0x50, "Start 3 Continue 3" )
	PORT_DIPSETTING(    0x60, "Start 2 Continue 2" )
	PORT_DIPSETTING(    0x70, "Start 1 Continue 1" )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_MODIFY("DSWC")
	PORT_DIPNAME( 0x03, 0x03, "Coin Slots" )            PORT_DIPLOCATION("SW(C):1,2")
//  PORT_DIPSETTING(    0x00, "2 Players 1 Shooter" )
	PORT_DIPSETTING(    0x01, "2 Players 1 Shooter" )
	PORT_DIPSETTING(    0x02, "3 Players 1 Shooter" )
	PORT_DIPSETTING(    0x03, "3 Players 3 Shooters" )
	PORT_BIT( 0xfc, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_MODIFY("IN1")
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_PLAYER(1)
	PORT_BIT( 0x4000, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_PLAYER(2)

	PORT_MODIFY("IN2")      /* Player 3 */
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_COIN3 ) PORT_NAME("Coin 3 (P3 Button 3 in-game)")
INPUT_PORTS_END

static INPUT_PORTS_START( ganbare )
	PORT_START("IN0")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_COIN2 ) /* definitely read here in test mode, coin lock prevents it though */
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_SERVICE1 )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_START2 )
	PORT_SERVICE_NO_TOGGLE( 0x40, IP_ACTIVE_LOW )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("IN1")
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(1)
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(1)
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0100, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0200, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0400, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0800, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x1000, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x2000, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x4000, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x8000, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("DSWA")
	PORT_DIPNAME( 0x01, 0x01, "Medal Setup" )                   PORT_DIPLOCATION("SW(A):1")
	PORT_DIPSETTING(    0x01, "1 Medal 1 Credit" )
	PORT_DIPSETTING(    0x00, "Don't use" )
	PORT_DIPNAME( 0x02, 0x02, "Coin Setup" )                    PORT_DIPLOCATION("SW(A):2")
	PORT_DIPSETTING(    0x02, "100 Yen" )
	PORT_DIPSETTING(    0x00, "10 Yen" )
	PORT_DIPNAME( 0x1c, 0x1c, "Change Setup" )                  PORT_DIPLOCATION("SW(A):3,4,5")
	PORT_DIPSETTING(    0x04, "12" )
	PORT_DIPSETTING(    0x00, "11" )
	PORT_DIPSETTING(    0x1c, "10" )
	PORT_DIPSETTING(    0x18, "8" )
	PORT_DIPSETTING(    0x14, "7" )
	PORT_DIPSETTING(    0x10, "6" )
	PORT_DIPSETTING(    0x0c, "5" )
	PORT_DIPSETTING(    0x08, "No change" )
	PORT_DIPNAME( 0x60, 0x60, "10 Yen Setup" )                  PORT_DIPLOCATION("SW(A):6,7")
	PORT_DIPSETTING(    0x60, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x40, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x20, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(    0x00, "Don't use" )
	PORT_DIPNAME( 0x80, 0x80, "Payout Setup" )                  PORT_DIPLOCATION("SW(A):8")
	PORT_DIPSETTING(    0x80, "Credit Mode" )
	PORT_DIPSETTING(    0x00, "Payout Mode" )

	PORT_START("DSWB")
	PORT_DIPNAME( 0x07, 0x07, "Payout Rate Setup" )             PORT_DIPLOCATION("SW(B):1,2,3")
	PORT_DIPSETTING(    0x01, "90%" )
	PORT_DIPSETTING(    0x00, "85%" )
	PORT_DIPSETTING(    0x07, "80%" )
	PORT_DIPSETTING(    0x06, "75%" )
	PORT_DIPSETTING(    0x05, "70%" )
	PORT_DIPSETTING(    0x04, "65%" )
	PORT_DIPSETTING(    0x03, "60%" )
	PORT_DIPSETTING(    0x02, "55%" )
	PORT_DIPUNKNOWN_DIPLOC( 0x08, 0x08, "SW(B):4" )
	PORT_DIPUNKNOWN_DIPLOC( 0x10, 0x10, "SW(B):5" )
	PORT_DIPUNKNOWN_DIPLOC( 0x20, 0x20, "SW(B):6" )
	PORT_DIPUNKNOWN_DIPLOC( 0x40, 0x40, "SW(B):7" )
	PORT_DIPUNKNOWN_DIPLOC( 0x80, 0x80, "SW(B):8" )

	PORT_START("DSWC")
	PORT_DIPNAME( 0x03, 0x03, DEF_STR( Demo_Sounds ) )          PORT_DIPLOCATION("SW(C):1,2")
	PORT_DIPSETTING(    0x03, DEF_STR( On ) )
	PORT_DIPSETTING(    0x02, "Every second sound" )
	PORT_DIPSETTING(    0x01, "Every third sound" )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPUNKNOWN_DIPLOC( 0x04, 0x04, "SW(C):3" )
	PORT_DIPUNKNOWN_DIPLOC( 0x08, 0x08, "SW(C):4" )
	PORT_DIPUNKNOWN_DIPLOC( 0x10, 0x10, "SW(C):5" )
	PORT_DIPUNKNOWN_DIPLOC( 0x20, 0x20, "SW(C):6" )
	PORT_DIPNAME( 0x40, 0x40, "Clear RAM" )                     PORT_DIPLOCATION("SW(C):7")
	PORT_DIPSETTING(    0x40, DEF_STR( No ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Yes ) )
	PORT_DIPNAME( 0x80, 0x80, "Tes Mode Display" )              PORT_DIPLOCATION("SW(C):8")
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
INPUT_PORTS_END


static INPUT_PORTS_START( sfzch )
	PORT_START("IN0")     /* IN0 */
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_BUTTON5) PORT_PLAYER(1)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_BUTTON5) PORT_PLAYER(2)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_SERVICE) PORT_NAME(DEF_STR(Pause)) PORT_CODE(KEYCODE_F1) /* pause */
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_SERVICE  )   /* pause */
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_START1)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_START2)
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_BUTTON6) PORT_PLAYER(1)
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_BUTTON6) PORT_PLAYER(2)

	PORT_START("DSWA")
	PORT_DIPNAME( 0xff, 0xff, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0xff, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

	PORT_START("DSWB")
	PORT_DIPNAME( 0xff, 0xff, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0xff, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

	PORT_START("DSWC")
	PORT_DIPNAME( 0xff, 0xff, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0xff, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

	PORT_START("IN1")     /* Player 1 & 2 */
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT) PORT_PLAYER(1) PORT_8WAY
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT) PORT_PLAYER(1) PORT_8WAY
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN) PORT_PLAYER(1) PORT_8WAY
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_UP) PORT_PLAYER(1) PORT_8WAY
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON1) PORT_PLAYER(1)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON2) PORT_PLAYER(1)
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_BUTTON3) PORT_PLAYER(1)
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_BUTTON4) PORT_PLAYER(1)
	PORT_BIT( 0x0100, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT) PORT_PLAYER(2) PORT_8WAY
	PORT_BIT( 0x0200, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT) PORT_PLAYER(2) PORT_8WAY
	PORT_BIT( 0x0400, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN) PORT_PLAYER(2) PORT_8WAY
	PORT_BIT( 0x0800, IP_ACTIVE_LOW, IPT_JOYSTICK_UP) PORT_PLAYER(2) PORT_8WAY
	PORT_BIT( 0x1000, IP_ACTIVE_LOW, IPT_BUTTON1) PORT_PLAYER(2)
	PORT_BIT( 0x2000, IP_ACTIVE_LOW, IPT_BUTTON2) PORT_PLAYER(2)
	PORT_BIT( 0x4000, IP_ACTIVE_LOW, IPT_BUTTON3) PORT_PLAYER(2)
	PORT_BIT( 0x8000, IP_ACTIVE_LOW, IPT_BUTTON4) PORT_PLAYER(2)

	PORT_START("IN2")      /* Read by wofch */
	PORT_BIT( 0xff, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("IN3")      /* Player 4 - not used */
	PORT_BIT( 0xff, IP_ACTIVE_LOW, IPT_UNUSED )

INPUT_PORTS_END

static INPUT_PORTS_START( wofch )
	PORT_INCLUDE( sfzch )

	PORT_START( "EEPROMIN" )
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_SPECIAL ) PORT_READ_LINE_DEVICE_MEMBER("eeprom", eeprom_serial_93cxx_device, do_read)

	PORT_START( "EEPROMOUT" )
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_OUTPUT ) PORT_WRITE_LINE_DEVICE_MEMBER("eeprom", eeprom_serial_93cxx_device, di_write)
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_OUTPUT ) PORT_WRITE_LINE_DEVICE_MEMBER("eeprom", eeprom_serial_93cxx_device, clk_write)
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_OUTPUT ) PORT_WRITE_LINE_DEVICE_MEMBER("eeprom", eeprom_serial_93cxx_device, cs_write)
INPUT_PORTS_END

/*
A Final Fight board with mismatched USA and Japan GFX proves that the columns
of the 8x8 tilemap alternate between sides of the 16x16 tile resulting
in a corrupt WDUD screen. ffightub is now fixed since a genuine pcb surfaced.
*/

static const gfx_layout cps1_layout8x8 =
{
	8,8,
	RGN_FRAC(1,1),
	4,
	{ 24, 16, 8, 0 },
	{ STEP8(0, 1) },
	{ STEP8(0, 4*16) },
	64*8
};

static const gfx_layout cps1_layout8x8_2 =
{
	8,8,
	RGN_FRAC(1,1),
	4,
	{ 24, 16, 8, 0 },
	{ STEP8(32, 1) },
	{ STEP8(0, 4*16) },
	64*8
};

static const gfx_layout cps1_layout16x16 =
{
	16,16,
	RGN_FRAC(1,1),
	4,
	{ 24, 16, 8, 0 },
	{ STEP8(0, 1), STEP8(32, 1) },
	{ STEP16(0, 4*16) },
	4*16*16
};

static const gfx_layout cps1_layout32x32 =
{
	32,32,
	RGN_FRAC(1,1),
	4,
	{ 24, 16, 8, 0 },
	{ STEP8(0, 1), STEP8(32, 1), STEP8(64, 1), STEP8(96, 1) },
	{ STEP32(0, 4*32) },
	4*32*32
};

GFXDECODE_START( cps1 )
	GFXDECODE_ENTRY( "gfx", 0, cps1_layout8x8,   0, 0x100 )
	GFXDECODE_ENTRY( "gfx", 0, cps1_layout8x8_2, 0, 0x100 )
	GFXDECODE_ENTRY( "gfx", 0, cps1_layout16x16, 0, 0x100 )
	GFXDECODE_ENTRY( "gfx", 0, cps1_layout32x32, 0, 0x100 )
GFXDECODE_END



/********************************************************************
*
*  Machine Driver macro
*  ====================
*
*  Abusing the pre-processor.
*
********************************************************************/

MACHINE_START_MEMBER(cps_state,common)
{
}

MACHINE_START_MEMBER(cps_state,cps1)
{
	MACHINE_START_CALL_MEMBER(common);
	membank("bank1")->configure_entries(0, 2, memregion("audiocpu")->base() + 0x10000, 0x4000);
}

MACHINE_START_MEMBER(cps_state,qsound)
{
	MACHINE_START_CALL_MEMBER(common);
	membank("bank1")->configure_entries(0, 6, memregion("audiocpu")->base() + 0x10000, 0x4000);
}

static MACHINE_CONFIG_START( cps1_10MHz, cps_state )

	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", M68000, XTAL_10MHz )    /* verified on pcb */
	MCFG_CPU_PROGRAM_MAP(main_map)
	MCFG_CPU_VBLANK_INT_DRIVER("screen", cps_state, cps1_interrupt)

	MCFG_CPU_ADD("audiocpu", Z80, XTAL_3_579545MHz)  /* verified on pcb */
	MCFG_CPU_PROGRAM_MAP(sub_map)

	MCFG_MACHINE_START_OVERRIDE(cps_state,cps1)

	/* video hardware */
	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_RAW_PARAMS(CPS_PIXEL_CLOCK, CPS_HTOTAL, CPS_HBEND, CPS_HBSTART, CPS_VTOTAL, CPS_VBEND, CPS_VBSTART)
	MCFG_SCREEN_UPDATE_DRIVER(cps_state, screen_update_cps1)
	MCFG_SCREEN_VBLANK_DRIVER(cps_state, screen_eof_cps1)
	MCFG_SCREEN_PALETTE("palette")

	MCFG_GFXDECODE_ADD("gfxdecode", "palette", cps1)
	MCFG_PALETTE_ADD("palette", 0xc00)

	MCFG_VIDEO_START_OVERRIDE(cps_state, cps1)

	/* sound hardware */
	MCFG_SPEAKER_STANDARD_MONO("mono")

	MCFG_YM2151_ADD("2151", XTAL_3_579545MHz)  /* verified on pcb */
	MCFG_YM2151_IRQ_HANDLER(INPUTLINE("audiocpu", 0))
	MCFG_SOUND_ROUTE(0, "mono", 0.35)
	MCFG_SOUND_ROUTE(1, "mono", 0.35)

	/* CPS PPU is fed by a 16mhz clock,pin 117 outputs a 4mhz clock which is divided by 4 using 2 74ls74 */
	MCFG_OKIM6295_ADD("oki", XTAL_16MHz/4/4, OKIM6295_PIN7_HIGH) // pin 7 can be changed by the game code, see f006 on z80
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.30)
MACHINE_CONFIG_END

MACHINE_CONFIG_DERIVED( cps1_12MHz, cps1_10MHz )

	/* basic machine hardware */
	MCFG_CPU_MODIFY("maincpu")
	MCFG_CPU_CLOCK( XTAL_12MHz )    /* verified on pcb */
MACHINE_CONFIG_END

static MACHINE_CONFIG_DERIVED( pang3, cps1_12MHz )

	/* basic machine hardware */
	MCFG_EEPROM_SERIAL_93C46_ADD("eeprom")
MACHINE_CONFIG_END

static MACHINE_CONFIG_DERIVED( ganbare, cps1_10MHz )

	/* basic machine hardware */
	MCFG_CPU_MODIFY("maincpu")
	MCFG_TIMER_DRIVER_ADD_SCANLINE("scantimer", cps_state, ganbare_interrupt, "screen", 0, 1) // need to investigate more

	MCFG_M48T35_ADD("m48t35")
MACHINE_CONFIG_END

static MACHINE_CONFIG_DERIVED( qsound, cps1_12MHz )

	/* basic machine hardware */
	MCFG_CPU_REPLACE("maincpu", M68000, XTAL_12MHz )    /* verified on pcb */
	MCFG_CPU_PROGRAM_MAP(qsound_main_map)
	MCFG_CPU_VBLANK_INT_DRIVER("screen", cps_state, cps1_interrupt)

	MCFG_CPU_REPLACE("audiocpu", Z80, XTAL_8MHz)  /* verified on pcb */
	MCFG_CPU_PROGRAM_MAP(qsound_sub_map)
	MCFG_CPU_DECRYPTED_OPCODES_MAP(qsound_decrypted_opcodes_map)
	MCFG_CPU_PERIODIC_INT_DRIVER(cps_state, irq0_line_hold, 250) // measured (cps2.c)

	MCFG_MACHINE_START_OVERRIDE(cps_state, qsound)

	MCFG_EEPROM_SERIAL_93C46_8BIT_ADD("eeprom")

	/* sound hardware */
	MCFG_DEVICE_REMOVE("mono")
	MCFG_SPEAKER_STANDARD_STEREO("lspeaker", "rspeaker")

	MCFG_DEVICE_REMOVE("2151")
	MCFG_DEVICE_REMOVE("oki")

	MCFG_QSOUND_ADD("qsound", QSOUND_CLOCK)
	MCFG_SOUND_ROUTE(0, "lspeaker", 1.0)
	MCFG_SOUND_ROUTE(1, "rspeaker", 1.0)
MACHINE_CONFIG_END

static MACHINE_CONFIG_DERIVED( wofhfh, cps1_12MHz )

	/* basic machine hardware */
	MCFG_EEPROM_SERIAL_93C46_8BIT_ADD("eeprom")
MACHINE_CONFIG_END

static MACHINE_CONFIG_DERIVED( sf2m3, cps1_12MHz)
	MCFG_CPU_MODIFY("maincpu")
	MCFG_CPU_PROGRAM_MAP(sf2m3_map)
MACHINE_CONFIG_END

static MACHINE_CONFIG_DERIVED( sf2m10, cps1_12MHz)
	MCFG_CPU_MODIFY("maincpu")
	MCFG_CPU_PROGRAM_MAP(sf2m10_map)
MACHINE_CONFIG_END


/***************************************************************************

  Game driver(s)

***************************************************************************/

#define CODE_SIZE 0x400000

/* B-Board 88621B-2 */
/*
   There are 4 surface mounted ROMs each on it's own small 88621B-sub satellite board, type HN62404FP-18 package is QFP44.
   The ROMs on the satellite boards are named and located as follows:

   LW-02 @ 6B
   LW-08 @ 9B
   LW-06 @ 9D
   LW-07 @ 10G

   OTHER:
   2 PALs labeled LW621 (near LW_1.2A) and LWI0 (near LW_00.13C)
   Custom chip -   CAPCOM CPS-B-01 (QFP160)
   NEC D4701AC
*/
/* Note that ROMs are labeled left to right, top to bottom, instead of top to bottom, left to right as usual. */
ROM_START( forgottn )
	ROM_REGION( CODE_SIZE, "maincpu", 0 )
	ROM_LOAD16_BYTE( "lw40.12f",       0x00000, 0x20000, CRC(73e920b7) SHA1(2df12fc1a66f488d06b0927db909da81466d7d07) ) /* Higher program numbers indicates a later revision */
	ROM_LOAD16_BYTE( "lw41.12h",       0x00001, 0x20000, CRC(58210b9e) SHA1(416cb56a81e74fce6f86c2b2519ba620457b785a) ) /* 1 byte difference: 0x66D4 == 0x0C versus 0x04 in lw15.12h below */
	ROM_LOAD16_BYTE( "lw42.13f",       0x40000, 0x20000, CRC(bea45994) SHA1(c419f65c5e0c11ae7508ec54412bf6b62fac4f72) )
	ROM_LOAD16_BYTE( "lw43.13h",       0x40001, 0x20000, CRC(539b2339) SHA1(8a9e452ef8ed05e0b956d36990266657d3077470) )
	ROM_LOAD16_WORD_SWAP( "lw-07.10g", 0x80000, 0x80000, CRC(fd252a26) SHA1(5cfb097984912a5167a8c7ec4c2e119b642f9970) ) // == lw-07.13e

	ROM_REGION( 0x400000, "gfx", 0 )
	ROMX_LOAD( "lw_2.2b",   0x000000, 0x20000, CRC(4bd75fee) SHA1(c27bfba951a0dc4f493937ceca335c50a1afeddf) , ROM_SKIP(7) ) // == lw-01.9d
	ROMX_LOAD( "lw_1.2a",   0x000001, 0x20000, CRC(65f41485) SHA1(fb05dffc87ee2f2b1b6646d54b13671f8eee0429) , ROM_SKIP(7) ) // == lw-01.9d
	ROMX_LOAD( "lw-08.9b",  0x000002, 0x80000, CRC(25a8e43c) SHA1(d57cee1fc508db2677e84882fb814e4d9ad20543) , ROM_GROUPWORD | ROM_SKIP(6) ) // == lw-08.9f
	ROMX_LOAD( "lw_18.5e",  0x000004, 0x20000, CRC(b4b6241b) SHA1(92b6b530e18ce27ba8739ebba0d8096b1551026c) , ROM_SKIP(7) )
	ROMX_LOAD( "lw_17.5c",  0x000005, 0x20000, CRC(c5eea115) SHA1(22fe692eaf9dd00a56a76f46c19fb76bb5e5f0d6) , ROM_SKIP(7) )
	ROMX_LOAD( "lw_30.8h",  0x000006, 0x20000, CRC(b385954e) SHA1(d33adb5842e7b85d304836bd92a7a96be4ff3694) , ROM_SKIP(7) ) // == lw-12.9g
	ROMX_LOAD( "lw_29.8f",  0x000007, 0x20000, CRC(7bda1ac6) SHA1(5b8bd05f52798f98ae16efa2ff61c06e28a4e3a0) , ROM_SKIP(7) ) // == lw-12.9g
	ROMX_LOAD( "lw_4.3b",   0x100000, 0x20000, CRC(50cf757f) SHA1(c70d7d34ac2d6671d40dd372e241ccb60bf3bf2b) , ROM_SKIP(7) ) // == lw-01.9d
	ROMX_LOAD( "lw_3.3a",   0x100001, 0x20000, CRC(c03ef278) SHA1(ad33b01bd8194025a2ecf7755894d6d638da457a) , ROM_SKIP(7) ) // == lw-01.9d
	ROMX_LOAD( "lw_20.7e",  0x100004, 0x20000, CRC(df1a3665) SHA1(7ba9c0edc64d4f9a8563533ce723a0a748352a15) , ROM_SKIP(7) )
	ROMX_LOAD( "lw_19.7c",  0x100005, 0x20000, CRC(15af8440) SHA1(5afd0e833593f1a78487af489fe4384ab68f52b1) , ROM_SKIP(7) )
	ROMX_LOAD( "lw_32.9h",  0x100006, 0x20000, CRC(30967a15) SHA1(6f6c6ca2f40aa9beec63ed64f0571bebc7c1aa50) , ROM_SKIP(7) ) // == lw-12.9g
	ROMX_LOAD( "lw_31.9f",  0x100007, 0x20000, CRC(c49d37fb) SHA1(ce400261a0f8d5a9b95d3823f8f52de87b8007f1) , ROM_SKIP(7) ) // == lw-12.9g
	ROMX_LOAD( "lw-02.6b",  0x200000, 0x80000, CRC(43e6c5c8) SHA1(d3e6c971de0477ec4e178adc82508208dd8b397f) , ROM_GROUPWORD | ROM_SKIP(6) ) // == lw-02.12d
	ROMX_LOAD( "lw_14.10b", 0x200002, 0x20000, CRC(82862cce) SHA1(727ca4ee55e076185b071a49afc87533fde9ec27) , ROM_SKIP(7) ) // == lw-09.12f
	ROMX_LOAD( "lw_13.10a", 0x200003, 0x20000, CRC(b81c0e96) SHA1(09f4235786b8ff92a57112669c0385b64477eb01) , ROM_SKIP(7) ) // == lw-09.12f
	ROMX_LOAD( "lw-06.9d",  0x200004, 0x80000, CRC(5b9edffc) SHA1(6fd8f4a3ab070733b52365ab1945bf86acb2bf62) , ROM_GROUPWORD | ROM_SKIP(6) ) // == lw-06.12e
	ROMX_LOAD( "lw_26.10e", 0x200006, 0x20000, CRC(57bcd032) SHA1(6db0f96fb909ed02fe4b7ee25fe662ea23f884d2) , ROM_SKIP(7) ) // == lw-13.12g
	ROMX_LOAD( "lw_25.10c", 0x200007, 0x20000, CRC(bac91554) SHA1(52f5de144193e0f78b9824cc8fd6f934dc19bab0) , ROM_SKIP(7) ) // == lw-13.12g
	ROMX_LOAD( "lw_16.11b", 0x300002, 0x20000, CRC(40b26554) SHA1(b4b27573d6c329bc2bc4c64fd857475bf2a10877) , ROM_SKIP(7) ) // == lw-09.12f
	ROMX_LOAD( "lw_15.11a", 0x300003, 0x20000, CRC(1b7d2e07) SHA1(0edf4d4b314fd9c29e7915d5d1adef6f9617f921) , ROM_SKIP(7) ) // == lw-09.12f
	ROMX_LOAD( "lw_28.11e", 0x300006, 0x20000, CRC(a805ad30) SHA1(baded4ab5fe4e87d53233b5df88edc693c292fc4) , ROM_SKIP(7) ) // == lw-13.12g
	ROMX_LOAD( "lw_27.11c", 0x300007, 0x20000, CRC(103c1bd2) SHA1(fc7ce74e108c30554139e55651c5348b11e9e3bd) , ROM_SKIP(7) ) // == lw-13.12g

	ROM_REGION( 0x8000, "stars", 0 )
	ROM_COPY( "gfx", 0x200000, 0x000000, 0x8000 )

	ROM_REGION( 0x18000, "audiocpu", 0 )
	ROM_LOAD( "lw_37.13c",  0x00000, 0x08000, CRC(59df2a63) SHA1(dfe1fffc7a17179a80a2ae623e93b30a7d6df20d) )    // == lw_00b.13c
	ROM_CONTINUE(           0x10000, 0x08000 )

	ROM_REGION( 0x40000, "oki", 0 ) /* Samples */
	ROM_LOAD( "lw-03u.12e", 0x00000, 0x20000, CRC(807d051f) SHA1(720e4733787b9b11f4d1cdce0892b69475802844) )    // == lw-03u.14c
	ROM_LOAD( "lw-04u.13e", 0x20000, 0x20000, CRC(e6cd098e) SHA1(667f6e5736f76a1c4c450c4e2035574ea89d7910) )    // == lw-04u.13c

	ROM_REGION( 0x0200, "aboardplds", 0 )
	ROM_LOAD( "buf1",         0x0000, 0x0117, CRC(eb122de7) SHA1(b26b5bfe258e3e184f069719f9fd008d6b8f6b9b) )
	ROM_LOAD( "ioa1",         0x0000, 0x0117, CRC(59c7ee3b) SHA1(fbb887c5b4f5cb8df77cec710eaac2985bc482a6) )
	ROM_LOAD( "prg1",         0x0000, 0x0117, CRC(f1129744) SHA1(a5300f301c1a08a7da768f0773fa0fe3f683b237) )
	ROM_LOAD( "rom1",         0x0000, 0x0117, CRC(41dc73b9) SHA1(7d4c9f1693c821fbf84e32dd6ef62ddf14967845) )
	ROM_LOAD( "sou1",         0x0000, 0x0117, CRC(84f4b2fe) SHA1(dcc9e86cc36316fe42eace02d6df75d08bc8bb6d) )

	ROM_REGION( 0x0200, "bboardplds", 0 )
	ROM_LOAD( "lw621.1a",     0x0000, 0x0117, CRC(5eec6ce9) SHA1(5ec8b60f1f1bdba865b1fa2387987ce99ff4093a) )
	ROM_LOAD( "lwio.12b",     0x0000, 0x0117, CRC(ad52b90c) SHA1(f0fd6aeea515ee449320fe15684e6b3ab7f97bf4) )
ROM_END

/* B-Board 88621B-2 */
/*
   These ROMs read from a dead and very unique top board. All EPROMs are type 27C1000, except LW_00.13C which is a 27C512.
   There are 5 surface mounted ROMs each on it's own small 88621B-sub satellite board, type HN62404FP-18 package is QFP44.
   The ROMs on the satellite boards are named and located as follows:

   LW-02 @ 6B
   LW-05 @ 6D (instead of LW_17.5C, LW_18.5E, LW_19.7C & LW_20.7E as above)
   LW-08 @ 9B
   LW-06 @ 9D
   LW-07 @ 10G

   Also known to have LW-13 @ 10D instead of LW_25.10C, LW_26.10E, LW_27.11C & LW_28.11E

   OTHER:
   2 PALs labeled LW621 (near LW_1.2A) and LWI0 (near LW_00.13C)
   Custom chip -   CAPCOM CPS-B-01 (QFP160)
   NEC D4701AC
*/
/* Note that ROMs are labeled left to right, top to bottom, instead of top to bottom, left to right as usual. */
ROM_START( forgottna )
	ROM_REGION( CODE_SIZE, "maincpu", 0 )
	ROM_LOAD16_BYTE( "lw11.12f",       0x00000, 0x20000, CRC(73e920b7) SHA1(2df12fc1a66f488d06b0927db909da81466d7d07) )
	ROM_LOAD16_BYTE( "lw15.12h",       0x00001, 0x20000, CRC(50d7012d) SHA1(f82a28a835f0a83b26c2c8170b824447b1d7409f) )
	ROM_LOAD16_BYTE( "lw10.13f",       0x40000, 0x20000, CRC(bea45994) SHA1(c419f65c5e0c11ae7508ec54412bf6b62fac4f72) )
	ROM_LOAD16_BYTE( "lw14.13h",       0x40001, 0x20000, CRC(539b2339) SHA1(8a9e452ef8ed05e0b956d36990266657d3077470) )
	ROM_LOAD16_WORD_SWAP( "lw-07.10g", 0x80000, 0x80000, CRC(fd252a26) SHA1(5cfb097984912a5167a8c7ec4c2e119b642f9970) ) // == lw-07.13e

	ROM_REGION( 0x400000, "gfx", 0 )
	ROMX_LOAD( "lw_2.2b",   0x000000, 0x20000, CRC(4bd75fee) SHA1(c27bfba951a0dc4f493937ceca335c50a1afeddf) , ROM_SKIP(7) ) // == lw-01.9d
	ROMX_LOAD( "lw_1.2a",   0x000001, 0x20000, CRC(65f41485) SHA1(fb05dffc87ee2f2b1b6646d54b13671f8eee0429) , ROM_SKIP(7) ) // == lw-01.9d
	ROMX_LOAD( "lw-08.9b",  0x000002, 0x80000, CRC(25a8e43c) SHA1(d57cee1fc508db2677e84882fb814e4d9ad20543) , ROM_GROUPWORD | ROM_SKIP(6) ) // == lw-08.9f
	ROMX_LOAD( "lw-05.6d",  0x000004, 0x80000, CRC(e4552fd7) SHA1(11147afc475904848458425661473586dd6f60cc) , ROM_GROUPWORD | ROM_SKIP(6) ) // == lw-05.9e
	ROMX_LOAD( "lw_30.8h",  0x000006, 0x20000, CRC(b385954e) SHA1(d33adb5842e7b85d304836bd92a7a96be4ff3694) , ROM_SKIP(7) ) // == lw-12.9g
	ROMX_LOAD( "lw_29.8f",  0x000007, 0x20000, CRC(7bda1ac6) SHA1(5b8bd05f52798f98ae16efa2ff61c06e28a4e3a0) , ROM_SKIP(7) ) // == lw-12.9g
	ROMX_LOAD( "lw_4.3b",   0x100000, 0x20000, CRC(50cf757f) SHA1(c70d7d34ac2d6671d40dd372e241ccb60bf3bf2b) , ROM_SKIP(7) ) // == lw-01.9d
	ROMX_LOAD( "lw_3.3a",   0x100001, 0x20000, CRC(c03ef278) SHA1(ad33b01bd8194025a2ecf7755894d6d638da457a) , ROM_SKIP(7) ) // == lw-01.9d
	ROMX_LOAD( "lw_32.9h",  0x100006, 0x20000, CRC(30967a15) SHA1(6f6c6ca2f40aa9beec63ed64f0571bebc7c1aa50) , ROM_SKIP(7) ) // == lw-12.9g
	ROMX_LOAD( "lw_31.9f",  0x100007, 0x20000, CRC(c49d37fb) SHA1(ce400261a0f8d5a9b95d3823f8f52de87b8007f1) , ROM_SKIP(7) ) // == lw-12.9g
	ROMX_LOAD( "lw-02.6b",  0x200000, 0x80000, CRC(43e6c5c8) SHA1(d3e6c971de0477ec4e178adc82508208dd8b397f) , ROM_GROUPWORD | ROM_SKIP(6) ) // == lw-02.12d
	ROMX_LOAD( "lw_14.10b", 0x200002, 0x20000, CRC(82862cce) SHA1(727ca4ee55e076185b071a49afc87533fde9ec27) , ROM_SKIP(7) ) // == lw-09.12f
	ROMX_LOAD( "lw_13.10a", 0x200003, 0x20000, CRC(b81c0e96) SHA1(09f4235786b8ff92a57112669c0385b64477eb01) , ROM_SKIP(7) ) // == lw-09.12f
	ROMX_LOAD( "lw-06.9d",  0x200004, 0x80000, CRC(5b9edffc) SHA1(6fd8f4a3ab070733b52365ab1945bf86acb2bf62) , ROM_GROUPWORD | ROM_SKIP(6) ) // == lw-06.12e
	ROMX_LOAD( "lw_26.10e", 0x200006, 0x20000, CRC(57bcd032) SHA1(6db0f96fb909ed02fe4b7ee25fe662ea23f884d2) , ROM_SKIP(7) ) // == lw-13.12g
	ROMX_LOAD( "lw_25.10c", 0x200007, 0x20000, CRC(bac91554) SHA1(52f5de144193e0f78b9824cc8fd6f934dc19bab0) , ROM_SKIP(7) ) // == lw-13.12g
	ROMX_LOAD( "lw_16.11b", 0x300002, 0x20000, CRC(40b26554) SHA1(b4b27573d6c329bc2bc4c64fd857475bf2a10877) , ROM_SKIP(7) ) // == lw-09.12f
	ROMX_LOAD( "lw_15.11a", 0x300003, 0x20000, CRC(1b7d2e07) SHA1(0edf4d4b314fd9c29e7915d5d1adef6f9617f921) , ROM_SKIP(7) ) // == lw-09.12f
	ROMX_LOAD( "lw_28.11e", 0x300006, 0x20000, CRC(a805ad30) SHA1(baded4ab5fe4e87d53233b5df88edc693c292fc4) , ROM_SKIP(7) ) // == lw-13.12g
	ROMX_LOAD( "lw_27.11c", 0x300007, 0x20000, CRC(103c1bd2) SHA1(fc7ce74e108c30554139e55651c5348b11e9e3bd) , ROM_SKIP(7) ) // == lw-13.12g

	ROM_REGION( 0x8000, "stars", 0 )
	ROM_COPY( "gfx", 0x200000, 0x000000, 0x8000 )

	ROM_REGION( 0x18000, "audiocpu", 0 )
	ROM_LOAD( "lw_00.13c",  0x00000, 0x08000, CRC(59df2a63) SHA1(dfe1fffc7a17179a80a2ae623e93b30a7d6df20d) )    // == lw_00b.14a
	ROM_CONTINUE(           0x10000, 0x08000 )

	ROM_REGION( 0x40000, "oki", 0 ) /* Samples */
	ROM_LOAD( "lw-03u.12e", 0x00000, 0x20000, CRC(807d051f) SHA1(720e4733787b9b11f4d1cdce0892b69475802844) )    // == lw-03u.14c
	ROM_LOAD( "lw-04u.13e", 0x20000, 0x20000, CRC(e6cd098e) SHA1(667f6e5736f76a1c4c450c4e2035574ea89d7910) )    // == lw-04u.13c

	ROM_REGION( 0x0200, "aboardplds", 0 )
	ROM_LOAD( "buf1",         0x0000, 0x0117, CRC(eb122de7) SHA1(b26b5bfe258e3e184f069719f9fd008d6b8f6b9b) )
	ROM_LOAD( "ioa1",         0x0000, 0x0117, CRC(59c7ee3b) SHA1(fbb887c5b4f5cb8df77cec710eaac2985bc482a6) )
	ROM_LOAD( "prg1",         0x0000, 0x0117, CRC(f1129744) SHA1(a5300f301c1a08a7da768f0773fa0fe3f683b237) )
	ROM_LOAD( "rom1",         0x0000, 0x0117, CRC(41dc73b9) SHA1(7d4c9f1693c821fbf84e32dd6ef62ddf14967845) )
	ROM_LOAD( "sou1",         0x0000, 0x0117, CRC(84f4b2fe) SHA1(dcc9e86cc36316fe42eace02d6df75d08bc8bb6d) )

	ROM_REGION( 0x0200, "bboardplds", 0 )
	ROM_LOAD( "lw621.1a",     0x0000, 0x0117, CRC(5eec6ce9) SHA1(5ec8b60f1f1bdba865b1fa2387987ce99ff4093a) )
	ROM_LOAD( "lwio.12b",     0x0000, 0x0117, CRC(ad52b90c) SHA1(f0fd6aeea515ee449320fe15684e6b3ab7f97bf4) )
ROM_END

/* B-Board 88621B-2 */
/* This set comes from a USA board with the same pcb layout of the parent forgottn, but uses one more surface mounted ROM
   named LW-13 and located @ 10D instead of LW_25.10C, LW_26.10E, LW_27.11C, LW_28.11E EPROMs. */
/* Note that ROMs are labeled left to right, top to bottom, instead of top to bottom, left to right as usual. */
ROM_START( forgottnu )
	ROM_REGION( CODE_SIZE, "maincpu", 0 )
	ROM_LOAD16_BYTE( "lw11c.12f",      0x00000, 0x20000, CRC(e62742b6) SHA1(39dd3bdd405a8217d8816567d4f2014fc77f5ce8) )
	ROM_LOAD16_BYTE( "lw15c.12h",      0x00001, 0x20000, CRC(1b70f216) SHA1(f200f615dca8aa23d166e74b8baa9a8863ee7a95) )
	ROM_LOAD16_BYTE( "lw10c.13f",      0x40000, 0x20000, CRC(8f5ea3f5) SHA1(c3e43659bd7e03ec3d5c79647db380bde391b0f1) )
	ROM_LOAD16_BYTE( "lw14c.13h",      0x40001, 0x20000, CRC(708e7472) SHA1(44f82db41ffd1eece2d3f3c977f481fd84dfdbf3) )
	ROM_LOAD16_WORD_SWAP( "lw-07.10g", 0x80000, 0x80000, CRC(fd252a26) SHA1(5cfb097984912a5167a8c7ec4c2e119b642f9970) ) // == lw-07.13e

	ROM_REGION( 0x400000, "gfx", 0 )
	ROMX_LOAD( "lw_2.2b",   0x000000, 0x20000, CRC(4bd75fee) SHA1(c27bfba951a0dc4f493937ceca335c50a1afeddf) , ROM_SKIP(7) ) // == lw-01.9d
	ROMX_LOAD( "lw_1.2a",   0x000001, 0x20000, CRC(65f41485) SHA1(fb05dffc87ee2f2b1b6646d54b13671f8eee0429) , ROM_SKIP(7) ) // == lw-01.9d
	ROMX_LOAD( "lw-08.9b",  0x000002, 0x80000, CRC(25a8e43c) SHA1(d57cee1fc508db2677e84882fb814e4d9ad20543) , ROM_GROUPWORD | ROM_SKIP(6) ) // == lw-08.9f
	ROMX_LOAD( "lw-05.6d",  0x000004, 0x80000, CRC(e4552fd7) SHA1(11147afc475904848458425661473586dd6f60cc) , ROM_GROUPWORD | ROM_SKIP(6) ) // == lw-05.9e
	ROMX_LOAD( "lw_30.8h",  0x000006, 0x20000, CRC(b385954e) SHA1(d33adb5842e7b85d304836bd92a7a96be4ff3694) , ROM_SKIP(7) ) // == lw-12.9g
	ROMX_LOAD( "lw_29.8f",  0x000007, 0x20000, CRC(7bda1ac6) SHA1(5b8bd05f52798f98ae16efa2ff61c06e28a4e3a0) , ROM_SKIP(7) ) // == lw-12.9g
	ROMX_LOAD( "lw_4.3b",   0x100000, 0x20000, CRC(50cf757f) SHA1(c70d7d34ac2d6671d40dd372e241ccb60bf3bf2b) , ROM_SKIP(7) ) // == lw-01.9d
	ROMX_LOAD( "lw_3.3a",   0x100001, 0x20000, CRC(c03ef278) SHA1(ad33b01bd8194025a2ecf7755894d6d638da457a) , ROM_SKIP(7) ) // == lw-01.9d
	ROMX_LOAD( "lw_32.9h",  0x100006, 0x20000, CRC(30967a15) SHA1(6f6c6ca2f40aa9beec63ed64f0571bebc7c1aa50) , ROM_SKIP(7) ) // == lw-12.9g
	ROMX_LOAD( "lw_31.9f",  0x100007, 0x20000, CRC(c49d37fb) SHA1(ce400261a0f8d5a9b95d3823f8f52de87b8007f1) , ROM_SKIP(7) ) // == lw-12.9g
	ROMX_LOAD( "lw-02.6b",  0x200000, 0x80000, CRC(43e6c5c8) SHA1(d3e6c971de0477ec4e178adc82508208dd8b397f) , ROM_GROUPWORD | ROM_SKIP(6) ) // == lw-02.12d
	ROMX_LOAD( "lw_14.10b", 0x200002, 0x20000, CRC(82862cce) SHA1(727ca4ee55e076185b071a49afc87533fde9ec27) , ROM_SKIP(7) ) // == lw-09.12f
	ROMX_LOAD( "lw_13.10a", 0x200003, 0x20000, CRC(b81c0e96) SHA1(09f4235786b8ff92a57112669c0385b64477eb01) , ROM_SKIP(7) ) // == lw-09.12f
	ROMX_LOAD( "lw-06.9d",  0x200004, 0x80000, CRC(5b9edffc) SHA1(6fd8f4a3ab070733b52365ab1945bf86acb2bf62) , ROM_GROUPWORD | ROM_SKIP(6) ) // == lw-06.12e
	ROMX_LOAD( "lw-13.10d", 0x200006, 0x80000, CRC(8e058ef5) SHA1(00f2c0050fd106276ea5398511c5861ebfbc0d10) , ROM_GROUPWORD | ROM_SKIP(6) ) // == lw-13.12g
	ROMX_LOAD( "lw_16.11b", 0x300002, 0x20000, CRC(40b26554) SHA1(b4b27573d6c329bc2bc4c64fd857475bf2a10877) , ROM_SKIP(7) ) // == lw-09.12f
	ROMX_LOAD( "lw_15.11a", 0x300003, 0x20000, CRC(1b7d2e07) SHA1(0edf4d4b314fd9c29e7915d5d1adef6f9617f921) , ROM_SKIP(7) ) // == lw-09.12f

	ROM_REGION( 0x8000, "stars", 0 )
	ROM_COPY( "gfx", 0x200000, 0x000000, 0x8000 )

	ROM_REGION( 0x18000, "audiocpu", 0 )
	ROM_LOAD( "lw_00.13c",  0x00000, 0x08000, CRC(59df2a63) SHA1(dfe1fffc7a17179a80a2ae623e93b30a7d6df20d) )    // == lw_00b.14a
	ROM_CONTINUE(           0x10000, 0x08000 )

	ROM_REGION( 0x40000, "oki", 0 ) /* Samples */
	ROM_LOAD( "lw-03u.12e", 0x00000, 0x20000, CRC(807d051f) SHA1(720e4733787b9b11f4d1cdce0892b69475802844) )    // == lw-03u.14c
	ROM_LOAD( "lw-04u.13e", 0x20000, 0x20000, CRC(e6cd098e) SHA1(667f6e5736f76a1c4c450c4e2035574ea89d7910) )    // == lw-04u.13c

	ROM_REGION( 0x0200, "aboardplds", 0 )
	ROM_LOAD( "buf1",         0x0000, 0x0117, CRC(eb122de7) SHA1(b26b5bfe258e3e184f069719f9fd008d6b8f6b9b) )
	ROM_LOAD( "ioa1",         0x0000, 0x0117, CRC(59c7ee3b) SHA1(fbb887c5b4f5cb8df77cec710eaac2985bc482a6) )
	ROM_LOAD( "prg1",         0x0000, 0x0117, CRC(f1129744) SHA1(a5300f301c1a08a7da768f0773fa0fe3f683b237) )
	ROM_LOAD( "rom1",         0x0000, 0x0117, CRC(41dc73b9) SHA1(7d4c9f1693c821fbf84e32dd6ef62ddf14967845) )
	ROM_LOAD( "sou1",         0x0000, 0x0117, CRC(84f4b2fe) SHA1(dcc9e86cc36316fe42eace02d6df75d08bc8bb6d) )

	ROM_REGION( 0x0200, "bboardplds", 0 )
	ROM_LOAD( "lw621.1a",     0x0000, 0x0117, CRC(5eec6ce9) SHA1(5ec8b60f1f1bdba865b1fa2387987ce99ff4093a) )
	ROM_LOAD( "lwio.12b",     0x0000, 0x0117, CRC(ad52b90c) SHA1(f0fd6aeea515ee449320fe15684e6b3ab7f97bf4) )
ROM_END

/* B-Board 88618B-2 */
/* Note that this set is equivalent to forgottnu, but ROMs use the 88618B-2 B-Board layout. */
ROM_START( forgottnu1 )
	ROM_REGION( CODE_SIZE, "maincpu", 0 )
	ROM_LOAD16_BYTE( "lw11c.14f",      0x00000, 0x20000, CRC(e62742b6) SHA1(39dd3bdd405a8217d8816567d4f2014fc77f5ce8) )
	ROM_LOAD16_BYTE( "lw15c.14g",      0x00001, 0x20000, CRC(1b70f216) SHA1(f200f615dca8aa23d166e74b8baa9a8863ee7a95) )
	ROM_LOAD16_BYTE( "lw10c.13f",      0x40000, 0x20000, CRC(8f5ea3f5) SHA1(c3e43659bd7e03ec3d5c79647db380bde391b0f1) )
	ROM_LOAD16_BYTE( "lw14c.13g",      0x40001, 0x20000, CRC(708e7472) SHA1(44f82db41ffd1eece2d3f3c977f481fd84dfdbf3) )
	ROM_LOAD16_WORD_SWAP( "lw-07.13e", 0x80000, 0x80000, CRC(fd252a26) SHA1(5cfb097984912a5167a8c7ec4c2e119b642f9970) )

	ROM_REGION( 0x400000, "gfx", 0 )
	ROMX_LOAD( "lw-01.9d",  0x000000, 0x80000, CRC(0318f298) SHA1(178ffd6da7bf845e30abf1bfc38a469cd319a73f) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "lw-08.9f",  0x000002, 0x80000, CRC(25a8e43c) SHA1(d57cee1fc508db2677e84882fb814e4d9ad20543) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "lw-05.9e",  0x000004, 0x80000, CRC(e4552fd7) SHA1(11147afc475904848458425661473586dd6f60cc) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "lw-12.9g",  0x000006, 0x80000, CRC(8e6a832b) SHA1(d63a1331fda2365f090fa31950098f321a720ea8) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "lw-02.12d", 0x200000, 0x80000, CRC(43e6c5c8) SHA1(d3e6c971de0477ec4e178adc82508208dd8b397f) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "lw-09.12f", 0x200002, 0x80000, CRC(899cb4ad) SHA1(95e61af338945e690f2a82746feba3871ea224eb) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "lw-06.12e", 0x200004, 0x80000, CRC(5b9edffc) SHA1(6fd8f4a3ab070733b52365ab1945bf86acb2bf62) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "lw-13.12g", 0x200006, 0x80000, CRC(8e058ef5) SHA1(00f2c0050fd106276ea5398511c5861ebfbc0d10) , ROM_GROUPWORD | ROM_SKIP(6) )

	ROM_REGION( 0x8000, "stars", 0 )
	ROM_COPY( "gfx", 0x200000, 0x000000, 0x8000 )

	ROM_REGION( 0x18000, "audiocpu", 0 )
	ROM_LOAD( "lw_00.14a",  0x00000, 0x08000, CRC(59df2a63) SHA1(dfe1fffc7a17179a80a2ae623e93b30a7d6df20d) )    // == lw_00b.14a
	ROM_CONTINUE(           0x10000, 0x08000 )

	ROM_REGION( 0x40000, "oki", 0 ) /* Samples */
	ROM_LOAD( "lw-03u.14c", 0x00000, 0x20000, CRC(807d051f) SHA1(720e4733787b9b11f4d1cdce0892b69475802844) )
	ROM_LOAD( "lw-04u.13c", 0x20000, 0x20000, CRC(e6cd098e) SHA1(667f6e5736f76a1c4c450c4e2035574ea89d7910) )

	ROM_REGION( 0x0200, "aboardplds", 0 )
	ROM_LOAD( "buf1",         0x0000, 0x0117, CRC(eb122de7) SHA1(b26b5bfe258e3e184f069719f9fd008d6b8f6b9b) )
	ROM_LOAD( "ioa1",         0x0000, 0x0117, CRC(59c7ee3b) SHA1(fbb887c5b4f5cb8df77cec710eaac2985bc482a6) )
	ROM_LOAD( "prg1",         0x0000, 0x0117, CRC(f1129744) SHA1(a5300f301c1a08a7da768f0773fa0fe3f683b237) )
	ROM_LOAD( "rom1",         0x0000, 0x0117, CRC(41dc73b9) SHA1(7d4c9f1693c821fbf84e32dd6ef62ddf14967845) )
	ROM_LOAD( "sou1",         0x0000, 0x0117, CRC(84f4b2fe) SHA1(dcc9e86cc36316fe42eace02d6df75d08bc8bb6d) )

	ROM_REGION( 0x0200, "bboardplds", 0 )
	ROM_LOAD( "lwchr.3a",     0x0000, 0x0117, CRC(54ed4c39) SHA1(961309335dc1c84482ebe99ea938b32d3a6ae9a8) )
	ROM_LOAD( "lwio.15e",     0x0000, 0x0117, CRC(ad52b90c) SHA1(f0fd6aeea515ee449320fe15684e6b3ab7f97bf4) )
ROM_END

/* B-Board 88618B-2 */
ROM_START( forgottnua )
	ROM_REGION( CODE_SIZE, "maincpu", 0 )
	ROM_LOAD16_BYTE( "lwu_11a.14f",    0x00000, 0x20000, CRC(ddf78831) SHA1(b9c815613efdfde933d4500b588798b7fb4c1854) )
	ROM_LOAD16_BYTE( "lwu_15a.14g",    0x00001, 0x20000, CRC(f7ce2097) SHA1(44c06fabdb6de7d8afc2164458c90b0be9cf945d) )
	ROM_LOAD16_BYTE( "lwu_10a.13f",    0x40000, 0x20000, CRC(8cb38c81) SHA1(1d36cab7d17ff778ee7dfcd9606a3a87f6906f21) )
	ROM_LOAD16_BYTE( "lwu_14a.13g",    0x40001, 0x20000, CRC(d70ef9fd) SHA1(b393aa2a7bea440fdcf057ffc6ff233fc0d35d4b) )
	ROM_LOAD16_WORD_SWAP( "lw-07.13e", 0x80000, 0x80000, CRC(fd252a26) SHA1(5cfb097984912a5167a8c7ec4c2e119b642f9970) )

	ROM_REGION( 0x400000, "gfx", 0 )
	ROMX_LOAD( "lw-01.9d",  0x000000, 0x80000, CRC(0318f298) SHA1(178ffd6da7bf845e30abf1bfc38a469cd319a73f) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "lw-08.9f",  0x000002, 0x80000, CRC(25a8e43c) SHA1(d57cee1fc508db2677e84882fb814e4d9ad20543) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "lw-05.9e",  0x000004, 0x80000, CRC(e4552fd7) SHA1(11147afc475904848458425661473586dd6f60cc) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "lw-12.9g",  0x000006, 0x80000, CRC(8e6a832b) SHA1(d63a1331fda2365f090fa31950098f321a720ea8) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "lw-02.12d", 0x200000, 0x80000, CRC(43e6c5c8) SHA1(d3e6c971de0477ec4e178adc82508208dd8b397f) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "lw-09.12f", 0x200002, 0x80000, CRC(899cb4ad) SHA1(95e61af338945e690f2a82746feba3871ea224eb) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "lw-06.12e", 0x200004, 0x80000, CRC(5b9edffc) SHA1(6fd8f4a3ab070733b52365ab1945bf86acb2bf62) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "lw-13.12g", 0x200006, 0x80000, CRC(8e058ef5) SHA1(00f2c0050fd106276ea5398511c5861ebfbc0d10) , ROM_GROUPWORD | ROM_SKIP(6) )

	ROM_REGION( 0x8000, "stars", 0 )
	ROM_COPY( "gfx", 0x200000, 0x000000, 0x8000 )

	ROM_REGION( 0x18000, "audiocpu", 0 )
	ROM_LOAD( "lwu_00.14a", 0x00000, 0x08000, CRC(59df2a63) SHA1(dfe1fffc7a17179a80a2ae623e93b30a7d6df20d) )    // == lw_00b.14a
	ROM_CONTINUE(           0x10000, 0x08000 )

	ROM_REGION( 0x40000, "oki", 0 ) /* Samples */
	ROM_LOAD( "lw-03u.14c", 0x00000, 0x20000, CRC(807d051f) SHA1(720e4733787b9b11f4d1cdce0892b69475802844) )
	ROM_LOAD( "lw-04u.13c", 0x20000, 0x20000, CRC(e6cd098e) SHA1(667f6e5736f76a1c4c450c4e2035574ea89d7910) )

	ROM_REGION( 0x0200, "aboardplds", 0 )
	ROM_LOAD( "buf1",         0x0000, 0x0117, CRC(eb122de7) SHA1(b26b5bfe258e3e184f069719f9fd008d6b8f6b9b) )
	ROM_LOAD( "ioa1",         0x0000, 0x0117, CRC(59c7ee3b) SHA1(fbb887c5b4f5cb8df77cec710eaac2985bc482a6) )
	ROM_LOAD( "prg1",         0x0000, 0x0117, CRC(f1129744) SHA1(a5300f301c1a08a7da768f0773fa0fe3f683b237) )
	ROM_LOAD( "rom1",         0x0000, 0x0117, CRC(41dc73b9) SHA1(7d4c9f1693c821fbf84e32dd6ef62ddf14967845) )
	ROM_LOAD( "sou1",         0x0000, 0x0117, CRC(84f4b2fe) SHA1(dcc9e86cc36316fe42eace02d6df75d08bc8bb6d) )

	ROM_REGION( 0x0200, "bboardplds", 0 )
	ROM_LOAD( "lwchr.3a",     0x0000, 0x0117, CRC(54ed4c39) SHA1(961309335dc1c84482ebe99ea938b32d3a6ae9a8) )
	ROM_LOAD( "lwio.15e",     0x0000, 0x0117, CRC(ad52b90c) SHA1(f0fd6aeea515ee449320fe15684e6b3ab7f97bf4) )
ROM_END

/* B-Board 88618B-2 */
ROM_START( forgottnuaa ) /* 1 byte difference to parent set. Region byte or pointer to the US "warning" screen at boot */
	ROM_REGION( CODE_SIZE, "maincpu", 0 )
	ROM_LOAD16_BYTE( "lwu_11aa.14f",   0x00000, 0x20000, CRC(73e920b7) SHA1(2df12fc1a66f488d06b0927db909da81466d7d07) ) // == lw11.12f
	ROM_LOAD16_BYTE( "lwu_15aa.14g",   0x00001, 0x20000, CRC(e47524b9) SHA1(2fad1e59432cb63017caf691bb28eb7cbcecb3c6) ) // 1 byte difference to lw15.12h 0x8B6 == 0x07
	ROM_LOAD16_BYTE( "lwu_10aa.13f",   0x40000, 0x20000, CRC(bea45994) SHA1(c419f65c5e0c11ae7508ec54412bf6b62fac4f72) ) // == lw10.13f
	ROM_LOAD16_BYTE( "lwu_14aa.13g",   0x40001, 0x20000, CRC(539b2339) SHA1(8a9e452ef8ed05e0b956d36990266657d3077470) ) // == lw14.13h
	ROM_LOAD16_WORD_SWAP( "lw-07.13e", 0x80000, 0x80000, CRC(fd252a26) SHA1(5cfb097984912a5167a8c7ec4c2e119b642f9970) )

	ROM_REGION( 0x400000, "gfx", 0 )
	ROMX_LOAD( "lw-01.9d",  0x000000, 0x80000, CRC(0318f298) SHA1(178ffd6da7bf845e30abf1bfc38a469cd319a73f) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "lw-08.9f",  0x000002, 0x80000, CRC(25a8e43c) SHA1(d57cee1fc508db2677e84882fb814e4d9ad20543) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "lw-05.9e",  0x000004, 0x80000, CRC(e4552fd7) SHA1(11147afc475904848458425661473586dd6f60cc) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "lw-12.9g",  0x000006, 0x80000, CRC(8e6a832b) SHA1(d63a1331fda2365f090fa31950098f321a720ea8) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "lw-02.12d", 0x200000, 0x80000, CRC(43e6c5c8) SHA1(d3e6c971de0477ec4e178adc82508208dd8b397f) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "lw-09.12f", 0x200002, 0x80000, CRC(899cb4ad) SHA1(95e61af338945e690f2a82746feba3871ea224eb) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "lw-06.12e", 0x200004, 0x80000, CRC(5b9edffc) SHA1(6fd8f4a3ab070733b52365ab1945bf86acb2bf62) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "lw-13.12g", 0x200006, 0x80000, CRC(8e058ef5) SHA1(00f2c0050fd106276ea5398511c5861ebfbc0d10) , ROM_GROUPWORD | ROM_SKIP(6) )

	ROM_REGION( 0x8000, "stars", 0 )
	ROM_COPY( "gfx", 0x200000, 0x000000, 0x8000 )

	ROM_REGION( 0x18000, "audiocpu", 0 )
	ROM_LOAD( "lwu_00.14a", 0x00000, 0x08000, CRC(59df2a63) SHA1(dfe1fffc7a17179a80a2ae623e93b30a7d6df20d) )    // == lw_00b.14a
	ROM_CONTINUE(           0x10000, 0x08000 )

	ROM_REGION( 0x40000, "oki", 0 ) /* Samples */
	ROM_LOAD( "lw-03u.14c", 0x00000, 0x20000, CRC(807d051f) SHA1(720e4733787b9b11f4d1cdce0892b69475802844) )
	ROM_LOAD( "lw-04u.13c", 0x20000, 0x20000, CRC(e6cd098e) SHA1(667f6e5736f76a1c4c450c4e2035574ea89d7910) )

	ROM_REGION( 0x0200, "aboardplds", 0 )
	ROM_LOAD( "buf1",         0x0000, 0x0117, CRC(eb122de7) SHA1(b26b5bfe258e3e184f069719f9fd008d6b8f6b9b) )
	ROM_LOAD( "ioa1",         0x0000, 0x0117, CRC(59c7ee3b) SHA1(fbb887c5b4f5cb8df77cec710eaac2985bc482a6) )
	ROM_LOAD( "prg1",         0x0000, 0x0117, CRC(f1129744) SHA1(a5300f301c1a08a7da768f0773fa0fe3f683b237) )
	ROM_LOAD( "rom1",         0x0000, 0x0117, CRC(41dc73b9) SHA1(7d4c9f1693c821fbf84e32dd6ef62ddf14967845) )
	ROM_LOAD( "sou1",         0x0000, 0x0117, CRC(84f4b2fe) SHA1(dcc9e86cc36316fe42eace02d6df75d08bc8bb6d) )

	ROM_REGION( 0x0200, "bboardplds", 0 )
	ROM_LOAD( "lwchr.3a",     0x0000, 0x0117, CRC(54ed4c39) SHA1(961309335dc1c84482ebe99ea938b32d3a6ae9a8) )
	ROM_LOAD( "lwio.15e",     0x0000, 0x0117, CRC(ad52b90c) SHA1(f0fd6aeea515ee449320fe15684e6b3ab7f97bf4) )
ROM_END

/* B-Board 88618B-2 */
ROM_START( lostwrld )
	ROM_REGION( CODE_SIZE, "maincpu", 0 )
	ROM_LOAD16_BYTE( "lw_11c.14f",     0x00000, 0x20000, CRC(67e42546) SHA1(3e385661f71616180a26b74e443978077246fe66) )
	ROM_LOAD16_BYTE( "lw_15c.14g",     0x00001, 0x20000, CRC(402e2a46) SHA1(cbb7017e75a425706505717bf83c2615f53309f9) )
	ROM_LOAD16_BYTE( "lw_10c.13f",     0x40000, 0x20000, CRC(c46479d7) SHA1(84fd9ef33ae7d0af2110e8dc299de25c0f039cee) )
	ROM_LOAD16_BYTE( "lw_14c.13g",     0x40001, 0x20000, CRC(97670f4a) SHA1(f249977c814abdff85007216d7fa57db5684be0f) )
	ROM_LOAD16_WORD_SWAP( "lw-07.13e", 0x80000, 0x80000, CRC(fd252a26) SHA1(5cfb097984912a5167a8c7ec4c2e119b642f9970) )

	ROM_REGION( 0x400000, "gfx", 0 )
	ROMX_LOAD( "lw-01.9d",  0x000000, 0x80000, CRC(0318f298) SHA1(178ffd6da7bf845e30abf1bfc38a469cd319a73f) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "lw-08.9f",  0x000002, 0x80000, CRC(25a8e43c) SHA1(d57cee1fc508db2677e84882fb814e4d9ad20543) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "lw-05.9e",  0x000004, 0x80000, CRC(e4552fd7) SHA1(11147afc475904848458425661473586dd6f60cc) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "lw-12.9g",  0x000006, 0x80000, CRC(8e6a832b) SHA1(d63a1331fda2365f090fa31950098f321a720ea8) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "lw-02.12d", 0x200000, 0x80000, CRC(43e6c5c8) SHA1(d3e6c971de0477ec4e178adc82508208dd8b397f) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "lw-09.12f", 0x200002, 0x80000, CRC(899cb4ad) SHA1(95e61af338945e690f2a82746feba3871ea224eb) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "lw-06.12e", 0x200004, 0x80000, CRC(5b9edffc) SHA1(6fd8f4a3ab070733b52365ab1945bf86acb2bf62) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "lw-13.12g", 0x200006, 0x80000, CRC(8e058ef5) SHA1(00f2c0050fd106276ea5398511c5861ebfbc0d10) , ROM_GROUPWORD | ROM_SKIP(6) )

	ROM_REGION( 0x8000, "stars", 0 )
	ROM_COPY( "gfx", 0x200000, 0x000000, 0x8000 )

	ROM_REGION( 0x18000, "audiocpu", 0 )
	ROM_LOAD( "lw_00b.14a", 0x00000, 0x08000, CRC(59df2a63) SHA1(dfe1fffc7a17179a80a2ae623e93b30a7d6df20d) )
	ROM_CONTINUE(           0x10000, 0x08000 )

	ROM_REGION( 0x40000, "oki", 0 ) /* Samples */
	ROM_LOAD( "lw-03.14c",  0x00000, 0x20000, CRC(ce2159e7) SHA1(77d564f8b768c1cbd6e5b334f7ee86c4c3f9d62e) )
	ROM_LOAD( "lw-04.13c",  0x20000, 0x20000, CRC(39305536) SHA1(ad24d7b6df2dc5e84a35aecb9ba9b0aaa27ab6e5) )

	ROM_REGION( 0x0200, "aboardplds", 0 )
	ROM_LOAD( "buf1",         0x0000, 0x0117, CRC(eb122de7) SHA1(b26b5bfe258e3e184f069719f9fd008d6b8f6b9b) )
	ROM_LOAD( "ioa1",         0x0000, 0x0117, CRC(59c7ee3b) SHA1(fbb887c5b4f5cb8df77cec710eaac2985bc482a6) )
	ROM_LOAD( "prg1",         0x0000, 0x0117, CRC(f1129744) SHA1(a5300f301c1a08a7da768f0773fa0fe3f683b237) )
	ROM_LOAD( "rom1",         0x0000, 0x0117, CRC(41dc73b9) SHA1(7d4c9f1693c821fbf84e32dd6ef62ddf14967845) )
	ROM_LOAD( "sou1",         0x0000, 0x0117, CRC(84f4b2fe) SHA1(dcc9e86cc36316fe42eace02d6df75d08bc8bb6d) )

	ROM_REGION( 0x0200, "bboardplds", 0 )
	ROM_LOAD( "lwchr.3a",     0x0000, 0x0117, CRC(54ed4c39) SHA1(961309335dc1c84482ebe99ea938b32d3a6ae9a8) )
	ROM_LOAD( "lwio.15e",     0x0000, 0x0117, CRC(ad52b90c) SHA1(f0fd6aeea515ee449320fe15684e6b3ab7f97bf4) )
ROM_END

/* B-Board 88618B-2 */
ROM_START( lostwrldo )
	ROM_REGION( CODE_SIZE, "maincpu", 0 )
	ROM_LOAD16_BYTE( "lw_11.14f",      0x00000, 0x20000, CRC(61e2cc56) SHA1(bc192e37806129dd00677ca2c2e4287aa670973d) )
	ROM_LOAD16_BYTE( "lw_15.14g",      0x00001, 0x20000, CRC(8a0c18d3) SHA1(7a4d80cefa0c2be427de43fbbbb906880876ed1d) )
	ROM_LOAD16_BYTE( "lw_10.13f",      0x40000, 0x20000, CRC(23bca4d5) SHA1(c9d356d052d82117b6c30e40aa02a5f1f5335a73) )
	ROM_LOAD16_BYTE( "lw_14.13g",      0x40001, 0x20000, CRC(3a023771) SHA1(30de9b76e104e897ddce64615f24bba0aa270d14) )
	ROM_LOAD16_WORD_SWAP( "lw-07.13e", 0x80000, 0x80000, CRC(fd252a26) SHA1(5cfb097984912a5167a8c7ec4c2e119b642f9970) )

	ROM_REGION( 0x400000, "gfx", 0 )
	ROMX_LOAD( "lw-01.9d",  0x000000, 0x80000, CRC(0318f298) SHA1(178ffd6da7bf845e30abf1bfc38a469cd319a73f) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "lw-08.9f",  0x000002, 0x80000, CRC(25a8e43c) SHA1(d57cee1fc508db2677e84882fb814e4d9ad20543) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "lw-05.9e",  0x000004, 0x80000, CRC(e4552fd7) SHA1(11147afc475904848458425661473586dd6f60cc) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "lw-12.9g",  0x000006, 0x80000, CRC(8e6a832b) SHA1(d63a1331fda2365f090fa31950098f321a720ea8) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "lw-02.12d", 0x200000, 0x80000, CRC(43e6c5c8) SHA1(d3e6c971de0477ec4e178adc82508208dd8b397f) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "lw-09.12f", 0x200002, 0x80000, CRC(899cb4ad) SHA1(95e61af338945e690f2a82746feba3871ea224eb) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "lw-06.12e", 0x200004, 0x80000, CRC(5b9edffc) SHA1(6fd8f4a3ab070733b52365ab1945bf86acb2bf62) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "lw-13.12g", 0x200006, 0x80000, CRC(8e058ef5) SHA1(00f2c0050fd106276ea5398511c5861ebfbc0d10) , ROM_GROUPWORD | ROM_SKIP(6) )

	ROM_REGION( 0x8000, "stars", 0 )
	ROM_COPY( "gfx", 0x200000, 0x000000, 0x8000 )

	ROM_REGION( 0x18000, "audiocpu", 0 )
	ROM_LOAD( "lw_00b.14a", 0x00000, 0x08000, CRC(59df2a63) SHA1(dfe1fffc7a17179a80a2ae623e93b30a7d6df20d) )
	ROM_CONTINUE(           0x10000, 0x08000 )

	ROM_REGION( 0x40000, "oki", 0 ) /* Samples */
	ROM_LOAD( "lw-03.14c",  0x00000, 0x20000, CRC(ce2159e7) SHA1(77d564f8b768c1cbd6e5b334f7ee86c4c3f9d62e) )
	ROM_LOAD( "lw-04.13c",  0x20000, 0x20000, CRC(39305536) SHA1(ad24d7b6df2dc5e84a35aecb9ba9b0aaa27ab6e5) )

	ROM_REGION( 0x0200, "aboardplds", 0 )
	ROM_LOAD( "buf1",         0x0000, 0x0117, CRC(eb122de7) SHA1(b26b5bfe258e3e184f069719f9fd008d6b8f6b9b) )
	ROM_LOAD( "ioa1",         0x0000, 0x0117, CRC(59c7ee3b) SHA1(fbb887c5b4f5cb8df77cec710eaac2985bc482a6) )
	ROM_LOAD( "prg1",         0x0000, 0x0117, CRC(f1129744) SHA1(a5300f301c1a08a7da768f0773fa0fe3f683b237) )
	ROM_LOAD( "rom1",         0x0000, 0x0117, CRC(41dc73b9) SHA1(7d4c9f1693c821fbf84e32dd6ef62ddf14967845) )
	ROM_LOAD( "sou1",         0x0000, 0x0117, CRC(84f4b2fe) SHA1(dcc9e86cc36316fe42eace02d6df75d08bc8bb6d) )

	ROM_REGION( 0x0200, "bboardplds", 0 )
	ROM_LOAD( "lwchr.3a",     0x0000, 0x0117, CRC(54ed4c39) SHA1(961309335dc1c84482ebe99ea938b32d3a6ae9a8) )
	ROM_LOAD( "lwio.15e",     0x0000, 0x0117, CRC(ad52b90c) SHA1(f0fd6aeea515ee449320fe15684e6b3ab7f97bf4) )
ROM_END

/* B-Board 88620-B-? */
/* Note that ROMs are labeled left to right, top to bottom, instead of top to bottom, left to right as usual. */
ROM_START( ghouls )
	ROM_REGION( CODE_SIZE, "maincpu", 0 )
	ROM_LOAD16_BYTE( "dme_29.10h", 0x00000, 0x20000, CRC(166a58a2) SHA1(f21fcf88d2ebb7bc9e8885fde760a5d82f295c1a) )
	ROM_LOAD16_BYTE( "dme_30.10j", 0x00001, 0x20000, CRC(7ac8407a) SHA1(3613699213db47bfeabedf87f12eb0fa7e5973b6) )
	ROM_LOAD16_BYTE( "dme_27.9h",  0x40000, 0x20000, CRC(f734b2be) SHA1(fa230bf5503487ec11d767485a18f0a55dcc13d2) )
	ROM_LOAD16_BYTE( "dme_28.9j",  0x40001, 0x20000, CRC(03d3e714) SHA1(a07786062358c89f3b4634b8822173261802290b) )
	ROM_LOAD16_WORD( "dm-17.7j",   0x80000, 0x80000, CRC(3ea1b0f2) SHA1(c51f1c38cdaed77ad715cedd845617a291ab2441) )

	ROM_REGION( 0x300000, "gfx", 0 )
	ROMX_LOAD( "dm-05.3a", 0x000000, 0x80000, CRC(0ba9c0b0) SHA1(c4945b603115f32b7346d72426571dc2d361159f) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "dm-07.3f", 0x000002, 0x80000, CRC(5d760ab9) SHA1(212176947933fcfef991bc80ad5bd91718689ffe) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "dm-06.3c", 0x000004, 0x80000, CRC(4ba90b59) SHA1(35bc9dec5ddbf064c30c951627581c16764456ac) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "dm-08.3g", 0x000006, 0x80000, CRC(4bdee9de) SHA1(7d0c4736f16577afe9966447a18f039728f6fbdf) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "09.4a",    0x200000, 0x10000, CRC(ae24bb19) SHA1(aa91c6ffe657b878e10e4e930457b530f7bb529b) , ROM_SKIP(7) )
	ROMX_LOAD( "18.7a",    0x200001, 0x10000, CRC(d34e271a) SHA1(55211fc2861dce32951f41624c9c99c09bf3b184) , ROM_SKIP(7) )
	ROMX_LOAD( "13.4e",    0x200002, 0x10000, CRC(3f70dd37) SHA1(9ecb4dec9d131e9fca15ded7d71986a9fcb62c19) , ROM_SKIP(7) )
	ROMX_LOAD( "22.7e",    0x200003, 0x10000, CRC(7e69e2e6) SHA1(4e0b4d2474beaa5c869c8f1a91893c79ac6e7f39) , ROM_SKIP(7) )
	ROMX_LOAD( "11.4c",    0x200004, 0x10000, CRC(37c9b6c6) SHA1(b2bb82537e335339846dbf9588cfacfdbdd75ee6) , ROM_SKIP(7) )
	ROMX_LOAD( "20.7c",    0x200005, 0x10000, CRC(2f1345b4) SHA1(14c450abcf9defa29c6f48e5ffd0b9d1e4a66a1d) , ROM_SKIP(7) )
	ROMX_LOAD( "15.4g",    0x200006, 0x10000, CRC(3c2a212a) SHA1(f8fa0e0e2d09ea37c54d1c2493752b4e97e3f534) , ROM_SKIP(7) )
	ROMX_LOAD( "24.7g",    0x200007, 0x10000, CRC(889aac05) SHA1(9301dcecee699e7f7672bb498122e1f4831ce536) , ROM_SKIP(7) )
	ROMX_LOAD( "10.4b",    0x280000, 0x10000, CRC(bcc0f28c) SHA1(02f587aa4ae71631f27b0e3aaf1829cdded1bdc2) , ROM_SKIP(7) )
	ROMX_LOAD( "19.7b",    0x280001, 0x10000, CRC(2a40166a) SHA1(dc4e75d7ed87ae5386d721a09113bba364740465) , ROM_SKIP(7) )
	ROMX_LOAD( "14.4f",    0x280002, 0x10000, CRC(20f85c03) SHA1(86385139a9b42270aade758bfe338525936f5671) , ROM_SKIP(7) )
	ROMX_LOAD( "23.7f",    0x280003, 0x10000, CRC(8426144b) SHA1(2dbf9625413b302fcdad5bef8733a9dfbfaead52) , ROM_SKIP(7) )
	ROMX_LOAD( "12.4d",    0x280004, 0x10000, CRC(da088d61) SHA1(67229eff2827a42af97a60ceb252e132e7f307bc) , ROM_SKIP(7) )
	ROMX_LOAD( "21.7d",    0x280005, 0x10000, CRC(17e11df0) SHA1(42fb15e9300b07fc5f4bc21744484869859b130c) , ROM_SKIP(7) )
	ROMX_LOAD( "16.4h",    0x280006, 0x10000, CRC(f187ba1c) SHA1(6d9441d04ecef2a9d9c7a2cc7781acd7904c2061) , ROM_SKIP(7) )
	ROMX_LOAD( "25.7h",    0x280007, 0x10000, CRC(29f79c78) SHA1(26000a58454a06c3016f99ebc3a79c52911a7070) , ROM_SKIP(7) )

	ROM_REGION( 0x18000, "audiocpu", 0 )
	ROM_LOAD( "26.10a",    0x00000, 0x08000, CRC(3692f6e5) SHA1(61b8438d60a39b4cf5062dff0a53228e8a4e4b5f) )
	ROM_CONTINUE(          0x10000, 0x08000 )

	ROM_REGION( 0x40000, "oki", ROMREGION_ERASEFF ) /* Samples (not present) */

	ROM_REGION( 0x0200, "aboardplds", 0 )
	ROM_LOAD( "buf1",         0x0000, 0x0117, CRC(eb122de7) SHA1(b26b5bfe258e3e184f069719f9fd008d6b8f6b9b) )
	ROM_LOAD( "ioa1",         0x0000, 0x0117, CRC(59c7ee3b) SHA1(fbb887c5b4f5cb8df77cec710eaac2985bc482a6) )
	ROM_LOAD( "prg1",         0x0000, 0x0117, CRC(f1129744) SHA1(a5300f301c1a08a7da768f0773fa0fe3f683b237) )
	ROM_LOAD( "rom1",         0x0000, 0x0117, CRC(41dc73b9) SHA1(7d4c9f1693c821fbf84e32dd6ef62ddf14967845) )
	ROM_LOAD( "sou1",         0x0000, 0x0117, CRC(84f4b2fe) SHA1(dcc9e86cc36316fe42eace02d6df75d08bc8bb6d) )

	ROM_REGION( 0x0200, "bboardplds", 0 )
	ROM_LOAD( "dm620.2a",     0x0000, 0x0117, CRC(f6e5f727) SHA1(8d38c458721347272ccc14b2c0e9885c4f891477) )
	ROM_LOAD( "lwio.8i",      0x0000, 0x0117, CRC(ad52b90c) SHA1(f0fd6aeea515ee449320fe15684e6b3ab7f97bf4) )
ROM_END

/* B-Board 88620-B-2 */
/* Note that ROMs are labeled left to right, top to bottom, instead of top to bottom, left to right as usual. */
ROM_START( ghoulsu )
	ROM_REGION( CODE_SIZE, "maincpu", 0 )
	ROM_LOAD16_BYTE( "dmu_29.10h", 0x00000, 0x20000, CRC(334d85b2) SHA1(89bacc28b7c799c7568420e3de5a99060baa7b0f) )
	ROM_LOAD16_BYTE( "dmu_30.10j", 0x00001, 0x20000, CRC(cee8ceb5) SHA1(fc8db1ce0c143dfda0b5989d02d5e5a872e27cd2) )
	ROM_LOAD16_BYTE( "dmu_27.9h",  0x40000, 0x20000, CRC(4a524140) SHA1(cebd651293c3570912d5506c1c223c39bcccc802) )
	ROM_LOAD16_BYTE( "dmu_28.9j",  0x40001, 0x20000, CRC(94aae205) SHA1(514b3c1b9b0b22300a94229825c3be66332ea5ed) )
	ROM_LOAD16_WORD( "dm-17.7j",   0x80000, 0x80000, CRC(3ea1b0f2) SHA1(c51f1c38cdaed77ad715cedd845617a291ab2441) )

	ROM_REGION( 0x300000, "gfx", 0 )
	ROMX_LOAD( "dm-05.3a", 0x000000, 0x80000, CRC(0ba9c0b0) SHA1(c4945b603115f32b7346d72426571dc2d361159f) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "dm-07.3f", 0x000002, 0x80000, CRC(5d760ab9) SHA1(212176947933fcfef991bc80ad5bd91718689ffe) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "dm-06.3c", 0x000004, 0x80000, CRC(4ba90b59) SHA1(35bc9dec5ddbf064c30c951627581c16764456ac) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "dm-08.3g", 0x000006, 0x80000, CRC(4bdee9de) SHA1(7d0c4736f16577afe9966447a18f039728f6fbdf) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "09.4a",    0x200000, 0x10000, CRC(ae24bb19) SHA1(aa91c6ffe657b878e10e4e930457b530f7bb529b) , ROM_SKIP(7) )
	ROMX_LOAD( "18.7a",    0x200001, 0x10000, CRC(d34e271a) SHA1(55211fc2861dce32951f41624c9c99c09bf3b184) , ROM_SKIP(7) )
	ROMX_LOAD( "13.4e",    0x200002, 0x10000, CRC(3f70dd37) SHA1(9ecb4dec9d131e9fca15ded7d71986a9fcb62c19) , ROM_SKIP(7) )
	ROMX_LOAD( "22.7e",    0x200003, 0x10000, CRC(7e69e2e6) SHA1(4e0b4d2474beaa5c869c8f1a91893c79ac6e7f39) , ROM_SKIP(7) )
	ROMX_LOAD( "11.4c",    0x200004, 0x10000, CRC(37c9b6c6) SHA1(b2bb82537e335339846dbf9588cfacfdbdd75ee6) , ROM_SKIP(7) )
	ROMX_LOAD( "20.7c",    0x200005, 0x10000, CRC(2f1345b4) SHA1(14c450abcf9defa29c6f48e5ffd0b9d1e4a66a1d) , ROM_SKIP(7) )
	ROMX_LOAD( "15.4g",    0x200006, 0x10000, CRC(3c2a212a) SHA1(f8fa0e0e2d09ea37c54d1c2493752b4e97e3f534) , ROM_SKIP(7) )
	ROMX_LOAD( "24.7g",    0x200007, 0x10000, CRC(889aac05) SHA1(9301dcecee699e7f7672bb498122e1f4831ce536) , ROM_SKIP(7) )
	ROMX_LOAD( "10.4b",    0x280000, 0x10000, CRC(bcc0f28c) SHA1(02f587aa4ae71631f27b0e3aaf1829cdded1bdc2) , ROM_SKIP(7) )
	ROMX_LOAD( "19.7b",    0x280001, 0x10000, CRC(2a40166a) SHA1(dc4e75d7ed87ae5386d721a09113bba364740465) , ROM_SKIP(7) )
	ROMX_LOAD( "14.4f",    0x280002, 0x10000, CRC(20f85c03) SHA1(86385139a9b42270aade758bfe338525936f5671) , ROM_SKIP(7) )
	ROMX_LOAD( "23.7f",    0x280003, 0x10000, CRC(8426144b) SHA1(2dbf9625413b302fcdad5bef8733a9dfbfaead52) , ROM_SKIP(7) )
	ROMX_LOAD( "12.4d",    0x280004, 0x10000, CRC(da088d61) SHA1(67229eff2827a42af97a60ceb252e132e7f307bc) , ROM_SKIP(7) )
	ROMX_LOAD( "21.7d",    0x280005, 0x10000, CRC(17e11df0) SHA1(42fb15e9300b07fc5f4bc21744484869859b130c) , ROM_SKIP(7) )
	ROMX_LOAD( "16.4h",    0x280006, 0x10000, CRC(f187ba1c) SHA1(6d9441d04ecef2a9d9c7a2cc7781acd7904c2061) , ROM_SKIP(7) )
	ROMX_LOAD( "25.7h",    0x280007, 0x10000, CRC(29f79c78) SHA1(26000a58454a06c3016f99ebc3a79c52911a7070) , ROM_SKIP(7) )

	ROM_REGION( 0x18000, "audiocpu", 0 )
	ROM_LOAD( "26.10a",    0x00000, 0x08000, CRC(3692f6e5) SHA1(61b8438d60a39b4cf5062dff0a53228e8a4e4b5f) )
	ROM_CONTINUE(          0x10000, 0x08000 )

	ROM_REGION( 0x40000, "oki", ROMREGION_ERASEFF ) /* Samples (not present) */

	ROM_REGION( 0x0200, "aboardplds", 0 )
	ROM_LOAD( "buf1",         0x0000, 0x0117, CRC(eb122de7) SHA1(b26b5bfe258e3e184f069719f9fd008d6b8f6b9b) )
	ROM_LOAD( "ioa1",         0x0000, 0x0117, CRC(59c7ee3b) SHA1(fbb887c5b4f5cb8df77cec710eaac2985bc482a6) )
	ROM_LOAD( "prg1",         0x0000, 0x0117, CRC(f1129744) SHA1(a5300f301c1a08a7da768f0773fa0fe3f683b237) )
	ROM_LOAD( "rom1",         0x0000, 0x0117, CRC(41dc73b9) SHA1(7d4c9f1693c821fbf84e32dd6ef62ddf14967845) )
	ROM_LOAD( "sou1",         0x0000, 0x0117, CRC(84f4b2fe) SHA1(dcc9e86cc36316fe42eace02d6df75d08bc8bb6d) )

	ROM_REGION( 0x0200, "bboardplds", 0 )
	ROM_LOAD( "dm620.2a",     0x0000, 0x0117, CRC(f6e5f727) SHA1(8d38c458721347272ccc14b2c0e9885c4f891477) )
	ROM_LOAD( "lwio.8i",      0x0000, 0x0117, CRC(ad52b90c) SHA1(f0fd6aeea515ee449320fe15684e6b3ab7f97bf4) )
ROM_END

/* B-Board 88622B-2 */
/* Note that ROMs are labeled left to right, top to bottom, instead of top to bottom, left to right as usual. */
ROM_START( daimakai )
	ROM_REGION( CODE_SIZE, "maincpu", 0 )
	ROM_LOAD16_BYTE( "dmj_38.12f", 0x00000, 0x20000, CRC(82fd1798) SHA1(7a199384659d8e6602384b1953339f221d61a9e6) )
	ROM_LOAD16_BYTE( "dmj_39.12h", 0x00001, 0x20000, CRC(35366ccc) SHA1(42c7004a641f34b9dd1333be51b50639a97e2be9) )
	ROM_LOAD16_BYTE( "dmj_40.13f", 0x40000, 0x20000, CRC(a17c170a) SHA1(62a9cb65df90827334d453a98e826dc1bfc27136) )
	ROM_LOAD16_BYTE( "dmj_41.13h", 0x40001, 0x20000, CRC(6af0b391) SHA1(5a2d74d207c04e24bcea7eeffa1c8b96b6df77e1) )
	ROM_LOAD16_BYTE( "dm_33.10f",  0x80000, 0x20000, CRC(384d60c4) SHA1(258f9e2334b7240cf665b530f2c69b8826850687) ) // == dm-17.7j
	ROM_LOAD16_BYTE( "dm_34.10h",  0x80001, 0x20000, CRC(19abe30f) SHA1(aea7d5c8357201b68dec70d7cc8f87dfb8fce207) ) // == dm-17.7j
	ROM_LOAD16_BYTE( "dm_35.11f",  0xc0000, 0x20000, CRC(c04b85c8) SHA1(f8659624bb9d418d02f63f43478d3b53cfe18718) ) // == dm-17.7j
	ROM_LOAD16_BYTE( "dm_36.11h",  0xc0001, 0x20000, CRC(89be83de) SHA1(6dfd1380304a85dee7cac4d0b2cfd7625b9020bf) ) // == dm-17.7j

	ROM_REGION( 0x300000, "gfx", 0 )
	ROMX_LOAD( "dm_02.4b",  0x000000, 0x20000, CRC(8b98dc48) SHA1(e827881e2ba5cccd907d1692a1945c1b75d46f12) , ROM_SKIP(7) ) // == dm-05.3a
	ROMX_LOAD( "dm_01.4a",  0x000001, 0x20000, CRC(80896c33) SHA1(20ffc427c596828005e34cdd8e4efa0d332262e9) , ROM_SKIP(7) ) // == dm-05.3a
	ROMX_LOAD( "dm_10.9b",  0x000002, 0x20000, CRC(c2e7d9ef) SHA1(52aae6cf373f8c7150833047be28b74dd5eb5af6) , ROM_SKIP(7) ) // == dm-07.3f
	ROMX_LOAD( "dm_09.9a",  0x000003, 0x20000, CRC(c9c4afa5) SHA1(34571e3e49c86b87fa34eefbc5f9fe258aba5f1a) , ROM_SKIP(7) ) // == dm-07.3f
	ROMX_LOAD( "dm_18.5e",  0x000004, 0x20000, CRC(1aa0db99) SHA1(69ac302b2f6f0b96f78cb57b0b4cdae464086262) , ROM_SKIP(7) ) // == dm-06.3c
	ROMX_LOAD( "dm_17.5c",  0x000005, 0x20000, CRC(dc6ed8ad) SHA1(1ffc4a48a7ff9b542ab6f63a60bea3c1a7a6e63b) , ROM_SKIP(7) ) // == dm-06.3c
	ROMX_LOAD( "dm_30.8h",  0x000006, 0x20000, CRC(d9d3f8bd) SHA1(6c6853a384f8d60ca46a0607fd47c76a83700fba) , ROM_SKIP(7) ) // == dm-08.3g
	ROMX_LOAD( "dm_29.8f",  0x000007, 0x20000, CRC(49a48796) SHA1(76c18c684dba4aa91ee6caae0f38fe3e1cc50832) , ROM_SKIP(7) ) // == dm-08.3g
	ROMX_LOAD( "dm_04.5b",  0x100000, 0x20000, CRC(a4f4f8f0) SHA1(edca0f61b40a18afe279f7007c233064130cfb4f) , ROM_SKIP(7) ) // == dm-05.3a
	ROMX_LOAD( "dm_03.5a",  0x100001, 0x20000, CRC(b1033e62) SHA1(547fc281dd9e7a74ac86c3692508c7bde9b6167b) , ROM_SKIP(7) ) // == dm-05.3a
	ROMX_LOAD( "dm_12.10b", 0x100002, 0x20000, CRC(10fdd76a) SHA1(aee774d6323292799dff7a30ef9559c92fe5507a) , ROM_SKIP(7) ) // == dm-07.3f
	ROMX_LOAD( "dm_11.10a", 0x100003, 0x20000, CRC(9040cb04) SHA1(b32e9056fc20a5162868eade10f3ef5efc167a28) , ROM_SKIP(7) ) // == dm-07.3f
	ROMX_LOAD( "dm_20.7e",  0x100004, 0x20000, CRC(281d0b3e) SHA1(70e1813de184ad0ec164145b7b843b5e387494e3) , ROM_SKIP(7) ) // == dm-06.3c
	ROMX_LOAD( "dm_19.7c",  0x100005, 0x20000, CRC(2623b52f) SHA1(fc4200924452bfbff687934782398ed345dc0aa0) , ROM_SKIP(7) ) // == dm-06.3c
	ROMX_LOAD( "dm_32.9h",  0x100006, 0x20000, CRC(99692344) SHA1(67dc70618568b7c0adcb00a612aaf5501f6c8c0f) , ROM_SKIP(7) ) // == dm-08.3g
	ROMX_LOAD( "dm_31.9f",  0x100007, 0x20000, CRC(54acb729) SHA1(d1fca43db36253fd19db4337c49272a6cadff597) , ROM_SKIP(7) ) // == dm-08.3g
	ROMX_LOAD( "dm_06.7b",  0x200000, 0x10000, CRC(ae24bb19) SHA1(aa91c6ffe657b878e10e4e930457b530f7bb529b) , ROM_SKIP(7) ) // == 09.4a
	ROMX_LOAD( "dm_05.7a",  0x200001, 0x10000, CRC(d34e271a) SHA1(55211fc2861dce32951f41624c9c99c09bf3b184) , ROM_SKIP(7) ) // == 18.7a
	ROMX_LOAD( "dm_14.11b", 0x200002, 0x10000, CRC(3f70dd37) SHA1(9ecb4dec9d131e9fca15ded7d71986a9fcb62c19) , ROM_SKIP(7) ) // == 13.4e
	ROMX_LOAD( "dm_13.11a", 0x200003, 0x10000, CRC(7e69e2e6) SHA1(4e0b4d2474beaa5c869c8f1a91893c79ac6e7f39) , ROM_SKIP(7) ) // == 22.7e
	ROMX_LOAD( "dm_22.8e",  0x200004, 0x10000, CRC(37c9b6c6) SHA1(b2bb82537e335339846dbf9588cfacfdbdd75ee6) , ROM_SKIP(7) ) // == 11.4c
	ROMX_LOAD( "dm_21.8c",  0x200005, 0x10000, CRC(2f1345b4) SHA1(14c450abcf9defa29c6f48e5ffd0b9d1e4a66a1d) , ROM_SKIP(7) ) // == 20.7c
	ROMX_LOAD( "dm_26.10e", 0x200006, 0x10000, CRC(3c2a212a) SHA1(f8fa0e0e2d09ea37c54d1c2493752b4e97e3f534) , ROM_SKIP(7) ) // == 15.4g
	ROMX_LOAD( "dm_25.10c", 0x200007, 0x10000, CRC(889aac05) SHA1(9301dcecee699e7f7672bb498122e1f4831ce536) , ROM_SKIP(7) ) // == 24.7g
	ROMX_LOAD( "dm_08.8b",  0x280000, 0x10000, CRC(bcc0f28c) SHA1(02f587aa4ae71631f27b0e3aaf1829cdded1bdc2) , ROM_SKIP(7) ) // == 10.4b
	ROMX_LOAD( "dm_07.8a",  0x280001, 0x10000, CRC(2a40166a) SHA1(dc4e75d7ed87ae5386d721a09113bba364740465) , ROM_SKIP(7) ) // == 19.7b
	ROMX_LOAD( "dm_16.12b", 0x280002, 0x10000, CRC(20f85c03) SHA1(86385139a9b42270aade758bfe338525936f5671) , ROM_SKIP(7) ) // == 14.4f
	ROMX_LOAD( "dm_15.12a", 0x280003, 0x10000, CRC(8426144b) SHA1(2dbf9625413b302fcdad5bef8733a9dfbfaead52) , ROM_SKIP(7) ) // == 23.7f
	ROMX_LOAD( "dm_24.9e",  0x280004, 0x10000, CRC(da088d61) SHA1(67229eff2827a42af97a60ceb252e132e7f307bc) , ROM_SKIP(7) ) // == 12.4d
	ROMX_LOAD( "dm_23.9c",  0x280005, 0x10000, CRC(17e11df0) SHA1(42fb15e9300b07fc5f4bc21744484869859b130c) , ROM_SKIP(7) ) // == 21.7d
	ROMX_LOAD( "dm_28.11e", 0x280006, 0x10000, CRC(f187ba1c) SHA1(6d9441d04ecef2a9d9c7a2cc7781acd7904c2061) , ROM_SKIP(7) ) // == 16.4h
	ROMX_LOAD( "dm_27.11c", 0x280007, 0x10000, CRC(29f79c78) SHA1(26000a58454a06c3016f99ebc3a79c52911a7070) , ROM_SKIP(7) ) // == 25.7h

	ROM_REGION( 0x18000, "audiocpu", 0 )
	ROM_LOAD( "37.13c",     0x00000, 0x08000, CRC(3692f6e5) SHA1(61b8438d60a39b4cf5062dff0a53228e8a4e4b5f) )    // == 26.10a
	ROM_CONTINUE(           0x10000, 0x08000 )

	ROM_REGION( 0x40000, "oki", ROMREGION_ERASEFF ) /* Samples (not present) */

	ROM_REGION( 0x0200, "aboardplds", 0 )
	ROM_LOAD( "buf1",         0x0000, 0x0117, CRC(eb122de7) SHA1(b26b5bfe258e3e184f069719f9fd008d6b8f6b9b) )
	ROM_LOAD( "ioa1",         0x0000, 0x0117, CRC(59c7ee3b) SHA1(fbb887c5b4f5cb8df77cec710eaac2985bc482a6) )
	ROM_LOAD( "prg1",         0x0000, 0x0117, CRC(f1129744) SHA1(a5300f301c1a08a7da768f0773fa0fe3f683b237) )
	ROM_LOAD( "rom1",         0x0000, 0x0117, CRC(41dc73b9) SHA1(7d4c9f1693c821fbf84e32dd6ef62ddf14967845) )
	ROM_LOAD( "sou1",         0x0000, 0x0117, CRC(84f4b2fe) SHA1(dcc9e86cc36316fe42eace02d6df75d08bc8bb6d) )

	ROM_REGION( 0x0200, "bboardplds", 0 )
	ROM_LOAD( "dm22a.1a",     0x0000, 0x0117, NO_DUMP )
	ROM_LOAD( "lwio.12c",     0x0000, 0x0117, CRC(ad52b90c) SHA1(f0fd6aeea515ee449320fe15684e6b3ab7f97bf4) )
ROM_END

/* B-Board 91634B-2, Japan Resale Ver. */
ROM_START( daimakair )
	ROM_REGION( CODE_SIZE, "maincpu", 0 )      /* 68000 code */
	ROM_LOAD16_WORD_SWAP( "damj_23.8f", 0x00000, 0x80000, CRC(c3b248ec) SHA1(5c016d2dcf882b2a9564e3c4502a0f51ee3d1803) )
	ROM_LOAD16_WORD_SWAP( "damj_22.7f", 0x80000, 0x80000, CRC(595ff2f3) SHA1(ac14b81e15f2c340526a03acbb4c28181d94d5b9) )

	ROM_REGION( 0x400000, "gfx", 0 )
	ROMX_LOAD( "dam_01.3a",  0x000000, 0x80000, CRC(0ba9c0b0) SHA1(c4945b603115f32b7346d72426571dc2d361159f) , ROM_GROUPWORD | ROM_SKIP(6) )    // == dm-05.3a
	ROMX_LOAD( "dam_02.4a",  0x000002, 0x80000, CRC(5d760ab9) SHA1(212176947933fcfef991bc80ad5bd91718689ffe) , ROM_GROUPWORD | ROM_SKIP(6) )    // == dm-07.3f
	ROMX_LOAD( "dam_03.5a",  0x000004, 0x80000, CRC(4ba90b59) SHA1(35bc9dec5ddbf064c30c951627581c16764456ac) , ROM_GROUPWORD | ROM_SKIP(6) )    // == dm-06.3c
	ROMX_LOAD( "dam_04.6a",  0x000006, 0x80000, CRC(4bdee9de) SHA1(7d0c4736f16577afe9966447a18f039728f6fbdf) , ROM_GROUPWORD | ROM_SKIP(6) )    // == dm-08.3g
	ROMX_LOAD( "dam_05.7a",  0x200000, 0x80000, CRC(7dc61b94) SHA1(7796bae7555c541b3c80aacfa24788aeb2ccdfd5) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "dam_06.8a",  0x200002, 0x80000, CRC(fde89758) SHA1(9a6192f629cd1e74e225ef7426338c2816c6b977) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "dam_07.9a",  0x200004, 0x80000, CRC(ec351d78) SHA1(1005a83be4b5577612143ae7f64ca4a08aae7959) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "dam_08.10a", 0x200006, 0x80000, CRC(ee2acc1e) SHA1(4628a9b2447266349d97132003992a21e2bb423a) , ROM_GROUPWORD | ROM_SKIP(6) )

	ROM_REGION( 0x28000, "audiocpu", 0 ) /* 64k for the audio CPU (+banks) */
	ROM_LOAD( "dam_09.12a",  0x00000, 0x08000, CRC(0656ff53) SHA1(063a8124dbe73d014b11f72007f1b877afd1a661) )   // == 26.10a + garbage
	ROM_CONTINUE(            0x10000, 0x18000 ) // second half of ROM is unused, not mapped in memory

	ROM_REGION( 0x40000, "oki", ROMREGION_ERASEFF ) /* Samples (not present) */

	ROM_REGION( 0x0200, "aboardplds", 0 )
	ROM_LOAD( "buf1",         0x0000, 0x0117, CRC(eb122de7) SHA1(b26b5bfe258e3e184f069719f9fd008d6b8f6b9b) )
	ROM_LOAD( "ioa1",         0x0000, 0x0117, CRC(59c7ee3b) SHA1(fbb887c5b4f5cb8df77cec710eaac2985bc482a6) )
	ROM_LOAD( "prg1",         0x0000, 0x0117, CRC(f1129744) SHA1(a5300f301c1a08a7da768f0773fa0fe3f683b237) )
	ROM_LOAD( "rom1",         0x0000, 0x0117, CRC(41dc73b9) SHA1(7d4c9f1693c821fbf84e32dd6ef62ddf14967845) )
	ROM_LOAD( "sou1",         0x0000, 0x0117, CRC(84f4b2fe) SHA1(dcc9e86cc36316fe42eace02d6df75d08bc8bb6d) )

	ROM_REGION( 0x0200, "bboardplds", 0 )
	ROM_LOAD( "dam63b.1a",    0x0000, 0x0117, CRC(474b3c8a) SHA1(da364581685067fc955ed43b982a7aa7a2648286) )
	ROM_LOAD( "iob1.12d",     0x0000, 0x0117, CRC(3abc0700) SHA1(973043aa46ec6d5d1db20dc9d5937005a0f9f6ae) )
	ROM_LOAD( "bprg1.11d",    0x0000, 0x0117, CRC(31793da7) SHA1(400fa7ac517421c978c1ee7773c30b9ed0c5d3f3) )

	ROM_REGION( 0x0200, "cboardplds", 0 )
	ROM_LOAD( "ioc1.ic7",     0x0000, 0x0117, CRC(0d182081) SHA1(475b3d417785da4bc512cce2b274bb00d4cc6792) )
	ROM_LOAD( "c632.ic1",     0x0000, 0x0117, CRC(0fbd9270) SHA1(d7e737b20c44d41e29ca94be56114b31934dde81) )
ROM_END

/* B-Board 89624B-2 */
ROM_START( strider )
	ROM_REGION( CODE_SIZE, "maincpu", 0 )      /* 68000 code */
	ROM_LOAD16_BYTE( "30.11f",        0x00000, 0x20000, CRC(da997474) SHA1(3e4ac98f9a6967d61899281b31c7de779723397b) )
	ROM_LOAD16_BYTE( "35.11h",        0x00001, 0x20000, CRC(5463aaa3) SHA1(e2d07ec2d818e9a2e2d7a77ff0309ae4011c0083) )
	ROM_LOAD16_BYTE( "31.12f",        0x40000, 0x20000, CRC(d20786db) SHA1(c9c75488e6bb37cfd0d56073faf87ff5713bc9a0) )
	ROM_LOAD16_BYTE( "36.12h",        0x40001, 0x20000, CRC(21aa2863) SHA1(446dc9280630318deb423531210a4eedfb4adfa6) )
	ROM_LOAD16_WORD_SWAP( "st-14.8h", 0x80000, 0x80000, CRC(9b3cfc08) SHA1(a7d7f270a097437affa845d80bed82a1fa874878) )  // in "32" socket

	ROM_REGION( 0x400000, "gfx", 0 )
	ROMX_LOAD( "st-2.8a",   0x000000, 0x80000, CRC(4eee9aea) SHA1(5e619fd5f3f1181e32a8fd9dbb4661d74ff8a484) , ROM_GROUPWORD | ROM_SKIP(6) ) // in "6" socket
	ROMX_LOAD( "st-11.10a", 0x000002, 0x80000, CRC(2d7f21e4) SHA1(593cec513de40ff802084d54313bb25a4561e25d) , ROM_GROUPWORD | ROM_SKIP(6) ) // in "8" socket
	ROMX_LOAD( "st-5.4a",   0x000004, 0x80000, CRC(7705aa46) SHA1(6cbfa30b2852fd117d117beefba434ce41d24c2f) , ROM_GROUPWORD | ROM_SKIP(6) ) // in "2" socket
	ROMX_LOAD( "st-9.6a",   0x000006, 0x80000, CRC(5b18b722) SHA1(cf71c62348ca6b404279e87a6686cb3a842eb381) , ROM_GROUPWORD | ROM_SKIP(6) ) // in "4" socket
	ROMX_LOAD( "st-1.7a",   0x200000, 0x80000, CRC(005f000b) SHA1(e6f65af7cc3295be9efaaded352e7ae6320b4133) , ROM_GROUPWORD | ROM_SKIP(6) ) // in "5" socket
	ROMX_LOAD( "st-10.9a",  0x200002, 0x80000, CRC(b9441519) SHA1(bb0926dc484dae4f64c5e5a6bce20afdc7aeba55) , ROM_GROUPWORD | ROM_SKIP(6) ) // in "7" socket
	ROMX_LOAD( "st-4.3a",   0x200004, 0x80000, CRC(b7d04e8b) SHA1(5c5a079baa694927c33d0e0c23e5ff09d6c9d985) , ROM_GROUPWORD | ROM_SKIP(6) ) // in "1" socket
	ROMX_LOAD( "st-8.5a",   0x200006, 0x80000, CRC(6b4713b4) SHA1(759b8b1fc7a5c4b00d74a27c2dd11667db44b09e) , ROM_GROUPWORD | ROM_SKIP(6) ) // in "3" socket

	ROM_REGION( 0x8000, "stars", 0 )
	ROM_COPY( "gfx", 0x000000, 0x000000, 0x8000 )

	ROM_REGION( 0x18000, "audiocpu", 0 ) /* 64k for the audio CPU (+banks) */
	ROM_LOAD( "09.12b",     0x00000, 0x08000, CRC(2ed403bc) SHA1(4ce863ea40d789db5a7cfce91d2c7c720deb9be5) )
	ROM_CONTINUE(           0x10000, 0x08000 )

	ROM_REGION( 0x40000, "oki", 0 ) /* Samples */
	ROM_LOAD( "18.11c",     0x00000, 0x20000, CRC(4386bc80) SHA1(fb2b261995aeacfa13e7ee40b1a973dfb178f015) )
	ROM_LOAD( "19.12c",     0x20000, 0x20000, CRC(444536d7) SHA1(a14f5de2f6b5b29ae5161dca1f8c08c566301a91) )

	ROM_REGION( 0x0200, "aboardplds", 0 )
	ROM_LOAD( "buf1",         0x0000, 0x0117, CRC(eb122de7) SHA1(b26b5bfe258e3e184f069719f9fd008d6b8f6b9b) )
	ROM_LOAD( "ioa1",         0x0000, 0x0117, CRC(59c7ee3b) SHA1(fbb887c5b4f5cb8df77cec710eaac2985bc482a6) )
	ROM_LOAD( "prg1",         0x0000, 0x0117, CRC(f1129744) SHA1(a5300f301c1a08a7da768f0773fa0fe3f683b237) )
	ROM_LOAD( "rom1",         0x0000, 0x0117, CRC(41dc73b9) SHA1(7d4c9f1693c821fbf84e32dd6ef62ddf14967845) )
	ROM_LOAD( "sou1",         0x0000, 0x0117, CRC(84f4b2fe) SHA1(dcc9e86cc36316fe42eace02d6df75d08bc8bb6d) )

	ROM_REGION( 0x0200, "bboardplds", 0 )
	ROM_LOAD( "st24m1.1a",    0x0000, 0x0117, CRC(a80d357e) SHA1(4cb79c99c62c8300e694f4cd26f41dab7818f17f) )
	ROM_LOAD( "lwio.11e",     0x0000, 0x0117, CRC(ad52b90c) SHA1(f0fd6aeea515ee449320fe15684e6b3ab7f97bf4) )
ROM_END

/* B-Board 89624B-3 */
ROM_START( striderua )
	ROM_REGION( CODE_SIZE, "maincpu", 0 )      /* 68000 code */
	ROM_LOAD16_BYTE( "30.11f", 0x00000, 0x20000, CRC(66aec273) SHA1(576b1e9062874e68d68f8725949c151509eb6d56) )    /* different CRC from strider, pcb verified */
	ROM_LOAD16_BYTE( "35.11h", 0x00001, 0x20000, CRC(50e0e865) SHA1(201ef385c228c124ed9412002233a501ea514efd) )    /* different CRC from strider, pcb verified */
	ROM_LOAD16_BYTE( "31.12f", 0x40000, 0x20000, CRC(eae93bd1) SHA1(b320a00b67ea3c7fffc6c37d57863163975f7b80) )    /* different CRC from strider, pcb verified */
	ROM_LOAD16_BYTE( "36.12h", 0x40001, 0x20000, CRC(b904a31d) SHA1(5509d1024151eb8548fd1b29e6c0c95775c61364) )    /* different CRC from strider, pcb verified */
	ROM_LOAD16_WORD_SWAP( "st-14.8h", 0x80000, 0x80000, CRC(9b3cfc08) SHA1(a7d7f270a097437affa845d80bed82a1fa874878) )    // in "32" socket

	ROM_REGION( 0x400000, "gfx", 0 )
	ROMX_LOAD( "st-2.8a",   0x000000, 0x80000, CRC(4eee9aea) SHA1(5e619fd5f3f1181e32a8fd9dbb4661d74ff8a484) , ROM_GROUPWORD | ROM_SKIP(6) ) // in "6" socket
	ROMX_LOAD( "st-11.10a", 0x000002, 0x80000, CRC(2d7f21e4) SHA1(593cec513de40ff802084d54313bb25a4561e25d) , ROM_GROUPWORD | ROM_SKIP(6) ) // in "8" socket
	ROMX_LOAD( "st-5.4a",   0x000004, 0x80000, CRC(7705aa46) SHA1(6cbfa30b2852fd117d117beefba434ce41d24c2f) , ROM_GROUPWORD | ROM_SKIP(6) ) // in "2" socket
	ROMX_LOAD( "st-9.6a",   0x000006, 0x80000, CRC(5b18b722) SHA1(cf71c62348ca6b404279e87a6686cb3a842eb381) , ROM_GROUPWORD | ROM_SKIP(6) ) // in "4" socket
	ROMX_LOAD( "st-1.7a",   0x200000, 0x80000, CRC(005f000b) SHA1(e6f65af7cc3295be9efaaded352e7ae6320b4133) , ROM_GROUPWORD | ROM_SKIP(6) ) // in "5" socket
	ROMX_LOAD( "st-10.9a",  0x200002, 0x80000, CRC(b9441519) SHA1(bb0926dc484dae4f64c5e5a6bce20afdc7aeba55) , ROM_GROUPWORD | ROM_SKIP(6) ) // in "7" socket
	ROMX_LOAD( "st-4.3a",   0x200004, 0x80000, CRC(b7d04e8b) SHA1(5c5a079baa694927c33d0e0c23e5ff09d6c9d985) , ROM_GROUPWORD | ROM_SKIP(6) ) // in "1" socket
	ROMX_LOAD( "st-8.5a",   0x200006, 0x80000, CRC(6b4713b4) SHA1(759b8b1fc7a5c4b00d74a27c2dd11667db44b09e) , ROM_GROUPWORD | ROM_SKIP(6) ) // in "3" socket

	ROM_REGION( 0x8000, "stars", 0 )
	ROM_COPY( "gfx", 0x000000, 0x000000, 0x8000 )

	ROM_REGION( 0x18000, "audiocpu", 0 ) /* 64k for the audio CPU (+banks) */
	ROM_LOAD( "09.12b", 0x00000, 0x08000, CRC(08d63519) SHA1(c120ecfe25c3c50bc51bc7d5a9ef1c8ca6591240) )   /* different CRC from strider, pcb verified */
	ROM_CONTINUE(       0x10000, 0x08000 )

	ROM_REGION( 0x40000, "oki", 0 ) /* Samples */
	ROM_LOAD( "18.11c",     0x00000, 0x20000, CRC(4386bc80) SHA1(fb2b261995aeacfa13e7ee40b1a973dfb178f015) )
	ROM_LOAD( "19.12c",     0x20000, 0x20000, CRC(444536d7) SHA1(a14f5de2f6b5b29ae5161dca1f8c08c566301a91) )

	ROM_REGION( 0x0200, "aboardplds", 0 )
	ROM_LOAD( "buf1",         0x0000, 0x0117, CRC(eb122de7) SHA1(b26b5bfe258e3e184f069719f9fd008d6b8f6b9b) )
	ROM_LOAD( "ioa1",         0x0000, 0x0117, CRC(59c7ee3b) SHA1(fbb887c5b4f5cb8df77cec710eaac2985bc482a6) )
	ROM_LOAD( "prg1",         0x0000, 0x0117, CRC(f1129744) SHA1(a5300f301c1a08a7da768f0773fa0fe3f683b237) )
	ROM_LOAD( "rom1",         0x0000, 0x0117, CRC(41dc73b9) SHA1(7d4c9f1693c821fbf84e32dd6ef62ddf14967845) )
	ROM_LOAD( "sou1",         0x0000, 0x0117, CRC(84f4b2fe) SHA1(dcc9e86cc36316fe42eace02d6df75d08bc8bb6d) )

	ROM_REGION( 0x0200, "bboardplds", 0 )
	ROM_LOAD( "st24b2.1a",    0x0000, 0x0117, NO_DUMP )
	ROM_LOAD( "lwio.11e",     0x0000, 0x0117, CRC(ad52b90c) SHA1(f0fd6aeea515ee449320fe15684e6b3ab7f97bf4) )
ROM_END

/* B-Board 88622B-3 */
ROM_START( striderj )
	ROM_REGION( CODE_SIZE, "maincpu", 0 )      /* 68000 code */
	ROM_LOAD16_BYTE( "sth_36.12f", 0x00000, 0x20000, CRC(53c7b006) SHA1(30daa256a32b209b907e5d916a82068017862a01) )
	ROM_LOAD16_BYTE( "sth_42.12h", 0x00001, 0x20000, CRC(4037f65f) SHA1(490b9fb15f80772316101ea15e61ab32f42feaec) )
	ROM_LOAD16_BYTE( "sth_37.13f", 0x40000, 0x20000, CRC(80e8877d) SHA1(806a62c03007efe6d58fb24dac467d4fc39bb96a) )
	ROM_LOAD16_BYTE( "sth_43.13h", 0x40001, 0x20000, CRC(6b3fa466) SHA1(6a3c9bd491eecf864971f7fdf02d01112d5ef7dd) )
	ROM_LOAD16_BYTE( "sth_34.10f", 0x80000, 0x20000, CRC(bea770b5) SHA1(b1d3111c8878708b6d0589d6bdfd3b380842d98b) ) // == st-14.8h
	ROM_LOAD16_BYTE( "sth_40.10h", 0x80001, 0x20000, CRC(43b922dc) SHA1(441c932080ae2b19e3834e7173d46be2e8762119) ) // == st-14.8h
	ROM_LOAD16_BYTE( "sth_35.11f", 0xc0000, 0x20000, CRC(5cc429da) SHA1(1d3593444d556fcb7b209ef254b7733cb32dc502) ) // == st-14.8h
	ROM_LOAD16_BYTE( "sth_41.11h", 0xc0001, 0x20000, CRC(50af457f) SHA1(fb7248e41c09f137c929a2bd9ef17591f48b7009) ) // == st-14.8h

	ROM_REGION( 0x400000, "gfx", 0 )
	ROMX_LOAD( "sth_09.4b",  0x000000, 0x20000, CRC(1ef6bfbd) SHA1(5e66168e2526b4b4569d9dabacd5602181b23874) , ROM_SKIP(7) )    // == st-2.8a
	ROMX_LOAD( "sth_01.4a",  0x000001, 0x20000, CRC(1e21b0c1) SHA1(2ac543dccefa7e2438d7eb53cdf2e6aff09c735b) , ROM_SKIP(7) )    // == st-2.8a
	ROMX_LOAD( "sth_13.9b",  0x000002, 0x20000, CRC(063263ae) SHA1(38ea256f9a0f3fd6786d21c8b540030b73742971) , ROM_SKIP(7) )    // == st-11.10a
	ROMX_LOAD( "sth_05.9a",  0x000003, 0x20000, CRC(ec9f8714) SHA1(8eb2b92ad576849b8049f9c90b850f74c2a5cba2) , ROM_SKIP(7) )    // == st-11.10a
	ROMX_LOAD( "sth_24.5e",  0x000004, 0x20000, CRC(6356f4d2) SHA1(8d6786e07dba3e59a609c1a13be8685e4fdb8879) , ROM_SKIP(7) )    // == st-5.4a
	ROMX_LOAD( "sth_17.5c",  0x000005, 0x20000, CRC(b4f73d86) SHA1(0ffbcc93ce7eadfd29255a15bc5cbd6e24f98759) , ROM_SKIP(7) )    // == st-5.4a
	ROMX_LOAD( "sth_38.8h",  0x000006, 0x20000, CRC(ee5abfc2) SHA1(1c162bc09991f1082b4d9d22fbce13f2c08e0ceb) , ROM_SKIP(7) )    // == st-9.6a
	ROMX_LOAD( "sth_32.8f",  0x000007, 0x20000, CRC(44a206a3) SHA1(67ec4b9cf3ff181c8c4c4751a1d2e8ee8e56278a) , ROM_SKIP(7) )    // == st-9.6a
	ROMX_LOAD( "sth_10.5b",  0x100000, 0x20000, CRC(df3dd3bc) SHA1(648218f94ecda25873103ac4e2d7132c79f1c5b2) , ROM_SKIP(7) )    // == st-2.8a
	ROMX_LOAD( "sth_02.5a",  0x100001, 0x20000, CRC(c75f9ea0) SHA1(e9268bfa6f5254935fda726ab4b5d9acb0f1942a) , ROM_SKIP(7) )    // == st-2.8a
	ROMX_LOAD( "sth_14.10b", 0x100002, 0x20000, CRC(6c03e19d) SHA1(83f892df551ea79534288643b07613a3c595d526) , ROM_SKIP(7) )    // == st-11.10a
	ROMX_LOAD( "sth_06.10a", 0x100003, 0x20000, CRC(d84f5478) SHA1(c3812056ff2563a43d11746ec8327700f370a053) , ROM_SKIP(7) )    // == st-11.10a
	ROMX_LOAD( "sth_25.7e",  0x100004, 0x20000, CRC(921e506a) SHA1(fd2b5e2121c2adedcb0325b4159273506dab27e8) , ROM_SKIP(7) )    // == st-5.4a
	ROMX_LOAD( "sth_18.7c",  0x100005, 0x20000, CRC(5b318956) SHA1(b1415472bf45787219b3b7680057910b210d57f5) , ROM_SKIP(7) )    // == st-5.4a
	ROMX_LOAD( "sth_39.9h",  0x100006, 0x20000, CRC(9321d6aa) SHA1(acec3c880d29692cf010540c120b9092d7dd8ce9) , ROM_SKIP(7) )    // == st-9.6a
	ROMX_LOAD( "sth_33.9f",  0x100007, 0x20000, CRC(b47ddfc7) SHA1(12388c37abdde85d63305f86244fc3c07f8b6b0c) , ROM_SKIP(7) )    // == st-9.6a
	ROMX_LOAD( "sth_11.7b",  0x200000, 0x20000, CRC(2484f241) SHA1(28c48526ec2577119cc3207e92138749124b5959) , ROM_SKIP(7) )    // == st-1.7a
	ROMX_LOAD( "sth_03.7a",  0x200001, 0x20000, CRC(aaa07245) SHA1(64a1b75b7613c1949eee6f9ba865dbdd7ec34413) , ROM_SKIP(7) )    // == st-1.7a
	ROMX_LOAD( "sth_15.11b", 0x200002, 0x20000, CRC(e415d943) SHA1(12069d02d3a6afa9241222b48420daaf97874271) , ROM_SKIP(7) )    // == st-10.9a
	ROMX_LOAD( "sth_07.11a", 0x200003, 0x20000, CRC(97d072d2) SHA1(fb0e10464a878ec6c0f3e6c6ddb0ea542bfb87a8) , ROM_SKIP(7) )    // == st-10.9a
	ROMX_LOAD( "sth_26.8e",  0x200004, 0x20000, CRC(0ebfcb02) SHA1(a7238e1c76dbc2de1b7ae0d2cc532170cd1ab6c2) , ROM_SKIP(7) )    // == st-4.3a
	ROMX_LOAD( "sth_19.8c",  0x200005, 0x20000, CRC(257ce683) SHA1(762f22b5ba24864d69dda303310a310d8dbfcc1c) , ROM_SKIP(7) )    // == st-4.3a
	ROMX_LOAD( "sth_28.10e", 0x200006, 0x20000, CRC(98ac8cd1) SHA1(53dbe418d5cb7af5ef4be91e5e6bcd4474d2fdfe) , ROM_SKIP(7) )    // == st-8.5a
	ROMX_LOAD( "sth_21.10c", 0x200007, 0x20000, CRC(538d9423) SHA1(418ea54d6582723d3e568364787862a6df2d1523) , ROM_SKIP(7) )    // == st-8.5a
	ROMX_LOAD( "sth_12.8b",  0x300000, 0x20000, CRC(f670a477) SHA1(de5154ca093a9e5f9adb836d9a933d14e939180d) , ROM_SKIP(7) )    // == st-1.7a
	ROMX_LOAD( "sth_04.8a",  0x300001, 0x20000, CRC(853d3e01) SHA1(422cc9f539e79c2a9b3bda47eb1fc714d79838d1) , ROM_SKIP(7) )    // == st-1.7a
	ROMX_LOAD( "sth_16.12b", 0x300002, 0x20000, CRC(4092019f) SHA1(2173e72a8325d12da70666bdc279409b23fb7024) , ROM_SKIP(7) )    // == st-10.9a
	ROMX_LOAD( "sth_08.12a", 0x300003, 0x20000, CRC(2ce9b4c7) SHA1(f267d323c9310433852e3308b36100440bee33d7) , ROM_SKIP(7) )    // == st-10.9a
	ROMX_LOAD( "sth_27.9e",  0x300004, 0x20000, CRC(f82c88d9) SHA1(200bd025800eb20c4a15af17e7c3effbfa6f00fa) , ROM_SKIP(7) )    // == st-4.3a
	ROMX_LOAD( "sth_20.9c",  0x300005, 0x20000, CRC(eb584dd4) SHA1(aeee39c0fc9f234249253b14de88a8da494b18d0) , ROM_SKIP(7) )    // == st-4.3a
	ROMX_LOAD( "sth_29.11e", 0x300006, 0x20000, CRC(34ae2997) SHA1(9449eb9c85b7cb4a12aa06cb65a9d849a528e633) , ROM_SKIP(7) )    // == st-8.5a
	ROMX_LOAD( "sth_22.11c", 0x300007, 0x20000, CRC(78dd9c48) SHA1(35fbf3ca21f56c9899283ba08c89c0faf7a8f717) , ROM_SKIP(7) )    // == st-8.5a

	ROM_REGION( 0x8000, "stars", 0 )
	ROM_COPY( "gfx", 0x000000, 0x000000, 0x8000 )

	ROM_REGION( 0x18000, "audiocpu", 0 ) /* 64k for the audio CPU (+banks) */
	ROM_LOAD( "sth_23.13c",  0x00000, 0x08000, CRC(2ed403bc) SHA1(4ce863ea40d789db5a7cfce91d2c7c720deb9be5) )   // == 09.12b
	ROM_CONTINUE(            0x10000, 0x08000 )

	ROM_REGION( 0x40000, "oki", 0 ) /* Samples */
	ROM_LOAD( "sth_30.12e",  0x00000, 0x20000, CRC(4386bc80) SHA1(fb2b261995aeacfa13e7ee40b1a973dfb178f015) )   // == 18.11c
	ROM_LOAD( "sth_31.13e",  0x20000, 0x20000, CRC(444536d7) SHA1(a14f5de2f6b5b29ae5161dca1f8c08c566301a91) )   // == 19.12c

	ROM_REGION( 0x0200, "aboardplds", 0 )
	ROM_LOAD( "buf1",         0x0000, 0x0117, CRC(eb122de7) SHA1(b26b5bfe258e3e184f069719f9fd008d6b8f6b9b) )
	ROM_LOAD( "ioa1",         0x0000, 0x0117, CRC(59c7ee3b) SHA1(fbb887c5b4f5cb8df77cec710eaac2985bc482a6) )
	ROM_LOAD( "prg1",         0x0000, 0x0117, CRC(f1129744) SHA1(a5300f301c1a08a7da768f0773fa0fe3f683b237) )
	ROM_LOAD( "rom1",         0x0000, 0x0117, CRC(41dc73b9) SHA1(7d4c9f1693c821fbf84e32dd6ef62ddf14967845) )
	ROM_LOAD( "sou1",         0x0000, 0x0117, CRC(84f4b2fe) SHA1(dcc9e86cc36316fe42eace02d6df75d08bc8bb6d) )

	ROM_REGION( 0x0200, "bboardplds", 0 )
	ROM_LOAD( "st22b.1a",     0x0000, 0x0117, CRC(68fecc55) SHA1(238f6781f972e834f2c6054cfe925ffec579dec3) )
	ROM_LOAD( "lwio.12c",     0x0000, 0x0117, CRC(ad52b90c) SHA1(f0fd6aeea515ee449320fe15684e6b3ab7f97bf4) )
ROM_END

/* B-Board 91634B-2, Japan Resale Ver. */
ROM_START( striderjr )
	ROM_REGION( CODE_SIZE, "maincpu", 0 )      /* 68000 code */
	ROM_LOAD16_WORD_SWAP( "sthj_23.8f", 0x00000, 0x80000, CRC(046e7b12) SHA1(a5761f730f6844a7e93556a6aeae76240a99540c) )
	ROM_LOAD16_WORD_SWAP( "sthj_22.7f", 0x80000, 0x80000, CRC(9b3cfc08) SHA1(a7d7f270a097437affa845d80bed82a1fa874878) )    // == st-14.8h

	ROM_REGION( 0x400000, "gfx", 0 )
	ROMX_LOAD( "sth_01.3a",  0x000000, 0x80000, CRC(4eee9aea) SHA1(5e619fd5f3f1181e32a8fd9dbb4661d74ff8a484) , ROM_GROUPWORD | ROM_SKIP(6) )    // == st-2.8a
	ROMX_LOAD( "sth_02.4a",  0x000002, 0x80000, CRC(2d7f21e4) SHA1(593cec513de40ff802084d54313bb25a4561e25d) , ROM_GROUPWORD | ROM_SKIP(6) )    // == st-11.10a
	ROMX_LOAD( "sth_03.5a",  0x000004, 0x80000, CRC(7705aa46) SHA1(6cbfa30b2852fd117d117beefba434ce41d24c2f) , ROM_GROUPWORD | ROM_SKIP(6) )    // == st-5.4a
	ROMX_LOAD( "sth_04.6a",  0x000006, 0x80000, CRC(5b18b722) SHA1(cf71c62348ca6b404279e87a6686cb3a842eb381) , ROM_GROUPWORD | ROM_SKIP(6) )    // == st-9.6a
	ROMX_LOAD( "sth_05.7a",  0x200000, 0x80000, CRC(005f000b) SHA1(e6f65af7cc3295be9efaaded352e7ae6320b4133) , ROM_GROUPWORD | ROM_SKIP(6) )    // == st-1.7a
	ROMX_LOAD( "sth_06.8a",  0x200002, 0x80000, CRC(b9441519) SHA1(bb0926dc484dae4f64c5e5a6bce20afdc7aeba55) , ROM_GROUPWORD | ROM_SKIP(6) )    // == st-10.9a
	ROMX_LOAD( "sth_07.9a",  0x200004, 0x80000, CRC(b7d04e8b) SHA1(5c5a079baa694927c33d0e0c23e5ff09d6c9d985) , ROM_GROUPWORD | ROM_SKIP(6) )    // == st-4.3a
	ROMX_LOAD( "sth_08.10a", 0x200006, 0x80000, CRC(6b4713b4) SHA1(759b8b1fc7a5c4b00d74a27c2dd11667db44b09e) , ROM_GROUPWORD | ROM_SKIP(6) )    // == st-8.5a

	ROM_REGION( 0x8000, "stars", 0 )
	ROM_COPY( "gfx", 0x000000, 0x000000, 0x8000 )

	ROM_REGION( 0x18000, "audiocpu", 0 ) /* 64k for the audio CPU (+banks) */
	ROM_LOAD( "sth_09.12a",  0x00000, 0x08000, CRC(08d63519) SHA1(c120ecfe25c3c50bc51bc7d5a9ef1c8ca6591240) )
	ROM_CONTINUE(            0x10000, 0x08000 )

	ROM_REGION( 0x40000, "oki", 0 ) /* Samples */
	ROM_LOAD( "sth_18.11c",  0x00000, 0x20000, CRC(4386bc80) SHA1(fb2b261995aeacfa13e7ee40b1a973dfb178f015) )
	ROM_LOAD( "sth_19.12c",  0x20000, 0x20000, CRC(444536d7) SHA1(a14f5de2f6b5b29ae5161dca1f8c08c566301a91) )

	ROM_REGION( 0x0200, "aboardplds", 0 )
	ROM_LOAD( "buf1",         0x0000, 0x0117, CRC(eb122de7) SHA1(b26b5bfe258e3e184f069719f9fd008d6b8f6b9b) )
	ROM_LOAD( "ioa1",         0x0000, 0x0117, CRC(59c7ee3b) SHA1(fbb887c5b4f5cb8df77cec710eaac2985bc482a6) )
	ROM_LOAD( "prg1",         0x0000, 0x0117, CRC(f1129744) SHA1(a5300f301c1a08a7da768f0773fa0fe3f683b237) )
	ROM_LOAD( "rom1",         0x0000, 0x0117, CRC(41dc73b9) SHA1(7d4c9f1693c821fbf84e32dd6ef62ddf14967845) )
	ROM_LOAD( "sou1",         0x0000, 0x0117, CRC(84f4b2fe) SHA1(dcc9e86cc36316fe42eace02d6df75d08bc8bb6d) )

	ROM_REGION( 0x0200, "bboardplds", 0 )
	ROM_LOAD( "sth63b.1a",    0x0000, 0x0117, BAD_DUMP CRC(c706b773) SHA1(ddfe2e747637eec081a5125cfefcb478a4ba9e76) ) /* Handcrafted but works on actual PCB.  Redump needed */
	ROM_LOAD( "iob1.12d",     0x0000, 0x0117, CRC(3abc0700) SHA1(973043aa46ec6d5d1db20dc9d5937005a0f9f6ae) )
	ROM_LOAD( "bprg1.11d",    0x0000, 0x0117, CRC(31793da7) SHA1(400fa7ac517421c978c1ee7773c30b9ed0c5d3f3) )

	ROM_REGION( 0x0200, "cboardplds", 0 )
	ROM_LOAD( "ioc1.ic7",     0x0000, 0x0117, CRC(0d182081) SHA1(475b3d417785da4bc512cce2b274bb00d4cc6792) )
	ROM_LOAD( "c632.ic1",     0x0000, 0x0117, CRC(0fbd9270) SHA1(d7e737b20c44d41e29ca94be56114b31934dde81) )
ROM_END

/* B-Board 89624B-? */
ROM_START( dynwar )
	ROM_REGION( CODE_SIZE, "maincpu", 0 )      /* 68000 code */
	ROM_LOAD16_BYTE( "30.11f",        0x00000, 0x20000, CRC(f9ec6d68) SHA1(02912db2b48f77489b0b841c2a5414bfb49b93f4) )
	ROM_LOAD16_BYTE( "35.11h",        0x00001, 0x20000, CRC(e41fff2f) SHA1(a960c39c69f97b46d5efcbcd3e2bc652888094c4) )
	ROM_LOAD16_BYTE( "31.12f",        0x40000, 0x20000, CRC(e3de76ff) SHA1(fdc552312e10c91dd00bfa72e4e686ac356d2244) )
	ROM_LOAD16_BYTE( "36.12h",        0x40001, 0x20000, CRC(7a13cfbf) SHA1(c6b4d775a2e507fdefbb895cc75bb5bdb442218d) )
	ROM_LOAD16_WORD_SWAP( "tkm-9.8h", 0x80000, 0x80000, CRC(93654bcf) SHA1(c72daeb2a98d350568555059a3225343c219a1d2) )  // in "32" socket

	ROM_REGION( 0x400000, "gfx", 0 )
	ROMX_LOAD( "tkm-5.7a",  0x000000, 0x80000, CRC(f64bb6a0) SHA1(8c0ae0305704ad876bb1938e46550c68b0de8e8e), ROM_GROUPWORD | ROM_SKIP(6) )  // in "5" socket
	ROMX_LOAD( "tkm-8.9a",  0x000002, 0x80000, CRC(21fe6274) SHA1(f92e509d88d5e264be9c7812966d64ad9ac518e7), ROM_GROUPWORD | ROM_SKIP(6) )  // in "7" socket
	ROMX_LOAD( "tkm-6.3a",  0x000004, 0x80000, CRC(0bf228cb) SHA1(e72957155cb459c4dee50df2e53256f271528964), ROM_GROUPWORD | ROM_SKIP(6) )  // in "1" socket
	ROMX_LOAD( "tkm-7.5a",  0x000006, 0x80000, CRC(1255dfb1) SHA1(c943e3c989d5b20fbe24e38e54ee8ca294b3d182), ROM_GROUPWORD | ROM_SKIP(6) )  // in "3" socket
	ROMX_LOAD( "tkm-1.8a",  0x200000, 0x80000, CRC(44f7661e) SHA1(f29b5ad0c5dfd91a56a4a1084ce578cfe496dd6f), ROM_GROUPWORD | ROM_SKIP(6) )  // in "6" socket
	ROMX_LOAD( "tkm-4.10a", 0x200002, 0x80000, CRC(a54c515d) SHA1(bfa457cef7e29ae56ee9b10f60e233d82b4efc61), ROM_GROUPWORD | ROM_SKIP(6) )  // in "8" socket
	ROMX_LOAD( "tkm-2.4a",  0x200004, 0x80000, CRC(ca5c687c) SHA1(de47cb5a071ffb3ff408f60d45b79345032232a7), ROM_GROUPWORD | ROM_SKIP(6) )  // in "2" socket
	ROMX_LOAD( "tkm-3.6a",  0x200006, 0x80000, CRC(f9fe6591) SHA1(260da5f9e305cccd621b8b5b2073c79e161ddeb0), ROM_GROUPWORD | ROM_SKIP(6) )  // in "4" socket

	ROM_REGION( 0x18000, "audiocpu", 0 ) /* 64k for the audio CPU (+banks) */
	ROM_LOAD( "tke_17.12b", 0x00000, 0x08000, CRC(b3b79d4f) SHA1(2b960545741d3b9a53ffbf3ed83030392aa02698) )    // in "9" socket
	ROM_CONTINUE(           0x10000, 0x08000 )

	ROM_REGION( 0x40000, "oki", 0 ) /* Samples */
	ROM_LOAD( "tke_18.11c", 0x00000, 0x20000, CRC(ac6e307d) SHA1(b490ce625bb7ce0904b0fd121fbfbd5252790f7a) )
	ROM_LOAD( "tke_19.12c", 0x20000, 0x20000, CRC(068741db) SHA1(ab48aff639a7ac218b7d5304145e10e92d61fd9f) )

	ROM_REGION( 0x0200, "aboardplds", 0 )
	ROM_LOAD( "buf1",         0x0000, 0x0117, CRC(eb122de7) SHA1(b26b5bfe258e3e184f069719f9fd008d6b8f6b9b) )
	ROM_LOAD( "ioa1",         0x0000, 0x0117, CRC(59c7ee3b) SHA1(fbb887c5b4f5cb8df77cec710eaac2985bc482a6) )
	ROM_LOAD( "prg1",         0x0000, 0x0117, CRC(f1129744) SHA1(a5300f301c1a08a7da768f0773fa0fe3f683b237) )
	ROM_LOAD( "rom1",         0x0000, 0x0117, CRC(41dc73b9) SHA1(7d4c9f1693c821fbf84e32dd6ef62ddf14967845) )
	ROM_LOAD( "sou1",         0x0000, 0x0117, CRC(84f4b2fe) SHA1(dcc9e86cc36316fe42eace02d6df75d08bc8bb6d) )

	ROM_REGION( 0x0200, "bboardplds", 0 )
	ROM_LOAD( "tk24b1.1a",    0x0000, 0x0117, CRC(ae4a7645) SHA1(593478a177ea1794c9dfff07c6c6d8624a883eae) )
	ROM_LOAD( "lwio.11e",     0x0000, 0x0117, CRC(ad52b90c) SHA1(f0fd6aeea515ee449320fe15684e6b3ab7f97bf4) )    // pal verification required
ROM_END

/* B-Board 88622B-3 */
ROM_START( dynwara )
	ROM_REGION( CODE_SIZE, "maincpu", 0 )      /* 68000 code */
	ROM_LOAD16_BYTE( "tke_36.12f", 0x00000, 0x20000, CRC(895991d1) SHA1(56b105b85ccab1c49c89ae8d4aa55c9374077df0) )
	ROM_LOAD16_BYTE( "tke_42.12h", 0x00001, 0x20000, CRC(c898d2e8) SHA1(c8b10685681bf155ea44e30f3cb0574df7d4f984) )
	ROM_LOAD16_BYTE( "tke_37.13f", 0x40000, 0x20000, CRC(b228d58c) SHA1(99a1f42d930f788c4f9b410addc95173fda801a2) )
	ROM_LOAD16_BYTE( "tke_43.13h", 0x40001, 0x20000, CRC(1a14375a) SHA1(2b1a62f7961dceabf98461266da37abfec13aaa9) )
	ROM_LOAD16_BYTE( "34.10f",     0x80000, 0x20000, CRC(8f663d00) SHA1(77811783c87c7aee058b8533e34049a01047258a) ) // == tkm-9.8h
	ROM_LOAD16_BYTE( "40.10h",     0x80001, 0x20000, CRC(1586dbf3) SHA1(d9f03e001effdef021a9ceda512e73a24726fca1) ) // == tkm-9.8h
	ROM_LOAD16_BYTE( "35.11f",     0xc0000, 0x20000, CRC(9db93d7a) SHA1(f75e3fb5273baef0cd5d8eea26f07d5acaa743ca) ) // == tkm-9.8h
	ROM_LOAD16_BYTE( "41.11h",     0xc0001, 0x20000, CRC(1aae69a4) SHA1(56e4761818f7857bc7520f2b8de90eabd857c577) ) // == tkm-9.8h

	ROM_REGION( 0x400000, "gfx", 0 )
	ROMX_LOAD( "09.4b",     0x000000, 0x20000, CRC(c3e83c69) SHA1(bd361a39dc6428fea8f56ebbe5cdcc4bf63a51f0) , ROM_SKIP(7) ) // == tkm-5.7a
	ROMX_LOAD( "01.4a",     0x000001, 0x20000, CRC(187b2886) SHA1(b16121f57926d9fd2c3bc82ae6babf6a2297f140) , ROM_SKIP(7) ) // == tkm-5.7a
	ROMX_LOAD( "13.9b",     0x000002, 0x20000, CRC(0273d87d) SHA1(7803b04d72eedb4c8b39f63fea458cfef0034813) , ROM_SKIP(7) ) // == tkm-8.9a
	ROMX_LOAD( "05.9a",     0x000003, 0x20000, CRC(339378b8) SHA1(c3dfe7039e4572b9ef56166346f3cbc6f6ab07c1) , ROM_SKIP(7) ) // == tkm-8.9a
	ROMX_LOAD( "24.5e",     0x000004, 0x20000, CRC(c6909b6f) SHA1(2828bd6bdc8e3f87a4a37d4e20bdff86cb6850c9) , ROM_SKIP(7) ) // == tkm-6.3a
	ROMX_LOAD( "17.5c",     0x000005, 0x20000, CRC(2e2f8320) SHA1(7bcb80447d9ce7cc9a38e2506196acd6bf50b49f) , ROM_SKIP(7) ) // == tkm-6.3a
	ROMX_LOAD( "38.8h",     0x000006, 0x20000, CRC(cd7923ed) SHA1(29187b99847a4b56f2f1763d086b8e7dc5cebed7) , ROM_SKIP(7) ) // == tkm-7.5a
	ROMX_LOAD( "32.8f",     0x000007, 0x20000, CRC(21a0a453) SHA1(ace38c5943f9f744212cfdb7caa2caa43312e82c) , ROM_SKIP(7) ) // == tkm-7.5a
	ROMX_LOAD( "10.5b",     0x100000, 0x20000, CRC(ff28f8d0) SHA1(c8c4851816f17a4a0494164f5e8cc910f16669e8) , ROM_SKIP(7) ) // == tkm-5.7a
	ROMX_LOAD( "02.5a",     0x100001, 0x20000, CRC(cc83c02f) SHA1(915e9d7acec1ba7a2035ae140f576839eba8694f) , ROM_SKIP(7) ) // == tkm-5.7a
	ROMX_LOAD( "14.10b",    0x100002, 0x20000, CRC(58d9b32f) SHA1(c13a12afcb83159b284b95053951dfa1841bb612) , ROM_SKIP(7) ) // == tkm-8.9a
	ROMX_LOAD( "06.10a",    0x100003, 0x20000, CRC(6f9edd75) SHA1(e8d43c0ec2165e88aefbb5c92048fbcd06fe578b) , ROM_SKIP(7) ) // == tkm-8.9a
	ROMX_LOAD( "25.7e",     0x100004, 0x20000, CRC(152ea74a) SHA1(c0c56b1bdfa0d7fdea040dbcc6ff871e5957a5b6) , ROM_SKIP(7) ) // == tkm-6.3a
	ROMX_LOAD( "18.7c",     0x100005, 0x20000, CRC(1833f932) SHA1(81f94d26bdb6758736ca02d7b1772801be4da181) , ROM_SKIP(7) ) // == tkm-6.3a
	ROMX_LOAD( "39.9h",     0x100006, 0x20000, CRC(bc09b360) SHA1(de2c9a42490db79c8e5fe57b9107f1adbe5dd241) , ROM_SKIP(7) ) // == tkm-7.5a
	ROMX_LOAD( "33.9f",     0x100007, 0x20000, CRC(89de1533) SHA1(e48312e37c0f98faeec91546acde5daf0da8f6b3) , ROM_SKIP(7) ) // == tkm-7.5a
	ROMX_LOAD( "11.7b",     0x200000, 0x20000, CRC(29eaf490) SHA1(42fcb67c7014e0ad62cde9e77c79e61268647528) , ROM_SKIP(7) ) // == tkm-1.8a
	ROMX_LOAD( "03.7a",     0x200001, 0x20000, CRC(7bf51337) SHA1(c21938029641ebcbc484680cf8a57186cdde220f) , ROM_SKIP(7) ) // == tkm-1.8a
	ROMX_LOAD( "15.11b",    0x200002, 0x20000, CRC(d36cdb91) SHA1(66ab873ce285e857f30294dd1c9b1dda0c6c6b76) , ROM_SKIP(7) ) // == tkm-4.10a
	ROMX_LOAD( "07.11a",    0x200003, 0x20000, CRC(e04af054) SHA1(f227b8a0a3d8f41e1922d184eaec7a1243c7c3af) , ROM_SKIP(7) ) // == tkm-4.10a
	ROMX_LOAD( "26.8e",     0x200004, 0x20000, CRC(07fc714b) SHA1(eda97a3c5596ebdfa61bdd01d39647c89b9a2f13) , ROM_SKIP(7) ) // == tkm-2.4a
	ROMX_LOAD( "19.8c",     0x200005, 0x20000, CRC(7114e5c6) SHA1(2f2925b942af50781857f4fe74e9a58f2cf7b883) , ROM_SKIP(7) ) // == tkm-2.4a
	ROMX_LOAD( "28.10e",    0x200006, 0x20000, CRC(af62bf07) SHA1(a6e0f598de1fa8a4960e89d655b7514572ed6310) , ROM_SKIP(7) ) // == tkm-3.6a
	ROMX_LOAD( "21.10c",    0x200007, 0x20000, CRC(523f462a) SHA1(b0fc9e29d6ca44aafb20a62355bde9f4b4cf1e43) , ROM_SKIP(7) ) // == tkm-3.6a
	ROMX_LOAD( "12.8b",     0x300000, 0x20000, CRC(38652339) SHA1(930a035bbe34c81c26d774d2ab45f53a3a9205fb) , ROM_SKIP(7) ) // == tkm-1.8a
	ROMX_LOAD( "04.8a",     0x300001, 0x20000, CRC(4951bc0f) SHA1(07f424c147d787321b668d787216733c35e2cff9) , ROM_SKIP(7) ) // == tkm-1.8a
	ROMX_LOAD( "16.12b",    0x300002, 0x20000, CRC(381608ae) SHA1(666e15e61c7c59df5a97bdc2d77db611d60b3ca8) , ROM_SKIP(7) ) // == tkm-4.10a
	ROMX_LOAD( "08.12a",    0x300003, 0x20000, CRC(b475d4e9) SHA1(dc5d223bc2a27904e6e38b68507d2e87fbbde158) , ROM_SKIP(7) ) // == tkm-4.10a
	ROMX_LOAD( "27.9e",     0x300004, 0x20000, CRC(a27e81fa) SHA1(b25854d4a7e52d500c19445badb4cfe745d88d23) , ROM_SKIP(7) ) // == tkm-2.4a
	ROMX_LOAD( "20.9c",     0x300005, 0x20000, CRC(002796dc) SHA1(2dba0434916dd82c59a66e2f3ce8d3165713c308) , ROM_SKIP(7) ) // == tkm-2.4a
	ROMX_LOAD( "29.11e",    0x300006, 0x20000, CRC(6b41f82d) SHA1(111af606d8107d377e3af618584a75ed6cfc9bbd) , ROM_SKIP(7) ) // == tkm-3.6a
	ROMX_LOAD( "22.11c",    0x300007, 0x20000, CRC(52145369) SHA1(bd422f0c51cdd62b69229f926569ad05d430bd57) , ROM_SKIP(7) ) // == tkm-3.6a

	ROM_REGION( 0x18000, "audiocpu", 0 ) /* 64k for the audio CPU (+banks) */
	ROM_LOAD( "23.13c",     0x00000, 0x08000, CRC(b3b79d4f) SHA1(2b960545741d3b9a53ffbf3ed83030392aa02698) )    // == tke_17.12b
	ROM_CONTINUE(           0x10000, 0x08000 )

	ROM_REGION( 0x40000, "oki", 0 ) /* Samples */
	ROM_LOAD( "tke_30.12e", 0x00000, 0x20000, CRC(ac6e307d) SHA1(b490ce625bb7ce0904b0fd121fbfbd5252790f7a) )    // == tke_18.11c
	ROM_LOAD( "tke_31.13e", 0x20000, 0x20000, CRC(068741db) SHA1(ab48aff639a7ac218b7d5304145e10e92d61fd9f) )    // == tke_19.12c

	ROM_REGION( 0x0200, "aboardplds", 0 )
	ROM_LOAD( "buf1",         0x0000, 0x0117, CRC(eb122de7) SHA1(b26b5bfe258e3e184f069719f9fd008d6b8f6b9b) )
	ROM_LOAD( "ioa1",         0x0000, 0x0117, CRC(59c7ee3b) SHA1(fbb887c5b4f5cb8df77cec710eaac2985bc482a6) )
	ROM_LOAD( "prg1",         0x0000, 0x0117, CRC(f1129744) SHA1(a5300f301c1a08a7da768f0773fa0fe3f683b237) )
	ROM_LOAD( "rom1",         0x0000, 0x0117, CRC(41dc73b9) SHA1(7d4c9f1693c821fbf84e32dd6ef62ddf14967845) )
	ROM_LOAD( "sou1",         0x0000, 0x0117, CRC(84f4b2fe) SHA1(dcc9e86cc36316fe42eace02d6df75d08bc8bb6d) )

	ROM_REGION( 0x0200, "bboardplds", 0 )
	ROM_LOAD( "tk22b.1a",     0x0000, 0x0117, CRC(1a1ab6d7) SHA1(eacd3a21a09683e1d8ad36215c1f8ea96a1f7c12) )
	ROM_LOAD( "lwio.12c",     0x0000, 0x0117, CRC(ad52b90c) SHA1(f0fd6aeea515ee449320fe15684e6b3ab7f97bf4) )
ROM_END

/* B-Board 88622B-3 */
ROM_START( dynwarj )
	ROM_REGION( CODE_SIZE, "maincpu", 0 )      /* 68000 code */
	ROM_LOAD16_BYTE( "36.12f", 0x00000, 0x20000, CRC(1a516657) SHA1(f5c7c3bfd482eb59221cfd3eec4d47e717b04efa) )
	ROM_LOAD16_BYTE( "42.12h", 0x00001, 0x20000, CRC(12a290a0) SHA1(29fd3f77c497ef8db48121301beab2862ca380b4) )
	ROM_LOAD16_BYTE( "37.13f", 0x40000, 0x20000, CRC(932fc943) SHA1(1bd1c696072e61db791c075fae8936dece73d1d8) )
	ROM_LOAD16_BYTE( "43.13h", 0x40001, 0x20000, CRC(872ad76d) SHA1(77cfb380dd358eb9e65894a026e0718918c5b68f) )
	ROM_LOAD16_BYTE( "34.10f", 0x80000, 0x20000, CRC(8f663d00) SHA1(77811783c87c7aee058b8533e34049a01047258a) ) // == tkm-9.8h
	ROM_LOAD16_BYTE( "40.10h", 0x80001, 0x20000, CRC(1586dbf3) SHA1(d9f03e001effdef021a9ceda512e73a24726fca1) ) // == tkm-9.8h
	ROM_LOAD16_BYTE( "35.11f", 0xc0000, 0x20000, CRC(9db93d7a) SHA1(f75e3fb5273baef0cd5d8eea26f07d5acaa743ca) ) // == tkm-9.8h
	ROM_LOAD16_BYTE( "41.11h", 0xc0001, 0x20000, CRC(1aae69a4) SHA1(56e4761818f7857bc7520f2b8de90eabd857c577) ) // == tkm-9.8h

	ROM_REGION( 0x400000, "gfx", 0 )
	ROMX_LOAD( "09.4b",             0x000000, 0x20000, CRC(c3e83c69) SHA1(bd361a39dc6428fea8f56ebbe5cdcc4bf63a51f0) , ROM_SKIP(7) ) // == tkm-5.7a
	ROMX_LOAD( "01.4a",             0x000001, 0x20000, CRC(187b2886) SHA1(b16121f57926d9fd2c3bc82ae6babf6a2297f140) , ROM_SKIP(7) ) // == tkm-5.7a
	ROMX_LOAD( "13.9b",             0x000002, 0x20000, CRC(0273d87d) SHA1(7803b04d72eedb4c8b39f63fea458cfef0034813) , ROM_SKIP(7) ) // == tkm-8.9a
	ROMX_LOAD( "05.9a",             0x000003, 0x20000, CRC(339378b8) SHA1(c3dfe7039e4572b9ef56166346f3cbc6f6ab07c1) , ROM_SKIP(7) ) // == tkm-8.9a
	ROMX_LOAD( "24.5e",             0x000004, 0x20000, CRC(c6909b6f) SHA1(2828bd6bdc8e3f87a4a37d4e20bdff86cb6850c9) , ROM_SKIP(7) ) // == tkm-6.3a
	ROMX_LOAD( "17.5c",             0x000005, 0x20000, CRC(2e2f8320) SHA1(7bcb80447d9ce7cc9a38e2506196acd6bf50b49f) , ROM_SKIP(7) ) // == tkm-6.3a
	ROMX_LOAD( "38.8h",             0x000006, 0x20000, CRC(cd7923ed) SHA1(29187b99847a4b56f2f1763d086b8e7dc5cebed7) , ROM_SKIP(7) ) // == tkm-7.5a
	ROMX_LOAD( "32.8f",             0x000007, 0x20000, CRC(21a0a453) SHA1(ace38c5943f9f744212cfdb7caa2caa43312e82c) , ROM_SKIP(7) ) // == tkm-7.5a
	ROMX_LOAD( "10.5b",             0x100000, 0x20000, CRC(ff28f8d0) SHA1(c8c4851816f17a4a0494164f5e8cc910f16669e8) , ROM_SKIP(7) ) // == tkm-5.7a
	ROMX_LOAD( "02.5a",             0x100001, 0x20000, CRC(cc83c02f) SHA1(915e9d7acec1ba7a2035ae140f576839eba8694f) , ROM_SKIP(7) ) // == tkm-5.7a
	ROMX_LOAD( "14.10b",            0x100002, 0x20000, CRC(18fb232c) SHA1(c690ca668a56c756c04ef5db4900eb3fd34897e7) , ROM_SKIP(7) ) /* 1 byte different from dynwara, pcb verified */
	ROMX_LOAD( "06.10a",            0x100003, 0x20000, CRC(6f9edd75) SHA1(e8d43c0ec2165e88aefbb5c92048fbcd06fe578b) , ROM_SKIP(7) ) // == tkm-8.9a
	ROMX_LOAD( "25.7e",             0x100004, 0x20000, CRC(152ea74a) SHA1(c0c56b1bdfa0d7fdea040dbcc6ff871e5957a5b6) , ROM_SKIP(7) ) // == tkm-6.3a
	ROMX_LOAD( "18.7c",             0x100005, 0x20000, CRC(1833f932) SHA1(81f94d26bdb6758736ca02d7b1772801be4da181) , ROM_SKIP(7) ) // == tkm-6.3a
	ROMX_LOAD( "39.9h",             0x100006, 0x20000, CRC(bc09b360) SHA1(de2c9a42490db79c8e5fe57b9107f1adbe5dd241) , ROM_SKIP(7) ) // == tkm-7.5a
	ROMX_LOAD( "33.9f",             0x100007, 0x20000, CRC(89de1533) SHA1(e48312e37c0f98faeec91546acde5daf0da8f6b3) , ROM_SKIP(7) ) // == tkm-7.5a
	ROMX_LOAD( "11.7b",             0x200000, 0x20000, CRC(29eaf490) SHA1(42fcb67c7014e0ad62cde9e77c79e61268647528) , ROM_SKIP(7) ) // == tkm-1.8a
	ROMX_LOAD( "03.7a",             0x200001, 0x20000, CRC(7bf51337) SHA1(c21938029641ebcbc484680cf8a57186cdde220f) , ROM_SKIP(7) ) // == tkm-1.8a
	ROMX_LOAD( "15.11b",            0x200002, 0x20000, CRC(d36cdb91) SHA1(66ab873ce285e857f30294dd1c9b1dda0c6c6b76) , ROM_SKIP(7) ) // == tkm-4.10a
	ROMX_LOAD( "07.11a",            0x200003, 0x20000, CRC(e04af054) SHA1(f227b8a0a3d8f41e1922d184eaec7a1243c7c3af) , ROM_SKIP(7) ) // == tkm-4.10a
	ROMX_LOAD( "26.8e",             0x200004, 0x20000, CRC(07fc714b) SHA1(eda97a3c5596ebdfa61bdd01d39647c89b9a2f13) , ROM_SKIP(7) ) // == tkm-2.4a
	ROMX_LOAD( "19.8c",             0x200005, 0x20000, CRC(7114e5c6) SHA1(2f2925b942af50781857f4fe74e9a58f2cf7b883) , ROM_SKIP(7) ) // == tkm-2.4a
	ROMX_LOAD( "28.10e",            0x200006, 0x20000, CRC(af62bf07) SHA1(a6e0f598de1fa8a4960e89d655b7514572ed6310) , ROM_SKIP(7) ) // == tkm-3.6a
	ROMX_LOAD( "21.10c",            0x200007, 0x20000, CRC(523f462a) SHA1(b0fc9e29d6ca44aafb20a62355bde9f4b4cf1e43) , ROM_SKIP(7) ) // == tkm-3.6a
	ROMX_LOAD( "12.8b",             0x300000, 0x20000, CRC(38652339) SHA1(930a035bbe34c81c26d774d2ab45f53a3a9205fb) , ROM_SKIP(7) ) // == tkm-1.8a
	ROMX_LOAD( "04.8a",             0x300001, 0x20000, CRC(4951bc0f) SHA1(07f424c147d787321b668d787216733c35e2cff9) , ROM_SKIP(7) ) // == tkm-1.8a
	ROMX_LOAD( "16.12b",            0x300002, 0x20000, CRC(381608ae) SHA1(666e15e61c7c59df5a97bdc2d77db611d60b3ca8) , ROM_SKIP(7) ) // == tkm-4.10a
	ROMX_LOAD( "08.12a",            0x300003, 0x20000, CRC(b475d4e9) SHA1(dc5d223bc2a27904e6e38b68507d2e87fbbde158) , ROM_SKIP(7) ) // == tkm-4.10a
	ROMX_LOAD( "27.9e",             0x300004, 0x20000, CRC(a27e81fa) SHA1(b25854d4a7e52d500c19445badb4cfe745d88d23) , ROM_SKIP(7) ) // == tkm-2.4a
	ROMX_LOAD( "20.9c",             0x300005, 0x20000, CRC(002796dc) SHA1(2dba0434916dd82c59a66e2f3ce8d3165713c308) , ROM_SKIP(7) ) // == tkm-2.4a
	ROMX_LOAD( "29.11e",            0x300006, 0x20000, CRC(6b41f82d) SHA1(111af606d8107d377e3af618584a75ed6cfc9bbd) , ROM_SKIP(7) ) // == tkm-3.6a
	ROMX_LOAD( "22.11c",            0x300007, 0x20000, CRC(52145369) SHA1(bd422f0c51cdd62b69229f926569ad05d430bd57) , ROM_SKIP(7) ) // == tkm-3.6a

	ROM_REGION( 0x18000, "audiocpu", 0 ) /* 64k for the audio CPU (+banks) */
	ROM_LOAD( "23.13c",             0x00000, 0x08000, CRC(b3b79d4f) SHA1(2b960545741d3b9a53ffbf3ed83030392aa02698) )    // == tke_17.12b
	ROM_CONTINUE(                   0x10000, 0x08000 )

	ROM_REGION( 0x40000, "oki", 0 ) /* Samples */
	ROM_LOAD( "30.12e",             0x00000, 0x20000, CRC(7e5f6cb4) SHA1(c7b6b7d6dfe5f9f0e1521e7ce990229f480cf68d) )
	ROM_LOAD( "31.13e",             0x20000, 0x20000, CRC(4a30c737) SHA1(426eb90f2edf73eb468c94b4a094da3d46acbab2) )

	ROM_REGION( 0x0200, "aboardplds", 0 )
	ROM_LOAD( "buf1",         0x0000, 0x0117, CRC(eb122de7) SHA1(b26b5bfe258e3e184f069719f9fd008d6b8f6b9b) )
	ROM_LOAD( "ioa1",         0x0000, 0x0117, CRC(59c7ee3b) SHA1(fbb887c5b4f5cb8df77cec710eaac2985bc482a6) )
	ROM_LOAD( "prg1",         0x0000, 0x0117, CRC(f1129744) SHA1(a5300f301c1a08a7da768f0773fa0fe3f683b237) )
	ROM_LOAD( "rom1",         0x0000, 0x0117, CRC(41dc73b9) SHA1(7d4c9f1693c821fbf84e32dd6ef62ddf14967845) )
	ROM_LOAD( "sou1",         0x0000, 0x0117, CRC(84f4b2fe) SHA1(dcc9e86cc36316fe42eace02d6df75d08bc8bb6d) )

	ROM_REGION( 0x0200, "bboardplds", 0 )
	ROM_LOAD( "tk22b.1a",     0x0000, 0x0117, CRC(1a1ab6d7) SHA1(eacd3a21a09683e1d8ad36215c1f8ea96a1f7c12) )
	ROM_LOAD( "lwio.12c",     0x0000, 0x0117, CRC(ad52b90c) SHA1(f0fd6aeea515ee449320fe15684e6b3ab7f97bf4) )
ROM_END

/* B-Board 91634B-2, Japan Resale Ver. */
ROM_START( dynwarjr )
	ROM_REGION( CODE_SIZE, "maincpu", 0 )      /* 68000 code */
	ROM_LOAD16_WORD_SWAP( "tk1j_23.8f", 0x00000, 0x80000, CRC(088a3009) SHA1(d4c8273d19291d278d2ff895712dfbd8dfda6c84) )
	ROM_LOAD16_WORD_SWAP( "tk1j_22.7f", 0x80000, 0x80000, CRC(93654bcf) SHA1(c72daeb2a98d350568555059a3225343c219a1d2) )    // == tkm-9.8h

	ROM_REGION( 0x400000, "gfx", 0 )
	ROMX_LOAD( "tk1_01.3a",  0x000000, 0x80000, CRC(f64bb6a0) SHA1(8c0ae0305704ad876bb1938e46550c68b0de8e8e) , ROM_GROUPWORD | ROM_SKIP(6) )    // == tkm-5.7a
	ROMX_LOAD( "tk1_02.4a",  0x000002, 0x80000, CRC(21fe6274) SHA1(f92e509d88d5e264be9c7812966d64ad9ac518e7) , ROM_GROUPWORD | ROM_SKIP(6) )    // == tkm-8.9a
	ROMX_LOAD( "tk1_03.5a",  0x000004, 0x80000, CRC(0bf228cb) SHA1(e72957155cb459c4dee50df2e53256f271528964) , ROM_GROUPWORD | ROM_SKIP(6) )    // == tkm-6.3a
	ROMX_LOAD( "tk1_04.6a",  0x000006, 0x80000, CRC(1255dfb1) SHA1(c943e3c989d5b20fbe24e38e54ee8ca294b3d182) , ROM_GROUPWORD | ROM_SKIP(6) )    // == tkm-7.5a
	ROMX_LOAD( "tk1_05.7a",  0x200000, 0x80000, CRC(44f7661e) SHA1(f29b5ad0c5dfd91a56a4a1084ce578cfe496dd6f) , ROM_GROUPWORD | ROM_SKIP(6) )    // == tkm-1.8a
	ROMX_LOAD( "tk1_06.8a",  0x200002, 0x80000, CRC(a54c515d) SHA1(bfa457cef7e29ae56ee9b10f60e233d82b4efc61) , ROM_GROUPWORD | ROM_SKIP(6) )    // == tkm-4.10a
	ROMX_LOAD( "tk1_07.9a",  0x200004, 0x80000, CRC(ca5c687c) SHA1(de47cb5a071ffb3ff408f60d45b79345032232a7) , ROM_GROUPWORD | ROM_SKIP(6) )    // == tkm-2.4a
	ROMX_LOAD( "tk1_08.10a", 0x200006, 0x80000, CRC(f9fe6591) SHA1(260da5f9e305cccd621b8b5b2073c79e161ddeb0) , ROM_GROUPWORD | ROM_SKIP(6) )    // == tkm-3.6a

	ROM_REGION( 0x28000, "audiocpu", 0 ) /* 64k for the audio CPU (+banks) */
	ROM_LOAD( "tk1_09.12a",  0x00000, 0x08000, CRC(db77d899) SHA1(314a8134ed15f603f1c7894fd50b14aef5c9fc7a) )
	ROM_CONTINUE(            0x10000, 0x18000 ) // second half of ROM is empty, not mapped in memory

	ROM_REGION( 0x40000, "oki", 0 ) /* Samples */
	ROM_LOAD( "tk1_18.11c",  0x00000, 0x20000, CRC(7e5f6cb4) SHA1(c7b6b7d6dfe5f9f0e1521e7ce990229f480cf68d) )
	ROM_LOAD( "tk1_19.12c",  0x20000, 0x20000, CRC(4a30c737) SHA1(426eb90f2edf73eb468c94b4a094da3d46acbab2) )

	ROM_REGION( 0x0200, "aboardplds", 0 )
	ROM_LOAD( "buf1",         0x0000, 0x0117, CRC(eb122de7) SHA1(b26b5bfe258e3e184f069719f9fd008d6b8f6b9b) )
	ROM_LOAD( "ioa1",         0x0000, 0x0117, CRC(59c7ee3b) SHA1(fbb887c5b4f5cb8df77cec710eaac2985bc482a6) )
	ROM_LOAD( "prg1",         0x0000, 0x0117, CRC(f1129744) SHA1(a5300f301c1a08a7da768f0773fa0fe3f683b237) )
	ROM_LOAD( "rom1",         0x0000, 0x0117, CRC(41dc73b9) SHA1(7d4c9f1693c821fbf84e32dd6ef62ddf14967845) )
	ROM_LOAD( "sou1",         0x0000, 0x0117, CRC(84f4b2fe) SHA1(dcc9e86cc36316fe42eace02d6df75d08bc8bb6d) )

	ROM_REGION( 0x0200, "bboardplds", 0 )
	ROM_LOAD( "tk163b.1a",    0x0000, 0x0117, NO_DUMP )
	ROM_LOAD( "iob1.12d",     0x0000, 0x0117, CRC(3abc0700) SHA1(973043aa46ec6d5d1db20dc9d5937005a0f9f6ae) )
	ROM_LOAD( "bprg1.11d",    0x0000, 0x0117, CRC(31793da7) SHA1(400fa7ac517421c978c1ee7773c30b9ed0c5d3f3) )

	ROM_REGION( 0x0200, "cboardplds", 0 )
	ROM_LOAD( "ioc1.ic7",     0x0000, 0x0117, CRC(0d182081) SHA1(475b3d417785da4bc512cce2b274bb00d4cc6792) )
	ROM_LOAD( "c632.ic1",     0x0000, 0x0117, CRC(0fbd9270) SHA1(d7e737b20c44d41e29ca94be56114b31934dde81) )
ROM_END

/* B-Board 89624B-3 */
ROM_START( willow )
	ROM_REGION( CODE_SIZE, "maincpu", 0 )      /* 68000 code */
	ROM_LOAD16_BYTE( "wle_30.11f",     0x00000, 0x20000, CRC(15372aa2) SHA1(ba8e1984180b0438255dfc68dc4eb560f3ecbe56) )
	ROM_LOAD16_BYTE( "wle_35.11h",     0x00001, 0x20000, CRC(2e64623b) SHA1(473f6fd10b2456553f1cbf92fd9a61ce94b1c59f) )
	ROM_LOAD16_BYTE( "wlu_31.12f",     0x40000, 0x20000, CRC(0eb48a83) SHA1(28c40c4b5d767f88922cd899e948abf11a85a864) )
	ROM_LOAD16_BYTE( "wlu_36.12h",     0x40001, 0x20000, CRC(36100209) SHA1(63c9338e71dba8b52daffba50b4bca31aaa10d9e) )
	ROM_LOAD16_WORD_SWAP( "wlm-32.8h", 0x80000, 0x80000, CRC(dfd9f643) SHA1(9c760c30af593a87e7fd39fb213a4c73c68ca440) )

	ROM_REGION( 0x400000, "gfx", 0 )
	ROMX_LOAD( "wlm-7.7a", 0x000000, 0x80000, CRC(afa74b73) SHA1(09081926260c76986a13ac5351dddd2ea11d7a10) , ROM_GROUPWORD | ROM_SKIP(6) )  // in "5" socket
	ROMX_LOAD( "wlm-5.9a", 0x000002, 0x80000, CRC(12a0dc0b) SHA1(fea235ce9489f04919daf52f4d3f3bac9b558316) , ROM_GROUPWORD | ROM_SKIP(6) )  // in "7" socket
	ROMX_LOAD( "wlm-3.3a", 0x000004, 0x80000, CRC(c6f2abce) SHA1(ff5fcfe417c43b4747bbe12db6052fdb60f5f0e4) , ROM_GROUPWORD | ROM_SKIP(6) )  // in "1" socket
	ROMX_LOAD( "wlm-1.5a", 0x000006, 0x80000, CRC(4aa4c6d3) SHA1(7dd6f18f6126c380821a2ca8955439fd6864f4c6) , ROM_GROUPWORD | ROM_SKIP(6) )  // in "3" socket
	ROMX_LOAD( "wl_24.7d", 0x200000, 0x20000, CRC(6f0adee5) SHA1(07b18e51b376001f25173b78e0e816f252400210) , ROM_SKIP(7) )
	ROMX_LOAD( "wl_14.7c", 0x200001, 0x20000, CRC(9cf3027d) SHA1(1e8eb20d51a54f6f756c0ab9395ac38b96e67fb2) , ROM_SKIP(7) )
	ROMX_LOAD( "wl_26.9d", 0x200002, 0x20000, CRC(f09c8ecf) SHA1(b39f83e80af010d6481693d9ec8b1d7e258b531d) , ROM_SKIP(7) )
	ROMX_LOAD( "wl_16.9c", 0x200003, 0x20000, CRC(e35407aa) SHA1(7ddae9cef96839da72488c1fe73268c50e0262ff) , ROM_SKIP(7) )
	ROMX_LOAD( "wl_20.3d", 0x200004, 0x20000, CRC(84992350) SHA1(f0ebd810ce099337cda94222dccce8ab9b3c3281) , ROM_SKIP(7) )
	ROMX_LOAD( "wl_10.3c", 0x200005, 0x20000, CRC(b87b5a36) SHA1(25fb8f9698142473233ee509d4146089920e94e1) , ROM_SKIP(7) )
	ROMX_LOAD( "wl_22.5d", 0x200006, 0x20000, CRC(fd3f89f0) SHA1(51ff95cff56ac78682ea56401b35a0aa63cef8cb) , ROM_SKIP(7) )
	ROMX_LOAD( "wl_12.5c", 0x200007, 0x20000, CRC(7da49d69) SHA1(b0ae7ac4f858ee8d72e6877c4275da7a631e2e4c) , ROM_SKIP(7) )

	ROM_REGION( 0x18000, "audiocpu", 0 ) /* 64k for the audio CPU (+banks) */
	ROM_LOAD( "wl_09.12b", 0x00000, 0x08000, CRC(f6b3d060) SHA1(0ed2e2f64ba53ba2c371b66ab1e52e40b16d8baf) )
	ROM_CONTINUE(          0x10000, 0x08000 )

	ROM_REGION( 0x40000, "oki", 0 ) /* Samples */
	ROM_LOAD( "wl_18.11c", 0x00000, 0x20000, CRC(bde23d4d) SHA1(d1fee2f99c858dfb07edcd600da491c7b656afe0) )
	ROM_LOAD( "wl_19.12c", 0x20000, 0x20000, CRC(683898f5) SHA1(316a77b663d78c8b9ff6d85756cb05aaaeef4003) )

	ROM_REGION( 0x0200, "aboardplds", 0 )
	ROM_LOAD( "buf1",         0x0000, 0x0117, CRC(eb122de7) SHA1(b26b5bfe258e3e184f069719f9fd008d6b8f6b9b) )
	ROM_LOAD( "ioa1",         0x0000, 0x0117, CRC(59c7ee3b) SHA1(fbb887c5b4f5cb8df77cec710eaac2985bc482a6) )
	ROM_LOAD( "prg1",         0x0000, 0x0117, CRC(f1129744) SHA1(a5300f301c1a08a7da768f0773fa0fe3f683b237) )
	ROM_LOAD( "rom1",         0x0000, 0x0117, CRC(41dc73b9) SHA1(7d4c9f1693c821fbf84e32dd6ef62ddf14967845) )
	ROM_LOAD( "sou1",         0x0000, 0x0117, CRC(84f4b2fe) SHA1(dcc9e86cc36316fe42eace02d6df75d08bc8bb6d) )

	ROM_REGION( 0x0200, "bboardplds", 0 )
	ROM_LOAD( "wl24b.1a",     0x0000, 0x0117, CRC(7101cdf1) SHA1(c848f109d09641b3159dbbb2d2ee49cf30bc9e9c) )
	ROM_LOAD( "lwio.11e",     0x0000, 0x0117, CRC(ad52b90c) SHA1(f0fd6aeea515ee449320fe15684e6b3ab7f97bf4) )
ROM_END

/* B-Board 89624B-3 */
ROM_START( willowu )
	ROM_REGION( CODE_SIZE, "maincpu", 0 )      /* 68000 code */
	ROM_LOAD16_BYTE( "wlu_30.11f",     0x00000, 0x20000, CRC(d604dbb1) SHA1(b5d78871011ff11a67f1a0cad147cd4de8d67f35) )
	ROM_LOAD16_BYTE( "35.11h",         0x00001, 0x20000, CRC(7a791e77) SHA1(fe1429588b7eceab1d369abe03f2cad8de727f71) )
	ROM_LOAD16_BYTE( "wlu_31.12f",     0x40000, 0x20000, CRC(0eb48a83) SHA1(28c40c4b5d767f88922cd899e948abf11a85a864) )
	ROM_LOAD16_BYTE( "wlu_36.12h",     0x40001, 0x20000, CRC(36100209) SHA1(63c9338e71dba8b52daffba50b4bca31aaa10d9e) ) /* seen the same pcb with WL_36.12H */
	ROM_LOAD16_WORD_SWAP( "wlm-32.8h", 0x80000, 0x80000, CRC(dfd9f643) SHA1(9c760c30af593a87e7fd39fb213a4c73c68ca440) )

	ROM_REGION( 0x400000, "gfx", 0 )
	ROMX_LOAD( "wlm-7.7a", 0x000000, 0x80000, CRC(afa74b73) SHA1(09081926260c76986a13ac5351dddd2ea11d7a10) , ROM_GROUPWORD | ROM_SKIP(6) )  // in "5" socket
	ROMX_LOAD( "wlm-5.9a", 0x000002, 0x80000, CRC(12a0dc0b) SHA1(fea235ce9489f04919daf52f4d3f3bac9b558316) , ROM_GROUPWORD | ROM_SKIP(6) )  // in "7" socket
	ROMX_LOAD( "wlm-3.3a", 0x000004, 0x80000, CRC(c6f2abce) SHA1(ff5fcfe417c43b4747bbe12db6052fdb60f5f0e4) , ROM_GROUPWORD | ROM_SKIP(6) )  // in "1" socket
	ROMX_LOAD( "wlm-1.5a", 0x000006, 0x80000, CRC(4aa4c6d3) SHA1(7dd6f18f6126c380821a2ca8955439fd6864f4c6) , ROM_GROUPWORD | ROM_SKIP(6) )  // in "3" socket
	ROMX_LOAD( "wl_24.7d", 0x200000, 0x20000, CRC(6f0adee5) SHA1(07b18e51b376001f25173b78e0e816f252400210) , ROM_SKIP(7) )
	ROMX_LOAD( "wl_14.7c", 0x200001, 0x20000, CRC(9cf3027d) SHA1(1e8eb20d51a54f6f756c0ab9395ac38b96e67fb2) , ROM_SKIP(7) )
	ROMX_LOAD( "wl_26.9d", 0x200002, 0x20000, CRC(f09c8ecf) SHA1(b39f83e80af010d6481693d9ec8b1d7e258b531d) , ROM_SKIP(7) )
	ROMX_LOAD( "wl_16.9c", 0x200003, 0x20000, CRC(e35407aa) SHA1(7ddae9cef96839da72488c1fe73268c50e0262ff) , ROM_SKIP(7) )
	ROMX_LOAD( "wl_20.3d", 0x200004, 0x20000, CRC(84992350) SHA1(f0ebd810ce099337cda94222dccce8ab9b3c3281) , ROM_SKIP(7) )
	ROMX_LOAD( "wl_10.3c", 0x200005, 0x20000, CRC(b87b5a36) SHA1(25fb8f9698142473233ee509d4146089920e94e1) , ROM_SKIP(7) )
	ROMX_LOAD( "wl_22.5d", 0x200006, 0x20000, CRC(fd3f89f0) SHA1(51ff95cff56ac78682ea56401b35a0aa63cef8cb) , ROM_SKIP(7) )
	ROMX_LOAD( "wl_12.5c", 0x200007, 0x20000, CRC(7da49d69) SHA1(b0ae7ac4f858ee8d72e6877c4275da7a631e2e4c) , ROM_SKIP(7) )

	ROM_REGION( 0x18000, "audiocpu", 0 ) /* 64k for the audio CPU (+banks) */
	ROM_LOAD( "wl_09.12b", 0x00000, 0x08000, CRC(f6b3d060) SHA1(0ed2e2f64ba53ba2c371b66ab1e52e40b16d8baf) )
	ROM_CONTINUE(          0x10000, 0x08000 )

	ROM_REGION( 0x40000, "oki", 0 ) /* Samples */
	ROM_LOAD( "wl_18.11c", 0x00000, 0x20000, CRC(bde23d4d) SHA1(d1fee2f99c858dfb07edcd600da491c7b656afe0) )
	ROM_LOAD( "wl_19.12c", 0x20000, 0x20000, CRC(683898f5) SHA1(316a77b663d78c8b9ff6d85756cb05aaaeef4003) )

	ROM_REGION( 0x0200, "aboardplds", 0 )
	ROM_LOAD( "buf1",         0x0000, 0x0117, CRC(eb122de7) SHA1(b26b5bfe258e3e184f069719f9fd008d6b8f6b9b) )
	ROM_LOAD( "ioa1",         0x0000, 0x0117, CRC(59c7ee3b) SHA1(fbb887c5b4f5cb8df77cec710eaac2985bc482a6) )
	ROM_LOAD( "prg1",         0x0000, 0x0117, CRC(f1129744) SHA1(a5300f301c1a08a7da768f0773fa0fe3f683b237) )
	ROM_LOAD( "rom1",         0x0000, 0x0117, CRC(41dc73b9) SHA1(7d4c9f1693c821fbf84e32dd6ef62ddf14967845) )
	ROM_LOAD( "sou1",         0x0000, 0x0117, CRC(84f4b2fe) SHA1(dcc9e86cc36316fe42eace02d6df75d08bc8bb6d) )

	ROM_REGION( 0x0200, "bboardplds", 0 )
	ROM_LOAD( "wl24b.1a",     0x0000, 0x0117, CRC(7101cdf1) SHA1(c848f109d09641b3159dbbb2d2ee49cf30bc9e9c) )
	ROM_LOAD( "lwio.11e",     0x0000, 0x0117, CRC(ad52b90c) SHA1(f0fd6aeea515ee449320fe15684e6b3ab7f97bf4) )
ROM_END

/* B-Board 89624B-3 */
/* Note that this set comes from a pcb running on an original Capcom USA Willow arcade cabinet, so even if there is
   the Japan "warning" it's confirmed to be a genuine USA set and almost certainly the first USA release. Then Capcom
   removed the incorrect "warning" releasing a new proper set of Willow (USA), as documented above. */
ROM_START( willowuo )
	ROM_REGION( CODE_SIZE, "maincpu", 0 )      /* 68000 code */
	ROM_LOAD16_BYTE( "wlu_30.11f",     0x00000, 0x20000, CRC(d604dbb1) SHA1(b5d78871011ff11a67f1a0cad147cd4de8d67f35) )
	ROM_LOAD16_BYTE( "wlu_35.11h",     0x00001, 0x20000, CRC(daee72fe) SHA1(2ec62f44394fac2887821881f56b6f24d05234b3) )
	ROM_LOAD16_BYTE( "wlu_31.12f",     0x40000, 0x20000, CRC(0eb48a83) SHA1(28c40c4b5d767f88922cd899e948abf11a85a864) )
	ROM_LOAD16_BYTE( "wlu_36.12h",     0x40001, 0x20000, CRC(36100209) SHA1(63c9338e71dba8b52daffba50b4bca31aaa10d9e) )
	ROM_LOAD16_WORD_SWAP( "wlm-32.8h", 0x80000, 0x80000, CRC(dfd9f643) SHA1(9c760c30af593a87e7fd39fb213a4c73c68ca440) )

	ROM_REGION( 0x400000, "gfx", 0 )
	ROMX_LOAD( "wlm-7.7a", 0x000000, 0x80000, CRC(afa74b73) SHA1(09081926260c76986a13ac5351dddd2ea11d7a10) , ROM_GROUPWORD | ROM_SKIP(6) )  // in "5" socket
	ROMX_LOAD( "wlm-5.9a", 0x000002, 0x80000, CRC(12a0dc0b) SHA1(fea235ce9489f04919daf52f4d3f3bac9b558316) , ROM_GROUPWORD | ROM_SKIP(6) )  // in "7" socket
	ROMX_LOAD( "wlm-3.3a", 0x000004, 0x80000, CRC(c6f2abce) SHA1(ff5fcfe417c43b4747bbe12db6052fdb60f5f0e4) , ROM_GROUPWORD | ROM_SKIP(6) )  // in "1" socket
	ROMX_LOAD( "wlm-1.5a", 0x000006, 0x80000, CRC(4aa4c6d3) SHA1(7dd6f18f6126c380821a2ca8955439fd6864f4c6) , ROM_GROUPWORD | ROM_SKIP(6) )  // in "3" socket
	ROMX_LOAD( "wl_24.7d", 0x200000, 0x20000, CRC(6f0adee5) SHA1(07b18e51b376001f25173b78e0e816f252400210) , ROM_SKIP(7) )
	ROMX_LOAD( "wl_14.7c", 0x200001, 0x20000, CRC(9cf3027d) SHA1(1e8eb20d51a54f6f756c0ab9395ac38b96e67fb2) , ROM_SKIP(7) )
	ROMX_LOAD( "wl_26.9d", 0x200002, 0x20000, CRC(f09c8ecf) SHA1(b39f83e80af010d6481693d9ec8b1d7e258b531d) , ROM_SKIP(7) )
	ROMX_LOAD( "wl_16.9c", 0x200003, 0x20000, CRC(e35407aa) SHA1(7ddae9cef96839da72488c1fe73268c50e0262ff) , ROM_SKIP(7) )
	ROMX_LOAD( "wl_20.3d", 0x200004, 0x20000, CRC(84992350) SHA1(f0ebd810ce099337cda94222dccce8ab9b3c3281) , ROM_SKIP(7) )
	ROMX_LOAD( "wl_10.3c", 0x200005, 0x20000, CRC(b87b5a36) SHA1(25fb8f9698142473233ee509d4146089920e94e1) , ROM_SKIP(7) )
	ROMX_LOAD( "wl_22.5d", 0x200006, 0x20000, CRC(fd3f89f0) SHA1(51ff95cff56ac78682ea56401b35a0aa63cef8cb) , ROM_SKIP(7) )
	ROMX_LOAD( "wl_12.5c", 0x200007, 0x20000, CRC(7da49d69) SHA1(b0ae7ac4f858ee8d72e6877c4275da7a631e2e4c) , ROM_SKIP(7) )

	ROM_REGION( 0x18000, "audiocpu", 0 ) /* 64k for the audio CPU (+banks) */
	ROM_LOAD( "wl_09.12b", 0x00000, 0x08000, CRC(f6b3d060) SHA1(0ed2e2f64ba53ba2c371b66ab1e52e40b16d8baf) )
	ROM_CONTINUE(          0x10000, 0x08000 )

	ROM_REGION( 0x40000, "oki", 0 ) /* Samples */
	ROM_LOAD( "wl_18.11c", 0x00000, 0x20000, CRC(bde23d4d) SHA1(d1fee2f99c858dfb07edcd600da491c7b656afe0) )
	ROM_LOAD( "wl_19.12c", 0x20000, 0x20000, CRC(683898f5) SHA1(316a77b663d78c8b9ff6d85756cb05aaaeef4003) )

	ROM_REGION( 0x0200, "aboardplds", 0 )
	ROM_LOAD( "buf1",         0x0000, 0x0117, CRC(eb122de7) SHA1(b26b5bfe258e3e184f069719f9fd008d6b8f6b9b) )
	ROM_LOAD( "ioa1",         0x0000, 0x0117, CRC(59c7ee3b) SHA1(fbb887c5b4f5cb8df77cec710eaac2985bc482a6) )
	ROM_LOAD( "prg1",         0x0000, 0x0117, CRC(f1129744) SHA1(a5300f301c1a08a7da768f0773fa0fe3f683b237) )
	ROM_LOAD( "rom1",         0x0000, 0x0117, CRC(41dc73b9) SHA1(7d4c9f1693c821fbf84e32dd6ef62ddf14967845) )
	ROM_LOAD( "sou1",         0x0000, 0x0117, CRC(84f4b2fe) SHA1(dcc9e86cc36316fe42eace02d6df75d08bc8bb6d) )

	ROM_REGION( 0x0200, "bboardplds", 0 )
	ROM_LOAD( "wl24b.1a",     0x0000, 0x0117, CRC(7101cdf1) SHA1(c848f109d09641b3159dbbb2d2ee49cf30bc9e9c) )
	ROM_LOAD( "lwio.11e",     0x0000, 0x0117, CRC(ad52b90c) SHA1(f0fd6aeea515ee449320fe15684e6b3ab7f97bf4) )
ROM_END

/* B-Board 88622B-3 */
ROM_START( willowj )
	ROM_REGION( CODE_SIZE, "maincpu", 0 )      /* 68000 code */
	ROM_LOAD16_BYTE( "wl_36.12f", 0x00000, 0x20000, CRC(2b0d7cbc) SHA1(58172b4fdf856efa8d77abbde76738de2424f712) )
	ROM_LOAD16_BYTE( "wl_42.12h", 0x00001, 0x20000, CRC(1ac39615) SHA1(c9fa6d20418b9bdc5a08df1fb86368b40709280a) )
	ROM_LOAD16_BYTE( "wl_37.13f", 0x40000, 0x20000, CRC(30a717fa) SHA1(cb815e9ee2691761925898e3932b502f8f399cb4) )
	ROM_LOAD16_BYTE( "wl_43.13h", 0x40001, 0x20000, CRC(d0dddc9e) SHA1(1176b9a43b3355a5ba44e59abde01ee7eaa89c25) )
	ROM_LOAD16_BYTE( "wl_34.10f", 0x80000, 0x20000, CRC(23a84f7a) SHA1(f7667ffc85e2dfb78f5ed8ea54b6223e7375433a) )  // == wlm-32.8h
	ROM_LOAD16_BYTE( "wl_40.10h", 0x80001, 0x20000, CRC(c7a0ed21) SHA1(190aec48a8f528652b434425f64ea2e52029ad3d) )  // == wlm-32.8h
	ROM_LOAD16_BYTE( "wl_35.11f", 0xc0000, 0x20000, CRC(5eff7951) SHA1(f336bfd07f3083e3b3e72c61d03abb52d443204d) )  // == wlm-32.8h
	ROM_LOAD16_BYTE( "wl_41.11h", 0xc0001, 0x20000, CRC(8d6477a3) SHA1(1bdb98f89ec8c2dcd47ab90bdd1be862ea59a16b) )  // == wlm-32.8h

	ROM_REGION( 0x400000, "gfx", 0 )
	ROMX_LOAD( "wl_09.4b",  0x000000, 0x20000, CRC(05aa71b4) SHA1(791f845928ff164eace07cfcc35d0bd9d7cbb2d0) , ROM_SKIP(7) ) // == wlm-7.7a
	ROMX_LOAD( "wl_01.4a",  0x000001, 0x20000, CRC(08c2df12) SHA1(269ca05ff6f9928e3b46f43b262c03ac3dd768b8) , ROM_SKIP(7) ) // == wlm-7.7a
	ROMX_LOAD( "wl_13.9b",  0x000002, 0x20000, CRC(1f7c87cd) SHA1(6817c8b42ef4e8183fc2fbd2765e49ac955cca80) , ROM_SKIP(7) ) // == wlm-5.9a
	ROMX_LOAD( "wl_05.9a",  0x000003, 0x20000, CRC(f5254bf2) SHA1(a53973b98b8e8903c731eb2514cb9ce4060d5164) , ROM_SKIP(7) ) // == wlm-5.9a
	ROMX_LOAD( "wl_24.5e",  0x000004, 0x20000, CRC(d9d73ba1) SHA1(5f7b5e7e6fe64106cd96a968a54556e4f5ad9387) , ROM_SKIP(7) ) // == wlm-3.3a
	ROMX_LOAD( "wl_17.5c",  0x000005, 0x20000, CRC(a652f30c) SHA1(b150d8978d37907c6d17042476c4608c76c4ea3f) , ROM_SKIP(7) ) // == wlm-3.3a
	ROMX_LOAD( "wl_38.8h",  0x000006, 0x20000, CRC(f6f9111b) SHA1(bd44d1b6084fe2c09cdd91b5ad6eff7a84f3e601) , ROM_SKIP(7) ) // == wlm-1.5a
	ROMX_LOAD( "wl_32.8f",  0x000007, 0x20000, CRC(10f64027) SHA1(abf387ba7d8f0530a562a984c08345a149cfa226) , ROM_SKIP(7) ) // == wlm-1.5a
	ROMX_LOAD( "wl_10.5b",  0x100000, 0x20000, CRC(dbba0a3f) SHA1(38c0cdd87e6631721f35a0b85fd5165940454603) , ROM_SKIP(7) ) // == wlm-7.7a
	ROMX_LOAD( "wl_02.5a",  0x100001, 0x20000, CRC(86fba7a5) SHA1(9389ad4e0968b3edd4baeac8b29637618f49dbfb) , ROM_SKIP(7) ) // == wlm-7.7a
	ROMX_LOAD( "wl_14.10b", 0x100002, 0x20000, CRC(7d5798b2) SHA1(7dfcc172e5df0e8f4b9188381463f9eca00945dc) , ROM_SKIP(7) ) // == wlm-5.9a
	ROMX_LOAD( "wl_06.10a", 0x100003, 0x20000, CRC(1f052948) SHA1(ca3a6be94ae50fa50604ee52704792b22cbbc083) , ROM_SKIP(7) ) // == wlm-5.9a
	ROMX_LOAD( "wl_25.7e",  0x100004, 0x20000, CRC(857d17d2) SHA1(e6b383df9571df6743a7aa3087f0632dc4e6143b) , ROM_SKIP(7) ) // == wlm-3.3a
	ROMX_LOAD( "wl_18.7c",  0x100005, 0x20000, CRC(316c7fbc) SHA1(631682c7f14fa73f5f5a4fb747bfb45d9613e507) , ROM_SKIP(7) ) // == wlm-3.3a
	ROMX_LOAD( "wl_39.9h",  0x100006, 0x20000, CRC(e6fce9b0) SHA1(46d50cf32c3518ab3dc717553c5375f3cb022cde) , ROM_SKIP(7) ) // == wlm-1.5a
	ROMX_LOAD( "wl_33.9f",  0x100007, 0x20000, CRC(a15d5517) SHA1(03a30e216f8240383d5fa9736b4fe5119d231685) , ROM_SKIP(7) ) // == wlm-1.5a
	ROMX_LOAD( "wl_11.7b",  0x200000, 0x20000, CRC(6f0adee5) SHA1(07b18e51b376001f25173b78e0e816f252400210) , ROM_SKIP(7) ) // == wl_24.7d
	ROMX_LOAD( "wl_03.7a",  0x200001, 0x20000, CRC(9cf3027d) SHA1(1e8eb20d51a54f6f756c0ab9395ac38b96e67fb2) , ROM_SKIP(7) ) // == wl_14.7c
	ROMX_LOAD( "wl_15.11b", 0x200002, 0x20000, CRC(f09c8ecf) SHA1(b39f83e80af010d6481693d9ec8b1d7e258b531d) , ROM_SKIP(7) ) // == wl_26.9d
	ROMX_LOAD( "wl_07.11a", 0x200003, 0x20000, CRC(e35407aa) SHA1(7ddae9cef96839da72488c1fe73268c50e0262ff) , ROM_SKIP(7) ) // == wl_16.9c
	ROMX_LOAD( "wl_26.8e",  0x200004, 0x20000, CRC(84992350) SHA1(f0ebd810ce099337cda94222dccce8ab9b3c3281) , ROM_SKIP(7) ) // == wl_20.3d
	ROMX_LOAD( "wl_19.8c",  0x200005, 0x20000, CRC(b87b5a36) SHA1(25fb8f9698142473233ee509d4146089920e94e1) , ROM_SKIP(7) ) // == wl_10.3c
	ROMX_LOAD( "wl_28.10e", 0x200006, 0x20000, CRC(fd3f89f0) SHA1(51ff95cff56ac78682ea56401b35a0aa63cef8cb) , ROM_SKIP(7) ) // == wl_22.5d
	ROMX_LOAD( "wl_21.10c", 0x200007, 0x20000, CRC(7da49d69) SHA1(b0ae7ac4f858ee8d72e6877c4275da7a631e2e4c) , ROM_SKIP(7) ) // == wl_12.5c

	ROM_REGION( 0x18000, "audiocpu", 0 ) /* 64k for the audio CPU (+banks) */
	ROM_LOAD( "wl_23.13c",  0x00000, 0x08000, CRC(f6b3d060) SHA1(0ed2e2f64ba53ba2c371b66ab1e52e40b16d8baf) )    // == wl_09.12b
	ROM_CONTINUE(           0x10000, 0x08000 )

	ROM_REGION( 0x40000, "oki", 0 ) /* Samples */
	ROM_LOAD( "wl_30.12e",  0x00000, 0x20000, CRC(bde23d4d) SHA1(d1fee2f99c858dfb07edcd600da491c7b656afe0) )    // == wl_18.11c
	ROM_LOAD( "wl_31.13e",  0x20000, 0x20000, CRC(683898f5) SHA1(316a77b663d78c8b9ff6d85756cb05aaaeef4003) )    // == wl_19.12c

	ROM_REGION( 0x0200, "aboardplds", 0 )
	ROM_LOAD( "buf1",         0x0000, 0x0117, CRC(eb122de7) SHA1(b26b5bfe258e3e184f069719f9fd008d6b8f6b9b) )
	ROM_LOAD( "ioa1",         0x0000, 0x0117, CRC(59c7ee3b) SHA1(fbb887c5b4f5cb8df77cec710eaac2985bc482a6) )
	ROM_LOAD( "prg1",         0x0000, 0x0117, CRC(f1129744) SHA1(a5300f301c1a08a7da768f0773fa0fe3f683b237) )
	ROM_LOAD( "rom1",         0x0000, 0x0117, CRC(41dc73b9) SHA1(7d4c9f1693c821fbf84e32dd6ef62ddf14967845) )
	ROM_LOAD( "sou1",         0x0000, 0x0117, CRC(84f4b2fe) SHA1(dcc9e86cc36316fe42eace02d6df75d08bc8bb6d) )

	ROM_REGION( 0x0200, "bboardplds", 0 )
	ROM_LOAD( "wl22b.1a",     0x0000, 0x0117, NO_DUMP )
	ROM_LOAD( "lwio.12c",     0x0000, 0x0117, CRC(ad52b90c) SHA1(f0fd6aeea515ee449320fe15684e6b3ab7f97bf4) )
ROM_END

/* B-Board 89624B-2 */
ROM_START( unsquad )
	ROM_REGION( CODE_SIZE, "maincpu", 0 )      /* 68000 code */
	ROM_LOAD16_BYTE( "aru_30.11f",     0x00000, 0x20000, CRC(24d8f88d) SHA1(9c39aa1140e92307d6d9c0ca198003282bf78c78) )
	ROM_LOAD16_BYTE( "aru_35.11h",     0x00001, 0x20000, CRC(8b954b59) SHA1(33114f1417d48f60c6da3e14a094be7c0f0fd979) )
	ROM_LOAD16_BYTE( "aru_31.12f",     0x40000, 0x20000, CRC(33e9694b) SHA1(90db3052ac2ff859ede8473dd13e0f5be148590c) )
	ROM_LOAD16_BYTE( "aru_36.12h",     0x40001, 0x20000, CRC(7cc8fb9e) SHA1(f70118d1a368fd4795d9953c55d283305d1f9630) )
	ROM_LOAD16_WORD_SWAP( "ar-32m.8h", 0x80000, 0x80000, CRC(ae1d7fb0) SHA1(bb51e77574db5e2d807c4ca8e85a5d9661f5d3b3) )

	ROM_REGION( 0x200000, "gfx", 0 )
	ROMX_LOAD( "ar-5m.7a",  0x000000, 0x80000, CRC(bf4575d8) SHA1(1b268e1698be8ff9c16f80f7b9081b6be9f72601) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "ar-7m.9a",  0x000002, 0x80000, CRC(a02945f4) SHA1(ff35cdbd6c1e43b16a906f68e416559cb3d5746b) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "ar-1m.3a",  0x000004, 0x80000, CRC(5965ca8d) SHA1(49abf80fc012a7f73306139a2871aeac7fd6a3d0) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "ar-3m.5a",  0x000006, 0x80000, CRC(ac6db17d) SHA1(78eef9ba6a392859f70467f6d7cb5aa91964abed) , ROM_GROUPWORD | ROM_SKIP(6) )

	ROM_REGION( 0x18000, "audiocpu", 0 ) /* 64k for the audio CPU (+banks) */
	ROM_LOAD( "ar_09.12b",  0x00000, 0x08000, CRC(f3dd1367) SHA1(09eef72e862ac6b1a5cce5a45938b45bf4e456ad) )
	ROM_CONTINUE(           0x10000, 0x08000 )

	ROM_REGION( 0x40000, "oki", 0 ) /* Samples */
	ROM_LOAD( "aru_18.11c", 0x00000, 0x20000, CRC(584b43a9) SHA1(7820815c8c67d484baf2fdad7e55d8c14b98b860) )
	/* 20000-3ffff empty */

	ROM_REGION( 0x0200, "aboardplds", 0 )
	ROM_LOAD( "buf1",         0x0000, 0x0117, CRC(eb122de7) SHA1(b26b5bfe258e3e184f069719f9fd008d6b8f6b9b) )
	ROM_LOAD( "ioa1",         0x0000, 0x0117, CRC(59c7ee3b) SHA1(fbb887c5b4f5cb8df77cec710eaac2985bc482a6) )
	ROM_LOAD( "prg1",         0x0000, 0x0117, CRC(f1129744) SHA1(a5300f301c1a08a7da768f0773fa0fe3f683b237) )
	ROM_LOAD( "rom1",         0x0000, 0x0117, CRC(41dc73b9) SHA1(7d4c9f1693c821fbf84e32dd6ef62ddf14967845) )
	ROM_LOAD( "sou1",         0x0000, 0x0117, CRC(84f4b2fe) SHA1(dcc9e86cc36316fe42eace02d6df75d08bc8bb6d) )

	ROM_REGION( 0x0200, "bboardplds", 0 )
	ROM_LOAD( "ar24b.1a",     0x0000, 0x0117, CRC(09a51271) SHA1(147f53f426f258ad127157967fa59d4098e5ed16) )
	ROM_LOAD( "lwio.11e",     0x0000, 0x0117, CRC(ad52b90c) SHA1(f0fd6aeea515ee449320fe15684e6b3ab7f97bf4) )
ROM_END

/* B-Board 88622B-3 */
ROM_START( area88 )
	ROM_REGION( CODE_SIZE, "maincpu", 0 )      /* 68000 code */
	ROM_LOAD16_BYTE( "ar_36.12f", 0x00000, 0x20000, CRC(65030392) SHA1(d9dea5cfde28345716b0e519ee033c475be0454b) )
	ROM_LOAD16_BYTE( "ar_42.12h", 0x00001, 0x20000, CRC(c48170de) SHA1(e968522dbdd217dd8e4cd6aaeaef801c63488c1d) )
	ROM_LOAD16_BYTE( "ar_37.13f", 0x40000, 0x20000, CRC(33e9694b) SHA1(90db3052ac2ff859ede8473dd13e0f5be148590c) )  // == aru_31.12f
	ROM_LOAD16_BYTE( "ar_43.13h", 0x40001, 0x20000, CRC(7cc8fb9e) SHA1(f70118d1a368fd4795d9953c55d283305d1f9630) )  // == aru_36.12h
	ROM_LOAD16_BYTE( "ar_34.10f", 0x80000, 0x20000, CRC(f6e80386) SHA1(462c1e9981b733df03e4d084df2d1fc58cf2022c) )  // == ar-32m.8h
	ROM_LOAD16_BYTE( "ar_40.10h", 0x80001, 0x20000, CRC(be36c145) SHA1(9ada7ac7361ff8871e2ae61f75e4e5d98936cdc3) )  // == ar-32m.8h
	ROM_LOAD16_BYTE( "ar_35.11f", 0xc0000, 0x20000, CRC(86d98ff3) SHA1(18137974fb7812b45f0d93e584ed14d0af2e6a3e) )  // == ar-32m.8h
	ROM_LOAD16_BYTE( "ar_41.11h", 0xc0001, 0x20000, CRC(758893d3) SHA1(1245bfd35b0f12bd701cd28c9ce2e85e166a4de2) )  // == ar-32m.8h

	ROM_REGION( 0x200000, "gfx", 0 )
	ROMX_LOAD( "ar_09.4b",  0x000000, 0x20000, CRC(db9376f8) SHA1(501fc5543c66509f8fc1075c128fb01606ced2ad) , ROM_SKIP(7) ) // == ar-5m.7a
	ROMX_LOAD( "ar_01.4a",  0x000001, 0x20000, CRC(392151b4) SHA1(ba3c69d852f750b43d8b4b2b58fcb7977cc0e5de) , ROM_SKIP(7) ) // == ar-5m.7a
	ROMX_LOAD( "ar_13.9b",  0x000002, 0x20000, CRC(81436481) SHA1(85ceba63382959f7084bacb6aedcef80ddd8ad6b) , ROM_SKIP(7) ) // == ar-7m.9a
	ROMX_LOAD( "ar_05.9a",  0x000003, 0x20000, CRC(e246ed9f) SHA1(aaaad5c81bf7c4ec4b2339fd8f0364096c86e903) , ROM_SKIP(7) ) // == ar-7m.9a
	ROMX_LOAD( "ar_24.5e",  0x000004, 0x20000, CRC(9cd6e2a3) SHA1(186e756af496b0fb5b65cf7a106fe32c78d542c9) , ROM_SKIP(7) ) // == ar-1m.3a
	ROMX_LOAD( "ar_17.5c",  0x000005, 0x20000, CRC(0b8e0df4) SHA1(ac8ab79e7237b72df9f42292a0c58aa56effe3a1) , ROM_SKIP(7) ) // == ar-1m.3a
	ROMX_LOAD( "ar_38.8h",  0x000006, 0x20000, CRC(8b9e75b9) SHA1(eeeaa8f84167f7e8127b90318f052fe5e00c36ac) , ROM_SKIP(7) ) // == ar-3m.5a
	ROMX_LOAD( "ar_32.8f",  0x000007, 0x20000, CRC(db6acdcf) SHA1(5842e29a0e29b4869b2c34a5f47f64c1a1f4609a) , ROM_SKIP(7) ) // == ar-3m.5a
	ROMX_LOAD( "ar_10.5b",  0x100000, 0x20000, CRC(4219b622) SHA1(ecfc47687b466893b9c8587224830d600c754d17) , ROM_SKIP(7) ) // == ar-5m.7a
	ROMX_LOAD( "ar_02.5a",  0x100001, 0x20000, CRC(bac5dec5) SHA1(e69f5c4e5c07db46f088d3eabfd394e7d639fea0) , ROM_SKIP(7) ) // == ar-5m.7a
	ROMX_LOAD( "ar_14.10b", 0x100002, 0x20000, CRC(e6bae179) SHA1(31af958dedce5fcc1a7c159f96af4a3a1a4651fb) , ROM_SKIP(7) ) // == ar-7m.9a
	ROMX_LOAD( "ar_06.10a", 0x100003, 0x20000, CRC(c8f04223) SHA1(c96eba0ce53e8505668dc646344e5b2456d60546) , ROM_SKIP(7) ) // == ar-7m.9a
	ROMX_LOAD( "ar_25.7e",  0x100004, 0x20000, CRC(15ccf981) SHA1(2dd7a2d573089aa70b33586d6d9e6b8d816bd28e) , ROM_SKIP(7) ) // == ar-1m.3a
	ROMX_LOAD( "ar_18.7c",  0x100005, 0x20000, CRC(9336db6a) SHA1(1704d6f0de08ed283c26ee0bcbb82a838060fe70) , ROM_SKIP(7) ) // == ar-1m.3a
	ROMX_LOAD( "ar_39.9h",  0x100006, 0x20000, CRC(9b8e1363) SHA1(f830834305248446235cc6b17b17f7f0dd6baa03) , ROM_SKIP(7) ) // == ar-3m.5a
	ROMX_LOAD( "ar_33.9f",  0x100007, 0x20000, CRC(3968f4b5) SHA1(42722c61c4b514b15f1594fdad688375e2c51e71) , ROM_SKIP(7) ) // == ar-3m.5a

	ROM_REGION( 0x18000, "audiocpu", 0 ) /* 64k for the audio CPU (+banks) */
	ROM_LOAD( "ar_23.13c",  0x00000, 0x08000, CRC(f3dd1367) SHA1(09eef72e862ac6b1a5cce5a45938b45bf4e456ad) )    // == ar_09.12b
	ROM_CONTINUE(           0x10000, 0x08000 )

	ROM_REGION( 0x40000, "oki", 0 ) /* Samples */
	ROM_LOAD( "ar_30.12e",  0x00000, 0x20000, CRC(584b43a9) SHA1(7820815c8c67d484baf2fdad7e55d8c14b98b860) )    // == aru_18.11c
	/* 20000-3ffff empty */

	ROM_REGION( 0x0200, "aboardplds", 0 )
	ROM_LOAD( "buf1",         0x0000, 0x0117, CRC(eb122de7) SHA1(b26b5bfe258e3e184f069719f9fd008d6b8f6b9b) )
	ROM_LOAD( "ioa1",         0x0000, 0x0117, CRC(59c7ee3b) SHA1(fbb887c5b4f5cb8df77cec710eaac2985bc482a6) )
	ROM_LOAD( "prg1",         0x0000, 0x0117, CRC(f1129744) SHA1(a5300f301c1a08a7da768f0773fa0fe3f683b237) )
	ROM_LOAD( "rom1",         0x0000, 0x0117, CRC(41dc73b9) SHA1(7d4c9f1693c821fbf84e32dd6ef62ddf14967845) )
	ROM_LOAD( "sou1",         0x0000, 0x0117, CRC(84f4b2fe) SHA1(dcc9e86cc36316fe42eace02d6df75d08bc8bb6d) )

	ROM_REGION( 0x0200, "bboardplds", 0 )
	ROM_LOAD( "ar22b.1a",     0x0000, 0x0117, CRC(f1db9030) SHA1(85a6eefb93e7bf7c7d3980737365a425c5324c08) )
	ROM_LOAD( "lwio.12c",     0x0000, 0x0117, CRC(ad52b90c) SHA1(f0fd6aeea515ee449320fe15684e6b3ab7f97bf4) )
ROM_END

/* B-Board 91634B-2, Japan Resale Ver. */
ROM_START( area88r )
	ROM_REGION( CODE_SIZE, "maincpu", 0 )      /* 68000 code */
	ROM_LOAD16_WORD_SWAP( "araj_23.8f", 0x00000, 0x80000, CRC(7045d6cb) SHA1(5f8f3160e8e960ee67778232368d4f3430a0dade) )
	ROM_LOAD16_WORD_SWAP( "araj_22.7f", 0x80000, 0x80000, CRC(9913002e) SHA1(b2da8ad34bf4ea51679d35d91601faa91ef40ae5) )

	ROM_REGION( 0x200000, "gfx", 0 )
	ROMX_LOAD( "ara_01.3a", 0x000000, 0x80000, CRC(bf4575d8) SHA1(1b268e1698be8ff9c16f80f7b9081b6be9f72601) , ROM_GROUPWORD | ROM_SKIP(6) ) // == ar-5m.7a
	ROMX_LOAD( "ara_02.4a", 0x000002, 0x80000, CRC(a02945f4) SHA1(ff35cdbd6c1e43b16a906f68e416559cb3d5746b) , ROM_GROUPWORD | ROM_SKIP(6) ) // == ar-7m.9a
	ROMX_LOAD( "ara_03.5a", 0x000004, 0x80000, CRC(5965ca8d) SHA1(49abf80fc012a7f73306139a2871aeac7fd6a3d0) , ROM_GROUPWORD | ROM_SKIP(6) ) // == ar-1m.3a
	ROMX_LOAD( "ara_04.6a", 0x000006, 0x80000, CRC(ac6db17d) SHA1(78eef9ba6a392859f70467f6d7cb5aa91964abed) , ROM_GROUPWORD | ROM_SKIP(6) ) // == ar-3m.5a

	ROM_REGION( 0x28000, "audiocpu", 0 ) /* 64k for the audio CPU (+banks) */
	ROM_LOAD( "ara_09.12a", 0x00000, 0x08000, CRC(af88359c) SHA1(5b7d872700a074fe1a4e11722e73374a23fa5e99) )    // == ar_09.12b + garbage
	ROM_CONTINUE(           0x10000, 0x18000 )  // second half of ROM is unused, not mapped in memory

	ROM_REGION( 0x40000, "oki", 0 ) /* Samples */
	ROM_LOAD( "ara_18.11c", 0x00000, 0x20000, CRC(584b43a9) SHA1(7820815c8c67d484baf2fdad7e55d8c14b98b860) )    // == aru_18.11c
	/* 20000-3ffff empty */

	ROM_REGION( 0x0200, "aboardplds", 0 )
	ROM_LOAD( "buf1",         0x0000, 0x0117, CRC(eb122de7) SHA1(b26b5bfe258e3e184f069719f9fd008d6b8f6b9b) )
	ROM_LOAD( "ioa1",         0x0000, 0x0117, CRC(59c7ee3b) SHA1(fbb887c5b4f5cb8df77cec710eaac2985bc482a6) )
	ROM_LOAD( "prg1",         0x0000, 0x0117, CRC(f1129744) SHA1(a5300f301c1a08a7da768f0773fa0fe3f683b237) )
	ROM_LOAD( "rom1",         0x0000, 0x0117, CRC(41dc73b9) SHA1(7d4c9f1693c821fbf84e32dd6ef62ddf14967845) )
	ROM_LOAD( "sou1",         0x0000, 0x0117, CRC(84f4b2fe) SHA1(dcc9e86cc36316fe42eace02d6df75d08bc8bb6d) )

	ROM_REGION( 0x0200, "bboardplds", 0 )
	ROM_LOAD( "ara63b.1a",    0x0000, 0x0117, BAD_DUMP CRC(f5569c93) SHA1(7db7cf23639036590eef1e5e309f08560859efaf) ) /* Handcrafted but works on actual PCB.  Redump needed */
	ROM_LOAD( "iob1.12d",     0x0000, 0x0117, CRC(3abc0700) SHA1(973043aa46ec6d5d1db20dc9d5937005a0f9f6ae) )
	ROM_LOAD( "bprg1.11d",    0x0000, 0x0117, CRC(31793da7) SHA1(400fa7ac517421c978c1ee7773c30b9ed0c5d3f3) )

	ROM_REGION( 0x0200, "cboardplds", 0 )
	ROM_LOAD( "ioc1.ic7",     0x0000, 0x0117, CRC(0d182081) SHA1(475b3d417785da4bc512cce2b274bb00d4cc6792) )
	ROM_LOAD( "c632.ic1",     0x0000, 0x0117, CRC(0fbd9270) SHA1(d7e737b20c44d41e29ca94be56114b31934dde81) )
ROM_END

/* B-Board 89624B-3 */
ROM_START( ffight )
	ROM_REGION( CODE_SIZE, "maincpu", 0 )      /* 68000 code */
	ROM_LOAD16_BYTE( "ff_36.11f",      0x00000, 0x20000, CRC(f9a5ce83) SHA1(0756ae576a1f6d5b8b22f8630dca40ef38567ea6) ) // in "30" socket
	ROM_LOAD16_BYTE( "ff_42.11h",      0x00001, 0x20000, CRC(65f11215) SHA1(5045a467f3e228c02b4a355b52f58263ffa90113) ) // in "35" socket
	ROM_LOAD16_BYTE( "ff_37.12f",      0x40000, 0x20000, CRC(e1033784) SHA1(38f44434c8befd623953ae23d6e5ff4e201d6627) ) // in "31" socket
	ROM_LOAD16_BYTE( "ffe_43.12h",     0x40001, 0x20000, CRC(995e968a) SHA1(de16873d1639ac1738be0937270b108a9914f263) ) // in "36" socket
	ROM_LOAD16_WORD_SWAP( "ff-32m.8h", 0x80000, 0x80000, CRC(c747696e) SHA1(d3362dadded31ccb7eaf71ef282d698d18edd722) )

	ROM_REGION( 0x200000, "gfx", 0 )
	ROMX_LOAD( "ff-5m.7a", 0x000000, 0x80000, CRC(9c284108) SHA1(7868f5801347340867720255f8380548ad1a65a7) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "ff-7m.9a", 0x000002, 0x80000, CRC(a7584dfb) SHA1(f7b00a3ca8cb85264ab293089f9f540a8292b49c) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "ff-1m.3a", 0x000004, 0x80000, CRC(0b605e44) SHA1(5ce16af72858a57aefbf6efed820c2c51935882a) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "ff-3m.5a", 0x000006, 0x80000, CRC(52291cd2) SHA1(df5f3d3aa96a7a33ff22f2a31382942c4c4f1111) , ROM_GROUPWORD | ROM_SKIP(6) )

	ROM_REGION( 0x18000, "audiocpu", 0 ) /* 64k for the audio CPU (+banks) */
	ROM_LOAD( "ff_09.12b", 0x00000, 0x08000, CRC(b8367eb5) SHA1(ec3db29fdd6200e9a8f4f8073a7e34aef731354f) )
	ROM_CONTINUE(          0x10000, 0x08000 )

	ROM_REGION( 0x40000, "oki", 0 ) /* Samples */
	ROM_LOAD( "ff_18.11c", 0x00000, 0x20000, CRC(375c66e7) SHA1(36189e23209ce4ae5d9cbabd1574540d0591e7b3) )
	ROM_LOAD( "ff_19.12c", 0x20000, 0x20000, CRC(1ef137f9) SHA1(974b5e72aa28b87ebfa7438efbdfeda769dedf5e) )

	ROM_REGION( 0x0200, "aboardplds", 0 )
	ROM_LOAD( "buf1",         0x0000, 0x0117, CRC(eb122de7) SHA1(b26b5bfe258e3e184f069719f9fd008d6b8f6b9b) )
	ROM_LOAD( "ioa1",         0x0000, 0x0117, CRC(59c7ee3b) SHA1(fbb887c5b4f5cb8df77cec710eaac2985bc482a6) )
	ROM_LOAD( "prg1",         0x0000, 0x0117, CRC(f1129744) SHA1(a5300f301c1a08a7da768f0773fa0fe3f683b237) )
	ROM_LOAD( "rom1",         0x0000, 0x0117, CRC(41dc73b9) SHA1(7d4c9f1693c821fbf84e32dd6ef62ddf14967845) )
	ROM_LOAD( "sou1",         0x0000, 0x0117, CRC(84f4b2fe) SHA1(dcc9e86cc36316fe42eace02d6df75d08bc8bb6d) )

	ROM_REGION( 0x0200, "bboardplds", 0 )
	ROM_LOAD( "s224b.1a",     0x0000, 0x0117, CRC(cdc4413e) SHA1(c74c60f8f4eb125ffb6414aefba270676d9b8a2d) )
	ROM_LOAD( "iob1.11e",     0x0000, 0x0117, CRC(3abc0700) SHA1(973043aa46ec6d5d1db20dc9d5937005a0f9f6ae) )    /* seen the same pcb with LWIO.11E */
ROM_END

/* B-Board 89624B-3 */
ROM_START( ffighta )
	ROM_REGION( CODE_SIZE, "maincpu", 0 )      /* 68000 code */
	ROM_LOAD16_BYTE( "ffe_30.11f",     0x00000, 0x20000, CRC(2347bf51) SHA1(de05b347a8b4df4889d4e594e1abf741afdf0ab4) )
	ROM_LOAD16_BYTE( "ffe_35.11h",     0x00001, 0x20000, CRC(5f694ecc) SHA1(57c45ffcada0cdaf1f97c7e33c8300539828a238) )
	ROM_LOAD16_BYTE( "ffe_31.12f",     0x40000, 0x20000, CRC(6dc6b792) SHA1(553abebed2a1fa1ee2d85a4117f40d90e2321cea) )
	ROM_LOAD16_BYTE( "ffe_36.12h",     0x40001, 0x20000, CRC(b36a0b99) SHA1(cfb27987ee3d54c83c6fb73437a79dcdd96df22f) )
	ROM_LOAD16_WORD_SWAP( "ff-32m.8h", 0x80000, 0x80000, CRC(c747696e) SHA1(d3362dadded31ccb7eaf71ef282d698d18edd722) )

	ROM_REGION( 0x200000, "gfx", 0 )
	ROMX_LOAD( "ff-5m.7a",  0x000000, 0x80000, CRC(9c284108) SHA1(7868f5801347340867720255f8380548ad1a65a7) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "ff-7m.9a",  0x000002, 0x80000, CRC(a7584dfb) SHA1(f7b00a3ca8cb85264ab293089f9f540a8292b49c) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "ff-1m.3a",  0x000004, 0x80000, CRC(0b605e44) SHA1(5ce16af72858a57aefbf6efed820c2c51935882a) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "ff-3m.5a",  0x000006, 0x80000, CRC(52291cd2) SHA1(df5f3d3aa96a7a33ff22f2a31382942c4c4f1111) , ROM_GROUPWORD | ROM_SKIP(6) )

	ROM_REGION( 0x18000, "audiocpu", 0 ) /* 64k for the audio CPU (+banks) */
	ROM_LOAD( "ffe_23.12b", 0x00000, 0x08000, CRC(b8367eb5) SHA1(ec3db29fdd6200e9a8f4f8073a7e34aef731354f) )    // == ff_09.12b /* label is FFE_23, pcb verified */
	ROM_CONTINUE(           0x10000, 0x08000 )

	ROM_REGION( 0x40000, "oki", 0 ) /* Samples */
	ROM_LOAD( "ff_18.11c",  0x00000, 0x20000, CRC(375c66e7) SHA1(36189e23209ce4ae5d9cbabd1574540d0591e7b3) )
	ROM_LOAD( "ff_19.12c",  0x20000, 0x20000, CRC(1ef137f9) SHA1(974b5e72aa28b87ebfa7438efbdfeda769dedf5e) )

	ROM_REGION( 0x0200, "aboardplds", 0 )
	ROM_LOAD( "buf1",         0x0000, 0x0117, CRC(eb122de7) SHA1(b26b5bfe258e3e184f069719f9fd008d6b8f6b9b) )
	ROM_LOAD( "ioa1",         0x0000, 0x0117, CRC(59c7ee3b) SHA1(fbb887c5b4f5cb8df77cec710eaac2985bc482a6) )
	ROM_LOAD( "prg1",         0x0000, 0x0117, CRC(f1129744) SHA1(a5300f301c1a08a7da768f0773fa0fe3f683b237) )
	ROM_LOAD( "rom1",         0x0000, 0x0117, CRC(41dc73b9) SHA1(7d4c9f1693c821fbf84e32dd6ef62ddf14967845) )
	ROM_LOAD( "sou1",         0x0000, 0x0117, CRC(84f4b2fe) SHA1(dcc9e86cc36316fe42eace02d6df75d08bc8bb6d) )

	ROM_REGION( 0x0200, "bboardplds", 0 )
	ROM_LOAD( "s224b.1a",     0x0000, 0x0117, CRC(cdc4413e) SHA1(c74c60f8f4eb125ffb6414aefba270676d9b8a2d) )
	ROM_LOAD( "lwio.11e",     0x0000, 0x0117, CRC(ad52b90c) SHA1(f0fd6aeea515ee449320fe15684e6b3ab7f97bf4) )
ROM_END

/* B-Board 89624B-3 */
ROM_START( ffightu )
	ROM_REGION( CODE_SIZE, "maincpu", 0 )      /* 68000 code */
	ROM_LOAD16_BYTE( "ff_36.11f",      0x00000, 0x20000, CRC(f9a5ce83) SHA1(0756ae576a1f6d5b8b22f8630dca40ef38567ea6) ) // in "30" socket
	ROM_LOAD16_BYTE( "ff_42.11h",      0x00001, 0x20000, CRC(65f11215) SHA1(5045a467f3e228c02b4a355b52f58263ffa90113) ) // in "35" socket
	ROM_LOAD16_BYTE( "ff_37.12f",      0x40000, 0x20000, CRC(e1033784) SHA1(38f44434c8befd623953ae23d6e5ff4e201d6627) ) // in "31" socket
	ROM_LOAD16_BYTE( "ffu_43.12h",     0x40001, 0x20000, CRC(4ca65947) SHA1(74ffe00df96273770a24d9a46f13e53ea8812670) ) // in "36" socket   /* seen the same pcb with FF_43.12H */
	ROM_LOAD16_WORD_SWAP( "ff-32m.8h", 0x80000, 0x80000, CRC(c747696e) SHA1(d3362dadded31ccb7eaf71ef282d698d18edd722) )

	ROM_REGION( 0x200000, "gfx", 0 )
	ROMX_LOAD( "ff-5m.7a", 0x000000, 0x80000, CRC(9c284108) SHA1(7868f5801347340867720255f8380548ad1a65a7) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "ff-7m.9a", 0x000002, 0x80000, CRC(a7584dfb) SHA1(f7b00a3ca8cb85264ab293089f9f540a8292b49c) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "ff-1m.3a", 0x000004, 0x80000, CRC(0b605e44) SHA1(5ce16af72858a57aefbf6efed820c2c51935882a) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "ff-3m.5a", 0x000006, 0x80000, CRC(52291cd2) SHA1(df5f3d3aa96a7a33ff22f2a31382942c4c4f1111) , ROM_GROUPWORD | ROM_SKIP(6) )

	ROM_REGION( 0x18000, "audiocpu", 0 ) /* 64k for the audio CPU (+banks) */
	ROM_LOAD( "ff_09.12b", 0x00000, 0x08000, CRC(b8367eb5) SHA1(ec3db29fdd6200e9a8f4f8073a7e34aef731354f) ) /* seen the same pcb with 23.12B */
	ROM_CONTINUE(          0x10000, 0x08000 )

	ROM_REGION( 0x40000, "oki", 0 ) /* Samples */
	ROM_LOAD( "ff_18.11c", 0x00000, 0x20000, CRC(375c66e7) SHA1(36189e23209ce4ae5d9cbabd1574540d0591e7b3) )
	ROM_LOAD( "ff_19.12c", 0x20000, 0x20000, CRC(1ef137f9) SHA1(974b5e72aa28b87ebfa7438efbdfeda769dedf5e) )

	ROM_REGION( 0x0200, "aboardplds", 0 )
	ROM_LOAD( "buf1",         0x0000, 0x0117, CRC(eb122de7) SHA1(b26b5bfe258e3e184f069719f9fd008d6b8f6b9b) )
	ROM_LOAD( "ioa1",         0x0000, 0x0117, CRC(59c7ee3b) SHA1(fbb887c5b4f5cb8df77cec710eaac2985bc482a6) )
	ROM_LOAD( "prg1",         0x0000, 0x0117, CRC(f1129744) SHA1(a5300f301c1a08a7da768f0773fa0fe3f683b237) )
	ROM_LOAD( "rom1",         0x0000, 0x0117, CRC(41dc73b9) SHA1(7d4c9f1693c821fbf84e32dd6ef62ddf14967845) )
	ROM_LOAD( "sou1",         0x0000, 0x0117, CRC(84f4b2fe) SHA1(dcc9e86cc36316fe42eace02d6df75d08bc8bb6d) )

	ROM_REGION( 0x0200, "bboardplds", 0 )
	ROM_LOAD( "s224b.1a",     0x0000, 0x0117, CRC(cdc4413e) SHA1(c74c60f8f4eb125ffb6414aefba270676d9b8a2d) )
	ROM_LOAD( "iob1.11e",     0x0000, 0x0117, CRC(3abc0700) SHA1(973043aa46ec6d5d1db20dc9d5937005a0f9f6ae) )
ROM_END

/* B-Board 89624B-3 */
/* Note that this set is equivalent to ffightu, but the 4Mbit MASK ROM FF-32M located @ 8H is replaced by 4 1Mbit EPROMs. */
ROM_START( ffightu1 )
	ROM_REGION( CODE_SIZE, "maincpu", 0 )      /* 68000 code */
	ROM_LOAD16_BYTE( "ff_36.11f",  0x00000, 0x20000, CRC(f9a5ce83) SHA1(0756ae576a1f6d5b8b22f8630dca40ef38567ea6) ) // in "30" socket
	ROM_LOAD16_BYTE( "ff_42.11h",  0x00001, 0x20000, CRC(65f11215) SHA1(5045a467f3e228c02b4a355b52f58263ffa90113) ) // in "35" socket
	ROM_LOAD16_BYTE( "ff_37.12f",  0x40000, 0x20000, CRC(e1033784) SHA1(38f44434c8befd623953ae23d6e5ff4e201d6627) ) // in "31" socket
	ROM_LOAD16_BYTE( "ffu_43.12h", 0x40001, 0x20000, CRC(4ca65947) SHA1(74ffe00df96273770a24d9a46f13e53ea8812670) ) // in "36" socket
	ROM_LOAD16_BYTE( "ff_34.9f",   0x80000, 0x20000, CRC(0c8dc3fc) SHA1(edcce3efd9cdd131ef0c96df15a68722d5c3498e) ) // in "28" socket   // == ff-32m.8h
	ROM_LOAD16_BYTE( "ff_40.9h",   0x80001, 0x20000, CRC(8075bab9) SHA1(f9c7405133f6fc5557c90e60e8ccc459e4f6fd7d) ) // in "33" socket   // == ff-32m.8h
	ROM_LOAD16_BYTE( "ff_35.10f",  0xc0000, 0x20000, CRC(4a934121) SHA1(3982c261582755a0eac340d6d7ed96e6c263c8b6) ) // in "29" socket   // == ff-32m.8h
	ROM_LOAD16_BYTE( "ff_41.10h",  0xc0001, 0x20000, CRC(2af68154) SHA1(7d549cb38650b4b79c68ad6d0dfcefdd62be4e99) ) // in "34" socket   // == ff-32m.8h

	ROM_REGION( 0x200000, "gfx", 0 )
	ROMX_LOAD( "ff-5m.7a", 0x000000, 0x80000, CRC(9c284108) SHA1(7868f5801347340867720255f8380548ad1a65a7) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "ff-7m.9a", 0x000002, 0x80000, CRC(a7584dfb) SHA1(f7b00a3ca8cb85264ab293089f9f540a8292b49c) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "ff-1m.3a", 0x000004, 0x80000, CRC(0b605e44) SHA1(5ce16af72858a57aefbf6efed820c2c51935882a) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "ff-3m.5a", 0x000006, 0x80000, CRC(52291cd2) SHA1(df5f3d3aa96a7a33ff22f2a31382942c4c4f1111) , ROM_GROUPWORD | ROM_SKIP(6) )

	ROM_REGION( 0x18000, "audiocpu", 0 ) /* 64k for the audio CPU (+banks) */
	ROM_LOAD( "ff_09.12b", 0x00000, 0x08000, CRC(b8367eb5) SHA1(ec3db29fdd6200e9a8f4f8073a7e34aef731354f) )
	ROM_CONTINUE(          0x10000, 0x08000 )

	ROM_REGION( 0x40000, "oki", 0 ) /* Samples */
	ROM_LOAD( "ff_18.11c", 0x00000, 0x20000, CRC(375c66e7) SHA1(36189e23209ce4ae5d9cbabd1574540d0591e7b3) )
	ROM_LOAD( "ff_19.12c", 0x20000, 0x20000, CRC(1ef137f9) SHA1(974b5e72aa28b87ebfa7438efbdfeda769dedf5e) )

	ROM_REGION( 0x0200, "aboardplds", 0 )
	ROM_LOAD( "buf1",         0x0000, 0x0117, CRC(eb122de7) SHA1(b26b5bfe258e3e184f069719f9fd008d6b8f6b9b) )
	ROM_LOAD( "ioa1",         0x0000, 0x0117, CRC(59c7ee3b) SHA1(fbb887c5b4f5cb8df77cec710eaac2985bc482a6) )
	ROM_LOAD( "prg1",         0x0000, 0x0117, CRC(f1129744) SHA1(a5300f301c1a08a7da768f0773fa0fe3f683b237) )
	ROM_LOAD( "rom1",         0x0000, 0x0117, CRC(41dc73b9) SHA1(7d4c9f1693c821fbf84e32dd6ef62ddf14967845) )
	ROM_LOAD( "sou1",         0x0000, 0x0117, CRC(84f4b2fe) SHA1(dcc9e86cc36316fe42eace02d6df75d08bc8bb6d) )

	ROM_REGION( 0x0200, "bboardplds", 0 )
	ROM_LOAD( "s224b.1a",     0x0000, 0x0117, CRC(cdc4413e) SHA1(c74c60f8f4eb125ffb6414aefba270676d9b8a2d) )
	ROM_LOAD( "iob1.11e",     0x0000, 0x0117, CRC(3abc0700) SHA1(973043aa46ec6d5d1db20dc9d5937005a0f9f6ae) )
ROM_END

/* B-Board 89624B-3 */
ROM_START( ffightua )
	ROM_REGION( CODE_SIZE, "maincpu", 0 )      /* 68000 code */
	ROM_LOAD16_BYTE( "ffu_36.11f",     0x00000, 0x20000, CRC(e2a48af9) SHA1(11e06f95bdf575af396dded2b84d858f6c7388f1) ) // in "30" socket
	ROM_LOAD16_BYTE( "ffu_42.11h",     0x00001, 0x20000, CRC(f4bb480e) SHA1(32114df1d2f4f98a4a2280a330c7b6af8ab4d862) ) // in "35" socket
	ROM_LOAD16_BYTE( "ffu_37.12f",     0x40000, 0x20000, CRC(c371c667) SHA1(633977c91a8ff09b7fe83128eced7c4dee9aee1d) ) // in "31" socket
	ROM_LOAD16_BYTE( "ffu_43.12h",     0x40001, 0x20000, CRC(2f5771f9) SHA1(fb532402bc00b5619a23dfa7e4525f1717020303) ) // in "36" socket   /* different CRC from ffightu, ffightu1, pcb verified */
	ROM_LOAD16_WORD_SWAP( "ff-32m.8h", 0x80000, 0x80000, CRC(c747696e) SHA1(d3362dadded31ccb7eaf71ef282d698d18edd722) )

	ROM_REGION( 0x200000, "gfx", 0 )
	ROMX_LOAD( "ff-5m.7a", 0x000000, 0x80000, CRC(9c284108) SHA1(7868f5801347340867720255f8380548ad1a65a7) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "ff-7m.9a", 0x000002, 0x80000, CRC(a7584dfb) SHA1(f7b00a3ca8cb85264ab293089f9f540a8292b49c) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "ff-1m.3a", 0x000004, 0x80000, CRC(0b605e44) SHA1(5ce16af72858a57aefbf6efed820c2c51935882a) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "ff-3m.5a", 0x000006, 0x80000, CRC(52291cd2) SHA1(df5f3d3aa96a7a33ff22f2a31382942c4c4f1111) , ROM_GROUPWORD | ROM_SKIP(6) )

	ROM_REGION( 0x18000, "audiocpu", 0 ) /* 64k for the audio CPU (+banks) */
	ROM_LOAD( "ff_09.12b", 0x00000, 0x08000, CRC(b8367eb5) SHA1(ec3db29fdd6200e9a8f4f8073a7e34aef731354f) )
	ROM_CONTINUE(          0x10000, 0x08000 )

	ROM_REGION( 0x40000, "oki", 0 ) /* Samples */
	ROM_LOAD( "ff_18.11c", 0x00000, 0x20000, CRC(375c66e7) SHA1(36189e23209ce4ae5d9cbabd1574540d0591e7b3) )
	ROM_LOAD( "ff_19.12c", 0x20000, 0x20000, CRC(1ef137f9) SHA1(974b5e72aa28b87ebfa7438efbdfeda769dedf5e) )

	ROM_REGION( 0x0200, "aboardplds", 0 )
	ROM_LOAD( "buf1",         0x0000, 0x0117, CRC(eb122de7) SHA1(b26b5bfe258e3e184f069719f9fd008d6b8f6b9b) )
	ROM_LOAD( "ioa1",         0x0000, 0x0117, CRC(59c7ee3b) SHA1(fbb887c5b4f5cb8df77cec710eaac2985bc482a6) )
	ROM_LOAD( "prg1",         0x0000, 0x0117, CRC(f1129744) SHA1(a5300f301c1a08a7da768f0773fa0fe3f683b237) )
	ROM_LOAD( "rom1",         0x0000, 0x0117, CRC(41dc73b9) SHA1(7d4c9f1693c821fbf84e32dd6ef62ddf14967845) )
	ROM_LOAD( "sou1",         0x0000, 0x0117, CRC(84f4b2fe) SHA1(dcc9e86cc36316fe42eace02d6df75d08bc8bb6d) )

	ROM_REGION( 0x0200, "bboardplds", 0 )
	ROM_LOAD( "s224b.1a",     0x0000, 0x0117, CRC(cdc4413e) SHA1(c74c60f8f4eb125ffb6414aefba270676d9b8a2d) )
	ROM_LOAD( "iob1.11e",     0x0000, 0x0117, CRC(3abc0700) SHA1(973043aa46ec6d5d1db20dc9d5937005a0f9f6ae) )
ROM_END

/* B-Board 89624B-3 */
ROM_START( ffightub )
	ROM_REGION( CODE_SIZE, "maincpu", 0 )      /* 68000 code */
	ROM_LOAD16_BYTE( "ffu_30_3.11f",   0x00000, 0x20000, CRC(e619eb30) SHA1(41c2589a1b2cab2d0ded527a89f8e0e39e61efe1))
	ROM_LOAD16_BYTE( "ffu_35_3.11h",   0x00001, 0x20000, CRC(bca85263) SHA1(249bc81426ee93cf2efa5594d6813d5dd896cea3) )
	ROM_LOAD16_BYTE( "ffu_31_3.12f",   0x40000, 0x20000, CRC(59abd207) SHA1(9bc1f4d5dabd02cebce84f56f848694591c0629d) )
	ROM_LOAD16_BYTE( "ffu_36_3.12h",   0x40001, 0x20000, CRC(df46ece8) SHA1(aa3081918d499f56664d6bb7b7ede2055d00375d) )
	ROM_LOAD16_WORD_SWAP( "ff-32m.8h", 0x80000, 0x80000, CRC(c747696e) SHA1(d3362dadded31ccb7eaf71ef282d698d18edd722) )

	ROM_REGION( 0x200000, "gfx", 0 )
	ROMX_LOAD( "ff-5m.7a", 0x000000, 0x80000, CRC(9c284108) SHA1(7868f5801347340867720255f8380548ad1a65a7) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "ff-7m.9a", 0x000002, 0x80000, CRC(a7584dfb) SHA1(f7b00a3ca8cb85264ab293089f9f540a8292b49c) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "ff-1m.3a", 0x000004, 0x80000, CRC(0b605e44) SHA1(5ce16af72858a57aefbf6efed820c2c51935882a) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "ff-3m.5a", 0x000006, 0x80000, CRC(52291cd2) SHA1(df5f3d3aa96a7a33ff22f2a31382942c4c4f1111) , ROM_GROUPWORD | ROM_SKIP(6) )

	ROM_REGION( 0x18000, "audiocpu", 0 ) /* 64k for the audio CPU (+banks) */
	ROM_LOAD( "ff_09.12b", 0x00000, 0x08000, CRC(b8367eb5) SHA1(ec3db29fdd6200e9a8f4f8073a7e34aef731354f) )
	ROM_CONTINUE(          0x10000, 0x08000 )

	ROM_REGION( 0x40000, "oki", 0 ) /* Samples */
	ROM_LOAD( "ff_18.11c", 0x00000, 0x20000, CRC(375c66e7) SHA1(36189e23209ce4ae5d9cbabd1574540d0591e7b3) )
	ROM_LOAD( "ff_19.12c", 0x20000, 0x20000, CRC(1ef137f9) SHA1(974b5e72aa28b87ebfa7438efbdfeda769dedf5e) )

	ROM_REGION( 0x0200, "aboardplds", 0 )
	ROM_LOAD( "buf1",         0x0000, 0x0117, CRC(eb122de7) SHA1(b26b5bfe258e3e184f069719f9fd008d6b8f6b9b) )
	ROM_LOAD( "ioa1",         0x0000, 0x0117, CRC(59c7ee3b) SHA1(fbb887c5b4f5cb8df77cec710eaac2985bc482a6) )
	ROM_LOAD( "prg1",         0x0000, 0x0117, CRC(f1129744) SHA1(a5300f301c1a08a7da768f0773fa0fe3f683b237) )
	ROM_LOAD( "rom1",         0x0000, 0x0117, CRC(41dc73b9) SHA1(7d4c9f1693c821fbf84e32dd6ef62ddf14967845) )
	ROM_LOAD( "sou1",         0x0000, 0x0117, CRC(84f4b2fe) SHA1(dcc9e86cc36316fe42eace02d6df75d08bc8bb6d) )

	ROM_REGION( 0x0200, "bboardplds", 0 )
	ROM_LOAD( "s224b.1a",     0x0000, 0x0117, CRC(cdc4413e) SHA1(c74c60f8f4eb125ffb6414aefba270676d9b8a2d) )
	ROM_LOAD( "iob1.11e",     0x0000, 0x0117, CRC(3abc0700) SHA1(973043aa46ec6d5d1db20dc9d5937005a0f9f6ae) )
ROM_END

/* B-Board 89624B-3 */
ROM_START( ffightuc )
	ROM_REGION( CODE_SIZE, "maincpu", 0 )      /* 68000 code */
	ROM_LOAD16_BYTE( "ffu_30.11f",     0x00000, 0x20000, CRC(ed988977) SHA1(c718e989206bd2b68832c8fcb5667397d500ebac) )
	ROM_LOAD16_BYTE( "ffu_35.11h",     0x00001, 0x20000, CRC(07bf1c21) SHA1(f21a939fd92607c7f54816dedbcb3c5818cf4183) )
	ROM_LOAD16_BYTE( "ffu_31.12f",     0x40000, 0x20000, CRC(dba5a476) SHA1(2f0176dd050f9630b914f1c1ca5d96215bcf567f) )
	ROM_LOAD16_BYTE( "ffu_36.12h",     0x40001, 0x20000, CRC(4d89f542) SHA1(0b7d483a2c5759715f99f287cbd8a36165b59de7) )
	ROM_LOAD16_WORD_SWAP( "ff-32m.8h", 0x80000, 0x80000, CRC(c747696e) SHA1(d3362dadded31ccb7eaf71ef282d698d18edd722) )

	ROM_REGION( 0x200000, "gfx", 0 )
	ROMX_LOAD( "ff-5m.7a", 0x000000, 0x80000, CRC(9c284108) SHA1(7868f5801347340867720255f8380548ad1a65a7) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "ff-7m.9a", 0x000002, 0x80000, CRC(a7584dfb) SHA1(f7b00a3ca8cb85264ab293089f9f540a8292b49c) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "ff-1m.3a", 0x000004, 0x80000, CRC(0b605e44) SHA1(5ce16af72858a57aefbf6efed820c2c51935882a) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "ff-3m.5a", 0x000006, 0x80000, CRC(52291cd2) SHA1(df5f3d3aa96a7a33ff22f2a31382942c4c4f1111) , ROM_GROUPWORD | ROM_SKIP(6) )

	ROM_REGION( 0x18000, "audiocpu", 0 ) /* 64k for the audio CPU (+banks) */
	ROM_LOAD( "ff_23.12b", 0x00000, 0x08000, CRC(b8367eb5) SHA1(ec3db29fdd6200e9a8f4f8073a7e34aef731354f) ) // == ff_09.12b /* label is FF_23, pcb verified */
	ROM_CONTINUE(          0x10000, 0x08000 )

	ROM_REGION( 0x40000, "oki", 0 ) /* Samples */
	ROM_LOAD( "ff_18.11c", 0x00000, 0x20000, CRC(375c66e7) SHA1(36189e23209ce4ae5d9cbabd1574540d0591e7b3) )
	ROM_LOAD( "ff_19.12c", 0x20000, 0x20000, CRC(1ef137f9) SHA1(974b5e72aa28b87ebfa7438efbdfeda769dedf5e) )

	ROM_REGION( 0x0200, "aboardplds", 0 )
	ROM_LOAD( "buf1",         0x0000, 0x0117, CRC(eb122de7) SHA1(b26b5bfe258e3e184f069719f9fd008d6b8f6b9b) )
	ROM_LOAD( "ioa1",         0x0000, 0x0117, CRC(59c7ee3b) SHA1(fbb887c5b4f5cb8df77cec710eaac2985bc482a6) )
	ROM_LOAD( "prg1",         0x0000, 0x0117, CRC(f1129744) SHA1(a5300f301c1a08a7da768f0773fa0fe3f683b237) )
	ROM_LOAD( "rom1",         0x0000, 0x0117, CRC(41dc73b9) SHA1(7d4c9f1693c821fbf84e32dd6ef62ddf14967845) )
	ROM_LOAD( "sou1",         0x0000, 0x0117, CRC(84f4b2fe) SHA1(dcc9e86cc36316fe42eace02d6df75d08bc8bb6d) )

	ROM_REGION( 0x0200, "bboardplds", 0 )
	ROM_LOAD( "s224b.1a",     0x0000, 0x0117, CRC(cdc4413e) SHA1(c74c60f8f4eb125ffb6414aefba270676d9b8a2d) )
	ROM_LOAD( "iob1.11e",     0x0000, 0x0117, CRC(3abc0700) SHA1(973043aa46ec6d5d1db20dc9d5937005a0f9f6ae) )
ROM_END

/* FIXME B-Board uncertain but should be 88622B/89625B from the program ROM names */
ROM_START( ffightj )
	ROM_REGION( CODE_SIZE, "maincpu", 0 )      /* 68000 code */
	ROM_LOAD16_BYTE( "ff36.bin",    0x00000, 0x20000, CRC(f9a5ce83) SHA1(0756ae576a1f6d5b8b22f8630dca40ef38567ea6) )    // == ff_36.11f
	ROM_LOAD16_BYTE( "ff42.bin",    0x00001, 0x20000, CRC(65f11215) SHA1(5045a467f3e228c02b4a355b52f58263ffa90113) )    // == ff_42.11h
	ROM_LOAD16_BYTE( "ff37.bin",    0x40000, 0x20000, CRC(e1033784) SHA1(38f44434c8befd623953ae23d6e5ff4e201d6627) )    // == ff_37.12f
	ROM_LOAD16_BYTE( "ff43.bin",    0x40001, 0x20000, CRC(b6dee1c3) SHA1(3a85312a2e9d8e06259b73d91ccb5e66a6bad62d) )
	ROM_LOAD16_BYTE( "ffj_34.10f",  0x80000, 0x20000, CRC(0c8dc3fc) SHA1(edcce3efd9cdd131ef0c96df15a68722d5c3498e) )    // == ff-32m.8h
	ROM_LOAD16_BYTE( "ffj_40.10h",  0x80001, 0x20000, CRC(8075bab9) SHA1(f9c7405133f6fc5557c90e60e8ccc459e4f6fd7d) )    // == ff-32m.8h
	ROM_LOAD16_BYTE( "ffj_35.11f",  0xc0000, 0x20000, CRC(4a934121) SHA1(3982c261582755a0eac340d6d7ed96e6c263c8b6) )    // == ff-32m.8h
	ROM_LOAD16_BYTE( "ffj_41.11h",  0xc0001, 0x20000, CRC(2af68154) SHA1(7d549cb38650b4b79c68ad6d0dfcefdd62be4e99) )    // == ff-32m.8h

	ROM_REGION( 0x200000, "gfx", 0 )
	ROMX_LOAD( "ffj_09.4b",  0x000000, 0x20000, CRC(5b116d0d) SHA1(a24e829fdfa043bd27b508d7cc0788ad80fd180e) , ROM_SKIP(7) )
	ROMX_LOAD( "ffj_01.4a",  0x000001, 0x20000, CRC(815b1797) SHA1(549e5eefc8f607fec1c954ba715ff21b8e44a5aa) , ROM_SKIP(7) )
	ROMX_LOAD( "ffj_13.9b",  0x000002, 0x20000, CRC(8721a7da) SHA1(39b2b324fd7810342503f23695e423f364a6294d) , ROM_SKIP(7) )
	ROMX_LOAD( "ffj_05.9a",  0x000003, 0x20000, CRC(d0fcd4b5) SHA1(97ebcbead0cca7e425143c500c433dbcf9cadcc2) , ROM_SKIP(7) )
	ROMX_LOAD( "ffj_24.5e",  0x000004, 0x20000, CRC(a1ab607a) SHA1(56784c028b82d9e2affd9610f56fde57063e4c28) , ROM_SKIP(7) )
	ROMX_LOAD( "ffj_17.5c",  0x000005, 0x20000, CRC(2dc18cf4) SHA1(5e3bd895600cd30d561a75a2fcb6cc8bc84f4bd1) , ROM_SKIP(7) )
	ROMX_LOAD( "ffj_38.8h",  0x000006, 0x20000, CRC(6535a57f) SHA1(f4da9ec13cad7e3287e34dcceb0eb2d20107bad6) , ROM_SKIP(7) )
	ROMX_LOAD( "ffj_32.8f",  0x000007, 0x20000, CRC(c8bc4a57) SHA1(3eaf2b4e910fe1f79154020122d786d23a2e594a) , ROM_SKIP(7) )
	ROMX_LOAD( "ffj_10.5b",  0x100000, 0x20000, CRC(624a924a) SHA1(48fd0498f9ed54003bf6578fbcbc8b7e90a195eb) , ROM_SKIP(7) )
	ROMX_LOAD( "ffj_02.5a",  0x100001, 0x20000, CRC(5d91f694) SHA1(e0ea9ec82dec985d8bf5e7cebf5fe3d8ef7557b1) , ROM_SKIP(7) )
	ROMX_LOAD( "ffj_14.10b", 0x100002, 0x20000, CRC(0a2e9101) SHA1(6c8d550d2066cd53355ccf14ac1fd35914982094) , ROM_SKIP(7) )
	ROMX_LOAD( "ffj_06.10a", 0x100003, 0x20000, CRC(1c18f042) SHA1(f708296570fecad82a76dc59744873a2f5568ea1) , ROM_SKIP(7) )
	ROMX_LOAD( "ffj_25.7e",  0x100004, 0x20000, CRC(6e8181ea) SHA1(2c32bc0364650ee6ca0d24754a7a3401295ffcd5) , ROM_SKIP(7) )
	ROMX_LOAD( "ffj_18.7c",  0x100005, 0x20000, CRC(b19ede59) SHA1(7e79ad9f17b36e042d774bef3bbb44018332ca01) , ROM_SKIP(7) )
	ROMX_LOAD( "ffj_39.9h",  0x100006, 0x20000, CRC(9416b477) SHA1(f2310dfcfe960e8b822c07849b594d54dfc2b2ca) , ROM_SKIP(7) )
	ROMX_LOAD( "ffj_33.9f",  0x100007, 0x20000, CRC(7369fa07) SHA1(3b2750fe33729395217c96909b4b6c5f3d6e9943) , ROM_SKIP(7) )

	ROM_REGION( 0x18000, "audiocpu", 0 ) /* 64k for the audio CPU (+banks) */
	ROM_LOAD( "ff_23.bin",   0x00000, 0x08000, CRC(b8367eb5) SHA1(ec3db29fdd6200e9a8f4f8073a7e34aef731354f) )   // == ff_09.12b
	ROM_CONTINUE(            0x10000, 0x08000 )

	ROM_REGION( 0x40000, "oki", 0 ) /* Samples */
	ROM_LOAD( "ffj_30.bin",  0x00000, 0x20000, CRC(375c66e7) SHA1(36189e23209ce4ae5d9cbabd1574540d0591e7b3) )   // == ff_18.11c
	ROM_LOAD( "ffj_31.bin",  0x20000, 0x20000, CRC(1ef137f9) SHA1(974b5e72aa28b87ebfa7438efbdfeda769dedf5e) )   // == ff_19.12c

	ROM_REGION( 0x0200, "aboardplds", 0 )
	ROM_LOAD( "buf1",         0x0000, 0x0117, CRC(eb122de7) SHA1(b26b5bfe258e3e184f069719f9fd008d6b8f6b9b) )
	ROM_LOAD( "ioa1",         0x0000, 0x0117, CRC(59c7ee3b) SHA1(fbb887c5b4f5cb8df77cec710eaac2985bc482a6) )
	ROM_LOAD( "prg1",         0x0000, 0x0117, CRC(f1129744) SHA1(a5300f301c1a08a7da768f0773fa0fe3f683b237) )
	ROM_LOAD( "rom1",         0x0000, 0x0117, CRC(41dc73b9) SHA1(7d4c9f1693c821fbf84e32dd6ef62ddf14967845) )
	ROM_LOAD( "sou1",         0x0000, 0x0117, CRC(84f4b2fe) SHA1(dcc9e86cc36316fe42eace02d6df75d08bc8bb6d) )

	ROM_REGION( 0x0200, "bboardplds", 0 )
	ROM_LOAD( "s222b.1a",     0x0000, 0x0117, CRC(6d86b45e) SHA1(2b27646adaf1ca2f58e14754d6f7ef4fdca77fbe) )
	ROM_LOAD( "lwio.12c",     0x0000, 0x0117, CRC(ad52b90c) SHA1(f0fd6aeea515ee449320fe15684e6b3ab7f97bf4) )    // pal & position verification required
ROM_END

/* B-Board 89625B-1 */
ROM_START( ffightj1 )
	ROM_REGION( CODE_SIZE, "maincpu", 0 )      /* 68000 code */
	ROM_LOAD16_BYTE( "ffj_36.12f", 0x00000, 0x20000, CRC(e2a48af9) SHA1(11e06f95bdf575af396dded2b84d858f6c7388f1) ) // == ffu_36.11f
	ROM_LOAD16_BYTE( "ffj_42.12h", 0x00001, 0x20000, CRC(f4bb480e) SHA1(32114df1d2f4f98a4a2280a330c7b6af8ab4d862) ) // == ffu_42.11h
	ROM_LOAD16_BYTE( "ffj_37.13f", 0x40000, 0x20000, CRC(c371c667) SHA1(633977c91a8ff09b7fe83128eced7c4dee9aee1d) ) // == ffu_37.12f
	ROM_LOAD16_BYTE( "ffj_43.13h", 0x40001, 0x20000, CRC(6f81f194) SHA1(2cddf75a0a607cf57395583876cf81bcca005871) )
	ROM_LOAD16_BYTE( "ffj_34.10f", 0x80000, 0x20000, CRC(0c8dc3fc) SHA1(edcce3efd9cdd131ef0c96df15a68722d5c3498e) ) // == ff-32m.8h
	ROM_LOAD16_BYTE( "ffj_40.10h", 0x80001, 0x20000, CRC(8075bab9) SHA1(f9c7405133f6fc5557c90e60e8ccc459e4f6fd7d) ) // == ff-32m.8h
	ROM_LOAD16_BYTE( "ffj_35.11f", 0xc0000, 0x20000, CRC(4a934121) SHA1(3982c261582755a0eac340d6d7ed96e6c263c8b6) ) // == ff-32m.8h
	ROM_LOAD16_BYTE( "ffj_41.11h", 0xc0001, 0x20000, CRC(2af68154) SHA1(7d549cb38650b4b79c68ad6d0dfcefdd62be4e99) ) // == ff-32m.8h

	ROM_REGION( 0x200000, "gfx", 0 )
	ROMX_LOAD( "ffj_09.4b",  0x000000, 0x20000, CRC(5b116d0d) SHA1(a24e829fdfa043bd27b508d7cc0788ad80fd180e) , ROM_SKIP(7) )
	ROMX_LOAD( "ffj_01.4a",  0x000001, 0x20000, CRC(815b1797) SHA1(549e5eefc8f607fec1c954ba715ff21b8e44a5aa) , ROM_SKIP(7) )
	ROMX_LOAD( "ffj_13.9b",  0x000002, 0x20000, CRC(8721a7da) SHA1(39b2b324fd7810342503f23695e423f364a6294d) , ROM_SKIP(7) )
	ROMX_LOAD( "ffj_05.9a",  0x000003, 0x20000, CRC(d0fcd4b5) SHA1(97ebcbead0cca7e425143c500c433dbcf9cadcc2) , ROM_SKIP(7) )
	ROMX_LOAD( "ffj_24.5e",  0x000004, 0x20000, CRC(a1ab607a) SHA1(56784c028b82d9e2affd9610f56fde57063e4c28) , ROM_SKIP(7) )
	ROMX_LOAD( "ffj_17.5c",  0x000005, 0x20000, CRC(2dc18cf4) SHA1(5e3bd895600cd30d561a75a2fcb6cc8bc84f4bd1) , ROM_SKIP(7) )
	ROMX_LOAD( "ffj_38.8h",  0x000006, 0x20000, CRC(6535a57f) SHA1(f4da9ec13cad7e3287e34dcceb0eb2d20107bad6) , ROM_SKIP(7) )
	ROMX_LOAD( "ffj_32.8f",  0x000007, 0x20000, CRC(c8bc4a57) SHA1(3eaf2b4e910fe1f79154020122d786d23a2e594a) , ROM_SKIP(7) )
	ROMX_LOAD( "ffj_10.5b",  0x100000, 0x20000, CRC(624a924a) SHA1(48fd0498f9ed54003bf6578fbcbc8b7e90a195eb) , ROM_SKIP(7) )
	ROMX_LOAD( "ffj_02.5a",  0x100001, 0x20000, CRC(5d91f694) SHA1(e0ea9ec82dec985d8bf5e7cebf5fe3d8ef7557b1) , ROM_SKIP(7) )
	ROMX_LOAD( "ffj_14.10b", 0x100002, 0x20000, CRC(0a2e9101) SHA1(6c8d550d2066cd53355ccf14ac1fd35914982094) , ROM_SKIP(7) )
	ROMX_LOAD( "ffj_06.10a", 0x100003, 0x20000, CRC(1c18f042) SHA1(f708296570fecad82a76dc59744873a2f5568ea1) , ROM_SKIP(7) )
	ROMX_LOAD( "ffj_25.7e",  0x100004, 0x20000, CRC(6e8181ea) SHA1(2c32bc0364650ee6ca0d24754a7a3401295ffcd5) , ROM_SKIP(7) )
	ROMX_LOAD( "ffj_18.7c",  0x100005, 0x20000, CRC(b19ede59) SHA1(7e79ad9f17b36e042d774bef3bbb44018332ca01) , ROM_SKIP(7) )
	ROMX_LOAD( "ffj_39.9h",  0x100006, 0x20000, CRC(9416b477) SHA1(f2310dfcfe960e8b822c07849b594d54dfc2b2ca) , ROM_SKIP(7) )
	ROMX_LOAD( "ffj_33.9f",  0x100007, 0x20000, CRC(7369fa07) SHA1(3b2750fe33729395217c96909b4b6c5f3d6e9943) , ROM_SKIP(7) )

	ROM_REGION( 0x18000, "audiocpu", 0 ) /* 64k for the audio CPU (+banks) */
	ROM_LOAD( "ff_23.13b",   0x00000, 0x08000, CRC(b8367eb5) SHA1(ec3db29fdd6200e9a8f4f8073a7e34aef731354f) )   // == ff_09.12b
	ROM_CONTINUE(            0x10000, 0x08000 )

	ROM_REGION( 0x40000, "oki", 0 ) /* Samples */
	ROM_LOAD( "ffj_30.12c",  0x00000, 0x20000, CRC(375c66e7) SHA1(36189e23209ce4ae5d9cbabd1574540d0591e7b3) )   // == ff_18.11c
	ROM_LOAD( "ffj_31.13c",  0x20000, 0x20000, CRC(1ef137f9) SHA1(974b5e72aa28b87ebfa7438efbdfeda769dedf5e) )   // == ff_19.12c

	ROM_REGION( 0x0200, "aboardplds", 0 )
	ROM_LOAD( "buf1",         0x0000, 0x0117, CRC(eb122de7) SHA1(b26b5bfe258e3e184f069719f9fd008d6b8f6b9b) )
	ROM_LOAD( "ioa1",         0x0000, 0x0117, CRC(59c7ee3b) SHA1(fbb887c5b4f5cb8df77cec710eaac2985bc482a6) )
	ROM_LOAD( "prg1",         0x0000, 0x0117, CRC(f1129744) SHA1(a5300f301c1a08a7da768f0773fa0fe3f683b237) )
	ROM_LOAD( "rom1",         0x0000, 0x0117, CRC(41dc73b9) SHA1(7d4c9f1693c821fbf84e32dd6ef62ddf14967845) )
	ROM_LOAD( "sou1",         0x0000, 0x0117, CRC(84f4b2fe) SHA1(dcc9e86cc36316fe42eace02d6df75d08bc8bb6d) )

	ROM_REGION( 0x0200, "bboardplds", 0 )
	ROM_LOAD( "s222b.1a",     0x0000, 0x0117, CRC(6d86b45e) SHA1(2b27646adaf1ca2f58e14754d6f7ef4fdca77fbe) )
	ROM_LOAD( "lwio.12e",     0x0000, 0x0117, CRC(ad52b90c) SHA1(f0fd6aeea515ee449320fe15684e6b3ab7f97bf4) )
ROM_END

/* B-Board 88622B-3 */
ROM_START( ffightj2 )
	ROM_REGION( CODE_SIZE, "maincpu", 0 )      /* 68000 code */
	ROM_LOAD16_BYTE( "ffj_36a.12f", 0x00000, 0x20000, CRC(088ed1c9) SHA1(7b96cd45f4d3d2c0fe94904882652814b7790869) )
	ROM_LOAD16_BYTE( "ffj_42a.12h", 0x00001, 0x20000, CRC(c4c491e6) SHA1(d0e34d7b94f67c33615710ea721da8fefe832e3a) )
	ROM_LOAD16_BYTE( "ffj_37a.13f", 0x40000, 0x20000, CRC(708557ff) SHA1(89e56bfd9486623a18fdbf984a72bb52054ca0e6) )
	ROM_LOAD16_BYTE( "ffj_43a.13h", 0x40001, 0x20000, CRC(c004004a) SHA1(10ccf27972591f65645a8dd2bb65989176ac07d5) )
	ROM_LOAD16_BYTE( "ff_34.10f",   0x80000, 0x20000, CRC(0c8dc3fc) SHA1(edcce3efd9cdd131ef0c96df15a68722d5c3498e) )    // == ff-32m.8h
	ROM_LOAD16_BYTE( "ff_40.10h",   0x80001, 0x20000, CRC(8075bab9) SHA1(f9c7405133f6fc5557c90e60e8ccc459e4f6fd7d) )    // == ff-32m.8h
	ROM_LOAD16_BYTE( "ff_35.11f",   0xc0000, 0x20000, CRC(4a934121) SHA1(3982c261582755a0eac340d6d7ed96e6c263c8b6) )    // == ff-32m.8h
	ROM_LOAD16_BYTE( "ff_41.11h",   0xc0001, 0x20000, CRC(2af68154) SHA1(7d549cb38650b4b79c68ad6d0dfcefdd62be4e99) )    // == ff-32m.8h

	ROM_REGION( 0x200000, "gfx", 0 )
	ROMX_LOAD( "ff_09.4b",  0x000000, 0x20000, CRC(5b116d0d) SHA1(a24e829fdfa043bd27b508d7cc0788ad80fd180e) , ROM_SKIP(7) )
	ROMX_LOAD( "ff_01.4a",  0x000001, 0x20000, CRC(815b1797) SHA1(549e5eefc8f607fec1c954ba715ff21b8e44a5aa) , ROM_SKIP(7) )
	ROMX_LOAD( "ff_13.9b",  0x000002, 0x20000, CRC(8721a7da) SHA1(39b2b324fd7810342503f23695e423f364a6294d) , ROM_SKIP(7) )
	ROMX_LOAD( "ff_05.9a",  0x000003, 0x20000, CRC(d0fcd4b5) SHA1(97ebcbead0cca7e425143c500c433dbcf9cadcc2) , ROM_SKIP(7) )
	ROMX_LOAD( "ff_24.5e",  0x000004, 0x20000, CRC(a1ab607a) SHA1(56784c028b82d9e2affd9610f56fde57063e4c28) , ROM_SKIP(7) )
	ROMX_LOAD( "ff_17.5c",  0x000005, 0x20000, CRC(2dc18cf4) SHA1(5e3bd895600cd30d561a75a2fcb6cc8bc84f4bd1) , ROM_SKIP(7) )
	ROMX_LOAD( "ff_38.8h",  0x000006, 0x20000, CRC(6535a57f) SHA1(f4da9ec13cad7e3287e34dcceb0eb2d20107bad6) , ROM_SKIP(7) )
	ROMX_LOAD( "ff_32.8f",  0x000007, 0x20000, CRC(c8bc4a57) SHA1(3eaf2b4e910fe1f79154020122d786d23a2e594a) , ROM_SKIP(7) )
	ROMX_LOAD( "ff_10.5b",  0x100000, 0x20000, CRC(624a924a) SHA1(48fd0498f9ed54003bf6578fbcbc8b7e90a195eb) , ROM_SKIP(7) )
	ROMX_LOAD( "ff_02.5a",  0x100001, 0x20000, CRC(5d91f694) SHA1(e0ea9ec82dec985d8bf5e7cebf5fe3d8ef7557b1) , ROM_SKIP(7) )
	ROMX_LOAD( "ff_14.10b", 0x100002, 0x20000, CRC(0a2e9101) SHA1(6c8d550d2066cd53355ccf14ac1fd35914982094) , ROM_SKIP(7) )
	ROMX_LOAD( "ff_06.10a", 0x100003, 0x20000, CRC(1c18f042) SHA1(f708296570fecad82a76dc59744873a2f5568ea1) , ROM_SKIP(7) )
	ROMX_LOAD( "ff_25.7e",  0x100004, 0x20000, CRC(6e8181ea) SHA1(2c32bc0364650ee6ca0d24754a7a3401295ffcd5) , ROM_SKIP(7) )
	ROMX_LOAD( "ff_18.7c",  0x100005, 0x20000, CRC(b19ede59) SHA1(7e79ad9f17b36e042d774bef3bbb44018332ca01) , ROM_SKIP(7) )
	ROMX_LOAD( "ff_39.9h",  0x100006, 0x20000, CRC(9416b477) SHA1(f2310dfcfe960e8b822c07849b594d54dfc2b2ca) , ROM_SKIP(7) )
	ROMX_LOAD( "ff_33.9f",  0x100007, 0x20000, CRC(7369fa07) SHA1(3b2750fe33729395217c96909b4b6c5f3d6e9943) , ROM_SKIP(7) )

	ROM_REGION( 0x18000, "audiocpu", 0 ) /* 64k for the audio CPU (+banks) */
	ROM_LOAD( "ff_23.13c",  0x00000, 0x08000, CRC(b8367eb5) SHA1(ec3db29fdd6200e9a8f4f8073a7e34aef731354f) )    // == ff_09.12b
	ROM_CONTINUE(           0x10000, 0x08000 )

	ROM_REGION( 0x40000, "oki", 0 ) /* Samples */
	ROM_LOAD( "ff_30.12e",  0x00000, 0x20000, CRC(375c66e7) SHA1(36189e23209ce4ae5d9cbabd1574540d0591e7b3) )    // == ff_18.11c
	ROM_LOAD( "ff_31.13e",  0x20000, 0x20000, CRC(1ef137f9) SHA1(974b5e72aa28b87ebfa7438efbdfeda769dedf5e) )    // == ff_19.12c

	ROM_REGION( 0x0200, "aboardplds", 0 )
	ROM_LOAD( "buf1",         0x0000, 0x0117, CRC(eb122de7) SHA1(b26b5bfe258e3e184f069719f9fd008d6b8f6b9b) )
	ROM_LOAD( "ioa1",         0x0000, 0x0117, CRC(59c7ee3b) SHA1(fbb887c5b4f5cb8df77cec710eaac2985bc482a6) )
	ROM_LOAD( "prg1",         0x0000, 0x0117, CRC(f1129744) SHA1(a5300f301c1a08a7da768f0773fa0fe3f683b237) )
	ROM_LOAD( "rom1",         0x0000, 0x0117, CRC(41dc73b9) SHA1(7d4c9f1693c821fbf84e32dd6ef62ddf14967845) )
	ROM_LOAD( "sou1",         0x0000, 0x0117, CRC(84f4b2fe) SHA1(dcc9e86cc36316fe42eace02d6df75d08bc8bb6d) )

	ROM_REGION( 0x0200, "bboardplds", 0 )
	ROM_LOAD( "s222b.1a",     0x0000, 0x0117, CRC(6d86b45e) SHA1(2b27646adaf1ca2f58e14754d6f7ef4fdca77fbe) )
	ROM_LOAD( "lwio.12c",     0x0000, 0x0117, CRC(ad52b90c) SHA1(f0fd6aeea515ee449320fe15684e6b3ab7f97bf4) )
ROM_END

/* B-Board 89625B-1 */
ROM_START( ffightj3 )
	ROM_REGION( CODE_SIZE, "maincpu", 0 )      /* 68000 code */
	ROM_LOAD16_BYTE( "ff_36.12f",              0x00000, 0x20000, CRC(ed988977) SHA1(c718e989206bd2b68832c8fcb5667397d500ebac) ) // == ffu_30.11f
	ROM_LOAD16_BYTE( "ffj_42.12h",             0x00001, 0x20000, CRC(07bf1c21) SHA1(f21a939fd92607c7f54816dedbcb3c5818cf4183) ) // sldh - == ffu_35.11h
	ROM_LOAD16_BYTE( "ff_37.13f",              0x40000, 0x20000, CRC(dba5a476) SHA1(2f0176dd050f9630b914f1c1ca5d96215bcf567f) ) // == ffu_31.12f
	ROM_LOAD16_BYTE( "ffj_43.13h",             0x40001, 0x20000, CRC(fbeca028) SHA1(85eeed6a25b401d73d12896ca1e2bf7402c921ee) ) // sldh
	ROM_LOAD16_BYTE( "ff_34.10f",              0x80000, 0x20000, CRC(0c8dc3fc) SHA1(edcce3efd9cdd131ef0c96df15a68722d5c3498e) ) // == ff-32m.8h
	ROM_LOAD16_BYTE( "ffj_40.10h",             0x80001, 0x20000, CRC(8075bab9) SHA1(f9c7405133f6fc5557c90e60e8ccc459e4f6fd7d) ) // == ff-32m.8h
	ROM_LOAD16_BYTE( "ff_35.11f",              0xc0000, 0x20000, CRC(4a934121) SHA1(3982c261582755a0eac340d6d7ed96e6c263c8b6) ) // == ff-32m.8h
	ROM_LOAD16_BYTE( "ffj_41.11h",             0xc0001, 0x20000, CRC(2af68154) SHA1(7d549cb38650b4b79c68ad6d0dfcefdd62be4e99) ) // == ff-32m.8h

	ROM_REGION( 0x200000, "gfx", 0 )
	ROMX_LOAD( "ff_09.4b",  0x000000, 0x20000, CRC(5b116d0d) SHA1(a24e829fdfa043bd27b508d7cc0788ad80fd180e) , ROM_SKIP(7) )
	ROMX_LOAD( "ff_01.4a",  0x000001, 0x20000, CRC(815b1797) SHA1(549e5eefc8f607fec1c954ba715ff21b8e44a5aa) , ROM_SKIP(7) )
	ROMX_LOAD( "ff_13.9b",  0x000002, 0x20000, CRC(8721a7da) SHA1(39b2b324fd7810342503f23695e423f364a6294d) , ROM_SKIP(7) )
	ROMX_LOAD( "ff_05.9a",  0x000003, 0x20000, CRC(d0fcd4b5) SHA1(97ebcbead0cca7e425143c500c433dbcf9cadcc2) , ROM_SKIP(7) )
	ROMX_LOAD( "ff_24.5e",  0x000004, 0x20000, CRC(a1ab607a) SHA1(56784c028b82d9e2affd9610f56fde57063e4c28) , ROM_SKIP(7) )
	ROMX_LOAD( "ff_17.5c",  0x000005, 0x20000, CRC(2dc18cf4) SHA1(5e3bd895600cd30d561a75a2fcb6cc8bc84f4bd1) , ROM_SKIP(7) )
	ROMX_LOAD( "ffj_38.8h", 0x000006, 0x20000, CRC(6535a57f) SHA1(f4da9ec13cad7e3287e34dcceb0eb2d20107bad6) , ROM_SKIP(7) )
	ROMX_LOAD( "ff_32.8f",  0x000007, 0x20000, CRC(c8bc4a57) SHA1(3eaf2b4e910fe1f79154020122d786d23a2e594a) , ROM_SKIP(7) )
	ROMX_LOAD( "ff_10.5b",  0x100000, 0x20000, CRC(624a924a) SHA1(48fd0498f9ed54003bf6578fbcbc8b7e90a195eb) , ROM_SKIP(7) )
	ROMX_LOAD( "ff_02.5a",  0x100001, 0x20000, CRC(5d91f694) SHA1(e0ea9ec82dec985d8bf5e7cebf5fe3d8ef7557b1) , ROM_SKIP(7) )
	ROMX_LOAD( "ff_14.10b", 0x100002, 0x20000, CRC(0a2e9101) SHA1(6c8d550d2066cd53355ccf14ac1fd35914982094) , ROM_SKIP(7) )
	ROMX_LOAD( "ff_06.10a", 0x100003, 0x20000, CRC(1c18f042) SHA1(f708296570fecad82a76dc59744873a2f5568ea1) , ROM_SKIP(7) )
	ROMX_LOAD( "ff_25.7e",  0x100004, 0x20000, CRC(6e8181ea) SHA1(2c32bc0364650ee6ca0d24754a7a3401295ffcd5) , ROM_SKIP(7) )
	ROMX_LOAD( "ff_18.7c",  0x100005, 0x20000, CRC(b19ede59) SHA1(7e79ad9f17b36e042d774bef3bbb44018332ca01) , ROM_SKIP(7) )
	ROMX_LOAD( "ffj_39.9h", 0x100006, 0x20000, CRC(9416b477) SHA1(f2310dfcfe960e8b822c07849b594d54dfc2b2ca) , ROM_SKIP(7) )
	ROMX_LOAD( "ff_33.9f",  0x100007, 0x20000, CRC(7369fa07) SHA1(3b2750fe33729395217c96909b4b6c5f3d6e9943) , ROM_SKIP(7) )

	ROM_REGION( 0x18000, "audiocpu", 0 ) /* 64k for the audio CPU (+banks) */
	ROM_LOAD( "ff_23.13b",  0x00000, 0x08000, CRC(b8367eb5) SHA1(ec3db29fdd6200e9a8f4f8073a7e34aef731354f) )   // == ff_09.12b
	ROM_CONTINUE(           0x10000, 0x08000 )

	ROM_REGION( 0x40000, "oki", 0 ) /* Samples */
	ROM_LOAD( "ff_30.12c",  0x00000, 0x20000, CRC(375c66e7) SHA1(36189e23209ce4ae5d9cbabd1574540d0591e7b3) )   // == ff_18.11c
	ROM_LOAD( "ff_31.13c",  0x20000, 0x20000, CRC(1ef137f9) SHA1(974b5e72aa28b87ebfa7438efbdfeda769dedf5e) )   // == ff_19.12c

	ROM_REGION( 0x0200, "aboardplds", 0 )
	ROM_LOAD( "buf1",         0x0000, 0x0117, CRC(eb122de7) SHA1(b26b5bfe258e3e184f069719f9fd008d6b8f6b9b) )
	ROM_LOAD( "ioa1",         0x0000, 0x0117, CRC(59c7ee3b) SHA1(fbb887c5b4f5cb8df77cec710eaac2985bc482a6) )
	ROM_LOAD( "prg1",         0x0000, 0x0117, CRC(f1129744) SHA1(a5300f301c1a08a7da768f0773fa0fe3f683b237) )
	ROM_LOAD( "rom1",         0x0000, 0x0117, CRC(41dc73b9) SHA1(7d4c9f1693c821fbf84e32dd6ef62ddf14967845) )
	ROM_LOAD( "sou1",         0x0000, 0x0117, CRC(84f4b2fe) SHA1(dcc9e86cc36316fe42eace02d6df75d08bc8bb6d) )

	ROM_REGION( 0x0200, "bboardplds", 0 )
	ROM_LOAD( "s222b.1a",     0x0000, 0x0117, CRC(6d86b45e) SHA1(2b27646adaf1ca2f58e14754d6f7ef4fdca77fbe) )
	ROM_LOAD( "lwio.12e",     0x0000, 0x0117, CRC(ad52b90c) SHA1(f0fd6aeea515ee449320fe15684e6b3ab7f97bf4) )    // pal verification required
ROM_END

/* B-Board 91634B-2 */
/* This hack uses an official and original Final Fight JAP CPS-B-21 Board. The EPROMS have been modified and phoenixed.
   The Board uses a GAL instead of the classic PAL located in position "1A". There is no ROM check at the start and no logo
   Capcom (instead a chinese logo... maybe the bootleggers logo). Even if it's a JAP Board, the game intro is in english. */
ROM_START( ffightjh )
	ROM_REGION( CODE_SIZE, "maincpu", 0 )      /* 68000 code */
	ROM_LOAD16_WORD_SWAP( "ff_23.8f", 0x00000, 0x80000, CRC(ae3dda7f) SHA1(5f08ce1e6b6b0d45994d3354d59ef79f489c7ad7) )
	ROM_LOAD16_WORD_SWAP( "ff_22.7f", 0x80000, 0x80000, CRC(b2d5a3aa) SHA1(b60f7480d3d3ceebad4c21025394bacc154d7042) )

	ROM_REGION( 0x200000, "gfx", 0 )
	ROMX_LOAD( "ff_1.3a",  0x000000, 0x80000, CRC(969d18e2) SHA1(7a30b7434e34ec98838a9ab8d953b2ef467d562e), ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "ff_2.4a",  0x000002, 0x80000, CRC(02b59f99) SHA1(c1cb2bbf95c2006baad58b0a5a7278f1a8999901), ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "ff_3.5a",  0x000004, 0x80000, CRC(01d507ae) SHA1(d7813daa5bd74bebdbbea578acb20ad3c8816997), ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "ff_4.6a",  0x000006, 0x80000, CRC(f7c4ceb0) SHA1(3e9cf6379f06c5a736b4a0aadb7fbda914b91190), ROM_GROUPWORD | ROM_SKIP(6) )

	ROM_REGION( 0x18000, "audiocpu", 0 ) /* 64k for the audio CPU (+banks) */
	ROM_LOAD( "ff_9.12a",  0x00000, 0x08000, CRC(b8367eb5) SHA1(ec3db29fdd6200e9a8f4f8073a7e34aef731354f) ) // == ff_09.12b
	ROM_CONTINUE(          0x10000, 0x08000 )

	ROM_REGION( 0x40000, "oki", 0 ) /* Samples */
	ROM_LOAD( "ff_18.11c", 0x00000, 0x20000, CRC(375c66e7) SHA1(36189e23209ce4ae5d9cbabd1574540d0591e7b3) )
	ROM_LOAD( "ff_19.12c", 0x20000, 0x20000, CRC(1ef137f9) SHA1(974b5e72aa28b87ebfa7438efbdfeda769dedf5e) )

	ROM_REGION( 0x0200, "aboardplds", 0 )
	ROM_LOAD( "buf1",         0x0000, 0x0117, CRC(eb122de7) SHA1(b26b5bfe258e3e184f069719f9fd008d6b8f6b9b) )
	ROM_LOAD( "ioa1",         0x0000, 0x0117, CRC(59c7ee3b) SHA1(fbb887c5b4f5cb8df77cec710eaac2985bc482a6) )
	ROM_LOAD( "prg1",         0x0000, 0x0117, CRC(f1129744) SHA1(a5300f301c1a08a7da768f0773fa0fe3f683b237) )
	ROM_LOAD( "rom1",         0x0000, 0x0117, CRC(41dc73b9) SHA1(7d4c9f1693c821fbf84e32dd6ef62ddf14967845) )
	ROM_LOAD( "sou1",         0x0000, 0x0117, CRC(84f4b2fe) SHA1(dcc9e86cc36316fe42eace02d6df75d08bc8bb6d) )

	ROM_REGION( 0x0200, "bboardplds", 0 )
	ROM_LOAD( "gal16v8a-15lp.1a",     0x0000, 0x0117, NO_DUMP )
	ROM_LOAD( "iob1.12d",             0x0000, 0x0117, CRC(3abc0700) SHA1(973043aa46ec6d5d1db20dc9d5937005a0f9f6ae) )
	ROM_LOAD( "bprg1.11d",            0x0000, 0x0117, CRC(31793da7) SHA1(400fa7ac517421c978c1ee7773c30b9ed0c5d3f3) )

	ROM_REGION( 0x0200, "cboardplds", 0 )
	ROM_LOAD( "ioc1.ic1",     0x0000, 0x0117, CRC(0d182081) SHA1(475b3d417785da4bc512cce2b274bb00d4cc6792) )
ROM_END

/* B-Board 89624B-3 */
ROM_START( 1941 )
	ROM_REGION( CODE_SIZE, "maincpu", 0 )      /* 68000 code */
	ROM_LOAD16_BYTE( "41em_30.11f",    0x00000, 0x20000, CRC(4249ec61) SHA1(5323cfa6938e6d95db8469f09b2fb5b5c5068bfc) ) /* label is 41EM_30, pcb verified */
	ROM_LOAD16_BYTE( "41em_35.11h",    0x00001, 0x20000, CRC(ddbee5eb) SHA1(0fef53398f4e2cd6ccc7bc122dd893e0a4e2e052) ) /* label is 41EM_35, pcb verified */
	ROM_LOAD16_BYTE( "41em_31.12f",    0x40000, 0x20000, CRC(584e88e5) SHA1(af254408d939cc439b5653e60afbccdf59777085) ) /* label is 41EM_31, pcb verified */
	ROM_LOAD16_BYTE( "41em_36.12h",    0x40001, 0x20000, CRC(3cfc31d0) SHA1(7c0614e4e25502ff2d18aa4a97c91bf05ead875f) ) /* label is 41EM_36, pcb verified */
	ROM_LOAD16_WORD_SWAP( "41-32m.8h", 0x80000, 0x80000, CRC(4e9648ca) SHA1(d8e67e6e3a6dc79053e4f56cfd83431385ea7611) )

	ROM_REGION( 0x200000, "gfx", 0 )
	ROMX_LOAD( "41-5m.7a", 0x000000, 0x80000, CRC(01d1cb11) SHA1(621e5377d1aaa9f7270d85bea1bdeef6721cdd05) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "41-7m.9a", 0x000002, 0x80000, CRC(aeaa3509) SHA1(6124ef06d9dfdd879181856bd49853f1800c3b87) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "41-1m.3a", 0x000004, 0x80000, CRC(ff77985a) SHA1(7e08df3a829bf9617470a46c79b713d4d9ebacae) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "41-3m.5a", 0x000006, 0x80000, CRC(983be58f) SHA1(83a4decdd775f859240771269b8af3a5981b244c) , ROM_GROUPWORD | ROM_SKIP(6) )

	ROM_REGION( 0x18000, "audiocpu", 0 ) /* 64k for the audio CPU (+banks) */
	ROM_LOAD( "41_9.12b",  0x00000, 0x08000, CRC(0f9d8527) SHA1(3a00dd5772f38081fde11d8d61ba467379e2a636) )
	ROM_CONTINUE(          0x10000, 0x08000 )

	ROM_REGION( 0x40000, "oki", 0 ) /* Samples */
	ROM_LOAD( "41_18.11c", 0x00000, 0x20000, CRC(d1f15aeb) SHA1(88089383f2d54fc97026a67f067d448eee5bd0c2) )
	ROM_LOAD( "41_19.12c", 0x20000, 0x20000, CRC(15aec3a6) SHA1(8153c03aba005bab62bf0e8b3d15ec1c346326fd) )

	ROM_REGION( 0x0200, "aboardplds", 0 )
	ROM_LOAD( "buf1",         0x0000, 0x0117, CRC(eb122de7) SHA1(b26b5bfe258e3e184f069719f9fd008d6b8f6b9b) )
	ROM_LOAD( "ioa1",         0x0000, 0x0117, CRC(59c7ee3b) SHA1(fbb887c5b4f5cb8df77cec710eaac2985bc482a6) )
	ROM_LOAD( "prg1",         0x0000, 0x0117, CRC(f1129744) SHA1(a5300f301c1a08a7da768f0773fa0fe3f683b237) )
	ROM_LOAD( "rom1",         0x0000, 0x0117, CRC(41dc73b9) SHA1(7d4c9f1693c821fbf84e32dd6ef62ddf14967845) )
	ROM_LOAD( "sou1",         0x0000, 0x0117, CRC(84f4b2fe) SHA1(dcc9e86cc36316fe42eace02d6df75d08bc8bb6d) )

	ROM_REGION( 0x0200, "bboardplds", 0 )
	ROM_LOAD( "yi24b.1a",     0x0000, 0x0117, CRC(3004dcdf) SHA1(a73d86d1545af1d99a03d88af83449ec82b03c5c) )
	ROM_LOAD( "iob1.11e",     0x0000, 0x0117, CRC(3abc0700) SHA1(973043aa46ec6d5d1db20dc9d5937005a0f9f6ae) )
ROM_END

/* B-Board 89624B-3 */
ROM_START( 1941r1 )
	ROM_REGION( CODE_SIZE, "maincpu", 0 )      /* 68000 code */
	ROM_LOAD16_BYTE( "41e_30.11f",     0x00000, 0x20000, CRC(9deb1e75) SHA1(68d9f91bef6a5c9e1bcbf286629aed6b37b4acb9) )
	ROM_LOAD16_BYTE( "41e_35.11h",     0x00001, 0x20000, CRC(d63942b3) SHA1(b4bc7d06dcefbc075d316f2d31abbd4c7a99dbae) )
	ROM_LOAD16_BYTE( "41e_31.12f",     0x40000, 0x20000, CRC(df201112) SHA1(d84f63bffeb9255cbabc02f23d7511f9b3c6a96c) )
	ROM_LOAD16_BYTE( "41e_36.12h",     0x40001, 0x20000, CRC(816a818f) SHA1(3e491a30352b71ddd775142f3a80cdde480b669f) )
	ROM_LOAD16_WORD_SWAP( "41-32m.8h", 0x80000, 0x80000, CRC(4e9648ca) SHA1(d8e67e6e3a6dc79053e4f56cfd83431385ea7611) )

	ROM_REGION( 0x200000, "gfx", 0 )
	ROMX_LOAD( "41-5m.7a", 0x000000, 0x80000, CRC(01d1cb11) SHA1(621e5377d1aaa9f7270d85bea1bdeef6721cdd05) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "41-7m.9a", 0x000002, 0x80000, CRC(aeaa3509) SHA1(6124ef06d9dfdd879181856bd49853f1800c3b87) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "41-1m.3a", 0x000004, 0x80000, CRC(ff77985a) SHA1(7e08df3a829bf9617470a46c79b713d4d9ebacae) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "41-3m.5a", 0x000006, 0x80000, CRC(983be58f) SHA1(83a4decdd775f859240771269b8af3a5981b244c) , ROM_GROUPWORD | ROM_SKIP(6) )

	ROM_REGION( 0x18000, "audiocpu", 0 ) /* 64k for the audio CPU (+banks) */
	ROM_LOAD( "41_9.12b",  0x00000, 0x08000, CRC(0f9d8527) SHA1(3a00dd5772f38081fde11d8d61ba467379e2a636) )
	ROM_CONTINUE(          0x10000, 0x08000 )

	ROM_REGION( 0x40000, "oki", 0 ) /* Samples */
	ROM_LOAD( "41_18.11c", 0x00000, 0x20000, CRC(d1f15aeb) SHA1(88089383f2d54fc97026a67f067d448eee5bd0c2) )
	ROM_LOAD( "41_19.12c", 0x20000, 0x20000, CRC(15aec3a6) SHA1(8153c03aba005bab62bf0e8b3d15ec1c346326fd) )

	ROM_REGION( 0x0200, "aboardplds", 0 )
	ROM_LOAD( "buf1",         0x0000, 0x0117, CRC(eb122de7) SHA1(b26b5bfe258e3e184f069719f9fd008d6b8f6b9b) )
	ROM_LOAD( "ioa1",         0x0000, 0x0117, CRC(59c7ee3b) SHA1(fbb887c5b4f5cb8df77cec710eaac2985bc482a6) )
	ROM_LOAD( "prg1",         0x0000, 0x0117, CRC(f1129744) SHA1(a5300f301c1a08a7da768f0773fa0fe3f683b237) )
	ROM_LOAD( "rom1",         0x0000, 0x0117, CRC(41dc73b9) SHA1(7d4c9f1693c821fbf84e32dd6ef62ddf14967845) )
	ROM_LOAD( "sou1",         0x0000, 0x0117, CRC(84f4b2fe) SHA1(dcc9e86cc36316fe42eace02d6df75d08bc8bb6d) )

	ROM_REGION( 0x0200, "bboardplds", 0 )
	ROM_LOAD( "yi24b.1a",     0x0000, 0x0117, CRC(3004dcdf) SHA1(a73d86d1545af1d99a03d88af83449ec82b03c5c) )
	ROM_LOAD( "iob1.11e",     0x0000, 0x0117, CRC(3abc0700) SHA1(973043aa46ec6d5d1db20dc9d5937005a0f9f6ae) )
ROM_END

/* B-Board 89624B-3 */
ROM_START( 1941u )
	ROM_REGION( CODE_SIZE, "maincpu", 0 )      /* 68000 code */
	ROM_LOAD16_BYTE( "41u_30.11f",     0x00000, 0x20000, CRC(be5439d0) SHA1(a62e0aa4f13f504c0eded99d731f7c820c51cb3d) )
	ROM_LOAD16_BYTE( "41u_35.11h",     0x00001, 0x20000, CRC(6ac96595) SHA1(efff8185fd1aee29d9b5164a9aac61f77f769ae4) )
	ROM_LOAD16_BYTE( "41u_31.12f",     0x40000, 0x20000, CRC(9811d6eb) SHA1(6dda86a3a726a2b5459f0b90f58343966e9783cc) )
	ROM_LOAD16_BYTE( "41u_36.12h",     0x40001, 0x20000, CRC(a87e6137) SHA1(d4b63a0c14a35ebd61cddd01cdc6fc955d521ae2) )
	ROM_LOAD16_WORD_SWAP( "41-32m.8h", 0x80000, 0x80000, CRC(4e9648ca) SHA1(d8e67e6e3a6dc79053e4f56cfd83431385ea7611) )

	ROM_REGION( 0x200000, "gfx", 0 )
	ROMX_LOAD( "41-5m.7a",  0x000000, 0x80000, CRC(01d1cb11) SHA1(621e5377d1aaa9f7270d85bea1bdeef6721cdd05) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "41-7m.9a",  0x000002, 0x80000, CRC(aeaa3509) SHA1(6124ef06d9dfdd879181856bd49853f1800c3b87) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "41-1m.3a",  0x000004, 0x80000, CRC(ff77985a) SHA1(7e08df3a829bf9617470a46c79b713d4d9ebacae) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "41-3m.5a",  0x000006, 0x80000, CRC(983be58f) SHA1(83a4decdd775f859240771269b8af3a5981b244c) , ROM_GROUPWORD | ROM_SKIP(6) )

	ROM_REGION( 0x18000, "audiocpu", 0 ) /* 64k for the audio CPU (+banks) */
	ROM_LOAD( "41_9.12b",   0x00000, 0x08000, CRC(0f9d8527) SHA1(3a00dd5772f38081fde11d8d61ba467379e2a636) )
	ROM_CONTINUE(           0x10000, 0x08000 )

	ROM_REGION( 0x40000, "oki", 0 ) /* Samples */
	ROM_LOAD( "41e_18.11c", 0x00000, 0x20000, CRC(d1f15aeb) SHA1(88089383f2d54fc97026a67f067d448eee5bd0c2) )    // == 41_18.11c /* label is 41E_18, pcb verified */
	ROM_LOAD( "41_19.12c",  0x20000, 0x20000, CRC(15aec3a6) SHA1(8153c03aba005bab62bf0e8b3d15ec1c346326fd) )

	ROM_REGION( 0x0200, "aboardplds", 0 )
	ROM_LOAD( "buf1",         0x0000, 0x0117, CRC(eb122de7) SHA1(b26b5bfe258e3e184f069719f9fd008d6b8f6b9b) )
	ROM_LOAD( "ioa1",         0x0000, 0x0117, CRC(59c7ee3b) SHA1(fbb887c5b4f5cb8df77cec710eaac2985bc482a6) )
	ROM_LOAD( "prg1",         0x0000, 0x0117, CRC(f1129744) SHA1(a5300f301c1a08a7da768f0773fa0fe3f683b237) )
	ROM_LOAD( "rom1",         0x0000, 0x0117, CRC(41dc73b9) SHA1(7d4c9f1693c821fbf84e32dd6ef62ddf14967845) )
	ROM_LOAD( "sou1",         0x0000, 0x0117, CRC(84f4b2fe) SHA1(dcc9e86cc36316fe42eace02d6df75d08bc8bb6d) )

	ROM_REGION( 0x0200, "bboardplds", 0 )
	ROM_LOAD( "yi24b.1a",     0x0000, 0x0117, CRC(3004dcdf) SHA1(a73d86d1545af1d99a03d88af83449ec82b03c5c) )
	ROM_LOAD( "iob1.11e",     0x0000, 0x0117, CRC(3abc0700) SHA1(973043aa46ec6d5d1db20dc9d5937005a0f9f6ae) )
ROM_END

/* B-Board 89625B-1 */
ROM_START( 1941j )
	ROM_REGION( CODE_SIZE, "maincpu", 0 )      /* 68000 code */
	ROM_LOAD16_BYTE( "41_36.12f", 0x00000, 0x20000, CRC(7fbd42ab) SHA1(4e52a599e3099bf3cccabb89152c69f216fde79e) )
	ROM_LOAD16_BYTE( "41_42.12h", 0x00001, 0x20000, CRC(c7781f89) SHA1(7e99c433de0c903791ae153a3cc8632042b0a90d) )
	ROM_LOAD16_BYTE( "41_37.13f", 0x40000, 0x20000, CRC(c6464b0b) SHA1(abef422d891d32334a858d49599f1ef7cf0db45d) )
	ROM_LOAD16_BYTE( "41_43.13h", 0x40001, 0x20000, CRC(440fc0b5) SHA1(e725535533c25a2c80a45a2200bbfd0dcda5ed97) )
	ROM_LOAD16_BYTE( "41_34.10f", 0x80000, 0x20000, CRC(b5f341ec) SHA1(95c740332e9e05250979a098e951600ca89cfde4) )  // == 41-32m.8h
	ROM_LOAD16_BYTE( "41_40.10h", 0x80001, 0x20000, CRC(3979837d) SHA1(e35bca5cd6f7559447c553c7866a3934080ca018) )  // == 41-32m.8h
	ROM_LOAD16_BYTE( "41_35.11f", 0xc0000, 0x20000, CRC(95cc979a) SHA1(96a22d2accdcd1ed3829ab0e1f17c1615f8bb2a9) )  // == 41-32m.8h
	ROM_LOAD16_BYTE( "41_41.11h", 0xc0001, 0x20000, CRC(57496819) SHA1(fe705a25a2c6523c3bc0105f4b183931c5ffcc81) )  // == 41-32m.8h

	ROM_REGION( 0x200000, "gfx", 0 )
	ROMX_LOAD( "41_09.4b",  0x000000, 0x20000, CRC(be1b6bc2) SHA1(0b11efb7adf6bb9c5fdef702171edd38c149e338) , ROM_SKIP(7) ) // == 41-5m.7a
	ROMX_LOAD( "41_01.4a",  0x000001, 0x20000, CRC(d8946fc1) SHA1(e18d2bd5517a9a74973f5a505e59e400db3843e2) , ROM_SKIP(7) ) // == 41-5m.7a
	ROMX_LOAD( "41_13.9b",  0x000002, 0x20000, CRC(2e06d0ec) SHA1(b7c886dce6bc5eefa34e39faec658dd9df34f82c) , ROM_SKIP(7) ) // == 41-7m.9a
	ROMX_LOAD( "41_05.9a",  0x000003, 0x20000, CRC(d8ba28e0) SHA1(dd1908166b32cf3bb3222f6d3febc21371463395) , ROM_SKIP(7) ) // == 41-7m.9a
	ROMX_LOAD( "41_24.5e",  0x000004, 0x20000, CRC(5aa43cee) SHA1(15e1aa2fee8deb25eefba067395961fee3e4c84d) , ROM_SKIP(7) ) // == 41-1m.3a
	ROMX_LOAD( "41_17.5c",  0x000005, 0x20000, CRC(bbeff902) SHA1(1bf421072387ab7d59b57ecd33cb5ae6fd8ecd06) , ROM_SKIP(7) ) // == 41-1m.3a
	ROMX_LOAD( "41_38.8h",  0x000006, 0x20000, CRC(8889c0aa) SHA1(23b74cc78fe1057c1776ea1f7371801441125a85) , ROM_SKIP(7) ) // == 41-3m.5a
	ROMX_LOAD( "41_32.8f",  0x000007, 0x20000, CRC(f0168249) SHA1(1ce2330ea4460e7e72f16a1f7edbfa8abba6b5a0) , ROM_SKIP(7) ) // == 41-3m.5a
	ROMX_LOAD( "41_10.5b",  0x100000, 0x20000, CRC(b7eb6a6d) SHA1(444b49e81cd46e90a9e1dfcf465f19c38189b2d7) , ROM_SKIP(7) ) // == 41-5m.7a
	ROMX_LOAD( "41_02.5a",  0x100001, 0x20000, CRC(802e8153) SHA1(baf19220c53949dac0d0b75368d5f1ee6c32b619) , ROM_SKIP(7) ) // == 41-5m.7a
	ROMX_LOAD( "41_14.10b", 0x100002, 0x20000, CRC(5a33f676) SHA1(96794463ddfae7ce4a7d7e6b65ff1b8da7b3dd74) , ROM_SKIP(7) ) // == 41-7m.9a
	ROMX_LOAD( "41_06.10a", 0x100003, 0x20000, CRC(4e53650b) SHA1(a5b19b1ce38f04296dee38d35015869607ecc260) , ROM_SKIP(7) ) // == 41-7m.9a
	ROMX_LOAD( "41_25.7e",  0x100004, 0x20000, CRC(94add360) SHA1(6f3542f028bdc453d0254de599b124240ea98259) , ROM_SKIP(7) ) // == 41-1m.3a
	ROMX_LOAD( "41_18.7c",  0x100005, 0x20000, CRC(a5e1c1f3) SHA1(1961ad3c8965c51da8a19a7283333f3a19bb2030) , ROM_SKIP(7) ) // == 41-1m.3a
	ROMX_LOAD( "41_39.9h",  0x100006, 0x20000, CRC(5b5c3949) SHA1(485b1baecee6bb9a6667baf404375fc758bdfb76) , ROM_SKIP(7) ) // == 41-3m.5a
	ROMX_LOAD( "41_33.9f",  0x100007, 0x20000, CRC(7a31b0e2) SHA1(f2eca12f4e89266d923a94d0837c555bc1c6cb29) , ROM_SKIP(7) ) // == 41-3m.5a

	ROM_REGION( 0x18000, "audiocpu", 0 ) /* 64k for the audio CPU (+banks) */
	ROM_LOAD( "41_23.13b",  0x00000, 0x08000, CRC(0f9d8527) SHA1(3a00dd5772f38081fde11d8d61ba467379e2a636) )    // == 41_9.12b
	ROM_CONTINUE(           0x10000, 0x08000 )

	ROM_REGION( 0x40000, "oki", 0 ) /* Samples */
	ROM_LOAD( "41_30.12c",  0x00000, 0x20000, CRC(d1f15aeb) SHA1(88089383f2d54fc97026a67f067d448eee5bd0c2) )    // == 41_18.11c
	ROM_LOAD( "41_31.13c",  0x20000, 0x20000, CRC(15aec3a6) SHA1(8153c03aba005bab62bf0e8b3d15ec1c346326fd) )    // == 41_19.12c

	ROM_REGION( 0x0200, "aboardplds", 0 )
	ROM_LOAD( "buf1",         0x0000, 0x0117, CRC(eb122de7) SHA1(b26b5bfe258e3e184f069719f9fd008d6b8f6b9b) )
	ROM_LOAD( "ioa1",         0x0000, 0x0117, CRC(59c7ee3b) SHA1(fbb887c5b4f5cb8df77cec710eaac2985bc482a6) )
	ROM_LOAD( "prg1",         0x0000, 0x0117, CRC(f1129744) SHA1(a5300f301c1a08a7da768f0773fa0fe3f683b237) )
	ROM_LOAD( "rom1",         0x0000, 0x0117, CRC(41dc73b9) SHA1(7d4c9f1693c821fbf84e32dd6ef62ddf14967845) )
	ROM_LOAD( "sou1",         0x0000, 0x0117, CRC(84f4b2fe) SHA1(dcc9e86cc36316fe42eace02d6df75d08bc8bb6d) )

	ROM_REGION( 0x0200, "bboardplds", 0 )
	ROM_LOAD( "yi22b.1a",     0x0000, 0x0117, NO_DUMP )
	ROM_LOAD( "lwio.12e",     0x0000, 0x0117, CRC(ad52b90c) SHA1(f0fd6aeea515ee449320fe15684e6b3ab7f97bf4) )    // pal verification required
ROM_END

/* B-Board 89624B-3 */
ROM_START( mercs )
	ROM_REGION( CODE_SIZE, "maincpu", 0 )      /* 68000 code */
	ROM_LOAD16_BYTE( "so2_30e.11f",     0x00000, 0x20000, CRC(e17f9bf7) SHA1(f44bb378de428b429c97a21f74829182d3187ace) )
	ROM_LOAD16_BYTE( "so2_35e.11h",     0x00001, 0x20000, CRC(78e63575) SHA1(5776de0daaaedd0dec2cec8d088a0fd8bb3d4dbe) )
	ROM_LOAD16_BYTE( "so2_31e.12f",     0x40000, 0x20000, CRC(51204d36) SHA1(af288fc369d092f38ea73be967705aacade06f28) )
	ROM_LOAD16_BYTE( "so2_36e.12h",     0x40001, 0x20000, CRC(9cfba8b4) SHA1(df8ee5e3a68f056f68f096c46fdb548f63d29446) )
	ROM_LOAD16_WORD_SWAP( "so2-32m.8h", 0x80000, 0x80000, CRC(2eb5cf0c) SHA1(e0d765fb6957d156ffd40cabf51ba6098cbbeb19) )

	ROM_REGION( 0x300000, "gfx", 0 )
	ROMX_LOAD( "so2-6m.8a",  0x000000, 0x80000, CRC(aa6102af) SHA1(4a45f3547a3640f256e5e20bfd72784f880f03f5) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "so2-8m.10a", 0x000002, 0x80000, CRC(839e6869) SHA1(7741141a9f1b1e2956edc1d11f9cc3974390c4ed) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "so2-2m.4a",  0x000004, 0x80000, CRC(597c2875) SHA1(440bd04db2c121a6976e5e1027071d28812942d3) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "so2-4m.6a",  0x000006, 0x80000, CRC(912a9ca0) SHA1(b226a4a388e57e23d7a7559773ebee434125a2e4) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "so2_24.7d",  0x200000, 0x20000, CRC(3f254efe) SHA1(5db36eb98a6d3c7acccb561d92c1988d1330cbbf) , ROM_SKIP(7) )
	ROMX_LOAD( "so2_14.7c",  0x200001, 0x20000, CRC(f5a8905e) SHA1(fada8b635d490c06b75711ed505a025bb0aa4454) , ROM_SKIP(7) )
	ROMX_LOAD( "so2_26.9d",  0x200002, 0x20000, CRC(f3aa5a4a) SHA1(bcb3396de5524fffd4110bfbeeeca1c936990eb3) , ROM_SKIP(7) )
	ROMX_LOAD( "so2_16.9c",  0x200003, 0x20000, CRC(b43cd1a8) SHA1(01c2bb802469848a172968802a674c0045a8b8dc) , ROM_SKIP(7) )
	ROMX_LOAD( "so2_20.3d",  0x200004, 0x20000, CRC(8ca751a3) SHA1(e93bbe7311f14e7e3cbfb42b83fd7fee4bb9cefc) , ROM_SKIP(7) )
	ROMX_LOAD( "so2_10.3c",  0x200005, 0x20000, CRC(e9f569fd) SHA1(39ae9eacdf1f35ef90d131444c37958d7aaf7238) , ROM_SKIP(7) )
	ROMX_LOAD( "so2_22.5d",  0x200006, 0x20000, CRC(fce9a377) SHA1(5de5f696f63326f2cb4c38bcb05e07bcf2246071) , ROM_SKIP(7) )
	ROMX_LOAD( "so2_12.5c",  0x200007, 0x20000, CRC(b7df8a06) SHA1(b42cb0d3f55a1e8fe8afbbd9aeae50074cdc5f08) , ROM_SKIP(7) )

	ROM_REGION( 0x18000, "audiocpu", 0 ) /* 64k for the audio CPU (+banks) */
	ROM_LOAD( "so2_09.12b",  0x00000, 0x08000, CRC(d09d7c7a) SHA1(8e8532be08818c855d9c3ce45716eb07cfab5767) )
	ROM_CONTINUE(            0x10000, 0x08000 )

	ROM_REGION( 0x40000, "oki", 0 ) /* Samples */
	ROM_LOAD( "so2_18.11c",  0x00000, 0x20000, CRC(bbea1643) SHA1(d43d68a120550067bf0b181f88687ad230cd7908) )
	ROM_LOAD( "so2_19.12c",  0x20000, 0x20000, CRC(ac58aa71) SHA1(93102272e358bc49d3936302efdc5bb68df84d68) )

	ROM_REGION( 0x0200, "aboardplds", 0 )
	ROM_LOAD( "buf1",         0x0000, 0x0117, CRC(eb122de7) SHA1(b26b5bfe258e3e184f069719f9fd008d6b8f6b9b) )
	ROM_LOAD( "ioa1",         0x0000, 0x0117, CRC(59c7ee3b) SHA1(fbb887c5b4f5cb8df77cec710eaac2985bc482a6) )
	ROM_LOAD( "prg1",         0x0000, 0x0117, CRC(f1129744) SHA1(a5300f301c1a08a7da768f0773fa0fe3f683b237) )
	ROM_LOAD( "rom1",         0x0000, 0x0117, CRC(41dc73b9) SHA1(7d4c9f1693c821fbf84e32dd6ef62ddf14967845) )
	ROM_LOAD( "sou1",         0x0000, 0x0117, CRC(84f4b2fe) SHA1(dcc9e86cc36316fe42eace02d6df75d08bc8bb6d) )

	ROM_REGION( 0x0200, "bboardplds", 0 )
	ROM_LOAD( "o224b.1a",     0x0000, 0x0117, CRC(c211c8cd) SHA1(d9464792e663549e6ad20aac6484622298f88a78) )
	ROM_LOAD( "iob1.11e",     0x0000, 0x0117, CRC(3abc0700) SHA1(973043aa46ec6d5d1db20dc9d5937005a0f9f6ae) )

	ROM_REGION( 0x0200, "cboardplds", 0 )
	ROM_LOAD( "c628",         0x0000, 0x0117, NO_DUMP )
ROM_END

/* B-Board 89624B-3 */
ROM_START( mercsu )
	ROM_REGION( CODE_SIZE, "maincpu", 0 )      /* 68000 code */
	ROM_LOAD16_BYTE( "so2_30a.11f",     0x00000, 0x20000, CRC(e4e725d7) SHA1(b0454dedeb741a7dd4ceb18bac958417ca74a7e6) )
	ROM_LOAD16_BYTE( "so2_35a.11h",     0x00001, 0x20000, CRC(e7843445) SHA1(192c85ced637e05b37ed889246ebb73e792e984b) )
	ROM_LOAD16_BYTE( "so2_31a.12f",     0x40000, 0x20000, CRC(c0b91dea) SHA1(5c1d086ae09e4f66384a03994b3c5e12d80582ff) )
	ROM_LOAD16_BYTE( "so2_36a.12h",     0x40001, 0x20000, CRC(591edf6c) SHA1(68d77e21fe32e0b95d2fabe40bc1cadd419ab0bd) )
	ROM_LOAD16_WORD_SWAP( "so2-32m.8h", 0x80000, 0x80000, CRC(2eb5cf0c) SHA1(e0d765fb6957d156ffd40cabf51ba6098cbbeb19) )

	ROM_REGION( 0x300000, "gfx", 0 )
	ROMX_LOAD( "so2-6m.8a",  0x000000, 0x80000, CRC(aa6102af) SHA1(4a45f3547a3640f256e5e20bfd72784f880f03f5) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "so2-8m.10a", 0x000002, 0x80000, CRC(839e6869) SHA1(7741141a9f1b1e2956edc1d11f9cc3974390c4ed) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "so2-2m.4a",  0x000004, 0x80000, CRC(597c2875) SHA1(440bd04db2c121a6976e5e1027071d28812942d3) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "so2-4m.6a",  0x000006, 0x80000, CRC(912a9ca0) SHA1(b226a4a388e57e23d7a7559773ebee434125a2e4) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "so2_24.7d",  0x200000, 0x20000, CRC(3f254efe) SHA1(5db36eb98a6d3c7acccb561d92c1988d1330cbbf) , ROM_SKIP(7) )
	ROMX_LOAD( "so2_14.7c",  0x200001, 0x20000, CRC(f5a8905e) SHA1(fada8b635d490c06b75711ed505a025bb0aa4454) , ROM_SKIP(7) )
	ROMX_LOAD( "so2_26.9d",  0x200002, 0x20000, CRC(f3aa5a4a) SHA1(bcb3396de5524fffd4110bfbeeeca1c936990eb3) , ROM_SKIP(7) )
	ROMX_LOAD( "so2_16.9c",  0x200003, 0x20000, CRC(b43cd1a8) SHA1(01c2bb802469848a172968802a674c0045a8b8dc) , ROM_SKIP(7) )
	ROMX_LOAD( "so2_20.3d",  0x200004, 0x20000, CRC(8ca751a3) SHA1(e93bbe7311f14e7e3cbfb42b83fd7fee4bb9cefc) , ROM_SKIP(7) )
	ROMX_LOAD( "so2_10.3c",  0x200005, 0x20000, CRC(e9f569fd) SHA1(39ae9eacdf1f35ef90d131444c37958d7aaf7238) , ROM_SKIP(7) )
	ROMX_LOAD( "so2_22.5d",  0x200006, 0x20000, CRC(fce9a377) SHA1(5de5f696f63326f2cb4c38bcb05e07bcf2246071) , ROM_SKIP(7) )
	ROMX_LOAD( "so2_12.5c",  0x200007, 0x20000, CRC(b7df8a06) SHA1(b42cb0d3f55a1e8fe8afbbd9aeae50074cdc5f08) , ROM_SKIP(7) )

	ROM_REGION( 0x18000, "audiocpu", 0 ) /* 64k for the audio CPU (+banks) */
	ROM_LOAD( "so2_09.12b",  0x00000, 0x08000, CRC(d09d7c7a) SHA1(8e8532be08818c855d9c3ce45716eb07cfab5767) )
	ROM_CONTINUE(            0x10000, 0x08000 )

	ROM_REGION( 0x40000, "oki", 0 ) /* Samples */
	ROM_LOAD( "so2_18.11c",  0x00000, 0x20000, CRC(bbea1643) SHA1(d43d68a120550067bf0b181f88687ad230cd7908) )
	ROM_LOAD( "so2_19.12c",  0x20000, 0x20000, CRC(ac58aa71) SHA1(93102272e358bc49d3936302efdc5bb68df84d68) )

	ROM_REGION( 0x0200, "aboardplds", 0 )
	ROM_LOAD( "buf1",         0x0000, 0x0117, CRC(eb122de7) SHA1(b26b5bfe258e3e184f069719f9fd008d6b8f6b9b) )
	ROM_LOAD( "ioa1",         0x0000, 0x0117, CRC(59c7ee3b) SHA1(fbb887c5b4f5cb8df77cec710eaac2985bc482a6) )
	ROM_LOAD( "prg1",         0x0000, 0x0117, CRC(f1129744) SHA1(a5300f301c1a08a7da768f0773fa0fe3f683b237) )
	ROM_LOAD( "rom1",         0x0000, 0x0117, CRC(41dc73b9) SHA1(7d4c9f1693c821fbf84e32dd6ef62ddf14967845) )
	ROM_LOAD( "sou1",         0x0000, 0x0117, CRC(84f4b2fe) SHA1(dcc9e86cc36316fe42eace02d6df75d08bc8bb6d) )

	ROM_REGION( 0x0200, "bboardplds", 0 )
	ROM_LOAD( "o224b.1a",     0x0000, 0x0117, CRC(c211c8cd) SHA1(d9464792e663549e6ad20aac6484622298f88a78) )
	ROM_LOAD( "iob1.11e",     0x0000, 0x0117, CRC(3abc0700) SHA1(973043aa46ec6d5d1db20dc9d5937005a0f9f6ae) )

	ROM_REGION( 0x0200, "cboardplds", 0 )
	ROM_LOAD( "c628",         0x0000, 0x0117, NO_DUMP )
ROM_END

/* B-Board 89624B-3 */
ROM_START( mercsur1 )
	ROM_REGION( CODE_SIZE, "maincpu", 0 )      /* 68000 code */
	ROM_LOAD16_BYTE( "so2_30.11f",      0x00000, 0x20000, CRC(e17f9bf7) SHA1(f44bb378de428b429c97a21f74829182d3187ace) )    // == so2_30e.11f
	ROM_LOAD16_BYTE( "so2_35.11h",      0x00001, 0x20000, CRC(4477df61) SHA1(e9b42357c7073c098e8fde7e7d0e4a6e3062fd0d) )
	ROM_LOAD16_BYTE( "so2_31.12f",      0x40000, 0x20000, CRC(51204d36) SHA1(af288fc369d092f38ea73be967705aacade06f28) )    // == so2_31e.12f
	ROM_LOAD16_BYTE( "so2_36.12h",      0x40001, 0x20000, CRC(9cfba8b4) SHA1(df8ee5e3a68f056f68f096c46fdb548f63d29446) )    // == so2_36e.12h
	ROM_LOAD16_WORD_SWAP( "so2-32m.8h", 0x80000, 0x80000, CRC(2eb5cf0c) SHA1(e0d765fb6957d156ffd40cabf51ba6098cbbeb19) )

	ROM_REGION( 0x300000, "gfx", 0 )
	ROMX_LOAD( "so2-6m.8a",  0x000000, 0x80000, CRC(aa6102af) SHA1(4a45f3547a3640f256e5e20bfd72784f880f03f5) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "so2-8m.10a", 0x000002, 0x80000, CRC(839e6869) SHA1(7741141a9f1b1e2956edc1d11f9cc3974390c4ed) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "so2-2m.4a",  0x000004, 0x80000, CRC(597c2875) SHA1(440bd04db2c121a6976e5e1027071d28812942d3) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "so2-4m.6a",  0x000006, 0x80000, CRC(912a9ca0) SHA1(b226a4a388e57e23d7a7559773ebee434125a2e4) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "so2_24.7d",  0x200000, 0x20000, CRC(3f254efe) SHA1(5db36eb98a6d3c7acccb561d92c1988d1330cbbf) , ROM_SKIP(7) )
	ROMX_LOAD( "so2_14.7c",  0x200001, 0x20000, CRC(f5a8905e) SHA1(fada8b635d490c06b75711ed505a025bb0aa4454) , ROM_SKIP(7) )
	ROMX_LOAD( "so2_26.9d",  0x200002, 0x20000, CRC(f3aa5a4a) SHA1(bcb3396de5524fffd4110bfbeeeca1c936990eb3) , ROM_SKIP(7) )
	ROMX_LOAD( "so2_16.9c",  0x200003, 0x20000, CRC(b43cd1a8) SHA1(01c2bb802469848a172968802a674c0045a8b8dc) , ROM_SKIP(7) )
	ROMX_LOAD( "so2_20.3d",  0x200004, 0x20000, CRC(8ca751a3) SHA1(e93bbe7311f14e7e3cbfb42b83fd7fee4bb9cefc) , ROM_SKIP(7) )
	ROMX_LOAD( "so2_10.3c",  0x200005, 0x20000, CRC(e9f569fd) SHA1(39ae9eacdf1f35ef90d131444c37958d7aaf7238) , ROM_SKIP(7) )
	ROMX_LOAD( "so2_22.5d",  0x200006, 0x20000, CRC(fce9a377) SHA1(5de5f696f63326f2cb4c38bcb05e07bcf2246071) , ROM_SKIP(7) )
	ROMX_LOAD( "so2_12.5c",  0x200007, 0x20000, CRC(b7df8a06) SHA1(b42cb0d3f55a1e8fe8afbbd9aeae50074cdc5f08) , ROM_SKIP(7) )

	ROM_REGION( 0x18000, "audiocpu", 0 ) /* 64k for the audio CPU (+banks) */
	ROM_LOAD( "so2_09.12b",  0x00000, 0x08000, CRC(d09d7c7a) SHA1(8e8532be08818c855d9c3ce45716eb07cfab5767) )
	ROM_CONTINUE(            0x10000, 0x08000 )

	ROM_REGION( 0x40000, "oki", 0 ) /* Samples */
	ROM_LOAD( "so2_18.11c",  0x00000, 0x20000, CRC(bbea1643) SHA1(d43d68a120550067bf0b181f88687ad230cd7908) )
	ROM_LOAD( "so2_19.12c",  0x20000, 0x20000, CRC(ac58aa71) SHA1(93102272e358bc49d3936302efdc5bb68df84d68) )

	ROM_REGION( 0x0200, "aboardplds", 0 )
	ROM_LOAD( "buf1",         0x0000, 0x0117, CRC(eb122de7) SHA1(b26b5bfe258e3e184f069719f9fd008d6b8f6b9b) )
	ROM_LOAD( "ioa1",         0x0000, 0x0117, CRC(59c7ee3b) SHA1(fbb887c5b4f5cb8df77cec710eaac2985bc482a6) )
	ROM_LOAD( "prg1",         0x0000, 0x0117, CRC(f1129744) SHA1(a5300f301c1a08a7da768f0773fa0fe3f683b237) )
	ROM_LOAD( "rom1",         0x0000, 0x0117, CRC(41dc73b9) SHA1(7d4c9f1693c821fbf84e32dd6ef62ddf14967845) )
	ROM_LOAD( "sou1",         0x0000, 0x0117, CRC(84f4b2fe) SHA1(dcc9e86cc36316fe42eace02d6df75d08bc8bb6d) )

	ROM_REGION( 0x0200, "bboardplds", 0 )
	ROM_LOAD( "o224b.1a",     0x0000, 0x0117, CRC(c211c8cd) SHA1(d9464792e663549e6ad20aac6484622298f88a78) )
	ROM_LOAD( "iob1.11e",     0x0000, 0x0117, CRC(3abc0700) SHA1(973043aa46ec6d5d1db20dc9d5937005a0f9f6ae) )

	ROM_REGION( 0x0200, "cboardplds", 0 )
	ROM_LOAD( "c628",         0x0000, 0x0117, NO_DUMP )
ROM_END

/* B-Board 89625B-1 */
ROM_START( mercsj )
	ROM_REGION( CODE_SIZE, "maincpu", 0 )      /* 68000 code */
	ROM_LOAD16_BYTE( "so2_36.12f", 0x00000, 0x20000, CRC(e17f9bf7) SHA1(f44bb378de428b429c97a21f74829182d3187ace) ) // == so2_30e.11f
	ROM_LOAD16_BYTE( "so2_42.12h", 0x00001, 0x20000, CRC(2c3884c6) SHA1(98c3e93741d2344fe0a699aacdc5038bdd9007a0) )
	ROM_LOAD16_BYTE( "so2_37.13f", 0x40000, 0x20000, CRC(51204d36) SHA1(af288fc369d092f38ea73be967705aacade06f28) ) // == so2_31e.12f
	ROM_LOAD16_BYTE( "so2_43.13h", 0x40001, 0x20000, CRC(9cfba8b4) SHA1(df8ee5e3a68f056f68f096c46fdb548f63d29446) ) // == so2_36e.12h
	ROM_LOAD16_BYTE( "so2_34.10f", 0x80000, 0x20000, CRC(b8dae95f) SHA1(2db4a20afd40b772a16f1bee999a0b82d3379ac7) ) // == so2-32m.8h
	ROM_LOAD16_BYTE( "so2_40.10h", 0x80001, 0x20000, CRC(de37771c) SHA1(45e1e2ef4e46dbe8881e809d700fdd3d06a03c92) ) // == so2-32m.8h
	ROM_LOAD16_BYTE( "so2_35.11f", 0xc0000, 0x20000, CRC(7d24394d) SHA1(2f4cf51fcfc1b960b68cfe3f1b75914402f2c702) ) // == so2-32m.8h
	ROM_LOAD16_BYTE( "so2_41.11h", 0xc0001, 0x20000, CRC(914f85e0) SHA1(0b32adf2d3c83e187a5f670de18728726fabb731) ) // == so2-32m.8h

	ROM_REGION( 0x300000, "gfx", 0 )
	ROMX_LOAD( "so2_09.4b",  0x000000, 0x20000, CRC(690c261d) SHA1(27219101fb62a0c0378e6f5d2f9c0bb5c9397193) , ROM_SKIP(7) )    // == so2-6m.8a
	ROMX_LOAD( "so2_01.4a",  0x000001, 0x20000, CRC(31fd2715) SHA1(d80b7a93c3b4e5e482fe6bb9ed9d261377980351) , ROM_SKIP(7) )    // == so2-6m.8a
	ROMX_LOAD( "so2_13.9b",  0x000002, 0x20000, CRC(b5e48282) SHA1(5f387929b4f1ebb8cb8c24138317d4208e2cf7c2) , ROM_SKIP(7) )    // == so2-8m.10a
	ROMX_LOAD( "so2_05.9a",  0x000003, 0x20000, CRC(54bed82c) SHA1(4a45ceaec3f6162443b2c62b816612c19a609341) , ROM_SKIP(7) )    // == so2-8m.10a
	ROMX_LOAD( "so2_24.5e",  0x000004, 0x20000, CRC(78b6f0cb) SHA1(679c39d71d3b73db088d0ab017e80fd8316045b5) , ROM_SKIP(7) )    // == so2-2m.4a
	ROMX_LOAD( "so2_17.5c",  0x000005, 0x20000, CRC(e78bb308) SHA1(7c0c2cf4c79e0bb3c401ba8fdcc88ccc3ed64246) , ROM_SKIP(7) )    // == so2-2m.4a
	ROMX_LOAD( "so2_38.8h",  0x000006, 0x20000, CRC(0010a9a2) SHA1(03fedaaa939b56afb1b376243542e68da68e2690) , ROM_SKIP(7) )    // == so2-4m.6a
	ROMX_LOAD( "so2_32.8f",  0x000007, 0x20000, CRC(75dffc9a) SHA1(ef296e1c0742e0b5a6e104032f0492151e631691) , ROM_SKIP(7) )    // == so2-4m.6a
	ROMX_LOAD( "so2_10.5b",  0x100000, 0x20000, CRC(2f871714) SHA1(8a39f120e3f50fc9a7e6cee659260b2f823fb0e0) , ROM_SKIP(7) )    // == so2-6m.8a
	ROMX_LOAD( "so2_02.5a",  0x100001, 0x20000, CRC(b4b2a0b7) SHA1(ee42ed3de9021e8d08d6c7115f2de73476b93452) , ROM_SKIP(7) )    // == so2-6m.8a
	ROMX_LOAD( "so2_14.10b", 0x100002, 0x20000, CRC(737a744b) SHA1(8e1477a67862f7c0c598d3d1a1f633946e7ab31b) , ROM_SKIP(7) )    // == so2-8m.10a
	ROMX_LOAD( "so2_06.10a", 0x100003, 0x20000, CRC(9d756f51) SHA1(4e9773ee25f6a952fb4f541d37e5e46e4089fd07) , ROM_SKIP(7) )    // == so2-8m.10a
	ROMX_LOAD( "so2_25.7e",  0x100004, 0x20000, CRC(6d0e05d6) SHA1(47cbec235bd4b250db007218dc357101ae453d1a) , ROM_SKIP(7) )    // == so2-2m.4a
	ROMX_LOAD( "so2_18.7c",  0x100005, 0x20000, CRC(96f61f4e) SHA1(954334bd8c2d2b02175de60d6a181a23e723d040) , ROM_SKIP(7) )    // == so2-2m.4a
	ROMX_LOAD( "so2_39.9h",  0x100006, 0x20000, CRC(d52ba336) SHA1(49550d316e575a4e64ea6a5f769f3cd716be6df5) , ROM_SKIP(7) )    // == so2-4m.6a
	ROMX_LOAD( "so2_33.9f",  0x100007, 0x20000, CRC(39b90d25) SHA1(1089cca168a4abeb398fa93eddd4d9fff70d5db5) , ROM_SKIP(7) )    // == so2-4m.6a
	ROMX_LOAD( "so2_11.7b",  0x200000, 0x20000, CRC(3f254efe) SHA1(5db36eb98a6d3c7acccb561d92c1988d1330cbbf) , ROM_SKIP(7) )    // == so2_24.7d
	ROMX_LOAD( "so2_03.7a",  0x200001, 0x20000, CRC(f5a8905e) SHA1(fada8b635d490c06b75711ed505a025bb0aa4454) , ROM_SKIP(7) )    // == so2_14.7c
	ROMX_LOAD( "so2_15.11b", 0x200002, 0x20000, CRC(f3aa5a4a) SHA1(bcb3396de5524fffd4110bfbeeeca1c936990eb3) , ROM_SKIP(7) )    // == so2_26.9d
	ROMX_LOAD( "so2_07.11a", 0x200003, 0x20000, CRC(b43cd1a8) SHA1(01c2bb802469848a172968802a674c0045a8b8dc) , ROM_SKIP(7) )    // == so2_16.9c
	ROMX_LOAD( "so2_26.8e",  0x200004, 0x20000, CRC(8ca751a3) SHA1(e93bbe7311f14e7e3cbfb42b83fd7fee4bb9cefc) , ROM_SKIP(7) )    // == so2_20.3d
	ROMX_LOAD( "so2_19.8c",  0x200005, 0x20000, CRC(e9f569fd) SHA1(39ae9eacdf1f35ef90d131444c37958d7aaf7238) , ROM_SKIP(7) )    // == so2_10.3c
	ROMX_LOAD( "so2_28.10e", 0x200006, 0x20000, CRC(fce9a377) SHA1(5de5f696f63326f2cb4c38bcb05e07bcf2246071) , ROM_SKIP(7) )    // == so2_22.5d
	ROMX_LOAD( "so2_21.10c", 0x200007, 0x20000, CRC(b7df8a06) SHA1(b42cb0d3f55a1e8fe8afbbd9aeae50074cdc5f08) , ROM_SKIP(7) )    // == so2_12.5c

	ROM_REGION( 0x18000, "audiocpu", 0 ) /* 64k for the audio CPU (+banks) */
	ROM_LOAD( "so2_23.13b",  0x00000, 0x08000, CRC(d09d7c7a) SHA1(8e8532be08818c855d9c3ce45716eb07cfab5767) )   // == so2_09.12b
	ROM_CONTINUE(            0x10000, 0x08000 )

	ROM_REGION( 0x40000, "oki", 0 ) /* Samples */
	ROM_LOAD( "so2_30.12c",  0x00000, 0x20000, CRC(bbea1643) SHA1(d43d68a120550067bf0b181f88687ad230cd7908) )   // == so2_18.11c
	ROM_LOAD( "so2_31.13c",  0x20000, 0x20000, CRC(ac58aa71) SHA1(93102272e358bc49d3936302efdc5bb68df84d68) )   // == so2_19.12c

	ROM_REGION( 0x0200, "aboardplds", 0 )
	ROM_LOAD( "buf1",         0x0000, 0x0117, CRC(eb122de7) SHA1(b26b5bfe258e3e184f069719f9fd008d6b8f6b9b) )
	ROM_LOAD( "ioa1",         0x0000, 0x0117, CRC(59c7ee3b) SHA1(fbb887c5b4f5cb8df77cec710eaac2985bc482a6) )
	ROM_LOAD( "prg1",         0x0000, 0x0117, CRC(f1129744) SHA1(a5300f301c1a08a7da768f0773fa0fe3f683b237) )
	ROM_LOAD( "rom1",         0x0000, 0x0117, CRC(41dc73b9) SHA1(7d4c9f1693c821fbf84e32dd6ef62ddf14967845) )
	ROM_LOAD( "sou1",         0x0000, 0x0117, CRC(84f4b2fe) SHA1(dcc9e86cc36316fe42eace02d6df75d08bc8bb6d) )

	ROM_REGION( 0x0200, "bboardplds", 0 )
	ROM_LOAD( "o222b.1a",     0x0000, 0x0117, NO_DUMP )
	ROM_LOAD( "lwio.12e",     0x0000, 0x0117, CRC(ad52b90c) SHA1(f0fd6aeea515ee449320fe15684e6b3ab7f97bf4) )    // pal verification required

	ROM_REGION( 0x0200, "cboardplds", 0 )
	ROM_LOAD( "c628",         0x0000, 0x0117, NO_DUMP )
ROM_END

/* B-Board 89624B-3 */
ROM_START( mtwins )
	ROM_REGION( CODE_SIZE, "maincpu", 0 )      /* 68000 code */
	ROM_LOAD16_BYTE( "che_30.11f",     0x00000, 0x20000, CRC(9a2a2db1) SHA1(57524e76311afc8ab5d5affa76c85cb1be5a1faf) )
	ROM_LOAD16_BYTE( "che_35.11h",     0x00001, 0x20000, CRC(a7f96b02) SHA1(b5fda02e5069f9e1cdafbacf98334510e9af8fcd) )
	ROM_LOAD16_BYTE( "che_31.12f",     0x40000, 0x20000, CRC(bbff8a99) SHA1(1f931fad9f43a1494f3b8dbcf910156d5b0bd458) )
	ROM_LOAD16_BYTE( "che_36.12h",     0x40001, 0x20000, CRC(0fa00c39) SHA1(6404d91590c5c521c8fe944a0aa7091e35b664ae) )
	ROM_LOAD16_WORD_SWAP( "ck-32m.8h", 0x80000, 0x80000, CRC(9b70bd41) SHA1(28ec37d9d0ace5b9fd212fdc02e0f13dc280c068) )

	ROM_REGION( 0x200000, "gfx", 0 )
	ROMX_LOAD( "ck-5m.7a", 0x000000, 0x80000, CRC(4ec75f15) SHA1(a4669e3864009b01894406db784116ad5cd2eced) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "ck-7m.9a", 0x000002, 0x80000, CRC(d85d00d6) SHA1(ca6ddcbfbb0f9ad98dc19f09e879fdac5b62d168) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "ck-1m.3a", 0x000004, 0x80000, CRC(f33ca9d4) SHA1(480d90ff16f27777cc7d7de6925ed6378b35dc27) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "ck-3m.5a", 0x000006, 0x80000, CRC(0ba2047f) SHA1(efee13b955c2ded52700025cecbb9fb301098b61) , ROM_GROUPWORD | ROM_SKIP(6) )

	ROM_REGION( 0x18000, "audiocpu", 0 ) /* 64k for the audio CPU (+banks) */
	ROM_LOAD( "ch_09.12b", 0x00000, 0x08000, CRC(4d4255b7) SHA1(81a76b58043af7252a854b7efc4109957ef0e679) )
	ROM_CONTINUE(          0x10000, 0x08000 )

	ROM_REGION( 0x40000, "oki", 0 ) /* Samples */
	ROM_LOAD( "ch_18.11c", 0x00000, 0x20000, CRC(f909e8de) SHA1(2dd5bd4076e7d5ded98b72919f868ea700df2e4f) )
	ROM_LOAD( "ch_19.12c", 0x20000, 0x20000, CRC(fc158cf7) SHA1(294b93d0aea60663ffe96364671552e944a1264b) )

	ROM_REGION( 0x0200, "aboardplds", 0 )
	ROM_LOAD( "buf1",         0x0000, 0x0117, CRC(eb122de7) SHA1(b26b5bfe258e3e184f069719f9fd008d6b8f6b9b) )
	ROM_LOAD( "ioa1",         0x0000, 0x0117, CRC(59c7ee3b) SHA1(fbb887c5b4f5cb8df77cec710eaac2985bc482a6) )
	ROM_LOAD( "prg1",         0x0000, 0x0117, CRC(f1129744) SHA1(a5300f301c1a08a7da768f0773fa0fe3f683b237) )
	ROM_LOAD( "rom1",         0x0000, 0x0117, CRC(41dc73b9) SHA1(7d4c9f1693c821fbf84e32dd6ef62ddf14967845) )
	ROM_LOAD( "sou1",         0x0000, 0x0117, CRC(84f4b2fe) SHA1(dcc9e86cc36316fe42eace02d6df75d08bc8bb6d) )

	ROM_REGION( 0x0200, "bboardplds", 0 )
	ROM_LOAD( "ck24b.1a",     0x0000, 0x0117, CRC(bd99c448) SHA1(2692c158f76769b0743103cc3a6d1c5d1f4f52ec) )
	ROM_LOAD( "iob1.11e",     0x0000, 0x0117, CRC(3abc0700) SHA1(973043aa46ec6d5d1db20dc9d5937005a0f9f6ae) )
ROM_END

/* B-Board 89625B-1 */
ROM_START( chikij )
	ROM_REGION( CODE_SIZE, "maincpu", 0 )      /* 68000 code */
	ROM_LOAD16_BYTE( "chj_36a.12f", 0x00000, 0x20000, CRC(ec1328d8) SHA1(a7111f9c264c56d1c6474ec3ad90e394a32a86f6) )
	ROM_LOAD16_BYTE( "chj_42a.12h", 0x00001, 0x20000, CRC(4ae13503) SHA1(c47db0445e107ad4fb62b74e277a7dc2b4d9b7ea) )
	ROM_LOAD16_BYTE( "chj_37a.13f", 0x40000, 0x20000, CRC(46d2cf7b) SHA1(5cb7ed3003d89a08882d4dcd326c8fd9430f0eac) )
	ROM_LOAD16_BYTE( "chj_43a.13h", 0x40001, 0x20000, CRC(8d387fe8) SHA1(7832ecd487b5ef4e49b5ea78e80e52f8e2dcaa17) )
	ROM_LOAD16_BYTE( "ch_34.10f",   0x80000, 0x20000, CRC(609ed2f9) SHA1(869924ff1bc78ac4b50bcfd37a8e76820a9fddf1) )    // == ck-32m.8h
	ROM_LOAD16_BYTE( "ch_40.10h",   0x80001, 0x20000, CRC(be0d8301) SHA1(28dbbb4176800b31068b1beecf54a78085092e5a) )    // == ck-32m.8h
	ROM_LOAD16_BYTE( "ch_35.11f",   0xc0000, 0x20000, CRC(b810867f) SHA1(c971d286c60a9b61f42ea3b792cf59847aacb965) )    // == ck-32m.8h
	ROM_LOAD16_BYTE( "ch_41.11h",   0xc0001, 0x20000, CRC(8ad96155) SHA1(673a5b5eb7330fbbc02aabcdb164efea193613a3) )    // == ck-32m.8h

	ROM_REGION( 0x200000, "gfx", 0 )
	ROMX_LOAD( "ch_09.4b",  0x000000, 0x20000, CRC(567ab3ca) SHA1(b3d1531d9307285fcecff937dce0bed7ce9e4253) , ROM_SKIP(7) ) // == ck-5m.7a
	ROMX_LOAD( "ch_01.4a",  0x000001, 0x20000, CRC(7f3b7b56) SHA1(cf78c3122628aa0e14f7b5017429aee35e9b266d) , ROM_SKIP(7) ) // == ck-5m.7a
	ROMX_LOAD( "ch_13.9b",  0x000002, 0x20000, CRC(12a7a8ba) SHA1(22eaa1c667213a37b2c700c5c80dbf9cb81b9f5f) , ROM_SKIP(7) ) // == ck-7m.9a
	ROMX_LOAD( "ch_05.9a",  0x000003, 0x20000, CRC(6c1afb9a) SHA1(cef28a63550c6ec3fe7cd7ec478a7fb726df4e27) , ROM_SKIP(7) ) // == ck-7m.9a
	ROMX_LOAD( "ch_24.5e",  0x000004, 0x20000, CRC(9cb6e6bc) SHA1(af241438de5bd754e176eec8ad45941f9bf30523) , ROM_SKIP(7) ) // == ck-1m.3a
	ROMX_LOAD( "ch_17.5c",  0x000005, 0x20000, CRC(fe490846) SHA1(0d1ddb79c1ee2a7ff4bcdb960e18fc3cfb115e75) , ROM_SKIP(7) ) // == ck-1m.3a
	ROMX_LOAD( "ch_38.8h",  0x000006, 0x20000, CRC(6e5c8cb6) SHA1(438b897c14dccc0a185032b1ae2b93d71eed305a) , ROM_SKIP(7) ) // == ck-3m.5a
	ROMX_LOAD( "ch_32.8f",  0x000007, 0x20000, CRC(317d27b0) SHA1(5d8a3ab24fcf65b30e1c0affd80301e29e3bf208) , ROM_SKIP(7) ) // == ck-3m.5a
	ROMX_LOAD( "ch_10.5b",  0x100000, 0x20000, CRC(e8251a9b) SHA1(e0d5eaba20dc1132643b9ea334b36034ce97fc6d) , ROM_SKIP(7) ) // == ck-5m.7a
	ROMX_LOAD( "ch_02.5a",  0x100001, 0x20000, CRC(7c8c88fb) SHA1(29d1e5d6780b7d6875efff6b086fd03bef779df7) , ROM_SKIP(7) ) // == ck-5m.7a
	ROMX_LOAD( "ch_14.10b", 0x100002, 0x20000, CRC(4012ec4b) SHA1(041e08e1f407528da84b973d16c5f64f02bd14fe) , ROM_SKIP(7) ) // == ck-7m.9a
	ROMX_LOAD( "ch_06.10a", 0x100003, 0x20000, CRC(81884b2b) SHA1(1e4682183c167c95b2fb3986887c31d3e8911484) , ROM_SKIP(7) ) // == ck-7m.9a
	ROMX_LOAD( "ch_25.7e",  0x100004, 0x20000, CRC(1dfcbac5) SHA1(a7e419326a4bb7062c5bc7d0b3194b96e00a92d0) , ROM_SKIP(7) ) // == ck-1m.3a
	ROMX_LOAD( "ch_18.7c",  0x100005, 0x20000, CRC(516a34d1) SHA1(6516fa19d251898c6f55ab0fda760fc5404bef06) , ROM_SKIP(7) ) // == ck-1m.3a
	ROMX_LOAD( "ch_39.9h",  0x100006, 0x20000, CRC(872fb2a4) SHA1(1f5d12b78100dfba7f6c9a076f7923811467aa2b) , ROM_SKIP(7) ) // == ck-3m.5a
	ROMX_LOAD( "ch_33.9f",  0x100007, 0x20000, CRC(30dc5ded) SHA1(34a7a6f9aa61ce47116d63c2feb7ea8a427b93e0) , ROM_SKIP(7) ) // == ck-3m.5a

	ROM_REGION( 0x18000, "audiocpu", 0 ) /* 64k for the audio CPU (+banks) */
	ROM_LOAD( "ch_23.13b",  0x00000, 0x08000, CRC(4d4255b7) SHA1(81a76b58043af7252a854b7efc4109957ef0e679) )    // == ch_09.12b
	ROM_CONTINUE(           0x10000, 0x08000 )

	ROM_REGION( 0x40000, "oki", 0 ) /* Samples */
	ROM_LOAD( "ch_30.12c",  0x00000, 0x20000, CRC(f909e8de) SHA1(2dd5bd4076e7d5ded98b72919f868ea700df2e4f) )    // == ch_18.11c
	ROM_LOAD( "ch_31.13c",  0x20000, 0x20000, CRC(fc158cf7) SHA1(294b93d0aea60663ffe96364671552e944a1264b) )    // == ch_19.12c

	ROM_REGION( 0x0200, "aboardplds", 0 )
	ROM_LOAD( "buf1",         0x0000, 0x0117, CRC(eb122de7) SHA1(b26b5bfe258e3e184f069719f9fd008d6b8f6b9b) )
	ROM_LOAD( "ioa1",         0x0000, 0x0117, CRC(59c7ee3b) SHA1(fbb887c5b4f5cb8df77cec710eaac2985bc482a6) )
	ROM_LOAD( "prg1",         0x0000, 0x0117, CRC(f1129744) SHA1(a5300f301c1a08a7da768f0773fa0fe3f683b237) )
	ROM_LOAD( "rom1",         0x0000, 0x0117, CRC(41dc73b9) SHA1(7d4c9f1693c821fbf84e32dd6ef62ddf14967845) )
	ROM_LOAD( "sou1",         0x0000, 0x0117, CRC(84f4b2fe) SHA1(dcc9e86cc36316fe42eace02d6df75d08bc8bb6d) )

	ROM_REGION( 0x0200, "bboardplds", 0 )
	ROM_LOAD( "ck22b.1a",     0x0000, 0x0117, CRC(24fdfdeb) SHA1(850b760de694dc5455e63c89983266ecda836400) )
	ROM_LOAD( "iob1.12e",     0x0000, 0x0117, CRC(3abc0700) SHA1(973043aa46ec6d5d1db20dc9d5937005a0f9f6ae) )    // pal verification required
ROM_END

/* B-Board 89624B-3 */
ROM_START( msword )
	ROM_REGION( CODE_SIZE, "maincpu", 0 )      /* 68000 code */
	ROM_LOAD16_BYTE( "mse_30.11f",     0x00000, 0x20000, CRC(03fc8dbc) SHA1(a9e4e8a06e2d170faeae75a8b17fd65e6e5fecd4) )
	ROM_LOAD16_BYTE( "mse_35.11h",     0x00001, 0x20000, CRC(d5bf66cd) SHA1(37c5bc4deafd7037ec5cf09c88bb89f35ea3d95c) )
	ROM_LOAD16_BYTE( "mse_31.12f",     0x40000, 0x20000, CRC(30332bcf) SHA1(1c77c06028b77473276cb5dde5ecf414b43a7b78) )
	ROM_LOAD16_BYTE( "mse_36.12h",     0x40001, 0x20000, CRC(8f7d6ce9) SHA1(7694c940023c12520663593f973ddb4168a6bfa5) )
	ROM_LOAD16_WORD_SWAP( "ms-32m.8h", 0x80000, 0x80000, CRC(2475ddfc) SHA1(cc34dfae8124aa781320be6870a1929495eee456) )

	ROM_REGION( 0x200000, "gfx", 0 )
	ROMX_LOAD( "ms-5m.7a", 0x000000, 0x80000, CRC(c00fe7e2) SHA1(1ce82ea36996908620d3ac8aabd3650118d6c255) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "ms-7m.9a", 0x000002, 0x80000, CRC(4ccacac5) SHA1(f2e30edf6ad100da411584bb0b828420256a9d5c) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "ms-1m.3a", 0x000004, 0x80000, CRC(0d2bbe00) SHA1(dca13fc7ff63ad7fb175a71ada1ee22d21a8811d) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "ms-3m.5a", 0x000006, 0x80000, CRC(3a1a5bf4) SHA1(88a7cc0bf29b3516a97f661691500ff28e91a362) , ROM_GROUPWORD | ROM_SKIP(6) )

	ROM_REGION( 0x18000, "audiocpu", 0 ) /* 64k for the audio CPU (+banks) */
	ROM_LOAD( "ms_09.12b", 0x00000, 0x08000, CRC(57b29519) SHA1(a6b4fc2b9595d1a49f2b93581f107b68d484d156) )
	ROM_CONTINUE(          0x10000, 0x08000 )

	ROM_REGION( 0x40000, "oki", 0 ) /* Samples */
	ROM_LOAD( "ms_18.11c", 0x00000, 0x20000, CRC(fb64e90d) SHA1(d1a596ce2f8ac14a80b34335b173369a14b45f55) )
	ROM_LOAD( "ms_19.12c", 0x20000, 0x20000, CRC(74f892b9) SHA1(bf48db5c438154e7b96fd31fde1be4aad5cf25eb) )

	ROM_REGION( 0x0200, "aboardplds", 0 )
	ROM_LOAD( "buf1",         0x0000, 0x0117, CRC(eb122de7) SHA1(b26b5bfe258e3e184f069719f9fd008d6b8f6b9b) )
	ROM_LOAD( "ioa1",         0x0000, 0x0117, CRC(59c7ee3b) SHA1(fbb887c5b4f5cb8df77cec710eaac2985bc482a6) )
	ROM_LOAD( "prg1",         0x0000, 0x0117, CRC(f1129744) SHA1(a5300f301c1a08a7da768f0773fa0fe3f683b237) )
	ROM_LOAD( "rom1",         0x0000, 0x0117, CRC(41dc73b9) SHA1(7d4c9f1693c821fbf84e32dd6ef62ddf14967845) )
	ROM_LOAD( "sou1",         0x0000, 0x0117, CRC(84f4b2fe) SHA1(dcc9e86cc36316fe42eace02d6df75d08bc8bb6d) )

	ROM_REGION( 0x0200, "bboardplds", 0 )
	ROM_LOAD( "ms24b.1a",     0x0000, 0x0117, CRC(636dbe6d) SHA1(6622a2294f82e70e9eb5ff24f84e0dc13e9168b5) )
	ROM_LOAD( "iob1.11e",     0x0000, 0x0117, CRC(3abc0700) SHA1(973043aa46ec6d5d1db20dc9d5937005a0f9f6ae) )
ROM_END

/* B-Board 89624B-3 */
ROM_START( mswordr1 )
	ROM_REGION( CODE_SIZE, "maincpu", 0 )      /* 68000 code */
	ROM_LOAD16_BYTE( "ms_30.11f",      0x00000, 0x20000, CRC(21c1f078) SHA1(f32bd3b462cc84466244b362a66510b9d40ac2fd) )
	ROM_LOAD16_BYTE( "ms_35.11h",      0x00001, 0x20000, CRC(a540a73a) SHA1(1c91241ba0d17d13adaa68e231b95dfd49d93b6d) )
	ROM_LOAD16_BYTE( "ms_31.12f",      0x40000, 0x20000, CRC(d7e762b5) SHA1(6977130e9c0cd36d8a67e242c132df38f7aea5b7) )
	ROM_LOAD16_BYTE( "ms_36.12h",      0x40001, 0x20000, CRC(66f2dcdb) SHA1(287508b1c96762d0048a10272cf2cbd39a7fba5c) )
	ROM_LOAD16_WORD_SWAP( "ms-32m.8h", 0x80000, 0x80000, CRC(2475ddfc) SHA1(cc34dfae8124aa781320be6870a1929495eee456) )

	ROM_REGION( 0x200000, "gfx", 0 )
	ROMX_LOAD( "ms-5m.7a", 0x000000, 0x80000, CRC(c00fe7e2) SHA1(1ce82ea36996908620d3ac8aabd3650118d6c255) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "ms-7m.9a", 0x000002, 0x80000, CRC(4ccacac5) SHA1(f2e30edf6ad100da411584bb0b828420256a9d5c) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "ms-1m.3a", 0x000004, 0x80000, CRC(0d2bbe00) SHA1(dca13fc7ff63ad7fb175a71ada1ee22d21a8811d) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "ms-3m.5a", 0x000006, 0x80000, CRC(3a1a5bf4) SHA1(88a7cc0bf29b3516a97f661691500ff28e91a362) , ROM_GROUPWORD | ROM_SKIP(6) )

	ROM_REGION( 0x18000, "audiocpu", 0 ) /* 64k for the audio CPU (+banks) */
	ROM_LOAD( "ms_09.12b", 0x00000, 0x08000, CRC(57b29519) SHA1(a6b4fc2b9595d1a49f2b93581f107b68d484d156) )
	ROM_CONTINUE(          0x10000, 0x08000 )

	ROM_REGION( 0x40000, "oki", 0 ) /* Samples */
	ROM_LOAD( "ms_18.11c", 0x00000, 0x20000, CRC(fb64e90d) SHA1(d1a596ce2f8ac14a80b34335b173369a14b45f55) )
	ROM_LOAD( "ms_19.12c", 0x20000, 0x20000, CRC(74f892b9) SHA1(bf48db5c438154e7b96fd31fde1be4aad5cf25eb) )

	ROM_REGION( 0x0200, "aboardplds", 0 )
	ROM_LOAD( "buf1",         0x0000, 0x0117, CRC(eb122de7) SHA1(b26b5bfe258e3e184f069719f9fd008d6b8f6b9b) )
	ROM_LOAD( "ioa1",         0x0000, 0x0117, CRC(59c7ee3b) SHA1(fbb887c5b4f5cb8df77cec710eaac2985bc482a6) )
	ROM_LOAD( "prg1",         0x0000, 0x0117, CRC(f1129744) SHA1(a5300f301c1a08a7da768f0773fa0fe3f683b237) )
	ROM_LOAD( "rom1",         0x0000, 0x0117, CRC(41dc73b9) SHA1(7d4c9f1693c821fbf84e32dd6ef62ddf14967845) )
	ROM_LOAD( "sou1",         0x0000, 0x0117, CRC(84f4b2fe) SHA1(dcc9e86cc36316fe42eace02d6df75d08bc8bb6d) )

	ROM_REGION( 0x0200, "bboardplds", 0 )
	ROM_LOAD( "ms24b.1a",     0x0000, 0x0117, CRC(636dbe6d) SHA1(6622a2294f82e70e9eb5ff24f84e0dc13e9168b5) )
	ROM_LOAD( "iob1.11e",     0x0000, 0x0117, CRC(3abc0700) SHA1(973043aa46ec6d5d1db20dc9d5937005a0f9f6ae) )
ROM_END

/* B-Board 89624B-3 */
ROM_START( mswordu )
	ROM_REGION( CODE_SIZE, "maincpu", 0 )      /* 68000 code */
	ROM_LOAD16_BYTE( "msu_30.11f",     0x00000, 0x20000, CRC(d963c816) SHA1(e23844a60ddfc0a8a98c0ada9c3d58fce71c5484) )
	ROM_LOAD16_BYTE( "msu_35.11h",     0x00001, 0x20000, CRC(72f179b3) SHA1(8d31cdc84b02fc345fc78e8f231410adeb834c28) )
	ROM_LOAD16_BYTE( "msu_31.12f",     0x40000, 0x20000, CRC(20cd7904) SHA1(cea2db01be97f69dc10e9da80f3b46f6ddaa953a) )
	ROM_LOAD16_BYTE( "msu_36.12h",     0x40001, 0x20000, CRC(bf88c080) SHA1(b8cd0b127fd3e1afc45402e667ff4b4b01602384) )
	ROM_LOAD16_WORD_SWAP( "ms-32m.8h", 0x80000, 0x80000, CRC(2475ddfc) SHA1(cc34dfae8124aa781320be6870a1929495eee456) )

	ROM_REGION( 0x200000, "gfx", 0 )
	ROMX_LOAD( "ms-5m.7a", 0x000000, 0x80000, CRC(c00fe7e2) SHA1(1ce82ea36996908620d3ac8aabd3650118d6c255) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "ms-7m.9a", 0x000002, 0x80000, CRC(4ccacac5) SHA1(f2e30edf6ad100da411584bb0b828420256a9d5c) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "ms-1m.3a", 0x000004, 0x80000, CRC(0d2bbe00) SHA1(dca13fc7ff63ad7fb175a71ada1ee22d21a8811d) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "ms-3m.5a", 0x000006, 0x80000, CRC(3a1a5bf4) SHA1(88a7cc0bf29b3516a97f661691500ff28e91a362) , ROM_GROUPWORD | ROM_SKIP(6) )

	ROM_REGION( 0x18000, "audiocpu", 0 ) /* 64k for the audio CPU (+banks) */
	ROM_LOAD( "ms_09.12b", 0x00000, 0x08000, CRC(57b29519) SHA1(a6b4fc2b9595d1a49f2b93581f107b68d484d156) )
	ROM_CONTINUE(          0x10000, 0x08000 )

	ROM_REGION( 0x40000, "oki", 0 ) /* Samples */
	ROM_LOAD( "ms_18.11c", 0x00000, 0x20000, CRC(fb64e90d) SHA1(d1a596ce2f8ac14a80b34335b173369a14b45f55) )
	ROM_LOAD( "ms_19.12c", 0x20000, 0x20000, CRC(74f892b9) SHA1(bf48db5c438154e7b96fd31fde1be4aad5cf25eb) )

	ROM_REGION( 0x0200, "aboardplds", 0 )
	ROM_LOAD( "buf1",         0x0000, 0x0117, CRC(eb122de7) SHA1(b26b5bfe258e3e184f069719f9fd008d6b8f6b9b) )
	ROM_LOAD( "ioa1",         0x0000, 0x0117, CRC(59c7ee3b) SHA1(fbb887c5b4f5cb8df77cec710eaac2985bc482a6) )
	ROM_LOAD( "prg1",         0x0000, 0x0117, CRC(f1129744) SHA1(a5300f301c1a08a7da768f0773fa0fe3f683b237) )
	ROM_LOAD( "rom1",         0x0000, 0x0117, CRC(41dc73b9) SHA1(7d4c9f1693c821fbf84e32dd6ef62ddf14967845) )
	ROM_LOAD( "sou1",         0x0000, 0x0117, CRC(84f4b2fe) SHA1(dcc9e86cc36316fe42eace02d6df75d08bc8bb6d) )

	ROM_REGION( 0x0200, "bboardplds", 0 )
	ROM_LOAD( "ms24b.1a",     0x0000, 0x0117, CRC(636dbe6d) SHA1(6622a2294f82e70e9eb5ff24f84e0dc13e9168b5) )
	ROM_LOAD( "iob1.11e",     0x0000, 0x0117, CRC(3abc0700) SHA1(973043aa46ec6d5d1db20dc9d5937005a0f9f6ae) )
ROM_END

/* B-Board 89625B-1 */
ROM_START( mswordj )
	ROM_REGION( CODE_SIZE, "maincpu", 0 )      /* 68000 code */
	ROM_LOAD16_BYTE( "msj_36.12f", 0x00000, 0x20000, CRC(04f0ef50) SHA1(76dac695732ade1873bb6da51834ef90c6595689) )
	ROM_LOAD16_BYTE( "msj_42.12h", 0x00001, 0x20000, CRC(9fcbb9cd) SHA1(bfbf805ddecd3fa9e209a658526e1430ad9e459a) )
	ROM_LOAD16_BYTE( "msj_37.13f", 0x40000, 0x20000, CRC(6c060d70) SHA1(7fe56f125bc11156955bf0defc956fe7c18a1c72) )
	ROM_LOAD16_BYTE( "msj_43.13h", 0x40001, 0x20000, CRC(aec77787) SHA1(3260f9a80b67394dd90dbabdd544c9b8b31e5817) )
	ROM_LOAD16_BYTE( "ms_34.10f",  0x80000, 0x20000, CRC(0e59a62d) SHA1(d109e5edfb32ce3dc7c32e10a78fc3e943029a73) ) // == ms-32m.8h
	ROM_LOAD16_BYTE( "ms_40.10h",  0x80001, 0x20000, CRC(babade3a) SHA1(00acdcb5b316611a6df55e54f6ac4ec3503e1cac) ) // == ms-32m.8h
	ROM_LOAD16_BYTE( "ms_35.11f",  0xc0000, 0x20000, CRC(03da99d1) SHA1(f21a27f1122e1ee237a53b06ecd24737ac0d2c0e) ) // == ms-32m.8h
	ROM_LOAD16_BYTE( "ms_41.11h",  0xc0001, 0x20000, CRC(fadf99ea) SHA1(a3f3ef357f02c30b7f76941e5f854746774b0114) ) // == ms-32m.8h

	ROM_REGION( 0x200000, "gfx", 0 )
	ROMX_LOAD( "ms_09.4b",  0x000000, 0x20000, CRC(4adee6f6) SHA1(3becf055313f2fb90c42b839124d8ba4ccc047e0) , ROM_SKIP(7) ) // == ms-5m.7a
	ROMX_LOAD( "ms_01.4a",  0x000001, 0x20000, CRC(f7ab1b88) SHA1(9dae1c21c5379413e173fb13521821339ef0852e) , ROM_SKIP(7) ) // == ms-5m.7a
	ROMX_LOAD( "ms_13.9b",  0x000002, 0x20000, CRC(e01adc4b) SHA1(1eaf3511a3ffbd4b26a755d5f818e604cfd7764a) , ROM_SKIP(7) ) // == ms-7m.9a
	ROMX_LOAD( "ms_05.9a",  0x000003, 0x20000, CRC(f62c2369) SHA1(568d05c256889a2450b53373e68858ea6ae52007) , ROM_SKIP(7) ) // == ms-7m.9a
	ROMX_LOAD( "ms_24.5e",  0x000004, 0x20000, CRC(be64a3a1) SHA1(92a8f44a7f141fa189543f7b2564b9c0e44fb44f) , ROM_SKIP(7) ) // == ms-1m.3a
	ROMX_LOAD( "ms_17.5c",  0x000005, 0x20000, CRC(0bc1665f) SHA1(4b92e4f1f423964ece9d5ecbe960be3a06c42565) , ROM_SKIP(7) ) // == ms-1m.3a
	ROMX_LOAD( "ms_38.8h",  0x000006, 0x20000, CRC(904a2ed5) SHA1(8954c13f5c008ab6f28fa3adfba811c1173a2d88) , ROM_SKIP(7) ) // == ms-3m.5a
	ROMX_LOAD( "ms_32.8f",  0x000007, 0x20000, CRC(3d89c530) SHA1(7a9d82e1ccd0dd3b27d91013ef127233a0dd42a1) , ROM_SKIP(7) ) // == ms-3m.5a
	ROMX_LOAD( "ms_10.5b",  0x100000, 0x20000, CRC(f02c0718) SHA1(6055673fff3b57b7ba69eea8b45d7df36dfc5ba7) , ROM_SKIP(7) ) // == ms-5m.7a
	ROMX_LOAD( "ms_02.5a",  0x100001, 0x20000, CRC(d071a405) SHA1(205d706a992efa4bd772699472ab40eedd70f686) , ROM_SKIP(7) ) // == ms-5m.7a
	ROMX_LOAD( "ms_14.10b", 0x100002, 0x20000, CRC(dfb2e4df) SHA1(371c2a8d97eb0592b7b8767c1b992b4375933ac4) , ROM_SKIP(7) ) // == ms-7m.9a
	ROMX_LOAD( "ms_06.10a", 0x100003, 0x20000, CRC(d3ce2a91) SHA1(21ed0f7d4a1e9d0b1eb6a1cf4e5d082b773eb36c) , ROM_SKIP(7) ) // == ms-7m.9a
	ROMX_LOAD( "ms_25.7e",  0x100004, 0x20000, CRC(0f199d56) SHA1(0df4eda96b3327bd1d1fe6416e75e8b76b6593ac) , ROM_SKIP(7) ) // == ms-1m.3a
	ROMX_LOAD( "ms_18.7c",  0x100005, 0x20000, CRC(1ba76df2) SHA1(db7c16e6fde29c764278bdf76fc04c05567666e5) , ROM_SKIP(7) ) // == ms-1m.3a
	ROMX_LOAD( "ms_39.9h",  0x100006, 0x20000, CRC(01efce86) SHA1(665182bb61b1efb300422f7076d8538d2ca514ce) , ROM_SKIP(7) ) // == ms-3m.5a
	ROMX_LOAD( "ms_33.9f",  0x100007, 0x20000, CRC(ce25defc) SHA1(885eef9bac1d401f3e49c46294e573dd9cfad3a1) , ROM_SKIP(7) ) // == ms-3m.5a

	ROM_REGION( 0x18000, "audiocpu", 0 ) /* 64k for the audio CPU (+banks) */
	ROM_LOAD( "ms_23.13b",  0x00000, 0x08000, CRC(57b29519) SHA1(a6b4fc2b9595d1a49f2b93581f107b68d484d156) )    // == ms_09.12b
	ROM_CONTINUE(           0x10000, 0x08000 )

	ROM_REGION( 0x40000, "oki", 0 ) /* Samples */
	ROM_LOAD( "ms_30.12c",  0x00000, 0x20000, CRC(fb64e90d) SHA1(d1a596ce2f8ac14a80b34335b173369a14b45f55) )    // == ms_18.11c
	ROM_LOAD( "ms_31.13c",  0x20000, 0x20000, CRC(74f892b9) SHA1(bf48db5c438154e7b96fd31fde1be4aad5cf25eb) )    // == ms_19.12c

	ROM_REGION( 0x0200, "aboardplds", 0 )
	ROM_LOAD( "buf1",         0x0000, 0x0117, CRC(eb122de7) SHA1(b26b5bfe258e3e184f069719f9fd008d6b8f6b9b) )
	ROM_LOAD( "ioa1",         0x0000, 0x0117, CRC(59c7ee3b) SHA1(fbb887c5b4f5cb8df77cec710eaac2985bc482a6) )
	ROM_LOAD( "prg1",         0x0000, 0x0117, CRC(f1129744) SHA1(a5300f301c1a08a7da768f0773fa0fe3f683b237) )
	ROM_LOAD( "rom1",         0x0000, 0x0117, CRC(41dc73b9) SHA1(7d4c9f1693c821fbf84e32dd6ef62ddf14967845) )
	ROM_LOAD( "sou1",         0x0000, 0x0117, CRC(84f4b2fe) SHA1(dcc9e86cc36316fe42eace02d6df75d08bc8bb6d) )

	ROM_REGION( 0x0200, "bboardplds", 0 )
	ROM_LOAD( "ms22b.1a",     0x0000, 0x0117, CRC(dde86cb0) SHA1(d0b93a0b62a7cc3c3473da31fc00043392bc8f75) )
	ROM_LOAD( "iob1.12e",     0x0000, 0x0117, CRC(3abc0700) SHA1(973043aa46ec6d5d1db20dc9d5937005a0f9f6ae) )
ROM_END

/* B-Board 89624B-3 */
ROM_START( cawing )
	ROM_REGION( CODE_SIZE, "maincpu", 0 )      /* 68000 code */
	ROM_LOAD16_BYTE( "cae_30a.11f",    0x00000, 0x20000, CRC(91fceacd) SHA1(4845999a96fee829264346ca399fdd64a8408001) )
	ROM_LOAD16_BYTE( "cae_35a.11h",    0x00001, 0x20000, CRC(3ef03083) SHA1(297dfc9ec1e0f07d6083bf5efaa0de8d0fb361fa) )
	ROM_LOAD16_BYTE( "cae_31a.12f",    0x40000, 0x20000, CRC(e5b75caf) SHA1(4d04220c78620867b7598deea5685bbe88298ae6) )
	ROM_LOAD16_BYTE( "cae_36a.12h",    0x40001, 0x20000, CRC(c73fd713) SHA1(fa202c252b2cc5972d42d634c466d89cf8b5d178) )
	ROM_LOAD16_WORD_SWAP( "ca-32m.8h", 0x80000, 0x80000, CRC(0c4837d4) SHA1(1c61958b43066b59d86eb4bae0b52c3109be4b07) )

	ROM_REGION( 0x200000, "gfx", 0 )
	ROMX_LOAD( "ca-5m.7a", 0x000000, 0x80000, CRC(66d4cc37) SHA1(d355ea64ff29d228dcbfeee72bcf11882bf1cd9d) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "ca-7m.9a", 0x000002, 0x80000, CRC(b6f896f2) SHA1(bdb6820b81fbce77d7eacb01777af7c380490402) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "ca-1m.3a", 0x000004, 0x80000, CRC(4d0620fd) SHA1(5f62cd551b6a230edefd81fa60c10c84186ca804) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "ca-3m.5a", 0x000006, 0x80000, CRC(0b0341c3) SHA1(c31f0e78f49d94ea9dea20eb0cbd98a6c613bcbf) , ROM_GROUPWORD | ROM_SKIP(6) )

	ROM_REGION( 0x18000, "audiocpu", 0 ) /* 64k for the audio CPU (+banks) */
	ROM_LOAD( "ca_9.12b",  0x00000, 0x08000, CRC(96fe7485) SHA1(10466889dfc6bc8afd3075385e241a16372efbeb) )
	ROM_CONTINUE(          0x10000, 0x08000 )

	ROM_REGION( 0x40000, "oki", 0 ) /* Samples */
	ROM_LOAD( "ca_18.11c", 0x00000, 0x20000, CRC(4a613a2c) SHA1(06e10644fc60925b85d2ca0888c9fa057bfe996a) )
	ROM_LOAD( "ca_19.12c", 0x20000, 0x20000, CRC(74584493) SHA1(5cfb15f1b9729323707972646313aee8ab3ac4eb) )

	ROM_REGION( 0x0200, "aboardplds", 0 )
	ROM_LOAD( "buf1",         0x0000, 0x0117, CRC(eb122de7) SHA1(b26b5bfe258e3e184f069719f9fd008d6b8f6b9b) )
	ROM_LOAD( "ioa1",         0x0000, 0x0117, CRC(59c7ee3b) SHA1(fbb887c5b4f5cb8df77cec710eaac2985bc482a6) )
	ROM_LOAD( "prg1",         0x0000, 0x0117, CRC(f1129744) SHA1(a5300f301c1a08a7da768f0773fa0fe3f683b237) )
	ROM_LOAD( "rom1",         0x0000, 0x0117, CRC(41dc73b9) SHA1(7d4c9f1693c821fbf84e32dd6ef62ddf14967845) )
	ROM_LOAD( "sou1",         0x0000, 0x0117, CRC(84f4b2fe) SHA1(dcc9e86cc36316fe42eace02d6df75d08bc8bb6d) )

	ROM_REGION( 0x0200, "bboardplds", 0 )
	ROM_LOAD( "ca24b.1a",     0x0000, 0x0117, CRC(76ec0b1c) SHA1(71a7e22613981182fd5b1156f4e495337ab8a172) )
	ROM_LOAD( "iob1.11e",     0x0000, 0x0117, CRC(3abc0700) SHA1(973043aa46ec6d5d1db20dc9d5937005a0f9f6ae) )
ROM_END

/* B-Board 89624B-3 */
ROM_START( cawingr1 )
	ROM_REGION( CODE_SIZE, "maincpu", 0 )      /* 68000 code */
	ROM_LOAD16_BYTE( "cae_30.11f",     0x00000, 0x20000, CRC(23305cd5) SHA1(59cbcb79c171b433f278e128c73cdd3635876370) )
	ROM_LOAD16_BYTE( "cae_35.11h",     0x00001, 0x20000, CRC(69419113) SHA1(cfbb6dbbe224ffaf7747fd70b65a7dbd4f696fe9) )
	ROM_LOAD16_BYTE( "cae_31.12f",     0x40000, 0x20000, CRC(9008dfb3) SHA1(81fdd21606caabe9e0df773fc33377c958ab80f6) )
	ROM_LOAD16_BYTE( "cae_36.12h",     0x40001, 0x20000, CRC(4dbf6f8e) SHA1(a2da49dce72c2366381bd8bea8ce4eba0b70d78c) )
	ROM_LOAD16_WORD_SWAP( "ca-32m.8h", 0x80000, 0x80000, CRC(0c4837d4) SHA1(1c61958b43066b59d86eb4bae0b52c3109be4b07) )

	ROM_REGION( 0x200000, "gfx", 0 )
	ROMX_LOAD( "ca-5m.7a",  0x000000, 0x80000, CRC(66d4cc37) SHA1(d355ea64ff29d228dcbfeee72bcf11882bf1cd9d) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "ca-7m.9a",  0x000002, 0x80000, CRC(b6f896f2) SHA1(bdb6820b81fbce77d7eacb01777af7c380490402) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "ca-1m.3a",  0x000004, 0x80000, CRC(4d0620fd) SHA1(5f62cd551b6a230edefd81fa60c10c84186ca804) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "ca-3m.5a",  0x000006, 0x80000, CRC(0b0341c3) SHA1(c31f0e78f49d94ea9dea20eb0cbd98a6c613bcbf) , ROM_GROUPWORD | ROM_SKIP(6) )

	ROM_REGION( 0x18000, "audiocpu", 0 ) /* 64k for the audio CPU (+banks) */
	ROM_LOAD( "cae_09.12b", 0x00000, 0x08000, CRC(96fe7485) SHA1(10466889dfc6bc8afd3075385e241a16372efbeb) )    // == ca_9.12b
	ROM_CONTINUE(           0x10000, 0x08000 )

	ROM_REGION( 0x40000, "oki", 0 ) /* Samples */
	ROM_LOAD( "cae_18.11c", 0x00000, 0x20000, CRC(4a613a2c) SHA1(06e10644fc60925b85d2ca0888c9fa057bfe996a) )    // == ca_18.11c
	ROM_LOAD( "cae_19.12c", 0x20000, 0x20000, CRC(74584493) SHA1(5cfb15f1b9729323707972646313aee8ab3ac4eb) )    // == ca_19.12c

	ROM_REGION( 0x0200, "aboardplds", 0 )
	ROM_LOAD( "buf1",         0x0000, 0x0117, CRC(eb122de7) SHA1(b26b5bfe258e3e184f069719f9fd008d6b8f6b9b) )
	ROM_LOAD( "ioa1",         0x0000, 0x0117, CRC(59c7ee3b) SHA1(fbb887c5b4f5cb8df77cec710eaac2985bc482a6) )
	ROM_LOAD( "prg1",         0x0000, 0x0117, CRC(f1129744) SHA1(a5300f301c1a08a7da768f0773fa0fe3f683b237) )
	ROM_LOAD( "rom1",         0x0000, 0x0117, CRC(41dc73b9) SHA1(7d4c9f1693c821fbf84e32dd6ef62ddf14967845) )
	ROM_LOAD( "sou1",         0x0000, 0x0117, CRC(84f4b2fe) SHA1(dcc9e86cc36316fe42eace02d6df75d08bc8bb6d) )

	ROM_REGION( 0x0200, "bboardplds", 0 )
	ROM_LOAD( "ca24b.1a",     0x0000, 0x0117, CRC(76ec0b1c) SHA1(71a7e22613981182fd5b1156f4e495337ab8a172) )
	ROM_LOAD( "iob1.11e",     0x0000, 0x0117, CRC(3abc0700) SHA1(973043aa46ec6d5d1db20dc9d5937005a0f9f6ae) )
ROM_END

/* B-Board 89624B-3 */
ROM_START( cawingu )
	ROM_REGION( CODE_SIZE, "maincpu", 0 )      /* 68000 code */
	ROM_LOAD16_BYTE( "cau_30a.11f",    0x00000, 0x20000, CRC(91fceacd) SHA1(4845999a96fee829264346ca399fdd64a8408001) ) // == cae_30a.11f
	ROM_LOAD16_BYTE( "cau_35a.11h",    0x00001, 0x20000, CRC(f090d9b2) SHA1(261dc4ac79507299a7f9a1ad5edb8425345db06c) )
	ROM_LOAD16_BYTE( "cau_31a.12f",    0x40000, 0x20000, CRC(e5b75caf) SHA1(4d04220c78620867b7598deea5685bbe88298ae6) ) // == cae_31a.12f
	ROM_LOAD16_BYTE( "cau_36a.12h",    0x40001, 0x20000, CRC(c73fd713) SHA1(fa202c252b2cc5972d42d634c466d89cf8b5d178) ) // == cae_36a.12h
	ROM_LOAD16_WORD_SWAP( "ca-32m.8h", 0x80000, 0x80000, CRC(0c4837d4) SHA1(1c61958b43066b59d86eb4bae0b52c3109be4b07) )

	ROM_REGION( 0x200000, "gfx", 0 )
	ROMX_LOAD( "ca-5m.7a",  0x000000, 0x80000, CRC(66d4cc37) SHA1(d355ea64ff29d228dcbfeee72bcf11882bf1cd9d) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "ca-7m.9a",  0x000002, 0x80000, CRC(b6f896f2) SHA1(bdb6820b81fbce77d7eacb01777af7c380490402) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "ca-1m.3a",  0x000004, 0x80000, CRC(4d0620fd) SHA1(5f62cd551b6a230edefd81fa60c10c84186ca804) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "ca-3m.5a",  0x000006, 0x80000, CRC(0b0341c3) SHA1(c31f0e78f49d94ea9dea20eb0cbd98a6c613bcbf) , ROM_GROUPWORD | ROM_SKIP(6) )

	ROM_REGION( 0x18000, "audiocpu", 0 ) /* 64k for the audio CPU (+banks) */
	ROM_LOAD( "cau_09.12b", 0x00000, 0x08000, CRC(96fe7485) SHA1(10466889dfc6bc8afd3075385e241a16372efbeb) )    // == ca_9.12b
	ROM_CONTINUE(           0x10000, 0x08000 )

	ROM_REGION( 0x40000, "oki", 0 ) /* Samples */
	ROM_LOAD( "cau_18.11c", 0x00000, 0x20000, CRC(4a613a2c) SHA1(06e10644fc60925b85d2ca0888c9fa057bfe996a) )    // == ca_18.11c
	ROM_LOAD( "cau_19.12c", 0x20000, 0x20000, CRC(74584493) SHA1(5cfb15f1b9729323707972646313aee8ab3ac4eb) )    // == ca_19.12c

	ROM_REGION( 0x0200, "aboardplds", 0 )
	ROM_LOAD( "buf1",         0x0000, 0x0117, CRC(eb122de7) SHA1(b26b5bfe258e3e184f069719f9fd008d6b8f6b9b) )
	ROM_LOAD( "ioa1",         0x0000, 0x0117, CRC(59c7ee3b) SHA1(fbb887c5b4f5cb8df77cec710eaac2985bc482a6) )
	ROM_LOAD( "prg1",         0x0000, 0x0117, CRC(f1129744) SHA1(a5300f301c1a08a7da768f0773fa0fe3f683b237) )
	ROM_LOAD( "rom1",         0x0000, 0x0117, CRC(41dc73b9) SHA1(7d4c9f1693c821fbf84e32dd6ef62ddf14967845) )
	ROM_LOAD( "sou1",         0x0000, 0x0117, CRC(84f4b2fe) SHA1(dcc9e86cc36316fe42eace02d6df75d08bc8bb6d) )

	ROM_REGION( 0x0200, "bboardplds", 0 )
	ROM_LOAD( "ca24b.1a",     0x0000, 0x0117, CRC(76ec0b1c) SHA1(71a7e22613981182fd5b1156f4e495337ab8a172) )
	ROM_LOAD( "iob1.11e",     0x0000, 0x0117, CRC(3abc0700) SHA1(973043aa46ec6d5d1db20dc9d5937005a0f9f6ae) )
ROM_END

/* B-Board 89625B-1 */
ROM_START( cawingj )
	ROM_REGION( CODE_SIZE, "maincpu", 0 )      /* 68000 code */
	ROM_LOAD16_BYTE( "caj_36a.12f", 0x00000, 0x20000, CRC(91fceacd) SHA1(4845999a96fee829264346ca399fdd64a8408001) )    // == cae_30a.11f
	ROM_LOAD16_BYTE( "caj_42a.12h", 0x00001, 0x20000, CRC(039f8362) SHA1(3fc7a642ddeaf94abdfdd5788a4b3c3b1f1b4c5e) )
	ROM_LOAD16_BYTE( "caj_37a.13f", 0x40000, 0x20000, CRC(e5b75caf) SHA1(4d04220c78620867b7598deea5685bbe88298ae6) )    // == cae_31a.12f
	ROM_LOAD16_BYTE( "caj_43a.13h", 0x40001, 0x20000, CRC(c73fd713) SHA1(fa202c252b2cc5972d42d634c466d89cf8b5d178) )    // == cae_36a.12h
	ROM_LOAD16_BYTE( "caj_34.10f",  0x80000, 0x20000, CRC(51ea57f4) SHA1(7d7080dbf4b6f9b801b796937e9c3c45afed602f) )
	ROM_LOAD16_BYTE( "caj_40.10h",  0x80001, 0x20000, CRC(2ab71ae1) SHA1(23814b58322902b23c4bdd744e60d819811462cc) )
	ROM_LOAD16_BYTE( "caj_35.11f",  0xc0000, 0x20000, CRC(01d71973) SHA1(1f5fc0d47f1456a6338284f883dabc89697f8aa5) )
	ROM_LOAD16_BYTE( "caj_41.11h",  0xc0001, 0x20000, CRC(3a43b538) SHA1(474a701500632cbd395ae404ede1d10a9969b342) )

	ROM_REGION( 0x200000, "gfx", 0 )
	ROMX_LOAD( "caj_09.4b",  0x000000, 0x20000, CRC(41b0f9a6) SHA1(5a59df64d0c665d5d479ef2d9e7ec191ca0e7a92) , ROM_SKIP(7) )
	ROMX_LOAD( "caj_01.4a",  0x000001, 0x20000, CRC(1002d0b8) SHA1(896bec683c1164c6f1fa1d81cadb8a1c549d4a4e) , ROM_SKIP(7) )
	ROMX_LOAD( "caj_13.9b",  0x000002, 0x20000, CRC(6f3948b2) SHA1(942d37b84c727074941316b042679110594ae249) , ROM_SKIP(7) )
	ROMX_LOAD( "caj_05.9a",  0x000003, 0x20000, CRC(207373d7) SHA1(4fa67c847f65e2657900f4fc93f1d8a7b95c12e6) , ROM_SKIP(7) )
	ROMX_LOAD( "caj_24.5e",  0x000004, 0x20000, CRC(e356aad7) SHA1(55f1489044e70a57ad15e1f2c20567bd6c770f71) , ROM_SKIP(7) )
	ROMX_LOAD( "caj_17.5c",  0x000005, 0x20000, CRC(540f2fd8) SHA1(4f500c7795aa41d472c59d2594fc84f6b17ed137) , ROM_SKIP(7) )
	ROMX_LOAD( "caj_38.8h",  0x000006, 0x20000, CRC(2464d4ab) SHA1(092864551e7c1c6adbeb901a556f650ccf6ca2f4) , ROM_SKIP(7) )
	ROMX_LOAD( "caj_32.8f",  0x000007, 0x20000, CRC(9b5836b3) SHA1(3fda5409d99104f355fa42ec413ccb799d1506c9) , ROM_SKIP(7) )
	ROMX_LOAD( "caj_10.5b",  0x100000, 0x20000, CRC(bf8a5f52) SHA1(8cdd31a58de560d282e708c57cda0fefa7d6c92f) , ROM_SKIP(7) )
	ROMX_LOAD( "caj_02.5a",  0x100001, 0x20000, CRC(125b018d) SHA1(edb3271f668e0328efd59e0929ee86efd5aa7b1f) , ROM_SKIP(7) )
	ROMX_LOAD( "caj_14.10b", 0x100002, 0x20000, CRC(8458e7d7) SHA1(7ebe0a3597a7d8482c2d33640ed1b994fd3a02b2) , ROM_SKIP(7) )
	ROMX_LOAD( "caj_06.10a", 0x100003, 0x20000, CRC(cf80e164) SHA1(d83573947fec01a9814919df719474aa3e6ae9a0) , ROM_SKIP(7) )
	ROMX_LOAD( "caj_25.7e",  0x100004, 0x20000, CRC(cdd0204d) SHA1(7cbc129bc148718f8c36e27f05583cdecc57b63e) , ROM_SKIP(7) )
	ROMX_LOAD( "caj_18.7c",  0x100005, 0x20000, CRC(29c1d4b1) SHA1(d0109ab2f521786a64548910947ca24976ec1218) , ROM_SKIP(7) )
	ROMX_LOAD( "caj_39.9h",  0x100006, 0x20000, CRC(eea23b67) SHA1(79d35c15b4b4430d90cd6c270cdd3a064bc2e1a3) , ROM_SKIP(7) )
	ROMX_LOAD( "caj_33.9f",  0x100007, 0x20000, CRC(dde3891f) SHA1(25b8069a9c8615323b94157b1ce39805559b68f4) , ROM_SKIP(7) )

	ROM_REGION( 0x18000, "audiocpu", 0 ) /* 64k for the audio CPU (+banks) */
	ROM_LOAD( "caj_23.13b",  0x00000, 0x08000, CRC(96fe7485) SHA1(10466889dfc6bc8afd3075385e241a16372efbeb) )   // == ca_9.12b
	ROM_CONTINUE(            0x10000, 0x08000 )

	ROM_REGION( 0x40000, "oki", 0 ) /* Samples */
	ROM_LOAD( "caj_30.12c",  0x00000, 0x20000, CRC(4a613a2c) SHA1(06e10644fc60925b85d2ca0888c9fa057bfe996a) )   // == ca_18.11c
	ROM_LOAD( "caj_31.13c",  0x20000, 0x20000, CRC(74584493) SHA1(5cfb15f1b9729323707972646313aee8ab3ac4eb) )   // == ca_19.12c

	ROM_REGION( 0x0200, "aboardplds", 0 )
	ROM_LOAD( "buf1",         0x0000, 0x0117, CRC(eb122de7) SHA1(b26b5bfe258e3e184f069719f9fd008d6b8f6b9b) )
	ROM_LOAD( "ioa1",         0x0000, 0x0117, CRC(59c7ee3b) SHA1(fbb887c5b4f5cb8df77cec710eaac2985bc482a6) )
	ROM_LOAD( "prg1",         0x0000, 0x0117, CRC(f1129744) SHA1(a5300f301c1a08a7da768f0773fa0fe3f683b237) )
	ROM_LOAD( "rom1",         0x0000, 0x0117, CRC(41dc73b9) SHA1(7d4c9f1693c821fbf84e32dd6ef62ddf14967845) )
	ROM_LOAD( "sou1",         0x0000, 0x0117, CRC(84f4b2fe) SHA1(dcc9e86cc36316fe42eace02d6df75d08bc8bb6d) )

	ROM_REGION( 0x0200, "bboardplds", 0 )
	ROM_LOAD( "ca22b.1a",     0x0000, 0x0117, CRC(5152e678) SHA1(ac61df30cd073b26f2145e3ea0c513ec804d047a) )
	ROM_LOAD( "iob1.12e",     0x0000, 0x0117, CRC(3abc0700) SHA1(973043aa46ec6d5d1db20dc9d5937005a0f9f6ae) )    /* seen the same pcb with LWIO.12E */
ROM_END

/* B-Board 89624B-3 */
ROM_START( nemo )
	ROM_REGION( CODE_SIZE, "maincpu", 0 )      /* 68000 code */
	ROM_LOAD16_BYTE( "nme_30a.11f",    0x00000, 0x20000, CRC(d2c03e56) SHA1(df468e3b5deba01a6825b742f1cc87bfb26c1981) )
	ROM_LOAD16_BYTE( "nme_35a.11h",    0x00001, 0x20000, CRC(5fd31661) SHA1(12f92a7255e8cae6975452db956670cf72d51768) )
	ROM_LOAD16_BYTE( "nme_31a.12f",    0x40000, 0x20000, CRC(b2bd4f6f) SHA1(82a59b5f36cb4c23dca05297e2a643842fc12609) )
	ROM_LOAD16_BYTE( "nme_36a.12h",    0x40001, 0x20000, CRC(ee9450e3) SHA1(a5454268ef58533e71fe07167b4c3fd263363f77) )
	ROM_LOAD16_WORD_SWAP( "nm-32m.8h", 0x80000, 0x80000, CRC(d6d1add3) SHA1(61c3013d322dbb7622cca032adcd020ba318e885) )

	ROM_REGION( 0x200000, "gfx", 0 )
	ROMX_LOAD( "nm-5m.7a",  0x000000, 0x80000, CRC(487b8747) SHA1(f14339b02b8f7ec2002632349e88fed4afc30050) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "nm-7m.9a",  0x000002, 0x80000, CRC(203dc8c6) SHA1(d52577500e822b89904d1510d559f8575c2aaa78) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "nm-1m.3a",  0x000004, 0x80000, CRC(9e878024) SHA1(9a5ce3a6a7952a8954d0709b9473db9253793d70) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "nm-3m.5a",  0x000006, 0x80000, CRC(bb01e6b6) SHA1(3883e28f721d0278b2f4f877a804e95ee14f53e4) , ROM_GROUPWORD | ROM_SKIP(6) )

	ROM_REGION( 0x18000, "audiocpu", 0 ) /* 64k for the audio CPU (+banks) */
	ROM_LOAD( "nme_09.12b", 0x00000, 0x08000, CRC(0f4b0581) SHA1(2e5a2885149c632abfaf4292a1bf032c13c8da6c) )
	ROM_CONTINUE(           0x10000, 0x08000 )

	ROM_REGION( 0x40000, "oki", 0 ) /* Samples */
	ROM_LOAD( "nme_18.11c", 0x00000, 0x20000, CRC(bab333d4) SHA1(c1d0fb61ec46f17eb7edf69e1ad5ac91b5d51daa) )
	ROM_LOAD( "nme_19.12c", 0x20000, 0x20000, CRC(2650a0a8) SHA1(e9e8cc1b27a2cb3e87124061fabcf42982f0611f) )

	ROM_REGION( 0x0200, "aboardplds", 0 )
	ROM_LOAD( "buf1",         0x0000, 0x0117, CRC(eb122de7) SHA1(b26b5bfe258e3e184f069719f9fd008d6b8f6b9b) )
	ROM_LOAD( "ioa1",         0x0000, 0x0117, CRC(59c7ee3b) SHA1(fbb887c5b4f5cb8df77cec710eaac2985bc482a6) )
	ROM_LOAD( "prg1",         0x0000, 0x0117, CRC(f1129744) SHA1(a5300f301c1a08a7da768f0773fa0fe3f683b237) )
	ROM_LOAD( "rom1",         0x0000, 0x0117, CRC(41dc73b9) SHA1(7d4c9f1693c821fbf84e32dd6ef62ddf14967845) )
	ROM_LOAD( "sou1",         0x0000, 0x0117, CRC(84f4b2fe) SHA1(dcc9e86cc36316fe42eace02d6df75d08bc8bb6d) )

	ROM_REGION( 0x0200, "bboardplds", 0 )
	ROM_LOAD( "nm24b.1a",     0x0000, 0x0117, CRC(7b25bac6) SHA1(fa0083c59c8d6da07798cb3a4fc25d388065b7cd) )
	ROM_LOAD( "iob1.11e",     0x0000, 0x0117, CRC(3abc0700) SHA1(973043aa46ec6d5d1db20dc9d5937005a0f9f6ae) )
ROM_END

/* B-Board 89624B-3 */
ROM_START( nemor1 )
	ROM_REGION( CODE_SIZE, "maincpu", 0 )      /* 68000 code */
	ROM_LOAD16_BYTE( "nme_30.11f",     0x00000, 0x20000, CRC(71b333db) SHA1(8bd8b62d8fef2ad9d2788ccf46e89da953e5817c) )
	ROM_LOAD16_BYTE( "nme_35.11h",     0x00001, 0x20000, CRC(d153bc18) SHA1(2b4a9a50081d403a6485b7311e179a31b659b74d) )
	ROM_LOAD16_BYTE( "nme_31.12f",     0x40000, 0x20000, CRC(7e83dbd2) SHA1(d4c7e3786faab4dd9ded3c1a8f6fea114d423e64) )
	ROM_LOAD16_BYTE( "nme_36.12h",     0x40001, 0x20000, CRC(6aeeec81) SHA1(d7dce52d0f2225d844d852d2c3e81a48cb53a43d) )
	ROM_LOAD16_WORD_SWAP( "nm-32m.8h", 0x80000, 0x80000, CRC(d6d1add3) SHA1(61c3013d322dbb7622cca032adcd020ba318e885) )

	ROM_REGION( 0x200000, "gfx", 0 )
	ROMX_LOAD( "nm-5m.7a",  0x000000, 0x80000, CRC(487b8747) SHA1(f14339b02b8f7ec2002632349e88fed4afc30050) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "nm-7m.9a",  0x000002, 0x80000, CRC(203dc8c6) SHA1(d52577500e822b89904d1510d559f8575c2aaa78) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "nm-1m.3a",  0x000004, 0x80000, CRC(9e878024) SHA1(9a5ce3a6a7952a8954d0709b9473db9253793d70) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "nm-3m.5a",  0x000006, 0x80000, CRC(bb01e6b6) SHA1(3883e28f721d0278b2f4f877a804e95ee14f53e4) , ROM_GROUPWORD | ROM_SKIP(6) )

	ROM_REGION( 0x18000, "audiocpu", 0 ) /* 64k for the audio CPU (+banks) */
	ROM_LOAD( "nme_09.12b", 0x00000, 0x08000, CRC(0f4b0581) SHA1(2e5a2885149c632abfaf4292a1bf032c13c8da6c) )
	ROM_CONTINUE(           0x10000, 0x08000 )

	ROM_REGION( 0x40000, "oki", 0 ) /* Samples */
	ROM_LOAD( "nme_18.11c", 0x00000, 0x20000, CRC(bab333d4) SHA1(c1d0fb61ec46f17eb7edf69e1ad5ac91b5d51daa) )
	ROM_LOAD( "nme_19.12c", 0x20000, 0x20000, CRC(2650a0a8) SHA1(e9e8cc1b27a2cb3e87124061fabcf42982f0611f) )

	ROM_REGION( 0x0200, "aboardplds", 0 )
	ROM_LOAD( "buf1",         0x0000, 0x0117, CRC(eb122de7) SHA1(b26b5bfe258e3e184f069719f9fd008d6b8f6b9b) )
	ROM_LOAD( "ioa1",         0x0000, 0x0117, CRC(59c7ee3b) SHA1(fbb887c5b4f5cb8df77cec710eaac2985bc482a6) )
	ROM_LOAD( "prg1",         0x0000, 0x0117, CRC(f1129744) SHA1(a5300f301c1a08a7da768f0773fa0fe3f683b237) )
	ROM_LOAD( "rom1",         0x0000, 0x0117, CRC(41dc73b9) SHA1(7d4c9f1693c821fbf84e32dd6ef62ddf14967845) )
	ROM_LOAD( "sou1",         0x0000, 0x0117, CRC(84f4b2fe) SHA1(dcc9e86cc36316fe42eace02d6df75d08bc8bb6d) )

	ROM_REGION( 0x0200, "bboardplds", 0 )
	ROM_LOAD( "nm24b.1a",     0x0000, 0x0117, CRC(7b25bac6) SHA1(fa0083c59c8d6da07798cb3a4fc25d388065b7cd) )
	ROM_LOAD( "iob1.11e",     0x0000, 0x0117, CRC(3abc0700) SHA1(973043aa46ec6d5d1db20dc9d5937005a0f9f6ae) )
ROM_END

/* B-Board 89625B-? */
ROM_START( nemoj )
	ROM_REGION( CODE_SIZE, "maincpu", 0 )      /* 68000 code */
	ROM_LOAD16_BYTE( "nmj_36a.12f", 0x00000, 0x20000, CRC(daeceabb) SHA1(ebd44922be9d07a3d3411af52edee8a60cb11dad) )
	ROM_LOAD16_BYTE( "nmj_42a.12h", 0x00001, 0x20000, CRC(55024740) SHA1(4bacbd191bb435de5dc548ac7fa16ed286bd2d3b) )
	ROM_LOAD16_BYTE( "nmj_37a.13f", 0x40000, 0x20000, CRC(619068b6) SHA1(2507c6f77a06a80f913c848dcb6816bcbf4bba8a) )
	ROM_LOAD16_BYTE( "nmj_43a.13h", 0x40001, 0x20000, CRC(a948a53b) SHA1(65c2abf321cf8b171bbfbb51ed57bc99eb552ca9) )
	ROM_LOAD16_BYTE( "nm_34.10f",   0x80000, 0x20000, CRC(5737feed) SHA1(2635715cc21381e9f0a4ae4227eb5896886ee3e2) )    // == nm-32m.8h
	ROM_LOAD16_BYTE( "nm_40.10h",   0x80001, 0x20000, CRC(8a4099f3) SHA1(d1af73d8992aa9ef6dcd729675a2fbea8c290311) )    // == nm-32m.8h
	ROM_LOAD16_BYTE( "nm_35.11f",   0xc0000, 0x20000, CRC(bd11a7f8) SHA1(1c09db7cbd132866d4f08720cdd60707069f8580) )    // == nm-32m.8h
	ROM_LOAD16_BYTE( "nm_41.11h",   0xc0001, 0x20000, CRC(6309603d) SHA1(51bee785ddb87340ad56960ad816c0513bc93eb8) )    // == nm-32m.8h

	ROM_REGION( 0x200000, "gfx", 0 )
	ROMX_LOAD( "nm_09.4b",  0x000000, 0x20000, CRC(9d60d286) SHA1(69ea9d584d735c3629e1c017cbd966df264e4324) , ROM_SKIP(7) ) // == nm-5m.7a
	ROMX_LOAD( "nm_01.4a",  0x000001, 0x20000, CRC(8a83f7c4) SHA1(ef24f47517d22dfecd3e6b5ef76e38073f6b81ba) , ROM_SKIP(7) ) // == nm-5m.7a
	ROMX_LOAD( "nm_13.9b",  0x000002, 0x20000, CRC(a4909fe0) SHA1(6b1a0e8c2bd2979f7fcc363a86de28d3e365709c) , ROM_SKIP(7) ) // == nm-7m.9a
	ROMX_LOAD( "nm_05.9a",  0x000003, 0x20000, CRC(16db1e61) SHA1(76ff93edd4d40a2527744585a87052a3fc0c77f6) , ROM_SKIP(7) ) // == nm-7m.9a
	ROMX_LOAD( "nm_24.5e",  0x000004, 0x20000, CRC(3312c648) SHA1(9e4f584fa360de16d42d65619dbe9426d4322c00) , ROM_SKIP(7) ) // == nm-1m.3a
	ROMX_LOAD( "nm_17.5c",  0x000005, 0x20000, CRC(ccfc50e2) SHA1(a238f050d11e925b1c0037bb860289ab746b7039) , ROM_SKIP(7) ) // == nm-1m.3a
	ROMX_LOAD( "nm_38.8h",  0x000006, 0x20000, CRC(ae98a997) SHA1(d2d499395e43aa85d9098966d04fde6bd055900b) , ROM_SKIP(7) ) // == nm-3m.5a
	ROMX_LOAD( "nm_32.8f",  0x000007, 0x20000, CRC(b3704dde) SHA1(d107fecb45f34e877faabffcdaba437935754906) , ROM_SKIP(7) ) // == nm-3m.5a
	ROMX_LOAD( "nm_10.5b",  0x100000, 0x20000, CRC(33c1388c) SHA1(cd1ec3e8d6d2b5a65648c749426ec4e254f93d8c) , ROM_SKIP(7) ) // == nm-5m.7a
	ROMX_LOAD( "nm_02.5a",  0x100001, 0x20000, CRC(84c69469) SHA1(700cf7be644056b1dbc5d8bed37caf6383a81cfe) , ROM_SKIP(7) ) // == nm-5m.7a
	ROMX_LOAD( "nm_14.10b", 0x100002, 0x20000, CRC(66612270) SHA1(0c996571459ac44d5ca5683bdcb6a6f08dd83480) , ROM_SKIP(7) ) // == nm-7m.9a
	ROMX_LOAD( "nm_06.10a", 0x100003, 0x20000, CRC(8b9bcf95) SHA1(e03c6dc4946a37bdab68d929722b1e10a2aca31a) , ROM_SKIP(7) ) // == nm-7m.9a
	ROMX_LOAD( "nm_25.7e",  0x100004, 0x20000, CRC(acfc84d2) SHA1(4cd9f3bc32ef62cb3b414de68db34f950d10f406) , ROM_SKIP(7) ) // == nm-1m.3a
	ROMX_LOAD( "nm_18.7c",  0x100005, 0x20000, CRC(4347deed) SHA1(fdd9b3f1ddad42464dcc7298e5b740ffe1622343) , ROM_SKIP(7) ) // == nm-1m.3a
	ROMX_LOAD( "nm_39.9h",  0x100006, 0x20000, CRC(6a274ecd) SHA1(66259fd6e71cfdb618c189b7f18749a996aacfdf) , ROM_SKIP(7) ) // == nm-3m.5a
	ROMX_LOAD( "nm_33.9f",  0x100007, 0x20000, CRC(c469dc74) SHA1(d06956eef5f9b31779f218d597a1a504c1e16bad) , ROM_SKIP(7) ) // == nm-3m.5a

	ROM_REGION( 0x18000, "audiocpu", 0 ) /* 64k for the audio CPU (+banks) */
	ROM_LOAD( "nm_23.13b",  0x00000, 0x08000, CRC(8d3c5a42) SHA1(cc7477da80f3d08cf014379318e39cb75b5d3205) )    /* 1 byte different from nemo, pcb verified */
	ROM_CONTINUE(           0x10000, 0x08000 )

	ROM_REGION( 0x40000, "oki", 0 ) /* Samples */
	ROM_LOAD( "nm_30.12c",  0x00000, 0x20000, CRC(bab333d4) SHA1(c1d0fb61ec46f17eb7edf69e1ad5ac91b5d51daa) )    // == nme_18.11c
	ROM_LOAD( "nm_31.13c",  0x20000, 0x20000, CRC(2650a0a8) SHA1(e9e8cc1b27a2cb3e87124061fabcf42982f0611f) )    // == nme_19.12c

	ROM_REGION( 0x0200, "aboardplds", 0 )
	ROM_LOAD( "buf1",         0x0000, 0x0117, CRC(eb122de7) SHA1(b26b5bfe258e3e184f069719f9fd008d6b8f6b9b) )
	ROM_LOAD( "ioa1",         0x0000, 0x0117, CRC(59c7ee3b) SHA1(fbb887c5b4f5cb8df77cec710eaac2985bc482a6) )
	ROM_LOAD( "prg1",         0x0000, 0x0117, CRC(f1129744) SHA1(a5300f301c1a08a7da768f0773fa0fe3f683b237) )
	ROM_LOAD( "rom1",         0x0000, 0x0117, CRC(41dc73b9) SHA1(7d4c9f1693c821fbf84e32dd6ef62ddf14967845) )
	ROM_LOAD( "sou1",         0x0000, 0x0117, CRC(84f4b2fe) SHA1(dcc9e86cc36316fe42eace02d6df75d08bc8bb6d) )

	ROM_REGION( 0x0200, "bboardplds", 0 )
	ROM_LOAD( "nm22b.1a",     0x0000, 0x0117, NO_DUMP )
	ROM_LOAD( "iob1.12e",     0x0000, 0x0117, CRC(3abc0700) SHA1(973043aa46ec6d5d1db20dc9d5937005a0f9f6ae) )    // pal verification required
ROM_END

/* B-Board 90629B-3 */
ROM_START( sf2 )
	ROM_REGION( CODE_SIZE, "maincpu", 0 )      /* 68000 code */
	ROM_LOAD16_BYTE( "sf2e_30g.11e", 0x00000, 0x20000, CRC(fe39ee33) SHA1(22558eb15e035b09b80935a32b8425d91cd79669) )
	ROM_LOAD16_BYTE( "sf2e_37g.11f", 0x00001, 0x20000, CRC(fb92cd74) SHA1(bf1ccfe7cc1133f0f65556430311108722add1f2) )
	ROM_LOAD16_BYTE( "sf2e_31g.12e", 0x40000, 0x20000, CRC(69a0a301) SHA1(86a3954335310865b14ce8b4e0e4499feb14fc12) )
	ROM_LOAD16_BYTE( "sf2e_38g.12f", 0x40001, 0x20000, CRC(5e22db70) SHA1(6565946591a18eaf46f04c1aa449ee0ae9ac2901) )
	ROM_LOAD16_BYTE( "sf2e_28g.9e",  0x80000, 0x20000, CRC(8bf9f1e5) SHA1(bbcef63f35e5bff3f373968ba1278dd6bd86b593) )
	ROM_LOAD16_BYTE( "sf2e_35g.9f",  0x80001, 0x20000, CRC(626ef934) SHA1(507bda3e4519de237aca919cf72e543403ec9724) )
	ROM_LOAD16_BYTE( "sf2_29b.10e",  0xc0000, 0x20000, CRC(bb4af315) SHA1(75f0827f4f7e9f292add46467f8d4fe19b2514c9) )
	ROM_LOAD16_BYTE( "sf2_36b.10f",  0xc0001, 0x20000, CRC(c02a13eb) SHA1(b807cc495bff3f95d03b061fc629c95f965cb6d8) )

	ROM_REGION( 0x600000, "gfx", 0 )
	ROMX_LOAD( "sf2-5m.4a",  0x000000, 0x80000, CRC(22c9cc8e) SHA1(b9194fb337b30502c1c9501cd6c64ae4035544d4) , ROM_GROUPWORD | ROM_SKIP(6) )    // in "2" socket
	ROMX_LOAD( "sf2-7m.6a",  0x000002, 0x80000, CRC(57213be8) SHA1(3759b851ac0904ec79cbb67a2264d384b6f2f9f9) , ROM_GROUPWORD | ROM_SKIP(6) )    // in "4" socket
	ROMX_LOAD( "sf2-1m.3a",  0x000004, 0x80000, CRC(ba529b4f) SHA1(520840d727161cf09ca784919fa37bc9b54cc3ce) , ROM_GROUPWORD | ROM_SKIP(6) )    // in "1" socket
	ROMX_LOAD( "sf2-3m.5a",  0x000006, 0x80000, CRC(4b1b33a8) SHA1(2360cff890551f76775739e2d6563858bff80e41) , ROM_GROUPWORD | ROM_SKIP(6) )    // in "3" socket
	ROMX_LOAD( "sf2-6m.4c",  0x200000, 0x80000, CRC(2c7e2229) SHA1(357c2275af9133fd0bd6fbb1fa9ad5e0b490b3a2) , ROM_GROUPWORD | ROM_SKIP(6) )    // in "11" socket
	ROMX_LOAD( "sf2-8m.6c",  0x200002, 0x80000, CRC(b5548f17) SHA1(baa92b91cf616bc9e2a8a66adc777ffbf962a51b) , ROM_GROUPWORD | ROM_SKIP(6) )    // in "13" socket
	ROMX_LOAD( "sf2-2m.3c",  0x200004, 0x80000, CRC(14b84312) SHA1(2eea16673e60ba7a10bd4d8f6c217bb2441a5b0e) , ROM_GROUPWORD | ROM_SKIP(6) )    // in "10" socket
	ROMX_LOAD( "sf2-4m.5c",  0x200006, 0x80000, CRC(5e9cd89a) SHA1(f787aab98668d4c2c54fc4ba677c0cb808e4f31e) , ROM_GROUPWORD | ROM_SKIP(6) )    // in "12" socket
	ROMX_LOAD( "sf2-13m.4d", 0x400000, 0x80000, CRC(994bfa58) SHA1(5669b845f624b10e7be56bfc89b76592258ce48b) , ROM_GROUPWORD | ROM_SKIP(6) )    // in "21" socket
	ROMX_LOAD( "sf2-15m.6d", 0x400002, 0x80000, CRC(3e66ad9d) SHA1(9af9df0826988872662753e9717c48d46f2974b0) , ROM_GROUPWORD | ROM_SKIP(6) )    // in "23" socket
	ROMX_LOAD( "sf2-9m.3d",  0x400004, 0x80000, CRC(c1befaa8) SHA1(a6a7f4725e52678cbd8d557285c01cdccb2c2602) , ROM_GROUPWORD | ROM_SKIP(6) )    // in "20" socket
	ROMX_LOAD( "sf2-11m.5d", 0x400006, 0x80000, CRC(0627c831) SHA1(f9a92d614e8877d648449de2612fc8b43c85e4c2) , ROM_GROUPWORD | ROM_SKIP(6) )    // in "22" socket

	ROM_REGION( 0x18000, "audiocpu", 0 ) /* 64k for the audio CPU (+banks) */
	ROM_LOAD( "sf2_9.12a",   0x00000, 0x08000, CRC(a4823a1b) SHA1(7b6bf59dfd578bfbbdb64c27988796783442d659) )
	ROM_CONTINUE(            0x10000, 0x08000 )

	ROM_REGION( 0x40000, "oki", 0 ) /* Samples */
	ROM_LOAD( "sf2_18.11c",  0x00000, 0x20000, CRC(7f162009) SHA1(346bf42992b4c36c593e21901e22c87ae4a7d86d) )
	ROM_LOAD( "sf2_19.12c",  0x20000, 0x20000, CRC(beade53f) SHA1(277c397dc12752719ec6b47d2224750bd1c07f79) )

	ROM_REGION( 0x0200, "aboardplds", 0 )
	ROM_LOAD( "buf1",         0x0000, 0x0117, CRC(eb122de7) SHA1(b26b5bfe258e3e184f069719f9fd008d6b8f6b9b) )
	ROM_LOAD( "ioa1",         0x0000, 0x0117, CRC(59c7ee3b) SHA1(fbb887c5b4f5cb8df77cec710eaac2985bc482a6) )
	ROM_LOAD( "prg1",         0x0000, 0x0117, CRC(f1129744) SHA1(a5300f301c1a08a7da768f0773fa0fe3f683b237) )
	ROM_LOAD( "rom1",         0x0000, 0x0117, CRC(41dc73b9) SHA1(7d4c9f1693c821fbf84e32dd6ef62ddf14967845) )
	ROM_LOAD( "sou1",         0x0000, 0x0117, CRC(84f4b2fe) SHA1(dcc9e86cc36316fe42eace02d6df75d08bc8bb6d) )

	ROM_REGION( 0x0200, "bboardplds", 0 )
	ROM_LOAD( "stf29.1a",     0x0000, 0x0117, CRC(043309c5) SHA1(f6b9610c9f896c495041ba56a654776a521c356b) )
	ROM_LOAD( "iob1.11d",     0x0000, 0x0117, CRC(3abc0700) SHA1(973043aa46ec6d5d1db20dc9d5937005a0f9f6ae) )

	ROM_REGION( 0x0200, "cboardplds", 0 )
	ROM_LOAD( "c632.ic1",     0x0000, 0x0117, CRC(0fbd9270) SHA1(d7e737b20c44d41e29ca94be56114b31934dde81) )
ROM_END

/* B-Board 90629B-2 */
ROM_START( sf2eb )
	ROM_REGION( CODE_SIZE, "maincpu", 0 )      /* 68000 code */
	ROM_LOAD16_BYTE( "sf2e_30b.11e", 0x00000, 0x20000, CRC(57bd7051) SHA1(5e211e75b1649b07723cabc03cf15636dbbae595) )
	ROM_LOAD16_BYTE( "sf2e_37b.11f", 0x00001, 0x20000, CRC(62691cdd) SHA1(328703c3e737ada544e67c36119eeb4a100ca740) )
	ROM_LOAD16_BYTE( "sf2e_31b.12e", 0x40000, 0x20000, CRC(a673143d) SHA1(e565f0ec23d6deb543c72af5a83f070c07319477) )
	ROM_LOAD16_BYTE( "sf2e_38b.12f", 0x40001, 0x20000, CRC(4c2ccef7) SHA1(77b119c70c255622b023de25d9af3b3aac52ea47) )
	ROM_LOAD16_BYTE( "sf2_28b.9e",   0x80000, 0x20000, CRC(4009955e) SHA1(7842dbef7650485639fbae49b9f4db7494d4f73d) )
	ROM_LOAD16_BYTE( "sf2_35b.9f",   0x80001, 0x20000, CRC(8c1f3994) SHA1(5e1d334399d05a837c2d80f79eada543e83afaf7) )
	ROM_LOAD16_BYTE( "sf2_29b.10e",  0xc0000, 0x20000, CRC(bb4af315) SHA1(75f0827f4f7e9f292add46467f8d4fe19b2514c9) )
	ROM_LOAD16_BYTE( "sf2_36b.10f",  0xc0001, 0x20000, CRC(c02a13eb) SHA1(b807cc495bff3f95d03b061fc629c95f965cb6d8) )

	ROM_REGION( 0x600000, "gfx", 0 )
	ROMX_LOAD( "sf2-5m.4a",  0x000000, 0x80000, CRC(22c9cc8e) SHA1(b9194fb337b30502c1c9501cd6c64ae4035544d4) , ROM_GROUPWORD | ROM_SKIP(6) )    // in "2" socket
	ROMX_LOAD( "sf2-7m.6a",  0x000002, 0x80000, CRC(57213be8) SHA1(3759b851ac0904ec79cbb67a2264d384b6f2f9f9) , ROM_GROUPWORD | ROM_SKIP(6) )    // in "4" socket
	ROMX_LOAD( "sf2-1m.3a",  0x000004, 0x80000, CRC(ba529b4f) SHA1(520840d727161cf09ca784919fa37bc9b54cc3ce) , ROM_GROUPWORD | ROM_SKIP(6) )    // in "1" socket
	ROMX_LOAD( "sf2-3m.5a",  0x000006, 0x80000, CRC(4b1b33a8) SHA1(2360cff890551f76775739e2d6563858bff80e41) , ROM_GROUPWORD | ROM_SKIP(6) )    // in "3" socket
	ROMX_LOAD( "sf2-6m.4c",  0x200000, 0x80000, CRC(2c7e2229) SHA1(357c2275af9133fd0bd6fbb1fa9ad5e0b490b3a2) , ROM_GROUPWORD | ROM_SKIP(6) )    // in "11" socket
	ROMX_LOAD( "sf2-8m.6c",  0x200002, 0x80000, CRC(b5548f17) SHA1(baa92b91cf616bc9e2a8a66adc777ffbf962a51b) , ROM_GROUPWORD | ROM_SKIP(6) )    // in "13" socket
	ROMX_LOAD( "sf2-2m.3c",  0x200004, 0x80000, CRC(14b84312) SHA1(2eea16673e60ba7a10bd4d8f6c217bb2441a5b0e) , ROM_GROUPWORD | ROM_SKIP(6) )    // in "10" socket
	ROMX_LOAD( "sf2-4m.5c",  0x200006, 0x80000, CRC(5e9cd89a) SHA1(f787aab98668d4c2c54fc4ba677c0cb808e4f31e) , ROM_GROUPWORD | ROM_SKIP(6) )    // in "12" socket
	ROMX_LOAD( "sf2-13m.4d", 0x400000, 0x80000, CRC(994bfa58) SHA1(5669b845f624b10e7be56bfc89b76592258ce48b) , ROM_GROUPWORD | ROM_SKIP(6) )    // in "21" socket
	ROMX_LOAD( "sf2-15m.6d", 0x400002, 0x80000, CRC(3e66ad9d) SHA1(9af9df0826988872662753e9717c48d46f2974b0) , ROM_GROUPWORD | ROM_SKIP(6) )    // in "23" socket
	ROMX_LOAD( "sf2-9m.3d",  0x400004, 0x80000, CRC(c1befaa8) SHA1(a6a7f4725e52678cbd8d557285c01cdccb2c2602) , ROM_GROUPWORD | ROM_SKIP(6) )    // in "20" socket
	ROMX_LOAD( "sf2-11m.5d", 0x400006, 0x80000, CRC(0627c831) SHA1(f9a92d614e8877d648449de2612fc8b43c85e4c2) , ROM_GROUPWORD | ROM_SKIP(6) )    // in "22" socket

	ROM_REGION( 0x18000, "audiocpu", 0 ) /* 64k for the audio CPU (+banks) */
	ROM_LOAD( "sf2_9.12a",   0x00000, 0x08000, CRC(a4823a1b) SHA1(7b6bf59dfd578bfbbdb64c27988796783442d659) )
	ROM_CONTINUE(            0x10000, 0x08000 )

	ROM_REGION( 0x40000, "oki", 0 ) /* Samples */
	ROM_LOAD( "sf2_18.11c",  0x00000, 0x20000, CRC(7f162009) SHA1(346bf42992b4c36c593e21901e22c87ae4a7d86d) )
	ROM_LOAD( "sf2_19.12c",  0x20000, 0x20000, CRC(beade53f) SHA1(277c397dc12752719ec6b47d2224750bd1c07f79) )

	ROM_REGION( 0x0200, "aboardplds", 0 )
	ROM_LOAD( "buf1",         0x0000, 0x0117, CRC(eb122de7) SHA1(b26b5bfe258e3e184f069719f9fd008d6b8f6b9b) )
	ROM_LOAD( "ioa1",         0x0000, 0x0117, CRC(59c7ee3b) SHA1(fbb887c5b4f5cb8df77cec710eaac2985bc482a6) )
	ROM_LOAD( "prg1",         0x0000, 0x0117, CRC(f1129744) SHA1(a5300f301c1a08a7da768f0773fa0fe3f683b237) )
	ROM_LOAD( "rom1",         0x0000, 0x0117, CRC(41dc73b9) SHA1(7d4c9f1693c821fbf84e32dd6ef62ddf14967845) )
	ROM_LOAD( "sou1",         0x0000, 0x0117, CRC(84f4b2fe) SHA1(dcc9e86cc36316fe42eace02d6df75d08bc8bb6d) )

	ROM_REGION( 0x0200, "bboardplds", 0 )
	ROM_LOAD( "stf29.1a",     0x0000, 0x0117, CRC(043309c5) SHA1(f6b9610c9f896c495041ba56a654776a521c356b) )
	ROM_LOAD( "iob1.11d",     0x0000, 0x0117, CRC(3abc0700) SHA1(973043aa46ec6d5d1db20dc9d5937005a0f9f6ae) )

	ROM_REGION( 0x0200, "cboardplds", 0 )
	ROM_LOAD( "c632.ic1",     0x0000, 0x0117, CRC(0fbd9270) SHA1(d7e737b20c44d41e29ca94be56114b31934dde81) )
ROM_END

/* B-Board 90629B-3 */
ROM_START( sf2ed )
	ROM_REGION( CODE_SIZE, "maincpu", 0 )      /* 68000 code */
	ROM_LOAD16_BYTE( "sf2e_30d.11e", 0x00000, 0x20000, CRC(4bb2657c) SHA1(b2d077296b77be7db371f953b7fc446a67d8a9d6) )
	ROM_LOAD16_BYTE( "sf2e_37d.11f", 0x00001, 0x20000, CRC(102f4561) SHA1(2fc77cd3b2ecf8fadc4f8614cb200cf2cba4c616) )   //only rom different from sf2ud
	ROM_LOAD16_BYTE( "sf2e_31d.12e", 0x40000, 0x20000, CRC(d57b67d7) SHA1(43d0b47c9fada8d9b445caa4b96ac8493061aa8b) )
	ROM_LOAD16_BYTE( "sf2e_38d.12f", 0x40001, 0x20000, CRC(9c8916ef) SHA1(a4629356a816454bcc1d7b41e70e147d4769a682) )
	ROM_LOAD16_BYTE( "sf2e_28d.9e",  0x80000, 0x20000, CRC(175819d1) SHA1(c98b6b7af4e57735dbfb3d1e61ba1bfb9f145d33) )
	ROM_LOAD16_BYTE( "sf2e_35d.9f",  0x80001, 0x20000, CRC(82060da4) SHA1(7487cfc28cce3d76772ece657aef83b56034011e) )
	ROM_LOAD16_BYTE( "sf2_29b.10e",  0xc0000, 0x20000, CRC(bb4af315) SHA1(75f0827f4f7e9f292add46467f8d4fe19b2514c9) )
	ROM_LOAD16_BYTE( "sf2_36b.10f",  0xc0001, 0x20000, CRC(c02a13eb) SHA1(b807cc495bff3f95d03b061fc629c95f965cb6d8) )

	ROM_REGION( 0x600000, "gfx", 0 )
	ROMX_LOAD( "sf2-5m.4a",  0x000000, 0x80000, CRC(22c9cc8e) SHA1(b9194fb337b30502c1c9501cd6c64ae4035544d4) , ROM_GROUPWORD | ROM_SKIP(6) )    // in "2" socket
	ROMX_LOAD( "sf2-7m.6a",  0x000002, 0x80000, CRC(57213be8) SHA1(3759b851ac0904ec79cbb67a2264d384b6f2f9f9) , ROM_GROUPWORD | ROM_SKIP(6) )    // in "4" socket
	ROMX_LOAD( "sf2-1m.3a",  0x000004, 0x80000, CRC(ba529b4f) SHA1(520840d727161cf09ca784919fa37bc9b54cc3ce) , ROM_GROUPWORD | ROM_SKIP(6) )    // in "1" socket
	ROMX_LOAD( "sf2-3m.5a",  0x000006, 0x80000, CRC(4b1b33a8) SHA1(2360cff890551f76775739e2d6563858bff80e41) , ROM_GROUPWORD | ROM_SKIP(6) )    // in "3" socket
	ROMX_LOAD( "sf2-6m.4c",  0x200000, 0x80000, CRC(2c7e2229) SHA1(357c2275af9133fd0bd6fbb1fa9ad5e0b490b3a2) , ROM_GROUPWORD | ROM_SKIP(6) )    // in "11" socket
	ROMX_LOAD( "sf2-8m.6c",  0x200002, 0x80000, CRC(b5548f17) SHA1(baa92b91cf616bc9e2a8a66adc777ffbf962a51b) , ROM_GROUPWORD | ROM_SKIP(6) )    // in "13" socket
	ROMX_LOAD( "sf2-2m.3c",  0x200004, 0x80000, CRC(14b84312) SHA1(2eea16673e60ba7a10bd4d8f6c217bb2441a5b0e) , ROM_GROUPWORD | ROM_SKIP(6) )    // in "10" socket
	ROMX_LOAD( "sf2-4m.5c",  0x200006, 0x80000, CRC(5e9cd89a) SHA1(f787aab98668d4c2c54fc4ba677c0cb808e4f31e) , ROM_GROUPWORD | ROM_SKIP(6) )    // in "12" socket
	ROMX_LOAD( "sf2-13m.4d", 0x400000, 0x80000, CRC(994bfa58) SHA1(5669b845f624b10e7be56bfc89b76592258ce48b) , ROM_GROUPWORD | ROM_SKIP(6) )    // in "21" socket
	ROMX_LOAD( "sf2-15m.6d", 0x400002, 0x80000, CRC(3e66ad9d) SHA1(9af9df0826988872662753e9717c48d46f2974b0) , ROM_GROUPWORD | ROM_SKIP(6) )    // in "23" socket
	ROMX_LOAD( "sf2-9m.3d",  0x400004, 0x80000, CRC(c1befaa8) SHA1(a6a7f4725e52678cbd8d557285c01cdccb2c2602) , ROM_GROUPWORD | ROM_SKIP(6) )    // in "20" socket
	ROMX_LOAD( "sf2-11m.5d", 0x400006, 0x80000, CRC(0627c831) SHA1(f9a92d614e8877d648449de2612fc8b43c85e4c2) , ROM_GROUPWORD | ROM_SKIP(6) )    // in "22" socket

	ROM_REGION( 0x18000, "audiocpu", 0 ) /* 64k for the audio CPU (+banks) */
	ROM_LOAD( "sf2_9.12a",   0x00000, 0x08000, CRC(a4823a1b) SHA1(7b6bf59dfd578bfbbdb64c27988796783442d659) )
	ROM_CONTINUE(            0x10000, 0x08000 )

	ROM_REGION( 0x40000, "oki", 0 ) /* Samples */
	ROM_LOAD( "sf2_18.11c",  0x00000, 0x20000, CRC(7f162009) SHA1(346bf42992b4c36c593e21901e22c87ae4a7d86d) )
	ROM_LOAD( "sf2_19.12c",  0x20000, 0x20000, CRC(beade53f) SHA1(277c397dc12752719ec6b47d2224750bd1c07f79) )

	ROM_REGION( 0x0200, "aboardplds", 0 )
	ROM_LOAD( "buf1",         0x0000, 0x0117, CRC(eb122de7) SHA1(b26b5bfe258e3e184f069719f9fd008d6b8f6b9b) )
	ROM_LOAD( "ioa1",         0x0000, 0x0117, CRC(59c7ee3b) SHA1(fbb887c5b4f5cb8df77cec710eaac2985bc482a6) )
	ROM_LOAD( "prg1",         0x0000, 0x0117, CRC(f1129744) SHA1(a5300f301c1a08a7da768f0773fa0fe3f683b237) )
	ROM_LOAD( "rom1",         0x0000, 0x0117, CRC(41dc73b9) SHA1(7d4c9f1693c821fbf84e32dd6ef62ddf14967845) )
	ROM_LOAD( "sou1",         0x0000, 0x0117, CRC(84f4b2fe) SHA1(dcc9e86cc36316fe42eace02d6df75d08bc8bb6d) )

	ROM_REGION( 0x0200, "bboardplds", 0 )
	ROM_LOAD( "stf29.1a",     0x0000, 0x0117, CRC(043309c5) SHA1(f6b9610c9f896c495041ba56a654776a521c356b) )
	ROM_LOAD( "iob1.11d",     0x0000, 0x0117, CRC(3abc0700) SHA1(973043aa46ec6d5d1db20dc9d5937005a0f9f6ae) )

	ROM_REGION( 0x0200, "cboardplds", 0 )
	ROM_LOAD( "c632.ic1",     0x0000, 0x0117, CRC(0fbd9270) SHA1(d7e737b20c44d41e29ca94be56114b31934dde81) )
ROM_END

/* B-Board 90629B-3 */
/* Note that this revision seems to be the only one that uses the IOB2 and C632B PALs instead of the IOB1 and C632,
   while STF29 PAL is confirmed to be the same as the other Street Fighter II: The World Warrior sets. */
ROM_START( sf2ee )
	ROM_REGION( CODE_SIZE, "maincpu", 0 )      /* 68000 code */
	ROM_LOAD16_BYTE( "sf2e_30e.11e", 0x00000, 0x20000, CRC(f37cd088) SHA1(48b71e44ce88d5f682ed679c737e7ec5262bb0df) )
	ROM_LOAD16_BYTE( "sf2e_37e.11f", 0x00001, 0x20000, CRC(c39468e6) SHA1(0b1ce83dbc4dcf9b205c67fa6a4b074570baf59b) )
	ROM_LOAD16_BYTE( "sf2e_31e.12e", 0x40000, 0x20000, CRC(7c4771b4) SHA1(6637b24194c86ec72a1775d4e976891243cd66fd) )
	ROM_LOAD16_BYTE( "sf2e_38e.12f", 0x40001, 0x20000, CRC(a4bd0cd9) SHA1(32a2bc18d1f860668141e53cbca862ceec238c19) )
	ROM_LOAD16_BYTE( "sf2e_28e.9e",  0x80000, 0x20000, CRC(e3b95625) SHA1(f7277f9980040f96434d1bd162eaf9ba0dfbb005) )
	ROM_LOAD16_BYTE( "sf2e_35e.9f",  0x80001, 0x20000, CRC(3648769a) SHA1(74e5934b0e3b4da35ff48086f41e7502b42731c6) )
	ROM_LOAD16_BYTE( "sf2_29b.10e",  0xc0000, 0x20000, CRC(bb4af315) SHA1(75f0827f4f7e9f292add46467f8d4fe19b2514c9) )
	ROM_LOAD16_BYTE( "sf2_36b.10f",  0xc0001, 0x20000, CRC(c02a13eb) SHA1(b807cc495bff3f95d03b061fc629c95f965cb6d8) )

	ROM_REGION( 0x600000, "gfx", 0 )
	ROMX_LOAD( "sf2-5m.4a",  0x000000, 0x80000, CRC(22c9cc8e) SHA1(b9194fb337b30502c1c9501cd6c64ae4035544d4) , ROM_GROUPWORD | ROM_SKIP(6) )    // in "2" socket
	ROMX_LOAD( "sf2-7m.6a",  0x000002, 0x80000, CRC(57213be8) SHA1(3759b851ac0904ec79cbb67a2264d384b6f2f9f9) , ROM_GROUPWORD | ROM_SKIP(6) )    // in "4" socket
	ROMX_LOAD( "sf2-1m.3a",  0x000004, 0x80000, CRC(ba529b4f) SHA1(520840d727161cf09ca784919fa37bc9b54cc3ce) , ROM_GROUPWORD | ROM_SKIP(6) )    // in "1" socket
	ROMX_LOAD( "sf2-3m.5a",  0x000006, 0x80000, CRC(4b1b33a8) SHA1(2360cff890551f76775739e2d6563858bff80e41) , ROM_GROUPWORD | ROM_SKIP(6) )    // in "3" socket
	ROMX_LOAD( "sf2-6m.4c",  0x200000, 0x80000, CRC(2c7e2229) SHA1(357c2275af9133fd0bd6fbb1fa9ad5e0b490b3a2) , ROM_GROUPWORD | ROM_SKIP(6) )    // in "11" socket
	ROMX_LOAD( "sf2-8m.6c",  0x200002, 0x80000, CRC(b5548f17) SHA1(baa92b91cf616bc9e2a8a66adc777ffbf962a51b) , ROM_GROUPWORD | ROM_SKIP(6) )    // in "13" socket
	ROMX_LOAD( "sf2-2m.3c",  0x200004, 0x80000, CRC(14b84312) SHA1(2eea16673e60ba7a10bd4d8f6c217bb2441a5b0e) , ROM_GROUPWORD | ROM_SKIP(6) )    // in "10" socket
	ROMX_LOAD( "sf2-4m.5c",  0x200006, 0x80000, CRC(5e9cd89a) SHA1(f787aab98668d4c2c54fc4ba677c0cb808e4f31e) , ROM_GROUPWORD | ROM_SKIP(6) )    // in "12" socket
	ROMX_LOAD( "sf2-13m.4d", 0x400000, 0x80000, CRC(994bfa58) SHA1(5669b845f624b10e7be56bfc89b76592258ce48b) , ROM_GROUPWORD | ROM_SKIP(6) )    // in "21" socket
	ROMX_LOAD( "sf2-15m.6d", 0x400002, 0x80000, CRC(3e66ad9d) SHA1(9af9df0826988872662753e9717c48d46f2974b0) , ROM_GROUPWORD | ROM_SKIP(6) )    // in "23" socket
	ROMX_LOAD( "sf2-9m.3d",  0x400004, 0x80000, CRC(c1befaa8) SHA1(a6a7f4725e52678cbd8d557285c01cdccb2c2602) , ROM_GROUPWORD | ROM_SKIP(6) )    // in "20" socket
	ROMX_LOAD( "sf2-11m.5d", 0x400006, 0x80000, CRC(0627c831) SHA1(f9a92d614e8877d648449de2612fc8b43c85e4c2) , ROM_GROUPWORD | ROM_SKIP(6) )    // in "22" socket

	ROM_REGION( 0x18000, "audiocpu", 0 ) /* 64k for the audio CPU (+banks) */
	ROM_LOAD( "sf2_9.12a",   0x00000, 0x08000, CRC(a4823a1b) SHA1(7b6bf59dfd578bfbbdb64c27988796783442d659) )
	ROM_CONTINUE(            0x10000, 0x08000 )

	ROM_REGION( 0x40000, "oki", 0 ) /* Samples */
	ROM_LOAD( "sf2_18.11c",  0x00000, 0x20000, CRC(7f162009) SHA1(346bf42992b4c36c593e21901e22c87ae4a7d86d) )
	ROM_LOAD( "sf2_19.12c",  0x20000, 0x20000, CRC(beade53f) SHA1(277c397dc12752719ec6b47d2224750bd1c07f79) )

	ROM_REGION( 0x0200, "aboardplds", 0 )
	ROM_LOAD( "buf1",         0x0000, 0x0117, CRC(eb122de7) SHA1(b26b5bfe258e3e184f069719f9fd008d6b8f6b9b) )
	ROM_LOAD( "ioa1",         0x0000, 0x0117, CRC(59c7ee3b) SHA1(fbb887c5b4f5cb8df77cec710eaac2985bc482a6) )
	ROM_LOAD( "prg1",         0x0000, 0x0117, CRC(f1129744) SHA1(a5300f301c1a08a7da768f0773fa0fe3f683b237) )
	ROM_LOAD( "rom1",         0x0000, 0x0117, CRC(41dc73b9) SHA1(7d4c9f1693c821fbf84e32dd6ef62ddf14967845) )
	ROM_LOAD( "sou1",         0x0000, 0x0117, CRC(84f4b2fe) SHA1(dcc9e86cc36316fe42eace02d6df75d08bc8bb6d) )

	ROM_REGION( 0x0200, "bboardplds", 0 )
	ROM_LOAD( "stf29.1a",     0x0000, 0x0117, CRC(043309c5) SHA1(f6b9610c9f896c495041ba56a654776a521c356b) )
	ROM_LOAD( "iob2.11d",     0x0000, 0x0117, CRC(d26f0a27) SHA1(22bb5647ff98df22fd19ae079ff98b9d100855f9) )

	ROM_REGION( 0x0200, "cboardplds", 0 )
	ROM_LOAD( "c632b.ic1",    0x0000, 0x0117, CRC(0fbd9270) SHA1(d7e737b20c44d41e29ca94be56114b31934dde81) )    // == c632.ic1
ROM_END

/* B-Board 90629B-2 */
ROM_START( sf2ua )
	ROM_REGION( CODE_SIZE, "maincpu", 0 )      /* 68000 code */
	ROM_LOAD16_BYTE( "sf2u_30a.11e", 0x00000, 0x20000, CRC(08beb861) SHA1(d47f16d0d692dc6405df0aecd7d9fc3f9718c0d1) )
	ROM_LOAD16_BYTE( "sf2u_37a.11f", 0x00001, 0x20000, CRC(b7638d69) SHA1(b615a2e0e8772462fd875b2e8d2ccba82a8b3c47) )
	ROM_LOAD16_BYTE( "sf2u_31a.12e", 0x40000, 0x20000, CRC(0d5394e0) SHA1(e1d88ff3669f1dbe1e3fbdf8aa9e2c63adbbcb48) )
	ROM_LOAD16_BYTE( "sf2u_38a.12f", 0x40001, 0x20000, CRC(42d6a79e) SHA1(5f1e2c176d065325883a60767d05b1a542372b6a) )
	ROM_LOAD16_BYTE( "sf2u_28a.9e",  0x80000, 0x20000, CRC(387a175c) SHA1(2635bb82758cf217cee63b254a537b02275a6838) )
	ROM_LOAD16_BYTE( "sf2u_35a.9f",  0x80001, 0x20000, CRC(a1a5adcc) SHA1(47874e6d403256d828474b29e3d93c92efd9e1ce) )
	ROM_LOAD16_BYTE( "sf2_29b.10e",  0xc0000, 0x20000, CRC(bb4af315) SHA1(75f0827f4f7e9f292add46467f8d4fe19b2514c9) )
	ROM_LOAD16_BYTE( "sf2_36b.10f",  0xc0001, 0x20000, CRC(c02a13eb) SHA1(b807cc495bff3f95d03b061fc629c95f965cb6d8) )

	ROM_REGION( 0x600000, "gfx", 0 )
	ROMX_LOAD( "sf2-5m.4a",  0x000000, 0x80000, CRC(22c9cc8e) SHA1(b9194fb337b30502c1c9501cd6c64ae4035544d4) , ROM_GROUPWORD | ROM_SKIP(6) )    // in "2" socket
	ROMX_LOAD( "sf2-7m.6a",  0x000002, 0x80000, CRC(57213be8) SHA1(3759b851ac0904ec79cbb67a2264d384b6f2f9f9) , ROM_GROUPWORD | ROM_SKIP(6) )    // in "4" socket
	ROMX_LOAD( "sf2-1m.3a",  0x000004, 0x80000, CRC(ba529b4f) SHA1(520840d727161cf09ca784919fa37bc9b54cc3ce) , ROM_GROUPWORD | ROM_SKIP(6) )    // in "1" socket
	ROMX_LOAD( "sf2-3m.5a",  0x000006, 0x80000, CRC(4b1b33a8) SHA1(2360cff890551f76775739e2d6563858bff80e41) , ROM_GROUPWORD | ROM_SKIP(6) )    // in "3" socket
	ROMX_LOAD( "sf2-6m.4c",  0x200000, 0x80000, CRC(2c7e2229) SHA1(357c2275af9133fd0bd6fbb1fa9ad5e0b490b3a2) , ROM_GROUPWORD | ROM_SKIP(6) )    // in "11" socket
	ROMX_LOAD( "sf2-8m.6c",  0x200002, 0x80000, CRC(b5548f17) SHA1(baa92b91cf616bc9e2a8a66adc777ffbf962a51b) , ROM_GROUPWORD | ROM_SKIP(6) )    // in "13" socket
	ROMX_LOAD( "sf2-2m.3c",  0x200004, 0x80000, CRC(14b84312) SHA1(2eea16673e60ba7a10bd4d8f6c217bb2441a5b0e) , ROM_GROUPWORD | ROM_SKIP(6) )    // in "10" socket
	ROMX_LOAD( "sf2-4m.5c",  0x200006, 0x80000, CRC(5e9cd89a) SHA1(f787aab98668d4c2c54fc4ba677c0cb808e4f31e) , ROM_GROUPWORD | ROM_SKIP(6) )    // in "12" socket
	ROMX_LOAD( "sf2-13m.4d", 0x400000, 0x80000, CRC(994bfa58) SHA1(5669b845f624b10e7be56bfc89b76592258ce48b) , ROM_GROUPWORD | ROM_SKIP(6) )    // in "21" socket
	ROMX_LOAD( "sf2-15m.6d", 0x400002, 0x80000, CRC(3e66ad9d) SHA1(9af9df0826988872662753e9717c48d46f2974b0) , ROM_GROUPWORD | ROM_SKIP(6) )    // in "23" socket
	ROMX_LOAD( "sf2-9m.3d",  0x400004, 0x80000, CRC(c1befaa8) SHA1(a6a7f4725e52678cbd8d557285c01cdccb2c2602) , ROM_GROUPWORD | ROM_SKIP(6) )    // in "20" socket
	ROMX_LOAD( "sf2-11m.5d", 0x400006, 0x80000, CRC(0627c831) SHA1(f9a92d614e8877d648449de2612fc8b43c85e4c2) , ROM_GROUPWORD | ROM_SKIP(6) )    // in "22" socket

	ROM_REGION( 0x18000, "audiocpu", 0 ) /* 64k for the audio CPU (+banks) */
	ROM_LOAD( "sf2_9.12a",   0x00000, 0x08000, CRC(a4823a1b) SHA1(7b6bf59dfd578bfbbdb64c27988796783442d659) )
	ROM_CONTINUE(            0x10000, 0x08000 )

	ROM_REGION( 0x40000, "oki", 0 ) /* Samples */
	ROM_LOAD( "sf2_18.11c",  0x00000, 0x20000, CRC(7f162009) SHA1(346bf42992b4c36c593e21901e22c87ae4a7d86d) )
	ROM_LOAD( "sf2_19.12c",  0x20000, 0x20000, CRC(beade53f) SHA1(277c397dc12752719ec6b47d2224750bd1c07f79) )

	ROM_REGION( 0x0200, "aboardplds", 0 )
	ROM_LOAD( "buf1",         0x0000, 0x0117, CRC(eb122de7) SHA1(b26b5bfe258e3e184f069719f9fd008d6b8f6b9b) )
	ROM_LOAD( "ioa1",         0x0000, 0x0117, CRC(59c7ee3b) SHA1(fbb887c5b4f5cb8df77cec710eaac2985bc482a6) )
	ROM_LOAD( "prg1",         0x0000, 0x0117, CRC(f1129744) SHA1(a5300f301c1a08a7da768f0773fa0fe3f683b237) )
	ROM_LOAD( "rom1",         0x0000, 0x0117, CRC(41dc73b9) SHA1(7d4c9f1693c821fbf84e32dd6ef62ddf14967845) )
	ROM_LOAD( "sou1",         0x0000, 0x0117, CRC(84f4b2fe) SHA1(dcc9e86cc36316fe42eace02d6df75d08bc8bb6d) )

	ROM_REGION( 0x0200, "bboardplds", 0 )
	ROM_LOAD( "stf29.1a",     0x0000, 0x0117, CRC(043309c5) SHA1(f6b9610c9f896c495041ba56a654776a521c356b) )
	ROM_LOAD( "iob1.11d",     0x0000, 0x0117, CRC(3abc0700) SHA1(973043aa46ec6d5d1db20dc9d5937005a0f9f6ae) )

	ROM_REGION( 0x0200, "cboardplds", 0 )
	ROM_LOAD( "c632.ic1",     0x0000, 0x0117, CRC(0fbd9270) SHA1(d7e737b20c44d41e29ca94be56114b31934dde81) )
ROM_END

/* B-Board 90629B-3 */
ROM_START( sf2ub )
	ROM_REGION( CODE_SIZE, "maincpu", 0 )      /* 68000 code */
	ROM_LOAD16_BYTE( "sf2u_30b.11e", 0x00000, 0x20000, CRC(57bd7051) SHA1(5e211e75b1649b07723cabc03cf15636dbbae595) )   // == sf2e_30b.11e
	ROM_LOAD16_BYTE( "sf2u_37b.11f", 0x00001, 0x20000, CRC(4a54d479) SHA1(eaff7a0d3c858a567c02086fde163850f0f5631e) )
	ROM_LOAD16_BYTE( "sf2u_31b.12e", 0x40000, 0x20000, CRC(a673143d) SHA1(e565f0ec23d6deb543c72af5a83f070c07319477) )   // == sf2e_31b.12e
	ROM_LOAD16_BYTE( "sf2u_38b.12f", 0x40001, 0x20000, CRC(4c2ccef7) SHA1(77b119c70c255622b023de25d9af3b3aac52ea47) )   // == sf2e_38b.12f
	ROM_LOAD16_BYTE( "sf2u_28b.9e",  0x80000, 0x20000, CRC(4009955e) SHA1(7842dbef7650485639fbae49b9f4db7494d4f73d) )   // == sf2_28b.9e
	ROM_LOAD16_BYTE( "sf2u_35b.9f",  0x80001, 0x20000, CRC(8c1f3994) SHA1(5e1d334399d05a837c2d80f79eada543e83afaf7) )   // == sf2_35b.9f
	ROM_LOAD16_BYTE( "sf2_29b.10e",  0xc0000, 0x20000, CRC(bb4af315) SHA1(75f0827f4f7e9f292add46467f8d4fe19b2514c9) )
	ROM_LOAD16_BYTE( "sf2_36b.10f",  0xc0001, 0x20000, CRC(c02a13eb) SHA1(b807cc495bff3f95d03b061fc629c95f965cb6d8) )

	ROM_REGION( 0x600000, "gfx", 0 )
	ROMX_LOAD( "sf2-5m.4a",  0x000000, 0x80000, CRC(22c9cc8e) SHA1(b9194fb337b30502c1c9501cd6c64ae4035544d4) , ROM_GROUPWORD | ROM_SKIP(6) )    // in "2" socket
	ROMX_LOAD( "sf2-7m.6a",  0x000002, 0x80000, CRC(57213be8) SHA1(3759b851ac0904ec79cbb67a2264d384b6f2f9f9) , ROM_GROUPWORD | ROM_SKIP(6) )    // in "4" socket
	ROMX_LOAD( "sf2-1m.3a",  0x000004, 0x80000, CRC(ba529b4f) SHA1(520840d727161cf09ca784919fa37bc9b54cc3ce) , ROM_GROUPWORD | ROM_SKIP(6) )    // in "1" socket
	ROMX_LOAD( "sf2-3m.5a",  0x000006, 0x80000, CRC(4b1b33a8) SHA1(2360cff890551f76775739e2d6563858bff80e41) , ROM_GROUPWORD | ROM_SKIP(6) )    // in "3" socket
	ROMX_LOAD( "sf2-6m.4c",  0x200000, 0x80000, CRC(2c7e2229) SHA1(357c2275af9133fd0bd6fbb1fa9ad5e0b490b3a2) , ROM_GROUPWORD | ROM_SKIP(6) )    // in "11" socket
	ROMX_LOAD( "sf2-8m.6c",  0x200002, 0x80000, CRC(b5548f17) SHA1(baa92b91cf616bc9e2a8a66adc777ffbf962a51b) , ROM_GROUPWORD | ROM_SKIP(6) )    // in "13" socket
	ROMX_LOAD( "sf2-2m.3c",  0x200004, 0x80000, CRC(14b84312) SHA1(2eea16673e60ba7a10bd4d8f6c217bb2441a5b0e) , ROM_GROUPWORD | ROM_SKIP(6) )    // in "10" socket
	ROMX_LOAD( "sf2-4m.5c",  0x200006, 0x80000, CRC(5e9cd89a) SHA1(f787aab98668d4c2c54fc4ba677c0cb808e4f31e) , ROM_GROUPWORD | ROM_SKIP(6) )    // in "12" socket
	ROMX_LOAD( "sf2-13m.4d", 0x400000, 0x80000, CRC(994bfa58) SHA1(5669b845f624b10e7be56bfc89b76592258ce48b) , ROM_GROUPWORD | ROM_SKIP(6) )    // in "21" socket
	ROMX_LOAD( "sf2-15m.6d", 0x400002, 0x80000, CRC(3e66ad9d) SHA1(9af9df0826988872662753e9717c48d46f2974b0) , ROM_GROUPWORD | ROM_SKIP(6) )    // in "23" socket
	ROMX_LOAD( "sf2-9m.3d",  0x400004, 0x80000, CRC(c1befaa8) SHA1(a6a7f4725e52678cbd8d557285c01cdccb2c2602) , ROM_GROUPWORD | ROM_SKIP(6) )    // in "20" socket
	ROMX_LOAD( "sf2-11m.5d", 0x400006, 0x80000, CRC(0627c831) SHA1(f9a92d614e8877d648449de2612fc8b43c85e4c2) , ROM_GROUPWORD | ROM_SKIP(6) )    // in "22" socket

	ROM_REGION( 0x18000, "audiocpu", 0 ) /* 64k for the audio CPU (+banks) */
	ROM_LOAD( "sf2_9.12a",   0x00000, 0x08000, CRC(a4823a1b) SHA1(7b6bf59dfd578bfbbdb64c27988796783442d659) )
	ROM_CONTINUE(            0x10000, 0x08000 )

	ROM_REGION( 0x40000, "oki", 0 ) /* Samples */
	ROM_LOAD( "sf2_18.11c",  0x00000, 0x20000, CRC(7f162009) SHA1(346bf42992b4c36c593e21901e22c87ae4a7d86d) )
	ROM_LOAD( "sf2_19.12c",  0x20000, 0x20000, CRC(beade53f) SHA1(277c397dc12752719ec6b47d2224750bd1c07f79) )

	ROM_REGION( 0x0200, "aboardplds", 0 )
	ROM_LOAD( "buf1",         0x0000, 0x0117, CRC(eb122de7) SHA1(b26b5bfe258e3e184f069719f9fd008d6b8f6b9b) )
	ROM_LOAD( "ioa1",         0x0000, 0x0117, CRC(59c7ee3b) SHA1(fbb887c5b4f5cb8df77cec710eaac2985bc482a6) )
	ROM_LOAD( "prg1",         0x0000, 0x0117, CRC(f1129744) SHA1(a5300f301c1a08a7da768f0773fa0fe3f683b237) )
	ROM_LOAD( "rom1",         0x0000, 0x0117, CRC(41dc73b9) SHA1(7d4c9f1693c821fbf84e32dd6ef62ddf14967845) )
	ROM_LOAD( "sou1",         0x0000, 0x0117, CRC(84f4b2fe) SHA1(dcc9e86cc36316fe42eace02d6df75d08bc8bb6d) )

	ROM_REGION( 0x0200, "bboardplds", 0 )
	ROM_LOAD( "stf29.1a",     0x0000, 0x0117, CRC(043309c5) SHA1(f6b9610c9f896c495041ba56a654776a521c356b) )
	ROM_LOAD( "iob1.11d",     0x0000, 0x0117, CRC(3abc0700) SHA1(973043aa46ec6d5d1db20dc9d5937005a0f9f6ae) )

	ROM_REGION( 0x0200, "cboardplds", 0 )
	ROM_LOAD( "c632.ic1",     0x0000, 0x0117, CRC(0fbd9270) SHA1(d7e737b20c44d41e29ca94be56114b31934dde81) )
ROM_END

/* B-Board 90629B-3 */
ROM_START( sf2uc )
	ROM_REGION( CODE_SIZE, "maincpu", 0 )      /* 68000 code */
	ROM_LOAD16_BYTE( "sf2u_30c.11e", 0x00000, 0x20000, CRC(6cb59385) SHA1(37750e8369298b597590d956f5fb7319301e6c7d) )
	ROM_LOAD16_BYTE( "sf2u_37c.11f", 0x00001, 0x20000, CRC(32e2c278) SHA1(6cc5e0a9179163136b8104a1583da85a53d537f4) )
	ROM_LOAD16_BYTE( "sf2u_31c.12e", 0x40000, 0x20000, CRC(c4fff4a9) SHA1(4b593ace201fe7f5a00b5cd7f4e8fc3f8dd4ceed) )
	ROM_LOAD16_BYTE( "sf2u_38c.12f", 0x40001, 0x20000, CRC(8210fc0e) SHA1(7d6cfb99afa89d0e6e991d9f7c1808b740def125) )
	ROM_LOAD16_BYTE( "sf2u_28c.9e",  0x80000, 0x20000, CRC(6eddd5e8) SHA1(62bd1c2fc0321809421c9a592f691b5b1a1d8807) )
	ROM_LOAD16_BYTE( "sf2u_35c.9f",  0x80001, 0x20000, CRC(6bcb404c) SHA1(b5f24556c633c521aadd94e016d78db6922e3dfa) )
	ROM_LOAD16_BYTE( "sf2_29b.10e",  0xc0000, 0x20000, CRC(bb4af315) SHA1(75f0827f4f7e9f292add46467f8d4fe19b2514c9) )
	ROM_LOAD16_BYTE( "sf2_36b.10f",  0xc0001, 0x20000, CRC(c02a13eb) SHA1(b807cc495bff3f95d03b061fc629c95f965cb6d8) )

	ROM_REGION( 0x600000, "gfx", 0 )
	ROMX_LOAD( "sf2-5m.4a",  0x000000, 0x80000, CRC(22c9cc8e) SHA1(b9194fb337b30502c1c9501cd6c64ae4035544d4) , ROM_GROUPWORD | ROM_SKIP(6) )    // in "2" socket
	ROMX_LOAD( "sf2-7m.6a",  0x000002, 0x80000, CRC(57213be8) SHA1(3759b851ac0904ec79cbb67a2264d384b6f2f9f9) , ROM_GROUPWORD | ROM_SKIP(6) )    // in "4" socket
	ROMX_LOAD( "sf2-1m.3a",  0x000004, 0x80000, CRC(ba529b4f) SHA1(520840d727161cf09ca784919fa37bc9b54cc3ce) , ROM_GROUPWORD | ROM_SKIP(6) )    // in "1" socket
	ROMX_LOAD( "sf2-3m.5a",  0x000006, 0x80000, CRC(4b1b33a8) SHA1(2360cff890551f76775739e2d6563858bff80e41) , ROM_GROUPWORD | ROM_SKIP(6) )    // in "3" socket
	ROMX_LOAD( "sf2-6m.4c",  0x200000, 0x80000, CRC(2c7e2229) SHA1(357c2275af9133fd0bd6fbb1fa9ad5e0b490b3a2) , ROM_GROUPWORD | ROM_SKIP(6) )    // in "11" socket
	ROMX_LOAD( "sf2-8m.6c",  0x200002, 0x80000, CRC(b5548f17) SHA1(baa92b91cf616bc9e2a8a66adc777ffbf962a51b) , ROM_GROUPWORD | ROM_SKIP(6) )    // in "13" socket
	ROMX_LOAD( "sf2-2m.3c",  0x200004, 0x80000, CRC(14b84312) SHA1(2eea16673e60ba7a10bd4d8f6c217bb2441a5b0e) , ROM_GROUPWORD | ROM_SKIP(6) )    // in "10" socket
	ROMX_LOAD( "sf2-4m.5c",  0x200006, 0x80000, CRC(5e9cd89a) SHA1(f787aab98668d4c2c54fc4ba677c0cb808e4f31e) , ROM_GROUPWORD | ROM_SKIP(6) )    // in "12" socket
	ROMX_LOAD( "sf2-13m.4d", 0x400000, 0x80000, CRC(994bfa58) SHA1(5669b845f624b10e7be56bfc89b76592258ce48b) , ROM_GROUPWORD | ROM_SKIP(6) )    // in "21" socket
	ROMX_LOAD( "sf2-15m.6d", 0x400002, 0x80000, CRC(3e66ad9d) SHA1(9af9df0826988872662753e9717c48d46f2974b0) , ROM_GROUPWORD | ROM_SKIP(6) )    // in "23" socket
	ROMX_LOAD( "sf2-9m.3d",  0x400004, 0x80000, CRC(c1befaa8) SHA1(a6a7f4725e52678cbd8d557285c01cdccb2c2602) , ROM_GROUPWORD | ROM_SKIP(6) )    // in "20" socket
	ROMX_LOAD( "sf2-11m.5d", 0x400006, 0x80000, CRC(0627c831) SHA1(f9a92d614e8877d648449de2612fc8b43c85e4c2) , ROM_GROUPWORD | ROM_SKIP(6) )    // in "22" socket

	ROM_REGION( 0x18000, "audiocpu", 0 ) /* 64k for the audio CPU (+banks) */
	ROM_LOAD( "sf2_9.12a",   0x00000, 0x08000, CRC(a4823a1b) SHA1(7b6bf59dfd578bfbbdb64c27988796783442d659) )   // could be sf2_09.12a
	ROM_CONTINUE(            0x10000, 0x08000 )

	ROM_REGION( 0x40000, "oki", 0 ) /* Samples */
	ROM_LOAD( "sf2_18.11c",  0x00000, 0x20000, CRC(7f162009) SHA1(346bf42992b4c36c593e21901e22c87ae4a7d86d) )
	ROM_LOAD( "sf2_19.12c",  0x20000, 0x20000, CRC(beade53f) SHA1(277c397dc12752719ec6b47d2224750bd1c07f79) )

	ROM_REGION( 0x0200, "aboardplds", 0 )
	ROM_LOAD( "buf1",         0x0000, 0x0117, CRC(eb122de7) SHA1(b26b5bfe258e3e184f069719f9fd008d6b8f6b9b) )
	ROM_LOAD( "ioa1",         0x0000, 0x0117, CRC(59c7ee3b) SHA1(fbb887c5b4f5cb8df77cec710eaac2985bc482a6) )
	ROM_LOAD( "prg1",         0x0000, 0x0117, CRC(f1129744) SHA1(a5300f301c1a08a7da768f0773fa0fe3f683b237) )
	ROM_LOAD( "rom1",         0x0000, 0x0117, CRC(41dc73b9) SHA1(7d4c9f1693c821fbf84e32dd6ef62ddf14967845) )
	ROM_LOAD( "sou1",         0x0000, 0x0117, CRC(84f4b2fe) SHA1(dcc9e86cc36316fe42eace02d6df75d08bc8bb6d) )

	ROM_REGION( 0x0200, "bboardplds", 0 )
	ROM_LOAD( "stf29.1a",     0x0000, 0x0117, CRC(043309c5) SHA1(f6b9610c9f896c495041ba56a654776a521c356b) )
	ROM_LOAD( "iob1.11d",     0x0000, 0x0117, CRC(3abc0700) SHA1(973043aa46ec6d5d1db20dc9d5937005a0f9f6ae) )

	ROM_REGION( 0x0200, "cboardplds", 0 )
	ROM_LOAD( "c632.ic1",     0x0000, 0x0117, CRC(0fbd9270) SHA1(d7e737b20c44d41e29ca94be56114b31934dde81) )
ROM_END

/* B-Board 90629B-3 */
ROM_START( sf2ud )
	ROM_REGION( CODE_SIZE, "maincpu", 0 )      /* 68000 code */
	ROM_LOAD16_BYTE( "sf2u_30d.11e", 0x00000, 0x20000, CRC(4bb2657c) SHA1(b2d077296b77be7db371f953b7fc446a67d8a9d6) )   // == sf2e_30d.11e
	ROM_LOAD16_BYTE( "sf2u_37d.11f", 0x00001, 0x20000, CRC(b33b42f2) SHA1(2e0babc8734c79dc2b51a6be64433bb2411c3da5) )
	ROM_LOAD16_BYTE( "sf2u_31d.12e", 0x40000, 0x20000, CRC(d57b67d7) SHA1(43d0b47c9fada8d9b445caa4b96ac8493061aa8b) )   // == sf2e_31d.12e
	ROM_LOAD16_BYTE( "sf2u_38d.12f", 0x40001, 0x20000, CRC(9c8916ef) SHA1(a4629356a816454bcc1d7b41e70e147d4769a682) )   // == sf2e_38d.12f
	ROM_LOAD16_BYTE( "sf2u_28d.9e",  0x80000, 0x20000, CRC(175819d1) SHA1(c98b6b7af4e57735dbfb3d1e61ba1bfb9f145d33) )   // == sf2e_28d.9e
	ROM_LOAD16_BYTE( "sf2u_35d.9f",  0x80001, 0x20000, CRC(82060da4) SHA1(7487cfc28cce3d76772ece657aef83b56034011e) )   // == sf2e_35d.9f
	ROM_LOAD16_BYTE( "sf2_29b.10e",  0xc0000, 0x20000, CRC(bb4af315) SHA1(75f0827f4f7e9f292add46467f8d4fe19b2514c9) )
	ROM_LOAD16_BYTE( "sf2_36b.10f",  0xc0001, 0x20000, CRC(c02a13eb) SHA1(b807cc495bff3f95d03b061fc629c95f965cb6d8) )

	ROM_REGION( 0x600000, "gfx", 0 )
	ROMX_LOAD( "sf2-5m.4a",  0x000000, 0x80000, CRC(22c9cc8e) SHA1(b9194fb337b30502c1c9501cd6c64ae4035544d4) , ROM_GROUPWORD | ROM_SKIP(6) )    // in "2" socket
	ROMX_LOAD( "sf2-7m.6a",  0x000002, 0x80000, CRC(57213be8) SHA1(3759b851ac0904ec79cbb67a2264d384b6f2f9f9) , ROM_GROUPWORD | ROM_SKIP(6) )    // in "4" socket
	ROMX_LOAD( "sf2-1m.3a",  0x000004, 0x80000, CRC(ba529b4f) SHA1(520840d727161cf09ca784919fa37bc9b54cc3ce) , ROM_GROUPWORD | ROM_SKIP(6) )    // in "1" socket
	ROMX_LOAD( "sf2-3m.5a",  0x000006, 0x80000, CRC(4b1b33a8) SHA1(2360cff890551f76775739e2d6563858bff80e41) , ROM_GROUPWORD | ROM_SKIP(6) )    // in "3" socket
	ROMX_LOAD( "sf2-6m.4c",  0x200000, 0x80000, CRC(2c7e2229) SHA1(357c2275af9133fd0bd6fbb1fa9ad5e0b490b3a2) , ROM_GROUPWORD | ROM_SKIP(6) )    // in "11" socket
	ROMX_LOAD( "sf2-8m.6c",  0x200002, 0x80000, CRC(b5548f17) SHA1(baa92b91cf616bc9e2a8a66adc777ffbf962a51b) , ROM_GROUPWORD | ROM_SKIP(6) )    // in "13" socket
	ROMX_LOAD( "sf2-2m.3c",  0x200004, 0x80000, CRC(14b84312) SHA1(2eea16673e60ba7a10bd4d8f6c217bb2441a5b0e) , ROM_GROUPWORD | ROM_SKIP(6) )    // in "10" socket
	ROMX_LOAD( "sf2-4m.5c",  0x200006, 0x80000, CRC(5e9cd89a) SHA1(f787aab98668d4c2c54fc4ba677c0cb808e4f31e) , ROM_GROUPWORD | ROM_SKIP(6) )    // in "12" socket
	ROMX_LOAD( "sf2-13m.4d", 0x400000, 0x80000, CRC(994bfa58) SHA1(5669b845f624b10e7be56bfc89b76592258ce48b) , ROM_GROUPWORD | ROM_SKIP(6) )    // in "21" socket
	ROMX_LOAD( "sf2-15m.6d", 0x400002, 0x80000, CRC(3e66ad9d) SHA1(9af9df0826988872662753e9717c48d46f2974b0) , ROM_GROUPWORD | ROM_SKIP(6) )    // in "23" socket
	ROMX_LOAD( "sf2-9m.3d",  0x400004, 0x80000, CRC(c1befaa8) SHA1(a6a7f4725e52678cbd8d557285c01cdccb2c2602) , ROM_GROUPWORD | ROM_SKIP(6) )    // in "20" socket
	ROMX_LOAD( "sf2-11m.5d", 0x400006, 0x80000, CRC(0627c831) SHA1(f9a92d614e8877d648449de2612fc8b43c85e4c2) , ROM_GROUPWORD | ROM_SKIP(6) )    // in "22" socket

	ROM_REGION( 0x18000, "audiocpu", 0 ) /* 64k for the audio CPU (+banks) */
	ROM_LOAD( "sf2_9.12a",   0x00000, 0x08000, CRC(a4823a1b) SHA1(7b6bf59dfd578bfbbdb64c27988796783442d659) )
	ROM_CONTINUE(            0x10000, 0x08000 )

	ROM_REGION( 0x40000, "oki", 0 ) /* Samples */
	ROM_LOAD( "sf2_18.11c",  0x00000, 0x20000, CRC(7f162009) SHA1(346bf42992b4c36c593e21901e22c87ae4a7d86d) )
	ROM_LOAD( "sf2_19.12c",  0x20000, 0x20000, CRC(beade53f) SHA1(277c397dc12752719ec6b47d2224750bd1c07f79) )

	ROM_REGION( 0x0200, "aboardplds", 0 )
	ROM_LOAD( "buf1",         0x0000, 0x0117, CRC(eb122de7) SHA1(b26b5bfe258e3e184f069719f9fd008d6b8f6b9b) )
	ROM_LOAD( "ioa1",         0x0000, 0x0117, CRC(59c7ee3b) SHA1(fbb887c5b4f5cb8df77cec710eaac2985bc482a6) )
	ROM_LOAD( "prg1",         0x0000, 0x0117, CRC(f1129744) SHA1(a5300f301c1a08a7da768f0773fa0fe3f683b237) )
	ROM_LOAD( "rom1",         0x0000, 0x0117, CRC(41dc73b9) SHA1(7d4c9f1693c821fbf84e32dd6ef62ddf14967845) )
	ROM_LOAD( "sou1",         0x0000, 0x0117, CRC(84f4b2fe) SHA1(dcc9e86cc36316fe42eace02d6df75d08bc8bb6d) )

	ROM_REGION( 0x0200, "bboardplds", 0 )
	ROM_LOAD( "stf29.1a",     0x0000, 0x0117, CRC(043309c5) SHA1(f6b9610c9f896c495041ba56a654776a521c356b) )
	ROM_LOAD( "iob1.11d",     0x0000, 0x0117, CRC(3abc0700) SHA1(973043aa46ec6d5d1db20dc9d5937005a0f9f6ae) )

	ROM_REGION( 0x0200, "cboardplds", 0 )
	ROM_LOAD( "c632.ic1",     0x0000, 0x0117, CRC(0fbd9270) SHA1(d7e737b20c44d41e29ca94be56114b31934dde81) )
ROM_END

/* B-Board 90629B-3 */
/* Note that this revision seems to be the only one that uses the IOB2 and C632B PALs instead of the IOB1 and C632,
   while STF29 PAL is confirmed to be the same as the other Street Fighter II: The World Warrior sets. */
ROM_START( sf2ue )
	ROM_REGION( CODE_SIZE, "maincpu", 0 )      /* 68000 code */
	ROM_LOAD16_BYTE( "sf2u_30e.11e", 0x00000, 0x20000, CRC(f37cd088) SHA1(48b71e44ce88d5f682ed679c737e7ec5262bb0df) )
	ROM_LOAD16_BYTE( "sf2u_37e.11f", 0x00001, 0x20000, CRC(6c61a513) SHA1(6dc9ccd58fd5ef15ff9df20c865ff6c850f2b7dc) )
	ROM_LOAD16_BYTE( "sf2u_31e.12e", 0x40000, 0x20000, CRC(7c4771b4) SHA1(6637b24194c86ec72a1775d4e976891243cd66fd) )
	ROM_LOAD16_BYTE( "sf2u_38e.12f", 0x40001, 0x20000, CRC(a4bd0cd9) SHA1(32a2bc18d1f860668141e53cbca862ceec238c19) )
	ROM_LOAD16_BYTE( "sf2u_28e.9e",  0x80000, 0x20000, CRC(e3b95625) SHA1(f7277f9980040f96434d1bd162eaf9ba0dfbb005) )
	ROM_LOAD16_BYTE( "sf2u_35e.9f",  0x80001, 0x20000, CRC(3648769a) SHA1(74e5934b0e3b4da35ff48086f41e7502b42731c6) )
	ROM_LOAD16_BYTE( "sf2_29b.10e",  0xc0000, 0x20000, CRC(bb4af315) SHA1(75f0827f4f7e9f292add46467f8d4fe19b2514c9) )
	ROM_LOAD16_BYTE( "sf2_36b.10f",  0xc0001, 0x20000, CRC(c02a13eb) SHA1(b807cc495bff3f95d03b061fc629c95f965cb6d8) )

	ROM_REGION( 0x600000, "gfx", 0 )
	ROMX_LOAD( "sf2-5m.4a",  0x000000, 0x80000, CRC(22c9cc8e) SHA1(b9194fb337b30502c1c9501cd6c64ae4035544d4) , ROM_GROUPWORD | ROM_SKIP(6) )    // in "2" socket
	ROMX_LOAD( "sf2-7m.6a",  0x000002, 0x80000, CRC(57213be8) SHA1(3759b851ac0904ec79cbb67a2264d384b6f2f9f9) , ROM_GROUPWORD | ROM_SKIP(6) )    // in "4" socket
	ROMX_LOAD( "sf2-1m.3a",  0x000004, 0x80000, CRC(ba529b4f) SHA1(520840d727161cf09ca784919fa37bc9b54cc3ce) , ROM_GROUPWORD | ROM_SKIP(6) )    // in "1" socket
	ROMX_LOAD( "sf2-3m.5a",  0x000006, 0x80000, CRC(4b1b33a8) SHA1(2360cff890551f76775739e2d6563858bff80e41) , ROM_GROUPWORD | ROM_SKIP(6) )    // in "3" socket
	ROMX_LOAD( "sf2-6m.4c",  0x200000, 0x80000, CRC(2c7e2229) SHA1(357c2275af9133fd0bd6fbb1fa9ad5e0b490b3a2) , ROM_GROUPWORD | ROM_SKIP(6) )    // in "11" socket
	ROMX_LOAD( "sf2-8m.6c",  0x200002, 0x80000, CRC(b5548f17) SHA1(baa92b91cf616bc9e2a8a66adc777ffbf962a51b) , ROM_GROUPWORD | ROM_SKIP(6) )    // in "13" socket
	ROMX_LOAD( "sf2-2m.3c",  0x200004, 0x80000, CRC(14b84312) SHA1(2eea16673e60ba7a10bd4d8f6c217bb2441a5b0e) , ROM_GROUPWORD | ROM_SKIP(6) )    // in "10" socket
	ROMX_LOAD( "sf2-4m.5c",  0x200006, 0x80000, CRC(5e9cd89a) SHA1(f787aab98668d4c2c54fc4ba677c0cb808e4f31e) , ROM_GROUPWORD | ROM_SKIP(6) )    // in "12" socket
	ROMX_LOAD( "sf2-13m.4d", 0x400000, 0x80000, CRC(994bfa58) SHA1(5669b845f624b10e7be56bfc89b76592258ce48b) , ROM_GROUPWORD | ROM_SKIP(6) )    // in "21" socket
	ROMX_LOAD( "sf2-15m.6d", 0x400002, 0x80000, CRC(3e66ad9d) SHA1(9af9df0826988872662753e9717c48d46f2974b0) , ROM_GROUPWORD | ROM_SKIP(6) )    // in "23" socket
	ROMX_LOAD( "sf2-9m.3d",  0x400004, 0x80000, CRC(c1befaa8) SHA1(a6a7f4725e52678cbd8d557285c01cdccb2c2602) , ROM_GROUPWORD | ROM_SKIP(6) )    // in "20" socket
	ROMX_LOAD( "sf2-11m.5d", 0x400006, 0x80000, CRC(0627c831) SHA1(f9a92d614e8877d648449de2612fc8b43c85e4c2) , ROM_GROUPWORD | ROM_SKIP(6) )    // in "22" socket

	ROM_REGION( 0x18000, "audiocpu", 0 ) /* 64k for the audio CPU (+banks) */
	ROM_LOAD( "sf2_9.12a",   0x00000, 0x08000, CRC(a4823a1b) SHA1(7b6bf59dfd578bfbbdb64c27988796783442d659) )
	ROM_CONTINUE(            0x10000, 0x08000 )

	ROM_REGION( 0x40000, "oki", 0 ) /* Samples */
	ROM_LOAD( "sf2_18.11c",  0x00000, 0x20000, CRC(7f162009) SHA1(346bf42992b4c36c593e21901e22c87ae4a7d86d) )
	ROM_LOAD( "sf2_19.12c",  0x20000, 0x20000, CRC(beade53f) SHA1(277c397dc12752719ec6b47d2224750bd1c07f79) )

	ROM_REGION( 0x0200, "aboardplds", 0 )
	ROM_LOAD( "buf1",         0x0000, 0x0117, CRC(eb122de7) SHA1(b26b5bfe258e3e184f069719f9fd008d6b8f6b9b) )
	ROM_LOAD( "ioa1",         0x0000, 0x0117, CRC(59c7ee3b) SHA1(fbb887c5b4f5cb8df77cec710eaac2985bc482a6) )
	ROM_LOAD( "prg1",         0x0000, 0x0117, CRC(f1129744) SHA1(a5300f301c1a08a7da768f0773fa0fe3f683b237) )
	ROM_LOAD( "rom1",         0x0000, 0x0117, CRC(41dc73b9) SHA1(7d4c9f1693c821fbf84e32dd6ef62ddf14967845) )
	ROM_LOAD( "sou1",         0x0000, 0x0117, CRC(84f4b2fe) SHA1(dcc9e86cc36316fe42eace02d6df75d08bc8bb6d) )

	ROM_REGION( 0x0200, "bboardplds", 0 )
	ROM_LOAD( "stf29.1a",     0x0000, 0x0117, CRC(043309c5) SHA1(f6b9610c9f896c495041ba56a654776a521c356b) )
	ROM_LOAD( "iob2.11d",     0x0000, 0x0117, CRC(d26f0a27) SHA1(22bb5647ff98df22fd19ae079ff98b9d100855f9) )

	ROM_REGION( 0x0200, "cboardplds", 0 )
	ROM_LOAD( "c632b.ic1",    0x0000, 0x0117, CRC(0fbd9270) SHA1(d7e737b20c44d41e29ca94be56114b31934dde81) )    // == c632.ic1
ROM_END

/* B-Board 90629B-3 */
ROM_START( sf2uf )
	ROM_REGION( CODE_SIZE, "maincpu", 0 )      /* 68000 code */
	ROM_LOAD16_BYTE( "sf2u_30f.11e", 0x00000, 0x20000, CRC(fe39ee33) SHA1(22558eb15e035b09b80935a32b8425d91cd79669) )   // == sf2e_30g.11e
	ROM_LOAD16_BYTE( "sf2u_37f.11f", 0x00001, 0x20000, CRC(169e7388) SHA1(c7cb2de529d94cea4a018ed3bd611037fe54abe7) )
	ROM_LOAD16_BYTE( "sf2u_31f.12e", 0x40000, 0x20000, CRC(69a0a301) SHA1(86a3954335310865b14ce8b4e0e4499feb14fc12) )   // == sf2e_31g.12e
	ROM_LOAD16_BYTE( "sf2u_38f.12f", 0x40001, 0x20000, CRC(1510e4e2) SHA1(fbfdd4e42c4bc894592dbe5a84c88d5f13d21da4) )
	ROM_LOAD16_BYTE( "sf2u_28f.9e",  0x80000, 0x20000, CRC(acd8175b) SHA1(504991c46fa568d31ce69bd63e2a67926a06b5a9) )
	ROM_LOAD16_BYTE( "sf2u_35f.9f",  0x80001, 0x20000, CRC(c0a80bd1) SHA1(ac25a9ed488e03baf4115541fdcce3973ce6a442) )
	ROM_LOAD16_BYTE( "sf2_29b.10e",  0xc0000, 0x20000, CRC(bb4af315) SHA1(75f0827f4f7e9f292add46467f8d4fe19b2514c9) )
	ROM_LOAD16_BYTE( "sf2_36b.10f",  0xc0001, 0x20000, CRC(c02a13eb) SHA1(b807cc495bff3f95d03b061fc629c95f965cb6d8) )

	ROM_REGION( 0x600000, "gfx", 0 )
	ROMX_LOAD( "sf2-5m.4a",  0x000000, 0x80000, CRC(22c9cc8e) SHA1(b9194fb337b30502c1c9501cd6c64ae4035544d4) , ROM_GROUPWORD | ROM_SKIP(6) )    // in "2" socket
	ROMX_LOAD( "sf2-7m.6a",  0x000002, 0x80000, CRC(57213be8) SHA1(3759b851ac0904ec79cbb67a2264d384b6f2f9f9) , ROM_GROUPWORD | ROM_SKIP(6) )    // in "4" socket
	ROMX_LOAD( "sf2-1m.3a",  0x000004, 0x80000, CRC(ba529b4f) SHA1(520840d727161cf09ca784919fa37bc9b54cc3ce) , ROM_GROUPWORD | ROM_SKIP(6) )    // in "1" socket
	ROMX_LOAD( "sf2-3m.5a",  0x000006, 0x80000, CRC(4b1b33a8) SHA1(2360cff890551f76775739e2d6563858bff80e41) , ROM_GROUPWORD | ROM_SKIP(6) )    // in "3" socket
	ROMX_LOAD( "sf2-6m.4c",  0x200000, 0x80000, CRC(2c7e2229) SHA1(357c2275af9133fd0bd6fbb1fa9ad5e0b490b3a2) , ROM_GROUPWORD | ROM_SKIP(6) )    // in "11" socket
	ROMX_LOAD( "sf2-8m.6c",  0x200002, 0x80000, CRC(b5548f17) SHA1(baa92b91cf616bc9e2a8a66adc777ffbf962a51b) , ROM_GROUPWORD | ROM_SKIP(6) )    // in "13" socket
	ROMX_LOAD( "sf2-2m.3c",  0x200004, 0x80000, CRC(14b84312) SHA1(2eea16673e60ba7a10bd4d8f6c217bb2441a5b0e) , ROM_GROUPWORD | ROM_SKIP(6) )    // in "10" socket
	ROMX_LOAD( "sf2-4m.5c",  0x200006, 0x80000, CRC(5e9cd89a) SHA1(f787aab98668d4c2c54fc4ba677c0cb808e4f31e) , ROM_GROUPWORD | ROM_SKIP(6) )    // in "12" socket
	ROMX_LOAD( "sf2-13m.4d", 0x400000, 0x80000, CRC(994bfa58) SHA1(5669b845f624b10e7be56bfc89b76592258ce48b) , ROM_GROUPWORD | ROM_SKIP(6) )    // in "21" socket
	ROMX_LOAD( "sf2-15m.6d", 0x400002, 0x80000, CRC(3e66ad9d) SHA1(9af9df0826988872662753e9717c48d46f2974b0) , ROM_GROUPWORD | ROM_SKIP(6) )    // in "23" socket
	ROMX_LOAD( "sf2-9m.3d",  0x400004, 0x80000, CRC(c1befaa8) SHA1(a6a7f4725e52678cbd8d557285c01cdccb2c2602) , ROM_GROUPWORD | ROM_SKIP(6) )    // in "20" socket
	ROMX_LOAD( "sf2-11m.5d", 0x400006, 0x80000, CRC(0627c831) SHA1(f9a92d614e8877d648449de2612fc8b43c85e4c2) , ROM_GROUPWORD | ROM_SKIP(6) )    // in "22" socket

	ROM_REGION( 0x18000, "audiocpu", 0 ) /* 64k for the audio CPU (+banks) */
	ROM_LOAD( "sf2_9.12a",   0x00000, 0x08000, CRC(a4823a1b) SHA1(7b6bf59dfd578bfbbdb64c27988796783442d659) )
	ROM_CONTINUE(            0x10000, 0x08000 )

	ROM_REGION( 0x40000, "oki", 0 ) /* Samples */
	ROM_LOAD( "sf2_18.11c",  0x00000, 0x20000, CRC(7f162009) SHA1(346bf42992b4c36c593e21901e22c87ae4a7d86d) )
	ROM_LOAD( "sf2_19.12c",  0x20000, 0x20000, CRC(beade53f) SHA1(277c397dc12752719ec6b47d2224750bd1c07f79) )

	ROM_REGION( 0x0200, "aboardplds", 0 )
	ROM_LOAD( "buf1",         0x0000, 0x0117, CRC(eb122de7) SHA1(b26b5bfe258e3e184f069719f9fd008d6b8f6b9b) )
	ROM_LOAD( "ioa1",         0x0000, 0x0117, CRC(59c7ee3b) SHA1(fbb887c5b4f5cb8df77cec710eaac2985bc482a6) )
	ROM_LOAD( "prg1",         0x0000, 0x0117, CRC(f1129744) SHA1(a5300f301c1a08a7da768f0773fa0fe3f683b237) )
	ROM_LOAD( "rom1",         0x0000, 0x0117, CRC(41dc73b9) SHA1(7d4c9f1693c821fbf84e32dd6ef62ddf14967845) )
	ROM_LOAD( "sou1",         0x0000, 0x0117, CRC(84f4b2fe) SHA1(dcc9e86cc36316fe42eace02d6df75d08bc8bb6d) )

	ROM_REGION( 0x0200, "bboardplds", 0 )
	ROM_LOAD( "stf29.1a",     0x0000, 0x0117, CRC(043309c5) SHA1(f6b9610c9f896c495041ba56a654776a521c356b) )
	ROM_LOAD( "iob1.11d",     0x0000, 0x0117, CRC(3abc0700) SHA1(973043aa46ec6d5d1db20dc9d5937005a0f9f6ae) )

	ROM_REGION( 0x0200, "cboardplds", 0 )
	ROM_LOAD( "c632.ic1",     0x0000, 0x0117, CRC(0fbd9270) SHA1(d7e737b20c44d41e29ca94be56114b31934dde81) )
ROM_END

/* B-Board 90629B-3 */
ROM_START( sf2ug )
	ROM_REGION( CODE_SIZE, "maincpu", 0 )      /* 68000 code */
	ROM_LOAD16_BYTE( "sf2u_30g.11e", 0x00000, 0x20000, CRC(fe39ee33) SHA1(22558eb15e035b09b80935a32b8425d91cd79669) )   // == sf2e_30g.11e
	ROM_LOAD16_BYTE( "sf2u_37g.11f", 0x00001, 0x20000, CRC(5886cae7) SHA1(8a621d267dfcf5d214b1bbec12b98a06153c86d3) )
	ROM_LOAD16_BYTE( "sf2u_31g.12e", 0x40000, 0x20000, CRC(69a0a301) SHA1(86a3954335310865b14ce8b4e0e4499feb14fc12) )   // == sf2e_31g.12e
	ROM_LOAD16_BYTE( "sf2u_38g.12f", 0x40001, 0x20000, CRC(5e22db70) SHA1(6565946591a18eaf46f04c1aa449ee0ae9ac2901) )   // == sf2e_38g.12f
	ROM_LOAD16_BYTE( "sf2u_28g.9e",  0x80000, 0x20000, CRC(8bf9f1e5) SHA1(bbcef63f35e5bff3f373968ba1278dd6bd86b593) )   // == sf2e_28g.9e
	ROM_LOAD16_BYTE( "sf2u_35g.9f",  0x80001, 0x20000, CRC(626ef934) SHA1(507bda3e4519de237aca919cf72e543403ec9724) )   // == sf2e_35g.9f
	ROM_LOAD16_BYTE( "sf2_29b.10e",  0xc0000, 0x20000, CRC(bb4af315) SHA1(75f0827f4f7e9f292add46467f8d4fe19b2514c9) )
	ROM_LOAD16_BYTE( "sf2_36b.10f",  0xc0001, 0x20000, CRC(c02a13eb) SHA1(b807cc495bff3f95d03b061fc629c95f965cb6d8) )

	ROM_REGION( 0x600000, "gfx", 0 )
	ROMX_LOAD( "sf2-5m.4a",  0x000000, 0x80000, CRC(22c9cc8e) SHA1(b9194fb337b30502c1c9501cd6c64ae4035544d4) , ROM_GROUPWORD | ROM_SKIP(6) )    // in "2" socket
	ROMX_LOAD( "sf2-7m.6a",  0x000002, 0x80000, CRC(57213be8) SHA1(3759b851ac0904ec79cbb67a2264d384b6f2f9f9) , ROM_GROUPWORD | ROM_SKIP(6) )    // in "4" socket
	ROMX_LOAD( "sf2-1m.3a",  0x000004, 0x80000, CRC(ba529b4f) SHA1(520840d727161cf09ca784919fa37bc9b54cc3ce) , ROM_GROUPWORD | ROM_SKIP(6) )    // in "1" socket
	ROMX_LOAD( "sf2-3m.5a",  0x000006, 0x80000, CRC(4b1b33a8) SHA1(2360cff890551f76775739e2d6563858bff80e41) , ROM_GROUPWORD | ROM_SKIP(6) )    // in "3" socket
	ROMX_LOAD( "sf2-6m.4c",  0x200000, 0x80000, CRC(2c7e2229) SHA1(357c2275af9133fd0bd6fbb1fa9ad5e0b490b3a2) , ROM_GROUPWORD | ROM_SKIP(6) )    // in "11" socket
	ROMX_LOAD( "sf2-8m.6c",  0x200002, 0x80000, CRC(b5548f17) SHA1(baa92b91cf616bc9e2a8a66adc777ffbf962a51b) , ROM_GROUPWORD | ROM_SKIP(6) )    // in "13" socket
	ROMX_LOAD( "sf2-2m.3c",  0x200004, 0x80000, CRC(14b84312) SHA1(2eea16673e60ba7a10bd4d8f6c217bb2441a5b0e) , ROM_GROUPWORD | ROM_SKIP(6) )    // in "10" socket
	ROMX_LOAD( "sf2-4m.5c",  0x200006, 0x80000, CRC(5e9cd89a) SHA1(f787aab98668d4c2c54fc4ba677c0cb808e4f31e) , ROM_GROUPWORD | ROM_SKIP(6) )    // in "12" socket
	ROMX_LOAD( "sf2-13m.4d", 0x400000, 0x80000, CRC(994bfa58) SHA1(5669b845f624b10e7be56bfc89b76592258ce48b) , ROM_GROUPWORD | ROM_SKIP(6) )    // in "21" socket
	ROMX_LOAD( "sf2-15m.6d", 0x400002, 0x80000, CRC(3e66ad9d) SHA1(9af9df0826988872662753e9717c48d46f2974b0) , ROM_GROUPWORD | ROM_SKIP(6) )    // in "23" socket
	ROMX_LOAD( "sf2-9m.3d",  0x400004, 0x80000, CRC(c1befaa8) SHA1(a6a7f4725e52678cbd8d557285c01cdccb2c2602) , ROM_GROUPWORD | ROM_SKIP(6) )    // in "20" socket
	ROMX_LOAD( "sf2-11m.5d", 0x400006, 0x80000, CRC(0627c831) SHA1(f9a92d614e8877d648449de2612fc8b43c85e4c2) , ROM_GROUPWORD | ROM_SKIP(6) )    // in "22" socket

	ROM_REGION( 0x18000, "audiocpu", 0 ) /* 64k for the audio CPU (+banks) */
	ROM_LOAD( "sf2_9.12a",   0x00000, 0x08000, CRC(a4823a1b) SHA1(7b6bf59dfd578bfbbdb64c27988796783442d659) )
	ROM_CONTINUE(            0x10000, 0x08000 )

	ROM_REGION( 0x40000, "oki", 0 ) /* Samples */
	ROM_LOAD( "sf2_18.11c",  0x00000, 0x20000, CRC(7f162009) SHA1(346bf42992b4c36c593e21901e22c87ae4a7d86d) )
	ROM_LOAD( "sf2_19.12c",  0x20000, 0x20000, CRC(beade53f) SHA1(277c397dc12752719ec6b47d2224750bd1c07f79) )

	ROM_REGION( 0x0200, "aboardplds", 0 )
	ROM_LOAD( "buf1",         0x0000, 0x0117, CRC(eb122de7) SHA1(b26b5bfe258e3e184f069719f9fd008d6b8f6b9b) )
	ROM_LOAD( "ioa1",         0x0000, 0x0117, CRC(59c7ee3b) SHA1(fbb887c5b4f5cb8df77cec710eaac2985bc482a6) )
	ROM_LOAD( "prg1",         0x0000, 0x0117, CRC(f1129744) SHA1(a5300f301c1a08a7da768f0773fa0fe3f683b237) )
	ROM_LOAD( "rom1",         0x0000, 0x0117, CRC(41dc73b9) SHA1(7d4c9f1693c821fbf84e32dd6ef62ddf14967845) )
	ROM_LOAD( "sou1",         0x0000, 0x0117, CRC(84f4b2fe) SHA1(dcc9e86cc36316fe42eace02d6df75d08bc8bb6d) )

	ROM_REGION( 0x0200, "bboardplds", 0 )
	ROM_LOAD( "stf29.1a",     0x0000, 0x0117, CRC(043309c5) SHA1(f6b9610c9f896c495041ba56a654776a521c356b) )
	ROM_LOAD( "iob1.11d",     0x0000, 0x0117, CRC(3abc0700) SHA1(973043aa46ec6d5d1db20dc9d5937005a0f9f6ae) )

	ROM_REGION( 0x0200, "cboardplds", 0 )
	ROM_LOAD( "c632.ic1",     0x0000, 0x0117, CRC(0fbd9270) SHA1(d7e737b20c44d41e29ca94be56114b31934dde81) )
ROM_END

/* B-Board 90629B-3 */
ROM_START( sf2ui )
	ROM_REGION( CODE_SIZE, "maincpu", 0 )      /* 68000 code */
	ROM_LOAD16_BYTE( "sf2u_30i.11e", 0x00000, 0x20000, CRC(fe39ee33) SHA1(22558eb15e035b09b80935a32b8425d91cd79669) )   // == sf2e_30g.11e
	ROM_LOAD16_BYTE( "sf2u_37i.11f", 0x00001, 0x20000, CRC(9df707dd) SHA1(b148ea450f9e96f3c20f487010a3c57f778e40c1) )
	ROM_LOAD16_BYTE( "sf2u_31i.12e", 0x40000, 0x20000, CRC(69a0a301) SHA1(86a3954335310865b14ce8b4e0e4499feb14fc12) )   // == sf2e_31g.12e
	ROM_LOAD16_BYTE( "sf2u_38i.12f", 0x40001, 0x20000, CRC(4cb46daf) SHA1(dee103ae1391cd9ac150f787187233cd8c06ea1e) )
	ROM_LOAD16_BYTE( "sf2u_28i.9e",  0x80000, 0x20000, CRC(1580be4c) SHA1(d89ed0ff4bf14ff2eaae4609f55970b6b37c8e32) )   /* seen the same pcb with SF2_28I.9E */
	ROM_LOAD16_BYTE( "sf2u_35i.9f",  0x80001, 0x20000, CRC(1468d185) SHA1(750de0cad3859e4917aebb02c2e137dea619f201) )
	ROM_LOAD16_BYTE( "sf2_29b.10e",  0xc0000, 0x20000, CRC(bb4af315) SHA1(75f0827f4f7e9f292add46467f8d4fe19b2514c9) )
	ROM_LOAD16_BYTE( "sf2_36b.10f",  0xc0001, 0x20000, CRC(c02a13eb) SHA1(b807cc495bff3f95d03b061fc629c95f965cb6d8) )

	ROM_REGION( 0x600000, "gfx", 0 )
	ROMX_LOAD( "sf2-5m.4a",  0x000000, 0x80000, CRC(22c9cc8e) SHA1(b9194fb337b30502c1c9501cd6c64ae4035544d4) , ROM_GROUPWORD | ROM_SKIP(6) )    // in "2" socket
	ROMX_LOAD( "sf2-7m.6a",  0x000002, 0x80000, CRC(57213be8) SHA1(3759b851ac0904ec79cbb67a2264d384b6f2f9f9) , ROM_GROUPWORD | ROM_SKIP(6) )    // in "4" socket
	ROMX_LOAD( "sf2-1m.3a",  0x000004, 0x80000, CRC(ba529b4f) SHA1(520840d727161cf09ca784919fa37bc9b54cc3ce) , ROM_GROUPWORD | ROM_SKIP(6) )    // in "1" socket
	ROMX_LOAD( "sf2-3m.5a",  0x000006, 0x80000, CRC(4b1b33a8) SHA1(2360cff890551f76775739e2d6563858bff80e41) , ROM_GROUPWORD | ROM_SKIP(6) )    // in "3" socket
	ROMX_LOAD( "sf2-6m.4c",  0x200000, 0x80000, CRC(2c7e2229) SHA1(357c2275af9133fd0bd6fbb1fa9ad5e0b490b3a2) , ROM_GROUPWORD | ROM_SKIP(6) )    // in "11" socket
	ROMX_LOAD( "sf2-8m.6c",  0x200002, 0x80000, CRC(b5548f17) SHA1(baa92b91cf616bc9e2a8a66adc777ffbf962a51b) , ROM_GROUPWORD | ROM_SKIP(6) )    // in "13" socket
	ROMX_LOAD( "sf2-2m.3c",  0x200004, 0x80000, CRC(14b84312) SHA1(2eea16673e60ba7a10bd4d8f6c217bb2441a5b0e) , ROM_GROUPWORD | ROM_SKIP(6) )    // in "10" socket
	ROMX_LOAD( "sf2-4m.5c",  0x200006, 0x80000, CRC(5e9cd89a) SHA1(f787aab98668d4c2c54fc4ba677c0cb808e4f31e) , ROM_GROUPWORD | ROM_SKIP(6) )    // in "12" socket
	ROMX_LOAD( "sf2-13m.4d", 0x400000, 0x80000, CRC(994bfa58) SHA1(5669b845f624b10e7be56bfc89b76592258ce48b) , ROM_GROUPWORD | ROM_SKIP(6) )    // in "21" socket
	ROMX_LOAD( "sf2-15m.6d", 0x400002, 0x80000, CRC(3e66ad9d) SHA1(9af9df0826988872662753e9717c48d46f2974b0) , ROM_GROUPWORD | ROM_SKIP(6) )    // in "23" socket
	ROMX_LOAD( "sf2-9m.3d",  0x400004, 0x80000, CRC(c1befaa8) SHA1(a6a7f4725e52678cbd8d557285c01cdccb2c2602) , ROM_GROUPWORD | ROM_SKIP(6) )    // in "20" socket
	ROMX_LOAD( "sf2-11m.5d", 0x400006, 0x80000, CRC(0627c831) SHA1(f9a92d614e8877d648449de2612fc8b43c85e4c2) , ROM_GROUPWORD | ROM_SKIP(6) )    // in "22" socket

	ROM_REGION( 0x18000, "audiocpu", 0 ) /* 64k for the audio CPU (+banks) */
	ROM_LOAD( "sf2_9.12a",   0x00000, 0x08000, CRC(a4823a1b) SHA1(7b6bf59dfd578bfbbdb64c27988796783442d659) )
	ROM_CONTINUE(            0x10000, 0x08000 )

	ROM_REGION( 0x40000, "oki", 0 ) /* Samples */
	ROM_LOAD( "sf2_18.11c",  0x00000, 0x20000, CRC(7f162009) SHA1(346bf42992b4c36c593e21901e22c87ae4a7d86d) )
	ROM_LOAD( "sf2_19.12c",  0x20000, 0x20000, CRC(beade53f) SHA1(277c397dc12752719ec6b47d2224750bd1c07f79) )

	ROM_REGION( 0x0200, "aboardplds", 0 )
	ROM_LOAD( "buf1",         0x0000, 0x0117, CRC(eb122de7) SHA1(b26b5bfe258e3e184f069719f9fd008d6b8f6b9b) )
	ROM_LOAD( "ioa1",         0x0000, 0x0117, CRC(59c7ee3b) SHA1(fbb887c5b4f5cb8df77cec710eaac2985bc482a6) )
	ROM_LOAD( "prg1",         0x0000, 0x0117, CRC(f1129744) SHA1(a5300f301c1a08a7da768f0773fa0fe3f683b237) )
	ROM_LOAD( "rom1",         0x0000, 0x0117, CRC(41dc73b9) SHA1(7d4c9f1693c821fbf84e32dd6ef62ddf14967845) )
	ROM_LOAD( "sou1",         0x0000, 0x0117, CRC(84f4b2fe) SHA1(dcc9e86cc36316fe42eace02d6df75d08bc8bb6d) )

	ROM_REGION( 0x0200, "bboardplds", 0 )
	ROM_LOAD( "stf29.1a",     0x0000, 0x0117, CRC(043309c5) SHA1(f6b9610c9f896c495041ba56a654776a521c356b) )
	ROM_LOAD( "iob1.11d",     0x0000, 0x0117, CRC(3abc0700) SHA1(973043aa46ec6d5d1db20dc9d5937005a0f9f6ae) )

	ROM_REGION( 0x0200, "cboardplds", 0 )
	ROM_LOAD( "c632.ic1",     0x0000, 0x0117, CRC(0fbd9270) SHA1(d7e737b20c44d41e29ca94be56114b31934dde81) )
ROM_END

/* B-Board 90629B-3 */
ROM_START( sf2uk )
	ROM_REGION( CODE_SIZE, "maincpu", 0 )      /* 68000 code */
	ROM_LOAD16_BYTE( "sf2u_30k.11e", 0x00000, 0x20000, CRC(8f66076c) SHA1(f9653b36bb5012e6bde5fe3bcade4a6a7a7e7def) )
	ROM_LOAD16_BYTE( "sf2u_37k.11f", 0x00001, 0x20000, CRC(4e1f6a83) SHA1(ee679b79ff3c3165979d3de23e0f668839cf465f) )
	ROM_LOAD16_BYTE( "sf2u_31k.12e", 0x40000, 0x20000, CRC(f9f89f60) SHA1(c3b71482b85c83576518f300be768655412276b0) )
	ROM_LOAD16_BYTE( "sf2u_38k.12f", 0x40001, 0x20000, CRC(6ce0a85a) SHA1(567fd18cd626c94496d9123ecef87dc638f0041a) )
	ROM_LOAD16_BYTE( "sf2u_28k.9e",  0x80000, 0x20000, CRC(8e958f31) SHA1(81359bc988c4e9e375b5bbd960921d425b77f706) )
	ROM_LOAD16_BYTE( "sf2u_35k.9f",  0x80001, 0x20000, CRC(fce76fad) SHA1(66f881ba600c7e6bbe960cfd0772ed16208b79c8) )
	ROM_LOAD16_BYTE( "sf2u_29a.10e", 0xc0000, 0x20000, CRC(bb4af315) SHA1(75f0827f4f7e9f292add46467f8d4fe19b2514c9) )   // == sf2_29b.10e
	ROM_LOAD16_BYTE( "sf2u_36a.10f", 0xc0001, 0x20000, CRC(c02a13eb) SHA1(b807cc495bff3f95d03b061fc629c95f965cb6d8) )   // == sf2_36b.10f

	ROM_REGION( 0x600000, "gfx", 0 )
	ROMX_LOAD( "sf2-5m.4a",  0x000000, 0x80000, CRC(22c9cc8e) SHA1(b9194fb337b30502c1c9501cd6c64ae4035544d4) , ROM_GROUPWORD | ROM_SKIP(6) )    // in "2" socket
	ROMX_LOAD( "sf2-7m.6a",  0x000002, 0x80000, CRC(57213be8) SHA1(3759b851ac0904ec79cbb67a2264d384b6f2f9f9) , ROM_GROUPWORD | ROM_SKIP(6) )    // in "4" socket
	ROMX_LOAD( "sf2-1m.3a",  0x000004, 0x80000, CRC(ba529b4f) SHA1(520840d727161cf09ca784919fa37bc9b54cc3ce) , ROM_GROUPWORD | ROM_SKIP(6) )    // in "1" socket
	ROMX_LOAD( "sf2-3m.5a",  0x000006, 0x80000, CRC(4b1b33a8) SHA1(2360cff890551f76775739e2d6563858bff80e41) , ROM_GROUPWORD | ROM_SKIP(6) )    // in "3" socket
	ROMX_LOAD( "sf2-6m.4c",  0x200000, 0x80000, CRC(2c7e2229) SHA1(357c2275af9133fd0bd6fbb1fa9ad5e0b490b3a2) , ROM_GROUPWORD | ROM_SKIP(6) )    // in "11" socket
	ROMX_LOAD( "sf2-8m.6c",  0x200002, 0x80000, CRC(b5548f17) SHA1(baa92b91cf616bc9e2a8a66adc777ffbf962a51b) , ROM_GROUPWORD | ROM_SKIP(6) )    // in "13" socket
	ROMX_LOAD( "sf2-2m.3c",  0x200004, 0x80000, CRC(14b84312) SHA1(2eea16673e60ba7a10bd4d8f6c217bb2441a5b0e) , ROM_GROUPWORD | ROM_SKIP(6) )    // in "10" socket
	ROMX_LOAD( "sf2-4m.5c",  0x200006, 0x80000, CRC(5e9cd89a) SHA1(f787aab98668d4c2c54fc4ba677c0cb808e4f31e) , ROM_GROUPWORD | ROM_SKIP(6) )    // in "12" socket
	ROMX_LOAD( "sf2-13m.4d", 0x400000, 0x80000, CRC(994bfa58) SHA1(5669b845f624b10e7be56bfc89b76592258ce48b) , ROM_GROUPWORD | ROM_SKIP(6) )    // in "21" socket
	ROMX_LOAD( "sf2-15m.6d", 0x400002, 0x80000, CRC(3e66ad9d) SHA1(9af9df0826988872662753e9717c48d46f2974b0) , ROM_GROUPWORD | ROM_SKIP(6) )    // in "23" socket
	ROMX_LOAD( "sf2-9m.3d",  0x400004, 0x80000, CRC(c1befaa8) SHA1(a6a7f4725e52678cbd8d557285c01cdccb2c2602) , ROM_GROUPWORD | ROM_SKIP(6) )    // in "20" socket
	ROMX_LOAD( "sf2-11m.5d", 0x400006, 0x80000, CRC(0627c831) SHA1(f9a92d614e8877d648449de2612fc8b43c85e4c2) , ROM_GROUPWORD | ROM_SKIP(6) )    // in "22" socket

	ROM_REGION( 0x18000, "audiocpu", 0 ) /* 64k for the audio CPU (+banks) */
	ROM_LOAD( "sf2_09.12a",  0x00000, 0x08000, CRC(a4823a1b) SHA1(7b6bf59dfd578bfbbdb64c27988796783442d659) )   // == sf2_9.12a
	ROM_CONTINUE(            0x10000, 0x08000 )

	ROM_REGION( 0x40000, "oki", 0 ) /* Samples */
	ROM_LOAD( "sf2_18.11c",  0x00000, 0x20000, CRC(7f162009) SHA1(346bf42992b4c36c593e21901e22c87ae4a7d86d) )
	ROM_LOAD( "sf2_19.12c",  0x20000, 0x20000, CRC(beade53f) SHA1(277c397dc12752719ec6b47d2224750bd1c07f79) )

	ROM_REGION( 0x0200, "aboardplds", 0 )
	ROM_LOAD( "buf1",         0x0000, 0x0117, CRC(eb122de7) SHA1(b26b5bfe258e3e184f069719f9fd008d6b8f6b9b) )
	ROM_LOAD( "ioa1",         0x0000, 0x0117, CRC(59c7ee3b) SHA1(fbb887c5b4f5cb8df77cec710eaac2985bc482a6) )
	ROM_LOAD( "prg1",         0x0000, 0x0117, CRC(f1129744) SHA1(a5300f301c1a08a7da768f0773fa0fe3f683b237) )
	ROM_LOAD( "rom1",         0x0000, 0x0117, CRC(41dc73b9) SHA1(7d4c9f1693c821fbf84e32dd6ef62ddf14967845) )
	ROM_LOAD( "sou1",         0x0000, 0x0117, CRC(84f4b2fe) SHA1(dcc9e86cc36316fe42eace02d6df75d08bc8bb6d) )

	ROM_REGION( 0x0200, "bboardplds", 0 )
	ROM_LOAD( "stf29.1a",     0x0000, 0x0117, CRC(043309c5) SHA1(f6b9610c9f896c495041ba56a654776a521c356b) )
	ROM_LOAD( "iob1.11d",     0x0000, 0x0117, CRC(3abc0700) SHA1(973043aa46ec6d5d1db20dc9d5937005a0f9f6ae) )

	ROM_REGION( 0x0200, "cboardplds", 0 )
	ROM_LOAD( "c632.ic1",     0x0000, 0x0117, CRC(0fbd9270) SHA1(d7e737b20c44d41e29ca94be56114b31934dde81) )
ROM_END

/* B-Board 90629B */
ROM_START( sf2j )
	ROM_REGION( CODE_SIZE, "maincpu", 0 )      /* 68000 code */
	ROM_LOAD16_BYTE( "sf2j30.bin",    0x00000, 0x20000, CRC(79022b31) SHA1(b7cfe0498260cdd2779580c47829dd02435ffff4) )
	ROM_LOAD16_BYTE( "sf2j37.bin",    0x00001, 0x20000, CRC(516776ec) SHA1(4f8b63c4d4265a105751fa72b50bd0fa538bf881) )
	ROM_LOAD16_BYTE( "sf2j31.bin",    0x40000, 0x20000, CRC(fe15cb39) SHA1(383478524881ea70d9e04c9b6143b8735b637eee) )
	ROM_LOAD16_BYTE( "sf2j38.bin",    0x40001, 0x20000, CRC(38614d70) SHA1(39c58096f3a8e01fb439639b742b83102bbaa7f6) )
	ROM_LOAD16_BYTE( "sf2j28.bin",    0x80000, 0x20000, CRC(d283187a) SHA1(5ea83d2652e43e46b831b614d1fe06d465bac9a3) )
	ROM_LOAD16_BYTE( "sf2j35.bin",    0x80001, 0x20000, CRC(d28158e4) SHA1(bf2bca6068e374011afa95e99809d262f522df18) )
	ROM_LOAD16_BYTE( "sf2_29a.bin",   0xc0000, 0x20000, CRC(bb4af315) SHA1(75f0827f4f7e9f292add46467f8d4fe19b2514c9) )
	ROM_LOAD16_BYTE( "sf2_36a.bin",   0xc0001, 0x20000, CRC(c02a13eb) SHA1(b807cc495bff3f95d03b061fc629c95f965cb6d8) )

	ROM_REGION( 0x600000, "gfx", 0 )
	ROMX_LOAD( "sf2_06.8a",  0x000000, 0x80000, CRC(22c9cc8e) SHA1(b9194fb337b30502c1c9501cd6c64ae4035544d4) , ROM_GROUPWORD | ROM_SKIP(6) )    // == sf2-5m.4a
	ROMX_LOAD( "sf2_08.10a", 0x000002, 0x80000, CRC(57213be8) SHA1(3759b851ac0904ec79cbb67a2264d384b6f2f9f9) , ROM_GROUPWORD | ROM_SKIP(6) )    // == sf2-7m.6a
	ROMX_LOAD( "sf2_05.7a",  0x000004, 0x80000, CRC(ba529b4f) SHA1(520840d727161cf09ca784919fa37bc9b54cc3ce) , ROM_GROUPWORD | ROM_SKIP(6) )    // == sf2-1m.3a
	ROMX_LOAD( "sf2_07.9a",  0x000006, 0x80000, CRC(4b1b33a8) SHA1(2360cff890551f76775739e2d6563858bff80e41) , ROM_GROUPWORD | ROM_SKIP(6) )    // == sf2-3m.5a
	ROMX_LOAD( "sf2_15.8c",  0x200000, 0x80000, CRC(2c7e2229) SHA1(357c2275af9133fd0bd6fbb1fa9ad5e0b490b3a2) , ROM_GROUPWORD | ROM_SKIP(6) )    // == sf2-6m.4c
	ROMX_LOAD( "sf2_17.10c", 0x200002, 0x80000, CRC(b5548f17) SHA1(baa92b91cf616bc9e2a8a66adc777ffbf962a51b) , ROM_GROUPWORD | ROM_SKIP(6) )    // == sf2-8m.6c
	ROMX_LOAD( "sf2_14.7c",  0x200004, 0x80000, CRC(14b84312) SHA1(2eea16673e60ba7a10bd4d8f6c217bb2441a5b0e) , ROM_GROUPWORD | ROM_SKIP(6) )    // == sf2-2m.3c
	ROMX_LOAD( "sf2_16.9c",  0x200006, 0x80000, CRC(5e9cd89a) SHA1(f787aab98668d4c2c54fc4ba677c0cb808e4f31e) , ROM_GROUPWORD | ROM_SKIP(6) )    // == sf2-4m.5c
	ROMX_LOAD( "sf2_25.8d",  0x400000, 0x80000, CRC(994bfa58) SHA1(5669b845f624b10e7be56bfc89b76592258ce48b) , ROM_GROUPWORD | ROM_SKIP(6) )    // == sf2-13m.4d
	ROMX_LOAD( "sf2_27.10d", 0x400002, 0x80000, CRC(3e66ad9d) SHA1(9af9df0826988872662753e9717c48d46f2974b0) , ROM_GROUPWORD | ROM_SKIP(6) )    // == sf2-15m.6d
	ROMX_LOAD( "sf2_24.7d",  0x400004, 0x80000, CRC(c1befaa8) SHA1(a6a7f4725e52678cbd8d557285c01cdccb2c2602) , ROM_GROUPWORD | ROM_SKIP(6) )    // == sf2-9m.3d
	ROMX_LOAD( "sf2_26.9d",  0x400006, 0x80000, CRC(0627c831) SHA1(f9a92d614e8877d648449de2612fc8b43c85e4c2) , ROM_GROUPWORD | ROM_SKIP(6) )    // == sf2-11m.5d

	ROM_REGION( 0x18000, "audiocpu", 0 ) /* 64k for the audio CPU (+banks) */
	ROM_LOAD( "sf2_09.bin",    0x00000, 0x08000, CRC(a4823a1b) SHA1(7b6bf59dfd578bfbbdb64c27988796783442d659) )
	ROM_CONTINUE(              0x10000, 0x08000 )

	ROM_REGION( 0x40000, "oki", 0 ) /* Samples */
	ROM_LOAD( "sf2_18.bin",    0x00000, 0x20000, CRC(7f162009) SHA1(346bf42992b4c36c593e21901e22c87ae4a7d86d) )
	ROM_LOAD( "sf2_19.bin",    0x20000, 0x20000, CRC(beade53f) SHA1(277c397dc12752719ec6b47d2224750bd1c07f79) )

	ROM_REGION( 0x0200, "aboardplds", 0 )
	ROM_LOAD( "buf1",         0x0000, 0x0117, CRC(eb122de7) SHA1(b26b5bfe258e3e184f069719f9fd008d6b8f6b9b) )
	ROM_LOAD( "ioa1",         0x0000, 0x0117, CRC(59c7ee3b) SHA1(fbb887c5b4f5cb8df77cec710eaac2985bc482a6) )
	ROM_LOAD( "prg1",         0x0000, 0x0117, CRC(f1129744) SHA1(a5300f301c1a08a7da768f0773fa0fe3f683b237) )
	ROM_LOAD( "rom1",         0x0000, 0x0117, CRC(41dc73b9) SHA1(7d4c9f1693c821fbf84e32dd6ef62ddf14967845) )
	ROM_LOAD( "sou1",         0x0000, 0x0117, CRC(84f4b2fe) SHA1(dcc9e86cc36316fe42eace02d6df75d08bc8bb6d) )

	ROM_REGION( 0x0200, "bboardplds", 0 )
	ROM_LOAD( "stf29.1a",     0x0000, 0x0117, CRC(043309c5) SHA1(f6b9610c9f896c495041ba56a654776a521c356b) )
	ROM_LOAD( "iob1.11d",     0x0000, 0x0117, CRC(3abc0700) SHA1(973043aa46ec6d5d1db20dc9d5937005a0f9f6ae) )

	ROM_REGION( 0x0200, "cboardplds", 0 )
	ROM_LOAD( "c632.ic1",     0x0000, 0x0117, CRC(0fbd9270) SHA1(d7e737b20c44d41e29ca94be56114b31934dde81) )
ROM_END

/* B-Board 90629B-2 */
ROM_START( sf2ja )
	ROM_REGION( CODE_SIZE, "maincpu", 0 )      /* 68000 code */
	ROM_LOAD16_BYTE( "sf2j_30a.11e", 0x00000, 0x20000, CRC(57bd7051) SHA1(5e211e75b1649b07723cabc03cf15636dbbae595) )   // == sf2e_30b.11e
	ROM_LOAD16_BYTE( "sf2j_37a.11f", 0x00001, 0x20000, CRC(1e1f6844) SHA1(c80e5ac6a6cea39511c38e31ea55b6cd3888024f) )
	ROM_LOAD16_BYTE( "sf2j_31a.12e", 0x40000, 0x20000, CRC(a673143d) SHA1(e565f0ec23d6deb543c72af5a83f070c07319477) )   // == sf2e_31b.12e
	ROM_LOAD16_BYTE( "sf2j_38a.12f", 0x40001, 0x20000, CRC(4c2ccef7) SHA1(77b119c70c255622b023de25d9af3b3aac52ea47) )   // == sf2e_38b.12f
	ROM_LOAD16_BYTE( "sf2j_28a.9e",  0x80000, 0x20000, CRC(4009955e) SHA1(7842dbef7650485639fbae49b9f4db7494d4f73d) )   // == sf2_28b.9e
	ROM_LOAD16_BYTE( "sf2j_35a.9f",  0x80001, 0x20000, CRC(8c1f3994) SHA1(5e1d334399d05a837c2d80f79eada543e83afaf7) )   // == sf2_35b.9f
	ROM_LOAD16_BYTE( "sf2j_29a.10e", 0xc0000, 0x20000, CRC(bb4af315) SHA1(75f0827f4f7e9f292add46467f8d4fe19b2514c9) )   // == sf2_29b.10e
	ROM_LOAD16_BYTE( "sf2j_36a.10f", 0xc0001, 0x20000, CRC(c02a13eb) SHA1(b807cc495bff3f95d03b061fc629c95f965cb6d8) )   // == sf2_36b.10f

	ROM_REGION( 0x600000, "gfx", 0 )
	ROMX_LOAD( "sf2_06.8a",  0x000000, 0x80000, CRC(22c9cc8e) SHA1(b9194fb337b30502c1c9501cd6c64ae4035544d4) , ROM_GROUPWORD | ROM_SKIP(6) )    // == sf2-5m.4a
	ROMX_LOAD( "sf2_08.10a", 0x000002, 0x80000, CRC(57213be8) SHA1(3759b851ac0904ec79cbb67a2264d384b6f2f9f9) , ROM_GROUPWORD | ROM_SKIP(6) )    // == sf2-7m.6a
	ROMX_LOAD( "sf2_05.7a",  0x000004, 0x80000, CRC(ba529b4f) SHA1(520840d727161cf09ca784919fa37bc9b54cc3ce) , ROM_GROUPWORD | ROM_SKIP(6) )    // == sf2-1m.3a
	ROMX_LOAD( "sf2_07.9a",  0x000006, 0x80000, CRC(4b1b33a8) SHA1(2360cff890551f76775739e2d6563858bff80e41) , ROM_GROUPWORD | ROM_SKIP(6) )    // == sf2-3m.5a
	ROMX_LOAD( "sf2_15.8c",  0x200000, 0x80000, CRC(2c7e2229) SHA1(357c2275af9133fd0bd6fbb1fa9ad5e0b490b3a2) , ROM_GROUPWORD | ROM_SKIP(6) )    // == sf2-6m.4c
	ROMX_LOAD( "sf2_17.10c", 0x200002, 0x80000, CRC(b5548f17) SHA1(baa92b91cf616bc9e2a8a66adc777ffbf962a51b) , ROM_GROUPWORD | ROM_SKIP(6) )    // == sf2-8m.6c
	ROMX_LOAD( "sf2_14.7c",  0x200004, 0x80000, CRC(14b84312) SHA1(2eea16673e60ba7a10bd4d8f6c217bb2441a5b0e) , ROM_GROUPWORD | ROM_SKIP(6) )    // == sf2-2m.3c
	ROMX_LOAD( "sf2_16.9c",  0x200006, 0x80000, CRC(5e9cd89a) SHA1(f787aab98668d4c2c54fc4ba677c0cb808e4f31e) , ROM_GROUPWORD | ROM_SKIP(6) )    // == sf2-4m.5c
	ROMX_LOAD( "sf2_25.8d",  0x400000, 0x80000, CRC(994bfa58) SHA1(5669b845f624b10e7be56bfc89b76592258ce48b) , ROM_GROUPWORD | ROM_SKIP(6) )    // == sf2-13m.4d
	ROMX_LOAD( "sf2_27.10d", 0x400002, 0x80000, CRC(3e66ad9d) SHA1(9af9df0826988872662753e9717c48d46f2974b0) , ROM_GROUPWORD | ROM_SKIP(6) )    // == sf2-15m.6d
	ROMX_LOAD( "sf2_24.7d",  0x400004, 0x80000, CRC(c1befaa8) SHA1(a6a7f4725e52678cbd8d557285c01cdccb2c2602) , ROM_GROUPWORD | ROM_SKIP(6) )    // == sf2-9m.3d
	ROMX_LOAD( "sf2_26.9d",  0x400006, 0x80000, CRC(0627c831) SHA1(f9a92d614e8877d648449de2612fc8b43c85e4c2) , ROM_GROUPWORD | ROM_SKIP(6) )    // == sf2-11m.5d

	ROM_REGION( 0x18000, "audiocpu", 0 ) /* 64k for the audio CPU (+banks) */
	ROM_LOAD( "sf2j_09.12a", 0x00000, 0x08000, CRC(a4823a1b) SHA1(7b6bf59dfd578bfbbdb64c27988796783442d659) )   // == sf2_9.12a
	ROM_CONTINUE(            0x10000, 0x08000 )

	ROM_REGION( 0x40000, "oki", 0 ) /* Samples */
	ROM_LOAD( "sf2j_18.11c", 0x00000, 0x20000, CRC(7f162009) SHA1(346bf42992b4c36c593e21901e22c87ae4a7d86d) )   // == sf2_18.11c
	ROM_LOAD( "sf2j_19.12c", 0x20000, 0x20000, CRC(beade53f) SHA1(277c397dc12752719ec6b47d2224750bd1c07f79) )   // == sf2_19.12c

	ROM_REGION( 0x0200, "aboardplds", 0 )
	ROM_LOAD( "buf1",         0x0000, 0x0117, CRC(eb122de7) SHA1(b26b5bfe258e3e184f069719f9fd008d6b8f6b9b) )
	ROM_LOAD( "ioa1",         0x0000, 0x0117, CRC(59c7ee3b) SHA1(fbb887c5b4f5cb8df77cec710eaac2985bc482a6) )
	ROM_LOAD( "prg1",         0x0000, 0x0117, CRC(f1129744) SHA1(a5300f301c1a08a7da768f0773fa0fe3f683b237) )
	ROM_LOAD( "rom1",         0x0000, 0x0117, CRC(41dc73b9) SHA1(7d4c9f1693c821fbf84e32dd6ef62ddf14967845) )
	ROM_LOAD( "sou1",         0x0000, 0x0117, CRC(84f4b2fe) SHA1(dcc9e86cc36316fe42eace02d6df75d08bc8bb6d) )

	ROM_REGION( 0x0200, "bboardplds", 0 )
	ROM_LOAD( "stf29.1a",     0x0000, 0x0117, CRC(043309c5) SHA1(f6b9610c9f896c495041ba56a654776a521c356b) )
	ROM_LOAD( "iob1.11d",     0x0000, 0x0117, CRC(3abc0700) SHA1(973043aa46ec6d5d1db20dc9d5937005a0f9f6ae) )

	ROM_REGION( 0x0200, "cboardplds", 0 )
	ROM_LOAD( "c632.ic1",     0x0000, 0x0117, CRC(0fbd9270) SHA1(d7e737b20c44d41e29ca94be56114b31934dde81) )
ROM_END

/* B-Board 90629B-3 */
ROM_START( sf2jc )
	ROM_REGION( CODE_SIZE, "maincpu", 0 )      /* 68000 code */
	ROM_LOAD16_BYTE( "sf2j_30c.11e", 0x00000, 0x20000, CRC(8add35ec) SHA1(b08428ff262ca4feddd3c72058b4b674a5401aba) )
	ROM_LOAD16_BYTE( "sf2j_37c.11f", 0x00001, 0x20000, CRC(0d74a256) SHA1(587fd0ee1c2ef54554237486eb5b0d1ec30c2868) )
	ROM_LOAD16_BYTE( "sf2j_31c.12e", 0x40000, 0x20000, CRC(c4fff4a9) SHA1(4b593ace201fe7f5a00b5cd7f4e8fc3f8dd4ceed) )   // == sf2u_31c.12e
	ROM_LOAD16_BYTE( "sf2j_38c.12f", 0x40001, 0x20000, CRC(8210fc0e) SHA1(7d6cfb99afa89d0e6e991d9f7c1808b740def125) )   // == sf2u_38c.12f
	ROM_LOAD16_BYTE( "sf2j_28c.9e",  0x80000, 0x20000, CRC(6eddd5e8) SHA1(62bd1c2fc0321809421c9a592f691b5b1a1d8807) )   // == sf2u_28c.9e
	ROM_LOAD16_BYTE( "sf2j_35c.9f",  0x80001, 0x20000, CRC(6bcb404c) SHA1(b5f24556c633c521aadd94e016d78db6922e3dfa) )   // == sf2u_35c.9f
	ROM_LOAD16_BYTE( "sf2j_29a.10e", 0xc0000, 0x20000, CRC(bb4af315) SHA1(75f0827f4f7e9f292add46467f8d4fe19b2514c9) )   // == sf2_29b.10e
	ROM_LOAD16_BYTE( "sf2j_36a.10f", 0xc0001, 0x20000, CRC(c02a13eb) SHA1(b807cc495bff3f95d03b061fc629c95f965cb6d8) )   // == sf2_36b.10f

	ROM_REGION( 0x600000, "gfx", 0 )
	ROMX_LOAD( "sf2_06.8a",  0x000000, 0x80000, CRC(22c9cc8e) SHA1(b9194fb337b30502c1c9501cd6c64ae4035544d4) , ROM_GROUPWORD | ROM_SKIP(6) )    // == sf2-5m.4a
	ROMX_LOAD( "sf2_08.10a", 0x000002, 0x80000, CRC(57213be8) SHA1(3759b851ac0904ec79cbb67a2264d384b6f2f9f9) , ROM_GROUPWORD | ROM_SKIP(6) )    // == sf2-7m.6a
	ROMX_LOAD( "sf2_05.7a",  0x000004, 0x80000, CRC(ba529b4f) SHA1(520840d727161cf09ca784919fa37bc9b54cc3ce) , ROM_GROUPWORD | ROM_SKIP(6) )    // == sf2-1m.3a
	ROMX_LOAD( "sf2_07.9a",  0x000006, 0x80000, CRC(4b1b33a8) SHA1(2360cff890551f76775739e2d6563858bff80e41) , ROM_GROUPWORD | ROM_SKIP(6) )    // == sf2-3m.5a
	ROMX_LOAD( "sf2_15.8c",  0x200000, 0x80000, CRC(2c7e2229) SHA1(357c2275af9133fd0bd6fbb1fa9ad5e0b490b3a2) , ROM_GROUPWORD | ROM_SKIP(6) )    // == sf2-6m.4c
	ROMX_LOAD( "sf2_17.10c", 0x200002, 0x80000, CRC(b5548f17) SHA1(baa92b91cf616bc9e2a8a66adc777ffbf962a51b) , ROM_GROUPWORD | ROM_SKIP(6) )    // == sf2-8m.6c
	ROMX_LOAD( "sf2_14.7c",  0x200004, 0x80000, CRC(14b84312) SHA1(2eea16673e60ba7a10bd4d8f6c217bb2441a5b0e) , ROM_GROUPWORD | ROM_SKIP(6) )    // == sf2-2m.3c
	ROMX_LOAD( "sf2_16.9c",  0x200006, 0x80000, CRC(5e9cd89a) SHA1(f787aab98668d4c2c54fc4ba677c0cb808e4f31e) , ROM_GROUPWORD | ROM_SKIP(6) )    // == sf2-4m.5c
	ROMX_LOAD( "sf2_25.8d",  0x400000, 0x80000, CRC(994bfa58) SHA1(5669b845f624b10e7be56bfc89b76592258ce48b) , ROM_GROUPWORD | ROM_SKIP(6) )    // == sf2-13m.4d
	ROMX_LOAD( "sf2_27.10d", 0x400002, 0x80000, CRC(3e66ad9d) SHA1(9af9df0826988872662753e9717c48d46f2974b0) , ROM_GROUPWORD | ROM_SKIP(6) )    // == sf2-15m.6d
	ROMX_LOAD( "sf2_24.7d",  0x400004, 0x80000, CRC(c1befaa8) SHA1(a6a7f4725e52678cbd8d557285c01cdccb2c2602) , ROM_GROUPWORD | ROM_SKIP(6) )    // == sf2-9m.3d
	ROMX_LOAD( "sf2_26.9d",  0x400006, 0x80000, CRC(0627c831) SHA1(f9a92d614e8877d648449de2612fc8b43c85e4c2) , ROM_GROUPWORD | ROM_SKIP(6) )    // == sf2-11m.5d

	ROM_REGION( 0x18000, "audiocpu", 0 ) /* 64k for the audio CPU (+banks) */
	ROM_LOAD( "sf2_09.12a",  0x00000, 0x08000, CRC(a4823a1b) SHA1(7b6bf59dfd578bfbbdb64c27988796783442d659) )   // == sf2_9.12a
	ROM_CONTINUE(            0x10000, 0x08000 )

	ROM_REGION( 0x40000, "oki", 0 ) /* Samples */
	ROM_LOAD( "sf2j_18.11c", 0x00000, 0x20000, CRC(7f162009) SHA1(346bf42992b4c36c593e21901e22c87ae4a7d86d) )   // == sf2_18.11c
	ROM_LOAD( "sf2j_19.12c", 0x20000, 0x20000, CRC(beade53f) SHA1(277c397dc12752719ec6b47d2224750bd1c07f79) )   // == sf2_19.12c

	ROM_REGION( 0x0200, "aboardplds", 0 )
	ROM_LOAD( "buf1",         0x0000, 0x0117, CRC(eb122de7) SHA1(b26b5bfe258e3e184f069719f9fd008d6b8f6b9b) )
	ROM_LOAD( "ioa1",         0x0000, 0x0117, CRC(59c7ee3b) SHA1(fbb887c5b4f5cb8df77cec710eaac2985bc482a6) )
	ROM_LOAD( "prg1",         0x0000, 0x0117, CRC(f1129744) SHA1(a5300f301c1a08a7da768f0773fa0fe3f683b237) )
	ROM_LOAD( "rom1",         0x0000, 0x0117, CRC(41dc73b9) SHA1(7d4c9f1693c821fbf84e32dd6ef62ddf14967845) )
	ROM_LOAD( "sou1",         0x0000, 0x0117, CRC(84f4b2fe) SHA1(dcc9e86cc36316fe42eace02d6df75d08bc8bb6d) )

	ROM_REGION( 0x0200, "bboardplds", 0 )
	ROM_LOAD( "stf29.1a",     0x0000, 0x0117, CRC(043309c5) SHA1(f6b9610c9f896c495041ba56a654776a521c356b) )
	ROM_LOAD( "iob1.11d",     0x0000, 0x0117, CRC(3abc0700) SHA1(973043aa46ec6d5d1db20dc9d5937005a0f9f6ae) )

	ROM_REGION( 0x0200, "cboardplds", 0 )
	ROM_LOAD( "c632.ic1",     0x0000, 0x0117, CRC(0fbd9270) SHA1(d7e737b20c44d41e29ca94be56114b31934dde81) )
ROM_END

/* B-Board 90629B-3 */
ROM_START( sf2jf )
	ROM_REGION( CODE_SIZE, "maincpu", 0 )      /* 68000 code */
	ROM_LOAD16_BYTE( "sf2j_30f.11e", 0x00000, 0x20000, CRC(fe39ee33) SHA1(22558eb15e035b09b80935a32b8425d91cd79669) )   // == sf2e_30g.11e
	ROM_LOAD16_BYTE( "sf2j_37f.11f", 0x00001, 0x20000, CRC(c1428cc6) SHA1(6a3f21de57c3dd60a7c991ca667dc2a77d813039) )
	ROM_LOAD16_BYTE( "sf2j_31f.12e", 0x40000, 0x20000, CRC(69a0a301) SHA1(86a3954335310865b14ce8b4e0e4499feb14fc12) )   // == sf2e_31g.12e
	ROM_LOAD16_BYTE( "sf2j_38f.12f", 0x40001, 0x20000, CRC(1510e4e2) SHA1(fbfdd4e42c4bc894592dbe5a84c88d5f13d21da4) )
	ROM_LOAD16_BYTE( "sf2j_28f.9e",  0x80000, 0x20000, CRC(acd8175b) SHA1(504991c46fa568d31ce69bd63e2a67926a06b5a9) )
	ROM_LOAD16_BYTE( "sf2j_35f.9f",  0x80001, 0x20000, CRC(c0a80bd1) SHA1(ac25a9ed488e03baf4115541fdcce3973ce6a442) )
	ROM_LOAD16_BYTE( "sf2j_29a.10e", 0xc0000, 0x20000, CRC(bb4af315) SHA1(75f0827f4f7e9f292add46467f8d4fe19b2514c9) )   // == sf2_29b.10e
	ROM_LOAD16_BYTE( "sf2j_36a.10f", 0xc0001, 0x20000, CRC(c02a13eb) SHA1(b807cc495bff3f95d03b061fc629c95f965cb6d8) )   // == sf2_36b.10f

	ROM_REGION( 0x600000, "gfx", 0 )
	ROMX_LOAD( "sf2_06.8a",  0x000000, 0x80000, CRC(22c9cc8e) SHA1(b9194fb337b30502c1c9501cd6c64ae4035544d4) , ROM_GROUPWORD | ROM_SKIP(6) )    // == sf2-5m.4a
	ROMX_LOAD( "sf2_08.10a", 0x000002, 0x80000, CRC(57213be8) SHA1(3759b851ac0904ec79cbb67a2264d384b6f2f9f9) , ROM_GROUPWORD | ROM_SKIP(6) )    // == sf2-7m.6a
	ROMX_LOAD( "sf2_05.7a",  0x000004, 0x80000, CRC(ba529b4f) SHA1(520840d727161cf09ca784919fa37bc9b54cc3ce) , ROM_GROUPWORD | ROM_SKIP(6) )    // == sf2-1m.3a
	ROMX_LOAD( "sf2_07.9a",  0x000006, 0x80000, CRC(4b1b33a8) SHA1(2360cff890551f76775739e2d6563858bff80e41) , ROM_GROUPWORD | ROM_SKIP(6) )    // == sf2-3m.5a
	ROMX_LOAD( "sf2_15.8c",  0x200000, 0x80000, CRC(2c7e2229) SHA1(357c2275af9133fd0bd6fbb1fa9ad5e0b490b3a2) , ROM_GROUPWORD | ROM_SKIP(6) )    // == sf2-6m.4c
	ROMX_LOAD( "sf2_17.10c", 0x200002, 0x80000, CRC(b5548f17) SHA1(baa92b91cf616bc9e2a8a66adc777ffbf962a51b) , ROM_GROUPWORD | ROM_SKIP(6) )    // == sf2-8m.6c
	ROMX_LOAD( "sf2_14.7c",  0x200004, 0x80000, CRC(14b84312) SHA1(2eea16673e60ba7a10bd4d8f6c217bb2441a5b0e) , ROM_GROUPWORD | ROM_SKIP(6) )    // == sf2-2m.3c
	ROMX_LOAD( "sf2_16.9c",  0x200006, 0x80000, CRC(5e9cd89a) SHA1(f787aab98668d4c2c54fc4ba677c0cb808e4f31e) , ROM_GROUPWORD | ROM_SKIP(6) )    // == sf2-4m.5c
	ROMX_LOAD( "sf2_25.8d",  0x400000, 0x80000, CRC(994bfa58) SHA1(5669b845f624b10e7be56bfc89b76592258ce48b) , ROM_GROUPWORD | ROM_SKIP(6) )    // == sf2-13m.4d
	ROMX_LOAD( "sf2_27.10d", 0x400002, 0x80000, CRC(3e66ad9d) SHA1(9af9df0826988872662753e9717c48d46f2974b0) , ROM_GROUPWORD | ROM_SKIP(6) )    // == sf2-15m.6d
	ROMX_LOAD( "sf2_24.7d",  0x400004, 0x80000, CRC(c1befaa8) SHA1(a6a7f4725e52678cbd8d557285c01cdccb2c2602) , ROM_GROUPWORD | ROM_SKIP(6) )    // == sf2-9m.3d
	ROMX_LOAD( "sf2_26.9d",  0x400006, 0x80000, CRC(0627c831) SHA1(f9a92d614e8877d648449de2612fc8b43c85e4c2) , ROM_GROUPWORD | ROM_SKIP(6) )    // == sf2-11m.5d

	ROM_REGION( 0x18000, "audiocpu", 0 ) /* 64k for the audio CPU (+banks) */
	ROM_LOAD( "sf2_09.12a",  0x00000, 0x08000, CRC(a4823a1b) SHA1(7b6bf59dfd578bfbbdb64c27988796783442d659) )   // == sf2_9.12a
	ROM_CONTINUE(            0x10000, 0x08000 )

	ROM_REGION( 0x40000, "oki", 0 ) /* Samples */
	ROM_LOAD( "sf2_18.11c",  0x00000, 0x20000, CRC(7f162009) SHA1(346bf42992b4c36c593e21901e22c87ae4a7d86d) )
	ROM_LOAD( "sf2_19.12c",  0x20000, 0x20000, CRC(beade53f) SHA1(277c397dc12752719ec6b47d2224750bd1c07f79) )

	ROM_REGION( 0x0200, "aboardplds", 0 )
	ROM_LOAD( "buf1",         0x0000, 0x0117, CRC(eb122de7) SHA1(b26b5bfe258e3e184f069719f9fd008d6b8f6b9b) )
	ROM_LOAD( "ioa1",         0x0000, 0x0117, CRC(59c7ee3b) SHA1(fbb887c5b4f5cb8df77cec710eaac2985bc482a6) )
	ROM_LOAD( "prg1",         0x0000, 0x0117, CRC(f1129744) SHA1(a5300f301c1a08a7da768f0773fa0fe3f683b237) )
	ROM_LOAD( "rom1",         0x0000, 0x0117, CRC(41dc73b9) SHA1(7d4c9f1693c821fbf84e32dd6ef62ddf14967845) )
	ROM_LOAD( "sou1",         0x0000, 0x0117, CRC(84f4b2fe) SHA1(dcc9e86cc36316fe42eace02d6df75d08bc8bb6d) )

	ROM_REGION( 0x0200, "bboardplds", 0 )
	ROM_LOAD( "stf29.1a",     0x0000, 0x0117, CRC(043309c5) SHA1(f6b9610c9f896c495041ba56a654776a521c356b) )
	ROM_LOAD( "iob1.11d",     0x0000, 0x0117, CRC(3abc0700) SHA1(973043aa46ec6d5d1db20dc9d5937005a0f9f6ae) )

	ROM_REGION( 0x0200, "cboardplds", 0 )
	ROM_LOAD( "c632.ic1",     0x0000, 0x0117, CRC(0fbd9270) SHA1(d7e737b20c44d41e29ca94be56114b31934dde81) )
ROM_END

/* B-Board 90629B-3 */
ROM_START( sf2jh )
	ROM_REGION( CODE_SIZE, "maincpu", 0 )      /* 68000 code */
	ROM_LOAD16_BYTE( "sf2j_30h.11e", 0x00000, 0x20000, CRC(fe39ee33) SHA1(22558eb15e035b09b80935a32b8425d91cd79669) )   // == sf2e_30g.11e
	ROM_LOAD16_BYTE( "sf2j_37h.11f", 0x00001, 0x20000, CRC(330304b0) SHA1(93bf761804228a79d16afa2fcfbe28e6942dff51) )
	ROM_LOAD16_BYTE( "sf2j_31h.12e", 0x40000, 0x20000, CRC(69a0a301) SHA1(86a3954335310865b14ce8b4e0e4499feb14fc12) )   // == sf2e_31g.12e
	ROM_LOAD16_BYTE( "sf2j_38h.12f", 0x40001, 0x20000, CRC(a659f678) SHA1(f3b99ebaa59edb889498cf5c9d7ceb939da1dedc) )
	ROM_LOAD16_BYTE( "sf2j_28h.9e",  0x80000, 0x20000, CRC(8a5c8ee0) SHA1(a6df46e96bf7596665177b18213a3aee4cc7c378) )
	ROM_LOAD16_BYTE( "sf2j_35h.9f",  0x80001, 0x20000, CRC(c828fc4d) SHA1(0025630bef45862d8b72a2d19a6c5adf96905bd5) )
	ROM_LOAD16_BYTE( "sf2j_29a.10e", 0xc0000, 0x20000, CRC(bb4af315) SHA1(75f0827f4f7e9f292add46467f8d4fe19b2514c9) )   // == sf2_29b.10e
	ROM_LOAD16_BYTE( "sf2j_36a.10f", 0xc0001, 0x20000, CRC(c02a13eb) SHA1(b807cc495bff3f95d03b061fc629c95f965cb6d8) )   // == sf2_36b.10f

	ROM_REGION( 0x600000, "gfx", 0 )
	ROMX_LOAD( "sf2_06.8a",  0x000000, 0x80000, CRC(22c9cc8e) SHA1(b9194fb337b30502c1c9501cd6c64ae4035544d4) , ROM_GROUPWORD | ROM_SKIP(6) )    // == sf2-5m.4a
	ROMX_LOAD( "sf2_08.10a", 0x000002, 0x80000, CRC(57213be8) SHA1(3759b851ac0904ec79cbb67a2264d384b6f2f9f9) , ROM_GROUPWORD | ROM_SKIP(6) )    // == sf2-7m.6a
	ROMX_LOAD( "sf2_05.7a",  0x000004, 0x80000, CRC(ba529b4f) SHA1(520840d727161cf09ca784919fa37bc9b54cc3ce) , ROM_GROUPWORD | ROM_SKIP(6) )    // == sf2-1m.3a
	ROMX_LOAD( "sf2_07.9a",  0x000006, 0x80000, CRC(4b1b33a8) SHA1(2360cff890551f76775739e2d6563858bff80e41) , ROM_GROUPWORD | ROM_SKIP(6) )    // == sf2-3m.5a
	ROMX_LOAD( "sf2_15.8c",  0x200000, 0x80000, CRC(2c7e2229) SHA1(357c2275af9133fd0bd6fbb1fa9ad5e0b490b3a2) , ROM_GROUPWORD | ROM_SKIP(6) )    // == sf2-6m.4c
	ROMX_LOAD( "sf2_17.10c", 0x200002, 0x80000, CRC(b5548f17) SHA1(baa92b91cf616bc9e2a8a66adc777ffbf962a51b) , ROM_GROUPWORD | ROM_SKIP(6) )    // == sf2-8m.6c
	ROMX_LOAD( "sf2_14.7c",  0x200004, 0x80000, CRC(14b84312) SHA1(2eea16673e60ba7a10bd4d8f6c217bb2441a5b0e) , ROM_GROUPWORD | ROM_SKIP(6) )    // == sf2-2m.3c
	ROMX_LOAD( "sf2_16.9c",  0x200006, 0x80000, CRC(5e9cd89a) SHA1(f787aab98668d4c2c54fc4ba677c0cb808e4f31e) , ROM_GROUPWORD | ROM_SKIP(6) )    // == sf2-4m.5c
	ROMX_LOAD( "sf2_25.8d",  0x400000, 0x80000, CRC(994bfa58) SHA1(5669b845f624b10e7be56bfc89b76592258ce48b) , ROM_GROUPWORD | ROM_SKIP(6) )    // == sf2-13m.4d
	ROMX_LOAD( "sf2_27.10d", 0x400002, 0x80000, CRC(3e66ad9d) SHA1(9af9df0826988872662753e9717c48d46f2974b0) , ROM_GROUPWORD | ROM_SKIP(6) )    // == sf2-15m.6d
	ROMX_LOAD( "sf2_24.7d",  0x400004, 0x80000, CRC(c1befaa8) SHA1(a6a7f4725e52678cbd8d557285c01cdccb2c2602) , ROM_GROUPWORD | ROM_SKIP(6) )    // == sf2-9m.3d
	ROMX_LOAD( "sf2_26.9d",  0x400006, 0x80000, CRC(0627c831) SHA1(f9a92d614e8877d648449de2612fc8b43c85e4c2) , ROM_GROUPWORD | ROM_SKIP(6) )    // == sf2-11m.5d

	ROM_REGION( 0x18000, "audiocpu", 0 ) /* 64k for the audio CPU (+banks) */
	ROM_LOAD( "sf2_09.12a",  0x00000, 0x08000, CRC(a4823a1b) SHA1(7b6bf59dfd578bfbbdb64c27988796783442d659) )   // == sf2_9.12a
	ROM_CONTINUE(            0x10000, 0x08000 )

	ROM_REGION( 0x40000, "oki", 0 ) /* Samples */
	ROM_LOAD( "sf2_18.11c",  0x00000, 0x20000, CRC(7f162009) SHA1(346bf42992b4c36c593e21901e22c87ae4a7d86d) )
	ROM_LOAD( "sf2_19.12c",  0x20000, 0x20000, CRC(beade53f) SHA1(277c397dc12752719ec6b47d2224750bd1c07f79) )

	ROM_REGION( 0x0200, "aboardplds", 0 )
	ROM_LOAD( "buf1",         0x0000, 0x0117, CRC(eb122de7) SHA1(b26b5bfe258e3e184f069719f9fd008d6b8f6b9b) )
	ROM_LOAD( "ioa1",         0x0000, 0x0117, CRC(59c7ee3b) SHA1(fbb887c5b4f5cb8df77cec710eaac2985bc482a6) )
	ROM_LOAD( "prg1",         0x0000, 0x0117, CRC(f1129744) SHA1(a5300f301c1a08a7da768f0773fa0fe3f683b237) )
	ROM_LOAD( "rom1",         0x0000, 0x0117, CRC(41dc73b9) SHA1(7d4c9f1693c821fbf84e32dd6ef62ddf14967845) )
	ROM_LOAD( "sou1",         0x0000, 0x0117, CRC(84f4b2fe) SHA1(dcc9e86cc36316fe42eace02d6df75d08bc8bb6d) )

	ROM_REGION( 0x0200, "bboardplds", 0 )
	ROM_LOAD( "stf29.1a",     0x0000, 0x0117, CRC(043309c5) SHA1(f6b9610c9f896c495041ba56a654776a521c356b) )
	ROM_LOAD( "iob1.11d",     0x0000, 0x0117, CRC(3abc0700) SHA1(973043aa46ec6d5d1db20dc9d5937005a0f9f6ae) )

	ROM_REGION( 0x0200, "cboardplds", 0 )
	ROM_LOAD( "c632.ic1",     0x0000, 0x0117, CRC(0fbd9270) SHA1(d7e737b20c44d41e29ca94be56114b31934dde81) )
ROM_END

/* B-Board 90629B-2 */
ROM_START( sf2jl )
	ROM_REGION( CODE_SIZE, "maincpu", 0 )      /* 68000 code */
	ROM_LOAD16_BYTE( "sf-2_30l.11e", 0x00000, 0x20000, CRC(34a1ce02) SHA1(6875813c49a92b8650f444c1857459adc86bc0d0) )   // These "L" revision roms are verified as being
	ROM_LOAD16_BYTE( "sf-2_37l.11f", 0x00001, 0x20000, CRC(5b630ed2) SHA1(b14ef94b86f55c82391db8aca4d6f9487b9758a7) )   // labeled "SF-2" without "J" for Japan region
	ROM_LOAD16_BYTE( "sf-2_31l.12e", 0x40000, 0x20000, CRC(64ebc8d2) SHA1(1817de5dcca47a1bb94838d26a9a3adf6b75c911) )
	ROM_LOAD16_BYTE( "sf-2_38l.12f", 0x40001, 0x20000, CRC(73847443) SHA1(72fb5472a04061e4a50cb57e4f65ba8ec022a965) )
	ROM_LOAD16_BYTE( "sf-2_28l.9e",  0x80000, 0x20000, CRC(eee2b426) SHA1(1d7b5073b9473d3c92fc95c0d0625716c82fc944) )
	ROM_LOAD16_BYTE( "sf-2_35l.9f",  0x80001, 0x20000, CRC(eca8b452) SHA1(f3d06bd32ef2649a8278d8a0517372067e3f99b7) )
	ROM_LOAD16_BYTE( "sf2j_29a.10e", 0xc0000, 0x20000, CRC(bb4af315) SHA1(75f0827f4f7e9f292add46467f8d4fe19b2514c9) )   // == sf2_29b.10e
	ROM_LOAD16_BYTE( "sf2j_36a.10f", 0xc0001, 0x20000, CRC(c02a13eb) SHA1(b807cc495bff3f95d03b061fc629c95f965cb6d8) )   // == sf2_36b.10f

	ROM_REGION( 0x600000, "gfx", 0 )
	ROMX_LOAD( "sf2_06.8a",  0x000000, 0x80000, CRC(22c9cc8e) SHA1(b9194fb337b30502c1c9501cd6c64ae4035544d4) , ROM_GROUPWORD | ROM_SKIP(6) )    // == sf2-5m.4a
	ROMX_LOAD( "sf2_08.10a", 0x000002, 0x80000, CRC(57213be8) SHA1(3759b851ac0904ec79cbb67a2264d384b6f2f9f9) , ROM_GROUPWORD | ROM_SKIP(6) )    // == sf2-7m.6a
	ROMX_LOAD( "sf2_05.7a",  0x000004, 0x80000, CRC(ba529b4f) SHA1(520840d727161cf09ca784919fa37bc9b54cc3ce) , ROM_GROUPWORD | ROM_SKIP(6) )    // == sf2-1m.3a
	ROMX_LOAD( "sf2_07.9a",  0x000006, 0x80000, CRC(4b1b33a8) SHA1(2360cff890551f76775739e2d6563858bff80e41) , ROM_GROUPWORD | ROM_SKIP(6) )    // == sf2-3m.5a
	ROMX_LOAD( "sf2_15.8c",  0x200000, 0x80000, CRC(2c7e2229) SHA1(357c2275af9133fd0bd6fbb1fa9ad5e0b490b3a2) , ROM_GROUPWORD | ROM_SKIP(6) )    // == sf2-6m.4c
	ROMX_LOAD( "sf2_17.10c", 0x200002, 0x80000, CRC(b5548f17) SHA1(baa92b91cf616bc9e2a8a66adc777ffbf962a51b) , ROM_GROUPWORD | ROM_SKIP(6) )    // == sf2-8m.6c
	ROMX_LOAD( "sf2_14.7c",  0x200004, 0x80000, CRC(14b84312) SHA1(2eea16673e60ba7a10bd4d8f6c217bb2441a5b0e) , ROM_GROUPWORD | ROM_SKIP(6) )    // == sf2-2m.3c
	ROMX_LOAD( "sf2_16.9c",  0x200006, 0x80000, CRC(5e9cd89a) SHA1(f787aab98668d4c2c54fc4ba677c0cb808e4f31e) , ROM_GROUPWORD | ROM_SKIP(6) )    // == sf2-4m.5c
	ROMX_LOAD( "sf2_25.8d",  0x400000, 0x80000, CRC(994bfa58) SHA1(5669b845f624b10e7be56bfc89b76592258ce48b) , ROM_GROUPWORD | ROM_SKIP(6) )    // == sf2-13m.4d
	ROMX_LOAD( "sf2_27.10d", 0x400002, 0x80000, CRC(3e66ad9d) SHA1(9af9df0826988872662753e9717c48d46f2974b0) , ROM_GROUPWORD | ROM_SKIP(6) )    // == sf2-15m.6d
	ROMX_LOAD( "sf2_24.7d",  0x400004, 0x80000, CRC(c1befaa8) SHA1(a6a7f4725e52678cbd8d557285c01cdccb2c2602) , ROM_GROUPWORD | ROM_SKIP(6) )    // == sf2-9m.3d
	ROMX_LOAD( "sf2_26.9d",  0x400006, 0x80000, CRC(0627c831) SHA1(f9a92d614e8877d648449de2612fc8b43c85e4c2) , ROM_GROUPWORD | ROM_SKIP(6) )    // == sf2-11m.5d

	ROM_REGION( 0x18000, "audiocpu", 0 ) /* 64k for the audio CPU (+banks) */
	ROM_LOAD( "sf2j_09.12a", 0x00000, 0x08000, CRC(a4823a1b) SHA1(7b6bf59dfd578bfbbdb64c27988796783442d659) )   // == sf2_9.12a
	ROM_CONTINUE(            0x10000, 0x08000 )

	ROM_REGION( 0x40000, "oki", 0 ) /* Samples */
	ROM_LOAD( "sf2j_18.11c", 0x00000, 0x20000, CRC(7f162009) SHA1(346bf42992b4c36c593e21901e22c87ae4a7d86d) )   // == sf2_18.11c
	ROM_LOAD( "sf2j_19.12c", 0x20000, 0x20000, CRC(beade53f) SHA1(277c397dc12752719ec6b47d2224750bd1c07f79) )   // == sf2_19.12c

	ROM_REGION( 0x0200, "aboardplds", 0 )
	ROM_LOAD( "buf1",         0x0000, 0x0117, CRC(eb122de7) SHA1(b26b5bfe258e3e184f069719f9fd008d6b8f6b9b) )
	ROM_LOAD( "ioa1",         0x0000, 0x0117, CRC(59c7ee3b) SHA1(fbb887c5b4f5cb8df77cec710eaac2985bc482a6) )
	ROM_LOAD( "prg1",         0x0000, 0x0117, CRC(f1129744) SHA1(a5300f301c1a08a7da768f0773fa0fe3f683b237) )
	ROM_LOAD( "rom1",         0x0000, 0x0117, CRC(41dc73b9) SHA1(7d4c9f1693c821fbf84e32dd6ef62ddf14967845) )
	ROM_LOAD( "sou1",         0x0000, 0x0117, CRC(84f4b2fe) SHA1(dcc9e86cc36316fe42eace02d6df75d08bc8bb6d) )

	ROM_REGION( 0x0200, "bboardplds", 0 )
	ROM_LOAD( "stf29.1a",     0x0000, 0x0117, CRC(043309c5) SHA1(f6b9610c9f896c495041ba56a654776a521c356b) )
	ROM_LOAD( "iob1.11d",     0x0000, 0x0117, CRC(3abc0700) SHA1(973043aa46ec6d5d1db20dc9d5937005a0f9f6ae) )

	ROM_REGION( 0x0200, "cboardplds", 0 )
	ROM_LOAD( "c632.ic1",     0x0000, 0x0117, CRC(0fbd9270) SHA1(d7e737b20c44d41e29ca94be56114b31934dde81) )
ROM_END

/*  Bootleg manufactured by TAB Austria

 -------------------------------------------------------
 |    ROM1    *2  *3                         ROM5 ROM9 |
 |    ROM2                                   ROM6 ROM8 |
 |            *1                              1B   1D  |
 |      6295                                  1A   1C  |
 |  YM                               ROM4     1F   1H  |
 |    ROM3                                    1E   1G  |
--     Z80                                    1J   1L  |
--                                            1I   1K  |
--                                                     |
--                                                     |
--                     TAB                             |
--                    AUSTRIA                          |
 | DIP8 6                                              |
 |      8  ROM10  ROM9                                 |
 | DIP8 0  ROM12  ROM11                                |
 |      0                                              |
 | DIP8 0                                              |
 |------------------------------------------------------

   *1 = 12.000 Mhz
   *2 = 3.579545 Mhz
   *3 = 16.000 Mhz

*/
ROM_START( sf2ebbl )
	ROM_REGION( CODE_SIZE, "maincpu", 0 )      /* 68000 code */
	ROM_LOAD16_BYTE( "12.bin",   0x00000, 0x40000, CRC(a258b4d5) SHA1(3433b6493794c98bb35c1b27cc65bb5f13d52e9b) )
	ROM_LOAD16_BYTE( "09.bin",   0x00001, 0x40000, CRC(59ccd474) SHA1(7bb28c28ee722435fdbb18eb73e52bd65b419103) )
	ROM_LOAD16_BYTE( "11.bin",   0x80000, 0x40000, CRC(82097d63) SHA1(881e7ffb78197f6794b5d41f5c2c87da35e8cb15) )
	ROM_LOAD16_BYTE( "10.bin",   0x80001, 0x40000, CRC(0c83844d) SHA1(4c25ba4a50d62c62789d026e3d304ed1dfb3c248) )

	ROM_REGION( 0x600000, "gfx", 0 )
	/* The 12 MASK roms on this PCB match the original roms exactly */
	ROMX_LOAD( "1b_yf082.bin", 0x000000, 0x80000, CRC(22c9cc8e) SHA1(b9194fb337b30502c1c9501cd6c64ae4035544d4) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "1d_yf028.bin", 0x000002, 0x80000, CRC(57213be8) SHA1(3759b851ac0904ec79cbb67a2264d384b6f2f9f9) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "1a_yf087.bin", 0x000004, 0x80000, CRC(ba529b4f) SHA1(520840d727161cf09ca784919fa37bc9b54cc3ce) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "1c_yf088.bin", 0x000006, 0x80000, CRC(4b1b33a8) SHA1(2360cff890551f76775739e2d6563858bff80e41) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "1f_yf085.bin", 0x200000, 0x80000, CRC(2c7e2229) SHA1(357c2275af9133fd0bd6fbb1fa9ad5e0b490b3a2) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "1h_yf115.bin", 0x200002, 0x80000, CRC(b5548f17) SHA1(baa92b91cf616bc9e2a8a66adc777ffbf962a51b) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "1e_yf111.bin", 0x200004, 0x80000, CRC(14b84312) SHA1(2eea16673e60ba7a10bd4d8f6c217bb2441a5b0e) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "1g_yf002.bin", 0x200006, 0x80000, CRC(5e9cd89a) SHA1(f787aab98668d4c2c54fc4ba677c0cb808e4f31e) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "1j_yf117.bin", 0x400000, 0x80000, CRC(994bfa58) SHA1(5669b845f624b10e7be56bfc89b76592258ce48b) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "1l_ye040.bin", 0x400002, 0x80000, CRC(3e66ad9d) SHA1(9af9df0826988872662753e9717c48d46f2974b0) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "1i_yf038.bin", 0x400004, 0x80000, CRC(c1befaa8) SHA1(a6a7f4725e52678cbd8d557285c01cdccb2c2602) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "1k_ye039.bin", 0x400006, 0x80000, CRC(0627c831) SHA1(f9a92d614e8877d648449de2612fc8b43c85e4c2) , ROM_GROUPWORD | ROM_SKIP(6) )
	/* These map over the MASK roms on this bootleg to get rid of the CAPCOM logo (wasteful, but correct) */
	ROMX_LOAD( "05.bin", 0x400000, 0x10000, CRC(a505621e) SHA1(8ffa8cedad54948870bbd8f629d927332dc9fcf6) , ROM_SKIP(7) )
	ROM_CONTINUE(        0x400004, 0x10000 )
	ROMX_LOAD( "07.bin", 0x400002, 0x10000, CRC(de6271fb) SHA1(574ec5d9992941a405fd00abe52da41aba4b29a7) , ROM_SKIP(7) )
	ROM_CONTINUE(        0x400006, 0x10000 )
	ROMX_LOAD( "06.bin", 0x400001, 0x10000, CRC(23775344) SHA1(0d6b54f66cd64c3f48c88c8e17a19fdb2002afb0) , ROM_SKIP(7) )
	ROM_CONTINUE(        0x400005, 0x10000 )
	ROMX_LOAD( "08.bin", 0x400003, 0x10000, CRC(81c9550f) SHA1(2d75e329148caadfff35c8f2f91b352f14dbe08a) , ROM_SKIP(7) )
	ROM_CONTINUE(        0x400007, 0x10000 )

	ROM_REGION( 0x18000, "audiocpu", 0 ) /* 64k for the audio CPU (+banks) */
	ROM_LOAD( "03.bin",    0x00000, 0x08000, CRC(a4823a1b) SHA1(7b6bf59dfd578bfbbdb64c27988796783442d659) )
	ROM_CONTINUE(              0x10000, 0x08000 )

	ROM_REGION( 0x20000, "user1", 0 ) /* unknown (bootleg priority?) */
	ROM_LOAD( "04.bin",    0x00000, 0x10000, CRC(13ea1c44) SHA1(5b05fe4c3920e33d94fac5f59e09ff14b3e427fe) )

	ROM_REGION( 0x40000, "oki", 0 ) /* Samples */
	ROM_LOAD( "02.bin",    0x00000, 0x20000, CRC(7f162009) SHA1(346bf42992b4c36c593e21901e22c87ae4a7d86d) )
	ROM_LOAD( "01.bin",    0x20000, 0x20000, CRC(beade53f) SHA1(277c397dc12752719ec6b47d2224750bd1c07f79) )
ROM_END

ROM_START( sf2ebbl2 )
	ROM_REGION( CODE_SIZE, "maincpu", 0 )      /* 68000 code */
	ROM_LOAD16_BYTE( "27c020.7",   0x00000, 0x40000, CRC(a258b4d5) SHA1(3433b6493794c98bb35c1b27cc65bb5f13d52e9b) )
	ROM_LOAD16_BYTE( "27c020.5",   0x00001, 0x40000, CRC(59ccd474) SHA1(7bb28c28ee722435fdbb18eb73e52bd65b419103) )
	ROM_LOAD16_BYTE( "27c020.6",   0x80000, 0x40000, CRC(82097d63) SHA1(881e7ffb78197f6794b5d41f5c2c87da35e8cb15) )
	ROM_LOAD16_BYTE( "27c020.4",   0x80001, 0x40000, CRC(0c83844d) SHA1(4c25ba4a50d62c62789d026e3d304ed1dfb3c248) )

	ROM_REGION( 0x600000, "gfx", 0 )
	/* 5 MASK roms on this PCB match the original roms exactly, */
	ROMX_LOAD( "a-se235.bin", 0x000000, 0x80000, CRC(a258de13) SHA1(2e477948c4c8a2fb7cfdc4a739766bc4a4e01c49) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROM_CONTINUE(       0x000004, 0x80000)
	ROMX_LOAD( "c-se005.bin", 0x000002, 0x80000, CRC(c781bf87) SHA1(034baa9807c2ce8dc800200963a38cd9262b21fb) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROM_CONTINUE(       0x000006, 0x80000)
	ROMX_LOAD( "b-se194.bin", 0x200000, 0x80000, CRC(5726cab8) SHA1(0b2243a9a7184d53d42ddab7a8c51b63001c2f56) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROM_CONTINUE(       0x200004, 0x80000)
	ROMX_LOAD( "d-se064.bin", 0x200002, 0x80000, CRC(4dd24197) SHA1(548beaa0a6f1c3c88f4fc83169d1a3c86e0755d4) , ROM_GROUPWORD | ROM_SKIP(6) )   //sf2-8m.6c 99.999809%
	ROM_CONTINUE(       0x200006, 0x80000)
	ROMX_LOAD( "e-sf004.bin", 0x400000, 0x80000, CRC(187667cc) SHA1(fae65bf23f49a32903fda8080659ccf8d42b911f) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROM_CONTINUE(       0x400004, 0x80000)
	ROMX_LOAD( "f-sf001.bin", 0x400002, 0x80000, CRC(5b585071) SHA1(ad3371b1ba0441c67d9fcbb23b09464710e4e28a) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROM_CONTINUE(       0x400006, 0x80000)

	// These map over the MASK roms on this bootleg why? isn't it a waste of eprom?
	ROMX_LOAD( "27c1024.10", 0x400000, 0x20000, CRC(84427d1b) SHA1(f988a2b53c8cc46eeb8032084f24966a539b3734) , ROM_GROUPWORD | ROM_SKIP(6) )//e-sf004.bin [1/8]      IDENTICAL
	ROMX_LOAD( "27c1024.12", 0x400002, 0x20000, CRC(55bc790c) SHA1(a1114b89f6fa4487210477676984c77ad94b5ef8) , ROM_GROUPWORD | ROM_SKIP(6) )//f-sf001.bin [1/8]      IDENTICAL
	ROMX_LOAD( "27c1024.9",  0x400004, 0x20000, CRC(f8725add) SHA1(fa3fcf6637ee4dd7667bd89766074b3c6ba4f166) , ROM_GROUPWORD | ROM_SKIP(6) )//e-sf004.bin [5/8]      IDENTICAL
	ROMX_LOAD( "27c1024.11", 0x400006, 0x20000, CRC(c2a5373e) SHA1(602b32e5ecc7007efe9ad30751040ee52b81f59a) , ROM_GROUPWORD | ROM_SKIP(6) )//f-sf001.bin [5/8]      IDENTICAL


	ROM_REGION( 0x18000, "audiocpu", 0 ) /* 64k for the audio CPU (+banks) */
	ROM_LOAD( "27c512.3",    0x00000, 0x08000, CRC(a4823a1b) SHA1(7b6bf59dfd578bfbbdb64c27988796783442d659) )
	ROM_CONTINUE(              0x10000, 0x08000 )

	ROM_REGION( 0x20000, "user1", 0 ) /* unknown (bootleg priority?) */
	ROM_LOAD( "27c512.8",    0x00000, 0x10000, CRC(13ea1c44) SHA1(5b05fe4c3920e33d94fac5f59e09ff14b3e427fe) )

	ROM_REGION( 0x40000, "oki", 0 ) /* Samples */
	ROM_LOAD( "27c010.2",    0x00000, 0x20000, CRC(7f162009) SHA1(346bf42992b4c36c593e21901e22c87ae4a7d86d) )
	ROM_LOAD( "27c010.1",    0x20000, 0x20000, CRC(beade53f) SHA1(277c397dc12752719ec6b47d2224750bd1c07f79) )
ROM_END

ROM_START( sf2ebbl3 )
	ROM_REGION( CODE_SIZE, "maincpu", 0 )      /* 68000 code */
	ROM_LOAD16_BYTE( "ce91e-b.bin",   0x00000, 0x80000, CRC(963200d2) SHA1(3fa23fdee6d43f01066a94b4b01b1811168637a2) )
	ROM_LOAD16_BYTE( "ce91e-a.bin",   0x00001, 0x80000, CRC(02e88ec7) SHA1(1898e110108301a08686945f449fd084e394fc17) )

	ROM_REGION( 0x600000, "gfx", 0 )
	/* There are 8 mask  and 8 otp eprom on this board, data match sf2 set in the end */
	ROMX_LOAD( "1-b-yf197.07",  0x000000, 0x80000, CRC(22c9cc8e) SHA1(b9194fb337b30502c1c9501cd6c64ae4035544d4) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "1-d-yf207.12",  0x000002, 0x80000, CRC(57213be8) SHA1(3759b851ac0904ec79cbb67a2264d384b6f2f9f9) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "27020.1",       0x000004, 0x40000, CRC(84afb959) SHA1(ff8f3fb39203d229552649b09046a509aa399b7d) , ROM_SKIP(7) )
	ROMX_LOAD( "27020.3",       0x000005, 0x40000, CRC(ab21635d) SHA1(c6294b06c482a43cc1736674b0c1bc790da8cc8c) , ROM_SKIP(7) )
	ROMX_LOAD( "27020.5",       0x000006, 0x40000, CRC(2ce56f9f) SHA1(5bd23823df52aeff8e84a6eb3199a5e75f38cd42) , ROM_SKIP(7) )
	ROMX_LOAD( "27020.7",       0x000007, 0x40000, CRC(0ad7fb2b) SHA1(51265b331cbbe9d133274bafdbf043151240a20d) , ROM_SKIP(7) )

	ROMX_LOAD( "27020.2",       0x200000, 0x40000, CRC(031525cc) SHA1(65e280607c058cf9c005a6fc809ed375d379c0bf) , ROM_SKIP(7) )
	ROMX_LOAD( "27020.4",       0x200001, 0x40000, CRC(c97046a5) SHA1(88c7becaa2713c85e22ddd3e2305770525a9132b) , ROM_SKIP(7) )
	ROMX_LOAD( "1-h-yg010.10",  0x200002, 0x80000, CRC(b5548f17) SHA1(baa92b91cf616bc9e2a8a66adc777ffbf962a51b) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "1-e-yg003.02",  0x200004, 0x80000, CRC(14b84312) SHA1(2eea16673e60ba7a10bd4d8f6c217bb2441a5b0e) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "27020.6",       0x200006, 0x40000, CRC(dbbfd400) SHA1(fc25928b6ed025fa2397d924bae93fd28a4e728f) , ROM_SKIP(7) )
	ROMX_LOAD( "27020.8",       0x200007, 0x40000, CRC(37635e97) SHA1(b4e5dd6e93174af76aaeeb47471ded0d0bc253e2) , ROM_SKIP(7) )

	ROMX_LOAD( "1-j-yf213.09",      0x400000, 0x80000, CRC(994bfa58) SHA1(5669b845f624b10e7be56bfc89b76592258ce48b) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "wm91m-11-yd025.11", 0x400002, 0x80000, CRC(3e66ad9d) SHA1(9af9df0826988872662753e9717c48d46f2974b0) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "1-i-yf224.03",      0x400004, 0x80000, CRC(c1befaa8) SHA1(a6a7f4725e52678cbd8d557285c01cdccb2c2602) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "1-k-yf036.06",      0x400006, 0x80000, CRC(0627c831) SHA1(f9a92d614e8877d648449de2612fc8b43c85e4c2) , ROM_GROUPWORD | ROM_SKIP(6) )

	/* These map over the MASK roms on this bootleg to get rid of the CAPCOM logo (wasteful, but correct) */
	ROMX_LOAD( "27010.09hi", 0x400000, 0x10000, CRC(a505621e) SHA1(8ffa8cedad54948870bbd8f629d927332dc9fcf6) , ROM_SKIP(7) )
	ROM_CONTINUE(        0x400004, 0x10000 )
	ROMX_LOAD( "27010.11", 0x400002, 0x10000, CRC(de6271fb) SHA1(574ec5d9992941a405fd00abe52da41aba4b29a7) , ROM_SKIP(7) )
	ROM_CONTINUE(        0x400006, 0x10000 )
	ROMX_LOAD( "27010.09lo", 0x400001, 0x10000, CRC(23775344) SHA1(0d6b54f66cd64c3f48c88c8e17a19fdb2002afb0) , ROM_SKIP(7) )
	ROM_CONTINUE(        0x400005, 0x10000 )
	ROMX_LOAD( "27010.06", 0x400003, 0x10000, CRC(81c9550f) SHA1(2d75e329148caadfff35c8f2f91b352f14dbe08a) , ROM_SKIP(7) )
	ROM_CONTINUE(        0x400007, 0x10000 )

	ROM_REGION( 0x18000, "audiocpu", 0 ) /* 64k for the audio CPU (+banks) */
	ROM_LOAD( "27512.3",    0x00000, 0x08000, CRC(a4823a1b) SHA1(7b6bf59dfd578bfbbdb64c27988796783442d659) )
	ROM_CONTINUE(              0x10000, 0x08000 )

	ROM_REGION( 0x20000, "user1", 0 ) /* unknown (bootleg priority?) */
	ROM_LOAD( "27512.8",    0x00000, 0x10000, CRC(13ea1c44) SHA1(5b05fe4c3920e33d94fac5f59e09ff14b3e427fe) )

	ROM_REGION( 0x40000, "oki", 0 ) /* Samples */
	ROM_LOAD( "27010.2",    0x00000, 0x20000, CRC(7f162009) SHA1(346bf42992b4c36c593e21901e22c87ae4a7d86d) )
	ROM_LOAD( "27010.1",    0x20000, 0x20000, CRC(beade53f) SHA1(277c397dc12752719ec6b47d2224750bd1c07f79) )
ROM_END

ROM_START( sf2stt )
	/* the PCB is not working on real hardware */
	ROM_REGION( CODE_SIZE, "maincpu", 0 )      /* 68000 code */
	/* do not move this outside comments, this is only for testing purpose
	ROM_LOAD16_BYTE( "12.bin",   0x00000, 0x40000, CRC(a258b4d5) SHA1(3433b6493794c98bb35c1b27cc65bb5f13d52e9b) )
	ROM_LOAD16_BYTE( "09.bin",   0x00001, 0x40000, CRC(59ccd474) SHA1(7bb28c28ee722435fdbb18eb73e52bd65b419103) )
	*/
	ROM_LOAD16_BYTE( "prg part 1.stt", 0x00000, 0x40000, NO_DUMP )
	ROM_LOAD16_BYTE( "prg part 2.stt", 0x00001, 0x40000, NO_DUMP )
	/* missing first part of program roms, so it can not boot */
	ROM_LOAD16_BYTE( "ce91e-b",        0x80000, 0x40000, CRC(0862386e) SHA1(9fcfbcbbc17529de75d5419018e7b1dd90b397c0) )
	ROM_LOAD16_BYTE( "ce91e-a",        0x80001, 0x40000, CRC(0c83844d) SHA1(4c25ba4a50d62c62789d026e3d304ed1dfb3c248) )

	ROM_REGION( 0x600000, "gfx", 0 )
	ROMX_LOAD( "stt-a", 0x000000, 0x80000, CRC(10a7036d) SHA1(5f7780154d12dc202ec4bf2f3666727820745ab0), ROM_GROUPWORD | ROM_SKIP(6) )
	ROM_CONTINUE(       0x000002, 0x80000)
	ROMX_LOAD( "stt-d", 0x000004, 0x80000, CRC(3580b124) SHA1(55b101e7be3d1244d0b9961ad3c6ff5d682214ce), ROM_GROUPWORD | ROM_SKIP(6) )
	ROM_CONTINUE(       0x000006, 0x80000)
	ROMX_LOAD( "stt-b", 0x200000, 0x80000, CRC(7a09224e) SHA1(77a95b931f0a1ebd38c9aaf51a3b59139e0aedeb), ROM_GROUPWORD | ROM_SKIP(6) )
	ROM_CONTINUE(       0x200002, 0x80000)
	ROMX_LOAD( "stt-e", 0x200004, 0x80000, CRC(382a612c) SHA1(ecdcb66ed9d47d850501abc5f77eb5ad87b868d3), ROM_GROUPWORD | ROM_SKIP(6) )
	ROM_CONTINUE(       0x200006, 0x80000)
	ROMX_LOAD( "stt-c", 0x400000, 0x80000, CRC(11701b8f) SHA1(4f0c897349a0f1701a144ec64a75fa23fec32ae5), ROM_GROUPWORD | ROM_SKIP(6) )
	ROM_CONTINUE(       0x400002, 0x80000)
	ROMX_LOAD( "stt-f", 0x400004, 0x80000, CRC(101a0b72) SHA1(9e166a4110a583e722566852e27cb0b074ec97c6), ROM_GROUPWORD | ROM_SKIP(6) )
	ROM_CONTINUE(       0x400006, 0x80000)
	/* extra gfx layer roms loaded over the former ones to remove the capcom copyright logo */
	ROMX_LOAD( "5.stt", 0x400000, 0x10000, CRC(a505621e) SHA1(8ffa8cedad54948870bbd8f629d927332dc9fcf6), ROM_SKIP(7) )
	ROM_CONTINUE(       0x400004, 0x10000 )
	ROMX_LOAD( "7.stt", 0x400002, 0x10000, CRC(de6271fb) SHA1(574ec5d9992941a405fd00abe52da41aba4b29a7), ROM_SKIP(7) )
	ROM_CONTINUE(       0x400006, 0x10000 )
	ROMX_LOAD( "6.stt", 0x400001, 0x10000, CRC(23775344) SHA1(0d6b54f66cd64c3f48c88c8e17a19fdb2002afb0), ROM_SKIP(7) )
	ROM_CONTINUE(       0x400005, 0x10000 )
	ROMX_LOAD( "8.stt", 0x400003, 0x10000, CRC(81c9550f) SHA1(2d75e329148caadfff35c8f2f91b352f14dbe08a), ROM_SKIP(7) )
	ROM_CONTINUE(       0x400007, 0x10000 )
	/* end of extra gfx layer roms */

	ROM_REGION( 0x18000, "audiocpu", 0 ) /* 64k for the audio CPU (+banks) */
	ROM_LOAD( "3.stt", 0x00000, 0x08000, CRC(a4823a1b) SHA1(7b6bf59dfd578bfbbdb64c27988796783442d659) )
	ROM_CONTINUE(      0x10000, 0x08000 )

	ROM_REGION( 0x20000, "user1", 0 ) /* unknown (bootleg priority?) */
	ROM_LOAD( "4.stt", 0x00000, 0x10000, CRC(13ea1c44) SHA1(5b05fe4c3920e33d94fac5f59e09ff14b3e427fe) )

	ROM_REGION( 0x40000, "oki", 0 ) /* Samples */
	ROM_LOAD( "2.stt", 0x00000, 0x20000, CRC(7f162009) SHA1(346bf42992b4c36c593e21901e22c87ae4a7d86d) )
	ROM_LOAD( "1.stt", 0x20000, 0x20000, CRC(beade53f) SHA1(277c397dc12752719ec6b47d2224750bd1c07f79) )
ROM_END

ROM_START( sf2rk ) /* unidentified bootleg, name based only on gfx chips label */
	/* the PCB is not working on real hardware */
	ROM_REGION( CODE_SIZE, "maincpu", 0 )      /* 68000 code */
	/* do not move this outside comments, this is only for testing purpose
	ROM_LOAD16_BYTE( "12.bin",             0x000000, 0x40000, CRC(a258b4d5) SHA1(3433b6493794c98bb35c1b27cc65bb5f13d52e9b) )
	ROM_LOAD16_BYTE( "09.bin",             0x000001, 0x40000, CRC(59ccd474) SHA1(7bb28c28ee722435fdbb18eb73e52bd65b419103) )
	ROM_LOAD16_BYTE( "11.bin",             0x080000, 0x40000, CRC(82097d63) SHA1(881e7ffb78197f6794b5d41f5c2c87da35e8cb15) )
	ROM_LOAD16_BYTE( "10.bin",             0x080001, 0x40000, CRC(0c83844d) SHA1(4c25ba4a50d62c62789d026e3d304ed1dfb3c248) )
	*/
	ROM_LOAD16_BYTE( "prg part 1.rk", 0x00000, 0x80000, NO_DUMP )
	ROM_LOAD16_BYTE( "prg part 2.rk", 0x00001, 0x80000, NO_DUMP )
	/* missing first part of program roms, so it can not boot */
	ROM_LOAD16_BYTE( "w-6",           0x100000, 0x20000, CRC(bb4af315) SHA1(75f0827f4f7e9f292add46467f8d4fe19b2514c9) )
	ROM_LOAD16_BYTE( "w-5",           0x100001, 0x20000, CRC(c02a13eb) SHA1(b807cc495bff3f95d03b061fc629c95f965cb6d8) )

	ROM_REGION( 0x600000, "gfx", 0 )
	ROMX_LOAD( "01 rk098", 0x000000, 0x80000, CRC(4296de4d) SHA1(2bd5a0ebe2a20c745b11da9c7dc4f13f20efdda7), ROM_GROUPWORD | ROM_SKIP(6) )
	ROM_CONTINUE(          0x000002, 0x80000)
	ROMX_LOAD( "03 rk097", 0x000004, 0x80000, CRC(16cf11d0) SHA1(e1adf34467e0a6902bcda63718885fe0bea831b1), ROM_GROUPWORD | ROM_SKIP(6) )
	ROM_CONTINUE(          0x000006, 0x80000)
	ROMX_LOAD( "02 rk037", 0x200000, 0x80000, CRC(68ca7fce) SHA1(f6a37e0ca4d9490f66c3d29308c531f2134fd7aa), ROM_GROUPWORD | ROM_SKIP(6) )
	ROM_CONTINUE(          0x200002, 0x80000)
	ROMX_LOAD( "04 rk033", 0x200004, 0x80000, CRC(9f46f926) SHA1(f1ddf8ce6d895358979631fbdecbeff51376b604), ROM_GROUPWORD | ROM_SKIP(6) )
	ROM_CONTINUE(          0x200006, 0x80000)
	ROMX_LOAD( "05 rk116", 0x400000, 0x80000, CRC(4c161fa9) SHA1(c3b2f642d3d3be5aab6ff1482d82c2858944d07a), ROM_GROUPWORD | ROM_SKIP(6) )
	ROM_CONTINUE(          0x400002, 0x80000)
	ROMX_LOAD( "06 rk077", 0x400004, 0x80000, CRC(ec949f8c) SHA1(34ea3d6d85486a5ff25c774dbc6a4b16037a7347), ROM_GROUPWORD | ROM_SKIP(6) )
	ROM_CONTINUE(          0x400006, 0x80000)
	/* extra gfx layer roms loaded over the former ones to remove the capcom copyright logo */
	ROMX_LOAD( "w-1",      0x400000, 0x08000, CRC(124b9ffc) SHA1(299368e0f95475cf575bc3b041c6179d243b9b81), ROM_SKIP(7) )
	ROM_CONTINUE(          0x400004, 0x08000 )
	ROMX_LOAD( "w-3",      0x400002, 0x08000, CRC(5992783a) SHA1(69c90621c72c7889978442009a4ddbbfb1f3444f), ROM_SKIP(7) )
	ROM_CONTINUE(          0x400006, 0x08000 )
	ROMX_LOAD( "w-2",      0x400001, 0x08000, CRC(11d715f7) SHA1(cdec16ba16ca1ec6b0ab94addf4cfeeddeff5063), ROM_SKIP(7) )
	ROM_CONTINUE(          0x400005, 0x08000 )
	ROMX_LOAD( "w-4",      0x400003, 0x08000, CRC(db0dd977) SHA1(c34ed40709001423325b7b7fad3f4ffeb38c3e7e), ROM_SKIP(7) )
	ROM_CONTINUE(          0x400007, 0x08000 )
	/* end of extra gfx layer roms */

	ROM_REGION( 0x18000, "audiocpu", 0 ) /* 64k for the audio CPU (+banks) */
	ROM_LOAD( "1.rk", 0x00000, 0x08000, CRC(a4823a1b) SHA1(7b6bf59dfd578bfbbdb64c27988796783442d659) )
	ROM_CONTINUE(     0x10000, 0x08000 )

	ROM_REGION( 0x20000, "user1", 0 ) /* unknown (bootleg priority?) */
	ROM_LOAD( "2.rk", 0x00000, 0x10000, CRC(13ea1c44) SHA1(5b05fe4c3920e33d94fac5f59e09ff14b3e427fe) )

	ROM_REGION( 0x40000, "oki", 0 ) /* Samples */
	ROM_LOAD( "sample part 1.rk", 0x20000, 0x20000, NO_DUMP )
	/* do not move this outside comments, this is only for testing purpose
	ROM_LOAD( "sf2_18.11c",     0x00000, 0x20000, CRC(7f162009) SHA1(346bf42992b4c36c593e21901e22c87ae4a7d86d) )
	*/
	ROM_LOAD( "w-7",              0x20000, 0x20000, CRC(beade53f) SHA1(277c397dc12752719ec6b47d2224750bd1c07f79) )
ROM_END

ROM_START( sf2qp1 )
	ROM_REGION( CODE_SIZE, "maincpu", 0 )      /* 68000 code */
	ROM_LOAD16_WORD_SWAP( "stfii-qkn-cps-17.33", 0x000000, 0x40000, CRC(3a9458ee) SHA1(cbf56115c22a45e5ce1cde313fa5a5ba496316eb) )
	ROM_CONTINUE(              0xc0000, 0x40000 )
	ROM_LOAD16_WORD_SWAP( "stfii-qkn-cps-17.34", 0x080000, 0x40000, CRC(4ed215d8) SHA1(b09075f883c3a4976ab47203acc04f8f993969a8) )
	ROM_CONTINUE(              0x40000, 0x40000 )

	ROM_REGION( 0x600000, "gfx", 0 )
	ROMX_LOAD( "sf2_06.bin", 0x000000, 0x80000, CRC(22c9cc8e) SHA1(b9194fb337b30502c1c9501cd6c64ae4035544d4) , ROM_GROUPWORD | ROM_SKIP(6) )    // == sf2-5m.4a
	ROMX_LOAD( "sf2_08.bin", 0x000002, 0x80000, CRC(57213be8) SHA1(3759b851ac0904ec79cbb67a2264d384b6f2f9f9) , ROM_GROUPWORD | ROM_SKIP(6) )    // == sf2-7m.6a
	ROMX_LOAD( "sf2_05.bin", 0x000004, 0x80000, CRC(ba529b4f) SHA1(520840d727161cf09ca784919fa37bc9b54cc3ce) , ROM_GROUPWORD | ROM_SKIP(6) )    // == sf2-1m.3a
	ROMX_LOAD( "sf2_07.bin", 0x000006, 0x80000, CRC(4b1b33a8) SHA1(2360cff890551f76775739e2d6563858bff80e41) , ROM_GROUPWORD | ROM_SKIP(6) )    // == sf2-3m.5a
	ROMX_LOAD( "sf2_15.bin", 0x200000, 0x80000, CRC(2c7e2229) SHA1(357c2275af9133fd0bd6fbb1fa9ad5e0b490b3a2) , ROM_GROUPWORD | ROM_SKIP(6) )    // == sf2-6m.4c
	ROMX_LOAD( "sf2_17.bin", 0x200002, 0x80000, CRC(b5548f17) SHA1(baa92b91cf616bc9e2a8a66adc777ffbf962a51b) , ROM_GROUPWORD | ROM_SKIP(6) )    // == sf2-8m.6c
	ROMX_LOAD( "sf2_14.bin", 0x200004, 0x80000, CRC(14b84312) SHA1(2eea16673e60ba7a10bd4d8f6c217bb2441a5b0e) , ROM_GROUPWORD | ROM_SKIP(6) )    // == sf2-2m.3c
	ROMX_LOAD( "sf2_16.bin", 0x200006, 0x80000, CRC(5e9cd89a) SHA1(f787aab98668d4c2c54fc4ba677c0cb808e4f31e) , ROM_GROUPWORD | ROM_SKIP(6) )    // == sf2-4m.5c
	ROMX_LOAD( "sf2_25.bin", 0x400000, 0x80000, CRC(994bfa58) SHA1(5669b845f624b10e7be56bfc89b76592258ce48b) , ROM_GROUPWORD | ROM_SKIP(6) )    // == sf2-13m.4d
	ROMX_LOAD( "sf2_27.bin", 0x400002, 0x80000, CRC(3e66ad9d) SHA1(9af9df0826988872662753e9717c48d46f2974b0) , ROM_GROUPWORD | ROM_SKIP(6) )    // == sf2-15m.6d
	ROMX_LOAD( "sf2_24.bin", 0x400004, 0x80000, CRC(c1befaa8) SHA1(a6a7f4725e52678cbd8d557285c01cdccb2c2602) , ROM_GROUPWORD | ROM_SKIP(6) )    // == sf2-9m.3d
	ROMX_LOAD( "sf2_26.bin", 0x400006, 0x80000, CRC(0627c831) SHA1(f9a92d614e8877d648449de2612fc8b43c85e4c2) , ROM_GROUPWORD | ROM_SKIP(6) )    // == sf2-11m.5d

	ROM_REGION( 0x18000, "audiocpu", 0 ) /* 64k for the audio CPU (+banks) */
	ROM_LOAD( "sf2_09.bin",    0x00000, 0x08000, CRC(a4823a1b) SHA1(7b6bf59dfd578bfbbdb64c27988796783442d659) )
	ROM_CONTINUE(              0x10000, 0x08000 )

	ROM_REGION( 0x40000, "oki", 0 ) /* Samples */
	ROM_LOAD( "sf2_18.bin",    0x00000, 0x20000, CRC(7f162009) SHA1(346bf42992b4c36c593e21901e22c87ae4a7d86d) )
	ROM_LOAD( "sf2_19.bin",    0x20000, 0x20000, CRC(beade53f) SHA1(277c397dc12752719ec6b47d2224750bd1c07f79) )

	ROM_REGION( 0x0200, "aboardplds", 0 )
	ROM_LOAD( "buf1",         0x0000, 0x0117, CRC(eb122de7) SHA1(b26b5bfe258e3e184f069719f9fd008d6b8f6b9b) )
	ROM_LOAD( "ioa1",         0x0000, 0x0117, CRC(59c7ee3b) SHA1(fbb887c5b4f5cb8df77cec710eaac2985bc482a6) )
	ROM_LOAD( "prg1",         0x0000, 0x0117, CRC(f1129744) SHA1(a5300f301c1a08a7da768f0773fa0fe3f683b237) )
	ROM_LOAD( "rom1",         0x0000, 0x0117, CRC(41dc73b9) SHA1(7d4c9f1693c821fbf84e32dd6ef62ddf14967845) )
	ROM_LOAD( "sou1",         0x0000, 0x0117, CRC(84f4b2fe) SHA1(dcc9e86cc36316fe42eace02d6df75d08bc8bb6d) )
ROM_END

ROM_START( sf2thndr )
	ROM_REGION( CODE_SIZE, "maincpu", 0 )      /* 68000 code */
	ROM_LOAD16_BYTE( "17_30.11e",     0x00000, 0x20000, CRC(d3cd6d18) SHA1(c7efe9e7a7dee29bfe40a571960145efa26ac22e) )
	ROM_LOAD16_BYTE( "17_37.11f",     0x00001, 0x20000, CRC(e892716e) SHA1(b3836b33bc026a0bccc5ed7cdecf8d2ba2b5607a) )
	ROM_LOAD16_BYTE( "sf2u_31b.12e",  0x40000, 0x20000, CRC(a673143d) SHA1(e565f0ec23d6deb543c72af5a83f070c07319477) )
	ROM_LOAD16_BYTE( "sf2u_38b.12f",  0x40001, 0x20000, CRC(4c2ccef7) SHA1(77b119c70c255622b023de25d9af3b3aac52ea47) )
	ROM_LOAD16_BYTE( "sf2u_28b.9e",   0x80000, 0x20000, CRC(4009955e) SHA1(7842dbef7650485639fbae49b9f4db7494d4f73d) )
	ROM_LOAD16_BYTE( "sf2u_35b.9f",   0x80001, 0x20000, CRC(8c1f3994) SHA1(5e1d334399d05a837c2d80f79eada543e83afaf7) )
	ROM_LOAD16_BYTE( "17_29.10e",     0xc0000, 0x20000, CRC(8830b54d) SHA1(fe5e958f69d52fa12c31c1a7ccf0dcfd340045cb) )
	ROM_LOAD16_BYTE( "17_36.10f",     0xc0001, 0x20000, CRC(3f13ada3) SHA1(412572f220f64e5b0ee2cad0a9204142ae2f7c90) )

	ROM_REGION( 0x600000, "gfx", 0 )
	ROMX_LOAD( "sf2_06.bin", 0x000000, 0x80000, CRC(22c9cc8e) SHA1(b9194fb337b30502c1c9501cd6c64ae4035544d4) , ROM_GROUPWORD | ROM_SKIP(6) )    // == sf2-5m.4a
	ROMX_LOAD( "sf2_08.bin", 0x000002, 0x80000, CRC(57213be8) SHA1(3759b851ac0904ec79cbb67a2264d384b6f2f9f9) , ROM_GROUPWORD | ROM_SKIP(6) )    // == sf2-7m.6a
	ROMX_LOAD( "sf2_05.bin", 0x000004, 0x80000, CRC(ba529b4f) SHA1(520840d727161cf09ca784919fa37bc9b54cc3ce) , ROM_GROUPWORD | ROM_SKIP(6) )    // == sf2-1m.3a
	ROMX_LOAD( "sf2_07.bin", 0x000006, 0x80000, CRC(4b1b33a8) SHA1(2360cff890551f76775739e2d6563858bff80e41) , ROM_GROUPWORD | ROM_SKIP(6) )    // == sf2-3m.5a
	ROMX_LOAD( "sf2_15.bin", 0x200000, 0x80000, CRC(2c7e2229) SHA1(357c2275af9133fd0bd6fbb1fa9ad5e0b490b3a2) , ROM_GROUPWORD | ROM_SKIP(6) )    // == sf2-6m.4c
	ROMX_LOAD( "sf2_17.bin", 0x200002, 0x80000, CRC(b5548f17) SHA1(baa92b91cf616bc9e2a8a66adc777ffbf962a51b) , ROM_GROUPWORD | ROM_SKIP(6) )    // == sf2-8m.6c
	ROMX_LOAD( "sf2_14.bin", 0x200004, 0x80000, CRC(14b84312) SHA1(2eea16673e60ba7a10bd4d8f6c217bb2441a5b0e) , ROM_GROUPWORD | ROM_SKIP(6) )    // == sf2-2m.3c
	ROMX_LOAD( "sf2_16.bin", 0x200006, 0x80000, CRC(5e9cd89a) SHA1(f787aab98668d4c2c54fc4ba677c0cb808e4f31e) , ROM_GROUPWORD | ROM_SKIP(6) )    // == sf2-4m.5c
	ROMX_LOAD( "sf2_25.bin", 0x400000, 0x80000, CRC(994bfa58) SHA1(5669b845f624b10e7be56bfc89b76592258ce48b) , ROM_GROUPWORD | ROM_SKIP(6) )    // == sf2-13m.4d
	ROMX_LOAD( "sf2_27.bin", 0x400002, 0x80000, CRC(3e66ad9d) SHA1(9af9df0826988872662753e9717c48d46f2974b0) , ROM_GROUPWORD | ROM_SKIP(6) )    // == sf2-15m.6d
	ROMX_LOAD( "sf2_24.bin", 0x400004, 0x80000, CRC(c1befaa8) SHA1(a6a7f4725e52678cbd8d557285c01cdccb2c2602) , ROM_GROUPWORD | ROM_SKIP(6) )    // == sf2-9m.3d
	ROMX_LOAD( "sf2_26.bin", 0x400006, 0x80000, CRC(0627c831) SHA1(f9a92d614e8877d648449de2612fc8b43c85e4c2) , ROM_GROUPWORD | ROM_SKIP(6) )    // == sf2-11m.5d

	ROM_REGION( 0x18000, "audiocpu", 0 ) /* 64k for the audio CPU (+banks) */
	ROM_LOAD( "sf2_09.bin",    0x00000, 0x08000, CRC(a4823a1b) SHA1(7b6bf59dfd578bfbbdb64c27988796783442d659) )
	ROM_CONTINUE(              0x10000, 0x08000 )

	ROM_REGION( 0x40000, "oki", 0 ) /* Samples */
	ROM_LOAD( "sf2_18.bin",    0x00000, 0x20000, CRC(7f162009) SHA1(346bf42992b4c36c593e21901e22c87ae4a7d86d) )
	ROM_LOAD( "sf2_19.bin",    0x20000, 0x20000, CRC(beade53f) SHA1(277c397dc12752719ec6b47d2224750bd1c07f79) )

	ROM_REGION( 0x0200, "aboardplds", 0 )
	ROM_LOAD( "buf1",         0x0000, 0x0117, CRC(eb122de7) SHA1(b26b5bfe258e3e184f069719f9fd008d6b8f6b9b) )
	ROM_LOAD( "ioa1",         0x0000, 0x0117, CRC(59c7ee3b) SHA1(fbb887c5b4f5cb8df77cec710eaac2985bc482a6) )
	ROM_LOAD( "prg1",         0x0000, 0x0117, CRC(f1129744) SHA1(a5300f301c1a08a7da768f0773fa0fe3f683b237) )
	ROM_LOAD( "rom1",         0x0000, 0x0117, CRC(41dc73b9) SHA1(7d4c9f1693c821fbf84e32dd6ef62ddf14967845) )
	ROM_LOAD( "sou1",         0x0000, 0x0117, CRC(84f4b2fe) SHA1(dcc9e86cc36316fe42eace02d6df75d08bc8bb6d) )
ROM_END

/* B-Board 89624B-3 */
ROM_START( 3wonders )
	ROM_REGION( CODE_SIZE, "maincpu", 0 )      /* 68000 code */
	ROM_LOAD16_BYTE( "rte_30a.11f", 0x00000, 0x20000, CRC(ef5b8b33) SHA1(2313168e5f10505ceece5fdaada0d30df3ca146c) )
	ROM_LOAD16_BYTE( "rte_35a.11h", 0x00001, 0x20000, CRC(7d705529) SHA1(b456629b5755b701cca8a438d24957367a260ec5) )
	ROM_LOAD16_BYTE( "rte_31a.12f", 0x40000, 0x20000, CRC(32835e5e) SHA1(9ec530561030a75a1283ff2aacc21e55613b682b) )
	ROM_LOAD16_BYTE( "rte_36a.12h", 0x40001, 0x20000, CRC(7637975f) SHA1(56935032eebd3e1c5059f6842b97001dae0aa55f) )
	ROM_LOAD16_BYTE( "rt_28a.9f",   0x80000, 0x20000, CRC(054137c8) SHA1(e4c406e0a32198323a5931093fbaa6836510b8ad) )
	ROM_LOAD16_BYTE( "rt_33a.9h",   0x80001, 0x20000, CRC(7264cb1b) SHA1(b367acb9f6579569321ecaa98a14e29dd775b9db) )
	ROM_LOAD16_BYTE( "rte_29a.10f", 0xc0000, 0x20000, CRC(cddaa919) SHA1(0c98e95ad5033d2c5ade7651243e7ccdb4e35463) )
	ROM_LOAD16_BYTE( "rte_34a.10h", 0xc0001, 0x20000, CRC(ed52e7e5) SHA1(352433ae484967d26376141e3a8a0f968b98fde6) )

	ROM_REGION( 0x400000, "gfx", 0 )
	ROMX_LOAD( "rt-5m.7a",  0x000000, 0x80000, CRC(86aef804) SHA1(723927ef3bf992d12395c52db051ece7bf57d5e5) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "rt-7m.9a",  0x000002, 0x80000, CRC(4f057110) SHA1(b7d35c883a74cf4bfb242d9f15a0e40ed1ec111f) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "rt-1m.3a",  0x000004, 0x80000, CRC(902489d0) SHA1(748ba416a8b9343059a3e7d8b93f02162feb1d0b) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "rt-3m.5a",  0x000006, 0x80000, CRC(e35ce720) SHA1(6c1a87a1f819bdc20408b5a7823cf35a79d34110) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "rt-6m.8a",  0x200000, 0x80000, CRC(13cb0e7c) SHA1(e429d594d9a7ff4cc6306e2796a9d6ad0fa25569) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "rt-8m.10a", 0x200002, 0x80000, CRC(1f055014) SHA1(d64f5be9bb2ef761ca9b2e797dbc3554cf996a79) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "rt-2m.4a",  0x200004, 0x80000, CRC(e9a034f4) SHA1(deb4cb5886705380b57d4fe9b9bf3c032e1d6227) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "rt-4m.6a",  0x200006, 0x80000, CRC(df0eea8b) SHA1(5afa05654cccb0504bd44569d42fd68f08fd172f) , ROM_GROUPWORD | ROM_SKIP(6) )

	ROM_REGION( 0x18000, "audiocpu", 0 ) /* 64k for the audio CPU (+banks) */
	ROM_LOAD( "rt_9.12b",   0x00000, 0x08000, CRC(abfca165) SHA1(428069d3bdc45775854cd0e8abe447f134fe5492) )
	ROM_CONTINUE(           0x10000, 0x08000 )

	ROM_REGION( 0x40000, "oki", 0 ) /* Samples */
	ROM_LOAD( "rt_18.11c",  0x00000, 0x20000, CRC(26b211ab) SHA1(0ea03fdd9edff41eacfc52aa9e0421c10968356b) )
	ROM_LOAD( "rt_19.12c",  0x20000, 0x20000, CRC(dbe64ad0) SHA1(09f2ad522fe75d7bcca094b8c6696c3733b539d5) )

	ROM_REGION( 0x0200, "aboardplds", 0 )
	ROM_LOAD( "buf1",         0x0000, 0x0117, CRC(eb122de7) SHA1(b26b5bfe258e3e184f069719f9fd008d6b8f6b9b) )
	ROM_LOAD( "ioa1",         0x0000, 0x0117, CRC(59c7ee3b) SHA1(fbb887c5b4f5cb8df77cec710eaac2985bc482a6) )
	ROM_LOAD( "prg1",         0x0000, 0x0117, CRC(f1129744) SHA1(a5300f301c1a08a7da768f0773fa0fe3f683b237) )
	ROM_LOAD( "rom1",         0x0000, 0x0117, CRC(41dc73b9) SHA1(7d4c9f1693c821fbf84e32dd6ef62ddf14967845) )
	ROM_LOAD( "sou1",         0x0000, 0x0117, CRC(84f4b2fe) SHA1(dcc9e86cc36316fe42eace02d6df75d08bc8bb6d) )

	ROM_REGION( 0x0200, "bboardplds", 0 )
	ROM_LOAD( "rt24b.1a",     0x0000, 0x0117, CRC(54b85159) SHA1(c6f4fb5d747a215f4f50e4f2258e35d3f9bdbb2e) )
	ROM_LOAD( "iob1.11e",     0x0000, 0x0117, CRC(3abc0700) SHA1(973043aa46ec6d5d1db20dc9d5937005a0f9f6ae) )    /* seen the same pcb with LWIO.11E */

	ROM_REGION( 0x0200, "cboardplds", 0 )
	ROM_LOAD( "ioc1.ic1",     0x0000, 0x0117, CRC(0d182081) SHA1(475b3d417785da4bc512cce2b274bb00d4cc6792) )
ROM_END

/* B-Board 89624B-3 */
ROM_START( 3wondersr1 )
	ROM_REGION( CODE_SIZE, "maincpu", 0 )      /* 68000 code */
	ROM_LOAD16_BYTE( "rte_30.11f", 0x00000, 0x20000, CRC(0d541519) SHA1(ce0f9ebaea038c98f8866624d3061c65c84cf20d) )
	ROM_LOAD16_BYTE( "rte_35.11h", 0x00001, 0x20000, CRC(73dd0e20) SHA1(b4f4086574176befa70863a85a3c0a8fd6e4bd90) )
	ROM_LOAD16_BYTE( "rte_31.12f", 0x40000, 0x20000, CRC(33e0337d) SHA1(53e5795baf307a789b4337f081a932359e569a98) )
	ROM_LOAD16_BYTE( "rte_36.12h", 0x40001, 0x20000, CRC(a8865243) SHA1(0a4d7e8e07d88104aa01457a10a8a9abd3ed4ce1) )
	ROM_LOAD16_BYTE( "rte_28.9f",  0x80000, 0x20000, CRC(054137c8) SHA1(e4c406e0a32198323a5931093fbaa6836510b8ad) ) // == rt_28a.9f
	ROM_LOAD16_BYTE( "rte_33.9h",  0x80001, 0x20000, CRC(7264cb1b) SHA1(b367acb9f6579569321ecaa98a14e29dd775b9db) ) // == rt_33a.9h
	ROM_LOAD16_BYTE( "rte_29.10f", 0xc0000, 0x20000, CRC(9a8df1e4) SHA1(5b61d2d729145d524e23036bb27ba3ba75c208fe) )
	ROM_LOAD16_BYTE( "rte_34.10h", 0xc0001, 0x20000, CRC(6348a79d) SHA1(2e4b58f9c6c1ad6a89c90d8ebb46cee2613f1397) )

	ROM_REGION( 0x400000, "gfx", 0 )
	ROMX_LOAD( "rt-5m.7a",  0x000000, 0x80000, CRC(86aef804) SHA1(723927ef3bf992d12395c52db051ece7bf57d5e5) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "rt-7m.9a",  0x000002, 0x80000, CRC(4f057110) SHA1(b7d35c883a74cf4bfb242d9f15a0e40ed1ec111f) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "rt-1m.3a",  0x000004, 0x80000, CRC(902489d0) SHA1(748ba416a8b9343059a3e7d8b93f02162feb1d0b) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "rt-3m.5a",  0x000006, 0x80000, CRC(e35ce720) SHA1(6c1a87a1f819bdc20408b5a7823cf35a79d34110) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "rt-6m.8a",  0x200000, 0x80000, CRC(13cb0e7c) SHA1(e429d594d9a7ff4cc6306e2796a9d6ad0fa25569) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "rt-8m.10a", 0x200002, 0x80000, CRC(1f055014) SHA1(d64f5be9bb2ef761ca9b2e797dbc3554cf996a79) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "rt-2m.4a",  0x200004, 0x80000, CRC(e9a034f4) SHA1(deb4cb5886705380b57d4fe9b9bf3c032e1d6227) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "rt-4m.6a",  0x200006, 0x80000, CRC(df0eea8b) SHA1(5afa05654cccb0504bd44569d42fd68f08fd172f) , ROM_GROUPWORD | ROM_SKIP(6) )

	ROM_REGION( 0x18000, "audiocpu", 0 ) /* 64k for the audio CPU (+banks) */
	ROM_LOAD( "rt_9.12b",   0x00000, 0x08000, CRC(7d5a77a7) SHA1(4f2a6534d7a9e518061102c1dd7d7aca39600c33) )   /* 1 byte different from 3wonders, pcb verified */
	ROM_CONTINUE(           0x10000, 0x08000 )

	ROM_REGION( 0x40000, "oki", 0 ) /* Samples */
	ROM_LOAD( "rt_18.11c", 0x00000, 0x20000, CRC(f6dc0d3d) SHA1(9959fc3339613042c95b90aad2bc1c3dafdb605e) )   /* 6 bytes different from 3wonders, pcb verified */
	ROM_LOAD( "rt_19.12c", 0x20000, 0x20000, CRC(dbe64ad0) SHA1(09f2ad522fe75d7bcca094b8c6696c3733b539d5) )

	ROM_REGION( 0x0200, "aboardplds", 0 )
	ROM_LOAD( "buf1",         0x0000, 0x0117, CRC(eb122de7) SHA1(b26b5bfe258e3e184f069719f9fd008d6b8f6b9b) )
	ROM_LOAD( "ioa1",         0x0000, 0x0117, CRC(59c7ee3b) SHA1(fbb887c5b4f5cb8df77cec710eaac2985bc482a6) )
	ROM_LOAD( "prg1",         0x0000, 0x0117, CRC(f1129744) SHA1(a5300f301c1a08a7da768f0773fa0fe3f683b237) )
	ROM_LOAD( "rom1",         0x0000, 0x0117, CRC(41dc73b9) SHA1(7d4c9f1693c821fbf84e32dd6ef62ddf14967845) )
	ROM_LOAD( "sou1",         0x0000, 0x0117, CRC(84f4b2fe) SHA1(dcc9e86cc36316fe42eace02d6df75d08bc8bb6d) )

	ROM_REGION( 0x0200, "bboardplds", 0 )
	ROM_LOAD( "rt24b.1a",     0x0000, 0x0117, CRC(54b85159) SHA1(c6f4fb5d747a215f4f50e4f2258e35d3f9bdbb2e) )
	ROM_LOAD( "iob1.11e",     0x0000, 0x0117, CRC(3abc0700) SHA1(973043aa46ec6d5d1db20dc9d5937005a0f9f6ae) )

	ROM_REGION( 0x0200, "cboardplds", 0 )
	ROM_LOAD( "ioc1.ic1",     0x0000, 0x0117, CRC(0d182081) SHA1(475b3d417785da4bc512cce2b274bb00d4cc6792) )
ROM_END

/* B-Board 89624B-3 */
ROM_START( 3wondersu )
	ROM_REGION( CODE_SIZE, "maincpu", 0 )      /* 68000 code */
	ROM_LOAD16_BYTE( "rtu_30a.11f", 0x00000, 0x20000, CRC(0b156fd8) SHA1(1ec811cd7cbd12066f876db7255394e754ceb25e) )
	ROM_LOAD16_BYTE( "rtu_35a.11h", 0x00001, 0x20000, CRC(57350bf4) SHA1(33e8685cce82eee7bcb7c2787318a130764e97e2) )
	ROM_LOAD16_BYTE( "rtu_31a.12f", 0x40000, 0x20000, CRC(0e723fcc) SHA1(91eeab6376a5aa852152af9920aef60bc7c689dd) )
	ROM_LOAD16_BYTE( "rtu_36a.12h", 0x40001, 0x20000, CRC(523a45dc) SHA1(6d6743803016fa5ba713e0d6f61affce8a3255ec) )
	ROM_LOAD16_BYTE( "rt_28a.9f",   0x80000, 0x20000, CRC(054137c8) SHA1(e4c406e0a32198323a5931093fbaa6836510b8ad) )
	ROM_LOAD16_BYTE( "rt_33a.9h",   0x80001, 0x20000, CRC(7264cb1b) SHA1(b367acb9f6579569321ecaa98a14e29dd775b9db) )
	ROM_LOAD16_BYTE( "rtu_29a.10f", 0xc0000, 0x20000, CRC(37ba3e20) SHA1(a128b1a17639b06a4fd8acffe0357f1dbd1d4fe9) )
	ROM_LOAD16_BYTE( "rtu_34a.10h", 0xc0001, 0x20000, CRC(f99f46c0) SHA1(cda24a6baa3f861e7078fb2fa91328cc1cddc866) )

	ROM_REGION( 0x400000, "gfx", 0 )
	ROMX_LOAD( "rt-5m.7a",  0x000000, 0x80000, CRC(86aef804) SHA1(723927ef3bf992d12395c52db051ece7bf57d5e5) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "rt-7m.9a",  0x000002, 0x80000, CRC(4f057110) SHA1(b7d35c883a74cf4bfb242d9f15a0e40ed1ec111f) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "rt-1m.3a",  0x000004, 0x80000, CRC(902489d0) SHA1(748ba416a8b9343059a3e7d8b93f02162feb1d0b) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "rt-3m.5a",  0x000006, 0x80000, CRC(e35ce720) SHA1(6c1a87a1f819bdc20408b5a7823cf35a79d34110) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "rt-6m.8a",  0x200000, 0x80000, CRC(13cb0e7c) SHA1(e429d594d9a7ff4cc6306e2796a9d6ad0fa25569) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "rt-8m.10a", 0x200002, 0x80000, CRC(1f055014) SHA1(d64f5be9bb2ef761ca9b2e797dbc3554cf996a79) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "rt-2m.4a",  0x200004, 0x80000, CRC(e9a034f4) SHA1(deb4cb5886705380b57d4fe9b9bf3c032e1d6227) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "rt-4m.6a",  0x200006, 0x80000, CRC(df0eea8b) SHA1(5afa05654cccb0504bd44569d42fd68f08fd172f) , ROM_GROUPWORD | ROM_SKIP(6) )

	ROM_REGION( 0x18000, "audiocpu", 0 ) /* 64k for the audio CPU (+banks) */
	ROM_LOAD( "rt_9.12b",   0x00000, 0x08000, CRC(abfca165) SHA1(428069d3bdc45775854cd0e8abe447f134fe5492) )
	ROM_CONTINUE(           0x10000, 0x08000 )

	ROM_REGION( 0x40000, "oki", 0 ) /* Samples */
	ROM_LOAD( "rt_18.11c",  0x00000, 0x20000, CRC(26b211ab) SHA1(0ea03fdd9edff41eacfc52aa9e0421c10968356b) )
	ROM_LOAD( "rt_19.12c",  0x20000, 0x20000, CRC(dbe64ad0) SHA1(09f2ad522fe75d7bcca094b8c6696c3733b539d5) )

	ROM_REGION( 0x0200, "aboardplds", 0 )
	ROM_LOAD( "buf1",         0x0000, 0x0117, CRC(eb122de7) SHA1(b26b5bfe258e3e184f069719f9fd008d6b8f6b9b) )
	ROM_LOAD( "ioa1",         0x0000, 0x0117, CRC(59c7ee3b) SHA1(fbb887c5b4f5cb8df77cec710eaac2985bc482a6) )
	ROM_LOAD( "prg1",         0x0000, 0x0117, CRC(f1129744) SHA1(a5300f301c1a08a7da768f0773fa0fe3f683b237) )
	ROM_LOAD( "rom1",         0x0000, 0x0117, CRC(41dc73b9) SHA1(7d4c9f1693c821fbf84e32dd6ef62ddf14967845) )
	ROM_LOAD( "sou1",         0x0000, 0x0117, CRC(84f4b2fe) SHA1(dcc9e86cc36316fe42eace02d6df75d08bc8bb6d) )

	ROM_REGION( 0x0200, "bboardplds", 0 )
	ROM_LOAD( "rt24b.1a",     0x0000, 0x0117, CRC(54b85159) SHA1(c6f4fb5d747a215f4f50e4f2258e35d3f9bdbb2e) )
	ROM_LOAD( "iob1.11e",     0x0000, 0x0117, CRC(3abc0700) SHA1(973043aa46ec6d5d1db20dc9d5937005a0f9f6ae) )    /* seen the same pcb with LWIO.11E */

	ROM_REGION( 0x0200, "cboardplds", 0 )
	ROM_LOAD( "ioc1.ic1",     0x0000, 0x0117, CRC(0d182081) SHA1(475b3d417785da4bc512cce2b274bb00d4cc6792) )
ROM_END

/* B-Board 89625B-1 */
ROM_START( wonder3 )
	ROM_REGION( CODE_SIZE, "maincpu", 0 )      /* 68000 code */
	ROM_LOAD16_BYTE( "rtj_36.12f", 0x00000, 0x20000, CRC(e3741247) SHA1(4deb0f667697631693fbefddaeb8cf98fd0b90ce) )
	ROM_LOAD16_BYTE( "rtj_42.12h", 0x00001, 0x20000, CRC(b4baa117) SHA1(44486b3d50f9b0a8c32c2c2dc5f1a046aface7b6) )
	ROM_LOAD16_BYTE( "rtj_37.13f", 0x40000, 0x20000, CRC(a1f677b0) SHA1(e1511ea0fa4a689d1355119ac37c075192880dde) )
	ROM_LOAD16_BYTE( "rtj_43.13h", 0x40001, 0x20000, CRC(85337a47) SHA1(0a247aa56c5cc17b1e888df7b502f65e88715469) )
	ROM_LOAD16_BYTE( "rt_34.10f",  0x80000, 0x20000, CRC(054137c8) SHA1(e4c406e0a32198323a5931093fbaa6836510b8ad) ) // == rt_28a.9f
	ROM_LOAD16_BYTE( "rt_40.10h",  0x80001, 0x20000, CRC(7264cb1b) SHA1(b367acb9f6579569321ecaa98a14e29dd775b9db) ) // == rt_33a.9h
	ROM_LOAD16_BYTE( "rtj_35.11f", 0xc0000, 0x20000, CRC(e72f9ea3) SHA1(c63df200416bd61af73e8589204f7daef743041e) )
	ROM_LOAD16_BYTE( "rtj_41.11h", 0xc0001, 0x20000, CRC(a11ee998) SHA1(b892398e2ff4e40e51b858cfdbce866a75c670e6) )

	ROM_REGION( 0x400000, "gfx", 0 )
	ROMX_LOAD( "rt_09.4b",  0x000000, 0x20000, CRC(2c40e480) SHA1(823a36aa6dba7a028d4da5faef752366aa18fe57) , ROM_SKIP(7) ) // == rt-5m.7a
	ROMX_LOAD( "rt_01.4a",  0x000001, 0x20000, CRC(3e11f8cd) SHA1(88d7edd7102b1abec2b6f87029d39414f3ebc094) , ROM_SKIP(7) ) // == rt-5m.7a
	ROMX_LOAD( "rt_13.9b",  0x000002, 0x20000, CRC(51009117) SHA1(55549a2bd1abb3aea5dc17f17a2bd4a46c6cf12a) , ROM_SKIP(7) ) // == rt-7m.9a
	ROMX_LOAD( "rt_05.9a",  0x000003, 0x20000, CRC(283fd470) SHA1(2d7ff9c5a747565f2186b9252b703ebac3117beb) , ROM_SKIP(7) ) // == rt-7m.9a
	ROMX_LOAD( "rt_24.5e",  0x000004, 0x20000, CRC(ee4484ce) SHA1(5bb27c119a4e69cb50425ca73556a2e4e4482728) , ROM_SKIP(7) ) // == rt-1m.3a
	ROMX_LOAD( "rt_17.5c",  0x000005, 0x20000, CRC(e5dcddeb) SHA1(9a1c740cdd66ee2f58737eda613e79ff6357142a) , ROM_SKIP(7) ) // == rt-1m.3a
	ROMX_LOAD( "rt_38.8h",  0x000006, 0x20000, CRC(b2940c2d) SHA1(62ea57fb0203dd8f0e123e55eef7637702f8466d) , ROM_SKIP(7) ) // == rt-3m.5a
	ROMX_LOAD( "rt_32.8f",  0x000007, 0x20000, CRC(08e2b758) SHA1(f56a4f16454fe528c358fd212449a1ecb0f826e0) , ROM_SKIP(7) ) // == rt-3m.5a
	ROMX_LOAD( "rt_10.5b",  0x100000, 0x20000, CRC(e3f3ff94) SHA1(d5e46da2d25ca5347037b9859227c949209b30a6) , ROM_SKIP(7) ) // == rt-5m.7a
	ROMX_LOAD( "rt_02.5a",  0x100001, 0x20000, CRC(fcffd73c) SHA1(64830ffc053bd97f22b406f53b1e2e4a78db6a97) , ROM_SKIP(7) ) // == rt-5m.7a
	ROMX_LOAD( "rt_14.10b", 0x100002, 0x20000, CRC(5c546d9a) SHA1(1e0d0451e83dddb3371bffae6af7e17908816aec) , ROM_SKIP(7) ) // == rt-7m.9a
	ROMX_LOAD( "rt_06.10a", 0x100003, 0x20000, CRC(d9650bc4) SHA1(d28d85595bee9f6d4a697486a9db3a71ce60de50) , ROM_SKIP(7) ) // == rt-7m.9a
	ROMX_LOAD( "rt_25.7e",  0x100004, 0x20000, CRC(11b28831) SHA1(bb8f97871ca15184dbed3a90f8968a40b83a4480) , ROM_SKIP(7) ) // == rt-1m.3a
	ROMX_LOAD( "rt_18.7c",  0x100005, 0x20000, CRC(ce1afb7c) SHA1(50a330bb2d748f1a738fa7895aba81d9f0c14579) , ROM_SKIP(7) ) // == rt-1m.3a
	ROMX_LOAD( "rt_39.9h",  0x100006, 0x20000, CRC(ea7ac9ee) SHA1(bc21d8e59eb190608a87072c22be9cb1cf1227cc) , ROM_SKIP(7) ) // == rt-3m.5a
	ROMX_LOAD( "rt_33.9f",  0x100007, 0x20000, CRC(d6a99384) SHA1(552b012eb911b8739ee859af13e176a8396cecf2) , ROM_SKIP(7) ) // == rt-3m.5a
	ROMX_LOAD( "rt_11.7b",  0x200000, 0x20000, CRC(04f3c298) SHA1(1f3f8713ed8a2ad2bf4afce4c733eb9cb850ca9f) , ROM_SKIP(7) ) // == rt-6m.8a
	ROMX_LOAD( "rt_03.7a",  0x200001, 0x20000, CRC(98087e08) SHA1(6a13786a62e11d77c4da8469422e402df1299162) , ROM_SKIP(7) ) // == rt-6m.8a
	ROMX_LOAD( "rt_15.11b", 0x200002, 0x20000, CRC(b6aba565) SHA1(a166c853a5b4bc2602ce14974c11f570ba29df6a) , ROM_SKIP(7) ) // == rt-8m.10a
	ROMX_LOAD( "rt_07.11a", 0x200003, 0x20000, CRC(c62defa1) SHA1(2533f39251c99d5a184d72a5b96b5603466c0d11) , ROM_SKIP(7) ) // == rt-8m.10a
	ROMX_LOAD( "rt_26.8e",  0x200004, 0x20000, CRC(532f542e) SHA1(c894b385aa10a5e80b548c01817958739e2afa89) , ROM_SKIP(7) ) // == rt-2m.4a
	ROMX_LOAD( "rt_19.8c",  0x200005, 0x20000, CRC(1f0f72bd) SHA1(ad1afcde397a3273afc7c7a0b084a9b68e9e736e) , ROM_SKIP(7) ) // == rt-2m.4a
	ROMX_LOAD( "rt_28.10e", 0x200006, 0x20000, CRC(6064e499) SHA1(3bc30b9d8dde5f5e8dda31afbdadb5b2e4d50932) , ROM_SKIP(7) ) // == rt-4m.6a
	ROMX_LOAD( "rt_21.10c", 0x200007, 0x20000, CRC(20012ddc) SHA1(4389f2554c429f0a421425a6645dd8e719f4995f) , ROM_SKIP(7) ) // == rt-4m.6a
	ROMX_LOAD( "rt_12.8b",  0x300000, 0x20000, CRC(e54664cc) SHA1(e3b5ff0e9af20580cb4228f644f23a05aad20478) , ROM_SKIP(7) ) // == rt-6m.8a
	ROMX_LOAD( "rt_04.8a",  0x300001, 0x20000, CRC(4d7b9a1a) SHA1(1a9dd66bb97e2a02f3264d5766b674b588ad7dfc) , ROM_SKIP(7) ) // == rt-6m.8a
	ROMX_LOAD( "rt_16.12b", 0x300002, 0x20000, CRC(37c96cfc) SHA1(270f824757c0f536b02fef147d8e0af07e8d7147) , ROM_SKIP(7) ) // == rt-8m.10a
	ROMX_LOAD( "rt_08.12a", 0x300003, 0x20000, CRC(75f4975b) SHA1(1cd78828db97931ab0bfe0339e7051c58b3eff60) , ROM_SKIP(7) ) // == rt-8m.10a
	ROMX_LOAD( "rt_27.9e",  0x300004, 0x20000, CRC(ec6edc0f) SHA1(6dc13d692ca7bc989cd9b40bab8a2943425b7d61) , ROM_SKIP(7) ) // == rt-2m.4a
	ROMX_LOAD( "rt_20.9c",  0x300005, 0x20000, CRC(4fe52659) SHA1(1bf22ae192b57cd62e92f290313cc9d3234b2700) , ROM_SKIP(7) ) // == rt-2m.4a
	ROMX_LOAD( "rt_29.11e", 0x300006, 0x20000, CRC(8fa77f9f) SHA1(2f6b37d8e5eed38a8847c9ad736a7cdbe9958a70) , ROM_SKIP(7) ) // == rt-4m.6a
	ROMX_LOAD( "rt_22.11c", 0x300007, 0x20000, CRC(228a0d4a) SHA1(bcaf12d01abe1d3cd5731bd5341cb22c4ca6139e) , ROM_SKIP(7) ) // == rt-4m.6a

	ROM_REGION( 0x18000, "audiocpu", 0 ) /* 64k for the audio CPU (+banks) */
	ROM_LOAD( "rt_23.13b",        0x00000, 0x08000, CRC(abfca165) SHA1(428069d3bdc45775854cd0e8abe447f134fe5492) )    // == rt_9.12b
//  ROM_LOAD( "rt_23.13b",        0x00000, 0x08000, CRC(7d5a77a7) SHA1(4f2a6534d7a9e518061102c1dd7d7aca39600c33) )    /* dumped from another board, 1 byte different, pcb verified */
	ROM_CONTINUE(                 0x10000, 0x08000 )

	ROM_REGION( 0x40000, "oki", 0 ) /* Samples */
	ROM_LOAD( "rt_30.12c",  0x00000, 0x20000, CRC(26b211ab) SHA1(0ea03fdd9edff41eacfc52aa9e0421c10968356b) )    // == rt_18.11c
	ROM_LOAD( "rt_31.13c",  0x20000, 0x20000, CRC(dbe64ad0) SHA1(09f2ad522fe75d7bcca094b8c6696c3733b539d5) )    // == rt_19.12c

	ROM_REGION( 0x0200, "aboardplds", 0 )
	ROM_LOAD( "buf1",         0x0000, 0x0117, CRC(eb122de7) SHA1(b26b5bfe258e3e184f069719f9fd008d6b8f6b9b) )
	ROM_LOAD( "ioa1",         0x0000, 0x0117, CRC(59c7ee3b) SHA1(fbb887c5b4f5cb8df77cec710eaac2985bc482a6) )
	ROM_LOAD( "prg1",         0x0000, 0x0117, CRC(f1129744) SHA1(a5300f301c1a08a7da768f0773fa0fe3f683b237) )
	ROM_LOAD( "rom1",         0x0000, 0x0117, CRC(41dc73b9) SHA1(7d4c9f1693c821fbf84e32dd6ef62ddf14967845) )
	ROM_LOAD( "sou1",         0x0000, 0x0117, CRC(84f4b2fe) SHA1(dcc9e86cc36316fe42eace02d6df75d08bc8bb6d) )

	ROM_REGION( 0x0200, "bboardplds", 0 )
	ROM_LOAD( "rt22b.1a",     0x0000, 0x0117, CRC(89560d6a) SHA1(0f88920536eb131948339becb14557d77e02b9f8) )
	ROM_LOAD( "iob1.12e",     0x0000, 0x0117, CRC(3abc0700) SHA1(973043aa46ec6d5d1db20dc9d5937005a0f9f6ae) )

	ROM_REGION( 0x0200, "cboardplds", 0 )
	ROM_LOAD( "ioc1.ic1",     0x0000, 0x0117, CRC(0d182081) SHA1(475b3d417785da4bc512cce2b274bb00d4cc6792) )
ROM_END

/* Three Wonders (bootleg) */
ROM_START( 3wondersb )
	ROM_REGION( CODE_SIZE, "maincpu", 0 )      /* 68000 code */
	ROM_LOAD16_BYTE( "274001.4", 0x00000, 0x80000, CRC(47887cf3) SHA1(cdb2df67c99fd698845484869608bfaafa782bd6) )
	ROM_LOAD16_BYTE( "274001.3", 0x00001, 0x80000, CRC(e79eacb3) SHA1(cb2712b4e566d7d2ab5639f2e62f6e15244a7f09) )

	// The remainder of the roms in this set have the same content as those from '3wondersh'.
	ROM_REGION( 0x400000, "gfx", 0 )
	ROMX_LOAD( "274001.12", 0x000000, 0x80000, CRC(47cf8dfb) SHA1(05fed8e61d8f8c0382d820e7bdef36db85ad416b) , ROM_SKIP(7) )
	ROMX_LOAD( "274001.8",  0x000001, 0x80000, CRC(3bc2ef5e) SHA1(6864838e8f116bc56e7d205674240763a55d7907) , ROM_SKIP(7) )
	ROMX_LOAD( "274001.10", 0x000002, 0x80000, CRC(a0d27605) SHA1(7f2a7d3cc8802e778a98a29a4992a223932bff2c) , ROM_SKIP(7) )
	ROMX_LOAD( "274001.6",  0x000003, 0x80000, CRC(312d790c) SHA1(8739ba0784447f4050da1e6de8e31df3537b307d) , ROM_SKIP(7) )
	ROMX_LOAD( "274001.11", 0x000004, 0x80000, CRC(8112bbb4) SHA1(5a93cee8627afeb255eec1d26ec043cd2c4d2218) , ROM_SKIP(7) )
	ROMX_LOAD( "274001.7",  0x000005, 0x80000, CRC(58307167) SHA1(e253b64e144ccf3c4a0268e4be68468f11b2b071) , ROM_SKIP(7) )
	ROMX_LOAD( "274001.9",  0x000006, 0x80000, CRC(cb73759d) SHA1(e6fa263bcd7876f83f499fb95fbbc45ebce950fe) , ROM_SKIP(7) )
	ROMX_LOAD( "274001.5",  0x000007, 0x80000, CRC(3f765ae8) SHA1(a37cccbf2212eaa38a77b2f712688ce45b17e3b5) , ROM_SKIP(7) )

	ROM_REGION( 0x18000, "audiocpu", 0 ) /* 64k for the audio CPU (+banks) */
	ROM_LOAD( "27512.2",    0x00000, 0x08000, CRC(abfca165) SHA1(428069d3bdc45775854cd0e8abe447f134fe5492) )
	ROM_CONTINUE(           0x10000, 0x08000 )

	ROM_REGION( 0x40000, "oki", 0 ) /* Samples */
	ROM_LOAD( "27020.1",    0x00000, 0x40000, CRC(3c4348cf) SHA1(cfa9fd2f9692ddfef856106de191d3bdb5289297) )
ROM_END

/* Three Wonders (hack) */
ROM_START( 3wondersh )
	ROM_REGION( CODE_SIZE, "maincpu", 0 )      /* 68000 code */
	ROM_LOAD16_BYTE( "22.bin",      0x00000, 0x20000, CRC(e6071884) SHA1(5cf1a859739cf98846cb049b64fd083733acb29c) )
	ROM_LOAD16_BYTE( "26.bin",      0x00001, 0x20000, CRC(a28447b7) SHA1(e3f11911f1d3d115c03edf1bb6c8a68fccd9e9b3) )
	ROM_LOAD16_BYTE( "23.bin",      0x40000, 0x20000, CRC(fd3d6509) SHA1(0824ec397d12c2b832c9e694c23b59c2e489ed3b) )
	ROM_LOAD16_BYTE( "27.bin",      0x40001, 0x20000, CRC(999cba3d) SHA1(99dfb902c0c77f798a868eb13340eb54fb4a84d3) )
	ROM_LOAD16_BYTE( "rt_28a.9f",   0x80000, 0x20000, CRC(054137c8) SHA1(e4c406e0a32198323a5931093fbaa6836510b8ad) )
	ROM_LOAD16_BYTE( "rt_33a.9h",   0x80001, 0x20000, CRC(7264cb1b) SHA1(b367acb9f6579569321ecaa98a14e29dd775b9db) )
	ROM_LOAD16_BYTE( "rte_29a.10f", 0xc0000, 0x20000, CRC(cddaa919) SHA1(0c98e95ad5033d2c5ade7651243e7ccdb4e35463) )
	ROM_LOAD16_BYTE( "rte_34a.10h", 0xc0001, 0x20000, CRC(ed52e7e5) SHA1(352433ae484967d26376141e3a8a0f968b98fde6) )

	ROM_REGION( 0x400000, "gfx", 0 )
	ROMX_LOAD( "16.bin",    0x000000, 0x40000, CRC(c997bca2) SHA1(6b6cd9c2fc56690c42a3368cd5e6c98d6bff5b5b) , ROM_SKIP(7) )
	ROMX_LOAD( "6.bin",     0x000001, 0x40000, CRC(3eea321a) SHA1(4368aaf8a532c29f4b950adb2daedd3069d84cf1) , ROM_SKIP(7) )
	ROMX_LOAD( "18.bin",    0x000002, 0x40000, CRC(98acdfd4) SHA1(910dfd6742e166530388c700c0797e692e501f97) , ROM_SKIP(7) )
	ROMX_LOAD( "8.bin",     0x000003, 0x40000, CRC(dc9ca6f9) SHA1(77549e9a128d1b7ca0c4547cdc56f43450d426e0) , ROM_SKIP(7) )
	ROMX_LOAD( "12.bin",    0x000004, 0x40000, CRC(0d8a6007) SHA1(48d61cbd91df01ba4b2f0e80e60375a50ddb065e) , ROM_SKIP(7) )
	ROMX_LOAD( "2.bin",     0x000005, 0x40000, CRC(d75563b9) SHA1(7b7b105b84dc5d7b17838961fdd8be5bac90cbc6) , ROM_SKIP(7) )
	ROMX_LOAD( "14.bin",    0x000006, 0x40000, CRC(84369a28) SHA1(3877186371fe289522133fd99be034b141a974ca) , ROM_SKIP(7) )
	ROMX_LOAD( "4.bin",     0x000007, 0x40000, CRC(d4831578) SHA1(9ff5860f22976e9e4c023946f35e24fde84fe8ea) , ROM_SKIP(7) )
	ROMX_LOAD( "17.bin",    0x200000, 0x40000, CRC(040edff5) SHA1(9747d67b980cc357c6fa732300a84ae55150bc51) , ROM_SKIP(7) )
	ROMX_LOAD( "7.bin",     0x200001, 0x40000, CRC(c7c0468c) SHA1(e7a14cf579b023e8954b7e06aa2337db4f53bedc) , ROM_SKIP(7) )
	ROMX_LOAD( "19.bin",    0x200002, 0x40000, CRC(9fef114f) SHA1(394afb083ce7c46d9a39097d0040f9e18aaab508) , ROM_SKIP(7) )
	ROMX_LOAD( "9.bin",     0x200003, 0x40000, CRC(48cbfba5) SHA1(74047433e50795e29d8299526ae2c424610f0a5e) , ROM_SKIP(7) )
	ROMX_LOAD( "13.bin",    0x200004, 0x40000, CRC(8fc3d7d1) SHA1(e2784e0fccfe062ea8dc440e4a884fc665f4a846) , ROM_SKIP(7) )
	ROMX_LOAD( "3.bin",     0x200005, 0x40000, CRC(c65e9a86) SHA1(359ab1e2dd0fcf38ed9815a6a50294cbeca8223c) , ROM_SKIP(7) )
	ROMX_LOAD( "15.bin",    0x200006, 0x40000, CRC(f239341a) SHA1(b1858f5f7a5d210c5327b84a797ed7e898250596) , ROM_SKIP(7) )
	ROMX_LOAD( "5.bin",     0x200007, 0x40000, CRC(947ac944) SHA1(d962f49ba532fc60209bb3957ff8a456855ef67f) , ROM_SKIP(7) )

	ROM_REGION( 0x18000, "audiocpu", 0 ) /* 64k for the audio CPU (+banks) */
	ROM_LOAD( "rt_9.12b",   0x00000, 0x08000, CRC(abfca165) SHA1(428069d3bdc45775854cd0e8abe447f134fe5492) )
	ROM_CONTINUE(           0x10000, 0x08000 )

	ROM_REGION( 0x40000, "oki", 0 ) /* Samples */
	ROM_LOAD( "rt_18.11c",  0x00000, 0x20000, CRC(26b211ab) SHA1(0ea03fdd9edff41eacfc52aa9e0421c10968356b) )
	ROM_LOAD( "rt_19.12c",  0x20000, 0x20000, CRC(dbe64ad0) SHA1(09f2ad522fe75d7bcca094b8c6696c3733b539d5) )

	ROM_REGION( 0x0200, "aboardplds", 0 )
	ROM_LOAD( "buf1",         0x0000, 0x0117, CRC(eb122de7) SHA1(b26b5bfe258e3e184f069719f9fd008d6b8f6b9b) )
	ROM_LOAD( "ioa1",         0x0000, 0x0117, CRC(59c7ee3b) SHA1(fbb887c5b4f5cb8df77cec710eaac2985bc482a6) )
	ROM_LOAD( "prg1",         0x0000, 0x0117, CRC(f1129744) SHA1(a5300f301c1a08a7da768f0773fa0fe3f683b237) )
	ROM_LOAD( "rom1",         0x0000, 0x0117, CRC(41dc73b9) SHA1(7d4c9f1693c821fbf84e32dd6ef62ddf14967845) )
	ROM_LOAD( "sou1",         0x0000, 0x0117, CRC(84f4b2fe) SHA1(dcc9e86cc36316fe42eace02d6df75d08bc8bb6d) )

	ROM_REGION( 0x0200, "bboardplds", 0 )
	ROM_LOAD( "rt24b.1a",     0x0000, 0x0117, CRC(54b85159) SHA1(c6f4fb5d747a215f4f50e4f2258e35d3f9bdbb2e) )
	ROM_LOAD( "iob1.11e",     0x0000, 0x0117, CRC(3abc0700) SHA1(973043aa46ec6d5d1db20dc9d5937005a0f9f6ae) )

	ROM_REGION( 0x0200, "cboardplds", 0 )
	ROM_LOAD( "ioc1.ic1",     0x0000, 0x0117, CRC(0d182081) SHA1(475b3d417785da4bc512cce2b274bb00d4cc6792) )
ROM_END

/* B-Board 90629B-3 */
ROM_START( kod )
	ROM_REGION( CODE_SIZE, "maincpu", 0 )      /* 68000 code */
	ROM_LOAD16_BYTE( "kde_30a.11e", 0x00000, 0x20000, CRC(fcb5efe2) SHA1(6122e4852633876ff2ccd4b72296fce96446b3ee) )
	ROM_LOAD16_BYTE( "kde_37a.11f", 0x00001, 0x20000, CRC(f22e5266) SHA1(2fbadce701218f4a56bfd6dfd758abacb34a2181) )
	ROM_LOAD16_BYTE( "kde_31a.12e", 0x40000, 0x20000, CRC(c710d722) SHA1(a2e9b84d3e7d835a910ab9f584bdc64c2559995a) )
	ROM_LOAD16_BYTE( "kde_38a.12f", 0x40001, 0x20000, CRC(57d6ed3a) SHA1(a47da5068723c8e16ed458fbfa3e3db57b32d87d) )
	ROM_LOAD16_BYTE( "kd_28.9e",    0x80000, 0x20000, CRC(9367bcd9) SHA1(8243b4b9bb9756f3fa726717e19a166cb2f5b50a) )
	ROM_LOAD16_BYTE( "kd_35.9f",    0x80001, 0x20000, CRC(4ca6a48a) SHA1(9d440ecd8d2d0e293fecf64ca3915252b94e7aef) )
	ROM_LOAD16_BYTE( "kd_29.10e",   0xc0000, 0x20000, CRC(0360fa72) SHA1(274769c8717a874397cf37369e3ef80a682d9ef2) )
	ROM_LOAD16_BYTE( "kd_36a.10f",  0xc0001, 0x20000, CRC(95a3cef8) SHA1(9b75c1ed0eafacc230197ffd9b81e0c8f4f2c464) )    /* seen the same pcb with KDE_36A.10F */

	ROM_REGION( 0x400000, "gfx", 0 )
	ROMX_LOAD( "kd-5m.4a", 0x000000, 0x80000, CRC(e45b8701) SHA1(604e39e455e81695ee4f899f102d0bcd789cedd0) , ROM_GROUPWORD | ROM_SKIP(6) )  // in "2" socket
	ROMX_LOAD( "kd-7m.6a", 0x000002, 0x80000, CRC(a7750322) SHA1(3c583496a53cd64edf377db35f7f40f02b59b7e7) , ROM_GROUPWORD | ROM_SKIP(6) )  // in "4" socket
	ROMX_LOAD( "kd-1m.3a", 0x000004, 0x80000, CRC(5f74bf78) SHA1(b7c43eea9bf77a0fb571dcd53f8be719e6655fd9) , ROM_GROUPWORD | ROM_SKIP(6) )  // in "1" socket
	ROMX_LOAD( "kd-3m.5a", 0x000006, 0x80000, CRC(5e5303bf) SHA1(d9f90b898ffdf4398b2bbeb48247f06f728e7c00) , ROM_GROUPWORD | ROM_SKIP(6) )  // in "3" socket
	ROMX_LOAD( "kd-6m.4c", 0x200000, 0x80000, CRC(113358f3) SHA1(9d98eafa74a046f65bf3847fe1d88ea1b0c82b0c) , ROM_GROUPWORD | ROM_SKIP(6) )  // in "11" socket
	ROMX_LOAD( "kd-8m.6c", 0x200002, 0x80000, CRC(38853c44) SHA1(a6e552fb0138a76a7878b90d202904e2b44ae7ec) , ROM_GROUPWORD | ROM_SKIP(6) )  // in "13" socket
	ROMX_LOAD( "kd-2m.3c", 0x200004, 0x80000, CRC(9ef36604) SHA1(b42ca0a910b65e1e7bb6e7d734e853ce67e821bf) , ROM_GROUPWORD | ROM_SKIP(6) )  // in "10" socket
	ROMX_LOAD( "kd-4m.5c", 0x200006, 0x80000, CRC(402b9b4f) SHA1(4c11976976eadf1ad293b31b0a4d047d05032b06) , ROM_GROUPWORD | ROM_SKIP(6) )  // in "12" socket

	ROM_REGION( 0x18000, "audiocpu", 0 ) /* 64k for the audio CPU (+banks) */
	ROM_LOAD( "kd_9.12a",  0x00000, 0x08000, CRC(bac6ec26) SHA1(6cbb6d55660150ae3f5270e023328275ee1bbf50) )
	ROM_CONTINUE(          0x10000, 0x08000 )

	ROM_REGION( 0x40000, "oki", 0 ) /* Samples */
	ROM_LOAD( "kd_18.11c", 0x00000, 0x20000, CRC(4c63181d) SHA1(270f27534a95cb0be3ff3f9ca71c502320d8090b) )
	ROM_LOAD( "kd_19.12c", 0x20000, 0x20000, CRC(92941b80) SHA1(5fa7c2793e6febee54a83042d118ddd4f2b7d127) )

	ROM_REGION( 0x0200, "aboardplds", 0 )
	ROM_LOAD( "buf1",         0x0000, 0x0117, CRC(eb122de7) SHA1(b26b5bfe258e3e184f069719f9fd008d6b8f6b9b) )
	ROM_LOAD( "ioa1",         0x0000, 0x0117, CRC(59c7ee3b) SHA1(fbb887c5b4f5cb8df77cec710eaac2985bc482a6) )
	ROM_LOAD( "prg1",         0x0000, 0x0117, CRC(f1129744) SHA1(a5300f301c1a08a7da768f0773fa0fe3f683b237) )
	ROM_LOAD( "rom1",         0x0000, 0x0117, CRC(41dc73b9) SHA1(7d4c9f1693c821fbf84e32dd6ef62ddf14967845) )
	ROM_LOAD( "sou1",         0x0000, 0x0117, CRC(84f4b2fe) SHA1(dcc9e86cc36316fe42eace02d6df75d08bc8bb6d) )

	ROM_REGION( 0x0200, "bboardplds", 0 )
	ROM_LOAD( "kd29b.1a",     0x0000, 0x0117, NO_DUMP )
	ROM_LOAD( "iob1.11d",     0x0000, 0x0117, CRC(3abc0700) SHA1(973043aa46ec6d5d1db20dc9d5937005a0f9f6ae) )

	ROM_REGION( 0x0200, "cboardplds", 0 )
	ROM_LOAD( "ioc1.ic7",     0x0000, 0x0117, CRC(0d182081) SHA1(475b3d417785da4bc512cce2b274bb00d4cc6792) )
	ROM_LOAD( "c632.ic1",     0x0000, 0x0117, CRC(0fbd9270) SHA1(d7e737b20c44d41e29ca94be56114b31934dde81) )
ROM_END

/* B-Board 90629B-3 */
ROM_START( kodr1 )
	ROM_REGION( CODE_SIZE, "maincpu", 0 )      /* 68000 code */
	ROM_LOAD16_BYTE( "kde_30.11e", 0x00000, 0x20000, CRC(c7414fd4) SHA1(37d27fbe7c617a26b53bfdfcd532cf573d42f33b) )
	ROM_LOAD16_BYTE( "kde_37.11f", 0x00001, 0x20000, CRC(a5bf40d2) SHA1(cd34dbeabd0974709411579e669f01d0d799c2a1) )
	ROM_LOAD16_BYTE( "kde_31.12e", 0x40000, 0x20000, CRC(1fffc7bd) SHA1(822c9ad996ca51a99a2bb1fe08fa19e18413030d) )
	ROM_LOAD16_BYTE( "kde_38.12f", 0x40001, 0x20000, CRC(89e57a82) SHA1(aad35f86a8b1b7e3a0b5f3e6efd0e844b3d3d82f) )
	ROM_LOAD16_BYTE( "kde_28.9e",  0x80000, 0x20000, CRC(9367bcd9) SHA1(8243b4b9bb9756f3fa726717e19a166cb2f5b50a) ) // == kd_28.9e
	ROM_LOAD16_BYTE( "kde_35.9f",  0x80001, 0x20000, CRC(4ca6a48a) SHA1(9d440ecd8d2d0e293fecf64ca3915252b94e7aef) ) // == kd_35.9f
	ROM_LOAD16_BYTE( "kde_29.10e", 0xc0000, 0x20000, CRC(6a0ba878) SHA1(82e4037d73889a76b0cdc7a4f8e77e585d38e56e) )
	ROM_LOAD16_BYTE( "kde_36.10f", 0xc0001, 0x20000, CRC(b509b39d) SHA1(6023855e54b170e55abf0f607600031e19e5e722) )

	ROM_REGION( 0x400000, "gfx", 0 )
	ROMX_LOAD( "kd-5m.4a", 0x000000, 0x80000, CRC(e45b8701) SHA1(604e39e455e81695ee4f899f102d0bcd789cedd0) , ROM_GROUPWORD | ROM_SKIP(6) )  // in "2" socket
	ROMX_LOAD( "kd-7m.6a", 0x000002, 0x80000, CRC(a7750322) SHA1(3c583496a53cd64edf377db35f7f40f02b59b7e7) , ROM_GROUPWORD | ROM_SKIP(6) )  // in "4" socket
	ROMX_LOAD( "kd-1m.3a", 0x000004, 0x80000, CRC(5f74bf78) SHA1(b7c43eea9bf77a0fb571dcd53f8be719e6655fd9) , ROM_GROUPWORD | ROM_SKIP(6) )  // in "1" socket
	ROMX_LOAD( "kd-3m.5a", 0x000006, 0x80000, CRC(5e5303bf) SHA1(d9f90b898ffdf4398b2bbeb48247f06f728e7c00) , ROM_GROUPWORD | ROM_SKIP(6) )  // in "3" socket
	ROMX_LOAD( "kd-6m.4c", 0x200000, 0x80000, CRC(113358f3) SHA1(9d98eafa74a046f65bf3847fe1d88ea1b0c82b0c) , ROM_GROUPWORD | ROM_SKIP(6) )  // in "11" socket
	ROMX_LOAD( "kd-8m.6c", 0x200002, 0x80000, CRC(38853c44) SHA1(a6e552fb0138a76a7878b90d202904e2b44ae7ec) , ROM_GROUPWORD | ROM_SKIP(6) )  // in "13" socket
	ROMX_LOAD( "kd-2m.3c", 0x200004, 0x80000, CRC(9ef36604) SHA1(b42ca0a910b65e1e7bb6e7d734e853ce67e821bf) , ROM_GROUPWORD | ROM_SKIP(6) )  // in "10" socket
	ROMX_LOAD( "kd-4m.5c", 0x200006, 0x80000, CRC(402b9b4f) SHA1(4c11976976eadf1ad293b31b0a4d047d05032b06) , ROM_GROUPWORD | ROM_SKIP(6) )  // in "12" socket

	ROM_REGION( 0x18000, "audiocpu", 0 ) /* 64k for the audio CPU (+banks) */
	ROM_LOAD( "kd_9.12a",           0x00000, 0x08000, CRC(f5514510) SHA1(07e9c836adf9ef2f7e7729e99015f71e3b5f16e0) )    /* different CRC from kod, pcb verified */
	ROM_CONTINUE(                   0x10000, 0x08000 )

	ROM_REGION( 0x40000, "oki", 0 ) /* Samples */
	ROM_LOAD( "kd_18.11c", 0x00000, 0x20000, CRC(69ecb2c8) SHA1(fadf266b6b20bd6329a3e638918c5a3106413476) )    /* different CRC from kod, pcb verified */
	ROM_LOAD( "kd_19.12c", 0x20000, 0x20000, CRC(02d851c1) SHA1(c959a6fc3e7d893557f319debae91f28471f4be2) )    /* different CRC from kod, pcb verified */

	ROM_REGION( 0x0200, "aboardplds", 0 )
	ROM_LOAD( "buf1",         0x0000, 0x0117, CRC(eb122de7) SHA1(b26b5bfe258e3e184f069719f9fd008d6b8f6b9b) )
	ROM_LOAD( "ioa1",         0x0000, 0x0117, CRC(59c7ee3b) SHA1(fbb887c5b4f5cb8df77cec710eaac2985bc482a6) )
	ROM_LOAD( "prg1",         0x0000, 0x0117, CRC(f1129744) SHA1(a5300f301c1a08a7da768f0773fa0fe3f683b237) )
	ROM_LOAD( "rom1",         0x0000, 0x0117, CRC(41dc73b9) SHA1(7d4c9f1693c821fbf84e32dd6ef62ddf14967845) )
	ROM_LOAD( "sou1",         0x0000, 0x0117, CRC(84f4b2fe) SHA1(dcc9e86cc36316fe42eace02d6df75d08bc8bb6d) )

	ROM_REGION( 0x0200, "bboardplds", 0 )
	ROM_LOAD( "kd29b.1a",     0x0000, 0x0117, NO_DUMP )
	ROM_LOAD( "iob1.11d",     0x0000, 0x0117, CRC(3abc0700) SHA1(973043aa46ec6d5d1db20dc9d5937005a0f9f6ae) )

	ROM_REGION( 0x0200, "cboardplds", 0 )
	ROM_LOAD( "ioc1.ic7",     0x0000, 0x0117, CRC(0d182081) SHA1(475b3d417785da4bc512cce2b274bb00d4cc6792) )
	ROM_LOAD( "c632.ic1",     0x0000, 0x0117, CRC(0fbd9270) SHA1(d7e737b20c44d41e29ca94be56114b31934dde81) )
ROM_END

/* B-Board 90629B-3 */
ROM_START( kodu )
	ROM_REGION( CODE_SIZE, "maincpu", 0 )      /* 68000 code */
	ROM_LOAD16_BYTE( "kdu_30b.11e", 0x00000, 0x20000, CRC(825817f9) SHA1(250f61effcbe59f8b70baaf26eb8aef419fed66b) )
	ROM_LOAD16_BYTE( "kdu_37b.11f", 0x00001, 0x20000, CRC(d2422dfb) SHA1(6e369a62012f3c480755b700d4d4f4c112c79483) )
	ROM_LOAD16_BYTE( "kdu_31b.12e", 0x40000, 0x20000, CRC(9af36039) SHA1(f2645178a042689a387f916b4ecd7d1d859d758a) )
	ROM_LOAD16_BYTE( "kdu_38b.12f", 0x40001, 0x20000, CRC(be8405a1) SHA1(8d4f9a0489dc4b2971b20170713284151bc10eb7) )
	ROM_LOAD16_BYTE( "kdu_28.9e",   0x80000, 0x20000, CRC(9367bcd9) SHA1(8243b4b9bb9756f3fa726717e19a166cb2f5b50a) )    // == kd_28.9e
	ROM_LOAD16_BYTE( "kdu_35.9f",   0x80001, 0x20000, CRC(4ca6a48a) SHA1(9d440ecd8d2d0e293fecf64ca3915252b94e7aef) )    // == kd_35.9f
	ROM_LOAD16_BYTE( "kdu_29.10e",  0xc0000, 0x20000, CRC(0360fa72) SHA1(274769c8717a874397cf37369e3ef80a682d9ef2) )    // == kd_29.10e
	ROM_LOAD16_BYTE( "kdu_36a.10f", 0xc0001, 0x20000, CRC(95a3cef8) SHA1(9b75c1ed0eafacc230197ffd9b81e0c8f4f2c464) )    // == kd_36a.10f

	ROM_REGION( 0x400000, "gfx", 0 )
	ROMX_LOAD( "kd-5m.4a", 0x000000, 0x80000, CRC(e45b8701) SHA1(604e39e455e81695ee4f899f102d0bcd789cedd0) , ROM_GROUPWORD | ROM_SKIP(6) )  // in "2" socket
	ROMX_LOAD( "kd-7m.6a", 0x000002, 0x80000, CRC(a7750322) SHA1(3c583496a53cd64edf377db35f7f40f02b59b7e7) , ROM_GROUPWORD | ROM_SKIP(6) )  // in "4" socket
	ROMX_LOAD( "kd-1m.3a", 0x000004, 0x80000, CRC(5f74bf78) SHA1(b7c43eea9bf77a0fb571dcd53f8be719e6655fd9) , ROM_GROUPWORD | ROM_SKIP(6) )  // in "1" socket
	ROMX_LOAD( "kd-3m.5a", 0x000006, 0x80000, CRC(5e5303bf) SHA1(d9f90b898ffdf4398b2bbeb48247f06f728e7c00) , ROM_GROUPWORD | ROM_SKIP(6) )  // in "3" socket
	ROMX_LOAD( "kd-6m.4c", 0x200000, 0x80000, CRC(113358f3) SHA1(9d98eafa74a046f65bf3847fe1d88ea1b0c82b0c) , ROM_GROUPWORD | ROM_SKIP(6) )  // in "11" socket
	ROMX_LOAD( "kd-8m.6c", 0x200002, 0x80000, CRC(38853c44) SHA1(a6e552fb0138a76a7878b90d202904e2b44ae7ec) , ROM_GROUPWORD | ROM_SKIP(6) )  // in "13" socket
	ROMX_LOAD( "kd-2m.3c", 0x200004, 0x80000, CRC(9ef36604) SHA1(b42ca0a910b65e1e7bb6e7d734e853ce67e821bf) , ROM_GROUPWORD | ROM_SKIP(6) )  // in "10" socket
	ROMX_LOAD( "kd-4m.5c", 0x200006, 0x80000, CRC(402b9b4f) SHA1(4c11976976eadf1ad293b31b0a4d047d05032b06) , ROM_GROUPWORD | ROM_SKIP(6) )  // in "12" socket

	ROM_REGION( 0x18000, "audiocpu", 0 ) /* 64k for the audio CPU (+banks) */
	ROM_LOAD( "kd_09.12a", 0x00000, 0x08000, CRC(bac6ec26) SHA1(6cbb6d55660150ae3f5270e023328275ee1bbf50) ) // == kd_9.12a
	ROM_CONTINUE(          0x10000, 0x08000 )

	ROM_REGION( 0x40000, "oki", 0 ) /* Samples */
	ROM_LOAD( "kd_18.11c", 0x00000, 0x20000, CRC(4c63181d) SHA1(270f27534a95cb0be3ff3f9ca71c502320d8090b) )
	ROM_LOAD( "kd_19.12c", 0x20000, 0x20000, CRC(92941b80) SHA1(5fa7c2793e6febee54a83042d118ddd4f2b7d127) )

	ROM_REGION( 0x0200, "aboardplds", 0 )
	ROM_LOAD( "buf1",         0x0000, 0x0117, CRC(eb122de7) SHA1(b26b5bfe258e3e184f069719f9fd008d6b8f6b9b) )
	ROM_LOAD( "ioa1",         0x0000, 0x0117, CRC(59c7ee3b) SHA1(fbb887c5b4f5cb8df77cec710eaac2985bc482a6) )
	ROM_LOAD( "prg1",         0x0000, 0x0117, CRC(f1129744) SHA1(a5300f301c1a08a7da768f0773fa0fe3f683b237) )
	ROM_LOAD( "rom1",         0x0000, 0x0117, CRC(41dc73b9) SHA1(7d4c9f1693c821fbf84e32dd6ef62ddf14967845) )
	ROM_LOAD( "sou1",         0x0000, 0x0117, CRC(84f4b2fe) SHA1(dcc9e86cc36316fe42eace02d6df75d08bc8bb6d) )

	ROM_REGION( 0x0200, "bboardplds", 0 )
	ROM_LOAD( "kd29b.1a",     0x0000, 0x0117, NO_DUMP )
	ROM_LOAD( "iob1.11d",     0x0000, 0x0117, CRC(3abc0700) SHA1(973043aa46ec6d5d1db20dc9d5937005a0f9f6ae) )

	ROM_REGION( 0x0200, "cboardplds", 0 )
	ROM_LOAD( "ioc1.ic7",     0x0000, 0x0117, CRC(0d182081) SHA1(475b3d417785da4bc512cce2b274bb00d4cc6792) )
	ROM_LOAD( "c632.ic1",     0x0000, 0x0117, CRC(0fbd9270) SHA1(d7e737b20c44d41e29ca94be56114b31934dde81) )
ROM_END

/* B-Board 90629B-3 */
ROM_START( kodj )
	ROM_REGION( CODE_SIZE, "maincpu", 0 )      /* 68000 code */
	ROM_LOAD16_BYTE( "kdj_30a.11e",  0x00000, 0x20000, CRC(ebc788ad) SHA1(f4a297e409fcdbb2c15a13b39a16e4a83e7e060b) )
	ROM_LOAD16_BYTE( "kdj_37a.11f",  0x00001, 0x20000, CRC(e55c3529) SHA1(a5254895499a53b4fbaac6fd50464b9e08175b8d) )
	ROM_LOAD16_BYTE( "kdj_31a.12e",  0x40000, 0x20000, CRC(c710d722) SHA1(a2e9b84d3e7d835a910ab9f584bdc64c2559995a) )   // == kde_31a.12e
	ROM_LOAD16_BYTE( "kdj_38a.12f",  0x40001, 0x20000, CRC(57d6ed3a) SHA1(a47da5068723c8e16ed458fbfa3e3db57b32d87d) )   // == kde_38a.12f
	ROM_LOAD16_WORD_SWAP("kd_33.6f", 0x80000, 0x80000, CRC(9bd7ad4b) SHA1(7bece5d408fd13116bd5518014b632ecc9a2feaa) )   // incorrect label, could be kdj_33a, kdj_33, kd_33a

	ROM_REGION( 0x400000, "gfx", 0 )
	ROMX_LOAD( "kd_06.8a",  0x000000, 0x80000, CRC(e45b8701) SHA1(604e39e455e81695ee4f899f102d0bcd789cedd0) , ROM_GROUPWORD | ROM_SKIP(6) ) // == kd-5m.4a
	ROMX_LOAD( "kd_08.10a", 0x000002, 0x80000, CRC(a7750322) SHA1(3c583496a53cd64edf377db35f7f40f02b59b7e7) , ROM_GROUPWORD | ROM_SKIP(6) ) // == kd-7m.6a
	ROMX_LOAD( "kd_05.7a",  0x000004, 0x80000, CRC(5f74bf78) SHA1(b7c43eea9bf77a0fb571dcd53f8be719e6655fd9) , ROM_GROUPWORD | ROM_SKIP(6) ) // == kd-1m.3a
	ROMX_LOAD( "kd_07.9a",  0x000006, 0x80000, CRC(5e5303bf) SHA1(d9f90b898ffdf4398b2bbeb48247f06f728e7c00) , ROM_GROUPWORD | ROM_SKIP(6) ) // == kd-3m.5a
	ROMX_LOAD( "kd_15.8c",  0x200000, 0x80000, CRC(113358f3) SHA1(9d98eafa74a046f65bf3847fe1d88ea1b0c82b0c) , ROM_GROUPWORD | ROM_SKIP(6) ) // == kd-6m.4c
	ROMX_LOAD( "kd_17.10c", 0x200002, 0x80000, CRC(38853c44) SHA1(a6e552fb0138a76a7878b90d202904e2b44ae7ec) , ROM_GROUPWORD | ROM_SKIP(6) ) // == kd-8m.6c
	ROMX_LOAD( "kd_14.7c",  0x200004, 0x80000, CRC(9ef36604) SHA1(b42ca0a910b65e1e7bb6e7d734e853ce67e821bf) , ROM_GROUPWORD | ROM_SKIP(6) ) // == kd-2m.3c
	ROMX_LOAD( "kd_16.9c",  0x200006, 0x80000, CRC(402b9b4f) SHA1(4c11976976eadf1ad293b31b0a4d047d05032b06) , ROM_GROUPWORD | ROM_SKIP(6) ) // == kd-4m.5c

	ROM_REGION( 0x18000, "audiocpu", 0 ) /* 64k for the audio CPU (+banks) */
	ROM_LOAD( "kd_09.12a",  0x00000, 0x08000, CRC(bac6ec26) SHA1(6cbb6d55660150ae3f5270e023328275ee1bbf50) )    // == kd_9.12a
	ROM_CONTINUE(           0x10000, 0x08000 )

	ROM_REGION( 0x40000, "oki", 0 ) /* Samples */
	ROM_LOAD( "kd_18.11c",  0x00000, 0x20000, CRC(4c63181d) SHA1(270f27534a95cb0be3ff3f9ca71c502320d8090b) )
	ROM_LOAD( "kd_19.12c",  0x20000, 0x20000, CRC(92941b80) SHA1(5fa7c2793e6febee54a83042d118ddd4f2b7d127) )

	ROM_REGION( 0x0200, "aboardplds", 0 )
	ROM_LOAD( "buf1",         0x0000, 0x0117, CRC(eb122de7) SHA1(b26b5bfe258e3e184f069719f9fd008d6b8f6b9b) )
	ROM_LOAD( "ioa1",         0x0000, 0x0117, CRC(59c7ee3b) SHA1(fbb887c5b4f5cb8df77cec710eaac2985bc482a6) )
	ROM_LOAD( "prg1",         0x0000, 0x0117, CRC(f1129744) SHA1(a5300f301c1a08a7da768f0773fa0fe3f683b237) )
	ROM_LOAD( "rom1",         0x0000, 0x0117, CRC(41dc73b9) SHA1(7d4c9f1693c821fbf84e32dd6ef62ddf14967845) )
	ROM_LOAD( "sou1",         0x0000, 0x0117, CRC(84f4b2fe) SHA1(dcc9e86cc36316fe42eace02d6df75d08bc8bb6d) )

	ROM_REGION( 0x0200, "bboardplds", 0 )
	ROM_LOAD( "kd29b.1a",     0x0000, 0x0117, NO_DUMP )
	ROM_LOAD( "iob1.11d",     0x0000, 0x0117, CRC(3abc0700) SHA1(973043aa46ec6d5d1db20dc9d5937005a0f9f6ae) )

	ROM_REGION( 0x0200, "cboardplds", 0 )
	ROM_LOAD( "ioc1.ic7",     0x0000, 0x0117, CRC(0d182081) SHA1(475b3d417785da4bc512cce2b274bb00d4cc6792) )
	ROM_LOAD( "c632.ic1",     0x0000, 0x0117, CRC(0fbd9270) SHA1(d7e737b20c44d41e29ca94be56114b31934dde81) )
ROM_END

/* B-Board 89625B-1 */
/* Note that this set is equivalent to kodj, but each 4Mbit EPROM is replaced by 4 1Mbit EPROMs. */
ROM_START( kodja )
	ROM_REGION( CODE_SIZE, "maincpu", 0 )      /* 68000 code */
	ROM_LOAD16_BYTE( "kdj_36a.12f", 0x00000, 0x20000, CRC(ebc788ad) SHA1(f4a297e409fcdbb2c15a13b39a16e4a83e7e060b) )    // == kdj_30a.11e
	ROM_LOAD16_BYTE( "kdj_42a.12h", 0x00001, 0x20000, CRC(e55c3529) SHA1(a5254895499a53b4fbaac6fd50464b9e08175b8d) )    // == kdj_37a.11f
	ROM_LOAD16_BYTE( "kdj_37a.13f", 0x40000, 0x20000, CRC(c710d722) SHA1(a2e9b84d3e7d835a910ab9f584bdc64c2559995a) )    // == kdj_31a.12e
	ROM_LOAD16_BYTE( "kdj_43a.13h", 0x40001, 0x20000, CRC(57d6ed3a) SHA1(a47da5068723c8e16ed458fbfa3e3db57b32d87d) )    // == kdj_38a.12f
	ROM_LOAD16_BYTE( "kd_34.10f",   0x80000, 0x20000, CRC(9367bcd9) SHA1(8243b4b9bb9756f3fa726717e19a166cb2f5b50a) )    // == kd_33.6f
	ROM_LOAD16_BYTE( "kd_40.10h",   0x80001, 0x20000, CRC(4ca6a48a) SHA1(9d440ecd8d2d0e293fecf64ca3915252b94e7aef) )    // == kd_33.6f
	ROM_LOAD16_BYTE( "kd_35.11f",   0xc0000, 0x20000, CRC(0360fa72) SHA1(274769c8717a874397cf37369e3ef80a682d9ef2) )    // == kd_33.6f
	ROM_LOAD16_BYTE( "kd_41a.11h",  0xc0001, 0x20000, CRC(95a3cef8) SHA1(9b75c1ed0eafacc230197ffd9b81e0c8f4f2c464) )    // == kd_33.6f

	ROM_REGION( 0x400000, "gfx", 0 )
	ROMX_LOAD( "kd_9.4b",   0x000000, 0x20000, CRC(401a98e3) SHA1(b762d0f0e38ad33b5135646a107c1d4279c02193) , ROM_SKIP(7) ) // == kd_06.8a
	ROMX_LOAD( "kd_1.4a",   0x000001, 0x20000, CRC(5894399a) SHA1(ee0bff29d313b4d5a318bc7345cc356d6e7318fc) , ROM_SKIP(7) ) // == kd_06.8a
	ROMX_LOAD( "kd_13.9b",  0x000002, 0x20000, CRC(b6685131) SHA1(c1fa66cb695cfcb61e8495031ce18e4c6736b561) , ROM_SKIP(7) ) // == kd_08.10a
	ROMX_LOAD( "kd_5.9a",   0x000003, 0x20000, CRC(c29b9ab3) SHA1(3ffa906d22563c9ad29b9ba4cd22b4ce5605b66a) , ROM_SKIP(7) ) // == kd_08.10a
	ROMX_LOAD( "kd_24.5e",  0x000004, 0x20000, CRC(97008fdb) SHA1(150a7907f48dede2de0c40f1bea46e95d2e31e9c) , ROM_SKIP(7) ) // == kd_05.7a
	ROMX_LOAD( "kd_17.5c",  0x000005, 0x20000, CRC(dc9a83d3) SHA1(6ceb5e6b11e7a57933c03fb308e1c6a850cf5ac7) , ROM_SKIP(7) ) // == kd_05.7a
	ROMX_LOAD( "kd_38.8h",  0x000006, 0x20000, CRC(9c3dd2d1) SHA1(3ab77954fbde45797fcb1c5d8a500b5ec2673e5f) , ROM_SKIP(7) ) // == kd_07.9a
	ROMX_LOAD( "kd_32.8f",  0x000007, 0x20000, CRC(1b2a802a) SHA1(f9b9e6da6a49aaf7c88b7b806c273b464a584a7b) , ROM_SKIP(7) ) // == kd_07.9a
	ROMX_LOAD( "kd_10.5b",  0x100000, 0x20000, CRC(e788ae96) SHA1(1c257a4c20842b8750e3ba3cf78adad0bc5e6f2c) , ROM_SKIP(7) ) // == kd_06.8a
	ROMX_LOAD( "kd_2.5a",   0x100001, 0x20000, CRC(b022e3e3) SHA1(a1e3700638e99677d4ee9aea5a250eb2380413c2) , ROM_SKIP(7) ) // == kd_06.8a
	ROMX_LOAD( "kd_14.10b", 0x100002, 0x20000, CRC(4840c5ef) SHA1(fcd7ee778ab4519887bd37ced5e04f7aa8748e5e) , ROM_SKIP(7) ) // == kd_08.10a
	ROMX_LOAD( "kd_6.10a",  0x100003, 0x20000, CRC(519faee4) SHA1(038afb9d2df872a0b45a7a6a43f7bb1389e8539a) , ROM_SKIP(7) ) // == kd_08.10a
	ROMX_LOAD( "kd_25.7e",  0x100004, 0x20000, CRC(5d0fa853) SHA1(81e69bfac5ad6909d47db0927b7387bd392b4ef3) , ROM_SKIP(7) ) // == kd_05.7a
	ROMX_LOAD( "kd_18.7c",  0x100005, 0x20000, CRC(6ad3b2bb) SHA1(c8d3bd2ce6305772224b5419301b59b9e78e7d44) , ROM_SKIP(7) ) // == kd_05.7a
	ROMX_LOAD( "kd_39.9h",  0x100006, 0x20000, CRC(d7920213) SHA1(ac7de3f6c8503e06ae27f7db43bf9884f244bf7a) , ROM_SKIP(7) ) // == kd_07.9a
	ROMX_LOAD( "kd_33.9f",  0x100007, 0x20000, CRC(65c2bed6) SHA1(932be92a418a2d3960f0b071eb5ffe6dae60d933) , ROM_SKIP(7) ) // == kd_07.9a
	ROMX_LOAD( "kd_11.7b",  0x200000, 0x20000, CRC(147e3310) SHA1(e371db6c6621d09c43325edf9e3e40777afb830c) , ROM_SKIP(7) ) // == kd_15.8c
	ROMX_LOAD( "kd_3.7a",   0x200001, 0x20000, CRC(5d18bc83) SHA1(49191e1073cdfb32272e6daeddedc4b309cf9fe6) , ROM_SKIP(7) ) // == kd_15.8c
	ROMX_LOAD( "kd_15.11b", 0x200002, 0x20000, CRC(57359746) SHA1(2f83b77e6ebbceffab10653018a1b54a91d39a3f) , ROM_SKIP(7) ) // == kd_17.10c
	ROMX_LOAD( "kd_7.11a",  0x200003, 0x20000, CRC(7fe03079) SHA1(be3be96d63a53cd8b6dc67eadc718c4b3b182123) , ROM_SKIP(7) ) // == kd_17.10c
	ROMX_LOAD( "kd_26.8e",  0x200004, 0x20000, CRC(57e5fab5) SHA1(e665ab581372b48b6a7a494fd65ed6c807b5c00a) , ROM_SKIP(7) ) // == kd_14.7c
	ROMX_LOAD( "kd_19.8c",  0x200005, 0x20000, CRC(b1f30f7c) SHA1(80308091a0af6ca46011a9174838c4d9dbeb96ee) , ROM_SKIP(7) ) // == kd_14.7c
	ROMX_LOAD( "kd_28.10e", 0x200006, 0x20000, CRC(3a424135) SHA1(05d77253f454d69553a39a5dca0798f6ccea6da0) , ROM_SKIP(7) ) // == kd_16.9c
	ROMX_LOAD( "kd_21.10c", 0x200007, 0x20000, CRC(ce10d2c3) SHA1(b694325b370d451cb3bcfe7d753c53ef68b83276) , ROM_SKIP(7) ) // == kd_16.9c
	ROMX_LOAD( "kd_12.8b",  0x300000, 0x20000, CRC(a6042aa2) SHA1(daa4e52574b53d125414401ad0fca6eea36abe64) , ROM_SKIP(7) ) // == kd_15.8c
	ROMX_LOAD( "kd_4.8a",   0x300001, 0x20000, CRC(0ce0ba30) SHA1(a19049b5fc58921452bf7b69d2d86d0faa5362bf) , ROM_SKIP(7) ) // == kd_15.8c
	ROMX_LOAD( "kd_16.12b", 0x300002, 0x20000, CRC(63dcb7e0) SHA1(1524321f31813452aabfef133fb64fd0cef165cf) , ROM_SKIP(7) ) // == kd_17.10c
	ROMX_LOAD( "kd_8.12a",  0x300003, 0x20000, CRC(c69b77ae) SHA1(31cacace828d4e376347ecd896f3454f79285a8b) , ROM_SKIP(7) ) // == kd_17.10c
	ROMX_LOAD( "kd_27.9e",  0x300004, 0x20000, CRC(40d7bfed) SHA1(924d3f981d85434d2a22d4ede45131720d7521c0) , ROM_SKIP(7) ) // == kd_14.7c
	ROMX_LOAD( "kd_20.9c",  0x300005, 0x20000, CRC(01c1f399) SHA1(ea25d4efc9e538ee8ed09acb6e467c5b444bf728) , ROM_SKIP(7) ) // == kd_14.7c
	ROMX_LOAD( "kd_29.11e", 0x300006, 0x20000, CRC(a1eeac03) SHA1(fdec33ec0becc4badf89c9019f3217c22e54e842) , ROM_SKIP(7) ) // == kd_16.9c
	ROMX_LOAD( "kd_22.11c", 0x300007, 0x20000, CRC(5ade98eb) SHA1(5d2474864bf0e41e89fe44c69025101b8ef2b482) , ROM_SKIP(7) ) // == kd_16.9c

	ROM_REGION( 0x18000, "audiocpu", 0 ) /* 64k for the audio CPU (+banks) */
	ROM_LOAD( "kd_23.13b",  0x00000, 0x08000, CRC(bac6ec26) SHA1(6cbb6d55660150ae3f5270e023328275ee1bbf50) )    // == kd_09.12a
	ROM_CONTINUE(           0x10000, 0x08000 )

	ROM_REGION( 0x40000, "oki", 0 ) /* Samples */
	ROM_LOAD( "kd_30.12c",  0x00000, 0x20000, CRC(4c63181d) SHA1(270f27534a95cb0be3ff3f9ca71c502320d8090b) )    // == kd_18.11c
	ROM_LOAD( "kd_31.13c",  0x20000, 0x20000, CRC(92941b80) SHA1(5fa7c2793e6febee54a83042d118ddd4f2b7d127) )    // == kd_19.12c

	ROM_REGION( 0x0200, "aboardplds", 0 )
	ROM_LOAD( "buf1",         0x0000, 0x0117, CRC(eb122de7) SHA1(b26b5bfe258e3e184f069719f9fd008d6b8f6b9b) )
	ROM_LOAD( "ioa1",         0x0000, 0x0117, CRC(59c7ee3b) SHA1(fbb887c5b4f5cb8df77cec710eaac2985bc482a6) )
	ROM_LOAD( "prg1",         0x0000, 0x0117, CRC(f1129744) SHA1(a5300f301c1a08a7da768f0773fa0fe3f683b237) )
	ROM_LOAD( "rom1",         0x0000, 0x0117, CRC(41dc73b9) SHA1(7d4c9f1693c821fbf84e32dd6ef62ddf14967845) )
	ROM_LOAD( "sou1",         0x0000, 0x0117, CRC(84f4b2fe) SHA1(dcc9e86cc36316fe42eace02d6df75d08bc8bb6d) )

	ROM_REGION( 0x0200, "bboardplds", 0 )
	ROM_LOAD( "kd22b.1a",     0x0000, 0x0117, NO_DUMP )
	ROM_LOAD( "iob1.12e",     0x0000, 0x0117, CRC(3abc0700) SHA1(973043aa46ec6d5d1db20dc9d5937005a0f9f6ae) )

	ROM_REGION( 0x0200, "cboardplds", 0 )
	ROM_LOAD( "ioc1.ic7",     0x0000, 0x0117, CRC(0d182081) SHA1(475b3d417785da4bc512cce2b274bb00d4cc6792) )
	ROM_LOAD( "c632.ic1",     0x0000, 0x0117, CRC(0fbd9270) SHA1(d7e737b20c44d41e29ca94be56114b31934dde81) )
ROM_END

/* B-Board 91635B-2 */
ROM_START( captcomm )
	ROM_REGION( CODE_SIZE, "maincpu", 0 )      /* 68000 code */
	ROM_LOAD16_WORD_SWAP( "cce_23f.8f", 0x000000, 0x80000, CRC(42c814c5) SHA1(60e6ae6b8a89cdaa1abf1749c60a5e3b3972e1ab) )
	ROM_LOAD16_WORD_SWAP( "cc_22f.7f",  0x080000, 0x80000, CRC(0fd34195) SHA1(fb2b9a53af43507f13c4f94eaebbf0b538b2e754) )
	ROM_LOAD16_BYTE( "cc_24f.9e",       0x100000, 0x20000, CRC(3a794f25) SHA1(7f3722a4ef0c1d7acb73e6bac9dd6ae7b35e6374) )
	ROM_LOAD16_BYTE( "cc_28f.9f",       0x100001, 0x20000, CRC(fc3c2906) SHA1(621c3b79b6fdea1665bb316eb539e5916e890656) )

	ROM_REGION( 0x400000, "gfx", 0 )
	ROMX_LOAD( "cc-5m.3a",  0x000000, 0x80000, CRC(7261d8ba) SHA1(4b66292e42d20d0b79a756f0e445492ddb9c6bbc) , ROM_GROUPWORD | ROM_SKIP(6) ) // in "1" socket
	ROMX_LOAD( "cc-7m.5a",  0x000002, 0x80000, CRC(6a60f949) SHA1(87391ff92abaf3e451f70d789a938cffbd1fd222) , ROM_GROUPWORD | ROM_SKIP(6) ) // in "3" socket
	ROMX_LOAD( "cc-1m.4a",  0x000004, 0x80000, CRC(00637302) SHA1(2c554b59cceec2de67a9a4bc6281fe846d3c8cd2) , ROM_GROUPWORD | ROM_SKIP(6) ) // in "2" socket
	ROMX_LOAD( "cc-3m.6a",  0x000006, 0x80000, CRC(cc87cf61) SHA1(7fb1f49494cc1a08aded20754bb0cefb1c323198) , ROM_GROUPWORD | ROM_SKIP(6) ) // in "4" socket
	ROMX_LOAD( "cc-6m.7a",  0x200000, 0x80000, CRC(28718bed) SHA1(dfdc4dd14dc609783bad94d608a9e9b137dea944) , ROM_GROUPWORD | ROM_SKIP(6) ) // in "5" socket
	ROMX_LOAD( "cc-8m.9a",  0x200002, 0x80000, CRC(d4acc53a) SHA1(d03282ebbde362e679cc97f772aa9baf163d7606) , ROM_GROUPWORD | ROM_SKIP(6) ) // in "7" socket
	ROMX_LOAD( "cc-2m.8a",  0x200004, 0x80000, CRC(0c69f151) SHA1(a170b8e568439e4a26d84376d53560e4248e4e2f) , ROM_GROUPWORD | ROM_SKIP(6) ) // in "6" socket
	ROMX_LOAD( "cc-4m.10a", 0x200006, 0x80000, CRC(1f9ebb97) SHA1(023d00cb7b6a52d1b29e2052abe08ef34cb0c55c) , ROM_GROUPWORD | ROM_SKIP(6) ) // in "8" socket

	ROM_REGION( 0x18000, "audiocpu", 0 ) /* 64k for the audio CPU (+banks) */
	ROM_LOAD( "cc_09.11a",  0x00000, 0x08000, CRC(698e8b58) SHA1(b7a3d905a7ed2c430426ca2e185e3d7e75e752a1) )
	ROM_CONTINUE(           0x10000, 0x08000 )

	ROM_REGION( 0x40000, "oki", 0 ) /* Samples */
	ROM_LOAD( "cc_18.11c",  0x00000, 0x20000, CRC(6de2c2db) SHA1(9a1eaba8d104f59a5e61f89679bb5de0c0c64364) )
	ROM_LOAD( "cc_19.12c",  0x20000, 0x20000, CRC(b99091ae) SHA1(b19197c7ad3aeaf5f41c26bf853b0c9b502ecfca) )

	ROM_REGION( 0x0200, "aboardplds", 0 )
	ROM_LOAD( "buf1",         0x0000, 0x0117, CRC(eb122de7) SHA1(b26b5bfe258e3e184f069719f9fd008d6b8f6b9b) )
	ROM_LOAD( "ioa1",         0x0000, 0x0117, CRC(59c7ee3b) SHA1(fbb887c5b4f5cb8df77cec710eaac2985bc482a6) )
	ROM_LOAD( "prg1",         0x0000, 0x0117, CRC(f1129744) SHA1(a5300f301c1a08a7da768f0773fa0fe3f683b237) )
	ROM_LOAD( "rom1",         0x0000, 0x0117, CRC(41dc73b9) SHA1(7d4c9f1693c821fbf84e32dd6ef62ddf14967845) )
	ROM_LOAD( "sou1",         0x0000, 0x0117, CRC(84f4b2fe) SHA1(dcc9e86cc36316fe42eace02d6df75d08bc8bb6d) )

	ROM_REGION( 0x0200, "bboardplds", 0 )
	ROM_LOAD( "cc63b.1a",     0x0000, 0x0117, CRC(cae8f0f9) SHA1(eadbd45e184195b2d170cd71a68e5caed64b69f7) )
	ROM_LOAD( "iob1.12d",     0x0000, 0x0117, CRC(3abc0700) SHA1(973043aa46ec6d5d1db20dc9d5937005a0f9f6ae) )
	ROM_LOAD( "ccprg1.11d",   0x0000, 0x0117, CRC(e1c225c4) SHA1(97146451ca9aa3cecd443cc6881151ed8df47fbf) )

	ROM_REGION( 0x0200, "cboardplds", 0 )
	ROM_LOAD( "ioc1.ic7",     0x0000, 0x0117, CRC(0d182081) SHA1(475b3d417785da4bc512cce2b274bb00d4cc6792) )
	ROM_LOAD( "c632b.ic1",    0x0000, 0x0117, CRC(0fbd9270) SHA1(d7e737b20c44d41e29ca94be56114b31934dde81) )
ROM_END

/* B-Board 91635B-2 */
ROM_START( captcommr1 )
	ROM_REGION( CODE_SIZE, "maincpu", 0 )      /* 68000 code */
	ROM_LOAD16_WORD_SWAP( "cce_23d.8f", 0x000000, 0x80000, CRC(19c58ece) SHA1(6e23e87db29b2c5698b7cead99d1106a2e190648) )
	ROM_LOAD16_WORD_SWAP( "cc_22d.7f",  0x080000, 0x80000, CRC(a91949b7) SHA1(c027af89cd8f6bd3aaed61114582322c42e0c74f) )
	ROM_LOAD16_BYTE( "cc_24d.9e",       0x100000, 0x20000, CRC(680e543f) SHA1(cfa963ab6329f615807db213bf53841860ed3149) )
	ROM_LOAD16_BYTE( "cc_28d.9f",       0x100001, 0x20000, CRC(8820039f) SHA1(d68ce0b34ade75b8c5214168b2b1e0cdff45cd52) )

	ROM_REGION( 0x400000, "gfx", 0 )
	ROMX_LOAD( "cc-5m.3a",  0x000000, 0x80000, CRC(7261d8ba) SHA1(4b66292e42d20d0b79a756f0e445492ddb9c6bbc) , ROM_GROUPWORD | ROM_SKIP(6) ) // in "1" socket
	ROMX_LOAD( "cc-7m.5a",  0x000002, 0x80000, CRC(6a60f949) SHA1(87391ff92abaf3e451f70d789a938cffbd1fd222) , ROM_GROUPWORD | ROM_SKIP(6) ) // in "3" socket
	ROMX_LOAD( "cc-1m.4a",  0x000004, 0x80000, CRC(00637302) SHA1(2c554b59cceec2de67a9a4bc6281fe846d3c8cd2) , ROM_GROUPWORD | ROM_SKIP(6) ) // in "2" socket
	ROMX_LOAD( "cc-3m.6a",  0x000006, 0x80000, CRC(cc87cf61) SHA1(7fb1f49494cc1a08aded20754bb0cefb1c323198) , ROM_GROUPWORD | ROM_SKIP(6) ) // in "4" socket
	ROMX_LOAD( "cc-6m.7a",  0x200000, 0x80000, CRC(28718bed) SHA1(dfdc4dd14dc609783bad94d608a9e9b137dea944) , ROM_GROUPWORD | ROM_SKIP(6) ) // in "5" socket
	ROMX_LOAD( "cc-8m.9a",  0x200002, 0x80000, CRC(d4acc53a) SHA1(d03282ebbde362e679cc97f772aa9baf163d7606) , ROM_GROUPWORD | ROM_SKIP(6) ) // in "7" socket
	ROMX_LOAD( "cc-2m.8a",  0x200004, 0x80000, CRC(0c69f151) SHA1(a170b8e568439e4a26d84376d53560e4248e4e2f) , ROM_GROUPWORD | ROM_SKIP(6) ) // in "6" socket
	ROMX_LOAD( "cc-4m.10a", 0x200006, 0x80000, CRC(1f9ebb97) SHA1(023d00cb7b6a52d1b29e2052abe08ef34cb0c55c) , ROM_GROUPWORD | ROM_SKIP(6) ) // in "8" socket

	ROM_REGION( 0x18000, "audiocpu", 0 ) /* 64k for the audio CPU (+banks) */
	ROM_LOAD( "cc_09.11a",  0x00000, 0x08000, CRC(698e8b58) SHA1(b7a3d905a7ed2c430426ca2e185e3d7e75e752a1) )
	ROM_CONTINUE(           0x10000, 0x08000 )

	ROM_REGION( 0x40000, "oki", 0 ) /* Samples */
	ROM_LOAD( "cc_18.11c",  0x00000, 0x20000, CRC(6de2c2db) SHA1(9a1eaba8d104f59a5e61f89679bb5de0c0c64364) )
	ROM_LOAD( "cc_19.12c",  0x20000, 0x20000, CRC(b99091ae) SHA1(b19197c7ad3aeaf5f41c26bf853b0c9b502ecfca) )

	ROM_REGION( 0x0200, "aboardplds", 0 )
	ROM_LOAD( "buf1",         0x0000, 0x0117, CRC(eb122de7) SHA1(b26b5bfe258e3e184f069719f9fd008d6b8f6b9b) )
	ROM_LOAD( "ioa1",         0x0000, 0x0117, CRC(59c7ee3b) SHA1(fbb887c5b4f5cb8df77cec710eaac2985bc482a6) )
	ROM_LOAD( "prg1",         0x0000, 0x0117, CRC(f1129744) SHA1(a5300f301c1a08a7da768f0773fa0fe3f683b237) )
	ROM_LOAD( "rom1",         0x0000, 0x0117, CRC(41dc73b9) SHA1(7d4c9f1693c821fbf84e32dd6ef62ddf14967845) )
	ROM_LOAD( "sou1",         0x0000, 0x0117, CRC(84f4b2fe) SHA1(dcc9e86cc36316fe42eace02d6df75d08bc8bb6d) )

	ROM_REGION( 0x0200, "bboardplds", 0 )
	ROM_LOAD( "cc63b.1a",     0x0000, 0x0117, CRC(cae8f0f9) SHA1(eadbd45e184195b2d170cd71a68e5caed64b69f7) )
	ROM_LOAD( "iob1.12d",     0x0000, 0x0117, CRC(3abc0700) SHA1(973043aa46ec6d5d1db20dc9d5937005a0f9f6ae) )
	ROM_LOAD( "ccprg.11d",    0x0000, 0x0117, CRC(e1c225c4) SHA1(97146451ca9aa3cecd443cc6881151ed8df47fbf) )    // == ccprg1.11d

	ROM_REGION( 0x0200, "cboardplds", 0 )
	ROM_LOAD( "ioc1.ic7",     0x0000, 0x0117, CRC(0d182081) SHA1(475b3d417785da4bc512cce2b274bb00d4cc6792) )
	ROM_LOAD( "c632.ic1",     0x0000, 0x0117, CRC(0fbd9270) SHA1(d7e737b20c44d41e29ca94be56114b31934dde81) )    // == c632b.ic1
ROM_END

/* B-Board 91635B-2 */
ROM_START( captcommu )
	ROM_REGION( CODE_SIZE, "maincpu", 0 )      /* 68000 code */
	ROM_LOAD16_WORD_SWAP( "ccu_23b.8f", 0x000000, 0x80000, CRC(03da44fd) SHA1(0bf382933b4b44082bbaf63e96acd83ab8808a34) )
	ROM_LOAD16_WORD_SWAP( "ccu_22c.7f", 0x080000, 0x80000, CRC(9b82a052) SHA1(8247fe45fea8c47072a66d6707202bcdb8c62923) )
	ROM_LOAD16_BYTE( "ccu_24b.9e",      0x100000, 0x20000, CRC(84ff99b2) SHA1(5b02c91f3d0f8fb46db9596136b683f5a22dc15f) )
	ROM_LOAD16_BYTE( "ccu_28b.9f",      0x100001, 0x20000, CRC(fbcec223) SHA1(daf484baece5b3a11f3dcabb758b8bdd736a1fb6) )

	ROM_REGION( 0x400000, "gfx", 0 )
	ROMX_LOAD( "cc-5m.3a",  0x000000, 0x80000, CRC(7261d8ba) SHA1(4b66292e42d20d0b79a756f0e445492ddb9c6bbc) , ROM_GROUPWORD | ROM_SKIP(6) ) // in "1" socket
	ROMX_LOAD( "cc-7m.5a",  0x000002, 0x80000, CRC(6a60f949) SHA1(87391ff92abaf3e451f70d789a938cffbd1fd222) , ROM_GROUPWORD | ROM_SKIP(6) ) // in "3" socket
	ROMX_LOAD( "cc-1m.4a",  0x000004, 0x80000, CRC(00637302) SHA1(2c554b59cceec2de67a9a4bc6281fe846d3c8cd2) , ROM_GROUPWORD | ROM_SKIP(6) ) // in "2" socket
	ROMX_LOAD( "cc-3m.6a",  0x000006, 0x80000, CRC(cc87cf61) SHA1(7fb1f49494cc1a08aded20754bb0cefb1c323198) , ROM_GROUPWORD | ROM_SKIP(6) ) // in "4" socket
	ROMX_LOAD( "cc-6m.7a",  0x200000, 0x80000, CRC(28718bed) SHA1(dfdc4dd14dc609783bad94d608a9e9b137dea944) , ROM_GROUPWORD | ROM_SKIP(6) ) // in "5" socket
	ROMX_LOAD( "cc-8m.9a",  0x200002, 0x80000, CRC(d4acc53a) SHA1(d03282ebbde362e679cc97f772aa9baf163d7606) , ROM_GROUPWORD | ROM_SKIP(6) ) // in "7" socket
	ROMX_LOAD( "cc-2m.8a",  0x200004, 0x80000, CRC(0c69f151) SHA1(a170b8e568439e4a26d84376d53560e4248e4e2f) , ROM_GROUPWORD | ROM_SKIP(6) ) // in "6" socket
	ROMX_LOAD( "cc-4m.10a", 0x200006, 0x80000, CRC(1f9ebb97) SHA1(023d00cb7b6a52d1b29e2052abe08ef34cb0c55c) , ROM_GROUPWORD | ROM_SKIP(6) ) // in "8" socket

	ROM_REGION( 0x18000, "audiocpu", 0 ) /* 64k for the audio CPU (+banks) */
	ROM_LOAD( "ccu_09.11a", 0x00000, 0x08000, CRC(698e8b58) SHA1(b7a3d905a7ed2c430426ca2e185e3d7e75e752a1) )    // == cc_09.11a
	ROM_CONTINUE(           0x10000, 0x08000 )

	ROM_REGION( 0x40000, "oki", 0 ) /* Samples */
	ROM_LOAD( "ccu_18.11c", 0x00000, 0x20000, CRC(6de2c2db) SHA1(9a1eaba8d104f59a5e61f89679bb5de0c0c64364) )    // == cc_18.11c
	ROM_LOAD( "ccu_19.12c", 0x20000, 0x20000, CRC(b99091ae) SHA1(b19197c7ad3aeaf5f41c26bf853b0c9b502ecfca) )    // == cc_19.12c

	ROM_REGION( 0x0200, "aboardplds", 0 )
	ROM_LOAD( "buf1",         0x0000, 0x0117, CRC(eb122de7) SHA1(b26b5bfe258e3e184f069719f9fd008d6b8f6b9b) )
	ROM_LOAD( "ioa1",         0x0000, 0x0117, CRC(59c7ee3b) SHA1(fbb887c5b4f5cb8df77cec710eaac2985bc482a6) )
	ROM_LOAD( "prg1",         0x0000, 0x0117, CRC(f1129744) SHA1(a5300f301c1a08a7da768f0773fa0fe3f683b237) )
	ROM_LOAD( "rom1",         0x0000, 0x0117, CRC(41dc73b9) SHA1(7d4c9f1693c821fbf84e32dd6ef62ddf14967845) )
	ROM_LOAD( "sou1",         0x0000, 0x0117, CRC(84f4b2fe) SHA1(dcc9e86cc36316fe42eace02d6df75d08bc8bb6d) )

	ROM_REGION( 0x0200, "bboardplds", 0 )
	ROM_LOAD( "cc63b.1a",     0x0000, 0x0117, CRC(cae8f0f9) SHA1(eadbd45e184195b2d170cd71a68e5caed64b69f7) )
	ROM_LOAD( "iob1.12d",     0x0000, 0x0117, CRC(3abc0700) SHA1(973043aa46ec6d5d1db20dc9d5937005a0f9f6ae) )
	ROM_LOAD( "ccprg1.11d",   0x0000, 0x0117, CRC(e1c225c4) SHA1(97146451ca9aa3cecd443cc6881151ed8df47fbf) )    /* seen the same pcb with CCPRG.11D */

	ROM_REGION( 0x0200, "cboardplds", 0 )
	ROM_LOAD( "ioc1.ic7",     0x0000, 0x0117, CRC(0d182081) SHA1(475b3d417785da4bc512cce2b274bb00d4cc6792) )
	ROM_LOAD( "c632.ic1",     0x0000, 0x0117, CRC(0fbd9270) SHA1(d7e737b20c44d41e29ca94be56114b31934dde81) )    // == c632b.ic1
ROM_END

/* B-Board 91634B-2 */
ROM_START( captcommj )
	ROM_REGION( CODE_SIZE, "maincpu", 0 )      /* 68000 code */
	ROM_LOAD16_WORD_SWAP( "ccj_23f.8f", 0x000000, 0x80000, CRC(5b482b62) SHA1(c871aa1eb9ecc117c3079995d1f5212193bd2e12) )
	ROM_LOAD16_WORD_SWAP( "ccj_22f.7f", 0x080000, 0x80000, CRC(0fd34195) SHA1(fb2b9a53af43507f13c4f94eaebbf0b538b2e754) )   // == cc_22f.7f
	ROM_LOAD16_BYTE( "ccj_24f.9e",      0x100000, 0x20000, CRC(3a794f25) SHA1(7f3722a4ef0c1d7acb73e6bac9dd6ae7b35e6374) )   // == cc_24f.9e
	ROM_LOAD16_BYTE( "ccj_28f.9f",      0x100001, 0x20000, CRC(fc3c2906) SHA1(621c3b79b6fdea1665bb316eb539e5916e890656) )   // == cc_28f.9f

	ROM_REGION( 0x400000, "gfx", 0 )
	ROMX_LOAD( "cc_01.3a",  0x000000, 0x80000, CRC(7261d8ba) SHA1(4b66292e42d20d0b79a756f0e445492ddb9c6bbc) , ROM_GROUPWORD | ROM_SKIP(6) ) // == cc-5m.3a
	ROMX_LOAD( "cc_02.4a",  0x000002, 0x80000, CRC(6a60f949) SHA1(87391ff92abaf3e451f70d789a938cffbd1fd222) , ROM_GROUPWORD | ROM_SKIP(6) ) // == cc-7m.5a
	ROMX_LOAD( "cc_03.5a",  0x000004, 0x80000, CRC(00637302) SHA1(2c554b59cceec2de67a9a4bc6281fe846d3c8cd2) , ROM_GROUPWORD | ROM_SKIP(6) ) // == cc-1m.4a
	ROMX_LOAD( "cc_04.6a",  0x000006, 0x80000, CRC(cc87cf61) SHA1(7fb1f49494cc1a08aded20754bb0cefb1c323198) , ROM_GROUPWORD | ROM_SKIP(6) ) // == cc-3m.6a
	ROMX_LOAD( "cc_05.7a",  0x200000, 0x80000, CRC(28718bed) SHA1(dfdc4dd14dc609783bad94d608a9e9b137dea944) , ROM_GROUPWORD | ROM_SKIP(6) ) // == cc-6m.7a
	ROMX_LOAD( "cc_06.8a",  0x200002, 0x80000, CRC(d4acc53a) SHA1(d03282ebbde362e679cc97f772aa9baf163d7606) , ROM_GROUPWORD | ROM_SKIP(6) ) // == cc-8m.9a
	ROMX_LOAD( "cc_07.9a",  0x200004, 0x80000, CRC(0c69f151) SHA1(a170b8e568439e4a26d84376d53560e4248e4e2f) , ROM_GROUPWORD | ROM_SKIP(6) ) // == cc-2m.8a
	ROMX_LOAD( "cc_08.10a", 0x200006, 0x80000, CRC(1f9ebb97) SHA1(023d00cb7b6a52d1b29e2052abe08ef34cb0c55c) , ROM_GROUPWORD | ROM_SKIP(6) ) // == cc-4m.10a

	ROM_REGION( 0x18000, "audiocpu", 0 ) /* 64k for the audio CPU (+banks) */
	ROM_LOAD( "ccj_09.12a", 0x00000, 0x08000, CRC(698e8b58) SHA1(b7a3d905a7ed2c430426ca2e185e3d7e75e752a1) )    // == cc_09.11a
	ROM_CONTINUE(           0x10000, 0x08000 )

	ROM_REGION( 0x40000, "oki", 0 ) /* Samples */
	ROM_LOAD( "ccj_18.11c", 0x00000, 0x20000, CRC(6de2c2db) SHA1(9a1eaba8d104f59a5e61f89679bb5de0c0c64364) )    // == cc_18.11c
	ROM_LOAD( "ccj_19.12c", 0x20000, 0x20000, CRC(b99091ae) SHA1(b19197c7ad3aeaf5f41c26bf853b0c9b502ecfca) )    // == cc_19.12c

	ROM_REGION( 0x0200, "aboardplds", 0 )
	ROM_LOAD( "buf1",         0x0000, 0x0117, CRC(eb122de7) SHA1(b26b5bfe258e3e184f069719f9fd008d6b8f6b9b) )
	ROM_LOAD( "ioa1",         0x0000, 0x0117, CRC(59c7ee3b) SHA1(fbb887c5b4f5cb8df77cec710eaac2985bc482a6) )
	ROM_LOAD( "prg1",         0x0000, 0x0117, CRC(f1129744) SHA1(a5300f301c1a08a7da768f0773fa0fe3f683b237) )
	ROM_LOAD( "rom1",         0x0000, 0x0117, CRC(41dc73b9) SHA1(7d4c9f1693c821fbf84e32dd6ef62ddf14967845) )
	ROM_LOAD( "sou1",         0x0000, 0x0117, CRC(84f4b2fe) SHA1(dcc9e86cc36316fe42eace02d6df75d08bc8bb6d) )

	ROM_REGION( 0x0200, "bboardplds", 0 )
	ROM_LOAD( "cc63b.1a",     0x0000, 0x0117, CRC(cae8f0f9) SHA1(eadbd45e184195b2d170cd71a68e5caed64b69f7) )
	ROM_LOAD( "iob1.12d",     0x0000, 0x0117, CRC(3abc0700) SHA1(973043aa46ec6d5d1db20dc9d5937005a0f9f6ae) )
	ROM_LOAD( "ccprg1.11d",   0x0000, 0x0117, CRC(e1c225c4) SHA1(97146451ca9aa3cecd443cc6881151ed8df47fbf) )    /* seen the same pcb with CCPRG.11D */

	ROM_REGION( 0x0200, "cboardplds", 0 )
	ROM_LOAD( "ioc1.ic7",     0x0000, 0x0117, CRC(0d182081) SHA1(475b3d417785da4bc512cce2b274bb00d4cc6792) )
	ROM_LOAD( "c632b.ic1",    0x0000, 0x0117, CRC(0fbd9270) SHA1(d7e737b20c44d41e29ca94be56114b31934dde81) )    /* seen the same pcb with C632.IC1 */
ROM_END

/* B-Board 91634B-2 */
ROM_START( captcommjr1 )
	ROM_REGION( CODE_SIZE, "maincpu", 0 )      /* 68000 code */
	ROM_LOAD16_WORD_SWAP( "ccj_23b.8f", 0x000000, 0x80000, CRC(e2a2d80e) SHA1(ec3c455974b77cd2e4357546aea4cb25690a041f) )
	ROM_LOAD16_WORD_SWAP( "ccj_22c.7f", 0x080000, 0x80000, CRC(9b82a052) SHA1(8247fe45fea8c47072a66d6707202bcdb8c62923) )   // == ccu_22c.7f
	ROM_LOAD16_BYTE( "ccj_24b.9e",      0x100000, 0x20000, CRC(84ff99b2) SHA1(5b02c91f3d0f8fb46db9596136b683f5a22dc15f) )   // == ccu_24b.9e
	ROM_LOAD16_BYTE( "ccj_28b.9f",      0x100001, 0x20000, CRC(fbcec223) SHA1(daf484baece5b3a11f3dcabb758b8bdd736a1fb6) )   // == ccu_28b.9f

	ROM_REGION( 0x400000, "gfx", 0 )
	ROMX_LOAD( "cc_01.3a",  0x000000, 0x80000, CRC(7261d8ba) SHA1(4b66292e42d20d0b79a756f0e445492ddb9c6bbc) , ROM_GROUPWORD | ROM_SKIP(6) ) // == cc-5m.3a
	ROMX_LOAD( "cc_02.4a",  0x000002, 0x80000, CRC(6a60f949) SHA1(87391ff92abaf3e451f70d789a938cffbd1fd222) , ROM_GROUPWORD | ROM_SKIP(6) ) // == cc-7m.5a
	ROMX_LOAD( "cc_03.5a",  0x000004, 0x80000, CRC(00637302) SHA1(2c554b59cceec2de67a9a4bc6281fe846d3c8cd2) , ROM_GROUPWORD | ROM_SKIP(6) ) // == cc-1m.4a
	ROMX_LOAD( "cc_04.6a",  0x000006, 0x80000, CRC(cc87cf61) SHA1(7fb1f49494cc1a08aded20754bb0cefb1c323198) , ROM_GROUPWORD | ROM_SKIP(6) ) // == cc-3m.6a
	ROMX_LOAD( "cc_05.7a",  0x200000, 0x80000, CRC(28718bed) SHA1(dfdc4dd14dc609783bad94d608a9e9b137dea944) , ROM_GROUPWORD | ROM_SKIP(6) ) // == cc-6m.7a
	ROMX_LOAD( "cc_06.8a",  0x200002, 0x80000, CRC(d4acc53a) SHA1(d03282ebbde362e679cc97f772aa9baf163d7606) , ROM_GROUPWORD | ROM_SKIP(6) ) // == cc-8m.9a
	ROMX_LOAD( "cc_07.9a",  0x200004, 0x80000, CRC(0c69f151) SHA1(a170b8e568439e4a26d84376d53560e4248e4e2f) , ROM_GROUPWORD | ROM_SKIP(6) ) // == cc-2m.8a
	ROMX_LOAD( "cc_08.10a", 0x200006, 0x80000, CRC(1f9ebb97) SHA1(023d00cb7b6a52d1b29e2052abe08ef34cb0c55c) , ROM_GROUPWORD | ROM_SKIP(6) ) // == cc-4m.10a

	ROM_REGION( 0x18000, "audiocpu", 0 ) /* 64k for the audio CPU (+banks) */
	ROM_LOAD( "ccj_09.12a", 0x00000, 0x08000, CRC(698e8b58) SHA1(b7a3d905a7ed2c430426ca2e185e3d7e75e752a1) )    // == cc_09.11a
	ROM_CONTINUE(           0x10000, 0x08000 )

	ROM_REGION( 0x40000, "oki", 0 ) /* Samples */
	ROM_LOAD( "ccj_18.11c", 0x00000, 0x20000, CRC(6de2c2db) SHA1(9a1eaba8d104f59a5e61f89679bb5de0c0c64364) )    // == cc_18.11c
	ROM_LOAD( "ccj_19.12c", 0x20000, 0x20000, CRC(b99091ae) SHA1(b19197c7ad3aeaf5f41c26bf853b0c9b502ecfca) )    // == cc_19.12c

	ROM_REGION( 0x0200, "aboardplds", 0 )
	ROM_LOAD( "buf1",         0x0000, 0x0117, CRC(eb122de7) SHA1(b26b5bfe258e3e184f069719f9fd008d6b8f6b9b) )
	ROM_LOAD( "ioa1",         0x0000, 0x0117, CRC(59c7ee3b) SHA1(fbb887c5b4f5cb8df77cec710eaac2985bc482a6) )
	ROM_LOAD( "prg1",         0x0000, 0x0117, CRC(f1129744) SHA1(a5300f301c1a08a7da768f0773fa0fe3f683b237) )
	ROM_LOAD( "rom1",         0x0000, 0x0117, CRC(41dc73b9) SHA1(7d4c9f1693c821fbf84e32dd6ef62ddf14967845) )
	ROM_LOAD( "sou1",         0x0000, 0x0117, CRC(84f4b2fe) SHA1(dcc9e86cc36316fe42eace02d6df75d08bc8bb6d) )

	ROM_REGION( 0x0200, "bboardplds", 0 )
	ROM_LOAD( "cc63b.1a",     0x0000, 0x0117, CRC(cae8f0f9) SHA1(eadbd45e184195b2d170cd71a68e5caed64b69f7) )
	ROM_LOAD( "iob1.12d",     0x0000, 0x0117, CRC(3abc0700) SHA1(973043aa46ec6d5d1db20dc9d5937005a0f9f6ae) )
	ROM_LOAD( "ccprg.11d",    0x0000, 0x0117, CRC(e1c225c4) SHA1(97146451ca9aa3cecd443cc6881151ed8df47fbf) )    // == ccprg1.11d

	ROM_REGION( 0x0200, "cboardplds", 0 )
	ROM_LOAD( "ioc1.ic7",     0x0000, 0x0117, CRC(0d182081) SHA1(475b3d417785da4bc512cce2b274bb00d4cc6792) )
	ROM_LOAD( "c632.ic1",     0x0000, 0x0117, CRC(0fbd9270) SHA1(d7e737b20c44d41e29ca94be56114b31934dde81) )    // == c632b.ic1
ROM_END

/* Captain Commando bootleg
 - there are 2 dumps of this, one has bad (half size) gfx roms tho, otherwise identical

ROMs from a Captain Commando bootleg PCB

Large single PCB containing.....
68000 @ 10MHz
Z80 @ 3.579545MHz
YM2151 @ 3.579545MHz
M6295 @ 1MHz (16/16), pin 7 HIGH
xtals 10MHz, 3.579545MHz, 16MHz
8-position DSWs x3
6116 (2kx8) SRAM x6
62256 (32kx8) SRAM x6
681000 (128kx8) SRAM x2
a few PALs
LOTS of logic
no special chips
no custom chips
no PLD/CPLD/FPGA
no PROMs

*/

ROM_START( captcommb )
	ROM_REGION( CODE_SIZE, "maincpu", 0 )      /* 68000 code */
	ROM_LOAD16_BYTE( "25.bin",        0x000000, 0x80000, CRC(cb71ed7a) SHA1(84f76b4861a3c7a59e67f38777f2d68749f19337) )
	ROM_LOAD16_BYTE( "27.bin",        0x000001, 0x80000, CRC(47cb2e87) SHA1(8a990a3a122045b50dd73d2e7b02fe60ab9af0a3) )
	ROM_LOAD16_BYTE( "24.bin",        0x100000, 0x40000, CRC(79794279) SHA1(5a43a4cef6653454ba9a81f2dd7f3f91c8a3354c) )
	ROM_LOAD16_BYTE( "26.bin",        0x100001, 0x40000, CRC(b01077ba) SHA1(0698fbfca7beea8e6a676aa19fcbf5ddea3defb1) )

	ROM_REGION( 0x400000, "gfx", 0 )
	ROMX_LOAD( "c91e-01.bin", 0x000000, 0x40000, CRC(f863071c) SHA1(c5154c4001f8e447623f9d71bf3e68a16f039e8f), ROM_SKIP(7) )
	ROM_CONTINUE(             0x000004, 0x40000)
	ROM_CONTINUE(             0x200000, 0x40000)
	ROM_CONTINUE(             0x200004, 0x40000)
	ROMX_LOAD( "c91e-02.bin", 0x000001, 0x40000, CRC(4b03c308) SHA1(d28d3ebba2571bea56b057cb3e09315a17d78b42), ROM_SKIP(7) )
	ROM_CONTINUE(             0x000005, 0x40000)
	ROM_CONTINUE(             0x200001, 0x40000)
	ROM_CONTINUE(             0x200005, 0x40000)
	ROMX_LOAD( "c91e-03.bin", 0x000002, 0x40000, CRC(3383ea96) SHA1(2a583d87c6d80919c97640f6f2e756cecc3e38ec), ROM_SKIP(7) )
	ROM_CONTINUE(             0x000006, 0x40000)
	ROM_CONTINUE(             0x200002, 0x40000)
	ROM_CONTINUE(             0x200006, 0x40000)
	ROMX_LOAD( "c91e-04.bin", 0x000003, 0x40000, CRC(b8e1f4cf) SHA1(6686df700c7ce49fe4ac7007aa4d622645e0e348), ROM_SKIP(7) )
	ROM_CONTINUE(             0x000007, 0x40000)
	ROM_CONTINUE(             0x200003, 0x40000)
	ROM_CONTINUE(             0x200007, 0x40000)

	ROM_REGION( 0x18000, "audiocpu", 0 ) /* 64k for the audio CPU (+banks) */
	ROM_LOAD( "l.bin",     0x00000, 0x08000, CRC(698e8b58) SHA1(b7a3d905a7ed2c430426ca2e185e3d7e75e752a1) )
	ROM_CONTINUE(              0x10000, 0x08000 )

	ROM_REGION( 0x40000, "oki", 0 ) /* Samples */
	ROM_LOAD( "c91e-05.bin",     0x00000, 0x40000, CRC(096115fb) SHA1(b496550f61b3d4b54ba43522d31efd0b09057493))
ROM_END

/* B-Board 91635B-2 */
ROM_START( knights )
	ROM_REGION( CODE_SIZE, "maincpu", 0 )      /* 68000 code */
	ROM_LOAD16_WORD_SWAP( "kr_23e.8f", 0x00000, 0x80000, CRC(1b3997eb) SHA1(724b68eff319fcdf0dd3bc1eb6662996c1f6ecd9) )
	ROM_LOAD16_WORD_SWAP( "kr_22.7f",  0x80000, 0x80000, CRC(d0b671a9) SHA1(9865472c5fc3f617345e23b5de5a9ba177945b5a) )

	ROM_REGION( 0x400000, "gfx", 0 )
	ROMX_LOAD( "kr-5m.3a",  0x000000, 0x80000, CRC(9e36c1a4) SHA1(772daae74e119371dfb76fde9775bda78a8ba125) , ROM_GROUPWORD | ROM_SKIP(6) ) // in "1" socket
	ROMX_LOAD( "kr-7m.5a",  0x000002, 0x80000, CRC(c5832cae) SHA1(a188cf401cd3a2909b377d3059f14d22ec3b0643) , ROM_GROUPWORD | ROM_SKIP(6) ) // in "3" socket
	ROMX_LOAD( "kr-1m.4a",  0x000004, 0x80000, CRC(f095be2d) SHA1(0427d1574062f277a9d04440019d5638b05de561) , ROM_GROUPWORD | ROM_SKIP(6) ) // in "2" socket
	ROMX_LOAD( "kr-3m.6a",  0x000006, 0x80000, CRC(179dfd96) SHA1(b1844e69da7ab13474da569978d5b47deb8eb2be) , ROM_GROUPWORD | ROM_SKIP(6) ) // in "4" socket
	ROMX_LOAD( "kr-6m.7a",  0x200000, 0x80000, CRC(1f4298d2) SHA1(4b162a7f649b0bcd676f8ca0c5eee9a1250d6452) , ROM_GROUPWORD | ROM_SKIP(6) ) // in "5" socket
	ROMX_LOAD( "kr-8m.9a",  0x200002, 0x80000, CRC(37fa8751) SHA1(b88b39d1f08621f15a5620095aef998346fa9891) , ROM_GROUPWORD | ROM_SKIP(6) ) // in "7" socket
	ROMX_LOAD( "kr-2m.8a",  0x200004, 0x80000, CRC(0200bc3d) SHA1(c900b1be2b4e49b951e5c1e3fd1e19d21b82986e) , ROM_GROUPWORD | ROM_SKIP(6) ) // in "6" socket
	ROMX_LOAD( "kr-4m.10a", 0x200006, 0x80000, CRC(0bb2b4e7) SHA1(983b800925d58e4aeb4e5105f93ed5faf66d009c) , ROM_GROUPWORD | ROM_SKIP(6) ) // in "8" socket

	ROM_REGION( 0x18000, "audiocpu", 0 ) /* 64k for the audio CPU (+banks) */
	ROM_LOAD( "kr_09.11a",  0x00000, 0x08000, CRC(5e44d9ee) SHA1(47a7503321be8d52b5c44af838e3bb82ee15a415) )
	ROM_CONTINUE(           0x10000, 0x08000 )

	ROM_REGION( 0x40000, "oki", 0 ) /* Samples */
	ROM_LOAD( "kr_18.11c",  0x00000, 0x20000, CRC(da69d15f) SHA1(9616207e693bae85705f786cef60b9f6951b5067) )
	ROM_LOAD( "kr_19.12c",  0x20000, 0x20000, CRC(bfc654e9) SHA1(01b3d92e4dedf55ea3933d387c7ddb9ba2549773) )

	ROM_REGION( 0x0200, "aboardplds", 0 )
	ROM_LOAD( "buf1",         0x0000, 0x0117, CRC(eb122de7) SHA1(b26b5bfe258e3e184f069719f9fd008d6b8f6b9b) )
	ROM_LOAD( "ioa1",         0x0000, 0x0117, CRC(59c7ee3b) SHA1(fbb887c5b4f5cb8df77cec710eaac2985bc482a6) )
	ROM_LOAD( "prg1",         0x0000, 0x0117, CRC(f1129744) SHA1(a5300f301c1a08a7da768f0773fa0fe3f683b237) )
	ROM_LOAD( "rom1",         0x0000, 0x0117, CRC(41dc73b9) SHA1(7d4c9f1693c821fbf84e32dd6ef62ddf14967845) )
	ROM_LOAD( "sou1",         0x0000, 0x0117, CRC(84f4b2fe) SHA1(dcc9e86cc36316fe42eace02d6df75d08bc8bb6d) )

	ROM_REGION( 0x0200, "bboardplds", 0 )
	ROM_LOAD( "kr63b.1a",     0x0000, 0x0117, CRC(fd5b6522) SHA1(5e6ebb2d736415402920a30d331d4b6dab557e5e) )
	ROM_LOAD( "iob1.12d",     0x0000, 0x0117, CRC(3abc0700) SHA1(973043aa46ec6d5d1db20dc9d5937005a0f9f6ae) )
	ROM_LOAD( "bprg1.11d",    0x0000, 0x0117, CRC(31793da7) SHA1(400fa7ac517421c978c1ee7773c30b9ed0c5d3f3) )

	ROM_REGION( 0x0200, "cboardplds", 0 )
	ROM_LOAD( "ioc1.ic7",     0x0000, 0x0117, CRC(0d182081) SHA1(475b3d417785da4bc512cce2b274bb00d4cc6792) )
	ROM_LOAD( "c632.ic1",     0x0000, 0x0117, CRC(0fbd9270) SHA1(d7e737b20c44d41e29ca94be56114b31934dde81) )
ROM_END

/* B-Board 91635B-2 */
ROM_START( knightsu )
	ROM_REGION( CODE_SIZE, "maincpu", 0 )      /* 68000 code */
	ROM_LOAD16_WORD_SWAP( "kr_23u.8f", 0x00000, 0x80000, CRC(252bc2ba) SHA1(4f4901c253bd64bbe68ea01994ae663fe2ccd056) )
	ROM_LOAD16_WORD_SWAP( "kr_22.7f",  0x80000, 0x80000, CRC(d0b671a9) SHA1(9865472c5fc3f617345e23b5de5a9ba177945b5a) )

	ROM_REGION( 0x400000, "gfx", 0 )
	ROMX_LOAD( "kr-5m.3a",  0x000000, 0x80000, CRC(9e36c1a4) SHA1(772daae74e119371dfb76fde9775bda78a8ba125) , ROM_GROUPWORD | ROM_SKIP(6) ) // in "1" socket
	ROMX_LOAD( "kr-7m.5a",  0x000002, 0x80000, CRC(c5832cae) SHA1(a188cf401cd3a2909b377d3059f14d22ec3b0643) , ROM_GROUPWORD | ROM_SKIP(6) ) // in "3" socket
	ROMX_LOAD( "kr-1m.4a",  0x000004, 0x80000, CRC(f095be2d) SHA1(0427d1574062f277a9d04440019d5638b05de561) , ROM_GROUPWORD | ROM_SKIP(6) ) // in "2" socket
	ROMX_LOAD( "kr-3m.6a",  0x000006, 0x80000, CRC(179dfd96) SHA1(b1844e69da7ab13474da569978d5b47deb8eb2be) , ROM_GROUPWORD | ROM_SKIP(6) ) // in "4" socket
	ROMX_LOAD( "kr-6m.7a",  0x200000, 0x80000, CRC(1f4298d2) SHA1(4b162a7f649b0bcd676f8ca0c5eee9a1250d6452) , ROM_GROUPWORD | ROM_SKIP(6) ) // in "5" socket
	ROMX_LOAD( "kr-8m.9a",  0x200002, 0x80000, CRC(37fa8751) SHA1(b88b39d1f08621f15a5620095aef998346fa9891) , ROM_GROUPWORD | ROM_SKIP(6) ) // in "7" socket
	ROMX_LOAD( "kr-2m.8a",  0x200004, 0x80000, CRC(0200bc3d) SHA1(c900b1be2b4e49b951e5c1e3fd1e19d21b82986e) , ROM_GROUPWORD | ROM_SKIP(6) ) // in "6" socket
	ROMX_LOAD( "kr-4m.10a", 0x200006, 0x80000, CRC(0bb2b4e7) SHA1(983b800925d58e4aeb4e5105f93ed5faf66d009c) , ROM_GROUPWORD | ROM_SKIP(6) ) // in "8" socket

	ROM_REGION( 0x18000, "audiocpu", 0 ) /* 64k for the audio CPU (+banks) */
	ROM_LOAD( "kr_09.11a",  0x00000, 0x08000, CRC(5e44d9ee) SHA1(47a7503321be8d52b5c44af838e3bb82ee15a415) )
	ROM_CONTINUE(           0x10000, 0x08000 )

	ROM_REGION( 0x40000, "oki", 0 ) /* Samples */
	ROM_LOAD( "kr_18.11c",  0x00000, 0x20000, CRC(da69d15f) SHA1(9616207e693bae85705f786cef60b9f6951b5067) )
	ROM_LOAD( "kr_19.12c",  0x20000, 0x20000, CRC(bfc654e9) SHA1(01b3d92e4dedf55ea3933d387c7ddb9ba2549773) )

	ROM_REGION( 0x0200, "aboardplds", 0 )
	ROM_LOAD( "buf1",         0x0000, 0x0117, CRC(eb122de7) SHA1(b26b5bfe258e3e184f069719f9fd008d6b8f6b9b) )
	ROM_LOAD( "ioa1",         0x0000, 0x0117, CRC(59c7ee3b) SHA1(fbb887c5b4f5cb8df77cec710eaac2985bc482a6) )
	ROM_LOAD( "prg1",         0x0000, 0x0117, CRC(f1129744) SHA1(a5300f301c1a08a7da768f0773fa0fe3f683b237) )
	ROM_LOAD( "rom1",         0x0000, 0x0117, CRC(41dc73b9) SHA1(7d4c9f1693c821fbf84e32dd6ef62ddf14967845) )
	ROM_LOAD( "sou1",         0x0000, 0x0117, CRC(84f4b2fe) SHA1(dcc9e86cc36316fe42eace02d6df75d08bc8bb6d) )

	ROM_REGION( 0x0200, "bboardplds", 0 )
	ROM_LOAD( "kr63b.1a",     0x0000, 0x0117, CRC(fd5b6522) SHA1(5e6ebb2d736415402920a30d331d4b6dab557e5e) )
	ROM_LOAD( "iob1.12d",     0x0000, 0x0117, CRC(3abc0700) SHA1(973043aa46ec6d5d1db20dc9d5937005a0f9f6ae) )
	ROM_LOAD( "bprg1.11d",    0x0000, 0x0117, CRC(31793da7) SHA1(400fa7ac517421c978c1ee7773c30b9ed0c5d3f3) )

	ROM_REGION( 0x0200, "cboardplds", 0 )
	ROM_LOAD( "ioc1.ic7",     0x0000, 0x0117, CRC(0d182081) SHA1(475b3d417785da4bc512cce2b274bb00d4cc6792) )
	ROM_LOAD( "c632.ic1",     0x0000, 0x0117, CRC(0fbd9270) SHA1(d7e737b20c44d41e29ca94be56114b31934dde81) )
ROM_END

/* B-Board 91634B-2 */
ROM_START( knightsj )
	ROM_REGION( CODE_SIZE, "maincpu", 0 )      /* 68000 code */
	ROM_LOAD16_WORD_SWAP( "kr_23j.8f", 0x00000, 0x80000, CRC(eae7417f) SHA1(2ec808265a9a231922e2397d7e8f3c3841a90859) )
	ROM_LOAD16_WORD_SWAP( "kr_22.7f",  0x80000, 0x80000, CRC(d0b671a9) SHA1(9865472c5fc3f617345e23b5de5a9ba177945b5a) )

	ROM_REGION( 0x400000, "gfx", 0 )
	ROMX_LOAD( "kr_01.3a",  0x000000, 0x80000, CRC(9e36c1a4) SHA1(772daae74e119371dfb76fde9775bda78a8ba125) , ROM_GROUPWORD | ROM_SKIP(6) ) // == kr-5m.3a
	ROMX_LOAD( "kr_02.4a",  0x000002, 0x80000, CRC(c5832cae) SHA1(a188cf401cd3a2909b377d3059f14d22ec3b0643) , ROM_GROUPWORD | ROM_SKIP(6) ) // == kr-7m.5a
	ROMX_LOAD( "kr_03.5a",  0x000004, 0x80000, CRC(f095be2d) SHA1(0427d1574062f277a9d04440019d5638b05de561) , ROM_GROUPWORD | ROM_SKIP(6) ) // == kr-1m.4a
	ROMX_LOAD( "kr_04.6a",  0x000006, 0x80000, CRC(179dfd96) SHA1(b1844e69da7ab13474da569978d5b47deb8eb2be) , ROM_GROUPWORD | ROM_SKIP(6) ) // == kr-3m.6a
	ROMX_LOAD( "kr_05.7a",  0x200000, 0x80000, CRC(1f4298d2) SHA1(4b162a7f649b0bcd676f8ca0c5eee9a1250d6452) , ROM_GROUPWORD | ROM_SKIP(6) ) // == kr-6m.7a
	ROMX_LOAD( "kr_06.8a",  0x200002, 0x80000, CRC(37fa8751) SHA1(b88b39d1f08621f15a5620095aef998346fa9891) , ROM_GROUPWORD | ROM_SKIP(6) ) // == kr-8m.9a
	ROMX_LOAD( "kr_07.9a",  0x200004, 0x80000, CRC(0200bc3d) SHA1(c900b1be2b4e49b951e5c1e3fd1e19d21b82986e) , ROM_GROUPWORD | ROM_SKIP(6) ) // == kr-2m.8a
	ROMX_LOAD( "kr_08.10a", 0x200006, 0x80000, CRC(0bb2b4e7) SHA1(983b800925d58e4aeb4e5105f93ed5faf66d009c) , ROM_GROUPWORD | ROM_SKIP(6) ) // == kr-4m.10a

	ROM_REGION( 0x18000, "audiocpu", 0 ) /* 64k for the audio CPU (+banks) */
	ROM_LOAD( "kr_09.12a",  0x00000, 0x08000, CRC(5e44d9ee) SHA1(47a7503321be8d52b5c44af838e3bb82ee15a415) )    // == kr_09.11a
	ROM_CONTINUE(           0x10000, 0x08000 )

	ROM_REGION( 0x40000, "oki", 0 ) /* Samples */
	ROM_LOAD( "kr_18.11c",  0x00000, 0x20000, CRC(da69d15f) SHA1(9616207e693bae85705f786cef60b9f6951b5067) )
	ROM_LOAD( "kr_19.12c",  0x20000, 0x20000, CRC(bfc654e9) SHA1(01b3d92e4dedf55ea3933d387c7ddb9ba2549773) )

	ROM_REGION( 0x0200, "aboardplds", 0 )
	ROM_LOAD( "buf1",         0x0000, 0x0117, CRC(eb122de7) SHA1(b26b5bfe258e3e184f069719f9fd008d6b8f6b9b) )
	ROM_LOAD( "ioa1",         0x0000, 0x0117, CRC(59c7ee3b) SHA1(fbb887c5b4f5cb8df77cec710eaac2985bc482a6) )
	ROM_LOAD( "prg1",         0x0000, 0x0117, CRC(f1129744) SHA1(a5300f301c1a08a7da768f0773fa0fe3f683b237) )
	ROM_LOAD( "rom1",         0x0000, 0x0117, CRC(41dc73b9) SHA1(7d4c9f1693c821fbf84e32dd6ef62ddf14967845) )
	ROM_LOAD( "sou1",         0x0000, 0x0117, CRC(84f4b2fe) SHA1(dcc9e86cc36316fe42eace02d6df75d08bc8bb6d) )

	ROM_REGION( 0x0200, "bboardplds", 0 )
	ROM_LOAD( "kr63b.1a",     0x0000, 0x0117, CRC(fd5b6522) SHA1(5e6ebb2d736415402920a30d331d4b6dab557e5e) )
	ROM_LOAD( "iob1.12d",     0x0000, 0x0117, CRC(3abc0700) SHA1(973043aa46ec6d5d1db20dc9d5937005a0f9f6ae) )
	ROM_LOAD( "bprg1.11d",    0x0000, 0x0117, CRC(31793da7) SHA1(400fa7ac517421c978c1ee7773c30b9ed0c5d3f3) )

	ROM_REGION( 0x0200, "cboardplds", 0 )
	ROM_LOAD( "ioc1.ic7",     0x0000, 0x0117, CRC(0d182081) SHA1(475b3d417785da4bc512cce2b274bb00d4cc6792) )
	ROM_LOAD( "c632.ic1",     0x0000, 0x0117, CRC(0fbd9270) SHA1(d7e737b20c44d41e29ca94be56114b31934dde81) )
ROM_END

/* B-Board 89625B-1 */
/* Note that this set is equivalent to knightsj, but each 4Mbit EPROM is replaced by 4 1Mbit EPROMs. */
ROM_START( knightsja )
	ROM_REGION( CODE_SIZE, "maincpu", 0 )      /* 68000 code */
	ROM_LOAD16_BYTE( "krj_36.12f", 0x00000, 0x20000, CRC(ad3d1a8e) SHA1(327f9e818f1500836fc549afeffbb2a3c5aafe8c) ) // == kr_23j.8f
	ROM_LOAD16_BYTE( "krj_42.12h", 0x00001, 0x20000, CRC(e694a491) SHA1(5a4d27c879c10032c49880019501de3e45ab1b35) ) // == kr_23j.8f
	ROM_LOAD16_BYTE( "krj_37.13f", 0x40000, 0x20000, CRC(85596094) SHA1(74ad294de63aa6b60aa8b885c45c3d41a07ce19a) ) // == kr_23j.8f
	ROM_LOAD16_BYTE( "krj_43.13h", 0x40001, 0x20000, CRC(9198bf8f) SHA1(aa3610600286ab25ce81705ea1319d42e7cc7f6c) ) // == kr_23j.8f
	ROM_LOAD16_BYTE( "kr_34.10f",  0x80000, 0x20000, CRC(fe6eb08d) SHA1(a67ca33895ff7f41e8521ff7b39e629048d04d30) ) // == kr_22.7f
	ROM_LOAD16_BYTE( "kr_40.10h",  0x80001, 0x20000, CRC(1172806d) SHA1(5ad08bf9bada036d89a55f5a542ee1db22df45b2) ) // == kr_22.7f
	ROM_LOAD16_BYTE( "kr_35.11f",  0xc0000, 0x20000, CRC(f854b020) SHA1(3d8bd0ab5e0fa2da3c868b591edff5c66f1f08ed) ) // == kr_22.7f
	ROM_LOAD16_BYTE( "kr_41.11h",  0xc0001, 0x20000, CRC(eb52e78d) SHA1(b9462c9d97848a6dc4d55475053824df8c9e7f02) ) // == kr_22.7f

	ROM_REGION( 0x400000, "gfx", 0 )
	ROMX_LOAD( "kr_09.4b",  0x000000, 0x20000, CRC(08b76e10) SHA1(b18833c4794ef59f8eaf7aef4d62790c3fec9d4d) , ROM_SKIP(7) ) // == kr_01.3a
	ROMX_LOAD( "kr_01.4a",  0x000001, 0x20000, CRC(40cecf5c) SHA1(a628199e1808664c403d0de183d1c6ae01670c57) , ROM_SKIP(7) ) // == kr_01.3a
	ROMX_LOAD( "kr_13.9b",  0x000002, 0x20000, CRC(435aaa03) SHA1(60af266dccab12220d7efce68887b64826aba37d) , ROM_SKIP(7) ) // == kr_02.4a
	ROMX_LOAD( "kr_05.9a",  0x000003, 0x20000, CRC(5b8a615b) SHA1(42aebdb7f338eac9a29380cb0b7553f0d3eaac04) , ROM_SKIP(7) ) // == kr_02.4a
	ROMX_LOAD( "kr_24.5e",  0x000004, 0x20000, CRC(de65153e) SHA1(f28fcde0d08634d036d07c2a699026007d19718f) , ROM_SKIP(7) ) // == kr_03.5a
	ROMX_LOAD( "kr_17.5c",  0x000005, 0x20000, CRC(b171c968) SHA1(cdbd7b0dc9fd2f3f027796d8f5cabf0a92a11941) , ROM_SKIP(7) ) // == kr_03.5a
	ROMX_LOAD( "kr_38.8h",  0x000006, 0x20000, CRC(f4466bf4) SHA1(86e1ffc354b189504ca8f96143a62344a530d267) , ROM_SKIP(7) ) // == kr_04.6a
	ROMX_LOAD( "kr_32.8f",  0x000007, 0x20000, CRC(87380ddd) SHA1(01e864060adcdafad987c5758db593525faed589) , ROM_SKIP(7) ) // == kr_04.6a
	ROMX_LOAD( "kr_10.5b",  0x100000, 0x20000, CRC(37006d66) SHA1(0efec364ed5b0e121023bc3424d51241a4b208e0) , ROM_SKIP(7) ) // == kr_01.3a
	ROMX_LOAD( "kr_02.5a",  0x100001, 0x20000, CRC(b54612e3) SHA1(1d33cf99fdaf3bf5c8d212d1a7c71f085c6c2cd4) , ROM_SKIP(7) ) // == kr_01.3a
	ROMX_LOAD( "kr_14.10b", 0x100002, 0x20000, CRC(0ae88766) SHA1(5e3954b51b030d6e7cabd8f94cb849e963cc7d52) , ROM_SKIP(7) ) // == kr_02.4a
	ROMX_LOAD( "kr_06.10a", 0x100003, 0x20000, CRC(ecb1a09a) SHA1(bbd96260774f52e50377ad020fc94d1b120f0a37) , ROM_SKIP(7) ) // == kr_02.4a
	ROMX_LOAD( "kr_25.7e",  0x100004, 0x20000, CRC(9aace189) SHA1(95332a14dbe9d24a7e294a26530913b8cb5237f2) , ROM_SKIP(7) ) // == kr_03.5a
	ROMX_LOAD( "kr_18.7c",  0x100005, 0x20000, CRC(09fa14a5) SHA1(36e7412a317d5f8b528d914d7e6c3c1872eb8533) , ROM_SKIP(7) ) // == kr_03.5a
	ROMX_LOAD( "kr_39.9h",  0x100006, 0x20000, CRC(fd8a9aeb) SHA1(a38ea74de95a78fc72c6ad165157e4c22aaa5aed) , ROM_SKIP(7) ) // == kr_04.6a
	ROMX_LOAD( "kr_33.9f",  0x100007, 0x20000, CRC(11803e95) SHA1(71603ba4a85967dd434d8499738014a9227d1d26) , ROM_SKIP(7) ) // == kr_04.6a
	ROMX_LOAD( "kr_11.7b",  0x200000, 0x20000, CRC(a967ceb3) SHA1(43cdc0e0114f7ccedc7db5cd49b9041ed1a2614f) , ROM_SKIP(7) ) // == kr_05.7a
	ROMX_LOAD( "kr_03.7a",  0x200001, 0x20000, CRC(ea10db07) SHA1(e537f3983ac32abcef06fa45afe51f82c87bfa86) , ROM_SKIP(7) ) // == kr_05.7a
	ROMX_LOAD( "kr_15.11b", 0x200002, 0x20000, CRC(8140b83b) SHA1(e9cec517d84aa688d60f4201bcee00dbcdd44798) , ROM_SKIP(7) ) // == kr_06.8a
	ROMX_LOAD( "kr_07.11a", 0x200003, 0x20000, CRC(6af10648) SHA1(b1d6c908496b1197e23571f5230f017502d5018c) , ROM_SKIP(7) ) // == kr_06.8a
	ROMX_LOAD( "kr_26.8e",  0x200004, 0x20000, CRC(8865d86b) SHA1(a79b528d2f94ee350a39eb59860189809267cfad) , ROM_SKIP(7) ) // == kr_07.9a
	ROMX_LOAD( "kr_19.8c",  0x200005, 0x20000, CRC(029f4abe) SHA1(e0f02d18e2a51eb02cabe507fbe77031ee3e67e7) , ROM_SKIP(7) ) // == kr_07.9a
	ROMX_LOAD( "kr_28.10e", 0x200006, 0x20000, CRC(5f84f92f) SHA1(77b254e8159024490497533073fe302a82531835) , ROM_SKIP(7) ) // == kr_08.10a
	ROMX_LOAD( "kr_21.10c", 0x200007, 0x20000, CRC(01b35065) SHA1(8a98860ec453ac88a3c0fc19c117bd4131d0c97c) , ROM_SKIP(7) ) // == kr_08.10a
	ROMX_LOAD( "kr_12.8b",  0x300000, 0x20000, CRC(03d945b1) SHA1(88588891422247e6ab42f35a1b91a5801b8fd5b0) , ROM_SKIP(7) ) // == kr_05.7a
	ROMX_LOAD( "kr_04.8a",  0x300001, 0x20000, CRC(e30c8388) SHA1(aa519f298f4156b339eb81767ca585e62dc9f4b5) , ROM_SKIP(7) ) // == kr_05.7a
	ROMX_LOAD( "kr_16.12b", 0x300002, 0x20000, CRC(40c39d1b) SHA1(578d081f5e1bbfdc30b286cdd5165474b3ed79f7) , ROM_SKIP(7) ) // == kr_06.8a
	ROMX_LOAD( "kr_08.12a", 0x300003, 0x20000, CRC(d310c9e8) SHA1(e23a4b318e7d3d6ea3bc6dd4509f49cd656d3d63) , ROM_SKIP(7) ) // == kr_06.8a
	ROMX_LOAD( "kr_27.9e",  0x300004, 0x20000, CRC(3e041444) SHA1(7f84158bf2466625ff50befed7e3008b8fe09f70) , ROM_SKIP(7) ) // == kr_07.9a
	ROMX_LOAD( "kr_20.9c",  0x300005, 0x20000, CRC(bd4bffb8) SHA1(b8f1162427a46ca9d2833390edc5723d28817a08) , ROM_SKIP(7) ) // == kr_07.9a
	ROMX_LOAD( "kr_29.11e", 0x300006, 0x20000, CRC(1387a076) SHA1(5fe32ee8e8557c5bc6c865b2054f3c6c2a1eae7d) , ROM_SKIP(7) ) // == kr_08.10a
	ROMX_LOAD( "kr_22.11c", 0x300007, 0x20000, CRC(fd351922) SHA1(7e08a9b23bf524d5fd566d81745aee1cd3a0f026) , ROM_SKIP(7) ) // == kr_08.10a

	ROM_REGION( 0x18000, "audiocpu", 0 ) /* 64k for the audio CPU (+banks) */
	ROM_LOAD( "kr_23.13b",  0x00000, 0x08000, CRC(5e44d9ee) SHA1(47a7503321be8d52b5c44af838e3bb82ee15a415) )    // == kr_09.12a
	ROM_CONTINUE(           0x10000, 0x08000 )

	ROM_REGION( 0x40000, "oki", 0 ) /* Samples */
	ROM_LOAD( "kr_30.12c",  0x00000, 0x20000, CRC(da69d15f) SHA1(9616207e693bae85705f786cef60b9f6951b5067) )    // == kr_18.11c
	ROM_LOAD( "kr_31.13c",  0x20000, 0x20000, CRC(bfc654e9) SHA1(01b3d92e4dedf55ea3933d387c7ddb9ba2549773) )    // == kr_19.12c

	ROM_REGION( 0x0200, "aboardplds", 0 )
	ROM_LOAD( "buf1",         0x0000, 0x0117, CRC(eb122de7) SHA1(b26b5bfe258e3e184f069719f9fd008d6b8f6b9b) )
	ROM_LOAD( "ioa1",         0x0000, 0x0117, CRC(59c7ee3b) SHA1(fbb887c5b4f5cb8df77cec710eaac2985bc482a6) )
	ROM_LOAD( "prg1",         0x0000, 0x0117, CRC(f1129744) SHA1(a5300f301c1a08a7da768f0773fa0fe3f683b237) )
	ROM_LOAD( "rom1",         0x0000, 0x0117, CRC(41dc73b9) SHA1(7d4c9f1693c821fbf84e32dd6ef62ddf14967845) )
	ROM_LOAD( "sou1",         0x0000, 0x0117, CRC(84f4b2fe) SHA1(dcc9e86cc36316fe42eace02d6df75d08bc8bb6d) )

	ROM_REGION( 0x0200, "bboardplds", 0 )
	ROM_LOAD( "kr22b.1a",     0x0000, 0x0117, NO_DUMP )
	ROM_LOAD( "lwio.12e",     0x0000, 0x0117, CRC(ad52b90c) SHA1(f0fd6aeea515ee449320fe15684e6b3ab7f97bf4) )

	ROM_REGION( 0x0200, "cboardplds", 0 )
	ROM_LOAD( "ioc1.ic7",     0x0000, 0x0117, CRC(0d182081) SHA1(475b3d417785da4bc512cce2b274bb00d4cc6792) )
	ROM_LOAD( "c632.ic1",     0x0000, 0x0117, CRC(0fbd9270) SHA1(d7e737b20c44d41e29ca94be56114b31934dde81) )
ROM_END

/* B-Board 91635B-2 */
ROM_START( sf2ce )
	ROM_REGION( CODE_SIZE, "maincpu", 0 )      /* 68000 code */
	ROM_LOAD16_WORD_SWAP( "s92e_23b.8f", 0x000000, 0x80000, CRC(0aaa1a3a) SHA1(774a2b52f7c1876c0e10d8d57a0850ad2d016cf6) )
	ROM_LOAD16_WORD_SWAP( "s92_22b.7f",  0x080000, 0x80000, CRC(2bbe15ed) SHA1(a8e2edef62fa99c5ef701b28bfb6bc42f3af183d) )
	ROM_LOAD16_WORD_SWAP( "s92_21a.6f",  0x100000, 0x80000, CRC(925a7877) SHA1(1960dca35f0ca6f2b399a9fccfbc0132ac6425d1) )

	ROM_REGION( 0x600000, "gfx", 0 )
	ROMX_LOAD( "s92-1m.3a",  0x000000, 0x80000, CRC(03b0d852) SHA1(f370f25c96ad2b94f8c53d6b7139100285a25bef) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "s92-3m.5a",  0x000002, 0x80000, CRC(840289ec) SHA1(2fb42a242f60ba7e74009b5a90eb26e035ba1e82) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "s92-2m.4a",  0x000004, 0x80000, CRC(cdb5f027) SHA1(4c7d944fef200fdfcaf57758b901b5511188ed2e) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "s92-4m.6a",  0x000006, 0x80000, CRC(e2799472) SHA1(27d3796429338d82a8de246a0ea06dd487a87768) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "s92-5m.7a",  0x200000, 0x80000, CRC(ba8a2761) SHA1(4b696d66c51611e43522bed752654314e76d33b6) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "s92-7m.9a",  0x200002, 0x80000, CRC(e584bfb5) SHA1(ebdf1f5e2638eed3a65dda82b1ed9151a355f4c9) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "s92-6m.8a",  0x200004, 0x80000, CRC(21e3f87d) SHA1(4a4961bb68c3a1ce15f9d393d9c03ecb2466cc29) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "s92-8m.10a", 0x200006, 0x80000, CRC(befc47df) SHA1(520390420da3a0271ba90b0a933e65143265e5cf) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "s92-10m.3c", 0x400000, 0x80000, CRC(960687d5) SHA1(2868c31121b1c7564e9767b9a19cdbf655c7ed1d) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "s92-12m.5c", 0x400002, 0x80000, CRC(978ecd18) SHA1(648a59706b93c84b4206a968ecbdc3e834c476f6) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "s92-11m.4c", 0x400004, 0x80000, CRC(d6ec9a0a) SHA1(ed6143f8737013b6ef1684e37c05e037e7a80dae) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "s92-13m.6c", 0x400006, 0x80000, CRC(ed2c67f6) SHA1(0083c0ffaf6fe7659ff0cf822be4346cd6e61329) , ROM_GROUPWORD | ROM_SKIP(6) )

	ROM_REGION( 0x18000, "audiocpu", 0 ) /* 64k for the audio CPU (+banks) */
	ROM_LOAD( "s92_09.11a",  0x00000, 0x08000, CRC(08f6b60e) SHA1(8258fcaca4ac419312531eec67079b97f471179c) )
	ROM_CONTINUE(            0x10000, 0x08000 )

	ROM_REGION( 0x40000, "oki", 0 ) /* Samples */
	ROM_LOAD( "s92_18.11c",  0x00000, 0x20000, CRC(7f162009) SHA1(346bf42992b4c36c593e21901e22c87ae4a7d86d) )
	ROM_LOAD( "s92_19.12c",  0x20000, 0x20000, CRC(beade53f) SHA1(277c397dc12752719ec6b47d2224750bd1c07f79) )

	ROM_REGION( 0x0200, "aboardplds", 0 )
	ROM_LOAD( "buf1",         0x0000, 0x0117, CRC(eb122de7) SHA1(b26b5bfe258e3e184f069719f9fd008d6b8f6b9b) )
	ROM_LOAD( "ioa1",         0x0000, 0x0117, CRC(59c7ee3b) SHA1(fbb887c5b4f5cb8df77cec710eaac2985bc482a6) )
	ROM_LOAD( "prg1",         0x0000, 0x0117, CRC(f1129744) SHA1(a5300f301c1a08a7da768f0773fa0fe3f683b237) )
	ROM_LOAD( "rom1",         0x0000, 0x0117, CRC(41dc73b9) SHA1(7d4c9f1693c821fbf84e32dd6ef62ddf14967845) )
	ROM_LOAD( "sou1",         0x0000, 0x0117, CRC(84f4b2fe) SHA1(dcc9e86cc36316fe42eace02d6df75d08bc8bb6d) )

	ROM_REGION( 0x0200, "bboardplds", 0 )
	ROM_LOAD( "s9263b.1a",    0x0000, 0x0117, CRC(0a7ecfe0) SHA1(f75e7eed4604fcf68273197fe3dd7f0d7a313ada) )
	ROM_LOAD( "iob1.12d",     0x0000, 0x0117, CRC(3abc0700) SHA1(973043aa46ec6d5d1db20dc9d5937005a0f9f6ae) )
	ROM_LOAD( "bprg1.11d",    0x0000, 0x0117, CRC(31793da7) SHA1(400fa7ac517421c978c1ee7773c30b9ed0c5d3f3) )

	ROM_REGION( 0x0200, "cboardplds", 0 )
	ROM_LOAD( "ioc1.ic7",     0x0000, 0x0117, CRC(0d182081) SHA1(475b3d417785da4bc512cce2b274bb00d4cc6792) )
	ROM_LOAD( "c632.ic1",     0x0000, 0x0117, CRC(0fbd9270) SHA1(d7e737b20c44d41e29ca94be56114b31934dde81) )
ROM_END

/* B-Board 91635B-2 */
ROM_START( sf2ceea )
	ROM_REGION( CODE_SIZE, "maincpu", 0 )      /* 68000 code */
	ROM_LOAD16_WORD_SWAP( "s92e_23a.8f", 0x000000, 0x80000, CRC(3f846b74) SHA1(c8d7a01b626771870123f1663a01a81f9c8fe582) )
	ROM_LOAD16_WORD_SWAP( "s92_22a.7f",  0x080000, 0x80000, CRC(99f1cca4) SHA1(64111eba81d743fc3fd51d7a89cd0b2eefcc900d) )
	ROM_LOAD16_WORD_SWAP( "s92_21a.6f",  0x100000, 0x80000, CRC(925a7877) SHA1(1960dca35f0ca6f2b399a9fccfbc0132ac6425d1) )

	ROM_REGION( 0x600000, "gfx", 0 )
	ROMX_LOAD( "s92-1m.3a",  0x000000, 0x80000, CRC(03b0d852) SHA1(f370f25c96ad2b94f8c53d6b7139100285a25bef) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "s92-3m.5a",  0x000002, 0x80000, CRC(840289ec) SHA1(2fb42a242f60ba7e74009b5a90eb26e035ba1e82) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "s92-2m.4a",  0x000004, 0x80000, CRC(cdb5f027) SHA1(4c7d944fef200fdfcaf57758b901b5511188ed2e) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "s92-4m.6a",  0x000006, 0x80000, CRC(e2799472) SHA1(27d3796429338d82a8de246a0ea06dd487a87768) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "s92-5m.7a",  0x200000, 0x80000, CRC(ba8a2761) SHA1(4b696d66c51611e43522bed752654314e76d33b6) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "s92-7m.9a",  0x200002, 0x80000, CRC(e584bfb5) SHA1(ebdf1f5e2638eed3a65dda82b1ed9151a355f4c9) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "s92-6m.8a",  0x200004, 0x80000, CRC(21e3f87d) SHA1(4a4961bb68c3a1ce15f9d393d9c03ecb2466cc29) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "s92-8m.10a", 0x200006, 0x80000, CRC(befc47df) SHA1(520390420da3a0271ba90b0a933e65143265e5cf) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "s92-10m.3c", 0x400000, 0x80000, CRC(960687d5) SHA1(2868c31121b1c7564e9767b9a19cdbf655c7ed1d) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "s92-12m.5c", 0x400002, 0x80000, CRC(978ecd18) SHA1(648a59706b93c84b4206a968ecbdc3e834c476f6) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "s92-11m.4c", 0x400004, 0x80000, CRC(d6ec9a0a) SHA1(ed6143f8737013b6ef1684e37c05e037e7a80dae) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "s92-13m.6c", 0x400006, 0x80000, CRC(ed2c67f6) SHA1(0083c0ffaf6fe7659ff0cf822be4346cd6e61329) , ROM_GROUPWORD | ROM_SKIP(6) )

	ROM_REGION( 0x18000, "audiocpu", 0 ) /* 64k for the audio CPU (+banks) */
	ROM_LOAD( "s92_09.11a",  0x00000, 0x08000, CRC(08f6b60e) SHA1(8258fcaca4ac419312531eec67079b97f471179c) )
	ROM_CONTINUE(            0x10000, 0x08000 )

	ROM_REGION( 0x40000, "oki", 0 ) /* Samples */
	ROM_LOAD( "s92_18.11c",  0x00000, 0x20000, CRC(7f162009) SHA1(346bf42992b4c36c593e21901e22c87ae4a7d86d) )
	ROM_LOAD( "s92_19.12c",  0x20000, 0x20000, CRC(beade53f) SHA1(277c397dc12752719ec6b47d2224750bd1c07f79) )

	ROM_REGION( 0x0200, "aboardplds", 0 )
	ROM_LOAD( "buf1",         0x0000, 0x0117, CRC(eb122de7) SHA1(b26b5bfe258e3e184f069719f9fd008d6b8f6b9b) )
	ROM_LOAD( "ioa1",         0x0000, 0x0117, CRC(59c7ee3b) SHA1(fbb887c5b4f5cb8df77cec710eaac2985bc482a6) )
	ROM_LOAD( "prg1",         0x0000, 0x0117, CRC(f1129744) SHA1(a5300f301c1a08a7da768f0773fa0fe3f683b237) )
	ROM_LOAD( "rom1",         0x0000, 0x0117, CRC(41dc73b9) SHA1(7d4c9f1693c821fbf84e32dd6ef62ddf14967845) )
	ROM_LOAD( "sou1",         0x0000, 0x0117, CRC(84f4b2fe) SHA1(dcc9e86cc36316fe42eace02d6df75d08bc8bb6d) )

	ROM_REGION( 0x0200, "bboardplds", 0 )
	ROM_LOAD( "s9263b.1a",    0x0000, 0x0117, CRC(0a7ecfe0) SHA1(f75e7eed4604fcf68273197fe3dd7f0d7a313ada) )
	ROM_LOAD( "iob1.12d",     0x0000, 0x0117, CRC(3abc0700) SHA1(973043aa46ec6d5d1db20dc9d5937005a0f9f6ae) )
	ROM_LOAD( "bprg1.11d",    0x0000, 0x0117, CRC(31793da7) SHA1(400fa7ac517421c978c1ee7773c30b9ed0c5d3f3) )

	ROM_REGION( 0x0200, "cboardplds", 0 )
	ROM_LOAD( "ioc1.ic7",     0x0000, 0x0117, CRC(0d182081) SHA1(475b3d417785da4bc512cce2b274bb00d4cc6792) )
	ROM_LOAD( "c632.ic1",     0x0000, 0x0117, CRC(0fbd9270) SHA1(d7e737b20c44d41e29ca94be56114b31934dde81) )
ROM_END

/* B-Board 91635B-2 */
ROM_START( sf2ceua )
	ROM_REGION( CODE_SIZE, "maincpu", 0 )      /* 68000 code */
	ROM_LOAD16_WORD_SWAP( "s92u_23a.8f", 0x000000, 0x80000, CRC(ac44415b) SHA1(218f8b1886eb72b8547127042b5ae47600e18944) )
	ROM_LOAD16_WORD_SWAP( "s92_22a.7f",  0x080000, 0x80000, CRC(99f1cca4) SHA1(64111eba81d743fc3fd51d7a89cd0b2eefcc900d) )
	ROM_LOAD16_WORD_SWAP( "s92_21a.6f",  0x100000, 0x80000, CRC(925a7877) SHA1(1960dca35f0ca6f2b399a9fccfbc0132ac6425d1) )

	ROM_REGION( 0x600000, "gfx", 0 )
	ROMX_LOAD( "s92-1m.3a",  0x000000, 0x80000, CRC(03b0d852) SHA1(f370f25c96ad2b94f8c53d6b7139100285a25bef) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "s92-3m.5a",  0x000002, 0x80000, CRC(840289ec) SHA1(2fb42a242f60ba7e74009b5a90eb26e035ba1e82) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "s92-2m.4a",  0x000004, 0x80000, CRC(cdb5f027) SHA1(4c7d944fef200fdfcaf57758b901b5511188ed2e) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "s92-4m.6a",  0x000006, 0x80000, CRC(e2799472) SHA1(27d3796429338d82a8de246a0ea06dd487a87768) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "s92-5m.7a",  0x200000, 0x80000, CRC(ba8a2761) SHA1(4b696d66c51611e43522bed752654314e76d33b6) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "s92-7m.9a",  0x200002, 0x80000, CRC(e584bfb5) SHA1(ebdf1f5e2638eed3a65dda82b1ed9151a355f4c9) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "s92-6m.8a",  0x200004, 0x80000, CRC(21e3f87d) SHA1(4a4961bb68c3a1ce15f9d393d9c03ecb2466cc29) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "s92-8m.10a", 0x200006, 0x80000, CRC(befc47df) SHA1(520390420da3a0271ba90b0a933e65143265e5cf) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "s92-10m.3c", 0x400000, 0x80000, CRC(960687d5) SHA1(2868c31121b1c7564e9767b9a19cdbf655c7ed1d) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "s92-12m.5c", 0x400002, 0x80000, CRC(978ecd18) SHA1(648a59706b93c84b4206a968ecbdc3e834c476f6) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "s92-11m.4c", 0x400004, 0x80000, CRC(d6ec9a0a) SHA1(ed6143f8737013b6ef1684e37c05e037e7a80dae) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "s92-13m.6c", 0x400006, 0x80000, CRC(ed2c67f6) SHA1(0083c0ffaf6fe7659ff0cf822be4346cd6e61329) , ROM_GROUPWORD | ROM_SKIP(6) )

	ROM_REGION( 0x18000, "audiocpu", 0 ) /* 64k for the audio CPU (+banks) */
	ROM_LOAD( "s92_09.11a",  0x00000, 0x08000, CRC(08f6b60e) SHA1(8258fcaca4ac419312531eec67079b97f471179c) )
	ROM_CONTINUE(            0x10000, 0x08000 )

	ROM_REGION( 0x40000, "oki", 0 ) /* Samples */
	ROM_LOAD( "s92_18.11c",  0x00000, 0x20000, CRC(7f162009) SHA1(346bf42992b4c36c593e21901e22c87ae4a7d86d) )
	ROM_LOAD( "s92_19.12c",  0x20000, 0x20000, CRC(beade53f) SHA1(277c397dc12752719ec6b47d2224750bd1c07f79) )

	ROM_REGION( 0x0200, "aboardplds", 0 )
	ROM_LOAD( "buf1",         0x0000, 0x0117, CRC(eb122de7) SHA1(b26b5bfe258e3e184f069719f9fd008d6b8f6b9b) )
	ROM_LOAD( "ioa1",         0x0000, 0x0117, CRC(59c7ee3b) SHA1(fbb887c5b4f5cb8df77cec710eaac2985bc482a6) )
	ROM_LOAD( "prg1",         0x0000, 0x0117, CRC(f1129744) SHA1(a5300f301c1a08a7da768f0773fa0fe3f683b237) )
	ROM_LOAD( "rom1",         0x0000, 0x0117, CRC(41dc73b9) SHA1(7d4c9f1693c821fbf84e32dd6ef62ddf14967845) )
	ROM_LOAD( "sou1",         0x0000, 0x0117, CRC(84f4b2fe) SHA1(dcc9e86cc36316fe42eace02d6df75d08bc8bb6d) )

	ROM_REGION( 0x0200, "bboardplds", 0 )
	ROM_LOAD( "s9263b.1a",    0x0000, 0x0117, CRC(0a7ecfe0) SHA1(f75e7eed4604fcf68273197fe3dd7f0d7a313ada) )
	ROM_LOAD( "iob1.12d",     0x0000, 0x0117, CRC(3abc0700) SHA1(973043aa46ec6d5d1db20dc9d5937005a0f9f6ae) )
	ROM_LOAD( "bprg1.11d",    0x0000, 0x0117, CRC(31793da7) SHA1(400fa7ac517421c978c1ee7773c30b9ed0c5d3f3) )

	ROM_REGION( 0x0200, "cboardplds", 0 )
	ROM_LOAD( "ioc1.ic7",     0x0000, 0x0117, CRC(0d182081) SHA1(475b3d417785da4bc512cce2b274bb00d4cc6792) )
	ROM_LOAD( "c632.ic1",     0x0000, 0x0117, CRC(0fbd9270) SHA1(d7e737b20c44d41e29ca94be56114b31934dde81) )
ROM_END

/* B-Board 91635B-2 */
ROM_START( sf2ceub )
	ROM_REGION( CODE_SIZE, "maincpu", 0 )      /* 68000 code */
	ROM_LOAD16_WORD_SWAP( "s92u_23b.8f", 0x000000, 0x80000, CRC(996a3015) SHA1(fdf45da54b1c14478a60f2b86e37ffe32a98b135) )
	ROM_LOAD16_WORD_SWAP( "s92_22b.7f",  0x080000, 0x80000, CRC(2bbe15ed) SHA1(a8e2edef62fa99c5ef701b28bfb6bc42f3af183d) )
	ROM_LOAD16_WORD_SWAP( "s92_21a.6f",  0x100000, 0x80000, CRC(925a7877) SHA1(1960dca35f0ca6f2b399a9fccfbc0132ac6425d1) )

	ROM_REGION( 0x600000, "gfx", 0 )
	ROMX_LOAD( "s92-1m.3a",  0x000000, 0x80000, CRC(03b0d852) SHA1(f370f25c96ad2b94f8c53d6b7139100285a25bef) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "s92-3m.5a",  0x000002, 0x80000, CRC(840289ec) SHA1(2fb42a242f60ba7e74009b5a90eb26e035ba1e82) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "s92-2m.4a",  0x000004, 0x80000, CRC(cdb5f027) SHA1(4c7d944fef200fdfcaf57758b901b5511188ed2e) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "s92-4m.6a",  0x000006, 0x80000, CRC(e2799472) SHA1(27d3796429338d82a8de246a0ea06dd487a87768) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "s92-5m.7a",  0x200000, 0x80000, CRC(ba8a2761) SHA1(4b696d66c51611e43522bed752654314e76d33b6) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "s92-7m.9a",  0x200002, 0x80000, CRC(e584bfb5) SHA1(ebdf1f5e2638eed3a65dda82b1ed9151a355f4c9) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "s92-6m.8a",  0x200004, 0x80000, CRC(21e3f87d) SHA1(4a4961bb68c3a1ce15f9d393d9c03ecb2466cc29) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "s92-8m.10a", 0x200006, 0x80000, CRC(befc47df) SHA1(520390420da3a0271ba90b0a933e65143265e5cf) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "s92-10m.3c", 0x400000, 0x80000, CRC(960687d5) SHA1(2868c31121b1c7564e9767b9a19cdbf655c7ed1d) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "s92-12m.5c", 0x400002, 0x80000, CRC(978ecd18) SHA1(648a59706b93c84b4206a968ecbdc3e834c476f6) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "s92-11m.4c", 0x400004, 0x80000, CRC(d6ec9a0a) SHA1(ed6143f8737013b6ef1684e37c05e037e7a80dae) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "s92-13m.6c", 0x400006, 0x80000, CRC(ed2c67f6) SHA1(0083c0ffaf6fe7659ff0cf822be4346cd6e61329) , ROM_GROUPWORD | ROM_SKIP(6) )

	ROM_REGION( 0x18000, "audiocpu", 0 ) /* 64k for the audio CPU (+banks) */
	ROM_LOAD( "s92_09.11a",  0x00000, 0x08000, CRC(08f6b60e) SHA1(8258fcaca4ac419312531eec67079b97f471179c) )
	ROM_CONTINUE(            0x10000, 0x08000 )

	ROM_REGION( 0x40000, "oki", 0 ) /* Samples */
	ROM_LOAD( "s92_18.11c",  0x00000, 0x20000, CRC(7f162009) SHA1(346bf42992b4c36c593e21901e22c87ae4a7d86d) )
	ROM_LOAD( "s92_19.12c",  0x20000, 0x20000, CRC(beade53f) SHA1(277c397dc12752719ec6b47d2224750bd1c07f79) )

	ROM_REGION( 0x0200, "aboardplds", 0 )
	ROM_LOAD( "buf1",         0x0000, 0x0117, CRC(eb122de7) SHA1(b26b5bfe258e3e184f069719f9fd008d6b8f6b9b) )
	ROM_LOAD( "ioa1",         0x0000, 0x0117, CRC(59c7ee3b) SHA1(fbb887c5b4f5cb8df77cec710eaac2985bc482a6) )
	ROM_LOAD( "prg1",         0x0000, 0x0117, CRC(f1129744) SHA1(a5300f301c1a08a7da768f0773fa0fe3f683b237) )
	ROM_LOAD( "rom1",         0x0000, 0x0117, CRC(41dc73b9) SHA1(7d4c9f1693c821fbf84e32dd6ef62ddf14967845) )
	ROM_LOAD( "sou1",         0x0000, 0x0117, CRC(84f4b2fe) SHA1(dcc9e86cc36316fe42eace02d6df75d08bc8bb6d) )

	ROM_REGION( 0x0200, "bboardplds", 0 )
	ROM_LOAD( "s9263b.1a",    0x0000, 0x0117, CRC(0a7ecfe0) SHA1(f75e7eed4604fcf68273197fe3dd7f0d7a313ada) )
	ROM_LOAD( "iob1.12d",     0x0000, 0x0117, CRC(3abc0700) SHA1(973043aa46ec6d5d1db20dc9d5937005a0f9f6ae) )
	ROM_LOAD( "bprg1.11d",    0x0000, 0x0117, CRC(31793da7) SHA1(400fa7ac517421c978c1ee7773c30b9ed0c5d3f3) )

	ROM_REGION( 0x0200, "cboardplds", 0 )
	ROM_LOAD( "ioc1.ic7",     0x0000, 0x0117, CRC(0d182081) SHA1(475b3d417785da4bc512cce2b274bb00d4cc6792) )
	ROM_LOAD( "c632.ic1",     0x0000, 0x0117, CRC(0fbd9270) SHA1(d7e737b20c44d41e29ca94be56114b31934dde81) )
ROM_END

/* B-Board 91635B-2 */
ROM_START( sf2ceuc )
	ROM_REGION( CODE_SIZE, "maincpu", 0 )      /* 68000 code */
	ROM_LOAD16_WORD_SWAP( "s92u_23c.8f", 0x000000, 0x80000, CRC(0a8b6aa2) SHA1(a19871271172119e1cf1ff47700bb1917b08514b) )
	ROM_LOAD16_WORD_SWAP( "s92_22c.7f",  0x080000, 0x80000, CRC(5fd8630b) SHA1(f0ef9c5ab91a4b421fb4b1747eef99c964c15de3) )
	ROM_LOAD16_WORD_SWAP( "s92_21a.6f",  0x100000, 0x80000, CRC(925a7877) SHA1(1960dca35f0ca6f2b399a9fccfbc0132ac6425d1) )

	ROM_REGION( 0x600000, "gfx", 0 )
	ROMX_LOAD( "s92-1m.3a",  0x000000, 0x80000, CRC(03b0d852) SHA1(f370f25c96ad2b94f8c53d6b7139100285a25bef) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "s92-3m.5a",  0x000002, 0x80000, CRC(840289ec) SHA1(2fb42a242f60ba7e74009b5a90eb26e035ba1e82) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "s92-2m.4a",  0x000004, 0x80000, CRC(cdb5f027) SHA1(4c7d944fef200fdfcaf57758b901b5511188ed2e) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "s92-4m.6a",  0x000006, 0x80000, CRC(e2799472) SHA1(27d3796429338d82a8de246a0ea06dd487a87768) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "s92-5m.7a",  0x200000, 0x80000, CRC(ba8a2761) SHA1(4b696d66c51611e43522bed752654314e76d33b6) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "s92-7m.9a",  0x200002, 0x80000, CRC(e584bfb5) SHA1(ebdf1f5e2638eed3a65dda82b1ed9151a355f4c9) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "s92-6m.8a",  0x200004, 0x80000, CRC(21e3f87d) SHA1(4a4961bb68c3a1ce15f9d393d9c03ecb2466cc29) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "s92-8m.10a", 0x200006, 0x80000, CRC(befc47df) SHA1(520390420da3a0271ba90b0a933e65143265e5cf) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "s92-10m.3c", 0x400000, 0x80000, CRC(960687d5) SHA1(2868c31121b1c7564e9767b9a19cdbf655c7ed1d) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "s92-12m.5c", 0x400002, 0x80000, CRC(978ecd18) SHA1(648a59706b93c84b4206a968ecbdc3e834c476f6) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "s92-11m.4c", 0x400004, 0x80000, CRC(d6ec9a0a) SHA1(ed6143f8737013b6ef1684e37c05e037e7a80dae) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "s92-13m.6c", 0x400006, 0x80000, CRC(ed2c67f6) SHA1(0083c0ffaf6fe7659ff0cf822be4346cd6e61329) , ROM_GROUPWORD | ROM_SKIP(6) )

	ROM_REGION( 0x18000, "audiocpu", 0 ) /* 64k for the audio CPU (+banks) */
	ROM_LOAD( "s92_09.11a",  0x00000, 0x08000, CRC(08f6b60e) SHA1(8258fcaca4ac419312531eec67079b97f471179c) )
	ROM_CONTINUE(            0x10000, 0x08000 )

	ROM_REGION( 0x40000, "oki", 0 ) /* Samples */
	ROM_LOAD( "s92_18.11c",  0x00000, 0x20000, CRC(7f162009) SHA1(346bf42992b4c36c593e21901e22c87ae4a7d86d) )
	ROM_LOAD( "s92_19.12c",  0x20000, 0x20000, CRC(beade53f) SHA1(277c397dc12752719ec6b47d2224750bd1c07f79) )

	ROM_REGION( 0x0200, "aboardplds", 0 )
	ROM_LOAD( "buf1",         0x0000, 0x0117, CRC(eb122de7) SHA1(b26b5bfe258e3e184f069719f9fd008d6b8f6b9b) )
	ROM_LOAD( "ioa1",         0x0000, 0x0117, CRC(59c7ee3b) SHA1(fbb887c5b4f5cb8df77cec710eaac2985bc482a6) )
	ROM_LOAD( "prg1",         0x0000, 0x0117, CRC(f1129744) SHA1(a5300f301c1a08a7da768f0773fa0fe3f683b237) )
	ROM_LOAD( "rom1",         0x0000, 0x0117, CRC(41dc73b9) SHA1(7d4c9f1693c821fbf84e32dd6ef62ddf14967845) )
	ROM_LOAD( "sou1",         0x0000, 0x0117, CRC(84f4b2fe) SHA1(dcc9e86cc36316fe42eace02d6df75d08bc8bb6d) )

	ROM_REGION( 0x0200, "bboardplds", 0 )
	ROM_LOAD( "s9263b.1a",    0x0000, 0x0117, CRC(0a7ecfe0) SHA1(f75e7eed4604fcf68273197fe3dd7f0d7a313ada) )
	ROM_LOAD( "iob1.12d",     0x0000, 0x0117, CRC(3abc0700) SHA1(973043aa46ec6d5d1db20dc9d5937005a0f9f6ae) )
	ROM_LOAD( "bprg1.11d",    0x0000, 0x0117, CRC(31793da7) SHA1(400fa7ac517421c978c1ee7773c30b9ed0c5d3f3) )

	ROM_REGION( 0x0200, "cboardplds", 0 )
	ROM_LOAD( "ioc1.ic7",     0x0000, 0x0117, CRC(0d182081) SHA1(475b3d417785da4bc512cce2b274bb00d4cc6792) )
	ROM_LOAD( "c632.ic1",     0x0000, 0x0117, CRC(0fbd9270) SHA1(d7e737b20c44d41e29ca94be56114b31934dde81) )
ROM_END

/* B-Board 91634B-2 */
ROM_START( sf2ceja )
	ROM_REGION( CODE_SIZE, "maincpu", 0 )      /* 68000 code */
	ROM_LOAD16_WORD_SWAP( "s92j_23a.8f", 0x000000, 0x80000, CRC(4f42bb5a) SHA1(59d0587c554e06ea45d4092ea4299ff086509d4b) )
	ROM_LOAD16_WORD_SWAP( "s92j_22a.7f", 0x080000, 0x80000, CRC(c4f64bcd) SHA1(262c0419bf727da80c2ac52b877a19276d9aac3c) )
	ROM_LOAD16_WORD_SWAP( "s92_21a.6f",  0x100000, 0x80000, CRC(925a7877) SHA1(1960dca35f0ca6f2b399a9fccfbc0132ac6425d1) )

	ROM_REGION( 0x600000, "gfx", 0 )
	ROMX_LOAD( "s92_01.3a",  0x000000, 0x80000, CRC(03b0d852) SHA1(f370f25c96ad2b94f8c53d6b7139100285a25bef) , ROM_GROUPWORD | ROM_SKIP(6) )    // == s92-1m.3a
	ROMX_LOAD( "s92_02.4a",  0x000002, 0x80000, CRC(840289ec) SHA1(2fb42a242f60ba7e74009b5a90eb26e035ba1e82) , ROM_GROUPWORD | ROM_SKIP(6) )    // == s92-3m.5a
	ROMX_LOAD( "s92_03.5a",  0x000004, 0x80000, CRC(cdb5f027) SHA1(4c7d944fef200fdfcaf57758b901b5511188ed2e) , ROM_GROUPWORD | ROM_SKIP(6) )    // == s92-2m.4a
	ROMX_LOAD( "s92_04.6a",  0x000006, 0x80000, CRC(e2799472) SHA1(27d3796429338d82a8de246a0ea06dd487a87768) , ROM_GROUPWORD | ROM_SKIP(6) )    // == s92-4m.6a
	ROMX_LOAD( "s92_05.7a",  0x200000, 0x80000, CRC(ba8a2761) SHA1(4b696d66c51611e43522bed752654314e76d33b6) , ROM_GROUPWORD | ROM_SKIP(6) )    // == s92-5m.7a
	ROMX_LOAD( "s92_06.8a",  0x200002, 0x80000, CRC(e584bfb5) SHA1(ebdf1f5e2638eed3a65dda82b1ed9151a355f4c9) , ROM_GROUPWORD | ROM_SKIP(6) )    // == s92-7m.9a
	ROMX_LOAD( "s92_07.9a",  0x200004, 0x80000, CRC(21e3f87d) SHA1(4a4961bb68c3a1ce15f9d393d9c03ecb2466cc29) , ROM_GROUPWORD | ROM_SKIP(6) )    // == s92-6m.8a
	ROMX_LOAD( "s92_08.10a", 0x200006, 0x80000, CRC(befc47df) SHA1(520390420da3a0271ba90b0a933e65143265e5cf) , ROM_GROUPWORD | ROM_SKIP(6) )    // == s92-8m.10a
	ROMX_LOAD( "s92_10.3c",  0x400000, 0x80000, CRC(960687d5) SHA1(2868c31121b1c7564e9767b9a19cdbf655c7ed1d) , ROM_GROUPWORD | ROM_SKIP(6) )    // == s92-10m.3c
	ROMX_LOAD( "s92_11.4c",  0x400002, 0x80000, CRC(978ecd18) SHA1(648a59706b93c84b4206a968ecbdc3e834c476f6) , ROM_GROUPWORD | ROM_SKIP(6) )    // == s92-12m.5c
	ROMX_LOAD( "s92_12.5c",  0x400004, 0x80000, CRC(d6ec9a0a) SHA1(ed6143f8737013b6ef1684e37c05e037e7a80dae) , ROM_GROUPWORD | ROM_SKIP(6) )    // == s92-11m.4c
	ROMX_LOAD( "s92_13.6c",  0x400006, 0x80000, CRC(ed2c67f6) SHA1(0083c0ffaf6fe7659ff0cf822be4346cd6e61329) , ROM_GROUPWORD | ROM_SKIP(6) )    // == s92-13m.6c

	ROM_REGION( 0x18000, "audiocpu", 0 ) /* 64k for the audio CPU (+banks) */
	ROM_LOAD( "s92_09.12a",  0x00000, 0x08000, CRC(08f6b60e) SHA1(8258fcaca4ac419312531eec67079b97f471179c) )   // == s92_09.11a
	ROM_CONTINUE(            0x10000, 0x08000 )

	ROM_REGION( 0x40000, "oki", 0 ) /* Samples */
	ROM_LOAD( "s92_18.11c",  0x00000, 0x20000, CRC(7f162009) SHA1(346bf42992b4c36c593e21901e22c87ae4a7d86d) )
	ROM_LOAD( "s92_19.12c",  0x20000, 0x20000, CRC(beade53f) SHA1(277c397dc12752719ec6b47d2224750bd1c07f79) )

	ROM_REGION( 0x0200, "aboardplds", 0 )
	ROM_LOAD( "buf1",         0x0000, 0x0117, CRC(eb122de7) SHA1(b26b5bfe258e3e184f069719f9fd008d6b8f6b9b) )
	ROM_LOAD( "ioa1",         0x0000, 0x0117, CRC(59c7ee3b) SHA1(fbb887c5b4f5cb8df77cec710eaac2985bc482a6) )
	ROM_LOAD( "prg1",         0x0000, 0x0117, CRC(f1129744) SHA1(a5300f301c1a08a7da768f0773fa0fe3f683b237) )
	ROM_LOAD( "rom1",         0x0000, 0x0117, CRC(41dc73b9) SHA1(7d4c9f1693c821fbf84e32dd6ef62ddf14967845) )
	ROM_LOAD( "sou1",         0x0000, 0x0117, CRC(84f4b2fe) SHA1(dcc9e86cc36316fe42eace02d6df75d08bc8bb6d) )

	ROM_REGION( 0x0200, "bboardplds", 0 )
	ROM_LOAD( "s9263b.1a",    0x0000, 0x0117, CRC(0a7ecfe0) SHA1(f75e7eed4604fcf68273197fe3dd7f0d7a313ada) )
	ROM_LOAD( "iob1.12d",     0x0000, 0x0117, CRC(3abc0700) SHA1(973043aa46ec6d5d1db20dc9d5937005a0f9f6ae) )
	ROM_LOAD( "bprg1.11d",    0x0000, 0x0117, CRC(31793da7) SHA1(400fa7ac517421c978c1ee7773c30b9ed0c5d3f3) )

	ROM_REGION( 0x0200, "cboardplds", 0 )
	ROM_LOAD( "ioc1.ic7",     0x0000, 0x0117, CRC(0d182081) SHA1(475b3d417785da4bc512cce2b274bb00d4cc6792) )
	ROM_LOAD( "c632.ic1",     0x0000, 0x0117, CRC(0fbd9270) SHA1(d7e737b20c44d41e29ca94be56114b31934dde81) )
ROM_END

/* B-Board 91634B-2 */
ROM_START( sf2cejb )
	ROM_REGION( CODE_SIZE, "maincpu", 0 )      /* 68000 code */
	ROM_LOAD16_WORD_SWAP( "s92j_23b.8f", 0x000000, 0x80000, CRC(140876c5) SHA1(304630e6d8bae9f8d29090e05f7e013c7dafe9cc) )
	ROM_LOAD16_WORD_SWAP( "s92j_22b.7f", 0x080000, 0x80000, CRC(2fbb3bfe) SHA1(e364564a12022730c2c0d0e8fd435e2c30ef9410) )
	ROM_LOAD16_WORD_SWAP( "s92_21a.6f",  0x100000, 0x80000, CRC(925a7877) SHA1(1960dca35f0ca6f2b399a9fccfbc0132ac6425d1) )  // could be s92j_21a.6f

	ROM_REGION( 0x600000, "gfx", 0 )
	ROMX_LOAD( "s92_01.3a",  0x000000, 0x80000, CRC(03b0d852) SHA1(f370f25c96ad2b94f8c53d6b7139100285a25bef) , ROM_GROUPWORD | ROM_SKIP(6) )    // == s92-1m.3a
	ROMX_LOAD( "s92_02.4a",  0x000002, 0x80000, CRC(840289ec) SHA1(2fb42a242f60ba7e74009b5a90eb26e035ba1e82) , ROM_GROUPWORD | ROM_SKIP(6) )    // == s92-3m.5a
	ROMX_LOAD( "s92_03.5a",  0x000004, 0x80000, CRC(cdb5f027) SHA1(4c7d944fef200fdfcaf57758b901b5511188ed2e) , ROM_GROUPWORD | ROM_SKIP(6) )    // == s92-2m.4a
	ROMX_LOAD( "s92_04.6a",  0x000006, 0x80000, CRC(e2799472) SHA1(27d3796429338d82a8de246a0ea06dd487a87768) , ROM_GROUPWORD | ROM_SKIP(6) )    // == s92-4m.6a
	ROMX_LOAD( "s92_05.7a",  0x200000, 0x80000, CRC(ba8a2761) SHA1(4b696d66c51611e43522bed752654314e76d33b6) , ROM_GROUPWORD | ROM_SKIP(6) )    // == s92-5m.7a
	ROMX_LOAD( "s92_06.8a",  0x200002, 0x80000, CRC(e584bfb5) SHA1(ebdf1f5e2638eed3a65dda82b1ed9151a355f4c9) , ROM_GROUPWORD | ROM_SKIP(6) )    // == s92-7m.9a
	ROMX_LOAD( "s92_07.9a",  0x200004, 0x80000, CRC(21e3f87d) SHA1(4a4961bb68c3a1ce15f9d393d9c03ecb2466cc29) , ROM_GROUPWORD | ROM_SKIP(6) )    // == s92-6m.8a
	ROMX_LOAD( "s92_08.10a", 0x200006, 0x80000, CRC(befc47df) SHA1(520390420da3a0271ba90b0a933e65143265e5cf) , ROM_GROUPWORD | ROM_SKIP(6) )    // == s92-8m.10a
	ROMX_LOAD( "s92_10.3c",  0x400000, 0x80000, CRC(960687d5) SHA1(2868c31121b1c7564e9767b9a19cdbf655c7ed1d) , ROM_GROUPWORD | ROM_SKIP(6) )    // == s92-10m.3c
	ROMX_LOAD( "s92_11.4c",  0x400002, 0x80000, CRC(978ecd18) SHA1(648a59706b93c84b4206a968ecbdc3e834c476f6) , ROM_GROUPWORD | ROM_SKIP(6) )    // == s92-12m.5c
	ROMX_LOAD( "s92_12.5c",  0x400004, 0x80000, CRC(d6ec9a0a) SHA1(ed6143f8737013b6ef1684e37c05e037e7a80dae) , ROM_GROUPWORD | ROM_SKIP(6) )    // == s92-11m.4c
	ROMX_LOAD( "s92_13.6c",  0x400006, 0x80000, CRC(ed2c67f6) SHA1(0083c0ffaf6fe7659ff0cf822be4346cd6e61329) , ROM_GROUPWORD | ROM_SKIP(6) )    // == s92-13m.6c

	ROM_REGION( 0x18000, "audiocpu", 0 ) /* 64k for the audio CPU (+banks) */
	ROM_LOAD( "s92_09.12a",  0x00000, 0x08000, CRC(08f6b60e) SHA1(8258fcaca4ac419312531eec67079b97f471179c) )   // == s92_09.11a
	ROM_CONTINUE(            0x10000, 0x08000 )

	ROM_REGION( 0x40000, "oki", 0 ) /* Samples */
	ROM_LOAD( "s92_18.11c",  0x00000, 0x20000, CRC(7f162009) SHA1(346bf42992b4c36c593e21901e22c87ae4a7d86d) )
	ROM_LOAD( "s92_19.12c",  0x20000, 0x20000, CRC(beade53f) SHA1(277c397dc12752719ec6b47d2224750bd1c07f79) )

	ROM_REGION( 0x0200, "aboardplds", 0 )
	ROM_LOAD( "buf1",         0x0000, 0x0117, CRC(eb122de7) SHA1(b26b5bfe258e3e184f069719f9fd008d6b8f6b9b) )
	ROM_LOAD( "ioa1",         0x0000, 0x0117, CRC(59c7ee3b) SHA1(fbb887c5b4f5cb8df77cec710eaac2985bc482a6) )
	ROM_LOAD( "prg1",         0x0000, 0x0117, CRC(f1129744) SHA1(a5300f301c1a08a7da768f0773fa0fe3f683b237) )
	ROM_LOAD( "rom1",         0x0000, 0x0117, CRC(41dc73b9) SHA1(7d4c9f1693c821fbf84e32dd6ef62ddf14967845) )
	ROM_LOAD( "sou1",         0x0000, 0x0117, CRC(84f4b2fe) SHA1(dcc9e86cc36316fe42eace02d6df75d08bc8bb6d) )

	ROM_REGION( 0x0200, "bboardplds", 0 )
	ROM_LOAD( "s9263b.1a",    0x0000, 0x0117, CRC(0a7ecfe0) SHA1(f75e7eed4604fcf68273197fe3dd7f0d7a313ada) )
	ROM_LOAD( "iob1.12d",     0x0000, 0x0117, CRC(3abc0700) SHA1(973043aa46ec6d5d1db20dc9d5937005a0f9f6ae) )
	ROM_LOAD( "bprg1.11d",    0x0000, 0x0117, CRC(31793da7) SHA1(400fa7ac517421c978c1ee7773c30b9ed0c5d3f3) )

	ROM_REGION( 0x0200, "cboardplds", 0 )
	ROM_LOAD( "ioc1.ic7",     0x0000, 0x0117, CRC(0d182081) SHA1(475b3d417785da4bc512cce2b274bb00d4cc6792) )
	ROM_LOAD( "c632.ic1",     0x0000, 0x0117, CRC(0fbd9270) SHA1(d7e737b20c44d41e29ca94be56114b31934dde81) )
ROM_END

/* B-Board 91634B-2 */
ROM_START( sf2cejc )
	ROM_REGION( CODE_SIZE, "maincpu", 0 )      /* 68000 code */
	ROM_LOAD16_WORD_SWAP( "s92j_23c.8f", 0x000000, 0x80000, CRC(f0120635) SHA1(5e4a9a4b0f65c6139e76ee4ffa02b9db245b1858) )
	ROM_LOAD16_WORD_SWAP( "s92j_22c.7f", 0x080000, 0x80000, CRC(8c0b2ed6) SHA1(408db039b4dad72b41458723575ed5352b71e10b) )
	ROM_LOAD16_WORD_SWAP( "s92j_21a.6f", 0x100000, 0x80000, CRC(925a7877) SHA1(1960dca35f0ca6f2b399a9fccfbc0132ac6425d1) )  // == s92_21a.6f

	ROM_REGION( 0x600000, "gfx", 0 )
	ROMX_LOAD( "s92_01.3a",  0x000000, 0x80000, CRC(03b0d852) SHA1(f370f25c96ad2b94f8c53d6b7139100285a25bef) , ROM_GROUPWORD | ROM_SKIP(6) )    // == s92-1m.3a
	ROMX_LOAD( "s92_02.4a",  0x000002, 0x80000, CRC(840289ec) SHA1(2fb42a242f60ba7e74009b5a90eb26e035ba1e82) , ROM_GROUPWORD | ROM_SKIP(6) )    // == s92-3m.5a
	ROMX_LOAD( "s92_03.5a",  0x000004, 0x80000, CRC(cdb5f027) SHA1(4c7d944fef200fdfcaf57758b901b5511188ed2e) , ROM_GROUPWORD | ROM_SKIP(6) )    // == s92-2m.4a
	ROMX_LOAD( "s92_04.6a",  0x000006, 0x80000, CRC(e2799472) SHA1(27d3796429338d82a8de246a0ea06dd487a87768) , ROM_GROUPWORD | ROM_SKIP(6) )    // == s92-4m.6a
	ROMX_LOAD( "s92_05.7a",  0x200000, 0x80000, CRC(ba8a2761) SHA1(4b696d66c51611e43522bed752654314e76d33b6) , ROM_GROUPWORD | ROM_SKIP(6) )    // == s92-5m.7a
	ROMX_LOAD( "s92_06.8a",  0x200002, 0x80000, CRC(e584bfb5) SHA1(ebdf1f5e2638eed3a65dda82b1ed9151a355f4c9) , ROM_GROUPWORD | ROM_SKIP(6) )    // == s92-7m.9a
	ROMX_LOAD( "s92_07.9a",  0x200004, 0x80000, CRC(21e3f87d) SHA1(4a4961bb68c3a1ce15f9d393d9c03ecb2466cc29) , ROM_GROUPWORD | ROM_SKIP(6) )    // == s92-6m.8a
	ROMX_LOAD( "s92_08.10a", 0x200006, 0x80000, CRC(befc47df) SHA1(520390420da3a0271ba90b0a933e65143265e5cf) , ROM_GROUPWORD | ROM_SKIP(6) )    // == s92-8m.10a
	ROMX_LOAD( "s92_10.3c",  0x400000, 0x80000, CRC(960687d5) SHA1(2868c31121b1c7564e9767b9a19cdbf655c7ed1d) , ROM_GROUPWORD | ROM_SKIP(6) )    // == s92-10m.3c
	ROMX_LOAD( "s92_11.4c",  0x400002, 0x80000, CRC(978ecd18) SHA1(648a59706b93c84b4206a968ecbdc3e834c476f6) , ROM_GROUPWORD | ROM_SKIP(6) )    // == s92-12m.5c
	ROMX_LOAD( "s92_12.5c",  0x400004, 0x80000, CRC(d6ec9a0a) SHA1(ed6143f8737013b6ef1684e37c05e037e7a80dae) , ROM_GROUPWORD | ROM_SKIP(6) )    // == s92-11m.4c
	ROMX_LOAD( "s92_13.6c",  0x400006, 0x80000, CRC(ed2c67f6) SHA1(0083c0ffaf6fe7659ff0cf822be4346cd6e61329) , ROM_GROUPWORD | ROM_SKIP(6) )    // == s92-13m.6c

	ROM_REGION( 0x18000, "audiocpu", 0 ) /* 64k for the audio CPU (+banks) */
	ROM_LOAD( "s92_09.12a",  0x00000, 0x08000, CRC(08f6b60e) SHA1(8258fcaca4ac419312531eec67079b97f471179c) )   // == s92_09.11a
	ROM_CONTINUE(            0x10000, 0x08000 )

	ROM_REGION( 0x40000, "oki", 0 ) /* Samples */
	ROM_LOAD( "s92_18.11c",  0x00000, 0x20000, CRC(7f162009) SHA1(346bf42992b4c36c593e21901e22c87ae4a7d86d) )
	ROM_LOAD( "s92_19.12c",  0x20000, 0x20000, CRC(beade53f) SHA1(277c397dc12752719ec6b47d2224750bd1c07f79) )

	ROM_REGION( 0x0200, "aboardplds", 0 )
	ROM_LOAD( "buf1",         0x0000, 0x0117, CRC(eb122de7) SHA1(b26b5bfe258e3e184f069719f9fd008d6b8f6b9b) )
	ROM_LOAD( "ioa1",         0x0000, 0x0117, CRC(59c7ee3b) SHA1(fbb887c5b4f5cb8df77cec710eaac2985bc482a6) )
	ROM_LOAD( "prg1",         0x0000, 0x0117, CRC(f1129744) SHA1(a5300f301c1a08a7da768f0773fa0fe3f683b237) )
	ROM_LOAD( "rom1",         0x0000, 0x0117, CRC(41dc73b9) SHA1(7d4c9f1693c821fbf84e32dd6ef62ddf14967845) )
	ROM_LOAD( "sou1",         0x0000, 0x0117, CRC(84f4b2fe) SHA1(dcc9e86cc36316fe42eace02d6df75d08bc8bb6d) )

	ROM_REGION( 0x0200, "bboardplds", 0 )
	ROM_LOAD( "s9263b.1a",    0x0000, 0x0117, CRC(0a7ecfe0) SHA1(f75e7eed4604fcf68273197fe3dd7f0d7a313ada) )
	ROM_LOAD( "iob1.12d",     0x0000, 0x0117, CRC(3abc0700) SHA1(973043aa46ec6d5d1db20dc9d5937005a0f9f6ae) )
	ROM_LOAD( "bprg1.11d",    0x0000, 0x0117, CRC(31793da7) SHA1(400fa7ac517421c978c1ee7773c30b9ed0c5d3f3) )

	ROM_REGION( 0x0200, "cboardplds", 0 )
	ROM_LOAD( "ioc1.ic7",     0x0000, 0x0117, CRC(0d182081) SHA1(475b3d417785da4bc512cce2b274bb00d4cc6792) )
	ROM_LOAD( "c632.ic1",     0x0000, 0x0117, CRC(0fbd9270) SHA1(d7e737b20c44d41e29ca94be56114b31934dde81) )
ROM_END

/* B-Board 91635B-2 */
ROM_START( sf2bhh )
	ROM_REGION( CODE_SIZE, "maincpu", 0 )      /* 68000 code */
	ROM_LOAD16_WORD_SWAP( "23",           0x000000, 0x40000, CRC(5cf63a9e) SHA1(af26165cacb4838572ddf1cd7a2ff22a5dddbf3c) ) // sldh
	ROM_CONTINUE(                         0x0c0000, 0x40000 )
	ROM_LOAD16_WORD_SWAP( "22",           0x080000, 0x40000, CRC(3ed72bca) SHA1(5565aee82e6b4734c4c0d06a73d8141abb1e4119) ) // sldh
	ROM_CONTINUE(                         0x040000, 0x40000 )
	ROM_LOAD16_WORD_SWAP( "s92_21a.6f",   0x100000, 0x80000, CRC(925a7877) SHA1(1960dca35f0ca6f2b399a9fccfbc0132ac6425d1) )

	ROM_REGION( 0x600000, "gfx", 0 )
	ROMX_LOAD( "s92-1m.3a",  0x000000, 0x80000, CRC(03b0d852) SHA1(f370f25c96ad2b94f8c53d6b7139100285a25bef) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "s92-3m.5a",  0x000002, 0x80000, CRC(840289ec) SHA1(2fb42a242f60ba7e74009b5a90eb26e035ba1e82) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "s92-2m.4a",  0x000004, 0x80000, CRC(cdb5f027) SHA1(4c7d944fef200fdfcaf57758b901b5511188ed2e) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "s92-4m.6a",  0x000006, 0x80000, CRC(e2799472) SHA1(27d3796429338d82a8de246a0ea06dd487a87768) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "s92-5m.7a",  0x200000, 0x80000, CRC(ba8a2761) SHA1(4b696d66c51611e43522bed752654314e76d33b6) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "s92-7m.9a",  0x200002, 0x80000, CRC(e584bfb5) SHA1(ebdf1f5e2638eed3a65dda82b1ed9151a355f4c9) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "s92-6m.8a",  0x200004, 0x80000, CRC(21e3f87d) SHA1(4a4961bb68c3a1ce15f9d393d9c03ecb2466cc29) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "s92-8m.10a", 0x200006, 0x80000, CRC(befc47df) SHA1(520390420da3a0271ba90b0a933e65143265e5cf) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "s92-10m.3c", 0x400000, 0x80000, CRC(960687d5) SHA1(2868c31121b1c7564e9767b9a19cdbf655c7ed1d) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "s92-12m.5c", 0x400002, 0x80000, CRC(978ecd18) SHA1(648a59706b93c84b4206a968ecbdc3e834c476f6) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "s92-11m.4c", 0x400004, 0x80000, CRC(d6ec9a0a) SHA1(ed6143f8737013b6ef1684e37c05e037e7a80dae) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "s92-13m.6c", 0x400006, 0x80000, CRC(ed2c67f6) SHA1(0083c0ffaf6fe7659ff0cf822be4346cd6e61329) , ROM_GROUPWORD | ROM_SKIP(6) )

	ROM_REGION( 0x18000, "audiocpu", 0 ) /* 64k for the audio CPU (+banks) */
	ROM_LOAD( "s92_09.11a",  0x00000, 0x08000, CRC(08f6b60e) SHA1(8258fcaca4ac419312531eec67079b97f471179c) )
	ROM_CONTINUE(            0x10000, 0x08000 )

	ROM_REGION( 0x40000, "oki", 0 ) /* Samples */
	ROM_LOAD( "s92_18.11c",  0x00000, 0x20000, CRC(7f162009) SHA1(346bf42992b4c36c593e21901e22c87ae4a7d86d) )
	ROM_LOAD( "s92_19.12c",  0x20000, 0x20000, CRC(beade53f) SHA1(277c397dc12752719ec6b47d2224750bd1c07f79) )

	ROM_REGION( 0x0200, "aboardplds", 0 )
	ROM_LOAD( "buf1",         0x0000, 0x0117, CRC(eb122de7) SHA1(b26b5bfe258e3e184f069719f9fd008d6b8f6b9b) )
	ROM_LOAD( "ioa1",         0x0000, 0x0117, CRC(59c7ee3b) SHA1(fbb887c5b4f5cb8df77cec710eaac2985bc482a6) )
	ROM_LOAD( "prg1",         0x0000, 0x0117, CRC(f1129744) SHA1(a5300f301c1a08a7da768f0773fa0fe3f683b237) )
	ROM_LOAD( "rom1",         0x0000, 0x0117, CRC(41dc73b9) SHA1(7d4c9f1693c821fbf84e32dd6ef62ddf14967845) )
	ROM_LOAD( "sou1",         0x0000, 0x0117, CRC(84f4b2fe) SHA1(dcc9e86cc36316fe42eace02d6df75d08bc8bb6d) )

	ROM_REGION( 0x0200, "bboardplds", 0 )
	ROM_LOAD( "s9263b.1a",    0x0000, 0x0117, CRC(0a7ecfe0) SHA1(f75e7eed4604fcf68273197fe3dd7f0d7a313ada) )
	ROM_LOAD( "iob1.12d",     0x0000, 0x0117, CRC(3abc0700) SHA1(973043aa46ec6d5d1db20dc9d5937005a0f9f6ae) )
	ROM_LOAD( "bprg1.11d",    0x0000, 0x0117, CRC(31793da7) SHA1(400fa7ac517421c978c1ee7773c30b9ed0c5d3f3) )

	ROM_REGION( 0x0200, "cboardplds", 0 )
	ROM_LOAD( "ioc1.ic7",     0x0000, 0x0117, CRC(0d182081) SHA1(475b3d417785da4bc512cce2b274bb00d4cc6792) )
	ROM_LOAD( "c632.ic1",     0x0000, 0x0117, CRC(0fbd9270) SHA1(d7e737b20c44d41e29ca94be56114b31934dde81) )
ROM_END

ROM_START( sf2rb )
	ROM_REGION( CODE_SIZE, "maincpu", 0 )      /* 68000 code */
	ROM_LOAD16_WORD( "sf2d__23.rom",      0x000000, 0x80000, CRC(450532b0) SHA1(14d5ff44ce97247ef4c42147157856d16c5fb4b8) )
	ROM_LOAD16_WORD( "sf2d__22.rom",      0x080000, 0x80000, CRC(fe9d9cf5) SHA1(91afb25d8c0fd1a721f982cebf8fdf563fe11760) )
	ROM_LOAD16_WORD_SWAP( "s92_21a.bin",  0x100000, 0x80000, CRC(925a7877) SHA1(1960dca35f0ca6f2b399a9fccfbc0132ac6425d1) )

	ROM_REGION( 0x600000, "gfx", 0 )
	ROMX_LOAD( "s92_01.bin",   0x000000, 0x80000, CRC(03b0d852) SHA1(f370f25c96ad2b94f8c53d6b7139100285a25bef) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "s92_02.bin",   0x000002, 0x80000, CRC(840289ec) SHA1(2fb42a242f60ba7e74009b5a90eb26e035ba1e82) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "s92_03.bin",   0x000004, 0x80000, CRC(cdb5f027) SHA1(4c7d944fef200fdfcaf57758b901b5511188ed2e) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "s92_04.bin",   0x000006, 0x80000, CRC(e2799472) SHA1(27d3796429338d82a8de246a0ea06dd487a87768) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "s92_05.bin",   0x200000, 0x80000, CRC(ba8a2761) SHA1(4b696d66c51611e43522bed752654314e76d33b6) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "s92_06.bin",   0x200002, 0x80000, CRC(e584bfb5) SHA1(ebdf1f5e2638eed3a65dda82b1ed9151a355f4c9) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "s92_07.bin",   0x200004, 0x80000, CRC(21e3f87d) SHA1(4a4961bb68c3a1ce15f9d393d9c03ecb2466cc29) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "s92_08.bin",   0x200006, 0x80000, CRC(befc47df) SHA1(520390420da3a0271ba90b0a933e65143265e5cf) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "s92_10.bin",   0x400000, 0x80000, CRC(960687d5) SHA1(2868c31121b1c7564e9767b9a19cdbf655c7ed1d) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "s92_11.bin",   0x400002, 0x80000, CRC(978ecd18) SHA1(648a59706b93c84b4206a968ecbdc3e834c476f6) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "s92_12.bin",   0x400004, 0x80000, CRC(d6ec9a0a) SHA1(ed6143f8737013b6ef1684e37c05e037e7a80dae) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "s92_13.bin",   0x400006, 0x80000, CRC(ed2c67f6) SHA1(0083c0ffaf6fe7659ff0cf822be4346cd6e61329) , ROM_GROUPWORD | ROM_SKIP(6) )

	ROM_REGION( 0x18000, "audiocpu", 0 ) /* 64k for the audio CPU (+banks) */
	ROM_LOAD( "s92_09.bin",    0x00000, 0x08000, CRC(08f6b60e) SHA1(8258fcaca4ac419312531eec67079b97f471179c) )
	ROM_CONTINUE(              0x10000, 0x08000 )

	ROM_REGION( 0x40000, "oki", 0 ) /* Samples */
	ROM_LOAD( "s92_18.bin",    0x00000, 0x20000, CRC(7f162009) SHA1(346bf42992b4c36c593e21901e22c87ae4a7d86d) )
	ROM_LOAD( "s92_19.bin",    0x20000, 0x20000, CRC(beade53f) SHA1(277c397dc12752719ec6b47d2224750bd1c07f79) )
ROM_END

ROM_START( sf2rb2 )
	ROM_REGION( CODE_SIZE, "maincpu", 0 )      /* 68000 code */
	ROM_LOAD16_BYTE( "27.bin",            0x000000, 0x20000, CRC(40296ecd) SHA1(6006b9bc7e2e8ccec47f87b51791d3c0512620df) )
	ROM_LOAD16_BYTE( "31.bin",            0x000001, 0x20000, CRC(87954a41) SHA1(67225f180e1f954f0bebba49b618f793a973af14) )
	ROM_LOAD16_BYTE( "26.bin",            0x040000, 0x20000, CRC(a6974195) SHA1(f7e0fd43bd75229d49d5c330820bdc5c3b11ab03) )
	ROM_LOAD16_BYTE( "30.bin",            0x040001, 0x20000, CRC(8141fe32) SHA1(e6ea1ee331f674c64e63a776ad4e428f6081c79c) )
	ROM_LOAD16_BYTE( "25.bin",            0x080000, 0x20000, CRC(9ef8f772) SHA1(3ee271413521cc2d6ac9544e401ff38eff8a1347) )
	ROM_LOAD16_BYTE( "29.bin",            0x080001, 0x20000, CRC(7d9c479c) SHA1(a1195444caac5230a1f74f3444b024ceaf1d0667) )
	ROM_LOAD16_BYTE( "24.bin",            0x0c0000, 0x20000, CRC(93579684) SHA1(9052b46f635cae7843e9d37a601db0189a89e0f9) )
	ROM_LOAD16_BYTE( "28.bin",            0x0c0001, 0x20000, CRC(ff728865) SHA1(ad4522294ff2e02b594d960b45940a3e57a5d1ec) )
	ROM_LOAD16_WORD_SWAP( "s92_21a.bin",  0x100000, 0x80000, CRC(925a7877) SHA1(1960dca35f0ca6f2b399a9fccfbc0132ac6425d1) )

	ROM_REGION( 0x600000, "gfx", 0 )
	ROMX_LOAD( "s92_01.bin",   0x000000, 0x80000, CRC(03b0d852) SHA1(f370f25c96ad2b94f8c53d6b7139100285a25bef) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "s92_02.bin",   0x000002, 0x80000, CRC(840289ec) SHA1(2fb42a242f60ba7e74009b5a90eb26e035ba1e82) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "s92_03.bin",   0x000004, 0x80000, CRC(cdb5f027) SHA1(4c7d944fef200fdfcaf57758b901b5511188ed2e) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "s92_04.bin",   0x000006, 0x80000, CRC(e2799472) SHA1(27d3796429338d82a8de246a0ea06dd487a87768) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "s92_05.bin",   0x200000, 0x80000, CRC(ba8a2761) SHA1(4b696d66c51611e43522bed752654314e76d33b6) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "s92_06.bin",   0x200002, 0x80000, CRC(e584bfb5) SHA1(ebdf1f5e2638eed3a65dda82b1ed9151a355f4c9) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "s92_07.bin",   0x200004, 0x80000, CRC(21e3f87d) SHA1(4a4961bb68c3a1ce15f9d393d9c03ecb2466cc29) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "s92_08.bin",   0x200006, 0x80000, CRC(befc47df) SHA1(520390420da3a0271ba90b0a933e65143265e5cf) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "s92_10.bin",   0x400000, 0x80000, CRC(960687d5) SHA1(2868c31121b1c7564e9767b9a19cdbf655c7ed1d) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "s92_11.bin",   0x400002, 0x80000, CRC(978ecd18) SHA1(648a59706b93c84b4206a968ecbdc3e834c476f6) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "s92_12.bin",   0x400004, 0x80000, CRC(d6ec9a0a) SHA1(ed6143f8737013b6ef1684e37c05e037e7a80dae) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "s92_13.bin",   0x400006, 0x80000, CRC(ed2c67f6) SHA1(0083c0ffaf6fe7659ff0cf822be4346cd6e61329) , ROM_GROUPWORD | ROM_SKIP(6) )

	ROM_REGION( 0x18000, "audiocpu", 0 ) /* 64k for the audio CPU (+banks) */
	ROM_LOAD( "s92_09.bin",    0x00000, 0x08000, CRC(08f6b60e) SHA1(8258fcaca4ac419312531eec67079b97f471179c) )
	ROM_CONTINUE(              0x10000, 0x08000 )

	ROM_REGION( 0x40000, "oki", 0 ) /* Samples */
	ROM_LOAD( "s92_18.bin",    0x00000, 0x20000, CRC(7f162009) SHA1(346bf42992b4c36c593e21901e22c87ae4a7d86d) )
	ROM_LOAD( "s92_19.bin",    0x20000, 0x20000, CRC(beade53f) SHA1(277c397dc12752719ec6b47d2224750bd1c07f79) )
ROM_END

/* this rainbow set DOES NOT require a custom PLD to work, runs on standard board with roms replaced */
ROM_START( sf2rb3 )
	ROM_REGION( CODE_SIZE, "maincpu", 0 )      /* 68000 code */
	ROM_LOAD16_WORD_SWAP( "sf2_ce_rb.23",      0x000000, 0x80000, CRC(202f9e50) SHA1(8f0259ade1bc4df65abf4ad0961db24ca27e3f4b) )
	ROM_LOAD16_WORD_SWAP( "sf2_ce_rb.22",      0x080000, 0x80000, CRC(145e5219) SHA1(0b1251ad817a395f37f6c9acee393c3fce07777a) )
	ROM_LOAD16_WORD_SWAP( "s92_21a.bin",       0x100000, 0x80000, CRC(925a7877) SHA1(1960dca35f0ca6f2b399a9fccfbc0132ac6425d1) )

	ROM_REGION( 0x600000, "gfx", 0 )
	ROMX_LOAD( "s92_01.bin",   0x000000, 0x80000, CRC(03b0d852) SHA1(f370f25c96ad2b94f8c53d6b7139100285a25bef) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "s92_02.bin",   0x000002, 0x80000, CRC(840289ec) SHA1(2fb42a242f60ba7e74009b5a90eb26e035ba1e82) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "s92_03.bin",   0x000004, 0x80000, CRC(cdb5f027) SHA1(4c7d944fef200fdfcaf57758b901b5511188ed2e) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "s92_04.bin",   0x000006, 0x80000, CRC(e2799472) SHA1(27d3796429338d82a8de246a0ea06dd487a87768) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "s92_05.bin",   0x200000, 0x80000, CRC(ba8a2761) SHA1(4b696d66c51611e43522bed752654314e76d33b6) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "s92_06.bin",   0x200002, 0x80000, CRC(e584bfb5) SHA1(ebdf1f5e2638eed3a65dda82b1ed9151a355f4c9) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "s92_07.bin",   0x200004, 0x80000, CRC(21e3f87d) SHA1(4a4961bb68c3a1ce15f9d393d9c03ecb2466cc29) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "s92_08.bin",   0x200006, 0x80000, CRC(befc47df) SHA1(520390420da3a0271ba90b0a933e65143265e5cf) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "s92_10.bin",   0x400000, 0x80000, CRC(960687d5) SHA1(2868c31121b1c7564e9767b9a19cdbf655c7ed1d) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "s92_11.bin",   0x400002, 0x80000, CRC(978ecd18) SHA1(648a59706b93c84b4206a968ecbdc3e834c476f6) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "s92_12.bin",   0x400004, 0x80000, CRC(d6ec9a0a) SHA1(ed6143f8737013b6ef1684e37c05e037e7a80dae) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "s92_13.bin",   0x400006, 0x80000, CRC(ed2c67f6) SHA1(0083c0ffaf6fe7659ff0cf822be4346cd6e61329) , ROM_GROUPWORD | ROM_SKIP(6) )

	ROM_REGION( 0x18000, "audiocpu", 0 ) /* 64k for the audio CPU (+banks) */
	ROM_LOAD( "s92_09.bin",    0x00000, 0x08000, CRC(08f6b60e) SHA1(8258fcaca4ac419312531eec67079b97f471179c) )
	ROM_CONTINUE(              0x10000, 0x08000 )

	ROM_REGION( 0x40000, "oki", 0 ) /* Samples */
	ROM_LOAD( "s92_18.bin",    0x00000, 0x20000, CRC(7f162009) SHA1(346bf42992b4c36c593e21901e22c87ae4a7d86d) )
	ROM_LOAD( "s92_19.bin",    0x20000, 0x20000, CRC(beade53f) SHA1(277c397dc12752719ec6b47d2224750bd1c07f79) )
ROM_END

ROM_START( sf2red )
	ROM_REGION( CODE_SIZE, "maincpu", 0 )      /* 68000 code */
	ROM_LOAD16_WORD_SWAP( "sf2red.23",    0x000000, 0x80000, CRC(40276abb) SHA1(a991661f5a1a3116445594bcfed3150e36971dd7) )
	ROM_LOAD16_WORD_SWAP( "sf2red.22",    0x080000, 0x80000, CRC(18daf387) SHA1(1a9e4c04ca54e8b33f19dd7bedbe05a200249701) )
	ROM_LOAD16_WORD_SWAP( "sf2red.21",    0x100000, 0x80000, CRC(52c486bb) SHA1(b7df7b10faa4c9a2f86ebf64cd63ac148d62dd09) )

	ROM_REGION( 0x600000, "gfx", 0 )
	ROMX_LOAD( "s92_01.bin",   0x000000, 0x80000, CRC(03b0d852) SHA1(f370f25c96ad2b94f8c53d6b7139100285a25bef) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "s92_02.bin",   0x000002, 0x80000, CRC(840289ec) SHA1(2fb42a242f60ba7e74009b5a90eb26e035ba1e82) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "s92_03.bin",   0x000004, 0x80000, CRC(cdb5f027) SHA1(4c7d944fef200fdfcaf57758b901b5511188ed2e) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "s92_04.bin",   0x000006, 0x80000, CRC(e2799472) SHA1(27d3796429338d82a8de246a0ea06dd487a87768) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "s92_05.bin",   0x200000, 0x80000, CRC(ba8a2761) SHA1(4b696d66c51611e43522bed752654314e76d33b6) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "s92_06.bin",   0x200002, 0x80000, CRC(e584bfb5) SHA1(ebdf1f5e2638eed3a65dda82b1ed9151a355f4c9) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "s92_07.bin",   0x200004, 0x80000, CRC(21e3f87d) SHA1(4a4961bb68c3a1ce15f9d393d9c03ecb2466cc29) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "s92_08.bin",   0x200006, 0x80000, CRC(befc47df) SHA1(520390420da3a0271ba90b0a933e65143265e5cf) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "s92_10.bin",   0x400000, 0x80000, CRC(960687d5) SHA1(2868c31121b1c7564e9767b9a19cdbf655c7ed1d) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "s92_11.bin",   0x400002, 0x80000, CRC(978ecd18) SHA1(648a59706b93c84b4206a968ecbdc3e834c476f6) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "s92_12.bin",   0x400004, 0x80000, CRC(d6ec9a0a) SHA1(ed6143f8737013b6ef1684e37c05e037e7a80dae) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "s92_13.bin",   0x400006, 0x80000, CRC(ed2c67f6) SHA1(0083c0ffaf6fe7659ff0cf822be4346cd6e61329) , ROM_GROUPWORD | ROM_SKIP(6) )

	ROM_REGION( 0x18000, "audiocpu", 0 ) /* 64k for the audio CPU (+banks) */
	ROM_LOAD( "s92_09.bin",    0x00000, 0x08000, CRC(08f6b60e) SHA1(8258fcaca4ac419312531eec67079b97f471179c) )
	ROM_CONTINUE(              0x10000, 0x08000 )

	ROM_REGION( 0x40000, "oki", 0 ) /* Samples */
	ROM_LOAD( "s92_18.bin",    0x00000, 0x20000, CRC(7f162009) SHA1(346bf42992b4c36c593e21901e22c87ae4a7d86d) )
	ROM_LOAD( "s92_19.bin",    0x20000, 0x20000, CRC(beade53f) SHA1(277c397dc12752719ec6b47d2224750bd1c07f79) )
ROM_END

ROM_START( sf2v004 )
	ROM_REGION( CODE_SIZE, "maincpu", 0 )      /* 68000 code */
	ROM_LOAD16_WORD_SWAP( "sf2v004.23",   0x000000, 0x80000, CRC(52d19f2c) SHA1(6a77b9244dc9b7d9a0ca8a642d4257cc944ac566) )
	ROM_LOAD16_WORD_SWAP( "sf2v004.22",   0x080000, 0x80000, CRC(4b26fde7) SHA1(48e3aacbf9147f2374a93e10f945291c87f24855) )
	ROM_LOAD16_WORD_SWAP( "sf2red.21",    0x100000, 0x80000, CRC(52c486bb) SHA1(b7df7b10faa4c9a2f86ebf64cd63ac148d62dd09) )

	ROM_REGION( 0x600000, "gfx", 0 )
	ROMX_LOAD( "s92_01.bin",   0x000000, 0x80000, CRC(03b0d852) SHA1(f370f25c96ad2b94f8c53d6b7139100285a25bef) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "s92_02.bin",   0x000002, 0x80000, CRC(840289ec) SHA1(2fb42a242f60ba7e74009b5a90eb26e035ba1e82) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "s92_03.bin",   0x000004, 0x80000, CRC(cdb5f027) SHA1(4c7d944fef200fdfcaf57758b901b5511188ed2e) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "s92_04.bin",   0x000006, 0x80000, CRC(e2799472) SHA1(27d3796429338d82a8de246a0ea06dd487a87768) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "s92_05.bin",   0x200000, 0x80000, CRC(ba8a2761) SHA1(4b696d66c51611e43522bed752654314e76d33b6) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "s92_06.bin",   0x200002, 0x80000, CRC(e584bfb5) SHA1(ebdf1f5e2638eed3a65dda82b1ed9151a355f4c9) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "s92_07.bin",   0x200004, 0x80000, CRC(21e3f87d) SHA1(4a4961bb68c3a1ce15f9d393d9c03ecb2466cc29) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "s92_08.bin",   0x200006, 0x80000, CRC(befc47df) SHA1(520390420da3a0271ba90b0a933e65143265e5cf) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "s92_10.bin",   0x400000, 0x80000, CRC(960687d5) SHA1(2868c31121b1c7564e9767b9a19cdbf655c7ed1d) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "s92_11.bin",   0x400002, 0x80000, CRC(978ecd18) SHA1(648a59706b93c84b4206a968ecbdc3e834c476f6) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "s92_12.bin",   0x400004, 0x80000, CRC(d6ec9a0a) SHA1(ed6143f8737013b6ef1684e37c05e037e7a80dae) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "s92_13.bin",   0x400006, 0x80000, CRC(ed2c67f6) SHA1(0083c0ffaf6fe7659ff0cf822be4346cd6e61329) , ROM_GROUPWORD | ROM_SKIP(6) )

	ROM_REGION( 0x18000, "audiocpu", 0 ) /* 64k for the audio CPU (+banks) */
	ROM_LOAD( "s92_09.bin",    0x00000, 0x08000, CRC(08f6b60e) SHA1(8258fcaca4ac419312531eec67079b97f471179c) )
	ROM_CONTINUE(              0x10000, 0x08000 )

	ROM_REGION( 0x40000, "oki", 0 ) /* Samples */
	ROM_LOAD( "s92_18.bin",    0x00000, 0x20000, CRC(7f162009) SHA1(346bf42992b4c36c593e21901e22c87ae4a7d86d) )
	ROM_LOAD( "s92_19.bin",    0x20000, 0x20000, CRC(beade53f) SHA1(277c397dc12752719ec6b47d2224750bd1c07f79) )
ROM_END

ROM_START( sf2acc )
	ROM_REGION( CODE_SIZE, "maincpu", 0 )      /* 68000 code */
	ROM_LOAD16_WORD_SWAP( "sf2ca_23-c.bin", 0x000000, 0x80000, CRC(e7c8c5a6) SHA1(247fb38c041f4d516899db32545d8b4d8debc4cf) )
	ROM_LOAD16_WORD_SWAP( "sf2ca_22-c.bin", 0x080000, 0x80000, CRC(99f1cca4) SHA1(64111eba81d743fc3fd51d7a89cd0b2eefcc900d) )
	ROM_LOAD16_WORD_SWAP( "sf2ca_21-c.bin", 0x100000, 0x40000, CRC(cf7fcc8c) SHA1(47d338634dd72d1ef912fbdb8c1a7ae945ca1a5f) )

	ROM_REGION( 0x600000, "gfx", 0 )
	ROMX_LOAD( "s92_01.bin",   0x000000, 0x80000, CRC(03b0d852) SHA1(f370f25c96ad2b94f8c53d6b7139100285a25bef) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "s92_02.bin",   0x000002, 0x80000, CRC(840289ec) SHA1(2fb42a242f60ba7e74009b5a90eb26e035ba1e82) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "s92_03.bin",   0x000004, 0x80000, CRC(cdb5f027) SHA1(4c7d944fef200fdfcaf57758b901b5511188ed2e) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "s92_04.bin",   0x000006, 0x80000, CRC(e2799472) SHA1(27d3796429338d82a8de246a0ea06dd487a87768) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "s92_05.bin",   0x200000, 0x80000, CRC(ba8a2761) SHA1(4b696d66c51611e43522bed752654314e76d33b6) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "s92_06.bin",   0x200002, 0x80000, CRC(e584bfb5) SHA1(ebdf1f5e2638eed3a65dda82b1ed9151a355f4c9) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "s92_07.bin",   0x200004, 0x80000, CRC(21e3f87d) SHA1(4a4961bb68c3a1ce15f9d393d9c03ecb2466cc29) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "s92_08.bin",   0x200006, 0x80000, CRC(befc47df) SHA1(520390420da3a0271ba90b0a933e65143265e5cf) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "s92_10.bin",   0x400000, 0x80000, CRC(960687d5) SHA1(2868c31121b1c7564e9767b9a19cdbf655c7ed1d) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "s92_11.bin",   0x400002, 0x80000, CRC(978ecd18) SHA1(648a59706b93c84b4206a968ecbdc3e834c476f6) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "s92_12.bin",   0x400004, 0x80000, CRC(d6ec9a0a) SHA1(ed6143f8737013b6ef1684e37c05e037e7a80dae) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "s92_13.bin",   0x400006, 0x80000, CRC(ed2c67f6) SHA1(0083c0ffaf6fe7659ff0cf822be4346cd6e61329) , ROM_GROUPWORD | ROM_SKIP(6) )

	ROM_REGION( 0x18000, "audiocpu", 0 ) /* 64k for the audio CPU (+banks) */
	ROM_LOAD( "s92_09.bin",    0x00000, 0x08000, CRC(08f6b60e) SHA1(8258fcaca4ac419312531eec67079b97f471179c) )
	ROM_CONTINUE(              0x10000, 0x08000 )

	ROM_REGION( 0x40000, "oki", 0 ) /* Samples */
	ROM_LOAD( "s92_18.bin",    0x00000, 0x20000, CRC(7f162009) SHA1(346bf42992b4c36c593e21901e22c87ae4a7d86d) )
	ROM_LOAD( "s92_19.bin",    0x20000, 0x20000, CRC(beade53f) SHA1(277c397dc12752719ec6b47d2224750bd1c07f79) )
ROM_END



ROM_START( sf2acca )
	ROM_REGION( CODE_SIZE, "maincpu", 0 )      /* 68000 code */
	ROM_LOAD16_WORD_SWAP( "23-c.8f", 0x000000, 0x80000, CRC(35f9517b) SHA1(0976870c65d69a6d8ca0232a48e042588a9ec5d8) )
	ROM_LOAD16_WORD_SWAP( "22-c.7f", 0x080000, 0x80000, CRC(99f1cca4) SHA1(64111eba81d743fc3fd51d7a89cd0b2eefcc900d) )
	ROM_LOAD16_WORD_SWAP( "21-c.6f", 0x100000, 0x80000, CRC(2ab2034f) SHA1(4dc90ba524e76529700945605e02068d49b56aba) )

	ROM_REGION( 0x600000, "gfx", 0 )
	ROMX_LOAD( "s92_01.bin",   0x000000, 0x80000, CRC(03b0d852) SHA1(f370f25c96ad2b94f8c53d6b7139100285a25bef) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "s92_02.bin",   0x000002, 0x80000, CRC(840289ec) SHA1(2fb42a242f60ba7e74009b5a90eb26e035ba1e82) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "s92_03.bin",   0x000004, 0x80000, CRC(cdb5f027) SHA1(4c7d944fef200fdfcaf57758b901b5511188ed2e) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "s92_04.bin",   0x000006, 0x80000, CRC(e2799472) SHA1(27d3796429338d82a8de246a0ea06dd487a87768) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "s92_05.bin",   0x200000, 0x80000, CRC(ba8a2761) SHA1(4b696d66c51611e43522bed752654314e76d33b6) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "s92_06.bin",   0x200002, 0x80000, CRC(e584bfb5) SHA1(ebdf1f5e2638eed3a65dda82b1ed9151a355f4c9) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "s92_07.bin",   0x200004, 0x80000, CRC(21e3f87d) SHA1(4a4961bb68c3a1ce15f9d393d9c03ecb2466cc29) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "s92_08.bin",   0x200006, 0x80000, CRC(befc47df) SHA1(520390420da3a0271ba90b0a933e65143265e5cf) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "s92_10.bin",   0x400000, 0x80000, CRC(960687d5) SHA1(2868c31121b1c7564e9767b9a19cdbf655c7ed1d) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "s92_11.bin",   0x400002, 0x80000, CRC(978ecd18) SHA1(648a59706b93c84b4206a968ecbdc3e834c476f6) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "s92_12.bin",   0x400004, 0x80000, CRC(d6ec9a0a) SHA1(ed6143f8737013b6ef1684e37c05e037e7a80dae) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "s92_13.bin",   0x400006, 0x80000, CRC(ed2c67f6) SHA1(0083c0ffaf6fe7659ff0cf822be4346cd6e61329) , ROM_GROUPWORD | ROM_SKIP(6) )

	ROM_REGION( 0x18000, "audiocpu", 0 ) /* 64k for the audio CPU (+banks) */
	ROM_LOAD( "s92_09.bin",    0x00000, 0x08000, CRC(08f6b60e) SHA1(8258fcaca4ac419312531eec67079b97f471179c) )
	ROM_CONTINUE(              0x10000, 0x08000 )

	ROM_REGION( 0x40000, "oki", 0 ) /* Samples */
	ROM_LOAD( "s92_18.bin",    0x00000, 0x20000, CRC(7f162009) SHA1(346bf42992b4c36c593e21901e22c87ae4a7d86d) )
	ROM_LOAD( "s92_19.bin",    0x20000, 0x20000, CRC(beade53f) SHA1(277c397dc12752719ec6b47d2224750bd1c07f79) )
ROM_END

ROM_START( sf2accp2 )
	ROM_REGION( CODE_SIZE, "maincpu", 0 )      /* 68000 code */
	ROM_LOAD16_WORD_SWAP( "sf2ca-23.bin", 0x000000, 0x80000, CRC(36c3ba2f) SHA1(a3ddc479b725ddb3521757c8efa7aed125004997) )
	ROM_LOAD16_WORD_SWAP( "sf2ca-22.bin", 0x080000, 0x80000, CRC(0550453d) SHA1(f9efed86528dd10f142636278f098584d33ccde6) )
	ROM_LOAD16_WORD_SWAP( "sf2ca-21.bin", 0x100000, 0x40000, CRC(4c1c43ba) SHA1(16abce268373eddd7c3b93fe8d44b200a8c140fe) )
	/* ROM space ends at 13ffff, but the code checks 180ca6 and */
	/* crashes if it isn't 0 - protection? */

	ROM_REGION( 0x600000, "gfx", 0 )
	ROMX_LOAD( "s92_01.bin",   0x000000, 0x80000, CRC(03b0d852) SHA1(f370f25c96ad2b94f8c53d6b7139100285a25bef) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "s92_02.bin",   0x000002, 0x80000, CRC(840289ec) SHA1(2fb42a242f60ba7e74009b5a90eb26e035ba1e82) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "s92_03.bin",   0x000004, 0x80000, CRC(cdb5f027) SHA1(4c7d944fef200fdfcaf57758b901b5511188ed2e) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "s92_04.bin",   0x000006, 0x80000, CRC(e2799472) SHA1(27d3796429338d82a8de246a0ea06dd487a87768) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "s92_05.bin",   0x200000, 0x80000, CRC(ba8a2761) SHA1(4b696d66c51611e43522bed752654314e76d33b6) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "s92_06.bin",   0x200002, 0x80000, CRC(e584bfb5) SHA1(ebdf1f5e2638eed3a65dda82b1ed9151a355f4c9) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "s92_07.bin",   0x200004, 0x80000, CRC(21e3f87d) SHA1(4a4961bb68c3a1ce15f9d393d9c03ecb2466cc29) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "s92_08.bin",   0x200006, 0x80000, CRC(befc47df) SHA1(520390420da3a0271ba90b0a933e65143265e5cf) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "s92_10.bin",   0x400000, 0x80000, CRC(960687d5) SHA1(2868c31121b1c7564e9767b9a19cdbf655c7ed1d) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "s92_11.bin",   0x400002, 0x80000, CRC(978ecd18) SHA1(648a59706b93c84b4206a968ecbdc3e834c476f6) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "s92_12.bin",   0x400004, 0x80000, CRC(d6ec9a0a) SHA1(ed6143f8737013b6ef1684e37c05e037e7a80dae) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "s92_13.bin",   0x400006, 0x80000, CRC(ed2c67f6) SHA1(0083c0ffaf6fe7659ff0cf822be4346cd6e61329) , ROM_GROUPWORD | ROM_SKIP(6) )

	ROM_REGION( 0x18000, "audiocpu", 0 ) /* 64k for the audio CPU (+banks) */
	ROM_LOAD( "s92_09.bin",    0x00000, 0x08000, CRC(08f6b60e) SHA1(8258fcaca4ac419312531eec67079b97f471179c) )
	ROM_CONTINUE(              0x10000, 0x08000 )

	ROM_REGION( 0x40000, "oki", 0 ) /* Samples */
	ROM_LOAD( "s92_18.bin",    0x00000, 0x20000, CRC(7f162009) SHA1(346bf42992b4c36c593e21901e22c87ae4a7d86d) )
	ROM_LOAD( "s92_19.bin",    0x20000, 0x20000, CRC(beade53f) SHA1(277c397dc12752719ec6b47d2224750bd1c07f79) )
ROM_END


/*
SF2' CE "bootleg"

CPS1 motherboard (no dash)
Original 88622-c-2 C board

Bootleg B board with
    PIC16c55 near PRG roms seem protected
    8 dipswith near both sound roms and gfx roms
    1,4,5,8 ON
    2,3,6,7 OFF

Sound rom match various romset
*/

ROM_START( sf2ceblp )
	ROM_REGION( CODE_SIZE, "maincpu", 0 )      /* 68000 code */

	ROM_LOAD16_BYTE( "prg31.bin",      0x00000, 0x20000, CRC(ea78f9b4) SHA1(967f84615f6ef4d046df3593d5ffce0d43a3ac64) )
	ROM_LOAD16_BYTE( "prg36.bin",      0x00001, 0x20000, CRC(d30c263e) SHA1(6d8b13f82dde3237115306d91f37ac95b8238e0a) )
	ROM_LOAD16_BYTE( "prg30.bin",      0x40000, 0x20000, CRC(005b54cc) SHA1(3d3d7742c11f31cc62d81931dc970cd1f4a26835) )
	ROM_LOAD16_BYTE( "prg35.bin",      0x40001, 0x20000, CRC(c184d26d) SHA1(b1011e869d0b84c1ab609fcf193b46c7d9cc3b28) )
	ROM_LOAD16_BYTE( "prg29.bin",      0x80000, 0x20000, CRC(524f5c55) SHA1(67adfd36bb325c4a6d3e1e5b668c4dc983ac0a62) )
	ROM_LOAD16_BYTE( "prg34.bin",      0x80001, 0x20000, CRC(f06a12f2) SHA1(ddc431ce01392d4a7562760743abd9ea73b06cf3) )
	ROM_LOAD16_BYTE( "prg28.bin",      0xc0000, 0x40000, CRC(b7ad5214) SHA1(17b05e0aa9a4eb5f1aaafe35fa029d2a9aea530d) )
	ROM_LOAD16_BYTE( "prg33.bin",      0xc0001, 0x40000, CRC(6340b914) SHA1(443e37a06058548c8ce7a15ecd10a6635e69d09f) )

	ROM_REGION( 0x600000, "gfx", 0 )
	ROMX_LOAD( "24.bin",   0x000000, 0x40000, CRC(a8b5633a) SHA1(6548a89d616910d06db126eb1a9c6b5979baff03) , ROM_SKIP(7) ) // sldh
	ROMX_LOAD( "14.bin",   0x000001, 0x40000, CRC(5db24ca7) SHA1(0543e89174fecc866a08e0ecc7c31a6efca15da5) , ROM_SKIP(7) )
	ROMX_LOAD( "26.bin",   0x000002, 0x40000, CRC(82e8e384) SHA1(a8849bdb97c496777ba87e8f1942585c0fc9a29b) , ROM_SKIP(7) ) // sldh
	ROMX_LOAD( "16.bin",   0x000003, 0x40000, CRC(1fd98ad0) SHA1(c0edf8ad305d5f8ba72498024cacf28b08491a57) , ROM_SKIP(7) )
	ROMX_LOAD( "20.bin",   0x000004, 0x40000, CRC(8d5d0045) SHA1(74691132c740cc28585ebe9469fa36fea28c1cef) , ROM_SKIP(7) )
	ROMX_LOAD( "10.bin",   0x000005, 0x40000, CRC(eb48f7f2) SHA1(f1d269949a8e06434e2206842535d86a70bcf58f) , ROM_SKIP(7) )
	ROMX_LOAD( "22.bin",   0x000006, 0x40000, CRC(4109d637) SHA1(f7b0961ae3053a0751749ab77f27503e03ae1e5b) , ROM_SKIP(7) )
	ROMX_LOAD( "12.bin",   0x000007, 0x40000, CRC(5d21d8b3) SHA1(1e75a4bf8d750e0858d70a489b59b50839b8c720) , ROM_SKIP(7) )

	ROMX_LOAD( "25.bin",   0x200000, 0x40000, CRC(72e923df) SHA1(f8db61f8a768fbf99a0be54af712aa19161978d1) , ROM_SKIP(7) ) // sldh
	ROMX_LOAD( "15.bin",   0x200001, 0x40000, CRC(9a96be48) SHA1(c0a46aefba442196ead30d4f5de2b0370b1c03f0) , ROM_SKIP(7) )
	ROMX_LOAD( "27.bin",   0x200002, 0x40000, CRC(4a3a8d09) SHA1(eaa7307963c7a5381a4d84a4ebcc483b55cc1411) , ROM_SKIP(7) ) // sldh
	ROMX_LOAD( "17.bin",   0x200003, 0x40000, CRC(a917a922) SHA1(2186095f8473538756ca22dbdb2c68d17cbf24ab) , ROM_SKIP(7) )
	ROMX_LOAD( "21.bin",   0x200004, 0x40000, CRC(55c2b455) SHA1(677ac8f988ea3c0564b2dd178ee2ab7077aeafc8) , ROM_SKIP(7) )
	ROMX_LOAD( "11.bin",   0x200005, 0x40000, CRC(ff36859e) SHA1(7f5ab9daacfdb338913d6bca723efe3128dc931f) , ROM_SKIP(7) )
	ROMX_LOAD( "23.bin",   0x200006, 0x40000, CRC(ef9c2d4d) SHA1(35c3ab5a3cc3ce2219b596c4c3e97d28b9cc2297) , ROM_SKIP(7) )
	ROMX_LOAD( "13.bin",   0x200007, 0x40000, CRC(bc937c96) SHA1(772ff4e06093043f693729d8e94e7f8e3f8e866f) , ROM_SKIP(7) )

	ROMX_LOAD( "6.bin",   0x400000, 0x40000, CRC(023baa18) SHA1(671600e629aff1cc161567ee4a5ceb2228731a36) , ROM_SKIP(7) )
	ROMX_LOAD( "5.bin",   0x400001, 0x40000, CRC(a6ad6ef3) SHA1(c670931a98dfc6ca39d54d8cb8848421232d8c8c) , ROM_SKIP(7) )
	ROMX_LOAD( "8.bin",   0x400002, 0x40000, CRC(26fb340c) SHA1(270455c10a0c7e4ba95a38ab7d815faedfe51fde) , ROM_SKIP(7) )
	ROMX_LOAD( "7.bin",   0x400003, 0x40000, CRC(f56085ba) SHA1(ce8c33d7be271a501e770da5249778f62ca0e0a1) , ROM_SKIP(7) )
	ROMX_LOAD( "1.bin",   0x400004, 0x40000, CRC(877b2b18) SHA1(36ebef5b534d2449b163df38b9a98f769c1d85ed) , ROM_SKIP(7) )
	ROMX_LOAD( "2.bin",   0x400005, 0x40000, CRC(144aa4c9) SHA1(f00bf10f04ae774dda7e3a1c862aba67f70d61c3) , ROM_SKIP(7) )
	ROMX_LOAD( "4.bin",   0x400006, 0x40000, CRC(f2c400b4) SHA1(090ff7184cd2a1b992cc9f1f8c03f8832bc11416) , ROM_SKIP(7) )
	ROMX_LOAD( "3.bin",   0x400007, 0x40000, CRC(8053335d) SHA1(c653cb249401162fbf39f293e1999fe5e09c35c4) , ROM_SKIP(7) )

	ROM_REGION( 0x18000, "audiocpu", 0 ) /* 64k for the audio CPU (+banks) */
	ROM_LOAD( "s92_09.bin",    0x00000, 0x08000, CRC(08f6b60e) SHA1(8258fcaca4ac419312531eec67079b97f471179c) )
	ROM_CONTINUE(              0x10000, 0x08000 )

	ROM_REGION( 0x40000, "oki", 0 ) /* Samples */
	ROM_LOAD( "s92_18.bin",    0x00000, 0x20000, CRC(7f162009) SHA1(346bf42992b4c36c593e21901e22c87ae4a7d86d) )
	ROM_LOAD( "s92_19.bin",    0x20000, 0x20000, CRC(beade53f) SHA1(277c397dc12752719ec6b47d2224750bd1c07f79) )

	ROM_REGION( 0x2000, "protectin_pic", 0 )
	ROM_LOAD( "pic16c55",    0x00000, 0x2000, BAD_DUMP CRC(f22e2311) SHA1(320edfba140728599e91c01e863a8b6d071e4bbf) )
ROM_END



ROM_START( sf2cebltw )
	ROM_REGION( CODE_SIZE, "maincpu", 0 )      /* 68000 code */
	ROM_LOAD16_BYTE( "27.12e",           0x000000, 0x20000, CRC(035ee5d9) SHA1(9d268333db96dc6d1539ea938d3b213482e276c6) )
	ROM_LOAD16_BYTE( "31.12f",           0x000001, 0x20000, CRC(353dbde1) SHA1(039785f8863fda945267fc8b9cf640d954ea9803) )
	ROM_LOAD16_BYTE( "24.9e",            0x040000, 0x20000, CRC(005b54cc) SHA1(3d3d7742c11f31cc62d81931dc970cd1f4a26835) )
	ROM_LOAD16_BYTE( "28.9f",            0x040001, 0x20000, CRC(c184d26d) SHA1(b1011e869d0b84c1ab609fcf193b46c7d9cc3b28) )
	ROM_LOAD16_BYTE( "25.10e",           0x080000, 0x20000, CRC(524f5c55) SHA1(67adfd36bb325c4a6d3e1e5b668c4dc983ac0a62) )
	ROM_LOAD16_BYTE( "29.10f",           0x080001, 0x20000, CRC(f06a12f2) SHA1(ddc431ce01392d4a7562760743abd9ea73b06cf3) )
	ROM_LOAD16_BYTE( "26.11e",           0x0c0000, 0x20000, CRC(8312d055) SHA1(e36c23c197096d50d71ceefbf93205d5758153e0) )
	ROM_LOAD16_BYTE( "30.11f",           0x0c0001, 0x20000, CRC(d0580ff2) SHA1(1b2e4c4abbe90a68283c86e7cb5328b242be5683) )
	ROM_LOAD16_WORD_SWAP( "s92_21a.5f",  0x100000, 0x80000, CRC(925a7877) SHA1(1960dca35f0ca6f2b399a9fccfbc0132ac6425d1) )

	ROM_REGION( 0x40000, "pal", 0 )	
	ROM_LOAD( "bruteforce.palce16v8h-25.11d",    0x00000, 0x40000, CRC(430f722d) SHA1(b0c0570057c782b1114819fae907f45a01c55065) )

	ROM_REGION( 0x600000, "gfx", 0 )
	ROMX_LOAD( "s92_01.bin",   0x000000, 0x80000, CRC(03b0d852) SHA1(f370f25c96ad2b94f8c53d6b7139100285a25bef) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "s92_02.bin",   0x000002, 0x80000, CRC(840289ec) SHA1(2fb42a242f60ba7e74009b5a90eb26e035ba1e82) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "s92_03.bin",   0x000004, 0x80000, CRC(cdb5f027) SHA1(4c7d944fef200fdfcaf57758b901b5511188ed2e) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "s92_04.bin",   0x000006, 0x80000, CRC(e2799472) SHA1(27d3796429338d82a8de246a0ea06dd487a87768) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "s92_05.bin",   0x200000, 0x80000, CRC(ba8a2761) SHA1(4b696d66c51611e43522bed752654314e76d33b6) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "s92_06.bin",   0x200002, 0x80000, CRC(e584bfb5) SHA1(ebdf1f5e2638eed3a65dda82b1ed9151a355f4c9) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "s92_07.bin",   0x200004, 0x80000, CRC(21e3f87d) SHA1(4a4961bb68c3a1ce15f9d393d9c03ecb2466cc29) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "s92_08.bin",   0x200006, 0x80000, CRC(befc47df) SHA1(520390420da3a0271ba90b0a933e65143265e5cf) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "s92_10.bin",   0x400000, 0x80000, CRC(960687d5) SHA1(2868c31121b1c7564e9767b9a19cdbf655c7ed1d) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "s92_11.bin",   0x400002, 0x80000, CRC(978ecd18) SHA1(648a59706b93c84b4206a968ecbdc3e834c476f6) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "s92_12.bin",   0x400004, 0x80000, CRC(d6ec9a0a) SHA1(ed6143f8737013b6ef1684e37c05e037e7a80dae) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "s92_13.bin",   0x400006, 0x80000, CRC(ed2c67f6) SHA1(0083c0ffaf6fe7659ff0cf822be4346cd6e61329) , ROM_GROUPWORD | ROM_SKIP(6) )

	ROM_REGION( 0x18000, "audiocpu", 0 ) /* 64k for the audio CPU (+banks) */
	ROM_LOAD( "s92_09.bin",    0x00000, 0x08000, CRC(08f6b60e) SHA1(8258fcaca4ac419312531eec67079b97f471179c) )
	ROM_CONTINUE(              0x10000, 0x08000 )

	ROM_REGION( 0x40000, "oki", 0 ) /* Samples */
	ROM_LOAD( "s92_18.bin",    0x00000, 0x20000, CRC(7f162009) SHA1(346bf42992b4c36c593e21901e22c87ae4a7d86d) )
	ROM_LOAD( "s92_19.bin",    0x20000, 0x20000, CRC(beade53f) SHA1(277c397dc12752719ec6b47d2224750bd1c07f79) )
ROM_END



ROM_START( sf2amf )
	/* the PCB is not working on real hardware */
	ROM_REGION( CODE_SIZE, "maincpu", 0 )      /* 68000 code */
	ROM_LOAD16_BYTE( "5.amf",          0x000000, 0x80000, CRC(03991fba) SHA1(6c42bf15248640fdb3e98fb01b0a870649deb410) )
	ROM_LOAD16_BYTE( "4.amf",          0x000001, 0x80000, CRC(39f15a1e) SHA1(901c4fea76bf5bff7330ed07ffde54cdccdaa680) )
	/* missing last part(s) of program roms, some gfx loading instructions are missing */
	ROM_LOAD16_BYTE( "prg part 3.amf", 0x100000, 0x40000, NO_DUMP )
	ROM_LOAD16_BYTE( "prg part 4.amf", 0x100001, 0x40000, NO_DUMP )
	/* do not move this outside comments, this is only for testing purpose
	ROM_LOAD16_BYTE( "u221.rom",   0x100000, 0x20000, CRC(64e6e091) SHA1(32ec05db955e538d4ada26d19ee50926f74b684f) )
	ROM_LOAD16_BYTE( "u195.rom",   0x100001, 0x20000, CRC(c95e4443) SHA1(28417dee9ccdfa65b0f4a92aa29b90279fe8cd85) )
	*/

	ROM_REGION( 0x600000, "gfx", 0 )
	ROMX_LOAD( "y.c.e.c m.k.r-001", 0x000000, 0x80000, CRC(a258de13) SHA1(2e477948c4c8a2fb7cfdc4a739766bc4a4e01c49), ROM_GROUPWORD | ROM_SKIP(6) )
	ROM_CONTINUE(                   0x000004, 0x80000)
	ROMX_LOAD( "y.c.e.c m.k.r-003", 0x000002, 0x80000, CRC(c781bf87) SHA1(034baa9807c2ce8dc800200963a38cd9262b21fb), ROM_GROUPWORD | ROM_SKIP(6) )
	ROM_CONTINUE(                   0x000006, 0x80000)
	ROMX_LOAD( "y.c.e.c m.k.r-002", 0x200000, 0x80000, CRC(5726cab8) SHA1(0b2243a9a7184d53d42ddab7a8c51b63001c2f56), ROM_GROUPWORD | ROM_SKIP(6) )
	ROM_CONTINUE(                   0x200004, 0x80000)
	ROMX_LOAD( "y.c.e.c d.w.c-011", 0x200002, 0x80000, CRC(bc90c12f) SHA1(ecdb776239b22bd56b7c3a87c9e561f650a4dfea), ROM_GROUPWORD | ROM_SKIP(6) )
	ROM_CONTINUE(                   0x200006, 0x80000)
	ROMX_LOAD( "y.c.e.c d.w.c-012", 0x400000, 0x80000, CRC(187667cc) SHA1(fae65bf23f49a32903fda8080659ccf8d42b911f), ROM_GROUPWORD | ROM_SKIP(6) )
	ROM_CONTINUE(                   0x400004, 0x80000)
	ROMX_LOAD( "y.c.e.c d.w.c-013", 0x400002, 0x80000, CRC(5b585071) SHA1(ad3371b1ba0441c67d9fcbb23b09464710e4e28a), ROM_GROUPWORD | ROM_SKIP(6) )
	ROM_CONTINUE(                   0x400006, 0x80000)
	/* extra gfx layer roms loaded over the former ones to remove the capcom copyright logo */
	ROMX_LOAD( "6.amf",             0x400000, 0x10000, CRC(3a85a275) SHA1(01907d69f912abffe3ad9745638ce3f282cfb2e8), ROM_SKIP(7) )
	ROM_CONTINUE(                   0x400004, 0x10000 )
	ROMX_LOAD( "9.amf",             0x400002, 0x10000, CRC(9156472f) SHA1(5db2acfc54308d4d26e0459f9486620a968c81d8), ROM_SKIP(7) )
	ROM_CONTINUE(                   0x400006, 0x10000 )
	ROMX_LOAD( "8.amf",             0x400001, 0x10000, CRC(ecdb083b) SHA1(899894c1db004e98f755ffbdf28d32296b9c0a86), ROM_SKIP(7) )
	ROM_CONTINUE(                   0x400005, 0x10000 )
	ROMX_LOAD( "10.amf",            0x400003, 0x10000, CRC(8fea8384) SHA1(8b31fd8d16cbafb5144f772653336b41db8f64fc), ROM_SKIP(7) )
	ROM_CONTINUE(                   0x400007, 0x10000 )
	/* end of extra gfx layer roms */

	ROM_REGION( 0x18000, "audiocpu", 0 ) /* 64k for the audio CPU (+banks) */
	ROM_LOAD( "3.amf", 0x00000, 0x08000, CRC(a4823a1b) SHA1(7b6bf59dfd578bfbbdb64c27988796783442d659) )
	ROM_CONTINUE(      0x10000, 0x08000 )

	ROM_REGION( 0x20000, "user1", 0 ) /* unknown (bootleg priority?) */
	ROM_LOAD( "7.amf", 0x00000, 0x10000, CRC(13ea1c44) SHA1(5b05fe4c3920e33d94fac5f59e09ff14b3e427fe) )

	ROM_REGION( 0x40000, "oki", 0 ) /* Samples */
	ROM_LOAD( "2.amf", 0x00000, 0x20000, CRC(7f162009) SHA1(346bf42992b4c36c593e21901e22c87ae4a7d86d) )
	ROM_LOAD( "1.amf", 0x20000, 0x20000, CRC(beade53f) SHA1(277c397dc12752719ec6b47d2224750bd1c07f79) )
ROM_END

ROM_START( sf2amf2 )
	ROM_REGION( CODE_SIZE, "maincpu", 0 )      /* 68000 code */
	ROM_LOAD16_BYTE( "m5m27c401.u222",          0x000000, 0x80000, CRC(03991fba) SHA1(6c42bf15248640fdb3e98fb01b0a870649deb410) ) // ==  5.amf                 sf2amf
	ROM_LOAD16_BYTE( "m5m27c401.u196",          0x000001, 0x80000, CRC(39f15a1e) SHA1(901c4fea76bf5bff7330ed07ffde54cdccdaa680) ) // ==  4.amf                 sf2amf

	ROM_LOAD16_BYTE( "27020.u221",   0x100000, 0x40000, CRC(aa4d55a6) SHA1(8fd1c21816886a7734aae42e9336d5f66ddab7bc) ) // different
	ROM_LOAD16_BYTE( "27020.u195",   0x100001, 0x40000, CRC(2bffa6f9) SHA1(eb1222356d89849edb08ea1898399cf90cf127f5) ) // different


	ROM_REGION( 0x600000, "gfx", 0 )
	ROMX_LOAD( "fun-u70.bin", 0x000004, 0x80000, CRC(a94a8b19) SHA1(49ba9e6032a0b33d7db9fe609710575f2f75e695), ROM_GROUPWORD | ROM_SKIP(6) )  // different
	ROM_CONTINUE(             0x000000, 0x80000)
	ROMX_LOAD( "fun-u68.bin", 0x000006, 0x80000, CRC(0405f21f) SHA1(dbebd2c2c46d5aae8db905f2eb51abd4a5c4ea97), ROM_GROUPWORD | ROM_SKIP(6) )  // different
	ROM_CONTINUE(             0x000002, 0x80000)
	ROMX_LOAD( "fun-u69.bin", 0x200004, 0x80000, CRC(05dc2043) SHA1(d16b89a48d2dd7cdfafc79567ce1e230d4bd41c1), ROM_GROUPWORD | ROM_SKIP(6) )  // different
	ROM_CONTINUE(             0x200000, 0x80000)
	ROMX_LOAD( "fun-u67.bin", 0x200006, 0x80000, CRC(055b64f1) SHA1(3dd68f52b81ed1b300b65c900ef6bfe435d41e4b), ROM_GROUPWORD | ROM_SKIP(6) )  // different
	ROM_CONTINUE(             0x200002, 0x80000)
	ROMX_LOAD( "fun-u19.bin", 0x400004, 0x80000, CRC(1a518609) SHA1(18ffca70d6cefb399ba6e3008e5c29dc37de52a0), ROM_GROUPWORD | ROM_SKIP(6) )  // different
	ROM_CONTINUE(             0x400000, 0x80000)
	ROMX_LOAD( "fun-u18.bin", 0x400006, 0x80000, CRC(84f9354f) SHA1(ecc190950b1f45b268da380c17859a8d0715b58f), ROM_GROUPWORD | ROM_SKIP(6) )  // different
	ROM_CONTINUE(             0x400002, 0x80000)
	/* extra gfx layer roms loaded over the former ones to remove the capcom copyright logo */
	ROMX_LOAD( "grp1.u31",    0x400004, 0x10000, CRC(6de44671) SHA1(dc6abba639e0c27033e391c7438d88dc89a93351), ROM_SKIP(7) )  // different
	ROM_CONTINUE(             0x400000, 0x10000 )
	ROMX_LOAD( "grp3.u29",    0x400006, 0x10000, CRC(e8f14362) SHA1(a20eb75e322011e2a8d8bf2acebe713bef3d3941), ROM_SKIP(7) )  // different
	ROM_CONTINUE(             0x400002, 0x10000 )
	ROMX_LOAD( "grp2.u30",    0x400005, 0x10000, CRC(bf0cd819) SHA1(f04a098fce07949277268327871c5e5520e3bb3c), ROM_SKIP(7) )  // different
	ROM_CONTINUE(             0x400001, 0x10000 )
	ROMX_LOAD( "grp4.u28",    0x400007, 0x10000, CRC(76f9f91f) SHA1(58a34062d2c8378558a7f1629140330279af9a43), ROM_SKIP(7) )  // different
	ROM_CONTINUE(             0x400003, 0x10000 )

	ROM_REGION( 0x18000, "audiocpu", 0 ) /* 64k for the audio CPU (+banks) */
	ROM_LOAD( "27512.u191", 0x00000, 0x08000, CRC(a4823a1b) SHA1(7b6bf59dfd578bfbbdb64c27988796783442d659) )
	ROM_CONTINUE(      0x10000, 0x08000 )

	ROM_REGION( 0x20000, "user1", 0 ) /* unknown (bootleg priority?) */
	ROM_LOAD( "27512.u133", 0x00000, 0x10000, CRC(13ea1c44) SHA1(5b05fe4c3920e33d94fac5f59e09ff14b3e427fe) )

	ROM_REGION( 0x40000, "oki", 0 ) /* Samples */
	ROM_LOAD( "fun-u210.bin", 0x00000, 0x40000, CRC(6cfffb11) SHA1(995526183ffd35f92e9096500a3fe6237faaa2dd) )
ROM_END

ROM_START( sf2dkot2 )
	ROM_REGION( CODE_SIZE, "maincpu", 0 )      /* 68000 code */
	/* The game reads values from 0x201201 and 0x281201 (at PC=E5452) and uses their difference to form a jump offset. */
	/* This arrangement seems to work */
	ROM_LOAD16_WORD_SWAP( "turboii.23", 0x000000, 0x80000, CRC(9bbfe420) SHA1(299ab5625f2d4ebc89bcf19a2b2d88edc8054747) )
	ROM_RELOAD(                         0x280000, 0x80000 )
	ROM_LOAD16_WORD_SWAP( "turboii.22", 0x080000, 0x80000, CRC(3e57ba19) SHA1(ae8170ac890c14ba7746e29c0f21dfb38c4c9837) )
	ROM_RELOAD(                         0x200000, 0x80000 )

	ROM_LOAD16_WORD_SWAP( "turboii.21", 0x100000, 0x80000, CRC(ed4186bd) SHA1(f3dfe91d8f4384275190b0d86488843c1161d86f) )

	ROM_REGION( 0x600000, "gfx", 0 )
	ROMX_LOAD( "s92_01.bin",   0x000000, 0x80000, CRC(03b0d852) SHA1(f370f25c96ad2b94f8c53d6b7139100285a25bef) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "s92_02.bin",   0x000002, 0x80000, CRC(840289ec) SHA1(2fb42a242f60ba7e74009b5a90eb26e035ba1e82) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "s92_03.bin",   0x000004, 0x80000, CRC(cdb5f027) SHA1(4c7d944fef200fdfcaf57758b901b5511188ed2e) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "s92_04.bin",   0x000006, 0x80000, CRC(e2799472) SHA1(27d3796429338d82a8de246a0ea06dd487a87768) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "s92_05.bin",   0x200000, 0x80000, CRC(ba8a2761) SHA1(4b696d66c51611e43522bed752654314e76d33b6) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "s92_06.bin",   0x200002, 0x80000, CRC(e584bfb5) SHA1(ebdf1f5e2638eed3a65dda82b1ed9151a355f4c9) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "s92_07.bin",   0x200004, 0x80000, CRC(21e3f87d) SHA1(4a4961bb68c3a1ce15f9d393d9c03ecb2466cc29) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "s92_08.bin",   0x200006, 0x80000, CRC(befc47df) SHA1(520390420da3a0271ba90b0a933e65143265e5cf) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "s92_10.bin",   0x400000, 0x80000, CRC(960687d5) SHA1(2868c31121b1c7564e9767b9a19cdbf655c7ed1d) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "s92_11.bin",   0x400002, 0x80000, CRC(978ecd18) SHA1(648a59706b93c84b4206a968ecbdc3e834c476f6) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "s92_12.bin",   0x400004, 0x80000, CRC(d6ec9a0a) SHA1(ed6143f8737013b6ef1684e37c05e037e7a80dae) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "s92_13.bin",   0x400006, 0x80000, CRC(ed2c67f6) SHA1(0083c0ffaf6fe7659ff0cf822be4346cd6e61329) , ROM_GROUPWORD | ROM_SKIP(6) )

	ROM_REGION( 0x18000, "audiocpu", 0 ) /* 64k for the audio CPU (+banks) */
	ROM_LOAD( "s92_09.bin",    0x00000, 0x08000, CRC(08f6b60e) SHA1(8258fcaca4ac419312531eec67079b97f471179c) )
	ROM_CONTINUE(              0x10000, 0x08000 )

	ROM_REGION( 0x40000, "oki", 0 ) /* Samples */
	ROM_LOAD( "s92_18.bin",    0x00000, 0x20000, CRC(7f162009) SHA1(346bf42992b4c36c593e21901e22c87ae4a7d86d) )
	ROM_LOAD( "s92_19.bin",    0x20000, 0x20000, CRC(beade53f) SHA1(277c397dc12752719ec6b47d2224750bd1c07f79) )
ROM_END

ROM_START( sf2m2 )
	ROM_REGION( CODE_SIZE, "maincpu", 0 )      /* 68000 code */
	ROM_LOAD16_BYTE( "ch222esp",          0x000000, 0x80000, CRC(9e6d058a) SHA1(8c9adca7b65dc929c325c0a62304d24dc0902c08) )
	ROM_LOAD16_BYTE( "ch196esp",          0x000001, 0x80000, CRC(ed2ff437) SHA1(e76fc2953b6c800d5955c8fb442b80142e40e375) )
	ROM_LOAD16_WORD_SWAP( "s92_21a.bin",  0x100000, 0x80000, CRC(925a7877) SHA1(1960dca35f0ca6f2b399a9fccfbc0132ac6425d1) )

	ROM_REGION( 0x600000, "gfx", 0 )
	ROMX_LOAD( "s92_01.bin",   0x000000, 0x80000, CRC(03b0d852) SHA1(f370f25c96ad2b94f8c53d6b7139100285a25bef) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "s92_02.bin",   0x000002, 0x80000, CRC(840289ec) SHA1(2fb42a242f60ba7e74009b5a90eb26e035ba1e82) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "s92_03.bin",   0x000004, 0x80000, CRC(cdb5f027) SHA1(4c7d944fef200fdfcaf57758b901b5511188ed2e) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "s92_04.bin",   0x000006, 0x80000, CRC(e2799472) SHA1(27d3796429338d82a8de246a0ea06dd487a87768) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "s92_05.bin",   0x200000, 0x80000, CRC(ba8a2761) SHA1(4b696d66c51611e43522bed752654314e76d33b6) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "s92_06.bin",   0x200002, 0x80000, CRC(e584bfb5) SHA1(ebdf1f5e2638eed3a65dda82b1ed9151a355f4c9) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "s92_07.bin",   0x200004, 0x80000, CRC(21e3f87d) SHA1(4a4961bb68c3a1ce15f9d393d9c03ecb2466cc29) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "s92_08.bin",   0x200006, 0x80000, CRC(befc47df) SHA1(520390420da3a0271ba90b0a933e65143265e5cf) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "s92_10.bin",   0x400000, 0x80000, CRC(960687d5) SHA1(2868c31121b1c7564e9767b9a19cdbf655c7ed1d) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "s92_11.bin",   0x400002, 0x80000, CRC(978ecd18) SHA1(648a59706b93c84b4206a968ecbdc3e834c476f6) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "s92_12.bin",   0x400004, 0x80000, CRC(d6ec9a0a) SHA1(ed6143f8737013b6ef1684e37c05e037e7a80dae) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "s92_13.bin",   0x400006, 0x80000, CRC(ed2c67f6) SHA1(0083c0ffaf6fe7659ff0cf822be4346cd6e61329) , ROM_GROUPWORD | ROM_SKIP(6) )

	ROM_REGION( 0x18000, "audiocpu", 0 ) /* 64k for the audio CPU (+banks) */
	ROM_LOAD( "s92_09.bin",    0x00000, 0x08000, CRC(08f6b60e) SHA1(8258fcaca4ac419312531eec67079b97f471179c) )
	ROM_CONTINUE(              0x10000, 0x08000 )

	ROM_REGION( 0x40000, "oki", 0 ) /* Samples */
	ROM_LOAD( "s92_18.bin",    0x00000, 0x20000, CRC(7f162009) SHA1(346bf42992b4c36c593e21901e22c87ae4a7d86d) )
	ROM_LOAD( "s92_19.bin",    0x20000, 0x20000, CRC(beade53f) SHA1(277c397dc12752719ec6b47d2224750bd1c07f79) )
ROM_END

ROM_START( sf2m3 )
	ROM_REGION( CODE_SIZE, "maincpu", 0 )      /* 68000 code */
	ROM_LOAD16_BYTE( "u222chp",           0x000000, 0x80000, CRC(db567b66) SHA1(315bfbf2786ef67a95afb87de836ab348523dbbe) )
	ROM_LOAD16_BYTE( "u196chp",           0x000001, 0x80000, CRC(95ea597e) SHA1(5eb82feaa1de5611a96888e4670744bbb7d90393) )
	ROM_LOAD16_WORD_SWAP( "s92_21a.bin",  0x100000, 0x80000, CRC(925a7877) SHA1(1960dca35f0ca6f2b399a9fccfbc0132ac6425d1) )

	ROM_REGION( 0x600000, "gfx", 0 )
	ROMX_LOAD( "s92_01.bin",   0x000000, 0x80000, CRC(03b0d852) SHA1(f370f25c96ad2b94f8c53d6b7139100285a25bef) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "s92_02.bin",   0x000002, 0x80000, CRC(840289ec) SHA1(2fb42a242f60ba7e74009b5a90eb26e035ba1e82) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "s92_03.bin",   0x000004, 0x80000, CRC(cdb5f027) SHA1(4c7d944fef200fdfcaf57758b901b5511188ed2e) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "s92_04.bin",   0x000006, 0x80000, CRC(e2799472) SHA1(27d3796429338d82a8de246a0ea06dd487a87768) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "s92_05.bin",   0x200000, 0x80000, CRC(ba8a2761) SHA1(4b696d66c51611e43522bed752654314e76d33b6) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "s92_06.bin",   0x200002, 0x80000, CRC(e584bfb5) SHA1(ebdf1f5e2638eed3a65dda82b1ed9151a355f4c9) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "s92_07.bin",   0x200004, 0x80000, CRC(21e3f87d) SHA1(4a4961bb68c3a1ce15f9d393d9c03ecb2466cc29) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "s92_08.bin",   0x200006, 0x80000, CRC(befc47df) SHA1(520390420da3a0271ba90b0a933e65143265e5cf) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "s92_10.bin",   0x400000, 0x80000, CRC(960687d5) SHA1(2868c31121b1c7564e9767b9a19cdbf655c7ed1d) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "s92_11.bin",   0x400002, 0x80000, CRC(978ecd18) SHA1(648a59706b93c84b4206a968ecbdc3e834c476f6) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "s92_12.bin",   0x400004, 0x80000, CRC(d6ec9a0a) SHA1(ed6143f8737013b6ef1684e37c05e037e7a80dae) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "s92_13.bin",   0x400006, 0x80000, CRC(ed2c67f6) SHA1(0083c0ffaf6fe7659ff0cf822be4346cd6e61329) , ROM_GROUPWORD | ROM_SKIP(6) )

	ROM_REGION( 0x18000, "audiocpu", 0 ) /* 64k for the audio CPU (+banks) */
	ROM_LOAD( "s92_09.bin",    0x00000, 0x08000, CRC(08f6b60e) SHA1(8258fcaca4ac419312531eec67079b97f471179c) )
	ROM_CONTINUE(              0x10000, 0x08000 )

	ROM_REGION( 0x40000, "oki", 0 ) /* Samples */
	ROM_LOAD( "s92_18.bin",    0x00000, 0x20000, CRC(7f162009) SHA1(346bf42992b4c36c593e21901e22c87ae4a7d86d) )
	ROM_LOAD( "s92_19.bin",    0x20000, 0x20000, CRC(beade53f) SHA1(277c397dc12752719ec6b47d2224750bd1c07f79) )
ROM_END

ROM_START( sf2m4 )
	ROM_REGION( CODE_SIZE, "maincpu", 0 )      /* 68000 code */
	ROM_LOAD16_BYTE( "u222ne",            0x000000, 0x80000, CRC(7133489e) SHA1(036ef100c64c6e912c911340b32eea0da0b6f6d9) )
	ROM_LOAD16_BYTE( "u196ne",            0x000001, 0x80000, CRC(b07a4f90) SHA1(7a4a800bddc43cfa60f9097723b44a05c9d290ae) )
	ROM_LOAD16_WORD_SWAP( "s92_21a.bin",  0x100000, 0x80000, CRC(925a7877) SHA1(1960dca35f0ca6f2b399a9fccfbc0132ac6425d1) )

	ROM_REGION( 0x600000, "gfx", 0 )
	ROMX_LOAD( "s92_01.bin",   0x000000, 0x80000, CRC(03b0d852) SHA1(f370f25c96ad2b94f8c53d6b7139100285a25bef) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "s92_02.bin",   0x000002, 0x80000, CRC(840289ec) SHA1(2fb42a242f60ba7e74009b5a90eb26e035ba1e82) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "s92_03.bin",   0x000004, 0x80000, CRC(cdb5f027) SHA1(4c7d944fef200fdfcaf57758b901b5511188ed2e) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "s92_04.bin",   0x000006, 0x80000, CRC(e2799472) SHA1(27d3796429338d82a8de246a0ea06dd487a87768) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "s92_05.bin",   0x200000, 0x80000, CRC(ba8a2761) SHA1(4b696d66c51611e43522bed752654314e76d33b6) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "s92_06.bin",   0x200002, 0x80000, CRC(e584bfb5) SHA1(ebdf1f5e2638eed3a65dda82b1ed9151a355f4c9) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "s92_07.bin",   0x200004, 0x80000, CRC(21e3f87d) SHA1(4a4961bb68c3a1ce15f9d393d9c03ecb2466cc29) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "s92_08.bin",   0x200006, 0x80000, CRC(befc47df) SHA1(520390420da3a0271ba90b0a933e65143265e5cf) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "s92_10.bin",   0x400000, 0x80000, CRC(960687d5) SHA1(2868c31121b1c7564e9767b9a19cdbf655c7ed1d) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "s92_11.bin",   0x400002, 0x80000, CRC(978ecd18) SHA1(648a59706b93c84b4206a968ecbdc3e834c476f6) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "s92_12.bin",   0x400004, 0x80000, CRC(d6ec9a0a) SHA1(ed6143f8737013b6ef1684e37c05e037e7a80dae) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "s92_13.bin",   0x400006, 0x80000, CRC(ed2c67f6) SHA1(0083c0ffaf6fe7659ff0cf822be4346cd6e61329) , ROM_GROUPWORD | ROM_SKIP(6) )

	ROM_REGION( 0x18000, "audiocpu", 0 ) /* 64k for the audio CPU (+banks) */
	ROM_LOAD( "s92_09.bin",    0x00000, 0x08000, CRC(08f6b60e) SHA1(8258fcaca4ac419312531eec67079b97f471179c) )
	ROM_CONTINUE(              0x10000, 0x08000 )

	ROM_REGION( 0x40000, "oki", 0 ) /* Samples */
	ROM_LOAD( "s92_18.bin",    0x00000, 0x20000, CRC(7f162009) SHA1(346bf42992b4c36c593e21901e22c87ae4a7d86d) )
	ROM_LOAD( "s92_19.bin",    0x20000, 0x20000, CRC(beade53f) SHA1(277c397dc12752719ec6b47d2224750bd1c07f79) )
ROM_END

ROM_START( sf2m5 )
	ROM_REGION( CODE_SIZE, "maincpu", 0 )      /* 68000 code */
	ROM_LOAD16_BYTE( "u222",              0x000000, 0x80000, CRC(03991fba) SHA1(6c42bf15248640fdb3e98fb01b0a870649deb410) )
	ROM_LOAD16_BYTE( "u196",              0x000001, 0x80000, CRC(39f15a1e) SHA1(901c4fea76bf5bff7330ed07ffde54cdccdaa680) )
	ROM_LOAD16_WORD_SWAP( "s92_21a.bin",  0x100000, 0x80000, CRC(925a7877) SHA1(1960dca35f0ca6f2b399a9fccfbc0132ac6425d1) )

	ROM_REGION( 0x600000, "gfx", 0 )
	ROMX_LOAD( "s92_01.bin",   0x000000, 0x80000, CRC(03b0d852) SHA1(f370f25c96ad2b94f8c53d6b7139100285a25bef) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "s92_02.bin",   0x000002, 0x80000, CRC(840289ec) SHA1(2fb42a242f60ba7e74009b5a90eb26e035ba1e82) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "s92_03.bin",   0x000004, 0x80000, CRC(cdb5f027) SHA1(4c7d944fef200fdfcaf57758b901b5511188ed2e) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "s92_04.bin",   0x000006, 0x80000, CRC(e2799472) SHA1(27d3796429338d82a8de246a0ea06dd487a87768) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "s92_05.bin",   0x200000, 0x80000, CRC(ba8a2761) SHA1(4b696d66c51611e43522bed752654314e76d33b6) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "s92_06.bin",   0x200002, 0x80000, CRC(e584bfb5) SHA1(ebdf1f5e2638eed3a65dda82b1ed9151a355f4c9) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "s92_07.bin",   0x200004, 0x80000, CRC(21e3f87d) SHA1(4a4961bb68c3a1ce15f9d393d9c03ecb2466cc29) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "s92_08.bin",   0x200006, 0x80000, CRC(befc47df) SHA1(520390420da3a0271ba90b0a933e65143265e5cf) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "s92_10.bin",   0x400000, 0x80000, CRC(960687d5) SHA1(2868c31121b1c7564e9767b9a19cdbf655c7ed1d) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "s92_11.bin",   0x400002, 0x80000, CRC(978ecd18) SHA1(648a59706b93c84b4206a968ecbdc3e834c476f6) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "s92_12.bin",   0x400004, 0x80000, CRC(d6ec9a0a) SHA1(ed6143f8737013b6ef1684e37c05e037e7a80dae) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "s92_13.bin",   0x400006, 0x80000, CRC(ed2c67f6) SHA1(0083c0ffaf6fe7659ff0cf822be4346cd6e61329) , ROM_GROUPWORD | ROM_SKIP(6) )

	ROM_REGION( 0x18000, "audiocpu", 0 ) /* 64k for the audio CPU (+banks) */
	ROM_LOAD( "s92_09.bin",    0x00000, 0x08000, CRC(08f6b60e) SHA1(8258fcaca4ac419312531eec67079b97f471179c) )
	ROM_CONTINUE(              0x10000, 0x08000 )

	ROM_REGION( 0x40000, "oki", 0 ) /* Samples */
	ROM_LOAD( "s92_18.bin",    0x00000, 0x20000, CRC(7f162009) SHA1(346bf42992b4c36c593e21901e22c87ae4a7d86d) )
	ROM_LOAD( "s92_19.bin",    0x20000, 0x20000, CRC(beade53f) SHA1(277c397dc12752719ec6b47d2224750bd1c07f79) )
ROM_END

ROM_START( sf2m6 )
	ROM_REGION( CODE_SIZE, "maincpu", 0 )      /* 68000 code */
	ROM_LOAD16_BYTE( "u222-6b",           0x000000, 0x80000, CRC(0a3692be) SHA1(7b937b7b0130e460b5f12188b19f464c55b507c9) )
	ROM_LOAD16_BYTE( "u196-6b",           0x000001, 0x80000, CRC(80454da7) SHA1(64f6dba14d342c9933ce632aa7ca126b34b4ee8b) )
	ROM_LOAD16_WORD_SWAP( "s92_21a.bin",  0x100000, 0x80000, CRC(925a7877) SHA1(1960dca35f0ca6f2b399a9fccfbc0132ac6425d1) )

	ROM_REGION( 0x600000, "gfx", 0 )
	ROMX_LOAD( "s92_01.bin",   0x000000, 0x80000, CRC(03b0d852) SHA1(f370f25c96ad2b94f8c53d6b7139100285a25bef) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "s92_02.bin",   0x000002, 0x80000, CRC(840289ec) SHA1(2fb42a242f60ba7e74009b5a90eb26e035ba1e82) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "s92_03.bin",   0x000004, 0x80000, CRC(cdb5f027) SHA1(4c7d944fef200fdfcaf57758b901b5511188ed2e) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "s92_04.bin",   0x000006, 0x80000, CRC(e2799472) SHA1(27d3796429338d82a8de246a0ea06dd487a87768) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "s92_05.bin",   0x200000, 0x80000, CRC(ba8a2761) SHA1(4b696d66c51611e43522bed752654314e76d33b6) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "s92_06.bin",   0x200002, 0x80000, CRC(e584bfb5) SHA1(ebdf1f5e2638eed3a65dda82b1ed9151a355f4c9) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "s92_07.bin",   0x200004, 0x80000, CRC(21e3f87d) SHA1(4a4961bb68c3a1ce15f9d393d9c03ecb2466cc29) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "s92_08.bin",   0x200006, 0x80000, CRC(befc47df) SHA1(520390420da3a0271ba90b0a933e65143265e5cf) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "s92_10.bin",   0x400000, 0x80000, CRC(960687d5) SHA1(2868c31121b1c7564e9767b9a19cdbf655c7ed1d) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "s92_11.bin",   0x400002, 0x80000, CRC(978ecd18) SHA1(648a59706b93c84b4206a968ecbdc3e834c476f6) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "s92_12.bin",   0x400004, 0x80000, CRC(d6ec9a0a) SHA1(ed6143f8737013b6ef1684e37c05e037e7a80dae) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "s92_13.bin",   0x400006, 0x80000, CRC(ed2c67f6) SHA1(0083c0ffaf6fe7659ff0cf822be4346cd6e61329) , ROM_GROUPWORD | ROM_SKIP(6) )

	ROM_REGION( 0x18000, "audiocpu", 0 ) /* 64k for the audio CPU (+banks) */
	ROM_LOAD( "s92_09.bin",    0x00000, 0x08000, CRC(08f6b60e) SHA1(8258fcaca4ac419312531eec67079b97f471179c) )
	ROM_CONTINUE(              0x10000, 0x08000 )

	ROM_REGION( 0x40000, "oki", 0 ) /* Samples */
	ROM_LOAD( "s92_18.bin",    0x00000, 0x20000, CRC(7f162009) SHA1(346bf42992b4c36c593e21901e22c87ae4a7d86d) )
	ROM_LOAD( "s92_19.bin",    0x20000, 0x20000, CRC(beade53f) SHA1(277c397dc12752719ec6b47d2224750bd1c07f79) )
ROM_END

ROM_START( sf2m7 )
	ROM_REGION( CODE_SIZE, "maincpu", 0 )      /* 68000 code */
	ROM_LOAD16_BYTE( "u222-2i",           0x000000, 0x40000, CRC(1ca7adbd) SHA1(45e9dc05766ad156edcfc9e59a9804f74f90dc68) )
	ROM_LOAD16_BYTE( "u196-2i",           0x000001, 0x40000, CRC(f758408c) SHA1(aac44a7287bb3b7ba35d68aff279e265dbd3f6d3) )
	ROM_LOAD16_BYTE( "u222-2s",           0x080000, 0x40000, CRC(720cea3e) SHA1(ec4f22159d44a8abc40643b986b88a4f947d6aea) )
	ROM_LOAD16_BYTE( "u196-2s",           0x080001, 0x40000, CRC(9932832c) SHA1(0da0f5ebab91b0759c5fc00902cfe4b12a856466) )
	ROM_LOAD16_WORD_SWAP( "s92_21a.bin",  0x100000, 0x80000, CRC(925a7877) SHA1(1960dca35f0ca6f2b399a9fccfbc0132ac6425d1) )

	ROM_REGION( 0x600000, "gfx", 0 )
	ROMX_LOAD( "s92_01.bin",   0x000000, 0x80000, CRC(03b0d852) SHA1(f370f25c96ad2b94f8c53d6b7139100285a25bef) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "s92_02.bin",   0x000002, 0x80000, CRC(840289ec) SHA1(2fb42a242f60ba7e74009b5a90eb26e035ba1e82) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "s92_03.bin",   0x000004, 0x80000, CRC(cdb5f027) SHA1(4c7d944fef200fdfcaf57758b901b5511188ed2e) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "s92_04.bin",   0x000006, 0x80000, CRC(e2799472) SHA1(27d3796429338d82a8de246a0ea06dd487a87768) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "s92_05.bin",   0x200000, 0x80000, CRC(ba8a2761) SHA1(4b696d66c51611e43522bed752654314e76d33b6) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "s92_06.bin",   0x200002, 0x80000, CRC(e584bfb5) SHA1(ebdf1f5e2638eed3a65dda82b1ed9151a355f4c9) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "s92_07.bin",   0x200004, 0x80000, CRC(21e3f87d) SHA1(4a4961bb68c3a1ce15f9d393d9c03ecb2466cc29) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "s92_08.bin",   0x200006, 0x80000, CRC(befc47df) SHA1(520390420da3a0271ba90b0a933e65143265e5cf) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "s92_10.bin",   0x400000, 0x80000, CRC(960687d5) SHA1(2868c31121b1c7564e9767b9a19cdbf655c7ed1d) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "s92_11.bin",   0x400002, 0x80000, CRC(978ecd18) SHA1(648a59706b93c84b4206a968ecbdc3e834c476f6) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "s92_12.bin",   0x400004, 0x80000, CRC(d6ec9a0a) SHA1(ed6143f8737013b6ef1684e37c05e037e7a80dae) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "s92_13.bin",   0x400006, 0x80000, CRC(ed2c67f6) SHA1(0083c0ffaf6fe7659ff0cf822be4346cd6e61329) , ROM_GROUPWORD | ROM_SKIP(6) )

	ROM_REGION( 0x18000, "audiocpu", 0 ) /* 64k for the audio CPU (+banks) */
	ROM_LOAD( "s92_09.bin",    0x00000, 0x08000, CRC(08f6b60e) SHA1(8258fcaca4ac419312531eec67079b97f471179c) )
	ROM_CONTINUE(              0x10000, 0x08000 )

	ROM_REGION( 0x40000, "oki", 0 ) /* Samples */
	ROM_LOAD( "s92_18.bin",    0x00000, 0x20000, CRC(7f162009) SHA1(346bf42992b4c36c593e21901e22c87ae4a7d86d) )
	ROM_LOAD( "s92_19.bin",    0x20000, 0x20000, CRC(beade53f) SHA1(277c397dc12752719ec6b47d2224750bd1c07f79) )
ROM_END

ROM_START( sf2m8 )
/* unconfirmed if working on real hardware */
	/* this board has unsupported pals */
	ROM_REGION( CODE_SIZE, "maincpu", 0 )      /* 68000 code */
	ROM_LOAD16_BYTE( "yyc-2.2", 0x000000, 0x80000, CRC(db567b66) SHA1(315bfbf2786ef67a95afb87de836ab348523dbbe) )
	ROM_LOAD16_BYTE( "yyc-3.4", 0x000001, 0x80000, CRC(95ea597e) SHA1(5eb82feaa1de5611a96888e4670744bbb7d90393) )
	ROM_LOAD16_BYTE( "yyc-4.1", 0x100000, 0x20000, CRC(1073b7b6) SHA1(81ca1eab65ceac69520584bb23a684ccb9d92f89) )
	ROM_LOAD16_BYTE( "yyc-5.3", 0x100001, 0x20000, CRC(924c6ce2) SHA1(676a912652bd75da5087f0c7eae047b7681a993c) )

	ROM_REGION( 0x600000, "gfx", 0 )
	ROMX_LOAD( "yyc-a",    0x000000, 0x40000, CRC(8242621f) SHA1(9d566176462bd25f9a377104b0c78a962708bc2b), ROM_SKIP(7) )
	ROM_CONTINUE(          0x000004, 0x40000)
	ROM_CONTINUE(          0x200000, 0x40000)
	ROM_CONTINUE(          0x200004, 0x40000)
	ROMX_LOAD( "yyc-c",    0x000001, 0x40000, CRC(0793a960) SHA1(f351163dd1090f8cd8d4c77e2a29764fee627b13), ROM_SKIP(7) )
	ROM_CONTINUE(          0x000005, 0x40000)
	ROM_CONTINUE(          0x200001, 0x40000)
	ROM_CONTINUE(          0x200005, 0x40000)
	ROMX_LOAD( "yyc-b",    0x000002, 0x40000, CRC(b0159973) SHA1(2a236b9d98fa0acddd844aa94bc5118012a6fb2f), ROM_SKIP(7) )
	ROM_CONTINUE(          0x000006, 0x40000)
	ROM_CONTINUE(          0x200002, 0x40000)
	ROM_CONTINUE(          0x200006, 0x40000)
	ROMX_LOAD( "yyc-d",    0x000003, 0x40000, CRC(92a8b572) SHA1(cbad24e519f0152989764c054da914f55e2b118c), ROM_SKIP(7) )
	ROM_CONTINUE(          0x000007, 0x40000)
	ROM_CONTINUE(          0x200003, 0x40000)
	ROM_CONTINUE(          0x200007, 0x40000)
	/* extra gfx layer roms loaded over the former ones to remove the capcom copyright logo */
	ROMX_LOAD( "yyc-6.1",  0x400000, 0x10000, CRC(94778332) SHA1(c0b9a05c710b89864ee5df1a53b39de30c994e2d), ROM_SKIP(7) )
	ROM_CONTINUE(          0x400004, 0x10000 )
	ROMX_LOAD( "yyc-8.9",  0x400002, 0x10000, CRC(f95bc505) SHA1(385beb2f9f8a473d928d729d722372ae49f410e7), ROM_SKIP(7) )
	ROM_CONTINUE(          0x400006, 0x10000 )
	ROMX_LOAD( "yyc-7.10", 0x400001, 0x10000, CRC(d1e452d3) SHA1(794f7ebf6c46c2938a5477451cf05f2e0c7b9049), ROM_SKIP(7) )
	ROM_CONTINUE(          0x400005, 0x10000 )
	ROMX_LOAD( "yyc-9.8",  0x400003, 0x10000, CRC(155824a9) SHA1(74d7e86be22c11234f5d9d0b25fa709b59ef471b), ROM_SKIP(7) )
	ROM_CONTINUE(          0x400007, 0x10000 )
	/* end of extra gfx layer roms */

	ROM_REGION( 0x18000, "audiocpu", 0 ) /* 64k for the audio CPU (+banks) */
	ROM_LOAD( "a-15.5", 0x00000, 0x08000, CRC(6f07d2cb) SHA1(8ef1338d04c1a0b43e24303085105cfdced0bd5e) )
	ROM_CONTINUE(       0x10000, 0x08000 )

	ROM_REGION( 0x10000, "user1", 0 ) /* unknown (bootleg priority?) */
	ROM_LOAD( "c-27.7", 0x00000, 0x10000, CRC(13ea1c44) SHA1(5b05fe4c3920e33d94fac5f59e09ff14b3e427fe) )

	ROM_REGION( 0x200000, "user2", 0 )
	ROM_LOAD( "yyc-e",    0x000000, 0x100000, CRC(61138469) SHA1(dec3b3af6e3f4fedf51600ddf0515f61b2122493) )
	ROM_LOAD( "yyc-f",    0x100000, 0x100000, CRC(b800dcdb) SHA1(2ec3251b78159b15032d55a5ee5138f159e67190) )

	ROM_REGION( 0x40000, "oki", 0 ) /* Samples */
	ROM_LOAD( "b-16.6", 0x00000, 0x40000, CRC(6cfffb11) SHA1(995526183ffd35f92e9096500a3fe6237faaa2dd) )
ROM_END

/* SF2M10
 From HBMAME (SF2H14)
 Same as sf2ebbl2, except it has program roms unique to sf2h14.

 Using sf2ce gfx roms to get clean gfx (the board was a bodgy
 half-done conversion attempt).

** PCB DETAILS ***
Street Fighter II Champion Edition (bootleg)

PCB Layout
----------

|-----------------------------------------------------|
|LM324 ROM1  3.579545MHz                   ROM10 ROM12|
|LM324 ROM2  16MHz 2018                    ROM9  ROM11|
|     M6295  12MHz 2018                    SE235 SE005|
|                  2018                               |
|YM2151  2018      2018             ROM8              |
|    ROM3                                  SE194 SE064|
|J    Z80                                  SF004 SF001|
|A                             2018 2018              |
|M     6116 6116               2018 2018              |
|M                                                    |
|A      |--|                                          |
|       |6 |                   62256                  |
|DSWA(8)|8 |62256 62256        62256                  |
|       |0 |ROM4  ROM6                                |
|DSWB(8)|0 |ROM5  ROM7                                |
|       |0 |                                          |
|DSWC(8)|--|                                          |
|-----------------------------------------------------|
Notes:
      68000 - Clock 12MHz
        Z80 - Clock 3.579545MHz
      M6295 - Clock 1.000MHz [16/16]. Pin 7 HIGH
     YM2151 - Clock 3.579545MHz
      62256 - 32k x8 SRAM
       2018 - 2k x8 SRAM
       6116 - 2k x8 SRAM
*/

ROM_START( sf2m10 )
	ROM_REGION( CODE_SIZE, "maincpu", 0 )
	ROM_LOAD16_BYTE( "sf2h14.7", 0x000000, 0x80000, CRC(74803532) SHA1(c1f774bbc4c7b18fcac15417711a86eb852b9957) )
	ROM_LOAD16_BYTE( "sf2h14.5", 0x000001, 0x80000, CRC(66c91972) SHA1(219aecad1feb60bb758190ea82223171075c858e) )
	ROM_LOAD16_BYTE( "yyc-4.1", 0x100000, 0x20000, CRC(1073b7b6) SHA1(81ca1eab65ceac69520584bb23a684ccb9d92f89) )
	ROM_LOAD16_BYTE( "yyc-5.3", 0x100001, 0x20000, CRC(924c6ce2) SHA1(676a912652bd75da5087f0c7eae047b7681a993c) )

	ROM_REGION( 0x600000, "gfx", 0 )
	// using the proper roms for this program
	ROMX_LOAD( "s92-1m.3a",  0x000000, 0x80000, CRC(03b0d852) SHA1(f370f25c96ad2b94f8c53d6b7139100285a25bef) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "s92-3m.5a",  0x000002, 0x80000, CRC(840289ec) SHA1(2fb42a242f60ba7e74009b5a90eb26e035ba1e82) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "s92-2m.4a",  0x000004, 0x80000, CRC(cdb5f027) SHA1(4c7d944fef200fdfcaf57758b901b5511188ed2e) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "s92-4m.6a",  0x000006, 0x80000, CRC(e2799472) SHA1(27d3796429338d82a8de246a0ea06dd487a87768) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "s92-5m.7a",  0x200000, 0x80000, CRC(ba8a2761) SHA1(4b696d66c51611e43522bed752654314e76d33b6) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "s92-7m.9a",  0x200002, 0x80000, CRC(e584bfb5) SHA1(ebdf1f5e2638eed3a65dda82b1ed9151a355f4c9) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "s92-6m.8a",  0x200004, 0x80000, CRC(21e3f87d) SHA1(4a4961bb68c3a1ce15f9d393d9c03ecb2466cc29) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "s92-8m.10a", 0x200006, 0x80000, CRC(befc47df) SHA1(520390420da3a0271ba90b0a933e65143265e5cf) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "s92-10m.3c", 0x400000, 0x80000, CRC(960687d5) SHA1(2868c31121b1c7564e9767b9a19cdbf655c7ed1d) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "s92-12m.5c", 0x400002, 0x80000, CRC(978ecd18) SHA1(648a59706b93c84b4206a968ecbdc3e834c476f6) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "s92-11m.4c", 0x400004, 0x80000, CRC(d6ec9a0a) SHA1(ed6143f8737013b6ef1684e37c05e037e7a80dae) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "s92-13m.6c", 0x400006, 0x80000, CRC(ed2c67f6) SHA1(0083c0ffaf6fe7659ff0cf822be4346cd6e61329) , ROM_GROUPWORD | ROM_SKIP(6) )
	// and overwrite some gfx with supplied extra roms
	ROMX_LOAD( "27c1024.10", 0x400000, 0x20000, CRC(84427d1b) SHA1(f988a2b53c8cc46eeb8032084f24966a539b3734) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "27c1024.12", 0x400002, 0x20000, CRC(55bc790c) SHA1(a1114b89f6fa4487210477676984c77ad94b5ef8) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "27c1024.9",  0x400004, 0x20000, CRC(f8725add) SHA1(fa3fcf6637ee4dd7667bd89766074b3c6ba4f166) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "27c1024.11", 0x400006, 0x20000, CRC(c2a5373e) SHA1(602b32e5ecc7007efe9ad30751040ee52b81f59a) , ROM_GROUPWORD | ROM_SKIP(6) )

	ROM_REGION( 0x18000, "audiocpu", 0 )
	ROM_LOAD( "27c512.3",    0x00000, 0x08000, CRC(a4823a1b) SHA1(7b6bf59dfd578bfbbdb64c27988796783442d659) )
	ROM_CONTINUE(            0x10000, 0x08000 )

	ROM_REGION( 0x20000, "user1", 0 )
	ROM_LOAD( "27c512.8",    0x00000, 0x10000, CRC(13ea1c44) SHA1(5b05fe4c3920e33d94fac5f59e09ff14b3e427fe) )

	ROM_REGION( 0x40000, "oki", 0 )
	ROM_LOAD( "27c010.2",    0x00000, 0x20000, CRC(7f162009) SHA1(346bf42992b4c36c593e21901e22c87ae4a7d86d) )
	ROM_LOAD( "27c010.1",    0x20000, 0x20000, CRC(beade53f) SHA1(277c397dc12752719ec6b47d2224750bd1c07f79) )
ROM_END

ROM_START( sf2yyc )
	ROM_REGION( CODE_SIZE, "maincpu", 0 )      /* 68000 code */
	ROM_LOAD16_BYTE( "b12.rom", 0x000000, 0x80000, CRC(8f742fd5) SHA1(a78a00e686856481011d8b5f5e60ed18197a5225) )
	ROM_LOAD16_BYTE( "b14.rom", 0x000001, 0x80000, CRC(8831ec7f) SHA1(0293ff189cbacf90098e734fb31fcbf3c3165e6b) )
	ROM_LOAD16_BYTE( "b11.rom", 0x100000, 0x20000, CRC(94a46525) SHA1(2712b979ce2bfd87e74da3369e0fceaae2a0654c) )
	ROM_RELOAD(                 0x140000, 0x20000 )
	ROM_LOAD16_BYTE( "b13.rom", 0x100001, 0x20000, CRC(8fb3dd47) SHA1(ebf30ad7ae60eeda446e23bd74f6e2d98dde4158) )
	ROM_RELOAD(                 0x140001, 0x20000 )

	ROM_REGION( 0x600000, "gfx", 0 )
	ROMX_LOAD( "s92_01.bin",   0x000000, 0x80000, CRC(03b0d852) SHA1(f370f25c96ad2b94f8c53d6b7139100285a25bef) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "s92_02.bin",   0x000002, 0x80000, CRC(840289ec) SHA1(2fb42a242f60ba7e74009b5a90eb26e035ba1e82) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "s92_03.bin",   0x000004, 0x80000, CRC(cdb5f027) SHA1(4c7d944fef200fdfcaf57758b901b5511188ed2e) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "s92_04.bin",   0x000006, 0x80000, CRC(e2799472) SHA1(27d3796429338d82a8de246a0ea06dd487a87768) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "s92_05.bin",   0x200000, 0x80000, CRC(ba8a2761) SHA1(4b696d66c51611e43522bed752654314e76d33b6) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "s92_06.bin",   0x200002, 0x80000, CRC(e584bfb5) SHA1(ebdf1f5e2638eed3a65dda82b1ed9151a355f4c9) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "s92_07.bin",   0x200004, 0x80000, CRC(21e3f87d) SHA1(4a4961bb68c3a1ce15f9d393d9c03ecb2466cc29) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "s92_08.bin",   0x200006, 0x80000, CRC(befc47df) SHA1(520390420da3a0271ba90b0a933e65143265e5cf) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "s92_10.bin",   0x400000, 0x80000, CRC(960687d5) SHA1(2868c31121b1c7564e9767b9a19cdbf655c7ed1d) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "s92_11.bin",   0x400002, 0x80000, CRC(978ecd18) SHA1(648a59706b93c84b4206a968ecbdc3e834c476f6) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "s92_12.bin",   0x400004, 0x80000, CRC(d6ec9a0a) SHA1(ed6143f8737013b6ef1684e37c05e037e7a80dae) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "s92_13.bin",   0x400006, 0x80000, CRC(ed2c67f6) SHA1(0083c0ffaf6fe7659ff0cf822be4346cd6e61329) , ROM_GROUPWORD | ROM_SKIP(6) )

	ROM_REGION( 0x18000, "audiocpu", 0 ) /* 64k for the audio CPU (+banks) */
	ROM_LOAD( "s92_09.bin",    0x00000, 0x08000, CRC(08f6b60e) SHA1(8258fcaca4ac419312531eec67079b97f471179c) )
	ROM_CONTINUE(              0x10000, 0x08000 )

	ROM_REGION( 0x40000, "oki", 0 ) /* Samples */
	ROM_LOAD( "s92_18.bin",    0x00000, 0x20000, CRC(7f162009) SHA1(346bf42992b4c36c593e21901e22c87ae4a7d86d) )
	ROM_LOAD( "s92_19.bin",    0x20000, 0x20000, CRC(beade53f) SHA1(277c397dc12752719ec6b47d2224750bd1c07f79) )
ROM_END

ROM_START( sf2koryu )
	ROM_REGION( CODE_SIZE, "maincpu", 0 )      /* 68000 code */
	ROM_LOAD16_BYTE( "u222.rom",   0x000000, 0x80000, CRC(9236a79a) SHA1(39c47b0b0ca2f5f569ff07ebb91040b95d0cb43b) )
	ROM_LOAD16_BYTE( "u196.rom",   0x000001, 0x80000, CRC(b23a869d) SHA1(24247d412f20d069919cc8a7fff208af3f7aa1d2) )
	ROM_LOAD16_BYTE( "u221.rom",   0x100000, 0x20000, CRC(64e6e091) SHA1(32ec05db955e538d4ada26d19ee50926f74b684f) )
	ROM_LOAD16_BYTE( "u195.rom",   0x100001, 0x20000, CRC(c95e4443) SHA1(28417dee9ccdfa65b0f4a92aa29b90279fe8cd85) )

	ROM_REGION( 0x600000, "gfx", 0 )
	ROMX_LOAD( "s92_01.bin",   0x000000, 0x80000, CRC(03b0d852) SHA1(f370f25c96ad2b94f8c53d6b7139100285a25bef) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "s92_02.bin",   0x000002, 0x80000, CRC(840289ec) SHA1(2fb42a242f60ba7e74009b5a90eb26e035ba1e82) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "s92_03.bin",   0x000004, 0x80000, CRC(cdb5f027) SHA1(4c7d944fef200fdfcaf57758b901b5511188ed2e) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "s92_04.bin",   0x000006, 0x80000, CRC(e2799472) SHA1(27d3796429338d82a8de246a0ea06dd487a87768) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "s92_05.bin",   0x200000, 0x80000, CRC(ba8a2761) SHA1(4b696d66c51611e43522bed752654314e76d33b6) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "s92_06.bin",   0x200002, 0x80000, CRC(e584bfb5) SHA1(ebdf1f5e2638eed3a65dda82b1ed9151a355f4c9) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "s92_07.bin",   0x200004, 0x80000, CRC(21e3f87d) SHA1(4a4961bb68c3a1ce15f9d393d9c03ecb2466cc29) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "s92_08.bin",   0x200006, 0x80000, CRC(befc47df) SHA1(520390420da3a0271ba90b0a933e65143265e5cf) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "s92_10.bin",   0x400000, 0x80000, CRC(960687d5) SHA1(2868c31121b1c7564e9767b9a19cdbf655c7ed1d) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "s92_11.bin",   0x400002, 0x80000, CRC(978ecd18) SHA1(648a59706b93c84b4206a968ecbdc3e834c476f6) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "s92_12.bin",   0x400004, 0x80000, CRC(d6ec9a0a) SHA1(ed6143f8737013b6ef1684e37c05e037e7a80dae) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "s92_13.bin",   0x400006, 0x80000, CRC(ed2c67f6) SHA1(0083c0ffaf6fe7659ff0cf822be4346cd6e61329) , ROM_GROUPWORD | ROM_SKIP(6) )

	ROM_REGION( 0x18000, "audiocpu", 0 ) /* 64k for the audio CPU (+banks) */
	ROM_LOAD( "s92_09.bin",    0x00000, 0x08000, CRC(08f6b60e) SHA1(8258fcaca4ac419312531eec67079b97f471179c) )
	ROM_CONTINUE(              0x10000, 0x08000 )

	ROM_REGION( 0x40000, "oki", 0 ) /* Samples */
	ROM_LOAD( "s92_18.bin",    0x00000, 0x20000, CRC(7f162009) SHA1(346bf42992b4c36c593e21901e22c87ae4a7d86d) )
	ROM_LOAD( "s92_19.bin",    0x20000, 0x20000, CRC(beade53f) SHA1(277c397dc12752719ec6b47d2224750bd1c07f79) )
ROM_END

ROM_START( sf2dongb )
	ROM_REGION( CODE_SIZE, "maincpu", 0 )      /* 68000 code */
	ROM_LOAD16_WORD_SWAP( "1.8f",        0x000000, 0x80000, CRC(19fffa37) SHA1(db464c64de3aebe929c85ee167961de05d2c16be) )
	ROM_LOAD16_WORD_SWAP( "1.7f",        0x080000, 0x80000, CRC(99f1cca4) SHA1(64111eba81d743fc3fd51d7a89cd0b2eefcc900d) ) // same as sf2ceea
	ROM_LOAD16_WORD_SWAP( "1.6f",        0x100000, 0x80000, CRC(65c2c719) SHA1(d407cae5f04fc8d05776d478a6eb4363b3c36805) )

	ROM_REGION( 0x600000, "gfx", 0 )
	ROMX_LOAD( "s92-1m.3a",  0x000000, 0x80000, CRC(03b0d852) SHA1(f370f25c96ad2b94f8c53d6b7139100285a25bef) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "s92-3m.5a",  0x000002, 0x80000, CRC(840289ec) SHA1(2fb42a242f60ba7e74009b5a90eb26e035ba1e82) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "s92-2m.4a",  0x000004, 0x80000, CRC(cdb5f027) SHA1(4c7d944fef200fdfcaf57758b901b5511188ed2e) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "s92-4m.6a",  0x000006, 0x80000, CRC(e2799472) SHA1(27d3796429338d82a8de246a0ea06dd487a87768) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "s92-5m.7a",  0x200000, 0x80000, CRC(ba8a2761) SHA1(4b696d66c51611e43522bed752654314e76d33b6) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "s92-7m.9a",  0x200002, 0x80000, CRC(e584bfb5) SHA1(ebdf1f5e2638eed3a65dda82b1ed9151a355f4c9) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "s92-6m.8a",  0x200004, 0x80000, CRC(21e3f87d) SHA1(4a4961bb68c3a1ce15f9d393d9c03ecb2466cc29) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "s92-8m.10a", 0x200006, 0x80000, CRC(befc47df) SHA1(520390420da3a0271ba90b0a933e65143265e5cf) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "s92-10m.3c", 0x400000, 0x80000, CRC(960687d5) SHA1(2868c31121b1c7564e9767b9a19cdbf655c7ed1d) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "s92-12m.5c", 0x400002, 0x80000, CRC(978ecd18) SHA1(648a59706b93c84b4206a968ecbdc3e834c476f6) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "s92-11m.4c", 0x400004, 0x80000, CRC(d6ec9a0a) SHA1(ed6143f8737013b6ef1684e37c05e037e7a80dae) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "s92-13m.6c", 0x400006, 0x80000, CRC(ed2c67f6) SHA1(0083c0ffaf6fe7659ff0cf822be4346cd6e61329) , ROM_GROUPWORD | ROM_SKIP(6) )

	ROM_REGION( 0x18000, "audiocpu", 0 ) /* 64k for the audio CPU (+banks) */
	ROM_LOAD( "s92_09.11a",  0x00000, 0x08000, CRC(08f6b60e) SHA1(8258fcaca4ac419312531eec67079b97f471179c) )
	ROM_CONTINUE(            0x10000, 0x08000 )

	ROM_REGION( 0x40000, "oki", 0 ) /* Samples */
	ROM_LOAD( "s92_18.11c",  0x00000, 0x20000, CRC(7f162009) SHA1(346bf42992b4c36c593e21901e22c87ae4a7d86d) )
	ROM_LOAD( "s92_19.12c",  0x20000, 0x20000, CRC(beade53f) SHA1(277c397dc12752719ec6b47d2224750bd1c07f79) )
ROM_END


/* B-Board 89625B-1 */
ROM_START( cworld2j )
	ROM_REGION( CODE_SIZE, "maincpu", 0 )      /* 68000 code */
	ROM_LOAD16_BYTE( "q5_36.12f",        0x00000, 0x20000, CRC(38a08099) SHA1(961f733baa2bbf8419e4de70f18660098dba7d52) )
	ROM_LOAD16_BYTE( "q5_42.12h",        0x00001, 0x20000, CRC(4d29b3a4) SHA1(bf40fc22c0161fe131ca69100b2a4d102e86bde6) )
	ROM_LOAD16_BYTE( "q5_37.13f",        0x40000, 0x20000, CRC(eb547ebc) SHA1(fce470b05ce095badd180c3740677146f52f6080) )
	ROM_LOAD16_BYTE( "q5_43.13h",        0x40001, 0x20000, CRC(3ef65ea8) SHA1(2348d84b380c0e8ebe270a37d4ff3ce5204abc8c) )
	ROM_LOAD16_BYTE( "q5_34.10f",        0x80000, 0x20000, CRC(7fcc1317) SHA1(672ca45d3fad5eec4d65bbbbd1d21cbf6be4ec8b) )
	ROM_LOAD16_BYTE( "q5_40.10h",        0x80001, 0x20000, CRC(7f14b7b4) SHA1(5564eb9f65dad76ebe40d12d5c39fec5e246adf0) )
	ROM_LOAD16_BYTE( "q5_35.11f",        0xc0000, 0x20000, CRC(abacee26) SHA1(2f513c02f715ffeec12a6d1c292619e214155cbc) )
//  ROM_LOAD16_BYTE( "q5_35.11f",        0xc0000, 0x20000, CRC(59961612) SHA1(ded5144746dd9612f2db1415e96a826e215ad176) ) /* dumped from another board, 1 byte different, pcb verified */
	ROM_LOAD16_BYTE( "q5_41.11h",        0xc0001, 0x20000, CRC(d3654067) SHA1(0b597483e136ff19b031171941cb8439bcd7f145) )

	ROM_REGION( 0x200000, "gfx", 0 )
	ROMX_LOAD( "q5_09.4b",  0x000000, 0x20000, CRC(48496d80) SHA1(bdfaca6375c8275b06b2bc170a25ff6aa62394dc) , ROM_SKIP(7) )
	ROMX_LOAD( "q5_01.4a",  0x000001, 0x20000, CRC(c5453f56) SHA1(113fe6cc6c830352df5992be9fa34c4d70bf32ed) , ROM_SKIP(7) )
	ROMX_LOAD( "q5_13.9b",  0x000002, 0x20000, CRC(c741ac52) SHA1(d8b4aeacfd62586b98a1381da357dcc5ab16c1c6) , ROM_SKIP(7) )
	ROMX_LOAD( "q5_05.9a",  0x000003, 0x20000, CRC(143e068f) SHA1(24cdc49c09a9f0c93e04b37cf7ebba09a929c9b0) , ROM_SKIP(7) )
	ROMX_LOAD( "q5_24.5e",  0x000004, 0x20000, CRC(b419d139) SHA1(46cd97da2413eb5fbd38fa2c20914f3c5f1c6ec8) , ROM_SKIP(7) )
	ROMX_LOAD( "q5_17.5c",  0x000005, 0x20000, CRC(bd3b4d11) SHA1(bb62169bc52562715878a33cc4f8558e05d581d3) , ROM_SKIP(7) )
	ROMX_LOAD( "q5_38.8h",  0x000006, 0x20000, CRC(9c24670c) SHA1(3b98078b7360e21b3905fd973e01b88b02090759) , ROM_SKIP(7) )
	ROMX_LOAD( "q5_32.8f",  0x000007, 0x20000, CRC(3ef9c7c2) SHA1(52a18d7b12f0c14c5cf68a3dd63571e955005f4c) , ROM_SKIP(7) )
	ROMX_LOAD( "q5_10.5b",  0x100000, 0x20000, CRC(119e5e93) SHA1(b5b6c2e3516ebe555a26ecfb5934f3b65371bf36) , ROM_SKIP(7) )
	ROMX_LOAD( "q5_02.5a",  0x100001, 0x20000, CRC(a2cadcbe) SHA1(3d1079f62cce628cbc5b810c0bd51c67c87f4eca) , ROM_SKIP(7) )
	ROMX_LOAD( "q5_14.10b", 0x100002, 0x20000, CRC(a8755f82) SHA1(0a2fbc8b96651f9ab72eb451723e56ca0a859868) , ROM_SKIP(7) )
	ROMX_LOAD( "q5_06.10a", 0x100003, 0x20000, CRC(c92a91fc) SHA1(dfe9682349cf94be414b7e1895b632de41729194) , ROM_SKIP(7) )
	ROMX_LOAD( "q5_25.7e",  0x100004, 0x20000, CRC(979237cb) SHA1(9534b05523317a220b3b957a18fec51f1d4e37b3) , ROM_SKIP(7) )
	ROMX_LOAD( "q5_18.7c",  0x100005, 0x20000, CRC(c57da03c) SHA1(ad7bce859f56d201d229032baf4fb9f65b54765b) , ROM_SKIP(7) )
	ROMX_LOAD( "q5_39.9h",  0x100006, 0x20000, CRC(a5839b25) SHA1(20c4c4f24f21a325a03538306de799df2a89f6cb) , ROM_SKIP(7) )
	ROMX_LOAD( "q5_33.9f",  0x100007, 0x20000, CRC(04d03930) SHA1(37f2556eeb52f8edfcddd3f3642fa24565d5a7bd) , ROM_SKIP(7) )

	ROM_REGION( 0x18000, "audiocpu", 0 ) /* 64k for the audio CPU (+banks) */
	ROM_LOAD( "q5_23.13b",  0x00000, 0x08000, CRC(e14dc524) SHA1(0020a9002572002458fbfe45e8a959cb90de3f03) )
	ROM_CONTINUE(           0x10000, 0x08000 )

	ROM_REGION( 0x40000, "oki", 0 ) /* Samples */
	ROM_LOAD( "q5_30.12c",  0x00000, 0x20000, CRC(d10c1b68) SHA1(2423241f3340d8ab1b6bf9514ca8c3bba1273873) )
	ROM_LOAD( "q5_31.13c",  0x20000, 0x20000, CRC(7d17e496) SHA1(a274b94ec4f042dddc239ecb9ac2e1e2375f5eb2) )

	ROM_REGION( 0x0200, "aboardplds", 0 )
	ROM_LOAD( "buf1",         0x0000, 0x0117, CRC(eb122de7) SHA1(b26b5bfe258e3e184f069719f9fd008d6b8f6b9b) )
	ROM_LOAD( "ioa1",         0x0000, 0x0117, CRC(59c7ee3b) SHA1(fbb887c5b4f5cb8df77cec710eaac2985bc482a6) )
	ROM_LOAD( "prg1",         0x0000, 0x0117, CRC(f1129744) SHA1(a5300f301c1a08a7da768f0773fa0fe3f683b237) )
	ROM_LOAD( "rom1",         0x0000, 0x0117, CRC(41dc73b9) SHA1(7d4c9f1693c821fbf84e32dd6ef62ddf14967845) )
	ROM_LOAD( "sou1",         0x0000, 0x0117, CRC(84f4b2fe) SHA1(dcc9e86cc36316fe42eace02d6df75d08bc8bb6d) )

	ROM_REGION( 0x0200, "bboardplds", 0 )
	ROM_LOAD( "q522b.1a",     0x0000, 0x0117, NO_DUMP )
	ROM_LOAD( "lwio.12e",     0x0000, 0x0117, CRC(ad52b90c) SHA1(f0fd6aeea515ee449320fe15684e6b3ab7f97bf4) )    /* seen the same pcb with IOB1.12E */

	ROM_REGION( 0x0200, "cboardplds", 0 )
	ROM_LOAD( "ioc1.ic1",     0x0000, 0x0117, CRC(0d182081) SHA1(475b3d417785da4bc512cce2b274bb00d4cc6792) )
ROM_END

/* B-Board 90629B-3  - all roms have 90629B on the labels, no battery, possibly unofficial / desuicided with reproduction stickers */
ROM_START( cworld2ja )
	ROM_REGION( CODE_SIZE, "maincpu", 0 )      /* 68000 code */
	ROM_LOAD16_WORD_SWAP("q5 - 34_90629b.8f", 0x00000, 0x80000, CRC(de54487f) SHA1(75b228a6c702c82d4d9a2a992933b5c3c420f6c2) )
	ROM_LOAD16_WORD_SWAP("q5 - 33_90629b.6f", 0x80000, 0x80000, CRC(93248458) SHA1(9dcdc6838f52efc9a0a6333fd0d734946db12dbd) )

	ROM_REGION( 0x200000, "gfx", 0 )
	ROMX_LOAD( "q5 - 06_90629b.8a",  0x000000, 0x80000, CRC(09d0e7ce) SHA1(ea502b975986222acce82ce8396348af72e1df72) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "q5 - 08_90629b.10a", 0x000002, 0x80000, CRC(22e4ce9a) SHA1(9e49aec8e1d6d15a68da63e69765b82fd53a9562) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "q5 - 05_90629b.7a",  0x000004, 0x80000, CRC(f7b3aed6) SHA1(bdfb4d5988307b07ad878ac9129954d14da8769b) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "q5 - 07_90629b.9a",  0x000006, 0x80000, CRC(520c6c88) SHA1(19ba8ca3d75aae71cdf471e6307e86a5df8a2851) , ROM_GROUPWORD | ROM_SKIP(6) )

	ROM_REGION( 0x18000, "audiocpu", 0 ) /* 64k for the audio CPU (+banks) */
	ROM_LOAD( "q5 - 09_90629b.12a",  0x00000, 0x08000, CRC(e14dc524) SHA1(0020a9002572002458fbfe45e8a959cb90de3f03) )
	ROM_CONTINUE(           0x10000, 0x08000 )

	ROM_REGION( 0x40000, "oki", 0 ) /* Samples */
	ROM_LOAD( "q5 - 18_90629b.11c",  0x00000, 0x20000, CRC(d10c1b68) SHA1(2423241f3340d8ab1b6bf9514ca8c3bba1273873) )
	ROM_LOAD( "q5 - 19_90629b.12c",  0x20000, 0x20000, CRC(7d17e496) SHA1(a274b94ec4f042dddc239ecb9ac2e1e2375f5eb2) )

	ROM_REGION( 0x0200, "aboardplds", 0 )
	ROM_LOAD( "buf1",         0x0000, 0x0117, CRC(eb122de7) SHA1(b26b5bfe258e3e184f069719f9fd008d6b8f6b9b) )
	ROM_LOAD( "ioa1",         0x0000, 0x0117, CRC(59c7ee3b) SHA1(fbb887c5b4f5cb8df77cec710eaac2985bc482a6) )
	ROM_LOAD( "prg1",         0x0000, 0x0117, CRC(f1129744) SHA1(a5300f301c1a08a7da768f0773fa0fe3f683b237) )
	ROM_LOAD( "rom1",         0x0000, 0x0117, CRC(41dc73b9) SHA1(7d4c9f1693c821fbf84e32dd6ef62ddf14967845) )
	ROM_LOAD( "sou1",         0x0000, 0x0117, CRC(84f4b2fe) SHA1(dcc9e86cc36316fe42eace02d6df75d08bc8bb6d) )

	ROM_REGION( 0x0200, "bboardplds", 0 )
	ROM_LOAD( "q529b.1a",     0x0000, 0x0117, NO_DUMP )
	ROM_LOAD( "iob1.11d",     0x0000, 0x0117, CRC(3abc0700) SHA1(973043aa46ec6d5d1db20dc9d5937005a0f9f6ae) )

	ROM_REGION( 0x0200, "cboardplds", 0 )
	ROM_LOAD( "ioc1.ic1",     0x0000, 0x0117, CRC(0d182081) SHA1(475b3d417785da4bc512cce2b274bb00d4cc6792) )
ROM_END

/* B-Board 91634B-2  - all roms have 91634B on the labels */
ROM_START( cworld2jb )
	ROM_REGION( CODE_SIZE, "maincpu", 0 )      /* 68000 code */
	ROM_LOAD16_WORD_SWAP( "q5 - 23_91634b.8f", 0x00000, 0x80000, CRC(709f577f) SHA1(3e0615d01f22eb1bf75cbd26dc80ca5a6d08120e) )
	ROM_LOAD16_WORD_SWAP( "q5 - 22_91634b.7f", 0x80000, 0x80000, CRC(93248458) SHA1(9dcdc6838f52efc9a0a6333fd0d734946db12dbd) )

	ROM_REGION( 0x200000, "gfx", 0 )
	ROMX_LOAD( "q5 - 01_91634b.3a", 0x000000, 0x80000, CRC(09d0e7ce) SHA1(ea502b975986222acce82ce8396348af72e1df72) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "q5 - 02_91634b.4a", 0x000002, 0x80000, CRC(22e4ce9a) SHA1(9e49aec8e1d6d15a68da63e69765b82fd53a9562) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "q5 - 03_91634b.5a", 0x000004, 0x80000, CRC(f7b3aed6) SHA1(bdfb4d5988307b07ad878ac9129954d14da8769b) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "q5 - 04_91634b.6a", 0x000006, 0x80000, CRC(520c6c88) SHA1(19ba8ca3d75aae71cdf471e6307e86a5df8a2851) , ROM_GROUPWORD | ROM_SKIP(6) )

	ROM_REGION( 0x18000, "audiocpu", 0 ) /* 64k for the audio CPU (+banks) */
	ROM_LOAD( "q5 - 09_91634b.12a", 0x00000, 0x08000, CRC(e14dc524) SHA1(0020a9002572002458fbfe45e8a959cb90de3f03) )
	ROM_CONTINUE(           0x10000, 0x08000 )

	ROM_REGION( 0x40000, "oki", 0 ) /* Samples */
	ROM_LOAD( "q5 - 18_91634b.11c", 0x00000, 0x20000, CRC(d10c1b68) SHA1(2423241f3340d8ab1b6bf9514ca8c3bba1273873) )
	ROM_LOAD( "q5 - 19_91634b.12c", 0x20000, 0x20000, CRC(7d17e496) SHA1(a274b94ec4f042dddc239ecb9ac2e1e2375f5eb2) )

	ROM_REGION( 0x0200, "aboardplds", 0 )
	ROM_LOAD( "buf1",         0x0000, 0x0117, CRC(eb122de7) SHA1(b26b5bfe258e3e184f069719f9fd008d6b8f6b9b) )
	ROM_LOAD( "ioa1",         0x0000, 0x0117, CRC(59c7ee3b) SHA1(fbb887c5b4f5cb8df77cec710eaac2985bc482a6) )
	ROM_LOAD( "prg1",         0x0000, 0x0117, CRC(f1129744) SHA1(a5300f301c1a08a7da768f0773fa0fe3f683b237) )
	ROM_LOAD( "rom1",         0x0000, 0x0117, CRC(41dc73b9) SHA1(7d4c9f1693c821fbf84e32dd6ef62ddf14967845) )
	ROM_LOAD( "sou1",         0x0000, 0x0117, CRC(84f4b2fe) SHA1(dcc9e86cc36316fe42eace02d6df75d08bc8bb6d) )

	ROM_REGION( 0x0200, "bboardplds", 0 )
	ROM_LOAD( "q563b.1a",     0x0000, 0x0117, NO_DUMP )
	ROM_LOAD( "iob1.12d",     0x0000, 0x0117, CRC(3abc0700) SHA1(973043aa46ec6d5d1db20dc9d5937005a0f9f6ae) )
	ROM_LOAD( "bprg1.11d",    0x0000, 0x0117, CRC(31793da7) SHA1(400fa7ac517421c978c1ee7773c30b9ed0c5d3f3) )

	ROM_REGION( 0x0200, "cboardplds", 0 ) // checkme
	ROM_LOAD( "ioc1.ic7",     0x0000, 0x0117, CRC(0d182081) SHA1(475b3d417785da4bc512cce2b274bb00d4cc6792) )
	ROM_LOAD( "c632.ic1",     0x0000, 0x0117, CRC(0fbd9270) SHA1(d7e737b20c44d41e29ca94be56114b31934dde81) )
ROM_END

/* B-Board 89624B-3 */
ROM_START( varth )
	ROM_REGION( CODE_SIZE, "maincpu", 0 )      /* 68000 code */
	ROM_LOAD16_BYTE( "vae_30b.11f", 0x00000, 0x20000, CRC(adb8d391) SHA1(5e7160509e0315eb32cc390ddd7e4ef7a4a1a70a) )
	ROM_LOAD16_BYTE( "vae_35b.11h", 0x00001, 0x20000, CRC(44e5548f) SHA1(17b4be1f4159f6b6803d8c2950823ece0bdde8b2) )
	ROM_LOAD16_BYTE( "vae_31b.12f", 0x40000, 0x20000, CRC(1749a71c) SHA1(bd9bfd5bbe2d426c94df755c977faa92a28f16ab) )
	ROM_LOAD16_BYTE( "vae_36b.12h", 0x40001, 0x20000, CRC(5f2e2450) SHA1(676e8d96406d81ceadd4a0a69959cdcb6d5d9ac8) )
	ROM_LOAD16_BYTE( "vae_28b.9f",  0x80000, 0x20000, CRC(e524ca50) SHA1(487d5ddabe852872f331362034c4fa16e0926e3d) )
	ROM_LOAD16_BYTE( "vae_33b.9h",  0x80001, 0x20000, CRC(c0bbf8c9) SHA1(447540b856776770af8022a291d46612c1bb5909) )
	ROM_LOAD16_BYTE( "vae_29b.10f", 0xc0000, 0x20000, CRC(6640996a) SHA1(3ed7bd947dc8224435680dedf4955ed6041c6028) )
	ROM_LOAD16_BYTE( "vae_34b.10h", 0xc0001, 0x20000, CRC(fa59be8a) SHA1(86a3d3a7126c021e2ca8ac20238695396367e098) )

	ROM_REGION( 0x200000, "gfx", 0 )
	ROMX_LOAD( "va-5m.7a", 0x000000, 0x80000, CRC(b1fb726e) SHA1(5ac0876b6c49d0a99710dda68653664f4d8c1167) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "va-7m.9a", 0x000002, 0x80000, CRC(4c6588cd) SHA1(d14e8cf051ac934ccc989d8c571c6cc9eed34af5) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "va-1m.3a", 0x000004, 0x80000, CRC(0b1ace37) SHA1(6f9493c22f667f683db2789972fd16bb94724679) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "va-3m.5a", 0x000006, 0x80000, CRC(44dfe706) SHA1(a013a434df3161a91aafbb35dc4e20dfb3f177f4) , ROM_GROUPWORD | ROM_SKIP(6) )

	ROM_REGION( 0x18000, "audiocpu", 0 ) /* 64k for the audio CPU (+banks) */
	ROM_LOAD( "va_09.12b", 0x00000, 0x08000, CRC(7a99446e) SHA1(ca027f41e3e58be5abc33ad7380746658cb5380a) ) // missing sticker, could be va_9.12b
	ROM_CONTINUE(          0x10000, 0x08000 )

	ROM_REGION( 0x40000, "oki", 0 ) /* Samples */
	ROM_LOAD( "va_18.11c", 0x00000, 0x20000, CRC(de30510e) SHA1(8e878696192606b76a3a0e53553e638d9621cff7) )
	ROM_LOAD( "va_19.12c", 0x20000, 0x20000, CRC(0610a4ac) SHA1(3da02ea6a7a56c85de898806d2a1cf6bc526c1b3) )

	ROM_REGION( 0x0200, "aboardplds", 0 )
	ROM_LOAD( "buf1",         0x0000, 0x0117, CRC(eb122de7) SHA1(b26b5bfe258e3e184f069719f9fd008d6b8f6b9b) )
	ROM_LOAD( "ioa1",         0x0000, 0x0117, CRC(59c7ee3b) SHA1(fbb887c5b4f5cb8df77cec710eaac2985bc482a6) )
	ROM_LOAD( "prg1",         0x0000, 0x0117, CRC(f1129744) SHA1(a5300f301c1a08a7da768f0773fa0fe3f683b237) )
	ROM_LOAD( "rom1",         0x0000, 0x0117, CRC(41dc73b9) SHA1(7d4c9f1693c821fbf84e32dd6ef62ddf14967845) )
	ROM_LOAD( "sou1",         0x0000, 0x0117, CRC(84f4b2fe) SHA1(dcc9e86cc36316fe42eace02d6df75d08bc8bb6d) )

	ROM_REGION( 0x0200, "bboardplds", 0 )
	ROM_LOAD( "va24b.1a",     0x0000, 0x0117, CRC(cc476650) SHA1(a417f7971b8709023932fc2b68c9e3cd699ab6f2) )
	ROM_LOAD( "iob1.11e",     0x0000, 0x0117, CRC(3abc0700) SHA1(973043aa46ec6d5d1db20dc9d5937005a0f9f6ae) )
ROM_END

/* B-Board 89624B-? */
ROM_START( varthr1 )
	ROM_REGION( CODE_SIZE, "maincpu", 0 )      /* 68000 code */
	ROM_LOAD16_BYTE( "vae_30a.11f", 0x00000, 0x20000, CRC(7fcd0091) SHA1(7bed452736eda4a26c43c5dd54ec6799afa6e770) )
	ROM_LOAD16_BYTE( "vae_35a.11h", 0x00001, 0x20000, CRC(35cf9509) SHA1(a189ca7740d77262413ec2891af034d0057892be) )
	ROM_LOAD16_BYTE( "vae_31a.12f", 0x40000, 0x20000, CRC(15e5ee81) SHA1(6c6248b07f7e956a37d5dcb4b67d026f57fae13b) )
	ROM_LOAD16_BYTE( "vae_36a.12h", 0x40001, 0x20000, CRC(153a201e) SHA1(5936e447d5cd02ff13802cf78393b521431ad06c) )
	ROM_LOAD16_BYTE( "vae_28a.9f",  0x80000, 0x20000, CRC(7a0e0d25) SHA1(203692ef1daeef7ba08b154cf029cc07a2e0e23d) )
	ROM_LOAD16_BYTE( "vae_33a.9h",  0x80001, 0x20000, CRC(f2365922) SHA1(efb2221033e4b46fedaf3d8c850e208f849e6af0) )
	ROM_LOAD16_BYTE( "vae_29a.10f", 0xc0000, 0x20000, CRC(5e2cd2c3) SHA1(eff955c7dc0d8ae215e7188cc4865726104c7777) )
	ROM_LOAD16_BYTE( "vae_34a.10h", 0xc0001, 0x20000, CRC(3d9bdf83) SHA1(d655803a6f07b90e44aacaa3e6059ac330ef2ec6) )

	ROM_REGION( 0x200000, "gfx", 0 )
	ROMX_LOAD( "va-5m.7a", 0x000000, 0x80000, CRC(b1fb726e) SHA1(5ac0876b6c49d0a99710dda68653664f4d8c1167) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "va-7m.9a", 0x000002, 0x80000, CRC(4c6588cd) SHA1(d14e8cf051ac934ccc989d8c571c6cc9eed34af5) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "va-1m.3a", 0x000004, 0x80000, CRC(0b1ace37) SHA1(6f9493c22f667f683db2789972fd16bb94724679) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "va-3m.5a", 0x000006, 0x80000, CRC(44dfe706) SHA1(a013a434df3161a91aafbb35dc4e20dfb3f177f4) , ROM_GROUPWORD | ROM_SKIP(6) )

	ROM_REGION( 0x18000, "audiocpu", 0 ) /* 64k for the audio CPU (+banks) */
	ROM_LOAD( "va_09.12b", 0x00000, 0x08000, CRC(7a99446e) SHA1(ca027f41e3e58be5abc33ad7380746658cb5380a) )
	ROM_CONTINUE(          0x10000, 0x08000 )

	ROM_REGION( 0x40000, "oki", 0 ) /* Samples */
	ROM_LOAD( "va_18.11c", 0x00000, 0x20000, CRC(de30510e) SHA1(8e878696192606b76a3a0e53553e638d9621cff7) )
	ROM_LOAD( "va_19.12c", 0x20000, 0x20000, CRC(0610a4ac) SHA1(3da02ea6a7a56c85de898806d2a1cf6bc526c1b3) )

	ROM_REGION( 0x0200, "aboardplds", 0 )
	ROM_LOAD( "buf1",         0x0000, 0x0117, CRC(eb122de7) SHA1(b26b5bfe258e3e184f069719f9fd008d6b8f6b9b) )
	ROM_LOAD( "ioa1",         0x0000, 0x0117, CRC(59c7ee3b) SHA1(fbb887c5b4f5cb8df77cec710eaac2985bc482a6) )
	ROM_LOAD( "prg1",         0x0000, 0x0117, CRC(f1129744) SHA1(a5300f301c1a08a7da768f0773fa0fe3f683b237) )
	ROM_LOAD( "rom1",         0x0000, 0x0117, CRC(41dc73b9) SHA1(7d4c9f1693c821fbf84e32dd6ef62ddf14967845) )
	ROM_LOAD( "sou1",         0x0000, 0x0117, CRC(84f4b2fe) SHA1(dcc9e86cc36316fe42eace02d6df75d08bc8bb6d) )

	ROM_REGION( 0x0200, "bboardplds", 0 )
	ROM_LOAD( "va24b.1a",     0x0000, 0x0117, CRC(cc476650) SHA1(a417f7971b8709023932fc2b68c9e3cd699ab6f2) )
	ROM_LOAD( "iob1.11e",     0x0000, 0x0117, CRC(3abc0700) SHA1(973043aa46ec6d5d1db20dc9d5937005a0f9f6ae) )
ROM_END

/* B-Board 91635B-2 */
ROM_START( varthu )
	ROM_REGION( CODE_SIZE, "maincpu", 0 )      /* 68000 code */
	ROM_LOAD16_WORD_SWAP( "vau_23a.8f", 0x00000, 0x80000, CRC(fbe68726) SHA1(68917d366551d2203400adc3261355dd3b332bcb) )
	ROM_LOAD16_WORD_SWAP( "vau_22a.7f", 0x80000, 0x80000, CRC(0ed71bbd) SHA1(e7f0f0edf0936a774e122842b09f5c5ce25a96ad) )

	ROM_REGION( 0x200000, "gfx", 0 )
	ROMX_LOAD( "va-5m.3a", 0x000000, 0x80000, CRC(b1fb726e) SHA1(5ac0876b6c49d0a99710dda68653664f4d8c1167) , ROM_GROUPWORD | ROM_SKIP(6) )  // in "1" socket    // == va-5m.7a
	ROMX_LOAD( "va-7m.5a", 0x000002, 0x80000, CRC(4c6588cd) SHA1(d14e8cf051ac934ccc989d8c571c6cc9eed34af5) , ROM_GROUPWORD | ROM_SKIP(6) )  // in "3" socket    // == va-7m.9a
	ROMX_LOAD( "va-1m.4a", 0x000004, 0x80000, CRC(0b1ace37) SHA1(6f9493c22f667f683db2789972fd16bb94724679) , ROM_GROUPWORD | ROM_SKIP(6) )  // in "2" socket    // == va-1m.3a
	ROMX_LOAD( "va-3m.6a", 0x000006, 0x80000, CRC(44dfe706) SHA1(a013a434df3161a91aafbb35dc4e20dfb3f177f4) , ROM_GROUPWORD | ROM_SKIP(6) )  // in "4" socket    // == va-3m.5a

	ROM_REGION( 0x18000, "audiocpu", 0 ) /* 64k for the audio CPU (+banks) */
	ROM_LOAD( "va_09.11a", 0x00000, 0x08000, CRC(7a99446e) SHA1(ca027f41e3e58be5abc33ad7380746658cb5380a) ) // == va_09.12b
	ROM_CONTINUE(          0x10000, 0x08000 )

	ROM_REGION( 0x40000, "oki", 0 ) /* Samples */
	ROM_LOAD( "va_18.11c", 0x00000, 0x20000, CRC(de30510e) SHA1(8e878696192606b76a3a0e53553e638d9621cff7) )
	ROM_LOAD( "va_19.12c", 0x20000, 0x20000, CRC(0610a4ac) SHA1(3da02ea6a7a56c85de898806d2a1cf6bc526c1b3) )

	ROM_REGION( 0x0200, "aboardplds", 0 )
	ROM_LOAD( "buf1",         0x0000, 0x0117, CRC(eb122de7) SHA1(b26b5bfe258e3e184f069719f9fd008d6b8f6b9b) )
	ROM_LOAD( "ioa1",         0x0000, 0x0117, CRC(59c7ee3b) SHA1(fbb887c5b4f5cb8df77cec710eaac2985bc482a6) )
	ROM_LOAD( "prg1",         0x0000, 0x0117, CRC(f1129744) SHA1(a5300f301c1a08a7da768f0773fa0fe3f683b237) )
	ROM_LOAD( "rom1",         0x0000, 0x0117, CRC(41dc73b9) SHA1(7d4c9f1693c821fbf84e32dd6ef62ddf14967845) )
	ROM_LOAD( "sou1",         0x0000, 0x0117, CRC(84f4b2fe) SHA1(dcc9e86cc36316fe42eace02d6df75d08bc8bb6d) )

	ROM_REGION( 0x0200, "bboardplds", 0 )
	ROM_LOAD( "va63b.1a",     0x0000, 0x0117, CRC(132ab7c5) SHA1(54b78b02cdd9430c90e2289d42585ee71cf71cdc) )
	ROM_LOAD( "iob1.12d",     0x0000, 0x0117, CRC(3abc0700) SHA1(973043aa46ec6d5d1db20dc9d5937005a0f9f6ae) )
	ROM_LOAD( "bprg1.11d",    0x0000, 0x0117, CRC(31793da7) SHA1(400fa7ac517421c978c1ee7773c30b9ed0c5d3f3) )
ROM_END

/* B-Board 88622B-3 */
ROM_START( varthj )
	ROM_REGION( CODE_SIZE, "maincpu", 0 )      /* 68000 code */
	ROM_LOAD16_BYTE( "vaj_36b.12f", 0x00000, 0x20000, CRC(1d798d6a) SHA1(b12579e9dcb88416bc00653b143832d9347adbaf) )
	ROM_LOAD16_BYTE( "vaj_42b.12h", 0x00001, 0x20000, CRC(0f720233) SHA1(2d9442ceafd5e2208aa8cd4bcb66861bff6aec47) )
	ROM_LOAD16_BYTE( "vaj_37b.13f", 0x40000, 0x20000, CRC(24414b17) SHA1(6c0b24cf8045fc033217c737dba2c046d7d0a09a) )
	ROM_LOAD16_BYTE( "vaj_43b.13h", 0x40001, 0x20000, CRC(34b4b06c) SHA1(3033d1d053ba97d6da17064d7b944a10817b93b1) )
	ROM_LOAD16_BYTE( "vaj_34b.10f", 0x80000, 0x20000, CRC(87c79aed) SHA1(bb90720d1d04ed6ad276a5230cb078229aa8a40a) )
	ROM_LOAD16_BYTE( "vaj_40b.10h", 0x80001, 0x20000, CRC(210b4bd0) SHA1(15771c32af9fb4760953ef5475de228200851b42) )
	ROM_LOAD16_BYTE( "vaj_35b.11f", 0xc0000, 0x20000, CRC(6b0da69f) SHA1(5883bea31a22a44ad7494d6acd523c88b62f8743) )
	ROM_LOAD16_BYTE( "vaj_41b.11h", 0xc0001, 0x20000, CRC(6542c8a4) SHA1(5f828cf28ef905e4701c92f317e1257a40964a65) )

	ROM_REGION( 0x200000, "gfx", 0 )
	ROMX_LOAD( "va_09.4b",  0x000000, 0x20000, CRC(183dfaa8) SHA1(230c65c1b11b3a5d1733455e0130dad2740e2d2d) , ROM_SKIP(7) ) // == va-5m.7a
	ROMX_LOAD( "va_01.4a",  0x000001, 0x20000, CRC(c41312b5) SHA1(4077cb8200fc403953a08d94652fa8f572dc202d) , ROM_SKIP(7) ) // == va-5m.7a
	ROMX_LOAD( "va_13.9b",  0x000002, 0x20000, CRC(45537e69) SHA1(18581cbf09b1ec35ea388dce73db7099a1790f60) , ROM_SKIP(7) ) // == va-7m.9a
	ROMX_LOAD( "va_05.9a",  0x000003, 0x20000, CRC(7065d4e9) SHA1(0e16b4ba2309cca609eaa906c99c61172ca273d0) , ROM_SKIP(7) ) // == va-7m.9a
	ROMX_LOAD( "va_24.5e",  0x000004, 0x20000, CRC(57191ccf) SHA1(8247b6ca36dd114ea2d030141ce48ea881ea648c) , ROM_SKIP(7) ) // == va-1m.3a
	ROMX_LOAD( "va_17.5c",  0x000005, 0x20000, CRC(054f5a5b) SHA1(28fc6ff2144daad18b5aed8c08d0b65e6fc2b06f) , ROM_SKIP(7) ) // == va-1m.3a
	ROMX_LOAD( "va_38.8h",  0x000006, 0x20000, CRC(e117a17e) SHA1(576ec580050e9ce3e3be96b849247288411ff68c) , ROM_SKIP(7) ) // == va-3m.5a
	ROMX_LOAD( "va_32.8f",  0x000007, 0x20000, CRC(3b4f40b2) SHA1(7033d0f754381fe8d5ed29b58ebbd665a0ba1725) , ROM_SKIP(7) ) // == va-3m.5a
	ROMX_LOAD( "va_10.5b",  0x100000, 0x20000, CRC(d62750cd) SHA1(0b792f806ed5ab7f6ec0c53bb9bf9965d7ddc47e) , ROM_SKIP(7) ) // == va-5m.7a
	ROMX_LOAD( "va_02.5a",  0x100001, 0x20000, CRC(11590325) SHA1(9d776f4008db76f8f141db5024a3eed78e364b6a) , ROM_SKIP(7) ) // == va-5m.7a
	ROMX_LOAD( "va_14.10b", 0x100002, 0x20000, CRC(dc2f4783) SHA1(f9c274d1ab24159980f29db7da5bcc179761237f) , ROM_SKIP(7) ) // == va-7m.9a
	ROMX_LOAD( "va_06.10a", 0x100003, 0x20000, CRC(06e833ac) SHA1(e8df6e2ef8300b5e412dd74cfe329b5535056e62) , ROM_SKIP(7) ) // == va-7m.9a
	ROMX_LOAD( "va_25.7e",  0x100004, 0x20000, CRC(51d90690) SHA1(9079d56007aae257f56ce47bbb24873dc18c5bd6) , ROM_SKIP(7) ) // == va-1m.3a
	ROMX_LOAD( "va_18.7c",  0x100005, 0x20000, CRC(a17817c0) SHA1(23d9ae2ae68e4c8be72da7013109ecdfc30d4b53) , ROM_SKIP(7) ) // == va-1m.3a
	ROMX_LOAD( "va_39.9h",  0x100006, 0x20000, CRC(b0b12f51) SHA1(68a33736dcb0703e46ba48918a29ecd559575a97) , ROM_SKIP(7) ) // == va-3m.5a
	ROMX_LOAD( "va_33.9f",  0x100007, 0x20000, CRC(4b003af7) SHA1(0c1d18a3ee7f3a48219f73eb21f88a260a9a001e) , ROM_SKIP(7) ) // == va-3m.5a

	ROM_REGION( 0x18000, "audiocpu", 0 ) /* 64k for the audio CPU (+banks) */
	ROM_LOAD( "va_23.13c",  0x00000, 0x08000, CRC(7a99446e) SHA1(ca027f41e3e58be5abc33ad7380746658cb5380a) )    // == va_09.12b
	ROM_CONTINUE(           0x10000, 0x08000 )

	ROM_REGION( 0x40000, "oki", 0 ) /* Samples */
	ROM_LOAD( "va_30.12e",  0x00000, 0x20000, CRC(de30510e) SHA1(8e878696192606b76a3a0e53553e638d9621cff7) )    // == va_18.11c
	ROM_LOAD( "va_31.13e",  0x20000, 0x20000, CRC(0610a4ac) SHA1(3da02ea6a7a56c85de898806d2a1cf6bc526c1b3) )    // == va_19.12c

	ROM_REGION( 0x0200, "aboardplds", 0 )
	ROM_LOAD( "buf1",         0x0000, 0x0117, CRC(eb122de7) SHA1(b26b5bfe258e3e184f069719f9fd008d6b8f6b9b) )
	ROM_LOAD( "ioa1",         0x0000, 0x0117, CRC(59c7ee3b) SHA1(fbb887c5b4f5cb8df77cec710eaac2985bc482a6) )
	ROM_LOAD( "prg1",         0x0000, 0x0117, CRC(f1129744) SHA1(a5300f301c1a08a7da768f0773fa0fe3f683b237) )
	ROM_LOAD( "rom1",         0x0000, 0x0117, CRC(41dc73b9) SHA1(7d4c9f1693c821fbf84e32dd6ef62ddf14967845) )
	ROM_LOAD( "sou1",         0x0000, 0x0117, CRC(84f4b2fe) SHA1(dcc9e86cc36316fe42eace02d6df75d08bc8bb6d) )

	ROM_REGION( 0x0200, "bboardplds", 0 )
	ROM_LOAD( "va22b.1a",     0x0000, 0x0117, CRC(bd7cd574) SHA1(00e49631aceb2871e9313f40264fa55eaaa3538c) )
	ROM_LOAD( "lwio.12c",     0x0000, 0x0117, CRC(ad52b90c) SHA1(f0fd6aeea515ee449320fe15684e6b3ab7f97bf4) )

	ROM_REGION( 0x0200, "cboardplds", 0 )
	ROM_LOAD( "ioc1.ic1",     0x0000, 0x0117, CRC(0d182081) SHA1(475b3d417785da4bc512cce2b274bb00d4cc6792) )
ROM_END

/* B-Board 91634B-2, Japan Resale Ver. */
ROM_START( varthjr )
	ROM_REGION( CODE_SIZE, "maincpu", 0 )      /* 68000 code */
	ROM_LOAD16_WORD_SWAP( "vaj_23b.8f", 0x00000, 0x80000, CRC(ad3d3522) SHA1(db627233f9d8a03c2d4bb31614951a0cdc81600d) )
	ROM_LOAD16_WORD_SWAP( "vaj_22b.7f", 0x80000, 0x80000, CRC(034e3e55) SHA1(eeb85a827cf18dafbdf0a2828aa39128352857f3) )

	ROM_REGION( 0x200000, "gfx", 0 )
	ROMX_LOAD( "va_01.3a", 0x000000, 0x80000, CRC(b1fb726e) SHA1(5ac0876b6c49d0a99710dda68653664f4d8c1167) , ROM_GROUPWORD | ROM_SKIP(6) )  // == va-5m.7a
	ROMX_LOAD( "va_02.4a", 0x000002, 0x80000, CRC(4c6588cd) SHA1(d14e8cf051ac934ccc989d8c571c6cc9eed34af5) , ROM_GROUPWORD | ROM_SKIP(6) )  // == va-7m.9a
	ROMX_LOAD( "va_03.5a", 0x000004, 0x80000, CRC(0b1ace37) SHA1(6f9493c22f667f683db2789972fd16bb94724679) , ROM_GROUPWORD | ROM_SKIP(6) )  // == va-1m.3a
	ROMX_LOAD( "va_04.6a", 0x000006, 0x80000, CRC(44dfe706) SHA1(a013a434df3161a91aafbb35dc4e20dfb3f177f4) , ROM_GROUPWORD | ROM_SKIP(6) )  // == va-3m.5a

	ROM_REGION( 0x18000, "audiocpu", 0 ) /* 64k for the audio CPU (+banks) */
	ROM_LOAD( "va_09.12a", 0x00000, 0x08000, CRC(7a99446e) SHA1(ca027f41e3e58be5abc33ad7380746658cb5380a) )
	ROM_CONTINUE(          0x10000, 0x08000 )

	ROM_REGION( 0x40000, "oki", 0 ) /* Samples */
	ROM_LOAD( "va_18.11c", 0x00000, 0x20000, CRC(de30510e) SHA1(8e878696192606b76a3a0e53553e638d9621cff7) )
	ROM_LOAD( "va_19.12c", 0x20000, 0x20000, CRC(0610a4ac) SHA1(3da02ea6a7a56c85de898806d2a1cf6bc526c1b3) )

	ROM_REGION( 0x0200, "aboardplds", 0 )
	ROM_LOAD( "buf1",         0x0000, 0x0117, CRC(eb122de7) SHA1(b26b5bfe258e3e184f069719f9fd008d6b8f6b9b) )
	ROM_LOAD( "ioa1",         0x0000, 0x0117, CRC(59c7ee3b) SHA1(fbb887c5b4f5cb8df77cec710eaac2985bc482a6) )
	ROM_LOAD( "prg1",         0x0000, 0x0117, CRC(f1129744) SHA1(a5300f301c1a08a7da768f0773fa0fe3f683b237) )
	ROM_LOAD( "rom1",         0x0000, 0x0117, CRC(41dc73b9) SHA1(7d4c9f1693c821fbf84e32dd6ef62ddf14967845) )
	ROM_LOAD( "sou1",         0x0000, 0x0117, CRC(84f4b2fe) SHA1(dcc9e86cc36316fe42eace02d6df75d08bc8bb6d) )

	ROM_REGION( 0x0200, "bboardplds", 0 )
	ROM_LOAD( "va63b.1a",     0x0000, 0x0117, CRC(132ab7c5) SHA1(54b78b02cdd9430c90e2289d42585ee71cf71cdc) )
	ROM_LOAD( "iob1.12d",     0x0000, 0x0117, CRC(3abc0700) SHA1(973043aa46ec6d5d1db20dc9d5937005a0f9f6ae) )
	ROM_LOAD( "bprg1.11d",    0x0000, 0x0117, CRC(31793da7) SHA1(400fa7ac517421c978c1ee7773c30b9ed0c5d3f3) )

	ROM_REGION( 0x0200, "cboardplds", 0 )
	ROM_LOAD( "ioc1.ic1",     0x0000, 0x0117, CRC(0d182081) SHA1(475b3d417785da4bc512cce2b274bb00d4cc6792) )
ROM_END

/* B-Board 89625B-1 */
ROM_START( qad )
	ROM_REGION( CODE_SIZE, "maincpu", 0 )      /* 68000 code */
	ROM_LOAD16_BYTE( "qdu_36a.12f", 0x00000, 0x20000, CRC(de9c24a0) SHA1(458962943e8d97d1f4e5a15ac1c8d3bcaa32918b) )
	ROM_LOAD16_BYTE( "qdu_42a.12h", 0x00001, 0x20000, CRC(cfe36f0c) SHA1(370a47461b2dbb7807f547f5b4b33296572c5d78) )
	ROM_LOAD16_BYTE( "qdu_37a.13f", 0x40000, 0x20000, CRC(10d22320) SHA1(73b2876d5447f50a850c466789d9297269f732d6) )
	ROM_LOAD16_BYTE( "qdu_43a.13h", 0x40001, 0x20000, CRC(15e6beb9) SHA1(68d11e9bdd82775060281c5880f249e3515dc235) )

	ROM_REGION( 0x200000, "gfx", 0 )
	ROMX_LOAD( "qd_09.4b",  0x000000, 0x20000, CRC(8c3f9f44) SHA1(b5ab20515b7f3e7db023be42d4c7ed1941b37d9b) , ROM_SKIP(7) )
	ROMX_LOAD( "qd_01.4a",  0x000001, 0x20000, CRC(f688cf8f) SHA1(1b20095e536a24406513715cded249c9be1aa1d2) , ROM_SKIP(7) )
	ROMX_LOAD( "qd_13.9b",  0x000002, 0x20000, CRC(afbd551b) SHA1(02e2f12196c542a004325689bda8949213ef0333) , ROM_SKIP(7) )
	ROMX_LOAD( "qd_05.9a",  0x000003, 0x20000, CRC(c3db0910) SHA1(cf3aa3d3b64031dea92a80e5650151315cf871bf) , ROM_SKIP(7) )
	ROMX_LOAD( "qd_24.5e",  0x000004, 0x20000, CRC(2f1bd0ec) SHA1(017e0dc521bf402c700775ee06cbc124f7ce0e3f) , ROM_SKIP(7) )
	ROMX_LOAD( "qd_17.5c",  0x000005, 0x20000, CRC(a812f9e2) SHA1(9b7ceb347fbe00c40338b97ee6e8e4d1db9e7cb3) , ROM_SKIP(7) )
	ROMX_LOAD( "qd_38.8h",  0x000006, 0x20000, CRC(ccdddd1f) SHA1(8304c4cdfaa1ae6b37e2733e9a6ddce9252fd43a) , ROM_SKIP(7) )
	ROMX_LOAD( "qd_32.8f",  0x000007, 0x20000, CRC(a8d295d3) SHA1(d4d0bdaeb40f652ef33b317cb2b566b4c3550242) , ROM_SKIP(7) )

	ROM_REGION( 0x18000, "audiocpu", 0 ) /* 64k for the audio CPU (+banks) */
	ROM_LOAD( "qd_23.13b",  0x00000, 0x08000, CRC(cfb5264b) SHA1(e662ed5555d02ccf4e62cdbcfa0bbfc019734ee1) )
	ROM_CONTINUE(           0x10000, 0x08000 )

	ROM_REGION( 0x40000, "oki", 0 ) /* Samples */
	ROM_LOAD( "qdu_30.12c", 0x00000, 0x20000, CRC(f190da84) SHA1(d5cd4c69b5d135a2f2fea8ca9631251c9da79e70) )
	ROM_LOAD( "qdu_31.13c", 0x20000, 0x20000, CRC(b7583f73) SHA1(3896e0fcf375e9e5d9ba70cc1ed001cd702f9ff7) )

	ROM_REGION( 0x0200, "aboardplds", 0 )
	ROM_LOAD( "buf1",         0x0000, 0x0117, CRC(eb122de7) SHA1(b26b5bfe258e3e184f069719f9fd008d6b8f6b9b) )
	ROM_LOAD( "ioa1",         0x0000, 0x0117, CRC(59c7ee3b) SHA1(fbb887c5b4f5cb8df77cec710eaac2985bc482a6) )
	ROM_LOAD( "prg1",         0x0000, 0x0117, CRC(f1129744) SHA1(a5300f301c1a08a7da768f0773fa0fe3f683b237) )
	ROM_LOAD( "rom1",         0x0000, 0x0117, CRC(41dc73b9) SHA1(7d4c9f1693c821fbf84e32dd6ef62ddf14967845) )
	ROM_LOAD( "sou1",         0x0000, 0x0117, CRC(84f4b2fe) SHA1(dcc9e86cc36316fe42eace02d6df75d08bc8bb6d) )

	ROM_REGION( 0x0200, "bboardplds", 0 )
	ROM_LOAD( "qd22b.1a",     0x0000, 0x0117, CRC(783c53ab) SHA1(1bf87e5fe7e7cbcec0d76ed094dcac823e45af14) )
	ROM_LOAD( "iob1.12e",     0x0000, 0x0117, CRC(3abc0700) SHA1(973043aa46ec6d5d1db20dc9d5937005a0f9f6ae) )    /* seen the same pcb with LWIO.12E */

	ROM_REGION( 0x0200, "cboardplds", 0 )
	ROM_LOAD( "ioc1.ic1",     0x0000, 0x0117, CRC(0d182081) SHA1(475b3d417785da4bc512cce2b274bb00d4cc6792) )
ROM_END

/* B-Board 91634B-2, Japan Resale Ver. */
ROM_START( qadjr )
	ROM_REGION( CODE_SIZE, "maincpu", 0 )      /* 68000 code */
	ROM_LOAD16_WORD_SWAP( "qad_23a.8f", 0x00000, 0x80000, CRC(4d3553de) SHA1(07eabcb02fbbe11397ce91405a2e6bb53b3d5d4f) )
	ROM_LOAD16_WORD_SWAP( "qad_22a.7f", 0x80000, 0x80000, CRC(3191ddd0) SHA1(2806021a5dc809ca43692bbe9c4f5ef690c9ac14) )

	ROM_REGION( 0x200000, "gfx", 0 )
	ROMX_LOAD( "qad_01.3a", 0x000000, 0x80000, CRC(9d853b57) SHA1(380b41a3eced1f4a5523999b63d80b7593a85eca) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "qad_02.4a", 0x000002, 0x80000, CRC(b35976c4) SHA1(3e128db89186c4e88c46be9da310b755ae5b816c) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "qad_03.5a", 0x000004, 0x80000, CRC(cea4ca8c) SHA1(5c50758647419129f2b35ab4dc712796fa801c12) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "qad_04.6a", 0x000006, 0x80000, CRC(41b74d1b) SHA1(78aa2faec512c505f98b4e8053fc161941d41773) , ROM_GROUPWORD | ROM_SKIP(6) )

	ROM_REGION( 0x18000, "audiocpu", 0 ) /* 64k for the audio CPU (+banks) */
	ROM_LOAD( "qad_09.12a", 0x00000, 0x08000, CRC(733161cc) SHA1(dfb8c5a1037bd3b2712fb327122ec39ceb993b8d) )
	ROM_CONTINUE(           0x10000, 0x08000 )

	ROM_REGION( 0x40000, "oki", 0 ) /* Samples */
	ROM_LOAD( "qad_18.11c", 0x00000, 0x20000, CRC(2bfe6f6a) SHA1(b2a98ac034c65b7ac8167431f05f35d4799032ea) )
	ROM_LOAD( "qad_19.12c", 0x20000, 0x20000, CRC(13d3236b) SHA1(785d49de484e9ac6971eaceebebfecb8e58563f6) )

	ROM_REGION( 0x0200, "aboardplds", 0 )
	ROM_LOAD( "buf1",         0x0000, 0x0117, CRC(eb122de7) SHA1(b26b5bfe258e3e184f069719f9fd008d6b8f6b9b) )
	ROM_LOAD( "ioa1",         0x0000, 0x0117, CRC(59c7ee3b) SHA1(fbb887c5b4f5cb8df77cec710eaac2985bc482a6) )
	ROM_LOAD( "prg1",         0x0000, 0x0117, CRC(f1129744) SHA1(a5300f301c1a08a7da768f0773fa0fe3f683b237) )
	ROM_LOAD( "rom1",         0x0000, 0x0117, CRC(41dc73b9) SHA1(7d4c9f1693c821fbf84e32dd6ef62ddf14967845) )
	ROM_LOAD( "sou1",         0x0000, 0x0117, CRC(84f4b2fe) SHA1(dcc9e86cc36316fe42eace02d6df75d08bc8bb6d) )

	ROM_REGION( 0x0200, "bboardplds", 0 )
	ROM_LOAD( "qad63b.1a",    0x0000, 0x0117, CRC(b3312b13) SHA1(24bd6235fe273a672de2d4749e57280994f11819) )
	ROM_LOAD( "iob1.12d",     0x0000, 0x0117, CRC(3abc0700) SHA1(973043aa46ec6d5d1db20dc9d5937005a0f9f6ae) )
	ROM_LOAD( "bprg1.11d",    0x0000, 0x0117, CRC(31793da7) SHA1(400fa7ac517421c978c1ee7773c30b9ed0c5d3f3) )

	ROM_REGION( 0x0200, "cboardplds", 0 )
	ROM_LOAD( "ioc1.ic7",     0x0000, 0x0117, CRC(0d182081) SHA1(475b3d417785da4bc512cce2b274bb00d4cc6792) )
	ROM_LOAD( "c632.ic1",     0x0000, 0x0117, CRC(0fbd9270) SHA1(d7e737b20c44d41e29ca94be56114b31934dde81) )
ROM_END

/* B-Board 91635B-2 */
ROM_START( wof )
	ROM_REGION( CODE_SIZE, "maincpu", 0 )      /* 68000 code */
	ROM_LOAD16_WORD_SWAP( "tk2e_23c.8f", 0x000000, 0x80000, CRC(0d708505) SHA1(10b8cb53a4600e3e76f471a3eee8a600e93096fc) )
	ROM_LOAD16_WORD_SWAP( "tk2e_22c.7f", 0x080000, 0x80000, CRC(608c17e3) SHA1(52c2d05279623d93b27856e6b76830796a089eae) )

	ROM_REGION( 0x400000, "gfx", 0 )
	ROMX_LOAD( "tk2-1m.3a",  0x000000, 0x80000, CRC(0d9cb9bf) SHA1(cc7140e9a01a14b252cb1090bcea32b0de461928) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "tk2-3m.5a",  0x000002, 0x80000, CRC(45227027) SHA1(b21afc593f0d4d8909dfa621d659cbb40507d1b2) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "tk2-2m.4a",  0x000004, 0x80000, CRC(c5ca2460) SHA1(cbe14867f7b94b638ca80db7c8e0c60881183469) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "tk2-4m.6a",  0x000006, 0x80000, CRC(e349551c) SHA1(1d977bdf256accf750ad9930ec4a0a19bbf86964) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "tk2-5m.7a",  0x200000, 0x80000, CRC(291f0f0b) SHA1(094baf0f960f25fc2525b3b1cc378a49d9a0955d) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "tk2-7m.9a",  0x200002, 0x80000, CRC(3edeb949) SHA1(c155698dd9ee9eb24bbc97a21118ef2e897ea82f) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "tk2-6m.8a",  0x200004, 0x80000, CRC(1abd14d6) SHA1(dffff3126f102b4ec028a81405fc5b9bd7bb65b3) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "tk2-8m.10a", 0x200006, 0x80000, CRC(b27948e3) SHA1(870d5d23f56798831c641e877ea94217058b2ddc) , ROM_GROUPWORD | ROM_SKIP(6) )

	ROM_REGION( 0x28000, "audiocpu", 0 ) /* QSound Z80 code */
	ROM_LOAD( "tk2_qa.5k",   0x00000, 0x08000, CRC(c9183a0d) SHA1(d8b1d41c572f08581f8ab9eb878de77d6ea8615d) )
	ROM_CONTINUE(            0x10000, 0x18000 )

	ROM_REGION( 0x200000, "qsound", 0 ) /* QSound samples */
	ROM_LOAD( "tk2-q1.1k",   0x000000, 0x80000, CRC(611268cf) SHA1(83ab059f2110fb25fdcff928d56b790fc1f5c975) )
	ROM_LOAD( "tk2-q2.2k",   0x080000, 0x80000, CRC(20f55ca9) SHA1(90134e9a9c4749bb65c728b66ea4dac1fd4d88a4) )
	ROM_LOAD( "tk2-q3.3k",   0x100000, 0x80000, CRC(bfcf6f52) SHA1(2a85ff3fc89b4cbabd20779ec12da2e116333c7c) )
	ROM_LOAD( "tk2-q4.4k",   0x180000, 0x80000, CRC(36642e88) SHA1(8ab25b19e2b67215a5cb1f3aa81b9d26009cfeb8) )

	ROM_REGION( 0x0200, "aboardplds", 0 )
	ROM_LOAD( "buf1",         0x0000, 0x0117, CRC(eb122de7) SHA1(b26b5bfe258e3e184f069719f9fd008d6b8f6b9b) )
	ROM_LOAD( "ioa1",         0x0000, 0x0117, CRC(59c7ee3b) SHA1(fbb887c5b4f5cb8df77cec710eaac2985bc482a6) )
	ROM_LOAD( "prg2",         0x0000, 0x0117, CRC(4386879a) SHA1(c36896d169d8c78393609acbbe4397931292a033) )
	ROM_LOAD( "rom1",         0x0000, 0x0117, CRC(41dc73b9) SHA1(7d4c9f1693c821fbf84e32dd6ef62ddf14967845) )

	ROM_REGION( 0x0200, "bboardplds", 0 )
	ROM_LOAD( "tk263b.1a",    0x0000, 0x0117, CRC(c4b0349b) SHA1(b4873dd5ad8735048deb3475222dde3c0b67eaaf) )
	ROM_LOAD( "iob1.12d",     0x0000, 0x0117, CRC(3abc0700) SHA1(973043aa46ec6d5d1db20dc9d5937005a0f9f6ae) )
	ROM_LOAD( "bprg1.11d",    0x0000, 0x0117, CRC(31793da7) SHA1(400fa7ac517421c978c1ee7773c30b9ed0c5d3f3) )

	ROM_REGION( 0x0200, "cboardplds", 0 )
	ROM_LOAD( "ioc1.ic1",     0x0000, 0x0117, CRC(0d182081) SHA1(475b3d417785da4bc512cce2b274bb00d4cc6792) )

	ROM_REGION( 0x0200, "dboardplds", 0 )
	ROM_LOAD( "d7l1.7l",      0x0000, 0x0117, CRC(27b7410d) SHA1(06d0cba0226850f100ff1f539bd7d5db0f90c730) )
	ROM_LOAD( "d8l1.8l",      0x0000, 0x0117, CRC(539fc7da) SHA1(cad5c91629c6247e49ccbbcbfe6b08229eafae07) )
	ROM_LOAD( "d9k1.9k",      0x0000, 0x0117, NO_DUMP )                                     // pal verification required
	ROM_LOAD( "d10f1.10f",    0x0000, 0x0117, CRC(6619c494) SHA1(3aef656c07182a2186f810f30e0d854dd5bd8d18) )
ROM_END

/* B-Board 91635B-2 */
/* FIXME Probably this set uses a patched program ROM coming from a desuicided board, or simply the original C-Board
   is not a 92641C-1. A verification and a new fresh dump are needed to confirm if it's genuine or not. */
ROM_START( wofr1 )
	ROM_REGION( CODE_SIZE, "maincpu", 0 )      /* 68000 code */
	ROM_LOAD16_WORD_SWAP( "tk2e_23b.8f", 0x000000, 0x80000, CRC(11fb2ed1) SHA1(19e09ad6f9edc7997b030cddfe1d9c96d88135f2) )  // patched?
	ROM_LOAD16_WORD_SWAP( "tk2e_22b.7f", 0x080000, 0x80000, CRC(479b3f24) SHA1(9fb8ae06856fe115addfb6794c28978a4f6716ec) )

	ROM_REGION( 0x400000, "gfx", 0 )
	ROMX_LOAD( "tk2-1m.3a",  0x000000, 0x80000, CRC(0d9cb9bf) SHA1(cc7140e9a01a14b252cb1090bcea32b0de461928) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "tk2-3m.5a",  0x000002, 0x80000, CRC(45227027) SHA1(b21afc593f0d4d8909dfa621d659cbb40507d1b2) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "tk2-2m.4a",  0x000004, 0x80000, CRC(c5ca2460) SHA1(cbe14867f7b94b638ca80db7c8e0c60881183469) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "tk2-4m.6a",  0x000006, 0x80000, CRC(e349551c) SHA1(1d977bdf256accf750ad9930ec4a0a19bbf86964) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "tk2-5m.7a",  0x200000, 0x80000, CRC(291f0f0b) SHA1(094baf0f960f25fc2525b3b1cc378a49d9a0955d) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "tk2-7m.9a",  0x200002, 0x80000, CRC(3edeb949) SHA1(c155698dd9ee9eb24bbc97a21118ef2e897ea82f) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "tk2-6m.8a",  0x200004, 0x80000, CRC(1abd14d6) SHA1(dffff3126f102b4ec028a81405fc5b9bd7bb65b3) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "tk2-8m.10a", 0x200006, 0x80000, CRC(b27948e3) SHA1(870d5d23f56798831c641e877ea94217058b2ddc) , ROM_GROUPWORD | ROM_SKIP(6) )

	ROM_REGION( 0x28000, "audiocpu", 0 ) /* QSound Z80 code */
	ROM_LOAD( "tk2_qa.5k",   0x00000, 0x08000, CRC(c9183a0d) SHA1(d8b1d41c572f08581f8ab9eb878de77d6ea8615d) )
	ROM_CONTINUE(            0x10000, 0x18000 )

	ROM_REGION( 0x200000, "qsound", 0 ) /* QSound samples */
	ROM_LOAD( "tk2-q1.1k",   0x000000, 0x80000, CRC(611268cf) SHA1(83ab059f2110fb25fdcff928d56b790fc1f5c975) )
	ROM_LOAD( "tk2-q2.2k",   0x080000, 0x80000, CRC(20f55ca9) SHA1(90134e9a9c4749bb65c728b66ea4dac1fd4d88a4) )
	ROM_LOAD( "tk2-q3.3k",   0x100000, 0x80000, CRC(bfcf6f52) SHA1(2a85ff3fc89b4cbabd20779ec12da2e116333c7c) )
	ROM_LOAD( "tk2-q4.4k",   0x180000, 0x80000, CRC(36642e88) SHA1(8ab25b19e2b67215a5cb1f3aa81b9d26009cfeb8) )

	ROM_REGION( 0x0200, "aboardplds", 0 )
	ROM_LOAD( "buf1",         0x0000, 0x0117, CRC(eb122de7) SHA1(b26b5bfe258e3e184f069719f9fd008d6b8f6b9b) )
	ROM_LOAD( "ioa1",         0x0000, 0x0117, CRC(59c7ee3b) SHA1(fbb887c5b4f5cb8df77cec710eaac2985bc482a6) )
	ROM_LOAD( "prg2",         0x0000, 0x0117, CRC(4386879a) SHA1(c36896d169d8c78393609acbbe4397931292a033) )
	ROM_LOAD( "rom1",         0x0000, 0x0117, CRC(41dc73b9) SHA1(7d4c9f1693c821fbf84e32dd6ef62ddf14967845) )

	ROM_REGION( 0x0200, "bboardplds", 0 )
	ROM_LOAD( "tk263b.1a",    0x0000, 0x0117, CRC(c4b0349b) SHA1(b4873dd5ad8735048deb3475222dde3c0b67eaaf) )
	ROM_LOAD( "iob1.12d",     0x0000, 0x0117, CRC(3abc0700) SHA1(973043aa46ec6d5d1db20dc9d5937005a0f9f6ae) )
	ROM_LOAD( "bprg1.11d",    0x0000, 0x0117, CRC(31793da7) SHA1(400fa7ac517421c978c1ee7773c30b9ed0c5d3f3) )

	ROM_REGION( 0x0200, "cboardplds", 0 )
	ROM_LOAD( "ioc1.ic1",     0x0000, 0x0117, CRC(0d182081) SHA1(475b3d417785da4bc512cce2b274bb00d4cc6792) )

	ROM_REGION( 0x0200, "dboardplds", 0 )
	ROM_LOAD( "d7l1.7l",      0x0000, 0x0117, CRC(27b7410d) SHA1(06d0cba0226850f100ff1f539bd7d5db0f90c730) )
	ROM_LOAD( "d8l1.8l",      0x0000, 0x0117, CRC(539fc7da) SHA1(cad5c91629c6247e49ccbbcbfe6b08229eafae07) )
	ROM_LOAD( "d9k1.9k",      0x0000, 0x0117, NO_DUMP )                                     // pal verification required
	ROM_LOAD( "d10f1.10f",    0x0000, 0x0117, CRC(6619c494) SHA1(3aef656c07182a2186f810f30e0d854dd5bd8d18) )
ROM_END

/* B-Board 91635B-2 */
ROM_START( wofu )
	ROM_REGION( CODE_SIZE, "maincpu", 0 )      /* 68000 code */
	ROM_LOAD16_WORD_SWAP( "tk2u_23c.8f", 0x000000, 0x80000, CRC(29b89c12) SHA1(2b474b4f45a4ccb0db2a4d5e7ef30e28b5c6cc3a) )
	ROM_LOAD16_WORD_SWAP( "tk2u_22c.7f", 0x080000, 0x80000, CRC(f5af4774) SHA1(f6d53cf5b330e6d68f84da3e8c831a475585b93e) )

	ROM_REGION( 0x400000, "gfx", 0 )
	ROMX_LOAD( "tk2-1m.3a",  0x000000, 0x80000, CRC(0d9cb9bf) SHA1(cc7140e9a01a14b252cb1090bcea32b0de461928) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "tk2-3m.5a",  0x000002, 0x80000, CRC(45227027) SHA1(b21afc593f0d4d8909dfa621d659cbb40507d1b2) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "tk2-2m.4a",  0x000004, 0x80000, CRC(c5ca2460) SHA1(cbe14867f7b94b638ca80db7c8e0c60881183469) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "tk2-4m.6a",  0x000006, 0x80000, CRC(e349551c) SHA1(1d977bdf256accf750ad9930ec4a0a19bbf86964) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "tk2-5m.7a",  0x200000, 0x80000, CRC(291f0f0b) SHA1(094baf0f960f25fc2525b3b1cc378a49d9a0955d) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "tk2-7m.9a",  0x200002, 0x80000, CRC(3edeb949) SHA1(c155698dd9ee9eb24bbc97a21118ef2e897ea82f) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "tk2-6m.8a",  0x200004, 0x80000, CRC(1abd14d6) SHA1(dffff3126f102b4ec028a81405fc5b9bd7bb65b3) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "tk2-8m.10a", 0x200006, 0x80000, CRC(b27948e3) SHA1(870d5d23f56798831c641e877ea94217058b2ddc) , ROM_GROUPWORD | ROM_SKIP(6) )

	ROM_REGION( 0x28000, "audiocpu", 0 ) /* QSound Z80 code */
	ROM_LOAD( "tk2_qa.5k",   0x00000, 0x08000, CRC(c9183a0d) SHA1(d8b1d41c572f08581f8ab9eb878de77d6ea8615d) )
	ROM_CONTINUE(            0x10000, 0x18000 )

	ROM_REGION( 0x200000, "qsound", 0 ) /* QSound samples */
	ROM_LOAD( "tk2-q1.1k",   0x000000, 0x80000, CRC(611268cf) SHA1(83ab059f2110fb25fdcff928d56b790fc1f5c975) )
	ROM_LOAD( "tk2-q2.2k",   0x080000, 0x80000, CRC(20f55ca9) SHA1(90134e9a9c4749bb65c728b66ea4dac1fd4d88a4) )
	ROM_LOAD( "tk2-q3.3k",   0x100000, 0x80000, CRC(bfcf6f52) SHA1(2a85ff3fc89b4cbabd20779ec12da2e116333c7c) )
	ROM_LOAD( "tk2-q4.4k",   0x180000, 0x80000, CRC(36642e88) SHA1(8ab25b19e2b67215a5cb1f3aa81b9d26009cfeb8) )

	ROM_REGION( 0x0200, "aboardplds", 0 )
	ROM_LOAD( "buf1",         0x0000, 0x0117, CRC(eb122de7) SHA1(b26b5bfe258e3e184f069719f9fd008d6b8f6b9b) )
	ROM_LOAD( "ioa1",         0x0000, 0x0117, CRC(59c7ee3b) SHA1(fbb887c5b4f5cb8df77cec710eaac2985bc482a6) )
	ROM_LOAD( "prg2",         0x0000, 0x0117, CRC(4386879a) SHA1(c36896d169d8c78393609acbbe4397931292a033) )
	ROM_LOAD( "rom1",         0x0000, 0x0117, CRC(41dc73b9) SHA1(7d4c9f1693c821fbf84e32dd6ef62ddf14967845) )

	ROM_REGION( 0x0200, "bboardplds", 0 )
	ROM_LOAD( "tk263b.1a",    0x0000, 0x0117, CRC(c4b0349b) SHA1(b4873dd5ad8735048deb3475222dde3c0b67eaaf) )
	ROM_LOAD( "iob1.12d",     0x0000, 0x0117, CRC(3abc0700) SHA1(973043aa46ec6d5d1db20dc9d5937005a0f9f6ae) )
	ROM_LOAD( "bprg1.11d",    0x0000, 0x0117, CRC(31793da7) SHA1(400fa7ac517421c978c1ee7773c30b9ed0c5d3f3) )

	ROM_REGION( 0x0200, "cboardplds", 0 )
	ROM_LOAD( "ioc1.ic1",     0x0000, 0x0117, CRC(0d182081) SHA1(475b3d417785da4bc512cce2b274bb00d4cc6792) )

	ROM_REGION( 0x0200, "dboardplds", 0 )
	ROM_LOAD( "d7l1.7l",      0x0000, 0x0117, CRC(27b7410d) SHA1(06d0cba0226850f100ff1f539bd7d5db0f90c730) )
	ROM_LOAD( "d8l1.8l",      0x0000, 0x0117, CRC(539fc7da) SHA1(cad5c91629c6247e49ccbbcbfe6b08229eafae07) )
	ROM_LOAD( "d9k1.9k",      0x0000, 0x0117, NO_DUMP )
	ROM_LOAD( "d10f1.10f",    0x0000, 0x0117, CRC(6619c494) SHA1(3aef656c07182a2186f810f30e0d854dd5bd8d18) )
ROM_END

/* B-Board 91634B-2 */
/* Note that this Asia set seems to be the only one where GFX are stored into MASK ROMs instead of the usual EPROMs. */
/* FIXME Probably this set uses a patched program ROM coming from a desuicided board, or simply the original C-Board
   is not a 92641C-1. A verification and a new fresh dump are needed to confirm if it's genuine or not. */
ROM_START( wofa )
	ROM_REGION( CODE_SIZE, "maincpu", 0 )      /* 68000 code */
	ROM_LOAD16_WORD_SWAP( "tk2a_23c.8f", 0x000000, 0x80000, CRC(2e024628) SHA1(647f8700fe3b410d798a823bac2e4a89cc9ad8d5) )  // patched?
	ROM_LOAD16_WORD_SWAP( "tk2a_22c.7f", 0x080000, 0x80000, CRC(900ad4cd) SHA1(988007447f93f3467029b9c29fd9670a7ecadaa3) )

	ROM_REGION( 0x400000, "gfx", 0 )
	ROMX_LOAD( "tk2-1m.3a",  0x000000, 0x80000, CRC(0d9cb9bf) SHA1(cc7140e9a01a14b252cb1090bcea32b0de461928) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "tk2-3m.5a",  0x000002, 0x80000, CRC(45227027) SHA1(b21afc593f0d4d8909dfa621d659cbb40507d1b2) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "tk2-2m.4a",  0x000004, 0x80000, CRC(c5ca2460) SHA1(cbe14867f7b94b638ca80db7c8e0c60881183469) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "tk2-4m.6a",  0x000006, 0x80000, CRC(e349551c) SHA1(1d977bdf256accf750ad9930ec4a0a19bbf86964) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "tk2-5m.7a",  0x200000, 0x80000, CRC(291f0f0b) SHA1(094baf0f960f25fc2525b3b1cc378a49d9a0955d) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "tk2-7m.9a",  0x200002, 0x80000, CRC(3edeb949) SHA1(c155698dd9ee9eb24bbc97a21118ef2e897ea82f) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "tk2-6m.8a",  0x200004, 0x80000, CRC(1abd14d6) SHA1(dffff3126f102b4ec028a81405fc5b9bd7bb65b3) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "tk2-8m.10a", 0x200006, 0x80000, CRC(b27948e3) SHA1(870d5d23f56798831c641e877ea94217058b2ddc) , ROM_GROUPWORD | ROM_SKIP(6) )

	ROM_REGION( 0x28000, "audiocpu", 0 ) /* QSound Z80 code */
	ROM_LOAD( "tk2_qa.5k",   0x00000, 0x08000, CRC(c9183a0d) SHA1(d8b1d41c572f08581f8ab9eb878de77d6ea8615d) )
	ROM_CONTINUE(            0x10000, 0x18000 )

	ROM_REGION( 0x200000, "qsound", 0 ) /* QSound samples */
	ROM_LOAD( "tk2-q1.1k",   0x000000, 0x80000, CRC(611268cf) SHA1(83ab059f2110fb25fdcff928d56b790fc1f5c975) )
	ROM_LOAD( "tk2-q2.2k",   0x080000, 0x80000, CRC(20f55ca9) SHA1(90134e9a9c4749bb65c728b66ea4dac1fd4d88a4) )
	ROM_LOAD( "tk2-q3.3k",   0x100000, 0x80000, CRC(bfcf6f52) SHA1(2a85ff3fc89b4cbabd20779ec12da2e116333c7c) )
	ROM_LOAD( "tk2-q4.4k",   0x180000, 0x80000, CRC(36642e88) SHA1(8ab25b19e2b67215a5cb1f3aa81b9d26009cfeb8) )

	ROM_REGION( 0x0200, "aboardplds", 0 )
	ROM_LOAD( "buf1",         0x0000, 0x0117, CRC(eb122de7) SHA1(b26b5bfe258e3e184f069719f9fd008d6b8f6b9b) )
	ROM_LOAD( "ioa1",         0x0000, 0x0117, CRC(59c7ee3b) SHA1(fbb887c5b4f5cb8df77cec710eaac2985bc482a6) )
	ROM_LOAD( "prg2",         0x0000, 0x0117, CRC(4386879a) SHA1(c36896d169d8c78393609acbbe4397931292a033) )
	ROM_LOAD( "rom1",         0x0000, 0x0117, CRC(41dc73b9) SHA1(7d4c9f1693c821fbf84e32dd6ef62ddf14967845) )

	ROM_REGION( 0x0200, "bboardplds", 0 )
	ROM_LOAD( "tk263b.1a",    0x0000, 0x0117, CRC(c4b0349b) SHA1(b4873dd5ad8735048deb3475222dde3c0b67eaaf) )
	ROM_LOAD( "iob1.12d",     0x0000, 0x0117, CRC(3abc0700) SHA1(973043aa46ec6d5d1db20dc9d5937005a0f9f6ae) )
	ROM_LOAD( "bprg1.11d",    0x0000, 0x0117, CRC(31793da7) SHA1(400fa7ac517421c978c1ee7773c30b9ed0c5d3f3) )

	ROM_REGION( 0x0200, "cboardplds", 0 )
	ROM_LOAD( "ioc1.ic1",     0x0000, 0x0117, CRC(0d182081) SHA1(475b3d417785da4bc512cce2b274bb00d4cc6792) )

	ROM_REGION( 0x0200, "dboardplds", 0 )
	ROM_LOAD( "d7l1.7l",      0x0000, 0x0117, CRC(27b7410d) SHA1(06d0cba0226850f100ff1f539bd7d5db0f90c730) )
	ROM_LOAD( "d8l1.8l",      0x0000, 0x0117, CRC(539fc7da) SHA1(cad5c91629c6247e49ccbbcbfe6b08229eafae07) )
	ROM_LOAD( "d9k1.9k",      0x0000, 0x0117, NO_DUMP )
	ROM_LOAD( "d10f1.10f",    0x0000, 0x0117, CRC(6619c494) SHA1(3aef656c07182a2186f810f30e0d854dd5bd8d18) )
ROM_END

/* B-Board 91634B-2 */
ROM_START( wofj )
	ROM_REGION( CODE_SIZE, "maincpu", 0 )      /* 68000 code */
	ROM_LOAD16_WORD_SWAP( "tk2j_23c.8f", 0x000000, 0x80000, CRC(9b215a68) SHA1(fc83ed26441fbfb15e21b093c7a6bed44b586e51) )
	ROM_LOAD16_WORD_SWAP( "tk2j_22c.7f", 0x080000, 0x80000, CRC(b74b09ac) SHA1(3a44d6db5f51e1b5d2b43ef0ad1191da21e48427) )

	ROM_REGION( 0x400000, "gfx", 0 )
	ROMX_LOAD( "tk2_01.3a",  0x000000, 0x80000, CRC(0d9cb9bf) SHA1(cc7140e9a01a14b252cb1090bcea32b0de461928) , ROM_GROUPWORD | ROM_SKIP(6) )    // == tk2-1m.3a
	ROMX_LOAD( "tk2_02.4a",  0x000002, 0x80000, CRC(45227027) SHA1(b21afc593f0d4d8909dfa621d659cbb40507d1b2) , ROM_GROUPWORD | ROM_SKIP(6) )    // == tk2-3m.5a
	ROMX_LOAD( "tk2_03.5a",  0x000004, 0x80000, CRC(c5ca2460) SHA1(cbe14867f7b94b638ca80db7c8e0c60881183469) , ROM_GROUPWORD | ROM_SKIP(6) )    // == tk2-2m.4a
	ROMX_LOAD( "tk2_04.6a",  0x000006, 0x80000, CRC(e349551c) SHA1(1d977bdf256accf750ad9930ec4a0a19bbf86964) , ROM_GROUPWORD | ROM_SKIP(6) )    // == tk2-4m.6a
	ROMX_LOAD( "tk2_05.7a",  0x200000, 0x80000, CRC(e4a44d53) SHA1(b747679f4d63e5e62d9fd81b3120fba0401fadfb) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "tk2_06.8a",  0x200002, 0x80000, CRC(58066ba8) SHA1(c93af968e21094d020e4b2002e0c6fc0d746af0b) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "tk2_07.9a",  0x200004, 0x80000, CRC(d706568e) SHA1(7886414dc86c42e35d24b85c4bfa41a9f0c167ac) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "tk2_08.10a", 0x200006, 0x80000, CRC(d4a19a02) SHA1(ff396b1d33d9b4842140f2c6d085fe05748e3244) , ROM_GROUPWORD | ROM_SKIP(6) )

	ROM_REGION( 0x28000, "audiocpu", 0 ) /* QSound Z80 code */
	ROM_LOAD( "tk2_qa.5k",   0x00000, 0x08000, CRC(c9183a0d) SHA1(d8b1d41c572f08581f8ab9eb878de77d6ea8615d) )
	ROM_CONTINUE(            0x10000, 0x18000 )

	ROM_REGION( 0x200000, "qsound", 0 ) /* QSound samples */
	ROM_LOAD( "tk2-q1.1k",   0x000000, 0x80000, CRC(611268cf) SHA1(83ab059f2110fb25fdcff928d56b790fc1f5c975) )
	ROM_LOAD( "tk2-q2.2k",   0x080000, 0x80000, CRC(20f55ca9) SHA1(90134e9a9c4749bb65c728b66ea4dac1fd4d88a4) )
	ROM_LOAD( "tk2-q3.3k",   0x100000, 0x80000, CRC(bfcf6f52) SHA1(2a85ff3fc89b4cbabd20779ec12da2e116333c7c) )
	ROM_LOAD( "tk2-q4.4k",   0x180000, 0x80000, CRC(36642e88) SHA1(8ab25b19e2b67215a5cb1f3aa81b9d26009cfeb8) )

	ROM_REGION( 0x0200, "aboardplds", 0 )
	ROM_LOAD( "buf1",         0x0000, 0x0117, CRC(eb122de7) SHA1(b26b5bfe258e3e184f069719f9fd008d6b8f6b9b) )
	ROM_LOAD( "ioa1",         0x0000, 0x0117, CRC(59c7ee3b) SHA1(fbb887c5b4f5cb8df77cec710eaac2985bc482a6) )
	ROM_LOAD( "prg2",         0x0000, 0x0117, CRC(4386879a) SHA1(c36896d169d8c78393609acbbe4397931292a033) )
	ROM_LOAD( "rom1",         0x0000, 0x0117, CRC(41dc73b9) SHA1(7d4c9f1693c821fbf84e32dd6ef62ddf14967845) )

	ROM_REGION( 0x0200, "bboardplds", 0 )
	ROM_LOAD( "tk263b.1a",    0x0000, 0x0117, CRC(c4b0349b) SHA1(b4873dd5ad8735048deb3475222dde3c0b67eaaf) )
	ROM_LOAD( "iob1.12d",     0x0000, 0x0117, CRC(3abc0700) SHA1(973043aa46ec6d5d1db20dc9d5937005a0f9f6ae) )
	ROM_LOAD( "bprg1.11d",    0x0000, 0x0117, CRC(31793da7) SHA1(400fa7ac517421c978c1ee7773c30b9ed0c5d3f3) )

	ROM_REGION( 0x0200, "cboardplds", 0 )
	ROM_LOAD( "ioc1.ic1",     0x0000, 0x0117, CRC(0d182081) SHA1(475b3d417785da4bc512cce2b274bb00d4cc6792) )

	ROM_REGION( 0x0200, "dboardplds", 0 )
	ROM_LOAD( "d7l1.7l",      0x0000, 0x0117, CRC(27b7410d) SHA1(06d0cba0226850f100ff1f539bd7d5db0f90c730) )
	ROM_LOAD( "d8l1.8l",      0x0000, 0x0117, CRC(539fc7da) SHA1(cad5c91629c6247e49ccbbcbfe6b08229eafae07) )
	ROM_LOAD( "d9k1.9k",      0x0000, 0x0117, NO_DUMP )                                     // pal verification required
	ROM_LOAD( "d10f1.10f",    0x0000, 0x0117, CRC(6619c494) SHA1(3aef656c07182a2186f810f30e0d854dd5bd8d18) )
ROM_END

/* Chinese bootleg board without QSound */
ROM_START( wofhfh )
	ROM_REGION( CODE_SIZE, "maincpu", 0 )      /* 68000 code */
	ROM_LOAD16_WORD_SWAP( "23",       0x000000, 0x80000, CRC(6ae4b312) SHA1(fa39f69385d180d90bccd8c5dc9262edd04a6457) )
	ROM_LOAD16_WORD_SWAP( "22",       0x080000, 0x80000, CRC(94e8d01a) SHA1(875763f6b22734c1a5a890e6c8063515c134045b) )

	ROM_REGION( 0x400000, "gfx", 0 )
	ROMX_LOAD( "1",   0x000000, 0x80000, CRC(0d9cb9bf) SHA1(cc7140e9a01a14b252cb1090bcea32b0de461928) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "2",   0x000002, 0x80000, CRC(45227027) SHA1(b21afc593f0d4d8909dfa621d659cbb40507d1b2) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "3",   0x000004, 0x80000, CRC(c5ca2460) SHA1(cbe14867f7b94b638ca80db7c8e0c60881183469) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "4",   0x000006, 0x80000, CRC(e349551c) SHA1(1d977bdf256accf750ad9930ec4a0a19bbf86964) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "5",   0x200000, 0x80000, CRC(34949d7b) SHA1(90925a77b08c97cfdbf0dbfbdaa359d1b33b6ae4) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "6",   0x200002, 0x80000, CRC(dfa70971) SHA1(477b99687de38220f0aec9fbba44db03f72cb62a) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "7",   0x200004, 0x80000, CRC(073686a6) SHA1(b774a8d4c6cdbedb123ac01455f718305f23b619) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "8",   0x200006, 0x80000, CRC(5300f8db) SHA1(b23a19910f680d60ff8afcbc15c471e74ee3569a) , ROM_GROUPWORD | ROM_SKIP(6) )

	ROM_REGION( 0x28000, "audiocpu", 0 ) /* 128k for the audio CPU (+banks) */
	ROM_LOAD( "9",              0x00000, 0x08000, CRC(86fe8a97) SHA1(cab82bcd0f49bcb40201b439cfdd10266f46752a) )
	ROM_CONTINUE(               0x10000, 0x18000 )

	ROM_REGION( 0x40000, "oki", 0 ) /* samples */
	ROM_LOAD( "18",             0x00000, 0x20000, CRC(c04be720) SHA1(2e544e0a0358b6afbdf826d35d9c4c59e4787a93) )
	ROM_LOAD( "19",             0x20000, 0x20000, CRC(fbb8d8c1) SHA1(8a7689bb7ed56243333133cbacf01a0ae825201e) )
ROM_END

/* B-Board 91635B-2 */
ROM_START( sf2hf )
	ROM_REGION( CODE_SIZE, "maincpu", 0 )      /* 68000 code */
	ROM_LOAD16_WORD_SWAP( "s2te_23.8f", 0x000000, 0x80000, CRC(2dd72514) SHA1(4411353c389669299c27ac183c7e1caa3d4cec90) )
	ROM_LOAD16_WORD_SWAP( "s2te_22.7f", 0x080000, 0x80000, CRC(aea6e035) SHA1(ce5fe961b2c1c95d231d1235bfc03b47de489f2a) )
	ROM_LOAD16_WORD_SWAP( "s2te_21.6f", 0x100000, 0x80000, CRC(fd200288) SHA1(3817b67ab77c7b3d4a573a63f18671bea6905e26) )

	ROM_REGION( 0x600000, "gfx", 0 )
	ROMX_LOAD( "s92-1m.3a",  0x000000, 0x80000, CRC(03b0d852) SHA1(f370f25c96ad2b94f8c53d6b7139100285a25bef) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "s92-3m.5a",  0x000002, 0x80000, CRC(840289ec) SHA1(2fb42a242f60ba7e74009b5a90eb26e035ba1e82) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "s92-2m.4a",  0x000004, 0x80000, CRC(cdb5f027) SHA1(4c7d944fef200fdfcaf57758b901b5511188ed2e) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "s92-4m.6a",  0x000006, 0x80000, CRC(e2799472) SHA1(27d3796429338d82a8de246a0ea06dd487a87768) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "s92-5m.7a",  0x200000, 0x80000, CRC(ba8a2761) SHA1(4b696d66c51611e43522bed752654314e76d33b6) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "s92-7m.9a",  0x200002, 0x80000, CRC(e584bfb5) SHA1(ebdf1f5e2638eed3a65dda82b1ed9151a355f4c9) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "s92-6m.8a",  0x200004, 0x80000, CRC(21e3f87d) SHA1(4a4961bb68c3a1ce15f9d393d9c03ecb2466cc29) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "s92-8m.10a", 0x200006, 0x80000, CRC(befc47df) SHA1(520390420da3a0271ba90b0a933e65143265e5cf) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "s92-10m.3c", 0x400000, 0x80000, CRC(960687d5) SHA1(2868c31121b1c7564e9767b9a19cdbf655c7ed1d) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "s92-12m.5c", 0x400002, 0x80000, CRC(978ecd18) SHA1(648a59706b93c84b4206a968ecbdc3e834c476f6) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "s92-11m.4c", 0x400004, 0x80000, CRC(d6ec9a0a) SHA1(ed6143f8737013b6ef1684e37c05e037e7a80dae) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "s92-13m.6c", 0x400006, 0x80000, CRC(ed2c67f6) SHA1(0083c0ffaf6fe7659ff0cf822be4346cd6e61329) , ROM_GROUPWORD | ROM_SKIP(6) )

	ROM_REGION( 0x18000, "audiocpu", 0 ) /* 64k for the audio CPU (+banks) */
	ROM_LOAD( "s92_09.11a",  0x00000, 0x08000, CRC(08f6b60e) SHA1(8258fcaca4ac419312531eec67079b97f471179c) )
	ROM_CONTINUE(            0x10000, 0x08000 )

	ROM_REGION( 0x40000, "oki", 0 ) /* Samples */
	ROM_LOAD( "s92_18.11c",  0x00000, 0x20000, CRC(7f162009) SHA1(346bf42992b4c36c593e21901e22c87ae4a7d86d) )
	ROM_LOAD( "s92_19.12c",  0x20000, 0x20000, CRC(beade53f) SHA1(277c397dc12752719ec6b47d2224750bd1c07f79) )

	ROM_REGION( 0x0200, "aboardplds", 0 )
	ROM_LOAD( "buf1",         0x0000, 0x0117, CRC(eb122de7) SHA1(b26b5bfe258e3e184f069719f9fd008d6b8f6b9b) )
	ROM_LOAD( "ioa1",         0x0000, 0x0117, CRC(59c7ee3b) SHA1(fbb887c5b4f5cb8df77cec710eaac2985bc482a6) )
	ROM_LOAD( "prg1",         0x0000, 0x0117, CRC(f1129744) SHA1(a5300f301c1a08a7da768f0773fa0fe3f683b237) )
	ROM_LOAD( "rom1",         0x0000, 0x0117, CRC(41dc73b9) SHA1(7d4c9f1693c821fbf84e32dd6ef62ddf14967845) )
	ROM_LOAD( "sou1",         0x0000, 0x0117, CRC(84f4b2fe) SHA1(dcc9e86cc36316fe42eace02d6df75d08bc8bb6d) )

	ROM_REGION( 0x0200, "bboardplds", 0 )
	ROM_LOAD( "s9263b.1a",    0x0000, 0x0117, CRC(0a7ecfe0) SHA1(f75e7eed4604fcf68273197fe3dd7f0d7a313ada) )
	ROM_LOAD( "iob1.12d",     0x0000, 0x0117, CRC(3abc0700) SHA1(973043aa46ec6d5d1db20dc9d5937005a0f9f6ae) )
	ROM_LOAD( "bprg1.11d",    0x0000, 0x0117, CRC(31793da7) SHA1(400fa7ac517421c978c1ee7773c30b9ed0c5d3f3) )

	ROM_REGION( 0x0200, "cboardplds", 0 )
	ROM_LOAD( "ioc1.ic7",     0x0000, 0x0117, CRC(0d182081) SHA1(475b3d417785da4bc512cce2b274bb00d4cc6792) )
	ROM_LOAD( "c632.ic1",     0x0000, 0x0117, CRC(0fbd9270) SHA1(d7e737b20c44d41e29ca94be56114b31934dde81) )
ROM_END

/* B-Board 91635B-2 */
ROM_START( sf2hfu )
	ROM_REGION( CODE_SIZE, "maincpu", 0 )      /* 68000 code */
	ROM_LOAD16_WORD_SWAP( "s2tu_23.8f", 0x000000, 0x80000, CRC(89a1fc38) SHA1(aafb40fc311e318250973be8c6aa0d3f7902cb3c) )
	ROM_LOAD16_WORD_SWAP( "s2tu_22.7f", 0x080000, 0x80000, CRC(aea6e035) SHA1(ce5fe961b2c1c95d231d1235bfc03b47de489f2a) )   // == s2te_22.7f
	ROM_LOAD16_WORD_SWAP( "s2tu_21.6f", 0x100000, 0x80000, CRC(fd200288) SHA1(3817b67ab77c7b3d4a573a63f18671bea6905e26) )   // == s2te_21.6f

	ROM_REGION( 0x600000, "gfx", 0 )
	ROMX_LOAD( "s92-1m.3a",  0x000000, 0x80000, CRC(03b0d852) SHA1(f370f25c96ad2b94f8c53d6b7139100285a25bef) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "s92-3m.5a",  0x000002, 0x80000, CRC(840289ec) SHA1(2fb42a242f60ba7e74009b5a90eb26e035ba1e82) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "s92-2m.4a",  0x000004, 0x80000, CRC(cdb5f027) SHA1(4c7d944fef200fdfcaf57758b901b5511188ed2e) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "s92-4m.6a",  0x000006, 0x80000, CRC(e2799472) SHA1(27d3796429338d82a8de246a0ea06dd487a87768) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "s92-5m.7a",  0x200000, 0x80000, CRC(ba8a2761) SHA1(4b696d66c51611e43522bed752654314e76d33b6) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "s92-7m.9a",  0x200002, 0x80000, CRC(e584bfb5) SHA1(ebdf1f5e2638eed3a65dda82b1ed9151a355f4c9) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "s92-6m.8a",  0x200004, 0x80000, CRC(21e3f87d) SHA1(4a4961bb68c3a1ce15f9d393d9c03ecb2466cc29) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "s92-8m.10a", 0x200006, 0x80000, CRC(befc47df) SHA1(520390420da3a0271ba90b0a933e65143265e5cf) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "s92-10m.3c", 0x400000, 0x80000, CRC(960687d5) SHA1(2868c31121b1c7564e9767b9a19cdbf655c7ed1d) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "s92-12m.5c", 0x400002, 0x80000, CRC(978ecd18) SHA1(648a59706b93c84b4206a968ecbdc3e834c476f6) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "s92-11m.4c", 0x400004, 0x80000, CRC(d6ec9a0a) SHA1(ed6143f8737013b6ef1684e37c05e037e7a80dae) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "s92-13m.6c", 0x400006, 0x80000, CRC(ed2c67f6) SHA1(0083c0ffaf6fe7659ff0cf822be4346cd6e61329) , ROM_GROUPWORD | ROM_SKIP(6) )

	ROM_REGION( 0x18000, "audiocpu", 0 ) /* 64k for the audio CPU (+banks) */
	ROM_LOAD( "s92_09.11a",  0x00000, 0x08000, CRC(08f6b60e) SHA1(8258fcaca4ac419312531eec67079b97f471179c) )
	ROM_CONTINUE(            0x10000, 0x08000 )

	ROM_REGION( 0x40000, "oki", 0 ) /* Samples */
	ROM_LOAD( "s92_18.11c",  0x00000, 0x20000, CRC(7f162009) SHA1(346bf42992b4c36c593e21901e22c87ae4a7d86d) )
	ROM_LOAD( "s92_19.12c",  0x20000, 0x20000, CRC(beade53f) SHA1(277c397dc12752719ec6b47d2224750bd1c07f79) )

	ROM_REGION( 0x0200, "aboardplds", 0 )
	ROM_LOAD( "buf1",         0x0000, 0x0117, CRC(eb122de7) SHA1(b26b5bfe258e3e184f069719f9fd008d6b8f6b9b) )
	ROM_LOAD( "ioa1",         0x0000, 0x0117, CRC(59c7ee3b) SHA1(fbb887c5b4f5cb8df77cec710eaac2985bc482a6) )
	ROM_LOAD( "prg1",         0x0000, 0x0117, CRC(f1129744) SHA1(a5300f301c1a08a7da768f0773fa0fe3f683b237) )
	ROM_LOAD( "rom1",         0x0000, 0x0117, CRC(41dc73b9) SHA1(7d4c9f1693c821fbf84e32dd6ef62ddf14967845) )
	ROM_LOAD( "sou1",         0x0000, 0x0117, CRC(84f4b2fe) SHA1(dcc9e86cc36316fe42eace02d6df75d08bc8bb6d) )

	ROM_REGION( 0x0200, "bboardplds", 0 )
	ROM_LOAD( "s9263b.1a",    0x0000, 0x0117, CRC(0a7ecfe0) SHA1(f75e7eed4604fcf68273197fe3dd7f0d7a313ada) )
	ROM_LOAD( "iob1.12d",     0x0000, 0x0117, CRC(3abc0700) SHA1(973043aa46ec6d5d1db20dc9d5937005a0f9f6ae) )
	ROM_LOAD( "bprg1.11d",    0x0000, 0x0117, CRC(31793da7) SHA1(400fa7ac517421c978c1ee7773c30b9ed0c5d3f3) )

	ROM_REGION( 0x0200, "cboardplds", 0 )
	ROM_LOAD( "ioc1.ic7",     0x0000, 0x0117, CRC(0d182081) SHA1(475b3d417785da4bc512cce2b274bb00d4cc6792) )
	ROM_LOAD( "c632.ic1",     0x0000, 0x0117, CRC(0fbd9270) SHA1(d7e737b20c44d41e29ca94be56114b31934dde81) )
ROM_END

/* B-Board 91634B-2 */
ROM_START( sf2hfj )
	ROM_REGION( CODE_SIZE, "maincpu", 0 )      /* 68000 code */
	ROM_LOAD16_WORD_SWAP( "s2tj_23.8f", 0x000000, 0x80000, CRC(ea73b4dc) SHA1(efbc73277d00bac86505755db35225e14ea25a36) )
	ROM_LOAD16_WORD_SWAP( "s2tj_22.7f", 0x080000, 0x80000, CRC(aea6e035) SHA1(ce5fe961b2c1c95d231d1235bfc03b47de489f2a) )   // == s2te_22.7f
	ROM_LOAD16_WORD_SWAP( "s2tj_21.6f", 0x100000, 0x80000, CRC(fd200288) SHA1(3817b67ab77c7b3d4a573a63f18671bea6905e26) )   // == s2te_21.6f

	ROM_REGION( 0x600000, "gfx", 0 )
	ROMX_LOAD( "s92_01.3a",  0x000000, 0x80000, CRC(03b0d852) SHA1(f370f25c96ad2b94f8c53d6b7139100285a25bef) , ROM_GROUPWORD | ROM_SKIP(6) )    // == s92-1m.3a
	ROMX_LOAD( "s92_02.4a",  0x000002, 0x80000, CRC(840289ec) SHA1(2fb42a242f60ba7e74009b5a90eb26e035ba1e82) , ROM_GROUPWORD | ROM_SKIP(6) )    // == s92-3m.5a
	ROMX_LOAD( "s92_03.5a",  0x000004, 0x80000, CRC(cdb5f027) SHA1(4c7d944fef200fdfcaf57758b901b5511188ed2e) , ROM_GROUPWORD | ROM_SKIP(6) )    // == s92-2m.4a
	ROMX_LOAD( "s92_04.6a",  0x000006, 0x80000, CRC(e2799472) SHA1(27d3796429338d82a8de246a0ea06dd487a87768) , ROM_GROUPWORD | ROM_SKIP(6) )    // == s92-4m.6a
	ROMX_LOAD( "s92_05.7a",  0x200000, 0x80000, CRC(ba8a2761) SHA1(4b696d66c51611e43522bed752654314e76d33b6) , ROM_GROUPWORD | ROM_SKIP(6) )    // == s92-5m.7a
	ROMX_LOAD( "s92_06.8a",  0x200002, 0x80000, CRC(e584bfb5) SHA1(ebdf1f5e2638eed3a65dda82b1ed9151a355f4c9) , ROM_GROUPWORD | ROM_SKIP(6) )    // == s92-7m.9a
	ROMX_LOAD( "s92_07.9a",  0x200004, 0x80000, CRC(21e3f87d) SHA1(4a4961bb68c3a1ce15f9d393d9c03ecb2466cc29) , ROM_GROUPWORD | ROM_SKIP(6) )    // == s92-6m.8a
	ROMX_LOAD( "s92_08.10a", 0x200006, 0x80000, CRC(befc47df) SHA1(520390420da3a0271ba90b0a933e65143265e5cf) , ROM_GROUPWORD | ROM_SKIP(6) )    // == s92-8m.10a
	ROMX_LOAD( "s2t_10.3c",  0x400000, 0x80000, CRC(3c042686) SHA1(307e1ca8ad0b11f3265b7e5467ba4c90f90ec97f) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "s2t_11.4c",  0x400002, 0x80000, CRC(8b7e7183) SHA1(c8eaedfbddbf0b83311d2dbb9e19a1efef0dffa9) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "s2t_12.5c",  0x400004, 0x80000, CRC(293c888c) SHA1(5992ea9aa90fdd8b9dacca9d2a1fdaf25ac2cb65) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "s2t_13.6c",  0x400006, 0x80000, CRC(842b35a4) SHA1(35864a140a0c8d76501e69b2e01bc4ad76f27909) , ROM_GROUPWORD | ROM_SKIP(6) )

	ROM_REGION( 0x18000, "audiocpu", 0 ) /* 64k for the audio CPU (+banks) */
	ROM_LOAD( "s92_09.12a",  0x00000, 0x08000, CRC(08f6b60e) SHA1(8258fcaca4ac419312531eec67079b97f471179c) )   // == s92_09.11a
	ROM_CONTINUE(            0x10000, 0x08000 )

	ROM_REGION( 0x40000, "oki", 0 ) /* Samples */
	ROM_LOAD( "s92_18.11c",  0x00000, 0x20000, CRC(7f162009) SHA1(346bf42992b4c36c593e21901e22c87ae4a7d86d) )
	ROM_LOAD( "s92_19.12c",  0x20000, 0x20000, CRC(beade53f) SHA1(277c397dc12752719ec6b47d2224750bd1c07f79) )

	ROM_REGION( 0x0200, "aboardplds", 0 )
	ROM_LOAD( "buf1",         0x0000, 0x0117, CRC(eb122de7) SHA1(b26b5bfe258e3e184f069719f9fd008d6b8f6b9b) )
	ROM_LOAD( "ioa1",         0x0000, 0x0117, CRC(59c7ee3b) SHA1(fbb887c5b4f5cb8df77cec710eaac2985bc482a6) )
	ROM_LOAD( "prg1",         0x0000, 0x0117, CRC(f1129744) SHA1(a5300f301c1a08a7da768f0773fa0fe3f683b237) )
	ROM_LOAD( "rom1",         0x0000, 0x0117, CRC(41dc73b9) SHA1(7d4c9f1693c821fbf84e32dd6ef62ddf14967845) )
	ROM_LOAD( "sou1",         0x0000, 0x0117, CRC(84f4b2fe) SHA1(dcc9e86cc36316fe42eace02d6df75d08bc8bb6d) )

	ROM_REGION( 0x0200, "bboardplds", 0 )
	ROM_LOAD( "s9263b.1a",    0x0000, 0x0117, CRC(0a7ecfe0) SHA1(f75e7eed4604fcf68273197fe3dd7f0d7a313ada) )
	ROM_LOAD( "iob1.12d",     0x0000, 0x0117, CRC(3abc0700) SHA1(973043aa46ec6d5d1db20dc9d5937005a0f9f6ae) )
	ROM_LOAD( "bprg1.11d",    0x0000, 0x0117, CRC(31793da7) SHA1(400fa7ac517421c978c1ee7773c30b9ed0c5d3f3) )

	ROM_REGION( 0x0200, "cboardplds", 0 )
	ROM_LOAD( "ioc1.ic7",     0x0000, 0x0117, CRC(0d182081) SHA1(475b3d417785da4bc512cce2b274bb00d4cc6792) )
	ROM_LOAD( "c632.ic1",     0x0000, 0x0117, CRC(0fbd9270) SHA1(d7e737b20c44d41e29ca94be56114b31934dde81) )
ROM_END

/* B-Board 91635B-2 */
ROM_START( dino )
	ROM_REGION( CODE_SIZE, "maincpu", 0 )      /* 68000 code */
	ROM_LOAD16_WORD_SWAP( "cde_23a.8f", 0x000000, 0x80000, CRC(8f4e585e) SHA1(55ecba3652066cdafad140c4524b1fc81228e69b) )
	ROM_LOAD16_WORD_SWAP( "cde_22a.7f", 0x080000, 0x80000, CRC(9278aa12) SHA1(58cbbd53a98abe640ccb233f8dbd8ca6d63475e7) )
	ROM_LOAD16_WORD_SWAP( "cde_21a.6f", 0x100000, 0x80000, CRC(66d23de2) SHA1(19b8a365f630411d524d055459020f4c8cf930f1) )

	ROM_REGION( 0x400000, "gfx", 0 )
	ROMX_LOAD( "cd-1m.3a",  0x000000, 0x80000, CRC(8da4f917) SHA1(4f7b2304b7d9b545d6707d7ec921d3e28200699d) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "cd-3m.5a",  0x000002, 0x80000, CRC(6c40f603) SHA1(cdbd11dfcec08e87355d7e21e9fd39f7eacab016) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "cd-2m.4a",  0x000004, 0x80000, CRC(09c8fc2d) SHA1(d0c0a1258ec5dd484ab6ec1c5663425431f929ee) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "cd-4m.6a",  0x000006, 0x80000, CRC(637ff38f) SHA1(859926b33b9955b3ed67471c61faa442d42b9696) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "cd-5m.7a",  0x200000, 0x80000, CRC(470befee) SHA1(a42e38319e9b7424381352512f11bd8edf0bbb96) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "cd-7m.9a",  0x200002, 0x80000, CRC(22bfb7a3) SHA1(c44959bd3d42b9fc8ecb482dfaf63fbd469d2c3e) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "cd-6m.8a",  0x200004, 0x80000, CRC(e7599ac4) SHA1(0e788a38547a8701115d01190ddeaca64388db4d) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "cd-8m.10a", 0x200006, 0x80000, CRC(211b4b15) SHA1(374f6b185faa0f14f5c45b9b1d60d0772d93fb17) , ROM_GROUPWORD | ROM_SKIP(6) )

	ROM_REGION( 0x28000, "audiocpu", 0 ) /* QSound Z80 code */
	ROM_LOAD( "cd_q.5k",    0x00000, 0x08000, CRC(605fdb0b) SHA1(9da90ddc6513aaaf2260f0c69719c6b0e585ba8c) )
	ROM_CONTINUE(           0x10000, 0x18000 )

	ROM_REGION( 0x200000, "qsound", 0 ) /* QSound samples */
	ROM_LOAD( "cd-q1.1k",   0x000000, 0x80000, CRC(60927775) SHA1(f8599bc84c38573ebbe8685822c58b6a38b50462) )
	ROM_LOAD( "cd-q2.2k",   0x080000, 0x80000, CRC(770f4c47) SHA1(fec8ef00a6669d4d5e37787ecc7b58ee46709326) )
	ROM_LOAD( "cd-q3.3k",   0x100000, 0x80000, CRC(2f273ffc) SHA1(f0de462f6c4d251911258e0ebd886152c14d1586) )
	ROM_LOAD( "cd-q4.4k",   0x180000, 0x80000, CRC(2c67821d) SHA1(6e2528d0b22508300a6a142a796dd3bf53a66946) )

	ROM_REGION( 0x0200, "aboardplds", 0 )
	ROM_LOAD( "buf1",         0x0000, 0x0117, CRC(eb122de7) SHA1(b26b5bfe258e3e184f069719f9fd008d6b8f6b9b) )
	ROM_LOAD( "ioa1",         0x0000, 0x0117, CRC(59c7ee3b) SHA1(fbb887c5b4f5cb8df77cec710eaac2985bc482a6) )
	ROM_LOAD( "prg2",         0x0000, 0x0117, CRC(4386879a) SHA1(c36896d169d8c78393609acbbe4397931292a033) )
	ROM_LOAD( "rom1",         0x0000, 0x0117, CRC(41dc73b9) SHA1(7d4c9f1693c821fbf84e32dd6ef62ddf14967845) )

	ROM_REGION( 0x0200, "bboardplds", 0 )
	ROM_LOAD( "cd63b.1a",     0x0000, 0x0117, CRC(ef72e902) SHA1(82fea3f63869c245d0dce2809085208fe719b57a) )
	ROM_LOAD( "iob1.12d",     0x0000, 0x0117, CRC(3abc0700) SHA1(973043aa46ec6d5d1db20dc9d5937005a0f9f6ae) )
	ROM_LOAD( "bprg1.11d",    0x0000, 0x0117, CRC(31793da7) SHA1(400fa7ac517421c978c1ee7773c30b9ed0c5d3f3) )

	ROM_REGION( 0x0200, "cboardplds", 0 )
	ROM_LOAD( "ioc1.ic1",     0x0000, 0x0117, CRC(0d182081) SHA1(475b3d417785da4bc512cce2b274bb00d4cc6792) )

	ROM_REGION( 0x0200, "dboardplds", 0 )
	ROM_LOAD( "d7l1.7l",      0x0000, 0x0117, CRC(27b7410d) SHA1(06d0cba0226850f100ff1f539bd7d5db0f90c730) )
	ROM_LOAD( "d8l1.8l",      0x0000, 0x0117, CRC(539fc7da) SHA1(cad5c91629c6247e49ccbbcbfe6b08229eafae07) )
	ROM_LOAD( "d9k2.9k",      0x0000, 0x0117, CRC(cd85a156) SHA1(a88f8939c5d93e65d7bcc0eb3ee5b6f4f1114e3a) )
	ROM_LOAD( "d10f1.10f",    0x0000, 0x0117, CRC(6619c494) SHA1(3aef656c07182a2186f810f30e0d854dd5bd8d18) )
ROM_END

/* B-Board 91635B-2 */
ROM_START( dinou )
	ROM_REGION( CODE_SIZE, "maincpu", 0 )      /* 68000 code */
	ROM_LOAD16_WORD_SWAP( "cdu_23a.8f", 0x000000, 0x80000, CRC(7c2543cd) SHA1(6b7a90392fe4c31b2d57620b0ddcb3412401efc3) )
	ROM_LOAD16_WORD_SWAP( "cdu_22a.7f", 0x080000, 0x80000, CRC(d19f981e) SHA1(acb951caba3867c21149286185b94beb37721bd2) )
	ROM_LOAD16_WORD_SWAP( "cdu_21a.6f", 0x100000, 0x80000, CRC(66d23de2) SHA1(19b8a365f630411d524d055459020f4c8cf930f1) )   // == cde_21a.6f

	ROM_REGION( 0x400000, "gfx", 0 )
	ROMX_LOAD( "cd-1m.3a",  0x000000, 0x80000, CRC(8da4f917) SHA1(4f7b2304b7d9b545d6707d7ec921d3e28200699d) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "cd-3m.5a",  0x000002, 0x80000, CRC(6c40f603) SHA1(cdbd11dfcec08e87355d7e21e9fd39f7eacab016) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "cd-2m.4a",  0x000004, 0x80000, CRC(09c8fc2d) SHA1(d0c0a1258ec5dd484ab6ec1c5663425431f929ee) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "cd-4m.6a",  0x000006, 0x80000, CRC(637ff38f) SHA1(859926b33b9955b3ed67471c61faa442d42b9696) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "cd-5m.7a",  0x200000, 0x80000, CRC(470befee) SHA1(a42e38319e9b7424381352512f11bd8edf0bbb96) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "cd-7m.9a",  0x200002, 0x80000, CRC(22bfb7a3) SHA1(c44959bd3d42b9fc8ecb482dfaf63fbd469d2c3e) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "cd-6m.8a",  0x200004, 0x80000, CRC(e7599ac4) SHA1(0e788a38547a8701115d01190ddeaca64388db4d) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "cd-8m.10a", 0x200006, 0x80000, CRC(211b4b15) SHA1(374f6b185faa0f14f5c45b9b1d60d0772d93fb17) , ROM_GROUPWORD | ROM_SKIP(6) )

	ROM_REGION( 0x28000, "audiocpu", 0 ) /* QSound Z80 code */
	ROM_LOAD( "cd_q.5k",    0x00000, 0x08000, CRC(605fdb0b) SHA1(9da90ddc6513aaaf2260f0c69719c6b0e585ba8c) )
	ROM_CONTINUE(           0x10000, 0x18000 )

	ROM_REGION( 0x200000, "qsound", 0 ) /* QSound samples */
	ROM_LOAD( "cd-q1.1k",   0x000000, 0x80000, CRC(60927775) SHA1(f8599bc84c38573ebbe8685822c58b6a38b50462) )
	ROM_LOAD( "cd-q2.2k",   0x080000, 0x80000, CRC(770f4c47) SHA1(fec8ef00a6669d4d5e37787ecc7b58ee46709326) )
	ROM_LOAD( "cd-q3.3k",   0x100000, 0x80000, CRC(2f273ffc) SHA1(f0de462f6c4d251911258e0ebd886152c14d1586) )
	ROM_LOAD( "cd-q4.4k",   0x180000, 0x80000, CRC(2c67821d) SHA1(6e2528d0b22508300a6a142a796dd3bf53a66946) )

	ROM_REGION( 0x0200, "aboardplds", 0 )
	ROM_LOAD( "buf1",         0x0000, 0x0117, CRC(eb122de7) SHA1(b26b5bfe258e3e184f069719f9fd008d6b8f6b9b) )
	ROM_LOAD( "ioa1",         0x0000, 0x0117, CRC(59c7ee3b) SHA1(fbb887c5b4f5cb8df77cec710eaac2985bc482a6) )
	ROM_LOAD( "prg2",         0x0000, 0x0117, CRC(4386879a) SHA1(c36896d169d8c78393609acbbe4397931292a033) )
	ROM_LOAD( "rom1",         0x0000, 0x0117, CRC(41dc73b9) SHA1(7d4c9f1693c821fbf84e32dd6ef62ddf14967845) )

	ROM_REGION( 0x0200, "bboardplds", 0 )
	ROM_LOAD( "cd63b.1a",     0x0000, 0x0117, CRC(ef72e902) SHA1(82fea3f63869c245d0dce2809085208fe719b57a) )
	ROM_LOAD( "iob1.12d",     0x0000, 0x0117, CRC(3abc0700) SHA1(973043aa46ec6d5d1db20dc9d5937005a0f9f6ae) )
	ROM_LOAD( "bprg1.11d",    0x0000, 0x0117, CRC(31793da7) SHA1(400fa7ac517421c978c1ee7773c30b9ed0c5d3f3) )

	ROM_REGION( 0x0200, "cboardplds", 0 )
	ROM_LOAD( "ioc1.ic1",     0x0000, 0x0117, CRC(0d182081) SHA1(475b3d417785da4bc512cce2b274bb00d4cc6792) )

	ROM_REGION( 0x0200, "dboardplds", 0 )
	ROM_LOAD( "d7l1.7l",      0x0000, 0x0117, CRC(27b7410d) SHA1(06d0cba0226850f100ff1f539bd7d5db0f90c730) )
	ROM_LOAD( "d8l1.8l",      0x0000, 0x0117, CRC(539fc7da) SHA1(cad5c91629c6247e49ccbbcbfe6b08229eafae07) )
	ROM_LOAD( "d9k2.9k",      0x0000, 0x0117, CRC(cd85a156) SHA1(a88f8939c5d93e65d7bcc0eb3ee5b6f4f1114e3a) )
	ROM_LOAD( "d10f1.10f",    0x0000, 0x0117, CRC(6619c494) SHA1(3aef656c07182a2186f810f30e0d854dd5bd8d18) )
ROM_END

/* B-Board 91634B-2 */
ROM_START( dinoj )
	ROM_REGION( CODE_SIZE, "maincpu", 0 )      /* 68000 code */
	ROM_LOAD16_WORD_SWAP( "cdj_23a.8f", 0x000000, 0x80000, CRC(5f3ece96) SHA1(33ffb08ff8c5d3bfb2fa17fa00f254da2fc61f44) )
	ROM_LOAD16_WORD_SWAP( "cdj_22a.7f", 0x080000, 0x80000, CRC(a0d8de29) SHA1(79d916f181804b6176581efe2a1b7f210ec79c07) )
	ROM_LOAD16_WORD_SWAP( "cdj_21a.6f", 0x100000, 0x80000, CRC(66d23de2) SHA1(19b8a365f630411d524d055459020f4c8cf930f1) )   // == cde_21a.6f

	ROM_REGION( 0x400000, "gfx", 0 )
	ROMX_LOAD( "cd_01.3a",  0x000000, 0x80000, CRC(8da4f917) SHA1(4f7b2304b7d9b545d6707d7ec921d3e28200699d) , ROM_GROUPWORD | ROM_SKIP(6) ) // == cd-1m.3a
	ROMX_LOAD( "cd_02.4a",  0x000002, 0x80000, CRC(6c40f603) SHA1(cdbd11dfcec08e87355d7e21e9fd39f7eacab016) , ROM_GROUPWORD | ROM_SKIP(6) ) // == cd-3m.5a
	ROMX_LOAD( "cd_03.5a",  0x000004, 0x80000, CRC(09c8fc2d) SHA1(d0c0a1258ec5dd484ab6ec1c5663425431f929ee) , ROM_GROUPWORD | ROM_SKIP(6) ) // == cd-2m.4a
	ROMX_LOAD( "cd_04.6a",  0x000006, 0x80000, CRC(637ff38f) SHA1(859926b33b9955b3ed67471c61faa442d42b9696) , ROM_GROUPWORD | ROM_SKIP(6) ) // == cd-4m.6a
	ROMX_LOAD( "cd_05.7a",  0x200000, 0x80000, CRC(470befee) SHA1(a42e38319e9b7424381352512f11bd8edf0bbb96) , ROM_GROUPWORD | ROM_SKIP(6) ) // == cd-5m.7a
	ROMX_LOAD( "cd_06.8a",  0x200002, 0x80000, CRC(22bfb7a3) SHA1(c44959bd3d42b9fc8ecb482dfaf63fbd469d2c3e) , ROM_GROUPWORD | ROM_SKIP(6) ) // == cd-7m.9a
	ROMX_LOAD( "cd_07.9a",  0x200004, 0x80000, CRC(e7599ac4) SHA1(0e788a38547a8701115d01190ddeaca64388db4d) , ROM_GROUPWORD | ROM_SKIP(6) ) // == cd-6m.8a
	ROMX_LOAD( "cd_08.10a", 0x200006, 0x80000, CRC(211b4b15) SHA1(374f6b185faa0f14f5c45b9b1d60d0772d93fb17) , ROM_GROUPWORD | ROM_SKIP(6) ) // == cd-8m.10a

	ROM_REGION( 0x28000, "audiocpu", 0 ) /* QSound Z80 code */
	ROM_LOAD( "cd_q.5k",    0x00000, 0x08000, CRC(605fdb0b) SHA1(9da90ddc6513aaaf2260f0c69719c6b0e585ba8c) )
	ROM_CONTINUE(           0x10000, 0x18000 )

	ROM_REGION( 0x200000, "qsound", 0 ) /* QSound samples */
	ROM_LOAD( "cd-q1.1k",   0x000000, 0x80000, CRC(60927775) SHA1(f8599bc84c38573ebbe8685822c58b6a38b50462) )
	ROM_LOAD( "cd-q2.2k",   0x080000, 0x80000, CRC(770f4c47) SHA1(fec8ef00a6669d4d5e37787ecc7b58ee46709326) )
	ROM_LOAD( "cd-q3.3k",   0x100000, 0x80000, CRC(2f273ffc) SHA1(f0de462f6c4d251911258e0ebd886152c14d1586) )
	ROM_LOAD( "cd-q4.4k",   0x180000, 0x80000, CRC(2c67821d) SHA1(6e2528d0b22508300a6a142a796dd3bf53a66946) )

	ROM_REGION( 0x0200, "aboardplds", 0 )
	ROM_LOAD( "buf1",         0x0000, 0x0117, CRC(eb122de7) SHA1(b26b5bfe258e3e184f069719f9fd008d6b8f6b9b) )
	ROM_LOAD( "ioa1",         0x0000, 0x0117, CRC(59c7ee3b) SHA1(fbb887c5b4f5cb8df77cec710eaac2985bc482a6) )
	ROM_LOAD( "prg2",         0x0000, 0x0117, CRC(4386879a) SHA1(c36896d169d8c78393609acbbe4397931292a033) )
	ROM_LOAD( "rom1",         0x0000, 0x0117, CRC(41dc73b9) SHA1(7d4c9f1693c821fbf84e32dd6ef62ddf14967845) )

	ROM_REGION( 0x0200, "bboardplds", 0 )
	ROM_LOAD( "cd63b.1a",     0x0000, 0x0117, CRC(ef72e902) SHA1(82fea3f63869c245d0dce2809085208fe719b57a) )
	ROM_LOAD( "iob1.12d",     0x0000, 0x0117, CRC(3abc0700) SHA1(973043aa46ec6d5d1db20dc9d5937005a0f9f6ae) )
	ROM_LOAD( "bprg1.11d",    0x0000, 0x0117, CRC(31793da7) SHA1(400fa7ac517421c978c1ee7773c30b9ed0c5d3f3) )

	ROM_REGION( 0x0200, "cboardplds", 0 )
	ROM_LOAD( "ioc1.ic1",     0x0000, 0x0117, CRC(0d182081) SHA1(475b3d417785da4bc512cce2b274bb00d4cc6792) )

	ROM_REGION( 0x0200, "dboardplds", 0 )
	ROM_LOAD( "d7l1.7l",      0x0000, 0x0117, CRC(27b7410d) SHA1(06d0cba0226850f100ff1f539bd7d5db0f90c730) )
	ROM_LOAD( "d8l1.8l",      0x0000, 0x0117, CRC(539fc7da) SHA1(cad5c91629c6247e49ccbbcbfe6b08229eafae07) )
	ROM_LOAD( "d9k2.9k",      0x0000, 0x0117, CRC(cd85a156) SHA1(a88f8939c5d93e65d7bcc0eb3ee5b6f4f1114e3a) )
	ROM_LOAD( "d10f1.10f",    0x0000, 0x0117, CRC(6619c494) SHA1(3aef656c07182a2186f810f30e0d854dd5bd8d18) )
ROM_END

/* Chinese bootleg board */
ROM_START( dinohunt )
	ROM_REGION( CODE_SIZE, "maincpu", 0 )      /* 68000 code */
	ROM_LOAD16_WORD_SWAP( "u23",  0x000000, 0x80000, CRC(8d5ddc5d) SHA1(3ed1da71d73425308d0c3ac1a01a7242e60b6677) )
	ROM_LOAD16_WORD_SWAP( "u22",  0x080000, 0x80000, CRC(f72cd219) SHA1(a3f580bd328b54d2546ae7e94262c1a706d27172) )
	ROM_LOAD16_WORD_SWAP( "u21",  0x100000, 0x80000, CRC(bc275b76) SHA1(1f7f7533b4ccb511dc479f531d9d6740d3ca6712) )
	ROM_LOAD16_WORD_SWAP( "u20",  0x180000, 0x80000, CRC(8987c975) SHA1(3fd856805d627f855f72acc7aacc2e8ecf0f12bd) )

	ROM_REGION( 0x400000, "gfx", 0 )
	ROMX_LOAD( "u1",   0x000000, 0x80000, CRC(a01a9fb5) SHA1(d1ab4c7ead04932c60923363a0a661d0d1472aaa) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "u2",   0x000002, 0x80000, CRC(bdf02c17) SHA1(046ba4c589a22a7ac65c86baaae18eff0bff6ecb) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "u3",   0x000004, 0x80000, CRC(058beefa) SHA1(0d09ab5a200643b5a16e0313228f7ea243752bd0) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "u4",   0x000006, 0x80000, CRC(5028a9f1) SHA1(ff3a988153860e76beb79918ad896e702c3ecd3d) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "u5",   0x200000, 0x80000, CRC(d77f89ea) SHA1(1dd8c2a18742fd8483ecade31aed384c686ed244) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "u6",   0x200002, 0x80000, CRC(bfbcb034) SHA1(c1eb4d420b10ffa05197d9a82c7adac4d265c546) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "u7",   0x200004, 0x80000, CRC(a2544d4e) SHA1(def704055e041bc7c3b8cf7977249b69aa7924a8) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "u8",   0x200006, 0x80000, CRC(8869bbb1) SHA1(e53f77baffb913bf3ef5396d84fbfbaec6a90ca2) , ROM_GROUPWORD | ROM_SKIP(6) )

	ROM_REGION( 0x18000, "audiocpu", 0 ) /* 64k for the audio CPU (+banks) */
	ROM_LOAD( "u9",             0x00000, 0x08000, CRC(2eb16a83) SHA1(067ea0bfc2c1e73520d6b836c72fbb9da9998311) )
	ROM_CONTINUE(               0x10000, 0x08000 )

	ROM_REGION( 0x40000, "oki", 0 ) /* samples */
	ROM_LOAD( "u18",            0x00000, 0x20000, CRC(8d2899ba) SHA1(0b3ac6cf2ce0323e3bfc9da3ebfcb0fd14bc405b) )
	ROM_LOAD( "u19",            0x20000, 0x20000, CRC(b34a4b42) SHA1(3eeb9e33bb911359e03d44949ac58439a3d3d54b) )
ROM_END

/* B-Board 91635B-2 */
ROM_START( punisher )
	ROM_REGION( CODE_SIZE, "maincpu", 0 )      /* 68000 code */
	ROM_LOAD16_BYTE( "pse_26.11e",    0x000000, 0x20000, CRC(389a99d2) SHA1(e97f4225554e4603cb0e3edd296a90bb2e467ca7) )
	ROM_LOAD16_BYTE( "pse_30.11f",    0x000001, 0x20000, CRC(68fb06ac) SHA1(189e25ca7e4aaa80492c03ce06696952cc1b1553) )
	ROM_LOAD16_BYTE( "pse_27.12e",    0x040000, 0x20000, CRC(3eb181c3) SHA1(a2442449f4bbe3be03d2be7d4e2cbb69f9741dac) )
	ROM_LOAD16_BYTE( "pse_31.12f",    0x040001, 0x20000, CRC(37108e7b) SHA1(78aaa6e2913e6b1b852b39416557ac4a394d7d8b) )
	ROM_LOAD16_BYTE( "pse_24.9e",     0x080000, 0x20000, CRC(0f434414) SHA1(aaacf835a93551fc792571d6e824a01f3c5d4469) )
	ROM_LOAD16_BYTE( "pse_28.9f",     0x080001, 0x20000, CRC(b732345d) SHA1(472d84f846e9f73f129562d78352376194e0211e) )
	ROM_LOAD16_BYTE( "pse_25.10e",    0x0c0000, 0x20000, CRC(b77102e2) SHA1(2e39b2c2c0eed5ca2320a57e69bcf377f809a20c) )
	ROM_LOAD16_BYTE( "pse_29.10f",    0x0c0001, 0x20000, CRC(ec037bce) SHA1(f86e7feb63d7662a38048e6d51d7b5a69dafaffb) )
	ROM_LOAD16_WORD_SWAP( "ps_21.6f", 0x100000, 0x80000, CRC(8affa5a9) SHA1(268760b83b1723ff50a019ec51ef7af2e49935bf) )

	ROM_REGION( 0x400000, "gfx", 0 )
	ROMX_LOAD( "ps-1m.3a",  0x000000, 0x80000, CRC(77b7ccab) SHA1(e08e5d55a79e4c0c8ca819d6d7d2a14f753c6ec3) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "ps-3m.5a",  0x000002, 0x80000, CRC(0122720b) SHA1(5f0d3097e097f64106048156fbb0d343fe78fffa) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "ps-2m.4a",  0x000004, 0x80000, CRC(64fa58d4) SHA1(d4a774285ed15273195b6b26d2965ce370e54e73) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "ps-4m.6a",  0x000006, 0x80000, CRC(60da42c8) SHA1(95eec4a58d9628a2d9764951dd8dc11e4860a899) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "ps-5m.7a",  0x200000, 0x80000, CRC(c54ea839) SHA1(0733f37329edd9d0cace1319a7544b40aa7ecb0b) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "ps-7m.9a",  0x200002, 0x80000, CRC(04c5acbd) SHA1(fddc94b0f36d4d22d7c357856ae15b7514c342d3) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "ps-6m.8a",  0x200004, 0x80000, CRC(a544f4cc) SHA1(9552df8934ba25f19a22f2e07783712d8c8ef03c) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "ps-8m.10a", 0x200006, 0x80000, CRC(8f02f436) SHA1(a2f0ebb7e9593469c7b843f8962a66f3d77f79e5) , ROM_GROUPWORD | ROM_SKIP(6) )

	ROM_REGION( 0x28000, "audiocpu", 0 ) /* QSound Z80 code */
	ROM_LOAD( "ps_q.5k",    0x00000, 0x08000, CRC(49ff4446) SHA1(87af12f87a940a6c5428b4574ad44a4b54867bc3) )
	ROM_CONTINUE(           0x10000, 0x18000 )

	ROM_REGION( 0x200000, "qsound", 0 ) /* QSound samples */
	ROM_LOAD( "ps-q1.1k",   0x000000, 0x80000, CRC(31fd8726) SHA1(1d73a76682e9fb908db0c55b9a18163f7539fea1) )
	ROM_LOAD( "ps-q2.2k",   0x080000, 0x80000, CRC(980a9eef) SHA1(36571381f349bc726508a7e618ba1c635ec9d271) )
	ROM_LOAD( "ps-q3.3k",   0x100000, 0x80000, CRC(0dd44491) SHA1(903cea1d7f3120545ea3229d30fbd687d11ad68f) )
	ROM_LOAD( "ps-q4.4k",   0x180000, 0x80000, CRC(bed42f03) SHA1(21302f7e75f9c795392a3b34e16a959fc5f6e4e9) )

	ROM_REGION( 0x0200, "aboardplds", 0 )
	ROM_LOAD( "buf1",         0x0000, 0x0117, CRC(eb122de7) SHA1(b26b5bfe258e3e184f069719f9fd008d6b8f6b9b) )
	ROM_LOAD( "ioa1",         0x0000, 0x0117, CRC(59c7ee3b) SHA1(fbb887c5b4f5cb8df77cec710eaac2985bc482a6) )
	ROM_LOAD( "prg2",         0x0000, 0x0117, CRC(4386879a) SHA1(c36896d169d8c78393609acbbe4397931292a033) )
	ROM_LOAD( "rom1",         0x0000, 0x0117, CRC(41dc73b9) SHA1(7d4c9f1693c821fbf84e32dd6ef62ddf14967845) )

	ROM_REGION( 0x0200, "bboardplds", 0 )
	ROM_LOAD( "ps63b.1a",     0x0000, 0x0117, NO_DUMP )
	ROM_LOAD( "iob1.12d",     0x0000, 0x0117, CRC(3abc0700) SHA1(973043aa46ec6d5d1db20dc9d5937005a0f9f6ae) )
	ROM_LOAD( "bprg1.11d",    0x0000, 0x0117, CRC(31793da7) SHA1(400fa7ac517421c978c1ee7773c30b9ed0c5d3f3) )

	ROM_REGION( 0x0200, "cboardplds", 0 )
	ROM_LOAD( "ioc1.ic1",     0x0000, 0x0117, CRC(0d182081) SHA1(475b3d417785da4bc512cce2b274bb00d4cc6792) )

	ROM_REGION( 0x0200, "dboardplds", 0 )
	ROM_LOAD( "d7l1.7l",      0x0000, 0x0117, CRC(27b7410d) SHA1(06d0cba0226850f100ff1f539bd7d5db0f90c730) )
	ROM_LOAD( "d8l1.8l",      0x0000, 0x0117, CRC(539fc7da) SHA1(cad5c91629c6247e49ccbbcbfe6b08229eafae07) )
	ROM_LOAD( "d9k2.9k",      0x0000, 0x0117, CRC(cd85a156) SHA1(a88f8939c5d93e65d7bcc0eb3ee5b6f4f1114e3a) )
	ROM_LOAD( "d10f1.10f",    0x0000, 0x0117, CRC(6619c494) SHA1(3aef656c07182a2186f810f30e0d854dd5bd8d18) )
ROM_END

/* B-Board 91635B-2 */
ROM_START( punisheru )
	ROM_REGION( CODE_SIZE, "maincpu", 0 )      /* 68000 code */
	ROM_LOAD16_BYTE ( "psu_26.11e",   0x000000, 0x20000, CRC(9236d121) SHA1(52d5d00009f61089157319943cde8f1a1ed48ad4) )
	ROM_LOAD16_BYTE ( "psu_30.11f",   0x000001, 0x20000, CRC(8320e501) SHA1(bb3b74135df9dd494a277a1bc3bef2917351203f) )
	ROM_LOAD16_BYTE ( "psu_27.12e",   0x040000, 0x20000, CRC(61c960a1) SHA1(f8fe651283cc1f138d013cab65b833505de6df9f) )
	ROM_LOAD16_BYTE ( "psu_31.12f",   0x040001, 0x20000, CRC(78d4c298) SHA1(6e7fbaed9ad9230a6e5035c6eda64b2f1f83048c) )
	ROM_LOAD16_BYTE ( "psu_24.9e",    0x080000, 0x20000, CRC(1cfecad7) SHA1(f4dcf5066dc59507cece0c53ccc208e4323ae26f) )
	ROM_LOAD16_BYTE ( "psu_28.9f",    0x080001, 0x20000, CRC(bdf921c1) SHA1(89a6709756c7c32e7c888806f983ce5af61cfcef) )
	ROM_LOAD16_BYTE ( "psu_25.10e",   0x0c0000, 0x20000, CRC(c51acc94) SHA1(34ffd6392914e3e67d7d0804215bd1193846b554) )
	ROM_LOAD16_BYTE ( "psu_29.10f",   0x0c0001, 0x20000, CRC(52dce1ca) SHA1(45277abe34feacdcaedaec56f513b7437d4260e9) )
	ROM_LOAD16_WORD_SWAP( "ps_21.6f", 0x100000, 0x80000, CRC(8affa5a9) SHA1(268760b83b1723ff50a019ec51ef7af2e49935bf) )

	ROM_REGION( 0x400000, "gfx", 0 )
	ROMX_LOAD( "ps-1m.3a",  0x000000, 0x80000, CRC(77b7ccab) SHA1(e08e5d55a79e4c0c8ca819d6d7d2a14f753c6ec3) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "ps-3m.5a",  0x000002, 0x80000, CRC(0122720b) SHA1(5f0d3097e097f64106048156fbb0d343fe78fffa) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "ps-2m.4a",  0x000004, 0x80000, CRC(64fa58d4) SHA1(d4a774285ed15273195b6b26d2965ce370e54e73) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "ps-4m.6a",  0x000006, 0x80000, CRC(60da42c8) SHA1(95eec4a58d9628a2d9764951dd8dc11e4860a899) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "ps-5m.7a",  0x200000, 0x80000, CRC(c54ea839) SHA1(0733f37329edd9d0cace1319a7544b40aa7ecb0b) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "ps-7m.9a",  0x200002, 0x80000, CRC(04c5acbd) SHA1(fddc94b0f36d4d22d7c357856ae15b7514c342d3) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "ps-6m.8a",  0x200004, 0x80000, CRC(a544f4cc) SHA1(9552df8934ba25f19a22f2e07783712d8c8ef03c) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "ps-8m.10a", 0x200006, 0x80000, CRC(8f02f436) SHA1(a2f0ebb7e9593469c7b843f8962a66f3d77f79e5) , ROM_GROUPWORD | ROM_SKIP(6) )

	ROM_REGION( 0x28000, "audiocpu", 0 ) /* QSound Z80 code */
	ROM_LOAD( "ps_q.5k",    0x00000, 0x08000, CRC(49ff4446) SHA1(87af12f87a940a6c5428b4574ad44a4b54867bc3) )
	ROM_CONTINUE(           0x10000, 0x18000 )

	ROM_REGION( 0x200000, "qsound", 0 ) /* QSound samples */
	ROM_LOAD( "ps-q1.1k",   0x000000, 0x80000, CRC(31fd8726) SHA1(1d73a76682e9fb908db0c55b9a18163f7539fea1) )
	ROM_LOAD( "ps-q2.2k",   0x080000, 0x80000, CRC(980a9eef) SHA1(36571381f349bc726508a7e618ba1c635ec9d271) )
	ROM_LOAD( "ps-q3.3k",   0x100000, 0x80000, CRC(0dd44491) SHA1(903cea1d7f3120545ea3229d30fbd687d11ad68f) )
	ROM_LOAD( "ps-q4.4k",   0x180000, 0x80000, CRC(bed42f03) SHA1(21302f7e75f9c795392a3b34e16a959fc5f6e4e9) )

	ROM_REGION( 0x0200, "aboardplds", 0 )
	ROM_LOAD( "buf1",         0x0000, 0x0117, CRC(eb122de7) SHA1(b26b5bfe258e3e184f069719f9fd008d6b8f6b9b) )
	ROM_LOAD( "ioa1",         0x0000, 0x0117, CRC(59c7ee3b) SHA1(fbb887c5b4f5cb8df77cec710eaac2985bc482a6) )
	ROM_LOAD( "prg2",         0x0000, 0x0117, CRC(4386879a) SHA1(c36896d169d8c78393609acbbe4397931292a033) )
	ROM_LOAD( "rom1",         0x0000, 0x0117, CRC(41dc73b9) SHA1(7d4c9f1693c821fbf84e32dd6ef62ddf14967845) )

	ROM_REGION( 0x0200, "bboardplds", 0 )
	ROM_LOAD( "ps63b.1a",     0x0000, 0x0117, NO_DUMP )
	ROM_LOAD( "iob1.12d",     0x0000, 0x0117, CRC(3abc0700) SHA1(973043aa46ec6d5d1db20dc9d5937005a0f9f6ae) )
	ROM_LOAD( "bprg1.11d",    0x0000, 0x0117, CRC(31793da7) SHA1(400fa7ac517421c978c1ee7773c30b9ed0c5d3f3) )

	ROM_REGION( 0x0200, "cboardplds", 0 )
	ROM_LOAD( "ioc1.ic1",     0x0000, 0x0117, CRC(0d182081) SHA1(475b3d417785da4bc512cce2b274bb00d4cc6792) )

	ROM_REGION( 0x0200, "dboardplds", 0 )
	ROM_LOAD( "d7l1.7l",      0x0000, 0x0117, CRC(27b7410d) SHA1(06d0cba0226850f100ff1f539bd7d5db0f90c730) )
	ROM_LOAD( "d8l1.8l",      0x0000, 0x0117, CRC(539fc7da) SHA1(cad5c91629c6247e49ccbbcbfe6b08229eafae07) )
	ROM_LOAD( "d9k2.9k",      0x0000, 0x0117, CRC(cd85a156) SHA1(a88f8939c5d93e65d7bcc0eb3ee5b6f4f1114e3a) )
	ROM_LOAD( "d10f1.10f",    0x0000, 0x0117, CRC(6619c494) SHA1(3aef656c07182a2186f810f30e0d854dd5bd8d18) )
ROM_END

/* B-Board 91635B-2 */
ROM_START( punisherh )
	ROM_REGION( CODE_SIZE, "maincpu", 0 )      /* 68000 code */
	ROM_LOAD16_BYTE( "psh_26.11e",    0x000000, 0x20000, CRC(6ad2bb83) SHA1(c1c5e8922af948955f53eb56d1b9902dd8944a71) )
	ROM_LOAD16_BYTE( "psh_30.11f",    0x000001, 0x20000, CRC(058d3659) SHA1(b3fad151f80e0cc21afff3ec00b7871f21d40437) )
	ROM_LOAD16_BYTE( "psh_27.12e",    0x040000, 0x20000, CRC(579f4fd3) SHA1(5459da68e844db6e6249186aa31fe71aa72d9ff4) )
	ROM_LOAD16_BYTE( "psh_31.12f",    0x040001, 0x20000, CRC(2c9f70b5) SHA1(c5d89f37def4b9340a0325e616306f0e9e68ab13) )
	ROM_LOAD16_BYTE( "psh_24.9e",     0x080000, 0x20000, CRC(faa14841) SHA1(d998f1cdfba3305eb1398909d4b07e7492dbdec0) )
	ROM_LOAD16_BYTE( "psh_28.9f",     0x080001, 0x20000, CRC(5c5b1f20) SHA1(47ee27dc9849a69c54a5a0e269b20e61f17f34c0) )
	ROM_LOAD16_BYTE( "psh_25.10e",    0x0c0000, 0x20000, CRC(724fdfda) SHA1(23d75e7864dfa0da8c2231785878fb05a5aac628) )
	ROM_LOAD16_BYTE( "psh_29.10f",    0x0c0001, 0x20000, CRC(779cf901) SHA1(6761edcf26084e44783f3eecd5822e50f1a099bd) )
	ROM_LOAD16_WORD_SWAP( "ps_21.6f", 0x100000, 0x80000, CRC(8affa5a9) SHA1(268760b83b1723ff50a019ec51ef7af2e49935bf) )

	ROM_REGION( 0x400000, "gfx", 0 )
	ROMX_LOAD( "ps-1m.3a",  0x000000, 0x80000, CRC(77b7ccab) SHA1(e08e5d55a79e4c0c8ca819d6d7d2a14f753c6ec3) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "ps-3m.5a",  0x000002, 0x80000, CRC(0122720b) SHA1(5f0d3097e097f64106048156fbb0d343fe78fffa) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "ps-2m.4a",  0x000004, 0x80000, CRC(64fa58d4) SHA1(d4a774285ed15273195b6b26d2965ce370e54e73) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "ps-4m.6a",  0x000006, 0x80000, CRC(60da42c8) SHA1(95eec4a58d9628a2d9764951dd8dc11e4860a899) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "ps-5m.7a",  0x200000, 0x80000, CRC(c54ea839) SHA1(0733f37329edd9d0cace1319a7544b40aa7ecb0b) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "ps-7m.9a",  0x200002, 0x80000, CRC(04c5acbd) SHA1(fddc94b0f36d4d22d7c357856ae15b7514c342d3) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "ps-6m.8a",  0x200004, 0x80000, CRC(a544f4cc) SHA1(9552df8934ba25f19a22f2e07783712d8c8ef03c) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "ps-8m.10a", 0x200006, 0x80000, CRC(8f02f436) SHA1(a2f0ebb7e9593469c7b843f8962a66f3d77f79e5) , ROM_GROUPWORD | ROM_SKIP(6) )

	ROM_REGION( 0x28000, "audiocpu", 0 ) /* QSound Z80 code */
	ROM_LOAD( "ps_q.5k",    0x00000, 0x08000, CRC(49ff4446) SHA1(87af12f87a940a6c5428b4574ad44a4b54867bc3) )
	ROM_CONTINUE(           0x10000, 0x18000 )

	ROM_REGION( 0x200000, "qsound", 0 ) /* QSound samples */
	ROM_LOAD( "ps-q1.1k",   0x000000, 0x80000, CRC(31fd8726) SHA1(1d73a76682e9fb908db0c55b9a18163f7539fea1) )
	ROM_LOAD( "ps-q2.2k",   0x080000, 0x80000, CRC(980a9eef) SHA1(36571381f349bc726508a7e618ba1c635ec9d271) )
	ROM_LOAD( "ps-q3.3k",   0x100000, 0x80000, CRC(0dd44491) SHA1(903cea1d7f3120545ea3229d30fbd687d11ad68f) )
	ROM_LOAD( "ps-q4.4k",   0x180000, 0x80000, CRC(bed42f03) SHA1(21302f7e75f9c795392a3b34e16a959fc5f6e4e9) )

	ROM_REGION( 0x0200, "aboardplds", 0 )
	ROM_LOAD( "buf1",         0x0000, 0x0117, CRC(eb122de7) SHA1(b26b5bfe258e3e184f069719f9fd008d6b8f6b9b) )
	ROM_LOAD( "ioa1",         0x0000, 0x0117, CRC(59c7ee3b) SHA1(fbb887c5b4f5cb8df77cec710eaac2985bc482a6) )
	ROM_LOAD( "prg2",         0x0000, 0x0117, CRC(4386879a) SHA1(c36896d169d8c78393609acbbe4397931292a033) )
	ROM_LOAD( "rom1",         0x0000, 0x0117, CRC(41dc73b9) SHA1(7d4c9f1693c821fbf84e32dd6ef62ddf14967845) )

	ROM_REGION( 0x0200, "bboardplds", 0 )
	ROM_LOAD( "ps63b.1a",     0x0000, 0x0117, NO_DUMP )
	ROM_LOAD( "iob1.12d",     0x0000, 0x0117, CRC(3abc0700) SHA1(973043aa46ec6d5d1db20dc9d5937005a0f9f6ae) )
	ROM_LOAD( "bprg1.11d",    0x0000, 0x0117, CRC(31793da7) SHA1(400fa7ac517421c978c1ee7773c30b9ed0c5d3f3) )

	ROM_REGION( 0x0200, "cboardplds", 0 )
	ROM_LOAD( "ioc1.ic1",     0x0000, 0x0117, CRC(0d182081) SHA1(475b3d417785da4bc512cce2b274bb00d4cc6792) )

	ROM_REGION( 0x0200, "dboardplds", 0 )
	ROM_LOAD( "d7l1.7l",      0x0000, 0x0117, CRC(27b7410d) SHA1(06d0cba0226850f100ff1f539bd7d5db0f90c730) )
	ROM_LOAD( "d8l1.8l",      0x0000, 0x0117, CRC(539fc7da) SHA1(cad5c91629c6247e49ccbbcbfe6b08229eafae07) )
	ROM_LOAD( "d9k2.9k",      0x0000, 0x0117, CRC(cd85a156) SHA1(a88f8939c5d93e65d7bcc0eb3ee5b6f4f1114e3a) )
	ROM_LOAD( "d10f1.10f",    0x0000, 0x0117, CRC(6619c494) SHA1(3aef656c07182a2186f810f30e0d854dd5bd8d18) )
ROM_END

/* B-Board 91634B-2 */
ROM_START( punisherj )
	ROM_REGION( CODE_SIZE, "maincpu", 0 )      /* 68000 code */
	ROM_LOAD16_WORD_SWAP( "psj_23.8f", 0x000000, 0x80000, CRC(6b2fda52) SHA1(5f95a79b7b802609ae9ddd6641cc52610d428bf4) )
	ROM_LOAD16_WORD_SWAP( "psj_22.7f", 0x080000, 0x80000, CRC(e01036bc) SHA1(a01886014dabe8f9ab45619865c6bd9f27472eae) )
	ROM_LOAD16_WORD_SWAP( "psj_21.6f", 0x100000, 0x80000, CRC(8affa5a9) SHA1(268760b83b1723ff50a019ec51ef7af2e49935bf) )    // == ps_21.6f

	ROM_REGION( 0x400000, "gfx", 0 )
	ROMX_LOAD( "ps_01.3a",  0x000000, 0x80000, CRC(77b7ccab) SHA1(e08e5d55a79e4c0c8ca819d6d7d2a14f753c6ec3) , ROM_GROUPWORD | ROM_SKIP(6) ) // == ps-1m.3a
	ROMX_LOAD( "ps_02.4a",  0x000002, 0x80000, CRC(0122720b) SHA1(5f0d3097e097f64106048156fbb0d343fe78fffa) , ROM_GROUPWORD | ROM_SKIP(6) ) // == ps-3m.5a
	ROMX_LOAD( "ps_03.5a",  0x000004, 0x80000, CRC(64fa58d4) SHA1(d4a774285ed15273195b6b26d2965ce370e54e73) , ROM_GROUPWORD | ROM_SKIP(6) ) // == ps-2m.4a
	ROMX_LOAD( "ps_04.6a",  0x000006, 0x80000, CRC(60da42c8) SHA1(95eec4a58d9628a2d9764951dd8dc11e4860a899) , ROM_GROUPWORD | ROM_SKIP(6) ) // == ps-4m.6a
	ROMX_LOAD( "ps_05.7a",  0x200000, 0x80000, CRC(c54ea839) SHA1(0733f37329edd9d0cace1319a7544b40aa7ecb0b) , ROM_GROUPWORD | ROM_SKIP(6) ) // == ps-5m.7a
	ROMX_LOAD( "ps_06.8a",  0x200002, 0x80000, CRC(04c5acbd) SHA1(fddc94b0f36d4d22d7c357856ae15b7514c342d3) , ROM_GROUPWORD | ROM_SKIP(6) ) // == ps-7m.9a
	ROMX_LOAD( "ps_07.9a",  0x200004, 0x80000, CRC(a544f4cc) SHA1(9552df8934ba25f19a22f2e07783712d8c8ef03c) , ROM_GROUPWORD | ROM_SKIP(6) ) // == ps-6m.8a
	ROMX_LOAD( "ps_08.10a", 0x200006, 0x80000, CRC(8f02f436) SHA1(a2f0ebb7e9593469c7b843f8962a66f3d77f79e5) , ROM_GROUPWORD | ROM_SKIP(6) ) // == ps-8m.10a

	ROM_REGION( 0x28000, "audiocpu", 0 ) /* QSound Z80 code */
	ROM_LOAD( "ps_q.5k",    0x00000, 0x08000, CRC(49ff4446) SHA1(87af12f87a940a6c5428b4574ad44a4b54867bc3) )
	ROM_CONTINUE(           0x10000, 0x18000 )

	ROM_REGION( 0x200000, "qsound", 0 ) /* QSound samples */
	ROM_LOAD( "ps-q1.1k",   0x000000, 0x80000, CRC(31fd8726) SHA1(1d73a76682e9fb908db0c55b9a18163f7539fea1) )
	ROM_LOAD( "ps-q2.2k",   0x080000, 0x80000, CRC(980a9eef) SHA1(36571381f349bc726508a7e618ba1c635ec9d271) )
	ROM_LOAD( "ps-q3.3k",   0x100000, 0x80000, CRC(0dd44491) SHA1(903cea1d7f3120545ea3229d30fbd687d11ad68f) )
	ROM_LOAD( "ps-q4.4k",   0x180000, 0x80000, CRC(bed42f03) SHA1(21302f7e75f9c795392a3b34e16a959fc5f6e4e9) )

	ROM_REGION( 0x0200, "aboardplds", 0 )
	ROM_LOAD( "buf1",         0x0000, 0x0117, CRC(eb122de7) SHA1(b26b5bfe258e3e184f069719f9fd008d6b8f6b9b) )
	ROM_LOAD( "ioa1",         0x0000, 0x0117, CRC(59c7ee3b) SHA1(fbb887c5b4f5cb8df77cec710eaac2985bc482a6) )
	ROM_LOAD( "prg2",         0x0000, 0x0117, CRC(4386879a) SHA1(c36896d169d8c78393609acbbe4397931292a033) )
	ROM_LOAD( "rom1",         0x0000, 0x0117, CRC(41dc73b9) SHA1(7d4c9f1693c821fbf84e32dd6ef62ddf14967845) )

	ROM_REGION( 0x0200, "bboardplds", 0 )
	ROM_LOAD( "ps63b.1a",     0x0000, 0x0117, NO_DUMP )
	ROM_LOAD( "iob1.12d",     0x0000, 0x0117, CRC(3abc0700) SHA1(973043aa46ec6d5d1db20dc9d5937005a0f9f6ae) )
	ROM_LOAD( "bprg1.11d",    0x0000, 0x0117, CRC(31793da7) SHA1(400fa7ac517421c978c1ee7773c30b9ed0c5d3f3) )

	ROM_REGION( 0x0200, "cboardplds", 0 )
	ROM_LOAD( "ioc1.ic1",     0x0000, 0x0117, CRC(0d182081) SHA1(475b3d417785da4bc512cce2b274bb00d4cc6792) )

	ROM_REGION( 0x0200, "dboardplds", 0 )
	ROM_LOAD( "d7l1.7l",      0x0000, 0x0117, CRC(27b7410d) SHA1(06d0cba0226850f100ff1f539bd7d5db0f90c730) )
	ROM_LOAD( "d8l1.8l",      0x0000, 0x0117, CRC(539fc7da) SHA1(cad5c91629c6247e49ccbbcbfe6b08229eafae07) )
	ROM_LOAD( "d9k2.9k",      0x0000, 0x0117, CRC(cd85a156) SHA1(a88f8939c5d93e65d7bcc0eb3ee5b6f4f1114e3a) )
	ROM_LOAD( "d10f1.10f",    0x0000, 0x0117, CRC(6619c494) SHA1(3aef656c07182a2186f810f30e0d854dd5bd8d18) )
ROM_END

/* Chinese bootleg board */
ROM_START( punisherbz )
	ROM_REGION( CODE_SIZE, "maincpu", 0 )      /* 68000 code */
	ROM_LOAD16_WORD_SWAP( "23.096",   0x000000, 0x80000, CRC(bfa45d23) SHA1(56d04d02c9b9b22603205b1ce9e376874d2eaa05) )
	ROM_LOAD16_WORD_SWAP( "22.096",   0x080000, 0x80000, CRC(092578a4) SHA1(c3c8f98f8b8fc9c71b33fb906b4c591067b4355b) )
	ROM_LOAD16_WORD_SWAP( "21.096",   0x100000, 0x80000, CRC(d21ccddb) SHA1(295c886cd14ee54e12acfe3f1e8f13513442c8c0) )
	ROM_LOAD16_WORD_SWAP( "20.096",   0x180000, 0x80000, CRC(f9f334ce) SHA1(22e58ce5096b95138a30e9726e1169895118e990) )

	ROM_REGION( 0x400000, "gfx", 0 )
	ROMX_LOAD( "1.096",         0x000000, 0x80000, CRC(ad468e07) SHA1(1ee8ecfd001b447982bc16972575eb2c5e01bc51) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "2.096",         0x000002, 0x80000, CRC(b9fdb6b5) SHA1(991fe0ded5c859f1e25766e9417e1ad691205e59) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "3.096",         0x000004, 0x80000, CRC(be0b1a78) SHA1(d002bd90d0f239d29f5ea481e455d85a1c088516) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "4.096",         0x000006, 0x80000, CRC(bba67a43) SHA1(9cd51a06a7661b2f78059ac392423f73de5cf56f) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "ps_gfx5.rom",   0x200000, 0x80000, CRC(c54ea839) SHA1(0733f37329edd9d0cace1319a7544b40aa7ecb0b) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "ps_gfx7.rom",   0x200002, 0x80000, CRC(04c5acbd) SHA1(fddc94b0f36d4d22d7c357856ae15b7514c342d3) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "ps_gfx6.rom",   0x200004, 0x80000, CRC(a544f4cc) SHA1(9552df8934ba25f19a22f2e07783712d8c8ef03c) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "ps_gfx8.rom",   0x200006, 0x80000, CRC(8f02f436) SHA1(a2f0ebb7e9593469c7b843f8962a66f3d77f79e5) , ROM_GROUPWORD | ROM_SKIP(6) )

	ROM_REGION( 0x18000, "audiocpu", 0 ) /* 64k for the audio CPU (+banks) */
	ROM_LOAD( "9.512",          0x00000, 0x08000, CRC(b8367eb5) SHA1(ec3db29fdd6200e9a8f4f8073a7e34aef731354f) )
	ROM_CONTINUE(               0x10000, 0x08000 )

	ROM_REGION( 0x40000, "oki", 0 ) /* samples */
	ROM_LOAD( "18.010",         0x00000, 0x20000, CRC(375c66e7) SHA1(36189e23209ce4ae5d9cbabd1574540d0591e7b3) )
	ROM_LOAD( "19.010",         0x20000, 0x20000, CRC(eb5ca884) SHA1(3592c69f77c7cd6ee241d6c1079c34a3e58abb5b) )
ROM_END

/* B-Board 91635B-2 */
ROM_START( slammast )
	ROM_REGION( CODE_SIZE, "maincpu", 0 )      /* 68000 code */
	ROM_LOAD16_WORD_SWAP( "mbe_23e.8f", 0x000000, 0x80000, CRC(5394057a) SHA1(57f8b40c0a15e82c98ce5f0a8c4bdf60a1bc3107) )
	ROM_LOAD16_BYTE( "mbe_24b.9e",      0x080000, 0x20000, CRC(95d5e729) SHA1(df3be896e55c92eb50887a4317178a3d11048433) )
	ROM_LOAD16_BYTE( "mbe_28b.9f",      0x080001, 0x20000, CRC(b1c7cbcb) SHA1(cf5ad72be4a055db876e7347b1826325b9bf81d9) )
	ROM_LOAD16_BYTE( "mbe_25b.10e",     0x0c0000, 0x20000, CRC(a50d3fd4) SHA1(dc3d108c3bc27f45b8b2e11919ba2a86e05b41d1) )
	ROM_LOAD16_BYTE( "mbe_29b.10f",     0x0c0001, 0x20000, CRC(08e32e56) SHA1(70ad78b079f777ec02089f0df20ce2baad7adce5) )
	ROM_LOAD16_WORD_SWAP( "mbe_21a.6f", 0x100000, 0x80000, CRC(d5007b05) SHA1(c55e55908aeda40ca2318c76ccbc05d333676875) )
	ROM_LOAD16_WORD_SWAP( "mbe_20a.5f", 0x180000, 0x80000, CRC(aeb557b0) SHA1(530551942961d776f0a85852e02bb243840ca671) )

	ROM_REGION( 0x600000, "gfx", 0 )
	ROMX_LOAD( "mb-1m.3a",  0x000000, 0x80000, CRC(41468e06) SHA1(fb365798f2889a20eebaea2393c9c2c8827003c4) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "mb-3m.5a",  0x000002, 0x80000, CRC(f453aa9e) SHA1(24a103dc6f0dc96f8d0f6164ad732909c9cd2d6a) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "mb-2m.4a",  0x000004, 0x80000, CRC(2ffbfea8) SHA1(13e30133664a009686e1114c92b558bdbb91ea32) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "mb-4m.6a",  0x000006, 0x80000, CRC(1eb9841d) SHA1(685da3e011a96b36be9f639a241b2f8f27da4629) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "mb-5m.7a",  0x200000, 0x80000, CRC(506b9dc9) SHA1(933bf2fb9bcc1a408f961f0e7052da80144bddad) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "mb-7m.9a",  0x200002, 0x80000, CRC(aff8c2fb) SHA1(ce37a6d5b1eb58c2d74f23f84ec824c214c93217) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "mb-6m.8a",  0x200004, 0x80000, CRC(b76c70e9) SHA1(c21e255815ec9a985919dbd760ed266c28bd47cd) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "mb-8m.10a", 0x200006, 0x80000, CRC(e60c9556) SHA1(b91c14092aa8dbb0922d96998123ef1970a658f6) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "mb-10m.3c", 0x400000, 0x80000, CRC(97976ff5) SHA1(ec9d3460816ab971a02fbce42960283091777e47) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "mb-12m.5c", 0x400002, 0x80000, CRC(b350a840) SHA1(2b8b996cd08051e7e8e134bff5448775d78058a0) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "mb-11m.4c", 0x400004, 0x80000, CRC(8fb94743) SHA1(294f6182c8a41b640d1f57cb5e3a2abce3b06482) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "mb-13m.6c", 0x400006, 0x80000, CRC(da810d5f) SHA1(392bbd405244b8c99024c9228cfec6a7ef0accdb) , ROM_GROUPWORD | ROM_SKIP(6) )

	ROM_REGION( 0x28000, "audiocpu", 0 ) /* QSound Z80 code */
	ROM_LOAD( "mb_qa.5k",   0x00000, 0x08000, CRC(e21a03c4) SHA1(98c03fd2c9b6bf8a4fc25a4edca87fff7c3c3819) )
	ROM_CONTINUE(           0x10000, 0x18000 )

	ROM_REGION( 0x8000, "user1", 0 )
	ROM_COPY( "audiocpu", 0x000000, 0x00000, 0x8000 )

	ROM_REGION( 0x400000, "qsound", 0 ) /* QSound samples */
	ROM_LOAD( "mb-q1.1k",   0x000000, 0x80000, CRC(0630c3ce) SHA1(520fc74c5c3638f611fa2f1b5efb08b91747e29b) )
	ROM_LOAD( "mb-q2.2k",   0x080000, 0x80000, CRC(354f9c21) SHA1(1dc6b39791fd0f760697f409a6b62361a7bf62e9) )
	ROM_LOAD( "mb-q3.3k",   0x100000, 0x80000, CRC(7838487c) SHA1(056b7da05cfca46873edacd674ca25c70855c6db) )
	ROM_LOAD( "mb-q4.4k",   0x180000, 0x80000, CRC(ab66e087) SHA1(066ea69a0157e8647eea3c44d0a1843898860678) )
	ROM_LOAD( "mb-q5.1m",   0x200000, 0x80000, CRC(c789fef2) SHA1(10d1e3d92288fccd4e064a3716a788a165efc3c9) )
	ROM_LOAD( "mb-q6.2m",   0x280000, 0x80000, CRC(ecb81b61) SHA1(e339f21ae47de4782f3b338befcdac659c3503f6) )
	ROM_LOAD( "mb-q7.3m",   0x300000, 0x80000, CRC(041e49ba) SHA1(3220b033a5c0cfbbe75c0c113cf2db39fb093a7e) )
	ROM_LOAD( "mb-q8.4m",   0x380000, 0x80000, CRC(59fe702a) SHA1(807178dfc6d864e49fd7aabb5c4895835cf0e85b) )

	ROM_REGION( 0x0200, "aboardplds", 0 )
	ROM_LOAD( "buf1",         0x0000, 0x0117, CRC(eb122de7) SHA1(b26b5bfe258e3e184f069719f9fd008d6b8f6b9b) )
	ROM_LOAD( "ioa1",         0x0000, 0x0117, CRC(59c7ee3b) SHA1(fbb887c5b4f5cb8df77cec710eaac2985bc482a6) )
	ROM_LOAD( "prg2",         0x0000, 0x0117, CRC(4386879a) SHA1(c36896d169d8c78393609acbbe4397931292a033) )
	ROM_LOAD( "rom1",         0x0000, 0x0117, CRC(41dc73b9) SHA1(7d4c9f1693c821fbf84e32dd6ef62ddf14967845) )

	ROM_REGION( 0x0200, "bboardplds", 0 )
	ROM_LOAD( "mb63b.1a",     0x0000, 0x0117, CRC(b8392f02) SHA1(8dedf9f43b30991694f8009302ca628cb50a0b1a) )
	ROM_LOAD( "iob1.12d",     0x0000, 0x0117, CRC(3abc0700) SHA1(973043aa46ec6d5d1db20dc9d5937005a0f9f6ae) )
	ROM_LOAD( "bprg1.11d",    0x0000, 0x0117, CRC(31793da7) SHA1(400fa7ac517421c978c1ee7773c30b9ed0c5d3f3) )

	ROM_REGION( 0x0200, "cboardplds", 0 )
	ROM_LOAD( "ioc1.ic1",     0x0000, 0x0117, CRC(0d182081) SHA1(475b3d417785da4bc512cce2b274bb00d4cc6792) )

	ROM_REGION( 0x0200, "dboardplds", 0 )
	ROM_LOAD( "d7l1.7l",      0x0000, 0x0117, CRC(27b7410d) SHA1(06d0cba0226850f100ff1f539bd7d5db0f90c730) )
	ROM_LOAD( "d8l1.8l",      0x0000, 0x0117, CRC(539fc7da) SHA1(cad5c91629c6247e49ccbbcbfe6b08229eafae07) )
	ROM_LOAD( "d9k2.9k",      0x0000, 0x0117, CRC(cd85a156) SHA1(a88f8939c5d93e65d7bcc0eb3ee5b6f4f1114e3a) )
	ROM_LOAD( "d10f1.10f",    0x0000, 0x0117, CRC(6619c494) SHA1(3aef656c07182a2186f810f30e0d854dd5bd8d18) )
ROM_END

/* B-Board 91635B-2 */
ROM_START( slammastu )
	ROM_REGION( CODE_SIZE, "maincpu", 0 )      /* 68000 code */
	ROM_LOAD16_WORD_SWAP( "mbu_23e.8f", 0x000000, 0x80000, CRC(224f0062) SHA1(d961f2e7db7acac576539c24a69e7dd9bf8fc406) )
	ROM_LOAD16_BYTE( "mbu_24b.9e",      0x080000, 0x20000, CRC(95d5e729) SHA1(df3be896e55c92eb50887a4317178a3d11048433) )   // == mbe_24b.9e
	ROM_LOAD16_BYTE( "mbu_28b.9f",      0x080001, 0x20000, CRC(b1c7cbcb) SHA1(cf5ad72be4a055db876e7347b1826325b9bf81d9) )   // == mbe_28b.9f
	ROM_LOAD16_BYTE( "mbu_25b.10e",     0x0c0000, 0x20000, CRC(a50d3fd4) SHA1(dc3d108c3bc27f45b8b2e11919ba2a86e05b41d1) )   // == mbe_25b.10e
	ROM_LOAD16_BYTE( "mbu_29b.10f",     0x0c0001, 0x20000, CRC(08e32e56) SHA1(70ad78b079f777ec02089f0df20ce2baad7adce5) )   // == mbe_29b.10f
	ROM_LOAD16_WORD_SWAP( "mbu_21a.6f", 0x100000, 0x80000, CRC(d5007b05) SHA1(c55e55908aeda40ca2318c76ccbc05d333676875) )   // == mbe_21a.6f
	ROM_LOAD16_WORD_SWAP( "mbu_20a.5f", 0x180000, 0x80000, CRC(fc848af5) SHA1(cd3f6e50779b89ee57a9d08bfa1d58dea286457c) )

	ROM_REGION( 0x600000, "gfx", 0 )
	ROMX_LOAD( "mb-1m.3a",  0x000000, 0x80000, CRC(41468e06) SHA1(fb365798f2889a20eebaea2393c9c2c8827003c4) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "mb-3m.5a",  0x000002, 0x80000, CRC(f453aa9e) SHA1(24a103dc6f0dc96f8d0f6164ad732909c9cd2d6a) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "mb-2m.4a",  0x000004, 0x80000, CRC(2ffbfea8) SHA1(13e30133664a009686e1114c92b558bdbb91ea32) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "mb-4m.6a",  0x000006, 0x80000, CRC(1eb9841d) SHA1(685da3e011a96b36be9f639a241b2f8f27da4629) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "mb-5m.7a",  0x200000, 0x80000, CRC(506b9dc9) SHA1(933bf2fb9bcc1a408f961f0e7052da80144bddad) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "mb-7m.9a",  0x200002, 0x80000, CRC(aff8c2fb) SHA1(ce37a6d5b1eb58c2d74f23f84ec824c214c93217) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "mb-6m.8a",  0x200004, 0x80000, CRC(b76c70e9) SHA1(c21e255815ec9a985919dbd760ed266c28bd47cd) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "mb-8m.10a", 0x200006, 0x80000, CRC(e60c9556) SHA1(b91c14092aa8dbb0922d96998123ef1970a658f6) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "mb-10m.3c", 0x400000, 0x80000, CRC(97976ff5) SHA1(ec9d3460816ab971a02fbce42960283091777e47) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "mb-12m.5c", 0x400002, 0x80000, CRC(b350a840) SHA1(2b8b996cd08051e7e8e134bff5448775d78058a0) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "mb-11m.4c", 0x400004, 0x80000, CRC(8fb94743) SHA1(294f6182c8a41b640d1f57cb5e3a2abce3b06482) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "mb-13m.6c", 0x400006, 0x80000, CRC(da810d5f) SHA1(392bbd405244b8c99024c9228cfec6a7ef0accdb) , ROM_GROUPWORD | ROM_SKIP(6) )

	ROM_REGION( 0x28000, "audiocpu", 0 ) /* QSound Z80 code */
	ROM_LOAD( "mb_qa.5k",   0x00000, 0x08000, CRC(e21a03c4) SHA1(98c03fd2c9b6bf8a4fc25a4edca87fff7c3c3819) )
	ROM_CONTINUE(           0x10000, 0x18000 )

	ROM_REGION( 0x8000, "user1", 0 )
	ROM_COPY( "audiocpu", 0x000000, 0x00000, 0x8000 )

	ROM_REGION( 0x400000, "qsound", 0 ) /* QSound samples */
	ROM_LOAD( "mb-q1.1k",   0x000000, 0x80000, CRC(0630c3ce) SHA1(520fc74c5c3638f611fa2f1b5efb08b91747e29b) )
	ROM_LOAD( "mb-q2.2k",   0x080000, 0x80000, CRC(354f9c21) SHA1(1dc6b39791fd0f760697f409a6b62361a7bf62e9) )
	ROM_LOAD( "mb-q3.3k",   0x100000, 0x80000, CRC(7838487c) SHA1(056b7da05cfca46873edacd674ca25c70855c6db) )
	ROM_LOAD( "mb-q4.4k",   0x180000, 0x80000, CRC(ab66e087) SHA1(066ea69a0157e8647eea3c44d0a1843898860678) )
	ROM_LOAD( "mb-q5.1m",   0x200000, 0x80000, CRC(c789fef2) SHA1(10d1e3d92288fccd4e064a3716a788a165efc3c9) )
	ROM_LOAD( "mb-q6.2m",   0x280000, 0x80000, CRC(ecb81b61) SHA1(e339f21ae47de4782f3b338befcdac659c3503f6) )
	ROM_LOAD( "mb-q7.3m",   0x300000, 0x80000, CRC(041e49ba) SHA1(3220b033a5c0cfbbe75c0c113cf2db39fb093a7e) )
	ROM_LOAD( "mb-q8.4m",   0x380000, 0x80000, CRC(59fe702a) SHA1(807178dfc6d864e49fd7aabb5c4895835cf0e85b) )

	ROM_REGION( 0x0200, "aboardplds", 0 )
	ROM_LOAD( "buf1",         0x0000, 0x0117, CRC(eb122de7) SHA1(b26b5bfe258e3e184f069719f9fd008d6b8f6b9b) )
	ROM_LOAD( "ioa1",         0x0000, 0x0117, CRC(59c7ee3b) SHA1(fbb887c5b4f5cb8df77cec710eaac2985bc482a6) )
	ROM_LOAD( "prg2",         0x0000, 0x0117, CRC(4386879a) SHA1(c36896d169d8c78393609acbbe4397931292a033) )
	ROM_LOAD( "rom1",         0x0000, 0x0117, CRC(41dc73b9) SHA1(7d4c9f1693c821fbf84e32dd6ef62ddf14967845) )

	ROM_REGION( 0x0200, "bboardplds", 0 )
	ROM_LOAD( "mb63b.1a",     0x0000, 0x0117, CRC(b8392f02) SHA1(8dedf9f43b30991694f8009302ca628cb50a0b1a) )
	ROM_LOAD( "iob1.12d",     0x0000, 0x0117, CRC(3abc0700) SHA1(973043aa46ec6d5d1db20dc9d5937005a0f9f6ae) )
	ROM_LOAD( "bprg1.11d",    0x0000, 0x0117, CRC(31793da7) SHA1(400fa7ac517421c978c1ee7773c30b9ed0c5d3f3) )

	ROM_REGION( 0x0200, "cboardplds", 0 )
	ROM_LOAD( "ioc1.ic1",     0x0000, 0x0117, CRC(0d182081) SHA1(475b3d417785da4bc512cce2b274bb00d4cc6792) )

	ROM_REGION( 0x0200, "dboardplds", 0 )
	ROM_LOAD( "d7l1.7l",      0x0000, 0x0117, CRC(27b7410d) SHA1(06d0cba0226850f100ff1f539bd7d5db0f90c730) )
	ROM_LOAD( "d8l1.8l",      0x0000, 0x0117, CRC(539fc7da) SHA1(cad5c91629c6247e49ccbbcbfe6b08229eafae07) )
	ROM_LOAD( "d9k2.9k",      0x0000, 0x0117, CRC(cd85a156) SHA1(a88f8939c5d93e65d7bcc0eb3ee5b6f4f1114e3a) )
	ROM_LOAD( "d10f1.10f",    0x0000, 0x0117, CRC(6619c494) SHA1(3aef656c07182a2186f810f30e0d854dd5bd8d18) )
ROM_END

/* B-Board 91634B-2 */
ROM_START( mbomberj )
	ROM_REGION( CODE_SIZE, "maincpu", 0 )      /* 68000 code */
	ROM_LOAD16_WORD_SWAP( "mbj_23e.8f", 0x000000, 0x80000, CRC(0d06036a) SHA1(e1b102888038b4bb612a41ac94a43333d468a245) )
	ROM_LOAD16_WORD_SWAP( "mbj_22b.7f", 0x080000, 0x80000, CRC(acd38478) SHA1(0bed74b52838006c6ec3661f92058dc2aa3e8193) )
	ROM_LOAD16_WORD_SWAP( "mbj_21a.6f", 0x100000, 0x80000, CRC(d5007b05) SHA1(c55e55908aeda40ca2318c76ccbc05d333676875) )   // == mbe_21a.6f
	ROM_LOAD16_WORD_SWAP( "mbj_20a.5f", 0x180000, 0x80000, CRC(aeb557b0) SHA1(530551942961d776f0a85852e02bb243840ca671) )   // == mbe_20a.5f

	ROM_REGION( 0x600000, "gfx", 0 )
	ROMX_LOAD( "mb_01.3a",  0x000000, 0x80000, CRC(a53b1c81) SHA1(d1efb88eeaf6e30e51aaf1432078003e52454dd9) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "mb_02.4a",  0x000002, 0x80000, CRC(23fe10f6) SHA1(deefa7cac4394b0642f7fb444f9374dbe0bc8843) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "mb_03.5a",  0x000004, 0x80000, CRC(cb866c2f) SHA1(b087f52e3b2a514a209612319d1d7c4f1c12b8bd) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "mb_04.6a",  0x000006, 0x80000, CRC(c9143e75) SHA1(e30090625ef6ac971a4f65d53f5458cebb5f146c) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "mb_05.7a",  0x200000, 0x80000, CRC(506b9dc9) SHA1(933bf2fb9bcc1a408f961f0e7052da80144bddad) , ROM_GROUPWORD | ROM_SKIP(6) ) // == mb-5m.7a
	ROMX_LOAD( "mb_06.8a",  0x200002, 0x80000, CRC(aff8c2fb) SHA1(ce37a6d5b1eb58c2d74f23f84ec824c214c93217) , ROM_GROUPWORD | ROM_SKIP(6) ) // == mb-7m.9a
	ROMX_LOAD( "mb_07.9a",  0x200004, 0x80000, CRC(b76c70e9) SHA1(c21e255815ec9a985919dbd760ed266c28bd47cd) , ROM_GROUPWORD | ROM_SKIP(6) ) // == mb-6m.8a
	ROMX_LOAD( "mb_08.10a", 0x200006, 0x80000, CRC(e60c9556) SHA1(b91c14092aa8dbb0922d96998123ef1970a658f6) , ROM_GROUPWORD | ROM_SKIP(6) ) // == mb-8m.10a
	ROMX_LOAD( "mb_10.3c",  0x400000, 0x80000, CRC(97976ff5) SHA1(ec9d3460816ab971a02fbce42960283091777e47) , ROM_GROUPWORD | ROM_SKIP(6) ) // == mb-10m.3c
	ROMX_LOAD( "mb_11.4c",  0x400002, 0x80000, CRC(b350a840) SHA1(2b8b996cd08051e7e8e134bff5448775d78058a0) , ROM_GROUPWORD | ROM_SKIP(6) ) // == mb-12m.5c
	ROMX_LOAD( "mb_12.5c",  0x400004, 0x80000, CRC(8fb94743) SHA1(294f6182c8a41b640d1f57cb5e3a2abce3b06482) , ROM_GROUPWORD | ROM_SKIP(6) ) // == mb-11m.4c
	ROMX_LOAD( "mb_13.6c",  0x400006, 0x80000, CRC(da810d5f) SHA1(392bbd405244b8c99024c9228cfec6a7ef0accdb) , ROM_GROUPWORD | ROM_SKIP(6) ) // == mb-13m.6c

	ROM_REGION( 0x28000, "audiocpu", 0 ) /* QSound Z80 code */
	ROM_LOAD( "mb_qa.5k",   0x00000, 0x08000, CRC(e21a03c4) SHA1(98c03fd2c9b6bf8a4fc25a4edca87fff7c3c3819) )
	ROM_CONTINUE(           0x10000, 0x18000 )

	ROM_REGION( 0x8000, "user1", 0 )
	ROM_COPY( "audiocpu", 0x000000, 0x00000, 0x8000 )

	ROM_REGION( 0x400000, "qsound", 0 ) /* QSound samples */
	ROM_LOAD( "mb-q1.1k",   0x000000, 0x80000, CRC(0630c3ce) SHA1(520fc74c5c3638f611fa2f1b5efb08b91747e29b) )
	ROM_LOAD( "mb-q2.2k",   0x080000, 0x80000, CRC(354f9c21) SHA1(1dc6b39791fd0f760697f409a6b62361a7bf62e9) )
	ROM_LOAD( "mb-q3.3k",   0x100000, 0x80000, CRC(7838487c) SHA1(056b7da05cfca46873edacd674ca25c70855c6db) )
	ROM_LOAD( "mb-q4.4k",   0x180000, 0x80000, CRC(ab66e087) SHA1(066ea69a0157e8647eea3c44d0a1843898860678) )
	ROM_LOAD( "mb-q5.1m",   0x200000, 0x80000, CRC(c789fef2) SHA1(10d1e3d92288fccd4e064a3716a788a165efc3c9) )
	ROM_LOAD( "mb-q6.2m",   0x280000, 0x80000, CRC(ecb81b61) SHA1(e339f21ae47de4782f3b338befcdac659c3503f6) )
	ROM_LOAD( "mb-q7.3m",   0x300000, 0x80000, CRC(041e49ba) SHA1(3220b033a5c0cfbbe75c0c113cf2db39fb093a7e) )
	ROM_LOAD( "mb-q8.4m",   0x380000, 0x80000, CRC(59fe702a) SHA1(807178dfc6d864e49fd7aabb5c4895835cf0e85b) )

	ROM_REGION( 0x0200, "aboardplds", 0 )
	ROM_LOAD( "buf1",         0x0000, 0x0117, CRC(eb122de7) SHA1(b26b5bfe258e3e184f069719f9fd008d6b8f6b9b) )
	ROM_LOAD( "ioa1",         0x0000, 0x0117, CRC(59c7ee3b) SHA1(fbb887c5b4f5cb8df77cec710eaac2985bc482a6) )
	ROM_LOAD( "prg2",         0x0000, 0x0117, CRC(4386879a) SHA1(c36896d169d8c78393609acbbe4397931292a033) )
	ROM_LOAD( "rom1",         0x0000, 0x0117, CRC(41dc73b9) SHA1(7d4c9f1693c821fbf84e32dd6ef62ddf14967845) )

	ROM_REGION( 0x0200, "bboardplds", 0 )
	ROM_LOAD( "mb63b.1a",     0x0000, 0x0117, CRC(b8392f02) SHA1(8dedf9f43b30991694f8009302ca628cb50a0b1a) )
	ROM_LOAD( "iob1.12d",     0x0000, 0x0117, CRC(3abc0700) SHA1(973043aa46ec6d5d1db20dc9d5937005a0f9f6ae) )
	ROM_LOAD( "bprg1.11d",    0x0000, 0x0117, CRC(31793da7) SHA1(400fa7ac517421c978c1ee7773c30b9ed0c5d3f3) )

	ROM_REGION( 0x0200, "cboardplds", 0 )
	ROM_LOAD( "ioc1.ic1",     0x0000, 0x0117, CRC(0d182081) SHA1(475b3d417785da4bc512cce2b274bb00d4cc6792) )

	ROM_REGION( 0x0200, "dboardplds", 0 )
	ROM_LOAD( "d7l1.7l",      0x0000, 0x0117, CRC(27b7410d) SHA1(06d0cba0226850f100ff1f539bd7d5db0f90c730) )
	ROM_LOAD( "d8l1.8l",      0x0000, 0x0117, CRC(539fc7da) SHA1(cad5c91629c6247e49ccbbcbfe6b08229eafae07) )
	ROM_LOAD( "d9k2.9k",      0x0000, 0x0117, CRC(cd85a156) SHA1(a88f8939c5d93e65d7bcc0eb3ee5b6f4f1114e3a) )
	ROM_LOAD( "d10f1.10f",    0x0000, 0x0117, CRC(6619c494) SHA1(3aef656c07182a2186f810f30e0d854dd5bd8d18) )
ROM_END

/* FIXME B-Board uncertain but should be 91635B from the program ROM names */
ROM_START( mbombrd )
	ROM_REGION( CODE_SIZE, "maincpu", 0 )      /* 68000 code */
	ROM_LOAD16_BYTE( "mbde_26.11e",     0x000000, 0x20000, CRC(72b7451c) SHA1(380ef57bb00f1c92d2f29e551b0a670eb5a56cb7) )
	ROM_LOAD16_BYTE( "mbde_30.11f",     0x000001, 0x20000, CRC(a036dc16) SHA1(a68cf74976f482dbc581734e143669511a9a4bee) )
	ROM_LOAD16_BYTE( "mbde_27.12e",     0x040000, 0x20000, CRC(4086f534) SHA1(a2b949f00035b06cb1cd01185902daca3d89d0e3) )
	ROM_LOAD16_BYTE( "mbde_31.12f",     0x040001, 0x20000, CRC(085f47f0) SHA1(ac93a196faf17b7dbe7179ce1e850d9cd7293a21) )
	ROM_LOAD16_BYTE( "mbde_24.9e",      0x080000, 0x20000, CRC(c20895a5) SHA1(35116f7ef8576753ec989647ca2f6a6131d6909f) )
	ROM_LOAD16_BYTE( "mbde_28.9f",      0x080001, 0x20000, CRC(2618d5e1) SHA1(50797c6dda04df95267ff9ef08933c17c3ce7057) )
	ROM_LOAD16_BYTE( "mbde_25.10e",     0x0c0000, 0x20000, CRC(9bdb6b11) SHA1(fbfbd6b5a72ca3237713ce43a798660f899b707d) )
	ROM_LOAD16_BYTE( "mbde_29.10f",     0x0c0001, 0x20000, CRC(3f52d5e5) SHA1(0b1ed8e876a6ec2cfb83676afe43a81e8a033e52) )
	ROM_LOAD16_WORD_SWAP( "mbde_21.6f", 0x100000, 0x80000, CRC(690c026a) SHA1(80ad780743b50750b6bfe1d4e28efe98e562233e) )
	ROM_LOAD16_WORD_SWAP( "mbde_20.5f", 0x180000, 0x80000, CRC(b8b2139b) SHA1(88c9169a9979b711ab7afb8272df0a1c80bb357b) )

	ROM_REGION( 0x600000, "gfx", 0 )
	ROMX_LOAD( "mb-1m.3a",  0x000000, 0x80000, CRC(41468e06) SHA1(fb365798f2889a20eebaea2393c9c2c8827003c4) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "mb-3m.5a",  0x000002, 0x80000, CRC(f453aa9e) SHA1(24a103dc6f0dc96f8d0f6164ad732909c9cd2d6a) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "mb-2m.4a",  0x000004, 0x80000, CRC(2ffbfea8) SHA1(13e30133664a009686e1114c92b558bdbb91ea32) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "mb-4m.6a",  0x000006, 0x80000, CRC(1eb9841d) SHA1(685da3e011a96b36be9f639a241b2f8f27da4629) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "mb-5m.7a",  0x200000, 0x80000, CRC(506b9dc9) SHA1(933bf2fb9bcc1a408f961f0e7052da80144bddad) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "mb-7m.9a",  0x200002, 0x80000, CRC(aff8c2fb) SHA1(ce37a6d5b1eb58c2d74f23f84ec824c214c93217) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "mb-6m.8a",  0x200004, 0x80000, CRC(b76c70e9) SHA1(c21e255815ec9a985919dbd760ed266c28bd47cd) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "mb-8m.10a", 0x200006, 0x80000, CRC(e60c9556) SHA1(b91c14092aa8dbb0922d96998123ef1970a658f6) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "mb-10m.3c", 0x400000, 0x80000, CRC(97976ff5) SHA1(ec9d3460816ab971a02fbce42960283091777e47) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "mb-12m.5c", 0x400002, 0x80000, CRC(b350a840) SHA1(2b8b996cd08051e7e8e134bff5448775d78058a0) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "mb-11m.4c", 0x400004, 0x80000, CRC(8fb94743) SHA1(294f6182c8a41b640d1f57cb5e3a2abce3b06482) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "mb-13m.6c", 0x400006, 0x80000, CRC(da810d5f) SHA1(392bbd405244b8c99024c9228cfec6a7ef0accdb) , ROM_GROUPWORD | ROM_SKIP(6) )

	ROM_REGION( 0x28000, "audiocpu", 0 ) /* QSound Z80 code */
	ROM_LOAD( "mb_q.5k",    0x00000, 0x08000, CRC(d6fa76d1) SHA1(3bfcb703e0e458ef1bb843230f8537167f1d4c3c) )
	ROM_CONTINUE(           0x10000, 0x18000 )

	ROM_REGION( 0x400000, "qsound", 0 ) /* QSound samples */
	ROM_LOAD( "mb-q1.1k",   0x000000, 0x80000, CRC(0630c3ce) SHA1(520fc74c5c3638f611fa2f1b5efb08b91747e29b) )
	ROM_LOAD( "mb-q2.2k",   0x080000, 0x80000, CRC(354f9c21) SHA1(1dc6b39791fd0f760697f409a6b62361a7bf62e9) )
	ROM_LOAD( "mb-q3.3k",   0x100000, 0x80000, CRC(7838487c) SHA1(056b7da05cfca46873edacd674ca25c70855c6db) )
	ROM_LOAD( "mb-q4.4k",   0x180000, 0x80000, CRC(ab66e087) SHA1(066ea69a0157e8647eea3c44d0a1843898860678) )
	ROM_LOAD( "mb-q5.1m",   0x200000, 0x80000, CRC(c789fef2) SHA1(10d1e3d92288fccd4e064a3716a788a165efc3c9) )
	ROM_LOAD( "mb-q6.2m",   0x280000, 0x80000, CRC(ecb81b61) SHA1(e339f21ae47de4782f3b338befcdac659c3503f6) )
	ROM_LOAD( "mb-q7.3m",   0x300000, 0x80000, CRC(041e49ba) SHA1(3220b033a5c0cfbbe75c0c113cf2db39fb093a7e) )
	ROM_LOAD( "mb-q8.4m",   0x380000, 0x80000, CRC(59fe702a) SHA1(807178dfc6d864e49fd7aabb5c4895835cf0e85b) )

	ROM_REGION( 0x0200, "aboardplds", 0 )
	ROM_LOAD( "buf1",         0x0000, 0x0117, CRC(eb122de7) SHA1(b26b5bfe258e3e184f069719f9fd008d6b8f6b9b) )
	ROM_LOAD( "ioa1",         0x0000, 0x0117, CRC(59c7ee3b) SHA1(fbb887c5b4f5cb8df77cec710eaac2985bc482a6) )
	ROM_LOAD( "prg2",         0x0000, 0x0117, CRC(4386879a) SHA1(c36896d169d8c78393609acbbe4397931292a033) )
	ROM_LOAD( "rom1",         0x0000, 0x0117, CRC(41dc73b9) SHA1(7d4c9f1693c821fbf84e32dd6ef62ddf14967845) )

	ROM_REGION( 0x0200, "bboardplds", 0 )
	ROM_LOAD( "mb63b.1a",     0x0000, 0x0117, CRC(b8392f02) SHA1(8dedf9f43b30991694f8009302ca628cb50a0b1a) )
	ROM_LOAD( "iob1.12d",     0x0000, 0x0117, CRC(3abc0700) SHA1(973043aa46ec6d5d1db20dc9d5937005a0f9f6ae) )
	ROM_LOAD( "bprg1.11d",    0x0000, 0x0117, CRC(31793da7) SHA1(400fa7ac517421c978c1ee7773c30b9ed0c5d3f3) )

	ROM_REGION( 0x0200, "cboardplds", 0 )
	ROM_LOAD( "ioc1.ic1",     0x0000, 0x0117, CRC(0d182081) SHA1(475b3d417785da4bc512cce2b274bb00d4cc6792) )

	ROM_REGION( 0x0200, "dboardplds", 0 )
	ROM_LOAD( "d7l1.7l",      0x0000, 0x0117, CRC(27b7410d) SHA1(06d0cba0226850f100ff1f539bd7d5db0f90c730) )
	ROM_LOAD( "d8l1.8l",      0x0000, 0x0117, CRC(539fc7da) SHA1(cad5c91629c6247e49ccbbcbfe6b08229eafae07) )
	ROM_LOAD( "d9k2.9k",      0x0000, 0x0117, CRC(cd85a156) SHA1(a88f8939c5d93e65d7bcc0eb3ee5b6f4f1114e3a) )    // pal verification required
	ROM_LOAD( "d10f1.10f",    0x0000, 0x0117, CRC(6619c494) SHA1(3aef656c07182a2186f810f30e0d854dd5bd8d18) )
ROM_END

/* B-Board 91634B-2 */
ROM_START( mbombrdj )
	ROM_REGION( CODE_SIZE, "maincpu", 0 )      /* 68000 code */
	ROM_LOAD16_BYTE( "mbdj_26.11e",     0x000000, 0x20000, CRC(72b7451c) SHA1(380ef57bb00f1c92d2f29e551b0a670eb5a56cb7) )   // == mbde_26.11e
	ROM_LOAD16_BYTE( "mbdj_30.11f",     0x000001, 0x20000, CRC(beff31cf) SHA1(8a3a1fa848fe8fad239d21aef1871e54bbcb582f) )
	ROM_LOAD16_BYTE( "mbdj_27.12e",     0x040000, 0x20000, CRC(4086f534) SHA1(a2b949f00035b06cb1cd01185902daca3d89d0e3) )   // == mbde_27.12e
	ROM_LOAD16_BYTE( "mbdj_31.12f",     0x040001, 0x20000, CRC(085f47f0) SHA1(ac93a196faf17b7dbe7179ce1e850d9cd7293a21) )   // == mbde_31.12f
	ROM_LOAD16_BYTE( "mbdj_24.9e",      0x080000, 0x20000, CRC(c20895a5) SHA1(35116f7ef8576753ec989647ca2f6a6131d6909f) )   // == mbde_24.9e
	ROM_LOAD16_BYTE( "mbdj_28.9f",      0x080001, 0x20000, CRC(2618d5e1) SHA1(50797c6dda04df95267ff9ef08933c17c3ce7057) )   // == mbde_28.9f
	ROM_LOAD16_BYTE( "mbdj_25.10e",     0x0c0000, 0x20000, CRC(9bdb6b11) SHA1(fbfbd6b5a72ca3237713ce43a798660f899b707d) )   // == mbde_25.10e
	ROM_LOAD16_BYTE( "mbdj_29.10f",     0x0c0001, 0x20000, CRC(3f52d5e5) SHA1(0b1ed8e876a6ec2cfb83676afe43a81e8a033e52) )   // == mbde_29.10f
	ROM_LOAD16_WORD_SWAP( "mbdj_21.6f", 0x100000, 0x80000, CRC(690c026a) SHA1(80ad780743b50750b6bfe1d4e28efe98e562233e) )   // == mbde_21.6f
	ROM_LOAD16_WORD_SWAP( "mbdj_20.5f", 0x180000, 0x80000, CRC(b8b2139b) SHA1(88c9169a9979b711ab7afb8272df0a1c80bb357b) )   // == mbde_20.5f

	ROM_REGION( 0x600000, "gfx", 0 )
	ROMX_LOAD( "mb_01.3a",  0x000000, 0x80000, CRC(a53b1c81) SHA1(d1efb88eeaf6e30e51aaf1432078003e52454dd9) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "mb_02.4a",  0x000002, 0x80000, CRC(23fe10f6) SHA1(deefa7cac4394b0642f7fb444f9374dbe0bc8843) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "mb_03.5a",  0x000004, 0x80000, CRC(cb866c2f) SHA1(b087f52e3b2a514a209612319d1d7c4f1c12b8bd) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "mb_04.6a",  0x000006, 0x80000, CRC(c9143e75) SHA1(e30090625ef6ac971a4f65d53f5458cebb5f146c) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "mb_05.7a",  0x200000, 0x80000, CRC(506b9dc9) SHA1(933bf2fb9bcc1a408f961f0e7052da80144bddad) , ROM_GROUPWORD | ROM_SKIP(6) ) // == mb-5m.7a
	ROMX_LOAD( "mb_06.8a",  0x200002, 0x80000, CRC(aff8c2fb) SHA1(ce37a6d5b1eb58c2d74f23f84ec824c214c93217) , ROM_GROUPWORD | ROM_SKIP(6) ) // == mb-7m.9a
	ROMX_LOAD( "mb_07.9a",  0x200004, 0x80000, CRC(b76c70e9) SHA1(c21e255815ec9a985919dbd760ed266c28bd47cd) , ROM_GROUPWORD | ROM_SKIP(6) ) // == mb-6m.8a
	ROMX_LOAD( "mb_08.10a", 0x200006, 0x80000, CRC(e60c9556) SHA1(b91c14092aa8dbb0922d96998123ef1970a658f6) , ROM_GROUPWORD | ROM_SKIP(6) ) // == mb-8m.10a
	ROMX_LOAD( "mb_10.3c",  0x400000, 0x80000, CRC(97976ff5) SHA1(ec9d3460816ab971a02fbce42960283091777e47) , ROM_GROUPWORD | ROM_SKIP(6) ) // == mb-10m.3c
	ROMX_LOAD( "mb_11.4c",  0x400002, 0x80000, CRC(b350a840) SHA1(2b8b996cd08051e7e8e134bff5448775d78058a0) , ROM_GROUPWORD | ROM_SKIP(6) ) // == mb-12m.5c
	ROMX_LOAD( "mb_12.5c",  0x400004, 0x80000, CRC(8fb94743) SHA1(294f6182c8a41b640d1f57cb5e3a2abce3b06482) , ROM_GROUPWORD | ROM_SKIP(6) ) // == mb-11m.4c
	ROMX_LOAD( "mb_13.6c",  0x400006, 0x80000, CRC(da810d5f) SHA1(392bbd405244b8c99024c9228cfec6a7ef0accdb) , ROM_GROUPWORD | ROM_SKIP(6) ) // == mb-13m.6c

	ROM_REGION( 0x28000, "audiocpu", 0 ) /* QSound Z80 code */
	ROM_LOAD( "mb_qa.5k",   0x00000, 0x08000, CRC(e21a03c4) SHA1(98c03fd2c9b6bf8a4fc25a4edca87fff7c3c3819) )
	ROM_CONTINUE(           0x10000, 0x18000 )

	ROM_REGION( 0x400000, "qsound", 0 ) /* QSound samples */
	ROM_LOAD( "mb-q1.1k",   0x000000, 0x80000, CRC(0630c3ce) SHA1(520fc74c5c3638f611fa2f1b5efb08b91747e29b) )
	ROM_LOAD( "mb-q2.2k",   0x080000, 0x80000, CRC(354f9c21) SHA1(1dc6b39791fd0f760697f409a6b62361a7bf62e9) )
	ROM_LOAD( "mb-q3.3k",   0x100000, 0x80000, CRC(7838487c) SHA1(056b7da05cfca46873edacd674ca25c70855c6db) )
	ROM_LOAD( "mb-q4.4k",   0x180000, 0x80000, CRC(ab66e087) SHA1(066ea69a0157e8647eea3c44d0a1843898860678) )
	ROM_LOAD( "mb-q5.1m",   0x200000, 0x80000, CRC(c789fef2) SHA1(10d1e3d92288fccd4e064a3716a788a165efc3c9) )
	ROM_LOAD( "mb-q6.2m",   0x280000, 0x80000, CRC(ecb81b61) SHA1(e339f21ae47de4782f3b338befcdac659c3503f6) )
	ROM_LOAD( "mb-q7.3m",   0x300000, 0x80000, CRC(041e49ba) SHA1(3220b033a5c0cfbbe75c0c113cf2db39fb093a7e) )
	ROM_LOAD( "mb-q8.4m",   0x380000, 0x80000, CRC(59fe702a) SHA1(807178dfc6d864e49fd7aabb5c4895835cf0e85b) )

	ROM_REGION( 0x0200, "aboardplds", 0 )
	ROM_LOAD( "buf1",         0x0000, 0x0117, CRC(eb122de7) SHA1(b26b5bfe258e3e184f069719f9fd008d6b8f6b9b) )
	ROM_LOAD( "ioa1",         0x0000, 0x0117, CRC(59c7ee3b) SHA1(fbb887c5b4f5cb8df77cec710eaac2985bc482a6) )
	ROM_LOAD( "prg2",         0x0000, 0x0117, CRC(4386879a) SHA1(c36896d169d8c78393609acbbe4397931292a033) )
	ROM_LOAD( "rom1",         0x0000, 0x0117, CRC(41dc73b9) SHA1(7d4c9f1693c821fbf84e32dd6ef62ddf14967845) )

	ROM_REGION( 0x0200, "bboardplds", 0 )
	ROM_LOAD( "mb63b.1a",     0x0000, 0x0117, CRC(b8392f02) SHA1(8dedf9f43b30991694f8009302ca628cb50a0b1a) )
	ROM_LOAD( "iob1.12d",     0x0000, 0x0117, CRC(3abc0700) SHA1(973043aa46ec6d5d1db20dc9d5937005a0f9f6ae) )
	ROM_LOAD( "bprg1.11d",    0x0000, 0x0117, CRC(31793da7) SHA1(400fa7ac517421c978c1ee7773c30b9ed0c5d3f3) )

	ROM_REGION( 0x0200, "cboardplds", 0 )
	ROM_LOAD( "ioc1.ic1",     0x0000, 0x0117, CRC(0d182081) SHA1(475b3d417785da4bc512cce2b274bb00d4cc6792) )

	ROM_REGION( 0x0200, "dboardplds", 0 )
	ROM_LOAD( "d7l1.7l",      0x0000, 0x0117, CRC(27b7410d) SHA1(06d0cba0226850f100ff1f539bd7d5db0f90c730) )
	ROM_LOAD( "d8l1.8l",      0x0000, 0x0117, CRC(539fc7da) SHA1(cad5c91629c6247e49ccbbcbfe6b08229eafae07) )
	ROM_LOAD( "d9k2.9k",      0x0000, 0x0117, CRC(cd85a156) SHA1(a88f8939c5d93e65d7bcc0eb3ee5b6f4f1114e3a) )
	ROM_LOAD( "d10f1.10f",    0x0000, 0x0117, CRC(6619c494) SHA1(3aef656c07182a2186f810f30e0d854dd5bd8d18) )
ROM_END

/* B-Board 89625B-1 */
ROM_START( pnickj )
	ROM_REGION( CODE_SIZE, "maincpu", 0 )      /* 68000 code */
	ROM_LOAD16_BYTE( "pnij_36.12f", 0x00000, 0x20000, CRC(2d4ffb2b) SHA1(6e49cf89a36834fd1de8b4b860fe66f3d7d67a84) )
	ROM_LOAD16_BYTE( "pnij_42.12h", 0x00001, 0x20000, CRC(c085dfaf) SHA1(a31ededc3413ec4f3f5e3a1fb615b60c6197f4a5) )

	ROM_REGION( 0x200000, "gfx", 0 )
	ROMX_LOAD( "pnij_09.4b",  0x000000, 0x20000, CRC(48177b0a) SHA1(eba5de6cd9bb0c4ad76a13bddc9cdeb2e4380122) , ROM_SKIP(7) )
	ROMX_LOAD( "pnij_01.4a",  0x000001, 0x20000, CRC(01a0f311) SHA1(9bcd8716f90ccd410543ffcdc5c2916077b8d4c3) , ROM_SKIP(7) )
	ROMX_LOAD( "pnij_13.9b",  0x000002, 0x20000, CRC(406451b0) SHA1(5a7a7fecba7de8b8cf4a284b2ae7adae901623f6) , ROM_SKIP(7) )
	ROMX_LOAD( "pnij_05.9a",  0x000003, 0x20000, CRC(8c515dc0) SHA1(aa1e13cf9e7cf0458bb5c4332b1ea73034f9a874) , ROM_SKIP(7) )
	ROMX_LOAD( "pnij_26.5e",  0x000004, 0x20000, CRC(e2af981e) SHA1(3c2b28b4a4d457aa94a760dfca0181a9f050c319) , ROM_SKIP(7) )
	ROMX_LOAD( "pnij_18.5c",  0x000005, 0x20000, CRC(f17a0e56) SHA1(7c89aca230f176e12f995892f9d1bce22c57fbdf) , ROM_SKIP(7) )
	ROMX_LOAD( "pnij_38.8h",  0x000006, 0x20000, CRC(eb75bd8c) SHA1(2129460e06eb64019fc5f7eab6334ff43229b995) , ROM_SKIP(7) )
	ROMX_LOAD( "pnij_32.8f",  0x000007, 0x20000, CRC(84560bef) SHA1(9e94ae434b50ecf82781080e11d0c4741e992d0d) , ROM_SKIP(7) )
	ROMX_LOAD( "pnij_10.5b",  0x100000, 0x20000, CRC(c2acc171) SHA1(7c86db3f2acca1252d403c5f12c871d0357fa109) , ROM_SKIP(7) )
	ROMX_LOAD( "pnij_02.5a",  0x100001, 0x20000, CRC(0e21fc33) SHA1(c4a29d45c4257c8871038d3c9b13140e874db0c1) , ROM_SKIP(7) )
	ROMX_LOAD( "pnij_14.10b", 0x100002, 0x20000, CRC(7fe59b19) SHA1(a273b8b8fbfd5d31d25479a9ede09ce35e1cc873) , ROM_SKIP(7) )
	ROMX_LOAD( "pnij_06.10a", 0x100003, 0x20000, CRC(79f4bfe3) SHA1(bc17cc1c8535e3d202588893713926b6c06f92fd) , ROM_SKIP(7) )
	ROMX_LOAD( "pnij_27.7e",  0x100004, 0x20000, CRC(83d5cb0e) SHA1(44c93fa5eedcafc8dc6d88ee827c6cadc9c671f0) , ROM_SKIP(7) )
	ROMX_LOAD( "pnij_19.7c",  0x100005, 0x20000, CRC(af08b230) SHA1(a3b5b3013012efa1860699648518f8d8031c5f30) , ROM_SKIP(7) )
	ROMX_LOAD( "pnij_39.9h",  0x100006, 0x20000, CRC(70fbe579) SHA1(b5b7ed5588ecd884b20dd50bfc5385a9af03c5d8) , ROM_SKIP(7) )
	ROMX_LOAD( "pnij_33.9f",  0x100007, 0x20000, CRC(3ed2c680) SHA1(0afe84d8d89f8d45afc79f6172337e622e29a8a2) , ROM_SKIP(7) )

	ROM_REGION( 0x18000, "audiocpu", 0 ) /* 64k for the audio CPU (+banks) */
	ROM_LOAD( "pnij_17.13b",  0x00000, 0x08000, CRC(e86f787a) SHA1(de04cbe89c655faf04afe169bfd9913049ccc4a8) )
	ROM_CONTINUE(             0x10000, 0x08000 )

	ROM_REGION( 0x40000, "oki", 0 ) /* Samples */
	ROM_LOAD( "pnij_24.12c",  0x00000, 0x20000, CRC(5092257d) SHA1(95dc9d10940653b2fb37baf5c1ed27145b02104e) )
	ROM_LOAD( "pnij_25.13c",  0x20000, 0x20000, CRC(22109aaa) SHA1(cf21e75674d81b2daae2083d02f9f4b6e52722c6) )

	ROM_REGION( 0x0200, "aboardplds", 0 )
	ROM_LOAD( "buf1",         0x0000, 0x0117, CRC(eb122de7) SHA1(b26b5bfe258e3e184f069719f9fd008d6b8f6b9b) )
	ROM_LOAD( "ioa1",         0x0000, 0x0117, CRC(59c7ee3b) SHA1(fbb887c5b4f5cb8df77cec710eaac2985bc482a6) )
	ROM_LOAD( "prg1",         0x0000, 0x0117, CRC(f1129744) SHA1(a5300f301c1a08a7da768f0773fa0fe3f683b237) )
	ROM_LOAD( "rom1",         0x0000, 0x0117, CRC(41dc73b9) SHA1(7d4c9f1693c821fbf84e32dd6ef62ddf14967845) )
	ROM_LOAD( "sou1",         0x0000, 0x0117, CRC(84f4b2fe) SHA1(dcc9e86cc36316fe42eace02d6df75d08bc8bb6d) )

	ROM_REGION( 0x0200, "bboardplds", 0 )
	ROM_LOAD( "pkb10b.1a",    0x0000, 0x0117, NO_DUMP )
	ROM_LOAD( "iob1.12e",     0x0000, 0x0117, CRC(3abc0700) SHA1(973043aa46ec6d5d1db20dc9d5937005a0f9f6ae) )

	ROM_REGION( 0x0200, "cboardplds", 0 )
	ROM_LOAD( "ioc1.ic7",     0x0000, 0x0117, CRC(0d182081) SHA1(475b3d417785da4bc512cce2b274bb00d4cc6792) )
	ROM_LOAD( "c632.ic1",     0x0000, 0x0117, CRC(0fbd9270) SHA1(d7e737b20c44d41e29ca94be56114b31934dde81) )
ROM_END

/* B-Board 90629B-3 */
ROM_START( qtono2j )
	ROM_REGION( CODE_SIZE, "maincpu", 0 )      /* 68000 code */
	ROM_LOAD16_BYTE( "tn2j_30.11e", 0x00000, 0x20000, CRC(9226eb5e) SHA1(91649974f9652caed90eb28ec6caf4fe61f5d279) )
	ROM_LOAD16_BYTE( "tn2j_37.11f", 0x00001, 0x20000, CRC(d1d30da1) SHA1(7ca1695ed804b5860d4c15964cdbb922db3918ee) )
	ROM_LOAD16_BYTE( "tn2j_31.12e", 0x40000, 0x20000, CRC(015e6a8a) SHA1(0835bec4867438a167bd01e3550090c88e7ae779) )
	ROM_LOAD16_BYTE( "tn2j_38.12f", 0x40001, 0x20000, CRC(1f139bcc) SHA1(ee907f1bfef1a887e2c768648fe811e0733eddf7) )
	ROM_LOAD16_BYTE( "tn2j_28.9e",  0x80000, 0x20000, CRC(86d27f71) SHA1(89d6d18e05deaaa1ac7deb70ca03d051d2fde472) )
	ROM_LOAD16_BYTE( "tn2j_35.9f",  0x80001, 0x20000, CRC(7a1ab87d) SHA1(f1729a8c0c82cf42f60644a7796dc8a39bf7c6fa) )
	ROM_LOAD16_BYTE( "tn2j_29.10e", 0xc0000, 0x20000, CRC(9c384e99) SHA1(3d3961f625ccc4776531eff50fc1b4bee062370e) )
	ROM_LOAD16_BYTE( "tn2j_36.10f", 0xc0001, 0x20000, CRC(4c4b2a0a) SHA1(9a25fcfb9358ea42d9bc662df2cafea08febb411) )

	ROM_REGION( 0x400000, "gfx", 0 )
	ROMX_LOAD( "tn2-02m.4a", 0x000000, 0x80000, CRC(f2016a34) SHA1(3862960fa14742547f6a6deacf0b9f409d08fee8) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "tn2-04m.6a", 0x000002, 0x80000, CRC(094e0fb1) SHA1(7c9a9a7d03e226109002dd389c872e3d4be43287) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "tn2-01m.3a", 0x000004, 0x80000, CRC(cb950cf9) SHA1(8337a500141c1aec82b6636ad79ecafbdbebd691) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "tn2-03m.5a", 0x000006, 0x80000, CRC(18a5bf59) SHA1(afbfcb28c40551747bb5276aac2b9c15a24328e1) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "tn2-11m.4c", 0x200000, 0x80000, CRC(d0edd30b) SHA1(a76d7f134f9e52f79a485402d17dcc7a1fe99f29) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "tn2-13m.6c", 0x200002, 0x80000, CRC(426621c3) SHA1(89156bc9d585f546cd619db419dd1f4d9871d930) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "tn2-10m.3c", 0x200004, 0x80000, CRC(a34ece70) SHA1(15864d6b280f624245add8a611f1699da570392b) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "tn2-12m.5c", 0x200006, 0x80000, CRC(e04ff2f4) SHA1(774c19909a2ae2c691f5d3f15b6e19cc94baf799) , ROM_GROUPWORD | ROM_SKIP(6) )

	ROM_REGION( 0x18000, "audiocpu", 0 ) /* 64k for the audio CPU (+banks) */
	ROM_LOAD( "tn2j_09.12a", 0x00000, 0x08000, CRC(e464b969) SHA1(18a0b9fd3a1eb4d1364b0e8601cd49a9574406c8) )
	ROM_CONTINUE(            0x10000, 0x08000 )

	ROM_REGION( 0x40000, "oki", 0 ) /* Samples */
	ROM_LOAD( "tn2j_18.11c", 0x00000, 0x20000, CRC(a40bf9a7) SHA1(07cb1076262a281e31a621cbcc10be0cae883175) )
	ROM_LOAD( "tn2j_19.12c", 0x20000, 0x20000, CRC(5b3b931e) SHA1(cf28891f84814cbfaa3adaade8bb08b1e0546a3d) )

	ROM_REGION( 0x0200, "aboardplds", 0 )
	ROM_LOAD( "buf1",         0x0000, 0x0117, CRC(eb122de7) SHA1(b26b5bfe258e3e184f069719f9fd008d6b8f6b9b) )
	ROM_LOAD( "ioa1",         0x0000, 0x0117, CRC(59c7ee3b) SHA1(fbb887c5b4f5cb8df77cec710eaac2985bc482a6) )
	ROM_LOAD( "prg1",         0x0000, 0x0117, CRC(f1129744) SHA1(a5300f301c1a08a7da768f0773fa0fe3f683b237) )
	ROM_LOAD( "rom1",         0x0000, 0x0117, CRC(41dc73b9) SHA1(7d4c9f1693c821fbf84e32dd6ef62ddf14967845) )
	ROM_LOAD( "sou1",         0x0000, 0x0117, CRC(84f4b2fe) SHA1(dcc9e86cc36316fe42eace02d6df75d08bc8bb6d) )

	ROM_REGION( 0x0200, "bboardplds", 0 )
	ROM_LOAD( "tn2292.1a",    0x0000, 0x0117, CRC(3d899539) SHA1(9ffa76229408a2d7438b1b3efa0ec7166bfcc751) )
	ROM_LOAD( "iob1.11d",     0x0000, 0x0117, CRC(3abc0700) SHA1(973043aa46ec6d5d1db20dc9d5937005a0f9f6ae) )

	ROM_REGION( 0x0200, "cboardplds", 0 )
	ROM_LOAD( "ioc1.ic7",     0x0000, 0x0117, CRC(0d182081) SHA1(475b3d417785da4bc512cce2b274bb00d4cc6792) )
	ROM_LOAD( "c632.ic1",     0x0000, 0x0117, CRC(0fbd9270) SHA1(d7e737b20c44d41e29ca94be56114b31934dde81) )
ROM_END

/* B-Board 94916-10 */
ROM_START( pang3 )
	ROM_REGION( CODE_SIZE, "maincpu", 0 )      /* 68000 code */
	ROM_LOAD16_WORD_SWAP( "pa3e_17a.11l", 0x00000, 0x80000, CRC(a213fa80) SHA1(fee6b0b21e2ef573ebfb35eaa1785533101c9170) )
	ROM_LOAD16_WORD_SWAP( "pa3e_16a.10l", 0x80000, 0x80000, CRC(7169ea67) SHA1(1076afaff7b05c9009798a0cea13e71fa27f2b7e) )

	ROM_REGION( 0x400000, "gfx", 0 )
	ROMX_LOAD( "pa3-01m.2c", 0x000000, 0x100000, CRC(068a152c) SHA1(fa491874068924c39bcc7de93dfda3b27f5d9613) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROM_CONTINUE(            0x000004, 0x100000 )
	ROMX_LOAD( "pa3-07m.2f", 0x000002, 0x100000, CRC(3a4a619d) SHA1(cfe68e24632b53fb6cd6d03b2166d6b5ba28b778) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROM_CONTINUE(            0x000006, 0x100000 )

	ROM_REGION( 0x18000, "audiocpu", 0 ) /* 64k for the audio CPU (+banks) */
	ROM_LOAD( "pa3_11.11f",  0x00000, 0x08000, CRC(cb1423a2) SHA1(3191bf5d340168647881738cb2aed09b1d86146e) )
	ROM_IGNORE( 0x18000 )

	ROM_REGION( 0x40000, "oki", 0 ) /* Samples */
	ROM_LOAD( "pa3_05.10d",  0x00000, 0x20000, CRC(73a10d5d) SHA1(999465e4fbc35a34746d2db61ad49f61403d5af7) )
	ROM_LOAD( "pa3_06.11d",  0x20000, 0x20000, CRC(affa4f82) SHA1(27b9292bbc121cf585f53297a79fe8f0d0a729ae) )

	ROM_REGION( 0x0200, "aboardplds", 0 )
	ROM_LOAD( "buf1",         0x0000, 0x0117, CRC(eb122de7) SHA1(b26b5bfe258e3e184f069719f9fd008d6b8f6b9b) )
	ROM_LOAD( "ioa1",         0x0000, 0x0117, CRC(59c7ee3b) SHA1(fbb887c5b4f5cb8df77cec710eaac2985bc482a6) )
	ROM_LOAD( "prg1",         0x0000, 0x0117, CRC(f1129744) SHA1(a5300f301c1a08a7da768f0773fa0fe3f683b237) )
	ROM_LOAD( "rom1",         0x0000, 0x0117, CRC(41dc73b9) SHA1(7d4c9f1693c821fbf84e32dd6ef62ddf14967845) )
	ROM_LOAD( "sou1",         0x0000, 0x0117, CRC(84f4b2fe) SHA1(dcc9e86cc36316fe42eace02d6df75d08bc8bb6d) )

	ROM_REGION( 0x0200, "bboardplds", 0 )
	ROM_LOAD( "cp1b1f.1f",    0x0000, 0x0117, CRC(3979b8e3) SHA1(07c9819d68b4d93bc37b96bd15d689ce54fe034e) )
	ROM_LOAD( "cp1b8k.8k",    0x0000, 0x0117, CRC(8a52ea7a) SHA1(47a59abc54a83292cfd6faa2d293c8f948c7ea03) )
	ROM_LOAD( "cp1b9ka.9k",   0x0000, 0x0117, CRC(238d3ff4) SHA1(597f429d6a0ea485746322592604188c1ec87595) )

	ROM_REGION( 0x0200, "cboardplds", 0 )
	ROM_LOAD( "ioc1.ic7",     0x0000, 0x0117, CRC(0d182081) SHA1(475b3d417785da4bc512cce2b274bb00d4cc6792) )
	ROM_LOAD( "c632.ic1",     0x0000, 0x0117, CRC(0fbd9270) SHA1(d7e737b20c44d41e29ca94be56114b31934dde81) )
ROM_END

/* B-Board 94916-10 */
ROM_START( pang3r1 )
	ROM_REGION( CODE_SIZE, "maincpu", 0 )      /* 68000 code */
	ROM_LOAD16_WORD_SWAP( "pa3e_17.11l", 0x00000, 0x80000, CRC(d7041d32) SHA1(b021f3defe7fc58030ba907125c713f987724187) )
	ROM_LOAD16_WORD_SWAP( "pa3e_16.10l", 0x80000, 0x80000, CRC(1be9a483) SHA1(6cff1dd15ca163237bc82fb4a3e1d469d35e7be8) )

	ROM_REGION( 0x400000, "gfx", 0 )
	ROMX_LOAD( "pa3-01m.2c", 0x000000, 0x100000, CRC(068a152c) SHA1(fa491874068924c39bcc7de93dfda3b27f5d9613) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROM_CONTINUE(            0x000004, 0x100000 )
	ROMX_LOAD( "pa3-07m.2f", 0x000002, 0x100000, CRC(3a4a619d) SHA1(cfe68e24632b53fb6cd6d03b2166d6b5ba28b778) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROM_CONTINUE(            0x000006, 0x100000 )

	ROM_REGION( 0x18000, "audiocpu", 0 ) /* 64k for the audio CPU (+banks) */
	ROM_LOAD( "pa3_11.11f",  0x00000, 0x08000, CRC(cb1423a2) SHA1(3191bf5d340168647881738cb2aed09b1d86146e) )
	ROM_IGNORE( 0x18000 )

	ROM_REGION( 0x40000, "oki", 0 ) /* Samples */
	ROM_LOAD( "pa3_05.10d",  0x00000, 0x20000, CRC(73a10d5d) SHA1(999465e4fbc35a34746d2db61ad49f61403d5af7) )
	ROM_LOAD( "pa3_06.11d",  0x20000, 0x20000, CRC(affa4f82) SHA1(27b9292bbc121cf585f53297a79fe8f0d0a729ae) )

	ROM_REGION( 0x0200, "aboardplds", 0 )
	ROM_LOAD( "buf1",         0x0000, 0x0117, CRC(eb122de7) SHA1(b26b5bfe258e3e184f069719f9fd008d6b8f6b9b) )
	ROM_LOAD( "ioa1",         0x0000, 0x0117, CRC(59c7ee3b) SHA1(fbb887c5b4f5cb8df77cec710eaac2985bc482a6) )
	ROM_LOAD( "prg1",         0x0000, 0x0117, CRC(f1129744) SHA1(a5300f301c1a08a7da768f0773fa0fe3f683b237) )
	ROM_LOAD( "rom1",         0x0000, 0x0117, CRC(41dc73b9) SHA1(7d4c9f1693c821fbf84e32dd6ef62ddf14967845) )
	ROM_LOAD( "sou1",         0x0000, 0x0117, CRC(84f4b2fe) SHA1(dcc9e86cc36316fe42eace02d6df75d08bc8bb6d) )

	ROM_REGION( 0x0200, "bboardplds", 0 )
	ROM_LOAD( "cp1b1f.1f",    0x0000, 0x0117, CRC(3979b8e3) SHA1(07c9819d68b4d93bc37b96bd15d689ce54fe034e) )
	ROM_LOAD( "cp1b8k.8k",    0x0000, 0x0117, CRC(8a52ea7a) SHA1(47a59abc54a83292cfd6faa2d293c8f948c7ea03) )
	ROM_LOAD( "cp1b9k.9k",    0x0000, 0x0117, CRC(a754bdc3) SHA1(9267b24cbddee4858b219468cc92f9df8f5fd0ef) )

	ROM_REGION( 0x0200, "cboardplds", 0 )
	ROM_LOAD( "ioc1.ic7",     0x0000, 0x0117, CRC(0d182081) SHA1(475b3d417785da4bc512cce2b274bb00d4cc6792) )
	ROM_LOAD( "c632.ic1",     0x0000, 0x0117, CRC(0fbd9270) SHA1(d7e737b20c44d41e29ca94be56114b31934dde81) )
ROM_END

/* B-Board 94916-10 */
ROM_START( pang3j )
	ROM_REGION( CODE_SIZE, "maincpu", 0 )      /* 68000 code */
	ROM_LOAD16_WORD_SWAP( "pa3j_17.11l", 0x00000, 0x80000, CRC(21f6e51f) SHA1(b447e05261f59b3b2e89bbc0f606d7136b29cb56) )
	ROM_LOAD16_WORD_SWAP( "pa3j_16.10l", 0x80000, 0x80000, CRC(ca1d7897) SHA1(46aa9232e81a838f3eff1e9b992492a264914fd5) )

	ROM_REGION( 0x400000, "gfx", 0 )
	ROMX_LOAD( "pa3-01m.2c", 0x000000, 0x100000, CRC(068a152c) SHA1(fa491874068924c39bcc7de93dfda3b27f5d9613) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROM_CONTINUE(            0x000004, 0x100000 )
	ROMX_LOAD( "pa3-07m.2f", 0x000002, 0x100000, CRC(3a4a619d) SHA1(cfe68e24632b53fb6cd6d03b2166d6b5ba28b778) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROM_CONTINUE(            0x000006, 0x100000 )

	ROM_REGION( 0x18000, "audiocpu", 0 ) /* 64k for the audio CPU (+banks) */
	ROM_LOAD( "pa3_11.11f",  0x00000, 0x08000, CRC(cb1423a2) SHA1(3191bf5d340168647881738cb2aed09b1d86146e) )
	ROM_IGNORE( 0x18000 )

	ROM_REGION( 0x40000, "oki", 0 ) /* Samples */
	ROM_LOAD( "pa3_05.10d",  0x00000, 0x20000, CRC(73a10d5d) SHA1(999465e4fbc35a34746d2db61ad49f61403d5af7) )
	ROM_LOAD( "pa3_06.11d",  0x20000, 0x20000, CRC(affa4f82) SHA1(27b9292bbc121cf585f53297a79fe8f0d0a729ae) )

	ROM_REGION( 0x0200, "aboardplds", 0 )
	ROM_LOAD( "buf1",         0x0000, 0x0117, CRC(eb122de7) SHA1(b26b5bfe258e3e184f069719f9fd008d6b8f6b9b) )
	ROM_LOAD( "ioa1",         0x0000, 0x0117, CRC(59c7ee3b) SHA1(fbb887c5b4f5cb8df77cec710eaac2985bc482a6) )
	ROM_LOAD( "prg1",         0x0000, 0x0117, CRC(f1129744) SHA1(a5300f301c1a08a7da768f0773fa0fe3f683b237) )
	ROM_LOAD( "rom1",         0x0000, 0x0117, CRC(41dc73b9) SHA1(7d4c9f1693c821fbf84e32dd6ef62ddf14967845) )
	ROM_LOAD( "sou1",         0x0000, 0x0117, CRC(84f4b2fe) SHA1(dcc9e86cc36316fe42eace02d6df75d08bc8bb6d) )

	ROM_REGION( 0x0200, "bboardplds", 0 )
	ROM_LOAD( "cp1b1f.1f",    0x0000, 0x0117, CRC(3979b8e3) SHA1(07c9819d68b4d93bc37b96bd15d689ce54fe034e) )
	ROM_LOAD( "cp1b8k.8k",    0x0000, 0x0117, CRC(8a52ea7a) SHA1(47a59abc54a83292cfd6faa2d293c8f948c7ea03) )
	ROM_LOAD( "cp1b9k.9k",    0x0000, 0x0117, CRC(a754bdc3) SHA1(9267b24cbddee4858b219468cc92f9df8f5fd0ef) )

	ROM_REGION( 0x0200, "cboardplds", 0 )
	ROM_LOAD( "ioc1.ic7",     0x0000, 0x0117, CRC(0d182081) SHA1(475b3d417785da4bc512cce2b274bb00d4cc6792) )
	ROM_LOAD( "c632.ic1",     0x0000, 0x0117, CRC(0fbd9270) SHA1(d7e737b20c44d41e29ca94be56114b31934dde81) )
ROM_END

/* B-Board 94916-10 */
/* This set comes from a not encrypted bootleg that uses a very well reproduced Mitchell 94916-10 B-Board surmounted by an
   original Capcom 92631C-6 C-Board. Mitchell mark has been erased, original 32 pin EPROM PA3_11.11F resized and replaced by
   one with 28 pin while the protection chip MACH215 mounted on the B-Board has been replaced by a 22 pin PAL. */
ROM_START( pang3b )
	ROM_REGION( CODE_SIZE, "maincpu", 0 )      /* 68000 code */
	ROM_LOAD16_WORD_SWAP( "pa3w_17.11l", 0x00000, 0x80000, CRC(12138234) SHA1(956a2c847a3cfb94007d1a636167fd2bb9f826ec) )
	ROM_LOAD16_WORD_SWAP( "pa3w_16.10l", 0x80000, 0x80000, CRC(d1ba585c) SHA1(c6d04441fe97abf0a72b23c917777a7b58e94a85) )

	ROM_REGION( 0x400000, "gfx", 0 )
	ROMX_LOAD( "pa3-01m.2c", 0x000000, 0x100000, CRC(068a152c) SHA1(fa491874068924c39bcc7de93dfda3b27f5d9613) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROM_CONTINUE(            0x000004, 0x100000 )
	ROMX_LOAD( "pa3-07m.2f", 0x000002, 0x100000, CRC(3a4a619d) SHA1(cfe68e24632b53fb6cd6d03b2166d6b5ba28b778) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROM_CONTINUE(            0x000006, 0x100000 )

	ROM_REGION( 0x18000, "audiocpu", 0 ) /* 64k for the audio CPU (+banks) */
	ROM_LOAD( "pa3_11.11f",  0x00000, 0x08000, CRC(90a08c46) SHA1(7544adab2d7e052e0d21c920bff7841d9d718345) )  // == pa3_11.11f but different size, resized by bootlegger

	ROM_REGION( 0x40000, "oki", 0 ) /* Samples */
	ROM_LOAD( "pa3_05.10d",  0x00000, 0x20000, CRC(73a10d5d) SHA1(999465e4fbc35a34746d2db61ad49f61403d5af7) )
	ROM_LOAD( "pa3_06.11d",  0x20000, 0x20000, CRC(affa4f82) SHA1(27b9292bbc121cf585f53297a79fe8f0d0a729ae) )

	ROM_REGION( 0x0200, "aboardplds", 0 )
	ROM_LOAD( "buf1",         0x0000, 0x0117, CRC(eb122de7) SHA1(b26b5bfe258e3e184f069719f9fd008d6b8f6b9b) )
	ROM_LOAD( "ioa1",         0x0000, 0x0117, CRC(59c7ee3b) SHA1(fbb887c5b4f5cb8df77cec710eaac2985bc482a6) )
	ROM_LOAD( "prg1",         0x0000, 0x0117, CRC(f1129744) SHA1(a5300f301c1a08a7da768f0773fa0fe3f683b237) )
	ROM_LOAD( "rom1",         0x0000, 0x0117, CRC(41dc73b9) SHA1(7d4c9f1693c821fbf84e32dd6ef62ddf14967845) )
	ROM_LOAD( "sou1",         0x0000, 0x0117, CRC(84f4b2fe) SHA1(dcc9e86cc36316fe42eace02d6df75d08bc8bb6d) )

	ROM_REGION( 0x0200, "bboardplds", 0 )
	ROM_LOAD( "cp1b1f.1f",    0x0000, 0x0117, CRC(3979b8e3) SHA1(07c9819d68b4d93bc37b96bd15d689ce54fe034e) )
	ROM_LOAD( "cp1b8k.8k",    0x0000, 0x0117, CRC(8a52ea7a) SHA1(47a59abc54a83292cfd6faa2d293c8f948c7ea03) )
	ROM_LOAD( "cp1b9k.9k",    0x0000, 0x0117, CRC(a754bdc3) SHA1(9267b24cbddee4858b219468cc92f9df8f5fd0ef) )

	ROM_REGION( 0x0200, "cboardplds", 0 )
	ROM_LOAD( "ioc1.ic7",     0x0000, 0x0117, CRC(0d182081) SHA1(475b3d417785da4bc512cce2b274bb00d4cc6792) )
	ROM_LOAD( "c632.ic1",     0x0000, 0x0117, CRC(0fbd9270) SHA1(d7e737b20c44d41e29ca94be56114b31934dde81) )
ROM_END

/* B-Board 91635B-2 */
/* Note that this USA set seems to be the only one where GFX are stored into EPROMs instead of the usual MASK ROMs. */
ROM_START( megaman )
	ROM_REGION( CODE_SIZE, "maincpu", 0 )      /* 68000 code */
	ROM_LOAD16_WORD_SWAP( "rcmu_23b.8f", 0x000000, 0x80000, CRC(1cd33c7a) SHA1(687fb3b6d660d7350447193f1911c47972e7a020) )
	ROM_LOAD16_WORD_SWAP( "rcmu_22b.7f", 0x080000, 0x80000, CRC(708268c4) SHA1(554e011cad285b95dd1b6aa19be61b2413662a3a) )
	ROM_LOAD16_WORD_SWAP( "rcmu_21a.6f", 0x100000, 0x80000, CRC(4376ea95) SHA1(7370ceffca513aa9f68a74f6869d561476589200) )

	ROM_REGION( 0x800000, "gfx", 0 )
	ROMX_LOAD( "rcm_01.3a",  0x000000, 0x80000, CRC(6ecdf13f) SHA1(2a8fe06bf5011e3f990f90d9224f91d8631ec0cc) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "rcm_02.4a",  0x000002, 0x80000, CRC(944d4f0f) SHA1(665dc9a537e9c9b565f6136f939ff5c2861f875f) , ROM_GROUPWORD | ROM_SKIP(6) )    // Dumped as rcm_03.5a
	ROMX_LOAD( "rcm_03.5a",  0x000004, 0x80000, CRC(36f3073c) SHA1(457d68e63599d06a136e152a9ad60adac1c91edd) , ROM_GROUPWORD | ROM_SKIP(6) )    // Dumped as rcm_02.4a
	ROMX_LOAD( "rcm_04.6a",  0x000006, 0x80000, CRC(54e622ff) SHA1(36f6297e3d410f041be5e582919478b0d52520ca) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "rcm_05.7a",  0x200000, 0x80000, CRC(5dd131fd) SHA1(1a7fc8cf38901245d40901996e946e7ad9c0e0c5) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "rcm_06.8a",  0x200002, 0x80000, CRC(f0faf813) SHA1(adff01c2ecc4c8ce6f8a50cbd07d8f8bb9f48168) , ROM_GROUPWORD | ROM_SKIP(6) )    // Dumped as rcm_07.9a
	ROMX_LOAD( "rcm_07.9a",  0x200004, 0x80000, CRC(826de013) SHA1(47f36b1d92a487c43c8dadc8293b8e6f40649286) , ROM_GROUPWORD | ROM_SKIP(6) )    // Dumped as rcm_06.8a
	ROMX_LOAD( "rcm_08.10a", 0x200006, 0x80000, CRC(fbff64cf) SHA1(f0cb531ef195dc1dcd224a208906a62fb5d199a1) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "rcm_10.3c",  0x400000, 0x80000, CRC(4dc8ada9) SHA1(776c2b3ef24c2b8f390c05a9c6728b14ceec696e) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "rcm_11.4c",  0x400002, 0x80000, CRC(f2b9ee06) SHA1(db315b00d1caed1a8c0f6e0ae726e8fa05b011fa) , ROM_GROUPWORD | ROM_SKIP(6) )    // Dumped as rcm_12.5c
	ROMX_LOAD( "rcm_12.5c",  0x400004, 0x80000, CRC(fed5f203) SHA1(23db14490519b5e2d0bb92ffe6e14540d1999e4b) , ROM_GROUPWORD | ROM_SKIP(6) )    // Dumped as rcm_11.4c
	ROMX_LOAD( "rcm_13.6c",  0x400006, 0x80000, CRC(5069d4a9) SHA1(b832b98be94371af52bd4bb911e18ec57430a7db) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "rcm_14.7c",  0x600000, 0x80000, CRC(303be3bd) SHA1(1e5c3fd71966ea9f457840c40582795b501c323e) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "rcm_15.8c",  0x600002, 0x80000, CRC(4f2d372f) SHA1(db6a94d1f92c1b96e404b38ebcb1eedbec3ae6cc) , ROM_GROUPWORD | ROM_SKIP(6) )    // Dumped as rcm_16.9c
	ROMX_LOAD( "rcm_16.9c",  0x600004, 0x80000, CRC(93d97fde) SHA1(e4be5216f98ad08a9118d629d398be2bd54e2e2a) , ROM_GROUPWORD | ROM_SKIP(6) )    // Dumped as rcm_15.8c
	ROMX_LOAD( "rcm_17.10c", 0x600006, 0x80000, CRC(92371042) SHA1(c55833cbaddcc986edd23c009a3e3c7ff09c2708) , ROM_GROUPWORD | ROM_SKIP(6) )

	ROM_REGION( 0x18000, "audiocpu", 0 ) /* 64k for the audio CPU (+banks) */
	ROM_LOAD( "rcm_09.11a",  0x00000, 0x08000, CRC(22ac8f5f) SHA1(d1441d880f98034645cb4fcecd7bb746bde638af) )   /* different size from megamana, rockmanj but same label, pcb verified */
	ROM_CONTINUE(            0x10000, 0x08000 )

	ROM_REGION( 0x40000, "oki", 0 ) /* Samples */
	ROM_LOAD( "rcm_18.11c",  0x00000, 0x20000, CRC(80f1f8aa) SHA1(4a5b7b2a6941ad68da7472c63362c7bcd353fa54) )
	ROM_LOAD( "rcm_19.12c",  0x20000, 0x20000, CRC(f257dbe1) SHA1(967def6b6f93039dbc46373caabeb3301577be75) )

	ROM_REGION( 0x0200, "aboardplds", 0 )
	ROM_LOAD( "buf1",         0x0000, 0x0117, CRC(eb122de7) SHA1(b26b5bfe258e3e184f069719f9fd008d6b8f6b9b) )
	ROM_LOAD( "ioa1",         0x0000, 0x0117, CRC(59c7ee3b) SHA1(fbb887c5b4f5cb8df77cec710eaac2985bc482a6) )
	ROM_LOAD( "prg1",         0x0000, 0x0117, CRC(f1129744) SHA1(a5300f301c1a08a7da768f0773fa0fe3f683b237) )
	ROM_LOAD( "rom1",         0x0000, 0x0117, CRC(41dc73b9) SHA1(7d4c9f1693c821fbf84e32dd6ef62ddf14967845) )
	ROM_LOAD( "sou1",         0x0000, 0x0117, CRC(84f4b2fe) SHA1(dcc9e86cc36316fe42eace02d6df75d08bc8bb6d) )

	ROM_REGION( 0x0200, "bboardplds", 0 )
	ROM_LOAD( "rcm63b.1a",    0x0000, 0x0117, CRC(84acd494) SHA1(20c861714c8c68bc8cf3bde9d051969807e9b3a3) )
	ROM_LOAD( "iob1.12d",     0x0000, 0x0117, CRC(3abc0700) SHA1(973043aa46ec6d5d1db20dc9d5937005a0f9f6ae) )
	ROM_LOAD( "bprg1.11d",    0x0000, 0x0117, CRC(31793da7) SHA1(400fa7ac517421c978c1ee7773c30b9ed0c5d3f3) )

	ROM_REGION( 0x0200, "cboardplds", 0 )
	ROM_LOAD( "ioc1.ic7",     0x0000, 0x0117, CRC(0d182081) SHA1(475b3d417785da4bc512cce2b274bb00d4cc6792) )
	ROM_LOAD( "c632.ic1",     0x0000, 0x0117, CRC(0fbd9270) SHA1(d7e737b20c44d41e29ca94be56114b31934dde81) )
ROM_END

/* B-Board 91634B-2 */
ROM_START( megamana )
	ROM_REGION( CODE_SIZE, "maincpu", 0 )      /* 68000 code */
	ROM_LOAD16_WORD_SWAP( "rcma_23b.8f", 0x000000, 0x80000, CRC(61e4a397) SHA1(a90b1cbef4206a4554398bc458a4b3e2c46d4c4f) )
	ROM_LOAD16_WORD_SWAP( "rcma_22b.7f", 0x080000, 0x80000, CRC(708268c4) SHA1(554e011cad285b95dd1b6aa19be61b2413662a3a) )  // == rcmu_22b.7f
	ROM_LOAD16_WORD_SWAP( "rcma_21a.6f", 0x100000, 0x80000, CRC(4376ea95) SHA1(7370ceffca513aa9f68a74f6869d561476589200) )  // == rcmu_21a.6f

	ROM_REGION( 0x800000, "gfx", 0 )
	ROMX_LOAD( "rcm_01.3a",  0x000000, 0x80000, CRC(6ecdf13f) SHA1(2a8fe06bf5011e3f990f90d9224f91d8631ec0cc) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "rcm_02.4a",  0x000002, 0x80000, CRC(944d4f0f) SHA1(665dc9a537e9c9b565f6136f939ff5c2861f875f) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "rcm_03.5a",  0x000004, 0x80000, CRC(36f3073c) SHA1(457d68e63599d06a136e152a9ad60adac1c91edd) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "rcm_04.6a",  0x000006, 0x80000, CRC(54e622ff) SHA1(36f6297e3d410f041be5e582919478b0d52520ca) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "rcm_05.7a",  0x200000, 0x80000, CRC(5dd131fd) SHA1(1a7fc8cf38901245d40901996e946e7ad9c0e0c5) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "rcm_06.8a",  0x200002, 0x80000, CRC(f0faf813) SHA1(adff01c2ecc4c8ce6f8a50cbd07d8f8bb9f48168) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "rcm_07.9a",  0x200004, 0x80000, CRC(826de013) SHA1(47f36b1d92a487c43c8dadc8293b8e6f40649286) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "rcm_08.10a", 0x200006, 0x80000, CRC(fbff64cf) SHA1(f0cb531ef195dc1dcd224a208906a62fb5d199a1) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "rcm_10.3c",  0x400000, 0x80000, CRC(4dc8ada9) SHA1(776c2b3ef24c2b8f390c05a9c6728b14ceec696e) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "rcm_11.4c",  0x400002, 0x80000, CRC(f2b9ee06) SHA1(db315b00d1caed1a8c0f6e0ae726e8fa05b011fa) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "rcm_12.5c",  0x400004, 0x80000, CRC(fed5f203) SHA1(23db14490519b5e2d0bb92ffe6e14540d1999e4b) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "rcm_13.6c",  0x400006, 0x80000, CRC(5069d4a9) SHA1(b832b98be94371af52bd4bb911e18ec57430a7db) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "rcm_14.7c",  0x600000, 0x80000, CRC(303be3bd) SHA1(1e5c3fd71966ea9f457840c40582795b501c323e) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "rcm_15.8c",  0x600002, 0x80000, CRC(4f2d372f) SHA1(db6a94d1f92c1b96e404b38ebcb1eedbec3ae6cc) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "rcm_16.9c",  0x600004, 0x80000, CRC(93d97fde) SHA1(e4be5216f98ad08a9118d629d398be2bd54e2e2a) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "rcm_17.10c", 0x600006, 0x80000, CRC(92371042) SHA1(c55833cbaddcc986edd23c009a3e3c7ff09c2708) , ROM_GROUPWORD | ROM_SKIP(6) )

	ROM_REGION( 0x28000, "audiocpu", 0 ) /* 64k for the audio CPU (+banks) */
	ROM_LOAD( "rcm_09.12a",  0x00000, 0x08000, CRC(9632d6ef) SHA1(2bcb6f17005ffbc9ef8fa4478a814f24b2e6e0b6) )
	ROM_CONTINUE(            0x10000, 0x18000 ) // second half of ROM is empty, not mapped in memory

	ROM_REGION( 0x40000, "oki", 0 ) /* Samples */
	ROM_LOAD( "rcm_18.11c",  0x00000, 0x20000, CRC(80f1f8aa) SHA1(4a5b7b2a6941ad68da7472c63362c7bcd353fa54) )
	ROM_LOAD( "rcm_19.12c",  0x20000, 0x20000, CRC(f257dbe1) SHA1(967def6b6f93039dbc46373caabeb3301577be75) )

	ROM_REGION( 0x0200, "aboardplds", 0 )
	ROM_LOAD( "buf1",         0x0000, 0x0117, CRC(eb122de7) SHA1(b26b5bfe258e3e184f069719f9fd008d6b8f6b9b) )
	ROM_LOAD( "ioa1",         0x0000, 0x0117, CRC(59c7ee3b) SHA1(fbb887c5b4f5cb8df77cec710eaac2985bc482a6) )
	ROM_LOAD( "prg1",         0x0000, 0x0117, CRC(f1129744) SHA1(a5300f301c1a08a7da768f0773fa0fe3f683b237) )
	ROM_LOAD( "rom1",         0x0000, 0x0117, CRC(41dc73b9) SHA1(7d4c9f1693c821fbf84e32dd6ef62ddf14967845) )
	ROM_LOAD( "sou1",         0x0000, 0x0117, CRC(84f4b2fe) SHA1(dcc9e86cc36316fe42eace02d6df75d08bc8bb6d) )

	ROM_REGION( 0x0200, "bboardplds", 0 )
	ROM_LOAD( "rcm63b.1a",    0x0000, 0x0117, CRC(84acd494) SHA1(20c861714c8c68bc8cf3bde9d051969807e9b3a3) )
	ROM_LOAD( "iob1.12d",     0x0000, 0x0117, CRC(3abc0700) SHA1(973043aa46ec6d5d1db20dc9d5937005a0f9f6ae) )
	ROM_LOAD( "bprg1.11d",    0x0000, 0x0117, CRC(31793da7) SHA1(400fa7ac517421c978c1ee7773c30b9ed0c5d3f3) )

	ROM_REGION( 0x0200, "cboardplds", 0 )
	ROM_LOAD( "ioc1.ic7",     0x0000, 0x0117, CRC(0d182081) SHA1(475b3d417785da4bc512cce2b274bb00d4cc6792) )
	ROM_LOAD( "c632.ic1",     0x0000, 0x0117, CRC(0fbd9270) SHA1(d7e737b20c44d41e29ca94be56114b31934dde81) )
ROM_END

/* B-Board 91634B-2 */
ROM_START( rockmanj )
	ROM_REGION( CODE_SIZE, "maincpu", 0 )      /* 68000 code */
	ROM_LOAD16_WORD_SWAP( "rcm_23a.8f", 0x000000, 0x80000, CRC(efd96cb2) SHA1(cbe81554f60d0c897f3f2ebc5bc966bb03cc23fe) )
	ROM_LOAD16_WORD_SWAP( "rcm_22a.7f", 0x080000, 0x80000, CRC(8729a689) SHA1(14ddb34d8201c544ea9d3d8c2cc52d380bc72930) )
	ROM_LOAD16_WORD_SWAP( "rcm_21a.6f", 0x100000, 0x80000, CRC(517ccde2) SHA1(492256c192f0c4814efa1ee1dd390453dd2e5865) )

	ROM_REGION( 0x800000, "gfx", 0 )
	ROMX_LOAD( "rcm_01.3a",  0x000000, 0x80000, CRC(6ecdf13f) SHA1(2a8fe06bf5011e3f990f90d9224f91d8631ec0cc) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "rcm_02.4a",  0x000002, 0x80000, CRC(944d4f0f) SHA1(665dc9a537e9c9b565f6136f939ff5c2861f875f) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "rcm_03.5a",  0x000004, 0x80000, CRC(36f3073c) SHA1(457d68e63599d06a136e152a9ad60adac1c91edd) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "rcm_04.6a",  0x000006, 0x80000, CRC(54e622ff) SHA1(36f6297e3d410f041be5e582919478b0d52520ca) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "rcm_05.7a",  0x200000, 0x80000, CRC(5dd131fd) SHA1(1a7fc8cf38901245d40901996e946e7ad9c0e0c5) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "rcm_06.8a",  0x200002, 0x80000, CRC(f0faf813) SHA1(adff01c2ecc4c8ce6f8a50cbd07d8f8bb9f48168) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "rcm_07.9a",  0x200004, 0x80000, CRC(826de013) SHA1(47f36b1d92a487c43c8dadc8293b8e6f40649286) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "rcm_08.10a", 0x200006, 0x80000, CRC(fbff64cf) SHA1(f0cb531ef195dc1dcd224a208906a62fb5d199a1) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "rcm_10.3c",  0x400000, 0x80000, CRC(4dc8ada9) SHA1(776c2b3ef24c2b8f390c05a9c6728b14ceec696e) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "rcm_11.4c",  0x400002, 0x80000, CRC(f2b9ee06) SHA1(db315b00d1caed1a8c0f6e0ae726e8fa05b011fa) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "rcm_12.5c",  0x400004, 0x80000, CRC(fed5f203) SHA1(23db14490519b5e2d0bb92ffe6e14540d1999e4b) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "rcm_13.6c",  0x400006, 0x80000, CRC(5069d4a9) SHA1(b832b98be94371af52bd4bb911e18ec57430a7db) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "rcm_14.7c",  0x600000, 0x80000, CRC(303be3bd) SHA1(1e5c3fd71966ea9f457840c40582795b501c323e) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "rcm_15.8c",  0x600002, 0x80000, CRC(4f2d372f) SHA1(db6a94d1f92c1b96e404b38ebcb1eedbec3ae6cc) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "rcm_16.9c",  0x600004, 0x80000, CRC(93d97fde) SHA1(e4be5216f98ad08a9118d629d398be2bd54e2e2a) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "rcm_17.10c", 0x600006, 0x80000, CRC(92371042) SHA1(c55833cbaddcc986edd23c009a3e3c7ff09c2708) , ROM_GROUPWORD | ROM_SKIP(6) )

	ROM_REGION( 0x28000, "audiocpu", 0 ) /* 64k for the audio CPU (+banks) */
	ROM_LOAD( "rcm_09.12a",  0x00000, 0x08000, CRC(9632d6ef) SHA1(2bcb6f17005ffbc9ef8fa4478a814f24b2e6e0b6) )
	ROM_CONTINUE(            0x10000, 0x18000 ) // second half of ROM is empty, not mapped in memory

	ROM_REGION( 0x40000, "oki", 0 ) /* Samples */
	ROM_LOAD( "rcm_18.11c",  0x00000, 0x20000, CRC(80f1f8aa) SHA1(4a5b7b2a6941ad68da7472c63362c7bcd353fa54) )
	ROM_LOAD( "rcm_19.12c",  0x20000, 0x20000, CRC(f257dbe1) SHA1(967def6b6f93039dbc46373caabeb3301577be75) )

	ROM_REGION( 0x0200, "aboardplds", 0 )
	ROM_LOAD( "buf1",         0x0000, 0x0117, CRC(eb122de7) SHA1(b26b5bfe258e3e184f069719f9fd008d6b8f6b9b) )
	ROM_LOAD( "ioa1",         0x0000, 0x0117, CRC(59c7ee3b) SHA1(fbb887c5b4f5cb8df77cec710eaac2985bc482a6) )
	ROM_LOAD( "prg1",         0x0000, 0x0117, CRC(f1129744) SHA1(a5300f301c1a08a7da768f0773fa0fe3f683b237) )
	ROM_LOAD( "rom1",         0x0000, 0x0117, CRC(41dc73b9) SHA1(7d4c9f1693c821fbf84e32dd6ef62ddf14967845) )
	ROM_LOAD( "sou1",         0x0000, 0x0117, CRC(84f4b2fe) SHA1(dcc9e86cc36316fe42eace02d6df75d08bc8bb6d) )

	ROM_REGION( 0x0200, "bboardplds", 0 )
	ROM_LOAD( "rcm63b.1a",    0x0000, 0x0117, CRC(84acd494) SHA1(20c861714c8c68bc8cf3bde9d051969807e9b3a3) )
	ROM_LOAD( "iob1.12d",     0x0000, 0x0117, CRC(3abc0700) SHA1(973043aa46ec6d5d1db20dc9d5937005a0f9f6ae) )
	ROM_LOAD( "bprg1.11d",    0x0000, 0x0117, CRC(31793da7) SHA1(400fa7ac517421c978c1ee7773c30b9ed0c5d3f3) )

	ROM_REGION( 0x0200, "cboardplds", 0 )
	ROM_LOAD( "ioc1.ic7",     0x0000, 0x0117, CRC(0d182081) SHA1(475b3d417785da4bc512cce2b274bb00d4cc6792) )
	ROM_LOAD( "c632.ic1",     0x0000, 0x0117, CRC(0fbd9270) SHA1(d7e737b20c44d41e29ca94be56114b31934dde81) )
ROM_END

/* B-Board 91634B-2 */
ROM_START( ganbare )
	ROM_REGION( CODE_SIZE, "maincpu", 0 )      /* 68000 code */
	ROM_LOAD16_WORD_SWAP( "mrnj_23d.8f", 0x00000, 0x80000, CRC(f929be72) SHA1(d175bdcace469277479ef85bf4e1b9d5a63cffde) )

	ROM_REGION( 0x400000, "gfx", 0 )
	ROMX_LOAD( "mrnj_01.3a",  0x000000, 0x80000, CRC(3f878020) SHA1(b18faa50d88c76d19db1af73cf4b3095e928f51f) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "mrnj_02.4a",  0x000002, 0x80000, CRC(3e5624d8) SHA1(502e4897916af1c9e121b096de1369d06f1ffe87) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "mrnj_03.5a",  0x000004, 0x80000, CRC(d1e61f96) SHA1(5f6dee8adbf83c697416e440fbdd3a84a6e698da) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "mrnj_04.6a",  0x000006, 0x80000, CRC(d241971b) SHA1(b641740b40a043affbb79ea91ba12f821a259bad) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "mrnj_05.7a",  0x200000, 0x80000, CRC(c0a14562) SHA1(2fb6cf98fed83ac92c33df9526102a101454e276) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "mrnj_06.8a",  0x200002, 0x80000, CRC(e6a71dfc) SHA1(67178b020f87fb28ef35292d008ce9b80e02a2db) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "mrnj_07.9a",  0x200004, 0x80000, CRC(99afb6c7) SHA1(5caead2b71cd54f6b53765f09829cc9e92e1e2d6) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "mrnj_08.10a", 0x200006, 0x80000, CRC(52882c20) SHA1(5e3fca6da3470aeb78534f01e1575d8c0e067c0e) , ROM_GROUPWORD | ROM_SKIP(6) )

	ROM_REGION( 0x18000, "audiocpu", 0 ) /* 64k for the audio CPU (+banks) */
	ROM_LOAD( "mrnj_09.12a",  0x00000, 0x08000, CRC(62470d72) SHA1(1de357a20f794defb49ed01af5b95ad00e2aa1d9) )
	ROM_CONTINUE(             0x10000, 0x08000 )

	ROM_REGION( 0x40000, "oki", 0 ) /* Samples */
	ROM_LOAD( "mrnj_18.11c",  0x00000, 0x20000, CRC(08e13940) SHA1(5c7dd7ff6a66f100b59cf9244e78f2c8702faca1) )
	ROM_LOAD( "mrnj_19.12c",  0x20000, 0x20000, CRC(5fa59927) SHA1(f05246cf566c214b008a91816c71e7c03b7cc218) )

	ROM_REGION( 0x8000, "timekeeper", 0) /* Timekeeper internal RAM was dumped (but game overwrites it - should I keep this here or remove it?) */
	ROM_LOAD( "m48t35y-70pc1.9n", 0x00000, 0x8000, CRC(96107b4a) SHA1(be9149736030e06c96083dcac73b5be3dbc318ac) )

	ROM_REGION( 0x0200, "aboardplds", 0 )
	ROM_LOAD( "buf1",         0x0000, 0x0117, CRC(eb122de7) SHA1(b26b5bfe258e3e184f069719f9fd008d6b8f6b9b) )
	ROM_LOAD( "ioa1",         0x0000, 0x0117, CRC(59c7ee3b) SHA1(fbb887c5b4f5cb8df77cec710eaac2985bc482a6) )
	ROM_LOAD( "prg1",         0x0000, 0x0117, CRC(f1129744) SHA1(a5300f301c1a08a7da768f0773fa0fe3f683b237) )
	ROM_LOAD( "rom1",         0x0000, 0x0117, CRC(41dc73b9) SHA1(7d4c9f1693c821fbf84e32dd6ef62ddf14967845) )
	ROM_LOAD( "sou1",         0x0000, 0x0117, CRC(84f4b2fe) SHA1(dcc9e86cc36316fe42eace02d6df75d08bc8bb6d) )

	ROM_REGION( 0x0200, "bboardplds", 0 )
	ROM_LOAD( "gbpr2.1a",     0x0000, 0x0117, CRC(486e8ca0) SHA1(4554befd49ec322af7dadd198188255ae7c7c059) )
	ROM_LOAD( "iob1.12d",     0x0000, 0x0117, CRC(3abc0700) SHA1(973043aa46ec6d5d1db20dc9d5937005a0f9f6ae) )
	ROM_LOAD( "bprg1.11d",    0x0000, 0x0117, CRC(31793da7) SHA1(400fa7ac517421c978c1ee7773c30b9ed0c5d3f3) )

	ROM_REGION( 0x0200, "cboardplds", 0 )
	ROM_LOAD( "ioc1.ic7",     0x0000, 0x0117, CRC(0d182081) SHA1(475b3d417785da4bc512cce2b274bb00d4cc6792) )
	ROM_LOAD( "c632.ic1",     0x0000, 0x0117, CRC(0fbd9270) SHA1(d7e737b20c44d41e29ca94be56114b31934dde81) )
ROM_END

/* Home 'CPS Changer' Unit - For MESS */

/* B-Board 91635B-2 */
ROM_START( wofch )
	ROM_REGION( CODE_SIZE, "maincpu", 0 )      /* 68000 code */
	ROM_LOAD16_WORD_SWAP( "tk2(ch)_23.8f", 0x000000, 0x80000, CRC(4e0b8dee) SHA1(d2fb716d62b7a259f46bbc74c1976a18d56696ea) )
	ROM_LOAD16_WORD_SWAP( "tk2(ch)_22.7f", 0x080000, 0x80000, CRC(d0937a8d) SHA1(01d7be446e2e3ef8ca767f59c178240dfd52dd93) )

	ROM_REGION( 0x400000, "gfx", 0 )
	ROMX_LOAD( "tk2-1m.3a",      0x000000, 0x80000, CRC(0d9cb9bf) SHA1(cc7140e9a01a14b252cb1090bcea32b0de461928) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "tk2-3m.5a",      0x000002, 0x80000, CRC(45227027) SHA1(b21afc593f0d4d8909dfa621d659cbb40507d1b2) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "tk2-2m.4a",      0x000004, 0x80000, CRC(c5ca2460) SHA1(cbe14867f7b94b638ca80db7c8e0c60881183469) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "tk2-4m.6a",      0x000006, 0x80000, CRC(e349551c) SHA1(1d977bdf256accf750ad9930ec4a0a19bbf86964) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "tk2(ch)_05.7a",  0x200000, 0x80000, CRC(e4a44d53) SHA1(b747679f4d63e5e62d9fd81b3120fba0401fadfb) , ROM_GROUPWORD | ROM_SKIP(6) )    // == tk2_05.7a
	ROMX_LOAD( "tk2(ch)_06.8a",  0x200002, 0x80000, CRC(58066ba8) SHA1(c93af968e21094d020e4b2002e0c6fc0d746af0b) , ROM_GROUPWORD | ROM_SKIP(6) )    // == tk2_06.8a
	ROMX_LOAD( "tk2(ch)_07.9a",  0x200004, 0x80000, CRC(cc9006c9) SHA1(cfcbec3a67052268a7739538aa28a6391fe5400e) , ROM_GROUPWORD | ROM_SKIP(6) )    /* 1 byte different from wofj, pcb verified */
	ROMX_LOAD( "tk2(ch)_08.10a", 0x200006, 0x80000, CRC(d4a19a02) SHA1(ff396b1d33d9b4842140f2c6d085fe05748e3244) , ROM_GROUPWORD | ROM_SKIP(6) )    // == tk2_08.10a

	ROM_REGION( 0x28000, "audiocpu", 0 ) /* QSound Z80 code */
	ROM_LOAD( "tk2_qa.5k",       0x00000, 0x08000, CRC(c9183a0d) SHA1(d8b1d41c572f08581f8ab9eb878de77d6ea8615d) )
	ROM_CONTINUE(                0x10000, 0x18000 )

	ROM_REGION( 0x200000, "qsound", 0 ) /* QSound samples */
	ROM_LOAD( "tk2-q1.1k",       0x000000, 0x80000, CRC(611268cf) SHA1(83ab059f2110fb25fdcff928d56b790fc1f5c975) )
	ROM_LOAD( "tk2-q2.2k",       0x080000, 0x80000, CRC(20f55ca9) SHA1(90134e9a9c4749bb65c728b66ea4dac1fd4d88a4) )
	ROM_LOAD( "tk2-q3.3k",       0x100000, 0x80000, CRC(bfcf6f52) SHA1(2a85ff3fc89b4cbabd20779ec12da2e116333c7c) )
	ROM_LOAD( "tk2-q4.4k",       0x180000, 0x80000, CRC(36642e88) SHA1(8ab25b19e2b67215a5cb1f3aa81b9d26009cfeb8) )

	ROM_REGION( 0x0200, "aboardplds", 0 )
	ROM_LOAD( "buf1",         0x0000, 0x0117, CRC(eb122de7) SHA1(b26b5bfe258e3e184f069719f9fd008d6b8f6b9b) )
	ROM_LOAD( "ioa1",         0x0000, 0x0117, CRC(59c7ee3b) SHA1(fbb887c5b4f5cb8df77cec710eaac2985bc482a6) )
	ROM_LOAD( "prg2",         0x0000, 0x0117, CRC(4386879a) SHA1(c36896d169d8c78393609acbbe4397931292a033) )
	ROM_LOAD( "rom1",         0x0000, 0x0117, CRC(41dc73b9) SHA1(7d4c9f1693c821fbf84e32dd6ef62ddf14967845) )
	ROM_LOAD( "sou1",         0x0000, 0x0117, CRC(84f4b2fe) SHA1(dcc9e86cc36316fe42eace02d6df75d08bc8bb6d) )

	ROM_REGION( 0x0200, "bboardplds", 0 )
	ROM_LOAD( "tk263b.1a",    0x0000, 0x0117, CRC(c4b0349b) SHA1(b4873dd5ad8735048deb3475222dde3c0b67eaaf) )
	ROM_LOAD( "iob1.12d",     0x0000, 0x0117, CRC(3abc0700) SHA1(973043aa46ec6d5d1db20dc9d5937005a0f9f6ae) )
	ROM_LOAD( "bprg1.11d",    0x0000, 0x0117, CRC(31793da7) SHA1(400fa7ac517421c978c1ee7773c30b9ed0c5d3f3) )

	ROM_REGION( 0x0200, "cboardplds", 0 )
	ROM_LOAD( "ioc1.ic7",     0x0000, 0x0117, CRC(0d182081) SHA1(475b3d417785da4bc512cce2b274bb00d4cc6792) )
	ROM_LOAD( "c632.ic1",     0x0000, 0x0117, CRC(0fbd9270) SHA1(d7e737b20c44d41e29ca94be56114b31934dde81) )

	ROM_REGION( 0x0200, "dboardplds", 0 )
	ROM_LOAD( "d7l1.7l",      0x0000, 0x0117, CRC(27b7410d) SHA1(06d0cba0226850f100ff1f539bd7d5db0f90c730) )
	ROM_LOAD( "d8l1.8l",      0x0000, 0x0117, CRC(539fc7da) SHA1(cad5c91629c6247e49ccbbcbfe6b08229eafae07) )
	ROM_LOAD( "d9k1.9k",      0x0000, 0x0117, NO_DUMP )
	ROM_LOAD( "d10f1.10f",    0x0000, 0x0117, CRC(6619c494) SHA1(3aef656c07182a2186f810f30e0d854dd5bd8d18) )
ROM_END

ROM_START( sfach )
	ROM_REGION( CODE_SIZE, "maincpu", 0 )      /* 68000 code */
	ROM_LOAD16_WORD_SWAP( "sfach23",        0x000000, 0x80000, CRC(02a1a853) SHA1(d92b9e774844fdcc9d9946b3e892b021e672d876) )
	ROM_LOAD16_WORD_SWAP( "sfza22",         0x080000, 0x80000, CRC(8d9b2480) SHA1(405305c1572908d00eab735f28676fbbadb4fac6) )
	ROM_LOAD16_WORD_SWAP( "sfzch21",        0x100000, 0x80000, CRC(5435225d) SHA1(6b1156fd82d0710e244ede39faaae0847c598376) )
	ROM_LOAD16_WORD_SWAP( "sfza20",         0x180000, 0x80000, CRC(806e8f38) SHA1(b6d6912aa8f2f590335d7ff9a8214648e7131ebb) )

	ROM_REGION( 0x800000, "gfx", 0 )
	ROMX_LOAD( "sfz01",         0x000000, 0x80000, CRC(0dd53e62) SHA1(5f3bcf5ca0fd564d115fe5075a4163d3ee3226df) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "sfz02",         0x000002, 0x80000, CRC(94c31e3f) SHA1(2187b3d4977514f2ae486eb33ed76c86121d5745) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "sfz03",         0x000004, 0x80000, CRC(9584ac85) SHA1(bbd62d66b0f6909630e801ce5d6331d43f44d741) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "sfz04",         0x000006, 0x80000, CRC(b983624c) SHA1(841106bb9453e3dfb7869c4b0e9149cc610d515a) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "sfz05",         0x200000, 0x80000, CRC(2b47b645) SHA1(bc6426eff5df9417f32666586744626fa544f7b5) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "sfz06",         0x200002, 0x80000, CRC(74fd9fb1) SHA1(7945472591f3c06970e96611a0363ed8f3d52c36) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "sfz07",         0x200004, 0x80000, CRC(bb2c734d) SHA1(97a06935f86f31755d2ffdc5b56bef53944bdecd) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "sfz08",         0x200006, 0x80000, CRC(454f7868) SHA1(eecccba7542d893bc41676246a20aa4914b79bbc) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "sfz10",         0x400000, 0x80000, CRC(2a7d675e) SHA1(0144ba34a29fb08b41c780ce65bb06d25724e88f) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "sfz11",         0x400002, 0x80000, CRC(e35546c8) SHA1(7b08aa3413494d12c5c550263a5f00b64b98e6ab) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "sfz12",         0x400004, 0x80000, CRC(f122693a) SHA1(71ce901d8d30207e506b6a8d6a4e0fcf3a1b0eac) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "sfz13",         0x400006, 0x80000, CRC(7cf942c8) SHA1(a7109facb97a8a11ddf1b4e07de6ff3164d713a1) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "sfz14",         0x600000, 0x80000, CRC(09038c81) SHA1(3461d70902fbfb92ce40f804be6388276a01d153) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "sfz15",         0x600002, 0x80000, CRC(1aa17391) SHA1(b4d0f760a430b7fc4443b6c94da2659315c5b926) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "sfz16",         0x600004, 0x80000, CRC(19a5abd6) SHA1(73ba1de15c883fdc69fd7dccdb58d00ca512d4ea) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "sfz17",         0x600006, 0x80000, CRC(248b3b73) SHA1(95810a17b1caf6372b33ed3e4ee8a7e51482c70d) , ROM_GROUPWORD | ROM_SKIP(6) )

	ROM_REGION( 0x8000, "stars", 0 )
	ROM_COPY( "gfx", 0x000000, 0x000000, 0x8000 )   /* stars */

	ROM_REGION( 0x18000, "audiocpu", 0 ) /* 64k for the audio CPU (+banks) */
	ROM_LOAD( "sfz09",         0x00000, 0x08000, CRC(c772628b) SHA1(ebc5b7c173caf1e151f733f23c1b20abec24e16d) )
	ROM_CONTINUE(              0x10000, 0x08000 )

	ROM_REGION( 0x40000, "oki", 0 ) /* Samples */
	ROM_LOAD( "sfz18",         0x00000, 0x20000, CRC(61022b2d) SHA1(6369d0c1d08a30ee19b94e52ab1463a7784b9de5) )
	ROM_LOAD( "sfz19",         0x20000, 0x20000, CRC(3b5886d5) SHA1(7e1b7d40ef77b5df628dd663d45a9a13c742cf58) )
ROM_END

/* FIXME B-Board uncertain but should be 91634B from the program ROM names */
ROM_START( sfzch )
	ROM_REGION( CODE_SIZE, "maincpu",0 )      /* 68000 code */
	ROM_LOAD16_WORD_SWAP( "sfzch23",        0x000000, 0x80000, CRC(1140743f) SHA1(10bcedb5cca266f2aa3ed99ede6f9a64fc877539))
	ROM_LOAD16_WORD_SWAP( "sfza22",         0x080000, 0x80000, CRC(8d9b2480) SHA1(405305c1572908d00eab735f28676fbbadb4fac6))
	ROM_LOAD16_WORD_SWAP( "sfzch21",        0x100000, 0x80000, CRC(5435225d) SHA1(6b1156fd82d0710e244ede39faaae0847c598376))
	ROM_LOAD16_WORD_SWAP( "sfza20",         0x180000, 0x80000, CRC(806e8f38) SHA1(b6d6912aa8f2f590335d7ff9a8214648e7131ebb))

	ROM_REGION( 0x800000, "gfx", 0 )
	ROMX_LOAD( "sfz_01.3a",  0x000000, 0x80000, CRC(0dd53e62) SHA1(5f3bcf5ca0fd564d115fe5075a4163d3ee3226df), ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "sfz_02.4a",  0x000002, 0x80000, CRC(94c31e3f) SHA1(2187b3d4977514f2ae486eb33ed76c86121d5745), ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "sfz_03.5a",  0x000004, 0x80000, CRC(9584ac85) SHA1(bbd62d66b0f6909630e801ce5d6331d43f44d741), ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "sfz_04.6a",  0x000006, 0x80000, CRC(b983624c) SHA1(841106bb9453e3dfb7869c4b0e9149cc610d515a), ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "sfz_05.7a",  0x200000, 0x80000, CRC(2b47b645) SHA1(bc6426eff5df9417f32666586744626fa544f7b5), ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "sfz_06.8a",  0x200002, 0x80000, CRC(74fd9fb1) SHA1(7945472591f3c06970e96611a0363ed8f3d52c36), ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "sfz_07.9a",  0x200004, 0x80000, CRC(bb2c734d) SHA1(97a06935f86f31755d2ffdc5b56bef53944bdecd), ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "sfz_08.10a", 0x200006, 0x80000, CRC(454f7868) SHA1(eecccba7542d893bc41676246a20aa4914b79bbc), ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "sfz_10.3c",  0x400000, 0x80000, CRC(2a7d675e) SHA1(0144ba34a29fb08b41c780ce65bb06d25724e88f), ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "sfz_11.4c",  0x400002, 0x80000, CRC(e35546c8) SHA1(7b08aa3413494d12c5c550263a5f00b64b98e6ab), ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "sfz_12.5c",  0x400004, 0x80000, CRC(f122693a) SHA1(71ce901d8d30207e506b6a8d6a4e0fcf3a1b0eac), ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "sfz_13.6c",  0x400006, 0x80000, CRC(7cf942c8) SHA1(a7109facb97a8a11ddf1b4e07de6ff3164d713a1), ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "sfz_14.7c",  0x600000, 0x80000, CRC(09038c81) SHA1(3461d70902fbfb92ce40f804be6388276a01d153), ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "sfz_15.8c",  0x600002, 0x80000, CRC(1aa17391) SHA1(b4d0f760a430b7fc4443b6c94da2659315c5b926), ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "sfz_16.9c",  0x600004, 0x80000, CRC(19a5abd6) SHA1(73ba1de15c883fdc69fd7dccdb58d00ca512d4ea), ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "sfz_17.10c", 0x600006, 0x80000, CRC(248b3b73) SHA1(95810a17b1caf6372b33ed3e4ee8a7e51482c70d), ROM_GROUPWORD | ROM_SKIP(6) )

	ROM_REGION( 0x8000, "stars", 0 )
	ROM_COPY( "gfx", 0x000000, 0x000000, 0x8000 )

	ROM_REGION( 0x18000, "audiocpu",0 ) /* 64k for the audio CPU (+banks) */
	ROM_LOAD( "sfz_09.12a",  0x00000, 0x08000, CRC(c772628b) SHA1(ebc5b7c173caf1e151f733f23c1b20abec24e16d))
	ROM_CONTINUE(            0x10000, 0x08000 )

	ROM_REGION( 0x40000, "oki",0 )  /* Samples */
	ROM_LOAD( "sfz_18.11c",  0x00000, 0x20000, CRC(61022b2d) SHA1(6369d0c1d08a30ee19b94e52ab1463a7784b9de5))
	ROM_LOAD( "sfz_19.12c",  0x20000, 0x20000, CRC(3b5886d5) SHA1(7e1b7d40ef77b5df628dd663d45a9a13c742cf58))
ROM_END

/* FIXME B-Board uncertain but should be 91634B from the program ROM names */
ROM_START( sfzbch )
	ROM_REGION( CODE_SIZE, "maincpu", 0 )      /* 68000 code */
	ROM_LOAD16_WORD_SWAP( "sfzbch23",       0x000000, 0x80000, CRC(53699f68) SHA1(d7f132faf8c31b5e79c32e6b0cce45377ec8d474) )
	ROM_LOAD16_WORD_SWAP( "sfza22",         0x080000, 0x80000, CRC(8d9b2480) SHA1(405305c1572908d00eab735f28676fbbadb4fac6) )
	ROM_LOAD16_WORD_SWAP( "sfzch21",        0x100000, 0x80000, CRC(5435225d) SHA1(6b1156fd82d0710e244ede39faaae0847c598376) )
	ROM_LOAD16_WORD_SWAP( "sfza20",         0x180000, 0x80000, CRC(806e8f38) SHA1(b6d6912aa8f2f590335d7ff9a8214648e7131ebb) )

	ROM_REGION( 0x800000, "gfx", 0 )
	ROMX_LOAD( "sfz_01.3a",  0x000000, 0x80000, CRC(0dd53e62) SHA1(5f3bcf5ca0fd564d115fe5075a4163d3ee3226df), ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "sfz_02.4a",  0x000002, 0x80000, CRC(94c31e3f) SHA1(2187b3d4977514f2ae486eb33ed76c86121d5745), ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "sfz_03.5a",  0x000004, 0x80000, CRC(9584ac85) SHA1(bbd62d66b0f6909630e801ce5d6331d43f44d741), ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "sfz_04.6a",  0x000006, 0x80000, CRC(b983624c) SHA1(841106bb9453e3dfb7869c4b0e9149cc610d515a), ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "sfz_05.7a",  0x200000, 0x80000, CRC(2b47b645) SHA1(bc6426eff5df9417f32666586744626fa544f7b5), ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "sfz_06.8a",  0x200002, 0x80000, CRC(74fd9fb1) SHA1(7945472591f3c06970e96611a0363ed8f3d52c36), ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "sfz_07.9a",  0x200004, 0x80000, CRC(bb2c734d) SHA1(97a06935f86f31755d2ffdc5b56bef53944bdecd), ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "sfz_08.10a", 0x200006, 0x80000, CRC(454f7868) SHA1(eecccba7542d893bc41676246a20aa4914b79bbc), ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "sfz_10.3c",  0x400000, 0x80000, CRC(2a7d675e) SHA1(0144ba34a29fb08b41c780ce65bb06d25724e88f), ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "sfz_11.4c",  0x400002, 0x80000, CRC(e35546c8) SHA1(7b08aa3413494d12c5c550263a5f00b64b98e6ab), ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "sfz_12.5c",  0x400004, 0x80000, CRC(f122693a) SHA1(71ce901d8d30207e506b6a8d6a4e0fcf3a1b0eac), ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "sfz_13.6c",  0x400006, 0x80000, CRC(7cf942c8) SHA1(a7109facb97a8a11ddf1b4e07de6ff3164d713a1), ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "sfz_14.7c",  0x600000, 0x80000, CRC(09038c81) SHA1(3461d70902fbfb92ce40f804be6388276a01d153), ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "sfz_15.8c",  0x600002, 0x80000, CRC(1aa17391) SHA1(b4d0f760a430b7fc4443b6c94da2659315c5b926), ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "sfz_16.9c",  0x600004, 0x80000, CRC(19a5abd6) SHA1(73ba1de15c883fdc69fd7dccdb58d00ca512d4ea), ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "sfz_17.10c", 0x600006, 0x80000, CRC(248b3b73) SHA1(95810a17b1caf6372b33ed3e4ee8a7e51482c70d), ROM_GROUPWORD | ROM_SKIP(6) )

	ROM_REGION( 0x8000, "stars", 0 )
	ROM_COPY( "gfx", 0x000000, 0x000000, 0x8000 )   /* stars */

	ROM_REGION( 0x18000, "audiocpu",0 ) /* 64k for the audio CPU (+banks) */
	ROM_LOAD( "sfz_09.12a",  0x00000, 0x08000, CRC(c772628b) SHA1(ebc5b7c173caf1e151f733f23c1b20abec24e16d))
	ROM_CONTINUE(            0x10000, 0x08000 )

	ROM_REGION( 0x40000, "oki",0 )  /* Samples */
	ROM_LOAD( "sfz_18.11c",  0x00000, 0x20000, CRC(61022b2d) SHA1(6369d0c1d08a30ee19b94e52ab1463a7784b9de5))
	ROM_LOAD( "sfz_19.12c",  0x20000, 0x20000, CRC(3b5886d5) SHA1(7e1b7d40ef77b5df628dd663d45a9a13c742cf58))
ROM_END


DRIVER_INIT_MEMBER(cps_state,forgottn)
{
	/* Forgotten Worlds has a NEC uPD4701AC on the B-board handling dial inputs from the CN-MOWS connector. */
	/* The memory mapping is handled by PAL LWIO */
	m_maincpu->space(AS_PROGRAM).install_write_handler(0x800040, 0x800041, write16_delegate(FUNC(cps_state::forgottn_dial_0_reset_w),this));
	m_maincpu->space(AS_PROGRAM).install_write_handler(0x800048, 0x800049, write16_delegate(FUNC(cps_state::forgottn_dial_1_reset_w),this));
	m_maincpu->space(AS_PROGRAM).install_read_handler(0x800052, 0x800055, read16_delegate(FUNC(cps_state::forgottn_dial_0_r),this));
	m_maincpu->space(AS_PROGRAM).install_read_handler(0x80005a, 0x80005d, read16_delegate(FUNC(cps_state::forgottn_dial_1_r),this));

	save_item(NAME(m_dial));

	m_dial[0] = 0;
	m_dial[1] = 0;

	DRIVER_INIT_CALL(cps1);
}

READ16_MEMBER(cps_state::sf2rb_prot_r)
{
	switch (offset)
	{
		case 0x01201/2:
			return 0x0002;

		case 0x81201/2:
			return 0x0040;
	}

	return 0;
}

DRIVER_INIT_MEMBER(cps_state,sf2rb)
{
	m_maincpu->space(AS_PROGRAM).install_read_handler(0x200000, 0x2fffff, read16_delegate(FUNC(cps_state::sf2rb_prot_r),this));

	DRIVER_INIT_CALL(cps1);
}

READ16_MEMBER(cps_state::sf2rb2_prot_r)
{
	switch (offset)
	{
		case 0x01201/2:
			return 0x0000;

		case 0x81201/2:
			return 0x0040;
	}

	return 0;
}

DRIVER_INIT_MEMBER(cps_state,sf2rb2)
{
	m_maincpu->space(AS_PROGRAM).install_read_handler(0x200000, 0x2fffff, read16_delegate(FUNC(cps_state::sf2rb2_prot_r),this));

	DRIVER_INIT_CALL(cps1);
}

DRIVER_INIT_MEMBER(cps_state,sf2ee)
{
	/* This specific revision of SF2 has the CPS-B custom mapped at a different address. */
	/* The mapping is handled by the PAL IOB2 on the B-board */
	m_maincpu->space(AS_PROGRAM).unmap_readwrite(0x800140, 0x80017f);
	m_maincpu->space(AS_PROGRAM).install_readwrite_handler(0x8001c0, 0x8001ff, read16_delegate(FUNC(cps_state::cps1_cps_b_r),this), write16_delegate(FUNC(cps_state::cps1_cps_b_w),this));

	DRIVER_INIT_CALL(cps1);
}

DRIVER_INIT_MEMBER(cps_state,sf2thndr)
{
	/* This particular hack uses a modified B-board PAL which mirrors the CPS-B registers at an alternate address */
	m_maincpu->space(AS_PROGRAM).install_readwrite_handler(0x8001c0, 0x8001ff, read16_delegate(FUNC(cps_state::cps1_cps_b_r),this), write16_delegate(FUNC(cps_state::cps1_cps_b_w),this));

	DRIVER_INIT_CALL(cps1);
}

DRIVER_INIT_MEMBER(cps_state,sf2hack)
{
	/* some SF2 hacks have some inputs wired to the LSB instead of MSB */
	m_maincpu->space(AS_PROGRAM).install_read_handler(0x800018, 0x80001f, read16_delegate(FUNC(cps_state::cps1_hack_dsw_r),this));

	DRIVER_INIT_CALL(cps1);
}


READ16_MEMBER(cps_state::sf2dongb_prot_r)
{
	switch (offset)
	{
		case 0x00000/2: // (ret & 0x0f00) == 0x0200
		case 0x77040/2: // (ret & 0x0ff0) == 0x0210
			return 0x0210;
	}

	return 0;
}

DRIVER_INIT_MEMBER(cps_state,sf2dongb)
{
	// There is a hacked up Altera EP910PC-30 DIP in the 5f socket instead of a 4th eprom
	m_maincpu->space(AS_PROGRAM).install_read_handler(0x180000, 0x1fffff, read16_delegate(FUNC(cps_state::sf2dongb_prot_r),this));

	DRIVER_INIT_CALL(cps1);
}

READ16_MEMBER(cps_state::sf2ceblp_prot_r)
{
	if (sf2ceblp_prot == 0x0)
		return 0x1992;
	if (sf2ceblp_prot == 0x04)
		return 0x0408;
	return 0xffff;
}

WRITE16_MEMBER(cps_state::sf2ceblp_prot_w)
{
	sf2ceblp_prot = data;
}


DRIVER_INIT_MEMBER( cps_state, sf2ceblp )
{
	m_maincpu->space(AS_PROGRAM).install_write_handler(0x5762b0, 0x5762b1, write16_delegate(FUNC(cps_state::sf2ceblp_prot_w),this));
	m_maincpu->space(AS_PROGRAM).install_read_handler(0x57A2b0, 0x57A2b1, read16_delegate(FUNC(cps_state::sf2ceblp_prot_r),this));

	DRIVER_INIT_CALL(cps1);
}

DRIVER_INIT_MEMBER( cps_state, sf2m8 )
{
	// unscramble gfx
	UINT8 *grom = memregion("gfx")->base();
	UINT8 *urom = memregion("user2")->base();
	int i = 0x480000, j = 0;

	for (j = 0x20000; j < 0x80000; j+=2)
	{
		grom[i++] = urom[j];
		grom[i++] = urom[j|0x100000];
		grom[i++] = urom[j|0x000001];
		grom[i++] = urom[j|0x100001];
		grom[i++] = urom[j|0x080000];
		grom[i++] = urom[j|0x180000];
		grom[i++] = urom[j|0x080001];
		grom[i++] = urom[j|0x180001];
	}

	DRIVER_INIT_CALL(cps1);
}

void cps_state::kabuki_setup(void (*decode)(UINT8 *src, UINT8 *dst))
{
	m_decrypt_kabuki = std::make_unique<UINT8[]>(0x8000);
	UINT8 *rom = memregion("audiocpu")->base();
	decode(rom, m_decrypt_kabuki.get());
	membank("decrypted")->set_base(m_decrypt_kabuki.get());
}

DRIVER_INIT_MEMBER(cps_state,wof)
{
	kabuki_setup(wof_decode);
	DRIVER_INIT_CALL(cps1);
}

DRIVER_INIT_MEMBER(cps_state,dino)
{
	kabuki_setup(dino_decode);
	DRIVER_INIT_CALL(cps1);
}

DRIVER_INIT_MEMBER(cps_state,punisher)
{
	kabuki_setup(punisher_decode);
	DRIVER_INIT_CALL(cps1);
}

DRIVER_INIT_MEMBER(cps_state,slammast)
{
	kabuki_setup(slammast_decode);
	DRIVER_INIT_CALL(cps1);
}

DRIVER_INIT_MEMBER(cps_state,pang3b)
{
	/* Pang 3 is the only non-QSound game to have an EEPROM. */
	/* It is mapped in the CPS-B address range so probably is on the C-board. */
	m_maincpu->space(AS_PROGRAM).install_readwrite_port(0x80017a, 0x80017b, "EEPROMIN", "EEPROMOUT");

	DRIVER_INIT_CALL(cps1);
}

DRIVER_INIT_MEMBER(cps_state,pang3)
{
	UINT16 *rom = (UINT16 *)memregion("maincpu")->base();
	int A, src, dst;

	for (A = 0x80000; A < 0x100000; A += 2)
	{
		/* only the low 8 bits of each word are encrypted */
		src = rom[A / 2];
		dst = src & 0xff00;
		if ( src & 0x01) dst ^= 0x04;
		if ( src & 0x02) dst ^= 0x21;
		if ( src & 0x04) dst ^= 0x01;
		if (~src & 0x08) dst ^= 0x50;
		if ( src & 0x10) dst ^= 0x40;
		if ( src & 0x20) dst ^= 0x06;
		if ( src & 0x40) dst ^= 0x08;
		if (~src & 0x80) dst ^= 0x88;
		rom[A/2] = dst;
	}

	DRIVER_INIT_CALL(pang3b);
}

READ16_MEMBER(cps_state::ganbare_ram_r)
{
	UINT16 result = 0xffff;

	if (ACCESSING_BITS_0_7)
		result = (result & ~0x00ff) | m_m48t35->read(space, offset, 0xff);
	if (ACCESSING_BITS_8_15)
		result = (result & ~0xff00) | (m_mainram[offset] & 0xff00);

	return result;
}

WRITE16_MEMBER(cps_state::ganbare_ram_w)
{
	COMBINE_DATA(&m_mainram[offset]);

	if (ACCESSING_BITS_0_7)
		m_m48t35->write(space, offset, data & 0xff, 0xff);
}

DRIVER_INIT_MEMBER(cps_state, ganbare)
{
	DRIVER_INIT_CALL(cps1);

	/* ram is shared between the CPS work ram and the timekeeper ram */
	m_maincpu->space(AS_PROGRAM).install_readwrite_handler(0xff0000, 0xffffff, read16_delegate(FUNC(cps_state::ganbare_ram_r),this), write16_delegate(FUNC(cps_state::ganbare_ram_w),this));
}

READ16_MEMBER(cps_state::dinohunt_sound_r)
{
	/*TODO: understand what's really going on here. According to MT05805;
	"I think that the values written are only qsound leftovers (after a lot of 0xFF values,
	there is the same qsound starting sequence, eg: 0x88, 0xFF, 0x0B, 0x00, 0x00, 0x00, ...)."*/
	return 0xff;
}

DRIVER_INIT_MEMBER(cps_state,dinohunt)
{
	// is this shared with the new sound hw?
	m_maincpu->space(AS_PROGRAM).install_read_handler(0xf18000, 0xf19fff, read16_delegate(FUNC(cps_state::dinohunt_sound_r), this));
	m_maincpu->space(AS_PROGRAM).install_read_port(0xfc0000, 0xfc0001, "IN2"); ;
	// the ym2151 doesn't seem to be used. Is it actually on the PCB?

	DRIVER_INIT_CALL(cps1);
}

WRITE16_MEMBER( cps_state::sf2m3_layer_w )
{
	cps1_cps_b_w(space,0x0a,data);
}



/*************************************************** Game Macros *****************************************************/

GAME( 1988, forgottn,    0,        cps1_10MHz, forgottn, cps_state,   forgottn, ROT0,   "Capcom", "Forgotten Worlds (World, newer)", MACHINE_SUPPORTS_SAVE )  // (c) Capcom U.S.A. but World "warning"
GAME( 1988, forgottna,   forgottn, cps1_10MHz, forgottn, cps_state,   forgottn, ROT0,   "Capcom", "Forgotten Worlds (World)", MACHINE_SUPPORTS_SAVE )  // (c) Capcom U.S.A. but World "warning"
GAME( 1988, forgottnu,   forgottn, cps1_10MHz, forgottn, cps_state,   forgottn, ROT0,   "Capcom", "Forgotten Worlds (USA, B-Board 88621B-2, Rev. C)", MACHINE_SUPPORTS_SAVE )
GAME( 1988, forgottnu1,  forgottn, cps1_10MHz, forgottn, cps_state,   forgottn, ROT0,   "Capcom", "Forgotten Worlds (USA, B-Board 88618B-2, Rev. C)", MACHINE_SUPPORTS_SAVE )
GAME( 1988, forgottnua,  forgottn, cps1_10MHz, forgottn, cps_state,   forgottn, ROT0,   "Capcom", "Forgotten Worlds (USA, B-Board 88618B-2, Rev. A)", MACHINE_SUPPORTS_SAVE )
GAME( 1988, forgottnuaa, forgottn, cps1_10MHz, forgottn, cps_state,   forgottn, ROT0,   "Capcom", "Forgotten Worlds (USA, B-Board 88618B-2, Rev. AA)", MACHINE_SUPPORTS_SAVE )
GAME( 1988, lostwrld,    forgottn, cps1_10MHz, forgottn, cps_state,   forgottn, ROT0,   "Capcom", "Lost Worlds (Japan)", MACHINE_SUPPORTS_SAVE )
GAME( 1988, lostwrldo,   forgottn, cps1_10MHz, forgottn, cps_state,   forgottn, ROT0,   "Capcom", "Lost Worlds (Japan Old Ver.)", MACHINE_SUPPORTS_SAVE )
GAME( 1988, ghouls,      0,        cps1_10MHz, ghouls,   cps_state,   cps1,     ROT0,   "Capcom", "Ghouls'n Ghosts (World)", MACHINE_SUPPORTS_SAVE )   // "EXPORT" // Wed.26.10.1988 in the ROMs
GAME( 1988, ghoulsu,     ghouls,   cps1_10MHz, ghoulsu,  cps_state,   cps1,     ROT0,   "Capcom", "Ghouls'n Ghosts (USA)", MACHINE_SUPPORTS_SAVE ) // "EXPORT" // Wed.26.10.1988 in the ROMs
GAME( 1988, daimakai,    ghouls,   cps1_10MHz, daimakai, cps_state,   cps1,     ROT0,   "Capcom", "Daimakaimura (Japan)", MACHINE_SUPPORTS_SAVE )              // Wed.26.10.1988 in the ROMs
GAME( 1988, daimakair,   ghouls,   cps1_12MHz, daimakai, cps_state,   cps1,     ROT0,   "Capcom", "Daimakaimura (Japan Resale Ver.)", MACHINE_SUPPORTS_SAVE )      // Wed.26.10.1988 in the ROMs   // 12MHz verified
GAME( 1989, strider,     0,        cps1_10MHz, strider,  cps_state,   cps1,     ROT0,   "Capcom", "Strider (USA, B-Board 89624B-2)", MACHINE_SUPPORTS_SAVE )
GAME( 1989, striderua,   strider,  cps1_10MHz, stridrua, cps_state,   cps1,     ROT0,   "Capcom", "Strider (USA, B-Board 89624B-3)", MACHINE_SUPPORTS_SAVE )
GAME( 1989, striderj,    strider,  cps1_10MHz, strider,  cps_state,   cps1,     ROT0,   "Capcom", "Strider Hiryu (Japan)", MACHINE_SUPPORTS_SAVE )
GAME( 1989, striderjr,   strider,  cps1_12MHz, strider,  cps_state,   cps1,     ROT0,   "Capcom", "Strider Hiryu (Japan Resale Ver.)", MACHINE_SUPPORTS_SAVE ) // 12MHz verified
GAME( 1989, dynwar,      0,        cps1_10MHz, dynwar,   cps_state,   cps1,     ROT0,   "Capcom", "Dynasty Wars (USA, B-Board 89624B-?)", MACHINE_SUPPORTS_SAVE )  // (c) Capcom U.S.A.
GAME( 1989, dynwara,     dynwar,   cps1_10MHz, dynwar,   cps_state,   cps1,     ROT0,   "Capcom", "Dynasty Wars (USA, B-Board 88622B-3)", MACHINE_SUPPORTS_SAVE )  // (c) Capcom U.S.A.
GAME( 1989, dynwarj,     dynwar,   cps1_10MHz, dynwar,   cps_state,   cps1,     ROT0,   "Capcom", "Tenchi wo Kurau (Japan)", MACHINE_SUPPORTS_SAVE )
GAME( 1989, dynwarjr,    dynwar,   cps1_12MHz, dynwar,   cps_state,   cps1,     ROT0,   "Capcom", "Tenchi wo Kurau (Japan Resale Ver.)", MACHINE_SUPPORTS_SAVE )   // 12MHz verified
GAME( 1989, willow,      0,        cps1_10MHz, willow,   cps_state,   cps1,     ROT0,   "Capcom", "Willow (World)", MACHINE_SUPPORTS_SAVE ) // No "Warning" (c) Capcom U.S.A., genuine export ROM labels
GAME( 1989, willowu,     willow,   cps1_10MHz, willow,   cps_state,   cps1,     ROT0,   "Capcom", "Willow (USA)", MACHINE_SUPPORTS_SAVE )
GAME( 1989, willowuo,    willow,   cps1_10MHz, willow,   cps_state,   cps1,     ROT0,   "Capcom", "Willow (USA Old Ver.)", MACHINE_SUPPORTS_SAVE ) // Japan "warning" but (c) Capcom U.S.A.
GAME( 1989, willowj,     willow,   cps1_10MHz, willow,   cps_state,   cps1,     ROT0,   "Capcom", "Willow (Japan)", MACHINE_SUPPORTS_SAVE )
GAME( 1989, unsquad,     0,        cps1_10MHz, unsquad,  cps_state,   cps1,     ROT0,   "Capcom / Daipro", "U.N. Squadron (USA)", MACHINE_SUPPORTS_SAVE )
GAME( 1989, area88,      unsquad,  cps1_10MHz, unsquad,  cps_state,   cps1,     ROT0,   "Capcom / Daipro", "Area 88 (Japan)", MACHINE_SUPPORTS_SAVE )
GAME( 1989, area88r,     unsquad,  cps1_12MHz, unsquad,  cps_state,   cps1,     ROT0,   "Capcom / Daipro", "Area 88 (Japan Resale Ver.)", MACHINE_SUPPORTS_SAVE )  // 12MHz verified
GAME( 1989, ffight,      0,        cps1_10MHz, ffight,   cps_state,   cps1,     ROT0,   "Capcom", "Final Fight (World, set 1)", MACHINE_SUPPORTS_SAVE )
GAME( 1989, ffighta,     ffight,   cps1_10MHz, ffight,   cps_state,   cps1,     ROT0,   "Capcom", "Final Fight (World, set 2)", MACHINE_SUPPORTS_SAVE )
GAME( 1989, ffightu,     ffight,   cps1_10MHz, ffight,   cps_state,   cps1,     ROT0,   "Capcom", "Final Fight (USA, set 1)", MACHINE_SUPPORTS_SAVE )
GAME( 1989, ffightu1,    ffight,   cps1_10MHz, ffight,   cps_state,   cps1,     ROT0,   "Capcom", "Final Fight (USA, set 2)", MACHINE_SUPPORTS_SAVE )
GAME( 1989, ffightua,    ffight,   cps1_10MHz, ffight,   cps_state,   cps1,     ROT0,   "Capcom", "Final Fight (USA 900112)", MACHINE_SUPPORTS_SAVE )
GAME( 1989, ffightub,    ffight,   cps1_10MHz, ffight,   cps_state,   cps1,     ROT0,   "Capcom", "Final Fight (USA 900424)", MACHINE_SUPPORTS_SAVE )
GAME( 1989, ffightuc,    ffight,   cps1_10MHz, ffight,   cps_state,   cps1,     ROT0,   "Capcom", "Final Fight (USA 900613)", MACHINE_SUPPORTS_SAVE )
GAME( 1989, ffightj,     ffight,   cps1_10MHz, ffight,   cps_state,   cps1,     ROT0,   "Capcom", "Final Fight (Japan)", MACHINE_SUPPORTS_SAVE )
GAME( 1989, ffightj1,    ffight,   cps1_10MHz, ffight,   cps_state,   cps1,     ROT0,   "Capcom", "Final Fight (Japan 900112)", MACHINE_SUPPORTS_SAVE )
GAME( 1989, ffightj2,    ffight,   cps1_10MHz, ffight,   cps_state,   cps1,     ROT0,   "Capcom", "Final Fight (Japan 900305)", MACHINE_SUPPORTS_SAVE )
GAME( 1989, ffightj3,    ffight,   cps1_10MHz, ffight,   cps_state,   cps1,     ROT0,   "Capcom", "Final Fight (Japan 900613)", MACHINE_SUPPORTS_SAVE )
GAME( 1989, ffightjh,    ffight,   cps1_10MHz, ffight,   cps_state,   cps1,     ROT0,   "bootleg", "Street Smart / Final Fight (Japan, hack)", MACHINE_SUPPORTS_SAVE )
GAME( 1990, 1941,        0,        cps1_10MHz, 1941,     cps_state,   cps1,     ROT270, "Capcom", "1941: Counter Attack (World 900227)", MACHINE_SUPPORTS_SAVE )   // "ETC"
GAME( 1990, 1941r1,      1941,     cps1_10MHz, 1941,     cps_state,   cps1,     ROT270, "Capcom", "1941: Counter Attack (World)", MACHINE_SUPPORTS_SAVE )
GAME( 1990, 1941u,       1941,     cps1_10MHz, 1941,     cps_state,   cps1,     ROT270, "Capcom", "1941: Counter Attack (USA 900227)", MACHINE_SUPPORTS_SAVE )
GAME( 1990, 1941j,       1941,     cps1_10MHz, 1941,     cps_state,   cps1,     ROT270, "Capcom", "1941: Counter Attack (Japan)", MACHINE_SUPPORTS_SAVE )
GAME( 1990, mercs,       0,        cps1_10MHz, mercs,    cps_state,   cps1,     ROT270, "Capcom", "Mercs (World 900302)", MACHINE_SUPPORTS_SAVE )  // "ETC"
GAME( 1990, mercsu,      mercs,    cps1_10MHz, mercs,    cps_state,   cps1,     ROT270, "Capcom", "Mercs (USA 900608)", MACHINE_SUPPORTS_SAVE )
GAME( 1990, mercsur1,    mercs,    cps1_10MHz, mercs,    cps_state,   cps1,     ROT270, "Capcom", "Mercs (USA 900302)", MACHINE_SUPPORTS_SAVE )
GAME( 1990, mercsj,      mercs,    cps1_10MHz, mercs,    cps_state,   cps1,     ROT270, "Capcom", "Senjou no Ookami II (Japan 900302)", MACHINE_SUPPORTS_SAVE )
GAME( 1990, mtwins,      0,        cps1_10MHz, mtwins,   cps_state,   cps1,     ROT0,   "Capcom", "Mega Twins (World 900619)", MACHINE_SUPPORTS_SAVE ) // "ETC" - (c) Capcom U.S.A. but World "warning"
GAME( 1990, chikij,      mtwins,   cps1_10MHz, mtwins,   cps_state,   cps1,     ROT0,   "Capcom", "Chiki Chiki Boys (Japan 900619)", MACHINE_SUPPORTS_SAVE )
GAME( 1990, msword,      0,        cps1_10MHz, msword,   cps_state,   cps1,     ROT0,   "Capcom", "Magic Sword: Heroic Fantasy (World 900725)", MACHINE_SUPPORTS_SAVE )    // 25.07.1990  "OTHER COUNTRY"
GAME( 1990, mswordr1,    msword,   cps1_10MHz, msword,   cps_state,   cps1,     ROT0,   "Capcom", "Magic Sword: Heroic Fantasy (World 900623)", MACHINE_SUPPORTS_SAVE )    // 23.06.1990  "OTHER COUNTRY"
GAME( 1990, mswordu,     msword,   cps1_10MHz, msword,   cps_state,   cps1,     ROT0,   "Capcom", "Magic Sword: Heroic Fantasy (USA 900725)", MACHINE_SUPPORTS_SAVE )  // 25.07.1990  "U.S.A."
GAME( 1990, mswordj,     msword,   cps1_10MHz, msword,   cps_state,   cps1,     ROT0,   "Capcom", "Magic Sword: Heroic Fantasy (Japan 900623)", MACHINE_SUPPORTS_SAVE )    // 23.06.1990  "JAPAN"
GAME( 1990, cawing,      0,        cps1_10MHz, cawing,   cps_state,   cps1,     ROT0,   "Capcom", "Carrier Air Wing (World 901012)", MACHINE_SUPPORTS_SAVE )   // "ETC"
GAME( 1990, cawingr1,    cawing,   cps1_10MHz, cawing,   cps_state,   cps1,     ROT0,   "Capcom", "Carrier Air Wing (World 901009)", MACHINE_SUPPORTS_SAVE )   // "ETC"
GAME( 1990, cawingu,     cawing,   cps1_10MHz, cawing,   cps_state,   cps1,     ROT0,   "Capcom", "Carrier Air Wing (USA 901012)", MACHINE_SUPPORTS_SAVE )
GAME( 1990, cawingj,     cawing,   cps1_10MHz, cawing,   cps_state,   cps1,     ROT0,   "Capcom", "U.S. Navy (Japan 901012)", MACHINE_SUPPORTS_SAVE )
GAME( 1990, nemo,        0,        cps1_10MHz, nemo,     cps_state,   cps1,     ROT0,   "Capcom", "Nemo (World 901130)", MACHINE_SUPPORTS_SAVE )   // "ETC"
GAME( 1990, nemor1,      nemo,     cps1_10MHz, nemo,     cps_state,   cps1,     ROT0,   "Capcom", "Nemo (World 901109)", MACHINE_SUPPORTS_SAVE )   // "ETC"
GAME( 1990, nemoj,       nemo,     cps1_10MHz, nemo,     cps_state,   cps1,     ROT0,   "Capcom", "Nemo (Japan 901120)", MACHINE_SUPPORTS_SAVE )
GAME( 1991, sf2,         0,        cps1_10MHz, sf2,      cps_state,   cps1,     ROT0,   "Capcom", "Street Fighter II: The World Warrior (World 910522)", MACHINE_SUPPORTS_SAVE )   // "ETC"
GAME( 1991, sf2eb,       sf2,      cps1_10MHz, sf2,      cps_state,   cps1,     ROT0,   "Capcom", "Street Fighter II: The World Warrior (World 910214)", MACHINE_SUPPORTS_SAVE )   // "ETC"
GAME( 1991, sf2ed,       sf2,      cps1_10MHz, sf2,      cps_state,   cps1,     ROT0,   "Capcom", "Street Fighter II: The World Warrior (World 910318)", MACHINE_SUPPORTS_SAVE )   // "ETC"
GAME( 1991, sf2ee,       sf2,      cps1_10MHz, sf2,      cps_state,   sf2ee,    ROT0,   "Capcom", "Street Fighter II: The World Warrior (World 910228)", MACHINE_SUPPORTS_SAVE )   // "ETC"
GAME( 1991, sf2ua,       sf2,      cps1_10MHz, sf2,      cps_state,   cps1,     ROT0,   "Capcom", "Street Fighter II: The World Warrior (USA 910206)", MACHINE_SUPPORTS_SAVE )
GAME( 1991, sf2ub,       sf2,      cps1_10MHz, sf2,      cps_state,   cps1,     ROT0,   "Capcom", "Street Fighter II: The World Warrior (USA 910214)", MACHINE_SUPPORTS_SAVE )
GAME( 1991, sf2uc,       sf2,      cps1_10MHz, sf2,      cps_state,   cps1,     ROT0,   "Capcom", "Street Fighter II: The World Warrior (USA 910306)", MACHINE_SUPPORTS_SAVE )
GAME( 1991, sf2ud,       sf2,      cps1_10MHz, sf2,      cps_state,   cps1,     ROT0,   "Capcom", "Street Fighter II: The World Warrior (USA 910318)", MACHINE_SUPPORTS_SAVE )
GAME( 1991, sf2ue,       sf2,      cps1_10MHz, sf2,      cps_state,   sf2ee,    ROT0,   "Capcom", "Street Fighter II: The World Warrior (USA 910228)", MACHINE_SUPPORTS_SAVE )
GAME( 1991, sf2uf,       sf2,      cps1_10MHz, sf2,      cps_state,   cps1,     ROT0,   "Capcom", "Street Fighter II: The World Warrior (USA 910411)", MACHINE_SUPPORTS_SAVE )
GAME( 1991, sf2ug,       sf2,      cps1_10MHz, sf2,      cps_state,   cps1,     ROT0,   "Capcom", "Street Fighter II: The World Warrior (USA 910522, Rev. G)", MACHINE_SUPPORTS_SAVE )
GAME( 1991, sf2ui,       sf2,      cps1_10MHz, sf2,      cps_state,   cps1,     ROT0,   "Capcom", "Street Fighter II: The World Warrior (USA 910522, Rev. I)", MACHINE_SUPPORTS_SAVE )
GAME( 1991, sf2uk,       sf2,      cps1_10MHz, sf2,      cps_state,   cps1,     ROT0,   "Capcom", "Street Fighter II: The World Warrior (USA 911101)", MACHINE_SUPPORTS_SAVE )
GAME( 1991, sf2j,        sf2,      cps1_10MHz, sf2j,     cps_state,   cps1,     ROT0,   "Capcom", "Street Fighter II: The World Warrior (Japan 911210)", MACHINE_SUPPORTS_SAVE )
GAME( 1991, sf2ja,       sf2,      cps1_10MHz, sf2j,     cps_state,   cps1,     ROT0,   "Capcom", "Street Fighter II: The World Warrior (Japan 910214)", MACHINE_SUPPORTS_SAVE )
GAME( 1991, sf2jc,       sf2,      cps1_10MHz, sf2j,     cps_state,   cps1,     ROT0,   "Capcom", "Street Fighter II: The World Warrior (Japan 910306)", MACHINE_SUPPORTS_SAVE )
GAME( 1991, sf2jf,       sf2,      cps1_10MHz, sf2j,     cps_state,   cps1,     ROT0,   "Capcom", "Street Fighter II: The World Warrior (Japan 910411)", MACHINE_SUPPORTS_SAVE )
GAME( 1991, sf2jh,       sf2,      cps1_10MHz, sf2j,     cps_state,   cps1,     ROT0,   "Capcom", "Street Fighter II: The World Warrior (Japan 910522)", MACHINE_SUPPORTS_SAVE )
GAME( 1991, sf2jl,       sf2,      cps1_10MHz, sf2j,     cps_state,   cps1,     ROT0,   "Capcom", "Street Fighter II: The World Warrior (Japan 920312)", MACHINE_SUPPORTS_SAVE )
GAME( 1992, sf2ebbl,     sf2,      cps1_10MHz, sf2hack,  cps_state,   sf2hack,  ROT0,   "bootleg", "Street Fighter II: The World Warrior (TAB Austria, bootleg, set 1)", MACHINE_SUPPORTS_SAVE )       // 910214 - based on World version
GAME( 1992, sf2ebbl2,    sf2,      cps1_10MHz, sf2hack,  cps_state,   sf2hack,  ROT0,   "bootleg", "Street Fighter II: The World Warrior (TAB Austria, bootleg, set 3)", MACHINE_SUPPORTS_SAVE )       // 910214 - based on World version
GAME( 1992, sf2ebbl3,    sf2,      cps1_10MHz, sf2hack,  cps_state,   sf2hack,  ROT0,   "bootleg", "Street Fighter II: The World Warrior (TAB Austria, bootleg, set 4)", MACHINE_SUPPORTS_SAVE )       // 910214 - based on World version
GAME( 1992, sf2stt,      sf2,      cps1_10MHz, sf2hack,  cps_state,   sf2hack,  ROT0,   "bootleg", "Street Fighter II: The World Warrior (TAB Austria, bootleg, set 2)", MACHINE_NOT_WORKING | MACHINE_SUPPORTS_SAVE )    // 910214 - based on World version
GAME( 1992, sf2rk,       sf2,      cps1_10MHz, sf2hack,  cps_state,   sf2hack,  ROT0,   "bootleg", "Street Fighter II: The World Warrior (RK, bootleg)", MACHINE_NOT_WORKING | MACHINE_SUPPORTS_SAVE )    // 920211 (pcb)
GAME( 1991, sf2qp1,      sf2,      cps1_10MHz, sf2,      cps_state,   cps1,     ROT0,   "bootleg", "Street Fighter II: The World Warrior (Quicken Pt-I, bootleg)", MACHINE_SUPPORTS_SAVE )     // 910214 - based on World version
GAME( 1991, sf2thndr,    sf2,      cps1_10MHz, sf2,      cps_state,   sf2thndr, ROT0,   "bootleg", "Street Fighter II: The World Warrior (Thunder Edition, bootleg)", MACHINE_SUPPORTS_SAVE )  // 910214 - based on World version
GAME( 1991, 3wonders,    0,        cps1_10MHz, 3wonders, cps_state,   cps1,     ROT0,   "Capcom", "Three Wonders (World 910520)", MACHINE_SUPPORTS_SAVE )  // "ETC"
GAME( 1991, 3wondersr1,  3wonders, cps1_10MHz, 3wonders, cps_state,   cps1,     ROT0,   "Capcom", "Three Wonders (World 910513)", MACHINE_SUPPORTS_SAVE )  // "ETC"
GAME( 1991, 3wondersu,   3wonders, cps1_10MHz, 3wonders, cps_state,   cps1,     ROT0,   "Capcom", "Three Wonders (USA 910520)", MACHINE_SUPPORTS_SAVE )
GAME( 1991, wonder3,     3wonders, cps1_10MHz, 3wonders, cps_state,   cps1,     ROT0,   "Capcom", "Wonder 3 (Japan 910520)", MACHINE_SUPPORTS_SAVE )
GAME( 1991, 3wondersb,   3wonders, cps1_10MHz, 3wonders, cps_state,   cps1,     ROT0,   "bootleg", "Three Wonders (bootleg)", MACHINE_SUPPORTS_SAVE )   // 910520 - based on World version
GAME( 1991, 3wondersh,   3wonders, cps1_10MHz, 3wonders, cps_state,   cps1,     ROT0,   "bootleg", "Three Wonders (hack)", MACHINE_SUPPORTS_SAVE ) // 910520 - based on World version
GAME( 1991, kod,         0,        cps1_10MHz, kod,      cps_state,   cps1,     ROT0,   "Capcom", "The King of Dragons (World 910805)", MACHINE_SUPPORTS_SAVE )    // "ETC"
GAME( 1991, kodr1,       kod,      cps1_10MHz, kodr1,    cps_state,   cps1,     ROT0,   "Capcom", "The King of Dragons (World 910711)", MACHINE_SUPPORTS_SAVE )    // "ETC"
GAME( 1991, kodu,        kod,      cps1_10MHz, kod,      cps_state,   cps1,     ROT0,   "Capcom", "The King of Dragons (USA 910910)", MACHINE_SUPPORTS_SAVE )
GAME( 1991, kodj,        kod,      cps1_10MHz, kod,      cps_state,   cps1,     ROT0,   "Capcom", "The King of Dragons (Japan 910805, B-Board 90629B-3)", MACHINE_SUPPORTS_SAVE )
GAME( 1991, kodja,       kod,      cps1_10MHz, kod,      cps_state,   cps1,     ROT0,   "Capcom", "The King of Dragons (Japan 910805, B-Board 89625B-1)", MACHINE_SUPPORTS_SAVE )
GAME( 1991, captcomm,    0,        cps1_10MHz, captcomm, cps_state,   cps1,     ROT0,   "Capcom", "Captain Commando (World 911202)", MACHINE_SUPPORTS_SAVE )   // "OTHER COUNTRY"
GAME( 1991, captcommr1,  captcomm, cps1_10MHz, captcomm, cps_state,   cps1,     ROT0,   "Capcom", "Captain Commando (World 911014)", MACHINE_SUPPORTS_SAVE )   // "OTHER COUNTRY"
GAME( 1991, captcommu,   captcomm, cps1_10MHz, captcomm, cps_state,   cps1,     ROT0,   "Capcom", "Captain Commando (USA 910928)", MACHINE_SUPPORTS_SAVE )
GAME( 1991, captcommj,   captcomm, cps1_10MHz, captcomm, cps_state,   cps1,     ROT0,   "Capcom", "Captain Commando (Japan 911202)", MACHINE_SUPPORTS_SAVE )
GAME( 1991, captcommjr1, captcomm, cps1_10MHz, captcomm, cps_state,   cps1,     ROT0,   "Capcom", "Captain Commando (Japan 910928)", MACHINE_SUPPORTS_SAVE )
GAME( 1991, captcommb,   captcomm, cps1_10MHz, captcomm, cps_state,   cps1,     ROT0,   "bootleg", "Captain Commando (bootleg)", MACHINE_SUPPORTS_SAVE )    // 911014 - based on World version
GAME( 1991, knights,     0,        cps1_10MHz, knights,  cps_state,   cps1,     ROT0,   "Capcom", "Knights of the Round (World 911127)", MACHINE_SUPPORTS_SAVE )   // "ETC"
GAME( 1991, knightsu,    knights,  cps1_10MHz, knights,  cps_state,   cps1,     ROT0,   "Capcom", "Knights of the Round (USA 911127)", MACHINE_SUPPORTS_SAVE )
GAME( 1991, knightsj,    knights,  cps1_10MHz, knights,  cps_state,   cps1,     ROT0,   "Capcom", "Knights of the Round (Japan 911127, B-Board 91634B-2)", MACHINE_SUPPORTS_SAVE )
GAME( 1991, knightsja,   knights,  cps1_10MHz, knights,  cps_state,   cps1,     ROT0,   "Capcom", "Knights of the Round (Japan 911127, B-Board 89625B-1)", MACHINE_SUPPORTS_SAVE )
GAME( 1992, sf2ce,       0,        cps1_12MHz, sf2,      cps_state,   cps1,     ROT0,   "Capcom", "Street Fighter II': Champion Edition (World 920513)", MACHINE_SUPPORTS_SAVE )   // "ETC"
GAME( 1992, sf2ceea,     sf2ce,    cps1_12MHz, sf2,      cps_state,   cps1,     ROT0,   "Capcom", "Street Fighter II': Champion Edition (World 920313)", MACHINE_SUPPORTS_SAVE )   // "ETC"
GAME( 1992, sf2ceua,     sf2ce,    cps1_12MHz, sf2,      cps_state,   cps1,     ROT0,   "Capcom", "Street Fighter II': Champion Edition (USA 920313)", MACHINE_SUPPORTS_SAVE )
GAME( 1992, sf2ceub,     sf2ce,    cps1_12MHz, sf2,      cps_state,   cps1,     ROT0,   "Capcom", "Street Fighter II': Champion Edition (USA 920513)", MACHINE_SUPPORTS_SAVE )
GAME( 1992, sf2ceuc,     sf2ce,    cps1_12MHz, sf2,      cps_state,   cps1,     ROT0,   "Capcom", "Street Fighter II': Champion Edition (USA 920803)", MACHINE_SUPPORTS_SAVE )
GAME( 1992, sf2ceja,     sf2ce,    cps1_12MHz, sf2,      cps_state,   cps1,     ROT0,   "Capcom", "Street Fighter II': Champion Edition (Japan 920322)", MACHINE_SUPPORTS_SAVE )
GAME( 1992, sf2cejb,     sf2ce,    cps1_12MHz, sf2,      cps_state,   cps1,     ROT0,   "Capcom", "Street Fighter II': Champion Edition (Japan 920513)", MACHINE_SUPPORTS_SAVE )
GAME( 1992, sf2cejc,     sf2ce,    cps1_12MHz, sf2,      cps_state,   cps1,     ROT0,   "Capcom", "Street Fighter II': Champion Edition (Japan 920803)", MACHINE_SUPPORTS_SAVE )
GAME( 1992, sf2bhh,      sf2ce,    cps1_12MHz, sf2,      cps_state,   sf2rb,    ROT0,   "bootleg", "Street Fighter II': Champion Edition (Hung Hsi, bootleg)", MACHINE_SUPPORTS_SAVE )
GAME( 1992, sf2rb,       sf2ce,    cps1_12MHz, sf2,      cps_state,   sf2rb,    ROT0,   "bootleg", "Street Fighter II': Champion Edition (Rainbow, bootleg, set 1)", MACHINE_SUPPORTS_SAVE )           // 920322 - based on World version
GAME( 1992, sf2rb2,      sf2ce,    cps1_12MHz, sf2,      cps_state,   sf2rb2,   ROT0,   "bootleg", "Street Fighter II': Champion Edition (Rainbow, bootleg, set 2)", MACHINE_SUPPORTS_SAVE )           // 920322 - based on World version
GAME( 1992, sf2rb3,      sf2ce,    cps1_12MHz, sf2,      cps_state,   cps1,     ROT0,   "bootleg", "Street Fighter II': Champion Edition (Rainbow, bootleg, set 3)", MACHINE_SUPPORTS_SAVE )           // 920322 - based on World version
GAME( 1992, sf2red,      sf2ce,    cps1_12MHz, sf2,      cps_state,   cps1,     ROT0,   "bootleg", "Street Fighter II': Champion Edition (Red Wave, bootleg)", MACHINE_SUPPORTS_SAVE )         // 920313 - based on World version
GAME( 1992, sf2v004,     sf2ce,    cps1_12MHz, sf2,      cps_state,   cps1,     ROT0,   "bootleg", "Street Fighter II': Champion Edition (V004, bootleg)", MACHINE_SUPPORTS_SAVE )             // 102092 !!! - based on (heavily modified) World version
GAME( 1992, sf2acc,      sf2ce,    cps1_12MHz, sf2,      cps_state,   cps1,     ROT0,   "bootleg", "Street Fighter II': Champion Edition (Accelerator!, bootleg, set 1)", MACHINE_SUPPORTS_SAVE )          // 920313 - based on World version
GAME( 1992, sf2acca,     sf2ce,    cps1_12MHz, sf2,      cps_state,   cps1,     ROT0,   "bootleg", "Street Fighter II': Champion Edition (Accelerator!, bootleg, set 2)", MACHINE_SUPPORTS_SAVE )          // 920313 - based on World version
GAME( 1992, sf2accp2,    sf2ce,    cps1_12MHz, sf2,      cps_state,   cps1,     ROT0,   "bootleg", "Street Fighter II': Champion Edition (Accelerator Pt.II, bootleg)", MACHINE_SUPPORTS_SAVE )        // 920313 - based on World version
GAME( 1992, sf2amf,      sf2ce,    cps1_12MHz, sf2amf,   cps_state,   sf2hack,  ROT0,   "bootleg", "Street Fighter II': Champion Edition (Alpha Magic-F, bootleg)", MACHINE_NOT_WORKING | MACHINE_SUPPORTS_SAVE )     // 920313 - based on World version
GAME( 1992, sf2amf2,     sf2ce,    cps1_12MHz, sf2hack,  cps_state,   sf2hack,  ROT0,   "bootleg", "Street Fighter II': Champion Edition (L735 Test Rom, bootleg)", MACHINE_SUPPORTS_SAVE )     // 920313 - based on World version
GAME( 1992, sf2dkot2,    sf2ce,    cps1_12MHz, sf2,      cps_state,   cps1,     ROT0,   "bootleg", "Street Fighter II': Champion Edition (Double K.O. Turbo II, bootleg)", MACHINE_SUPPORTS_SAVE ) // 902140 !!! - based on USA version
GAME( 1992, sf2ceblp,    sf2ce,    cps1_10MHz, sf2,      cps_state,   sf2ceblp, ROT0,   "bootleg", "Street Fighter II': Champion Edition (protected bootleg on non-dash board)", MACHINE_SUPPORTS_SAVE )          // 920313 - based on USA version
GAME( 1992, sf2cebltw,   sf2ce,    cps1_12MHz, sf2,      cps_state,   cps1,     ROT0,   "bootleg", "Street Fighter II': Champion Edition ('Taiwan' bootleg with PAL)", MACHINE_SUPPORTS_SAVE ) // 'Taiwan'
GAME( 1992, sf2m2,       sf2ce,    cps1_12MHz, sf2m2,    cps_state,   sf2hack,  ROT0,   "bootleg", "Street Fighter II': Champion Edition (M2, bootleg)", MACHINE_SUPPORTS_SAVE )               // 920313 - based on World version
GAME( 1992, sf2m3,       sf2ce,    sf2m3,      sf2,      cps_state,   cps1,     ROT0,   "bootleg", "Street Fighter II': Champion Edition (M3, bootleg)", MACHINE_SUPPORTS_SAVE )               // 920313 - based on USA version
GAME( 1992, sf2m4,       sf2ce,    cps1_12MHz, sf2m4,    cps_state,   sf2hack,  ROT0,   "bootleg", "Street Fighter II': Champion Edition (M4, bootleg)", MACHINE_SUPPORTS_SAVE )               // 920322 - based on Japan version
GAME( 1992, sf2m5,       sf2ce,    cps1_12MHz, sf2hack,  cps_state,   sf2hack,  ROT0,   "bootleg", "Street Fighter II': Champion Edition (M5, bootleg)", MACHINE_SUPPORTS_SAVE )               // 920313 - based on World version
GAME( 1992, sf2m6,       sf2ce,    cps1_12MHz, sf2hack,  cps_state,   sf2hack,  ROT0,   "bootleg", "Street Fighter II': Champion Edition (M6, bootleg)", MACHINE_SUPPORTS_SAVE )               // 811102 !!! - based on World version
GAME( 1992, sf2m7,       sf2ce,    cps1_12MHz, sf2hack,  cps_state,   sf2hack,  ROT0,   "bootleg", "Street Fighter II': Champion Edition (M7, bootleg)", MACHINE_SUPPORTS_SAVE )               // 920313 - based on World version
GAME( 1992, sf2m8,       sf2ce,    sf2m3,      sf2,      cps_state,   sf2m8,    ROT0,   "bootleg", "Street Fighter II': Champion Edition (M8, bootleg)", MACHINE_SUPPORTS_SAVE )               // 920313 - based on USA version
GAME( 1992, sf2m10,      sf2ce,    sf2m10,     sf2,      cps_state,   cps1,     ROT0,   "bootleg", "Street Fighter II': Champion Edition (M10, bootleg)", MACHINE_SUPPORTS_SAVE )
GAME( 1992, sf2yyc,      sf2ce,    cps1_12MHz, sf2hack,  cps_state,   sf2hack,  ROT0,   "bootleg", "Street Fighter II': Champion Edition (YYC, bootleg)", MACHINE_SUPPORTS_SAVE )              // 920313 - based on World version
GAME( 1992, sf2koryu,    sf2ce,    cps1_12MHz, sf2hack,  cps_state,   sf2hack,  ROT0,   "bootleg", "Street Fighter II': Champion Edition (Xiang Long, Chinese bootleg)", MACHINE_SUPPORTS_SAVE )       // 811102 !!! - based on World version
GAME( 1992, sf2dongb,    sf2ce,    cps1_12MHz, sf2,      cps_state,   sf2dongb, ROT0,   "bootleg", "Street Fighter II': Champion Edition (Dongfang Bubai protection, bootleg)", MACHINE_SUPPORTS_SAVE ) // 920313 - based on World version
GAME( 1992, cworld2j,    0,        cps1_12MHz, cworld2j, cps_state,   cps1,     ROT0,   "Capcom", "Adventure Quiz Capcom World 2 (Japan 920611)", MACHINE_SUPPORTS_SAVE )
GAME( 1992, cworld2ja,   cworld2j, cps1_12MHz, cworld2j, cps_state,   cps1,     ROT0,   "Capcom", "Adventure Quiz Capcom World 2 (Japan 920611, B-Board 90629B-3, no battery)", MACHINE_SUPPORTS_SAVE )
GAME( 1992, cworld2jb,   cworld2j, cps1_12MHz, cworld2j, cps_state,   cps1,     ROT0,   "Capcom", "Adventure Quiz Capcom World 2 (Japan 920611, B-Board 91634B-2)", MACHINE_SUPPORTS_SAVE )
GAME( 1992, varth,       0,        cps1_12MHz, varth,    cps_state,   cps1,     ROT270, "Capcom", "Varth: Operation Thunderstorm (World 920714)", MACHINE_SUPPORTS_SAVE )  // "ETC"    // 12MHz verified
GAME( 1992, varthr1,     varth,    cps1_12MHz, varth,    cps_state,   cps1,     ROT270, "Capcom", "Varth: Operation Thunderstorm (World 920612)", MACHINE_SUPPORTS_SAVE )  // "ETC"
GAME( 1992, varthu,      varth,    cps1_12MHz, varth,    cps_state,   cps1,     ROT270, "Capcom (Romstar license)", "Varth: Operation Thunderstorm (USA 920612)", MACHINE_SUPPORTS_SAVE )
GAME( 1992, varthj,      varth,    cps1_12MHz, varth,    cps_state,   cps1,     ROT270, "Capcom", "Varth: Operation Thunderstorm (Japan 920714)", MACHINE_SUPPORTS_SAVE )
GAME( 1992, varthjr,     varth,    cps1_12MHz, varth,    cps_state,   cps1,     ROT270, "Capcom", "Varth: Operation Thunderstorm (Japan Resale Ver. 920714)", MACHINE_SUPPORTS_SAVE )
GAME( 1992, qad,         0,        cps1_12MHz, qad,      cps_state,   cps1,     ROT0,   "Capcom", "Quiz & Dragons: Capcom Quiz Game (USA 920701)", MACHINE_SUPPORTS_SAVE ) // 12MHz verified
GAME( 1994, qadjr,       qad,      cps1_12MHz, qadjr,    cps_state,   cps1,     ROT0,   "Capcom", "Quiz & Dragons: Capcom Quiz Game (Japan Resale Ver. 940921)", MACHINE_SUPPORTS_SAVE )
GAME( 1992, wof,         0,        qsound,     wof,      cps_state,   wof,      ROT0,   "Capcom", "Warriors of Fate (World 921031)", MACHINE_SUPPORTS_SAVE )   // "ETC"
GAME( 1992, wofr1,       wof,      qsound,     wof,      cps_state,   wof,      ROT0,   "Capcom", "Warriors of Fate (World 921002)", MACHINE_SUPPORTS_SAVE )   // "ETC"
GAME( 1992, wofu,        wof,      qsound,     wof,      cps_state,   wof,      ROT0,   "Capcom", "Warriors of Fate (USA 921031)", MACHINE_SUPPORTS_SAVE ) // World "warning"
GAME( 1992, wofa,        wof,      qsound,     wof,      cps_state,   wof,      ROT0,   "Capcom", "Sangokushi II (Asia 921005)", MACHINE_SUPPORTS_SAVE )   // World "warning"
GAME( 1992, wofj,        wof,      qsound,     wof,      cps_state,   wof,      ROT0,   "Capcom", "Tenchi wo Kurau II: Sekiheki no Tatakai (Japan 921031)", MACHINE_SUPPORTS_SAVE )
GAME( 1999, wofhfh,      wof,      wofhfh,     wofhfh,   cps_state,   cps1,     ROT0,   "bootleg", "Huo Feng Huang (Chinese bootleg of Sangokushi II)", MACHINE_SUPPORTS_SAVE )    // 921005 - based on Asia version
GAME( 1992, sf2hf,       0,        cps1_12MHz, sf2,      cps_state,   cps1,     ROT0,   "Capcom", "Street Fighter II': Hyper Fighting (World 921209)", MACHINE_SUPPORTS_SAVE ) // "ETC"
GAME( 1992, sf2hfu,      sf2hf,    cps1_12MHz, sf2,      cps_state,   cps1,     ROT0,   "Capcom", "Street Fighter II': Hyper Fighting (USA 921209)", MACHINE_SUPPORTS_SAVE )
GAME( 1992, sf2hfj,      sf2hf,    cps1_12MHz, sf2j,     cps_state,   cps1,     ROT0,   "Capcom", "Street Fighter II' Turbo: Hyper Fighting (Japan 921209)", MACHINE_SUPPORTS_SAVE )
GAME( 1993, dino,        0,        qsound,     dino,     cps_state,   dino,     ROT0,   "Capcom", "Cadillacs and Dinosaurs (World 930201)", MACHINE_SUPPORTS_SAVE )    // "ETC"
GAME( 1993, dinou,       dino,     qsound,     dino,     cps_state,   dino,     ROT0,   "Capcom", "Cadillacs and Dinosaurs (USA 930201)", MACHINE_SUPPORTS_SAVE )
GAME( 1993, dinoj,       dino,     qsound,     dino,     cps_state,   dino,     ROT0,   "Capcom", "Cadillacs: Kyouryuu Shin Seiki (Japan 930201)", MACHINE_SUPPORTS_SAVE )
GAME( 1993, dinohunt,    dino,     wofhfh,     dinoh,    cps_state,   dinohunt, ROT0,   "bootleg", "Dinosaur Hunter (Chinese bootleg of Cadillacs and Dinosaurs)", MACHINE_SUPPORTS_SAVE )  // 930223 - based on Asia TW version, the original is still undumped
GAME( 1993, punisher,    0,        qsound,     punisher, cps_state,   punisher, ROT0,   "Capcom", "The Punisher (World 930422)", MACHINE_SUPPORTS_SAVE )   // "ETC"
GAME( 1993, punisheru,   punisher, qsound,     punisher, cps_state,   punisher, ROT0,   "Capcom", "The Punisher (USA 930422)", MACHINE_SUPPORTS_SAVE )
GAME( 1993, punisherh,   punisher, qsound,     punisher, cps_state,   punisher, ROT0,   "Capcom", "The Punisher (Hispanic 930422)", MACHINE_SUPPORTS_SAVE )
GAME( 1993, punisherj,   punisher, qsound,     punisher, cps_state,   punisher, ROT0,   "Capcom", "The Punisher (Japan 930422)", MACHINE_SUPPORTS_SAVE )
GAME( 1993, punisherbz,  punisher, wofhfh,     punisherbz, cps_state, cps1,     ROT0,   "bootleg", "Biaofeng Zhanjing (Chinese bootleg of The Punisher)", MACHINE_SUPPORTS_SAVE )  // (c) 2002, they ripped the sound from Final Fight!
GAME( 1993, slammast,    0,        qsound,     slammast, cps_state,   slammast, ROT0,   "Capcom", "Saturday Night Slam Masters (World 930713)", MACHINE_SUPPORTS_SAVE )    // "ETC"
GAME( 1993, slammastu,   slammast, qsound,     slammast, cps_state,   slammast, ROT0,   "Capcom", "Saturday Night Slam Masters (USA 930713)", MACHINE_SUPPORTS_SAVE )
GAME( 1993, mbomberj,    slammast, qsound,     slammast, cps_state,   slammast, ROT0,   "Capcom", "Muscle Bomber: The Body Explosion (Japan 930713)", MACHINE_SUPPORTS_SAVE )
GAME( 1993, mbombrd,     0,        qsound,     slammast, cps_state,   slammast, ROT0,   "Capcom", "Muscle Bomber Duo: Ultimate Team Battle (World 931206)", MACHINE_SUPPORTS_SAVE )    // "ETC"
GAME( 1993, mbombrdj,    mbombrd,  qsound,     slammast, cps_state,   slammast, ROT0,   "Capcom", "Muscle Bomber Duo: Heat Up Warriors (Japan 931206)", MACHINE_SUPPORTS_SAVE )
GAME( 1994, pnickj,      0,        cps1_12MHz, pnickj,   cps_state,   cps1,     ROT0,   "Compile (Capcom license)", "Pnickies (Japan 940608)", MACHINE_SUPPORTS_SAVE )
GAME( 1995, qtono2j,     0,        cps1_12MHz, qtono2j,  cps_state,   cps1,     ROT0,   "Capcom", "Quiz Tonosama no Yabou 2: Zenkoku-ban (Japan 950123)", MACHINE_SUPPORTS_SAVE )
GAME( 1995, megaman,     0,        cps1_12MHz, megaman,  cps_state,   cps1,     ROT0,   "Capcom", "Mega Man: The Power Battle (CPS1, USA 951006)", MACHINE_SUPPORTS_SAVE )
GAME( 1995, megamana,    megaman,  cps1_12MHz, megaman,  cps_state,   cps1,     ROT0,   "Capcom", "Mega Man: The Power Battle (CPS1, Asia 951006)", MACHINE_SUPPORTS_SAVE )
GAME( 1995, rockmanj,    megaman,  cps1_12MHz, rockmanj, cps_state,   cps1,     ROT0,   "Capcom", "Rockman: The Power Battle (CPS1, Japan 950922)", MACHINE_SUPPORTS_SAVE )
GAME( 2000, ganbare,     0,        ganbare,    ganbare,  cps_state,   ganbare,  ROT0,   "Capcom", "Ganbare! Marine Kun (Japan 2K0411)", MACHINE_SUPPORTS_SAVE )

/* Games released on CPS-1 hardware by Mitchell */

GAME( 1995, pang3,       0,        pang3,      pang3,    cps_state,   pang3,    ROT0,   "Mitchell", "Pang! 3 (Euro 950601)", MACHINE_SUPPORTS_SAVE )
GAME( 1995, pang3r1,     pang3,    pang3,      pang3,    cps_state,   pang3,    ROT0,   "Mitchell", "Pang! 3 (Euro 950511)", MACHINE_SUPPORTS_SAVE )
GAME( 1995, pang3j,      pang3,    pang3,      pang3,    cps_state,   pang3,    ROT0,   "Mitchell", "Pang! 3: Kaitou Tachi no Karei na Gogo (Japan 950511)", MACHINE_SUPPORTS_SAVE )
GAME( 1995, pang3b,      pang3,    pang3,      pang3b,   cps_state,   pang3b,   ROT0,   "bootleg", "Pang! 3 (bootleg)", MACHINE_SUPPORTS_SAVE )    // 950511 - based on Euro version

/* Home 'CPS Changer' Unit - For MESS */

CONS( 1994, wofch,  0, 0,     qsound,     wofch,      cps_state, wof,      "Capcom", "Tenchi wo Kurau II: Sekiheki no Tatakai (CPS Changer, Japan 921031)", MACHINE_SUPPORTS_SAVE )
CONS( 1995, sfzch,  0, 0,     cps1_12MHz, sfzch,      cps_state, cps1,     "Capcom", "Street Fighter Zero (CPS Changer, Japan 951020)", MACHINE_SUPPORTS_SAVE )
// are these 2 legit sets, or did somebody region hack it?
CONS( 1995, sfach,  sfzch, 0, cps1_12MHz, sfzch,      cps_state, cps1,     "Capcom", "Street Fighter Alpha: Warriors' Dreams (CPS Changer, Publicity USA 950727)", MACHINE_SUPPORTS_SAVE )
CONS( 1995, sfzbch, sfzch, 0, cps1_12MHz, sfzch,      cps_state, cps1,     "Capcom", "Street Fighter Zero (CPS Changer, Brazil 950727)", MACHINE_SUPPORTS_SAVE )

// Ken Sei Mogura: Street Fighter II - see kenseim.c
