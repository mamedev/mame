// license:BSD-3-Clause
// copyright-holders:Roberto Fresca
/********************************************************************************

  AMERICAN POKER 2
  ----------------

  Company:  Novomatic.
  Year:     1990.

  Driver by Roberto Fresca, with a lot of help of Grull Osgo.
  Based on a preliminary work of Curt Coder.

  --- Supported Sets ---

  Set Name | Relation | Description
  ---------+----------+------------------------------------
  ampoker2 |  parent  |  American Poker II.
  ampkr2b1 |  clone   |  American Poker II (bootleg, set 1).
  ampkr2b2 |  clone   |  American Poker II (bootleg, set 2).
  ampkr2b3 |  clone   |  American Poker II (bootleg, set 3).
  ampkr2b4 |  clone   |  American Poker II (bootleg, set 4).
  ampkr228 |  clone   |  American Poker II (iamp2 v28).
  pkrdewin |  clone   |  Poker De Win.
  ampkr95  |  clone   |  American Poker 95.
  videomat |  clone   |  Videomat (polish bootleg).
  rabbitpk |  clone   |  Rabbit Poker / Arizona Poker 1.1? (with PIC)
  sigmapkr |  parent  |  Sigma Poker.
  sigma2k  |  parent  |  Sigma Poker 2000.
  piccolop |  parent  |  Piccolo Poker 100.

*********************************************************************************

  Game Notes...
  -------------

  American Poker II uses a standard deck of 52 cards plus 1 Joker card.
  The Joker card substitutes for any card. The player places a bet for
  the 1st hand deal and can then choose to buy a 2nd draw.

  The win combination Jacks or Better is only paid if a 2nd draw is bought.


  Operation...

  To 'init' (boot) the machine:
  1) Turn ON the Operator Key (9).
  2) Keep pressed the DOOR key (W). You are entering the Operator Mode.
  3) Turn OFF the Operator Key (9).
  4) Reset the machine. (you must reset manually the machine due to watchdog issues).

  The set ampkr2b3 has a special init code/key:

  1) Turn ON the Supervisor Key (0).
  2) Enter the following sequence: HOLD1, HOLD5, HOLD4, HOLD3, HOLD2 and HOLD3.
  3) Wait till the Supervisor Mode appear, and turn OFF the Supervisor Key (0).
  4) Reset the machine. (you must reset manually the machine due to watchdog issues).

  ...The key is produced taking 6 bytes at offset 0x5d40 XORed with their respective pairs
  at offset 0x5d50, resulting the sequence of HOLD keys to init the machine.


  Operator Mode:
  Press '9' once to turn ON the Operator Key. You can find the interim meters (Page 1).
  Press HOLD3 to clear the meters. Press '9' again to turn OFF the Operator Key.
  Reset the machine to enter the Game Mode.

  Supervisor Mode:
  Press '0' once to turn ON the Supervisor Key. You can find the permanent meters (Page 1 to 5).
  HOLD1, HOLD3 and HOLD5 are the valid keys to navigate through the mode. Press '0' again to turn
  OFF the Operator Key.
  Reset the machine to enter the Game Mode.

  To access both modes, no credits should be placed in the machine.


  From Novomatic web site:

  "1990 - American Poker II kommt auf den Markt und wird als
  'The Legend' in die Geschichte des Gl?cksspiels eingehen."

  "1990 - American Poker II comes on the market and is known as
  'The Legend' in the history of gaming."

    ----

  Sigma Poker:
  This poker game was also sold as "upgrade kit" for American Poker II taiwanese boards.
  The game has a lot of improvements. New graphics, sounds, bonus, and a totally new
  'double-up' feature. Very addictive, in fact.

  Sigma only released 2 games for this hardware: Sigma Poker and Sigma Poker 2000.
  Both games need some modifications to the original board to be installed.
  The rest of Sigma poker games (2001 onwards) were developed for B-52 mainboards.
  (2x 6809; HD63484 video controller).


  Sigma Poker 2000:

  This game has better graphics and use 4 times more tiles than other games running
  on this hardware. To manage this, the game use 2 extra bits from the color RAM.

  To init the game:

  1) Turn ON the Supervisor Key (0).
  2) Press HOLD5 3 times to enter into page 4 (setup) of the supervisor menu.
  3) Press HOLD3 to navegate between options, and highlight "Clear All Informations"
  4) Keep pressed HOLD1 for more than 3 seconds.
  5) Turn OFF the Supervisor Key (0).


  Piccolo Poker 100:

  To 'init' (boot) the machine:
  1) Turn ON the Operator Key (9).
  2) Press the DOOR key (O). You are entering the Operator Mode.
  3) Turn OFF the Operator Key (9).
  4) Reset the machine. (sometimes you must reset manually the machine due to watchdog issues).

  If you win some credits, you'll be on troubles due to unemulated hopper.
  Just discharge the credits one by one pressing quickly the door switch (O)
  If you want to play without the hopper issues, just leave the door open (O). In this mode,
  the game is playable but doesn't contabilize in the meters.


*********************************************************************************

  *** Technical Notes ***


  DIP Switches
  ------------

  DIP1       Remote Credits
  OFF        x100
  ON         x50

  DIP2       Auto Hold
  OFF        Off
  ON         On

  DIP3 DIP4  Rate Tables (*)
  OFF  OFF   RATE: 1 2 3 4 5 10 20 30 40 50
  OFF  ON    RATE: 5 10 15 20 25 30 35 40 50 100
  ON   OFF   RATE: 5 10 15 20 25 30 35 40 45 50
  ON   ON    RATE: 10 20 30 40 50 60 70 80 90 100

  DIP6       Take Winnings (On Double Up)
  OFF        Take Part (10 Credits steps)
  ON         Take All

  DIP8       Jackpot
  OFF        Off
  ON         On

  (*) Working only in ampkr95.


  Hardware Notes:
  --------------

  - CPU:            1x Z80 @ 3 MHz.
  - Video:          TTL Logic Raster - 6 MHz Dot Clock.
  - Osc:            6.000 MHz Xtal.
  - RAM:            1x 6116 (4Kx8) Static RAM.
  - VRAM            2x 2016 (4Kx8) Static RAM.
  - I/O:            8x 74LS251; 8x 74LS259 (Multiplex 8 Ports > 1 Bit).
  - PRG ROMs:       1x 27C512 (64Kx8) EPROM or similar.
  - GFX ROMs:       1x 27C128 (16Kx8) EPROM or similar.
  - Color PROM:     1x 82S147AN.
  - Sound:          1x AY-3-8910.
  - Backup Battery: 1x NI-CD 3.6 Volt.
  - DIP Switches:   1x 8 switches.
  - Watchdog:       1x TL7705 (Texas Instruments). Refresh: 200 Ms.


  Taiwanese PCB Layout:
   ________________________________________________________________________________
  |                                                                                |
  |  ______   _________    _________    ______________    _________   __________   |
  | | XTAL | | 74LS04  |  | 74LS138 |  |              |  | 74LS163 | |PALCE16V8H|  |
  | | 6MHz | |_________|  |_________|  |  UM6116-2    |  |_________| |__________|  |
  | |______|  _________    _________   |______________|   _________   ________  __ |
  |          | 74LS74A |  | DM7407N |  _______________   | 74LS163 | | 74LS00 ||74||
  |          |_________|  |_________| |               |  |_________| |________||LS||
  | ____________________   _________  |    27C512     |   _________   ________ |08||
  ||                    | | MC14020 | |_______________|  | 74LS163 | | 74LS157||__||
  || NI-CD 3.6V BATTERY | |_________|                    |_________| |________|    |
  ||____________________|  __________    _____________    _________   ________     |
  |____                   | 74LS244  |  |   74LS245   |  | 74LS163 | | 74LS157|    |
       |                  |__________|  |_____________|  |_________| |________|    |
   ____|                                                                           |
  |_28_                         ______________________    _________   ________     |
  |____                        |       - Z80A -       |  | 74LS74  | | 74LS157|    |
  |____                        |     TMPZ84C00AP-6    |  |_________| |________|    |
  |____                        |______________________|   __________  __________   |
  |____                     ___________    ____________  |PALCE16V8H||D4016CX-20|  |
  |____                    | 74LS244N  |  |  74LS259N  | |__________||__________|  |
  |____                    |___________|  |____________|  ________    __________   |
  |____                     ___________     __________   | 74LS02 |  |D4016CX-20|  |
  |____                    | TD62003AP |   | 74LS251P |  |________|  |__________|  |
  |____                    |___________|   |__________|   ____________________     |
  |____                     ___________     __________   |AY-3-8910 / YM2149F |    |
  |____                    | 74LS259N  |   | 74LS251P |  |    or KC89C72      |    |
  |____                    |___________|   |__________|  |____________________|    |
  |____                     ___________     __________    __________   _________   |
  |____                    | 74LS259N  |   | 74LS251P |  | 82S147AN | | 74LS245 |  |
  |____                    |___________|   |__________|  |__________| |_________|  |
  |____                     ___________     __________    _________                |
  |____                    | 74LS259N  |   | 74LS251P |  | 74LS377 |               |
  |____                    |___________|   |__________|  |_________|               |
  |____                     ___________     __________    _________                |
  |____        ________    | TD62003AP |   | 74LS251P |  | 74LS377 |               |
  |____       |12345678|   |___________|   |__________|  |_________|               |
  |____       |________|                    __________    _________   _________    |
  |____          DSW1                      | 74LS194  |  | 74LS194 | | 74LS245 |   |
  |____                                    |__________|  |_________| |_________|   |
  |____                                                                            |
  |____         ______        ________________         _____________   _________   |
  |_01_        |TL7705|      |01 oooooooooo 10|       |   27C128    | | 74LS377 |  |
       |       |______|      |________________|       |   D27128D   | |_________|  |
   ____|                         CONNECTOR1           |_____________|              |
  |                                                                                |
  |________________________________________________________________________________|


  The main clock (6 MHz.) is generated with a crystal and 74LS04 inverters. This frequency is
  used as the pixel clock, then is divided by 2 in a flip-flop (7474) and again to the video
  stage like DOT/2 (the video hardware uses a 3 MHz clock synchronous to the pixel clock).

  Once again this 3 MHz clock signal is further divided by 2 through a flip-flop (7474) to get
  the 1.5 MHz for the AY8910.

  The 4020 is clocked at 1.5 MHz. The Q10 output (pin 14) is approximately 1464.84 Hz. Using
  an oscilloscope, I measured a value of 60 uS = 1538 Hz. We used a NMI period of 1536 Hz due
  to a better binary composition (1024+512).

  Inputs/Ouputs are driven through 74LS251 and 74LS259 multiplexers. Each one handles 1 bit
  from data bus, and there are many devices as addressed ports (8x 74LS251 and 8x 74LS259).

  Input ports are mapped to offsets 0xC410 through 0xC417. Output ports are mapped to 0xC4000
  to 0xC407 and are polled/updated during NMI.

  These 1-bit controls are relative to buttons, keys, lights and counters. Other output ports
  like watchdog or PSG (AY8910) are operated directly.


  Resistor Network
  ----------------

  The following diagram is related to taiwanese and argentine PCBs.

   82S147AN
  +---------+
  |         |    470
  | O1-Pin06|---/\/\/\----+---> BLUE
  |         |    220      |
  | O2-Pin07|---/\/\/\----+
  |         |    1K
  | O3-Pin08|---/\/\/\----+---> GREEN
  |         |    470      |
  | O4-Pin09|---/\/\/\----+
  |         |    220      |
  | O5-Pin11|---/\/\/\----+
  |         |    1K
  | O6-Pin12|---/\/\/\----+---> RED
  |         |    470      |
  | O7-Pin13|---/\/\/\----+
  |         |    220      |
  | O8-Pin14|---/\/\/\----+
  |         |
  +---------+

  All colors are directly routed to the edge connector.
  There are not pull-up or pull-down resistors.


*********************************************************************************

  --- DRIVER UPDATES ---


  [2010-09-28]

  Piccolo Poker 100 from Admiral - Novomatic.
  - Added a workaround to get the game booting.
  - Created inputs from the scratch.
  - Promoted to 'working'.
  - Added technical and game notes.


  [2009-08-17]

  - Added Rabbit Poker / Arizona Poker? set (with GAL22V10 and PIC16F84A).
  - Added proper decryption algorithms.
  - Updated technical notes.


  [2008-10-07]
  - Improved the button-lamps layout to all games. Now are more realistic.


  [2008-06-09]
  - Added Videomat (polish bootleg).


  [2008-06-02]

  - Reworked the input system for Sigma Poker 2000.
  - Promoted Sigma Poker 2000 to 'WORKING' state.
  - Updated technical notes.


  [2008-05-23]

  - Reworked the color routines switching to resnet system.
  - Added a resistor network diagram.
  - Switch to pre-defined crystal value.
  - Changed the WATCHDOG_TIME_INIT to be based on miliseconds instead of hertz.
  - Other minor cleanup/fixes.
  - Updated technical notes.


  [2007-11-15]

  ******** REWRITE ********

  - Crystal documented via #define.
  - CPU and sound clocks derived from #defined crystal value.
  - Reworked TILE_GET_INFO to handle the proper tiles/color codes.
  - Added the correct GFX dump to sigma2k.
  - Added proper TILE_GET_INFO, VIDEO_START, GFX_LAYOUT, GFXDECODE and MACHINE to sigma2k.
  - Fixed interrupts (NMI).
  - Corrected AY8910 frequency to 1.5 MHz to match the real thing.
  - Arranged the AY8910 volume in all games avoiding clips.
  - Corrected the screen visible area.
  - Added NVRAM support.
  - Reworked the memory map, mapping all the hardware I/O ports.
  - Reworked the Inputs for all sets.
  - Added implementation of Operator and Supervisor Keys.
  - Fixed some timing troubles.
  - Mapped the input buttons in the same way I mapped them in other poker games.
  - Added partial DIP switch support with diplocations to all sets.

  - Removed the hack in DRIVER_INIT.
  - Hooked write handlers for output ports.
  - Added watchdog routines.
  - Dumped, hooked, wired and decoded the color PROM in all sets. Colors are perfect.
  - Modified the refresh rate to 60 fps according to hardware measurements.
  - Cleaned up and renamed all sets, defining parent-clone relationship.
  - Wired the lamps for all sets. Created their respective layouts.
  - Updated flags in game drivers.
  - Splitted the driver to driver/video.
  - Other minor fixes.

  - New set dumped/added: American Poker 95.
  - New set dumped/added: American Poker 2 (bootleg, set 1).
  - New set dumped/added: Sigma Poker.
  - New set dumped/added: Sigma Poker 2000.

  - Rewritten the technical notes from the scratch (still in progress).
  - Added a PCB layout to the technical notes.



  *** TODO ***

  - Find why the watchdog sometimes stop to work.
  - Analyze the write to port 0x21 after reset.
  - Proper lamps for Piccolo Poker.
  - Hopper emulation.


*********************************************************************************/


