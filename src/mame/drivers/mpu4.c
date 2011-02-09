/***********************************************************************************************************
  Barcrest MPU4 highly preliminary driver by J.Wallace, and Anonymous.

  This is the core driver, no video specific stuff should go in here.
  This driver holds all the mechanical games.

     01-2011: Adding the missing 'OKI' sound card, and documented it.
     05-2009: Miscellaneous lamp fixes, based on data from FME Forever.
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

See http://agemame.mameworld.info/techinfo/mpu4.php for Information.

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
 0880      |R/W| D D D D D D D D | PIA6821 on soundboard (Oki MSM6376@16MHz)
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

TODO: - Fix lamp timing, MAME doesn't update fast enough to see everything
      - Distinguish door switches using manual
      - Complete stubs for hoppers (needs slightly better 68681 emulation, and new 'hoppers' device)
      - Any reel using the remote meter drive (CB2) slips backwards due to timing mismatches, a better method
      is needed to combine the data. This eventually leads to spin alarms i.e Flo's move in Great Escape
      - Add a BwB game with characteriser.
***********************************************************************************************************/
#include "emu.h"
#include "machine/6821pia.h"
#include "machine/6840ptm.h"
#include "machine/nvram.h"

#include "cpu/m6809/m6809.h"
#include "sound/ay8910.h"
#include "sound/okim6376.h"
#include "sound/2413intf.h"
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
#define LOG_SS(x)	do { if (1) logerror x; } while (0)

#include "video/awpvid.h"		//Fruit Machines Only
#include "connect4.lh"
#include "gamball.lh"
#include "mpu4.lh"
#include "mpu4ext.lh"
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
static int aux1_input;
static int aux2_input;
static int IC23G1;
static int IC23G2A;
static int IC23G2B;
static int IC23GC;
static int IC23GB;
static int IC23GA;
static int prot_col;
static int lamp_col;
static int reel_flag;
static int ic23_active;
static int led_lamp;
static int link7a_connected;
static emu_timer *ic24_timer;
static TIMER_CALLBACK( ic24_timeout );

static int expansion_latch;// OKI MOD 4 and above only (PIA on Gamecard)
static int global_volume;// OKI MOD 4 and above only (PIA on Gamecard)
/* 32 multiplexed inputs - but a further 8 possible per AUX.
Two connectors 'orange' (sampled every 8ms) and 'black' (sampled every 16ms)
Each connector carries two banks of eight inputs and two enable signals */
static int	  input_strobe;	  /* IC23 74LS138 A = CA2 IC7, B = CA2 IC4, C = CA2 IC8 */
static UINT8  lamp_strobe,lamp_strobe2;
static UINT8  lamp_data;
static UINT8  ay_data;

static UINT8 Lamps[224];		/* 128 multiplexed lamps + 96 extension */
static int optic_pattern;
static int active_reel;
static int remote_meter;

static UINT8 reel_mux_table[8]= {0,4,2,6,1,5,3,7};//include 7, although I don't think it's used
static UINT8 reel_mux_table7[8]= {3,1,5,6,4,2,0,7};

static int reel_mux;		/* added support for up to 8 reels */
#define STANDARD_REEL  0	/* As originally designed 3/4 reels*/
#define FIVE_REEL_5TO8 1	/* Interfaces to meter port, allows some mechanical metering, but there is significant 'bounce' in the extra reel*/
#define FIVE_REEL_8TO5 2	/* Mounted backwards for space reasons, but different board*/
#define FIVE_REEL_3TO6 3	/* Connected to the centre of the meter connector, taking up meters 3 to 6 */
#define SIX_REEL_1TO8  4	/* Two reels on the meter drives*/
#define SIX_REEL_5TO8  5	/* Like FIVE_REEL_5TO8, but with an extra reel elsewhere*/
#define SEVEN_REEL     6	/* Mainly club machines, significant reworking of reel hardware*/

static int lamp_extender;/* Plug in boards that add additional lamps and leds via the AUX ports. */
static int last_b7;
static int last_latch;
#define NO_EXTENDER			0	/* As originally designed */
#define SMALL_CARD			1
#define LARGE_CARD_A		2 //96 Lamps
#define LARGE_CARD_B		3 // 96 Lamps, 16 LEDs - as used by BwB
#define LARGE_CARD_C		4 //Identical to B, no built in LED support

static int lamp_sense;
static int card_live;

static int led_extender;	/* Plug in boards that add leds via the AUX ports. */
#define CARD_A			1
#define CARD_B			2
#define CARD_C			3

static int hopper =0;
#define TUBES				0
#define HOPPER_DUART_A		1
#define HOPPER_DUART_B		2
#define HOPPER_DUART_C		3
#define HOPPER_NONDUART_A	4
#define HOPPER_NONDUART_B	5
/* Lookup table for CHR data */

struct mpu4_chr_table
{
	UINT8 call;
	UINT8 response;
};

static const mpu4_chr_table* mpu4_current_chr_table;

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

static UINT8 led_segs[40];//8 regular, up to 32 on extender cards


/* Process lamp and LED data for output system */
static void mpu4_draw_led(UINT8 id, UINT8 value)
{
	output_set_digit_value(id,value);
}


static void draw_lamps(void)
{
	int i,j;

	for (i = 0; i < 8; i++)
	{
		output_set_lamp_value((8*input_strobe)+i, (Lamps[(8*input_strobe)+i]));
		output_set_lamp_value((8*input_strobe)+i+64, (Lamps[(8*input_strobe)+i+64]));
	}
	if (lamp_extender)
	{
		for (j = 0; j < 6; j++)
		{
			output_set_lamp_value((6*input_strobe)+128+j, (Lamps[(6*input_strobe)+128+j]));
			output_set_lamp_value((6*input_strobe)+176+j, (Lamps[(6*input_strobe)+176+j]));
		}
	}
}

