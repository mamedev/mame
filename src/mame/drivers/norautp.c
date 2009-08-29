/******************************************************************************

   - NORAUT POKER -
  ------------------

  Driver by Roberto Fresca & Angelo Salese.


  Games running on this hardware:

   * Noraut Poker,                 1988,  Noraut Ltd.
   * Noraut Joker Poker,           1988,  Noraut Ltd.
   * Noraut Red Hot Joker Poker,   1988,  Noraut Ltd.
   * Noraut Poker (NTX10A),        1988,  Noraut Ltd.
   * Noraut Joker Poker (V3.010a), 1988,  Noraut Ltd.
   * GTI Poker,                    1983,  GTI Inc.
   * Noraut Poker (bootleg),       198?,  Unknown.


*******************************************************************************


  HARDWARE NOTES:
  ---------------

  Hardware Layout (norautp):

  1x Z80
  3x PPI 8255
  2x 6116 SRAM
  1x 3.6 Vcc Battery.
  1x 18.432 MHz. Xtal.

  1x 555 + unknown yellow resonator, near the edge connector.
  1x 555 + resnet, near the battery.

  1x 10 DIP switches bank.
  2x 3pins jumpers (between the Z80 and ROM)

     JP1 (ABC);  JP2 (DEF)

  PCB silksceened:  AB+DE=512  BC+DE=256
                    (CUT BC)   EF=64/128

  PCB silksceened:  SMART-BOARD 131191 ISS.E (Made in USA)

  -------------------------------------------------------------


  Hardware Layout (norautjp):

  - CPU:             1x TMPZ84C00AP-8
  - RAM:             2x HM6116LP-4 CMOS Static Ram
  - I/O:             3x D8255AC-2 Peripeheral Interface Adapter
  - Prg ROMs:        1x 2764 Eprom
  - Gfx ROMs:        1x 2732 Eprom
  - Sound:           Discrete
  - Battery:         1x 3.6v Ni-cd 65Mah
  - Crystal:         1x 18.432Mhz
  - Resistor Array:  4x 9 Pin SIP 472G


  PCB Layout (norautjp):                                                       Edge Connector 36x2
   ______________________________________________________________________________________________
  |  _____                  _________    _________    _____         .........    _________       |
  | |D5555|                |74LS174N |  |74LS153N |  |D5555|        .........   |ULN2003A |      |
  | |_____|                |_________|  |_________|  |_____|    Resistor Array  |_________|      |
  |                                                               ___                            |
  |                                                              |VR1|                           |
  |              DIP SW x4                                       |___|                           |
  |  ________     _______                                                                        |
  | |Battery |   |1|2|3|4|  _________    _________    _________    _________     _________       |
  | |  3.6v  |   |_|_|_|_| |74LS157N |  |74LS153N |  |74LS161AP|  |74LS86AN |   |ULN2003A |      |
  | |________|             |_________|  |_________|  |_________|  |_________|   |_________|      |
  |                                                                                              |
  |                                                                                              |
  |                                                                                              | 36
  |  _________              _________                 _________    _________     _________       |___
  | |HD74LS04P|            |74LS166AP|               |74LS161AN|  |74LS153N |   |ULN2003A |       ___|
  | |_________|            |_________|               |_________|  |_________|   |_________|       ___|
  |                                                                                               ___|
  |                                   DIP SW x 8                               ________________   ___|
  |               _____________    _______________    _________    _________  |                |  ___|
  |              |             |  |1|2|3|4|5|6|7|8|  |74LS161AN|  |74LS157N | |    D8255AC-2   |  ___|
  |              |    2732     |  |_|_|_|_|_|_|_|_|  |_________|  |_________| |________________|  ___|
  |              |_____________|                                                                  ___|
  |                                                                                               ___|
  |                                                                            ________________   ___|
  |                                                                           |                |  ___|
  |  _____________        _____________                                       |    D8255AC-2   |  ___|
  | |             |      |             |                                      |________________|  ___|
  | |    6116     |      |    6116     |                                                          ___|
  | |_____________|      |_____________|              _________    _________                      ___|
  |                                                  |74LS161AN|  |74LS157N |                     ___|
  |                                                  |_________|  |_________|                     ___|
  |                                                                                               ___|
  |  ______________       ________________                                                        ___|
  | |              |     |                |           _________    _________                      ___|
  | |     2764     |     |    D8255AC-2   |          |74LS161AN|  |74LS157N |     .........       ___|
  | |______________|     |________________|          |_________|  |_________|     .........       ___|
  |                                                                              Resistor Array   ___|
  |                                                                                               |
  |                                                                                               | 1
  |                      _________                    _________    _________    _________         |
  |                     | 74LS32N |                  |74LS161AN|  | 74LS86P |  | 74LS04N |        |
  |                     |_________|                  |_________|  |_________|  |_________|        |
  |                                                                                               |
  |                                             XTAL                                              |
  |                                            .----.                                             |
  |  ____________________     __________      _________    _________    _________    _________    |
  | |                    |   |PALce16v8H|    | 74LS04N |  |74LS157N |  | 74LS11N |  |74LS74AN |   |
  | |   TMPZ84C00AP-8    |   |__________|    |_________|  |_________|  |_________|  |_________|   |
  | |____________________|                                                                        |
  |                                                                                               |
  |_______________________________________________________________________________________________|



*******************************************************************************


  Noraut Edge Connector (pinouts)
  --------------------------------
  Component     PN   Solder Side
  --------------------------------
  GND           01   GND
  5v DC         02   5v DC
                03
  12v DC        04   12v DC
                05
                06
                07
  0v            08   Readout Switch
  0v            09   Low level hopper
  0v            10   50p in
  0v            11   pound in
  0v            12   Bet switch
  0v            13   Deal switch
  0v            14   Hold 1 switch
  0v            15   Half Gamble switch
  0v            16   Change Card switch
  Refil         17   Coin count/sense from hopper
  Low Switch    18   High swicth
  Hold 3 Switch 19   Hold 2 switch
  Hold 5 Switch 20   Hold 4 switch
  10p coin      21   Deflect
                22   50p in meter
                23   Hopper Motor Drive (low volt switch line NOT 24v)
                24
                25   spk+
                26   Panel lamps clock
  Monitor sync  27   Hold 1 lamp
  Bet lamp      28   Deal lamp
  Change lamp   29   Hold 4 lamp
  Hold 5 lamp   30   Panel lights reset
  High lamp     31   Half Gamble lamp
  Hold 2 lamp   32   Low lamp
  10p Meter out 33   Meter refil
  Video Green   34   Hold 3 lamp
  Video Blue    35   10p in Meter
  Video Red     36   Spark Detect (Not on all boards)

*******************************************************************************

  Control Panel
  -------------

  There are 2 control panel schemes:

  * The default one (11 button-lamps) for systems without CANCEL button.

  .--------------------------------------------------------------------------.
  |                                              .-------. .------. .------. |
  | .------. .------. .------. .------. .------. |  BET  | | HALF | |  HI  | |
  | |      | |      | |      | |      | |      | |COLLECT| |GAMBLE| |      | |
  | | HOLD | | HOLD | | HOLD | | HOLD | | HOLD | '-------' '------' '------' |
  | |CANCEL| |CANCEL| |CANCEL| |CANCEL| |CANCEL| .-------. .------. .------. |
  | |      | |      | |      | |      | |      | | DEAL  | |CHANGE| |  LO  | |
  | '------' '------' '------' '------' '------' | DRAW  | | CARD | |      | |
  |                                              '-------' '------' '------' |
  '--------------------------------------------------------------------------'

   HOLD buttons              = red.
   BET, DEAL, HI & LO        = yellow.
   HALF GAMBLE & CHANGE CARD = orange. 


  * The alternate one (12 button-lamps) for systems with CANCEL button.

  .-------------------------------------------------------------.
  | .------. .------. .------. .------. .------.   .----------. | 
  | | HOLD | | HOLD | | HOLD | | HOLD | | HOLD |   |   HIGH   | |
  | '------' '------' '------' '------' '------'   '----------' |
  | .------. .------. .------. .------. .------.   .----------. |
  | |CANCEL| |STAND | | SAVE | | DEAL | | BET  |   |   LOW    | |
  | '------' '------' '------' '------' '------'   '----------' |
  '-------------------------------------------------------------'

   HOLD & CANCEL buttons     = yellow (1).
   STAND & DEAL buttons      = orange (1).
   SAVE (HALF GAMBLE) button = blued-green (1).
   BET button                = red (1).
   HIGH & LOW buttons        = yellow.

   (1) Circular-shaped buttons.

  Some lamps are wired in different way in this scheme.


*******************************************************************************


  Noraut Poker discrete audio circuitry
  -------------------------------------

  3x ULN2003A (Darlington transistor array)
  1x D5555C   (CMOS timer)
  1x KN2222A  (Epitaxial planar general purpose NPN transistor)
  1x VR/POT

  .------.                              .------------.               .-------.
  |      |                              |   D5555C   |               |KN2222A|
  |      |                             4|            |3     R3       |       |      -->
  |   PC7|------------------------------|RST      OUT|-----ZZZZ------|B     E|>----ZZZZZ-----> Audio Out.
  |   PC6|----------.                  6|            |8              |   C   |      VR1
  |   PC5|-----.    |2-in         .-----|THR      VCC|-----------.   '---+---'          .----> Audio Out.
  |   PC4|--.  |  .-+------.      |    5|            |7          |       |              |
  |      |  |  |  |ULN2003A|      |  .--|CVOLT   DISC|--.        |       |              |
  |      |  |  |  '-+------'      |  |  |            |  |        +-------'            --+--
  |      |  |  |    |2-out   C1   |  |  |    GND     |  |        |                     GND
  | 8255 |  |  |    '--------||---+  |  '-----+------'  |        |                      -
  |  AP  |  |  '--.               |  |        |1        |        |                      '
  |      |  |     |3-in           |  |        |         |        |
  '------'  |  .--+-----.         |  |   C5   |         |        |
            |  |ULN2003A|         |  '---||---+         |        |    +5V
            |  '--+-----'         |           |         |        |    -+-
            |     |3-out     C2   |      C4   |         |    C6  |     |
            |     '----------||---+------||---+-------. | .--||--+-----'
            |2-in                 |           |       | | |      |
          .-+------.              |         --+--      '-'       |
          |ULN2003A|              |          GND        |        |
          '-+------'              |           -         |        |
            |2-out           C3   |     R1    '         |   R2   |
            '----------------||---+----ZZZZ-------------+--ZZZZ--'

  VR1 = 100 ohms

  R1 = 120 K ; Tolerance +/- 5%
  R2 = 2.2 K ; Tolerance +/- 5%
  R3 = 1 K   ; Tolerance +/- 5%

  C1 = 223J = 22000 pF  =  22 nF = 0.022 uF ; Tolerance +/- 5%
  C2 = 103J = 10000 pF  =  10 nF = 0.01 uF  ; Tolerance +/- 5%
  C3 = 473J = 47000 pF  =  47 nF = 0.047 uF ; Tolerance +/- 5%
  C4 = 103J = 10000 pF  =  10 nF = 0.01 uF  ; Tolerance +/- 5%

  C5 = 103  = 10000 pF  =  10 nF = 0.01 uF
  C6 = 104  = 100000 pF = 100 nF = 0.1 uF

  - C1, C2, C3 & C4 are polyester film / mylar capacitors.
  - C5 & C6 are ceramic capacitors.
  - All Capacitors are non-polarised.


*******************************************************************************


  Narout System Ports Map
  -----------------------

  (*) Confirmed lines.


  PPI-0 (60h-63h); PortA IN.
  DIP Switches bank:

  7654 3210
  ---- ---x  * DIP switch 8
  ---- --x-  * DIP switch 7
  ---- -x--  * DIP switch 6
  ---- x---  * DIP switch 5
  ---x ----  * DIP switch 4
  --x- ----  * DIP switch 3
  -x-- ----  * DIP switch 2
  x--- ----  * DIP switch 1


  PPI-0 (60h-63h); PortB OUT.
  Lamps:

  7654 3210
  ---- ---x  * CHANGE CARD lamp.
  ---- --x-  * SAVE / HALF GAMBLE lamp.
  ---- -x--  * HOLD 1 lamp.
  ---- x---  * HOLD 2 lamp.
  ---x ----  * HOLD 3 lamp.
  --x- ----  * HOLD 4 lamp.
  -x-- ----  * HOLD 5 lamp.
  x--- ----  * CANCEL lamp.


  PPI-0 (60h-63h); PortC OUT.
  Lamps & Coin Counters:

  7654 3210
  ---- ---x  * HI lamp.
  ---- --x-  * LO lamp.
  ---- -x--  unknown.
  ---- x---  * Payout pulse.
  ---x ----  * Coin 2 counter.
  --x- ----  * Coin 1 counter.
  -x-- ----  unknown.
  x--- ----  unknown (always activated).


-----------------------------------------------------------

  PPI-1 (a0h-a3h); PortA IN.
  Regular Inputs:

  7654 3210
  ---- ---x  * DEAL / DRAW button.
  ---- --x-  * BET / CHANGE CARD button.
  ---- -x--  * COIN 1 mech.
  ---- x---  * COIN 2 mech.
  ---x ----  * READOUT button.
  --x- ----  * HI button.
  -x-- ----  * LO button.
  x--- ----  * PAYOUT button.


  PPI-1 (a0h-a3h); PortB IN.
  Regular Inputs:

  7654 3210
  ---- ---x  * STAND / TAKE button.
  ---- --x-  * SAVE / HALF GAMBLE button.
  ---- -x--  * HOLD 1 button.
  ---- x---  * HOLD 2 button.
  ---x ----  * HOLD 3 button.
  --x- ----  * HOLD 4 button.
  -x-- ----  * HOLD 5 button.
  x--- ----  * CANCEL button.


  PPI-1 (a0h-a3h); PortC OUT.
  Sound & Lamps:

  7654 3210
  ---- ---x  * DEAL / DRAW Lamp.
  ---- --x-  * BET / COLLECT Lamp.
  ---- -x--  unknown (always activated after initalize).
  ---- x---  unknown.
  xxxx ----  * Discrete Sound Lines.


-----------------------------------------------------------

  PPI-2 (a0h-a3h); PortA IN/OUT
  VRAM Handlers:

  7654 3210
  xxxx xxxx  VRAM DATA.


  PPI-2 (a0h-a3h); PortB IN? (should be OUT)
  VRAM Handlers:

  7654 3210
  xxxx xxxx  VRAM ADDRESSING.


  PPI-2 (a0h-a3h); PortC (PortA handshake lines)
  VRAM Handlers:

  7654 3210
  xxxx xxxx  HANDSHAKE LINES.


*******************************************************************************


  *** Game Notes ***


  - norautjp:
  
	At the first start-up, the game will give you a very clever
    "FU" screen. Press the following buttons *together* on different times
    to get rid of it (and actually initialize the machine):

    * start + bet buttons (1+2);
    * Hold 3 + Hold 2 + Save (Half Gamble) + Change Card (C+X+F+D)

    Also notice that you actually need to map the last four buttons on the
    same button / on a joypad since MAME's steady key doesn't seem to work on
    my end...


*******************************************************************************

  --------------------
  ***  Memory Map  ***
  --------------------

  0x0000 - 0x1FFF    ; ROM space.
  0x6000 - 0x63FF    ; NVRAM.

  0x60 - 0x63        ; PPI 8255 0 - DIP Switches, lamps & counters.
  0xA0 - 0xA3        ; PPI 8255 1 - Regular Inputs, sound lines & remaining lamps.
  0xC0 - 0xC3        ; PPI 8255 2 - Video RAM access.


*******************************************************************************


  DRIVER UPDATES:


  [2009-01-27]

  - Initial release.
  - Defined ROM, RAM.
  - Added 2x PPI 8255 for regular I/O.
  - Added complete inputs and hooked DIP switches.
  - Added video RAM support.
  - Added NVRAM.
  - Added lamps support.
  - Added coin counters.
  - Identified the sound writes.
  - Added hardware description.
  - Added pinout scheme.
  - Added technical notes.


  [2009-01-28]

  - Merged GTI Poker (gtipoker.c) with this driver.
  - Added new memory map and machine driver for gtipoker.
  - Hooked 2x PPI 8255 to gtipoker.
  - Hooked the video RAM access ports to gtipoker.
  - Changed norautpn description from Noraut Poker (No Payout),
    to Noraut Poker (bootleg), since the game has payout system.
  - Some clean-ups.


  [2009-08-21]

  - Switched to pre-defined Xtal clock.
  - Changed the way how graphics are banked/accessed.
  - Fixed the graphics offset and number of tiles per bank.
  - Added new set: Noraut Red Hot Joker Poker.
  - Added new set: Noraut Poker (NTX10A).
  - Added new set: Noraut Joker Poker (V3.010a).
  - Fixed the tile size/decode for the first GFX bank.
  - Added proper norautrh inputs, including the readout button.
  - Added partial DIP switches to norautrh.
  - Added more technical notes.


  [2009-08-23/26]

  - Added a default NVRAM to Noraut Joker Poker to bypass the 'F U' screen.
    This is due to the phisical keyboard limitation when needs to enter
    4 simultaneous inputs.
  - Executed a trojan on 2 noraut systems to confirm the way 16x32 tiles are decoded.
  - Fixed the x-offset for 32x32 tiles lines.
  - Fixed the screen aspect and visible area.
  - Confirmed correct colors. No bipolar PROM involved.
  - Added Noraut Joker Poker hardware and PCB layouts.
  - Documented the discrete audio circuitery. Added a full diagram.


  [2009-08-29]

  - Fixed the coin counters.
  - Documented all the output ports.
  - Added a scheme with descriptions for every existent port.
  - Added full lamps support to naroutp, naroutjp, naroutrh and naroutpn.
  - Created lamps layouts for 11 and 12-lamps scheme.
  - Rerouted some inputs to mantain the inputs layout.
  - Renamed some inputs to match the text with the real cab buttons.
  - Removed the imperfect colors flag from the existent sets.
  - Added 2 different control panel layouts to the source.
  - Updated technical notes.


  TODO:

  - Analize the third 8255 at 0xc0-0xc3 (full bidirectional port w/hshk lines)
  - Video RAM (through 3rd PPI?).
  - Find if wide chars are hardcoded or tied to a bit.
  - Discrete sound.
  - Save support.


*******************************************************************************/


