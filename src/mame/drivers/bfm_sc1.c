/*****************************************************************************************

    Bellfruit scorpion1 driver, (under heavy construction !!!)

    M.A.M.E Core Copyright Nicola Salmoria and the MAME Team,
    used under license from http://mamedev.org

******************************************************************************************

     04-2011: J Wallace: Fixed watchdog to match actual circuit, also fixed lamping code.
  20-01-2007: J Wallace: Tidy up of coding
  30-12-2006: J Wallace: Fixed init routines.
  07-03-2006: El Condor: Recoded to more accurately represent the hardware setup.
  19-08-2005: Re-Animator
  16-08-2005: Converted to MAME protocol for when Viper board is completed.
  25-08-2005: Added support for adder2 (Toppoker), added support for NEC upd7759 soundcard

Standard scorpion1 memorymap
Note the similarity to system85 - indeed, the working title for this system was System 88,
and was considered an update to existing technology. Later revisions made it a platform in
its own right, prompting marketers to change the name.

SOME SETS COULD BE IN THE WRONG DRIVER AS A RESULT OF THESE SIMILARITIES!

___________________________________________________________________________________
   hex     |r/w| D D D D D D D D |
 location  |   | 7 6 5 4 3 2 1 0 | function
-----------+---+-----------------+-------------------------------------------------
0000-1FFF  |R/W| D D D D D D D D | RAM (8k) battery backed up
-----------+---+-----------------+-------------------------------------------------
2000-21FF  | W | D D D D D D D D | Reel 3 + 4 stepper latch
-----------+---+-----------------+-------------------------------------------------
2200-23FF  | W | D D D D D D D D | Reel 1 + 2 stepper latch
-----------+---+-----------------+-------------------------------------------------
2400-25FF  | W | D D D D D D D D | vfd + coin inhibits
-----------+---+-----------------+-------------------------------------------------
2600-27FF  | W | D D D D D D D D | electro mechanical meters
-----------+---+-----------------+-------------------------------------------------
2800-28FF  | W | D D D D D D D D | triacs used for payslides/hoppers
-----------+---+-----------------+-------------------------------------------------
2A00       |R/W| D D D D D D D D | MUX1 control reg (IN: mux inputs, OUT:lamps)
-----------+---+-----------------+-------------------------------------------------
2A01       | W | D D D D D D D D | MUX1 low data bits
-----------+---+-----------------+-------------------------------------------------
2A02       | W | D D D D D D D D | MUX1 hi  data bits
-----------+---+-----------------+-------------------------------------------------
2E00       | R | ? ? ? ? ? ? D D | IRQ status
-----------+---+-----------------+-------------------------------------------------
3001       |   | D D D D D D D D | AY-8912 data
-----------+---+-----------------+-------------------------------------------------
3101       | W | D D D D D D D D | AY-8912 address
-----------+---+-----------------+-------------------------------------------------
3406       |R/W| D D D D D D D D | MC6850
-----------+---+-----------------+-------------------------------------------------
3407       |R/W| D D D D D D D D | MC6850
-----------+---+-----------------+-------------------------------------------------
3408       |R/W| D D D D D D D D | MUX2 control reg (IN: reel optos, OUT: lamps)
-----------+---+-----------------+-------------------------------------------------
3409       | W | D D D D D D D D | MUX2 low data bits
-----------+---+-----------------+-------------------------------------------------
340A       | W | D D D D D D D D | MUX2 hi  data bits
-----------+---+-----------------+-------------------------------------------------
3600       | W | ? ? ? ? ? ? D D | ROM page select (select page 3 after reset)
-----------+---+-----------------+-------------------------------------------------
4000-5FFF  | R | D D D D D D D D | ROM (8k)
-----------+---+-----------------+-------------------------------------------------
6000-7FFF  | R | D D D D D D D D | Paged ROM (8k)
           |   |                 |   page 0 : rom area 0x0000 - 0x1FFF
           |   |                 |   page 1 : rom area 0x2000 - 0x3FFF
           |   |                 |   page 2 : rom area 0x4000 - 0x5FFF
           |   |                 |   page 3 : rom area 0x6000 - 0x7FFF
-----------+---+-----------------+-------------------------------------------------
8000-FFFF  | R | D D D D D D D D | ROM (32k)
-----------+---+-----------------+-------------------------------------------------

Optional (on expansion card) (Viper)
-----------+---+-----------------+-------------------------------------------------
3404       | R | D D D D D D D D | Coin acceptor, direct input
-----------+---+-----------------+-------------------------------------------------
3800-38FF  |R/W| D D D D D D D D | NEC uPD7759 soundchip W:control R:status
           |   |                 | normally 0x3801 is used can conflict with reel5+6
-----------+---+-----------------+-------------------------------------------------
3800-38FF  | W | D D D D D D D D | Reel 5 + 6 stepper latch (normally 0x3800 used)
-----------+---+-----------------+-------------------------------------------------

***************************************************************************/

#include "emu.h"
#include "cpu/m6809/m6809.h"
#include "cpu/z80/z80.h"
#include "video/awpvid.h"
#include "video/bfm_adr2.h"
#include "machine/steppers.h" // stepper motor
#include "machine/bfm_bd1.h" // vfd
#include "machine/meters.h"
#include "sound/ay8910.h"
#include "sound/upd7759.h"
#include "machine/nvram.h"
#include "machine/bfm_comn.h"
#include "bfm_sc1.lh"


class bfm_sc1_state : public driver_device
{
public:
	bfm_sc1_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag) { }

	int m_mmtr_latch;
	int m_triac_latch;
	int m_vfd_latch;
	int m_irq_status;
	int m_optic_pattern;
	int m_acia_status;
	int m_locked;
	int m_is_timer_enabled;
	int m_reel_changed;
	int m_coin_inhibits;
	int m_mux1_outputlatch;
	int m_mux1_datalo;
	int m_mux1_datahi;
	int m_mux1_input;
	int m_mux2_outputlatch;
	int m_mux2_datalo;
	int m_mux2_datahi;
	int m_mux2_input;
	UINT8 m_sc1_Inputs[64];
	UINT8 m_codec_data[256];

	int m_defaultbank;
};

#define VFD_RESET  0x20
#define VFD_CLOCK1 0x80
#define VFD_DATA   0x40

#define MASTER_CLOCK	(XTAL_4MHz)
#define ADDER_CLOCK		(XTAL_8MHz)

///////////////////////////////////////////////////////////////////////////

static void Scorpion1_SetSwitchState(bfm_sc1_state *drvstate, int strobe, int data, int state)
{
	if ( state ) drvstate->m_sc1_Inputs[strobe] |=  (1<<data);
	else		 drvstate->m_sc1_Inputs[strobe] &= ~(1<<data);
}

///////////////////////////////////////////////////////////////////////////
#ifdef UNUSED_FUNCTION
static int Scorpion1_GetSwitchState(bfm_sc1_state *drvstate, int strobe, int data)
{
	int state = 0;

	if ( strobe < 7 && data < 8 ) state = (drvstate->sc1_Inputs[strobe] & (1<<data))?1:0;

	return state;
}
#endif
///////////////////////////////////////////////////////////////////////////

static WRITE8_HANDLER( bankswitch_w )
{
//  printf("bankswitch %02x\n", data);
	memory_set_bank(space->machine(),"bank1",data & 0x03);
}

///////////////////////////////////////////////////////////////////////////

static INTERRUPT_GEN( timer_irq )
{
	bfm_sc1_state *state = device->machine().driver_data<bfm_sc1_state>();

	if ( state->m_is_timer_enabled )
	{
		state->m_irq_status = 0x01 |0x02; //0xff;

	    state->m_sc1_Inputs[2] = input_port_read(device->machine(),"STROBE0");

		generic_pulse_irq_line(device->machine().device("maincpu"), M6809_IRQ_LINE);
	}
}

///////////////////////////////////////////////////////////////////////////

static READ8_HANDLER( irqlatch_r )
{
	bfm_sc1_state *state = space->machine().driver_data<bfm_sc1_state>();
	int result = state->m_irq_status | 0x02;

	state->m_irq_status = 0;

	return result;
}

///////////////////////////////////////////////////////////////////////////

static WRITE8_HANDLER( reel12_w )
{
	bfm_sc1_state *state = space->machine().driver_data<bfm_sc1_state>();
	if ( state->m_locked & 0x01 )
	{	// hardware is still locked,
		if ( data == 0x46 ) state->m_locked &= ~0x01;
	}
	else
	{
		if ( stepper_update(0, (data>>4)&0x0f) ) state->m_reel_changed |= 0x01;
		if ( stepper_update(1, data&0x0f   ) ) state->m_reel_changed |= 0x02;

		if ( stepper_optic_state(0) ) state->m_optic_pattern |=  0x01;
		else                          state->m_optic_pattern &= ~0x01;
		if ( stepper_optic_state(1) ) state->m_optic_pattern |=  0x02;
		else                          state->m_optic_pattern &= ~0x02;
	}
	awp_draw_reel(0);
	awp_draw_reel(1);
}

///////////////////////////////////////////////////////////////////////////

static WRITE8_HANDLER( reel34_w )
{
	bfm_sc1_state *state = space->machine().driver_data<bfm_sc1_state>();
	if ( state->m_locked & 0x02 )
	{	// hardware is still locked,
		if ( data == 0x42 ) state->m_locked &= ~0x02;
	}
	else
	{
		if ( stepper_update(2, (data>>4)&0x0f) ) state->m_reel_changed |= 0x04;
		if ( stepper_update(3, data&0x0f   ) ) state->m_reel_changed |= 0x08;

		if ( stepper_optic_state(2) ) state->m_optic_pattern |=  0x04;
		else                          state->m_optic_pattern &= ~0x04;
		if ( stepper_optic_state(3) ) state->m_optic_pattern |=  0x08;
		else                          state->m_optic_pattern &= ~0x08;
	}
	awp_draw_reel(2);
	awp_draw_reel(3);
}

///////////////////////////////////////////////////////////////////////////

static WRITE8_HANDLER( reel56_w )
{
	bfm_sc1_state *state = space->machine().driver_data<bfm_sc1_state>();
	if ( stepper_update(4, (data>>4)&0x0f) ) state->m_reel_changed |= 0x10;
	if ( stepper_update(5, data&0x0f   ) ) state->m_reel_changed |= 0x20;

	if ( stepper_optic_state(4) ) state->m_optic_pattern |=  0x10;
	else                          state->m_optic_pattern &= ~0x10;
	if ( stepper_optic_state(5) ) state->m_optic_pattern |=  0x20;
	else                          state->m_optic_pattern &= ~0x20;
	awp_draw_reel(5);
	awp_draw_reel(6);
}

///////////////////////////////////////////////////////////////////////////
// mechanical meters //////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////

static WRITE8_HANDLER( mmtr_w )
{
	bfm_sc1_state *state = space->machine().driver_data<bfm_sc1_state>();
	int i;
	if ( state->m_locked & 0x04 )
	{	// hardware is still locked,
		state->m_locked &= ~0x04;
	}
	else
	{
		int  changed = state->m_mmtr_latch ^ data;

		state->m_mmtr_latch = data;

		for (i=0; i<8; i++)
		{
			if ( changed & (1 << i) )
			{
				MechMtr_update(i, data & (1 << i) );
				generic_pulse_irq_line(space->machine().device("maincpu"), M6809_FIRQ_LINE);
			}
		}
	}
}

///////////////////////////////////////////////////////////////////////////

static READ8_HANDLER( mmtr_r )
{
	bfm_sc1_state *state = space->machine().driver_data<bfm_sc1_state>();
	return state->m_mmtr_latch;
}

///////////////////////////////////////////////////////////////////////////

static READ8_HANDLER( dipcoin_r )
{
	return input_port_read(space->machine(),"STROBE0") & 0x1F;
}

///////////////////////////////////////////////////////////////////////////

static READ8_DEVICE_HANDLER( nec_r )
{
	return 1;
}

///////////////////////////////////////////////////////////////////////////

static WRITE8_HANDLER( vfd_w )
{
	bfm_sc1_state *state = space->machine().driver_data<bfm_sc1_state>();
	int changed = state->m_vfd_latch ^ data;

	state->m_vfd_latch = data;

	if ( changed )
	{
		if ( changed & VFD_RESET )
		{ // vfd reset line changed
			if ( !(data & VFD_RESET) )
			{ // reset the vfd
				BFM_BD1_reset(0);
				BFM_BD1_reset(1);
				BFM_BD1_reset(2);
			}
		}
		if ( changed & VFD_CLOCK1 )
		{ // clock line changed
			if ( !(data & VFD_CLOCK1) && (data & VFD_RESET) )
			{ // new data clocked into vfd
				BFM_BD1_shift_data(0, data & VFD_DATA );
			}
		}
		BFM_BD1_draw(0);
		BFM_BD1_draw(1);
		BFM_BD1_draw(2);
	}
}

/////////////////////////////////////////////////////////////////////////////////////
// input / output multiplexers //////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////

// conversion table BFM strobe data to internal lamp numbers

static const UINT8 BFM_strcnv[] =
{
	0x00,0x01,0x02,0x03,0x04,0x05,0x06,0x07, 0x80,0x81,0x82,0x83,0x84,0x85,0x86,0x87,
	0x10,0x11,0x12,0x13,0x14,0x15,0x16,0x17, 0x90,0x91,0x92,0x93,0x94,0x95,0x96,0x97,
	0x20,0x21,0x22,0x23,0x24,0x25,0x26,0x27, 0xA0,0xA1,0xA2,0xA3,0xA4,0xA5,0xA6,0xA7,
	0x30,0x31,0x32,0x33,0x34,0x35,0x36,0x37, 0xB0,0xB1,0xB2,0xB3,0xB4,0xB5,0xB6,0xB7,
	0x40,0x41,0x42,0x43,0x44,0x45,0x46,0x47, 0xC0,0xC1,0xC2,0xC3,0xC4,0xC5,0xC6,0xC7,
	0x50,0x51,0x52,0x53,0x54,0x55,0x56,0x57, 0xD0,0xD1,0xD2,0xD3,0xD4,0xD5,0xD6,0xD7,
	0x60,0x61,0x62,0x63,0x64,0x65,0x66,0x67, 0xE0,0xE1,0xE2,0xE3,0xE4,0xE5,0xE6,0xE7,
	0x70,0x71,0x72,0x73,0x74,0x75,0x76,0x77, 0xF0,0xF1,0xF2,0xF3,0xF4,0xF5,0xF6,0xF7,

	0x08,0x09,0x0A,0x0B,0x0C,0x0D,0x0E,0x0F, 0x88,0x89,0x8A,0x8B,0x8C,0x8D,0x8E,0x8F,
	0x18,0x19,0x1A,0x1B,0x1C,0x1D,0x1E,0x1F, 0x98,0x99,0x9A,0x9B,0x9C,0x9D,0x9E,0x9F,
	0x28,0x29,0x2A,0x2B,0x2C,0x2D,0x2E,0x2F, 0xA8,0xA9,0xAA,0xAB,0xAC,0xAD,0xAE,0xAF,
	0x38,0x39,0x3A,0x3B,0x3C,0x3D,0x3E,0x3F, 0xB8,0xB9,0xBA,0xBB,0xBC,0xBD,0xBE,0xBF,
	0x48,0x49,0x4A,0x4B,0x4C,0x4D,0x4E,0x4F, 0xC8,0xC9,0xCA,0xCB,0xCC,0xCD,0xCE,0xCF,
	0x58,0x59,0x5A,0x5B,0x5C,0x5D,0x5E,0x5F, 0xD8,0xD9,0xDA,0xDB,0xDC,0xDD,0xDE,0xDF,
	0x68,0x69,0x6A,0x6B,0x6C,0x6D,0x6E,0x6F, 0xE8,0xE9,0xEA,0xEB,0xEC,0xED,0xEE,0xEF,
	0x78,0x79,0x7A,0x7B,0x7C,0x7D,0x7E,0x7F, 0xF8,0xF9,0xFA,0xFB,0xFC,0xFD,0xFE,0xFF
};

//ACIA helper functions

/////////////////////////////////////////////////////////////////////////////////////

static READ8_HANDLER( mux1latch_r )
{
	bfm_sc1_state *state = space->machine().driver_data<bfm_sc1_state>();
	return state->m_mux1_input;
}

/////////////////////////////////////////////////////////////////////////////////////

static READ8_HANDLER( mux1datlo_r )
{
	return 0;
}

/////////////////////////////////////////////////////////////////////////////////////

static READ8_HANDLER( mux1dathi_r )
{
	return 0;
}

/////////////////////////////////////////////////////////////////////////////////////

static WRITE8_HANDLER( mux1latch_w )
{
	bfm_sc1_state *state = space->machine().driver_data<bfm_sc1_state>();
	int changed = state->m_mux1_outputlatch ^ data;
	static const char *const portnames[] = { "STROBE0", "STROBE1", "STROBE2", "STROBE3", "STROBE4", "STROBE5", "STROBE6", "STROBE7" };
	state->m_mux1_outputlatch = data;

	if ( changed & 0x08 )
	{ // clock changed

		int input_strobe = data & 0x07;
		if ( !(data & 0x08) )
		{ // clock changed to low
			int strobe,	offset,	pattern, i;

			strobe  = data & 0x07;
			offset  = strobe<<4;
			pattern = 0x01;

			for ( i = 0; i < 8; i++ )
			{
				output_set_lamp_value(BFM_strcnv[offset  ], (state->m_mux1_datalo & pattern?1:0) );
				output_set_lamp_value(BFM_strcnv[offset+8], (state->m_mux1_datahi & pattern?1:0) );
				pattern<<=1;
				offset++;
			}

		}

		if ( !(data & 0x08) )
		{
			state->m_sc1_Inputs[ input_strobe ] = input_port_read(space->machine(),portnames[input_strobe]);

			state->m_mux1_input = state->m_sc1_Inputs[ input_strobe ];
		}
	}
}

/////////////////////////////////////////////////////////////////////////////////////

static WRITE8_HANDLER( mux1datlo_w )
{
	bfm_sc1_state *state = space->machine().driver_data<bfm_sc1_state>();
	state->m_mux1_datalo = data;
}

/////////////////////////////////////////////////////////////////////////////////////

static WRITE8_HANDLER( mux1dathi_w )
{
	bfm_sc1_state *state = space->machine().driver_data<bfm_sc1_state>();
	state->m_mux1_datahi = data;
}

/////////////////////////////////////////////////////////////////////////////////////

static READ8_HANDLER( mux2latch_r )
{
	bfm_sc1_state *state = space->machine().driver_data<bfm_sc1_state>();
	return state->m_mux2_input;
}

/////////////////////////////////////////////////////////////////////////////////////

static READ8_HANDLER( mux2datlo_r )
{
	return 0;
}

/////////////////////////////////////////////////////////////////////////////////////

static READ8_HANDLER( mux2dathi_r )
{
	return 0;
}

/////////////////////////////////////////////////////////////////////////////////////

static WRITE8_HANDLER( mux2latch_w )
{
	bfm_sc1_state *state = space->machine().driver_data<bfm_sc1_state>();
	int changed = state->m_mux2_outputlatch ^ data;

	state->m_mux2_outputlatch = data;

	if ( changed & 0x08 )
	{ // clock changed

		if ( !(data & 0x08) )
		{ // clock changed to low
			int strobe, offset, pattern, i;

			strobe  = data & 0x07;
			offset  = 128+(strobe<<4);
			pattern = 0x01;

			for ( i = 0; i < 8; i++ )
			{
				output_set_lamp_value(BFM_strcnv[offset  ], (state->m_mux2_datalo & pattern?1:0) );
				output_set_lamp_value(BFM_strcnv[offset+8], (state->m_mux2_datahi & pattern?1:0) );
				pattern<<=1;
				offset++;
			}
		}

		if ( !(data & 0x08) )
		{
			state->m_mux2_input = 0x3F ^ state->m_optic_pattern;
		}
	}
}

/////////////////////////////////////////////////////////////////////////////////////

static WRITE8_HANDLER( mux2datlo_w )
{
	bfm_sc1_state *state = space->machine().driver_data<bfm_sc1_state>();
	state->m_mux2_datalo = data;
}

/////////////////////////////////////////////////////////////////////////////////////

static WRITE8_HANDLER( mux2dathi_w )
{
	bfm_sc1_state *state = space->machine().driver_data<bfm_sc1_state>();
	state->m_mux2_datahi = data;
}

/////////////////////////////////////////////////////////////////////////////////////
// serial port //////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////

static WRITE8_HANDLER( aciactrl_w )
{
}

/////////////////////////////////////////////////////////////////////////////////////

static WRITE8_HANDLER( aciadata_w )
{
}

/////////////////////////////////////////////////////////////////////////////////////

static READ8_HANDLER( aciastat_r )
{
	bfm_sc1_state *state = space->machine().driver_data<bfm_sc1_state>();
	return state->m_acia_status;
}

/////////////////////////////////////////////////////////////////////////////////////

static READ8_HANDLER( aciadata_r )
{
	return 0;
}

/////////////////////////////////////////////////////////////////////////////////////
// payslide triacs //////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////

static WRITE8_HANDLER( triac_w )
{
	bfm_sc1_state *state = space->machine().driver_data<bfm_sc1_state>();
	state->m_triac_latch = data;
}

/////////////////////////////////////////////////////////////////////////////////////

static READ8_HANDLER( triac_r )
{
	bfm_sc1_state *state = space->machine().driver_data<bfm_sc1_state>();
	return state->m_triac_latch;
}

/////////////////////////////////////////////////////////////////////////////////////
#ifdef UNUSED_FUNCTION
static WRITE8_DEVICE_HANDLER( nec_reset_w )
{
	upd7759_start_w(device, 0);
	upd7759_reset_w(device, data);
}
#endif
/////////////////////////////////////////////////////////////////////////////////////
static WRITE8_DEVICE_HANDLER( nec_latch_w )
{
	upd7759_port_w (device, 0, data&0x3F);	// setup sample
	upd7759_start_w(device, 0);
	upd7759_start_w(device, 1);			// start
}

/////////////////////////////////////////////////////////////////////////////////////

static WRITE8_HANDLER( vid_uart_tx_w )
{
	adder2_send(data);
	cputag_set_input_line(space->machine(), "adder2", M6809_IRQ_LINE, ASSERT_LINE );//HOLD_LINE);// trigger IRQ
}

///////////////////////////////////////////////////////////////////////////

static WRITE8_HANDLER( vid_uart_ctrl_w )
{
}

///////////////////////////////////////////////////////////////////////////

static READ8_HANDLER( vid_uart_rx_r )
{
	return adder2_receive();
}

///////////////////////////////////////////////////////////////////////////

static READ8_HANDLER( vid_uart_ctrl_r )
{
	return adder2_status();
}


// machine start (called only once) /////////////////////////////////////////////////

static MACHINE_RESET( bfm_sc1 )
{
	bfm_sc1_state *state = machine.driver_data<bfm_sc1_state>();
	BFM_BD1_init(0);
	state->m_vfd_latch         = 0;
	state->m_mmtr_latch        = 0;
	state->m_triac_latch       = 0;
	state->m_irq_status        = 0;
	state->m_is_timer_enabled  = 1;
	state->m_coin_inhibits     = 0;
	state->m_mux1_outputlatch  = 0x08;	// clock HIGH
	state->m_mux1_datalo       = 0;
	state->m_mux1_datahi		  = 0;
	state->m_mux1_input        = 0;
	state->m_mux2_outputlatch  = 0x08;	// clock HIGH
	state->m_mux2_datalo       = 0;
	state->m_mux2_datahi		  = 0;
	state->m_mux2_input        = 0;

	BFM_BD1_reset(0);	// reset display1
	BFM_BD1_reset(1);	// reset display2
	BFM_BD1_reset(2);	// reset display3

// reset stepper motors /////////////////////////////////////////////////////////////
	{
		int pattern =0, i;

		for ( i = 0; i < 6; i++)
		{
			stepper_reset_position(i);
			if ( stepper_optic_state(i) ) pattern |= 1<<i;
		}

		state->m_optic_pattern = pattern;

	}

	state->m_acia_status   = 0x02; // MC6850 transmit buffer empty !!!
	state->m_locked		  = 0x07; // hardware is locked

// init rom bank ////////////////////////////////////////////////////////////////////
	{
		UINT8 *rom = machine.region("maincpu")->base();

		memory_configure_bank(machine,"bank1", 0, 4, &rom[0x0000], 0x02000);
		memory_set_bank(machine,"bank1",state->m_defaultbank);
	}
}

/////////////////////////////////////////////////////////////////////////////////////
// scorpion1 board memory map ///////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////

