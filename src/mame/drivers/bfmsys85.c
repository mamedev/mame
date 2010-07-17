/***************************************************************************

    Bellfruit system85 driver, (under heavy construction !!!)

    A.G.E Code Copyright J. Wallace and the AGEMAME Development Team.
    Visit http://agemame.mameworld.info for more information.

    M.A.M.E Core Copyright Nicola Salmoria and the MAME Team,
    used under license from http://mamedev.org


******************************************************************************************

  19-08-2005: Re-Animator

Standard system85 memorymap
___________________________________________________________________________
   hex     |r/w| D D D D D D D D |
 location  |   | 7 6 5 4 3 2 1 0 | function
-----------+---+-----------------+-----------------------------------------
0000-1FFF  |R/W| D D D D D D D D | RAM (8k) battery backed up
-----------+---+-----------------+-----------------------------------------
2000-21FF  | W | D D D D D D D D | Reel 3 + 4 stepper latch
-----------+---+-----------------+-----------------------------------------
2200-23FF  | W | D D D D D D D D | Reel 1 + 2 stepper latch
-----------+---+-----------------+-----------------------------------------
2400-25FF  | W | D D D D D D D D | vfd + coin inhibits
-----------+---+-----------------+-----------------------------------------
2600-27FF  | W | D D D D D D D D | electro mechanical meters
-----------+---+-----------------+-----------------------------------------
2800-28FF  | W | D D D D D D D D | triacs used for payslides/hoppers
-----------+---+-----------------+-----------------------------------------
2A00       |R/W| D D D D D D D D | MUX data
-----------+---+-----------------+-----------------------------------------
2A01       | W | D D D D D D D D | MUX control
-----------+---+-----------------+-----------------------------------------
2E00       | R | ? ? ? ? ? ? D D | IRQ status
-----------+---+-----------------+-----------------------------------------
3000       | W | D D D D D D D D | AY8912 data
-----------+---+-----------------+-----------------------------------------
3200       | W | D D D D D D D D | AY8912 address reg
-----------+---+-----------------+-----------------------------------------
3402       |R/W| D D D D D D D D | MC6850 control reg
-----------+---+-----------------+-----------------------------------------
3403       |R/W| D D D D D D D D | MC6850 data
-----------+---+-----------------+-----------------------------------------
3600       | W | ? ? ? ? ? ? D D | MUX enable
-----------+---+-----------------+-----------------------------------------
4000-5FFF  | R | D D D D D D D D | ROM (8k)
-----------+---+-----------------+-----------------------------------------
6000-7FFF  | R | D D D D D D D D | ROM (8k)
-----------+---+-----------------+-----------------------------------------
8000-FFFF  | R | D D D D D D D D | ROM (32k)
-----------+---+-----------------+-----------------------------------------

  TODO: - change this

***************************************************************************/

#include "emu.h"
#include "cpu/m6809/m6809.h"
#include "video/awpvid.h"
#include "machine/6850acia.h"
#include "machine/meters.h"
#include "machine/roc10937.h"  // vfd
#include "machine/steppers.h" // stepper motor
#include "sound/ay8910.h"

#define VFD_RESET  0x20
#define VFD_CLOCK1 0x80
#define VFD_DATA   0x40

#define MASTER_CLOCK	(XTAL_4MHz)

// local prototypes ///////////////////////////////////////////////////////


// local vars /////////////////////////////////////////////////////////////

static int mmtr_latch;		  // mechanical meter latch
static int triac_latch;		  // payslide triac latch
static int vfd_latch;		  // vfd latch
static int irq_status;		  // custom chip IRQ status
static int optic_pattern;     // reel optics
static int locked;			  // hardware lock/unlock status (0=unlocked)
static int is_timer_enabled;
static int reel_changed;
static int coin_inhibits;
static int mux_output_strobe;
static int mux_input_strobe;
static int mux_input;

// user interface stuff ///////////////////////////////////////////////////

