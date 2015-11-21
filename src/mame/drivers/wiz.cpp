// license:BSD-3-Clause
// copyright-holders:Zsolt Vasvari
/***************************************************************************

  Seibu Stinger/Wiz hardware

  Driver by Zsolt Vasvari


XTAL: 18.432MHz??
2xZ80, 3xAY8910 (stinger/scion have 2xAY8910 + discrete sound)

These boards are similar to a Galaxian board in the way it handles scrolling
and sprites, but the similarities pretty much end there. The most notable
difference is that there are 2 independently scrollable playfields.


Wiz/Stinger/Scion/Kung-Fu Taikun memory map (preliminary)

Main CPU:

0000-BFFF  ROM
C000-C7FF  RAM
D000-D3FF  Video RAM (Foreground)
D400-D7FF  Color RAM (Foreground) (Wiz)
D800-D83F  Attributes RAM (Foreground)
D840-D85F  Sprite RAM 1
E000-E3FF  Video RAM (Background)
E400-E7FF  Color RAM (Background) (Wiz)
E800-E83F  Attributes RAM (Background)
E840-E85F  Sprite RAM 2

I/O read:
d400 Protection (Wiz)
f000 DIP SW#1
f008 DIP SW#2
f010 Input Port 1
f018 Input Port 2
f800 Watchdog

I/O write:
c800 Coin Counter A
c801 Coin Counter B
f000 Sprite bank select (Wiz)
f001 NMI enable
f002 \ Palette select
f003 /
f004 \ Character bank select
f005 /
f006 \ Flip screen
f007 /
f800 Sound Command write
f818 (?) Sound or Background color


Sound CPU:

0000-1FFF  ROM
2000-23FF  RAM

I/O read:
3000 Sound Command Read (Stinger/Scion)
7000 Sound Command Read (Wiz)

I/O write:
3000 NMI enable (Stinger/Scion)
4000 AY8910 Control Port #1 (Wiz)
4001 AY8910 Write Port #1   (Wiz)
5000 AY8910 Control Port #2
5001 AY8910 Write Port #2
6000 AY8910 Control Port #3
6001 AY8910 Write Port #3
7000 NMI enable (Wiz)


****************************************************************************

TODO:

- Verify sprite colors in stinger/scion
- Global palette is wrong in stinger/scion compared to pcb, or could it be
  due to gamma/hue?
- cpu/video/interrupt frequency measurements
- sprite-sprite priorities are not correct yet, eg:
  * stinger popcorn enemies at beginning of the game should have higher priority
    than that 'vacuum cleaner' enemy sphere,
  * the cloud in kungfut should have higher prio than player char
- Improve stinger/scion discrete sound, shot sound should include noise
- stinger/scion audiocpu jumps to lalaland after receiving a soundlatch of 0x90,
  basically resetting itself. I assume this is a game bug
- scion insert-coin sound sometimes repeats or is silent due to soundlatch timing,
  this could be a game bug as well
- Background noise in scion (but not scionc). Note that the sound program is
  almost identical, except for three patches affecting noise period, noise
  channel C enable and channel C volume. So it looks just like a bug in the
  original (weird), or some strange form of protection.
- Is wiz protection emulation complete?
- Wiz: the supplier of the screenshot says there still may be some wrong
  colors. Just before the break on Level 2 there is a crescent moon,
  the background should probably be black.


****************************************************************************

2002-Nov-30 Kung-Fu Taikun added
  2xZ80 , 3x AY8910
  (DSW 1 , bit 2 )
  "THE MICROPHONE IS OUT OF CONTROL, SO THIS GAME DEPENDS ON THE BUTTONS"
  There's no additional hw or connectors on the pcb
  (except for small (bit 0 - ON, bit 1 - ON)  DSW near AY chips )
  Tomasz Slanina -  dox@space.pl

Notes:
  The microphone is for summoning clouds. The game falls back to use
  buttons if it's not functioning.


****************************************************************************

Stephh's notes (based on the games Z80 code and some tests) :

1a) 'stinger'

  - Here are some infos about the "Debug Mode" Dip Switch :

      * when it it set to OFF, the value which was previously written
        to 0xc500 is NOT erased, but the only value written to it is 0x00.
      * when it is set to ON, 0x00 is ALWAYS written to 0xc500.
        (check code at 0x0ef6)

    As you can see, there is no VISIBLE difference ...

    This is because it's in fact a leftover from 'stinger2' (the code for the
    "TEST PLAY" still exists !)


1b) 'stinger2'

  - Here are some infos about the "Debug Mode" Dip Switch :

      * when it it set to OFF, the value which was previously written
        to 0xc500 is NOT erased, but the only value written to it is 0x00.
      * when it is set to ON, 0x01 is ALWAYS written to 0xc500.
        (check code at 0x0ef6)

  - When 0xc500 = 0x01, you enter what the game calls "TEST PLAY".

  - "TEST PLAY" features :

      * automatically sets credits to 1
      * impossible to insert a coin
      * only one player game is available
      * player 1 has infinite lives (always set to 5)

  - "Coin B" settings also affect the "difficulty" and the "bongo time" :
    compare the code from 0x0e69 to 0x0e9c in the 2 sets, and you'll notice
    that it is the SAME ! That's why I've set the default coinage to be
    the same as the "difficulty" and "bongo time" settings in 'stinger'.


2)  'scion*'

  - Dip Switches 1-6 to 1-8 must remain OFF because of code at 0x28bf :
    there is NO "and $03" instruction, so it goes over the "coin B" table,
    which means that you have really weird coinage if you set them ON !


3)  'kungfut'

  - When "Microphone" Dip Switch is OFF, press BUTTON3 (bit 4 of IN1 -
    shared by the 2 players) to make a little cloud appear so players
    can climb it to go up.
  - "20000 60000" setting for the "Bonus Life" Dip Switch is NEVER used
    due to the "bit 0,a" instruction at 0x94fd !


***************************************************************************/

#include "emu.h"
#include "cpu/z80/z80.h"
#include "sound/ay8910.h"
#include "includes/wiz.h"


/***************************************************************************

  Stinger/Scion discrete sound

***************************************************************************/

#define STINGER_SHOT_EN1    NODE_01
#define STINGER_SHOT_EN2    NODE_02
#define STINGER_BOOM_EN1    NODE_03
#define STINGER_BOOM_EN2    NODE_04

// cut-and-pasted from Asteroid
static const discrete_lfsr_desc stinger_lfsr =
{
	DISC_CLK_IS_FREQ,
	16,         /* Bit Length */
	0,          /* Reset Value */
	6,          /* Use Bit 6 as XOR input 0 */
	14,         /* Use Bit 14 as XOR input 1 */
	DISC_LFSR_XNOR,     /* Feedback stage1 is XNOR */
	DISC_LFSR_OR,       /* Feedback stage2 is just stage 1 output OR with external feed */
	DISC_LFSR_REPLACE,  /* Feedback stage3 replaces the shifted register contents */
	0x000001,       /* Everything is shifted into the first bit only */
	0,          /* Output is already inverted by XNOR */
	16          /* Output bit is feedback bit */
};

static DISCRETE_SOUND_START(stinger)

#define STINGER_SHOT_OUT    NODE_90
#define STINGER_BOOM_OUT    NODE_91
#define STINGER_FINAL_MIX   NODE_99

	// triggers are interleaved to give each circuit sufficient time to reset
	DISCRETE_INPUT_LOGIC    (STINGER_SHOT_EN1) // even-inteval shots
	DISCRETE_INPUT_LOGIC    (STINGER_SHOT_EN2) // odd-inteval shots
	DISCRETE_INPUT_LOGIC    (STINGER_BOOM_EN1) // even-inteval explosions
	DISCRETE_INPUT_LOGIC    (STINGER_BOOM_EN2) // odd-inteval explosions

	//---------------------------------------
	// Sample Shot Sound Circuit

	#define SHOT_IN1    NODE_11
	#define SHOT_IN2    NODE_12
	#define SHOT_MOD    NODE_13
	#define SHOT_FRQ    NODE_14
	#define SHOT_AMP    NODE_15

	DISCRETE_RCDISC     (SHOT_IN1, STINGER_SHOT_EN1, 1.0, 0.2, 1.0)
	DISCRETE_RCDISC     (SHOT_IN2, STINGER_SHOT_EN2, 1.0, 0.2, 1.0)
	DISCRETE_SWITCH     (SHOT_MOD, 1, STINGER_SHOT_EN1, SHOT_IN2, SHOT_IN1)
	DISCRETE_MULTIPLY   (SHOT_FRQ, SHOT_MOD, 2000)
	DISCRETE_MULTIPLY   (SHOT_AMP, SHOT_MOD,  800)
	DISCRETE_SQUAREWAVE (STINGER_SHOT_OUT, 1, SHOT_FRQ, SHOT_AMP, 50, 0, 0)

	//---------------------------------------
	// Sample Explosion Sound Circuit

	#define BOOM_IN1    NODE_21
	#define BOOM_IN2    NODE_22
	#define BOOM_MOD    NODE_23
	#define BOOM_AMP    NODE_24

	DISCRETE_RCDISC     (BOOM_IN1, STINGER_BOOM_EN1, 1.0, 0.25, 1.0)
	DISCRETE_RCDISC     (BOOM_IN2, STINGER_BOOM_EN2, 1.0, 0.25, 1.0)
	DISCRETE_SWITCH     (BOOM_MOD, 1, STINGER_BOOM_EN1, BOOM_IN2, BOOM_IN1)
	DISCRETE_MULTIPLY   (BOOM_AMP, BOOM_MOD, 1500)
	DISCRETE_LFSR_NOISE (STINGER_BOOM_OUT, 1, 1, 1800, BOOM_AMP, 0, 0, &stinger_lfsr)

	//---------------------------------------

	DISCRETE_ADDER2 (STINGER_FINAL_MIX, 1, STINGER_SHOT_OUT, STINGER_BOOM_OUT)

	DISCRETE_OUTPUT (STINGER_FINAL_MIX, 5)