static void update_lamps(void)
{
	int i,j;

	for (i = 0; i < 8; i++)
	{
		Lamps[(8*input_strobe)+i]    = (lamp_strobe  & (1 << i)) != 0;
		Lamps[(8*input_strobe)+i+64] = (lamp_strobe2 & (1 << i)) != 0;
	}

	if (led_lamp)
	{
		/* Some games (like Connect 4) use 'programmable' LED displays, built from light display lines. */
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
	mpu4_draw_led(input_strobe, led_segs[input_strobe]);
	if (led_extender)
	{
		for (j = 0; j < 5; j++)
		{
			mpu4_draw_led(((8*input_strobe)+8+j), led_segs[((8*input_strobe)+8+j)]);
		}
	}
}

static void lamp_extend_small(int data)
{
	int lamp_strobe_ext,column,i;
	column = data & 0x07;

	lamp_strobe_ext = 0x1f - ((data & 0xf8) >> 3);
	if ( lamp_strobe_ext )
	{
		for (i = 0; i < 5; i++)
		{
			Lamps[(5*column)+i+128]    = (lamp_strobe_ext  & (1 << i)) != 0;
		}
    }
}

static void lamp_extend_largea(int data,int column,int active)
{
	int lampbase,i,byte7;

	lamp_sense = 0;
	byte7 = data & 0x80;
	if ( byte7 != last_b7 )
	{
		if ( byte7 )
		{
			lampbase = 128 + column * 8;
		}
		else
		{
			lampbase = 192 + column * 8;
		}
		if ( data & 0x3f )
		{
	        lamp_sense = 1;
		}
		if ( active )
		{
			for (i = 0; i < 6; i++)
			{
				Lamps[(lampbase)+i]    = (data  & (1 << i)) != 0;
			}
		}
    }
    last_b7 = byte7;
}

static void lamp_extend_largebc(int data,int column,int active)
{
	int lampbase,i,byte7;

	lamp_sense = 0;
	byte7 = data & 0x80;
	if ( byte7 != last_b7 )
	{
		card_live = 1;
	    if ( byte7 )
		{
			lampbase = 128 + column * 8;
		}
		else
		{
			lampbase = 192 + column * 8;
		}
		if ( data & 0x3f )
		{
			lamp_sense = 1;
		}
    	if ( active )
		{
			for (i = 0; i < 6; i++)
			{
				Lamps[(lampbase)+i]    = (data  & (1 << i)) != 0;
			}
		}
	last_b7 = byte7;
	}
	else
	{
		card_live = 0;
	}
}

static void led_write_latch(int latch, int data, int column)
{
	int diff;

	diff = (latch ^ last_latch) & latch;
	column = 7 - column;
	data = ~data;//inverted?

	if ( diff & 1 )
	{
		led_segs[column] = data;
    }
	if ( diff & 2 )
	{
		led_segs[8+column] = data;
	}
	if ( diff & 4 )
	{
		led_segs[16+column] = data;
	}
	if ( diff & 8 )
	{
		led_segs[24+column] = data;
	}
	if ( diff & 16 )
	{
		led_segs[32+column] = data;
	}
	last_latch = diff;
}


static void update_meters(void)
{
	int meter;
	int data = ((mmtr_data & 0x7f) | remote_meter);
	switch (reel_mux)
	{
		case STANDARD_REEL:
		// Change nothing
		break;
		case FIVE_REEL_5TO8:
		stepper_update(4, (data >> 4));
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
	}

	MechMtr_update(7, (data & 0x80));
	for (meter = 0; meter < 4; meter ++)
	{
		MechMtr_update(meter, (data & (1 << meter)));
	}
	if (reel_mux == STANDARD_REEL)
	{
		for (meter = 4; meter < 7; meter ++)
		{
			MechMtr_update(meter, (data & (1 << meter)));
		}
	}
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
		UINT8 *rom = machine->region("maincpu")->base();

		memory_configure_bank(machine, "bank1", 0, 8, &rom[0x01000], 0x10000);

		memory_set_bank(machine, "bank1",0);
		machine->device("maincpu")->reset();
	}

}


/* 6809 IRQ handler */
static WRITE_LINE_DEVICE_HANDLER( cpu0_irq )
{
	device_t *pia3 = device->machine->device("pia_ic3");
	device_t *pia4 = device->machine->device("pia_ic4");
	device_t *pia5 = device->machine->device("pia_ic5");
	device_t *pia6 = device->machine->device("pia_ic6");
	device_t *pia7 = device->machine->device("pia_ic7");
	device_t *pia8 = device->machine->device("pia_ic8");
	device_t *ptm2 = device->machine->device("ptm_ic2");

	/* The PIA and PTM IRQ lines are all connected to a common PCB track, leading directly to the 6809 IRQ line. */
	int combined_state = pia6821_get_irq_a(pia3) | pia6821_get_irq_b(pia3) |
						 pia6821_get_irq_a(pia4) | pia6821_get_irq_b(pia4) |
						 pia6821_get_irq_a(pia5) | pia6821_get_irq_b(pia5) |
						 pia6821_get_irq_a(pia6) | pia6821_get_irq_b(pia6) |
						 pia6821_get_irq_a(pia7) | pia6821_get_irq_b(pia7) |
						 pia6821_get_irq_a(pia8) | pia6821_get_irq_b(pia8) |
						 ptm6840_get_irq(ptm2);

	if (!link7a_connected) //7B = IRQ, 7A = FIRQ, both = NMI
	{
		cputag_set_input_line(device->machine, "maincpu", M6809_IRQ_LINE, combined_state ? ASSERT_LINE : CLEAR_LINE);
		LOG(("6809 int%d \n", combined_state));
	}
	else
	{
		cputag_set_input_line(device->machine, "maincpu", M6809_FIRQ_LINE, combined_state ? ASSERT_LINE : CLEAR_LINE);
		LOG(("6809 fint%d \n", combined_state));
	}
}

/* Bankswitching */
static WRITE8_HANDLER( bankswitch_w )
{
	memory_set_bank(space->machine, "bank1",data & 0x07);
}


static READ8_HANDLER( bankswitch_r )
{
	return memory_get_bank(space->machine, "bank1");
}


/* IC2 6840 PTM handler */
static WRITE8_DEVICE_HANDLER( ic2_o1_callback )
{
	ptm6840_set_c2(device, 0, data);

	/* copy output value to IC2 c2
    this output is the clock for timer2 */
	/* 1200Hz System interrupt timer */
}


static WRITE8_DEVICE_HANDLER( ic2_o2_callback )
{
	device_t *pia = device->machine->device("pia_ic3");
	pia6821_ca1_w(pia, data); /* copy output value to IC3 ca1 */
	/* the output from timer2 is the input clock for timer3 */
	/* miscellaneous interrupts generated here */
	ptm6840_set_c3(device, 0, data);
}


static WRITE8_DEVICE_HANDLER( ic2_o3_callback )
{
	/* the output from timer3 is used as a square wave for the alarm output
    and as an external clock source for timer 1! */
	/* also runs lamp fade */
	ptm6840_set_c1(device, 0, data);
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
	LOG_IC3(("%s: IC3 PIA Port A Set to %2x (lamp strobes 1 - 9)\n", device->machine->describe_context(),data));

	if(ic23_active)
	{
		lamp_strobe = data;
	}
}


static WRITE8_DEVICE_HANDLER( pia_ic3_portb_w )
{
	LOG_IC3(("%s: IC3 PIA Port B Set to %2x  (lamp strobes 10 - 17)\n", device->machine->describe_context(),data));

	if(ic23_active)
	{
		lamp_strobe2 = data;
	}
}


static WRITE_LINE_DEVICE_HANDLER( pia_ic3_ca2_w )
{
	LOG_IC3(("%s: IC3 PIA Write CA2 (alpha data), %02X\n", device->machine->describe_context(),state));

	alpha_data_line = state;
	ROC10937_draw_16seg(0);
}


static WRITE_LINE_DEVICE_HANDLER( pia_ic3_cb2_w )
{
	LOG_IC3(("%s: IC3 PIA Write CB (alpha reset), %02X\n",device->machine->describe_context(),state));

	if ( state ) ROC10937_reset(0);
	ROC10937_draw_16seg(0);
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
			ic24_timer->adjust(attotime::from_double(duration));
		}
	}
}