#define MASTER_CLOCK	XTAL_18_432MHz

#include "driver.h"
#include "cpu/z80/z80.h"
#include "machine/8255ppi.h"
#include "sound/dac.h"

#include "noraut11.lh"
#include "noraut12.lh"

static UINT16 *np_vram;
static UINT16 np_addr;


/*************************
*     Video Hardware     *
*************************/

static VIDEO_START( norautp )
{
	np_vram = auto_alloc_array(machine, UINT16, 0x1000/2);
}


static VIDEO_UPDATE( norautp )
{
	int x, y, count;

	count = 0;

	bitmap_fill(bitmap, cliprect, screen->machine->pens[0]); //black pen

	for(y = 0; y < 8; y++)
	{
		/* Double width, displaced 8 pixels in X */
		if(y == 2 || (y >= 4 && y < 6))
		{
			for(x = 0; x < 16; x++)
			{
				int tile = np_vram[count] & 0x3f;
				int colour = (np_vram[count] & 0xc0) >> 6;

				drawgfx_opaque(bitmap,cliprect, screen->machine->gfx[1], tile, colour, 0, 0, (x * 32) + 8, y * 32);

				count+=2;
			}
		}
		else
		{
			for(x = 0; x < 32; x++)
			{
				int tile = np_vram[count] & 0x3f;
				int colour = (np_vram[count] & 0xc0) >> 6;

				drawgfx_opaque(bitmap,cliprect, screen->machine->gfx[0], tile, colour, 0, 0, x * 16, y * 32);

				count++;
			}
		}
	}

	return 0;
}


