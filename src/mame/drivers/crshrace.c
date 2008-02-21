/***************************************************************************

Crash Race       (c) 1993 Video System Co.

driver by Nicola Salmoria

Notes:
- Keep player1 button1&2 pressed while entering service mode to get an
  extended menu


Stephh's notes (based on the games M68000 code and some tests) :

0) all games

  - There seems to be preliminary support for 3 simulataneous players, but the
    game resets before the race starts if the 3 players don't play against each
    other ! I can't tell however if it's an ingame or an emulation bug.
    To test this, change CRSHRACE_3P_HACK to 1, set the "Reset on P.O.S.T. Error"
    Dip Switch to "No" (because of the ROMS patch), and set the "Maximum Players"
    Dip Switch to "3".

  - There are 2 buttons for each player (one for accel and one for brake),
    the 3rd one being for "debug" purpose (see notes below).
  - The "Difficulty" Dip Switch also determines the time to complete the race.
  - "Coin B" Dip Switches only has an effect if you set the "Coin Slot"
    Dip Switch to "Same".
    If you set it to "Individual", it will use the coinage from "Coin A".
  - COIN3 adds 1 credit only if you set the "Coin Slot" Dip Switch to "Same".
    If you set it to "Individual", it will add 1 credit to fake player 3,
    thus having no effect.

  - DSW 3 bit 0 used to be a "Max Players" Dip Switch (but it is now unused) :
      * when Off, 2 players cabinet
      * when On,  3 players cabinet

  - DSW 3 bits 1 to 3 used to be a "Coin C" Dip Switch (but they are now unused)
    which is in fact similar to the table for "Coin A" and "Coin B" :
       1   2   3      Coinage
      Off Off Off      1C_1C
      On  Off Off      2C_1C
      Off On  Off      3C_1C
      On  On  Off      1C_2C
      Off Off On       1C_3C
      On  Off On       1C_4C
      Off On  On       1C_5C
      On  On  On       1C_6C

  - DSW 3 bit 7 is tested only if an error has occured during P.O.S.T. :
      * when Off, the game is reset
      * when On,  don't bother with the error and continue

  - There are NO differences between Country code 0x0004 ("World") and 0x0005.
    Country code is stored at 0xfe1c9e and can have the following values :
      * 0000 : Japan
      * 0001 : USA & Canada
      * 0002 : Korea
      * 0003 : Hong Kong & Taiwan
      * 0004 : World
      * 0005 : ???

  - When in the "test mode" with the extended menu, pressing "P1 button 3"
    causes a "freeze"; press it again to unfreeze.
  - When in the "test mode" with the extended menu, pressing "P2 button 3"
    has an unknown effect (sound related ?), but sets bit 2 at 0xfe0019.

  - There are writes to 0xfff00c and 0xfff00d, but these addresses aren't mapped :
      * when "Flip Screen" Dip Switch is Off, 0x0001 is written to 0xfff00c.w
      * when "Flip Screen" Dip Switch is Off, 0xc001 is written to 0xfff00c.w
    I can't tell however what is the effect of these writes 8(


1) 'crshrace'

  - Even if there is code for it, there is NO possibility to select a 3 players
    game due to code at 0x003778 which "invalidates" the previous reading of DSW 3 :

    00363C: 13F8 F00B 00FE 1C85      move.b  $f00b.w, $fe1c85.l
    ...
    003650: 4639 00FE 1C85           not.b   $fe1c85.l
    ...
    003778: 51F9 00FE 1C85           sf      $fe1c85.l

  - When in the "test mode" with the extended menu, pressing "P1 start" +
    "P2 start" + the 3 buttons of the SAME player causes a reset of the game
    (code at 0x003182).
  - When in the "test play" menu of the "test mode", pressing "P1 button 1" +
    "P1 button 2" + "P2 button 1" + "P2 button 2" + "P2 button 3" returns
    to the "test mode" (code at 0x0040de).


2) 'crshrac2'

  - Even if there is code for it, there is NO possibility to select a 3 players
    game due to code at 0x003796 which "invalidates" the previous reading of DSW 3 :

    00365A: 13F8 F00B 00FE 1C85      move.b  $f00b.w, $fe1c85.l
    ...
    00366E: 4639 00FE 1C85           not.b   $fe1c85.l
    ...
    003796: 51F9 00FE 1C85           sf      $fe1c85.l

  - When in the "test mode" with the extended menu, pressing "P1 start" +
    "P2 start" + the 3 buttons of the SAME player causes a reset of the game
    (code at 0x0031a0).
  - When in the "test play" menu of the "test mode", pressing "P1 button 1" +
    "P1 button 2" + "P2 button 1" + "P2 button 2" + "P2 button 3" returns
    to the "test mode" (code at 0x0040fc).

  - I can't determine the effect of DSW 1 bit 4 8( All I can tell is that code
    at 0x00ea9c is called when initialising the race "parameters".


TODO:
- handle screen flip correctly
- sprite lag - I think it needs sprites to be delayed TWO frames
- is bg color in service mode right (blue)? Should it be black instead?
- handling of layer priority & enable might not be correct, though it should be
  enough to run this game.
- unknown writes to fff044/fff046. They look like two more scroll registers,
  but for what? The first starts at 0 when going over the start line and
  increases during the race

***************************************************************************/

