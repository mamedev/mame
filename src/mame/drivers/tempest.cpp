// license:BSD-3-Clause
// copyright-holders:Brad Oliver, Bernd Wiebelt, Allard van der Bas
/***************************************************************************

    Atari Tempest hardware

    Games supported:
        * Tempest [5 sets]
        * Tempest Tubes

    Known bugs:
        * none at this time

****************************************************************************

    TEMPEST
    -------
    HEX        R/W   D7 D6 D5 D4 D3 D2 D2 D0  function
    0000-07FF  R/W   D  D  D  D  D  D  D  D   program ram (2K)
    0800-080F   W                D  D  D  D   Colour ram

    0C00        R                         D   Right coin sw
    0C00        R                      D      Center coin sw
    0C00        R                   D         Left coin sw
    0C00        R                D            Slam sw
    0C00        R             D               Self test sw
    0C00        R          D                  Diagnostic step sw
    0C00        R       D                     Halt
    0C00        R    D                        3kHz ??
    0D00        R    D  D  D  D  D  D  D  D   option switches
    0E00        R    D  D  D  D  D  D  D  D   option switches

    2000-2FFF  R/W   D  D  D  D  D  D  D  D   Vector Ram (4K)
    3000-3FFF   R    D  D  D  D  D  D  D  D   Vector Rom (4K)

    4000        W                         D   Right coin counter
    4000        W                      D      left  coin counter
    4000        W                D            Video invert - x
    4000        W             D               Video invert - y
    4800        W                             Vector generator GO

    5000        W                             WD clear
    5800        W                             Vect gen reset

    6000-603F   W    D  D  D  D  D  D  D  D   EAROM write
    6040        W    D  D  D  D  D  D  D  D   EAROM control
    6040        R    D                        Mathbox status
    6050        R    D  D  D  D  D  D  D  D   EAROM read

    6060        R    D  D  D  D  D  D  D  D   Mathbox read
    6070        R    D  D  D  D  D  D  D  D   Mathbox read
    6080-609F   W    D  D  D  D  D  D  D  D   Mathbox start

    60C0-60CF  R/W   D  D  D  D  D  D  D  D   Custom audio chip 1
    60D0-60DF  R/W   D  D  D  D  D  D  D  D   Custom audio chip 2

    60E0        R                         D   one player start LED
    60E0        R                      D      two player start LED
    60E0        R                   D         FLIP

    9000-DFFF  R     D  D  D  D  D  D  D  D   Program ROM (20K)

    notes: program ram decode may be incorrect, but it appears like
    this on the schematics, and the troubleshooting guide.

    ZAP1,FIRE1,FIRE2,ZAP2 go to pokey2 , bits 3,and 4
    (depending on state of FLIP)
    player1 start, player2 start are pokey2 , bits 5 and 6

    encoder wheel goes to pokey1 bits 0-3
    pokey1, bit4 is cocktail detect


    TEMPEST SWITCH SETTINGS (Atari, 1980)
    -------------------------------------


    GAME OPTIONS:
    (8-position switch at L12 on Analog Vector-Generator PCB)

    1   2   3   4   5   6   7   8   Meaning
    -------------------------------------------------------------------------
    Off Off                         2 lives per game
    On  On                          3 lives per game
    On  Off                         4 lives per game
    Off On                          5 lives per game
            On  On  Off             Bonus life every 10000 pts
            On  On  On              Bonus life every 20000 pts
            On  Off On              Bonus life every 30000 pts
            On  Off Off             Bonus life every 40000 pts
            Off On  On              Bonus life every 50000 pts
            Off On  Off             Bonus life every 60000 pts
            Off Off On              Bonus life every 70000 pts
            Off Off Off             No bonus lives
                        On  On      English
                        On  Off     French
                        Off On      German
                        Off Off     Spanish
                                On  1-credit minimum
                                Off 2-credit minimum


    GAME OPTIONS:
    (4-position switch at D/E2 on Math Box PCB)

    1   2   3   4                   Meaning
    -------------------------------------------------------------------------
        Off                         Minimum rating range: 1, 3, 5, 7, 9
        On                          Minimum rating range tied to high score
            Off Off                 Medium difficulty (see notes)
            Off On                  Easy difficulty (see notes)
            On  Off                 Hard difficulty (see notes)
            On  On                  Medium difficulty (see notes)


    PRICING OPTIONS:
    (8-position switch at N13 on Analog Vector-Generator PCB)

    1   2   3   4   5   6   7   8   Meaning
    -------------------------------------------------------------------------
    On  On  On                      No bonus coins
    On  On  Off                     For every 2 coins, game adds 1 more coin
    On  Off On                      For every 4 coins, game adds 1 more coin
    On  Off Off                     For every 4 coins, game adds 2 more coins
    Off On  On                      For every 5 coins, game adds 1 more coin
    Off On  Off                     For every 3 coins, game adds 1 more coin
    On  Off                 Off On  Demonstration Mode (see notes)
    Off Off                 Off On  Demonstration-Freeze Mode (see notes)
                On                  Left coin mech * 1
                Off                 Left coin mech * 2
                    On  On          Right coin mech * 1
                    On  Off         Right coin mech * 4
                    Off On          Right coin mech * 5
                    Off Off         Right coin mech * 6
                            Off On  Free Play
                            Off Off 1 coin 2 plays
                            On  On  1 coin 1 play
                            On  Off 2 coins 1 play


    GAME SETTING NOTES:
    -------------------

    Demonstration Mode:
    - Plays a normal game of Tempest, but pressing SUPERZAP sends you
      directly to the next level.

    Demonstration-Freeze Mode:
    - Just like Demonstration Mode, but with frozen screen action.

    Both Demonstration Modes:
    - Pressing RESET in either mode will cause the game to lock up.
      To recover, set switch 1 to On.
    - You can start at any level from 1..81, so it's an easy way of
      seeing what the game can throw at you
    - The score is zeroed at the end of the game, so you also don't
      have to worry about artificially high scores disrupting your
      scoring records as stored in the game's EAROM.

    Easy Difficulty:
    - Enemies move more slowly
    - One less enemy shot on the screen at any given time

    Hard Difficulty:
    - Enemies move more quickly
    - 1-4 more enemy shots on the screen at any given time
    - One more enemy may be on the screen at any given time

    High Scores:
    - Changing toggles 1-5 at L12 (more/fewer lives, bonus ship levels)
      will erase the high score table.
    - You should also wait 8-10 seconds after a game has been played
      before entering self-test mode or powering down; otherwise, you
      might erase or corrupt the high score table.

-----------------------------------------

Atari Bulletin, December 4, 1981

Tempest Program Bug

Tempest Uprights Prior to Serial #17426

  If the score on your Tempest(tm) is greater then 170,000, there
is a 12% chance that a program bug may award 40 credits for one
quarter.

  The ROM (#136002-217) in this package, replaces the ROM in
location J-1 on the main PCB and will correct the problem.

  All cabaret and cocktail cabinets will have the correct ROM
Installed.

Thank you,

Fred McCord
Field Service Manager

Tech Tip, December 11, 1981

Tempest(tm) ROM #136002-117

We have found that the above part number in location J1 should be replaced with part
number 136002-217 in the main board in order to eliminate the possibility of receiving
extra bonus plays after 170,000 points.

RMA# T1700

Exchange offer expires on March 15, 1982

-----------------------------------------

Skill-Step(tm) feature of your new Tempest(tm) game:

I. Player rating mode

  1. Occurs at beginning of every game
  2. Player is given 10 seconds to choose his starting level
  3. Player may choose from those levels displayed at bottom of screen
  4. Player chooses level by:
   a. spinning knob until white box surrounds desired level and then
   b. pressing fire or superzapper (or start 1 or start 2 if remaining
      time is less then 8 seconds), or by waiting until timer expires
  5. Player is given a 3 second warning
  6. Level choices are determined by several factors
   a. If the game has been idle for 1 or more attract mode cycles
      since the last game, then the choices are levels 1,3,5,7,9
   b. If a player has just finished a game and pressed start before
      the attract mode has finished its play mode, then the choices
      depend on the highest level reached in that previous game as
      follows

Highest level
reached in
last game            Level choices this game
-------------        -----------------------
1 through 11         1,3,5,7,9
12 or 13             1,3,5,7,9,11
14 or 15             1,3,5,7,9,11,13
16 or 17             1,3,5,7,9,11,13,15
18, 19 or 20         1,3,5,7,9,11,13,15,17
21 or 22             1,3,5,7,9,11,13,15,17,20
23 or 24             1,3,5,7,9,11,13,15,17,20,22
25 or 26             1,3,5,7,9,11,13,15,17,20,22,24
27 or 28             1,3,5,7,9,11,13,15,17,20,22,24,26
29, 30 or 31         1,3,5,7,9,11,13,15,17,20,22,24,26,28
32 or 33             1,3,5,7,9,11,13,15,17,20,22,24,26,28,31
34, 35 or 36         1,3,5,7,9,11,13,15,17,20,22,24,26,28,31,33
37,38,39,40          1,3,5,7,9,11,13,15,17,20,22,24,26,28,31,33,36
41,42,43,44          1,3,5,7,9,11,13,15,17,20,22,24,26,28,31,33,36,40
45, 46 or 47         1,3,5,7,9,11,13,15,17,20,22,24,26,28,31,33,36,40,44
48 or 49             1,3,5,7,9,11,13,15,17,20,22,24,26,28,31,33,36,40,44,47
50, 51 or 52         1,3,5,7,9,11,13,15,17,20,22,24,26,28,31,33,36,40,44,47,49
53,54,55,56          1,3,5,7,9,11,13,15,17,20,22,24,26,28,31,33,36,40,44,47,49,52
57,58,59,60          1,3,5,7,9,11,13,15,17,20,22,24,26,28,31,33,36,40,44,47,49,52,56
61, 62 or 63         1,3,5,7,9,11,13,15,17,20,22,24,26,28,31,33,36,40,44,47,49,52,56,60
64 or 65             1,3,5,7,9,11,13,15,17,20,22,24,26,28,31,33,36,40,44,47,49,52,56,60,63
66 through 73        1,3,5,7,9,11,13,15,17,20,22,24,26,28,31,33,36,40,44,47,49,52,56,60,63,65
74 through 81        1,3,5,7,9,11,13,15,17,20,22,24,26,28,31,33,36,40,44,47,49,52,56,60,63,65,73
82 through 99        1,3,5,7,9,11,13,15,17,20,22,24,26,28,31,33,36,40,44,47,49,52,56,60,63,65,73,81

Rom revisions and bug fixes / effects:

Revision 2
136002-217.j1  Fixes the score cheat (see tech notes above)
136002-222.r1  In test mode, changes spinner letters to a line, described in Tempest manual C0-190-01 New Roms

Revision 3
136002-316.h1  Fixes screen collapse between players when using newer deflection board

136002-134.f1  Contains the 136002-316.h1 code fix even though it's listed as a rev 1 rom

Note: Roms for Tempest Analog Vector-Generator PCB Assembly A037383-03 or A037383-04 are twice
      the size as those for Tempest Analog Vector-Generator PCB Assembly A037383-01 or A037383-02

***************************************************************************/

