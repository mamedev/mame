// license:BSD-3-Clause
// copyright-holders:David Haywood
/* Angel Kids / Space Position hardware driver

 driver by David Haywood
 with some help from Steph (DSWs, Inputs, other
 bits here and there)

2 Board System, Uses Boards X090-PC-A & X090-PC-B

Both games appear to be joint Sega / Nasco efforts
(although all I see in Angel Kids is 'Exa Planning'
 but I think that has something to do with Nasco   )

Space Position is encrypted, the main processor is
D317-0005 (NEC Z80 Custom), see machine/segacrpt.c
for details on this encryption scheme

*/

/* started 23/01/2002 */

/* notes / todo:

Decrypt Space Position Somehow (not something I
can do)
Unknown Reads / Writes
Whats the Prom for? nothing important?
Is the level order correct?
the progress sprite on the side of the screen re-appears at the bottom when you get
to the top, but the wrap-around is needed for other things, actual game bug?

*/

/* readme's

------------------------------------------------------------------------

Angel Kids
833-6599-01
Sega 1988

Nasco X090-PC-A  (Sega 837-6600)

 SW1   SW2


 8255

 8255

       11429 6116 Z80   YM2203 YM2203


 11424 11425 11426 11427  -  -  -  -  5M5165 11428  Z80
                                                         4MHz

                                                         6MHz


Nasco X090-PC-B

                                  2016-55
11437  11445    2016-55  2016-55             U5
11436  11444
11435  11443
11434  11442
11433  11441                  2016-55    2016-55
11432  11440
11431  11439    11446         2016-55

                                             11148
                                             11147
   2016-55 2016-55 2016-55

                                               18.432MHz

11430  11438

------------------------------------------------------------------------

Space Position (JPN Ver.)
(c)1986 Sega / Nasco
X090-PC-A 171-5383
X090-PC-B 171-5384


CPU :D317-0005 (NEC Z80 Custom)
Sound   :NEC D780C-1
    :YM2203C x 2
OSC :4.000MHz 6.000MHz
    :18.432MHz


EPR10120.C1 prg
EPR10121.C2  |
EPR10122.C3  |
EPR10123.C4  |
EPR10124.C5  |
EPR10125.C10    /

EPR10126.D4 snd

EPR10127.06
EPR10128.07
EPR10129.08
EPR10130.14
EPR10131.15
EPR10132.16

EPR10133.17

EPR10134.18
EPR10135.19

63S081N.U5


--- Team Japump!!! ---
Dumped by Chackn
02/25/2000

------------------------------------------------------------------------

*/


#include "emu.h"
#include "cpu/z80/z80.h"
#include "machine/segacrp2.h"
#include "sound/2203intf.h"
#include "includes/angelkds.h"






/*** CPU Banking

*/

WRITE8_MEMBER(angelkds_state::angelkds_cpu_bank_write)
{
	membank("bank1")->set_entry(data & 0x0f);   // shall we check (data & 0x0f) < # of available banks (8 or 10 resp.)?
}


/*** Fake Inputs

these make the game a bit easier for testing purposes

*/

#define FAKEINPUTS 0

#if FAKEINPUTS

READ8_MEMBER(angelkds_state::angelkds_input_r)
{
	int fake;
	static const char *const portnames[] = { "I81", "I82" };
	static const char *const fakenames[] = { "FAKE1", "FAKE2" };

	fake = ioport(fakenames[offset])->read();

	return ((fake & 0x01) ? fake  : ioport(portnames[offset])->read());
}

#else

READ8_MEMBER(angelkds_state::angelkds_input_r)
{
	static const char *const portnames[] = { "I81", "I82" };

	return ioport(portnames[offset])->read();
}

#endif

/*** Memory Structures

Angel Kids:
I would have expected f003 to be the scroll register for the bottom
part of the screen, in the attract mode this works fine, but in the
game it doesn't, so maybe it wasn't really hooked up and instead
only one of the register (f001) is used for both part?

 update, it is correct, the screen is meant to split in two when
 the kid goes what would be offscreen, just looked kinda odd

Interesting note, each Bank in the 0x8000 - 0xbfff appears to
contain a level.

*/

