/***********************************************************************************************************
  Barcrest MPU4 highly preliminary driver by J.Wallace, and Anonymous.

  This is the core driver, no video specific stuff should go in here.
  This driver holds all the mechanical games.

     04-2011: More accurate gamball code, fixed ROM banking (Project Amber), added BwB CHR simulator (Amber)
              This is still a hard coded system, but significantly different to Barcrest's version.
     03-2011: Lamp timing fixes, support for all known expansion cards added.
     01-2011: Adding the missing 'OKI' sound card, and documented it, but needs 6376 rewrite.
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

In addition there are two auxiliary ports taht can be accessed separately to these and are bidirectional

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

TODO: - Distinguish door switches using manual
      - Complete stubs for hoppers (needs slightly better 68681 emulation, and new 'hoppers' device emulation)
      - It seems that the MPU4 core program relies on some degree of persistence when switching strobes and handling
      writes to the various hardware ports. This explains the occasional lamping/LED blackout and switching bugs
      For now, we're ignoring any extra writes to strobes, as the alternative is to assign a timer to *everything*
      - Flo's move in Great Escape gives spin alarms - need a different opto setting for reverse spin reels?
      - Fix BwB characteriser, need to be able to calculate stabiliser bytes. Anyone fancy reading 6809 source?
      - Fix MSM6376 - We're triggering 'contact MAMEDEV' since we need all features of the chip,
      including dynamic sample rate adjustment and BEEP.
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
#define LOG_SS(x)	do { if (MPU4VERBOSE) logerror x; } while (0)

#include "video/awpvid.h"		//Fruit Machines Only
#include "connect4.lh"
#include "gamball.lh"
#include "mpu4.lh"
#include "mpu4ext.lh"
#define MPU4_MASTER_CLOCK (6880000)

static TIMER_CALLBACK( ic24_timeout );


static const UINT8 reel_mux_table[8]= {0,4,2,6,1,5,3,7};//include 7, although I don't think it's used, this is basically a wire swap
static const UINT8 reel_mux_table7[8]= {3,1,5,6,4,2,0,7};

static const UINT8 bwb_chr_table_common[10]= {0x00,0x04,0x04,0x0c,0x0c,0x1c,0x14,0x2c,0x5c,0x2c};

#define STANDARD_REEL  0	/* As originally designed 3/4 reels*/
#define FIVE_REEL_5TO8 1	/* Interfaces to meter port, allows some mechanical metering, but there is significant 'bounce' in the extra reel*/
#define FIVE_REEL_8TO5 2	/* Mounted backwards for space reasons, but different board*/
#define FIVE_REEL_3TO6 3	/* Connected to the centre of the meter connector, taking up meters 3 to 6 */
#define SIX_REEL_1TO8  4	/* Two reels on the meter drives*/
#define SIX_REEL_5TO8  5	/* Like FIVE_REEL_5TO8, but with an extra reel elsewhere*/
#define SEVEN_REEL     6	/* Mainly club machines, significant reworking of reel hardware*/
#define FLUTTERBOX     7	/* Will you start the fans, please!  A fan using a reel mux-like setup, but not actually a reel*/

#define NO_EXTENDER			0	/* As originally designed */
#define SMALL_CARD			1
#define LARGE_CARD_A		2 //96 Lamps
#define LARGE_CARD_B		3 // 96 Lamps, 16 LEDs - as used by BwB
#define LARGE_CARD_C		4 //Identical to B, no built in LED support

#define CARD_A			1
#define CARD_B			2
#define CARD_C			3

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

struct bwb_chr_table//dynamic populated table
{
	UINT8 response;
};

/* Video stuff */
struct ef9369_t
{
	UINT32 addr;
	UINT16 clut[16];	/* 13-bits - a marking bit and a 444 color */
};

struct bt471_t
{
	UINT8 address;
	UINT8 addr_cnt;
	UINT8 pixmask;
	UINT8 command;
	rgb_t color;
};


class mpu4_state : public driver_device
{
public:
	mpu4_state(running_machine &machine, const driver_device_config_base &config)
		: driver_device(machine, config) { }

