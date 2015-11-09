// license:BSD-3-Clause
// copyright-holders:Brad Oliver, Bernd Wiebelt, Allard van der Bas
/***************************************************************************

    Atari Asteroids hardware

    Games supported:
        * Asteroids
        * Asteroids Deluxe
        * Lunar Lander

    Known bugs:
        * the ERROR message in Asteroids Deluxe self test is related to a pokey problem

Information from a Tech Tip:

Asteroids Modification:

As a result of inquires regarding the achievement of extremely high scores on Asteroids,
we have developed a modification that will accomplish the following:
  When the small flying saucer enters the screen, it fires immediately in the direction
    of the player. The missiles also have a wraparound capability.
  Originally, the program was entered so that the saucer would go one-sixth of the way
    across the screen before firing, without the wraparound capability.

There are two revisions currently in the field, ROM revision 02 has the flip-flop capability,
  version 01 does not.
Kits are available immediately from your Atari Distributor.  To determine which kit you
  require, check ROMs on your Asteroids PCB.  If ROM code is "-01" order kit no. 08-0303009.
  If ROM code is "-02", order no. 08-0303008.

For kit 08-0303009 (from rev 01) swap the following:

 035127-01 --> 035127-02
 035143-01 --> 035143-02
 035144-01 --> 034144-04E
 035145-01 --> 034145-04E

For kit 08-0303008 (from rev 02) swap the following:

 035144-02 --> 034144-04E
 035145-02 --> 034145-04E

  A rom set was found labeled:

    035144-02-03
    035145-02-03

  However, when dumped they matched the rev 02 set.

There is not a rev 03 known or dumped. An Asteroids rev 03 is not mentioned in any known Atari docs found to date.

    Asteroids-deluxe state-prom added by HIGHWAYMAN.
    The prom pcb location is:C8 and is 256x4
    (i need to update the dump, this one is read in 8bit-mode)
****************************************************************************

    Asteroids Memory Map (preliminary)

    Asteroids settings:

    0 = OFF  1 = ON  X = Don't Care  $ = Atari suggests


    8 SWITCH DIP
    87654321
    --------
    XXXXXX11   English
    XXXXXX10   German
    XXXXXX01   French
    XXXXXX00   Spanish
    XXXXX1XX   4-ship game
    XXXXX0XX   3-ship game
    11XXXXXX   Free Play
    10XXXXXX   1 Coin  for 2 Plays
    01XXXXXX   1 Coin  for 1 Play
    00XXXXXX   2 Coins for 1 Play

    Asteroids Deluxe settings:

    0 = OFF  1 = ON  X = Don't Care  $ = Atari suggests


    8 SWITCH DIP (R5)
    87654321
    --------
    XXXXXX11   English $
    XXXXXX10   German
    XXXXXX01   French
    XXXXXX00   Spanish
    XXXX11XX   2-4 ships
    XXXX10XX   3-5 ships $
    XXXX01XX   4-6 ships
    XXXX00XX   5-7 ships
    XXX1XXXX   1-play minimum $
    XXX0XXXX   2-play minimum
    XX1XXXXX   Easier gameplay for first 30000 points +
    XX0XXXXX   Hard gameplay throughout the game      +
    11XXXXXX   Bonus ship every 10,000 points $ !
    10XXXXXX   Bonus ship every 12,000 points !
    01XXXXXX   Bonus ship every 15,000 points !
    00XXXXXX   No bonus ships (adds one ship at game start)

    + only with the newer romset
    ! not "every", but "at", e.g. only once.

    Thanks to Gregg Woodcock for the info.

    8 SWITCH DIP (L8)
    87654321
    --------
    XXXXXX11   Free Play
    XXXXXX10   1 Coin = 2 Plays
    XXXXXX01   1 Coin = 1 Play
    XXXXXX00   2 Coins = 1 Play $
    XXXX11XX   Right coin mech * 1 $
    XXXX10XX   Right coin mech * 4
    XXXX01XX   Right coin mech * 5
    XXXX00XX   Right coin mech * 6
    XXX1XXXX   Center coin mech * 1 $
    XXX0XXXX   Center coin mech * 2
    111XXXXX   No bonus coins
    110XXXXX   For every 2 coins inserted, game logic adds 1 more coin
    101XXXXX   For every 4 coins inserted, game logic adds 1 more coin
    100XXXXX   For every 4 coins inserted, game logic adds 2 more coins $
    011XXXXX   For every 5 coins inserted, game logic adds 1 more coin

****************************************************************************

    Lunar Lander Memory Map (preliminary)

    Lunar Lander settings:

    0 = OFF  1 = ON  x = Don't Care  $ = Atari suggests


    8 SWITCH DIP (P8) with -01 ROMs on PCB
    87654321
    --------
    11xxxxxx   450 fuel units per coin
    10xxxxxx   600 fuel units per coin
    01xxxxxx   750 fuel units per coin  $
    00xxxxxx   900 fuel units per coin
    xxx0xxxx   Free play
    xxx1xxxx   Coined play as determined by toggles 7 & 8  $
    xxxx00xx   German instructions
    xxxx01xx   Spanish instructions
    xxxx10xx   French instructions
    xxxx11xx   English instructions  $
    xxxxxx11   Right coin == 1 credit/coin  $
    xxxxxx10   Right coin == 4 credit/coin
    xxxxxx01   Right coin == 5 credit/coin
    xxxxxx00   Right coin == 6 credit/coin
               (Left coin always registers 1 credit/coin)


    8 SWITCH DIP (P8) with -02 ROMs on PCB
    87654321
    --------
    11x1xxxx   450 fuel units per coin
    10x1xxxx   600 fuel units per coin
    01x1xxxx   750 fuel units per coin  $
    00x1xxxx   900 fuel units per coin
    11x0xxxx   1100 fuel units per coin
    10x0xxxx   1300 fuel units per coin
    01x0xxxx   1550 fuel units per coin
    00x0xxxx   1800 fuel units per coin
    xx0xxxxx   Free play
    xx1xxxxx   Coined play as determined by toggles 5, 7, & 8  $
    xxxx00xx   German instructions
    xxxx01xx   Spanish instructions
    xxxx10xx   French instructions
    xxxx11xx   English instructions  $
    xxxxxx11   Right coin == 1 credit/coin  $
    xxxxxx10   Right coin == 4 credit/coin
    xxxxxx01   Right coin == 5 credit/coin
    xxxxxx00   Right coin == 6 credit/coin
               (Left coin always registers 1 credit/coin)

   DIP locations verified from manual for:
    - asteroid
    - llander
    - llander1
    - astdelux

***************************************************************************/

#include "emu.h"
#include "cpu/m6502/m6502.h"
#include "video/vector.h"
#include "video/avgdvg.h"
#include "machine/atari_vg.h"
#include "includes/asteroid.h"
#include "sound/discrete.h"
#include "sound/pokey.h"

#include "astdelux.lh"

#define MASTER_CLOCK (XTAL_12_096MHz)
#define CLOCK_3KHZ   ((double)MASTER_CLOCK / 4096)

/*************************************
 *
 *  Coin counters
 *
 *************************************/

WRITE8_MEMBER(asteroid_state::astdelux_coin_counter_w)
{
	coin_counter_w(machine(), offset,data);
}



/*************************************
 *
 *  Lunar Lander LEDs/lamps
 *
 *************************************/

WRITE8_MEMBER(asteroid_state::llander_led_w)
{
	static const char *const lampname[] =
	{
		"lamp0", "lamp1", "lamp2", "lamp3", "lamp4"
	};

	for (int i = 0; i < 5; i++)
		output_set_value(lampname[i], (data >> (4 - i)) & 1);
}


/*************************************
 *
 *  Main CPU memory handlers
 *
 *************************************/

static ADDRESS_MAP_START( asteroid_map, AS_PROGRAM, 8, asteroid_state )
	ADDRESS_MAP_GLOBAL_MASK(0x7fff)
	AM_RANGE(0x0000, 0x01ff) AM_RAM
	AM_RANGE(0x0200, 0x02ff) AM_RAMBANK("ram1") AM_SHARE("ram1")
	AM_RANGE(0x0300, 0x03ff) AM_RAMBANK("ram2") AM_SHARE("ram2")
	AM_RANGE(0x2000, 0x2007) AM_READ(asteroid_IN0_r)    /* IN0 */
	AM_RANGE(0x2400, 0x2407) AM_READ(asteroid_IN1_r)    /* IN1 */
	AM_RANGE(0x2800, 0x2803) AM_READ(asteroid_DSW1_r)   /* DSW1 */
	AM_RANGE(0x3000, 0x3000) AM_DEVWRITE("dvg", dvg_device, go_w)
	AM_RANGE(0x3200, 0x3200) AM_WRITE(asteroid_bank_switch_w)
	AM_RANGE(0x3400, 0x3400) AM_WRITE(watchdog_reset_w)
	AM_RANGE(0x3600, 0x3600) AM_WRITE(asteroid_explode_w)
	AM_RANGE(0x3a00, 0x3a00) AM_WRITE(asteroid_thump_w)
	AM_RANGE(0x3c00, 0x3c05) AM_WRITE(asteroid_sounds_w)
	AM_RANGE(0x3e00, 0x3e00) AM_WRITE(asteroid_noise_reset_w)
	AM_RANGE(0x4000, 0x47ff) AM_RAM AM_SHARE("vectorram") AM_REGION("maincpu", 0x4000)
	AM_RANGE(0x5000, 0x57ff) AM_ROM                     /* vector rom */
	AM_RANGE(0x6800, 0x7fff) AM_ROM
