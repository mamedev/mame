/***********************************************************************************************************
  Barcrest MPU4 highly preliminary driver by J.Wallace, and Anonymous.

  This is the core driver, no video specific stuff should go in here.
  This driver holds all the mechanical games.

     06-2011: Fixed boneheaded interface glitch that was causing samples to not be cancelled correctly.
              Added the ability to read each segment of an LED display separately, this may be necessary for some
              games that use them as surrogate lamp lines.
              New persistence 'hack' to stop light flicker for the small extender.
     05-2011: Add better OKI emulation
     04-2011: More accurate gamball code, fixed ROM banking (Project Amber), added BwB CHR simulator (Amber)
              This is still a hard coded system, but significantly different to Barcrest's version.
              Started adding support for the Crystal Gaming program card, and the link keys for setting parameters.
     03-2011: Lamp timing fixes, support for all known expansion cards added.
     01-2011: Adding the missing 'OKI' sound card, and documented it, but it needs a 6376 rewrite.
     09-2007: Haze: Added Deal 'Em video support.
  03-08-2007: J Wallace: Removed audio filter for now, since sound is more accurate without them.
                         Connect 4 now has the right sound.
  03-07-2007: J Wallace: Several major changes, including input relabelling, and system timer improvements.
     06-2007: Atari Ace, many cleanups and optimizations of I/O routines
  09-06-2007: J Wallace: Fixed 50Hz detection circuit.
  17-02-2007: J Wallace: Added Deal 'Em - still needs some work.
  10-02-2007: J Wallace: Improved input timing.
  30-01-2007: J Wallace: Characteriser rewritten to run the 'extra' data needed by some games.
  24-01-2007: J Wallace: With thanks to Canonman and HIGHWAYMAN/System 80, I was able to confirm a seemingly
              ghastly misuse of a PIA is actually on the real hardware. This fixes the meters.

See http://agemame.mameworld.info/techinfo/mpu4.php for Information.

--- Board Setup ---

The MPU4 BOARD is the driver board, originally designed to run Fruit Machines made by the Barcrest Group, but later
licensed to other firms as a general purpose unit (even some old Photo-Me booths used the unit).

This board uses a ~1.72 Mhz 6809B CPU, and a number of PIA6821 chips for multiplexing inputs and the like.

To some extent, the hardware feels like a revision of the MPU3 design, integrating into the base unit features that were
previously added through expansion ports. However, there is no backwards compatibility, and the entire memory map has been
reworked.

Like MPU3, a 6840PTM is used for internal timing, and other miscellaneous control functions, including as a crude analogue sound device
(a square wave from the PTM being used as the alarm sound generator). However, the main sound functionality is provided by
dedicated hardware (an AY8913).

A MPU4 GAME CARD (cartridge) plugs into the MPU4 board containing the game, and a protection PAL (the 'characteriser').
This PAL, as well as protecting the games, also controlled some of the lamp address matrix for many games, and acted as
an anti-tampering device which helped to prevent the hacking of certain titles in a manner which broke UK gaming laws.

Like MPU3, over the years developers have added more capabilities through the spare inputs and outputs provided. These provided
support for more reels, lamps and LEDs through daughtercards.
Several solutions were released depending on the manufacturer of the machine, all are emulated here.

In later revisions of the main board (MOD4 onwards), the AY8913 was removed entirely, as two official alternatives for sound had been produced.
In one, a YM2413 is built into the gameboard, and in the other an OKI MSM6376 is interfaced with a PIA and PTM to allow sophisticated
sampled sound.

The lamping and input handling side of the machine rely entirely on a column by column 'strobe' system, with lights and LEDs selected in turn.
In the inputs there are two orange connectors (sampled every 8ms) and two black ones (sampled every 16ms), giving 32 multiplexed inputs.

In addition there are two auxiliary ports that can be accessed separately to these and are bidirectional

--- Preliminary MPU4 Memorymap  ---

(NV) indicates an item which is not present on the video version, which has a Comms card instead.

   hex     |r/w| D D D D D D D D |
 location  |   | 7 6 5 4 3 2 1 0 | function
-----------+---+-----------------+--------------------------------------------------------------------------
 0000-07FF |R/W| D D D D D D D D | 2k RAM
-----------+---+-----------------+--------------------------------------------------------------------------
 0800      |R/W|                 | Characteriser (Security PAL) (NV)
-----------+---+-----------------+--------------------------------------------------------------------------
 0850 ?    | W | ??????????????? | page latch (NV)
-----------+---+-----------------+--------------------------------------------------------------------------
 0880      |R/W| D D D D D D D D | PIA6821 on soundboard (Oki MSM6376 clocked by 6840 (8C0))
           |   |                 | port A = ??
           |   |                 | port B (882)
           |   |                 |        b7 = NAR
           |   |                 |        b6 = 0 if OKI busy, 1 if OKI ready
           |   |                 |        b5 = volume control clock
           |   |                 |        b4 = volume control direction (0= up, 1 = down)
           |   |                 |        b3 = ??
           |   |                 |        b2 = ??
           |   |                 |        b1 = 2ch
           |   |                 |        b0 = ST
-----------+---+-----------------+--------------------------------------------------------------------------
 08C0      |   |                 | MC6840 on sound board
-----------+---+-----------------+--------------------------------------------------------------------------
 0900-     |R/W| D D D D D D D D | MC6840 PTM IC2


  Clock1 <--------------------------------------
     |                                          |
     V                                          |
  Output1 ---> Clock2                           |
                                                |
               Output2 --+-> Clock3             |
                         |                      |
                         |   Output3 ---> 'to audio amp' ??
                         |
                         +--------> CA1 IC3 (

IRQ line connected to CPU

-----------+---+-----------------+--------------------------------------------------------------------------
 0A00-0A03 |R/W| D D D D D D D D | PIA6821 IC3 port A Lamp Drives 1,2,3,4,6,7,8,9 (sic)(IC14)
           |   |                 |
           |   |                 |          CA1 <= output2 from PTM6840 (IC2)
           |   |                 |          CA2 => alpha data
           |   |                 |
           |   |                 |          port B Lamp Drives 10,11,12,13,14,15,16,17 (sic)(IC13)
           |   |                 |
           |   |                 |          CB2 => alpha reset (clock on Dutch systems)
           |   |                 |
-----------+---+-----------------+--------------------------------------------------------------------------
 0B00-0B03 |R/W| D D D D D D D D | PIA6821 IC4 port A = data for 7seg leds (pins 10 - 17, via IC32)
           |   |                 |
           |   |                 |             CA1 INPUT, 50 Hz input (used to generate IRQ)
           |   |                 |             CA2 OUTPUT, connected to pin2 74LS138 CE for multiplexer
           |   |                 |                        (B on LED strobe multiplexer)
           |   |                 |             IRQA connected to IRQ of CPU
           |   |                 |             port B
           |   |                 |                    PB7 = INPUT, serial port Receive data (Rx)
           |   |                 |                    PB6 = INPUT, reel A sensor
           |   |                 |                    PB5 = INPUT, reel B sensor
           |   |                 |                    PB4 = INPUT, reel C sensor
           |   |                 |                    PB3 = INPUT, reel D sensor
           |   |                 |                    PB2 = INPUT, Connected to CA1 (50Hz signal)
           |   |                 |                    PB1 = INPUT, undercurrent sense
           |   |                 |                    PB0 = INPUT, overcurrent  sense
           |   |                 |
           |   |                 |             CB1 INPUT,  used to generate IRQ on edge of serial input line
           |   |                 |             CB2 OUTPUT, enable signal for reel optics
           |   |                 |             IRQB connected to IRQ of CPU
           |   |                 |
-----------+---+-----------------+--------------------------------------------------------------------------
 0C00-0C03 |R/W| D D D D D D D D | PIA6821 IC5 port A
           |   |                 |
           |   |                 |                    PA0-PA7, INPUT AUX1 connector
           |   |                 |
           |   |                 |             CA2  OUTPUT, serial port Transmit line
           |   |                 |             CA1  not connected
           |   |                 |             IRQA connected to IRQ of CPU
           |   |                 |
           |   |                 |             port B
           |   |                 |
           |   |                 |                    PB0-PB7 INPUT, AUX2 connector
           |   |                 |
           |   |                 |             CB1  INPUT,  connected to PB7 (Aux2 connector pin 4)
           |   |                 |
           |   |                 |             CB2  OUTPUT, AY8913 chip select line
           |   |                 |             IRQB connected to IRQ of CPU
           |   |                 |
-----------+---+-----------------+--------------------------------------------------------------------------
 0D00-0D03 |R/W| D D D D D D D D | PIA6821 IC6
           |   |                 |
           |   |                 |  port A
           |   |                 |
           |   |                 |        PA0 - PA7 (INPUT/OUTPUT) data port AY8913 sound chip
           |   |                 |
           |   |                 |        CA1 INPUT,  not connected
           |   |                 |        CA2 OUTPUT, BC1 pin AY8913 sound chip
           |   |                 |        IRQA , connected to IRQ CPU
           |   |                 |
           |   |                 |  port B
           |   |                 |
           |   |                 |        PB0-PB3 OUTPUT, reel A
           |   |                 |        PB4-PB7 OUTPUT, reel B
           |   |                 |
           |   |                 |        CB1 INPUT,  not connected
           |   |                 |        CB2 OUTPUT, B01R pin AY8913 sound chip
           |   |                 |        IRQB , connected to IRQ CPU
           |   |                 |
-----------+---+-----------------+--------------------------------------------------------------------------
 0E00-0E03 |R/W| D D D D D D D D | PIA6821 IC7
           |   |                 |
           |   |                 |  port A
           |   |                 |
           |   |                 |        PA0-PA3 OUTPUT, reel C
           |   |                 |        PA4-PA7 OUTPUT, reel D
           |   |                 |        CA1     INPUT,  not connected
           |   |                 |        CA2     OUTPUT, A on LED strobe multiplexer
           |   |                 |        IRQA , connected to IRQ CPU
           |   |                 |
           |   |                 |  port B
           |   |                 |
           |   |                 |        PB0-PB6 OUTPUT mech meter 1-7 or reel E + F
           |   |                 |        PB7     Voltage drop sensor
           |   |                 |        CB1     INPUT, not connected
           |   |                 |        CB2     OUTPUT,mech meter 8
           |   |                 |        IRQB , connected to IRQ CPU
           |   |                 |
-----------+---+-----------------+--------------------------------------------------------------------------
 0F00-0F03 |R/W| D D D D D D D D | PIA6821 IC8
           |   |                 |
           |   |                 | port A
           |   |                 |
           |   |                 |        PA0-PA7 INPUT  multiplexed inputs data
           |   |                 |
           |   |                 |        CA1     INPUT, not connected
           |   |                 |        CA2    OUTPUT, C on LED strobe multiplexer
           |   |                 |        IRQA           connected to IRQ CPU
           |   |                 |
           |   |                 | port B
           |   |                 |
           |   |                 |        PB0-PB7 OUTPUT  triacs outputs connector PL6
           |   |                 |        used for slides / hoppers
           |   |                 |
           |   |                 |        CB1     INPUT, not connected
           |   |                 |        CB2    OUTPUT, pin1 alpha display PL7 (clock signal)
           |   |                 |        IRQB           connected to IRQ CPU
           |   |                 |
-----------+---+-----------------+--------------------------------------------------------------------------
 1000-FFFF | R | D D D D D D D D | ROM (can be bank switched by 0x850 in 8 banks of 64 k ) (NV)
-----------+---+-----------------+--------------------------------------------------------------------------

TODO: - Distinguish door switches using manual
      - Complete stubs for hoppers (needs slightly better 68681 emulation, and new 'hoppers' device emulation)
      - It seems that the MPU4 core program relies on some degree of persistence when switching strobes and handling
      writes to the various hardware ports. This explains the occasional lamping/LED blackout and switching bugs
      For now, we're ignoring any extra writes to strobes, as the alternative is to assign a timer to *everything* and
      start modelling the individual hysteresis curves of filament lamps.
      - Fix BwB characteriser, need to be able to calculate stabiliser bytes. Anyone fancy reading 6809 source?
      - Strange bug in Andy's Great Escape - Mystery nudge sound effect is not played, mpu4 latches in silence instead (?)
***********************************************************************************************************/
#include "emu.h"
#include "machine/6821pia.h"
#include "machine/6840ptm.h"
#include "machine/nvram.h"

#include "cpu/m6809/m6809.h"
#include "sound/ay8910.h"
#include "sound/okim6376.h"
#include "sound/2413intf.h"
#include "sound/upd7759.h"
#include "machine/steppers.h"
#include "machine/roc10937.h"
#include "machine/meters.h"
#include "includes/mpu4.h"


#include "video/awpvid.h"		//Fruit Machines Only
#include "connect4.lh"
#include "gamball.lh"
#include "mpu4.lh"
#include "mpu4ext.lh"


static TIMER_CALLBACK( ic24_timeout );


/*
LED Segments related to pins (5 is not connected):
Unlike the controllers emulated in the layout code, each
segment of an MPU4 LED can be set individually, even
being used as individual lamps. However, we can get away
with settings like this in the majority of cases.
   _9_
  |   |
  3   8
  |   |
   _2_
  |   |
  4   7
  |_ _|
    6  1

8 display enables (pins 10 - 17)
*/

static void lamp_extend_small(mpu4_state *state, int data)
{
	int lamp_ext_data,column,i;
	column = data & 0x07;

	lamp_ext_data = 0x1f - ((data & 0xf8) >> 3);//remove the mux lines from the data

	if ((state->m_lamp_strobe_ext_persistence == 0))
	//One write to reset the drive lines, one with the data, one to clear the lines, so only the 2nd write does anything
	//Once again, lamp persistences would take care of this, but we can't do that
	{
		for (i = 0; i < 5; i++)
		{
			output_set_lamp_value((8*column)+i+128,((lamp_ext_data  & (1 << i)) != 0));
		}
	}
	state->m_lamp_strobe_ext_persistence ++;
	if ((state->m_lamp_strobe_ext_persistence == 3)||(state->m_lamp_strobe_ext!=column))
	{
		state->m_lamp_strobe_ext_persistence = 0;
		state->m_lamp_strobe_ext=column;
	}
}

static void lamp_extend_large(mpu4_state *state, int data,int column,int active)
{
	int lampbase,i,bit7;

	state->m_lamp_sense = 0;
	bit7 = data & 0x80;
	if ( bit7 != state->m_last_b7 )
	{
		state->m_card_live = 1;
		//depending on bit 7, we can access one of two 'blocks' of 64 lamps
		lampbase = bit7 ? 0 : 64;
		if ( data & 0x3f )
		{
			state->m_lamp_sense = 1;
		}
		if ( active )
		{
			if (state->m_lamp_strobe_ext != column)
			{
				for (i = 0; i < 8; i++)
				{//CHECK, this includes bit 7
					output_set_lamp_value((8*column)+i+128+lampbase ,(data  & (1 << i)) != 0);
				}
				state->m_lamp_strobe_ext = column;
			}
		}
	    state->m_last_b7 = bit7;
	}
	else
	{
		state->m_card_live = 0;
	}
}

static void led_write_latch(mpu4_state *state, int latch, int data, int column)
{
	int diff,i,j;

	diff = (latch ^ state->m_last_latch) & latch;
	column = 7 - column; // like main board, these are wired up in reverse
	data = ~data;//inverted drive lines?

	for(i=0; i<5; i++)
	{
		if (diff & (1<<i))
		{
			column += i;
		}
	}
	for(j=0; j<8; j++)
	{
		output_set_indexed_value("mpu4led",(8*column)+j,(data & (1 << j)) !=0);
	}
	output_set_digit_value(column * 8, data);

	state->m_last_latch = diff;
}


static void update_meters(mpu4_state *state)
{
	int meter;
	int data = ((state->m_mmtr_data & 0x7f) | state->m_remote_meter);
	switch (state->m_reel_mux)
	{
		case STANDARD_REEL:
		// Change nothing
		break;
		case FIVE_REEL_5TO8:
		stepper_update(4, ((data >> 4) & 0x0f));
		data = (data & 0x0F); //Strip reel data from meter drives, leaving active elements
		awp_draw_reel(4);
		break;
		case FIVE_REEL_8TO5:
		stepper_update(4, (((data & 0x01) + ((data & 0x08) >> 2) + ((data & 0x20) >> 3) + ((data & 0x80) >> 4)) & 0x0f)) ;
		data = 0x00; //Strip all reel data from meter drives, nothing is connected
		awp_draw_reel(4);
		break;
		case FIVE_REEL_3TO6:
		stepper_update(4, ((data >> 2) & 0x0f));
		data = 0x00; //Strip all reel data from meter drives
		awp_draw_reel(4);
		break;
		case SIX_REEL_1TO8:
		stepper_update(4, (data & 0x0f));
		stepper_update(5, ((data >> 4) & 0x0f));
		data = 0x00; //Strip all reel data from meter drives
		awp_draw_reel(4);
		awp_draw_reel(5);
		break;
		case SIX_REEL_5TO8:
		stepper_update(4, ((data >> 4) & 0x0f));
		data = 0x00; //Strip all reel data from meter drives
		awp_draw_reel(4);
		break;
		case SEVEN_REEL:
		stepper_update(0, (((data & 0x01) + ((data & 0x08) >> 2) + ((data & 0x20) >> 3) + ((data & 0x80) >> 4)) & 0x0f)) ;
		data = 0x00; //Strip all reel data from meter drives
		awp_draw_reel(0);
		break;
		case FLUTTERBOX: //The backbox fan assembly fits in a reel unit sized box, wired to the remote meter pin, so we can handle it here
		output_set_value("flutterbox", data & 0x80);
		data &= ~0x80; //Strip flutterbox data from meter drives
	}

	MechMtr_update(7, (data & 0x80));
	for (meter = 0; meter < 4; meter ++)
	{
		MechMtr_update(meter, (data & (1 << meter)));
	}
	if (state->m_reel_mux == STANDARD_REEL)
	{
		for (meter = 4; meter < 7; meter ++)
		{
			MechMtr_update(meter, (data & (1 << meter)));
		}
	}
}

/* called if board is reset */
void mpu4_stepper_reset(mpu4_state *state)
{
	int pattern = 0,reel;
	for (reel = 0; reel < 6; reel++)
	{
		stepper_reset_position(reel);
		if(!state->m_reel_mux)
		{
			if (stepper_optic_state(reel)) pattern |= 1<<reel;
		}
	}
	state->m_optic_pattern = pattern;
}


static MACHINE_RESET( mpu4 )
{
	mpu4_state *state = machine.driver_data<mpu4_state>();
	ROC10937_reset(0);	/* reset display1 */

	mpu4_stepper_reset(state);

	state->m_lamp_strobe    = 0;
	state->m_lamp_strobe2   = 0;
	state->m_led_strobe     = 0;
	state->m_mmtr_data      = 0;
	state->m_remote_meter   = 0;

	state->m_IC23GC    = 0;
	state->m_IC23GB    = 0;
	state->m_IC23GA    = 0;
	state->m_IC23G1    = 1;
	state->m_IC23G2A   = 0;
	state->m_IC23G2B   = 0;

	state->m_prot_col  = 0;
	state->m_chr_counter    = 0;
	state->m_chr_value		= 0;


	/* init rom bank, some games don't set this, and will assume bank 0,set 0 */
	{
		UINT8 *rom = machine.region("maincpu")->base();

		memory_configure_bank(machine, "bank1", 0, 8, &rom[0x01000], 0x10000);

		memory_set_bank(machine, "bank1",0);
		machine.device("maincpu")->reset();
	}
}


/* 6809 IRQ handler */
WRITE_LINE_DEVICE_HANDLER( cpu0_irq )
{
	mpu4_state *drvstate = device->machine().driver_data<mpu4_state>();
	pia6821_device *pia3 = device->machine().device<pia6821_device>("pia_ic3");
	pia6821_device *pia4 = device->machine().device<pia6821_device>("pia_ic4");
	pia6821_device *pia5 = device->machine().device<pia6821_device>("pia_ic5");
	pia6821_device *pia6 = device->machine().device<pia6821_device>("pia_ic6");
	pia6821_device *pia7 = device->machine().device<pia6821_device>("pia_ic7");
	pia6821_device *pia8 = device->machine().device<pia6821_device>("pia_ic8");
	ptm6840_device *ptm2 = device->machine().device<ptm6840_device>("ptm_ic2");

	/* The PIA and PTM IRQ lines are all connected to a common PCB track, leading directly to the 6809 IRQ line. */
	int combined_state = pia3->irq_a_state() | pia3->irq_b_state() |
						 pia4->irq_a_state() | pia4->irq_b_state() |
						 pia5->irq_a_state() | pia5->irq_b_state() |
						 pia6->irq_a_state() | pia6->irq_b_state() |
						 pia7->irq_a_state() | pia7->irq_b_state() |
						 pia8->irq_a_state() | pia8->irq_b_state() |
						 ptm2->irq_state();

	if (!drvstate->m_link7a_connected) //7B = IRQ, 7A = FIRQ, both = NMI
	{
		cputag_set_input_line(device->machine(), "maincpu", M6809_IRQ_LINE, combined_state ? ASSERT_LINE : CLEAR_LINE);
		LOG(("6809 int%d \n", combined_state));
	}
	else
	{
		cputag_set_input_line(device->machine(), "maincpu", M6809_FIRQ_LINE, combined_state ? ASSERT_LINE : CLEAR_LINE);
		LOG(("6809 fint%d \n", combined_state));
	}
}

/* Bankswitching
The MOD 4 ROM cards are set up to handle 8 separate ROM pages, arranged as 2 sets of 4.
The bankswitch selects which of the 4 pages in the set is active, while the bankset
switches between the sets.
It appears that the cards were originally intended to be used in a 'half' page setup,
where the two halves of the ROM space could be mixed and matched as appropriate.
However, there is no evidence to suggest this was ever implemented.
The controls for it exist however, in the form of the Soundboard PIA CB2 pin, which is
used in some cabinets instead of the main control.
*/
static WRITE8_HANDLER( bankswitch_w )
{
	mpu4_state *state = space->machine().driver_data<mpu4_state>();
	state->m_pageval=(data&0x03);
	memory_set_bank(space->machine(), "bank1",state->m_pageval + (state->m_pageset?4:0));
}


static READ8_HANDLER( bankswitch_r )
{
	return memory_get_bank(space->machine(), "bank1");
}


static WRITE8_HANDLER( bankset_w )
{
	mpu4_state *state = space->machine().driver_data<mpu4_state>();
	state->m_pageval=(data - 2);//writes 2 and 3, to represent 0 and 1 - a hangover from the half page design?
	memory_set_bank(space->machine(), "bank1",state->m_pageval + (state->m_pageset?4:0));
}


/* IC2 6840 PTM handler */
static WRITE8_DEVICE_HANDLER( ic2_o1_callback )
{
	downcast<ptm6840_device *>(device)->set_c2(data);

	/* copy output value to IC2 c2
    this output is the clock for timer2 */
	/* 1200Hz System interrupt timer */
}


static WRITE8_DEVICE_HANDLER( ic2_o2_callback )
{
	pia6821_device *pia = device->machine().device<pia6821_device>("pia_ic3");
	pia->ca1_w(data); /* copy output value to IC3 ca1 */
	/* the output from timer2 is the input clock for timer3 */
	/* miscellaneous interrupts generated here */
	downcast<ptm6840_device *>(device)->set_c3(data);
}


static WRITE8_DEVICE_HANDLER( ic2_o3_callback )
{
	/* the output from timer3 is used as a square wave for the alarm output
    and as an external clock source for timer 1! */
	/* also runs lamp fade */
	downcast<ptm6840_device *>(device)->set_c1(data);
}


static const ptm6840_interface ptm_ic2_intf =
{
	MPU4_MASTER_CLOCK / 4,
	{ 0, 0, 0 },
	{ DEVCB_HANDLER(ic2_o1_callback),
	  DEVCB_HANDLER(ic2_o2_callback),
	  DEVCB_HANDLER(ic2_o3_callback) },
	DEVCB_LINE(cpu0_irq)
};


/* 6821 PIA handlers */
/* IC3, lamp data lines + alpha numeric display */
static WRITE8_DEVICE_HANDLER( pia_ic3_porta_w )
{
	mpu4_state *state = device->machine().driver_data<mpu4_state>();
	int i;
	LOG_IC3(("%s: IC3 PIA Port A Set to %2x (lamp strobes 1 - 9)\n", device->machine().describe_context(),data));

	if(state->m_ic23_active)
	{
		if (state->m_lamp_strobe != state->m_input_strobe)
		{
			// Because of the nature of the lamping circuit, there is an element of persistance
			// As a consequence, the lamp column data can change before the input strobe without
			// causing the relevant lamps to black out.

			for (i = 0; i < 8; i++)
			{
				output_set_lamp_value((8*state->m_input_strobe)+i, ((data  & (1 << i)) !=0));
			}
			state->m_lamp_strobe = state->m_input_strobe;
		}
	}
}

static WRITE8_DEVICE_HANDLER( pia_ic3_portb_w )
{
	mpu4_state *state = device->machine().driver_data<mpu4_state>();
	int i;
	LOG_IC3(("%s: IC3 PIA Port B Set to %2x  (lamp strobes 10 - 17)\n", device->machine().describe_context(),data));

	if(state->m_ic23_active)
	{
		if (state->m_lamp_strobe2 != state->m_input_strobe)
		{
			for (i = 0; i < 8; i++)
			{
				output_set_lamp_value((8*state->m_input_strobe)+i+64, ((data  & (1 << i)) !=0));
			}
			state->m_lamp_strobe2 = state->m_input_strobe;
		}

		if (state->m_led_lamp)
		{
			/* Some games (like Connect 4) use 'programmable' LED displays, built from light display lines in section 2. */
			/* These are mostly low-tech machines, where such wiring proved cheaper than an extender card */
			UINT8 pled_segs[2] = {0,0};

			static const int lamps1[8] = { 106, 107, 108, 109, 104, 105, 110, 133 };
			static const int lamps2[8] = { 114, 115, 116, 117, 112, 113, 118, 119 };

			for (i = 0; i < 8; i++)
			{
				if (output_get_lamp_value(lamps1[i])) pled_segs[0] |= (1 << i);
				if (output_get_lamp_value(lamps2[i])) pled_segs[1] |= (1 << i);
			}

			output_set_digit_value(8,pled_segs[0]);
			output_set_digit_value(9,pled_segs[1]);
		}
	}
}

static WRITE_LINE_DEVICE_HANDLER( pia_ic3_ca2_w )
{
	mpu4_state *drvstate = device->machine().driver_data<mpu4_state>();
	LOG_IC3(("%s: IC3 PIA Write CA2 (alpha data), %02X\n", device->machine().describe_context(),state));

	drvstate->m_alpha_data_line = state;
}


static WRITE_LINE_DEVICE_HANDLER( pia_ic3_cb2_w )
{
	LOG_IC3(("%s: IC3 PIA Write CB (alpha reset), %02X\n",device->machine().describe_context(),state));
// DM Data pin A
	if ( !state )
	{
		ROC10937_reset(0);
	}
}


static const pia6821_interface pia_ic3_intf =
{
	DEVCB_NULL,		/* port A in */
	DEVCB_NULL,		/* port B in */
	DEVCB_NULL,		/* line CA1 in */
	DEVCB_NULL,		/* line CB1 in */
	DEVCB_NULL,		/* line CA2 in */
	DEVCB_NULL,		/* line CB2 in */
	DEVCB_HANDLER(pia_ic3_porta_w),		/* port A out */
	DEVCB_HANDLER(pia_ic3_portb_w),		/* port B out */
	DEVCB_LINE(pia_ic3_ca2_w),			/* line CA2 out */
	DEVCB_LINE(pia_ic3_cb2_w),			/* port CB2 out */
	DEVCB_LINE(cpu0_irq),				/* IRQA */
	DEVCB_LINE(cpu0_irq)				/* IRQB */
};


/*
IC23 emulation

IC23 is a 74LS138 1-of-8 Decoder

It is used as a multiplexer for the LEDs, lamp selects and inputs.*/

static void ic23_update(mpu4_state *state)
{
	if (!state->m_IC23G2A)
	{
		if (!state->m_IC23G2B)
		{
			if (state->m_IC23G1)
			{
				if ( state->m_IC23GA ) state->m_input_strobe |= 0x01;
				else				 state->m_input_strobe &= ~0x01;

				if ( state->m_IC23GB ) state->m_input_strobe |= 0x02;
				else		         state->m_input_strobe &= ~0x02;

				if ( state->m_IC23GC ) state->m_input_strobe |= 0x04;
				else				 state->m_input_strobe &= ~0x04;
			}
		}
	}
	else
	if ((state->m_IC23G2A)||(state->m_IC23G2B))
	{
		state->m_input_strobe = 0x00;
	}
}


/*
IC24 emulation

IC24 is a 74LS122 pulse generator

CLEAR and B2 are tied high and A1 and A2 tied low, meaning any pulse
on B1 will give a low pulse on the output pin.
*/
static void ic24_output(mpu4_state *state, int data)
{
	state->m_IC23G2A = data;
	ic23_update(state);
}


static void ic24_setup(mpu4_state *state)
{
	if (state->m_IC23GA)
	{
		double duration = TIME_OF_74LS123((220*1000),(0.1*0.000001));
		{
			state->m_ic23_active=1;
			ic24_output(state, 0);
			state->m_ic24_timer->adjust(attotime::from_double(duration));
		}
	}
}


static TIMER_CALLBACK( ic24_timeout )
{
	mpu4_state *state = machine.driver_data<mpu4_state>();
	state->m_ic23_active=0;
	ic24_output(state, 1);
}


/* IC4, 7 seg leds, 50Hz timer reel sensors, current sensors */
static WRITE8_DEVICE_HANDLER( pia_ic4_porta_w )
{
	int i;
	mpu4_state *state = device->machine().driver_data<mpu4_state>();
	if(state->m_ic23_active)
	{
		if (((state->m_lamp_extender == NO_EXTENDER)||(state->m_lamp_extender == SMALL_CARD)||(state->m_lamp_extender == LARGE_CARD_C))&& (state->m_led_extender == NO_EXTENDER))
		{
			if(state->m_led_strobe != state->m_input_strobe)
			{
				for(i=0; i<8; i++)
				{
					output_set_indexed_value("mpu4led",((7 - state->m_input_strobe) * 8) +i,(data & (1 << i)) !=0);
				}
				output_set_digit_value(7 - state->m_input_strobe,data);
			}
			state->m_led_strobe = state->m_input_strobe;
		}
	}
}

static WRITE8_DEVICE_HANDLER( pia_ic4_portb_w )
{
	mpu4_state *state = device->machine().driver_data<mpu4_state>();
	if (state->m_reel_mux)
	{
		/* A write here connects one reel (and only one)
        to the optic test circuit. This allows 8 reels
        to be supported instead of 4. */
		if (state->m_reel_mux == SEVEN_REEL)
		{
			state->m_active_reel= reel_mux_table7[(data >> 4) & 0x07];
		}
		else
		state->m_active_reel= reel_mux_table[(data >> 4) & 0x07];
	}
}

static READ8_DEVICE_HANDLER( pia_ic4_portb_r )
{
	mpu4_state *state = device->machine().driver_data<mpu4_state>();
	pia6821_device *pia = downcast<pia6821_device *>(device);
	if ( state->m_serial_data )
	{
		state->m_ic4_input_b |=  0x80;
		pia->cb1_w(1);
	}
	else
	{
		state->m_ic4_input_b &= ~0x80;
		pia->cb1_w(0);
	}

	if (!state->m_reel_mux)
	{
		if ( state->m_optic_pattern & 0x01 ) state->m_ic4_input_b |=  0x40; /* reel A tab */
		else							   state->m_ic4_input_b &= ~0x40;

		if ( state->m_optic_pattern & 0x02 ) state->m_ic4_input_b |=  0x20; /* reel B tab */
		else							   state->m_ic4_input_b &= ~0x20;

		if ( state->m_optic_pattern & 0x04 ) state->m_ic4_input_b |=  0x10; /* reel C tab */
		else							   state->m_ic4_input_b &= ~0x10;

		if ( state->m_optic_pattern & 0x08 ) state->m_ic4_input_b |=  0x08; /* reel D tab */
		else							   state->m_ic4_input_b &= ~0x08;

	}
	else
	{
		if (stepper_optic_state(state->m_active_reel))
		{
			state->m_ic4_input_b |=  0x08;
		}
		else
		{
			state->m_ic4_input_b &= ~0x08;
		}
	}
	if ( state->m_signal_50hz )			state->m_ic4_input_b |=  0x04; /* 50 Hz */
	else								state->m_ic4_input_b &= ~0x04;

	if (state->m_ic4_input_b & 0x02)
	{
		state->m_ic4_input_b &= ~0x02;
	}
	else
	{
		state->m_ic4_input_b |= 0x02; //Pulse the overcurrent line with every read to show the CPU each lamp has lit
	}
	#ifdef UNUSED_FUNCTION
	if ( lamp_undercurrent ) state->m_ic4_input_b |= 0x01;
	#endif

	LOG_IC3(("%s: IC4 PIA Read of Port B %x\n",device->machine().describe_context(),state->m_ic4_input_b));
	return state->m_ic4_input_b;
}


static WRITE_LINE_DEVICE_HANDLER( pia_ic4_ca2_w )
{
	mpu4_state *drvstate = device->machine().driver_data<mpu4_state>();
	LOG_IC3(("%s: IC4 PIA Write CA (input MUX strobe /LED B), %02X\n", device->machine().describe_context(),state));

	drvstate->m_IC23GB = state;
	ic23_update(drvstate);
}

static WRITE_LINE_DEVICE_HANDLER( pia_ic4_cb2_w )
{
	mpu4_state *drvstate = device->machine().driver_data<mpu4_state>();
	LOG_IC3(("%s: IC4 PIA Write CA (input MUX strobe /LED B), %02X\n", device->machine().describe_context(),state));
	drvstate->m_reel_flag=state;
}
static const pia6821_interface pia_ic4_intf =
{
	DEVCB_NULL,		/* port A in */
	DEVCB_HANDLER(pia_ic4_portb_r),	/* port B in */
	DEVCB_NULL,		/* line CA1 in */
	DEVCB_NULL,		/* line CB1 in */
	DEVCB_NULL,		/* line CA2 in */
	DEVCB_NULL,		/* line CB2 in */
	DEVCB_HANDLER(pia_ic4_porta_w),		/* port A out */
	DEVCB_HANDLER(pia_ic4_portb_w),		/* port B out */
	DEVCB_LINE(pia_ic4_ca2_w),		/* line CA2 out */
	DEVCB_LINE(pia_ic4_cb2_w),		/* line CB2 out */
	DEVCB_LINE(cpu0_irq),		/* IRQA */
	DEVCB_LINE(cpu0_irq)		/* IRQB */
};

/* IC5, AUX ports, coin lockouts and AY sound chip select (MODs below 4 only) */
static READ8_DEVICE_HANDLER( pia_ic5_porta_r )
{
	mpu4_state *state = device->machine().driver_data<mpu4_state>();
	if (state->m_lamp_extender == LARGE_CARD_A)
	{
		if (state->m_lamp_sense && state->m_ic23_active)
		{
			state->m_aux1_input |= 0x40;
		}
		else
		{
			state->m_aux1_input &= ~0x40; //Pulse the overcurrent line with every read to show the CPU each lamp has lit
		}
	}
	if (state->m_hopper == HOPPER_NONDUART_A)
	{
/*      if (hopper1_active)
        {
            state->m_aux1_input |= 0x04;
        }
        else
        {
            state->m_aux1_input &= ~0x04;
        }*/
	}
	LOG(("%s: IC5 PIA Read of Port A (AUX1)\n",device->machine().describe_context()));

	return input_port_read(device->machine(), "AUX1")|state->m_aux1_input;
}

