// license:BSD-3-Clause
// copyright-holders:Kevin Horton,Jonathan Gevaryahu,Sandro Ronco
/******************************************************************************
*
*  Fidelity Electronics Z80 based board driver
*  By Kevin 'kevtris' Horton, Jonathan Gevaryahu AKA Lord Nightmare and Sandro Ronco
*
*  All detailed RE work done by Kevin 'kevtris' Horton
*
*  TODO:
*  * Figure out why it says the first speech line twice; it shouldn't?
*    It sometimes does this on Voice Sensory Chess Challenger real hardware.
*    It can also be heard on Advanced Talking Chess Challenger real hardware, cold boot.
*  * Get rom locations from pcb (done for UVC, VCC is probably similar)
*  * correctly hook up 7002/VBRC and 7014/bridgec3 speech so that the z80 is halted while words are being spoken
*
***********************************************************************

Talking Chess Challenger (VCC)
Advanced Talking Chess Challenger (UVC)
(which both share the same hardware)
----------------------

The CPU is a Z80 running at 4MHz.  The TSI chip runs at around 25KHz, using a
470K / 100pf RC network.  This system is very very basic, and is composed of just
the Z80, 4 ROMs, the TSI chip, and an 8255.


The Z80's interrupt inputs are all pulled to VCC, so no interrupts are used.

Reset is connected to a power-on reset circuit and a button on the keypad (marked RE).

The TSI chip connects to a 4K ROM.  All of the 'Voiced' Chess Challengers
use this same ROM  (three or four).  The later chess boards use a slightly different part
number, but the contents are identical.

Memory map (VCC):
-----------
0000-0FFF: 4K 2332 ROM 101-32103
1000-1FFF: 4K 2332 ROM VCC2
2000-2FFF: 4K 2332 ROM VCC3
4000-5FFF: 1K RAM (2114 SRAM x2)
6000-FFFF: empty

Memory map (UVC):
-----------
0000-1FFF: 8K 2364 ROM 101-64017
2000-2FFF: 4K 2332 ROM 101-32010
4000-5FFF: 1K RAM (2114 SRAM x2)
6000-FFFF: empty

I/O map:
--------
00-FF: 8255 port chip [LN edit: 00-03, mirrored over the 00-FF range; program accesses F4-F7]


8255 connections:
-----------------

PA.0 - segment G, TSI A0 (W)
PA.1 - segment F, TSI A1 (W)
PA.2 - segment E, TSI A2 (W)
PA.3 - segment D, TSI A3 (W)
PA.4 - segment C, TSI A4 (W)
PA.5 - segment B, TSI A5 (W)
PA.6 - segment A, language latch Data (W)
PA.7 - TSI START line, language latch clock (W, see below)

PB.0 - dot commons (W)
PB.1 - NC
PB.2 - digit 0, bottom dot (W)
PB.3 - digit 1, top dot (W)
PB.4 - digit 2 (W)
PB.5 - digit 3 (W)
PB.6 - enable language switches (W, see below)
PB.7 - TSI DONE line (R)

(button rows pulled up to 5V through 2.2K resistors)
PC.0 - button row 0, German language jumper (R)
PC.1 - button row 1, French language jumper (R)
PC.2 - button row 2, Spanish language jumper (R)
PC.3 - button row 3, special language jumper (R)
PC.4 - button column A (W)
PC.5 - button column B (W)
PC.6 - button column C (W)
PC.7 - button column D (W)


language switches:
------------------

When PB.6 is pulled low, the language switches can be read.  There are four.
They connect to the button rows.  When enabled, the row(s) will read low if
the jumper is present.  English only VCC's do not have the 367 or any pads stuffed.
The jumpers are labelled: French, German, Spanish, and special.


language latch:
---------------

There's an unstuffed 7474 on the board that connects to PA.6 and PA.7.  It allows
one to latch the state of A12 to the speech ROM.  The English version has the chip
missing, and a jumper pulling "A12" to ground.  This line is really a negative
enable.

To make the VCC multi-language, one would install the 74367 (note: it must be a 74367
or possibly a 74LS367.  A 74HC367 would not work since they rely on the input current
to keep the inputs pulled up), solder a piggybacked ROM to the existing English
speech ROM, and finally install a 7474 dual flipflop.

This way, the game can then detect which secondary language is present, and then it can
automatically select the correct ROM(s).  I have to test whether it will do automatic
determination and give you a language option on power up or something.

***********************************************************************

Chess Challenger 3/10
----------------------

This is an earlier hardware upon which the VCC and UVC above were based on;
The hardware is nearly the same; in fact the only significant differences are
the RAM being located in a different place, the lack of a speech chip, and
the connections to ports A and B on the PPI:

8255 connections:
-----------------

PA.0 - segment G (W)
PA.1 - segment F (W)
PA.2 - segment E (W)
PA.3 - segment D (W)
PA.4 - segment C (W)
PA.5 - segment B (W)
PA.6 - segment A (W)
PA.7 - 'beeper' direct speaker output (W)

PB.0 - dot commons (W)
PB.1 - NC
PB.2 - digit 0, bottom dot (W)
PB.3 - digit 1, top dot (W)
PB.4 - digit 2 (W)
PB.5 - digit 3 (W)
PB.6 - NC
PB.7 - Mode select (cc3 vs cc10, R)

(button rows pulled up to 5V through 2.2K resistors)
PC.0 - button row 0 (R)
PC.1 - button row 1 (R)
PC.2 - button row 2 (R)
PC.3 - button row 3 (R)
PC.4 - button column A (W)
PC.5 - button column B (W)
PC.6 - button column C (W)
PC.7 - button column D (W)

******************************************************************************
Voice Bridge Challenger (Model VBRC, later reissued as Model 7002)
and Bridge Challenger 3 (Model 7014)
(which both share the same* hardware)
--------------------------------
* The Bridge Challenger 3 does not actually have the 8 LEDs nor the
latches which operate them populated and the plastic indicator cap locations
are instead are covered by a piece of plastic, but they do work if manually
added.

This unit is similar in construction kinda to the chess challengers, however it
has an 8041 which does ALL of the system I/O.  The Z80 has NO IO AT ALL other than
what is performed through the 8041!

The main CPU is a Z80 running at 2.5MHz

INT connects to VCC (not used)
NMI connects to VCC (not used)
RST connects to power on reset, and reset button

The 8041 runs at 5MHz.

Memory Map:
-----------

0000-1FFF: 8K 101-64108 ROM
2000-3FFF: 8K 101-64109 ROM
4000-5FFF: 8K 101-64110 ROM
6000-7FFF: 1K of RAM (2114 * 2)
8000-DFFF: unused
E000-FFFF: write to TSI chip

NOTE: when the TSI chip is written to, the CPU IS STOPPED.  The CPU will run again
when the word is done being spoken.  This is because D0-D5 run to the TSI chip directly.

The TSI chip's ROM is 4K, and is marked 101-32118.  The clock is the same as the Chess
Challengers- 470K/100pf which gives a frequency around 25KHz or so.

I/O Map:
--------

00-FF: 8041 I/O ports (A0 selects between the two)



8041 pinout:
------------

(note: columns are pulled up with 10K resistors)

P10 - column H, RD LED, VFD grid 0
P11 - column G, DB LED, VFD grid 1
P12 - column F, <>V LED, VFD grid 2
P13 - column E, ^V LED, VFD grid 3
P14 - column D, W LED, VFD grid 4
P15 - column C, S LED, VFD grid 5
P16 - column B, E LED, VFD grid 6
P17 - column A, N LED, VFD grid 7

P20 - I/O expander
P21 - I/O expander
P22 - I/O expander
P23 - I/O expander
P24 - row 0 through inverter
P25 - row 1 through inverter
P26 - row 2 through inverter
P27 - row 3 through inverter

PROG - I/O expander

T0 - optical card sensor (high = bright/reflective, low = dark/non reflective)
T1 - connects to inverter, then nothing

D8243C I/O expander:
--------------------

P4.0 - segment M
P4.1 - segment L
P4.2 - segment N
P4.3 - segment E

P5.0 - segment D
P5.1 - segment I
P5.2 - segment K
P5.3 - segment J

P6.0 - segment A
P6.1 - segment B
P6.2 - segment F
P6.3 - segment G

P7.0 - LED enable (high = LEDs can be lit.  low = LEDs will not light)
P7.1 - goes through inverter, to pads that are not used
P7.2 - segment C
P7.3 - segment H


button matrix:
--------------

the matrix is composed of 8 columns by 4 rows.

     A  B  C  D     E  F  G  H
     -------------------------
0-   RE xx CL EN    J  Q  K  A
1-   BR PB DB SC    7  8  9 10
2-   DL CV VL PL    3  4  5  6
3-   cl di he sp   NT  P  1  2

xx - speaker symbol
cl - clubs symbol
di - diamonds symbol
he - hearts symbol
sp - spades symbol

NOTE: RE is not wired into the matrix, and is run separately out.

There are 8 LEDs, and an 8 digit 14 segment VFD with commas and periods.
This display is the same one as can be found on the speak and spell.

       A       * comma
  ***********  *
 * *I  *J K* *
F*  *  *  *  *B
 *   * * *   *
  G**** *****H
 *   * * *   *
E*  *  *  *  *C
 * *N  *M L* *
  ***********  *decimal point
       D

The digits of the display are numbered left to right, 0 through 7 and are controlled
by the grids.  hi = grid on, hi = segment on.

A detailed description of the hardware can be found also in the patent 4,373,719.

******************************************************************************

Sensory Chess Challenger champion (6502 based, needs its own driver .c file)
---------------------------------

Memory map:
-----------
0000-07FF: 2K of RAM
0800-0FFF: 1K of RAM (note: mirrored twice)
1000-17FF: PIA 0 (display, TSI speech chip)
1800-1FFF: PIA 1 (keypad, LEDs)
2000-3FFF: 101-64019 ROM (also used on the regular sensory chess challenger)
4000-7FFF: mirror of 0000-3FFF
8000-9FFF: not used
A000-BFFF: 101-1025A03 ROM
C000-DFFF: 101-1025A02 ROM
E000-FDFF: 101-1025A01 ROM
FE00-FFFF: 512 byte 74S474 PROM


CPU is a 6502 running at 1.95MHz (3.9MHz resonator, divided by 2)

NMI is not used.
IRQ is connected to a 600Hz oscillator (38.4KHz divided by 64).
Reset is connected to a power-on reset circuit.


PIA port lines:
---------------


PIA 0:
------

PA0 - 7seg segments H, TSI A0
PA1 - 7seg segments G, TSI A1
PA2 - 7seg segments C, TSI A2
PA3 - 7seg segments B, TSI A3
PA4 - 7seg segments A, TSI A4
PA5 - 7seg segments F, TSI A5
PA6 - 7seg segments E
PA7 - 7seg segments D

PB0 - A12 on speech ROM (if used... not used on this model, ROM is 4K)
PB1 - START line on S14001A
PB2 - white wire
PB3 - DONE line from S14001A
PB4 - Tone line (toggle to make a tone in the speaker)
PB5 - button column I
PB6 - selection jumper (resistor to 5V)
PB7 - selection jumper (resistor to ground)

CA1 - NC
CA2 - violet wire

CB1 - NC
CB2 - NC (connects to pin 14 of soldered connector)

PIA 1:
------

PA0 - button row 1
PA1 - button row 2
PA2 - button row 3
PA3 - button row 4
PA4 - button row 5
PA5 - button row 6
PA6 - 7442 selector bit 0
PA7 - 7442 selector bit 1

PB0 - LED row 1
PB1 - LED row 2
PB2 - LED row 3
PB3 - LED row 4
PB4 - LED row 5
PB5 - LED row 6
PB6 - LED row 7
PB7 - LED row 8

CA1 - button row 7
CA2 - selector bit 3

CB1 - button row 8
CB2 - selector bit 2

Selector: (attached to PIA 1, outputs 1 of 10 pins low.  7442)
---------

output # (selected turns this column on, and all others off)
0 - LED column A, button column A, 7seg digit 1
1 - LED column B, button column B, 7seg digit 2
2 - LED column C, button column C, 7seg digit 3
3 - LED column D, button column D, 7seg digit 4
4 - LED column E, button column E
5 - LED column F, button column F
6 - LED column G, button column G
7 - LED column H, button column H
8 - button column I
9 -

The rows/columns are indicated on the game board:


 ABCDEFGH   I
--------------
|            | 8
|            | 7
|            | 6
|            | 5
|            | 4
|            | 3
|            | 2
|            | 1
--------------

The "lone LED" is above the control column.
column I is the "control column" on the right for starting a new game, etc.

The upper 6 buttons are connected as such:

column A - speak
column B - RV
column C - TM
column D - LV
column E - DM
column F - ST

these 6 buttons use row 9 (connects to PIA 0)

LED display:
------------

43 21 (digit number)
-----
88:88

The LED display is four 7 segment digits.  normal ABCDEFG lettering is used for segments.

The upper dot is connected to digit 3 common
The lower dot is connected to digit 4 common
The lone LED is connected to digit 1 common

All three of the above are called "segment H".


***********************************************************************
Sensory Chess Challenger
------------------------

The display/button/LED/speech technology is identical to the above product.
Only the CPU board was changed.  As such, it works the same but is interfaced
to different port chips this time.

Hardware:
---------

On the board are 13 chips.

The CPU is a Z80A running at 3.9MHz, with 20K of ROM and 1K of RAM mapped.
I/O is composed of an 8255 triple port adaptor, and a Z80A PIO parallel I/O
interface.

There's the usual TSI S14001A speech synth with its requisite 4K ROM which is the
same as on the other talking chess boards.  The TSI chip is running at 26.37KHz.
It uses a 470K resistor and a 100pf capacitor.

The "perfect" clock would be 1/RC most likely (actually this will be skewed a tad by
duty cycle of the oscillator) which with those parts values gives 21.27KHz.  The
formula is probably more likely to be 1/1.2RC or so.

Rounding out the hardware are three driver chips for the LEDs, a 7404 inverter to
run the crystal osc, a 555 timer to generate a clock, and a 74138 selector.

NMI runs to a 555 oscillator that generates a 600Hz clock (measured: 598.9Hz.  It has a multiturn pot to adjust).
INT is pulled to 5V
RST connects to a power-on reset circuit


Memory map:
-----------

0000-1FFF: 8K ROM 101-64018
2000-3FFF: 8K ROM 101-64019 (also used on the sensory champ. chess challenger)
4000-5FFF: 4K ROM 101-32024
6000-7FFF: 1K of RAM (2114 * 2)
8000-FFFF: not used, maps to open bus

I/O map:
--------

There's only two chips in the I/O map, an 8255 triple port chip, and a Z80A PIO
parallel input/output device.

Decoding isn't performed using a selector, but instead address lines are used.

A2 connects to /CE on the 8255
A3 connects to /CE on the Z80A PIO

A1 connects to control/data select on PIO & A0 of 8255
A0 connects to port A/B select on PIO & A1 of 8255

So to enable only the 8255, you'd write/read to 08-0Bh for example
To enable only the PIO, you'd write/read to 04-07h for example.

writing to 00-03h will enable and write to BOTH chips, and reading 00-03h
will return data from BOTH chips (and cause a bus conflict).  The code probably
never does either of these things.

Likewise, writing/reading to 0Ch-0Fh will result in open bus, because neither chip's
enable line will be low.

This sequence repeats every 16 addresses.  So to recap:

00-03: both chips enabled (probably not used)
04-07: PIO enabled
08-0B: 8255 enabled
0C-0F: neither enabled

10-FF: mirrors of 00-0F.


Refer to the Sensory Champ. Chess Chall. above for explanations of the below
I/O names and labels.  It's the same.

8255:
-----

PA.0 - segment D, TSI A0
PA.1 - segment E, TSI A1
PA.2 - segment F, TSI A2
PA.3 - segment A, TSI A3
PA.4 - segment B, TSI A4
PA.5 - segment C, TSI A5
PA.6 - segment G
PA.7 - segment H

PB.0 - LED row 1
PB.1 - LED row 2
PB.2 - LED row 3
PB.3 - LED row 4
PB.4 - LED row 5
PB.5 - LED row 6
PB.6 - LED row 7
PB.7 - LED row 8

PC.0 - LED column A, button column A, 7seg digit 1
PC.1 - LED column B, button column B, 7seg digit 2
PC.2 - LED column C, button column C, 7seg digit 3
PC.3 - LED column D, button column D, 7seg digit 4
PC.4 - LED column E, button column E
PC.5 - LED column F, button column F
PC.6 - LED column G, button column G
PC.7 - LED column H, button column H


Z80A PIO:
---------

PA.0 - button row 1
PA.1 - button row 2
PA.2 - button row 3
PA.3 - button row 4
PA.4 - button row 5
PA.5 - button row 6
PA.6 - button row 7
PA.7 - button row 8

PB.0 - button column I
PB.1 - button row 9
PB.2 - Tone line (toggle to make tone in the speaker)
PB.3 - violet wire
PB.4 - white wire (and TSI done line)
PB.5 - selection jumper input (see below)
PB.6 - TSI start line
PB.7 - TSI ROM D0 line

selection jumpers:
------------------

These act like another row of buttons.  It is composed of two diode locations,
so there's up to 4 possible configurations.  My board does not have either diode
stuffed, so this most likely is "English".  I suspect it selects which language to use
for the speech synth.  Of course you need the other speech ROMs for this to function
properly.

Anyways, the two jumpers are connected to button columns A and B and the common
connects to Z80A PIO PB.5, which basically makes a 10th button row.  I would
expect that the software reads these once on startup only.

******************************************************************************/


