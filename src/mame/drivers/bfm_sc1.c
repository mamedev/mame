/*****************************************************************************************

    Bellfruit scorpion1 driver, (under heavy construction !!!)

    A.G.E Code Copyright J. Wallace and the AGEMAME Development Team.
    Visit http://www.mameworld.net/agemame/ for more information.

    M.A.M.E Core Copyright Nicola Salmoria and the MAME Team,
    used under license from http://mamedev.org

******************************************************************************************

  20-01-2007: J Wallace: Tidy up of coding
  30-12-2006: J Wallace: Fixed init routines.
  07-03-2006: El Condor: Recoded to more accurately represent the hardware setup.
  19-08-2005: Re-Animator
  16-08-2005: Converted to MAME protocol for when Viper board is completed.
  25-08-2005: Added support for adder2 (Toppoker), added support for NEC upd7759 soundcard

Standard scorpion1 memorymap
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

#include "driver.h"
#include "cpu/m6809/m6809.h"
#include "cpu/z80/z80.h"
#include "video/awpvid.h"
#include "video/bfm_adr2.h"
#include "machine/steppers.h" // stepper motor
#include "machine/bfm_bd1.h" // vfd
#include "machine/meters.h"
#include "sound/ay8910.h"
#include "sound/upd7759.h"
#include "bfm_sc1.lh"
#define VFD_RESET  0x20
#define VFD_CLOCK1 0x80
#define VFD_DATA   0x40

#define MASTER_CLOCK	(XTAL_4MHz)
#define ADDER_CLOCK		(XTAL_8MHz)

// local vars /////////////////////////////////////////////////////////////

static int mmtr_latch;		  // mechanical meter latch
static int triac_latch;		  // payslide triac latch
static int vfd_latch;		  // vfd latch
static int irq_status;		  // custom chip IRQ status
static int optic_pattern;     // reel optics
static int acia_status;		  // MC6850 status
static int locked;			  // hardware lock/unlock status (0=unlocked)
static int is_timer_enabled;
static int reel_changed;
static int coin_inhibits;
static int mux1_outputlatch;
static int mux1_datalo;
static int mux1_datahi;
static int mux1_input;

static int mux2_outputlatch;
static int mux2_datalo;
static int mux2_datahi;
static int mux2_input;

static int watchdog_cnt;
static int watchdog_kicked;

// user interface stuff ///////////////////////////////////////////////////

static UINT8 Lamps[256];		  // 256 multiplexed lamps
static UINT8 sc1_Inputs[64];		  // 64? multiplexed inputs

///////////////////////////////////////////////////////////////////////////

static void Scorpion1_SetSwitchState(int strobe, int data, int state)
{
	if ( state ) sc1_Inputs[strobe] |=  (1<<data);
	else		 sc1_Inputs[strobe] &= ~(1<<data);
}

///////////////////////////////////////////////////////////////////////////
#ifdef UNUSED_FUNCTION
static int Scorpion1_GetSwitchState(int strobe, int data)
{
	int state = 0;

	if ( strobe < 7 && data < 8 ) state = (sc1_Inputs[strobe] & (1<<data))?1:0;

	return state;
}
#endif
///////////////////////////////////////////////////////////////////////////

static WRITE8_HANDLER( bankswitch_w )
{
	memory_set_bank(space->machine,"bank1",data & 0x03);
}

///////////////////////////////////////////////////////////////////////////

static INTERRUPT_GEN( timer_irq )
{
	if ( watchdog_kicked )
	{
		watchdog_cnt    = 0;
		watchdog_kicked = 0;
	}
	else
	{
		watchdog_cnt++;
		if ( watchdog_cnt > 2 )	// this is a hack, i don't know what the watchdog timeout is, 3 IRQ's works fine
		{  // reset board
			mame_schedule_soft_reset(device->machine);// reset entire machine. CPU 0 should be enough, but that doesn't seem to work !!
			return;
		}
	}

	if ( is_timer_enabled )
	{
		irq_status = 0x01 |0x02; //0xff;

	    sc1_Inputs[2] = input_port_read(device->machine,"STROBE0");

		generic_pulse_irq_line(cputag_get_cpu(device->machine, "maincpu"), M6809_IRQ_LINE);
	}
}

///////////////////////////////////////////////////////////////////////////

static READ8_HANDLER( irqlatch_r )
{
	int result = irq_status | 0x02;

	irq_status = 0;

	return result;
}

///////////////////////////////////////////////////////////////////////////

static WRITE8_HANDLER( reel12_w )
{
	if ( locked & 0x01 )
	{	// hardware is still locked,
		if ( data == 0x46 ) locked &= ~0x01;
	}
	else
	{
		if ( stepper_update(0, data>>4) ) reel_changed |= 0x01;
		if ( stepper_update(1, data   ) ) reel_changed |= 0x02;

		if ( stepper_optic_state(0) ) optic_pattern |=  0x01;
		else                          optic_pattern &= ~0x01;
		if ( stepper_optic_state(1) ) optic_pattern |=  0x02;
		else                          optic_pattern &= ~0x02;
	}
	awp_draw_reel(0);
	awp_draw_reel(1);
}

///////////////////////////////////////////////////////////////////////////

static WRITE8_HANDLER( reel34_w )
{
	if ( locked & 0x02 )
	{	// hardware is still locked,
		if ( data == 0x42 ) locked &= ~0x02;
	}
	else
	{
		if ( stepper_update(2, data>>4) ) reel_changed |= 0x04;
		if ( stepper_update(3, data   ) ) reel_changed |= 0x08;

		if ( stepper_optic_state(2) ) optic_pattern |=  0x04;
		else                          optic_pattern &= ~0x04;
		if ( stepper_optic_state(3) ) optic_pattern |=  0x08;
		else                          optic_pattern &= ~0x08;
	}
	awp_draw_reel(2);
	awp_draw_reel(3);
}

///////////////////////////////////////////////////////////////////////////

static WRITE8_HANDLER( reel56_w )
{
	if ( stepper_update(4, data>>4) ) reel_changed |= 0x10;
	if ( stepper_update(5, data   ) ) reel_changed |= 0x20;

	if ( stepper_optic_state(4) ) optic_pattern |=  0x10;
	else                          optic_pattern &= ~0x10;
	if ( stepper_optic_state(5) ) optic_pattern |=  0x20;
	else                          optic_pattern &= ~0x20;
	awp_draw_reel(5);
	awp_draw_reel(6);
}

///////////////////////////////////////////////////////////////////////////
// mechanical meters //////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////

static WRITE8_HANDLER( mmtr_w )
{
	int i;
	if ( locked & 0x04 )
	{	// hardware is still locked,
		locked &= ~0x04;
	}
	else
	{
		int  changed = mmtr_latch ^ data;
		UINT64 cycles  = cpu_get_total_cycles(space->cpu);

		mmtr_latch = data;

		for (i=0; i<8; i++)
		{
			if ( changed & (1 << i) )
			{
				Mechmtr_update(i, cycles, data & (1 << i) );
				generic_pulse_irq_line(cputag_get_cpu(space->machine, "maincpu"), M6809_FIRQ_LINE);
			}
		}
	}
}

///////////////////////////////////////////////////////////////////////////

static READ8_HANDLER( mmtr_r )
{
	return mmtr_latch;
}

///////////////////////////////////////////////////////////////////////////

static READ8_HANDLER( dipcoin_r )
{
	return input_port_read(space->machine,"STROBE0") & 0x1F;
}

///////////////////////////////////////////////////////////////////////////

static READ8_DEVICE_HANDLER( nec_r )
{
	return 1;
}

///////////////////////////////////////////////////////////////////////////

static WRITE8_HANDLER( vfd_w )
{
	int changed = vfd_latch ^ data;

	vfd_latch = data;

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

static void send_to_adder(running_machine *machine, int data)
{
	adder2_data_from_sc2 = 1;		// set flag, data from scorpion2 board available
	adder2_sc2data       = data;	// store data

	adder2_acia_triggered = 1;		// set flag, acia IRQ triggered
	cputag_set_input_line(machine, "adder2", M6809_IRQ_LINE, ASSERT_LINE );//HOLD_LINE);// trigger IRQ
}

///////////////////////////////////////////////////////////////////////////

static int receive_from_adder(void)
{
	int data = adder2_data;
	adder2_data_to_sc2 = 0;	  // clr flag, data from adder available

	return data;
}

/////////////////////////////////////////////////////////////////////////////////////

static READ8_HANDLER( mux1latch_r )
{
	return mux1_input;
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
	int changed = mux1_outputlatch ^ data;
	static const char *const portnames[] = { "STROBE0", "STROBE1", "STROBE2", "STROBE3", "STROBE4", "STROBE5", "STROBE6", "STROBE7" };
	mux1_outputlatch = data;

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
				Lamps[ BFM_strcnv[offset  ] ] = mux1_datalo & pattern?1:0;
				Lamps[ BFM_strcnv[offset+8] ] = mux1_datahi & pattern?1:0;
				pattern<<=1;
				offset++;
			}

			if (strobe == 0)
			{
				for ( i = 0; i < 256; i++ )
				{
					output_set_lamp_value(i, Lamps[i]);
				}
			}
		}

		if ( !(data & 0x08) )
		{
			sc1_Inputs[ input_strobe ] = input_port_read(space->machine,portnames[input_strobe]);

			mux1_input = sc1_Inputs[ input_strobe ];
		}
	}
}

/////////////////////////////////////////////////////////////////////////////////////

static WRITE8_HANDLER( mux1datlo_w )
{
	mux1_datalo = data;
}

/////////////////////////////////////////////////////////////////////////////////////

static WRITE8_HANDLER( mux1dathi_w )
{
	mux1_datahi = data;
}

/////////////////////////////////////////////////////////////////////////////////////

static READ8_HANDLER( mux2latch_r )
{
	return mux2_input;
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
	int changed = mux2_outputlatch ^ data;

	mux2_outputlatch = data;

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
				Lamps[ BFM_strcnv[offset  ] ] = mux2_datalo & pattern?1:0;
				Lamps[ BFM_strcnv[offset+8] ] = mux2_datahi & pattern?1:0;
				pattern<<=1;
				offset++;
			}
		}

		if ( !(data & 0x08) )
		{
			mux2_input = 0x3F ^ optic_pattern;
		}
	}
}

/////////////////////////////////////////////////////////////////////////////////////

static WRITE8_HANDLER( mux2datlo_w )
{
	mux2_datalo = data;
}

/////////////////////////////////////////////////////////////////////////////////////

static WRITE8_HANDLER( mux2dathi_w )
{
	mux2_datahi = data;
}

/////////////////////////////////////////////////////////////////////////////////////

static WRITE8_HANDLER( watchdog_w )
{
	watchdog_kicked = 1;
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
	return acia_status;
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
	triac_latch = data;
}

/////////////////////////////////////////////////////////////////////////////////////

static READ8_HANDLER( triac_r )
{
	return triac_latch;
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
	send_to_adder(space->machine,data);
}

///////////////////////////////////////////////////////////////////////////

static WRITE8_HANDLER( vid_uart_ctrl_w )
{
}

///////////////////////////////////////////////////////////////////////////

static READ8_HANDLER( vid_uart_rx_r )
{
	int data = receive_from_adder();

	return data;
}

///////////////////////////////////////////////////////////////////////////

static READ8_HANDLER( vid_uart_ctrl_r )
{
	int status = 0;

	if ( adder2_data_to_sc2  ) status |= 0x01; // receive  buffer full
	if ( !adder2_data_from_sc2) status |= 0x02; // transmit buffer empty

	return status;
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

static UINT8 codec_data[256];


static void decode_sc1(running_machine *machine,const char *rom_region)
{
	UINT8 *tmp, *rom;

	rom = memory_region(machine,rom_region);

	tmp = alloc_array_or_die(UINT8, 0x10000);

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

		codec_data[i] = newdata;
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

			rom[newaddress] = codec_data[ tmp[address] ];
		}
		free( tmp );
	}
}
// machine start (called only once) /////////////////////////////////////////////////

static MACHINE_RESET( bfm_sc1 )
{
	BFM_BD1_init(0);
	vfd_latch         = 0;
	mmtr_latch        = 0;
	triac_latch       = 0;
	irq_status        = 0;
	is_timer_enabled  = 1;
	coin_inhibits     = 0;
	mux1_outputlatch  = 0x08;	// clock HIGH
	mux1_datalo       = 0;
	mux1_datahi		  = 0;
	mux1_input        = 0;
	mux2_outputlatch  = 0x08;	// clock HIGH
	mux2_datalo       = 0;
	mux2_datahi		  = 0;
	mux2_input        = 0;

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

		optic_pattern = pattern;

	}

	acia_status   = 0x02; // MC6850 transmit buffer empty !!!
	locked		  = 0x07; // hardware is locked

// init rom bank ////////////////////////////////////////////////////////////////////
	{
		UINT8 *rom = memory_region(machine, "maincpu");

		memory_configure_bank(machine,"bank1", 0, 1, &rom[0x10000], 0);
		memory_configure_bank(machine,"bank1", 1, 3, &rom[0x02000], 0x02000);

		memory_set_bank(machine,"bank1",3);
	}
}

/////////////////////////////////////////////////////////////////////////////////////
// scorpion1 board memory map ///////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////

static ADDRESS_MAP_START( memmap, ADDRESS_SPACE_PROGRAM, 8 )

	AM_RANGE(0x0000, 0x1FFF) AM_RAM AM_BASE_SIZE_GENERIC(nvram) //8k RAM
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
	AM_RANGE(0x8000, 0xFFFF) AM_RAM_WRITE(watchdog_w)	// 32k ROM

ADDRESS_MAP_END

/////////////////////////////////////////////////////////////////////////////////////
// scorpion1 board + adder2 expansion memory map ////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////

static ADDRESS_MAP_START( memmap_adder2, ADDRESS_SPACE_PROGRAM, 8 )

	AM_RANGE(0x0000, 0x1FFF) AM_RAM AM_BASE_SIZE_GENERIC(nvram) //8k RAM
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
	AM_RANGE(0x8000, 0xFFFF) AM_ROM AM_WRITE(watchdog_w)	// 32k ROM

ADDRESS_MAP_END


/////////////////////////////////////////////////////////////////////////////////////
// scorpion1 board + upd7759 soundcard memory map ///////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////

static ADDRESS_MAP_START( sc1_nec_uk, ADDRESS_SPACE_PROGRAM, 8 )

	AM_RANGE(0x0000, 0x1FFF) AM_RAM AM_BASE_SIZE_GENERIC(nvram) //8k RAM
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
	AM_RANGE(0x8000, 0xFFFF) AM_ROM AM_WRITE(watchdog_w)	// 32k ROM

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

static MACHINE_DRIVER_START( scorpion1 )
	MDRV_MACHINE_RESET(bfm_sc1)							// main scorpion1 board initialisation
	MDRV_CPU_ADD("maincpu", M6809, MASTER_CLOCK/4)			// 6809 CPU at 1 Mhz
	MDRV_CPU_PROGRAM_MAP(memmap)						// setup read and write memorymap
	MDRV_CPU_PERIODIC_INT(timer_irq, 1000 )				// generate 1000 IRQ's per second

	MDRV_SPEAKER_STANDARD_MONO("mono")
	MDRV_SOUND_ADD("aysnd",AY8912, MASTER_CLOCK/4)
	MDRV_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.25)

	MDRV_NVRAM_HANDLER(generic_0fill)
	MDRV_DEFAULT_LAYOUT(layout_awpvid14)
MACHINE_DRIVER_END

/////////////////////////////////////////////////////////////////////////////////////
// machine driver for scorpion1 board + adder2 extension ////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////

static MACHINE_DRIVER_START( scorpion1_adder2 )
	MDRV_IMPORT_FROM( scorpion1 )

	MDRV_CPU_MODIFY("maincpu")
	MDRV_CPU_PROGRAM_MAP(memmap_adder2)				// setup read and write memorymap

	MDRV_DEFAULT_LAYOUT(layout_bfm_sc1)
	MDRV_SCREEN_ADD("adder", RASTER)
	MDRV_SCREEN_FORMAT(BITMAP_FORMAT_INDEXED16)
	MDRV_SCREEN_REFRESH_RATE(50)
	MDRV_SCREEN_SIZE( 400, 300)
	MDRV_SCREEN_VISIBLE_AREA(  0, 400-1, 0, 300-1)

	MDRV_VIDEO_START( adder2)
	MDRV_VIDEO_RESET( adder2)
	MDRV_VIDEO_UPDATE(adder2)

	MDRV_PALETTE_LENGTH(16)

	MDRV_PALETTE_INIT(adder2)
	MDRV_GFXDECODE(adder2)

	MDRV_CPU_ADD("adder2", M6809, ADDER_CLOCK/4 )		// adder2 board 6809 CPU at 2 Mhz
	MDRV_CPU_PROGRAM_MAP(adder2_memmap)				// setup adder2 board memorymap
	MDRV_CPU_VBLANK_INT("adder",adder2_vbl)				// board has a VBL IRQ
MACHINE_DRIVER_END

/////////////////////////////////////////////////////////////////////////////////////
// machine driver for scorpion1 board ///////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////

static MACHINE_DRIVER_START( scorpion1_nec_uk )
	MDRV_IMPORT_FROM( scorpion1 )
	MDRV_CPU_MODIFY("maincpu")
	MDRV_CPU_PROGRAM_MAP(sc1_nec_uk)					// setup read and write memorymap

	MDRV_SOUND_ADD("upd",UPD7759, UPD7759_STANDARD_CLOCK)
	MDRV_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.50)
MACHINE_DRIVER_END

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

static void sc1_common_init(running_machine *machine, int reels, int decrypt)
{
	UINT8 *rom, i;

	rom = memory_region(machine, "maincpu");
	if ( rom )
	{
		memcpy(&rom[0x10000], &rom[0x00000], 0x2000);
	}

	memset(sc1_Inputs, 0, sizeof(sc1_Inputs));

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
	Mechmtr_init(8);

	BFM_BD1_init(0);
}

static DRIVER_INIT(lotse)
{
	sc1_common_init(machine,6,1);
	Mechmtr_init(8);

	BFM_BD1_init(0);
	BFM_BD1_init(1);
}

/////////////////////////////////////////////////////////////////////////////////////

static DRIVER_INIT(rou029)
{
	sc1_common_init(machine,6,0);
	Mechmtr_init(8);

	BFM_BD1_init(0);
}

/////////////////////////////////////////////////////////////////////////////////////

static DRIVER_INIT(clatt)
{
	sc1_common_init(machine,6,1);
	Mechmtr_init(8);

	BFM_BD1_init(0);

	Scorpion1_SetSwitchState(3,2,1);
	Scorpion1_SetSwitchState(3,3,1);
	Scorpion1_SetSwitchState(3,6,1);
	Scorpion1_SetSwitchState(4,1,1);
}

/////////////////////////////////////////////////////////////////////////////////////

//    year, name,     parent,   machine,            input,            init,    monitor, company,       fullname,                                    flags
GAME( 1988, m_lotsse, 0,        scorpion1,			scorpion1,		  lotse,	0,       "BFM/ELAM",    "Lotus SE (Dutch)",							GAME_NOT_WORKING|GAME_SUPPORTS_SAVE|GAME_REQUIRES_ARTWORK )
GAME( 1988, m_roulet, 0,        scorpion1,			scorpion1,		  rou029,	0,       "BFM/ELAM",    "Roulette (Dutch, Game Card 39-360-129?)",	GAME_NOT_WORKING|GAME_SUPPORTS_SAVE|GAME_REQUIRES_ARTWORK )
GAME( 1990, m_clattr, 0,        scorpion1_nec_uk,	clatt,			  clatt,	0,       "BFM",         "Club attraction (UK, Game Card 39-370-196)",GAME_NOT_WORKING|GAME_SUPPORTS_SAVE|GAME_REQUIRES_ARTWORK )

//Adder
GAME( 1996, m_tppokr, 0,        scorpion1_adder2,	toppoker,		  toppoker,	0,       "BFM/ELAM",    "Toppoker (Dutch, Game Card 95-750-899)",	GAME_NOT_WORKING|GAME_SUPPORTS_SAVE|GAME_REQUIRES_ARTWORK )