static PALETTE_INIT( norautp )
{
	/* 1st gfx bank */
	palette_set_color(machine, 0, MAKE_RGB(0x00, 0x00, 0xff));	/* blue */
	palette_set_color(machine, 1, MAKE_RGB(0xff, 0xff, 0x00)); 	/* yellow */
	palette_set_color(machine, 2, MAKE_RGB(0x00, 0x00, 0xff));	/* blue */
	palette_set_color(machine, 3, MAKE_RGB(0xff, 0xff, 0xff));	/* white */
	palette_set_color(machine, 4, MAKE_RGB(0xff, 0xff, 0xff));	/* white */
	palette_set_color(machine, 5, MAKE_RGB(0xff, 0x00, 0x00));	/* red */
	palette_set_color(machine, 6, MAKE_RGB(0xff, 0xff, 0xff));	/* white */
	palette_set_color(machine, 7, MAKE_RGB(0x00, 0x00, 0x00));	/* black */
}


/*************************
*      R/W Handlers      *
*************************/

static WRITE8_DEVICE_HANDLER( mainlamps_w )
{
/*  PPI-0 (60h-63h); PortB OUT.
    Lamps:

    7654 3210
    ---- ---x  * CHANGE CARD lamp.
    ---- --x-  * SAVE / HALF GAMBLE lamp.
    ---- -x--  * HOLD 1 lamp.
    ---- x---  * HOLD 2 lamp.
    ---x ----  * HOLD 3 lamp.
    --x- ----  * HOLD 4 lamp.
    -x-- ----  * HOLD 5 lamp.
    x--- ----  * CANCEL lamp.
*/
	output_set_lamp_value(0, (data >> 0) & 1);	/* CHANGE CARD lamp */
	output_set_lamp_value(1, (data >> 1) & 1);	/* SAVE / HALF GAMBLE lamp */
	output_set_lamp_value(2, (data >> 2) & 1);	/* HOLD 1 lamp */
	output_set_lamp_value(3, (data >> 3) & 1);	/* HOLD 2 lamp */
	output_set_lamp_value(4, (data >> 4) & 1);	/* HOLD 3 lamp */
	output_set_lamp_value(5, (data >> 5) & 1);	/* HOLD 4 lamp */
	output_set_lamp_value(6, (data >> 6) & 1);	/* HOLD 5 lamp */
	output_set_lamp_value(7, (data >> 7) & 1);	/* CANCEL lamp */
}