static WRITE8_DEVICE_HANDLER( pia_ic5_porta_w )
{
	int i;
	mpu4_state *state = device->machine().driver_data<mpu4_state>();
	pia6821_device *pia_ic4 = device->machine().device<pia6821_device>("pia_ic4");
	if (state->m_hopper == HOPPER_NONDUART_A)
	{
		//hopper1_drive_sensor(data&0x10);
	}
	switch (state->m_lamp_extender)
	{
		case NO_EXTENDER:
		if (state->m_led_extender == CARD_B)
		{
			led_write_latch(state, data & 0x1f, pia_ic4->a_output(),state->m_input_strobe);
		}
		else if ((state->m_led_extender != CARD_A)||(state->m_led_extender != NO_EXTENDER))
		{
			for(i=0; i<8; i++)
			{
				output_set_indexed_value("mpu4led",((state->m_input_strobe + 8) * 8) +i,(data & (1 << i)) !=0);
			}
			output_set_digit_value((state->m_input_strobe+8),data);
		}
		break;
		case SMALL_CARD:
		if(state->m_ic23_active)
		{
			lamp_extend_small(state,data);
		}
		break;
		case LARGE_CARD_A:
		lamp_extend_large(state,data,state->m_input_strobe,state->m_ic23_active);
		break;
		case LARGE_CARD_B:
		lamp_extend_large(state,data,state->m_input_strobe,state->m_ic23_active);
		if ((state->m_ic23_active) && state->m_card_live)
		{
			for(i=0; i<8; i++)
			{
				output_set_indexed_value("mpu4led",(((8*(state->m_last_b7 >>7))+ state->m_input_strobe) * 8) +i,(~data & (1 << i)) !=0);
			}
			output_set_digit_value(((8*(state->m_last_b7 >>7))+state->m_input_strobe),~data);
		}
		break;
		case LARGE_CARD_C:
		lamp_extend_large(state,data,state->m_input_strobe,state->m_ic23_active);
		break;
	}
	if (state->m_reel_mux == SIX_REEL_5TO8)
	{
		stepper_update(4, data&0x0F);
		stepper_update(5, (data >> 4)&0x0F);
		awp_draw_reel(4);
		awp_draw_reel(5);
	}
	else
	if (state->m_reel_mux == SEVEN_REEL)
	{
		stepper_update(1, data&0x0F);
		stepper_update(2, (data >> 4)&0x0F);
		awp_draw_reel(1);
		awp_draw_reel(2);
	}

		if (mame_stricmp(device->machine().system().name, "m4gmball") == 0)
		{
		/* The 'Gamball' device is a unique piece of mechanical equipment, designed to
        provide a truly fair hi-lo gamble for an AWP machine. Functionally, it consists of
        a ping-pong ball or similar enclosed in the machine's backbox, on a platform with 12
        holes. When the low 4 bytes of AUX1 are triggered, this fires the ball out from the
        hole it's currently in, to land in another. Landing in the same hole causes the machine to
        refire the ball. The ball detection is done by the high 4 bytes of AUX1.
        Here we call the MAME RNG, once to pick a row, once to pick from the four pockets within it. We
        then trigger the switches corresponding to the correct number. This appears to be the best way
        of making the game fair, short of simulating the physics of a bouncing ball ;)*/
		if (data & 0x0f)
		{
			switch (device->machine().rand() & 0x2)
			{
				case 0x00: //Top row
				{
					switch (device->machine().rand() & 0x3)
					{
						case 0x00: //7
						state->m_aux1_input = (state->m_aux1_input & 0x0f);
						state->m_aux1_input|= 0xa0;
						break;
						case 0x01://4
						state->m_aux1_input = (state->m_aux1_input & 0x0f);
						state->m_aux1_input|= 0xb0;
						break;
						case 0x02://9
						state->m_aux1_input = (state->m_aux1_input & 0x0f);
						state->m_aux1_input|= 0xc0;
						break;
						case 0x03://8
						state->m_aux1_input = (state->m_aux1_input & 0x0f);
						state->m_aux1_input|= 0xd0;
						break;
					}
				}
				case 0x01: //Middle row - note switches don't match pattern
				{
					switch (device->machine().rand() & 0x3)
					{
						case 0x00://12
						state->m_aux1_input = (state->m_aux1_input & 0x0f);
						state->m_aux1_input|= 0x40;
						break;
						case 0x01://1
						state->m_aux1_input = (state->m_aux1_input & 0x0f);
						state->m_aux1_input|= 0x50;
						break;
						case 0x02://11
						state->m_aux1_input = (state->m_aux1_input & 0x0f);
						state->m_aux1_input|= 0x80;
						break;
						case 0x03://2
						state->m_aux1_input = (state->m_aux1_input & 0x0f);
						state->m_aux1_input|= 0x90;
						break;
					}
				}
				case 0x02: //Bottom row
				{
					switch (device->machine().rand() & 0x3)
					{
						case 0x00://5
						state->m_aux1_input = (state->m_aux1_input & 0x0f);
						state->m_aux1_input|= 0x00;
						break;
						case 0x01://10
						state->m_aux1_input = (state->m_aux1_input & 0x0f);
						state->m_aux1_input|= 0x10;
						break;
						case 0x02://3
						state->m_aux1_input = (state->m_aux1_input & 0x0f);
						state->m_aux1_input|= 0x20;
						break;
						case 0x03://6
						state->m_aux1_input = (state->m_aux1_input & 0x0f);
						state->m_aux1_input|= 0x30;
						break;
					}
				}
			}
		}
	}
}

static WRITE8_DEVICE_HANDLER( pia_ic5_portb_w )
{
	mpu4_state *state = device->machine().driver_data<mpu4_state>();
	if (state->m_hopper == HOPPER_NONDUART_B)
	{
		//hopper1_drive_motor(data &0x01)
		//hopper1_drive_sensor(data &0x08)
	}
	if (state->m_led_extender == CARD_A)
	{
		// led_write_latch(state, data & 0x07, pia_get_output_a(pia_ic4),state->m_input_strobe)
	}

}
READ8_DEVICE_HANDLER( pia_ic5_portb_r )
{
	mpu4_state *state = device->machine().driver_data<mpu4_state>();
	pia6821_device *pia_ic5 = device->machine().device<pia6821_device>("pia_ic5");
	if (state->m_hopper == HOPPER_NONDUART_B)
	{/*
        if (hopper1_active)
        {
            state->m_aux2_input |= 0x08;
        }
        else
        {
            state->m_aux2_input &= ~0x08;
        }*/
	}

	LOG(("%s: IC5 PIA Read of Port B (coin input AUX2)\n",device->machine().describe_context()));
	coin_lockout_w(device->machine(), 0, (pia_ic5->b_output() & 0x01) );
	coin_lockout_w(device->machine(), 1, (pia_ic5->b_output() & 0x02) );
	coin_lockout_w(device->machine(), 2, (pia_ic5->b_output() & 0x04) );
	coin_lockout_w(device->machine(), 3, (pia_ic5->b_output() & 0x08) );
	return input_port_read(device->machine(), "AUX2") | state->m_aux2_input;
}


WRITE_LINE_DEVICE_HANDLER( pia_ic5_ca2_w )
{
	mpu4_state *drvstate = device->machine().driver_data<mpu4_state>();
	LOG(("%s: IC5 PIA Write CA2 (Serial Tx) %2x\n",device->machine().describe_context(),state));
	drvstate->m_serial_data = state;
}


/* ---------------------------------------
   AY Chip sound function selection -
   ---------------------------------------
The databus of the AY sound chip is connected to IC6 Port A.
Data is read from/written to the AY chip through this port.

If this sounds familiar, Amstrad did something very similar with their home computers.

The PSG function, defined by the BC1,BC2 and BDIR signals, is controlled by CA2 and CB2 of IC6.

PSG function selection:
-----------------------
BDIR = IC6 CB2 and BC1 = IC6 CA2

Pin            | PSG Function
BDIR BC1       |
0    0         | Inactive
0    1         | Read from selected PSG register. When function is set, the PSG will make the register data available to Port A.
1    0         | Write to selected PSG register. When set, the PSG will take the data at Port A and write it into the selected PSG register.
1    1         | Select PSG register. When set, the PSG will take the data at Port A and select a register.
*/

/* PSG function selected */
static void update_ay(device_t *device)
{
	mpu4_state *state = device->machine().driver_data<mpu4_state>();
	pia6821_device *pia = downcast<pia6821_device *>(device);
	if (!pia->cb2_output())
	{
		switch (state->m_ay8913_address)
		{
			case 0x00:
			{
				/* Inactive */
				break;
			}
			case 0x01:
			{	/* CA2 = 1 CB2 = 0? : Read from selected PSG register and make the register data available to Port A */
				pia6821_device *pia_ic6 = device->machine().device<pia6821_device>("pia_ic6");
				LOG(("AY8913 address = %d \n",pia_ic6->a_output()&0x0f));
				break;
			}
			case 0x02:
			{/* CA2 = 0 CB2 = 1? : Write to selected PSG register and write data to Port A */
				pia6821_device *pia_ic6 = device->machine().device<pia6821_device>("pia_ic6");
				device_t *ay = device->machine().device("ay8913");
				ay8910_data_w(ay, 0, pia_ic6->a_output());
				LOG(("AY Chip Write \n"));
				break;
			}
			case 0x03:
			{/* CA2 = 1 CB2 = 1? : The register will now be selected and the user can read from or write to it.
             The register will remain selected until another is chosen.*/
				pia6821_device *pia_ic6 = device->machine().device<pia6821_device>("pia_ic6");
				device_t *ay = device->machine().device("ay8913");
				ay8910_address_w(ay, 0, pia_ic6->a_output());
				LOG(("AY Chip Select \n"));
				break;
			}
			default:
			{
				LOG(("AY Chip error \n"));
			}
		}
	}
}


WRITE_LINE_DEVICE_HANDLER( pia_ic5_cb2_w )
{
    update_ay(device);
}


static const pia6821_interface pia_ic5_intf =
{
	DEVCB_HANDLER(pia_ic5_porta_r),		/* port A in */
	DEVCB_HANDLER(pia_ic5_portb_r),	/* port B in */
	DEVCB_NULL,		/* line CA1 in */
	DEVCB_NULL,		/* line CB1 in */
	DEVCB_NULL,		/* line CA2 in */
	DEVCB_NULL,		/* line CB2 in */
	DEVCB_HANDLER(pia_ic5_porta_w),		/* port A out */
	DEVCB_HANDLER(pia_ic5_portb_w),		/* port B out */
	DEVCB_LINE(pia_ic5_ca2_w),		/* line CA2 out */
	DEVCB_LINE(pia_ic5_cb2_w),		/* port CB2 out */
	DEVCB_LINE(cpu0_irq),			/* IRQA */
	DEVCB_LINE(cpu0_irq)			/* IRQB */
};


/* IC6, Reel A and B and AY registers (MODs below 4 only) */
static WRITE8_DEVICE_HANDLER( pia_ic6_portb_w )
{
	mpu4_state *state = device->machine().driver_data<mpu4_state>();
	LOG(("%s: IC6 PIA Port B Set to %2x (Reel A and B)\n", device->machine().describe_context(),data));

	if (state->m_reel_mux == SEVEN_REEL)
	{
		stepper_update(3, data&0x0F);
		stepper_update(4, (data >> 4)&0x0F);
		awp_draw_reel(3);
		awp_draw_reel(4);
	}
	else if (state->m_reels)
	{
		stepper_update(0, data & 0x0F );
		stepper_update(1, (data>>4) & 0x0F );
		awp_draw_reel(0);
		awp_draw_reel(1);
	}

	if (state->m_reel_flag && (state->m_reel_mux == STANDARD_REEL) && state->m_reels)
	{
		if ( stepper_optic_state(0) ) state->m_optic_pattern |=  0x01;
		else                          state->m_optic_pattern &= ~0x01;

		if ( stepper_optic_state(1) ) state->m_optic_pattern |=  0x02;
		else                          state->m_optic_pattern &= ~0x02;
	}
}


static WRITE8_DEVICE_HANDLER( pia_ic6_porta_w )
{
	mpu4_state *state = device->machine().driver_data<mpu4_state>();
	LOG(("%s: IC6 PIA Write A %2x\n", device->machine().describe_context(),data));
	if (state->m_mod_number <4)
	{
		state->m_ay_data = data;
		update_ay(device);
	}
}


static WRITE_LINE_DEVICE_HANDLER( pia_ic6_ca2_w )
{
	mpu4_state *drvstate = device->machine().driver_data<mpu4_state>();
	LOG(("%s: IC6 PIA write CA2 %2x (AY8913 BC1)\n", device->machine().describe_context(),state));
	if (drvstate->m_mod_number <4)
	{
		if ( state ) drvstate->m_ay8913_address |=  0x01;
		else         drvstate->m_ay8913_address &= ~0x01;
		update_ay(device);
	}
}


static WRITE_LINE_DEVICE_HANDLER( pia_ic6_cb2_w )
{
	mpu4_state *drvstate = device->machine().driver_data<mpu4_state>();
	LOG(("%s: IC6 PIA write CB2 %2x (AY8913 BCDIR)\n", device->machine().describe_context(),state));
	if (drvstate->m_mod_number <4)
	{
		if ( state ) drvstate->m_ay8913_address |=  0x02;
		else         drvstate->m_ay8913_address &= ~0x02;
		update_ay(device);
	}
}


static const pia6821_interface pia_ic6_intf =
{
	DEVCB_NULL,		/* port A in */
	DEVCB_NULL,		/* port B in */
	DEVCB_NULL,		/* line CA1 in */
	DEVCB_NULL,		/* line CB1 in */
	DEVCB_NULL,		/* line CA2 in */
	DEVCB_NULL,		/* line CB2 in */
	DEVCB_HANDLER(pia_ic6_porta_w),		/* port A out */
	DEVCB_HANDLER(pia_ic6_portb_w),		/* port B out */
	DEVCB_LINE(pia_ic6_ca2_w),			/* line CA2 out */
	DEVCB_LINE(pia_ic6_cb2_w),			/* port CB2 out */
	DEVCB_LINE(cpu0_irq),				/* IRQA */
	DEVCB_LINE(cpu0_irq)				/* IRQB */
};


/* IC7 Reel C and D, mechanical meters/Reel E and F, input strobe bit A */
static WRITE8_DEVICE_HANDLER( pia_ic7_porta_w )
{
	mpu4_state *state = device->machine().driver_data<mpu4_state>();
	LOG(("%s: IC7 PIA Port A Set to %2x (Reel C and D)\n", device->machine().describe_context(),data));
	if (state->m_reel_mux == SEVEN_REEL)
	{
		stepper_update(5, data&0x0F);
		stepper_update(6, (data >> 4)&0x0F);
		awp_draw_reel(5);
		awp_draw_reel(6);
	}
	else if (state->m_reels)
	{
		stepper_update(2, data & 0x0F );
		stepper_update(3, (data>>4) & 0x0F );
		awp_draw_reel(2);
		awp_draw_reel(3);
	}

	if (state->m_reel_flag && (state->m_reel_mux == STANDARD_REEL) && state->m_reels)
	{
		if ( stepper_optic_state(2) ) state->m_optic_pattern |=  0x04;
		else                          state->m_optic_pattern &= ~0x04;
		if ( stepper_optic_state(3) ) state->m_optic_pattern |=  0x08;
		else                          state->m_optic_pattern &= ~0x08;
	}
}

static WRITE8_DEVICE_HANDLER( pia_ic7_portb_w )
{
	mpu4_state *state = device->machine().driver_data<mpu4_state>();
	if (state->m_hopper == HOPPER_DUART_A)
	{
		//duart write data
	}
	else if (state->m_hopper == HOPPER_NONDUART_A)
	{
		//hoppr1_drive_motor(data & 0x10);
	}

    state->m_mmtr_data = data;
}

static READ8_DEVICE_HANDLER( pia_ic7_portb_r )
{
/* The meters are connected to a voltage drop sensor, where current
flowing through them also passes through pin B7, meaning that when
any meter is activated, pin B7 goes high.
As for why they connected this to an output port rather than using
CB1, no idea, although it proved of benefit when the reel multiplexer was designed
as it allows a separate meter to be used when the rest of the port is blocked.
This appears to have confounded the schematic drawer, who has assumed that
all eight meters are driven from this port, giving the 8 line driver chip
9 connections in total. */

	//This may be overkill, but the meter sensing is VERY picky

	int combined_meter = MechMtr_GetActivity(0) | MechMtr_GetActivity(1) |
						 MechMtr_GetActivity(2) | MechMtr_GetActivity(3) |
						 MechMtr_GetActivity(4) | MechMtr_GetActivity(5) |
						 MechMtr_GetActivity(6) | MechMtr_GetActivity(7);

	if(combined_meter)
	{
		return 0x80;
	}
	else
	{
		return 0x00;
	}
}







static WRITE_LINE_DEVICE_HANDLER( pia_ic7_ca2_w )
{
	mpu4_state *drvstate = device->machine().driver_data<mpu4_state>();
	LOG(("%s: IC7 PIA write CA2 %2x (input strobe bit 0 / LED A)\n", device->machine().describe_context(),state));

	drvstate->m_IC23GA = state;
	ic24_setup(drvstate);
	ic23_update(drvstate);
}

static WRITE_LINE_DEVICE_HANDLER( pia_ic7_cb2_w )
{
	mpu4_state *drvstate = device->machine().driver_data<mpu4_state>();
	drvstate->m_remote_meter = state?0x80:0x00;
}

static const pia6821_interface pia_ic7_intf =
{
	DEVCB_NULL,		/* port A in */
	DEVCB_HANDLER(pia_ic7_portb_r),		/* port B in */
	DEVCB_NULL,		/* line CA1 in */
	DEVCB_NULL,		/* line CB1 in */
	DEVCB_NULL,		/* line CA2 in */
	DEVCB_NULL,		/* line CB2 in */
	DEVCB_HANDLER(pia_ic7_porta_w),		/* port A out */
	DEVCB_HANDLER(pia_ic7_portb_w),		/* port B out */
	DEVCB_LINE(pia_ic7_ca2_w),			/* line CA2 out */
	DEVCB_LINE(pia_ic7_cb2_w),			/* line CB2 out */
	DEVCB_LINE(cpu0_irq),				/* IRQA */
	DEVCB_LINE(cpu0_irq)				/* IRQB */
};


/* IC8, Inputs, TRIACS, alpha clock */
static READ8_DEVICE_HANDLER( pia_ic8_porta_r )
{
	mpu4_state *state = device->machine().driver_data<mpu4_state>();
	static const char *const portnames[] = { "ORANGE1", "ORANGE2", "BLACK1", "BLACK2", "ORANGE1", "ORANGE2", "DIL1", "DIL2" };
	pia6821_device *pia_ic5 = device->machine().device<pia6821_device>("pia_ic5");

	LOG_IC8(("%s: IC8 PIA Read of Port A (MUX input data)\n", device->machine().describe_context()));
/* The orange inputs are polled twice as often as the black ones, for reasons of efficiency.
   This is achieved via connecting every input line to an AND gate, thus allowing two strobes
   to represent each orange input bank (strobes are active low). */
	pia_ic5->cb1_w(input_port_read(device->machine(), "AUX2") & 0x80);
	return input_port_read(device->machine(), portnames[state->m_input_strobe]);
}


static WRITE8_DEVICE_HANDLER( pia_ic8_portb_w )
{
	mpu4_state *state = device->machine().driver_data<mpu4_state>();
	if (state->m_hopper == HOPPER_DUART_B)
	{
//      duart.drive_sensor(data & 0x04, data & 0x01, 0, 0);
	}
	else if (state->m_hopper == HOPPER_DUART_C)
	{
//      duart.drive_sensor(data & 0x04, data & 0x01, data & 0x04, data & 0x02);
	}
	int i;
	LOG_IC8(("%s: IC8 PIA Port B Set to %2x (OUTPUT PORT, TRIACS)\n", device->machine().describe_context(),data));
	for (i = 0; i < 8; i++)
	{
		output_set_indexed_value("triac", i, data & (1 << i));
	}
}

static WRITE_LINE_DEVICE_HANDLER( pia_ic8_ca2_w )
{
	mpu4_state *drvstate = device->machine().driver_data<mpu4_state>();
	LOG_IC8(("%s: IC8 PIA write CA2 (input_strobe bit 2 / LED C) %02X\n", device->machine().describe_context(), state & 0xFF));

	drvstate->m_IC23GC = state;
	ic23_update(drvstate);
}


static WRITE_LINE_DEVICE_HANDLER( pia_ic8_cb2_w )
{
	mpu4_state *drvstate = device->machine().driver_data<mpu4_state>();
	LOG_IC8(("%s: IC8 PIA write CB2 (alpha clock) %02X\n", device->machine().describe_context(), state & 0xFF));

	// DM Data pin B
	if (drvstate->m_alpha_clock != state)
	{
		if (!drvstate->m_alpha_clock)//falling edge
		{
			ROC10937_shift_data(0, drvstate->m_alpha_data_line?0:1);
		}
	}
	drvstate->m_alpha_clock = state;
	ROC10937_draw_16seg(0);
}


static const pia6821_interface pia_ic8_intf =
{
	DEVCB_HANDLER(pia_ic8_porta_r),		/* port A in */
	DEVCB_NULL,		/* port B in */
	DEVCB_NULL,		/* line CA1 in */
	DEVCB_NULL,		/* line CB1 in */
	DEVCB_NULL,		/* line CA2 in */
	DEVCB_NULL,		/* line CB2 in */
	DEVCB_NULL,		/* port A out */
	DEVCB_HANDLER(pia_ic8_portb_w),		/* port B out */
	DEVCB_LINE(pia_ic8_ca2_w),			/* line CA2 out */
	DEVCB_LINE(pia_ic8_cb2_w),			/* port CB2 out */
	DEVCB_LINE(cpu0_irq),				/* IRQA */
	DEVCB_LINE(cpu0_irq)				/* IRQB */
};

// universal sampled sound program card PCB 683077
// Sampled sound card, using a PIA and PTM for timing and data handling
static WRITE8_DEVICE_HANDLER( pia_gb_porta_w )
{
	device_t *msm6376 = device->machine().device("msm6376");
	LOG_SS(("%s: GAMEBOARD: PIA Port A Set to %2x\n", device->machine().describe_context(),data));
	okim6376_w(msm6376, 0, data);
}

static WRITE8_DEVICE_HANDLER( pia_gb_portb_w )
{
	mpu4_state *state = device->machine().driver_data<mpu4_state>();
	device_t *msm6376 = device->machine().device("msm6376");
	okim6376_device *msm = device->machine().device<okim6376_device>("msm6376");

	int changed = state->m_expansion_latch^data;

	LOG_SS(("%s: GAMEBOARD: PIA Port B Set to %2x\n", device->machine().describe_context(),data));

	if ( changed & 0x20)
	{ // digital volume clock line changed
		if ( !(data & 0x20) )
		{ // changed from high to low,
			if ( !(data & 0x10) )//down
			{
				if ( state->m_global_volume < 32 ) state->m_global_volume++; //steps unknown
			}
			else
			{//up
				if ( state->m_global_volume > 0  ) state->m_global_volume--;
			}

			{
				float percent = (32-state->m_global_volume)/32.0;
				msm->set_output_gain(0, percent);
				msm->set_output_gain(1, percent);
			}
		}
	}
	okim6376_ch2_w(msm6376,data&0x02);
	okim6376_st_w(msm6376,data&0x01);
}
static READ8_DEVICE_HANDLER( pia_gb_portb_r )
{
	device_t *msm6376 = device->machine().device("msm6376");
	mpu4_state *state = device->machine().driver_data<mpu4_state>();
	LOG_SS(("%s: GAMEBOARD: PIA Read of Port B\n",device->machine().describe_context()));
	int data=0;
	// b7 NAR - we can load another address into Channel 1
	// b6, 1 = OKI ready, 0 = OKI busy
	// b5, vol clock
	// b4, 1 = Vol down, 0 = Vol up
	//

	if ( okim6376_nar_r(msm6376) ) data |= 0x80;
	else							data &= ~0x80;

	if ( okim6376_busy_r(msm6376) )	data |= 0x40;
	else							data &= ~0x40;

	return ( data | state->m_expansion_latch );
}

static WRITE_LINE_DEVICE_HANDLER( pia_gb_ca2_w )
{
	LOG_SS(("%s: GAMEBOARD: OKI RESET data = %02X\n", device->machine().describe_context(), state));

//  reset line
}

static WRITE_LINE_DEVICE_HANDLER( pia_gb_cb2_w )
{
	mpu4_state *mstate = device->machine().driver_data<mpu4_state>();
	//Some BWB games use this to drive the bankswitching
	if (mstate->m_bwb_bank)
	{
		mstate->m_pageval=state;

		memory_set_bank(device->machine(), "bank1",mstate->m_pageval + (mstate->m_pageset?4:0));
	}
}

static const pia6821_interface pia_ic4ss_intf =
{
	DEVCB_NULL,		/* port A in */
	DEVCB_HANDLER(pia_gb_portb_r),	/* port B in */
	DEVCB_NULL,		/* line CA1 in */
	DEVCB_NULL,		/* line CB1 in */
	DEVCB_NULL,		/* line CA2 in */
	DEVCB_NULL,		/* line CB2 in */
	DEVCB_HANDLER(pia_gb_porta_w),		/* port A out */
	DEVCB_HANDLER(pia_gb_portb_w),		/* port B out */
	DEVCB_LINE(pia_gb_ca2_w),		/* line CA2 out */
	DEVCB_LINE(pia_gb_cb2_w),		/* line CB2 out */
	DEVCB_NULL,		/* IRQA */
	DEVCB_NULL		/* IRQB */
};

//Sampled sound timer
/*
The MSM6376 sound chip is configured in a slightly strange way, to enable dynamic
sample rate changes (8Khz, 10.6 Khz, 16 KHz) by varying the clock.
According to the BwB programmer's guide, the formula is:
MSM6376 clock frequency:-
freq = (1720000/((t3L+1)(t3H+1)))*[(t3H(T3L+1)+1)/(2(t1+1))]
where [] means rounded up integer,
t3L is the LSB of Clock 3,
t3H is the MSB of Clock 3,
and t1 is the initial value in clock 1.
*/

//O3 -> G1  O1 -> c2 o2 -> c1
static WRITE8_DEVICE_HANDLER( ic3ss_o1_callback )
{
	downcast<ptm6840_device *>(device)->set_c2(data);
}


static WRITE8_DEVICE_HANDLER( ic3ss_o2_callback )//Generates 'beep' tone
{
	downcast<ptm6840_device *>(device)->set_c1(data);//?
}


static WRITE8_DEVICE_HANDLER( ic3ss_o3_callback )
{
	//downcast<ptm6840_device *>(device)->set_g1(data); /* this output is the clock for timer1 */
}

/* This is a bit of a cheat - since we don't clock into the OKI chip directly, we need to
calculate the oscillation frequency in advance. We're running the timer for interrupt
purposes, but the frequency calculation is done by plucking the values out as they are written.*/
WRITE8_HANDLER( ic3ss_w )
{
	device_t *ic3ss = space->machine().device("ptm_ic3ss");
	mpu4_state *state = space->machine().driver_data<mpu4_state>();
	downcast<ptm6840_device *>(ic3ss)->write(offset,data);
	device_t *msm6376 = space->machine().device("msm6376");

	if (offset == 3)
	{
		state->m_t1 = data;
	}
	if (offset == 6)
	{
		state->m_t3h = data;
	}
	if (offset == 7)
	{
		state->m_t3l = data;
	}

	float num = (1720000/((state->m_t3l + 1)*(state->m_t3h + 1)));
	float denom1 = ((state->m_t3h *(state->m_t3l + 1)+ 1)/(2*(state->m_t1 + 1)));

	int denom2 = denom1 +0.5;//need to round up, this gives same precision as chip
	int freq=num*denom2;

	if (freq)
	{
		okim6376_set_frequency(msm6376, freq);
	}
}


static const ptm6840_interface ptm_ic3ss_intf =
{
	MPU4_MASTER_CLOCK / 4,
	{ 0, 0, 0 },
	{ DEVCB_HANDLER(ic3ss_o1_callback),
	  DEVCB_HANDLER(ic3ss_o2_callback),
	  DEVCB_HANDLER(ic3ss_o3_callback) },
	DEVCB_NULL//LINE(cpu1_ptm_irq)
};

/* input ports for MPU4 board */
INPUT_PORTS_START( mpu4 )
	PORT_START("ORANGE1")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_OTHER) PORT_NAME("00")//  20p level
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_OTHER) PORT_NAME("01")// 100p level
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_OTHER) PORT_NAME("02")// Token 1 level
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_OTHER) PORT_NAME("03")// Token 2 level
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_OTHER) PORT_NAME("04")
	PORT_CONFNAME( 0xE0, 0x00, "Stake Key" )
	PORT_CONFSETTING(    0x00, "Not fitted / 5p"  )
	PORT_CONFSETTING(    0x20, "10p" )
	PORT_CONFSETTING(    0x40, "20p" )
	PORT_CONFSETTING(    0x60, "25p" )
	PORT_CONFSETTING(    0x80, "30p" )
	PORT_CONFSETTING(    0xA0, "40p" )
	PORT_CONFSETTING(    0xC0, "50p" )
	PORT_CONFSETTING(    0xE0, "1 GBP" )

	PORT_START("ORANGE2")
	PORT_CONFNAME( 0x0F, 0x00, "Jackpot / Prize Key" )
	PORT_CONFSETTING(    0x00, "Not fitted"  )
	PORT_CONFSETTING(    0x01, "3 GBP"  )
	PORT_CONFSETTING(    0x02, "4 GBP"  )
	PORT_CONFSETTING(    0x08, "5 GBP"  )
	PORT_CONFSETTING(    0x03, "6 GBP"  )
	PORT_CONFSETTING(    0x04, "6 GBP Token"  )
	PORT_CONFSETTING(    0x05, "8 GBP"  )
	PORT_CONFSETTING(    0x06, "8 GBP Token"  )
	PORT_CONFSETTING(    0x07, "10 GBP"  )
	PORT_CONFSETTING(    0x09, "15 GBP"  )
	PORT_CONFSETTING(    0x0A, "25 GBP"  )
	PORT_CONFSETTING(    0x0B, "25 GBP (Licensed Betting Office Profile)"  )
	PORT_CONFSETTING(    0x0C, "35 GBP"  )
	PORT_CONFSETTING(    0x0D, "70 GBP"  )
	PORT_CONFSETTING(    0x0E, "Reserved"  )
	PORT_CONFSETTING(    0x0F, "Reserved"  )

	PORT_CONFNAME( 0xF0, 0x00, "Percentage Key" )
	PORT_CONFSETTING(    0x00, "Not fitted / 68% (Invalid for UK Games)"  )
	PORT_CONFSETTING(    0x10, "70" )
	PORT_CONFSETTING(    0x20, "72" )
	PORT_CONFSETTING(    0x30, "74" )
	PORT_CONFSETTING(    0x40, "76" )
	PORT_CONFSETTING(    0x50, "78" )
	PORT_CONFSETTING(    0x60, "80" )
	PORT_CONFSETTING(    0x70, "82" )
	PORT_CONFSETTING(    0x80, "84" )
	PORT_CONFSETTING(    0x90, "86" )
	PORT_CONFSETTING(    0xA0, "88" )
	PORT_CONFSETTING(    0xB0, "90" )
	PORT_CONFSETTING(    0xC0, "92" )
	PORT_CONFSETTING(    0xD0, "94" )
	PORT_CONFSETTING(    0xE0, "96" )
	PORT_CONFSETTING(    0xF0, "98" )

	PORT_START("BLACK1")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_BUTTON1) PORT_NAME("Hi")
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_BUTTON2) PORT_NAME("Lo")
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_OTHER)   PORT_NAME("18")
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_OTHER)   PORT_NAME("19")
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_OTHER)	PORT_NAME("20")
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_SERVICE) PORT_NAME("Test Button") PORT_CODE(KEYCODE_W)
	PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_SERVICE) PORT_NAME("Refill Key") PORT_CODE(KEYCODE_R) PORT_TOGGLE
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_INTERLOCK) PORT_NAME("Cashbox (Back) Door")  PORT_CODE(KEYCODE_Q) PORT_TOGGLE

	PORT_START("BLACK2")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_OTHER) PORT_NAME("24")
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_OTHER) PORT_NAME("25")
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_BUTTON3) PORT_NAME("Cancel")
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_BUTTON4) PORT_NAME("Hold 1")
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_BUTTON5) PORT_NAME("Hold 2")
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_BUTTON6) PORT_NAME("Hold 3")
	PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_BUTTON7) PORT_NAME("Hold 4")
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_START1)

	PORT_START("DIL1")
	PORT_DIPNAME( 0x01, 0x00, DEF_STR( Unused ) ) PORT_DIPLOCATION("DIL1:01")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x01, DEF_STR( On  ) )
	PORT_DIPNAME( 0x02, 0x00, DEF_STR( Unused ) ) PORT_DIPLOCATION("DIL1:02")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x02, DEF_STR( On  ) )
	PORT_DIPNAME( 0x04, 0x00, DEF_STR( Unused ) ) PORT_DIPLOCATION("DIL1:03")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x04, DEF_STR( On  ) )
	PORT_DIPNAME( 0x08, 0x00, DEF_STR( Unused ) ) PORT_DIPLOCATION("DIL1:04")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x08, DEF_STR( On  ) )
	PORT_DIPNAME( 0xF0, 0x00, "Target Percentage (if key not fitted)" )PORT_DIPLOCATION("DIL1:05,06,07,08")
	PORT_DIPSETTING(    0x00, "Unset (Program Optimum)"  )
	PORT_DIPSETTING(    0x10, "70" )
	PORT_DIPSETTING(    0x20, "72" )
	PORT_DIPSETTING(    0x30, "74" )
	PORT_DIPSETTING(    0x40, "76" )
	PORT_DIPSETTING(    0x50, "78" )
	PORT_DIPSETTING(    0x60, "80" )
	PORT_DIPSETTING(    0x70, "82" )
	PORT_DIPSETTING(    0x80, "84" )
	PORT_DIPSETTING(    0x90, "86" )
	PORT_DIPSETTING(    0xA0, "88" )
	PORT_DIPSETTING(    0xB0, "90" )
	PORT_DIPSETTING(    0xC0, "92" )
	PORT_DIPSETTING(    0xD0, "94" )
	PORT_DIPSETTING(    0xE0, "96" )
	PORT_DIPSETTING(    0xF0, "98" )

	PORT_START("DIL2")
	PORT_DIPNAME( 0x01, 0x00, "Token Lockout when full" ) PORT_DIPLOCATION("DIL2:01")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x01, DEF_STR( On  ) )
	PORT_DIPNAME( 0x02, 0x00, DEF_STR( Unused )) PORT_DIPLOCATION("DIL2:02")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x02, DEF_STR( On  ) )
	PORT_DIPNAME( 0x04, 0x00, "Scottish Coin Handling" ) PORT_DIPLOCATION("DIL2:03")//20p payout
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x04, DEF_STR( On  ) )
	PORT_DIPNAME( 0x08, 0x08, "Out of Credit Display Inhibit" ) PORT_DIPLOCATION("DIL2:04")  // many games need this on to boot
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x08, DEF_STR( On  ) )
	PORT_DIPNAME( 0x10, 0x00, "OCD Audio Enable" ) PORT_DIPLOCATION("DIL2:05")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x10, DEF_STR( On  ) )
	PORT_DIPNAME( 0x20, 0x00, "Coin Alarm Inhibit" ) PORT_DIPLOCATION("DIL2:06")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x20, DEF_STR( On  ) )
	PORT_DIPNAME( 0x40, 0x00, "Token Refill Level Inhibit" ) PORT_DIPLOCATION("DIL2:07")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x40, DEF_STR( On  ) )
	PORT_DIPNAME( 0x80, 0x00, "Single Credit Entry" ) PORT_DIPLOCATION("DIL2:08")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x80, DEF_STR( On  ) )

	PORT_START("AUX1")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_OTHER) PORT_NAME("0")
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_OTHER) PORT_NAME("1")
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_OTHER) PORT_NAME("2")
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_OTHER) PORT_NAME("3")
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_OTHER) PORT_NAME("4")
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_OTHER) PORT_NAME("5")
	PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_OTHER) PORT_NAME("6")
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_OTHER) PORT_NAME("7")

	PORT_START("AUX2")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_SPECIAL)
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_SPECIAL)
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_SPECIAL)
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_SPECIAL)
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_COIN1) PORT_NAME("10p")PORT_IMPULSE(5)
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_COIN2) PORT_NAME("20p")PORT_IMPULSE(5)
	PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_COIN3) PORT_NAME("50p")PORT_IMPULSE(5)
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_COIN4) PORT_NAME("100p")PORT_IMPULSE(5)
INPUT_PORTS_END