static TIMER_CALLBACK( ic24_timeout )
{
	ic23_active=0;
	ic24_output(1);
}


/* IC4, 7 seg leds, 50Hz timer reel sensors, current sensors */
static WRITE8_DEVICE_HANDLER( pia_ic4_porta_w )
{
	if(ic23_active)
	{
		if (((lamp_extender == NO_EXTENDER)||(lamp_extender == SMALL_CARD)||(lamp_extender == LARGE_CARD_C))&& (led_extender == NO_EXTENDER))
		led_segs[input_strobe] = data;
	}
}

static WRITE8_DEVICE_HANDLER( pia_ic4_portb_w )
{
	if (reel_mux)
	{
		/* A write here connects one reel (and only one)
        to the optic test circuit. This allows 8 reels
        to be supported instead of 4. */
		if (reel_mux == SEVEN_REEL)
		{
			active_reel= reel_mux_table7[(data >> 4) & 0x07];
		}
		else
		active_reel= reel_mux_table[(data >> 4) & 0x07];
	}
}


static READ8_DEVICE_HANDLER( pia_ic4_portb_r )
{
	if ( serial_data )
	{
		ic4_input_b |=  0x80;
		pia6821_cb1_w(device, 1);
	}
	else
	{
		ic4_input_b &= ~0x80;
		pia6821_cb1_w(device, 0);
	}

	if (!reel_mux)
	{
		if ( optic_pattern & 0x01 ) ic4_input_b |=  0x40; /* reel A tab */
		else                        ic4_input_b &= ~0x40;

		if ( optic_pattern & 0x02 ) ic4_input_b |=  0x20; /* reel B tab */
		else                        ic4_input_b &= ~0x20;

		if ( optic_pattern & 0x04 ) ic4_input_b |=  0x10; /* reel C tab */
		else                        ic4_input_b &= ~0x10;

		if ( optic_pattern & 0x08 ) ic4_input_b |=  0x08; /* reel D tab */
		else                        ic4_input_b &= ~0x08;

	}
	else
	{
		if (stepper_optic_state(active_reel))
		{
			ic4_input_b |=  0x08;
		}
		else
		{
			ic4_input_b &= ~0x08;
		}
	}
	if ( signal_50hz )			ic4_input_b |=  0x04; /* 50 Hz */
	else	                    ic4_input_b &= ~0x04;

	if (ic4_input_b & 0x02)
	{
		ic4_input_b &= ~0x02;
	}
	else
	{
		ic4_input_b |= 0x02; //Pulse the overcurrent line with every read to show the CPU each lamp has lit
	}
	#ifdef UNUSED_FUNCTION
	if ( lamp_undercurrent ) ic4_input_b |= 0x01;
	#endif

	LOG_IC3(("%s: IC4 PIA Read of Port B %x\n",device->machine->describe_context(),ic4_input_b));
	return ic4_input_b;
}


static WRITE_LINE_DEVICE_HANDLER( pia_ic4_ca2_w )
{
	LOG_IC3(("%s: IC4 PIA Write CA (input MUX strobe /LED B), %02X\n", device->machine->describe_context(),state));

	IC23GB = state;
	ic23_update();
}

static WRITE_LINE_DEVICE_HANDLER( pia_ic4_cb2_w )
{
	LOG_IC3(("%s: IC4 PIA Write CA (input MUX strobe /LED B), %02X\n", device->machine->describe_context(),state));
	reel_flag=state;
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
	if (lamp_extender == LARGE_CARD_A)
	{
		if (lamp_sense && ic23_active)
		{
			aux1_input |= 0x40;
		}
		else
		{
			aux1_input &= ~0x40; //Pulse the overcurrent line with every read to show the CPU each lamp has lit
		}
	}
	if (hopper == HOPPER_NONDUART_A)
	{
/*      if (hopper1_active)
        {
            aux1_input |= 0x04;
        }
        else
        {
            aux1_input &= ~0x04;
        }*/
	}
	LOG(("%s: IC5 PIA Read of Port A (AUX1)\n",device->machine->describe_context()));

	return input_port_read(device->machine, "AUX1")|aux1_input;
}

static WRITE8_DEVICE_HANDLER( pia_ic5_porta_w )
{
	device_t *pia_ic4 = device->machine->device("pia_ic5");
	if (hopper == HOPPER_NONDUART_A)
	{
		//hopper1_drive_sensor(data&0x10);
	}
	switch (lamp_extender)
	{
	case NO_EXTENDER:
		if (led_extender == CARD_B)
		{
			led_write_latch(data & 0x1f, pia6821_get_output_a(pia_ic4),input_strobe);
		}
		else if ((led_extender != CARD_A)||(led_extender != NO_EXTENDER))
		{
			led_segs[(input_strobe+8)] = data;
		}
		break;
	case SMALL_CARD:
		if(ic23_active)
		{
			lamp_extend_small(data);
		}
		break;
	case LARGE_CARD_A:
		lamp_extend_largea(data,input_strobe,ic23_active);
		break;
	case LARGE_CARD_B:
		lamp_extend_largebc(data,input_strobe,ic23_active);
		if ((ic23_active) && card_live)
		{
			led_segs[(8*(last_b7 >>7))+input_strobe] = (~data);
		}
		break;
	case LARGE_CARD_C:
		lamp_extend_largebc(data,input_strobe,ic23_active);
		break;
	}
	if (reel_mux == SIX_REEL_5TO8)
	{
		stepper_update(4, data&0x0F);
		stepper_update(5, (data >> 4)&0x0F);
		awp_draw_reel(4);
		awp_draw_reel(5);
	}
	else
	if (reel_mux == SEVEN_REEL)
	{
		stepper_update(1, data&0x0F);
		stepper_update(2, (data >> 4)&0x0F);
		awp_draw_reel(1);
		awp_draw_reel(2);
	}

}

static WRITE8_DEVICE_HANDLER( pia_ic5_portb_w )
{
	if (hopper == HOPPER_NONDUART_B)
	{
		//hopper1_drive_motor(data &0x01)
		//hopper1_drive_sensor(data &0x08)
	}
	if (led_extender == CARD_A)
	{
		// led_write_latch(data & 0x07, pia_get_output_a(pia_ic4),input_strobe)
	}
}
static READ8_DEVICE_HANDLER( pia_ic5_portb_r )
{
	device_t *pia_ic5 = device->machine->device("pia_ic5");
	if (hopper == HOPPER_NONDUART_B)
	{/*
        if (hopper1_active)
        {
            aux2_input |= 0x08;
        }
        else
        {
            aux2_input &= ~0x08;
        }*/
	}

	LOG(("%s: IC5 PIA Read of Port B (coin input AUX2)\n",device->machine->describe_context()));
	coin_lockout_w(device->machine, 0, (pia6821_get_output_b(pia_ic5) & 0x01) );
	coin_lockout_w(device->machine, 1, (pia6821_get_output_b(pia_ic5) & 0x02) );
	coin_lockout_w(device->machine, 2, (pia6821_get_output_b(pia_ic5) & 0x04) );
	coin_lockout_w(device->machine, 3, (pia6821_get_output_b(pia_ic5) & 0x08) );
	return input_port_read(device->machine, "AUX2") | aux2_input;
}


