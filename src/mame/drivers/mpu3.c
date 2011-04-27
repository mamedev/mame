/***********************************************************************************************************
  Barcrest MPU3 highly preliminary driver by J.Wallace, and Anonymous.

--- Board Setup ---

This original board uses a ~1.Mhz 6808 CPU, and a number of PIA6821 chips for multiplexing inputs and the like.

A 6840PTM is used for internal timing
A GAME CARD (cartridge) plugs into the board containing the game, and a protection PAL (the 'characteriser').

--- Preliminary MPU3 Memorymap  ---

   hex     |r/w| D D D D D D D D |
 location  |   | 7 6 5 4 3 2 1 0 | function
-----------+---+-----------------+--------------------------------------------------------------------------
 0000-07FF |R/W| D D D D D D D D | 2k RAM
-----------+---+-----------------+--------------------------------------------------------------------------

 8800-881F |R/W| ???????????????? | MC6840 PTM IC2 (unusual bus setup, address shifted two bytes)

Outputs hooked into IC 19

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
9000-      |R/W| D D D D D D D D | PIA6821 IC3
           |   |                 | PA2-PA7 INPUT  multiplexed inputs data
           |   |                 | PA1, CB1 connected to IC2 C1 pin
           |   |                 |        CA1     INPUT, not connected
           |   |                 |        CA2    OUTPUT, A on strobe multiplexer
           |   |                 |        IRQA           Not connected
           |   |                 |
           |   |                 |        CB2 IC19 Triac switch
           |   |                 | Port B triacs

9800       |R/W| D D D D D D D D | PIA6821 IC4
           |   |                 | PA0-PA6 INPUT  Yellow 15 way data (7SEG LED or Meters)
           |   |                 | PA7     802 (Q) IC11 G1
           |   |                 | PB0-PB7 INPUT  IC24
           |   |                 |        CA2    OUTPUT, B on strobe multiplexer
           |   |                 |        CB2    TRiac switch

A000-A003  |R/W| D D D D D D D D | PIA6821 IC5 port A Reel drives
           |   |                 |          port B Reel opto
           |   |                 |
           |   |                 |        CA2    OUTPUT, C on strobe multiplexer
           |   |                 |        CB2    TRiac switch
-----------+---+-----------------+--------------------------------------------------------------------------
A800-A803  |R/W| D D D D D D D D | PIA6821 IC6 PA0-PA7, AUX1 connector (ALPHA)
           |   |                 |             PB0-PB7, AUX2 connector
           |   |                 |
           |   |                 |

-----------+---+-----------------+--------------------------------------------------------------------------
??? B000-FFFF | R | D D D D D D D D | ROM
-----------+---+-----------------+--------------------------------------------------------------------------

TODO: - Distinguish door switches using manual
      - Add discrete sound functionality (same as MPU4 alarm)
      - Is pulse timer ic11 right?
      - 12V meter alarm on hyper viper - how are meters sensed?

***********************************************************************************************************/
#include "emu.h"
#include "machine/6821pia.h"
#include "machine/6840ptm.h"
#include "machine/nvram.h"

#include "cpu/m6800/m6800.h"
#include "machine/steppers.h"
#include "machine/roc10937.h"
#include "machine/meters.h"

#ifdef MAME_DEBUG
#define MPU3VERBOSE 1
#else
#define MPU3VERBOSE 0
#endif

#define LOG(x)	do { if (MPU3VERBOSE) logerror x; } while (0)

#include "video/awpvid.h"		//Fruit Machines Only

#define MPU3_MASTER_CLOCK (XTAL_4MHz)

/* Lookup table for CHR data */

struct mpu3_chr_table
{
	UINT8 call;
	UINT8 response;
};

class mpu3_state : public driver_device
{
public:
	mpu3_state(const machine_config &mconfig, device_type type, const char *tag)
	: driver_device(mconfig, type, tag) { }


int m_alpha_data_line;
int m_alpha_clock;
int m_triac_ic3;
int m_triac_ic4;
int m_triac_ic5;
int m_ic3_data;
int m_IC11G1;
int m_IC11G2A;
int m_IC11G2B;
int m_IC11GC;
int m_IC11GB;
int m_IC11GA;

int m_ic10_output;
int m_ic11_active;
int m_ic11_output;
int m_disp_func;

int m_ic4_input_a;
int m_aux1_input;
int m_aux2_input;
int	m_input_strobe;	  /* IC11 74LS138 A = CA2 IC3, B = CA2 IC4, C = CA2 IC5 */
UINT8 m_lamp_strobe;
UINT8 m_led_strobe;
int m_signal_50hz;

const mpu3_chr_table* m_current_chr_table;
int m_prot_col;

int m_optic_pattern;

emu_timer *m_ic21_timer;
};

