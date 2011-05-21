/***************************************************************************

    Bellfruit system85 driver, (under heavy construction !!!)

    M.A.M.E Core Copyright Nicola Salmoria and the MAME Team,
    used under license from http://mamedev.org


******************************************************************************************

     04-2011: J Wallace: Fixed lamping code.
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
#include "machine/nvram.h"


class bfmsys85_state : public driver_device
{
public:
	bfmsys85_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag) { }

	int m_mmtr_latch;
	int m_triac_latch;
	int m_vfd_latch;
	int m_irq_status;
	int m_optic_pattern;
	int m_locked;
	int m_is_timer_enabled;
	int m_reel_changed;
	int m_coin_inhibits;
	int m_mux_output_strobe;
	int m_mux_input_strobe;
	int m_mux_input;
	UINT8 m_Inputs[64];
	UINT8 m_sys85_data_line_r;
	UINT8 m_sys85_data_line_t;
};


#define VFD_RESET  0x20
#define VFD_CLOCK1 0x80
#define VFD_DATA   0x40

#define MASTER_CLOCK	(XTAL_4MHz)

///////////////////////////////////////////////////////////////////////////
// Serial Communications (Where does this go?) ////////////////////////////
///////////////////////////////////////////////////////////////////////////


static READ_LINE_DEVICE_HANDLER( sys85_data_r )
{
	bfmsys85_state *state = device->machine().driver_data<bfmsys85_state>();
	return state->m_sys85_data_line_r;
}

static WRITE_LINE_DEVICE_HANDLER( sys85_data_w )
{
	bfmsys85_state *drvstate = device->machine().driver_data<bfmsys85_state>();
	drvstate->m_sys85_data_line_t = state;
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
	bfmsys85_state *state = machine.driver_data<bfmsys85_state>();
	state->m_vfd_latch         = 0;
	state->m_mmtr_latch        = 0;
	state->m_triac_latch       = 0;
	state->m_irq_status        = 0;
	state->m_is_timer_enabled  = 1;
	state->m_coin_inhibits     = 0;
	state->m_mux_output_strobe = 0;
	state->m_mux_input_strobe  = 0;
	state->m_mux_input         = 0;

	ROC10937_reset(0);	// reset display1

// reset stepper motors ///////////////////////////////////////////////////
	{
		int pattern =0, i;

		for ( i = 0; i < 6; i++)
		{
			stepper_reset_position(i);
			if ( stepper_optic_state(i) ) pattern |= 1<<i;
		}
	state->m_optic_pattern = pattern;
	}
	state->m_locked		  = 0x00; // hardware is open
}

///////////////////////////////////////////////////////////////////////////

static WRITE8_HANDLER( watchdog_w )
{
}

///////////////////////////////////////////////////////////////////////////

static INTERRUPT_GEN( timer_irq )
{
	bfmsys85_state *state = device->machine().driver_data<bfmsys85_state>();
	if ( state->m_is_timer_enabled )
	{
		state->m_irq_status = 0x01 |0x02; //0xff;
		generic_pulse_irq_line(device, M6809_IRQ_LINE);
	}
}

///////////////////////////////////////////////////////////////////////////

static READ8_HANDLER( irqlatch_r )
{
	bfmsys85_state *state = space->machine().driver_data<bfmsys85_state>();
	int result = state->m_irq_status | 0x02;

	state->m_irq_status = 0;

	return result;
}

///////////////////////////////////////////////////////////////////////////

static WRITE8_HANDLER( reel12_w )
{
	bfmsys85_state *state = space->machine().driver_data<bfmsys85_state>();
	if ( stepper_update(0, (data>>4)&0x0f) ) state->m_reel_changed |= 0x01;
	if ( stepper_update(1, data&0x0f   ) ) state->m_reel_changed |= 0x02;

	if ( stepper_optic_state(0) ) state->m_optic_pattern |=  0x01;
	else                          state->m_optic_pattern &= ~0x01;
	if ( stepper_optic_state(1) ) state->m_optic_pattern |=  0x02;
	else                          state->m_optic_pattern &= ~0x02;
	awp_draw_reel(0);
	awp_draw_reel(1);
}

///////////////////////////////////////////////////////////////////////////

static WRITE8_HANDLER( reel34_w )
{
	bfmsys85_state *state = space->machine().driver_data<bfmsys85_state>();
	if ( stepper_update(2, (data>>4)&0x0f) ) state->m_reel_changed |= 0x04;
	if ( stepper_update(3, data&0x0f   ) ) state->m_reel_changed |= 0x08;

	if ( stepper_optic_state(2) ) state->m_optic_pattern |=  0x04;
	else                          state->m_optic_pattern &= ~0x04;
	if ( stepper_optic_state(3) ) state->m_optic_pattern |=  0x08;
	else                          state->m_optic_pattern &= ~0x08;
	awp_draw_reel(2);
	awp_draw_reel(3);
}

///////////////////////////////////////////////////////////////////////////
// mechanical meters //////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////

static WRITE8_HANDLER( mmtr_w )
{
	bfmsys85_state *state = space->machine().driver_data<bfmsys85_state>();
	int i;
	int  changed = state->m_mmtr_latch ^ data;

	state->m_mmtr_latch = data;

	for (i=0; i<8; i++)
	if ( changed & (1 << i) )	MechMtr_update(i, data & (1 << i) );

	if ( data ) generic_pulse_irq_line(space->machine().device("maincpu"), M6809_FIRQ_LINE);
}
///////////////////////////////////////////////////////////////////////////

static READ8_HANDLER( mmtr_r )
{
	bfmsys85_state *state = space->machine().driver_data<bfmsys85_state>();
	return state->m_mmtr_latch;
}
///////////////////////////////////////////////////////////////////////////

static WRITE8_HANDLER( vfd_w )
{
	bfmsys85_state *state = space->machine().driver_data<bfmsys85_state>();
	int changed = state->m_vfd_latch ^ data;

	state->m_vfd_latch = data;

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

static WRITE8_HANDLER( mux_ctrl_w )
{
	bfmsys85_state *state = space->machine().driver_data<bfmsys85_state>();
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
		state->m_mux_input_strobe = data & 0x07;

		if ( state->m_mux_input_strobe == 5 ) state->m_Inputs[5] = 0xFF ^ state->m_optic_pattern;

		state->m_mux_input = ~state->m_Inputs[state->m_mux_input_strobe];
		break;

		case 0x80:
		state->m_mux_output_strobe = data & 0x0F;
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
	bfmsys85_state *state = space->machine().driver_data<bfmsys85_state>();
	int pattern = 0x01, i,
	off = state->m_mux_output_strobe<<4;

	for ( i = 0; i < 8; i++ )
	{
		output_set_lamp_value(off, (data & pattern ? 1 : 0));
		pattern <<= 1;
		off++;
	}
}

static READ8_HANDLER( mux_data_r )
{
	bfmsys85_state *state = space->machine().driver_data<bfmsys85_state>();
	return state->m_mux_input;
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
	bfmsys85_state *state = space->machine().driver_data<bfmsys85_state>();
	state->m_triac_latch = data;
}

///////////////////////////////////////////////////////////////////////////

static READ8_HANDLER( triac_r )
{
	bfmsys85_state *state = space->machine().driver_data<bfmsys85_state>();
	return state->m_triac_latch;
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

static ADDRESS_MAP_START( memmap, AS_PROGRAM, 8 )

	AM_RANGE(0x0000, 0x1fff) AM_RAM AM_SHARE("nvram") //8k RAM
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

	AM_RANGE(0x3402, 0x3402) AM_DEVWRITE_MODERN("acia6850_0", acia6850_device, control_write)
	AM_RANGE(0x3403, 0x3403) AM_DEVWRITE_MODERN("acia6850_0", acia6850_device, data_write)

	AM_RANGE(0x3406, 0x3406) AM_DEVREAD_MODERN("acia6850_0", acia6850_device, status_read)
	AM_RANGE(0x3407, 0x3407) AM_DEVREAD_MODERN("acia6850_0", acia6850_device, data_read)

	AM_RANGE(0x3600, 0x3600) AM_WRITE(mux_enable_w)		// mux enable

	AM_RANGE(0x4000, 0xffff) AM_ROM						// 48K ROM
	AM_RANGE(0x8000, 0xFFFF) AM_WRITE(watchdog_w)		// kick watchdog

ADDRESS_MAP_END

// machine driver for system85 board //////////////////////////////////////

static MACHINE_CONFIG_START( bfmsys85, bfmsys85_state )
	MCFG_MACHINE_START(bfm_sys85)						// main system85 board initialisation
	MCFG_MACHINE_RESET(bfm_sys85)
	MCFG_CPU_ADD("maincpu", M6809, MASTER_CLOCK/4)			// 6809 CPU at 1 Mhz
	MCFG_CPU_PROGRAM_MAP(memmap)						// setup read and write memorymap
	MCFG_CPU_PERIODIC_INT(timer_irq, 1000 )				// generate 1000 IRQ's per second

	MCFG_ACIA6850_ADD("acia6850_0", m6809_acia_if)

	MCFG_SPEAKER_STANDARD_MONO("mono")
	MCFG_SOUND_ADD("aysnd",AY8912, MASTER_CLOCK/4)			// add AY8912 soundchip
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.25)

	MCFG_NVRAM_ADD_0FILL("nvram")						// load/save nv RAM

	MCFG_DEFAULT_LAYOUT(layout_awpvid16)
MACHINE_CONFIG_END

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
