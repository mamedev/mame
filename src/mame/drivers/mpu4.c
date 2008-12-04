/***********************************************************************************************************
  Barcrest MPU4 highly preliminary driver by J.Wallace, and Anonymous.
  This is the core driver, no game specific stuff should go in here.

     09-2007: Haze: Added Deal 'Em video support.
  03-08-2007: J Wallace: Removed audio filter for now, since sound is more accurate without them.
                         Connect 4 now has the right sound.
  03-07-2007: J Wallace: Several major changes, including input relabelling, and system timer improvements.
     06-2007: Atari Ace, many cleanups and optimizations of I/O routines
  09-06-2007: J Wallace: Fixed 50Hz detection circuit.
  09-04-2007: J Wallace: Corrected a lot of out of date info in this revision history.
                         Revisionism, if you will.
  17-02-2007: J Wallace: Added Deal 'Em - still needs some work.
  10-02-2007: J Wallace: Improved input timing.
  30-01-2007: J Wallace: Characteriser rewritten to run the 'extra' data needed by some games.
  24-01-2007: J Wallace: With thanks to Canonman and HIGHWAYMAN/System 80, I was able to confirm a seemingly
              ghastly misuse of a PIA is actually on the real hardware. This fixes the meters.
  30-12-2006: J Wallace: Fixed init routines, state saving is theoretically supported.
  23-09-2006: Converted 7Segment code to cleaner version, but have yet to add new 16k EPROM, as
              the original image was purported to come from a Barcrest BBS service, and is probably
              more 'official'.
  07-09-2006: It appears that the video firmware is intended to be a 16k EPROM, not 64k as dumped.
              In fact, the current dump is just the code repeated 4 times, when nothing earlier than
              0xC000 is ever called. Presumably, the ROM is loaded into C000, and the remainder of the
              'ROM' space is in fact the extra 32k of RAM apparently needed to hold the video board data.
              For now, we'll continue to use the old dump, as I am unsure as to how to map this in MAME.
              Changed Turn Over's name, it's either 12 Pound or 20 Pound Turn Over, depending on settings.

              At this stage, I cannot see how to progress any further with the emulation, so I have transferred
              a non-AWP version of this driver to MAME for further study.

  06-09-2006: Connect 4 added - VFD is currently reversed, although it looks to me like
              that's the correct behaviour, and that my 'correction' for Old Timer was wrong.
              AY sound does work, but is horrendous - presumably there's a filter on the
              sound that I haven't spotted. Trackball is apparently connected to AUX1, via
              Schmitt triggers - I wish I had a clue what this meant (El Condor).

  05-09-2006: And the award for most bone-headed bug goes to.. me! the 6840 handler wasn't
              resetting the clocks after timeout, so the wave frequencies were way off.
              Machines now hang on reset due to a lack of reel support.
              CHR decoding now included, but still requires knowledge of the data table
              location - this should be present in the ROMs somewhere.

  11-08-2006: It appears that the PIA IRQ's are not connected after all - but even
              disabling these won't get around the PTM issues.
              It also looks like the video card CHR is just a word-based version
              of the original, byte-based chip, meaning that should be fairly simple
              to emulate too, when the time comes.

  08-07-2006: Revised mapping of peripherals, and found method of calculating CHR
              values - although their use is still unknown.

  11-05-2006: El Condor, working from schematics and photos at present
  28-04-2006: El Condor
  20-05-2004: Re-Animator

  See http://www.mameworld.net/agemame/techinfo/mpu4.php for Information.

--- Board Setup ---

The MPU4 BOARD is the driver board, originally designed to run Fruit Machines made by the Barcrest Group, but later
licensed to other firms as a general purpose unit (even some old Photo-Me booths used the unit).

This original board uses a ~1.72 Mhz 6809B CPU, and a number of PIA6821 chips for multiplexing inputs and the like.

A 6840PTM is used for internal timing, one of it's functions is to act with an AY8913 chip as a crude analogue sound device.
(Data is transmitted through a PIA, with a square wave from the PTM being used as the alarm sound generator)

A MPU4 GAME CARD (cartridge) plugs into the MPU4 board containing the game, and a protection PAL (the 'characteriser').
This PAL, as well as protecting the games, also controlled some of the lamp address matrix for many games, and acted as
an anti-tampering device which helped to prevent the hacking of certain titles in a manner which broke UK gaming laws.

One of the advantages of the hardware setup was that the developer could change the nature of the game card
up to a point, adding extra lamp support, different amounts of RAM, and (in many cases) an OKI MSM6376 or Yamaha synth chip
and related PIA and PTM for improved audio (This was eventually made the only way to generate sound in MOD4 of the hardware,
when the AY8913 was removed from the main board)

Everything here is preliminary...  the boards are quite fussy with regards their self tests
and the timing may have to be perfect for them to function correctly.  (as the comms are
timer driven, the video is capable of various raster effects etc.)

Datasheets are available for the main components, The AGEMAME site mirrors a few of the harder-to-find ones.

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
 0880      |R/W| D D D D D D D D | PIA6821 on soundboard (Oki MSM6376@16MHz,(NV)
           |   |                 | port A = ??
           |   |                 | port B (882)
           |   |                 |        b7 = 0 if OKI busy
           |   |                 |             1 if OKI ready
           |   |                 |        b6 = ??
           |   |                 |        b5 = volume control clock
           |   |                 |        b4 = volume control direction (0= up, 1 = down)
           |   |                 |        b3 = ??
           |   |                 |        b2 = ??
           |   |                 |        b1 = ??
           |   |                 |        b0 = ??
-----------+---+-----------------+--------------------------------------------------------------------------
 08C0      |   |                 | MC6840 on sound board(NV?)
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

TODO: - Fix lamp timing, MAME doesn't update fast enough to see everything
      - Distinguish door switches using manual
***********************************************************************************************************/