static WRITE8_DEVICE_HANDLER( soundlamps_w )
{
/*  PPI-1 (a0h-a3h); PortC OUT.
    Sound & Lamps:

    7654 3210
    ---- ---x  * DEAL / DRAW Lamp.
    ---- --x-  * BET / COLLECT Lamp.
    ---- -x--  unknown (always activated after initalize).
    ---- x---  unknown.
    xxxx ----  * Discrete Sound Lines.
*/
	output_set_lamp_value(8, (data >> 0) & 1);	/* DEAL / DRAW lamp */
	output_set_lamp_value(9, (data >> 1) & 1);	/* BET / COLLECT lamp */

	/* the 4 MSB are for discrete sound */
	dac_data_w(devtag_get_device(device->machine, "dac"), (data & 0xf0));	/* Discrete Sound */
}

static WRITE8_DEVICE_HANDLER( counterlamps_w )
{
/*  PPI-0 (60h-63h); PortC OUT.
    Lamps & Coin Counters:

    7654 3210
    ---- ---x  * HI lamp.
    ---- --x-  * LO lamp.
    ---- -x--  unknown.
    ---- x---  * Payout pulse.
    ---x ----  * Coin 2 counter.
    --x- ----  * Coin 1 counter.
    -x-- ----  unknown.
    x--- ----  unknown (always activated).
*/
	output_set_lamp_value(10, (data >> 0) & 1);	/* HI lamp */
	output_set_lamp_value(11, (data >> 1) & 1);	/* LO lamp */

	coin_counter_w(0, data & 0x10);	/* Coin1/3 counter */
	coin_counter_w(1, data & 0x20);	/* Coin2 counter */
	coin_counter_w(2, data & 0x08);	/* Payout pulse */
}