static WRITE_LINE_DEVICE_HANDLER( pia_ic5_ca2_w )
{
	LOG(("%s: IC5 PIA Write CA2 (Serial Tx) %2x\n",device->machine->describe_context(),state));
	serial_data = state;
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
	if (!pia6821_get_output_cb2(device))
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
				device_t *pia_ic6 = device->machine->device("pia_ic6");
				LOG(("AY8913 address = %d \n",pia6821_get_output_a(pia_ic6)&0x0f));
				break;
			}
			case 0x02:
			{/* CA2 = 0 CB2 = 1? : Write to selected PSG register and write data to Port A */
				device_t *pia_ic6 = device->machine->device("pia_ic6");
				device_t *ay = device->machine->device("ay8913");
				ay8910_data_w(ay, 0, pia6821_get_output_a(pia_ic6));
				LOG(("AY Chip Write \n"));
				break;
			}
			case 0x03:
			{/* CA2 = 1 CB2 = 1? : The register will now be selected and the user can read from or write to it.
             The register will remain selected until another is chosen.*/
				device_t *pia_ic6 = device->machine->device("pia_ic6");
				device_t *ay = device->machine->device("ay8913");
				ay8910_address_w(ay, 0, pia6821_get_output_a(pia_ic6));
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


static WRITE_LINE_DEVICE_HANDLER( pia_ic5_cb2_w )
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
	LOG(("%s: IC6 PIA Port B Set to %2x (Reel A and B)\n", device->machine->describe_context(),data));

	if (reel_mux == SEVEN_REEL)
	{
		stepper_update(3, data&0x0F);
		stepper_update(4, (data >> 4)&0x0F);
		awp_draw_reel(3);
		awp_draw_reel(4);
	}
	else
	{
		stepper_update(0, data & 0x0F );
		stepper_update(1, (data>>4) & 0x0F );
		awp_draw_reel(0);
		awp_draw_reel(1);
	}

	if (reel_flag && (reel_mux == STANDARD_REEL))
	{
		if ( stepper_optic_state(0) ) optic_pattern |=  0x01;
		else                          optic_pattern &= ~0x01;

		if ( stepper_optic_state(1) ) optic_pattern |=  0x02;
		else                          optic_pattern &= ~0x02;
	}
}


static WRITE8_DEVICE_HANDLER( pia_ic6_porta_w )
{
	LOG(("%s: IC6 PIA Write A %2x\n", device->machine->describe_context(),data));
	if (mod_number <4)
	{
		ay_data = data;
	    update_ay(device);
	}
}


static WRITE_LINE_DEVICE_HANDLER( pia_ic6_ca2_w )
{
	LOG(("%s: IC6 PIA write CA2 %2x (AY8913 BC1)\n", device->machine->describe_context(),state));
	if (mod_number <4)
	{
		if ( state ) ay8913_address |=  0x01;
		else         ay8913_address &= ~0x01;
		update_ay(device);
	}
}