static ADDRESS_MAP_START( sc1_base, AS_PROGRAM, 8 )

	AM_RANGE(0x0000, 0x1FFF) AM_RAM AM_SHARE("nvram") //8k RAM
	AM_RANGE(0x2000, 0x21FF) AM_WRITE(reel34_w)				// reel 2+3 latch
	AM_RANGE(0x2200, 0x23FF) AM_WRITE(reel12_w)				// reel 1+2 latch
	AM_RANGE(0x2400, 0x25FF) AM_WRITE(vfd_w)				// vfd latch

	AM_RANGE(0x2600, 0x27FF) AM_READWRITE(mmtr_r,mmtr_w)	// mechanical meters
	AM_RANGE(0x2800, 0x2800) AM_READWRITE(triac_r,triac_w)	// payslide triacs

	AM_RANGE(0x2A00, 0x2A00) AM_READWRITE(mux1latch_r,mux1latch_w) // mux1
	AM_RANGE(0x2A01, 0x2A01) AM_READWRITE(mux1datlo_r,mux1datlo_w)
	AM_RANGE(0x2A02, 0x2A02) AM_READWRITE(mux1dathi_r,mux1dathi_w)

	AM_RANGE(0x2E00, 0x2E00) AM_READ(irqlatch_r)			// irq latch

	AM_RANGE(0x3001, 0x3001) AM_READ(soundlatch_r)
	AM_RANGE(0x3001, 0x3001) AM_DEVWRITE("aysnd", ay8910_data_w)
	AM_RANGE(0x3101, 0x3201) AM_DEVWRITE("aysnd", ay8910_address_w)

	AM_RANGE(0x3406, 0x3406) AM_READWRITE(aciastat_r,aciactrl_w)  // MC6850 status register
	AM_RANGE(0x3407, 0x3407) AM_READWRITE(aciadata_r,aciadata_w)  // MC6850 data register

	AM_RANGE(0x3408, 0x3408) AM_READWRITE(mux2latch_r,mux2latch_w) // mux2
	AM_RANGE(0x3409, 0x3409) AM_READWRITE(mux2datlo_r,mux2datlo_w)
	AM_RANGE(0x340A, 0x340A) AM_READWRITE(mux2dathi_r,mux2dathi_w)

	AM_RANGE(0x3600, 0x3600) AM_WRITE(bankswitch_w) 		// write bank
	AM_RANGE(0x3800, 0x39FF) AM_WRITE(reel56_w)				// reel 5+6 latch

	AM_RANGE(0x4000, 0x5FFF) AM_ROM							// 8k  ROM
	AM_RANGE(0x6000, 0x7FFF) AM_ROMBANK("bank1")					// 8k  paged ROM (4 pages)
	AM_RANGE(0x8000, 0xFFFF) AM_ROM AM_WRITE (watchdog_reset_w) // 32k ROM

ADDRESS_MAP_END

/////////////////////////////////////////////////////////////////////////////////////
// scorpion1 board + adder2 expansion memory map ////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////

static ADDRESS_MAP_START( sc1_adder2, AS_PROGRAM, 8 )
	AM_IMPORT_FROM( sc1_base )

	AM_RANGE(0x3E00, 0x3E00) AM_READWRITE(vid_uart_ctrl_r,vid_uart_ctrl_w)	// video uart control reg read
	AM_RANGE(0x3E01, 0x3E01) AM_READWRITE(vid_uart_rx_r,vid_uart_tx_w)		// video uart receive  reg
ADDRESS_MAP_END


/////////////////////////////////////////////////////////////////////////////////////
// scorpion1 board + upd7759 soundcard memory map ///////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////

static ADDRESS_MAP_START( sc1_viper, AS_PROGRAM, 8 )
	AM_IMPORT_FROM( sc1_base )

	AM_RANGE(0x3404, 0x3404) AM_READ(dipcoin_r ) // coin input on gamecard
	AM_RANGE(0x3801, 0x3801) AM_DEVREAD("upd", nec_r)
	AM_RANGE(0x3800, 0x39FF) AM_DEVWRITE("upd", nec_latch_w)
ADDRESS_MAP_END

// input ports for scorpion1 board //////////////////////////////////////////////////

static INPUT_PORTS_START( scorpion1 )

	PORT_START("STROBE0")
    PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_COIN1 ) PORT_IMPULSE(3)
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_COIN2 ) PORT_IMPULSE(3)
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_COIN3 ) PORT_IMPULSE(3)
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_COIN4 ) PORT_IMPULSE(3)
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_COIN5 ) PORT_IMPULSE(3)
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_SERVICE ) PORT_NAME("Green Test")
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_SERVICE ) PORT_NAME("Red Test")
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_UNKNOWN )

	PORT_START("STROBE1")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_UNKNOWN )

	PORT_START("STROBE2")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_START1 )
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_BUTTON1 )
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_BUTTON2 )
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_BUTTON3 )
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_BUTTON4 )
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_BUTTON5 ) // collect?
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_BUTTON6 )
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_BUTTON7 )

	PORT_START("STROBE3")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_BUTTON8 ) // service?
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_BUTTON9 )
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_BUTTON10 )
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_BUTTON11 )
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_BUTTON12 )
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_BUTTON13 )
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_BUTTON14 )
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_BUTTON15 )

	PORT_START("STROBE4")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_UNKNOWN )

	PORT_START("STROBE5")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_UNKNOWN )

	PORT_START("STROBE6")
	PORT_DIPNAME( 0x01, 0x00, "DIL01" )PORT_DIPLOCATION("DIL:01")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x01, DEF_STR( On  ) )
	PORT_DIPNAME( 0x02, 0x00, "DIL02" )PORT_DIPLOCATION("DIL:02")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x02, DEF_STR( On  ) )
	PORT_DIPNAME( 0x04, 0x00, "DIL03" )PORT_DIPLOCATION("DIL:03")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x04, DEF_STR( On  ) )
	PORT_DIPNAME( 0x08, 0x00, "Coin Acceptor" )PORT_DIPLOCATION("DIL:04")
	PORT_DIPSETTING(    0x00, "Mars" )
	PORT_DIPSETTING(    0x08, "Sentinel" )
	PORT_DIPNAME( 0x10, 0x00, "DIL05" )PORT_DIPLOCATION("DIL:05")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x10, DEF_STR( On  ) )
	PORT_DIPNAME( 0x20, 0x00, "Coin Lockout" )PORT_DIPLOCATION("DIL:06")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x20, DEF_STR( On  ) )
	PORT_DIPNAME( 0x40, 0x00, "Cashpot Frequency?" )PORT_DIPLOCATION("DIL:07")
	PORT_DIPSETTING(    0x00, DEF_STR( High ))
	PORT_DIPSETTING(    0x40, DEF_STR( Low ))
	PORT_DIPNAME( 0x80, 0x00, "DIL08" )PORT_DIPLOCATION("DIL:08")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x80, DEF_STR( On  ) )

	PORT_START("STROBE7")
	PORT_DIPNAME( 0x01, 0x00, "DIL09" )PORT_DIPLOCATION("DIL:09")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x01, DEF_STR( On  ) )
	PORT_DIPNAME( 0x02, 0x00, "Coin Jam Alarm" )PORT_DIPLOCATION("DIL:10")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x02, DEF_STR( On  ) )
	PORT_DIPNAME( 0x04, 0x00, "DIL11" )PORT_DIPLOCATION("DIL:11")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x04, DEF_STR( On  ) )
	PORT_DIPNAME( 0x08, 0x00, "DIL12" )PORT_DIPLOCATION("DIL:12")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x08, DEF_STR( On  ) )
	PORT_DIPNAME( 0x10, 0x00, "DIL13" )PORT_DIPLOCATION("DIL:13")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x10, DEF_STR( On  ) )
	PORT_DIPNAME( 0xe0, 0x00, "Payout Percentage?" )PORT_DIPLOCATION("DIL:14,15,16")
	PORT_DIPSETTING(    0x60, "72%")
	PORT_DIPSETTING(    0x20, "75%")
	PORT_DIPSETTING(    0x00, "78%")
	PORT_DIPSETTING(    0x80, "81%")
	PORT_DIPSETTING(    0xc0, "85%")

INPUT_PORTS_END

// input ports for scorpion1 board ////////////////////////////////////////

static INPUT_PORTS_START( clatt )
	PORT_START("STROBE0")
    PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_COIN1 ) PORT_IMPULSE(3) PORT_NAME("10p")
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_COIN2 ) PORT_IMPULSE(3) PORT_NAME("20p")
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_COIN3 ) PORT_IMPULSE(3) PORT_NAME("50p")
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_COIN4 ) PORT_IMPULSE(3) PORT_NAME("1 Pound")
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_SERVICE ) PORT_NAME("Green Test")
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_SERVICE ) PORT_NAME("Red Test")
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_UNKNOWN )

	PORT_START("STROBE1")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_NAME("Cancel")
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_BUTTON2 ) PORT_NAME("Hold 1")
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_BUTTON3 ) PORT_NAME("Hold 2")
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_BUTTON4 ) PORT_NAME("Hold 3")
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_BUTTON5 ) PORT_NAME("Hold 4")
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_BUTTON6 ) PORT_NAME("Exchange")
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_UNKNOWN )

	PORT_START("STROBE2")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_BUTTON7 )	PORT_NAME("Stop/Gamble")
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_START1  )	PORT_NAME("Start")
	PORT_BIT( 0x04, IP_ACTIVE_LOW,  IPT_INTERLOCK )	PORT_NAME("Cashbox Door") PORT_CODE(KEYCODE_Q) PORT_TOGGLE
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_SERVICE )	PORT_NAME("Refill Key")   PORT_CODE(KEYCODE_R) PORT_TOGGLE
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_INTERLOCK )	PORT_NAME("Front Door")   PORT_CODE(KEYCODE_W) PORT_TOGGLE
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_UNKNOWN )

	PORT_START("STROBE3")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_UNKNOWN )

	PORT_START("STROBE4")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_UNKNOWN )

	PORT_START("STROBE5")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_UNKNOWN )

	PORT_START("STROBE6")
	PORT_DIPNAME( 0x01, 0x00, "DIL01" ) PORT_DIPLOCATION("DIL:01")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x01, DEF_STR( On  ) )
	PORT_DIPNAME( 0x02, 0x00, "DIL02" )PORT_DIPLOCATION("DIL:02")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x02, DEF_STR( On  ) )
	PORT_DIPNAME( 0x04, 0x00, "DIL03" )PORT_DIPLOCATION("DIL:03")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x04, DEF_STR( On  ) )
	PORT_DIPNAME( 0x08, 0x00, "Coin Acceptor" )PORT_DIPLOCATION("DIL:04")
	PORT_DIPSETTING(    0x00, "Mars" )
	PORT_DIPSETTING(    0x08, "Sentinel" )
	PORT_DIPNAME( 0x10, 0x00, "DIL05" )PORT_DIPLOCATION("DIL:05")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x10, DEF_STR( On  ) )
	PORT_DIPNAME( 0x20, 0x00, "Coin Lockout" )PORT_DIPLOCATION("DIL:06")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x20, DEF_STR( On  ) )
	PORT_DIPNAME( 0x40, 0x00, "Cashpot Frequency?" )PORT_DIPLOCATION("DIL:07")
	PORT_DIPSETTING(    0x00, DEF_STR( High ))
	PORT_DIPSETTING(    0x40, DEF_STR( Low ))
	PORT_DIPNAME( 0x80, 0x00, "DIL08" )PORT_DIPLOCATION("DIL:08")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x80, DEF_STR( On  ) )

	PORT_START("STROBE7")
	PORT_DIPNAME( 0x01, 0x00, "DIL09" )PORT_DIPLOCATION("DIL:09")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x01, DEF_STR( On  ) )
	PORT_DIPNAME( 0x02, 0x00, "Coin Jam Alarm" )PORT_DIPLOCATION("DIL:10")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x02, DEF_STR( On  ) )
	PORT_DIPNAME( 0x04, 0x00, "DIL11" )PORT_DIPLOCATION("DIL:11")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x04, DEF_STR( On  ) )
	PORT_DIPNAME( 0x08, 0x00, "DIL12" )PORT_DIPLOCATION("DIL:12")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x08, DEF_STR( On  ) )
	PORT_DIPNAME( 0x10, 0x00, "DIL13" )PORT_DIPLOCATION("DIL:13")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x10, DEF_STR( On  ) )
	PORT_DIPNAME( 0xe0, 0x00, "Payout Percentage?" )PORT_DIPLOCATION("DIL:14,15,16")
	PORT_DIPSETTING(    0x60, "72%")
	PORT_DIPSETTING(    0x20, "75%")
	PORT_DIPSETTING(    0x00, "78%")
	PORT_DIPSETTING(    0x80, "81%")
	PORT_DIPSETTING(    0xc0, "85%")

INPUT_PORTS_END

static INPUT_PORTS_START( toppoker )
	PORT_START("STROBE0")
    PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_COIN1 ) PORT_IMPULSE(3) PORT_NAME("Fl 5.00")
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_COIN2 ) PORT_IMPULSE(3) PORT_NAME("Fl 2.50")
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_COIN3 ) PORT_IMPULSE(3) PORT_NAME("Fl 1.00")
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_COIN4 ) PORT_IMPULSE(3) PORT_NAME("Fl 0.25")
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_SERVICE ) PORT_NAME("Green Test")
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_SERVICE ) PORT_NAME("Red Test")
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_UNKNOWN )

	PORT_START("STROBE1")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_BUTTON1) PORT_NAME("Vast 1")
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_BUTTON2) PORT_NAME("Vast 2/Kop")
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_BUTTON3) PORT_NAME("Vast 3/Munt")
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_BUTTON4) PORT_NAME("Herstellen/Neem Win")
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_BUTTON5) PORT_NAME("Verander/Inzet")
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_BUTTON6) PORT_NAME("Neem Club Win")
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_BUTTON7) PORT_NAME("Narr Club/Deal")
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_UNUSED)

	PORT_START("STROBE2")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_START1)
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_BUTTON8) PORT_NAME("Neem Feature")
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_BUTTON9) PORT_NAME("Neem Club Meter")
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_BUTTON10) PORT_NAME("Neem Win Bank")
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_BUTTON11) PORT_NAME("Uitbetalen")
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_BUTTON12) PORT_NAME("Vast Monitor 1")
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("STROBE3")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_INTERLOCK)PORT_NAME("Back Door") PORT_CODE(KEYCODE_Q) PORT_TOGGLE
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_SERVICE) PORT_NAME("Slide Dump") PORT_CODE(KEYCODE_W) PORT_TOGGLE
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_SERVICE) PORT_NAME("Refill Key") PORT_CODE(KEYCODE_R) PORT_TOGGLE
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_SERVICE) PORT_NAME("Meter Key") PORT_CODE(KEYCODE_E) PORT_TOGGLE
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("STROBE4")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_SPECIAL )//Tube status Low switch for 1 Pound
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_BUTTON13) PORT_NAME("Vast Monitor 2")
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_BUTTON14) PORT_NAME("Vast Monitor 3")
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_BUTTON15) PORT_NAME("Vast Monitor 4")
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_INTERLOCK)PORT_NAME("Cashbox Door") PORT_CODE(KEYCODE_W) PORT_TOGGLE
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_BUTTON16) PORT_NAME("Vast Monitor 5")
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
	PORT_DIPNAME( 0x01, 0x00, "DIL01" ) PORT_DIPLOCATION("DIL:01")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x01, DEF_STR( On  ) )
	PORT_DIPNAME( 0x02, 0x00, "DIL02" )PORT_DIPLOCATION("DIL:02")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x02, DEF_STR( On  ) )
	PORT_DIPNAME( 0x04, 0x00, "DIL03" )PORT_DIPLOCATION("DIL:03")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x04, DEF_STR( On  ) )
	PORT_DIPNAME( 0x08, 0x00, "Coin Acceptor" )PORT_DIPLOCATION("DIL:04")
	PORT_DIPSETTING(    0x00, "Mars" )
	PORT_DIPSETTING(    0x08, "Sentinel" )
	PORT_DIPNAME( 0x10, 0x00, "DIL05" )PORT_DIPLOCATION("DIL:05")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x10, DEF_STR( On  ) )
	PORT_DIPNAME( 0x20, 0x00, "Coin Lockout" )PORT_DIPLOCATION("DIL:06")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x20, DEF_STR( On  ) )
	PORT_DIPNAME( 0x40, 0x00, "Cashpot Frequency?" )PORT_DIPLOCATION("DIL:07")
	PORT_DIPSETTING(    0x00, DEF_STR( High ))
	PORT_DIPSETTING(    0x40, DEF_STR( Low ))
	PORT_DIPNAME( 0x80, 0x00, "DIL08" )PORT_DIPLOCATION("DIL:08")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x80, DEF_STR( On  ) )

	PORT_START("STROBE7")
	PORT_DIPNAME( 0x01, 0x00, "DIL09" )PORT_DIPLOCATION("DIL:09")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x01, DEF_STR( On  ) )
	PORT_DIPNAME( 0x02, 0x00, "Coin Jam Alarm" )PORT_DIPLOCATION("DIL:10")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x02, DEF_STR( On  ) )
	PORT_DIPNAME( 0x04, 0x00, "DIL11" )PORT_DIPLOCATION("DIL:11")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x04, DEF_STR( On  ) )
	PORT_DIPNAME( 0x08, 0x00, "DIL12" )PORT_DIPLOCATION("DIL:12")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x08, DEF_STR( On  ) )
	PORT_DIPNAME( 0x10, 0x00, "DIL13" )PORT_DIPLOCATION("DIL:13")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x10, DEF_STR( On  ) )
	PORT_DIPNAME( 0xe0, 0x00, "Payout Percentage?" )PORT_DIPLOCATION("DIL:14,15,16")
	PORT_DIPSETTING(    0x60, "72%")
	PORT_DIPSETTING(    0x20, "75%")
	PORT_DIPSETTING(    0x00, "78%")
	PORT_DIPSETTING(    0x80, "81%")
	PORT_DIPSETTING(    0xc0, "85%")

INPUT_PORTS_END

/////////////////////////////////////////////////////////////////////////////////////
// machine driver for scorpion1 board ///////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////

static MACHINE_CONFIG_START( scorpion1, bfm_sc1_state )
	MCFG_MACHINE_RESET(bfm_sc1)							// main scorpion1 board initialisation
	MCFG_CPU_ADD("maincpu", M6809, MASTER_CLOCK/4)			// 6809 CPU at 1 Mhz
	MCFG_CPU_PROGRAM_MAP(sc1_base)						// setup read and write memorymap
	MCFG_CPU_PERIODIC_INT(timer_irq, 1000 )				// generate 1000 IRQ's per second
	MCFG_WATCHDOG_TIME_INIT(PERIOD_OF_555_MONOSTABLE(120000,100e-9))

	MCFG_SPEAKER_STANDARD_MONO("mono")
	MCFG_SOUND_ADD("aysnd",AY8912, MASTER_CLOCK/4)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.25)

	MCFG_NVRAM_ADD_0FILL("nvram")
	MCFG_DEFAULT_LAYOUT(layout_awpvid14)
MACHINE_CONFIG_END

/////////////////////////////////////////////////////////////////////////////////////
// machine driver for scorpion1 board + adder2 extension ////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////

static MACHINE_CONFIG_DERIVED( scorpion1_adder2, scorpion1 )

	MCFG_CPU_MODIFY("maincpu")
	MCFG_CPU_PROGRAM_MAP(sc1_adder2)				// setup read and write memorymap

	MCFG_DEFAULT_LAYOUT(layout_bfm_sc1)
	MCFG_SCREEN_ADD("adder", RASTER)
	MCFG_SCREEN_FORMAT(BITMAP_FORMAT_INDEXED16)
	MCFG_SCREEN_REFRESH_RATE(50)
	MCFG_SCREEN_SIZE( 400, 300)
	MCFG_SCREEN_VISIBLE_AREA(  0, 400-1, 0, 300-1)
	MCFG_SCREEN_UPDATE(adder2)

	MCFG_VIDEO_START( adder2)
	MCFG_VIDEO_RESET( adder2)

	MCFG_PALETTE_LENGTH(16)

	MCFG_PALETTE_INIT(adder2)
	MCFG_GFXDECODE(adder2)

	MCFG_CPU_ADD("adder2", M6809, ADDER_CLOCK/4 )		// adder2 board 6809 CPU at 2 Mhz
	MCFG_CPU_PROGRAM_MAP(adder2_memmap)				// setup adder2 board memorymap
	MCFG_CPU_VBLANK_INT("adder",adder2_vbl)				// board has a VBL IRQ
MACHINE_CONFIG_END

/////////////////////////////////////////////////////////////////////////////////////
// machine driver for scorpion1 board ///////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////

static MACHINE_CONFIG_DERIVED( scorpion1_viper, scorpion1 )
	MCFG_CPU_MODIFY("maincpu")
	MCFG_CPU_PROGRAM_MAP(sc1_viper)					// setup read and write memorymap

	MCFG_SOUND_ADD("upd",UPD7759, UPD7759_STANDARD_CLOCK)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.50)
MACHINE_CONFIG_END


static void sc1_common_init(running_machine &machine, int reels, int decrypt, int defaultbank)
{
	bfm_sc1_state *state = machine.driver_data<bfm_sc1_state>();
	UINT8 i;

	memset(state->m_sc1_Inputs, 0, sizeof(state->m_sc1_Inputs));

	// setup n default 96 half step reels ///////////////////////////////////////////
	for ( i = 0; i < reels; i++ )
	{
		stepper_config(machine, i, &starpoint_interface_48step);
	}
	if (decrypt) bfm_decode_mainrom(machine,"maincpu", state->m_codec_data);	// decode main rom
	if (reels)
	{
		awp_reel_setup();
	}


	state->m_defaultbank = defaultbank;

}

static DRIVER_INIT(toppoker)
{
	sc1_common_init(machine,3,1, 3);
	adder2_decode_char_roms(machine);	// decode GFX roms
	MechMtr_config(machine,8);

	BFM_BD1_init(0);
}

static DRIVER_INIT(lotse)
{
	sc1_common_init(machine,6,1, 3);
	MechMtr_config(machine,8);

	BFM_BD1_init(0);
	BFM_BD1_init(1);
}

static DRIVER_INIT(lotse_bank0)
{
	sc1_common_init(machine,6,1, 0);
	MechMtr_config(machine,8);

	BFM_BD1_init(0);
	BFM_BD1_init(1);
}


static DRIVER_INIT(nocrypt)
{
	sc1_common_init(machine,6,0, 3);
	MechMtr_config(machine,8);

	BFM_BD1_init(0);
	BFM_BD1_init(1);
}

static DRIVER_INIT(nocrypt_bank0)
{
	sc1_common_init(machine,6,0, 0);
	MechMtr_config(machine,8);

	BFM_BD1_init(0);
	BFM_BD1_init(1);
}


/////////////////////////////////////////////////////////////////////////////////////

static DRIVER_INIT(rou029)
{
	sc1_common_init(machine,6,0, 3);
	MechMtr_config(machine,8);

	BFM_BD1_init(0);
}

/////////////////////////////////////////////////////////////////////////////////////

static DRIVER_INIT(clatt)
{
	bfm_sc1_state *state = machine.driver_data<bfm_sc1_state>();
	sc1_common_init(machine,6,1, 3);
	MechMtr_config(machine,8);

	BFM_BD1_init(0);

	Scorpion1_SetSwitchState(state,3,2,1);
	Scorpion1_SetSwitchState(state,3,3,1);
	Scorpion1_SetSwitchState(state,3,6,1);
	Scorpion1_SetSwitchState(state,4,1,1);
}


// ROM definition ///////////////////////////////////////////////////////////////////

ROM_START( sc1lotus )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "lotusse.bin",  0x0000, 0x10000,  CRC(636dadc4) SHA1(85bad5d76dac028fe9f3303dd09e8266aba7db4d))

	ROM_REGION( 0x10000, "altrevs", 0 )
	ROM_LOAD( "lotusse_.bin", 0x0000, 0x010000, CRC(e5f51a36) SHA1(9cddf757c1636911fce370168e636ffcff7bfab6) )
ROM_END

/////////////////////////////////////////////////////////////////////////////////////

ROM_START( sc1roul )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "rou029.bin",   0x8000, 0x08000,  CRC(31723f0A) SHA1(e220976116a0aaf24dc0c4af78a9311a360e8104))
ROM_END

/////////////////////////////////////////////////////////////////////////////////////

ROM_START( sc1clatt )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "39370196.1",   0x8000, 0x08000,  CRC(4c2e465f) SHA1(101939d37d9c033f6d1dfb83b4beb54e4061aec2))
	ROM_LOAD( "39370196.2",   0x0000, 0x08000,  CRC(c809c22d) SHA1(fca7515bc84d432150ffe5e32fccc6aed458b8b0))
ROM_END

ROM_START( sc1clatta )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "club attraction 393717552 prom a.bin", 0x8000, 0x008000, CRC(795e93cf) SHA1(017fa5ea3d9ad1f7a7a619d88a5892a9ffe6f3bc) )
	ROM_LOAD( "club attraction 393717553 prom b.bin", 0x0000, 0x008000, CRC(06f41627) SHA1(0e54314147a5f0d833d83f6f0ee828bd1c875f3e) )
ROM_END



/////////////////////////////////////////////////////////////////////////////////////

ROM_START( m_tppokr )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "95750899.bin", 0x0000, 0x10000,  CRC(639d1d62) SHA1(80620c14bf9f953588555510fc2e6e930140923f))

	ROM_REGION( 0x20000, "adder2", 0 )
	ROM_LOAD( "tpk010.vid", 0x0000, 0x20000,  CRC(ea4eddca) SHA1(5fb805d35376ec7ee8d58684e584621dbb2b2a9c))

	ROM_REGION( 0x40000, "gfx1", ROMREGION_ERASEFF )
	ROM_LOAD( "tpk011.chr",	0x00000, 0x20000,  CRC(4dc23ad8) SHA1(8e8cc699412dbb092e16e14518f407353f477ee1))
ROM_END