ADDRESS_MAP_END


static ADDRESS_MAP_START( astdelux_map, AS_PROGRAM, 8, asteroid_state )
	ADDRESS_MAP_GLOBAL_MASK(0x7fff)
	AM_RANGE(0x0000, 0x01ff) AM_RAM
	AM_RANGE(0x0200, 0x02ff) AM_RAMBANK("ram1") AM_SHARE("ram1")
	AM_RANGE(0x0300, 0x03ff) AM_RAMBANK("ram2") AM_SHARE("ram2")
	AM_RANGE(0x2000, 0x2007) AM_READ(asteroid_IN0_r)    /* IN0 */
	AM_RANGE(0x2400, 0x2407) AM_READ(asteroid_IN1_r)    /* IN1 */
	AM_RANGE(0x2800, 0x2803) AM_READ(asteroid_DSW1_r)   /* DSW1 */
	AM_RANGE(0x2c00, 0x2c0f) AM_DEVREADWRITE("pokey", pokey_device, read, write)
	AM_RANGE(0x2c40, 0x2c7f) AM_DEVREAD("earom", atari_vg_earom_device, read)
	AM_RANGE(0x3000, 0x3000) AM_DEVWRITE("dvg", dvg_device, go_w)
	AM_RANGE(0x3200, 0x323f) AM_DEVWRITE("earom", atari_vg_earom_device, write)
	AM_RANGE(0x3400, 0x3400) AM_WRITE(watchdog_reset_w)
	AM_RANGE(0x3600, 0x3600) AM_WRITE(asteroid_explode_w)
	AM_RANGE(0x3a00, 0x3a00) AM_DEVWRITE("earom", atari_vg_earom_device, ctrl_w)
	AM_RANGE(0x3c00, 0x3c01) AM_WRITE(astdelux_led_w)
	AM_RANGE(0x3c03, 0x3c03) AM_WRITE(astdelux_sounds_w)
	AM_RANGE(0x3c04, 0x3c04) AM_WRITE(astdelux_bank_switch_w)
	AM_RANGE(0x3c05, 0x3c07) AM_WRITE(astdelux_coin_counter_w)
	AM_RANGE(0x3e00, 0x3e00) AM_WRITE(asteroid_noise_reset_w)
	AM_RANGE(0x4000, 0x47ff) AM_RAM AM_SHARE("vectorram") AM_REGION("maincpu", 0x4000)
	AM_RANGE(0x4800, 0x57ff) AM_ROM                     /* vector rom */
	AM_RANGE(0x6000, 0x7fff) AM_ROM
ADDRESS_MAP_END


static ADDRESS_MAP_START( llander_map, AS_PROGRAM, 8, asteroid_state )
	ADDRESS_MAP_GLOBAL_MASK(0x7fff)
	AM_RANGE(0x0000, 0x00ff) AM_RAM AM_MIRROR(0x1f00)
	AM_RANGE(0x2000, 0x2000) AM_READ_PORT("IN0")
	AM_RANGE(0x2400, 0x2407) AM_READ(asteroid_IN1_r)    /* IN1 */
	AM_RANGE(0x2800, 0x2803) AM_READ(asteroid_DSW1_r)   /* DSW1 */
	AM_RANGE(0x2c00, 0x2c00) AM_READ_PORT("THRUST")
	AM_RANGE(0x3000, 0x3000) AM_DEVWRITE("dvg", dvg_device, go_w)
	AM_RANGE(0x3200, 0x3200) AM_WRITE(llander_led_w)
	AM_RANGE(0x3400, 0x3400) AM_WRITE(watchdog_reset_w)
	AM_RANGE(0x3c00, 0x3c00) AM_WRITE(llander_sounds_w)
	AM_RANGE(0x3e00, 0x3e00) AM_WRITE(llander_snd_reset_w)
	AM_RANGE(0x4000, 0x47ff) AM_RAM AM_SHARE("vectorram") AM_REGION("maincpu", 0x4000)
	AM_RANGE(0x4800, 0x5fff) AM_ROM                     /* vector rom */
	AM_RANGE(0x6000, 0x7fff) AM_ROM
ADDRESS_MAP_END



/*************************************
 *
 *  Port definitions
 *
 *************************************/

CUSTOM_INPUT_MEMBER(asteroid_state::clock_r)
{
	return (m_maincpu->total_cycles() & 0x100) ? 1 : 0;
}

static INPUT_PORTS_START( asteroid )
	PORT_START("IN0")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	/* Bit 2 is the 3 KHz source and Bit 3 the VG_HALT bit    */
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_SPECIAL ) PORT_CUSTOM_MEMBER(DEVICE_SELF, asteroid_state,clock_r, NULL)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_SPECIAL ) PORT_CUSTOM_MEMBER("dvg", dvg_device, done_r, NULL)
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_BUTTON5 ) PORT_CODE(KEYCODE_SPACE) PORT_CODE(JOYCODE_BUTTON3)       /* hyperspace */
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_BUTTON3 ) PORT_CODE(KEYCODE_LCONTROL) PORT_CODE(JOYCODE_BUTTON1)    /* fire */
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_SERVICE ) PORT_NAME("Diagnostic Step") PORT_CODE(KEYCODE_F1)
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_TILT )
	PORT_SERVICE( 0x80, IP_ACTIVE_HIGH )

	PORT_START("IN1")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_COIN1 )
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_COIN2 )
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_COIN3 )
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_START1 )
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_START2 )
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_BUTTON4 ) PORT_CODE(KEYCODE_LALT) PORT_CODE(JOYCODE_BUTTON2)        /* thrust */
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_BUTTON2 ) PORT_CODE(KEYCODE_RIGHT) PORT_CODE(JOYCODE_X_RIGHT_SWITCH)/* right */
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_CODE(KEYCODE_LEFT) PORT_CODE(JOYCODE_X_LEFT_SWITCH)  /* left */

	PORT_START("DSW1")
	PORT_DIPNAME( 0x03, 0x00, DEF_STR( Language ) ) PORT_DIPLOCATION("SW:1,2")
	PORT_DIPSETTING (   0x00, DEF_STR( English ) )
	PORT_DIPSETTING (   0x01, DEF_STR( German ) )
	PORT_DIPSETTING (   0x02, DEF_STR( French ) )
	PORT_DIPSETTING (   0x03, DEF_STR( Spanish ) )
	PORT_DIPNAME( 0x04, 0x04, DEF_STR( Lives ) )    PORT_DIPLOCATION("SW:3")
	PORT_DIPSETTING (   0x04, "3" )
	PORT_DIPSETTING (   0x00, "4" )
	PORT_DIPNAME( 0x08, 0x00, "Center Mech" )       PORT_DIPLOCATION("SW:4") /* Left/Center for 3-door mech */
	PORT_DIPSETTING (   0x00, "X 1" )
	PORT_DIPSETTING (   0x08, "X 2" )
	PORT_DIPNAME( 0x30, 0x00, "Right Mech" )        PORT_DIPLOCATION("SW:5,6")
	PORT_DIPSETTING (   0x00, "X 1" )
	PORT_DIPSETTING (   0x10, "X 4" )
	PORT_DIPSETTING (   0x20, "X 5" )
	PORT_DIPSETTING (   0x30, "X 6" )
	PORT_DIPNAME( 0xc0, 0x80, DEF_STR( Coinage ) )  PORT_DIPLOCATION("SW:7,8")
	PORT_DIPSETTING (   0xc0, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING (   0x80, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING (   0x40, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING (   0x00, DEF_STR( Free_Play ) )
INPUT_PORTS_END


static INPUT_PORTS_START( asteroidb )
	PORT_INCLUDE( asteroid )

	PORT_MODIFY("IN0")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNKNOWN )    /* resets */
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNKNOWN )    /* resets */
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_TILT )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	/* Bit 7 is VG_HALT */
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_SPECIAL ) PORT_CUSTOM_MEMBER("dvg", dvg_device, done_r, NULL)

	PORT_MODIFY("IN1")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_START1 )
	PORT_BIT( 0x02, IP_ACTIVE_LOW,  IPT_COIN1 )
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_BUTTON4 ) PORT_CODE(KEYCODE_LALT) PORT_CODE(JOYCODE_BUTTON2)            /* thrust */
	PORT_BIT( 0x08, IP_ACTIVE_LOW,  IPT_COIN2 )
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_BUTTON3 ) PORT_CODE(KEYCODE_LCONTROL) PORT_CODE(JOYCODE_BUTTON1)        /* fire */

	PORT_MODIFY("DSW1")
	PORT_DIPUNKNOWN_DIPLOC( 0x08, 0x08, "SW:4" )
	PORT_DIPUNKNOWN_DIPLOC( 0x10, 0x10, "SW:5" )
	PORT_DIPUNKNOWN_DIPLOC( 0x20, 0x20, "SW:6" )

	PORT_START("HS") /* hyperspace */
	PORT_BIT( 0x7f, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_BUTTON5 ) PORT_CODE(KEYCODE_SPACE) PORT_CODE(JOYCODE_BUTTON3)            /* hyperspace */