#include "driver.h"
#include "machine/6821pia.h"
#include "machine/6840ptm.h"

#include "deprecat.h"
#include "timer.h"
#include "cpu/m6809/m6809.h"
#include "sound/ay8910.h"
#include "machine/steppers.h"
#include "machine/roc10937.h"
#include "machine/meters.h"

#ifdef MAME_DEBUG
#define MPU4VERBOSE 1
#else
#define MPU4VERBOSE 0
#endif

#define LOG(x)	do { if (MPU4VERBOSE) logerror x; } while (0)
#define LOG_CHR(x)	do { if (MPU4VERBOSE) logerror x; } while (0)
#define LOG_CHR_FULL(x)	do { if (MPU4VERBOSE) logerror x; } while (0)
#define LOG_IC3(x)	do { if (MPU4VERBOSE) logerror x; } while (0)
#define LOG_IC8(x)	do { if (MPU4VERBOSE) logerror x; } while (0)

#ifndef AWP_VIDEO /*Defined for fruit machines with mechanical reels*/
#define draw_reel(x)
#else
#define draw_reel(x) awp_draw_reel x
#endif
#include "mpu4.lh"
#include "connect4.lh"
#define MPU4_MASTER_CLOCK (6880000)

/* local vars */
static int mod_number;
static int mmtr_data;
static int alpha_data_line;
static int alpha_clock;
static int ay8913_address;
static int serial_data;
static int signal_50hz;
static int ic4_input_b;
static int IC23G1;
static int IC23G2A;
static int IC23G2B;
static int IC23GC;
static int IC23GB;
static int IC23GA;
static int prot_col;
static int lamp_col;
static int ic23_active;
static int led_extend;
static int serial_card_connected;
static emu_timer *ic24_timer;
static TIMER_CALLBACK( ic24_timeout );


/* 32 multiplexed inputs - but a further 8 possible per AUX.
Two connectors 'orange' (sampled every 8ms) and 'black' (sampled every 16ms)
Each connector carries two banks of eight inputs and two enable signals */
static int	  input_strobe;	  /* IC23 74LS138 A = CA2 IC7, B = CA2 IC4, C = CA2 IC8 */
static UINT8  lamp_strobe,lamp_strobe2;
static UINT8  lamp_data;
static UINT8  ay_data;

extern const UINT8 MPU4_chr_lut[72];
static UINT8 MPU4_chr_data[72];

static UINT8 Lamps[128];		/* 128 multiplexed lamps */
static int optic_pattern;

/* Lookup table for CHR data */
const UINT8 MPU4_chr_lut[72]= {	0x00,0x1A,0x04,0x10,0x18,0x0F,0x13,0x1B,
								0x03,0x07,0x17,0x1D,0x36,0x35,0x2B,0x28,
								0x39,0x21,0x22,0x25,0x2C,0x29,0x31,0x34,
								0x0A,0x1F,0x06,0x0E,0x1C,0x12,0x1E,0x0D,
								0x14,0x0A,0x19,0x15,0x06,0x0F,0x08,0x1B,
								0x1E,0x04,0x01,0x0C,0x18,0x1A,0x11,0x0B,
								0x03,0x17,0x10,0x1D,0x0E,0x07,0x12,0x09,
								0x0D,0x1F,0x16,0x05,0x13,0x1C,0x02,0x00,
								0x00,0x01,0x04,0x09,0x10,0x19,0x24,0x31};

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

static UINT8 led_segs[8];


/* Process lamp and LED data for output system */
static void mpu4_draw_led(UINT8 id, UINT8 value)
{
	output_set_digit_value(id,value);
}


static void draw_lamps(void)
{
	int i;

	for (i = 0; i < 8; i++)
	{
		output_set_lamp_value((8*input_strobe)+i, (Lamps[(8*input_strobe)+i]));
		output_set_lamp_value((8*input_strobe)+i+64, (Lamps[(8*input_strobe)+i+64]));
	}
}


static void update_lamps(void)
{
	int i;

	for (i = 0; i < 8; i++)
	{
		Lamps[(8*input_strobe)+i]    = (lamp_strobe  & (1 << i)) != 0;
		Lamps[(8*input_strobe)+i+64] = (lamp_strobe2 & (1 << i)) != 0;
	}

	if (led_extend)
	{
		/* Some games uses 'programmable' LED displays, built from light display lines. */
		UINT8 pled_segs[2] = {0,0};

		static const int lamps1[8] = { 106, 107, 108, 109, 104, 105, 110, 133 };
		static const int lamps2[8] = { 114, 115, 116, 117, 112, 113, 118, 119 };

		for (i = 0; i < 8; i++)
		{
			if (output_get_lamp_value(lamps1[i])) pled_segs[0] |= (1 << i);
			if (output_get_lamp_value(lamps2[i])) pled_segs[1] |= (1 << i);
		}

		mpu4_draw_led(8, pled_segs[0]);
		mpu4_draw_led(9, pled_segs[1]);
	}
	draw_lamps();
}


/* called if board is reset */
static void mpu4_stepper_reset(void)
{
	int pattern = 0,reel;
	for (reel = 0; reel < 6; reel++)
	{
		stepper_reset_position(reel);
		if (stepper_optic_state(reel)) pattern |= 1<<reel;
	}
	optic_pattern = pattern;
}