/* game waits for bit 7 (0x80) to be set.*/
static READ8_HANDLER( test_r )
{
	return 0xff;
}

/* game waits for bit 4 (0x10) to be reset.*/
static READ8_HANDLER( test2_r )
{
	return 0x00;
}

static WRITE8_HANDLER( vram_data_w )
{
	np_vram[np_addr] = data & 0xff;
}

static WRITE8_HANDLER( vram_addr_w )
{
	np_addr = data;
}


/*************************
* Memory Map Information *
*************************/

static ADDRESS_MAP_START( norautp_map, ADDRESS_SPACE_PROGRAM, 8 )
	AM_RANGE(0x0000, 0x1fff) AM_ROM
	AM_RANGE(0x6000, 0x63ff) AM_RAM AM_BASE(&generic_nvram) AM_SIZE(&generic_nvram_size)
ADDRESS_MAP_END

static ADDRESS_MAP_START( norautp_portmap, ADDRESS_SPACE_IO, 8 )
	ADDRESS_MAP_GLOBAL_MASK(0xff)
	AM_RANGE(0x60, 0x63) AM_DEVREADWRITE("ppi8255_0", ppi8255_r, ppi8255_w)
	AM_RANGE(0xa0, 0xa3) AM_DEVREADWRITE("ppi8255_1", ppi8255_r, ppi8255_w)
	AM_RANGE(0xc0, 0xc0) AM_WRITE(vram_data_w)
	AM_RANGE(0xc1, 0xc1) AM_WRITE(vram_addr_w)
	AM_RANGE(0xc2, 0xc2) AM_READ(test_r)
//  AM_RANGE(0xc0, 0xc3) AM_DEVREADWRITE("ppi8255_2", ppi8255_r, ppi8255_w)
ADDRESS_MAP_END

/*
  Video RAM R/W:

  c0 --> W  ; data (in case of PPI, port data)
  c1 --> W  ; addressing
  c2 --> R  ; status?
  c3 --> W  ; alternate 00 & 01 (in case of PPI, setting resetting bit 0 of handshaked port)

  The strange issue is that some sets just configure the PPI to mixed mode2 and mode0 input.
  it means that port A should be bidirectional and port B just as input. (port C as hshk regs).
  So... it doesn't match the addressing through c1.
*/

static ADDRESS_MAP_START( gtipoker_map, ADDRESS_SPACE_PROGRAM, 8 )
	AM_RANGE(0x0000, 0x1fff) AM_ROM
	AM_RANGE(0xd000, 0xd3ff) AM_RAM AM_BASE(&generic_nvram) AM_SIZE(&generic_nvram_size)
ADDRESS_MAP_END

static ADDRESS_MAP_START( gtipoker_portmap, ADDRESS_SPACE_IO, 8 )
	ADDRESS_MAP_GLOBAL_MASK(0xff)
	AM_RANGE(0x7c, 0x7f) AM_DEVREADWRITE("ppi8255_0", ppi8255_r, ppi8255_w)
	AM_RANGE(0xbc, 0xbf) AM_DEVREADWRITE("ppi8255_1", ppi8255_r, ppi8255_w)
	AM_RANGE(0xdc, 0xdc) AM_WRITE(vram_data_w)
	AM_RANGE(0xdd, 0xdd) AM_WRITE(vram_addr_w)
	AM_RANGE(0xde, 0xde) AM_READ(test_r)
//  AM_RANGE(0xdc, 0xdf) AM_DEVREADWRITE("ppi8255_2", ppi8255_r, ppi8255_w)
	AM_RANGE(0xef, 0xef) AM_READ(test2_r)
ADDRESS_MAP_END


/*************************
*      Input Ports       *
*************************/

static INPUT_PORTS_START( norautp )
	PORT_START("IN0")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_GAMBLE_DEAL ) PORT_NAME("Deal / Draw")
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_POKER_BET )   PORT_NAME("Bet / Collect")
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_COIN1 ) PORT_IMPULSE(2)	/* Coin A */
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_COIN2 ) PORT_IMPULSE(2)	/* Coin B */
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_SERVICE )     PORT_NAME("Readout") PORT_CODE(KEYCODE_9)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_GAMBLE_HIGH ) PORT_NAME("Hi")
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_GAMBLE_LOW )  PORT_NAME("Lo")
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_GAMBLE_PAYOUT )

	PORT_START("IN1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_BUTTON1 )     PORT_CODE(KEYCODE_F) PORT_NAME("Change Card")
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_GAMBLE_HALF ) PORT_NAME("Half Gamble / Save")
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_POKER_HOLD1 )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_POKER_HOLD2 )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_POKER_HOLD3 )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_POKER_HOLD4 )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_POKER_HOLD5 )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_COIN3 ) PORT_IMPULSE(2)	/* Coin C */

	PORT_START("DSW1")
	PORT_DIPNAME( 0x01, 0x01, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x02, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x02, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x04, 0x04, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x04, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x08, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x08, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x10, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x20, DEF_STR( Coinage ) )
	PORT_DIPSETTING(    0x20, "A=5; B=25; C=1" )
	PORT_DIPSETTING(    0x00, "A=50; B=25; C=5" )
	PORT_DIPNAME( 0x40, 0x40, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, "Set Value" )
	PORT_DIPSETTING(    0x80, "2 Pence" )
	PORT_DIPSETTING(    0x00, "10 Pence" )
INPUT_PORTS_END