#include "driver.h"
#include "deprecat.h"
#include "video/konamiic.h"
#include "sound/2610intf.h"
#include "crshrace.h"


#define CRSHRACE_3P_HACK	0


static READ16_HANDLER( extrarom1_r )
{
	UINT8 *rom = memory_region(REGION_USER1);

	offset *= 2;

	return rom[offset] | (rom[offset+1] << 8);
}

static READ16_HANDLER( extrarom2_r )
{
	UINT8 *rom = memory_region(REGION_USER2);

	offset *= 2;

	return rom[offset] | (rom[offset+1] << 8);
}

static WRITE8_HANDLER( crshrace_sh_bankswitch_w )
{
	UINT8 *rom = memory_region(REGION_CPU2) + 0x10000;

	memory_set_bankptr(1,rom + (data & 0x03) * 0x8000);
}


static int pending_command;

static WRITE16_HANDLER( sound_command_w )
{
	if (ACCESSING_LSB)
	{
		pending_command = 1;
		soundlatch_w(offset,data & 0xff);
		cpunum_set_input_line(Machine, 1, INPUT_LINE_NMI, PULSE_LINE);
	}
}

static READ16_HANDLER( country_sndpending_r )
{
	return readinputport(5) | (pending_command ? 0x8000 : 0);
}

static WRITE8_HANDLER( pending_command_clear_w )
{
	pending_command = 0;
}



static ADDRESS_MAP_START( readmem, ADDRESS_SPACE_PROGRAM, 16 )
	AM_RANGE(0x000000, 0x07ffff) AM_READ(MRA16_ROM)
	AM_RANGE(0x300000, 0x3fffff) AM_READ(extrarom1_r)
	AM_RANGE(0x400000, 0x4fffff) AM_READ(extrarom2_r)
	AM_RANGE(0x500000, 0x5fffff) AM_READ(extrarom2_r)	/* mirror */
	AM_RANGE(0xa00000, 0xa0ffff) AM_READ(MRA16_RAM)
	AM_RANGE(0xd00000, 0xd01fff) AM_READ(MRA16_RAM)
	AM_RANGE(0xe00000, 0xe01fff) AM_READ(MRA16_RAM)
	AM_RANGE(0xfe0000, 0xfeffff) AM_READ(MRA16_RAM)
	AM_RANGE(0xffd000, 0xffdfff) AM_READ(MRA16_RAM)
	AM_RANGE(0xffe000, 0xffefff) AM_READ(MRA16_RAM)
	AM_RANGE(0xfff000, 0xfff001) AM_READ(input_port_0_word_r)
	AM_RANGE(0xfff002, 0xfff003) AM_READ(input_port_1_word_r)
	AM_RANGE(0xfff004, 0xfff005) AM_READ(input_port_2_word_r)
	AM_RANGE(0xfff006, 0xfff007) AM_READ(country_sndpending_r)
	AM_RANGE(0xfff00a, 0xfff00b) AM_READ(input_port_3_word_r)
	AM_RANGE(0xfff00e, 0xfff00f) AM_READ(input_port_4_word_r)
ADDRESS_MAP_END

static ADDRESS_MAP_START( writemem, ADDRESS_SPACE_PROGRAM, 16 )
	AM_RANGE(0x000000, 0x07ffff) AM_WRITE(MWA16_ROM)
	AM_RANGE(0xa00000, 0xa0ffff) AM_WRITE(MWA16_RAM) AM_BASE(&spriteram16_2) AM_SIZE(&spriteram_2_size)			// RAM-5
	AM_RANGE(0xd00000, 0xd01fff) AM_WRITE(crshrace_videoram1_w) AM_BASE(&crshrace_videoram1)				// RAM-3 H/L
	AM_RANGE(0xe00000, 0xe01fff) AM_WRITE(MWA16_RAM) AM_BASE(&spriteram16) AM_SIZE(&spriteram_size)				// RAM-6
	AM_RANGE(0xffc000, 0xffc001) AM_WRITE(crshrace_roz_bank_w)
	AM_RANGE(0xfe0000, 0xfeffff) AM_WRITE(MWA16_RAM)	/* work RAM */								// RAM-1 H/L
	AM_RANGE(0xffd000, 0xffdfff) AM_WRITE(crshrace_videoram2_w) AM_BASE(&crshrace_videoram2)				// RAM-2 H/L
	AM_RANGE(0xffe000, 0xffefff) AM_WRITE(paletteram16_xGGGGGBBBBBRRRRR_word_w) AM_BASE(&paletteram16)	// RAM-4 H/L
	AM_RANGE(0xfff000, 0xfff001) AM_WRITE(crshrace_gfxctrl_w)
	AM_RANGE(0xfff008, 0xfff009) AM_WRITE(sound_command_w)
	AM_RANGE(0xfff020, 0xfff03f) AM_WRITE(MWA16_RAM) AM_BASE(&K053936_0_ctrl)
	AM_RANGE(0xfff044, 0xfff047) AM_WRITE(MWA16_RAM)	// ??? moves during race
