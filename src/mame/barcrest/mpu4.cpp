// license:BSD-3-Clause
// copyright-holders:David Haywood, James Wallace
// thanks-to:Chris Wren, Tony Friery, MFME

/*
    mpu4.cpp - MPU4 hardware emulation

    this file contains emulation of the hardware only

*/


/*

--- Board Setup ---

The MPU4 BOARD is the driver board, originally designed to run Fruit Machines made by the Barcrest Group, but later
licensed to other firms as a general purpose unit (even some old Photo-Me booths used the unit).

This board uses a ~1.72 Mhz 6809B CPU, and a number of PIA6821 chips for multiplexing inputs and the like.

To some extent, the hardware feels like a revision of the MPU3 design, integrating into the base unit features that were
previously added through expansion ports. However, there is no backwards compatibility, and the entire memory map has been
reworked.

Like MPU3, a 6840PTM is used for internal timing, and other miscellaneous control functions, including as a crude analogue sound device
(a square wave from the PTM being fed into a 1-bit DAC circuit as the alarm sound generator). However, the main sound functionality is provided by
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
                         |   Output3 ---> 'to audio amp' (square wave)
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
           |   |                 |             CA2  OUTPUT, serial port Transmit line (Tx)
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

***********************************************************************************************************/
#include "emu.h"
#include "mpu4.h"

#include "awpvid.h"       //Fruit Machines Only

#include "machine/rescap.h"
#include "speaker.h"

#include "mpu4.lh"
#include "mpu4ext.lh"

#define LOG_IC3      (1U << 1)
#define LOG_IC8      (1U << 2)

#ifdef MAME_DEBUG
#define VERBOSE (LOG_GENERAL | LOG_IC3 | LOG_IC8)
#else
#define VERBOSE (0)
#endif
#include "logmacro.h"

#include <cmath>


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

void mpu4_state::lamp_extend_small(uint8_t data)
{
	uint8_t lamp_ext_data, column;
	column = data & 0x07;

	lamp_ext_data = 0x1f - ((data & 0xf8) >> 3);//remove the mux lines from the data

	if (m_lamp_strobe_ext_persistence == 0)
	{
		//One write to reset the drive lines, one with the data, one to clear the lines, so only the 2nd write does anything
		//TODO: PWM
		for (int i = 0; i < 5; i++)
		{
			m_lamps[(8*column) + i + 128] = BIT(lamp_ext_data, i);
		}
	}
	m_lamp_strobe_ext_persistence++;
	if ((m_lamp_strobe_ext_persistence == 3)||(m_lamp_strobe_ext[0] != column))
	{
		m_lamp_strobe_ext_persistence = 0;
		m_lamp_strobe_ext[0] = column;
	}
}

void mpu4_state::lamp_extend_large(uint8_t data, uint8_t column, bool active)
{
	m_lamp_sense = false;
	uint8_t bit7 = BIT(data, 7);
	if (bit7 != m_last_b7)
	{
		m_card_live = true;
		//depending on bit 7, we can access one of two 'blocks' of 64 lamps
		uint8_t lampbase = bit7 ? 0 : 64;
		if (data & 0x3f)
		{
			m_lamp_sense = true;
		}
		if (active)
		{
			if (m_lamp_strobe_ext[bit7] != column)
			{
				for (int i = 0; i < 8; i++)
				{
					// this includes bit 7, so you don't get a true 128 extra lamps as the last row is always 0 or 1 depending on which set of 64 we're dealing with
					m_lamps[(8*column) + i + 128 + lampbase] = BIT(data, i);
				}
				m_lamp_strobe_ext[bit7] = column;
			}
		}
		m_last_b7 = bit7;
	}
	else
	{
		m_card_live = false;
	}
}

void mpu4_state::led_write_extender(uint8_t latch, uint8_t data, uint8_t column)
{
	const uint8_t diff = (latch ^ m_last_latch) & latch;
	const uint8_t ext_strobe = (7 - column) * 8;

	data = ~data;//invert drive lines
	for (int i = 0; i < 5; i++)
	{
		if (BIT(diff, i))
		{
			for (int j = 0; j < 8; j++)
			{
				m_mpu4leds[(ext_strobe + i) | j], BIT(data, j);
			}
			m_digits[(ext_strobe + i)] = data;
		}
	}

	m_last_latch = diff;
}


void mpu4_state::update_meters()
{
	uint8_t data = ((m_mmtr_data & 0x7f) | m_remote_meter);
	switch (m_reel_mux)
	{
	case STANDARD_REEL:
		if (m_hopper_type != TUBES)
		{
			data = (data & 0x0f); //Strip reel data from meter drives, leaving active elements
		}
		break;

	case FIVE_REEL_5TO8:
		m_reel[4]->update(((data >> 4) & 0x0f));
		data = (data & 0x0f); //Strip reel data from meter drives, leaving active elements
		awp_draw_reel(machine(),"reel5", *m_reel[4]);
		break;

	case FIVE_REEL_8TO5:
		m_reel[4]->update((((data & 0x01) + ((data & 0x08) >> 2) + ((data & 0x20) >> 3) + ((data & 0x80) >> 4)) & 0x0f)) ;
		data = 0x00; //Strip all reel data from meter drives, nothing is connected
		awp_draw_reel(machine(),"reel5", *m_reel[4]);
		break;

	case FIVE_REEL_3TO6:
		m_reel[4]->update(((data >> 2) & 0x0f));
		data = 0x00; //Strip all reel data from meter drives
		awp_draw_reel(machine(),"reel5", *m_reel[4]);
		break;

	case SIX_REEL_1TO8:
		m_reel[4]->update( data       & 0x0f);
		m_reel[5]->update((data >> 4) & 0x0f);
		data = 0x00; //Strip all reel data from meter drives
		awp_draw_reel(machine(),"reel5", *m_reel[4]);
		awp_draw_reel(machine(),"reel6", *m_reel[5]);
		break;

#if 0
	case SIX_REEL_5TO8: // m_reel[4] for this case is already handled in pia_ic5_porta_w
		m_reel[4]->update(((data >> 4) & 0x0f));
		//data = 0x00; //Strip all reel data from meter drives
		data = (data & 0x0f);
		awp_draw_reel(machine(),"reel5", *m_reel[4]);
		break;
#endif

	case SEVEN_REEL:
		m_reel[0]->update((((data & 0x01) + ((data & 0x08) >> 2) + ((data & 0x20) >> 3) + ((data & 0x80) >> 4)) & 0x0f)) ;
		data = 0x00; //Strip all reel data from meter drives
		awp_draw_reel(machine(),"reel1", *m_reel[0]);
		break;

	case FLUTTERBOX: //The backbox fan assembly fits in a reel unit sized box, wired to the remote meter pin, so we can handle it here
		m_flutterbox = BIT(data, 7);
		data &= ~0x80; //Strip flutterbox data from meter drives
		break;
	}

	m_meters->update(7, data & 0x80);

	for (int meter = 0; meter < 4; meter++)
	{
		m_meters->update(meter, (data & (1 << meter)));
	}

	if (m_reel_mux == STANDARD_REEL)
	{
		for (int meter = 4; meter < 7; meter++)
		{
			m_meters->update(meter, (data & (1 << meter)));
		}
	}
}

/* called if board is reset */
MACHINE_RESET_MEMBER(mpu4_state, mpu4)
{
	m_vfd->reset();

	m_lamp_strobe    = 0;
	m_lamp_strobe2   = 0;
	m_led_strobe     = 0;
	m_pia4_porta_leds_strobe = 0;
	m_simplecard_leds_strobe = 0;
	m_mmtr_data      = 0;
	m_remote_meter   = 0;

	m_IC23GC    = 0;
	m_IC23GB    = 0;
	m_IC23GA    = 0;
	m_IC23G1    = 1;
	m_IC23G2A   = 0;
	m_IC23G2B   = 0;

	// Bwb Orlando, which uses the bwb_bank address for banking requires the default bank to be 0!
	if (m_numbanks)
		m_bank1->set_entry(m_default_to_low_bank ? 0 : m_numbanks);

	m_maincpu->reset();
}


