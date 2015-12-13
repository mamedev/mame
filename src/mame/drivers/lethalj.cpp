// license:BSD-3-Clause
// copyright-holders:Aaron Giles
/***************************************************************************

    The Game Room Lethal Justice hardware

    driver by Aaron Giles

    Games supported:
        * Lethal Justice
        * Egg Venture
        * Ripper Ribit
        * Chicken Farm
        * Crazzy Clownz

    Note: I.C.E. is Innovative Concepts in Entertainment

****************************************************************************

Egg Venture
The Gameroom, 1997

PCB Layout
----------

(C) 1996 I.C.E.
|-------------------------------------------------------|
|           EU21.U21  EU18.U18  EU20.U20        32MHz   |
|                                             |-------| |
|           M6295     M6295     M6295         |XILINX | |
|           2MHz      2MHz      2MHz          |XC3042 | |
|  DSW(8)                        KYLR1        |-------| |
|J       EGR9.VC9                             EGR4.GR4  |
|A DSW(4)EGR8.VC8               |-------|               |
|M      M5M442256               |XILINX |     EGR6.GR6  |
|M      M5M442256   11.0592MHz  |XC3042 |               |
|A      M5M442256     MACH210   |-------|     EGR3.GR3  |
|4116R  M5M442256     MACH210 MACH210 MACH210           |
|4116R  |--------|               W241024      EGR1.GR2  |
|       |TMS34010|               W241024                |
|2803A  |-50     |               W241024      EGR1.GR1  |
|       |        |               W241024                |
| 40MHz |--------|                            EGR5.GR5  |
|                         BT121 MACH210 MACH210 MACH210 |
|-------------------------------------------------------|

Notes:
      TMS34010  - TMS34010FNL-50 CPU, clock input 20.000MHz [40/2] (PLCC68)
      M6295     - Clock input 2.000MHz, pin 7 HIGH (QFP44)
      XC3042    - XILINX XC3042 FPGA (PLCC84)
      BT121     - BT121KPJ80 Triple 8-bit 80MHz Video DAC (PLCC44)
      MACH210   - AMD MACH210A-10JC Complex Programmable Logic Device (CPLD, PLCC44)
      2803A     - ST ULN2803A Eight Darlington Transistor Arrays With Common Emitters (DIP18)
      4116R     - 4116R-001 Bourns Type 4100R Series Resistor Network (DIP16)
      M5M442256 - Mitsubishi M5M442256AL-8 256k x4 DRAM (ZIP28)
      W241024   - Winbond W241024AK-20 128 x8 SRAM (NDIP32)
      KYLR1     - 8 Pin Gun Connector

Note 1: Some PCBs use a 11.2896MHz OSC instead of the 11.0592MHz
Note 2: Some PCBs use a TMS34010FNL-40 instead of the TMS34010FNL-50

                Egg Venture & Lethal Justice JAMMA Pinout

                          Main Jamma Connector
            Solder Side          |             Parts Side
------------------------------------------------------------------
             GND             | A | 1 |             GND
             GND             | B | 2 |             GND
             +5              | C | 3 |             +5
             +5              | D | 4 |             +5
             -5              | E | 5 |             -5
             +12             | F | 6 |             +12
------------ KEY ------------| H | 7 |------------ KEY -----------
                             | J | 8 |      Coin Counter # 1
                             | K | 9 |
        L Speaker (-)        | L | 10|        L Speaker (+)
        R Speaker (-)        | M | 11|        R Speaker (+)
        Video Green          | N | 12|        Video Red
        Video Sync           | P | 13|        Video Blue
       Service Switch        | R | 14|        Video GND
                             | S | 15|
        Coin Switch 2        | T | 16|         Coin Switch 1
       Start Player 2        | U | 17|        Start Player 1
                             | V | 18|
                             | W | 19|
                             | X | 20|
                             | Y | 21|
                             | Z | 22|
                             | a | 23|
                             | b | 24|
                             | c | 25|
                             | d | 26|
             GND             | e | 27|             GND
             GND             | f | 28|             GND


 Gun Connector Pinout

   1| +5 Volts
   2| Gun OPTO Player 1
   3| Gun OPTO Player 2
   4| NOT USED
   5| Gun Trigger Player 1
   6| NOT USED
   7| Gun Trigger Player 2
   8| KEY
   9| Ground

+5v and GND are wired to both player 1 & 2


The Egg Venture/Lethal Justice PCB does NOT supply an amplified
sound signal.  An external sound AMP is required

Addition information for Sound & AMP hookup:

                 Power AMP                 /|
           +-------------------+          / |
           |                   |   -------| | Right Speaker
          [| Volume/Gain       |   |  ----| |
           |                 R+|]--|  |   \ |
          [| BASS            R-|]-----|    \|
           |                   |
          [| Treble          L+|]-----|    /|
           |                 L-|]--|  |   / |
           |                   |   |  ----| | Left Speaker
Pin #L (-) |                   |   -------| |
 --------->|\                  |          \ |
Pin #10(+) | | L I             |           \|
 --------->|/    N             |
           |     P             |
Pin #M (-) |     U             |
 --------->|\    T             |
Pin #11(+) | | R               |
 --------->|/               RMT|]--|           +12 Volts
           |                +12|]--+----------- To Power Source
           |                GND|]--------------
           +-------------------+               Ground

***************************************************************************/

#include "emu.h"
#include "cpu/tms34010/tms34010.h"
#include "includes/lethalj.h"
#include "sound/okim6295.h"


#define MASTER_CLOCK            XTAL_40MHz
#define SOUND_CLOCK             XTAL_2MHz

#define VIDEO_CLOCK             XTAL_11_289MHz
#define VIDEO_CLOCK_LETHALJ     XTAL_11_0592MHz



/*************************************
 *
 *  Custom inputs
 *
 *************************************/

CUSTOM_INPUT_MEMBER(lethalj_state::cclownz_paddle)
{
	int value = m_paddle->read();
	return ((value << 4) & 0xf00) | (value & 0x00f);
}



/*************************************
 *
 *  Output controls
 *
 *************************************/

WRITE16_MEMBER(lethalj_state::ripribit_control_w)
{
	coin_counter_w(machine(), 0, data & 1);
	m_ticket->write(space, 0, ((data >> 1) & 1) << 7);
	output_set_lamp_value(0, (data >> 2) & 1);
}


WRITE16_MEMBER(lethalj_state::cfarm_control_w)
{
	m_ticket->write(space, 0, ((data >> 0) & 1) << 7);
	output_set_lamp_value(0, (data >> 2) & 1);
	output_set_lamp_value(1, (data >> 3) & 1);
	output_set_lamp_value(2, (data >> 4) & 1);
	coin_counter_w(machine(), 0, (data >> 7) & 1);
}


WRITE16_MEMBER(lethalj_state::cclownz_control_w)
{
	m_ticket->write(space, 0, ((data >> 0) & 1) << 7);
	output_set_lamp_value(0, (data >> 2) & 1);
	output_set_lamp_value(1, (data >> 4) & 1);
	output_set_lamp_value(2, (data >> 5) & 1);
	coin_counter_w(machine(), 0, (data >> 6) & 1);
}



/*************************************
 *
 *  Memory maps
 *
 *************************************/

static ADDRESS_MAP_START( lethalj_map, AS_PROGRAM, 16, lethalj_state )
	AM_RANGE(0x00000000, 0x003fffff) AM_RAM
	AM_RANGE(0x04000000, 0x0400000f) AM_DEVREADWRITE8("oki1", okim6295_device, read, write, 0x00ff)
	AM_RANGE(0x04000010, 0x0400001f) AM_DEVREADWRITE8("oki2", okim6295_device, read, write, 0x00ff)
	AM_RANGE(0x04100000, 0x0410000f) AM_DEVREADWRITE8("oki3", okim6295_device, read, write, 0x00ff)
//  AM_RANGE(0x04100010, 0x0410001f) AM_READNOP     /* read but never examined */
	AM_RANGE(0x04200000, 0x0420001f) AM_WRITENOP    /* clocks bits through here */
	AM_RANGE(0x04300000, 0x0430007f) AM_READ(lethalj_gun_r)
	AM_RANGE(0x04400000, 0x0440000f) AM_WRITENOP    /* clocks bits through here */
	AM_RANGE(0x04500010, 0x0450001f) AM_READ_PORT("IN0")
	AM_RANGE(0x04600000, 0x0460000f) AM_READ_PORT("IN1")
	AM_RANGE(0x04700000, 0x0470007f) AM_WRITE(lethalj_blitter_w)
	AM_RANGE(0xc0000000, 0xc00001ff) AM_DEVREADWRITE("maincpu", tms34010_device, io_register_r, io_register_w)
	AM_RANGE(0xc0000240, 0xc000025f) AM_WRITENOP    /* seems to be a bug in their code, one of many. */
	AM_RANGE(0xff800000, 0xffffffff) AM_ROM AM_REGION("user1", 0)
ADDRESS_MAP_END



/*************************************
 *
 *  Input ports
 *
 *************************************/

