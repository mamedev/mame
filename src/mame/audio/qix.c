/*************************************************************************

    audio\qix.c

*************************************************************************/

#include "driver.h"
#include "cpu/m6800/m6800.h"
#include "cpu/m6809/m6809.h"
#include "includes/qix.h"
#include "sound/sn76496.h"
#include "sound/discrete.h"

/* Discrete Sound Input Nodes */
#define QIX_DAC_DATA		NODE_01
#define QIX_VOL_DATA		NODE_02
#define QIX_VOL_DATA_L		NODE_03
#define QIX_VOL_DATA_R		NODE_04


static WRITE_LINE_DEVICE_HANDLER( qix_pia_dint );
static WRITE_LINE_DEVICE_HANDLER( qix_pia_sint );


/***************************************************************************
Audio handlers
***************************************************************************/

static WRITE8_DEVICE_HANDLER( qix_dac_w )
{
	discrete_sound_w(device, QIX_DAC_DATA, data);
}

static WRITE8_DEVICE_HANDLER( qix_vol_w )
{
	discrete_sound_w(device, QIX_VOL_DATA, data);
}


/************************************************************************/
/* qix Sound System Analog emulation                                    */
/************************************************************************/
/*
 * This hardware is capable of independant L/R volume control,
 * but only sdungeon uses it for a stereo effect.
 * Other games just use it for fixed L/R volume control.
 *
 * This is such a basic sound system that there is only one effect.
 * So I won't bother keeping proper voltages levels, and will just
 * start with the final gain.
 */

static const discrete_comp_adder_table qix_attn_table =
{
	DISC_COMP_P_RESISTOR, 0, 4,
	{RES_K(22)+250, RES_K(10)+250, RES_K(5.6)+250, RES_K(3.3)+250}
};

static DISCRETE_SOUND_START(qix)
	/*                    NODE                      */
	DISCRETE_INPUTX_DATA(QIX_DAC_DATA, 128, -128*128, 128)
	DISCRETE_INPUT_DATA (QIX_VOL_DATA)

	/* Seperate the two 4-bit channels. */
	DISCRETE_TRANSFORM3(QIX_VOL_DATA_L, QIX_VOL_DATA, 16, 0x0f, "01/2&")
	DISCRETE_TRANSFORM2(QIX_VOL_DATA_R, QIX_VOL_DATA, 0x0f, "01&")

	/* Work out the parallel resistance of the selected resistors. */
	DISCRETE_COMP_ADDER(NODE_10, QIX_VOL_DATA_L, &qix_attn_table)
	DISCRETE_COMP_ADDER(NODE_20, QIX_VOL_DATA_R, &qix_attn_table)

	/* Then use it for the resistor divider network. */
	DISCRETE_TRANSFORM3(NODE_11, NODE_10, RES_K(10), QIX_DAC_DATA, "001+/2*")
	DISCRETE_TRANSFORM3(NODE_21, NODE_20, RES_K(10), QIX_DAC_DATA, "001+/2*")

	/* If no resistors are selected (0), then the volume is full. */
	DISCRETE_SWITCH(NODE_12, 1, QIX_VOL_DATA_L, QIX_DAC_DATA, NODE_11)
	DISCRETE_SWITCH(NODE_22, 1, QIX_VOL_DATA_R, QIX_DAC_DATA, NODE_21)

	/* Filter the DC using the lowest case filter. */
	DISCRETE_CRFILTER(NODE_13, NODE_12, RES_K(1.5), CAP_U(1))
	DISCRETE_CRFILTER(NODE_23, NODE_22, RES_K(1.5), CAP_U(1))

	DISCRETE_OUTPUT(NODE_13, 1)
	DISCRETE_OUTPUT(NODE_23, 1)
DISCRETE_SOUND_END



/*************************************
 *
 *  PIA handlers
 *
 *************************************/

static WRITE8_DEVICE_HANDLER( sndpia_2_warning_w )
{
	popmessage("PIA 5 write!!");
}


static TIMER_CALLBACK( deferred_sndpia1_porta_w )
{
	const device_config *device = (const device_config *)ptr;
	pia6821_porta_w(device, 0, param);
}


static WRITE8_DEVICE_HANDLER( sync_sndpia1_porta_w )
{
	/* we need to synchronize this so the sound CPU doesn't drop anything important */
	timer_call_after_resynch(device->machine, (void *)device, data, deferred_sndpia1_porta_w);
}


static WRITE8_DEVICE_HANDLER( slither_coinctl_w )
{
	coin_lockout_w(device->machine, 0, (~data >> 6) & 1);
	coin_counter_w(device->machine, 0, (data >> 5) & 1);
}



/*************************************
 *
 *  IRQ generation
 *
 *************************************/

static WRITE_LINE_DEVICE_HANDLER( qix_pia_dint )
{
	int combined_state = pia6821_get_irq_a(device) | pia6821_get_irq_b(device);

	/* DINT is connected to the data CPU's IRQ line */
	cputag_set_input_line(device->machine, "maincpu", M6809_IRQ_LINE, combined_state ? ASSERT_LINE : CLEAR_LINE);
}


static WRITE_LINE_DEVICE_HANDLER( qix_pia_sint )
{
	int combined_state = pia6821_get_irq_a(device) | pia6821_get_irq_b(device);

	/* SINT is connected to the sound CPU's IRQ line */
	cputag_set_input_line(device->machine, "audiocpu", M6800_IRQ_LINE, combined_state ? ASSERT_LINE : CLEAR_LINE);
}



/*************************************
 *
 *  Audio CPU memory handlers
 *
 *************************************/