/* 6809 IRQ handler */
void mpu4_state::cpu0_irq(int state)
{
	/* The PIA and PTM IRQ lines are all connected to a common PCB track, leading directly to the 6809 IRQ line. */
	uint8_t combined_state = m_pia3->irq_a_state() | m_pia3->irq_b_state() |
							m_pia4->irq_a_state() | m_pia4->irq_b_state() |
							m_pia5->irq_a_state() | m_pia5->irq_b_state() |
							m_pia6->irq_a_state() | m_pia6->irq_b_state() |
							m_pia7->irq_a_state() | m_pia7->irq_b_state() |
							m_pia8->irq_a_state() | m_pia8->irq_b_state() |
							m_6840ptm->irq_state();

	if (m_link7b_connected) //7B = IRQ, 7A = FIRQ, both = NMI
	{
		if (!m_link7a_connected)
		{
			m_maincpu->set_input_line(M6809_IRQ_LINE, combined_state ? ASSERT_LINE : CLEAR_LINE);
			LOG("6809 IRQ %d \n", combined_state);
		}
		else
		{
			m_maincpu->set_input_line(INPUT_LINE_NMI, combined_state ? ASSERT_LINE : CLEAR_LINE);
			LOG("6809 NMI %d \n", combined_state);
		}
	}
	else
	{
		m_maincpu->set_input_line(M6809_FIRQ_LINE, combined_state ? ASSERT_LINE : CLEAR_LINE);
		LOG("6809 FIRQ %d \n", combined_state);
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
void mpu4_state::bankswitch_w(uint8_t data)
{
	m_pageval = (data & 0x03);
	m_bank1->set_entry((m_pageval + (m_pageset ? 4 : 0)) & m_numbanks);
}


uint8_t mpu4_state::bankswitch_r()
{
	return m_bank1->entry();
}


void mpu4_state::bankset_w(uint8_t data)
{
	m_pageval = (data - 2);//writes 2 and 3, to represent 0 and 1 - a hangover from the half page design?
	m_bank1->set_entry((m_pageval + (m_pageset ? 4 : 0)) & m_numbanks);
}


/* IC2 6840 PTM handler */
void mpu4_state::ic2_o1_callback(int state)
{
	m_6840ptm->set_c2(state);    /* copy output value to IC2 c2
	this output is the clock for timer2 */
	/* 1200Hz System interrupt timer */
}


void mpu4_state::ic2_o2_callback(int state)
{
	m_pia3->ca1_w(state);    /* copy output value to IC3 ca1 */
	/* the output from timer2 is the input clock for timer3 */
	/* miscellaneous interrupts generated here */
	m_6840ptm->set_c3(state);
}


void mpu4_state::ic2_o3_callback(int state)
{
	/* the output from timer3 is used as a square wave for the alarm output
	and as an external clock source for timer 1! */
	/* also runs lamp fade */
	m_alarmdac->write(state);
	m_6840ptm->set_c1(state);
}

/* 6821 PIA handlers */
/* IC3, lamp data lines + alpha numeric display */
void mpu4_state::pia_ic3_porta_w(uint8_t data)
{
	LOGMASKED(LOG_IC3, "%s: IC3 PIA Port A Set to %2x (lamp strobes 1 - 9)\n", machine().describe_context(), data);

	if(m_ic23_active)
	{
		if (m_lamp_strobe != m_input_strobe)
		{
			// Because of the nature of the lamping circuit, there is an element of persistance
			// As a consequence, the lamp column data can change before the input strobe without
			// causing the relevant lamps to black out.

			if (m_overcurrent_detect)  m_overcurrent = true;

			if (m_undercurrent_detect) m_undercurrent = true;

			for (int i = 0; i < 8; i++)
			{
				m_lamps[(8*m_input_strobe) + i] = BIT(data, i);
			}
			m_lamp_strobe = m_input_strobe;
		}
	}
}

void mpu4_state::pia_ic3_portb_w(uint8_t data)
{
	LOGMASKED(LOG_IC3, "%s: IC3 PIA Port B Set to %2x  (lamp strobes 10 - 17)\n", machine().describe_context(), data);

	if (m_ic23_active)
	{
		if (m_lamp_strobe2 != m_input_strobe)
		{
			if (m_overcurrent_detect)  m_overcurrent = true;

			if (m_undercurrent_detect) m_undercurrent = true;

			for (int i = 0; i < 8; i++)
			{
				m_lamps[(8*m_input_strobe) + i + 64] = BIT(data, i);
			}
			m_lamp_strobe2 = m_input_strobe;
		}

	}
}

void mpu4_state::pia_ic3_ca2_w(int state)
{
	LOGMASKED(LOG_IC3, "%s: IC3 PIA Write CA2 (alpha data), %02X\n", machine().describe_context(), state);
	m_vfd->data(state);
}


void mpu4_state::pia_ic3_cb2_w(int state)
{
	LOGMASKED(LOG_IC3, "%s: IC3 PIA Write CB (alpha reset), %02X\n", machine().describe_context(), state);
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
				if (m_IC23GA) m_input_strobe |= 0x01;
				else          m_input_strobe &= ~0x01;

				if (m_IC23GB) m_input_strobe |= 0x02;
				else          m_input_strobe &= ~0x02;

				if (m_IC23GC) m_input_strobe |= 0x04;
				else          m_input_strobe &= ~0x04;
			}
		}
	}
	else if (m_IC23G2A || m_IC23G2B)
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
void mpu4_state::ic24_output(uint8_t data)
{
	m_IC23G2A = data;
	ic23_update();
}


void mpu4_state::ic24_setup()
{
	if (m_IC23GA)
	{
		double duration = TIME_OF_74LS123((220*1000),(0.1*0.000001));
		m_ic23_active = true;
		ic24_output(0);
		m_ic24_timer->adjust(attotime::from_double(duration));
	}
}


TIMER_CALLBACK_MEMBER(mpu4_state::update_ic24)
{
	m_ic23_active=false;
	ic24_output(1);
}


void mpu4_state::dataport_rxd(int state)
{
	m_pia4->cb1_w(state);
	m_serial_output = state;
	LOGMASKED(LOG_IC3, "Dataport RX %x\n", state);
}

/* IC4, 7 seg leds, 50Hz timer reel sensors, current sensors */
void mpu4_state::pia_ic4_porta_w(uint8_t data)
{
	if(m_ic23_active)
	{
		if (m_use_pia4_porta_leds)
		{
			if (m_pia4_porta_leds_strobe != m_input_strobe)
			{
				for (int i=0; i<8; i++)
				{
					m_mpu4leds[(((7 - m_input_strobe) | m_pia4_porta_leds_base) << 3) | i] = BIT(data, i);
				}
				m_digits[(7 - m_input_strobe) | m_pia4_porta_leds_base] = data;
			}
			m_pia4_porta_leds_strobe = m_input_strobe;
		}
	}
}

void mpu4_state::pia_ic4_portb_w(uint8_t data)
{
	if (m_reel_mux)
	{
		/* A write here connects one reel (and only one)
		to the optic test circuit. This allows 8 reels
		to be supported instead of 4. */
		if (m_reel_mux == SEVEN_REEL)
		{
			m_active_reel = reel_mux_table7[(data >> 4) & 0x07];
		}
		else
		{
			m_active_reel = reel_mux_table[(data >> 4) & 0x07];
		}
	}
}

uint8_t mpu4_state::pia_ic4_portb_r()
{
	m_ic4_input_b = 0x00;

	if (m_serial_output) m_ic4_input_b |= 0x80;

	if (!m_reel_mux)
	{
		if (m_optic_pattern & 0x01) m_ic4_input_b |= 0x40; /* reel A tab */

		if (m_optic_pattern & 0x02) m_ic4_input_b |= 0x20; /* reel B tab */

		if (m_optic_pattern & 0x04) m_ic4_input_b |= 0x10; /* reel C tab */

		if (m_optic_pattern & 0x08) m_ic4_input_b |= 0x08; /* reel D tab */
	}
	else
	{
		if (m_optic_pattern & (1<<m_active_reel)) m_ic4_input_b |=  0x08;
	}

	if (m_low_volt_detect)
	{
		if (m_signal_50hz) m_ic4_input_b |= 0x04;
	}

	if (m_overcurrent) m_ic4_input_b |= 0x02;

	if (m_undercurrent) m_ic4_input_b |= 0x01;

	LOGMASKED(LOG_IC3, "%s: IC4 PIA Read of Port B %x\n", machine().describe_context(), m_ic4_input_b);
	return m_ic4_input_b;
}


void mpu4_state::pia_ic4_ca2_w(int state)
{
	LOGMASKED(LOG_IC3, "%s: IC4 PIA Write CA (input MUX strobe /LED B), %02X\n", machine().describe_context(), state);

	m_IC23GB = state;
	ic23_update();
}

void mpu4_state::pia_ic4_cb2_w(int state)
{
	LOGMASKED(LOG_IC3, "%s: IC4 PIA Write CB (Reel optic flag), %02X\n", machine().describe_context(), state);
	m_reel_flag=state;
}

/* IC5, AUX ports, coin lockouts and AY sound chip select (MODs below 4 only) */
uint8_t mpu4_state::pia_ic5_porta_r()
{
	if (m_lamp_extender == LARGE_CARD_A)
	{
		if (m_overcurrent_detect)
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
	}
	if (m_hopper_type == HOPPER_NONDUART_A)
	{
		if (m_hopper1->line_r() && m_hopper1_opto)
		{
			m_aux1_input |= 0x04;
		}
		else
		{
			m_aux1_input &= ~0x04;
		}
	}
	else if (m_hopper_type == HOPPER_TWIN_HOPPER)
	{
		if (m_hopper1->line_r())
		{
			m_aux1_input |= 0x08;
		}
		else
		{
			m_aux1_input &= ~0x08;
		}

		if (m_hopper2->line_r())
		{
			m_aux1_input |= 0x04;
		}
		else
		{
			m_aux1_input &= ~0x04;
		}
	}

	LOG("%s: IC5 PIA Read of Port A (AUX1)\n", machine().describe_context());


	uint8_t tempinput = m_aux1_port->read() | m_aux1_input;
	return tempinput;
}