/////////////////////////////////////////////////////////////////////////////////////

ROM_START( sc1actv8 )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "ac82019.bin", 0x00000, 0x10000, CRC(91855497) SHA1(dee8b6df953a3761fb67395842f701672e93a71e) )

	ROM_REGION( 0x20000, "upd", 0 )
	ROM_LOAD( "95000600.bin", 0x00000, 0x10000, CRC(f324959a) SHA1(5be8c81dcfcf5f6b8b64a85891cd17e221e9ca08) )
	ROM_LOAD( "95000601.bin", 0x10000, 0x10000, CRC(585323f3) SHA1(e2e83b16bbad24f748a7dc9313b722862a91e5a2) )
ROM_END

ROM_START( sc1armad )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "armada.bin", 0x0000, 0x010000, CRC(9a1be4ca) SHA1(d18b7c8779a8eb50321fbff4d6d8cf6d512bea8b) )
ROM_END


ROM_START( sc1bartk )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "bartrekgameb.bin", 0x0000, 0x8000, CRC(24c7c803) SHA1(ab5051c8727cab44ad59913edab3d5d145728cb5) )
	ROM_LOAD( "bartrekgamea.bin", 0x8000, 0x8000, CRC(a7a84c16) SHA1(8c5ab34268e932be12e85eed5a56386681f13da4) )

	ROM_REGION( 0x20000, "upd", 0 )
	ROM_LOAD( "bartreksnd1.bin", 0x000000, 0x010000, CRC(690b18c3) SHA1(0a3ecadc8d47670bc0f36d76b4335f027ef68542) )
	ROM_LOAD( "bartreksnd2.bin", 0x010000, 0x010000, CRC(4ff8201c) SHA1(859378b4bb8fc5d3497a53c9218302410884e091) )
ROM_END

ROM_START( sc1barcd )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "95740352 b.bin", 0x0000, 0x8000, CRC(6dc3cfd3) SHA1(d71d433ae560ac4db345630ee7f04a7cfb7e933e) )
	ROM_LOAD( "95740351 a.bin", 0x8000, 0x8000, CRC(0891350b) SHA1(ea1295768738b9b89eac19d04411220a8c9d10c7) )

	ROM_REGION( 0x10000, "altrevs", 0 )
	ROM_LOAD( "barcode 5_10p a.bin", 0x0000, 0x008000, CRC(e864aba1) SHA1(b3f707b6d5f3d7236e4a5e9ed78c61a78c3e8196) )
	ROM_LOAD( "barcode 5_10p b.bin", 0x0000, 0x008000, CRC(69d4d0b2) SHA1(bb73b917cf414623dcd239c5daeeccb4e0ccc2ed) )
	ROM_LOAD( "barcode.p1", 0x0000, 0x008000, CRC(0be64bfb) SHA1(3b5cfee8825f2b7d2598f04411d50b8f1245ac65) )
	ROM_LOAD( "barcode.p2", 0x0000, 0x008000, CRC(44b79b14) SHA1(ec0745be0dde818c673c62ca584e22871a73e66e) )

	ROM_REGION( 0x20000, "upd", 0 )
	ROM_LOAD( "barsnd1.bin", 0x000000, 0x010000, CRC(c9de8ff4) SHA1(c3e77e84d4ecc1c779929a96d1c445a1af24865b) )
	ROM_LOAD( "barsnd2.bin", 0x010000, 0x010000, CRC(56af984a) SHA1(aebd30f3ca767dc5fc77fb01765833ee627a5aee) )
ROM_END

ROM_START( sc1bigmt )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "bigmatch.bin", 0x0000, 0x010000, CRC(3c81663c) SHA1(a9670a48059d35d6581ce3007c0a6223291e0a12) )

	ROM_REGION( 0x20000, "upd", 0 )
	ROM_LOAD( "bigmsnd1.bin", 0x000000, 0x010000, CRC(51828aa0) SHA1(99b46c1c4b45f26a393bf3e658ad499c84bdf8f5) )
	ROM_LOAD( "bigmsnd2.bin", 0x010000, 0x010000, CRC(cf1f0f6b) SHA1(6521f0fe52a0587af049940bb81846d40d8847b8) )
ROM_END


ROM_START( sc1calyp )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "calypso.bin", 0x0000, 0x010000, CRC(b8194d31) SHA1(de7d374d8a1c18ec324daf92112652461e2a113e) )
ROM_END

ROM_START( sc1carro )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "carrousel.bin", 0x0000, 0x010000, CRC(d1f7ae57) SHA1(301727b95f30d8e934a9c790838daf65aadd6dc7) )
ROM_END

ROM_START( sc1cshat )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "cash_attraction_b", 0x0000, 0x8000, CRC(79870574) SHA1(89e5db89064a9e24bc37389d78f4defb7d2f479b) )
	ROM_LOAD( "cash_attraction_a", 0x8000, 0x8000, CRC(fab3283c) SHA1(669b425687faad0ebf88c1aaaafa40c446fa2e24) )

	ROM_REGION( 0x10000, "altrevs", 0 )
	ROM_LOAD( "957172.20 std var% a.bin", 0x0000, 0x008000, CRC(e67fc9e1) SHA1(39ac2c30d605f2b3109a57c6633a597e77651e79) )
	ROM_LOAD( "957172.21 var% b.bin", 0x0000, 0x008000, CRC(ea705443) SHA1(fdd941b5e6785d97e990f4ca74578e539512422b) )
	ROM_LOAD( "957172.40 b std var%.bin", 0x0000, 0x008000, CRC(5e4381f9) SHA1(ae6d64c42ae7ddc2ed0ab5c3b56222090004d88a) )
	ROM_LOAD( "95717270 20 n.p a.bin", 0x0000, 0x008000, CRC(4e90868a) SHA1(f88a1b578b2d9091f5e5212768547db19e6b5379) )
	ROM_LOAD( "95717271 20p std b.bin", 0x0000, 0x008000, CRC(79870574) SHA1(89e5db89064a9e24bc37389d78f4defb7d2f479b) )
	ROM_LOAD( "957182.20 var% proto a.bin", 0x0000, 0x008000, CRC(3a2dd72d) SHA1(29d962702095aa0f252210da68a89c557fa9db69) )
	ROM_LOAD( "957182.39 74-78 proto a.bin", 0x0000, 0x008000, CRC(f890b2d3) SHA1(e714973c63486e6983912fb6aebee3a71e003be5) )
	ROM_LOAD( "957182.39 proto var%.bin", 0x0000, 0x008000, CRC(43f452a7) SHA1(13ef94b4a4ecf729dfe481da26804f2e6f0631b0) )
	ROM_LOAD( "957272.20 74-78 standard.bin", 0x0000, 0x008000, CRC(06def19d) SHA1(721d8ffc7e6b0e76f097d82b3be7618d97d73041) )
	ROM_LOAD( "957272.21 74-78b.bin", 0x0000, 0x008000, CRC(531e97fb) SHA1(c7ae94c503f9e13d68ae463dd19212f146b0e8bc) )
	ROM_LOAD( "957272.40 74-78b.bin", 0x0000, 0x008000, CRC(e72d4241) SHA1(487a00f49fa5451f39c2400f6f23a5f067afaa66) )
	ROM_LOAD( "95728.20 74-78 proto a.bin", 0x0000, 0x008000, CRC(7e557f21) SHA1(49bbbbafff757acd078d156bae2c942991f055af) )
	ROM_LOAD( "957282.20 74-78 proto a.bin", 0x0000, 0x008000, CRC(7e557f21) SHA1(49bbbbafff757acd078d156bae2c942991f055af) )
ROM_END


ROM_START( sc1cshcd )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "cascrd2.bin", 0x0000, 0x8000, CRC(862d5ea9) SHA1(f0c0334aed028ab995b4d092abe10ece90be40a5) )
	ROM_LOAD( "cascrd1.bin", 0x8000, 0x8000, CRC(23142134) SHA1(40a900d190480677c883912e60f447e83b4a5c92) )
ROM_END

ROM_START( sc1cshcda )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "95717128 var% b.bin", 0x0000, 0x8000, CRC(10662200) SHA1(79a35b88eca408ae2f5daead498662303e0360e1) )
	ROM_LOAD( "95717127 var% a.bin", 0x8000, 0x8000, CRC(1f7ef1ec) SHA1(9f8f43037788787f4f11501689cb82eeebc6d7f8) )
ROM_END

ROM_START( sc1cshcdb )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "95719104b c.cards 78%.bin", 0x0000, 0x8000, CRC(e9055e1b) SHA1(8c6b7e164c9998c3b932e16c3c4e4a95beb29f50) )
	ROM_LOAD( "95719103a c.cards 78%.bin", 0x8000, 0x8000, CRC(af65962c) SHA1(d10dd9e1bbdd1e506d5f8732ffbb6521e34fbefe) )
ROM_END

ROM_START( sc1ccoin )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "cashcoin005.bin", 0x0000, 0x10000, CRC(5ce29d18) SHA1(c9e8d0aa52ba532177d912901a39e4fc8024810f) )

	ROM_REGION( 0x20000, "upd", 0 )
	ROM_LOAD( "cashcoinic7.bin", 0x00000, 0x10000, CRC(7c2f52ed) SHA1(d435402459efc9311707ac691992874b56cbbeec) )
	ROM_LOAD( "cashcoinic8.bin", 0x10000, 0x10000, CRC(23b99731) SHA1(7cc1c51d9b72480d8a1020fc3621a05ba83d7629) )
ROM_END

ROM_START( sc1cexpd )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "cashexpl.bin", 0x8000, 0x008000, CRC(83c6196c) SHA1(931fb5223c3ebc52ca2bd232d71000b8af4397e1) )
ROM_END


ROM_START( sc1cexpl )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "cashxpl2.bin", 0x0000, 0x008000, CRC(0199136c) SHA1(bed1df64ecc0d7ef951a59717e219e6fe7ebf99c) )
	ROM_LOAD( "cashxpl1.bin", 0x8000, 0x008000, CRC(0fe62ead) SHA1(bd56a216292e9bf2b7753616a6cd25b37e22095f) )

	ROM_REGION( 0x10000, "altrevs", 0 ) // i think some of this is on different hw
	ROM_LOAD( "95717083a mk1.bin", 0x0000, 0x008000, CRC(949c2d18) SHA1(26db983c7d8624c3fd0461dba2336d5c59be29f0) )
	ROM_LOAD( "95717083a mk2 var%.bin", 0x0000, 0x008000, CRC(1fe1b3a1) SHA1(dd10d74c71a455900a2325ac9d7b3c8e45eb9c6c) )
	ROM_LOAD( "95717083a.bin", 0x0000, 0x008000, CRC(1fe1b3a1) SHA1(dd10d74c71a455900a2325ac9d7b3c8e45eb9c6c) )
	ROM_LOAD( "95717084b mk1.bin", 0x0000, 0x008000, CRC(e2d973be) SHA1(56ed3e3d6caf12f82d6ccc1527ff8da215e09cb0) )
	ROM_LOAD( "95717084b mk2 var%.bin", 0x0000, 0x008000, CRC(e2d973be) SHA1(56ed3e3d6caf12f82d6ccc1527ff8da215e09cb0) )
	ROM_LOAD( "95717084b.bin", 0x0000, 0x008000, CRC(e2d973be) SHA1(56ed3e3d6caf12f82d6ccc1527ff8da215e09cb0) )
	ROM_LOAD( "cash_explosion_dat_ac_8_10pnd_20p_a.bin", 0x0000, 0x010000, CRC(1d155799) SHA1(4e76328a4d093d1f9c64c633c3558db2dce4e219) )
	ROM_LOAD( "cash_explosion_dat_ac_var_8pnd_a.bin", 0x0000, 0x010000, CRC(4aa53121) SHA1(cf0510e224de62b837915d39c2fe3559cfe8c85f) )
	ROM_LOAD( "cash_explosion_dat_wi_ac_10pnd_20p_a.bin", 0x0000, 0x010000, CRC(889eb206) SHA1(91b23a2cc475e68470d01976b88b9ea7aa0afed9) )
	ROM_LOAD( "cash_explosion_std_ac_8_10pnd_20p_a.bin", 0x0000, 0x010000, CRC(de6bbee2) SHA1(3c321fa442b25a27c3f14b7ac94255f020056663) )
	ROM_LOAD( "cash_explosion_std_ac_var_8pnd_a.bin", 0x0000, 0x010000, CRC(e8a21401) SHA1(479edd734ca949e344fb7e17ed7af7c8c9604efc) )
	ROM_LOAD( "cash_explosion_std_wi_ac_10pnd_20p_a.bin", 0x0000, 0x010000, CRC(2901a315) SHA1(c9733488894ccead7a69b161f2afacdb3f892b89) )
	ROM_LOAD( "cbexpp1", 0x0000, 0x008000, CRC(8819728c) SHA1(691d6317fd38e09fa333fc49c82e85f69a04e359) )
	ROM_LOAD( "cbexpp2", 0x0000, 0x008000, CRC(82eb61ac) SHA1(9c06542b43b01be5ec7be081fead92bfe9f905c5) )
ROM_END


ROM_START( sc1cshwz )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "95717205a std.bin", 0x8000, 0x008000, CRC(795bbeea) SHA1(22e0fc9bc3c70e05e51cb98837a9c706eb2ca080) )
	ROM_LOAD( "95717206b std.bin", 0x0000, 0x008000, CRC(2478530f) SHA1(be82a4e36a3c076b9e94fa2364904ca463b6b4ed) )

	ROM_REGION( 0x10000, "altrevs", 0 )
	ROM_LOAD( "95718205a std ptel.bin", 0x0000, 0x008000, CRC(c88f476c) SHA1(a5d8f12ade77bdb100ece5f2eecec35ae09f3b0e) )
	ROM_LOAD( "95718210a proto var.bin", 0x0000, 0x008000, CRC(0997c4e9) SHA1(1013a12803796d3926cceeb671c7c07cc66d418e) )
	ROM_LOAD( "95727205a 74-78 adj.bin", 0x0000, 0x008000, CRC(7c7ddabc) SHA1(b4c7a9ee929b5635091366948257f273a21d7818) )
	ROM_LOAD( "95727206b 74-78 adj.bin", 0x0000, 0x008000, CRC(2b0ea9dc) SHA1(a9099abe2cf4cdf119a00e5a218507798d410eff) )
	ROM_LOAD( "95727210a 74-78 sw.bin", 0x0000, 0x008000, CRC(6276ee67) SHA1(cc9b794f0add6d68677858719831e10afbdbc699) )
	ROM_LOAD( "95727211b 74-78 sw.bin", 0x0000, 0x008000, CRC(e20ee4d3) SHA1(3440ad647f8e009a13de6ff9797a47c636a50123) )
	ROM_LOAD( "95728205a 74-78 adj proto.bin", 0x0000, 0x008000, CRC(1cdadddb) SHA1(33c7ed10b1c9ddc0fc6065ad9b1cf80ee9f8e958) )
	ROM_LOAD( "95728210a 74-78 proto.bin", 0x0000, 0x008000, CRC(5c502423) SHA1(4fc93de9dd3aff7a8a8f828760d8b095b7a13630) )
	ROM_LOAD( "cashwisep1.bin", 0x0000, 0x008000, CRC(6276ee67) SHA1(cc9b794f0add6d68677858719831e10afbdbc699) )
	ROM_LOAD( "cashwisep2.bin", 0x0000, 0x008000, CRC(e20ee4d3) SHA1(3440ad647f8e009a13de6ff9797a47c636a50123) )
	ROM_LOAD( "cwise_a.bin", 0x0000, 0x008000, CRC(5b305f11) SHA1(592ea71fcb72eaa90fd421e3bd3761cfd686b019) )
	ROM_LOAD( "cwise_b.bin", 0x0000, 0x008000, CRC(0528a718) SHA1(27f4225c948d93ce1c833679f97e045f3b7a6aac) )
ROM_END


ROM_START( sc1cshin )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "cashino-a.bin", 0x8000, 0x008000, CRC(8a585683) SHA1(01859c82a6d6b082de11e9208f8d38c519dc2575) )
	ROM_LOAD( "cashino-b.bin", 0x0000, 0x008000, CRC(c0d3fb09) SHA1(7e0a302547b18946851d31be4d25c17aca32b767) )
ROM_END


ROM_START( sc1china )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "club-china-town_dat_ac_200pnd_ass.bin", 0x8000, 0x008000, CRC(5aa465b9) SHA1(3c2d805f0421d7d1db93f21358a2beb648c05f8e) )
	ROM_LOAD( "club-china-town_dat_ac_200pnd_bss.bin", 0x0000, 0x008000, CRC(4895098f) SHA1(e08f9b85c634a423a93608a7b592436ae253ca42) )

	ROM_REGION( 0x10000, "altrevs", 0 )
	ROM_LOAD( "39371857.p1", 0x0000, 0x008000, CRC(5aa465b9) SHA1(3c2d805f0421d7d1db93f21358a2beb648c05f8e) )
	ROM_LOAD( "club-china-town_dat_ac_150pnd_lfj_ass.bin", 0x0000, 0x008000, CRC(9547727a) SHA1(ac4a23ae78d9331261ee0ab59816f65c5c1547d7) )
	ROM_LOAD( "club-china-town_dat_ac_150pnd_lfj_bss.bin", 0x0000, 0x008000, CRC(d41c6999) SHA1(cc2eb2e74ca3bfa78d74dd08f83acb2fe650e13d) )
	ROM_LOAD( "club-china-town_dat_ac_rot_ass.bin", 0x0000, 0x008000, CRC(109b722c) SHA1(19426f3f907f108dc16b4036d3986c6395f799d0) )
	ROM_LOAD( "club-china-town_dat_ac_rot_bss.bin", 0x0000, 0x008000, CRC(6e09a878) SHA1(4084b1dc3425ceb980ef5c63a883720f3ad84d7f) )
	ROM_LOAD( "club-china-town_std_ac_150pnd_lfj_ass.bin", 0x0000, 0x008000, CRC(8c3e69f1) SHA1(cb0cbf7a6039549b969160a162a0cd5511b24cd3) )
	ROM_LOAD( "club-china-town_std_ac_150pnd_lfj_bss.bin", 0x0000, 0x008000, CRC(d41c6999) SHA1(cc2eb2e74ca3bfa78d74dd08f83acb2fe650e13d) )
	ROM_LOAD( "club-china-town_std_ac_200pnd_rot_ass.bin", 0x0000, 0x008000, CRC(a9ed6493) SHA1(8049fe4b42110afab91dd2d9ccd132d4f2c1c0ff) )
	ROM_LOAD( "club-china-town_std_ac_200pnd_rot_bss.bin", 0x0000, 0x008000, CRC(4895098f) SHA1(e08f9b85c634a423a93608a7b592436ae253ca42) )
	ROM_LOAD( "club-china-town_std_ac_rot_ass.bin", 0x0000, 0x008000, CRC(de12ac34) SHA1(0caeb2a6b209ee34d67d4c619dd63562c839261e) )
	ROM_LOAD( "club-china-town_std_ac_rot_bss.bin", 0x0000, 0x008000, CRC(6e09a878) SHA1(4084b1dc3425ceb980ef5c63a883720f3ad84d7f) )

	ROM_REGION( 0x20000, "upd", 0 ) // are these used, or for one of the other revs?
	ROM_LOAD( "ctowsnd1.bin", 0x00000, 0x010000, CRC(faf28e18) SHA1(0586a905f944bcc990d4a1b400629412a69fc160) )
	ROM_LOAD( "ctowsnd2.bin", 0x10000, 0x010000, CRC(f4f9c1a4) SHA1(af5aff58b3e362a14e26a5e8cae83affda905819) )
ROM_END

ROM_START( sc1class )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "classic606.bin", 0x0000, 0x010000, CRC(f1dc300e) SHA1(17b1d69ed2fd3ce91ce86e9b0160a150a74a624b) )
ROM_END

ROM_START( sc1cwcl )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "95717153a.bin", 0x8000, 0x008000, CRC(233174a1) SHA1(94cf071a955e3716f463c4370daabfe94db2fd0e) )
	ROM_LOAD( "95717154b.bin", 0x0000, 0x008000, CRC(e6422f75) SHA1(4ab33a5503209377f4739dbe11e4afa8d7e43699) )

	ROM_REGION( 0x20000, "upd", 0 )
	ROM_LOAD( "95000001snd.bin", 0x00000, 0x008000, CRC(38f85127) SHA1(c9c7c8892396180aa4c4a727422391b9ce93a10a) )
	ROM_LOAD( "95000002snd.bin", 0x08000, 0x008000, CRC(ca2f5547) SHA1(fe8378ee485ce396b665ea504650caf51843fd74) )
	ROM_LOAD( "95000003snd.bin", 0x10000, 0x008000, CRC(475695f9) SHA1(9f6ba3de7b4b38946106a3aeab9a2a2eb2a99193) )
ROM_END


ROM_START( sc1clown )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "clowarou.786", 0x0000, 0x010000, CRC(d63b991c) SHA1(2f016aec3d2d8ebadcdbe794230ebf18dd660876) )

	ROM_REGION( 0x20000, "upd", 0 )
	ROM_LOAD( "clownsound1.bin", 0x0000, 0x010000, CRC(d72b344b) SHA1(dbf180830ce74dc8d0b832f3932b5c11259acab2) )
	ROM_LOAD( "clownsound2.bin", 0x0000, 0x010000, CRC(98c0440c) SHA1(ef6d7ecf21d49aa8e838429e3431ebcd30fec21e) )
ROM_END


ROM_START( sc1cl2k )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "club2000.rom", 0x0000, 0x010000, CRC(2806b89d) SHA1(d1641b33e61de42dc7a643875226a276cf480832) )
ROM_END

ROM_START( sc1cl2k1 )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "club2001.bin", 0x0000, 0x010000, CRC(4bb26aca) SHA1(41a896be314f2fefdaba962b44e9562aaf0642b1) )
ROM_END


ROM_START( sc1cl65 )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "65spp1", 0x8000, 0x008000, CRC(2c4cb63b) SHA1(5d09b575cf80beecd83c07286b74af29de7ec553) )
	ROM_LOAD( "65spp2", 0x0000, 0x008000, CRC(11332a28) SHA1(76f9eee54351e0d8dc4b620ec92661538929e75d) )

	ROM_REGION( 0x10000, "altrevs", 0 )
	ROM_LOAD( "39370694.p2", 0x0000, 0x008000, CRC(3371dc55) SHA1(52d75a90933acc7a03821e5c2821df6126c72a6c) )
	ROM_LOAD( "39370714.p2", 0x0000, 0x008000, CRC(cb9f944f) SHA1(49955a968264f3d963317f5c772629d9bbdd33f7) )
	ROM_LOAD( "39370858.p2", 0x0000, 0x008000, CRC(ff0e35c0) SHA1(0d3d46b541e188200cb4b9cc65eb60eac913dc2b) )
	ROM_LOAD( "39370859.p2", 0x0000, 0x008000, CRC(f04065a0) SHA1(d63ba578931b2f4f156ca875da9cf69cf283a27c) )
	ROM_LOAD( "club-six-five-special_dat_ac_200pnd_rot_10po_ass.bin", 0x0000, 0x008000, CRC(836e65d8) SHA1(931e5831b0b64e7ce29fb497435d486e40dce839) )
	ROM_LOAD( "club-six-five-special_dat_ac_200pnd_rot_20po_ass.bin", 0x0000, 0x008000, CRC(83a0253f) SHA1(a9b463e2aa87a736f88c5e71f233ff9d6a8b25b4) )
	ROM_LOAD( "club-six-five-special_dat_ac_rot_10po_ass.bin", 0x0000, 0x008000, CRC(77ddf81d) SHA1(522d9f84ab6e31586f371548e2f146ac193f06f5) )
	ROM_LOAD( "club-six-five-special_dat_ac_rot_20po_ass.bin", 0x0000, 0x008000, CRC(028ff7b2) SHA1(500b6f8d85678e99ae804600099fe78b542ad6a3) )
	ROM_LOAD( "club-six-five-special_snd_a.bin", 0x0000, 0x010000, CRC(915802cd) SHA1(5bca3a80199a6534e084a5cf4337da4e9c48f45c) )
	ROM_LOAD( "club-six-five-special_snd_b.bin", 0x0000, 0x010000, CRC(b3b230d8) SHA1(022e95f38b14922137222805c0bec7498c5956cc) )
	ROM_LOAD( "club-six-five-special_std_ac_200pnd_rot_10po_ass.bin", 0x0000, 0x008000, CRC(eedd9fa1) SHA1(6233e8304ac94798cfb908b2ba31ec6c98808ce8) )
	ROM_LOAD( "club-six-five-special_std_ac_200pnd_rot_10po_bss.bin", 0x0000, 0x008000, CRC(f04065a0) SHA1(d63ba578931b2f4f156ca875da9cf69cf283a27c) )
	ROM_LOAD( "club-six-five-special_std_ac_200pnd_rot_20po_ass.bin", 0x0000, 0x008000, CRC(dd188272) SHA1(d6b7f7b060e632bd3eacc7f7721399a1c8349698) )
	ROM_LOAD( "club-six-five-special_std_ac_200pnd_rot_20po_bss.bin", 0x0000, 0x008000, CRC(ff0e35c0) SHA1(0d3d46b541e188200cb4b9cc65eb60eac913dc2b) )
	ROM_LOAD( "club-six-five-special_std_ac_a.bin", 0x0000, 0x008000, CRC(8bd817f8) SHA1(6ae91a29a6263c085f6254a049fb3c2ba9cac662) )
	ROM_LOAD( "club-six-five-special_std_ac_b.bin", 0x0000, 0x008000, CRC(cb9f944f) SHA1(49955a968264f3d963317f5c772629d9bbdd33f7) )
	ROM_LOAD( "club-six-five-special_std_ac_rot_10po_ass.bin", 0x0000, 0x008000, CRC(cf48ba99) SHA1(5da4321ff349964e903f1bebd3e5ddd0799fc478) )
	ROM_LOAD( "club-six-five-special_std_ac_rot_10po_bss.bin", 0x0000, 0x008000, CRC(3371dc55) SHA1(52d75a90933acc7a03821e5c2821df6126c72a6c) )

	ROM_REGION( 0x20000, "upd", 0 )
	ROM_LOAD( "65sndp1.bin", 0x000000, 0x010000, CRC(e532fcf5) SHA1(7de3bd4a3efae7d1cfeee23c008efbff39ce46f8) )
	ROM_LOAD( "65sndp2.bin", 0x010000, 0x010000, CRC(2703ea2d) SHA1(a4876a10d8d4b1de01dfab76e4ee21cb120aa783) )