static ADDRESS_MAP_START( main_map, AS_PROGRAM, 8, angelkds_state )
	AM_RANGE(0x0000, 0x7fff) AM_ROM
	AM_RANGE(0x8000, 0xbfff) AM_ROMBANK("bank1")
	AM_RANGE(0xc000, 0xdfff) AM_RAM
	AM_RANGE(0xe000, 0xe3ff) AM_RAM_WRITE(angelkds_bgtopvideoram_w) AM_SHARE("bgtopvideoram") /* Top Half of Screen */
	AM_RANGE(0xe400, 0xe7ff) AM_RAM_WRITE(angelkds_bgbotvideoram_w) AM_SHARE("bgbotvideoram") /* Bottom Half of Screen */
	AM_RANGE(0xe800, 0xebff) AM_RAM_WRITE(angelkds_txvideoram_w) AM_SHARE("txvideoram")
	AM_RANGE(0xec00, 0xecff) AM_RAM AM_SHARE("spriteram")
	AM_RANGE(0xed00, 0xeeff) AM_RAM_WRITE(angelkds_paletteram_w) AM_SHARE("paletteram")
	AM_RANGE(0xef00, 0xefff) AM_RAM
	AM_RANGE(0xf000, 0xf000) AM_WRITE(angelkds_bgtopbank_write)
	AM_RANGE(0xf001, 0xf001) AM_WRITE(angelkds_bgtopscroll_write)
	AM_RANGE(0xf002, 0xf002) AM_WRITE(angelkds_bgbotbank_write)
	AM_RANGE(0xf003, 0xf003) AM_WRITE(angelkds_bgbotscroll_write)
	AM_RANGE(0xf004, 0xf004) AM_WRITE(angelkds_txbank_write)
	AM_RANGE(0xf005, 0xf005) AM_WRITE(angelkds_layer_ctrl_write)
ADDRESS_MAP_END

static ADDRESS_MAP_START( decrypted_opcodes_map, AS_DECRYPTED_OPCODES, 8, angelkds_state )
	AM_RANGE(0x0000, 0x7fff) AM_ROM AM_SHARE("decrypted_opcodes")
	AM_RANGE(0x8000, 0xbfff) AM_ROMBANK("bank1")
ADDRESS_MAP_END

static ADDRESS_MAP_START( main_portmap, AS_IO, 8, angelkds_state )
	ADDRESS_MAP_GLOBAL_MASK(0xff)
	AM_RANGE(0x00, 0x00) AM_WRITENOP // 00 on start-up, not again
	AM_RANGE(0x42, 0x42) AM_WRITE(angelkds_cpu_bank_write)
	AM_RANGE(0x43, 0x43) AM_WRITENOP // 9a on start-up, not again
	AM_RANGE(0x40, 0x40) AM_READ_PORT("I40")    /* "Coinage" Dip Switches */
	AM_RANGE(0x41, 0x41) AM_READ_PORT("I41")    /* Other Dip Switches */
	AM_RANGE(0x42, 0x42) AM_READ_PORT("I42")    /* Players inputs (not needed ?) */
	AM_RANGE(0x80, 0x80) AM_READ_PORT("I80")    /* System inputs */
	AM_RANGE(0x81, 0x82) AM_READ(angelkds_input_r)  /* Players inputs */
	AM_RANGE(0x83, 0x83) AM_WRITENOP // 9b on start-up, not again
	AM_RANGE(0xc0, 0xc3) AM_READWRITE(angelkds_main_sound_r, angelkds_main_sound_w) // 02 various points
ADDRESS_MAP_END

/* sub cpu */

static ADDRESS_MAP_START( sub_map, AS_PROGRAM, 8, angelkds_state )
	AM_RANGE(0x0000, 0x7fff) AM_ROM
	AM_RANGE(0x8000, 0x87ff) AM_RAM
	AM_RANGE(0xaaa9, 0xaaa9) AM_READNOP
	AM_RANGE(0xaaab, 0xaaab) AM_READNOP
	AM_RANGE(0xaaac, 0xaaac) AM_READNOP
ADDRESS_MAP_END

static ADDRESS_MAP_START( sub_portmap, AS_IO, 8, angelkds_state )
	ADDRESS_MAP_GLOBAL_MASK(0xff)
	AM_RANGE(0x00, 0x01) AM_DEVREADWRITE("ym1", ym2203_device, read, write)
	AM_RANGE(0x40, 0x41) AM_DEVREADWRITE("ym2", ym2203_device, read, write)
	AM_RANGE(0x80, 0x83) AM_READWRITE(angelkds_sub_sound_r, angelkds_sub_sound_w) // spcpostn
ADDRESS_MAP_END


/* Input Ports */

#define ANGELDSK_PLAYERS_INPUT( player ) \
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICKRIGHT_UP ) PORT_PLAYER(player) PORT_8WAY \
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICKRIGHT_DOWN ) PORT_PLAYER(player) PORT_8WAY \
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICKRIGHT_LEFT ) PORT_PLAYER(player) PORT_8WAY \
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICKRIGHT_RIGHT ) PORT_PLAYER(player) PORT_8WAY \
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_JOYSTICKLEFT_UP ) PORT_PLAYER(player) PORT_8WAY \
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_JOYSTICKLEFT_DOWN ) PORT_PLAYER(player) PORT_8WAY \
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_JOYSTICKLEFT_LEFT ) PORT_PLAYER(player) PORT_8WAY \
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_JOYSTICKLEFT_RIGHT ) PORT_PLAYER(player) PORT_8WAY

