/****************************************************************************************

    bfm_sc2.c

    Bellfruit scorpion2/3 driver, (under heavy construction !!!)

*****************************************************************************************

     04-2011: J Wallace: Fixed watchdog to match actual circuit, also fixed lamping code.
  30-12-2006: J Wallace: Fixed init routines.
  07-03-2006: El Condor: Recoded to more accurately represent the hardware setup.
  18-01-2006: Cleaned up for MAME inclusion
  19-08-2005: Re-Animator

Standard scorpion2 memorymap
The hardware in Scorpion 2 is effectively a Scorpion 1 board with better, non-compatible
microcontrollers, incorporating many of the old expansions on board.

   hex     |r/w| D D D D D D D D |
 location  |   | 7 6 5 4 3 2 1 0 | function
-----------+---+-----------------+-----------------------------------------
0000-1FFF  |R/W| D D D D D D D D | RAM (8k) battery backed up
-----------+---+-----------------+-----------------------------------------
2000-20FF  | W | D D D D D D D D | Reel 1 + 2 stepper latch
-----------+---+-----------------+-----------------------------------------
2000       | R | D D D D D D D D | vfd status
-----------+---+-----------------+-----------------------------------------
2100-21FF  | W | D D D D D D D D | Reel 3 + 4 stepper latch
-----------+---+-----------------+-----------------------------------------
2200-22FF  | W | D D D D D D D D | Reel 5 + 6 stepper latch
-----------+---+-----------------+-----------------------------------------
2300-231F  | W | D D D D D D D D | output mux
-----------+---+-----------------+-----------------------------------------
2300-230B  | R | D D D D D D D D | input mux
-----------+---+-----------------+-----------------------------------------
2320       |R/W| D D D D D D D D | dimas0 ?
-----------+---+-----------------+-----------------------------------------
2321       |R/W| D D D D D D D D | dimas1 ?
-----------+---+-----------------+-----------------------------------------
2322       |R/W| D D D D D D D D | dimas2 ?
-----------+---+-----------------+-----------------------------------------
2323       |R/W| D D D D D D D D | dimas3 ?
-----------+---+-----------------+-----------------------------------------
2324       |R/W| D D D D D D D D | expansion latch
-----------+---+-----------------+-----------------------------------------
2325       | ? | D D D D D D D D | ???
-----------+---+-----------------+-----------------------------------------
2326       | ? | D D D D D D D D | ???
-----------+---+-----------------+-----------------------------------------
2327       | ? | D D D D D D D D | ???
-----------+---+-----------------+-----------------------------------------
2328       |R/W| D D D D D D D D | muxena
-----------+---+-----------------+-----------------------------------------
2329       | W | D D D D D D D D | Timer IRQ enable
-----------+---+-----------------+-----------------------------------------
232A       |R/W| D D D D D D D D | blkdiv ?
-----------+---+-----------------+-----------------------------------------
232B       | ? | D D D D D D D D | ???
-----------+---+-----------------+-----------------------------------------
232C       |R/W| D D D D D D D D | dimena ?
-----------+---+-----------------+-----------------------------------------
232D       | ? | D D D D D D D D | ???
-----------+---+-----------------+-----------------------------------------
232E       | R | D D D D D D D D | chip status b0 = IRQ status
-----------+---+-----------------+-----------------------------------------
232F       | W | D D D D D D D D | coin inhibits
-----------+---+-----------------+-----------------------------------------
2330       | W | D D D D D D D D | payout slide latch
-----------+---+-----------------+-----------------------------------------
2331       | W | D D D D D D D D | payout triac latch
-----------+---+-----------------+-----------------------------------------
2332       |R/W| D D D D D D D D | Watchdog timer
-----------+---+-----------------+-----------------------------------------
2333       | W | D D D D D D D D | electro mechanical meters
-----------+---+-----------------+-----------------------------------------
2334       | ? | D D D D D D D D | ???
-----------+---+-----------------+-----------------------------------------
2335       | ? | D D D D D D D D | ???
-----------+---+-----------------+-----------------------------------------
2336       |?/W| D D D D D D D D | dimcnt ?
-----------+---+-----------------+-----------------------------------------
2337       | W | D D D D D D D D | volume override
-----------+---+-----------------+-----------------------------------------
2338       | W | D D D D D D D D | payout chip select
-----------+---+-----------------+-----------------------------------------
2339       | W | D D D D D D D D | clkden ?
-----------+---+-----------------+-----------------------------------------
2400       |R/W| D D D D D D D D | uart1 (MC6850 compatible) control/status
-----------+---+-----------------+-----------------------------------------
2500       |R/W| D D D D D D D D | uart1 (MC6850 compatible) data
-----------+---+-----------------+-----------------------------------------
2600       |R/W| D D D D D D D D | uart2 (MC6850 compatible) control/status
-----------+---+-----------------+-----------------------------------------
2700       |R/W| D D D D D D D D | uart2 (MC6850 compatible) data
-----------+---+-----------------+-----------------------------------------
2800       |R/W| D D D D D D D D | vfd1
-----------+---+-----------------+-----------------------------------------
2900       |R/W| D D D D D D D D | reset vfd1 + vfd2
-----------+---+-----------------+-----------------------------------------
2D00       |R/W| D D D D D D D D | ym2413 control
-----------+---+-----------------+-----------------------------------------
2D01       |R/W| D D D D D D D D | ym2413 data
-----------+---+-----------------+-----------------------------------------
2E00       |R/W| D D D D D D D D | ROM page latch
-----------+---+-----------------+-----------------------------------------
2F00       |R/W| D D D D D D D D | vfd2
-----------+---+-----------------+-----------------------------------------
3FFE       | R | D D D D D D D D | direct input1
-----------+---+-----------------+-----------------------------------------
3FFF       | R | D D D D D D D D | direct input2
-----------+---+-----------------+-----------------------------------------
2A00       | W | D D D D D D D D | NEC uPD7759 data
-----------+---+-----------------+-----------------------------------------
2B00       | W | D D D D D D D D | NEC uPD7759 reset
-----------+---+-----------------+-----------------------------------------
4000-5FFF  | R | D D D D D D D D | ROM (8k)
-----------+---+-----------------+-----------------------------------------
6000-7FFF  | R | D D D D D D D D | Paged ROM (8k)
           |   |                 |   page 0 : rom area 0x0000 - 0x1FFF
           |   |                 |   page 1 : rom area 0x2000 - 0x3FFF
           |   |                 |   page 2 : rom area 0x4000 - 0x5FFF
           |   |                 |   page 3 : rom area 0x6000 - 0x7FFF
-----------+---+-----------------+-----------------------------------------
8000-FFFF  | R | D D D D D D D D | ROM (32k)
-----------+---+-----------------+-----------------------------------------

Adder hardware:
    Games supported:
        * Quintoon (2 sets Dutch, 2 sets UK)
        * Pokio (1 set)
        * Paradice (1 set)
        * Pyramid (1 set)
        * Slots (1 set Dutch, 2 sets Belgian)
        * Golden Crown (1 Set)

    Known issues:
        * Need to find the 'missing' game numbers
        * Fix RS232 protocol
***************************************************************************/

#include "emu.h"
#include "cpu/m6809/m6809.h"

#include "machine/nvram.h"

#include "video/bfm_adr2.h"

#include "sound/2413intf.h"
#include "sound/upd7759.h"

/* fruit machines only */
#include "video/awpvid.h"
#include "machine/steppers.h" // stepper motor

#include "machine/bfm_bd1.h"  // vfd
#include "machine/meters.h"

#include "bfm_sc2.lh"
#include "gldncrwn.lh"
#include "paradice.lh"
#include "pokio.lh"
#include "pyramid.lh"
#include "quintoon.lh"
#include "sltblgpo.lh"
#include "sltblgtk.lh"
#include "slots.lh"

/* fruit machines only */
#include "video/bfm_dm01.h"
#include "awpdmd.lh"
#include "drwho.lh"
#include "awpvid14.lh"
#include "awpvid16.lh"
#include "machine/bfm_comn.h"


class bfm_sc2_state : public driver_device
{
public:
	bfm_sc2_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag) { }

	int m_sc2gui_update_mmtr;
	UINT8 *m_nvram;
	UINT8 m_key[16];
	UINT8 m_e2ram[1024];
	int m_mmtr_latch;
	int m_triac_latch;
	int m_vfd1_latch;
	int m_vfd2_latch;
	int m_irq_status;
	int m_optic_pattern;
	int m_uart1_data;
	int m_uart2_data;
	int m_data_to_uart1;
	int m_data_to_uart2;
	int m_locked;
	int m_is_timer_enabled;
	int m_reel_changed;
	int m_coin_inhibits;
	int m_irq_timer_stat;
	int m_expansion_latch;
	int m_global_volume;
	int m_volume_override;
	int m_sc2_show_door;
	int m_sc2_door_state;
	int m_reels;
	int m_reel12_latch;
	int m_reel34_latch;
	int m_reel56_latch;
	int m_pay_latch;
	int m_slide_states[6];
	int m_slide_pay_sensor[6];
	int m_has_hopper;
	int m_triac_select;
	int m_hopper_running;
	int m_hopper_coin_sense;
	int m_timercnt;
	UINT8 m_sc2_Inputs[64];
	UINT8 m_input_override[64];
	int m_e2reg;
	int m_e2state;
	int m_e2cnt;
	int m_e2data;
	int m_e2address;
	int m_e2rw;
	int m_e2data_pin;
	int m_e2dummywrite;
	int m_e2data_to_read;
	UINT8 m_codec_data[256];
};


#ifdef MAME_DEBUG
#define VERBOSE 1
#else
#define VERBOSE 0
#endif

// log serial communication between mainboard (scorpion2) and videoboard (adder2)
#define LOG_SERIAL(x) do { if (VERBOSE) logerror x; } while (0)
#define UART_LOG(x) do { if (VERBOSE) logerror x; } while (0)
#define LOG(x) do { if (VERBOSE) logerror x; } while (0)

#define MASTER_CLOCK		(XTAL_8MHz)

// local prototypes ///////////////////////////////////////////////////////

static int  read_e2ram(running_machine &machine);
static void e2ram_reset(running_machine &machine);

/*      INPUTS layout

     b7 b6 b5 b4 b3 b2 b1 b0

     82 81 80 04 03 02 01 00  0
     92 91 90 14 13 12 11 10  1
     A2 A1 A0 24 23 22 21 20  2
     B2 B1 B0 34 33 32 31 30  3
     -- 84 83 44 43 42 41 40  4
     -- 94 93 54 53 52 51 50  5
     -- A4 A3 64 63 62 61 60  6
     -- B4 B3 74 73 72 71 70  7

     B7 B6 B5 B4 B3 B2 B1 B0
      0  1  1  0  0  0

*/

///////////////////////////////////////////////////////////////////////////
// called if board is reset ///////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////

static void on_scorpion2_reset(running_machine &machine)
{
	bfm_sc2_state *state = machine.driver_data<bfm_sc2_state>();
	state->m_vfd1_latch        = 0;
	state->m_vfd2_latch        = 0;
	state->m_mmtr_latch        = 0;
	state->m_triac_latch       = 0;
	state->m_irq_status        = 0;
	state->m_is_timer_enabled  = 1;
	state->m_coin_inhibits     = 0;
	state->m_irq_timer_stat    = 0;
	state->m_expansion_latch   = 0;
	state->m_global_volume     = 0;
	state->m_volume_override   = 0;
	state->m_triac_select      = 0;
	state->m_pay_latch         = 0;

	state->m_reel12_latch      = 0;
	state->m_reel34_latch      = 0;
	state->m_reel56_latch      = 0;

	state->m_hopper_running    = 0;  // for video games
	state->m_hopper_coin_sense = 0;

	state->m_slide_states[0] = 0;
	state->m_slide_states[1] = 0;
	state->m_slide_states[2] = 0;
	state->m_slide_states[3] = 0;
	state->m_slide_states[4] = 0;
	state->m_slide_states[5] = 0;

	BFM_BD1_reset(0);	// reset display1
	BFM_BD1_reset(1);	// reset display2

	e2ram_reset(machine);

	devtag_reset(machine, "ymsnd");

  // reset stepper motors /////////////////////////////////////////////////
	{
		int pattern =0, i;

		for ( i = 0; i < state->m_reels; i++)
		{
			stepper_reset_position(i);
			if ( stepper_optic_state(i) ) pattern |= 1<<i;
		}

		state->m_optic_pattern = pattern;

	}

	state->m_locked        = 0;

	// make sure no inputs are overidden ////////////////////////////////////
	memset(state->m_input_override, 0, sizeof(state->m_input_override));

	// init rom bank ////////////////////////////////////////////////////////

	{
		UINT8 *rom = machine.region("maincpu")->base();

		memory_configure_bank(machine, "bank1", 0, 4, &rom[0x00000], 0x02000);

		memory_set_bank(machine, "bank1",3);
	}
}

///////////////////////////////////////////////////////////////////////////

void Scorpion2_SetSwitchState(running_machine &machine, int strobe, int data, int state)
{
	bfm_sc2_state *drvstate = machine.driver_data<bfm_sc2_state>();
	if ( strobe < 11 && data < 8 )
	{
		if ( strobe < 8 )
		{
			drvstate->m_input_override[strobe] |= (1<<data);

			if ( state ) drvstate->m_sc2_Inputs[strobe] |=  (1<<data);
			else		 drvstate->m_sc2_Inputs[strobe] &= ~(1<<data);
		}
		else
		{
			if ( data > 2 )
			{
				drvstate->m_input_override[strobe-8+4] |= (1<<(data+2));

				if ( state ) drvstate->m_sc2_Inputs[strobe-8+4] |=  (1<<(data+2));
				else		 drvstate->m_sc2_Inputs[strobe-8+4] &= ~(1<<(data+2));
			}
			else
			{
				drvstate->m_input_override[strobe-8] |= (1<<(data+5));

				if ( state ) drvstate->m_sc2_Inputs[strobe-8] |=  (1 << (data+5));
				else		 drvstate->m_sc2_Inputs[strobe-8] &= ~(1 << (data+5));
			}
		}
	}
}

///////////////////////////////////////////////////////////////////////////

int Scorpion2_GetSwitchState(running_machine &machine, int strobe, int data)
{
	bfm_sc2_state *drvstate = machine.driver_data<bfm_sc2_state>();
	int state = 0;

	if ( strobe < 11 && data < 8 )
	{
		if ( strobe < 8 )
		{
			state = (drvstate->m_sc2_Inputs[strobe] & (1<<data) ) ? 1 : 0;
		}
		else
		{
			if ( data > 2 )
			{
				state = (drvstate->m_sc2_Inputs[strobe-8+4] & (1<<(data+2)) ) ? 1 : 0;
			}
			else
			{
				state = (drvstate->m_sc2_Inputs[strobe-8] & (1 << (data+5)) ) ? 1 : 0;
			}
		}
	}
	return state;
}

///////////////////////////////////////////////////////////////////////////

static NVRAM_HANDLER( bfm_sc2 )
{
	bfm_sc2_state *state = machine.driver_data<bfm_sc2_state>();
	static const UINT8 init_e2ram[10] = { 1, 4, 10, 20, 0, 1, 1, 4, 10, 20 };
	if ( read_or_write )
	{	// writing
		file->write(state->m_e2ram,sizeof(state->m_e2ram));
	}
	else
	{ // reading
		if ( file )
		{
			file->read(state->m_e2ram,sizeof(state->m_e2ram));
		}
		else
		{
			memset(state->m_e2ram,0x00,sizeof(state->m_e2ram));
			memcpy(state->m_e2ram,init_e2ram,sizeof(init_e2ram));
		}
	}
}

///////////////////////////////////////////////////////////////////////////

static WRITE8_HANDLER( bankswitch_w )
{
	memory_set_bank(space->machine(), "bank1",data & 0x03);
}

///////////////////////////////////////////////////////////////////////////

static INTERRUPT_GEN( timer_irq )
{
	bfm_sc2_state *state = device->machine().driver_data<bfm_sc2_state>();
	state->m_timercnt++;

	if ( state->m_is_timer_enabled )
	{
		state->m_irq_timer_stat = 0x01;
		state->m_irq_status     = 0x02;

		generic_pulse_irq_line(device, M6809_IRQ_LINE);
	}
}

///////////////////////////////////////////////////////////////////////////

static WRITE8_HANDLER( reel12_vid_w )  // in a video cabinet this is used to drive a hopper
{
	bfm_sc2_state *state = space->machine().driver_data<bfm_sc2_state>();
	state->m_reel12_latch = data;

	if ( state->m_has_hopper )
	{
		int oldhop = state->m_hopper_running;

		if ( data & 0x01 )
		{ // hopper power
			if ( data & 0x02 )
			{
				state->m_hopper_running    = 1;
			}
			else
			{
				state->m_hopper_running    = 0;
			}
		}
		else
		{
			//state->m_hopper_coin_sense = 0;
			state->m_hopper_running    = 0;
		}

		if ( oldhop != state->m_hopper_running )
		{
			state->m_hopper_coin_sense = 0;
			oldhop = state->m_hopper_running;
		}
	}
}


/* Reels 1 and 2 */
static WRITE8_HANDLER( reel12_w )
{
	bfm_sc2_state *state = space->machine().driver_data<bfm_sc2_state>();
	state->m_reel12_latch = data;

	if ( stepper_update(0, data&0x0f   ) ) state->m_reel_changed |= 0x01;
	if ( stepper_update(1, (data>>4))&0x0f ) state->m_reel_changed |= 0x02;

	if ( stepper_optic_state(0) ) state->m_optic_pattern |=  0x01;
	else                          state->m_optic_pattern &= ~0x01;
	if ( stepper_optic_state(1) ) state->m_optic_pattern |=  0x02;
	else                          state->m_optic_pattern &= ~0x02;

	awp_draw_reel(0);
	awp_draw_reel(1);
}

static WRITE8_HANDLER( reel34_w )
{
	bfm_sc2_state *state = space->machine().driver_data<bfm_sc2_state>();
	state->m_reel34_latch = data;

	if ( stepper_update(2, data&0x0f ) ) state->m_reel_changed |= 0x04;
	if ( stepper_update(3, (data>>4)&0x0f) ) state->m_reel_changed |= 0x08;

	if ( stepper_optic_state(2) ) state->m_optic_pattern |=  0x04;
	else                          state->m_optic_pattern &= ~0x04;
	if ( stepper_optic_state(3) ) state->m_optic_pattern |=  0x08;
	else                          state->m_optic_pattern &= ~0x08;

	awp_draw_reel(2);
	awp_draw_reel(3);
}

///////////////////////////////////////////////////////////////////////////

static WRITE8_HANDLER( reel56_w )
{
	bfm_sc2_state *state = space->machine().driver_data<bfm_sc2_state>();
	state->m_reel56_latch = data;

	if ( stepper_update(4, data&0x0f   ) ) state->m_reel_changed |= 0x10;
	if ( stepper_update(5, (data>>4)&0x0f) ) state->m_reel_changed |= 0x20;

	if ( stepper_optic_state(4) ) state->m_optic_pattern |=  0x10;
	else                          state->m_optic_pattern &= ~0x10;
	if ( stepper_optic_state(5) ) state->m_optic_pattern |=  0x20;
	else                          state->m_optic_pattern &= ~0x20;

	awp_draw_reel(4);
	awp_draw_reel(5);
}



///////////////////////////////////////////////////////////////////////////
// mechanical meters //////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////

static WRITE8_HANDLER( mmtr_w )
{
	bfm_sc2_state *state = space->machine().driver_data<bfm_sc2_state>();
	int i;
	int  changed = state->m_mmtr_latch ^ data;

	state->m_mmtr_latch = data;

	for (i = 0; i<8; i++)
	{
		if ( changed & (1 << i) )
		{
			MechMtr_update(i, data & (1 << i) );
		}
	}
	if ( data & 0x1F ) cputag_set_input_line(space->machine(), "maincpu", M6809_FIRQ_LINE, ASSERT_LINE );
}

///////////////////////////////////////////////////////////////////////////

static WRITE8_HANDLER( mux_output_w )
{
	int i;
	int off = offset<<3;

	for (i=0; i<8; i++)
		output_set_lamp_value(off+i, ((data & (1 << i)) != 0));

}

///////////////////////////////////////////////////////////////////////////

static READ8_HANDLER( mux_input_r )
{
	bfm_sc2_state *state = space->machine().driver_data<bfm_sc2_state>();
	int result = 0xFF,t1,t2;
	static const char *const port[] = { "STROBE0", "STROBE1", "STROBE2", "STROBE3", "STROBE4", "STROBE5", "STROBE6", "STROBE7", "STROBE8", "STROBE9", "STROBE10", "STROBE11" };

	if (offset < 8)
	{
		int idx = (offset & 4) ? 4 : 8;
		t1 = state->m_input_override[offset];	// strobe 0-7 data 0-4
		t2 = state->m_input_override[offset+idx];	// strobe 8-B data 0-4

		t1 = (state->m_sc2_Inputs[offset]   & t1) | ( ( input_port_read(space->machine(), port[offset])   & ~t1) & 0x1F);
		if (idx == 8)
			t2 = (state->m_sc2_Inputs[offset+8] & t2) | ( ( input_port_read(space->machine(), port[offset+8]) & ~t2) << 5);
		else
			t2 =  (state->m_sc2_Inputs[offset+4] & t2) | ( ( ( input_port_read(space->machine(), port[offset+4]) & ~t2) << 2) & 0x60);

		state->m_sc2_Inputs[offset]   = (state->m_sc2_Inputs[offset]   & ~0x1F) | t1;
		state->m_sc2_Inputs[offset+idx] = (state->m_sc2_Inputs[offset+idx] & ~0x60) | t2;
		result = t1 | t2;
	}

	return result;
}

///////////////////////////////////////////////////////////////////////////

static WRITE8_HANDLER( unlock_w )
{
}

///////////////////////////////////////////////////////////////////////////

static WRITE8_HANDLER( dimas_w )
{
}

///////////////////////////////////////////////////////////////////////////

static WRITE8_HANDLER( dimcnt_w )
{
}

///////////////////////////////////////////////////////////////////////////

static WRITE8_HANDLER( unknown_w )
{
}

///////////////////////////////////////////////////////////////////////////

static WRITE8_HANDLER( volume_override_w )
{
	bfm_sc2_state *state = space->machine().driver_data<bfm_sc2_state>();
	int old = state->m_volume_override;

	state->m_volume_override = data?1:0;

	if ( old != state->m_volume_override )
	{
		ym2413_device *ym = space->machine().device<ym2413_device>("ymsnd");
		upd7759_device *upd = space->machine().device<upd7759_device>("upd");
		float percent = state->m_volume_override? 1.0f : (32-state->m_global_volume)/32.0f;

		ym->set_output_gain(0, percent);
		ym->set_output_gain(1, percent);
		upd->set_output_gain(0, percent);
	}
}

///////////////////////////////////////////////////////////////////////////

static WRITE8_DEVICE_HANDLER( nec_reset_w )
{
	upd7759_start_w(device, 0);
	upd7759_reset_w(device, data);
}

///////////////////////////////////////////////////////////////////////////

static WRITE8_DEVICE_HANDLER( nec_latch_w )
{
	bfm_sc2_state *state = device->machine().driver_data<bfm_sc2_state>();
	int bank = 0;

	if ( data & 0x80 )         bank |= 0x01;
	if ( state->m_expansion_latch & 2 ) bank |= 0x02;

	upd7759_set_bank_base(device, bank*0x20000);

	upd7759_port_w(device, 0, data&0x3F);	// setup sample
	upd7759_start_w(device, 0);
	upd7759_start_w(device, 1);
}

///////////////////////////////////////////////////////////////////////////

static READ8_HANDLER( vfd_status_hop_r )	// on video games, hopper inputs are connected to this
{
	bfm_sc2_state *state = space->machine().driver_data<bfm_sc2_state>();
	// b7 = NEC busy
	// b6 = alpha busy (also matrix board)
	// b5 - b0 = reel optics

	int result = 0;

	if ( state->m_has_hopper )
	{
		result |= 0x04; // hopper high level
		result |= 0x08; // hopper low  level

		result |= 0x01|0x02;

		if ( state->m_hopper_running )
		{
			result &= ~0x01;								  // set motor running input

			if ( state->m_timercnt & 0x04 ) state->m_hopper_coin_sense ^= 1;	  // toggle coin seen

			if ( state->m_hopper_coin_sense ) result &= ~0x02;		  // update coin seen input
		}
	}

	if ( !upd7759_busy_r(space->machine().device("upd")) ) result |= 0x80;			  // update sound busy input

	return result;
}

///////////////////////////////////////////////////////////////////////////

static WRITE8_HANDLER( expansion_latch_w )
{
	bfm_sc2_state *state = space->machine().driver_data<bfm_sc2_state>();
	int changed = state->m_expansion_latch^data;

	state->m_expansion_latch = data;

	// bit0,  1 = lamp mux disabled, 0 = lamp mux enabled
	// bit1,  ? used in Del's millions
	// bit2,  digital volume pot meter, clock line
	// bit3,  digital volume pot meter, direction line
	// bit4,  ?
	// bit5,  ?
	// bit6,  ? used in Del's millions
	// bit7   ?

	if ( changed & 0x04)
	{ // digital volume clock line changed
		if ( !(data & 0x04) )
		{ // changed from high to low,
			if ( !(data & 0x08) )
			{
				if ( state->m_global_volume < 31 ) state->m_global_volume++; //0-31 expressed as 1-32
			}
			else
			{
				if ( state->m_global_volume > 0  ) state->m_global_volume--;
			}

			{
				ym2413_device *ym = space->machine().device<ym2413_device>("ymsnd");
				upd7759_device *upd = space->machine().device<upd7759_device>("upd");
				float percent = state->m_volume_override ? 1.0f : (32-state->m_global_volume)/32.0f;

				ym->set_output_gain(0, percent);
				ym->set_output_gain(1, percent);
				upd->set_output_gain(0, percent);
			}
		}
	}
}

///////////////////////////////////////////////////////////////////////////

static READ8_HANDLER( expansion_latch_r )
{
	return 0;
}

///////////////////////////////////////////////////////////////////////////

static WRITE8_HANDLER( muxena_w )
{
}

///////////////////////////////////////////////////////////////////////////

static WRITE8_HANDLER( timerirq_w )
{
	bfm_sc2_state *state = space->machine().driver_data<bfm_sc2_state>();
	state->m_is_timer_enabled = data & 1;
}

///////////////////////////////////////////////////////////////////////////

static READ8_HANDLER( timerirqclr_r )
{
	bfm_sc2_state *state = space->machine().driver_data<bfm_sc2_state>();
	state->m_irq_timer_stat = 0;
	state->m_irq_status     = 0;

	return 0;
}

///////////////////////////////////////////////////////////////////////////

static READ8_HANDLER( irqstatus_r )
{
	bfm_sc2_state *state = space->machine().driver_data<bfm_sc2_state>();
	int result = state->m_irq_status | state->m_irq_timer_stat | 0x80;	// 0x80 = ~MUXERROR

	state->m_irq_timer_stat = 0;

	return result;
}

///////////////////////////////////////////////////////////////////////////

static WRITE8_HANDLER( coininhib_w )
{
	bfm_sc2_state *state = space->machine().driver_data<bfm_sc2_state>();
	int changed = state->m_coin_inhibits^data,i,p;

	state->m_coin_inhibits = data;

	p = 0x01;
	i = 0;

	while ( i < 8 && changed )
	{
		if ( changed & p )
		{ // this inhibit line has changed
			coin_lockout_w(space->machine(), i, (~data & p) ); // update lockouts
			changed &= ~p;
		}

		p <<= 1;
		i++;
	}
}

///////////////////////////////////////////////////////////////////////////

static READ8_HANDLER( coin_input_r )
{
	return input_port_read(space->machine(), "COINS");
}

///////////////////////////////////////////////////////////////////////////

static WRITE8_HANDLER( payout_latch_w )
{
	bfm_sc2_state *state = space->machine().driver_data<bfm_sc2_state>();
	state->m_pay_latch = data;
}

///////////////////////////////////////////////////////////////////////////

static WRITE8_HANDLER( payout_triac_w )
{
	bfm_sc2_state *state = space->machine().driver_data<bfm_sc2_state>();
	if ( state->m_triac_select == 0x57 )
	{
		int slide = 0;

		switch ( state->m_pay_latch )
		{
			case 0x01: slide = 1;
				break;

			case 0x02: slide = 2;
				break;

			case 0x04: slide = 3;
				break;

			case 0x08: slide = 4;
				break;

			case 0x10: slide = 5;
				break;

			case 0x20: slide = 6;
				break;
		}

		if ( slide )
		{
			if ( data == 0x4D )
			{
				if ( !state->m_slide_states[slide] )
				{
					if ( state->m_slide_pay_sensor[slide] )
					{
						int strobe = state->m_slide_pay_sensor[slide]>>4, data = state->m_slide_pay_sensor[slide]&0x0F;

						Scorpion2_SetSwitchState(space->machine(), strobe, data, 0);
					}
					state->m_slide_states[slide] = 1;
				}
			}
			else
			{
				if ( state->m_slide_states[slide] )
				{
					if ( state->m_slide_pay_sensor[slide] )
					{
						int strobe = state->m_slide_pay_sensor[slide]>>4, data = state->m_slide_pay_sensor[slide]&0x0F;

						Scorpion2_SetSwitchState(space->machine(), strobe, data, 1);
					}
					state->m_slide_states[slide] = 0;
				}
			}
		}
	}
}

///////////////////////////////////////////////////////////////////////////

static WRITE8_HANDLER( payout_select_w )
{
	bfm_sc2_state *state = space->machine().driver_data<bfm_sc2_state>();
	state->m_triac_select = data;
}



///////////////////////////////////////////////////////////////////////////

static WRITE8_HANDLER( vfd2_data_w )
{
	bfm_sc2_state *state = space->machine().driver_data<bfm_sc2_state>();
	state->m_vfd2_latch = data;
	BFM_BD1_newdata(1, data);
	BFM_BD1_draw(1);
}

///////////////////////////////////////////////////////////////////////////

static WRITE8_HANDLER( vfd_reset_w )
{
	BFM_BD1_reset(0);	  // reset both VFD's
	BFM_BD1_reset(1);
	BFM_BD1_draw(0);
	BFM_BD1_draw(1);
}

///////////////////////////////////////////////////////////////////////////
// serial port ////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////

static READ8_HANDLER( uart1stat_r )
{
	bfm_sc2_state *state = space->machine().driver_data<bfm_sc2_state>();
	int status = 0x06;

	if ( state->m_data_to_uart1  ) status |= 0x01;
	if ( !state->m_data_to_uart2 ) status |= 0x02;

	return status;
}
///////////////////////////////////////////////////////////////////////////

static READ8_HANDLER( uart1data_r )
{
	bfm_sc2_state *state = space->machine().driver_data<bfm_sc2_state>();
	return state->m_uart1_data;
}

//////////////////////////////////////////////////////////////////////////

static WRITE8_HANDLER( uart1ctrl_w )
{
	UART_LOG(("uart1ctrl:%x\n", data));
}
///////////////////////////////////////////////////////////////////////////

static WRITE8_HANDLER( uart1data_w )
{
	bfm_sc2_state *state = space->machine().driver_data<bfm_sc2_state>();
	state->m_data_to_uart2 = 1;
	state->m_uart1_data    = data;
	UART_LOG(("uart1:%x\n", data));
}
///////////////////////////////////////////////////////////////////////////

static READ8_HANDLER( uart2stat_r )
{
	bfm_sc2_state *state = space->machine().driver_data<bfm_sc2_state>();
	int status = 0x06;

	if ( state->m_data_to_uart2  ) status |= 0x01;
	if ( !state->m_data_to_uart1 ) status |= 0x02;

	return status;
}
///////////////////////////////////////////////////////////////////////////

static READ8_HANDLER( uart2data_r )
{
	bfm_sc2_state *state = space->machine().driver_data<bfm_sc2_state>();
	return state->m_uart2_data;
}

///////////////////////////////////////////////////////////////////////////

static WRITE8_HANDLER( uart2ctrl_w )
{
	UART_LOG(("uart2ctrl:%x\n", data));
}

///////////////////////////////////////////////////////////////////////////

static WRITE8_HANDLER( uart2data_w )
{
	bfm_sc2_state *state = space->machine().driver_data<bfm_sc2_state>();
	state->m_data_to_uart1 = 1;
	state->m_uart2_data    = data;
	UART_LOG(("uart2:%x\n", data));
}

///////////////////////////////////////////////////////////////////////////

static WRITE8_HANDLER( vid_uart_tx_w )
{
	adder2_send(data);
	cputag_set_input_line(space->machine(), "adder2", M6809_IRQ_LINE, HOLD_LINE );

	LOG_SERIAL(("sadder  %02X  (%c)\n",data, data ));
}

///////////////////////////////////////////////////////////////////////////

static WRITE8_HANDLER( vid_uart_ctrl_w )
{
}

///////////////////////////////////////////////////////////////////////////

static READ8_HANDLER( vid_uart_rx_r )
{
	int data = adder2_receive();

	LOG_SERIAL(("radder:  %02X(%c)\n",data, data ));

	return data;
}

///////////////////////////////////////////////////////////////////////////

static READ8_HANDLER( vid_uart_ctrl_r )
{
	return adder2_status();
}

///////////////////////////////////////////////////////////////////////////

static READ8_HANDLER( key_r )
{
	bfm_sc2_state *state = space->machine().driver_data<bfm_sc2_state>();
	int result = state->m_key[ offset ];

	if ( offset == 7 )
	{
		result = (result & 0xFE) | read_e2ram(space->machine());
	}

	return result;
}

///////////////////////////////////////////////////////////////////////////
/*

The X24C08 is a CMOS 8,192 bit serial EEPROM,
internally organized 1024 x 8. The X24C08 features a
serial interface and software protocol allowing operation
on a simple two wire bus.

*/




#define SCL 0x01	//SCL pin (clock)
#define SDA	0x02	//SDA pin (data)


static void e2ram_reset(running_machine &machine)
{
	bfm_sc2_state *state = machine.driver_data<bfm_sc2_state>();
	state->m_e2reg   = 0;
	state->m_e2state = 0;
	state->m_e2address = 0;
	state->m_e2rw    = 0;
	state->m_e2data_pin = 0;
	state->m_e2data  = (SDA|SCL);
	state->m_e2dummywrite = 0;
	state->m_e2data_to_read = 0;
}

static int recdata(bfm_sc2_state *state, int changed, int data)
{
	int res = 1;

	if ( state->m_e2cnt < 8 )
	{
		res = 0;

		if ( (changed & SCL) && (data & SCL) )
		{ // clocked in new data
			int pattern = 1 << (7-state->m_e2cnt);

			if ( data & SDA ) state->m_e2data |=  pattern;
			else              state->m_e2data &= ~pattern;

			state->m_e2data_pin = state->m_e2data_to_read & 0x80 ? 1 : 0;

			state->m_e2data_to_read <<= 1;

			LOG(("e2d pin= %d\n", state->m_e2data_pin));

			state->m_e2cnt++;
			if ( state->m_e2cnt >= 8 )
			{
				res++;
			}
		}
	}

	return res;
}

static int recAck(int changed, int data)
{
	int result = 0;

	if ( (changed & SCL) && (data & SCL) )
	{
		if ( data & SDA )
		{
			result = 1;
		}
		else
		{
			result = -1;
		}
	}
	return result;
}

///////////////////////////////////////////////////////////////////////////


/* VFD Status */
static READ8_HANDLER( vfd_status_r )
{
	bfm_sc2_state *state = space->machine().driver_data<bfm_sc2_state>();
	/* b7 = NEC busy */
	/* b6 = alpha busy (also matrix board) */
	/* b5 - b0 = reel optics */

	int result = state->m_optic_pattern;

	if ( !upd7759_busy_r(space->machine().device("upd")) ) result |= 0x80;

	if (space->machine().device("matrix"))
		if ( BFM_dm01_busy() ) result |= 0x40;

	return result;
}

static WRITE8_HANDLER( vfd1_data_w )
{
	bfm_sc2_state *state = space->machine().driver_data<bfm_sc2_state>();
	state->m_vfd1_latch = data;

	if (space->machine().device("matrix"))
	{
		BFM_dm01_writedata(space->machine(),data);
	}
	else
	{
		BFM_BD1_newdata(0, data);
		BFM_BD1_draw(0);
	}
}


//
static WRITE8_HANDLER( e2ram_w )
{
	bfm_sc2_state *state = space->machine().driver_data<bfm_sc2_state>(); // b0 = clock b1 = data

	int changed, ack;

	data ^= (SDA|SCL);  // invert signals

	changed  = (state->m_e2reg^data) & 0x03;

	state->m_e2reg = data;

	if ( changed )
	{
		while ( 1 )
		{
			if ( (  (changed & SDA) && !(data & SDA))	&&  // 1->0 on SDA  AND
				( !(changed & SCL) && (data & SCL) )    // SCL=1 and not changed
				)
			{	// X24C08 Start condition (1->0 on SDA while SCL=1)
				state->m_e2dummywrite = ( state->m_e2state == 5 );

				LOG(("e2ram:   c:%d d:%d Start condition dummywrite=%d\n", (data & SCL)?1:0, (data&SDA)?1:0, state->m_e2dummywrite ));

				state->m_e2state = 1; // ready for commands
				state->m_e2cnt   = 0;
				state->m_e2data  = 0;
				break;
			}

			if ( (  (changed & SDA) && (data & SDA))	&&  // 0->1 on SDA  AND
				( !(changed & SCL) && (data & SCL) )     // SCL=1 and not changed
				)
			{	// X24C08 Stop condition (0->1 on SDA while SCL=1)
				LOG(("e2ram:   c:%d d:%d Stop condition\n", (data & SCL)?1:0, (data&SDA)?1:0 ));
				state->m_e2state = 0;
				state->m_e2data  = 0;
				break;
			}

			switch ( state->m_e2state )
			{
				case 1: // Receiving address + R/W bit

					if ( recdata(state, changed, data) )
					{
						state->m_e2address = (state->m_e2address & 0x00FF) | ((state->m_e2data>>1) & 0x03) << 8;
						state->m_e2cnt   = 0;
						state->m_e2rw    = state->m_e2data & 1;

						LOG(("e2ram: Slave address received !!  device id=%01X device adr=%01d high order adr %0X RW=%d) %02X\n",
							state->m_e2data>>4, (state->m_e2data & 0x08)?1:0, (state->m_e2data>>1) & 0x03, state->m_e2rw , state->m_e2data ));

						state->m_e2state = 2;
					}
					break;

				case 2: // Receive Acknowledge

					ack = recAck(changed,data);
					if ( ack )
					{
						state->m_e2data_pin = 0;

						if ( ack < 0 )
						{
							LOG(("ACK = 0\n"));
							state->m_e2state = 0;
						}
						else
						{
							LOG(("ACK = 1\n"));
							if ( state->m_e2dummywrite )
							{
								state->m_e2dummywrite = 0;

								state->m_e2data_to_read = state->m_e2ram[state->m_e2address];

								if ( state->m_e2rw & 1 ) state->m_e2state = 7; // read data
								else		  state->m_e2state = 0; //?not sure
							}
							else
							{
								if ( state->m_e2rw & 1 ) state->m_e2state = 7; // reading
								else            state->m_e2state = 3; // writing
							}
							switch ( state->m_e2state )
							{
								case 7:
									LOG(("read address %04X\n",state->m_e2address));
									state->m_e2data_to_read = state->m_e2ram[state->m_e2address];
									break;
								case 3:
									LOG(("write, awaiting address\n"));
									break;
								default:
									LOG(("?unknow action %04X\n",state->m_e2address));
									break;
							}
						}
						state->m_e2data = 0;
					}
					break;

				case 3: // writing data, receiving address

					if ( recdata(state, changed, data) )
					{
						state->m_e2data_pin = 0;
						state->m_e2address = (state->m_e2address & 0xFF00) | state->m_e2data;

						LOG(("write address = %04X waiting for ACK\n", state->m_e2address));
						state->m_e2state = 4;
						state->m_e2cnt   = 0;
						state->m_e2data  = 0;
					}
					break;

				case 4: // wait ack, for write address

					ack = recAck(changed,data);
					if ( ack )
					{
						state->m_e2data_pin = 0;	// pin=0, no error !!

						if ( ack < 0 )
						{
							state->m_e2state = 0;
							LOG(("ACK = 0, cancel write\n" ));
						}
						else
						{
							state->m_e2state = 5;
							LOG(("ACK = 1, awaiting data to write\n" ));
						}
					}
					break;

				case 5: // receive data to write
					if ( recdata(state, changed, data) )
					{
						LOG(("write data = %02X received, awaiting ACK\n", state->m_e2data));
						state->m_e2cnt   = 0;
						state->m_e2state = 6;  // wait ack
					}
					break;

				case 6: // Receive Acknowlede after writing

					ack = recAck(changed,data);
					if ( ack )
					{
						if ( ack < 0 )
						{
							state->m_e2state = 0;
							LOG(("ACK=0, write canceled\n"));
						}
						else
						{
							LOG(("ACK=1, writing %02X to %04X\n", state->m_e2data, state->m_e2address));

							state->m_e2ram[state->m_e2address] = state->m_e2data;

							state->m_e2address = (state->m_e2address & ~0x000F) | ((state->m_e2address+1)&0x0F);

							state->m_e2state = 5; // write next address
						}
					}
					break;

				case 7: // receive address from read

					if ( recdata(state, changed, data) )
					{
						//state->m_e2data_pin = 0;

						LOG(("address read, data = %02X waiting for ACK\n", state->m_e2data ));

						state->m_e2state = 8;
					}
					break;

				case 8:

					if ( recAck(changed, data) )
					{
						state->m_e2state = 7;

						state->m_e2address = (state->m_e2address & ~0x0F) | ((state->m_e2address+1)&0x0F); // lower 4 bits wrap around

						state->m_e2data_to_read = state->m_e2ram[state->m_e2address];

						LOG(("ready for next address %04X\n", state->m_e2address));

						state->m_e2cnt   = 0;
						state->m_e2data  = 0;
					}
					break;

				case 0:

					LOG(("e2ram: ? c:%d d:%d\n", (data & SCL)?1:0, (data&SDA)?1:0 ));
					break;
			}
			break;
		}
	}
}

static int read_e2ram(running_machine &machine)
{
	bfm_sc2_state *state = machine.driver_data<bfm_sc2_state>();
	LOG(("e2ram: r %d (%02X) \n", state->m_e2data_pin, state->m_e2data_to_read ));

	return state->m_e2data_pin;
}



// machine init (called only once) ////////////////////////////////////////

static MACHINE_RESET( init )
{
	// reset adder2
	MACHINE_RESET_CALL(adder2);

	// reset the board //////////////////////////////////////////////////////

	on_scorpion2_reset(machine);
	BFM_BD1_init(0);
	BFM_BD1_init(1);
}

static SCREEN_UPDATE( addersc2 )
{
	bfm_sc2_state *state = screen->machine().driver_data<bfm_sc2_state>();
	if ( state->m_sc2_show_door )
	{
		output_set_value("door",( Scorpion2_GetSwitchState(screen->machine(),state->m_sc2_door_state>>4, state->m_sc2_door_state & 0x0F) ) );
	}

	return SCREEN_UPDATE_CALL(adder2);
}


static READ8_HANDLER( direct_input_r )
{
	return 0;
}




static ADDRESS_MAP_START( sc2_basemap, AS_PROGRAM, 8 )
	AM_RANGE(0x0000, 0x1fff) AM_RAM AM_SHARE("nvram") //8k
	AM_RANGE(0x2000, 0x2000) AM_READ(vfd_status_r)
	AM_RANGE(0x2000, 0x20FF) AM_WRITE(reel12_w)
	AM_RANGE(0x2100, 0x21FF) AM_WRITE(reel34_w)
	AM_RANGE(0x2200, 0x22FF) AM_WRITE(reel56_w)

	AM_RANGE(0x2300, 0x230B) AM_READ(mux_input_r)
	AM_RANGE(0x2300, 0x231F) AM_WRITE(mux_output_w)
	AM_RANGE(0x2320, 0x2323) AM_WRITE(dimas_w)				/* ?unknown dim related */

	AM_RANGE(0x2324, 0x2324) AM_READWRITE(expansion_latch_r, expansion_latch_w)
	AM_RANGE(0x2325, 0x2327) AM_WRITE(unknown_w)
	AM_RANGE(0x2328, 0x2328) AM_WRITE(muxena_w)
	AM_RANGE(0x2329, 0x2329) AM_READWRITE(timerirqclr_r, timerirq_w)
	AM_RANGE(0x232A, 0x232D) AM_WRITE(unknown_w)
	AM_RANGE(0x232E, 0x232E) AM_READ(irqstatus_r)

	AM_RANGE(0x232F, 0x232F) AM_WRITE(coininhib_w)
	AM_RANGE(0x2330, 0x2330) AM_WRITE(payout_latch_w)
	AM_RANGE(0x2331, 0x2331) AM_WRITE(payout_triac_w)
	AM_RANGE(0x2332, 0x2332) AM_WRITE(watchdog_reset_w)
	AM_RANGE(0x2333, 0x2333) AM_WRITE(mmtr_w)
	AM_RANGE(0x2334, 0x2335) AM_WRITE(unknown_w)
	AM_RANGE(0x2336, 0x2336) AM_WRITE(dimcnt_w)
	AM_RANGE(0x2337, 0x2337) AM_WRITE(volume_override_w)
	AM_RANGE(0x2338, 0x2338) AM_WRITE(payout_select_w)
	AM_RANGE(0x2339, 0x2339) AM_WRITE(unknown_w)
	AM_RANGE(0x2400, 0x2400) AM_READWRITE(uart1stat_r, uart1ctrl_w)	/* mc6850 compatible uart */
	AM_RANGE(0x2500, 0x2500) AM_READWRITE(uart1data_r, uart1data_w)
	AM_RANGE(0x2600, 0x2600) AM_READWRITE(uart2stat_r, uart2ctrl_w)	/* mc6850 compatible uart */
	AM_RANGE(0x2700, 0x2700) AM_READWRITE(uart2data_r, uart2data_w)
	AM_RANGE(0x2800, 0x2800) AM_WRITE(vfd1_data_w)					/* vfd1 data */
	AM_RANGE(0x2900, 0x2900) AM_WRITE(vfd_reset_w)					/* vfd1+vfd2 reset line */
	AM_RANGE(0x2A00, 0x2AFF) AM_DEVWRITE("upd", nec_latch_w)
	AM_RANGE(0x2B00, 0x2BFF) AM_DEVWRITE("upd", nec_reset_w)
	AM_RANGE(0x2C00, 0x2C00) AM_WRITE(unlock_w)						/* custom chip unlock */
	AM_RANGE(0x2D00, 0x2D01) AM_DEVWRITE("ymsnd", ym2413_w)
	AM_RANGE(0x2E00, 0x2E00) AM_WRITE(bankswitch_w)					/* write bank (rom page select for 0x6000 - 0x7fff ) */
	AM_RANGE(0x2F00, 0x2F00) AM_WRITE(vfd2_data_w)					/* vfd2 data */

	AM_RANGE(0x3FFE, 0x3FFE) AM_READ( direct_input_r )
	AM_RANGE(0x3FFF, 0x3FFF) AM_READ( coin_input_r)
	AM_RANGE(0x4000, 0x5FFF) AM_ROM
	AM_RANGE(0x4000, 0xFFFF) AM_WRITE(unknown_w)			// contains unknown I/O registers
	AM_RANGE(0x6000, 0x7FFF) AM_ROMBANK("bank1")
	AM_RANGE(0x8000, 0xFFFF) AM_ROM
ADDRESS_MAP_END

// memory map for scorpion2 board video addon /////////////////////////////

static ADDRESS_MAP_START( memmap_vid, AS_PROGRAM, 8 )
	AM_IMPORT_FROM( sc2_basemap )

	AM_RANGE(0x2000, 0x2000) AM_READ(vfd_status_hop_r)		// vfd status register
	AM_RANGE(0x2000, 0x20FF) AM_WRITE(reel12_vid_w)
	AM_RANGE(0x2100, 0x21FF) AM_WRITENOP
	AM_RANGE(0x2200, 0x22FF) AM_WRITENOP

	AM_RANGE(0x3C00, 0x3C07) AM_READ(  key_r   )
	AM_RANGE(0x3C80, 0x3C80) AM_WRITE( e2ram_w )

	AM_RANGE(0x3E00, 0x3E00) AM_READWRITE(vid_uart_ctrl_r, vid_uart_ctrl_w)		// video uart control reg
	AM_RANGE(0x3E01, 0x3E01) AM_READWRITE(vid_uart_rx_r, vid_uart_tx_w)			// video uart data  reg
ADDRESS_MAP_END

// input ports for pyramid ////////////////////////////////////////

static INPUT_PORTS_START( pyramid )
	PORT_START("COINS")
    PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_COIN1 ) PORT_IMPULSE(3) PORT_NAME("Fl 5.00")
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_COIN2 ) PORT_IMPULSE(3) PORT_NAME("Fl 2.50")
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_COIN3 ) PORT_IMPULSE(3) PORT_NAME("Fl 1.00")
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_COIN4 ) PORT_IMPULSE(3) PORT_NAME("Fl 0.50")
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_UNUSED)

	PORT_START("STROBE0")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_UNUSED)

	PORT_START("STROBE1")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_START1 )
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_BUTTON1) PORT_NAME("Left")
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_BUTTON2) PORT_NAME("Up")
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_UNUSED)

	PORT_START("STROBE2")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_BUTTON3) PORT_NAME("Right")
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_OTHER) PORT_NAME("Enter") PORT_CODE(KEYCODE_E)
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_OTHER) PORT_NAME("Collect") PORT_CODE(KEYCODE_C)
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_UNUSED)

	PORT_START("STROBE3")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_UNUSED)

	PORT_START("STROBE4")
	PORT_BIT( 0x01, IP_ACTIVE_LOW,  IPT_INTERLOCK) PORT_NAME("Cashbox Door") PORT_CODE(KEYCODE_Q) PORT_TOGGLE
	PORT_BIT( 0x02, IP_ACTIVE_LOW,  IPT_INTERLOCK) PORT_NAME("Front Door") PORT_CODE(KEYCODE_W) PORT_TOGGLE
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_OTHER)     PORT_NAME("Refill Key") PORT_CODE(KEYCODE_R) PORT_TOGGLE
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_UNUSED)

	PORT_START("STROBE5")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_UNUSED)

	PORT_START("STROBE6")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_UNUSED)

	PORT_START("STROBE7")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_UNUSED)

	PORT_START("STROBE8")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_UNUSED)

	PORT_START("STROBE9")
	PORT_SERVICE_NO_TOGGLE(0x01,IP_ACTIVE_HIGH)
	PORT_DIPNAME( 0x02, 0x00, "Coin 1 Lockout")PORT_DIPLOCATION("DIL:!02")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x02, DEF_STR( On ) )
	PORT_DIPNAME( 0x04, 0x00, "Coin 2 Lockout")PORT_DIPLOCATION("DIL:!03")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x04, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x00, "Coin 3 Lockout")PORT_DIPLOCATION("DIL:!04")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x08, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x00, "Coin 4 Lockout")PORT_DIPLOCATION("DIL:!05")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x10, DEF_STR( On ) )

	PORT_START("STROBE10")
	PORT_DIPUNKNOWN_DIPLOC( 0x01, 0x00, "DIL:!06" )
	PORT_DIPNAME( 0x02, 0x00, "Attract mode language" ) PORT_DIPLOCATION("DIL:!07")
	PORT_DIPSETTING(    0x00, DEF_STR( English ) )
	PORT_DIPSETTING(    0x02, "Dutch"       )
	PORT_DIPNAME( 0x0C, 0x00, "Skill Level" ) PORT_DIPLOCATION("DIL:!08,!10")
	PORT_DIPSETTING(    0x00, DEF_STR( Low ) )
	PORT_DIPSETTING(    0x04, "Medium-Low" )
	PORT_DIPSETTING(    0x08, "Medium-High")
	PORT_DIPSETTING(    0x0C, DEF_STR( High ) )
	PORT_DIPUNKNOWN_DIPLOC( 0x10, 0x00, "DIL:!11" )

	PORT_START("STROBE11")
	PORT_DIPUNKNOWN_DIPLOC( 0x01, 0x00, "DIL:!12" )
	PORT_DIPUNKNOWN_DIPLOC( 0x02, 0x00, "DIL:!13" )
	PORT_DIPNAME( 0x04, 0x00, "Attract mode" ) PORT_DIPLOCATION("DIL:!14")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x04, DEF_STR( On  ) )
	PORT_DIPNAME( 0x18, 0x00, "Stake" ) PORT_DIPLOCATION("DIL:!15,!16")
	PORT_DIPSETTING(    0x00, "4 credits per game"  )
	PORT_DIPSETTING(    0x08, "1 credit  per round" )
	PORT_DIPSETTING(    0x10, "2 credit  per round" )
	PORT_DIPSETTING(    0x18, "4 credits per round" )
INPUT_PORTS_END

// input ports for golden crown ///////////////////////////////////

static INPUT_PORTS_START( gldncrwn )
	PORT_INCLUDE( pyramid )

	PORT_MODIFY("COINS")
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_COIN4 ) PORT_IMPULSE(3) PORT_NAME("Fl 0.25")

	PORT_MODIFY("STROBE1")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_OTHER)   PORT_NAME( "Collect") PORT_CODE(KEYCODE_C)
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_BUTTON1) PORT_NAME( "Reel 1" )
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_BUTTON2) PORT_NAME( "Reel 2" )
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_BUTTON3) PORT_NAME( "Reel 3" )
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_UNUSED)

	PORT_MODIFY("STROBE2")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_BUTTON4) PORT_NAME( "Reel 4" )
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_BUTTON5) PORT_NAME( "Reel 5" )
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_BUTTON6) PORT_NAME( "Reel 6" )
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_START1 )
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_NAME( "Hall Of Fame" ) PORT_CODE( KEYCODE_J )
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_UNUSED)

	PORT_MODIFY("STROBE10")
	PORT_DIPNAME( 0x01, 0x00, "Attract mode language" )PORT_DIPLOCATION("DIL:!06")
	PORT_DIPSETTING(    0x00, "Dutch")
	PORT_DIPSETTING(    0x01, DEF_STR( English ) )
	PORT_DIPNAME( 0x02, 0x00, "Max number of spins" )PORT_DIPLOCATION("DIL:!07")
	PORT_DIPSETTING(    0x00, "99")
	PORT_DIPSETTING(    0x02, "50")
	PORT_DIPNAME( 0x0C, 0x00, "Skill Level" )PORT_DIPLOCATION("DIL:!08,!10")
	PORT_DIPSETTING(    0x00, DEF_STR( Low ))
	PORT_DIPSETTING(    0x04, "Medium-Low"  )
	PORT_DIPSETTING(    0x08, "Medium-High" )
	PORT_DIPSETTING(    0x0C, DEF_STR( High ) )
	PORT_DIPNAME( 0x10, 0x00, "Base Pricing on:" )PORT_DIPLOCATION("DIL:!11")
	PORT_DIPSETTING(    0x00, "Full Game")
	PORT_DIPSETTING(    0x10, "Individual Rounds")

	PORT_MODIFY("STROBE11")
	PORT_DIPNAME( 0x01, 0x00, "Credits required:" )PORT_DIPLOCATION("DIL:!12")
	PORT_DIPSETTING(    0x00, "4 credits per game")PORT_CONDITION("STROBE10",0x10,PORTCOND_EQUALS,0x00)
	PORT_DIPSETTING(    0x01, "2 credits per game")PORT_CONDITION("STROBE10",0x10,PORTCOND_EQUALS,0x00)
	PORT_DIPSETTING(    0x00, "1 credit  per round")PORT_CONDITION("STROBE10",0x10,PORTCOND_EQUALS,0x10)
	PORT_DIPSETTING(    0x01, "4 credits per round")PORT_CONDITION("STROBE10",0x10,PORTCOND_EQUALS,0x10)
	PORT_DIPNAME( 0x02, 0x00, "Attract Mode" )PORT_DIPLOCATION("DIL:!13")
	PORT_DIPSETTING(    0x02, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On  ) )
	PORT_DIPNAME( 0x04, 0x00, "Time bar" )PORT_DIPLOCATION("DIL:!14")
	PORT_DIPSETTING(    0x04, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On  ) )
	PORT_DIPNAME( 0x18, 0x00, "Time bar speed" )PORT_DIPLOCATION("DIL:!15,!16")
	PORT_DIPSETTING(    0x00, "1 (fast)" )
	PORT_DIPSETTING(    0x08, "2" )
	PORT_DIPSETTING(    0x10, "3" )
	PORT_DIPSETTING(    0x18, "4 (slow)" )
INPUT_PORTS_END

// input ports for dutch quintoon /////////////////////////////////

static INPUT_PORTS_START( qntoond )
	PORT_INCLUDE( pyramid )

	PORT_MODIFY("COINS")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_COIN1 ) PORT_IMPULSE(3) PORT_NAME("Fl 5.00")
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_COIN2 ) PORT_IMPULSE(3) PORT_NAME("Fl 2.50")
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_COIN3 ) PORT_IMPULSE(3) PORT_NAME("Fl 1.00")
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_COIN4 ) PORT_IMPULSE(3) PORT_NAME("Fl 0.50")

	PORT_MODIFY("STROBE1")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_OTHER)	  PORT_NAME("Collect") PORT_CODE(KEYCODE_C)
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_NAME("Hand 1" )
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_BUTTON2 ) PORT_NAME("Hand 2" )
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_BUTTON3 ) PORT_NAME("Hand 3" )
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_BUTTON4 ) PORT_NAME("Hand 4" )

	PORT_MODIFY("STROBE2")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_BUTTON5 ) PORT_NAME("Hand 5" )
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_START1 )
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_UNUSED)

	PORT_MODIFY("STROBE9")
	PORT_SERVICE_NO_TOGGLE(0x01,IP_ACTIVE_HIGH)
	PORT_DIPNAME( 0x1e, 0x1c, DEF_STR( Coinage ) ) PORT_DIPLOCATION("DIL:!02,!03,!04,!05")
	PORT_DIPSETTING(    0x00, DEF_STR( Free_Play ) )
	PORT_DIPSETTING(    0x02, "C1=2.5 C2=1.25 C3=0.5 C4=2C_0.25C" )
	PORT_DIPSETTING(    0x04, "C1=10 C2=5 C3=2 C4=0.5" )
	PORT_DIPSETTING(    0x06, "C1=1.5/3.25/5 C2=0.75/1.5/3.25 C3=0.25/0.5/1 C4=3C_0.25C" )
	PORT_DIPSETTING(    0x08, "C1=20 C2=10 C3=4 C4=1" )
	PORT_DIPSETTING(    0x0a, "C1=2 C2=1 C3=0.25/0.75/1/1.5/2 C4=3C_0.25C 5C_0.5C" )
	PORT_DIPSETTING(    0x0c, "C1=5 C2=2.5 C3=1 C4=0.25" )
	PORT_DIPSETTING(    0x0e, "C1=1.25 C2=0.5/1.25/1.75 C3=0.25 C4=0.25" )
	//PORT_DIPSETTING(    0x10, DEF_STR( Free_Play ) )
	PORT_DIPSETTING(    0x12, "C1=3 C2=1.5 C3=0.5 C4=0.25" )
	PORT_DIPSETTING(    0x14, "C1=12 C2=6 C3=2 C4=0.5" )
	PORT_DIPSETTING(    0x16, "C1=2 C2=1 C3=0.25 C4=3C_0.25C" )
	PORT_DIPSETTING(    0x18, "C1=24 C2=12 C3=4 C4=1" )
	PORT_DIPSETTING(    0x1a, "C1=2.25/4.75 C2=1/2.25/3.5/4.75/6 C3=0.25/0.75/1/1.5/2 C4=3C_0.25C 5C_0.5C" )
	PORT_DIPSETTING(    0x1c, "C1=6 C2=3 C3=1 C4=0.25" )
	PORT_DIPSETTING(    0x1e, "C1=1.5 C2=0.75 C3=0.25 C4=4C_0.25C" )

	PORT_MODIFY("STROBE10")
	PORT_DIPNAME( 0x01, 0x00, "Coin 1 Lockout")PORT_DIPLOCATION("DIL:!06")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x01, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x00, "Coin 2 Lockout")PORT_DIPLOCATION("DIL:!07")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x02, DEF_STR( On ) )
	PORT_DIPNAME( 0x04, 0x00, "Coin 3 Lockout")PORT_DIPLOCATION("DIL:!08")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x04, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x00, "Coin 4 Lockout")PORT_DIPLOCATION("DIL:!10")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x08, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x00, "Coin Jam Alarm" )PORT_DIPLOCATION("DIL:!11")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x10, DEF_STR( On  ) )

	PORT_MODIFY("STROBE11")
	PORT_DIPNAME( 0x01, 0x00, "Time bar" )PORT_DIPLOCATION("DIL:!12")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x01, DEF_STR( On  ) )
	PORT_DIPNAME( 0x02, 0x00, "Clear credits on reset" )PORT_DIPLOCATION("DIL:!13")
	PORT_DIPSETTING(    0x00, DEF_STR( No ) )
	PORT_DIPSETTING(    0x02, DEF_STR( Yes  ) )
	PORT_DIPNAME( 0x04, 0x00, "Attract mode" )PORT_DIPLOCATION("DIL:!14")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x04, DEF_STR( On  ) )
	PORT_DIPNAME( 0x08, 0x00, "Attract mode language" )PORT_DIPLOCATION("DIL:!15")
	PORT_DIPSETTING(    0x00, DEF_STR( English  ) )
	PORT_DIPSETTING(    0x08, "Dutch"    )
	PORT_DIPUNKNOWN_DIPLOC( 0x10, 0x00, "DIL:!16" )
INPUT_PORTS_END

// input ports for UK quintoon ////////////////////////////////////////////

static INPUT_PORTS_START( quintoon )
	PORT_INCLUDE( pyramid )

	PORT_MODIFY("COINS")
    PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_COIN1 ) PORT_IMPULSE(3) PORT_NAME("10p")
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_COIN2 ) PORT_IMPULSE(3) PORT_NAME("20p")
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_COIN3 ) PORT_IMPULSE(3) PORT_NAME("50p")
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_COIN4 ) PORT_IMPULSE(3) PORT_NAME("GBP 1.00")

	PORT_MODIFY("STROBE1")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_OTHER)   PORT_NAME("Collect") PORT_CODE(KEYCODE_C)
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_BUTTON1) PORT_NAME("Hand 1")
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_BUTTON2) PORT_NAME("Hand 2")
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_BUTTON3) PORT_NAME("Hand 3")
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_BUTTON4) PORT_NAME("Hand 4")

	PORT_MODIFY("STROBE2")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_BUTTON5) PORT_NAME("Hand 5")
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_BUTTON7) PORT_NAME("?1") PORT_CODE(KEYCODE_U)
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_START1)
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_BUTTON8) PORT_NAME("?2") PORT_CODE(KEYCODE_I)
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_BUTTON9) PORT_NAME("?3") PORT_CODE(KEYCODE_O)

	PORT_MODIFY("STROBE5")
	PORT_BIT( 0x04, IP_ACTIVE_LOW,  IPT_SPECIAL) //Payout opto

	PORT_MODIFY("STROBE9")
	PORT_DIPUNKNOWN_DIPLOC( 0x02, 0x00, "DIL:!02" )
	PORT_DIPUNKNOWN_DIPLOC( 0x04, 0x00, "DIL:!03" )
	PORT_DIPUNKNOWN_DIPLOC( 0x08, 0x00, "DIL:!04" )
	PORT_DIPUNKNOWN_DIPLOC( 0x10, 0x00, "DIL:!05" )

	PORT_MODIFY("STROBE10")
	PORT_DIPNAME( 0x01, 0x00, "Coin Lockout")PORT_DIPLOCATION("DIL:!06")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) ) //Will activate coin lockout when Credit >= 1 Play
	PORT_DIPSETTING(    0x01, DEF_STR( On ) )
	PORT_DIPUNKNOWN_DIPLOC( 0x02, 0x00, "DIL:!07" )
	PORT_DIPUNKNOWN_DIPLOC( 0x04, 0x00, "DIL:!08" )
	PORT_DIPUNKNOWN_DIPLOC( 0x08, 0x00, "DIL:!10" )
	PORT_DIPNAME( 0x10, 0x00, "Stake per Game / Jackpot" )PORT_DIPLOCATION("DIL:!11")
	PORT_DIPSETTING(    0x00, "20p / 6 Pounds" )
	PORT_DIPSETTING(    0x10, "50p / 20 Pounds" )

	PORT_MODIFY("STROBE11")
	PORT_DIPUNKNOWN_DIPLOC( 0x01, 0x00, "DIL:!12" )
	PORT_DIPUNKNOWN_DIPLOC( 0x02, 0x00, "DIL:!13" )
	PORT_DIPNAME( 0x1C, 0x00, "Target percentage" )PORT_DIPLOCATION("DIL:!14,!15,!16")
	PORT_DIPSETTING(    0x1C, "50%")
	PORT_DIPSETTING(    0x0C, "55%")
	PORT_DIPSETTING(    0x08, "60%")
	PORT_DIPSETTING(    0x18, "65%")
	PORT_DIPSETTING(    0x10, "70%")
	PORT_DIPSETTING(    0x00, "75%")
	PORT_DIPSETTING(    0x04, "80%")
	PORT_DIPSETTING(    0x14, "85%")
INPUT_PORTS_END

// input ports for slotsnl  ///////////////////////////////////////////////

static INPUT_PORTS_START( slotsnl )
	PORT_INCLUDE( pyramid )

	PORT_MODIFY("COINS")
    PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_COIN1 ) PORT_IMPULSE(3) PORT_NAME("Fl 0.25")
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_COIN2 ) PORT_IMPULSE(3) PORT_NAME("Fl 1.00")
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_COIN3 ) PORT_IMPULSE(3) PORT_NAME("Fl 2.50")
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_COIN4 ) PORT_IMPULSE(3) PORT_NAME("Fl 5.00")

	PORT_MODIFY("STROBE1")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_BUTTON1) PORT_NAME("Slot 1")
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_BUTTON2) PORT_NAME("Slot 2")
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_MODIFY("STROBE2")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_BUTTON3)  PORT_NAME("Slot 3")
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_BUTTON4)  PORT_NAME("Slot 4")
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_OTHER )   PORT_NAME("Enter") PORT_CODE( KEYCODE_E )
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_START2 )

	PORT_MODIFY("STROBE3")
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_START1)

	PORT_MODIFY("STROBE10")
	PORT_DIPUNKNOWN_DIPLOC( 0x01, 0x00, "DIL:!06" )
	PORT_DIPUNKNOWN_DIPLOC( 0x02, 0x00, "DIL:!07" )
	PORT_DIPUNKNOWN_DIPLOC( 0x04, 0x00, "DIL:!08" )
	PORT_DIPUNKNOWN_DIPLOC( 0x08, 0x00, "DIL:!10" )
	PORT_DIPNAME( 0x10, 0x00, "Coin Jam Alarm" )PORT_DIPLOCATION("DIL:!11")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x10, DEF_STR( On  ) )

	PORT_MODIFY("STROBE11")
	PORT_DIPUNKNOWN_DIPLOC( 0x01, 0x00, "DIL:!12" )
	PORT_DIPUNKNOWN_DIPLOC( 0x02, 0x00, "DIL:!13" )
	PORT_DIPNAME( 0x04, 0x00, "Attract mode" )PORT_DIPLOCATION("DIL:!14")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x04, DEF_STR( On  ) )
	PORT_DIPNAME( 0x18, 0x00, "Timebar speed" )PORT_DIPLOCATION("DIL:!15,!16")
	PORT_DIPSETTING(    0x00, "1" )
	PORT_DIPSETTING(    0x10, "2" )
	PORT_DIPSETTING(    0x08, "3" )
	PORT_DIPSETTING(    0x18, "4" )
INPUT_PORTS_END

// input ports for sltblgtk  //////////////////////////////////////////////

static INPUT_PORTS_START( sltblgtk )
	PORT_INCLUDE( pyramid )

	PORT_MODIFY("COINS")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_COIN1 ) PORT_IMPULSE(3) PORT_NAME("Token")
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_COIN2 ) PORT_IMPULSE(3) PORT_NAME("20 BFr")
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_COIN3 ) PORT_IMPULSE(3) PORT_NAME("50 BFr")

	PORT_MODIFY("STROBE1")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_BUTTON1) PORT_NAME("Slot 1")
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_BUTTON2) PORT_NAME("Slot 2")
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_MODIFY("STROBE2")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_BUTTON3) PORT_NAME("Slot 3")
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_BUTTON4) PORT_NAME("Slot 4")
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_OTHER  ) PORT_NAME("Enter") PORT_CODE( KEYCODE_E )
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_START2 )

	PORT_MODIFY("STROBE3")
	PORT_BIT( 0x01, IP_ACTIVE_LOW,  IPT_SPECIAL ) //Tube 1
	PORT_BIT( 0x02, IP_ACTIVE_LOW,  IPT_SPECIAL ) //Tube 2
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_START1)

	PORT_MODIFY("STROBE9")
	PORT_SERVICE_NO_TOGGLE(0x01,IP_ACTIVE_HIGH)
	PORT_DIPNAME( 0x02, 0x00, "CashMeters in refill menu" )PORT_DIPLOCATION("DIL:!02")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x02, DEF_STR( On  ) )
	PORT_DIPNAME( 0x04, 0x00, "Token Lockout" )PORT_DIPLOCATION("DIL:!03")
	PORT_DIPSETTING(    0x00, DEF_STR( No  ) )
	PORT_DIPSETTING(    0x04, DEF_STR( Yes ) )
	PORT_DIPNAME( 0x08, 0x00, "20 Bfr Lockout" )PORT_DIPLOCATION("DIL:!04")
	PORT_DIPSETTING(    0x00, DEF_STR( No  ) )
	PORT_DIPSETTING(    0x08, DEF_STR( Yes ) )
	PORT_DIPNAME( 0x10, 0x00, "50 Bfr Lockout" )PORT_DIPLOCATION("DIL:!05")
	PORT_DIPSETTING(    0x00, DEF_STR( No ) )
	PORT_DIPSETTING(    0x10, DEF_STR( Yes  ) )

	PORT_MODIFY("STROBE10")
	PORT_DIPUNKNOWN_DIPLOC( 0x01, 0x00, "DIL:!06" )
	PORT_DIPNAME( 0x0E, 0x00, "Payout Percentage" )PORT_DIPLOCATION("DIL:!07,!08,!10")
	PORT_DIPSETTING(    0x00, "60%")
	PORT_DIPSETTING(    0x08, "65%")
	PORT_DIPSETTING(    0x04, "70%")
	PORT_DIPSETTING(    0x0C, "75%")
	PORT_DIPSETTING(    0x02, "80%")
	PORT_DIPSETTING(    0x0A, "84%")
	PORT_DIPSETTING(    0x06, "88%")
	PORT_DIPSETTING(    0x0E, "90%")
	PORT_DIPNAME( 0x10, 0x00, "Coin Jam Alarm" )PORT_DIPLOCATION("DIL:!11")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x10, DEF_STR( On  ) )

	PORT_MODIFY("STROBE11")
	PORT_DIPNAME( 0x01, 0x00, "Timebar" )PORT_DIPLOCATION("DIL:!12")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x01, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x00, "Clear credits" )PORT_DIPLOCATION("DIL:!13")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x02, DEF_STR( On ) )
	PORT_DIPNAME( 0x04, 0x00, "Attract mode" )PORT_DIPLOCATION("DIL:!14")
	PORT_DIPSETTING(    0x00, DEF_STR( Off  ) )
	PORT_DIPSETTING(    0x04, DEF_STR( On   ) )
	PORT_DIPNAME( 0x08, 0x00, "Show hints" )PORT_DIPLOCATION("DIL:!15")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x08, DEF_STR( On  ) )
	PORT_DIPNAME( 0x10, 0x00, "Pay win to credits" )PORT_DIPLOCATION("DIL:!16")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x10, DEF_STR( On  ) )
INPUT_PORTS_END

// input ports for sltblgpo  //////////////////////////////////////////////

static INPUT_PORTS_START( sltblgpo )
	PORT_INCLUDE( pyramid )

	PORT_MODIFY("COINS")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_COIN1 ) PORT_IMPULSE(3) PORT_NAME("Bfr 20")
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_COIN2 ) PORT_IMPULSE(3) PORT_NAME("Bfr 50")

	PORT_MODIFY("STROBE1")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_BUTTON1) PORT_NAME("Hand 1")
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_BUTTON2) PORT_NAME("Hand 2")
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_UNUSED)

	PORT_MODIFY("STROBE2")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_BUTTON3)  PORT_NAME("Hand 3")
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_BUTTON4)  PORT_NAME("Hand 4")
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_START1)
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_BUTTON1)  PORT_NAME("Stake")  PORT_CODE( KEYCODE_O )

	PORT_MODIFY("STROBE3")
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_OTHER) PORT_NAME("Collect") PORT_CODE(KEYCODE_C)

	PORT_MODIFY("STROBE9")
	PORT_SERVICE_NO_TOGGLE(0x01,IP_ACTIVE_HIGH)
	PORT_DIPNAME( 0x02, 0x00, "Hopper Limit" )PORT_DIPLOCATION("DIL:!02")
	PORT_DIPSETTING(    0x00, "300" )
	PORT_DIPSETTING(    0x02, "500" )
	PORT_DIPUNKNOWN_DIPLOC( 0x04, 0x00, "DIL:!03" )
	PORT_DIPNAME( 0x18, 0x00, "Attendant payout" )PORT_DIPLOCATION("DIL:!04,!05")
	PORT_DIPSETTING(    0x00, "1000 Bfr" )
	PORT_DIPSETTING(    0x08, "1250 Bfr" )
	PORT_DIPSETTING(    0x10, "1500 Bfr" )
	PORT_DIPSETTING(    0x18, "1750 Bfr" )

	PORT_MODIFY("STROBE10")
	PORT_DIPNAME( 0x01, 0x00, "Bfr 20 Lockout" )PORT_DIPLOCATION("DIL:!06")
	PORT_DIPSETTING(    0x00, DEF_STR( No ) )
	PORT_DIPSETTING(    0x01, DEF_STR( Yes ) )
	PORT_DIPUNKNOWN_DIPLOC( 0x02, 0x00, "DIL:!07" )
	PORT_DIPUNKNOWN_DIPLOC( 0x04, 0x00, "DIL:!08" )
	PORT_DIPUNKNOWN_DIPLOC( 0x08, 0x00, "DIL:!10" )
	PORT_DIPNAME( 0x10, 0x00, "Coin Jam Alarm" )PORT_DIPLOCATION("DIL:!11")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x10, DEF_STR( On  ) )

	PORT_MODIFY("STROBE11")
	PORT_DIPNAME( 0x01, 0x00, "Clear credits on reset?" )PORT_DIPLOCATION("DIL:!12")
	PORT_DIPSETTING(    0x00, DEF_STR( No ) )
	PORT_DIPSETTING(    0x01, DEF_STR( Yes ) )
	PORT_DIPNAME( 0x02, 0x00, "Attract Mode" )PORT_DIPLOCATION("DIL:!13")
	PORT_DIPSETTING(    0x02, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On  ) )
	PORT_DIPNAME( 0x1C, 0x00, "Target Percentage" )PORT_DIPLOCATION("DIL:!14,!15,!16")
	PORT_DIPSETTING(    0x14, "80%")
	PORT_DIPSETTING(    0x04, "82%")
	PORT_DIPSETTING(    0x1C, "84%")
	PORT_DIPSETTING(    0x0C, "86%")
	PORT_DIPSETTING(    0x10, "90%")
	PORT_DIPSETTING(    0x00, "92%")
	PORT_DIPSETTING(    0x18, "94%")
	PORT_DIPSETTING(    0x08, "96%")
INPUT_PORTS_END

// input ports for paradice ///////////////////////////////////////////////

static INPUT_PORTS_START( paradice )
	PORT_INCLUDE( pyramid )

	PORT_MODIFY("COINS")
    PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_COIN1 ) PORT_IMPULSE(3) PORT_NAME("Fl 0.25")
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_COIN2 ) PORT_IMPULSE(3) PORT_NAME("Fl 1.00")
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_COIN3 ) PORT_IMPULSE(3) PORT_NAME("Fl 2.50")
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_COIN4 ) PORT_IMPULSE(3) PORT_NAME("Fl 5.00")

	PORT_MODIFY("STROBE1")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_START1 ) PORT_NAME( "1 Player Start (Left)" )
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_START2 ) PORT_NAME( "2 Player Start (Right)" )
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_BUTTON1) PORT_NAME( "A" )
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_BUTTON2) PORT_NAME( "B" )

	PORT_MODIFY("STROBE2")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_BUTTON3) PORT_NAME( "C" )
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_OTHER  ) PORT_NAME( "Enter" ) PORT_CODE( KEYCODE_E )
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_MODIFY("STROBE10")
	PORT_DIPNAME( 0x01, 0x00, "Joker" )PORT_DIPLOCATION("DIL:!06")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x01, DEF_STR( On  ) )
	PORT_DIPNAME( 0x02, 0x00, DEF_STR( Language ) )PORT_DIPLOCATION("DIL:!07")
	PORT_DIPSETTING(    0x00, DEF_STR( English ) )
	PORT_DIPSETTING(    0x02, "Dutch"    )
	PORT_DIPNAME( 0x0C, 0x00, "Payout level" )PORT_DIPLOCATION("DIL:!08,!10")
	PORT_DIPSETTING(    0x00, DEF_STR( Low ) )
	PORT_DIPSETTING(    0x08, "Medium-Low"  )
	PORT_DIPSETTING(    0x04, "Medium-High" )
	PORT_DIPSETTING(    0x0C, DEF_STR( High ) )
	PORT_DIPNAME( 0x10, 0x00, DEF_STR( Difficulty ) )PORT_DIPLOCATION("DIL:!11")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x10, DEF_STR( On  ) )

	PORT_MODIFY("STROBE11")
	PORT_DIPNAME( 0x03, 0x00, "Winlines to go" )PORT_DIPLOCATION("DIL:!12,!13")
	PORT_DIPSETTING(    0x00, "6" )
	PORT_DIPSETTING(    0x02, "7" )
	PORT_DIPSETTING(    0x01, "8" )
	PORT_DIPSETTING(    0x03, "9" )
	PORT_DIPNAME( 0x04, 0x00, "Attract mode" )PORT_DIPLOCATION("DIL:!14")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x04, DEF_STR( On  ) )
	PORT_DIPNAME( 0x18, 0x00, "Timebar speed" )PORT_DIPLOCATION("DIL:!15,!16")
	PORT_DIPSETTING(    0x00, "0" )
	PORT_DIPSETTING(    0x08, "1" )
	PORT_DIPSETTING(    0x10, "2" )
	PORT_DIPSETTING(    0x18, "3" )
INPUT_PORTS_END

// input ports for pokio //////////////////////////////////////////////////

static INPUT_PORTS_START( pokio )
	PORT_INCLUDE( pyramid )

	PORT_MODIFY("COINS")
    PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_COIN1 ) PORT_IMPULSE(3) PORT_NAME("Fl 0.25")
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_COIN2 ) PORT_IMPULSE(3) PORT_NAME("Fl 1.00")
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_COIN3 ) PORT_IMPULSE(3) PORT_NAME("Fl 2.50")
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_COIN4 ) PORT_IMPULSE(3) PORT_NAME("Fl 5.00")

	PORT_MODIFY("STROBE1")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_NAME( "Hand 1 Left" )
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_BUTTON2 ) PORT_NAME( "Hand 2 Left" )
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_BUTTON3 ) PORT_NAME( "Hand 3 Left" )
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_UNUSED)

	PORT_MODIFY("STROBE2")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_START1 ) PORT_NAME( "1 Player Start (Left)" )
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_OTHER )  PORT_NAME( "Enter" ) PORT_CODE( KEYCODE_SPACE )
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_START2 ) PORT_NAME( "2 Player Start (Right)" )
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_BUTTON6 )PORT_NAME( "Hand 3 Right" )
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_BUTTON5 )PORT_NAME( "Hand 2 Right" )

	PORT_MODIFY("STROBE3")
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_BUTTON4 )PORT_NAME( "Hand 1 Right" )

	PORT_MODIFY("STROBE10")
	PORT_DIPUNKNOWN_DIPLOC( 0x01, 0x00, "DIL:!06" )
	PORT_DIPUNKNOWN_DIPLOC( 0x02, 0x00, "DIL:!07" )
	PORT_DIPUNKNOWN_DIPLOC( 0x04, 0x00, "DIL:!08" )
	PORT_DIPUNKNOWN_DIPLOC( 0x08, 0x00, "DIL:!10" )
	PORT_DIPNAME( 0x10, 0x00, "Coin Jam Alarm" )PORT_DIPLOCATION("DIL:!11")
	PORT_DIPSETTING(    0x10, DEF_STR( Off  ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On   ) )

	PORT_MODIFY("STROBE11")
	PORT_DIPNAME( 0x01, 0x00, "Time bar" ) PORT_DIPLOCATION("DIL:!12")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x01, DEF_STR( On  ) )
	PORT_DIPUNKNOWN_DIPLOC( 0x02, 0x00, "DIL:!13" )
	PORT_DIPNAME( 0x04, 0x00, "Attract mode" )PORT_DIPLOCATION("DIL:!14")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x04, DEF_STR( On  ) )
	PORT_DIPNAME( 0x18, 0x00, "Timebar speed" )PORT_DIPLOCATION("DIL:!15,!16")
	PORT_DIPSETTING(    0x00, "0" )
	PORT_DIPSETTING(    0x08, "1" )
	PORT_DIPSETTING(    0x10, "2" )
	PORT_DIPSETTING(    0x18, "3" )
INPUT_PORTS_END


///////////////////////////////////////////////////////////////////////////
// machine driver for scorpion2 board + adder2 expansion //////////////////
///////////////////////////////////////////////////////////////////////////

static MACHINE_CONFIG_START( scorpion2_vid, bfm_sc2_state )
	MCFG_MACHINE_RESET( init )							// main scorpion2 board initialisation
	MCFG_QUANTUM_TIME(attotime::from_hz(960))									// needed for serial communication !!
	MCFG_CPU_ADD("maincpu", M6809, MASTER_CLOCK/4 )	// 6809 CPU at 2 Mhz
	MCFG_CPU_PROGRAM_MAP(memmap_vid)					// setup scorpion2 board memorymap
	MCFG_CPU_PERIODIC_INT(timer_irq, 1000)				// generate 1000 IRQ's per second
	MCFG_WATCHDOG_TIME_INIT(PERIOD_OF_555_MONOSTABLE(120000,100e-9))

	MCFG_NVRAM_ADD_0FILL("nvram")
	MCFG_NVRAM_HANDLER(bfm_sc2)
	MCFG_DEFAULT_LAYOUT(layout_bfm_sc2)

	MCFG_SCREEN_ADD("adder", RASTER)
	MCFG_SCREEN_FORMAT(BITMAP_FORMAT_INDEXED16)
	MCFG_SCREEN_SIZE( 400, 280)
	MCFG_SCREEN_VISIBLE_AREA(  0, 400-1, 0, 280-1)
	MCFG_SCREEN_REFRESH_RATE(50)
	MCFG_SCREEN_UPDATE(addersc2)

	MCFG_VIDEO_START( adder2)
	MCFG_VIDEO_RESET( adder2)

	MCFG_PALETTE_LENGTH(16)
	MCFG_PALETTE_INIT(adder2)
	MCFG_GFXDECODE(adder2)

	MCFG_CPU_ADD("adder2", M6809, MASTER_CLOCK/4 )	// adder2 board 6809 CPU at 2 Mhz
	MCFG_CPU_PROGRAM_MAP(adder2_memmap)				// setup adder2 board memorymap
	MCFG_CPU_VBLANK_INT("adder", adder2_vbl)		// board has a VBL IRQ

	MCFG_SPEAKER_STANDARD_MONO("mono")
	MCFG_SOUND_ADD("upd", UPD7759, UPD7759_STANDARD_CLOCK)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.50)

	MCFG_SOUND_ADD("ymsnd", YM2413, XTAL_3_579545MHz)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 1.0)
MACHINE_CONFIG_END

static void sc2_common_init(running_machine &machine, int decrypt)
{
	bfm_sc2_state *state = machine.driver_data<bfm_sc2_state>();

	if (decrypt) bfm_decode_mainrom(machine, "maincpu", state->m_codec_data);		  // decode main rom

	memset(state->m_sc2_Inputs, 0, sizeof(state->m_sc2_Inputs));  // clear all inputs
}

static void adder2_common_init(running_machine &machine)
{
	bfm_sc2_state *state = machine.driver_data<bfm_sc2_state>();
	UINT8 *pal;

	pal = machine.region("proms")->base();
	if ( pal )
	{
		memcpy(state->m_key, pal, 8);
	}
}

// UK quintoon initialisation ////////////////////////////////////////////////

static DRIVER_INIT (quintoon)
{
	bfm_sc2_state *state = machine.driver_data<bfm_sc2_state>();
	sc2_common_init(machine, 1);
	adder2_decode_char_roms(machine);
	MechMtr_config(machine,8);					// setup mech meters

	state->m_has_hopper = 0;

	Scorpion2_SetSwitchState(machine,3,0,1);	// tube1 level switch
	Scorpion2_SetSwitchState(machine,3,1,1);	// tube2 level switch
	Scorpion2_SetSwitchState(machine,3,2,1);	// tube3 level switch

	Scorpion2_SetSwitchState(machine,5,2,1);
	Scorpion2_SetSwitchState(machine,6,4,1);

	state->m_sc2_show_door   = 1;
	state->m_sc2_door_state  = 0x41;
}

// dutch pyramid intialisation //////////////////////////////////////////////

static DRIVER_INIT( pyramid )
{
	bfm_sc2_state *state = machine.driver_data<bfm_sc2_state>();
	sc2_common_init(machine, 1);
	adder2_decode_char_roms(machine);			// decode GFX roms
	adder2_common_init(machine);

	state->m_has_hopper = 1;

	Scorpion2_SetSwitchState(machine,3,0,1);	// tube1 level switch
	Scorpion2_SetSwitchState(machine,3,1,1);	// tube2 level switch
	Scorpion2_SetSwitchState(machine,3,2,1);	// tube3 level switch

	state->m_sc2_show_door   = 1;
	state->m_sc2_door_state  = 0x41;
}
// belgian slots initialisation /////////////////////////////////////////////

static DRIVER_INIT( sltsbelg )
{
	bfm_sc2_state *state = machine.driver_data<bfm_sc2_state>();
	sc2_common_init(machine, 1);
	adder2_decode_char_roms(machine);			// decode GFX roms
	adder2_common_init(machine);

	state->m_has_hopper = 1;

	state->m_sc2_show_door   = 1;
	state->m_sc2_door_state  = 0x41;
}

// other dutch adder games ////////////////////////////////////////////////

static DRIVER_INIT( adder_dutch )
{
	bfm_sc2_state *state = machine.driver_data<bfm_sc2_state>();
	sc2_common_init(machine, 1);
	adder2_decode_char_roms(machine);			// decode GFX roms
	adder2_common_init(machine);

	state->m_has_hopper = 0;

	Scorpion2_SetSwitchState(machine,3,0,1);	// tube1 level switch
	Scorpion2_SetSwitchState(machine,3,1,1);	// tube2 level switch
	Scorpion2_SetSwitchState(machine,3,2,1);	// tube3 level switch

	state->m_sc2_show_door   = 1;
	state->m_sc2_door_state  = 0x41;
}

// golden crown //////////////////////////////////////////////////////////

static DRIVER_INIT( gldncrwn )
{
	bfm_sc2_state *state = machine.driver_data<bfm_sc2_state>();
	sc2_common_init(machine, 1);
	adder2_decode_char_roms(machine);			// decode GFX roms
	adder2_common_init(machine);

	state->m_has_hopper = 0;

	Scorpion2_SetSwitchState(machine,3,0,1);	// tube1 level switch
	Scorpion2_SetSwitchState(machine,3,1,1);	// tube2 level switch
	Scorpion2_SetSwitchState(machine,3,2,1);	// tube3 level switch

	state->m_sc2_show_door   = 0;
	state->m_sc2_door_state  = 0x41;
}

// ROM definition UK Quintoon ////////////////////////////////////////////

ROM_START( quintoon )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD("95750206.p1",	0x00000, 0x10000,  CRC(05f4bfad) SHA1(22751573f3a51a9fd2d2a75a7d1b20d78112e0bb))

	ROM_REGION( 0x20000, "adder2", 0 )
	ROM_LOAD("quinp132",		0x00000, 0x20000,  CRC(63896a7f) SHA1(81aa56874a15faa3aabdfc0fc524b2e25b751f22))

	ROM_REGION( 0x20000, "upd", 0 ) // using Dutch samples, need to check a UK Quintoon PCB
	ROM_LOAD("95001016.snd",	0x00000, 0x20000, BAD_DUMP CRC(cf097d41) SHA1(6712f93896483360256d8baffc05977c8e532ef1))

	ROM_REGION( 0x40000, "gfx1", ROMREGION_ERASEFF )
	ROM_LOAD("quinp233",		0x00000, 0x20000, CRC(3d4ebecf) SHA1(b339cf16797ccf7a1ec20fcebf52b6edad9a1047))
ROM_END

// ROM definition UK Quintoon (older) ////////////////////////////////////

ROM_START( quintono )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD("95750203.bin",	0x00000, 0x10000,  CRC(037ef2d0) SHA1(6958624e29629a7639a80e8929b833a8b0201833))

	ROM_REGION( 0x20000, "adder2", 0 )
	ROM_LOAD("quinp132",		0x00000, 0x20000,  CRC(63896a7f) SHA1(81aa56874a15faa3aabdfc0fc524b2e25b751f22))

	ROM_REGION( 0x20000, "upd", 0 ) // using Dutch samples, need to check a UK Quintoon PCB
	ROM_LOAD("95001016.snd",	0x00000, 0x20000, BAD_DUMP CRC(cf097d41) SHA1(6712f93896483360256d8baffc05977c8e532ef1))

	ROM_REGION( 0x40000, "gfx1", ROMREGION_ERASEFF )
	ROM_LOAD("quinp233",		0x00000, 0x20000, CRC(3d4ebecf) SHA1(b339cf16797ccf7a1ec20fcebf52b6edad9a1047))
ROM_END

// ROM definition UK Quintoon (data) /////////////////////////////////////

ROM_START( quintond )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD("95751206.bin",	0x00000, 0x10000,  CRC(63def707) SHA1(d016df74f4f83cd72b16f9ccbe78cc382bf056c8))

	ROM_REGION( 0x20000, "adder2", 0 )
	ROM_LOAD("quinp132",		0x00000, 0x20000,  CRC(63896a7f) SHA1(81aa56874a15faa3aabdfc0fc524b2e25b751f22))

	ROM_REGION( 0x20000, "upd", 0 ) // using Dutch samples, need to check a UK Quintoon PCB
	ROM_LOAD("95001016.snd",	0x00000, 0x20000, BAD_DUMP CRC(cf097d41) SHA1(6712f93896483360256d8baffc05977c8e532ef1))

	ROM_REGION( 0x40000, "gfx1", ROMREGION_ERASEFF )
	ROM_LOAD("quinp233",		0x00000, 0x20000, CRC(3d4ebecf) SHA1(b339cf16797ccf7a1ec20fcebf52b6edad9a1047))
ROM_END

// ROM definition Dutch Quintoon ///////////////////////////////////////////

ROM_START( qntoond )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD("95750243.bin", 0x00000, 0x10000, CRC(36a8dcd1) SHA1(ab21301312fbb6609f850e1cf6bcda5a2b7f66f5))

	ROM_REGION( 0x20000, "adder2", 0 )
	ROM_LOAD("95770024.vid", 0x00000, 0x20000, CRC(5bc7ac55) SHA1(b54e9684f750b73c357d41b88ca8c527258e2a10))

	ROM_REGION( 0x20000, "upd", 0 )
	ROM_LOAD("95001016.snd", 0x00000, 0x20000, CRC(cf097d41) SHA1(6712f93896483360256d8baffc05977c8e532ef1))

	ROM_REGION( 0x40000, "gfx1", ROMREGION_ERASEFF )
	ROM_LOAD("95770025.chr", 0x00000, 0x20000, CRC(f59748ea) SHA1(f0f7f914fdf72db8eb60717b95e7d027c0081339))
ROM_END

// ROM definition Dutch Quintoon alternate set /////////////////////////////

ROM_START( qntoondo )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD("95750136.bin", 0x00000, 0x10000, CRC(839ea01d) SHA1(d7f77dbaea4e87c3d782408eb50d10f44b6df5e2))

	ROM_REGION( 0x20000, "adder2", 0 )
	ROM_LOAD("95770024.vid", 0x00000, 0x20000, CRC(5bc7ac55) SHA1(b54e9684f750b73c357d41b88ca8c527258e2a10))

	ROM_REGION( 0x20000, "upd", 0 )
	ROM_LOAD("95001016.snd", 0x00000, 0x20000, CRC(cf097d41) SHA1(6712f93896483360256d8baffc05977c8e532ef1))

	ROM_REGION( 0x40000, "gfx1", ROMREGION_ERASEFF )
	ROM_LOAD("95770025.chr", 0x00000, 0x20000, CRC(f59748ea) SHA1(f0f7f914fdf72db8eb60717b95e7d027c0081339))
ROM_END

// ROM definition dutch golden crown //////////////////////////////////////

ROM_START( gldncrwn )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD("95752011.bin", 0x00000, 0x10000, CRC(54f7cca0) SHA1(835727d88113700a38060f880b4dfba2ded41487))

	ROM_REGION( 0x20000, "adder2", 0 )
	ROM_LOAD("95770117.vid", 0x00000, 0x20000, CRC(598ba7cb) SHA1(ab518d7df24b0b453ec3fcddfc4db63e0391fde7))

	ROM_REGION( 0x20000, "upd", 0 )
	ROM_LOAD("95001039.snd", 0x00000, 0x20000, CRC(6af26157) SHA1(9b3a85f5dd760c4430e38e2844928b74aadc7e75))

	ROM_REGION( 0x40000, "gfx1", ROMREGION_ERASEFF )
	ROM_LOAD("95770118.ch1", 0x00000, 0x20000, CRC(9c9ac946) SHA1(9a571e7d00f6654242aface032c2fb186ef44aba))
	ROM_LOAD("95770119.ch2", 0x20000, 0x20000, CRC(9e0fdb2e) SHA1(05e8257285b0009df4fcc73e93490876358a8be8))

	ROM_REGION( 0x10, "proms", 0 )
	ROM_LOAD("gcrpal.bin", 0, 8 , CRC(4edd5a1d) SHA1(d6fe38377d5f2291d33ee8ed808548871e63c4d7))
ROM_END

// ROM definition Dutch Paradice //////////////////////////////////////////

ROM_START( paradice )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD("95750615.bin", 0x00000, 0x10000, CRC(f51192e5) SHA1(a1290e32bba698006e83fd8d6075202586232929))

	ROM_REGION( 0x20000, "adder2", 0 )
	ROM_LOAD("95770084.vid", 0x00000, 0x20000, CRC(8f27bd34) SHA1(fccf7283b5c952b74258ee6e5138c1ca89384e24))

	ROM_REGION( 0x20000, "upd", 0 )
	ROM_LOAD("95001037.snd", 0x00000, 0x20000, CRC(82f74276) SHA1(c51c3caeb7bf514ec7a1b452c8effc4c79186062))

	ROM_REGION( 0x40000, "gfx1", ROMREGION_ERASEFF )
	ROM_LOAD("95770085.ch1", 0x00000, 0x20000, CRC(4d1fb82f) SHA1(054f683d1d7c884911bd2d0f85aab4c59ddf9930))
	ROM_LOAD("95770086.ch2", 0x20000, 0x20000, CRC(7b566e11) SHA1(f34c82ad75a0f88204ac4ae83a00801215c46ca9))

	ROM_REGION( 0x10, "proms", 0 )
	ROM_LOAD( "pdcepal.bin", 0, 8 , CRC(64020c97) SHA1(9371841e2df950c1f2e5b5a4b52621beb6f60945))
ROM_END

// ROM definition Dutch Pokio /////////////////////////////////////////////

ROM_START( pokio )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD("95750278.bin", 0x00000, 0x10000, CRC(5124b24d) SHA1(9bc63891a8e9283c2baa64c264a5d6d1625d44b2))

	ROM_REGION( 0x20000, "adder2", 0 )
	ROM_LOAD("95770044.vid", 0x00000, 0x20000, CRC(46d7a6d8) SHA1(01f58e735621661b57c61491b3769ae99e92476a))

	ROM_REGION( 0x20000, "upd", 0 )
	ROM_LOAD("95001016.snd", 0x00000, 0x20000, CRC(98aaff76) SHA1(4a59cf83daf018d93f1ff7805e06309d2f3d7252))

	ROM_REGION( 0x40000, "gfx1", ROMREGION_ERASEFF )
	ROM_LOAD("95770045.chr", 0x00000, 0x20000, CRC(dd30da90) SHA1(b4f5a229d88613c0c7d43adf3f325c619abe38a3))

	ROM_REGION( 0x10, "proms", 0 )
	ROM_LOAD("pokiopal.bin", 0, 8 , CRC(53535184) SHA1(c5c98085e39ca3671dca72c21a8466d7d70cd341))
ROM_END

// ROM definition pyramid prototype  //////////////////////////////////////

ROM_START( pyramid )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD("95750898.bin", 0x00000, 0x10000,  CRC(3b0df16c) SHA1(9af599fe604f86c72986aa1610d74837852e023f))

	ROM_REGION( 0x20000, "adder2", 0 )
	ROM_LOAD("95770108.vid", 0x00000, 0x20000,  CRC(216ff683) SHA1(227764771600ce88c5f36bed9878e6bb9988ae8f))

	ROM_REGION( 0x20000, "upd", 0 )
	ROM_LOAD("95001038.snd", 0x00000, 0x20000, CRC(f885c42e) SHA1(4d79fc5ae4c58247740d78d81302bfbb43331c43))

	ROM_REGION( 0x40000, "gfx1", ROMREGION_ERASEFF )
	ROM_LOAD("95770106.ch1", 0x00000, 0x20000, CRC(a83c27ae) SHA1(f61ca3cdf19a933bae18c1b32a5fb0a2204dde78))
	ROM_LOAD("95770107.ch2", 0x20000, 0x20000, CRC(52e59f64) SHA1(ea4828c2cfb72cd77c92c60560b4d5ee424f7dca))

	ROM_REGION( 0x10, "proms", 0 )
	ROM_LOAD("pyrmdpal.bin", 0, 8 , CRC(1c7c37bb) SHA1(fe0276603fee8f58e4318f91645260368212b78b))
ROM_END

// ROM definition Dutch slots /////////////////////////////////////////////

ROM_START( slotsnl )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD("95750368.bin", 0x00000, 0x10000, CRC(3a43048c) SHA1(13728e05b334cba90ea9cc51ea00c4384baa8614))

	ROM_REGION( 0x20000, "adder2", 0 )
	ROM_LOAD("video.vid",	 0x00000, 0x20000, CRC(cc760208) SHA1(cc01b1e31335b26f2d0f3470d8624476b153655f))

	ROM_REGION( 0x20000, "upd", 0 )
	ROM_LOAD("95001029.snd", 0x00000, 0x20000, CRC(7749c724) SHA1(a87cce0c99e392f501bba44b3936a7059d682c9c))

	ROM_REGION( 0x40000, "gfx1", ROMREGION_ERASEFF )
	ROM_LOAD("charset.chr",	 0x00000, 0x20000,  CRC(ef4300b6) SHA1(a1f765f38c2f146651fc685ea6195af72465f559))

	ROM_REGION( 0x10, "proms", 0 )
	ROM_LOAD( "slotspal.bin", 0, 8 , CRC(ee5421f0) SHA1(21bdcbf11dda8b1a93c49ae1c706954bba53c917))
ROM_END

// ROM definition Belgian Slots (Token pay per round) Payslide ////////////

ROM_START( sltblgtk )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD("95750943.bin", 0x00000, 0x10000, CRC(c9fb8153) SHA1(7c1d0660c15f05b1e0784d8322c62981fe8dc4c9))

	ROM_REGION( 0x20000, "adder2", 0 )
	ROM_LOAD("adder121.bin", 0x00000, 0x20000, CRC(cedbbf28) SHA1(559ae341b55462feea771127394a54fc65266818))

	ROM_REGION( 0x20000, "upd", 0 )
	ROM_LOAD("sound029.bin", 0x00000, 0x20000, CRC(7749c724) SHA1(a87cce0c99e392f501bba44b3936a7059d682c9c))

	ROM_REGION( 0x40000, "gfx1", ROMREGION_ERASEFF )
	ROM_LOAD("chr122.bin",	 0x00000, 0x20000, CRC(a1e3bdf4) SHA1(f0cabe08dee028e2014cbf0fc3fe0806cdfa60c6))

	ROM_REGION( 0x10, "proms", 0 )
	ROM_LOAD("stsbtpal.bin", 0, 8 , CRC(20e13635) SHA1(5aa7e7cac8c00ebc193d63d0c6795904f42c70fa))
ROM_END

// ROM definition Belgian Slots (Cash Payout) /////////////////////////////

ROM_START( sltblgp1 )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD("95752008.bin", 0x00000, 0x10000, CRC(3167d3b9) SHA1(a28563f65d55c4d47f3e7fdb41e050d8a733b9bd))

	ROM_REGION( 0x20000, "adder2", 0 )
	ROM_LOAD("adder142.bin", 0x00000, 0x20000, CRC(a6f6356b) SHA1(b3d3063155ee3ea888273081f844279b6e33f7d9))

	ROM_REGION( 0x20000, "upd", 0 )
	ROM_LOAD("sound033.bin", 0x00000, 0x20000, CRC(bb1dfa55) SHA1(442454fccfe03e6f4c3353551cb7459e184a099d))

	ROM_REGION( 0x40000, "gfx1", ROMREGION_ERASEFF )
	ROM_LOAD("chr143.bin",	 0x00000, 0x20000, CRC(a40e91e2) SHA1(87dc76963ea961fcfbe4f3e25df9162348d39d79))

	ROM_REGION( 0x10, "proms", 0 )
	ROM_LOAD("stsbcpal.bin", 0, 8 , CRC(c63bcab6) SHA1(238841165d5b3241b0bcc5c1792e9c0be1fc0177))
ROM_END

// ROM definition Belgian Slots (Cash Payout) /////////////////////////////

ROM_START( sltblgpo )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD("95770938.bin", 0x00000, 0x10000, CRC(7e802634) SHA1(fecf86e632546649d5e647c42a248b39fc2cf982))

	ROM_REGION( 0x20000, "adder2", 0 )
	ROM_LOAD("95770120.chr", 0x00000, 0x20000, CRC(ad505138) SHA1(67ccd8dc30e76283247ab5a62b22337ebaff74cd))

	ROM_REGION( 0x20000, "upd", 0 )
	ROM_LOAD("sound033.bin", 0x00000, 0x20000, CRC(bb1dfa55) SHA1(442454fccfe03e6f4c3353551cb7459e184a099d))

	ROM_REGION( 0x40000, "gfx1", ROMREGION_ERASEFF )
	ROM_LOAD("95770110.add", 0x00000, 0x20000, CRC(64b03284) SHA1(4b1c17b75e449c9762bb949d7cde0694a3aaabeb))

	ROM_REGION( 0x10, "proms", 0 )
	ROM_LOAD("stsbcpal.bin", 0, 8 , CRC(c63bcab6) SHA1(238841165d5b3241b0bcc5c1792e9c0be1fc0177))
ROM_END


/**************************************************************************

    Mechanical Scorpion 2 Games
        AGEMAME driver

***************************************************************************

  30-12-2006: J Wallace: Fixed init routines.
  07-03-2006: El Condor: Recoded to more accurately represent the hardware
              setup.
  18-01-2006: Cleaned up for MAME inclusion
  19-08-2005: Re-Animator

***************************************************************************/



///////////////////////////////////////////////////////////////////////////






#ifdef UNUSED_FUNCTION
/* Scorpion 3 expansion */
static READ8_HANDLER( sc3_expansion_r )
{
    int result = 0;

    switch ( offset )
    {
        case 0: result = 0;
        break;
        case 1: result = input_port_read_indexed(machine,0);  /* coin input */
    }

    return result;
}


static WRITE8_HANDLER( sc3_expansion_w )
{
    switch ( offset )
    {
	    case 0:
    	break;
	    case 1:
    	break;
    }
}
#endif


/* machine init (called only once) */
static MACHINE_RESET( awp_init )
{
	on_scorpion2_reset(machine);
	BFM_BD1_init(0);
	BFM_BD1_init(1);
}


static MACHINE_RESET( dm01_init )
{
	on_scorpion2_reset(machine);
	BFM_dm01_reset(machine);
}




#ifdef UNREFERENCED_CODE
static INPUT_PORTS_START( scorpion2 )
	PORT_START("COINS")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_COIN1 ) PORT_IMPULSE(3)
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_COIN2 ) PORT_IMPULSE(3)
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_COIN3 ) PORT_IMPULSE(3)
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_COIN4 ) PORT_IMPULSE(3)
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_COIN5 ) PORT_IMPULSE(3)
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_COIN6 ) PORT_IMPULSE(3)
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_COIN7 ) PORT_IMPULSE(3)
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_COIN8 ) PORT_IMPULSE(3)

	PORT_START("STROBE0")
	PORT_BIT( 0xFF, IP_ACTIVE_HIGH, IPT_UNUSED)

	PORT_START("STROBE1")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_BUTTON1) PORT_NAME("I10")
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_BUTTON2) PORT_NAME("I11")
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_BUTTON3) PORT_NAME("I12")
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_BUTTON4) PORT_NAME("I13")
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_BUTTON5) PORT_NAME("I14")
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("STROBE2")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_BUTTON6 ) PORT_NAME("I20")
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_BUTTON7 ) PORT_NAME("I21")
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_BUTTON8 ) PORT_NAME("I22")
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_BUTTON9 ) PORT_NAME("I23")
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_BUTTON10) PORT_NAME("I24")
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_UNUSED  )
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_UNUSED  )
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_UNUSED  )

	PORT_START("STROBE3")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_OTHER )	PORT_NAME("I30")
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_OTHER )	PORT_NAME("I31")
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_OTHER )	PORT_NAME("I32")
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_OTHER )	PORT_NAME("I33")
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_OTHER )	PORT_NAME("I34")
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_UNUSED)

	PORT_START("STROBE4")
	PORT_BIT( 0x01, IP_ACTIVE_LOW,  IPT_INTERLOCK)	PORT_NAME("Cashbox Door") PORT_CODE(KEYCODE_Q) PORT_TOGGLE
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_INTERLOCK)	PORT_NAME("Front Door") PORT_CODE(KEYCODE_W) PORT_TOGGLE
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_OTHER )		PORT_NAME("Refill Key") PORT_CODE(KEYCODE_R) PORT_TOGGLE
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_OTHER )		PORT_NAME("I43")
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_OTHER )		PORT_NAME("I44")
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_UNUSED)

	PORT_START("STROBE5")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_NAME("I50")
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_NAME("I51")
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_NAME("I52")
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_NAME("I53")
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_NAME("I54")
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_UNUSED)

	PORT_START("STROBE6")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_NAME("I60")
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_NAME("I61")
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_NAME("I62")
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_NAME("I63")
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_NAME("I64")
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_UNUSED)

	PORT_START("STROBE7")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_NAME("I70")
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_NAME("I71")
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_NAME("I72")
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_NAME("I73")
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_NAME("I74")
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_UNUSED)

	PORT_START("STROBE8")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_NAME("I80")
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_NAME("I81")
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_NAME("I82")
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_NAME("I83")
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_NAME("I84")
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_UNUSED)

	PORT_START("STROBE9")
	PORT_SERVICE_NO_TOGGLE(0x01,IP_ACTIVE_HIGH)
	PORT_DIPNAME( 0x02, 0x00, "DIL02" ) PORT_DIPLOCATION("DIL:02")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x02, DEF_STR( On  ) )
	PORT_DIPNAME( 0x04, 0x00, "DIL03" ) PORT_DIPLOCATION("DIL:03")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x04, DEF_STR( On  ) )
	PORT_DIPNAME( 0x08, 0x00, "DIL04" ) PORT_DIPLOCATION("DIL:04")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x08, DEF_STR( On  ) )
	PORT_DIPNAME( 0x10, 0x00, "DIL05" ) PORT_DIPLOCATION("DIL:05")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x10, DEF_STR( On  ) )

	PORT_START("STROBE10")
	PORT_DIPNAME( 0x01, 0x00, "DIL06" ) PORT_DIPLOCATION("DIL:06")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x01, DEF_STR( On  ) )
	PORT_DIPNAME( 0x02, 0x00, "DIL07" ) PORT_DIPLOCATION("DIL:07")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x02, DEF_STR( On  ) )
	PORT_DIPNAME( 0x04, 0x00, "DIL08" ) PORT_DIPLOCATION("DIL:08")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x04, DEF_STR( On  ) )
	PORT_DIPNAME( 0x08, 0x00, "DIL10" ) PORT_DIPLOCATION("DIL:10")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x08, DEF_STR( On  ) )
	PORT_DIPNAME( 0x10, 0x00, "DIL11" ) PORT_DIPLOCATION("DIL:11")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x10, DEF_STR( On  ) )

	PORT_START("STROBE11")
	PORT_DIPNAME( 0x01, 0x00, "DIL12" ) PORT_DIPLOCATION("DIL:12")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x01, DEF_STR( On  ) )
	PORT_DIPNAME( 0x02, 0x00, "DIL13" ) PORT_DIPLOCATION("DIL:13")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x02, DEF_STR( On  ) )
	PORT_DIPNAME( 0x04, 0x00, "DIL14" ) PORT_DIPLOCATION("DIL:14")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x04, DEF_STR( On  ) )
	PORT_DIPNAME( 0x08, 0x00, "DIL15" ) PORT_DIPLOCATION("DIL:15")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x08, DEF_STR( On  ) )
	PORT_DIPNAME( 0x10, 0x00, "DIL16" ) PORT_DIPLOCATION("DIL:16")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x10, DEF_STR( On  ) )
INPUT_PORTS_END
#endif

static INPUT_PORTS_START( bbrkfst )
	PORT_START("COINS")
    PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_COIN1 ) PORT_IMPULSE(3) PORT_NAME("10p")
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_COIN2 ) PORT_IMPULSE(3) PORT_NAME("20p")
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_COIN3 ) PORT_IMPULSE(3) PORT_NAME("50p")
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_COIN4 ) PORT_IMPULSE(3) PORT_NAME("GBP 1.00")
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_COIN5 ) PORT_IMPULSE(3) PORT_NAME("Token")
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_UNUSED)

	PORT_START("STROBE0")
	PORT_BIT( 0xFF, IP_ACTIVE_HIGH, IPT_UNUSED)

	PORT_START("STROBE1")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_BUTTON1) PORT_NAME("Cancel")
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_BUTTON2) PORT_NAME("Hold 1")
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_BUTTON3) PORT_NAME("Hold 2/Hi")
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_BUTTON4) PORT_NAME("Hold 3/Lo")
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_BUTTON5) PORT_NAME("Stop/Collect")
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("STROBE2")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_BUTTON6 ) PORT_NAME("Exchange")
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_START1  )
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_BUTTON7 ) PORT_NAME("Take Big Breakfast")
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_BUTTON8 ) PORT_NAME("Take Feature")
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_UNUSED  )
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_UNUSED  )
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_UNUSED  )

	PORT_START("STROBE3")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("STROBE4")
	PORT_BIT( 0x01, IP_ACTIVE_LOW,  IPT_INTERLOCK)	PORT_NAME("Cashbox Door") PORT_CODE(KEYCODE_Q) PORT_TOGGLE
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_INTERLOCK)	PORT_NAME("Front Door") PORT_CODE(KEYCODE_W) PORT_TOGGLE
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_OTHER )		PORT_NAME("Refill Key") PORT_CODE(KEYCODE_R) PORT_TOGGLE
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("STROBE5")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("STROBE6")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("STROBE7")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("STROBE8")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("STROBE9")
	PORT_SERVICE_NO_TOGGLE(0x01,IP_ACTIVE_HIGH)
	PORT_DIPNAME( 0x02, 0x00, "DIL02" ) PORT_DIPLOCATION("DIL:02")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x02, DEF_STR( On  ) )
	PORT_DIPNAME( 0x04, 0x00, "DIL03" ) PORT_DIPLOCATION("DIL:03")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x04, DEF_STR( On  ) )
	PORT_DIPNAME( 0x08, 0x00, "DIL04" ) PORT_DIPLOCATION("DIL:04")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x08, DEF_STR( On  ) )
	PORT_DIPNAME( 0x10, 0x00, "DIL05" ) PORT_DIPLOCATION("DIL:05")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x10, DEF_STR( On  ) )

	PORT_START("STROBE10")
	PORT_DIPNAME( 0x01, 0x00, "DIL06" ) PORT_DIPLOCATION("DIL:06")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x01, DEF_STR( On  ) )
	PORT_DIPNAME( 0x02, 0x00, "DIL07" ) PORT_DIPLOCATION("DIL:07")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x02, DEF_STR( On  ) )
	PORT_DIPNAME( 0x04, 0x00, "DIL08" ) PORT_DIPLOCATION("DIL:08")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x04, DEF_STR( On  ) )
	PORT_DIPNAME( 0x08, 0x00, "DIL10" ) PORT_DIPLOCATION("DIL:10")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x08, DEF_STR( On  ) )
	PORT_DIPNAME( 0x10, 0x00, "DIL11" ) PORT_DIPLOCATION("DIL:11")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x10, DEF_STR( On  ) )

	PORT_START("STROBE11")
	PORT_DIPNAME( 0x01, 0x00, "DIL12" ) PORT_DIPLOCATION("DIL:12")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x01, DEF_STR( On  ) )
	PORT_DIPNAME( 0x02, 0x00, "DIL13" ) PORT_DIPLOCATION("DIL:13")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x02, DEF_STR( On  ) )
	PORT_DIPNAME( 0x04, 0x00, "DIL14" ) PORT_DIPLOCATION("DIL:14")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x04, DEF_STR( On  ) )
	PORT_DIPNAME( 0x08, 0x00, "DIL15" ) PORT_DIPLOCATION("DIL:15")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x08, DEF_STR( On  ) )
	PORT_DIPNAME( 0x10, 0x00, "DIL16" ) PORT_DIPLOCATION("DIL:16")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x10, DEF_STR( On  ) )
INPUT_PORTS_END

static INPUT_PORTS_START( drwho )
	PORT_START("COINS")
    PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_COIN1 ) PORT_IMPULSE(3) PORT_NAME("10p")
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_COIN2 ) PORT_IMPULSE(3) PORT_NAME("20p")
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_COIN3 ) PORT_IMPULSE(3) PORT_NAME("50p")
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_COIN4 ) PORT_IMPULSE(3) PORT_NAME("GBP 1.00")
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_COIN5 ) PORT_IMPULSE(3) PORT_NAME("Token")
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_UNUSED)

	PORT_START("STROBE0")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_UNUSED)

	PORT_START("STROBE1")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_BUTTON1) PORT_NAME("Cancel")
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_BUTTON2) PORT_NAME("Hold 1")
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_BUTTON3) PORT_NAME("Hold 2/Hi")
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_BUTTON4) PORT_NAME("Hold 3/Lo")
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_BUTTON5) PORT_NAME("Stop/Collect")
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_UNUSED)

	PORT_START("STROBE2")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_BUTTON6) PORT_NAME("Exchange")
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_START1 )
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("STROBE3")
	PORT_BIT( 0x01, IP_ACTIVE_LOW,  IPT_INTERLOCK)PORT_NAME("Cashbox Door") PORT_CODE(KEYCODE_Q) PORT_TOGGLE
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_INTERLOCK)PORT_NAME("Front Door") PORT_CODE(KEYCODE_W) PORT_TOGGLE
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_OTHER  )  PORT_NAME("Refill Key") PORT_CODE(KEYCODE_R) PORT_TOGGLE
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("STROBE4")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_SPECIAL )/*Tube status Low switch for 1 Pound*/
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_SPECIAL )/*Tube status Low switch for 20p*/
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_SPECIAL )/*Tube status Low switch for Token Front*/
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_SPECIAL )/*Tube status Low switch for Token Rear*/
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("STROBE5")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_UNUSED)

	PORT_START("STROBE6")
	PORT_DIPNAME( 0x07, 0x07, "PERCENTAGE KEY" ) PORT_DIPLOCATION("STROBE6:01,02,03")/*Certain combinations give different percentages*/
	PORT_DIPSETTING(    0x00, "No key") /*Some day, I'll work all these values out.*/
	PORT_DIPSETTING(    0x01, "Key 1" )
	PORT_DIPSETTING(    0x02, "Key 2" )
	PORT_DIPSETTING(    0x03, "Key 3" )
	PORT_DIPSETTING(    0x04, "Key 4" )
	PORT_DIPSETTING(    0x05, "Key 5" )
	PORT_DIPSETTING(    0x06, "Key 6" )
	PORT_DIPSETTING(    0x07, "Key 7" )
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("STROBE7")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_SPECIAL )/*Tube status High switch for 1 Pound*/
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_SPECIAL )/*Tube status High switch for 20p*/
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_SPECIAL )/*Tube status High switch for Token Front*/
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_SPECIAL )/*Tube status High switch for Token Rear*/
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("STROBE8")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_UNUSED)

	PORT_START("STROBE9")
	PORT_SERVICE_NO_TOGGLE(0x01,IP_ACTIVE_HIGH)
	PORT_DIPNAME( 0x02, 0x00, "DIL02" ) PORT_DIPLOCATION("DIL:02")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x02, DEF_STR( On  ) )
	PORT_DIPNAME( 0x04, 0x00, "DIL03" ) PORT_DIPLOCATION("DIL:03")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x04, DEF_STR( On  ) )
	PORT_DIPNAME( 0x08, 0x00, "DIL04" ) PORT_DIPLOCATION("DIL:04")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x08, DEF_STR( On  ) )
	PORT_DIPNAME( 0x10, 0x00, "DIL05" ) PORT_DIPLOCATION("DIL:05")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x10, DEF_STR( On  ) )

	PORT_START("STROBE10")
	PORT_DIPNAME( 0x01, 0x00, "DIL06" ) PORT_DIPLOCATION("DIL:06")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x01, DEF_STR( On  ) )
	PORT_DIPNAME( 0x02, 0x00, "DIL07" ) PORT_DIPLOCATION("DIL:07")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x02, DEF_STR( On  ) )
	PORT_DIPNAME( 0x04, 0x00, "DIL08" ) PORT_DIPLOCATION("DIL:08")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x04, DEF_STR( On  ) )
	PORT_DIPNAME( 0x08, 0x00, "DIL10" ) PORT_DIPLOCATION("DIL:10")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x08, DEF_STR( On  ) )
	PORT_DIPNAME( 0x10, 0x00, "DIL11" ) PORT_DIPLOCATION("DIL:11")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x10, DEF_STR( On  ) )

	PORT_START("STROBE11")
	PORT_DIPNAME( 0x01, 0x00, "DIL12" ) PORT_DIPLOCATION("DIL:12")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x01, DEF_STR( On  ) )
	PORT_DIPNAME( 0x02, 0x00, "DIL13" ) PORT_DIPLOCATION("DIL:13")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x02, DEF_STR( On  ) )
	PORT_DIPNAME( 0x04, 0x00, "DIL14" ) PORT_DIPLOCATION("DIL:14")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x04, DEF_STR( On  ) )
	PORT_DIPNAME( 0x08, 0x00, "DIL15" ) PORT_DIPLOCATION("DIL:15")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x08, DEF_STR( On  ) )
	PORT_DIPNAME( 0x10, 0x00, "DIL16" ) PORT_DIPLOCATION("DIL:16")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x10, DEF_STR( On  ) )
INPUT_PORTS_END

static INPUT_PORTS_START( cpeno1 )
	PORT_START("COINS")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_COIN1 ) PORT_IMPULSE(3)
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_COIN2 ) PORT_IMPULSE(3)
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_COIN3 ) PORT_IMPULSE(3)
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_COIN4 ) PORT_IMPULSE(3)
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_COIN5 ) PORT_IMPULSE(3)
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_COIN6 ) PORT_IMPULSE(3)
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_COIN7 ) PORT_IMPULSE(3)
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_COIN8 ) PORT_IMPULSE(3)

	PORT_START("STROBE0")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_UNUSED)

	PORT_START("STROBE1")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_BUTTON1) PORT_NAME("Hold 1")
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_BUTTON2) PORT_NAME("Hold 1?")
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_BUTTON3) PORT_NAME("Hold 2/Hi")
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_BUTTON4) PORT_NAME("Hold 3/Lo")
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_BUTTON5) PORT_NAME("Cancel/Collect")
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("STROBE2")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_BUTTON6) PORT_NAME("Stop/Exchange")
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_START1 )
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("STROBE3")
	PORT_BIT( 0x01, IP_ACTIVE_LOW,  IPT_INTERLOCK) PORT_NAME("Cashbox Door") PORT_CODE(KEYCODE_Q) PORT_TOGGLE
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_INTERLOCK) PORT_NAME("Front Door") PORT_CODE(KEYCODE_W) PORT_TOGGLE
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_OTHER  )   PORT_NAME("Refill Key") PORT_CODE(KEYCODE_R) PORT_TOGGLE
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("STROBE4")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_UNUSED)

	PORT_START("STROBE5")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_UNUSED)

	PORT_START("STROBE6")
	PORT_DIPNAME( 0x07, 0x07, "PERCENTAGE KEY" ) PORT_DIPLOCATION("STROBE6:01,02,03")
	PORT_DIPSETTING(    0x00, "No key" )
	PORT_DIPSETTING(    0x01, "Key 1" )
	PORT_DIPSETTING(    0x02, "Key 2" )
	PORT_DIPSETTING(    0x03, "Key 3" )
	PORT_DIPSETTING(    0x04, "Key 4" )
	PORT_DIPSETTING(    0x05, "Key 5" )
	PORT_DIPSETTING(    0x06, "Key 6" )
	PORT_DIPSETTING(    0x07, "Key 7" )
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("STROBE7")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_UNUSED)

	PORT_START("STROBE8")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_UNUSED)

	PORT_START("STROBE9")
	PORT_SERVICE_NO_TOGGLE(0x01,IP_ACTIVE_HIGH)
	PORT_DIPNAME( 0x02, 0x00, "DIL02" ) PORT_DIPLOCATION("DIL:02")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x02, DEF_STR( On  ) )
	PORT_DIPNAME( 0x04, 0x00, "Attract Hi/Lo reel" ) PORT_DIPLOCATION("DIL:03")
	PORT_DIPSETTING(    0x04, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On  ) )
	PORT_DIPNAME( 0x08, 0x00, "Acceptor type" ) PORT_DIPLOCATION("DIL:04")
	PORT_DIPSETTING(    0x00, "Mars" )
	PORT_DIPSETTING(    0x08, "Sentinel" )
	PORT_DIPNAME( 0x10, 0x00, "DIL05" ) PORT_DIPLOCATION("DIL:05")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x10, DEF_STR( On  ) )

	PORT_START("STROBE10")
	PORT_DIPNAME( 0x01, 0x00, "Coin play" ) PORT_DIPLOCATION("DIL:06")
	PORT_DIPSETTING(    0x00, "Multi coin" )
	PORT_DIPSETTING(    0x01, "Single coin" )
	PORT_DIPNAME( 0x02, 0x00, "CashPot Freq" ) PORT_DIPLOCATION("DIL:07")
	PORT_DIPSETTING(    0x00, DEF_STR( High) )
	PORT_DIPSETTING(    0x02, DEF_STR( Low ) )
	PORT_DIPNAME( 0x04, 0x00, "DIL08" ) PORT_DIPLOCATION("DIL:08")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x04, DEF_STR( On  ) )
	PORT_DIPNAME( 0x08, 0x00, "Coin Jam Alarm" ) PORT_DIPLOCATION("DIL:10")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x08, DEF_STR( On  ) )
	PORT_DIPNAME( 0x10, 0x00, "DIL11" ) PORT_DIPLOCATION("DIL:11")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x10, DEF_STR( On  ) )

	PORT_START("STROBE11")
	PORT_DIPNAME( 0x01, 0x00, "DIL12" ) PORT_DIPLOCATION("DIL:12")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x01, DEF_STR( On  ) )
	PORT_DIPNAME( 0x02, 0x00, "DIL13" ) PORT_DIPLOCATION("DIL:13")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x02, DEF_STR( On  ) )
	PORT_DIPNAME( 0x1C, 0x00, "Percentage setting" ) PORT_DIPLOCATION("DIL:14,15,16")
	PORT_DIPSETTING(    0x0C, "72%" )
	PORT_DIPSETTING(    0x04, "75%" )
	PORT_DIPSETTING(    0x00, "78%" )
	PORT_DIPSETTING(    0x10, "81%" )
	PORT_DIPSETTING(    0x18, "85%" )
INPUT_PORTS_END

static INPUT_PORTS_START( luvjub )
	PORT_START("COINS")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_COIN1 ) PORT_IMPULSE(3)
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_COIN2 ) PORT_IMPULSE(3)
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_COIN3 ) PORT_IMPULSE(3)
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_COIN4 ) PORT_IMPULSE(3)
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_COIN5 ) PORT_IMPULSE(3)
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_COIN6 ) PORT_IMPULSE(3)
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_COIN7 ) PORT_IMPULSE(3)
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_COIN8 ) PORT_IMPULSE(3)

	PORT_START("STROBE0")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_UNUSED)

	PORT_START("STROBE1")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_BUTTON1) PORT_NAME("Cancel")
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_BUTTON2) PORT_NAME("Hold1")
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_BUTTON3) PORT_NAME("Hold2")
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_BUTTON4) PORT_NAME("Hold3")
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_BUTTON5) PORT_NAME("Stop")
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("STROBE2")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_BUTTON6) PORT_NAME("Take win")
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_START1 )
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_BUTTON7) PORT_NAME( "Yes!" )
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_BUTTON8) PORT_NAME( "No!" )
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("STROBE3")
	PORT_BIT( 0x01, IP_ACTIVE_LOW,  IPT_INTERLOCK) PORT_NAME("Cashbox Door") PORT_CODE(KEYCODE_Q) PORT_TOGGLE
	PORT_BIT( 0x02, IP_ACTIVE_LOW,  IPT_INTERLOCK) PORT_NAME("Front Door") PORT_CODE(KEYCODE_W) PORT_TOGGLE
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_OTHER  )   PORT_NAME("Refill Key") PORT_CODE(KEYCODE_R) PORT_TOGGLE
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_BUTTON9)   PORT_NAME("Answer the phone")
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("STROBE4")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_UNUSED)

	PORT_START("STROBE5")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_UNUSED)

	PORT_START("STROBE6")
	PORT_DIPNAME( 0x07, 0x07, "PERCENTAGE KEY" ) PORT_DIPLOCATION("STROBE6:01,02,03")
	PORT_DIPSETTING(    0x00, "No key" )
	PORT_DIPSETTING(    0x01, "Key 1" )
	PORT_DIPSETTING(    0x02, "Key 2" )
	PORT_DIPSETTING(    0x03, "Key 3" )
	PORT_DIPSETTING(    0x04, "Key 4" )
	PORT_DIPSETTING(    0x05, "Key 5" )
	PORT_DIPSETTING(    0x06, "Key 6" )
	PORT_DIPSETTING(    0x07, "Key 7" )
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_UNUSED)

	PORT_START("STROBE7")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_UNUSED)

	PORT_START("STROBE8")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_UNUSED)

	PORT_START("STROBE9")
	PORT_SERVICE_NO_TOGGLE(0x01,IP_ACTIVE_HIGH)
	PORT_DIPNAME( 0x02, 0x00, "DIL02" ) PORT_DIPLOCATION("DIL:02")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x02, DEF_STR( On  ) )
	PORT_DIPNAME( 0x04, 0x00, "DIL03" ) PORT_DIPLOCATION("DIL:03")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x04, DEF_STR( On  ) )
	PORT_DIPNAME( 0x08, 0x00, "DIL04" ) PORT_DIPLOCATION("DIL:04")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x08, DEF_STR( On  ) )
	PORT_DIPNAME( 0x10, 0x00, "DIL05" ) PORT_DIPLOCATION("DIL:05")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x10, DEF_STR( On  ) )

	PORT_START("STROBE10")
	PORT_DIPNAME( 0x01, 0x00, "DIL06" ) PORT_DIPLOCATION("DIL:06")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x01, DEF_STR( On  ) )
	PORT_DIPNAME( 0x02, 0x00, "DIL07" ) PORT_DIPLOCATION("DIL:07")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x02, DEF_STR( On  ) )
	PORT_DIPNAME( 0x04, 0x00, "DIL08" ) PORT_DIPLOCATION("DIL:08")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x04, DEF_STR( On  ) )
	PORT_DIPNAME( 0x08, 0x00, "DIL10" ) PORT_DIPLOCATION("DIL:10")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x08, DEF_STR( On  ) )
	PORT_DIPNAME( 0x10, 0x00, "DIL11" ) PORT_DIPLOCATION("DIL:11")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x10, DEF_STR( On  ) )

	PORT_START("STROBE11")
	PORT_DIPNAME( 0x01, 0x00, "DIL12" ) PORT_DIPLOCATION("DIL:12")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x01, DEF_STR( On  ) )
	PORT_DIPNAME( 0x02, 0x00, "DIL13" ) PORT_DIPLOCATION("DIL:13")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x02, DEF_STR( On  ) )
	PORT_DIPNAME( 0x04, 0x00, "DIL14" ) PORT_DIPLOCATION("DIL:14")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x04, DEF_STR( On  ) )
	PORT_DIPNAME( 0x08, 0x00, "DIL15" ) PORT_DIPLOCATION("DIL:15")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x08, DEF_STR( On  ) )
	PORT_DIPNAME( 0x10, 0x00, "DIL16" ) PORT_DIPLOCATION("DIL:16")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x10, DEF_STR( On  ) )
INPUT_PORTS_END

static INPUT_PORTS_START( bfmcgslm )
	PORT_START("COINS")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_COIN1 ) PORT_IMPULSE(3)
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_COIN2 ) PORT_IMPULSE(3)
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_COIN3 ) PORT_IMPULSE(3)
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_COIN4 ) PORT_IMPULSE(3)
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_COIN5 ) PORT_IMPULSE(3)
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_COIN6 ) PORT_IMPULSE(3)
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_COIN7 ) PORT_IMPULSE(3)
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_COIN8 ) PORT_IMPULSE(3)

	PORT_START("STROBE0")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_UNUSED)

	PORT_START("STROBE1")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_BUTTON1) PORT_NAME("Cancel")
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_BUTTON2) PORT_NAME("Hold 1")
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_BUTTON3) PORT_NAME("Hold 2")
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_BUTTON4) PORT_NAME("Hold 3")
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_BUTTON5) PORT_NAME("Stop")
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("STROBE2")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_BUTTON6) PORT_NAME("Exchange")
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_START1 )
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("STROBE3")
	PORT_BIT( 0x01, IP_ACTIVE_LOW,  IPT_INTERLOCK) PORT_NAME("Cashbox Door") PORT_CODE(KEYCODE_Q) PORT_TOGGLE
	PORT_BIT( 0x02, IP_ACTIVE_LOW,  IPT_INTERLOCK) PORT_NAME("Front Door") PORT_CODE(KEYCODE_W) PORT_TOGGLE
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_OTHER)     PORT_NAME("Refill Key") PORT_CODE(KEYCODE_R) PORT_TOGGLE
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("STROBE4")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_UNUSED)

	PORT_START("STROBE5")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_UNUSED)

	PORT_START("STROBE6")
	PORT_DIPNAME( 0x07, 0x07, "PERCENTAGE KEY" ) PORT_DIPLOCATION("STROBE6:01,02,03")
	PORT_DIPSETTING(    0x00, "No key" )
	PORT_DIPSETTING(    0x01, "Key 1" )
	PORT_DIPSETTING(    0x02, "Key 2" )
	PORT_DIPSETTING(    0x03, "Key 3" )
	PORT_DIPSETTING(    0x04, "Key 4" )
	PORT_DIPSETTING(    0x05, "Key 5" )
	PORT_DIPSETTING(    0x06, "Key 6" )
	PORT_DIPSETTING(    0x07, "Key 7" )
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_UNUSED)

	PORT_START("STROBE7")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_UNUSED)

	PORT_START("STROBE8")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_UNUSED)

	PORT_START("STROBE9")
	PORT_SERVICE_NO_TOGGLE(0x01,IP_ACTIVE_HIGH)
	PORT_DIPNAME( 0x02, 0x00, "DIL02" ) PORT_DIPLOCATION("DIL:02")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x02, DEF_STR( On  ) )
	PORT_DIPNAME( 0x04, 0x00, "DIL03" ) PORT_DIPLOCATION("DIL:03")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x04, DEF_STR( On  ) )
	PORT_DIPNAME( 0x08, 0x00, "DIL04" ) PORT_DIPLOCATION("DIL:04")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x08, DEF_STR( On  ) )
	PORT_DIPNAME( 0x10, 0x00, "DIL05" ) PORT_DIPLOCATION("DIL:05")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x10, DEF_STR( On  ) )

	PORT_START("STROBE10")
	PORT_DIPNAME( 0x01, 0x00, "DIL06" ) PORT_DIPLOCATION("DIL:06")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x01, DEF_STR( On  ) )
	PORT_DIPNAME( 0x02, 0x00, "DIL07" ) PORT_DIPLOCATION("DIL:07")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x02, DEF_STR( On  ) )
	PORT_DIPNAME( 0x04, 0x00, "DIL08" ) PORT_DIPLOCATION("DIL:08")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x04, DEF_STR( On  ) )
	PORT_DIPNAME( 0x08, 0x00, "DIL10" ) PORT_DIPLOCATION("DIL:10")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x08, DEF_STR( On  ) )
	PORT_DIPNAME( 0x10, 0x00, "DIL11" ) PORT_DIPLOCATION("DIL:11")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x10, DEF_STR( On  ) )

	PORT_START("STROBE11")
	PORT_DIPNAME( 0x01, 0x00, "DIL12" ) PORT_DIPLOCATION("DIL:12")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x01, DEF_STR( On  ) )
	PORT_DIPNAME( 0x02, 0x00, "DIL13" ) PORT_DIPLOCATION("DIL:13")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x02, DEF_STR( On  ) )
	PORT_DIPNAME( 0x04, 0x00, "DIL14" ) PORT_DIPLOCATION("DIL:14")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x04, DEF_STR( On  ) )
	PORT_DIPNAME( 0x08, 0x00, "DIL15" ) PORT_DIPLOCATION("DIL:15")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x08, DEF_STR( On  ) )
	PORT_DIPNAME( 0x10, 0x00, "DIL16" ) PORT_DIPLOCATION("DIL:16")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x10, DEF_STR( On  ) )
INPUT_PORTS_END

static INPUT_PORTS_START( scorpion3 )
	PORT_START("COINS")
    PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_COIN1 ) PORT_IMPULSE(3) PORT_NAME("Fl 5.00")
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_COIN2 ) PORT_IMPULSE(3) PORT_NAME("Fl 2.50")
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_COIN3 ) PORT_IMPULSE(3) PORT_NAME("Fl 1.00")
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_COIN4 ) PORT_IMPULSE(3) PORT_NAME("Fl 0.25")
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_UNUSED)

	PORT_START("STROBE0")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_UNUSED)

	PORT_START("STROBE1")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_BUTTON1) PORT_NAME("Hold 1")
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_BUTTON2) PORT_NAME("Hold 1?")
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_BUTTON3) PORT_NAME("Hold 2/Hi")
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_BUTTON4) PORT_NAME("Hold 3/Lo")
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_BUTTON5) PORT_NAME("Cancel/Collect")
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("STROBE2")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_BUTTON6) PORT_NAME("Stop/Exchange")
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_START1 )
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("STROBE3")
	PORT_BIT( 0x01, IP_ACTIVE_LOW,  IPT_INTERLOCK) PORT_NAME("Cashbox Door") PORT_CODE(KEYCODE_Q) PORT_TOGGLE
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_INTERLOCK) PORT_NAME("Front Door") PORT_CODE(KEYCODE_W) PORT_TOGGLE
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_OTHER  )   PORT_NAME("Refill Key") PORT_CODE(KEYCODE_R) PORT_TOGGLE
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("STROBE4")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_UNUSED)

	PORT_START("STROBE5")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_UNUSED)

	PORT_START("STROBE6")
	PORT_DIPNAME( 0x07, 0x07, "PERCENTAGE KEY" ) PORT_DIPLOCATION("STROBE6:01,02,03")
	PORT_DIPSETTING(    0x00, "No key" )
	PORT_DIPSETTING(    0x01, "Key 1" )
	PORT_DIPSETTING(    0x02, "Key 2" )
	PORT_DIPSETTING(    0x03, "Key 3" )
	PORT_DIPSETTING(    0x04, "Key 4" )
	PORT_DIPSETTING(    0x05, "Key 5" )
	PORT_DIPSETTING(    0x06, "Key 6" )
	PORT_DIPSETTING(    0x07, "Key 7" )
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("STROBE7")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_UNUSED)

	PORT_START("STROBE8")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_UNUSED)

	PORT_START("STROBE9")
	PORT_SERVICE_NO_TOGGLE(0x01,IP_ACTIVE_HIGH)
	PORT_DIPNAME( 0x02, 0x00, "DIL02" ) PORT_DIPLOCATION("DIL:02")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x02, DEF_STR( On  ) )
	PORT_DIPNAME( 0x04, 0x00, "Attract Hi/Lo reel" ) PORT_DIPLOCATION("DIL:03")
	PORT_DIPSETTING(    0x04, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On  ) )
	PORT_DIPNAME( 0x08, 0x00, "Acceptor type" ) PORT_DIPLOCATION("DIL:04")
	PORT_DIPSETTING(    0x00, "Mars" )
	PORT_DIPSETTING(    0x08, "Sentinel" )
	PORT_DIPNAME( 0x10, 0x00, "DIL05" ) PORT_DIPLOCATION("DIL:05")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x10, DEF_STR( On  ) )

	PORT_START("STROBE10")
	PORT_DIPNAME( 0x01, 0x00, "Coin play" ) PORT_DIPLOCATION("DIL:06")
	PORT_DIPSETTING(    0x00, "Multi coin" )
	PORT_DIPSETTING(    0x01, "Single coin" )
	PORT_DIPNAME( 0x02, 0x00, "CashPot Freq" ) PORT_DIPLOCATION("DIL:07")
	PORT_DIPSETTING(    0x00, DEF_STR( High) )
	PORT_DIPSETTING(    0x02, DEF_STR( Low ) )
	PORT_DIPNAME( 0x04, 0x00, "DIL08" ) PORT_DIPLOCATION("DIL:08")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x04, DEF_STR( On  ) )
	PORT_DIPNAME( 0x08, 0x00, "Coin Jam Alarm" ) PORT_DIPLOCATION("DIL:10")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x08, DEF_STR( On  ) )
	PORT_DIPNAME( 0x10, 0x00, "DIL11" ) PORT_DIPLOCATION("DIL:11")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x10, DEF_STR( On  ) )

	PORT_START("STROBE11")
	PORT_DIPNAME( 0x01, 0x00, "DIL12" ) PORT_DIPLOCATION("DIL:12")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x01, DEF_STR( On  ) )
	PORT_DIPNAME( 0x02, 0x00, "DIL13" ) PORT_DIPLOCATION("DIL:13")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x02, DEF_STR( On  ) )
	PORT_DIPNAME( 0x1C, 0x00, "Percentage setting" ) PORT_DIPLOCATION("DIL:14,15,16")
	PORT_DIPSETTING(    0x0C, "72%" )
	PORT_DIPSETTING(    0x04, "75%" )
	PORT_DIPSETTING(    0x00, "78%" )
	PORT_DIPSETTING(    0x10, "81%" )
	PORT_DIPSETTING(    0x18, "85%" )

INPUT_PORTS_END


/* machine driver for scorpion2 board */

static MACHINE_CONFIG_START( scorpion2, bfm_sc2_state )
	MCFG_MACHINE_RESET(awp_init)
	MCFG_CPU_ADD("maincpu", M6809, MASTER_CLOCK/4 )
	MCFG_CPU_PROGRAM_MAP(sc2_basemap)
	MCFG_CPU_PERIODIC_INT(timer_irq, 1000 )
	MCFG_WATCHDOG_TIME_INIT(PERIOD_OF_555_MONOSTABLE(120000,100e-9))

	MCFG_SPEAKER_STANDARD_MONO("mono")
	MCFG_SOUND_ADD("upd",UPD7759, UPD7759_STANDARD_CLOCK)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.50)

	MCFG_SOUND_ADD("ymsnd",YM2413, XTAL_3_579545MHz)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 1.0)

	MCFG_NVRAM_ADD_0FILL("nvram")
	MCFG_NVRAM_HANDLER(bfm_sc2)

	/* video hardware */
	MCFG_DEFAULT_LAYOUT(layout_awpvid14)
MACHINE_CONFIG_END


/* machine driver for scorpion3 board */
static MACHINE_CONFIG_DERIVED( scorpion3, scorpion2 )
	MCFG_CPU_MODIFY("maincpu")
	MCFG_CPU_PROGRAM_MAP(sc2_basemap)
MACHINE_CONFIG_END


/* machine driver for scorpion2 board + matrix board */
static MACHINE_CONFIG_START( scorpion2_dm01, bfm_sc2_state )
	MCFG_MACHINE_RESET(dm01_init)
	MCFG_QUANTUM_TIME(attotime::from_hz(960))									// needed for serial communication !!
	MCFG_CPU_ADD("maincpu", M6809, MASTER_CLOCK/4 )
	MCFG_CPU_PROGRAM_MAP(sc2_basemap)
	MCFG_CPU_PERIODIC_INT(timer_irq, 1000 )
	MCFG_WATCHDOG_TIME_INIT(PERIOD_OF_555_MONOSTABLE(120000,100e-9))


	MCFG_SPEAKER_STANDARD_MONO("mono")
	MCFG_SOUND_ADD("ymsnd",YM2413, XTAL_3_579545MHz)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 1.0)

	MCFG_SOUND_ADD("upd",UPD7759, UPD7759_STANDARD_CLOCK)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.50)

	MCFG_NVRAM_ADD_0FILL("nvram")
	MCFG_NVRAM_HANDLER(bfm_sc2)

	/* video hardware */
	MCFG_DEFAULT_LAYOUT(layout_awpdmd)
	MCFG_CPU_ADD("matrix", M6809, 2000000 )				/* matrix board 6809 CPU at 2 Mhz ?? I don't know the exact freq.*/
	MCFG_CPU_PROGRAM_MAP(bfm_dm01_memmap)
	MCFG_CPU_PERIODIC_INT(bfm_dm01_vbl, 1500 )			/* generate 1500 NMI's per second ?? what is the exact freq?? */
MACHINE_CONFIG_END

static void sc2awp_common_init(running_machine &machine,int reels, int decrypt)
{
	bfm_sc2_state *state = machine.driver_data<bfm_sc2_state>();

	int n;
	sc2_common_init(machine, decrypt);
	/* setup n default 96 half step reels */

	state->m_reels=reels;

	for ( n = 0; n < reels; n++ )
	{
		stepper_config(machine, n, &starpoint_interface_48step);
	}
	if (reels)
	{
		awp_reel_setup();
	}
}

static DRIVER_INIT (bbrkfst)
{
	bfm_sc2_state *state = machine.driver_data<bfm_sc2_state>();
	sc2awp_common_init(machine,5, 1);
	MechMtr_config(machine,8);

	BFM_BD1_init(0);
	BFM_BD1_init(1);

	state->m_has_hopper = 0;

	Scorpion2_SetSwitchState(machine,4,0, 1);	  /* GBP1 Low Level Switch */
	Scorpion2_SetSwitchState(machine,4,1, 1);	  /* 20p Low Level Switch */
	Scorpion2_SetSwitchState(machine,4,2, 1);	  /* Token Front Low Level Switch */
	Scorpion2_SetSwitchState(machine,4,3, 1);	  /* Token Rear  Low Level Switch */
	Scorpion2_SetSwitchState(machine,4,4, 1);
	Scorpion2_SetSwitchState(machine,6,0, 0);
	Scorpion2_SetSwitchState(machine,6,1, 1);
	Scorpion2_SetSwitchState(machine,6,2, 0);
	Scorpion2_SetSwitchState(machine,6,3, 1);

}

static DRIVER_INIT (drwho_common)
{
	bfm_sc2_state *state = machine.driver_data<bfm_sc2_state>();

	MechMtr_config(machine,8);

	BFM_BD1_init(0);
	BFM_BD1_init(1);

	state->m_has_hopper = 0;

	Scorpion2_SetSwitchState(machine,4,0, 0);	  /* GBP1 Low Level Switch */
	Scorpion2_SetSwitchState(machine,4,1, 0);	  /* 20p Low Level Switch */
	Scorpion2_SetSwitchState(machine,4,2, 0);	  /* Token Front Low Level Switch */
	Scorpion2_SetSwitchState(machine,4,3, 0);	  /* Token Rear  Low Level Switch */
	Scorpion2_SetSwitchState(machine,7,0, 0);	  /* GBP1 High Level Switch */
	Scorpion2_SetSwitchState(machine,7,1, 0);	  /* 20P High Level Switch */
	Scorpion2_SetSwitchState(machine,7,2, 0);	  /* Token Front High Level Switch */
	Scorpion2_SetSwitchState(machine,7,3, 0);	  /* Token Rear High Level Switch */
}

static DRIVER_INIT (drwho)
{
	sc2awp_common_init(machine,6, 1);
	DRIVER_INIT_CALL(drwho_common);
}

static DRIVER_INIT (drwhon)
{
	sc2awp_common_init(machine,4, 0);
	DRIVER_INIT_CALL(drwho_common);
}


static DRIVER_INIT (focus)
{
	sc2awp_common_init(machine,6, 1);
	MechMtr_config(machine,5);

	BFM_BD1_init(0);
}

static DRIVER_INIT (cpeno1)
{
	bfm_sc2_state *state = machine.driver_data<bfm_sc2_state>();
	sc2awp_common_init(machine,6, 1);

	MechMtr_config(machine,5);

	Scorpion2_SetSwitchState(machine,3,3,1);	/*  5p play */
	Scorpion2_SetSwitchState(machine,3,4,1);	/* 20p play */

	Scorpion2_SetSwitchState(machine,4,0,1);	/* pay tube low (1 pound front) */
	Scorpion2_SetSwitchState(machine,4,1,1);	/* pay tube low (20p) */
	Scorpion2_SetSwitchState(machine,4,2,1);	/* pay tube low (?1 right) */
	Scorpion2_SetSwitchState(machine,4,3,1);	/* pay tube low (?1 left) */

	Scorpion2_SetSwitchState(machine,5,0,1);	/* pay sensor (GBP1 front) */
	Scorpion2_SetSwitchState(machine,5,1,1);	/* pay sensor (20 p) */
	Scorpion2_SetSwitchState(machine,5,2,1);	/* pay sensor (1 right) */
	Scorpion2_SetSwitchState(machine,5,3,1);	/* pay sensor (?1 left) */
	Scorpion2_SetSwitchState(machine,5,4,1);	/* payout unit present */

	state->m_slide_pay_sensor[0] = 0x50;
	state->m_slide_pay_sensor[1] = 0x51;
	state->m_slide_pay_sensor[2] = 0x52;
	state->m_slide_pay_sensor[3] = 0x53;
	state->m_slide_pay_sensor[4] = 0;
	state->m_slide_pay_sensor[5] = 0;

	Scorpion2_SetSwitchState(machine,6,0,1);	/* ? percentage key */
	Scorpion2_SetSwitchState(machine,6,1,1);
	Scorpion2_SetSwitchState(machine,6,2,1);
	Scorpion2_SetSwitchState(machine,6,3,1);
	Scorpion2_SetSwitchState(machine,6,4,1);

	Scorpion2_SetSwitchState(machine,7,0,0);	/* GBP1 High Level Switch  */
	Scorpion2_SetSwitchState(machine,7,1,0);	/* 20P High Level Switch */
	Scorpion2_SetSwitchState(machine,7,2,0);	/* Token Front High Level Switch */
	Scorpion2_SetSwitchState(machine,7,3,0);	/* Token Rear High Level Switch */

	state->m_sc2_show_door   = 1;
	state->m_sc2_door_state  = 0x31;

	state->m_has_hopper = 0;
}

static DRIVER_INIT (bfmcgslm)
{
	bfm_sc2_state *state = machine.driver_data<bfm_sc2_state>();
	sc2awp_common_init(machine,6, 1);
	MechMtr_config(machine,8);
	BFM_BD1_init(0);
	state->m_has_hopper = 0;
}

static DRIVER_INIT (luvjub)
{
	bfm_sc2_state *state = machine.driver_data<bfm_sc2_state>();
	sc2awp_common_init(machine,6, 1);
	MechMtr_config(machine,8);
	state->m_has_hopper = 0;

	Scorpion2_SetSwitchState(machine,3,0,1);
	Scorpion2_SetSwitchState(machine,3,1,1);

	Scorpion2_SetSwitchState(machine,4,0,1);
	Scorpion2_SetSwitchState(machine,4,1,1);
	Scorpion2_SetSwitchState(machine,4,2,1);
	Scorpion2_SetSwitchState(machine,4,3,1);

	Scorpion2_SetSwitchState(machine,6,0,1);
	Scorpion2_SetSwitchState(machine,6,1,1);
	Scorpion2_SetSwitchState(machine,6,2,1);
	Scorpion2_SetSwitchState(machine,6,3,0);

	Scorpion2_SetSwitchState(machine,7,0,0);
	Scorpion2_SetSwitchState(machine,7,1,0);
	Scorpion2_SetSwitchState(machine,7,2,0);
	Scorpion2_SetSwitchState(machine,7,3,0);
}


ROM_START( sc2brkfs )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD("big-breakfast_std_ar_var_a.bin",	0x00000, 0x10000, CRC(5f016daa) SHA1(25ee10138bddf453588e3c458268533a88a51217) )

	ROM_REGION( 0x10000, "altrevs", 0 )
	ROM_LOAD( "big-breakfast_dat_ac_10pnd-20p_a.bin", 0x0000, 0x010000, CRC(cc54617f) SHA1(078e56b948d68ebcfaf986dd0f15be64607d0e4f) )
	ROM_LOAD( "big-breakfast_dat_ac_var_10pnd-20p_a.bin", 0x0000, 0x010000, CRC(a5967b05) SHA1(f0d4bc804181781a391fa052251c4bbf7d8f5e50) )
	ROM_LOAD( "big-breakfast_dat_ac_var_8pnd_a.bin", 0x0000, 0x010000, CRC(d97dbf7a) SHA1(d46270ff69cbc636744fc902d38cc282613cfdd2) )
	ROM_LOAD( "big-breakfast_dat_ar_var_a.bin", 0x0000, 0x010000, CRC(ade2834f) SHA1(54914fbc8416b2d08c13c56088b1665e267e6777) )
	ROM_LOAD( "big-breakfast_dat_ss_var_a.bin", 0x0000, 0x010000, CRC(57aff227) SHA1(5d4c6190194719b3fa5c02d30e7c6b59978c93c3) )

	ROM_REGION( 0x80000, "upd", 0 )
	ROM_LOAD("bigbreakfastsnd.bin", 0x00000, 0x80000, CRC(bf91aa2b) SHA1(40942165e65ff9b027015d500e5a9726c44ba1c5))
ROM_END

ROM_START( sc2brkfsm )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "bigbreakfastcasino", 0x0000, 0x010000, CRC(db45b17b) SHA1(927513f6fe326b216b0f13f34bbbc9970ab4f0cc) )
	ROM_LOAD( "98400022", 0x0000, 0x010000, CRC(66482cbb) SHA1(933d8ec98d5bc3026d547b657093e07f96fbdafa) )
	ROM_LOAD( "98400020", 0x0000, 0x010000, CRC(7a18f268) SHA1(ad352d613333072c62c38a493cf3183d387b7562) )

	ROM_REGION( 0x80000, "upd", 0 ) // might not be right for this version
	ROM_LOAD("bigbreakfastsnd.bin", 0x00000, 0x80000, CRC(bf91aa2b) SHA1(40942165e65ff9b027015d500e5a9726c44ba1c5))
ROM_END


ROM_START( sc2brkfs1 )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD("big-breakfast_std_ss_var_a.bin",	0x00000, 0x10000, CRC(08d1fa7d) SHA1(a3dba79eef32835f0b46dbd7b376b797324df904) )

	ROM_REGION( 0x80000, "upd", 0 )
	ROM_LOAD("bigbreakfastsnd.bin", 0x00000, 0x80000, CRC(bf91aa2b) SHA1(40942165e65ff9b027015d500e5a9726c44ba1c5))
ROM_END

ROM_START( sc2brkfs2 )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD("big-breakfast_std_ac_var_jp-8_a.bin",	0x00000, 0x10000, CRC(2671af1b) SHA1(0a34dd2953a99be9fb2a128f9d1f7ddc0fc8242a) )

	ROM_REGION( 0x80000, "upd", 0 )
	ROM_LOAD("bigbreakfastsnd.bin", 0x00000, 0x80000, CRC(bf91aa2b) SHA1(40942165e65ff9b027015d500e5a9726c44ba1c5))
ROM_END

ROM_START( sc2brkfs3 )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD("big-breakfast_std_ac_8pnd20p_a.bin",	0x00000, 0x10000, CRC(054c38ad) SHA1(f4ab55f977848e3d2a933bba1ab619ffa3e14db6) )

	ROM_REGION( 0x80000, "upd", 0 )
	ROM_LOAD("bigbreakfastsnd.bin", 0x00000, 0x80000, CRC(bf91aa2b) SHA1(40942165e65ff9b027015d500e5a9726c44ba1c5))
ROM_END

ROM_START( sc2brkfs4 )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD("big-breakfast_std_ac_var_10pnd-20p_a.bin",	0x00000, 0x10000, CRC(d879feaa) SHA1(2656fbe018fe40194c2b77d289b77fabbc9e537c) )

	ROM_REGION( 0x80000, "upd", 0 )
	ROM_LOAD("bigbreakfastsnd.bin", 0x00000, 0x80000, CRC(bf91aa2b) SHA1(40942165e65ff9b027015d500e5a9726c44ba1c5))
ROM_END

ROM_START( sc2brkfs5 )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD("big-breakfast_std_ac_10pnd-20p_a.bin",	0x00000, 0x10000, CRC(55d7321c) SHA1(0b4a6b66aa64fbb3238539a2167f761d0910b814) )

	ROM_REGION( 0x80000, "upd", 0 )
	ROM_LOAD("bigbreakfastsnd.bin", 0x00000, 0x80000, CRC(bf91aa2b) SHA1(40942165e65ff9b027015d500e5a9726c44ba1c5))
ROM_END


ROM_START( sc2drwho )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD("95750288.bin",	0x00000, 0x10000, CRC(fe95b5a5) SHA1(876a812f69903fd99f896b35eeaf132c215b0035) ) // dr-who-time-lord_std_ss_20p_ass.bin


	ROM_REGION( 0x80000, "upd", 0 )
	ROM_LOAD("timelordsnd.bin", 0x00000, 0x80000, CRC(57fdaf3a) SHA1(f7cbaddb7f2ab8e1c7b17f187bab263e0dde463b))
ROM_END

ROM_START( sc2drwho1 )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD("95750661.p1",	0x00000, 0x10000, CRC(4b5b50eb) SHA1(fe2b820c214b3e967348b99ccff30a4bfe0251dc) )

	ROM_REGION( 0x80000, "upd", 0 )
	ROM_LOAD("timelordsnd.bin", 0x00000, 0x80000, CRC(57fdaf3a) SHA1(f7cbaddb7f2ab8e1c7b17f187bab263e0dde463b))
ROM_END

ROM_START( sc2drwho2 )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD("dr-who-time-lord_dat_ac_ass.bin",	0x00000, 0x10000, CRC(5a467a44) SHA1(d5a3dcdf50e07e36187350072b5d82d620f8f1d8) )

	ROM_REGION( 0x80000, "upd", 0 )
	ROM_LOAD("timelordsnd.bin", 0x00000, 0x80000, CRC(57fdaf3a) SHA1(f7cbaddb7f2ab8e1c7b17f187bab263e0dde463b))
ROM_END

ROM_START( sc2drwho3 )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD("dr-who-time-lord_dat_ss_20p_ass.bin",	0x00000, 0x10000, CRC(8ce06af9) SHA1(adb58507b2b6aae59857384748d59485f1739eaf) )

	ROM_REGION( 0x80000, "upd", 0 )
	ROM_LOAD("timelordsnd.bin", 0x00000, 0x80000, CRC(57fdaf3a) SHA1(f7cbaddb7f2ab8e1c7b17f187bab263e0dde463b))
ROM_END

ROM_START( sc2drwho4 )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD("dr-who-time-lord_std_ac_ass.bin",	0x00000, 0x10000, CRC(053313cc) SHA1(2a52b7edae0ce676255eb347bba17a2e48c1707a) )

	ROM_REGION( 0x80000, "upd", 0 )
	ROM_LOAD("timelordsnd.bin", 0x00000, 0x80000, CRC(57fdaf3a) SHA1(f7cbaddb7f2ab8e1c7b17f187bab263e0dde463b))
ROM_END

ROM_START( sc2drwho5 )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD("dr-who-time-lord_std_var_20p_ass.bin",	0x00000, 0x10000, CRC(35f4e6ab) SHA1(5e5e35889adb7d3384aae663c667b0251d39aeee) )

	ROM_REGION( 0x80000, "upd", 0 )
	ROM_LOAD("timelordsnd.bin", 0x00000, 0x80000, CRC(57fdaf3a) SHA1(f7cbaddb7f2ab8e1c7b17f187bab263e0dde463b))
ROM_END

ROM_START( sc2drwho6 )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD("time-lord_dat_20p_uk94_ass.bin",	0x00000, 0x10000, CRC(e65717c2) SHA1(9b8db0bcac9fd996de29527440d6af3592102120) )

	ROM_REGION( 0x80000, "upd", 0 )
	ROM_LOAD("timelordsnd.bin", 0x00000, 0x80000, CRC(57fdaf3a) SHA1(f7cbaddb7f2ab8e1c7b17f187bab263e0dde463b))
ROM_END

ROM_START( sc2drwho7 )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD("time-lord_dat_ac_10pnd-20p-25p_ass.bin",	0x00000, 0x10000, CRC(9a27ac6d) SHA1(d1b0e85d41198c5d2cd1b492e53359a5dc1ac474) )

	ROM_REGION( 0x80000, "upd", 0 )
	ROM_LOAD("timelordsnd.bin", 0x00000, 0x80000, CRC(57fdaf3a) SHA1(f7cbaddb7f2ab8e1c7b17f187bab263e0dde463b))
ROM_END

ROM_START( sc2drwho8 )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD("time-lord_dat_ac_8pnd-20p_ass.bin",	0x00000, 0x10000, CRC(b6629b5e) SHA1(d20085b4ab9a0786063eb063f7d1df2a6814f40c) )

	ROM_REGION( 0x80000, "upd", 0 )
	ROM_LOAD("timelordsnd.bin", 0x00000, 0x80000, CRC(57fdaf3a) SHA1(f7cbaddb7f2ab8e1c7b17f187bab263e0dde463b))
ROM_END

ROM_START( sc2drwho9 )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD("time-lord_dat_ar_10p_ass.bin",	0x00000, 0x10000, CRC(04653c3b) SHA1(0c23f939103772fac628342074de820ec6b472ce) )

	ROM_REGION( 0x80000, "upd", 0 )
	ROM_LOAD("timelordsnd.bin", 0x00000, 0x80000, CRC(57fdaf3a) SHA1(f7cbaddb7f2ab8e1c7b17f187bab263e0dde463b))
ROM_END

ROM_START( sc2drwho10 )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD("time-lord_dat_ar_20p_uk94_ass.bin",	0x00000, 0x10000, CRC(40aaa98f) SHA1(80705e24e419558d8a7b1f886bfc2b3ce5465446) )

	ROM_REGION( 0x80000, "upd", 0 )
	ROM_LOAD("timelordsnd.bin", 0x00000, 0x80000, CRC(57fdaf3a) SHA1(f7cbaddb7f2ab8e1c7b17f187bab263e0dde463b))
ROM_END

ROM_START( sc2drwho11 )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD("time-lord_dat_var_no-jp-spin_ass.bin",	0x00000, 0x10000, CRC(bf087547) SHA1(f4b7289a76e814af5fb3affc360a9ac659c09bbe) )

	ROM_REGION( 0x80000, "upd", 0 )
	ROM_LOAD("timelordsnd.bin", 0x00000, 0x80000, CRC(57fdaf3a) SHA1(f7cbaddb7f2ab8e1c7b17f187bab263e0dde463b))
ROM_END

ROM_START( sc2drwho12 )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD("time-lord_std_20p_uk94_ass.bin",	0x00000, 0x10000, CRC(278f559e) SHA1(d4396df02a5e24b3684c26fcaa57c8e499789332) )

	ROM_REGION( 0x80000, "upd", 0 )
	ROM_LOAD("timelordsnd.bin", 0x00000, 0x80000, CRC(57fdaf3a) SHA1(f7cbaddb7f2ab8e1c7b17f187bab263e0dde463b))
ROM_END

ROM_START( sc2drwho13 )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD("time-lord_std_ac_8pnd-20p_ass.bin",	0x00000, 0x10000, CRC(0b2850c8) SHA1(5fac64f35a6b6158d8c15f41e82574768b1c3617) )

	ROM_REGION( 0x80000, "upd", 0 )
	ROM_LOAD("timelordsnd.bin", 0x00000, 0x80000, CRC(57fdaf3a) SHA1(f7cbaddb7f2ab8e1c7b17f187bab263e0dde463b))
ROM_END

ROM_START( sc2drwho14 )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD("time-lord_std_ar_10p_ass.bin",	0x00000, 0x10000, CRC(f716a21d) SHA1(340df4cdea3309bfebeba7c419057f1bf5ed5024) )

	ROM_REGION( 0x80000, "upd", 0 )
	ROM_LOAD("timelordsnd.bin", 0x00000, 0x80000, CRC(57fdaf3a) SHA1(f7cbaddb7f2ab8e1c7b17f187bab263e0dde463b))
ROM_END

ROM_START( sc2drwho15 )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD("time-lord_std_ar_20p_uk94_ass.bin",	0x00000, 0x10000, CRC(8dd0f908) SHA1(2eca748874cc061f9a8145b081d2c097a40e1e47) )

	ROM_REGION( 0x80000, "upd", 0 )
	ROM_LOAD("timelordsnd.bin", 0x00000, 0x80000, CRC(57fdaf3a) SHA1(f7cbaddb7f2ab8e1c7b17f187bab263e0dde463b))
ROM_END

ROM_START( sc2drwho16 )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD("tmld5pa",	0x00000, 0x10000, CRC(b9ddfd0d) SHA1(915afd83eab330a0e70635c35f031f2041b9f5ad) )

	ROM_REGION( 0x80000, "upd", 0 )
	ROM_LOAD("timelordsnd.bin", 0x00000, 0x80000, CRC(57fdaf3a) SHA1(f7cbaddb7f2ab8e1c7b17f187bab263e0dde463b))
ROM_END

ROM_START( sc2drwho18 )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "98401002.bin", 0x0000, 0x010000, CRC(e7c23331) SHA1(f6823fa206d28f53a13ef44c9e4cf37d6b8aa758) )

	ROM_REGION( 0x80000, "upd", 0 )
	ROM_LOAD("timelordsnd.bin", 0x00000, 0x80000, CRC(57fdaf3a) SHA1(f7cbaddb7f2ab8e1c7b17f187bab263e0dde463b))
ROM_END

ROM_START( sc2drwho19 )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "98400002.bin", 0x0000, 0x010000, CRC(40cc7d8b) SHA1(05f98e29bb92b3581691ee6df8ff5ae73e351d40) )

	ROM_REGION( 0x80000, "upd", 0 )
	ROM_LOAD("timelordsnd.bin", 0x00000, 0x80000, CRC(57fdaf3a) SHA1(f7cbaddb7f2ab8e1c7b17f187bab263e0dde463b))
ROM_END

ROM_START( sc2drwho20 )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "tlrdx56c", 0x0000, 0x010000, CRC(80da4ba0) SHA1(0c725da5eead9371d895ca9650fbbec8aa1509b2) )

	ROM_REGION( 0x80000, "upd", 0 )
	ROM_LOAD("timelordsnd.bin", 0x00000, 0x80000, CRC(57fdaf3a) SHA1(f7cbaddb7f2ab8e1c7b17f187bab263e0dde463b))
ROM_END


/* not encrypted, bootleg? */
ROM_START( sc2drwho17 )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD("drwho.bin",	0x00000, 0x10000, CRC(9e53a1f7) SHA1(60c6aa226c96678a6e487fbf0f32554fd85ebd66) )

	ROM_REGION( 0x80000, "upd", 0 )
	ROM_LOAD("timelordsnd.bin", 0x00000, 0x80000, CRC(57fdaf3a) SHA1(f7cbaddb7f2ab8e1c7b17f187bab263e0dde463b))
ROM_END



ROM_START( sc2focus )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD("focus.bin",	 0x00000, 0x10000, CRC(ddd1a21e) SHA1(cbb467b03642d6de37f6dc204b902f2d7e92230e))

	ROM_REGION( 0x20000, "upd", 0 )
	ROM_LOAD("focsound.bin", 0x00000, 0x20000, CRC(fce86700) SHA1(546680dd85234608c1b7e850bad3165400fd981c))
ROM_END


ROM_START( sc2gslam )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD("95750843.bin", 0x00000, 0x10000, CRC(e159ddf6) SHA1(c897564a956becbd9d4c155df33b239e899156c0))

	ROM_REGION( 0x10000, "altrevs", 0 )
	ROM_LOAD( "95752056", 0x0000, 0x010000, CRC(b28dcd9c) SHA1(f20ef0f0a1b5cc287cf93a175fede98dde3fecf4) )
	ROM_LOAD( "club-grand-slam_dat_ac_var_rot_ass.bin", 0x0000, 0x010000, CRC(d505db66) SHA1(6e40186a699a81138674e332acbd0d7d3939b9f6) )
	ROM_LOAD( "club-grand-slam_dat_acss.bin", 0x0000, 0x010000, CRC(82ff3cb9) SHA1(87794063421724201c8a3e67cd6e454b0f578c3e) )
	ROM_LOAD( "club-grand-slam_std_ac_ass.bin", 0x0000, 0x010000, CRC(b28dcd9c) SHA1(f20ef0f0a1b5cc287cf93a175fede98dde3fecf4) )

	ROM_REGION( 0x40000, "upd", 0 )
	ROM_LOAD("gslamsnd.bin", 0x00000, 0x40000, CRC(9afb8b42) SHA1(20e108c0041412fcd7b2969701f47a4a99d3677c))

	ROM_REGION( 0x80000, "altupd", 0 )
	ROM_LOAD( "grandslamsnd.bin", 0x0000, 0x080000, CRC(e4af3787) SHA1(9aa40f7c4c4db3618b553505b02663c1d5f297c3) )
	ROM_LOAD( "gslamsndb.bin", 0x0000, 0x080000, CRC(c9dfb6f5) SHA1(6e529c210b26e7ce164cebbff8ec314c6fa8f7bf) )
ROM_END



ROM_START( sc2cshcl )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "club_cashino_dat_ac_f65_rot_ass.bin", 0x0000, 0x010000, CRC(c2552162) SHA1(2c373b60588d870acd34d88025f6bb14687694fb) )

	ROM_REGION( 0x10000, "altrevs", 0 )
	ROM_LOAD( "club_cashino_dat_ac_var_ass.bin", 0x0000, 0x010000, CRC(b529604e) SHA1(87f8dca7e570472697de2cbe7565a038503a6251) )
	ROM_LOAD( "club_cashino_std_ac_f65_rot_ass.bin", 0x0000, 0x010000, CRC(23aa2c72) SHA1(155df9b501cf5ae9eb3afca48c4100617793ac09) )
	ROM_LOAD( "club_cashino_std_ac_var_ass.bin", 0x0000, 0x010000, CRC(0e9fad24) SHA1(d14569f106ba29f9cb7769234f5531382e28bd69) )

	ROM_REGION( 0x80000, "upd", 0 )
	ROM_LOAD( "cashsnd", 0x0000, 0x080000, CRC(807d37a6) SHA1(bd5f7c39a64a562e96a850a2cc82bfe3f74f1e54) )
ROM_END


ROM_START( sc2catms )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "cat and mouse p1.bin", 0x0000, 0x010000, CRC(b33b2a75) SHA1(ac57b4d33ac1218e39b8bbd669c40bdbb3839ccf) )

	ROM_REGION( 0x10000, "altrevs", 0 )
	ROM_LOAD( "cat and mouse ver puss7.2.bin", 0x0000, 0x010000, CRC(6968bf9c) SHA1(c44faf2e5b391bee43021ad8544fb8d502f90433) )
	ROM_LOAD( "cat-and-mouse-mk2_dat_20p_uk94_ass.bin", 0x0000, 0x010000, CRC(6806cfad) SHA1(8eb427688bc19e9b1508de1afa584bcba7e8d421) )
	ROM_LOAD( "cat-and-mouse-mk2_dat_ar_10p_ass.bin", 0x0000, 0x010000, CRC(c332595b) SHA1(3ea62b98129913b2ff576c42cfa7fe4d15a34b8e) )
	ROM_LOAD( "cat-and-mouse-mk2_dat_ar_20p_uk94_ass.bin", 0x0000, 0x010000, CRC(22e2d319) SHA1(ca3f335f9f52cd152e420bd6c2e15fc1fac4eb29) )
	ROM_LOAD( "cat-and-mouse-mk2_dat_ar_ac_8pnd-20p_uk94_ass.bin", 0x0000, 0x010000, CRC(87b5fc94) SHA1(3e2b4aba0847fe1958710bff394ea98e02276b43) )
	ROM_LOAD( "cat-and-mouse-mk2_std_20p_uk94_ass.bin", 0x0000, 0x010000, CRC(d8e72750) SHA1(b0431cbb311c88b4701bae3bbfdf1d45a070181c) )
	ROM_LOAD( "cat-and-mouse-mk2_std_ar_20p_uk94_ass.bin", 0x0000, 0x010000, CRC(95beca0c) SHA1(6e2b175139c616cf80f020588b073f325a0c2684) )
	ROM_LOAD( "cat-and-mouse-mk2_std_ar_ac_8pnd-20p_uk94_ass.bin", 0x0000, 0x010000, CRC(c5fccfb0) SHA1(c427b42da60cd14516991a08a08f68421fa9ff88) )
	ROM_LOAD( "cat-and-mouse_dat_ac_10pnd-25p_ass.bin", 0x0000, 0x010000, CRC(d9811472) SHA1(dffab64155ed2c5193c24a660af7ad7c3c7bc093) )
	ROM_LOAD( "cat-and-mouse_dat_ac_10pnd_ass.bin", 0x0000, 0x010000, CRC(40ba729e) SHA1(d7b4fe209588d77921d6c37d1739805aed80f103) )
	ROM_LOAD( "cat-and-mouse_std_ac_10pnd-25p_ass.bin", 0x0000, 0x010000, CRC(4c538143) SHA1(4045599cfe57f442ac58aa1f0ed3a03ce63e2e4c) )
	ROM_LOAD( "cat-and-mouse_std_ac_10pnd_ass.bin", 0x0000, 0x010000, CRC(05396936) SHA1(61d976c22ba82bbff12fdcfb6b9320efebc9ad37) )
	ROM_LOAD( "cm20std", 0x0000, 0x010000, CRC(74ca0fd5) SHA1(2345bf3810820a12c613013fedad936ab9134b22) )
	ROM_LOAD( "cnm20mk2", 0x0000, 0x010000, CRC(0604a78a) SHA1(c75b90f93b1d36928ad46643cfce03dda2b20408) )

	ROM_REGION( 0x80000, "upd", 0 )
	ROM_LOAD( "catandmousesnd.bin", 0x0000, 0x080000, CRC(00d3b224) SHA1(5ae35a7bfa65e8343564e6f6a219bc674710fadc) )
ROM_END




ROM_START( sc2eggs )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "eggs-on-legs_std_wi_ac_10pnd-20p_ass.bin", 0x0000, 0x010000, CRC(3fdad116) SHA1(d5fc405af8b14d8b85acb10aaa3c8a219753c864) )

	ROM_REGION( 0x10000, "altrevs", 0 )
	ROM_LOAD( "eggs-on-legs_dat_ac_var_10pnd_ass.bin", 0x0000, 0x010000, CRC(718915f2) SHA1(717b57c0e81a48db005516135fdd4d82f7cfda28) )
	ROM_LOAD( "eggs-on-legs_dat_wi_ac_10pnd-20p_ass.bin", 0x0000, 0x010000, CRC(cdde5a4d) SHA1(b61e61193db4921217a7c285fd8fe2780d1f8091) )
	ROM_LOAD( "95750746.p1", 0x0000, 0x010000, CRC(a4b13487) SHA1(7ef2953ca11526bbae57b1aebb7a90de59c2d379) )

	ROM_REGION( 0x80000, "upd", 0 )
	ROM_LOAD( "eggsonlegssnd.bin", 0x0000, 0x080000, CRC(24fef504) SHA1(75a05e0cf064f736dd9164c24ccef77a46aaee94) )
ROM_END


ROM_START( sc2gsclb )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "club-game-show_std_ac_p65_ass.bin", 0x0000, 0x010000, CRC(9a390095) SHA1(ee4b08956de0b018b9ceaf16a6410463053c1f3d) )

	ROM_REGION( 0x10000, "altrevs", 0 )
	ROM_LOAD( "club-game-show_dat_ac_250pnd-25p_ass.bin", 0x0000, 0x010000, CRC(299b89f3) SHA1(eb78378410ca2380ec564e8268a51309dc8044ce) )
	ROM_LOAD( "club-game-show_dat_ac_250pnd-25p_p65_ass.bin", 0x0000, 0x010000, CRC(5d59e87e) SHA1(91684551db11d95768c364515cf5cd337b3f482b) )
	ROM_LOAD( "club-game-show_dat_ac_p65_ass.bin", 0x0000, 0x010000, CRC(61adb76f) SHA1(a7fcc6504d5eeae664b9aaca190bbf43bd989c93) )
	ROM_LOAD( "club-game-show_dat_ac_var_ass.bin", 0x0000, 0x010000, CRC(85cf033f) SHA1(ca7e506437e1ff229f2d79bedb13ae0fe5dd2696) )
	ROM_LOAD( "club-game-show_dat_ac_var_ffp_ass.bin", 0x0000, 0x010000, CRC(7e003d2a) SHA1(f8a6f6810b1733f46e470e89fa821cd51fbe1c5e) )
	ROM_LOAD( "club-game-show_dat_fe_ac_ass.bin", 0x0000, 0x010000, CRC(b5a03c26) SHA1(ef1bc28905a8a9db71299f5c30a15c5576766346) )
	ROM_LOAD( "club-game-show_std_ac_250pnd-24p_p65_ass.bin", 0x0000, 0x010000, CRC(142d828a) SHA1(2fe40e9d641be1cf89cfe9fe5cd4b29dd9ea01e7) )
	ROM_LOAD( "club-game-show_std_ac_var_ass.bin", 0x0000, 0x010000, CRC(01ae9d52) SHA1(3b85a7ebc346d4eb6a16b2b9a03aa12220020aff) )
	ROM_LOAD( "club-game-show_std_ac_var_ffp_ass.bin", 0x0000, 0x010000, CRC(d2819fc3) SHA1(23c7cbf9e04913f5cb62ef6accdd5b470eed3cd4) )
	ROM_LOAD( "club-game-show_std_fe_ac_ass.bin", 0x0000, 0x010000, CRC(6e479cc4) SHA1(99c15b0d1584ab7b460f273de825eb17681c5d0a) )
	ROM_LOAD( "gameshow.bin", 0x0000, 0x010000, CRC(babeb912) SHA1(41bc1cf82bef84f840998af1278c55ea1727a163) )
	ROM_LOAD( "95750844.p1", 0x0000, 0x010000, CRC(36efa743) SHA1(0f5392f55e42d7ac17e179c966997f41859f925a) )

	ROM_REGION( 0x80000, "upd", 0 )
	//ROM_LOAD( "gameshowsnd.bin", 0x0000, 0x080000, CRC(e1a0323f) SHA1(a015d99c882962651869d8ec71a6c17a1cba687f) )
	ROM_LOAD( "95004024.bin", 0x0000, 0x080000, CRC(e1a0323f) SHA1(a015d99c882962651869d8ec71a6c17a1cba687f) )
ROM_END



ROM_START( sc2cpg )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "club-pharaohs-gold_std_ac_250pnd-20p_rot_ass.bin", 0x0000, 0x010000, CRC(f83a68dc) SHA1(1a7aa08835d03116199034378ae0c617520a5ac6) )

	ROM_REGION( 0x10000, "altrevs", 0 )
	ROM_LOAD( "club-pharaohs-gold_dat_ac_250pnd-20p_rot_ass.bin", 0x0000, 0x010000, CRC(2de3b252) SHA1(02c3bfabd5c732e37e71278be5aad0b6b44d28c6) )
	ROM_LOAD( "club-pharaohs-gold_dat_ac_var_ass.bin", 0x0000, 0x010000, CRC(bb790c4b) SHA1(d1126b9848047f15a65119e6446caced2c982287) )
	ROM_LOAD( "club-pharaohs-gold_dat_fe_ac_p65_rot_ass.bin", 0x0000, 0x010000, CRC(4ccba14d) SHA1(a0529a732a1a8c5c9a3d9830072ff1003c80b7d2) )
	ROM_LOAD( "club-pharaohs-gold_std_ac_var_ass.bin", 0x0000, 0x010000, CRC(9376c3c4) SHA1(9e67c982dfb838cde538d0893ea36eafe8bda2d3) )
	ROM_LOAD( "club-pharaohs-gold_std_fe_ac_p65_rot_ass.bin", 0x0000, 0x010000, CRC(e97c5bb4) SHA1(4df5f50bbfe453fbc351855dc6f6a24296563498) )

	ROM_REGION( 0x80000, "upd", 0 )
	ROM_LOAD( "pharaohsgoldsnd.bin", 0x0000, 0x080000, CRC(7d67d53e) SHA1(159e0e9af1cfd6adc141daaa0f75d38af55218c3) )
ROM_END


ROM_START( sc2suprz )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "surprize-surprise_std_ga_20p_ass.bin", 0x0000, 0x010000, CRC(7e52c975) SHA1(a610f7170fda13f64e805e3d99b5f57c61206cfe) )

	ROM_REGION( 0x10000, "altrevs", 0 )
	ROM_LOAD( "surprize-surprise_dat_ac_6pnd-20p_ass.bin", 0x0000, 0x010000, CRC(7e0b263e) SHA1(bcbd82a87e7db65db22e55d9111b0f819a62150a) )
	ROM_LOAD( "surprize-surprise_dat_ga_20p_ass.bin", 0x0000, 0x010000, CRC(8ee54a57) SHA1(471a06d9840ecbf850c8896f8bf45264c0b8390f) )
	ROM_LOAD( "surprize-surprise_dat_var_ass.bin", 0x0000, 0x010000, CRC(37ab423e) SHA1(6b2ab927eb851b8f77eb474a1c5b68c335a17b2f) )
	ROM_LOAD( "surprize-surprise_std_ac_6pnd-20p_ass.bin", 0x0000, 0x010000, CRC(297959d7) SHA1(9bc8bc3d1be1f282573a3ad6994f06ee7bb64dfd) )
	ROM_LOAD( "surprize-surprise_std_var_ass.bin", 0x0000, 0x010000, CRC(5ef85273) SHA1(2ca9e3245c97fbed97a781e135fbb79df5b1bf18) )
	ROM_LOAD( "surprise-surprize-6pound.bin", 0x0000, 0x010000, CRC(d00de4ab) SHA1(cdee9c2c27ab6bad8b0c633ce396fbe2987dbb61) )

	ROM_REGION( 0x80000, "upd", 0 )
	ROM_LOAD( "surprisesurprizesnd.bin", 0x0000, 0x01fedb, CRC(c0981343) SHA1(71278c3446cf204a31415dd2ed8f1de7f7a16645) )
ROM_END


ROM_START( sc2motd )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "match-of-the-day_std_20p_ass.bin", 0x0000, 0x010000, CRC(441931ef) SHA1(9c8c79470dda2a6589d04e4eb8d00d8a984bd1ed) )

	ROM_REGION( 0x10000, "altrevs", 0 )
	ROM_LOAD( "match-of-the-day_dat_20p_uk94_ass.bin", 0x0000, 0x010000, CRC(fa9216fa) SHA1(3d5d164419f022488e60e738958d3f66f4206e87) )
	ROM_LOAD( "match-of-the-day_dat_8pnd-20p_ass.bin", 0x0000, 0x010000, CRC(da77960d) SHA1(e6fc97994612d9280b60df6600c26aa7919381d2) )
	ROM_LOAD( "match-of-the-day_dat_ac_10pnd_tri1_ass.bin", 0x0000, 0x010000, CRC(948b3ede) SHA1(f1c7b4e9fb83ba848d4d8a3ab02a1a5e3b630054) )
	ROM_LOAD( "match-of-the-day_dat_ac_10pnd_uk94_ass.bin", 0x0000, 0x010000, CRC(632325d8) SHA1(92c68b51b4e594bec5d9af43a697a4dd912ed864) )
	ROM_LOAD( "match-of-the-day_dat_ac_8pnd-20p_ass.bin", 0x0000, 0x010000, CRC(86baaf46) SHA1(acb9c5cad4c35621219380a997ae67accaea4206) )
	ROM_LOAD( "match-of-the-day_dat_ar_20p_ass.bin", 0x0000, 0x010000, CRC(ab1c44b9) SHA1(ce34570fabcb2c6ceab48ef7c4367ccafa95ef1a) )
	ROM_LOAD( "match-of-the-day_dat_ar_20p_uk94_ass.bin", 0x0000, 0x010000, CRC(f5adb4aa) SHA1(85afff3251e13808f140d6e58f1c9e2e23ce9d8c) )
	ROM_LOAD( "match-of-the-day_dat_ms_20p_ass.bin", 0x0000, 0x010000, CRC(77710913) SHA1(709fff877ee863021e958bcecbd5cd58a977ea09) )
	ROM_LOAD( "match-of-the-day_dat_ss_20p_ass.bin", 0x0000, 0x010000, CRC(19dafe2d) SHA1(8a7bc4bfb7acd5386fdcadf91c2ba4f5615fa3c9) )
	ROM_LOAD( "match-of-the-day_dat_wi_ac_10pnd_ass.bin", 0x0000, 0x010000, CRC(412a30ed) SHA1(c7118954c086fb1243e441ed7728d801667e98ba) )
	ROM_LOAD( "match-of-the-day_std_8pnd-20p_ass.bin", 0x0000, 0x010000, CRC(8042a61d) SHA1(3e0e75918d6df2d4ed537ee532d1a7fa0bb359b7) )
	ROM_LOAD( "match-of-the-day_std_ac_10pnd_tri1_ass.bin", 0x0000, 0x010000, CRC(10b7a217) SHA1(615bf8e6d1b79c96efd91335a9c6f5db0df95891) )
	ROM_LOAD( "match-of-the-day_std_ac_10pnd_uk94_ass.bin", 0x0000, 0x010000, CRC(f75d128d) SHA1(7da2fb6bc7265848c20cfc137de846439af83b90) )
	ROM_LOAD( "match-of-the-day_std_ac_var_uk94.bin", 0x0000, 0x010000, CRC(ae2330f0) SHA1(d309284f0f0333f6e065f30d7ac9416b2fc4ee1f) )
	ROM_LOAD( "match-of-the-day_std_ar_20p_ass.bin", 0x0000, 0x010000, CRC(27f942a3) SHA1(928d3c2eef6b202c0d71b0843f64aba15aab4f42) )
	ROM_LOAD( "match-of-the-day_std_ar_20p_uk94_ass.bin", 0x0000, 0x010000, CRC(96687a5a) SHA1(dafd7b0af3e26d609b5927c431f4adf2f424322a) )
	ROM_LOAD( "match-of-the-day_std_ss_20p_ass.bin", 0x0000, 0x010000, CRC(ce926573) SHA1(dff243d0eb12d4c13c8334099c5958e897cb8bd5) )
	ROM_LOAD( "match-of-the-day_std_wi_ac_10pnd_ass.bin", 0x0000, 0x010000, CRC(b059fe93) SHA1(33d15c464f3f80f4600d961ddade0b6a661747ba) )
	ROM_LOAD( "motd6ac", 0x0000, 0x010000, CRC(d8e7811c) SHA1(ac67683984465aaf8a96322e71ab7b7bffe92361) )

	ROM_REGION( 0x80000, "upd", 0 )
	ROM_LOAD( "modsndf.bin", 0x0000, 0x080000, CRC(088471f5) SHA1(49fb22daf04450186e9a83aee3312bb85ccf6842) )
ROM_END



ROM_START( sc2easy )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "easy-money_std_ac_var_8-10pnd_ass.bin", 0x0000, 0x010000, CRC(e9f581ca) SHA1(aee8a1af609921a0b33db7b460e4a58517bf9276) )

	ROM_REGION( 0x10000, "altrevs", 0 )
	ROM_LOAD( "easy-money_dat_ac_var_8-10pnd_ass.bin", 0x0000, 0x010000, CRC(e5633ac3) SHA1(d868d782e7d5f6c62ab8958150857336b7acff97) )
	ROM_LOAD( "easy-money_dat_wi_ac_10pnd_tri3_ass.bin", 0x0000, 0x010000, CRC(f841d5cf) SHA1(05afdfa483271635b530652385e2e566920e533d) )
	ROM_LOAD( "easy-money_dat_wi_ac_var_10pnd-20p_ass.bin", 0x0000, 0x010000, CRC(97f62e2d) SHA1(0884ddd0b25e78dd402983158e8c623ff4326cbd) )
	ROM_LOAD( "easy-money_std_wi_ac_10pnd_tri3_ass.bin", 0x0000, 0x010000, CRC(38434925) SHA1(17148ba440c8fd139f7889a211a914ed679a195f) )
	ROM_LOAD( "easy-money_std_wi_ac_var_10pnd-20p_ass.bin", 0x0000, 0x010000, CRC(05622afc) SHA1(169a492870a70aeb17078b2b27c36f5b82274b3f) )

	ROM_REGION( 0x80000, "upd", 0 )
	ROM_LOAD( "easy-money_snd.bin", 0x0000, 0x080000, CRC(56d224c5) SHA1(43b81a1a9a7d30ef7bfb2bbc61e3106faa927778) )
ROM_END



ROM_START( sc2majes )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "majestic.p1", 0x0000, 0x010000, CRC(37289a5f) SHA1(a9d86ed16fc2ff2b83b60e48a1704b4e189c3ac7) )

	ROM_REGION( 0x80000, "upd", 0 )
	ROM_LOAD( "majesticsnd.bin", 0x0000, 0x080000, CRC(3ee3fee3) SHA1(6a5e72e8a808d870a84a0e3523eebfadfab6d5df) )
ROM_END


ROM_START( sc2luvv )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD("95750808.bin", 0x00000, 0x10000, CRC(e6668fc7) SHA1(71dd412114c6386cba72e2b29ea07f2d99d14065))

	ROM_REGION( 0x20000, "matrix", 0 )
	ROM_LOAD("mtx_ass.bin",	 0x00000, 0x10000, CRC(cfdd7bb2) SHA1(90086aaff743a7b2385488af1e8a126029113028))

	ROM_REGION( 0x20000, "altmatrix", 0 )
	ROM_LOAD( "luvvley-jubbley_mat_ass.bin", 0x0000, 0x010000, CRC(e4e06767) SHA1(bee2385c2a9c7ca39ff6a599f827ddba4324b903) )
	ROM_LOAD( "95000575.mtx", 0x0000, 0x0054e8, CRC(d81296df) SHA1(c248cdd5eb59a19fab9098d5bee2c60e9e474fd6) )
	ROM_LOAD( "95000584.mtx", 0x0000, 0x0054d3, CRC(d372b3ef) SHA1(076460d8aaf996d80397da2ebc32e8f1efb63572) )

	ROM_REGION( 0x10000, "altrevs", 0 )
	ROM_LOAD( "luvvley-jubbley_dat_ac_10pnd-20p_ass.bin", 0x0000, 0x010000, CRC(9dee74fc) SHA1(d29756d743b781ab9ce7baf990f4a2cc0e9d7972) )
	ROM_LOAD( "luvvley-jubbley_dat_ac_10pnd-25p_ass.bin", 0x0000, 0x010000, CRC(355210a0) SHA1(c03e1109ee1a419fc4ebdcf861d5220303a9c587) )
	ROM_LOAD( "luvvley-jubbley_dat_ac_4pnd-5p_ass.bin", 0x0000, 0x010000, CRC(4b3155b8) SHA1(aaba2e3d54a2b099b63ee4f5d3560d8eb562c4f1) )
	ROM_LOAD( "luvvley-jubbley_dat_ga_20p_ass.bin", 0x0000, 0x010000, CRC(8c0a6180) SHA1(1c1ee2b5081ee901b5929405a78d3e7a7989916a) )
	ROM_LOAD( "luvvley-jubbley_dat_ms_20p_ass.bin", 0x0000, 0x010000, CRC(886a3a8e) SHA1(4c986e0c7278bd058ce2df2d755cbc8e4f31b3fa) )
	ROM_LOAD( "luvvley-jubbley_std_ac_4pnd-5p_ass.bin", 0x0000, 0x010000, CRC(065ee9bb) SHA1(5d46f0e1b5d48dc94b9843998dedf6d3dfc83e3c) )
	ROM_LOAD( "luvvley-jubbley_std_ms_20p_ass.bin", 0x0000, 0x010000, CRC(d40a59d0) SHA1(7173fc6d349868b9194c4ad581762d299dfb1c69) )
	ROM_LOAD( "luvvley-jubbley_std_ac_10pnd-20p_a.bin", 0x0000, 0x010000, CRC(e4440803) SHA1(be9b49cbe2cfcaa0e640365e190da9c3fcf82bea) )
	ROM_LOAD( "luvvley-jubbley_std_ac_10pnd-20p_ass.bin", 0x0000, 0x010000, CRC(e4440803) SHA1(be9b49cbe2cfcaa0e640365e190da9c3fcf82bea) )

	ROM_REGION( 0x80000, "upd", 0 )
	ROM_LOAD("snd.bin",		 0x00000, 0x80000, CRC(19efac32) SHA1(26f901fc11f052a4d3cff67f8f61dcdd04f3dc22))
ROM_END



ROM_START( sc2ptytm )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "95750806.p1", 0x0000, 0x010000, CRC(4e98c6c6) SHA1(7f4ec51f384b5203229da28f39c3127cd40cf67d) )

	ROM_REGION( 0x10000, "altrevs", 0 )
	ROM_LOAD( "party-time_dat_ac_4pnd-10p_ass.bin", 0x0000, 0x010000, CRC(a33a6d08) SHA1(cf93f42971978b00a15e17d4da6bb6e16e8f1fab) )
	ROM_LOAD( "partytime.bin", 0x0000, 0x010000, CRC(20ef430c) SHA1(b5d35704da425e7ca84500071f34b4d65d87b9fa) )

	ROM_REGION( 0x20000, "matrix", 0 )
	ROM_LOAD( "party-time_mtx_ass.bin", 0x0000, 0x010000, CRC(0672a9f4) SHA1(9e8e01aaa081ffb68aa494fe9dbae0620da0f6b9) )

	ROM_REGION( 0x20000, "altmatrix", 0 )
	ROM_LOAD( "95000585.mtx", 0x0000, 0x004a27, CRC(84682dd9) SHA1(038dd54c071d59f164b39b53c4e0888113489cf1) )
	ROM_LOAD( "partydot.bin", 0x0000, 0x010000, CRC(8a09b858) SHA1(bc932bebc7718da2b97e5f6ef06eb739748353f4) )

	ROM_REGION( 0x80000, "upd", 0 )
	ROM_LOAD( "partysnd1.bin", 0x0000, 0x020000, CRC(b5a5cc9e) SHA1(c9b132ad0d1ce9ff6b56ebde89d5006a5cf7dff6) )
ROM_END



ROM_START( sc2ofool )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "fools & horses 10m 6.bin", 0x0000, 0x010000, CRC(5fe48a02) SHA1(fd5b07a58567e0c5eb75bf1526a853b3a60ddfa9) )

	ROM_REGION( 0x10000, "altrevs", 0 )
	ROM_LOAD( "fools scor ii 10p.bin", 0x0000, 0x010000, CRC(1d6245b7) SHA1(f73b4741cf07d96ec79d907b88d07cd20c748dd3) )
	ROM_LOAD( "fools.bin", 0x0000, 0x010000, CRC(eaa0757a) SHA1(b6bec8f4f443d6c22c18e16ec0d65839fe30b61c) )
	ROM_LOAD( "fools6ac.bin", 0x0000, 0x010000, CRC(5fe48a02) SHA1(fd5b07a58567e0c5eb75bf1526a853b3a60ddfa9) )
	ROM_LOAD( "game 147s only fools.bin", 0x0000, 0x010000, CRC(6cb6cef1) SHA1(bfa40f517b1455e4d563be5964605be63e950e87) )
	ROM_LOAD( "onlyfoolsnhorses_std.bin", 0x0000, 0x010000, CRC(03cc611a) SHA1(e37d6b87017a52f8de339bbd69b2ccbff9872fae) )

	ROM_REGION( 0x20000, "matrix", 0 )
	ROM_LOAD( "onlyfoolsnhorsesdotmatrix.bin", 0x0000, 0x010000, CRC(521611f7) SHA1(08cdc9f7434657151d90fcfd26ce4668477c2998) )

	ROM_REGION( 0x80000, "upd", 0 )
	ROM_LOAD( "onlyfools_snd.bin", 0x0000, 0x080000, CRC(c073bb0c) SHA1(54b3df8c8d814af1fbb662834739a32a693fc7ee) )
ROM_END




ROM_START( sc2town )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "atown20p", 0x0000, 0x010000, CRC(4f7ec25e) SHA1(52af065633942a9e4c195f3294b81ae57bf0c414) )

	ROM_REGION( 0x10000, "altrevs", 0 )
	ROM_LOAD( "round-the-town_dat_ac_10pnd-20p-25p_ass.bin", 0x0000, 0x010000, CRC(8291ad4e) SHA1(cd304052123dfe6d8504a6f5e92413c569bcaf8e) )
	ROM_LOAD( "round-the-town_dat_ar_var_ass.bin", 0x0000, 0x010000, CRC(3d811bb4) SHA1(134e1c65f4f8377eca6d7ccfded5d4600d2949bf) )
	ROM_LOAD( "round-the-town_dat_var_ass.bin", 0x0000, 0x010000, CRC(85110517) SHA1(30eba3987cc60ccbaecbc4c700bb2f1ba088d12f) )
	ROM_LOAD( "round-the-town_std_ac_10pnd-20p-25p_ass.bin", 0x0000, 0x010000, CRC(8394c0e9) SHA1(b9b45e0c855a5f7270259543337fb441694b61e2) )
	ROM_LOAD( "round-the-town_std_ac_20p_20po_ass.bin", 0x0000, 0x010000, CRC(6bc0c2ff) SHA1(9a2bac50978f2b7d2072e0febe4bf4a935bf287d) )
	ROM_LOAD( "round-the-town_std_ar_var_ass.bin", 0x0000, 0x010000, CRC(e5be3a13) SHA1(8a31c67641bce3c2160bb1c651535902374349b4) )
	ROM_LOAD( "round-the-town_std_var_ass.bin", 0x0000, 0x010000, CRC(1909994f) SHA1(47268e1119c808096ddff872e28444ed67bc5dbf) )
	ROM_LOAD( "rtt8ac", 0x0000, 0x010000, CRC(e495e5ea) SHA1(4fb6a43cee1c79ce05b71b35b195f2d35913c40c) )
	ROM_LOAD( "95750069.p1", 0x0000, 0x010000, CRC(6bc0c2ff) SHA1(9a2bac50978f2b7d2072e0febe4bf4a935bf287d) )

	ROM_REGION( 0x20000, "matrix", 0 )
	ROM_LOAD( "round-the-town_mtx.bin", 0x0000, 0x010000, CRC(aa6aac1d) SHA1(57ed376f602dd70495b3bd356bea5113fa8e861e) )
	//ROM_LOAD( "attdot.bin", 0x0000, 0x010000, CRC(aa6aac1d) SHA1(57ed376f602dd70495b3bd356bea5113fa8e861e) )

	ROM_REGION( 0x20000, "altmatrix", 0 )
	ROM_LOAD( "round-the-town_mtx_ass.bin", 0x0000, 0x010000, CRC(1a3b2fb1) SHA1(3d51c6e16558c1ac8ad852a461cd89aef9bc91e4) )
	ROM_LOAD( "95000581.mtx", 0x0000, 0x005c57, CRC(55c55c76) SHA1(3db65ba2acd8cd09f8c12a9135a1d93b71e0838b) )

	ROM_REGION( 0x80000, "upd", 0 )
	ROM_LOAD( "attsnd.bin", 0x0000, 0x040000, CRC(9b5327c8) SHA1(b9e5aeb3e9a6ece796e9164e425829d97c5f3a82) )
ROM_END


ROM_START( sc2cpe )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD("ce1std25p.bin", 0x00000, 0x10000, CRC(2fad9a49) SHA1(5ffb53031eef8778363836143c4e8d2a65361d51))

	ROM_REGION( 0x10000, "altrevs", 0 )
	ROM_LOAD( "95000572.p1", 0x0000, 0x010000, CRC(551ef8ca) SHA1(825f4c3ff56cb2da20ffe1b2ec33f1692f6806b2) )
	ROM_LOAD( "95750273.p1", 0x0000, 0x010000, CRC(950da13c) SHA1(2c544e06112969f7914a5b4fd15e6b0dfedf6b0b) )
	ROM_LOAD( "club-public-enemy-no1_dat_ac_200pnd_ass.bin", 0x0000, 0x010000, CRC(fec925a3) SHA1(5ce3b6f1236f511ae8975c7ecd1549e8d427a245) )
	ROM_LOAD( "club-public-enemy-no1_dat_ac_250pnd-25p_p65_ass.bin", 0x0000, 0x010000, CRC(131375cd) SHA1(4899e8dd4acec9563fa40109bb9b839c5d7209a8) )
	ROM_LOAD( "club-public-enemy-no1_dat_ac_25p_ass.bin", 0x0000, 0x010000, CRC(00bedbdf) SHA1(97b3e23fed6692ae88e6a6110008124422478355) )
	ROM_LOAD( "club-public-enemy-no1_dat_fe_ac_200pnd_p65_rot_ass.bin", 0x0000, 0x010000, CRC(8d5ff953) SHA1(bdf6b5e014c46f6abac792a5913e98cb897b2a73) )
	ROM_LOAD( "club-public-enemy-no1_dat_fe_ac_250pnd-25p_ass.bin", 0x0000, 0x010000, CRC(5a79358b) SHA1(bf728108aad6937be0a5d79fa604f7ac3b191b42) )
	ROM_LOAD( "club-public-enemy-no1_std_ac_200pnd_ass.bin", 0x0000, 0x010000, CRC(5704e52d) SHA1(dfae48734794cea2e9a952d808dedb96fd5204b3) )
	ROM_LOAD( "club-public-enemy-no1_std_ac_250pnd-25p_p65_ass.bin", 0x0000, 0x010000, CRC(2d56a73b) SHA1(31195fa16c1c95d49716448b80f1d0aa973f29d5) )
	ROM_LOAD( "club-public-enemy-no1_std_fe_ac_250pnd-25p_ass.bin", 0x0000, 0x010000, CRC(0a36fd07) SHA1(6338858eb0dd6ba43bfea66afde0d6d1d5097aee) )
	ROM_LOAD( "pe1.bin", 0x0000, 0x010000, CRC(5704e52d) SHA1(dfae48734794cea2e9a952d808dedb96fd5204b3) )


	ROM_REGION( 0x20000, "matrix", 0 )
	ROM_LOAD("cpe1_mtx.bin",  0x00000, 0x10000, CRC(5fd1fd7c) SHA1(7645f8c011be77ac48f4eb2c75c92cc4245fdad4))

	ROM_REGION( 0x20000, "altmatrix", 0 )
	ROM_LOAD( "club-public-enemy-no1_mtx_25pss.hex", 0x0000, 0x01be8c, CRC(e57e66b5) SHA1(f3e44cdb697e6e666bd0008824e802a2cf997aa5) )
	ROM_LOAD( "matrix.bin", 0x0000, 0x010000, CRC(64014f73) SHA1(67d44db91944738fcadc38bfd0d2b7c0536adb9a) )
	ROM_LOAD( "95000572.mtx", 0x0000, 0x008680, CRC(b7f486a0) SHA1(298ae0cf1b256517daa052efd25769230d0ce8a5) )

	ROM_REGION( 0x80000, "upd", 0 )
	ROM_LOAD("cpe1_snd.bin",  0x00000, 0x80000, CRC(ca8a56bb) SHA1(36434dae4369f004fa5b4dd00eb6b1a965be60f9))

	ROM_REGION( 0x80000, "altupd", 0 )
	ROM_LOAD( "pen1c_snd.bin", 0x0000, 0x080000, CRC(57f3d152) SHA1(f5ccd11042d54396352df149e85c4aa271342d49) )
	ROM_LOAD( "95004012.p1", 0x0000, 0x080000, CRC(30d1f22a) SHA1(73cb2d12b090841a12a2ed21653248f41d02e125) )
ROM_END




ROM_START( sc2cops )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "cops & robbers 10 p1 (27512)", 0x0000, 0x010000, CRC(2a74bf68) SHA1(e6d0cf5c26815184d74bc2b1769d13321ce5e33a) )

	ROM_REGION( 0x10000, "altrevs", 0 )
	ROM_LOAD( "casino-cops-and-robbers_dat_ac_var_10pnd_ass.bin", 0x0000, 0x010000, CRC(fadde12b) SHA1(9b041c932558a0132c853514ca3f325f6f97bc65) )
	ROM_LOAD( "casino-cops-and-robbers_dat_ms_to_8pnd_ass.bin", 0x0000, 0x010000, CRC(361ad99f) SHA1(444f2aeef404b087d49e2283bb36bde5e4e673ee) )
	ROM_LOAD( "casino-cops-and-robbers_std_ac_var_10pnd_ass.bin", 0x0000, 0x010000, CRC(549457c2) SHA1(271c7077fd3ee5de67c914faf095b5295dfb6207) )
	ROM_LOAD( "casino-cops-and-robbers_std_ms_to_8pnd_ass.bin", 0x0000, 0x010000, CRC(600a91fd) SHA1(b04bce98df824d2c217c70bd8a49349f93043360) )
	ROM_LOAD( "casino-cops-n-robbers.rom", 0x0000, 0x010000, CRC(54a5168f) SHA1(dfc2bf940ced5a53255238cd9e7d0503e3227691) )
	ROM_LOAD( "cops & robbers 6 25p (27512)", 0x0000, 0x010000, CRC(0ad3fedf) SHA1(25775a80272c72234be9f528cc8f13cf9e1adbf7) )
	ROM_LOAD( "cops-and-robbers_dat_ac_10pnd_ass.bin", 0x0000, 0x010000, CRC(2e3d0614) SHA1(b8be9a1d0be643d0dde7f6d89c067af1e85018bf) )
	ROM_LOAD( "cops-and-robbers_dat_ar_var_ass.bin", 0x0000, 0x010000, CRC(6f544505) SHA1(177a8d4038759dc0e52c14b463aaa6afce81d338) )
	ROM_LOAD( "cops-and-robbers_dat_ss_var_ass.bin", 0x0000, 0x010000, CRC(f14af5f8) SHA1(8bb4d9fc78f1f2c274c4b21c7f4e67c3856f0019) )
	ROM_LOAD( "cops-and-robbers_std_ac_10pnd_a.bin", 0x0000, 0x010000, CRC(2a74bf68) SHA1(e6d0cf5c26815184d74bc2b1769d13321ce5e33a) )
	ROM_LOAD( "cops-and-robbers_std_ac_10pnd_ass.bin", 0x0000, 0x010000, CRC(2a74bf68) SHA1(e6d0cf5c26815184d74bc2b1769d13321ce5e33a) )
	ROM_LOAD( "cops-and-robbers_std_ss_var_ass.bin", 0x0000, 0x010000, CRC(664216d2) SHA1(e222147d71f251554207627b7e5e9de5f10cfff8) )
	ROM_LOAD( "cops1020", 0x0000, 0x010000, CRC(3219a07f) SHA1(1f775189b50eeb55c584dd1054c9119d02b2f738) )
	ROM_LOAD( "cops8ac", 0x0000, 0x010000, CRC(c2ef20ff) SHA1(3841fcaacb739ee90ddc064d42d3275dc6a64016) )
	// are these different HW? (SC1?)
	ROM_LOAD( "cop56cp1", 0x0000, 0x008000, CRC(214edd7d) SHA1(007c17cc522c8f0d30bc1fd08bb18850344f62ad) )
	ROM_LOAD( "cop56cp2", 0x0000, 0x008000, CRC(c862ee34) SHA1(e807d1072953e67581ce0181bfd82a7efcee7bf0) )
	ROM_LOAD( "cops&robbers5pv1-3a(27256)", 0x0000, 0x008000, CRC(29513083) SHA1(f2ce0b573d6756e7d835488b8d8eed3266787255) )
	ROM_LOAD( "cops&robbers5pv1-3b(27256)", 0x0000, 0x008000, CRC(6f5425d6) SHA1(7673841ccfe16eaa0a5cfca1596383f7711f2dbe) )
	ROM_LOAD( "cops & robbers 5p v1-3 a (27256)", 0x0000, 0x008000, CRC(29513083) SHA1(f2ce0b573d6756e7d835488b8d8eed3266787255) )
	ROM_LOAD( "cops & robbers 5p v1-3 b (27256)", 0x0000, 0x008000, CRC(6f5425d6) SHA1(7673841ccfe16eaa0a5cfca1596383f7711f2dbe) )


	ROM_REGION( 0x20000, "matrix", 0 )
	ROM_LOAD( "cops & robbers 10 p2 (27512)", 0x0000, 0x010000, CRC(bdd56a09) SHA1(92d0416578c55075a127f1c2af8d6de5216dd189) )

	ROM_REGION( 0x20000, "altmatrix", 0 )
	ROM_LOAD( "copsdot8", 0x0000, 0x010000, CRC(0eff2127) SHA1(e9788999ac6006faf0eb4e9d8ef1fd52f092be5a) )
	ROM_LOAD( "cops-and-robbers_mtx_a.bin", 0x0000, 0x010000, CRC(bdd56a09) SHA1(92d0416578c55075a127f1c2af8d6de5216dd189) )
	ROM_LOAD( "cops-and-robbers_mtx_ass.bin", 0x0000, 0x010000, CRC(bdd56a09) SHA1(92d0416578c55075a127f1c2af8d6de5216dd189) )
	ROM_LOAD( "copdot10", 0x0000, 0x010000, CRC(30c41ddd) SHA1(9aa66c30aa0fcbd3fb79a6d0d45d777a116f951c) )
	ROM_LOAD( "95000578.mtx", 0x0000, 0x00438f, CRC(8fd08810) SHA1(fbb278629067ed2fb17479f6a9fd439e41809f53) ) // same as bdd56a09 rom, but zipped? check


	ROM_REGION( 0x80000, "upd", 0 )
	ROM_LOAD( "copssnd.bin", 0x0000, 0x040000, CRC(4bebbc37) SHA1(10eb8542a9de35efc0f75b532c94e1b3e0d21e47) )

	ROM_REGION( 0x80000, "altupd", 0 ) // probably just the same but with data repeated, check
	ROM_LOAD( "copsnrobbers.bin", 0x0000, 0x080000, CRC(04ebfc07) SHA1(3c8e9f0e47f3b9b4d787dcd576e11a9b4a71757e) )
ROM_END


ROM_START( sc2copcl )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "club-cops-and-robbers_std_fe_ac_ass.bin", 0x0000, 0x010000, CRC(c7461e95) SHA1(f4088056e848742d3795f5b067476b56071f99bd) )

	ROM_REGION( 0x10000, "altrevs", 0 )
	ROM_LOAD( "club-cops-and-robbers_dat_ac_200pnd-20p_ass.bin", 0x0000, 0x010000, CRC(b4071611) SHA1(2596ccee2b94bb56aa629ee892bd357b706005b0) )
	ROM_LOAD( "club-cops-and-robbers_dat_ac_250pnd-20p_rot_ass.bin", 0x0000, 0x010000, CRC(05635f8b) SHA1(d3cf98e3858189db725621d4ba07728a585d7a3b) )
	ROM_LOAD( "club-cops-and-robbers_dat_ac_250pnd-25p_phx_ass.bin", 0x0000, 0x010000, CRC(f6e9a013) SHA1(02b6c203c3facdd7015ba1119bcb70bf34b4ec00) )
	ROM_LOAD( "club-cops-and-robbers_dat_ac_ffp_ass.bin", 0x0000, 0x010000, CRC(ec92b62d) SHA1(f10bc8fa55cd59127f179a35a61c1a57597856b6) )
	ROM_LOAD( "club-cops-and-robbers_dat_fe_ac_ass.bin", 0x0000, 0x010000, CRC(ead8cbe5) SHA1(5594eb9a736e0f15a6f0f097a8cbbd8352e46fc4) )
	ROM_LOAD( "club-cops-and-robbers_dat_fe_ac_p67_ass.bin", 0x0000, 0x010000, CRC(327db998) SHA1(aa8583cedd52a3cd06be6423a32e48273ec6218a) )
	ROM_LOAD( "club-cops-and-robbers_dat_fr_ac_p63_ass.bin", 0x0000, 0x010000, CRC(93965bfc) SHA1(52af75234f56a77f082132d9532d3ffcaef5d271) )
	ROM_LOAD( "club-cops-and-robbers_std_ac_200pnd-20p_ass.bin", 0x0000, 0x010000, CRC(214cda40) SHA1(fc585f211256495bfaaa6cb6c4d9c8a110ab5051) )
	ROM_LOAD( "club-cops-and-robbers_std_ac_250pnd-20p_ass.bin", 0x0000, 0x010000, CRC(229c65c1) SHA1(8052c4b8702275235545807e7b075571fc97d4f3) )
	ROM_LOAD( "club-cops-and-robbers_std_ac_250pnd-20p_rot_ass.bin", 0x0000, 0x010000, CRC(078651b5) SHA1(2acc45e5d66625753e5869f6f3ac1379d0c9dfcd) )
	ROM_LOAD( "club-cops-and-robbers_std_ac_250pnd-25p_phx_a.bin", 0x0000, 0x010000, CRC(668def2e) SHA1(802ca565a20d0fce2f5e4340c646429af6aadff6) )
	ROM_LOAD( "club-cops-and-robbers_std_ac_250pnd-25p_phx_ass.bin", 0x0000, 0x010000, CRC(668def2e) SHA1(802ca565a20d0fce2f5e4340c646429af6aadff6) )
	ROM_LOAD( "club-cops-and-robbers_std_ac_ffp_ass.bin", 0x0000, 0x010000, CRC(347255bf) SHA1(7f96277579e68bdf1e21788cc5e35941d98df87f) )
	ROM_LOAD( "club-cops-and-robbers_std_ac_npr_ass.bin", 0x0000, 0x010000, CRC(b9c0bcb4) SHA1(c1a398bd58097411b80d36030760e7820dc346f4) )
	ROM_LOAD( "club-cops-and-robbers_std_ac_var_200pnd_ijf_ass.bin", 0x0000, 0x010000, CRC(db5a287e) SHA1(5615480767348061b7f08a709a16aa0b9cf0658e) )
	ROM_LOAD( "club-cops-and-robbers_std_ac_var_p65_ass.bin", 0x0000, 0x010000, CRC(23d80392) SHA1(d7f5bab4fc8f42c1a38e26b54bc519e0f03d20bc) )
	ROM_LOAD( "club-cops-and-robbers_std_fe_ac_p63_ass.bin", 0x0000, 0x010000, CRC(fc7f9b85) SHA1(d9f940bca29919d097fa7d128869725e01d6dbc3) )
	ROM_LOAD( "club-cops-and-robbers_std_fe_ac_p67_ass.bin", 0x0000, 0x010000, CRC(4906d170) SHA1(c304a2986560d675b2e776965fdf444e4d56f104) )
	ROM_LOAD( "cop200.hex", 0x0000, 0x010000, CRC(db5a287e) SHA1(5615480767348061b7f08a709a16aa0b9cf0658e) )
	ROM_LOAD( "cops200", 0x0000, 0x010000, CRC(05d29adc) SHA1(06a986356c1b48ad5ee92c9a7f6fb2531e1806af) )

	ROM_REGION( 0x20000, "matrix", 0 ) // based on the name this would belong with the deluxe set below...
	ROM_LOAD( "club-deluxe-cops-and-robbers_mtx_250pnd-25p.bin", 0x0000, 0x010000, CRC(e1e4c10d) SHA1(5c508fe8ed96191eb1fa7156a09441f2f840544f) )

	ROM_REGION( 0x80000, "upd", 0 )
	ROM_LOAD( "club-cops-and-robbers-sound.bin", 0x0000, 0x040000, CRC(b5ba009d) SHA1(806b1d739fbf00b7e55ed0b8056440e47bfba87a) )
ROM_END

ROM_START( sc2copdc )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "club-deluxe-cops-and-robbers_std_ac_20p_p63_ass.bin", 0x0000, 0x010000, CRC(cb2c995c) SHA1(2a618eb611637e048dc054de0d8f6466f5071617) )

	ROM_REGION( 0x10000, "altrevs", 0 )
	ROM_LOAD( "club-deluxe-cops-and-robbers_dat_ac_20p_p63_ass.bin", 0x0000, 0x010000, CRC(5c97d505) SHA1(6ade77a6dcf1cc57afe879502534f855f6bd4cc8) )
	ROM_LOAD( "club-deluxe-cops-and-robbers_dat_ac_250pnd-20p_ass.bin", 0x0000, 0x010000, CRC(c5f6c4f6) SHA1(69be1c6f134406a5457cf4bd7ed78dc4524bac6d) )
	ROM_LOAD( "club-deluxe-cops-and-robbers_dat_ac_250pnd-25p_p65_ass.bin", 0x0000, 0x010000, CRC(f2433167) SHA1(88c90c047f67361e1974ea29a887f11c79c78b55) )
	ROM_LOAD( "club-deluxe-cops-and-robbers_dat_ac_250pnd-25p_p67_ass.bin", 0x0000, 0x010000, CRC(734c5e16) SHA1(e6a6a31ef5156e207dd77c40f5b29b10ef4f9def) )
	ROM_LOAD( "club-deluxe-cops-and-robbers_dat_ac_250pnd_ass.bin", 0x0000, 0x010000, CRC(6b899a10) SHA1(58b7e2e9eda0d3715de8a4af31b49e059942b6f2) )
	ROM_LOAD( "club-deluxe-cops-and-robbers_dat_ac_var_200pnd_ass.bin", 0x0000, 0x010000, CRC(a914cb23) SHA1(cd3332506229184cf0c3db37c43d2fa4cd2e54d9) )
	ROM_LOAD( "club-deluxe-cops-and-robbers_std_ac_250pnd-20p_ass.bin", 0x0000, 0x010000, CRC(4e7da1cb) SHA1(1c61f47f30a9d27f558548c23ddf6de2e5366344) )
	ROM_LOAD( "club-deluxe-cops-and-robbers_std_ac_250pnd-25p_p65_ass.bin", 0x0000, 0x010000, CRC(8f5396a6) SHA1(c7cd83bdeca3a852a8203330ca14574608b9a9e9) )
	ROM_LOAD( "club-deluxe-cops-and-robbers_std_ac_250pnd-25p_p67_ass.bin", 0x0000, 0x010000, CRC(fd19db9a) SHA1(441d80b8463ffd5f8783b3cb80d8321f64e8fcc5) )
	ROM_LOAD( "club-deluxe-cops-and-robbers_std_ac_250pnd_ass.bin", 0x0000, 0x010000, CRC(10a9d7d3) SHA1(7d147ce9c2c98f10694ee99e14286be3f74bbdf4) )
	ROM_LOAD( "club-deluxe-cops-and-robbers_std_ac_var_200pnd_ass.bin", 0x0000, 0x010000, CRC(23d239fa) SHA1(44dae2cd2be573df71b60ba3918cc2d728cde4b4) )
	ROM_LOAD( "clubcopsnrobbersdeluxe.bin", 0x0000, 0x010000, CRC(055e0f2c) SHA1(8aa7386031fd381deb7d79ce3217bab0d01671f0) )

	ROM_REGION( 0x20000, "matrix", 0 )
	ROM_LOAD( "club-deluxe-cops-and-robbers_mtx_250pnd-25pss.hex", 0x0000, 0x01cfbf, CRC(b2abbab4) SHA1(40e202e1678f637f7c0097b4f8f4884de439935e) )

	ROM_REGION( 0x80000, "upd", ROMREGION_ERASE00 )
ROM_END

ROM_START( sc2dels )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "del's-millions_std_ac_10pnd-20p-25p_a.bin", 0x0000, 0x010000, CRC(b1e8d4ef) SHA1(189184aa6f9ff2204e35d0f7ae40493bcb0751bd) )

	ROM_REGION( 0x10000, "altrevs", 0 )
	ROM_LOAD( "95751541.p1", 0x0000, 0x010000, CRC(495b7cec) SHA1(779a80371580b9154f0915e7c438dbf965dd1a02) )
	ROM_LOAD( "del's-millions_dat_ac_10pnd-20p-25p_a.bin", 0x0000, 0x010000, CRC(c81f200f) SHA1(8a9ee842e17a63276a0850adc52159dc46a239c0) )
	ROM_LOAD( "del's-millions_dat_ac_8pnd-20p_a.bin", 0x0000, 0x010000, CRC(92c0e403) SHA1(5410365137ab8debb10358f24cdd0b0b74755677) )
	ROM_LOAD( "del's-millions_dat_ac_8pnd_a.bin", 0x0000, 0x010000, CRC(23eca216) SHA1(f427d92929e51d6f0148d212e13067ddc15e2307) )
	ROM_LOAD( "del's-millions_dat_ms_20p_a.bin", 0x0000, 0x010000, CRC(57ade491) SHA1(3aed99d92c391f99fa8ff7d61370d59245156121) )
	ROM_LOAD( "del's-millions_dat_wi_ac_10pnd-20p_a.bin", 0x0000, 0x010000, CRC(fdb33c9b) SHA1(2506fe8e7e1e49f90652309996813ac5967442a0) )
	ROM_LOAD( "del's-millions_std_ac_8pnd-20p_a.bin", 0x0000, 0x010000, CRC(9194fb69) SHA1(30d2c5a8a16c96c081f442a66172f8b9fb1d602d) )
	ROM_LOAD( "del's-millions_std_ac_8pnd_a.bin", 0x0000, 0x010000, CRC(58f87c90) SHA1(a6dcdf1edc7620226d89c907a5910c4a4b2d4190) )
	ROM_LOAD( "del's-millions_std_ms_20p_ass.bin", 0x0000, 0x010000, CRC(f4a5803d) SHA1(c9b6f71847a4dd87ea34b51935618df5a735150d) )
	ROM_LOAD( "del's-millions_std_ss_20p_a.bin", 0x0000, 0x010000, CRC(755b8546) SHA1(67d2bb5556c03acf71e0b50c8cf54ac92acbce69) )
	ROM_LOAD( "del's-millions_std_wi_ac_10pnd-20p_a.bin", 0x0000, 0x010000, CRC(dd44aecb) SHA1(1e8ced54323580f43facf683c1f489f1ea281e16) )
	ROM_LOAD( "delm20p", 0x0000, 0x010000, CRC(9d8acc21) SHA1(04d9cb4d01ddfb4e33774b313446dcd763f869fa) )
	ROM_LOAD( "dels millions ck 8f98 std 8.bin", 0x0000, 0x010000, CRC(755b8546) SHA1(67d2bb5556c03acf71e0b50c8cf54ac92acbce69) )
	ROM_LOAD( "dels10", 0x0000, 0x010000, CRC(8bf1b9f5) SHA1(eb9c36579d56f83d72952fab9911a991aeec0579) )
	ROM_LOAD( "dels8mss", 0x0000, 0x002000, CRC(a91764fc) SHA1(3196cfbe04af74ea330a23a1155a6e223cb670bb) ) // bad dump?
	ROM_LOAD( "delsdlx6", 0x0000, 0x010000, CRC(64acb285) SHA1(7a011b915809712fd69902258f1e6c9b42f163eb) )
	ROM_LOAD( "delsmillions.bin", 0x0000, 0x010000, CRC(58f87c90) SHA1(a6dcdf1edc7620226d89c907a5910c4a4b2d4190) )
	ROM_LOAD( "dem20arc", 0x0000, 0x010000, CRC(9ae6291d) SHA1(966416d234e2ec708984595dedbfbe554ff1c867) )
	// sets below are mazooma
	ROM_LOAD( "98400006", 0x0000, 0x010000, CRC(2dc3355c) SHA1(6db6ddc93e05516b75d0dd27d5ab190d183a2bd1) )
	ROM_LOAD( "98400007", 0x0000, 0x010000, CRC(f29b0110) SHA1(b2a56e68a2bb4f4cc5b0f32933bf9e9acb0582d2) )
	ROM_LOAD( "98400008", 0x0000, 0x010000, CRC(38a0159b) SHA1(2f25ae4d858f68750a627d298556a7ce461480e5) )
	ROM_LOAD( "98401005", 0x0000, 0x010000, CRC(d91beaa2) SHA1(b018d335e8551efe4cc09381324d7ae3d77b2907) )
	ROM_LOAD( "98401006", 0x0000, 0x010000, CRC(262d57f9) SHA1(157bfa2d9de8da9f7791295b1e476bf2329f55cd) )
	ROM_LOAD( "98401007", 0x0000, 0x010000, CRC(013c5e7c) SHA1(f3e960b44faecc7d19c6e058b62a30e45c3cfeae) )
	ROM_LOAD( "98401008", 0x0000, 0x010000, CRC(665b3af4) SHA1(a7d51976caa8c373ac772e1315a33f0f042974a6) )
	ROM_LOAD( "98400005", 0x0000, 0x010000, CRC(bd9153cf) SHA1(695a897077b2136ba4d0699cad616df5ceadf824) )

	ROM_REGION( 0x80000, "upd", 0 )
	ROM_LOAD( "dmsnd.bin", 0x0000, 0x080000, CRC(0a68550b) SHA1(82a4a8d2a754a59da553b3568df870107e33f978) )

	ROM_REGION( 0x80000, "altupd", 0 )
	ROM_LOAD( "delssnd.bin", 0x0000, 0x080000, CRC(cb298f06) SHA1(fdc857101ad15d58aeb7ffc4a489c3de9373fc80) )
ROM_END



ROM_START( sc2wembl )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "road-to-wembley_std_ac_10pnd_15rm_ass.bin", 0x0000, 0x010000, CRC(7b8e7a47) SHA1(3026850a18ef9cb44584550e28f62165bfa690e9) )

	ROM_REGION( 0x10000, "altrevs", 0 )
	ROM_LOAD( "95750499.p1", 0x0000, 0x010000, CRC(a2b11ca6) SHA1(cc1931504f8da98119f771499db616898d92e0d9) )
	ROM_LOAD( "95750500.p1", 0x0000, 0x010000, CRC(bfe45926) SHA1(6a2814735e0894bb5152cba8f90d98cfa98c250b) )
	ROM_LOAD( "95750501.p1", 0x0000, 0x010000, CRC(cab3da07) SHA1(8ef7ed8427cbb213f218328666da3ebd92aca5a5) )
	ROM_LOAD( "road-to-wembley_dat_20p_15rm_ass.bin", 0x0000, 0x010000, CRC(45c3df4c) SHA1(48ef0e46a94a815e1e429f402cc8fd13bde4d738) )
	ROM_LOAD( "road-to-wembley_dat_ac_10pnd_15rm_ass.bin", 0x0000, 0x010000, CRC(6ab89e2f) SHA1(6b2faa587153f453e9fdf043c6ca5a90d8c6b66d) )
	ROM_LOAD( "road-to-wembley_dat_ac_10pnd_ass.bin", 0x0000, 0x010000, CRC(bf15d498) SHA1(f94d21d1202107db7955829340ada445d59f74ff) )
	ROM_LOAD( "road-to-wembley_dat_ac_8pnd_16rm_ass.bin", 0x0000, 0x010000, CRC(512fafcb) SHA1(fe90c7fc58bd3dc0bc84e060c6b7a37dd855733b) )
	ROM_LOAD( "road-to-wembley_dat_ar_20p_16rm_ass.bin", 0x0000, 0x010000, CRC(550f82ec) SHA1(80b1d0839f600b01f2a60de0e191add0faaad089) )
	ROM_LOAD( "road-to-wembley_dat_ss_10p_ass.bin", 0x0000, 0x010000, CRC(630b5306) SHA1(aa23645cc7f1c86e88a62420a837ab64c5090d09) )
	ROM_LOAD( "road-to-wembley_dat_ss_20p_16rm_ass.bin", 0x0000, 0x010000, CRC(55b1764a) SHA1(1b1e5b89eda0d07662af003d1259e0da725abbc9) )
	ROM_LOAD( "road-to-wembley_std_20p_15rm_ass.bin", 0x0000, 0x010000, CRC(065f2f8b) SHA1(81471db8de879b7d5b8741beefa5214f2c48ef84) )
	ROM_LOAD( "road-to-wembley_std_ac_10pnd_ass.bin", 0x0000, 0x010000, CRC(ae2330f0) SHA1(d309284f0f0333f6e065f30d7ac9416b2fc4ee1f) )
	ROM_LOAD( "road-to-wembley_std_ss_20p_16rm_ass.bin", 0x0000, 0x010000, CRC(17cd6162) SHA1(80129b26db4617281bb6e5aa1f573cf222660303) )
	ROM_LOAD( "rtw816rm", 0x0000, 0x010000, CRC(337264ae) SHA1(5e3e67bd20416331df6e35c6a384d5b88b70aa17) )
	ROM_LOAD( "rtwn8arc.bin", 0x0000, 0x010000, CRC(b054b38e) SHA1(98aa68a4fb6db4a53a63a4976954277c082ee8bf) )

	ROM_REGION( 0x80000, "upd", ROMREGION_ERASE00 )
	ROM_LOAD( "wembley_sound.bin", 0x0000, 0x080000, CRC(5ce2fc50) SHA1(26533428582058f0cd618e3657f967bc64e551fc) )
ROM_END

ROM_START( sc2wemblm )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "98400014", 0x0000, 0x010000, CRC(e4f3e02d) SHA1(ce2b961e6142ecfb1532daaa53746d785e2342eb) )

	ROM_REGION( 0x80000, "upd", ROMREGION_ERASE00 )
	ROM_LOAD( "wembley_sound.bin", 0x0000, 0x080000, CRC(5ce2fc50) SHA1(26533428582058f0cd618e3657f967bc64e551fc) )
ROM_END







ROM_START( sc2prem )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "premier-club-manager_std_ac_250pnd-25p_ass.bin", 0x0000, 0x010000, CRC(404716ed) SHA1(57916fb70621c96eccb0e5bbee821ca2133aaa5f) )

	ROM_REGION( 0x10000, "altrevs", 0 )
	ROM_LOAD( "premclub.bin", 0x0000, 0x010000, CRC(5231ab3e) SHA1(a9e16a5bbeaa0612212d3ef0e78fbc7628cfc0fa) )
	ROM_LOAD( "premier-club-manager_dat_ac_250pnd-25p_ass.bin", 0x0000, 0x010000, CRC(6446176c) SHA1(17cccc00d443ffde11943ebda112ef1e79134455) )
	ROM_LOAD( "premier-club-manager_dat_ac_var_ass.bin", 0x0000, 0x010000, CRC(d1880c7a) SHA1(d1f7891fc8d4570e02c0bfc23e1ed0b159e280c1) )
	ROM_LOAD( "premier-club-manager_std_ac_var_ass.bin", 0x0000, 0x010000, CRC(68e5474e) SHA1(927d41f73e287c71546823ffe829f1e046f3cca6) )

	ROM_REGION( 0x20000, "matrix", 0 )
	ROM_LOAD( "premier-club-manager_mtx_250pnd-25p_ass.bin", 0x0000, 0x010000, CRC(4b4bdb8b) SHA1(de9b52da600629e680fd96f0d82a9f76fbc84bdf) )

	ROM_REGION( 0x20000, "altmatrix", 0 )
	ROM_LOAD( "premier-club-manager_mtx_ass.bin", 0x0000, 0x010000, CRC(7ac2a278) SHA1(f95a7451d1514be19d747707a32bf7280dcfb8b6) )
	ROM_LOAD( "95000570.mtx", 0x0000, 0x004e21, CRC(1b38ddeb) SHA1(86795dcb67306eccabbf0d2a214667497104ef77) )
	ROM_LOAD( "95000571.mtx", 0x0000, 0x004ddc, CRC(0772adea) SHA1(6d3beb1662fd4e1eeef0ca57cdc07f347879bf15) )

	ROM_REGION( 0x80000, "upd", ROMREGION_ERASE00 )
	ROM_LOAD( "premclubsnd.bin", 0x0000, 0x080000, CRC(b20c74f1) SHA1(b43a79f8f59387ef777fffd07a39b7333811d464) )
ROM_END



ROM_START( sc2downt )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "down-town_std_ar_20p_15rm_ass.bin", 0x0000, 0x010000, CRC(bffe2f17) SHA1(c9daeec2b715d318649c8883b4437fdd997d0dc8) )

	ROM_REGION( 0x10000, "altrevs", 0 )
	ROM_LOAD( "down-town_dat_20p_15rm_ass.bin", 0x0000, 0x010000, CRC(3390da28) SHA1(80abda7a0d6913b701fb030b525db794d130df5b) )
	ROM_LOAD( "down-town_dat_ac_10pnd-20p_15rm_ass.bin", 0x0000, 0x010000, CRC(b082210f) SHA1(cd8d18fc2dcaf6fc02bc05d4c9e4a76f2199ad8d) )
	ROM_LOAD( "down-town_dat_ac_10pnd-20p_16rm_ass.bin", 0x0000, 0x010000, CRC(d6d95ff4) SHA1(55d2b97a0609e305d28c92f439eb3b834d29aff5) )
	ROM_LOAD( "down-town_dat_ar_20p_15rm_ass.bin", 0x0000, 0x010000, CRC(39fc9af0) SHA1(3b3a2a2ada79fa822332c066d50d81e64860292b) )
	ROM_LOAD( "down-town_dat_ar_20p_ass.bin", 0x0000, 0x010000, CRC(a84c92c7) SHA1(99519d3e6166ab80236f1c16be82f7b2648f0aff) )
	ROM_LOAD( "down-town_dat_ss_20p_16rm_ass.bin", 0x0000, 0x010000, CRC(39a1cd5d) SHA1(bce1e1bfe4e9e3bc62bdf8a57b0b2db2b3accd4f) )
	ROM_LOAD( "down-town_dat_wi_ac_10pnd_15rm_ass.bin", 0x0000, 0x010000, CRC(29a1a709) SHA1(6b2de1e7902ba5b678aebf04b0f8c3bceed8f637) )
	ROM_LOAD( "down-town_dat_wi_ac_10pnd_16rm_ass.bin", 0x0000, 0x010000, CRC(6f6f8c71) SHA1(5fba18cc092a04b3b737bb17a03d5e37a33da985) )
	ROM_LOAD( "down-town_std_20p_15rm_ass.bin", 0x0000, 0x010000, CRC(ef4c489f) SHA1(3b4e0c811edcb4f1f9c133ce92b7d965e167e51c) )
	ROM_LOAD( "down-town_std_ac_10pnd-20p_15rm_ass.bin", 0x0000, 0x010000, CRC(7ef9d60d) SHA1(54000f31eac051efd2fd3fe485076f845ef3da30) )
	ROM_LOAD( "down-town_std_ac_10pnd-20p_16rm_ass.bin", 0x0000, 0x010000, CRC(932e49d9) SHA1(05ae4751f55eefe9884444745bcf3f2ecb69e332) )
	ROM_LOAD( "down-town_std_ar_20p_ass.bin", 0x0000, 0x010000, CRC(a162c04a) SHA1(516f754b2e9cc33d43bac37f1f0697c1a886027e) )
	ROM_LOAD( "down-town_std_ss_20p_16rm_ass.bin", 0x0000, 0x010000, CRC(593f59a5) SHA1(578173ec26980072a00bb46370c2c1113916c279) )
	ROM_LOAD( "down-town_std_wi_ac_10pnd_15rm_ass.bin", 0x0000, 0x010000, CRC(bb448916) SHA1(ed62858cb78c9f08a55679cfdb19a3fa951d1aed) )
	ROM_LOAD( "down-town_std_wi_ac_10pnd_16rm_ass.bin", 0x0000, 0x010000, CRC(96ddfacd) SHA1(9085cdafc7b3ddf5ef77251a9ff4d4b4beff4ff1) )
	ROM_LOAD( "dtown8c.bin", 0x0000, 0x010000, CRC(6b93171c) SHA1(90e01e827b473bb6ffb567a350d9d8de9119cf8d) )
	ROM_LOAD( "dtwn20v", 0x0000, 0x010000, CRC(5e6f05e4) SHA1(78ba0636aca6d6f5d8aee0f27c337975c5680e98) )
	ROM_LOAD( "dtwnac", 0x0000, 0x010000, CRC(f553e337) SHA1(1881912807e4d245b8f2455ca8ca6d0c158ac5a8) )

	ROM_REGION( 0x80000, "upd", ROMREGION_ERASE00 )
	ROM_LOAD( "dtownsnd.dat", 0x0000, 0x080000, CRC(a41b109b) SHA1(22470d731741521321d004fc56ff8217e506ef69) )
ROM_END

ROM_START( sc2goldr )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "gold_reserve_game", 0x0000, 0x010000, CRC(581726a3) SHA1(7e122a9d48f49648feeeb3fe430013402a5dc8d7) )

	ROM_REGION( 0x10000, "altrevs", 0 )
	ROM_LOAD( "gr1_3.bin", 0x0000, 0x010000, CRC(caed7c10) SHA1(3ea4b786d7574a3274131554885a372283eb1cf4) )
	ROM_LOAD( "gr1_3d.bin", 0x0000, 0x010000, CRC(e5ad5d10) SHA1(8a2bf68b923848421b90af8a1c42f5cef1a02121) )

	ROM_REGION( 0x80000, "upd", ROMREGION_ERASE00 )
	ROM_LOAD( "gold_reserve_snd", 0x0000, 0x080000, CRC(e8e7ab7b) SHA1(ce43e8ffccc0421548c6683a72267b7e5f805db4) )
ROM_END

ROM_START( sc2hifly )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "hiflyergame.bin", 0x0000, 0x010000, CRC(b3627b55) SHA1(105ff7da69eb2ca722ee251a4a6af49c46ab1bc8) )

	ROM_REGION( 0x10000, "altrevs", 0 )
	ROM_LOAD( "hf2_1dat", 0x0000, 0x010000, CRC(6c1350eb) SHA1(062e4533c28c8129aae787805bdf99a2837f93f5) )
	ROM_LOAD( "hf3_1.bin", 0x0000, 0x010000, CRC(0ec80578) SHA1(8bbe5aaefe7c5ab77e27daad3fe43d7bbe600a54) )
	ROM_LOAD( "hf4_1.bin", 0x0000, 0x010000, CRC(ee58ed3b) SHA1(4372ca48854b5a4b2c9ac24b17afce899a88da15) )

	ROM_REGION( 0x80000, "upd", ROMREGION_ERASE00 )
	ROM_LOAD( "hiflyersound.bin", 0x0000, 0x080000, CRC(acdef7dc) SHA1(c2cc219ca8f4a3e3cdcb1147ad49cd69adb3751b) )
ROM_END


ROM_START( sc2inst )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "instant-jackpot_std_ac_var_10pnd_ass.bin", 0x0000, 0x010000, CRC(81a235e9) SHA1(3ed26da7511b2b2324d74f8395215157c41850ce) )

	ROM_REGION( 0x10000, "altrevs", 0 )
	ROM_LOAD( "instant-jackpot_dat_ac_var_10pnd_ass.bin", 0x0000, 0x010000, CRC(01034a5b) SHA1(c4f7b05d5c15c309d0c13f4bef72429e54e4fd5e) )
	ROM_LOAD( "instant-jackpot_dat_var_8pnd_ass.bin", 0x0000, 0x010000, CRC(5ded0d95) SHA1(6f1f57e6883f4b0421ca4d49f7593a937918f9e4) )
	ROM_LOAD( "instant-jackpot_dat_var_ass.bin", 0x0000, 0x010000, CRC(26f50252) SHA1(587ca9490e04247c8b93c4c931caadf0b5aea4b3) )
	ROM_LOAD( "instant-jackpot_dat_var_to_htpa_ass.bin", 0x0000, 0x010000, CRC(0dcd87a1) SHA1(4d53a346665bf22e467cc0e0859ee44c177b7661) )
	ROM_LOAD( "instant-jackpot_dat_wi_ac_10pnd_ass.bin", 0x0000, 0x010000, CRC(6d289dfa) SHA1(a1245373ad5a99e2794751dd8e4d3ea28dcb0a53) )
	ROM_LOAD( "instant-jackpot_std_var_8pnd_ass.bin", 0x0000, 0x010000, CRC(f21c8970) SHA1(67ecb5202cc4a8f2568df6c0a4ed36f4c85b8bb4) )
	ROM_LOAD( "instant-jackpot_std_var_ass.bin", 0x0000, 0x010000, CRC(ca8ab34a) SHA1(ecf5ccf0f95a8d149326d24ac468660dde073a16) )
	ROM_LOAD( "instant-jackpot_std_var_to_htpa_ass.bin", 0x0000, 0x010000, CRC(1566696f) SHA1(c8cda3f1d15bcb8ba67fab8cb4b972c02106eceb) )
	ROM_LOAD( "instant-jackpot_std_wi_ac_10pnd_ass.bin", 0x0000, 0x010000, CRC(641928a0) SHA1(8d68af148838987a4ebfd7927b8eda5cfa4bbb53) )
	ROM_LOAD( "instantjackpotgame.bin", 0x0000, 0x010000, CRC(183d53bf) SHA1(4ceca64324a95580270b66d60e678996c79db965) )
	ROM_LOAD( "instantjackpotgame21.bin", 0x0000, 0x010000, CRC(478a4ee9) SHA1(bb33c63d3db961dc14a02f9ab69908757b8ccd87) )

	ROM_REGION( 0x80000, "upd", ROMREGION_ERASE00 )
	ROM_LOAD( "instantjackpotssnd.bin", 0x0000, 0x080000, CRC(ba922860) SHA1(7d84c7fa72b1fb567faccf8464e0fd859c76838d) )
ROM_END


ROM_START( sc2mam )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "make-a-million_std_ac_10pnd-20p-25p_ass.bin", 0x0000, 0x010000, CRC(33fce86f) SHA1(1fa06c834397f97e3723091eb331adab91e3d720) )

	ROM_REGION( 0x10000, "altrevs", 0 )
	ROM_LOAD( "make-a-million_dat_ac_10pnd-25p-20p_ass.bin", 0x0000, 0x010000, CRC(b721a965) SHA1(23c8f3e98b7a2d7aa11593bff2caea26c893a98a) )
	ROM_LOAD( "make-a-million_dat_ac_var_8pnd_ass.bin", 0x0000, 0x010000, CRC(722420ea) SHA1(7c3a8a7218770645f5644a68c65b8e2104857367) )
	ROM_LOAD( "make-a-million_dat_ar_var_ass.bin", 0x0000, 0x010000, CRC(f9307781) SHA1(56bef9b7d4db0d4569a855dba49d931125f038a4) )
	ROM_LOAD( "make-a-million_dat_ss_var_ass.bin", 0x0000, 0x010000, CRC(be526b6b) SHA1(e3e6eb91480015edc3ef46158a277c90d1bf5662) )
	ROM_LOAD( "make-a-million_dat_wi_ac_10pnd-20p_ass.bin", 0x0000, 0x010000, CRC(a63f1ae3) SHA1(37920ade2a162f6663a8384ff3cf55e1de71d3d6) )
	ROM_LOAD( "make-a-million_std_ac_var_8pnd_a.bin", 0x0000, 0x010000, CRC(9150bd44) SHA1(0ef8884337c188c696a15cf2bc5a821bdc64d8ae) )
	ROM_LOAD( "make-a-million_std_ac_var_8pnd_ass.bin", 0x0000, 0x010000, CRC(9150bd44) SHA1(0ef8884337c188c696a15cf2bc5a821bdc64d8ae) )
	ROM_LOAD( "make-a-million_std_ar_var_ass.bin", 0x0000, 0x010000, CRC(06759280) SHA1(168743d4d116850c3c23db3cd0149c7f5f8b4da3) )
	ROM_LOAD( "make-a-million_std_ss_var_ass.bin", 0x0000, 0x010000, CRC(4de6346a) SHA1(ae30a5adfad59dd282ca3c2e16e18cbd17d956e9) )
	ROM_LOAD( "make-a-million_std_wi_ac_10pnd-20p_ass.bin", 0x0000, 0x010000, CRC(d2dceb05) SHA1(f4dd4f0ce3aa97caba0356a19fe78e3c3455af54) )
	ROM_LOAD( "mam8arc.bin", 0x0000, 0x010000, CRC(91ee99ca) SHA1(8e7e26e0ab518e55784b91b5d8c9780eb1f72525) )


	ROM_REGION( 0x80000, "upd", ROMREGION_ERASE00 )
	// ROM_LOAD( "makeamillionsnd1.bin", 0x0000, 0x080000, CRC(9a80977a) SHA1(0a6dc9465efa9e3d12894daf88a2746e74409349) ) // this is in the club set too, belongs there instead?
	ROM_LOAD( "mamsnd.bin", 0x0000, 0x080000, CRC(32537b18) SHA1(c26697162edde97ec999ed0459656edb85a01a50) )
ROM_END

ROM_START( sc2mamcl )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "club-make-a-million_std_ac_250pnd-20p_ass.bin", 0x0000, 0x010000, CRC(f7b67981) SHA1(ccddb63cd24969fb74a3e4c51c8ab7453b3e99a1) )

	ROM_REGION( 0x10000, "altrevs", 0 )
	ROM_LOAD( "club-make-a-million_dat_ac_250pnd-20p_ass.bin", 0x0000, 0x010000, CRC(4a6a6e05) SHA1(684bb86de514e66409cc04255d4212569ad5f2e6) )
	ROM_LOAD( "club-make-a-million_dat_ac_var_ass.bin", 0x0000, 0x010000, CRC(484ea479) SHA1(c1542dcd664508e4ebea3b66b9961680b7f4d711) )
	ROM_LOAD( "club-make-a-million_dat_ac_var_p65_ass.bin", 0x0000, 0x010000, CRC(ee687364) SHA1(a414c71659a81fc464bc167c05e9426a37d33f82) )
	ROM_LOAD( "club-make-a-million_std_ac_var_ass.bin", 0x0000, 0x010000, CRC(c0685075) SHA1(4906d1e81e7d9b43e6c147ebc72081634dd7cd45) )
	ROM_LOAD( "club-make-a-million_std_ac_var_p65_ass.bin", 0x0000, 0x010000, CRC(296b5724) SHA1(437d789313960db9e4da147353da81d3e162e563) )
	ROM_LOAD( "mmilclub", 0x0000, 0x010000, CRC(c3c6856a) SHA1(6163bfcf4271bef2517bdf16b526a882574c0bf1) )

	ROM_REGION( 0x80000, "upd", ROMREGION_ERASE00 )
	ROM_LOAD( "cmamsnd.bin", 0x0000, 0x080000, CRC(9a80977a) SHA1(0a6dc9465efa9e3d12894daf88a2746e74409349) )
ROM_END

ROM_START( sc2scc )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "sc4_4t.bin", 0x0000, 0x010000, CRC(99235ed7) SHA1(f2d851ce1abe6c1dc4ab1ce3aea067c6434ef6ee) )

	ROM_REGION( 0x80000, "upd", ROMREGION_ERASE00 )
	ROM_LOAD( "scsnd", 0x0000, 0x040000, CRC(5f201e1a) SHA1(cc67bcd3a59681b7eb535c966a1e100a17ca1acc) )
ROM_END


ROM_START( sc2showt )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "showtime-spectacular_std_ac_8-10pnd-20p_ass.bin", 0x0000, 0x010000, CRC(44176459) SHA1(e7321fb659be162507f095e3b586706837892c2d) )

	ROM_REGION( 0x10000, "altrevs", 0 )
	ROM_LOAD( "showtime-spectacular_dat_ac_8-10pnd20p_ass.bin", 0x0000, 0x010000, CRC(98111157) SHA1(ddc0e194d330348ce133467324155787f98bf8fd) )
	ROM_LOAD( "showtime-spectacular_dat_ac_var_8pnd_ass.bin", 0x0000, 0x010000, CRC(99bddd9c) SHA1(256b11ffc0415c21ad20d7192cf5bb67dca38a54) )
	ROM_LOAD( "showtime-spectacular_dat_ar_var_ass.bin", 0x0000, 0x010000, CRC(f70d696e) SHA1(5ddaa1323586dd7de87ee18f666c632a149b8c6c) )
	ROM_LOAD( "showtime-spectacular_dat_ss_var_ass.bin", 0x0000, 0x010000, CRC(180984c3) SHA1(789cf4e7d99ad25d21ea02ec4de39f30fb6e7474) )
	ROM_LOAD( "showtime-spectacular_dat_wi_ac_8-10pnd_ass.bin", 0x0000, 0x010000, CRC(9233e7e2) SHA1(fafc9fe99fb3b04e494302e2e1c566e611c1cd54) )
	ROM_LOAD( "showtime-spectacular_std_ac_var_8pnd_ass.bin", 0x0000, 0x010000, CRC(c6760a9b) SHA1(bf85edd0a0d10da04b1a3608fa2f2f3c5d4ed7ec) )
	ROM_LOAD( "showtime-spectacular_std_ar_var_ass.bin", 0x0000, 0x010000, CRC(b2a8470c) SHA1(43eecd76e6a028595ee91a7be92490bda9d8eef0) )
	ROM_LOAD( "showtime-spectacular_std_ss_var_ass.bin", 0x0000, 0x010000, CRC(a42d951d) SHA1(e6c0491e69195043f0f228b80ded6c84116b8ddc) )
	ROM_LOAD( "showtime-spectacular_std_wi_ac_8-10.pnd_ass.bin", 0x0000, 0x010000, CRC(d4867696) SHA1(7d8d9eed052ab6a84c52136bb604b91987f6120e) )
	ROM_LOAD( "stspecss", 0x0000, 0x002000, CRC(e7f3c2ad) SHA1(9fe0cda10d1778d42ebc8db7d1f1e393c00848c4) ) // looks like a bad dump, first 0x2000 of some of the above roms?

	ROM_REGION( 0x80000, "upd", ROMREGION_ERASE00 )
	ROM_LOAD( "stspec", 0x0000, 0x080000, CRC(01e4a017) SHA1(f2f0cadf2334edf35db98af0dcb6d827c991f3f2) )
ROM_END


ROM_START( sc2sstar )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "superstar_std_ac_var_4-8-10pnd_tri2_rot_ass.bin", 0x0000, 0x010000, CRC(1e294299) SHA1(c961be1289bc77e34535d913ff19c75b1edeaba7) )

	ROM_REGION( 0x10000, "altrevs", 0 )
	ROM_LOAD( "superstar.bin", 0x0000, 0x010000, CRC(1e294299) SHA1(c961be1289bc77e34535d913ff19c75b1edeaba7) )
	ROM_LOAD( "superstar_dat_ac_8pnd-20p_tri2_ass.bin", 0x0000, 0x010000, CRC(c1134d01) SHA1(d36ea1c58261353c86da562825ccadcdc2ddb9e8) )
	ROM_LOAD( "superstar_dat_ac_tri3_ass.bin", 0x0000, 0x010000, CRC(caeaf463) SHA1(c07569da462de24f477a974f7d18368ea7b6b461) )
	ROM_LOAD( "superstar_dat_ac_var_4-8-10pnd_tri2_rot_ass.bin", 0x0000, 0x010000, CRC(f65ed8c9) SHA1(c0322c63d02d11425518fdacb98d30e7e49e498b) )
	ROM_LOAD( "superstar_dat_wi_ac_10pnd_tri2_rot_ass.bin", 0x0000, 0x010000, CRC(71ef63d6) SHA1(c0b1cbca8c801002a3eb7fd11474107c6bc6a1d1) )
	ROM_LOAD( "superstar_std_ac_8pnd-20p_tri2_ass.bin", 0x0000, 0x010000, CRC(441b76ff) SHA1(46b1ac77798cee4dfdd703af768c83b5c246f135) )
	ROM_LOAD( "superstar_std_ac_tri3_ass.bin", 0x0000, 0x010000, CRC(7a6c9f8d) SHA1(2a721823a95b2c324dd8500b32a04e8492e49f67) )
	ROM_LOAD( "superstar_std_wi_ac_10pnd_tri2_rot_ass.bin", 0x0000, 0x010000, CRC(adca7b5a) SHA1(4c889a0cda94c2698a4102a53d04594f7f931ee5) )
	ROM_LOAD( "supst20.15", 0x0000, 0x010000, CRC(c3446ec4) SHA1(3c1ad27385547a33993a839b53873d8b92214ade) )

	ROM_REGION( 0x80000, "upd", ROMREGION_ERASE00 )
	ROM_LOAD( "superstarsnd.bin", 0x0000, 0x080000, CRC(9a2609b5) SHA1(d29a5029e39cd44739682954f034f2d1f2e1cebf) )
ROM_END


ROM_START( sc2pe1g )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "pe95750415proma.bin", 0x0000, 0x010000, CRC(e518f28e) SHA1(0f693814409b9aa69d736dc97f26d2a79afd06c5) ) // not scrambled?

	ROM_REGION( 0x80000, "upd", ROMREGION_ERASE00 )
	ROM_LOAD( "pe95004031sound.bin", 0x0000, 0x080000, CRC(82f895b0) SHA1(888e172b24cb95c2723d9aa5cf1153a3af2ff2c7) )

	ROM_REGION( 0x80000, "other", ROMREGION_ERASE00 )
	ROM_LOAD( "pal.bin", 0x0000, 0x000010, CRC(d33fb7d2) SHA1(6de1a205808bccb9bc86f630c0eda261041a3b00) )
ROM_END


ROM_START( sc2wwcl )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "club-wild-west_std_ac_var_250pnd_ass.bin", 0x0000, 0x010000, CRC(a4c33524) SHA1(34d46b912488f630ddec301bde5ee1d87661b2a4) )

	ROM_REGION( 0x10000, "altrevs", 0 )
	ROM_LOAD( "club-wild-west_dat_ac_var_250pnd_ass.bin", 0x0000, 0x010000, CRC(deca21f2) SHA1(a79ef84271742f98e4557cba7b6b976f4d5b220f) )
	ROM_LOAD( "club-wild-west_dat_ac_var_ffp_ass.bin", 0x0000, 0x010000, CRC(2361e6c7) SHA1(5277d8d784a358441b86f4b9e3999511c74b7b09) )
	ROM_LOAD( "club-wild-west_std_ac_var_ffp_ass.bin", 0x0000, 0x010000, CRC(74b2592a) SHA1(f83a1fb5db69403a6b2922d2e3654fb753e0079c) )

	ROM_REGION( 0x80000, "upd", ROMREGION_ERASE00 )
	ROM_LOAD( "club-wild-west_sound.bin", 0x0000, 0x080000, NO_DUMP ) // guessing it's missing
ROM_END


ROM_START( sc2dick )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "spotteddick2_2.bin", 0x0000, 0x010000, CRC(497ef3b2) SHA1(f5021e35397081c62e817b86ff9e8a49d78748a5) )

	ROM_REGION( 0x10000, "altrevs", 0 )
	ROM_LOAD( "sp151-4n.p1", 0x0000, 0x010000, CRC(ee18a5a1) SHA1(17c2984fb305a571df83c663c9e42164f2322938) )
	ROM_LOAD( "sp151-4p.p1", 0x0000, 0x010000, CRC(94d96a28) SHA1(307e1cb5fe3c6050eb039dcd97e6ac88494707b3) )
	ROM_LOAD( "spot3-1p.p1", 0x0000, 0x010000, CRC(fa027939) SHA1(7fc6d26d179d976add3ca18c5df71dd9df7af1f2) )
//  ROM_LOAD( "spotd_v2_2.bin", 0x0000, 0x010000, CRC(497ef3b2) SHA1(f5021e35397081c62e817b86ff9e8a49d78748a5) )
	ROM_LOAD( "global-spotted-dick_euro.bin", 0x0000, 0x010000, CRC(695a3ec4) SHA1(f9f2f47f74479ef444997e2deef1c5f4677368ca) ) // this one isn't scrambled
	ROM_LOAD( "spotd31", 0x0000, 0x010000, CRC(794cec5b) SHA1(91ba4fcc459194fcf89f27e9c687cbdb8a10bb78) )
	ROM_LOAD( "spotteddickeuro.bin", 0x0000, 0x010000, CRC(c3b68821) SHA1(d86e098c3f0aec4f8068942934134e394075473d) )

	ROM_REGION( 0x100000, "upd", ROMREGION_ERASE00 ) // not upd?
	ROM_LOAD( "global-spotted-dick_snd.bin", 0x0000, 0x100000, CRC(f2c66aab) SHA1(6fe94a193779c91711588365591cf42d197cb7b9) )
ROM_END

ROM_START( sc2pick )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "pick2-3n.p1", 0x0000, 0x010000, CRC(b89c1dde) SHA1(8e1ece392dbb8e88daece79c5bea832149d8f442) )

	ROM_REGION( 0x10000, "altrevs", 0 )
	ROM_LOAD( "pick2-3p.p1", 0x0000, 0x010000, CRC(53ced0cb) SHA1(113a5e9414a3fcf0dacb6024748681f2b8e8bb55) )

	// club version roms
	ROM_LOAD( "dpic1-9n.p1", 0x0000, 0x010000, CRC(89b24a0b) SHA1(f56a79258497bc787b50d37ddf75b5d4920848e8) )
	ROM_LOAD( "dpic1-9p.p1", 0x0000, 0x010000, CRC(1c0adb51) SHA1(aeca44490c8b0517eddd69fcdc36cf2cafb4d844) )


	ROM_REGION( 0x200000, "upd", ROMREGION_ERASE00 ) // not upd?
	ROM_LOAD( "pickofthebunchsnd1.bin", 0x000000, 0x100000, CRC(f717b9c7) SHA1(06c90cc9779d475100926e986c742f0acffa0dc3) )
	ROM_LOAD( "pickofthebunchsnd2.bin", 0x100000, 0x100000, CRC(eaac3e67) SHA1(3aaed6514eeeb41c26f365789d8736908785b1c2) )
ROM_END


ROM_START( sc2rock )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "hb151-6n.p1", 0x0000, 0x010000, CRC(982de54a) SHA1(20e65e163f0455d683eb47ac37bc1e3355548c9a) )

	ROM_REGION( 0x10000, "altrevs", 0 )
	ROM_LOAD( "hb151-6p.p1", 0x0000, 0x010000, CRC(c9063e3c) SHA1(e47765ff56abb8d25c559cc5ebbe679ca40c498b) )
	ROM_LOAD( "hbiyr_euro.bin", 0x0000, 0x010000, CRC(bc4f8ffe) SHA1(de51fda4fe1c57945133a25c2ad8fba48064a23c) )
	ROM_LOAD( "rock1-4n.p1", 0x0000, 0x010000, CRC(e3888e8b) SHA1(7e394cbc219259a5eed9ccb283fff5f4b257e87f) )
	ROM_LOAD( "rock1-4p.p1", 0x0000, 0x010000, CRC(a4b61df4) SHA1(ffbfab5fc976edc68bb599625387295df793f449) )

	ROM_REGION( 0x200000, "upd", ROMREGION_ERASE00 ) // not upd?
	ROM_LOAD( "hbiyr_snd.bin", 0x0000, 0x100000, CRC(96cc0d54) SHA1(612f8c7f353bb847c1a28e2b76b64916d5b2d36a) )
ROM_END

ROM_START( sc2call )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "your2-7n.p1", 0x0000, 0x010000, CRC(9d3b4987) SHA1(131808aa90627b0aa830c6b49b12e15af96665a5) )

	ROM_REGION( 0x20000, "altrevs", 0 )
	ROM_LOAD( "dyor1-6p.p1", 0x0000, 0x010000, CRC(843edbd2) SHA1(67496753f3687800413418d65dcfc764695b4997) )
	ROM_LOAD( "your2-7p.p1", 0x0000, 0x020000, CRC(32ba485b) SHA1(00d962ba30f029ee2cc4447c42d57d9bc2592000) ) // seems to contain a different game in the 2nd half??

	// club rom
	ROM_LOAD( "dyor1-6n.p1", 0x0000, 0x010000, CRC(5e516bd1) SHA1(52a108e3d7aa9fdffb25e09922fa84c0155f18f5) )


	ROM_REGION( 0x200000, "upd", ROMREGION_ERASE00 ) // not upd?
	/* missing? */
ROM_END


ROM_START( sc2prom )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "alongtheprom.bin", 0x0000, 0x010000, CRC(0f212ba9) SHA1(34dfe67f8cbdf1cba806dcc7a3e872a8b59747d3) )

	ROM_REGION( 0x200000, "upd", ROMREGION_ERASE00 )
	ROM_LOAD( "alongthepromsnd.bin", 0x0000, 0x040000, CRC(380f56af) SHA1(9125c09e6585e6f4a2de9ea8715371662245aa9a) )
ROM_END

ROM_START( sc2payr )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "98400023", 0x0000, 0x010000, CRC(9478e97a) SHA1(c269f2a8e7eb6d76bf51563c6588d21bd71c1acf) )

	ROM_REGION( 0x200000, "upd", ROMREGION_ERASE00 )
	/* missing */
ROM_END

ROM_START( sc2bar7 )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "b7code.bin", 0x0000, 0x010000, CRC(bf8dbb1f) SHA1(fb07fbd1cc48bd0a6712ac9b71dcb8202720f86b) )

	ROM_REGION( 0x200000, "altrevs", ROMREGION_ERASE00 )
	ROM_LOAD( "bar73v1", 0x0000, 0x010000, CRC(2cc8dad8) SHA1(c3cce5e9ae032a6797828c8d42948ad749a03777) )
	ROM_LOAD( "bar72v5", 0x0000, 0x010000, CRC(6a9ce006) SHA1(c2b1efcdc576ea49243852f8a65c89fbca0ba7a4) )
	ROM_LOAD( "1v9.bin", 0x0000, 0x010000, CRC(f2aacd4d) SHA1(c6386de65cacbfb877ead00ee48d7cf9d43e61b0) )
	ROM_LOAD( "1v9p.bin", 0x0000, 0x010000, CRC(d7d38831) SHA1(5053b7e586b95d5e0853a506bb3df9203672469c) )
	ROM_LOAD( "2v0.bin", 0x0000, 0x010000, CRC(c5226f5d) SHA1(157b744b56a04f507798e857001b2e8255f2a3d9) )
	ROM_LOAD( "2v2.bin", 0x0000, 0x010000, CRC(0e7c9399) SHA1(9892313a4c8c7e8cca0f580ac6a2ad62fdf1ad1b) )
	ROM_LOAD( "2v3.bin", 0x0000, 0x010000, CRC(11fed7c4) SHA1(9164a81933fae960ba06d2b5aa5c47125db80fb7) )
	ROM_LOAD( "2v4.bin", 0x0000, 0x010000, CRC(26ddef97) SHA1(ab42a3b328c78257e4a207be0ab4e643c5c07b23) )
	ROM_LOAD( "2v6.bin", 0x0000, 0x010000, CRC(5842d19c) SHA1(a764a899745cf5a81f7c62ff8339c0847a7f8d50) )
	ROM_LOAD( "b7tok.bin", 0x0000, 0x020000, CRC(c3913709) SHA1(73024a3bbfbe13477e4daae78f54c694d112b936) )
	ROM_LOAD( "bar7", 0x008000, 0x008000, CRC(ce0429bc) SHA1(d9cda09589a6e7c72c4d777de2964abe6b4e18c3) )
	ROM_LOAD( "bar71v7", 0x0000, 0x010000, CRC(c3e01545) SHA1(4a4c06226587acb0875e6d19985916469b2eaa23) )
	ROM_REGION( 0x200000, "upd", ROMREGION_ERASE00 )
	ROM_LOAD( "b7snd1.bin", 0x0000, 0x00ff28, CRC(27efbf06) SHA1(735ffb552aacebe46405828b87de947b99edc4ea) )
ROM_END

ROM_START( sc2bbar7 )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "bb71v11.lit", 0x0000, 0x010000, CRC(4ba2cbbc) SHA1(6767d5935e12586a6bbd213e999940e3990af007) )

	ROM_REGION( 0x200000, "altrevs", ROMREGION_ERASE00 )
	ROM_LOAD( "bb7cz10", 0x0000, 0x010000, CRC(672f262f) SHA1(8d4ebf6df585ec34a6142175ab114367029b2cd8) )
	ROM_LOAD( "bb7v1", 0x0000, 0x020000, CRC(dee2e740) SHA1(e5bd24cb0722d2aec3ac2799f66cf5c8dd7ddd74) )
	ROM_LOAD( "big16a.bin", 0x0000, 0x010000, CRC(4fd95f69) SHA1(424c074efaccb2ad2bf4c97fdd37d9fa01c0a411) )
	ROM_LOAD( "big16b.bin", 0x0000, 0x010000, CRC(4604a0ff) SHA1(55d95ce2be1ef01fdeae7d727682989744da863d) )
	ROM_LOAD( "big16c.bin", 0x0000, 0x010000, CRC(f93eab19) SHA1(488c722d55c354923dc302558f28b58b1e71a64e) )
	ROM_LOAD( "bigb713a.bin", 0x0000, 0x010000, CRC(39792e6c) SHA1(5288cdd5d03314b07fa02d1c14c2d37068ba947e) )
	ROM_LOAD( "bigb713b.bin", 0x0000, 0x010000, CRC(7471adcd) SHA1(99369d9063c1bbe10ca7994b7d7936bbefc3c9ee) )
	ROM_LOAD( "bigb713c.bin", 0x0000, 0x010000, CRC(a4185331) SHA1(b501e7046ac4a7ea91b7e3b1ee56e57a3321d988) )
	ROM_LOAD( "bigb714a.bin", 0x0000, 0x010000, CRC(dbe28212) SHA1(eae79d4b671c5e9ac02ff71acdc45159a3ddc6a2) )
	ROM_LOAD( "bigb714b.bin", 0x0000, 0x010000, CRC(f59500b2) SHA1(90eb80249d1c1798922c0e39053b6839027cd20d) )
	ROM_LOAD( "bigb714c.bin", 0x0000, 0x010000, CRC(7d0fe1ab) SHA1(bcbdef94dc984560cede1249cc21803141539717) )
	ROM_LOAD( "bigb71v11.bin", 0x0000, 0x010000, CRC(7151e450) SHA1(4348c2cc3de96e28326325b4ae81b9cd20cda2cb) )
	ROM_LOAD( "bigx.bin", 0x0000, 0x008000, CRC(d6b6996b) SHA1(5226fc89e892ce0b3884bea0d220e3835dbb6c17) )
	ROM_LOAD( "bigx003.bin", 0x0000, 0x008000, CRC(638391f3) SHA1(8b34282c1d96d929f6e193486ddb6f348330d08c) )
	ROM_LOAD( "bigx007.bin", 0x0000, 0x008000, CRC(ac618c9d) SHA1(27813c09493f3a8d8fbf4a976ce1f5573c65a24d) )
	ROM_LOAD( "podbig7.bin", 0x0000, 0x010000, CRC(44c76818) SHA1(eb467c8bb1a9347c7537ef0c6b664620e0d5f015) )

	ROM_REGION( 0x200000, "upd", ROMREGION_ERASE00 )
	ROM_LOAD( "bb7snd", 0x0000, 0x080000, CRC(044c4ad5) SHA1(3d5e2e268bc2a4bac8df60e7d29b883f3d2fe61d) ) //Seems bad (loads of 00)
ROM_END

ROM_START( sc2flutr )
	//This is weird, it looks like the sc2 board is some sort of master controller for linked machines (serial connection)?.
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "code.bin", 0x0000, 0x010000, CRC(3e5d54d6) SHA1(a0ad4a4c723e0d03683c7f53fd0932b46f49cb41) )

	ROM_REGION( 0x200000, "altrevs", ROMREGION_ERASE00 )
	ROM_LOAD( "102.bin", 0x0000, 0x010000, CRC(c65261f5) SHA1(605799fc75b11255e6f17e168ad6c717e0d8d911) )
	ROM_LOAD( "103.bin", 0x0000, 0x010000, CRC(1923fbb3) SHA1(783a707580771842511d0aefa24694f1762f296e) )
	ROM_LOAD( "105.bin", 0x0000, 0x010000, CRC(b7069d2e) SHA1(62c6accb383a85f395ba33d50290044fffeb5d1d) )
	ROM_LOAD( "106.bin", 0x0000, 0x010000, CRC(8ce41a8f) SHA1(5d1fbd0ec16f19a10645315fb3adbb117ed30a4d) )
	ROM_LOAD( "107.bin", 0x0000, 0x010000, CRC(07862655) SHA1(d3d7cd7b8ecb3d5b821bc813c414ed99daa72b5b) )
	ROM_LOAD( "108.bin", 0x0000, 0x010000, CRC(5082e079) SHA1(e78489cd9e8763426de16b49af298fc9b6aaf6cc) )
	ROM_LOAD( "flutter.bin", 0x0000, 0x018008, CRC(281a9c91) SHA1(9ada7698aaafc0c60985a028ed6aab680eb355fb) )

	ROM_REGION( 0x80000, "upd", 0 )
	ROM_LOAD( "flutsnd.bin", 0x0000, 0x0105cb, CRC(947cddfa) SHA1(7ae5a3cae065e35519a13007767568471aacca1a) )
ROM_END

ROM_START( sc2smnud )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "smn1.2", 0x8000, 0x008000, CRC(e2d2fdd9) SHA1(0e2f44fa64dfa342752e53e9d514ca64e70b3046) )

	ROM_REGION( 0x200000, "altrevs", ROMREGION_ERASE00 )
	ROM_LOAD( "super_multi-nudge_game_(27512)", 0x0000, 0x010000, CRC(6a0de579) SHA1(308fec509371b93cb6ab957c83f2e041db449dfe) ) // both halves identical, but doesn't work, start vector is 4000?
	ROM_LOAD( "chezb10.bin", 0x0000, 0x010000, CRC(f00b6b95) SHA1(e2c3c7127bc9f9c77bd5b1f36aef47ffa05143a9) )
	ROM_LOAD( "chezb10.s", 0x0000, 0x010000, CRC(78e526a0) SHA1(2e7c90efa5c8d04214b5065aba446f9782c8298c) )

	ROM_REGION( 0x80000, "upd", 0 )
	ROM_LOAD( "super_multi-nudge_sound_(4meg)", 0x0000, 0x080000, CRC(efd87dab) SHA1(8b4b5de351ce3b1cefa4d0dc01072a942db072dc) )
ROM_END

ROM_START( sc2sghst )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "ghostgrn.bin", 0x0000, 0x010000, CRC(56d0141d) SHA1(0dd1b71892d60361626e073da12ca8f2ec2e610b) )

	ROM_REGION( 0x200000, "altrevs", ROMREGION_ERASE00 )
	ROM_LOAD( "gstgrncx", 0x0000, 0x010000, CRC(b0d4ff82) SHA1(afbec53b4be1d39bdb2227b58bea77d024cf3e2a) )
	ROM_LOAD( "lg1v0.bin", 0x0000, 0x010000, CRC(20950ebb) SHA1(f301dc147f0997d781cfecca35ef97bcb4627106) )
	ROM_LOAD( "sg1v0.bin", 0x0000, 0x010000, CRC(79ed8e14) SHA1(29ec1556d98bc536936046d6ca0b572f8d7674a4) )
	ROM_LOAD( "sg1v1.bin", 0x0000, 0x010000, CRC(d836be41) SHA1(d294180fcd2110d5d350bc578c91b67745996d51) )
	ROM_LOAD( "sg1v2.bin", 0x0000, 0x010000, CRC(d2cc95a0) SHA1(02394e08d108c215e2bff8556a61002acdb8a453) )
	ROM_LOAD( "sg1v4.bin", 0x0000, 0x010000, CRC(033cd636) SHA1(8be8f828a9b966b00f395a9cf33ec5f6b469cddb) )
	ROM_LOAD( "sg2v0.bin", 0x0000, 0x010000, CRC(7247d3a1) SHA1(1c349ed86ea335d5db78045d770ba550f8c365d0) )
	ROM_LOAD( "sg2v1.bin", 0x0000, 0x010000, CRC(ebfd636b) SHA1(9c25f2a368556ceff218e006b0917850ff80a53d) )
	ROM_LOAD( "sg2v2.bin", 0x0000, 0x010000, CRC(894856bc) SHA1(f193f538d80a6b0c5eb2b21a67b7a96db7127c8f) )
	ROM_LOAD( "sg2v2sam.bin", 0x0000, 0x039ac3, CRC(394bcb10) SHA1(dc48e22ead641945373f27e480680db37979c64b) )
	ROM_LOAD( "sg2v5.bin", 0x0000, 0x010000, CRC(6f8954c8) SHA1(e720b2cb49068d3788a2aef90ed464090cb757e1) )
	ROM_LOAD( "sg2v6.bin", 0x0000, 0x010000, CRC(52f79b3a) SHA1(13cfcd60d853283ef6bb722bb08756da88c4bfe8) )
	ROM_LOAD( "sg2v7.bin", 0x0000, 0x010000, CRC(1774d598) SHA1(f80ea78c0337d396fd6b4807fb59e1a54e929ea6) )
	ROM_LOAD( "sg2v7b.bin", 0x0000, 0x010000, CRC(9c14b804) SHA1(c3831a96640be9ab89f8e05a36c1ac967d50bd69) )
	ROM_LOAD( "sg2v7c.bin", 0x0000, 0x010000, CRC(07fcf016) SHA1(6f12018336c71afb98206a6c2e9276d6a21272ec) )
	ROM_LOAD( "sg2v8.bin", 0x0000, 0x010000, CRC(6362c3b6) SHA1(cbb7c56f64fc960e05f06632608b4e55f9e6385d) )
	ROM_LOAD( "sg2v9.bin", 0x0000, 0x010000, CRC(829ff8dd) SHA1(32aa1577aa61b3d7fc79e8890906a90225490542) )
	ROM_LOAD( "sghost.bin", 0x0000, 0x010000, CRC(a48a0c03) SHA1(0c647efaf0b9917bd9a7e07e010d3157f160e040) )
	ROM_LOAD( "sghost_gamesman_oneonly.bin", 0x0000, 0x010000, CRC(8fee4957) SHA1(95256c5bd511ffa11df25d3791c0ad8eeef9d9b6) )
	ROM_LOAD( "superghost2v9.bin", 0x0000, 0x010000, CRC(4ff0c3c2) SHA1(e8cefbcec11dab118299e04ef757cf7c2c485927) )
	ROM_REGION( 0x80000, "upd", 0 )
	ROM_LOAD( "sgsnd2", 0x0000, 0x080000, CRC(8ecf978a) SHA1(dd7cd6beb43dab011d661d9c507b20e507ad289b) )
	ROM_REGION( 0x800000, "altupd", 0 )
	ROM_LOAD( "sgstsnd", 0x0000, 0x03af13, CRC(2c6b2237) SHA1(7da432ccea45ce30bba72a0b565d53b33257f877) )
	ROM_LOAD( "ghostsnd.dat", 0x0000, 0x0d9ce8, CRC(56f4377f) SHA1(ddf296d2d705def19870b24019ecfdb42bc45342) )
ROM_END

ROM_START( sc2scshx )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "scx0v1e.bin", 0x0000, 0x010000, CRC(f0e52b5e) SHA1(431a56bbcb2c519ba5af9f58a3588ce5c2deb1f8) )
	ROM_REGION( 0x20000, "altrevs", 0 )
	ROM_LOAD( "scx 1p bin.dat", 0x0000, 0x010000, CRC(4a4d70d2) SHA1(9f14f6ba0a3c6c5c1a78c68b0bba49fb948142e5) )
	ROM_LOAD( "scx1.1b", 0x0000, 0x010000, CRC(7856b2bb) SHA1(cfa89418113bbbf9400c45b06a6e86fd15b57d52) )
	ROM_LOAD( "scx1v0.bin", 0x0000, 0x010000, CRC(8ea1be86) SHA1(42bd63e94e3876f21643813de64f16e701c1429f) )
	ROM_LOAD( "scx1v1.bin", 0x0000, 0x010000, CRC(4cb99292) SHA1(956b951a51d1dfae361f9e554eb918730c8013fc) )
	ROM_LOAD( "scx1v1a.bin", 0x0000, 0x010000, CRC(f01c5926) SHA1(5f499306f60111a423a74cdb624da07550ce48f5) )
	ROM_LOAD( "scx1v1a~.bin", 0x0000, 0x010000, CRC(90ce3521) SHA1(8cb7dbc78ac02e6772aaa3341b904767dd1c1301) )
	ROM_LOAD( "scx1v2.bin", 0x0000, 0x010000, CRC(054603f1) SHA1(9fca7772812bdfed1d67d916da520cbfd2bf82a8) )
	ROM_LOAD( "scx1v3.bin", 0x0000, 0x010000, CRC(711a0f93) SHA1(5b3efda6a01663655ec614feab9e1d0c857e823e) )
	ROM_LOAD( "scx1v6hi.bin", 0x0000, 0x010000, CRC(cae3fd0b) SHA1(1fe2ab0037c5a0be58378e95f72dc2782325fb71) )
	ROM_LOAD( "scx1v6lo.bin", 0x0000, 0x010000, CRC(ca5fdbca) SHA1(60079aeb4904e42a4a45feb7f31cf6c71b611845) )
	ROM_LOAD( "scx1v7hi.bin", 0x0000, 0x010000, CRC(b8ae7542) SHA1(22230e9a67c0f8408d6ba7adafd581cd3d62c5ad) )
	ROM_LOAD( "scx1v7lo.bin", 0x0000, 0x010000, CRC(1ed97ef6) SHA1(1aaf911369dc814ee2edf5d59baa2961bfc73168) )
	ROM_LOAD( "scx1v8hi.bin", 0x0000, 0x010000, CRC(06e35b38) SHA1(0a48489aee24066526da2cf56775f805d9603995) )
	ROM_LOAD( "scx1v8lo.bin", 0x0000, 0x010000, CRC(82bc1820) SHA1(301775e0e32e44d5cbe43c0cb83d94cf2aab9a50) )
	ROM_LOAD( "scxsp10.bin", 0x0000, 0x010000, CRC(e006d449) SHA1(73acc9c729e73d3a262d1a21fe89e00047eabdb2) )
	ROM_LOAD( "scxv2hi.pg", 0x0000, 0x010000, CRC(ee5219bd) SHA1(d193289ab9d2348292f122a7dfd4121c37b1635a) )
	ROM_LOAD( "scxv2lo.pg", 0x0000, 0x010000, CRC(48aea8e3) SHA1(601c22fda44171e292a284c0e6cb202cb8a14e24) )
	ROM_LOAD( "supercashx1v8.bin", 0x0000, 0x010000, CRC(3123327f) SHA1(b2edc4cbbe2fb1c451dc22dd8a7cf40d7012a3f3) )
/*
    QF18144*
    QP44*
    QV0*
    F0*
    X0*
    J0 0*
    N DEVICE XC9536-15-PC44 */
	ROM_REGION( 0x1000, "plds", 0 )
	ROM_LOAD( "supercx.jed", 0x0000, 0x0008e0, CRC(d80bc698) SHA1(2cfda3f945250253097b8a87924f14946c294894) )
ROM_END

ROM_START( sc2scshxgman )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "scxgm10.bin", 0x0000, 0x010000, CRC(f8c5bac8) SHA1(7858b2c8442b80b69598244870620d45042b7abb) )
	ROM_REGION( 0x200000, "altrevs", ROMREGION_ERASE00 )
	ROM_LOAD( "scxgm10a.bin", 0x0000, 0x010000, CRC(deab7e4e) SHA1(472a55b0ba289b0f4e538bb4c8b826dede3a40bb) )
//scxgm10b.bin identical
	ROM_LOAD( "scxhiv1.gmn", 0x0000, 0x010000, CRC(c43c2f43) SHA1(8bd8b2a71f19d6fd1f96d6032d1b60bb75dcaeb8) )
	ROM_LOAD( "scxhiv2.gmn", 0x0000, 0x010000, CRC(83a1ecc9) SHA1(b0176b25c97739442f3743136833d0e5fe51c03f) )
	ROM_LOAD( "scxlov1.gm", 0x0000, 0x010000, CRC(e305ff5a) SHA1(0bbc1cfaf7c7aaf324c65fd22148437e2bd4ca1e) )
	ROM_LOAD( "scxlov2.gm", 0x0000, 0x010000, CRC(62d384bd) SHA1(b7a0821fb37e3fb290e620888411d81525ab1635) )
	ROM_LOAD( "scxcoastv2.bin", 0x0000, 0x010000, CRC(53a0708a) SHA1(01fc2f5cd7f126989da6df7b295c2dae41b1a622) )
	ROM_LOAD( "scxcstv1", 0x0000, 0x010000, CRC(d8b62a7e) SHA1(c099f1d75b02c1535b81473a7ded6f58ab439430) )
	ROM_LOAD( "scxcstv2", 0x0000, 0x010000, CRC(ea0a9f41) SHA1(35799bacb2a1f3862169881f0f6dc10417d57fc4) )
	ROM_LOAD( "scxgm10c.bin", 0x0000, 0x010000, CRC(4a4d70d2) SHA1(9f14f6ba0a3c6c5c1a78c68b0bba49fb948142e5) )
	ROM_LOAD( "scxgm14n", 0x0000, 0x010000, CRC(604ec82a) SHA1(01876c1d97be5d4c32641b01314909254a7b5b26) )
	ROM_LOAD( "scxgm14o", 0x0000, 0x010000, CRC(52f27d15) SHA1(72a87d09f57b88f18ca185aace5026db870a40ff) )
	ROM_LOAD( "scxgm1jg", 0x0000, 0x010000, CRC(4995b83b) SHA1(aeb2d19dab1dab906f3418b5047bcebe0b395c90) )
	ROM_LOAD( "scxgm2.0", 0x0000, 0x020000, CRC(216cb51b) SHA1(0814115cb0d8f1042b3b9c9802079be0adc0e106) )
	ROM_LOAD( "scxgmbb.bin", 0x0000, 0x010000, CRC(1786f17d) SHA1(91b6f1badc09d28d81cfa08d8713ababae59dfab) )
	ROM_LOAD( "scxgmbt.bin", 0x0000, 0x010000, CRC(253a4442) SHA1(d362261a9e537e61be52efb13e825942934fa2ac) )
	ROM_LOAD( "scxgmv2.grn", 0x0000, 0x010000, CRC(b682bc15) SHA1(45b8aeedb63b8e0aa9ebf5b3b74e44cb07aedff9) )
	ROM_LOAD( "scxgmv2b", 0x0000, 0x010000, CRC(b682bc15) SHA1(45b8aeedb63b8e0aa9ebf5b3b74e44cb07aedff9) )
	ROM_LOAD( "scxgv1gr", 0x0000, 0x010000, CRC(a7f159ec) SHA1(6aedd61233d3e29e074b2c44679a7ac7ab999949) )
	ROM_LOAD( "scxgv1hi", 0x0000, 0x010000, CRC(04730062) SHA1(ea84b52556b03abe2ed2676cb14ef3a4d7dfdc64) )
	ROM_LOAD( "scxgv1lo", 0x0000, 0x010000, CRC(c5db0a69) SHA1(7a500bd4f68ce3bc56fd3d370f1144c485089023) )
	ROM_LOAD( "scxgv2gr", 0x0000, 0x010000, CRC(2e37f306) SHA1(ba0a8dc107abc9ab093c2d6f81ec3f11e5460598) )
	ROM_LOAD( "scxgv2hi", 0x0000, 0x010000, CRC(8db5aa88) SHA1(626db3b1eddb50137e8f05535137db9dff466806) )
	ROM_LOAD( "scxgv2lo", 0x0000, 0x010000, CRC(4c1da083) SHA1(75684018ed2988688bb3be7990dc0050d28bd4ef) )
ROM_END

ROM_START( sc2scshxstar )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "scxsp10.bin", 0x0000, 0x010000, CRC(2fe512ad) SHA1(d409f27a62405dc45f487f9351e4d158e4d35440) )
	ROM_REGION( 0x200000, "altrevs", ROMREGION_ERASE00 )
	ROM_LOAD( "cxspv1gr", 0x0000, 0x010000, CRC(67f69bd4) SHA1(ee2dd0cd98c98a4727df8c7c721de9ac49b583ff) )
	ROM_LOAD( "cxspv2gr", 0x0000, 0x010000, CRC(2fe512ad) SHA1(d409f27a62405dc45f487f9351e4d158e4d35440) )
	ROM_LOAD( "scxspv1grn.bin", 0x0000, 0x010000, CRC(67f69bd4) SHA1(ee2dd0cd98c98a4727df8c7c721de9ac49b583ff) )
	ROM_LOAD( "scxspv2grn.bin", 0x0000, 0x010000, CRC(2fe512ad) SHA1(d409f27a62405dc45f487f9351e4d158e4d35440) )
	ROM_LOAD( "scxv1hi.str", 0x0000, 0x010000, CRC(5c7781d7) SHA1(0bbd48d6b506a31fe7d48122589f434a4473c225) )
	ROM_LOAD( "scxv1lo.str", 0x0000, 0x010000, CRC(2dc93bce) SHA1(63ce1eecf454f83f51107ec7c1d8ac04408c7414) )
	ROM_LOAD( "scxv2hi.str", 0x0000, 0x010000, CRC(e7e921ea) SHA1(fa150e78981bd91f5b8d148a1a32836ee4dde926) )
	ROM_LOAD( "scxv2lo.str", 0x0000, 0x010000, CRC(96579bf3) SHA1(02abf8c84119a3ac828f91c236ce8573cf6cd646) )
	ROM_LOAD( "scxhiv1.stp", 0x0000, 0x010000, CRC(f087f88c) SHA1(a303e1d8249eb2a83e122f5b355dc084ce46b172) )
	ROM_LOAD( "scxhiv2.stp", 0x0000, 0x010000, CRC(adf9a0bf) SHA1(58c0e64175ceb222e285fae29337f2a5437364e4) )
	ROM_LOAD( "scxlov1.stp", 0x0000, 0x010000, CRC(3b53fcaa) SHA1(bb6d9b70063dbbeb7562225a07610b424d1ebdd4) )
	ROM_LOAD( "scxlov2.stp", 0x0000, 0x010000, CRC(42086397) SHA1(254bc42c9f2cc55bbeecbe2fb06234aaeda7967d) )
ROM_END

ROM_START( sc2scshxcas )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "scx1v7h", 0x0000, 0x010000, CRC(b8ae7542) SHA1(22230e9a67c0f8408d6ba7adafd581cd3d62c5ad) )
	ROM_REGION( 0x200000, "altrevs", ROMREGION_ERASE00 )
	ROM_LOAD( "scx1v7l", 0x0000, 0x010000, CRC(1ed97ef6) SHA1(1aaf911369dc814ee2edf5d59baa2961bfc73168) )// Second board?
ROM_END

ROM_START( sc2cgc )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "95751968.p1", 0x0000, 0x010000, CRC(e9eef2be) SHA1(61015e0c90fd516da56243a7eef3d5d2412d880f) )
	ROM_REGION( 0x200000, "upd", ROMREGION_ERASE00 )
ROM_END

ROM_START( sc2cnile )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "club-cash-on-the-nile_std_ac_var_200pnd_rot_ass.bin", 0x0000, 0x010000, CRC(41cbb60d) SHA1(4fede32a8d0957a46732f6851d4af7fd959d9fb5) )

	ROM_REGION( 0x80000, "altrevs", 0 )
	ROM_LOAD( "cash-nile 150 047s.bin", 0x0000, 0x010000, CRC(2d8e9037) SHA1(b3d93488d662260cfaaf624baec68dbe92f71640) )
	ROM_LOAD( "club-cash-on-the-nile_dat_ac_var_200pnd_rot_ass.bin", 0x0000, 0x010000, CRC(3bfac54c) SHA1(ecfd7607676c1620ee37718578675437911cf147) )
	ROM_LOAD( "club-cash-on-the-nile_dat_ac_var_250pnd_rot_ass.bin", 0x0000, 0x010000, CRC(42d0a11d) SHA1(b38fa1360f0b8d465bb0e0759f73e0b98a545ad3) )
	ROM_LOAD( "club-cash-on-the-nile_std_ac_var_150pnd_rot_ass.bin", 0x0000, 0x010000, CRC(4a5b4b9f) SHA1(aaeaa42cf42d91002c61e4c0df49d7ef97e00b2a) )
	ROM_REGION( 0x200000, "upd", ROMREGION_ERASE00 )
ROM_END

ROM_START( sc2casr )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "casino-royale_std_wit_ac_10pnd-20p_ass.bin", 0x0000, 0x010000, CRC(11663ae8) SHA1(f8e0fb8b23c192f48df4e5d9fc94f8c625d4771c) )

	ROM_REGION( 0x80000, "altrevs", 0 )
	ROM_LOAD( "casino-royale_dat_ac_var_8-10pnd_ass.bin", 0x0000, 0x010000, CRC(8c59c768) SHA1(fec9cfbd9a8c262d053ae84c09535a7d8331bfa2) )
	ROM_LOAD( "casino-royale_dat_ac_var_tri3_ass.bin", 0x0000, 0x010000, CRC(bc805e51) SHA1(c6b0e2fc1011688ca9c374bb5cca5788e6dea005) )
	ROM_LOAD( "casino-royale_dat_ss_ac_var_8pnd_ass.bin", 0x0000, 0x010000, CRC(341e69a8) SHA1(3b719a437e11ca71a9acccc76cd5f2b05325e203) )
	ROM_LOAD( "casino-royale_dat_wit_ac_10pnd-20p_ass.bin", 0x0000, 0x010000, CRC(8a6eab70) SHA1(ef2bb7f7fd534dfee1322b9fd151e24642cb28bf) )
	ROM_LOAD( "casino-royale_dat_wit_ac_10pnd_tri3_ass.bin", 0x0000, 0x010000, CRC(6436974c) SHA1(419d4f58f518582f0fe334323d0d9fa68f9458a6) )
	ROM_LOAD( "casino-royale_std_ac_var_8-10pnd_ass.bin", 0x0000, 0x010000, CRC(a25c051a) SHA1(6b7e954c53e3f1f90d24f88c7fd09606a1cd8630) )
	ROM_LOAD( "casino-royale_std_ac_var_tri3ss.bin", 0x0000, 0x010000, CRC(5f47c57b) SHA1(5ce7baab279ee28c337a4ee72038b6d6cee1da9c) )
	ROM_LOAD( "casino-royale_std_ss_ac_var_8pnd_ass.bin", 0x0000, 0x010000, CRC(6a585c28) SHA1(8ac91085efd8382544868b8b0b45fddede38b5ec) )
	ROM_LOAD( "casino-royale_std_wit_ac_10pnd_tri3_ass.bin", 0x0000, 0x010000, CRC(cc2ef9dd) SHA1(9e85e319fbe74f31de1fddc4f15dd0ce49691d2c) )
	ROM_REGION( 0x200000, "upd", ROMREGION_ERASE00 )
ROM_END

ROM_START( sc2cmbt )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "cat-and-mouse-and-bonzo-too_std_ac_10pnd_ass.bin", 0x0000, 0x010000, CRC(6d1612e7) SHA1(641104b4ebc99ec3b20a081fccbde70084cc329a) )

	ROM_REGION( 0x80000, "altrevs", 0 )
	ROM_LOAD( "cat-and-mouse-and-bonzo-too_dat_ac_10pnd_ass.bin", 0x0000, 0x010000, CRC(14770ed7) SHA1(6e7a0f596063c28cad0ecc13241e53e4a5b025f9) )
	ROM_REGION( 0x200000, "upd", ROMREGION_ERASE00 )
ROM_END





ROM_START( sc2dbl )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "double-diamond_std_ga_20p_ass.bin", 0x0000, 0x010000, CRC(eded5c38) SHA1(31a687de56f95f0ab730fed2b618e492fbc0c749) )

	ROM_REGION( 0x80000, "altrevs", 0 )
	ROM_LOAD( "double-diamond_dat_ga_20p_ass.bin", 0x0000, 0x010000, CRC(7e117a69) SHA1(d73ec1cfe3d2b9d9e1f18a3979d76b13b5d89988) )
	ROM_LOAD( "double-diamond_dat_ms_20p_ass.bin", 0x0000, 0x010000, CRC(d45b4a5c) SHA1(ccf33b36c01155e78492e861ae4a328b4086ade9) )
	ROM_LOAD( "double-diamond_std_ms_20p_ass.bin", 0x0000, 0x010000, CRC(90a52fc4) SHA1(afb6078fc884e08afb4f6a9ac2a8abcb36fae2bd) )
	ROM_REGION( 0x200000, "upd", ROMREGION_ERASE00 )
ROM_END




ROM_START( sc2flaca )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "flashcash_std_ac_var_10pnd_tri2_ass.bin", 0x0000, 0x010000, CRC(e7e0fe1f) SHA1(a6c1f6565d785aa36daecb55a5c33042a84117e6) )

	ROM_REGION( 0x80000, "altrevs", 0 )
	ROM_LOAD( "flashcash_dat_ac_var_10pnd_tri2_ass.bin", 0x0000, 0x010000, CRC(89d6df7f) SHA1(cc8f6ca2233d77cbdcb0735cb31de9cdd9a66408) )
	ROM_LOAD( "flashcash_dat_ac_var_10pnd_tri3_ass.bin", 0x0000, 0x010000, CRC(0d6d8eb2) SHA1(c47e61d08afbeb542132b2f8e157417a008e9387) )
	ROM_LOAD( "flashcash_dat_wi_ac_10pnd_tri2_ass.bin", 0x0000, 0x010000, CRC(b781ecf9) SHA1(1d62a315f8292144d8129c1be9a83a5e717c6ed9) )
	ROM_LOAD( "flashcash_std_ac_var_10pnd_tri3_ass.bin", 0x0000, 0x010000, CRC(2cb1802a) SHA1(ae7bc9374f5882ba4142d67ad83335d4d2accf0c) )
	ROM_LOAD( "flashcash_std_wi_ac_10pnd_tri2_ass.bin", 0x0000, 0x010000, CRC(5f5b879b) SHA1(1b0f0cf54112615ea6b2ecdebc4076d132531a2c) )
	ROM_REGION( 0x200000, "upd", ROMREGION_ERASE00 )
ROM_END

ROM_START( sc2foot )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "football-club_std_ac_250pnd-20p_ass.bin", 0x0000, 0x010000, CRC(b4d46ee2) SHA1(3cbe603c2703570eb49682ca9dbb6ad9ede020e6) )

	ROM_REGION( 0x80000, "altrevs", 0 )
	ROM_LOAD( "95000590.mtx", 0x0000, 0x004d18, CRC(cc596be2) SHA1(190d9b9e3e06b933be7ff0b6bc69955709dedd7d) )
	ROM_LOAD( "football-club_dat_ac_250pnd-20p_ass.bin", 0x0000, 0x010000, CRC(fabaf221) SHA1(ddefc6f46339f83b6cfbacbe1ff6cf065d0157aa) )
	ROM_LOAD( "football-club_dat_ac_var_100pnd-5p_ass.bin", 0x0000, 0x010000, CRC(7f6acf47) SHA1(b6e8254d4af1e5a85166e4eca1dc2b1ea2eed292) )
	ROM_LOAD( "football-club_dat_var_ass.bin", 0x0000, 0x010000, CRC(ac088604) SHA1(d1db45aa19b645aad56bbf84e551dc1cca22f92d) )
	ROM_LOAD( "football-club_mtx_ass.bin", 0x0000, 0x010000, CRC(6b78de57) SHA1(84638836cdbfa6e4b3b76cd38e238d12bb312c53) )
	ROM_LOAD( "football-club_std_ac_var_100pnd-5p_ass.bin", 0x0000, 0x010000, CRC(bf35ad75) SHA1(c5e8906138184449b90eea2e280e6f75e6768776) )
	ROM_LOAD( "football-club_std_var_ass.bin", 0x0000, 0x010000, CRC(cae35c7a) SHA1(2beda0150cd2d413269c350e34102c0e1d3ed007) )
	ROM_LOAD( "footballclub_std_var_ass.bin", 0x0000, 0x010000, CRC(cae35c7a) SHA1(2beda0150cd2d413269c350e34102c0e1d3ed007) )
	ROM_REGION( 0x200000, "upd", ROMREGION_ERASE00 )
ROM_END


ROM_START( sc2gcclb )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "club-golden-casino_std_ac_250pnd-20p_ass.bin", 0x0000, 0x010000, CRC(5f7fc343) SHA1(264c5bba36c820440c2ed97c04d4dd3592e111da) )

	ROM_REGION( 0x80000, "altrevs", 0 )
	ROM_LOAD( "95004065.p1", 0x0000, 0x080000, CRC(2670726b) SHA1(0f8045c68131191fceea5728e14c901d159bfb57) )
	ROM_LOAD( "club-golden-casino_dat_ac_100pnd-5p_ass.bin", 0x0000, 0x010000, CRC(1f5c2a2b) SHA1(facaab47716ae3c4a10839523f3249074ae8abb1) )
	ROM_LOAD( "club-golden-casino_dat_ac_250pnd-20p_ass.bin", 0x0000, 0x010000, CRC(b4dee6d2) SHA1(856672fb4767f66e976619392fc8e659fbca3c2e) )
	ROM_LOAD( "club-golden-casino_dat_ac_var_ass.bin", 0x0000, 0x010000, CRC(2ad9647e) SHA1(d423c060996417f3f7f1b61e911b6e523ad08e7a) )
	ROM_LOAD( "club-golden-casino_std_ac_100pnd-5p_ass.bin", 0x0000, 0x010000, CRC(bf7b9ff1) SHA1(890a6b96592e9d2e890bea95e711b890c1cda7ad) )
	ROM_LOAD( "club-golden-casino_std_ac_var_ass.bin", 0x0000, 0x010000, CRC(2de27b80) SHA1(57f1c40ceeb6ab82e9bac547aa00d8c1c1c07dab) )
	ROM_LOAD( "95000589.p1", 0x0000, 0x010000, CRC(36400074) SHA1(611b48650e59b52f661be2730afaef2e5772607c) )
	ROM_REGION( 0x200000, "upd", ROMREGION_ERASE00 )
	// guess
	ROM_LOAD( "gold_cas.snd", 0x0000, 0x080000, CRC(d93d39fb) SHA1(ce0c0c1430a6136ce39ffae018b009e629cbad61) )
ROM_END

ROM_START( sc2groul )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "golden-roulette_std_ms_20p_ass.bin", 0x0000, 0x010000, CRC(d865188a) SHA1(c4318984b6abdb5671fe7c323608e4af84d1ae6e) )

	ROM_REGION( 0x10000, "altrevs", 0 )
	ROM_LOAD( "golden-roulette_dat_ms_20p_ass.bin", 0x0000, 0x010000, CRC(c388fa79) SHA1(4ce7d183130fd2aae2c4ffeff652e2602208c3ff) )
	ROM_REGION( 0x200000, "upd", ROMREGION_ERASE00 )
ROM_END

ROM_START( sc2gldsh )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "gsho1_3", 0x0000, 0x010000, CRC(783ee8cb) SHA1(b509f167fddc71e313ffbff0a3e1ce7d387c424e) )
	ROM_REGION( 0x200000, "upd", ROMREGION_ERASE00 )
ROM_END

ROM_START( sc2gtr )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "gtr.bin", 0x0000, 0x010000, CRC(b6cd277c) SHA1(4951bb6b4cc1bf655d3b63b7af4f1a6a297a201c) )

	ROM_REGION( 0x200000, "upd", ROMREGION_ERASE00 )
	ROM_LOAD( "gtr_snd.bin", 0x0000, 0x080000, CRC(90eaa8b6) SHA1(9c15787d73889013717f01c6b11780b7f9314b05) )
ROM_END

ROM_START( sc2heypr )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "hey-presto_std_ac_4pnd-10p_ass.bin", 0x0000, 0x010000, CRC(7f3803fa) SHA1(56a12bb96fe7cce07734842f6c5581648154154e) )

	ROM_REGION( 0x10000, "altrevs", 0 )
	ROM_LOAD( "hey-presto_dat_ac_4pnd-10p_ass.bin", 0x0000, 0x010000, CRC(cb8780ad) SHA1(a0a3cd2c9c3caf6607b55d2d14f6e3d581540808) )
	ROM_REGION( 0x200000, "upd", ROMREGION_ERASE00 )
ROM_END



ROM_START( sc2hypr )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "hyperactive_std_ac_var_10pnd_ass.bin", 0x0000, 0x010000, CRC(042b848c) SHA1(ceec2cb26ae9b969c5da3cc0be25455b1f89d09f) )

	ROM_REGION( 0x10000, "altrevs", 0 )
	ROM_LOAD( "hyperactive_dat_ac_var_10pnd_ass.bin", 0x0000, 0x010000, CRC(e6956fec) SHA1(ea8e25e16a451a1f52f30567571090f635379f4c) )
	ROM_LOAD( "hyperactive_dat_wi_ac_10pnd-20p_ass.bin", 0x0000, 0x010000, CRC(6d11a9eb) SHA1(d68564a96984c5dde536add4507bc8bae75e19ea) )
	ROM_LOAD( "hyperactive_std_wi_ac_10pnd-20p_ass.bin", 0x0000, 0x010000, CRC(2d01bc08) SHA1(c2186fb639735d4e1d46ceaeae6eee63c7a740b7) )
	ROM_REGION( 0x200000, "upd", ROMREGION_ERASE00 )
ROM_END

ROM_START( sc2kcclb )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "club-king-cash_std_ac_var_ass.bin", 0x0000, 0x010000, CRC(60c1eccd) SHA1(5b9f5c8c7cc501b557eadcf7e520967c58b8ce1a) )

	ROM_REGION( 0x10000, "altrevs", 0 )
	ROM_LOAD( "club-king-cash_dat_ac_ds_rot_ass.bin", 0x0000, 0x010000, CRC(3fb9f61f) SHA1(176e517d049b4e588a2fe425041d701ff8e3e7b8) )
	ROM_LOAD( "club-king-cash_dat_ac_var_ass.bin", 0x0000, 0x010000, CRC(c83be316) SHA1(9e87152977fdabb71ee6d8be1d382b978d856c83) )
	ROM_LOAD( "club-king-cash_std_ac_ds_rot_ass.bin", 0x0000, 0x010000, CRC(cf13d7e4) SHA1(6b3bfc8e7e4877e7ab7e5d3adbd89a6bcc2ebde9) )
	ROM_REGION( 0x200000, "upd", ROMREGION_ERASE00 )
ROM_END

ROM_START( sc2maina )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "game115s.rom", 0x0000, 0x010000, CRC(6f3b16d2) SHA1(b5c7796a4a87dc5ffa6243863ac3f9bc777228ca) )
	ROM_REGION( 0x200000, "upd", ROMREGION_ERASE00 )
	ROM_LOAD( "snd2008.rom", 0x0000, 0x040000, CRC(9b2b5b33) SHA1(3ec9200529eba5bc4ef4a9a289d58312f29628a5) )
ROM_END


ROM_START( sc2olgld )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "olympic-gold_std_var_ac_8-10pnd_ass.bin", 0x0000, 0x010000, CRC(baa98b60) SHA1(2b73eb21d6b612fabf855edf9f6c46897714729b) )

	ROM_REGION( 0x10000, "altrevs", 0 )
	ROM_LOAD( "olympic-gold_dat_var_ac_8-10pnd_ass.bin", 0x0000, 0x010000, CRC(1348a519) SHA1(e7a2434235347433522c55e4d4f89fbb97759765) )
	ROM_LOAD( "olympic-gold_dat_wi_var_ac_10pnd_ass.bin", 0x0000, 0x010000, CRC(bb50e0a2) SHA1(b208053e114f7fb411f16f02aab3061f6075b42c) )
	ROM_LOAD( "olympic-gold_std_wi_var_ac_10pnd_ass.bin", 0x0000, 0x010000, CRC(8a8b8429) SHA1(ba886878d4ef428653032d04e21a9031fdea68e0) )
	ROM_REGION( 0x200000, "upd", ROMREGION_ERASE00 )
ROM_END

ROM_START( sc2relgm )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "reel-gems_std_ms_20p_ass.bin", 0x0000, 0x010000, CRC(ebbae111) SHA1(6372e19b0dd030aac517344449ce47e8f6f74b29) )

	ROM_REGION( 0x10000, "altrevs", 0 )
	ROM_LOAD( "reel-gems_dat_ga_20p_ass.bin", 0x0000, 0x010000, CRC(86e81781) SHA1(7b59efa627f70b2c3598c5abd276a7c2737b0751) )
	ROM_LOAD( "reel-gems_dat_ms_20p_ass.bin", 0x0000, 0x010000, CRC(5abde2bc) SHA1(74a745938934533b1b33c99828b79fa9d1e86a91) )
	ROM_REGION( 0x200000, "upd", ROMREGION_ERASE00 )
ROM_END


ROM_START( sc2topwk )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "top-wack_std_wi_ac_10pnd_tri1_ass.bin", 0x0000, 0x010000, CRC(248080cf) SHA1(067077af93dd6a41bd6d84d9ace9ac4cea36f01b) )
	ROM_REGION( 0x10000, "altrevs", 0 )
	ROM_LOAD( "top-wack_dat_wi_ac_10pnd_tri1_ass.bin", 0x0000, 0x010000, CRC(56fd3003) SHA1(37ef5c9a750f9bdc609fc78ea5131424eb74c79d) )
	ROM_REGION( 0x200000, "upd", ROMREGION_ERASE00 )
ROM_END


ROM_START( sc2cb7 )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "bar7.bin", 0x0000, 0x010000, CRC(c5b426e8) SHA1(a60aed70f2a4cf4356fae61c1031124fd5987d86) )

	ROM_REGION( 0x10000, "altrevs", 0 )
	ROM_LOAD( "casino-bar-7_std_10pnd_ass.bin", 0x0000, 0x010000, CRC(3d0ae920) SHA1(4c6575d979f686e928842afc3ee9b344e45e3a31) )
	ROM_LOAD( "casino-bar-7_dat_10pnd_ass.bin", 0x0000, 0x010000, CRC(6960f4f8) SHA1(7274276d1d4032ed7fe660ac0f87eea1e9c6e4e4) )
	ROM_LOAD( "casinobar7_bfm_allcash.bin", 0x0000, 0x010000, CRC(2d459734) SHA1(293cf250b7b71b55325b18a10be7dead1cddb565) )
	ROM_LOAD( "bar7protocol.bin", 0x8000, 0x008000, CRC(e9c022ed) SHA1(e93b4506830a2f098eceb0b419d648bf3a9d02a4) ) // half size?

	ROM_REGION( 0x80000, "upd", ROMREGION_ERASE00 )
	ROM_LOAD( "casinobar7_bfm_snd1.bin", 0x0000, 0x080000, CRC(9a2609b5) SHA1(d29a5029e39cd44739682954f034f2d1f2e1cebf) ) // == superstarsnd.bin
ROM_END


ROM_START( sc2cgcas )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "club-grand-casino_std_ac_p65_ass.bin", 0x0000, 0x010000, CRC(6ca2cccb) SHA1(762e0809e70d4dd2161a2ffcc30d191720e8ad9a) )
	ROM_REGION( 0x10000, "altrevs", 0 )
	ROM_LOAD( "club-grand-casino_dat_ac_250pnd-25p_ass.bin", 0x0000, 0x010000, CRC(77cf0f11) SHA1(88da3f2e18f621033a8d32428b1422d5e3873ab5) )
	ROM_LOAD( "club-grand-casino_dat_ac_var_ass.bin", 0x0000, 0x010000, CRC(15c3b640) SHA1(94a4e105b9fbd4b12ec246a0f1a6751acf25eac2) )
	ROM_LOAD( "club-grand-casino_std_ac_250pnd-25p_ass.bin", 0x0000, 0x010000, CRC(edfc3d74) SHA1(192a893b5a9b188de094d0f45881788306523e0b) )
	ROM_REGION( 0x200000, "upd", ROMREGION_ERASE00 )
ROM_END


ROM_START( sc2cvega )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "cash_vegas_std_ac_var_a.bin", 0x0000, 0x010000, CRC(88dd09b9) SHA1(36b4f3504794b638a31e45d1f155360166f77ab2) )
	ROM_REGION( 0x10000, "altrevs", 0 )
	ROM_LOAD( "cash_vegas_dat_ac_var_10pnd_a.bin", 0x0000, 0x010000, CRC(e880c6b6) SHA1(387f7e3659e42ac488db9a4768c2035f7c870c44) )
	ROM_LOAD( "cash_vegas_dat_ac_var_10pnd_tri3.bin", 0x0000, 0x010000, CRC(ab3e503c) SHA1(2c26865eab6cf128d8f3ff09077daa3c4d2aee30) )
	ROM_LOAD( "cash_vegas_dat_to_8pnd_a.bin", 0x0000, 0x010000, CRC(cabec1cd) SHA1(acbe41e0d5fa77f11df8d119ad09aeccd421f603) )
	ROM_LOAD( "cash_vegas_dat_wi_10pnd_20p_a.bin", 0x0000, 0x010000, CRC(99ee9eef) SHA1(c4b325a39e898f069ac3471af8ea955c62c488a5) )
	ROM_LOAD( "cash_vegas_std_ac_var_10pnd_tri3_a.bin", 0x0000, 0x010000, CRC(3d808af5) SHA1(db29c03a33dce6342fec4da3664590ab072dd6d9) )
	ROM_LOAD( "cash_vegas_std_to_var_8pnd_a.bin", 0x0000, 0x010000, CRC(c8e98a0e) SHA1(1436f3a464b2f298b161e5328f0540cf23441803) )
	ROM_LOAD( "cash_vegas_std_w_i_10pnd_20p_a.bin", 0x0000, 0x010000, CRC(521b918d) SHA1(4d9b94d561d89aa1dd8746a33eb27d89b53b6ba9) )
	ROM_REGION( 0x200000, "upd", ROMREGION_ERASE00 )
ROM_END







/* Video Based (Adder 2) */

#define GAME_FLAGS GAME_SUPPORTS_SAVE|GAME_REQUIRES_ARTWORK|GAME_NOT_WORKING|GAME_MECHANICAL

GAMEL( 1993, qntoondo, qntoond,	  scorpion2_vid, qntoond,   adder_dutch,0,       "BFM/ELAM", "Quintoon (Dutch, Game Card 95-750-136)",		GAME_SUPPORTS_SAVE,layout_quintoon )
GAMEL( 1993, quintoon, 0,		  scorpion2_vid, quintoon,  quintoon,   0,       "BFM",      "Quintoon (UK, Game Card 95-750-206)",			GAME_SUPPORTS_SAVE|GAME_IMPERFECT_SOUND,layout_quintoon ) //Current samples need verification
GAMEL( 1993, quintond, quintoon,  scorpion2_vid, quintoon,  quintoon,   0,       "BFM",      "Quintoon (UK, Game Card 95-751-206, Datapak)",GAME_SUPPORTS_SAVE|GAME_IMPERFECT_SOUND|GAME_NOT_WORKING,layout_quintoon ) //Current samples need verification
GAMEL( 1993, quintono, quintoon,  scorpion2_vid, quintoon,  quintoon,   0,       "BFM",      "Quintoon (UK, Game Card 95-750-203)",			GAME_SUPPORTS_SAVE|GAME_IMPERFECT_SOUND,layout_quintoon ) //Current samples need verification
GAMEL( 1993, qntoond,  0,		  scorpion2_vid, qntoond,   adder_dutch,0,       "BFM/ELAM", "Quintoon (Dutch, Game Card 95-750-243)",		GAME_SUPPORTS_SAVE,layout_quintoon )
GAMEL( 1994, pokio,    0,		  scorpion2_vid, pokio,     adder_dutch,0,       "BFM/ELAM", "Pokio (Dutch, Game Card 95-750-278)",			GAME_SUPPORTS_SAVE,layout_pokio )
GAMEL( 1995, slotsnl,  0,		  scorpion2_vid, slotsnl,   adder_dutch,0,       "BFM/ELAM", "Slots (Dutch, Game Card 95-750-368)",			GAME_SUPPORTS_SAVE,layout_slots )
GAMEL( 1995, paradice, 0,		  scorpion2_vid, paradice,  adder_dutch,0,       "BFM/ELAM", "Paradice (Dutch, Game Card 95-750-615)",		GAME_SUPPORTS_SAVE,layout_paradice )
GAMEL( 1996, pyramid,  0,		  scorpion2_vid, pyramid,   pyramid,	0,       "BFM/ELAM", "Pyramid (Dutch, Game Card 95-750-898)",		GAME_SUPPORTS_SAVE,layout_pyramid )

GAMEL( 1996, sltblgtk, 0,		  scorpion2_vid, sltblgtk,  sltsbelg,   0,       "BFM/ELAM", "Slots (Belgian Token, Game Card 95-750-943)",	GAME_SUPPORTS_SAVE,layout_sltblgtk )
GAMEL( 1996, sltblgpo, 0,		  scorpion2_vid, sltblgpo,  sltsbelg,   0,       "BFM/ELAM", "Slots (Belgian Cash, Game Card 95-750-938)",	GAME_SUPPORTS_SAVE,layout_sltblgpo )
GAMEL( 1996, sltblgp1, sltblgpo,  scorpion2_vid, sltblgpo,  sltsbelg,   0,       "BFM/ELAM", "Slots (Belgian Cash, Game Card 95-752-008)",	GAME_SUPPORTS_SAVE,layout_sltblgpo )
GAMEL( 1997, gldncrwn, 0,		  scorpion2_vid, gldncrwn,  gldncrwn,   0,       "BFM/ELAM", "Golden Crown (Dutch, Game Card 95-752-011)",	GAME_SUPPORTS_SAVE,layout_gldncrwn )

/* Non-Video */

GAMEL( 1994, sc2drwho	, 0			,  scorpion2		, drwho		, drwho		, 0,		 "BFM",      "Dr.Who The Timelord (set 1, UK, Game Card 95-750-288) (Scorpion 2/3)", GAME_SUPPORTS_SAVE|GAME_REQUIRES_ARTWORK|GAME_MECHANICAL,layout_drwho)
GAMEL( 1994, sc2drwho1	, sc2drwho	,  scorpion2		, drwho		, drwho		, 0,		 "BFM",      "Dr.Who The Timelord (set 2, UK, Game Card 95-750-661) (Scorpion 2/3)", GAME_SUPPORTS_SAVE|GAME_REQUIRES_ARTWORK|GAME_MECHANICAL,layout_drwho)
GAMEL( 1994, sc2drwho2	, sc2drwho	,  scorpion2		, drwho		, drwho		, 0,		 "BFM",      "Dr.Who The Timelord (set 3) (Scorpion 2/3)", GAME_SUPPORTS_SAVE|GAME_REQUIRES_ARTWORK|GAME_MECHANICAL,layout_drwho)
GAMEL( 1994, sc2drwho3	, sc2drwho	,  scorpion2		, drwho		, drwho		, 0,		 "BFM",      "Dr.Who The Timelord (set 4) (Scorpion 2/3)", GAME_SUPPORTS_SAVE|GAME_REQUIRES_ARTWORK|GAME_MECHANICAL,layout_drwho)
GAMEL( 1994, sc2drwho4	, sc2drwho	,  scorpion2		, drwho		, drwho		, 0,		 "BFM",      "Dr.Who The Timelord (set 5) (Scorpion 2/3)", GAME_SUPPORTS_SAVE|GAME_REQUIRES_ARTWORK|GAME_MECHANICAL,layout_drwho)
GAMEL( 1994, sc2drwho5	, sc2drwho	,  scorpion2		, drwho		, drwho		, 0,		 "BFM",      "Dr.Who The Timelord (set 6) (Scorpion 2/3)", GAME_SUPPORTS_SAVE|GAME_REQUIRES_ARTWORK|GAME_MECHANICAL,layout_drwho)
GAMEL( 1994, sc2drwho6	, sc2drwho	,  scorpion2		, drwho		, drwho		, 0,		 "BFM",      "Dr.Who The Timelord (set 7) (Scorpion 2/3)", GAME_SUPPORTS_SAVE|GAME_REQUIRES_ARTWORK|GAME_MECHANICAL,layout_drwho)
GAMEL( 1994, sc2drwho7	, sc2drwho	,  scorpion2		, drwho		, drwho		, 0,		 "BFM",      "Dr.Who The Timelord (set 8) (Scorpion 2/3)", GAME_SUPPORTS_SAVE|GAME_REQUIRES_ARTWORK|GAME_MECHANICAL,layout_drwho)
GAMEL( 1994, sc2drwho8	, sc2drwho	,  scorpion2		, drwho		, drwho		, 0,		 "BFM",      "Dr.Who The Timelord (set 9) (Scorpion 2/3)", GAME_SUPPORTS_SAVE|GAME_REQUIRES_ARTWORK|GAME_MECHANICAL,layout_drwho)
GAMEL( 1994, sc2drwho9	, sc2drwho	,  scorpion2		, drwho		, drwho		, 0,		 "BFM",      "Dr.Who The Timelord (set 10) (Scorpion 2/3)", GAME_SUPPORTS_SAVE|GAME_REQUIRES_ARTWORK|GAME_MECHANICAL,layout_drwho)
GAMEL( 1994, sc2drwho10	, sc2drwho	,  scorpion2		, drwho		, drwho		, 0,		 "BFM",      "Dr.Who The Timelord (set 11) (Scorpion 2/3)", GAME_SUPPORTS_SAVE|GAME_REQUIRES_ARTWORK|GAME_MECHANICAL,layout_drwho)
GAMEL( 1994, sc2drwho11	, sc2drwho	,  scorpion2		, drwho		, drwho		, 0,		 "BFM",      "Dr.Who The Timelord (set 12) (Scorpion 2/3)", GAME_SUPPORTS_SAVE|GAME_REQUIRES_ARTWORK|GAME_MECHANICAL,layout_drwho)
GAMEL( 1994, sc2drwho12	, sc2drwho	,  scorpion2		, drwho		, drwho		, 0,		 "BFM",      "Dr.Who The Timelord (set 13) (Scorpion 2/3)", GAME_SUPPORTS_SAVE|GAME_REQUIRES_ARTWORK|GAME_MECHANICAL,layout_drwho)
GAMEL( 1994, sc2drwho13	, sc2drwho	,  scorpion2		, drwho		, drwho		, 0,		 "BFM",      "Dr.Who The Timelord (set 14) (Scorpion 2/3)", GAME_SUPPORTS_SAVE|GAME_REQUIRES_ARTWORK|GAME_MECHANICAL,layout_drwho)
GAMEL( 1994, sc2drwho14	, sc2drwho	,  scorpion2		, drwho		, drwho		, 0,		 "BFM",      "Dr.Who The Timelord (set 15) (Scorpion 2/3)", GAME_SUPPORTS_SAVE|GAME_REQUIRES_ARTWORK|GAME_MECHANICAL,layout_drwho)
GAMEL( 1994, sc2drwho15	, sc2drwho	,  scorpion2		, drwho		, drwho		, 0,		 "BFM",      "Dr.Who The Timelord (set 16) (Scorpion 2/3)", GAME_SUPPORTS_SAVE|GAME_REQUIRES_ARTWORK|GAME_MECHANICAL,layout_drwho)
GAMEL( 1994, sc2drwho16	, sc2drwho	,  scorpion2		, drwho		, drwho		, 0,		 "BFM",      "Dr.Who The Timelord (set 17) (Scorpion 2/3)", GAME_SUPPORTS_SAVE|GAME_REQUIRES_ARTWORK|GAME_MECHANICAL,layout_drwho)
GAMEL( 1994, sc2drwho17	, sc2drwho	,  scorpion2		, drwho		, drwhon	, 0,		 "BFM",      "Dr.Who The Timelord (set 18, not encrypted) (Scorpion 2/3)", GAME_SUPPORTS_SAVE|GAME_REQUIRES_ARTWORK|GAME_MECHANICAL,layout_drwho)
GAMEL( 1994, sc2drwho18	, sc2drwho	,  scorpion2		, drwho		, drwhon	, 0,		 "BFM/Mazooma",      "Dr.Who The Timelord (set 19) (Scorpion 2/3)", GAME_SUPPORTS_SAVE|GAME_REQUIRES_ARTWORK|GAME_MECHANICAL | GAME_NOT_WORKING,layout_drwho) // error 99
GAMEL( 1994, sc2drwho19	, sc2drwho	,  scorpion2		, drwho		, drwhon	, 0,		 "BFM/Mazooma",      "Dr.Who The Timelord (set 20) (Scorpion 2/3)", GAME_SUPPORTS_SAVE|GAME_REQUIRES_ARTWORK|GAME_MECHANICAL | GAME_NOT_WORKING,layout_drwho) // error 99
GAMEL( 1994, sc2drwho20	, sc2drwho	,  scorpion2		, drwho		, drwhon	, 0,		 "BFM",      "Dr.Who The Timelord Deluxe (Scorpion 2/3)", GAME_SUPPORTS_SAVE|GAME_REQUIRES_ARTWORK|GAME_MECHANICAL,layout_drwho)

GAME( 1994, sc2brkfs	, 0			,  scorpion2		, bbrkfst	, bbrkfst	, 0,		 "BFM",      "The Big Breakfast (set 1 UK, Game Card 95-750-524) (Scorpion 2/3)", GAME_FLAGS)
GAME( 1994, sc2brkfs1	, sc2brkfs	,  scorpion2		, bbrkfst	, bbrkfst	, 0,		 "BFM",      "The Big Breakfast (set 2) (Scorpion 2/3)", GAME_FLAGS)
GAME( 1994, sc2brkfs2	, sc2brkfs	,  scorpion2		, bbrkfst	, bbrkfst	, 0,		 "BFM",      "The Big Breakfast (set 3) (Scorpion 2/3)", GAME_FLAGS)
GAME( 1994, sc2brkfs3	, sc2brkfs	,  scorpion2		, bbrkfst	, bbrkfst	, 0,		 "BFM",      "The Big Breakfast (set 4) (Scorpion 2/3)", GAME_FLAGS)
GAME( 1994, sc2brkfs4	, sc2brkfs	,  scorpion2		, bbrkfst	, bbrkfst	, 0,		 "BFM",      "The Big Breakfast (set 5) (Scorpion 2/3)", GAME_FLAGS)
GAME( 1994, sc2brkfs5	, sc2brkfs	,  scorpion2		, bbrkfst	, bbrkfst	, 0,		 "BFM",      "The Big Breakfast (set 6) (Scorpion 2/3)", GAME_FLAGS)
GAME( 1994, sc2brkfsm	, sc2brkfs	,  scorpion2		, bbrkfst	, bbrkfst	, 0,		 "BFM/Mazooma",      "The Big Breakfast Casino (Scorpion 2/3)", GAME_FLAGS)

GAME( 1995, sc2focus	, 0			,  scorpion3		, scorpion3	, focus		, 0,		 "BFM/ELAM", "Focus (Dutch, Game Card 95-750-347) (Scorpion 2/3)", GAME_NOT_WORKING|GAME_SUPPORTS_SAVE|GAME_REQUIRES_ARTWORK|GAME_MECHANICAL)
GAME( 1996, sc2gslam	, 0			,  scorpion2		, bfmcgslm	, bfmcgslm	, 0,		 "BFM",      "Grandslam Club (UK, Game Card 95-750-843) (Scorpion 2/3)", GAME_NOT_WORKING|GAME_SUPPORTS_SAVE|GAME_REQUIRES_ARTWORK|GAME_MECHANICAL)
GAME( 199?, sc2cshcl	, 0			,  scorpion2		, bbrkfst	, bbrkfst	, 0,		 "BFM",      "Cashino Club (Bellfruit) (Scorpion 2/3)", GAME_FLAGS)
GAME( 199?, sc2catms	, 0			,  scorpion2		, bbrkfst	, bbrkfst	, 0,		 "BFM",      "Cat & Mouse (Bellfruit) (Scorpion 2/3)", GAME_FLAGS)
GAME( 199?, sc2eggs		, 0			,  scorpion2		, drwho		, drwho		, 0,		 "BFM",      "Eggs On Legs Tour (Bellfruit) (Scorpion 2/3)", GAME_FLAGS)
GAME( 199?, sc2gsclb	, 0			,  scorpion2		, drwho		, drwho		, 0,		 "BFM",      "The Game Show Club (Bellfruit) (Scorpion 2/3)", GAME_FLAGS)
GAME( 199?, sc2suprz	, 0			,  scorpion2		, drwho		, drwho		, 0,		 "BFM",      "Surprise Surprize (Bellfruit) (Scorpion 2/3)", GAME_FLAGS)
GAME( 199?, sc2cpg		, 0			,  scorpion2		, drwho		, drwho		, 0,		 "BFM",      "Pharaoh's Gold Club (Bellfruit) (Scorpion 2/3)", GAME_FLAGS)
GAME( 199?, sc2motd		, 0			,  scorpion2		, drwho		, drwho		, 0,		 "BFM",      "Match Of The Day (Bellfruit) (Scorpion 2/3)", GAME_FLAGS)
GAME( 199?, sc2easy		, 0			,  scorpion2		, drwho		, drwho		, 0,		 "BFM",      "Easy Money (Bellfruit) (Scorpion 2/3)", GAME_FLAGS)
GAME( 199?, sc2majes	, 0			,  scorpion2		, drwho		, drwho		, 0,		 "BFM",      "Majestic Bells (Bellfruit) (Scorpion 2/3)", GAME_FLAGS)
GAME( 199?, sc2dels		, 0			,  scorpion2		, drwho		, drwho		, 0,		 "BFM",      "Del's Millions (Bellfruit) (Scorpion 2/3)", GAME_FLAGS)
GAME( 199?, sc2wembl	, 0			,  scorpion2		, drwho		, drwho		, 0,		 "BFM",      "Road To Wembley (Bellfruit) (Scorpion 2/3)", GAME_FLAGS)
GAME( 199?, sc2wemblm	, sc2wembl	,  scorpion2		, drwho		, drwho		, 0,		 "BFM/Mazooma",      "Road To Wembley (Bellfruit/Mazooma) (Scorpion 2/3)", GAME_FLAGS) // error 99
GAME( 199?, sc2downt	, 0			,  scorpion2		, drwho		, drwho		, 0,		 "BFM",	     "Down Town (Bellfruit) (Scorpion 2/3)", GAME_FLAGS)
GAME( 199?, sc2inst		, 0			,  scorpion2		, drwho		, drwho		, 0,		 "BFM",      "Instant Jackpot (Bellfruit) (Scorpion 2/3)", GAME_FLAGS)
GAME( 199?, sc2mam		, 0			,  scorpion2		, drwho		, drwho		, 0,		 "BFM",      "Make A Million (Bellfruit) (Scorpion 2/3)", GAME_FLAGS)
GAME( 199?, sc2mamcl	, 0			,  scorpion2		, drwho		, drwho		, 0,		 "BFM",      "Make A Million Club (Bellfruit) (Scorpion 2/3)", GAME_FLAGS)
GAME( 199?, sc2showt	, 0			,  scorpion2		, drwho		, drwho		, 0,		 "BFM",      "Showtime Spectacular (Bellfruit) (Scorpion 2/3)", GAME_FLAGS)
GAME( 199?, sc2sstar	, 0			,  scorpion2		, drwho		, drwho		, 0,		 "BFM",      "Superstar (Bellfruit) (Scorpion 2/3)", GAME_FLAGS)
GAME( 199?, sc2wwcl		, 0			,  scorpion2		, drwho		, drwho		, 0,		 "BFM",      "Wild West Club (Bellfruit) (Scorpion 2/3)", GAME_FLAGS)
// this one is a bit strange (not encrypted, gives 'PROM ERROR 2'), is it really sc2? BFMemulator layout dat says it is
GAME( 199?, sc2pe1g		, 0			,  scorpion2		, drwho		, drwhon	, 0,		 "BFM",      "Public Enemy No.1 (Bellfruit) [German] (Scorpion 2/3)", GAME_FLAGS)

// these need inverted service door, and seem to have some issues with the reels jumping between 2 values?
GAME( 199?, sc2goldr	, 0			,  scorpion2		, drwho		, drwho		, 0,		 "Mdm",      "Gold Reserve (Mdm) (Scorpion 2/3)", GAME_FLAGS)
GAME( 199?, sc2hifly	, 0			,  scorpion2		, drwho		, drwho		, 0,		 "Mdm",      "High Flyer (Mdm) (Scorpion 2/3)", GAME_FLAGS)
GAME( 199?, sc2scc		, 0			,  scorpion2		, drwho		, drwho		, 0,		 "Mdm",      "Safe Cracker Club (Mdm) (Scorpion 2/3)", GAME_FLAGS) // also marked as 'GLOBAL'?

// custom Global sound system?
GAME( 199?, sc2dick		, 0			,  scorpion2		, drwho		, drwho		, 0,		 "Global",   "Spotted Dick (Global) (Scorpion 2/3)", GAME_FLAGS)
GAME( 199?, sc2pick		, 0			,  scorpion2		, drwho		, drwho		, 0,		 "Global",   "Pick Of The Bunch (Global) (Scorpion 2/3)", GAME_FLAGS)
GAME( 199?, sc2rock		, 0			,  scorpion2		, drwho		, drwho		, 0,		 "Global",   "How Big's Your Rock (Global) (Scorpion 2/3)", GAME_FLAGS)
GAME( 199?, sc2call		, 0			,  scorpion2		, drwho		, drwho		, 0,		 "Global",   "It's Your Call (Global) (Scorpion 2/3)", GAME_FLAGS)

GAME( 199?, sc2prom		, 0			,  scorpion2		, drwho		, drwho		, 0,		 "BFM",   "Along The Prom (Bellfruit) (Scorpion 2/3)", GAME_FLAGS)
GAME( 199?, sc2payr		, 0			,  scorpion2		, drwho		, drwho		, 0,		 "BFM/Mazooma",   "Pay Roll Casino (Bellfruit/Mazooma) (Scorpion 2/3)", GAME_FLAGS)

GAME( 199?, sc2bar7		, 0			,  scorpion2		, drwho		, drwho		, 0,		 "Concept",   "Bar 7 (Concept)", GAME_FLAGS)
GAME( 199?, sc2bbar7	, 0			,  scorpion2		, drwho		, drwho		, 0,		 "Concept",   "Big Bar 7 (Concept)", GAME_FLAGS)
GAME( 199?, sc2flutr	, 0			,  scorpion2		, drwho		, drwho		, 0,		 "Concept",   "Flutter (Concept)", GAME_FLAGS)
GAME( 199?, sc2smnud	, 0			,  scorpion2		, drwho		, drwho		, 0,		 "Concept",   "Super Multi Nudger (Concept)", GAME_FLAGS)

//Seems to be plain Scorpion 2 - keeps tripping watchdog?
GAME( 199?, sc2scshx	, 0			,  scorpion2		, drwho		, drwho			, 0,		 "Concept",   "Super Cash X (Concept)", GAME_FLAGS)
GAME( 199?, sc2sghst	, 0			,  scorpion2		, drwho		, drwho			, 0,		 "Concept",   "Super Ghost (Concept)", GAME_FLAGS)
GAME( 199?, sc2scshxgman, sc2scshx	,  scorpion2		, drwho		, drwho			, 0,		 "Concept",   "Super Cash X (Concept) (Gamesman Hardware)", GAME_FLAGS)
GAME( 199?, sc2scshxstar, sc2scshx	,  scorpion2		, drwho		, drwho			, 0,		 "Concept",   "Super Cash X (Concept) (Starpoint Hardware)", GAME_FLAGS)
GAME( 199?, sc2scshxcas,  sc2scshx	,  scorpion2		, drwho		, drwho			, 0,		 "Concept",   "Super Casino Cash X (Concept)", GAME_FLAGS)

GAME( 199?, sc2cgc		, 0			,  scorpion2		, drwho		, drwho		, 0,		 "BFM",   "Carrot Gold Club (Bellfruit) (Scorpion 2/3)", GAME_FLAGS)
GAME( 199?, sc2cnile		, 0			,  scorpion2		, drwho		, drwho		, 0,		 "BFM",   "Cash On The Nile Club (Bellfruit) (Scorpion 2/3)", GAME_FLAGS)
GAME( 199?, sc2casr		, 0			,  scorpion2		, drwho		, drwho		, 0,		 "BFM",   "Casino Royale (Bellfruit) (Scorpion 2/3)", GAME_FLAGS)
GAME( 199?, sc2cmbt		, 0			,  scorpion2		, drwho		, drwho		, 0,		 "BFM",   "Cat & Mouse & Bonzo Too (Bellfruit) (Scorpion 2/3)", GAME_FLAGS)
GAME( 199?, sc2dbl		, 0			,  scorpion2		, drwho		, drwho		, 0,		 "BFM",   "Double Diamond (Bellfruit) (Scorpion 2/3)", GAME_FLAGS)
GAME( 199?, sc2flaca		, 0			,  scorpion2		, drwho		, drwho		, 0,		 "BFM",   "Flash Cash (Bellfruit) (Scorpion 2/3)", GAME_FLAGS)
GAME( 199?, sc2foot		, 0			,  scorpion2		, drwho		, drwho		, 0,		 "BFM",   "Football Club (Bellfruit) (Scorpion 2/3)", GAME_FLAGS)
GAME( 199?, sc2gcclb		, 0			,  scorpion2		, drwho		, drwho		, 0,		 "BFM",   "Golden Casino Club (Bellfruit) (Scorpion 2/3)", GAME_FLAGS)
GAME( 199?, sc2groul		, 0			,  scorpion2		, drwho		, drwho		, 0,		 "BFM",   "Golden Roulette (Bellfruit) (Scorpion 2/3)", GAME_FLAGS)
GAME( 199?, sc2gldsh		, 0			,  scorpion2		, drwho		, drwho		, 0,		 "BFM",   "Golden Shot (Bellfruit) (Scorpion 2/3)", GAME_FLAGS)
GAME( 199?, sc2gtr		, 0			,  scorpion2		, drwho		, drwho		, 0,		 "BFM",   "Great Train Robbery (Bellfruit) (Scorpion 2/3)", GAME_FLAGS)
GAME( 199?, sc2heypr		, 0			,  scorpion2		, drwho		, drwho		, 0,		 "BFM",   "Hey Presto (Bellfruit) (Scorpion 2/3)", GAME_FLAGS)
GAME( 199?, sc2hypr		, 0			,  scorpion2		, drwho		, drwho		, 0,		 "BFM",   "Hyperactive (Bellfruit) (Scorpion 2/3)", GAME_FLAGS)
GAME( 199?, sc2kcclb		, 0			,  scorpion2		, drwho		, drwho		, 0,		 "BFM",   "King Cash Club (Bellfruit) (Scorpion 2/3)", GAME_FLAGS)
GAME( 199?, sc2maina		, 0			,  scorpion2		, drwho		, drwho		, 0,		 "BFM",   "Main Attraction (Bellfruit) (Scorpion 2/3)", GAME_FLAGS)
GAME( 199?, sc2olgld		, 0			,  scorpion2		, drwho		, drwho		, 0,		 "BFM",   "Olympic Gold (Bellfruit) (Scorpion 2/3)", GAME_FLAGS)
GAME( 199?, sc2relgm		, 0			,  scorpion2		, drwho		, drwho		, 0,		 "BFM",   "Reel Gems (Bellfruit) (Scorpion 2/3)", GAME_FLAGS)
GAME( 199?, sc2topwk		, 0			,  scorpion2		, drwho		, drwho		, 0,		 "BFM",   "Top Wack (Bellfruit) (Scorpion 2/3)", GAME_FLAGS)
GAME( 199?, sc2cb7		, 0			,  scorpion2		, drwho		, drwho		, 0,		 "BFM",   "Casino Bar 7 (Bellfruit) (Scorpion 2/3)", GAME_FLAGS)
GAME( 199?, sc2cgcas		, 0			,  scorpion2		, drwho		, drwho		, 0,		 "BFM",   "Club Grand Casino (Bellfruit) (Scorpion 2/3)", GAME_FLAGS)
GAME( 199?, sc2cvega		, 0			,  scorpion2		, drwho		, drwho		, 0,		 "BFM",   "Cash Vegas (Bellfruit) (Scorpion 2/3)", GAME_FLAGS)


// Games with Dot Matrix Displays */

GAME( 1996, sc2luvv		, 0			,  scorpion2_dm01	, luvjub	, luvjub	, 0,		 "BFM",      "Luvvly Jubbly (UK Multisite 10/25p, Game Card 95-750-808) (Scorpion 2/3)", GAME_NOT_WORKING|GAME_REQUIRES_ARTWORK|GAME_MECHANICAL)
GAME( 1996, sc2cpe		, 0			,  scorpion2_dm01	, cpeno1	, cpeno1	, 0,		 "BFM",      "Club Public Enemy No.1 (UK, Game Card 95-750-846) (Scorpion 2/3)", GAME_NOT_WORKING|GAME_REQUIRES_ARTWORK|GAME_MECHANICAL)
GAME( 199?, sc2town		, 0			,  scorpion2_dm01	, drwho		, drwho		, 0,		 "BFM",      "Round The Town (Bellfruit) (Scorpion 2/3)", GAME_FLAGS)
GAME( 199?, sc2ofool	, 0			,  scorpion2_dm01	, drwho		, drwho		, 0,		 "BFM",      "Only Fools & Horses (Bellfruit) (Scorpion 2/3)", GAME_FLAGS)
GAME( 199?, sc2ptytm	, 0			,  scorpion2_dm01	, drwho		, drwho		, 0,		 "BFM",      "Party Time (Bellfruit) (Scorpion 2/3)", GAME_FLAGS)
GAME( 199?, sc2cops		, 0			,  scorpion2_dm01	, drwho		, drwho		, 0,		 "BFM",      "Cops 'n' Robbers (Bellfruit) (Scorpion 2/3)", GAME_FLAGS)
GAME( 199?, sc2copcl	, 0			,  scorpion2_dm01	, drwho		, drwho		, 0,		 "BFM",      "Cops 'n' Robbers Club (Bellfruit) (Scorpion 2/3)", GAME_FLAGS)
GAME( 199?, sc2copdc	, 0			,  scorpion2_dm01	, drwho		, drwho		, 0,		 "BFM",      "Cops 'n' Robbers Club Deluxe (Bellfruit) (Scorpion 2/3)", GAME_FLAGS)
GAME( 199?, sc2prem		, 0			,  scorpion2_dm01	, drwho		, drwho		, 0,		 "BFM",      "Premier Club Manager (Bellfruit) (Scorpion 2/3)", GAME_FLAGS)