/* Core includes */
#include "emu.h"
#include "cpu/z80/z80.h"
#include "cpu/mcs48/mcs48.h"
#include "machine/i8255.h"
#include "machine/i8243.h"
#include "machine/z80pio.h"
#include "sound/beep.h"
#include "sound/s14001a.h"
#include "includes/fidelz80.h"
#include "fidelz80.lh"
#include "vsc.lh"
#include "bridgec3.lh"

//#include "debugger.h"

/* Devices */

/******************************************************************************
    I8255 Device, for VCC/UVC
******************************************************************************/

void fidelz80_state::update_display()
{
	// data for the 4x 7seg leds, bits are 0bxABCDEFG
	UINT8 out_digit = BITSWAP8( m_digit_data,7,0,1,2,3,4,5,6 ) & 0x7f;

	if (m_led_selected&0x04)
	{
		output_set_digit_value(0, out_digit);

		output_set_led_value(1, m_led_data & 0x01);
	}
	if (m_led_selected&0x08)
	{
		output_set_digit_value(1, out_digit);

		output_set_led_value(0, m_led_data & 0x01);
	}
	if (m_led_selected&0x10)
	{
		output_set_digit_value(2, out_digit);
	}
	if (m_led_selected&0x20)
	{
		output_set_digit_value(3, out_digit);
	}
}