static WRITE_LINE_DEVICE_HANDLER( pia_ic6_cb2_w )
{
	LOG(("%s: IC6 PIA write CB2 %2x (AY8913 BCDIR)\n", device->machine->describe_context(),state));
	if (mod_number <4)
	{
		if ( state ) ay8913_address |=  0x02;
		else         ay8913_address &= ~0x02;
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
	LOG(("%s: IC7 PIA Port A Set to %2x (Reel C and D)\n", device->machine->describe_context(),data));
	if (reel_mux == SEVEN_REEL)
	{
		stepper_update(5, data&0x0F);
		stepper_update(6, (data >> 4)&0x0F);
		awp_draw_reel(5);
		awp_draw_reel(6);
	}
	else
	{
		stepper_update(2, data & 0x0F );
		stepper_update(3, (data>>4) & 0x0F );
		awp_draw_reel(2);
		awp_draw_reel(3);
	}

	if (reel_flag && (reel_mux == STANDARD_REEL))
	{
		if ( stepper_optic_state(2) ) optic_pattern |=  0x04;
		else                          optic_pattern &= ~0x04;
		if ( stepper_optic_state(3) ) optic_pattern |=  0x08;
		else                          optic_pattern &= ~0x08;
	}
}

static WRITE8_DEVICE_HANDLER( pia_ic7_portb_w )
{
	if (hopper == HOPPER_DUART_A)
	{
		//duart write data
	}
	else if (hopper == HOPPER_NONDUART_A)
	{
		//hoppr1_drive_motor(data & 0x10);
	}

    mmtr_data = data;
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
	LOG(("%s: IC7 PIA write CA2 %2x (input strobe bit 0 / LED A)\n", device->machine->describe_context(),state));

	IC23GA = state;
	ic24_setup();
	ic23_update();
}

static WRITE_LINE_DEVICE_HANDLER( pia_ic7_cb2_w )
{
	remote_meter = state?0x80:0x00;
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
	static const char *const portnames[] = { "ORANGE1", "ORANGE2", "BLACK1", "BLACK2", "ORANGE1", "ORANGE2", "DIL1", "DIL2" };
	device_t *pia_ic5 = device->machine->device("pia_ic5");

	LOG_IC8(("%s: IC8 PIA Read of Port A (MUX input data)\n", device->machine->describe_context()));
/* The orange inputs are polled twice as often as the black ones, for reasons of efficiency.
   This is achieved via connecting every input line to an AND gate, thus allowing two strobes
   to represent each orange input bank (strobes are active low). */
	pia6821_cb1_w(pia_ic5, (input_port_read(device->machine, "AUX2") & 0x80));
	return input_port_read(device->machine, portnames[input_strobe]);
}


static WRITE8_DEVICE_HANDLER( pia_ic8_portb_w )
{
	if (hopper == HOPPER_DUART_B)
	{
//      duart.drive_sensor(data & 0x04, data & 0x01, 0, 0);
	}
	else if (hopper == HOPPER_DUART_C)
	{
//      duart.drive_sensor(data & 0x04, data & 0x01, data & 0x04, data & 0x02);
	}
	int i;
	LOG_IC8(("%s: IC8 PIA Port B Set to %2x (OUTPUT PORT, TRIACS)\n", device->machine->describe_context(),data));
	for (i = 0; i < 8; i++)
	{
		output_set_indexed_value("triac", i, data & (1 << i));
	}
}

static WRITE_LINE_DEVICE_HANDLER( pia_ic8_ca2_w )
{
	LOG_IC8(("%s: IC8 PIA write CA2 (input_strobe bit 2 / LED C) %02X\n", device->machine->describe_context(), state & 0xFF));

	IC23GC = state;
	ic23_update();
}


static WRITE_LINE_DEVICE_HANDLER( pia_ic8_cb2_w )
{
	LOG_IC8(("%s: IC8 PIA write CB2 (alpha clock) %02X\n", device->machine->describe_context(), state & 0xFF));

	if ( !alpha_clock && (state) )
	{
		ROC10937_shift_data(0, alpha_data_line&0x01?0:1);
	}
	alpha_clock = state;
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


// Sampled sound card, using a PIA and PTM for timing and data handling
static WRITE8_DEVICE_HANDLER( pia_gb_porta_w )
{
	device_t *msm6376 = device->machine->device("msm6376");
	LOG_SS(("%s: GAMEBOARD: PIA Port A Set to %2x\n", device->machine->describe_context(),data));
	okim6376_w(msm6376, 0, data);
}

static WRITE8_DEVICE_HANDLER( pia_gb_portb_w )
{
	int changed = expansion_latch^data;

	LOG_SS(("%s: GAMEBOARD: PIA Port A Set to %2x\n", device->machine->describe_context(),data));

	expansion_latch = data;

	if ( changed & 0x20)
	{ // digital volume clock line changed
		if ( !(data & 0x20) )
		{ // changed from high to low,
			if ( !(data & 0x10) )
			{
				if ( global_volume < 31 ) global_volume++; //steps unknown
			}
			else
			{
				if ( global_volume > 0  ) global_volume--;
			}

			{
//              float percent = (32-global_volume)/32.0; //volume_override?1.0:(32-global_volume)/32.0;
//              LOG(("GAMEBOARD: OKI volume %f \n",percent));
			}
		}
	}
}
static READ8_DEVICE_HANDLER( pia_gb_portb_r )
{
	device_t *msm6376 = device->machine->device("msm6376");
	LOG_SS(("%s: GAMEBOARD: PIA Read of Port B\n",device->machine->describe_context()));
	//
	// b7, 1 = OKI ready, 0 = OKI busy
	// b5, vol clock
	// b4, 1 = Vol down, 0 = Vol up
	//

	return okim6376_r(msm6376,0);
}

static WRITE_LINE_DEVICE_HANDLER( pia_gb_ca2_w )
{
	LOG_SS(("%s: GAMEBOARD: OKI RESET data = %02X\n", device->machine->describe_context(), state));

//  reset line
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
	DEVCB_NULL,		/* port CB2 out */
	DEVCB_NULL,		/* IRQA */
	DEVCB_NULL		/* IRQB */
};

//Sampled sound timer
/*
FIXME
The MSM6376 sound chip is configured in a slightly strange way, to enable dynamic
sample rate changes (8Khz, 10.6 Khz, 16 KHz) by varying the clock.
According to the BwB programmer's guide, the formula is:
MSM6376 clock frequency:-
freq = (1720000/((t3L+1)(t3H+1)))*[(t3H(T3L+1)+1)/(2(t1+1))]
where [] means rounded up integer,
t3L is the LSB of Clock 3,
t3H is the MSB of Clock 3,
and t1 is the figure added to clock 1.

The sample speed divisor is f/300
*/

//O3 -> G1  O1 -> c2 o2 -> c1
static WRITE8_DEVICE_HANDLER( ic3ss_o1_callback )
{
	ptm6840_set_c2(device, 0, data);
}


static WRITE8_DEVICE_HANDLER( ic3ss_o2_callback )//Generates 'beep' tone
{
	ptm6840_set_c1(device, 0, data);//?
}


static WRITE8_DEVICE_HANDLER( ic3ss_o3_callback )
{
	ptm6840_set_g1(device, 0, data); /* this output is the clock for timer1 */
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
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_OTHER)	PORT_NAME("20")
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

static INPUT_PORTS_START( gamball )
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
	PORT_DIPNAME( 0x10, 0x00, "AUX105" )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x10, DEF_STR( On  ) )
	PORT_DIPNAME( 0x20, 0x00, "AUX106" )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x20, DEF_STR( On  ) )
	PORT_DIPNAME( 0x40, 0x00, "AUX107" )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x40, DEF_STR( On  ) )
	PORT_DIPNAME( 0x80, 0x00, "AUX108" )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x80, DEF_STR( On  ) )

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
static const stepper_interface barcrest_opto1_interface =
{
	BARCREST_48STEP_REEL,
	4,
	12,
	0x00
};

/* Common configurations */
static void mpu4_config_common(running_machine *machine)
{
	ic24_timer = machine->scheduler().timer_alloc(FUNC(ic24_timeout));
}

static void mpu4_config_common_reels(running_machine *machine,int reels)
{
	int n;
	/* setup n default 96 half step reels, using the standard optic flag */
	for ( n = 0; n < reels; n++ )
	{
		stepper_config(machine, n, &barcrest_reel_interface);
	}
	awp_reel_setup();
}

static MACHINE_START( mpu4mod2 )
{
	mpu4_config_common(machine);

	link7a_connected=0;
	mod_number=2;

	/* setup 8 mechanical meters */
	MechMtr_config(machine,8);

	/* setup the standard oki MSC1937 display */
	ROC10937_init(0, MSC1937,0);
}

static MACHINE_START( mpu4dutch )
{
	mpu4_config_common(machine);

	link7a_connected=0;

// setup 8 mechanical meters ////////////////////////////////////////////
	MechMtr_config(machine,8);

// setup the standard oki MSC1937 display ///////////////////////////////
	ROC10937_init(0, MSC1937,0);	// does oldtimer use a OKI MSC1937 alpha display controller?
}

static MACHINE_START( mpu4mod4 )
{
	mpu4_config_common(machine);

	link7a_connected=0;
	mod_number=4;

// setup 8 mechanical meters ////////////////////////////////////////////
	MechMtr_config(machine,8);

// setup the standard oki MSC1937 display ///////////////////////////////
	ROC10937_init(0, MSC1937,0);
}

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
through the table starting at the last accessed data value, unless 00 is used to reset to the beginning.

However, a final 8 byte row, that controls the lamp matrix is not tested - to date, no-one outside of Barcrest knows
how this is generated, and currently trial and error is the only sensible method. It is noted that the default,
of all 00, is sometimes the correct answer, particularly in non-Barcrest use of the CHR chip, though when used normally,
there are again fixed call values.
*/


