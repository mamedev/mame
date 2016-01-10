// license:BSD-3-Clause
// copyright-holders:James Wallace
/* MPU4 hardware emulation
  for sets see mpu4.c
*/

/* Note 19/07/11 DH
 - added lots of sets

   these are mostly unsorted and need to be split into clones
   the original source of these was a mess, assume things to be mislabled, bad, duplicated, or otherwise
   badly organized.  a lot of work is needed to sort them out, especially the Barcrest sets!  Some of this
   stuff MIGHT be in the wrong driver, or missing roms (sound roms especially)
*/

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

Additional Notes:

Games from around the era of Road Hog and Chase Invaders had sufficient additional space to store three sets of reel
start/stop sounds.

To change between them, follow these instructions:

1) Load the game.
2) Open the cashbox door and insert the refill key.
3) Use Hi/Lo to adjust volume
4) Use Hold 1/2/3 to choose between "Default", "Standard" and "Alternative" sound sets
5) Use Cancel/collect to test the sounds.
6) To return to the game, remove the refill key and close the door

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

#include "includes/mpu4.h"


#include "video/awpvid.h"       //Fruit Machines Only

#include "mpu4.lh"
#include "mpu4ext.lh"


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

void mpu4_state::lamp_extend_small(int data)
{
	int lamp_ext_data,column,i;
	column = data & 0x07;

	lamp_ext_data = 0x1f - ((data & 0xf8) >> 3);//remove the mux lines from the data

	if (m_lamp_strobe_ext_persistence == 0)
	//One write to reset the drive lines, one with the data, one to clear the lines, so only the 2nd write does anything
	//Once again, lamp persistences would take care of this, but we can't do that
	{
		for (i = 0; i < 5; i++)
		{
			output().set_lamp_value((8*column)+i+128,((lamp_ext_data  & (1 << i)) != 0));
		}
	}
	m_lamp_strobe_ext_persistence ++;
	if ((m_lamp_strobe_ext_persistence == 3)||(m_lamp_strobe_ext!=column))
	{
		m_lamp_strobe_ext_persistence = 0;
		m_lamp_strobe_ext=column;
	}
}

void mpu4_state::lamp_extend_large(int data,int column,int active)
{
	int lampbase,i,bit7;

	m_lamp_sense = 0;
	bit7 = data & 0x80;
	if ( bit7 != m_last_b7 )
	{
		m_card_live = 1;
		//depending on bit 7, we can access one of two 'blocks' of 64 lamps
		lampbase = bit7 ? 0 : 64;
		if ( data & 0x3f )
		{
			m_lamp_sense = 1;
		}
		if ( active )
		{
			if (m_lamp_strobe_ext != column)
			{
				for (i = 0; i < 8; i++)
				{//CHECK, this includes bit 7
					output().set_lamp_value((8*column)+i+128+lampbase ,(data  & (1 << i)) != 0);
				}
				m_lamp_strobe_ext = column;
			}
		}
		m_last_b7 = bit7;
	}
	else
	{
		m_card_live = 0;
	}
}

void mpu4_state::led_write_latch(int latch, int data, int column)
{
	int diff,i,j;

	diff = (latch ^ m_last_latch) & latch;
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
		output().set_indexed_value("mpu4led",(8*column)+j,(data & (1 << j)) !=0);
	}
	output().set_digit_value(column * 8, data);

	m_last_latch = diff;
}


void mpu4_state::update_meters()
{
	int meter;
	int data = ((m_mmtr_data & 0x7f) | m_remote_meter);
	switch (m_reel_mux)
	{
	case STANDARD_REEL:
		// Change nothing
		break;

	case FIVE_REEL_5TO8:
		m_reel4->update(((data >> 4) & 0x0f));
		data = (data & 0x0F); //Strip reel data from meter drives, leaving active elements
		awp_draw_reel(machine(),"reel5", m_reel4);
		break;

	case FIVE_REEL_8TO5:
		m_reel4->update((((data & 0x01) + ((data & 0x08) >> 2) + ((data & 0x20) >> 3) + ((data & 0x80) >> 4)) & 0x0f)) ;
		data = 0x00; //Strip all reel data from meter drives, nothing is connected
		awp_draw_reel(machine(),"reel5", m_reel4);
		break;

	case FIVE_REEL_3TO6:
		m_reel4->update(((data >> 2) & 0x0f));
		data = 0x00; //Strip all reel data from meter drives
		awp_draw_reel(machine(),"reel5", m_reel4);
		break;

	case SIX_REEL_1TO8:
		m_reel4->update( data       & 0x0f);
		m_reel5->update((data >> 4) & 0x0f);
		data = 0x00; //Strip all reel data from meter drives
		awp_draw_reel(machine(),"reel5", m_reel4);
		awp_draw_reel(machine(),"reel6", m_reel5);
		break;

	case SIX_REEL_5TO8:
		m_reel4->update(((data >> 4) & 0x0f));
		data = 0x00; //Strip all reel data from meter drives
		awp_draw_reel(machine(),"reel5", m_reel4);
		break;

	case SEVEN_REEL:
		m_reel0->update((((data & 0x01) + ((data & 0x08) >> 2) + ((data & 0x20) >> 3) + ((data & 0x80) >> 4)) & 0x0f)) ;
		data = 0x00; //Strip all reel data from meter drives
		awp_draw_reel(machine(),"reel1", m_reel0);
		break;

	case FLUTTERBOX: //The backbox fan assembly fits in a reel unit sized box, wired to the remote meter pin, so we can handle it here
		output().set_value("flutterbox", data & 0x80);
		data &= ~0x80; //Strip flutterbox data from meter drives
		break;
	}

	m_meters->update(7, (data & 0x80));
	for (meter = 0; meter < 4; meter ++)
	{
		m_meters->update(meter, (data & (1 << meter)));
	}
	if (m_reel_mux == STANDARD_REEL)
	{
		for (meter = 4; meter < 7; meter ++)
		{
			m_meters->update(meter, (data & (1 << meter)));
		}
	}
}

/* called if board is reset */
MACHINE_RESET_MEMBER(mpu4_state,mpu4)
{
	m_vfd->reset();

	m_lamp_strobe    = 0;
	m_lamp_strobe2   = 0;
	m_led_strobe     = 0;
	m_mmtr_data      = 0;
	m_remote_meter   = 0;

	m_IC23GC    = 0;
	m_IC23GB    = 0;
	m_IC23GA    = 0;
	m_IC23G1    = 1;
	m_IC23G2A   = 0;
	m_IC23G2B   = 0;

	m_prot_col  = 0;
	m_chr_counter    = 0;
	m_chr_value     = 0;


	{
		if (m_numbanks)
			m_bank1->set_entry(m_numbanks);

		m_maincpu->reset();
	}
}


