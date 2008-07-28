/****************************************************************************************

    LUCKY 74 - WING CO.LTD.
    -----------------------

    Driver by Roberto Fresca.


    Games running on this hardware:

    * Lucky 74 (small),  1988,  Wing Co.Ltd.
    * Lucky 74 (big),    1988,  Wing Co.Ltd.


*****************************************************************************************


    Hardware Notes:
    ---------------

    CPU:  1x Big black box stickered 'WING CPU A001' (Z80 based).

    Co-Processor: 1x NPC SM7831

    Sound: 1x Yamaha YM-2149F.
           1x OKI M5205 (4-bit ADPCM samples @ 8kHz).

    ROMs: 1x 27C512 (program).
          1x 27C512 (sound samples).
          8x 27C256 (graphics).

    PROMs: 6x 24s10 (color system).


    Clock: 1x Xtal 12.000 MHz.


    Other: 2x M5M82C255ASP (2x 8255 PPI each).
           1x M5L8251AP-5  (8251 UART).
           1x 40-legs IC silkscreened '09R81P 7K4'.
           1x 28-legs IC silkscreened '06B49P 2G1'.
           2x 28-legs IC silkscreened '06B53P 9G3'.
           1x 3.6V lithium battery.
           4x 8 DIP switches banks.
           1x Reset switch.

    Connectors: 1x (2x36) edge connector.
                1x (2x10) edge connector.
                1x 6-pins connector.


    There are 2 extra marks on the black CPU box:

    Silkscreened: 'B 0L2'

    Sticker:      'WE8703 1992.10'
                    'LUCKY 74-7'


    PCB is original from WING Co.Ltd.


    The M5M82C255ASP (82c255) is a general purpose programmable I/O. It is compatible with the 8255
    programmable I/O. The 82c255 is equivalent to two 8255s. It has 48 I/O lines which may be indi-
    vidually programmed in 6 groups of 8 or 4 groups of 12 and used in 3 major modes of operation.


*****************************************************************************************


    Color Circuitry
    ---------------

    Here is a little diagram showing how the color PROMs are connected to a 74174
    and therefore to a resistor network that derives to the RGB connector.


                                  220
    (E6)24s10-12 -+- 74174-02 ---/\/\/\----+
    (E7)24s10-12 _|                        |
                                  470      |
    (E6)24s10-11 -+- 74174-10 ---/\/\/\----+
    (E7)24s10-11 _|                        |
                                   1K      |
    (E6)24s10-10 -+- 74174-12 ---/\/\/\----+
    (E7)24s10-10 _|                        |
                                   2K      |
    (E6)24s10-09 -+- 74174-15 ---/\/\/\----+---> Red
    (E7)24s10-09 _|                        |
                                           /
                                        1K \
                                           /
                                           |
                                           _

                                  220
    (D6)24s10-12 -+- 74174-02 ---/\/\/\----+
    (D7)24s10-12 _|                        |
                                  470      |
    (D6)24s10-11 -+- 74174-10 ---/\/\/\----+
    (D7)24s10-11 _|                        |
                                   1K      |
    (D6)24s10-10 -+- 74174-12 ---/\/\/\----+
    (D7)24s10-10 _|                        |
                                   2K      |
    (D6)24s10-09 -+- 74174-15 ---/\/\/\----+---> Green
    (D7)24s10-09 _|                        |
                                           /
                                        1K \
                                           /
                                           |
                                           _

                                  220
    (C6)24s10-12 -+- 74174-02 ---/\/\/\----+
    (C7)24s10-12 _|                        |
                                  470      |
    (C6)24s10-11 -+- 74174-10 ---/\/\/\----+
    (C7)24s10-11 _|                        |
                                   1K      |
    (C6)24s10-10 -+- 74174-12 ---/\/\/\----+
    (C7)24s10-10 _|                        |
                                   2K      |
    (C6)24s10-09 -+- 74174-15 ---/\/\/\----+---> Blue
    (C7)24s10-09 _|                        |
                                           /
                                        1K \
                                           /
                                           |
                                           _


    Regarding the abobe diagram, there are 2 different states controlled by some bit.
    Each state drives a different palette that will be assigned to each graphics bank.

    As we can see here, same pin of different PROMs are connected together in parallel.
    We need to check the pin 13 (/G1) and pin 14 (/G2) from each PROM to see if they are
    connected to the same signal, and futhermore check if one or the other is tied to GND.

    To reproduce the states, we need to create a double-sized palette and fill the first
    half with the values created through state 1, then fill the second half with proper
    values from state 2.


*****************************************************************************************


    *** Dev Notes ***


    - Sound seems to be triggered at 0xf100. Each event writes sequences
      like 0x84 0x07 0x99 0x9c 0x9f...

    - After z80 port 0x00 request, there are writes to port 0x00 to 0x05.



*****************************************************************************************


    *** Game Notes ***


    This game was one of the 'classics'.

    Lucky 74 is a strip poker game with anime theme. It has a nice double-up feature, and
    the objective is obviously to win hands till you can see the girl naked, like other
    strip poker games.


    To enter the test mode, press F2 and then reset. To exit, press F2 again and then reset.

    To enter the book-keeping mode, press BOOKS (key 0), and then press BOOKS again to
    change between pages. Press START (key 1) to exit the mode.



    - DIP Switch 3-7 change the pay table.

    Table 1 (per unit): 500 100 40 10 7 5 3 2 1
    Table 2 (per unit): 500 200 100 40 10 7 5 3 2


    If Bet Max (DSW3-1) is 20:

    Table 1 (max): 10000 2000 800 200 140 100 60 40 20
    Table 2 (max): 10000 4000 2000 800 200 140 100 60 40

    If Bet Max (DSW3-1) is 40:

    Table 1 (max): 20000 4000 1600 400 280 200 120 80 40
    Table 2 (max): 20000 8000 4000 1600 400 280 200 120 80



*****************************************************************************************


    --------------------
    ***  Memory Map  ***
    --------------------

    0x0000 - 0xBFFF    ; ROM space.
    0xC000 - 0xCFFF    ; NVRAM (settings, meters, etc).
    0xD000 - 0xD7FF    ; Video RAM 1 (VRAM1-1).
    0xD800 - 0xDFFF    ; Color RAM 1 (VRAM1-2).
    0xE000 - 0xE7FF    ; Video RAM 2 (VRAM2-1).
    0xE800 - 0xEFFF    ; Color RAM 2 (VRAM2-2).
    0xF000 - 0xF003    ; PPI8255_0 --> Input Ports 0 & 1.
    0xF080 - 0xF083    ; PPI8255_2 --> DSW1, DSW2, DSW3.
    0xF0C0 - 0xF0C3    ; PPI8255_3 --> DSW4.
    0xF200 - 0xF203    ; PPI8255_1 --> Input Ports 2 & 4.
    0xF400 - 0xF400    ; YM2149 control port 0.
    0xF600 - 0xF600    ; YM2149 R/W port 0 (Input Port 3).



*****************************************************************************************


    DRIVER UPDATES:


    [2008-07-06]

    - Initial release.
    - Set the proper screen size.
    - Decoded graphics.
    - Decoded the dual-state color circuitry.
    - Mapped the NVRAM, VRAM1-1, VRAM1-2, VRAM2-1 and VRAM2-2 properly.
    - Emulated 2x PPI 8255 devices.
    - Mapped the 4x DIP switches banks.
    - Added PORT_DIPLOCATION to all DIP switches.
    - Added DIP switches for 'Bet Max' and 'Limit'.
    - Added DIP switches for 'Jackpot' and 'Pay Table'.
    - Added the Memory Reset Switch.
    - Added the 2nd video & color RAM.
    - Added a 2nd tilemap for background graphics.
    - Simplified the graphics banks.
    - Fixed colors for foreground graphics.
    - Fixed visible area to show the top of background graphics.
    - Finally fixed colors for background graphics.
    - Added all coinage DIP switches.
    - Mapped all remaining inputs (service and player buttons).
    - Added pulse time limitation to coins A, B & C.
    - Switched to use 4x 8255 in replace of 2x 82c255 for I/O.
    - Created a handler to feed the z80 port0 requests.
    - Promoted lucky74s to 'working' state.
    - Added an alternate set, but the program ROM looks like incomplete,
      protected or just a bad dump.
    - Parent/clone relationship.
    - Added technical notes.

    From Dox:

    - Hooked interrupts.
    - Hooked the AY8910 and therefore the NMI trigger.
    - Changed the input "Key In" to active high.


    TODO:

    - Fix sounds (YM-2149F).
    - Add ADPCM samples (OKI M5205).
    - Figure out the unknown R/W.
    - Switch the color system to RESNET.
    - USART comm.
    - Co-processor.


*****************************************************************************************/