ROM_END



ROM_START( sc1clbdm )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "clubdiamond.bin", 0x8000, 0x008000, CRC(7e6a569e) SHA1(ba15478ae0312d3e9c21546aa676b4ab95ae944c) )
ROM_END




ROM_START( sc1clbxp )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "clubexplosion1.bin", 0x8000, 0x008000, CRC(876161db) SHA1(a6262d70870a6edb71469ec8cea317b185aec49e) )
	ROM_LOAD( "clubexplosion2.bin", 0x0000, 0x008000, CRC(da56fbdd) SHA1(0ea35f6672a4a4b9236d8341733496450b64238e) )

	ROM_REGION( 0x20000, "upd", 0 )//Did a version of this have a UPD sound board, if so, these seem to be ROMs for it
	ROM_LOAD( "95000004.bin", 0x000000, 0x008000, CRC(6ed10c9b) SHA1(cd209e8f9e0a3fd41e4ed8b6c9387ee91c19704c) )
	ROM_LOAD( "95000005.bin", 0x008000, 0x008000, CRC(9e16aee2) SHA1(25610fcd4c073ff7f20a3d24f96792913fa447f7) )
	ROM_LOAD( "95000006.bin", 0x010000, 0x008000, CRC(41636b3d) SHA1(8bc4dfcd5bd56422e303c73d50c2e7afa2edef5a) )
ROM_END





ROM_START( sc1clbrn )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "clubrunner.bin", 0x0000, 0x010000, CRC(32b2d57b) SHA1(26523518bfb726d55d6808451f4041756f99b1d9) )
ROM_END




ROM_START( sc1clbsp )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "rveclsp0108_1.bin", 0x0000, 0x010000, CRC(d60c9f4b) SHA1(dcbb6a10db2f658b734ed0fdecf907a4a32eedaa) )
ROM_END




ROM_START( sc1clbtm )
	ROM_REGION( 0x10000, "maincpu", 0 )

	/* these are other hw (SC4?)
    ROM_LOAD( "95008055.bin", 0x0000, 0x100000, CRC(df9ae6e3) SHA1(5766cb1749aa92c34a76270a641f7a9302cc44d7) )
    ROM_LOAD( "95008055.p1", 0x0000, 0x800000, CRC(ef474fd3) SHA1(e7427184683603b57a3a8b37452fa6ec7a41e34c) )
    ROM_LOAD( "95008056.p2", 0x0000, 0x800000, CRC(39b1b566) SHA1(937ec27964124b92b75d4b37d09a35585baa68c6) )
    ROM_LOAD( "95402149.lo", 0x0000, 0x080000, CRC(f5eee630) SHA1(102ef766562a67cd5c339d755f527252aee924be) )
    ROM_LOAD( "95402150.hi", 0x0000, 0x080000, CRC(19ada5f4) SHA1(3719e29465249026c781fe5226d05770c2e8ce99) )
    ROM_LOAD( "95402257.lo", 0x0000, 0x080000, CRC(fca966af) SHA1(63aa17640405fb858d776799e2388679dfe02a26) )
    ROM_LOAD( "95402258.hi", 0x0000, 0x080000, CRC(24ca572a) SHA1(5daa89c4427e70d0fbebfca116ea6932716f38f8) )
    ROM_LOAD( "95403149.lo", 0x0000, 0x080000, CRC(a62d1945) SHA1(197f9245d46cb156987cbacc1ac0c8230030fdcd) )
    ROM_LOAD( "95403150.hi", 0x0000, 0x080000, CRC(9f79e460) SHA1(0162d5fc54361c5853dc865d61849f766f833545) )
    ROM_LOAD( "95403257.lo", 0x0000, 0x080000, CRC(5aada808) SHA1(e24bc5e523961abdedfb27ec5cff2cbb6f45bddd) )
    ROM_LOAD( "95403258.hi", 0x0000, 0x080000, CRC(2b133e43) SHA1(805ca1ddf1ebcf6cbbd4b8b41baf5c2056b5bf17) )
    ROM_LOAD( "95404682.lo", 0x0000, 0x080000, CRC(3b9e429f) SHA1(01046d8671a287b2a1c739f84d28c69ba2c8c80f) )
    ROM_LOAD( "95404683.hi", 0x0000, 0x080000, CRC(f84b37ad) SHA1(4430c7fe6274d25de58342d255c4c4a52966b0b3) )
    ROM_LOAD( "95405682.lo", 0x0000, 0x080000, CRC(12d4bf1a) SHA1(2b28c5fdca13e7a70496984e02bfd3b98d60a9ac) )
    ROM_LOAD( "95405683.hi", 0x0000, 0x080000, CRC(b9aba368) SHA1(ec6404447c9e23e179ec1200848d966d5f3f846b) )
    ROM_LOAD( "95717692a.bin", 0x0000, 0x008000, CRC(f9fe7b9a) SHA1(0e3fe5da9fc837726d08f02a2c6ed782f016c982) )
    ROM_LOAD( "club-temptation_mtx_(ihex)ss.hex", 0x0000, 0x01d0da, CRC(08ebee96) SHA1(2e87d734c966abab1d4a59c9481ebea161f77286) )
    ROM_LOAD( "clubtempdot.bin", 0x0000, 0x010000, CRC(283d2d9c) SHA1(5b76a13ad674f8a40c270e5dbc61dac04d411d02) )
    */
	ROM_LOAD( "temp11a.bin", 0x8000, 0x008000, CRC(37c8b73e) SHA1(f718572d170be7b582c3818df7163309cea232b5) )
	ROM_LOAD( "temp12b.bin", 0x0000, 0x008000, CRC(3c27c592) SHA1(081d61f974e2ae5c64729b32be4c0e5067a20550) )

	ROM_REGION( 0x40000, "altrevs", 0 )
	ROM_LOAD( "temptp1", 0x0000, 0x008000, CRC(6f03648d) SHA1(a6402c94ebf4d570d1d3fb462eb621566c27f307) )
	ROM_LOAD( "temptp2", 0x0000, 0x008000, CRC(d165fa87) SHA1(aef8a4af8b6e83ef09dffc8aca305eaf7dd3936b) )


	ROM_REGION( 0x40000, "upd", 0 )
	ROM_LOAD( "tempsnd1.bin", 0x0000, 0x010000, CRC(168e2a18) SHA1(db97acf9131b1a54efe1cd375aecae1679bab19e) )
	ROM_LOAD( "tempsnd2.bin", 0x0000, 0x010000, CRC(b717f347) SHA1(189c82318d622f18580a23eed48b17c0c34dedd5) )
ROM_END




ROM_START( sc1clbw )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "clbwzp1", 0x8000, 0x008000, CRC(c61dd4eb) SHA1(e1756f8841dabe1bc002aadba6b224a558096a96) )
	ROM_LOAD( "clbwzp2", 0x0000, 0x008000, CRC(44bb7e16) SHA1(d3c258ea286be18dc667df6a7138280462db661b) )

	ROM_REGION( 0x40000, "altrevs", 0 )
	ROM_LOAD( "club wise 95717550a.bin", 0x0000, 0x008000, CRC(5b305f11) SHA1(592ea71fcb72eaa90fd421e3bd3761cfd686b019) )
	ROM_LOAD( "club wise 95717551b.bin", 0x0000, 0x008000, CRC(0528a718) SHA1(27f4225c948d93ce1c833679f97e045f3b7a6aac) )
	ROM_LOAD( "cwise_a.bin", 0x0000, 0x008000, CRC(5b305f11) SHA1(592ea71fcb72eaa90fd421e3bd3761cfd686b019) )
	ROM_LOAD( "cwise_b.bin", 0x0000, 0x008000, CRC(0528a718) SHA1(27f4225c948d93ce1c833679f97e045f3b7a6aac) )

	ROM_REGION( 0x20000, "upd", 0 )
	ROM_LOAD( "wisesnd.bin", 0x0000, 0x020000, CRC(35cb0314) SHA1(6ceec1fc17015d8228d55331fcffb77390161136) )
	ROM_LOAD( "wisesnd1.bin", 0x0000, 0x010000, CRC(204605a6) SHA1(193a60878ed46f122e5d2d8f35fc6ea967b8734f) )
	ROM_LOAD( "wisesnd2.bin", 0x010000, 0x010000, CRC(6aa66166) SHA1(2e7cc67afdce2febb541bb1d0e7c107876d4233d) )
ROM_END



ROM_START( sc1copdd )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "game_835.bin", 0x0000, 0x010000, CRC(af134088) SHA1(c6467102903a2910c67f2b8051e1f788576ef62f) )

	ROM_REGION( 0x40000, "upd", 0 )
	ROM_LOAD( "snd1ic7_519.bin", 0x0000, 0x010000, CRC(5cd39b04) SHA1(83cc51c208e8a9d3ccd0b4fcd2ab74a5f71e0c28) )
	ROM_LOAD( "snd2ic8_520.bin", 0x0000, 0x010000, CRC(a22621ec) SHA1(add91e6b1e14118c718614a7cfaa2d3aabbf01b3) )
ROM_END




ROM_START( sc1copdx )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "cops & robbers deluxe 5p-10p 6 p1 95840219 (27256)", 0x8000, 0x008000, CRC(47867f55) SHA1(33f879a8e1e4e2f53b5da8b4ee597bd3870c75d1) )
	ROM_LOAD( "cops & robbers deluxe 5p-10p 6 p2 95840220 (27256)", 0x0000, 0x008000, CRC(32a22682) SHA1(c173688ace476a2ada398d5e7b5dfed5306e3c50) )

	ROM_REGION( 0x40000, "upd", 0 )
	ROM_LOAD( "cops & robbers deluxe sound 1 295 (27512)", 0x000000, 0x010000, CRC(81227c21) SHA1(6af8e15f8405fdfbaa3a8853ec7ec62fe5ec34ae) )
	ROM_LOAD( "cops & robbers deluxe sound 2 296 (27512)", 0x010000, 0x010000, CRC(8ecf1f5e) SHA1(4159b5c3800708cde94ce62a5e07b58ad8aaedf8) )
ROM_END




ROM_START( sc1count )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "club-count-cash_std_ac_200pnd_rot_ass.bin", 0x8000, 0x008000, CRC(a6a1a604) SHA1(86e59578fed7023b0e6a42495b9a60e7178ee566) )
	ROM_LOAD( "club-count-cash_std_ac_200pnd_rot_bss.bin", 0x0000, 0x008000, CRC(8e385a9e) SHA1(67c45734501c16be3b8270f388dc1313bce289f8) )

	ROM_REGION( 0x40000, "altrevs", 0 )
	ROM_LOAD( "club-count-cash_dat_ac_200pnd_rot_ass.bin", 0x0000, 0x008000, CRC(da097abe) SHA1(85f01d8b5dce535a5559fadaf1cf7373c6967882) )
	ROM_LOAD( "club-count-cash_dat_ac_200pnd_rot_bss.bin", 0x0000, 0x008000, CRC(8e385a9e) SHA1(67c45734501c16be3b8270f388dc1313bce289f8) )
	ROM_LOAD( "club-count-cash_dat_ac_rnr_ass.bin", 0x0000, 0x008000, CRC(87f68f57) SHA1(fe99c8577a80a7ec791bf87e78cf429eebbc7785) )
	ROM_LOAD( "club-count-cash_dat_ac_rnr_bss.bin", 0x0000, 0x008000, CRC(69df417d) SHA1(a7788a9f3056919017616960ba5017bcd94b8a98) )
	ROM_LOAD( "club-count-cash_std_ac_rot_ass.bin", 0x0000, 0x008000, CRC(b081333c) SHA1(75a46634458a790f91360be26cace0e42bbf3481) )
	ROM_LOAD( "club-count-cash_std_ac_rot_bss.bin", 0x0000, 0x008000, CRC(69df417d) SHA1(a7788a9f3056919017616960ba5017bcd94b8a98) )
	ROM_LOAD( "club-game-show_std_ac_250pnd-25p_ass.bin", 0x0000, 0x010000, CRC(36efa743) SHA1(0f5392f55e42d7ac17e179c966997f41859f925a) )
ROM_END




ROM_START( sc1dago )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "dgnlg051_single_site_euro.bin", 0x0000, 0x010000, CRC(5ccb9773) SHA1(4940d7b17d36d409504a263acd54d017c6cbb1e1) )
ROM_END




ROM_START( sc1disc )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "disc-88.a", 0x8000, 0x008000, CRC(1ac052d0) SHA1(a37cc2896fb884af7e922289d7fda1e7d26fc387) )
	ROM_LOAD( "disc-88.b", 0x0000, 0x008000, CRC(f6e2d800) SHA1(a0c7ab0c913d9284cdbfa1d35b62afefb903c086) )

	ROM_REGION( 0x40000, "altrevs", 0 )
	ROM_LOAD( "ds1.bin", 0x0000, 0x008000, CRC(22f6ce92) SHA1(5db8f54bc83e963687ebe2f13769e3f2f678d356) )
	ROM_LOAD( "ds2.bin", 0x0000, 0x008000, CRC(fa549c55) SHA1(93a31e4f847dcd326760d17753c994f6210fb6ed) )
ROM_END




ROM_START( sc1dblch )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "doublechancegame1.bin", 0x8000, 0x008000, CRC(9e24e0e3) SHA1(fff1fe9219c052750709d13c06148c7926a22910) )
	ROM_LOAD( "doublechancegame2.bin", 0x0000, 0x008000, CRC(d4f49454) SHA1(53b97f941a4abfeb3e498b4295f98e80bd182b7e) )

	ROM_REGION( 0x40000, "altrevs", 0 )
	ROM_LOAD( "717416_dc_std.a", 0x0000, 0x008000, CRC(949726ed) SHA1(6ecebd20387aa73b0404ab4b7342e2b39d77b37f) )
	ROM_LOAD( "717417_dc_std.b", 0x0000, 0x008000, CRC(51e5459b) SHA1(b6ffbcff63fd3543226778c61fbe2246f40635dd) )
	ROM_LOAD( "95000110_dc_sound_2", 0x0000, 0x010000, CRC(bee6af3e) SHA1(334fe491a00f58a2142f65344674b26c766a7c5b) )
	ROM_LOAD( "95000111_dc_sound_1", 0x0000, 0x010000, CRC(bbadc876) SHA1(902e387ea9bcd833cf75a6f049b5b2822ec6dc2a) )
	ROM_LOAD( "95717787 10p20p.bin", 0x0000, 0x008000, CRC(69ba126c) SHA1(f59aa5a632d0bc5102c206f986f86b6c7c1352fb) )
	ROM_LOAD( "95717789 10p.bin", 0x0000, 0x008000, CRC(fc338d38) SHA1(65457f2611ffa22ac35f1e7ad10c290c01b9c3ac) )
	ROM_LOAD( "95717790 5p.bin", 0x0000, 0x008000, CRC(c82e57f9) SHA1(456ce5290db322292170412a00f0252b86743ed0) )

	ROM_REGION( 0x40000, "upd", 0 )
	ROM_LOAD( "doublechancesnd1.bin", 0x0000, 0x010000, CRC(bee6af3e) SHA1(334fe491a00f58a2142f65344674b26c766a7c5b) )
	ROM_LOAD( "doublechancesnd2.bin", 0x0000, 0x010000, CRC(bbadc876) SHA1(902e387ea9bcd833cf75a6f049b5b2822ec6dc2a) )
ROM_END





ROM_START( sc1dream )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "95752021.bin", 0x0000, 0x010000, CRC(ea7d1cec) SHA1(d277b639575498c458f98e0e1a629d914ca36cfe) )

	ROM_REGION( 0x20000, "upd", 0 )
	ROM_LOAD( "95000604.bin", 0x0000, 0x010000, CRC(7c2f52ed) SHA1(d435402459efc9311707ac691992874b56cbbeec) )
	ROM_LOAD( "95000605.bin", 0x0000, 0x010000, CRC(23b99731) SHA1(7cc1c51d9b72480d8a1020fc3621a05ba83d7629) )
ROM_END




ROM_START( sc1final )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "finaltouch_prg.bin", 0x0000, 0x010000, CRC(27a74fbb) SHA1(8c3e76c67605866acf8e6e28b14788a5cbcd43b4) )

	ROM_REGION( 0x40000, "upd", 0 )
	ROM_LOAD( "finaltouch_snd1.bin", 0x000000, 0x010000, CRC(18ebcdd2) SHA1(17efd903d205d7285f642017de8b5799ede2110b) )
	ROM_LOAD( "finaltouch_snd2.bin", 0x010000, 0x010000, CRC(75a76a6a) SHA1(a660285d56517876414dc951e98185ea14e8fb4e) )
ROM_END



ROM_START( sc1flash )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "flash.bin", 0x0000, 0x010000, CRC(42475bd9) SHA1(634759e64ecc001da5eca01b89e5b93749de541d) )
ROM_END



ROM_START( sc1fruit )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "fruitlnsa.bin", 0x8000, 0x008000, CRC(c002dde4) SHA1(7f7601108975f09ed5846d8acf90a5db36319bbd) )
	ROM_LOAD( "fruitlnsb.bin", 0x0000, 0x008000, CRC(9a44bfdc) SHA1(cd7890b781411b1fdf8abe17e3337a92b40596c7) )
ROM_END



ROM_START( sc1frtln )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "fruitlines.bin", 0x0000, 0x010000, CRC(b26f8c8f) SHA1(f0384046e52fcf5fe5eabb7a155b119725f3cdd9) )
ROM_END



ROM_START( sc1funh )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "club-fun-house_std_ac_a.bin", 0x008000, 0x008000, CRC(f81dff1b) SHA1(4c205b3901f683d3679af9d311813ad912ecb436) )
	ROM_LOAD( "club-fun-house_std_ac_b.bin", 0x0000, 0x008000, CRC(1a838f0d) SHA1(747153e1bb9fc4fc28451e828fa2473f2e6d5e0e) )

	ROM_REGION( 0x40000, "altrevs", 0 )
	ROM_LOAD( "club-fun-house_dat_ac_rot_ass.bin", 0x0000, 0x00781f, CRC(9a24dc71) SHA1(bb19ef26d6d46605107c8b53c6d9b4f08ed4c721) )
	ROM_LOAD( "club-fun-house_dat_ac_rot_bss.bin", 0x0000, 0x008000, CRC(1a838f0d) SHA1(747153e1bb9fc4fc28451e828fa2473f2e6d5e0e) )
	ROM_LOAD( "club-fun-house_std_ac_ass.bin", 0x0000, 0x008000, CRC(f81dff1b) SHA1(4c205b3901f683d3679af9d311813ad912ecb436) )
	ROM_LOAD( "club-fun-house_std_ac_bss.bin", 0x0000, 0x008000, CRC(1a838f0d) SHA1(747153e1bb9fc4fc28451e828fa2473f2e6d5e0e) )
	ROM_LOAD( "funhop1", 0x0000, 0x008000, CRC(282d5651) SHA1(bd8c0985143d8fb5c8e0a2bfedea248569c8cf98) )
	ROM_LOAD( "funhop2", 0x0000, 0x008000, CRC(2454e295) SHA1(9785d278afe05c632e1ab326d1b8fbabcc591fb6) )
	ROM_LOAD( "funhouse.bin", 0x0000, 0x010000, CRC(4e342025) SHA1(288125ff5e3da7249d89dfcc3cd0915f791f7d43) )

	ROM_REGION( 0x40000, "upd", 0 )
	ROM_LOAD( "fhsesnd1.bin", 0x0000, 0x010000, CRC(bf371dbf) SHA1(0c9bc0d0964a858fba5324080a2cf5da119bf3db) )
	ROM_LOAD( "fhsesnd2.bin", 0x0000, 0x010000, CRC(c51415e3) SHA1(f0e4eb5ce38faaef336a5b69e598985ea2486ceb) )
ROM_END



ROM_START( sc1gtime )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "goodtime.bin", 0x0000, 0x010000, CRC(9958dc86) SHA1(43221d0eb50ebe3db8b1d1e784e19b5cbb86c24c) )

	ROM_REGION( 0x40000, "upd", 0 )
	ROM_LOAD( "goodtimesound1.bin", 0x0000, 0x010000, CRC(554d1157) SHA1(ae802338b40a0b35dcdf788c19ef42c2ed7e9a37) )
	ROM_LOAD( "goodtimesound2.bin", 0x010000, 0x010000, CRC(e6c53e20) SHA1(30cb83d03fe873b4ec822d3aa1001b7fed9571ff) )
ROM_END

ROM_START( sc1tiara )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "tiara_prg1u.bin", 0x0000, 0x010000, CRC(963fc838) SHA1(375bc2fb72c89095d1afae77762e94d7adb79133) )

	ROM_REGION( 0x40000, "upd", 0 ) // same sound roms as good times?
	ROM_LOAD( "tiara_snd1.bin", 0x0000, 0x010000, CRC(554d1157) SHA1(ae802338b40a0b35dcdf788c19ef42c2ed7e9a37) )
	ROM_LOAD( "tiara_snd2.bin", 0x010000, 0x010000, CRC(e6c53e20) SHA1(30cb83d03fe873b4ec822d3aa1001b7fed9571ff) )
ROM_END



ROM_START( sc1gprix )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "gp.bin", 0x0000, 0x010000, CRC(35cfe52c) SHA1(5debd45553e91d2aab102c5a712f912efdd6ada3) )

	ROM_REGION( 0x40000, "upd", 0 )
	ROM_LOAD( "grandprix_snd1.bin", 0x0000, 0x010000, CRC(86139b6a) SHA1(13b9483f7379e3cc25f5474fa950878e0a2853d2) )
	ROM_LOAD( "grandprix_snd2.bin", 0x010000, 0x010000, CRC(f1a91ced) SHA1(97e3f03b7eac975ff9dd4e0f10eb18314c36f201) )
ROM_END




ROM_START( sc1gslam )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "gs.bin", 0x0000, 0x010000, CRC(7a239eef) SHA1(5af894dd2df7256c9347b46a5aabd93961c83324) )

	ROM_REGION( 0x40000, "upd", 0 )
	ROM_LOAD( "gslsnd1.bin", 0x0000, 0x010000, CRC(52ca000a) SHA1(a8d4cedf02fae8bb24ea8cf1f62dace49c773858) )
	ROM_LOAD( "gslsnd2.bin", 0x0000, 0x010000, CRC(cb52721e) SHA1(395024a425f057f78a8d83cdbbbc9bf1521f3597) )
	ROM_LOAD( "grandslam_snd_a.bin", 0x0000, 0x010000, CRC(52ca000a) SHA1(a8d4cedf02fae8bb24ea8cf1f62dace49c773858) )
	ROM_LOAD( "grandslam_snd_b.bin", 0x0000, 0x010000, CRC(cb52721e) SHA1(395024a425f057f78a8d83cdbbbc9bf1521f3597) )
ROM_END




ROM_START( sc1happy )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "happyhourromd.bin", 0x0000, 0x010000, CRC(7a03df4f) SHA1(78dbadd4acc3ac7d06e2bc8bf9be080e4cd888fb) )

	ROM_REGION( 0x40000, "upd", 0 )
	ROM_LOAD( "happyhoursnd1.bin", 0x0000, 0x010000, CRC(a7b84f42) SHA1(bfe0b4f7b1c6c55d4fa45ac26f95a045cc21313e) )
	ROM_LOAD( "happyhoursnd2.bin", 0x010000, 0x010000, CRC(e90ffa86) SHA1(8b4f68e3f010854e13abd689db0961092d2dc491) )
ROM_END




ROM_START( sc1impc )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "impact.bin", 0x0000, 0x010000, CRC(dd5d94d4) SHA1(1674ec497daa7dd61412a07ebca3447b69c5780e) )
ROM_END



ROM_START( sc1kings )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "kingsclub.bin", 0x0000, 0x010000, CRC(6f547e05) SHA1(e52872ab94e6bdcb8aa131db6f21535b78cf53ef) )

	ROM_REGION( 0x40000, "upd", 0 )
	ROM_LOAD( "kings279.bin", 0x000000, 0x010000, CRC(8388d112) SHA1(fa31f001011fb463d7cffe88b7cd994ceb3a6977) )
	ROM_LOAD( "kings280.bin", 0x010000, 0x010000, CRC(5566f2bd) SHA1(a49b7a25cf3a008c78dc59c08aaccb6e0e1e480f) )