/* 6809 IRQ handler */
WRITE_LINE_MEMBER(mpu4_state::cpu0_irq)
{
	/* The PIA and PTM IRQ lines are all connected to a common PCB track, leading directly to the 6809 IRQ line. */
	int combined_state = m_pia3->irq_a_state() | m_pia3->irq_b_state() |
							m_pia4->irq_a_state() | m_pia4->irq_b_state() |
							m_pia5->irq_a_state() | m_pia5->irq_b_state() |
							m_pia6->irq_a_state() | m_pia6->irq_b_state() |
							m_pia7->irq_a_state() | m_pia7->irq_b_state() |
							m_pia8->irq_a_state() | m_pia8->irq_b_state() |
							m_6840ptm->irq_state();

	if (!m_link7a_connected) //7B = IRQ, 7A = FIRQ, both = NMI
	{
		m_maincpu->set_input_line(M6809_IRQ_LINE, combined_state ? ASSERT_LINE : CLEAR_LINE);
		LOG(("6809 int%d \n", combined_state));
	}
	else
	{
		m_maincpu->set_input_line(M6809_FIRQ_LINE, combined_state ? ASSERT_LINE : CLEAR_LINE);
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
WRITE8_MEMBER(mpu4_state::bankswitch_w)
{
//  printf("bankswitch_w %02x\n", data);

	// m_pageset is never even set??
	m_pageval = (data & 0x03);
	m_bank1->set_entry((m_pageval + (m_pageset ? 4 : 0)) & m_numbanks);
}


READ8_MEMBER(mpu4_state::bankswitch_r)
{
	return m_bank1->entry();
}


WRITE8_MEMBER(mpu4_state::bankset_w)
{
//  printf("bankset_w %02x\n", data);

	// m_pageset is never even set??

	m_pageval = (data - 2);//writes 2 and 3, to represent 0 and 1 - a hangover from the half page design?
	m_bank1->set_entry((m_pageval + (m_pageset ? 4 : 0)) & m_numbanks);
}


/* IC2 6840 PTM handler */
WRITE8_MEMBER(mpu4_state::ic2_o1_callback)
{
	m_6840ptm->set_c2(data);    /* copy output value to IC2 c2
    this output is the clock for timer2 */
	/* 1200Hz System interrupt timer */
}


WRITE8_MEMBER(mpu4_state::ic2_o2_callback)
{
	m_pia3->ca1_w(data);    /* copy output value to IC3 ca1 */
	/* the output from timer2 is the input clock for timer3 */
	/* miscellaneous interrupts generated here */
	m_6840ptm->set_c3(data);
}


WRITE8_MEMBER(mpu4_state::ic2_o3_callback)
{
	/* the output from timer3 is used as a square wave for the alarm output
	and as an external clock source for timer 1! */
	/* also runs lamp fade */
	m_6840ptm->set_c1(data);
}

/* 6821 PIA handlers */
/* IC3, lamp data lines + alpha numeric display */
WRITE8_MEMBER(mpu4_state::pia_ic3_porta_w)
{
	int i;
	LOG_IC3(("%s: IC3 PIA Port A Set to %2x (lamp strobes 1 - 9)\n", machine().describe_context(),data));

	if(m_ic23_active)
	{
		if (m_lamp_strobe != m_input_strobe)
		{
			// Because of the nature of the lamping circuit, there is an element of persistance
			// As a consequence, the lamp column data can change before the input strobe without
			// causing the relevant lamps to black out.

			for (i = 0; i < 8; i++)
			{
				output().set_lamp_value((8*m_input_strobe)+i, ((data  & (1 << i)) !=0));
			}
			m_lamp_strobe = m_input_strobe;
		}
	}
}

WRITE8_MEMBER(mpu4_state::pia_ic3_portb_w)
{
	int i;
	LOG_IC3(("%s: IC3 PIA Port B Set to %2x  (lamp strobes 10 - 17)\n", machine().describe_context(),data));

	if(m_ic23_active)
	{
		if (m_lamp_strobe2 != m_input_strobe)
		{
			for (i = 0; i < 8; i++)
			{
				output().set_lamp_value((8*m_input_strobe)+i+64, ((data  & (1 << i)) !=0));
			}
			m_lamp_strobe2 = m_input_strobe;
		}

		if (m_led_lamp)
		{
			/* Some games (like Connect 4) use 'programmable' LED displays, built from light display lines in section 2. */
			/* These are mostly low-tech machines, where such wiring proved cheaper than an extender card */
			/* TODO: replace this with 'segment' lamp masks, to make it more generic */
			UINT8 pled_segs[2] = {0,0};

			static const int lamps1[8] = { 106, 107, 108, 109, 104, 105, 110, 133 };
			static const int lamps2[8] = { 114, 115, 116, 117, 112, 113, 118, 119 };

			for (i = 0; i < 8; i++)
			{
				if (output().get_lamp_value(lamps1[i])) pled_segs[0] |= (1 << i);
				if (output().get_lamp_value(lamps2[i])) pled_segs[1] |= (1 << i);
			}

			output().set_digit_value(8,pled_segs[0]);
			output().set_digit_value(9,pled_segs[1]);
		}
	}
}

WRITE_LINE_MEMBER(mpu4_state::pia_ic3_ca2_w)
{
	LOG_IC3(("%s: IC3 PIA Write CA2 (alpha data), %02X\n", machine().describe_context(),state));
	m_vfd->data(state);
}


WRITE_LINE_MEMBER(mpu4_state::pia_ic3_cb2_w)
{
	LOG_IC3(("%s: IC3 PIA Write CB (alpha reset), %02X\n",machine().describe_context(),state));
// DM Data pin A
	m_vfd->por(state);
}


/*
IC23 emulation

IC23 is a 74LS138 1-of-8 Decoder

It is used as a multiplexer for the LEDs, lamp selects and inputs.*/

void mpu4_state::ic23_update()
{
	if (!m_IC23G2A)
	{
		if (!m_IC23G2B)
		{
			if (m_IC23G1)
			{
				if ( m_IC23GA ) m_input_strobe |= 0x01;
				else            m_input_strobe &= ~0x01;

				if ( m_IC23GB ) m_input_strobe |= 0x02;
				else            m_input_strobe &= ~0x02;

				if ( m_IC23GC ) m_input_strobe |= 0x04;
				else            m_input_strobe &= ~0x04;
			}
		}
	}
	else
	if ((m_IC23G2A)||(m_IC23G2B))
	{
		m_input_strobe = 0x00;
	}
}


/*
IC24 emulation

IC24 is a 74LS122 pulse generator

CLEAR and B2 are tied high and A1 and A2 tied low, meaning any pulse
on B1 will give a low pulse on the output pin.
*/
void mpu4_state::ic24_output(int data)
{
	m_IC23G2A = data;
	ic23_update();
}


void mpu4_state::ic24_setup()
{
	if (m_IC23GA)
	{
		double duration = TIME_OF_74LS123((220*1000),(0.1*0.000001));
		{
			m_ic23_active=1;
			ic24_output(0);
			m_ic24_timer->adjust(attotime::from_double(duration));
		}
	}
}


void mpu4_state::device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr)
{
	switch(id)
	{
	case TIMER_IC24:
		m_ic23_active=0;
		ic24_output(1);
		break;
	}
}


/* IC4, 7 seg leds, 50Hz timer reel sensors, current sensors */
WRITE8_MEMBER(mpu4_state::pia_ic4_porta_w)
{
	int i;
	if(m_ic23_active)
	{
		if (((m_lamp_extender == NO_EXTENDER)||(m_lamp_extender == SMALL_CARD)||(m_lamp_extender == LARGE_CARD_C))&& (m_led_extender == NO_EXTENDER))
		{
			if(m_led_strobe != m_input_strobe)
			{
				for(i=0; i<8; i++)
				{
					output().set_indexed_value("mpu4led",((7 - m_input_strobe) * 8) +i,(data & (1 << i)) !=0);
				}
				output().set_digit_value(7 - m_input_strobe,data);
			}
			m_led_strobe = m_input_strobe;
		}
	}
}

WRITE8_MEMBER(mpu4_state::pia_ic4_portb_w)
{
	if (m_reel_mux)
	{
		/* A write here connects one reel (and only one)
		to the optic test circuit. This allows 8 reels
		to be supported instead of 4. */
		if (m_reel_mux == SEVEN_REEL)
		{
			m_active_reel= reel_mux_table7[(data >> 4) & 0x07];
		}
		else
		m_active_reel= reel_mux_table[(data >> 4) & 0x07];
	}
}

READ8_MEMBER(mpu4_state::pia_ic4_portb_r)
{
	/// TODO: this shouldn't be clocked from a read callback
	if ( m_serial_data )
	{
		m_ic4_input_b |=  0x80;
		m_pia4->cb1_w(1);
	}
	else
	{
		m_ic4_input_b &= ~0x80;
		m_pia4->cb1_w(0);
	}

	if (!m_reel_mux)
	{
		if ( m_optic_pattern & 0x01 ) m_ic4_input_b |=  0x40; /* reel A tab */
		else                          m_ic4_input_b &= ~0x40;

		if ( m_optic_pattern & 0x02 ) m_ic4_input_b |=  0x20; /* reel B tab */
		else                          m_ic4_input_b &= ~0x20;

		if ( m_optic_pattern & 0x04 ) m_ic4_input_b |=  0x10; /* reel C tab */
		else                          m_ic4_input_b &= ~0x10;

		if ( m_optic_pattern & 0x08 ) m_ic4_input_b |=  0x08; /* reel D tab */
		else                          m_ic4_input_b &= ~0x08;

	}
	else
	{
		if (m_optic_pattern & (1<<m_active_reel))
		{
			m_ic4_input_b |=  0x08;
		}
		else
		{
			m_ic4_input_b &= ~0x08;
		}
	}
	if ( m_signal_50hz )            m_ic4_input_b |=  0x04; /* 50 Hz */
	else                            m_ic4_input_b &= ~0x04;

	if (m_ic4_input_b & 0x02)
	{
		m_ic4_input_b &= ~0x02;
	}
	else
	{
		m_ic4_input_b |= 0x02; //Pulse the overcurrent line with every read to show the CPU each lamp has lit
	}
	#ifdef UNUSED_FUNCTION
	if ( lamp_undercurrent ) m_ic4_input_b |= 0x01;
	#endif

	LOG_IC3(("%s: IC4 PIA Read of Port B %x\n",machine().describe_context(),m_ic4_input_b));
	return m_ic4_input_b;
}


WRITE_LINE_MEMBER(mpu4_state::pia_ic4_ca2_w)
{
	LOG_IC3(("%s: IC4 PIA Write CA (input MUX strobe /LED B), %02X\n", machine().describe_context(),state));

	m_IC23GB = state;
	ic23_update();
}

WRITE_LINE_MEMBER(mpu4_state::pia_ic4_cb2_w)
{
	LOG_IC3(("%s: IC4 PIA Write CA (input MUX strobe /LED B), %02X\n", machine().describe_context(),state));
	m_reel_flag=state;
}

/* IC5, AUX ports, coin lockouts and AY sound chip select (MODs below 4 only) */
READ8_MEMBER(mpu4_state::pia_ic5_porta_r)
{
	if (m_lamp_extender == LARGE_CARD_A)
	{
		if (m_lamp_sense && m_ic23_active)
		{
			m_aux1_input |= 0x40;
		}
		else
		{
			m_aux1_input &= ~0x40; //Pulse the overcurrent line with every read to show the CPU each lamp has lit
		}
	}
	if (m_hopper == HOPPER_NONDUART_A)
	{
/*      if (hopper1_active)
        {
            m_aux1_input |= 0x04;
        }
        else
        {
            m_aux1_input &= ~0x04;
        }*/
	}
	LOG(("%s: IC5 PIA Read of Port A (AUX1)\n",machine().describe_context()));

	return m_aux1_port->read()|m_aux1_input;
}

WRITE8_MEMBER(mpu4_state::pia_ic5_porta_w)
{
	int i;
	pia6821_device *pia_ic4 = m_pia4;
	if (m_hopper == HOPPER_NONDUART_A)
	{
		//hopper1_drive_sensor(data&0x10);
	}
	switch (m_lamp_extender)
	{
	case NO_EXTENDER:
		if (m_led_extender == CARD_B)
		{
			led_write_latch(data & 0x1f, pia_ic4->a_output(),m_input_strobe);
		}
		else if ((m_led_extender != CARD_A)&&(m_led_extender != NO_EXTENDER))
		{
			for(i=0; i<8; i++)
			{
				output().set_indexed_value("mpu4led",((m_input_strobe + 8) * 8) +i,(data & (1 << i)) !=0);
			}
			output().set_digit_value((m_input_strobe+8),data);
		}
		break;

	case SMALL_CARD:
		if(m_ic23_active)
		{
			lamp_extend_small(data);
		}
		break;

	case LARGE_CARD_A:
		lamp_extend_large(data,m_input_strobe,m_ic23_active);
		break;

	case LARGE_CARD_B:
		lamp_extend_large(data,m_input_strobe,m_ic23_active);
		if ((m_ic23_active) && m_card_live)
		{
			for(i=0; i<8; i++)
			{
				output().set_indexed_value("mpu4led",(((8*(m_last_b7 >>7))+ m_input_strobe) * 8) +i,(~data & (1 << i)) !=0);
			}
			output().set_digit_value(((8*(m_last_b7 >>7))+m_input_strobe),~data);
		}
		break;

	case LARGE_CARD_C:
		lamp_extend_large(data,m_input_strobe,m_ic23_active);
		break;
	}
	if (m_reel_mux == SIX_REEL_5TO8)
	{
		m_reel4->update( data      &0x0F);
		m_reel5->update((data >> 4)&0x0F);
		awp_draw_reel(machine(),"reel5", m_reel4);
		awp_draw_reel(machine(),"reel6", m_reel5);
	}
	else
	if (m_reel_mux == SEVEN_REEL)
	{
		m_reel1->update( data      &0x0F);
		m_reel2->update((data >> 4)&0x0F);
		awp_draw_reel(machine(),"reel2", m_reel1);
		awp_draw_reel(machine(),"reel3", m_reel2);
	}

	if (core_stricmp(machine().system().name, "m4gambal") == 0)
	{
		/* The 'Gamball' device is a unique piece of mechanical equipment, designed to
		provide a truly fair hi-lo gamble for an AWP. Functionally, it consists of
		a ping-pong ball or similar enclosed in the machine's backbox, on a platform with 12
		holes. When the low 4 bytes of AUX1 are triggered, this fires the ball out from the
		hole it's currently in, to land in another. Landing in the same hole causes the machine to
		refire the ball. The ball detection is done by the high 4 bytes of AUX1.
		Here we call the MAME RNG, once to pick a row, once to pick from the four pockets within it. We
		then trigger the switches corresponding to the correct number. This appears to be the best way
		of making the game fair, short of simulating the physics of a bouncing ball ;)*/
		if (data & 0x0f)
		{
			switch ((machine().rand()>>5) % 0x3)
			{
			case 0x00: //Top row
				switch (machine().rand() & 0x3)
				{
				case 0x00: //7
					m_aux1_input = (m_aux1_input & 0x0f);
					m_aux1_input|= 0xa0;
					break;

				case 0x01://4
					m_aux1_input = (m_aux1_input & 0x0f);
					m_aux1_input|= 0xb0;
					break;

				case 0x02://9
					m_aux1_input = (m_aux1_input & 0x0f);
					m_aux1_input|= 0xc0;
					break;

				case 0x03://8
					m_aux1_input = (m_aux1_input & 0x0f);
					m_aux1_input|= 0xd0;
					break;
				}

			case 0x01: //Middle row - note switches don't match pattern
				switch (machine().rand() & 0x3)
				{
				case 0x00://12
					m_aux1_input = (m_aux1_input & 0x0f);
					m_aux1_input|= 0x40;
					break;

				case 0x01://1
					m_aux1_input = (m_aux1_input & 0x0f);
					m_aux1_input|= 0x50;
					break;

				case 0x02://11
					m_aux1_input = (m_aux1_input & 0x0f);
					m_aux1_input|= 0x80;
					break;

				case 0x03://2
					m_aux1_input = (m_aux1_input & 0x0f);
					m_aux1_input|= 0x90;
					break;
				}

			case 0x02: //Bottom row
				switch (machine().rand() & 0x3)
				{
				case 0x00://5
					m_aux1_input = (m_aux1_input & 0x0f);
					m_aux1_input|= 0x00;
					break;

				case 0x01://10
					m_aux1_input = (m_aux1_input & 0x0f);
					m_aux1_input|= 0x10;
					break;

				case 0x02://3
					m_aux1_input = (m_aux1_input & 0x0f);
					m_aux1_input|= 0x20;
					break;

				case 0x03://6
					m_aux1_input = (m_aux1_input & 0x0f);
					m_aux1_input|= 0x30;
					break;
				}
			}
		}
	}
}

WRITE8_MEMBER(mpu4_state::pia_ic5_portb_w)
{
	if (m_hopper == HOPPER_NONDUART_B)
	{
		//hopper1_drive_motor(data &0x01)
		//hopper1_drive_sensor(data &0x08)
	}
	if (m_led_extender == CARD_A)
	{
		// led_write_latch(data & 0x07, pia_get_output_a(pia_ic4),m_input_strobe)
	}

}
READ8_MEMBER(mpu4_state::pia_ic5_portb_r)
{
	if (m_hopper == HOPPER_NONDUART_B)
	{/*
        if (hopper1_active)
        {
            m_aux2_input |= 0x08;
        }
        else
        {
            m_aux2_input &= ~0x08;
        }*/
	}

	LOG(("%s: IC5 PIA Read of Port B (coin input AUX2)\n",machine().describe_context()));
	machine().bookkeeping().coin_lockout_w(0, (m_pia5->b_output() & 0x01) );
	machine().bookkeeping().coin_lockout_w(1, (m_pia5->b_output() & 0x02) );
	machine().bookkeeping().coin_lockout_w(2, (m_pia5->b_output() & 0x04) );
	machine().bookkeeping().coin_lockout_w(3, (m_pia5->b_output() & 0x08) );
	return m_aux2_port->read() | m_aux2_input;
}


WRITE_LINE_MEMBER(mpu4_state::pia_ic5_ca2_w)
{
	LOG(("%s: IC5 PIA Write CA2 (Serial Tx) %2x\n",machine().describe_context(),state));
	m_serial_data = state;
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
void mpu4_state::update_ay(device_t *device)
{
	ay8910_device *ay8910 = machine().device<ay8910_device>("ay8913");
	if (!ay8910) return;

	pia6821_device *pia = downcast<pia6821_device *>(device);
	if (!pia->cb2_output())
	{
		switch (m_ay8913_address)
		{
		case 0x00:
			/* Inactive */
			break;

		case 0x01:
			/* CA2 = 1 CB2 = 0? : Read from selected PSG register and make the register data available to Port A */
			LOG(("AY8913 address = %d \n",m_pia6->a_output()&0x0f));
			break;

		case 0x02:
			/* CA2 = 0 CB2 = 1? : Write to selected PSG register and write data to Port A */
			ay8910->data_w(generic_space(), 0, m_pia6->a_output());
			LOG(("AY Chip Write \n"));
			break;

		case 0x03:
			/* CA2 = 1 CB2 = 1? : The register will now be selected and the user can read from or write to it.
			The register will remain selected until another is chosen.*/
			ay8910->address_w(generic_space(), 0, m_pia6->a_output());
			LOG(("AY Chip Select \n"));
			break;

		default:
			LOG(("AY Chip error \n"));
			break;
		}
	}
}


WRITE_LINE_MEMBER(mpu4_state::pia_ic5_cb2_w)
{
	update_ay(m_pia5);
}


/* IC6, Reel A and B and AY registers (MODs below 4 only) */
WRITE8_MEMBER(mpu4_state::pia_ic6_portb_w)
{
	LOG(("%s: IC6 PIA Port B Set to %2x (Reel A and B)\n", machine().describe_context(),data));

	if (m_reel_mux == SEVEN_REEL)
	{
		m_reel3->update( data      &0x0F);
		m_reel4->update((data >> 4)&0x0F);
		awp_draw_reel(machine(),"reel4", m_reel3);
		awp_draw_reel(machine(),"reel5", m_reel4);
	}
	else if (m_reels)
	{
		m_reel0->update( data      &0x0F);
		m_reel1->update((data >> 4)&0x0F);
		awp_draw_reel(machine(),"reel1", m_reel0);
		awp_draw_reel(machine(),"reel2", m_reel1);
	}
}


WRITE8_MEMBER(mpu4_state::pia_ic6_porta_w)
{
	LOG(("%s: IC6 PIA Write A %2x\n", machine().describe_context(),data));
	if (m_mod_number <4)
	{
		m_ay_data = data;
		update_ay(m_pia6);
	}
}


WRITE_LINE_MEMBER(mpu4_state::pia_ic6_ca2_w)
{
	LOG(("%s: IC6 PIA write CA2 %2x (AY8913 BC1)\n", machine().describe_context(),state));
	if (m_mod_number <4)
	{
		if ( state ) m_ay8913_address |=  0x01;
		else         m_ay8913_address &= ~0x01;
		update_ay(m_pia6);
	}
}


WRITE_LINE_MEMBER(mpu4_state::pia_ic6_cb2_w)
{
	LOG(("%s: IC6 PIA write CB2 %2x (AY8913 BCDIR)\n", machine().describe_context(),state));
	if (m_mod_number <4)
	{
		if ( state ) m_ay8913_address |=  0x02;
		else         m_ay8913_address &= ~0x02;
		update_ay(m_pia6);
	}
}


/* IC7 Reel C and D, mechanical meters/Reel E and F, input strobe bit A */
WRITE8_MEMBER(mpu4_state::pia_ic7_porta_w)
{
	LOG(("%s: IC7 PIA Port A Set to %2x (Reel C and D)\n", machine().describe_context(),data));
	if (m_reel_mux == SEVEN_REEL)
	{
		m_reel5->update( data      &0x0F);
		m_reel6->update((data >> 4)&0x0F);
		awp_draw_reel(machine(),"reel6", m_reel5);
		awp_draw_reel(machine(),"reel7", m_reel7);
	}
	else if (m_reels)
	{
		m_reel2->update( data      &0x0F);
		m_reel3->update((data >> 4)&0x0F);
		awp_draw_reel(machine(),"reel3", m_reel2);
		awp_draw_reel(machine(),"reel4", m_reel3);
	}
}

WRITE8_MEMBER(mpu4_state::pia_ic7_portb_w)
{
	if (m_hopper == HOPPER_DUART_A)
	{
		//duart write data
	}
	else if (m_hopper == HOPPER_NONDUART_A)
	{
		//hoppr1_drive_motor(data & 0x10);
	}

	m_mmtr_data = data;
}

READ8_MEMBER(mpu4_state::pia_ic7_portb_r)
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

	int combined_meter = m_meters->GetActivity(0) | m_meters->GetActivity(1) |
							m_meters->GetActivity(2) | m_meters->GetActivity(3) |
							m_meters->GetActivity(4) | m_meters->GetActivity(5) |
							m_meters->GetActivity(6) | m_meters->GetActivity(7);

	if(combined_meter)
	{
		return 0x80;
	}
	else
	{
		return 0x00;
	}
}

WRITE_LINE_MEMBER(mpu4_state::pia_ic7_ca2_w)
{
	LOG(("%s: IC7 PIA write CA2 %2x (input strobe bit 0 / LED A)\n", machine().describe_context(),state));

	m_IC23GA = state;
	ic24_setup();
	ic23_update();
}

WRITE_LINE_MEMBER(mpu4_state::pia_ic7_cb2_w)
{
	m_remote_meter = state?0x80:0x00;
}


/* IC8, Inputs, TRIACS, alpha clock */
READ8_MEMBER(mpu4_state::pia_ic8_porta_r)
{
	ioport_port * portnames[] = { m_orange1_port, m_orange2_port, m_black1_port, m_black2_port, m_orange1_port, m_orange2_port, m_dil1_port, m_dil2_port };

	LOG_IC8(("%s: IC8 PIA Read of Port A (MUX input data)\n", machine().describe_context()));
/* The orange inputs are polled twice as often as the black ones, for reasons of efficiency.
   This is achieved via connecting every input line to an AND gate, thus allowing two strobes
   to represent each orange input bank (strobes are active low). */
	m_pia5->cb1_w(m_aux2_port->read() & 0x80);
	return (portnames[m_input_strobe])->read();
}


WRITE8_MEMBER(mpu4_state::pia_ic8_portb_w)
{
	if (m_hopper == HOPPER_DUART_B)
	{
//      duart.drive_sensor(data & 0x04, data & 0x01, 0, 0);
	}
	else if (m_hopper == HOPPER_DUART_C)
	{
//      duart.drive_sensor(data & 0x04, data & 0x01, data & 0x04, data & 0x02);
	}
	int i;
	LOG_IC8(("%s: IC8 PIA Port B Set to %2x (OUTPUT PORT, TRIACS)\n", machine().describe_context(),data));
	for (i = 0; i < 8; i++)
	{
		output().set_indexed_value("triac", i, data & (1 << i));
	}
}

WRITE_LINE_MEMBER(mpu4_state::pia_ic8_ca2_w)
{
	LOG_IC8(("%s: IC8 PIA write CA2 (input_strobe bit 2 / LED C) %02X\n", machine().describe_context(), state & 0xFF));

	m_IC23GC = state;
	ic23_update();
}


WRITE_LINE_MEMBER(mpu4_state::pia_ic8_cb2_w)
{
	LOG_IC8(("%s: IC8 PIA write CB2 (alpha clock) %02X\n", machine().describe_context(), state & 0xFF));

	// DM Data pin B

	m_vfd->sclk(!state);
}

// universal sampled sound program card PCB 683077
// Sampled sound card, using a PIA and PTM for timing and data handling
WRITE8_MEMBER(mpu4_state::pia_gb_porta_w)
{
	LOG_SS(("%s: GAMEBOARD: PIA Port A Set to %2x\n", machine().describe_context(),data));
	m_msm6376->write(space, 0, data);
}

WRITE8_MEMBER(mpu4_state::pia_gb_portb_w)
{
	int changed = m_expansion_latch^data;

	LOG_SS(("%s: GAMEBOARD: PIA Port B Set to %2x\n", machine().describe_context(),data));

	if ( changed & 0x20)
	{ // digital volume clock line changed
		if ( !(data & 0x20) )
		{ // changed from high to low,
			if ( !(data & 0x10) )//down
			{
				if ( m_global_volume < 32 ) m_global_volume++; //steps unknown
			}
			else
			{//up
				if ( m_global_volume > 0  ) m_global_volume--;
			}

			{
				float percent = (32-m_global_volume)/32.0;
				m_msm6376->set_output_gain(0, percent);
				m_msm6376->set_output_gain(1, percent);
			}
		}
	}
	m_msm6376->ch2_w(data&0x02);
	m_msm6376->st_w(data&0x01);
}
READ8_MEMBER(mpu4_state::pia_gb_portb_r)
{
	LOG_SS(("%s: GAMEBOARD: PIA Read of Port B\n",machine().describe_context()));
	int data=0;
	// b7 NAR - we can load another address into Channel 1
	// b6, 1 = OKI ready, 0 = OKI busy
	// b5, vol clock
	// b4, 1 = Vol down, 0 = Vol up
	//

	if ( m_msm6376->nar_r() ) data |= 0x80;
	else                           data &= ~0x80;

	if ( m_msm6376->busy_r() ) data |= 0x40;
	else                            data &= ~0x40;

	return ( data | m_expansion_latch );
}

WRITE_LINE_MEMBER(mpu4_state::pia_gb_ca2_w)
{
	LOG_SS(("%s: GAMEBOARD: OKI RESET data = %02X\n", machine().describe_context(), state));

//  reset line
}

WRITE_LINE_MEMBER(mpu4_state::pia_gb_cb2_w)
{
	//Some BWB games use this to drive the bankswitching
	if (m_bwb_bank)
	{
		//printf("pia_gb_cb2_w %d\n", state);
		m_pageval = state;
		m_bank1->set_entry((m_pageval + (m_pageset ? 4 : 0)) & m_numbanks);
	}
}

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

/* This is a bit of a cheat - since we don't clock into the OKI chip directly, we need to
calculate the oscillation frequency in advance. We're running the timer for interrupt
purposes, but the frequency calculation is done by plucking the values out as they are written.*/
WRITE8_MEMBER(mpu4_state::ic3ss_w)
{
	device_t *ic3ss = machine().device("ptm_ic3ss");
	downcast<ptm6840_device *>(ic3ss)->write(offset,data);

	if (offset == 3)
	{
		m_t1 = data;
	}
	if (offset == 6)
	{
		m_t3h = data;
	}
	if (offset == 7)
	{
		m_t3l = data;
	}

	float num = (1720000/((m_t3l + 1)*(m_t3h + 1)));
	float denom1 = ((m_t3h *(m_t3l + 1)+ 1)/(2*(m_t1 + 1)));

	int denom2 = denom1 + 0.5f;//need to round up, this gives same precision as chip
	int freq=num*denom2;

	if (freq)
	{
		m_msm6376->set_frequency(freq);
	}
}

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
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_OTHER)   PORT_NAME("20")
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
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_COIN1) PORT_NAME("10p")//PORT_IMPULSE(5)
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_COIN2) PORT_NAME("20p")//PORT_IMPULSE(5)
	PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_COIN3) PORT_NAME("50p")//PORT_IMPULSE(5)
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_COIN4) PORT_NAME("100p")//PORT_IMPULSE(5)
INPUT_PORTS_END