void mpu4_state::pia_ic5_porta_w(uint8_t data)
{
	if (m_hopper_type == HOPPER_NONDUART_A)
	{
		m_hopper1_opto = (data & 0x10);
	}

	switch (m_lamp_extender)
	{
	case NO_EXTENDER:
		if (m_led_extender == CARD_B)
		{
			led_write_extender(data & 0x1f, m_pia4->a_output(), m_input_strobe);
		}
		else if ((m_led_extender != CARD_A) && (m_led_extender != NO_EXTENDER))
		{
			for (int i = 0; i < 8; i++)
			{
				m_mpu4leds[((m_input_strobe | 8) << 3) | i] = BIT(data, i);
			}
		}
		break;

	case SMALL_CARD:
		if(m_ic23_active)
		{
			lamp_extend_small(data);
		}
		break;

	case LARGE_CARD_A:
		lamp_extend_large(data, m_input_strobe, m_ic23_active);
		break;

	case LARGE_CARD_B:
		lamp_extend_large(data, m_input_strobe, m_ic23_active);
#if 0
		if ((m_ic23_active) && m_card_live)
		{
			for (int i = 0; i < 8; i++)
			{
				m_mpu4leds[(m_last_b7 << 6) | (m_input_strobe << 3) | i] = BIT(~data, i);
			}
			m_digits[(m_last_b7 << 3) | m_input_strobe] = ~data;
		}
#endif
		break;

	case LARGE_CARD_C:
		lamp_extend_large(data, m_input_strobe, m_ic23_active);
		break;
	}


	if (m_reel_mux == SIX_REEL_5TO8)
	{
		m_reel[4]->update( data      &0x0f);
		m_reel[5]->update((data >> 4)&0x0f);
		awp_draw_reel(machine(),"reel5", *m_reel[4]);
		awp_draw_reel(machine(),"reel6", *m_reel[5]);
	}
	else if (m_reel_mux == SEVEN_REEL)
	{
		m_reel[1]->update( data      &0x0f);
		m_reel[2]->update((data >> 4)&0x0f);
		awp_draw_reel(machine(),"reel2", *m_reel[1]);
		awp_draw_reel(machine(),"reel3", *m_reel[2]);
	}
}


void mpu4_state::pia_ic5_portb_w(uint8_t data)
{
	if (m_hopper_type == HOPPER_NONDUART_B)
	{
		m_hopper1->motor_w(data & 0x01);
		m_hopper1_opto = (data & 0x08);
	}
	if (m_led_extender == CARD_A)
	{
		led_write_extender(data & 0x07, m_pia4->a_output(), m_input_strobe);
	}

	if (m_use_simplecard_leds)
	{
		if(m_simplecard_leds_strobe != m_input_strobe)
		{
			for(int i=0; i<8; i++)
			{
				m_mpu4leds[( ( (7 - m_input_strobe) | m_simplecard_leds_base) << 3) | i] = BIT(m_pia4->a_output(), i);
			}
			m_digits[(7 - m_input_strobe) | m_simplecard_leds_base] = m_pia4->a_output();
		}
		m_simplecard_leds_strobe = m_input_strobe;
	}
}

uint8_t mpu4_state::pia_ic5_portb_r()
{
	if (m_hopper_type == HOPPER_NONDUART_B)
	{
		if (m_hopper1->line_r() && m_hopper1_opto)
		{
			m_aux2_input |= 0x08;
		}
		else
		{
			m_aux2_input &= ~0x08;
		}
	}

	LOG("%s: IC5 PIA Read of Port B (coin input AUX2)\n", machine().describe_context());
	if (m_use_coinlocks)
	{
		// why are these being set in a read, not when the outputs are written?
		// maybe should be done as an output 'port' as differs between games?
		machine().bookkeeping().coin_lockout_w(0, (m_pia5->b_output() & 0x01));
		machine().bookkeeping().coin_lockout_w(1, (m_pia5->b_output() & 0x02));
		machine().bookkeeping().coin_lockout_w(2, (m_pia5->b_output() & 0x04));
		machine().bookkeeping().coin_lockout_w(3, (m_pia5->b_output() & 0x08));
	}

	uint8_t tempinput = m_aux2_port->read() | m_aux2_input;
	return tempinput;
}


/* ---------------------------------------
   AY Chip sound function selection -
   ---------------------------------------
The databus of the AY sound chip is connected to IC6 Port A.
Data is read from/written to the AY chip through this port.

If this sounds familiar, Amstrad did something very similar with their home computers.

The PSG function, defined by the BC1,BC2 and BDIR signals, is controlled by CA2 and CB2 of IC6.

The chipselect for the AY chip itself, however, is CB2 on IC5, so unless this goes live, things are likely to fail

PSG function selection:
-----------------------
CSEL = IC5 CB2
BDIR = IC6 CB2 and BC1 = IC6 CA2

Pin            | PSG Function
BDIR BC1       |
0    0         | Inactive
0    1         | Read from selected PSG register. When function is set, the PSG will make the register data available to Port A.
1    0         | Write to selected PSG register. When set, the PSG will take the data at Port A and write it into the selected PSG register.
1    1         | Select PSG register. When set, the PSG will take the data at Port A and select a register.
*/

/* PSG function selected */
void mpu4_state::update_ay()
{
	if (!m_ay8913) return;

	if (!m_pia5->cb2_output())
	{
		switch (m_ay8913_address)
		{
		case 0x00:
			/* Inactive */
			break;

		case 0x01:
			/* CA2 = 1 CB2 = 0? : Read from selected PSG register and make the register data available to Port A */
			LOG("AY8913 address = %d \n", m_pia6->a_output()&0x0f);
			break;

		case 0x02:
			/* CA2 = 0 CB2 = 1? : Write to selected PSG register and write data to Port A */
			m_ay8913->data_w(m_pia6->a_output());
			LOG("AY Chip Write \n");
			break;

		case 0x03:
			/* CA2 = 1 CB2 = 1? : The register will now be selected and the user can read from or write to it.
			The register will remain selected until another is chosen.*/
			m_ay8913->address_w(m_pia6->a_output());
			LOG("AY Chip Select \n");
			break;

		default:
			LOG("AY Chip error \n");
			break;
		}
	}
}


void mpu4_state::pia_ic5_cb2_w(int state)
{
	update_ay();
}


/* IC6, Reel A and B and AY registers (MODs below 4 only) */
void mpu4_state::pia_ic6_portb_w(uint8_t data)
{
	LOG("%s: IC6 PIA Port B Set to %2x (Reel A and B)\n", machine().describe_context(), data);

	if (m_reel_mux == SEVEN_REEL)
	{
		m_reel[3]->update( data      &0x0f);
		m_reel[4]->update((data >> 4)&0x0f);
		awp_draw_reel(machine(),"reel4", *m_reel[3]);
		awp_draw_reel(machine(),"reel5", *m_reel[4]);
	}
	else if (m_reels)
	{
		m_reel[0]->update( data      &0x0f);
		m_reel[1]->update((data >> 4)&0x0f);
		awp_draw_reel(machine(),"reel1", *m_reel[0]);
		awp_draw_reel(machine(),"reel2", *m_reel[1]);
	}
}


void mpu4_state::pia_ic6_porta_w(uint8_t data)
{
	LOG("%s: IC6 PIA Write A %2x\n", machine().describe_context(), data);
	if (m_ay8913.found())
	{
		m_ay_data = data;
		update_ay();
	}
}


void mpu4_state::pia_ic6_ca2_w(int state)
{
	LOG("%s: IC6 PIA write CA2 %2x (AY8913 BC1)\n", machine().describe_context(), state);
	if (m_ay8913.found())
	{
		if ( state ) m_ay8913_address |=  0x01;
		else         m_ay8913_address &= ~0x01;
		update_ay();
	}
}


void mpu4_state::pia_ic6_cb2_w(int state)
{
	LOG("%s: IC6 PIA write CB2 %2x (AY8913 BCDIR)\n", machine().describe_context(), state);
	if (m_ay8913.found())
	{
		if ( state ) m_ay8913_address |=  0x02;
		else         m_ay8913_address &= ~0x02;
		update_ay(); // using m_pia5 here allows m4fourmr to have sound
	}
}


/* IC7 Reel C and D, mechanical meters/Reel E and F, input strobe bit A */
void mpu4_state::pia_ic7_porta_w(uint8_t data)
{
	LOG("%s: IC7 PIA Port A Set to %2x (Reel C and D)\n", machine().describe_context(), data);
	if (m_reel_mux == SEVEN_REEL)
	{
		m_reel[5]->update( data      &0x0f);
		m_reel[6]->update((data >> 4)&0x0f);
		awp_draw_reel(machine(),"reel6", *m_reel[5]);
		awp_draw_reel(machine(),"reel7", *m_reel[6]);
	}
	else if (m_reels)
	{
		m_reel[2]->update( data      &0x0f);
		m_reel[3]->update((data >> 4)&0x0f);
		awp_draw_reel(machine(),"reel3", *m_reel[2]);
		awp_draw_reel(machine(),"reel4", *m_reel[3]);
	}
}