INPUT_PORTS_END


static INPUT_PORTS_START( aerolitos )
	PORT_INCLUDE( asteroid )

	PORT_MODIFY("DSW1") // this bootleg was for the Spanish market, so set it to Spanish by default
	PORT_DIPNAME( 0x03, 0x03, DEF_STR( Language ) ) PORT_DIPLOCATION("SW:1,2")
	PORT_DIPSETTING (   0x00, DEF_STR( English ) )
	PORT_DIPSETTING (   0x01, DEF_STR( German ) )
	PORT_DIPSETTING (   0x02, DEF_STR( French ) )
	PORT_DIPSETTING (   0x03, DEF_STR( Spanish ) )
INPUT_PORTS_END


static INPUT_PORTS_START( asterock )
	PORT_INCLUDE( asteroid )

	PORT_MODIFY("IN0")
	/* Bit 0 is VG_HALT and Bit 2 is the 3 KHz source */
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_SPECIAL ) PORT_CUSTOM_MEMBER("dvg", dvg_device, done_r, NULL)
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_SPECIAL ) PORT_CUSTOM_MEMBER(DEVICE_SELF, asteroid_state,clock_r, NULL)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_BUTTON5 ) PORT_CODE(KEYCODE_SPACE) PORT_CODE(JOYCODE_BUTTON3)        /* hyperspace */
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_CODE(KEYCODE_LCONTROL) PORT_CODE(JOYCODE_BUTTON1)     /* fire */
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_SERVICE ) PORT_NAME("Diagnostic Step") PORT_CODE(KEYCODE_F1)
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_TILT )
	PORT_SERVICE( 0x80, IP_ACTIVE_LOW )

	PORT_MODIFY("DSW1")
	PORT_DIPNAME( 0x03, 0x03, DEF_STR( Language ) )                 PORT_DIPLOCATION("SW:1,2")
	PORT_DIPSETTING(    0x00, DEF_STR( English ) )
	PORT_DIPSETTING(    0x01, DEF_STR( French ) )
	PORT_DIPSETTING(    0x02, DEF_STR( German ) )
	PORT_DIPSETTING(    0x03, DEF_STR( Italian ) )
	PORT_DIPNAME( 0x0c, 0x04, DEF_STR( Lives ) )                    PORT_DIPLOCATION("SW:3,4")
	PORT_DIPSETTING(    0x00, "2" )
	PORT_DIPSETTING(    0x04, "3" )
	PORT_DIPSETTING(    0x08, "4" )
	PORT_DIPSETTING(    0x0c, "5" )
	PORT_DIPNAME( 0x10, 0x00, "Records Table" )                     PORT_DIPLOCATION("SW:5")
	PORT_DIPSETTING(    0x00, DEF_STR( Normal ) )
	PORT_DIPSETTING(    0x10, "Special" )
	PORT_DIPNAME( 0x20, 0x00, "Coin Mode" )                         PORT_DIPLOCATION("SW:6")
	PORT_DIPSETTING (   0x00, DEF_STR( Normal ) )
	PORT_DIPSETTING (   0x20, "Special" )
	PORT_DIPNAME( 0xc0, 0x80, DEF_STR( Coinage ) )                  PORT_DIPLOCATION("SW:7,8")
	PORT_DIPSETTING (   0xc0, DEF_STR( 2C_1C ) )                    PORT_CONDITION("DSW1",0x20,EQUALS,0x00)
	PORT_DIPSETTING (   0x80, DEF_STR( 1C_1C ) )                    PORT_CONDITION("DSW1",0x20,EQUALS,0x00)
	PORT_DIPSETTING (   0x40, DEF_STR( 1C_2C ) )                    PORT_CONDITION("DSW1",0x20,EQUALS,0x00)
//  PORT_DIPSETTING (   0x00, DEF_STR( 1C_1C ) )                    PORT_CONDITION("DSW1",0x20,EQUALS,0x00)
	PORT_DIPSETTING (   0xc0, "Coin A 2/1 Coin B 2/1 Coin C 1/1" )  PORT_CONDITION("DSW1",0x20,NOTEQUALS,0x00)
	PORT_DIPSETTING (   0x80, "Coin A 1/1 Coin B 1/1 Coin C 1/2" )  PORT_CONDITION("DSW1",0x20,NOTEQUALS,0x00)
	PORT_DIPSETTING (   0x40, "Coin A 1/2 Coin B 1/2 Coin C 1/4" )  PORT_CONDITION("DSW1",0x20,NOTEQUALS,0x00)
//  PORT_DIPSETTING (   0x00, "Coin A 1/1 Coin B 1/1 Coin C 1/2" )  PORT_CONDITION("DSW1",0x20,NOTEQUALS,0x00)
INPUT_PORTS_END