static MACHINE_RESET( mpu4 )
{
	ROC10937_reset(0);	/* reset display1 */

	mpu4_stepper_reset();

	lamp_strobe    = 0;
	lamp_strobe2   = 0;
	lamp_data      = 0;

	IC23GC    = 0;
	IC23GB    = 0;
	IC23GA    = 0;
	IC23G1    = 1;
	IC23G2A   = 0;
	IC23G2B   = 0;

	prot_col  = 0;

/* init rom bank, some games don't set this */
	{
		UINT8 *rom = memory_region(machine, "main");

		memory_configure_bank(machine, 1, 0, 8, &rom[0x01000], 0x10000);

		memory_set_bank(machine, 1,0);
		cpu_reset(machine->cpu[0]);
	}

}


/* 6809 IRQ handler */
static void cpu0_irq(running_machine *machine, int state)
{
	/* The PIA and PTM IRQ lines are all connected to a common PCB track, leading directly to the 6809 IRQ line. */
	int combined_state = pia_get_irq_a(0) | pia_get_irq_b(0) |
						 pia_get_irq_a(1) | pia_get_irq_b(1) |
						 pia_get_irq_a(2) | pia_get_irq_b(2) |
						 pia_get_irq_a(3) | pia_get_irq_b(3) |
						 pia_get_irq_a(4) | pia_get_irq_b(4) |
						 pia_get_irq_a(5) | pia_get_irq_b(5) |
						 ptm6840_get_irq(0);

	if (!serial_card_connected)
	{
		cpu_set_input_line(machine->cpu[0], M6809_IRQ_LINE, combined_state ? ASSERT_LINE : CLEAR_LINE);
		LOG(("6809 int%d \n", combined_state));
	}
	else
	{
		cpu_set_input_line(machine->cpu[0], M6809_FIRQ_LINE, combined_state ? ASSERT_LINE : CLEAR_LINE);
		LOG(("6809 fint%d \n", combined_state));
	}
}


/* Bankswitching */
static WRITE8_HANDLER( bankswitch_w )
{
	memory_set_bank(space->machine, 1,data & 0x07);
}


static READ8_HANDLER( bankswitch_r )
{
	return memory_get_bank(space->machine, 1);
}


/* IC2 6840 PTM handler */
static WRITE8_HANDLER( ic2_o1_callback )
{
	ptm6840_set_c2(space->machine,0,data);

	/* copy output value to IC2 c2
    this output is the clock for timer2 */
}


static WRITE8_HANDLER( ic2_o2_callback )
{
	pia_set_input_ca1(0, data); /* copy output value to IC3 ca1 */

	/* the output from timer2 is the input clock for timer3 */
	ptm6840_set_c3(   space->machine, 0, data);
}


static WRITE8_HANDLER( ic2_o3_callback )
{
	/* the output from timer3 is used as a square wave for the alarm output
    and as an external clock source for timer 1! */

	ptm6840_set_c1(    space->machine, 0, data);
}


static const ptm6840_interface ptm_ic2_intf =
{
	MPU4_MASTER_CLOCK/4,
	{ 0,0,0 },
	{ ic2_o1_callback, ic2_o2_callback, ic2_o3_callback },
	cpu0_irq
};


/* 6821 PIA handlers */
/* IC3, lamp data lines + alpha numeric display */
static WRITE8_HANDLER( pia_ic3_porta_w )
{
	LOG_IC3(("%04x IC3 PIA Port A Set to %2x (lamp strobes 1 - 9)\n", cpu_get_previouspc(space->cpu),data));

	if(ic23_active)
	{
		lamp_strobe = data;
		update_lamps();
	}
}


static WRITE8_HANDLER( pia_ic3_portb_w )
{
	LOG_IC3(("%04x IC3 PIA Port B Set to %2x  (lamp strobes 10 - 17)\n", cpu_get_previouspc(space->cpu),data));

	if(ic23_active)
	{
		lamp_strobe2 = data;
		update_lamps();
	}
}


static WRITE8_HANDLER( pia_ic3_ca2_w )
{
	LOG_IC3(("%04x IC3 PIA Write CA2 (alpha data), %02X\n", cpu_get_previouspc(space->cpu),data));

	alpha_data_line = data;
	ROC10937_draw_16seg(0);
}


static WRITE8_HANDLER( pia_ic3_cb2_w )
{
	LOG_IC3(("%04x IC3 PIA Write CB (alpha reset), %02X\n",cpu_get_previouspc(space->cpu),data));

	if ( data ) ROC10937_reset(0);
	ROC10937_draw_16seg(0);
}


static const pia6821_interface pia_ic3_intf =
{
	/*inputs : A/B,CA/B1,CA/B2 */ 0, 0, 0, 0, 0, 0,
	/*outputs: A/B,CA/B2       */ pia_ic3_porta_w, pia_ic3_portb_w, pia_ic3_ca2_w, pia_ic3_cb2_w,
	/*irqs   : A/B             */ cpu0_irq, cpu0_irq
};

/*
IC23 emulation

IC23 is a 74LS138 1-of-8 Decoder

It is used as a multiplexer for the LEDs, lamp selects and inputs.*/

static void ic23_update(void)
{
	if (!IC23G2A)
	{
		if (!IC23G2B)
		{
			if (IC23G1)
			{
				if ( IC23GA ) input_strobe |= 0x01;
				else          input_strobe &= ~0x01;

				if ( IC23GB ) input_strobe |= 0x02;
				else          input_strobe &= ~0x02;

				if ( IC23GC ) input_strobe |= 0x04;
				else          input_strobe &= ~0x04;
			}
		}
	}
	else
	if ((IC23G2A)||(IC23G2B))
	{
		input_strobe = 0x00;
	}
}