#define ANGELDSK_FAKE_PLAYERS_INPUT( player ) \
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(player)    /* To enter initials */ \
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNKNOWN )        /* Unused */ \
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_PLAYER(player) PORT_8WAY \
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_PLAYER(player) PORT_8WAY \
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_PLAYER(player) PORT_8WAY \
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_PLAYER(player) PORT_8WAY \
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(player)    /* To shorten the rope and */ \
										/* move right in hiscores table */


static INPUT_PORTS_START( angelkds )
	/*
	    Free Play: Set SW1:1-8 ON (A:Free Play & B:Free Play).
	    Sound Test: Set SW1:1-8 ON (A:Free Play & B:Free Play), hold test switch and reboot.
	    Joystick Test: Set SW1:1-7 ON & SW1:8 OFF (A:Free Play & B:3C_1C), hold test switch and reboot.
	    Joystick Test Coin_A & Coin_B seem to be switched, only works when setting A to 3C_1C and B to Free Play.
	*/
	PORT_START("I40")       /* inport $40 */
	PORT_DIPNAME( 0xf0, 0xf0, DEF_STR( Coin_A ) )       PORT_DIPLOCATION("SW1:1,2,3,4")
	PORT_DIPSETTING(    0x70, DEF_STR( 4C_1C ) )
	PORT_DIPSETTING(    0x80, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(    0x90, DEF_STR( 2C_1C ) )
//  PORT_DIPSETTING(    0x60, DEF_STR( 2C_1C ) )
//  PORT_DIPSETTING(    0x50, DEF_STR( 2C_1C ) )
//  PORT_DIPSETTING(    0x40, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0xf0, DEF_STR( 1C_1C ) )
//  PORT_DIPSETTING(    0x30, DEF_STR( 1C_1C ) )
//  PORT_DIPSETTING(    0x20, DEF_STR( 1C_1C ) )
//  PORT_DIPSETTING(    0x10, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0xe0, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0xd0, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(    0xc0, DEF_STR( 1C_4C ) )
	PORT_DIPSETTING(    0xb0, DEF_STR( 1C_5C ) )
	PORT_DIPSETTING(    0xa0, DEF_STR( 1C_6C ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Free_Play ) )

	PORT_DIPNAME( 0x0f, 0x0f, DEF_STR( Coin_B ) )       PORT_DIPLOCATION("SW1:5,6,7,8")
	PORT_DIPSETTING(    0x07, DEF_STR( 4C_1C ) )
	PORT_DIPSETTING(    0x08, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(    0x09, DEF_STR( 2C_1C ) )
//  PORT_DIPSETTING(    0x06, DEF_STR( 2C_1C ) )
//  PORT_DIPSETTING(    0x05, DEF_STR( 2C_1C ) )
//  PORT_DIPSETTING(    0x04, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x0f, DEF_STR( 1C_1C ) )
//  PORT_DIPSETTING(    0x03, DEF_STR( 1C_1C ) )
//  PORT_DIPSETTING(    0x02, DEF_STR( 1C_1C ) )
//  PORT_DIPSETTING(    0x01, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x0e, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0x0d, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(    0x0c, DEF_STR( 1C_4C ) )
	PORT_DIPSETTING(    0x0b, DEF_STR( 1C_5C ) )
	PORT_DIPSETTING(    0x0a, DEF_STR( 1C_6C ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Free_Play ) )

	PORT_START("I41")       /* inport $41 */
	PORT_DIPNAME( 0x01, 0x00, DEF_STR( Cabinet ) )              PORT_DIPLOCATION("SW2:1")
	PORT_DIPSETTING(    0x00, DEF_STR( Upright ) )
	PORT_DIPSETTING(    0x01, DEF_STR( Cocktail ) )
	PORT_DIPNAME( 0x02, 0x00, "High Score Characters" )         PORT_DIPLOCATION("SW2:2")
	PORT_DIPSETTING(    0x00, "3" )
	PORT_DIPSETTING(    0x02, "10" )
	PORT_DIPNAME( 0x0c, 0x08, DEF_STR( Bonus_Life ) )           PORT_DIPLOCATION("SW2:3,4")
	PORT_DIPSETTING(    0x0c, "20k, 50k, 100k, 200k and 500k" )
	PORT_DIPSETTING(    0x08, "50k, 100k, 200k and 500k" )
	PORT_DIPSETTING(    0x04, "100k, 200k and 500k" )
	PORT_DIPSETTING(    0x00, DEF_STR( None ) )
	PORT_DIPNAME( 0x30, 0x30, DEF_STR( Lives ) )                PORT_DIPLOCATION("SW2:5,6")
	PORT_DIPSETTING(    0x30, "3" )
	PORT_DIPSETTING(    0x20, "4" )
	PORT_DIPSETTING(    0x10, "5" )
	PORT_DIPSETTING(    0x00, "99 (Cheat)" )
	PORT_DIPNAME( 0xc0, 0xc0, DEF_STR( Difficulty ) )           PORT_DIPLOCATION("SW2:7,8") /* Stored at 0xc023 */
	PORT_DIPSETTING(    0xc0, DEF_STR( Very_Easy ) )
	PORT_DIPSETTING(    0x40, DEF_STR( Easy ) )
	PORT_DIPSETTING(    0x80, DEF_STR( Hard ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Very_Hard ) )

	PORT_START("I42")       /* inport $42 */
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNKNOWN )    // duplicated IPT_JOYSTICK_LEFTRIGHT
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN ) PORT_8WAY  // duplicated IPT_JOYSTICK_LEFTRIGHT
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN ) PORT_8WAY PORT_COCKTAIL
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("I80")       /* inport $80 */
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_SERVICE1 )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_START2 )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_SERVICE( 0x80, IP_ACTIVE_LOW )

	PORT_START("I81")       /* inport $81 */
	ANGELDSK_PLAYERS_INPUT( 1 )

	PORT_START("I82")       /* inport $82 */
	ANGELDSK_PLAYERS_INPUT( 2 )