#include "emu.h"
#include "cpu/m6502/m6502.h"
#include "machine/mathbox.h"
#include "video/avgdvg.h"
#include "video/vector.h"
#include "machine/atari_vg.h"
#include "sound/pokey.h"


#define MASTER_CLOCK (XTAL_12_096MHz)
#define CLOCK_3KHZ   ((double)MASTER_CLOCK / 4096)

#define TEMPEST_KNOB_P1_TAG "KNOBP1"
#define TEMPEST_KNOB_P2_TAG "KNOBP2"
#define TEMPEST_BUTTONS_P1_TAG  "BUTTONSP1"
#define TEMPEST_BUTTONS_P2_TAG  "BUTTONSP2"

class tempest_state : public driver_device
{
public:
	tempest_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_mathbox(*this, "mathbox"),
		m_avg(*this, "avg"),
		m_rom(*this, "maincpu"),
		m_knob_p1(*this, TEMPEST_KNOB_P1_TAG),
		m_knob_p2(*this, TEMPEST_KNOB_P2_TAG),
		m_buttons_p1(*this, TEMPEST_BUTTONS_P1_TAG),
		m_buttons_p2(*this, TEMPEST_BUTTONS_P2_TAG),
		m_in1(*this, "IN1/DSW0"),
		m_in2(*this, "IN2")
	{ }

	required_device<cpu_device> m_maincpu;
	required_device<mathbox_device> m_mathbox;
	required_device<avg_tempest_device> m_avg;
	required_region_ptr<UINT8> m_rom;

	required_ioport m_knob_p1;
	required_ioport m_knob_p2;
	required_ioport m_buttons_p1;
	required_ioport m_buttons_p2;
	required_ioport m_in1;
	required_ioport m_in2;

	UINT8 m_player_select;
	DECLARE_WRITE8_MEMBER(wdclr_w);
	DECLARE_WRITE8_MEMBER(tempest_led_w);
	DECLARE_WRITE8_MEMBER(tempest_coin_w);
	DECLARE_CUSTOM_INPUT_MEMBER(tempest_knob_r);
	DECLARE_CUSTOM_INPUT_MEMBER(tempest_buttons_r);
	DECLARE_CUSTOM_INPUT_MEMBER(clock_r);
	DECLARE_READ8_MEMBER(input_port_1_bit_r);
	DECLARE_READ8_MEMBER(input_port_2_bit_r);

	DECLARE_READ8_MEMBER(rom_ae1f_r);

	virtual void machine_start() override;
};


void tempest_state::machine_start()
{
	save_item(NAME(m_player_select));
}

/*************************************
 *
 *  Interrupt ack
 *
 *************************************/

WRITE8_MEMBER(tempest_state::wdclr_w)
{
	m_maincpu->set_input_line(0, CLEAR_LINE);
	machine().watchdog_reset();
}

/*************************************
 *
 *  Input ports
 *
 *************************************/

CUSTOM_INPUT_MEMBER(tempest_state::tempest_knob_r)
{
	return (m_player_select == 0) ? m_knob_p1->read() : m_knob_p2->read();
}

CUSTOM_INPUT_MEMBER(tempest_state::tempest_buttons_r)
{
	return (m_player_select == 0) ? m_buttons_p1->read() : m_buttons_p2->read();
}


CUSTOM_INPUT_MEMBER(tempest_state::clock_r)
{
	/* Emulate the 3kHz source on bit 7 (divide 1.5MHz by 512) */
	return (m_maincpu->total_cycles() & 0x100) ? 1 : 0;
}