/*
IC24 emulation

IC24 is a 74LS122 pulse generator

CLEAR and B2 are tied high and A1 and A2 tied low, meaning any pulse
on B1 will give a low pulse on the output pin.
*/
static void ic24_output(int data)
{
	IC23G2A = data;
	ic23_update();
}


static void ic24_setup(void)
{
	if (IC23GA)
	{
		double duration = TIME_OF_74LS123((220*1000),(0.1*0.000001));
		{
			ic23_active=1;
			ic24_output(0);
			timer_adjust_oneshot(ic24_timer, double_to_attotime(duration), 0);
		}
	}
}


static TIMER_CALLBACK( ic24_timeout )
{
	ic23_active=0;
	ic24_output(1);
}


/* IC4 IC4, 7 seg leds, 50Hz timer reel sensors, current sensors */
static WRITE8_HANDLER( pia_ic4_porta_w )
{
	if(ic23_active)
	{
		led_segs[input_strobe] = data;
		mpu4_draw_led(input_strobe, led_segs[input_strobe]);
	}
}


static READ8_HANDLER( pia_ic4_portb_r )
{
	if ( serial_data )
	{
		ic4_input_b |=  0x80;
		pia_set_input_cb1(1, 1);
	}
	else
	{
		ic4_input_b &= ~0x80;
		pia_set_input_cb1(1, 0);
	}

	if ( optic_pattern & 0x01 ) ic4_input_b |=  0x40; /* reel A tab */
	else                        ic4_input_b &= ~0x40;

	if ( optic_pattern & 0x02 ) ic4_input_b |=  0x20; /* reel B tab */
	else                        ic4_input_b &= ~0x20;

	if ( optic_pattern & 0x04 ) ic4_input_b |=  0x10; /* reel C tab */
	else                        ic4_input_b &= ~0x10;

	if ( optic_pattern & 0x08 ) ic4_input_b |=  0x08; /* reel D tab */
	else                        ic4_input_b &= ~0x08;

	if ( signal_50hz ) 			ic4_input_b |=  0x04; /* 50 Hz */
	else   	                    ic4_input_b &= ~0x04;

	#ifdef UNUSED_FUNCTION
	if ( lamp_overcurrent  ) ic4_input_b |= 0x02;
	if ( lamp_undercurrent ) ic4_input_b |= 0x01;
	#endif

	LOG_IC3(("%04x IC4 PIA Read of Port B %x\n",cpu_get_previouspc(space->cpu),ic4_input_b));
	return ic4_input_b;
}


static WRITE8_HANDLER( pia_ic4_ca2_w )
{
	LOG_IC3(("%04x IC4 PIA Write CA (input MUX strobe /LED B), %02X\n", cpu_get_previouspc(space->cpu),data));

	IC23GB = data;
	ic23_update();
}


static const pia6821_interface pia_ic4_intf =
{
	/*inputs : A/B,CA/B1,CA/B2 */ 0, pia_ic4_portb_r, 0, 0, 0, 0,
	/*outputs: A/B,CA/B2       */ pia_ic4_porta_w, 0, pia_ic4_ca2_w, 0,
	/*irqs   : A/B             */ cpu0_irq, cpu0_irq
};


/* IC5, AUX ports, coin lockouts and AY sound chip select (MODs below 4 only) */
static READ8_HANDLER( pia_ic5_porta_r )
{
	LOG(("%04x IC5 PIA Read of Port A (AUX1)\n",cpu_get_previouspc(space->cpu)));
	return input_port_read(space->machine, "AUX1");
}


static READ8_HANDLER( pia_ic5_portb_r )
{
	LOG(("%04x IC5 PIA Read of Port B (coin input AUX2)\n",cpu_get_previouspc(space->cpu)));
	coin_lockout_w(0, (pia_get_output_b(2) & 0x01) );
	coin_lockout_w(1, (pia_get_output_b(2) & 0x02) );
	coin_lockout_w(2, (pia_get_output_b(2) & 0x04) );
	coin_lockout_w(3, (pia_get_output_b(2) & 0x08) );
	return input_port_read(space->machine, "AUX2");
}