ADDRESS_MAP_END

static ADDRESS_MAP_START( sound_readmem, ADDRESS_SPACE_PROGRAM, 8 )
	AM_RANGE(0x0000, 0x77ff) AM_READ(MRA8_ROM)
	AM_RANGE(0x7800, 0x7fff) AM_READ(MRA8_RAM)
	AM_RANGE(0x8000, 0xffff) AM_READ(MRA8_BANK1)
ADDRESS_MAP_END

static ADDRESS_MAP_START( sound_writemem, ADDRESS_SPACE_PROGRAM, 8 )
	AM_RANGE(0x0000, 0x77ff) AM_WRITE(MWA8_ROM)
	AM_RANGE(0x7800, 0x7fff) AM_WRITE(MWA8_RAM)
	AM_RANGE(0x8000, 0xffff) AM_WRITE(MWA8_ROM)
ADDRESS_MAP_END

static ADDRESS_MAP_START( sound_readport, ADDRESS_SPACE_IO, 8 )
	ADDRESS_MAP_FLAGS( AMEF_ABITS(8) )
	AM_RANGE(0x04, 0x04) AM_READ(soundlatch_r)
	AM_RANGE(0x08, 0x08) AM_READ(YM2610_status_port_0_A_r)
	AM_RANGE(0x0a, 0x0a) AM_READ(YM2610_status_port_0_B_r)
ADDRESS_MAP_END

static ADDRESS_MAP_START( sound_writeport, ADDRESS_SPACE_IO, 8 )
	ADDRESS_MAP_FLAGS( AMEF_ABITS(8) )
	AM_RANGE(0x00, 0x00) AM_WRITE(crshrace_sh_bankswitch_w)
	AM_RANGE(0x04, 0x04) AM_WRITE(pending_command_clear_w)
	AM_RANGE(0x08, 0x08) AM_WRITE(YM2610_control_port_0_A_w)
	AM_RANGE(0x09, 0x09) AM_WRITE(YM2610_data_port_0_A_w)
	AM_RANGE(0x0a, 0x0a) AM_WRITE(YM2610_control_port_0_B_w)
	AM_RANGE(0x0b, 0x0b) AM_WRITE(YM2610_data_port_0_B_w)
ADDRESS_MAP_END