static INPUT_PORTS_START( connect4 )
	PORT_START("ORANGE1")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_OTHER) PORT_NAME("00")
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_OTHER) PORT_NAME("01")
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_OTHER) PORT_NAME("02")
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_OTHER) PORT_NAME("03")
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_OTHER) PORT_NAME("04")
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_OTHER) PORT_NAME("05")
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_OTHER) PORT_NAME("06")
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_OTHER) PORT_NAME("07")

	PORT_START("ORANGE2")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_OTHER) PORT_NAME("08")
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_OTHER) PORT_NAME("09")
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_OTHER) PORT_NAME("10")
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_OTHER) PORT_NAME("11")
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_OTHER) PORT_NAME("12")
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_OTHER) PORT_NAME("13")
	PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_OTHER) PORT_NAME("14")
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_OTHER) PORT_NAME("15")

	PORT_START("BLACK1")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_OTHER) PORT_NAME("16")
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_OTHER) PORT_NAME("17")
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_OTHER) PORT_NAME("18")
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_OTHER) PORT_NAME("19")
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_OTHER) PORT_NAME("20")
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_SERVICE) PORT_NAME("Test Switch")
	PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_SERVICE) PORT_NAME("Refill Key") PORT_CODE(KEYCODE_R) PORT_TOGGLE
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_OTHER) PORT_NAME("Door Switch?") PORT_TOGGLE

	PORT_START("BLACK2")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_BUTTON1) PORT_NAME("Select")
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_OTHER) PORT_NAME("25")
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_START2) PORT_NAME("Pass")
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_START1) PORT_NAME("Play")
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_OTHER) PORT_NAME("28")
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_OTHER) PORT_NAME("29")
	PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_OTHER) PORT_NAME("30")
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_BUTTON2) PORT_NAME("Drop")

	PORT_START("DIL1")
	PORT_DIPNAME( 0x80, 0x00, "DIL101" ) PORT_DIPLOCATION("DIL1:01")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x80, DEF_STR( On  ) )
	PORT_DIPNAME( 0x40, 0x00, "DIL102" ) PORT_DIPLOCATION("DIL1:02")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x40, DEF_STR( On  ) )
	PORT_DIPNAME( 0x20, 0x00, "DIL103" ) PORT_DIPLOCATION("DIL1:03")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x20, DEF_STR( On  ) )
	PORT_DIPNAME( 0x10, 0x00, "DIL104" ) PORT_DIPLOCATION("DIL1:04")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x10, DEF_STR( On  ) )
	PORT_DIPNAME( 0x08, 0x00, "DIL105" ) PORT_DIPLOCATION("DIL1:05")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x08, DEF_STR( On  ) )
	PORT_DIPNAME( 0x04, 0x00, "DIL106" ) PORT_DIPLOCATION("DIL1:06")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x04, DEF_STR( On  ) )
	PORT_DIPNAME( 0x02, 0x00, "DIL107" ) PORT_DIPLOCATION("DIL1:07")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x02, DEF_STR( On  ) )
	PORT_DIPNAME( 0x01, 0x00, "DIL108" ) PORT_DIPLOCATION("DIL1:08")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x01, DEF_STR( On  ) )

	PORT_START("DIL2")
	PORT_DIPNAME( 0x80, 0x00, "DIL201" ) PORT_DIPLOCATION("DIL2:01")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x80, DEF_STR( On  ) )
	PORT_DIPNAME( 0x40, 0x00, "DIL202" ) PORT_DIPLOCATION("DIL2:02")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x40, DEF_STR( On  ) )
	PORT_DIPNAME( 0x20, 0x00, "DIL203" ) PORT_DIPLOCATION("DIL2:03")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x20, DEF_STR( On  ) )
	PORT_DIPNAME( 0x10, 0x00, "DIL204" ) PORT_DIPLOCATION("DIL2:04")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x10, DEF_STR( On  ) )
	PORT_DIPNAME( 0x08, 0x00, "DIL205" ) PORT_DIPLOCATION("DIL2:05")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x08, DEF_STR( On  ) )
	PORT_DIPNAME( 0x04, 0x00, "DIL206" ) PORT_DIPLOCATION("DIL2:06")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x04, DEF_STR( On  ) )
	PORT_DIPNAME( 0x02, 0x00, "DIL207" ) PORT_DIPLOCATION("DIL2:07")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x02, DEF_STR( On  ) )
	PORT_DIPNAME( 0x01, 0x00, "DIL208" ) PORT_DIPLOCATION("DIL2:08")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x01, DEF_STR( On  ) )

	PORT_START("AUX1")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_OTHER) PORT_NAME("0")
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_OTHER) PORT_NAME("1")
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_OTHER) PORT_NAME("2")
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_OTHER) PORT_NAME("3")
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_OTHER) PORT_NAME("4")
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_OTHER) PORT_NAME("5")
	PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_OTHER) PORT_NAME("6")
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_OTHER) PORT_NAME("7")

	PORT_START("AUX2")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_SPECIAL)
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_SPECIAL)
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_SPECIAL)
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_SPECIAL)
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_COIN1) PORT_NAME("10p")PORT_IMPULSE(5)
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_COIN2) PORT_NAME("20p")PORT_IMPULSE(5)
	PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_COIN3) PORT_NAME("50p")PORT_IMPULSE(5)
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_COIN4) PORT_NAME("100p")PORT_IMPULSE(5)
INPUT_PORTS_END

static INPUT_PORTS_START( gamball )
	PORT_START("ORANGE1")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_OTHER) PORT_NAME("00")//  20p level
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_OTHER) PORT_NAME("01")// 100p level
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_OTHER) PORT_NAME("02")// Token 1 level
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_OTHER) PORT_NAME("03")// Token 2 level
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_OTHER) PORT_NAME("04")
	PORT_CONFNAME( 0xE0, 0x00, "Stake Key" )
	PORT_CONFSETTING(    0x00, "Not fitted / 5p"  )
	PORT_CONFSETTING(    0x20, "10p" )
	PORT_CONFSETTING(    0x40, "20p" )
	PORT_CONFSETTING(    0x60, "25p" )
	PORT_CONFSETTING(    0x80, "30p" )
	PORT_CONFSETTING(    0xA0, "40p" )
	PORT_CONFSETTING(    0xC0, "50p" )
	PORT_CONFSETTING(    0xE0, "1 GBP" )

	PORT_START("ORANGE2")
	PORT_CONFNAME( 0x0F, 0x00, "Jackpot / Prize Key" )
	PORT_CONFSETTING(    0x00, "Not fitted"  )
	PORT_CONFSETTING(    0x01, "3 GBP"  )
	PORT_CONFSETTING(    0x02, "4 GBP"  )
	PORT_CONFSETTING(    0x08, "5 GBP"  )
	PORT_CONFSETTING(    0x03, "6 GBP"  )
	PORT_CONFSETTING(    0x04, "6 GBP Token"  )
	PORT_CONFSETTING(    0x05, "8 GBP"  )
	PORT_CONFSETTING(    0x06, "8 GBP Token"  )
	PORT_CONFSETTING(    0x07, "10 GBP"  )
	PORT_CONFSETTING(    0x09, "15 GBP"  )
	PORT_CONFSETTING(    0x0A, "25 GBP"  )
	PORT_CONFSETTING(    0x0B, "25 GBP (Licensed Betting Office Profile)"  )
	PORT_CONFSETTING(    0x0C, "35 GBP"  )
	PORT_CONFSETTING(    0x0D, "70 GBP"  )
	PORT_CONFSETTING(    0x0E, "Reserved"  )
	PORT_CONFSETTING(    0x0F, "Reserved"  )

	PORT_CONFNAME( 0xF0, 0x00, "Percentage Key" )
	PORT_CONFSETTING(    0x00, "As Option Switches"  )
	PORT_CONFSETTING(    0x10, "70" )
	PORT_CONFSETTING(    0x20, "72" )
	PORT_CONFSETTING(    0x30, "74" )
	PORT_CONFSETTING(    0x40, "76" )
	PORT_CONFSETTING(    0x50, "78" )
	PORT_CONFSETTING(    0x60, "80" )
	PORT_CONFSETTING(    0x70, "82" )
	PORT_CONFSETTING(    0x80, "84" )
	PORT_CONFSETTING(    0x90, "86" )
	PORT_CONFSETTING(    0xA0, "88" )
	PORT_CONFSETTING(    0xB0, "90" )
	PORT_CONFSETTING(    0xC0, "92" )
	PORT_CONFSETTING(    0xD0, "94" )
	PORT_CONFSETTING(    0xE0, "96" )
	PORT_CONFSETTING(    0xF0, "98" )

	PORT_START("BLACK1")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_BUTTON1) PORT_NAME("Hi")
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_BUTTON2) PORT_NAME("Lo")
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_OTHER) PORT_NAME("18")
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_OTHER) PORT_NAME("19")
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_OTHER) PORT_NAME("20")
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_SERVICE) PORT_NAME("Test Button") PORT_CODE(KEYCODE_W)
	PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_SERVICE) PORT_NAME("Refill Key") PORT_CODE(KEYCODE_R) PORT_TOGGLE
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_INTERLOCK) PORT_NAME("Cashbox Door")  PORT_CODE(KEYCODE_Q) PORT_TOGGLE

	PORT_START("BLACK2")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_OTHER) PORT_NAME("24")
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_OTHER) PORT_NAME("25")
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_BUTTON3) PORT_NAME("Cancel/Collect")
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_BUTTON4) PORT_NAME("Hold/Nudge 1")
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_BUTTON5) PORT_NAME("Hold/Nudge 2")
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_BUTTON6) PORT_NAME("Hold/Nudge 3")
	PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_BUTTON7) PORT_NAME("Hold/Nudge 4")
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_START1)

	PORT_START("DIL1")
	PORT_DIPNAME( 0x80, 0x00, "DIL101" ) PORT_DIPLOCATION("DIL1:01")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x80, DEF_STR( On  ) )
	PORT_DIPNAME( 0x40, 0x00, "DIL102" ) PORT_DIPLOCATION("DIL1:02")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x40, DEF_STR( On  ) )
	PORT_DIPNAME( 0x20, 0x00, "DIL103" ) PORT_DIPLOCATION("DIL1:03")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x20, DEF_STR( On  ) )
	PORT_DIPNAME( 0x10, 0x00, "DIL104" ) PORT_DIPLOCATION("DIL1:04")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x10, DEF_STR( On  ) )
	PORT_DIPNAME( 0x08, 0x00, "DIL105" ) PORT_DIPLOCATION("DIL1:05")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x08, DEF_STR( On  ) )
	PORT_DIPNAME( 0x04, 0x00, "DIL106" ) PORT_DIPLOCATION("DIL1:06")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x04, DEF_STR( On  ) )
	PORT_DIPNAME( 0x02, 0x00, "DIL107" ) PORT_DIPLOCATION("DIL1:07")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x02, DEF_STR( On  ) )
	PORT_DIPNAME( 0x01, 0x00, "DIL108" ) PORT_DIPLOCATION("DIL1:08")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x01, DEF_STR( On  ) )

	PORT_START("DIL2")
	PORT_DIPNAME( 0x80, 0x00, "DIL201" ) PORT_DIPLOCATION("DIL2:01")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x80, DEF_STR( On  ) )
	PORT_DIPNAME( 0x40, 0x00, "DIL202" ) PORT_DIPLOCATION("DIL2:02")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x40, DEF_STR( On  ) )
	PORT_DIPNAME( 0x20, 0x00, "DIL203" ) PORT_DIPLOCATION("DIL2:03")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x20, DEF_STR( On  ) )
	PORT_DIPNAME( 0x10, 0x00, "DIL204" ) PORT_DIPLOCATION("DIL2:04")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x10, DEF_STR( On  ) )
	PORT_DIPNAME( 0x08, 0x00, "DIL205" ) PORT_DIPLOCATION("DIL2:05")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x08, DEF_STR( On  ) )
	PORT_DIPNAME( 0x04, 0x00, "DIL206" ) PORT_DIPLOCATION("DIL2:06")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x04, DEF_STR( On  ) )
	PORT_DIPNAME( 0x02, 0x00, "DIL207" ) PORT_DIPLOCATION("DIL2:07")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x02, DEF_STR( On  ) )
	PORT_DIPNAME( 0x01, 0x00, "DIL208" ) PORT_DIPLOCATION("DIL2:08")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x01, DEF_STR( On  ) )

	PORT_START("AUX1")
	PORT_BIT(0xFF, IP_ACTIVE_HIGH, IPT_SPECIAL)//Handled by Gamball unit

	PORT_START("AUX2")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_SPECIAL)
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_SPECIAL)
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_SPECIAL)
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_SPECIAL)
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_COIN1) PORT_NAME("10p")PORT_IMPULSE(5)
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_COIN2) PORT_NAME("20p")PORT_IMPULSE(5)
	PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_COIN3) PORT_NAME("50p")PORT_IMPULSE(5)
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_COIN4) PORT_NAME("100p")PORT_IMPULSE(5)
INPUT_PORTS_END

static INPUT_PORTS_START( grtecp )
	PORT_START("ORANGE1")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_OTHER) PORT_NAME("00")//  20p level
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_OTHER) PORT_NAME("01")// 100p level
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_OTHER) PORT_NAME("02")// Token 1 level
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_OTHER) PORT_NAME("03")// Token 2 level
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_OTHER) PORT_NAME("04")
	PORT_CONFNAME( 0xE0, 0x00, "Stake Key" )
	PORT_CONFSETTING(    0x00, "Not fitted / 5p"  )
	PORT_CONFSETTING(    0x20, "10p" )
	PORT_CONFSETTING(    0x40, "20p" )
	PORT_CONFSETTING(    0x60, "25p" )
	PORT_CONFSETTING(    0x80, "30p" )
	PORT_CONFSETTING(    0xA0, "40p" )
	PORT_CONFSETTING(    0xC0, "50p" )
	PORT_CONFSETTING(    0xE0, "1 GBP" )

	PORT_START("ORANGE2")
	PORT_CONFNAME( 0x0F, 0x00, "Jackpot / Prize Key" )
	PORT_CONFSETTING(    0x00, "Not fitted"  )
	PORT_CONFSETTING(    0x01, "3 GBP"  )
	PORT_CONFSETTING(    0x02, "4 GBP"  )
	PORT_CONFSETTING(    0x08, "5 GBP"  )
	PORT_CONFSETTING(    0x03, "6 GBP"  )
	PORT_CONFSETTING(    0x04, "6 GBP Token"  )
	PORT_CONFSETTING(    0x05, "8 GBP"  )
	PORT_CONFSETTING(    0x06, "8 GBP Token"  )
	PORT_CONFSETTING(    0x07, "10 GBP"  )
	PORT_CONFSETTING(    0x09, "15 GBP"  )
	PORT_CONFSETTING(    0x0A, "25 GBP"  )
	PORT_CONFSETTING(    0x0B, "25 GBP (Licensed Betting Office Profile)"  )
	PORT_CONFSETTING(    0x0C, "35 GBP"  )
	PORT_CONFSETTING(    0x0D, "70 GBP"  )
	PORT_CONFSETTING(    0x0E, "Reserved"  )
	PORT_CONFSETTING(    0x0F, "Reserved"  )

	PORT_CONFNAME( 0xF0, 0x00, "Percentage Key" )
	PORT_CONFSETTING(    0x00, "As Option Switches"  )
	PORT_CONFSETTING(    0x10, "70" )
	PORT_CONFSETTING(    0x20, "72" )
	PORT_CONFSETTING(    0x30, "74" )
	PORT_CONFSETTING(    0x40, "76" )
	PORT_CONFSETTING(    0x50, "78" )
	PORT_CONFSETTING(    0x60, "80" )
	PORT_CONFSETTING(    0x70, "82" )
	PORT_CONFSETTING(    0x80, "84" )
	PORT_CONFSETTING(    0x90, "86" )
	PORT_CONFSETTING(    0xA0, "88" )
	PORT_CONFSETTING(    0xB0, "90" )
	PORT_CONFSETTING(    0xC0, "92" )
	PORT_CONFSETTING(    0xD0, "94" )
	PORT_CONFSETTING(    0xE0, "96" )
	PORT_CONFSETTING(    0xF0, "98" )

	PORT_START("BLACK1")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_SERVICE) PORT_NAME("Test Button") PORT_CODE(KEYCODE_W)
	PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_SERVICE) PORT_NAME("Refill Key") PORT_CODE(KEYCODE_R) PORT_TOGGLE
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_INTERLOCK) PORT_NAME("Cashbox (Back) Door") PORT_CODE(KEYCODE_Q) PORT_TOGGLE

	PORT_START("BLACK2")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_BUTTON1) PORT_NAME("Collect/Cancel")
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_BUTTON2) PORT_NAME("Hold 1")
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_BUTTON3) PORT_NAME("Hold 2")
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_BUTTON4) PORT_NAME("Hold 3")
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_BUTTON5) PORT_NAME("Hi")
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_BUTTON6) PORT_NAME("Lo")
	PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_BUTTON7) PORT_NAME("Exchange")
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_START1)

	PORT_START("DIL1")
	PORT_DIPNAME( 0x01, 0x00, DEF_STR( Unused ) ) PORT_DIPLOCATION("DIL1:01")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x01, DEF_STR( On  ) )
	PORT_DIPNAME( 0x02, 0x00, DEF_STR( Unused ) ) PORT_DIPLOCATION("DIL1:02")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x02, DEF_STR( On  ) )
	PORT_DIPNAME( 0x04, 0x00, DEF_STR( Unused ) ) PORT_DIPLOCATION("DIL1:03")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x04, DEF_STR( On  ) )
	PORT_DIPNAME( 0x08, 0x00, DEF_STR( Unused ) ) PORT_DIPLOCATION("DIL1:04")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x08, DEF_STR( On  ) )
	PORT_DIPNAME( 0xF0, 0x00, "Target Percentage (if key not fitted)" )PORT_DIPLOCATION("DIL1:05,06,07,08")
	PORT_DIPSETTING(    0x00, "Unset (Program Optimum)"  )
	PORT_DIPSETTING(    0x10, "70" )
	PORT_DIPSETTING(    0x20, "72" )
	PORT_DIPSETTING(    0x30, "74" )
	PORT_DIPSETTING(    0x40, "76" )
	PORT_DIPSETTING(    0x50, "78" )
	PORT_DIPSETTING(    0x60, "80" )
	PORT_DIPSETTING(    0x70, "82" )
	PORT_DIPSETTING(    0x80, "84" )
	PORT_DIPSETTING(    0x90, "86" )
	PORT_DIPSETTING(    0xA0, "88" )
	PORT_DIPSETTING(    0xB0, "90" )
	PORT_DIPSETTING(    0xC0, "92" )
	PORT_DIPSETTING(    0xD0, "94" )
	PORT_DIPSETTING(    0xE0, "96" )
	PORT_DIPSETTING(    0xF0, "98" )

	PORT_START("DIL2")
	PORT_DIPNAME( 0x01, 0x00, "Token Lockout when full" ) PORT_DIPLOCATION("DIL2:01")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x01, DEF_STR( On  ) )
	PORT_DIPNAME( 0x02, 0x00, DEF_STR( Unused )) PORT_DIPLOCATION("DIL2:02")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x02, DEF_STR( On  ) )
	PORT_DIPNAME( 0x04, 0x00, "Scottish Coin Handling" ) PORT_DIPLOCATION("DIL2:03")//20p payout
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x04, DEF_STR( On  ) )
	PORT_DIPNAME( 0x08, 0x00, "Out of Credit Display Inhibit" ) PORT_DIPLOCATION("DIL2:04")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x08, DEF_STR( On  ) )
	PORT_DIPNAME( 0x10, 0x00, "OCD Audio Enable" ) PORT_DIPLOCATION("DIL2:05")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x10, DEF_STR( On  ) )
	PORT_DIPNAME( 0x20, 0x00, "Coin Alarm Inhibit" ) PORT_DIPLOCATION("DIL2:06")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x20, DEF_STR( On  ) )
	PORT_DIPNAME( 0x40, 0x00, "Token Refill Level Inhibit" ) PORT_DIPLOCATION("DIL2:07")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x40, DEF_STR( On  ) )
	PORT_DIPNAME( 0x80, 0x00, "Single Credit Entry" ) PORT_DIPLOCATION("DIL2:08")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x80, DEF_STR( On  ) )

	PORT_START("AUX1")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_OTHER) PORT_NAME("0")
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_OTHER) PORT_NAME("1")
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_OTHER) PORT_NAME("2")
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_OTHER) PORT_NAME("3")
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_OTHER) PORT_NAME("4")
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_OTHER) PORT_NAME("5")
	PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_OTHER) PORT_NAME("6")
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_OTHER) PORT_NAME("7")

	PORT_START("AUX2")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_SPECIAL)
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_SPECIAL)
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_SPECIAL)
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_SPECIAL)
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_COIN1) PORT_NAME("10p")PORT_IMPULSE(5)
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_COIN2) PORT_NAME("20p")PORT_IMPULSE(5)
	PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_COIN3) PORT_NAME("50p")PORT_IMPULSE(5)
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_COIN4) PORT_NAME("100p")PORT_IMPULSE(5)
INPUT_PORTS_END

static const stepper_interface barcrest_reel_interface =
{
	BARCREST_48STEP_REEL,
	92,
	4,
	0x00
};

static const stepper_interface barcrest_reelrev_interface =
{
	BARCREST_48STEP_REEL,
	92,
	4,
	0x00,
	1
};

static const stepper_interface barcrest_opto1_interface =
{
	BARCREST_48STEP_REEL,
	4,
	12,
	0x00
};

static const stepper_interface barcrest_opto2_interface =
{
	BARCREST_48STEP_REEL,
	92,
	3,
	0x00
};

static const stepper_interface barcrest_opto3_interface =
{
	BARCREST_48STEP_REEL,
	0,
	5,
	0x00
};

static const stepper_interface bwb_opto1_interface =
{
	BARCREST_48STEP_REEL,
	96,
	3,
	0x00
};

/*
Characteriser (CHR)

As built, the CHR is a PAL which can perform basic bit manipulation according to
an as yet unknown unique key. However, the programmers decided to best use this protection device in read/write/compare
cycles, storing almost the entire 'hidden' data table in the ROMs in plain sight. Only later rebuilds by BwB
avoided this 'feature' of the development kit, and will need a different setup.

This information has been used to generate the CHR tables loaded by the programs, until a key can be determined.

For most Barcrest games, the following method was used:

The initial 'PALTEST' routine as found in the Barcrest programs simply writes the first 'call' to the CHR space,
to read back the 'response'. There is no attempt to alter the order or anything else, just
a simple runthrough of the entire data table. The only 'catch' in this is to note that the CHR chip always scans
through the table starting at the last accessed data value, unless 00 is used to reset to the beginning. This is obviously
a simplification, in fact the PAL does bit manipulation with some latching.

However, a final 8 byte row, that controls the lamp matrix is not tested - to date, no-one outside of Barcrest knows
how this is generated, and currently trial and error is the only sensible method. It is noted that the default,
of all 00, is sometimes the correct answer, particularly in non-Barcrest use of the CHR chip, though when used normally,
there are again fixed call values.
*/


static WRITE8_HANDLER( characteriser_w )
{
	mpu4_state *state = space->machine().driver_data<mpu4_state>();
	int x;
	int call=data;
	LOG_CHR_FULL(("%04x Characteriser write offset %02X data %02X", cpu_get_previouspc(&space->device()),offset,data));
	if (!state->m_current_chr_table)
	{
		logerror("No Characteriser Table @ %04x\n", cpu_get_previouspc(&space->device()));
		return;
	}



	if (offset == 0)
	{
		{
			if (call == 0)
			{
				state->m_prot_col = 0;
			}
			else
			{
				for (x = state->m_prot_col; x < 64; x++)
				{
					if	(state->m_current_chr_table[(x)].call == call)
					{
						state->m_prot_col = x;
						LOG_CHR(("Characteriser find column %02X\n",state->m_prot_col));
						break;
					}
				}
			}
		}
	}
	else if (offset == 2)
	{
		LOG_CHR(("Characteriser write 2 data %02X\n",data));
		switch (call)
		// Rather than the search strategy, we can map the calls directly here. Note that they are hex versions of the square number series
		{
			case 0x00:
			state->m_lamp_col = 0;
			break;
			case 0x01:
			state->m_lamp_col = 1;
			break;
			case 0x04:
			state->m_lamp_col = 2;
			break;
			case 0x09:
			state->m_lamp_col = 3;
			break;
			case 0x10:
			state->m_lamp_col = 4;
			break;
			case 0x19:
			state->m_lamp_col = 5;
			break;
			case 0x24:
			state->m_lamp_col = 6;
			break;
			case 0x31:
			state->m_lamp_col = 7;
			break;
		}
		LOG_CHR(("Characteriser find 2 column %02X\n",state->m_lamp_col));
	}
}


static READ8_HANDLER( characteriser_r )
{
	mpu4_state *state = space->machine().driver_data<mpu4_state>();
	if (!state->m_current_chr_table)
	{
		logerror("No Characteriser Table @ %04x\n", cpu_get_previouspc(&space->device()));
		return 0x00;
	}

	LOG_CHR(("Characteriser read offset %02X \n",offset));
	if (offset == 0)
	{
		LOG_CHR(("Characteriser read data %02X \n",state->m_current_chr_table[state->m_prot_col].response));
		return state->m_current_chr_table[state->m_prot_col].response;
	}
	if (offset == 3)
	{
		LOG_CHR(("Characteriser read data off 3 %02X \n",state->m_current_chr_table[state->m_lamp_col+64].response));
		return state->m_current_chr_table[state->m_lamp_col+64].response;
	}
	return 0;
}

/*
BwB Characteriser (CHR)

The BwB method of protection is considerably different to the Barcrest one, with any
incorrect behaviour manifesting in ridiculously large payouts. The hardware is the
same, however the main weakness of the software has been eliminated.

In fact, the software seems deliberately designed to mislead, but is (fortunately for
us) prone to similar weaknesses that allow a per game solution.

Project Amber performed a source analysis (available on request) which appears to make things work.
Said weaknesses (A Cheats Guide according to Project Amber)

The common initialisation sequence is "00 04 04 0C 0C 1C 14 2C 5C 2C"
                                        0  1  2  3  4  5  6  7  8
Using debug search for the first read from said string (best to find it first).

At this point, the X index on the CPU is at the magic number address.

The subsequent calls for each can be found based on the magic address

           (0) = ( (BWBMagicAddress))
           (1) = ( (BWBMagicAddress + 1))
           (2) = ( (BWBMagicAddress + 2))
           (3) = ( (BWBMagicAddress + 4))
           (4) = ( (BWBMagicAddress - 5))
           (5) = ( (BWBMagicAddress - 4))
           (6) = ( (BWBMagicAddress - 3))
           (7) = ( (BWBMagicAddress - 2))
           (8) = ( (BWBMagicAddress - 1))

These return the standard init sequence as above.

For ease of understanding, we use three tables, one holding the common responses
and two holding the appropriate call and response pairs for the two stages of operation
*/


static WRITE8_HANDLER( bwb_characteriser_w )
{
	mpu4_state *state = space->machine().driver_data<mpu4_state>();
	int x;
	int call=data;
	LOG_CHR_FULL(("%04x Characteriser write offset %02X data %02X \n", cpu_get_previouspc(&space->device()),offset,data));
	if (!state->m_current_chr_table)
		fatalerror("No Characteriser Table @ %04x\n", cpu_get_previouspc(&space->device()));

	if ((offset & 0x3f)== 0)//initialisation is always at 0x800
	{
		if (!state->m_chr_state)
		{
			state->m_chr_state=1;
			state->m_chr_counter=0;
		}
		if (call == 0)
		{
			state->m_init_col ++;
		}
		else
		{
			state->m_init_col =0;
		}
	}

	state->m_chr_value = space->machine().rand();
	for (x = 0; x < 4; x++)
	{
		if	(state->m_current_chr_table[(x)].call == call)
		{
			if (x == 0) // reinit
			{
				state->m_bwb_return = 0;
			}
			state->m_chr_value = bwb_chr_table_common[(state->m_bwb_return)];
			state->m_bwb_return++;
			break;
		}
	}
}

static READ8_HANDLER( bwb_characteriser_r )
{
	mpu4_state *state = space->machine().driver_data<mpu4_state>();

	LOG_CHR(("Characteriser read offset %02X \n",offset));


	if (offset ==0)
	{
		switch (state->m_chr_counter)
		{
			case 6:
			case 13:
			case 20:
			case 27:
			case 34:
			{
				return state->m_bwb_chr_table1[(((state->m_chr_counter + 1) / 7) - 1)].response;
				break;
			}
			default:
			{
				if (state->m_chr_counter > 34)
				{
					state->m_chr_counter = 35;
					state->m_chr_state = 2;
				}
				state->m_chr_counter ++;
				return state->m_chr_value;
			}
		}
	}
	else
	{
		return state->m_chr_value;
	}
}

/* Common configurations */

void mpu4_install_mod4yam_space(address_space *space)
{
//	ym2413_device *ym = space->machine().device<ym2413_device>("ym2413");
//	space->install_legacy_write_handler(0x0880, 0x0881, write8_delegate(FUNC(ym2413_w), ym));
}

void mpu4_install_mod4oki_space(address_space *space)
{
	pia6821_device *pia_ic4ss = space->machine().device<pia6821_device>("pia_ic4ss");
	ptm6840_device *ptm_ic3ss = space->machine().device<ptm6840_device>("ptm_ic3ss");

	space->install_readwrite_handler(0x0880, 0x0883, 0, 0, read8_delegate(FUNC(pia6821_device::read), pia_ic4ss), write8_delegate(FUNC(pia6821_device::write), pia_ic4ss));
	space->install_read_handler(0x08c0, 0x08c7, 0, 0, read8_delegate(FUNC(ptm6840_device::read), ptm_ic3ss));
	space->install_legacy_write_handler(0x08c0, 0x08c7, 0, 0, FUNC(ic3ss_w));
}

void mpu4_install_mod4bwb_space(address_space *space)
{
	space->install_legacy_write_handler(0x0858, 0x0858, 0, 0, FUNC(bankswitch_w));
	space->install_legacy_write_handler(0x0878, 0x0878, 0, 0, FUNC(bankset_w));
	space->install_legacy_readwrite_handler(0x0810, 0x0810, 0, 0, FUNC(bwb_characteriser_r),FUNC(bwb_characteriser_w));
	mpu4_install_mod4oki_space(space);
}

void mpu4_install_mod4cry_space(address_space *space)
{
	space->install_legacy_write_handler(0x0858, 0x0858, 0, 0, FUNC(bankswitch_w));
	space->install_legacy_write_handler(0x0878, 0x0878, 0, 0, FUNC(bankset_w));
}

void mpu4_config_common(running_machine &machine)
{
	mpu4_state *state = machine.driver_data<mpu4_state>();
	state->m_ic24_timer = machine.scheduler().timer_alloc(FUNC(ic24_timeout));
	state->m_lamp_strobe_ext_persistence = 0;
	/* setup 8 mechanical meters */
	MechMtr_config(machine,8);

	/* setup the standard oki MSC1937 display */
	ROC10937_init(0, MSC1937,0);

}

static void mpu4_config_common_reels(running_machine &machine,int reels)
{
	int n;
	/* setup n default 96 half step reels, using the standard optic flag */
	for ( n = 0; n < reels; n++ )
	{
		stepper_config(machine, n, &barcrest_reel_interface);
	}
	awp_reel_setup();
}

MACHINE_START( mpu4mod2 )
{
	mpu4_state *state = machine.driver_data<mpu4_state>();
	mpu4_config_common(machine);

	state->m_link7a_connected=0;
	state->m_mod_number=2;
}

static MACHINE_START( mpu4yam )
{
	mpu4_state *state = machine.driver_data<mpu4_state>();
	address_space *space = machine.device("maincpu")->memory().space(AS_PROGRAM);
	mpu4_config_common(machine);

	state->m_link7a_connected=0;
	state->m_mod_number=4;
	mpu4_install_mod4yam_space(space);
}

static MACHINE_START( mpu4oki )
{
	mpu4_state *state = machine.driver_data<mpu4_state>();
	address_space *space = machine.device("maincpu")->memory().space(AS_PROGRAM);
	mpu4_config_common(machine);

	state->m_link7a_connected=0;
	state->m_mod_number=4;
	mpu4_install_mod4oki_space(space);
}

static MACHINE_START( mpu4bwb )
{
	mpu4_state *state = machine.driver_data<mpu4_state>();
	address_space *space = machine.device("maincpu")->memory().space(AS_PROGRAM);
	mpu4_config_common(machine);

	state->m_link7a_connected=0;
	state->m_mod_number=4;
	mpu4_install_mod4bwb_space(space);
}

static MACHINE_START( mpu4cry )
{
	mpu4_state *state = machine.driver_data<mpu4_state>();
	address_space *space = machine.device("maincpu")->memory().space(AS_PROGRAM);
	mpu4_config_common(machine);

	state->m_link7a_connected=0;
	state->m_mod_number=4;
	mpu4_install_mod4cry_space(space);
}

/* CHR Tables */

static const mpu4_chr_table ccelbr_data[72] = {
{0x00, 0x00},{0x1a, 0x84},{0x04, 0x8c},{0x10, 0xb8},{0x18, 0x74},{0x0f, 0x80},{0x13, 0x1c},{0x1b, 0xb4},
{0x03, 0xd8},{0x07, 0x74},{0x17, 0x00},{0x1d, 0xd4},{0x36, 0xc8},{0x35, 0x78},{0x2b, 0xa4},{0x28, 0x4c},
{0x39, 0xe0},{0x21, 0xdc},{0x22, 0xf4},{0x25, 0x88},{0x2c, 0x78},{0x29, 0x24},{0x31, 0x84},{0x34, 0xcc},
{0x0a, 0xb8},{0x1f, 0x74},{0x06, 0x90},{0x0e, 0x48},{0x1c, 0xa0},{0x12, 0x1c},{0x1e, 0x24},{0x0d, 0x94},
{0x14, 0xc8},{0x0a, 0xb8},{0x19, 0x74},{0x15, 0x00},{0x06, 0x94},{0x0f, 0x48},{0x08, 0x30},{0x1b, 0x90},
{0x1e, 0x08},{0x04, 0x60},{0x01, 0xd4},{0x0c, 0x58},{0x18, 0xf4},{0x1a, 0x18},{0x11, 0x74},{0x0b, 0x80},
{0x03, 0xdc},{0x17, 0x74},{0x10, 0xd0},{0x1d, 0x58},{0x0e, 0x24},{0x07, 0x94},{0x12, 0xd8},{0x09, 0x34},
{0x0d, 0x90},{0x1f, 0x58},{0x16, 0xf4},{0x05, 0x88},{0x13, 0x38},{0x1c, 0x24},{0x02, 0xd4},{0x00, 0x00},
{0x00, 0x00},{0x01, 0x50},{0x04, 0x00},{0x09, 0x50},{0x10, 0x10},{0x19, 0x40},{0x24, 0x04},{0x31, 0x00}
};

static const mpu4_chr_table gmball_data[72] = {
{0x00, 0x00},{0x1a, 0x0c},{0x04, 0x50},{0x10, 0x90},{0x18, 0xb0},{0x0f, 0x38},{0x13, 0xd4},{0x1b, 0xa0},
{0x03, 0xbc},{0x07, 0xd4},{0x17, 0x30},{0x1d, 0x90},{0x36, 0x38},{0x35, 0xc4},{0x2b, 0xac},{0x28, 0x70},
{0x39, 0x98},{0x21, 0xdc},{0x22, 0xdc},{0x25, 0x54},{0x2c, 0x80},{0x29, 0xb4},{0x31, 0x38},{0x34, 0xcc},
{0x0a, 0xe8},{0x1f, 0xf8},{0x06, 0xd4},{0x0e, 0x30},{0x1c, 0x00},{0x12, 0x84},{0x1e, 0x2c},{0x0d, 0xc8},
{0x14, 0xf8},{0x0a, 0x4c},{0x19, 0x58},{0x15, 0xd4},{0x06, 0xa8},{0x0f, 0x78},{0x08, 0x44},{0x1b, 0x0c},
{0x1e, 0x48},{0x04, 0x50},{0x01, 0x98},{0x0c, 0xd4},{0x18, 0xb0},{0x1a, 0xa0},{0x11, 0xa4},{0x0b, 0x3c},
{0x03, 0xdc},{0x17, 0xd4},{0x10, 0xb8},{0x1d, 0xd4},{0x0e, 0x30},{0x07, 0x88},{0x12, 0xe0},{0x09, 0x24},
{0x0d, 0x8c},{0x1f, 0xf8},{0x16, 0xcc},{0x05, 0x70},{0x13, 0x90},{0x1c, 0x20},{0x02, 0x9c},{0x00, 0x00},
{0x00, 0x00},{0x01, 0x18},{0x04, 0x08},{0x09, 0x10},{0x10, 0x00},{0x19, 0x18},{0x24, 0x08},{0x31, 0x00}
};