	int m_mod_number;
	int m_mmtr_data;
	int m_alpha_data_line;
	int m_alpha_clock;
	int m_ay8913_address;
	int m_serial_data;
	int m_signal_50hz;
	int m_ic4_input_b;
	int m_aux1_input;
	int m_aux2_input;
	int m_IC23G1;
	int m_IC23G2A;
	int m_IC23G2B;
	int m_IC23GC;
	int m_IC23GB;
	int m_IC23GA;
	int m_prot_col;
	int m_lamp_col;
	int m_init_col;
	int m_reel_flag;
	int m_ic23_active;
	int m_led_lamp;
	int m_link7a_connected;
	emu_timer *m_ic24_timer;
	int m_expansion_latch;
	int m_global_volume;
	int m_input_strobe;
	UINT8 m_lamp_strobe;
	UINT8 m_lamp_strobe2;
	UINT8 m_led_strobe;
	UINT8 m_ay_data;
	int m_optic_pattern;
	int m_active_reel;
	int m_remote_meter;
	int m_reel_mux;
	int m_lamp_extender;
	int m_last_b7;
	int m_last_latch;
	int m_lamp_sense;
	int m_card_live;
	int m_led_extender;
	int m_bwb_bank;
	int m_chr_state;
	int m_chr_counter;
	int m_chr_value;
	int m_bwb_return;
	int m_pageval;
	int m_pageset;
	int m_hopper;
	const mpu4_chr_table* m_current_chr_table;
	const bwb_chr_table* m_bwb_chr_table1;
	//Video
	UINT8 m_m6840_irq_state;
	UINT8 m_m6850_irq_state;
	UINT8 m_scn2674_irq_state;
	UINT8 m_m68k_m6809_line;
	UINT8 m_m6809_m68k_line;
	UINT8 m_m68k_acia_cts;
	UINT8 m_m6809_acia_cts;
	UINT8 m_m6809_acia_rts;
	UINT8 m_m6809_acia_dcd;
	int m_gfx_index;
	UINT16 * m_vid_vidram;
	UINT16 * m_vid_mainram;
	UINT8 m_scn2674_IR[16];
	UINT8 m_scn2675_IR_pointer;
	UINT8 m_scn2674_screen1_l;
	UINT8 m_scn2674_screen1_h;
	UINT8 m_scn2674_cursor_l;
	UINT8 m_scn2674_cursor_h;
	UINT8 m_scn2674_screen2_l;
	UINT8 m_scn2674_screen2_h;
	UINT8 m_scn2674_irq_register;
	UINT8 m_scn2674_status_register;
	UINT8 m_scn2674_irq_mask;
	UINT8 m_scn2674_gfx_enabled;
	UINT8 m_scn2674_display_enabled;
	UINT8 m_scn2674_cursor_enabled;
	UINT8 m_IR0_scn2674_double_ht_wd;
	UINT8 m_IR0_scn2674_scanline_per_char_row;
	UINT8 m_IR0_scn2674_sync_select;
	UINT8 m_IR0_scn2674_buffer_mode_select;
	UINT8 m_IR1_scn2674_interlace_enable;
	UINT8 m_IR1_scn2674_equalizing_constant;
	UINT8 m_IR2_scn2674_row_table;
	UINT8 m_IR2_scn2674_horz_sync_width;
	UINT8 m_IR2_scn2674_horz_back_porch;
	UINT8 m_IR3_scn2674_vert_front_porch;
	UINT8 m_IR3_scn2674_vert_back_porch;
	UINT8 m_IR4_scn2674_rows_per_screen;
	UINT8 m_IR4_scn2674_character_blink_rate;
	UINT8 m_IR5_scn2674_character_per_row;
	UINT8 m_IR8_scn2674_display_buffer_first_address_LSB;
	UINT8 m_IR9_scn2674_display_buffer_first_address_MSB;
	UINT8 m_IR9_scn2674_display_buffer_last_address;
	UINT8 m_IR10_scn2674_display_pointer_address_lower;
	UINT8 m_IR11_scn2674_display_pointer_address_upper;
	UINT8 m_IR12_scn2674_scroll_start;
	UINT8 m_IR12_scn2674_split_register_1;
	UINT8 m_IR13_scn2674_scroll_end;
	UINT8 m_IR13_scn2674_split_register_2;
	INT8 m_cur[2];
	UINT8 *m_dealem_videoram;
	int m_rowcounter;
	int m_linecounter;
	struct ef9369_t m_pal;
	struct bt471_t m_bt471;
};

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