#define MASTER_CLOCK	XTAL_12MHz		/* confirmed */

#include "driver.h"
#include "sound/ay8910.h"
#include "sound/msm5205.h"
#include "machine/8255ppi.h"

static UINT8 unk;


/*************************
*     Video Hardware     *
*************************/

static UINT8 *videoram1, *videoram2, *colorram1, *colorram2;
static tilemap *fg_tilemap, *bg_tilemap;


static WRITE8_HANDLER( lucky74_videoram1_w )
{
	videoram1[offset] = data;
	tilemap_mark_tile_dirty(fg_tilemap, offset);
}

static WRITE8_HANDLER( lucky74_colorram1_w )
{
	colorram1[offset] = data;
	tilemap_mark_tile_dirty(fg_tilemap, offset);
}

static WRITE8_HANDLER( lucky74_videoram2_w )
{
	videoram2[offset] = data;
	tilemap_mark_tile_dirty(bg_tilemap, offset);
}

static WRITE8_HANDLER( lucky74_colorram2_w )
{
	colorram2[offset] = data;
	tilemap_mark_tile_dirty(bg_tilemap, offset);
}


static TILE_GET_INFO( get_fg_tile_info )
{
/*  - bits -
    7654 3210
    ---- xxxx   tiles color.
    xxxx ----   tiles page offset.
*/
	int bank = 0;
	int attr = colorram1[tile_index];
	int code = videoram1[tile_index] + ((attr & 0xf0) << 4);
	int color = (attr & 0x0f);

	SET_TILE_INFO(bank, code, color, 0);
}


static TILE_GET_INFO( get_bg_tile_info )
{
/*  - bits -
    7654 3210
    ---- xxxx   tiles color.
    xxxx ----   tiles page offset.
*/
	int bank = 1;
	int attr = colorram2[tile_index];
	int code = videoram2[tile_index] + ((attr & 0xf0) << 4);
	int color = (attr & 0x0f);

	SET_TILE_INFO(bank, code, color, 0);
}


static VIDEO_START( lucky74 )
{
	bg_tilemap = tilemap_create(get_bg_tile_info, tilemap_scan_rows, 8, 8, 64, 32);
	fg_tilemap = tilemap_create(get_fg_tile_info, tilemap_scan_rows, 8, 8, 64, 32);

	tilemap_set_transparent_pen(fg_tilemap, 0);
}


static VIDEO_UPDATE( lucky74 )
{
	tilemap_draw(bitmap, cliprect, bg_tilemap, 0, 0);
	tilemap_draw(bitmap, cliprect, fg_tilemap, 0, 0);
	return 0;
}