static INPUT_PORTS_START( norautrh )
	PORT_START("IN0")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_GAMBLE_DEAL ) PORT_NAME("Deal / Draw")
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_POKER_BET )   PORT_NAME("Bet / Change Card")
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_COIN1 ) PORT_IMPULSE(2)	/* Coin A */
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_COIN2 ) PORT_IMPULSE(2)	/* Coin B */
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_SERVICE )     PORT_NAME("Readout") PORT_CODE(KEYCODE_9)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_GAMBLE_HIGH ) PORT_NAME("Hi")
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_GAMBLE_LOW )  PORT_NAME("Lo")
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_GAMBLE_PAYOUT )

	PORT_START("IN1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_GAMBLE_TAKE ) PORT_NAME("Stand / Take")
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_GAMBLE_HALF ) PORT_NAME("Save / Half Gamble")
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_POKER_HOLD1 )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_POKER_HOLD2 )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_POKER_HOLD3 )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_POKER_HOLD4 )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_POKER_HOLD5 )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_POKER_CANCEL )	/* Coin C for other games */

	PORT_START("DSW1")
	PORT_DIPNAME( 0x01, 0x01, DEF_STR( Unknown ) )	PORT_DIPLOCATION("DSW1:8")
	PORT_DIPSETTING(    0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x02, DEF_STR( Unknown ) )	PORT_DIPLOCATION("DSW1:7")
	PORT_DIPSETTING(    0x02, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x04, 0x04, "Bet Max" )			PORT_DIPLOCATION("DSW1:6")
	PORT_DIPSETTING(    0x04, "5" )
	PORT_DIPSETTING(    0x00, "25" )
	PORT_DIPNAME( 0x08, 0x08, "Raise Ante" )		PORT_DIPLOCATION("DSW1:5")
	PORT_DIPSETTING(    0x08, "Random" )
	PORT_DIPSETTING(    0x00, "Always" )
	PORT_DIPNAME( 0x10, 0x10, "Type of Game" )		PORT_DIPLOCATION("DSW1:4")
	PORT_DIPSETTING(    0x10, "Jacks Plus" )
	PORT_DIPSETTING(    0x00, "Joker Poker" )
	PORT_DIPNAME( 0xa0, 0x20, DEF_STR( Coinage ) )	PORT_DIPLOCATION("DSW1:3,1")
	PORT_DIPSETTING(    0x00, "A=1; B=5" )
	PORT_DIPSETTING(    0xa0, "A=5; B=25" )
	PORT_DIPSETTING(    0x20, "A=10; B=5" )
	PORT_DIPSETTING(    0x80, "A=50; B=25" )
	PORT_DIPNAME( 0x40, 0x00, "Show Bet")			PORT_DIPLOCATION("DSW1:2")
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
INPUT_PORTS_END

static INPUT_PORTS_START( naroutpn )
	PORT_START("IN0")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_GAMBLE_DEAL ) PORT_NAME("Deal / Start")
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_POKER_BET )   PORT_NAME("Bet / Change Card")
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_COIN1 ) PORT_IMPULSE(2)	/* Coin A */
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_COIN2 ) PORT_IMPULSE(2)	/* Coin B */
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_SERVICE )     PORT_NAME("Readout") PORT_CODE(KEYCODE_9)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_GAMBLE_HIGH ) PORT_NAME("Hi")
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_GAMBLE_LOW )  PORT_NAME("Lo")
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_GAMBLE_PAYOUT )

	PORT_START("IN1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_GAMBLE_TAKE ) PORT_NAME("Stand / Take")
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_GAMBLE_HALF ) PORT_NAME("Save / Half Gamble")
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_POKER_HOLD1 )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_POKER_HOLD2 )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_POKER_HOLD3 )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_POKER_HOLD4 )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_POKER_HOLD5 )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_POKER_CANCEL )

	PORT_START("DSW1")
	PORT_DIPNAME( 0x01, 0x01, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x02, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x02, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x04, 0x04, "Bet Max" )
	PORT_DIPSETTING(    0x04, "1" )
	PORT_DIPSETTING(    0x00, "5" )
	PORT_DIPNAME( 0x08, 0x08, DEF_STR( Coinage ) )
	PORT_DIPSETTING(    0x08, "A=1; B=5" )
	PORT_DIPSETTING(    0x00, "A=50; B=5" )
	PORT_DIPNAME( 0x10, 0x10, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x20, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x40, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
INPUT_PORTS_END


/*************************
*    Graphics Layouts    *
*************************/

static const gfx_layout charlayout =
/*
  Trojanned 2 Narout Poker PCBs to see how the hardware decodes
  the 16x32 tiles. The following GFX layout is 100% accurate.
*/
{
	16, 32,
	RGN_FRAC(1,2),
	1,
	{ 0 },
	{ 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15 },
	{ 0*16, 0*16, 0*16, 0*16, 0*16, 0*16, 0*16, 0*16, 0*16, 1*16, 2*16, 3*16, 4*16, 5*16, 6*16, 7*16,
	  8*16, 9*16, 10*16, 11*16, 12*16, 13*16, 14*16, 15*16, 0*16, 0*16, 0*16, 0*16, 0*16, 0*16, 0*16, 0*16, },
	16*16
};

static const gfx_layout charlayout32x32 =
{
	32,32,
	RGN_FRAC(1,2),
	1,
	{ 0 },
	{ 0,0, 1,1, 2,2, 3,3, 4,4, 5,5, 6,6, 7,7, 8,8, 9,9, 10,10, 11,11, 12,12, 13,13, 14,14, 15,15 },
	{ 0*16, 0*16, 1*16, 1*16, 2*16, 2*16, 3*16, 3*16, 4*16, 4*16, 5*16, 5*16, 6*16, 6*16, 7*16, 7*16,
	  8*16, 8*16, 9*16, 9*16, 10*16,10*16,11*16,11*16,12*16,12*16,13*16,13*16,14*16,14*16,15*16,15*16 },
	16*16
};


/******************************
* Graphics Decode Information *
******************************/

/* GFX are stored in the 2nd half... Maybe the HW could handle 2 bitplanes? */
static GFXDECODE_START( norautp )
	GFXDECODE_ENTRY( "gfx", 0x800, charlayout,      0, 4 )
	GFXDECODE_ENTRY( "gfx", 0x800, charlayout32x32, 0, 4 )
GFXDECODE_END


/************************************
*      PPI 8255 (x3) Interface      *
************************************/

static const ppi8255_interface ppi8255_intf[3] =
{
	{	/* (60-63) Mode 0 - Port A set as input */
		DEVCB_INPUT_PORT("DSW1"),		/* Port A read */
		DEVCB_NULL,						/* Port B read */
		DEVCB_NULL,						/* Port C read */
		DEVCB_NULL,						/* Port A write */
		DEVCB_HANDLER(mainlamps_w),		/* Port B write */
		DEVCB_HANDLER(counterlamps_w)	/* Port C write */
	},
	{	/* (a0-a3) Mode 0 - Ports A & B set as input */
		DEVCB_INPUT_PORT("IN0"),		/* Port A read */
		DEVCB_INPUT_PORT("IN1"),		/* Port B read */
		DEVCB_NULL,						/* Port C read */
		DEVCB_NULL,						/* Port A write */
		DEVCB_NULL,						/* Port B write */
		DEVCB_HANDLER(soundlamps_w)		/* Port C write */
	},
	{	/* (c0-c3) Group A Mode 2 (5-handshacked bidirectional port) */
		DEVCB_NULL,						/* Port A read */
		DEVCB_NULL,						/* Port B read */
		DEVCB_NULL,						/* Port C read  (should has test_r tied) */
		DEVCB_NULL,						/* Port A write (should has vram_data_w tied) */
		DEVCB_NULL,						/* Port B write (should has vram_addr_w tied) */
		DEVCB_NULL						/* Port C write */
	}
};


/*************************
*    Machine Drivers     *
*************************/

static MACHINE_DRIVER_START( norautp )

	/* basic machine hardware */
	MDRV_CPU_ADD("maincpu", Z80, MASTER_CLOCK/6)	/* guess */
	MDRV_CPU_PROGRAM_MAP(norautp_map)
	MDRV_CPU_IO_MAP(norautp_portmap)
	MDRV_CPU_VBLANK_INT("screen", irq0_line_hold)

	MDRV_NVRAM_HANDLER(generic_0fill)

	/* 3x 8255 */
	MDRV_PPI8255_ADD( "ppi8255_0", ppi8255_intf[0] )
	MDRV_PPI8255_ADD( "ppi8255_1", ppi8255_intf[1] )
	MDRV_PPI8255_ADD( "ppi8255_2", ppi8255_intf[2] )

	/* video hardware */
	MDRV_SCREEN_ADD("screen", RASTER)
	MDRV_SCREEN_REFRESH_RATE(60)
	MDRV_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(0))
	MDRV_SCREEN_FORMAT(BITMAP_FORMAT_INDEXED16)
	MDRV_SCREEN_SIZE(32*16, 32*16)
	MDRV_SCREEN_VISIBLE_AREA(3*16, 31*16-1, (0*16) + 8, 16*16-1)	/* the hardware clips the top 8 pixels */

	MDRV_GFXDECODE(norautp)

	MDRV_PALETTE_INIT(norautp)
	MDRV_PALETTE_LENGTH(8)
	MDRV_VIDEO_START(norautp)
	MDRV_VIDEO_UPDATE(norautp)

	/* sound hardware */
	MDRV_SPEAKER_STANDARD_MONO("mono")
	MDRV_SOUND_ADD("dac", DAC, 0)
	MDRV_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.50)
