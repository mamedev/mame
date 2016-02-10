// license:BSD-3-Clause
// copyright-holders:David Haywood
/* Fit of Fighting / The History of Martial Arts / 'BB' */

/* NIX or Novatecnia (both spanish) may have produced these
   its probably NIX due to somes similarities with Pirates

    Supported Games                  Rip-off of

    Fit of Fighting                  Art of Fighting (neogeo.c)
    The History of Martial Arts      Fighter's History (deco32.c)
    'BB' Untitled Prototype          -none, original-

   'BB' Prototype isn't a game as such, 'BB' was the label on
   the prototype board. which appears to have been used simply
   for testings / practice.  There is no 'game' to it, no
   title screen, simply 3 characters, 1 portrait, 1 background,
   the characters don't appear to have any moves other than
   basic walking around.  There is also no sound.

   The lack of Sound, GFX and Gameplay in this 'game' are NOT
   emulation bugs.

   There are some unused GFX in the roms, it might be interesting
   to try and put them together and see what they form.

*/

/*

68k interrupts (fitfight)
lev 1 : 0x64 : 0000 150C -
lev 2 : 0x68 : 0000 3676 -
lev 3 : 0x6c : 0000 1752 -
lev 4 : 0x70 : 0000 1768 -
lev 5 : 0x74 : 0000 177e -
lev 6 : 0x78 : 0000 1794 -
lev 7 : 0x7c : 0000 17aa -


todo:

fix scrolling
sound
fix s7prite colour problems.
should these be considered clones or not since the game has
been rewritten, they just use the gfx ...

Stephh's notes :

1) 'fitfight'

  - Gameplay :
      * The player who beats the other wins a point and the round number is increased.
      * When there is a draw, nobody scores a point, but the round number is increased.
      * The level ends when a players reaches the needed number of points or when the
        round number is > the maximum round number.

  - Winner :
      * Player 1 wins if his number of points is >= player 2 number of points.
      * Player 2 wins if his number of points is >  player 1 number of points.

  - The "winner rule" introduces an ingame bug : when you play with LEFT player
    against the CPU, you can win a level by only scoring "draws".


2) 'histryma'

  - Gameplay :
      * The player who beats the other wins a point.
      * When there is a draw, both players score a point.
      * The level ends when a players reaches the needed number of points or when the
        total of points is >= the maximum number of points.

  - Winner :
      * Player 1 wins if his number of points is >  player 2 number of points.
      * Player 2 wins if his number of points is >= player 1 number of points.

  - The "winner rule" introduces an ingame bug : when you play with RIGHT player
    against the CPU, you can win a level by only scoring "draws".

  - The "test mode" isn't correct for the Dip Switches !

*/

#include "emu.h"
#include "cpu/m68000/m68000.h"
#include "cpu/upd7810/upd7810.h"
#include "sound/okim6295.h"
#include "includes/fitfight.h"

READ16_MEMBER( fitfight_state::hotmindff_unk_r )
{
	// won't boot unless things in here change, this is p1/p2 inputs in fitfight
	return space.machine().rand();
}

READ16_MEMBER(fitfight_state::fitfight_700000_r)
{
	UINT16 data = m_fof_700000_data;
	return (data << 2);
}

READ16_MEMBER(fitfight_state::histryma_700000_r)
{
	UINT16 data = (m_fof_700000_data & 0x00AA);
	data |= ((m_fof_700000_data & 0x0055) >> 2);
	return (data);
}

READ16_MEMBER(fitfight_state::bbprot_700000_r)
{
	UINT16 data = 0;
	data  =  (m_fof_700000_data & 0x000b);
	data |= ((m_fof_700000_data & 0x01d0) >> 2);
	data |= ((m_fof_700000_data & 0x0004) << 6);
	data |= ((m_fof_700000_data & 0x0020) << 2);
	return (data);
}

WRITE16_MEMBER(fitfight_state::fitfight_700000_w)
{
	COMBINE_DATA(&m_fof_700000[offset]);        // needed for scrolling

	if (data < 0x0200)              // to avoid considering writes of 0x0200
		m_fof_700000_data = data;
}

static ADDRESS_MAP_START( fitfight_main_map, AS_PROGRAM, 16, fitfight_state )
	AM_RANGE(0x000000, 0x0fffff) AM_ROM

	AM_RANGE(0x100000, 0x100001) AM_WRITEONLY AM_SHARE("fof_100000")
	//written at scanline 5, allways 1. Used by histryma/fitfight @0x0000ec2c/@0x0000f076

	AM_RANGE(0x200000, 0x200001) AM_READ_PORT("P1_P2")
	AM_RANGE(0x300000, 0x300001) AM_READ_PORT("EXTRA")  // for 'histryma' only
	AM_RANGE(0x400000, 0x400001) AM_READ_PORT("SYSTEM_DSW2")
	AM_RANGE(0x500000, 0x500001) AM_READ_PORT("DSW3_DSW1")

	AM_RANGE(0x600000, 0x600001) AM_WRITEONLY AM_SHARE("fof_600000")
	//  Is 0x600000 controlling the slave audio CPU? data is 0x1111000zzzzzzzzz (9 sign. bits)
	//  Used by histryma/fitfight:
	//      @0x000031ae/0x00002b3a: 0xF000, once, during POST
	//       0xe001ae/0xe00096 holds the address (0x600000), 0xe001b2/0xe0009a holds the word to output
	//      @0x00003294/0x00002c1a: word content of 0xe001b2
	//      @0x000032cc/?: 0xF0dd byte from 0xe001b5, dd seems to be allways 0xFD
	//      @0x000036bc/?: 0xF0FD when inserting coin
	//      @0x000037a6/0x000030e6: 0x??dd byte from 0xe08c05, 0xF101 then 0xF001/0xF157 then 0xF057

//  AM_RANGE(0x700000, 0x700001) AM_READ(xxxx) /* see init */
	AM_RANGE(0x700000, 0x700001) AM_WRITE(fitfight_700000_w) AM_SHARE("fof_700000")
	//  kept at 0xe07900/0xe04c56

	AM_RANGE(0x800000, 0x800001) AM_RAM AM_SHARE("fof_800000")
	//written at scanline 1, allways 0. Used by histryma/fitfight @0x00001d76/@0x00000f6a

	AM_RANGE(0x900000, 0x900001) AM_RAM AM_SHARE("fof_900000") //mid tilemap scroll
	//  fitfigth: @0x00002b42,@0x00000f76
	//  histryma: @0x000031b6,@0x00001d82

	AM_RANGE(0xa00000, 0xa00001) AM_RAM AM_SHARE("fof_a00000") //bak tilemap scroll
	//  fitfight: @0x00002b4a,@0x00000f82
	//  histryma: @0x000031be,@0x00001d8e

	AM_RANGE(0xb00000, 0xb03fff) AM_RAM /* unused layer? */
	AM_RANGE(0xb04000, 0xb07fff) AM_RAM_WRITE(fof_bak_tileram_w) AM_SHARE("fof_bak_tileram")
	AM_RANGE(0xb08000, 0xb0bfff) AM_RAM_WRITE(fof_mid_tileram_w) AM_SHARE("fof_mid_tileram")
	AM_RANGE(0xb0c000, 0xb0ffff) AM_RAM_WRITE(fof_txt_tileram_w) AM_SHARE("fof_txt_tileram")

	AM_RANGE(0xb10000, 0xb13fff) AM_RAM //used by histryma @0x0000b25a
	AM_RANGE(0xb14000, 0xb17fff) AM_RAM //used by histryma @0x0000b25a,b270
	AM_RANGE(0xb18000, 0xb1bfff) AM_RAM //used by histryma @0x0000b25a,b270,b286

	AM_RANGE(0xc00000, 0xc00fff) AM_RAM_DEVWRITE("palette", palette_device, write) AM_SHARE("palette")

	AM_RANGE(0xd00000, 0xd007ff) AM_RAM AM_SHARE("spriteram")

	AM_RANGE(0xe00000, 0xe0ffff) AM_RAM
	AM_RANGE(0xff0000, 0xffffff) AM_RAM // hot mind uses RAM here (mirror?)
ADDRESS_MAP_END