void mpu4_state::pia_ic7_portb_w(uint8_t data)
{
	if (m_hopper_type == HOPPER_DUART_A)
	{
		m_hopper1->motor_w(data & 0x10);
		//opto line is DUART op BIT 4 (MR, channel B)
	}
	else if (m_hopper_type == HOPPER_NONDUART_A)
	{
		m_hopper1->motor_w(data & 0x20);
	}
	else if (m_hopper_type == HOPPER_TWIN_HOPPER)
	{
		m_hopper1->motor_w(data & 0x20);
		m_hopper2->motor_w(data & 0x40);
	}

	m_mmtr_data = data;
}

uint8_t mpu4_state::pia_ic7_portb_r()
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

	//This may be overkill, but the meter sensing is VERY picky.

	uint8_t combined_meter = m_meters->get_activity(0) | m_meters->get_activity(1) |
							m_meters->get_activity(2) | m_meters->get_activity(3) |
							m_meters->get_activity(4) | m_meters->get_activity(5) |
							m_meters->get_activity(6) | m_meters->get_activity(7);

	if(combined_meter)
	{
		return 0x80;
	}
	else
	{
		return 0x00;
	}
}

void mpu4_state::pia_ic7_ca2_w(int state)
{
	LOG("%s: IC7 PIA write CA2 %2x (input strobe bit 0 / LED A)\n", machine().describe_context(), state);

	m_IC23GA = state;
	ic24_setup();
	ic23_update();
}

void mpu4_state::pia_ic7_cb2_w(int state)
{
	m_remote_meter = state?0x80:0x00;
}


/* IC8, Inputs, TRIACS, alpha clock */
uint8_t mpu4_state::pia_ic8_porta_r()
{
	LOGMASKED(LOG_IC8, "%s: IC8 PIA Read of Port A (MUX input data)\n", machine().describe_context());
	/* The orange inputs are polled twice as often as the black ones, for reasons of efficiency.
	   This is achieved via connecting every input line to an AND gate, thus allowing two strobes
	   to represent each orange input bank (strobes are active low). */
	m_pia5->cb1_w(m_aux2_port->read() & 0x80);

	return (m_port_mux[m_input_strobe])->read();
}


void mpu4_state::pia_ic8_portb_w(uint8_t data)
{
	if (m_hopper_type == HOPPER_DUART_B)
	{
		m_hopper1->motor_w(data & 0x01);
		m_hopper1_opto =  (data & 0x04);
		data &= ~0x05; //remove Triacs from use
	}
	else if (m_hopper_type == HOPPER_DUART_C)
	{
		// Dual DUART hoppers share an opto line for some reason
		m_hopper1->motor_w(data & 0x01);
		m_hopper1_opto =  (data & 0x04);
		m_hopper2->motor_w(data & 0x02);
		m_hopper2_opto =  (data & 0x04);
		data &= ~0x07; //remove Triacs from use
	}
	LOGMASKED(LOG_IC8, "%s: IC8 PIA Port B Set to %2x (OUTPUT PORT, TRIACS)\n", machine().describe_context(), data);
	for (uint8_t i = 0; i < 8; i++)
	{
		m_triacs[i] = BIT(data, i);
	}
}

void mpu4_state::pia_ic8_ca2_w(int state)
{
	LOGMASKED(LOG_IC8, "%s: IC8 PIA write CA2 (input_strobe bit 2 / LED C) %02X\n", machine().describe_context(), state & 0xff);

	m_IC23GC = state;
	ic23_update();
}


void mpu4_state::pia_ic8_cb2_w(int state)
{
	LOGMASKED(LOG_IC8, "%s: IC8 PIA write CB2 (alpha clock) %02X\n", machine().describe_context(), state & 0xff);

	// DM Data pin B

	m_vfd->sclk(!state);
}

void mpu4_state::pia_gb_cb2_w(int state)
{
	// Some BWB games use this to drive the bankswitching
	// should the regular bankswitch still work in these cases?
	if (m_bwb_bank)
	{
		m_pageval = state;
		m_bank1->set_entry((m_pageval + (m_pageset ? 4 : 0)) & m_numbanks);
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
	PORT_CONFNAME( 0xe0, 0x00, "Stake Key" )
	PORT_CONFSETTING(    0x00, "Not fitted / 5p"  )
	PORT_CONFSETTING(    0x20, "10p" )
	PORT_CONFSETTING(    0x40, "20p" )
	PORT_CONFSETTING(    0x60, "25p" )
	PORT_CONFSETTING(    0x80, "30p" )
	PORT_CONFSETTING(    0xa0, "40p" )
	PORT_CONFSETTING(    0xc0, "50p" )
	PORT_CONFSETTING(    0xe0, "1 GBP" )

	PORT_START("ORANGE2")
	PORT_CONFNAME( 0x0f, 0x00, "Jackpot / Prize Key" )
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
	PORT_CONFSETTING(    0x0a, "25 GBP"  )
	PORT_CONFSETTING(    0x0b, "25 GBP (Licensed Betting Office Profile)"  )
	PORT_CONFSETTING(    0x0c, "35 GBP"  )
	PORT_CONFSETTING(    0x0d, "70 GBP"  )
	PORT_CONFSETTING(    0x0e, "Reserved"  )
	PORT_CONFSETTING(    0x0f, "Reserved"  )

	PORT_CONFNAME( 0xf0, 0x00, "Percentage Key" )
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
	PORT_CONFSETTING(    0xa0, "88" )
	PORT_CONFSETTING(    0xb0, "90" )
	PORT_CONFSETTING(    0xc0, "92" )
	PORT_CONFSETTING(    0xd0, "94" )
	PORT_CONFSETTING(    0xe0, "96" )
	PORT_CONFSETTING(    0xf0, "98" )

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
	PORT_DIPNAME( 0xf0, 0x00, "Target Percentage (if key not fitted)" )PORT_DIPLOCATION("DIL1:05,06,07,08")
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
	PORT_DIPSETTING(    0xa0, "88" )
	PORT_DIPSETTING(    0xb0, "90" )
	PORT_DIPSETTING(    0xc0, "92" )
	PORT_DIPSETTING(    0xd0, "94" )
	PORT_DIPSETTING(    0xe0, "96" )
	PORT_DIPSETTING(    0xf0, "98" )

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
	PORT_DIPNAME( 0x08, 0x00, "Out of Credit Display Inhibit" ) PORT_DIPLOCATION("DIL2:04")  // many games need this 'OFF' to display attract cycles
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
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_OTHER) PORT_NAME("A1_0")
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_OTHER) PORT_NAME("A1_1")
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_OTHER) PORT_NAME("A1_2")
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_OTHER) PORT_NAME("A1_3")
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_OTHER) PORT_NAME("A1_4")
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_OTHER) PORT_NAME("A1_5")
	PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_OTHER) PORT_NAME("A1_6")
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_OTHER) PORT_NAME("A1_7")

	PORT_START("AUX2")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_CUSTOM) //Lockouts, in same order as below
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_CUSTOM)
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_CUSTOM)
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_CUSTOM)
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_COIN1)
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_COIN2)
	PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_COIN3)
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_COIN4)
INPUT_PORTS_END