#define DISPLAY_PORT 0
#define METER_PORT 1
#define BWB_FUNCTIONALITY 2
static TIMER_CALLBACK( ic21_timeout );

static void update_triacs(running_machine &machine)
{
	mpu3_state *state = machine.driver_data<mpu3_state>();
	int i,triacdata;

	triacdata=state->m_triac_ic3 + (state->m_triac_ic4 << 8) + (state->m_triac_ic5 << 9);

	for (i = 0; i < 8; i++)
	{
		output_set_indexed_value("triac", i, triacdata & (1 << i));
	}
}

/* called if board is reset */
static void mpu3_stepper_reset(running_machine &machine)
{
	mpu3_state *state = machine.driver_data<mpu3_state>();
	int pattern = 0,reel;
	for (reel = 0; reel < 6; reel++)
	{
		stepper_reset_position(reel);
		if (stepper_optic_state(reel)) pattern |= 1<<reel;
	}
	state->m_optic_pattern = pattern;
}

static MACHINE_RESET( mpu3 )
{
	mpu3_state *state = machine.driver_data<mpu3_state>();
	ROC10937_reset(0);	/* reset display1 */

	mpu3_stepper_reset(machine);

	state->m_lamp_strobe   = 0;
	state->m_led_strobe    = 0;

	state->m_IC11GC    = 0;
	state->m_IC11GB    = 0;
	state->m_IC11GA    = 0;
	state->m_IC11G1    = 1;
	state->m_IC11G2A   = 0;
	state->m_IC11G2B   = 0;
}

/* 6808 IRQ handler */
static WRITE_LINE_DEVICE_HANDLER( cpu0_irq )
{
	device_t *pia3 = device->machine().device("pia_ic3");
	device_t *pia4 = device->machine().device("pia_ic4");
	device_t *pia5 = device->machine().device("pia_ic5");
	device_t *pia6 = device->machine().device("pia_ic6");
	device_t *ptm2 = device->machine().device("ptm_ic2");

	/* The PIA and PTM IRQ lines are all connected to a common PCB track, leading directly to the 6809 IRQ line. */
	int combined_state = pia6821_get_irq_a(pia3) | pia6821_get_irq_b(pia3) |
						 pia6821_get_irq_a(pia4) | pia6821_get_irq_b(pia4) |
						 pia6821_get_irq_a(pia5) | pia6821_get_irq_b(pia5) |
						 pia6821_get_irq_a(pia6) | pia6821_get_irq_b(pia6) |
						 ptm6840_get_irq(ptm2);

		cputag_set_input_line(device->machine(), "maincpu", M6800_IRQ_LINE, combined_state ? ASSERT_LINE : CLEAR_LINE);
		LOG(("6808 int%d \n", combined_state));
}


/* IC2 6840 PTM handler probably clocked from elsewhere*/
static WRITE8_DEVICE_HANDLER( ic2_o1_callback )
{
}

//FIXME FROM HERE
static WRITE8_DEVICE_HANDLER( ic2_o2_callback )
{
}


static WRITE8_DEVICE_HANDLER( ic2_o3_callback )
{
}


static const ptm6840_interface ptm_ic2_intf =
{
	MPU3_MASTER_CLOCK,///4,
	{ 0, 0, 0 },
	{ DEVCB_HANDLER(ic2_o1_callback),
	  DEVCB_HANDLER(ic2_o2_callback),
	  DEVCB_HANDLER(ic2_o3_callback) },
	DEVCB_LINE(cpu0_irq)
};

/*
IC23 emulation

IC23 is a 74LS138 1-of-8 Decoder

It is used as a multiplexer for the LEDs, lamp selects and inputs.*/