READ8_MEMBER( fidelz80_state::fidelz80_portc_r )
{
	UINT8 data = 0xff;

	if (!(m_kp_matrix&0x10))
	{
		data &= ioport("LINE1")->read();
	}
	if (!(m_kp_matrix&0x20))
	{
		data &= ioport("LINE2")->read();
	}
	if (!(m_kp_matrix&0x40))
	{
		data &= ioport("LINE3")->read();
	}
	if (!(m_kp_matrix&0x80))
	{
		data &= ioport("LINE4")->read();
	}

	return data;
}

WRITE8_MEMBER( fidelz80_state::fidelz80_portb_w )
{
	if (!(data & 0x80))
	{
		m_led_data = (data&0x01);   // common for two leds

		m_led_selected = data;

		update_display();
	}

	// ignoring the language switch enable for now, is bit 0x40
}

WRITE8_MEMBER( fidelz80_state::fidelz80_portc_w )
{
	m_kp_matrix = data;
}

WRITE8_MEMBER( fidelz80_state::cc10_porta_w )
{
	m_beep->set_state((data & 0x80) ? 0 : 1);

	m_digit_data = data;

	update_display();
}

READ8_MEMBER( fidelz80_state::vcc_portb_r )
{
	return (m_speech->bsy_r() != 0) ? 0x80 : 0x00;
}

WRITE8_MEMBER( fidelz80_state::vcc_porta_w )
{
	m_speech->set_volume(15); // hack, s14001a core should assume a volume of 15 unless otherwise stated...
	m_speech->reg_w(data & 0x3f);
	m_speech->rst_w(BIT(data, 7));

	m_digit_data = data;

	update_display();
}

/******************************************************************************
    I8255 Device, for VSC
******************************************************************************/

WRITE8_MEMBER( fidelz80_state::vsc_porta_w )
{
	UINT8 out_digit = BITSWAP8( data,7,6,2,1,0,5,4,3 );

	if (m_kp_matrix & 0x01)
	{
		output_set_digit_value(0, out_digit & 0x7f);
		output_set_value("pm_led", BIT(out_digit, 7));
	}
	if (m_kp_matrix & 0x02)
	{
		output_set_digit_value(1, out_digit & 0x7f);
	}
	if (m_kp_matrix & 0x04)
	{
		output_set_digit_value(2, out_digit & 0x7f);
		output_set_value("up_dot", BIT(out_digit, 7));
	}
	if (m_kp_matrix & 0x08)
	{
		output_set_digit_value(3, out_digit & 0x7f);
		output_set_value("low_dot", BIT(out_digit, 7));
	}

	m_speech->reg_w(data & 0x3f);
}