#if FAKEINPUTS

	/* Fake inputs to allow to play the game with 1 joystick instead of 2 */
	PORT_START("FAKE1")
	PORT_DIPNAME( 0x01, 0x00, "FAKE (for debug) Joysticks (Player 1)" )
	PORT_DIPSETTING(    0x01, "1" )
	PORT_DIPSETTING(    0x00, "2" )
	ANGELDSK_FAKE_PLAYERS_INPUT( 1 )

	PORT_START("FAKE2")
	PORT_DIPNAME( 0x01, 0x00, "FAKE (for debug) Joysticks (Player 2)" )
	PORT_DIPSETTING(    0x01, "1" )
	PORT_DIPSETTING(    0x00, "2" )
	ANGELDSK_FAKE_PLAYERS_INPUT( 2 )

#endif

INPUT_PORTS_END

static INPUT_PORTS_START( spcpostn )
	PORT_START("I40")       /* inport $40 */
	PORT_DIPNAME( 0x0f, 0x0f, DEF_STR( Coin_A ) )           PORT_DIPLOCATION("SW1:1,2,3,4")
	PORT_DIPSETTING(    0x02, DEF_STR( 4C_1C ) )
	PORT_DIPSETTING(    0x05, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(    0x08, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x04, DEF_STR( 3C_2C ) )
	PORT_DIPSETTING(    0x01, DEF_STR( 4C_3C ) )
	PORT_DIPSETTING(    0x0f, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x03, DEF_STR( 3C_4C ) )
	PORT_DIPSETTING(    0x07, DEF_STR( 2C_3C ) )
	PORT_DIPSETTING(    0x0e, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0x06, DEF_STR( 2C_5C ) )
	PORT_DIPSETTING(    0x0d, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(    0x0c, DEF_STR( 1C_4C ) )
	PORT_DIPSETTING(    0x0b, DEF_STR( 1C_5C ) )
	PORT_DIPSETTING(    0x0a, DEF_STR( 1C_6C ) )
	PORT_DIPSETTING(    0x09, DEF_STR( 1C_7C ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Free_Play ) )
	PORT_DIPNAME( 0xf0, 0xf0, DEF_STR( Coin_B ) )           PORT_DIPLOCATION("SW1:5,6,7,8")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x20, DEF_STR( 4C_1C ) )
	PORT_DIPSETTING(    0x50, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(    0x80, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x40, DEF_STR( 3C_2C ) )
	PORT_DIPSETTING(    0x10, DEF_STR( 4C_3C ) )
	PORT_DIPSETTING(    0xf0, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x30, DEF_STR( 3C_4C ) )
	PORT_DIPSETTING(    0x70, DEF_STR( 2C_3C ) )
	PORT_DIPSETTING(    0xe0, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0x60, DEF_STR( 2C_5C ) )
	PORT_DIPSETTING(    0xd0, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(    0xc0, DEF_STR( 1C_4C ) )
	PORT_DIPSETTING(    0xb0, DEF_STR( 1C_5C ) )
	PORT_DIPSETTING(    0xa0, DEF_STR( 1C_6C ) )
	PORT_DIPSETTING(    0x90, DEF_STR( 1C_7C ) )

	PORT_START("I41")       /* inport $41 */
	PORT_DIPNAME( 0x01, 0x01, DEF_STR(Allow_Continue ) )    PORT_DIPLOCATION("SW2:1")
	PORT_DIPSETTING(    0x01, DEF_STR( No ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Yes ) )
	PORT_DIPNAME( 0x02, 0x02, "Obstruction Car" )           PORT_DIPLOCATION("SW2:2")
	PORT_DIPSETTING(    0x02, DEF_STR( Normal ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Hard ) )
	PORT_DIPNAME( 0x0c, 0x08, "Time Limit" )                PORT_DIPLOCATION("SW2:3,4")
	PORT_DIPSETTING(    0x00, "1:10" )
	PORT_DIPSETTING(    0x04, "1:20" )
	PORT_DIPSETTING(    0x08, "1:30" )
	PORT_DIPSETTING(    0x0c, "1:40" )
	PORT_DIPNAME( 0x30, 0x20, "Power Down" )                PORT_DIPLOCATION("SW2:5,6")
	PORT_DIPSETTING(    0x30, "Slow" )
	PORT_DIPSETTING(    0x20, DEF_STR( Normal ) )
	PORT_DIPSETTING(    0x10, "Fast" )
	PORT_DIPSETTING(    0x00, "Fastest" )
	PORT_DIPNAME( 0x40, 0x00, DEF_STR( Demo_Sounds ) )      PORT_DIPLOCATION("SW2:7")
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPUNUSED_DIPLOC( 0x80, 0x80, "SW2:8" )            /* Listed as "Unused" */

	PORT_START("I42")       /* inport $42 */
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("I80")       /* inport $80 */
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_SERVICE1 )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_START2 )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_SERVICE( 0x80, IP_ACTIVE_LOW )

	PORT_START("I81")       /* inport $81 */
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_UP )    PORT_PLAYER(1) PORT_8WAY
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN )  PORT_PLAYER(1) PORT_8WAY
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT )  PORT_PLAYER(1) PORT_8WAY
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_PLAYER(1) PORT_8WAY
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(1)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(1)
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNUSED ) PORT_PLAYER(1) // probably unused
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNUSED ) PORT_PLAYER(1) // probably unused

	PORT_START("I82")       /* inport $82 */
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_UP )    PORT_PLAYER(2) PORT_8WAY
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN )  PORT_PLAYER(2) PORT_8WAY
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT )  PORT_PLAYER(2) PORT_8WAY
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_PLAYER(2) PORT_8WAY
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(2)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(2)
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNUSED ) PORT_PLAYER(2) // probably unused
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNUSED ) PORT_PLAYER(2) // probably unused