static PALETTE_INIT( lucky74 )
/*
   There are 2 states (see the technical notes).
   We're constructing a double-sized palette with one half for each state.
*/
{
	int i;

	for (i = 0; i < 256; i++)
	{
		int bit0, bit1, bit2, bit3, r1, g1, b1, r2, g2, b2;

        /* red component (state 1, PROM E6) */
		bit0 = (color_prom[0x000 + i] >> 0) & 0x01;
		bit1 = (color_prom[0x000 + i] >> 1) & 0x01;
		bit2 = (color_prom[0x000 + i] >> 2) & 0x01;
		bit3 = (color_prom[0x000 + i] >> 3) & 0x01;
		r1 = 0x0e * bit0 + 0x1f * bit1 + 0x43 * bit2 + 0x8f * bit3;

        /* red component (state 2, PROM E7) */
		bit0 = (color_prom[0x100 + i] >> 0) & 0x01;
		bit1 = (color_prom[0x100 + i] >> 1) & 0x01;
		bit2 = (color_prom[0x100 + i] >> 2) & 0x01;
		bit3 = (color_prom[0x100 + i] >> 3) & 0x01;
		r2 = 0x0e * bit0 + 0x1f * bit1 + 0x43 * bit2 + 0x8f * bit3;

        /* green component (state 1, PROM D6) */
		bit0 = (color_prom[0x200 + i] >> 0) & 0x01;
		bit1 = (color_prom[0x200 + i] >> 1) & 0x01;
		bit2 = (color_prom[0x200 + i] >> 2) & 0x01;
		bit3 = (color_prom[0x200 + i] >> 3) & 0x01;
		g1 = 0x0e * bit0 + 0x1f * bit1 + 0x43 * bit2 + 0x8f * bit3;

        /* green component (state 2, PROM D7) */
		bit0 = (color_prom[0x300 + i] >> 0) & 0x01;
		bit1 = (color_prom[0x300 + i] >> 1) & 0x01;
		bit2 = (color_prom[0x300 + i] >> 2) & 0x01;
		bit3 = (color_prom[0x300 + i] >> 3) & 0x01;
		g2 = 0x0e * bit0 + 0x1f * bit1 + 0x43 * bit2 + 0x8f * bit3;

        /* blue component (state 1, PROM C6) */
		bit0 = (color_prom[0x400 + i] >> 0) & 0x01;
		bit1 = (color_prom[0x400 + i] >> 1) & 0x01;
		bit2 = (color_prom[0x400 + i] >> 2) & 0x01;
		bit3 = (color_prom[0x400 + i] >> 3) & 0x01;
		b1 = 0x0e * bit0 + 0x1f * bit1 + 0x43 * bit2 + 0x8f * bit3;

        /* blue component (state 2, PROM C7) */
		bit0 = (color_prom[0x500 + i] >> 0) & 0x01;
		bit1 = (color_prom[0x500 + i] >> 1) & 0x01;
		bit2 = (color_prom[0x500 + i] >> 2) & 0x01;
		bit3 = (color_prom[0x500 + i] >> 3) & 0x01;
		b2 = 0x0e * bit0 + 0x1f * bit1 + 0x43 * bit2 + 0x8f * bit3;


        /* PROMs circuitry, 1st state */
		palette_set_color(machine, i, MAKE_RGB(r1, g1, b1));

        /* PROMs circuitry, 2nd state */
		palette_set_color(machine, i + 256, MAKE_RGB(r2, g2, b2));
	}
}


/*****************************
*    Read/Write  Handlers    *
*****************************/

static READ8_HANDLER( testport_rw )
{
	return 0xff;
}

static WRITE8_HANDLER( unk_w )
{
	unk = data;
}


/************************
*    Interrupts Gen     *
************************/

static INTERRUPT_GEN( nmi_interrupt )
{
	if ((unk & 0x10) == 0)	/* ym2149 portB bit 4 trigger the NMI */
	{
		cpunum_set_input_line(machine, 0, INPUT_LINE_NMI, PULSE_LINE);
	}
}


/*************************
* Memory Map Information *
*************************/

static ADDRESS_MAP_START( lucky74_map, ADDRESS_SPACE_PROGRAM, 8 )
	AM_RANGE(0x0000, 0xbfff) AM_ROM
	AM_RANGE(0xc000, 0xcfff) AM_RAM AM_BASE(&generic_nvram) AM_SIZE(&generic_nvram_size)	/* NVRAM */
	AM_RANGE(0xd000, 0xd7ff) AM_RAM AM_WRITE(lucky74_videoram1_w) AM_BASE(&videoram1)		/* VRAM1-1 */
	AM_RANGE(0xd800, 0xdfff) AM_RAM AM_WRITE(lucky74_colorram1_w) AM_BASE(&colorram1)		/* VRAM1-2 */
	AM_RANGE(0xe000, 0xe7ff) AM_RAM AM_WRITE(lucky74_videoram2_w) AM_BASE(&videoram2)		/* VRAM2-1 */
	AM_RANGE(0xe800, 0xefff) AM_RAM AM_WRITE(lucky74_colorram2_w) AM_BASE(&colorram2)		/* VRAM2-2 */
	AM_RANGE(0xf000, 0xf003) AM_DEVREADWRITE(PPI8255, "ppi8255_0", ppi8255_r, ppi8255_w)	/* Input Ports 0 & 1 */
	AM_RANGE(0xf080, 0xf083) AM_DEVREADWRITE(PPI8255, "ppi8255_2", ppi8255_r, ppi8255_w)	/* DSW 1, 2 & 3 */
	AM_RANGE(0xf0c0, 0xf0c3) AM_DEVREADWRITE(PPI8255, "ppi8255_3", ppi8255_r, ppi8255_w)	/* DSW 4 */
	AM_RANGE(0xf200, 0xf203) AM_DEVREADWRITE(PPI8255, "ppi8255_1", ppi8255_r, ppi8255_w)	/* Input Ports 2 & 4 */
	AM_RANGE(0xf400, 0xf400) AM_WRITE(AY8910_control_port_0_w)
	AM_RANGE(0xf600, 0xf600) AM_READWRITE(AY8910_read_port_0_r, AY8910_write_port_0_w)		/* YM2149 (Input Port 1) */
ADDRESS_MAP_END