static WRITE8_HANDLER( characteriser_w )
{
	int x;
	int call=data;
	LOG_CHR_FULL(("%04x Characteriser write offset %02X data %02X", cpu_get_previouspc(space->cpu),offset,data));
	if (!mpu4_current_chr_table)
		fatalerror("No Characteriser Table @ %04x\n", cpu_get_previouspc(space->cpu));

	if (offset == 0)
	{
		{
			if (call == 0)
			{
				prot_col = 0;
			}
			else
			{
				for (x = prot_col; x < 64; x++)
				{
					if	(mpu4_current_chr_table[(x)].call == call)
					{
						prot_col = x;
						LOG_CHR(("Characteriser find column %02X\n",prot_col));
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
			lamp_col = 0;
			break;
			case 0x01:
			lamp_col = 1;
			break;
			case 0x04:
			lamp_col = 2;
			break;
			case 0x09:
			lamp_col = 3;
			break;
			case 0x10:
			lamp_col = 4;
			break;
			case 0x19:
			lamp_col = 5;
			break;
			case 0x24:
			lamp_col = 6;
			break;
			case 0x31:
			lamp_col = 7;
			break;
		}
		LOG_CHR(("Characteriser find 2 column %02X\n",lamp_col));
	}
}


static READ8_HANDLER( characteriser_r )
{
	if (!mpu4_current_chr_table)
		fatalerror("No Characteriser Table @ %04x\n", cpu_get_previouspc(space->cpu));

	LOG_CHR(("Characteriser read offset %02X \n",offset));
	if (offset == 0)
	{
		LOG_CHR(("Characteriser read data %02X \n",mpu4_current_chr_table[prot_col].response));
		return mpu4_current_chr_table[prot_col].response;
	}
	if (offset == 3)
	{
		LOG_CHR(("Characteriser read data off 3 %02X \n",mpu4_current_chr_table[lamp_col+64].response));
		return mpu4_current_chr_table[lamp_col+64].response;
	}
	return 0;
}

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

static DRIVER_INIT (m_oldtmr)
{
	reel_mux=SIX_REEL_1TO8;

	stepper_config(machine, 0, &barcrest_opto1_interface);
	stepper_config(machine, 1, &barcrest_opto1_interface);
	stepper_config(machine, 2, &barcrest_opto1_interface);
	stepper_config(machine, 3, &barcrest_opto1_interface);
	stepper_config(machine, 4, &barcrest_opto1_interface);
	stepper_config(machine, 5, &barcrest_opto1_interface);

	awp_reel_setup();
	mpu4_current_chr_table = oldtmr_data;
}

static DRIVER_INIT (m_ccelbr)
{
	reel_mux=STANDARD_REEL;
	// setup 4 default 96 half step reels ///////////////////////////////////
	mpu4_config_common_reels(machine,4);

	mpu4_current_chr_table = ccelbr_data;
}

static DRIVER_INIT (m_gmball)
{
	reel_mux=STANDARD_REEL;
	// setup 4 default 96 half step reels ///////////////////////////////////
	mpu4_config_common_reels(machine,4);

	mpu4_current_chr_table = gmball_data;
}

static DRIVER_INIT (m_grtecp)
{
	reel_mux=FIVE_REEL_5TO8;
	lamp_extender=SMALL_CARD;
	// setup 5 default 96 half step reels with the mux board
	mpu4_config_common_reels(machine,5);
	mpu4_current_chr_table = grtecp_data;
}

static DRIVER_INIT (mpu4tst2)
{
	reel_mux=STANDARD_REEL;
	// setup 5 default 96 half step reels with the mux board
	mpu4_config_common_reels(machine,4);

}

static DRIVER_INIT (mpu4utst)
{
	reel_mux=STANDARD_REEL;
	// setup 4 default 96 half step reels ///////////////////////////////////
	mpu4_config_common_reels(machine,4);
}

static DRIVER_INIT (connect4)
{
	led_lamp=1;
}

/* generate a 50 Hz signal (based on an RC time) */
static TIMER_DEVICE_CALLBACK( gen_50hz )
{
	/* Although reported as a '50Hz' signal, the fact that both rising and
    falling edges of the pulse are used means the timer actually gives a 100Hz
    oscillating signal.*/
	signal_50hz = signal_50hz?0:1;
	update_lamps();
	pia6821_ca1_w(timer.machine->device("pia_ic4"), signal_50hz);	/* signal is connected to IC4 CA1 */

	if (signal_50hz)
	{
		update_meters();
	}

}

static ADDRESS_MAP_START( mod2_memmap, ADDRESS_SPACE_PROGRAM, 8 )
	AM_RANGE(0x0000, 0x07ff) AM_RAM AM_SHARE("nvram")
	AM_RANGE(0x0800, 0x0810) AM_READWRITE(characteriser_r,characteriser_w)

	AM_RANGE(0x0850, 0x0850) AM_READWRITE(bankswitch_r,bankswitch_w)	/* write bank (rom page select) */

/*  AM_RANGE(0x08e0, 0x08e7) AM_READWRITE(68681_duart_r,68681_duart_w) */ //Runs hoppers

	AM_RANGE(0x0900, 0x0907) AM_DEVREADWRITE("ptm_ic2", ptm6840_read, ptm6840_write)/* PTM6840 IC2 */

	AM_RANGE(0x0a00, 0x0a03) AM_DEVREADWRITE("pia_ic3", pia6821_r, pia6821_w)		/* PIA6821 IC3 */
	AM_RANGE(0x0b00, 0x0b03) AM_DEVREADWRITE("pia_ic4", pia6821_r, pia6821_w)		/* PIA6821 IC4 */
	AM_RANGE(0x0c00, 0x0c03) AM_DEVREADWRITE("pia_ic5", pia6821_r, pia6821_w)		/* PIA6821 IC5 */
	AM_RANGE(0x0d00, 0x0d03) AM_DEVREADWRITE("pia_ic6", pia6821_r, pia6821_w)		/* PIA6821 IC6 */
	AM_RANGE(0x0e00, 0x0e03) AM_DEVREADWRITE("pia_ic7", pia6821_r, pia6821_w)		/* PIA6821 IC7 */
	AM_RANGE(0x0f00, 0x0f03) AM_DEVREADWRITE("pia_ic8", pia6821_r, pia6821_w)		/* PIA6821 IC8 */

	AM_RANGE(0x1000, 0xffff) AM_ROMBANK("bank1")	/* 64k  paged ROM (4 pages)  */
ADDRESS_MAP_END

static ADDRESS_MAP_START( mod4_yam_map, ADDRESS_SPACE_PROGRAM, 8 )
	AM_RANGE(0x0000, 0x07ff) AM_RAM AM_SHARE("nvram")

	AM_RANGE(0x0800, 0x0810) AM_READWRITE(characteriser_r,characteriser_w)

	AM_RANGE(0x0850, 0x0850) AM_WRITE(bankswitch_w)	// write bank (rom page select)

	AM_RANGE(0x0880, 0x0881) AM_DEVWRITE( "ym2413", ym2413_w )

/*  AM_RANGE(0x08e0, 0x08e7) AM_READWRITE(68681_duart_r,68681_duart_w) */ //Runs hoppers

	AM_RANGE(0x0900, 0x0907) AM_DEVREADWRITE("ptm_ic2", ptm6840_read, ptm6840_write)/* PTM6840 IC2 */

	AM_RANGE(0x0a00, 0x0a03) AM_DEVREADWRITE("pia_ic3", pia6821_r, pia6821_w)		/* PIA6821 IC3 */
	AM_RANGE(0x0b00, 0x0b03) AM_DEVREADWRITE("pia_ic4", pia6821_r, pia6821_w)		/* PIA6821 IC4 */
	AM_RANGE(0x0c00, 0x0c03) AM_DEVREADWRITE("pia_ic5", pia6821_r, pia6821_w)		/* PIA6821 IC5 */
	AM_RANGE(0x0d00, 0x0d03) AM_DEVREADWRITE("pia_ic6", pia6821_r, pia6821_w)		/* PIA6821 IC6 */
	AM_RANGE(0x0e00, 0x0e03) AM_DEVREADWRITE("pia_ic7", pia6821_r, pia6821_w)		/* PIA6821 IC7 */
	AM_RANGE(0x0f00, 0x0f03) AM_DEVREADWRITE("pia_ic8", pia6821_r, pia6821_w)		/* PIA6821 IC8 */

	AM_RANGE(0x1000, 0xffff) AM_ROMBANK("bank1")	// 64k  paged ROM (4 pages)
ADDRESS_MAP_END

static ADDRESS_MAP_START( mod4_oki_map, ADDRESS_SPACE_PROGRAM, 8 )
	AM_RANGE(0x0000, 0x07ff) AM_RAM AM_SHARE("nvram")

	AM_RANGE(0x0800, 0x0810) AM_READWRITE(characteriser_r,characteriser_w)

	AM_RANGE(0x0850, 0x0850) AM_WRITE(bankswitch_w)	// write bank (rom page select)

	AM_RANGE(0x0880, 0x0883) AM_DEVREADWRITE("pia_ic4ss", pia6821_r,pia6821_w)      // PIA6821 on sampled sound board

	AM_RANGE(0x08c0, 0x08c7) AM_DEVREADWRITE("ptm_ic3ss", ptm6840_read, ptm6840_write)  // 6840PTM on sampled sound board

//  AM_RANGE(0x08e0, 0x08e7) AM_READWRITE(68681_duart_r,68681_duart_w) //Runs hoppers

	AM_RANGE(0x0900, 0x0907) AM_DEVREADWRITE("ptm_ic2", ptm6840_read, ptm6840_write)/* PTM6840 IC2 */

	AM_RANGE(0x0a00, 0x0a03) AM_DEVREADWRITE("pia_ic3", pia6821_r, pia6821_w)		/* PIA6821 IC3 */
	AM_RANGE(0x0b00, 0x0b03) AM_DEVREADWRITE("pia_ic4", pia6821_r, pia6821_w)		/* PIA6821 IC4 */
	AM_RANGE(0x0c00, 0x0c03) AM_DEVREADWRITE("pia_ic5", pia6821_r, pia6821_w)		/* PIA6821 IC5 */
	AM_RANGE(0x0d00, 0x0d03) AM_DEVREADWRITE("pia_ic6", pia6821_r, pia6821_w)		/* PIA6821 IC6 */
	AM_RANGE(0x0e00, 0x0e03) AM_DEVREADWRITE("pia_ic7", pia6821_r, pia6821_w)		/* PIA6821 IC7 */
	AM_RANGE(0x0f00, 0x0f03) AM_DEVREADWRITE("pia_ic8", pia6821_r, pia6821_w)		/* PIA6821 IC8 */

	AM_RANGE(0x1000, 0xffff) AM_ROMBANK("bank1")	// 64k  paged ROM (4 pages)
ADDRESS_MAP_END

// memory map for barcrest mpu4 board /////////////////////////////////////

static ADDRESS_MAP_START( dutch_memmap, ADDRESS_SPACE_PROGRAM, 8 )

	AM_RANGE(0x0000, 0x07ff) AM_RAM AM_SHARE("nvram")

//  AM_RANGE(0x0800, 0x0810) AM_READWRITE(characteriser_r,characteriser_w)

	AM_RANGE(0x0850, 0x0850) AM_WRITE(bankswitch_w)	// write bank (rom page select)
	AM_RANGE(0x0880, 0x0883) AM_DEVREADWRITE("pia_ic4ss", pia6821_r,pia6821_w)      // PIA6821 on sampled sound board

	AM_RANGE(0x08c0, 0x08c7) AM_DEVREADWRITE("ptm_ic3ss", ptm6840_read, ptm6840_write)  // 6840PTM on sampled sound board

//  AM_RANGE(0x08e0, 0x08e7) AM_READWRITE(68681_duart_r,68681_duart_w) //Runs hoppers

	AM_RANGE(0x0900, 0x0907) AM_DEVREADWRITE("ptm_ic2", ptm6840_read, ptm6840_write)/* PTM6840 IC2 */

	AM_RANGE(0x0a00, 0x0a03) AM_DEVREADWRITE("pia_ic3", pia6821_r, pia6821_w)		/* PIA6821 IC3 */
	AM_RANGE(0x0b00, 0x0b03) AM_DEVREADWRITE("pia_ic4", pia6821_r, pia6821_w)		/* PIA6821 IC4 */
	AM_RANGE(0x0c00, 0x0c03) AM_DEVREADWRITE("pia_ic5", pia6821_r, pia6821_w)		/* PIA6821 IC5 */
	AM_RANGE(0x0d00, 0x0d03) AM_DEVREADWRITE("pia_ic6", pia6821_r, pia6821_w)		/* PIA6821 IC6 */
	AM_RANGE(0x0e00, 0x0e03) AM_DEVREADWRITE("pia_ic7", pia6821_r, pia6821_w)		/* PIA6821 IC7 */
	AM_RANGE(0x0f00, 0x0f03) AM_DEVREADWRITE("pia_ic8", pia6821_r, pia6821_w)		/* PIA6821 IC8 */

	AM_RANGE(0x1000, 0xffff) AM_ROMBANK("bank1")	// 64k paged ROM (4 pages)
ADDRESS_MAP_END

static const ay8910_interface ay8910_config =
{
	AY8910_SINGLE_OUTPUT,
	{820,0,0},
	DEVCB_NULL,
	DEVCB_NULL,
	DEVCB_NULL,
	DEVCB_NULL
};


/* machine driver for MOD 2 board */
static MACHINE_CONFIG_START( mpu4mod2, driver_device )

	MCFG_MACHINE_START(mpu4mod2)
	MCFG_MACHINE_RESET(mpu4)
	MCFG_CPU_ADD("maincpu", M6809, MPU4_MASTER_CLOCK/4)
	MCFG_CPU_PROGRAM_MAP(mod2_memmap)

	MCFG_TIMER_ADD_PERIODIC("50hz",gen_50hz, attotime::from_hz(100))

	/* 6840 PTM */
	MCFG_PTM6840_ADD("ptm_ic2", ptm_ic2_intf)

	MCFG_PIA6821_ADD("pia_ic3", pia_ic3_intf)
	MCFG_PIA6821_ADD("pia_ic4", pia_ic4_intf)
	MCFG_PIA6821_ADD("pia_ic5", pia_ic5_intf)
	MCFG_PIA6821_ADD("pia_ic6", pia_ic6_intf)
	MCFG_PIA6821_ADD("pia_ic7", pia_ic7_intf)
	MCFG_PIA6821_ADD("pia_ic8", pia_ic8_intf)

	MCFG_SPEAKER_STANDARD_MONO("mono")
	MCFG_SOUND_ADD("ay8913",AY8913, MPU4_MASTER_CLOCK/4)
	MCFG_SOUND_CONFIG(ay8910_config)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 1.0)

	MCFG_NVRAM_ADD_0FILL("nvram")

	MCFG_DEFAULT_LAYOUT(layout_mpu4)
MACHINE_CONFIG_END

static MACHINE_CONFIG_DERIVED( mod4yam, mpu4mod2 )
	MCFG_MACHINE_START(mpu4mod4)

	MCFG_CPU_MODIFY("maincpu")
	MCFG_CPU_PROGRAM_MAP(mod4_yam_map)

	MCFG_DEVICE_REMOVE("ay8913")
	MCFG_SOUND_ADD("ym2413", YM2413, MPU4_MASTER_CLOCK/4)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.5)
MACHINE_CONFIG_END

static MACHINE_CONFIG_DERIVED( mod4oki, mpu4mod2 )
	MCFG_MACHINE_START(mpu4mod4)

	MCFG_PTM6840_ADD("ptm_ic3ss", ptm_ic3ss_intf)
	MCFG_PIA6821_ADD("pia_ic4ss", pia_ic4ss_intf)

	MCFG_CPU_MODIFY("maincpu")
	MCFG_CPU_PROGRAM_MAP(mod4_oki_map)

	MCFG_DEVICE_REMOVE("ay8913")
	MCFG_SOUND_ADD("msm6376", OKIM6376, 64000) //Dynamic, can also be 85430 at 10.5KHz and 128000 at 16KHz
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 1.0)
MACHINE_CONFIG_END