INPUT_PORTS_END

/*** Sound Hardware

todo: verify / correct things
seems a bit strange are all the addresses really
sound related ?

*/

WRITE8_MEMBER(angelkds_state::angelkds_main_sound_w)
{
	m_sound[offset] = data;
}

READ8_MEMBER(angelkds_state::angelkds_main_sound_r)
{
	return m_sound2[offset];
}

WRITE8_MEMBER(angelkds_state::angelkds_sub_sound_w)
{
	m_sound2[offset] = data;
}

READ8_MEMBER(angelkds_state::angelkds_sub_sound_r)
{
	return m_sound[offset];
}


WRITE_LINE_MEMBER(angelkds_state::irqhandler)
{
	m_subcpu->set_input_line(0, state ? ASSERT_LINE : CLEAR_LINE);
}

/*** Graphics Decoding

all the 8x8 tiles are in one format, the 16x16 sprites in another

*/

static const gfx_layout angelkds_charlayout =
{
	8,8,
	RGN_FRAC(1,1),
	4,
	{ 0,1,2,3 },
	{ 0, 4, 8, 12, 16, 20, 24, 28 },
	{ 0*32, 1*32, 2*32, 3*32, 4*32, 5*32, 6*32, 7*32 },
	8*32
};


static const gfx_layout angelkds_spritelayout =
{
	16,16,
	RGN_FRAC(1,2),
	4,
	{ 0,4,  RGN_FRAC(1,2)+0,    RGN_FRAC(1,2)+4 },
	{ 0, 1, 2, 3, 8, 9, 10, 11, 16,17,18,19, 24,25,26,27 },
	{ 0*32, 1*32, 2*32, 3*32, 4*32, 5*32, 6*32, 7*32, 8*32, 9*32, 10*32, 11*32, 12*32, 13*32, 14*32, 15*32  },
	16*32
};

static GFXDECODE_START( angelkds )
	GFXDECODE_ENTRY( "gfx1", 0, angelkds_charlayout,   0x30, 1  )
	GFXDECODE_ENTRY( "gfx3", 0, angelkds_charlayout,   0, 16 )
	GFXDECODE_ENTRY( "gfx4", 0, angelkds_charlayout,   0, 16 )
	GFXDECODE_ENTRY( "gfx2", 0, angelkds_spritelayout, 0x20, 0x0d )
GFXDECODE_END