DISCRETE_SOUND_END

WRITE8_MEMBER(wiz_state::stinger_explosion_w)
{
	// explosion sound trigger(analog?)
	m_discrete->write(space, STINGER_BOOM_EN1, m_dsc1);
	m_discrete->write(space, STINGER_BOOM_EN2, m_dsc1^=1);
}

WRITE8_MEMBER(wiz_state::stinger_shot_w)
{
	// player shot sound trigger(analog?)
	m_discrete->write(space, STINGER_SHOT_EN1, m_dsc0);
	m_discrete->write(space, STINGER_SHOT_EN2, m_dsc0^=1);
}



/***************************************************************************

  I/O, Memory maps

***************************************************************************/

READ8_MEMBER(wiz_state::wiz_protection_r)
{
	switch (m_colorram2[0])
	{
		case 0x35: return 0x25; /* FIX: sudden player death + free play afterwards   */
		case 0x8f: return 0x1f; /* FIX: early boss appearance with corrupt graphics  */
		case 0xa0: return 0x00; /* FIX: executing junk code after defeating the boss */
	}

	return m_colorram2[0];
}

WRITE8_MEMBER(wiz_state::wiz_coin_counter_w)
{
	coin_counter_w(machine(), offset, data & 1);
}

WRITE8_MEMBER(wiz_state::wiz_main_nmi_mask_w)
{
	m_main_nmi_mask = data & 1;
}


static ADDRESS_MAP_START( kungfut_main_map, AS_PROGRAM, 8, wiz_state )
	AM_RANGE(0x0000, 0xbfff) AM_ROM
	AM_RANGE(0xc000, 0xc7ff) AM_RAM
	AM_RANGE(0xd000, 0xd3ff) AM_RAM AM_SHARE("videoram2")
	AM_RANGE(0xd400, 0xd7ff) AM_RAM AM_SHARE("colorram2")
	AM_RANGE(0xd800, 0xd83f) AM_RAM AM_SHARE("attrram2")
	AM_RANGE(0xd840, 0xd85f) AM_RAM AM_SHARE("spriteram2")
	AM_RANGE(0xe000, 0xe3ff) AM_RAM AM_SHARE("videoram")
	AM_RANGE(0xe400, 0xe7ff) AM_RAM AM_SHARE("colorram")
	AM_RANGE(0xe800, 0xe83f) AM_RAM AM_SHARE("attrram")
	AM_RANGE(0xe840, 0xe85f) AM_RAM AM_SHARE("spriteram")
	AM_RANGE(0xf000, 0xf000) AM_READ_PORT("DSW0")
	AM_RANGE(0xf001, 0xf001) AM_WRITE(wiz_main_nmi_mask_w)
	AM_RANGE(0xf002, 0xf003) AM_WRITE(wiz_palette_bank_w)
	AM_RANGE(0xf004, 0xf005) AM_WRITE(wiz_char_bank_w)
	AM_RANGE(0xf006, 0xf006) AM_WRITE(wiz_flipx_w)
	AM_RANGE(0xf007, 0xf007) AM_WRITE(wiz_flipy_w)
	AM_RANGE(0xf008, 0xf008) AM_READ_PORT("DSW1")
	AM_RANGE(0xf010, 0xf010) AM_READ_PORT("IN0")
	AM_RANGE(0xf018, 0xf018) AM_READ_PORT("IN1")
	AM_RANGE(0xf800, 0xf800) AM_WRITE(soundlatch_byte_w)
	AM_RANGE(0xf818, 0xf818) AM_WRITE(wiz_bgcolor_w)
ADDRESS_MAP_END

static ADDRESS_MAP_START( decrypted_opcodes_map, AS_DECRYPTED_OPCODES, 8, wiz_state )
	AM_RANGE(0x0000, 0xbfff) AM_ROM AM_SHARE("decrypted_opcodes")
ADDRESS_MAP_END

static ADDRESS_MAP_START( wiz_main_map, AS_PROGRAM, 8, wiz_state )
	AM_RANGE(0xc800, 0xc801) AM_WRITE(wiz_coin_counter_w)
	AM_RANGE(0xd400, 0xd400) AM_READ(wiz_protection_r)
	AM_RANGE(0xf000, 0xf000) AM_WRITE(wiz_sprite_bank_w)
	AM_IMPORT_FROM( kungfut_main_map )
ADDRESS_MAP_END

static ADDRESS_MAP_START( stinger_main_map, AS_PROGRAM, 8, wiz_state )
//  AM_RANGE(0xf008, 0xf00f) AM_WRITENOP // ?
	AM_RANGE(0xf800, 0xf800) AM_READ(watchdog_reset_r)
	AM_RANGE(0xf808, 0xf808) AM_WRITE(stinger_explosion_w)
	AM_RANGE(0xf80a, 0xf80a) AM_WRITE(stinger_shot_w)
	AM_IMPORT_FROM( kungfut_main_map )
ADDRESS_MAP_END


/**************************************************************************/

WRITE8_MEMBER(wiz_state::wiz_sound_nmi_mask_w)
{
	m_sound_nmi_mask = data & 1;
}


static ADDRESS_MAP_START( kungfut_sound_map, AS_PROGRAM, 8, wiz_state )
	ADDRESS_MAP_GLOBAL_MASK(0x7fff)
	AM_RANGE(0x0000, 0x1fff) AM_ROM
	AM_RANGE(0x2000, 0x23ff) AM_RAM
	AM_RANGE(0x4000, 0x4001) AM_DEVWRITE("8910.3", ay8910_device, address_data_w)
	AM_RANGE(0x5000, 0x5001) AM_DEVWRITE("8910.1", ay8910_device, address_data_w)
	AM_RANGE(0x6000, 0x6001) AM_DEVWRITE("8910.2", ay8910_device, address_data_w)
	AM_RANGE(0x7000, 0x7000) AM_READWRITE(soundlatch_byte_r, wiz_sound_nmi_mask_w)
ADDRESS_MAP_END

static ADDRESS_MAP_START( stinger_sound_map, AS_PROGRAM, 8, wiz_state )
	ADDRESS_MAP_GLOBAL_MASK(0x7fff)
	AM_RANGE(0x0000, 0x1fff) AM_ROM
	AM_RANGE(0x2000, 0x23ff) AM_RAM
	AM_RANGE(0x3000, 0x3000) AM_READWRITE(soundlatch_byte_r, wiz_sound_nmi_mask_w)
	AM_RANGE(0x4000, 0x4000) AM_WRITENOP // ?
	AM_RANGE(0x5000, 0x5001) AM_DEVWRITE("8910.1", ay8910_device, address_data_w)
	AM_RANGE(0x6000, 0x6001) AM_DEVWRITE("8910.2", ay8910_device, address_data_w)
	AM_RANGE(0x5000, 0x7fff) AM_READNOP // prevent error.log spam, cpu jumps here by accident
ADDRESS_MAP_END



/***************************************************************************

  Inputs

***************************************************************************/