READ8_MEMBER(tempest_state::input_port_1_bit_r)
{
	return (m_in1->read() & (1 << offset)) ? 0 : 228;
}


READ8_MEMBER(tempest_state::input_port_2_bit_r)
{
	return (m_in2->read() & (1 << offset)) ? 0 : 228;
}



/*************************************
 *
 *  Output ports
 *
 *************************************/

WRITE8_MEMBER(tempest_state::tempest_led_w)
{
	output().set_led_value(0, ~data & 0x02);
	output().set_led_value(1, ~data & 0x01);
	/* FLIP is bit 0x04 */
	m_player_select = data & 0x04;
}


WRITE8_MEMBER(tempest_state::tempest_coin_w)
{
	machine().bookkeeping().coin_counter_w(0, (data & 0x01));
	machine().bookkeeping().coin_counter_w(1, (data & 0x02));
	machine().bookkeeping().coin_counter_w(2, (data & 0x04));
	m_avg->set_flip_x(data & 0x08);
	m_avg->set_flip_y(data & 0x10);
}



/*************************************
 *
 *  Main CPU memory handlers
 *
 *************************************/

READ8_MEMBER(tempest_state::rom_ae1f_r)
{
	// This is needed to ensure that the routine starting at ae1c passes checks and does not corrupt data;
	// MCFG_QUANTUM_PERFECT_CPU("maincpu") would be very taxing on this driver.
	machine().scheduler().boost_interleave(attotime::zero, attotime::from_usec(100));
	machine().scheduler().abort_timeslice();

	return m_rom[0xae1f];
}


static ADDRESS_MAP_START( main_map, AS_PROGRAM, 8, tempest_state )
	AM_RANGE(0x0000, 0x07ff) AM_RAM
	AM_RANGE(0x0800, 0x080f) AM_WRITEONLY AM_SHARE("colorram")
	AM_RANGE(0x0c00, 0x0c00) AM_READ_PORT("IN0")
	AM_RANGE(0x0d00, 0x0d00) AM_READ_PORT("DSW1")
	AM_RANGE(0x0e00, 0x0e00) AM_READ_PORT("DSW2")
	AM_RANGE(0x2000, 0x2fff) AM_RAM AM_SHARE("vectorram") AM_REGION("maincpu", 0x2000)
	AM_RANGE(0x3000, 0x3fff) AM_ROM
	AM_RANGE(0x4000, 0x4000) AM_WRITE(tempest_coin_w)
	AM_RANGE(0x4800, 0x4800) AM_DEVWRITE("avg", avg_tempest_device, go_w)
	AM_RANGE(0x5000, 0x5000) AM_WRITE(wdclr_w)
	AM_RANGE(0x5800, 0x5800) AM_DEVWRITE("avg", avg_tempest_device, reset_w)
	AM_RANGE(0x6000, 0x603f) AM_DEVWRITE("earom", atari_vg_earom_device, write)
	AM_RANGE(0x6040, 0x6040) AM_DEVREAD("mathbox", mathbox_device, status_r) AM_DEVWRITE("earom", atari_vg_earom_device, ctrl_w)
	AM_RANGE(0x6050, 0x6050) AM_DEVREAD("earom", atari_vg_earom_device, read)
	AM_RANGE(0x6060, 0x6060) AM_DEVREAD("mathbox", mathbox_device, lo_r)
	AM_RANGE(0x6070, 0x6070) AM_DEVREAD("mathbox", mathbox_device, hi_r)
	AM_RANGE(0x6080, 0x609f) AM_DEVWRITE("mathbox", mathbox_device, go_w)
	AM_RANGE(0x60c0, 0x60cf) AM_DEVREADWRITE("pokey1", pokey_device, read, write)
	AM_RANGE(0x60d0, 0x60df) AM_DEVREADWRITE("pokey2", pokey_device, read, write)
	AM_RANGE(0x60e0, 0x60e0) AM_WRITE(tempest_led_w)
	AM_RANGE(0xae1f, 0xae1f) AM_READ(rom_ae1f_r)
	AM_RANGE(0x9000, 0xdfff) AM_ROM
	AM_RANGE(0xf000, 0xffff) AM_ROM /* for the reset / interrupt vectors */
ADDRESS_MAP_END


/*************************************
 *
 *  Port definitions
 *
 *************************************/