static void lamp_extend_small(int data)
{
	int lamp_strobe_ext,column,i;
	column = data & 0x07;

	lamp_strobe_ext = 0x1f - ((data & 0xf8) >> 3);
	if ( lamp_strobe_ext )
	{
		for (i = 0; i < 5; i++)
		{
			output_set_lamp_value((5*column)+i+128,((lamp_strobe_ext  & (1 << i)) != 0));
		}
    }
}

static void lamp_extend_largea(mpu4_state *state, int data,int column,int active)
{
	int lampbase,i,byte7;

	state->m_lamp_sense = 0;
	byte7 = data & 0x80;
	if ( byte7 != state->m_last_b7 )
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
			state->m_lamp_sense = 1;
		}
		if ( active )
		{
			for (i = 0; i < 6; i++)
			{
				output_set_lamp_value(lampbase+i,(data  & (1 << i)) != 0);
			}
		}
    }
    state->m_last_b7 = byte7;
}

static void lamp_extend_largebc(mpu4_state *state, int data,int column,int active)
{
	int lampbase,i,byte7;

	state->m_lamp_sense = 0;
	byte7 = data & 0x80;
	if ( byte7 != state->m_last_b7 )
	{
		state->m_card_live = 1;
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
			state->m_lamp_sense = 1;
		}
		if ( active )
		{
			for (i = 0; i < 6; i++)
			{
				output_set_lamp_value(lampbase+i,(data  & (1 << i)) != 0);
			}
		}
		state->m_last_b7 = byte7;
	}
	else
	{
		state->m_card_live = 0;
	}
}