static INPUT_PORTS_START( stinger )
	PORT_START("IN0")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_COIN1 )
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_BUTTON2 ) PORT_COCKTAIL
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_BUTTON2 )
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_START2 )
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_START1 )
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_COIN2 )
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_BUTTON1 )
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_COCKTAIL

	PORT_START("IN1")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT )
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT )
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT )  PORT_COCKTAIL
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT ) PORT_COCKTAIL
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_JOYSTICK_UP )
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_JOYSTICK_DOWN )
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_JOYSTICK_UP )    PORT_COCKTAIL
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_JOYSTICK_DOWN )  PORT_COCKTAIL

	PORT_START("DSW0")
	PORT_DIPNAME( 0x07, 0x07, DEF_STR( Coin_A ) )
	PORT_DIPSETTING(    0x01, DEF_STR( 4C_1C ) )
	PORT_DIPSETTING(    0x02, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(    0x04, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x00, DEF_STR( 3C_2C ) )
	PORT_DIPSETTING(    0x07, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x03, DEF_STR( 2C_3C ) )
	PORT_DIPSETTING(    0x06, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0x05, DEF_STR( 1C_3C ) )
	PORT_DIPNAME( 0x18, 0x08, DEF_STR( Lives ) )
	PORT_DIPSETTING(    0x00, "2" )
	PORT_DIPSETTING(    0x08, "3" )
	PORT_DIPSETTING(    0x10, "4" )
	PORT_DIPSETTING(    0x18, "5" )
	PORT_DIPNAME( 0xe0, 0xe0, DEF_STR( Bonus_Life ) )
	PORT_DIPSETTING(    0xe0, "20000 50000" )
	PORT_DIPSETTING(    0xc0, "20000 60000" )
	PORT_DIPSETTING(    0xa0, "20000 70000" )
	PORT_DIPSETTING(    0x80, "20000 80000" )
	PORT_DIPSETTING(    0x60, "20000 90000" )
	PORT_DIPSETTING(    0x40, "30000 80000" )
	PORT_DIPSETTING(    0x20, "30000 90000" )
	PORT_DIPSETTING(    0x00, DEF_STR( None ) )

	PORT_START("DSW1")
	PORT_DIPNAME( 0x01, 0x00, "Debug Mode" )        /* See notes */
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x01, DEF_STR( On ) )
	PORT_DIPNAME( 0x0e, 0x0e, DEF_STR( Coin_B ) )
	PORT_DIPSETTING(    0x00, DEF_STR( 5C_1C ) )
	PORT_DIPSETTING(    0x08, DEF_STR( 4C_1C ) )
	PORT_DIPSETTING(    0x0c, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(    0x0a, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x04, DEF_STR( 3C_2C ) )
	PORT_DIPSETTING(    0x0e, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x02, DEF_STR( 2C_3C ) )
	PORT_DIPSETTING(    0x06, DEF_STR( 1C_2C ) )
	PORT_DIPNAME( 0x30, 0x20, "Bongo Time" )
	PORT_DIPSETTING(    0x30, "Long" )
	PORT_DIPSETTING(    0x20, DEF_STR( Medium ) )
	PORT_DIPSETTING(    0x10, "Short" )
	PORT_DIPSETTING(    0x00, "Shortest" )
	PORT_DIPNAME( 0x40, 0x00, DEF_STR( Difficulty ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Normal ) )
	PORT_DIPSETTING(    0x40, DEF_STR( Hard ) )
	PORT_DIPNAME( 0x80, 0x80, DEF_STR( Cabinet ) )
	PORT_DIPSETTING(    0x80, DEF_STR( Upright ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Cocktail ) )
INPUT_PORTS_END

static INPUT_PORTS_START( stinger2 )
	PORT_INCLUDE( stinger )

	PORT_MODIFY("DSW1")
	PORT_DIPNAME( 0x02, 0x00, DEF_STR( Unused ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x02, DEF_STR( On ) )
	PORT_DIPNAME( 0x04, 0x00, DEF_STR( Unused ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x04, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x00, DEF_STR( Unused ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x08, DEF_STR( On ) )
	PORT_DIPNAME( 0x70, 0x20, DEF_STR( Coin_B ) )   /* See notes */
	PORT_DIPSETTING(    0x70, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x60, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0x50, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(    0x40, DEF_STR( 1C_4C ) )
	PORT_DIPSETTING(    0x30, DEF_STR( 1C_5C ) )
	PORT_DIPSETTING(    0x20, DEF_STR( 1C_6C ) )
	PORT_DIPSETTING(    0x10, DEF_STR( 1C_7C ) )
	PORT_DIPSETTING(    0x00, DEF_STR( 1C_8C ) )
INPUT_PORTS_END

static INPUT_PORTS_START( scion )
	PORT_INCLUDE( stinger )

	PORT_MODIFY("IN1")
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT ) PORT_COCKTAIL
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_JOYSTICK_DOWN )  PORT_COCKTAIL
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT )  PORT_COCKTAIL
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_JOYSTICK_UP )    PORT_COCKTAIL

	PORT_MODIFY("DSW0")
	PORT_DIPNAME( 0x01, 0x01, DEF_STR( Cabinet ) )
	PORT_DIPSETTING(    0x01, DEF_STR( Upright ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Cocktail ) )
	PORT_DIPNAME( 0x02, 0x00, DEF_STR( Difficulty ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Easy ) )
	PORT_DIPSETTING(    0x02, DEF_STR( Hard ) )
	PORT_DIPNAME( 0x0c, 0x04, DEF_STR( Lives ) )
	PORT_DIPSETTING(    0x00, "2" )
	PORT_DIPSETTING(    0x04, "3" )
	PORT_DIPSETTING(    0x08, "4" )
	PORT_DIPSETTING(    0x0c, "5" )
	PORT_DIPNAME( 0x30, 0x00, DEF_STR( Bonus_Life ) )
	PORT_DIPSETTING(    0x00, "20000 40000" )
	PORT_DIPSETTING(    0x20, "20000 60000" )
	PORT_DIPSETTING(    0x10, "20000 80000" )
	PORT_DIPSETTING(    0x30, "30000 90000" )
	PORT_DIPNAME( 0x40, 0x00, DEF_STR( Unused ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x40, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x00, DEF_STR( Unused ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x80, DEF_STR( On ) )

	PORT_MODIFY("DSW1")
	PORT_DIPNAME( 0x07, 0x00, DEF_STR( Coin_A ) )
	PORT_DIPSETTING(    0x07, DEF_STR( 5C_1C ) )
	PORT_DIPSETTING(    0x03, DEF_STR( 4C_1C ) )
	PORT_DIPSETTING(    0x05, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(    0x01, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x00, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x04, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0x02, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(    0x06, DEF_STR( 1C_5C ) )
	PORT_DIPNAME( 0x18, 0x00, DEF_STR( Coin_B ) )
	PORT_DIPSETTING(    0x18, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(    0x08, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x00, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x10, DEF_STR( 1C_2C ) )
	PORT_DIPNAME( 0x20, 0x00, DEF_STR( Unused ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
//  PORT_DIPSETTING(    0x20, DEF_STR( On ) )       /* See notes */
	PORT_DIPNAME( 0x40, 0x00, DEF_STR( Unused ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
//  PORT_DIPSETTING(    0x40, DEF_STR( On ) )       /* See notes */
	PORT_DIPNAME( 0x80, 0x00, DEF_STR( Unused ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
//  PORT_DIPSETTING(    0x80, DEF_STR( On ) )       /* See notes */
INPUT_PORTS_END

static INPUT_PORTS_START( kungfut )
	PORT_START("IN0")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_COIN1 )
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_BUTTON2 ) PORT_PLAYER(2)
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_BUTTON2 )
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_START2 )
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_START1 )
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_COIN2 )
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_BUTTON1 )
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_PLAYER(2)

	PORT_START("IN1")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT ) PORT_2WAY
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT )  PORT_2WAY
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT ) PORT_2WAY PORT_PLAYER(2)
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_BUTTON3 ) PORT_NAME ("Microphone Input") // call up cloud
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT ) PORT_2WAY PORT_PLAYER(2)
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_UNKNOWN )

	PORT_START("DSW0")
	PORT_DIPNAME( 0x07, 0x00, DEF_STR( Coin_A ) )
	PORT_DIPSETTING(    0x07, DEF_STR( 5C_1C ) )
	PORT_DIPSETTING(    0x03, DEF_STR( 4C_1C ) )
	PORT_DIPSETTING(    0x05, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(    0x01, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x00, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x04, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0x02, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(    0x06, DEF_STR( 1C_5C ) )
	PORT_DIPNAME( 0x18, 0x00, DEF_STR( Coin_B ) )
	PORT_DIPSETTING(    0x18, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(    0x08, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x00, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x10, DEF_STR( 1C_2C ) )
	PORT_DIPNAME( 0x20, 0x20, "2 Players Game" )
	PORT_DIPSETTING(    0x00, "1 Credit" )
	PORT_DIPSETTING(    0x20, "2 Credits" )
	PORT_DIPNAME( 0x40, 0x00, DEF_STR( Unused ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x40, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x00, DEF_STR( Unused ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x80, DEF_STR( On ) )

	PORT_START("DSW1")
	PORT_DIPNAME( 0x01, 0x00, DEF_STR( Difficulty ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Easy ) )
	PORT_DIPSETTING(    0x01, DEF_STR( Hard ) )
	PORT_DIPNAME( 0x02, 0x00, DEF_STR( Unused ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x02, DEF_STR( On ) )
	PORT_DIPNAME( 0x04, 0x04, "Microphone" )        /* See notes */
	PORT_DIPSETTING(    0x04, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x18, 0x08, DEF_STR( Lives ) )
	PORT_DIPSETTING(    0x00, "2" )
	PORT_DIPSETTING(    0x08, "3" )
	PORT_DIPSETTING(    0x10, "4" )
	PORT_DIPSETTING(    0x18, "5" )
	PORT_DIPNAME( 0x60, 0x00, DEF_STR( Bonus_Life ) )
	PORT_DIPSETTING(    0x00, "20000 40000" )
//  PORT_DIPSETTING(    0x20, "20000 40000" )       // duplicated setting
	PORT_DIPSETTING(    0x10, "20000 80000" )
	PORT_DIPSETTING(    0x30, "30000 90000" )
	PORT_DIPNAME( 0x80, 0x00, DEF_STR( Unused ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x80, DEF_STR( On ) )
INPUT_PORTS_END

static INPUT_PORTS_START( wiz )
	PORT_START("IN0")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_COIN1 )
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_COCKTAIL     // Shoot
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_BUTTON1 )                   // Shoot
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_START2 )
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_START1 )
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_COIN2 )
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_BUTTON2 )                   // Magic
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_BUTTON2 ) PORT_COCKTAIL     // Magic

	PORT_START("IN1")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT )
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT )
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_JOYSTICK_UP )
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_JOYSTICK_DOWN )
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT ) PORT_COCKTAIL
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT )  PORT_COCKTAIL
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_JOYSTICK_UP )    PORT_COCKTAIL
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_JOYSTICK_DOWN )  PORT_COCKTAIL

	PORT_START("DSW0")
	PORT_DIPNAME( 0x07, 0x00, DEF_STR( Coin_A ) )
	PORT_DIPSETTING(    0x07, DEF_STR( 5C_1C ) )
	PORT_DIPSETTING(    0x03, DEF_STR( 4C_1C ) )
	PORT_DIPSETTING(    0x05, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(    0x01, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x00, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x04, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0x02, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(    0x06, DEF_STR( 1C_5C ) )
	PORT_DIPNAME( 0x18, 0x00, DEF_STR( Coin_B ) )
	PORT_DIPSETTING(    0x08, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x00, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x18, DEF_STR( 2C_3C ) )
	PORT_DIPSETTING(    0x10, DEF_STR( 1C_2C ) )
	PORT_DIPNAME( 0x20, 0x00, DEF_STR( Unused ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x20, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x00, DEF_STR( Unused ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x40, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x00, DEF_STR( Free_Play ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x80, DEF_STR( On ) )

	PORT_START("DSW1")
	PORT_DIPNAME( 0x01, 0x00, DEF_STR( Cabinet ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Upright ) )
	PORT_DIPSETTING(    0x01, DEF_STR( Cocktail ) )
	PORT_DIPNAME( 0x06, 0x00, DEF_STR( Difficulty ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Easy ) )
	PORT_DIPSETTING(    0x02, DEF_STR( Normal ) )
	PORT_DIPSETTING(    0x04, DEF_STR( Hard ) )
	PORT_DIPSETTING(    0x06, DEF_STR( Hardest ) )
	PORT_DIPNAME( 0x18, 0x10, DEF_STR( Lives ) )
	PORT_DIPSETTING(    0x08, "1" )
	PORT_DIPSETTING(    0x10, "3" )
	PORT_DIPSETTING(    0x18, "5" )
	PORT_DIPSETTING(    0x00, "255 (Cheat)")
	PORT_DIPNAME( 0x60, 0x00, DEF_STR( Bonus_Life ) )
	PORT_DIPSETTING(    0x00, "10000 30000" )
	PORT_DIPSETTING(    0x20, "20000 40000" )
	PORT_DIPSETTING(    0x40, "30000 60000" )
	PORT_DIPSETTING(    0x60, "40000 80000" )
	PORT_SERVICE( 0x80, IP_ACTIVE_HIGH )
INPUT_PORTS_END



/***************************************************************************

  Machine configs, misc. interface

***************************************************************************/

static const gfx_layout charlayout =
{
	8,8,    /* 8*8 characters */
	256,    /* 256 characters */
	3,      /* 3 bits per pixel */
	{ 0x4000*8, 0x2000*8, 0 },  /* the three bitplanes are separated */
	{ 0, 1, 2, 3, 4, 5, 6, 7 },
	{ 0*8, 1*8, 2*8, 3*8, 4*8, 5*8, 6*8, 7*8 },
	8*8 /* every char takes 8 consecutive bytes */
};

static const gfx_layout spritelayout =
{
	16,16,  /* 16*16 sprites */
	256,    /* 256 sprites */
	3,      /* 3 bits per pixel */
	{ 0x4000*8, 0x2000*8, 0 },  /* the three bitplanes are separated */
	{ 0, 1, 2, 3, 4, 5, 6, 7,
		8*8+0, 8*8+1, 8*8+2, 8*8+3, 8*8+4, 8*8+5, 8*8+6, 8*8+7 },
	{ 0*8, 1*8, 2*8, 3*8, 4*8, 5*8, 6*8, 7*8,
		16*8, 17*8, 18*8, 19*8, 20*8, 21*8, 22*8, 23*8 },
	32*8    /* every sprite takes 32 consecutive bytes */
};


static GFXDECODE_START( wiz )
	GFXDECODE_ENTRY( "gfx1", 0x0000, charlayout,   0, 32 )
	GFXDECODE_ENTRY( "gfx1", 0x0800, charlayout,   0, 32 )
	GFXDECODE_ENTRY( "gfx2", 0x6000, charlayout,   0, 32 )
	GFXDECODE_ENTRY( "gfx2", 0x0000, charlayout,   0, 32 )
	GFXDECODE_ENTRY( "gfx2", 0x0800, charlayout,   0, 32 )
	GFXDECODE_ENTRY( "gfx2", 0x6800, charlayout,   0, 32 )
	GFXDECODE_ENTRY( "gfx1", 0x0000, spritelayout, 0, 32 )
	GFXDECODE_ENTRY( "gfx2", 0x0000, spritelayout, 0, 32 )
	GFXDECODE_ENTRY( "gfx2", 0x6000, spritelayout, 0, 32 )
GFXDECODE_END

static GFXDECODE_START( stinger )
	GFXDECODE_ENTRY( "gfx1", 0x0000, charlayout,   0, 32 )
	GFXDECODE_ENTRY( "gfx1", 0x0800, charlayout,   0, 32 )
	GFXDECODE_ENTRY( "gfx2", 0x0000, charlayout,   0, 32 )
	GFXDECODE_ENTRY( "gfx2", 0x0800, charlayout,   0, 32 )
	GFXDECODE_ENTRY( "gfx1", 0x0000, spritelayout, 0, 32 )
	GFXDECODE_ENTRY( "gfx2", 0x0000, spritelayout, 0, 32 )
GFXDECODE_END

/**************************************************************************/

void wiz_state::machine_reset()
{
	m_main_nmi_mask = 0;
	m_sound_nmi_mask = 0;
	m_dsc0 = m_dsc1 = 1;

	m_sprite_bank = 0;
	m_charbank[0] = m_charbank[1] = 0;
	m_palbank[0] = m_palbank[1] = 0;
	m_flipx = 0;
	m_flipy = 0;
	m_bgcolor = 0;
}

void wiz_state::machine_start()
{
	// register for savestates
	save_item(NAME(m_main_nmi_mask));
	save_item(NAME(m_sound_nmi_mask));
	save_item(NAME(m_dsc0));
	save_item(NAME(m_dsc1));

	save_item(NAME(m_sprite_bank));
	save_item(NAME(m_charbank));
	save_item(NAME(m_palbank));
	save_item(NAME(m_flipx));
	save_item(NAME(m_flipy));
	save_item(NAME(m_bgcolor));
}

/**************************************************************************/

INTERRUPT_GEN_MEMBER(wiz_state::wiz_vblank_interrupt)
{
	if (m_main_nmi_mask & 1)
		device.execute().set_input_line(INPUT_LINE_NMI, PULSE_LINE);
}

INTERRUPT_GEN_MEMBER(wiz_state::wiz_sound_interrupt)
{
	if (m_sound_nmi_mask & 1)
		device.execute().set_input_line(INPUT_LINE_NMI, PULSE_LINE);
}

static MACHINE_CONFIG_START( kungfut, wiz_state )

	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", Z80, 18432000/6) /* 3.072 MHz ??? */
	MCFG_CPU_PROGRAM_MAP(kungfut_main_map)
	MCFG_CPU_VBLANK_INT_DRIVER("screen", wiz_state, wiz_vblank_interrupt)

	MCFG_CPU_ADD("audiocpu", Z80, 18432000/6) /* 3.072 MHz ??? */
	MCFG_CPU_PROGRAM_MAP(kungfut_sound_map)
	MCFG_CPU_PERIODIC_INT_DRIVER(wiz_state, wiz_sound_interrupt, 4*60) /* ??? */

	/* video hardware */
	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_REFRESH_RATE(60)
	MCFG_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(2500) /* not accurate */ )
	MCFG_SCREEN_SIZE(32*8, 32*8)
	MCFG_SCREEN_VISIBLE_AREA(0*8, 32*8-1, 2*8, 30*8-1)
	MCFG_SCREEN_UPDATE_DRIVER(wiz_state, screen_update_kungfut)
	MCFG_SCREEN_PALETTE("palette")

	MCFG_GFXDECODE_ADD("gfxdecode", "palette", stinger)
	MCFG_PALETTE_ADD("palette", 256)
	MCFG_PALETTE_INIT_OWNER(wiz_state, wiz)

	/* sound hardware */
	MCFG_SPEAKER_STANDARD_MONO("mono")

	MCFG_SOUND_ADD("8910.1", AY8910, 18432000/12)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.10)

	MCFG_SOUND_ADD("8910.2", AY8910, 18432000/12)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.10)

	MCFG_SOUND_ADD("8910.3", AY8910, 18432000/12)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.10)
MACHINE_CONFIG_END

static MACHINE_CONFIG_DERIVED( wiz, kungfut )

	/* basic machine hardware */
	MCFG_CPU_MODIFY("maincpu")
	MCFG_CPU_PROGRAM_MAP(wiz_main_map)

	/* video hardware */
	MCFG_GFXDECODE_MODIFY("gfxdecode", wiz)
	MCFG_SCREEN_MODIFY("screen")
	MCFG_SCREEN_UPDATE_DRIVER(wiz_state, screen_update_wiz)
MACHINE_CONFIG_END


static MACHINE_CONFIG_DERIVED( stinger, kungfut )

	/* basic machine hardware */
	MCFG_CPU_MODIFY("maincpu")
	MCFG_CPU_PROGRAM_MAP(stinger_main_map)
	MCFG_CPU_DECRYPTED_OPCODES_MAP(decrypted_opcodes_map)

	/* basic machine hardware */
	MCFG_CPU_MODIFY("audiocpu")
	MCFG_CPU_PROGRAM_MAP(stinger_sound_map)

	/* video hardware */
	MCFG_SCREEN_MODIFY("screen")
	MCFG_SCREEN_UPDATE_DRIVER(wiz_state, screen_update_stinger)

	/* sound hardware */
	MCFG_DEVICE_REMOVE("8910.3")

	MCFG_SOUND_ADD("discrete", DISCRETE, 0)
	MCFG_DISCRETE_INTF(stinger)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.5)