static void ic11_update(mpu3_state *state)
{
	if (!state->m_IC11G2A)
	{
		if (!state->m_IC11G2B)
		{
			if (state->m_IC11G1)
			{
				if ( state->m_IC11GA )	state->m_input_strobe |= 0x01;
				else					state->m_input_strobe &= ~0x01;

				if ( state->m_IC11GB )	state->m_input_strobe |= 0x02;
				else					state->m_input_strobe &= ~0x02;

				if ( state->m_IC11GC )	state->m_input_strobe |= 0x04;
				else					state->m_input_strobe &= ~0x04;
			}
		}
	}
	else
	if ((state->m_IC11G2A)||(state->m_IC11G2B)||(!state->m_IC11G1))
	{
		state->m_input_strobe = 0x00;
	}
}


/*
IC21 emulation

IC21 is an 8602 or equivalent pulse generator

CLEAR and B2 are tied high, so a high to low transition on A2
(tied to IC11 GA) will trigger operation.

Cx = 2.2 uF

Rx = 47k

K = ~0.34

t/ns = .34 * 47 * 2.2e6 [ 1+ (1/47)]


This seems less stable than the revised version used in MPU4
*/
static void ic21_output(mpu3_state *state,int data)
{
	state->m_IC11G1 = data;
	ic11_update(state);
}

static void ic21_setup(mpu3_state *state)
{
	if (state->m_IC11GA)
	{
		{
			state->m_ic11_active=1;
			ic21_output(state,1);
			state->m_ic21_timer->adjust(attotime::from_nsec( (0.34 * 47 * 2200000) *(1+(1/47))));
		}
	}
}

static TIMER_CALLBACK( ic21_timeout )
{
	mpu3_state *state = machine.driver_data<mpu3_state>();
	state->m_ic11_active=0;
	ic21_output(state,0);
}

static READ8_DEVICE_HANDLER( pia_ic3_porta_r )
{
	mpu3_state *state = device->machine().driver_data<mpu3_state>();
	static const char *const portnames[] = { "ORANGE1", "ORANGE2", "BLACK1", "BLACK2", "DIL1", "DIL1", "DIL2", "DIL2" };
	int data=0,swizzle;
	LOG(("%s: IC3 PIA Read of Port A (MUX input data)\n", device->machine().describe_context()));
	popmessage("%x",state->m_input_strobe);
	switch (state->m_input_strobe)
	{
		case 0:
		case 1:
		case 2:
		case 3:
		{
			data = (input_port_read(device->machine(), portnames[state->m_input_strobe])<<2);
			break;
		}
		case 4://DIL1
		case 6://DIL2
		{
			swizzle = (input_port_read(device->machine(), portnames[state->m_input_strobe]));
			data = (((swizzle & 0x01) << 7) + ((swizzle & 0x02) << 5) + ((swizzle & 0x04) << 3)
					+ ((swizzle & 0x08) << 1) +((swizzle & 0x10) >> 1) + ((swizzle & 0x20) >> 3));
			break;
		}
		case 5://DIL1
		case 7://DIL2
		{
			swizzle = (input_port_read(device->machine(), portnames[state->m_input_strobe]));
			data = (((swizzle & 0x80) >> 1) + ((swizzle & 0x40) << 1));
			break;
		}
	}
	if (state->m_signal_50hz)
	{
		data |= 0x02;
	}
	else
	{
		data &= ~0x02;
	}
	return data;
}

static WRITE8_DEVICE_HANDLER( pia_ic3_portb_w )
{
	mpu3_state *state = device->machine().driver_data<mpu3_state>();
	LOG(("%s: IC3 PIA Port B Set to %2x (Triac)\n", device->machine().describe_context(),data));
	state->m_triac_ic3 =data;
}


static WRITE_LINE_DEVICE_HANDLER( pia_ic3_ca2_w )
{
	mpu3_state *mstate = device->machine().driver_data<mpu3_state>();
	LOG(("%s: IC3 PIA Port CA2 Set to %2x (input A)\n", device->machine().describe_context(),state));
	mstate->m_IC11GA = state;
	ic21_setup(mstate);
	ic11_update(mstate);
}

static const pia6821_interface pia_ic3_intf =
{
	DEVCB_HANDLER(pia_ic3_porta_r),		/* port A in */
	DEVCB_NULL,		/* port B in */
	DEVCB_NULL,		/* line CA1 in */
	DEVCB_NULL,		/* line CB1 in */
	DEVCB_NULL,		/* line CA2 in */
	DEVCB_NULL,		/* line CB2 in */
	DEVCB_NULL,		/* port A out */
	DEVCB_HANDLER(pia_ic3_portb_w),		/* port B out */
	DEVCB_LINE(pia_ic3_ca2_w),			/* line CA2 out */
	DEVCB_NULL,							/* port CB2 out */
	DEVCB_NULL,							/* IRQA */
	DEVCB_LINE(cpu0_irq)				/* IRQB */
};

