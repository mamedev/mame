/***************************************************************************

    Taito Qix hardware

    driver by John Butler, Ed Mueller, Aaron Giles

***************************************************************************/

#include "driver.h"
#include "deprecat.h"
#include "qix.h"
#include "machine/6821pia.h"
#include "cpu/m6800/m6800.h"
#include "cpu/m6805/m6805.h"
#include "cpu/m6809/m6809.h"
#include "sound/sn76496.h"


/* Globals */
UINT8 *qix_68705_port_out;
UINT8 *qix_68705_ddr;


/* Local variables */
static UINT8 qix_68705_port_in[3];
static UINT8 qix_coinctrl;


/* Prototypes */
static READ8_HANDLER( qixmcu_coin_r );
static WRITE8_HANDLER( qixmcu_coinctrl_w );
static WRITE8_HANDLER( qixmcu_coin_w );

static WRITE8_HANDLER( sync_pia_4_porta_w );

static WRITE8_HANDLER( pia_5_warning_w );

static WRITE8_HANDLER( qix_inv_flag_w );

static WRITE8_HANDLER( qix_coinctl_w );
static WRITE8_HANDLER( slither_coinctl_w );

static void qix_pia_sint(int state);
static void qix_pia_dint(int state);

static WRITE8_HANDLER( slither_76489_0_w );
static WRITE8_HANDLER( slither_76489_1_w );

static READ8_HANDLER( slither_trak_lr_r );
static READ8_HANDLER( slither_trak_ud_r );



/***************************************************************************

    Qix has 6 PIAs on board:

    From the ROM I/O schematic:

    PIA 0 = U11: (mapped to $9400 on the data CPU)
        port A = external input (input_port_0)
        port B = external input (input_port_1) (coin)

    PIA 1 = U20: (mapped to $9800/$9900 on the data CPU)
        port A = external input (input_port_2)
        port B = external input (input_port_3)

    PIA 2 = U30: (mapped to $9c00 on the data CPU)
        port A = external input (input_port_4)
        port B = external output (coin control)


    From the data/sound processor schematic:

    PIA 3 = U20: (mapped to $9000 on the data CPU)
        port A = data CPU to sound CPU communication
        port B = stereo volume control, 2 4-bit values
        CA1 = interrupt signal from sound CPU
        CA2 = interrupt signal to sound CPU
        CB1 = VS input signal (vertical sync)
        CB2 = INV output signal (cocktail flip)
        IRQA = /DINT1 signal
        IRQB = /DINT1 signal

    PIA 4 = U8: (mapped to $4000 on the sound CPU)
        port A = sound CPU to data CPU communication
        port B = DAC value (port B)
        CA1 = interrupt signal from data CPU
        CA2 = interrupt signal to data CPU
        IRQA = /SINT1 signal
        IRQB = /SINT1 signal

    PIA 5 = U7: (never actually used, mapped to $2000 on the sound CPU)
        port A = unused
        port B = sound CPU to TMS5220 communication
        CA1 = interrupt signal from TMS5220
        CA2 = write signal to TMS5220
        CB1 = ready signal from TMS5220
        CB2 = read signal to TMS5220
        IRQA = /SINT2 signal
        IRQB = /SINT2 signal

***************************************************************************/

static const pia6821_interface qix_pia_0_intf =
{
	/*inputs : A/B,CA/B1,CA/B2 */ input_port_0_r, input_port_1_r, 0, 0, 0, 0,
	/*outputs: A/B,CA/B2       */ 0, 0, 0, 0,
	/*irqs   : A/B             */ 0, 0
};

static const pia6821_interface qix_pia_1_intf =
{
	/*inputs : A/B,CA/B1,CA/B2 */ input_port_2_r, input_port_3_r, 0, 0, 0, 0,
	/*outputs: A/B,CA/B2       */ 0, 0, 0, 0,
	/*irqs   : A/B             */ 0, 0
};

static const pia6821_interface qix_pia_2_intf =
{
	/*inputs : A/B,CA/B1,CA/B2 */ input_port_4_r, 0, 0, 0, 0, 0,
	/*outputs: A/B,CA/B2       */ 0, qix_coinctl_w, 0, 0,
	/*irqs   : A/B             */ 0, 0
};

static const pia6821_interface qix_pia_3_intf =
{
	/*inputs : A/B,CA/B1,CA/B2 */ 0, 0, 0, 0, 0, 0,
	/*outputs: A/B,CA/B2       */ sync_pia_4_porta_w, qix_vol_w, pia_4_ca1_w, qix_inv_flag_w,
	/*irqs   : A/B             */ qix_pia_dint, qix_pia_dint
};