ROM_END




ROM_START( sc1linx )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "95710137 a.bin", 0x8000, 0x008000, CRC(29c6b8b1) SHA1(643d06a5064ba74902bfbe115b1bd7b1abe14381) )
	ROM_LOAD( "95710138 b.bin", 0x0000, 0x008000, CRC(e731dc61) SHA1(c65b2c7006e58e924370261bdb5ac3f5e3e86471) )

	ROM_REGION( 0x40000, "altrevs", 0 )
	ROM_LOAD( "linx 10p a.bin", 0x0000, 0x008000, CRC(87d8907c) SHA1(3584441870b0a57284e831b0e68422fa3138b4bf) )
	ROM_LOAD( "linx 20p n.p.a 95717315.bin", 0x0000, 0x008000, CRC(ea53e0e1) SHA1(d8d57d44188a33e2751bfc4f21249efc32815877) )
	ROM_LOAD( "linx 20p std 95717316 b.bin", 0x0000, 0x008000, CRC(419b3d7e) SHA1(83b27914cb95afc1053578a279dc936181562217) )
	ROM_LOAD( "linx var% data 10p b.bin", 0x0000, 0x008000, CRC(c23fc39c) SHA1(4e6d2a16606544c00bd175ade4d9e6491ec317ff) )
ROM_END



ROM_START( sc1magc )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "mc.bin", 0x8000, 0x008000, CRC(75cd57c4) SHA1(94409fdf206ebe071fd58bc175622c2bfa439299) )
ROM_END




ROM_START( sc1manha )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "mana.bin", 0x8000, 0x008000, CRC(961dc746) SHA1(98ecfce91f8d111b38d9e658b50bfd921e567a68) )
	ROM_LOAD( "manb.bin", 0x0000, 0x008000, CRC(a0e73800) SHA1(bb56f2aa211ff48e5d4d8bfdff4fc1c7464e01ca) )
ROM_END




ROM_START( sc1mast )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "masterclubgame.bin", 0x0000, 0x010000, CRC(1156383a) SHA1(eb93fae25b1083bfd343015bcbc33f029571b700) )

	ROM_REGION( 0x40000, "upd", 0 )
	ROM_LOAD( "asnd1.bin", 0x0000, 0x010000, CRC(229d0666) SHA1(68650c920d60df1eff00cd77e0308f5c2fd88baf) )
	ROM_LOAD( "asnd2.bin", 0x0000, 0x010000, CRC(3b286391) SHA1(0e0cd818d23d73b681905db98c0b9890809b25f6) )
ROM_END




ROM_START( sc1mist )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "mistral.bin", 0x0000, 0x010000, CRC(7da31d22) SHA1(a63cd098d66af869d3967b15694b6d6ba8cc8d1e) )
ROM_END




ROM_START( sc1olym )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "olympia.bin", 0x0000, 0x010000, CRC(15728d0a) SHA1(addf84f0efec140eecad48116a84c36662a85db2) )
ROM_END




ROM_START( sc1orac )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "oracle_a.bin", 0x0000, 0x008000, CRC(2177f249) SHA1(5144819a8934734b5de9d56384ae89d015b8acee) )
	ROM_LOAD( "oracle_b.bin", 0x8000, 0x008000, CRC(93f38e60) SHA1(741402d0f25b59a9d651875bf5ccbc06389b1ea9) )
ROM_END




ROM_START( sc1pwrl )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "powlinea.bin", 0x8000, 0x008000, CRC(6d03d6ce) SHA1(4a932b87e44e37fed44ff80da542228f2d4b9876) )
	ROM_LOAD( "powlineb.bin", 0x0000, 0x008000, CRC(9d13e39e) SHA1(2df1f402fb49aacc3fc1fecdf536ea1dcee5521f) )

	ROM_REGION( 0x40000, "altrevs", 0 ) // or something else?
	ROM_LOAD( "95000013.bin", 0x0000, 0x008000, CRC(80573db9) SHA1(34e028d1d01328719f6260aafb58f40d664ab7ea) )
	ROM_LOAD( "95000014.bin", 0x0000, 0x008000, CRC(cad7c87b) SHA1(052324bbad28b67d23a018d61a03783dd4dfd9cf) )
	ROM_LOAD( "95000015.bin", 0x0000, 0x008000, CRC(c46911ca) SHA1(a270d0708574a549b88f13f9cde1d7dcdfc624a9) )


	ROM_REGION( 0x40000, "upd", 0 )
	ROM_LOAD( "powl_snd.bin", 0x0000, 0x040000, CRC(e87af436) SHA1(fc853eca052fe13babde5f4579e202321ecb8f7e) )

ROM_END




ROM_START( sc1quat )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "quatro.bin", 0x0000, 0x010000, CRC(c264c520) SHA1(469e0b394061ae4dcf9b0a2c66c6b85404113f5f) )
ROM_END




ROM_START( sc1rain )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "rainbow1.bin", 0x8000, 0x008000, CRC(cdc20fc9) SHA1(306f9877c59b4cfb0653e1f453ef188c93b7b4d3) )
	ROM_LOAD( "rainbow2.bin", 0x0000, 0x008000, CRC(1adf16b0) SHA1(90d0935ec3a0803e1f7fcf8be24cce36f3a53962) )
ROM_END




ROM_START( sc1re )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "reelcashd.bin", 0x0000, 0x010000, CRC(2519c32f) SHA1(b371dbddad617a6d749e1b784cba11758e3b37b8) )
ROM_END




ROM_START( sc1rese )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "reelcashse.bin", 0x0000, 0x010000, CRC(db24d5aa) SHA1(3e673b3652899d2e7e65554ffdfaca67cf3b02bf) )
ROM_END




ROM_START( sc1revo )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "revolution.bin", 0x8000, 0x008000, CRC(d477e4ab) SHA1(01614f9009f1736d1f1c5f2ddea48cf92fd66b0e) )
ROM_END





ROM_START( sc1rose )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "rosecrow.802", 0x0000, 0x010000, CRC(b4cb3517) SHA1(d19e0cc8da5da7d1bcde174cf68cf7d9230cd53d) )
ROM_END



ROM_START( sc1sant )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "santana.bin", 0x0000, 0x010000, CRC(debd19fc) SHA1(e4394c014c5db621647dd54aa7d434705431750c) )
ROM_END




ROM_START( sc1sat )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "sat.bin", 0x8000, 0x008000, CRC(5e1843db) SHA1(14cef347b5409ded4e52ae60fc4990dc79bfbae3) )
ROM_END




ROM_START( sc1shan )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "sha626a.dat", 0x8000, 0x008000, CRC(ea770c35) SHA1(247cec799c439d11d739a7a6f2d1c0cdc7b61e18) )
	ROM_LOAD( "sha626b.dat", 0x0000, 0x008000, CRC(1df2ca25) SHA1(c960a5e536a3fe1c868ae7f0f9983e7f77f61a2a) )
ROM_END





ROM_START( sc1spct )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "spectre_1.bin", 0x8000, 0x008000, CRC(2dadb250) SHA1(d648678864e482bedd27008b50c3bfe50553f0c2) )
	ROM_LOAD( "spectre_2.bin", 0x0000, 0x008000, CRC(7edc2788) SHA1(8336166151a89f9df5735e969d376375059b0024) )

	ROM_REGION( 0x10000, "altrevs", 0 )
	ROM_LOAD( "spec5pa", 0x0000, 0x010000, CRC(65fa549c) SHA1(68fd5a11eb89088f87a727e9c3bb621a4235adf4) )

	ROM_REGION( 0x20000, "upd", 0 )
	ROM_LOAD( "spectre_snd_1.bin", 0x000000, 0x010000, CRC(ecdf085b) SHA1(117c63f7672112308bfe64527148ee66f8c26c12) )
	ROM_LOAD( "spectre_snd_2.bin", 0x010000, 0x010000, CRC(55087557) SHA1(a3f2613a27defa547f8c2e46ee0cdf9ee18678be) )
ROM_END



ROM_START( sc1spit )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "spitfire.bin", 0x0000, 0x010000, CRC(557cdd61) SHA1(1c4a6c969569267e61119b2cd9e506d948c35517) )
ROM_END



ROM_START( sc1ster )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "sterling.bin", 0x0000, 0x010000, CRC(a6e68f9a) SHA1(d49c4a0c6ab78f369217cc06f82a847db4f208b9) )
ROM_END



ROM_START( sc1str4 )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "strike4.ts", 0x8000, 0x008000, CRC(c636f698) SHA1(7373ad663966e51dd1a0737a447bd61e07cd16e2) )
ROM_END

ROM_START( sc1str4a )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "strike4.bin", 0x8000, 0x008000, CRC(1abcfb49) SHA1(f13891a38e260a72ffe841862ed73532c94f6c44) )
ROM_END

ROM_START( sc1sir )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "s.i.rich 2p 2.40 p1 9.3.90 0b49.bin", 0x8000, 0x008000, CRC(c54703f8) SHA1(9ac3af9021cf5012562b0ab057a30e11e01eef65) )
	ROM_LOAD( "s.i.rich 2p 2.40 p2 9.3.90 1a96.bin", 0x0000, 0x008000, CRC(618841ca) SHA1(2e690ca91da0a1ff36245a6f1e2ad681a6ed4f32) )

	ROM_REGION( 0x40000, "altrevs", 0 )
	ROM_LOAD( "95717077 proto.bin", 0x0000, 0x008000, CRC(38691d92) SHA1(17f33f74d221ac37249a04846670a1a1c0ee618e) )
	ROM_LOAD( "95717078 proto.bin", 0x0000, 0x008000, CRC(7a83b794) SHA1(8befa43c7afa37a296b309730d5cdfd32dfc363d) )
	ROM_LOAD( "rich2_1.rom", 0x0000, 0x008000, CRC(ee75ebcb) SHA1(968b8a0bff9779681b16456f9399b0d122b1796c) )
	ROM_LOAD( "s.i.rich 5p 2.40 p1 9.3.90.bin", 0x0000, 0x008000, CRC(6a37f38d) SHA1(1e7640446ecb6e00d57a92ab3592c389a172f257) )
	ROM_LOAD( "s.i.rich 5p 2.40 p2 9.3.90.bin", 0x0000, 0x008000, CRC(cd3df765) SHA1(798d051afbba5a474b1b619621e4425f5ff7f8db) )
	ROM_LOAD( "strike it rich a.bin", 0x0000, 0x008000, CRC(92ddbbca) SHA1(d888e663d0965d99dbfa68e3aed995e31411f2ba) )
	ROM_LOAD( "strike it rich b.bin", 0x8000, 0x008000, CRC(bdafc4c9) SHA1(5ffc46088818f0e89eb840e039296945905ca4f3) )

ROM_END



ROM_START( sc1sups )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "superstar_prg.bin", 0x0000, 0x010000, CRC(e859e20a) SHA1(b72ace14ceb0e601c8284a1b654a3e49368644b9) )

	ROM_REGION( 0x40000, "upd", 0 )
	ROM_LOAD( "superstar_snd1.bin", 0x000000, 0x010000, CRC(639467fb) SHA1(b40563735fa8053350d4a6eca9cc00cbfe2d2c11) )
	ROM_LOAD( "superstar_snd2.bin", 0x010000, 0x010000, CRC(29157c5c) SHA1(ccece847979358626819d6f265cd3eb932b5a400) )
ROM_END



ROM_START( sc1torn )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "tor930a", 0x8000, 0x008000, CRC(c645212d) SHA1(0cb0a6f15b22e3174a1600fe15a742d5f63d9ab2) )
	ROM_LOAD( "tor930b", 0x0000, 0x008000, CRC(f6181dd5) SHA1(44b12e6f66bf45e2b2a91424941b10ea5e75428f) )

	ROM_REGION( 0x40000, "altrevs", 0 )
	ROM_LOAD( "tornado.792", 0x0000, 0x010000, CRC(7e8e8ad1) SHA1(0e093b81f4ab3d202f89215b26b360aac7f32218) )

	ROM_REGION( 0x40000, "upd", 0 )
	ROM_LOAD( "torsnd1.bin", 0x0000, 0x010000, CRC(713ae672) SHA1(a6038004da7a4907eb413b5f39a00d7e131a2382) )
	ROM_LOAD( "torsnd2.bin", 0x010000, 0x010000, CRC(187f0c17) SHA1(acc8cffc91f8a92257bfd87ee8dc809139dc5301) )
ROM_END



ROM_START( sc1tri )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "20p a.bin", 0x8000, 0x008000, CRC(d162ebd5) SHA1(cfab100ab8cc34b61108fc7b8a3ec1f1b22f90ba) )
	ROM_LOAD( "20p b.bin", 0x0000, 0x008000, CRC(ef5bc525) SHA1(2881b9292f9dd7376997992941e07d288640703b) )

	ROM_REGION( 0x40000, "altrevs", 0 )
	ROM_LOAD( "957172.41 std var% a.bin", 0x0000, 0x008000, CRC(b314f739) SHA1(793c01f292c5144a1f5975b276b4985c565a2833) )
	ROM_LOAD( "957172.42 std var% b.bin", 0x0000, 0x008000, CRC(d8d70cac) SHA1(8137ab06912bc27f26bcbb800a09b095ba2175bb) )
	ROM_LOAD( "957182.41 proto var% a.bin", 0x0000, 0x008000, CRC(1af55594) SHA1(9e65c7bbb37d75662e4243fc6ba13f249183e2a3) )
	ROM_LOAD( "957272.41 std a.bin", 0x0000, 0x008000, CRC(635ded7e) SHA1(3e8bda8c2fa6fc8e46ba3e3a70dfb183fad3223b) )
	ROM_LOAD( "957272.42 std b.bin", 0x0000, 0x008000, CRC(634b1927) SHA1(60f2bf02a12021da3c7995122dff85ce7831ed42) )
	ROM_LOAD( "957282.41 proto std a.bin", 0x0000, 0x008000, CRC(e5999ec8) SHA1(0a11544da03fc2197dc2cc6780cbaeee55372069) )
ROM_END



ROM_START( sc1typ )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "club-typhoon_std_ac_rot_ass.bin", 0x8000, 0x008000, CRC(5d6819b2) SHA1(14cc0b3b5f42f4ff92ff96629737b9e75bb0ea10) )
	ROM_LOAD( "club-typhoon_std_ac_rot_bss.bin", 0x0000, 0x008000, CRC(0f3e160d) SHA1(3be936fe288b23e2f35be7d5638894776d676c11) )

	ROM_REGION( 0x40000, "altrevs", 0 )
	ROM_LOAD( "club-typhoon_dat_ac_rot_ass.bin", 0x0000, 0x008000, CRC(3a67d55e) SHA1(ce75e5c07795b3c67f234a869efb78fbf22b76c2) )
	ROM_LOAD( "club-typhoon_dat_ac_rot_bss.bin", 0x0000, 0x008000, CRC(0f3e160d) SHA1(3be936fe288b23e2f35be7d5638894776d676c11) )

	ROM_REGION( 0x40000, "xxxx", 0 )
	ROM_LOAD( "club-typhoon_snd_a_(inhex)ss.hex", 0x0000, 0x026efc, CRC(c913008a) SHA1(9b75a40670db0fbe8a0f6fc54784d3b415a975f5) )
	ROM_LOAD( "club-typhoon_snd_b_(inhex)ss.hex", 0x0000, 0x023972, CRC(2106a5f1) SHA1(17e0f24c4e9a8ba227c5a6ec63bcba3d8796f7f7) )

	ROM_REGION( 0x20000, "upd", 0 )
	ROM_LOAD( "club-typhoon_snd_a.bin", 0x000000, 0x010000, CRC(ffec0dde) SHA1(a8c66a6ebb4d805e04d7eb7d1fe2ecd90e7eee54) )
	ROM_LOAD( "club-typhoon_snd_b.bin", 0x010000, 0x010000, CRC(52e36599) SHA1(4bc003a08e666f9e1abfe00e82bb43a33009b6f2) )
ROM_END



ROM_START( sc1ult )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "ultimate.bin", 0x0000, 0x010000, CRC(66c34ef8) SHA1(2c7e2e826f6bd7a31cb3432dc74ebe382c131225) )

	ROM_REGION( 0x20000, "upd", 0 )
	ROM_LOAD( "ult1.bin", 0x0000, 0x010000, CRC(7c2f52ed) SHA1(d435402459efc9311707ac691992874b56cbbeec) )
	ROM_LOAD( "ult2.bin", 0x0000, 0x010000, CRC(23b99731) SHA1(7cc1c51d9b72480d8a1020fc3621a05ba83d7629) )
ROM_END



ROM_START( sc1vent )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "ventura1.bin", 0x8000, 0x008000, CRC(7c05f39e) SHA1(84793abbffbc345bf08873ddd3185bffd8fc95df) )
	ROM_LOAD( "ventura2.bin", 0x0000, 0x008000, CRC(3c396285) SHA1(a9cb8b54ace1d228a0d365909836bc2b02db1931) )
ROM_END



ROM_START( sc1vict )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "vic1.bin", 0x8000, 0x008000, CRC(2cfbdc26) SHA1(45e492f4cba1cb90e0670fbe8f4fcd0440414316) )
	ROM_LOAD( "vic2.bin", 0x0000, 0x008000, CRC(9eab3510) SHA1(29ba3445c75a0dcdf325312fbc64e8911ba958c3) )

	ROM_REGION( 0x10000, "xxx", 0 )
	ROM_LOAD( "pal.bin", 0x0000, 0x000010, CRC(d33fb7d2) SHA1(6de1a205808bccb9bc86f630c0eda261041a3b00) )
ROM_END




ROM_START( sc1voy )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "voyager_042_arcade.bin", 0x0000, 0x010000, CRC(7db87ef9) SHA1(e2160457a862d6eba3d8348866429043df0ed2bb) )
ROM_END

ROM_START( sc1voya )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "voyager_052_single_site_euro.bin", 0x0000, 0x010000, CRC(a9042f9e) SHA1(0469ef2d2a2f9c7c4147ee8d528ec369bf943103) )
ROM_END



ROM_START( sc1winfl )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "winfalls_a.bin", 0x0000, 0x008000, CRC(4617ec80) SHA1(8ac5a47d2ae94c2869cc6645f01cfe9d880b1e5c) )
	ROM_LOAD( "winfalls_b.bin", 0x8000, 0x008000, CRC(7498ede7) SHA1(5fb66c39865ea963fb7eeb9d4813cfa5e68f709e) )
ROM_END




ROM_START( sc1winst )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "95717110 proto var% b.bin", 0x0000, 0x008000, CRC(1c2ebd26) SHA1(462baa4df7c01d101798df1d90bb5719cdc9647e) )
	ROM_LOAD( "95718109 proto var% a.bin", 0x8000, 0x008000, CRC(05d5ad4a) SHA1(5e165499601978e88159726f83310576216853c4) )

	ROM_REGION( 0x10000, "altrevs", 0 ) // are some of these different hw?
	ROM_LOAD( "95717109 var% a.bin", 0x0000, 0x008000, CRC(f8b03a06) SHA1(b919366b432d23fd9f0c986e112650048621d7b2) )
	ROM_LOAD( "95717110 var% b.bin", 0x0000, 0x008000, CRC(1c2ebd26) SHA1(462baa4df7c01d101798df1d90bb5719cdc9647e) )
	ROM_LOAD( "95719109a ws 78%.bin", 0x0000, 0x008000, CRC(cea7ff32) SHA1(ce20742bcad1eea450affab81822cfdaaf927984) )
	ROM_LOAD( "95719110b ws 78%.bin", 0x0000, 0x008000, CRC(5871aad0) SHA1(6677c94b74a2e2dcece3fdcd730fbc8034833a7d) )
	ROM_LOAD( "winning-streak_dat_ac_var_8-10pnd_ass.bin", 0x0000, 0x010000, CRC(351560f4) SHA1(b33c6bdeadeabbe5a4231b8bd5b134f9ea402133) )
	ROM_LOAD( "winning-streak_dat_ar_var_8pnd_ass.bin", 0x0000, 0x010000, CRC(a83633ef) SHA1(66caadd3127a424249fe78918ff99be833b81fad) )
	ROM_LOAD( "winning-streak_dat_ss_var_ass.bin", 0x0000, 0x010000, CRC(311550dd) SHA1(17dc789cba542e7c3c137a7e6a2a2d8869c84a7a) )
	ROM_LOAD( "winning-streak_dat_wi_ac_10pnd-20p_ass.bin", 0x0000, 0x010000, CRC(ae418733) SHA1(f63c63232056929760742fcf7f8beda387f5c597) )
	ROM_LOAD( "winning-streak_dat_wi_ac_10pnd-20p_tri3_ass.bin", 0x0000, 0x010000, CRC(39ac4021) SHA1(bd5f4d8800a794fdca8abee15acc3ea8d30c538a) )
	ROM_LOAD( "winning-streak_std_ac_tri3_ass.bin", 0x0000, 0x010000, CRC(b3e2b2d6) SHA1(0008e9d329327b4aecae5d861303c486942ef694) )
	ROM_LOAD( "winning-streak_std_ac_var_8-10pnd_ass.bin", 0x0000, 0x010000, CRC(f2d16bd5) SHA1(bd6a9da9da24459b14917386c64ecbc46c8adfda) )
	ROM_LOAD( "winning-streak_std_ar_var_8pnd_ass.bin", 0x0000, 0x010000, CRC(d7a10aeb) SHA1(7346c83df7fd3de57a1b6f0ce498daabacb11491) )
	ROM_LOAD( "winning-streak_std_ss_var_ass.bin", 0x0000, 0x010000, CRC(c88f9a6e) SHA1(19a2b708f90a53a8dcfe69d2f6c683362867daba) )
	ROM_LOAD( "winning-streak_std_wi_ac_10pnd-20p_ass.bin", 0x0000, 0x010000, CRC(ecbb7707) SHA1(ea064149c515e39b17e851bcd39092ea3ae999a0) )
	ROM_LOAD( "winning-streak_std_wi_ac_10pnd-20p_tri3_ass.bin", 0x0000, 0x010000, CRC(eb9ee9ae) SHA1(3150aec95039aa65a9126a0326e4dd10829347b2) )

	ROM_REGION( 0x80000, "upd", 0 )
	ROM_LOAD( "winningstreaksnd.bin", 0x0000, 0x080000, CRC(ba30cb97) SHA1(e7f5ca36ca993ad14b3a348868e73d7ba02be7c5) )
ROM_END



ROM_START( sc1zep )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "zepp.bin", 0x8000, 0x008000, CRC(fbc38903) SHA1(9eefef9bbde263e35a98e51b7aeb8d2348d36c06) )
	ROM_LOAD( "zepp1.bin", 0x0000, 0x008000, CRC(bfbbbc35) SHA1(5c28b6359d79c96d53319408fbc2d7cb2629185d) )
ROM_END

ROM_START( sc1wthn )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "wildthingprogamm1.bin", 0x0000, 0x010000, CRC(80157a9c) SHA1(ec8e217e17ac7f4c5bc05d9848bf5f37b2d82fac) )

	ROM_REGION( 0x80000, "upd", 0 )
	ROM_LOAD( "wildthingsound1.bin", 0x00000, 0x010000, CRC(85389209) SHA1(029dda285b035525b730b4c72ff182554f5dbe47) )
	ROM_LOAD( "wildthingsound2.bin", 0x010000, 0x010000, CRC(664ab695) SHA1(d4148ebffbe41eb1d265548991ad3cb984205497) )
ROM_END


ROM_START( sc1days )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "all2-5n.p2", 0x0000, 0x008000, CRC(fa75d835) SHA1(78e6b48bea8f1297530f08dff6bada4d228e090d) )
	ROM_LOAD( "all2-5n.p1", 0x8000, 0x008000, CRC(7d58a415) SHA1(8bd0d23ac825ba0294f2fd26e9acb87eb1f3d10c) )

	ROM_REGION( 0x20000, "altrevs", 0 )
	ROM_LOAD( "allinv1-6b.bin", 0x0000, 0x008000, CRC(59589a00) SHA1(c73b45f383f908d1257f6d031f359f73e5b2f966) )
	ROM_LOAD( "allinv1-6a.bin", 0x8000, 0x008000, CRC(36a83181) SHA1(a2cb6493efb00e9bcf76388f65098af9346f855e) )

	ROM_REGION( 0x200000, "upd", ROMREGION_ERASE00 ) // not upd?
	/* missing? */
ROM_END


ROM_START( sc1cscl )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "cldl1-8n.p1", 0x8000, 0x008000, CRC(4cb5239d) SHA1(a0f22440a5453ea28093f32856ab5417a6c82037) )
	ROM_LOAD( "cldl1-8n.p2", 0x0000, 0x008000, CRC(8feee244) SHA1(50c7eab298078def5d82bc3bebbe3e08b612bc47) )

	ROM_REGION( 0x20000, "altrevs", 0 )
	ROM_LOAD( "cash classic 2p sa1-083.bin", 0x0000, 0x010000, CRC(36a45c0d) SHA1(51eb91e42297894ae575502903833e219ac5add9) )
	ROM_LOAD( "cash classic sa1-082 5p.bin", 0x0000, 0x010000, CRC(42d68675) SHA1(ed191e03bc7b42ae1884657b4559588eeedbdf31) )
	ROM_LOAD( "clas2-0n.p2", 0x0000, 0x008000, CRC(45d40f1e) SHA1(03388a8ea809b088850865cb288af3181d3dd962) )
	ROM_LOAD( "clas2-0n.p1", 0x8000, 0x008000, CRC(ebd514b1) SHA1(5267b49de98f8a93ac206f68d56ee12e1d228a7d) )

	ROM_REGION( 0x200000, "upd", ROMREGION_ERASE00 ) // not upd?
	/* missing? */