static ADDRESS_MAP_START( lucky74_portmap, ADDRESS_SPACE_IO, 8 )
	ADDRESS_MAP_GLOBAL_MASK(0xff)
	AM_RANGE(0x00, 0x00) AM_READ(testport_rw)
//  AM_RANGE(0x00, 0x05) AM_WRITE(???)
//  AM_RANGE(0xff, 0xff) AM_READWRITE(???)
ADDRESS_MAP_END

/* I/O byte R/W

    0x00    R    when hang....
    0x00      W  unknown....
    0x01      W  unknown....
    0x02      W  unknown....
    0x03      W  unknown....
    0x04      W  unknown....
    0x05      W  unknown....

    0xFF    R/W  unknown....

   -----------------

   Unknown writes:


   0xf100        W  sound?
   0xf300        W  unknown...
   0xf500        W  unknown...

   0xf400        W  ym2149
   0xf600      R W  ym2149

   0xf701        W  unknown...

   0xf800-03   R W  unknown...


*** log ***

cpu #0 (PC=00000105): unmapped I/O byte write to 000000FF = 04
cpu #0 (PC=00000107): unmapped I/O byte read from 000000FF
cpu #0 (PC=00000111): unmapped I/O byte write to 000000FF = FB
cpu #0 (PC=00000113): unmapped I/O byte read from 000000FF

cpu #0 (PC=0000011E): unmapped program memory byte write to 0000F400 = 07  ; YM2149 control
cpu #0 (PC=00000123): unmapped program memory byte write to 0000F600 = 80  ; YM2149 data
cpu #0 (PC=00000128): unmapped program memory byte write to 0000F400 = 0F  ; YM2149 control
cpu #0 (PC=0000012D): unmapped program memory byte write to 0000F600 = 16  ; YM2149 data

cpu #0 (PC=00000132): unmapped program memory byte write to 0000F003 = 92  ; PPI 8255_0 --> ports A & B as input.
cpu #0 (PC=00000137): unmapped program memory byte write to 0000F203 = 99  ; PPI 8255_1 --> ports A & C as input.
cpu #0 (PC=0000013C): unmapped program memory byte write to 0000F083 = 9B  ; PPI 8255_2 --> ports A, B and half of C as input.
cpu #0 (PC=00000141): unmapped program memory byte write to 0000F0C3 = 90  ; PPI 8255_3 --> port A as input.

cpu #0 (PC=0000014B): unmapped program memory byte write to 0000F100 = 9F  ; unknown...
cpu #0 (PC=0000014F): unmapped program memory byte write to 0000F100 = BF
cpu #0 (PC=00000153): unmapped program memory byte write to 0000F100 = DF
cpu #0 (PC=00000157): unmapped program memory byte write to 0000F100 = FF
cpu #0 (PC=0000015B): unmapped program memory byte write to 0000F300 = 9F
cpu #0 (PC=0000015F): unmapped program memory byte write to 0000F300 = BF
cpu #0 (PC=00000163): unmapped program memory byte write to 0000F300 = DF
cpu #0 (PC=00000167): unmapped program memory byte write to 0000F300 = FF
cpu #0 (PC=0000016B): unmapped program memory byte write to 0000F500 = 9F
cpu #0 (PC=0000016F): unmapped program memory byte write to 0000F500 = BF
cpu #0 (PC=00000173): unmapped program memory byte write to 0000F500 = DF
cpu #0 (PC=00000177): unmapped program memory byte write to 0000F500 = FF

cpu #0 (PC=0000017C): unmapped program memory byte write to 0000F800 = 00  ; unknown...
cpu #0 (PC=00000181): unmapped program memory byte write to 0000F802 = 80
cpu #0 (PC=00000186): unmapped program memory byte write to 0000F803 = C3
cpu #0 (PC=0000018B): unmapped program memory byte write to 0000F803 = 3C

cpu #0 (PC=0000018F): unmapped program memory byte write to 0000F701 = 00  ; unknown...
cpu #0 (PC=00000192): unmapped program memory byte write to 0000F701 = 00
cpu #0 (PC=00000195): unmapped program memory byte write to 0000F701 = 00
cpu #0 (PC=0000019A): unmapped program memory byte write to 0000F701 = 40
cpu #0 (PC=0000019F): unmapped program memory byte write to 0000F701 = 4F

cpu #0 (PC=00000105): unmapped I/O byte write to 000000FF = 04
cpu #0 (PC=00000107): unmapped I/O byte read from 000000FF
cpu #0 (PC=00000111): unmapped I/O byte write to 000000FF = FB
cpu #0 (PC=00000113): unmapped I/O byte read from 000000FF

cpu #0 (PC=000002A6): unmapped program memory byte write to 0000F803 = 55  ; testing bits?...
cpu #0 (PC=000002AB): unmapped program memory byte write to 0000F803 = AA
cpu #0 (PC=000002B0): unmapped program memory byte write to 0000F803 = 99
cpu #0 (PC=000002B5): unmapped program memory byte write to 0000F803 = 66
cpu #0 (PC=000002BA): unmapped program memory byte write to 0000F801 = 22

*/

/*************************
*      Input Ports       *
*************************/

static INPUT_PORTS_START( lucky74 )