INPUT_PORTS_START( mpu4_dutch )
	PORT_INCLUDE( mpu4 )

	PORT_MODIFY("ORANGE1")
	PORT_DIPNAME( 0x01, 0x00, DEF_STR( Unused ) ) PORT_DIPLOCATION("ORANGE1:01")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x01, DEF_STR( On  ) )
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_OTHER) // avoid REFILL NEEDED
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_OTHER) // avoid REFILL NEEDED
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_OTHER) // avoid NO TUBES
	PORT_DIPNAME( 0x10, 0x00, DEF_STR( Unused ) ) PORT_DIPLOCATION("ORANGE1:05")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x10, DEF_STR( On  ) )
	PORT_DIPNAME( 0x20, 0x00, DEF_STR( Unused ) ) PORT_DIPLOCATION("ORANGE1:06")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x20, DEF_STR( On  ) )
	PORT_DIPNAME( 0x40, 0x00, DEF_STR( Unused ) ) PORT_DIPLOCATION("ORANGE1:07")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x40, DEF_STR( On  ) )
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_INTERLOCK) PORT_NAME("Unknown Door")  PORT_CODE(KEYCODE_T) PORT_TOGGLE

	PORT_MODIFY("ORANGE2")
	PORT_DIPNAME( 0x01, 0x00, DEF_STR( Unused ) ) PORT_DIPLOCATION("ORANGE2:01")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x01, DEF_STR( On  ) )
	PORT_DIPNAME( 0x02, 0x00, DEF_STR( Unused ) ) PORT_DIPLOCATION("ORANGE2:02")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x02, DEF_STR( On  ) )
	PORT_DIPNAME( 0x04, 0x00, DEF_STR( Unused ) ) PORT_DIPLOCATION("ORANGE2:03")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x04, DEF_STR( On  ) )
	PORT_DIPNAME( 0x08, 0x00, DEF_STR( Unused ) ) PORT_DIPLOCATION("ORANGE2:04")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x08, DEF_STR( On  ) )
	PORT_DIPNAME( 0x10, 0x00, DEF_STR( Unused ) ) PORT_DIPLOCATION("ORANGE2:05")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x10, DEF_STR( On  ) )
	PORT_DIPNAME( 0x20, 0x00, DEF_STR( Unused ) ) PORT_DIPLOCATION("ORANGE2:06")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x20, DEF_STR( On  ) )
	PORT_DIPNAME( 0x40, 0x00, DEF_STR( Unused ) ) PORT_DIPLOCATION("ORANGE2:07")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x40, DEF_STR( On  ) )
	PORT_DIPNAME( 0x80, 0x00, DEF_STR( Unused ) ) PORT_DIPLOCATION("ORANGE2:08")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x80, DEF_STR( On  ) )

	PORT_MODIFY("DIL1")
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
	PORT_DIPNAME( 0x10, 0x00, DEF_STR( Unused ) ) PORT_DIPLOCATION("DIL1:05")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x10, DEF_STR( On  ) )
	PORT_DIPNAME( 0x20, 0x00, DEF_STR( Unused ) ) PORT_DIPLOCATION("DIL1:06")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x20, DEF_STR( On  ) )
	PORT_DIPNAME( 0x40, 0x00, DEF_STR( Unused ) ) PORT_DIPLOCATION("DIL1:07")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x40, DEF_STR( On  ) )
	PORT_DIPNAME( 0x80, 0x00, DEF_STR( Unused ) ) PORT_DIPLOCATION("DIL1:08")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x80, DEF_STR( On  ) )

INPUT_PORTS_END

INPUT_PORTS_START( mpu4_dutch_invcoin )
	PORT_INCLUDE( mpu4_dutch )

	PORT_MODIFY("AUX2")
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_COIN1)
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_COIN2)
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_COIN3)
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_COIN4)
INPUT_PORTS_END

INPUT_PORTS_START( mpu4_dutch_alt_invcoin )
	PORT_INCLUDE( mpu4_dutch_invcoin )

	PORT_MODIFY("AUX2")
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_OTHER) // needed for several sets to boot but gives coin jam error if pressed
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_OTHER)
INPUT_PORTS_END

INPUT_PORTS_START( mpu4_impcoin )
	PORT_INCLUDE( mpu4 )

	PORT_MODIFY("AUX2")
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_COIN1) PORT_IMPULSE(4)
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_COIN2) PORT_IMPULSE(4)
	PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_COIN3) PORT_IMPULSE(4)
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_COIN4) PORT_IMPULSE(4)
INPUT_PORTS_END

INPUT_PORTS_START( mpu4_invcoin )
	PORT_INCLUDE( mpu4 )

	PORT_MODIFY("AUX2")
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_COIN1)
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_COIN2)
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_COIN3)
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_COIN4)
INPUT_PORTS_END

INPUT_PORTS_START( mpu4_invimpcoin )
	PORT_INCLUDE( mpu4 )

	PORT_MODIFY("AUX2")
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_COIN1) PORT_IMPULSE(5)
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_COIN2) PORT_IMPULSE(5)
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_COIN3) PORT_IMPULSE(5)
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_COIN4) PORT_IMPULSE(5)
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

INPUT_PORTS_START( mpu420p )
	PORT_INCLUDE( mpu4 )

	PORT_MODIFY("ORANGE1")
	PORT_CONFNAME( 0xe0, 0x40, "Stake Key" )
	PORT_CONFSETTING(    0x00, "Not fitted / 5p" )
	PORT_CONFSETTING(    0x20, "10p" )
	PORT_CONFSETTING(    0x40, "20p" )
	PORT_CONFSETTING(    0x60, "25p" )
	PORT_CONFSETTING(    0x80, "30p" )
	PORT_CONFSETTING(    0xa0, "40p" )
	PORT_CONFSETTING(    0xc0, "50p" )
	PORT_CONFSETTING(    0xe0, "1 GBP" )
INPUT_PORTS_END


INPUT_PORTS_START( mpu4jackpot8tkn )
	PORT_INCLUDE( mpu4 )

	PORT_MODIFY("ORANGE2")
	PORT_CONFNAME( 0x0f, 0x06, "Jackpot / Prize Key" )
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
	PORT_CONFSETTING(    0x0a, "25 GBP"  )
	PORT_CONFSETTING(    0x0b, "25 GBP (Licensed Betting Office Profile)"  )
	PORT_CONFSETTING(    0x0c, "35 GBP"  )
	PORT_CONFSETTING(    0x0d, "70 GBP"  )
	PORT_CONFSETTING(    0x0e, "Reserved"  )
	PORT_CONFSETTING(    0x0f, "Reserved"  )
INPUT_PORTS_END


INPUT_PORTS_START( mpu4jackpot8tkn20p )
	PORT_INCLUDE( mpu4jackpot8tkn )

	PORT_MODIFY("ORANGE1")
	PORT_CONFNAME( 0xe0, 0x40, "Stake Key" )
	PORT_CONFSETTING(    0x00, "Not fitted / 5p" )
	PORT_CONFSETTING(    0x20, "10p" )
	PORT_CONFSETTING(    0x40, "20p" )
	PORT_CONFSETTING(    0x60, "25p" )
	PORT_CONFSETTING(    0x80, "30p" )
	PORT_CONFSETTING(    0xa0, "40p" )
	PORT_CONFSETTING(    0xc0, "50p" )
	PORT_CONFSETTING(    0xe0, "1 GBP" )
INPUT_PORTS_END

INPUT_PORTS_START( mpu4jackpot8tkn20p90pc )

	PORT_INCLUDE( mpu4jackpot8tkn20p )

	PORT_MODIFY("ORANGE2")
	PORT_CONFNAME( 0xf0, 0xb0, "Percentage Key" )
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
	PORT_CONFSETTING(    0xa0, "88" )
	PORT_CONFSETTING(    0xb0, "90" )
	PORT_CONFSETTING(    0xc0, "92" )
	PORT_CONFSETTING(    0xd0, "94" )
	PORT_CONFSETTING(    0xe0, "96" )
	PORT_CONFSETTING(    0xf0, "98" )
INPUT_PORTS_END

INPUT_PORTS_START( mpu4jackpot8per )
	PORT_INCLUDE( mpu4 )

	PORT_MODIFY("ORANGE2")
	PORT_CONFNAME( 0x0f, 0x06, "Jackpot / Prize Key" )
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
	PORT_CONFSETTING(    0x0a, "25 GBP"  )
	PORT_CONFSETTING(    0x0b, "25 GBP (Licensed Betting Office Profile)"  )
	PORT_CONFSETTING(    0x0c, "35 GBP"  )
	PORT_CONFSETTING(    0x0d, "70 GBP"  )
	PORT_CONFSETTING(    0x0e, "Reserved"  )
	PORT_CONFSETTING(    0x0f, "Reserved"  )

	PORT_CONFNAME( 0xf0, 0x10, "Percentage Key" )
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
	PORT_CONFSETTING(    0xa0, "88" )
	PORT_CONFSETTING(    0xb0, "90" )
	PORT_CONFSETTING(    0xc0, "92" )
	PORT_CONFSETTING(    0xd0, "94" )
	PORT_CONFSETTING(    0xe0, "96" )
	PORT_CONFSETTING(    0xf0, "98" )
INPUT_PORTS_END

INPUT_PORTS_START( mpu4_70pc )
	PORT_INCLUDE( mpu4 )

	PORT_MODIFY("ORANGE2")
	PORT_CONFNAME( 0xf0, 0x10, "Percentage Key" )
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
	PORT_CONFSETTING(    0xa0, "88" )
	PORT_CONFSETTING(    0xb0, "90" )
	PORT_CONFSETTING(    0xc0, "92" )
	PORT_CONFSETTING(    0xd0, "94" )
	PORT_CONFSETTING(    0xe0, "96" )
	PORT_CONFSETTING(    0xf0, "98" )
INPUT_PORTS_END

INPUT_PORTS_START( mpu4jackpot10 )
	PORT_INCLUDE( mpu4 )

	PORT_MODIFY("ORANGE2")
	PORT_CONFNAME( 0x0f, 0x07, "Jackpot / Prize Key" )
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
	PORT_CONFSETTING(    0x0a, "25 GBP"  )
	PORT_CONFSETTING(    0x0b, "25 GBP (Licensed Betting Office Profile)"  )
	PORT_CONFSETTING(    0x0c, "35 GBP"  )
	PORT_CONFSETTING(    0x0d, "70 GBP"  )
	PORT_CONFSETTING(    0x0e, "Reserved"  )
	PORT_CONFSETTING(    0x0f, "Reserved"  )