static const pia6821_interface qix_pia_4_intf =
{
	/*inputs : A/B,CA/B1,CA/B2 */ 0, 0, 0, 0, 0, 0,
	/*outputs: A/B,CA/B2       */ pia_3_porta_w, qix_dac_w, pia_3_ca1_w, 0,
	/*irqs   : A/B             */ qix_pia_sint, qix_pia_sint
};

static const pia6821_interface qix_pia_5_intf =
{
	/*inputs : A/B,CA/B1,CA/B2 */ 0, 0, 0, 0, 0, 0,
	/*outputs: A/B,CA/B2       */ pia_5_warning_w, pia_5_warning_w, pia_5_warning_w, pia_5_warning_w,
	/*irqs   : A/B             */ 0, 0
};


static WRITE8_HANDLER( pia_5_warning_w )
{
	popmessage("PIA 5 write!!");
}


/***************************************************************************

    Games with an MCU need to handle coins differently, and provide
    communication with the MCU

***************************************************************************/

static const pia6821_interface qixmcu_pia_0_intf =
{
	/*inputs : A/B,CA/B1,CA/B2 */ input_port_0_r, qixmcu_coin_r, 0, 0, 0, 0,
	/*outputs: A/B,CA/B2       */ 0, qixmcu_coin_w, 0, 0,
	/*irqs   : A/B             */ 0, 0
};

static const pia6821_interface qixmcu_pia_2_intf =
{
	/*inputs : A/B,CA/B1,CA/B2 */ input_port_4_r, 0, 0, 0, 0, 0,
	/*outputs: A/B,CA/B2       */ 0, qixmcu_coinctrl_w, 0, 0,
	/*irqs   : A/B             */ 0, 0
};



/***************************************************************************

    Slither uses 2 SN76489's for sound instead of the 6802+DAC; these
    are accessed via the PIAs.

***************************************************************************/

static const pia6821_interface slither_pia_1_intf =
{
	/*inputs : A/B,CA/B1,CA/B2 */ slither_trak_lr_r, 0, 0, 0, 0, 0,
	/*outputs: A/B,CA/B2       */ 0, slither_76489_0_w, 0, 0,
	/*irqs   : A/B             */ 0, 0
};

static const pia6821_interface slither_pia_2_intf =
{
	/*inputs : A/B,CA/B1,CA/B2 */ slither_trak_ud_r, 0, 0, 0, 0, 0,
	/*outputs: A/B,CA/B2       */ 0, slither_76489_1_w, 0, 0,
	/*irqs   : A/B             */ 0, 0
};

static const pia6821_interface slither_pia_3_intf =
{
	/*inputs : A/B,CA/B1,CA/B2 */ input_port_2_r, 0, 0, 0, 0, 0,
	/*outputs: A/B,CA/B2       */ 0, slither_coinctl_w, 0, qix_inv_flag_w,
	/*irqs   : A/B             */ qix_pia_dint, qix_pia_dint
};



/*************************************
 *
 *  Machine initialization
 *
 *************************************/

MACHINE_START( qix )
{
	/* configure the PIAs */
	pia_config(0, &qix_pia_0_intf);
	pia_config(1, &qix_pia_1_intf);
	pia_config(2, &qix_pia_2_intf);
	pia_config(3, &qix_pia_3_intf);
	pia_config(4, &qix_pia_4_intf);
	pia_config(5, &qix_pia_5_intf);
}

MACHINE_RESET( qix )
{
	/* reset the PIAs */
	pia_reset();

	/* reset the coin counter register */
	qix_coinctrl = 0x00;
}


MACHINE_START( qixmcu )
{
	/* configure the PIAs */
	pia_config(0, &qixmcu_pia_0_intf);
	pia_config(1, &qix_pia_1_intf);
	pia_config(2, &qixmcu_pia_2_intf);
	pia_config(3, &qix_pia_3_intf);
	pia_config(4, &qix_pia_4_intf);
	pia_config(5, &qix_pia_5_intf);

	/* set up save states */
	state_save_register_global_array(qix_68705_port_in);
	state_save_register_global(qix_coinctrl);
}

MACHINE_START( slither )
{
	/* configure the PIAs */
	pia_config(0, &qix_pia_0_intf);
	pia_config(1, &slither_pia_1_intf);
	pia_config(2, &slither_pia_2_intf);
	pia_config(3, &slither_pia_3_intf);
}