static ADDRESS_MAP_START( bbprot_main_map, AS_PROGRAM, 16, fitfight_state )
	AM_RANGE(0x000000, 0x0fffff) AM_ROM

	AM_RANGE(0x100000, 0x100001) AM_WRITEONLY AM_SHARE("fof_100000")

	AM_RANGE(0x300000, 0x300001) AM_READ_PORT("P1_P2")
	AM_RANGE(0x380000, 0x380001) AM_READ_PORT("EXTRA")
	AM_RANGE(0x400000, 0x400001) AM_READ_PORT("SYSTEM_DSW2")
	AM_RANGE(0x480000, 0x480001) AM_READ_PORT("DSW3_DSW1")

	AM_RANGE(0x600000, 0x600001) AM_WRITEONLY AM_SHARE("fof_600000")

	AM_RANGE(0x700000, 0x700001) AM_READWRITE(bbprot_700000_r, fitfight_700000_w) AM_SHARE("fof_700000")

	AM_RANGE(0x800000, 0x800001) AM_WRITEONLY AM_SHARE("fof_800000")
	AM_RANGE(0x900000, 0x900001) AM_WRITEONLY AM_SHARE("fof_900000")
	AM_RANGE(0xa00000, 0xa00001) AM_WRITEONLY AM_SHARE("fof_a00000")

	AM_RANGE(0xb00000, 0xb03fff) AM_WRITENOP /* unused layer? */
	AM_RANGE(0xb04000, 0xb07fff) AM_RAM_WRITE(fof_bak_tileram_w) AM_SHARE("fof_bak_tileram")
	AM_RANGE(0xb08000, 0xb0bfff) AM_RAM_WRITE(fof_mid_tileram_w) AM_SHARE("fof_mid_tileram")
	AM_RANGE(0xb0c000, 0xb0ffff) AM_RAM_WRITE(fof_txt_tileram_w) AM_SHARE("fof_txt_tileram")

	AM_RANGE(0xc00000, 0xc00fff) AM_READONLY
	AM_RANGE(0xc00000, 0xc03fff) AM_DEVWRITE("palette", palette_device, write) AM_SHARE("palette")

	AM_RANGE(0xd00000, 0xd007ff) AM_RAM AM_SHARE("spriteram")

	AM_RANGE(0xe00000, 0xe0ffff) AM_RAM
ADDRESS_MAP_END


/* 7810 (?) sound cpu */

static ADDRESS_MAP_START( snd_mem, AS_PROGRAM, 8, fitfight_state )
	AM_RANGE(0x0000, 0x3fff) AM_ROM
	AM_RANGE(0x4000, 0x7fff) AM_ROMBANK("bank1")    /* ??? External ROM */
	AM_RANGE(0x8000, 0x87ff) AM_RAM
	AM_RANGE(0xff00, 0xffff) AM_RAM
ADDRESS_MAP_END

READ8_MEMBER(fitfight_state::snd_porta_r)
{
	//osd_printf_debug("PA R @%x\n",space.device().safe_pc());
	return machine().rand();
}

READ8_MEMBER(fitfight_state::snd_portb_r)
{
	//osd_printf_debug("PB R @%x\n",space.device().safe_pc());
	return machine().rand();
}

READ8_MEMBER(fitfight_state::snd_portc_r)
{
	//osd_printf_debug("PC R @%x\n",space.device().safe_pc());
	return machine().rand();
}

WRITE8_MEMBER(fitfight_state::snd_porta_w)
{
	//osd_printf_debug("PA W %x @%x\n",data,space.device().safe_pc());
}

WRITE8_MEMBER(fitfight_state::snd_portb_w)
{
	//osd_printf_debug("PB W %x @%x\n",data,space.device().safe_pc());
}

WRITE8_MEMBER(fitfight_state::snd_portc_w)
{
	//osd_printf_debug("PC W %x @%x\n",data,space.device().safe_pc());
}

static ADDRESS_MAP_START( snd_io, AS_IO, 8, fitfight_state )
		AM_RANGE(UPD7810_PORTA, UPD7810_PORTA) AM_READ(snd_porta_r) AM_WRITE(snd_porta_w)
		AM_RANGE(UPD7810_PORTB, UPD7810_PORTB) AM_READ(snd_portb_r) AM_WRITE(snd_portb_w)
		AM_RANGE(UPD7810_PORTC, UPD7810_PORTC) AM_READ(snd_portc_r) AM_WRITE(snd_portc_w)
ADDRESS_MAP_END

INTERRUPT_GEN_MEMBER(fitfight_state::snd_irq)
{
	generic_pulse_irq_line(device.execute(), UPD7810_INTF2, 1);
}


// #define PRIORITY_EASINESS_TO_PLAY

/* I've put the inputs the same way they can be read in the "test mode" */