#define MASTER_CLOCK    XTAL_6MHz

#include "emu.h"
#include "cpu/z80/z80.h"
#include "sound/ay8910.h"
#include "machine/nvram.h"
#include "ampoker2.lh"
#include "sigmapkr.lh"
#include "includes/ampoker2.h"

/**********************
* Read/Write Handlers *
*  - Output Ports -   *
***********************


  There are only five bits wired for each Port

  PORT   ADDRESS    BIT0      BIT1      BIT2      BIT3      BIT4
  -------------------------------------------------------------------

  0x30 - 0xc000     ----      ----      ----      ----      ----
  0x31 - 0xc001     ----    L.Bet/Red  L.Hold4   L.Hold2   Twr.Yell
  0x32 - 0xc002     ----      ----      ----     L.Hold3    ----
  0x33 - 0xc003     ----      ----      ----      ----      ----
  0x34 - 0xc004     ----      ----      ----      ----     L.Black
  0x35 - 0xc005     ----      ----      ----      ----      ----
  0x36 - 0xc006   Twr.Grn.   L.Hold5   L.Deal    L.Hold1    ----
  0x37 - 0xc007     ----      ----      ----     WatchDog   ----


  --- Lamps wiring (done) ---

    L0 = DEAL / TAKE
    L1 = RED / BET
    L2 = BLACK
    L3 = HOLD 1
    L4 = HOLD 2
    L5 = HOLD 3
    L6 = HOLD 4
    L7 = HOLD 5
    L8 = TOWER YELLOW (not in lay)
    L9 = TOWER GREEN  (not in lay)


  --- Counters wiring ---

    C0 = METER 1 - REMOTE IN
    C1 = METER 2 - TOTAL OUT
    C2 = METER 3 - TOTAL IN
    C3 = METER 4 - BILLS
    C4 = METER 5 - JACKPOT
    C5 = METER 6 - CASH BOX
    C6 = METER 7 - GAMES
    C7 = METER 8 - ?


  --- Devices wiring ---

    D0 = Enable Coin to Hopper / Diverter
    D1 = Enable Coin 1 / Acceptor
    D2 = Enable Coin 2
    D3 = Enable Coin 3
    D4 = Enable Coin 4 / Bill Reader
    D5 = Hopper 1 Enable
    D6 = Hopper 2 Enable (Optional)
    D7 = Hopper 1 2 Enable


  --- Unused wiring ---

    U0 = Pin 15A (ULN2064)
    U1 = Pin 17C (ULN2064)
    U2 = Pin 19A (ULN2064)
    U3 = Pin  6C (ULN2064)
    U4 = Pin 13C
    U5 = Pin 14A

*/

