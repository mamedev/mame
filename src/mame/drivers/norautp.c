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


    Hardware Notes:
    ---------------

    1x Z80
    3x PPI 8255
    2x 6116 SRAM
    1x 3.6 Vcc Battery.
    1x 18.432 MHz. Xtal.

    1x 555 + unknown yellow resonator, near the edge connector.
    1x 555 + resnet, near the battery.

    2x 3pins jumpers (between the Z80 and ROM)

       JP1 (ABC);  JP2 (DEF)

    PCB silksceened:  AB+DE=512  BC+DE=256
                      (CUT BC)   EF=64/128


    1x 10 DIP switches bank.


    PCB silksceened:  SMART-BOARD 131191 ISS.E (Made in USA)


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


    *** Game Notes ***

    Nothing, yet...


*******************************************************************************

    --------------------
    ***  Memory Map  ***
    --------------------

    0x0000 - 0x1FFF    ; ROM space.
    0x6000 - 0x63FF    ; NVRAM.

    0x60 - 0x63        ; PPI 8255 0 - DIP Switches.
    0xA0 - 0xA3        ; PPI 8255 1 - Regular Inputs.
    0xC0 - 0xC3        ; PPI 8255 2 - Video RAM.


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


    Notes:
    - norautjp: at the first start-up, the game will give you a very clever
      "FU" screen. Press the following buttons *together* on different times
      to get rid of it (and actually initialize the machine):
      * start + bet buttons (1+2);
      * Hold 3 + Hold 2 + Save (Half Gamble) + Change Card (C+X+F+D)
      Also notice that you actually need to map the last four buttons on the
      same button / on a joypad since MAME's steady key doesn't seem to work on
      my end...


    TODO:

    - Analize the extra 8255 at 0xc0-0xc3 (full bidirectional port w/hshk)
    - Video RAM (through 3rd PPI?).
    - Find if wide chars are hardcoded or tied to a bit.
    - Fix the little displacement between GFX banks. 
    - Proper colors (missing PROM?)
    - Lamps layout.
    - Discrete sound.


*******************************************************************************/


#define MASTER_CLOCK	XTAL_18_432MHz

#include "driver.h"
#include "cpu/z80/z80.h"
#include "machine/8255ppi.h"
#include "sound/dac.h"

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
		/*Double width*/
		if(y == 2 || (y >= 4 && y < 6))
		{
			for(x = 0; x < 16; x++)
			{
				int tile = np_vram[count] & 0x3f;
				int colour = (np_vram[count] & 0xc0) >> 6;

				drawgfx_opaque(bitmap,cliprect, screen->machine->gfx[1], tile, colour, 0, 0, x * 32, y * 32);

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

static WRITE8_DEVICE_HANDLER( lamps_w )
{
/*  LAMPS:

    7654 3210
    ---- ---x  Change Card / (Save?)
    ---- --x-  Hi/Lo
    ---- -x--  Hold 1
    ---- x---  Hold 2
    ---x ----  Hold 3
    --x- ----  Hold 4
    -x-- ----  Hold 5
    x--- ----  Start (poker)
*/

	output_set_lamp_value(0, (data >> 0) & 1);	/* Change */
	output_set_lamp_value(1, (data >> 1) & 1);	/* Hi/Lo  */
	output_set_lamp_value(2, (data >> 2) & 1);	/* Hold 1 */
	output_set_lamp_value(3, (data >> 3) & 1);	/* Hold 2 */
	output_set_lamp_value(4, (data >> 4) & 1);	/* Hold 3 */
	output_set_lamp_value(5, (data >> 5) & 1);	/* Hold 4 */
	output_set_lamp_value(6, (data >> 6) & 1);	/* Hold 5 */
	output_set_lamp_value(7, (data >> 7) & 1);	/* Start  */
}

static WRITE8_DEVICE_HANDLER( ccounter_w )
{
	coin_counter_w(0, data & 0x20);	/* Coin1 */
	coin_counter_w(1, data & 0x10);	/* Coin2 */
	coin_counter_w(2, data & 0x08);	/* Payout */
}

static WRITE8_DEVICE_HANDLER( sndlamp_w )
{
	output_set_lamp_value(8, (data >> 0) & 1);	/* Start? */
	output_set_lamp_value(9, (data >> 1) & 1);	/* Bet */

	/* the 4 MSB are for discrete sound */
	dac_data_w(devtag_get_device(device->machine, "dac"), (data & 0xf0));	/* Discrete Sound */
}

/*game waits for bit 7 (0x80) to be set.*/
static READ8_HANDLER( test_r )
{
	return 0xff;
}

/*game waits for bit 4 (0x10) to be reset.*/
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
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_GAMBLE_DEAL ) PORT_NAME("Deal / Start")
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_POKER_BET )   PORT_NAME("Bet / Take")
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_COIN1 ) PORT_IMPULSE(2)	/* Coin A */
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_COIN2 ) PORT_IMPULSE(2)	/* Coin B */
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_SERVICE )     PORT_NAME("Readout") PORT_CODE(KEYCODE_9)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_GAMBLE_HIGH ) PORT_NAME("Hi")
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_GAMBLE_LOW )  PORT_NAME("Lo")
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_GAMBLE_PAYOUT )

	PORT_START("IN1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_BUTTON1 )     PORT_CODE(KEYCODE_F) PORT_NAME("Change Card")
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_GAMBLE_HALF ) PORT_NAME("Save (Half Gamble)")
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