INPUT_PORTS_END

INPUT_PORTS_START( mpu4jackpot10_20p )
	PORT_INCLUDE( mpu4jackpot10 )

	PORT_MODIFY("ORANGE1")
	PORT_CONFNAME( 0xe0, 0x40, "Stake Key" )
	PORT_CONFSETTING(    0x00, "Not fitted / 5p" )
	PORT_CONFSETTING(    0x20, "10p" )
	PORT_CONFSETTING(    0x40, "20p" )
	PORT_CONFSETTING(    0x60, "25p" )
	PORT_CONFSETTING(    0x80, "30p" )
	PORT_CONFSETTING(    0xa0, "40p" )
	PORT_CONFSETTING(    0xc0, "50p" )
	PORT_CONFSETTING(    0xe0, "1 GBP" )
INPUT_PORTS_END

INPUT_PORTS_START( grtecp )
	PORT_START("ORANGE1")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_OTHER) PORT_NAME("00")//  20p level
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_OTHER) PORT_NAME("01")// 100p level
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_OTHER) PORT_NAME("02")// Token 1 level
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_OTHER) PORT_NAME("03")// Token 2 level
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_OTHER) PORT_NAME("04")
	PORT_CONFNAME( 0xe0, 0x00, "Stake Key" )
	PORT_CONFSETTING(    0x00, "Not fitted / 5p"  )
	PORT_CONFSETTING(    0x20, "10p" )
	PORT_CONFSETTING(    0x40, "20p" )
	PORT_CONFSETTING(    0x60, "25p" )
	PORT_CONFSETTING(    0x80, "30p" )
	PORT_CONFSETTING(    0xa0, "40p" )
	PORT_CONFSETTING(    0xc0, "50p" )
	PORT_CONFSETTING(    0xe0, "1 GBP" )

	PORT_START("ORANGE2")
	PORT_CONFNAME( 0x0f, 0x00, "Jackpot / Prize Key" )
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
	PORT_CONFSETTING(    0x0a, "25 GBP"  )
	PORT_CONFSETTING(    0x0b, "25 GBP (Licensed Betting Office Profile)"  )
	PORT_CONFSETTING(    0x0c, "35 GBP"  )
	PORT_CONFSETTING(    0x0d, "70 GBP"  )
	PORT_CONFSETTING(    0x0e, "Reserved"  )
	PORT_CONFSETTING(    0x0f, "Reserved"  )

	PORT_CONFNAME( 0xf0, 0x00, "Percentage Key" )
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
	PORT_CONFSETTING(    0xa0, "88" )
	PORT_CONFSETTING(    0xb0, "90" )
	PORT_CONFSETTING(    0xc0, "92" )
	PORT_CONFSETTING(    0xd0, "94" )
	PORT_CONFSETTING(    0xe0, "96" )
	PORT_CONFSETTING(    0xf0, "98" )

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
	PORT_DIPNAME( 0xf0, 0x00, "Target Percentage (if key not fitted)" )PORT_DIPLOCATION("DIL1:05,06,07,08")
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
	PORT_DIPSETTING(    0xa0, "88" )
	PORT_DIPSETTING(    0xb0, "90" )
	PORT_DIPSETTING(    0xc0, "92" )
	PORT_DIPSETTING(    0xd0, "94" )
	PORT_DIPSETTING(    0xe0, "96" )
	PORT_DIPSETTING(    0xf0, "98" )

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
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_CUSTOM)
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_CUSTOM)
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_CUSTOM)
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_CUSTOM)
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_COIN1) PORT_NAME("10p")//PORT_IMPULSE(5)
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_COIN2) PORT_NAME("20p")//PORT_IMPULSE(5)
	PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_COIN3) PORT_NAME("50p")//PORT_IMPULSE(5)
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_COIN4) PORT_NAME("100p")//PORT_IMPULSE(5)
INPUT_PORTS_END


/* Common configurations */

void mpu4_state::mpu4_install_mod4oki_space(address_space &space)
{
	space.install_readwrite_handler(0x0880, 0x0883, read8sm_delegate(*m_okicard, FUNC(mpu4_oki_sampled_sound::ic4_read)), write8sm_delegate(*m_okicard, FUNC(mpu4_oki_sampled_sound::ic4_write)));

	space.install_read_handler(0x08c0, 0x08c7, read8sm_delegate(*m_okicard, FUNC(mpu4_oki_sampled_sound::ic3_read)));
	space.install_write_handler(0x08c0, 0x08c7, write8sm_delegate(*m_okicard, FUNC(mpu4_oki_sampled_sound::ic3_write)));

}



void mpu4_state::mpu4_config_common()
{
	m_lamps.resolve();
	m_mpu4leds.resolve();
	m_digits.resolve();
	m_triacs.resolve();
	m_flutterbox.resolve();

	m_ic24_timer = timer_alloc(FUNC(mpu4_state::update_ic24), this);


	save_item(NAME( m_mmtr_data ));
	save_item(NAME( m_ay8913_address ));
	save_item(NAME( m_signal_50hz ));
	save_item(NAME( m_ic4_input_b ));
	save_item(NAME( m_aux1_input ));
	save_item(NAME( m_aux2_input ));
	save_item(NAME( m_IC23G1 ));
	save_item(NAME( m_IC23G2A ));
	save_item(NAME( m_IC23G2B ));
	save_item(NAME( m_IC23GC ));
	save_item(NAME( m_IC23GB ));
	save_item(NAME( m_IC23GA ));

	save_item(NAME( m_reel_flag ));
	save_item(NAME( m_ic23_active ));
	save_item(NAME( m_input_strobe ));
	save_item(NAME( m_lamp_strobe ));
	save_item(NAME( m_lamp_strobe2 ));
	save_item(NAME( m_lamp_strobe_ext_persistence ));
	save_item(NAME( m_led_strobe ));
	save_item(NAME( m_ay_data ));
	save_item(NAME( m_optic_pattern ));

	save_item(NAME( m_active_reel ));
	save_item(NAME( m_remote_meter ));
	save_item(NAME( m_reel_mux ));
	save_item(NAME( m_lamp_extender ));
	save_item(NAME( m_last_b7 ));
	save_item(NAME( m_last_latch ));
	save_item(NAME( m_lamp_sense ));
	save_item(NAME( m_card_live ));
	save_item(NAME( m_led_extender ));
	save_item(NAME( m_bwb_bank ));
	save_item(NAME( m_default_to_low_bank ));

	save_item(NAME( m_use_pia4_porta_leds ));
	save_item(NAME( m_pia4_porta_leds_base ));
	save_item(NAME( m_pia4_porta_leds_strobe ));

	save_item(NAME( m_use_simplecard_leds ));
	save_item(NAME( m_simplecard_leds_base ));
	save_item(NAME( m_simplecard_leds_strobe ));

	save_item(NAME( m_pageval ));
	save_item(NAME( m_pageset ));
	save_item(NAME( m_hopper_type ));
	save_item(NAME( m_reels ));
	save_item(NAME( m_chrdata ));
	save_item(NAME( m_serial_output ));

	save_item(NAME( m_numbanks ));

	save_item(NAME( m_link7a_connected ));
	save_item(NAME( m_link7b_connected ));

	save_item(NAME( m_overcurrent ));
	save_item(NAME( m_undercurrent ));

	save_item(NAME( m_overcurrent_detect ));
	save_item(NAME( m_undercurrent_detect ));

	save_item(NAME( m_low_volt_detect ));

	save_item(NAME( m_use_coinlocks ));

	save_item(NAME( m_hack_duart_fixed_low ));

	save_item(NAME( m_hopper1_opto ));
	save_item(NAME( m_hopper2_opto ));

	m_lamp_strobe_ext_persistence = 0;

}

MACHINE_START_MEMBER(mpu4_state, mod2)
{
	mpu4_config_common();

	m_link7a_connected=false;
	m_link7b_connected=true;
}

MACHINE_START_MEMBER(mpu4_state, mpu4oki)
{
	address_space &space = m_maincpu->space(AS_PROGRAM);
	mpu4_config_common();

	m_link7a_connected=false;
	m_link7b_connected=true;
	mpu4_install_mod4oki_space(space);
}

void mpu4_state::init_m4()
{
	m_bwb_bank = false;
	setup_rom_banks();
}