static void led_write_latch(mpu4_state *state, int latch, int data, int column)
{
	int diff,i;

	diff = (latch ^ state->m_last_latch) & latch;
	column = 7 - column; // like main board, these are wired up in reverse
	data = ~data;//inverted?

	for(i=0; i<5; i++)
	{
		if (diff & (1<<i))
		{
			column += (i*8);
		}
	}
	output_set_digit_value(column, data);

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
static void mpu4_stepper_reset(mpu4_state *state)
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
static WRITE_LINE_DEVICE_HANDLER( cpu0_irq )
{
	mpu4_state *drvstate = device->machine().driver_data<mpu4_state>();
	device_t *pia3 = device->machine().device("pia_ic3");
	device_t *pia4 = device->machine().device("pia_ic4");
	device_t *pia5 = device->machine().device("pia_ic5");
	device_t *pia6 = device->machine().device("pia_ic6");
	device_t *pia7 = device->machine().device("pia_ic7");
	device_t *pia8 = device->machine().device("pia_ic8");
	device_t *ptm2 = device->machine().device("ptm_ic2");

	/* The PIA and PTM IRQ lines are all connected to a common PCB track, leading directly to the 6809 IRQ line. */
	int combined_state = pia6821_get_irq_a(pia3) | pia6821_get_irq_b(pia3) |
						 pia6821_get_irq_a(pia4) | pia6821_get_irq_b(pia4) |
						 pia6821_get_irq_a(pia5) | pia6821_get_irq_b(pia5) |
						 pia6821_get_irq_a(pia6) | pia6821_get_irq_b(pia6) |
						 pia6821_get_irq_a(pia7) | pia6821_get_irq_b(pia7) |
						 pia6821_get_irq_a(pia8) | pia6821_get_irq_b(pia8) |
						 ptm6840_get_irq(ptm2);

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
	ptm6840_set_c2(device, 0, data);

	/* copy output value to IC2 c2
    this output is the clock for timer2 */
	/* 1200Hz System interrupt timer */
}


static WRITE8_DEVICE_HANDLER( ic2_o2_callback )
{
	device_t *pia = device->machine().device("pia_ic3");
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
	ROC10937_draw_16seg(0);
}


static WRITE_LINE_DEVICE_HANDLER( pia_ic3_cb2_w )
{
	LOG_IC3(("%s: IC3 PIA Write CB (alpha reset), %02X\n",device->machine().describe_context(),state));
// DM Data pin A
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
	mpu4_state *state = device->machine().driver_data<mpu4_state>();
	if(state->m_ic23_active)
	{
		if (((state->m_lamp_extender == NO_EXTENDER)||(state->m_lamp_extender == SMALL_CARD)||(state->m_lamp_extender == LARGE_CARD_C))&& (state->m_led_extender == NO_EXTENDER))
		{
			if(state->m_led_strobe != state->m_input_strobe)
			{
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
	if ( state->m_serial_data )
	{
		state->m_ic4_input_b |=  0x80;
		pia6821_cb1_w(device, 1);
	}
	else
	{
		state->m_ic4_input_b &= ~0x80;
		pia6821_cb1_w(device, 0);
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
	mpu4_state *state = device->machine().driver_data<mpu4_state>();
	device_t *pia_ic4 = device->machine().device("pia_ic4");
	if (state->m_hopper == HOPPER_NONDUART_A)
	{
		//hopper1_drive_sensor(data&0x10);
	}
	switch (state->m_lamp_extender)
	{
		case NO_EXTENDER:
		if (state->m_led_extender == CARD_B)
		{
			led_write_latch(state, data & 0x1f, pia6821_get_output_a(pia_ic4),state->m_input_strobe);
		}
		else if ((state->m_led_extender != CARD_A)||(state->m_led_extender != NO_EXTENDER))
		{
			output_set_digit_value((state->m_input_strobe+8),data);
		}
		break;
		case SMALL_CARD:
		if(state->m_ic23_active)
		{
			lamp_extend_small(data);
		}
		break;
		case LARGE_CARD_A:
		lamp_extend_largea(state,data,state->m_input_strobe,state->m_ic23_active);
		break;
		case LARGE_CARD_B:
		lamp_extend_largebc(state,data,state->m_input_strobe,state->m_ic23_active);
		if ((state->m_ic23_active) && state->m_card_live)
		{
			output_set_digit_value(((8*(state->m_last_b7 >>7))+state->m_input_strobe),~data);
		}
		break;
		case LARGE_CARD_C:
		lamp_extend_largebc(state,data,state->m_input_strobe,state->m_ic23_active);
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

		if (mame_stricmp(device->machine().system().name, "m_gmball") == 0)
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
static READ8_DEVICE_HANDLER( pia_ic5_portb_r )
{
	mpu4_state *state = device->machine().driver_data<mpu4_state>();
	device_t *pia_ic5 = device->machine().device("pia_ic5");
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
	coin_lockout_w(device->machine(), 0, (pia6821_get_output_b(pia_ic5) & 0x01) );
	coin_lockout_w(device->machine(), 1, (pia6821_get_output_b(pia_ic5) & 0x02) );
	coin_lockout_w(device->machine(), 2, (pia6821_get_output_b(pia_ic5) & 0x04) );
	coin_lockout_w(device->machine(), 3, (pia6821_get_output_b(pia_ic5) & 0x08) );
	return input_port_read(device->machine(), "AUX2") | state->m_aux2_input;
}


static WRITE_LINE_DEVICE_HANDLER( pia_ic5_ca2_w )
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
	if (!pia6821_get_output_cb2(device))
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
				device_t *pia_ic6 = device->machine().device("pia_ic6");
				LOG(("AY8913 address = %d \n",pia6821_get_output_a(pia_ic6)&0x0f));
				break;
			}
			case 0x02:
			{/* CA2 = 0 CB2 = 1? : Write to selected PSG register and write data to Port A */
				device_t *pia_ic6 = device->machine().device("pia_ic6");
				device_t *ay = device->machine().device("ay8913");
				ay8910_data_w(ay, 0, pia6821_get_output_a(pia_ic6));
				LOG(("AY Chip Write \n"));
				break;
			}
			case 0x03:
			{/* CA2 = 1 CB2 = 1? : The register will now be selected and the user can read from or write to it.
             The register will remain selected until another is chosen.*/
				device_t *pia_ic6 = device->machine().device("pia_ic6");
				device_t *ay = device->machine().device("ay8913");
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
	mpu4_state *state = device->machine().driver_data<mpu4_state>();
	LOG(("%s: IC6 PIA Port B Set to %2x (Reel A and B)\n", device->machine().describe_context(),data));

	if (state->m_reel_mux == SEVEN_REEL)
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

	if (state->m_reel_flag && (state->m_reel_mux == STANDARD_REEL))
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
	else
	{
		stepper_update(2, data & 0x0F );
		stepper_update(3, (data>>4) & 0x0F );
		awp_draw_reel(2);
		awp_draw_reel(3);
	}

	if (state->m_reel_flag && (state->m_reel_mux == STANDARD_REEL))
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
	device_t *pia_ic5 = device->machine().device("pia_ic5");

	LOG_IC8(("%s: IC8 PIA Read of Port A (MUX input data)\n", device->machine().describe_context()));
/* The orange inputs are polled twice as often as the black ones, for reasons of efficiency.
   This is achieved via connecting every input line to an AND gate, thus allowing two strobes
   to represent each orange input bank (strobes are active low). */
	pia6821_cb1_w(pia_ic5, (input_port_read(device->machine(), "AUX2") & 0x80));
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
	if ( !drvstate->m_alpha_clock && (state) )
	{
		ROC10937_shift_data(0, drvstate->m_alpha_data_line&0x01?0:1);
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
	int changed = state->m_expansion_latch^data;

	LOG_SS(("%s: GAMEBOARD: PIA Port A Set to %2x\n", device->machine().describe_context(),data));

	state->m_expansion_latch = data;

	if ( changed & 0x20)
	{ // digital volume clock line changed
		if ( !(data & 0x20) )
		{ // changed from high to low,
			if ( !(data & 0x10) )
			{
				if ( state->m_global_volume < 31 ) state->m_global_volume++; //steps unknown
			}
			else
			{
				if ( state->m_global_volume > 0  ) state->m_global_volume--;
			}

			{
//              float percent = (32-state->m_global_volume)/32.0; //volume_override?1.0:(32-state->m_global_volume)/32.0;
//              LOG(("GAMEBOARD: OKI volume %f \n",percent));
			}
		}
	}
}
static READ8_DEVICE_HANDLER( pia_gb_portb_r )
{
	device_t *msm6376 = device->machine().device("msm6376");
	LOG_SS(("%s: GAMEBOARD: PIA Read of Port B\n",device->machine().describe_context()));
	//
	// b7, 1 = OKI ready, 0 = OKI busy
	// b5, vol clock
	// b4, 1 = Vol down, 0 = Vol up
	//

	return okim6376_r(msm6376,0);
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
FIXME
The MSM6376 sound chip is configured in a slightly strange way, to enable dynamic
sample rate changes (8Khz, 10.6 Khz, 16 KHz) by varying the clock.
According to the BwB programmer's guide, the formula is:
MSM6376 clock frequency:-
freq = (1720000/((t3L+1)(t3H+1)))*[(t3H(T3L+1)+1)/(2(t1+1))]
where [] means rounded up integer,
t3L is the LSB of Clock 3,
t3H is the MSB of Clock 3,
+and t1 is the initial value in clock 1.

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

/* Common configurations */
static void mpu4_config_common(running_machine &machine)
{
	mpu4_state *state = machine.driver_data<mpu4_state>();
	state->m_ic24_timer = machine.scheduler().timer_alloc(FUNC(ic24_timeout));
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

static MACHINE_START( mpu4mod2 )
{
	mpu4_state *state = machine.driver_data<mpu4_state>();
	mpu4_config_common(machine);

	state->m_link7a_connected=0;
	state->m_mod_number=2;

	/* setup 8 mechanical meters */
	MechMtr_config(machine,8);

	/* setup the standard oki MSC1937 display */
	ROC10937_init(0, MSC1937,0);
}

static MACHINE_START( mpu4dutch )
{
	mpu4_state *state = machine.driver_data<mpu4_state>();
	mpu4_config_common(machine);

	state->m_link7a_connected=0;

// setup 8 mechanical meters ////////////////////////////////////////////
	MechMtr_config(machine,8);

// setup the standard oki MSC1937 display ///////////////////////////////
	ROC10937_init(0, MSC1937,0);	// does oldtimer use a OKI MSC1937 alpha display controller?
}

static MACHINE_START( mpu4mod4 )
{
	mpu4_state *state = machine.driver_data<mpu4_state>();
	mpu4_config_common(machine);

	state->m_link7a_connected=0;
	state->m_mod_number=4;

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
		fatalerror("No Characteriser Table @ %04x\n", cpu_get_previouspc(&space->device()));

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
		fatalerror("No Characteriser Table @ %04x\n", cpu_get_previouspc(&space->device()));

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
	// setup 4 default 96 half step reels ///////////////////////////////////
	mpu4_config_common_reels(machine,4);

	state->m_current_chr_table = ccelbr_data;
}

static DRIVER_INIT (m_gmball)
{
	mpu4_state *state = machine.driver_data<mpu4_state>();
	state->m_reel_mux=STANDARD_REEL;
	// setup 4 default 96 half step reels ///////////////////////////////////
	mpu4_config_common_reels(machine,4);

	state->m_current_chr_table = gmball_data;
}

static DRIVER_INIT (m_grtecp)
{
	mpu4_state *state = machine.driver_data<mpu4_state>();
	state->m_reel_mux=FIVE_REEL_5TO8;
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

	stepper_config(machine, 0, &bwb_opto1_interface);
	stepper_config(machine, 1, &bwb_opto1_interface);
	stepper_config(machine, 2, &bwb_opto1_interface);
	stepper_config(machine, 3, &bwb_opto1_interface);
	stepper_config(machine, 4, &bwb_opto1_interface);
	state->m_bwb_chr_table1 = blsbys_data1;
	state->m_current_chr_table = blsbys_data;
}

static DRIVER_INIT (mpu4tst2)
{
	mpu4_state *state = machine.driver_data<mpu4_state>();
	state->m_reel_mux=STANDARD_REEL;
	// setup 5 default 96 half step reels with the mux board
	mpu4_config_common_reels(machine,4);

}

static DRIVER_INIT (mpu4utst)
{
	mpu4_state *state = machine.driver_data<mpu4_state>();
	state->m_reel_mux=STANDARD_REEL;
	// setup 4 default 96 half step reels ///////////////////////////////////
	mpu4_config_common_reels(machine,4);
}

static DRIVER_INIT (connect4)
{
	mpu4_state *state = machine.driver_data<mpu4_state>();
	state->m_led_lamp=1;
}

/* generate a 50 Hz signal (based on an RC time) */
static TIMER_DEVICE_CALLBACK( gen_50hz )
{
	mpu4_state *state = timer.machine().driver_data<mpu4_state>();
	/* Although reported as a '50Hz' signal, the fact that both rising and
    falling edges of the pulse are used means the timer actually gives a 100Hz
    oscillating signal.*/
	state->m_signal_50hz = state->m_signal_50hz?0:1;
	pia6821_ca1_w(timer.machine().device("pia_ic4"), state->m_signal_50hz);	/* signal is connected to IC4 CA1 */

	update_meters(state);//run at 100Hz to sync with PIAs
}

static ADDRESS_MAP_START( mod2_memmap, AS_PROGRAM, 8 )
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

static ADDRESS_MAP_START( mod4_yam_map, AS_PROGRAM, 8 )
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

static ADDRESS_MAP_START( mod4_oki_map, AS_PROGRAM, 8 )
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

static ADDRESS_MAP_START( mpu4_bwb_map, AS_PROGRAM, 8 )
	AM_RANGE(0x0000, 0x07ff) AM_RAM AM_SHARE("nvram")

	AM_RANGE(0x0800, 0x083F) AM_READWRITE(bwb_characteriser_r,bwb_characteriser_w)// Game selects a random value within this range

	AM_RANGE(0x0850, 0x0850) AM_WRITE(bankswitch_w)	// write bank (rom page select)

	AM_RANGE(0x0858, 0x0858) AM_WRITE(bankswitch_w)	// write bank (rom page select)
	AM_RANGE(0x0878, 0x0878) AM_WRITE(bankset_w)	// write bank (rom page select)
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

static ADDRESS_MAP_START( dutch_memmap, AS_PROGRAM, 8 )

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
static MACHINE_CONFIG_START( mpu4mod2, mpu4_state )

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

static MACHINE_CONFIG_DERIVED(bwboki, mod4oki )
	MCFG_CPU_MODIFY("maincpu")
	MCFG_CPU_PROGRAM_MAP(mpu4_bwb_map)				// setup read and write memorymap
MACHINE_CONFIG_END

ROM_START( m_oldtmr )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "dot11.bin",  0x00000, 0x10000,  CRC(da095666) SHA1(bc7654dc9da1f830a43f925db8079f27e18bb61e))

	ROM_REGION( 0x200000, "msm6376", 0 )
	ROM_LOAD( "snd.p1",  0x000000, 0x080000,  NO_DUMP )
	ROM_LOAD( "snd.p2",  0x080000, 0x080000,  NO_DUMP )
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

ROM_START( m_blsbys )
	ROM_REGION( 0x80000, "maincpu", 0 )
	ROM_LOAD("bbprog.bin",  0x00000, 0x20000,  CRC(c262cfda) SHA1(f004895e0dd3f8420683927915554e19e41bd20b))
	ROM_RELOAD(0x40000,0x20000)

	ROM_REGION( 0x200000, "msm6376", 0 )
	ROM_LOAD( "bbsnd.p1",  0x000000, 0x080000,  CRC(715c9e95) SHA1(6a0c9c63e56cfc21bf77cf29c1b844b8e0844c1e) )
	ROM_LOAD( "bbsnd.p2",  0x080000, 0x080000,  CRC(594a87f8) SHA1(edfef7d08fab41fb5814c92930f08a565371eae1) )
ROM_END

//    year,  name,    parent,  machine,  input,   init,     monitor,company,        fullname,                     flags
GAME( 198?,  m_oldtmr,0,      mpu4dutch,mpu4,	  m_oldtmr, ROT0,   "Barcrest",		"Old Timer",				  GAME_NOT_WORKING|GAME_NO_SOUND|GAME_REQUIRES_ARTWORK )
GAME( 198?,  m_ccelbr,0,      mpu4mod2, mpu4,	  m_ccelbr, ROT0,   "Barcrest",		"Club Celebration",			  GAME_NOT_WORKING|GAME_REQUIRES_ARTWORK )
GAMEL(198?,  m_gmball,0,      mod4yam,  gamball,  m_gmball, ROT0,   "Barcrest",     "Gamball",					  GAME_NOT_WORKING|GAME_REQUIRES_ARTWORK|GAME_MECHANICAL,layout_gamball )//Mechanical ball launcher
GAMEL(198?,  m_grtecp,0,      mod4oki,  mpu4,	  m_grtecp, ROT0,   "Barcrest",		"Andy's Great Escape",		  GAME_NOT_WORKING|GAME_REQUIRES_ARTWORK,layout_mpu4ext )//5 reel meter mux
GAME(199?,   m_blsbys,0,	  bwboki,   mpu4,	  m_blsbys, ROT0,   "Barcrest",		"Blues Boys (Version 6)",	  GAME_NOT_WORKING|GAME_REQUIRES_ARTWORK )

//SWP
GAMEL(1989?,connect4,        0, mpu4mod2,   connect4,   connect4,   ROT0, "Dolbeck Systems","Connect 4",GAME_IMPERFECT_GRAPHICS|GAME_REQUIRES_ARTWORK,layout_connect4 )

//Diagnostic ROMs
GAME( 198?, mpu4utst,        0, mpu4mod2,   mpu4,       mpu4utst,   ROT0, "Barcrest","MPU4 Unit Test (Program 4)",GAME_MECHANICAL )
GAME( 198?, mpu4tst2,        0, mpu4mod2,   mpu4,       mpu4tst2,   ROT0, "Barcrest","MPU4 Unit Test (Program 2)",GAME_MECHANICAL )
GAME( 198?, mpu4met0,        0, mpu4mod2,   mpu4,       0,          ROT0, "Barcrest","MPU4 Meter Clear ROM",GAME_MECHANICAL )

#include "drivers/mpu4drvr.c"