static INPUT_PORTS_START( poker )
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
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_GAMBLE_TAKE ) PORT_NAME("Stand (Take)")
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_GAMBLE_HALF ) PORT_NAME("Save (Half Gamble)")
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

static INPUT_PORTS_START( norautrh )
	PORT_START("IN0")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_GAMBLE_DEAL ) PORT_NAME("Deal / Start")
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_POKER_BET )   PORT_NAME("Bet")
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_COIN1 ) PORT_IMPULSE(2)	/* Coin A */
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_COIN2 ) PORT_IMPULSE(2)	/* Coin B */
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_SERVICE )     PORT_NAME("Readout") PORT_CODE(KEYCODE_9)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_GAMBLE_HIGH ) PORT_NAME("Hi")
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_GAMBLE_LOW )  PORT_NAME("Lo")
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_GAMBLE_PAYOUT )

	PORT_START("IN1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_BUTTON1 )     PORT_CODE(KEYCODE_F) PORT_NAME("Change Card")
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_GAMBLE_HALF ) PORT_NAME("Save (Half Gamble)")
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_POKER_HOLD1 )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_POKER_HOLD2 )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_POKER_HOLD3 )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_POKER_HOLD4 )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_POKER_HOLD5 )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_POKER_CANCEL )	/* Coin C for other games */

	PORT_START("DSW1")
	PORT_DIPNAME( 0x01, 0x01, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x02, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x02, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x04, 0x04, "Bet Max" )
	PORT_DIPSETTING(    0x04, "5" )
	PORT_DIPSETTING(    0x00, "25" )
	PORT_DIPNAME( 0x08, 0x08, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x08, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x10, "Type of Game" )
	PORT_DIPSETTING(    0x10, "Jacks Plus" )
	PORT_DIPSETTING(    0x00, "Joker Poker" )
	PORT_DIPNAME( 0xa0, 0x20, DEF_STR( Coinage ) )
	PORT_DIPSETTING(    0x00, "A=1; B=5" )
	PORT_DIPSETTING(    0xa0, "A=5; B=25" )
	PORT_DIPSETTING(    0x20, "A=10; B=5" )
	PORT_DIPSETTING(    0x80, "A=50; B=25" )
	PORT_DIPNAME( 0x40, 0x00, "Show Bet")
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
INPUT_PORTS_END


/*************************
*    Graphics Layouts    *
*************************/