/*** Machine Driver

 2 x z80 (one for game, one for sound)
 2 x YM2203 (for sound)

 all fairly straightforward

*/

void angelkds_state::machine_start()
{
	save_item(NAME(m_layer_ctrl));
	save_item(NAME(m_txbank));
	save_item(NAME(m_bgbotbank));
	save_item(NAME(m_bgtopbank));
	save_item(NAME(m_sound));
	save_item(NAME(m_sound2));
}

void angelkds_state::machine_reset()
{
	int i;

	for (i = 0; i < 4; i++)
	{
		m_sound[i] = 0;
		m_sound2[i] = 0;
	}

	m_layer_ctrl = 0;
	m_txbank = 0;
	m_bgbotbank = 0;
	m_bgtopbank = 0;
}

static MACHINE_CONFIG_START( angelkds, angelkds_state )

	MCFG_CPU_ADD("maincpu", Z80, XTAL_6MHz)
	MCFG_CPU_PROGRAM_MAP(main_map)
	MCFG_CPU_IO_MAP(main_portmap)
	MCFG_CPU_VBLANK_INT_DRIVER("screen", angelkds_state,  irq0_line_hold)

	MCFG_CPU_ADD("sub", Z80, XTAL_4MHz)
	MCFG_CPU_PROGRAM_MAP(sub_map)
	MCFG_CPU_IO_MAP(sub_portmap)


	MCFG_QUANTUM_TIME(attotime::from_hz(6000))

	/* video hardware */
	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_REFRESH_RATE(60)
	MCFG_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(0))
	MCFG_SCREEN_SIZE(32*8, 32*8)
	MCFG_SCREEN_VISIBLE_AREA(0*8, 32*8-1, 1*8, 31*8-1)
	MCFG_SCREEN_UPDATE_DRIVER(angelkds_state, screen_update_angelkds)
	MCFG_SCREEN_PALETTE("palette")
	MCFG_SCREEN_ORIENTATION(ROT90)

	MCFG_GFXDECODE_ADD("gfxdecode", "palette", angelkds)
	MCFG_PALETTE_ADD("palette", 0x100)


	MCFG_SPEAKER_STANDARD_MONO("mono")

	MCFG_SOUND_ADD("ym1", YM2203, XTAL_4MHz)
	MCFG_YM2203_IRQ_HANDLER(WRITELINE(angelkds_state, irqhandler))
	MCFG_SOUND_ROUTE(0, "mono", 0.65)
	MCFG_SOUND_ROUTE(1, "mono", 0.65)
	MCFG_SOUND_ROUTE(2, "mono", 0.65)
	MCFG_SOUND_ROUTE(3, "mono", 0.45)

	MCFG_SOUND_ADD("ym2", YM2203, XTAL_4MHz)
	MCFG_SOUND_ROUTE(0, "mono", 0.65)
	MCFG_SOUND_ROUTE(1, "mono", 0.65)
	MCFG_SOUND_ROUTE(2, "mono", 0.65)
	MCFG_SOUND_ROUTE(3, "mono", 0.45)
MACHINE_CONFIG_END

static MACHINE_CONFIG_DERIVED( spcpostn, angelkds )
	/* encryption */
	MCFG_DEVICE_MODIFY("maincpu")
	MCFG_CPU_DECRYPTED_OPCODES_MAP(decrypted_opcodes_map)
MACHINE_CONFIG_END

/*** Rom Loading

 "maincpu" for the main code
 "user1" for the banked data
 "sub" for the sound cpu code
 "gfx1" for the 8x8 Txt Layer Tiles
 "gfx2" for the 16x16 Sprites
 "gfx3" for the 8x8 Bg Layer Tiles (top tilemap)
 "gfx4" for the 8x8 Bg Layer Tiles (bottom tilemap)
 "proms" for the Prom (same between games)

*/