static UINT8 Lamps[128];		  // 128 multiplexed lamps
static UINT8 Inputs[64];		  // ??  multiplexed inputs

///////////////////////////////////////////////////////////////////////////
// Serial Communications (Where does this go?) ////////////////////////////
///////////////////////////////////////////////////////////////////////////

static UINT8 sys85_data_line_r;
static UINT8 sys85_data_line_t;

static READ_LINE_DEVICE_HANDLER( sys85_data_r )
{
	return sys85_data_line_r;
}

static WRITE_LINE_DEVICE_HANDLER( sys85_data_w )
{
	sys85_data_line_t = state;
}

static ACIA6850_INTERFACE( m6809_acia_if )
{
	500000,
	500000,
	DEVCB_LINE(sys85_data_r),
	DEVCB_LINE(sys85_data_w),
	DEVCB_NULL
};

///////////////////////////////////////////////////////////////////////////
// called if board is reset ///////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////

static MACHINE_RESET( bfm_sys85 )
{
	vfd_latch         = 0;
	mmtr_latch        = 0;
	triac_latch       = 0;
	irq_status        = 0;
	is_timer_enabled  = 1;
	coin_inhibits     = 0;
	mux_output_strobe = 0;
	mux_input_strobe  = 0;
	mux_input         = 0;

	ROC10937_reset(0);	// reset display1

// reset stepper motors ///////////////////////////////////////////////////
	{
		int pattern =0, i;

		for ( i = 0; i < 6; i++)
		{
			stepper_reset_position(i);
			if ( stepper_optic_state(i) ) pattern |= 1<<i;
		}
	optic_pattern = pattern;
	}
	locked		  = 0x00; // hardware is NOT locked
}

///////////////////////////////////////////////////////////////////////////

static WRITE8_HANDLER( watchdog_w )
{
}

///////////////////////////////////////////////////////////////////////////

static INTERRUPT_GEN( timer_irq )
{
	if ( is_timer_enabled )
	{
		irq_status = 0x01 |0x02; //0xff;
		generic_pulse_irq_line(device, M6809_IRQ_LINE);
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
	if ( stepper_update(0, data>>4) ) reel_changed |= 0x01;
	if ( stepper_update(1, data   ) ) reel_changed |= 0x02;

	if ( stepper_optic_state(0) ) optic_pattern |=  0x01;
	else                          optic_pattern &= ~0x01;
	if ( stepper_optic_state(1) ) optic_pattern |=  0x02;
	else                          optic_pattern &= ~0x02;
	awp_draw_reel(0);
	awp_draw_reel(1);
}

///////////////////////////////////////////////////////////////////////////

static WRITE8_HANDLER( reel34_w )
{
	if ( stepper_update(2, data>>4) ) reel_changed |= 0x04;
	if ( stepper_update(3, data   ) ) reel_changed |= 0x08;

	if ( stepper_optic_state(2) ) optic_pattern |=  0x04;
	else                          optic_pattern &= ~0x04;
	if ( stepper_optic_state(3) ) optic_pattern |=  0x08;
	else                          optic_pattern &= ~0x08;
	awp_draw_reel(2);
	awp_draw_reel(3);
}

///////////////////////////////////////////////////////////////////////////
// mechanical meters //////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////

static WRITE8_HANDLER( mmtr_w )
{
	int i;
	int  changed = mmtr_latch ^ data;
	UINT64 cycles  = downcast<cpu_device *>(space->cpu)->total_cycles();

	mmtr_latch = data;

	for (i=0; i<8; i++)
	if ( changed & (1 << i) )	Mechmtr_update(i, cycles, data & (1 << i) );

	if ( data ) generic_pulse_irq_line(space->machine->device("maincpu"), M6809_FIRQ_LINE);
}
///////////////////////////////////////////////////////////////////////////

static READ8_HANDLER( mmtr_r )
{
	return mmtr_latch;
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
				ROC10937_reset(0);
				ROC10937_reset(1);
				ROC10937_reset(2);
			}
		}

		if ( changed & VFD_CLOCK1 )
		{ // clock line changed
			if ( !(data & VFD_CLOCK1) && (data & VFD_RESET) )
			{ // new data clocked into vfd //////////////////////////////////////
				ROC10937_shift_data(0, data & VFD_DATA );
			}
		}
		ROC10937_draw_16seg(0);
	}
}