/*  Player buttons are the same for players 1 & 2.
    Test mode shows them as dupes. Maybe are multiplexed?
*/
	PORT_START_TAG("IN0")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_NAME("Hold 1") PORT_CODE(KEYCODE_Z)	/* 'A' in test mode */
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_NAME("Hold 2") PORT_CODE(KEYCODE_X)	/* 'B' in test mode */
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_NAME("Hold 3") PORT_CODE(KEYCODE_C)	/* 'C' in test mode */
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_BUTTON4 ) PORT_NAME("Hold 4") PORT_CODE(KEYCODE_V)	/* 'D' in test mode */
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON5 ) PORT_NAME("Hold 5") PORT_CODE(KEYCODE_B)	/* 'E' in test mode */
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON6 ) PORT_NAME("Small")  PORT_CODE(KEYCODE_S)	/* 'F' in test mode */
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("Input G")  PORT_CODE(KEYCODE_J)	/* 'G' in test mode */
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("Input H")  PORT_CODE(KEYCODE_K)	/* 'H' in test mode */

	PORT_START_TAG("IN1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_BUTTON7 ) PORT_NAME("Bet")         PORT_CODE(KEYCODE_2)	/* 'I' in test mode */
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_BUTTON8 ) PORT_NAME("Start")       PORT_CODE(KEYCODE_1)	/* 'J' in test mode */
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_BUTTON9 ) PORT_NAME("Cancel")      PORT_CODE(KEYCODE_N)	/* 'K' in test mode */
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_BUTTON10 ) PORT_NAME("Double-Up")  PORT_CODE(KEYCODE_3)	/* 'L' in test mode */
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON11 ) PORT_NAME("Take Score") PORT_CODE(KEYCODE_4)	/* 'M' & 'Q' in test mode */
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON12 ) PORT_NAME("Big")        PORT_CODE(KEYCODE_A)	/* 'N' in test mode */
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("Input O & P")   PORT_CODE(KEYCODE_M)	/* 'O' & 'P' in test mode */
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )	/* not in test mode */

	PORT_START_TAG("IN2")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_SERVICE ) PORT_NAME("Books")		PORT_CODE(KEYCODE_0)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_SERVICE ) PORT_NAME("Test Mode") PORT_CODE(KEYCODE_F2) PORT_TOGGLE
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_SERVICE ) PORT_NAME("Pay Out")	PORT_CODE(KEYCODE_E)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_SERVICE ) PORT_NAME("Key Out")	PORT_CODE(KEYCODE_W)
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START_TAG("IN3")	/* driven through YM2149, port A */
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_COIN1 )    PORT_IMPULSE(2)	/* Coin A */
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_SERVICE ) PORT_NAME("Key In")	PORT_CODE(KEYCODE_Q)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_COIN2 )    PORT_IMPULSE(2)	/* Coin B */
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_COIN3 )    PORT_IMPULSE(2)	/* Coin C */
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_SERVICE )  PORT_NAME("Service")	PORT_CODE(KEYCODE_9)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START_TAG("IN4")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_SERVICE ) PORT_NAME("Memory Reset Switch")	PORT_CODE(KEYCODE_R)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START_TAG("DSW1")
	PORT_DIPNAME( 0x01, 0x01, "Unknown 1-1" )	PORT_DIPLOCATION("DSW1:1")
	PORT_DIPSETTING(    0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x02, "Jackpot" )		PORT_DIPLOCATION("DSW1:2")
	PORT_DIPSETTING(    0x02, "2000" )
	PORT_DIPSETTING(    0x00, "3000" )
	PORT_DIPNAME( 0x04, 0x04, "Unknown 1-3" )	PORT_DIPLOCATION("DSW1:3")
	PORT_DIPSETTING(    0x04, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x08, "Unknown 1-4" )	PORT_DIPLOCATION("DSW1:4")
	PORT_DIPSETTING(    0x08, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x10, "Unknown 1-5" )	PORT_DIPLOCATION("DSW1:5")
	PORT_DIPSETTING(    0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x20, "Unknown 1-6" )	PORT_DIPLOCATION("DSW1:6")
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x40, "Unknown 1-7" )	PORT_DIPLOCATION("DSW1:7")
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, "Unknown 1-8" )	PORT_DIPLOCATION("DSW1:8")
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

	PORT_START_TAG("DSW2")
    /* DIPs 1-4 handle the harcoded coinage for Coin A, B and Remote credits */
	PORT_DIPNAME( 0x0f, 0x0f, "Coinage A, B & Remote" )	PORT_DIPLOCATION("DSW2:1,2,3,4")
	PORT_DIPSETTING(    0x00, "A: 20 Coins/1 Credit; B: 4 Coins/1 Credit;   R: 2 Pulses/1 Credit   " )
	PORT_DIPSETTING(    0x01, "A: 15 Coins/1 Credit; B: 3 Coins/1 Credit;   R: 15 Pulses/10 Credits" )
	PORT_DIPSETTING(    0x02, "A: 10 Coins/1 Credit; B: 2 Coins/1 Credit;   R: 1 Pulse/1 Credit    " )
	PORT_DIPSETTING(    0x03, "A: 4 Coins/1 Credit;  B: 5 Coins/5 Credits;  R: 4 Pulses/10 Credits " )
	PORT_DIPSETTING(    0x04, "A: 3 Coins/2 Credits; B: 3 Coins/10 Credits; R: 3 Pulses/20 Credits " )
	PORT_DIPSETTING(    0x05, "A: 3 Coins/1 Credit;  B: 3 Coins/5 Credits;  R: 3 Pulses/10 Credits " )
	PORT_DIPSETTING(    0x06, "A: 2 Coins/1 Credit;  B: 2 Coins/5 Credits;  R: 1 Pulse/5 Credits   " )
	PORT_DIPSETTING(    0x07, "A: 5 Coins/1 Credit;  B: 1 Coin/1 Credit;    R: 1 Pulse/2 Credits   " )
	PORT_DIPSETTING(    0x08, "A: 5 Coins/2 Credits; B: 1 Coin/2 Credits;   R: 1 Pulse/4 Credits   " )
	PORT_DIPSETTING(    0x09, "A: 5 Coins/3 Credits; B: 1 Coin/3 Credits;   R: 1 Pulse/6 Credits   " )
	PORT_DIPSETTING(    0x0a, "A: 5 Coins/4 Credits; B: 1 Coin/4 Credits;   R: 1 Pulse/8 Credits   " )
	PORT_DIPSETTING(    0x0b, "A: 1 Coin/1 Credit;   B: 1 Coin/5 Credits;   R: 1 Pulse/10 Credits  " )
	PORT_DIPSETTING(    0x0c, "A: 5 Coins/6 Credits; B: 1 Coin/6 Credits;   R: 1 Pulse/12 Credits  " )
	PORT_DIPSETTING(    0x0d, "A: 1 Coin/2 Credits;  B: 1 Coin/10 Credits;  R: 1 Pulse/20 Credits  " )
	PORT_DIPSETTING(    0x0e, "A: 1 Coin/5 Credits;  B: 1 Coin/25 Credits;  R: 1 Pulse/50 Credits  " )
	PORT_DIPSETTING(    0x0f, "A: 1 Coin/10 Credits; B: 1 Coin/50 Credits;  R: 1 Pulse/100 Credits " )
    /* DIPs 5-8 handle the Coin C coinage */
	PORT_DIPNAME( 0xf0, 0xf0, "Coinage C" )				PORT_DIPLOCATION("DSW2:5,6,7,8")
	PORT_DIPSETTING(    0x00, "10 Coins/1 Credit" )
	PORT_DIPSETTING(    0x10, DEF_STR( 5C_1C ) )
	PORT_DIPSETTING(    0x30, DEF_STR( 4C_1C ) )
	PORT_DIPSETTING(    0x40, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(    0x20, "5 Coins/2 Credits" )		/* 2.5 coins per credit */
	PORT_DIPSETTING(    0x50, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x70, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x60, DEF_STR( 2C_3C ) )
	PORT_DIPSETTING(    0x80, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0x90, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(    0xa0, DEF_STR( 1C_5C ) )
	PORT_DIPSETTING(    0xb0, "1 Coin/10 Credits" )
	PORT_DIPSETTING(    0xc0, "1 Coin/20 Credits" )
	PORT_DIPSETTING(    0xd0, "1 Coin/25 Credits" )
	PORT_DIPSETTING(    0xe0, "1 Coin/40 Credits" )
	PORT_DIPSETTING(    0xf0, "1 Coin/50 Credits" )

	PORT_START_TAG("DSW3")
	PORT_DIPNAME( 0x01, 0x01, "Bet Max" )		PORT_DIPLOCATION("DSW3:1")
	PORT_DIPSETTING(    0x01, "20" )
	PORT_DIPSETTING(    0x00, "40" )
	PORT_DIPNAME( 0x02, 0x02, "Unknown 3-2" )	PORT_DIPLOCATION("DSW3:2")
	PORT_DIPSETTING(    0x02, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x04, 0x04, "Unknown 3-3" )	PORT_DIPLOCATION("DSW3:3")
	PORT_DIPSETTING(    0x04, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x18, 0x18, "Limit" )			PORT_DIPLOCATION("DSW3:4,5")
	PORT_DIPSETTING(    0x18, "No Limit" )
	PORT_DIPSETTING(    0x10, "10000" )
	PORT_DIPSETTING(    0x08, "15000" )
	PORT_DIPSETTING(    0x00, "20000" )
	PORT_DIPNAME( 0x20, 0x20, "Unknown 3-6" )	PORT_DIPLOCATION("DSW3:6")
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x40, "Pay Table" )		PORT_DIPLOCATION("DSW3:7")
	PORT_DIPSETTING(    0x40, "Table 1" )	/* see the game notes */
	PORT_DIPSETTING(    0x00, "Table 2" )	/* see the game notes */
	PORT_DIPNAME( 0x80, 0x80, "Unknown 3-8" )	PORT_DIPLOCATION("DSW3:8")
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

	PORT_START_TAG("DSW4")
	PORT_DIPNAME( 0x01, 0x01, "Unknown 4-1" )	PORT_DIPLOCATION("DSW4:1")
	PORT_DIPSETTING(    0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x02, "Unknown 4-2" )	PORT_DIPLOCATION("DSW4:2")
	PORT_DIPSETTING(    0x02, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x04, 0x04, "Unknown 4-3" )	PORT_DIPLOCATION("DSW4:3")
	PORT_DIPSETTING(    0x04, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x08, "Unknown 4-4" )	PORT_DIPLOCATION("DSW4:4")
	PORT_DIPSETTING(    0x08, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x10, "Unknown 4-5" )	PORT_DIPLOCATION("DSW4:5")
	PORT_DIPSETTING(    0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x20, "Unknown 4-6" )	PORT_DIPLOCATION("DSW4:6")
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x40, "Unknown 4-7" )	PORT_DIPLOCATION("DSW4:7")
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, "Unknown 4-8" )	PORT_DIPLOCATION("DSW4:8")
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
INPUT_PORTS_END