/*************************************
 *
 *  VSYNC interrupt handling
 *
 *************************************/

static TIMER_CALLBACK( vblank_stop )
{
	pia_3_cb1_w(0, 0);
}


INTERRUPT_GEN( qix_vblank_start )
{
	pia_3_cb1_w(0, 1);
	timer_set(video_screen_get_time_until_pos(0, 0, 0), NULL, 0, vblank_stop);
}



/*************************************
 *
 *  Zoo Keeper bankswitching
 *
 *************************************/

WRITE8_HANDLER( zoo_bankswitch_w )
{
	memory_set_bank(1, (data >> 2) & 1);
	/* not necessary, but technically correct */
	qix_palettebank_w(offset,data);
}



/*************************************
 *
 *  Data CPU FIRQ generation/ack
 *
 *************************************/

WRITE8_HANDLER( qix_data_firq_w )
{
	cpunum_set_input_line(Machine, 0, M6809_FIRQ_LINE, ASSERT_LINE);
}


WRITE8_HANDLER( qix_data_firq_ack_w )
{
	cpunum_set_input_line(Machine, 0, M6809_FIRQ_LINE, CLEAR_LINE);
}


READ8_HANDLER( qix_data_firq_r )
{
	cpunum_set_input_line(Machine, 0, M6809_FIRQ_LINE, ASSERT_LINE);
	return 0xff;
}


READ8_HANDLER( qix_data_firq_ack_r )
{
	cpunum_set_input_line(Machine, 0, M6809_FIRQ_LINE, CLEAR_LINE);
	return 0xff;
}



/*************************************
 *
 *  Video CPU FIRQ generation/ack
 *
 *************************************/

WRITE8_HANDLER( qix_video_firq_w )
{
	cpunum_set_input_line(Machine, 1, M6809_FIRQ_LINE, ASSERT_LINE);
}


WRITE8_HANDLER( qix_video_firq_ack_w )
{
	cpunum_set_input_line(Machine, 1, M6809_FIRQ_LINE, CLEAR_LINE);
}


READ8_HANDLER( qix_video_firq_r )
{
	cpunum_set_input_line(Machine, 1, M6809_FIRQ_LINE, ASSERT_LINE);
	return 0xff;
}


READ8_HANDLER( qix_video_firq_ack_r )
{
	cpunum_set_input_line(Machine, 1, M6809_FIRQ_LINE, CLEAR_LINE);
	return 0xff;
}



/*************************************
 *
 *  Sound PIA interfaces
 *
 *************************************/

static TIMER_CALLBACK( deferred_pia_4_porta_w )
{
	pia_4_porta_w(0, param);
}


static WRITE8_HANDLER( sync_pia_4_porta_w )
{
	/* we need to synchronize this so the sound CPU doesn't drop anything important */
	timer_call_after_resynch(NULL, data, deferred_pia_4_porta_w);
}



/*************************************
 *
 *  IRQ generation
 *
 *************************************/

static void qix_pia_dint(int state)
{
	int combined_state = pia_get_irq_a(3) | pia_get_irq_b(3);

	/* DINT is connected to the data CPU's IRQ line */
	cpunum_set_input_line(Machine, 0, M6809_IRQ_LINE, combined_state ? ASSERT_LINE : CLEAR_LINE);
}


static void qix_pia_sint(int state)
{
	int combined_state = pia_get_irq_a(4) | pia_get_irq_b(4);

	/* SINT is connected to the sound CPU's IRQ line */
	cpunum_set_input_line(Machine, 2, M6802_IRQ_LINE, combined_state ? ASSERT_LINE : CLEAR_LINE);
}



/*************************************
 *
 *  68705 Communication
 *
 *************************************/

READ8_HANDLER( qixmcu_coin_r )
{
	logerror("6809:qixmcu_coin_r = %02X\n", qix_68705_port_out[0]);
	return qix_68705_port_out[0];
}


static WRITE8_HANDLER( qixmcu_coin_w )
{
	logerror("6809:qixmcu_coin_w = %02X\n", data);
	/* this is a callback called by pia_0_w(), so I don't need to synchronize */
	/* the CPUs - they have already been synchronized by qix_pia_0_w() */
	qix_68705_port_in[0] = data;
}