void mpu4_state::init_m4big()
{
	int size = memregion("maincpu")->bytes();
	if (size <= 0x10000)
	{
		fatalerror("Error: Extended banking selected on set <=0x10000 in size\n");
	}

	address_space &space = m_maincpu->space(AS_PROGRAM);

	m_bwb_bank = true;
	space.install_write_handler(0x0858, 0x0858, write8smo_delegate(*this, FUNC(mpu4_state::bankswitch_w)));
	space.install_write_handler(0x0878, 0x0878, write8smo_delegate(*this, FUNC(mpu4_state::bankset_w)));
	uint8_t *rom = memregion("maincpu")->base();

	m_numbanks = size / 0x10000;
	m_bank1->configure_entries(0, m_numbanks, &rom[0x01000], 0x10000);
	m_numbanks--;

	// some Bwb games must default to the last bank, does anything not like this
	// behavior?
	// some Bwb games don't work anyway tho, they seem to dislike something else
	// about the way the regular banking behaves, not related to the CB2 stuff
	m_bank1->set_entry(m_numbanks);
}

void mpu4_state::init_m4big_low()
{
	init_m4big();
	m_default_to_low_bank = true;
}

void mpu4_state::setup_rom_banks()
{
	//Initialise paging for non-extended ROM space
	uint8_t *rom = memregion("maincpu")->base();
	membank("bank1")->configure_entries(0, 4, &rom[0x01000], 0x10000);
	membank("bank1")->set_entry(0);
}

/* generate a 50 Hz signal (based on an RC time) */
TIMER_DEVICE_CALLBACK_MEMBER(mpu4_state::gen_50hz)
{
	if (m_low_volt_detect)
	{
		/* Although reported as a '50Hz' signal, the fact that both rising and
		falling edges of the pulse are used means the timer actually gives a 100Hz
		oscillating signal.*/
		m_signal_50hz = m_signal_50hz?0:1;
		m_pia4->ca1_w(m_signal_50hz);  /* signal is connected to IC4 CA1 */
	}
	update_meters();//run at 100Hz to sync with PIAs
}


void mpu4_state::mpu4_memmap(address_map &map)
{
	map(0x0000, 0x07ff).ram().share("nvram");
//  map(0x0800, 0x081f) // optional protection device lives here, see other maps
	map(0x0850, 0x0850).rw(FUNC(mpu4_state::bankswitch_r), FUNC(mpu4_state::bankswitch_w));    /* write bank (rom page select) */
	map(0x08e0, 0x08ef).rw(m_duart68681, FUNC(mc68681_device::read), FUNC(mc68681_device::write)); //Runs hoppers
	map(0x0900, 0x0907).rw(m_6840ptm, FUNC(ptm6840_device::read), FUNC(ptm6840_device::write));/* PTM6840 IC2 */
	map(0x0a00, 0x0a03).rw(m_pia3, FUNC(pia6821_device::read), FUNC(pia6821_device::write));        /* PIA6821 IC3 */
	map(0x0b00, 0x0b03).rw(m_pia4, FUNC(pia6821_device::read), FUNC(pia6821_device::write));        /* PIA6821 IC4 */
	map(0x0c00, 0x0c03).rw(m_pia5, FUNC(pia6821_device::read), FUNC(pia6821_device::write));        /* PIA6821 IC5 */
	map(0x0d00, 0x0d03).rw(m_pia6, FUNC(pia6821_device::read), FUNC(pia6821_device::write));        /* PIA6821 IC6 */
	map(0x0e00, 0x0e03).rw(m_pia7, FUNC(pia6821_device::read), FUNC(pia6821_device::write));        /* PIA6821 IC7 */
	map(0x0f00, 0x0f03).rw(m_pia8, FUNC(pia6821_device::read), FUNC(pia6821_device::write));        /* PIA6821 IC8 */
	map(0x1000, 0xffff).bankr("bank1");    /* 64k  paged ROM (4 pages)  */
}

void mpu4_state::mpu4_memmap_characteriser(address_map &map)
{
	mpu4_memmap(map);
	map(0x0800, 0x0810).rw(m_characteriser, FUNC(mpu4_characteriser_pal::read), FUNC(mpu4_characteriser_pal::write));
}

void mpu4_state::mpu4_memmap_bootleg_characteriser(address_map &map)
{
	mpu4_memmap(map);
	// a few sets use 0x840 for protection, 0x850 is where banking maps, so map up to that point
	map(0x0800, 0x084f).rw(m_characteriser_bl, FUNC(mpu4_characteriser_bl::read), FUNC(mpu4_characteriser_bl::write));
}

void mpu4_state::mpu4_memmap_bl_characteriser_blastbank(address_map &map)
{
	mpu4_memmap(map);
	map(0x0800, 0x081f).rw(m_characteriser_blastbank, FUNC(mpu4_characteriser_bl_blastbank::read), FUNC(mpu4_characteriser_bl_blastbank::write));
}

void mpu4_state::mpu4_reels(machine_config &config, uint8_t NumberOfReels, int16_t start_index, int16_t end_index)
{
	for(uint8_t i=0; i != NumberOfReels; i++)
	{
		REEL(config, m_reel[i], BARCREST_48STEP_REEL, start_index, end_index, 0x00, 2);
		m_reel[i]->optic_handler().set([this, i](int state) {
										   if (state)
											   m_optic_pattern |= (1 << i);
										   else
											   m_optic_pattern &= ~(1 << i);
									   });
	}
}

void mpu4_state::tr_r4(machine_config &config)
{
	m_reel_mux = STANDARD_REEL;
	m_reels = 4;
}

void mpu4_state::tr_r5(machine_config &config)
{
	m_reel_mux = FIVE_REEL_5TO8;
	m_reels = 5;
}

void mpu4_state::tr_r5r(machine_config &config)
{
	m_reel_mux = FIVE_REEL_8TO5;
	m_reels = 5;
}

void mpu4_state::tr_r5a(machine_config &config)
{
	m_reel_mux = FIVE_REEL_3TO6;
	m_reels = 5;
}

void mpu4_state::tr_r6(machine_config &config)
{
	m_reel_mux = SIX_REEL_1TO8;
	m_reels = 6;
}

void mpu4_state::tr_r6a(machine_config &config)
{
	m_reel_mux = SIX_REEL_5TO8;
	m_reels = 6;
}

void mpu4_state::tr_r7(machine_config &config)
{
	m_reel_mux = SEVEN_REEL;
	m_reels = 7;
}

void mpu4_state::tr_r8(machine_config &config)
{
	m_reel_mux = STANDARD_REEL;
	m_reels = 8;
}

void mpu4_state::tr_rt1(machine_config &config)
{
	mpu4_reels(config, m_reels, 1, 3);
}

void mpu4_state::tr_rt2(machine_config &config)
{
	mpu4_reels(config, m_reels, 4, 12);
}

void mpu4_state::tr_rt3(machine_config &config)
{
	mpu4_reels(config, m_reels, 96, 3);
}

void mpu4_state::tr_lps(machine_config &config)
{
	m_lamp_extender = SMALL_CARD;
}

void mpu4_state::tr_lpla(machine_config &config)
{
	m_lamp_extender = LARGE_CARD_A;
}

void mpu4_state::tr_lplb(machine_config &config)
{
	m_lamp_extender = LARGE_CARD_B;
}

void mpu4_state::tr_lplc(machine_config &config)
{
	m_lamp_extender = LARGE_CARD_C;
}

void mpu4_state::tr_lds(machine_config &config)
{
	m_led_extender = SIMPLE_CARD;
}

void mpu4_state::tr_lda(machine_config &config)
{
	m_led_extender = CARD_A;
}

void mpu4_state::tr_ldb(machine_config &config)
{
	m_led_extender = CARD_B;
}

void mpu4_state::tr_ldc(machine_config &config)
{
	m_led_extender = CARD_C;
}

void mpu4_state::tr_ht(machine_config &config)
{
	m_hopper_type = TUBES;
}

void mpu4_state::tr_hda(machine_config &config)
{
	m_hopper_type = HOPPER_DUART_A;
	m_hopper1->dispense_handler().set("duart68681", FUNC(mc68681_device::ip5_w));
}

void mpu4_state::tr_hdb(machine_config &config)
{
	m_hopper_type = HOPPER_DUART_B;
	m_hopper1->dispense_handler().set("duart68681", FUNC(mc68681_device::ip5_w));
}

void mpu4_state::tr_hdc(machine_config &config)
{
	m_hopper_type = HOPPER_DUART_C;
	m_hopper1->dispense_handler().set("duart68681", FUNC(mc68681_device::ip5_w));
	m_hopper2->dispense_handler().set("duart68681", FUNC(mc68681_device::ip6_w));
}

void mpu4_state::tr_hna(machine_config &config)
{
	m_hopper_type = HOPPER_NONDUART_A;
}

void mpu4_state::tr_hnb(machine_config &config)
{
	m_hopper_type = HOPPER_NONDUART_B;
}

void mpu4_state::tr_htw(machine_config &config)
{
	m_hopper_type = HOPPER_TWIN_HOPPER;
}

void mpu4_state::tr_over(machine_config &config)
{
	m_overcurrent_detect = true;
}

void mpu4_state::tr_lvdoff(machine_config &config)
{
	m_low_volt_detect = false;
}

void mpu4_state::tr_p4l(machine_config &config)
{
	m_use_pia4_porta_leds = true;
}

void mpu4_state::tr_scardl(machine_config &config)
{
	m_use_simplecard_leds = true;
}


