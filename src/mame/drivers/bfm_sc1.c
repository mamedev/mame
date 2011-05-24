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

// scorpion1 board init ///////////////////////////////////////////////////

static const UINT16 AddressDecode[]=
{
	0x0800,0x1000,0x0001,0x0004,0x0008,0x0020,0x0080,0x0200,
	0x0100,0x0040,0x0002,0x0010,0x0400,0x2000,0x4000,0x8000,

	0
};

static const UINT8 DataDecode[]=
{
	0x02,0x08,0x20,0x40,0x10,0x04,0x01,0x80,

	0
};



static void decode_sc1(running_machine &machine,const char *rom_region)
{
	bfm_sc1_state *state = machine.driver_data<bfm_sc1_state>();
	UINT8 *tmp, *rom;

	rom = machine.region(rom_region)->base();

	tmp = auto_alloc_array(machine, UINT8, 0x10000);

	{
		int i;
		long address;

		memcpy(tmp, rom, 0x10000);

		for ( i = 0; i < 256; i++ )
		{
			UINT8 data, pattern, newdata, *tab;
			data    = i;

			tab     = (UINT8*)DataDecode;
			pattern = 0x01;
			newdata = 0;

			do
			{
				newdata |= data & pattern ? *tab : 0;
				pattern <<= 1;
			} while ( *(++tab) );

		state->m_codec_data[i] = newdata;
		}

		for ( address = 0; address < 0x10000; address++)
		{
			int newaddress,pattern;
			UINT16 *tab;

			tab      = (UINT16*)AddressDecode;
			pattern  = 0x0001;
			newaddress = 0;
			do
			{
				newaddress |= address & pattern ? *tab : 0;
				pattern <<= 1;
			} while ( *(++tab) );

			rom[newaddress] = state->m_codec_data[ tmp[address] ];
		}
		auto_free( machine, tmp );
	}
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

		memory_configure_bank(machine,"bank1", 0, 1, &rom[0x10000], 0);
		memory_configure_bank(machine,"bank1", 1, 3, &rom[0x02000], 0x02000);

		memory_set_bank(machine,"bank1",3);
	}
}

/////////////////////////////////////////////////////////////////////////////////////
// scorpion1 board memory map ///////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////

static ADDRESS_MAP_START( memmap, AS_PROGRAM, 8 )

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

	AM_RANGE(0x3404, 0x3404) AM_READ(dipcoin_r )			// coin input on gamecard
	AM_RANGE(0x3801, 0x3801) AM_READNOP						// uPD5579 status on soundcard (not installed)

	AM_RANGE(0x3600, 0x3600) AM_WRITE(bankswitch_w) 		// write bank
	AM_RANGE(0x3800, 0x39FF) AM_WRITE(reel56_w)				// reel 5+6 latch

	AM_RANGE(0x4000, 0x5FFF) AM_ROM							// 8k  ROM
	AM_RANGE(0x6000, 0x7FFF) AM_ROMBANK("bank1")					// 8k  paged ROM (4 pages)
	AM_RANGE(0x8000, 0xFFFF) AM_ROM AM_WRITE (watchdog_reset_w) // 32k ROM

ADDRESS_MAP_END

/////////////////////////////////////////////////////////////////////////////////////
// scorpion1 board + adder2 expansion memory map ////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////