static INPUT_PORTS_START( astdelux )
	PORT_START("IN0")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_UNUSED ) /* According to schematics */
	/* Bit 2 is the 3 KHz source and Bit 3 the VG_HALT bit    */
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_SPECIAL ) PORT_CUSTOM_MEMBER(DEVICE_SELF, asteroid_state,clock_r, NULL)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_SPECIAL ) PORT_CUSTOM_MEMBER("dvg", dvg_device, done_r, NULL)
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_BUTTON5 ) PORT_CODE(KEYCODE_SPACE) PORT_CODE(JOYCODE_BUTTON3)       /* hyperspace */
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_BUTTON3 ) PORT_CODE(KEYCODE_LCONTROL) PORT_CODE(JOYCODE_BUTTON1)    /* fire */
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_SERVICE ) PORT_NAME("Diagnostic Step") PORT_CODE(KEYCODE_F1)
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_TILT )
	PORT_SERVICE( 0x80, IP_ACTIVE_HIGH )

	PORT_START("IN1")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_COIN1 ) /* Coin Left */
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_COIN2 ) /* Coin Center */
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_COIN3 ) /* Coin Right */
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_START1 )
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_START2 )
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_BUTTON4 ) PORT_CODE(KEYCODE_LALT) PORT_CODE(JOYCODE_BUTTON2)        /* thrust */
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_BUTTON2 ) PORT_CODE(KEYCODE_RIGHT) PORT_CODE(JOYCODE_X_RIGHT_SWITCH)/* right */
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_CODE(KEYCODE_LEFT) PORT_CODE(JOYCODE_X_LEFT_SWITCH)  /* left */

	PORT_START("DSW1")
	PORT_DIPNAME( 0x03, 0x00, DEF_STR( Language ) )     PORT_DIPLOCATION("R5:1,2")
	PORT_DIPSETTING (   0x00, DEF_STR( English ) )
	PORT_DIPSETTING (   0x01, DEF_STR( German ) )
	PORT_DIPSETTING (   0x02, DEF_STR( French ) )
	PORT_DIPSETTING (   0x03, DEF_STR( Spanish ) )
	/*  Default lives is 2,3,4,5. Values incremented by 1 if Bonus Life set to None or Coinage set to 2C_1C.
	    Incremented by 2 if both are set at the same time. PORT_CONDITION() can only test for 1 switch at a time. */
	PORT_DIPNAME( 0x0c, 0x00, DEF_STR( Lives ) )        PORT_DIPLOCATION("R5:3,4") /* Default is 2 or 3 depending on manual version */
	PORT_DIPSETTING (   0x00, "2-4" )
	PORT_DIPSETTING (   0x04, "3-5" )
	PORT_DIPSETTING (   0x08, "4-6" )
	PORT_DIPSETTING (   0x0c, "5-7" )
	PORT_DIPNAME( 0x10, 0x00, "Minimum Plays" )         PORT_DIPLOCATION("R5:5")
	PORT_DIPSETTING (   0x00, "1" )
	PORT_DIPSETTING (   0x10, "2" )
	PORT_DIPNAME( 0x20, 0x00, DEF_STR( Difficulty ) )   PORT_DIPLOCATION("R5:6") /* Listed as "Unused" */
	PORT_DIPSETTING (   0x00, DEF_STR( Hard ) )
	PORT_DIPSETTING (   0x20, DEF_STR( Easy ) )
	PORT_DIPNAME( 0xc0, 0x00, DEF_STR( Bonus_Life ) )   PORT_DIPLOCATION("R5:7,8")
	PORT_DIPSETTING (   0x00, "10000" )
	PORT_DIPSETTING (   0x40, "12000" )
	PORT_DIPSETTING (   0x80, "15000" )
	PORT_DIPSETTING (   0xc0, DEF_STR( None ) )

	PORT_START("DSW2")
	PORT_DIPNAME( 0x03, 0x01, DEF_STR( Coinage ) )      PORT_DIPLOCATION("L8:1,2")
	PORT_DIPSETTING (   0x00, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING (   0x01, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING (   0x02, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING (   0x03, DEF_STR( Free_Play ) )
	PORT_DIPNAME( 0x0c, 0x0c, "Right Coin" )            PORT_DIPLOCATION("L8:3,4")
	PORT_DIPSETTING (   0x00, "X 6" )
	PORT_DIPSETTING (   0x04, "X 5" )
	PORT_DIPSETTING (   0x08, "X 4" )
	PORT_DIPSETTING (   0x0c, "X 1" )
	PORT_DIPNAME( 0x10, 0x10, "Center Coin" )           PORT_DIPLOCATION("L8:5") /* "Left Coin" in a 2-mech door */
	PORT_DIPSETTING (   0x00, "X 2" )
	PORT_DIPSETTING (   0x10, "X 1" )
	PORT_DIPNAME( 0xe0, 0xe0, "Bonus Coins" )           PORT_DIPLOCATION("L8:6,7,8")
	PORT_DIPSETTING (   0x60, "1 Coin Each 5 Coins" )
	PORT_DIPSETTING (   0x80, "2 Coins Each 4 Coins" )
	PORT_DIPSETTING (   0xa0, "1 Coin Each 4 Coins" )
	PORT_DIPSETTING (   0xc0, "1 Coin Each 2 Coins" )
	PORT_DIPSETTING (   0xe0, DEF_STR( None ) )

	/* The manual includes a 3rd DIP controlling the number & configuration of coin counters, defined as: */
#if 0
	PORT_START("DSW3")                                  // 4-Toggle switch located on game PCB at M12
	PORT_DIPNAME( 0x03, 0x00, "Coin Counters" )             PORT_DIPLOCATION("M12:1,2")
	PORT_DIPSETTING (   0x00, "1=Left, Center & Right" )    // "For games having these coin doors: Thai 1Baht/1Baht, German 1DM/1DM, US 25c/25c,
															// Belgian or French 5Fr/5Fr, Swiss or French 1Fr/1Fr, US 25c/25c/25c,
															// Japanese 100Y/100Y, Swedish 1Kr/1Kr, UK 10P/10P, Australian 20c/20c, or Italian 100L/100L."
	PORT_DIPSETTING (   0x01, "1=Left & Center, 2=Right" )  // "For games having these coin doors: German 2DM/1DM, German 1DM/5DM, US 25c/25c/1$, or US 25c/1$."
	PORT_DIPSETTING (   0x02, "1=Left, 2=Center & Right" )  // "No coin door is currently designed for this configuration."
	PORT_DIPSETTING (   0x03, "1=Left, 2=Center, 3=Right" ) // "For games having these coin doors: German 1DM/2DM/5DM."
	PORT_DIPUNUSED_DIPLOC( 0x04, 0x04, "M12:3" )            // Listed as "Unused"
	PORT_DIPUNUSED_DIPLOC( 0x08, 0x08, "M12:4" )            // Listed as "Unused"
#endif
INPUT_PORTS_END


static INPUT_PORTS_START( llander )
	PORT_START("IN0")
	/* Bit 0 is VG_HALT */
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_SPECIAL ) PORT_CUSTOM_MEMBER("dvg", dvg_device, done_r, NULL)
	PORT_SERVICE( 0x02, IP_ACTIVE_LOW )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_TILT )
	/* Of the rest, Bit 6 is the 3KHz source. 3,4 and 5 are unknown */
	PORT_BIT( 0x38, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_SPECIAL ) PORT_CUSTOM_MEMBER(DEVICE_SELF, asteroid_state,clock_r, NULL)
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_SERVICE ) PORT_NAME("Diagnostic Step") PORT_CODE(KEYCODE_F1)

	PORT_START("IN1")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_START1 )
	PORT_BIT( 0x02, IP_ACTIVE_LOW,  IPT_COIN1 )
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_COIN2 )
	PORT_BIT( 0x08, IP_ACTIVE_LOW,  IPT_COIN3 )
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_START2 )  PORT_NAME("Select Game")
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_BUTTON3 ) PORT_NAME("Abort")
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_BUTTON2 ) PORT_CODE(KEYCODE_RIGHT) PORT_CODE(JOYCODE_X_RIGHT_SWITCH)    /* right */
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_CODE(KEYCODE_LEFT) PORT_CODE(JOYCODE_X_LEFT_SWITCH)      /* left */

	PORT_START("DSW1")
	PORT_DIPNAME( 0x03, 0x00, "Right Coin" )            PORT_DIPLOCATION("P8:1,2") /* "Left Coin Mech always registers X 1" */
	PORT_DIPSETTING (   0x00, "X 1" )
	PORT_DIPSETTING (   0x01, "X 4" )
	PORT_DIPSETTING (   0x02, "X 5" )
	PORT_DIPSETTING (   0x03, "X 6" )
	PORT_DIPNAME( 0x0c, 0x00, DEF_STR( Language ) )     PORT_DIPLOCATION("P8:3,4")
	PORT_DIPSETTING (   0x00, DEF_STR( English ) )
	PORT_DIPSETTING (   0x04, DEF_STR( French ) )
	PORT_DIPSETTING (   0x08, DEF_STR( Spanish ) )
	PORT_DIPSETTING (   0x0c, DEF_STR( German ) )
	PORT_DIPNAME( 0x20, 0x00, DEF_STR( Coinage ) )      PORT_DIPLOCATION("P8:6")
	PORT_DIPSETTING (   0x00, DEF_STR( Normal ) )
	PORT_DIPSETTING (   0x20, DEF_STR( Free_Play ) )
	PORT_DIPNAME( 0xd0, 0x80, "Fuel Units Per Coin" )   PORT_DIPLOCATION("P8:5,7,8")
	PORT_DIPSETTING (   0x00, "450" )
	PORT_DIPSETTING (   0x40, "600" )
	PORT_DIPSETTING (   0x80, "750" )
	PORT_DIPSETTING (   0xc0, "900" )
	PORT_DIPSETTING (   0x10, "1100" )
	PORT_DIPSETTING (   0x50, "1300" )
	PORT_DIPSETTING (   0x90, "1550" )
	PORT_DIPSETTING (   0xd0, "1800" )

	/* The next one is a potentiometer */
	/* The way the DAC/counter circuit always tries to self center at the voltage derived from the thrust control, */
	/* I don't think it ever expected to get to 0xff. We can not emulate the external DAC circuit exactly, */
	/* so changing the range to 0xfe seems to solve the problem. */
	/* The thrust control is basically a hand operated pedal. */
	/* so IPT_PEDAL is used because it more accurately emulates the control then using IPT_PADDLE */
	PORT_START("THRUST")
	PORT_BIT( 0xff, 0x00, IPT_PEDAL ) PORT_MINMAX(0,254) PORT_SENSITIVITY(100) PORT_KEYDELTA(10)
INPUT_PORTS_END


static INPUT_PORTS_START( llander1 )
	PORT_INCLUDE( llander )

	PORT_MODIFY("DSW1")
	PORT_DIPNAME( 0x10, 0x00, DEF_STR( Coinage ) )      PORT_DIPLOCATION("P8:5") /* "Left Coin Mech always registers X 1" */
	PORT_DIPSETTING (   0x00, DEF_STR( Normal ) )
	PORT_DIPSETTING (   0x10, DEF_STR( Free_Play ) )
	PORT_DIPUNUSED_DIPLOC( 0x20, 0x20, "P8:6" )         /* Listed as "Unused" */
	PORT_DIPNAME( 0xc0, 0x80, "Fuel units" )            PORT_DIPLOCATION("P8:7,8")
	PORT_DIPSETTING (   0x00, "450" )
	PORT_DIPSETTING (   0x40, "600" )
	PORT_DIPSETTING (   0x80, "750" )
	PORT_DIPSETTING (   0xc0, "900" )