static INPUT_PORTS_START( fitfight )
	PORT_START("P1_P2") // 0x200000.w
	/* players inputs -> 0xe022cc.w */
	PORT_BIT(  0x0001, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_PLAYER(1)
	PORT_BIT(  0x0002, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_PLAYER(1)
	PORT_BIT(  0x0004, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_PLAYER(1)
	PORT_BIT(  0x0008, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_PLAYER(1)
	PORT_BIT(  0x0010, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(1)
	PORT_BIT(  0x0020, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(1)
	PORT_BIT(  0x0040, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_PLAYER(1)
	PORT_BIT(  0x0080, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT(  0x0100, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_PLAYER(2)
	PORT_BIT(  0x0200, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_PLAYER(2)
	PORT_BIT(  0x0400, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_PLAYER(2)
	PORT_BIT(  0x0800, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_PLAYER(2)
	PORT_BIT(  0x1000, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(2)
	PORT_BIT(  0x2000, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(2)
	PORT_BIT(  0x4000, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_PLAYER(2)
	PORT_BIT(  0x8000, IP_ACTIVE_LOW, IPT_START2 )

	PORT_START("EXTRA") // 0x300000.w (unused)
	PORT_BIT(  0xffff, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("SYSTEM_DSW2")   // 0x400000.w
	/* LSB : system inputs -> 0xe022cf.b */
	PORT_BIT(  0x0001, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT(  0x0002, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_BIT(  0x0004, IP_ACTIVE_LOW, IPT_COIN3 )
	PORT_BIT(  0x0008, IP_ACTIVE_LOW, IPT_COIN4 )
	PORT_BIT(  0x0010, IP_ACTIVE_LOW, IPT_UNKNOWN )         // "Test"
	PORT_BIT(  0x0020, IP_ACTIVE_LOW, IPT_SERVICE1 )
	PORT_BIT(  0x0040, IP_ACTIVE_LOW, IPT_UNKNOWN )         // "Fault" (= "Tilt" ?)
	PORT_BIT(  0x0080, IP_ACTIVE_LOW, IPT_UNKNOWN )
	/* MSB : SW2 -> 0xe04c26.b (cpl) */
	PORT_DIPNAME( 0xf800, 0xf800, "Time" ) PORT_DIPLOCATION("SW2:5,4,3,2,1")
	PORT_DIPSETTING(      0xf000, "02" )
	PORT_DIPSETTING(      0xe800, "05" )
	PORT_DIPSETTING(      0xe000, "08" )
	PORT_DIPSETTING(      0xd800, "11" )
	PORT_DIPSETTING(      0xd000, "14" )
	PORT_DIPSETTING(      0xc800, "17" )
	PORT_DIPSETTING(      0xc000, "20" )
	PORT_DIPSETTING(      0xb800, "23" )
	PORT_DIPSETTING(      0xb000, "26" )
	PORT_DIPSETTING(      0xa800, "29" )
	PORT_DIPSETTING(      0xa000, "32" )
	PORT_DIPSETTING(      0x9800, "35" )
	PORT_DIPSETTING(      0x9000, "38" )
	PORT_DIPSETTING(      0x8800, "41" )
	PORT_DIPSETTING(      0x8000, "44" )
	PORT_DIPSETTING(      0x7800, "47" )
	PORT_DIPSETTING(      0x7000, "50" )
	PORT_DIPSETTING(      0x6800, "53" )
	PORT_DIPSETTING(      0x6000, "56" )
	PORT_DIPSETTING(      0x5800, "59" )
	PORT_DIPSETTING(      0x5000, "62" )
	PORT_DIPSETTING(      0x4800, "65" )
	PORT_DIPSETTING(      0x4000, "68" )
	PORT_DIPSETTING(      0x3800, "71" )
	PORT_DIPSETTING(      0x3000, "74" )
	PORT_DIPSETTING(      0x2800, "77" )
	PORT_DIPSETTING(      0x2000, "80" )
	PORT_DIPSETTING(      0x1800, "83" )
	PORT_DIPSETTING(      0x1000, "86" )
	PORT_DIPSETTING(      0x0800, "89" )
	PORT_DIPSETTING(      0x0000, "92" )
	PORT_DIPSETTING(      0xf800, "99" )
	PORT_DIPNAME( 0x0700, 0x0700, "First Credit" ) PORT_DIPLOCATION("SW2:8,7,6")
	PORT_DIPSETTING(      0x0000, DEF_STR( 8C_1C ) )
	PORT_DIPSETTING(      0x0100, DEF_STR( 7C_1C ) )
	PORT_DIPSETTING(      0x0200, DEF_STR( 6C_1C ) )
	PORT_DIPSETTING(      0x0300, DEF_STR( 5C_1C ) )
	PORT_DIPSETTING(      0x0400, DEF_STR( 4C_1C ) )
	PORT_DIPSETTING(      0x0500, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(      0x0600, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(      0x0700, DEF_STR( 1C_1C ) )

	PORT_START("DSW3_DSW1") // 0x500000.w
	/* MSB : SW3 -> 0xe04c24.b (cpl) */
	PORT_DIPNAME( 0xe000, 0xe000, "Next Credit" ) PORT_DIPLOCATION("SW3:3,2,1")
	PORT_DIPSETTING(      0x0000, DEF_STR( 4C_1C ) )
	PORT_DIPSETTING(      0x4000, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(      0x2000, DEF_STR( 3C_2C ) )
	PORT_DIPSETTING(      0xe000, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(      0xc000, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(      0xa000, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(      0x8000, DEF_STR( 1C_4C ) )
	PORT_DIPSETTING(      0x6000, DEF_STR( 1C_6C ) )
	PORT_DIPNAME( 0x1c00, 0x1000, DEF_STR( Difficulty ) ) PORT_DIPLOCATION("SW3:6,5,4")
	PORT_DIPSETTING(      0x1c00, DEF_STR( Easiest ) )
	PORT_DIPSETTING(      0x1800, DEF_STR( Easier ) )
	PORT_DIPSETTING(      0x1400, DEF_STR( Easy ) )
	PORT_DIPSETTING(      0x1000, DEF_STR( Normal ) )
	PORT_DIPSETTING(      0x0c00, DEF_STR( Medium ) )
	PORT_DIPSETTING(      0x0800, DEF_STR( Hard ) )
	PORT_DIPSETTING(      0x0400, DEF_STR( Hard ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( Hardest ) )
	PORT_DIPNAME( 0x0200, 0x0000, DEF_STR( Free_Play ) ) PORT_DIPLOCATION("SW3:7")
	PORT_DIPSETTING(      0x0000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0200, DEF_STR( On ) )
	PORT_SERVICE( 0x0100, IP_ACTIVE_LOW )
	/* LSB : SW1 -> 0xe04c25.b (cpl) */
	PORT_DIPNAME( 0x0080, 0x0080, DEF_STR( Demo_Sounds ) ) PORT_DIPLOCATION("SW1:1")    // To be confirmed
	PORT_DIPSETTING(      0x0000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0080, DEF_STR( On ) )
	PORT_DIPNAME( 0x0070, 0x0060, "Needed Points/Maximum Rounds" ) PORT_DIPLOCATION("SW1:4,3,2")    // see notes
	PORT_DIPSETTING(      0x0070, "Endless" )
	PORT_DIPSETTING(      0x0060, "1/2" )
	PORT_DIPSETTING(      0x0050, "2/3" )
	PORT_DIPSETTING(      0x0040, "2/4" )
	PORT_DIPSETTING(      0x0030, "3/5" )
	PORT_DIPSETTING(      0x0020, "3/6" )
	PORT_DIPSETTING(      0x0010, "4/7" )
	PORT_DIPSETTING(      0x0000, "4/8" )
	PORT_DIPNAME( 0x0008, 0x0000, "Select All Players" ) PORT_DIPLOCATION("SW1:5")      // in a 1 player game
	PORT_DIPSETTING(      0x0008, DEF_STR( No ) )           // only Ryo and Robert available
	PORT_DIPSETTING(      0x0000, DEF_STR( Yes ) )
	PORT_DIPUNKNOWN_DIPLOC( 0x0004, 0x0004, "SW1:6" )
	PORT_DIPUNKNOWN_DIPLOC( 0x0002, 0x0002, "SW1:7" )       // must be Off during P.O.S.T. !
	PORT_DIPUNKNOWN_DIPLOC( 0x0001, 0x0001, "SW1:8" )
INPUT_PORTS_END

static INPUT_PORTS_START( histryma )
	PORT_START("P1_P2") // 0x200000.w
	/* players inputs -> 0xe02cf2.w and 0xe02cf8.w */
	PORT_BIT(  0x0001, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_PLAYER(1)
	PORT_BIT(  0x0002, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_PLAYER(1)
	PORT_BIT(  0x0004, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_PLAYER(1)
	PORT_BIT(  0x0008, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_PLAYER(1)
	PORT_BIT(  0x0010, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(1)
	PORT_BIT(  0x0020, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(1)
	PORT_BIT(  0x0040, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_PLAYER(1)
	PORT_BIT(  0x0080, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT(  0x0100, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_PLAYER(2)
	PORT_BIT(  0x0200, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_PLAYER(2)
	PORT_BIT(  0x0400, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_PLAYER(2)
	PORT_BIT(  0x0800, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_PLAYER(2)
	PORT_BIT(  0x1000, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(2)
	PORT_BIT(  0x2000, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(2)
	PORT_BIT(  0x4000, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_PLAYER(2)
	PORT_BIT(  0x8000, IP_ACTIVE_LOW, IPT_START2 )

	PORT_START("EXTRA") // 0x300000.w
	/* LSB : players extra inputs -> 0xe02cf5.b and 0xe02cfb.b */
	PORT_BIT(  0x0001, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT(  0x0002, IP_ACTIVE_LOW, IPT_BUTTON6 ) PORT_PLAYER(1)
	PORT_BIT(  0x0004, IP_ACTIVE_LOW, IPT_BUTTON5 ) PORT_PLAYER(1)
	PORT_BIT(  0x0008, IP_ACTIVE_LOW, IPT_BUTTON4 ) PORT_PLAYER(1)
	PORT_BIT(  0x0010, IP_ACTIVE_LOW, IPT_BUTTON4 ) PORT_PLAYER(2)
	PORT_BIT(  0x0020, IP_ACTIVE_LOW, IPT_BUTTON5 ) PORT_PLAYER(2)
	PORT_BIT(  0x0040, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT(  0x0080, IP_ACTIVE_LOW, IPT_UNKNOWN )
	/* MSB : unused */
	PORT_BIT(  0xff00, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("SYSTEM_DSW2")   // 0x400000.w
	/* LSB : system inputs -> 0xe02cf7.b and 0xe02cfd.b */
	PORT_BIT(  0x0001, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT(  0x0002, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_BIT(  0x0004, IP_ACTIVE_LOW, IPT_BUTTON6 ) PORT_PLAYER(2)
	PORT_BIT(  0x0008, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT(  0x0010, IP_ACTIVE_LOW, IPT_UNKNOWN )         // "Test"
	PORT_BIT(  0x0020, IP_ACTIVE_LOW, IPT_SERVICE1 )
	PORT_BIT(  0x0040, IP_ACTIVE_LOW, IPT_UNKNOWN )         // "Fault" (= "Tilt" ?)
	PORT_BIT(  0x0080, IP_ACTIVE_LOW, IPT_UNKNOWN )         // "Test" (duplicated)
	/* MSB : SW2 -> 0xe05874.b (cpl) */
	PORT_DIPNAME( 0xf800, 0x0000, "Time" ) PORT_DIPLOCATION("SW2:5,4,3,2,1")
	#ifndef PRIORITY_EASINESS_TO_PLAY
		PORT_DIPSETTING(      0xf800, "15" )                // duplicated setting
		PORT_DIPSETTING(      0xf000, "15" )                // duplicated setting
		PORT_DIPSETTING(      0xe800, "15" )                // duplicated setting
	#endif
	PORT_DIPSETTING(      0xe000, "15" )
	PORT_DIPSETTING(      0xd800, "18" )
	PORT_DIPSETTING(      0xd000, "21" )
	PORT_DIPSETTING(      0xc800, "24" )
	PORT_DIPSETTING(      0xc000, "27" )
	PORT_DIPSETTING(      0xb800, "30" )
	PORT_DIPSETTING(      0xb000, "33" )
	PORT_DIPSETTING(      0xa800, "36" )
	PORT_DIPSETTING(      0xa000, "39" )
	PORT_DIPSETTING(      0x9800, "42" )
	PORT_DIPSETTING(      0x9000, "45" )
	PORT_DIPSETTING(      0x8800, "48" )
	PORT_DIPSETTING(      0x8000, "51" )
	PORT_DIPSETTING(      0x7800, "54" )
	PORT_DIPSETTING(      0x7000, "57" )
	PORT_DIPSETTING(      0x6800, "60" )
	PORT_DIPSETTING(      0x6000, "63" )
	PORT_DIPSETTING(      0x5800, "66" )
	PORT_DIPSETTING(      0x5000, "69" )
	PORT_DIPSETTING(      0x4800, "72" )
	PORT_DIPSETTING(      0x4000, "75" )
	PORT_DIPSETTING(      0x3800, "78" )
	PORT_DIPSETTING(      0x3000, "81" )
	PORT_DIPSETTING(      0x2800, "84" )
	PORT_DIPSETTING(      0x2000, "87" )
	PORT_DIPSETTING(      0x1800, "90" )
	PORT_DIPSETTING(      0x1000, "93" )
	PORT_DIPSETTING(      0x0800, "96" )
	PORT_DIPSETTING(      0x0000, "99" )
	PORT_DIPNAME( 0x0700, 0x0700, DEF_STR( Coin_A ) ) PORT_DIPLOCATION("SW2:8,7,6")
	PORT_DIPSETTING(      0x0200, DEF_STR( 6C_1C ) )
	PORT_DIPSETTING(      0x0300, DEF_STR( 5C_1C ) )
	PORT_DIPSETTING(      0x0400, DEF_STR( 4C_1C ) )
	PORT_DIPSETTING(      0x0500, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(      0x0600, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(      0x0100, DEF_STR( 3C_2C ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( 4C_3C ) )
	PORT_DIPSETTING(      0x0700, DEF_STR( 1C_1C ) )

	PORT_START("DSW3_DSW1") // 0x500000.w
	/* MSB : SW3 -> 0xe05872.b (cpl) */
	PORT_DIPNAME( 0xe000, 0xe000, DEF_STR( Coin_B ) ) PORT_DIPLOCATION("SW3:3,2,1")
	PORT_DIPSETTING(      0xe000, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( 3C_4C ) )
	PORT_DIPSETTING(      0x2000, DEF_STR( 2C_3C ) )
	PORT_DIPSETTING(      0xc000, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(      0xa000, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(      0x8000, DEF_STR( 1C_4C ) )
	PORT_DIPSETTING(      0x6000, DEF_STR( 1C_5C ) )
	PORT_DIPSETTING(      0x4000, DEF_STR( 1C_6C ) )
	PORT_DIPNAME( 0x1c00, 0x1000, DEF_STR( Difficulty ) ) PORT_DIPLOCATION("SW3:6,5,4")
	PORT_DIPSETTING(      0x1c00, DEF_STR( Easiest ) )
	PORT_DIPSETTING(      0x1800, DEF_STR( Easier ) )
	PORT_DIPSETTING(      0x1400, DEF_STR( Easy ) )
	PORT_DIPSETTING(      0x1000, DEF_STR( Normal ) )
	PORT_DIPSETTING(      0x0c00, DEF_STR( Medium ) )
	PORT_DIPSETTING(      0x0800, DEF_STR( Hard ) )
	PORT_DIPSETTING(      0x0400, DEF_STR( Harder ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( Hardest ) )
	PORT_DIPNAME( 0x0200, 0x0000, DEF_STR( Free_Play ) ) PORT_DIPLOCATION("SW3:7")
	PORT_DIPSETTING(      0x0000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0200, DEF_STR( On ) )
	PORT_SERVICE_DIPLOC( 0x0100, IP_ACTIVE_LOW, "SW3:8" )
	/* LSB : SW1 -> 0xe05873.b (cpl) */
	PORT_DIPNAME( 0x0080, 0x0080, DEF_STR( Demo_Sounds ) ) PORT_DIPLOCATION("SW1:1")    // To be confirmed
	PORT_DIPSETTING(      0x0000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0080, DEF_STR( On ) )
	PORT_DIPNAME( 0x0070, 0x0060, "Needed Points/Maximum Points" ) PORT_DIPLOCATION("SW1:4,3,2")    // see notes
	PORT_DIPSETTING(      0x0070, "Endless" )               // ends on a draw
	PORT_DIPSETTING(      0x0060, "1/2" )
	PORT_DIPSETTING(      0x0050, "2/3" )
	PORT_DIPSETTING(      0x0040, "2/4" )
	PORT_DIPSETTING(      0x0030, "3/5" )
	PORT_DIPSETTING(      0x0020, "3/6" )
	PORT_DIPSETTING(      0x0010, "4/7" )
	PORT_DIPSETTING(      0x0000, "4/8" )
	PORT_DIPNAME( 0x0008, 0x0000, "Buttons" ) PORT_DIPLOCATION("SW1:5") // 3 or 6 buttons as default ?
	PORT_DIPSETTING(      0x0008, "3" )
	PORT_DIPSETTING(      0x0000, "6" )
	PORT_DIPUNKNOWN_DIPLOC( 0x0004, 0x0004, "SW1:6" )
	PORT_DIPUNKNOWN_DIPLOC( 0x0002, 0x0002, "SW1:7" )       // must be Off during P.O.S.T. !
	PORT_DIPUNKNOWN_DIPLOC( 0x0001, 0x0001, "SW1:8" )
INPUT_PORTS_END

/* Check inputs again when video emulation is better */
/* Surprisingly, the Dip Switches look very similar to the ones from 'histryma'
   (the only difference being that there is no "Needed Points/Maximum Points"
   Dip Switch, the value always being set to "2/3") */
static INPUT_PORTS_START( bbprot )
	PORT_START("P1_P2") // 0x300000.w
	/* players inputs -> 0xe0545e.w and 0xe05464.w */
	PORT_BIT(  0x0001, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_PLAYER(1)
	PORT_BIT(  0x0002, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_PLAYER(1)
	PORT_BIT(  0x0004, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_PLAYER(1)
	PORT_BIT(  0x0008, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_PLAYER(1)
	PORT_BIT(  0x0010, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(1)
	PORT_BIT(  0x0020, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(1)
	PORT_BIT(  0x0040, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_PLAYER(1)
	PORT_BIT(  0x0080, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT(  0x0100, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_PLAYER(2)
	PORT_BIT(  0x0200, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_PLAYER(2)
	PORT_BIT(  0x0400, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_PLAYER(2)
	PORT_BIT(  0x0800, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_PLAYER(2)
	PORT_BIT(  0x1000, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(2)
	PORT_BIT(  0x2000, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(2)
	PORT_BIT(  0x4000, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_PLAYER(2)
	PORT_BIT(  0x8000, IP_ACTIVE_LOW, IPT_START2 )

	PORT_START("EXTRA") // 0x380000.w
	/* LSB : players extra inputs -> 0xe05461.b and 0xe05467.b */
	PORT_BIT(  0x0001, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT(  0x0002, IP_ACTIVE_LOW, IPT_BUTTON6 ) PORT_PLAYER(1)
	PORT_BIT(  0x0004, IP_ACTIVE_LOW, IPT_BUTTON5 ) PORT_PLAYER(1)
	PORT_BIT(  0x0008, IP_ACTIVE_LOW, IPT_BUTTON4 ) PORT_PLAYER(1)
	PORT_BIT(  0x0010, IP_ACTIVE_LOW, IPT_BUTTON4 ) PORT_PLAYER(2)
	PORT_BIT(  0x0020, IP_ACTIVE_LOW, IPT_BUTTON5 ) PORT_PLAYER(2)
	PORT_BIT(  0x0040, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT(  0x0080, IP_ACTIVE_LOW, IPT_UNKNOWN )
	/* MSB : unused */
	PORT_BIT(  0xff00, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("SYSTEM_DSW2")   // 0x400000.w
	/* LSB : system inputs -> 0xe05463.b and 0xe05469.b */
	PORT_BIT(  0x0001, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT(  0x0002, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_BIT(  0x0004, IP_ACTIVE_LOW, IPT_BUTTON6 ) PORT_PLAYER(2)
	PORT_BIT(  0x0008, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT(  0x0010, IP_ACTIVE_LOW, IPT_UNKNOWN )         // "Test"
	PORT_BIT(  0x0020, IP_ACTIVE_LOW, IPT_SERVICE1 )
	PORT_BIT(  0x0040, IP_ACTIVE_LOW, IPT_UNKNOWN )         // "Fault" (= "Tilt" ?)
	PORT_BIT(  0x0080, IP_ACTIVE_LOW, IPT_UNKNOWN )         // "Test" (duplicated)
	/* MSB : SW2 -> 0xe07e84.b (cpl) */
	PORT_DIPNAME( 0xf800, 0x0000, "Time" ) PORT_DIPLOCATION("SW2:5,4,3,2,1")
	#ifndef PRIORITY_EASINESS_TO_PLAY
		PORT_DIPSETTING(      0xf800, "15" )                // duplicated setting
		PORT_DIPSETTING(      0xf000, "15" )                // duplicated setting
		PORT_DIPSETTING(      0xe800, "15" )                // duplicated setting
	#endif
	PORT_DIPSETTING(      0xe000, "15" )
	PORT_DIPSETTING(      0xd800, "18" )
	PORT_DIPSETTING(      0xd000, "21" )
	PORT_DIPSETTING(      0xc800, "24" )
	PORT_DIPSETTING(      0xc000, "27" )
	PORT_DIPSETTING(      0xb800, "30" )
	PORT_DIPSETTING(      0xb000, "33" )
	PORT_DIPSETTING(      0xa800, "36" )
	PORT_DIPSETTING(      0xa000, "39" )
	PORT_DIPSETTING(      0x9800, "42" )
	PORT_DIPSETTING(      0x9000, "45" )
	PORT_DIPSETTING(      0x8800, "48" )
	PORT_DIPSETTING(      0x8000, "51" )
	PORT_DIPSETTING(      0x7800, "54" )
	PORT_DIPSETTING(      0x7000, "57" )
	PORT_DIPSETTING(      0x6800, "60" )
	PORT_DIPSETTING(      0x6000, "63" )
	PORT_DIPSETTING(      0x5800, "66" )
	PORT_DIPSETTING(      0x5000, "69" )
	PORT_DIPSETTING(      0x4800, "72" )
	PORT_DIPSETTING(      0x4000, "75" )
	PORT_DIPSETTING(      0x3800, "78" )
	PORT_DIPSETTING(      0x3000, "81" )
	PORT_DIPSETTING(      0x2800, "84" )
	PORT_DIPSETTING(      0x2000, "87" )
	PORT_DIPSETTING(      0x1800, "90" )
	PORT_DIPSETTING(      0x1000, "93" )
	PORT_DIPSETTING(      0x0800, "96" )
	PORT_DIPSETTING(      0x0000, "99" )
	PORT_DIPNAME( 0x0700, 0x0700, DEF_STR( Coin_A ) ) PORT_DIPLOCATION("SW2:8,7,6")
	PORT_DIPSETTING(      0x0200, DEF_STR( 6C_1C ) )
	PORT_DIPSETTING(      0x0300, DEF_STR( 5C_1C ) )
	PORT_DIPSETTING(      0x0400, DEF_STR( 4C_1C ) )
	PORT_DIPSETTING(      0x0500, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(      0x0600, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(      0x0100, DEF_STR( 3C_2C ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( 4C_3C ) )
	PORT_DIPSETTING(      0x0700, DEF_STR( 1C_1C ) )

	PORT_START("DSW3_DSW1") // 0x480000.w
	/* MSB : SW3 -> 0xe07e82.b (cpl) */
	PORT_DIPNAME( 0xe000, 0xe000, DEF_STR( Coin_B ) ) PORT_DIPLOCATION("SW3:3,2,1")
	PORT_DIPSETTING(      0xe000, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( 3C_4C ) )
	PORT_DIPSETTING(      0x2000, DEF_STR( 2C_3C ) )
	PORT_DIPSETTING(      0xc000, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(      0xa000, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(      0x8000, DEF_STR( 1C_4C ) )
	PORT_DIPSETTING(      0x6000, DEF_STR( 1C_5C ) )
	PORT_DIPSETTING(      0x4000, DEF_STR( 1C_6C ) )
	PORT_DIPNAME( 0x1c00, 0x1000, DEF_STR( Difficulty ) ) PORT_DIPLOCATION("SW3:6,5,4")
	PORT_DIPSETTING(      0x1c00, DEF_STR( Easiest ) )
	PORT_DIPSETTING(      0x1800, DEF_STR( Easier ) )
	PORT_DIPSETTING(      0x1400, DEF_STR( Easy ) )
	PORT_DIPSETTING(      0x1000, DEF_STR( Normal ) )
	PORT_DIPSETTING(      0x0c00, DEF_STR( Medium ) )
	PORT_DIPSETTING(      0x0800, DEF_STR( Hard ) )
	PORT_DIPSETTING(      0x0400, DEF_STR( Hard ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( Hardest ) )
	PORT_DIPNAME( 0x0200, 0x0000, DEF_STR( Free_Play ) ) PORT_DIPLOCATION("SW3:7")
	PORT_DIPSETTING(      0x0000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0200, DEF_STR( On ) )
	PORT_SERVICE_DIPLOC( 0x0100, IP_ACTIVE_LOW, "SW3:8" )
	/* LSB : SW1 -> 0xe07e83.b (cpl) */
	PORT_DIPNAME( 0x0080, 0x0080, DEF_STR( Demo_Sounds ) ) PORT_DIPLOCATION("SW1:1")    // To be confirmed
	PORT_DIPSETTING(      0x0000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0080, DEF_STR( On ) )
	PORT_DIPUNKNOWN_DIPLOC( 0x0040, 0x0040, "SW1:2" )
	PORT_DIPUNKNOWN_DIPLOC( 0x0020, 0x0020, "SW1:3" )
	PORT_DIPUNKNOWN_DIPLOC( 0x0010, 0x0010, "SW1:4" )
	PORT_DIPNAME( 0x0008, 0x0000, "Buttons" ) PORT_DIPLOCATION("SW1:5") // 3 or 6 buttons as default ?
	PORT_DIPSETTING(      0x0008, "3" )
	PORT_DIPSETTING(      0x0000, "6" )
	PORT_DIPUNKNOWN_DIPLOC( 0x0004, 0x0004, "SW1:6" )
	PORT_DIPUNKNOWN_DIPLOC( 0x0002, 0x0002, "SW1:7" )       // must be Off during P.O.S.T. !
	PORT_DIPUNKNOWN_DIPLOC( 0x0001, 0x0001, "SW1:8" )
INPUT_PORTS_END


static const gfx_layout fof_tile_layout =
{
	8,8,
	RGN_FRAC(1,4),
	4,
	{ 0,RGN_FRAC(1,4),RGN_FRAC(2,4),RGN_FRAC(3,4) },
	{ 0,1,2,3,4,5,6,7 },
	{ 0*8, 1*8, 2*8, 3*8, 4*8, 5*8, 6*8, 7*8 },
	8*8
};


static const gfx_layout fof_sprite_layout =
{
	16,16,
	RGN_FRAC(1,4),
	4,
	{ 0,RGN_FRAC(1,4),RGN_FRAC(2,4),RGN_FRAC(3,4) },
	{ 0,1,2,3,4,5,6,7,8*8+0,8*8+1,8*8+2,8*8+3,8*8+4,8*8+5,8*8+6,8*8+7 },
	{ 0*8, 1*8, 2*8, 3*8, 4*8, 5*8, 6*8, 7*8,
	16*8+0*8,16*8+1*8,16*8+2*8,16*8+3*8,16*8+4*8,16*8+5*8,16*8+6*8,16*8+7*8

	},
	16*16
};

static const gfx_layout bbprot_sprite_layout =
{
	16,16,
	RGN_FRAC(1,5),
	5,
	{ 0,RGN_FRAC(1,5),RGN_FRAC(2,5),RGN_FRAC(3,5),RGN_FRAC(4,5) },
	{ 0,1,2,3,4,5,6,7,8*8+0,8*8+1,8*8+2,8*8+3,8*8+4,8*8+5,8*8+6,8*8+7 },
	{ 0*8, 1*8, 2*8, 3*8, 4*8, 5*8, 6*8, 7*8,
	16*8+0*8,16*8+1*8,16*8+2*8,16*8+3*8,16*8+4*8,16*8+5*8,16*8+6*8,16*8+7*8

	},
	16*16
};

static GFXDECODE_START( fitfight )
	GFXDECODE_ENTRY( "gfx1", 0, fof_tile_layout,   0x000, 256  ) /* tx tiles */
	GFXDECODE_ENTRY( "gfx1", 0, fof_tile_layout,   0x200, 256  ) /* mid tiles */
	GFXDECODE_ENTRY( "gfx1", 0, fof_tile_layout,   0x400, 256  ) /* bg tiles */
	GFXDECODE_ENTRY( "gfx2", 0, fof_sprite_layout, 0x600, 256  ) /* sprites */

GFXDECODE_END

static GFXDECODE_START( prot )
	GFXDECODE_ENTRY( "gfx1", 0, fof_tile_layout,     0x0000, 256  ) /* tx tiles */
	GFXDECODE_ENTRY( "gfx1", 0, fof_tile_layout,     0x0800, 256  ) /* mid tiles */
	GFXDECODE_ENTRY( "gfx1", 0, fof_tile_layout,     0x1000, 256  ) /* bg tiles */
	GFXDECODE_ENTRY( "gfx2", 0, bbprot_sprite_layout,0x1800, 256  ) /* sprites */

GFXDECODE_END


void fitfight_state::machine_start()
{
	save_item(NAME(m_fof_700000_data));
}

void fitfight_state::machine_reset()
{
	m_fof_700000_data = 0;
}

static MACHINE_CONFIG_START( fitfight, fitfight_state )

	MCFG_CPU_ADD("maincpu",M68000, 12000000)
	MCFG_CPU_PROGRAM_MAP(fitfight_main_map)
	MCFG_CPU_VBLANK_INT_DRIVER("screen", fitfight_state,  irq2_line_hold)

	MCFG_CPU_ADD("audiocpu", UPD7810, 12000000)
	MCFG_CPU_PROGRAM_MAP(snd_mem)
	MCFG_CPU_IO_MAP(snd_io)
	MCFG_CPU_VBLANK_INT_DRIVER("screen", fitfight_state,  snd_irq)


	MCFG_GFXDECODE_ADD("gfxdecode", "palette", fitfight)

	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_REFRESH_RATE(60)
	MCFG_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(0))
	MCFG_SCREEN_SIZE(40*8, 32*8)
	MCFG_SCREEN_VISIBLE_AREA(2*8, 39*8-1, 2*8, 30*8-1)
	MCFG_SCREEN_UPDATE_DRIVER(fitfight_state, screen_update_fitfight)
	MCFG_SCREEN_PALETTE("palette")

	MCFG_PALETTE_ADD("palette", 0x800)
	MCFG_PALETTE_FORMAT(xRRRRRGGGGGBBBBB)


	MCFG_SPEAKER_STANDARD_MONO("mono")

	MCFG_OKIM6295_ADD("oki", 1333333, OKIM6295_PIN7_LOW) // ~8080Hz ??? TODO: find out the real frequency
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 1.0)
MACHINE_CONFIG_END

static MACHINE_CONFIG_START( bbprot, fitfight_state )

	MCFG_CPU_ADD("maincpu",M68000, 12000000)
	MCFG_CPU_PROGRAM_MAP(bbprot_main_map)
	MCFG_CPU_VBLANK_INT_DRIVER("screen", fitfight_state,  irq2_line_hold)


	MCFG_GFXDECODE_ADD("gfxdecode", "palette", prot)

	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_REFRESH_RATE(60)
	MCFG_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(0))
	MCFG_SCREEN_SIZE(40*8, 32*8)
	MCFG_SCREEN_VISIBLE_AREA(2*8, 39*8-1, 2*8, 30*8-1)
	MCFG_SCREEN_UPDATE_DRIVER(fitfight_state, screen_update_fitfight)
	MCFG_SCREEN_PALETTE("palette")

	MCFG_PALETTE_ADD("palette", 0x2000)
	MCFG_PALETTE_FORMAT(xRRRRRGGGGGBBBBB)


	MCFG_SPEAKER_STANDARD_MONO("mono")

	MCFG_OKIM6295_ADD("oki", 1333333, OKIM6295_PIN7_LOW) // ~8080Hz ??? TODO: find out the real frequency
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 1.0)
MACHINE_CONFIG_END

/***

Here's the info about this dump:

Name:            Fit of Fighting
Manufacturer:    Unknow (There are chances that was produced by NIX, but it's not                          possible to verify)
Year:            Unknow
Date Dumped:     16-07-2002 (DD-MM-YYYY)

CPU:             68000, possibly at 12mhz
SOUND:           OKIM6295
GFX:             Unknown

About the game:

This game is a very horrible Art of Fighting rip-off, ripped graphics but
reprogrammed from 0, FM music, no zooms, no damage in the fighters faces, poor IA,
poor gameplay (you have to execute the special attacks very slowly to get them
running!), but incredibly fun to see such a thing :) Hope you enjoy it!

***/

ROM_START( fitfight )
	ROM_REGION( 0x100000, "maincpu", 0 ) /* 68000 Code */
	ROM_LOAD16_BYTE( "u138_ff1.bin", 0x000001, 0x080000, CRC(165600fe) SHA1(b1987dbf34abdb6d08bdf7f71b256b62125e6517) )
	ROM_LOAD16_BYTE( "u125_ff1.bin", 0x000000, 0x080000, CRC(2f9bdb66) SHA1(4c1ade349f1219d448453b27d4a7517966912ffa) )

	ROM_REGION( 0x01c000, "audiocpu", 0 ) /* Sound Program */
	ROM_LOAD( "u23_ff1.bin",  0x000000, 0x004000, CRC(e2d6d768) SHA1(233e5501ffda8db48341fa66f16b630544803a89) )
	ROM_CONTINUE(          0x010000, 0x00c000 )

	ROM_REGION( 0x100000, "oki", 0 ) /* OKI Samples? */
	ROM_LOAD( "h7e_ff1.bin",  0x000000, 0x080000, CRC(3e12dfd8) SHA1(8f21abfc6a6aac9ad3fafe97d0279739c7b9fab9) ) //seems to be a merge of 2 0x040000 roms
	ROM_LOAD( "h18e_ff1.bin", 0x080000, 0x080000, CRC(a7f36dbe) SHA1(206efb7f32d6123ed3e22790ff38dd0a8e1626d7) ) //seems to be a merge of 2 0x040000 roms

	ROM_REGION( 0x100000, "gfx1", 0 ) /* GFX */
	ROM_LOAD( "p1_ff1.bin",   0x0c0000, 0x040000, CRC(542593b3) SHA1(068d9b5dc98a8353462705c64d2d287f270510a9) )
	ROM_LOAD( "p2_ff1.bin",   0x080000, 0x040000, CRC(fc517470) SHA1(45f33de393a89301051ec865ba665ad3366e29f7) )
	ROM_LOAD( "p4_ff1.bin",   0x040000, 0x040000, CRC(a8754268) SHA1(c03ea06ba79ff799399d17dc0eb86f5a7e2e3f8e) )
	ROM_LOAD( "p8_ff1.bin",   0x000000, 0x040000, CRC(bd55182a) SHA1(5253565fc2b73c70d9cbc8dbc9b0a201b21efa91) )

	ROM_REGION( 0x400000, "gfx2", 0 ) /* Sprites */
	ROM_LOAD( "s1_ff1.bin",   0x300000, 0x080000, CRC(90a57445) SHA1(44a88de1377d685c2bc185187b5ba151d900792d) )
	ROM_LOAD( "s1_ff1h.bin",  0x380000, 0x080000, CRC(07e23f95) SHA1(c2af437199f352264e5238547f9d14639cb6f329) )

	ROM_LOAD( "s2_ff1.bin",   0x200000, 0x080000, CRC(ee4a972b) SHA1(eb9c30691f1620bb1777ca5b911b11172bbadc53) )
	ROM_LOAD( "s2_ff1h.bin",  0x280000, 0x080000, CRC(726add2e) SHA1(404c62b4c60daaaace78641dd77a101fecfef1ad) )

	ROM_LOAD( "s4_ff1.bin",   0x100000, 0x080000, CRC(cfdcbdfb) SHA1(4305a67441cbbddeb214a5140446a9b8d04940ff) )
	ROM_LOAD( "s4_ff1h.bin",  0x180000, 0x080000, CRC(eecce2d7) SHA1(575e389c51c1528e2245db8c79fff6bc4d17a87d) )

	ROM_LOAD( "s8_ff1.bin",   0x000000, 0x080000, CRC(0edf5706) SHA1(481bcc031ea9489c14925510b0d567f858428783) )
	ROM_LOAD( "s8_ff1h.bin",  0x080000, 0x080000, CRC(1d00074f) SHA1(d5c6963aee5c47a77a097b0b1e254b1f1bc69a73) )
ROM_END

/***

Name:            The History of Martial Arts
Manufacturer:    Unknow, maybe NIX / Novatecnia
Year:            Unknow
Date Dumped:     18-07-2002 (DD-MM-YYYY)

CPU:             68000
SOUND:           OKIM6295
GFX:             Unknown

Country:         Maybe Spain

About the game:

This is a Karnov's revenge ripp off like Fit of Fighting with Art of Fighting.
Same GFX, but reprogrammed from 0, and with FM music... Another nice bootleg!
It was dumped from a faulty board, wich doesn't boot, but with intact eproms :)

***/

ROM_START( histryma )
	ROM_REGION( 0x100000, "maincpu", 0 ) /* 68000 Code */
	ROM_LOAD16_BYTE( "l_th.bin", 0x000001, 0x080000, CRC(5af9356a) SHA1(f3d797dcc528a3a2a4f0ebbf07d59bd2cc868622) )
	ROM_LOAD16_BYTE( "r_th.bin", 0x000000, 0x080000, CRC(1a44b504) SHA1(621d95b67d413da3e8a90c0cde494b2529b92407) )

	ROM_REGION( 0x01c000, "audiocpu", 0 ) /* Sound Program */
	ROM_LOAD( "y61f.bin",  0x000000, 0x004000, CRC(b588525a) SHA1(b768bd75d6351430f9656289146119e9c0308554) )
	ROM_CONTINUE(          0x010000, 0x00c000 )

	ROM_REGION( 0x100000, "oki", 0 ) /* OKI Samples? */
	ROM_LOAD( "u7_th.bin",  0x000000, 0x080000, CRC(88b41ef5) SHA1(565e2c4554dde79cd2da8b8a181b3378818223cc) ) //seems to be a merge of 2 0x040000 roms
	ROM_LOAD( "u18_th.bin", 0x080000, 0x080000, CRC(a734cd77) SHA1(3fe4cba6f6d691dfc4775de634e6e39bf4bb08b8) ) //seems to be a merge of 2 0x040000 roms

	ROM_REGION( 0x100000, "gfx1", 0 ) /* GFX */
	ROM_LOAD( "p1_th.bin",   0x0c0000, 0x040000, CRC(501c5336) SHA1(1743221d73d59ba40ddad3f69bc4aa1a51c29962) )
	ROM_LOAD( "p2_th.bin",   0x080000, 0x040000, CRC(f50666c7) SHA1(46856e70426388c9704eed94d4dbbbe5674b9be6) )
	ROM_LOAD( "p4_th.bin",   0x040000, 0x040000, CRC(c70223cf) SHA1(d3ee1b73a22a0aaab909141b32696c6b48f2b7ee) )
	ROM_LOAD( "p8_th.bin",   0x000000, 0x040000, CRC(8104b963) SHA1(ed02c3be16b8e94d9316391fe0f9fcf39679c1de) )

	ROM_REGION( 0x400000, "gfx2", 0 ) /* Sprites */
	ROM_LOAD( "s1_th.bin",   0x300000, 0x080000, CRC(e9c2d27a) SHA1(f47cc318f4a3db84858b4d302be4063f3bb1b071) )
	ROM_LOAD( "s1h_th.bin",  0x380000, 0x080000, CRC(d806c92e) SHA1(4781dd08755cd91f3d09ff418a9d13744d888922) )

	ROM_LOAD( "s2_th.bin",   0x200000, 0x080000, CRC(fa011056) SHA1(8f896d248c74faae5f320e0ef1730681fb7af57b) )
	ROM_LOAD( "s2h_th.bin",  0x280000, 0x080000, CRC(ef5f2268) SHA1(d2f13e6a393256cba24a70f1f85c7aeec93d0c51) )

	ROM_LOAD( "s4_th.bin",   0x100000, 0x080000, CRC(fa80fdec) SHA1(590dd27d727d1910b5935dbcb8f958c435b6077a) )
	ROM_LOAD( "s4h_th.bin",  0x180000, 0x080000, CRC(0fd3b43e) SHA1(971e8813e3a3d7735a96f31d557c28b9bbcf91d1) )

	ROM_LOAD( "s8_th.bin",   0x000000, 0x080000, CRC(57fd170f) SHA1(2b8cc688de894bbb7d44f9458b3d012a64f79b20) )
	ROM_LOAD( "s8h_th.bin",  0x080000, 0x080000, CRC(cd7bd0de) SHA1(2cd0f41e2575e667aa971f2f5716694dee203ab3) )
ROM_END

/***

Here's the info about this dump:

Name:            "BB" (Protoype name in some of the EPROM stickers)
Manufacturer:    Unknow (There are chances that was produced by NIX, but it's not                          possible to verify, same as Fit of Fighting )
Year:            Unknow
Date Dumped:     17-07-2002 (DD-MM-YYYY)

CPU:             68000, possibly at 12mhz
SOUND:           OKIM6295 (It is present in the board, but it's not used,
                           this prototype game does not have sound)
GFX:             Unknown

About the game:

This is a prototype in VERY early stages of development. Maybe it was coded
to gain some experience to be able to make Fit of Fighting bootleg.  A lot of
missing/random graphics, no sound, no game itself, just jump, move, crouch... :)
Badly coded scroll, no screen when you power on the board (just start pushing first
button, coin and start to get into a game) or leave it some seconds to see a
dramatically lame fight)... Character selection just shows garbage, maybe stage
selection shows rubbish, and the game itself is all the time displaying a lot of
garbage.. But this is VERY nice! :) The most funny thing about this game is that
the three visible fighter characters can be based on some of the NIX workers, but
i really can't verify this :)

Some of the eproms seem to be two times inserted in different slots of the board,
but with a different date wrote on the stickers, those are the ??_DD_MM.bin files.

***/

ROM_START( bbprot )
	ROM_REGION( 0x100000, "maincpu", 0 ) /* 68000 Code */
	ROM_LOAD16_BYTE( "l_bb.bin", 0x000001, 0x080000, CRC(2b7b9a9a) SHA1(51088358814cc337af150526ac7fd6216c102299) )
	ROM_LOAD16_BYTE( "r_bb.bin", 0x000000, 0x080000, CRC(28480f3e) SHA1(b89533fd01781e1b83c98b0b61a77f554fbdb4f3) )

	ROM_REGION( 0x100000, "oki", ROMREGION_ERASEFF ) /* OKI Samples? */

	ROM_REGION( 0x200000, "gfx1", 0 ) /* BG GFX */
	ROM_LOAD( "p1_29_11.bin",   0x180000, 0x080000, CRC(e7da36c1) SHA1(c7ea40b57088145019c1be3e34ba9e94c60bef78) )
	ROM_LOAD( "p2_29_11.bin",   0x100000, 0x080000, CRC(0411e1aa) SHA1(99ae1a45848f8227899989334f3035222bd5da39) )
	ROM_LOAD( "p4_29_11.bin",   0x080000, 0x080000, CRC(885942bf) SHA1(d9bde8c12be7d02dde442873e0852c7c85478254) )
	ROM_LOAD( "p8_29_11.bin",   0x000000, 0x080000, CRC(44f94575) SHA1(90c1a97e70d312b4475ce2d333e110e027c377b9) )

	ROM_REGION( 0x780000, "gfx2", 0 ) /* Sprites */
	ROM_LOAD( "s1_21_12.bin",  0x600000, 0x080000, CRC(0b396256) SHA1(6e69641cd012e60618a68386342108d04e826e3c) )
	ROM_LOAD( "s1_x.bin",      0x700000, 0x080000, CRC(b95dfbb8) SHA1(b37452900b07bc13bc9d1702ef1ff737ff0f393c) )
	ROM_LOAD( "s1_h.bin",      0x680000, 0x080000, CRC(d20b6ac3) SHA1(8b6a5f1da47be456da528ddaaf18e2321261683e) )

	ROM_LOAD( "s2_21_12.bin",  0x480000, 0x080000, CRC(46e8b73c) SHA1(f4dc349a2c955659ae44cdaecb422255a71193dc) )
	ROM_LOAD( "s2_x.bin",      0x580000, 0x080000, CRC(c90a52e8) SHA1(60e99e2737efd9bc0df52ecafa097d943fb7b9d9) )
	ROM_LOAD( "s2_h.bin",      0x500000, 0x080000, CRC(7970be11) SHA1(443ae8208171dee26f5d5f0908f1f35609cd327c) )

	ROM_LOAD( "s4_21_12.bin",  0x300000, 0x080000, CRC(f46d47bd) SHA1(4b733e299baa1da3eb962cb04bab1c80453e9f3f) )
	ROM_LOAD( "s4_x.bin",      0x400000, 0x080000, CRC(0fe4325d) SHA1(3fed50df1c42288ce44e54ff869a2be323fff4a8) )
	ROM_LOAD( "s4_h.bin",      0x380000, 0x080000, CRC(32a0bbb2) SHA1(b6ec622257c80237c9c8c99d04f0a166de64ab55) )

	ROM_LOAD( "s8_21_12.bin",  0x180000, 0x080000, CRC(f810567c) SHA1(b5aba9e3a22437f93a4aeaee55967fc7bd28af4b) )
	ROM_LOAD( "s8_x.bin",      0x280000, 0x080000, CRC(6ec466ea) SHA1(2ba5a04d78242a3911b914247614b6ef7c1f6317) )
	ROM_LOAD( "s8_h.bin",      0x200000, 0x080000, CRC(a425cc5b) SHA1(3e195e0e90481799256ce2020a09288555bdefd1) )

	ROM_LOAD( "s16_21_1.bin",  0x000000, 0x080000, CRC(023c1e63) SHA1(4d082261c355bb010f6b829e7d08ba88d9b677fc) )
	ROM_LOAD( "s16_x_mc.bin",  0x100000, 0x080000, CRC(1ad5447f) SHA1(65f542c91358fe460b3ad3c7ef435505aa95b3e6) )
	ROM_LOAD( "s16_h_mc.bin",  0x080000, 0x080000, CRC(3b9091de) SHA1(b426dab6361f5d48519dc9de146e1b2270af6c8b) )
ROM_END

ROM_START( hotmindff )
	ROM_REGION( 0x100000, "maincpu", 0 ) /* 68000 Code */
	ROM_LOAD16_BYTE( "21.u138", 0x000001, 0x020000, CRC(00d9ba73) SHA1(5f33f7e9b9446cdc380e6236381d86c46f55ede7) )
	ROM_LOAD16_BYTE( "20.u125", 0x000000, 0x020000, CRC(8a7b2cc2) SHA1(031255cff4764dd61f5418be8090310193aa2e59) )

	ROM_REGION( 0x01c000, "audiocpu", 0 ) /* Sound Program */
	ROM_LOAD( "22.u23",  0x000000, 0x004000, CRC(ca0327a2) SHA1(b725f625860dae4723e0ed4a6c7c29a2b1a3d15f) )
	ROM_CONTINUE(          0x010000, 0x00c000 )

	ROM_REGION( 0x100000, "oki", 0 ) /* OKI Samples? */
	ROM_LOAD( "19.u18",  0x000000, 0x040000, CRC(296d71da) SHA1(49735012691a9e65c4da6132428f7a9149504fd3) )

	ROM_REGION( 0x100000, "gfx1", 0 ) /* GFX */
	ROM_LOAD( "29.bin",  0x000000, 0x020000, CRC(cd4cab01) SHA1(90165f57adbfd7d85eaccfe794d347ed795dfba1) )
	ROM_LOAD( "30.bin",  0x040000, 0x020000, CRC(33f991b5) SHA1(8fddb5a3c78e73eb2a741aa970ff2c650a847317) )
	ROM_LOAD( "25.bin",  0x080000, 0x020000, CRC(21e7c729) SHA1(bf2a89ab54ef362bfee9fdcb7623d0f092559793) )
	ROM_LOAD( "26.bin",  0x0c0000, 0x020000, CRC(5802e7d0) SHA1(6a6661c753b31492295813952fc559e5189aae9f) )

	ROM_REGION( 0x400000, "gfx2", 0 ) /* Sprites */
	ROM_LOAD( "23.bin",  0x000000, 0x020000, CRC(4d2c79ed) SHA1(df7004b11f5bcd51f7629fa1cc907976d9b8c8dc) )
	ROM_LOAD( "24.bin",  0x100000, 0x020000, CRC(f43618d0) SHA1(7c383582ebcab713f2d8e6b4c73fc0415465d62d) )
	ROM_LOAD( "27.bin",  0x200000, 0x020000, CRC(9cee7e50) SHA1(3e131b4acf35b58d511f7313425d002e939a7ff9) )
	ROM_LOAD( "28.bin",  0x300000, 0x020000, CRC(059c7bcf) SHA1(b9d8b1e1482baeede750a54749834c68c2d0cfef) )
ROM_END



/* INIT */

DRIVER_INIT_MEMBER(fitfight_state,fitfight)
{
//  UINT16 *mem16 = (UINT16 *)memregion("maincpu")->base();
//  mem16[0x0165B2/2] = 0x4e71; // for now so it boots
	m_maincpu->space(AS_PROGRAM).install_read_handler(0x700000, 0x700001, read16_delegate(FUNC(fitfight_state::fitfight_700000_r),this));
	m_bbprot_kludge = 0;
}

DRIVER_INIT_MEMBER(fitfight_state,histryma)
{
//  UINT16 *mem16 = (UINT16 *)memregion("maincpu")->base();
//  mem16[0x017FDC/2] = 0x4e71; // for now so it boots
	m_maincpu->space(AS_PROGRAM).install_read_handler(0x700000, 0x700001, read16_delegate(FUNC(fitfight_state::histryma_700000_r),this));
	m_bbprot_kludge = 0;
}

DRIVER_INIT_MEMBER(fitfight_state,bbprot)
{
	m_bbprot_kludge = 1;
}

DRIVER_INIT_MEMBER(fitfight_state,hotmindff)
{
	m_maincpu->space(AS_PROGRAM).install_read_handler(0x200000, 0x200001, 0, 0, read16_delegate(FUNC(fitfight_state::hotmindff_unk_r),this));
	DRIVER_INIT_CALL(fitfight);
}


/* GAME */

GAME( 199?, fitfight, 0, fitfight, fitfight, fitfight_state, fitfight, ROT0, "bootleg", "Fit of Fighting", MACHINE_IMPERFECT_GRAPHICS | MACHINE_NO_SOUND | MACHINE_SUPPORTS_SAVE )
GAME( 199?, histryma, 0, fitfight, histryma, fitfight_state, histryma, ROT0, "bootleg", "The History of Martial Arts", MACHINE_IMPERFECT_GRAPHICS | MACHINE_NO_SOUND | MACHINE_SUPPORTS_SAVE )
GAME( 199?, bbprot,   0, bbprot,   bbprot, fitfight_state,   bbprot,   ROT0, "<unknown>", "unknown fighting game 'BB' (prototype)", MACHINE_IS_INCOMPLETE | MACHINE_IMPERFECT_GRAPHICS | MACHINE_NO_SOUND | MACHINE_SUPPORTS_SAVE )
GAME( 199?, hotmindff,  hotmind, fitfight,   fitfight, fitfight_state,   hotmindff,   ROT0, "Playmark", "Hot Mind (Fit of Fighting hardware)", MACHINE_NOT_WORKING | MACHINE_NO_SOUND ) // need to fix scroll offsets + inputs