ROM_START( angelkds )
	/* Nasco X090-PC-A  (Sega 837-6600) */
	ROM_REGION( 0x8000, "maincpu", 0 )
	ROM_LOAD( "11428.c10",    0x00000, 0x08000, CRC(90daacd2) SHA1(7e50ad1cbed0c1e6bad04ef1611cad25538c905f) )

	ROM_REGION( 0x20000, "user1", 0 ) /* Banked Code */
	ROM_LOAD( "11424.c1",     0x00000, 0x08000, CRC(b55997f6) SHA1(7ed746becac1851f39591f1fdbeff64aa97d6206) )
	ROM_LOAD( "11425.c2",     0x08000, 0x08000, CRC(299359de) SHA1(f531dd3bfe6f64e9e043cb4f85d5657455241dc7) )
	ROM_LOAD( "11426.c3",     0x10000, 0x08000, CRC(5fad8bd3) SHA1(4d865342eb10dcfb779eee4ac1e159bb9ec140cb) )
	ROM_LOAD( "11427.c4",     0x18000, 0x08000, CRC(ef920c74) SHA1(81c0fbe4ace5441e4cd99ba423e0190cc541da31) )

	ROM_REGION( 0x10000, "sub", 0 )
	ROM_LOAD( "11429.d4",     0x00000, 0x08000, CRC(0ca50a66) SHA1(cccb081b447419138b1ebd309e7f291e392a44d5) )

	/* Nasco X090-PC-B */
	ROM_REGION( 0x08000, "gfx1", 0 )
	ROM_LOAD( "11446",        0x00000, 0x08000, CRC(45052470) SHA1(c2312a9f814d6dbe42aa465147a04a2bd9b2aa1b) )

	ROM_REGION( 0x10000, "gfx2", 0 )
	ROM_LOAD( "11447.f7",     0x08000, 0x08000, CRC(b3afc5b3) SHA1(376d527f60e9044f18d19a5535bca77606efbd4c) )
	ROM_LOAD( "11448.h7",     0x00000, 0x08000, CRC(05dab626) SHA1(73feaca6e23c673a7d8c9e972714b20bd8f2d51e) )

	/* both tilemaps on angelkds use the same gfx */
	ROM_REGION( 0x40000, "gfx3", 0 )
	ROM_LOAD( "11437",        0x00000, 0x08000, CRC(a520b628) SHA1(2b51f59e760e740e5e6b06dad61bbc23fc84a72b) )
	ROM_LOAD( "11436",        0x08000, 0x08000, CRC(469ab216) SHA1(8223f072a6f9135ff84841c95410368bcea073d8) )
	ROM_LOAD( "11435",        0x10000, 0x08000, CRC(b0f8c245) SHA1(882e27eaceac46c397fdae8427a082caa7d6b7dc) )
	ROM_LOAD( "11434",        0x18000, 0x08000, CRC(cbde81f5) SHA1(5d5b8e709c9dd09a45dfced6f3d4a9c52500da6b) )
	ROM_LOAD( "11433",        0x20000, 0x08000, CRC(b63fa414) SHA1(25adcafd7e17ab0be0fed2ec44245124febd74b3) )
	ROM_LOAD( "11432",        0x28000, 0x08000, CRC(00dc747b) SHA1(041b73aa48b45162af33b5f416ccc0c0dbbd995b) )
	ROM_LOAD( "11431",        0x30000, 0x08000, CRC(ac2025af) SHA1(2aba145df3ccdb1a7f0fec524bd2de3f9aab4161) )
	ROM_LOAD( "11430",        0x38000, 0x08000, CRC(d640f89e) SHA1(38fb67bcb2a3d1ad614fc62e42f22a66bc757137) )

	ROM_REGION( 0x40000, "gfx4", 0 )
	ROM_LOAD( "11445",        0x00000, 0x08000, CRC(a520b628) SHA1(2b51f59e760e740e5e6b06dad61bbc23fc84a72b) )
	ROM_LOAD( "11444",        0x08000, 0x08000, CRC(469ab216) SHA1(8223f072a6f9135ff84841c95410368bcea073d8) )
	ROM_LOAD( "11443",        0x10000, 0x08000, CRC(b0f8c245) SHA1(882e27eaceac46c397fdae8427a082caa7d6b7dc) )
	ROM_LOAD( "11442",        0x18000, 0x08000, CRC(cbde81f5) SHA1(5d5b8e709c9dd09a45dfced6f3d4a9c52500da6b) )
	ROM_LOAD( "11441",        0x20000, 0x08000, CRC(b63fa414) SHA1(25adcafd7e17ab0be0fed2ec44245124febd74b3) )
	ROM_LOAD( "11440",        0x28000, 0x08000, CRC(00dc747b) SHA1(041b73aa48b45162af33b5f416ccc0c0dbbd995b) )
	ROM_LOAD( "11439",        0x30000, 0x08000, CRC(ac2025af) SHA1(2aba145df3ccdb1a7f0fec524bd2de3f9aab4161) )
	ROM_LOAD( "11438",        0x38000, 0x08000, CRC(d640f89e) SHA1(38fb67bcb2a3d1ad614fc62e42f22a66bc757137) )

	ROM_REGION( 0x20, "proms", 0 )
	ROM_LOAD( "63s081n.u5",   0x00,    0x20,    CRC(36b98627) SHA1(d2d54d92d1d47e7cc85104989ee421ce5d80a42a) )
ROM_END