INPUT_PORTS_START( mpu4_cw )
//Inputs for CoinWorld games
	PORT_INCLUDE( mpu4 )
	PORT_MODIFY("DIL1")
	PORT_DIPNAME( 0x01, 0x00, "Profile Type" ) PORT_DIPLOCATION("DIL1:01")
	PORT_DIPSETTING(    0x00, "Bingo Profile" )
	PORT_DIPSETTING(    0x01, "Arcade" )
	PORT_DIPNAME( 0x02, 0x00, "Accept 2 GBP Coin?" ) PORT_DIPLOCATION("DIL1:02")
	PORT_DIPSETTING(    0x00, DEF_STR( No ) )
	PORT_DIPSETTING(    0x02, DEF_STR( Yes  ) )
	PORT_DIPNAME( 0x0C, 0x00, "Jackpot" ) PORT_DIPLOCATION("DIL1:03,04")
	PORT_DIPSETTING(    0x04, "15 GBP" )
	PORT_DIPSETTING(    0x00, "10 GBP" )
	PORT_DIPSETTING(    0x08, "5 GBP" )
	PORT_DIPNAME( 0x10, 0x00, "Hold Mode" ) PORT_DIPLOCATION("DIL1:05")
	PORT_DIPSETTING(    0x00, "Show Hints" )
	PORT_DIPSETTING(    0x10, "Auto Hold" )
	PORT_DIPNAME( 0x20, 0x00, "Coin Mech Type" ) PORT_DIPLOCATION("DIL1:05")
	PORT_DIPSETTING(    0x00, "6 Coin" )
	PORT_DIPSETTING(    0x20, "5 Coin" )
	PORT_DIPNAME( 0x40, 0x00, "Reel Motor Type" ) PORT_DIPLOCATION("DIL1:05")
	PORT_DIPSETTING(    0x00, "Slim motor" )
	PORT_DIPSETTING(    0x40, "Fat motor" )
	PORT_DIPNAME( 0x80, 0x00, "Payout Tube" ) PORT_DIPLOCATION("DIL1:05")
	PORT_DIPSETTING(    0x00, "20p" )
	PORT_DIPSETTING(    0x80, "10p" )

	PORT_MODIFY("DIL2")
	PORT_DIPNAME( 0x07, 0x00, "Stake Setting" )
	PORT_DIPSETTING(    0x00, "Not fitted / 5p"  )
	PORT_DIPSETTING(    0x01, "10p" )
	PORT_DIPSETTING(    0x02, "20p" )
	PORT_DIPSETTING(    0x03, "25p" )
	PORT_DIPSETTING(    0x04, "30p" )
	PORT_BIT(0xE0, IP_ACTIVE_HIGH, IPT_UNUSED)
	INPUT_PORTS_END