INPUT_PORTS_END


static INPUT_PORTS_START( llandert )
	PORT_START("IN0")
	PORT_BIT( 0xff, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("IN1")
	PORT_BIT( 0xff, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("DSW1")
	PORT_DIPUNKNOWN_DIPLOC( 0x01, 0x00, "P8:1" ) // lock up
	PORT_DIPUNKNOWN_DIPLOC( 0x02, 0x00, "P8:2" ) // lock up
	PORT_DIPUNKNOWN_DIPLOC( 0x08, 0x00, "P8:4" ) // lock up
	PORT_DIPNAME( 0x24, 0x00, "Parameter 1" )   PORT_DIPLOCATION("P8:3,6")
	PORT_DIPSETTING (   0x00, "0" )
	PORT_DIPSETTING (   0x04, "1" )
	PORT_DIPSETTING (   0x20, "2" )
	PORT_DIPSETTING (   0x24, "Invalid" )
	PORT_DIPNAME( 0xd0, 0x40, "Parameter 2" )   PORT_DIPLOCATION("P8:5,7,8")
	PORT_DIPSETTING (   0x00, "0" )
	PORT_DIPSETTING (   0x40, "1" )
	PORT_DIPSETTING (   0x80, "2" )
	PORT_DIPSETTING (   0xc0, "3" )
	PORT_DIPSETTING (   0x10, "4" )
	PORT_DIPSETTING (   0x50, "5" )
	PORT_DIPSETTING (   0x90, "6" )
	PORT_DIPSETTING (   0xd0, "7" )

	PORT_START("THRUST")
	PORT_BIT( 0xff, IP_ACTIVE_HIGH, IPT_UNUSED )
INPUT_PORTS_END


/*************************************
 *
 *  Machine drivers
 *
 *************************************/

static MACHINE_CONFIG_START( asteroid, asteroid_state )

	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", M6502, MASTER_CLOCK/8)
	MCFG_CPU_PROGRAM_MAP(asteroid_map)
	MCFG_CPU_PERIODIC_INT_DRIVER(asteroid_state, asteroid_interrupt, CLOCK_3KHZ/12)

	/* video hardware */
	MCFG_VECTOR_ADD("vector")
	MCFG_SCREEN_ADD("screen", VECTOR)
	MCFG_SCREEN_REFRESH_RATE(CLOCK_3KHZ/12/4)
	MCFG_SCREEN_SIZE(400,300)
	MCFG_SCREEN_VISIBLE_AREA(522, 1566, 394, 1182)
	MCFG_SCREEN_UPDATE_DEVICE("vector", vector_device, screen_update)

	MCFG_DEVICE_ADD("dvg", DVG, 0)
	MCFG_AVGDVG_VECTOR("vector")

	/* sound hardware */
	MCFG_SPEAKER_STANDARD_MONO("mono")

	MCFG_SOUND_ADD("discrete", DISCRETE, 0)
	MCFG_DISCRETE_INTF(asteroid)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 1.4)
MACHINE_CONFIG_END

static MACHINE_CONFIG_DERIVED( asterock, asteroid )

	/* basic machine hardware */
	MCFG_CPU_MODIFY("maincpu")
	MCFG_CPU_PERIODIC_INT_DRIVER(asteroid_state, asterock_interrupt, CLOCK_3KHZ/12)
MACHINE_CONFIG_END


static MACHINE_CONFIG_DERIVED( astdelux, asteroid )

	/* basic machine hardware */
	MCFG_CPU_MODIFY("maincpu")
	MCFG_CPU_PROGRAM_MAP(astdelux_map)

	MCFG_ATARIVGEAROM_ADD("earom")

	/* sound hardware */
	MCFG_SOUND_REPLACE("discrete", DISCRETE, 0)
	MCFG_DISCRETE_INTF(astdelux)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 1.0)

	MCFG_SOUND_ADD("pokey", POKEY, MASTER_CLOCK/8)
	MCFG_POKEY_ALLPOT_R_CB(IOPORT("DSW2"))
	MCFG_POKEY_OUTPUT_RC(RES_K(10), CAP_U(0.015), 5.0)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 1.0)
MACHINE_CONFIG_END


static MACHINE_CONFIG_DERIVED( llander, asteroid )

	/* basic machine hardware */
	MCFG_CPU_MODIFY("maincpu")
	MCFG_CPU_PROGRAM_MAP(llander_map)
	MCFG_CPU_PERIODIC_INT_DRIVER(asteroid_state, llander_interrupt,  (double)MASTER_CLOCK/4096/12)

	MCFG_SCREEN_MODIFY("screen")
	MCFG_SCREEN_REFRESH_RATE(CLOCK_3KHZ/12/6)
	MCFG_SCREEN_VISIBLE_AREA(522, 1566, 270, 1070)
	MCFG_SCREEN_UPDATE_DEVICE("vector", vector_device, screen_update)

	/* sound hardware */
	MCFG_SOUND_REPLACE("discrete", DISCRETE, 0)
	MCFG_DISCRETE_INTF(llander)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 1.0)
MACHINE_CONFIG_END



/*************************************
 *
 *  ROM definitions
 *
 *************************************/

ROM_START( asteroid )
	ROM_REGION( 0x8000, "maincpu", 0 )
	ROM_LOAD( "035145-04e.ef2", 0x6800, 0x0800, CRC(b503eaf7) SHA1(5369dcfe01c0b9e48b15a96a0de8d23ee8ef9145) )
	ROM_LOAD( "035144-04e.h2",  0x7000, 0x0800, CRC(25233192) SHA1(51b2865fa897cdaa84ac6500c4b4833a80827019) )
	ROM_LOAD( "035143-02.j2",   0x7800, 0x0800, CRC(312caa02) SHA1(1ce2eac1ab90b972e3f1fc3d250908f26328d6cb) )
	/* Vector ROM */
	ROM_LOAD( "035127-02.np3",  0x5000, 0x0800, CRC(8b71fd9e) SHA1(8cd5005e531eafa361d6b7e9eed159d164776c70) )

	/* DVG PROM */
	ROM_REGION( 0x100, "user1", 0 )
	ROM_LOAD( "034602-01.c8",   0x0000, 0x0100, CRC(97953db8) SHA1(8cbded64d1dd35b18c4d5cece00f77e7b2cab2ad) )
ROM_END

ROM_START( asteroid2 )
	ROM_REGION( 0x8000, "maincpu", 0 )
	ROM_LOAD( "035145-02.ef2",  0x6800, 0x0800, CRC(0cc75459) SHA1(2af85c9689b878155004da47fedbde5853a18723) )
	ROM_LOAD( "035144-02.h2",   0x7000, 0x0800, CRC(096ed35c) SHA1(064d680ded7f30c543f93ae5ca85f90d550f73e5) )
	ROM_LOAD( "035143-02.j2",   0x7800, 0x0800, CRC(312caa02) SHA1(1ce2eac1ab90b972e3f1fc3d250908f26328d6cb) )
	/* Vector ROM */
	ROM_LOAD( "035127-02.np3",  0x5000, 0x0800, CRC(8b71fd9e) SHA1(8cd5005e531eafa361d6b7e9eed159d164776c70) )

	/* DVG PROM */
	ROM_REGION( 0x100, "user1", 0 )
	ROM_LOAD( "034602-01.c8",   0x0000, 0x0100, CRC(97953db8) SHA1(8cbded64d1dd35b18c4d5cece00f77e7b2cab2ad) )
ROM_END

ROM_START( asteroid1 )
	ROM_REGION( 0x8000, "maincpu", 0 )
	ROM_LOAD( "035145-01.ef2",  0x6800, 0x0800, CRC(e9bfda64) SHA1(291dc567ebb31b35df83d9fb87f4080f251ff9c8) )
	ROM_LOAD( "035144-01.h2",   0x7000, 0x0800, CRC(e53c28a9) SHA1(d9f081e73511ec43377f0c6457747f15a470d4dc) )
	ROM_LOAD( "035143-01.j2",   0x7800, 0x0800, CRC(7d4e3d05) SHA1(d88000e904e158efde50e453e2889ecd2cb95f24) )
	/* Vector ROM */
	ROM_LOAD( "035127-01.np3",  0x5000, 0x0800, CRC(99699366) SHA1(9b2828fc1cef7727f65fa65e1e11e309b7c98792) )

	/* DVG PROM */
	ROM_REGION( 0x100, "user1", 0 )
	ROM_LOAD( "034602-01.c8",   0x0000, 0x0100, CRC(97953db8) SHA1(8cbded64d1dd35b18c4d5cece00f77e7b2cab2ad) )