//////////////////////////////////////////////////////////////////////////////////
// input / output multiplexers ///////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////
// conversion table BFM strobe data to internal lamp numbers
static const UINT8 BFM_strcnv85[] =
{
	0x00,0x01,0x02,0x03,0x04,0x05,0x06,0x07, 0x08,0x09,0x0A,0x0B,0x0C,0x0D,0x0E,0x0F,
	0x10,0x11,0x12,0x13,0x14,0x15,0x16,0x17, 0x18,0x19,0x1A,0x1B,0x1C,0x1D,0x1E,0x1F,
	0x20,0x21,0x22,0x23,0x24,0x25,0x26,0x27, 0x28,0x29,0x2A,0x2B,0x2C,0x2D,0x2E,0x2F,
	0x30,0x31,0x32,0x33,0x34,0x35,0x36,0x37, 0x38,0x39,0x3A,0x3B,0x3C,0x3D,0x3E,0x3F,
	0x40,0x41,0x42,0x43,0x44,0x45,0x46,0x47, 0x48,0x49,0x4A,0x4B,0x4C,0x4D,0x4E,0x4F,
	0x50,0x51,0x52,0x53,0x54,0x55,0x56,0x57, 0x58,0x59,0x5A,0x5B,0x5C,0x5D,0x5E,0x5F,
	0x60,0x61,0x62,0x63,0x64,0x65,0x66,0x67, 0x68,0x69,0x6A,0x6B,0x6C,0x6D,0x6E,0x6F,
	0x70,0x71,0x72,0x73,0x74,0x75,0x76,0x77, 0x78,0x79,0x7A,0x7B,0x7C,0x7D,0x7E,0x7F,

	0x80,0x81,0x82,0x83,0x84,0x85,0x86,0x87, 0x88,0x89,0x8A,0x8B,0x8C,0x8D,0x8E,0x8F,
	0x90,0x91,0x92,0x93,0x94,0x95,0x96,0x97, 0x98,0x99,0x9A,0x9B,0x9C,0x9D,0x9E,0x9F,
	0xA0,0xA1,0xA2,0xA3,0xA4,0xA5,0xA6,0xA7, 0xA8,0xA9,0xAA,0xAB,0xAC,0xAD,0xAE,0xAF,
	0xB0,0xB1,0xB2,0xB3,0xB4,0xB5,0xB6,0xB7, 0xB8,0xB9,0xBA,0xBB,0xBC,0xBD,0xBE,0xBF,
	0xC0,0xC1,0xC2,0xC3,0xC4,0xC5,0xC6,0xC7, 0xC8,0xC9,0xCA,0xCB,0xCC,0xCD,0xCE,0xCF,
	0xD0,0xD1,0xD2,0xD3,0xD4,0xD5,0xD6,0xD7, 0xD8,0xD9,0xDA,0xDB,0xDC,0xDD,0xDE,0xDF,
	0xE0,0xE1,0xE2,0xE3,0xE4,0xE5,0xE6,0xE7, 0xE8,0xE9,0xEA,0xEB,0xEC,0xED,0xEE,0xEF,
	0xF0,0xF1,0xF2,0xF3,0xF4,0xF5,0xF6,0xF7, 0xF8,0xF9,0xFA,0xFB,0xFC,0xFD,0xFE,0xFF
};

///////////////////////////////////////////////////////////////////////////////////

