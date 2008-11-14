/*************************************************************************

    audio\qix.c

*************************************************************************/
#include "driver.h"
#include "qix.h"
#include "machine/6821pia.h"
#include "sound/sn76496.h"
#include "sound/discrete.h"

/* Discrete Sound Input Nodes */
#define QIX_DAC_DATA		NODE_01
#define QIX_VOL_DATA		NODE_02
#define QIX_VOL_DATA_L		NODE_03
#define QIX_VOL_DATA_R		NODE_04


/***************************************************************************
Audio handlers
***************************************************************************/

WRITE8_HANDLER( qix_dac_w )
{
	discrete_sound_w(space, QIX_DAC_DATA, data);
}

WRITE8_HANDLER( qix_vol_w )
{
	discrete_sound_w(space, QIX_VOL_DATA, data);
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
	DISCRETE_CRFILTER(NODE_13, 1, NODE_12, RES_K(1.5), CAP_U(1))
	DISCRETE_CRFILTER(NODE_23, 1, NODE_22, RES_K(1.5), CAP_U(1))

	DISCRETE_OUTPUT(NODE_13, 1)
	DISCRETE_OUTPUT(NODE_23, 1)
DISCRETE_SOUND_END



/*************************************
 *
 *  Audio CPU memory handlers
 *
 *************************************/

static ADDRESS_MAP_START( audio_map, ADDRESS_SPACE_PROGRAM, 8 )
	AM_RANGE(0x0000, 0x007f) AM_RAM
	AM_RANGE(0x2000, 0x2003) AM_MIRROR(0x5ffc) AM_READWRITE(pia_5_r, pia_5_w)
	AM_RANGE(0x4000, 0x4003) AM_MIRROR(0x3ffc) AM_READWRITE(pia_4_r, pia_4_w)
	AM_RANGE(0xd000, 0xffff) AM_ROM
ADDRESS_MAP_END



/*************************************
 *
 *  Machine drivers
 *
 *************************************/

MACHINE_DRIVER_START( qix_audio )
	MDRV_CPU_ADD("audio", M6802, SOUND_CLOCK_OSC/2)		/* 0.92 MHz */
	MDRV_CPU_PROGRAM_MAP(audio_map,0)

	MDRV_SPEAKER_STANDARD_STEREO("left", "right")

	MDRV_SOUND_ADD("discrete", DISCRETE, 0)
	MDRV_SOUND_CONFIG_DISCRETE(qix)
	MDRV_SOUND_ROUTE(0, "left", 1.0)
	MDRV_SOUND_ROUTE(1, "right", 1.0)
MACHINE_DRIVER_END


MACHINE_DRIVER_START( slither_audio )
	MDRV_CPU_REMOVE("audio")

	MDRV_SPEAKER_REMOVE("left")
	MDRV_SPEAKER_REMOVE("right")
	MDRV_SPEAKER_STANDARD_MONO("mono")

	MDRV_SOUND_REPLACE("discrete", SN76489, SLITHER_CLOCK_OSC/4/4)
	MDRV_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.50)

	MDRV_SOUND_ADD("sn", SN76489, SLITHER_CLOCK_OSC/4/4)
	MDRV_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.50)
MACHINE_DRIVER_END