WRITE8_MEMBER( fidelz80_state::vsc_portb_w )
{
	for (int row=1; row<=8; row++)
	{
		if (m_kp_matrix & 0x01)
			output_set_indexed_value("led_a", row, BIT(data, 8-row));
		if (m_kp_matrix & 0x02)
			output_set_indexed_value("led_b", row, BIT(data, 8-row));
		if (m_kp_matrix & 0x04)
			output_set_indexed_value("led_c", row, BIT(data, 8-row));
		if (m_kp_matrix & 0x08)
			output_set_indexed_value("led_d", row, BIT(data, 8-row));
		if (m_kp_matrix & 0x10)
			output_set_indexed_value("led_e", row, BIT(data, 8-row));
		if (m_kp_matrix & 0x20)
			output_set_indexed_value("led_f", row, BIT(data, 8-row));
		if (m_kp_matrix & 0x40)
			output_set_indexed_value("led_g", row, BIT(data, 8-row));
		if (m_kp_matrix & 0x80)
			output_set_indexed_value("led_h", row, BIT(data, 8-row));
	}
}

WRITE8_MEMBER( fidelz80_state::vsc_portc_w )
{
	m_kp_matrix = (m_kp_matrix & 0x300) | data;
}

/******************************************************************************
    PIO Device, for VSC
******************************************************************************/

READ8_MEMBER( fidelz80_state::vsc_pio_porta_r )
{
	UINT8 data = 0;

	if (m_kp_matrix & 0x01)
		data |= (ioport("COL_A")->read());
	if (m_kp_matrix & 0x02)
		data |= (ioport("COL_B")->read());
	if (m_kp_matrix & 0x04)
		data |= (ioport("COL_C")->read());
	if (m_kp_matrix & 0x08)
		data |= (ioport("COL_D")->read());
	if (m_kp_matrix & 0x10)
		data |= (ioport("COL_E")->read());
	if (m_kp_matrix & 0x20)
		data |= (ioport("COL_F")->read());
	if (m_kp_matrix & 0x40)
		data |= (ioport("COL_G")->read());
	if (m_kp_matrix & 0x80)
		data |= (ioport("COL_H")->read());
	if (m_kp_matrix & 0x100)
		data |= (ioport("COL_I")->read());
	if (m_kp_matrix & 0x200)
		data |= (ioport("COL_L")->read());

	return data & 0xff;
}

READ8_MEMBER( fidelz80_state::vsc_pio_portb_r )
{
	UINT8 data = 0x00;

	if (m_speech->bsy_r() == 0)
		data |= 0x10;

	return data;
}

WRITE8_MEMBER( fidelz80_state::vsc_pio_portb_w )
{
	m_kp_matrix = (m_kp_matrix & 0xff) | ((data & 0x03)<<8);

	m_speech->set_volume(15); // hack, s14001a core should assume a volume of 15 unless otherwise stated...
	m_speech->rst_w(BIT(data, 6));
}

/******************************************************************************
    I8041 MCU, for VBRC/7002 and bridgec3/7014
******************************************************************************/

WRITE8_MEMBER(fidelz80_state::kp_matrix_w)
{
	UINT16 out_data = BITSWAP16(m_digit_data,12,13,1,6,5,2,0,7,15,11,10,14,4,3,9,8);
	UINT16 out_digit = out_data & 0x3fff;
	UINT8 out_led = BIT(out_data, 15) ? 0 : 1;

	// output the digit before update the matrix
	if (m_kp_matrix & 0x01)
	{
		output_set_digit_value(1, out_digit);
		output_set_led_value(8, out_led);
	}
	if (m_kp_matrix & 0x02)
	{
		output_set_digit_value(2, out_digit);
		output_set_led_value(7, out_led);
	}
	if (m_kp_matrix & 0x04)
	{
		output_set_digit_value(3, out_digit);
		output_set_led_value(6, out_led);
	}
	if (m_kp_matrix & 0x08)
	{
		output_set_digit_value(4, out_digit);
		output_set_led_value(5, out_led);
	}
	if (m_kp_matrix & 0x10)
	{
		output_set_digit_value(5, out_digit);
		output_set_led_value(4, out_led);
	}
	if (m_kp_matrix & 0x20)
	{
		output_set_digit_value(6, out_digit);
		output_set_led_value(3, out_led);
	}
	if (m_kp_matrix & 0x40)
	{
		output_set_digit_value(7, out_digit);
		output_set_led_value(2, out_led);
	}
	if (m_kp_matrix & 0x80)
	{
		output_set_digit_value(8, out_digit);
		output_set_led_value(1, out_led);
	}

	memset(m_digit_line_status, 0, sizeof(m_digit_line_status));

	m_kp_matrix = data;
}

READ8_MEMBER(fidelz80_state::exp_i8243_p2_r)
{
	UINT8 data = 0xff;

	if (m_kp_matrix & 0x01)
		data &= ioport("LINE1")->read();
	if (m_kp_matrix & 0x02)
		data &= ioport("LINE2")->read();
	if (m_kp_matrix & 0x04)
		data &= ioport("LINE3")->read();
	if (m_kp_matrix & 0x08)
		data &= ioport("LINE4")->read();
	if (m_kp_matrix & 0x10)
		data &= ioport("LINE5")->read();
	if (m_kp_matrix & 0x20)
		data &= ioport("LINE6")->read();
	if (m_kp_matrix & 0x40)
		data &= ioport("LINE7")->read();
	if (m_kp_matrix & 0x80)
		data &= ioport("LINE8")->read();

	return (m_i8243->i8243_p2_r(space, offset)&0x0f) | (data&0xf0);
}

WRITE8_MEMBER(fidelz80_state::exp_i8243_p2_w)
{
	m_i8243->i8243_p2_w(space, offset, data&0x0f);
}

// probably related to the card scanner
READ8_MEMBER(fidelz80_state::unknown_r)
{
	return 0;
}

READ8_MEMBER(fidelz80_state::unknown2_r)
{
	return machine().rand();
}

/******************************************************************************
    I8243 expander
******************************************************************************/

WRITE8_MEMBER(fidelz80_state::digit_w)
{
	if (m_digit_line_status[offset])
		return;

	m_digit_line_status[offset&3] = 1;

	switch (offset)
	{
	case 0:
		m_digit_data = (m_digit_data&(~0x000f)) | ((data<<0)&0x000f);
		break;
	case 1:
		m_digit_data = (m_digit_data&(~0x00f0)) | ((data<<4)&0x00f0);
		break;
	case 2:
		m_digit_data = (m_digit_data&(~0x0f00)) | ((data<<8)&0x0f00);
		break;
	case 3:
		m_digit_data = (m_digit_data&(~0xf000)) | ((data<<12)&0xf000);
		break;
	}
}

/******************************************************************************
    basic machine
******************************************************************************/

WRITE8_MEMBER(fidelz80_state::mcu_data_w)
{
	m_i8041->upi41_master_w(space, 0, data);
}

WRITE8_MEMBER(fidelz80_state::mcu_command_w)
{
	m_i8041->upi41_master_w(space, 1, data);
}

READ8_MEMBER(fidelz80_state::mcu_data_r)
{
	return m_i8041->upi41_master_r(space, 0);
}

READ8_MEMBER(fidelz80_state::mcu_status_r)
{
	return m_i8041->upi41_master_r(space, 1);
}

WRITE8_MEMBER( fidelz80_state::bridgec_speech_w )
{
	// todo: HALT THE z80 here, and set up a callback to poll the s14001a DONE line to resume z80
	m_speech->set_volume(15); // hack, s14001a core should assume a volume of 15 unless otherwise stated...
	m_speech->reg_w(data & 0x3f);
	m_speech->rst_w(BIT(data, 7));
}