static const gfx_layout charlayout =
/*
  Not sure if this is the correct way to decode.
  Each tile should be 16x32 conformed by:
  - 8 empty lines.
  - 16x16 char.
  - 8 empty lines.

  Each tile is a regular 16x16 char centered into 16x32 tile.
  Only alphanumeric chars are used.
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
		DEVCB_INPUT_PORT("DSW1"),	/* Port A read */
		DEVCB_NULL,					/* Port B read */
		DEVCB_NULL,					/* Port C read */
		DEVCB_NULL,					/* Port A write */
		DEVCB_HANDLER(lamps_w),	    /* Port B write */
		DEVCB_HANDLER(ccounter_w)   /* Port C write */
	},
	{	/* (a0-a3) Mode 0 - Ports A & B set as input */
		DEVCB_INPUT_PORT("IN0"),    /* Port A read */
		DEVCB_INPUT_PORT("IN1"),	/* Port B read */
		DEVCB_NULL,				    /* Port C read */
		DEVCB_NULL,				    /* Port A write */
		DEVCB_NULL,				    /* Port B write */
		DEVCB_HANDLER(sndlamp_w)    /* Port C write */
	},
	{	/* (c0-c3) Group A Mode 2 (5-handshacked bidirectional port) */
		DEVCB_NULL,					/* Port A read */
		DEVCB_NULL,					/* Port B read */
		DEVCB_NULL,					/* Port C read  (should has test_r tied) */
		DEVCB_NULL,					/* Port A write (should has vram_data_w tied) */
		DEVCB_NULL,					/* Port B write (should has vram_addr_w tied) */
		DEVCB_NULL					/* Port C write */
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
	MDRV_SCREEN_VISIBLE_AREA(2*16, 31*16-1, 0*16, 16*16-1)

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

	ROM_REGION( 0x1000,  "gfx", 0 )
	ROM_FILL(                     0x0000, 0x0800, 0xff )
	ROM_LOAD( "2732-1char.bin",   0x0800, 0x0800, CRC(d94be899) SHA1(b7212162324fa2d67383a475052e3b351bb1af5f) ) 	/* first half 0xff filled */
	ROM_CONTINUE(                 0x0800, 0x0800 )
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

/*    YEAR  NAME      PARENT   MACHINE   INPUT     INIT  ROT    COMPANY        FULLNAME                       FLAGS */
GAME( 1988, norautp,  0,       norautp,  norautp,  0,    ROT0, "Noraut Ltd.", "Noraut Poker",                 GAME_NO_SOUND | GAME_IMPERFECT_COLORS )
GAME( 1988, norautjp, norautp, norautp,  norautp,  0,    ROT0, "Noraut Ltd.", "Noraut Joker Poker",           GAME_NO_SOUND | GAME_IMPERFECT_COLORS )
GAME( 1988, norautrh, 0,       norautp,  norautrh, 0,    ROT0, "Noraut Ltd.", "Noraut Red Hot Joker Poker",   GAME_NO_SOUND | GAME_IMPERFECT_COLORS )
GAME( 1988, norautu,  0,       norautp,  norautp,  0,    ROT0, "Noraut Ltd.", "Noraut Poker (NTX10A)",        GAME_NO_SOUND | GAME_IMPERFECT_COLORS | GAME_NOT_WORKING )
GAME( 1988, norautv3, 0,       norautp,  norautp,  0,    ROT0, "Noraut Ltd.", "Noraut Joker Poker (V3.010a)", GAME_NO_SOUND | GAME_IMPERFECT_COLORS | GAME_NOT_WORKING )
GAME( 1983, gtipoker, 0,       gtipoker, norautp,  0,    ROT0, "GTI Inc",     "GTI Poker",                    GAME_NO_SOUND | GAME_IMPERFECT_COLORS | GAME_NOT_WORKING )

/*The following has everything uncertain, seems a bootleg/hack and doesn't have any identification strings in program rom. */
GAME( 198?, norautpn, norautp, norautp,  poker,    0,    ROT0, "bootleg?",    "Noraut Poker (bootleg)",       GAME_NO_SOUND | GAME_IMPERFECT_COLORS )