/*************************
*    Graphics Layouts    *
*************************/

static const gfx_layout tilelayout =
{
	8, 8,
	RGN_FRAC(1,4),	/* 4096 tiles */
	4,
	{ 0, RGN_FRAC(1,4), RGN_FRAC(2,4), RGN_FRAC(3,4) },	/* bitplanes are separated */
	{ 0, 1, 2, 3, 4, 5, 6, 7 },
	{ 0*8, 1*8, 2*8, 3*8, 4*8, 5*8, 6*8, 7*8 },
	8*8	/* every char takes 8 consecutive bytes */
};


/******************************
* Graphics Decode Information *
******************************/

static GFXDECODE_START( lucky74 )
	GFXDECODE_ENTRY( "gfx1", 0, tilelayout, 0, 16 )	/* text, frames & cards */
	GFXDECODE_ENTRY( "gfx2", 0, tilelayout, 256, 16 )	/* title & whores */
GFXDECODE_END


/*************************************
*     PPI 82C255 (x2) Interfaces     *
*************************************/

/* Each 82C255 behaves like 2x 8255.
   Since we don't support yet, we replace both 82C255
   with 4x 8255...
*/
static const ppi8255_interface ppi8255_intf[4] =
{
	{	/* A & B as input */
		input_port_0_r,			/* Port A read, IN0 */
		input_port_1_r,			/* Port B read, IN1 */
		NULL,					/* Port C read */
		NULL,					/* Port A write */
		NULL,					/* Port B write */
		NULL					/* Port C write */
	},
	{	/* A & C as input */
		input_port_2_r,			/* Port A read, IN2 */
		NULL,					/* Port B read */
		input_port_4_r,			/* Port C read, IN4 */
		NULL,					/* Port A write */
		NULL,					/* Port B write */
		NULL					/* Port C write */
	},
	{	/* A, B & C as input */
		input_port_5_r,			/* Port A read, DSW1 */
		input_port_6_r,			/* Port B read, DSW2 */
		input_port_7_r,			/* Port C read, DSW3 */
		NULL,					/* Port A write */
		NULL,					/* Port B write */
		NULL					/* Port C write */
	},
	{	/* A as input */
		input_port_8_r,			/* Port A read, DSW4 */
		NULL,					/* Port B read */
		NULL,					/* Port C read */
		NULL,					/* Port A write */
		NULL,					/* Port B write */
		NULL					/* Port C write */
	}
};