void fidelz80_state::machine_reset()
{
	m_led_selected = 0;
	m_kp_matrix = 0;
	m_digit_data = 0;
	m_led_data = 0;
	memset(m_digit_line_status, 0, sizeof(m_digit_line_status));
}

TIMER_DEVICE_CALLBACK_MEMBER(fidelz80_state::nmi_timer)
{
	m_maincpu->set_input_line(INPUT_LINE_NMI, PULSE_LINE);
}

/******************************************************************************
    Address Maps
******************************************************************************/

static ADDRESS_MAP_START(cc10_z80_mem, AS_PROGRAM, 8, fidelz80_state)
	ADDRESS_MAP_UNMAP_HIGH
	AM_RANGE(0x0000, 0x0fff) AM_ROM
	AM_RANGE(0x3000, 0x31ff) AM_RAM
ADDRESS_MAP_END

static ADDRESS_MAP_START(vcc_z80_mem, AS_PROGRAM, 8, fidelz80_state)
	ADDRESS_MAP_UNMAP_HIGH
	AM_RANGE(0x0000, 0x0fff) AM_ROM // 4k rom
	AM_RANGE(0x1000, 0x1fff) AM_ROM // 4k rom
	AM_RANGE(0x2000, 0x2fff) AM_ROM // 4k rom
	AM_RANGE(0x4000, 0x43ff) AM_RAM AM_MIRROR(0x1c00) // 1k ram (2114*2) mirrored 8 times
ADDRESS_MAP_END

static ADDRESS_MAP_START(vsc_mem, AS_PROGRAM, 8, fidelz80_state)
	ADDRESS_MAP_UNMAP_HIGH
	AM_RANGE(0x0000, 0x1fff) AM_ROM // 8k rom
	AM_RANGE(0x2000, 0x3fff) AM_ROM // 8k rom
	AM_RANGE(0x4000, 0x5fff) AM_ROM // 4k rom
	AM_RANGE(0x6000, 0x7fff) AM_RAM AM_MIRROR(0x1c00) // 1k ram (2114*2) mirrored 8 times
ADDRESS_MAP_END

static ADDRESS_MAP_START(bridgec_z80_mem, AS_PROGRAM, 8, fidelz80_state)
	ADDRESS_MAP_UNMAP_HIGH
	AM_RANGE(0x0000, 0x1fff) AM_ROM // 8k rom
	AM_RANGE(0x2000, 0x3fff) AM_ROM // 8k rom
	AM_RANGE(0x4000, 0x5fff) AM_ROM // 8k rom
	AM_RANGE(0x6000, 0x63ff) AM_RAM AM_MIRROR(0x1c00) // 1k ram (2114*2) mirrored 8 times
	AM_RANGE(0xE000, 0xE000) AM_WRITE(bridgec_speech_w) AM_MIRROR(0x1FFF) // write to speech chip, halts cpu
ADDRESS_MAP_END

static ADDRESS_MAP_START(fidel_z80_io, AS_IO, 8, fidelz80_state)
	ADDRESS_MAP_UNMAP_HIGH
	ADDRESS_MAP_GLOBAL_MASK(0xff)
	AM_RANGE(0x00, 0x03) AM_MIRROR(0xFC) AM_DEVREADWRITE("ppi8255", i8255_device, read, write) // 8255 i/o chip
ADDRESS_MAP_END

static ADDRESS_MAP_START(vsc_io, AS_IO, 8, fidelz80_state)
	ADDRESS_MAP_UNMAP_HIGH
	ADDRESS_MAP_GLOBAL_MASK(0xff)
	AM_RANGE(0x04, 0x07) AM_MIRROR(0xf0) AM_DEVREADWRITE("z80pio", z80pio_device, read, write)
	AM_RANGE(0x08, 0x0b) AM_MIRROR(0xf0) AM_DEVREADWRITE("ppi8255", i8255_device, read, write)
ADDRESS_MAP_END

static ADDRESS_MAP_START(bridgec_z80_io, AS_IO, 8, fidelz80_state)
	ADDRESS_MAP_UNMAP_HIGH
	ADDRESS_MAP_GLOBAL_MASK(0xff)
	AM_RANGE(0x00, 0x00) AM_READWRITE(mcu_data_r, mcu_data_w)
	AM_RANGE(0x01, 0x01) AM_READWRITE(mcu_status_r, mcu_command_w)
ADDRESS_MAP_END

static ADDRESS_MAP_START(bridgec_mcu_io, AS_IO, 8, fidelz80_state)
	ADDRESS_MAP_UNMAP_LOW
	AM_RANGE(MCS48_PORT_P1, MCS48_PORT_P1) AM_WRITE(kp_matrix_w)
	AM_RANGE(MCS48_PORT_P2, MCS48_PORT_P2) AM_READWRITE(exp_i8243_p2_r, exp_i8243_p2_w)
	AM_RANGE(MCS48_PORT_PROG, MCS48_PORT_PROG) AM_DEVWRITE("i8243", i8243_device, i8243_prog_w)

	// related to the card scanner, probably clock and data optical
	AM_RANGE(MCS48_PORT_T0, MCS48_PORT_T0) AM_READ(unknown_r)
	AM_RANGE(MCS48_PORT_T1, MCS48_PORT_T1) AM_READ(unknown2_r)
ADDRESS_MAP_END

/******************************************************************************
 Input Ports
******************************************************************************/

INPUT_CHANGED_MEMBER(fidelz80_state::fidelz80_trigger_reset)
{
	m_maincpu->set_input_line(INPUT_LINE_RESET, newval ? CLEAR_LINE : ASSERT_LINE);
}

INPUT_CHANGED_MEMBER(fidelz80_state::bridgec_trigger_reset)
{
	m_maincpu->set_input_line(INPUT_LINE_RESET, newval ? CLEAR_LINE : ASSERT_LINE);
	m_i8041->set_input_line(INPUT_LINE_RESET, newval ? CLEAR_LINE : ASSERT_LINE);
}

static INPUT_PORTS_START( fidelz80 )
	PORT_START("LEVEL")     // cc10 only
		PORT_CONFNAME( 0x80, 0x00, "Number of levels" )
		PORT_CONFSETTING( 0x00, "10" )
		PORT_CONFSETTING( 0x80, "3" )

	PORT_START("LINE1")
		PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYPAD) PORT_NAME("RE") PORT_CODE(KEYCODE_R) PORT_CHANGED_MEMBER(DEVICE_SELF, fidelz80_state, fidelz80_trigger_reset, 0)
		PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYPAD) PORT_NAME("LV") PORT_CODE(KEYCODE_V)
		PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYPAD) PORT_NAME("A1") PORT_CODE(KEYCODE_1) PORT_CODE(KEYCODE_A)
		PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYPAD) PORT_NAME("E5") PORT_CODE(KEYCODE_5) PORT_CODE(KEYCODE_E)

	PORT_START("LINE2")
		PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYPAD) PORT_NAME("CB") PORT_CODE(KEYCODE_Z)
		PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYPAD) PORT_NAME("DM") PORT_CODE(KEYCODE_M)
		PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYPAD) PORT_NAME("B2") PORT_CODE(KEYCODE_2) PORT_CODE(KEYCODE_B)
		PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYPAD) PORT_NAME("F6") PORT_CODE(KEYCODE_6) PORT_CODE(KEYCODE_F)

	PORT_START("LINE3")
		PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYPAD) PORT_NAME("CL") PORT_CODE(KEYCODE_DEL)
		PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYPAD) PORT_NAME("PB") PORT_CODE(KEYCODE_P)
		PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYPAD) PORT_NAME("C3") PORT_CODE(KEYCODE_3) PORT_CODE(KEYCODE_C)
		PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYPAD) PORT_NAME("G7") PORT_CODE(KEYCODE_7) PORT_CODE(KEYCODE_G)

	PORT_START("LINE4")
		PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYPAD) PORT_NAME("EN") PORT_CODE(KEYCODE_ENTER)
		PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYPAD) PORT_NAME("PV") PORT_CODE(KEYCODE_O)
		PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYPAD) PORT_NAME("D4") PORT_CODE(KEYCODE_4) PORT_CODE(KEYCODE_D)
		PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYPAD) PORT_NAME("H8") PORT_CODE(KEYCODE_8) PORT_CODE(KEYCODE_H)