ROM_END

ROM_START( sc1driv )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "ddslb1-3.p2", 0x0000, 0x008000, CRC(32896702) SHA1(1ef36daca6bf3f45dfff5edc401bdbd313ad9121) )
	ROM_LOAD( "ddslb1-3.p1", 0x8000, 0x008000, CRC(81fc84a7) SHA1(f0d5a181d4ca027df2c5ca11573eb7687b3abf29) )

	ROM_REGION( 0x20000, "altrevs", 0 )
	ROM_LOAD( "ddsnl1-3.p1", 0x0000, 0x008000, CRC(96f8bc52) SHA1(de0d180d4640eef451984f466be8732d0a08cee8) )
	ROM_LOAD( "ddsnl1-3.p2", 0x0000, 0x008000, CRC(a5b663c8) SHA1(5b6675874ff4e3a5c74dbd66c4a47c34d36f1222) )
	ROM_LOAD( "dslb1-5.p1", 0x0000, 0x008000, CRC(6adaf17b) SHA1(8930daac71fbe3f7eb91358d7101f2b8d05d224e) )
	ROM_LOAD( "dslb1-5.p2", 0x0000, 0x008000, CRC(193e6aaa) SHA1(f083747a9cad72690b01181cc46ae7bdc3de6ea6) )
	ROM_LOAD( "dsnl1-6.p1", 0x0000, 0x008000, CRC(174c4432) SHA1(82519ede8220d3d717ee0ebe57374357afe38949) )
	ROM_LOAD( "dsnl1-6.p2", 0x0000, 0x008000, CRC(0eb10c01) SHA1(16456ec1e32bfbd873bdebd6a760041bc9cd8648) )

	ROM_REGION( 0x200000, "upd", ROMREGION_ERASE00 ) // not upd?
	ROM_LOAD( "ds_snd1.bin", 0x000000, 0x020000, CRC(a9d7e8ec) SHA1(5b1d459d378e23d3108a1190b5988eebedf95667) )
	ROM_LOAD( "ds_snd2.bin", 0x020000, 0x020000, CRC(3b67c1b3) SHA1(8b9dbff45955f72a73fb739b5e74aa2f9c23dd08) )
	ROM_LOAD( "ds_snd3.bin", 0x040000, 0x020000, CRC(00c252ec) SHA1(5de2e70f142a71f22eeb28a271ca9d7809322faa) )
ROM_END

ROM_START( sc1vsd )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "supd1-4n.p1", 0x8000, 0x008000, CRC(ad581f7d) SHA1(99b9bf1016cd52467f5c9f6e427305e81033e82f) )

	ROM_REGION( 0x200000, "upd", ROMREGION_ERASE00 ) // not upd?
	/* missing? */
ROM_END


ROM_START( sc1moonl )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "moon lite 5p 86var v5.1.bin", 0x8000, 0x008000, CRC(31db928a) SHA1(0e07c11bf85a13df62bb704a03a42712d6e7ff62) )

	ROM_REGION( 0x200000, "upd", ROMREGION_ERASE00 )
ROM_END

ROM_START( sc1ltdv )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "little devil v4 arcade.bin", 0x8000, 0x008000, CRC(ff32cdcf) SHA1(84bb86e30ace57aa8f591a3778801d44fb3f8fe1) )

	ROM_REGION( 0x200000, "upd", ROMREGION_ERASE00 )
ROM_END

ROM_START( sc1t1k )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "top1000b.bin", 0x8000, 0x008000, CRC(0124b7c0) SHA1(620196e7580f44423ede6644f76e37091fdf30b6) )
	ROM_LOAD( "top1000a.bin", 0x0000, 0x008000, CRC(c986ee8b) SHA1(e5a600942e725d0ad6be10fbac7fb05eb0d2b07f) )

	ROM_REGION( 0x200000, "upd", ROMREGION_ERASE00 )
	/* missing? */
ROM_END

ROM_START( sc1dip )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "dip0111.bin", 0x0000, 0x010000, CRC(19632509) SHA1(69c9947da11892b99e9936675d0b1bdabdc16ae8) )

	ROM_REGION( 0x200000, "ram", ROMREGION_ERASE00 ) // is this just some default settings?
	ROM_LOAD( "ram.bin", 0x0000, 0x002000, CRC(3962d8cf) SHA1(b893a92d467e8f5ffc2cffa8a7121d92fe2492eb) )

	ROM_REGION( 0x200000, "upd", ROMREGION_ERASE00 )
	/* missing? */
ROM_END

ROM_START( sc1lamb )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "lambada.bin", 0x0000, 0x010000, CRC(4321495c) SHA1(d3ef15d2a1b2c7aec33ac226c89a7a0c0a18884a) )

	ROM_REGION( 0x200000, "upd", ROMREGION_ERASE00 )
	/* missing? */
ROM_END

ROM_START( sc1reply )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "repl0110.bin", 0x0000, 0x010000, CRC(b2bfa2fb) SHA1(9c704321428c05f97593ea7541ba1a08ff448571) )

	ROM_REGION( 0x80000, "upd", 0 )
	ROM_LOAD( "repl0110snd.bin", 0x0000, 0x010000, CRC(86547dc7) SHA1(4bf64f22e84c0ee82d961b0ba64932b8bf6a521f) )
ROM_END


ROM_START( sc1smoke )
	ROM_REGION( 0x80000, "maincpu", 0 )
	ROM_LOAD( "sb6-3_0b.bin", 0x0000, 0x008000, CRC(31647e2f) SHA1(35dd1bef0dd72fd45c063f181cd190f8d21df207) )
	ROM_LOAD( "sb6-3_0a.bin", 0x8000, 0x008000, CRC(45ca0067) SHA1(be1947d055320c101ea75c669733b19d2f61a0f9) )

	ROM_REGION( 0x80000, "altrevs", 0 )
	ROM_LOAD( "sb8-5_0a.bin", 0x0000, 0x008000, CRC(eafe5fac) SHA1(4798a37ada523d078f2e10976c5f90cccab1c406) )
	ROM_LOAD( "sb8-5_0b.bin", 0x0000, 0x008000, CRC(0aac6b91) SHA1(85b4dfe15d456b7d808295c890264163bc6115f1) )
	ROM_LOAD( "svb58pa", 0x0000, 0x008000, CRC(4496ce3d) SHA1(400dee4249fd930473cb003d85b25bb991041bc6) )

	ROM_REGION( 0x200000, "upd", ROMREGION_ERASE00 )
ROM_END

ROM_START( sc1ccroc )
	ROM_REGION( 0x80000, "maincpu", 0 )
	ROM_LOAD( "cc6-4_0b.bin", 0x0000, 0x008000, CRC(4093cbe6) SHA1(6ea678d0e288bb075d58ef72089ae387d6285477) )
	ROM_LOAD( "cc6-4_0a.bin", 0x8000, 0x008000, CRC(c3d963e8) SHA1(35688841e102c264124c23de526417db618ea898) )

	ROM_REGION( 0x80000, "altrevs", 0 )
	ROM_LOAD( "cc8-7_0a.bin", 0x0000, 0x008000, CRC(4a6cd887) SHA1(27f394a63bdb68d35d6eecb6b0f6b3f3f61d36b5) )
	ROM_LOAD( "cc8-7_0b.bin", 0x0000, 0x008000, CRC(a638c58a) SHA1(0a5d53a9c0f772263c7a726f90943a1ccfe5db20) )
	ROM_LOAD( "cct-8_0a.bin", 0x0000, 0x008000, CRC(a90a5f23) SHA1(befb389cc6ff045462f02c2aa9025d92c47da0fa) )
	ROM_LOAD( "cct-8_0b.bin", 0x0000, 0x008000, CRC(1e0e93f3) SHA1(e5ceef529bd406d2b395b6e24cff370422b0e1f2) )
	ROM_LOAD( "crocs6a.bin", 0x0000, 0x008000, CRC(087330cb) SHA1(f143a8a44024f0f851a8b677f42b9d4011ab92d4) )
	ROM_LOAD( "crocs6b.bin", 0x0000, 0x008000, CRC(42ad6fb0) SHA1(d60961d9993a8458668177013d7561d0b7423cda) )
	ROM_LOAD( "cs1_1.rom", 0x0000, 0x040000, CRC(f4c6f9f1) SHA1(4277ff51dc91c35d4c6e9ab1c16e087ef7e8d140) )
	ROM_LOAD( "cs1_2.rom", 0x0000, 0x040000, CRC(ba4dad49) SHA1(795342d5fd3deaa058a20d491206c028c529fd55) )

	ROM_REGION( 0x200000, "upd", ROMREGION_ERASE00 )
ROM_END

ROM_START( sc1crocr )
	ROM_REGION( 0x80000, "maincpu", 0 )
	ROM_LOAD( "croc58pb", 0x0000, 0x010000, CRC(95d2b0ac) SHA1(369a2f5efc981aa03780b80e0b14d5171c25e72b) ) // 2nd half empty
	ROM_LOAD( "croc58pa", 0x8000, 0x010000, CRC(39501e80) SHA1(f03bc602df839374adf7722af295cee562353782) ) // 2nd half empty

	ROM_REGION( 0x200000, "upd", ROMREGION_ERASE00 )
ROM_END

ROM_START( sc1btclk )
	ROM_REGION( 0x80000, "maincpu", 0 )
	ROM_LOAD( "bc6-4_0b.bin", 0x0000, 0x008000, CRC(106265c8) SHA1(6465f7e868c5b04776fee69295a52197abb45ad0) )
	ROM_LOAD( "bc6-4_0a.bin", 0x8000, 0x008000, CRC(750645e7) SHA1(65eee2a00a1914bb8dc989b131eaa39d2881105d) )

	ROM_REGION( 0x80000, "altrevs", 0 )
	ROM_LOAD( "bc8-5_0a.bin", 0x0000, 0x008000, CRC(f8fafc49) SHA1(7d8109fdabe37c7e958696512d3c2c35f9890bee) )
	ROM_LOAD( "bc8-5_0b.bin", 0x0000, 0x008000, CRC(4be7220f) SHA1(5eb2b3fd05ff06b645f16bf95f6766b8bea82525) )
	ROM_LOAD( "btc58pa", 0x0000, 0x010000, CRC(d21e5ed9) SHA1(99c189fde84f5abbdcd85d1f816c61f8fe72554e) )
	ROM_LOAD( "btc58pb", 0x0000, 0x010000, CRC(4cfde48a) SHA1(8567667f4af96fd00a807380a65fe809cd051c76) )

	ROM_REGION( 0x200000, "upd", ROMREGION_ERASE00 )
ROM_END




ROM_START( sc1btbc )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "beatb34", 0x8000, 0x008000, CRC(0791f889) SHA1(f090b9aacdbb33cc0934f53621e43520b970d789) )
	ROM_REGION( 0x200000, "upd", ROMREGION_ERASE00 )
ROM_END


ROM_START( sc1boncl )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "cb_v91.bin", 0x8000, 0x008000, CRC(a1b902f4) SHA1(47bff5f0921800052ac99fd7b945ea05fc5951d6) )
	ROM_REGION( 0x200000, "upd", ROMREGION_ERASE00 )
ROM_END



ROM_START( sc1clins )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "cashlines5p2.40p1 9.3.90.bin", 0x8000, 0x008000, CRC(cb69c335) SHA1(8fe302274d01e98f8636fbc44eb4736180345b16) )
	ROM_LOAD( "cashlines5p2.40p2 9.3.90.bin", 0x0000, 0x008000, CRC(068959a2) SHA1(6c212ceb756024662ed880b66b4c6aac21b0c726) )
	ROM_REGION( 0x200000, "upd", ROMREGION_ERASE00 )
ROM_END


ROM_START( sc1clinsa )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "cashlines2p2.40p1 9.3.90.bin", 0x8000, 0x008000, CRC(4dcdfcd1) SHA1(38b67a2450ededd9cf27b9f5d5fffe45f4e4b80d) )
	ROM_LOAD( "cashlines2p2.40p2 9.3.90.bin", 0x0000, 0x008000, CRC(0a4d6692) SHA1(9437a0ed1fb9eb706dede7a6b1670e2bd873d7fe) )
	ROM_REGION( 0x200000, "upd", ROMREGION_ERASE00 )
ROM_END

ROM_START( sc1clinsb )
	ROM_REGION( 0x10000, "maincpu", 0 )
//  ROM_LOAD( "95718066 proto.bin", 0x0000, 0x008000, CRC(0977e287) SHA1(e937a3787d4cd056c5f9944bca1532b84ed335f6) )
	ROM_LOAD( "clines proto a.bin", 0x8000, 0x008000, CRC(0977e287) SHA1(e937a3787d4cd056c5f9944bca1532b84ed335f6) )
	ROM_LOAD( "clines proto b.bin", 0x0000, 0x008000, CRC(fca396e1) SHA1(3304a58a30fd0c79e8d1decd4bd8792d3acbad3e) )
	ROM_REGION( 0x200000, "upd", ROMREGION_ERASE00 )
ROM_END

ROM_START( sc1clinsc )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "39370028a.bin", 0x8000, 0x008000, CRC(e0250ea4) SHA1(01cc9013c37bc22f5ab69565d453ece99f739e6b) )
	ROM_LOAD( "39370028b.bin", 0x0000, 0x008000, CRC(fca396e1) SHA1(3304a58a30fd0c79e8d1decd4bd8792d3acbad3e) )
//  ROM_LOAD( "95717067b.bin", 0x0000, 0x008000, CRC(fca396e1) SHA1(3304a58a30fd0c79e8d1decd4bd8792d3acbad3e) )
	ROM_REGION( 0x200000, "upd", ROMREGION_ERASE00 )
ROM_END

ROM_START( sc1clinsd )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "957172.00 all cash.bin", 0x8000, 0x008000, CRC(cafc2409) SHA1(125f7c1826e58619a53b56ecd4f5b0b7f607aeef) )
	ROM_LOAD( "957172.01 all cash.bin", 0x0000, 0x008000, CRC(27d941cf) SHA1(797d47c15d6a52f5647a566eb8ad1985324d81cb) )
	ROM_REGION( 0x200000, "upd", ROMREGION_ERASE00 )
ROM_END

ROM_START( sc1clinse ) // bad? (SUMCHECK ERROR)
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "957171.11 var% a.bin", 0x8000, 0x008000, CRC(1e74ef1a) SHA1(6c70f9b7f3caf6a5e9734b2e4ee74985c2b169d6) )
	ROM_LOAD( "957171.12 var% b.bin", 0x0000, 0x008000, CRC(80243558) SHA1(6b7cc811998d11397e5fa03a50154d165997ae7b) )
	ROM_REGION( 0x200000, "upd", ROMREGION_ERASE00 )
ROM_END


ROM_START( sc1clb3 )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "cl3000.bin", 0x0000, 0x010000, CRC(998b58fa) SHA1(73b2837d6287667f16c64edada1e3ec5ffa54c74) )

	ROM_REGION( 0x200000, "upd", ROMREGION_ERASE00 )
	ROM_LOAD( "95000593.ic7", 0x000000, 0x010000, CRC(743d4ecd) SHA1(23c2a3673d6b09bc829297751c283de444d32fa3) )
	ROM_LOAD( "95000594.ic8", 0x010000, 0x010000, CRC(9e143e49) SHA1(28547cc2f271f76a29d332f670e47a8bb836593e) )
ROM_END




ROM_START( sc1czbrk )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "crazybreakp1.bin", 0x0000, 0x008000, CRC(47cbb5fd) SHA1(b5a7a20f9874f1010f7fc973d0cc5fcb87beaaf5) )
	ROM_LOAD( "crazybreakp2.bin", 0x8000, 0x008000, CRC(71bfb2fe) SHA1(3371421268a1e0a4518eafd27b2c23a0c7475e11) )
	ROM_REGION( 0x200000, "upd", ROMREGION_ERASE00 )
ROM_END


ROM_START( sc1energ )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "energy_v109_u2.bin", 0x8000, 0x008000, CRC(ce5da71b) SHA1(c0cb687523bf7a8f42740dd3f54999eaa1db3cd0) )
	ROM_LOAD( "energy_v109_u4.bin", 0x0000, 0x008000, CRC(bde92e45) SHA1(ae1b73ecd59131a11202487ecb4d34fc68e4101d) )
	ROM_REGION( 0x200000, "upd", ROMREGION_ERASE00 )
ROM_END


ROM_START( sc1frpus )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "95750025.p1", 0x0000, 0x010000, CRC(75d21cbf) SHA1(8161dec9b0533383acc6172da564f1353e4367c1) )

	ROM_REGION( 0x80000, "altrevs", 0 )
	ROM_LOAD( "95752025.p1", 0x0000, 0x010000, CRC(0d223a7d) SHA1(7b110989b988f5fc57eac2b21b9f0cdb326174a0) )
	ROM_REGION( 0x200000, "upd", ROMREGION_ERASE00 )
ROM_END



ROM_START( sc1hipt )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "high point 6 b 20p.bin", 0x0000, 0x008000, CRC(535c1df1) SHA1(99d3033ee708c27134d461591eb7d19a573768d4) )
	ROM_LOAD( "high point 6 a 20p.bin", 0x8000, 0x008000, CRC(228c3eef) SHA1(c60da857fa5630809b072c20cf1f24ee26c38d0b) )
	ROM_REGION( 0x200000, "upd", ROMREGION_ERASE00 )
ROM_END

ROM_START( sc1hipta )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "high point 69 6.bin", 0x8000, 0x008000, CRC(82564d75) SHA1(436b75e6617a6c2bb89ea0994696928b8452317d) )
	ROM_LOAD( "high point 70 6.bin", 0x0000, 0x008000, CRC(752b4c6e) SHA1(30503128ccca5c88e66174bd3e54b115eded1db6) )
	ROM_REGION( 0x200000, "upd", ROMREGION_ERASE00 )
ROM_END



ROM_START( sc1satse )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "satellitese.bin", 0x0000, 0x010000, CRC(de88d59c) SHA1(0df9ff2aa4be2634bc66e8f5539a7aa8c71b340a) )
	ROM_REGION( 0x200000, "upd", ROMREGION_ERASE00 )
ROM_END

ROM_START( sc1strk )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "strike.bin", 0x0000, 0x010000, CRC(8bfae942) SHA1(325b74e3df527ad56e68b58b206fb3a491a44305) )
	ROM_REGION( 0x200000, "upd", ROMREGION_ERASE00 )
	ROM_LOAD( "strikesoundp1.bin", 0x0000, 0x010000, CRC(bca5518c) SHA1(1b66e72e110702754eb3991f351cce689d6ad41c) )
	ROM_LOAD( "strikesoundp2.bin", 0x0000, 0x010000, CRC(50d6c506) SHA1(cb9851ebad21c0b14cf3d57159034a8660a32f74) )
ROM_END

ROM_START( sc1supfl )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "superflushse.bin", 0x0000, 0x010000, CRC(50f890d1) SHA1(6edad44aaba069b2a3cc2bd16ed4cf383d6f7029) )
	ROM_REGION( 0x200000, "upd", ROMREGION_ERASE00 )
ROM_END


ROM_START( sc1ofs56 )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "ofs56cp1", 0x8000, 0x008000, CRC(928c0a32) SHA1(1c83e497d62112850ff1607f9b20a12fe07a88cc) )
	ROM_LOAD( "ofs56cp2", 0x0000, 0x008000, CRC(c3af2861) SHA1(4fe47355ea9431360f17ff4004a7529111aa1d50) )
	ROM_REGION( 0x200000, "upd", ROMREGION_ERASE00 )
ROM_END



ROM_START( sc1wof )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "frds1-4n.p1", 0x8000, 0x008000, CRC(add2f2f8) SHA1(6c9852493b5e13cc694deacb96fe6d04f49e5c30) )
	ROM_LOAD( "frds1-4n.p2", 0x0000, 0x008000, CRC(60e56657) SHA1(4f02be663cfb36beeaa47be37fca7447d6ff9ebc) )
	ROM_REGION( 0x200000, "upd", ROMREGION_ERASE00 )
ROM_END

ROM_START( sc1wofa )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "wfor3-3n.p1", 0x8000, 0x008000, CRC(a6bb27bd) SHA1(abe240ecb5ceee1012d0ff547380e2d122380efc) )
	ROM_LOAD( "wfor3-3n.p2", 0x0000, 0x008000, CRC(610f5700) SHA1(1752604a2ea3ac658d86b5a5baea03d67b8a6e99) )
	ROM_REGION( 0x200000, "upd", ROMREGION_ERASE00 )
ROM_END

ROM_START( sc1wofb )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "wof.bin", 0x0000, 0x010000, CRC(6aa9ccce) SHA1(d8781e225c97ccf2fd847ead1ae8e200358f8a96) )
	ROM_REGION( 0x200000, "upd", ROMREGION_ERASE00 )
ROM_END


ROM_START( sc1crzyc )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "craz1-5n.p1", 0x8000, 0x008000, CRC(943166ce) SHA1(9fbc97a1ede5ef18d5d5c544484b4a63f9a9901b) )
	ROM_REGION( 0x200000, "upd", ROMREGION_ERASE00 )
ROM_END


ROM_START( sc1crzyca )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "ccds1-2n.p1", 0x8000, 0x008000, CRC(62d75b51) SHA1(82c9a211e9465c04cbf7597481ee4fb3cbac9a94) )
	ROM_REGION( 0x200000, "upd", ROMREGION_ERASE00 )
ROM_END

ROM_START( sc1clbdy )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "ddpv1-1a.gg", 0x8000, 0x008000, CRC(1614ee7b) SHA1(e777a122062f24a18fbe827371e359bbdd4298e7) )
	ROM_LOAD( "ddpv1-1b.gg", 0x0000, 0x008000, CRC(692d4347) SHA1(6cdf3dbbaffe47fc026debaa74303d4ad36a5b63) )
	ROM_REGION( 0x200000, "upd", ROMREGION_ERASE00 )
ROM_END

ROM_START( sc1clbdya )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "dyn1-6n.p1", 0x8000, 0x008000, CRC(9cb42e58) SHA1(bb92e7618efb9a95e96d55d6ee46ba4f08cb825b) )
	ROM_LOAD( "dyn1-6n.p2", 0x0000, 0x008000, CRC(425b8cf6) SHA1(8b3dd294ff965103b5621da462b39629445456b9) )
	ROM_REGION( 0x200000, "upd", ROMREGION_ERASE00 )
ROM_END


ROM_START( sc1chqfl )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "cflg1-5n.p1", 0x8000, 0x008000, CRC(2337b8ed) SHA1(c27b3b91ca52dd7edb05743753b4510c05f29055) )
	ROM_REGION( 0x200000, "upd", ROMREGION_ERASE00 )
ROM_END

ROM_START( sc1s1000 )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "super1000.bin", 0x8000, 0x008000, CRC(879e56e6) SHA1(5c0a08375a30213142e1d3835ea46462d882982d) )
	ROM_REGION( 0x200000, "upd", ROMREGION_ERASE00 )
ROM_END

ROM_START( sc1cdm )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "dd13b.bin", 0x0000, 0x008000, CRC(7a663587) SHA1(6d03a34047ba5f995b1877fc4c0ab9703aa4defc) )
	ROM_LOAD( "dd13a.bin", 0x8000, 0x008000, CRC(e674bca9) SHA1(31481d791f3aaf1d4ba790924f0f9e4100a82da5) )

	ROM_REGION( 0x10000, "altrevs", 0 )
	ROM_LOAD( "dd13ap.bin", 0x0000, 0x008000, CRC(84a51666) SHA1(89cf10c7e732b5f77b798bf58fe8ebfc701da57b) )

	ROM_REGION( 0x200000, "upd", ROMREGION_ERASE00 )
	ROM_LOAD( "dd_snd1.bin", 0x000000, 0x010000, CRC(c00a70ab) SHA1(c0014b3e4308281203921994f41f19e0243148e0) )
	ROM_LOAD( "dd_snd2.bin", 0x010000, 0x010000, CRC(c03827f6) SHA1(16e844fb83d79d1e4fbb0069debaf71af5ad6814) )
ROM_END

ROM_START( sc1hfcc )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "cz14b.bin", 0x0000, 0x008000, CRC(976233ca) SHA1(554da440a0fe1d66fa95bef51ac168cec35d1636) )
	ROM_LOAD( "cz14a.bin", 0x8000, 0x008000, CRC(34324f0b) SHA1(946ff8fa40788748a0caabd48d125f2a4f9c36c3) )

	ROM_REGION( 0x10000, "altrevs", 0 )
	ROM_LOAD( "cz14ap.bin", 0x0000, 0x008000, CRC(56e3e5c4) SHA1(3017007e03139204732f7945ded61d35499055ac) )

	ROM_REGION( 0x200000, "upd", ROMREGION_ERASE00 )
	/* Missing? */
ROM_END