/*****************************
*      Sound Interfaces      *
*****************************/

static const struct AY8910interface ay8910_interface =
{
	AY8910_LEGACY_OUTPUT,
	AY8910_DEFAULT_LOADS,
	input_port_3_r,
	NULL,	/* a sort of status byte */
	NULL,
	unk_w
};


/*************************
*    Machine Drivers     *
*************************/

static MACHINE_DRIVER_START( lucky74 )

	/* basic machine hardware */
	MDRV_CPU_ADD("main", Z80, MASTER_CLOCK/8)	/* guess */
	MDRV_CPU_PROGRAM_MAP(lucky74_map, 0)
	MDRV_CPU_IO_MAP(lucky74_portmap,0)
	MDRV_CPU_VBLANK_INT("main", nmi_interrupt)

	MDRV_NVRAM_HANDLER(generic_0fill)

	/* 2x 82c255 (4x 8255) */
	MDRV_DEVICE_ADD( "ppi8255_0", PPI8255 )
	MDRV_DEVICE_CONFIG( ppi8255_intf[0] )

	MDRV_DEVICE_ADD( "ppi8255_1", PPI8255 )
	MDRV_DEVICE_CONFIG( ppi8255_intf[1] )

	MDRV_DEVICE_ADD( "ppi8255_2", PPI8255 )
	MDRV_DEVICE_CONFIG( ppi8255_intf[2] )

	MDRV_DEVICE_ADD( "ppi8255_3", PPI8255 )
	MDRV_DEVICE_CONFIG( ppi8255_intf[3] )


	/* video hardware */
	MDRV_SCREEN_ADD("main", RASTER)
	MDRV_SCREEN_REFRESH_RATE(60)
	MDRV_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(0))
	MDRV_SCREEN_FORMAT(BITMAP_FORMAT_INDEXED16)
	MDRV_SCREEN_SIZE(64*8, 32*8)
	MDRV_SCREEN_VISIBLE_AREA(0*8, 64*8-1, 1*8, 30*8-1)	/* allow to show the top of bg gfx */

	MDRV_GFXDECODE(lucky74)

	MDRV_PALETTE_INIT(lucky74)
	MDRV_PALETTE_LENGTH(512)

	MDRV_VIDEO_START(lucky74)
	MDRV_VIDEO_UPDATE(lucky74)

	/* sound hardware */
	MDRV_SPEAKER_STANDARD_MONO("mono")

	MDRV_SOUND_ADD("ay", AY8910, MASTER_CLOCK/8)	/* YM2149F */
	MDRV_SOUND_CONFIG(ay8910_interface)
	MDRV_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.50)

MACHINE_DRIVER_END


/*************************
*        Rom Load        *
*************************/

ROM_START( lucky74s )
	ROM_REGION( 0x10000, RGNCLASS_CPU, "main", 0 )
	ROM_LOAD( "luckychi.00",	0x0000, 0x10000, CRC(3b906f0e) SHA1(1f9abd168c60b0d22fa6c7391bfdf5f3aabd66ef) )

	ROM_REGION( 0x20000, RGNCLASS_GFX, "gfx1", ROMREGION_DISPOSE )
	ROM_LOAD( "luckychi.12",	0x00000, 0x8000, CRC(ff934c20) SHA1(07cd2225dfc0e5b74be2e1b379c6b180e37660db) )
	ROM_LOAD( "luckychi.11",	0x08000, 0x8000, CRC(2fd6fb8a) SHA1(1a910e0a2e6db22a8d9a65d7b932f9ca39601e9c) )
	ROM_LOAD( "luckychi.13",	0x10000, 0x8000, CRC(c70a6da3) SHA1(195772ef649e21a5c54c5871e7b858967b6ebee8) )
	ROM_LOAD( "luckychi.14",	0x18000, 0x8000, CRC(b5813b67) SHA1(cce38e33a5218d6839d956174807d88e7c070d5a) )

	ROM_REGION( 0x20000, RGNCLASS_GFX, "gfx2", ROMREGION_DISPOSE )
	ROM_LOAD( "luckychi.17",	0x00000, 0x8000, CRC(010ffa4a) SHA1(8856d61b71e951509073bc359851f47c39c4274d) )
	ROM_LOAD( "luckychi.16",	0x08000, 0x8000, CRC(15104810) SHA1(586df734740209e2a05932e31d2a301d330e8cbd) )
	ROM_LOAD( "luckychi.18",	0x10000, 0x8000, CRC(f2d45e76) SHA1(46df7bf98434c836fd38539575a35bf67c9ec2c6) )
	ROM_LOAD( "luckychi.19",	0x18000, 0x8000, CRC(6b0196f3) SHA1(277049279dcfcf07189dbdb20935c2a71b2f6061) )

	ROM_REGION( 0x10000, RGNCLASS_SOUND, "adpcm", 0 )	/* 4-bit ADPCM samples @ 8kHz */
	ROM_LOAD( "luckyson.15",	0x00000, 0x10000, CRC(b896c87f) SHA1(985e625a937abd6353218f0cace14d3adec4c1bf) )

	ROM_REGION( 0x0600, RGNCLASS_PROMS, "proms", 0 )
	ROM_LOAD( "luckyprom.e6",	0x0000, 0x0100, CRC(ae793fef) SHA1(e4e2d2dccabad7d756811fb2d5e123bf30f106f3) )
	ROM_LOAD( "luckyprom.e7",	0x0100, 0x0100, CRC(7c772d0c) SHA1(9c99daa01ca56c7ebd48945505fcbae184998b13) )
	ROM_LOAD( "luckyprom.d6",	0x0200, 0x0100, CRC(61716584) SHA1(7a3e17f47ce173d79c12b2394edb8f32b7509e39) )
	ROM_LOAD( "luckyprom.d7",	0x0300, 0x0100, CRC(4003bc8f) SHA1(f830203c22a4f94b8b9f0b24e287204a742a8322) )
	ROM_LOAD( "luckyprom.c6",	0x0400, 0x0100, CRC(a8d2b3db) SHA1(7b346797bedc627fb2d49f19b18860a81c69e122) )
	ROM_LOAD( "luckyprom.c7",	0x0500, 0x0100, CRC(e62fd192) SHA1(86a189df2e2ccef6bd2a4e6d969e777fbba8cdf7) )