INPUT_PORTS_END

static INPUT_PORTS_START( vsc )
	//chessboard buttons
	PORT_START("COL_A")
		PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYPAD)
		PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYPAD)
		PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYPAD)
		PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYPAD)
		PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_KEYPAD)
		PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_KEYPAD)
		PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_KEYPAD)
		PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_KEYPAD)
	PORT_START("COL_B")
		PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYPAD)
		PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYPAD)
		PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYPAD)
		PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYPAD)
		PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_KEYPAD)
		PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_KEYPAD)
		PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_KEYPAD)
		PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_KEYPAD)
	PORT_START("COL_C")
		PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYPAD)
		PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYPAD)
		PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYPAD)
		PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYPAD)
		PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_KEYPAD)
		PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_KEYPAD)
		PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_KEYPAD)
		PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_KEYPAD)
	PORT_START("COL_D")
		PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYPAD)
		PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYPAD)
		PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYPAD)
		PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYPAD)
		PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_KEYPAD)
		PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_KEYPAD)
		PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_KEYPAD)
		PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_KEYPAD)
	PORT_START("COL_E")
		PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYPAD)
		PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYPAD)
		PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYPAD)
		PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYPAD)
		PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_KEYPAD)
		PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_KEYPAD)
		PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_KEYPAD)
		PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_KEYPAD)
	PORT_START("COL_F")
		PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYPAD)
		PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYPAD)
		PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYPAD)
		PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYPAD)
		PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_KEYPAD)
		PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_KEYPAD)
		PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_KEYPAD)
		PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_KEYPAD)
	PORT_START("COL_G")
		PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYPAD)
		PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYPAD)
		PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYPAD)
		PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYPAD)
		PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_KEYPAD)
		PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_KEYPAD)
		PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_KEYPAD)
		PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_KEYPAD)
	PORT_START("COL_H")
		PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYPAD)
		PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYPAD)
		PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYPAD)
		PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYPAD)
		PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_KEYPAD)
		PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_KEYPAD)
		PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_KEYPAD)
		PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_KEYPAD)

	//buttons on the right
	PORT_START("COL_I")
		PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_NAME("Pawn")    PORT_CODE(KEYCODE_1)
		PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_NAME("Rook")    PORT_CODE(KEYCODE_2)
		PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_NAME("Knight")  PORT_CODE(KEYCODE_3)
		PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_NAME("Bishop")  PORT_CODE(KEYCODE_4)
		PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_NAME("Queen")   PORT_CODE(KEYCODE_5)
		PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_NAME("King")    PORT_CODE(KEYCODE_6)
		PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_NAME("CL")      PORT_CODE(KEYCODE_DEL)
		PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_NAME("RE")      PORT_CODE(KEYCODE_R)

	//buttons beside the display
	PORT_START("COL_L")
		PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_NAME("TM")      PORT_CODE(KEYCODE_T)
		PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_NAME("RV")      PORT_CODE(KEYCODE_V)
		PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_NAME("Speak")   PORT_CODE(KEYCODE_SPACE)
		PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_NAME("LV")      PORT_CODE(KEYCODE_L)
		PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_NAME("DM")      PORT_CODE(KEYCODE_M)
		PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_NAME("ST")      PORT_CODE(KEYCODE_S)
INPUT_PORTS_END

static INPUT_PORTS_START( bridgec )
	PORT_START("LINE1")
		PORT_BIT(0x0f, IP_ACTIVE_LOW, IPT_UNUSED) PORT_UNUSED
		PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYPAD) PORT_NAME("A") PORT_CODE(KEYCODE_A)
		PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYPAD) PORT_NAME("10") PORT_CODE(KEYCODE_0)
		PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYPAD) PORT_NAME("6") PORT_CODE(KEYCODE_6)
		PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYPAD) PORT_NAME("2") PORT_CODE(KEYCODE_2)

	PORT_START("LINE2")
		PORT_BIT(0x0f, IP_ACTIVE_LOW, IPT_UNUSED) PORT_UNUSED
		PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYPAD) PORT_NAME("K") PORT_CODE(KEYCODE_K)
		PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYPAD) PORT_NAME("9") PORT_CODE(KEYCODE_9)
		PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYPAD) PORT_NAME("5") PORT_CODE(KEYCODE_5)
		PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYPAD) PORT_NAME("1") PORT_CODE(KEYCODE_1)

	PORT_START("LINE3")
		PORT_BIT(0x0f, IP_ACTIVE_LOW, IPT_UNUSED) PORT_UNUSED
		PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYPAD) PORT_NAME("Q") PORT_CODE(KEYCODE_Q)
		PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYPAD) PORT_NAME("8") PORT_CODE(KEYCODE_8)
		PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYPAD) PORT_NAME("4") PORT_CODE(KEYCODE_4)
		PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYPAD) PORT_NAME("P") PORT_CODE(KEYCODE_Z)

	PORT_START("LINE4")
		PORT_BIT(0x0f, IP_ACTIVE_LOW, IPT_UNUSED) PORT_UNUSED
		PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYPAD) PORT_NAME("J") PORT_CODE(KEYCODE_J)
		PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYPAD) PORT_NAME("7") PORT_CODE(KEYCODE_7)
		PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYPAD) PORT_NAME("3") PORT_CODE(KEYCODE_3)
		PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYPAD) PORT_NAME("NT") PORT_CODE(KEYCODE_N)

	PORT_START("LINE5")
		PORT_BIT(0x0f, IP_ACTIVE_LOW, IPT_UNUSED) PORT_UNUSED
		PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYPAD) PORT_NAME("EN") PORT_CODE(KEYCODE_E)
		PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYPAD) PORT_NAME("SC") PORT_CODE(KEYCODE_S)
		PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYPAD) PORT_NAME("PL") PORT_CODE(KEYCODE_X)
		PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYPAD) PORT_NAME("Spades") PORT_CODE(KEYCODE_1_PAD)

	PORT_START("LINE6")
		PORT_BIT(0x0f, IP_ACTIVE_LOW, IPT_UNUSED) PORT_UNUSED
		PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYPAD) PORT_NAME("CL") PORT_CODE(KEYCODE_C)
		PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYPAD) PORT_NAME("DB") PORT_CODE(KEYCODE_D)
		PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYPAD) PORT_NAME("VL") PORT_CODE(KEYCODE_V)
		PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYPAD) PORT_NAME("Hearts") PORT_CODE(KEYCODE_2_PAD)

	PORT_START("LINE7")
		PORT_BIT(0x0f, IP_ACTIVE_LOW, IPT_UNUSED) PORT_UNUSED
		PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYPAD) PORT_NAME("Beep on/off") PORT_CODE(KEYCODE_SPACE)
		PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYPAD) PORT_NAME("PB") PORT_CODE(KEYCODE_B)
		PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYPAD) PORT_NAME("CV") PORT_CODE(KEYCODE_G)
		PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYPAD) PORT_NAME("Diamonds") PORT_CODE(KEYCODE_3_PAD)

	PORT_START("LINE8")
		PORT_BIT(0x0f, IP_ACTIVE_LOW, IPT_UNUSED) PORT_UNUSED
		PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYPAD) PORT_NAME("RE") PORT_CODE(KEYCODE_R) PORT_CHANGED_MEMBER(DEVICE_SELF, fidelz80_state, bridgec_trigger_reset, 0)
		PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYPAD) PORT_NAME("BR") PORT_CODE(KEYCODE_T)
		PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYPAD) PORT_NAME("DL") PORT_CODE(KEYCODE_L)
		PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYPAD) PORT_NAME("Clubs") PORT_CODE(KEYCODE_4_PAD)