static const mpu4_chr_table grtecp_data[72] = {
{0x00, 0x00},{0x1a, 0x84},{0x04, 0xa4},{0x10, 0xac},{0x18, 0x70},{0x0f, 0x80},{0x13, 0x2c},{0x1b, 0xc0},
{0x03, 0xbc},{0x07, 0x5c},{0x17, 0x5c},{0x1d, 0x5c},{0x36, 0xdc},{0x35, 0x5c},{0x2b, 0xcc},{0x28, 0x68},
{0x39, 0xd0},{0x21, 0xb8},{0x22, 0xdc},{0x25, 0x54},{0x2c, 0x08},{0x29, 0x58},{0x31, 0x54},{0x34, 0x90},
{0x0a, 0xb8},{0x1f, 0x5c},{0x06, 0x5c},{0x0e, 0x44},{0x1c, 0x84},{0x12, 0xac},{0x1e, 0xe0},{0x0d, 0xbc},
{0x14, 0xcc},{0x0a, 0xe8},{0x19, 0x70},{0x15, 0x00},{0x06, 0x8c},{0x0f, 0x70},{0x08, 0x00},{0x1b, 0x84},
{0x1e, 0xa4},{0x04, 0xa4},{0x01, 0xbc},{0x0c, 0xdc},{0x18, 0x5c},{0x1a, 0xcc},{0x11, 0xe8},{0x0b, 0xe0},
{0x03, 0xbc},{0x17, 0x4c},{0x10, 0xc8},{0x1d, 0xf8},{0x0e, 0xd4},{0x07, 0xa8},{0x12, 0x68},{0x09, 0x40},
{0x0d, 0x0c},{0x1f, 0xd8},{0x16, 0xdc},{0x05, 0x54},{0x13, 0x98},{0x1c, 0x44},{0x02, 0x9c},{0x00, 0x00},
{0x00, 0x00},{0x01, 0x18},{0x04, 0x00},{0x09, 0x18},{0x10, 0x08},{0x19, 0x10},{0x24, 0x00},{0x31, 0x00}
};

static const mpu4_chr_table oldtmr_data[72] = {
{0x00, 0x00},{0x1a, 0x90},{0x04, 0xc0},{0x10, 0x54},{0x18, 0xa4},{0x0f, 0xf0},{0x13, 0x64},{0x1b, 0x90},
{0x03, 0xe4},{0x07, 0xd4},{0x17, 0x60},{0x1d, 0xb4},{0x36, 0xc0},{0x35, 0x70},{0x2b, 0x80},{0x28, 0x74},
{0x39, 0xa4},{0x21, 0xf4},{0x22, 0xe4},{0x25, 0xd0},{0x2c, 0x64},{0x29, 0x10},{0x31, 0x20},{0x34, 0x90},
{0x0a, 0xe4},{0x1f, 0xf4},{0x06, 0xc4},{0x0e, 0x70},{0x1c, 0x00},{0x12, 0x14},{0x1e, 0x00},{0x0d, 0x14},
{0x14, 0xa0},{0x0a, 0xf0},{0x19, 0x64},{0x15, 0x10},{0x06, 0x84},{0x0f, 0x70},{0x08, 0x00},{0x1b, 0x90},
{0x1e, 0x40},{0x04, 0x90},{0x01, 0xe4},{0x0c, 0xf4},{0x18, 0x64},{0x1a, 0x90},{0x11, 0x64},{0x0b, 0x90},
{0x03, 0xe4},{0x17, 0x50},{0x10, 0x24},{0x1d, 0xb4},{0x0e, 0xe0},{0x07, 0xd4},{0x12, 0xe4},{0x09, 0x50},
{0x0d, 0x04},{0x1f, 0xb4},{0x16, 0xc0},{0x05, 0xd0},{0x13, 0x64},{0x1c, 0x90},{0x02, 0xe4},{0x00, 0x00},
{0x00, 0x00},{0x01, 0x00},{0x04, 0x00},{0x09, 0x00},{0x10, 0x00},{0x19, 0x10},{0x24, 0x00},{0x31, 0x00}
};

static const bwb_chr_table blsbys_data1[5] = {
//Magic number 724A

// PAL Codes
// 0   1   2  3  4  5  6  7  8
// ??  ?? 20 0F 24 3C 36 27 09

	{0x67},{0x17},{0x0f},{0x24},{0x3c},
};

static const mpu4_chr_table blsbys_data[8] = {
{0xEF, 0x02},{0x81, 0x00},{0xCE, 0x00},{0x00, 0x2e},
{0x06, 0x20},{0xC6, 0x0f},{0xF8, 0x24},{0x8E, 0x3c},
};

// set percentage and other options. 2e 20 0f
// PAL Codes
// 0   1   2  3  4  5  6  7  8
// 42  2E 20 0F 24 3C 36 27 09
   //      6  0  7  0  8  0  7  0  0  8
//request 36 42 27 42 09 42 27 42 42 09
//verify  00 04 04 0C 0C 1C 14 2C 5C 2C

static DRIVER_INIT (m_oldtmr)
{
	mpu4_state *state = machine.driver_data<mpu4_state>();
	state->m_reel_mux=SIX_REEL_1TO8;
	state->m_reels = 6;

	stepper_config(machine, 0, &barcrest_opto1_interface);
	stepper_config(machine, 1, &barcrest_opto1_interface);
	stepper_config(machine, 2, &barcrest_opto1_interface);
	stepper_config(machine, 3, &barcrest_opto1_interface);
	stepper_config(machine, 4, &barcrest_opto1_interface);
	stepper_config(machine, 5, &barcrest_opto1_interface);

	awp_reel_setup();
	state->m_current_chr_table = oldtmr_data;
}

static DRIVER_INIT (m_ccelbr)
{
	mpu4_state *state = machine.driver_data<mpu4_state>();
	state->m_reel_mux=STANDARD_REEL;
	state->m_reels = 4;
	// setup 4 default 96 half step reels ///////////////////////////////////
	mpu4_config_common_reels(machine,4);

	state->m_current_chr_table = ccelbr_data;
}

static DRIVER_INIT (m_gmball)
{
	mpu4_state *state = machine.driver_data<mpu4_state>();
	state->m_reel_mux=STANDARD_REEL;
	state->m_reels = 4;
	// setup 4 default 96 half step reels ///////////////////////////////////
	mpu4_config_common_reels(machine,4);

	state->m_current_chr_table = gmball_data;
}

static DRIVER_INIT (m_grtecp)
{
	mpu4_state *state = machine.driver_data<mpu4_state>();
	state->m_reel_mux=FIVE_REEL_5TO8;
	state->m_reels = 5;
	state->m_lamp_extender=SMALL_CARD;
	// setup 4 default 96 half step reels with the mux board
	mpu4_config_common_reels(machine,4);
	stepper_config(machine, 4, &barcrest_reelrev_interface);
	state->m_current_chr_table = grtecp_data;
}

static DRIVER_INIT (m_blsbys)
{
	mpu4_state *state = machine.driver_data<mpu4_state>();
	state->m_bwb_bank=1;
	state->m_reel_mux=FIVE_REEL_5TO8;
	state->m_reels = 5;
	stepper_config(machine, 0, &bwb_opto1_interface);
	stepper_config(machine, 1, &bwb_opto1_interface);
	stepper_config(machine, 2, &bwb_opto1_interface);
	stepper_config(machine, 3, &bwb_opto1_interface);
	stepper_config(machine, 4, &bwb_opto1_interface);
	state->m_bwb_chr_table1 = blsbys_data1;
	state->m_current_chr_table = blsbys_data;
}

static DRIVER_INIT (m4tst2)
{
	mpu4_state *state = machine.driver_data<mpu4_state>();
	state->m_reel_mux=STANDARD_REEL;
	state->m_reels = 4;
	mpu4_config_common_reels(machine,4);
}

static DRIVER_INIT (m4tst)
{
	mpu4_state *state = machine.driver_data<mpu4_state>();
	state->m_reel_mux=STANDARD_REEL;
	state->m_reels = 4;
	mpu4_config_common_reels(machine,4);
}

static DRIVER_INIT (connect4)
{
	mpu4_state *state = machine.driver_data<mpu4_state>();
	state->m_reels = 0; //reel-free game
	state->m_led_lamp=1;
}

static DRIVER_INIT (m_frkstn)
{
  mpu4_state *state = machine.driver_data<mpu4_state>();
  state->m_reel_mux=STANDARD_REEL;
  state->m_reels = 4;

  // setup 4 default 96 half step reels with the mux board
  mpu4_config_common_reels(machine,4);

  //This is a Crystal ROM, needs UPD chip and Project Amber's ROM swizzle.
}

static DRIVER_INIT (m4default)
{
  mpu4_state *state = machine.driver_data<mpu4_state>();
  state->m_reel_mux=STANDARD_REEL;
  state->m_reels = 4;
  mpu4_config_common_reels(machine,4);
}

/* generate a 50 Hz signal (based on an RC time) */
TIMER_DEVICE_CALLBACK( gen_50hz )
{
	mpu4_state *state = timer.machine().driver_data<mpu4_state>();
	/* Although reported as a '50Hz' signal, the fact that both rising and
    falling edges of the pulse are used means the timer actually gives a 100Hz
    oscillating signal.*/
	state->m_signal_50hz = state->m_signal_50hz?0:1;
	timer.machine().device<pia6821_device>("pia_ic4")->ca1_w(state->m_signal_50hz);	/* signal is connected to IC4 CA1 */

	update_meters(state);//run at 100Hz to sync with PIAs
}

static ADDRESS_MAP_START( mpu4_memmap, AS_PROGRAM, 8 )
	AM_RANGE(0x0000, 0x07ff) AM_RAM AM_SHARE("nvram")
	AM_RANGE(0x0800, 0x0810) AM_READWRITE(characteriser_r,characteriser_w)
	AM_RANGE(0x0850, 0x0850) AM_READWRITE(bankswitch_r,bankswitch_w)	/* write bank (rom page select) */
/*  AM_RANGE(0x08e0, 0x08e7) AM_READWRITE(68681_duart_r,68681_duart_w) */ //Runs hoppers
	AM_RANGE(0x0900, 0x0907) AM_DEVREADWRITE_MODERN("ptm_ic2", ptm6840_device, read, write)/* PTM6840 IC2 */
	AM_RANGE(0x0a00, 0x0a03) AM_DEVREADWRITE_MODERN("pia_ic3", pia6821_device, read, write)		/* PIA6821 IC3 */
	AM_RANGE(0x0b00, 0x0b03) AM_DEVREADWRITE_MODERN("pia_ic4", pia6821_device, read, write)		/* PIA6821 IC4 */
	AM_RANGE(0x0c00, 0x0c03) AM_DEVREADWRITE_MODERN("pia_ic5", pia6821_device, read, write)		/* PIA6821 IC5 */
	AM_RANGE(0x0d00, 0x0d03) AM_DEVREADWRITE_MODERN("pia_ic6", pia6821_device, read, write)		/* PIA6821 IC6 */
	AM_RANGE(0x0e00, 0x0e03) AM_DEVREADWRITE_MODERN("pia_ic7", pia6821_device, read, write)		/* PIA6821 IC7 */
	AM_RANGE(0x0f00, 0x0f03) AM_DEVREADWRITE_MODERN("pia_ic8", pia6821_device, read, write)		/* PIA6821 IC8 */
	AM_RANGE(0x1000, 0xffff) AM_ROMBANK("bank1")	/* 64k  paged ROM (4 pages)  */
ADDRESS_MAP_END

const ay8910_interface ay8910_config =
{
	AY8910_SINGLE_OUTPUT,
	{820,0,0},
	DEVCB_NULL,
	DEVCB_NULL,
	DEVCB_NULL,
	DEVCB_NULL
};

MACHINE_CONFIG_FRAGMENT( mpu4_common )
	MCFG_TIMER_ADD_PERIODIC("50hz",gen_50hz, attotime::from_hz(100))

	/* 6840 PTM */
	MCFG_PTM6840_ADD("ptm_ic2", ptm_ic2_intf)

	MCFG_PIA6821_ADD("pia_ic3", pia_ic3_intf)
	MCFG_PIA6821_ADD("pia_ic4", pia_ic4_intf)
	MCFG_PIA6821_ADD("pia_ic5", pia_ic5_intf)
	MCFG_PIA6821_ADD("pia_ic6", pia_ic6_intf)
	MCFG_PIA6821_ADD("pia_ic7", pia_ic7_intf)
	MCFG_PIA6821_ADD("pia_ic8", pia_ic8_intf)
MACHINE_CONFIG_END

MACHINE_CONFIG_FRAGMENT( mpu4_common2 )
	MCFG_PTM6840_ADD("ptm_ic3ss", ptm_ic3ss_intf)
	MCFG_PIA6821_ADD("pia_ic4ss", pia_ic4ss_intf)
MACHINE_CONFIG_END

/* machine driver for MOD 2 board */
MACHINE_CONFIG_START( mpu4mod2, mpu4_state )

	MCFG_MACHINE_START(mpu4mod2)
	MCFG_MACHINE_RESET(mpu4)
	MCFG_CPU_ADD("maincpu", M6809, MPU4_MASTER_CLOCK/4)
	MCFG_CPU_PROGRAM_MAP(mpu4_memmap)


	MCFG_FRAGMENT_ADD(mpu4_common)


	MCFG_SPEAKER_STANDARD_MONO("mono")
	MCFG_SOUND_ADD("ay8913",AY8913, MPU4_MASTER_CLOCK/4)
	MCFG_SOUND_CONFIG(ay8910_config)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 1.0)

	MCFG_NVRAM_ADD_0FILL("nvram")

	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_REFRESH_RATE(60)
	MCFG_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(0))
	MCFG_SCREEN_FORMAT(BITMAP_FORMAT_INDEXED16)
	MCFG_SCREEN_SIZE(64*8, 32*8)
	MCFG_SCREEN_VISIBLE_AREA(8*8, 48*8-1, 2*8, 30*8-1)
	MCFG_PALETTE_LENGTH(0x200)


	MCFG_DEFAULT_LAYOUT(layout_mpu4)
MACHINE_CONFIG_END

static MACHINE_CONFIG_DERIVED( mod4yam, mpu4mod2 )
	MCFG_MACHINE_START(mpu4yam)

	MCFG_DEVICE_REMOVE("ay8913")
	MCFG_SOUND_ADD("ym2413", YM2413, MPU4_MASTER_CLOCK/4)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.5)
MACHINE_CONFIG_END

static MACHINE_CONFIG_DERIVED( mod4oki, mpu4mod2 )
	MCFG_MACHINE_START(mpu4oki)

	MCFG_FRAGMENT_ADD(mpu4_common2)

	MCFG_DEVICE_REMOVE("ay8913")
	MCFG_SOUND_ADD("msm6376", OKIM6376, 128000)		//16KHz sample Can also be 85430 at 10.5KHz and 64000 at 8KHz
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 1.0)
MACHINE_CONFIG_END

static MACHINE_CONFIG_DERIVED( bwboki, mpu4mod2 )
	MCFG_MACHINE_START(mpu4bwb)
	
	MCFG_FRAGMENT_ADD(mpu4_common2)

	MCFG_DEVICE_REMOVE("ay8913")
	MCFG_SOUND_ADD("msm6376", OKIM6376, 128000)		//16KHz sample Can also be 85430 at 10.5KHz and 64000 at 8KHz
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 1.0)
MACHINE_CONFIG_END

static MACHINE_CONFIG_DERIVED(mpu4crys, mpu4mod2 )
	MCFG_MACHINE_START(mpu4cry)

	MCFG_SOUND_ADD("upd", UPD7759, UPD7759_STANDARD_CLOCK)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 1.00)
MACHINE_CONFIG_END



ROM_START( m4oldtmr )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "dot11.bin",  0x00000, 0x10000,  CRC(da095666) SHA1(bc7654dc9da1f830a43f925db8079f27e18bb61e))

	ROM_REGION( 0x200000, "msm6376", 0 )
	ROM_LOAD( "snd.p1",  0x000000, 0x080000,  NO_DUMP )
	ROM_LOAD( "snd.p2",  0x080000, 0x080000,  NO_DUMP )
ROM_END

ROM_START( m4ccelbr )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "cels.p1",  0x00000, 0x10000,  CRC(19d2162f) SHA1(24fe435809352725e7614c32e2184142f355298e))
ROM_END

ROM_START( m4gmball )
	ROM_REGION( 0x10000, "maincpu", ROMREGION_ERASE00  )
	ROM_LOAD( "gbbx.p1",	0x0000, 0x10000,  CRC(0b5adcd0) SHA1(1a198bd4a1e7d6bf4cf025c43d35aaef351415fc))
ROM_END

ROM_START( m4conn4 )
	ROM_REGION( 0x10000, "maincpu", ROMREGION_ERASE00  )
	ROM_LOAD( "connect4.p2",  0x8000, 0x4000,  CRC(6090633c) SHA1(0cd2725a235bf93cfe94f2ca648d5fccb87b8e5c) )
	ROM_LOAD( "connect4.p1",  0xC000, 0x4000,  CRC(b1af50c0) SHA1(7c9645ea378f0857b849ca24a239d9114f62da7f) )
ROM_END

ROM_START( m4tst )
	ROM_REGION( 0x10000, "maincpu", ROMREGION_ERASE00  )
	ROM_LOAD( "ut4.p1",  0xC000, 0x4000,  CRC(086dc325) SHA1(923caeb61347ac9d3e6bcec45998ddf04b2c8ffd))
ROM_END

ROM_START( m4tst2 )
	ROM_REGION( 0x10000, "maincpu", ROMREGION_ERASE00  )
	ROM_LOAD( "ut2.p1",  0xE000, 0x2000,  CRC(f7fb6575) SHA1(f7961cbd0801b9561d8cd2d23081043d733e1902))
ROM_END

ROM_START( m4clr )
	ROM_REGION( 0x10000, "maincpu", ROMREGION_ERASE00  )
	ROM_LOAD( "meter-zero.p1",  0x8000, 0x8000,  CRC(e74297e5) SHA1(49a2cc85eda14199975ec37a794b685c839d3ab9))
ROM_END

ROM_START( m4grtecp )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD("an2k.p1",  0x00000, 0x10000,  CRC(c0886dff) SHA1(ef2b509fde05ef4ef055a09275afc9e153f50efc))

	ROM_REGION( 0x200000, "msm6376", 0 )
	ROM_LOAD( "an2snd.p1",  0x000000, 0x080000,  CRC(5394e9ae) SHA1(86ccd8531fc87f34d3c5482ba7e5a2c06ea69491) )
	ROM_LOAD( "an2snd.p2",  0x080000, 0x080000,  CRC(109ace1f) SHA1(9f0e8065186beb61ed50fea834de2d91e68db953) )
ROM_END

ROM_START( m4blsbys )
	ROM_REGION( 0x80000, "maincpu", 0 )
	ROM_LOAD("bbprog.bin",  0x00000, 0x20000,  CRC(c262cfda) SHA1(f004895e0dd3f8420683927915554e19e41bd20b))
	ROM_RELOAD(0x40000,0x20000)

	ROM_REGION( 0x200000, "msm6376", 0 )
	ROM_LOAD( "bbsnd.p1",  0x000000, 0x080000,  CRC(715c9e95) SHA1(6a0c9c63e56cfc21bf77cf29c1b844b8e0844c1e) )
	ROM_LOAD( "bbsnd.p2",  0x080000, 0x080000,  CRC(594a87f8) SHA1(edfef7d08fab41fb5814c92930f08a565371eae1) )
ROM_END

ROM_START( m4frkstn )
	ROM_REGION( 0x10000, "maincpu", ROMREGION_ERASE00  )
	ROM_LOAD( "fr1.536",  0x8000, 0x8000,  CRC(422b7209) SHA1(3c3f942d375a83d2470467651bca20f0feabdd3b))

	ROM_REGION( 0x40000, "upd", 0 )
	ROM_LOAD("fr1snd.bin",	0x00000, 0x40000, CRC(2d77bbde) SHA1(0397ede538e913dc2972e260589022564fcd8fe4))
ROM_END





ROM_START( m42punlm )
	ROM_REGION( 0x020000, "maincpu", 0 )
	ROM_LOAD( "2pun0-0.bin", 0x0000, 0x020000, CRC(f8fd7b92) SHA1(400a66d0b401b2df2e2fb0f70eae6da7e547a50b) )

	ROM_REGION( 0x080000, "msm6376", 0 )
	ROM_LOAD( "ctnsnd.hex", 0x0000, 0x080000, CRC(150a4513) SHA1(97147e11b49d18225c527d8a0926118a83ee906c))
ROM_END


ROM_START( m4aao )
	ROM_REGION( 0x010000, "maincpu", 0 )
	ROM_LOAD( "aao2_8.bin", 0x0000, 0x010000, CRC(94ce4016) SHA1(2aecb6dbe798b7bbfb3d27f4d115b6611c7d990f) )

	ROM_REGION( 0x080000, "msm6376", 0 )
	ROM_LOAD( "aaosnd.bin", 0x0000, 0x080000, CRC(7bf30b96) SHA1(f0086ae239b1d973018a3ea04e816a87f8f20bad) )
ROM_END


ROM_START( m4apachg )
	ROM_REGION( 0x020000, "maincpu", 0 )
	ROM_LOAD( "ag0_3x.bin", 0x0000, 0x020000, CRC(b521b3fd) SHA1(ffdfd4a67f0eb1665f14274f2abc7f59d0050fe5) )

	ROM_REGION( 0x180000, "msm6376", 0 )
	ROM_LOAD( "agsnd1.bin", 0x000000, 0x080000, CRC(4fcc8cf1) SHA1(339684ad1bf9f58782bb1ec0d1767fc98bb86b0f) )
	ROM_LOAD( "agsnd2.bin", 0x080000, 0x080000, CRC(d2824ef6) SHA1(32bf329c87a8ea7416cfc217519cd963d4d2430d) )
	ROM_LOAD( "agsnd3.bin", 0x100000, 0x080000, CRC(316549d6) SHA1(72fc19cbee363ba7c71801c480cb87ebf9e64e86) )
ROM_END

ROM_START( m4apachga )
	ROM_REGION( 0x020000, "maincpu", 0 )
	ROM_LOAD( "ag0_4.bin", 0x0000, 0x020000, CRC(99415578) SHA1(73f9947ecee575a4f284a2e3837ec6b87ac2c007) )

	ROM_REGION( 0x180000, "msm6376", 0 )
	ROM_LOAD( "agsnd1.bin", 0x000000, 0x080000, CRC(4fcc8cf1) SHA1(339684ad1bf9f58782bb1ec0d1767fc98bb86b0f) )
	ROM_LOAD( "agsnd2.bin", 0x080000, 0x080000, CRC(d2824ef6) SHA1(32bf329c87a8ea7416cfc217519cd963d4d2430d) )
	ROM_LOAD( "agsnd3.bin", 0x100000, 0x080000, CRC(316549d6) SHA1(72fc19cbee363ba7c71801c480cb87ebf9e64e86) )
ROM_END

ROM_START( m4apachgb )
	ROM_REGION( 0x020000, "maincpu", 0 )
	ROM_LOAD( "ag0_4i.bin", 0x0000, 0x020000, CRC(4671a784) SHA1(ea95e82192ad6b53d19cc3c4166b872cd94396d1) )

	ROM_REGION( 0x180000, "msm6376", 0 )
	ROM_LOAD( "agsnd1.bin", 0x000000, 0x080000, CRC(4fcc8cf1) SHA1(339684ad1bf9f58782bb1ec0d1767fc98bb86b0f) )
	ROM_LOAD( "agsnd2.bin", 0x080000, 0x080000, CRC(d2824ef6) SHA1(32bf329c87a8ea7416cfc217519cd963d4d2430d) )
	ROM_LOAD( "agsnd3.bin", 0x100000, 0x080000, CRC(316549d6) SHA1(72fc19cbee363ba7c71801c480cb87ebf9e64e86) )
ROM_END

ROM_START( m4apachgc )
	ROM_REGION( 0x020000, "maincpu", 0 )
	ROM_LOAD( "ag1_1x.bin", 0x0000, 0x020000, CRC(d478cd5f) SHA1(8e0adee7cc88ff072154a0db8ceee94d40046c01) )

	ROM_REGION( 0x180000, "msm6376", 0 )
	ROM_LOAD( "agsnd1.bin", 0x000000, 0x080000, CRC(4fcc8cf1) SHA1(339684ad1bf9f58782bb1ec0d1767fc98bb86b0f) )
	ROM_LOAD( "agsnd2.bin", 0x080000, 0x080000, CRC(d2824ef6) SHA1(32bf329c87a8ea7416cfc217519cd963d4d2430d) )
	ROM_LOAD( "agsnd3.bin", 0x100000, 0x080000, CRC(316549d6) SHA1(72fc19cbee363ba7c71801c480cb87ebf9e64e86) )
ROM_END

ROM_START( m4apachgd )
	ROM_REGION( 0x020000, "maincpu", 0 )
	ROM_LOAD( "ag1_2.bin", 0x0000, 0x020000, CRC(aa857c28) SHA1(5fe95e59f97b2b6a9fa1996d6501c3230f955081) )

	ROM_REGION( 0x180000, "msm6376", 0 )
	ROM_LOAD( "agsnd1.bin", 0x000000, 0x080000, CRC(4fcc8cf1) SHA1(339684ad1bf9f58782bb1ec0d1767fc98bb86b0f) )
	ROM_LOAD( "agsnd2.bin", 0x080000, 0x080000, CRC(d2824ef6) SHA1(32bf329c87a8ea7416cfc217519cd963d4d2430d) )
	ROM_LOAD( "agsnd3.bin", 0x100000, 0x080000, CRC(316549d6) SHA1(72fc19cbee363ba7c71801c480cb87ebf9e64e86) )
ROM_END

ROM_START( m4apachge )
	ROM_REGION( 0x020000, "maincpu", 0 )
	ROM_LOAD( "ag1_2i.bin", 0x0000, 0x020000, CRC(71c3fc7f) SHA1(48000f6068d967504d2bde4d5f9974dd102f7368) )

	ROM_REGION( 0x180000, "msm6376", 0 )
	ROM_LOAD( "agsnd1.bin", 0x000000, 0x080000, CRC(4fcc8cf1) SHA1(339684ad1bf9f58782bb1ec0d1767fc98bb86b0f) )
	ROM_LOAD( "agsnd2.bin", 0x080000, 0x080000, CRC(d2824ef6) SHA1(32bf329c87a8ea7416cfc217519cd963d4d2430d) )
	ROM_LOAD( "agsnd3.bin", 0x100000, 0x080000, CRC(316549d6) SHA1(72fc19cbee363ba7c71801c480cb87ebf9e64e86) )
ROM_END

ROM_START( m4apachgf )
	ROM_REGION( 0x020000, "maincpu", 0 )
	ROM_LOAD( "ag2_0x.bin", 0x0000, 0x020000, CRC(083f9f62) SHA1(67beac70ec79c240bd231279abdc97b6eb1872a5) )

	ROM_REGION( 0x180000, "msm6376", 0 )
	ROM_LOAD( "agsnd1.bin", 0x000000, 0x080000, CRC(4fcc8cf1) SHA1(339684ad1bf9f58782bb1ec0d1767fc98bb86b0f) )
	ROM_LOAD( "agsnd2.bin", 0x080000, 0x080000, CRC(d2824ef6) SHA1(32bf329c87a8ea7416cfc217519cd963d4d2430d) )
	ROM_LOAD( "agsnd3.bin", 0x100000, 0x080000, CRC(316549d6) SHA1(72fc19cbee363ba7c71801c480cb87ebf9e64e86) )
ROM_END



ROM_START( m4bandgd )
	ROM_REGION( 0x020000, "maincpu", 0 )
	ROM_LOAD( "bog.bin", 0x0000, 0x020000, CRC(21186fb9) SHA1(3d536098c7541cbdf02d68a18a38cae71155d7ff) )

	ROM_REGION( 0x080000, "msm6376", 0 )
	ROM_LOAD( "bandsofgoldsnd.bin", 0x0000, 0x080000, CRC(95c6235f) SHA1(a13afa048b73fabfad229b5c2f8ef5ee9948d9fb) )
ROM_END
  
ROM_START( m4bangrs )
	ROM_REGION( 0x010000, "maincpu", 0 )
	ROM_LOAD( "bnc3_0.bin", 0x0000, 0x010000, CRC(c30f947a) SHA1(c734bd966142023e2b7b498ba939972ed32c9fd6) )

	ROM_REGION( 0x100000, "msm6376", 0 )
	ROM_LOAD( "bncsnd1.bin", 0x000000, 0x080000, CRC(593f29c7) SHA1(b743fa6a029b19570ccc31e5108dccec3a752849) )
	ROM_LOAD( "bncsnd2.bin", 0x080000, 0x080000, CRC(589170a9) SHA1(62ec8bfc4d834c07308d5105979b86452340e98b) )
ROM_END

ROM_START( m4bangrsa )
	ROM_REGION( 0x010000, "maincpu", 0 )
	ROM_LOAD( "bnc3_0i.bin", 0x0000, 0x010000, CRC(44afb119) SHA1(5145530fee853c7f63a65566bd1b58b62921dcac) )

	ROM_REGION( 0x100000, "msm6376", 0 )
	ROM_LOAD( "bncsnd1.bin", 0x000000, 0x080000, CRC(593f29c7) SHA1(b743fa6a029b19570ccc31e5108dccec3a752849) )
	ROM_LOAD( "bncsnd2.bin", 0x080000, 0x080000, CRC(589170a9) SHA1(62ec8bfc4d834c07308d5105979b86452340e98b) )
ROM_END

ROM_START( m4bangrsb )
	ROM_REGION( 0x010000, "maincpu", 0 )
	ROM_LOAD( "bnc3_0x.bin", 0x0000, 0x010000, CRC(ff267a9b) SHA1(0b07ef99233df32fdc9621b3f1dbca0549ad99a7) )

	ROM_REGION( 0x100000, "msm6376", 0 )
	ROM_LOAD( "bncsnd1.bin", 0x000000, 0x080000, CRC(593f29c7) SHA1(b743fa6a029b19570ccc31e5108dccec3a752849) )
	ROM_LOAD( "bncsnd2.bin", 0x080000, 0x080000, CRC(589170a9) SHA1(62ec8bfc4d834c07308d5105979b86452340e98b) )
ROM_END


ROM_START( m4bangin )
	ROM_REGION( 0x020000, "maincpu", 0 )
	ROM_LOAD( "bang.hex", 0x0000, 0x020000, CRC(e40f21d3) SHA1(62319967882f01bbd4d10bca52daffd2fe3ec03a) )
	ROM_REGION( 0x48, "chr", 0 )
	ROM_LOAD( "bang.chr", 0x0000, 0x000048, CRC(aacbab22) SHA1(1f394b8947486f319743c0703884ecd35214c433) )

	ROM_REGION( 0x180000, "msm6376", 0 )
	ROM_LOAD( "bangs1.hex", 0x000000, 0x080000, CRC(aa460a90) SHA1(e5e80d8b14bd976ed104e376c2e6f995870b0d77) )
	ROM_LOAD( "bangs2.hex", 0x080000, 0x080000, CRC(518ebd38) SHA1(6eaaf0cb34dd430f16b88f9d1ed97d6fb59d00ea) )
	ROM_LOAD( "bangs3.hex", 0x100000, 0x080000, CRC(2da78a75) SHA1(95975bc76fc32d05bd998bb75dcafc6eef7661b3) )
ROM_END

ROM_START( m4bangina )
	ROM_REGION( 0x020000, "maincpu", 0 )
	ROM_LOAD( "bang1-5n.p1", 0x0000, 0x020000, CRC(dabb462a) SHA1(c02fa204bfab07d5edbc784ccaca50f119ce8d5a) )
	ROM_REGION( 0x48, "chr", 0 )
	ROM_LOAD( "bang.chr", 0x0000, 0x000048, CRC(aacbab22) SHA1(1f394b8947486f319743c0703884ecd35214c433) )

	ROM_REGION( 0x180000, "msm6376", 0 )
	ROM_LOAD( "bangs1.hex", 0x000000, 0x080000, CRC(aa460a90) SHA1(e5e80d8b14bd976ed104e376c2e6f995870b0d77) )
	ROM_LOAD( "bangs2.hex", 0x080000, 0x080000, CRC(518ebd38) SHA1(6eaaf0cb34dd430f16b88f9d1ed97d6fb59d00ea) )
	ROM_LOAD( "bangs3.hex", 0x100000, 0x080000, CRC(2da78a75) SHA1(95975bc76fc32d05bd998bb75dcafc6eef7661b3) )
ROM_END

ROM_START( m4banginb )
	ROM_REGION( 0x020000, "maincpu", 0 )
	ROM_LOAD( "bang1-5p.p1", 0x0000, 0x020000, CRC(84bb8da8) SHA1(ae601957a3cd0b7e4b176987a5592d0b7c9be19d) )
	ROM_REGION( 0x48, "chr", 0 )
	ROM_LOAD( "bang.chr", 0x0000, 0x000048, CRC(aacbab22) SHA1(1f394b8947486f319743c0703884ecd35214c433) )

	ROM_REGION( 0x180000, "msm6376", 0 )
	ROM_LOAD( "bangs1.hex", 0x000000, 0x080000, CRC(aa460a90) SHA1(e5e80d8b14bd976ed104e376c2e6f995870b0d77) )
	ROM_LOAD( "bangs2.hex", 0x080000, 0x080000, CRC(518ebd38) SHA1(6eaaf0cb34dd430f16b88f9d1ed97d6fb59d00ea) )
	ROM_LOAD( "bangs3.hex", 0x100000, 0x080000, CRC(2da78a75) SHA1(95975bc76fc32d05bd998bb75dcafc6eef7661b3) )
ROM_END


ROM_START( m4bankrd )
	ROM_REGION( 0x010000, "maincpu", 0 )
	ROM_LOAD( "br3_1x.bin", 0x0000, 0x010000, CRC(a7bc60b3) SHA1(73fc3c0f775b88ce4f8ccf7d60399371656c2144) )

	ROM_REGION( 0x100000, "msm6376", 0 )
	ROM_LOAD( "raidsnd1.bin", 0x000000, 0x080000, CRC(97427f72) SHA1(ca68bbf9b701a78d69690cddb10bcdcc4214c161) )
	ROM_LOAD( "raidsnd2.bin", 0x080000, 0x080000, CRC(6bc06e6f) SHA1(8f821feeece6fa9b253d3b35c0bc05f0491c359c) )
ROM_END

ROM_START( m4bankrda )
	ROM_REGION( 0x010000, "maincpu", 0 )
	ROM_LOAD( "br0_2.bin", 0x0000, 0x010000, CRC(3368f610) SHA1(6af4f91675228d0bebca0b7fcfd4661c561d1e0b) )

	ROM_REGION( 0x100000, "msm6376", 0 )
	ROM_LOAD( "raidsnd1.bin", 0x000000, 0x080000, CRC(97427f72) SHA1(ca68bbf9b701a78d69690cddb10bcdcc4214c161) )
	ROM_LOAD( "raidsnd2.bin", 0x080000, 0x080000, CRC(6bc06e6f) SHA1(8f821feeece6fa9b253d3b35c0bc05f0491c359c) )
ROM_END

ROM_START( m4bankrdb )
	ROM_REGION( 0x010000, "maincpu", 0 )
	ROM_LOAD( "br1_0i.bin", 0x0000, 0x010000, CRC(545aea13) SHA1(ed8b334ccde1581e4e0b3de15c5d42126cb5a752) )

	ROM_REGION( 0x100000, "msm6376", 0 )
	ROM_LOAD( "raidsnd1.bin", 0x000000, 0x080000, CRC(97427f72) SHA1(ca68bbf9b701a78d69690cddb10bcdcc4214c161) )
	ROM_LOAD( "raidsnd2.bin", 0x080000, 0x080000, CRC(6bc06e6f) SHA1(8f821feeece6fa9b253d3b35c0bc05f0491c359c) )
ROM_END

ROM_START( m4bankrdc )
	ROM_REGION( 0x010000, "maincpu", 0 )
	ROM_LOAD( "br3_0.bin", 0x0000, 0x010000, CRC(3e6e2ede) SHA1(eb2b00e3eb62acef89d55dff0fa814c75b3df701) )

	ROM_REGION( 0x100000, "msm6376", 0 )
	ROM_LOAD( "raidsnd1.bin", 0x000000, 0x080000, CRC(97427f72) SHA1(ca68bbf9b701a78d69690cddb10bcdcc4214c161) )
	ROM_LOAD( "raidsnd2.bin", 0x080000, 0x080000, CRC(6bc06e6f) SHA1(8f821feeece6fa9b253d3b35c0bc05f0491c359c) )
ROM_END