ROM_END

ROM_START( lucky74b )

/*  The program ROM seems incomplete.
    At start, just pop some registers and make a RTI.
    Maybe protection?
*/
	ROM_REGION( 0x10000, RGNCLASS_CPU, "main", 0 )
	ROM_LOAD( "luckygde.00",	0x0000, 0x10000, CRC(e3f7db99) SHA1(5c7d9d3fed9eb19d3d666c8c08b34968a9996a96) )	/* bad dump? */

	ROM_REGION( 0x20000, RGNCLASS_GFX, "gfx1", ROMREGION_DISPOSE )
	ROM_LOAD( "luckygde.12",	0x00000, 0x8000, CRC(7127465b) SHA1(3f72f91652fcab52c073744b1651fdfe772c584a) )
	ROM_LOAD( "luckygde.11",	0x08000, 0x8000, CRC(8a5ea91a) SHA1(8d22615c00ff7c8a27cd721618b5d32a8d089c95) )
	ROM_LOAD( "luckygde.13",	0x10000, 0x8000, CRC(bbb63ac1) SHA1(ab986055e34d90e81caf20c28c5ad89715209d0e) )
	ROM_LOAD( "luckygde.14",	0x18000, 0x8000, CRC(dcffdf07) SHA1(d63fd7d23e488650d3731830f07bce0ce64424b8) )

	ROM_REGION( 0x20000, RGNCLASS_GFX, "gfx2", ROMREGION_DISPOSE )
	ROM_LOAD( "luckygde.17",	0x00000, 0x8000, CRC(18da3468) SHA1(6dc60da939bfa7528e1fe75a85328a32047c8990) )
	ROM_LOAD( "luckygde.16",	0x08000, 0x8000, CRC(0e831be5) SHA1(302a68203f565718f7f537dab50fb52250c48859) )
	ROM_LOAD( "luckygde.18",	0x10000, 0x8000, CRC(717e5f4e) SHA1(0f14c9525bf77bbc4de0d9695648acb40870a176) )
	ROM_LOAD( "luckygde.19",	0x18000, 0x8000, CRC(bb4608ae) SHA1(cc8ec596f445fe0364f254241227de368f309ebb) )

	ROM_REGION( 0x10000, RGNCLASS_SOUND, "adpcm", 0 )	/* 4-bit ADPCM samples @ 8kHz */
	ROM_LOAD( "luckyson.15",	0x00000, 0x10000, CRC(b896c87f) SHA1(985e625a937abd6353218f0cace14d3adec4c1bf) )

	ROM_REGION( 0x0600, RGNCLASS_PROMS, "proms", 0 )
	ROM_LOAD( "luckyprom.e6",	0x0000, 0x0100, CRC(ae793fef) SHA1(e4e2d2dccabad7d756811fb2d5e123bf30f106f3) )
	ROM_LOAD( "luckyprom.e7",	0x0100, 0x0100, CRC(7c772d0c) SHA1(9c99daa01ca56c7ebd48945505fcbae184998b13) )
	ROM_LOAD( "luckyprom.d6",	0x0200, 0x0100, CRC(61716584) SHA1(7a3e17f47ce173d79c12b2394edb8f32b7509e39) )
	ROM_LOAD( "luckyprom.d7",	0x0300, 0x0100, CRC(4003bc8f) SHA1(f830203c22a4f94b8b9f0b24e287204a742a8322) )
	ROM_LOAD( "luckyprom.c6",	0x0400, 0x0100, CRC(a8d2b3db) SHA1(7b346797bedc627fb2d49f19b18860a81c69e122) )
	ROM_LOAD( "luckyprom.c7",	0x0500, 0x0100, CRC(e62fd192) SHA1(86a189df2e2ccef6bd2a4e6d969e777fbba8cdf7) )
ROM_END


/*********************************************
*                Game Drivers                *
**********************************************

      YEAR  NAME      PARENT    MACHINE  INPUT     INIT  ROT    COMPANY         FULLNAME           FLAGS */
GAME( 1988, lucky74s, 0,        lucky74, lucky74,  0,    ROT0, "Wing Co.Ltd.", "Lucky 74 (small)", GAME_NO_SOUND )
GAME( 1988, lucky74b, lucky74s, lucky74, lucky74,  0,    ROT0, "Wing Co.Ltd.", "Lucky 74 (big)",   GAME_NO_SOUND | GAME_NOT_WORKING )