WRITE8_MEMBER(ampoker2_state::ampoker2_port30_w)
/*-------------------------------------------------
    PORT_30 C000H         ;OUTPUT PORT 30H
---------------------------------------------------
    BIT 0 =
    BIT 1 =
    BIT 2 =
    BIT 3 =
    BIT 4 =
--------------------------------------------------*/
{
}


WRITE8_MEMBER(ampoker2_state::ampoker2_port31_w)
/*-------------------------------------------------
    PORT_31 C001H         ;OUTPUT PORT 31H
---------------------------------------------------
    BIT 0 =
    BIT 1 = LAMP_1        ;Lamp 1  (RED)
    BIT 2 = LAMP_6        ;Lamp 6  (HOLD4)
    BIT 3 = LAMP_4        ;Lamp 4  (HOLD2)
    BIT 4 = TWL_YELL      ;Tower Light YELLOW
--------------------------------------------------*/
{
	output().set_lamp_value(1, ((data >> 1) & 1));    /* Lamp 1 - BET/RED */
	output().set_lamp_value(6, ((data >> 2) & 1));    /* Lamp 6 - HOLD 4 */
	output().set_lamp_value(4, ((data >> 3) & 1));    /* Lamp 4 - HOLD 2 */
	output().set_lamp_value(8, ((data >> 4) & 1));    /* Lamp 8 - TWR.YELLOW */
}


WRITE8_MEMBER(ampoker2_state::ampoker2_port32_w)
/*-------------------------------------------------
    PORT_32 C002H         ;OUTPUT PORT 32H
---------------------------------------------------
    BIT 0 =
    BIT 1 =
    BIT 2 =
    BIT 3 = LAMP_5        ;Lamp 5  (HOLD3)
    BIT 4 =
--------------------------------------------------*/
{
	output().set_lamp_value(5, ((data >> 3) & 1));    /* Lamp 5 - HOLD 3 */
}


WRITE8_MEMBER(ampoker2_state::ampoker2_port33_w)
/*-------------------------------------------------
    PORT_33 C003H         ;OUTPUT PORT 33H
---------------------------------------------------
    BIT 0 =
    BIT 1 =
    BIT 2 =
    BIT 3 =
    BIT 4 =
--------------------------------------------------*/
{
}


WRITE8_MEMBER(ampoker2_state::ampoker2_port34_w)
/*-------------------------------------------------
    PORT_34 C004H         ;OUTPUT PORT 34H
---------------------------------------------------
    BIT 0 =
    BIT 1 =
    BIT 2 =
    BIT 3 =
    BIT 4 = LAMP_2        ;Lamp 3  (BLACK)
--------------------------------------------------*/
{
	output().set_lamp_value(2, ((data >> 4) & 1));    /* Lamp 2 - BLACK */
}


WRITE8_MEMBER(ampoker2_state::ampoker2_port35_w)
/*-------------------------------------------------
    PORT_35 C005H         ;OUTPUT PORT 35H
---------------------------------------------------
    BIT 0 =
    BIT 1 =
    BIT 2 =
    BIT 3 =
    BIT 4 =
--------------------------------------------------*/
{
}


WRITE8_MEMBER(ampoker2_state::ampoker2_port36_w)
/*-------------------------------------------------
    PORT_36 C006H         ;OUTPUT PORT 36H
---------------------------------------------------
    BIT 0 = TWL_GREEN     ;Tower Light GREEN
    BIT 1 =
    BIT 2 = LAMP_9        ;Lamp 9  (HOLD5)
    BIT 3 = LAMP_0        ;Lamp 0  (DEAL)
    BIT 4 = LAMP_3        ;Lamp 3  (HOLD1)
--------------------------------------------------*/
{
	output().set_lamp_value(9, (data & 1));           /* Lamp 9 - TWR.GREEN */
	output().set_lamp_value(7, ((data >> 2) & 1));    /* Lamp 7 - HOLD 5 */
	output().set_lamp_value(0, ((data >> 3) & 1));    /* Lamp 0 - DEAL */
	output().set_lamp_value(3, ((data >> 4) & 1));    /* Lamp 3 - HOLD 1 */
}


WRITE8_MEMBER(ampoker2_state::ampoker2_watchdog_reset_w)
/*-------------------------------------------------
    PORT_37 C007H         ;OUTPUT PORT 37H
---------------------------------------------------
    BIT 3 = W_DOG         ;WATCHDOG.
--------------------------------------------------*/
{
	/* watchdog sometimes stop to work */

	if (((data >> 3) & 0x01) == 0)      /* check for refresh value (0x08) */
	{
		machine().watchdog_reset();
//      popmessage("%02x", data);
	}
	else
	{
//      popmessage("%02x", data);
	}
}


/*************************
* Memory map information *
*************************/

static ADDRESS_MAP_START( ampoker2_map, AS_PROGRAM, 8, ampoker2_state )
	AM_RANGE(0x0000, 0xbfff) AM_ROM
	AM_RANGE(0xc000, 0xcfff) AM_RAM AM_SHARE("nvram")
	AM_RANGE(0xe000, 0xefff) AM_RAM_WRITE(ampoker2_videoram_w) AM_SHARE("videoram")
ADDRESS_MAP_END

static ADDRESS_MAP_START( ampoker2_io_map, AS_IO, 8, ampoker2_state )
	ADDRESS_MAP_GLOBAL_MASK(0xff)
	AM_RANGE(0x08, 0x0f) AM_WRITENOP                /* inexistent in the real hardware */
	AM_RANGE(0x10, 0x10) AM_READ_PORT("IN0")
	AM_RANGE(0x11, 0x11) AM_READ_PORT("IN1")
	AM_RANGE(0x12, 0x12) AM_READ_PORT("IN2")
	AM_RANGE(0x13, 0x13) AM_READ_PORT("IN3")
	AM_RANGE(0x14, 0x14) AM_READ_PORT("IN4")
	AM_RANGE(0x15, 0x15) AM_READ_PORT("IN5")
	AM_RANGE(0x16, 0x16) AM_READ_PORT("IN6")
	AM_RANGE(0x17, 0x17) AM_READ_PORT("IN7")