ROM_END

ROM_START( asteroidb )
	ROM_REGION( 0x8000, "maincpu", 0 )
	ROM_LOAD( "035145ll.de1",  0x6800, 0x0800, CRC(605fc0f2) SHA1(8d897a3b75bd1f2537470f0a34a97a8c0853ee08) )
	ROM_LOAD( "035144ll.c1",   0x7000, 0x0800, CRC(e106de77) SHA1(003e99d095bd4df6fae243ea1dd5b12f3eb974f1) )
	ROM_LOAD( "035143ll.b1",   0x7800, 0x0800, CRC(6b1d8594) SHA1(ff3cd93f1bc5734bface285e442125b395602d7d) )
	/* Vector ROM */
	ROM_LOAD( "035127-02.np3",  0x5000, 0x0800, CRC(8b71fd9e) SHA1(8cd5005e531eafa361d6b7e9eed159d164776c70) )

	/* DVG PROM */
	ROM_REGION( 0x100, "user1", 0 )
	ROM_LOAD( "034602-01.c8",  0x0000, 0x0100, CRC(97953db8) SHA1(8cbded64d1dd35b18c4d5cece00f77e7b2cab2ad) )
ROM_END

ROM_START( aerolitos )
	ROM_REGION( 0x8000, "maincpu", 0 )
	ROM_LOAD( "2516_1e.bin", 0x6800, 0x0800, CRC(0cc75459) SHA1(2af85c9689b878155004da47fedbde5853a18723) )
	ROM_LOAD( "2516_1d.bin", 0x7000, 0x0800, CRC(096ed35c) SHA1(064d680ded7f30c543f93ae5ca85f90d550f73e5) )
	ROM_LOAD( "2516_1c.bin", 0x7800, 0x0800, CRC(b912754d) SHA1(d4ada3e162ff454a48468f6309947276df0c5331) )
	/* Vector ROM */
	ROM_LOAD( "2716_3n.bin", 0x5000, 0x0800, CRC(32e69e66) SHA1(a4cce36bc781443b430003280ef4185a4a04de96) )

	/* DVG PROM */
	ROM_REGION( 0x100, "user1", 0 )
	ROM_LOAD( "034602-01.c8",   0x0000, 0x0100, CRC(97953db8) SHA1(8cbded64d1dd35b18c4d5cece00f77e7b2cab2ad) )
ROM_END

ROM_START( asterock )
	ROM_REGION( 0x8000, "maincpu", 0 )
	ROM_LOAD( "10505.2",       0x6800, 0x0400, CRC(cdf720c6) SHA1(85fe748096478e28a06bd98ff3aad73ab21b22a4) )
	ROM_LOAD( "10505.3",       0x6c00, 0x0400, CRC(ee58bdf0) SHA1(80094cb5dafd327aff6658ede33106f0493a809d) )
	ROM_LOAD( "10505.4",       0x7000, 0x0400, CRC(8d3e421e) SHA1(5f5719ab84d4755e69bef205d313b455bc59c413) )
	ROM_LOAD( "10505.5",       0x7400, 0x0400, CRC(d2ce7672) SHA1(b6012e09b2439a614a55bcf23be0692c42830e21) )
	ROM_LOAD( "10505.6",       0x7800, 0x0400, CRC(74103c87) SHA1(e568b5ac573a6d0474cf672b3c62abfbd3320799) )
	ROM_LOAD( "10505.7",       0x7c00, 0x0400, CRC(75a39768) SHA1(bf22998fd692fb01964d8894e421435c55d746a0) )
	/* Vector ROM */
	ROM_LOAD( "10505.0",       0x5000, 0x0400, CRC(6bd2053f) SHA1(790f2858f44bbb1854e2d9d549e29f4815c4665b) )
	ROM_LOAD( "10505.1",       0x5400, 0x0400, CRC(231ce201) SHA1(710f4c19864d725ba1c9ea447a97e84001a679f7) )

	/* DVG PROM */
	ROM_REGION( 0x100, "user1", 0 )
	ROM_LOAD( "034602-01.c8",  0x0000, 0x0100, CRC(97953db8) SHA1(8cbded64d1dd35b18c4d5cece00f77e7b2cab2ad) )
ROM_END

ROM_START( asterockv )
	ROM_REGION( 0x8000, "maincpu", 0 )
	ROM_LOAD( "10505.2",       0x6800, 0x0400, CRC(cdf720c6) SHA1(85fe748096478e28a06bd98ff3aad73ab21b22a4) )
	ROM_LOAD( "10505.3",       0x6c00, 0x0400, CRC(ee58bdf0) SHA1(80094cb5dafd327aff6658ede33106f0493a809d) )
	ROM_LOAD( "10505.4",       0x7000, 0x0400, CRC(8d3e421e) SHA1(5f5719ab84d4755e69bef205d313b455bc59c413) )
	ROM_LOAD( "10505.5",       0x7400, 0x0400, CRC(d2ce7672) SHA1(b6012e09b2439a614a55bcf23be0692c42830e21) )
	ROM_LOAD( "10505.6",       0x7800, 0x0400, CRC(74103c87) SHA1(e568b5ac573a6d0474cf672b3c62abfbd3320799) )
	ROM_LOAD( "10505.7",       0x7c00, 0x0400, CRC(75a39768) SHA1(bf22998fd692fb01964d8894e421435c55d746a0) )
	/* Vector ROM */
	ROM_LOAD( "videotronas.0", 0x5000, 0x0400, CRC(d1ac90b5) SHA1(7209027d2099c75c6336605ae80491ffc5673674) ) // only this rom differs from Sidam's Asterock
	ROM_LOAD( "10505.1",       0x5400, 0x0400, CRC(231ce201) SHA1(710f4c19864d725ba1c9ea447a97e84001a679f7) )

	/* DVG PROM */
	ROM_REGION( 0x100, "user1", 0 )
	ROM_LOAD( "034602-01.c8",  0x0000, 0x0100, CRC(97953db8) SHA1(8cbded64d1dd35b18c4d5cece00f77e7b2cab2ad) )
ROM_END

ROM_START( meteorts )
	ROM_REGION( 0x8000, "maincpu", 0 )
	ROM_LOAD( "m0_c1.bin",    0x6800, 0x0800, CRC(dff88688) SHA1(7f4148a580fb6f605499c99e7dde7068eca1651a) )
	ROM_LOAD( "m1_f1.bin",    0x7000, 0x0800, CRC(e53c28a9) SHA1(d9f081e73511ec43377f0c6457747f15a470d4dc) )
	ROM_LOAD( "m2_j1.bin",    0x7800, 0x0800, CRC(64bd0408) SHA1(141d053cb4cce3fece98293136928b527d3ade0f) )
	/* Vector ROM */
	ROM_LOAD( "mv_np3.bin",   0x5000, 0x0800, CRC(11d1c4ae) SHA1(433c2c05b92094bbe102c356d7f1a907db13da67) )

	/* DVG PROM */
	ROM_REGION( 0x100, "user1", 0 )
	ROM_LOAD( "034602-01.c8",    0x0000, 0x0100, CRC(97953db8) SHA1(8cbded64d1dd35b18c4d5cece00f77e7b2cab2ad) )
ROM_END

ROM_START( meteorho )
	ROM_REGION( 0x8000, "maincpu", 0 )
	ROM_LOAD( "g.bin",    0x6800, 0x0400, CRC(7420421b) SHA1(e84a340c0cbc8816bbe43120bc8e692d2a3db0ab) )
	ROM_LOAD( "h.bin",    0x6c00, 0x0400, CRC(a6aa56bc) SHA1(8298e1667c3bd9af9e0be7d53c00d73ef59d742e) )
	ROM_LOAD( "f.bin",    0x7000, 0x0400, CRC(2711bd52) SHA1(219499b9b8dcc221173f9b9a34c9e6f2fb936231) )
	ROM_LOAD( "d.bin",    0x7400, 0x0400, CRC(9f169db9) SHA1(b6a4a8ea9d48c6b1faebf104faae7c67b2b060b5) )
	ROM_LOAD( "c.bin",    0x7800, 0x0400, CRC(bd99556a) SHA1(8c712b205125c0c2a45dbb4fa9e5e5302c5bbd1b) )
	ROM_LOAD( "e.bin",    0x7c00, 0x0400, CRC(10fdfe9a) SHA1(9db4b3ab904e66d3622ec98e13ef6baf5d4f7099) )
	/* Vector ROM */
	ROM_LOAD( "a.bin",    0x5000, 0x0400, CRC(d7822110) SHA1(bf6c5e622fdc16c39a1d8f23fc029abaa1e99b19) )
	ROM_LOAD( "b.bin",    0x5400, 0x0400, CRC(d62b2887) SHA1(8832953c7166d2f0ed1067c43ebf369db4a4aa70) )

	/* DVG PROM */
	ROM_REGION( 0x100, "user1", 0 )
	ROM_LOAD( "prom.bin",   0x0000, 0x0100, CRC(9e237193) SHA1(f663e12d5db0fa50ea49d03591475ae0a7168bc0) )