static INPUT_PORTS_START( tempest )
	PORT_START("IN0")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_COIN3 )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_TILT )
	PORT_SERVICE( 0x10, IP_ACTIVE_LOW )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_SERVICE ) PORT_NAME("Diagnostic Step") PORT_CODE(KEYCODE_F1)
	/* bit 6 is the VG HALT bit. We set it to "low" */
	/* per default (busy vector processor). */
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_SPECIAL ) PORT_CUSTOM_MEMBER("avg", avg_tempest_device, done_r, NULL)
	/* bit 7 is tied to a 3kHz (?) clock */
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_SPECIAL ) PORT_CUSTOM_MEMBER(DEVICE_SELF, tempest_state,clock_r, NULL)

	PORT_START("IN1/DSW0")
	PORT_BIT( 0x0f, IP_ACTIVE_HIGH, IPT_SPECIAL ) PORT_CUSTOM_MEMBER(DEVICE_SELF, tempest_state,tempest_knob_r, NULL)
	/* The next one is reponsible for cocktail mode.
	 * According to the documentation, this is not a switch, although
	 * it may have been planned to put it on the Math Box PCB, D/E2 )
	 */
	PORT_DIPNAME( 0x10, 0x10, DEF_STR( Cabinet ) )
	PORT_DIPSETTING(    0x10, DEF_STR( Upright ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Cocktail ) )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("IN2")
	PORT_DIPNAME(  0x03, 0x03, DEF_STR( Difficulty ) ) PORT_DIPLOCATION("DE2:4,3")
	PORT_DIPSETTING(     0x02, DEF_STR( Easy ) )
	PORT_DIPSETTING(     0x03, "Medium1" )
	PORT_DIPSETTING(     0x00, "Medium2" )
	PORT_DIPSETTING(     0x01, DEF_STR( Hard ) )
	PORT_DIPNAME(  0x04, 0x04, "Rating" ) PORT_DIPLOCATION("DE2:2")
	PORT_DIPSETTING(     0x04, "1, 3, 5, 7, 9" )
	PORT_DIPSETTING(     0x00, "tied to high score" )
	PORT_BIT(0x18, IP_ACTIVE_HIGH, IPT_SPECIAL ) PORT_CUSTOM_MEMBER(DEVICE_SELF, tempest_state,tempest_buttons_r, NULL)
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_START2 )
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("DSW1")  /* N13 on analog vector generator PCB */
	PORT_DIPNAME( 0x03, 0x00, DEF_STR( Coinage ) ) PORT_DIPLOCATION("N13:8,7")
	PORT_DIPSETTING(    0x01, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x00, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x03, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0x02, DEF_STR( Free_Play ) )
	PORT_DIPNAME( 0x0c, 0x00, "Right Coin" ) PORT_DIPLOCATION("N13:6,5")
	PORT_DIPSETTING(    0x00, "*1" )
	PORT_DIPSETTING(    0x04, "*4" )
	PORT_DIPSETTING(    0x08, "*5" )
	PORT_DIPSETTING(    0x0c, "*6" )
	PORT_DIPNAME( 0x10, 0x00, "Left Coin" ) PORT_DIPLOCATION("N13:4")
	PORT_DIPSETTING(    0x00, "*1" )
	PORT_DIPSETTING(    0x10, "*2" )
	PORT_DIPNAME( 0xe0, 0x00, "Bonus Coins" ) PORT_DIPLOCATION("N13:3,2,1")
	PORT_DIPSETTING(    0x00, DEF_STR( None ) )
	PORT_DIPSETTING(    0x80, "1 each 5" )
	PORT_DIPSETTING(    0x40, "1 each 4 (+Demo)" )
	PORT_DIPSETTING(    0xa0, "1 each 3" )
	PORT_DIPSETTING(    0x60, "2 each 4 (+Demo)" )
	PORT_DIPSETTING(    0x20, "1 each 2" )
	PORT_DIPSETTING(    0xc0, "Freeze Mode" )
	PORT_DIPSETTING(    0xe0, "Freeze Mode" )

	PORT_START("DSW2")  /* L12 on analog vector generator PCB */
	PORT_DIPNAME( 0x01, 0x00, "Minimum" ) PORT_DIPLOCATION("L12:8")
	PORT_DIPSETTING(    0x00, "1 Credit" )
	PORT_DIPSETTING(    0x01, "2 Credit" )
	PORT_DIPNAME( 0x06, 0x00, DEF_STR( Language ) ) PORT_DIPLOCATION("L12:7,6")
	PORT_DIPSETTING(    0x00, DEF_STR( English ) )
	PORT_DIPSETTING(    0x02, DEF_STR( French ) )
	PORT_DIPSETTING(    0x04, DEF_STR( German ) )
	PORT_DIPSETTING(    0x06, DEF_STR( Spanish ) )
	PORT_DIPNAME( 0x38, 0x00, DEF_STR( Bonus_Life ) ) PORT_DIPLOCATION("L12:5,4,3")
	PORT_DIPSETTING(    0x08, "10000" )
	PORT_DIPSETTING(    0x00, "20000" )
	PORT_DIPSETTING(    0x10, "30000" )
	PORT_DIPSETTING(    0x18, "40000" )
	PORT_DIPSETTING(    0x20, "50000" )
	PORT_DIPSETTING(    0x28, "60000" )
	PORT_DIPSETTING(    0x30, "70000" )
	PORT_DIPSETTING(    0x38, DEF_STR( None ) )
	PORT_DIPNAME( 0xc0, 0x00, DEF_STR( Lives ) ) PORT_DIPLOCATION("L12:2,1")
	PORT_DIPSETTING(    0xc0, "2" )
	PORT_DIPSETTING(    0x00, "3" )
	PORT_DIPSETTING(    0x40, "4" )
	PORT_DIPSETTING(    0x80, "5" )

	PORT_START(TEMPEST_KNOB_P1_TAG)
	/* This is the Tempest spinner input. It only uses 4 bits. */
	PORT_BIT( 0x0f, 0x00, IPT_DIAL ) PORT_SENSITIVITY(100) PORT_KEYDELTA(20) PORT_PLAYER(1) PORT_FULL_TURN_COUNT(72)
	PORT_BIT( 0xf0, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START(TEMPEST_KNOB_P2_TAG)
	/* This is the Tempest spinner input. It only uses 4 bits. */
	PORT_BIT( 0x0f, 0x00, IPT_DIAL ) PORT_SENSITIVITY(100) PORT_KEYDELTA(20) PORT_PLAYER(2) PORT_FULL_TURN_COUNT(72)
	PORT_BIT( 0xf0, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START(TEMPEST_BUTTONS_P1_TAG)
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(1)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(1)
	PORT_BIT( 0xfc, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START(TEMPEST_BUTTONS_P2_TAG)
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(2)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(2)
	PORT_BIT( 0xfc, IP_ACTIVE_LOW, IPT_UNUSED )
INPUT_PORTS_END


/*************************************
 *
 *  Machine drivers
 *
 *************************************/

static MACHINE_CONFIG_START( tempest, tempest_state )

	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", M6502, MASTER_CLOCK / 8)
	MCFG_CPU_PROGRAM_MAP(main_map)

	MCFG_CPU_PERIODIC_INT_DRIVER(tempest_state, irq0_line_assert, CLOCK_3KHZ / 12)
	MCFG_WATCHDOG_TIME_INIT(attotime::from_hz(CLOCK_3KHZ / 256))

	MCFG_ATARIVGEAROM_ADD("earom")

	/* video hardware */
	MCFG_VECTOR_ADD("vector")
	MCFG_SCREEN_ADD("screen", VECTOR)
	MCFG_SCREEN_REFRESH_RATE(60)
	MCFG_SCREEN_SIZE(400, 300)
	MCFG_SCREEN_VISIBLE_AREA(0, 580, 0, 570)
	MCFG_SCREEN_UPDATE_DEVICE("vector", vector_device, screen_update)

	MCFG_DEVICE_ADD("avg", AVG_TEMPEST, 0)
	MCFG_AVGDVG_VECTOR("vector")

	/* Drivers */
	MCFG_MATHBOX_ADD("mathbox")

	/* sound hardware */
	MCFG_SPEAKER_STANDARD_MONO("mono")

	MCFG_SOUND_ADD("pokey1", POKEY, MASTER_CLOCK / 8)
	MCFG_POKEY_POT0_R_CB(READ8(tempest_state, input_port_1_bit_r))
	MCFG_POKEY_POT1_R_CB(READ8(tempest_state, input_port_1_bit_r))
	MCFG_POKEY_POT2_R_CB(READ8(tempest_state, input_port_1_bit_r))
	MCFG_POKEY_POT3_R_CB(READ8(tempest_state, input_port_1_bit_r))
	MCFG_POKEY_POT4_R_CB(READ8(tempest_state, input_port_1_bit_r))
	MCFG_POKEY_POT5_R_CB(READ8(tempest_state, input_port_1_bit_r))
	MCFG_POKEY_POT6_R_CB(READ8(tempest_state, input_port_1_bit_r))
	MCFG_POKEY_POT7_R_CB(READ8(tempest_state, input_port_1_bit_r))
	MCFG_POKEY_OUTPUT_RC(RES_K(10), CAP_U(0.015), 5.0)

	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.5)

	MCFG_SOUND_ADD("pokey2", POKEY, MASTER_CLOCK / 8)
	MCFG_POKEY_POT0_R_CB(READ8(tempest_state, input_port_2_bit_r))
	MCFG_POKEY_POT1_R_CB(READ8(tempest_state, input_port_2_bit_r))
	MCFG_POKEY_POT2_R_CB(READ8(tempest_state, input_port_2_bit_r))
	MCFG_POKEY_POT3_R_CB(READ8(tempest_state, input_port_2_bit_r))
	MCFG_POKEY_POT4_R_CB(READ8(tempest_state, input_port_2_bit_r))
	MCFG_POKEY_POT5_R_CB(READ8(tempest_state, input_port_2_bit_r))
	MCFG_POKEY_POT6_R_CB(READ8(tempest_state, input_port_2_bit_r))
	MCFG_POKEY_POT7_R_CB(READ8(tempest_state, input_port_2_bit_r))
	MCFG_POKEY_OUTPUT_RC(RES_K(10), CAP_U(0.015), 5.0)

	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.5)
MACHINE_CONFIG_END



/*************************************
 *
 *  ROM definitions
 *
 *************************************/

ROM_START( tempest ) /* rev 3 */
	ROM_REGION( 0x10000, "maincpu", 0 )
	/* Roms are for Tempest Analog Vector-Generator PCB Assembly A037383-03 or A037383-04 */
	ROM_LOAD( "136002-133.d1",  0x9000, 0x1000, CRC(1d0cc503) SHA1(7bef95db9b1102d6b1166bda0ccb276ef4cc3764) ) /* 136002-113 + 136002-114 */
	ROM_LOAD( "136002-134.f1",  0xa000, 0x1000, CRC(c88e3524) SHA1(89144baf1efc703b2336774793ce345b37829ee7) ) /* 136002-115 + 136002-316 */
	ROM_LOAD( "136002-235.j1",  0xb000, 0x1000, CRC(a4b2ce3f) SHA1(a5f5fb630a48c5d25346f90d4c13aaa98f60b228) ) /* 136002-217 + 136002-118 */
	ROM_LOAD( "136002-136.lm1", 0xc000, 0x1000, CRC(65a9a9f9) SHA1(73aa7d6f4e7093ccb2d97f6344f354872bcfd72a) ) /* 136002-119 + 136002-120 */
	ROM_LOAD( "136002-237.p1",  0xd000, 0x1000, CRC(de4e9e34) SHA1(04be074e45bf5cd95a852af97cd04e35b7f27fc4) ) /* 136002-121 + 136002-222 */
	ROM_RELOAD(                 0xf000, 0x1000 ) /* for reset/interrupt vectors */
	/* Vector ROM */
	ROM_LOAD( "136002-138.np3", 0x3000, 0x1000, CRC(9995256d) SHA1(2b725ee1a57d423c7d7377a1744f48412e0f2f69) )

	/* AVG PROM */
	ROM_REGION( 0x100, "user1", 0 )
	ROM_LOAD( "136002-125.d7",   0x0000, 0x0100, CRC(5903af03) SHA1(24bc0366f394ad0ec486919212e38be0f08d0239) )

	/* Mathbox PROMs */
	ROM_REGION( 0x20, "user2", 0 )
	ROM_LOAD( "136002.126",   0x0000, 0x0020, CRC(8b04f921) SHA1(317b3397482f13b2d1bc21f296d3b3f9a118787b) )

	ROM_REGION32_BE( 0x400, "user3", 0 )
	ROMX_LOAD( "136002.132", 0, 0x100, CRC(2af82e87) SHA1(3816835a9ccf99a76d246adf204989d9261bb065), ROM_NIBBLE | ROM_SHIFT_NIBBLE_LO | ROM_SKIP(3))
	ROMX_LOAD( "136002.131", 0, 0x100, CRC(b31f6e24) SHA1(ce5f8ca34d06a5cfa0076b47400e61e0130ffe74), ROM_NIBBLE | ROM_SHIFT_NIBBLE_HI | ROM_SKIP(3))
	ROMX_LOAD( "136002.130", 1, 0x100, CRC(8119b847) SHA1(c4fbaedd4ce1ad6a4128cbe902b297743edb606a), ROM_NIBBLE | ROM_SHIFT_NIBBLE_LO | ROM_SKIP(3))
	ROMX_LOAD( "136002.129", 1, 0x100, CRC(09f5a4d5) SHA1(d6f2ac07ca9ee385c08831098b0dcaf56808993b), ROM_NIBBLE | ROM_SHIFT_NIBBLE_HI | ROM_SKIP(3))
	ROMX_LOAD( "136002.128", 2, 0x100, CRC(823b61ae) SHA1(d99a839874b45f64e14dae92a036e47a53705d16), ROM_NIBBLE | ROM_SHIFT_NIBBLE_LO | ROM_SKIP(3))
	ROMX_LOAD( "136002.127", 2, 0x100, CRC(276eadd5) SHA1(55718cd8ec4bcf75076d5ef0ee1ed2551e19d9ba), ROM_NIBBLE | ROM_SHIFT_NIBBLE_HI | ROM_SKIP(3))
ROM_END


ROM_START( tempest1r ) /* rev 1 */
	ROM_REGION( 0x10000, "maincpu", 0 )
	/* Roms are for Tempest Analog Vector-Generator PCB Assembly A037383-03 or A037383-04 */
	ROM_LOAD( "136002-133.d1",  0x9000, 0x1000, CRC(1d0cc503) SHA1(7bef95db9b1102d6b1166bda0ccb276ef4cc3764) ) /* 136002-113 + 136002-114 */
	ROM_LOAD( "136002-134.f1",  0xa000, 0x1000, CRC(c88e3524) SHA1(89144baf1efc703b2336774793ce345b37829ee7) ) /* 136002-115 + 136002-316 */
	ROM_LOAD( "136002-135.j1",  0xb000, 0x1000, CRC(1ca27781) SHA1(cafbec28d682e98a74fdd5b8dfcfa33c64ff6a93) ) /* 136002-117 + 136002-118 */
	ROM_LOAD( "136002-136.lm1", 0xc000, 0x1000, CRC(65a9a9f9) SHA1(73aa7d6f4e7093ccb2d97f6344f354872bcfd72a) ) /* 136002-119 + 136002-120 */
	ROM_LOAD( "136002-137.p1",  0xd000, 0x1000, CRC(d75fd2ef) SHA1(19f611a77989d201d346b3e89fac5789663a01ce) ) /* 136002-121 + 136002-122 */
	ROM_RELOAD(                 0xf000, 0x1000 ) /* for reset/interrupt vectors */
	/* Vector ROM */
	ROM_LOAD( "136002-138.np3", 0x3000, 0x1000, CRC(9995256d) SHA1(2b725ee1a57d423c7d7377a1744f48412e0f2f69) )

	/* AVG PROM */
	ROM_REGION( 0x100, "user1", 0 )
	ROM_LOAD( "136002-125.d7",   0x0000, 0x0100, CRC(5903af03) SHA1(24bc0366f394ad0ec486919212e38be0f08d0239) )

	/* Mathbox PROMs */
	ROM_REGION( 0x20, "user2", 0 )
	ROM_LOAD( "136002.126",   0x0000, 0x0020, CRC(8b04f921) SHA1(317b3397482f13b2d1bc21f296d3b3f9a118787b) )

	ROM_REGION32_BE( 0x400, "user3", 0 )
	ROMX_LOAD( "136002.132", 0, 0x100, CRC(2af82e87) SHA1(3816835a9ccf99a76d246adf204989d9261bb065), ROM_NIBBLE | ROM_SHIFT_NIBBLE_LO | ROM_SKIP(3))
	ROMX_LOAD( "136002.131", 0, 0x100, CRC(b31f6e24) SHA1(ce5f8ca34d06a5cfa0076b47400e61e0130ffe74), ROM_NIBBLE | ROM_SHIFT_NIBBLE_HI | ROM_SKIP(3))
	ROMX_LOAD( "136002.130", 1, 0x100, CRC(8119b847) SHA1(c4fbaedd4ce1ad6a4128cbe902b297743edb606a), ROM_NIBBLE | ROM_SHIFT_NIBBLE_LO | ROM_SKIP(3))
	ROMX_LOAD( "136002.129", 1, 0x100, CRC(09f5a4d5) SHA1(d6f2ac07ca9ee385c08831098b0dcaf56808993b), ROM_NIBBLE | ROM_SHIFT_NIBBLE_HI | ROM_SKIP(3))
	ROMX_LOAD( "136002.128", 2, 0x100, CRC(823b61ae) SHA1(d99a839874b45f64e14dae92a036e47a53705d16), ROM_NIBBLE | ROM_SHIFT_NIBBLE_LO | ROM_SKIP(3))
	ROMX_LOAD( "136002.127", 2, 0x100, CRC(276eadd5) SHA1(55718cd8ec4bcf75076d5ef0ee1ed2551e19d9ba), ROM_NIBBLE | ROM_SHIFT_NIBBLE_HI | ROM_SKIP(3))
ROM_END


ROM_START( tempest3 ) /* rev 3 */
	ROM_REGION( 0x10000, "maincpu", 0 )
	/* Roms are for Tempest Analog Vector-Generator PCB Assembly A037383-01 or A037383-02 */
	ROM_LOAD( "136002-113.d1",   0x9000, 0x0800, CRC(65d61fe7) SHA1(38a1e8a8f65b7887cf3e190269fe4ce2c6f818aa) )
	ROM_LOAD( "136002-114.e1",   0x9800, 0x0800, CRC(11077375) SHA1(ed8ff0ca969da6672a7683b93d4fcf2935a0d903) )
	ROM_LOAD( "136002-115.f1",   0xa000, 0x0800, CRC(f3e2827a) SHA1(bd04fcfbbba995e08c3144c1474fcddaaeb1c700) )
	ROM_LOAD( "136002-316.h1",   0xa800, 0x0800, CRC(aeb0f7e9) SHA1(a5cc25015b98692673cfc1c7c2e9634efd750870) )
	ROM_LOAD( "136002-217.j1",   0xb000, 0x0800, CRC(ef2eb645) SHA1(b1a2c969e8897e335d5354de6ae04a65d4b2a1e4) )
	ROM_LOAD( "136002-118.k1",   0xb800, 0x0800, CRC(beb352ab) SHA1(f213166d3970e0bd0f29d8dea8d6afa6990cce38) )
	ROM_LOAD( "136002-119.lm1",  0xc000, 0x0800, CRC(a4de050f) SHA1(ea302e43a313a5a18115e74ddbaaedde0fbecda7) )
	ROM_LOAD( "136002-120.mn1",  0xc800, 0x0800, CRC(35619648) SHA1(48f1e8bed7ec6afa0b4c549a30e5ec331c071e40) )
	ROM_LOAD( "136002-121.p1",   0xd000, 0x0800, CRC(73d38e47) SHA1(9980606376a79ba94f8e2a325871a6c8d10d83fc) )
	ROM_LOAD( "136002-222.r1",   0xd800, 0x0800, CRC(707bd5c3) SHA1(2f0af6fb7154c244c794f7247e5c16a1e06ddf7d) )
	ROM_RELOAD(                  0xf800, 0x0800 ) /* for reset/interrupt vectors */
	/* Vector ROM */
	ROM_LOAD( "136002-123.np3",  0x3000, 0x0800, CRC(29f7e937) SHA1(686c8b9b8901262e743497cee7f2f7dd5cb3af7e) ) /* May be labeled "136002-111", same data */
	ROM_LOAD( "136002-124.r3",   0x3800, 0x0800, CRC(c16ec351) SHA1(a30a3662c740810c0f20e3712679606921b8ca06) ) /* May be labeled "136002-112", same data */

	/* AVG PROM */
	ROM_REGION( 0x100, "user1", 0 )
	ROM_LOAD( "136002-125.d7",   0x0000, 0x0100, CRC(5903af03) SHA1(24bc0366f394ad0ec486919212e38be0f08d0239) )

	/* Mathbox PROMs */
	ROM_REGION( 0x20, "user2", 0 )
	ROM_LOAD( "136002.126",   0x0000, 0x0020, CRC(8b04f921) SHA1(317b3397482f13b2d1bc21f296d3b3f9a118787b) )

	ROM_REGION32_BE( 0x400, "user3", 0 )
	ROMX_LOAD( "136002.132", 0, 0x100, CRC(2af82e87) SHA1(3816835a9ccf99a76d246adf204989d9261bb065), ROM_NIBBLE | ROM_SHIFT_NIBBLE_LO | ROM_SKIP(3))
	ROMX_LOAD( "136002.131", 0, 0x100, CRC(b31f6e24) SHA1(ce5f8ca34d06a5cfa0076b47400e61e0130ffe74), ROM_NIBBLE | ROM_SHIFT_NIBBLE_HI | ROM_SKIP(3))
	ROMX_LOAD( "136002.130", 1, 0x100, CRC(8119b847) SHA1(c4fbaedd4ce1ad6a4128cbe902b297743edb606a), ROM_NIBBLE | ROM_SHIFT_NIBBLE_LO | ROM_SKIP(3))
	ROMX_LOAD( "136002.129", 1, 0x100, CRC(09f5a4d5) SHA1(d6f2ac07ca9ee385c08831098b0dcaf56808993b), ROM_NIBBLE | ROM_SHIFT_NIBBLE_HI | ROM_SKIP(3))
	ROMX_LOAD( "136002.128", 2, 0x100, CRC(823b61ae) SHA1(d99a839874b45f64e14dae92a036e47a53705d16), ROM_NIBBLE | ROM_SHIFT_NIBBLE_LO | ROM_SKIP(3))
	ROMX_LOAD( "136002.127", 2, 0x100, CRC(276eadd5) SHA1(55718cd8ec4bcf75076d5ef0ee1ed2551e19d9ba), ROM_NIBBLE | ROM_SHIFT_NIBBLE_HI | ROM_SKIP(3))
ROM_END


ROM_START( tempest2 ) /* rev 2 */
	ROM_REGION( 0x10000, "maincpu", 0 )
	/* Roms are for Tempest Analog Vector-Generator PCB Assembly A037383-01 or A037383-02 */
	ROM_LOAD( "136002-113.d1",   0x9000, 0x0800, CRC(65d61fe7) SHA1(38a1e8a8f65b7887cf3e190269fe4ce2c6f818aa) )
	ROM_LOAD( "136002-114.e1",   0x9800, 0x0800, CRC(11077375) SHA1(ed8ff0ca969da6672a7683b93d4fcf2935a0d903) )
	ROM_LOAD( "136002-115.f1",   0xa000, 0x0800, CRC(f3e2827a) SHA1(bd04fcfbbba995e08c3144c1474fcddaaeb1c700) )
	ROM_LOAD( "136002-116.h1",   0xa800, 0x0800, CRC(7356896c) SHA1(a013ede292189a8f5a907de882ee1a573d784b3c) )
	ROM_LOAD( "136002-217.j1",   0xb000, 0x0800, CRC(ef2eb645) SHA1(b1a2c969e8897e335d5354de6ae04a65d4b2a1e4) )
	ROM_LOAD( "136002-118.k1",   0xb800, 0x0800, CRC(beb352ab) SHA1(f213166d3970e0bd0f29d8dea8d6afa6990cce38) )
	ROM_LOAD( "136002-119.lm1",  0xc000, 0x0800, CRC(a4de050f) SHA1(ea302e43a313a5a18115e74ddbaaedde0fbecda7) )
	ROM_LOAD( "136002-120.mn1",  0xc800, 0x0800, CRC(35619648) SHA1(48f1e8bed7ec6afa0b4c549a30e5ec331c071e40) )
	ROM_LOAD( "136002-121.p1",   0xd000, 0x0800, CRC(73d38e47) SHA1(9980606376a79ba94f8e2a325871a6c8d10d83fc) )
	ROM_LOAD( "136002-222.r1",   0xd800, 0x0800, CRC(707bd5c3) SHA1(2f0af6fb7154c244c794f7247e5c16a1e06ddf7d) )
	ROM_RELOAD(                  0xf800, 0x0800 ) /* for reset/interrupt vectors */
	/* Vector ROM */
	ROM_LOAD( "136002-123.np3",  0x3000, 0x0800, CRC(29f7e937) SHA1(686c8b9b8901262e743497cee7f2f7dd5cb3af7e) ) /* May be labeled "136002-111", same data */
	ROM_LOAD( "136002-124.r3",   0x3800, 0x0800, CRC(c16ec351) SHA1(a30a3662c740810c0f20e3712679606921b8ca06) ) /* May be labeled "136002-112", same data */

	/* AVG PROM */
	ROM_REGION( 0x100, "user1", 0 )
	ROM_LOAD( "136002-125.d7",   0x0000, 0x0100, CRC(5903af03) SHA1(24bc0366f394ad0ec486919212e38be0f08d0239) )

	/* Mathbox PROMs */
	ROM_REGION( 0x20, "user2", 0 )
	ROM_LOAD( "136002.126",   0x0000, 0x0020, CRC(8b04f921) SHA1(317b3397482f13b2d1bc21f296d3b3f9a118787b) )

	ROM_REGION32_BE( 0x400, "user3", 0 )
	ROMX_LOAD( "136002.132", 0, 0x100, CRC(2af82e87) SHA1(3816835a9ccf99a76d246adf204989d9261bb065), ROM_NIBBLE | ROM_SHIFT_NIBBLE_LO | ROM_SKIP(3))
	ROMX_LOAD( "136002.131", 0, 0x100, CRC(b31f6e24) SHA1(ce5f8ca34d06a5cfa0076b47400e61e0130ffe74), ROM_NIBBLE | ROM_SHIFT_NIBBLE_HI | ROM_SKIP(3))
	ROMX_LOAD( "136002.130", 1, 0x100, CRC(8119b847) SHA1(c4fbaedd4ce1ad6a4128cbe902b297743edb606a), ROM_NIBBLE | ROM_SHIFT_NIBBLE_LO | ROM_SKIP(3))
	ROMX_LOAD( "136002.129", 1, 0x100, CRC(09f5a4d5) SHA1(d6f2ac07ca9ee385c08831098b0dcaf56808993b), ROM_NIBBLE | ROM_SHIFT_NIBBLE_HI | ROM_SKIP(3))
	ROMX_LOAD( "136002.128", 2, 0x100, CRC(823b61ae) SHA1(d99a839874b45f64e14dae92a036e47a53705d16), ROM_NIBBLE | ROM_SHIFT_NIBBLE_LO | ROM_SKIP(3))
	ROMX_LOAD( "136002.127", 2, 0x100, CRC(276eadd5) SHA1(55718cd8ec4bcf75076d5ef0ee1ed2551e19d9ba), ROM_NIBBLE | ROM_SHIFT_NIBBLE_HI | ROM_SKIP(3))
ROM_END


ROM_START( tempest1 ) /* rev 1 */
	ROM_REGION( 0x10000, "maincpu", 0 )
	/* Roms are for Tempest Analog Vector-Generator PCB Assembly A037383-01 or A037383-02 */
	ROM_LOAD( "136002-113.d1",   0x9000, 0x0800, CRC(65d61fe7) SHA1(38a1e8a8f65b7887cf3e190269fe4ce2c6f818aa) )
	ROM_LOAD( "136002-114.e1",   0x9800, 0x0800, CRC(11077375) SHA1(ed8ff0ca969da6672a7683b93d4fcf2935a0d903) )
	ROM_LOAD( "136002-115.f1",   0xa000, 0x0800, CRC(f3e2827a) SHA1(bd04fcfbbba995e08c3144c1474fcddaaeb1c700) )
	ROM_LOAD( "136002-116.h1",   0xa800, 0x0800, CRC(7356896c) SHA1(a013ede292189a8f5a907de882ee1a573d784b3c) )
	ROM_LOAD( "136002-117.j1",   0xb000, 0x0800, CRC(55952119) SHA1(470d914fa52fce3786cb6330889876d3547dca65) )
	ROM_LOAD( "136002-118.k1",   0xb800, 0x0800, CRC(beb352ab) SHA1(f213166d3970e0bd0f29d8dea8d6afa6990cce38) )
	ROM_LOAD( "136002-119.lm1",  0xc000, 0x0800, CRC(a4de050f) SHA1(ea302e43a313a5a18115e74ddbaaedde0fbecda7) )
	ROM_LOAD( "136002-120.mn1",  0xc800, 0x0800, CRC(35619648) SHA1(48f1e8bed7ec6afa0b4c549a30e5ec331c071e40) )
	ROM_LOAD( "136002-121.p1",   0xd000, 0x0800, CRC(73d38e47) SHA1(9980606376a79ba94f8e2a325871a6c8d10d83fc) )
	ROM_LOAD( "136002-122.r1",   0xd800, 0x0800, CRC(796a9918) SHA1(c862a0d4ea330161e4c3cc8e5e9ad38893fffbd4) )
	ROM_RELOAD(                  0xf800, 0x0800 ) /* for reset/interrupt vectors */
	/* Vector ROM */
	ROM_LOAD( "136002-123.np3",  0x3000, 0x0800, CRC(29f7e937) SHA1(686c8b9b8901262e743497cee7f2f7dd5cb3af7e) ) /* May be labeled "136002-111", same data */
	ROM_LOAD( "136002-124.r3",   0x3800, 0x0800, CRC(c16ec351) SHA1(a30a3662c740810c0f20e3712679606921b8ca06) ) /* May be labeled "136002-112", same data */

	/* AVG PROM */
	ROM_REGION( 0x100, "user1", 0 )
	ROM_LOAD( "136002-125.d7",   0x0000, 0x0100, CRC(5903af03) SHA1(24bc0366f394ad0ec486919212e38be0f08d0239) )

	/* Mathbox PROMs */
	ROM_REGION( 0x20, "user2", 0 )
	ROM_LOAD( "136002.126",   0x0000, 0x0020, CRC(8b04f921) SHA1(317b3397482f13b2d1bc21f296d3b3f9a118787b) )

	ROM_REGION32_BE( 0x400, "user3", 0 )
	ROMX_LOAD( "136002.132", 0, 0x100, CRC(2af82e87) SHA1(3816835a9ccf99a76d246adf204989d9261bb065), ROM_NIBBLE | ROM_SHIFT_NIBBLE_LO | ROM_SKIP(3))
	ROMX_LOAD( "136002.131", 0, 0x100, CRC(b31f6e24) SHA1(ce5f8ca34d06a5cfa0076b47400e61e0130ffe74), ROM_NIBBLE | ROM_SHIFT_NIBBLE_HI | ROM_SKIP(3))
	ROMX_LOAD( "136002.130", 1, 0x100, CRC(8119b847) SHA1(c4fbaedd4ce1ad6a4128cbe902b297743edb606a), ROM_NIBBLE | ROM_SHIFT_NIBBLE_LO | ROM_SKIP(3))
	ROMX_LOAD( "136002.129", 1, 0x100, CRC(09f5a4d5) SHA1(d6f2ac07ca9ee385c08831098b0dcaf56808993b), ROM_NIBBLE | ROM_SHIFT_NIBBLE_HI | ROM_SKIP(3))
	ROMX_LOAD( "136002.128", 2, 0x100, CRC(823b61ae) SHA1(d99a839874b45f64e14dae92a036e47a53705d16), ROM_NIBBLE | ROM_SHIFT_NIBBLE_LO | ROM_SKIP(3))
	ROMX_LOAD( "136002.127", 2, 0x100, CRC(276eadd5) SHA1(55718cd8ec4bcf75076d5ef0ee1ed2551e19d9ba), ROM_NIBBLE | ROM_SHIFT_NIBBLE_HI | ROM_SKIP(3))
ROM_END


ROM_START( temptube )
	ROM_REGION( 0x10000, "maincpu", 0 )
	/* Roms are for Tempest Analog Vector-Generator PCB Assembly A037383-01 or A037383-02 */
	ROM_LOAD( "136002-113.d1",   0x9000, 0x0800, CRC(65d61fe7) SHA1(38a1e8a8f65b7887cf3e190269fe4ce2c6f818aa) )
	ROM_LOAD( "136002-114.e1",   0x9800, 0x0800, CRC(11077375) SHA1(ed8ff0ca969da6672a7683b93d4fcf2935a0d903) )
	ROM_LOAD( "136002-115.f1",   0xa000, 0x0800, CRC(f3e2827a) SHA1(bd04fcfbbba995e08c3144c1474fcddaaeb1c700) )
	ROM_LOAD( "136002-316.h1",   0xa800, 0x0800, CRC(aeb0f7e9) SHA1(a5cc25015b98692673cfc1c7c2e9634efd750870) )
	ROM_LOAD( "136002-217.j1",   0xb000, 0x0800, CRC(ef2eb645) SHA1(b1a2c969e8897e335d5354de6ae04a65d4b2a1e4) )
	ROM_LOAD( "tube-118.k1",     0xb800, 0x0800, CRC(cefb03f0) SHA1(41ddfa4991fa49a31d4740a04551556acca66196) )
	ROM_LOAD( "136002-119.lm1",  0xc000, 0x0800, CRC(a4de050f) SHA1(ea302e43a313a5a18115e74ddbaaedde0fbecda7) )
	ROM_LOAD( "136002-120.mn1",  0xc800, 0x0800, CRC(35619648) SHA1(48f1e8bed7ec6afa0b4c549a30e5ec331c071e40) )
	ROM_LOAD( "136002-121.p1",   0xd000, 0x0800, CRC(73d38e47) SHA1(9980606376a79ba94f8e2a325871a6c8d10d83fc) )
	ROM_LOAD( "136002-222.r1",   0xd800, 0x0800, CRC(707bd5c3) SHA1(2f0af6fb7154c244c794f7247e5c16a1e06ddf7d) )
	ROM_RELOAD(                  0xf800, 0x0800 ) /* for reset/interrupt vectors */
	/* Vector ROM */
	ROM_LOAD( "136002-123.np3",  0x3000, 0x0800, CRC(29f7e937) SHA1(686c8b9b8901262e743497cee7f2f7dd5cb3af7e) ) /* May be labeled "136002-111", same data */
	ROM_LOAD( "136002-124.r3",   0x3800, 0x0800, CRC(c16ec351) SHA1(a30a3662c740810c0f20e3712679606921b8ca06) ) /* May be labeled "136002-112", same data */

	/* AVG PROM */
	ROM_REGION( 0x100, "user1", 0 )
	ROM_LOAD( "136002-125.d7",   0x0000, 0x0100, CRC(5903af03) SHA1(24bc0366f394ad0ec486919212e38be0f08d0239) )

	/* Mathbox PROMs */
	ROM_REGION( 0x20, "user2", 0 )
	ROM_LOAD( "136002.126",   0x0000, 0x0020, CRC(8b04f921) SHA1(317b3397482f13b2d1bc21f296d3b3f9a118787b) )

	ROM_REGION32_BE( 0x400, "user3", 0 )
	ROMX_LOAD( "136002.132", 0, 0x100, CRC(2af82e87) SHA1(3816835a9ccf99a76d246adf204989d9261bb065), ROM_NIBBLE | ROM_SHIFT_NIBBLE_LO | ROM_SKIP(3))
	ROMX_LOAD( "136002.131", 0, 0x100, CRC(b31f6e24) SHA1(ce5f8ca34d06a5cfa0076b47400e61e0130ffe74), ROM_NIBBLE | ROM_SHIFT_NIBBLE_HI | ROM_SKIP(3))
	ROMX_LOAD( "136002.130", 1, 0x100, CRC(8119b847) SHA1(c4fbaedd4ce1ad6a4128cbe902b297743edb606a), ROM_NIBBLE | ROM_SHIFT_NIBBLE_LO | ROM_SKIP(3))
	ROMX_LOAD( "136002.129", 1, 0x100, CRC(09f5a4d5) SHA1(d6f2ac07ca9ee385c08831098b0dcaf56808993b), ROM_NIBBLE | ROM_SHIFT_NIBBLE_HI | ROM_SKIP(3))
	ROMX_LOAD( "136002.128", 2, 0x100, CRC(823b61ae) SHA1(d99a839874b45f64e14dae92a036e47a53705d16), ROM_NIBBLE | ROM_SHIFT_NIBBLE_LO | ROM_SKIP(3))
	ROMX_LOAD( "136002.127", 2, 0x100, CRC(276eadd5) SHA1(55718cd8ec4bcf75076d5ef0ee1ed2551e19d9ba), ROM_NIBBLE | ROM_SHIFT_NIBBLE_HI | ROM_SKIP(3))
ROM_END


/*************************************
 *
 *  Game drivers
 *
 *************************************/

GAME( 1980, tempest,   0,       tempest, tempest, driver_device, 0, ROT270, "Atari", "Tempest (rev 3, Revised Hardware)", MACHINE_SUPPORTS_SAVE )
GAME( 1980, tempest3,  tempest, tempest, tempest, driver_device, 0, ROT270, "Atari", "Tempest (rev 3)", MACHINE_SUPPORTS_SAVE )
GAME( 1980, tempest2,  tempest, tempest, tempest, driver_device, 0, ROT270, "Atari", "Tempest (rev 2)", MACHINE_SUPPORTS_SAVE )
GAME( 1980, tempest1,  tempest, tempest, tempest, driver_device, 0, ROT270, "Atari", "Tempest (rev 1)", MACHINE_SUPPORTS_SAVE )
GAME( 1980, tempest1r, tempest, tempest, tempest, driver_device, 0, ROT270, "Atari", "Tempest (rev 1, Revised Hardware)", MACHINE_SUPPORTS_SAVE )
GAME( 1980, temptube,  tempest, tempest, tempest, driver_device, 0, ROT270, "hack (Duncan Brown)", "Tempest Tubes", MACHINE_SUPPORTS_SAVE )