static READ8_DEVICE_HANDLER( pia_ic4_porta_r )
{
	mpu3_state *state = device->machine().driver_data<mpu3_state>();
	if (state->m_ic11_active)
	{
		state->m_ic4_input_a|=0x80;
	}
	else
	{
		state->m_ic4_input_a&=~0x80;
	}
	return state->m_ic4_input_a;
}

/*  IC4, 7 seg leds */
static WRITE8_DEVICE_HANDLER( pia_ic4_porta_w )
{
	mpu3_state *state = device->machine().driver_data<mpu3_state>();
	int meter,swizzle;
	LOG(("%s: IC4 PIA Port A Set to %2x (DISPLAY PORT)\n", device->machine().describe_context(),data));
	state->m_ic4_input_a=data;
	switch (state->m_disp_func)
	{
		case DISPLAY_PORT:
		if(state->m_ic11_active)
		{
			if(state->m_led_strobe != state->m_input_strobe)
			{
				swizzle = ((state->m_ic4_input_a & 0x01) << 2)+(state->m_ic4_input_a & 0x02)+((state->m_ic4_input_a & 0x4) >> 2)+(state->m_ic4_input_a & 0x08)+((state->m_ic4_input_a & 0x10) << 2)+(state->m_ic4_input_a & 0x20)+((state->m_ic4_input_a & 0x40) >> 2);
				output_set_digit_value(7 - state->m_input_strobe,swizzle);
			}
			state->m_led_strobe = state->m_input_strobe;
		}
		break;

		case METER_PORT:
		for (meter = 0; meter < 6; meter ++)
		{
			swizzle = ((state->m_ic4_input_a ^ 0xff) & 0x3f);
			MechMtr_update(meter, (swizzle & (1 << meter)));
		}
		break;

		case BWB_FUNCTIONALITY:
			//Need to find a game to work this out, MFME has a specific option for it, but I see no activity there.
		break;

	}
}

static WRITE8_DEVICE_HANDLER( pia_ic4_portb_w )
{
	mpu3_state *state = device->machine().driver_data<mpu3_state>();
	LOG(("%s: IC4 PIA Port B Set to %2x (Lamp)\n", device->machine().describe_context(),data));
	int i;
	if(state->m_ic11_active)
	{
		if (state->m_lamp_strobe != state->m_input_strobe)
		{
			// Because of the nature of the lamping circuit, there is an element of persistance where the lamp retains residual charge
			// As a consequence, the lamp column data can change before the input strobe (effectively writing 0 to the previous strobe)
			// without causing the relevant lamps to black out.

			for (i = 0; i < 8; i++)
			{
				output_set_lamp_value((8*state->m_input_strobe)+i, ((data  & (1 << i)) !=0));
			}
			state->m_lamp_strobe = state->m_input_strobe;
		}
	}
}

static WRITE_LINE_DEVICE_HANDLER( pia_ic4_ca2_w )
{
	mpu3_state *mstate = device->machine().driver_data<mpu3_state>();
	LOG(("%s: IC4 PIA Port CA2 Set to %2x (Input B)\n", device->machine().describe_context(),state));
	mstate->m_IC11GB = state;
	ic11_update(mstate);
}

static WRITE_LINE_DEVICE_HANDLER( pia_ic4_cb2_w )
{
	mpu3_state *mstate = device->machine().driver_data<mpu3_state>();
	LOG(("%s: IC4 PIA Port CA2 Set to %2x (Triac)\n", device->machine().describe_context(),state));
	mstate->m_triac_ic4=state;
}

static const pia6821_interface pia_ic4_intf =
{
	DEVCB_HANDLER(pia_ic4_porta_r),		/* port A in */
	DEVCB_NULL,		/* port B in */
	DEVCB_NULL,		/* line CA1 in */
	DEVCB_NULL,		/* line CB1 in */
	DEVCB_NULL,		/* line CA2 in */
	DEVCB_NULL,		/* line CB2 in */
	DEVCB_HANDLER(pia_ic4_porta_w),		/* port A out */
	DEVCB_HANDLER(pia_ic4_portb_w),		/* port B out */
	DEVCB_LINE(pia_ic4_ca2_w),		/* line CA2 out */
	DEVCB_LINE(pia_ic4_cb2_w),		/* line CB2 out */
	DEVCB_LINE(cpu0_irq),		/* IRQA */
	DEVCB_NULL		/* IRQB */
};