ROM_START( spcpostn )
	/* X090-PC-A 171-5383 */
	ROM_REGION( 0x8000, "maincpu", 0 ) /* D317-0005 (NEC Z80 Custom) */
	ROM_LOAD( "epr10125.c10", 0x00000, 0x08000, CRC(bffd38c6) SHA1(af02907124343ddecd21439d25f1ebb81ef9f51a) ) /* encrypted */

	ROM_REGION( 0x28000, "user1", 0 ) /* Banked Code */
	ROM_LOAD( "epr10120.c1",  0x00000, 0x08000, CRC(d6399f99) SHA1(4c7d19a8798e5a10b688bf793ca74f5170fd9b51) )
	ROM_LOAD( "epr10121.c2",  0x08000, 0x08000, CRC(d4861560) SHA1(74d28c36a08880abbd3c398cc3e990e8986caccb) )
	ROM_LOAD( "epr10122.c3",  0x10000, 0x08000, CRC(7a1bff1b) SHA1(e1bda8430fd632c1813dd78e0f210a358e1b0d2f) )
	ROM_LOAD( "epr10123.c4",  0x18000, 0x08000, CRC(6aed2925) SHA1(75848c8086c460b72494da2367f592d7d5dcf9f1) )
	ROM_LOAD( "epr10124.c5",  0x20000, 0x08000, CRC(a1d7ae6b) SHA1(ec81fecf63e0515cae2077e2623262227adfdf37) )

	ROM_REGION( 0x10000, "sub", 0 ) /* NEC D780C-1 */
	ROM_LOAD( "epr10126.d4",  0x00000, 0x08000, CRC(ab17f852) SHA1(dc0db427ddb4df97bb40dfb6fc65cb9354a6b9ad) )

	/* X090-PC-B 171-5384 */
	ROM_REGION( 0x08000, "gfx1", 0 )
	ROM_LOAD( "epr10133.17",  0x00000, 0x08000, CRC(642e6609) SHA1(2dfb4cc66f89543b55ed2a5b914e2c9304e821ca) )

	ROM_REGION( 0x10000, "gfx2", 0 )
	ROM_LOAD( "epr10134.18",  0x08000, 0x08000, CRC(c674ff88) SHA1(9f240910a1ffb7c9e09d2326de280e6a5dd84565) )
	ROM_LOAD( "epr10135.19",  0x00000, 0x08000, CRC(0685c4fa) SHA1(6950d9ad9ec13236cf24e83e87adb62aa53af7bb) )

	ROM_REGION( 0x30000, "gfx3", 0 )
	ROM_LOAD( "epr10130.14",  0x10000, 0x08000, CRC(b68fcb36) SHA1(3943dd550b13f2911d56d8dad675410da79196e6) )
	ROM_LOAD( "epr10131.15",  0x08000, 0x08000, CRC(de223817) SHA1(1860db0a19c926fcfaabe676cb57fff38c4df8e6) )
	ROM_LOAD( "epr10132.16",  0x00000, 0x08000, CRC(2df8b1bd) SHA1(cad8befa3f2c158d2aa74073066ccd2b54e68825) )

	ROM_REGION( 0x18000, "gfx4", 0 )
	ROM_LOAD( "epr10127.06",  0x10000, 0x08000, CRC(b68fcb36) SHA1(3943dd550b13f2911d56d8dad675410da79196e6) )
	ROM_LOAD( "epr10128.07",  0x08000, 0x08000, CRC(de223817) SHA1(1860db0a19c926fcfaabe676cb57fff38c4df8e6) )
	ROM_LOAD( "epr10129.08",  0x00000, 0x08000, CRC(a6f21023) SHA1(8d573446a2d3d3428409707d0c59b118d1463131) )

	ROM_REGION( 0x20, "proms", 0 )
	ROM_LOAD( "63s081n.u5",   0x00,    0x20,    CRC(36b98627) SHA1(d2d54d92d1d47e7cc85104989ee421ce5d80a42a) )
ROM_END


DRIVER_INIT_MEMBER(angelkds_state,angelkds)
{
	UINT8 *RAM = memregion("user1")->base();
	membank("bank1")->configure_entries(0, 8, &RAM[0x0000], 0x4000);
}

DRIVER_INIT_MEMBER(angelkds_state,spcpostn)
{
	UINT8 *RAM = memregion("user1")->base();

	// 317-0005
	sega_decode_317(memregion("maincpu")->base(), m_decrypted_opcodes, 1);

	membank("bank1")->configure_entries(0, 10, &RAM[0x0000], 0x4000);
}


GAME( 1988, angelkds, 0, angelkds, angelkds, angelkds_state, angelkds,  ROT90,  "Sega / Nasco?", "Angel Kids (Japan)" ,     MACHINE_SUPPORTS_SAVE) /* Nasco not displayed but 'Exa Planning' is */
GAME( 1986, spcpostn, 0, spcpostn, spcpostn, angelkds_state, spcpostn,  ROT90,  "Sega / Nasco",  "Space Position (Japan)" , MACHINE_SUPPORTS_SAVE) /* encrypted */