ROM_START( sc1twice )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "twiceasnice-arcstd.bin", 0x8000, 0x008000, CRC(4ba39f58) SHA1(185513023e0c87d926e0e821ed94f121182880c1) )
	ROM_REGION( 0x200000, "upd", ROMREGION_ERASE00 )
ROM_END



ROM_START( sc1chain )
	ROM_REGION( 0x10000, "maincpu", 0 )
//  ROM_LOAD( "95717174 b.bin", 0x0000, 0x008000, CRC(6cdc8d15) SHA1(582e5e7bcefe0085917d3499b7c83e27c19662d2) )
//  ROM_LOAD( "95717173 a.bin", 0x8000, 0x008000, CRC(4989e6c6) SHA1(17184c6a3624dfaa61bc4ddb3ac1813949eaf834) )
	ROM_LOAD( "95717174.bin", 0x0000, 0x008000, CRC(6cdc8d15) SHA1(582e5e7bcefe0085917d3499b7c83e27c19662d2) )
	ROM_LOAD( "95717173.bin", 0x8000, 0x008000, CRC(4989e6c6) SHA1(17184c6a3624dfaa61bc4ddb3ac1813949eaf834) )

	ROM_REGION( 0x10000, "altrevs", 0 )
	ROM_LOAD( "95716173 proto.bin", 0x0000, 0x008000, CRC(4f806f1d) SHA1(cfa8bcc2afbb47e549836d968c3390bef04c6c30) )
	ROM_LOAD( "95717210.bin", 0x0000, 0x008000, CRC(102d2bc8) SHA1(8ed5f44e6014e21f677762e40076d648901d1ff2) )
	ROM_LOAD( "95717211.bin", 0x0000, 0x008000, CRC(ed781e00) SHA1(67ebb58beda5123f061a22dacd008f1feb75b8d9) )

	ROM_REGION( 0x200000, "upd", ROMREGION_ERASE00 )
	/* Missing? */
ROM_END

ROM_START( sc1potp )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "potp 95715159 2x1.bin", 0x6000, 0x002000, CRC(b47cd8f3) SHA1(bf26fdc440a111dc1326b200281c2dff5c517c67) )
	ROM_LOAD( "potp 95717908 2x1.bin", 0x8000, 0x008000, CRC(953c3e78) SHA1(f14ab2c4337e93605be4baac51b8ad3b9bf0e155) )

	ROM_REGION( 0x200000, "upd", ROMREGION_ERASE00 )
ROM_END

ROM_START( sc1potpa )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "potp 95715146.bin", 0x6000, 0x002000, CRC(9557ebc4) SHA1(a9d3b2d901875b9d53ac9500acdb9b725b4edcb5) )
	ROM_LOAD( "potp 95350166.bin", 0x8000, 0x008000, CRC(45f0effa) SHA1(afd7aabac7da04b5960c2cc55863b917a2692c4f) )

	ROM_REGION( 0x200000, "upd", ROMREGION_ERASE00 )
ROM_END

// these mostly look like the same thing, and clearly have the BFM address scramble, but might be
// bad dumps / missing the first half (in all cases it's either 0xff or a mirror of the 2nd half)
// alternatively there might be an additional scramble
ROM_START( sc1scunk )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "su_cashx", 0x0000, 0x010000, CRC(1ed97ef6) SHA1(1aaf911369dc814ee2edf5d59baa2961bfc73168) )
	ROM_LOAD( "s_nudge.p1", 0x0000, 0x010000, CRC(ca5fdbca) SHA1(60079aeb4904e42a4a45feb7f31cf6c71b611845) )
	ROM_LOAD( "s_ghost.p1", 0x0000, 0x010000, CRC(e1e63cfd) SHA1(1e966758eb890eb8515bd943e7f8077e2948e22c) )
	ROM_LOAD( "s.che", 0x0000, 0x010000, CRC(e285d761) SHA1(1d5aebebd41d388bc69777610dc3ee449e4a504e) )

	ROM_REGION( 0x200000, "upd", ROMREGION_ERASE00 )
ROM_END



/////////////////////////////////////////////////////////////////////////////////////


GAME( 1988, sc1lotus		, 0			, scorpion1			, scorpion1	, lotse			, 0,       "BFM/ELAM", "Lotus SE (Dutch)", GAME_NOT_WORKING|GAME_SUPPORTS_SAVE|GAME_REQUIRES_ARTWORK|GAME_MECHANICAL )
GAME( 1988, sc1roul			, 0			, scorpion1			, scorpion1	, rou029		, 0,       "BFM/ELAM", "Roulette (Dutch, Game Card 39-360-129?)", GAME_NOT_WORKING|GAME_SUPPORTS_SAVE|GAME_REQUIRES_ARTWORK|GAME_MECHANICAL )
GAME( 1990, sc1clatt		, 0			, scorpion1			, clatt		, clatt			, 0,       "BFM",      "Club Attraction (UK, Game Card 39-370-196)", GAME_NOT_WORKING|GAME_SUPPORTS_SAVE|GAME_REQUIRES_ARTWORK|GAME_MECHANICAL )
GAME( 1990, sc1clatta		, sc1clatt	, scorpion1			, clatt		, clatt			, 0,       "BFM",      "Club Attraction (set 2)", GAME_NOT_WORKING|GAME_SUPPORTS_SAVE|GAME_REQUIRES_ARTWORK|GAME_MECHANICAL )
GAME( 198?, sc1actv8		, 0			, scorpion1_viper	, scorpion1	, nocrypt		, 0,       "BFM",      "Active 8 (Bellfruit) (Scorpion 1)", GAME_NOT_WORKING|GAME_SUPPORTS_SAVE|GAME_REQUIRES_ARTWORK|GAME_MECHANICAL )
GAME( 198?, sc1armad		, 0			, scorpion1			, scorpion1	, lotse			, 0,       "BFM/ELAM", "Armada (Bellfruit) (Dutch) (Scorpion 1)", GAME_NOT_WORKING|GAME_SUPPORTS_SAVE|GAME_REQUIRES_ARTWORK|GAME_MECHANICAL )
GAME( 198?, sc1bartk		, 0			, scorpion1_viper	, clatt		, lotse			, 0,       "BFM",      "Bar Trek (Bellfruit) (Scorpion 1)", GAME_NOT_WORKING|GAME_SUPPORTS_SAVE|GAME_REQUIRES_ARTWORK|GAME_MECHANICAL )
GAME( 198?, sc1barcd		, 0			, scorpion1_viper	, clatt		, lotse			, 0,       "BFM",      "Barcode (Bellfruit) (Scorpion 1)", GAME_NOT_WORKING|GAME_SUPPORTS_SAVE|GAME_REQUIRES_ARTWORK|GAME_MECHANICAL )
GAME( 198?, sc1bigmt		, 0			, scorpion1_viper	, clatt		, nocrypt		, 0,       "BFM",      "The Big Match (Bellfruit) (Scorpion 1)", GAME_NOT_WORKING|GAME_SUPPORTS_SAVE|GAME_REQUIRES_ARTWORK|GAME_MECHANICAL )
GAME( 198?, sc1calyp		, 0			, scorpion1			, scorpion1	, lotse			, 0,       "BFM/ELAM", "Calypso (Bellfruit) (Dutch) (Scorpion 1)", GAME_NOT_WORKING|GAME_SUPPORTS_SAVE|GAME_REQUIRES_ARTWORK|GAME_MECHANICAL )
GAME( 198?, sc1carro		, 0			, scorpion1			, scorpion1	, nocrypt_bank0	, 0,       "BFM/ELAM", "Carrousel (Bellfruit) (Scorpion 1)", GAME_NOT_WORKING|GAME_SUPPORTS_SAVE|GAME_REQUIRES_ARTWORK|GAME_MECHANICAL )
GAME( 198?, sc1cshat		, 0			, scorpion1			, scorpion1 , lotse 		, 0,       "BFM",      "Cash Attraction (Bellfruit) (Scorpion 1)", GAME_NOT_WORKING|GAME_SUPPORTS_SAVE|GAME_REQUIRES_ARTWORK|GAME_MECHANICAL )
GAME( 198?, sc1cshcd		, 0			, scorpion1			, scorpion1	, lotse			, 0,       "BFM/ELAM", "Cash Card (Bellfruit) (Dutch) (Scorpion 1)", GAME_NOT_WORKING|GAME_SUPPORTS_SAVE|GAME_REQUIRES_ARTWORK|GAME_MECHANICAL )
GAME( 198?, sc1cshcda		, sc1cshcd	, scorpion1			, scorpion1	, lotse			, 0,       "BFM",	   "Cash Card (Bellfruit) (Scorpion 1, set 1)", GAME_NOT_WORKING|GAME_SUPPORTS_SAVE|GAME_REQUIRES_ARTWORK|GAME_MECHANICAL )
GAME( 198?, sc1cshcdb		, sc1cshcd	, scorpion1			, scorpion1	, lotse			, 0,       "BFM",      "Cash Card (Bellfruit) (Scorpion 1, set 2)", GAME_NOT_WORKING|GAME_SUPPORTS_SAVE|GAME_REQUIRES_ARTWORK|GAME_MECHANICAL )
GAME( 198?, sc1ccoin		, 0			, scorpion1			, scorpion1	, lotse			, 0,       "BFM",      "Cash Coin (Bellfruit) (Scorpion 1)", GAME_NOT_WORKING|GAME_SUPPORTS_SAVE|GAME_REQUIRES_ARTWORK|GAME_MECHANICAL )
GAME( 198?, sc1cexpd		, 0			, scorpion1			, scorpion1	, lotse			, 0,       "BFM/ELAM", "Cash Explosion (Bellfruit) [Dutch] (Scorpion 1)", GAME_NOT_WORKING|GAME_SUPPORTS_SAVE|GAME_REQUIRES_ARTWORK|GAME_MECHANICAL )
GAME( 198?, sc1cexpl		, 0			, scorpion1			, scorpion1	, lotse			, 0,       "BFM",      "Cash Explosion (Bellfruit) (Scorpion 1)", GAME_NOT_WORKING|GAME_SUPPORTS_SAVE|GAME_REQUIRES_ARTWORK|GAME_MECHANICAL )
GAME( 198?, sc1cshwz		, 0			, scorpion1			, scorpion1	, lotse			, 0,       "BFM",      "Cash Wise (Bellfruit) (Scorpion 1)", GAME_NOT_WORKING|GAME_SUPPORTS_SAVE|GAME_REQUIRES_ARTWORK|GAME_MECHANICAL )
GAME( 198?, sc1cshin		, 0			, scorpion1			, scorpion1	, lotse			, 0,       "BFM/ELAM", "Cashino (Bellfruit) [Dutch] (Scorpion 1)", GAME_NOT_WORKING|GAME_SUPPORTS_SAVE|GAME_REQUIRES_ARTWORK|GAME_MECHANICAL )
GAME( 198?, sc1china		, 0			, scorpion1_viper	, scorpion1	, lotse			, 0,       "BFM",      "China Town Club (Bellfruit) (Scorpion 1)", GAME_NOT_WORKING|GAME_SUPPORTS_SAVE|GAME_REQUIRES_ARTWORK|GAME_MECHANICAL )
GAME( 198?, sc1class		, 0			, scorpion1			, scorpion1	, nocrypt		, 0,       "BFM/ELAM", "Classic (Bellfruit) [Dutch] (Scorpion 1)", GAME_NOT_WORKING|GAME_SUPPORTS_SAVE|GAME_REQUIRES_ARTWORK|GAME_MECHANICAL )
GAME( 198?, sc1cwcl			, 0			, scorpion1			, scorpion1	, lotse			, 0,       "BFM",      "Clockwise Club (Bellfruit) (Scorpion 1)", GAME_NOT_WORKING|GAME_SUPPORTS_SAVE|GAME_REQUIRES_ARTWORK|GAME_MECHANICAL )
GAME( 198?, sc1clown		, 0			, scorpion1			, scorpion1	, lotse			, 0,       "BFM/ELAM", "Clown Around (Bellfruit) (Scorpion 1)", GAME_NOT_WORKING|GAME_SUPPORTS_SAVE|GAME_REQUIRES_ARTWORK|GAME_MECHANICAL )
GAME( 198?, sc1cl2k			, 0			, scorpion1			, scorpion1	, nocrypt		, 0,       "BFM/ELAM", "Club 2000 (Bellfruit) (Scorpion 1)", GAME_NOT_WORKING|GAME_SUPPORTS_SAVE|GAME_REQUIRES_ARTWORK|GAME_MECHANICAL )
GAME( 198?, sc1cl2k1		, 0			, scorpion1			, scorpion1	, lotse			, 0,       "BFM/ELAM", "Club 2001 (Bellfruit) [Dutch] (Scorpion 1)", GAME_NOT_WORKING|GAME_SUPPORTS_SAVE|GAME_REQUIRES_ARTWORK|GAME_MECHANICAL )
GAME( 198?, sc1cl65			, 0			, scorpion1			, scorpion1	, lotse			, 0,       "BFM",      "Club 65 Special (Bellfruit) (Scorpion 1)", GAME_NOT_WORKING|GAME_SUPPORTS_SAVE|GAME_REQUIRES_ARTWORK|GAME_MECHANICAL )
GAME( 198?, sc1clbdm		, 0			, scorpion1			, scorpion1	, lotse			, 0,       "BFM",      "Club Diamond (Bellfruit) (Scorpion 1)", GAME_NOT_WORKING|GAME_SUPPORTS_SAVE|GAME_REQUIRES_ARTWORK|GAME_MECHANICAL )
GAME( 198?, sc1clbxp		, 0			, scorpion1			, scorpion1	, lotse			, 0,       "BFM",      "Club Explosion (Bellfruit) (Scorpion 1)", GAME_NOT_WORKING|GAME_SUPPORTS_SAVE|GAME_REQUIRES_ARTWORK|GAME_MECHANICAL )
GAME( 198?, sc1clbrn		, 0			, scorpion1			, scorpion1	, lotse			, 0,       "BFM",      "Club Runner (Bellfruit) (Scorpion 1)", GAME_NOT_WORKING|GAME_SUPPORTS_SAVE|GAME_REQUIRES_ARTWORK|GAME_MECHANICAL )
GAME( 198?, sc1clbsp		, 0			, scorpion1			, scorpion1	, lotse			, 0,       "BFM",      "Club Spinner (Bellfruit) (Scorpion 1)", GAME_NOT_WORKING|GAME_SUPPORTS_SAVE|GAME_REQUIRES_ARTWORK|GAME_MECHANICAL )
GAME( 198?, sc1clbtm		, 0			, scorpion1			, scorpion1	, lotse			, 0,       "BFM",      "Club Temptation (Bellfruit) (Scorpion 1)", GAME_NOT_WORKING|GAME_SUPPORTS_SAVE|GAME_REQUIRES_ARTWORK|GAME_MECHANICAL )
GAME( 198?, sc1clbw			, 0			, scorpion1			, scorpion1	, lotse			, 0,       "BFM",      "Club Wise (Bellfruit) (Scorpion 1)", GAME_NOT_WORKING|GAME_SUPPORTS_SAVE|GAME_REQUIRES_ARTWORK|GAME_MECHANICAL )
GAME( 198?, sc1copdd		, 0			, scorpion1			, scorpion1	, lotse			, 0,       "BFM/ELAM", "Cops 'n' Robbers Deluxe (Bellfruit) [Dutch] (Scorpion 1)", GAME_NOT_WORKING|GAME_SUPPORTS_SAVE|GAME_REQUIRES_ARTWORK|GAME_MECHANICAL )
GAME( 198?, sc1copdx		, 0			, scorpion1			, scorpion1	, lotse			, 0,       "BFM",      "Cops 'n' Robbers Deluxe (Bellfruit) (Scorpion 1)", GAME_NOT_WORKING|GAME_SUPPORTS_SAVE|GAME_REQUIRES_ARTWORK|GAME_MECHANICAL ) // is this really sc1? it does nothing
GAME( 198?, sc1count		, 0			, scorpion1			, scorpion1	, lotse			, 0,       "BFM",      "Count Cash Club (Bellfruit) (Scorpion 1)", GAME_NOT_WORKING|GAME_SUPPORTS_SAVE|GAME_REQUIRES_ARTWORK|GAME_MECHANICAL )
GAME( 198?, sc1dago			, 0			, scorpion1			, scorpion1	, lotse			, 0,       "ELAM",     "Dagobert's Vault (Bellfruit) [Dutch] (Scorpion 1)", GAME_NOT_WORKING|GAME_SUPPORTS_SAVE|GAME_REQUIRES_ARTWORK|GAME_MECHANICAL )
GAME( 198?, sc1disc			, 0			, scorpion1			, scorpion1	, lotse			, 0,       "BFM/ELAM", "Discovey (Bellfruit) [Dutch] (Scorpion 1)", GAME_NOT_WORKING|GAME_SUPPORTS_SAVE|GAME_REQUIRES_ARTWORK|GAME_MECHANICAL )
GAME( 198?, sc1dblch		, 0			, scorpion1			, scorpion1	, lotse			, 0,       "BFM",      "Double Chance (Bellfruit) (Scorpion 1)", GAME_NOT_WORKING|GAME_SUPPORTS_SAVE|GAME_REQUIRES_ARTWORK|GAME_MECHANICAL )
GAME( 198?, sc1dream		, 0			, scorpion1			, scorpion1	, lotse			, 0,       "BFM",      "Dream Machine (Bellfruit) (Scorpion 1)", GAME_NOT_WORKING|GAME_SUPPORTS_SAVE|GAME_REQUIRES_ARTWORK|GAME_MECHANICAL )
GAME( 198?, sc1final		, 0			, scorpion1			, scorpion1	, lotse			, 0,       "BFM",      "Final Touch (Bellfruit) (Scorpion 1)", GAME_NOT_WORKING|GAME_SUPPORTS_SAVE|GAME_REQUIRES_ARTWORK|GAME_MECHANICAL )
GAME( 198?, sc1flash		, 0			, scorpion1			, scorpion1	, nocrypt		, 0,       "BFM/ELAM", "Flash (Bellfruit) [Dutch] (Scorpion 1)", GAME_NOT_WORKING|GAME_SUPPORTS_SAVE|GAME_REQUIRES_ARTWORK|GAME_MECHANICAL )
GAME( 198?, sc1fruit		, 0			, scorpion1			, scorpion1	, lotse			, 0,       "BFM/ELAM", "Fruit Lines (Bellfruit) (Scorpion 1, set 1)", GAME_NOT_WORKING|GAME_SUPPORTS_SAVE|GAME_REQUIRES_ARTWORK|GAME_MECHANICAL )
GAME( 198?, sc1frtln		, sc1fruit	, scorpion1			, scorpion1	, lotse			, 0,       "BFM/ELAM", "Fruit Lines (Bellfruit) (Scorpion 1, set 2)", GAME_NOT_WORKING|GAME_SUPPORTS_SAVE|GAME_REQUIRES_ARTWORK|GAME_MECHANICAL )
GAME( 198?, sc1funh			, 0			, scorpion1			, scorpion1	, lotse			, 0,       "BFM",      "Fun House Club (Bellfruit) (Scorpion 1)", GAME_NOT_WORKING|GAME_SUPPORTS_SAVE|GAME_REQUIRES_ARTWORK|GAME_MECHANICAL )
GAME( 198?, sc1gtime		, 0			, scorpion1			, scorpion1	, lotse			, 0,       "BFM",      "Good Times (Bellfruit) (Scorpion 1)", GAME_NOT_WORKING|GAME_SUPPORTS_SAVE|GAME_REQUIRES_ARTWORK|GAME_MECHANICAL )
GAME( 198?, sc1tiara		, 0			, scorpion1			, scorpion1	, nocrypt       , 0,       "ELAM",     "Tiara (Bellfruit) (Scorpion 1)", GAME_NOT_WORKING|GAME_SUPPORTS_SAVE|GAME_REQUIRES_ARTWORK|GAME_MECHANICAL )
GAME( 198?, sc1gprix		, 0			, scorpion1			, scorpion1	, nocrypt		, 0,       "BFM/ELAM", "Grand Prix (Bellfruit) [Dutch] (Scorpion 1)", GAME_NOT_WORKING|GAME_SUPPORTS_SAVE|GAME_REQUIRES_ARTWORK|GAME_MECHANICAL )
GAME( 198?, sc1gslam		, 0			, scorpion1			, scorpion1	, nocrypt		, 0,       "BFM",      "Grand Slam (Bellfruit) [Dutch] (Scorpion 1)", GAME_NOT_WORKING|GAME_SUPPORTS_SAVE|GAME_REQUIRES_ARTWORK|GAME_MECHANICAL )
GAME( 198?, sc1happy		, 0			, scorpion1			, scorpion1	, nocrypt		, 0,       "BFM",      "Happy Hour (Bellfruit - Elam) (Scorpion 1)", GAME_NOT_WORKING|GAME_SUPPORTS_SAVE|GAME_REQUIRES_ARTWORK|GAME_MECHANICAL )
GAME( 198?, sc1impc			, 0			, scorpion1			, scorpion1	, lotse			, 0,       "BFM/ELAM", "Impact (Bellfruit) [Dutch] (Scorpion 1)", GAME_NOT_WORKING|GAME_SUPPORTS_SAVE|GAME_REQUIRES_ARTWORK|GAME_MECHANICAL )
GAME( 198?, sc1kings		, 0			, scorpion1			, scorpion1	, lotse_bank0	, 0,       "BFM/ELAM", "Kings Club (Bellfruit) [Dutch] (Scorpion 1)", GAME_NOT_WORKING|GAME_SUPPORTS_SAVE|GAME_REQUIRES_ARTWORK|GAME_MECHANICAL )
GAME( 198?, sc1linx			, 0			, scorpion1			, scorpion1	, lotse			, 0,       "BFM",      "Linx (Bellfruit) (Scorpion 1)", GAME_NOT_WORKING|GAME_SUPPORTS_SAVE|GAME_REQUIRES_ARTWORK|GAME_MECHANICAL )
GAME( 198?, sc1magc			, 0			, scorpion1			, scorpion1	, nocrypt		, 0,       "BFM/ELAM", "Magic Circle (Bellfruit) [Dutch] (Scorpion 1)", GAME_NOT_WORKING|GAME_SUPPORTS_SAVE|GAME_REQUIRES_ARTWORK|GAME_MECHANICAL )
GAME( 198?, sc1manha		, 0			, scorpion1			, scorpion1	, lotse_bank0	, 0,       "BFM/ELAM", "Manhattan (Bellfruit) [Dutch] (Scorpion 1)", GAME_NOT_WORKING|GAME_SUPPORTS_SAVE|GAME_REQUIRES_ARTWORK|GAME_MECHANICAL )
GAME( 198?, sc1mast			, 0			, scorpion1			, scorpion1	, lotse			, 0,       "BFM",      "Master Club (Bellfruit) (Scorpion 1)", GAME_NOT_WORKING|GAME_SUPPORTS_SAVE|GAME_REQUIRES_ARTWORK|GAME_MECHANICAL )
GAME( 198?, sc1quat			, 0			, scorpion1			, scorpion1	, nocrypt		, 0,       "BFM/ELAM", "Quatro (Bellfruit) [Dutch] (Scorpion 1)", GAME_NOT_WORKING|GAME_SUPPORTS_SAVE|GAME_REQUIRES_ARTWORK|GAME_MECHANICAL )
GAME( 198?, sc1mist			, 0			, scorpion1			, scorpion1	, lotse_bank0	, 0,       "BFM/ELAM", "Mistral (Bellfruit) [Dutch] (Scorpion 1)", GAME_NOT_WORKING|GAME_SUPPORTS_SAVE|GAME_REQUIRES_ARTWORK|GAME_MECHANICAL )
GAME( 198?, sc1olym			, 0			, scorpion1			, scorpion1	, lotse			, 0,       "BFM/ELAM", "Olympia (Bellfruit) [Dutch] (Scorpion 1)", GAME_NOT_WORKING|GAME_SUPPORTS_SAVE|GAME_REQUIRES_ARTWORK|GAME_MECHANICAL )
GAME( 198?, sc1orac			, 0			, scorpion1			, scorpion1	, lotse			, 0,       "BFM/ELAM", "Oracle (Bellfruit) [Dutch] (Scorpion 1)", GAME_NOT_WORKING|GAME_SUPPORTS_SAVE|GAME_REQUIRES_ARTWORK|GAME_MECHANICAL )
GAME( 198?, sc1pwrl			, 0			, scorpion1			, scorpion1	, lotse			, 0,       "BFM",      "Power Lines (Bellfruit) (Scorpion 1)", GAME_NOT_WORKING|GAME_SUPPORTS_SAVE|GAME_REQUIRES_ARTWORK|GAME_MECHANICAL )
GAME( 198?, sc1rain			, 0			, scorpion1			, scorpion1	, lotse			, 0,       "BFM/ELAM", "Rainbow (Bellfruit) [Dutch] (Scorpion 1)", GAME_NOT_WORKING|GAME_SUPPORTS_SAVE|GAME_REQUIRES_ARTWORK|GAME_MECHANICAL )
GAME( 198?, sc1re			, 0			, scorpion1			, scorpion1	, nocrypt		, 0,       "BFM/ELAM", "Reel Cash (Bellfruit) [Dutch] (Scorpion 1)", GAME_NOT_WORKING|GAME_SUPPORTS_SAVE|GAME_REQUIRES_ARTWORK|GAME_MECHANICAL )
GAME( 198?, sc1rese			, sc1re		, scorpion1			, scorpion1	, lotse			, 0,       "BFM/ELAM", "Reel Cash SE (Bellfruit) [Dutch] (Scorpion 1)", GAME_NOT_WORKING|GAME_SUPPORTS_SAVE|GAME_REQUIRES_ARTWORK|GAME_MECHANICAL ) // doesn't say 'SE'
GAME( 198?, sc1revo			, 0			, scorpion1			, scorpion1	, nocrypt		, 0,       "BFM/ELAM", "Revolution (Bellfruit) [Dutch] (Scorpion 1)", GAME_NOT_WORKING|GAME_SUPPORTS_SAVE|GAME_REQUIRES_ARTWORK|GAME_MECHANICAL )
GAME( 198?, sc1rose			, 0			, scorpion1			, scorpion1	, lotse			, 0,       "BFM/ELAM", "Rose 'n' Crown (Bellfruit) [Dutch] (Scorpion 1)", GAME_NOT_WORKING|GAME_SUPPORTS_SAVE|GAME_REQUIRES_ARTWORK|GAME_MECHANICAL )
GAME( 198?, sc1sant			, 0			, scorpion1			, scorpion1	, lotse			, 0,       "BFM/ELAM", "Santana (Bellfruit) [Dutch] (Scorpion 1)", GAME_NOT_WORKING|GAME_SUPPORTS_SAVE|GAME_REQUIRES_ARTWORK|GAME_MECHANICAL )
GAME( 198?, sc1sat			, 0			, scorpion1			, scorpion1	, lotse			, 0,       "BFM/ELAM", "Satellite (Bellfruit) [Dutch] (Scorpion 1)", GAME_NOT_WORKING|GAME_SUPPORTS_SAVE|GAME_REQUIRES_ARTWORK|GAME_MECHANICAL )
GAME( 198?, sc1shan			, 0			, scorpion1			, scorpion1	, lotse			, 0,       "BFM/ELAM", "Shanghai (Bellfruit) [Dutch] (Scorpion 1)", GAME_NOT_WORKING|GAME_SUPPORTS_SAVE|GAME_REQUIRES_ARTWORK|GAME_MECHANICAL )
GAME( 198?, sc1spct			, 0			, scorpion1			, scorpion1	, lotse			, 0,       "BFM",      "Spectre (Bellfruit) (Scorpion 1)", GAME_NOT_WORKING|GAME_SUPPORTS_SAVE|GAME_REQUIRES_ARTWORK|GAME_MECHANICAL )
GAME( 198?, sc1spit			, 0			, scorpion1			, scorpion1	, lotse			, 0,       "BFM",      "Spitfire (Elam) (Scorpion 1)", GAME_NOT_WORKING|GAME_SUPPORTS_SAVE|GAME_REQUIRES_ARTWORK|GAME_MECHANICAL )
GAME( 198?, sc1ster			, 0			, scorpion1			, scorpion1	, lotse			, 0,       "BFM/ELAM", "Sterling (Bellfruit) [Dutch] (Scorpion 1)", GAME_NOT_WORKING|GAME_SUPPORTS_SAVE|GAME_REQUIRES_ARTWORK|GAME_MECHANICAL )
GAME( 198?, sc1str4			, 0			, scorpion1			, scorpion1	, lotse			, 0,       "BFM",      "Strike 4 (Bellfruit) (Scorpion 1, set 1)", GAME_NOT_WORKING|GAME_SUPPORTS_SAVE|GAME_REQUIRES_ARTWORK|GAME_MECHANICAL )
GAME( 198?, sc1str4a		, sc1str4	, scorpion1			, scorpion1	, nocrypt		, 0,       "BFM/ELAM", "Strike 4 (Bellfruit) (Scorpion 1, set 2)", GAME_NOT_WORKING|GAME_SUPPORTS_SAVE|GAME_REQUIRES_ARTWORK|GAME_MECHANICAL )
GAME( 198?, sc1sir			, 0			, scorpion1			, scorpion1	, lotse			, 0,       "BFM",      "Strike It Rich (Bellfruit) (Scorpion 1)", GAME_NOT_WORKING|GAME_SUPPORTS_SAVE|GAME_REQUIRES_ARTWORK|GAME_MECHANICAL )
GAME( 198?, sc1sups			, 0			, scorpion1			, scorpion1	, lotse			, 0,       "BFM",      "Superstar (Bellfruit) [Dutch] (Scorpion 1)", GAME_NOT_WORKING|GAME_SUPPORTS_SAVE|GAME_REQUIRES_ARTWORK|GAME_MECHANICAL )
GAME( 198?, sc1torn			, 0			, scorpion1			, scorpion1	, lotse			, 0,       "BFM/ELAM", "Tornado (Bellfruit) [Dutch] (Scorpion 1)", GAME_NOT_WORKING|GAME_SUPPORTS_SAVE|GAME_REQUIRES_ARTWORK|GAME_MECHANICAL )
GAME( 198?, sc1tri			, 0			, scorpion1			, scorpion1	, lotse			, 0,       "BFM",      "Tri Star (Bellfruit) (Scorpion 1)", GAME_NOT_WORKING|GAME_SUPPORTS_SAVE|GAME_REQUIRES_ARTWORK|GAME_MECHANICAL )
GAME( 198?, sc1typ			, 0			, scorpion1			, scorpion1	, lotse			, 0,       "BFM",      "Typhoon Club (Bellfruit) (Scorpion 1)", GAME_NOT_WORKING|GAME_SUPPORTS_SAVE|GAME_REQUIRES_ARTWORK|GAME_MECHANICAL )
GAME( 198?, sc1ult			, 0			, scorpion1			, scorpion1	, lotse			, 0,       "BFM",      "Ultimate (Bellfruit) (Scorpion 1)", GAME_NOT_WORKING|GAME_SUPPORTS_SAVE|GAME_REQUIRES_ARTWORK|GAME_MECHANICAL )
GAME( 198?, sc1vent			, 0			, scorpion1			, scorpion1	, lotse			, 0,       "BFM/ELAM", "Ventura (Bellfruit) [Dutch] (Scorpion 1)", GAME_NOT_WORKING|GAME_SUPPORTS_SAVE|GAME_REQUIRES_ARTWORK|GAME_MECHANICAL )
GAME( 198?, sc1vict			, 0			, scorpion1			, scorpion1	, lotse			, 0,       "BFM/ELAM", "Victory (Bellfruit) [Dutch] (Scorpion 1)", GAME_NOT_WORKING|GAME_SUPPORTS_SAVE|GAME_REQUIRES_ARTWORK|GAME_MECHANICAL )
GAME( 198?, sc1voy			, 0			, scorpion1			, scorpion1	, lotse			, 0,       "ELAM",     "Voyager (Bellfruit) (Scorpion 1, set 1)", GAME_NOT_WORKING|GAME_SUPPORTS_SAVE|GAME_REQUIRES_ARTWORK|GAME_MECHANICAL )
GAME( 198?, sc1voya			, sc1voy	, scorpion1			, scorpion1	, lotse			, 0,       "ELAM",     "Voyager (Bellfruit) (Scorpion 1, set 2)", GAME_NOT_WORKING|GAME_SUPPORTS_SAVE|GAME_REQUIRES_ARTWORK|GAME_MECHANICAL )
GAME( 198?, sc1winfl		, 0			, scorpion1			, scorpion1	, lotse			, 0,       "BFM/ELAM", "Winfalls [Dutch] (Bellfruit) (Scorpion 1)", GAME_NOT_WORKING|GAME_SUPPORTS_SAVE|GAME_REQUIRES_ARTWORK|GAME_MECHANICAL )
GAME( 198?, sc1winst		, 0			, scorpion1			, scorpion1	, lotse			, 0,       "BFM",      "Winning Streak (Bellfruit) (Scorpion 1)", GAME_NOT_WORKING|GAME_SUPPORTS_SAVE|GAME_REQUIRES_ARTWORK|GAME_MECHANICAL )
GAME( 198?, sc1zep			, 0			, scorpion1			, scorpion1	, lotse			, 0,       "ELAM",     "Zeppelin (Bellfruit) (Scorpion 1)", GAME_NOT_WORKING|GAME_SUPPORTS_SAVE|GAME_REQUIRES_ARTWORK|GAME_MECHANICAL )
GAME( 198?, sc1wthn			, 0			, scorpion1			, scorpion1	, lotse			, 0,       "Eurocoin", "Wild Thing (Eurocoin) (Scorpion 1)", GAME_NOT_WORKING|GAME_SUPPORTS_SAVE|GAME_REQUIRES_ARTWORK|GAME_MECHANICAL )
GAME( 199?, sc1moonl		, 0			, scorpion1			, scorpion1	, lotse			, 0,	   "Bwb",      "Moon Lite (Bwb)", GAME_SUPPORTS_SAVE|GAME_REQUIRES_ARTWORK|GAME_NOT_WORKING|GAME_MECHANICAL)
GAME( 199?, sc1ltdv			, 0			, scorpion1			, scorpion1	, lotse			, 0,	   "Pcp",      "Little Devil (Pcp)", GAME_SUPPORTS_SAVE|GAME_REQUIRES_ARTWORK|GAME_NOT_WORKING|GAME_MECHANICAL)
GAME( 198?, sc1t1k			, 0			, scorpion1			, scorpion1	, lotse			, 0,       "Eurocoin", "Top 1000 (Eurocoin) (Scorpion 1)", GAME_NOT_WORKING|GAME_SUPPORTS_SAVE|GAME_REQUIRES_ARTWORK|GAME_MECHANICAL )
GAME( 199?, sc1smoke		, 0			, scorpion1			, scorpion1	, nocrypt		, 0,	   "Mdm",      "Smokey Vs The Bandit (Mdm) (Scorpion 2/3)",   GAME_NOT_WORKING|GAME_REQUIRES_ARTWORK|GAME_NO_SOUND )
GAME( 199?, sc1ccroc		, 0			, scorpion1			, scorpion1	, nocrypt		, 0,	   "Mdm",      "Crazy Crocs (Mdm) (Scorpion 2/3)",   GAME_NOT_WORKING|GAME_REQUIRES_ARTWORK|GAME_NO_SOUND )
GAME( 199?, sc1crocr		, 0			, scorpion1			, scorpion1	, nocrypt		, 0,	   "Mdm",      "Croc And Roll (Mdm) (Scorpion 2/3)",   GAME_NOT_WORKING|GAME_REQUIRES_ARTWORK|GAME_NO_SOUND )
GAME( 199?, sc1btclk		, 0			, scorpion1			, scorpion1	, nocrypt		, 0,	   "Mdm",      "Beat The Clock (Mdm) (Scorpion 2/3)",   GAME_NOT_WORKING|GAME_REQUIRES_ARTWORK|GAME_NO_SOUND )
GAME( 199?, sc1clins		, 0			, scorpion1			, scorpion1	, lotse			, 0,	   "BFM/PCP",  "Cash Lines (Bellfruit) (Scorpion 1) (set 1)", GAME_SUPPORTS_SAVE|GAME_REQUIRES_ARTWORK|GAME_NOT_WORKING|GAME_MECHANICAL)
GAME( 199?, sc1clinsa		, sc1clins	, scorpion1			, scorpion1	, lotse			, 0,	   "BFM/PCP",  "Cash Lines (Bellfruit) (Scorpion 1) (set 2)", GAME_SUPPORTS_SAVE|GAME_REQUIRES_ARTWORK|GAME_NOT_WORKING|GAME_MECHANICAL)
GAME( 199?, sc1clinsb		, sc1clins	, scorpion1			, scorpion1	, lotse			, 0,	   "BFM",	   "Cash Lines (Bellfruit) (Scorpion 1) (set 3)", GAME_SUPPORTS_SAVE|GAME_REQUIRES_ARTWORK|GAME_NOT_WORKING|GAME_MECHANICAL)
GAME( 199?, sc1clinsc		, sc1clins	, scorpion1			, scorpion1	, lotse			, 0,	   "BFM",	   "Cash Lines (Bellfruit) (Scorpion 1) (set 4)", GAME_SUPPORTS_SAVE|GAME_REQUIRES_ARTWORK|GAME_NOT_WORKING|GAME_MECHANICAL)
GAME( 199?, sc1clinsd		, sc1clins	, scorpion1			, scorpion1	, lotse			, 0,	   "BFM",	   "Cash Lines (Bellfruit) (Scorpion 1) (set 5)", GAME_SUPPORTS_SAVE|GAME_REQUIRES_ARTWORK|GAME_NOT_WORKING|GAME_MECHANICAL)
GAME( 199?, sc1clinse		, sc1clins	, scorpion1			, scorpion1	, lotse			, 0,	   "BFM",	   "Cash Lines (Bellfruit) (Scorpion 1) (set 6)", GAME_SUPPORTS_SAVE|GAME_REQUIRES_ARTWORK|GAME_NOT_WORKING|GAME_MECHANICAL)
GAME( 199?, sc1clb3			, 0			, scorpion1			, scorpion1	, lotse			, 0,	   "BFM",      "Club 3000 (Bellfruit) (Scorpion 1)", GAME_SUPPORTS_SAVE|GAME_REQUIRES_ARTWORK|GAME_NOT_WORKING|GAME_MECHANICAL)
GAME( 199?, sc1czbrk		, 0			, scorpion1			, scorpion1	, lotse			, 0,	   "BFM",      "Crazy Break (Bellfruit) (Scorpion 1)", GAME_SUPPORTS_SAVE|GAME_REQUIRES_ARTWORK|GAME_NOT_WORKING|GAME_MECHANICAL) // Battery Defect
GAME( 199?, sc1energ		, 0			, scorpion1			, scorpion1	, lotse			, 0,	   "BFM",      "Energy (Bellfruit) (Scorpion 1)", GAME_SUPPORTS_SAVE|GAME_REQUIRES_ARTWORK|GAME_NOT_WORKING|GAME_MECHANICAL) // Battery Defect
GAME( 199?, sc1hipt			, 0			, scorpion1			, scorpion1	, lotse			, 0,	   "BFM",      "High Point (Bellfruit) (Scorpion 1) (set 1)", GAME_SUPPORTS_SAVE|GAME_REQUIRES_ARTWORK|GAME_NOT_WORKING|GAME_MECHANICAL)
GAME( 199?, sc1hipta		, sc1hipt	, scorpion1			, scorpion1	, lotse			, 0,	   "BFM",      "High Point (Bellfruit) (Scorpion 1) (set 2)", GAME_SUPPORTS_SAVE|GAME_REQUIRES_ARTWORK|GAME_NOT_WORKING|GAME_MECHANICAL)
GAME( 199?, sc1satse		, 0			, scorpion1			, scorpion1	, lotse			, 0,	   "BFM/ELAM", "Satellite SE (Bellfruit) (Dutch) (Scorpion 1)", GAME_SUPPORTS_SAVE|GAME_REQUIRES_ARTWORK|GAME_NOT_WORKING|GAME_MECHANICAL)
GAME( 199?, sc1strk			, 0			, scorpion1			, scorpion1	, lotse			, 0,	   "BFM/ELAM", "Strike (Bellfruit) (Dutch) (Scorpion 1)", GAME_SUPPORTS_SAVE|GAME_REQUIRES_ARTWORK|GAME_NOT_WORKING|GAME_MECHANICAL)
GAME( 199?, sc1supfl		, 0			, scorpion1			, scorpion1	, lotse			, 0,	   "BFM/ELAM", "Super Flush (Bellfruit) (Dutch) (Scorpion 1)", GAME_SUPPORTS_SAVE|GAME_REQUIRES_ARTWORK|GAME_NOT_WORKING|GAME_MECHANICAL)
// are they really SC1?
GAME( 199?, sc1btbc			, 0			, scorpion1			, scorpion1	, lotse			, 0,	   "BFM",      "Beat The Bank Club (Bellfruit) (Scorpion 1?)", GAME_SUPPORTS_SAVE|GAME_REQUIRES_ARTWORK|GAME_NOT_WORKING|GAME_MECHANICAL) // behaves like sc1clbdya, but then locks up
GAME( 199?, sc1frpus		, 0			, scorpion1			, scorpion1	, lotse			, 0,	   "BFM",      "Fruit Pursuit (Bellfruit) (Scorpion 1?)", GAME_SUPPORTS_SAVE|GAME_REQUIRES_ARTWORK|GAME_NOT_WORKING|GAME_MECHANICAL)
GAME( 199?, sc1ofs56		, 0			, scorpion1			, scorpion1	, lotse			, 0,	   "BFM",      "unknown 'ofs56cp' (Bellfruit) (Scorpion 1?)", GAME_SUPPORTS_SAVE|GAME_REQUIRES_ARTWORK|GAME_NOT_WORKING|GAME_MECHANICAL) // OFAH - Phoenix 1
GAME( 199?, sc1boncl		, 0			, scorpion1			, scorpion1	, lotse			, 0,	   "BFM",      "Bonanza Club (Bellfruit) (Scorpion 1)", GAME_SUPPORTS_SAVE|GAME_REQUIRES_ARTWORK|GAME_NOT_WORKING|GAME_MECHANICAL)
// are these really SC1? they do more here than in SC2 at least!
GAME( 199?, sc1days			, 0			, scorpion1			, scorpion1	, lotse			, 0,	   "Global",   "All In A Days Work (Global)", GAME_SUPPORTS_SAVE|GAME_REQUIRES_ARTWORK|GAME_NOT_WORKING|GAME_MECHANICAL)
GAME( 199?, sc1cscl			, 0			, scorpion1			, scorpion1	, lotse			, 0,	   "Global",   "Cash Classic (Global)", GAME_SUPPORTS_SAVE|GAME_REQUIRES_ARTWORK|GAME_NOT_WORKING|GAME_MECHANICAL)
GAME( 199?, sc1driv			, 0			, scorpion1			, scorpion1	, lotse			, 0,	   "Global",   "Driving School (Global)", GAME_SUPPORTS_SAVE|GAME_REQUIRES_ARTWORK|GAME_NOT_WORKING|GAME_MECHANICAL)
GAME( 199?, sc1vsd			, 0			, scorpion1			, scorpion1	, lotse			, 0,	   "Global",   "Vegas Super Deal (Global)", GAME_SUPPORTS_SAVE|GAME_REQUIRES_ARTWORK|GAME_NOT_WORKING|GAME_MECHANICAL)