/* IC5, AUX ports, coin lockouts and AY sound chip select (MODs below 4 only) */
static WRITE8_DEVICE_HANDLER( pia_ic5_porta_w )
{
	mpu3_state *state = device->machine().driver_data<mpu3_state>();

	LOG(("%s: IC5 PIA Port A Set to %2x (Reel)\n", device->machine().describe_context(),data));
	stepper_update(0, data & 0x03 );
	stepper_update(1, (data>>2) & 0x03 );
	stepper_update(2, (data>>4) & 0x03 );
	stepper_update(3, (data>>6) & 0x03 );
	awp_draw_reel(0);
	awp_draw_reel(1);
	awp_draw_reel(2);
	awp_draw_reel(3);

	{
		if ( stepper_optic_state(0) ) state->m_optic_pattern |=  0x01;
		else                          state->m_optic_pattern &= ~0x01;

		if ( stepper_optic_state(1) ) state->m_optic_pattern |=  0x02;
		else                          state->m_optic_pattern &= ~0x02;
		if ( stepper_optic_state(2) ) state->m_optic_pattern |=  0x04;
		else                          state->m_optic_pattern &= ~0x04;

		if ( stepper_optic_state(3) ) state->m_optic_pattern |=  0x08;
		else                          state->m_optic_pattern &= ~0x08;

	}

}

static READ8_DEVICE_HANDLER( pia_ic5_portb_r )
{
	mpu3_state *state = device->machine().driver_data<mpu3_state>();
	if (state->m_ic3_data & 0x80 )
	{
		return state->m_optic_pattern;
	}
	else
	{
		return state->m_ic3_data;
	}
}



static WRITE8_DEVICE_HANDLER( pia_ic5_portb_w )
{
	mpu3_state *state = device->machine().driver_data<mpu3_state>();
	state->m_ic3_data = data;
}

static WRITE_LINE_DEVICE_HANDLER( pia_ic5_ca2_w )
{
	mpu3_state *mstate = device->machine().driver_data<mpu3_state>();
	LOG(("%s: IC5 PIA Port CA2 Set to %2x (C)\n", device->machine().describe_context(),state));
	mstate->m_IC11GC = state;
	ic11_update(mstate);
}

static WRITE_LINE_DEVICE_HANDLER( pia_ic5_cb2_w )
{
	mpu3_state *mstate = device->machine().driver_data<mpu3_state>();
	LOG(("%s: IC5 PIA Port CB2 Set to %2x (Triac)\n", device->machine().describe_context(),state));
	mstate->m_triac_ic5 = state;
}

static const pia6821_interface pia_ic5_intf =
{
	DEVCB_NULL,		/* port A in */
	DEVCB_HANDLER(pia_ic5_portb_r),	/* port B in */
	DEVCB_NULL,		/* line CA1 in */
	DEVCB_NULL,		/* line CB1 in */
	DEVCB_NULL,		/* line CA2 in */
	DEVCB_NULL,		/* line CB2 in */
	DEVCB_HANDLER(pia_ic5_porta_w),	/* port A out */
	DEVCB_HANDLER(pia_ic5_portb_w),	/* port B out */
	DEVCB_LINE(pia_ic5_ca2_w),		/* line CA2 out */
	DEVCB_LINE(pia_ic5_cb2_w),		/* port CB2 out */
	DEVCB_NULL,			/* IRQA */
	DEVCB_NULL			/* IRQB */
};


/* IC6, AUX ports*/
static READ8_DEVICE_HANDLER( pia_ic6_porta_r )
{
	mpu3_state *state = device->machine().driver_data<mpu3_state>();
	return (input_port_read(device->machine(), "AUX1"))|state->m_aux1_input;
}


static READ8_DEVICE_HANDLER( pia_ic6_portb_r )
{
	mpu3_state *state = device->machine().driver_data<mpu3_state>();
	return (input_port_read(device->machine(), "AUX2"))|state->m_aux2_input;
}