INPUT_PORTS_END

/******************************************************************************
 Machine Drivers
******************************************************************************/

static MACHINE_CONFIG_START( cc10, fidelz80_state )
	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", Z80, XTAL_4MHz)
	MCFG_CPU_PROGRAM_MAP(cc10_z80_mem)
	MCFG_CPU_IO_MAP(fidel_z80_io)
	MCFG_QUANTUM_TIME(attotime::from_hz(60))

	/* video hardware */
	MCFG_DEFAULT_LAYOUT(layout_fidelz80)

	/* other hardware */
	MCFG_DEVICE_ADD("ppi8255", I8255, 0)
	MCFG_I8255_OUT_PORTA_CB(WRITE8(fidelz80_state, cc10_porta_w))
	MCFG_I8255_IN_PORTB_CB(IOPORT("LEVEL"))
	MCFG_I8255_OUT_PORTB_CB(WRITE8(fidelz80_state, fidelz80_portb_w))
	MCFG_I8255_IN_PORTC_CB(READ8(fidelz80_state, fidelz80_portc_r))
	MCFG_I8255_OUT_PORTC_CB(WRITE8(fidelz80_state, fidelz80_portc_w))

	/* sound hardware */
	MCFG_SPEAKER_STANDARD_MONO( "mono" )
	MCFG_SOUND_ADD( "beeper", BEEP, 0 )
	MCFG_SOUND_ROUTE( ALL_OUTPUTS, "mono", 1.00 )
MACHINE_CONFIG_END

static MACHINE_CONFIG_START( vcc, fidelz80_state )
	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", Z80, XTAL_4MHz)
	MCFG_CPU_PROGRAM_MAP(vcc_z80_mem)
	MCFG_CPU_IO_MAP(fidel_z80_io)
	MCFG_QUANTUM_TIME(attotime::from_hz(60))

	/* video hardware */
	MCFG_DEFAULT_LAYOUT(layout_fidelz80)

	/* other hardware */
	MCFG_DEVICE_ADD("ppi8255", I8255, 0)
	// Port A Read - NULL : only bit 6 is readable (and only sometimes) and I'm not emulating the language latch unless needed
	MCFG_I8255_OUT_PORTA_CB(WRITE8(fidelz80_state, vcc_porta_w))       // display segments and s14001a lines
	MCFG_I8255_IN_PORTB_CB(READ8(fidelz80_state, vcc_portb_r))         // bit 7 is readable and is the done line from the s14001a
	MCFG_I8255_OUT_PORTB_CB(WRITE8(fidelz80_state, fidelz80_portb_w))  // display digits and led dots
	MCFG_I8255_IN_PORTC_CB(READ8(fidelz80_state, fidelz80_portc_r))    // bits 0,1,2,3 are readable, have to do with input
	MCFG_I8255_OUT_PORTC_CB(WRITE8(fidelz80_state, fidelz80_portc_w))  // bits 4,5,6,7 are writable, have to do with input

	/* sound hardware */
	MCFG_SPEAKER_STANDARD_MONO("mono")
	MCFG_SOUND_ADD("speech", S14001A, 25000) // around 25khz
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.25)
MACHINE_CONFIG_END

static MACHINE_CONFIG_START( vsc, fidelz80_state )
	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", Z80, XTAL_4MHz)
	MCFG_CPU_PROGRAM_MAP(vsc_mem)
	MCFG_CPU_IO_MAP(vsc_io)

	MCFG_DEFAULT_LAYOUT(layout_vsc)

	/* other hardware */
	MCFG_DEVICE_ADD("ppi8255", I8255, 0)
	MCFG_I8255_OUT_PORTA_CB(WRITE8(fidelz80_state, vsc_porta_w))
	MCFG_I8255_OUT_PORTB_CB(WRITE8(fidelz80_state, vsc_portb_w))
	MCFG_I8255_OUT_PORTC_CB(WRITE8(fidelz80_state, vsc_portb_w))

	MCFG_DEVICE_ADD("z80pio", Z80PIO, XTAL_4MHz)
	MCFG_Z80PIO_IN_PA_CB(READ8(fidelz80_state, vsc_pio_porta_r))
	MCFG_Z80PIO_IN_PB_CB(READ8(fidelz80_state, vsc_pio_portb_r))
	MCFG_Z80PIO_OUT_PB_CB(WRITE8(fidelz80_state, vsc_pio_portb_w))

	MCFG_TIMER_DRIVER_ADD_PERIODIC("nmi_timer", fidelz80_state, nmi_timer, attotime::from_hz(600))
	MCFG_TIMER_START_DELAY(attotime::from_hz(600))

	/* sound hardware */
	MCFG_SPEAKER_STANDARD_MONO("mono")
	MCFG_SOUND_ADD("speech", S14001A, 25000) // around 25khz
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.25)
MACHINE_CONFIG_END

static MACHINE_CONFIG_START( bridgec, fidelz80_state )
	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", Z80, XTAL_5MHz/2) // 2.5MHz
	MCFG_CPU_PROGRAM_MAP(bridgec_z80_mem)
	MCFG_CPU_IO_MAP(bridgec_z80_io)
	MCFG_QUANTUM_TIME(attotime::from_hz(60))

	/* video hardware */
	MCFG_DEFAULT_LAYOUT(layout_bridgec3)

	/* other hardware */
	MCFG_CPU_ADD("mcu", I8041, XTAL_5MHz) // 5MHz
	MCFG_CPU_IO_MAP(bridgec_mcu_io)

	MCFG_I8243_ADD("i8243", NOOP, WRITE8(fidelz80_state,digit_w))

	/* sound hardware */
	MCFG_SPEAKER_STANDARD_MONO( "mono" )
	MCFG_SOUND_ADD("speech", S14001A, 25000) // around 25khz
	MCFG_SOUND_ROUTE( ALL_OUTPUTS, "mono", 1.00 )
MACHINE_CONFIG_END


/******************************************************************************
 ROM Definitions
******************************************************************************/

ROM_START( cc10 )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "cc10.bin",   0x0000, 0x1000, CRC(bb9e6055) SHA1(18276e57cf56465a6352239781a828c5f3d5ba63))
ROM_END