INPUT_PORTS_START( mpu4jackpot8tkn )
	PORT_INCLUDE( mpu4 )

	PORT_MODIFY("ORANGE2")
	PORT_CONFNAME( 0x0F, 0x06, "Jackpot / Prize Key" )
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
INPUT_PORTS_END

INPUT_PORTS_START( mpu4jackpot8per )
	PORT_INCLUDE( mpu4 )

	PORT_MODIFY("ORANGE2")
	PORT_CONFNAME( 0x0F, 0x06, "Jackpot / Prize Key" )
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

	PORT_CONFNAME( 0xF0, 0x10, "Percentage Key" )
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
INPUT_PORTS_END




INPUT_PORTS_START( grtecp )
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
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_COIN1) PORT_NAME("10p")//PORT_IMPULSE(5)
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_COIN2) PORT_NAME("20p")//PORT_IMPULSE(5)
	PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_COIN3) PORT_NAME("50p")//PORT_IMPULSE(5)
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_COIN4) PORT_NAME("100p")//PORT_IMPULSE(5)
INPUT_PORTS_END



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

Apparently, just before the characteriser is checked bit 1 at 0x61DF is checked and if zero the characteriser
check is bypassed. This may be something to look at for prototype ROMs and hacks.

*/


WRITE8_MEMBER(mpu4_state::characteriser_w)
{
	int x;
	int call=data;
	LOG_CHR_FULL(("%04x Characteriser write offset %02X data %02X", space.device().safe_pcbase(),offset,data));
	if (!m_current_chr_table)
	{
		logerror("No Characteriser Table @ %04x\n", space.device().safe_pcbase());
		return;
	}



	if (offset == 0)
	{
		{
			if (call == 0)
			{
				m_prot_col = 0;
			}
			else
			{
				for (x = m_prot_col; x < 64; x++)
				{
					if  (m_current_chr_table[(x)].call == call)
					{
						m_prot_col = x;
						LOG_CHR(("Characteriser find column %02X\n",m_prot_col));
						break;
					}
				}
			}
		}
	}
	else if (offset == 2)
	{
		LOG_CHR(("Characteriser write 2 data %02X\n",data));
		// Rather than the search strategy, we can map the calls directly here. Note that they are hex versions of the square number series
		switch (call)
		{
		case 0x00:
			m_lamp_col = 0;
			break;

		case 0x01:
			m_lamp_col = 1;
			break;

		case 0x04:
			m_lamp_col = 2;
			break;

		case 0x09:
			m_lamp_col = 3;
			break;

		case 0x10:
			m_lamp_col = 4;
			break;

		case 0x19:
			m_lamp_col = 5;
			break;

		case 0x24:
			m_lamp_col = 6;
			break;

		case 0x31:
			m_lamp_col = 7;
			break;
		}
		LOG_CHR(("Characteriser find 2 column %02X\n",m_lamp_col));
	}
}