MACHINE_DRIVER_END

static MACHINE_DRIVER_START( gtipoker )
	MDRV_IMPORT_FROM(norautp)

	MDRV_CPU_MODIFY("maincpu")
	MDRV_CPU_PROGRAM_MAP(gtipoker_map)
	MDRV_CPU_IO_MAP(gtipoker_portmap)

MACHINE_DRIVER_END


/*************************
*        Rom Load        *
*************************/

ROM_START( norautp )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "jpoker.bin",	    0x0000,  0x2000,  CRC(e22ed34d) SHA1(108f034335b5bed183ee316a61880f7b9485b34f) )

	ROM_REGION( 0x10000, "gfx", 0 )
	ROM_LOAD( "displayrom.bin",	0x00000, 0x10000, CRC(ed3605bd) SHA1(0174e880835815558328789226234e36b673b249) )
ROM_END

/* Has (c)1983 GTI in the roms, and was called 'Poker.zip'  GFX roms contain 16x16 tiles of cards */
/* Nothing else is known about this set / game */

ROM_START( gtipoker )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "u12.rom", 0x0000, 0x1000, CRC(abaa257a) SHA1(f830213ae0aaad5a9a44ec77c5a186e9e02fa041) )
	ROM_LOAD( "u18.rom", 0x1000, 0x1000, CRC(1b7e2877) SHA1(717fb70889804baa468203f20b1e7f73b55cc21e) )

	ROM_REGION( 0x1000, "gfx",0 )
	ROM_LOAD( "u31.rom", 0x0000, 0x1000, CRC(2028db2c) SHA1(0f81bb71e88c60df3817f58c28715ce2ea01ad4d) )
ROM_END

/*
Poker game - manufacturer unknown

Z80 CPU

Program rom = 2764 (2nd Half blank)
Character rom = 2732

18.432 Mhz crystal

sound probably discrete with ne555 timer chip (located near amp/volume control)
*/

ROM_START( norautpn )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "prog.bin",   0x0000, 0x2000, CRC(8b1cfd24) SHA1(d673baed1c1e5b54a34b7a5857b269a725737e92) )

	ROM_REGION( 0x1000,  "gfx", 0 )
	ROM_LOAD( "char.bin",   0x0000, 0x1000, CRC(955eac6f) SHA1(470d8bad1a5d2a0a08dd129e6393c3c3a4ef2159) )
ROM_END

/*
Noraut Joker Poker

Program:27C64
Marked:
"MX10A JOKER G.L
 N.C.R  C.C  M.S"

Char:2732
Marked:
"N1-057-5"

CPU: TMPZ84C00AP-8
*/

ROM_START( norautjp )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "2764-1prog.bin",   0x0000, 0x2000, CRC(5f776ce1) SHA1(673b8c67ebd5c1334187a9407b86a43150cbe67b) )

	ROM_REGION( 0x1000,	"gfx", 0 )
	ROM_FILL(                     0x0000, 0x0800, 0xff )
	ROM_LOAD( "2732-1char.bin",   0x0800, 0x0800, CRC(d94be899) SHA1(b7212162324fa2d67383a475052e3b351bb1af5f) ) 	/* first half 0xff filled */
	ROM_CONTINUE(                 0x0800, 0x0800 )

	ROM_REGION( 0x400,	"nvram", 0 )
	ROM_LOAD( "norautjp_nv.bin",  0x0000, 0x0400, CRC(0a0614b2) SHA1(eb21b2723b41743daf787cfc379bc67cce2b8538) )	/* default NVRAM */