static ADDRESS_MAP_START( memmap_adder2, AS_PROGRAM, 8 )

	AM_RANGE(0x0000, 0x1FFF) AM_RAM AM_SHARE("nvram") //8k RAM
	AM_RANGE(0x2000, 0x21FF) AM_WRITE(reel34_w)	  // reel 2+3 latch
	AM_RANGE(0x2200, 0x23FF) AM_WRITE(reel12_w)	  // reel 1+2 latch
	AM_RANGE(0x2400, 0x25FF) AM_WRITE(vfd_w)	  // vfd latch

	AM_RANGE(0x2600, 0x27FF) AM_READWRITE(mmtr_r,mmtr_w)      // mechanical meters

	AM_RANGE(0x2800, 0x2800) AM_READ(triac_r)     // payslide triacs
	AM_RANGE(0x2800, 0x29FF) AM_WRITE(triac_w)

	AM_RANGE(0x2A00, 0x2A00) AM_READWRITE(mux1latch_r,mux1latch_w) // mux1
	AM_RANGE(0x2A01, 0x2A01) AM_READWRITE(mux1datlo_r,mux1datlo_w)
	AM_RANGE(0x2A02, 0x2A02) AM_READWRITE(mux1dathi_r,mux1dathi_w)

	AM_RANGE(0x2E00, 0x2E00) AM_READ(irqlatch_r)  // irq latch

	AM_RANGE(0x3001, 0x3001) AM_READ(soundlatch_r)
	AM_RANGE(0x3001, 0x3001) AM_DEVWRITE("aysnd", ay8910_data_w)
	AM_RANGE(0x3101, 0x3201) AM_DEVWRITE("aysnd", ay8910_address_w)

	AM_RANGE(0x3406, 0x3406) AM_READWRITE(aciastat_r,aciactrl_w)  // MC6850 status register
	AM_RANGE(0x3407, 0x3407) AM_READWRITE(aciadata_r,aciadata_w)  // MC6850 data register

	AM_RANGE(0x3408, 0x3408) AM_READWRITE(mux2latch_r,mux2latch_w) // mux2
	AM_RANGE(0x3409, 0x3409) AM_READWRITE(mux2datlo_r,mux2datlo_w)
	AM_RANGE(0x340A, 0x340A) AM_READWRITE(mux2dathi_r,mux2dathi_w)

//  AM_RANGE(0x3404, 0x3404) AM_READ(dipcoin_r ) // coin input on gamecard
	AM_RANGE(0x3801, 0x3801) AM_READNOP			 // uPD5579 status on soundcard (not installed)

	AM_RANGE(0x3600, 0x3600) AM_WRITE(bankswitch_w) // write bank
	AM_RANGE(0x3800, 0x39FF) AM_WRITE(reel56_w)	 // reel 5+6 latch

	AM_RANGE(0x3E00, 0x3E00) AM_READWRITE(vid_uart_ctrl_r,vid_uart_ctrl_w)	// video uart control reg read
	AM_RANGE(0x3E01, 0x3E01) AM_READWRITE(vid_uart_rx_r,vid_uart_tx_w)		// video uart receive  reg

	AM_RANGE(0x4000, 0x5FFF) AM_ROM							// 8k  ROM
	AM_RANGE(0x6000, 0x7FFF) AM_ROMBANK("bank1")					// 8k  paged ROM (4 pages)
	AM_RANGE(0x8000, 0xFFFF) AM_ROM AM_WRITE(watchdog_reset_w)	// 32k ROM

ADDRESS_MAP_END


/////////////////////////////////////////////////////////////////////////////////////
// scorpion1 board + upd7759 soundcard memory map ///////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////