READ8_MEMBER(mpu4_state::characteriser_r)
{
	if (!m_current_chr_table)
	{
		logerror("No Characteriser Table @ %04x", space.device().safe_pcbase());

		/* a cheat ... many early games use a standard check */
		int addr = space.device().state().state_int(M6809_X);
		if ((addr>=0x800) && (addr<=0xfff)) return 0x00; // prevent recursion, only care about ram/rom areas for this cheat.

		UINT8 ret = space.read_byte(addr);
		logerror(" (returning %02x)",ret);

		logerror("\n");

		return ret;
	}

	LOG_CHR(("Characteriser read offset %02X \n",offset));
	if (offset == 0)
	{
		LOG_CHR(("Characteriser read data %02X \n",m_current_chr_table[m_prot_col].response));
		return m_current_chr_table[m_prot_col].response;
	}

	if (offset == 3)
	{
		LOG_CHR(("Characteriser read data off 3 %02X \n",m_current_chr_table[m_lamp_col+64].response));
		return m_current_chr_table[m_lamp_col+64].response;
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


WRITE8_MEMBER(mpu4_state::bwb_characteriser_w)
{
	int x;
	int call=data;
	LOG_CHR_FULL(("%04x Characteriser write offset %02X data %02X \n", space.device().safe_pcbase(),offset,data));
	if (!m_current_chr_table)
		fatalerror("No Characteriser Table @ %04x\n", space.device().safe_pcbase());

	if ((offset & 0x3f)== 0)//initialisation is always at 0x800
	{
		if (!m_chr_state)
		{
			m_chr_state=1;
			m_chr_counter=0;
		}
		if (call == 0)
		{
			m_init_col ++;
		}
		else
		{
			m_init_col =0;
		}
	}

	m_chr_value = machine().rand();
	for (x = 0; x < 4; x++)
	{
		if  (m_current_chr_table[(x)].call == call)
		{
			if (x == 0) // reinit
			{
				m_bwb_return = 0;
			}
			m_chr_value = bwb_chr_table_common[(m_bwb_return)];
			m_bwb_return++;
			break;
		}
	}
}

READ8_MEMBER(mpu4_state::bwb_characteriser_r)
{
	LOG_CHR(("Characteriser read offset %02X \n",offset));


	if (offset ==0)
	{
		switch (m_chr_counter)
		{
		case 6:
		case 13:
		case 20:
		case 27:
		case 34:
			return m_bwb_chr_table1[(((m_chr_counter + 1) / 7) - 1)].response;

		default:
			if (m_chr_counter > 34)
			{
				m_chr_counter = 35;
				m_chr_state = 2;
			}
			m_chr_counter ++;
			return m_chr_value;
		}
	}
	else
	{
		return m_chr_value;
	}
}

/* Common configurations */

WRITE8_MEMBER(mpu4_state::mpu4_ym2413_w)
{
	ym2413_device *ym2413 = machine().device<ym2413_device>("ym2413");
	if (ym2413) ym2413->write(space,offset,data);
}

READ8_MEMBER(mpu4_state::mpu4_ym2413_r)
{
//  ym2413_device *ym2413 = machine().device<ym2413_device>("ym2413");
//  if (ym2413) return ym2413->read(space,offset);
	return 0xff;
}


void mpu4_state::mpu4_install_mod4yam_space(address_space &space)
{
	space.install_read_handler(0x0880, 0x0882, read8_delegate(FUNC(mpu4_state::mpu4_ym2413_r),this));
	space.install_write_handler(0x0880, 0x0881, write8_delegate(FUNC(mpu4_state::mpu4_ym2413_w),this));
}

void mpu4_state::mpu4_install_mod4oki_space(address_space &space)
{
	pia6821_device *pia_ic4ss = space.machine().device<pia6821_device>("pia_ic4ss");
	ptm6840_device *ptm_ic3ss = space.machine().device<ptm6840_device>("ptm_ic3ss");

	space.install_readwrite_handler(0x0880, 0x0883, 0, 0, read8_delegate(FUNC(pia6821_device::read), pia_ic4ss), write8_delegate(FUNC(pia6821_device::write), pia_ic4ss));
	space.install_read_handler(0x08c0, 0x08c7, 0, 0, read8_delegate(FUNC(ptm6840_device::read), ptm_ic3ss));
	space.install_write_handler(0x08c0, 0x08c7, 0, 0, write8_delegate(FUNC(mpu4_state::ic3ss_w),this));
}

void mpu4_state::mpu4_install_mod4bwb_space(address_space &space)
{
	space.install_readwrite_handler(0x0810, 0x0810, 0, 0, read8_delegate(FUNC(mpu4_state::bwb_characteriser_r),this),write8_delegate(FUNC(mpu4_state::bwb_characteriser_w),this));
	mpu4_install_mod4oki_space(space);
}


void mpu4_state::mpu4_config_common()
{
	m_ic24_timer = timer_alloc(TIMER_IC24);
	m_lamp_strobe_ext_persistence = 0;
}

MACHINE_START_MEMBER(mpu4_state,mod2)
{
	mpu4_config_common();

	m_link7a_connected=0;
	m_mod_number=2;
}

MACHINE_START_MEMBER(mpu4_state,mpu4yam)
{
	address_space &space = m_maincpu->space(AS_PROGRAM);
	mpu4_config_common();

	m_link7a_connected=0;
	m_mod_number=4;
	mpu4_install_mod4yam_space(space);
}

MACHINE_START_MEMBER(mpu4_state,mpu4oki)
{
	address_space &space = m_maincpu->space(AS_PROGRAM);
	mpu4_config_common();

	m_link7a_connected=0;
	m_mod_number=4;
	mpu4_install_mod4oki_space(space);
}

MACHINE_START_MEMBER(mpu4_state,mpu4bwb)
{
	address_space &space = m_maincpu->space(AS_PROGRAM);
	mpu4_config_common();

	m_link7a_connected=0;
	m_mod_number=4;
	mpu4_install_mod4bwb_space(space);
}

MACHINE_START_MEMBER(mpu4_state,mpu4cry)
{
	mpu4_config_common();

	m_link7a_connected=0;
	m_mod_number=4;
}

/* CHR Tables */

static mpu4_chr_table ccelbr_data[72] = {
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


static mpu4_chr_table gmball_data[72] = {
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




static mpu4_chr_table grtecp_data[72] = {
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

static mpu4_chr_table oldtmr_data[72] = {
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

static mpu4_chr_table blsbys_data[8] = {
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

DRIVER_INIT_MEMBER(mpu4_state,m_oldtmr)
{
	m_reel_mux=SIX_REEL_1TO8;
	m_reels = 6;

	DRIVER_INIT_CALL(m4default_banks);

	m_current_chr_table = oldtmr_data;
}

DRIVER_INIT_MEMBER(mpu4_state,m4altreels)
{
	m_reel_mux=SIX_REEL_1TO8;
	m_reels = 6;

	DRIVER_INIT_CALL(m4default_banks);
}


DRIVER_INIT_MEMBER(mpu4_state,m_ccelbr)
{
	DRIVER_INIT_CALL(m4default);
	m_current_chr_table = ccelbr_data;
}

DRIVER_INIT_MEMBER(mpu4_state,m4gambal)
{
	DRIVER_INIT_CALL(m4default);
	m_current_chr_table = gmball_data;
}

DRIVER_INIT_MEMBER(mpu4_state,m_grtecp)
{
	m_reel_mux=FIVE_REEL_5TO8;
	m_reels = 5;
	m_lamp_extender=SMALL_CARD;
	DRIVER_INIT_CALL(m4default_banks);

	m_current_chr_table = grtecp_data;
}

DRIVER_INIT_MEMBER(mpu4_state,m_blsbys)
{
	m_bwb_bank=1;
	m_reel_mux=FIVE_REEL_5TO8;
	m_reels = 5;
	m_bwb_chr_table1 = blsbys_data1;
	m_current_chr_table = blsbys_data;
	DRIVER_INIT_CALL(m4default_big);
}

DRIVER_INIT_MEMBER(mpu4_state,m4default_reels)
{
	m_reel_mux=STANDARD_REEL;
	m_reels = 4;
	m_bwb_bank=0;
}

DRIVER_INIT_MEMBER(mpu4_state,m4default_banks)
{
	//Initialise paging for non-extended ROM space
	UINT8 *rom = memregion("maincpu")->base();
	membank("bank1")->configure_entries(0, 4, &rom[0x01000], 0x10000);
	membank("bank1")->set_entry(0);
}

DRIVER_INIT_MEMBER(mpu4_state,m4default_alt)
{
	m_reel_mux=STANDARD_REEL;
	m_reels = 8;
	DRIVER_INIT_CALL(m4default_banks);

	m_bwb_bank=0;
}

DRIVER_INIT_MEMBER(mpu4_state,m4default)
{
	DRIVER_INIT_CALL(m4default_reels);
	DRIVER_INIT_CALL(m4default_banks);
}

DRIVER_INIT_MEMBER(mpu4_state,m4default_big)
{
	address_space &space = m_maincpu->space(AS_PROGRAM);

	int size = memregion( "maincpu" )->bytes();
	if (size<=0x10000)
	{
		printf("Error: Extended banking selected on set <=0x10000 in size, ignoring\n");
		DRIVER_INIT_CALL(m4default_reels);
		DRIVER_INIT_CALL(m4default_banks);
	}
	else
	{
		m_bwb_bank=1;
		space.install_write_handler(0x0858, 0x0858, 0, 0, write8_delegate(FUNC(mpu4_state::bankswitch_w),this));
		space.install_write_handler(0x0878, 0x0878, 0, 0, write8_delegate(FUNC(mpu4_state::bankset_w),this));
		UINT8 *rom = memregion("maincpu")->base();

		m_numbanks = size / 0x10000;

		m_bank1->configure_entries(0, m_numbanks, &rom[0x01000], 0x10000);

		m_numbanks--;

		// some Bwb games must default to the last bank, does anything not like this
		// behavior?
		// some Bwb games don't work anyway tho, they seem to dislike something else
		// about the way the regular banking behaves, not related to the CB2 stuff
		m_bank1->set_entry(m_numbanks);
	}
}





READ8_MEMBER(mpu4_state::crystal_sound_r)
{
	return machine().rand();
}
//this may be a YMZ280B
WRITE8_MEMBER(mpu4_state::crystal_sound_w)
{
	printf("crystal_sound_w %02x\n",data);
}

DRIVER_INIT_MEMBER(mpu4_state,m_frkstn)
{
	address_space &space = m_maincpu->space(AS_PROGRAM);
	DRIVER_INIT_CALL(m4default_big);
	space.install_read_handler(0x0880, 0x0880, 0, 0, read8_delegate(FUNC(mpu4_state::crystal_sound_r),this));
	space.install_write_handler(0x0881, 0x0881, 0, 0, write8_delegate(FUNC(mpu4_state::crystal_sound_w),this));
}

// thanks to Project Amber for descramble information
static void descramble_crystal( UINT8* region, int start, int end, UINT8 extra_xor)
{
	for (int i=start;i<end;i++)
	{
		UINT8 x = region[i];
		switch (i & 0x58)
		{
		case 0x00: // same as 0x08
		case 0x08: x = BITSWAP8( x^0xca , 3,2,1,0,7,4,6,5 ); break;
		case 0x10: x = BITSWAP8( x^0x30 , 3,0,4,6,1,5,7,2 ); break;
		case 0x18: x = BITSWAP8( x^0x89 , 4,1,2,5,7,0,6,3 ); break;
		case 0x40: x = BITSWAP8( x^0x14 , 6,1,4,3,2,5,0,7 ); break;
		case 0x48: x = BITSWAP8( x^0x40 , 1,0,3,2,5,4,7,6 ); break;
		case 0x50: x = BITSWAP8( x^0xcb , 3,2,1,0,7,6,5,4 ); break;
		case 0x58: x = BITSWAP8( x^0xc0 , 2,3,6,0,5,1,7,4 ); break;
		}
		region[i] = x ^ extra_xor;
	}
}


DRIVER_INIT_MEMBER(mpu4_state,crystal)
{
	DRIVER_INIT_CALL(m_frkstn);
	descramble_crystal(memregion( "maincpu" )->base(), 0x0000, 0x10000, 0x00);
}

DRIVER_INIT_MEMBER(mpu4_state,crystali)
{
	DRIVER_INIT_CALL(m_frkstn);
	descramble_crystal(memregion( "maincpu" )->base(), 0x0000, 0x10000, 0xff); // invert after decrypt?!
}

/* generate a 50 Hz signal (based on an RC time) */
TIMER_DEVICE_CALLBACK_MEMBER(mpu4_state::gen_50hz)
{
	/* Although reported as a '50Hz' signal, the fact that both rising and
	falling edges of the pulse are used means the timer actually gives a 100Hz
	oscillating signal.*/
	m_signal_50hz = m_signal_50hz?0:1;
	m_pia4->ca1_w(m_signal_50hz);  /* signal is connected to IC4 CA1 */

	update_meters();//run at 100Hz to sync with PIAs
}

static ADDRESS_MAP_START( mpu4_memmap, AS_PROGRAM, 8, mpu4_state )
	AM_RANGE(0x0000, 0x07ff) AM_RAM AM_SHARE("nvram")
	AM_RANGE(0x0800, 0x0810) AM_READWRITE(characteriser_r,characteriser_w)
	AM_RANGE(0x0850, 0x0850) AM_READWRITE(bankswitch_r,bankswitch_w)    /* write bank (rom page select) */
/*  AM_RANGE(0x08e0, 0x08e7) AM_READWRITE(68681_duart_r,68681_duart_w) */ //Runs hoppers
	AM_RANGE(0x0900, 0x0907) AM_DEVREADWRITE("ptm_ic2", ptm6840_device, read, write)/* PTM6840 IC2 */
	AM_RANGE(0x0a00, 0x0a03) AM_DEVREADWRITE("pia_ic3", pia6821_device, read, write)        /* PIA6821 IC3 */
	AM_RANGE(0x0b00, 0x0b03) AM_DEVREADWRITE("pia_ic4", pia6821_device, read, write)        /* PIA6821 IC4 */
	AM_RANGE(0x0c00, 0x0c03) AM_DEVREADWRITE("pia_ic5", pia6821_device, read, write)        /* PIA6821 IC5 */
	AM_RANGE(0x0d00, 0x0d03) AM_DEVREADWRITE("pia_ic6", pia6821_device, read, write)        /* PIA6821 IC6 */
	AM_RANGE(0x0e00, 0x0e03) AM_DEVREADWRITE("pia_ic7", pia6821_device, read, write)        /* PIA6821 IC7 */
	AM_RANGE(0x0f00, 0x0f03) AM_DEVREADWRITE("pia_ic8", pia6821_device, read, write)        /* PIA6821 IC8 */
	AM_RANGE(0x1000, 0xffff) AM_ROMBANK("bank1")    /* 64k  paged ROM (4 pages)  */
ADDRESS_MAP_END

#define MCFG_MPU4_STD_REEL_ADD(_tag)\
	MCFG_STEPPER_ADD(_tag)\
	MCFG_STEPPER_REEL_TYPE(BARCREST_48STEP_REEL)\
	MCFG_STEPPER_START_INDEX(1)\
	MCFG_STEPPER_END_INDEX(3)\
	MCFG_STEPPER_INDEX_PATTERN(0x00)\
	MCFG_STEPPER_INIT_PHASE(2)

#define MCFG_MPU4_TYPE2_REEL_ADD(_tag)\
	MCFG_STEPPER_ADD(_tag)\
	MCFG_STEPPER_REEL_TYPE(BARCREST_48STEP_REEL)\
	MCFG_STEPPER_START_INDEX(4)\
	MCFG_STEPPER_END_INDEX(12)\
	MCFG_STEPPER_INDEX_PATTERN(0x00)\
	MCFG_STEPPER_INIT_PHASE(2)

#define MCFG_MPU4_TYPE3_REEL_ADD(_tag)\
	MCFG_STEPPER_ADD(_tag)\
	MCFG_STEPPER_REEL_TYPE(BARCREST_48STEP_REEL)\
	MCFG_STEPPER_START_INDEX(92)\
	MCFG_STEPPER_END_INDEX(3)\
	MCFG_STEPPER_INDEX_PATTERN(0x00)\
	MCFG_STEPPER_INIT_PHASE(2)

#define MCFG_MPU4_BWB_REEL_ADD(_tag)\
	MCFG_STEPPER_ADD(_tag)\
	MCFG_STEPPER_REEL_TYPE(BARCREST_48STEP_REEL)\
	MCFG_STEPPER_START_INDEX(96)\
	MCFG_STEPPER_END_INDEX(3)\
	MCFG_STEPPER_INDEX_PATTERN(0x00)\
	MCFG_STEPPER_INIT_PHASE(2)


MACHINE_CONFIG_FRAGMENT( mpu4_std_4reel )
	MCFG_MPU4_STD_REEL_ADD("reel0")
	MCFG_STEPPER_OPTIC_CALLBACK(WRITELINE(mpu4_state, reel0_optic_cb))
	MCFG_MPU4_STD_REEL_ADD("reel1")
	MCFG_STEPPER_OPTIC_CALLBACK(WRITELINE(mpu4_state, reel1_optic_cb))
	MCFG_MPU4_STD_REEL_ADD("reel2")
	MCFG_STEPPER_OPTIC_CALLBACK(WRITELINE(mpu4_state, reel2_optic_cb))
	MCFG_MPU4_STD_REEL_ADD("reel3")
	MCFG_STEPPER_OPTIC_CALLBACK(WRITELINE(mpu4_state, reel3_optic_cb))
MACHINE_CONFIG_END

MACHINE_CONFIG_FRAGMENT( mpu4_std_5reel )
	MCFG_MPU4_STD_REEL_ADD("reel0")
	MCFG_STEPPER_OPTIC_CALLBACK(WRITELINE(mpu4_state, reel0_optic_cb))
	MCFG_MPU4_STD_REEL_ADD("reel1")
	MCFG_STEPPER_OPTIC_CALLBACK(WRITELINE(mpu4_state, reel1_optic_cb))
	MCFG_MPU4_STD_REEL_ADD("reel2")
	MCFG_STEPPER_OPTIC_CALLBACK(WRITELINE(mpu4_state, reel2_optic_cb))
	MCFG_MPU4_STD_REEL_ADD("reel3")
	MCFG_STEPPER_OPTIC_CALLBACK(WRITELINE(mpu4_state, reel3_optic_cb))
	MCFG_MPU4_STD_REEL_ADD("reel4")
	MCFG_STEPPER_OPTIC_CALLBACK(WRITELINE(mpu4_state, reel4_optic_cb))
MACHINE_CONFIG_END

MACHINE_CONFIG_FRAGMENT( mpu4_std_6reel )
	MCFG_MPU4_STD_REEL_ADD("reel0")
	MCFG_STEPPER_OPTIC_CALLBACK(WRITELINE(mpu4_state, reel0_optic_cb))
	MCFG_MPU4_STD_REEL_ADD("reel1")
	MCFG_STEPPER_OPTIC_CALLBACK(WRITELINE(mpu4_state, reel1_optic_cb))
	MCFG_MPU4_STD_REEL_ADD("reel2")
	MCFG_STEPPER_OPTIC_CALLBACK(WRITELINE(mpu4_state, reel2_optic_cb))
	MCFG_MPU4_STD_REEL_ADD("reel3")
	MCFG_STEPPER_OPTIC_CALLBACK(WRITELINE(mpu4_state, reel3_optic_cb))
	MCFG_MPU4_STD_REEL_ADD("reel4")
	MCFG_STEPPER_OPTIC_CALLBACK(WRITELINE(mpu4_state, reel4_optic_cb))
	MCFG_MPU4_STD_REEL_ADD("reel5")
	MCFG_STEPPER_OPTIC_CALLBACK(WRITELINE(mpu4_state, reel4_optic_cb))
MACHINE_CONFIG_END

MACHINE_CONFIG_FRAGMENT( mpu4_type2_6reel )
	MCFG_MPU4_TYPE2_REEL_ADD("reel0")
	MCFG_STEPPER_OPTIC_CALLBACK(WRITELINE(mpu4_state, reel0_optic_cb))
	MCFG_MPU4_TYPE2_REEL_ADD("reel1")
	MCFG_STEPPER_OPTIC_CALLBACK(WRITELINE(mpu4_state, reel1_optic_cb))
	MCFG_MPU4_TYPE2_REEL_ADD("reel2")
	MCFG_STEPPER_OPTIC_CALLBACK(WRITELINE(mpu4_state, reel2_optic_cb))
	MCFG_MPU4_TYPE2_REEL_ADD("reel3")
	MCFG_STEPPER_OPTIC_CALLBACK(WRITELINE(mpu4_state, reel3_optic_cb))
	MCFG_MPU4_TYPE2_REEL_ADD("reel4")
	MCFG_STEPPER_OPTIC_CALLBACK(WRITELINE(mpu4_state, reel4_optic_cb))
	MCFG_MPU4_TYPE2_REEL_ADD("reel5")
	MCFG_STEPPER_OPTIC_CALLBACK(WRITELINE(mpu4_state, reel5_optic_cb))
MACHINE_CONFIG_END


MACHINE_CONFIG_FRAGMENT( mpu4_bwb_5reel )
	MCFG_MPU4_BWB_REEL_ADD("reel0")
	MCFG_STEPPER_OPTIC_CALLBACK(WRITELINE(mpu4_state, reel0_optic_cb))
	MCFG_MPU4_BWB_REEL_ADD("reel1")
	MCFG_STEPPER_OPTIC_CALLBACK(WRITELINE(mpu4_state, reel1_optic_cb))
	MCFG_MPU4_BWB_REEL_ADD("reel2")
	MCFG_STEPPER_OPTIC_CALLBACK(WRITELINE(mpu4_state, reel2_optic_cb))
	MCFG_MPU4_BWB_REEL_ADD("reel3")
	MCFG_STEPPER_OPTIC_CALLBACK(WRITELINE(mpu4_state, reel3_optic_cb))
	MCFG_MPU4_BWB_REEL_ADD("reel4")
	MCFG_STEPPER_OPTIC_CALLBACK(WRITELINE(mpu4_state, reel4_optic_cb))
MACHINE_CONFIG_END

MACHINE_CONFIG_FRAGMENT( mpu4_alt_7reel )
	MCFG_MPU4_TYPE3_REEL_ADD("reel0")
	MCFG_STEPPER_OPTIC_CALLBACK(WRITELINE(mpu4_state, reel0_optic_cb))
	MCFG_MPU4_TYPE3_REEL_ADD("reel1")
	MCFG_STEPPER_OPTIC_CALLBACK(WRITELINE(mpu4_state, reel1_optic_cb))
	MCFG_MPU4_TYPE3_REEL_ADD("reel2")
	MCFG_STEPPER_OPTIC_CALLBACK(WRITELINE(mpu4_state, reel2_optic_cb))
	MCFG_MPU4_TYPE3_REEL_ADD("reel3")
	MCFG_STEPPER_OPTIC_CALLBACK(WRITELINE(mpu4_state, reel3_optic_cb))
	MCFG_MPU4_TYPE3_REEL_ADD("reel4")
	MCFG_STEPPER_OPTIC_CALLBACK(WRITELINE(mpu4_state, reel4_optic_cb))
	MCFG_MPU4_TYPE3_REEL_ADD("reel5")
	MCFG_STEPPER_OPTIC_CALLBACK(WRITELINE(mpu4_state, reel5_optic_cb))
	MCFG_MPU4_TYPE3_REEL_ADD("reel6")
	MCFG_STEPPER_OPTIC_CALLBACK(WRITELINE(mpu4_state, reel6_optic_cb))
	MCFG_MPU4_TYPE3_REEL_ADD("reel7")
	MCFG_STEPPER_OPTIC_CALLBACK(WRITELINE(mpu4_state, reel7_optic_cb))
MACHINE_CONFIG_END

MACHINE_CONFIG_FRAGMENT( mpu4_common )
	MCFG_TIMER_DRIVER_ADD_PERIODIC("50hz", mpu4_state, gen_50hz, attotime::from_hz(100))

	MCFG_MSC1937_ADD("vfd",0)
	/* 6840 PTM */
	MCFG_DEVICE_ADD("ptm_ic2", PTM6840, 0)
	MCFG_PTM6840_INTERNAL_CLOCK(MPU4_MASTER_CLOCK / 4)
	MCFG_PTM6840_EXTERNAL_CLOCKS(0, 0, 0)
	MCFG_PTM6840_OUT0_CB(WRITE8(mpu4_state, ic2_o1_callback))
	MCFG_PTM6840_OUT1_CB(WRITE8(mpu4_state, ic2_o2_callback))
	MCFG_PTM6840_OUT2_CB(WRITE8(mpu4_state, ic2_o3_callback))
	MCFG_PTM6840_IRQ_CB(WRITELINE(mpu4_state, cpu0_irq))

	MCFG_DEVICE_ADD("pia_ic3", PIA6821, 0)
	MCFG_PIA_WRITEPA_HANDLER(WRITE8(mpu4_state, pia_ic3_porta_w))
	MCFG_PIA_WRITEPB_HANDLER(WRITE8(mpu4_state, pia_ic3_portb_w))
	MCFG_PIA_CA2_HANDLER(WRITELINE(mpu4_state, pia_ic3_ca2_w))
	MCFG_PIA_CB2_HANDLER(WRITELINE(mpu4_state, pia_ic3_cb2_w))
	MCFG_PIA_IRQA_HANDLER(WRITELINE(mpu4_state, cpu0_irq))
	MCFG_PIA_IRQB_HANDLER(WRITELINE(mpu4_state, cpu0_irq))

	MCFG_DEVICE_ADD("pia_ic4", PIA6821, 0)
	MCFG_PIA_READPB_HANDLER(READ8(mpu4_state, pia_ic4_portb_r))
	MCFG_PIA_WRITEPA_HANDLER(WRITE8(mpu4_state, pia_ic4_porta_w))
	MCFG_PIA_WRITEPB_HANDLER(WRITE8(mpu4_state, pia_ic4_portb_w))
	MCFG_PIA_CA2_HANDLER(WRITELINE(mpu4_state,pia_ic4_ca2_w))
	MCFG_PIA_CB2_HANDLER(WRITELINE(mpu4_state,pia_ic4_cb2_w))
	MCFG_PIA_IRQA_HANDLER(WRITELINE(mpu4_state,cpu0_irq))
	MCFG_PIA_IRQB_HANDLER(WRITELINE(mpu4_state,cpu0_irq))

	MCFG_DEVICE_ADD("pia_ic5", PIA6821, 0)
	MCFG_PIA_READPA_HANDLER(READ8(mpu4_state, pia_ic5_porta_r))
	MCFG_PIA_READPB_HANDLER(READ8(mpu4_state, pia_ic5_portb_r))
	MCFG_PIA_WRITEPA_HANDLER(WRITE8(mpu4_state, pia_ic5_porta_w))
	MCFG_PIA_WRITEPB_HANDLER(WRITE8(mpu4_state, pia_ic5_portb_w))
	MCFG_PIA_CA2_HANDLER(WRITELINE(mpu4_state, pia_ic5_ca2_w))
	MCFG_PIA_CB2_HANDLER(WRITELINE(mpu4_state, pia_ic5_cb2_w))
	MCFG_PIA_IRQA_HANDLER(WRITELINE(mpu4_state, cpu0_irq))
	MCFG_PIA_IRQB_HANDLER(WRITELINE(mpu4_state, cpu0_irq))

	MCFG_DEVICE_ADD("pia_ic6", PIA6821, 0)
	MCFG_PIA_WRITEPA_HANDLER(WRITE8(mpu4_state, pia_ic6_porta_w))
	MCFG_PIA_WRITEPB_HANDLER(WRITE8(mpu4_state, pia_ic6_portb_w))
	MCFG_PIA_CA2_HANDLER(WRITELINE(mpu4_state, pia_ic6_ca2_w))
	MCFG_PIA_CB2_HANDLER(WRITELINE(mpu4_state, pia_ic6_cb2_w))
	MCFG_PIA_IRQA_HANDLER(WRITELINE(mpu4_state, cpu0_irq))
	MCFG_PIA_IRQB_HANDLER(WRITELINE(mpu4_state, cpu0_irq))

	MCFG_DEVICE_ADD("pia_ic7", PIA6821, 0)
	MCFG_PIA_READPB_HANDLER(READ8(mpu4_state, pia_ic7_portb_r))
	MCFG_PIA_WRITEPA_HANDLER(WRITE8(mpu4_state, pia_ic7_porta_w))
	MCFG_PIA_WRITEPB_HANDLER(WRITE8(mpu4_state, pia_ic7_portb_w))
	MCFG_PIA_CA2_HANDLER(WRITELINE(mpu4_state, pia_ic7_ca2_w))
	MCFG_PIA_CB2_HANDLER(WRITELINE(mpu4_state, pia_ic7_cb2_w))
	MCFG_PIA_IRQA_HANDLER(WRITELINE(mpu4_state, cpu0_irq))
	MCFG_PIA_IRQB_HANDLER(WRITELINE(mpu4_state, cpu0_irq))

	MCFG_DEVICE_ADD("pia_ic8", PIA6821, 0)
	MCFG_PIA_READPA_HANDLER(READ8(mpu4_state, pia_ic8_porta_r))
	MCFG_PIA_WRITEPB_HANDLER(WRITE8(mpu4_state, pia_ic8_portb_w))
	MCFG_PIA_CA2_HANDLER(WRITELINE(mpu4_state, pia_ic8_ca2_w))
	MCFG_PIA_CB2_HANDLER(WRITELINE(mpu4_state, pia_ic8_cb2_w))
	MCFG_PIA_IRQA_HANDLER(WRITELINE(mpu4_state, cpu0_irq))
	MCFG_PIA_IRQB_HANDLER(WRITELINE(mpu4_state, cpu0_irq))
	
	MCFG_DEVICE_ADD("meters", METERS, 0)
	MCFG_METERS_NUMBER(8)

MACHINE_CONFIG_END

MACHINE_CONFIG_FRAGMENT( mpu4_common2 )
	MCFG_DEVICE_ADD("ptm_ic3ss", PTM6840, 0)
	MCFG_PTM6840_INTERNAL_CLOCK(MPU4_MASTER_CLOCK / 4)
	MCFG_PTM6840_EXTERNAL_CLOCKS(0, 0, 0)
	MCFG_PTM6840_OUT0_CB(DEVWRITELINE("ptm_ic3ss", ptm6840_device, set_c2))
	MCFG_PTM6840_OUT1_CB(DEVWRITELINE("ptm_ic3ss", ptm6840_device, set_c1))
	//MCFG_PTM6840_OUT2_CB(DEVWRITELINE("ptm_ic3ss", ptm6840_device, set_g1))
	//MCFG_PTM6840_IRQ_CB(WRITELINE(mpu4_state, cpu1_ptm_irq))

	MCFG_DEVICE_ADD("pia_ic4ss", PIA6821, 0)
	MCFG_PIA_READPB_HANDLER(READ8(mpu4_state, pia_gb_portb_r))
	MCFG_PIA_WRITEPA_HANDLER(WRITE8(mpu4_state, pia_gb_porta_w))
	MCFG_PIA_WRITEPB_HANDLER(WRITE8(mpu4_state, pia_gb_portb_w))
	MCFG_PIA_CA2_HANDLER(WRITELINE(mpu4_state, pia_gb_ca2_w))
	MCFG_PIA_CB2_HANDLER(WRITELINE(mpu4_state, pia_gb_cb2_w))
MACHINE_CONFIG_END

/* machine driver for MOD 2 board */
MACHINE_CONFIG_START( mpu4base, mpu4_state )

	MCFG_MACHINE_START_OVERRIDE(mpu4_state,mod2    )
	MCFG_MACHINE_RESET_OVERRIDE(mpu4_state,mpu4)
	MCFG_CPU_ADD("maincpu", M6809, MPU4_MASTER_CLOCK/4)
	MCFG_CPU_PROGRAM_MAP(mpu4_memmap)

	MCFG_FRAGMENT_ADD(mpu4_common)

	MCFG_SPEAKER_STANDARD_STEREO("lspeaker", "rspeaker")

	MCFG_NVRAM_ADD_0FILL("nvram")

	MCFG_DEFAULT_LAYOUT(layout_mpu4)
MACHINE_CONFIG_END


MACHINE_CONFIG_DERIVED( mod2    , mpu4base )
	MCFG_SOUND_ADD("ay8913", AY8913, MPU4_MASTER_CLOCK/4)
	MCFG_AY8910_OUTPUT_TYPE(AY8910_SINGLE_OUTPUT)
	MCFG_AY8910_RES_LOADS(820, 0, 0)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "lspeaker", 1.0)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "rspeaker", 1.0)
	MCFG_FRAGMENT_ADD(mpu4_std_6reel)
MACHINE_CONFIG_END

MACHINE_CONFIG_DERIVED( mod2_alt    , mpu4base )
	MCFG_SOUND_ADD("ay8913", AY8913, MPU4_MASTER_CLOCK/4)
	MCFG_AY8910_OUTPUT_TYPE(AY8910_SINGLE_OUTPUT)
	MCFG_AY8910_RES_LOADS(820, 0, 0)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "lspeaker", 1.0)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "rspeaker", 1.0)
	MCFG_FRAGMENT_ADD(mpu4_type2_6reel)
MACHINE_CONFIG_END



MACHINE_CONFIG_DERIVED( mod4yam, mpu4base )
	MCFG_MACHINE_START_OVERRIDE(mpu4_state,mpu4yam)

	MCFG_FRAGMENT_ADD(mpu4_std_6reel)

	MCFG_SOUND_ADD("ym2413", YM2413, MPU4_MASTER_CLOCK/4)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "lspeaker", 1.0)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "rspeaker", 1.0)
MACHINE_CONFIG_END

MACHINE_CONFIG_DERIVED( mod4oki, mpu4base )
	MCFG_MACHINE_START_OVERRIDE(mpu4_state,mpu4oki)

	MCFG_FRAGMENT_ADD(mpu4_common2)
	MCFG_FRAGMENT_ADD(mpu4_std_6reel)

	MCFG_SOUND_ADD("msm6376", OKIM6376, 128000)     //16KHz sample Can also be 85430 at 10.5KHz and 64000 at 8KHz
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "lspeaker", 1.0)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "rspeaker", 1.0)
MACHINE_CONFIG_END