MACHINE_CONFIG_END

static MACHINE_CONFIG_DERIVED( scion, stinger )

	MCFG_CPU_MODIFY("maincpu")
	MCFG_DEVICE_REMOVE_ADDRESS_MAP(AS_DECRYPTED_OPCODES)

	/* video hardware */
	MCFG_SCREEN_MODIFY("screen")
	MCFG_SCREEN_VISIBLE_AREA(2*8, 32*8-1, 2*8, 30*8-1)
MACHINE_CONFIG_END



/***************************************************************************

  Game driver(s)

***************************************************************************/

ROM_START( kungfut )
	ROM_REGION( 0xc000, "maincpu", 0 )
	ROM_LOAD( "p1.bin",  0x0000, 0x4000, CRC(b1e56960) SHA1(993388bbb663412110d1012be9ffc00b06fce4d0) )
	ROM_LOAD( "p3.bin",  0x4000, 0x4000, CRC(6fc346f8) SHA1(bd1663fa780e41eafd668bf502b40c9750270e55) )
	ROM_LOAD( "p2.bin",  0x8000, 0x4000, CRC(042cc9c5) SHA1(09f87e240c2aaa19fe7b8cb548ded828ab67b18b) )

	ROM_REGION( 0x2000, "audiocpu", 0 )
	ROM_LOAD( "1.bin",  0x0000, 0x2000, CRC(68028a5d) SHA1(2fabf5e55e09a34cd090d123737d31970e4086e8) )

	ROM_REGION( 0x6000,  "gfx1", 0 )
	ROM_LOAD( "2.bin",  0x0000, 0x2000, CRC(5c3ef697) SHA1(5218d361e6020aefb1925a8034a5ed6eb7bb1001) )
	ROM_LOAD( "3.bin",  0x2000, 0x2000, CRC(905e81fa) SHA1(8d3328b2dc7e99ab1e43420a517f04ec4d463b05) )
	ROM_LOAD( "4.bin",  0x4000, 0x2000, CRC(965bb5d1) SHA1(ea837118d98378303cf9173005cfd50823b1596a) )

	ROM_REGION( 0x6000,  "gfx2", 0 )
	ROM_LOAD( "5.bin",  0x0000, 0x2000, CRC(763bb61a) SHA1(9bea4a929db5d2e8c925a847591b9e5b2ad5aaaa) )
	ROM_LOAD( "6.bin",  0x2000, 0x2000, CRC(c9649fce) SHA1(f65e75355d2f7b0899ea3769146a55b187da37d3) )
	ROM_LOAD( "7.bin",  0x4000, 0x2000, CRC(32f02c13) SHA1(85781f03cca622ce8ee66924a1e72758ce42bdfe) )

	ROM_REGION( 0x0300,  "proms", 0 )
	ROM_LOAD( "82s129.0", 0x0000, 0x0100, CRC(eb823177) SHA1(a28233dbf87744a9896fe675b76603557e7f596b) )
	ROM_LOAD( "82s129.1", 0x0100, 0x0100, CRC(6eec5dd9) SHA1(e846209c167b2a7d790faacea082a7edc1338e47) )
	ROM_LOAD( "82s129.2", 0x0200, 0x0100, CRC(c31eb3e6) SHA1(94fb8c6d83432c5f456510d628971147d373faf5) )