static WRITE8_DEVICE_HANDLER( pia_ic6_porta_w )
{
	mpu3_state *state = device->machine().driver_data<mpu3_state>();
	LOG(("%s: IC6 PIA Port A Set to %2x (Alpha)\n", device->machine().describe_context(),data));
	if ( data & 0x08 ) ROC10937_reset(0);

	state->m_alpha_data_line = ((data & 0x20) >> 5);
	if ( !state->m_alpha_clock && (data & 0x10) )
	{
		ROC10937_shift_data(0, state->m_alpha_data_line&0x01?0:1);
	}
	state->m_alpha_clock = (data & 0x10);

	ROC10937_draw_16seg(0);
}

static WRITE8_DEVICE_HANDLER( pia_ic6_portb_w )
{
	mpu3_state *state = device->machine().driver_data<mpu3_state>();
	LOG(("%s: IC6 PIA Port B Set to %2x (AUX2)\n", device->machine().describe_context(),data));
	state->m_aux2_input = data;
}

static const pia6821_interface pia_ic6_intf =
{
	DEVCB_HANDLER(pia_ic6_porta_r),		/* port A in */
	DEVCB_HANDLER(pia_ic6_portb_r),		/* port B in */
	DEVCB_NULL,		/* line CA1 in */
	DEVCB_NULL,		/* line CB1 in */
	DEVCB_NULL,		/* line CA2 in */
	DEVCB_NULL,		/* line CB2 in */
	DEVCB_HANDLER(pia_ic6_porta_w),		/* port A out */
	DEVCB_HANDLER(pia_ic6_portb_w),		/* port B out */
	DEVCB_NULL,			/* line CA2 out */
	DEVCB_NULL,			/* port CB2 out */
	DEVCB_LINE(cpu0_irq),				/* IRQA */
	DEVCB_LINE(cpu0_irq)				/* IRQB */
};

static INPUT_PORTS_START( mpu3 )
	PORT_START("ORANGE1")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_OTHER) PORT_NAME("Cancel")
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_OTHER) PORT_NAME("01")
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_OTHER) PORT_NAME("02")
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_OTHER) PORT_NAME("03")
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_OTHER) PORT_NAME("04")
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_OTHER) PORT_NAME("05")


	PORT_START("ORANGE2")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_OTHER) PORT_NAME("08")
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_OTHER) PORT_NAME("09")
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_OTHER) PORT_NAME("Exchange")
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_OTHER) PORT_NAME("11")
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_OTHER) PORT_NAME("12")
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_OTHER) PORT_NAME("13")

	PORT_START("BLACK1")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_BUTTON1) PORT_NAME("Hi")
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_BUTTON2) PORT_NAME("Lo")
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_OTHER)   PORT_NAME("Auto Nudge")
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_SERVICE) PORT_NAME("Test Button") PORT_CODE(KEYCODE_W)
	PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_SERVICE) PORT_NAME("Refill Key") PORT_CODE(KEYCODE_R) PORT_TOGGLE
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_INTERLOCK) PORT_NAME("Cashbox Door")  PORT_CODE(KEYCODE_Q) PORT_TOGGLE

	PORT_START("BLACK2")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_OTHER) PORT_NAME("20p")
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_OTHER) PORT_NAME("10p")
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_BUTTON3) PORT_NAME("Token")
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_BUTTON4) PORT_NAME("Hold 1")
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_BUTTON5) PORT_NAME("Hold 2")
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_BUTTON6) PORT_NAME("Hold 3")

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

static const stepper_interface mpu3_reel_interface =
{
	MPU3_48STEP_REEL,
	92,
	2,
	0x00
};

/* Common configurations */
static void mpu3_config_common(running_machine &machine)
{
	mpu3_state *state = machine.driver_data<mpu3_state>();
	state->m_ic21_timer = machine.scheduler().timer_alloc(FUNC(ic21_timeout));
}

static MACHINE_START( mpu3 )
{
	mpu3_config_common(machine);

	/* setup 8 mechanical meters */
	MechMtr_config(machine,8);

	/* setup 4 reels */
	stepper_config(machine, 0, &mpu3_reel_interface);
	stepper_config(machine, 1, &mpu3_reel_interface);
	stepper_config(machine, 2, &mpu3_reel_interface);
	stepper_config(machine, 3, &mpu3_reel_interface);

	/* setup the standard oki MSC1937 display */
	ROC10937_init(0, MSC1937,0);
}
/*
Characteriser (CHR)

The MPU3 characteriser is surprisingly simple to simulate, as it operates a call, response pairing. The only factor that provides a challenge is that the
address of the PAL in memory varies between games. Once found, the memory location of the data table is easily found from the X register of the CPU.
*/