static ADDRESS_MAP_START( audio_map, ADDRESS_SPACE_PROGRAM, 8 )
	AM_RANGE(0x0000, 0x007f) AM_RAM
	AM_RANGE(0x2000, 0x2003) AM_MIRROR(0x5ffc) AM_DEVREADWRITE("sndpia2", pia6821_r, pia6821_w)
	AM_RANGE(0x4000, 0x4003) AM_MIRROR(0x3ffc) AM_DEVREADWRITE("sndpia1", pia6821_r, pia6821_w)
	AM_RANGE(0xd000, 0xffff) AM_ROM
ADDRESS_MAP_END



/*************************************
 *
 *  PIA interfaces
 *
 *************************************/

static const pia6821_interface qixsnd_pia_0_intf =
{
	DEVCB_NULL,		/* port A in */
	DEVCB_NULL,		/* port B in */
	DEVCB_NULL,		/* line CA1 in */
	DEVCB_NULL,		/* line CB1 in */
	DEVCB_NULL,		/* line CA2 in */
	DEVCB_NULL,		/* line CB2 in */
	DEVCB_DEVICE_HANDLER("sndpia1", sync_sndpia1_porta_w),			/* port A out */
	DEVCB_DEVICE_HANDLER("discrete", qix_vol_w),					/* port B out */
	DEVCB_DEVICE_HANDLER("sndpia1", pia6821_ca1_w),						/* line CA2 out */
	DEVCB_HANDLER(qix_flip_screen_w),								/* port CB2 out */
	DEVCB_LINE(qix_pia_dint),										/* IRQA */
	DEVCB_LINE(qix_pia_dint)										/* IRQB */
};

static const pia6821_interface qixsnd_pia_1_intf =
{
	DEVCB_NULL,		/* port A in */
	DEVCB_NULL,		/* port B in */
	DEVCB_NULL,		/* line CA1 in */
	DEVCB_NULL,		/* line CB1 in */
	DEVCB_NULL,		/* line CA2 in */
	DEVCB_NULL,		/* line CB2 in */
	DEVCB_DEVICE_HANDLER("sndpia0", pia6821_porta_w),			/* port A out */
	DEVCB_DEVICE_HANDLER("discrete", qix_dac_w),			/* port B out */
	DEVCB_DEVICE_HANDLER("sndpia0", pia6821_ca1_w),				/* line CA2 out */
	DEVCB_NULL,		/* line CB2 out */
	DEVCB_LINE(qix_pia_sint),								/* IRQA */
	DEVCB_LINE(qix_pia_sint)								/* IRQB */
};

static const pia6821_interface qixsnd_pia_2_intf =
{
	DEVCB_NULL,		/* port A in */
	DEVCB_NULL,		/* port B in */
	DEVCB_NULL,		/* line CA1 in */
	DEVCB_NULL,		/* line CB1 in */
	DEVCB_NULL,		/* line CA2 in */
	DEVCB_NULL,		/* line CB2 in */
	DEVCB_HANDLER(sndpia_2_warning_w),	/* port A out */
	DEVCB_HANDLER(sndpia_2_warning_w),	/* port B out */
	DEVCB_HANDLER(sndpia_2_warning_w),	/* line CA2 out */
	DEVCB_HANDLER(sndpia_2_warning_w),	/* line CB2 out */
	DEVCB_NULL,
	DEVCB_NULL
};

static const pia6821_interface slithersnd_pia_0_intf =
{
	DEVCB_INPUT_PORT("P2"),		/* port A in */
	DEVCB_NULL,		/* port B in */
	DEVCB_NULL,		/* line CA1 in */
	DEVCB_NULL,		/* line CB1 in */
	DEVCB_NULL,		/* line CA2 in */
	DEVCB_NULL,		/* line CB2 in */
	DEVCB_NULL,		/* port A out */
	DEVCB_HANDLER(slither_coinctl_w),	/* port B out */
	DEVCB_NULL,		/* line CA2 out */
	DEVCB_HANDLER(qix_flip_screen_w),	/* port CB2 out */
	DEVCB_LINE(qix_pia_dint),			/* IRQA */
	DEVCB_LINE(qix_pia_dint)			/* IRQB */
};



/*************************************
 *
 *  Machine drivers
 *
 *************************************/

MACHINE_DRIVER_START( qix_audio )
	MDRV_CPU_ADD("audiocpu", M6802, SOUND_CLOCK_OSC/2)		/* 0.92 MHz */
	MDRV_CPU_PROGRAM_MAP(audio_map)

	MDRV_PIA6821_ADD("sndpia0", qixsnd_pia_0_intf)
	MDRV_PIA6821_ADD("sndpia1", qixsnd_pia_1_intf)
	MDRV_PIA6821_ADD("sndpia2", qixsnd_pia_2_intf)

	MDRV_SPEAKER_STANDARD_STEREO("lspeaker", "rspeaker")

	MDRV_SOUND_ADD("discrete", DISCRETE, 0)
	MDRV_SOUND_CONFIG_DISCRETE(qix)
	MDRV_SOUND_ROUTE(0, "lspeaker", 1.0)
	MDRV_SOUND_ROUTE(1, "rspeaker", 1.0)
MACHINE_DRIVER_END


MACHINE_DRIVER_START( slither_audio )
	MDRV_PIA6821_ADD("sndpia0", slithersnd_pia_0_intf)

	MDRV_SPEAKER_STANDARD_MONO("mono")

	MDRV_SOUND_ADD("sn1", SN76489, SLITHER_CLOCK_OSC/4/4)
	MDRV_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.50)

	MDRV_SOUND_ADD("sn2", SN76489, SLITHER_CLOCK_OSC/4/4)
	MDRV_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.50)
MACHINE_DRIVER_END