ROM_END

ROM_START( kungfuta )
	ROM_REGION( 0xc000, "maincpu", 0 )
	ROM_LOAD( "kungfu.01",  0x0000, 0x4000, CRC(48dada70) SHA1(a90901d2aef73d4fa9d9c80769b82a3fead0a0f9) )
	ROM_LOAD( "kungfu.02",  0x4000, 0x4000, CRC(c08c5152) SHA1(c5db3ee1ee165708d93d296b51a5bb43265ac75f) )
	ROM_LOAD( "kungfu.03",  0x8000, 0x4000, CRC(09b8670c) SHA1(170e7cbf87727d940e959fa7a0328b4cc1aba195) )

	ROM_REGION( 0x2000, "audiocpu", 0 )
	ROM_LOAD( "kungfu.04",  0x0000, 0x2000,  CRC(352bff48) SHA1(87d2408d31e1326ec810debcb2c724d1f003ae7b) )

	ROM_REGION( 0x6000,  "gfx1", 0 )
	ROM_LOAD( "kungfu.08",  0x0000, 0x2000, CRC(60b91d2f) SHA1(4a3337bb8f475a40b9b7f31d4b42e73179177763) )
	ROM_LOAD( "kungfu.09",  0x2000, 0x2000, CRC(121ba029) SHA1(4e4375cb9e93af45dd16e2a295fa88753201e6e8) )
	ROM_LOAD( "kungfu.10",  0x4000, 0x2000, CRC(146df9de) SHA1(8dcc33bc281f1e5b069b52645123de62037261e6) )

	ROM_REGION( 0x6000,  "gfx2", 0 )
	ROM_LOAD( "kungfu.07",  0x0000, 0x2000, CRC(1df48de5) SHA1(e620ea55a1ed2dc934878d077d5cd6437e833a6d) )
	ROM_LOAD( "kungfu.06",  0x2000, 0x2000, CRC(1921d49b) SHA1(fcc5500c8c1605e571b203828d6a7de36ad76fab) )
	ROM_LOAD( "kungfu.05",  0x4000, 0x2000, CRC(ff9aced4) SHA1(b13f8ea4131b54bdd2888841f52f1482b02b6624) )

	ROM_REGION( 0x0300,  "proms", 0 )
	ROM_LOAD( "82s129.0", 0x0000, 0x0100, CRC(eb823177) SHA1(a28233dbf87744a9896fe675b76603557e7f596b) ) // ic.23
	ROM_LOAD( "82s129.1", 0x0100, 0x0100, CRC(6eec5dd9) SHA1(e846209c167b2a7d790faacea082a7edc1338e47) ) // ic.24
	ROM_LOAD( "82s129.2", 0x0200, 0x0100, CRC(c31eb3e6) SHA1(94fb8c6d83432c5f456510d628971147d373faf5) ) // ic.25