static WRITE8_HANDLER( characteriser_w )
{
	mpu3_state *state = space->machine().driver_data<mpu3_state>();
	int x;
	int call=data;
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
						break;
					}
				}
			}
		}
	}
}


static READ8_HANDLER( characteriser_r )
{
	mpu3_state *state = space->machine().driver_data<mpu3_state>();
	if (!state->m_current_chr_table)
		fatalerror("No Characteriser Table @ %04x\n", cpu_get_previouspc(&space->device()));

	if (offset == 0)
	{
		return state->m_current_chr_table[state->m_prot_col].response;
	}
	return 0;
}

/* generate a 50 Hz signal (some components rely on this for external sync) */
static TIMER_DEVICE_CALLBACK( gen_50hz )
{
	mpu3_state *state = timer.machine().driver_data<mpu3_state>();
	/* Although reported as a '50Hz' signal, the fact that both rising and
    falling edges of the pulse are used means the timer actually gives a 100Hz
    oscillating signal.*/
	state->m_signal_50hz = state->m_signal_50hz?0:1;
	ptm6840_set_c1(timer.machine().device("ptm_ic2"), 0, state->m_signal_50hz);
	pia6821_cb1_w(timer.machine().device("pia_ic3"), ~state->m_signal_50hz);
	update_triacs(timer.machine());
}

static TIMER_DEVICE_CALLBACK( ic10_callback )
{
	mpu3_state *state = timer.machine().driver_data<mpu3_state>();
	// TODO: Use discrete handler for 555, this is far too simplistic

	state->m_ic10_output = state->m_ic10_output?0:1;
	ptm6840_set_c2(timer.machine().device("ptm_ic2"), 0, state->m_ic10_output);
	pia6821_ca1_w(timer.machine().device("pia_ic4"), state->m_ic10_output);

}
static WRITE8_HANDLER( mpu3ptm_w )
{
	device_t *ptm2 = space->machine().device("ptm_ic2");

	ptm6840_write(ptm2,offset >>2,data);//((offset & 0x1f) >>2),data);
}

static READ8_HANDLER( mpu3ptm_r )
{
	device_t *ptm2 = space->machine().device("ptm_ic2");

	return ptm6840_read(ptm2,offset >>2);
}

static ADDRESS_MAP_START( mpu3_basemap, AS_PROGRAM, 8 )
	AM_RANGE(0x0000, 0x07ff) AM_RAM AM_SHARE("nvram")
	AM_RANGE(0x8800, 0x881f) AM_READWRITE(mpu3ptm_r, mpu3ptm_w)/* PTM6840 IC2 */
	AM_RANGE(0x9000, 0x9003) AM_DEVREADWRITE("pia_ic3", pia6821_r, pia6821_w)		/* PIA6821 IC3 */
	AM_RANGE(0x9800, 0x9803) AM_DEVREADWRITE("pia_ic4", pia6821_r, pia6821_w)		/* PIA6821 IC4 */
	AM_RANGE(0xa000, 0xa003) AM_DEVREADWRITE("pia_ic5", pia6821_r, pia6821_w)		/* PIA6821 IC5 */
	AM_RANGE(0xa800, 0xa803) AM_DEVREADWRITE("pia_ic6", pia6821_r, pia6821_w)		/* PIA6821 IC6 */

	AM_RANGE(0x1000, 0xffff) AM_ROM
ADDRESS_MAP_END

static MACHINE_CONFIG_START( mpu3base, mpu3_state )
	MCFG_MACHINE_START(mpu3)
	MCFG_MACHINE_RESET(mpu3)
	MCFG_CPU_ADD("maincpu", M6808, MPU3_MASTER_CLOCK)///4)
	MCFG_CPU_PROGRAM_MAP(mpu3_basemap)

	MCFG_TIMER_ADD_PERIODIC("50hz",gen_50hz, attotime::from_hz(100))
	MCFG_TIMER_ADD_PERIODIC("555_ic10",ic10_callback, PERIOD_OF_555_ASTABLE(10000,1000,0.0000001))

	/* 6840 PTM */
	MCFG_PTM6840_ADD("ptm_ic2", ptm_ic2_intf)

	MCFG_PIA6821_ADD("pia_ic3", pia_ic3_intf)
	MCFG_PIA6821_ADD("pia_ic4", pia_ic4_intf)
	MCFG_PIA6821_ADD("pia_ic5", pia_ic5_intf)
	MCFG_PIA6821_ADD("pia_ic6", pia_ic6_intf)

	MCFG_NVRAM_ADD_0FILL("nvram")

	MCFG_DEFAULT_LAYOUT(layout_awpvid16)