static WRITE8_HANDLER( pia_ic5_ca2_w )
{
	LOG(("%04x IC5 PIA Write CA2 (Serial Tx) %2x\n",cpu_get_previouspc(space->cpu),data));
	serial_data = data;
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
static void update_ay(const address_space *space)
{
	if (!pia_get_output_cb2(2))
	{
		switch (ay8913_address)
		{
  			case 0x00:
			{
				/* Inactive */
				break;
		    }
		  	case 0x01:
			{	/* CA2 = 1 CB2 = 0? : Read from selected PSG register and make the register data available to Port A */
				LOG(("AY8913 address = %d \n",pia_get_output_a(3)&0x0f));
				break;
		  	}
		  	case 0x02:
			{/* CA2 = 0 CB2 = 1? : Write to selected PSG register and write data to Port A */
	  			ay8910_write_port_0_w(space, 0, pia_get_output_a(3));
				LOG(("AY Chip Write \n"));
				break;
	  		}
		  	case 0x03:
			{/* CA2 = 1 CB2 = 1? : The register will now be selected and the user can read from or write to it.
             The register will remain selected until another is chosen.*/
				ay8910_control_port_0_w(space, 0, pia_get_output_a(3));
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


static WRITE8_HANDLER( pia_ic5_cb2_w )
{
    update_ay(space);
}


static const pia6821_interface pia_ic5_intf =
{
	/*inputs : A/B,CA/B1,CA/B2 */ pia_ic5_porta_r, pia_ic5_portb_r, 0, 0, 0, 0,
	/*outputs: A/B,CA/B2       */ 0, 0, pia_ic5_ca2_w,  pia_ic5_cb2_w,
	/*irqs   : A/B             */ cpu0_irq, cpu0_irq
};


/* IC6, Reel A and B and AY registers (MODs below 4 only) */
static WRITE8_HANDLER( pia_ic6_portb_w )
{
	LOG(("%04x IC6 PIA Port B Set to %2x (Reel A and B)\n", cpu_get_previouspc(space->cpu),data));
	stepper_update(0, data & 0x0F );
	stepper_update(1, (data>>4) & 0x0F );

/*  if ( pia_get_output_cb2(1)) */
	{
		if ( stepper_optic_state(0) ) optic_pattern |=  0x01;
		else                          optic_pattern &= ~0x01;

		if ( stepper_optic_state(1) ) optic_pattern |=  0x02;
		else                          optic_pattern &= ~0x02;
	}
	draw_reel((0));
	draw_reel((1));
}


static WRITE8_HANDLER( pia_ic6_porta_w )
{
	LOG(("%04x IC6 PIA Write A %2x\n", cpu_get_previouspc(space->cpu),data));
	if (mod_number <4)
	{
	  	ay_data = data;
	    update_ay(space);
	}
}


static WRITE8_HANDLER( pia_ic6_ca2_w )
{
	LOG(("%04x IC6 PIA write CA2 %2x (AY8913 BC1)\n", cpu_get_previouspc(space->cpu),data));
	if (mod_number <4)
	{
		if ( data ) ay8913_address |=  0x01;
		else        ay8913_address &= ~0x01;
		update_ay(space);
	}
}


static WRITE8_HANDLER( pia_ic6_cb2_w )
{
	LOG(("%04x IC6 PIA write CB2 %2x (AY8913 BCDIR)\n", cpu_get_previouspc(space->cpu),data));
	if (mod_number <4)
	{
		if ( data ) ay8913_address |=  0x02;
		else        ay8913_address &= ~0x02;
		update_ay(space);
	}
}


static const pia6821_interface pia_ic6_intf =
{
	/*inputs : A/B,CA/B1,CA/B2 */ 0, 0, 0, 0, 0, 0,
	/*outputs: A/B,CA/B2       */ pia_ic6_porta_w, pia_ic6_portb_w, pia_ic6_ca2_w, pia_ic6_cb2_w,
	/*irqs   : A/B             */ cpu0_irq, cpu0_irq
};


/* IC7 Reel C and D, mechanical meters/Reel E and F, input strobe bit A */
static WRITE8_HANDLER( pia_ic7_porta_w )
{
	LOG(("%04x IC7 PIA Port A Set to %2x (Reel C and D)\n", cpu_get_previouspc(space->cpu),data));
	stepper_update(2, data & 0x0F );
	stepper_update(3, (data >> 4)& 0x0F );

/*  if ( pia_get_output_cb2(1)) */
	{
		if ( stepper_optic_state(2) ) optic_pattern |=  0x04;
		else                          optic_pattern &= ~0x04;
		if ( stepper_optic_state(3) ) optic_pattern |=  0x08;
		else                          optic_pattern &= ~0x08;
	}
	draw_reel((2));
	draw_reel((3));
}


static WRITE8_HANDLER( pia_ic7_portb_w )
{
	int meter;
	UINT64 cycles = cpu_get_total_cycles(space->cpu);

/* The meters are connected to a voltage drop sensor, where current
flowing through them also passes through pin B7, meaning that when
any meter is activated, pin B7 goes high.
As for why they connected this to an output port rather than using
CB1, no idea.
This appears to have confounded the schematic drawer, who has assumed that
all eight meters are driven from this port, giving the 8 line driver chip
9 connections in total. */

	mmtr_data = data;
	if (mmtr_data)
	{
		pia_set_input_b(4, mmtr_data | 0x80);
		for (meter = 0; meter < 8; meter ++)
		if (mmtr_data & (1 << meter))	Mechmtr_update(meter, cycles, mmtr_data & (1 << meter));
	}
	else
	{
		pia_set_input_b(4, mmtr_data &~0x80);
	}

	LOG(("%04x IC7 PIA Port B Set to %2x (Meters, Reel E and F)\n", cpu_get_previouspc(space->cpu),data));
}


static WRITE8_HANDLER( pia_ic7_ca2_w )
{
	LOG(("%04x IC7 PIA write CA2 %2x (input strobe bit 0 / LED A)\n", cpu_get_previouspc(space->cpu),data));

	IC23GA = data;
	ic24_setup();
	ic23_update();
}


static WRITE8_HANDLER( pia_ic7_cb2_w )
{
/* The eighth meter is connected here, because the voltage sensor
is on PB7. */
	UINT64 cycles = cpu_get_total_cycles(space->cpu);
	if (data)
	{
		pia_set_input_b(4,mmtr_data|0x80);
		Mechmtr_update(7, cycles, data );
	}
	LOG(("%04x IC7 PIA write CB2 %2x \n", cpu_get_previouspc(space->cpu),data));
}


static const pia6821_interface pia_ic7_intf =
{
	/*inputs : A/B,CA/B1,CA/B2 */ 0, 0, 0, 0, 0, 0,
	/*outputs: A/B,CA/B2       */ pia_ic7_porta_w, pia_ic7_portb_w, pia_ic7_ca2_w, pia_ic7_cb2_w,
	/*irqs   : A/B             */ cpu0_irq, cpu0_irq
};


/* IC8, Inputs, TRIACS, alpha clock */
static READ8_HANDLER( pia_ic8_porta_r )
{
	static const char *const portnames[] = { "ORANGE1", "ORANGE2", "BLACK1", "BLACK2", "ORANGE1", "ORANGE2", "DIL1", "DIL2" };

	LOG_IC8(("%04x IC8 PIA Read of Port A (MUX input data)\n", cpu_get_previouspc(space->cpu)));
/* The orange inputs are polled twice as often as the black ones, for reasons of efficiency.
   This is achieved via connecting every input line to an AND gate, thus allowing two strobes
   to represent each orange input bank (strobes are active low). */
	pia_set_input_cb1(2, (input_port_read(space->machine, "AUX2") & 0x80));
	return input_port_read(space->machine, portnames[input_strobe]);
}


static WRITE8_HANDLER( pia_ic8_portb_w )
{
	int i;
	LOG_IC8(("%04x IC8 PIA Port B Set to %2x (OUTPUT PORT, TRIACS)\n", cpu_get_previouspc(space->cpu),data));
	for (i = 0; i < 8; i++)
		if ( data & (1 << i) )		output_set_indexed_value("triac", i, data & (1 << i));
}


static WRITE8_HANDLER( pia_ic8_ca2_w )
{
	LOG_IC8(("%04x IC8 PIA write CA2 (input_strobe bit 2 / LED C) %02X\n", cpu_get_previouspc(space->cpu), data & 0xFF));

	IC23GC = data;
	ic23_update();
}


static WRITE8_HANDLER( pia_ic8_cb2_w )
{
	LOG_IC8(("%04x IC8 PIA write CB2 (alpha clock) %02X\n", cpu_get_previouspc(space->cpu), data & 0xFF));

	if ( !alpha_clock && (data) )
	{
		ROC10937_shift_data(0, alpha_data_line&0x01?0:1);
	}
	alpha_clock = data;
	ROC10937_draw_16seg(0);
}


static const pia6821_interface pia_ic8_intf =
{
	/*inputs : A/B,CA/B1,CA/B2 */ pia_ic8_porta_r, 0, 0, 0, 0, 0,
	/*outputs: A/B,CA/B2       */ 0, pia_ic8_portb_w, pia_ic8_ca2_w, pia_ic8_cb2_w,
	/*irqs   : A/B             */ cpu0_irq, cpu0_irq
};


/* input ports for MPU4 board */
static INPUT_PORTS_START( mpu4 )
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
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_BUTTON1) PORT_NAME("Hi")
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_BUTTON2) PORT_NAME("Lo")
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_OTHER)   PORT_NAME("18")
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_OTHER)   PORT_NAME("19")
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_OTHER) 	PORT_NAME("20")
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_SERVICE) PORT_NAME("Test Button") PORT_CODE(KEYCODE_W)
	PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_SERVICE) PORT_NAME("Refill Key") PORT_CODE(KEYCODE_R) PORT_TOGGLE
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_INTERLOCK) PORT_NAME("Cashbox Door")  PORT_CODE(KEYCODE_Q) PORT_TOGGLE

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
	PORT_DIPNAME( 0x01, 0x00, "DIL101" ) PORT_DIPLOCATION("DIL1:01")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x01, DEF_STR( On  ) )
	PORT_DIPNAME( 0x02, 0x00, "DIL102" ) PORT_DIPLOCATION("DIL1:02")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x02, DEF_STR( On  ) )
	PORT_DIPNAME( 0x04, 0x00, "DIL103" ) PORT_DIPLOCATION("DIL1:03")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x04, DEF_STR( On  ) )
	PORT_DIPNAME( 0x08, 0x00, "DIL104" ) PORT_DIPLOCATION("DIL1:04")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x08, DEF_STR( On  ) )
	PORT_DIPNAME( 0x10, 0x00, "DIL105" ) PORT_DIPLOCATION("DIL1:05")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x10, DEF_STR( On  ) )
	PORT_DIPNAME( 0x20, 0x00, "DIL106" ) PORT_DIPLOCATION("DIL1:06")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x20, DEF_STR( On  ) )
	PORT_DIPNAME( 0x40, 0x00, "DIL107" ) PORT_DIPLOCATION("DIL1:07")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x40, DEF_STR( On  ) )
	PORT_DIPNAME( 0x80, 0x00, "DIL108" ) PORT_DIPLOCATION("DIL1:08")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x80, DEF_STR( On  ) )

	PORT_START("DIL2")
	PORT_DIPNAME( 0x01, 0x00, "DIL201" ) PORT_DIPLOCATION("DIL2:01")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x01, DEF_STR( On  ) )
	PORT_DIPNAME( 0x02, 0x00, "DIL202" ) PORT_DIPLOCATION("DIL2:02")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x02, DEF_STR( On  ) )
	PORT_DIPNAME( 0x04, 0x00, "DIL203" ) PORT_DIPLOCATION("DIL2:03")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x04, DEF_STR( On  ) )
	PORT_DIPNAME( 0x08, 0x00, "DIL204" ) PORT_DIPLOCATION("DIL2:04")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x08, DEF_STR( On  ) )
	PORT_DIPNAME( 0x10, 0x00, "DIL205" ) PORT_DIPLOCATION("DIL2:05")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x10, DEF_STR( On  ) )
	PORT_DIPNAME( 0x20, 0x00, "DIL206" ) PORT_DIPLOCATION("DIL2:06")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x20, DEF_STR( On  ) )
	PORT_DIPNAME( 0x40, 0x00, "DIL207" ) PORT_DIPLOCATION("DIL2:07")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x40, DEF_STR( On  ) )
	PORT_DIPNAME( 0x80, 0x00, "DIL208" ) PORT_DIPLOCATION("DIL2:08")
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
	PORT_DIPNAME( 0x01, 0x00, "DIL101" ) PORT_DIPLOCATION("DIL1:01")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x01, DEF_STR( On  ) )
	PORT_DIPNAME( 0x02, 0x00, "DIL102" ) PORT_DIPLOCATION("DIL1:02")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x02, DEF_STR( On  ) )
	PORT_DIPNAME( 0x04, 0x00, "DIL103" ) PORT_DIPLOCATION("DIL1:03")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x04, DEF_STR( On  ) )
	PORT_DIPNAME( 0x08, 0x00, "DIL104" ) PORT_DIPLOCATION("DIL1:04")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x08, DEF_STR( On  ) )
	PORT_DIPNAME( 0x10, 0x00, "DIL105" ) PORT_DIPLOCATION("DIL1:05")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x10, DEF_STR( On  ) )
	PORT_DIPNAME( 0x20, 0x00, "DIL106" ) PORT_DIPLOCATION("DIL1:06")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x20, DEF_STR( On  ) )
	PORT_DIPNAME( 0x40, 0x00, "DIL107" ) PORT_DIPLOCATION("DIL1:07")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x40, DEF_STR( On  ) )
	PORT_DIPNAME( 0x80, 0x00, "Invert Alpha?" ) PORT_DIPLOCATION("DIL1:08")
	PORT_DIPSETTING(    0x00, DEF_STR( No ) )
	PORT_DIPSETTING(    0x80, DEF_STR( Yes  ) )

	PORT_START("DIL2")
	PORT_DIPNAME( 0x01, 0x00, "DIL201" ) PORT_DIPLOCATION("DIL2:01")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x01, DEF_STR( On  ) )
	PORT_DIPNAME( 0x02, 0x00, "DIL202" ) PORT_DIPLOCATION("DIL2:02")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x02, DEF_STR( On  ) )
	PORT_DIPNAME( 0x04, 0x00, "DIL203" ) PORT_DIPLOCATION("DIL2:03")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x04, DEF_STR( On  ) )
	PORT_DIPNAME( 0x08, 0x00, "DIL204" ) PORT_DIPLOCATION("DIL2:04")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x08, DEF_STR( On  ) )
	PORT_DIPNAME( 0x10, 0x00, "DIL205" ) PORT_DIPLOCATION("DIL2:05")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x10, DEF_STR( On  ) )
	PORT_DIPNAME( 0x20, 0x00, "DIL206" ) PORT_DIPLOCATION("DIL2:06")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x20, DEF_STR( On  ) )
	PORT_DIPNAME( 0x40, 0x00, "DIL207" ) PORT_DIPLOCATION("DIL2:07")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x40, DEF_STR( On  ) )
	PORT_DIPNAME( 0x80, 0x00, "DIL208" ) PORT_DIPLOCATION("DIL2:08")
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
	0,
	8,
	0x00
};