ROM_END

ROM_START( wiz )
	ROM_REGION( 0xc000, "maincpu", 0 )
	ROM_LOAD( "ic07_01.bin",  0x0000, 0x4000, CRC(c05f2c78) SHA1(98b93234684a3a228552ef41a08512fef1befedd) )
	ROM_LOAD( "ic05_03.bin",  0x4000, 0x4000, CRC(7978d879) SHA1(866efdff3c111793d5a3cc2fa0b03a2b4e371c49) )
	ROM_LOAD( "ic06_02.bin",  0x8000, 0x4000, CRC(9c406ad2) SHA1(cd82c3dc622886b6ebb30ba565f3c34d5a4e229b) )

	ROM_REGION( 0x2000, "audiocpu", 0 )
	ROM_LOAD( "ic57_10.bin",  0x0000, 0x2000, CRC(8a7575bd) SHA1(5470c4c3a40139f45db7a9e260f40b5244f10123) )

	ROM_REGION( 0x6000,  "gfx1", 0 )    /* sprites/chars */
	ROM_LOAD( "ic12_04.bin",  0x0000, 0x2000, CRC(8969acdd) SHA1(f37c4697232b4fb4171d6290c9407f740e7d1448) )
	ROM_LOAD( "ic13_05.bin",  0x2000, 0x2000, CRC(2868e6a5) SHA1(1b8ac71a6b901df845bab945bfcf11df47932990) )
	ROM_LOAD( "ic14_06.bin",  0x4000, 0x2000, CRC(b398e142) SHA1(1cafaf5cbfa96b410ae236a298473ff51122d9fc) )

	ROM_REGION( 0xc000,  "gfx2", 0 )    /* sprites/chars */
	ROM_LOAD( "ic03_07.bin",  0x0000, 0x2000, CRC(297c02fc) SHA1(8eee765a660e3ff1b6cdcdac0d068177098cc339) )
	ROM_CONTINUE(             0x6000, 0x2000  )
	ROM_LOAD( "ic02_08.bin",  0x2000, 0x2000, CRC(ede77d37) SHA1(01fe35fc3373b7513ea90e8262d66200629b89fe) )
	ROM_CONTINUE(             0x8000, 0x2000  )
	ROM_LOAD( "ic01_09.bin",  0x4000, 0x2000, CRC(4d86b041) SHA1(fe7f8c89ef16020f45a97ed875ddd7396a32665d) )
	ROM_CONTINUE(             0xa000, 0x2000  )

	ROM_REGION( 0x0300, "proms", 0 )
	ROM_LOAD( "ic23_3-1.bin", 0x0000, 0x0100, CRC(2dd52fb2) SHA1(61722aba7a370f4a97cafbd5df88ec7c6263c4ad) )    /* palette red component */
	ROM_LOAD( "ic23_3-2.bin", 0x0100, 0x0100, CRC(8c2880c9) SHA1(9b4c17f7fa5d6dc01d79c40cec9725ab97f514cb) )    /* palette green component */
	ROM_LOAD( "ic23_3-3.bin", 0x0200, 0x0100, CRC(a488d761) SHA1(6dade1dd16905b4751778d49f374936795c3fb6e) )    /* palette blue component */
ROM_END

ROM_START( wizt )
	ROM_REGION( 0xc000, "maincpu", 0 )
	ROM_LOAD( "wiz1.bin",     0x0000, 0x4000, CRC(5a6d3c60) SHA1(faeb7e7ddeee9638ec046655e87f866d81fdbee0) )
	ROM_LOAD( "ic05_03.bin",  0x4000, 0x4000, CRC(7978d879) SHA1(866efdff3c111793d5a3cc2fa0b03a2b4e371c49) )
	ROM_LOAD( "ic06_02.bin",  0x8000, 0x4000, CRC(9c406ad2) SHA1(cd82c3dc622886b6ebb30ba565f3c34d5a4e229b) )

	ROM_REGION( 0x2000, "audiocpu", 0 )
	ROM_LOAD( "ic57_10.bin",  0x0000, 0x2000, CRC(8a7575bd) SHA1(5470c4c3a40139f45db7a9e260f40b5244f10123) )

	ROM_REGION( 0x6000,  "gfx1", 0 )    /* sprites/chars */
	ROM_LOAD( "wiz4.bin",     0x0000, 0x2000, CRC(e6c636b3) SHA1(0d5b98d404d2d87f375cde5d5a90c7d6318ea197) )
	ROM_LOAD( "wiz5.bin",     0x2000, 0x2000, CRC(77986058) SHA1(8002affdd9ac246a0b9c887654d0db8d3a6913b2) )
	ROM_LOAD( "wiz6.bin",     0x4000, 0x2000, CRC(f6970b23) SHA1(82d1fe0fee6bf9c6c2f472ed3479c02da85d5f69) )

	ROM_REGION( 0xc000,  "gfx2", 0 )    /* sprites/chars */
	ROM_LOAD( "wiz7.bin",     0x0000, 0x2000, CRC(601f2f3f) SHA1(6c0cc7de5fd94628eaecca409c4faa155f684bdc) )
	ROM_CONTINUE(             0x6000, 0x2000 )
	ROM_LOAD( "wiz8.bin",     0x2000, 0x2000, CRC(f5ab982d) SHA1(5e0e72ec702dd5f48814a15f1a92bcdd29c944d8) )
	ROM_CONTINUE(             0x8000, 0x2000 )
	ROM_LOAD( "wiz9.bin",     0x4000, 0x2000, CRC(f6c662e2) SHA1(54e904d731ea30f532dfea60d47edf2da99f32eb) )
	ROM_CONTINUE(             0xa000, 0x2000 )

	ROM_REGION( 0x0300, "proms", 0 )
	ROM_LOAD( "ic23_3-1.bin", 0x0000, 0x0100, CRC(2dd52fb2) SHA1(61722aba7a370f4a97cafbd5df88ec7c6263c4ad) )    /* palette red component */
	ROM_LOAD( "ic23_3-2.bin", 0x0100, 0x0100, CRC(8c2880c9) SHA1(9b4c17f7fa5d6dc01d79c40cec9725ab97f514cb) )    /* palette green component */
	ROM_LOAD( "ic23_3-3.bin", 0x0200, 0x0100, CRC(a488d761) SHA1(6dade1dd16905b4751778d49f374936795c3fb6e) )    /* palette blue component */
ROM_END

/* was marked as Wiz Alt Sound */
ROM_START( wizta )
	ROM_REGION( 0xc000, "maincpu", 0 )
	ROM_LOAD( "ic7",  0x0000, 0x4000, CRC(b2ec49ad) SHA1(f1624995e9d426dd69d6567a91713aa023e716ad) )
	ROM_LOAD( "ic5",  0x4000, 0x4000, CRC(f7e8f792) SHA1(2c1f82e6f6b2573d8bf0a27fb344fe86f475ddef) )
	ROM_LOAD( "ic6",  0x8000, 0x4000, CRC(9c406ad2) SHA1(cd82c3dc622886b6ebb30ba565f3c34d5a4e229b) )

	ROM_REGION( 0x2000, "audiocpu", 0 )
	ROM_LOAD( "ic57",  0x0000, 0x2000, CRC(8a7575bd) SHA1(5470c4c3a40139f45db7a9e260f40b5244f10123) )

	ROM_REGION( 0x6000,  "gfx1", 0 )    /* sprites/chars */
	ROM_LOAD( "ic12",     0x0000, 0x2000, CRC(e6c636b3) SHA1(0d5b98d404d2d87f375cde5d5a90c7d6318ea197) )
	ROM_LOAD( "ic13",     0x2000, 0x2000, CRC(77986058) SHA1(8002affdd9ac246a0b9c887654d0db8d3a6913b2) )
	ROM_LOAD( "ic14",     0x4000, 0x2000, CRC(f6970b23) SHA1(82d1fe0fee6bf9c6c2f472ed3479c02da85d5f69) )

	ROM_REGION( 0xc000,  "gfx2", 0 )    /* sprites/chars */
	ROM_LOAD( "ic3",      0x0000, 0x2000, CRC(601f2f3f) SHA1(6c0cc7de5fd94628eaecca409c4faa155f684bdc) )
	ROM_CONTINUE(         0x6000, 0x2000 )
	ROM_LOAD( "ic2",      0x2000, 0x2000, CRC(f5ab982d) SHA1(5e0e72ec702dd5f48814a15f1a92bcdd29c944d8) )
	ROM_CONTINUE(         0x8000, 0x2000 )
	ROM_LOAD( "ic1",      0x4000, 0x2000, CRC(f6c662e2) SHA1(54e904d731ea30f532dfea60d47edf2da99f32eb) )
	ROM_CONTINUE(         0xa000, 0x2000 )

	ROM_REGION( 0x0300, "proms", 0 )
	ROM_LOAD( "ic23_3-1.bin", 0x0000, 0x0100, CRC(2dd52fb2) SHA1(61722aba7a370f4a97cafbd5df88ec7c6263c4ad) )    /* palette red component */
	ROM_LOAD( "ic23_3-2.bin", 0x0100, 0x0100, CRC(8c2880c9) SHA1(9b4c17f7fa5d6dc01d79c40cec9725ab97f514cb) )    /* palette green component */
	ROM_LOAD( "ic23_3-3.bin", 0x0200, 0x0100, CRC(a488d761) SHA1(6dade1dd16905b4751778d49f374936795c3fb6e) )    /* palette blue component */