GAME( 199?, sc1wof			, 0			, scorpion1			, scorpion1	, lotse		    , 0,	   "Global",   "Wheel Of Fortune (Global) (set 1)", GAME_SUPPORTS_SAVE|GAME_REQUIRES_ARTWORK|GAME_NOT_WORKING|GAME_MECHANICAL)
GAME( 199?, sc1wofa			, sc1wof	, scorpion1			, scorpion1	, lotse		    , 0,	   "Global",   "Wheel Of Fortune (Global) (set 2)", GAME_SUPPORTS_SAVE|GAME_REQUIRES_ARTWORK|GAME_NOT_WORKING|GAME_MECHANICAL)
GAME( 199?, sc1wofb			, sc1wof	, scorpion1			, scorpion1	, nocrypt	    , 0,	   "Global",   "Wheel Of Fortune (Global) (set 3)", GAME_SUPPORTS_SAVE|GAME_REQUIRES_ARTWORK|GAME_NOT_WORKING|GAME_MECHANICAL)
GAME( 199?, sc1crzyc		, 0			, scorpion1			, scorpion1	, lotse	        , 0,	   "Global",   "Crazy Cash (Global) (set 1)", GAME_SUPPORTS_SAVE|GAME_REQUIRES_ARTWORK|GAME_NOT_WORKING|GAME_MECHANICAL)
GAME( 199?, sc1crzyca		, sc1crzyc	, scorpion1			, scorpion1	, lotse	        , 0,	   "Global",   "Crazy Cash (Global) (set 2)", GAME_SUPPORTS_SAVE|GAME_REQUIRES_ARTWORK|GAME_NOT_WORKING|GAME_MECHANICAL)
GAME( 199?, sc1clbdy		, 0			, scorpion1			, scorpion1	, lotse	        , 0,	   "Global",   "Club Dynamite (Global) (set 1)", GAME_SUPPORTS_SAVE|GAME_REQUIRES_ARTWORK|GAME_NOT_WORKING|GAME_MECHANICAL)
GAME( 199?, sc1clbdya		, sc1clbdy	, scorpion1			, scorpion1	, lotse	        , 0,	   "Global",   "Club Dynamite (Global) (set 2)", GAME_SUPPORTS_SAVE|GAME_REQUIRES_ARTWORK|GAME_NOT_WORKING|GAME_MECHANICAL)
GAME( 199?, sc1chqfl		, 0			, scorpion1			, scorpion1	, lotse	        , 0,	   "Global",   "Chequered Flag (Global)", GAME_SUPPORTS_SAVE|GAME_REQUIRES_ARTWORK|GAME_NOT_WORKING|GAME_MECHANICAL)
GAME( 1992, sc1s1000		, 0			, scorpion1			, scorpion1	, lotse	        , 0,	   "Deltasoft",   "Super 1000 (Deltasoft)", GAME_SUPPORTS_SAVE|GAME_REQUIRES_ARTWORK|GAME_NOT_WORKING|GAME_MECHANICAL) // JT/Deltasoft Nov 1992




GAME( 199?, sc1dip			, 0			, scorpion1			, scorpion1	, nocrypt		, 0,	   "Eurocoin", "Diplomat (Eurocoin) (Scorpion 1)", GAME_SUPPORTS_SAVE|GAME_REQUIRES_ARTWORK|GAME_NOT_WORKING|GAME_MECHANICAL) // 53 RY error
GAME( 199?, sc1lamb			, 0			, scorpion1			, scorpion1	, nocrypt		, 0,	   "Eurocoin", "Lambada (Eurocoin) (Scorpion 1)", GAME_SUPPORTS_SAVE|GAME_REQUIRES_ARTWORK|GAME_NOT_WORKING|GAME_MECHANICAL) //
GAME( 199?, sc1reply		, 0			, scorpion1			, scorpion1	, nocrypt		, 0,	   "Eurocoin", "Replay (Eurocoin) (Scorpion 1)", GAME_SUPPORTS_SAVE|GAME_REQUIRES_ARTWORK|GAME_NOT_WORKING|GAME_MECHANICAL) //

GAME( 199?, sc1cdm		, 0			,  scorpion1		, scorpion1		, lotse_bank0		, 0,		 "Crystal",   "Club Diamond (Crystal) (Scorpion 1)", GAME_SUPPORTS_SAVE|GAME_REQUIRES_ARTWORK|GAME_NOT_WORKING|GAME_MECHANICAL)
GAME( 199?, sc1hfcc		, 0			,  scorpion1		, scorpion1		, lotse_bank0		, 0,		 "Crystal",   "Hi Flyer Club (Crystal) (Scorpion 1)", GAME_SUPPORTS_SAVE|GAME_REQUIRES_ARTWORK|GAME_NOT_WORKING|GAME_MECHANICAL)

GAME( 199?, sc1twice		, 0			,  scorpion1		, scorpion1		, lotse_bank0		, 0,		 "Associated Leisure",   "Twice As Nice (Associated Leisure) (Scorpion 1)", GAME_SUPPORTS_SAVE|GAME_REQUIRES_ARTWORK|GAME_NOT_WORKING|GAME_MECHANICAL) // this has valid strings in it BEFORE the bfm decode, but decodes to valid code, does it use some funky mapping, or did they just fill unused space with valid looking data?
GAME( 199?, sc1chain			, 0			, scorpion1			, scorpion1	, lotse			, 0,	   "BFM",      "Chain Reaction (Bellfruit) (Scorpion 1)", GAME_SUPPORTS_SAVE|GAME_REQUIRES_ARTWORK|GAME_NOT_WORKING|GAME_MECHANICAL)
GAME( 199?, sc1potp			, 0			, scorpion1			, scorpion1	, lotse			, 0,	   "BFM",      "Pick Of The Pack (Bellfruit) (Scorpion 1)", GAME_SUPPORTS_SAVE|GAME_REQUIRES_ARTWORK|GAME_NOT_WORKING|GAME_MECHANICAL) //was mixed with the sc4/5 potp roms..  System status 98
GAME( 199?, sc1potpa			, sc1potp			, scorpion1			, scorpion1	, lotse			, 0,	   "BFM",      "Double Dealer (Bellfruit) (Scorpion 1)", GAME_SUPPORTS_SAVE|GAME_REQUIRES_ARTWORK|GAME_NOT_WORKING|GAME_MECHANICAL) // ^ with above.. seems the same game, but different name


GAME( 199?, sc1scunk			, 0			, scorpion1			, scorpion1	, lotse			, 0,	   "<unknown>",      "unknown Scorpion 1 'Super ?' (Bellfruit) (Scorpion 1)", GAME_SUPPORTS_SAVE|GAME_REQUIRES_ARTWORK|GAME_NOT_WORKING|GAME_MECHANICAL) // ^ with above.. seems the same game, but different name


//Adder 2
GAME( 1996, m_tppokr, 0,        scorpion1_adder2	, toppoker	, toppoker		, 0,       "BFM/ELAM",    "Top Poker (Dutch, Game Card 95-750-899)", GAME_NOT_WORKING|GAME_SUPPORTS_SAVE|GAME_REQUIRES_ARTWORK )