ROM_START( m4bankrdd )
	ROM_REGION( 0x010000, "maincpu", 0 )
	ROM_LOAD( "br3_0i.bin", 0x0000, 0x010000, CRC(b9ce0bbd) SHA1(92144925e0e389db4e0b1dcf88e6fb8d21ada8db) )

	ROM_REGION( 0x100000, "msm6376", 0 )
	ROM_LOAD( "raidsnd1.bin", 0x000000, 0x080000, CRC(97427f72) SHA1(ca68bbf9b701a78d69690cddb10bcdcc4214c161) )
	ROM_LOAD( "raidsnd2.bin", 0x080000, 0x080000, CRC(6bc06e6f) SHA1(8f821feeece6fa9b253d3b35c0bc05f0491c359c) )
ROM_END




ROM_START( m4bigapl )
	ROM_REGION( 0x010000, "maincpu", 0 )
	ROM_LOAD( "app1_5.bin", 0x0000, 0x010000, CRC(ebe6e65f) SHA1(aae70efc4b7e0ad9125424acef634361439e0594) )

	ROM_REGION( 0x100000, "msm6376", 0 )
	ROM_LOAD( "bigapplesnd.p1", 0x000000, 0x080000, CRC(4afba9b0) SHA1(0a60bb5897ed4403c30902ba12290fd61284d0e7) )
	ROM_LOAD( "bigapplesnd.p2", 0x080000, 0x080000, CRC(7de7b74f) SHA1(419998b585e856925d838fe25ecd9f6f41dfc6a8) )
ROM_END

ROM_START( m4bigapla )
	ROM_REGION( 0x010000, "maincpu", 0 )
	ROM_LOAD( "app1_5c", 0x0000, 0x010000, CRC(458e77ce) SHA1(c01f2dd52c67381b4c09051e6a696841ef0777eb) )

	ROM_REGION( 0x100000, "msm6376", 0 )
	ROM_LOAD( "bigapplesnd.p1", 0x000000, 0x080000, CRC(4afba9b0) SHA1(0a60bb5897ed4403c30902ba12290fd61284d0e7) )
	ROM_LOAD( "bigapplesnd.p2", 0x080000, 0x080000, CRC(7de7b74f) SHA1(419998b585e856925d838fe25ecd9f6f41dfc6a8) )
ROM_END

ROM_START( m4bigaplb )
	ROM_REGION( 0x010000, "maincpu", 0 )
	ROM_LOAD( "ba2_0.bin", 0x0000, 0x010000, CRC(2d3db68d) SHA1(9bed97820a527b83f515dca06f032c332cbc11d2) )

	ROM_REGION( 0x100000, "msm6376", 0 )
	ROM_LOAD( "bigapplesnd.p1", 0x000000, 0x080000, CRC(4afba9b0) SHA1(0a60bb5897ed4403c30902ba12290fd61284d0e7) )
	ROM_LOAD( "bigapplesnd.p2", 0x080000, 0x080000, CRC(7de7b74f) SHA1(419998b585e856925d838fe25ecd9f6f41dfc6a8) )
ROM_END

ROM_START( m4bigaplc )
	ROM_REGION( 0x010000, "maincpu", 0 )
	ROM_LOAD( "ba2_0d.bin", 0x0000, 0x010000, CRC(aa9d93ee) SHA1(08c58bcffdf943873a713a006ecb104423b9bb93) )

	ROM_REGION( 0x100000, "msm6376", 0 )
	ROM_LOAD( "bigapplesnd.p1", 0x000000, 0x080000, CRC(4afba9b0) SHA1(0a60bb5897ed4403c30902ba12290fd61284d0e7) )
	ROM_LOAD( "bigapplesnd.p2", 0x080000, 0x080000, CRC(7de7b74f) SHA1(419998b585e856925d838fe25ecd9f6f41dfc6a8) )
ROM_END

ROM_START( m4bigapld )
	ROM_REGION( 0x010000, "maincpu", 0 )
	ROM_LOAD( "ba6_0x.bin", 0x0000, 0x010000, CRC(7c67032f) SHA1(38f44ec527995a850a0b8fe1d9eab4ee9cae06fa) )

	ROM_REGION( 0x100000, "msm6376", 0 )
	ROM_LOAD( "bigapplesnd.p1", 0x000000, 0x080000, CRC(4afba9b0) SHA1(0a60bb5897ed4403c30902ba12290fd61284d0e7) )
	ROM_LOAD( "bigapplesnd.p2", 0x080000, 0x080000, CRC(7de7b74f) SHA1(419998b585e856925d838fe25ecd9f6f41dfc6a8) )
ROM_END

ROM_START( m4bigaple )
	ROM_REGION( 0x010000, "maincpu", 0 )
	ROM_LOAD( "ba7_0x.bin", 0x0000, 0x010000, CRC(f393490a) SHA1(48d23e9deb60fe99f9cbe5601053ee6df2b9bf8b) )

	ROM_REGION( 0x100000, "msm6376", 0 )
	ROM_LOAD( "bigapplesnd.p1", 0x000000, 0x080000, CRC(4afba9b0) SHA1(0a60bb5897ed4403c30902ba12290fd61284d0e7) )
	ROM_LOAD( "bigapplesnd.p2", 0x080000, 0x080000, CRC(7de7b74f) SHA1(419998b585e856925d838fe25ecd9f6f41dfc6a8) )
ROM_END


ROM_START( m4bigben )
	ROM_REGION( 0x010000, "maincpu", 0 )
	ROM_LOAD( "b_bv2_7.bin", 0x0000, 0x010000, CRC(9f3a7638) SHA1(b7169dc26a6e136d6daaf8d012f4c3d017e99e4a) )

	ROM_REGION( 0x100000, "msm6376", 0 )
	ROM_LOAD( "big-bensnd1.bin", 0x000000, 0x080000, CRC(e41c3ec1) SHA1(a0c09f51229afcd14f09bb9080d4f3bb198b2050) )
	ROM_LOAD( "big-bensnd2.bin", 0x080000, 0x080000, CRC(ed71dbe1) SHA1(e67ca3c178caacb99118bacfcd7612e699f40455) )
ROM_END

ROM_START( m4bigbena )
	ROM_REGION( 0x010000, "maincpu", 0 )
	ROM_LOAD( "b_bv2_9.bin", 0x0000, 0x010000, CRC(86a745ee) SHA1(2347e8e38c743ea4d00faee6a56bb77e05c9c94d) )

	ROM_REGION( 0x100000, "msm6376", 0 )
	ROM_LOAD( "big-bensnd1.bin", 0x000000, 0x080000, CRC(e41c3ec1) SHA1(a0c09f51229afcd14f09bb9080d4f3bb198b2050) )
	ROM_LOAD( "big-bensnd2.bin", 0x080000, 0x080000, CRC(ed71dbe1) SHA1(e67ca3c178caacb99118bacfcd7612e699f40455) )
ROM_END

ROM_START( m4bigbenb )
	ROM_REGION( 0x010000, "maincpu", 0 )
	ROM_LOAD( "bb1_9p.bin", 0x0000, 0x010000, CRC(c76c5a09) SHA1(b0e3b38998428f535841ab5373d57cb0d5b21ed3) )

	ROM_REGION( 0x100000, "msm6376", 0 )
	ROM_LOAD( "big-bensnd1.bin", 0x000000, 0x080000, CRC(e41c3ec1) SHA1(a0c09f51229afcd14f09bb9080d4f3bb198b2050) )
	ROM_LOAD( "big-bensnd2.bin", 0x080000, 0x080000, CRC(ed71dbe1) SHA1(e67ca3c178caacb99118bacfcd7612e699f40455) )
ROM_END

ROM_START( m4bigbenc )
	ROM_REGION( 0x010000, "maincpu", 0 )
	ROM_LOAD( "bb2_9.bin", 0x0000, 0x010000, CRC(86a745ee) SHA1(2347e8e38c743ea4d00faee6a56bb77e05c9c94d) )

	ROM_REGION( 0x100000, "msm6376", 0 )
	ROM_LOAD( "big-bensnd1.bin", 0x000000, 0x080000, CRC(e41c3ec1) SHA1(a0c09f51229afcd14f09bb9080d4f3bb198b2050) )
	ROM_LOAD( "big-bensnd2.bin", 0x080000, 0x080000, CRC(ed71dbe1) SHA1(e67ca3c178caacb99118bacfcd7612e699f40455) )
ROM_END

ROM_START( m4bigbend )
	ROM_REGION( 0x010000, "maincpu", 0 )
	ROM_LOAD( "bb_2_1.bin", 0x0000, 0x010000, CRC(d3511805) SHA1(c86756998d36e729874c71a5d6442785069c57e9) )

	ROM_REGION( 0x100000, "msm6376", 0 )
	ROM_LOAD( "big-bensnd1.bin", 0x000000, 0x080000, CRC(e41c3ec1) SHA1(a0c09f51229afcd14f09bb9080d4f3bb198b2050) )
	ROM_LOAD( "big-bensnd2.bin", 0x080000, 0x080000, CRC(ed71dbe1) SHA1(e67ca3c178caacb99118bacfcd7612e699f40455) )
ROM_END

ROM_START( m4bigbene )
	ROM_REGION( 0x010000, "maincpu", 0 )
	ROM_LOAD( "bbs_2_9p.bin", 0x0000, 0x010000, CRC(0107608d) SHA1(9e5def90e77f65c366aea2a9ac24d5f17c4d0ae8) )

	ROM_REGION( 0x100000, "msm6376", 0 )
	ROM_LOAD( "big-bensnd1.bin", 0x000000, 0x080000, CRC(e41c3ec1) SHA1(a0c09f51229afcd14f09bb9080d4f3bb198b2050) )
	ROM_LOAD( "big-bensnd2.bin", 0x080000, 0x080000, CRC(ed71dbe1) SHA1(e67ca3c178caacb99118bacfcd7612e699f40455) )
ROM_END


ROM_START( m4bigchs )
	ROM_REGION( 0x020000, "maincpu", 0 )
	ROM_LOAD( "bc1_2.bin", 0x0000, 0x020000, CRC(9d68a5f7) SHA1(3b7d7af95b9aaca2cbc249402cf1e3b074dc0817) )

	ROM_REGION( 0x180000, "msm6376", 0 )
	ROM_LOAD( "bcsnd1.bin", 0x000000, 0x080000, CRC(7a0a5144) SHA1(84b8a5c58566cf769826023cc221741dd4d6dd0e) )
	ROM_LOAD( "bcsnd2.bin", 0x080000, 0x080000, CRC(9faf37ab) SHA1(03eb4918d7de6e472351a563f2beb652094b98f4) )
	ROM_LOAD( "bcsnd3.bin", 0x100000, 0x080000, CRC(cd6e26de) SHA1(d84274b3b4bc7126e19bf6c6e1aac561a7aaab77) )
ROM_END

ROM_START( m4bigchsa )
	ROM_REGION( 0x020000, "maincpu", 0 )
	ROM_LOAD( "bc1_2i.bin", 0x0000, 0x020000, CRC(462e25a0) SHA1(52e0b6f89a8c933eca0600e776419234c73e4bdc) )

	ROM_REGION( 0x180000, "msm6376", 0 )
	ROM_LOAD( "bcsnd1.bin", 0x000000, 0x080000, CRC(7a0a5144) SHA1(84b8a5c58566cf769826023cc221741dd4d6dd0e) )
	ROM_LOAD( "bcsnd2.bin", 0x080000, 0x080000, CRC(9faf37ab) SHA1(03eb4918d7de6e472351a563f2beb652094b98f4) )
	ROM_LOAD( "bcsnd3.bin", 0x100000, 0x080000, CRC(cd6e26de) SHA1(d84274b3b4bc7126e19bf6c6e1aac561a7aaab77) )
ROM_END

ROM_START( m4bigchsb )
	ROM_REGION( 0x020000, "maincpu", 0 )
	ROM_LOAD( "bc1_2x.bin", 0x0000, 0x020000, CRC(2e418b81) SHA1(489c5a70d289176ad0f66fa630621e24b2c18ce1) )

	ROM_REGION( 0x180000, "msm6376", 0 )
	ROM_LOAD( "bcsnd1.bin", 0x000000, 0x080000, CRC(7a0a5144) SHA1(84b8a5c58566cf769826023cc221741dd4d6dd0e) )
	ROM_LOAD( "bcsnd2.bin", 0x080000, 0x080000, CRC(9faf37ab) SHA1(03eb4918d7de6e472351a563f2beb652094b98f4) )
	ROM_LOAD( "bcsnd3.bin", 0x100000, 0x080000, CRC(cd6e26de) SHA1(d84274b3b4bc7126e19bf6c6e1aac561a7aaab77) )
ROM_END

  

ROM_START( m4blztrl )
	ROM_REGION( 0x010000, "maincpu", 0 )
	ROM_LOAD( "bt1_2.bin", 0x0000, 0x010000, CRC(184f5277) SHA1(52342577a09f787f77f8e026e8f7a11998681fb5) )

	ROM_REGION( 0x180000, "msm6376", 0 )
	ROM_LOAD( "blazingtrails.p1", 0x000000, 0x080000, CRC(2b82701b) SHA1(377fed5d378d6d3abdce59ba9e7f0d7edd756337) )
	ROM_LOAD( "blazingtrails.p2", 0x080000, 0x080000, CRC(9338b7fc) SHA1(9252e94feac0d65f40152bb9d049cea85ecd16fa) )
	ROM_LOAD( "blazingtrails.p3", 0x100000, 0x080000, CRC(ef37f3fa) SHA1(4e71296cd8eb61ff6b4cf9136dbecdcd6d167472) )
ROM_END

ROM_START( m4blztrla )
	ROM_REGION( 0x010000, "maincpu", 0 )
	ROM_LOAD( "bt1_3.bin", 0x0000, 0x010000, CRC(6ae5901d) SHA1(0cd2293ca445549e6c0e0a6f9f6adcf5cdc935b0) )

	ROM_REGION( 0x180000, "msm6376", 0 )
	ROM_LOAD( "blazingtrails.p1", 0x000000, 0x080000, CRC(2b82701b) SHA1(377fed5d378d6d3abdce59ba9e7f0d7edd756337) )
	ROM_LOAD( "blazingtrails.p2", 0x080000, 0x080000, CRC(9338b7fc) SHA1(9252e94feac0d65f40152bb9d049cea85ecd16fa) )
	ROM_LOAD( "blazingtrails.p3", 0x100000, 0x080000, CRC(ef37f3fa) SHA1(4e71296cd8eb61ff6b4cf9136dbecdcd6d167472) )
ROM_END



ROM_START( m4bodymt )
	ROM_REGION( 0x020000, "maincpu", 0 )
	ROM_LOAD( "bm0-1.bin", 0x0000, 0x020000, CRC(15c379d9) SHA1(04d3c869870a4eda4dfd075b1c1a6efb1cf3bf57) )

	ROM_REGION( 0x100000, "msm6376", 0 )
	ROM_LOAD( "bmsnd1.bin", 0x000000, 0x080000, CRC(7f645f66) SHA1(db16e92b6b0c9ac12f7305a4f874a304b93404e6) )
	ROM_LOAD( "bmsnd2.bin", 0x080000, 0x080000, CRC(b8062605) SHA1(570deb41ab8523c5d9b6281a86b915852f6a2305) )
ROM_END


ROM_START( m4boltbl )
	ROM_REGION( 0x010000, "maincpu", 0 )
	ROM_LOAD( "bfb.bin", 0x8000, 0x008000, CRC(63058a6b) SHA1(ebccc647a937c36ffc6c7cfc01389f04f829999c) )
ROM_END

ROM_START( m4boltbla )
	ROM_REGION( 0x010000, "maincpu", 0 )
	ROM_LOAD( "bfb1.1.bin", 0x8000, 0x008000, CRC(7a91122d) SHA1(28229e86feb4411978e556f7f7bd85bfd996b8aa) )
ROM_END

ROM_START( m4boltblb )
	ROM_REGION( 0x010000, "maincpu", 0 )
	ROM_LOAD( "bfb9 5p cash.bin", 0x8000, 0x008000, CRC(792bff34) SHA1(6996e87f22df6bac7bbe9908534b7e0480f03ede) )
ROM_END

ROM_START( m4boltblc )
	ROM_REGION( 0x010000, "maincpu", 0 )
	ROM_LOAD( "bolt-gilwern.bin", 0x8000, 0x008000, CRC(74e2c821) SHA1(1dcdc58585d1dcfc93e2aeb3df0cd41705cde196) )
ROM_END



ROM_START( m4cwalk )
	ROM_REGION( 0x010000, "maincpu", 0 )
	ROM_LOAD( "cw1_1.bin", 0x0000, 0x010000, CRC(a1108d79) SHA1(fa2a5510f2bb2d3811550547bad7c3ef0eb0ddc0) )

	ROM_REGION( 0x100000, "msm6376", 0 )
	ROM_LOAD( "cakesnd1.bin", 0x000000, 0x080000, CRC(abd16cc4) SHA1(744dd1da6c2126bc8673f14aac556af29878f2c4) )
	ROM_LOAD( "cakesnd2.bin", 0x080000, 0x080000, CRC(34da47c0) SHA1(b55f352a7a62172d1dfe990bef39bcbd50e48597) )
ROM_END



ROM_START( m4cstrik )
	ROM_REGION( 0x20000, "maincpu", 0 )
	ROM_LOAD( "cs3_1.bin", 0x0000, 0x020000, CRC(10b68449) SHA1(8e5688b8d240f4ed4429ddfd97366ca4c998b6ab) )

	ROM_REGION( 0x180000, "msm6376", 0 )
	ROM_LOAD( "cssnd1.bin", 0x000000, 0x080000, CRC(a6fedbe0) SHA1(a687151734d378d5c9605be82a22ba50f256885f) )
	ROM_LOAD( "cssnd2.bin", 0x080000, 0x080000, CRC(6160f67c) SHA1(c781d47fe3c6f230442e19ca26523b34808b44a1) )
	ROM_LOAD( "cssnd3.bin", 0x100000, 0x080000, CRC(3911d57a) SHA1(2f0a3a15237876d04b5c9cb72648b27966cd7fb6) )
ROM_END
  
ROM_START( m4cstrika )
	ROM_REGION( 0x20000, "maincpu", 0 )
	ROM_LOAD( "cs3_1i.bin", 0x0000, 0x020000, CRC(cbf0041e) SHA1(9baf9f209f1c4bf59f31437d07051a6cb71e877c) )

	ROM_REGION( 0x180000, "msm6376", 0 )
	ROM_LOAD( "cssnd1.bin", 0x000000, 0x080000, CRC(a6fedbe0) SHA1(a687151734d378d5c9605be82a22ba50f256885f) )
	ROM_LOAD( "cssnd2.bin", 0x080000, 0x080000, CRC(6160f67c) SHA1(c781d47fe3c6f230442e19ca26523b34808b44a1) )
	ROM_LOAD( "cssnd3.bin", 0x100000, 0x080000, CRC(3911d57a) SHA1(2f0a3a15237876d04b5c9cb72648b27966cd7fb6) )
ROM_END
  
ROM_START( m4cstrikb )
	ROM_REGION( 0x20000, "maincpu", 0 )
	ROM_LOAD( "cs3_1x.bin", 0x0000, 0x020000, CRC(d01bda0b) SHA1(528df320593656040b7491a0f3f24cc489b45722) )

	ROM_REGION( 0x180000, "msm6376", 0 )
	ROM_LOAD( "cssnd1.bin", 0x000000, 0x080000, CRC(a6fedbe0) SHA1(a687151734d378d5c9605be82a22ba50f256885f) )
	ROM_LOAD( "cssnd2.bin", 0x080000, 0x080000, CRC(6160f67c) SHA1(c781d47fe3c6f230442e19ca26523b34808b44a1) )
	ROM_LOAD( "cssnd3.bin", 0x100000, 0x080000, CRC(3911d57a) SHA1(2f0a3a15237876d04b5c9cb72648b27966cd7fb6) )
ROM_END
  


ROM_START( m4chacec )
	ROM_REGION( 0x20000, "maincpu", 0 )
	ROM_LOAD( "ca1_0.bin", 0x0000, 0x020000, CRC(0c9a73b7) SHA1(2ee089ce89f29e804371fcfca82bf22a2ac3197b) )

	ROM_REGION( 0x180000, "msm6376", 0 )
	ROM_LOAD( "casnd1.bin", 0x000000, 0x080000, CRC(9bbe8fe4) SHA1(2e5406d72ca731a5960be3a621bbd72064745677) )
	ROM_LOAD( "casnd2.bin", 0x080000, 0x080000, CRC(aa9a45d3) SHA1(47289537451aac1049f7a524b079f2912d97b7cf) )
	ROM_LOAD( "casnd3.bin", 0x100000, 0x080000, CRC(5764e36d) SHA1(6601946bda40886e3a606accd7c11b31efcdab28) )
ROM_END

ROM_START( m4chaceca )
	ROM_REGION( 0x20000, "maincpu", 0 )
	ROM_LOAD( "ca1_0x.bin", 0x0000, 0x020000, CRC(a2476d24) SHA1(12f86733c8fa34d84ff6a1840a24eb96bf547c3d) )

	ROM_REGION( 0x180000, "msm6376", 0 )
	ROM_LOAD( "casnd1.bin", 0x000000, 0x080000, CRC(9bbe8fe4) SHA1(2e5406d72ca731a5960be3a621bbd72064745677) )
	ROM_LOAD( "casnd2.bin", 0x080000, 0x080000, CRC(aa9a45d3) SHA1(47289537451aac1049f7a524b079f2912d97b7cf) )
	ROM_LOAD( "casnd3.bin", 0x100000, 0x080000, CRC(5764e36d) SHA1(6601946bda40886e3a606accd7c11b31efcdab28) )
ROM_END

 

ROM_START( m4chacef )
	ROM_REGION( 0x20000, "maincpu", 0 )
	ROM_LOAD( "ca2-1_0.bin", 0x0000, 0x020000, CRC(c45e650d) SHA1(121e5d178c05d9d38dad167083cb0612f70cbd61) )

	ROM_REGION( 0x180000, "msm6376", 0 )
	ROM_LOAD( "casnd1.bin", 0x000000, 0x080000, CRC(9bbe8fe4) SHA1(2e5406d72ca731a5960be3a621bbd72064745677) )
	ROM_LOAD( "casnd2.bin", 0x080000, 0x080000, CRC(aa9a45d3) SHA1(47289537451aac1049f7a524b079f2912d97b7cf) )
	ROM_LOAD( "casnd3.bin", 0x100000, 0x080000, CRC(5764e36d) SHA1(6601946bda40886e3a606accd7c11b31efcdab28) )
ROM_END

ROM_START( m4chacefa )
	ROM_REGION( 0x20000, "maincpu", 0 )
	ROM_LOAD( "ca2_1.bin", 0x0000, 0x020000, CRC(be069065) SHA1(63c108df781345fdb64dc5177bc28b121b097d3a) )

	ROM_REGION( 0x180000, "msm6376", 0 )
	ROM_LOAD( "casnd1.bin", 0x000000, 0x080000, CRC(9bbe8fe4) SHA1(2e5406d72ca731a5960be3a621bbd72064745677) )
	ROM_LOAD( "casnd2.bin", 0x080000, 0x080000, CRC(aa9a45d3) SHA1(47289537451aac1049f7a524b079f2912d97b7cf) )
	ROM_LOAD( "casnd3.bin", 0x100000, 0x080000, CRC(5764e36d) SHA1(6601946bda40886e3a606accd7c11b31efcdab28) )
ROM_END

ROM_START( m4chacefb )
	ROM_REGION( 0x20000, "maincpu", 0 )
	ROM_LOAD( "ca2_1i.bin", 0x0000, 0x020000, CRC(fceba4d3) SHA1(9e3ba760dc28122f60e610470ce1f7708eefcbfd) )

	ROM_REGION( 0x180000, "msm6376", 0 )
	ROM_LOAD( "casnd1.bin", 0x000000, 0x080000, CRC(9bbe8fe4) SHA1(2e5406d72ca731a5960be3a621bbd72064745677) )
	ROM_LOAD( "casnd2.bin", 0x080000, 0x080000, CRC(aa9a45d3) SHA1(47289537451aac1049f7a524b079f2912d97b7cf) )
	ROM_LOAD( "casnd3.bin", 0x100000, 0x080000, CRC(5764e36d) SHA1(6601946bda40886e3a606accd7c11b31efcdab28) )
ROM_END

ROM_START( m4chacefc )
	ROM_REGION( 0x20000, "maincpu", 0 )
	ROM_LOAD( "ca2_1x.bin", 0x0000, 0x020000, CRC(e1d69187) SHA1(623202d8aaa701709bbcf1fabf0ce6db4fcc18ef) )

	ROM_REGION( 0x180000, "msm6376", 0 )
	ROM_LOAD( "casnd1.bin", 0x000000, 0x080000, CRC(9bbe8fe4) SHA1(2e5406d72ca731a5960be3a621bbd72064745677) )
	ROM_LOAD( "casnd2.bin", 0x080000, 0x080000, CRC(aa9a45d3) SHA1(47289537451aac1049f7a524b079f2912d97b7cf) )
	ROM_LOAD( "casnd3.bin", 0x100000, 0x080000, CRC(5764e36d) SHA1(6601946bda40886e3a606accd7c11b31efcdab28) )
ROM_END




ROM_START( m4coloss )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "col0_6k.bin", 0x0000, 0x010000, CRC(53d2431a) SHA1(44da207ce0ba24d110a1aaf6c0705f9c2245d212) )

	ROM_REGION( 0x100000, "msm6376", 0 )
	ROM_LOAD( "colsnd1.bin", 0x000000, 0x080000, CRC(a6fa2c68) SHA1(e5572b37086a28bee9ce4dfe549ed60ddbffe444) )
	ROM_LOAD( "colsnd2.bin", 0x080000, 0x080000, CRC(8b01f0cb) SHA1(990fb0b51ddf4eb3f436e11d01d0e5e3b2465ac5) )
ROM_END

ROM_START( m4colossa )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "col0_6s.bin", 0x0000, 0x010000, CRC(e4db7ff7) SHA1(0f8a15b40923ac1cf8780b3b99b0ce070ed1d13d) )

	ROM_REGION( 0x100000, "msm6376", 0 )
	ROM_LOAD( "colsnd1.bin", 0x000000, 0x080000, CRC(a6fa2c68) SHA1(e5572b37086a28bee9ce4dfe549ed60ddbffe444) )
	ROM_LOAD( "colsnd2.bin", 0x080000, 0x080000, CRC(8b01f0cb) SHA1(990fb0b51ddf4eb3f436e11d01d0e5e3b2465ac5) )
ROM_END

ROM_START( m4colossb )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "col2_0x.bin", 0x0000, 0x010000, CRC(fae24132) SHA1(91bdafe8bfba2b6b350c783fd46963846ca481c8) )

	ROM_REGION( 0x100000, "msm6376", 0 )
	ROM_LOAD( "colsnd1.bin", 0x000000, 0x080000, CRC(a6fa2c68) SHA1(e5572b37086a28bee9ce4dfe549ed60ddbffe444) )
	ROM_LOAD( "colsnd2.bin", 0x080000, 0x080000, CRC(8b01f0cb) SHA1(990fb0b51ddf4eb3f436e11d01d0e5e3b2465ac5) )
ROM_END

ROM_START( m4colossc )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "col3_0x.bin", 0x0000, 0x010000, CRC(40e0ae7f) SHA1(e6a40c07efbde324091f8a52e615e367ccbb4eaf) )

	ROM_REGION( 0x100000, "msm6376", 0 )
	ROM_LOAD( "colsnd1.bin", 0x000000, 0x080000, CRC(a6fa2c68) SHA1(e5572b37086a28bee9ce4dfe549ed60ddbffe444) )
	ROM_LOAD( "colsnd2.bin", 0x080000, 0x080000, CRC(8b01f0cb) SHA1(990fb0b51ddf4eb3f436e11d01d0e5e3b2465ac5) )
ROM_END

ROM_START( m4colossd )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "col3_1.bin", 0x0000, 0x010000, CRC(79d7f4fb) SHA1(8d4a19fbde135d95b5f0e6978a8b89baa4dfe139) )

	ROM_REGION( 0x100000, "msm6376", 0 )
	ROM_LOAD( "colsnd1.bin", 0x000000, 0x080000, CRC(a6fa2c68) SHA1(e5572b37086a28bee9ce4dfe549ed60ddbffe444) )
	ROM_LOAD( "colsnd2.bin", 0x080000, 0x080000, CRC(8b01f0cb) SHA1(990fb0b51ddf4eb3f436e11d01d0e5e3b2465ac5) )
ROM_END

ROM_START( m4colosse )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "col_0.6.bin", 0x0000, 0x010000, CRC(53d2431a) SHA1(44da207ce0ba24d110a1aaf6c0705f9c2245d212) )

	ROM_REGION( 0x100000, "msm6376", 0 )
	ROM_LOAD( "colsnd1.bin", 0x000000, 0x080000, CRC(a6fa2c68) SHA1(e5572b37086a28bee9ce4dfe549ed60ddbffe444) )
	ROM_LOAD( "colsnd2.bin", 0x080000, 0x080000, CRC(8b01f0cb) SHA1(990fb0b51ddf4eb3f436e11d01d0e5e3b2465ac5) )
ROM_END

ROM_START( m4colossf )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "coll10.bin", 0x0000, 0x010000, CRC(a2468607) SHA1(e926025548e4c0ad1e97b35215b2d28c058126dd) )

	ROM_REGION( 0x100000, "msm6376", 0 )
	ROM_LOAD( "colsnd1.bin", 0x000000, 0x080000, CRC(a6fa2c68) SHA1(e5572b37086a28bee9ce4dfe549ed60ddbffe444) )
	ROM_LOAD( "colsnd2.bin", 0x080000, 0x080000, CRC(8b01f0cb) SHA1(990fb0b51ddf4eb3f436e11d01d0e5e3b2465ac5) )
ROM_END

ROM_START( m4colossg )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "colossus-v1.0.bin", 0x0000, 0x010000, CRC(4fc41c62) SHA1(1c088dd278e414081e98689a49b8305c3d3d4db3) )

	ROM_REGION( 0x100000, "msm6376", 0 )
	ROM_LOAD( "colsnd1.bin", 0x000000, 0x080000, CRC(a6fa2c68) SHA1(e5572b37086a28bee9ce4dfe549ed60ddbffe444) )
	ROM_LOAD( "colsnd2.bin", 0x080000, 0x080000, CRC(8b01f0cb) SHA1(990fb0b51ddf4eb3f436e11d01d0e5e3b2465ac5) )
ROM_END

ROM_START( m4colossh )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "colossus_8.bin", 0x0000, 0x010000, CRC(4ab3ee66) SHA1(2b61b6f9b43592826f7cb755898fcbc4a381f9b3) )

	ROM_REGION( 0x100000, "msm6376", 0 )
	ROM_LOAD( "colsnd1.bin", 0x000000, 0x080000, CRC(a6fa2c68) SHA1(e5572b37086a28bee9ce4dfe549ed60ddbffe444) )
	ROM_LOAD( "colsnd2.bin", 0x080000, 0x080000, CRC(8b01f0cb) SHA1(990fb0b51ddf4eb3f436e11d01d0e5e3b2465ac5) )
ROM_END

ROM_START( m4crzcap )
	ROM_REGION( 0x20000, "maincpu", 0 )
	ROM_LOAD( "cc1_0.bin", 0x0000, 0x020000, CRC(e227690c) SHA1(df236e03d2a22a712cd740ed90b55d48d29aaf65) )

	ROM_REGION( 0x100000, "msm6376", 0 )
	ROM_LOAD( "ccsnd1.bin", 0x000000, 0x080000, CRC(0961a254) SHA1(a6392f00ff6199a1a31395a12695255b9bd67136) )
	ROM_LOAD( "ccsnd2.bin", 0x080000, 0x080000, CRC(555a4a7a) SHA1(552275fcf0bb5476f97ecb37aa2d4431eb3256fa) )
ROM_END

ROM_START( m4crzcapa )
	ROM_REGION( 0x20000, "maincpu", 0 )
	ROM_LOAD( "cc1_0i.bin", 0x0000, 0x020000, CRC(3961e95b) SHA1(09deee5c3d016da1f1f1b81ed9e4edd7e1633a64) )

	ROM_REGION( 0x100000, "msm6376", 0 )
	ROM_LOAD( "ccsnd1.bin", 0x000000, 0x080000, CRC(0961a254) SHA1(a6392f00ff6199a1a31395a12695255b9bd67136) )
	ROM_LOAD( "ccsnd2.bin", 0x080000, 0x080000, CRC(555a4a7a) SHA1(552275fcf0bb5476f97ecb37aa2d4431eb3256fa) )
ROM_END

ROM_START( m4crzcapb )
	ROM_REGION( 0x20000, "maincpu", 0 )
	ROM_LOAD( "cc1_0x.bin", 0x0000, 0x020000, CRC(88a55a48) SHA1(9567a7d08322de21911bad9b7267c7e5041aa1d1) )

	ROM_REGION( 0x100000, "msm6376", 0 )
	ROM_LOAD( "ccsnd1.bin", 0x000000, 0x080000, CRC(0961a254) SHA1(a6392f00ff6199a1a31395a12695255b9bd67136) )
	ROM_LOAD( "ccsnd2.bin", 0x080000, 0x080000, CRC(555a4a7a) SHA1(552275fcf0bb5476f97ecb37aa2d4431eb3256fa) )
ROM_END

ROM_START( m4crzcapc )
	ROM_REGION( 0x20000, "maincpu", 0 )
	ROM_LOAD( "cc2_0x.bin", 0x0000, 0x020000, CRC(ac987339) SHA1(b56f77120a544893a92689060eb46b6faf9c91dc) )

	ROM_REGION( 0x100000, "msm6376", 0 )
	ROM_LOAD( "ccsnd1.bin", 0x000000, 0x080000, CRC(0961a254) SHA1(a6392f00ff6199a1a31395a12695255b9bd67136) )
	ROM_LOAD( "ccsnd2.bin", 0x080000, 0x080000, CRC(555a4a7a) SHA1(552275fcf0bb5476f97ecb37aa2d4431eb3256fa) )
ROM_END

ROM_START( m4crfire )
	ROM_REGION( 0x20000, "maincpu", 0 )
	ROM_LOAD( "cf1_1x.bin", 0x0000, 0x020000, CRC(4267b0f8) SHA1(f0160952af1bfcc08970bb31ba872c8c7e6da996) )

	ROM_REGION( 0x180000, "msm6376", 0 )
	ROM_LOAD( "cfsnd1.bin", 0x000000, 0x080000, CRC(cf7d98b3) SHA1(56448d620ab0c9af5ea0f56c29457b80407aa715) )
	ROM_LOAD( "cfsnd2.bin", 0x080000, 0x080000, CRC(e413643d) SHA1(b3b1862a79efd8c777c472c9b07668343deb51b6) )
	ROM_LOAD( "cfsnd3.bin", 0x100000, 0x080000, CRC(21b51239) SHA1(f8fb9cfc23467d2789474d160038324d366c58f4) )
ROM_END



ROM_START( m4crfirea )
	ROM_REGION( 0x20000, "maincpu", 0 )
	ROM_LOAD( "cf2_1x.bin", 0x0000, 0x020000, CRC(b872d707) SHA1(1565fd8e15d823fc943da7c35347f5c24cde0858) )

	ROM_REGION( 0x180000, "msm6376", 0 )
	ROM_LOAD( "cfsnd1.bin", 0x000000, 0x080000, CRC(cf7d98b3) SHA1(56448d620ab0c9af5ea0f56c29457b80407aa715) )
	ROM_LOAD( "cfsnd2.bin", 0x080000, 0x080000, CRC(e413643d) SHA1(b3b1862a79efd8c777c472c9b07668343deb51b6) )
	ROM_LOAD( "cfsnd3.bin", 0x100000, 0x080000, CRC(21b51239) SHA1(f8fb9cfc23467d2789474d160038324d366c58f4) )
ROM_END



ROM_START( m4dblchn )
	ROM_REGION( 0x010000, "maincpu", 0 )
	ROM_LOAD( "doublechance.bin", 0x0000, 0x010000, CRC(6feeeb7d) SHA1(40fe67d854fbf48959e08fdb5743e14d340c16e7) )

	ROM_REGION( 0x080000, "msm6376", 0 )
	ROM_LOAD( "doublechancesnd.bin", 0x0000, 0x080000, CRC(3e80f8bd) SHA1(2e3a195b49448da11cc0c089a8a9b462894c766b) )
ROM_END