static INPUT_PORTS_START( crshrace )
	PORT_START
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(1)	// "Accel"
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(1)	// "Brake"
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_PLAYER(1)
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0100, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x0200, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_BIT( 0x0400, IP_ACTIVE_LOW, IPT_COIN3 )
	PORT_BIT( 0x0800, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x1000, IP_ACTIVE_LOW, IPT_START2 )
	PORT_BIT( 0x2000, IP_ACTIVE_LOW, IPT_SERVICE2 )				// "Test"
	PORT_BIT( 0x4000, IP_ACTIVE_LOW, IPT_TILT )
	PORT_BIT( 0x8000, IP_ACTIVE_LOW, IPT_SERVICE1 )

	PORT_START
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(2)	// "Accel"
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(2)	// "Brake"
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_PLAYER(2)
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START
	/* DSW1 : 0xfe1c83 = !(0xfff004) */
	PORT_DIPNAME( 0x0100, 0x0100, "Coin Slot" )
	PORT_DIPSETTING(      0x0100, "Same" )
	PORT_DIPSETTING(      0x0000, "Individual" )
	PORT_DIPNAME( 0x0e00, 0x0e00, DEF_STR( Coin_A ) )
	PORT_DIPSETTING(      0x0a00, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(      0x0c00, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(      0x0e00, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(      0x0800, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(      0x0600, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(      0x0400, DEF_STR( 1C_4C ) )
	PORT_DIPSETTING(      0x0200, DEF_STR( 1C_5C ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( 1C_6C ) )
	PORT_DIPNAME( 0x7000, 0x7000, DEF_STR( Coin_B ) )
	PORT_DIPSETTING(      0x5000, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(      0x6000, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(      0x7000, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(      0x4000, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(      0x3000, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(      0x2000, DEF_STR( 1C_4C ) )
	PORT_DIPSETTING(      0x1000, DEF_STR( 1C_5C ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( 1C_6C ) )
	PORT_DIPNAME( 0x8000, 0x8000, "2 to Start, 1 to Cont." )	// Other desc. was too long !
	PORT_DIPSETTING(      0x8000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	/* DSW2 : 0xfe1c84 = !(0xfff005) */
	PORT_DIPNAME( 0x0001, 0x0001, DEF_STR( Flip_Screen ) )
	PORT_DIPSETTING(      0x0001, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0002, 0x0002, DEF_STR( Demo_Sounds ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0002, DEF_STR( On ) )
	PORT_DIPNAME( 0x0004, 0x0004, DEF_STR( Free_Play ) )
	PORT_DIPSETTING(      0x0004, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_SERVICE( 0x0008, IP_ACTIVE_LOW )
	PORT_DIPNAME( 0x0010, 0x0010, DEF_STR( Unused ) )
	PORT_DIPSETTING(      0x0010, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0020, 0x0020, DEF_STR( Unused ) )
	PORT_DIPSETTING(      0x0020, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x00c0, 0x00c0, DEF_STR( Difficulty ) )
	PORT_DIPSETTING(      0x0080, DEF_STR( Easy ) )
	PORT_DIPSETTING(      0x00c0, DEF_STR( Normal ) )
	PORT_DIPSETTING(      0x0040, DEF_STR( Hard ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( Hardest ) )

	PORT_START
	/* DSW3 : 0xfe1c85 = !(0xfff00b) */
#if CRSHRACE_3P_HACK
	PORT_DIPNAME( 0x0001, 0x0001, "Maximum Players" )
	PORT_DIPSETTING(      0x0001, "2" )
	PORT_DIPSETTING(      0x0000, "3" )
	PORT_DIPNAME( 0x000e, 0x000e, "Coin C" )
	PORT_DIPSETTING(      0x000a, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(      0x000c, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(      0x000e, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(      0x0008, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(      0x0006, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(      0x0004, DEF_STR( 1C_4C ) )
	PORT_DIPSETTING(      0x0002, DEF_STR( 1C_5C ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( 1C_6C ) )
#else
	PORT_DIPNAME( 0x0001, 0x0001, DEF_STR( Unused ) )
	PORT_DIPSETTING(      0x0001, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0002, 0x0002, DEF_STR( Unused ) )
	PORT_DIPSETTING(      0x0002, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0004, 0x0004, DEF_STR( Unused ) )
	PORT_DIPSETTING(      0x0004, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0008, 0x0008, DEF_STR( Unused ) )
	PORT_DIPSETTING(      0x0008, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
#endif
	PORT_DIPNAME( 0x0010, 0x0010, DEF_STR( Unused ) )
	PORT_DIPSETTING(      0x0010, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0020, 0x0020, DEF_STR( Unused ) )
	PORT_DIPSETTING(      0x0020, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0040, 0x0040, DEF_STR( Unused ) )
	PORT_DIPSETTING(      0x0040, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0080, 0x0080, "Reset on P.O.S.T. Error" )	// Check code at 0x003812
	PORT_DIPSETTING(      0x0000, DEF_STR( No ) )
	PORT_DIPSETTING(      0x0080, DEF_STR( Yes ) )

	PORT_START
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY PORT_PLAYER(3)
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY PORT_PLAYER(3)
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY PORT_PLAYER(3)
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_PLAYER(3)
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(3)
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(3)
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_PLAYER(3)
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_START3 )

	PORT_START
	PORT_DIPNAME( 0x0f00, 0x0100, "Country" )
	PORT_DIPSETTING(      0x0100, DEF_STR( World ) )
	PORT_DIPSETTING(      0x0800, "USA & Canada" )
	PORT_DIPSETTING(      0x0000, DEF_STR( Japan ) )
	PORT_DIPSETTING(      0x0200, "Korea" )
	PORT_DIPSETTING(      0x0400, "Hong Kong & Taiwan" )
/*
    the following are all the same and seem to act like the World setting, possibly
    with a slightly different attract sequence
    PORT_DIPSETTING(      0x0300, "5" )
    PORT_DIPSETTING(      0x0500, "5" )
    PORT_DIPSETTING(      0x0600, "5" )
    PORT_DIPSETTING(      0x0700, "5" )
    PORT_DIPSETTING(      0x0900, "5" )
    PORT_DIPSETTING(      0x0a00, "5" )
    PORT_DIPSETTING(      0x0b00, "5" )
    PORT_DIPSETTING(      0x0c00, "5" )
    PORT_DIPSETTING(      0x0d00, "5" )
    PORT_DIPSETTING(      0x0e00, "5" )
    PORT_DIPSETTING(      0x0f00, "5" )
*/
	PORT_BIT( 0x8000, IP_ACTIVE_HIGH, IPT_SPECIAL )	/* pending sound command */
INPUT_PORTS_END

/* Same as 'crshrace', but additional "unknown" Dip Switch (see notes) */
static INPUT_PORTS_START( crshrac2 )
	PORT_START
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(1)	// "Accel"
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(1)	// "Brake"
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_PLAYER(1)
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0100, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x0200, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_BIT( 0x0400, IP_ACTIVE_LOW, IPT_COIN3 )
	PORT_BIT( 0x0800, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x1000, IP_ACTIVE_LOW, IPT_START2 )
	PORT_BIT( 0x2000, IP_ACTIVE_LOW, IPT_SERVICE2 )				// "Test"
	PORT_BIT( 0x4000, IP_ACTIVE_LOW, IPT_TILT )
	PORT_BIT( 0x8000, IP_ACTIVE_LOW, IPT_SERVICE1 )

	PORT_START
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(2)	// "Accel"
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(2)	// "Brake"
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_PLAYER(2)
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START
	/* DSW2 : 0xfe1c84 = !(0xfff005) */
	PORT_DIPNAME( 0x0001, 0x0001, DEF_STR( Flip_Screen ) )
	PORT_DIPSETTING(      0x0001, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0002, 0x0002, DEF_STR( Demo_Sounds ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0002, DEF_STR( On ) )
	PORT_DIPNAME( 0x0004, 0x0004, DEF_STR( Free_Play ) )
	PORT_DIPSETTING(      0x0004, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_SERVICE( 0x0008, IP_ACTIVE_LOW )
	PORT_DIPNAME( 0x0010, 0x0010, DEF_STR( Unknown ) )		// Check code at 0x00ea36
	PORT_DIPSETTING(      0x0010, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0020, 0x0020, DEF_STR( Unused ) )
	PORT_DIPSETTING(      0x0020, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x00c0, 0x00c0, DEF_STR( Difficulty ) )
	PORT_DIPSETTING(      0x0080, DEF_STR( Easy ) )
	PORT_DIPSETTING(      0x00c0, DEF_STR( Normal ) )
	PORT_DIPSETTING(      0x0040, DEF_STR( Hard ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( Hardest ) )
	/* DSW1 : 0xfe1c83 = !(0xfff004) */
	PORT_DIPNAME( 0x0100, 0x0100, "Coin Slot" )
	PORT_DIPSETTING(      0x0100, "Same" )
	PORT_DIPSETTING(      0x0000, "Individual" )
	PORT_DIPNAME( 0x0e00, 0x0e00, DEF_STR( Coin_A ) )
	PORT_DIPSETTING(      0x0a00, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(      0x0c00, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(      0x0e00, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(      0x0800, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(      0x0600, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(      0x0400, DEF_STR( 1C_4C ) )
	PORT_DIPSETTING(      0x0200, DEF_STR( 1C_5C ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( 1C_6C ) )
	PORT_DIPNAME( 0x7000, 0x7000, DEF_STR( Coin_B ) )
	PORT_DIPSETTING(      0x5000, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(      0x6000, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(      0x7000, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(      0x4000, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(      0x3000, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(      0x2000, DEF_STR( 1C_4C ) )
	PORT_DIPSETTING(      0x1000, DEF_STR( 1C_5C ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( 1C_6C ) )
	PORT_DIPNAME( 0x8000, 0x8000, "2 to Start, 1 to Cont." )	// Other desc. was too long !
	PORT_DIPSETTING(      0x8000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )

	PORT_START
	/* DSW3 : 0xfe1c85 = !(0xfff00b) */
#if CRSHRACE_3P_HACK
	PORT_DIPNAME( 0x0001, 0x0001, "Maximum Players" )
	PORT_DIPSETTING(      0x0001, "2" )
	PORT_DIPSETTING(      0x0000, "3" )
	PORT_DIPNAME( 0x000e, 0x000e, "Coin C" )
	PORT_DIPSETTING(      0x000a, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(      0x000c, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(      0x000e, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(      0x0008, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(      0x0006, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(      0x0004, DEF_STR( 1C_4C ) )
	PORT_DIPSETTING(      0x0002, DEF_STR( 1C_5C ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( 1C_6C ) )
#else
	PORT_DIPNAME( 0x0001, 0x0001, DEF_STR( Unused ) )
	PORT_DIPSETTING(      0x0001, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0002, 0x0002, DEF_STR( Unused ) )
	PORT_DIPSETTING(      0x0002, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0004, 0x0004, DEF_STR( Unused ) )
	PORT_DIPSETTING(      0x0004, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0008, 0x0008, DEF_STR( Unused ) )
	PORT_DIPSETTING(      0x0008, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
#endif
	PORT_DIPNAME( 0x0010, 0x0010, DEF_STR( Unused ) )
	PORT_DIPSETTING(      0x0010, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0020, 0x0020, DEF_STR( Unused ) )
	PORT_DIPSETTING(      0x0020, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0040, 0x0040, DEF_STR( Unused ) )
	PORT_DIPSETTING(      0x0040, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0080, 0x0080, "Reset on P.O.S.T. Error" )	// Check code at 0x003830
	PORT_DIPSETTING(      0x0000, DEF_STR( No ) )
	PORT_DIPSETTING(      0x0080, DEF_STR( Yes ) )

	PORT_START
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY PORT_PLAYER(3)
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY PORT_PLAYER(3)
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY PORT_PLAYER(3)
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_PLAYER(3)
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(3)
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(3)
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_PLAYER(3)
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_START3 )

	PORT_START
	PORT_DIPNAME( 0x0f00, 0x0100, "Country" )
	PORT_DIPSETTING(      0x0100, DEF_STR( World ) )
	PORT_DIPSETTING(      0x0800, "USA & Canada" )
	PORT_DIPSETTING(      0x0000, DEF_STR( Japan ) )
	PORT_DIPSETTING(      0x0200, "Korea" )
	PORT_DIPSETTING(      0x0400, "Hong Kong & Taiwan" )
/*
    the following are all the same and seem to act like the World setting, possibly
    with a slightly different attract sequence
    PORT_DIPSETTING(      0x0300, "5" )
    PORT_DIPSETTING(      0x0500, "5" )
    PORT_DIPSETTING(      0x0600, "5" )
    PORT_DIPSETTING(      0x0700, "5" )
    PORT_DIPSETTING(      0x0900, "5" )
    PORT_DIPSETTING(      0x0a00, "5" )
    PORT_DIPSETTING(      0x0b00, "5" )
    PORT_DIPSETTING(      0x0c00, "5" )
    PORT_DIPSETTING(      0x0d00, "5" )
    PORT_DIPSETTING(      0x0e00, "5" )
    PORT_DIPSETTING(      0x0f00, "5" )
*/
	PORT_BIT( 0x8000, IP_ACTIVE_HIGH, IPT_SPECIAL )	/* pending sound command */
INPUT_PORTS_END



static const gfx_layout charlayout =
{
	8,8,
	RGN_FRAC(1,1),
	8,
	{ 0, 1, 2, 3, 4, 5, 6, 7 },
	{ 0*8, 1*8, 2*8, 3*8, 4*8, 5*8, 6*8, 7*8 },
	{ 0*64, 1*64, 2*64, 3*64, 4*64, 5*64, 6*64, 7*64 },
	64*8
};

static const gfx_layout tilelayout =
{
	16,16,
	RGN_FRAC(1,1),
	4,
	{ 0, 1, 2, 3 },
	{ 2*4, 3*4, 0*4, 1*4, 6*4, 7*4, 4*4, 5*4,
			10*4, 11*4, 8*4, 9*4, 14*4, 15*4, 12*4, 13*4 },
	{ 0*64, 1*64, 2*64, 3*64, 4*64, 5*64, 6*64, 7*64,
			8*64, 9*64, 10*64, 11*64, 12*64, 13*64, 14*64, 15*64 },
	128*8
};

static const gfx_layout spritelayout =
{
	16,16,
	RGN_FRAC(1,1),
	4,
	{ 0, 1, 2, 3 },
	{ 1*4, 0*4, 3*4, 2*4, 5*4, 4*4, 7*4, 6*4,
			9*4, 8*4, 11*4, 10*4, 13*4, 12*4, 15*4, 14*4 },
	{ 0*64, 1*64, 2*64, 3*64, 4*64, 5*64, 6*64, 7*64,
			8*64, 9*64, 10*64, 11*64, 12*64, 13*64, 14*64, 15*64 },
	128*8
};

static GFXDECODE_START( crshrace )
	GFXDECODE_ENTRY( REGION_GFX1, 0, charlayout,     0,  1 )
	GFXDECODE_ENTRY( REGION_GFX2, 0, tilelayout,   256, 16 )
	GFXDECODE_ENTRY( REGION_GFX3, 0, spritelayout, 512, 32 )
GFXDECODE_END



static void irqhandler(int irq)
{
	cpunum_set_input_line(Machine, 1,0,irq ? ASSERT_LINE : CLEAR_LINE);
}

static const struct YM2610interface ym2610_interface =
{
	irqhandler,
	REGION_SOUND1,
	REGION_SOUND2
};



static MACHINE_DRIVER_START( crshrace )

	/* basic machine hardware */
	MDRV_CPU_ADD(M68000,16000000)	/* 16 MHz ??? */
	MDRV_CPU_PROGRAM_MAP(readmem,writemem)
	MDRV_CPU_VBLANK_INT(irq1_line_hold,1)

	MDRV_CPU_ADD(Z80,4000000)	/* 4 MHz ??? */
	/* audio CPU */
	MDRV_CPU_PROGRAM_MAP(sound_readmem,sound_writemem)
	MDRV_CPU_IO_MAP(sound_readport,sound_writeport)

	/* video hardware */
	MDRV_VIDEO_ATTRIBUTES(VIDEO_BUFFERS_SPRITERAM)

	MDRV_SCREEN_ADD("main", RASTER)
	MDRV_SCREEN_REFRESH_RATE(60)
	MDRV_SCREEN_FORMAT(BITMAP_FORMAT_INDEXED16)
	MDRV_SCREEN_SIZE(64*8, 32*8)
	MDRV_SCREEN_VISIBLE_AREA(0*8, 40*8-1, 0*8, 28*8-1)

	MDRV_GFXDECODE(crshrace)
	MDRV_PALETTE_LENGTH(2048)

	MDRV_VIDEO_START(crshrace)
	MDRV_VIDEO_EOF(crshrace)
	MDRV_VIDEO_UPDATE(crshrace)

	/* sound hardware */
	MDRV_SPEAKER_STANDARD_STEREO("left", "right")

	MDRV_SOUND_ADD(YM2610, 8000000)
	MDRV_SOUND_CONFIG(ym2610_interface)
	MDRV_SOUND_ROUTE(0, "left",  0.25)
	MDRV_SOUND_ROUTE(0, "right", 0.25)
	MDRV_SOUND_ROUTE(1, "left",  1.0)
	MDRV_SOUND_ROUTE(2, "right", 1.0)
MACHINE_DRIVER_END


ROM_START( crshrace )
	ROM_REGION( 0x80000, REGION_CPU1, 0 )	/* 68000 code */
	ROM_LOAD16_WORD_SWAP( "1",            0x000000, 0x80000, CRC(21e34fb7) SHA1(be47b4a9bce2d6ce0a127dffe032c61547b2a3c0) )

	ROM_REGION( 0x100000, REGION_USER1, 0 )	/* extra ROM */
	ROM_LOAD( "w21",          0x000000, 0x100000, CRC(a5df7325) SHA1(614095a086164af5b5e73245744411187d81deec) )

	ROM_REGION( 0x100000, REGION_USER2, 0 )	/* extra ROM */
	ROM_LOAD( "w22",          0x000000, 0x100000, CRC(fc9d666d) SHA1(45aafcce82b668f93e51b5e4d092b1d0077e5192) )

	ROM_REGION( 0x30000, REGION_CPU2, 0 )	/* 64k for the audio CPU + banks */
	ROM_LOAD( "2",            0x00000, 0x20000, CRC(e70a900f) SHA1(edfe5df2dab5a7dccebe1a6f978144bcd516ab03) )
	ROM_RELOAD(               0x10000, 0x20000 )

	ROM_REGION( 0x100000, REGION_GFX1, ROMREGION_DISPOSE )
	ROM_LOAD( "h895",         0x000000, 0x100000, CRC(36ad93c3) SHA1(f68f229dd1a1f8bfd3b8f73b6627f5f00f809d34) )

	ROM_REGION( 0x400000, REGION_GFX2, ROMREGION_DISPOSE )
	ROM_LOAD( "w18",          0x000000, 0x100000, CRC(b15df90d) SHA1(56e38e6c40a02553b6b8c5282aa8f16b20779ebf) )
	ROM_LOAD( "w19",          0x100000, 0x100000, CRC(28326b93) SHA1(997e9b250b984b012ce1d165add59c741fb18171) )
	ROM_LOAD( "w20",          0x200000, 0x100000, CRC(d4056ad1) SHA1(4b45b14aa0766d7aef72f060e1cd28d67690d5fe) )
	/* 300000-3fffff empty */

	ROM_REGION( 0x400000, REGION_GFX3, ROMREGION_DISPOSE )
	ROM_LOAD( "h897",         0x000000, 0x200000, CRC(e3230128) SHA1(758c65f113481cf25bf0359deecd6736a7c9ee7e) )
	ROM_LOAD( "h896",         0x200000, 0x200000, CRC(fff60233) SHA1(56b4b708883a80761dc5f9184780477d72b80351) )

	ROM_REGION( 0x100000, REGION_SOUND1, 0 ) /* sound samples */
	ROM_LOAD( "h894",         0x000000, 0x100000, CRC(d53300c1) SHA1(4c3ff7d3156791cb960c28845a5f1906605bce55) )

	ROM_REGION( 0x100000, REGION_SOUND2, 0 ) /* sound samples */
	ROM_LOAD( "h893",         0x000000, 0x100000, CRC(32513b63) SHA1(c4ede4aaa2611cedb53d47448422a1926acf3052) )
ROM_END

ROM_START( crshrac2 )
	ROM_REGION( 0x80000, REGION_CPU1, 0 )	/* 68000 code */
	ROM_LOAD16_WORD_SWAP( "01-ic10.bin",  0x000000, 0x80000, CRC(b284aacd) SHA1(f0ef279cdec30eb32e8aa8cdd51e289b70f2d6f5) )

	ROM_REGION( 0x100000, REGION_USER1, 0 )	/* extra ROM */
	ROM_LOAD( "w21",          0x000000, 0x100000, CRC(a5df7325) SHA1(614095a086164af5b5e73245744411187d81deec) )	// IC14.BIN

	ROM_REGION( 0x100000, REGION_USER2, 0 )	/* extra ROM */
	ROM_LOAD( "w22",          0x000000, 0x100000, CRC(fc9d666d) SHA1(45aafcce82b668f93e51b5e4d092b1d0077e5192) )	// IC13.BIN

	ROM_REGION( 0x30000, REGION_CPU2, 0 )	/* 64k for the audio CPU + banks */
	ROM_LOAD( "2",            0x00000, 0x20000, CRC(e70a900f) SHA1(edfe5df2dab5a7dccebe1a6f978144bcd516ab03) )	// 02-IC58.BIN
	ROM_RELOAD(               0x10000, 0x20000 )

	ROM_REGION( 0x100000, REGION_GFX1, ROMREGION_DISPOSE )
	ROM_LOAD( "h895",         0x000000, 0x100000, CRC(36ad93c3) SHA1(f68f229dd1a1f8bfd3b8f73b6627f5f00f809d34) )	// IC50.BIN

	ROM_REGION( 0x400000, REGION_GFX2, ROMREGION_DISPOSE )
	ROM_LOAD( "w18",          0x000000, 0x100000, CRC(b15df90d) SHA1(56e38e6c40a02553b6b8c5282aa8f16b20779ebf) )	// ROM-A.BIN
	ROM_LOAD( "w19",          0x100000, 0x100000, CRC(28326b93) SHA1(997e9b250b984b012ce1d165add59c741fb18171) )	// ROM-B.BIN
	ROM_LOAD( "w20",          0x200000, 0x100000, CRC(d4056ad1) SHA1(4b45b14aa0766d7aef72f060e1cd28d67690d5fe) )	// ROM-C.BIN
	/* 300000-3fffff empty */

	ROM_REGION( 0x400000, REGION_GFX3, ROMREGION_DISPOSE )
	ROM_LOAD( "h897",         0x000000, 0x200000, CRC(e3230128) SHA1(758c65f113481cf25bf0359deecd6736a7c9ee7e) )	// IC29.BIN
	ROM_LOAD( "h896",         0x200000, 0x200000, CRC(fff60233) SHA1(56b4b708883a80761dc5f9184780477d72b80351) )	// IC75.BIN

	ROM_REGION( 0x100000, REGION_SOUND1, 0 ) /* sound samples */
	ROM_LOAD( "h894",         0x000000, 0x100000, CRC(d53300c1) SHA1(4c3ff7d3156791cb960c28845a5f1906605bce55) )	// IC73.BIN

	ROM_REGION( 0x100000, REGION_SOUND2, 0 ) /* sound samples */
	ROM_LOAD( "h893",         0x000000, 0x100000, CRC(32513b63) SHA1(c4ede4aaa2611cedb53d47448422a1926acf3052) )	// IC69.BIN
ROM_END


#ifdef UNUSED_FUNCTION
void crshrace_patch_code(UINT16 offset)
{
	/* A hack which shows 3 player mode in code which is disabled */
	UINT16 *RAM = (UINT16 *)memory_region(REGION_CPU1);
	RAM[(offset + 0)/2] = 0x4e71;
	RAM[(offset + 2)/2] = 0x4e71;
	RAM[(offset + 4)/2] = 0x4e71;
}
#endif


static DRIVER_INIT( crshrace )
{
	#if CRSHRACE_3P_HACK
	crshrace_patch_code(0x003778);
	#endif
}

static DRIVER_INIT( crshrac2 )
{
	#if CRSHRACE_3P_HACK
	crshrace_patch_code(0x003796);
	#endif
}


GAME( 1993, crshrace, 0,        crshrace, crshrace, crshrace, ROT270, "Video System Co.", "Lethal Crash Race (set 1)", GAME_NO_COCKTAIL )
GAME( 1993, crshrac2, crshrace, crshrace, crshrac2, crshrac2, ROT270, "Video System Co.", "Lethal Crash Race (set 2)", GAME_NO_COCKTAIL )