ROM_END

ROM_START( hyperspc )
	ROM_REGION( 0x8000, "maincpu", 0 )
	ROM_LOAD( "035145-01.bin",   0x6800, 0x0800, CRC(e9bfda64) SHA1(291dc567ebb31b35df83d9fb87f4080f251ff9c8) )
	ROM_LOAD( "035144-01.bin",   0x7000, 0x0800, CRC(e53c28a9) SHA1(d9f081e73511ec43377f0c6457747f15a470d4dc) )
	ROM_LOAD( "035143-01.bin",   0x7800, 0x0800, CRC(7d4e3d05) SHA1(d88000e904e158efde50e453e2889ecd2cb95f24) )
	/* Vector ROM */
	ROM_LOAD( "035127-01.bin",   0x5000, 0x0800, CRC(7dec48bd) SHA1(8bc926a763ff80b101b2e1c24d45615c3daf67d5) )

	/* DVG PROM */
	ROM_REGION( 0x100, "user1", 0 )
	ROM_LOAD( "034602-01.c8",   0x0000, 0x0100, CRC(97953db8) SHA1(8cbded64d1dd35b18c4d5cece00f77e7b2cab2ad) )
ROM_END

ROM_START( astdelux )
	ROM_REGION( 0x8000, "maincpu", 0 )
	ROM_LOAD( "036430-02.d1",  0x6000, 0x0800, CRC(a4d7a525) SHA1(abe262193ec8e1981be36928e9a89a8ac95cd0ad) )
	ROM_LOAD( "036431-02.ef1", 0x6800, 0x0800, CRC(d4004aae) SHA1(aa2099b8fc62a79879efeea70ea1e9ed77e3e6f0) )
	ROM_LOAD( "036432-02.fh1", 0x7000, 0x0800, CRC(6d720c41) SHA1(198218cd2f43f8b83e4463b1f3a8aa49da5015e4) )
	ROM_LOAD( "036433-03.j1",  0x7800, 0x0800, CRC(0dcc0be6) SHA1(bf10ffb0c4870e777d6b509cbede35db8bb6b0b8) )
	/* Vector ROM */
	ROM_LOAD( "036800-02.r2",  0x4800, 0x0800, CRC(bb8cabe1) SHA1(cebaa1b91b96e8b80f2b2c17c6fd31fa9f156386) )
	ROM_LOAD( "036799-01.np2", 0x5000, 0x0800, CRC(7d511572) SHA1(1956a12bccb5d3a84ce0c1cc10c6ad7f64e30b40) )

	/* DVG PROM */
	ROM_REGION( 0x100, "user1", 0 )
	ROM_LOAD( "034602-01.c8",  0x0000, 0x0100, CRC(97953db8) SHA1(8cbded64d1dd35b18c4d5cece00f77e7b2cab2ad) )
ROM_END

ROM_START( astdelux2 )
	ROM_REGION( 0x8000, "maincpu", 0 )
	ROM_LOAD( "036430-01.d1",  0x6000, 0x0800, CRC(8f5dabc6) SHA1(5d7543e19acab99ddb63c0ffd60f54d7a0f267f5) )
	ROM_LOAD( "036431-01.ef1", 0x6800, 0x0800, CRC(157a8516) SHA1(9041d8c2369d004f198681e02b59a923fa8f70c9) )
	ROM_LOAD( "036432-01.fh1", 0x7000, 0x0800, CRC(fdea913c) SHA1(ded0138a20d80317d67add5bb2a64e6274e0e409) )
	ROM_LOAD( "036433-02.j1",  0x7800, 0x0800, CRC(d8db74e3) SHA1(52b64e867df98d14742eb1817b59931bb7f941d9) )
	/* Vector ROM */
	ROM_LOAD( "036800-01.r2",  0x4800, 0x0800, CRC(3b597407) SHA1(344fea2e5d84acce365d76daed61e96b9b6b37cc) )
	ROM_LOAD( "036799-01.np2", 0x5000, 0x0800, CRC(7d511572) SHA1(1956a12bccb5d3a84ce0c1cc10c6ad7f64e30b40) )

	/* DVG PROM */
	ROM_REGION( 0x100, "user1", 0 )
	ROM_LOAD( "034602-01.c8",  0x0000, 0x0100, CRC(97953db8) SHA1(8cbded64d1dd35b18c4d5cece00f77e7b2cab2ad) )
ROM_END

/***************************************************************************

Asteroids Deluxe revision 1 romset
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

I have had a number of Asteroids deluxe pcbs pass through my hands, every one i have come across has had
a revision -01 marked romset apart from the rom at J1, this is always marked -02.

This revision 2 rom in that location works fine, and i believe was a
factory upgrade, and production fit on 99% of pcbs.

This pcb was from a cabinet with a serial number of 000967, so its an early one, and
it has a completely -01 romset on the pcb.

Coincidentally i went through a pile of my manuals and came across some service bulletins which might
explain why there is this mismatch in revision numbers, it seems Atari released the game, and found
they needed to change default settings becasue of earnings potential in their default
first set (revision 1), not once, but twice! then they changed the romset altogether. The documents
in question are CO-174-02 (2 pages, 3 sides). this document shows what i think was the final revision
of roms to be produced, they consist of a wholly -02 romset, with the expection being a -03 rom at
location J1.

So, anyhow, find in this file, the FIRST revision of rom -01 in J1. This pcb contained the following
roms in the following locations :

all roms are TMS 2516, marked with silver atari labels 'ATARI 8105 ' then the info below:

R2   = 036800 01E
N/P2 = 036799 01E
J1   = 036433 01E
F/H1 = 036432 01E
E/F1 = 036431 01E
D1   = 036430 01E

Dumped 24/08/05
by Andy Welburn
www.andys-arcade.com
*/
ROM_START( astdelux1 )
	ROM_REGION( 0x8000, "maincpu", 0 )
	ROM_LOAD( "036430-01.d1",  0x6000, 0x0800, CRC(8f5dabc6) SHA1(5d7543e19acab99ddb63c0ffd60f54d7a0f267f5) )
	ROM_LOAD( "036431-01.ef1", 0x6800, 0x0800, CRC(157a8516) SHA1(9041d8c2369d004f198681e02b59a923fa8f70c9) )
	ROM_LOAD( "036432-01.fh1", 0x7000, 0x0800, CRC(fdea913c) SHA1(ded0138a20d80317d67add5bb2a64e6274e0e409) )
	ROM_LOAD( "036433-01.j1",  0x7800, 0x0800, CRC(ef09bac7) SHA1(6a4b37dbfe4e6badc4e81036b1430da2e9cb8ca4) )
	/* Vector ROM */
	ROM_LOAD( "036800-01.r2",  0x4800, 0x0800, CRC(3b597407) SHA1(344fea2e5d84acce365d76daed61e96b9b6b37cc) )
	ROM_LOAD( "036799-01.np2", 0x5000, 0x0800, CRC(7d511572) SHA1(1956a12bccb5d3a84ce0c1cc10c6ad7f64e30b40) )

	/* DVG PROM */
	ROM_REGION( 0x100, "user1", 0 )
	ROM_LOAD( "034602-01.c8",  0x0000, 0x0100, CRC(97953db8) SHA1(8cbded64d1dd35b18c4d5cece00f77e7b2cab2ad) )
ROM_END

ROM_START( llander )
	ROM_REGION( 0x8000, "maincpu", 0 )
	ROM_LOAD( "034572-02.f1",  0x6000, 0x0800, CRC(b8763eea) SHA1(5a15eaeaf825ccdf9ce013a6789cf51da20f785c) )
	ROM_LOAD( "034571-02.de1", 0x6800, 0x0800, CRC(77da4b2f) SHA1(4be6cef5af38734d580cbfb7e4070fe7981ddfd6) )
	ROM_LOAD( "034570-01.c1",  0x7000, 0x0800, CRC(2724e591) SHA1(ecf4430a0040c227c896aa2cd81ee03960b4d641) )
	ROM_LOAD( "034569-02.b1",  0x7800, 0x0800, CRC(72837a4e) SHA1(9b21ba5e1518079c326ca6e15b9993e6c4483caa) )
	/* Vector ROM */
	ROM_LOAD( "034599-01.r3",  0x4800, 0x0800, CRC(355a9371) SHA1(6ecb40169b797d9eb623bcb17872f745b1bf20fa) )
	ROM_LOAD( "034598-01.np3", 0x5000, 0x0800, CRC(9c4ffa68) SHA1(eb4ffc289d254f699f821df3146aa2c6cd78597f) )
	ROM_LOAD( "034597-01.m3",  0x5800, 0x0800, CRC(503f992e) SHA1(3e88a110ced9ee1927b0ba760b8d92cbc93e645d) ) /* built from original Atari source code */

	/* DVG PROM */
	ROM_REGION( 0x100, "user1", 0 )
	ROM_LOAD( "034602-01.c8",  0x0000, 0x0100, CRC(97953db8) SHA1(8cbded64d1dd35b18c4d5cece00f77e7b2cab2ad) )