static WRITE8_HANDLER( mux_ctrl_w )
{
	switch ( data & 0xF0 )
	{
		case 0x10:
		//logerror(" sys85 mux: left entry matrix scan %02X\n", data & 0x0F);
		break;

		case 0x20:
		//logerror(" sys85 mux: set scan rate %02X\n", data & 0x0F);
		break;

		case 0x40:
		//logerror(" sys85 mux: read strobe");
		mux_input_strobe = data & 0x07;

		if ( mux_input_strobe == 5 ) Inputs[5] = 0xFF ^ optic_pattern;

		mux_input = ~Inputs[mux_input_strobe];
		break;

		case 0x80:
		mux_output_strobe = data & 0x0F;
		break;

		case 0xC0:
		//logerror(" sys85 mux: clear all outputs\n");
		break;

		case 0xE0:	  // End of interrupt
		break;

	}
}

static READ8_HANDLER( mux_ctrl_r )
{
  // software waits for bit7 to become low

  return 0;
}

static WRITE8_HANDLER( mux_data_w )
{
	int pattern = 0x01, i,
	off = mux_output_strobe<<4;

	for ( i = 0; i < 8; i++ )
	{
		Lamps[BFM_strcnv85[off]] = data & pattern ? 1 : 0;
		pattern <<= 1;
		off++;
	}

	if (mux_output_strobe == 0)
	{
		for ( i = 0; i < 128; i++ )
		{
			output_set_lamp_value(i, Lamps[i]);
		}
	}
}

static READ8_HANDLER( mux_data_r )
{
	return mux_input;
}

///////////////////////////////////////////////////////////////////////////

static WRITE8_HANDLER( mux_enable_w )
{
}

///////////////////////////////////////////////////////////////////////////
// payslide triacs ////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////

static WRITE8_HANDLER( triac_w )
{
	triac_latch = data;
}

///////////////////////////////////////////////////////////////////////////

static READ8_HANDLER( triac_r )
{
	return triac_latch;
}

// machine start (called only once) ////////////////////////////////////////

static MACHINE_START( bfm_sys85 )
{
	int i;
	for ( i = 0; i < 4; i++ )
	{
		stepper_config(machine, i, &starpoint_interface_48step);
	}

	ROC10937_init(0,MSC1937,1);//?

	awp_reel_setup();
}

// memory map for bellfruit system85 board ////////////////////////////////

static ADDRESS_MAP_START( memmap, ADDRESS_SPACE_PROGRAM, 8 )

	AM_RANGE(0x0000, 0x1fff) AM_RAM AM_BASE_SIZE_GENERIC(nvram) //8k RAM
	AM_RANGE(0x2000, 0x21FF) AM_WRITE(reel34_w)			// reel 3+4 latch
	AM_RANGE(0x2200, 0x23FF) AM_WRITE(reel12_w)			// reel 1+2 latch
	AM_RANGE(0x2400, 0x25FF) AM_WRITE(vfd_w)			// vfd latch

	AM_RANGE(0x2600, 0x27FF) AM_READWRITE(mmtr_r,mmtr_w)// mechanical meter latch
	AM_RANGE(0x2800, 0x2800) AM_READ(triac_r)			// payslide triacs
	AM_RANGE(0x2800, 0x29FF) AM_WRITE(triac_w)			// triacs

	AM_RANGE(0x2A00, 0x2A00) AM_READWRITE(mux_data_r,mux_data_w)// mux
	AM_RANGE(0x2A01, 0x2A01) AM_READWRITE(mux_ctrl_r,mux_ctrl_w)// mux status register
	AM_RANGE(0x2E00, 0x2E00) AM_READ(irqlatch_r)		// irq latch ( MC6850 / timer )

	AM_RANGE(0x3000, 0x3000) AM_DEVWRITE("aysnd", ay8910_data_w)
	AM_RANGE(0x3001, 0x3001) AM_READNOP //sound latch
	AM_RANGE(0x3200, 0x3200) AM_DEVWRITE("aysnd", ay8910_address_w)

	AM_RANGE(0x3402, 0x3402) AM_DEVWRITE("acia6850_0", acia6850_ctrl_w)
	AM_RANGE(0x3403, 0x3403) AM_DEVWRITE("acia6850_0", acia6850_data_w)

	AM_RANGE(0x3406, 0x3406) AM_DEVREAD("acia6850_0", acia6850_stat_r)
	AM_RANGE(0x3407, 0x3407) AM_DEVREAD("acia6850_0", acia6850_data_r)

	AM_RANGE(0x3600, 0x3600) AM_WRITE(mux_enable_w)		// mux enable

	AM_RANGE(0x4000, 0xffff) AM_ROM						// 48K ROM
	AM_RANGE(0x8000, 0xFFFF) AM_WRITE(watchdog_w)		// kick watchdog