ROM_END

/*
Noraut Red Hot Joker Poker
Red hot joker poker scrolls across screen
and eprom has Red Hot on sticker
Char:
Handwritten sticker with "Club250 grapics" on it

Pressing the readout button brings you to a menu with RESET / READOUT
pressing on Readout brings you to "coins in" and "coins out" and "balance".

No date info on board or found in rom
*/

ROM_START( norautrh )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "norautrh.bin",  0x0000, 0x2000, CRC(f5447d1a) SHA1(75d6439481e469e82e5561146966c9c7b44f34fe) )

	ROM_REGION( 0x1000,  "gfx", 0 )
	ROM_LOAD( "club250.bin",   0x0000, 0x1000, CRC(d94be899) SHA1(b7212162324fa2d67383a475052e3b351bb1af5f) )
ROM_END

/*
Unknown Noraut: "NTX10A  V6"
None working old noraut board with daughter card upgrade
daughter card looks like an old upgrade PCB marked:
"Noraut LTD Game Module"
half of which is incased in epoxy resin.
only thing not visble on this board compared to others i have is the cpu
with is under the epoxy not sure what else is their.

D Card contains:
Backup Battery

Program Eprom:27C256
Marked: "NTX10A  V6"
           "C201"

CPU:
Unknown Incased in epoxy

NVRAM: HY6264A

PAL:
PAL16L8ACN

Charcter Eprom is mounted on main board
CHAR Eprom:2732
Marked: "GU27"

daughter card is connected on to another card containing only pcb tracks no components
This second board connects to main board with ribbon cable to the 40pin socket where
the original cpu would of been.

No date info on board or found in rom.
*/

ROM_START( norautu )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "2563.bin",    0x0000, 0x8000, CRC(6cbe68bd) SHA1(93201baaf03a9bba6c52c64cc26e8e445aa6454e) )

	ROM_REGION( 0x1000,  "gfx", 0 )
	ROM_LOAD( "club250.bin", 0x0000, 0x1000, CRC(d94be899) SHA1(b7212162324fa2d67383a475052e3b351bb1af5f) )
ROM_END


/*NORAUT V3.010a:
Board upgraded with daughter card.
daughter card looks modern and is marked
"memory expansion module"
"Unique Ireland"

D Card contains:
Backup Battery

Program Eprom:27C512
Marked:
"G45P A V3.010a GU27
 Euro 27C512 20MAR02"


PAL:PAL16l8ANC
Marked VER.2

CPU:
Zilog
Z8400APS
Z80 CPU

NVRAM: 6116

Two jumpers on card , game will not boot if these are removed or placed on other pins
cabinet beeps and shows grapics on screen. Removing while game is on cause game to freeze.
Unknown what their for.

Charcter Eprom is mounted on main board
CHAR Eprom:2716
Marked "GU27"

No date info found in rom,  program eprom sticker "Euro 27C512 20MAR02"
This version contains a hidden menu with lots of differnt options
to access this menu you must hold the HI and LOW button and press the readout/test switch
the screen will go blank then you release the 3 buttons and the menu appears.

Pressing the readout button brings you to a menu with RESET / READOUT
pressing on Readout brings you to "coins in" and "coins out" and "balance".

The daughter card connects direct to main pcb through 40 pins into original cpu socket
and 12 pins to one side of original program eprom.
*/

ROM_START( norautv3 )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "g45pa.bin", 0x0000, 0x10000, CRC(f966f4d2) SHA1(99c21ceb59664f32fd1269351fa976370d486f2e) )

	ROM_REGION( 0x1000,  "gfx", 0 )
	ROM_FILL(              0x0000, 0x0800, 0xff )
	ROM_LOAD( "gu27.bin",  0x0800, 0x0800, CRC(174a5eec) SHA1(44d84a0cf29a0bf99674d95084c905d3bb0445ad) )
ROM_END


/*************************
*      Game Drivers      *
*************************/

/*     YEAR  NAME      PARENT   MACHINE   INPUT     INIT  ROT    COMPANY        FULLNAME                       FLAGS                              LAYOUT */
GAMEL( 1988, norautp,  0,       norautp,  norautp,  0,    ROT0, "Noraut Ltd.", "Noraut Poker",                 GAME_NO_SOUND,                     layout_noraut11 )
GAMEL( 1988, norautjp, norautp, norautp,  norautp,  0,    ROT0, "Noraut Ltd.", "Noraut Joker Poker",           GAME_NO_SOUND,                     layout_noraut11 )
GAMEL( 1988, norautrh, 0,       norautp,  norautrh, 0,    ROT0, "Noraut Ltd.", "Noraut Red Hot Joker Poker",   GAME_NO_SOUND,                     layout_noraut12 )
GAME(  1988, norautu,  0,       norautp,  norautp,  0,    ROT0, "Noraut Ltd.", "Noraut Poker (NTX10A)",        GAME_NO_SOUND | GAME_NOT_WORKING )
GAME(  1988, norautv3, 0,       norautp,  norautp,  0,    ROT0, "Noraut Ltd.", "Noraut Joker Poker (V3.010a)", GAME_NO_SOUND | GAME_NOT_WORKING )
GAME(  1983, gtipoker, 0,       gtipoker, norautp,  0,    ROT0, "GTI Inc",     "GTI Poker",                    GAME_NO_SOUND | GAME_NOT_WORKING )

/*The following has everything uncertain, seems a bootleg/hack and doesn't have any identification strings in program rom. */
GAMEL( 198?, norautpn, norautp, norautp,  naroutpn, 0,    ROT0, "bootleg?",    "Noraut Poker (bootleg)",       GAME_NO_SOUND,                     layout_noraut12 )