static WRITE8_HANDLER( qixmcu_coinctrl_w )
{
	if (!(data & 0x04))
	{
		cpunum_set_input_line(Machine, 3, M68705_IRQ_LINE, ASSERT_LINE);
		/* temporarily boost the interleave to sync things up */
		/* note: I'm using 50 because 30 is not enough for space dungeon at game over */
		cpu_boost_interleave(attotime_zero, ATTOTIME_IN_USEC(50));
	}
	else
		cpunum_set_input_line(Machine, 3, M68705_IRQ_LINE, CLEAR_LINE);

	/* this is a callback called by pia_0_w(), so I don't need to synchronize */
	/* the CPUs - they have already been synchronized by qix_pia_0_w() */
	qix_coinctrl = data;
	logerror("6809:qixmcu_coinctrl_w = %02X\n", data);
}



/*************************************
 *
 *  68705 Port Inputs
 *
 *************************************/

READ8_HANDLER( qix_68705_portA_r )
{
	UINT8 ddr = qix_68705_ddr[0];
	UINT8 out = qix_68705_port_out[0];
	UINT8 in = qix_68705_port_in[0];
	logerror("68705:portA_r = %02X (%02X)\n", (out & ddr) | (in & ~ddr), in);
	return (out & ddr) | (in & ~ddr);
}


READ8_HANDLER( qix_68705_portB_r )
{
	UINT8 ddr = qix_68705_ddr[1];
	UINT8 out = qix_68705_port_out[1];
	UINT8 in = (readinputport(1) & 0x0f) | ((readinputport(1) & 0x80) >> 3);
	return (out & ddr) | (in & ~ddr);
}


READ8_HANDLER( qix_68705_portC_r )
{
	UINT8 ddr = qix_68705_ddr[2];
	UINT8 out = qix_68705_port_out[2];
	UINT8 in = (qix_coinctrl & 0x08) | ((readinputport(1) & 0x70) >> 4);
	return (out & ddr) | (in & ~ddr);
}



/*************************************
 *
 *  68705 Port Outputs
 *
 *************************************/

WRITE8_HANDLER( qix_68705_portA_w )
{
	logerror("68705:portA_w = %02X\n", data);
	qix_68705_port_out[0] = data;
}


WRITE8_HANDLER( qix_68705_portB_w )
{
	qix_68705_port_out[1] = data;
	coin_lockout_w(0, (~data >> 6) & 1);
	coin_counter_w(0, (data >> 7) & 1);
}


WRITE8_HANDLER( qix_68705_portC_w )
{
	qix_68705_port_out[2] = data;
}



/*************************************
 *
 *  Data CPU PIA 0 synchronization
 *
 *************************************/

static TIMER_CALLBACK( pia_0_w_callback )
{
	pia_0_w(param >> 8, param & 0xff);
}


WRITE8_HANDLER( qix_pia_0_w )
{
	/* make all the CPUs synchronize, and only AFTER that write the command to the PIA */
	/* otherwise the 68705 will miss commands */
	timer_call_after_resynch(NULL, data | (offset << 8), pia_0_w_callback);
}



/*************************************
 *
 *  Cocktail flip
 *
 *************************************/

static WRITE8_HANDLER( qix_inv_flag_w )
{
	qix_cocktail_flip = data;
}



/*************************************
 *
 *  Coin I/O for games without coin CPU
 *
 *************************************/

static WRITE8_HANDLER( qix_coinctl_w )
{
	coin_lockout_w(0, (~data >> 2) & 1);
	coin_counter_w(0, (data >> 1) & 1);
}


static WRITE8_HANDLER( slither_coinctl_w )
{
	coin_lockout_w(0, (~data >> 6) & 1);
	coin_counter_w(0, (data >> 5) & 1);
}



/*************************************
 *
 *  Slither SN76489 I/O
 *
 *************************************/

static WRITE8_HANDLER( slither_76489_0_w )
{
	/* write to the sound chip */
	SN76496_0_w(0, data);

	/* clock the ready line going back into CB1 */
	pia_1_cb1_w(0, 0);
	pia_1_cb1_w(0, 1);
}


static WRITE8_HANDLER( slither_76489_1_w )
{
	/* write to the sound chip */
	SN76496_1_w(0, data);

	/* clock the ready line going back into CB1 */
	pia_2_cb1_w(0, 0);
	pia_2_cb1_w(0, 1);
}



/*************************************
 *
 *  Slither trackball I/O
 *
 *************************************/

static READ8_HANDLER( slither_trak_lr_r )
{
	return readinputport(qix_cocktail_flip ? 6 : 4);
}


static READ8_HANDLER( slither_trak_ud_r )
{
	return readinputport(qix_cocktail_flip ? 5 : 3);
}