/* Common configurations */
static void mpu4_config_common(running_machine *machine)
{
	pia_config(0,&pia_ic3_intf);
	pia_config(1,&pia_ic4_intf);
	pia_config(2,&pia_ic5_intf);
	pia_config(3,&pia_ic6_intf);
	pia_config(4,&pia_ic7_intf);
	pia_config(5,&pia_ic8_intf);

	ic24_timer = timer_alloc(machine, ic24_timeout, NULL);
	/* setup 6840ptm */
	ptm6840_config(machine, 0, &ptm_ic2_intf );
}

static MACHINE_START( mpu4mod2 )
{
	mpu4_config_common(machine);
	pia_reset();

	serial_card_connected=0;
	mod_number=2;

	/* setup 8 mechanical meters */
	Mechmtr_init(8);

	/* setup 4 reels */
	stepper_config(0, &barcrest_reel_interface);
	stepper_config(1, &barcrest_reel_interface);
	stepper_config(2, &barcrest_reel_interface);
	stepper_config(3, &barcrest_reel_interface);

	/* setup the standard oki MSC1937 display */
	ROC10937_init(0, MSC1937,0);
}


/*
Characteriser (CHR)

I haven't been able to work out all the ways of finding the CHR data, but there must be a flag in the ROMs
somewhere to pick it out. As built, the CHR is a PAL which can perform basic bit manipulation according to an as yet unknown unique key. However, the programmers decided to best use this protection device in read/write/compare
cycles, storing almost the entire 'hidden' data table in the ROMs in plain sight. Only later rebuilds by BwB
avoided this 'feature' of the development kit, and as such, only low level access can defeat their protection.

This information has been used to generate the CHR tables loaded by the programs, until a key can be determined.

For most Barcrest games, the following method was used:

To calculate the values necessary to program the CHR, we must first find the version string, taking
the example of an AWP called Club Celebration, this starts at 0xff28 and terminates at 0xff2f.
0xff2f then represents the CHR address.
For some reason, the tables always seem to start and end with '00 00'.

From that point on, every word represents a call and response pair, until we have generated 8 8 byte rows of data.

The call values appear to be fixed, so it would be possible to make a switch statement, but I believe the emulated
behaviour to be more likely.

The initial 'PALTEST' routine as found in the Barcrest programs simply writes the first 'call' to the CHR space,
to read back the 'response'. There is no attempt to alter the order or anything else, just
a simple runthrough of the entire data table. The only 'catch' in this is to note that the CHR chip always scans
through the table starting at the last accessed data value, unless 00 is used to reset to the beginning.

However, a final 8 byte row, that controls the lamp matrix is not tested - to date, no-one outside of Barcrest knows
how this is generated, and currently trial and error is the only sensible method. It is noted that the default,
of all 00, is sometimes the correct answer, particularly in non-Barcrest use of the CHR chip, though when used normally,
there are again fixed call values.
*/