//  AM_RANGE(0x21, 0x21) AM_WRITENOP                    /* undocumented, write 0x1a after each reset */
	AM_RANGE(0x30, 0x30) AM_WRITE(ampoker2_port30_w)    /* see write handlers */
	AM_RANGE(0x31, 0x31) AM_WRITE(ampoker2_port31_w)    /* see write handlers */
	AM_RANGE(0x32, 0x32) AM_WRITE(ampoker2_port32_w)    /* see write handlers */
	AM_RANGE(0x33, 0x33) AM_WRITE(ampoker2_port33_w)    /* see write handlers */
	AM_RANGE(0x34, 0x34) AM_WRITE(ampoker2_port34_w)    /* see write handlers */
	AM_RANGE(0x35, 0x35) AM_WRITE(ampoker2_port35_w)    /* see write handlers */
	AM_RANGE(0x36, 0x36) AM_WRITE(ampoker2_port36_w)    /* see write handlers */
	AM_RANGE(0x37, 0x37) AM_WRITE(ampoker2_watchdog_reset_w)
	AM_RANGE(0x38, 0x39) AM_DEVWRITE("aysnd", ay8910_device, address_data_w)
	AM_RANGE(0x3A, 0x3A) AM_DEVREAD("aysnd", ay8910_device, data_r)
ADDRESS_MAP_END

/*

  Rabbit Poker writes...

  12dc W \ Writing to the PIC?... The program doesn't seems to poll it.
  12dd W /

  Something is going wrong here. 12xx is ROM space. Put a BP on 12e5 and
  you can see the NVRAM checking routine (NVRAM = c000-cfff)

'maincpu' (000012E5): unmapped program memory byte write to 000012DC = E5
'maincpu' (000012E5): unmapped program memory byte write to 000012DD = 12
'maincpu' (000012F0): unmapped program memory byte write to 000012DC = F0
'maincpu' (000012F0): unmapped program memory byte write to 000012DD = 12
'maincpu' (000012F0): unmapped program memory byte write to 000012DC = F0
'maincpu' (000012F0): unmapped program memory byte write to 000012DD = 12
'maincpu' (000012F0): unmapped program memory byte write to 000012DC = F0
'maincpu' (000012F0): unmapped program memory byte write to 000012DD = 12
'maincpu' (000012F0): unmapped program memory byte write to 000012DC = F0
'maincpu' (000012F0): unmapped program memory byte write to 000012DD = 12
'maincpu' (000012F0): unmapped program memory byte write to 000012DC = F0
'maincpu' (000012F0): unmapped program memory byte write to 000012DD = 12
'maincpu' (000012F0): unmapped program memory byte write to 000012DC = F0
'maincpu' (000012F0): unmapped program memory byte write to 000012DD = 12
'maincpu' (000012F2): unmapped program memory byte write to 000012DC = F2
'maincpu' (000012F2): unmapped program memory byte write to 000012DD = 12

*/

/*************************
*      Input ports       *
*************************/