ADDRESS_MAP_END

// machine driver for system85 board //////////////////////////////////////

static MACHINE_DRIVER_START( bfmsys85 )
	MDRV_MACHINE_START(bfm_sys85)						// main system85 board initialisation
	MDRV_MACHINE_RESET(bfm_sys85)
	MDRV_CPU_ADD("maincpu", M6809, MASTER_CLOCK/4)			// 6809 CPU at 1 Mhz
	MDRV_CPU_PROGRAM_MAP(memmap)						// setup read and write memorymap
	MDRV_CPU_PERIODIC_INT(timer_irq, 1000 )				// generate 1000 IRQ's per second

	MDRV_ACIA6850_ADD("acia6850_0", m6809_acia_if)

	MDRV_SPEAKER_STANDARD_MONO("mono")
	MDRV_SOUND_ADD("aysnd",AY8912, MASTER_CLOCK/4)			// add AY8912 soundchip
	MDRV_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.25)

	MDRV_NVRAM_HANDLER(generic_0fill)					// load/save nv RAM

	MDRV_DEFAULT_LAYOUT(layout_awpvid16)
MACHINE_DRIVER_END

// input ports for system85 board /////////////////////////////////////////

static INPUT_PORTS_START( bfmsys85 )
	PORT_START("IN0")
    PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_COIN1 ) PORT_IMPULSE(3) PORT_NAME("Fl 5.00")
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_COIN2 ) PORT_IMPULSE(3) PORT_NAME("Fl 2.50")
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_COIN3 ) PORT_IMPULSE(3) PORT_NAME("Fl 1.00")
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_COIN4 ) PORT_IMPULSE(3) PORT_NAME("Fl 0.25")
	PORT_BIT( 0xF0, IP_ACTIVE_HIGH, IPT_UNKNOWN  )

	PORT_START("IN1")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_BUTTON1) PORT_NAME("10")
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_BUTTON2) PORT_NAME("11")
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_BUTTON3) PORT_NAME("12")
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_BUTTON4) PORT_NAME("13")
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_BUTTON5) PORT_NAME("14")
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_BUTTON6) PORT_NAME("15")
	PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_BUTTON7) PORT_NAME("16")
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_BUTTON8) PORT_NAME("17")

	//PORT_BIT( 0x01, IP_ACTIVE_HIGH,IPT_SERVICE) PORT_NAME("Refill Key") PORT_CODE(KEYCODE_R) PORT_TOGGLE
	//PORT_BIT( 0x02, IP_ACTIVE_HIGH,IPT_SERVICE) PORT_NAME("Bookkeeping") PORT_CODE(KEYCODE_F1) PORT_TOGGLE
INPUT_PORTS_END


// ROM definition /////////////////////////////////////////////////////////

ROM_START( m_supcrd )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "sc271.bin",  0x8000, 0x8000,  CRC(58e9c9df) SHA1(345c5aa279327d7142edc6823aad0cfd40cbeb73))
ROM_END

GAME( 1985,m_supcrd, 0, 	bfmsys85, bfmsys85, 		0,	  0,       "BFM/ELAM",   "Supercards (Dutch, Game Card 39-340-271?)", GAME_NOT_WORKING|GAME_SUPPORTS_SAVE|GAME_REQUIRES_ARTWORK )