ROM_END

ROM_START( stinger )
	ROM_REGION( 0xc000, "maincpu", 0 )
	ROM_LOAD( "1-5j.bin",     0x0000, 0x2000, CRC(1a2ca600) SHA1(473e89f2c49f6e6f38df5d6fc2267ffecf84c6c8) )    /* encrypted */
	ROM_LOAD( "2-6j.bin",     0x2000, 0x2000, CRC(957cd39c) SHA1(38bb589b3bfd962415b31d1151adf4bdb661122f) )    /* encrypted */
	ROM_LOAD( "3-8j.bin",     0x4000, 0x2000, CRC(404c932e) SHA1(c23eac49e06ff38564062c0e8c8cdadf877f1d6a) )    /* encrypted */
	ROM_LOAD( "4-9j.bin",     0x6000, 0x2000, CRC(2d570f91) SHA1(31d54d9fd5254c33f07c605bd6112c7eb53c42a1) )    /* encrypted */
	ROM_LOAD( "5-10j.bin",    0x8000, 0x2000, CRC(c841795c) SHA1(e03860813c03ca1c737935accc2b5fe87c6b624a) )    /* encrypted */

	ROM_REGION( 0x2000, "audiocpu", 0 )
	ROM_LOAD( "6-9f.bin",     0x0000, 0x2000, CRC(79757f0c) SHA1(71be938c32c6a84618763761786ecc5d7d47581a) )

	ROM_REGION( 0x6000,  "gfx1", 0 )    /* sprites/chars */
	ROM_LOAD( "7-9e.bin",     0x0000, 0x2000, CRC(775489be) SHA1(5fccede323895626cf2eabd606ed21282aa36356) )
	ROM_LOAD( "8-11e.bin",    0x2000, 0x2000, CRC(43c61b3f) SHA1(5cdb6a5096b42406c2f2784d37e4e39207c35d40) )
	ROM_LOAD( "9-14e.bin",    0x4000, 0x2000, CRC(c9ed8fc7) SHA1(259d7681b663adb1c5fe057e2ef08469ddcbd3c3) )

	ROM_REGION( 0x6000,  "gfx2", 0 )    /* sprites/chars */
	ROM_LOAD( "10-9h.bin",    0x0000, 0x2000, CRC(6fc3a22d) SHA1(6875b86d60a06aa329d8ff18d0eb48d158074c5d) )
	ROM_LOAD( "11-11h.bin",   0x2000, 0x2000, CRC(3df1f57e) SHA1(e365ee4cc8c055cc39abb4598ad80597d3ae19c7) )
	ROM_LOAD( "12-14h.bin",   0x4000, 0x2000, CRC(2fbe1391) SHA1(669edc154164944d82dfccda328774ea4a2318ba) )

	ROM_REGION( 0x0300,  "proms", 0 )
	ROM_LOAD( "stinger.a7",   0x0000, 0x0100, CRC(52c06fc2) SHA1(b416077fcfabe0dbb1ca30752de6a219ea896f75) )    /* red component */
	ROM_LOAD( "stinger.b7",   0x0100, 0x0100, CRC(9985e575) SHA1(b0d609968917121325760f8d4777066abdb7ccfc) )    /* green component */
	ROM_LOAD( "stinger.a8",   0x0200, 0x0100, CRC(76b57629) SHA1(836763948753b7fed97c9e5d90a16dc4ba68f42a) )    /* blue component */
ROM_END

ROM_START( stinger2 )
	ROM_REGION( 0xc000, "maincpu", 0 )
	ROM_LOAD( "n1.bin",       0x0000, 0x2000, CRC(f2d2790c) SHA1(0e5e92ef45b5bc27b0818f83c89b3bda0e701403) )    /* encrypted */
	ROM_LOAD( "n2.bin",       0x2000, 0x2000, CRC(8fd2d8d8) SHA1(d3318a81fddeb3fa50d01569c1e1145e26ce7277) )    /* encrypted */
	ROM_LOAD( "n3.bin",       0x4000, 0x2000, CRC(f1794d36) SHA1(7954500f489c0bc58cda8e7ffc2e4474759fdc33) )    /* encrypted */
	ROM_LOAD( "n4.bin",       0x6000, 0x2000, CRC(230ba682) SHA1(c419ffebd021d41b3f5021948007fb6bcdb1cdf7) )    /* encrypted */
	ROM_LOAD( "n5.bin",       0x8000, 0x2000, CRC(a03a01da) SHA1(28fecac7a821ac4718242919840266a907160df0) )    /* encrypted */

	ROM_REGION( 0x2000, "audiocpu", 0 )
	ROM_LOAD( "6-9f.bin",     0x0000, 0x2000, CRC(79757f0c) SHA1(71be938c32c6a84618763761786ecc5d7d47581a) )

	ROM_REGION( 0x6000,  "gfx1", 0 )    /* sprites/chars */
	ROM_LOAD( "7-9e.bin",     0x0000, 0x2000, CRC(775489be) SHA1(5fccede323895626cf2eabd606ed21282aa36356) )
	ROM_LOAD( "8-11e.bin",    0x2000, 0x2000, CRC(43c61b3f) SHA1(5cdb6a5096b42406c2f2784d37e4e39207c35d40) )
	ROM_LOAD( "9-14e.bin",    0x4000, 0x2000, CRC(c9ed8fc7) SHA1(259d7681b663adb1c5fe057e2ef08469ddcbd3c3) )

	ROM_REGION( 0x6000,  "gfx2", 0 )    /* sprites/chars */
	ROM_LOAD( "10.bin",       0x0000, 0x2000, CRC(f6721930) SHA1(fb903f1deb5f093ff5fe129e213966af58a68339) )
	ROM_LOAD( "11.bin",       0x2000, 0x2000, CRC(a4404e63) SHA1(50ae99748547af20e04f6c6c8c7eba85f967b9dc) )
	ROM_LOAD( "12.bin",       0x4000, 0x2000, CRC(b60fa88c) SHA1(2d3bca35076625251933989f5e566d5d3290542b) )

	ROM_REGION( 0x0300,  "proms", 0 )
	ROM_LOAD( "stinger.a7",   0x0000, 0x0100, CRC(52c06fc2) SHA1(b416077fcfabe0dbb1ca30752de6a219ea896f75) )    /* red component */
	ROM_LOAD( "stinger.b7",   0x0100, 0x0100, CRC(9985e575) SHA1(b0d609968917121325760f8d4777066abdb7ccfc) )    /* green component */
	ROM_LOAD( "stinger.a8",   0x0200, 0x0100, CRC(76b57629) SHA1(836763948753b7fed97c9e5d90a16dc4ba68f42a) )    /* blue component */
ROM_END