static MACHINE_CONFIG_DERIVED( mpu4dutch, mod4oki )
	MCFG_CPU_MODIFY("maincpu")
	MCFG_CPU_PROGRAM_MAP(dutch_memmap)				// setup read and write memorymap
	MCFG_MACHINE_START(mpu4dutch)					// main mpu4 board initialisation
MACHINE_CONFIG_END

ROM_START( m_oldtmr )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "dot11.bin",  0x00000, 0x10000,  CRC(da095666) SHA1(bc7654dc9da1f830a43f925db8079f27e18bb61e))
ROM_END

ROM_START( m_ccelbr )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "cels.p1",  0x00000, 0x10000,  CRC(19d2162f) SHA1(24fe435809352725e7614c32e2184142f355298e))
ROM_END

ROM_START( m_gmball )
	ROM_REGION( 0x10000, "maincpu", ROMREGION_ERASE00  )
	ROM_LOAD( "gbbx.p1",	0x0000, 0x10000,  CRC(0b5adcd0) SHA1(1a198bd4a1e7d6bf4cf025c43d35aaef351415fc))
ROM_END

ROM_START( connect4 )
	ROM_REGION( 0x10000, "maincpu", ROMREGION_ERASE00  )
	ROM_LOAD( "connect4.p2",  0x8000, 0x4000,  CRC(6090633c) SHA1(0cd2725a235bf93cfe94f2ca648d5fccb87b8e5c) )
	ROM_LOAD( "connect4.p1",  0xC000, 0x4000,  CRC(b1af50c0) SHA1(7c9645ea378f0857b849ca24a239d9114f62da7f) )