MACHINE_CONFIG_DERIVED( mod4oki_alt, mpu4base )
	MCFG_MACHINE_START_OVERRIDE(mpu4_state,mpu4oki)

	MCFG_FRAGMENT_ADD(mpu4_common2)
	MCFG_FRAGMENT_ADD(mpu4_type2_6reel)

	MCFG_SOUND_ADD("msm6376", OKIM6376, 128000)     //16KHz sample Can also be 85430 at 10.5KHz and 64000 at 8KHz
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "lspeaker", 1.0)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "rspeaker", 1.0)
MACHINE_CONFIG_END

MACHINE_CONFIG_DERIVED( mod4oki_5r, mpu4base )
	MCFG_MACHINE_START_OVERRIDE(mpu4_state,mpu4oki)

	MCFG_FRAGMENT_ADD(mpu4_common2)
	MCFG_FRAGMENT_ADD(mpu4_std_5reel)

	MCFG_SOUND_ADD("msm6376", OKIM6376, 128000)     //16KHz sample Can also be 85430 at 10.5KHz and 64000 at 8KHz
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "lspeaker", 1.0)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "rspeaker", 1.0)
MACHINE_CONFIG_END

MACHINE_CONFIG_DERIVED( bwboki, mpu4base )
	MCFG_MACHINE_START_OVERRIDE(mpu4_state,mpu4bwb)
	MCFG_FRAGMENT_ADD(mpu4_common2)
	MCFG_FRAGMENT_ADD(mpu4_bwb_5reel)

	MCFG_SOUND_ADD("msm6376", OKIM6376, 128000)     //16KHz sample Can also be 85430 at 10.5KHz and 64000 at 8KHz
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "lspeaker", 1.0)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "rspeaker", 1.0)
MACHINE_CONFIG_END

MACHINE_CONFIG_DERIVED(mpu4crys, mod2     )
	MCFG_MACHINE_START_OVERRIDE(mpu4_state,mpu4cry)

	MCFG_SOUND_ADD("upd", UPD7759, UPD7759_STANDARD_CLOCK)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "lspeaker", 1.0)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "rspeaker", 1.0)
MACHINE_CONFIG_END