static INPUT_PORTS_START( ampoker2 )
	PORT_START("IN0")
	PORT_DIPNAME( 0x01, 0x01, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("Hopper 1") PORT_CODE(KEYCODE_Y)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_COIN2 ) PORT_IMPULSE(2)
	PORT_DIPNAME( 0x08, 0x08, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x08, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_COIN3 ) PORT_IMPULSE(2)

	PORT_START("IN1")
	PORT_DIPNAME( 0x01, 0x01, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_POKER_HOLD1 )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_GAMBLE_SERVICE ) PORT_NAME("Operator Key") PORT_TOGGLE
	PORT_DIPNAME( 0x08, 0x08, "Remote Mode" )
	PORT_DIPSETTING(    0x08, "Mode 1" )
	PORT_DIPSETTING(    0x00, "Mode 2" )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_POKER_HOLD4 )

	PORT_START("IN2")
	PORT_DIPNAME( 0x01, 0x01, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("RTS") PORT_CODE(KEYCODE_U)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_GAMBLE_DOOR ) PORT_NAME("Door Switch")
	PORT_DIPNAME( 0x08, 0x08, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x08, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_GAMBLE_PAYOUT )

	PORT_START("IN3")
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("Hopper Out") PORT_CODE(KEYCODE_G)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("Supervisor Key") PORT_TOGGLE PORT_CODE(KEYCODE_0)
	PORT_DIPNAME( 0x08, 0x08, "Remote Credits" ) PORT_DIPLOCATION("SW1:1") /* DSW1 */
	PORT_DIPSETTING(    0x08, "Cred x 100" ) PORT_CONDITION("IN1", 0x08, EQUALS,0x08)
	PORT_DIPSETTING(    0x00, "Cred x  50" ) PORT_CONDITION("IN1", 0x08, EQUALS,0x08)
	PORT_DIPSETTING(    0x08, "Cred x  20" ) PORT_CONDITION("IN1", 0x08, EQUALS,0x00) /* x100 in ampkr95 */
	PORT_DIPSETTING(    0x00, "Remote Off" ) PORT_CONDITION("IN1", 0x08, EQUALS,0x00)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_GAMBLE_LOW ) PORT_NAME("Black Card")

	PORT_START("IN4")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNKNOWN ) /* not used */
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("Hopper Low") PORT_CODE(KEYCODE_H)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_POKER_HOLD2 )
	PORT_DIPNAME( 0x08, 0x08, "Auto Hold" )      PORT_DIPLOCATION("SW1:2") /* DSW2 */
	PORT_DIPSETTING(    0x08, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_GAMBLE_DEAL ) PORT_NAME("Deal / Take")

	PORT_START("IN5")
	PORT_DIPNAME( 0x01, 0x01, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("Return Line") PORT_CODE(KEYCODE_J)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("TILT") PORT_CODE(KEYCODE_K)
	PORT_DIPNAME( 0x08, 0x08, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x08, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_POKER_HOLD3 )

	PORT_START("IN6")
	PORT_DIPNAME( 0x01, 0x01, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_GAMBLE_HIGH ) PORT_NAME("Red Card / Bet")
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_POKER_HOLD5 )
	PORT_DIPNAME( 0x08, 0x08, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x08, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_COIN1 ) PORT_IMPULSE(2)

	PORT_START("IN7")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_UNKNOWN ) /* not used */
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("Remote Credit") PORT_IMPULSE(12) PORT_CODE(KEYCODE_3)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_COIN4 ) PORT_IMPULSE(2)
	PORT_DIPNAME( 0x08, 0x08, "Jackpot" )        PORT_DIPLOCATION("SW1:8") /* DSW8 */
	PORT_DIPSETTING(    0x08, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("Clear Credits") PORT_CODE(KEYCODE_4)
INPUT_PORTS_END

static INPUT_PORTS_START( ampkr95 )
	PORT_START("IN0")
	PORT_DIPNAME( 0x01, 0x01, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("Hopper 1") PORT_CODE(KEYCODE_Y)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_COIN2 ) PORT_IMPULSE(2)
	PORT_DIPNAME( 0x08, 0x08, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x08, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_COIN3 ) PORT_IMPULSE(2)

	PORT_START("IN1")
	PORT_DIPNAME( 0x01, 0x01, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_POKER_HOLD1 )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_SERVICE ) PORT_NAME("Operator Key") PORT_TOGGLE
	PORT_DIPNAME( 0x08, 0x08, "Remote Mode" )
	PORT_DIPSETTING(    0x08, "Mode 1" )
	PORT_DIPSETTING(    0x00, "Mode 2" )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_POKER_HOLD4 )

	PORT_START("IN2")
	PORT_DIPNAME( 0x01, 0x01, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("RTS") PORT_CODE(KEYCODE_U)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_GAMBLE_DOOR ) PORT_NAME("Door Switch")
	PORT_DIPNAME( 0x08, 0x08, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x08, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON9 ) PORT_NAME("Payout") PORT_CODE(KEYCODE_Q)

	PORT_START("IN3")
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("Hopper Out") PORT_CODE(KEYCODE_G)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("Supervisor Key") PORT_TOGGLE PORT_CODE(KEYCODE_0)
	PORT_DIPNAME( 0x08, 0x08, "Remote Credits" ) PORT_DIPLOCATION("SW1:1") /* DSW1 */
	PORT_DIPSETTING(    0x08, "Cred x 100" ) PORT_CONDITION("IN1",0x08,EQUALS,0x08)
	PORT_DIPSETTING(    0x00, "Cred x  50" ) PORT_CONDITION("IN1",0x08,EQUALS,0x08)
	PORT_DIPSETTING(    0x08, "Cred x 100" ) PORT_CONDITION("IN1",0x08,EQUALS,0x00) /* x100 in ampkr95 */
	PORT_DIPSETTING(    0x00, "Remote Off" ) PORT_CONDITION("IN1",0x08,EQUALS,0x00)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_GAMBLE_LOW ) PORT_NAME("Black Card")

	PORT_START("IN4")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNKNOWN ) /* not used */
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("Hopper Low") PORT_CODE(KEYCODE_H)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_POKER_HOLD2 )
	PORT_DIPNAME( 0x08, 0x08, "Auto Hold" )      PORT_DIPLOCATION("SW1:2") /* DSW2 */
	PORT_DIPSETTING(    0x08, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_GAMBLE_DEAL ) PORT_NAME("Deal / Take")

	PORT_START("IN5")
	PORT_DIPNAME( 0x01, 0x01, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("Return Line") PORT_CODE(KEYCODE_J)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("TILT") PORT_CODE(KEYCODE_K)
	PORT_DIPNAME( 0x08, 0x08, "Rate Table SW1" ) PORT_DIPLOCATION("SW1:3") /* DSW3 (should be arranged with DSW4) */
	PORT_DIPSETTING(    0x08, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_POKER_HOLD3 )

	PORT_START("IN6")
	PORT_DIPNAME( 0x01, 0x01, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_GAMBLE_HIGH ) PORT_NAME("Bet / Red")
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_POKER_HOLD5 )
	PORT_DIPNAME( 0x08, 0x08, "Rate Table SW2" ) PORT_DIPLOCATION("SW1:4") /* DSW4 (should be arranged with DSW3) */
	PORT_DIPSETTING(    0x08, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_COIN1 ) PORT_IMPULSE(2)

	PORT_START("IN7")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_UNKNOWN ) /* not used */
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("Remote Credit") PORT_IMPULSE(12) PORT_CODE(KEYCODE_3)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_COIN4 ) PORT_IMPULSE(2)
	PORT_DIPNAME( 0x08, 0x08, "Jackpot" )        PORT_DIPLOCATION("SW1:8") /* DSW8 */
	PORT_DIPSETTING(    0x08, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("Clear Credits") PORT_CODE(KEYCODE_4)
INPUT_PORTS_END

static INPUT_PORTS_START( sigmapkr )
	PORT_START("IN0")
	PORT_DIPNAME( 0x01, 0x01, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("Hopper 1") PORT_CODE(KEYCODE_Y)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_COIN2 ) PORT_IMPULSE(2)
	PORT_DIPNAME( 0x08, 0x08, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x08, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_COIN3 ) PORT_IMPULSE(2)

	PORT_START("IN1")
	PORT_DIPNAME( 0x01, 0x01, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_POKER_HOLD1 )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_SERVICE ) PORT_NAME("Operator Key") PORT_TOGGLE
	PORT_DIPNAME( 0x08, 0x08, "Remote Mode" )
	PORT_DIPSETTING(    0x08, "Mode 1" )
	PORT_DIPSETTING(    0x00, "Mode 2" )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_POKER_HOLD4 )

	PORT_START("IN2")
	PORT_DIPNAME( 0x01, 0x01, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("RTS") PORT_CODE(KEYCODE_U)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_GAMBLE_DOOR ) PORT_NAME("Door Switch")
	PORT_DIPNAME( 0x08, 0x08, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x08, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON9 ) PORT_NAME("Payout") PORT_CODE(KEYCODE_Q)

	PORT_START("IN3")
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("Hopper Out") PORT_CODE(KEYCODE_G)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("Supervisor Key") PORT_TOGGLE PORT_CODE(KEYCODE_0)
	PORT_DIPNAME( 0x08, 0x08, "Remote Credits" ) PORT_DIPLOCATION("SW1:1") /* DSW1 */
	PORT_DIPSETTING(    0x08, "Cred x 100" ) PORT_CONDITION("IN1",0x08,EQUALS,0x08)
	PORT_DIPSETTING(    0x00, "Cred x  50" ) PORT_CONDITION("IN1",0x08,EQUALS,0x08)
	PORT_DIPSETTING(    0x08, "Cred x 100" ) PORT_CONDITION("IN1",0x08,EQUALS,0x00) /* x100 in ampkr95 */
	PORT_DIPSETTING(    0x00, "Remote Off" ) PORT_CONDITION("IN1",0x08,EQUALS,0x00)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_NAME("Double") PORT_CODE(KEYCODE_S)

	PORT_START("IN4")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNKNOWN ) /* not used */
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("Hopper Low") PORT_CODE(KEYCODE_H)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_POKER_HOLD2 )
	PORT_DIPNAME( 0x08, 0x08, "Auto Hold" )      PORT_DIPLOCATION("SW1:2") /* DSW2 */
	PORT_DIPSETTING(    0x08, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_GAMBLE_DEAL ) PORT_NAME("Deal / Take")

	PORT_START("IN5")
	PORT_DIPNAME( 0x01, 0x01, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("Return Line") PORT_CODE(KEYCODE_J)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("TILT") PORT_CODE(KEYCODE_K)
	PORT_DIPNAME( 0x08, 0x08, "Rate Table SW1" ) PORT_DIPLOCATION("SW1:3") /* DSW3 (should be arranged with DSW4) */
	PORT_DIPSETTING(    0x08, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_POKER_HOLD3 )

	PORT_START("IN6")
	PORT_DIPNAME( 0x01, 0x01, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_GAMBLE_BET )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_POKER_HOLD5 )
	PORT_DIPNAME( 0x08, 0x08, "Rate Table SW2" ) PORT_DIPLOCATION("SW1:4") /* DSW4 (should be arranged with DSW3) */
	PORT_DIPSETTING(    0x08, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_COIN1 ) PORT_IMPULSE(2)

	PORT_START("IN7")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_UNKNOWN ) /* not used */
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("Remote Credit") PORT_IMPULSE(12) PORT_CODE(KEYCODE_3)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_COIN4 ) PORT_IMPULSE(2)
	PORT_DIPNAME( 0x08, 0x08, "Jackpot" )        PORT_DIPLOCATION("SW1:8") /* DSW8 */
	PORT_DIPSETTING(    0x08, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("Clear Credits") PORT_CODE(KEYCODE_4)
INPUT_PORTS_END

static INPUT_PORTS_START( sigma2k )
	PORT_START("IN0")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_DIPNAME( 0x08, 0x08, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x08, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("IN1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_POKER_HOLD1 )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_SERVICE ) PORT_NAME("Operator Key") PORT_TOGGLE
	PORT_DIPNAME( 0x08, 0x08, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x08, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_POKER_HOLD4 )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("IN2")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_DIPNAME( 0x08, 0x08, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x08, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("IN3")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("Supervisor Key") PORT_TOGGLE PORT_CODE(KEYCODE_0)
	PORT_DIPNAME( 0x08, 0x08, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x08, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_GAMBLE_HALF ) PORT_NAME("Half Gamble")
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("IN4")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_POKER_HOLD2 )
	PORT_DIPNAME( 0x08, 0x08, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x08, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_GAMBLE_DEAL ) PORT_NAME("Deal / Take")
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("IN5")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_DIPNAME( 0x08, 0x08, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x08, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_POKER_HOLD3 )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("IN6")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_GAMBLE_BET )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_POKER_HOLD5 )
	PORT_DIPNAME( 0x08, 0x08, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x08, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("IN7")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_COIN1 ) PORT_NAME("Credits In")
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_DIPNAME( 0x08, 0x08, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x08, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("Clear Credits") PORT_CODE(KEYCODE_4)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNUSED )
INPUT_PORTS_END

static INPUT_PORTS_START( piccolop )
	PORT_START("IN0")
	PORT_DIPNAME( 0x01, 0x01, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("IN0-2") PORT_CODE(KEYCODE_1_PAD)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_COIN2 ) PORT_IMPULSE(2)
	PORT_DIPNAME( 0x08, 0x08, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x08, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_COIN3 ) PORT_IMPULSE(2)

	PORT_START("IN1")
	PORT_DIPNAME( 0x01, 0x01, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_GAMBLE_DEAL ) PORT_NAME("Deal/Take")
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_GAMBLE_SERVICE ) PORT_NAME("Operator Key") PORT_TOGGLE
	PORT_DIPNAME( 0x08, 0x08, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x08, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("IN1-5") PORT_CODE(KEYCODE_2_PAD)

	PORT_START("IN2")
	PORT_DIPNAME( 0x01, 0x01, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("IN2-2") PORT_CODE(KEYCODE_3_PAD)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_GAMBLE_DOOR ) PORT_NAME("Door Switch")
	PORT_DIPNAME( 0x08, 0x08, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x08, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_POKER_HOLD1 )

	PORT_START("IN3")
	PORT_DIPNAME( 0x01, 0x01, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_SERVICE ) PORT_NAME("Hopper Out") PORT_CODE(KEYCODE_G)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_SERVICE ) PORT_NAME("Supervisor Key") PORT_TOGGLE PORT_CODE(KEYCODE_0)
	PORT_DIPNAME( 0x08, 0x08, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x08, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_POKER_HOLD3 )

	PORT_START("IN4")
	PORT_DIPNAME( 0x01, 0x01, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_SERVICE ) PORT_NAME("Hopper Low") PORT_CODE(KEYCODE_H)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_POKER_HOLD4 ) PORT_NAME("Hold 4 / Red")
	PORT_DIPNAME( 0x08, 0x08, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x08, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_POKER_HOLD2 )

	PORT_START("IN5")
	PORT_DIPNAME( 0x01, 0x01, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_SERVICE ) PORT_NAME("Return Line") PORT_CODE(KEYCODE_J)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_SERVICE ) PORT_NAME("TILT") PORT_CODE(KEYCODE_K)
	PORT_DIPNAME( 0x08, 0x08, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x08, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_POKER_HOLD5 ) PORT_NAME("Hold 5 / Black")

	PORT_START("IN6")
	PORT_DIPNAME( 0x01, 0x01, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("IN6-2") PORT_CODE(KEYCODE_4_PAD)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("IN6-3") PORT_CODE(KEYCODE_5_PAD)
	PORT_DIPNAME( 0x08, 0x08, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x08, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_COIN1 ) PORT_IMPULSE(2)

	PORT_START("IN7")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_UNKNOWN)    // lack of v-sync if low
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("IN7-2") PORT_CODE(KEYCODE_6_PAD)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("IN7-3") PORT_CODE(KEYCODE_7_PAD)
	PORT_DIPNAME( 0x08, 0x08, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x08, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_SERVICE ) PORT_NAME("Coin Refill") PORT_CODE(KEYCODE_R)