ROM_END

ROM_START( llander1 )
	ROM_REGION( 0x8000, "maincpu", 0 )
	ROM_LOAD( "034572-01.f1",  0x6000, 0x0800, CRC(2aff3140) SHA1(4fc8aae640ce655417c11d9a3121aae9a1238e7c) )
	ROM_LOAD( "034571-01.de1", 0x6800, 0x0800, CRC(493e24b7) SHA1(125a2c335338ccabababef12fd7096ef4b605a31) )
	ROM_LOAD( "034570-01.c1",  0x7000, 0x0800, CRC(2724e591) SHA1(ecf4430a0040c227c896aa2cd81ee03960b4d641) )
	ROM_LOAD( "034569-01.b1",  0x7800, 0x0800, CRC(b11a7d01) SHA1(8f2935dbe04ee68815d69ea9e71853b5a145d7c3) )
	/* Vector ROM */
	ROM_LOAD( "034599-01.r3",  0x4800, 0x0800, CRC(355a9371) SHA1(6ecb40169b797d9eb623bcb17872f745b1bf20fa) )
	ROM_LOAD( "034598-01.np3", 0x5000, 0x0800, CRC(9c4ffa68) SHA1(eb4ffc289d254f699f821df3146aa2c6cd78597f) )
	ROM_LOAD( "034597-01.m3",  0x5800, 0x0800, CRC(503f992e) SHA1(3e88a110ced9ee1927b0ba760b8d92cbc93e645d) ) /* built from original Atari source code */

	/* DVG PROM */
	ROM_REGION( 0x100, "user1", 0 )
	ROM_LOAD( "034602-01.c8",  0x0000, 0x0100, CRC(97953db8) SHA1(8cbded64d1dd35b18c4d5cece00f77e7b2cab2ad) )
ROM_END

ROM_START( llandert )
	ROM_REGION( 0x8000, "maincpu", 0 )
	ROM_LOAD( "llprom0.de1",   0x6800, 0x0800, CRC(b5302947) SHA1(622245053682b838762b4eb04be5d4bbed5e78ef) )
	ROM_LOAD( "llprom1.c1",    0x7000, 0x0800, CRC(761a5b45) SHA1(dde08ef856caed4b017bfbb8e3f3260747c0a1e5) )
	ROM_LOAD( "llprom2.b1",    0x7800, 0x0800, CRC(9ec62656) SHA1(2247fd187d0aa7305d7326722c99578ad20718fa) )

	/* Vector ROM */
	ROM_LOAD( "llvrom0.r3",    0x4800, 0x0800, CRC(c307b42a) SHA1(6872c7671f4ab314962892b3cf93cc9d6c380ee2) ) // unused
	ROM_LOAD( "llvrom1.np3",   0x5000, 0x0800, CRC(ace6b2be) SHA1(afdf4e6fc4be23197977e67c04b9baf9597756a0) ) // unused
	ROM_LOAD( "llvrom2.m3",    0x5800, 0x0800, CRC(56c38219) SHA1(714878c0b24c9657c972a2ba25e790a4d3b81d64) ) // unused, filled with garbage? (it is not the language rom)

	/* DVG PROM */
	ROM_REGION( 0x100, "user1", 0 )
	ROM_LOAD( "034602-01.c8",  0x0000, 0x0100, CRC(97953db8) SHA1(8cbded64d1dd35b18c4d5cece00f77e7b2cab2ad) ) // taken from parent
ROM_END



/*************************************
 *
 *  Driver initialization
 *
 *************************************/

DRIVER_INIT_MEMBER(asteroid_state,asteroidb)
{
	m_maincpu->space(AS_PROGRAM).install_read_port(0x2000, 0x2000, "IN0");
	m_maincpu->space(AS_PROGRAM).install_read_port(0x2003, 0x2003, "HS");
}


DRIVER_INIT_MEMBER(asteroid_state,asterock)
{
	m_maincpu->space(AS_PROGRAM).install_read_handler(0x2000, 0x2007, read8_delegate(FUNC(asteroid_state::asterock_IN0_r),this));
}

/*************************************
 *
 *  Game drivers
 *
 *************************************/

GAME( 1979, asteroid,  0,         asteroid,  asteroid,  driver_device,  0,         ROT0, "Atari",   "Asteroids (rev 4)",        MACHINE_SUPPORTS_SAVE )
GAME( 1979, asteroid2, asteroid,  asteroid,  asteroid,  driver_device,  0,         ROT0, "Atari",   "Asteroids (rev 2)",        MACHINE_SUPPORTS_SAVE )
GAME( 1979, asteroid1, asteroid,  asteroid,  asteroid,  driver_device,  0,         ROT0, "Atari",   "Asteroids (rev 1)",        MACHINE_SUPPORTS_SAVE )
GAME( 1979, asteroidb, asteroid,  asteroid,  asteroidb, asteroid_state, asteroidb, ROT0, "bootleg", "Asteroids (bootleg on Lunar Lander hardware)",  MACHINE_SUPPORTS_SAVE )
GAME( 1980, aerolitos, asteroid,  asteroid,  aerolitos, driver_device,  0,         ROT0, "bootleg (Rodmar Elec.)","Aerolitos (Spanish bootleg of Asteroids)",        MACHINE_SUPPORTS_SAVE ) // 'Aerolitos' appears on the cabinet, this was distributed in Spain, the Spanish text is different to that contained in the original version (corrected)
GAME( 1979, asterock,  asteroid,  asterock,  asterock,  asteroid_state, asterock,  ROT0, "bootleg (Sidam)",       "Asterock (Sidam bootleg of Asteroids)",     MACHINE_SUPPORTS_SAVE )
GAME( 1979, asterockv, asteroid,  asterock,  asterock,  asteroid_state, asterock,  ROT0, "bootleg (Videotron)",   "Asterock (Videotron bootleg of Asteroids)", MACHINE_SUPPORTS_SAVE )
GAME( 1979, meteorts,  asteroid,  asteroid,  asteroid,  driver_device,  0,         ROT0, "bootleg (VGG)",         "Meteorites (bootleg of Asteroids)", MACHINE_SUPPORTS_SAVE )
GAME( 1979, meteorho,  asteroid,  asteroid,  asteroid,  driver_device,  0,         ROT0, "bootleg (Hoei)",        "Meteor (bootleg of Asteroids)",     MACHINE_SUPPORTS_SAVE )
GAME( 1979, hyperspc,  asteroid,  asteroid,  asteroid,  driver_device,  0,         ROT0, "bootleg (Rumiano)",     "Hyperspace (bootleg of Asteroids)", MACHINE_SUPPORTS_SAVE )

GAMEL(1980, astdelux,  0,         astdelux,  astdelux,  driver_device,  0,         ROT0, "Atari",   "Asteroids Deluxe (rev 3)", MACHINE_SUPPORTS_SAVE, layout_astdelux )
GAMEL(1980, astdelux2, astdelux,  astdelux,  astdelux,  driver_device,  0,         ROT0, "Atari",   "Asteroids Deluxe (rev 2)", MACHINE_SUPPORTS_SAVE, layout_astdelux )
GAMEL(1980, astdelux1, astdelux,  astdelux,  astdelux,  driver_device,  0,         ROT0, "Atari",   "Asteroids Deluxe (rev 1)", MACHINE_SUPPORTS_SAVE, layout_astdelux )

GAME( 1979, llander,   0,         llander,   llander,   driver_device,  0,         ROT0, "Atari",   "Lunar Lander (rev 2)",     MACHINE_SUPPORTS_SAVE )
GAME( 1979, llander1,  llander,   llander,   llander1,  driver_device,  0,         ROT0, "Atari",   "Lunar Lander (rev 1)",     MACHINE_SUPPORTS_SAVE )
GAME( 1979, llandert,  llander,   llander,   llandert,  driver_device,  0,         ROT0, "Atari",   "Lunar Lander (screen test)", MACHINE_SUPPORTS_SAVE ) // no copyright shown, assume it's an in-house diagnostics romset (PCB came from a seller that has had Atari prototypes in his possession before)