void mpu4_state::mpu4_common(machine_config &config)
{
	TIMER(config, "50hz").configure_periodic(FUNC(mpu4_state::gen_50hz), attotime::from_hz(100));

	MSC1937(config, m_vfd);
	/* 6840 PTM */
	PTM6840(config, m_6840ptm, MPU4_MASTER_CLOCK / 4);
	m_6840ptm->set_external_clocks(0, 0, 0);
	m_6840ptm->o1_callback().set(FUNC(mpu4_state::ic2_o1_callback));
	m_6840ptm->o2_callback().set(FUNC(mpu4_state::ic2_o2_callback));
	m_6840ptm->o3_callback().set(FUNC(mpu4_state::ic2_o3_callback));
	m_6840ptm->irq_callback().set(FUNC(mpu4_state::cpu0_irq));

	PIA6821(config, m_pia3);
	m_pia3->writepa_handler().set(FUNC(mpu4_state::pia_ic3_porta_w));
	m_pia3->writepb_handler().set(FUNC(mpu4_state::pia_ic3_portb_w));
	m_pia3->ca2_handler().set(FUNC(mpu4_state::pia_ic3_ca2_w));
	m_pia3->cb2_handler().set(FUNC(mpu4_state::pia_ic3_cb2_w));
	m_pia3->irqa_handler().set(FUNC(mpu4_state::cpu0_irq));
	m_pia3->irqb_handler().set(FUNC(mpu4_state::cpu0_irq));

	PIA6821(config, m_pia4);
	m_pia4->readpb_handler().set(FUNC(mpu4_state::pia_ic4_portb_r));
	m_pia4->writepa_handler().set(FUNC(mpu4_state::pia_ic4_porta_w));
	m_pia4->writepb_handler().set(FUNC(mpu4_state::pia_ic4_portb_w));
	m_pia4->ca2_handler().set(FUNC(mpu4_state::pia_ic4_ca2_w));
	m_pia4->cb2_handler().set(FUNC(mpu4_state::pia_ic4_cb2_w));
	m_pia4->irqa_handler().set(FUNC(mpu4_state::cpu0_irq));
	m_pia4->irqb_handler().set(FUNC(mpu4_state::cpu0_irq));

	PIA6821(config, m_pia5);
	m_pia5->readpa_handler().set(FUNC(mpu4_state::pia_ic5_porta_r));
	m_pia5->readpb_handler().set(FUNC(mpu4_state::pia_ic5_portb_r));
	m_pia5->writepa_handler().set(FUNC(mpu4_state::pia_ic5_porta_w));
	m_pia5->writepb_handler().set(FUNC(mpu4_state::pia_ic5_portb_w));
	m_pia5->ca2_handler().set(m_dataport, FUNC(bacta_datalogger_device::write_txd));
	m_pia5->cb2_handler().set(FUNC(mpu4_state::pia_ic5_cb2_w));
	m_pia5->irqa_handler().set(FUNC(mpu4_state::cpu0_irq));
	m_pia5->irqb_handler().set(FUNC(mpu4_state::cpu0_irq));
	m_pia5->set_port_a_input_overrides_output_mask(0x40); // needed for m4madhse

	PIA6821(config, m_pia6);
	m_pia6->writepa_handler().set(FUNC(mpu4_state::pia_ic6_porta_w));
	m_pia6->writepb_handler().set(FUNC(mpu4_state::pia_ic6_portb_w));
	m_pia6->ca2_handler().set(FUNC(mpu4_state::pia_ic6_ca2_w));
	m_pia6->cb2_handler().set(FUNC(mpu4_state::pia_ic6_cb2_w));
	m_pia6->irqa_handler().set(FUNC(mpu4_state::cpu0_irq));
	m_pia6->irqb_handler().set(FUNC(mpu4_state::cpu0_irq));

	PIA6821(config, m_pia7);
	m_pia7->readpb_handler().set(FUNC(mpu4_state::pia_ic7_portb_r));
	m_pia7->writepa_handler().set(FUNC(mpu4_state::pia_ic7_porta_w));
	m_pia7->writepb_handler().set(FUNC(mpu4_state::pia_ic7_portb_w));
	m_pia7->ca2_handler().set(FUNC(mpu4_state::pia_ic7_ca2_w));
	m_pia7->cb2_handler().set(FUNC(mpu4_state::pia_ic7_cb2_w));
	m_pia7->irqa_handler().set(FUNC(mpu4_state::cpu0_irq));
	m_pia7->irqb_handler().set(FUNC(mpu4_state::cpu0_irq));

	PIA6821(config, m_pia8);
	m_pia8->readpa_handler().set(FUNC(mpu4_state::pia_ic8_porta_r));
	m_pia8->writepb_handler().set(FUNC(mpu4_state::pia_ic8_portb_w));
	m_pia8->ca2_handler().set(FUNC(mpu4_state::pia_ic8_ca2_w));
	m_pia8->cb2_handler().set(FUNC(mpu4_state::pia_ic8_cb2_w));
	m_pia8->irqa_handler().set(FUNC(mpu4_state::cpu0_irq));
	m_pia8->irqb_handler().set(FUNC(mpu4_state::cpu0_irq));

	METERS(config, m_meters, 0).set_number(8);

	BACTA_DATALOGGER(config, m_dataport, 0);
	m_dataport->rxd_handler().set(FUNC(mpu4_state::dataport_rxd));

	HOPPER(config, m_hopper1, attotime::from_msec(100));


	SPEAKER(config, "mono").front_center();

	DAC_1BIT(config, m_alarmdac, 0);
	m_alarmdac->add_route(ALL_OUTPUTS, "mono", 0.50);
}

/***********************************************************************************************

  Base config with no sound

***********************************************************************************************/

void mpu4_state::mpu4base(machine_config &config)
{
	MCFG_MACHINE_START_OVERRIDE(mpu4_state, mod2)
	MCFG_MACHINE_RESET_OVERRIDE(mpu4_state, mpu4)
	MC6809(config, m_maincpu, MPU4_MASTER_CLOCK); // MC68B09P
	m_maincpu->set_addrmap(AS_PROGRAM, &mpu4_state::mpu4_memmap);

	mpu4_common(config);


	NVRAM(config, "nvram", nvram_device::DEFAULT_ALL_0);

	MC68681(config, m_duart68681, MPU4_MASTER_CLOCK); // ?

	config.set_default_layout(layout_mpu4);
}

/***********************************************************************************************

  Configs for Mod2

  TODO: mod2 should eventually become a subclass

***********************************************************************************************/

void mpu4_state::mod2_f(machine_config &config)
{
	mpu4base(config);
	AY8913(config, m_ay8913, MPU4_MASTER_CLOCK/4);

	m_ay8913->set_flags(AY8910_SINGLE_OUTPUT);
	m_ay8913->set_resistors_load(820, 0, 0);
	m_ay8913->add_route(ALL_OUTPUTS, "mono", 1.0);
}

void mpu4_state::mod2_no_bacta_f(machine_config &config)
{
	mod2_f(config);
	config.device_remove("dataport");
	m_pia5->ca2_handler().set(FUNC(mpu4_state::dataport_rxd));
}

void mpu4_state::mod2_cheatchr_f(machine_config &config)
{
	mod2_f(config);

	m_maincpu->set_addrmap(AS_PROGRAM, &mpu4_state::mpu4_memmap_characteriser);

	MPU4_CHARACTERISER_PAL(config, m_characteriser, 0);
	m_characteriser->set_cpu_tag("maincpu");
	m_characteriser->set_allow_6809_cheat(true);
	m_characteriser->set_lamp_table(nullptr);
}

/***********************************************************************************************

  Configs for Mod4 with OKI

  TODO: OKI is its own device, should mod4oki eventually become a subclass

***********************************************************************************************/

// standard reel setup

void mpu4_state::mod4oki_f(machine_config &config)
{
	mpu4base(config);
	MCFG_MACHINE_START_OVERRIDE(mpu4_state, mpu4oki)

	MPU4_OKI_SAMPLED_SOUND(config, m_okicard, MPU4_MASTER_CLOCK/4);
	m_okicard->add_route(ALL_OUTPUTS, "mono", 1.0);

	m_okicard->cb2_handler().set(FUNC(mpu4_state::pia_gb_cb2_w));

}

void mpu4_state::mod4oki_no_bacta_f(machine_config &config)
{
	mod4oki_f(config);
	config.device_remove("dataport");
	m_pia5->ca2_handler().set(FUNC(mpu4_state::dataport_rxd));
}

void mpu4_state::mod4oki_cheatchr_f(machine_config &config)
{
	mod4oki_f(config);

	m_maincpu->set_addrmap(AS_PROGRAM, &mpu4_state::mpu4_memmap_characteriser);

	MPU4_CHARACTERISER_PAL(config, m_characteriser, 0);
	m_characteriser->set_cpu_tag("maincpu");
	m_characteriser->set_allow_6809_cheat(true);
	m_characteriser->set_lamp_table(nullptr);
}