INPUT_PORTS_END


/*************************
*    Graphics Layouts    *
*************************/

static const gfx_layout charlayout =
{
	8, 8,
	1024,
	2,
	{ 0, 4 },
	{ 0, 1, 2, 3, 8, 9, 10, 11 },
	{ 0*8, 2*8, 4*8, 6*8, 8*8, 10*8, 12*8, 14*8 },
	16*8
};

static const gfx_layout s2k_charlayout =
{
	8, 8,
	4096,
	2,
	{ 0, 4 },
	{ 0, 1, 2, 3, 8, 9, 10, 11 },
	{ 0*8, 2*8, 4*8, 6*8, 8*8, 10*8, 12*8, 14*8 },
	16*8
};


/******************************
* Graphics Decode Information *
******************************/

static GFXDECODE_START( ampoker2 )
	GFXDECODE_ENTRY( "gfx1", 0x0000, charlayout, 0, 128 )
GFXDECODE_END

static GFXDECODE_START( sigma2k )
	GFXDECODE_ENTRY( "gfx1", 0x0000, s2k_charlayout, 0, 128 )
GFXDECODE_END

/*************************
*     Machine Driver     *
*************************/

static MACHINE_CONFIG_START( ampoker2, ampoker2_state )

	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", Z80, MASTER_CLOCK/2)        /* 3 MHz */
	MCFG_CPU_PROGRAM_MAP(ampoker2_map)
	MCFG_CPU_IO_MAP(ampoker2_io_map)
	MCFG_CPU_PERIODIC_INT_DRIVER(ampoker2_state, nmi_line_pulse,  1536)
	MCFG_WATCHDOG_TIME_INIT(attotime::from_msec(200))   /* 200 ms, measured */

	MCFG_NVRAM_ADD_0FILL("nvram")

	/* video hardware */
	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_REFRESH_RATE(60)
	/*  if VBLANK is used, the watchdog timer stop to work.
	MCFG_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(0))
	*/
	MCFG_SCREEN_SIZE(64*8, 32*8)
	MCFG_SCREEN_VISIBLE_AREA(20*8, 56*8-1, 2*8, 32*8-1)
	MCFG_SCREEN_UPDATE_DRIVER(ampoker2_state, screen_update_ampoker2)
	MCFG_SCREEN_PALETTE("palette")

	MCFG_GFXDECODE_ADD("gfxdecode", "palette", ampoker2)
	MCFG_PALETTE_ADD("palette", 512)
	MCFG_PALETTE_INIT_OWNER(ampoker2_state, ampoker2)

	/* sound hardware */
	MCFG_SPEAKER_STANDARD_MONO("mono")
	MCFG_SOUND_ADD("aysnd", AY8910,MASTER_CLOCK/4)  /* 1.5 MHz, measured */
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.30)
MACHINE_CONFIG_END

static MACHINE_CONFIG_DERIVED( sigma2k, ampoker2 )

	/* basic machine hardware */

	/* video hardware */
	MCFG_GFXDECODE_MODIFY("gfxdecode", sigma2k)
	MCFG_VIDEO_START_OVERRIDE(ampoker2_state,sigma2k)
MACHINE_CONFIG_END


/*************************
*        Rom Load        *
*************************/

ROM_START( ampoker2 )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "poker9.003", 0x4000, 0x8000, CRC(a31221fc) SHA1(4a8bdd8ce8d5bff7e7cfc4ae91e27c1d366dc54d) )
	ROM_COPY( "maincpu", 0x8000, 0x0000, 0x4000 ) /* poker9.003 contains the 16K halves swapped around */
	ROM_LOAD( "poker9.002", 0x8000, 0x4000, CRC(bfde5bce) SHA1(c7c7ca2268694015e8ec673e8fa5c48043086d3f) )

	ROM_REGION( 0x4000, "gfx1", 0 )
	ROM_LOAD( "poker9.028", 0x0000, 0x4000, CRC(65bccb40) SHA1(75f154a2aaf9f9be62e0e1dd8cbe630b9ea0145c) )

	ROM_REGION( 0x200, "proms", 0 )
	ROM_LOAD( "82s147an.u48", 0x0000, 0x0200, CRC(9bc8e543) SHA1(e4882868a43e21a509a180b9731600d1dd63b5cc) )
ROM_END

ROM_START( ampkr2b1 )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "airl-v00.512", 0x0000, 0x10000, CRC(e5953bf4) SHA1(291367431e3b21b57704228c63e4da853e6d25b7) )

	ROM_REGION( 0x4000, "gfx1", 0 )
	ROM_LOAD( "ampoker.u47", 0x0000, 0x4000, CRC(cefed6c7) SHA1(79591339eab2712b432dfe89929dbc97000a13d2) )

	ROM_REGION( 0x200, "proms", 0 )
	ROM_LOAD( "82s147an.u48", 0x0000, 0x0200, CRC(9bc8e543) SHA1(e4882868a43e21a509a180b9731600d1dd63b5cc) )