MACHINE_CONFIG_END


static const mpu3_chr_table hprvpr_data[64] = {
{0x00, 0x00},{0x1a, 0x80},{0x04, 0xec},{0x10, 0x1c},{0x18, 0xf0},{0x0f, 0x28},{0x13, 0x54},{0x1b, 0x04},
{0x03, 0xa4},{0x07, 0xac},{0x17, 0x3c},{0x1d, 0xf4},{0x16, 0x48},{0x15, 0x54},{0x0b, 0x00},{0x08, 0xc4},
{0x19, 0x88},{0x01, 0x7c},{0x02, 0xf4},{0x05, 0x48},{0x0c, 0x54},{0x09, 0x04},{0x11, 0xc0},{0x14, 0xc8},
{0x0a, 0x7c},{0x1f, 0xf4},{0x06, 0x6c},{0x0e, 0x70},{0x1c, 0x00},{0x12, 0xa4},{0x1e, 0x8c},{0x0d, 0x5c},
{0x14, 0xf0},{0x0a, 0x28},{0x19, 0x30},{0x15, 0x40},{0x06, 0xe4},{0x0f, 0x88},{0x08, 0x5c},{0x1b, 0x94},
{0x1e, 0x0c},{0x04, 0x74},{0x01, 0x40},{0x0c, 0xc4},{0x18, 0x8c},{0x1a, 0x7c},{0x11, 0xd4},{0x0b, 0x08},
{0x03, 0x34},{0x17, 0x40},{0x10, 0xa4},{0x1d, 0xe8},{0x0e, 0x78},{0x07, 0x90},{0x12, 0x0c},{0x09, 0x54},
{0x0d, 0x04},{0x1f, 0xc0},{0x16, 0xc8},{0x05, 0x78},{0x13, 0xd4},{0x1c, 0x0c},{0x02, 0x74},{0x00, 0x00},
};

static DRIVER_INIT (m_hprvpr)
{
	mpu3_state *state = machine.driver_data<mpu3_state>();
	address_space *space = machine.device("maincpu")->memory().space(AS_PROGRAM);

	state->m_disp_func=METER_PORT;
	state->m_current_chr_table = hprvpr_data;
	space->install_legacy_readwrite_handler(0xc000, 0xc000 , FUNC(characteriser_r),FUNC(characteriser_w));

}

ROM_START( mpu3utst )
	ROM_REGION( 0x10000, "maincpu", ROMREGION_ERASE00  )
	ROM_LOAD( "ut5.bin",  0x7800, 0x0800,  CRC(89994adb) SHA1(03e38a860b6ba9d7ae2dbfa7845e85bcf67a6f4d))
	ROM_RELOAD(0xF800, 0x0800)
ROM_END

ROM_START( m_hprvpr )
	ROM_REGION( 0x10000, "maincpu", ROMREGION_ERASE00  )
	ROM_LOAD( "hv.p3",  0x2000, 0x2000,  CRC(63f8e04f) SHA1(9bfe06a6ea8c308e4a7035752722babbfd792160))
	ROM_RELOAD(0xc000, 0x2000)
	ROM_LOAD( "hv.p2",  0x4000, 0x2000,  CRC(b3b6f19e) SHA1(b2251dbe632b04ea7b5952b8a6217605b0df904c))
	ROM_RELOAD(0xc000, 0x2000)
	ROM_LOAD( "hv.p1",  0x6000, 0x2000,  CRC(05c0cf97) SHA1(085f432b608cec6054f1e03cf24ca80d3949a0de))
	ROM_RELOAD(0xe000, 0x2000)
ROM_END

//    year, name,    parent,  machine,  input,       init,    monitor, company,         fullname,                                    flags
//Diagnostic ROMs
GAME( 198?,  mpu3utst,  0,    mpu3base, mpu3,     0,        ROT0, "Barcrest",		"MPU3 Unit Test (Program 5)",										GAME_NO_SOUND )
GAME( 198?,  m_hprvpr,  0,    mpu3base, mpu3,     m_hprvpr,        ROT0, "Barcrest",		"Hyper Viper",										GAME_NO_SOUND|GAME_UNEMULATED_PROTECTION )