static WRITE8_HANDLER( characteriser_w )
{
	UINT8 x;
	int call=data;
	LOG_CHR_FULL(("%04x Characteriser write offset %02X data %02X", cpu_get_previouspc(space->cpu),offset,data));
	if (offset == 0)
	{
		if (call == 0)
		{
			prot_col = 0;
		}
		else
		{
			for (x = prot_col; x < 64; x++)
			{
				if	(MPU4_chr_lut[(x)] == call)
				{
					prot_col = x;
					LOG_CHR(("Characteriser find column %02X\n",prot_col));
					LOG_CHR(("Characteriser find data %02X\n",MPU4_chr_data[prot_col]));
					break;
				}
			}
		}
	}
	else if (offset == 2)
	{
		LOG_CHR(("Characteriser write 2 data %02X\n",data));
		for (x = lamp_col; x < 16; x++)
		{
			if	(MPU4_chr_lut[(64+x)] == call)
			{
				lamp_col = x;
				LOG_CHR(("Characteriser find column %02X\n",lamp_col));
				break;
			}
			if (lamp_col > 7)
			{
				lamp_col = 0;
			}
		}
	}
}


static READ8_HANDLER( characteriser_r )
{
	LOG_CHR(("Characteriser read offset %02X \n",offset));
	LOG_CHR_FULL(("%04x Characteriser read offset %02X", cpu_get_previouspc(space->cpu),offset));
	if (offset == 0)
	{
		LOG_CHR(("Characteriser read data %02X \n",MPU4_chr_data[prot_col]));
		return MPU4_chr_data[prot_col];
	}
	if (offset == 3)
	{
		LOG_CHR(("Characteriser read data %02X \n",MPU4_chr_data[lamp_col+64]));
		return MPU4_chr_data[lamp_col+64];
	}
	return 0;
}