ROM_END

ROM_START( ampkr2b2 )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "rom9.u6", 0x0000, 0x10000, CRC(820a491d) SHA1(36654aacac010e7c086dd18d4e0ca5d959b9044f) )

	ROM_REGION( 0x4000, "gfx1", 0 )
	ROM_LOAD( "rom0.u47", 0x0000, 0x4000, CRC(cefed6c7) SHA1(79591339eab2712b432dfe89929dbc97000a13d2) )

	ROM_REGION( 0x200, "proms", 0 )
	ROM_LOAD( "82s147an.u48", 0x0000, 0x0200, CRC(9bc8e543) SHA1(e4882868a43e21a509a180b9731600d1dd63b5cc) )
ROM_END

ROM_START( ampkr2b3 )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "ampoker.u6", 0x0000, 0x10000, CRC(d7b055bd) SHA1(f5231d2ec80f740eabedaba07547ccbb977accc1) )

	ROM_REGION( 0x4000, "gfx1", 0 )
	ROM_LOAD( "ampoker.u47", 0x0000, 0x4000, CRC(cefed6c7) SHA1(79591339eab2712b432dfe89929dbc97000a13d2) )

	ROM_REGION( 0x200, "proms", 0 )
	ROM_LOAD( "82s147an.u48", 0x0000, 0x0200, CRC(9bc8e543) SHA1(e4882868a43e21a509a180b9731600d1dd63b5cc) )
ROM_END

ROM_START( ampkr2b4 )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "9.bin", 0x0000, 0x10000, CRC(657fa846) SHA1(1ef8fea81627b86aab6f682919d7432c57816e5f) )

	ROM_REGION( 0x4000, "gfx1", 0 )
	ROM_LOAD( "ampoker.u47", 0x0000, 0x4000, CRC(cefed6c7) SHA1(79591339eab2712b432dfe89929dbc97000a13d2) )

	ROM_REGION( 0x200, "proms", 0 )
	ROM_LOAD( "82s147an.u48", 0x0000, 0x0200, CRC(9bc8e543) SHA1(e4882868a43e21a509a180b9731600d1dd63b5cc) )
ROM_END

ROM_START( ampkr95 )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "amp95rus.u6", 0x0000, 0x10000, CRC(6ec74b2b) SHA1(2dca05bc111071f1407dd524b67b5a3dc5848c70) )

	ROM_REGION( 0x4000, "gfx1", 0 )
	ROM_LOAD( "ampoker.u47", 0x0000, 0x4000, CRC(cefed6c7) SHA1(79591339eab2712b432dfe89929dbc97000a13d2) )

	ROM_REGION( 0x200, "proms", 0 )
	ROM_LOAD( "82s147an.u48", 0x0000, 0x0200, CRC(9bc8e543) SHA1(e4882868a43e21a509a180b9731600d1dd63b5cc) )
ROM_END

/*
  iamp2 v28

- Taiwanese board.
- Original Novomatic program??
- Gfx set seems from a bootleg...
- No scroll in the attract.
- Analysis page in operator/supervisor mode.
- Min-Max bet, and a kind of 3-strings password given in supervisor mode.

*/
ROM_START( ampkr228 )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "9.u6",  0x0000, 0x10000, CRC(747316cf) SHA1(7c2bb7a1a28e421a27f743eefe3c8878967ce4a9) )

	ROM_REGION( 0x4000, "gfx1", 0 )
	ROM_LOAD( "0.u47", 0x0000, 0x4000, CRC(cefed6c7) SHA1(79591339eab2712b432dfe89929dbc97000a13d2) )

	ROM_REGION( 0x200, "proms", 0 )
	ROM_LOAD( "82s147an.u48", 0x0000, 0x0200, CRC(9bc8e543) SHA1(e4882868a43e21a509a180b9731600d1dd63b5cc) )

	ROM_REGION( 0x400, "plds", 0 )
	ROM_LOAD( "gal16v8a_bad.u41", 0x0000, 0x0117, CRC(a26ba7e6) SHA1(fd22ccb1ff3bf6956f300668ecd8cfe699182b39) )
	ROM_LOAD( "gal16v8b.u8",      0x0200, 0x0117, CRC(7edb3276) SHA1(1302aec1d9703e6ce9da77fc7a0613e7eff1ccb5) )
ROM_END

ROM_START( pkrdewin )
	ROM_REGION( 0x14000, "maincpu", 0 )
	ROM_LOAD( "poker7.001", 0x4000, 0x10000, CRC(eca16b9e) SHA1(5063d733721457ab3b08caafbe8d33b2cbe4f88b) )
	ROM_COPY( "maincpu",    0x8000, 0x0000, 0x4000 ) /* poker7.001 contains the 1st and 2nd 16K quarters swapped */
	ROM_COPY( "maincpu", 0x10000, 0x8000, 0x4000 ) /* poker7.001 contains the 1st and 2nd 16K quarters swapped */

	ROM_REGION( 0x4000, "gfx1", 0 )
	ROM_LOAD( "poker7.002", 0x0000, 0x4000, CRC(65bccb40) SHA1(75f154a2aaf9f9be62e0e1dd8cbe630b9ea0145c) )

	ROM_REGION( 0x200, "proms", 0 )
	ROM_LOAD( "82s147an.u48", 0x0000, 0x0200, CRC(9bc8e543) SHA1(e4882868a43e21a509a180b9731600d1dd63b5cc) )
ROM_END

ROM_START( videomat )   /* polish bootleg */
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "rom.bin", 0x0000, 0x10000, CRC(910cd941) SHA1(350ca70370c5082901343d0c0c1424729d77b006) )

	ROM_REGION( 0x4000, "gfx1", 0 )
	ROM_LOAD( "rom0.u47", 0x0000, 0x4000, CRC(cefed6c7) SHA1(79591339eab2712b432dfe89929dbc97000a13d2) )

	ROM_REGION( 0x200, "proms", 0 )
	ROM_LOAD( "82s147an.u48", 0x0000, 0x0200, CRC(9bc8e543) SHA1(e4882868a43e21a509a180b9731600d1dd63b5cc) )
ROM_END

ROM_START( sigmapkr )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "sigmapkr.u6", 0x0000, 0x10000, CRC(aa3f429a) SHA1(8c82e86de7280590ba157860cbf9783f893f8554) )

	ROM_REGION( 0x4000, "gfx1", 0 )
	ROM_LOAD( "sigmapkr.u47", 0x0000, 0x4000, CRC(49eb69a8) SHA1(22be5870d501d229aa56fb18146ec0d8f8eea72e) )

	ROM_REGION( 0x200, "proms", 0 )
	ROM_LOAD( "82s147an_spkr.u48", 0x0000, 0x0200, CRC(3d8683d0) SHA1(1d99cd89db1b3c8e14bdafab05d1f70ad5bc604d) )
ROM_END

ROM_START( sigma2k )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "sigma2k.u6", 0x0000, 0x10000, CRC(608d1771) SHA1(0ec94d780565472c7e68da7e3ce19aea3f1ab4a5) )

	ROM_REGION( 0x10000, "gfx1", 0 )
	ROM_LOAD( "sigma2k.u47", 0x0000, 0x10000, CRC(3ed7b9df) SHA1(788a90ffa6cb0bfebf607815a695a5afe930945c) )

	ROM_REGION( 0x200, "proms", 0 )
	ROM_LOAD( "82s147an_s2k.u48", 0x0000, 0x0200, CRC(715361cc) SHA1(cac239399c9ec5d7498e49a906fb5b934ef7f4dc) )
ROM_END