ROM_START( m4eezee )
	ROM_REGION( 0x010000, "maincpu", 0 )
	ROM_LOAD( "ez2511.bin", 0x0000, 0x010000, CRC(86e93c3f) SHA1(a999830685da5d183058769a0598c338c20accdf) )

	ROM_REGION( 0x080000, "msm6376", 0 )
	ROM_LOAD( "eezeefruitssnd.bin", 0x0000, 0x080000, CRC(e6e5aa12) SHA1(0f35eaf0a29050365f53d039e4a7880240c28dc4) )
ROM_END


ROM_START( m4eureka )
	ROM_REGION( 0x020000, "maincpu", 0 )
	ROM_LOAD( "eu1_4i.bin", 0x0000, 0x020000, CRC(2280f25a) SHA1(1898aa5eb73f27b33a902c1696679f6dce115640) )

	ROM_REGION( 0x180000, "msm6376", 0 )
	ROM_LOAD( "eusnd1.bin", 0x000000, 0x080000, CRC(fa35fcf4) SHA1(9869fb16383b7aa93d044cfc6fd864a442f225e7) )
	ROM_LOAD( "eusnd2.bin", 0x080000, 0x080000, CRC(f0218dc1) SHA1(28c149c0f94fe724734b6095b34e54a1e7449f28) )
	ROM_LOAD( "eusnd3.bin", 0x100000, 0x080000, CRC(31e63a47) SHA1(985cedec8945c4bb5dec0ed7d888fb4e291bda8b) )
ROM_END

ROM_START( m4eurekaa )
	ROM_REGION( 0x020000, "maincpu", 0 )
	ROM_LOAD( "eu1_4.bin", 0x0000, 0x020000, CRC(f9c6720d) SHA1(cd477099821a36c9731fdaaea900fe3614614798) )

	ROM_REGION( 0x180000, "msm6376", 0 )
	ROM_LOAD( "eusnd1.bin", 0x000000, 0x080000, CRC(fa35fcf4) SHA1(9869fb16383b7aa93d044cfc6fd864a442f225e7) )
	ROM_LOAD( "eusnd2.bin", 0x080000, 0x080000, CRC(f0218dc1) SHA1(28c149c0f94fe724734b6095b34e54a1e7449f28) )
	ROM_LOAD( "eusnd3.bin", 0x100000, 0x080000, CRC(31e63a47) SHA1(985cedec8945c4bb5dec0ed7d888fb4e291bda8b) )
ROM_END

ROM_START( m4eurekab )
	ROM_REGION( 0x020000, "maincpu", 0 )
	ROM_LOAD( "eu1_4x.bin", 0x0000, 0x020000, CRC(eb2262e4) SHA1(e807f2a5dccb3ceda4edd2a295fcfd7f154bf54d) )

	ROM_REGION( 0x180000, "msm6376", 0 )
	ROM_LOAD( "eusnd1.bin", 0x000000, 0x080000, CRC(fa35fcf4) SHA1(9869fb16383b7aa93d044cfc6fd864a442f225e7) )
	ROM_LOAD( "eusnd2.bin", 0x080000, 0x080000, CRC(f0218dc1) SHA1(28c149c0f94fe724734b6095b34e54a1e7449f28) )
	ROM_LOAD( "eusnd3.bin", 0x100000, 0x080000, CRC(31e63a47) SHA1(985cedec8945c4bb5dec0ed7d888fb4e291bda8b) )
ROM_END



ROM_START( m4firebl )
	ROM_REGION( 0x010000, "maincpu", 0 )
	ROM_LOAD( "fb2_0x.bin", 0x0000, 0x010000, CRC(78b6d22f) SHA1(cb23ac9985a39a6052156732a9be1588207834ce) )
ROM_END

ROM_START( m4firebla )
	ROM_REGION( 0x010000, "maincpu", 0 )
	ROM_LOAD( "fb3_1.bin", 0x0000, 0x010000, CRC(baa497e1) SHA1(5732fbb9b590fc389cad21a40ac264d01461f6e5) )
ROM_END

ROM_START( m4fireblb )
	ROM_REGION( 0x010000, "maincpu", 0 )
	ROM_LOAD( "fb3_3x.bin", 0x0000, 0x010000, CRC(bc4f65cb) SHA1(8397437f6098b246568666e06e8bf40a3f2ea51c) )
ROM_END

ROM_START( m4fireblc )
	ROM_REGION( 0x010000, "maincpu", 0 )
	ROM_LOAD( "fba2_6c", 0x0000, 0x010000, CRC(f2cdbb1f) SHA1(fa12e8992d8af6ded01ab54eedff56ab050e8ee1) )
ROM_END

ROM_START( m4firebld ) // looks weird, bad dump?
	ROM_REGION( 0x080000, "temp", 0 )
	ROM_LOAD( "fba2_6", 0x0000, 0x080000, CRC(cf16b2d0) SHA1(32249fb15708dfa408c4642be7a41fba7aeda657) )

	ROM_REGION( 0x010000, "maincpu", 0 ) 
	// reports src/mame/drivers/mpu4.c: m4firebld has ROM fba2_6 extending past the defined memory region (bug)
	//ROM_LOAD( "fba2_6", 0x0000, 0x010000, CRC(cf16b2d0) SHA1(32249fb15708dfa408c4642be7a41fba7aeda657) )
	//ROM_IGNORE(0x070000)
	ROM_COPY( "temp", 0x0000, 0x0000, 0x010000 )	
ROM_END




ROM_START( m4fright )
	ROM_REGION( 0x020000, "maincpu", 0 )
	ROM_LOAD( "fn4_1x.bin", 0x0000, 0x020000, CRC(f7bb8da6) SHA1(753edb1123d3ea364ded86b566a2c62a039c3d65) )

	ROM_REGION( 0x180000, "msm6376", 0 )
	ROM_LOAD( "fnsnd1.bin", 0x000000, 0x080000, CRC(0f7a6d97) SHA1(f812631af8eb46e188d457d567f42aecceb9e5d2) )
	ROM_LOAD( "fnsnd2.bin", 0x080000, 0x080000, CRC(f2d0c27c) SHA1(4d18049a926898f7fbca54dd30519199fe39f8ea) )
	ROM_LOAD( "fnsnd3.bin", 0x100000, 0x080000, CRC(7ad8aecc) SHA1(8d10a27efbde41af8e04ebe7e8b4b921443bd560) )
ROM_END
  

ROM_START( m4frighta )
	ROM_REGION( 0x020000, "maincpu", 0 )
	ROM_LOAD( "fn4_1.bin", 0x0000, 0x020000, CRC(801c3db2) SHA1(6c3e9b5ac47807196fb7c9e59112fcbd71edb65d) )

	ROM_REGION( 0x180000, "msm6376", 0 )
	ROM_LOAD( "fnsnd1.bin", 0x000000, 0x080000, CRC(0f7a6d97) SHA1(f812631af8eb46e188d457d567f42aecceb9e5d2) )
	ROM_LOAD( "fnsnd2.bin", 0x080000, 0x080000, CRC(f2d0c27c) SHA1(4d18049a926898f7fbca54dd30519199fe39f8ea) )
	ROM_LOAD( "fnsnd3.bin", 0x100000, 0x080000, CRC(7ad8aecc) SHA1(8d10a27efbde41af8e04ebe7e8b4b921443bd560) )
ROM_END

ROM_START( m4frightb )
	ROM_REGION( 0x020000, "maincpu", 0 )
	ROM_LOAD( "fn4_1i.bin", 0x0000, 0x020000, CRC(5b5abde5) SHA1(0583c65755954cc228d32672ea55b7f2afc052c4) )

	ROM_REGION( 0x180000, "msm6376", 0 )
	ROM_LOAD( "fnsnd1.bin", 0x000000, 0x080000, CRC(0f7a6d97) SHA1(f812631af8eb46e188d457d567f42aecceb9e5d2) )
	ROM_LOAD( "fnsnd2.bin", 0x080000, 0x080000, CRC(f2d0c27c) SHA1(4d18049a926898f7fbca54dd30519199fe39f8ea) )
	ROM_LOAD( "fnsnd3.bin", 0x100000, 0x080000, CRC(7ad8aecc) SHA1(8d10a27efbde41af8e04ebe7e8b4b921443bd560) )
ROM_END

ROM_START( m4frightc )
	ROM_REGION( 0x020000, "maincpu", 0 ) // fixed bits, probably the same as one of the others sets anyway, remove?
	ROM_LOAD( "frnt8ac", 0x0000, 0x020000, BAD_DUMP CRC(db081875) SHA1(1e994dd411c81eb9d152b9fa2c3e53258d680dfa) )

	ROM_REGION( 0x180000, "msm6376", 0 )
	ROM_LOAD( "fnsnd1.bin", 0x000000, 0x080000, CRC(0f7a6d97) SHA1(f812631af8eb46e188d457d567f42aecceb9e5d2) )
	ROM_LOAD( "fnsnd2.bin", 0x080000, 0x080000, CRC(f2d0c27c) SHA1(4d18049a926898f7fbca54dd30519199fe39f8ea) )
	ROM_LOAD( "fnsnd3.bin", 0x100000, 0x080000, CRC(7ad8aecc) SHA1(8d10a27efbde41af8e04ebe7e8b4b921443bd560) )
ROM_END


ROM_START( m4frdrop )
	ROM_REGION( 0x010000, "maincpu", 0 )
	ROM_LOAD( "fruitdrop.bin", 0x0000, 0x010000, CRC(7235d3f0) SHA1(e327e28e341ec859f503b71065c40b5d47f448fe) )

	ROM_REGION( 0x080000, "msm6376", 0 )
	ROM_LOAD( "fruitdropsnd.bin", 0x0000, 0x080000, CRC(27880a95) SHA1(6286ab0c342db7de174c3582f56cf9dd60c46883) )
ROM_END


ROM_START( m4gamblr )
	ROM_REGION( 0x010000, "maincpu", 0 )
	ROM_LOAD( "tg4_0k.bin", 0x0000, 0x010000, CRC(d579bd7e) SHA1(b3db3c8a7f30d773a63aab0efe753deacd3db96c) )

	ROM_REGION( 0x100000, "msm6376", 0 )
	ROM_LOAD( "gambsnd1.bin", 0x000000, 0x080000, CRC(a3114336) SHA1(539c896ae512a01340471e2e0df542e582b11258) )
	ROM_LOAD( "gambsnd2.bin", 0x080000, 0x080000, CRC(bc8b78bc) SHA1(6a27804483eaed7912fb6a6e673d1ce9f36371cd) )

	ROM_REGION( 0x100000, "altrevs", 0 )
	// different SFX, does this belong to a specific revision?
	ROM_LOAD( "gambsnd1f.bin", 0x000000, 0x080000, CRC(249ae0fd) SHA1(024ae694f6d09b7f2bf5b94e3a07e9267707f794) )
	ROM_LOAD( "gambsnd2f.bin", 0x080000, 0x080000, CRC(bc8b78bc) SHA1(6a27804483eaed7912fb6a6e673d1ce9f36371cd) )
ROM_END

ROM_START( m4gamblra )
	ROM_REGION( 0x010000, "maincpu", 0 )
	ROM_LOAD( "tg4_0ki.bin", 0x0000, 0x010000, CRC(52d9981d) SHA1(3e30120491d0546b3e19b4b84079cecadd6cdb94) )

	ROM_REGION( 0x100000, "msm6376", 0 )
	ROM_LOAD( "gambsnd1.bin", 0x000000, 0x080000, CRC(a3114336) SHA1(539c896ae512a01340471e2e0df542e582b11258) )
	ROM_LOAD( "gambsnd2.bin", 0x080000, 0x080000, CRC(bc8b78bc) SHA1(6a27804483eaed7912fb6a6e673d1ce9f36371cd) )
ROM_END

ROM_START( m4gamblrb )
	ROM_REGION( 0x010000, "maincpu", 0 )
	ROM_LOAD( "tg4_1x.bin", 0x0000, 0x010000, CRC(e238c6c2) SHA1(6b148221d8c9468efca8eddc0520f4abf5a38200) )

	ROM_REGION( 0x100000, "msm6376", 0 )
	ROM_LOAD( "gambsnd1.bin", 0x000000, 0x080000, CRC(a3114336) SHA1(539c896ae512a01340471e2e0df542e582b11258) )
	ROM_LOAD( "gambsnd2.bin", 0x080000, 0x080000, CRC(bc8b78bc) SHA1(6a27804483eaed7912fb6a6e673d1ce9f36371cd) )
ROM_END


ROM_START( m4gtrain )
	ROM_REGION( 0x010000, "maincpu", 0 )
	ROM_LOAD( "ghosttrainvers3-0.bin", 0x0000, 0x010000, CRC(17f3dd0f) SHA1(0364b4fe3fc273a658feeaecee1ebc0b55a12d98) )

	ROM_REGION( 0x100000, "msm6376", 0 )
	ROM_LOAD( "gtsnd1.bin", 0x000000, 0x080000, CRC(7fd83279) SHA1(65b52330e8d6ccf5c0575924a1791e7d2001c3d8) )
	ROM_LOAD( "gtsnd2.bin", 0x080000, 0x080000, CRC(5bfd0ea2) SHA1(af9adcf517801c775eb316c36538b1bf2262ebb2) )
ROM_END

ROM_START( m4gtraina )
	ROM_REGION( 0x010000, "maincpu", 0 )
	ROM_LOAD( "gt3_0kx.bin", 0x0000, 0x010000, CRC(a6a5461f) SHA1(89652a760a6419064f5c9a52c1bfd066e79e345e) )

	ROM_REGION( 0x100000, "msm6376", 0 )
	ROM_LOAD( "gtsnd1.bin", 0x000000, 0x080000, CRC(7fd83279) SHA1(65b52330e8d6ccf5c0575924a1791e7d2001c3d8) )
	ROM_LOAD( "gtsnd2.bin", 0x080000, 0x080000, CRC(5bfd0ea2) SHA1(af9adcf517801c775eb316c36538b1bf2262ebb2) )
ROM_END

ROM_START( m4gtrainb )
	ROM_REGION( 0x010000, "maincpu", 0 )
	ROM_LOAD( "gt3_1k.bin", 0x0000, 0x010000, CRC(13934116) SHA1(a7e7420e62df0e34a77800e61a9df4ba1e3772c6) )

	ROM_REGION( 0x100000, "msm6376", 0 )
	ROM_LOAD( "gtsnd1.bin", 0x000000, 0x080000, CRC(7fd83279) SHA1(65b52330e8d6ccf5c0575924a1791e7d2001c3d8) )
	ROM_LOAD( "gtsnd2.bin", 0x080000, 0x080000, CRC(5bfd0ea2) SHA1(af9adcf517801c775eb316c36538b1bf2262ebb2) )
ROM_END

ROM_START( m4gtrainc )
	ROM_REGION( 0x010000, "maincpu", 0 )
	ROM_LOAD( "gt3_1ki.bin", 0x0000, 0x010000, CRC(94336475) SHA1(4655631bc65faa82270da606bba8ffcb2d335f26) )

	ROM_REGION( 0x100000, "msm6376", 0 )
	ROM_LOAD( "gtsnd1.bin", 0x000000, 0x080000, CRC(7fd83279) SHA1(65b52330e8d6ccf5c0575924a1791e7d2001c3d8) )
	ROM_LOAD( "gtsnd2.bin", 0x080000, 0x080000, CRC(5bfd0ea2) SHA1(af9adcf517801c775eb316c36538b1bf2262ebb2) )
ROM_END



ROM_START( m4gobana )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "go_b_11_dr_c468.bin", 0x0000, 0x010000, CRC(036776d1) SHA1(73d63731978495cedc5e4c095a83925424ac79c3) )
ROM_END


ROM_START( m4gobanaa )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "go_b_11p_9877.bin", 0x0000, 0x010000, CRC(d985a6eb) SHA1(59576ad9e7589651a6479adb57e8c8bf32a8ef97) )
ROM_END

ROM_START( m4gobanab )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "go_b_11p_dr_c46b.bin", 0x0000, 0x010000, CRC(7343588f) SHA1(e40416a8fdea5a71677493047e569d452ba193df) )
ROM_END

ROM_START( m4gobanac )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "go_b_v11_9873.bin", 0x0000, 0x010000, CRC(3c23581c) SHA1(81daa4903ace4419c1329acbba8907c46b6e6233) )
ROM_END

ROM_START( m4gobanad )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "go_bananas_mecca_v2_31a5.bin", 0x0000, 0x010000, CRC(0c7ae1da) SHA1(9b144a2f1b91def5d9f054fba3d9f1adfe957646) )
ROM_END


ROM_START( m4goldfv )
	ROM_REGION( 0x020000, "maincpu", 0 )
	ROM_LOAD( "gf1_4.bin", 0x0000, 0x020000, CRC(9eb00e69) SHA1(3d04b8c6776bead54d21c0a40d51ed044716897e) )

	ROM_REGION( 0x080000, "msm6376", 0 )
	ROM_LOAD( "gfsnd.bin", 0x0000, 0x080000, CRC(1bb14a13) SHA1(44e888e625cce27bc550a93fce3747885802f5c2) )
ROM_END
  
ROM_START( m4gvibes )
	ROM_REGION( 0x010000, "maincpu", 0 )
	ROM_LOAD( "gv0_1.bin", 0x0000, 0x010000, CRC(ba9a507b) SHA1(ba3b2038b50248ec1f3319e2b39d02313ce3ad08) )

	ROM_REGION( 0x080000, "msm6376", 0 )
	ROM_LOAD( "gvsnd.p1", 0x0000, 0x080000, CRC(ac56b475) SHA1(8017784e5dd8e6d85857ff989c553d04c2ea217a) )

	ROM_REGION( 0x080000, "altrevs", 0 )
	// different SFX, does this belong to a specific revision?
	ROM_LOAD( "gv2snd.bin", 0x0000, 0x080000, CRC(e11ebc9c) SHA1(3f4e8148bc3687af77838b770bbc219a3f50f1c6) )
ROM_END

ROM_START( m4gvibesa )
	ROM_REGION( 0x010000, "maincpu", 0 )
	ROM_LOAD( "gv0_3.bin", 0x0000, 0x010000, CRC(c20e896e) SHA1(9105ba56b9f4b55a158e4dc2b4268a4b93765c88) )

	ROM_REGION( 0x100000, "msm6376", 0 )
	ROM_LOAD( "gvsnd.p1", 0x0000, 0x080000, CRC(ac56b475) SHA1(8017784e5dd8e6d85857ff989c553d04c2ea217a) )
ROM_END


ROM_START( m4haunt )
	ROM_REGION( 0x010000, "maincpu", 0 )
	ROM_LOAD( "hh0.3", 0x0000, 0x010000, CRC(95370728) SHA1(63c7f2fa890c385556a570f3e8941f083a3917bc) )

	ROM_REGION( 0x100000, "msm6376", 0 )
	ROM_LOAD( "hhsnd1.bin", 0x000000, 0x080000, CRC(a2eff4c6) SHA1(86441371b8efbffb93c6c7d02d45cd5dae73ca45) )
	ROM_LOAD( "hhsnd2.bin", 0x080000, 0x080000, CRC(6eb3f52c) SHA1(7b6f7a5bdc5e9937e0b74ce317c951d9ad82425c) )
ROM_END

ROM_START( m4haunta )
	ROM_REGION( 0x010000, "maincpu", 0 )
	ROM_LOAD( "hh0_2x.bin", 0x0000, 0x010000, CRC(3a32332f) SHA1(837b75eb37367ea204c758918ec8eb6370196aa8) )

	ROM_REGION( 0x100000, "msm6376", 0 )
	ROM_LOAD( "hhsnd1.bin", 0x000000, 0x080000, CRC(a2eff4c6) SHA1(86441371b8efbffb93c6c7d02d45cd5dae73ca45) )
	ROM_LOAD( "hhsnd2.bin", 0x080000, 0x080000, CRC(6eb3f52c) SHA1(7b6f7a5bdc5e9937e0b74ce317c951d9ad82425c) )
ROM_END

ROM_START( m4hauntb )
	ROM_REGION( 0x010000, "maincpu", 0 )
	ROM_LOAD( "hh3_0.bin", 0x0000, 0x010000, CRC(f0fc3475) SHA1(24f3ab5990d40b742416f600ffa50e6fc02990ca) )

	ROM_REGION( 0x100000, "msm6376", 0 )
	ROM_LOAD( "hhsnd1.bin", 0x000000, 0x080000, CRC(a2eff4c6) SHA1(86441371b8efbffb93c6c7d02d45cd5dae73ca45) )
	ROM_LOAD( "hhsnd2.bin", 0x080000, 0x080000, CRC(6eb3f52c) SHA1(7b6f7a5bdc5e9937e0b74ce317c951d9ad82425c) )
ROM_END

ROM_START( m4hauntc )
	ROM_REGION( 0x010000, "maincpu", 0 )
	ROM_LOAD( "hh3_0i.bin", 0x0000, 0x010000, CRC(775c1116) SHA1(9fcc9d99b0fc97d98c1a74de7f60e3307ee06448) )

	ROM_REGION( 0x100000, "msm6376", 0 )
	ROM_LOAD( "hhsnd1.bin", 0x000000, 0x080000, CRC(a2eff4c6) SHA1(86441371b8efbffb93c6c7d02d45cd5dae73ca45) )
	ROM_LOAD( "hhsnd2.bin", 0x080000, 0x080000, CRC(6eb3f52c) SHA1(7b6f7a5bdc5e9937e0b74ce317c951d9ad82425c) )
ROM_END

ROM_START( m4hauntd )
	ROM_REGION( 0x010000, "maincpu", 0 )
	ROM_LOAD( "hh3_0x.bin", 0x0000, 0x010000, CRC(42b064db) SHA1(158ec14a34423bea0f9bfb0255ad7b1b2618c9ca) )

	ROM_REGION( 0x100000, "msm6376", 0 )
	ROM_LOAD( "hhsnd1.bin", 0x000000, 0x080000, CRC(a2eff4c6) SHA1(86441371b8efbffb93c6c7d02d45cd5dae73ca45) )
	ROM_LOAD( "hhsnd2.bin", 0x080000, 0x080000, CRC(6eb3f52c) SHA1(7b6f7a5bdc5e9937e0b74ce317c951d9ad82425c) )
ROM_END

ROM_START( m4haunte )
	ROM_REGION( 0x010000, "maincpu", 0 )
	ROM_LOAD( "hs3_0i.bin", 0x0000, 0x010000, CRC(c4d06c05) SHA1(e9256e656c698723158f835a32cdf668ed6120c8) )

	ROM_REGION( 0x100000, "msm6376", 0 )
	ROM_LOAD( "hhsnd1.bin", 0x000000, 0x080000, CRC(a2eff4c6) SHA1(86441371b8efbffb93c6c7d02d45cd5dae73ca45) )
	ROM_LOAD( "hhsnd2.bin", 0x080000, 0x080000, CRC(6eb3f52c) SHA1(7b6f7a5bdc5e9937e0b74ce317c951d9ad82425c) )
ROM_END







ROM_START( m4hisprt )
	ROM_REGION( 0x010000, "maincpu", 0 )
	ROM_LOAD( "hisp1.4", 0x0000, 0x010000, CRC(f80ceefb) SHA1(f8925329f8a1f0f0b61d3de9ebc2d76a7b64be45) )

	ROM_REGION( 0x100000, "msm6376", 0 )
	ROM_LOAD( "highsnd1.bin", 0x000000, 0x080000, CRC(b5084d9c) SHA1(0b59ec1735ccc641f3883746027aab6660fac471) )
	ROM_LOAD( "highsnd2.bin", 0x080000, 0x080000, CRC(0d3b50e9) SHA1(fdca97ec314e2efdd9fcd471ee509fd83f980df6) )

	//different hw
	//ROM_LOAD( "spir0_1e_demo.p1", 0x0000, 0x020000, CRC(016e68db) SHA1(efb9da76b16352588ba9a831210f135b13c0fec9) )
	//ROM_LOAD( "spir0_1e_demo.p2", 0x0000, 0x020000, CRC(49b62046) SHA1(e07db0ce27896af4f508993d935135264cfe0ba1) )
	//ROM_LOAD( "spirsnd_demo.bin", 0x0000, 0x080000, CRC(15c6771f) SHA1(a99f142f53637af361699a73e229dcce224b117f) )
ROM_END

ROM_START( m4hisprta )
	ROM_REGION( 0x010000, "maincpu", 0 )
	ROM_LOAD( "hs1_0x.bin", 0x0000, 0x010000, CRC(0cd54416) SHA1(54c1959ecd0e40b4fd2bce7cbf435f66ddc34626) )

	ROM_REGION( 0x100000, "msm6376", 0 )
	ROM_LOAD( "highsnd1.bin", 0x000000, 0x080000, CRC(b5084d9c) SHA1(0b59ec1735ccc641f3883746027aab6660fac471) )
	ROM_LOAD( "highsnd2.bin", 0x080000, 0x080000, CRC(0d3b50e9) SHA1(fdca97ec314e2efdd9fcd471ee509fd83f980df6) )
ROM_END

ROM_START( m4hisprtb )
	ROM_REGION( 0x010000, "maincpu", 0 )
	ROM_LOAD( "hs1_3.bin", 0x0000, 0x010000, CRC(816101e8) SHA1(654812e4a6cf76787d944abdd914aa5727e06437) )

	ROM_REGION( 0x100000, "msm6376", 0 )
	ROM_LOAD( "highsnd1.bin", 0x000000, 0x080000, CRC(b5084d9c) SHA1(0b59ec1735ccc641f3883746027aab6660fac471) )
	ROM_LOAD( "highsnd2.bin", 0x080000, 0x080000, CRC(0d3b50e9) SHA1(fdca97ec314e2efdd9fcd471ee509fd83f980df6) )
ROM_END


ROM_START( m4hisprtc )
	ROM_REGION( 0x010000, "maincpu", 0 )
	ROM_LOAD( "hs3_0.bin", 0x0000, 0x010000, CRC(43704966) SHA1(78989fa9743efc348f1e81ce040ef9eaf00a47fe) )

	ROM_REGION( 0x100000, "msm6376", 0 )
	ROM_LOAD( "highsnd1.bin", 0x000000, 0x080000, CRC(b5084d9c) SHA1(0b59ec1735ccc641f3883746027aab6660fac471) )
	ROM_LOAD( "highsnd2.bin", 0x080000, 0x080000, CRC(0d3b50e9) SHA1(fdca97ec314e2efdd9fcd471ee509fd83f980df6) )
ROM_END

ROM_START( m4hisprtd )
	ROM_REGION( 0x010000, "maincpu", 0 )
	ROM_LOAD( "hs3_0i.bin", 0x0000, 0x010000, CRC(c4d06c05) SHA1(e9256e656c698723158f835a32cdf668ed6120c8) )

	ROM_REGION( 0x100000, "msm6376", 0 )
	ROM_LOAD( "highsnd1.bin", 0x000000, 0x080000, CRC(b5084d9c) SHA1(0b59ec1735ccc641f3883746027aab6660fac471) )
	ROM_LOAD( "highsnd2.bin", 0x080000, 0x080000, CRC(0d3b50e9) SHA1(fdca97ec314e2efdd9fcd471ee509fd83f980df6) )
ROM_END

ROM_START( m4hisprte )
	ROM_REGION( 0x010000, "maincpu", 0 )
	ROM_LOAD( "hs3_0x.bin", 0x0000, 0x010000, CRC(9a7276f1) SHA1(a683dcf0272d868dbc8be83ad2debcd174453559) )

	ROM_REGION( 0x100000, "msm6376", 0 )
	ROM_LOAD( "highsnd1.bin", 0x000000, 0x080000, CRC(b5084d9c) SHA1(0b59ec1735ccc641f3883746027aab6660fac471) )
	ROM_LOAD( "highsnd2.bin", 0x080000, 0x080000, CRC(0d3b50e9) SHA1(fdca97ec314e2efdd9fcd471ee509fd83f980df6) )
ROM_END


ROM_START( m4hotcsh )
	ROM_REGION( 0x010000, "maincpu", 0 )
	ROM_LOAD( "hc3_0k.bin", 0x0000, 0x010000, CRC(e3cfa94a) SHA1(d21d2dac4edbf3fde9adab399bdd530e034af122) )
	ROM_REGION( 0x48, "chr", 0 )
	ROM_LOAD( "db.chr", 0x00, 0x48, CRC(0fc2bb52) SHA1(0d0e47938f6e00166e7352732ddfb7c610f44db2) )

	ROM_REGION( 0x100000, "msm6376", 0 )
	ROM_LOAD( "hotsnd1.bin", 0x000000, 0x080000, CRC(eef55915) SHA1(b673a05a0313271cc16645f277d37a4a03deced1) )
	ROM_LOAD( "hotsnd2.bin", 0x080000, 0x080000, CRC(92e921ab) SHA1(11e1f3c61a2eddfdcb40f606672d8845000c4ce7) )
ROM_END

ROM_START( m4hotcsha )
	ROM_REGION( 0x010000, "maincpu", 0 )
	ROM_LOAD( "hc3_0ki.bin", 0x0000, 0x010000, CRC(646f8c29) SHA1(19d60faf77a7a83efc3ea4b614a4bc1dee53b8d8) )
	ROM_REGION( 0x48, "chr", 0 )
	ROM_LOAD( "db.chr", 0x00, 0x48, CRC(0fc2bb52) SHA1(0d0e47938f6e00166e7352732ddfb7c610f44db2) )

	ROM_REGION( 0x100000, "msm6376", 0 )
	ROM_LOAD( "hotsnd1.bin", 0x000000, 0x080000, CRC(eef55915) SHA1(b673a05a0313271cc16645f277d37a4a03deced1) )
	ROM_LOAD( "hotsnd2.bin", 0x080000, 0x080000, CRC(92e921ab) SHA1(11e1f3c61a2eddfdcb40f606672d8845000c4ce7) )
ROM_END

ROM_START( m4hotcshb )
	ROM_REGION( 0x010000, "maincpu", 0 )
	ROM_LOAD( "hc3_0kx.bin", 0x0000, 0x010000, CRC(3abebe72) SHA1(fec09ca41e8e43628140456bb44ce6e7c66f5270) )
	ROM_REGION( 0x48, "chr", 0 )
	ROM_LOAD( "db.chr", 0x00, 0x48, CRC(0fc2bb52) SHA1(0d0e47938f6e00166e7352732ddfb7c610f44db2) )

	ROM_REGION( 0x100000, "msm6376", 0 )
	ROM_LOAD( "hotsnd1.bin", 0x000000, 0x080000, CRC(eef55915) SHA1(b673a05a0313271cc16645f277d37a4a03deced1) )
	ROM_LOAD( "hotsnd2.bin", 0x080000, 0x080000, CRC(92e921ab) SHA1(11e1f3c61a2eddfdcb40f606672d8845000c4ce7) )
ROM_END


ROM_START( m4jne )
	ROM_REGION( 0x20000, "maincpu", 0 )
	ROM_LOAD( "jne2_5.bin", 0x0000, 0x020000, CRC(541794df) SHA1(08ce8fa1f9ab715bf8bd55a71fc25deead204026) )

	ROM_REGION( 0x100000, "msm6376", 0 )
	ROM_LOAD( "jnesnd.bin", 0x0000, 0x080000, CRC(47301e22) SHA1(b7ec2ff3b78ceecc0e50142dbbc40929f2526f3f) )
ROM_END


ROM_START( m4kqclub )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "kingsque.p1", 0x8000, 0x008000, CRC(6501e501) SHA1(e289a9418c640415967fafda43f20877b38e3671) )
ROM_END


ROM_START( m4lotty )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "lottytime.bin", 0x0000, 0x010000, CRC(c032c422) SHA1(37c6e10c1fa1cab3de7b4d27ed027604ecea4394) )

	ROM_REGION( 0x080000, "msm6376", 0 )
	ROM_LOAD( "lottytimesnd.bin", 0x0000, 0x080000, CRC(e1966300) SHA1(2a69e39310b49c685bc4307e0396a3b9a0849472) )
ROM_END


ROM_START( m4maxmze )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "maximize_dr_v13_5146.bin", 0x0000, 0x010000, CRC(86507f09) SHA1(e2dbf9b77a5155faeeba05a939ae217289949bce) )
ROM_END

ROM_START( m4maxmzea )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "maximize_dr_v13p_514c.bin", 0x0000, 0x010000, CRC(3d483ae3) SHA1(0687912795a597254815b8f359965bdf2e887be1) )
ROM_END

ROM_START( m4maxmzeb )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "maximize_mecca_v2_50cf.bin", 0x0000, 0x010000, CRC(88426c07) SHA1(2b026d75cbda9375cbfb62f19c25f31c658c56bc) )
ROM_END

ROM_START( m4maxmzec )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "maximize_v13_7107.bin", 0x0000, 0x010000, CRC(ed52f047) SHA1(d6b62934a92dcd6d4df3ac9ae1d9d143000e7c92) )
ROM_END

ROM_START( m4maxmzed )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "maximize_v13p_710b.bin", 0x0000, 0x010000, CRC(1e71a801) SHA1(4cd213e422c91fa10dceea0a93fd81576b1c3f7f) )
ROM_END



ROM_START( m4mayhem )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "may4_0.bin", 0x0000, 0x010000, CRC(fbbe89eb) SHA1(e5e2e4adabfa41d130dbb7a77c147105ef20ac79) )

	ROM_REGION( 0x100000, "msm6376", 0 )
	ROM_LOAD( "mayhemsnd.p1", 0x000000, 0x080000, CRC(f61650ec) SHA1(946fd801ea5f4dd09a911460a709f3942fa412af) )
	ROM_LOAD( "mayhemsnd.p2", 0x080000, 0x080000, CRC(637a6b41) SHA1(d342309b78af21a35f50fb23dd2c7ed737abfdb9) )
ROM_END

ROM_START( m4mayhema )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "mayhemv2.bin", 0x0000, 0x010000, CRC(e7c79dd0) SHA1(b7af1fd4853a6ff33d2f3960737caece91714681) )

	ROM_REGION( 0x100000, "msm6376", 0 )
	ROM_LOAD( "mayhemsnd.p1", 0x000000, 0x080000, CRC(f61650ec) SHA1(946fd801ea5f4dd09a911460a709f3942fa412af) )
	ROM_LOAD( "mayhemsnd.p2", 0x080000, 0x080000, CRC(637a6b41) SHA1(d342309b78af21a35f50fb23dd2c7ed737abfdb9) )
ROM_END


ROM_START( m4mecca )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "mecca_money_dr_v4_ef11.bin", 0x0000, 0x010000, CRC(2429ee8b) SHA1(785304c687a658706b26080f6b8f4bc65830dcea) )
ROM_END

ROM_START( m4themob )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "mob1_5.bin", 0x0000, 0x010000, CRC(adbdbf43) SHA1(650d9af466a258d55d9f6703968501a6eebddfef) )

	ROM_REGION( 0x180000, "msm6376", 0 )
	ROM_LOAD( "mobsnd1.bin", 0x000000, 0x080000, CRC(b15c0eb4) SHA1(5d06853a1218f10741ea43bd72a48a727b62ddb1) )
	ROM_LOAD( "mobsnd2.bin", 0x080000, 0x080000, CRC(6ef3b72c) SHA1(47bf1edacd9da7249e19342234985c746ebf1c4b) )
	ROM_LOAD( "mobsnd3.bin", 0x100000, 0x080000, CRC(62a2bf65) SHA1(155536dc29eecc34351c5726d26f02ee8cb0d014) )
ROM_END

ROM_START( m4themoba )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "mob1_5d.bin", 0x0000, 0x010000, CRC(2a1d9a20) SHA1(1cd07edcf75f17a98b6377fbfdf88fd7c0abd864) )

	ROM_REGION( 0x180000, "msm6376", 0 )
	ROM_LOAD( "mobsnd1.bin", 0x000000, 0x080000, CRC(b15c0eb4) SHA1(5d06853a1218f10741ea43bd72a48a727b62ddb1) )
	ROM_LOAD( "mobsnd2.bin", 0x080000, 0x080000, CRC(6ef3b72c) SHA1(47bf1edacd9da7249e19342234985c746ebf1c4b) )
	ROM_LOAD( "mobsnd3.bin", 0x100000, 0x080000, CRC(62a2bf65) SHA1(155536dc29eecc34351c5726d26f02ee8cb0d014) )
ROM_END