ROM_START( scion )
	ROM_REGION( 0xc000, "maincpu", 0 )
	ROM_LOAD( "sc1",          0x0000, 0x2000, CRC(8dcad575) SHA1(3f194ece25e730b1cbbf3f332bbdebc3a6a72b0f) )
	ROM_LOAD( "sc2",          0x2000, 0x2000, CRC(f608e0ba) SHA1(e55b0ad4dc117339d45a999e13760f4ab3ca4ce0) )
	ROM_LOAD( "sc3",          0x4000, 0x2000, CRC(915289b9) SHA1(b32b40f93de4501619486a8c5a8367d3b2e357a6) )
	ROM_LOAD( "4.9j",         0x6000, 0x2000, CRC(0f40d002) SHA1(13b04f3902ebdda02670fcb667e181cf70594c37) )
	ROM_LOAD( "5.10j",        0x8000, 0x2000, CRC(dc4923b7) SHA1(ec5c22ef1f9ba0fe4da3de62a63a44aa3ff850f4) )

	ROM_REGION( 0x2000, "audiocpu", 0 )
	ROM_LOAD( "sc6",          0x0000, 0x2000, CRC(09f5f9c1) SHA1(83e489f32597880fb1a13f0bafedd275facb21f7) )

	ROM_REGION( 0x6000,  "gfx1", 0 )    /* sprites/chars */
	ROM_LOAD( "7.10e",        0x0000, 0x2000, CRC(223e0d2a) SHA1(073638172ce0762d103cc07705fc493432e5aa63) )
	ROM_LOAD( "8.12e",        0x2000, 0x2000, CRC(d3e39b48) SHA1(c686ef35bf866d044637df295bb70c9c005fc98c) )
	ROM_LOAD( "9.15e",        0x4000, 0x2000, CRC(630861b5) SHA1(a6ccfa10e43e92407c452f9744aa1735b257c28e) )

	ROM_REGION( 0x6000,  "gfx2", 0 )    /* sprites/chars */
	ROM_LOAD( "10.10h",       0x0000, 0x2000, CRC(0d2a0d1e) SHA1(518689f91019e64138ed3560e161d3ef93d0671d) )
	ROM_LOAD( "11.12h",       0x2000, 0x2000, CRC(dc6ef8ab) SHA1(ba93392a494a66336197d28e45832b9f8f3e4376) )
	ROM_LOAD( "12.15h",       0x4000, 0x2000, CRC(c82c28bf) SHA1(8952b515f01027a94bee0186221a1989ea2cd919) )

	ROM_REGION( 0x0300,  "proms", 0 )
	ROM_LOAD( "82s129.7a",    0x0000, 0x0100, CRC(2f89d9ea) SHA1(37adbddb9b3253b995a02a74e0de27ad594dc544) )    /* red component */
	ROM_LOAD( "82s129.7b",    0x0100, 0x0100, CRC(ba151e6a) SHA1(3d3139936de9e1913dee94317420a171bd3d2062) )    /* green component */
	ROM_LOAD( "82s129.8a",    0x0200, 0x0100, CRC(f681ce59) SHA1(4ac74c1d04e6b3f14a0f4530a41ba188f5a8f6be) )    /* blue component */
ROM_END

ROM_START( scionc )
	ROM_REGION( 0xc000, "maincpu", 0 )
	ROM_LOAD( "1.5j",         0x0000, 0x2000, CRC(5aaf571e) SHA1(53becfad13e95012dce6597625c64dcba9ac4433) )
	ROM_LOAD( "2.6j",         0x2000, 0x2000, CRC(d5a66ac9) SHA1(3192da12b2d6a07e203999ed97cdba16d4917a98) )
	ROM_LOAD( "3.8j",         0x4000, 0x2000, CRC(6e616f28) SHA1(ea32add6173251152ca84426c098c92ace123878) )
	ROM_LOAD( "4.9j",         0x6000, 0x2000, CRC(0f40d002) SHA1(13b04f3902ebdda02670fcb667e181cf70594c37) )
	ROM_LOAD( "5.10j",        0x8000, 0x2000, CRC(dc4923b7) SHA1(ec5c22ef1f9ba0fe4da3de62a63a44aa3ff850f4) )

	ROM_REGION( 0x2000, "audiocpu", 0 )
	ROM_LOAD( "6.9f",         0x0000, 0x2000, CRC(a66a0ce6) SHA1(b2d6a8ded007c362c58496ead33d1561a982440a) )

	ROM_REGION( 0x6000,  "gfx1", 0 )    /* sprites/chars */
	ROM_LOAD( "7.10e",        0x0000, 0x2000, CRC(223e0d2a) SHA1(073638172ce0762d103cc07705fc493432e5aa63) )
	ROM_LOAD( "8.12e",        0x2000, 0x2000, CRC(d3e39b48) SHA1(c686ef35bf866d044637df295bb70c9c005fc98c) )
	ROM_LOAD( "9.15e",        0x4000, 0x2000, CRC(630861b5) SHA1(a6ccfa10e43e92407c452f9744aa1735b257c28e) )

	ROM_REGION( 0x6000,  "gfx2", 0 )    /* sprites/chars */
	ROM_LOAD( "10.10h",       0x0000, 0x2000, CRC(0d2a0d1e) SHA1(518689f91019e64138ed3560e161d3ef93d0671d) )
	ROM_LOAD( "11.12h",       0x2000, 0x2000, CRC(dc6ef8ab) SHA1(ba93392a494a66336197d28e45832b9f8f3e4376) )
	ROM_LOAD( "12.15h",       0x4000, 0x2000, CRC(c82c28bf) SHA1(8952b515f01027a94bee0186221a1989ea2cd919) )

	ROM_REGION( 0x0300,  "proms", 0 )
	ROM_LOAD( "82s129.7a",    0x0000, 0x0100, CRC(2f89d9ea) SHA1(37adbddb9b3253b995a02a74e0de27ad594dc544) )    /* red component */
	ROM_LOAD( "82s129.7b",    0x0100, 0x0100, CRC(ba151e6a) SHA1(3d3139936de9e1913dee94317420a171bd3d2062) )    /* green component */
	ROM_LOAD( "82s129.8a",    0x0200, 0x0100, CRC(f681ce59) SHA1(4ac74c1d04e6b3f14a0f4530a41ba188f5a8f6be) )    /* blue component */
ROM_END


DRIVER_INIT_MEMBER(wiz_state,stinger)
{
	static const UINT8 swap_xor_table[4][4] =
	{
		{ 7,3,5, 0xa0 },
		{ 3,7,5, 0x88 },
		{ 5,3,7, 0x80 },
		{ 5,7,3, 0x28 }
	};

	UINT8 *rom = memregion("maincpu")->base();
	const UINT8 *tbl;

	for (int a = 0x0000; a < 0xc000; a++)
	{
		int row;
		UINT8 src;

		if (a & 0x2040)
		{
			/* not encrypted */
			m_decrypted_opcodes[a] = rom[a];
		}
		else
		{
			src = rom[a];

			/* pick the translation table from bits 3 and 5 of the address */
			row = ((a >> 3) & 1) + (((a >> 5) & 1) << 1);

			/* decode the opcodes */
			tbl = swap_xor_table[row];
			m_decrypted_opcodes[a] = BITSWAP8(src, tbl[0], 6, tbl[1], 4, tbl[2], 2, 1, 0) ^ tbl[3];
		}
	}
}


GAME( 1983, stinger,  0,       stinger, stinger,  wiz_state,     stinger, ROT90,  "Seibu Denshi", "Stinger", MACHINE_IMPERFECT_SOUND | MACHINE_IMPERFECT_COLORS | MACHINE_SUPPORTS_SAVE )
GAME( 1983, stinger2, stinger, stinger, stinger2, wiz_state,     stinger, ROT90,  "Seibu Denshi", "Stinger (prototype?)", MACHINE_IMPERFECT_SOUND | MACHINE_IMPERFECT_COLORS | MACHINE_SUPPORTS_SAVE )
GAME( 1984, scion,    0,       scion,   scion,    driver_device, 0,       ROT0,   "Seibu Denshi", "Scion", MACHINE_IMPERFECT_SOUND | MACHINE_IMPERFECT_COLORS | MACHINE_SUPPORTS_SAVE )
GAME( 1984, scionc,   scion,   scion,   scion,    driver_device, 0,       ROT0,   "Seibu Denshi (Cinematronics license)", "Scion (Cinematronics)", MACHINE_IMPERFECT_SOUND | MACHINE_IMPERFECT_COLORS | MACHINE_SUPPORTS_SAVE )
GAME( 1984, kungfut,  0,       kungfut, kungfut,  driver_device, 0,       ROT0,   "Seibu Kaihatsu", "Kung-Fu Taikun (set 1)", MACHINE_SUPPORTS_SAVE )
GAME( 1984, kungfuta, kungfut, kungfut, kungfut,  driver_device, 0,       ROT0,   "Seibu Kaihatsu", "Kung-Fu Taikun (set 2)", MACHINE_SUPPORTS_SAVE ) /* board was a bootleg but set might still be original */
GAME( 1985, wiz,      0,       wiz,     wiz,      driver_device, 0,       ROT270, "Seibu Kaihatsu", "Wiz", MACHINE_SUPPORTS_SAVE )
GAME( 1985, wizt,     wiz,     wiz,     wiz,      driver_device, 0,       ROT270, "Seibu Kaihatsu (Taito license)", "Wiz (Taito, set 1)", MACHINE_SUPPORTS_SAVE )
GAME( 1985, wizta,    wiz,     wiz,     wiz,      driver_device, 0,       ROT270, "Seibu Kaihatsu (Taito license)", "Wiz (Taito, set 2)", MACHINE_SUPPORTS_SAVE )