static INPUT_PORTS_START( lethalj )
	PORT_START("IN0")
	PORT_BIT( 0x0003, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(2)
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_UNKNOWN )      /* ??? Seems to be rigged up to the auto scroll, and acts as a fast forward*/
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(1)
	PORT_BIT( 0xffe0, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("IN1")
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_SERVICE1 )
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_DIPNAME( 0x00c0, 0x0000, DEF_STR( Coinage ) )
	PORT_DIPSETTING(      0x0040, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(      0x0080, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(      0x00c0, DEF_STR( Free_Play ) )
	PORT_DIPNAME( 0x0300, 0x0100, DEF_STR( Lives ) )
	PORT_DIPSETTING(      0x0000, "2" )
	PORT_DIPSETTING(      0x0100, "3" )
	PORT_DIPSETTING(      0x0200, "4" )
	PORT_DIPSETTING(      0x0300, "5" )
	PORT_DIPNAME( 0x0c10, 0x0010, "Right Gun Offset" )
	PORT_DIPSETTING(      0x0000, "-4" )
	PORT_DIPSETTING(      0x0400, "-3" )
	PORT_DIPSETTING(      0x0800, "-2" )
	PORT_DIPSETTING(      0x0c00, "-1" )
	PORT_DIPSETTING(      0x0010, "0" )
	PORT_DIPSETTING(      0x0410, "+1" )
	PORT_DIPSETTING(      0x0810, "+2" )
	PORT_DIPSETTING(      0x0c10, "+3" )
	PORT_DIPNAME( 0x3020, 0x0020, "Left Gun Offset" )
	PORT_DIPSETTING(      0x0000, "-4" )
	PORT_DIPSETTING(      0x1000, "-3" )
	PORT_DIPSETTING(      0x2000, "-2" )
	PORT_DIPSETTING(      0x3000, "-1" )
	PORT_DIPSETTING(      0x0020, "0" )
	PORT_DIPSETTING(      0x1020, "+1" )
	PORT_DIPSETTING(      0x2020, "+2" )
	PORT_DIPSETTING(      0x3020, "+3" )
	PORT_DIPNAME( 0x4000, 0x0000, "DIP E" )
	PORT_DIPSETTING(      0x0000, "0" )
	PORT_DIPSETTING(      0x4000, "1" )
	PORT_DIPNAME( 0x8000, 0x8000, "Global Gun Offset" )
	PORT_DIPSETTING(      0x0000, "-2.5" )
	PORT_DIPSETTING(      0x8000, "+0" )

	PORT_START("LIGHT0_X")          /* fake analog X */
	PORT_BIT( 0xff, 0x80, IPT_LIGHTGUN_X ) PORT_CROSSHAIR(X, 1.0, 0.0, 0) PORT_SENSITIVITY(50) PORT_KEYDELTA(10)

	PORT_START("LIGHT0_Y")              /* fake analog Y */
	PORT_BIT( 0xff, 0x80, IPT_LIGHTGUN_Y ) PORT_CROSSHAIR(Y, 1.0, 0.0, 0) PORT_SENSITIVITY(70) PORT_KEYDELTA(10)

	PORT_START("LIGHT1_X")              /* fake analog X */
	PORT_BIT( 0xff, 0x80, IPT_LIGHTGUN_X ) PORT_CROSSHAIR(X, 1.0, 0.0, 0) PORT_SENSITIVITY(50) PORT_KEYDELTA(10) PORT_PLAYER(2)

	PORT_START("LIGHT1_Y")              /* fake analog Y */
	PORT_BIT( 0xff, 0x80, IPT_LIGHTGUN_Y ) PORT_CROSSHAIR(Y, 1.0, 0.0, 0) PORT_SENSITIVITY(70) PORT_KEYDELTA(10) PORT_PLAYER(2)
INPUT_PORTS_END


static INPUT_PORTS_START( eggventr )
	PORT_START("IN0")
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_SERVICE1 )
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_DIPNAME( 0x0070, 0x0000, DEF_STR( Coinage ) )  PORT_DIPLOCATION("SW1:4,3,2")
	PORT_DIPSETTING(      0x0040, DEF_STR( 8C_1C ) )
	PORT_DIPSETTING(      0x0030, DEF_STR( 4C_1C ) )
	PORT_DIPSETTING(      0x0020, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(      0x0010, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(      0x0050, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(      0x0060, DEF_STR( 1C_4C ) )
	PORT_DIPSETTING(      0x0070, DEF_STR( Free_Play ) )
	PORT_DIPUNUSED_DIPLOC( 0x0080, IP_ACTIVE_LOW, "SW1:1" )                 // 4-position switch - switch 1 not used
	PORT_DIPNAME( 0x0300, 0x0200, DEF_STR( Lives ) )    PORT_DIPLOCATION("SW3:8,7") // Verified Correct
	PORT_DIPSETTING(      0x0000, "3" )
	PORT_DIPSETTING(      0x0100, "4" )
	PORT_DIPSETTING(      0x0200, "5" )
	PORT_DIPSETTING(      0x0300, "6" )
	PORT_DIPNAME( 0x0c00, 0x0400, DEF_STR( Difficulty ) )   PORT_DIPLOCATION("SW3:6,5") // According to info from The Gameroom / Manual
	PORT_DIPSETTING(      0x0c00, DEF_STR( Very_Easy) )
	PORT_DIPSETTING(      0x0800, DEF_STR( Easy ) )
	PORT_DIPSETTING(      0x0400, DEF_STR( Medium ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( Hard ) )
	PORT_DIPNAME( 0x1000, 0x1000, "Slot Machine" )      PORT_DIPLOCATION("SW3:4")   // Verified Correct
	PORT_DIPSETTING(      0x0000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x1000, DEF_STR( On ) )
	PORT_DIPUNUSED_DIPLOC( 0x2000, IP_ACTIVE_LOW, "SW3:3" )                 // Manual says switches 1-3 are reserved
	PORT_DIPUNUSED_DIPLOC( 0x4000, IP_ACTIVE_LOW, "SW3:2" )
	PORT_DIPUNUSED_DIPLOC( 0x8000, IP_ACTIVE_LOW, "SW3:1" )

	PORT_START("IN1")
	PORT_BIT( 0x000f, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(1)
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(2)
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x7f00, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x8000, IP_ACTIVE_LOW, IPT_START2 )

	PORT_START("LIGHT0_X")              /* fake analog X */
	PORT_BIT( 0xff, 0x80, IPT_LIGHTGUN_X ) PORT_CROSSHAIR(X, 1.0, 0.0, 0) PORT_SENSITIVITY(50) PORT_KEYDELTA(10)

	PORT_START("LIGHT0_Y")              /* fake analog Y */
	PORT_BIT( 0xff, 0x80, IPT_LIGHTGUN_Y ) PORT_CROSSHAIR(Y, 1.0, 0.0, 0) PORT_SENSITIVITY(70) PORT_KEYDELTA(10)

	PORT_START("LIGHT1_X")              /* fake analog X */
	PORT_BIT( 0xff, 0x80, IPT_LIGHTGUN_X ) PORT_CROSSHAIR(X, 1.0, 0.0, 0) PORT_SENSITIVITY(50) PORT_KEYDELTA(10) PORT_PLAYER(2)

	PORT_START("LIGHT1_Y")              /* fake analog Y */
	PORT_BIT( 0xff, 0x80, IPT_LIGHTGUN_Y ) PORT_CROSSHAIR(Y, 1.0, 0.0, 0) PORT_SENSITIVITY(70) PORT_KEYDELTA(10) PORT_PLAYER(2)
INPUT_PORTS_END

static INPUT_PORTS_START( eggventr2 )
	PORT_INCLUDE(eggventr)

	PORT_MODIFY("IN0")
	PORT_DIPNAME( 0x0300, 0x0200, DEF_STR( Lives ) )    PORT_DIPLOCATION("SW3:8,7") // Verified Correct - 1 extra life per setting
	PORT_DIPSETTING(      0x0000, "4" )
	PORT_DIPSETTING(      0x0100, "5" )
	PORT_DIPSETTING(      0x0200, "6" )
	PORT_DIPSETTING(      0x0300, "7" )
INPUT_PORTS_END

static INPUT_PORTS_START( eggvntdx )
	PORT_INCLUDE(eggventr)

	PORT_MODIFY("IN0")
	PORT_DIPUNUSED_DIPLOC( 0x1000, IP_ACTIVE_LOW, "SW3:4" ) // Was "Slot Machine" - The slot machince is present in the code as a 'bonus stage'
								//  (when the egg reaches Vegas?), but not actually called (EC).
INPUT_PORTS_END


static INPUT_PORTS_START( ripribit )
	PORT_START("IN0")
	PORT_BIT( 0x000f, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(1)
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_SPECIAL ) PORT_READ_LINE_DEVICE_MEMBER("ticket", ticket_dispenser_device, line_r)
	PORT_BIT( 0xffc0, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("IN1")
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_SERVICE1 )
	PORT_SERVICE_NO_TOGGLE( 0x0002, IP_ACTIVE_HIGH )
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_DIPNAME( 0x0010, 0x0010, "Bonus Ticket Increment" )
	PORT_DIPSETTING(      0x0010, "+1 Ticket Per Game" )
	PORT_DIPSETTING(      0x0000, "Fixed" )
	PORT_DIPNAME( 0x0020, 0x0020, DEF_STR( Demo_Sounds ) )
	PORT_DIPSETTING(      0x0020, DEF_STR( On ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( Off ) )
	PORT_DIPNAME( 0x00c0, 0x0000, DEF_STR( Coinage ) )
	PORT_DIPSETTING(      0x0080, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(      0x0040, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(      0x00c0, DEF_STR( 1C_2C ) )
	PORT_DIPNAME( 0x0700, 0x0200, "Starting Jackpot" )
	PORT_DIPSETTING(      0x0000, "0" )
	PORT_DIPSETTING(      0x0100, "5" )
	PORT_DIPSETTING(      0x0200, "10" )
	PORT_DIPSETTING(      0x0300, "15" )
	PORT_DIPSETTING(      0x0400, "20" )
	PORT_DIPSETTING(      0x0500, "25" )
	PORT_DIPSETTING(      0x0600, "30" )
	PORT_DIPSETTING(      0x0700, "35" )
	PORT_DIPNAME( 0x1800, 0x1800, "Bonus Screen Setting" )
	PORT_DIPSETTING(      0x0000, "1" )
	PORT_DIPSETTING(      0x0800, "2" )
	PORT_DIPSETTING(      0x1000, "3" )
	PORT_DIPSETTING(      0x1800, "4" )
	PORT_DIPNAME( 0xe000, 0x8000, "Points per Ticket" )
	PORT_DIPSETTING(      0xe000, "200" )
	PORT_DIPSETTING(      0xc000, "300" )
	PORT_DIPSETTING(      0xa000, "400" )
	PORT_DIPSETTING(      0x8000, "500" )
	PORT_DIPSETTING(      0x6000, "600" )
	PORT_DIPSETTING(      0x4000, "700" )
	PORT_DIPSETTING(      0x2000, "800" )
	PORT_DIPSETTING(      0x0000, "1000" )
INPUT_PORTS_END


static INPUT_PORTS_START( cfarm )
	PORT_START("IN0")
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_SERVICE1 )
	PORT_SERVICE_NO_TOGGLE( 0x0002, IP_ACTIVE_HIGH )
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_DIPNAME( 0x0010, 0x0010, "Bonus Ticket Increment" )
	PORT_DIPSETTING(      0x0010, "+1 Ticket Per Game" )
	PORT_DIPSETTING(      0x0000, "Fixed" )
	PORT_DIPNAME( 0x0020, 0x0020, DEF_STR( Demo_Sounds ) )
	PORT_DIPSETTING(      0x0020, DEF_STR( On ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( Off ) )
	PORT_DIPNAME( 0x00c0, 0x0000, DEF_STR( Coinage ) )
	PORT_DIPSETTING(      0x0080, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(      0x0040, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(      0x00c0, DEF_STR( 1C_2C ) )
	PORT_DIPNAME( 0x0700, 0x0300, "Starting Jackpot" )
	PORT_DIPSETTING(      0x0000, "0" )
	PORT_DIPSETTING(      0x0100, "5" )
	PORT_DIPSETTING(      0x0200, "8" )
	PORT_DIPSETTING(      0x0300, "10" )
	PORT_DIPSETTING(      0x0400, "12" )
	PORT_DIPSETTING(      0x0500, "15" )
	PORT_DIPSETTING(      0x0600, "18" )
	PORT_DIPSETTING(      0x0700, "20" )
	PORT_DIPNAME( 0x1800, 0x1800, "Bonus Screen Setting" )
	PORT_DIPSETTING(      0x0000, "1" )
	PORT_DIPSETTING(      0x0800, "2" )
	PORT_DIPSETTING(      0x1000, "3" )
	PORT_DIPSETTING(      0x1800, "4" )
	PORT_DIPNAME( 0xe000, 0x8000, "Eggs per Ticket" )
	PORT_DIPSETTING(      0xe000, "1" )
	PORT_DIPSETTING(      0xc000, "2" )
	PORT_DIPSETTING(      0xa000, "3" )
	PORT_DIPSETTING(      0x8000, "4" )
	PORT_DIPSETTING(      0x6000, "5" )
	PORT_DIPSETTING(      0x4000, "6" )
	PORT_DIPSETTING(      0x2000, "8" )
	PORT_DIPSETTING(      0x0000, "10" )

	PORT_START("IN1")
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_SPECIAL ) PORT_READ_LINE_DEVICE_MEMBER("ticket", ticket_dispenser_device, line_r)
	PORT_BIT( 0x0006, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_PLAYER(1)
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(1)
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(1)
	PORT_BIT( 0xffc0, IP_ACTIVE_LOW, IPT_UNUSED )
INPUT_PORTS_END


static INPUT_PORTS_START( cclownz )
	PORT_START("IN0")
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_SERVICE1 )
	PORT_SERVICE_NO_TOGGLE( 0x0002, IP_ACTIVE_HIGH )
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_DIPNAME( 0x0010, 0x0010, "Bonus Ticket Increment" )
	PORT_DIPSETTING(      0x0010, "+1 Ticket Per Game" )
	PORT_DIPSETTING(      0x0000, "Fixed" )
	PORT_DIPNAME( 0x0020, 0x0020, DEF_STR( Demo_Sounds ) )
	PORT_DIPSETTING(      0x0020, DEF_STR( On ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( Off ) )
	PORT_DIPNAME( 0x00c0, 0x0000, DEF_STR( Coinage ) )
	PORT_DIPSETTING(      0x0080, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(      0x0040, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(      0x00c0, DEF_STR( 1C_2C ) )
	PORT_DIPNAME( 0x0700, 0x0700, "Starting Jackpot" )
	PORT_DIPSETTING(      0x0000, "0" )
	PORT_DIPSETTING(      0x0100, "2" )
	PORT_DIPSETTING(      0x0200, "5" )
	PORT_DIPSETTING(      0x0300, "8" )
	PORT_DIPSETTING(      0x0400, "10" )
	PORT_DIPSETTING(      0x0500, "15" )
	PORT_DIPSETTING(      0x0600, "20" )
	PORT_DIPSETTING(      0x0700, "30" )
	PORT_DIPNAME( 0x1800, 0x1800, "Bonus Screen Settings" )
	PORT_DIPSETTING(      0x0000, "1" )
	PORT_DIPSETTING(      0x0800, "2" )
	PORT_DIPSETTING(      0x1000, "3" )
	PORT_DIPSETTING(      0x1800, "4" )
	PORT_DIPNAME( 0xe000, 0x8000, "Points per Ticket" )
	PORT_DIPSETTING(      0xe000, "700" )
	PORT_DIPSETTING(      0xc000, "900" )
	PORT_DIPSETTING(      0xa000, "1200" )
	PORT_DIPSETTING(      0x8000, "1500" )
	PORT_DIPSETTING(      0x6000, "1800" )
	PORT_DIPSETTING(      0x4000, "2100" )
	PORT_DIPSETTING(      0x2000, "2400" )
	PORT_DIPSETTING(      0x0000, "3000" )

	PORT_START("IN1")
	PORT_BIT( 0x0f0f, IP_ACTIVE_HIGH, IPT_SPECIAL ) PORT_CUSTOM_MEMBER(DEVICE_SELF, lethalj_state,cclownz_paddle, NULL)
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_SPECIAL ) PORT_READ_LINE_DEVICE_MEMBER("ticket", ticket_dispenser_device, line_r)
	PORT_BIT( 0x0060, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0xf000, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("PADDLE")
	PORT_BIT( 0x00ff, 0x0000, IPT_PADDLE ) PORT_PLAYER(1) PORT_SENSITIVITY(50) PORT_KEYDELTA(8) PORT_CENTERDELTA(0) PORT_REVERSE
INPUT_PORTS_END


static INPUT_PORTS_START( franticf ) // how do the directional inputs work?
	PORT_START("IN0")
	PORT_DIPNAME( 0x0001, 0x0001, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0001, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0002, 0x0002, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0002, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0004, 0x0004, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0004, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0008, 0x0008, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0008, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0010, 0x0010, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0010, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0020, 0x0020, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0020, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0040, 0x0040, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0040, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0080, 0x0080, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0080, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0100, 0x0100, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0100, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0200, 0x0200, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0200, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0400, 0x0400, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0400, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0800, 0x0800, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0800, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x1000, 0x1000, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x1000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x2000, 0x2000, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x2000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x4000, 0x4000, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x4000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x8000, 0x8000, "x" )
	PORT_DIPSETTING(      0x8000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )

	PORT_START("IN1")
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_SERVICE1 )
	PORT_DIPNAME( 0x0002, 0x0002, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0002, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_DIPNAME( 0x0010, 0x0010, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0010, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0020, 0x0020, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0020, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0040, 0x0040, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0040, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0080, 0x0080, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0080, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0100, 0x0100, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0100, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0200, 0x0200, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0200, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x1c00, 0x0400, "Number of Fruit" )
	PORT_DIPSETTING(      0x0000, "3" )
	PORT_DIPSETTING(      0x0400, "5" )
	PORT_DIPSETTING(      0x0800, "7" )
	PORT_DIPSETTING(      0x0c00, "9" )
	PORT_DIPSETTING(      0x1000, "9 (duplicate 1)" ) // appear to be duplicates but could affect something else too
	PORT_DIPSETTING(      0x1400, "9 (duplicate 2)" )
	PORT_DIPSETTING(      0x1800, "9 (duplicate 3)" )
	PORT_DIPSETTING(      0x1c00, "9 (duplicate 4)" )
	PORT_DIPNAME( 0x6000, 0x2000, "Initial Fruit Values" )
	PORT_DIPSETTING(      0x0000, "Lowest" )
	PORT_DIPSETTING(      0x2000, "Low" )
	PORT_DIPSETTING(      0x4000, "Medium" )
	PORT_DIPSETTING(      0x6000, "High" )
	PORT_DIPNAME( 0x8000, 0x0000, DEF_STR( Demo_Sounds ) )
	PORT_DIPSETTING(      0x8000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )

//  PORT_START("PADDLE")
//  PORT_BIT( 0x00ff, 0x0000, IPT_PADDLE ) PORT_PLAYER(1) PORT_SENSITIVITY(50) PORT_KEYDELTA(8) PORT_CENTERDELTA(0) PORT_REVERSE
INPUT_PORTS_END


/*************************************
 *
 *  Machine drivers
 *
 *************************************/

static MACHINE_CONFIG_START( gameroom, lethalj_state )

	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", TMS34010, MASTER_CLOCK)
	MCFG_CPU_PROGRAM_MAP(lethalj_map)
	MCFG_TMS340X0_HALT_ON_RESET(FALSE) /* halt on reset */
	MCFG_TMS340X0_PIXEL_CLOCK(VIDEO_CLOCK) /* pixel clock */
	MCFG_TMS340X0_PIXELS_PER_CLOCK(1) /* pixels per clock */
	MCFG_TMS340X0_SCANLINE_IND16_CB(lethalj_state, scanline_update)     /* scanline updater (indexed16) */

	MCFG_TICKET_DISPENSER_ADD("ticket", attotime::from_msec(200), TICKET_MOTOR_ACTIVE_HIGH, TICKET_STATUS_ACTIVE_HIGH)

	/* video hardware */
	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_RAW_PARAMS(VIDEO_CLOCK, 701, 0, 512, 263, 0, 236)
	MCFG_SCREEN_UPDATE_DEVICE("maincpu", tms34010_device, tms340x0_ind16)
	MCFG_SCREEN_PALETTE("palette")

	MCFG_PALETTE_ADD_RRRRRGGGGGBBBBB("palette")

	/* sound hardware */
	MCFG_SPEAKER_STANDARD_MONO("mono")

	MCFG_OKIM6295_ADD("oki1", SOUND_CLOCK, OKIM6295_PIN7_HIGH)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.8)

	MCFG_OKIM6295_ADD("oki2", SOUND_CLOCK, OKIM6295_PIN7_HIGH)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.8)

	MCFG_OKIM6295_ADD("oki3", SOUND_CLOCK, OKIM6295_PIN7_HIGH)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.8)
MACHINE_CONFIG_END


static MACHINE_CONFIG_DERIVED( lethalj, gameroom )

	MCFG_CPU_MODIFY("maincpu")
	MCFG_TMS340X0_PIXEL_CLOCK(VIDEO_CLOCK_LETHALJ) /* pixel clock */

	MCFG_SCREEN_MODIFY("screen")
	MCFG_SCREEN_RAW_PARAMS(VIDEO_CLOCK_LETHALJ, 689, 0, 512, 259, 0, 236)
MACHINE_CONFIG_END



/*************************************
 *
 *  ROM definitions
 *
 *************************************/

ROM_START( lethalj )
	ROM_REGION16_LE( 0x100000, "user1", 0 )     /* 34010 code */
	ROM_LOAD16_BYTE( "lethal_vc8_2.3.vc8",  0x000000, 0x080000, CRC(8d568e1d) SHA1(e4dd3794789f9ccd7be8374978a3336f2b79136f) ) /* Labeled as LETHAL VC8 2.3, also found labeled as VC-8 */
	ROM_LOAD16_BYTE( "lethal_vc9_2.3.vc9",  0x000001, 0x080000, CRC(8f22add4) SHA1(e773d3ae9cf512810fc266e784d21ed115c8830c) ) /* Labeled as LETHAL VC9 2.3, also found labeled as VC-9 */

	ROM_REGION16_LE( 0x600000, "gfx1", 0 )          /* graphics data */
	ROM_LOAD16_BYTE( "gr1.gr1",             0x000000, 0x100000, CRC(27f7b244) SHA1(628b29c066e217e1fe54553ea3ed98f86735e262) ) /* These had non specific GRx labels, also found labeled as GR-x */
	ROM_LOAD16_BYTE( "gr2.gr2",             0x000001, 0x100000, CRC(1f25d3ab) SHA1(bdb8a3c546cdee9a5630c47b9c5079a956e8a093) )
	ROM_LOAD16_BYTE( "gr4.gr4",             0x200000, 0x100000, CRC(c5838b4c) SHA1(9ad03d0f316eb31fdf0ca6f65c02a27d3406d072) )
	ROM_LOAD16_BYTE( "gr3.gr3",             0x200001, 0x100000, CRC(ba9fa057) SHA1(db6f11a8964870f04f94fef6f1b1a58168a942ad) )
	ROM_LOAD16_BYTE( "lethal_gr6_2.3.gr6",  0x400000, 0x100000, CRC(51c99b85) SHA1(9a23bf21a73d2884b49c64a8f42c288534c79dc5) ) /* Labeled as LETHAL GR6 2.3, also found labeled as GR-6 */
	ROM_LOAD16_BYTE( "lethal_gr5_2.3.gr5",  0x400001, 0x100000, CRC(80dda9b5) SHA1(d8a79cad112bc7d9e4ba31a950e4807581f3bf46) ) /* Labeled as LETHAL GR5 2.3, also found labeled as GR-5 */

	ROM_REGION( 0x40000, "oki1", 0 )                /* sound data */
	ROM_LOAD( "sound1.u20", 0x00000, 0x40000, CRC(7d93ca66) SHA1(9e1dc0efa5d0f770c7e1f10de56fbf5620dea437) )

	ROM_REGION( 0x40000, "oki2", 0 )                /* sound data */
	ROM_LOAD( "sound1.u21", 0x00000, 0x40000, CRC(7d3beae0) SHA1(5ec753c5fd5ca0f9492c9e274703a1aa758062a7) )

	ROM_REGION( 0x40000, "oki3", 0 )                /* sound data */
	ROM_LOAD( "sound1.u18", 0x00000, 0x40000, CRC(7d93ca66) SHA1(9e1dc0efa5d0f770c7e1f10de56fbf5620dea437) )
ROM_END


ROM_START( eggventr )
	ROM_REGION16_LE( 0x100000, "user1", 0 )     /* 34010 code */
	ROM_LOAD16_BYTE( "evc8.10.vc8", 0x000000, 0x020000, CRC(225d1164) SHA1(b0dc55f2e8ded1fe7874de05987fcf879772289e) ) /* Labeled as EVC8.10 */
	ROM_LOAD16_BYTE( "evc9.10.vc9", 0x000001, 0x020000, CRC(42f6e904) SHA1(11be8e7383a218aac0e1a63236bbdb7cca0993bf) ) /* Labeled as EVC9.10 */
	ROM_COPY( "user1", 0x00000, 0x040000, 0x040000 ) // Program roms found as 27C010 & 27C040 with 0xff filled 0x20000-0x7ffff
	ROM_COPY( "user1", 0x00000, 0x080000, 0x080000 ) // Program roms found as 27C010 & 27C040 with 0xff filled 0x20000-0x7ffff

	ROM_REGION16_LE( 0x600000, "gfx1", 0 )          /* graphics data */
	ROM_LOAD16_BYTE( "egr1.gr1",   0x000000, 0x100000, CRC(f73f80d9) SHA1(6278b45579a256b9576ba6d4f5a15fab26797c3d) )
	ROM_LOAD16_BYTE( "egr2.gr2",   0x000001, 0x100000, CRC(3a9ba910) SHA1(465aa3119af103aa65b25042b3572fdcb9c1887a) )
	ROM_LOAD16_BYTE( "egr4.gr4",   0x200000, 0x100000, CRC(4ea5900e) SHA1(20341337ee3c6c22580c52312156b818f4187693) )
	ROM_LOAD16_BYTE( "egr3.gr3",   0x200001, 0x100000, CRC(3f8dfc73) SHA1(83a168069f896ea7e67a97c6d591d09b19d5f486) )
	ROM_LOAD16_BYTE( "egr6.3.gr6", 0x400000, 0x100000, CRC(f299d818) SHA1(abbb333c43675d34c59201b5d297779cfea8b092) ) /* Labeled as EGR6.3 */
	ROM_LOAD16_BYTE( "egr5.3.gr5", 0x400001, 0x100000, CRC(ebfca07b) SHA1(20465d14b41d99651166f221057737d7b3cc770c) ) /* Labeled as EGR5.3 */

	ROM_REGION( 0x80000, "oki1", 0 )                /* sound data */
	ROM_LOAD( "eu20.u20", 0x00000, 0x80000, CRC(cca5dba0) SHA1(9c750256f0cc0ed8847db85df061be3b000b0b25) )

	ROM_REGION( 0x80000, "oki2", 0 )                /* sound data */
	ROM_LOAD( "eu21.u21", 0x00000, 0x80000, CRC(cca5dba0) SHA1(9c750256f0cc0ed8847db85df061be3b000b0b25) )

	ROM_REGION( 0x80000, "oki3", 0 )                /* sound data */
	ROM_LOAD( "eu18.u18", 0x00000, 0x80000, CRC(cca5dba0) SHA1(9c750256f0cc0ed8847db85df061be3b000b0b25) )
ROM_END


ROM_START( eggventr8 )
	ROM_REGION16_LE( 0x100000, "user1", 0 )     /* 34010 code */
	ROM_LOAD16_BYTE( "evc8.8.vc8", 0x000000, 0x020000, CRC(5a130c04) SHA1(00408912b436efa003bb02dce90fae4fe33a0180) ) /* Labeled as EVC8.8 */
	ROM_LOAD16_BYTE( "evc9.8.vc9", 0x000001, 0x020000, CRC(3ac0a95b) SHA1(7f3bd0e6d2d790af4aa6881ea8de8b296a64164a) ) /* Labeled as EVC9.8 */
	ROM_COPY( "user1", 0x00000, 0x040000, 0x040000 ) // Program roms found as 27C010 & 27C040 with 0xff filled 0x20000-0x7ffff
	ROM_COPY( "user1", 0x00000, 0x080000, 0x080000 ) // Program roms found as 27C010 & 27C040 with 0xff filled 0x20000-0x7ffff

	ROM_REGION16_LE( 0x600000, "gfx1", 0 )          /* graphics data */
	ROM_LOAD16_BYTE( "egr1.gr1",   0x000000, 0x100000, CRC(f73f80d9) SHA1(6278b45579a256b9576ba6d4f5a15fab26797c3d) )
	ROM_LOAD16_BYTE( "egr2.gr2",   0x000001, 0x100000, CRC(3a9ba910) SHA1(465aa3119af103aa65b25042b3572fdcb9c1887a) )
	ROM_LOAD16_BYTE( "egr4.gr4",   0x200000, 0x100000, CRC(4ea5900e) SHA1(20341337ee3c6c22580c52312156b818f4187693) )
	ROM_LOAD16_BYTE( "egr3.gr3",   0x200001, 0x100000, CRC(3f8dfc73) SHA1(83a168069f896ea7e67a97c6d591d09b19d5f486) )
	ROM_LOAD16_BYTE( "egr6.3.gr6", 0x400000, 0x100000, CRC(f299d818) SHA1(abbb333c43675d34c59201b5d297779cfea8b092) ) /* Labeled as EGR6.3 */
	ROM_LOAD16_BYTE( "egr5.3.gr5", 0x400001, 0x100000, CRC(ebfca07b) SHA1(20465d14b41d99651166f221057737d7b3cc770c) ) /* Labeled as EGR5.3 */

	ROM_REGION( 0x80000, "oki1", 0 )                /* sound data */
	ROM_LOAD( "eu20.u20", 0x00000, 0x80000, CRC(cca5dba0) SHA1(9c750256f0cc0ed8847db85df061be3b000b0b25) )

	ROM_REGION( 0x80000, "oki2", 0 )                /* sound data */
	ROM_LOAD( "eu21.u21", 0x00000, 0x80000, CRC(cca5dba0) SHA1(9c750256f0cc0ed8847db85df061be3b000b0b25) )

	ROM_REGION( 0x80000, "oki3", 0 )                /* sound data */
	ROM_LOAD( "eu18.u18", 0x00000, 0x80000, CRC(cca5dba0) SHA1(9c750256f0cc0ed8847db85df061be3b000b0b25) )
ROM_END


ROM_START( eggventr7 )
	ROM_REGION16_LE( 0x100000, "user1", 0 )     /* 34010 code */
	ROM_LOAD16_BYTE( "evc8.7.vc8", 0x000000, 0x020000, CRC(99999899) SHA1(e3908600fa711baa7f7562f86498ec7e988a5bea) ) /* Labeled as EVC8.7 */
	ROM_LOAD16_BYTE( "evc9.7.vc9", 0x000001, 0x020000, CRC(1b608155) SHA1(256dd981515d57f806a3770bdc6ff46b9000f7f3) ) /* Labeled as EVC9.7 */
	ROM_COPY( "user1", 0x00000, 0x040000, 0x040000 ) // Program roms found as 27C010 & 27C040 with 0xff filled 0x20000-0x7ffff
	ROM_COPY( "user1", 0x00000, 0x080000, 0x080000 ) // Program roms found as 27C010 & 27C040 with 0xff filled 0x20000-0x7ffff

	ROM_REGION16_LE( 0x600000, "gfx1", 0 )          /* graphics data */
	ROM_LOAD16_BYTE( "egr1.gr1",   0x000000, 0x100000, CRC(f73f80d9) SHA1(6278b45579a256b9576ba6d4f5a15fab26797c3d) )
	ROM_LOAD16_BYTE( "egr2.gr2",   0x000001, 0x100000, CRC(3a9ba910) SHA1(465aa3119af103aa65b25042b3572fdcb9c1887a) )
	ROM_LOAD16_BYTE( "egr4.gr4",   0x200000, 0x100000, CRC(4ea5900e) SHA1(20341337ee3c6c22580c52312156b818f4187693) )
	ROM_LOAD16_BYTE( "egr3.gr3",   0x200001, 0x100000, CRC(3f8dfc73) SHA1(83a168069f896ea7e67a97c6d591d09b19d5f486) )
	ROM_LOAD16_BYTE( "egr6.3.gr6", 0x400000, 0x100000, CRC(f299d818) SHA1(abbb333c43675d34c59201b5d297779cfea8b092) ) /* Labeled as EGR6.3 */
	ROM_LOAD16_BYTE( "egr5.3.gr5", 0x400001, 0x100000, CRC(ebfca07b) SHA1(20465d14b41d99651166f221057737d7b3cc770c) ) /* Labeled as EGR5.3 */

	ROM_REGION( 0x80000, "oki1", 0 )                /* sound data */
	ROM_LOAD( "eu20.u20", 0x00000, 0x80000, CRC(cca5dba0) SHA1(9c750256f0cc0ed8847db85df061be3b000b0b25) )

	ROM_REGION( 0x80000, "oki2", 0 )                /* sound data */
	ROM_LOAD( "eu21.u21", 0x00000, 0x80000, CRC(cca5dba0) SHA1(9c750256f0cc0ed8847db85df061be3b000b0b25) )

	ROM_REGION( 0x80000, "oki3", 0 )                /* sound data */
	ROM_LOAD( "eu18.u18", 0x00000, 0x80000, CRC(cca5dba0) SHA1(9c750256f0cc0ed8847db85df061be3b000b0b25) )
ROM_END


ROM_START( eggventr2 ) /* Comes from a PCB with an early serial number EV00123, program roms are 27C040 with required data at 0x7ffe0 in each rom */
	ROM_REGION16_LE( 0x100000, "user1", 0 )     /* 34010 code */
	ROM_LOAD16_BYTE( "ev_vc8.2.vc8", 0x000000, 0x080000, CRC(ce1da4f7) SHA1(c163041d684dc6a6fab07394e8aac3d82a2ecb52) ) /* Labeled as EV VC8.2 */
	ROM_LOAD16_BYTE( "ev_vc9.2.vc9", 0x000001, 0x080000, CRC(4b24906b) SHA1(2e9b85a658cb02d76854f3ee5a071e4161d0d0cf) ) /* Labeled as EV VC9.2 */

	ROM_REGION16_LE( 0x600000, "gfx1", 0 )          /* graphics data */
	ROM_LOAD16_BYTE( "egr1.gr1",     0x000000, 0x100000, CRC(f73f80d9) SHA1(6278b45579a256b9576ba6d4f5a15fab26797c3d) )
	ROM_LOAD16_BYTE( "egr2.gr2",     0x000001, 0x100000, CRC(3a9ba910) SHA1(465aa3119af103aa65b25042b3572fdcb9c1887a) )
	ROM_LOAD16_BYTE( "egr4.gr4",     0x200000, 0x100000, CRC(4ea5900e) SHA1(20341337ee3c6c22580c52312156b818f4187693) )
	ROM_LOAD16_BYTE( "egr3.gr3",     0x200001, 0x100000, CRC(3f8dfc73) SHA1(83a168069f896ea7e67a97c6d591d09b19d5f486) )
	ROM_LOAD16_BYTE( "ev_gr6.2.gr6", 0x400000, 0x100000, CRC(a4d9d126) SHA1(d5f2bc4bfa7c0462865907dbc39fc0479340e1c8) ) /* Labeled as EV GR6.2 */
	ROM_LOAD16_BYTE( "ev_gr5.2.gr5", 0x400001, 0x100000, CRC(b5162234) SHA1(3f05c7eb5b00805eb7fc1e3634dea29b1ce3af62) ) /* Labeled as EV GR5.2 */

	ROM_REGION( 0x80000, "oki1", 0 )                /* sound data */
	ROM_LOAD( "eu20.u20", 0x00000, 0x80000, CRC(cca5dba0) SHA1(9c750256f0cc0ed8847db85df061be3b000b0b25) )

	ROM_REGION( 0x80000, "oki2", 0 )                /* sound data */
	ROM_LOAD( "eu21.u21", 0x00000, 0x80000, CRC(cca5dba0) SHA1(9c750256f0cc0ed8847db85df061be3b000b0b25) )

	ROM_REGION( 0x80000, "oki3", 0 )                /* sound data */
	ROM_LOAD( "eu18.u18", 0x00000, 0x80000, CRC(cca5dba0) SHA1(9c750256f0cc0ed8847db85df061be3b000b0b25) )
ROM_END


ROM_START( eggventra ) /* A.L. Australia license */
	ROM_REGION16_LE( 0x100000, "user1", 0 )     /* 34010 code */
	ROM_LOAD16_BYTE( "egr8.vc8", 0x000000, 0x080000, CRC(a62c4143) SHA1(a21d6b7efdba4965285265426ed79f3249a86685) )
	ROM_LOAD16_BYTE( "egr9.vc9", 0x000001, 0x080000, CRC(bc55bc7a) SHA1(d6e3fc76b4a0a20176af1338a32bb81f0599fdc0) )

	ROM_REGION16_LE( 0x600000, "gfx1", 0 )          /* graphics data */
	ROM_LOAD16_BYTE( "egr1.gr1", 0x000000, 0x100000, CRC(f73f80d9) SHA1(6278b45579a256b9576ba6d4f5a15fab26797c3d) )
	ROM_LOAD16_BYTE( "egr2.gr2", 0x000001, 0x100000, CRC(3a9ba910) SHA1(465aa3119af103aa65b25042b3572fdcb9c1887a) )
	ROM_LOAD16_BYTE( "egr4.gr4", 0x200000, 0x100000, CRC(4ea5900e) SHA1(20341337ee3c6c22580c52312156b818f4187693) )
	ROM_LOAD16_BYTE( "egr3.gr3", 0x200001, 0x100000, CRC(3f8dfc73) SHA1(83a168069f896ea7e67a97c6d591d09b19d5f486) )
	ROM_LOAD16_BYTE( "egr6.gr6", 0x400000, 0x100000, CRC(0d73dd85) SHA1(d99a95ace89483688bae48021b416fc0a3c531d6) )
	ROM_LOAD16_BYTE( "egr5.gr5", 0x400001, 0x100000, CRC(6d89c4e3) SHA1(613703a3f194af3ed44a58610d99b7dc99382725) )

	ROM_REGION( 0x80000, "oki1", 0 )                /* sound data */
	ROM_LOAD( "eu20.u20", 0x00000, 0x80000, CRC(cca5dba0) SHA1(9c750256f0cc0ed8847db85df061be3b000b0b25) )

	ROM_REGION( 0x80000, "oki2", 0 )                /* sound data */
	ROM_LOAD( "eu21.u21", 0x00000, 0x80000, CRC(cca5dba0) SHA1(9c750256f0cc0ed8847db85df061be3b000b0b25) )

	ROM_REGION( 0x80000, "oki3", 0 )                /* sound data */
	ROM_LOAD( "eu18.u18", 0x00000, 0x80000, CRC(cca5dba0) SHA1(9c750256f0cc0ed8847db85df061be3b000b0b25) )
ROM_END


ROM_START( eggventrd )
	ROM_REGION16_LE( 0x100000, "user1", 0 )     /* 34010 code */
	ROM_LOAD16_BYTE( "eggdlx.vc8", 0x000000, 0x020000, CRC(8d678842) SHA1(92b18ec903ec8579e7dffb40284987f1d44255b8) )
	ROM_LOAD16_BYTE( "eggdlx.vc9", 0x000001, 0x020000, CRC(9db3fd23) SHA1(165a12a2d107c93cf216e755596e7457010a8f17) )
	ROM_COPY( "user1", 0x00000, 0x040000, 0x040000 ) // Program roms found as 27C010 & 27C040 with data repeated 4 times
	ROM_COPY( "user1", 0x00000, 0x080000, 0x080000 ) // Program roms found as 27C010 & 27C040 with data repeated 4 times

	ROM_REGION16_LE( 0x600000, "gfx1", 0 )          /* graphics data */
	ROM_LOAD16_BYTE( "egr1.gr1",   0x000000, 0x100000, CRC(f73f80d9) SHA1(6278b45579a256b9576ba6d4f5a15fab26797c3d) )
	ROM_LOAD16_BYTE( "egr2.gr2",   0x000001, 0x100000, CRC(3a9ba910) SHA1(465aa3119af103aa65b25042b3572fdcb9c1887a) )
	ROM_LOAD16_BYTE( "eggdlx.gr4", 0x200000, 0x100000, CRC(cfb1e28b) SHA1(8d535a27158acee893233cf2012b4ab0ffc8dc03) )
	ROM_LOAD16_BYTE( "eggdlx.gr3", 0x200001, 0x100000, CRC(a7da3891) SHA1(9139c846006bbed4bdb183659a5b40aaa0000708) )
	ROM_LOAD16_BYTE( "eggdlx.gr6", 0x400000, 0x100000, CRC(97d02e8a) SHA1(6f9532fb031953c1187782b4fce5a0cfaf9461b3) )
	ROM_LOAD16_BYTE( "eggdlx.gr5", 0x400001, 0x100000, CRC(387d9176) SHA1(9f26f97cab8baeea1d5e4860a8a35a55bdc601e8) )

	ROM_REGION( 0x80000, "oki1", 0 )                /* sound data */
	ROM_LOAD( "eu20.u20", 0x00000, 0x80000, CRC(cca5dba0) SHA1(9c750256f0cc0ed8847db85df061be3b000b0b25) )

	ROM_REGION( 0x80000, "oki2", 0 )                /* sound data */
	ROM_LOAD( "eu21.u21", 0x00000, 0x80000, CRC(cca5dba0) SHA1(9c750256f0cc0ed8847db85df061be3b000b0b25) )

	ROM_REGION( 0x80000, "oki3", 0 )                /* sound data */
	ROM_LOAD( "eu18.u18", 0x00000, 0x80000, CRC(cca5dba0) SHA1(9c750256f0cc0ed8847db85df061be3b000b0b25) )
ROM_END


ROM_START( franticf )
	ROM_REGION16_LE( 0x100000, "user1", 0 )     /* 34010 code */
	ROM_LOAD16_BYTE( "fred_vc-8.vc8", 0x000000, 0x080000, CRC(f7eb92a2) SHA1(c56a0432b8c4fe8522f6dd1e0b60eded3dfc25d2) )
	ROM_LOAD16_BYTE( "fred_vc-9.vc9", 0x000001, 0x080000, CRC(b657b800) SHA1(12649becab0019ea7150b5d797b72b07121c6a3e) )

	ROM_REGION16_LE( 0x600000, "gfx1", 0 )          /* graphics data */
	ROM_LOAD16_BYTE( "fred_gr1.gr1", 0x000000, 0x080000, CRC(acb75e63) SHA1(637ec6b7101f34a2bb93be8d0d5eaa800aafd332) )
	ROM_LOAD16_BYTE( "fred_gr2.gr2", 0x000001, 0x080000, CRC(b47c6363) SHA1(0acfd7dc45d21e6e73b5abbc544e7c0fa192c462) )
	ROM_LOAD16_BYTE( "fred_gr4.gr4", 0x200000, 0x080000, CRC(ac63729f) SHA1(dd856d983d85c38a784666105cb2d421bee8e76a) )
	ROM_LOAD16_BYTE( "fred_gr3.gr3", 0x200001, 0x080000, CRC(d7444ecc) SHA1(47b9369fec845e844ffccd121fdde12cb4842ec6) )
	ROM_LOAD16_BYTE( "fred_gr6.gr6", 0x400000, 0x080000, CRC(a0f1c918) SHA1(2004c2081a90ecc940d56f120f6e63190c8897a2) )
	ROM_LOAD16_BYTE( "fred_gr5.gr5", 0x400001, 0x080000, CRC(fcdf73a6) SHA1(081daa1dc6af59ce63b976e059533b23097cedd9) )

	ROM_REGION( 0x80000, "oki1", 0 )                /* sound data */
	ROM_LOAD( "fred_u20.u20", 0x00000, 0x80000, CRC(2fb2e5a6) SHA1(8599ec10500016c3486f9078b72cb3bda3381208) )

	ROM_REGION( 0x80000, "oki2", ROMREGION_ERASE00 )                /* sound data */
	/* Not populated */

	ROM_REGION( 0x80000, "oki3", 0 )                /* sound data */
	ROM_LOAD( "fred_u18.u18", 0x00000, 0x80000, CRC(2fb2e5a6) SHA1(8599ec10500016c3486f9078b72cb3bda3381208) )
ROM_END


ROM_START( cclownz )
	ROM_REGION16_LE( 0x100000, "user1", 0 )     /* 34010 code */
	ROM_LOAD16_BYTE( "cc-v1-vc8.bin", 0x000000, 0x080000, CRC(433fe6ac) SHA1(dea7aede9882ee52be88927418b7395418757d12) )
	ROM_LOAD16_BYTE( "cc-v1-vc9.bin", 0x000001, 0x080000, CRC(9d1b3dae) SHA1(44a97c38bc9685e97721722c67505832fa06b44d) )

	ROM_REGION16_LE( 0x600000, "gfx1", 0 )          /* graphics data */
	ROM_LOAD16_BYTE( "cc-gr1.bin",   0x000000, 0x100000, CRC(17c0ab2a) SHA1(f5ec66f4ac3292ef74f6434fe3ef17f9e977e8f6) )
	ROM_LOAD16_BYTE( "cc-gr2.bin",   0x000001, 0x100000, CRC(dead9528) SHA1(195ad9f7da61ecb5a364da92ba837aa3fcb3a347) )
	ROM_LOAD16_BYTE( "cc-gr4.bin",   0x200000, 0x100000, CRC(78cceed8) SHA1(bc8e5bb625072b17a5711402b07a39ea4a87a0f8) )
	ROM_LOAD16_BYTE( "cc-gr3.bin",   0x200001, 0x100000, CRC(af836fee) SHA1(9e32d5030d3bc5ff106242e5d4969b0150b2c516) )
	ROM_LOAD16_BYTE( "cc-gr6.bin",   0x400000, 0x100000, CRC(889d2771) SHA1(3222d7105c3a68e2050f00b07e8d84d57a9f7a19) )
	ROM_LOAD16_BYTE( "cc-gr5.bin",   0x400001, 0x100000, CRC(2a15ef8f) SHA1(3e33cff2657bb1371acf25641080aff2d8da6c05) )

	ROM_REGION( 0x80000, "oki1", 0 )                /* sound data */
	ROM_LOAD( "cc-s-u20.bin", 0x00000, 0x80000, CRC(252fc4b5) SHA1(bbc6c3599869f3f46d3df4f3f8d0a8d88d8e0132) )

	ROM_REGION( 0x80000, "oki2", 0 )                /* sound data */
	ROM_LOAD( "cc-s-u21.bin", 0x00000, 0x80000, CRC(6c3da4ed) SHA1(f10cbea6e03ada5ac1535041636e96b6224967fa) )

	ROM_REGION( 0x80000, "oki3", 0 )                /* sound data */
	ROM_LOAD( "cc-s-u18.bin", 0x00000, 0x80000, CRC(9cdf87af) SHA1(77dfc0bc1d535b5d585071dd4e9deb367003ab2d) )

	ROM_REGION( 0x80000, "user2", 0 ) /* convert these */
	ROM_LOAD( "vc-12.jed", 0x0000, 0x3f03, CRC(6947ea9e) SHA1(5a418cd04851841a49beeeea274c1441fefde173) )
	ROM_LOAD( "vc-16.jed", 0x0000, 0x3efb, CRC(e535b16a) SHA1(e2c17c2a42386be957b603d2c2da4f1ac28a4074) )
	ROM_LOAD( "vc-22.jed", 0x0000, 0x3efb, CRC(e535b16a) SHA1(e2c17c2a42386be957b603d2c2da4f1ac28a4074) )
	ROM_LOAD( "vc-23.jed", 0x0000, 0x3efa, CRC(c054cb13) SHA1(1a45548747712112e2457bd933db5ced70dae72e) )
	ROM_LOAD( "vc-24.jed", 0x0000, 0x3efa, CRC(c054cb13) SHA1(1a45548747712112e2457bd933db5ced70dae72e) )
	/* 25 / 26 are secure? */
ROM_END


ROM_START( ripribit )
	ROM_REGION16_LE( 0x100000, "user1", 0 )     /* 34010 code */
	ROM_LOAD16_BYTE( "ribbit_vc8_v3.5.vc8", 0x000000, 0x080000, CRC(8ce7f8f2) SHA1(b40b5127a0dc84a44e0283711cc526114e012c09) )
	ROM_LOAD16_BYTE( "ribbit_vc9_v3.5.vc9", 0x000001, 0x080000, CRC(70be27c3) SHA1(61328e51d083b0ffde739711675d19cfe3253244) )

	ROM_REGION16_LE( 0x600000, "gfx1", 0 )          /* graphics data */
	ROM_LOAD16_BYTE( "ribbit_gr1_rv2.81.gr1",   0x000000, 0x100000, CRC(e02c79b7) SHA1(75e352424c449cd5cba1057555928d7ee13ab113) )
	ROM_LOAD16_BYTE( "ribbit_gr2_rv2.81.gr2",   0x000001, 0x100000, CRC(09f48db7) SHA1(d0156c6e3d05ff81540c0eeb66e9a5e7fc4d053c) )
	ROM_LOAD16_BYTE( "ribbit_gr4_rv2.81.gr4",   0x200000, 0x100000, CRC(94d0db81) SHA1(aa46c2e5a627cf01c1d57002204ec3419f0d4503) )
	ROM_LOAD16_BYTE( "ribbit_gr3_rv2.81.gr3",   0x200001, 0x100000, CRC(b65e1a36) SHA1(4feb7ea0bec509fa07d27c76e5a3904b8d1690c4) )
	ROM_LOAD16_BYTE( "ribbit_gr6_rv2.81.gr6",   0x400000, 0x100000, CRC(c9ac211b) SHA1(75cbfa0f875da82d510d75ad28b9db0892b3da85) )
	ROM_LOAD16_BYTE( "ribbit_gr5_rv2.81.gr5",   0x400001, 0x100000, CRC(84ae466a) SHA1(4e7b3dc27a46f735ff13a753806b3688f34a64fe) )

	ROM_REGION( 0x80000, "oki1", 0 )  /* sound data (music) */
	ROM_LOAD( "ribbit_rr_u20.u20", 0x00000, 0x80000, CRC(c345b779) SHA1(418058bbda74727ec99ac375982c9cd2c8bc5c86) )

	ROM_REGION( 0x80000, "oki2", ROMREGION_ERASE00 )                /* sound data */
	/* Not populated */

	ROM_REGION( 0x80000, "oki3", 0 ) /* sound data (effects) */
	ROM_LOAD( "ribbit_rr_u18.u18", 0x00000, 0x80000, CRC(badb9cb6) SHA1(716d65b5ff8d3f8ff25ae70426ce318af9a92b7e) )

	ROM_REGION( 0x80000, "user2", 0 ) /* convert these */
	ROM_LOAD( "vc-12.jed", 0x0000, 0x3f03, CRC(6947ea9e) SHA1(5a418cd04851841a49beeeea274c1441fefde173) )
	ROM_LOAD( "vc-16.jed", 0x0000, 0x3efb, CRC(e535b16a) SHA1(e2c17c2a42386be957b603d2c2da4f1ac28a4074) )
	ROM_LOAD( "vc-22.jed", 0x0000, 0x3efb, CRC(e535b16a) SHA1(e2c17c2a42386be957b603d2c2da4f1ac28a4074) )
	ROM_LOAD( "vc-23.jed", 0x0000, 0x3efa, CRC(c054cb13) SHA1(1a45548747712112e2457bd933db5ced70dae72e) )
	ROM_LOAD( "vc-24.jed", 0x0000, 0x3efa, CRC(c054cb13) SHA1(1a45548747712112e2457bd933db5ced70dae72e) )
	/* 25 / 26 are secure? */
ROM_END


ROM_START( ripribita )
	ROM_REGION16_LE( 0x100000, "user1", 0 )     /* 34010 code */
	ROM_LOAD16_BYTE( "rr_v2-84-vc8.bin", 0x000000, 0x080000, CRC(5ecc432d) SHA1(073062528fbcf63be7e3c6695d60d048430f6e4b) )
	ROM_LOAD16_BYTE( "rr_v2-84-vc9.bin", 0x000001, 0x080000, CRC(d9bae3f8) SHA1(fcf8099ebe170ad5778aaa533bcfd1e5ead46e6b) )

	ROM_REGION16_LE( 0x600000, "gfx1", 0 )          /* graphics data */
	ROM_LOAD16_BYTE( "rr-gr1.bin",   0x000000, 0x100000, CRC(e02c79b7) SHA1(75e352424c449cd5cba1057555928d7ee13ab113) ) /* Same data, different labels */
	ROM_LOAD16_BYTE( "rr-gr2.bin",   0x000001, 0x100000, CRC(09f48db7) SHA1(d0156c6e3d05ff81540c0eeb66e9a5e7fc4d053c) )
	ROM_LOAD16_BYTE( "rr-gr4.bin",   0x200000, 0x100000, CRC(94d0db81) SHA1(aa46c2e5a627cf01c1d57002204ec3419f0d4503) )
	ROM_LOAD16_BYTE( "rr-gr3.bin",   0x200001, 0x100000, CRC(b65e1a36) SHA1(4feb7ea0bec509fa07d27c76e5a3904b8d1690c4) )
	ROM_LOAD16_BYTE( "rr-gr6.bin",   0x400000, 0x100000, CRC(c9ac211b) SHA1(75cbfa0f875da82d510d75ad28b9db0892b3da85) )
	ROM_LOAD16_BYTE( "rr-gr5.bin",   0x400001, 0x100000, CRC(84ae466a) SHA1(4e7b3dc27a46f735ff13a753806b3688f34a64fe) )

	ROM_REGION( 0x80000, "oki1", 0 )  /* sound data (music) */
	ROM_LOAD( "rr-s-u20.bin", 0x00000, 0x80000, CRC(c345b779) SHA1(418058bbda74727ec99ac375982c9cd2c8bc5c86) ) /* Same data, different label */

	ROM_REGION( 0x80000, "oki2", ROMREGION_ERASE00 )                /* sound data */
	/* Not populated */

	ROM_REGION( 0x80000, "oki3", 0 ) /* sound data (effects) */
	ROM_LOAD( "rr-s-u18.bin", 0x00000, 0x80000, CRC(badb9cb6) SHA1(716d65b5ff8d3f8ff25ae70426ce318af9a92b7e) ) /* Same data, different label */

	ROM_REGION( 0x80000, "user2", 0 ) /* convert these */
	ROM_LOAD( "vc-12.jed", 0x0000, 0x3f03, CRC(6947ea9e) SHA1(5a418cd04851841a49beeeea274c1441fefde173) )
	ROM_LOAD( "vc-16.jed", 0x0000, 0x3efb, CRC(e535b16a) SHA1(e2c17c2a42386be957b603d2c2da4f1ac28a4074) )
	ROM_LOAD( "vc-22.jed", 0x0000, 0x3efb, CRC(e535b16a) SHA1(e2c17c2a42386be957b603d2c2da4f1ac28a4074) )
	ROM_LOAD( "vc-23.jed", 0x0000, 0x3efa, CRC(c054cb13) SHA1(1a45548747712112e2457bd933db5ced70dae72e) )
	ROM_LOAD( "vc-24.jed", 0x0000, 0x3efa, CRC(c054cb13) SHA1(1a45548747712112e2457bd933db5ced70dae72e) )
	/* 25 / 26 are secure? */
ROM_END


ROM_START( cfarm )
	ROM_REGION16_LE( 0x100000, "user1", 0 )     /* 34010 code */
	ROM_LOAD16_BYTE( "cf-v2-vc8.bin", 0x000000, 0x080000, CRC(93bcf145) SHA1(134ac3ee4fd837f56fb0b338289cf03108346539) )
	ROM_LOAD16_BYTE( "cf-v2-vc9.bin", 0x000001, 0x080000, CRC(954421f9) SHA1(bf1faa9b085f066d1e2ff6ee01c468b1c1d945e9) )

	ROM_REGION16_LE( 0x600000, "gfx1", 0 )          /* graphics data */
	ROM_LOAD16_BYTE( "cf-gr1.bin",   0x000000, 0x100000, CRC(2241a06e) SHA1(f07a99372bb951dd345378da212b41cb8204e782) )
	ROM_LOAD16_BYTE( "cf-gr2.bin",   0x000001, 0x100000, CRC(31182263) SHA1(d5d36f9b5d612f681e6aa563831b6704bc05489e) )
	ROM_LOAD16_BYTE( "cf-gr4.bin",   0x200000, 0x100000, CRC(0883a6f2) SHA1(ef259dcdc7b1325f15a98f6c97ecb965b2b6f9b1) )
	ROM_LOAD16_BYTE( "cf-gr3.bin",   0x200001, 0x100000, CRC(572f45d6) SHA1(a48cb6ab16654d5e07e8833e2848802ddc0e2667) )
	ROM_LOAD16_BYTE( "cf-gr6.bin",   0x400000, 0x100000, CRC(8709a62c) SHA1(3691fb055155ae339c78ec8b7f485aa7d576556b) )
	ROM_LOAD16_BYTE( "cf-gr5.bin",   0x400001, 0x100000, CRC(6de18621) SHA1(9e83f8ed3a2999ee4fdca389c5e792c5b1293717) )

	ROM_REGION( 0x80000, "oki1", 0 )                /* sound data */
	ROM_LOAD( "cf-s-u20.bin", 0x00000, 0x80000, CRC(715a12dd) SHA1(374185b062853f3e2ea069ea53494cbe3d8dd511) )

	ROM_REGION( 0x80000, "oki2", 0 )                /* sound data */
	ROM_LOAD( "cf-s-u21.bin", 0x00000, 0x80000, CRC(bc27e3d5) SHA1(a25215b8314fe44974e9efe78cdc10de34f7bfba) )

	ROM_REGION( 0x80000, "oki3", 0 )                /* sound data */
	ROM_LOAD( "cf-s-u18.bin", 0x00000, 0x80000, CRC(63984658) SHA1(5594965c9304850187859ba730aff26001782f0f) )

	ROM_REGION( 0x80000, "user2", 0 ) /* convert these */
	ROM_LOAD( "vc-12.jed", 0x0000, 0x3f03, CRC(6947ea9e) SHA1(5a418cd04851841a49beeeea274c1441fefde173) )
	ROM_LOAD( "vc-16.jed", 0x0000, 0x3efb, CRC(e535b16a) SHA1(e2c17c2a42386be957b603d2c2da4f1ac28a4074) )
	ROM_LOAD( "vc-22.jed", 0x0000, 0x3efb, CRC(e535b16a) SHA1(e2c17c2a42386be957b603d2c2da4f1ac28a4074) )
	ROM_LOAD( "vc-23.jed", 0x0000, 0x3efa, CRC(c054cb13) SHA1(1a45548747712112e2457bd933db5ced70dae72e) )
	ROM_LOAD( "vc-24.jed", 0x0000, 0x3efa, CRC(c054cb13) SHA1(1a45548747712112e2457bd933db5ced70dae72e) )
	/* 25 / 26 are secure? */
ROM_END



/*************************************
 *
 *  Driver-specific initialization
 *
 *************************************/

DRIVER_INIT_MEMBER(lethalj_state,ripribit)
{
	m_maincpu->space(AS_PROGRAM).install_write_handler(0x04100010, 0x0410001f, write16_delegate(FUNC(lethalj_state::ripribit_control_w),this));
}


DRIVER_INIT_MEMBER(lethalj_state,cfarm)
{
	m_maincpu->space(AS_PROGRAM).install_write_handler(0x04100010, 0x0410001f, write16_delegate(FUNC(lethalj_state::cfarm_control_w),this));
}


DRIVER_INIT_MEMBER(lethalj_state,cclownz)
{
	m_maincpu->space(AS_PROGRAM).install_write_handler(0x04100010, 0x0410001f, write16_delegate(FUNC(lethalj_state::cclownz_control_w),this));
}



/*************************************
 *
 *  Game drivers
 *
 *************************************/

GAME( 1996, lethalj,   0,        lethalj,  lethalj,   driver_device, 0,        ROT0,  "The Game Room", "Lethal Justice (Version 2.3)", 0 )
GAME( 1996, franticf,  0,        gameroom, franticf,  driver_device, 0,        ROT0,  "The Game Room", "Frantic Fred", MACHINE_NOT_WORKING )
GAME( 1997, eggventr,  0,        gameroom, eggventr,  driver_device, 0,        ROT0,  "The Game Room", "Egg Venture (Release 10)", 0 )
GAME( 1997, eggventr8, eggventr, gameroom, eggventr,  driver_device, 0,        ROT0,  "The Game Room", "Egg Venture (Release 8)", 0 )
GAME( 1997, eggventr7, eggventr, gameroom, eggventr,  driver_device, 0,        ROT0,  "The Game Room", "Egg Venture (Release 7)", 0 )
GAME( 1997, eggventr2, eggventr, gameroom, eggventr2, driver_device, 0,        ROT0,  "The Game Room", "Egg Venture (Release 2)", 0 )
GAME( 1997, eggventra, eggventr, gameroom, eggventr,  driver_device, 0,        ROT0,  "The Game Room (A.L. Australia license)", "Egg Venture (A.L. Release)", 0 )
GAME( 1997, eggventrd, eggventr, gameroom, eggvntdx,  driver_device, 0,        ROT0,  "The Game Room", "Egg Venture Deluxe", 0 )
GAME( 1997, ripribit,  0,        gameroom, ripribit,  lethalj_state, ripribit, ROT0,  "LAI Games",     "Ripper Ribbit (Version 3.5)", 0 )
GAME( 1997, ripribita, ripribit, gameroom, ripribit,  lethalj_state, ripribit, ROT0,  "LAI Games",     "Ripper Ribbit (Version 2.8.4)", 0 )
GAME( 1999, cfarm,     0,        gameroom, cfarm,     lethalj_state, cfarm,    ROT90, "LAI Games",     "Chicken Farm (Version 2.0)", 0 )
GAME( 1999, cclownz,   0,        gameroom, cclownz,   lethalj_state, cclownz,  ROT0,  "LAI Games",     "Crazzy Clownz (Version 1.0)", 0 )