ROM_START( m4themobb )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "mob1_6.bin", 0x0000, 0x010000, CRC(9c718009) SHA1(99b259d5a93f4657ad2b4ae6cd0b2e1324178022) )

	ROM_REGION( 0x180000, "msm6376", 0 )
	ROM_LOAD( "mobsnd1.bin", 0x000000, 0x080000, CRC(b15c0eb4) SHA1(5d06853a1218f10741ea43bd72a48a727b62ddb1) )
	ROM_LOAD( "mobsnd2.bin", 0x080000, 0x080000, CRC(6ef3b72c) SHA1(47bf1edacd9da7249e19342234985c746ebf1c4b) )
	ROM_LOAD( "mobsnd3.bin", 0x100000, 0x080000, CRC(62a2bf65) SHA1(155536dc29eecc34351c5726d26f02ee8cb0d014) )
ROM_END



ROM_START( m4monspn )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "ms2_0.bin", 0x0000, 0x010000, CRC(c20172a8) SHA1(0fb97258dd33fa7ff83b8082149069aaf3577480) )

	ROM_REGION( 0x100000, "msm6376", 0 )
	ROM_LOAD( "mssnd1.bin", 0x000000, 0x080000, CRC(e8f0e818) SHA1(a874981fc980a0ed49352f5bf89caf80176b3865) )
	ROM_LOAD( "mssnd2.bin", 0x080000, 0x080000, CRC(636c329d) SHA1(27503035ea57c7e03a9a07dfc58da997c47dda34) )
ROM_END

ROM_START( m4monspna )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "ms2_0i.bin", 0x0000, 0x010000, CRC(45a157cb) SHA1(619e05bc0ee01bdb3254269619f761b513d77ee8) )

	ROM_REGION( 0x100000, "msm6376", 0 )
	ROM_LOAD( "mssnd1.bin", 0x000000, 0x080000, CRC(e8f0e818) SHA1(a874981fc980a0ed49352f5bf89caf80176b3865) )
	ROM_LOAD( "mssnd2.bin", 0x080000, 0x080000, CRC(636c329d) SHA1(27503035ea57c7e03a9a07dfc58da997c47dda34) )
ROM_END

ROM_START( m4monspnb )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "ms2_0x.bin", 0x0000, 0x010000, CRC(e9e40e01) SHA1(487c11c03bfa582424b680d204417eb5e85abfb4) )

	ROM_REGION( 0x100000, "msm6376", 0 )
	ROM_LOAD( "mssnd1.bin", 0x000000, 0x080000, CRC(e8f0e818) SHA1(a874981fc980a0ed49352f5bf89caf80176b3865) )
	ROM_LOAD( "mssnd2.bin", 0x080000, 0x080000, CRC(636c329d) SHA1(27503035ea57c7e03a9a07dfc58da997c47dda34) )
ROM_END

ROM_START( m4nudbon )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "nb4_0.bin", 0x0000, 0x010000, CRC(c7802b0f) SHA1(6ebff8ccbd2baf6de55d764d2e7cb34f2ff2384f) )

// something else? or sound related?
//	ROM_LOAD( "nubnzdl1.bin", 0x0000, 0x002000, CRC(bae682d2) SHA1(650082af9210b0af8b08870a4cdf4196035ea8a5) )
//  ROM_LOAD( "nubnzdl2.bin", 0x0000, 0x002000, CRC(ff150af7) SHA1(02e25200560e8435ebbf19c3ae9c3e9cf00342c1) )
//  ROM_LOAD( "nubnzdl3.bin", 0x0000, 0x004000, CRC(450d7fc9) SHA1(f82acb017e765f7188a874dade6fd1a5d6b2033e) )
ROM_END

ROM_START( m4nudbona )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "nubonza.bin", 0x0000, 0x010000, CRC(ce8b1c9b) SHA1(3f4e019256ddbd668c8cadecf86015b78b5eaf8c) )
ROM_END



ROM_START( m4nudgem )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "gem2_0.bin", 0x0000, 0x010000, CRC(bbab66b8) SHA1(a3a7d40d0ca41e57cd0d6965c0306edca372da1d) )

	ROM_REGION( 0x080000, "msm6376", 0 )
	ROM_LOAD( "nudgejems.p1", 0x0000, 0x080000, CRC(e875d82e) SHA1(50fb941ad801397ef3dee651be126c01c9423386) )
ROM_END

ROM_START( m4nudgema )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "nudgejemsprg.p1", 0x0000, 0x010000, CRC(bbab66b8) SHA1(a3a7d40d0ca41e57cd0d6965c0306edca372da1d) )

	ROM_REGION( 0x080000, "msm6376", 0 )
	ROM_LOAD( "nudgejems.p1", 0x0000, 0x080000, CRC(e875d82e) SHA1(50fb941ad801397ef3dee651be126c01c9423386) )
ROM_END




ROM_START( m4pbnudg )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "pinball nudger v0-1", 0x0000, 0x010000, CRC(8d2e5ded) SHA1(51d4ea44d4e8a7bd53f321fd677b12f6bafbc721) )

	ROM_REGION( 0x080000, "msm6376", 0 )
	ROM_LOAD( "pinsnd.bin", 0x0000, 0x080000, CRC(30f61dcb) SHA1(c844272ffc264d6dabe1958ef57d10d1ba0c2b1e) )
ROM_END

ROM_START( m4pbnudga )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "pn0_2.bin", 0x0000, 0x010000, CRC(d2aab1e0) SHA1(203257c50f79df46561ece5116277a8d56552b04) )

	ROM_REGION( 0x080000, "msm6376", 0 )
	ROM_LOAD( "pinsnd.bin", 0x0000, 0x080000, CRC(30f61dcb) SHA1(c844272ffc264d6dabe1958ef57d10d1ba0c2b1e) )
ROM_END

ROM_START( m4pbnudgb )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "pn1_0.bin", 0x0000, 0x010000, CRC(95dabff1) SHA1(846577f76c6c99cb05f3aab88de80c3373e570c0) )

	ROM_REGION( 0x080000, "msm6376", 0 )
	ROM_LOAD( "pinsnd.bin", 0x0000, 0x080000, CRC(30f61dcb) SHA1(c844272ffc264d6dabe1958ef57d10d1ba0c2b1e) )
ROM_END


ROM_START( m4pitfal )
	ROM_REGION( 0x20000, "maincpu", 0 )
	ROM_LOAD( "pf1_3.bin", 0x0000, 0x020000, CRC(5bdacadf) SHA1(3f48faf92ef25ecbb20f6af90adf5bffdaf8bad8) )

	ROM_REGION( 0x180000, "msm6376", 0 )
	ROM_LOAD( "pfsnd1.bin", 0x000000, 0x080000, CRC(50ad158e) SHA1(0efcb9f5683cbe5bdec1e13791d8a01cdfcb5f1a) )
	ROM_LOAD( "pfsnd2.bin", 0x080000, 0x080000, CRC(80ad0d94) SHA1(2c1c60b681cc80624f8bd120034639f1cce06cc5) )
	ROM_LOAD( "pfsnd3.bin", 0x100000, 0x080000, CRC(abc0a0da) SHA1(3048edf44a31d58794a3ee1dad1399559bf14211) )
ROM_END

ROM_START( m4pitfala )
	ROM_REGION( 0x20000, "maincpu", 0 )
	ROM_LOAD( "pf1_4x.bin", 0x0000, 0x020000, CRC(42cd08bc) SHA1(69a5a5158d78c51e188520dbb16a0a89566ea8b3) )

	ROM_REGION( 0x180000, "msm6376", 0 )
	ROM_LOAD( "pfsnd1.bin", 0x000000, 0x080000, CRC(50ad158e) SHA1(0efcb9f5683cbe5bdec1e13791d8a01cdfcb5f1a) )
	ROM_LOAD( "pfsnd2.bin", 0x080000, 0x080000, CRC(80ad0d94) SHA1(2c1c60b681cc80624f8bd120034639f1cce06cc5) )
	ROM_LOAD( "pfsnd3.bin", 0x100000, 0x080000, CRC(abc0a0da) SHA1(3048edf44a31d58794a3ee1dad1399559bf14211) )
ROM_END

ROM_START( m4pitfalb )
	ROM_REGION( 0x20000, "maincpu", 0 )
	ROM_LOAD( "pf2_0.bin", 0x0000, 0x020000, CRC(159f1ac6) SHA1(5af44ac650b9408afedc7533c2b3e558a84eb727) )

	ROM_REGION( 0x180000, "msm6376", 0 )
	ROM_LOAD( "pfsnd1.bin", 0x000000, 0x080000, CRC(50ad158e) SHA1(0efcb9f5683cbe5bdec1e13791d8a01cdfcb5f1a) )
	ROM_LOAD( "pfsnd2.bin", 0x080000, 0x080000, CRC(80ad0d94) SHA1(2c1c60b681cc80624f8bd120034639f1cce06cc5) )
	ROM_LOAD( "pfsnd3.bin", 0x100000, 0x080000, CRC(abc0a0da) SHA1(3048edf44a31d58794a3ee1dad1399559bf14211) )
ROM_END

ROM_START( m4pitfalc )
	ROM_REGION( 0x20000, "maincpu", 0 )
	ROM_LOAD( "pf2_0x.bin", 0x0000, 0x020000, CRC(ba59a3f9) SHA1(1e2f21c67e8ca41a1cb8e9c412cf911b62511e05) )

	ROM_REGION( 0x180000, "msm6376", 0 )
	ROM_LOAD( "pfsnd1.bin", 0x000000, 0x080000, CRC(50ad158e) SHA1(0efcb9f5683cbe5bdec1e13791d8a01cdfcb5f1a) )
	ROM_LOAD( "pfsnd2.bin", 0x080000, 0x080000, CRC(80ad0d94) SHA1(2c1c60b681cc80624f8bd120034639f1cce06cc5) )
	ROM_LOAD( "pfsnd3.bin", 0x100000, 0x080000, CRC(abc0a0da) SHA1(3048edf44a31d58794a3ee1dad1399559bf14211) )
ROM_END



ROM_START( m4purmad )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "pmad.p1", 0x0000, 0x010000, CRC(e430510b) SHA1(bb8d443429aa7d39a99de5cd1387154398b74d9c) )

	ROM_REGION( 0x080000, "msm6376", 0 )
	ROM_LOAD( "pmadsnd.bin", 0x0000, 0x080000, CRC(88312507) SHA1(64e386c3c9b3f82a390777b61c7207567cc962e7) )
ROM_END


ROM_START( m4revolv )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "revolva.bin", 0x0000, 0x010000, CRC(e195feac) SHA1(dc8d736784819fd15f0e7e29e9f91cf1c601ebb9) )

	ROM_REGION( 0x100000, "msm6376", 0 )
	ROM_LOAD( "revolvasnd.p1", 0x000000, 0x080000, CRC(a9138bdf) SHA1(aaa5b3e82a8d89776b636e46ada9aae8a4febaab) )
	ROM_LOAD( "revolvasnd.p2", 0x080000, 0x080000, CRC(5ae7b26a) SHA1(a17cef58e37bccb5954b9acab01f92ff21375ab1) )
ROM_END


ROM_START( m4rckrol )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "rock0_1.hex", 0x0000, 0x010000, CRC(2c786a34) SHA1(3800af04550f12e8f58b1929b49e21572f19d589) )

	ROM_REGION( 0x080000, "msm6376", 0 )
	ROM_LOAD( "rocksnd.bin", 0x0000, 0x080000, CRC(c3e96650) SHA1(71952267d3149786cfef1dd49cc070664bb007a4) )
ROM_END

ROM_START( m4rckrola )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "rock1.hex", 0x0000, 0x010000, CRC(b7dfd181) SHA1(566531bcb9d0b7d3caebed3f7c96f5fb23a7cef2) )

	ROM_REGION( 0x080000, "msm6376", 0 )
	ROM_LOAD( "rocksnd.bin", 0x0000, 0x080000, CRC(c3e96650) SHA1(71952267d3149786cfef1dd49cc070664bb007a4) )
ROM_END

ROM_START( m4rckrolb )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "rock3.bin", 0x0000, 0x010000, CRC(c5134d03) SHA1(c4dfa43ffd077690d0b9a20cb82bbf92a1dc1ac9) )

	ROM_REGION( 0x080000, "msm6376", 0 )
	ROM_LOAD( "rocksnd.bin", 0x0000, 0x080000, CRC(c3e96650) SHA1(71952267d3149786cfef1dd49cc070664bb007a4) )
ROM_END







ROM_START( m4rotex )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "rotex1_2.bin", 0x0000, 0x010000, CRC(202e794d) SHA1(15b7459db7f3a5317a92138d997e9817d4367750) )

	ROM_REGION( 0x080000, "msm6376", 0 )
	ROM_LOAD( "rotsnd.bin", 0x0000, 0x080000, CRC(02f8c2e2) SHA1(33a051e7af6d8c33708f7b4e0654d66312eeede5) )
ROM_END




ROM_START( m4select )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "selectawp.bin", 0x0000, 0x010000, CRC(e248539c) SHA1(b1db8fdc221c70d2a699cb86cea5681527d7d06a) )

	ROM_REGION( 0x080000, "msm6376", 0 )
	ROM_LOAD( "selectsnd.bin", 0x0000, 0x080000, CRC(93fd4253) SHA1(69feda7ffc56defd515c9cd1ce204af3d9731a3f) )
ROM_END



ROM_START( m4smshgb )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "sm2_0x.bin", 0x0000, 0x010000, CRC(52042750) SHA1(2fb5ece50aef457bdbdc1fb880b1a87638551545) )

	ROM_REGION( 0x100000, "msm6376", 0 )
	ROM_LOAD( "sngsnd.p1", 0x000000, 0x080000, CRC(0bff940e) SHA1(e53df95cd33d759f89f0278312e6e5f9b8abe341) )
	ROM_LOAD( "sngsnd.p2", 0x080000, 0x080000, CRC(414aa5a5) SHA1(c6e6bba8c4655dc761dd040c4d615f9d4a739663) )
ROM_END

ROM_START( m4smshgba )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "sm3_0x.bin", 0x0000, 0x010000, CRC(9a4bb2bd) SHA1(8642b15f658e855c0d682fe84b024ddd85eb527e) )

	ROM_REGION( 0x100000, "msm6376", 0 )
	ROM_LOAD( "sngsnd.p1", 0x000000, 0x080000, CRC(0bff940e) SHA1(e53df95cd33d759f89f0278312e6e5f9b8abe341) )
	ROM_LOAD( "sngsnd.p2", 0x080000, 0x080000, CRC(414aa5a5) SHA1(c6e6bba8c4655dc761dd040c4d615f9d4a739663) )
ROM_END

ROM_START( m4smshgbb )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "sm6_0.bin", 0x0000, 0x010000, CRC(7a58d60f) SHA1(989c816dadf01500be01e2c333befef0c3e12054) )

	ROM_REGION( 0x100000, "msm6376", 0 )
	ROM_LOAD( "sngsnd.p1", 0x000000, 0x080000, CRC(0bff940e) SHA1(e53df95cd33d759f89f0278312e6e5f9b8abe341) )
	ROM_LOAD( "sngsnd.p2", 0x080000, 0x080000, CRC(414aa5a5) SHA1(c6e6bba8c4655dc761dd040c4d615f9d4a739663) )
ROM_END

ROM_START( m4smshgbc )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "sma5_8c", 0x0000, 0x010000, CRC(79e12ac0) SHA1(9e8d4ea8f97d1f73ccab079cbf57aa89a12d2e7d) )

	ROM_REGION( 0x100000, "msm6376", 0 )
	ROM_LOAD( "sngsnd.p1", 0x000000, 0x080000, CRC(0bff940e) SHA1(e53df95cd33d759f89f0278312e6e5f9b8abe341) )
	ROM_LOAD( "sngsnd.p2", 0x080000, 0x080000, CRC(414aa5a5) SHA1(c6e6bba8c4655dc761dd040c4d615f9d4a739663) )
ROM_END



ROM_START( m4snklad )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "snakesnladders.bin", 0x0000, 0x010000, CRC(52bdc684) SHA1(00d052629b214b48a9a1d68622ba188206276166) )
ROM_END





ROM_START( m4snookr )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "snooker.ts2", 0x8000, 0x004000, CRC(a6906eb3) SHA1(43b91e88f909b758f880d83df4f889f15aa17eb3) )
 	ROM_LOAD( "snooker.ts1", 0xc000, 0x004000, CRC(3e3072dd) SHA1(9ea8b270044b48767a2e6c19e8ed257d5491c1d0) )
ROM_END


ROM_START( m4spnwin )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "saw.bin", 0x0000, 0x010000, CRC(f8aac65f) SHA1(2cf8402bffe1638bddc0c2dd145d7be3cc7bd02b) )
ROM_END



ROM_START( m4stakex )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "stakex.bin", 0x0000, 0x010000, CRC(098c7117) SHA1(27f04cfb88ef870fc30afd055cf32ffe448275ea) )

	ROM_REGION( 0x080000, "msm6376", 0 )
	ROM_LOAD( "stakexsnd.bin", 0x0000, 0x080000, CRC(baf17991) SHA1(282e0ac9d18299e9f7a0fecaf9edf0cb4205ef0e) )
ROM_END

ROM_START( m4stakexa )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "stakex2.bin", 0x0000, 0x010000, CRC(77ae3f63) SHA1(c5f1cfd5bffcf3156f584757de57ef6530214511) )

	ROM_REGION( 0x080000, "msm6376", 0 )
	ROM_LOAD( "stakexsnd.bin", 0x0000, 0x080000, CRC(baf17991) SHA1(282e0ac9d18299e9f7a0fecaf9edf0cb4205ef0e) )
ROM_END


ROM_START( m4stand2 )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "stand 2 del 8.bin", 0x08000, 0x08000, CRC(a9a5edc7) SHA1(035d3f3b3373cec475753f1b0de2f4db48d6d288) )
ROM_END



ROM_START( m4supfru )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "supa11l.bin", 0x0000, 0x010000, CRC(6cdbf08a) SHA1(4c0faf7144b9ac19c8d55b81ce51b519570b0d1f) )

	ROM_REGION( 0x080000, "msm6376", 0 )
	ROM_LOAD( "supafruitssnd.bin", 0x0000, 0x080000, CRC(b70184ca) SHA1(dba2204cb606f0c6dad8a4c46fbbb1beb5b5e31c) )
ROM_END


ROM_START( m4supfrua )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "supafruits.bin", 0x0000, 0x010000, CRC(c6242e33) SHA1(810be1fb99ddb810c5506a974c9214ce23426a87) )

	ROM_REGION( 0x080000, "msm6376", 0 )
	ROM_LOAD( "supafruitssnd.bin", 0x0000, 0x080000, CRC(b70184ca) SHA1(dba2204cb606f0c6dad8a4c46fbbb1beb5b5e31c) )
ROM_END

ROM_START( m4sstrek )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "rhr2pprgpatched.bin", 0x0000, 0x010000, CRC(a0b3439d) SHA1(0976537a5170bf4c4f595f7fa04243a68f14b2ae) )
ROM_END

ROM_START( m4ttrail )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "tt3_0.bin", 0x0000, 0x010000, CRC(62f31f70) SHA1(e1b50b98cc90513c9fa06d0ea8f70aa45bddc0e6) )
	
	ROM_REGION( 0x100000, "msm6376", 0 )
	ROM_LOAD( "ttsnd1.bin", 0x000000, 0x080000, CRC(6af0c76f) SHA1(8587b499b88b609e48553e610a0ee539f98b70ce) )
	ROM_LOAD( "ttsnd2.bin", 0x080000, 0x080000, CRC(9f243ed1) SHA1(c4b83a9b788e4fa2065ff7a270f0dcdecb125e66) )
ROM_END

ROM_START( m4ttraila )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "tt3_0i.bin", 0x0000, 0x010000, CRC(e5533a13) SHA1(0d23503d32c8156112676aaddece1a44614230eb) )
	
	ROM_REGION( 0x100000, "msm6376", 0 )
	ROM_LOAD( "ttsnd1.bin", 0x000000, 0x080000, CRC(6af0c76f) SHA1(8587b499b88b609e48553e610a0ee539f98b70ce) )
	ROM_LOAD( "ttsnd2.bin", 0x080000, 0x080000, CRC(9f243ed1) SHA1(c4b83a9b788e4fa2065ff7a270f0dcdecb125e66) )
ROM_END

ROM_START( m4ttrailb )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "tt3_0x.bin", 0x0000, 0x010000, CRC(7d00e4ae) SHA1(6bb30af001fc73e354c17a99633b6fa4c50b374d) )
	
	ROM_REGION( 0x100000, "msm6376", 0 )
	ROM_LOAD( "ttsnd1.bin", 0x000000, 0x080000, CRC(6af0c76f) SHA1(8587b499b88b609e48553e610a0ee539f98b70ce) )
	ROM_LOAD( "ttsnd2.bin", 0x080000, 0x080000, CRC(9f243ed1) SHA1(c4b83a9b788e4fa2065ff7a270f0dcdecb125e66) )
ROM_END

ROM_START( m4trimad )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "tmad1_1p", 0x000000, 0x010000, CRC(7a49546b) SHA1(cfce6fda74682e6c60a5731fee44c41c5f5bbbeb) )

	ROM_REGION( 0x180000, "msm6376", 0 )
	ROM_LOAD( "triple.s1", 0x000000, 0x080000, CRC(e808f0ae) SHA1(4d88c1d8ed9396629509a0c7614e7510401f1325) )
	ROM_LOAD( "triple.s2", 0x080000, 0x080000, CRC(40161063) SHA1(a24edb311fea466c0c15aebce40044f8db448e50) )
	ROM_LOAD( "triple.s3", 0x100000, 0x080000, CRC(f9109afb) SHA1(4e4a863b60915ddb2865c12af19cc38bcad6d062) )
ROM_END

ROM_START( m4unibox )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "unibox.bin", 0x0000, 0x010000, CRC(9e99eed4) SHA1(c59f9ab2cae487991b0202bd18c6ab514ba3b29d) )

	ROM_REGION( 0x80000, "msm6376", 0 )
	ROM_LOAD( "uniboxsnd.bin", 0x0000, 0x080000, CRC(cce8fda1) SHA1(e9d6514dc2badb046201ea44802690cf104e6075) )
ROM_END

ROM_START( m4uniboxa )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "unibox1.bin", 0x0000, 0x010000, CRC(dd8fa3bd) SHA1(34719608e429e6ef0cc0ea9a75df7b00439e53ed) )

	ROM_REGION( 0x80000, "msm6376", 0 )
	ROM_LOAD( "uniboxsnd.bin", 0x0000, 0x080000, CRC(cce8fda1) SHA1(e9d6514dc2badb046201ea44802690cf104e6075) )
ROM_END

ROM_START( m4unique )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "unique.bin", 0x000000, 0x010000, CRC(ac8c83da) SHA1(11912c1b9a028e17db2395ca85792c4d10fae3ad) )

	ROM_REGION( 0x80000, "msm6376", 0 )
	ROM_LOAD( "uniquesnd.bin", 0x000000, 0x080000, CRC(177a4c07) SHA1(180f51ba982ccf7d19dfa50e94c295395799c360) )
ROM_END

ROM_START( m4uniquep )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "uniquepatched64k.bin", 0x000000, 0x010000, CRC(889575a2) SHA1(dcd0830d75cb4ddc901f22d71a28836e55d969cc) )

	ROM_REGION( 0x80000, "msm6376", 0 )
	ROM_LOAD( "uniquesnd.bin", 0x000000, 0x080000, CRC(177a4c07) SHA1(180f51ba982ccf7d19dfa50e94c295395799c360) )
ROM_END

/* Barcrest */
GAME( 198?, m4tst,        0, mpu4mod2,   mpu4,       m4tst,   ROT0, "Barcrest","MPU4 Unit Test (Program 4)",GAME_MECHANICAL )
GAME( 198?, m4tst2,       0, mpu4mod2,   mpu4,       m4tst2,  ROT0, "Barcrest","MPU4 Unit Test (Program 2)",GAME_MECHANICAL )
GAME( 198?, m4clr,        0, mpu4mod2,   mpu4,       0,       ROT0, "Barcrest","MPU4 Meter Clear ROM",GAME_MECHANICAL )

/* Empire
   most of these boot (after a single reset to initialize)
   but have broken text, I guess due to the characterizer (protection) */

GAME(199?, m4apachg,  0,		mod4oki, mpu4, m4default, ROT0,   "Empire","Apache Gold (Empire) (MPU4, set 1)",   GAME_NOT_WORKING|GAME_REQUIRES_ARTWORK|GAME_NO_SOUND )
GAME(199?, m4apachga, m4apachg,	mod4oki, mpu4, m4default, ROT0,   "Empire","Apache Gold (Empire) (MPU4, set 2)",   GAME_NOT_WORKING|GAME_REQUIRES_ARTWORK|GAME_NO_SOUND )
GAME(199?, m4apachgb, m4apachg,	mod4oki, mpu4, m4default, ROT0,   "Empire","Apache Gold (Empire) (MPU4, set 3)",   GAME_NOT_WORKING|GAME_REQUIRES_ARTWORK|GAME_NO_SOUND )
GAME(199?, m4apachgc, m4apachg,	mod4oki, mpu4, m4default, ROT0,   "Empire","Apache Gold (Empire) (MPU4, set 4)",   GAME_NOT_WORKING|GAME_REQUIRES_ARTWORK|GAME_NO_SOUND )
GAME(199?, m4apachgd, m4apachg,	mod4oki, mpu4, m4default, ROT0,   "Empire","Apache Gold (Empire) (MPU4, set 5)",   GAME_NOT_WORKING|GAME_REQUIRES_ARTWORK|GAME_NO_SOUND )
GAME(199?, m4apachge, m4apachg,	mod4oki, mpu4, m4default, ROT0,   "Empire","Apache Gold (Empire) (MPU4, set 6)",   GAME_NOT_WORKING|GAME_REQUIRES_ARTWORK|GAME_NO_SOUND )
GAME(199?, m4apachgf, m4apachg,	mod4oki, mpu4, m4default, ROT0,   "Empire","Apache Gold (Empire) (MPU4, set 7)",   GAME_NOT_WORKING|GAME_REQUIRES_ARTWORK|GAME_NO_SOUND )
GAME(199?, m4bangrs,  0,		mod4oki, mpu4, m4default, ROT0,   "Empire","Bangers 'n' Cash (Empire) (MPU4, set 1)",   GAME_NOT_WORKING|GAME_REQUIRES_ARTWORK|GAME_NO_SOUND )
GAME(199?, m4bangrsa, m4bangrs,	mod4oki, mpu4, m4default, ROT0,   "Empire","Bangers 'n' Cash (Empire) (MPU4, set 2)",   GAME_NOT_WORKING|GAME_REQUIRES_ARTWORK|GAME_NO_SOUND )
GAME(199?, m4bangrsb, m4bangrs,	mod4oki, mpu4, m4default, ROT0,   "Empire","Bangers 'n' Cash (Empire) (MPU4, set 3)",   GAME_NOT_WORKING|GAME_REQUIRES_ARTWORK|GAME_NO_SOUND )
GAME(199?, m4bankrd,  0,		mod4oki, mpu4, m4default, ROT0,   "Empire","Bank Raid (Empire) (MPU4, set 1)",   GAME_NOT_WORKING|GAME_REQUIRES_ARTWORK|GAME_NO_SOUND )
GAME(199?, m4bankrda, m4bankrd,	mod4oki, mpu4, m4default, ROT0,   "Empire","Bank Raid (Empire) (MPU4, set 2)",   GAME_NOT_WORKING|GAME_REQUIRES_ARTWORK|GAME_NO_SOUND )
GAME(199?, m4bankrdb, m4bankrd,	mod4oki, mpu4, m4default, ROT0,   "Empire","Bank Raid (Empire) (MPU4, set 3)",   GAME_NOT_WORKING|GAME_REQUIRES_ARTWORK|GAME_NO_SOUND )
GAME(199?, m4bankrdc, m4bankrd,	mod4oki, mpu4, m4default, ROT0,   "Empire","Bank Raid (Empire) (MPU4, set 4)",   GAME_NOT_WORKING|GAME_REQUIRES_ARTWORK|GAME_NO_SOUND )
GAME(199?, m4bankrdd, m4bankrd,	mod4oki, mpu4, m4default, ROT0,   "Empire","Bank Raid (Empire) (MPU4, set 5)",   GAME_NOT_WORKING|GAME_REQUIRES_ARTWORK|GAME_NO_SOUND )
GAME(199?, m4bigchs,  0,		mod4oki, mpu4, m4default, ROT0,   "Empire","Big Cheese (Empire) (MPU4, set 1)",   GAME_NOT_WORKING|GAME_REQUIRES_ARTWORK|GAME_NO_SOUND )
GAME(199?, m4bigchsa, m4bigchs,	mod4oki, mpu4, m4default, ROT0,   "Empire","Big Cheese (Empire) (MPU4, set 2)",   GAME_NOT_WORKING|GAME_REQUIRES_ARTWORK|GAME_NO_SOUND )
GAME(199?, m4bigchsb, m4bigchs,	mod4oki, mpu4, m4default, ROT0,   "Empire","Big Cheese (Empire) (MPU4, set 3)",   GAME_NOT_WORKING|GAME_REQUIRES_ARTWORK|GAME_NO_SOUND )
GAME(199?, m4cstrik,  0,		mod4oki, mpu4, m4default, ROT0,   "Empire","Cash Strike (Empire) (MPU4, set 1)",   GAME_NOT_WORKING|GAME_REQUIRES_ARTWORK|GAME_NO_SOUND )
GAME(199?, m4cstrika, m4cstrik,	mod4oki, mpu4, m4default, ROT0,   "Empire","Cash Strike (Empire) (MPU4, set 2)",   GAME_NOT_WORKING|GAME_REQUIRES_ARTWORK|GAME_NO_SOUND )
GAME(199?, m4cstrikb, m4cstrik,	mod4oki, mpu4, m4default, ROT0,   "Empire","Cash Strike (Empire) (MPU4, set 3)",   GAME_NOT_WORKING|GAME_REQUIRES_ARTWORK|GAME_NO_SOUND )
GAME(199?, m4chacec,  0,		mod4oki, mpu4, m4default, ROT0,   "Empire","Chase The Ace [Cards] (Empire) (MPU4, set 1)",   GAME_NOT_WORKING|GAME_REQUIRES_ARTWORK|GAME_NO_SOUND )
GAME(199?, m4chaceca, m4chacec,	mod4oki, mpu4, m4default, ROT0,   "Empire","Chase The Ace [Cards] (Empire) (MPU4, set 2)",   GAME_NOT_WORKING|GAME_REQUIRES_ARTWORK|GAME_NO_SOUND )
GAME(199?, m4chacef,  0,		mod4oki, mpu4, m4default, ROT0,   "Empire","Chase The Ace [Fruits] (Empire) (MPU4, set 1)",   GAME_NOT_WORKING|GAME_REQUIRES_ARTWORK|GAME_NO_SOUND )
GAME(199?, m4chacefa, m4chacef,	mod4oki, mpu4, m4default, ROT0,   "Empire","Chase The Ace [Fruits] (Empire) (MPU4, set 2)",   GAME_NOT_WORKING|GAME_REQUIRES_ARTWORK|GAME_NO_SOUND )
GAME(199?, m4chacefb, m4chacef,	mod4oki, mpu4, m4default, ROT0,   "Empire","Chase The Ace [Fruits] (Empire) (MPU4, set 3)",   GAME_NOT_WORKING|GAME_REQUIRES_ARTWORK|GAME_NO_SOUND )
GAME(199?, m4chacefc, m4chacef,	mod4oki, mpu4, m4default, ROT0,   "Empire","Chase The Ace [Fruits] (Empire) (MPU4, set 4)",   GAME_NOT_WORKING|GAME_REQUIRES_ARTWORK|GAME_NO_SOUND )
GAME(199?, m4crzcap,  0,		mod4oki, mpu4, m4default, ROT0,   "Empire","Crazy Capers (Empire) (MPU4, set 1)",   GAME_NOT_WORKING|GAME_REQUIRES_ARTWORK|GAME_NO_SOUND )
GAME(199?, m4crzcapa, m4crzcap,	mod4oki, mpu4, m4default, ROT0,   "Empire","Crazy Capers (Empire) (MPU4, set 2)",   GAME_NOT_WORKING|GAME_REQUIRES_ARTWORK|GAME_NO_SOUND )
GAME(199?, m4crzcapb, m4crzcap,	mod4oki, mpu4, m4default, ROT0,   "Empire","Crazy Capers (Empire) (MPU4, set 3)",   GAME_NOT_WORKING|GAME_REQUIRES_ARTWORK|GAME_NO_SOUND )
GAME(199?, m4crzcapc, m4crzcap,	mod4oki, mpu4, m4default, ROT0,   "Empire","Crazy Capers (Empire) (MPU4, set 4)",   GAME_NOT_WORKING|GAME_REQUIRES_ARTWORK|GAME_NO_SOUND )
GAME(199?, m4crfire,  0,		mod4oki, mpu4, m4default, ROT0,   "Empire","Crossfire (Empire) (MPU4, set 1)",   GAME_NOT_WORKING|GAME_REQUIRES_ARTWORK|GAME_NO_SOUND ) // doesn't boot, alarm
GAME(199?, m4crfirea, m4crfire,	mod4oki, mpu4, m4default, ROT0,   "Empire","Crossfire (Empire) (MPU4, set 2)",   GAME_NOT_WORKING|GAME_REQUIRES_ARTWORK|GAME_NO_SOUND ) // doesn't boot, alarm
GAME(199?, m4eureka,  0,		mod4oki, mpu4, m4default, ROT0,   "Empire","Eureka (Empire) (MPU4, set 1)",   GAME_NOT_WORKING|GAME_REQUIRES_ARTWORK|GAME_NO_SOUND )
GAME(199?, m4eurekaa, m4eureka,	mod4oki, mpu4, m4default, ROT0,   "Empire","Eureka (Empire) (MPU4, set 2)",   GAME_NOT_WORKING|GAME_REQUIRES_ARTWORK|GAME_NO_SOUND )
GAME(199?, m4eurekab, m4eureka,	mod4oki, mpu4, m4default, ROT0,   "Empire","Eureka (Empire) (MPU4, set 3)",   GAME_NOT_WORKING|GAME_REQUIRES_ARTWORK|GAME_NO_SOUND )
GAME(199?, m4fright,  0,		mod4oki, mpu4, m4default, ROT0,   "Empire","Fright Night (Empire) (MPU4, set 1)",   GAME_NOT_WORKING|GAME_REQUIRES_ARTWORK|GAME_NO_SOUND )
GAME(199?, m4frighta, m4fright,	mod4oki, mpu4, m4default, ROT0,   "Empire","Fright Night (Empire) (MPU4, set 2)",   GAME_NOT_WORKING|GAME_REQUIRES_ARTWORK|GAME_NO_SOUND )
GAME(199?, m4frightb, m4fright,	mod4oki, mpu4, m4default, ROT0,   "Empire","Fright Night (Empire) (MPU4, set 3)",   GAME_NOT_WORKING|GAME_REQUIRES_ARTWORK|GAME_NO_SOUND )
GAME(199?, m4frightc, m4fright,	mod4oki, mpu4, m4default, ROT0,   "Empire","Fright Night (Empire) (MPU4, set 4)",   GAME_NOT_WORKING|GAME_REQUIRES_ARTWORK|GAME_NO_SOUND ) // bad dump
GAME(199?, m4gamblr,  0,		mod4oki, mpu4, m4default, ROT0,   "Empire","The Gambler (Empire) (MPU4, set 1)",   GAME_NOT_WORKING|GAME_REQUIRES_ARTWORK|GAME_NO_SOUND )
GAME(199?, m4gamblra, m4gamblr, mod4oki, mpu4, m4default, ROT0,   "Empire","The Gambler (Empire) (MPU4, set 2)",   GAME_NOT_WORKING|GAME_REQUIRES_ARTWORK|GAME_NO_SOUND )
GAME(199?, m4gamblrb, m4gamblr, mod4oki, mpu4, m4default, ROT0,   "Empire","The Gambler (Empire) (MPU4, set 3)",   GAME_NOT_WORKING|GAME_REQUIRES_ARTWORK|GAME_NO_SOUND )
GAME(199?, m4gtrain,  0,		mod4oki, mpu4, m4default, ROT0,   "Empire","Ghost Train (Empire) (MPU4, set 1)",   GAME_NOT_WORKING|GAME_REQUIRES_ARTWORK|GAME_NO_SOUND )
GAME(199?, m4gtraina, m4gtrain,	mod4oki, mpu4, m4default, ROT0,   "Empire","Ghost Train (Empire) (MPU4, set 2)",   GAME_NOT_WORKING|GAME_REQUIRES_ARTWORK|GAME_NO_SOUND )
GAME(199?, m4gtrainb, m4gtrain,	mod4oki, mpu4, m4default, ROT0,   "Empire","Ghost Train (Empire) (MPU4, set 3)",   GAME_NOT_WORKING|GAME_REQUIRES_ARTWORK|GAME_NO_SOUND )
GAME(199?, m4gtrainc, m4gtrain,	mod4oki, mpu4, m4default, ROT0,   "Empire","Ghost Train (Empire) (MPU4, set 4)",   GAME_NOT_WORKING|GAME_REQUIRES_ARTWORK|GAME_NO_SOUND )
GAME(199?, m4goldfv,  0,		mod4oki, mpu4, m4default, ROT0,   "Empire","Gold Fever (Empire) (MPU4)",   GAME_NOT_WORKING|GAME_REQUIRES_ARTWORK|GAME_NO_SOUND )
GAME(199?, m4haunt,   0,		mod4oki, mpu4, m4default, ROT0,   "Empire","Haunted House (Empire) (MPU4, set 1)",   GAME_NOT_WORKING|GAME_REQUIRES_ARTWORK|GAME_NO_SOUND )
GAME(199?, m4haunta,  m4haunt,	mod4oki, mpu4, m4default, ROT0,   "Empire","Haunted House (Empire) (MPU4, set 2)",   GAME_NOT_WORKING|GAME_REQUIRES_ARTWORK|GAME_NO_SOUND )
GAME(199?, m4hauntb,  m4haunt,	mod4oki, mpu4, m4default, ROT0,   "Empire","Haunted House (Empire) (MPU4, set 3)",   GAME_NOT_WORKING|GAME_REQUIRES_ARTWORK|GAME_NO_SOUND )
GAME(199?, m4hauntc,  m4haunt,	mod4oki, mpu4, m4default, ROT0,   "Empire","Haunted House (Empire) (MPU4, set 4)",   GAME_NOT_WORKING|GAME_REQUIRES_ARTWORK|GAME_NO_SOUND )
GAME(199?, m4hauntd,  m4haunt,	mod4oki, mpu4, m4default, ROT0,   "Empire","Haunted House (Empire) (MPU4, set 5)",   GAME_NOT_WORKING|GAME_REQUIRES_ARTWORK|GAME_NO_SOUND )
GAME(199?, m4haunte,  m4haunt,	mod4oki, mpu4, m4default, ROT0,   "Empire","Haunted House (Empire) (MPU4, set 6)",   GAME_NOT_WORKING|GAME_REQUIRES_ARTWORK|GAME_NO_SOUND )
GAME(199?, m4hisprt,  0,		mod4oki, mpu4, m4default, ROT0,   "Empire","High Spirits (Empire) (MPU4, set 1)",   GAME_NOT_WORKING|GAME_REQUIRES_ARTWORK|GAME_NO_SOUND )
GAME(199?, m4hisprta, m4hisprt,	mod4oki, mpu4, m4default, ROT0,   "Empire","High Spirits (Empire) (MPU4, set 2)",   GAME_NOT_WORKING|GAME_REQUIRES_ARTWORK|GAME_NO_SOUND )
GAME(199?, m4hisprtb, m4hisprt,	mod4oki, mpu4, m4default, ROT0,   "Empire","High Spirits (Empire) (MPU4, set 3)",   GAME_NOT_WORKING|GAME_REQUIRES_ARTWORK|GAME_NO_SOUND )
GAME(199?, m4hisprtc, m4hisprt,	mod4oki, mpu4, m4default, ROT0,   "Empire","High Spirits (Empire) (MPU4, set 4)",   GAME_NOT_WORKING|GAME_REQUIRES_ARTWORK|GAME_NO_SOUND )
GAME(199?, m4hisprtd, m4hisprt,	mod4oki, mpu4, m4default, ROT0,   "Empire","High Spirits (Empire) (MPU4, set 5)",   GAME_NOT_WORKING|GAME_REQUIRES_ARTWORK|GAME_NO_SOUND )
GAME(199?, m4hisprte, m4hisprt,	mod4oki, mpu4, m4default, ROT0,   "Empire","High Spirits (Empire) (MPU4, set 6)",   GAME_NOT_WORKING|GAME_REQUIRES_ARTWORK|GAME_NO_SOUND )
GAME(199?, m4hotcsh,  0,		mod4oki, mpu4, m4default, ROT0,   "Empire","Hot Cash (Empire) (MPU4, set 1)",   GAME_NOT_WORKING|GAME_REQUIRES_ARTWORK|GAME_NO_SOUND )
GAME(199?, m4hotcsha, m4hotcsh, mod4oki, mpu4, m4default, ROT0,   "Empire","Hot Cash (Empire) (MPU4, set 2)",   GAME_NOT_WORKING|GAME_REQUIRES_ARTWORK|GAME_NO_SOUND )
GAME(199?, m4hotcshb, m4hotcsh,	mod4oki, mpu4, m4default, ROT0,   "Empire","Hot Cash (Empire) (MPU4, set 3)",   GAME_NOT_WORKING|GAME_REQUIRES_ARTWORK|GAME_NO_SOUND )
GAME(199?, m4monspn,  0,		mod4oki, mpu4, m4default, ROT0,   "Empire","Money Spinner (Empire) (MPU4, set 1)",   GAME_NOT_WORKING|GAME_REQUIRES_ARTWORK|GAME_NO_SOUND )
GAME(199?, m4monspna, m4monspn,	mod4oki, mpu4, m4default, ROT0,   "Empire","Money Spinner (Empire) (MPU4, set 2)",   GAME_NOT_WORKING|GAME_REQUIRES_ARTWORK|GAME_NO_SOUND )
GAME(199?, m4monspnb, m4monspn,	mod4oki, mpu4, m4default, ROT0,   "Empire","Money Spinner (Empire) (MPU4, set 3)",   GAME_NOT_WORKING|GAME_REQUIRES_ARTWORK|GAME_NO_SOUND )
GAME(199?, m4pbnudg,  0,		mod4oki, mpu4, m4default, ROT0,   "Empire","Pinball Nudger (Empire) (MPU4, set 1)",   GAME_NOT_WORKING|GAME_REQUIRES_ARTWORK|GAME_NO_SOUND )
GAME(199?, m4pbnudga, m4pbnudg,	mod4oki, mpu4, m4default, ROT0,   "Empire","Pinball Nudger (Empire) (MPU4, set 2)",   GAME_NOT_WORKING|GAME_REQUIRES_ARTWORK|GAME_NO_SOUND )
GAME(199?, m4pbnudgb, m4pbnudg,	mod4oki, mpu4, m4default, ROT0,   "Empire","Pinball Nudger (Empire) (MPU4, set 3)",   GAME_NOT_WORKING|GAME_REQUIRES_ARTWORK|GAME_NO_SOUND )
GAME(199?, m4pitfal,  0,		mod4oki, mpu4, m4default, ROT0,   "Empire","Pitfall (Empire) (MPU4, set 1)",   GAME_NOT_WORKING|GAME_REQUIRES_ARTWORK|GAME_NO_SOUND ) // doesn't boot, alarm
GAME(199?, m4pitfala, m4pitfal,	mod4oki, mpu4, m4default, ROT0,   "Empire","Pitfall (Empire) (MPU4, set 2)",   GAME_NOT_WORKING|GAME_REQUIRES_ARTWORK|GAME_NO_SOUND ) // doesn't boot, alarm
GAME(199?, m4pitfalb, m4pitfal,	mod4oki, mpu4, m4default, ROT0,   "Empire","Pitfall (Empire) (MPU4, set 3)",   GAME_NOT_WORKING|GAME_REQUIRES_ARTWORK|GAME_NO_SOUND ) // doesn't boot, alarm
GAME(199?, m4pitfalc, m4pitfal,	mod4oki, mpu4, m4default, ROT0,   "Empire","Pitfall (Empire) (MPU4, set 4)",   GAME_NOT_WORKING|GAME_REQUIRES_ARTWORK|GAME_NO_SOUND ) // doesn't boot, alarm
GAME(199?, m4ttrail, 0,			mod4oki, mpu4, m4default, ROT0,   "Empire","Treasure Trail (Empire) (MPU4, set 1)",   GAME_NOT_WORKING|GAME_REQUIRES_ARTWORK|GAME_NO_SOUND )
GAME(199?, m4ttraila,m4ttrail,	mod4oki, mpu4, m4default, ROT0,   "Empire","Treasure Trail (Empire) (MPU4, set 2)",   GAME_NOT_WORKING|GAME_REQUIRES_ARTWORK|GAME_NO_SOUND )
GAME(199?, m4ttrailb,m4ttrail,	mod4oki, mpu4, m4default, ROT0,   "Empire","Treasure Trail (Empire) (MPU4, set 3)",   GAME_NOT_WORKING|GAME_REQUIRES_ARTWORK|GAME_NO_SOUND )
// doesn't seem like the other Empire games (starts with RESETTING JNE, licensed, mislabeled?)
GAME(199?, m4jne,	  0,		mod4oki, mpu4, m4default, ROT0,   "Empire","The Jackpot's Not Enough (Empire) (MPU4)",   GAME_NOT_WORKING|GAME_REQUIRES_ARTWORK|GAME_NO_SOUND )