ROM_END

ROM_START( mpu4utst )
	ROM_REGION( 0x10000, "maincpu", ROMREGION_ERASE00  )
	ROM_LOAD( "ut4.p1",  0xC000, 0x4000,  CRC(086dc325) SHA1(923caeb61347ac9d3e6bcec45998ddf04b2c8ffd))
ROM_END

ROM_START( mpu4tst2 )
	ROM_REGION( 0x10000, "maincpu", ROMREGION_ERASE00  )
	ROM_LOAD( "ut2.p1",  0xE000, 0x2000,  CRC(f7fb6575) SHA1(f7961cbd0801b9561d8cd2d23081043d733e1902))
ROM_END

ROM_START( mpu4met0 )
	ROM_REGION( 0x10000, "maincpu", ROMREGION_ERASE00  )
	ROM_LOAD( "meter-zero.p1",  0x8000, 0x8000,  CRC(e74297e5) SHA1(49a2cc85eda14199975ec37a794b685c839d3ab9))
ROM_END

ROM_START( m_grtecp )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD("an2k.p1",  0x00000, 0x10000,  CRC(c0886dff) SHA1(ef2b509fde05ef4ef055a09275afc9e153f50efc))

	ROM_REGION( 0x200000, "msm6376", 0 )
	ROM_LOAD( "an2snd.p1",  0x000000, 0x080000,  CRC(5394e9ae) SHA1(86ccd8531fc87f34d3c5482ba7e5a2c06ea69491) )
	ROM_LOAD( "an2snd.p2",  0x080000, 0x080000,  CRC(109ace1f) SHA1(9f0e8065186beb61ed50fea834de2d91e68db953) )

ROM_END
//    year,  name,    parent,  machine,  input,   init,     monitor,company,        fullname,                     flags
GAME( 198?,  m_oldtmr,0,      mpu4dutch,mpu4,	  m_oldtmr, ROT0,   "Barcrest",		"Old Timer",				  GAME_NOT_WORKING|GAME_NO_SOUND|GAME_REQUIRES_ARTWORK )
GAME( 198?,  m_ccelbr,0,      mpu4mod2, mpu4,	  m_ccelbr, ROT0,   "Barcrest",		"Club Celebration",			  GAME_NOT_WORKING|GAME_REQUIRES_ARTWORK )
GAMEL(198?,  m_gmball,0,      mod4yam,  gamball,  m_gmball, ROT0,   "Barcrest",     "Gamball",					  GAME_NOT_WORKING|GAME_REQUIRES_ARTWORK|GAME_MECHANICAL,layout_gamball )//Mechanical ball launcher
GAMEL(198?,  m_grtecp,0,      mod4oki, mpu4,	  m_grtecp, ROT0,   "Barcrest",		"Andy's Great Escape",		  GAME_NOT_WORKING|GAME_REQUIRES_ARTWORK,layout_mpu4ext )//5 reel meter mux

//SWP
GAMEL(1989?,connect4,        0, mpu4mod2,   connect4,   connect4,   ROT0, "Dolbeck Systems","Connect 4",GAME_IMPERFECT_GRAPHICS|GAME_REQUIRES_ARTWORK,layout_connect4 )

//Diagnostic ROMs
GAME( 198?, mpu4utst,        0, mpu4mod2,   mpu4,       mpu4utst,   ROT0, "Barcrest","MPU4 Unit Test (Program 4)",GAME_MECHANICAL )
GAME( 198?, mpu4tst2,        0, mpu4mod2,   mpu4,       mpu4tst2,   ROT0, "Barcrest","MPU4 Unit Test (Program 2)",GAME_MECHANICAL )
GAME( 198?, mpu4met0,        0, mpu4mod2,   mpu4,       0,          ROT0, "Barcrest","MPU4 Meter Clear ROM",GAME_MECHANICAL )

#include "drivers/mpu4drvr.c"