/* generate a 50 Hz signal (based on an RC time) */
static TIMER_DEVICE_CALLBACK( gen_50hz )
{
	/* Although reported as a '50Hz' signal, the fact that both rising and
    falling edges of the pulse are used, the timer actually gives a 100Hz
    oscillating signal.*/
	signal_50hz = signal_50hz?0:1;

	pia_set_input_ca1(1,signal_50hz);	/* signal is connected to IC4 CA1 */
}


static ADDRESS_MAP_START( mod2_memmap, ADDRESS_SPACE_PROGRAM, 8 )
	AM_RANGE(0x0000, 0x07ff) AM_RAM AM_BASE(&generic_nvram) AM_SIZE(&generic_nvram_size)
	AM_RANGE(0x0800, 0x0810) AM_READWRITE(characteriser_r,characteriser_w)

	AM_RANGE(0x0850, 0x0850) AM_READWRITE(bankswitch_r,bankswitch_w)	/* write bank (rom page select) */

/*  AM_RANGE(0x08E0, 0x08E7) AM_READWRITE(68681_duart_r,68681_duart_w) */

	AM_RANGE(0x0900, 0x0907) AM_READWRITE(ptm6840_0_r,ptm6840_0_w)  /* 6840PTM */

	AM_RANGE(0x0A00, 0x0A03) AM_READWRITE(pia_0_r,pia_0_w)	  	/* PIA6821 IC3 */
	AM_RANGE(0x0B00, 0x0B03) AM_READWRITE(pia_1_r,pia_1_w)	  	/* PIA6821 IC4 */
	AM_RANGE(0x0C00, 0x0C03) AM_READWRITE(pia_2_r,pia_2_w)	  	/* PIA6821 IC5 */
	AM_RANGE(0x0D00, 0x0D03) AM_READWRITE(pia_3_r,pia_3_w)		/* PIA6821 IC6 */
	AM_RANGE(0x0E00, 0x0E03) AM_READWRITE(pia_4_r,pia_4_w)		/* PIA6821 IC7 */
	AM_RANGE(0x0F00, 0x0F03) AM_READWRITE(pia_5_r,pia_5_w)		/* PIA6821 IC8 */

	AM_RANGE(0x1000, 0xffff) AM_READ(SMH_BANK1)	/* 64k  paged ROM (4 pages)  */
ADDRESS_MAP_END

static const ay8910_interface ay8910_config =
{
	AY8910_SINGLE_OUTPUT,
	{820,0,0},
	NULL,
	NULL,
	NULL,
	NULL
};


/* machine driver for MOD 2 board */
static MACHINE_DRIVER_START( mpu4mod2 )

	MDRV_MACHINE_START(mpu4mod2)
	MDRV_MACHINE_RESET(mpu4)
	MDRV_CPU_ADD("main", M6809, MPU4_MASTER_CLOCK/4)
	MDRV_CPU_PROGRAM_MAP(mod2_memmap,0)

	MDRV_TIMER_ADD_PERIODIC("50hz",gen_50hz, HZ(100))

	MDRV_SPEAKER_STANDARD_MONO("mono")
	MDRV_SOUND_ADD("ay8913",AY8913, MPU4_MASTER_CLOCK/4)
	MDRV_SOUND_CONFIG(ay8910_config)
	MDRV_SOUND_ROUTE(ALL_OUTPUTS, "mono", 1.0)

	MDRV_NVRAM_HANDLER(generic_0fill)

	MDRV_DEFAULT_LAYOUT(layout_mpu4)
MACHINE_DRIVER_END


#include "drivers/mpu4drvr.c"