/* MDM
   most of these boot and act similar to the Empire games (ie bad text, but run OK) */
GAME(199?, m42punlm,     0,		mod4oki, mpu4, m4default, ROT0,   "Mdm","2p Unlimited (Mdm) (MPU4)",   GAME_NOT_WORKING|GAME_REQUIRES_ARTWORK|GAME_NO_SOUND )
GAME(199?, m4bigapl,  0,		mod4oki, mpu4, m4default, ROT0,   "Mdm","Big Apple (Mdm) (MPU4, set 1)",   GAME_NOT_WORKING|GAME_REQUIRES_ARTWORK|GAME_NO_SOUND )
GAME(199?, m4bigapla, m4bigapl,	mod4oki, mpu4, m4default, ROT0,   "Mdm","Big Apple (Mdm) (MPU4, set 2)",   GAME_NOT_WORKING|GAME_REQUIRES_ARTWORK|GAME_NO_SOUND )
GAME(199?, m4bigaplb, m4bigapl,	mod4oki, mpu4, m4default, ROT0,   "Mdm","Big Apple (Mdm) (MPU4, set 3)",   GAME_NOT_WORKING|GAME_REQUIRES_ARTWORK|GAME_NO_SOUND )
GAME(199?, m4bigaplc, m4bigapl,	mod4oki, mpu4, m4default, ROT0,   "Mdm","Big Apple (Mdm) (MPU4, set 4)",   GAME_NOT_WORKING|GAME_REQUIRES_ARTWORK|GAME_NO_SOUND )
GAME(199?, m4bigapld, m4bigapl,	mod4oki, mpu4, m4default, ROT0,   "Mdm","Big Apple (Mdm) (MPU4, set 5)",   GAME_NOT_WORKING|GAME_REQUIRES_ARTWORK|GAME_NO_SOUND )
GAME(199?, m4bigaple, m4bigapl,	mod4oki, mpu4, m4default, ROT0,   "Mdm","Big Apple (Mdm) (MPU4, set 6)",   GAME_NOT_WORKING|GAME_REQUIRES_ARTWORK|GAME_NO_SOUND )
GAME(199?, m4blztrl,  0,		mod4oki, mpu4, m4default, ROT0,   "Mdm","Blazing Trails (Mdm) (MPU4, set 1)",   GAME_NOT_WORKING|GAME_REQUIRES_ARTWORK|GAME_NO_SOUND )
GAME(199?, m4blztrla, m4blztrl,	mod4oki, mpu4, m4default, ROT0,   "Mdm","Blazing Trails (Mdm) (MPU4, set 2)",   GAME_NOT_WORKING|GAME_REQUIRES_ARTWORK|GAME_NO_SOUND )
GAME(199?, m4bodymt,  0,		mod4oki, mpu4, m4default, ROT0,   "Mdm","Body Match (Mdm) (MPU4)",   GAME_NOT_WORKING|GAME_REQUIRES_ARTWORK|GAME_NO_SOUND ) // doesn't boot, various alarms
GAME(199?, m4coloss,  0,		mod4oki, mpu4, m4default, ROT0,   "Mdm","Colossus (Mdm) (MPU4, set 1)",   GAME_NOT_WORKING|GAME_REQUIRES_ARTWORK|GAME_NO_SOUND )
GAME(199?, m4colossa, m4coloss, mod4oki, mpu4, m4default, ROT0,   "Mdm","Colossus (Mdm) (MPU4, set 2)",   GAME_NOT_WORKING|GAME_REQUIRES_ARTWORK|GAME_NO_SOUND )
GAME(199?, m4colossb, m4coloss, mod4oki, mpu4, m4default, ROT0,   "Mdm","Colossus (Mdm) (MPU4, set 3)",   GAME_NOT_WORKING|GAME_REQUIRES_ARTWORK|GAME_NO_SOUND )
GAME(199?, m4colossc, m4coloss, mod4oki, mpu4, m4default, ROT0,   "Mdm","Colossus (Mdm) (MPU4, set 4)",   GAME_NOT_WORKING|GAME_REQUIRES_ARTWORK|GAME_NO_SOUND )
GAME(199?, m4colossd, m4coloss, mod4oki, mpu4, m4default, ROT0,   "Mdm","Colossus (Mdm) (MPU4, set 5)",   GAME_NOT_WORKING|GAME_REQUIRES_ARTWORK|GAME_NO_SOUND )
GAME(199?, m4colosse, m4coloss, mod4oki, mpu4, m4default, ROT0,   "Mdm","Colossus (Mdm) (MPU4, set 6)",   GAME_NOT_WORKING|GAME_REQUIRES_ARTWORK|GAME_NO_SOUND )
GAME(199?, m4colossf, m4coloss, mod4oki, mpu4, m4default, ROT0,   "Mdm","Colossus (Mdm) (MPU4, set 7)",   GAME_NOT_WORKING|GAME_REQUIRES_ARTWORK|GAME_NO_SOUND )
GAME(199?, m4colossg, m4coloss, mod4oki, mpu4, m4default, ROT0,   "Mdm","Colossus (Mdm) (MPU4, set 8)",   GAME_NOT_WORKING|GAME_REQUIRES_ARTWORK|GAME_NO_SOUND )
GAME(199?, m4colossh, m4coloss, mod4oki, mpu4, m4default, ROT0,   "Mdm","Colossus (Mdm) (MPU4, set 9)",   GAME_NOT_WORKING|GAME_REQUIRES_ARTWORK|GAME_NO_SOUND )
GAME(199?, m4firebl,  0,		mpu4mod2,mpu4, m4default, ROT0,   "Mdm","Fireball (Mdm) (MPU4, set 1)",   GAME_NOT_WORKING|GAME_REQUIRES_ARTWORK|GAME_NO_SOUND ) // hangs after spin (sound status?)
GAME(199?, m4firebla, m4firebl,	mpu4mod2,mpu4, m4default, ROT0,   "Mdm","Fireball (Mdm) (MPU4, set 2)",   GAME_NOT_WORKING|GAME_REQUIRES_ARTWORK|GAME_NO_SOUND ) // hangs after spin (sound status?)
GAME(199?, m4fireblb, m4firebl,	mpu4mod2,mpu4, m4default, ROT0,   "Mdm","Fireball (Mdm) (MPU4, set 3)",   GAME_NOT_WORKING|GAME_REQUIRES_ARTWORK|GAME_NO_SOUND ) // hangs after spin (sound status?)
GAME(199?, m4fireblc, m4firebl,	mpu4mod2,mpu4, m4default, ROT0,   "Mdm","Fireball (Mdm) (MPU4, set 4)",   GAME_NOT_WORKING|GAME_REQUIRES_ARTWORK|GAME_NO_SOUND ) // hangs after spin (sound status?)
GAME(199?, m4firebld, m4firebl,	mpu4mod2,mpu4, m4default, ROT0,   "Mdm","Fireball (Mdm) (MPU4, set 5)",   GAME_NOT_WORKING|GAME_REQUIRES_ARTWORK|GAME_NO_SOUND ) // hangs after spin (sound status?)
GAME(199?, m4mayhem,  0,		mod4oki, mpu4, m4default, ROT0,   "Mdm","Mayhem (Mdm) (MPU4, set 1)",   GAME_NOT_WORKING|GAME_REQUIRES_ARTWORK|GAME_NO_SOUND )
GAME(199?, m4mayhema, m4mayhem,	mod4oki, mpu4, m4default, ROT0,   "Mdm","Mayhem (Mdm) (MPU4, set 2)",   GAME_NOT_WORKING|GAME_REQUIRES_ARTWORK|GAME_NO_SOUND )
GAME(199?, m4themob,  0,		mod4oki, mpu4, m4default, ROT0,   "Mdm","The Mob (Mdm) (MPU4, set 1)",   GAME_NOT_WORKING|GAME_REQUIRES_ARTWORK|GAME_NO_SOUND )
GAME(199?, m4themoba, m4themob, mod4oki, mpu4, m4default, ROT0,   "Mdm","The Mob (Mdm) (MPU4, set 2)",   GAME_NOT_WORKING|GAME_REQUIRES_ARTWORK|GAME_NO_SOUND )
GAME(199?, m4themobb, m4themob, mod4oki, mpu4, m4default, ROT0,   "Mdm","The Mob (Mdm) (MPU4, set 3)",   GAME_NOT_WORKING|GAME_REQUIRES_ARTWORK|GAME_NO_SOUND )
GAME(199?, m4nudbon,  0,		mpu4mod2,mpu4, m4default, ROT0,   "Mdm","Nudge Bonanza (Mdm) (MPU4, set 1)",   GAME_NOT_WORKING|GAME_REQUIRES_ARTWORK|GAME_NO_SOUND )
GAME(199?, m4nudbona, m4nudbon,	mpu4mod2,mpu4, m4default, ROT0,   "Mdm","Nudge Bonanza (Mdm) (MPU4, set 2)",   GAME_NOT_WORKING|GAME_REQUIRES_ARTWORK|GAME_NO_SOUND )
GAME(199?, m4nudgem,  0,		mod4oki, mpu4, m4default, ROT0,   "Mdm","Nudge Gems (Mdm) (MPU4, set 1)",   GAME_NOT_WORKING|GAME_REQUIRES_ARTWORK|GAME_NO_SOUND )
GAME(199?, m4nudgema, m4nudgem,	mod4oki, mpu4, m4default, ROT0,   "Mdm","Nudge Gems (Mdm) (MPU4, set 2)",   GAME_NOT_WORKING|GAME_REQUIRES_ARTWORK|GAME_NO_SOUND )
GAME(199?, m4smshgb, 0,			mod4oki, mpu4, m4default, ROT0,   "Mdm","Smash 'n' Grab (Mdm) (MPU4, set 1)",   GAME_NOT_WORKING|GAME_REQUIRES_ARTWORK|GAME_NO_SOUND )
GAME(199?, m4smshgba,m4smshgb,	mod4oki, mpu4, m4default, ROT0,   "Mdm","Smash 'n' Grab (Mdm) (MPU4, set 2)",   GAME_NOT_WORKING|GAME_REQUIRES_ARTWORK|GAME_NO_SOUND )
GAME(199?, m4smshgbb,m4smshgb,	mod4oki, mpu4, m4default, ROT0,   "Mdm","Smash 'n' Grab (Mdm) (MPU4, set 3)",   GAME_NOT_WORKING|GAME_REQUIRES_ARTWORK|GAME_NO_SOUND )
GAME(199?, m4smshgbc,m4smshgb,	mod4oki, mpu4, m4default, ROT0,   "Mdm","Smash 'n' Grab (Mdm) (MPU4, set 4)",   GAME_NOT_WORKING|GAME_REQUIRES_ARTWORK|GAME_NO_SOUND )
GAME(199?, m4snklad, 0,			mpu4mod2,mpu4, m4default, ROT0,   "Mdm","Snakes & Ladders (Mdm) (MPU4)",   GAME_NOT_WORKING|GAME_REQUIRES_ARTWORK|GAME_NO_SOUND )

/* Union
  these don't boot, at best you get a 'CLEAR' message */
GAME(199?, m4cwalk,   0,		mod4oki, mpu4, m4default, ROT0,   "Union","Cake Walk (Union) (MPU4)",   GAME_NOT_WORKING|GAME_REQUIRES_ARTWORK|GAME_NO_SOUND )
GAME(199?, m4eezee,   0,		mod4oki, mpu4, m4default, ROT0,   "Union","Eezee Fruits (Union) (MPU4)",   GAME_NOT_WORKING|GAME_REQUIRES_ARTWORK|GAME_NO_SOUND )
GAME(199?, m4frdrop,  0,		mod4oki, mpu4, m4default, ROT0,   "Union","Fruit Drop (Union) (MPU4)",   GAME_NOT_WORKING|GAME_REQUIRES_ARTWORK|GAME_NO_SOUND )
GAME(199?, m4gobana,  0,		mpu4mod2,mpu4, m4default, ROT0,   "Union","Go Bananas (Union) (MPU4, set 1)",   GAME_NOT_WORKING|GAME_REQUIRES_ARTWORK|GAME_NO_SOUND )
GAME(199?, m4gobanaa, m4gobana,	mpu4mod2,mpu4, m4default, ROT0,   "Union","Go Bananas (Union) (MPU4, set 2)",   GAME_NOT_WORKING|GAME_REQUIRES_ARTWORK|GAME_NO_SOUND )
GAME(199?, m4gobanab, m4gobana,	mpu4mod2,mpu4, m4default, ROT0,   "Union","Go Bananas (Union) (MPU4, set 3)",   GAME_NOT_WORKING|GAME_REQUIRES_ARTWORK|GAME_NO_SOUND )
GAME(199?, m4gobanac, m4gobana,	mpu4mod2,mpu4, m4default, ROT0,   "Union","Go Bananas (Union) (MPU4, set 4)",   GAME_NOT_WORKING|GAME_REQUIRES_ARTWORK|GAME_NO_SOUND )
GAME(199?, m4gobanad, m4gobana,	mpu4mod2,mpu4, m4default, ROT0,   "Union","Go Bananas (Union) (MPU4, set 5)",   GAME_NOT_WORKING|GAME_REQUIRES_ARTWORK|GAME_NO_SOUND )
GAME(199?, m4lotty,	  0,		mpu4mod2,mpu4, m4default, ROT0,   "Union","Lotty Time (Union) (MPU4)",   GAME_NOT_WORKING|GAME_REQUIRES_ARTWORK|GAME_NO_SOUND )
GAME(199?, m4maxmze,  0,		mpu4mod2,mpu4, m4default, ROT0,   "Union","Maximize (Union) (MPU4, set 1)",   GAME_NOT_WORKING|GAME_REQUIRES_ARTWORK|GAME_NO_SOUND )
GAME(199?, m4maxmzea, m4maxmze,	mpu4mod2,mpu4, m4default, ROT0,   "Union","Maximize (Union) (MPU4, set 2)",   GAME_NOT_WORKING|GAME_REQUIRES_ARTWORK|GAME_NO_SOUND )
GAME(199?, m4maxmzeb, m4maxmze,	mpu4mod2,mpu4, m4default, ROT0,   "Union","Maximize (Union) (MPU4, set 3)",   GAME_NOT_WORKING|GAME_REQUIRES_ARTWORK|GAME_NO_SOUND )
GAME(199?, m4maxmzec, m4maxmze,	mpu4mod2,mpu4, m4default, ROT0,   "Union","Maximize (Union) (MPU4, set 4)",   GAME_NOT_WORKING|GAME_REQUIRES_ARTWORK|GAME_NO_SOUND )
GAME(199?, m4maxmzed, m4maxmze,	mpu4mod2,mpu4, m4default, ROT0,   "Union","Maximize (Union) (MPU4, set 5)",   GAME_NOT_WORKING|GAME_REQUIRES_ARTWORK|GAME_NO_SOUND )
GAME(199?, m4mecca,   0,		mpu4mod2,mpu4, m4default, ROT0,   "Union","Mecca Money (Union) (MPU4)",   GAME_NOT_WORKING|GAME_REQUIRES_ARTWORK|GAME_NO_SOUND )
GAME(199?, m4purmad,  0,		mod4oki, mpu4, m4default, ROT0,   "Union","Pure Madness (Union)",   GAME_NOT_WORKING|GAME_REQUIRES_ARTWORK|GAME_NO_SOUND )
GAME(199?, m4revolv,  0,		mod4oki, mpu4, m4default, ROT0,   "Union","Revolva (Union) (MPU4)",   GAME_NOT_WORKING|GAME_REQUIRES_ARTWORK|GAME_NO_SOUND )
GAME(199?, m4rotex,  0,			mod4oki, mpu4, m4default, ROT0,   "Union","Rotex (Union) (MPU4)",   GAME_NOT_WORKING|GAME_REQUIRES_ARTWORK|GAME_NO_SOUND )
GAME(199?, m4select, 0,			mod4oki, mpu4, m4default, ROT0,   "Union","Select (Union) (MPU4)",   GAME_NOT_WORKING|GAME_REQUIRES_ARTWORK|GAME_NO_SOUND )
GAME(199?, m4supfru, 0,			mod4oki, mpu4, m4default, ROT0,   "Union","Supafruits (Union) (MPU4, set 1)",   GAME_NOT_WORKING|GAME_REQUIRES_ARTWORK|GAME_NO_SOUND )
GAME(199?, m4supfrua,m4supfru,	mod4oki, mpu4, m4default, ROT0,   "Union","Supafruits (Union) (MPU4, set 2)",   GAME_NOT_WORKING|GAME_REQUIRES_ARTWORK|GAME_NO_SOUND )
GAME(199?, m4trimad, 0,			mod4oki, mpu4, m4default, ROT0,   "Union","Triple Madness (Union) (MPU4)",   GAME_NOT_WORKING|GAME_REQUIRES_ARTWORK|GAME_NO_SOUND )
GAME(199?, m4unibox, 0,			mod4oki, mpu4, m4default, ROT0,   "Union","Unibox (Union) (MPU4, set 1)",   GAME_NOT_WORKING|GAME_REQUIRES_ARTWORK|GAME_NO_SOUND )
GAME(199?, m4uniboxa,m4unibox,	mod4oki, mpu4, m4default, ROT0,   "Union","Unibox (Union) (MPU4, set 2)",   GAME_NOT_WORKING|GAME_REQUIRES_ARTWORK|GAME_NO_SOUND )
GAME(199?, m4unique, 0,			mod4oki, mpu4, m4default, ROT0,   "Union","Unique (Union) (MPU4, set 1)",   GAME_NOT_WORKING|GAME_REQUIRES_ARTWORK|GAME_NO_SOUND )
GAME(199?, m4uniquep,m4unique,	mod4oki, mpu4, m4default, ROT0,   "Union","Unique (Union) (MPU4, set 2)",   GAME_NOT_WORKING|GAME_REQUIRES_ARTWORK|GAME_NO_SOUND )

/* Union + Empire
   same as Union above */
GAME(199?, m4gvibes,  0,		mod4oki, mpu4, m4default, ROT0,   "Union / Empire","Good Vibrations (Union - Empire) (MPU4, set 1)",   GAME_NOT_WORKING|GAME_REQUIRES_ARTWORK|GAME_NO_SOUND )
GAME(199?, m4gvibesa, m4gvibes,	mod4oki, mpu4, m4default, ROT0,   "Union / Empire","Good Vibrations (Union - Empire) (MPU4, set 2)",   GAME_NOT_WORKING|GAME_REQUIRES_ARTWORK|GAME_NO_SOUND )
GAME(199?, m4rckrol,  0,		mod4oki, mpu4, m4default, ROT0,   "Union / Empire","Rock 'n' Roll (Union - Empire) (MPU4, set 1)",   GAME_NOT_WORKING|GAME_REQUIRES_ARTWORK|GAME_NO_SOUND )
GAME(199?, m4rckrola, m4rckrol,	mod4oki, mpu4, m4default, ROT0,   "Union / Empire","Rock 'n' Roll (Union - Empire) (MPU4, set 2)",   GAME_NOT_WORKING|GAME_REQUIRES_ARTWORK|GAME_NO_SOUND )
GAME(199?, m4rckrolb, m4rckrol,	mod4oki, mpu4, m4default, ROT0,   "Union / Empire","Rock 'n' Roll (Union - Empire) (MPU4, set 3)",   GAME_NOT_WORKING|GAME_REQUIRES_ARTWORK|GAME_NO_SOUND )

/* Others */

GAME(199?, m4aao,     0,		mod4oki, mpu4, m4default, ROT0,   "Eurotek","Against All Odds (Eurotek) (MPU4)",   GAME_NOT_WORKING|GAME_REQUIRES_ARTWORK|GAME_NO_SOUND )
GAME(199?, m4bandgd,  0,		mod4oki, mpu4, m4default, ROT0,   "Eurogames","Bands Of Gold (Eurogames) (MPU4)",   GAME_NOT_WORKING|GAME_REQUIRES_ARTWORK|GAME_NO_SOUND )
GAME(199?, m4bangin,  0,		mod4oki, mpu4, m4default, ROT0,   "Global","Bangin' Away (Global) (MPU4, set 1)",   GAME_NOT_WORKING|GAME_REQUIRES_ARTWORK|GAME_NO_SOUND )
GAME(199?, m4bangina, m4bangin,	mod4oki, mpu4, m4default, ROT0,   "Global","Bangin' Away (Global) (MPU4, set 2)",   GAME_NOT_WORKING|GAME_REQUIRES_ARTWORK|GAME_NO_SOUND )
GAME(199?, m4banginb, m4bangin,	mod4oki, mpu4, m4default, ROT0,   "Global","Bangin' Away (Global) (MPU4, set 3)",   GAME_NOT_WORKING|GAME_REQUIRES_ARTWORK|GAME_NO_SOUND )
GAME(199?, m4bigben,  0,		mod4oki, mpu4, m4default, ROT0,   "Coinworld","Big Ben (Coinworld) (MPU4, set 1)",   GAME_NOT_WORKING|GAME_REQUIRES_ARTWORK|GAME_NO_SOUND )
GAME(199?, m4bigbena, m4bigben,	mod4oki, mpu4, m4default, ROT0,   "Coinworld","Big Ben (Coinworld) (MPU4, set 2)",   GAME_NOT_WORKING|GAME_REQUIRES_ARTWORK|GAME_NO_SOUND )
GAME(199?, m4bigbenb, m4bigben,	mod4oki, mpu4, m4default, ROT0,   "Coinworld","Big Ben (Coinworld) (MPU4, set 3)",   GAME_NOT_WORKING|GAME_REQUIRES_ARTWORK|GAME_NO_SOUND )
GAME(199?, m4bigbenc, m4bigben,	mod4oki, mpu4, m4default, ROT0,   "Coinworld","Big Ben (Coinworld) (MPU4, set 4)",   GAME_NOT_WORKING|GAME_REQUIRES_ARTWORK|GAME_NO_SOUND )
GAME(199?, m4bigbend, m4bigben,	mod4oki, mpu4, m4default, ROT0,   "Coinworld","Big Ben (Coinworld) (MPU4, set 5)",   GAME_NOT_WORKING|GAME_REQUIRES_ARTWORK|GAME_NO_SOUND )
GAME(199?, m4bigbene, m4bigben,	mod4oki, mpu4, m4default, ROT0,   "Coinworld","Big Ben (Coinworld) (MPU4, set 6)",   GAME_NOT_WORKING|GAME_REQUIRES_ARTWORK|GAME_NO_SOUND )
GAME(199?, m4kqclub,  0,		mpu4mod2,mpu4, m4default, ROT0,   "Newby","Kings & Queens Club (Newby) (MPU4)",   GAME_NOT_WORKING|GAME_REQUIRES_ARTWORK|GAME_NO_SOUND )
GAME(199?, m4snookr,  0,		mpu4mod2,mpu4, m4default, ROT0,   "Eurocoin","Snooker (Eurocoin) (MPU4)",   GAME_NOT_WORKING|GAME_REQUIRES_ARTWORK|GAME_NO_SOUND ) // works?
GAME(199?, m4spnwin,  0,		mpu4mod2,mpu4, m4default, ROT0,   "Cotswold Microsystems","Spin A Win (Cotswold Microsystems) (MPU4)",   GAME_NOT_WORKING|GAME_REQUIRES_ARTWORK|GAME_NO_SOUND ) // works?
GAME(199?, m4stakex,  0,		mod4oki, mpu4, m4default, ROT0,   "Leisurama","Stake X (Leisurama) (MPU4, set 1)",   GAME_NOT_WORKING|GAME_REQUIRES_ARTWORK|GAME_NO_SOUND ) // can't coin, no sound
GAME(199?, m4stakexa, m4stakex,	mod4oki, mpu4, m4default, ROT0,   "Leisurama","Stake X (Leisurama) (MPU4, set 2)",   GAME_NOT_WORKING|GAME_REQUIRES_ARTWORK|GAME_NO_SOUND ) // works?
GAME(199?, m4sstrek,  0,		mpu4mod2,mpu4, m4default, ROT0,   "bootleg","Super Streak (bootleg) (MPU4)",   GAME_NOT_WORKING|GAME_REQUIRES_ARTWORK|GAME_NO_SOUND ) // works?, no sound
GAME(199?, m4boltbl,  0,		mpu4mod2,mpu4, m4default, ROT0,   "DJE","Bolt From The Blue (DJE) (MPU4, set 1)",   GAME_NOT_WORKING|GAME_REQUIRES_ARTWORK|GAME_NO_SOUND )
GAME(199?, m4boltbla, m4boltbl,	mpu4mod2,mpu4, m4default, ROT0,   "DJE","Bolt From The Blue (DJE) (MPU4, set 2)",   GAME_NOT_WORKING|GAME_REQUIRES_ARTWORK|GAME_NO_SOUND )
GAME(199?, m4boltblb, m4boltbl,	mpu4mod2,mpu4, m4default, ROT0,   "DJE","Bolt From The Blue (DJE) (MPU4, set 3)",   GAME_NOT_WORKING|GAME_REQUIRES_ARTWORK|GAME_NO_SOUND )
GAME(199?, m4boltblc, m4boltbl,	mpu4mod2,mpu4, m4default, ROT0,   "DJE","Bolt From The Blue (DJE) (MPU4, set 4)",   GAME_NOT_WORKING|GAME_REQUIRES_ARTWORK|GAME_NO_SOUND )
GAME(199?, m4dblchn,  0,		mod4oki, mpu4, m4default, ROT0,   "DJE","Double Chance (DJE) (MPU4)",   GAME_NOT_WORKING|GAME_REQUIRES_ARTWORK|GAME_NO_SOUND )
GAME(199?, m4stand2,  0,		mpu4mod2,mpu4, m4default, ROT0,   "DJE","Stand To Deliver (DJE) (MPU4)",   GAME_NOT_WORKING|GAME_REQUIRES_ARTWORK|GAME_NO_SOUND )


/* stuff that was already in the driver */

GAME( 199?,  m4oldtmr,0,      mod4oki,mpu4,	  m_oldtmr, ROT0,   "Barcrest",		 "Old Timer",						GAME_NOT_WORKING|GAME_NO_SOUND|GAME_REQUIRES_ARTWORK )
GAME( 198?,  m4ccelbr,0,      mpu4mod2, mpu4,	  m_ccelbr, ROT0,   "Barcrest",		 "Club Celebration",				GAME_NOT_WORKING|GAME_REQUIRES_ARTWORK )
GAMEL(198?,  m4gmball,0,      mod4yam,  gamball,  m_gmball, ROT0,	"Barcrest",      "Gamball",							GAME_REQUIRES_ARTWORK|GAME_MECHANICAL,layout_gamball )//Mechanical ball launcher
GAMEL(198?,  m4grtecp,0,      mod4oki,  grtecp,	  m_grtecp, ROT0,   "Barcrest",		 "Andy's Great Escape",				GAME_NOT_WORKING|GAME_REQUIRES_ARTWORK,layout_mpu4ext )//5 reel meter mux
GAME(199?,   m4blsbys,0,	  bwboki,   mpu4,	  m_blsbys, ROT0,   "Barcrest",		 "Blues Boys (Version 6)",			GAME_NOT_WORKING|GAME_REQUIRES_ARTWORK )
GAME(199?,   m4frkstn,0,	  mpu4crys, mpu4,     m_frkstn, ROT0,   "Crystal Gaming","Frank 'n' Stein (unencrypted)",   GAME_NOT_WORKING|GAME_REQUIRES_ARTWORK|GAME_NO_SOUND )

//SWP
GAMEL(1989?,  m4conn4,        0, mpu4mod2,   connect4,   connect4,   ROT0, "Dolbeck Systems","Connect 4",GAME_IMPERFECT_GRAPHICS|GAME_REQUIRES_ARTWORK,layout_connect4 )