static ADDRESS_MAP_START( sc1_nec_uk, AS_PROGRAM, 8 )

	AM_RANGE(0x0000, 0x1FFF) AM_RAM AM_SHARE("nvram") //8k RAM
	AM_RANGE(0x2000, 0x21FF) AM_WRITE(reel34_w)	  // reel 2+3 latch
	AM_RANGE(0x2200, 0x23FF) AM_WRITE(reel12_w)	  // reel 1+2 latch
	AM_RANGE(0x2400, 0x25FF) AM_WRITE(vfd_w)	  // vfd latch

	AM_RANGE(0x2600, 0x27FF) AM_READWRITE(mmtr_r,mmtr_w)      // mechanical meters

	AM_RANGE(0x2800, 0x2800) AM_READ(triac_r)     // payslide triacs
	AM_RANGE(0x2800, 0x29FF) AM_WRITE(triac_w)

	AM_RANGE(0x2A00, 0x2A00) AM_READWRITE(mux1latch_r,mux1latch_w) // mux1
	AM_RANGE(0x2A01, 0x2A01) AM_READWRITE(mux1datlo_r,mux1datlo_w)
	AM_RANGE(0x2A02, 0x2A02) AM_READWRITE(mux1dathi_r,mux1dathi_w)

	AM_RANGE(0x2E00, 0x2E00) AM_READ(irqlatch_r)	  // irq latch

	AM_RANGE(0x3001, 0x3001) AM_READ(soundlatch_r)
	AM_RANGE(0x3001, 0x3001) AM_DEVWRITE("aysnd", ay8910_data_w)
	AM_RANGE(0x3101, 0x3201) AM_DEVWRITE("aysnd", ay8910_address_w)

	AM_RANGE(0x3406, 0x3406) AM_READWRITE(aciastat_r,aciactrl_w)  // MC6850 status register
	AM_RANGE(0x3407, 0x3407) AM_READWRITE(aciadata_r,aciadata_w)  // MC6850 data register

	AM_RANGE(0x3408, 0x3408) AM_READWRITE(mux2latch_r,mux2latch_w) // mux2
	AM_RANGE(0x3409, 0x3409) AM_READWRITE(mux2datlo_r,mux2datlo_w)
	AM_RANGE(0x340A, 0x340A) AM_READWRITE(mux2dathi_r,mux2dathi_w)

	AM_RANGE(0x3600, 0x3600) AM_WRITE(bankswitch_w) // write bank

	AM_RANGE(0x3801, 0x3801) AM_DEVREAD("upd", nec_r)
	AM_RANGE(0x3800, 0x39FF) AM_DEVWRITE("upd", nec_latch_w)

	AM_RANGE(0x4000, 0x5FFF) AM_ROM							// 8k  ROM
	AM_RANGE(0x6000, 0x7FFF) AM_ROMBANK("bank1")					// 8k  paged ROM (4 pages)
	AM_RANGE(0x8000, 0xFFFF) AM_ROM AM_WRITE(watchdog_reset_w)	// 32k ROM

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
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_UNKNOWN )
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
	MCFG_CPU_PROGRAM_MAP(memmap)						// setup read and write memorymap
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
	MCFG_CPU_PROGRAM_MAP(memmap_adder2)				// setup read and write memorymap

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

static MACHINE_CONFIG_DERIVED( scorpion1_nec_uk, scorpion1 )
	MCFG_CPU_MODIFY("maincpu")
	MCFG_CPU_PROGRAM_MAP(sc1_nec_uk)					// setup read and write memorymap

	MCFG_SOUND_ADD("upd",UPD7759, UPD7759_STANDARD_CLOCK)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.50)
MACHINE_CONFIG_END

// ROM definition ///////////////////////////////////////////////////////////////////

ROM_START( m_lotsse )
	ROM_REGION( 0x12000, "maincpu", 0 )
	ROM_LOAD( "lotusse.bin",  0x0000, 0x10000,  CRC(636dadc4) SHA1(85bad5d76dac028fe9f3303dd09e8266aba7db4d))
ROM_END

/////////////////////////////////////////////////////////////////////////////////////

ROM_START( m_roulet )
	ROM_REGION( 0x12000, "maincpu", 0 )
	ROM_LOAD( "rou029.bin",   0x8000, 0x08000,  CRC(31723f0A) SHA1(e220976116a0aaf24dc0c4af78a9311a360e8104))
ROM_END

/////////////////////////////////////////////////////////////////////////////////////

ROM_START( m_clattr )
	ROM_REGION( 0x12000, "maincpu", 0 )
	ROM_LOAD( "39370196.1",   0x8000, 0x08000,  CRC(4c2e465f) SHA1(101939d37d9c033f6d1dfb83b4beb54e4061aec2))
	ROM_LOAD( "39370196.2",   0x0000, 0x08000,  CRC(c809c22d) SHA1(fca7515bc84d432150ffe5e32fccc6aed458b8b0))
ROM_END

/////////////////////////////////////////////////////////////////////////////////////