/*

Rabbit Poker, or Arizona Poker 1.1 ??

American Poker 2 board
program rom on small daughter board
with GAL22V10 and PIC16F84A
prom not dumped

*/
ROM_START( rabbitpk )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "poldi_ren.u6", 0x0000, 0x10000, CRC(ef0d5b47) SHA1(5d209c803ab8ced08953d24202a364ce1aa677c2) )

	ROM_REGION( 0x4000, "gfx1", 0 )
	ROM_LOAD( "poldi_graf.u47", 0x0000, 0x4000, CRC(f1807f39) SHA1(631645272c7508104749e0ff1357bd74098851d5) )

	ROM_REGION( 0x200, "proms", 0 )  /* not dumped. using the ampoker2 one instead */
	ROM_LOAD( "82s147an.u48", 0x0000, 0x0200, CRC(9bc8e543) SHA1(e4882868a43e21a509a180b9731600d1dd63b5cc) )
ROM_END

/*

Piccolo Poker (Admiral, licenced by Novomatic).
Seems a interesting American Poker II variant.

Roms have swapped halves.
Rechecked on PCB.

*/
ROM_START( piccolop )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "v4.1.bin", 0x4000, 0x4000, CRC(ae092c43) SHA1(191e233310d59d3b4eb71c7081799835efcae069) )
	ROM_CONTINUE(         0x0000, 0x4000)
	ROM_LOAD( "v4.2.bin", 0xc000, 0x4000, CRC(69fb6fd5) SHA1(e95c2793aaa11b9501ca544dd0a045e8e7bc52bf) )
	ROM_CONTINUE(         0x8000, 0x4000)

	ROM_REGION( 0x8000, "gfx1", 0 )
	ROM_LOAD( "zei_8.11.bin", 0x4000, 0x4000, CRC(1b003672) SHA1(e58bd58023f332c30851204491b7e0bd7c5d9631) )
	ROM_CONTINUE(             0x0000, 0x4000)

	ROM_REGION( 0x200, "proms", 0 )  /* not dumped. using the ampoker2 one instead */
	ROM_LOAD( "82s147an.u48", 0x0000, 0x0200, CRC(9bc8e543) SHA1(e4882868a43e21a509a180b9731600d1dd63b5cc) )
ROM_END


/*************************
*      Driver Init       *
*************************/

DRIVER_INIT_MEMBER(ampoker2_state,rabbitpk)
{
	UINT8 *rom = memregion("maincpu")->base();
	int size = memregion("maincpu")->bytes();
	int start = 0;
	int i;

	UINT8 dec_base[32] =
	{
		0x00, 0x43, 0x45, 0x06, 0xc3, 0x80, 0x86, 0xc5,
		0x84, 0xc7, 0xc1, 0x82, 0x47, 0x04, 0x02, 0x41,
		0x86, 0xc5, 0xc3, 0x80, 0x45, 0x06, 0x00, 0x43,
		0x02, 0x41, 0x47, 0x04, 0xc1, 0x82, 0x84, 0xc7
	};

	for (i = start; i < size; i++)
	{
		rom[i] = BITSWAP8(rom[i], 1, 2, 5, 4, 3, 0, 7, 6) ^ dec_base[(i >> 2) & 0x1f];
	}
}

DRIVER_INIT_MEMBER(ampoker2_state,piccolop)
{
/*
  The protection is based on a stuck bit at RAM offset $C416.

  1382: 41            ld   b,c
  1383: 80            add  a,b
  1384: 00            nop  ------\
  1385: 00            nop         |  Obvious patch...
  1386: 00            nop         |  Dunno if was made originarily.
  1387: 00            nop  ------/
  1388: 3E 08         ld   a,$08
  138A: D3 37         out  ($37),a   ; Sets bit3 to keep happy the watchdog reset.
  138C: 32 01 C0      ld   ($C001),a
  138F: 18 FE         jr   $138F     ; INFINITE LOOP --- WTH???

  1541: 21 16 C4      ld   hl,$C416  ; Load $C416 into HL...
  1544: CB 4E         bit  1,(hl)
  1546: 7E            ld   a,(hl)    ; Load the $C416 contents...
  1547: E6 E0         and  $E0       ; AND $E0
  1549: FE 40         cp   $40       ; Compare with $40
  154B: C2 88 13      jp   nz,$1388  ; Jumps to stuck!

  154E: FD 21 AA C0   ld   iy,$C0AA  ; Else continue...
  1552: 11 16 C5      ld   de,$C516
  1555: 21 88 57      ld   hl,$5788
  1558: 06 08         ld   b,$08
  155A: 1A            ld   a,(de)
  155B: BE            cp   (hl)
  155C: C2 13 2A      jp   nz,$2A13

*/

	UINT8 *rom = memregion("maincpu")->base();

	// NOP'ing the mortal jump...
	rom[0x154b] = 0x00;
	rom[0x154c] = 0x00;
	rom[0x154d] = 0x00;

}


/*************************
*      Game Drivers      *
*************************/

//     YEAR  NAME      PARENT    MACHINE   INPUT     STATE           INIT      ROT    COMPANY              FULLNAME                             FLAGS                   LAYOUT
GAMEL( 1990, ampoker2, 0,        ampoker2, ampoker2, driver_device,  0,        ROT0, "Novomatic",         "American Poker II",                  MACHINE_SUPPORTS_SAVE,     layout_ampoker2 )
GAMEL( 1990, ampkr2b1, ampoker2, ampoker2, ampoker2, driver_device,  0,        ROT0, "bootleg",           "American Poker II (bootleg, set 1)", MACHINE_SUPPORTS_SAVE,     layout_ampoker2 )
GAMEL( 1990, ampkr2b2, ampoker2, ampoker2, ampoker2, driver_device,  0,        ROT0, "bootleg",           "American Poker II (bootleg, set 2)", MACHINE_SUPPORTS_SAVE,     layout_ampoker2 )
GAMEL( 1994, ampkr2b3, ampoker2, ampoker2, ampoker2, driver_device,  0,        ROT0, "bootleg",           "American Poker II (bootleg, set 3)", MACHINE_SUPPORTS_SAVE,     layout_ampoker2 )
GAMEL( 1994, ampkr2b4, ampoker2, ampoker2, ampoker2, driver_device,  0,        ROT0, "bootleg",           "American Poker II (bootleg, set 4)", MACHINE_SUPPORTS_SAVE,     layout_ampoker2 )
GAMEL( 1994, ampkr228, ampoker2, ampoker2, ampoker2, driver_device,  0,        ROT0, "bootleg?",          "American Poker II (iamp2 v28)",      MACHINE_SUPPORTS_SAVE,     layout_ampoker2 )
GAMEL( 1995, ampkr95,  ampoker2, ampoker2, ampkr95,  driver_device,  0,        ROT0, "bootleg",           "American Poker 95",                  MACHINE_SUPPORTS_SAVE,     layout_ampoker2 )
GAMEL( 1990, pkrdewin, ampoker2, ampoker2, ampoker2, driver_device,  0,        ROT0, "bootleg",           "Poker De Win",                       MACHINE_SUPPORTS_SAVE,     layout_ampoker2 )
GAMEL( 1990, videomat, ampoker2, ampoker2, ampoker2, driver_device,  0,        ROT0, "bootleg",           "Videomat (Polish bootleg)",          MACHINE_SUPPORTS_SAVE,     layout_ampoker2 )
GAMEL( 1990, rabbitpk, ampoker2, ampoker2, ampoker2, ampoker2_state, rabbitpk, ROT0, "bootleg",           "Rabbit Poker (Arizona Poker v1.1?)", MACHINE_SUPPORTS_SAVE,     layout_ampoker2 )
GAMEL( 1995, sigmapkr, 0,        ampoker2, sigmapkr, driver_device,  0,        ROT0, "Sigma Inc.",        "Sigma Poker",                        MACHINE_SUPPORTS_SAVE,     layout_sigmapkr )
GAMEL( 1998, sigma2k,  0,        sigma2k,  sigma2k,  driver_device,  0,        ROT0, "Sigma Inc.",        "Sigma Poker 2000",                   MACHINE_SUPPORTS_SAVE,     layout_sigmapkr )
GAME(  1990, piccolop, ampoker2, ampoker2, piccolop, ampoker2_state, piccolop, ROT0, "Admiral/Novomatic", "Piccolo Poker 100",                  MACHINE_SUPPORTS_SAVE )