ROM_START(vcc)
	ROM_REGION(0x10000, "maincpu", 0)
	ROM_LOAD("101-32103.bin", 0x0000, 0x1000, CRC(257bb5ab) SHA1(f7589225bb8e5f3eac55f23e2bd526be780b38b5)) // 32014.VCC??? at location b3?
	ROM_LOAD("vcc2.bin", 0x1000, 0x1000, CRC(f33095e7) SHA1(692fcab1b88c910b74d04fe4d0660367aee3f4f0)) // at location a2?
	ROM_LOAD("vcc3.bin", 0x2000, 0x1000, CRC(624f0cd5) SHA1(7c1a4f4497fe5882904de1d6fecf510c07ee6fc6)) // at location a1?

	ROM_REGION(0x2000, "speech", 0)
	ROM_LOAD("vcc-engl.bin", 0x0000, 0x1000, CRC(f35784f9) SHA1(348e54a7fa1e8091f89ac656b4da22f28ca2e44d)) // at location c4?
ROM_END

ROM_START(uvc)
	ROM_REGION(0x10000, "maincpu", 0)
	ROM_LOAD("101-64017.b3", 0x0000, 0x2000, CRC(f1133abf) SHA1(09dd85051c4e7d364d43507c1cfea5c2d08d37f4)) // "MOS // 101-64017 // 3880"
	ROM_LOAD("101-32010.a1", 0x2000, 0x1000, CRC(624f0cd5) SHA1(7c1a4f4497fe5882904de1d6fecf510c07ee6fc6)) // "NEC P9Z021 // D2332C 228 // 101-32010", == vcc3.bin on vcc

	ROM_REGION(0x2000, "speech", 0)
	ROM_LOAD("101-32107.c4", 0x0000, 0x1000, CRC(f35784f9) SHA1(348e54a7fa1e8091f89ac656b4da22f28ca2e44d)) // "NEC P9Y019 // D2332C 229 // 101-32107", == vcc-engl.bin on vcc
ROM_END

ROM_START(vsc)
	ROM_REGION(0x10000, "maincpu", 0)
	ROM_LOAD("101-64108.bin", 0x0000, 0x2000, CRC(c9c98490) SHA1(e6db883df088d60463e75db51433a4b01a3e7626))
	ROM_LOAD("101-64109.bin", 0x2000, 0x2000, CRC(08a3577c) SHA1(69fe379d21a9d4b57c84c3832d7b3e7431eec341))
	ROM_LOAD("101-32024.bin", 0x4000, 0x1000, CRC(2a078676) SHA1(db2f0aba7e8ac0f84a17bae7155210cdf0813afb))

	ROM_REGION(0x2000, "speech", 0)
	ROM_LOAD("101-32107.bin", 0x0000, 0x1000, CRC(f35784f9) SHA1(348e54a7fa1e8091f89ac656b4da22f28ca2e44d))
ROM_END

ROM_START(vbrc) // AKA model 7002
	ROM_REGION(0x10000, "maincpu", 0)
	// nec 2364 mask roms; pin 27 (PGM, probably NC here due to mask roms) goes to the pcb
	ROM_LOAD("101-64108.g3", 0x0000, 0x2000, CRC(08472223) SHA1(859865b13c908dbb474333263dc60f6a32461141))
	ROM_LOAD("101-64109.f3", 0x2000, 0x2000, CRC(320afa0f) SHA1(90edfe0ac19b108d232cda376b03a3a24befad4c))
	ROM_LOAD("101-64110.e3", 0x4000, 0x2000, CRC(3040d0bd) SHA1(caa55fc8d9196e408fb41e7171a68e5099519813))

	ROM_REGION(0x1000, "mcu", 0)
	ROM_LOAD("100-1009.a3", 0x0000, 0x0400, CRC(60eb343f) SHA1(8a63e95ebd62e123bdecc330c0484a47c354bd1a))

	ROM_REGION(0x2000, "speech", 0)
	ROM_LOAD("101-32118.i2", 0x0000, 0x1000, CRC(a0b8bb8f) SHA1(f56852108928d5c6caccfc8166fa347d6760a740))
ROM_END

ROM_START(bridgec3) // 510-1016 Rev.1 PCB has neither locations nor ic labels, so I declare the big heatsink is at C1, numbers count on the shorter length of pcb
	ROM_REGION(0x10000, "maincpu", 0)
	// TMM2764AD-20 EPROMS with tiny hole-punch sized colored stickers (mostly) covering the quartz windows. pin 27 (PGM) is tied to vcc with small rework wires and does not connect to pcb.
	ROM_LOAD("7014_white.g3", 0x0000, 0x2000, CRC(eb1620ef) SHA1(987a9abc8c685f1a68678ea4ee65ec4a99419179)) // white sticker
	ROM_LOAD("7014_red.f3", 0x2000, 0x2000, CRC(74af0019) SHA1(8dc05950c254ca050b95b93e5d0cf48f913a6d49)) // red sticker
	ROM_LOAD("7014_blue.e3", 0x4000, 0x2000, CRC(341d9ca6) SHA1(370876573bb9408e75f4fc797304b6c64af0590a)) // blue sticker

	ROM_REGION(0x1000, "mcu", 0)
	ROM_LOAD("100-1009.a3", 0x0000, 0x0400, CRC(60eb343f) SHA1(8a63e95ebd62e123bdecc330c0484a47c354bd1a)) // "NEC P07021-027 || D8041C 563 100-1009"

	ROM_REGION(0x2000, "speech", 0)
	ROM_LOAD("101-32118.i2", 0x0000, 0x1000, CRC(a0b8bb8f) SHA1(f56852108928d5c6caccfc8166fa347d6760a740)) // "ea 101-32118 || (C) 1980 || EA 8332A247-4 || 8034"
ROM_END

/******************************************************************************
 Drivers
******************************************************************************/

/*    YEAR  NAME        PARENT      COMPAT  MACHINE     INPUT   INIT      COMPANY                     FULLNAME                                                    FLAGS */
COMP( 1978, cc10,       0,          0,      cc10,  fidelz80, driver_device, 0,      "Fidelity Electronics",   "Chess Challenger 10 (Model CC10/BCC)", MACHINE_NOT_WORKING )
COMP( 1979, vcc,        0,          0,      vcc,   fidelz80, driver_device, 0,      "Fidelity Electronics",   "Talking Chess Challenger (model VCC)", MACHINE_NOT_WORKING )
COMP( 1979, vbrc,       0,          0,      bridgec,   bridgec, driver_device,      0,      "Fidelity Electronics",   "Bridge Challenger (model VBRC/7002)",  MACHINE_NOT_WORKING )
COMP( 1980, uvc,        vcc,        0,      vcc,   fidelz80, driver_device, 0,      "Fidelity Electronics",   "Advanced Talking Chess Challenger (model UVC)", MACHINE_NOT_WORKING )
COMP( 1980, bridgec3,   vbrc,       0,      bridgec,   bridgec, driver_device,      0,      "Fidelity Electronics",   "Bridge Challenger 3 (model 7014)", MACHINE_NOT_WORKING )
COMP( 1980, vsc,        0,          0,      vsc,   vsc, driver_device,      0,      "Fidelity Electronics",   "Voice Sensory Chess Challenger (model VSC)", MACHINE_NOT_WORKING | MACHINE_CLICKABLE_ARTWORK )