ROM_START( m_tppokr )
	ROM_REGION( 0x12000, "maincpu", 0 )
	ROM_LOAD( "95750899.bin", 0x0000, 0x10000,  CRC(639d1d62) SHA1(80620c14bf9f953588555510fc2e6e930140923f))

	ROM_REGION( 0x20000, "adder2", 0 )
	ROM_LOAD( "tpk010.vid", 0x0000, 0x20000,  CRC(ea4eddca) SHA1(5fb805d35376ec7ee8d58684e584621dbb2b2a9c))

	ROM_REGION( 0x40000, "gfx1", ROMREGION_ERASEFF )
	ROM_LOAD( "tpk011.chr",	0x00000, 0x20000,  CRC(4dc23ad8) SHA1(8e8cc699412dbb092e16e14518f407353f477ee1))
ROM_END

/////////////////////////////////////////////////////////////////////////////////////

static void sc1_common_init(running_machine &machine, int reels, int decrypt)
{
	bfm_sc1_state *state = machine.driver_data<bfm_sc1_state>();
	UINT8 *rom, i;

	rom = machine.region("maincpu")->base();
	if ( rom )
	{
		memcpy(&rom[0x10000], &rom[0x00000], 0x2000);
	}

	memset(state->m_sc1_Inputs, 0, sizeof(state->m_sc1_Inputs));

	// setup n default 96 half step reels ///////////////////////////////////////////
	for ( i = 0; i < reels; i++ )
	{
		stepper_config(machine, i, &starpoint_interface_48step);
	}
	if (decrypt) decode_sc1(machine,"maincpu");	// decode main rom
	if (reels)
	{
		awp_reel_setup();
	}
}

static DRIVER_INIT(toppoker)
{
	sc1_common_init(machine,3,1);
	adder2_decode_char_roms(machine);	// decode GFX roms
	MechMtr_config(machine,8);

	BFM_BD1_init(0);
}

static DRIVER_INIT(lotse)
{
	sc1_common_init(machine,6,1);
	MechMtr_config(machine,8);

	BFM_BD1_init(0);
	BFM_BD1_init(1);
}

/////////////////////////////////////////////////////////////////////////////////////

static DRIVER_INIT(rou029)
{
	sc1_common_init(machine,6,0);
	MechMtr_config(machine,8);

	BFM_BD1_init(0);
}

/////////////////////////////////////////////////////////////////////////////////////

static DRIVER_INIT(clatt)
{
	bfm_sc1_state *state = machine.driver_data<bfm_sc1_state>();
	sc1_common_init(machine,6,1);
	MechMtr_config(machine,8);

	BFM_BD1_init(0);

	Scorpion1_SetSwitchState(state,3,2,1);
	Scorpion1_SetSwitchState(state,3,3,1);
	Scorpion1_SetSwitchState(state,3,6,1);
	Scorpion1_SetSwitchState(state,4,1,1);
}

/////////////////////////////////////////////////////////////////////////////////////

GAME( 1988, m_lotsse, 0,        scorpion1,	 scorpion1,  lotse,	0,       "BFM/ELAM",    "Lotus SE (Dutch)", GAME_NOT_WORKING|GAME_SUPPORTS_SAVE|GAME_REQUIRES_ARTWORK|GAME_MECHANICAL )
GAME( 1988, m_roulet, 0,        scorpion1,	 scorpion1,  rou029,	0,       "BFM/ELAM",    "Roulette (Dutch, Game Card 39-360-129?)", GAME_NOT_WORKING|GAME_SUPPORTS_SAVE|GAME_REQUIRES_ARTWORK|GAME_MECHANICAL )
GAME( 1990, m_clattr, 0,        scorpion1_nec_uk,clatt,	     clatt,	0,       "BFM",         "Club attraction (UK, Game Card 39-370-196)", GAME_NOT_WORKING|GAME_SUPPORTS_SAVE|GAME_REQUIRES_ARTWORK|GAME_MECHANICAL )

//Adder
GAME( 1996, m_tppokr, 0,        scorpion1_adder2,toppoker,   toppoker,	0,       "BFM/ELAM",    "Toppoker (Dutch, Game Card 95-750-899)", GAME_NOT_WORKING|GAME_SUPPORTS_SAVE|GAME_REQUIRES_ARTWORK )
