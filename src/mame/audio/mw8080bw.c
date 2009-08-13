/***************************************************************************

    Midway 8080-based black and white hardware

****************************************************************************/

#include "driver.h"
#include "mw8080bw.h"
#include "sound/samples.h"
#include "sound/sn76477.h"
#include "sound/discrete.h"



/*************************************
 *
 *  Globals
 *
 *************************************/

static UINT8 port_1_last;
static UINT8 port_2_last;



/*************************************
 *
 *  Audio setup
 *
 *************************************/

static SOUND_START( samples )
{
	/* setup for save states */
	state_save_register_global(machine, port_1_last);
	state_save_register_global(machine, port_2_last);
}



/*************************************
 *
 *  Implementation of tone generator used
 *  by a few of these games
 *
 *************************************/

#define MIDWAY_TONE_EN				NODE_100
#define MIDWAY_TONE_DATA_L			NODE_101
#define MIDWAY_TONE_DATA_H			NODE_102
#define MIDWAY_TONE_SND				NODE_103
#define MIDWAY_TONE_TRASFORM_OUT	NODE_104
#define MIDWAY_TONE_BEFORE_AMP_SND	NODE_105


#define MIDWAY_TONE_GENERATOR(discrete_op_amp_tvca_info) \
		/* bit 0 of tone data is always 0 */ \
		/* join the L & H tone bits */ \
		DISCRETE_INPUT_LOGIC(MIDWAY_TONE_EN) \
		DISCRETE_INPUT_DATA (MIDWAY_TONE_DATA_L) \
		DISCRETE_INPUT_DATA (MIDWAY_TONE_DATA_H) \
		DISCRETE_TRANSFORM4(MIDWAY_TONE_TRASFORM_OUT, MIDWAY_TONE_DATA_H, 0x40, MIDWAY_TONE_DATA_L, 0x02, "01*23*+") \
		DISCRETE_NOTE(MIDWAY_TONE_BEFORE_AMP_SND, 1, (double)MW8080BW_MASTER_CLOCK/10/2, MIDWAY_TONE_TRASFORM_OUT, 0xfff, 1, DISC_CLK_IS_FREQ) \
		DISCRETE_OP_AMP_TRIG_VCA(MIDWAY_TONE_SND, MIDWAY_TONE_BEFORE_AMP_SND, MIDWAY_TONE_EN, 0, 12, 0, &discrete_op_amp_tvca_info)


WRITE8_DEVICE_HANDLER( midway_tone_generator_lo_w )
{
	discrete_sound_w(device, MIDWAY_TONE_EN, (data >> 0) & 0x01);

	discrete_sound_w(device, MIDWAY_TONE_DATA_L, (data >> 1) & 0x1f);

	/* D6 and D7 are not connected */
}


WRITE8_DEVICE_HANDLER( midway_tone_generator_hi_w )
{
	discrete_sound_w(device, MIDWAY_TONE_DATA_H, data & 0x3f);

	/* D6 and D7 are not connected */
}



/*************************************
 *
 *  Implementation of the common
 *  noise circuits
 *
 *************************************/

static const discrete_lfsr_desc midway_lfsr =
{
	DISC_CLK_IS_FREQ,
	17,					/* bit length */
						/* the RC network fed into pin 4, has the effect
                           of presetting all bits high at power up */
	0x1ffff,			/* reset value */
	4,					/* use bit 4 as XOR input 0 */
	16,					/* use bit 16 as XOR input 1 */
	DISC_LFSR_XOR,		/* feedback stage1 is XOR */
	DISC_LFSR_OR,		/* feedback stage2 is just stage 1 output OR with external feed */
	DISC_LFSR_REPLACE,	/* feedback stage3 replaces the shifted register contents */
	0x000001,			/* everything is shifted into the first bit only */
	0,					/* output is not inverted */
	12					/* output bit */
};



/*************************************
 *
 *  Sea Wolf
 *
 *************************************/

static const char *const seawolf_sample_names[] =
{
	"*seawolf",
	"shiphit.wav",
	"torpedo.wav",
	"dive.wav",
	"sonar.wav",
	"minehit.wav",
	0
};


static const samples_interface seawolf_samples_interface =
{
	5,	/* 5 channels */
	seawolf_sample_names
};


MACHINE_DRIVER_START( seawolf_audio )
	MDRV_SOUND_START(samples)

	MDRV_SPEAKER_STANDARD_MONO("mono")
	MDRV_SOUND_ADD("samples", SAMPLES, 0)
	MDRV_SOUND_CONFIG(seawolf_samples_interface)
	MDRV_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.6)
MACHINE_DRIVER_END


WRITE8_HANDLER( seawolf_audio_w )
{
	const device_config *samples = devtag_get_device(space->machine, "samples");
	UINT8 rising_bits = data & ~port_1_last;

	/* if (data & 0x01)  enable SHIP HIT sound */
	if (rising_bits & 0x01) sample_start(samples, 0, 0, 0);

	/* if (data & 0x02)  enable TORPEDO sound */
	if (rising_bits & 0x02) sample_start(samples, 1, 1, 0);

	/* if (data & 0x04)  enable DIVE sound */
	if (rising_bits & 0x04) sample_start(samples, 2, 2, 0);

	/* if (data & 0x08)  enable SONAR sound */
	if (rising_bits & 0x08) sample_start(samples, 3, 3, 0);

	/* if (data & 0x10)  enable MINE HIT sound */
	if (rising_bits & 0x10) sample_start(samples, 4, 4, 0);

	coin_counter_w(0, (data >> 5) & 0x01);

	/* D6 and D7 are not connected */

	port_1_last = data;
}



/*************************************
 *
 *  Gun Fight
 *
 *************************************/

static const char *const gunfight_sample_names[] =
{
	"*gunfight",
	"gunshot.wav",
	"killed.wav",
	0
};


static const samples_interface gunfight_samples_interface =
{
	1,	/* 1 channel */
	gunfight_sample_names
};


MACHINE_DRIVER_START( gunfight_audio )
	MDRV_SOUND_START(samples)

	MDRV_SPEAKER_STANDARD_STEREO("lspeaker", "rspeaker")

	MDRV_SOUND_ADD("samples1", SAMPLES, 0)
	MDRV_SOUND_CONFIG(gunfight_samples_interface)
	MDRV_SOUND_ROUTE(ALL_OUTPUTS, "lspeaker", 0.50)

	MDRV_SOUND_ADD("samples2", SAMPLES, 0)
	MDRV_SOUND_CONFIG(gunfight_samples_interface)
	MDRV_SOUND_ROUTE(ALL_OUTPUTS, "rspeaker", 0.50)
MACHINE_DRIVER_END


WRITE8_HANDLER( gunfight_audio_w )
{
	const device_config *samples0 = devtag_get_device(space->machine, "samples1");
	const device_config *samples1 = devtag_get_device(space->machine, "samples2");

	/* D0 and D1 are just tied to 1k resistors */

	coin_counter_w(0, (data >> 2) & 0x01);

	/* the 74175 latches and inverts the top 4 bits */
	switch ((~data >> 4) & 0x0f)
	{
	case 0x00:
		break;

	case 0x01:
		/* enable LEFT SHOOT sound (left speaker) */
		sample_start(samples0, 0, 0, 0);
		break;

	case 0x02:
		/* enable RIGHT SHOOT sound (right speaker) */
		sample_start(samples1, 0, 0, 0);
		break;

	case 0x03:
		/* enable LEFT HIT sound (left speaker) */
		sample_start(samples0, 0, 1, 0);
		break;

	case 0x04:
		/* enable RIGHT HIT sound (right speaker) */
		sample_start(samples1, 0, 1, 0);
		break;

	default:
		logerror("%04x:  Unknown sh port write %02x\n",cpu_get_pc(space->cpu),data);
		break;
	}
}



/*************************************
 *
 *  Tornado Baseball
 *
 *************************************/

#define TORNBASE_SQUAREW_240		NODE_01
#define TORNBASE_SQUAREW_960		NODE_02
#define TORNBASE_SQUAREW_120		NODE_03

#define TORNBASE_TONE_240_EN		NODE_04
#define TORNBASE_TONE_960_EN		NODE_05
#define TORNBASE_TONE_120_EN		NODE_06

#define TORNBASE_TONE_240_SND		NODE_07
#define TORNBASE_TONE_960_SND		NODE_08
#define TORNBASE_TONE_120_SND		NODE_09
#define TORNBASE_TONE_SND			NODE_10
#define TORNBASE_TONE_SND_FILT		NODE_11


static DISCRETE_SOUND_START(tornbase)

	/* the 3 enable lines coming out of the 74175 flip-flop at G5 */
	DISCRETE_INPUT_LOGIC(TORNBASE_TONE_240_EN)		/* pin 2 */
	DISCRETE_INPUT_LOGIC(TORNBASE_TONE_960_EN)		/* pin 7 */
	DISCRETE_INPUT_LOGIC(TORNBASE_TONE_120_EN)		/* pin 5 */

	/* 3 different freq square waves (240, 960 and 120Hz).
       Originates from the CPU board via an edge connector.
       The wave is in the 0/+1 range */
	DISCRETE_SQUAREWFIX(TORNBASE_SQUAREW_240, 1, 240, 1.0, 50.0, 1.0/2, 0)	/* pin X */
	DISCRETE_SQUAREWFIX(TORNBASE_SQUAREW_960, 1, 960, 1.0, 50.0, 1.0/2, 0)	/* pin Y */
	DISCRETE_SQUAREWFIX(TORNBASE_SQUAREW_120, 1, 120, 1.0, 50.0, 1.0/2, 0)	/* pin V */

	/* 7403 O/C NAND gate at G6.  3 of the 4 gates used with their outputs tied together */
	DISCRETE_LOGIC_NAND(TORNBASE_TONE_240_SND, 1, TORNBASE_SQUAREW_240, TORNBASE_TONE_240_EN)	/* pins 4,5,6 */
	DISCRETE_LOGIC_NAND(TORNBASE_TONE_960_SND, 1, TORNBASE_SQUAREW_960, TORNBASE_TONE_960_EN)	/* pins 2,1,3 */
	DISCRETE_LOGIC_NAND(TORNBASE_TONE_120_SND, 1, TORNBASE_SQUAREW_120, TORNBASE_TONE_120_EN)	/* pins 13,12,11 */
	DISCRETE_LOGIC_AND3(TORNBASE_TONE_SND,     1, TORNBASE_TONE_240_SND, TORNBASE_TONE_960_SND, TORNBASE_TONE_120_SND)

	/* 47K resistor (R601) and 0.047uF capacitor (C601)
       There is also a 50K pot acting as a volume control, but we output at
       the maximum volume as MAME has its own volume adjustment */
	DISCRETE_CRFILTER(TORNBASE_TONE_SND_FILT, 1, TORNBASE_TONE_SND, RES_K(47), CAP_U(0.047))

	/* amplify for output */
	DISCRETE_OUTPUT(TORNBASE_TONE_SND_FILT, 32767)

DISCRETE_SOUND_END


MACHINE_DRIVER_START( tornbase_audio )
	MDRV_SPEAKER_STANDARD_MONO("mono")
	MDRV_SOUND_ADD("discrete", DISCRETE, 0)
	MDRV_SOUND_CONFIG_DISCRETE(tornbase)
	MDRV_SOUND_ROUTE(ALL_OUTPUTS, "mono", 1)
MACHINE_DRIVER_END


WRITE8_DEVICE_HANDLER( tornbase_audio_w )
{
	discrete_sound_w(device, TORNBASE_TONE_240_EN, (data >> 0) & 0x01);

	discrete_sound_w(device, TORNBASE_TONE_960_EN, (data >> 1) & 0x01);

	discrete_sound_w(device, TORNBASE_TONE_120_EN, (data >> 2) & 0x01);

	/* if (data & 0x08)  enable SIREN sound */

	/* if (data & 0x10)  enable CHEER sound */

	if (tornbase_get_cabinet_type(device->machine) == TORNBASE_CAB_TYPE_UPRIGHT_OLD)
	{
		/* if (data & 0x20)  enable WHISTLE sound */

		/* D6 is not connected on this cabinet type */
	}
	else
	{
		/* D5 is not connected on this cabinet type */

		/* if (data & 0x40)  enable WHISTLE sound */
	}

	coin_counter_w(0, (data >> 7) & 0x01);
}



/*************************************
 *
 *  280 ZZZAP / Laguna Racer
 *
 *************************************/

MACHINE_DRIVER_START( zzzap_audio )
	MDRV_SPEAKER_STANDARD_MONO("mono")
MACHINE_DRIVER_END


WRITE8_HANDLER( zzzap_audio_1_w )
{
	/* set ENGINE SOUND FREQ(data & 0x0f)  the value written is
                                           the gas pedal position */

	/* if (data & 0x10)  enable HI SHIFT engine sound modifier */

	/* if (data & 0x20)  enable LO SHIFT engine sound modifier */

	/* D6 and D7 are not connected */
}


WRITE8_HANDLER( zzzap_audio_2_w )
{
	/* if (data & 0x01)  enable BOOM sound */

	/* if (data & 0x02)  enable ENGINE sound (global) */

	/* if (data & 0x04)  enable CR 1 (screeching sound) */

	/* if (data & 0x08)  enable NOISE CR 2 (happens only after the car blows up, but
                                            before it appears again, not sure what
                                            it is supposed to sound like) */

	coin_counter_w(0, (data >> 5) & 0x01);

	/* D4, D6 and D7 are not connected */
}



/*************************************
 *
 *  Amazing Maze
 *
 *  Discrete sound emulation: Feb 2007, D.R.
 *
 *************************************/

/* nodes - inputs */
#define MAZE_P1_DATA			NODE_01
#define MAZE_P2_DATA			NODE_02
#define MAZE_TONE_TIMING		NODE_03
#define MAZE_COIN				NODE_04

/* nodes - other */
#define MAZE_JOYSTICK_IN_USE	NODE_11
#define MAZE_AUDIO_ENABLE		NODE_12
#define MAZE_TONE_ENABLE		NODE_13
#define MAZE_GAME_OVER			NODE_14
#define MAZE_R305_306_308		NODE_15
#define MAZE_R303_309			NODE_16
#define MAZE_PLAYER_SEL			NODE_17

/* nodes - sounds */
#define MAZE_SND				NODE_18


static const discrete_555_desc maze_555_F2 =
{
	DISC_555_OUT_SQW | DISC_555_OUT_DC | DISC_555_TRIGGER_IS_LOGIC | DISC_555_TRIGGER_DISCHARGES_CAP,
	5,				/* B+ voltage of 555 */
	DEFAULT_555_VALUES
};


static const double maze_74147_table[] =
{
	3, 3, 3, 3, 3, 3, 3, 3, 0, 0, 0, 0, 1, 1, 2, 3
};


static const discrete_comp_adder_table maze_r305_306_308 =
{
	DISC_COMP_P_RESISTOR,	/* type of circuit */
	RES_K(100),				/* R308 */
	2,						/* length */
	{ RES_M(1.5),			/* R304 */
	  RES_K(820) }			/* R304 */
};


static const discrete_comp_adder_table maze_r303_309 =
{
	DISC_COMP_P_RESISTOR,	/* type of circuit */
	RES_K(330),				/* R309 */
	1,						/* length */
	{ RES_M(1) }			/* R303 */
};


static const discrete_op_amp_osc_info maze_op_amp_osc =
{
	DISC_OP_AMP_OSCILLATOR_1 | DISC_OP_AMP_IS_NORTON | DISC_OP_AMP_OSCILLATOR_OUT_SQW,	/* type */
	RES_M(1),			/* R306 */
	RES_K(430),			/* R307 */
	MAZE_R305_306_308,	/* R304, R305, R308 switchable circuit */
	MAZE_R303_309,		/* R303, R309 switchable circuit */
	RES_K(330),			/* R310 */
	0, 0, 0,			/* not used */
	CAP_P(3300),		/* C300 */
	5					/* vP */
};


static DISCRETE_SOUND_START(maze)

	/************************************************
     * Input register mapping
     ************************************************/
	DISCRETE_INPUT_DATA (MAZE_P1_DATA)
	DISCRETE_INPUT_DATA (MAZE_P2_DATA)
	DISCRETE_INPUT_LOGIC(MAZE_TONE_TIMING)
	DISCRETE_INPUT_LOGIC(MAZE_COIN)
	DISCRETE_INPUT_LOGIC(MAZE_JOYSTICK_IN_USE)	/* IC D2, pin 8 */

	/* The following circuits control when audio is heard. */
	/* Basically there is sound for 30s after a coin is inserted. */
	/* This time is extended whenever a control is pressed. */
	/* After the 30s has expired, there is no sound until the next coin is inserted. */
	/* There is also sound for the first 30s after power up even without a coin. */
	DISCRETE_LOGIC_INVERT(NODE_20,				/* IC E2, pin 8 */
					1,							/* ENAB */
					MAZE_JOYSTICK_IN_USE)		/* IN0 */
	DISCRETE_555_MSTABLE(MAZE_GAME_OVER,		/* IC F2, pin 3 */
					1,							/* RESET */
					NODE_20,					/* TRIG */
					RES_K(270),					/* R203 */
					CAP_U(100),					/* C204 */
					&maze_555_F2)
	DISCRETE_LOGIC_JKFLIPFLOP(MAZE_AUDIO_ENABLE,/* IC F1, pin 5 */
					1,							/* ENAB */
					MAZE_COIN,					/* RESET */
					1,							/* SET */
					MAZE_GAME_OVER,				/* CLK */
					1,							/* J */
					0)							/* K */
	DISCRETE_LOGIC_INVERT(MAZE_TONE_ENABLE,		/* IC F1, pin 6 */
					1,							/* ENAB */
					MAZE_AUDIO_ENABLE)			/* IN0 */
	DISCRETE_LOGIC_AND3(NODE_21,
					1,							/* ENAB */
					MAZE_JOYSTICK_IN_USE,		/* INP0 */
					MAZE_TONE_ENABLE,			/* INP1 */
					MAZE_TONE_TIMING)			/* INP2 */

	/* The following circuits use the control info to generate a tone. */
	DISCRETE_LOGIC_JKFLIPFLOP(MAZE_PLAYER_SEL,	/* IC C1, pin 3 */
					1,							/* ENAB */
					1,							/* RESET */
					1,							/* SET */
					MAZE_TONE_TIMING,			/* CLK */
					1,							/* J */
					1)							/* K */
	DISCRETE_MULTIPLEX2(NODE_31,				/* IC D1 */
					1,							/* ENAB */
					MAZE_PLAYER_SEL,			/* ADDR */
					MAZE_P1_DATA,				/* INP0 */
					MAZE_P2_DATA)				/* INP1 */
	DISCRETE_LOOKUP_TABLE(NODE_32,				/* IC E1 */
					1,							/* ENAB */
					NODE_31,					/* ADDR */
					16,							/* SIZE */
					&maze_74147_table)
	DISCRETE_COMP_ADDER(MAZE_R305_306_308,		/* value of selected parallel circuit R305, R306, R308 */
					NODE_32,					/* DATA */
					&maze_r305_306_308)
	DISCRETE_COMP_ADDER(MAZE_R303_309,			/* value of selected parallel circuit R303, R309 */
					MAZE_PLAYER_SEL,			/* DATA */
					&maze_r303_309)
	DISCRETE_OP_AMP_OSCILLATOR(NODE_36,			/* IC J1, pin 4 */
					1,							/* ENAB */
					&maze_op_amp_osc)

	/* The following circuits remove DC poping noises when the tone is switched in/out. */
	DISCRETE_CRFILTER_VREF(NODE_40,
					1,							/* ENAB */
					NODE_36,					/* IN0 */
					RES_K(250),					/* R311, R312, R402, R403 in parallel */
					CAP_U(0.1),					/* c301 */
					2.5)						/* center voltage of R311, R312 */
	DISCRETE_SWITCH(NODE_41,					/* IC H3, pin 10 */
					1,							/* ENAB */
					NODE_21,					/* switch */
					2.5,						/* INP0 - center voltage of R402, R403 */
					NODE_40)					/* INP1 */
	DISCRETE_CRFILTER(NODE_42,
					1,							/* ENAB */
					NODE_41,					/* IN0 */
					RES_K(56 + 390),			/* R404 + R405 */
					CAP_P(0.01)	)				/* C401 */
	DISCRETE_RCFILTER(NODE_43,
					1,							/* ENAB */
					NODE_42,					/* IN0 */
					RES_K(56),					/* R404 */
					CAP_P(4700)	)				/* C400 */
	DISCRETE_SWITCH(MAZE_SND,					/* H3 saturates op-amp J3 when enabled, disabling audio */
					1,							/* ENAB */
					MAZE_AUDIO_ENABLE,			/* SWITCH */
					NODE_43,					/* INP0 */
					0)							/* INP1 */

	DISCRETE_OUTPUT(MAZE_SND, 96200)
DISCRETE_SOUND_END


MACHINE_DRIVER_START( maze_audio )
	MDRV_SPEAKER_STANDARD_MONO("mono")
	MDRV_SOUND_ADD("discrete", DISCRETE, 0)
	MDRV_SOUND_CONFIG_DISCRETE(maze)
	MDRV_SOUND_ROUTE(ALL_OUTPUTS, "mono", 1.0)
MACHINE_DRIVER_END


void maze_write_discrete(const device_config *device, UINT8 maze_tone_timing_state)
{
	/* controls need to be active low */
	int controls = ~input_port_read(device->machine, "IN0") & 0xff;

	discrete_sound_w(device, MAZE_TONE_TIMING, maze_tone_timing_state);
	discrete_sound_w(device, MAZE_P1_DATA, controls & 0x0f);
	discrete_sound_w(device, MAZE_P2_DATA, (controls >> 4) & 0x0f);
	discrete_sound_w(device, MAZE_JOYSTICK_IN_USE, controls != 0xff);

	/* The coin line is connected directly to the discrete circuit. */
	/* We can't really do that, so updating it with the tone timing is close enough. */
	/* A better option might be to update it at vblank or set a timer to do it. */
	/* The only noticeable difference doing it here, is that the controls don't */
	/* imediately start making tones if pressed right after the coin is inserted. */
	discrete_sound_w(device, MAZE_COIN, (~input_port_read(device->machine, "IN1") >> 3) & 0x01);
}



/*************************************
 *
 *  Boot Hill
 *
 *  Discrete sound emulation: Jan 2007, D.R.
 *
 *************************************/

/* nodes - inputs */
#define BOOTHILL_GAME_ON_EN			NODE_01
#define BOOTHILL_LEFT_SHOT_EN		NODE_02
#define BOOTHILL_RIGHT_SHOT_EN		NODE_03
#define BOOTHILL_LEFT_HIT_EN		NODE_04
#define BOOTHILL_RIGHT_HIT_EN		NODE_05

/* nodes - sounds */
#define BOOTHILL_NOISE				NODE_06
#define BOOTHILL_L_SHOT_SND			NODE_07
#define BOOTHILL_R_SHOT_SND			NODE_08
#define BOOTHILL_L_HIT_SND			NODE_09
#define BOOTHILL_R_HIT_SND			NODE_10

/* nodes - adjusters */
#define BOOTHILL_MUSIC_ADJ			NODE_11


static const discrete_op_amp_tvca_info boothill_tone_tvca_info =
{
	RES_M(3.3),
	RES_K(100) + RES_K(680),
	0,
	RES_K(680),
	RES_K(10),
	0,
	RES_K(680),
	0,
	0,
	0,
	0,
	CAP_U(.001),
	0,
	0,
	12,
	0,
	0,
	12,
	DISC_OP_AMP_TRIGGER_FUNCTION_TRG0,
	DISC_OP_AMP_TRIGGER_FUNCTION_NONE,
	DISC_OP_AMP_TRIGGER_FUNCTION_TRG1,
	DISC_OP_AMP_TRIGGER_FUNCTION_NONE,
	DISC_OP_AMP_TRIGGER_FUNCTION_NONE,
	DISC_OP_AMP_TRIGGER_FUNCTION_NONE
};


static const discrete_op_amp_tvca_info boothill_shot_tvca_info =
{
	RES_M(2.7),
	RES_K(510),
	0,
	RES_K(510),
	RES_K(10),
	0,
	RES_K(510),
	0,
	0,
	0,
	0,
	CAP_U(0.22),
	0,
	0,
	12,
	0,
	0,
	12,
	DISC_OP_AMP_TRIGGER_FUNCTION_NONE,
	DISC_OP_AMP_TRIGGER_FUNCTION_NONE,
	DISC_OP_AMP_TRIGGER_FUNCTION_TRG0,
	DISC_OP_AMP_TRIGGER_FUNCTION_NONE,
	DISC_OP_AMP_TRIGGER_FUNCTION_NONE,
	DISC_OP_AMP_TRIGGER_FUNCTION_NONE
};


static const discrete_op_amp_tvca_info boothill_hit_tvca_info =
{
	RES_M(2.7),
	RES_K(510),
	0,
	RES_K(510),
	RES_K(10),
	0,
	RES_K(510),
	0,
	0,
	0,
	0,
	CAP_U(1),
	0,
	0,
	12,
	0,
	0,
	12,
	DISC_OP_AMP_TRIGGER_FUNCTION_NONE,
	DISC_OP_AMP_TRIGGER_FUNCTION_NONE,
	DISC_OP_AMP_TRIGGER_FUNCTION_TRG0,
	DISC_OP_AMP_TRIGGER_FUNCTION_NONE,
	DISC_OP_AMP_TRIGGER_FUNCTION_NONE,
	DISC_OP_AMP_TRIGGER_FUNCTION_NONE
};


static const discrete_mixer_desc boothill_l_mixer =
{
	DISC_MIXER_IS_OP_AMP,
	{ RES_K(12) + RES_K(68) + RES_K(33),
	  RES_K(12) + RES_K(100) + RES_K(33) },
	{ 0 },
	{ 0 },
	0,
	RES_K(100),
	0,
	CAP_U(0.1),
	0,
	7200	/* final gain */
};


static const discrete_mixer_desc boothill_r_mixer =
{
	DISC_MIXER_IS_OP_AMP,
	{ RES_K(12) + RES_K(68) + RES_K(33),
	  RES_K(12) + RES_K(100) + RES_K(33),
	  RES_K(33) },
	{ 0,
	  0,
	  BOOTHILL_MUSIC_ADJ },
	{ 0 },
	0,
	RES_K(100),
	0,
	CAP_U(0.1),
	0,
	7200	/* final gain */
};


static DISCRETE_SOUND_START(boothill)

	/************************************************
     * Input register mapping
     ************************************************/
	DISCRETE_INPUT_LOGIC(BOOTHILL_GAME_ON_EN)
	DISCRETE_INPUT_LOGIC(BOOTHILL_LEFT_SHOT_EN)
	DISCRETE_INPUT_LOGIC(BOOTHILL_RIGHT_SHOT_EN)
	DISCRETE_INPUT_LOGIC(BOOTHILL_LEFT_HIT_EN)
	DISCRETE_INPUT_LOGIC(BOOTHILL_RIGHT_HIT_EN)

	/* The low value of the pot is set to 75000.  A real 1M pot will never go to 0 anyways.
       This will give the control more apparent volume range.
       The music way overpowers the rest of the sounds anyways. */
	DISCRETE_ADJUSTMENT_TAG(BOOTHILL_MUSIC_ADJ, RES_M(1), 75000, DISC_LOGADJ, "MUSIC_ADJ")

	/************************************************
     * Tone generator
     ************************************************/
	MIDWAY_TONE_GENERATOR(boothill_tone_tvca_info)

	/************************************************
     * Shot sounds
     ************************************************/
	/* Noise clock was breadboarded and measured at 7700Hz */
	DISCRETE_LFSR_NOISE(BOOTHILL_NOISE, 1, 1, 7700, 12.0, 0, 12.0/2, &midway_lfsr)

	DISCRETE_OP_AMP_TRIG_VCA(NODE_30, BOOTHILL_LEFT_SHOT_EN, 0, 0, BOOTHILL_NOISE, 0, &boothill_shot_tvca_info)
	DISCRETE_RCFILTER(NODE_31, 1, NODE_30, RES_K(12), CAP_U(.01))
	DISCRETE_RCFILTER(BOOTHILL_L_SHOT_SND, 1, NODE_31, RES_K(12) + RES_K(68), CAP_U(.0022))

	DISCRETE_OP_AMP_TRIG_VCA(NODE_35, BOOTHILL_RIGHT_SHOT_EN, 0, 0, BOOTHILL_NOISE, 0, &boothill_shot_tvca_info)
	DISCRETE_RCFILTER(NODE_36, 1, NODE_35, RES_K(12), CAP_U(.01))
	DISCRETE_RCFILTER(BOOTHILL_R_SHOT_SND, 1, NODE_36, RES_K(12) + RES_K(68), CAP_U(.0033))

	/************************************************
     * Hit sounds
     ************************************************/
	DISCRETE_OP_AMP_TRIG_VCA(NODE_40, BOOTHILL_LEFT_HIT_EN, 0, 0, BOOTHILL_NOISE, 0, &boothill_hit_tvca_info)
	DISCRETE_RCFILTER(NODE_41, 1, NODE_40, RES_K(12), CAP_U(.033))
	DISCRETE_RCFILTER(BOOTHILL_L_HIT_SND, 1, NODE_41, RES_K(12) + RES_K(100), CAP_U(.0033))

	DISCRETE_OP_AMP_TRIG_VCA(NODE_45, BOOTHILL_RIGHT_HIT_EN, 0, 0, BOOTHILL_NOISE, 0, &boothill_hit_tvca_info)
	DISCRETE_RCFILTER(NODE_46, 1, NODE_45, RES_K(12), CAP_U(.0033))
	DISCRETE_RCFILTER(BOOTHILL_R_HIT_SND, 1, NODE_46, RES_K(12) + RES_K(100), CAP_U(.0022))

	/************************************************
     * Combine all sound sources.
     ************************************************/
	/* There is a 1uF cap on the input to the amp that I was too lazy to simulate.
     * It is just a DC blocking cap needed by the Norton amp.  Doing the extra
     * work to simulate it is not going to make a difference to the waveform
     * or to how it sounds.  Also I use a regular amp in place of the Norton
     * for the same reasons.  Ease of coding/simulation. */

	/* The schematics show the Hit sounds as shown.
     * This makes the death of the enemy sound on the players side.
     * This should be verified. */

	DISCRETE_MIXER2(NODE_91, BOOTHILL_GAME_ON_EN, BOOTHILL_L_SHOT_SND, BOOTHILL_L_HIT_SND, &boothill_l_mixer)

	/* Music is only added to the right channel per schematics */
	/* This should be verified on the real game */
	DISCRETE_MIXER3(NODE_92, BOOTHILL_GAME_ON_EN, BOOTHILL_R_SHOT_SND, BOOTHILL_R_HIT_SND, MIDWAY_TONE_SND, &boothill_r_mixer)

	DISCRETE_OUTPUT(NODE_91, 1)
	DISCRETE_OUTPUT(NODE_92, 1)
DISCRETE_SOUND_END


MACHINE_DRIVER_START( boothill_audio )
	MDRV_SPEAKER_STANDARD_STEREO("lspeaker", "rspeaker")
	MDRV_SOUND_ADD("discrete", DISCRETE, 0)
	MDRV_SOUND_CONFIG_DISCRETE(boothill)
	MDRV_SOUND_ROUTE(0, "lspeaker", 1.0)
	MDRV_SOUND_ROUTE(1, "rspeaker", 1.0)
MACHINE_DRIVER_END


WRITE8_DEVICE_HANDLER( boothill_audio_w )
{
	/* D0 and D1 are not connected */

	coin_counter_w(0, (data >> 2) & 0x01);

	discrete_sound_w(device, BOOTHILL_GAME_ON_EN, (data >> 3) & 0x01);

	discrete_sound_w(device, BOOTHILL_LEFT_SHOT_EN, (data >> 4) & 0x01);

	discrete_sound_w(device, BOOTHILL_RIGHT_SHOT_EN, (data >> 5) & 0x01);

	discrete_sound_w(device, BOOTHILL_LEFT_HIT_EN, (data >> 6) & 0x01);

	discrete_sound_w(device, BOOTHILL_RIGHT_HIT_EN, (data >> 7) & 0x01);
}



/*************************************
 *
 *  Checkmate
 *
 *************************************/

/* nodes - inputs */
#define CHECKMAT_BOOM_EN			NODE_01
#define CHECKMAT_TONE_EN			NODE_02
#define CHECKMAT_TONE_DATA_45		NODE_03
#define CHECKMAT_TONE_DATA_67		NODE_04

/* nodes - other */
#define CHECKMAT_R401_402_400		NODE_06
#define CHECKMAT_R407_406_410		NODE_07

/* nodes - sounds */
#define CHECKMAT_BOOM_SND			NODE_10
#define CHECKMAT_TONE_SND			NODE_11
#define CHECKMAT_FINAL_SND			NODE_12

/* nodes - adjusters */
#define CHECKMAT_R309				NODE_15
#define CHECKMAT_R411				NODE_16


static const discrete_comp_adder_table checkmat_r401_402_400 =
{
	DISC_COMP_P_RESISTOR,	/* type of circuit */
	RES_K(100),				/* R401 */
	2,						/* length */
	{ RES_M(1.5),			/* R402 */
	  RES_K(820) }			/* R400 */
};


static const discrete_comp_adder_table checkmat_r407_406_410 =
{
	DISC_COMP_P_RESISTOR,	/* type of circuit */
	RES_K(330),				/* R407 */
	2,						/* length */
	{ RES_M(1),				/* R406 */
	  RES_K(510) }			/* R410 */
};


static const discrete_op_amp_osc_info checkmat_op_amp_osc =
{
	DISC_OP_AMP_OSCILLATOR_1 | DISC_OP_AMP_IS_NORTON | DISC_OP_AMP_OSCILLATOR_OUT_SQW,	/* type */
	RES_M(1),				/* R403 */
	RES_K(430),				/* R405 */
	CHECKMAT_R401_402_400,	/* R401, R402, R400 switchable circuit */
	CHECKMAT_R407_406_410,	/* R407, R406, R410 switchable circuit */
	RES_K(330),				/* R404 */
	0, 0, 0,				/* not used */
	CAP_P(3300),			/* C400 */
	5						/* vP */
};


static const discrete_op_amp_tvca_info checkmat_op_amp_tvca =
{
	RES_M(1.2),	/* R302 */
	RES_M(1),	/* R305 */
	0,			/* r3 - not used */
	RES_M(1.2),	/* R304 */
	RES_K(1),	/* M4 */
	0,			/* r6 - not used */
	RES_M(1),	/* R303 */
	0,			/* r8 - not used */
	0,			/* r9 - not used */
	0,			/* r10 - not used */
	0,			/* r11 - not used */
	CAP_U(1),	/* C300 */
	0,			/* c2 - not used */
	0,			/* c3 - not used */
	5,			/* v1 */
	0,			/* v2 */
	0,			/* v3 */
	5,			/* vP */
	DISC_OP_AMP_TRIGGER_FUNCTION_NONE,	/* f0 - not used */
	DISC_OP_AMP_TRIGGER_FUNCTION_NONE,	/* f1 - not used */
	DISC_OP_AMP_TRIGGER_FUNCTION_TRG0,	/* f2 */
	DISC_OP_AMP_TRIGGER_FUNCTION_NONE,	/* f3 - not used */
	DISC_OP_AMP_TRIGGER_FUNCTION_NONE,	/* f4 - not used */
	DISC_OP_AMP_TRIGGER_FUNCTION_NONE,	/* f5 - not used */
};


static const discrete_mixer_desc checkmat_mixer =
{
	DISC_MIXER_IS_OP_AMP,	/* type */
	{ RES_K(100),		/* R308 - VERIFY - can't read schematic */
	  RES_K(56 + 47) },	/* R412 + R408 */
	{ CHECKMAT_R309,	/* R309 */
	  CHECKMAT_R411},	/* R411 */
	{ CAP_U(10),		/* C305 */
	  CAP_U(0.01) },	/* C401 */
	0,					/* rI - not used */
	RES_K(100),			/* R507 */
	0,					/* cF - not used */
	CAP_U(1),			/* C505 */
	0,					/* vRef - GND */
	1					/* gain */
};

static DISCRETE_SOUND_START(checkmat)

	/************************************************
     * Input register mapping
     ************************************************/
	DISCRETE_INPUT_LOGIC(CHECKMAT_BOOM_EN)
	DISCRETE_INPUT_LOGIC(CHECKMAT_TONE_EN)
	DISCRETE_INPUT_DATA (CHECKMAT_TONE_DATA_45)
	DISCRETE_INPUT_DATA (CHECKMAT_TONE_DATA_67)

	/* The low value of the resistors are tweaked to give a good volume range. */
	/* This is needed because the original controls are infinite, but the UI only gives 100 steps. */
	/* Also real variable resistors never hit 0 ohms.  There is always some resistance. */
	/* R309 mostly just increases the Boom clipping, making it sound bassier. */
	DISCRETE_ADJUSTMENT_TAG(CHECKMAT_R309, RES_K(100), 1000, DISC_LOGADJ, "R309")
	DISCRETE_ADJUSTMENT_TAG(CHECKMAT_R411, RES_M(1), 1000, DISC_LOGADJ, "R411")

	/************************************************
     * Boom Sound
     *
     * The zener diode noise source is hard to
     * emulate.  Guess for now.
     ************************************************/
	/* FIX - find noise freq and amplitude */
	DISCRETE_NOISE(NODE_20,
					1,							/* ENAB */
					1500,						/* FREQ */
					2,							/* AMP */
					0)						/* BIAS */
	DISCRETE_OP_AMP_TRIG_VCA(NODE_21,
					CHECKMAT_BOOM_EN,			/* TRG0 */
					0,							/* TRG1 - not used */
					0,							/* TRG2 - not used */
					NODE_20,					/* IN0 */
					0,							/* IN1 - not used */
					&checkmat_op_amp_tvca)
	/* The next 5 modules emulate the filter. */
	DISCRETE_FILTER2(NODE_23,
					1,							/* ENAB */
					NODE_21,					/* INP0 */
					35,						/* FREQ */
					1.0 / 8,					/* DAMP */
					DISC_FILTER_BANDPASS)
	DISCRETE_GAIN(NODE_24,
					NODE_23,					/* IN0 */
					15)							/* GAIN */
	DISCRETE_CLAMP(CHECKMAT_BOOM_SND,			/* IC Q2/3, pin 10 */
					1,							/* ENAB */
					NODE_24,					/* IN0 */
					0 - 6,						/* MIN */
					12.0 - OP_AMP_NORTON_VBE -6,/* MAX */
					0)							/* CLAMP */

	/************************************************
     * Tone generator
     ************************************************/
	DISCRETE_COMP_ADDER(CHECKMAT_R401_402_400,	/* value of selected parallel circuit R401, R402, R400 */
					CHECKMAT_TONE_DATA_45,		/* DATA */
					&checkmat_r401_402_400)
	DISCRETE_COMP_ADDER(CHECKMAT_R407_406_410,	/* value of selected parallel circuit R407, R406, R410 */
					CHECKMAT_TONE_DATA_67,		/* DATA */
					&checkmat_r407_406_410)
	DISCRETE_OP_AMP_OSCILLATOR(NODE_30,			/* IC N3/4, pin 4 */
					1,							/* ENAB */
					&checkmat_op_amp_osc)

	/* The following circuits remove DC poping noises when the tone is switched in/out. */
	DISCRETE_CRFILTER_VREF(NODE_31,
					1,							/* ENAB */
					NODE_30,					/* IN0 */
					RES_K(250),					/* R409, R415, R414, R413 in parallel */
					CAP_U(0.1),					/* c401 */
					2.5)						/* center voltage of R409, R415 */
	DISCRETE_SWITCH(NODE_32,					/* IC R3/4, pin 9 */
					1,							/* ENAB */
					CHECKMAT_TONE_EN,			/* switch */
					2.5,						/* INP0 - center voltage of R413, R414 */
					NODE_31)					/* INP1 */
	DISCRETE_CRFILTER(NODE_33,
					1,							/* ENAB */
					NODE_32,					/* IN0 */
					RES_K(56 + 47 + 200),		/* R412 + R408 + part of R411 */
					CAP_P(0.01)	)				/* C404 */
	DISCRETE_RCFILTER(CHECKMAT_TONE_SND,
					1,							/* ENAB */
					NODE_33,					/* IN0 */
					RES_K(56),					/* R412 */
					CAP_P(4700)	)				/* C403 */

	/************************************************
     * Final mix and output
     ************************************************/
	DISCRETE_MIXER2(CHECKMAT_FINAL_SND,
					1,							/* ENAB */
					CHECKMAT_BOOM_SND,			/* IN0 */
					CHECKMAT_TONE_SND,			/* IN1 */
					&checkmat_mixer)
	DISCRETE_OUTPUT(CHECKMAT_FINAL_SND, 300000)
DISCRETE_SOUND_END


MACHINE_DRIVER_START( checkmat_audio )
	MDRV_SPEAKER_STANDARD_MONO("mono")
	MDRV_SOUND_ADD("discrete", DISCRETE, 0)
	MDRV_SOUND_CONFIG_DISCRETE(checkmat)
	MDRV_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.4)
MACHINE_DRIVER_END


WRITE8_DEVICE_HANDLER( checkmat_audio_w )
{
	discrete_sound_w(device, CHECKMAT_TONE_EN, data & 0x01);

	discrete_sound_w(device, CHECKMAT_BOOM_EN, (data >> 1) & 0x01);

	coin_counter_w(0, (data >> 2) & 0x01);

	sound_global_enable((data >> 3) & 0x01);

	discrete_sound_w(device, CHECKMAT_TONE_DATA_45, (data >> 4) & 0x03);
	discrete_sound_w(device, CHECKMAT_TONE_DATA_67, (data >> 6) & 0x03);
}



/*************************************
 *
 *  Desert Gun
 *
 *  Discrete sound emulation: Jan 2007, D.R.
 *
 *************************************/

/* nodes - inputs */
#define DESERTGU_GAME_ON_EN					NODE_01
#define DESERTGU_RIFLE_SHOT_EN				NODE_02
#define DESERTGU_BOTTLE_HIT_EN				NODE_03
#define DESERTGU_ROAD_RUNNER_HIT_EN			NODE_04
#define DESERTGU_CREATURE_HIT_EN			NODE_05
#define DESERTGU_ROADRUNNER_BEEP_BEEP_EN	NODE_06
#define DESERTGU_TRIGGER_CLICK_EN			NODE_07

/* nodes - sounds */
#define DESERTGU_NOISE						NODE_08
#define DESERTGU_RIFLE_SHOT_SND				NODE_09
#define DESERTGU_BOTTLE_HIT_SND				NODE_10
#define DESERTGU_ROAD_RUNNER_HIT_SND		NODE_11
#define DESERTGU_CREATURE_HIT_SND			NODE_12
#define DESERTGU_ROADRUNNER_BEEP_BEEP_SND	NODE_13
#define DESERTGU_TRIGGER_CLICK_SND			DESERTGU_TRIGGER_CLICK_EN

/* nodes - adjusters */
#define DESERTGU_MUSIC_ADJ					NODE_15


static const discrete_op_amp_tvca_info desertgu_tone_music_info =
{
	RES_M(3.3),					/* r502 */
	RES_K(10) + RES_K(680),		/* r505 + r506 */
	0,
	RES_K(680),					/* r503 */
	RES_K(10),					/* r500 */
	0,
	RES_K(680),					/* r501 */
	0,
	0,
	0,
	0,
	CAP_U(.001),				/* c500 */
	0, 0,
	12,			/* v1 */
	0,			/* v2 */
	0,			/* v3 */
	12,			/* vP */
	DISC_OP_AMP_TRIGGER_FUNCTION_TRG0,
	DISC_OP_AMP_TRIGGER_FUNCTION_NONE,
	DISC_OP_AMP_TRIGGER_FUNCTION_TRG1,
	DISC_OP_AMP_TRIGGER_FUNCTION_NONE,
	DISC_OP_AMP_TRIGGER_FUNCTION_NONE,
	DISC_OP_AMP_TRIGGER_FUNCTION_NONE
};


static const discrete_op_amp_tvca_info desertgu_rifle_shot_tvca_info =
{
	RES_M(2.7),
	RES_K(680),
	0,
	RES_K(680),
	RES_K(10),
	0,
	RES_K(680),
	0,
	0,
	0,
	0,
	CAP_U(0.47),
	0,
	0,
	12,			/* v1 */
	0,			/* v2 */
	0,			/* v3 */
	12,			/* vP */
	DISC_OP_AMP_TRIGGER_FUNCTION_NONE,
	DISC_OP_AMP_TRIGGER_FUNCTION_NONE,
	DISC_OP_AMP_TRIGGER_FUNCTION_TRG0,
	DISC_OP_AMP_TRIGGER_FUNCTION_NONE,
	DISC_OP_AMP_TRIGGER_FUNCTION_NONE,
	DISC_OP_AMP_TRIGGER_FUNCTION_NONE
};


static const discrete_mixer_desc desertgu_filter_mixer =
{
	DISC_MIXER_IS_RESISTOR,
	{ RES_K(2),
	  RES_K(27),
	  RES_K(2) + RES_K(1) },
	{ 0 },
	{ 0 },
	0,
	0,
	0,
	0,
	0,
	1
};


static const discrete_op_amp_filt_info desertgu_filter =
{
	1.0 / ( 1.0 / RES_K(2) + 1.0 / RES_K(27) + 1.0 / (RES_K(2) + RES_K(1))),
	0,
	68,
	0,
	RES_K(39),
	CAP_U(0.033),
	CAP_U(0.033),
	0,
	0,
	12,
	0
};


static const discrete_mixer_desc desertgu_mixer =
{
	DISC_MIXER_IS_OP_AMP,
	{ RES_K(12) + RES_K(68) + RES_K(30),
	  RES_K(56),
	  RES_K(180),
	  RES_K(47),
	  RES_K(30) },
	{ 0,
	  0,
	  0,
	  0,
	  DESERTGU_MUSIC_ADJ },
	{ CAP_U(0.1),
	  CAP_U(0.1),
	  CAP_U(0.1),
	  CAP_U(0.1),
	  CAP_U(0.1) },
	0,
	RES_K(100),
	0,
	CAP_U(0.1),
	0,
	6000	/* final gain */
};


static DISCRETE_SOUND_START(desertgu)

	/************************************************
     * Input register mapping
     ************************************************/
	DISCRETE_INPUT_LOGIC(DESERTGU_GAME_ON_EN)
	DISCRETE_INPUT_LOGIC(DESERTGU_RIFLE_SHOT_EN)
	DISCRETE_INPUT_LOGIC(DESERTGU_BOTTLE_HIT_EN)
	DISCRETE_INPUT_LOGIC(DESERTGU_ROAD_RUNNER_HIT_EN)
	DISCRETE_INPUT_LOGIC(DESERTGU_CREATURE_HIT_EN)
	DISCRETE_INPUT_LOGIC(DESERTGU_ROADRUNNER_BEEP_BEEP_EN)
	DISCRETE_INPUTX_LOGIC(DESERTGU_TRIGGER_CLICK_SND, 12, 0, 0)

	/* The low value of the pot is set to 75000.  A real 1M pot will never go to 0 anyways. */
	/* This will give the control more apparent volume range. */
	/* The music way overpowers the rest of the sounds anyways. */
	DISCRETE_ADJUSTMENT_TAG(DESERTGU_MUSIC_ADJ, RES_M(1), 75000, DISC_LOGADJ, "MUSIC_ADJ")

	/************************************************
     * Tone generator
     ************************************************/
	MIDWAY_TONE_GENERATOR(desertgu_tone_music_info)

	/************************************************
     * Rifle shot sound
     ************************************************/
	/* Noise clock was breadboarded and measured at 7515Hz */
	DISCRETE_LFSR_NOISE(DESERTGU_NOISE, 1, 1, 7515, 12.0, 0, 12.0/2, &midway_lfsr)

	DISCRETE_OP_AMP_TRIG_VCA(NODE_30, DESERTGU_RIFLE_SHOT_EN, 0, 0, DESERTGU_NOISE, 0, &desertgu_rifle_shot_tvca_info)
	DISCRETE_RCFILTER(NODE_31, 1, NODE_30, RES_K(12), CAP_U(.01))
	DISCRETE_CRFILTER(DESERTGU_RIFLE_SHOT_SND, 1, NODE_31, RES_K(12) + RES_K(68), CAP_U(.0022))

	/************************************************
     * Bottle hit sound
     ************************************************/
	DISCRETE_CONSTANT(DESERTGU_BOTTLE_HIT_SND, 0)  /* placeholder for incomplete sound */

	/************************************************
     * Road Runner hit sound
     ************************************************/
	DISCRETE_CONSTANT(DESERTGU_ROAD_RUNNER_HIT_SND, 0)  /* placeholder for incomplete sound */

	/************************************************
     * Creature hit sound
     ************************************************/
	DISCRETE_CONSTANT(DESERTGU_CREATURE_HIT_SND, 0)  /* placeholder for incomplete sound */

	/************************************************
     * Beep-Beep sound
     ************************************************/
	DISCRETE_CONSTANT(DESERTGU_ROADRUNNER_BEEP_BEEP_SND, 0) /* placeholder for incomplete sound */

	/************************************************
     * Mix and filter
     ************************************************/
	DISCRETE_MIXER3(NODE_80, 1, DESERTGU_BOTTLE_HIT_SND, DESERTGU_ROADRUNNER_BEEP_BEEP_SND, DESERTGU_TRIGGER_CLICK_SND, &desertgu_filter_mixer)
	DISCRETE_OP_AMP_FILTER(NODE_81, 1, NODE_80, 0, DISC_OP_AMP_FILTER_IS_BAND_PASS_1, &desertgu_filter)

	/************************************************
     * Combine all sound sources.
     ************************************************/
	DISCRETE_MIXER5(NODE_91, DESERTGU_GAME_ON_EN, DESERTGU_RIFLE_SHOT_SND, DESERTGU_ROAD_RUNNER_HIT_SND, DESERTGU_CREATURE_HIT_SND, NODE_81, MIDWAY_TONE_SND, &desertgu_mixer)

	DISCRETE_OUTPUT(NODE_91, 1)
DISCRETE_SOUND_END


MACHINE_DRIVER_START( desertgu_audio )
	MDRV_SPEAKER_STANDARD_MONO("mono")
	MDRV_SOUND_ADD("discrete", DISCRETE, 0)
	MDRV_SOUND_CONFIG_DISCRETE(desertgu)
	MDRV_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.8)
MACHINE_DRIVER_END


WRITE8_DEVICE_HANDLER( desertgu_audio_1_w )
{
	/* D0 and D1 are not connected */

	coin_counter_w(0, (data >> 2) & 0x01);

	discrete_sound_w(device, DESERTGU_GAME_ON_EN, (data >> 3) & 0x01);

	discrete_sound_w(device, DESERTGU_RIFLE_SHOT_EN, (data >> 4) & 0x01);

	discrete_sound_w(device, DESERTGU_BOTTLE_HIT_EN, (data >> 5) & 0x01);

	discrete_sound_w(device, DESERTGU_ROAD_RUNNER_HIT_EN, (data >> 6) & 0x01);

	discrete_sound_w(device, DESERTGU_CREATURE_HIT_EN, (data >> 7) & 0x01);
}


WRITE8_DEVICE_HANDLER( desertgu_audio_2_w )
{
	discrete_sound_w(device, DESERTGU_ROADRUNNER_BEEP_BEEP_EN, (data >> 0) & 0x01);

	discrete_sound_w(device, DESERTGU_TRIGGER_CLICK_EN, (data >> 1) & 0x01);

	output_set_value("KICKER", (data >> 2) & 0x01);

	desertgun_set_controller_select((data >> 3) & 0x01);

	/* D4-D7 are not connected */
}



/*************************************
 *
 *  Double Play / Extra Inning
 *
 *  Discrete sound emulation: Jan 2007, D.R.
 *
 *************************************/

/* nodes - inputs */
#define DPLAY_GAME_ON_EN	NODE_01
#define DPLAY_TONE_ON_EN	NODE_02
#define DPLAY_SIREN_EN		NODE_03
#define DPLAY_WHISTLE_EN	NODE_04
#define DPLAY_CHEER_EN		NODE_05

/* nodes - sounds */
#define DPLAY_NOISE			NODE_06
#define DPLAY_TONE_SND		NODE_07
#define DPLAY_SIREN_SND		NODE_08
#define DPLAY_WHISTLE_SND	NODE_09
#define DPLAY_CHEER_SND		NODE_10

/* nodes - adjusters */
#define DPLAY_MUSIC_ADJ		NODE_11


static const discrete_lfsr_desc dplay_lfsr =
{
	DISC_CLK_IS_FREQ,
	17,					/* bit length */
						/* the RC network fed into pin 4, has the effect
                           of presetting all bits high at power up */
	0x1ffff,			/* reset value */
	4,					/* use bit 4 as XOR input 0 */
	16,					/* use bit 16 as XOR input 1 */
	DISC_LFSR_XOR,		/* feedback stage1 is XOR */
	DISC_LFSR_OR,		/* feedback stage2 is just stage 1 output OR with external feed */
	DISC_LFSR_REPLACE,	/* feedback stage3 replaces the shifted register contents */
	0x000001,			/* everything is shifted into the first bit only */
	0,					/* output is not inverted */
	8					/* output bit */
};


static const discrete_op_amp_tvca_info dplay_music_tvca_info =
{
	RES_M(3.3),
	RES_K(10) + RES_K(680),
	0,
	RES_K(680),
	RES_K(10),
	0,
	RES_K(680),
	0,
	0,
	0,
	0,
	CAP_U(.001),
	0,
	0,
	12,			/* v1 */
	0,			/* v2 */
	0,			/* v3 */
	12,			/* vP */
	DISC_OP_AMP_TRIGGER_FUNCTION_TRG0,
	DISC_OP_AMP_TRIGGER_FUNCTION_NONE,
	DISC_OP_AMP_TRIGGER_FUNCTION_TRG1,
	DISC_OP_AMP_TRIGGER_FUNCTION_NONE,
	DISC_OP_AMP_TRIGGER_FUNCTION_NONE,
	DISC_OP_AMP_TRIGGER_FUNCTION_NONE
};


static const discrete_integrate_info dplay_siren_integrate_info =
{
	DISC_INTEGRATE_OP_AMP_1 | DISC_OP_AMP_IS_NORTON,
	RES_M(1),
	RES_K(100),
	0,
	CAP_U(3.3),
	12,
	12,
	DISC_OP_AMP_TRIGGER_FUNCTION_NONE,
	DISC_OP_AMP_TRIGGER_FUNCTION_NONE,
	DISC_OP_AMP_TRIGGER_FUNCTION_NONE
};


static const discrete_op_amp_osc_info dplay_siren_osc =
{
	DISC_OP_AMP_OSCILLATOR_VCO_2 | DISC_OP_AMP_IS_NORTON | DISC_OP_AMP_OSCILLATOR_OUT_SQW,	/* type */
	RES_K(390),		/* r1 */
	RES_M(5.6),		/* r2 */
	RES_M(1),		/* r3 */
	RES_M(1.5),		/* r4 */
	RES_M(3.3),		/* r5 */
	RES_K(56),		/* r6 */
	0,				/* no r7 */
	0,				/* no r8 */
	CAP_U(0.0022),	/* c */
	12				/* vP */
};

static const discrete_integrate_info dplay_whistle_integrate_info =
{
	DISC_INTEGRATE_OP_AMP_1 | DISC_OP_AMP_IS_NORTON,
	RES_M(1),
	RES_K(220) + RES_K(10),
	0,
	CAP_U(3.3),
	12,
	12,
	DISC_OP_AMP_TRIGGER_FUNCTION_NONE,
	DISC_OP_AMP_TRIGGER_FUNCTION_NONE,
	DISC_OP_AMP_TRIGGER_FUNCTION_NONE
};


static const discrete_op_amp_osc_info dplay_whistle_osc =
{
	DISC_OP_AMP_OSCILLATOR_VCO_2 | DISC_OP_AMP_IS_NORTON | DISC_OP_AMP_OSCILLATOR_OUT_SQW,	/* type */
	RES_K(510),		/* r1 */
	RES_M(5.6),		/* r2 */
	RES_M(1),		/* r3 */
	RES_M(1.5),		/* r4 */
	RES_M(3.3),		/* r5 */
	RES_K(300),		/* r6 */
	0,				/* no r7 */
	0,				/* no r8 */
	CAP_P(220),		/* c */
	12				/* vP */
};


static const discrete_integrate_info dplay_cheer_integrate_info =
{
	DISC_INTEGRATE_OP_AMP_1 | DISC_OP_AMP_IS_NORTON,
	RES_M(1.5),
	RES_K(100),
	0,
	CAP_U(4.7),
	12,
	12,
	DISC_OP_AMP_TRIGGER_FUNCTION_NONE,
	DISC_OP_AMP_TRIGGER_FUNCTION_NONE,
	DISC_OP_AMP_TRIGGER_FUNCTION_NONE
};


static const discrete_op_amp_filt_info dplay_cheer_filter =
{
	RES_K(100),
	0,
	RES_K(100),
	0,
	RES_K(150),
	CAP_U(0.0047),
	CAP_U(0.0047),
	0,
	0,
	12,
	0
};


static const discrete_mixer_desc dplay_mixer =
{
	DISC_MIXER_IS_OP_AMP,
	{ RES_K(68),
	  RES_K(68),
	  RES_K(68),
	  RES_K(18),
	  RES_K(68) },
	{ 0,
	  0,
	  0,
	  0,
	  DPLAY_MUSIC_ADJ },
	{ CAP_U(0.1),
	  CAP_U(0.1),
	  CAP_U(0.1),
	  CAP_U(0.1),
	  CAP_U(0.1) }
	, 0, RES_K(100), 0, CAP_U(0.1), 0,
	2000	/* final gain */
};


static DISCRETE_SOUND_START(dplay)

	/************************************************
     * Input register mapping
     ************************************************/
	DISCRETE_INPUT_LOGIC (DPLAY_GAME_ON_EN)
	DISCRETE_INPUT_LOGIC (DPLAY_TONE_ON_EN)
	DISCRETE_INPUTX_LOGIC(DPLAY_SIREN_EN, 5, 0, 0)
	DISCRETE_INPUTX_LOGIC(DPLAY_WHISTLE_EN, 12, 0, 0)
	DISCRETE_INPUTX_LOGIC(DPLAY_CHEER_EN, 5, 0, 0)

	/* The low value of the pot is set to 1000.  A real 1M pot will never go to 0 anyways. */
	/* This will give the control more apparent volume range. */
	/* The music way overpowers the rest of the sounds anyways. */
	DISCRETE_ADJUSTMENT_TAG(DPLAY_MUSIC_ADJ, RES_M(1), 1000, DISC_LOGADJ, "MUSIC_ADJ")

	/************************************************
     * Music and Tone Generator
     ************************************************/
	MIDWAY_TONE_GENERATOR(dplay_music_tvca_info)

	DISCRETE_OP_AMP_TRIG_VCA(DPLAY_TONE_SND, MIDWAY_TONE_BEFORE_AMP_SND, DPLAY_TONE_ON_EN, 0, 12, 0, &dplay_music_tvca_info)

	/************************************************
     * Siren
     ************************************************/
	DISCRETE_INTEGRATE(NODE_30,
					DPLAY_SIREN_EN,					/* TRG0 */
					0			,					/* TRG1 */
					&dplay_siren_integrate_info)
	DISCRETE_OP_AMP_VCO1(DPLAY_SIREN_SND,
					1,								/* ENAB */
					NODE_30,						/* VMOD1 */
					&dplay_siren_osc)

	/************************************************
     * Whistle
     ************************************************/
	DISCRETE_INTEGRATE(NODE_40,
					DPLAY_WHISTLE_EN,				/* TRG0 */
					0			,					/* TRG1 */
					&dplay_whistle_integrate_info)
	DISCRETE_OP_AMP_VCO1(DPLAY_WHISTLE_SND,
					1,								/* ENAB */
					NODE_40,						/* VMOD1 */
					&dplay_whistle_osc)

	/************************************************ * Cheer
    ************************************************/ /* Noise clock was
    breadboarded and measured at 7700Hz */ DISCRETE_LFSR_NOISE(DPLAY_NOISE, 1,
	1, 7700, 12.0, 0, 12.0/2, &dplay_lfsr)

	DISCRETE_INTEGRATE(NODE_50, DPLAY_CHEER_EN, 0, &dplay_cheer_integrate_info)
	DISCRETE_SWITCH(NODE_51, 1, DPLAY_NOISE, 0, NODE_50)
	DISCRETE_OP_AMP_FILTER(DPLAY_CHEER_SND, 1, NODE_51, 0, DISC_OP_AMP_FILTER_IS_BAND_PASS_1M, &dplay_cheer_filter)

	/************************************************
     * Combine all sound sources.
     ************************************************/
	DISCRETE_MIXER5(NODE_91, DPLAY_GAME_ON_EN, DPLAY_TONE_SND, DPLAY_SIREN_SND, DPLAY_WHISTLE_SND, DPLAY_CHEER_SND, MIDWAY_TONE_SND, &dplay_mixer)

	DISCRETE_OUTPUT(NODE_91, 1)
DISCRETE_SOUND_END


MACHINE_DRIVER_START( dplay_audio )
	MDRV_SPEAKER_STANDARD_MONO("mono")
	MDRV_SOUND_ADD("discrete", DISCRETE, 0)
	MDRV_SOUND_CONFIG_DISCRETE(dplay)
	MDRV_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.8)
MACHINE_DRIVER_END


WRITE8_DEVICE_HANDLER( dplay_audio_w )
{
	discrete_sound_w(device, DPLAY_TONE_ON_EN, (data >> 0) & 0x01);

	discrete_sound_w(device, DPLAY_CHEER_EN, (data >> 1) & 0x01);

	discrete_sound_w(device, DPLAY_SIREN_EN, (data >> 2) & 0x01);

	discrete_sound_w(device, DPLAY_WHISTLE_EN, (data >> 3) & 0x01);

	discrete_sound_w(device, DPLAY_GAME_ON_EN, (data >> 4) & 0x01);

	coin_counter_w(0, (data >> 5) & 0x01);

	/* D6 and D7 are not connected */
}



/*************************************
 *
 *  Guided Missile
 *
 *************************************/

static const char *const gmissile_sample_names[] =
{
	"*gmissile",
	"1.wav",	/* missle */
	"2.wav",	/* explosion */
	0
};


static const samples_interface gmissile_samples_interface =
{
	1,	/* 1 channel */
	gmissile_sample_names
};


MACHINE_DRIVER_START( gmissile_audio )
	MDRV_SOUND_START(samples)

	MDRV_SPEAKER_STANDARD_STEREO("lspeaker", "rspeaker")

	MDRV_SOUND_ADD("samples1", SAMPLES, 0)
	MDRV_SOUND_CONFIG(gmissile_samples_interface)
	MDRV_SOUND_ROUTE(ALL_OUTPUTS, "lspeaker", 1.9)

	MDRV_SOUND_ADD("samples2", SAMPLES, 0)
	MDRV_SOUND_CONFIG(gmissile_samples_interface)
	MDRV_SOUND_ROUTE(ALL_OUTPUTS, "rspeaker", 1.9)
MACHINE_DRIVER_END


WRITE8_HANDLER( gmissile_audio_1_w )
{
	/* note that the schematics shows the left and right explosions
       reversed (D5=R, D7=L), but the software confirms that
       ours is right */

	const device_config *samples0 = devtag_get_device(space->machine, "samples1");
	const device_config *samples1 = devtag_get_device(space->machine, "samples2");
	UINT8 rising_bits = data & ~port_1_last;

	/* D0 and D1 are not connected */

	coin_counter_w(0, (data >> 2) & 0x01);

	sound_global_enable((data >> 3) & 0x01);

	/* if (data & 0x10)  enable RIGHT MISSILE sound (goes to right speaker) */
	if (rising_bits & 0x10) sample_start(samples1, 0, 0, 0);

	/* if (data & 0x20)  enable LEFT EXPLOSION sound (goes to left speaker) */
	output_set_value("L_EXP_LIGHT", (data >> 5) & 0x01);
	if (rising_bits & 0x20) sample_start(samples0, 0, 1, 0);

	/* if (data & 0x40)  enable LEFT MISSILE sound (goes to left speaker) */
	if (rising_bits & 0x40) sample_start(samples0, 0, 0, 0);

	/* if (data & 0x80)  enable RIGHT EXPLOSION sound (goes to right speaker) */
	output_set_value("R_EXP_LIGHT", (data >> 7) & 0x01);
	if (rising_bits & 0x80) sample_start(samples1, 0, 1, 0);

	port_1_last = data;
}


WRITE8_HANDLER( gmissile_audio_2_w )
{
	/* set AIRPLANE/COPTER/JET PAN(data & 0x07) */

	/* set TANK PAN((data >> 3) & 0x07) */

	/* D6 and D7 are not connected */
}


WRITE8_HANDLER( gmissile_audio_3_w )
{
	/* if (data & 0x01)  enable AIRPLANE (bi-plane) sound (goes to AIRPLANE/COPTER/JET panning circuit) */

	/* if (data & 0x02)  enable TANK sound (goes to TANK panning circuit) */

	/* if (data & 0x04)  enable COPTER sound (goes to AIRPLANE/COPTER/JET panning circuit) */

	/* D3 and D4 are not connected */

	/* if (data & 0x20)  enable JET (3 fighter jets) sound (goes to AIRPLANE/COPTER/JET panning circuit) */

	/* D6 and D7 are not connected */
}



/*************************************
 *
 *  M-4
 *
 *************************************/

/* Noise clock was breadboarded and measured at 3760Hz */

static const char *const m4_sample_names[] =
{
	"*m4",
	"1.wav",	/* missle */
	"2.wav",	/* explosion */
	0
};


static const samples_interface m4_samples_interface =
{
	2,	/* 2 channels */
	m4_sample_names
};


MACHINE_DRIVER_START( m4_audio )
	MDRV_SOUND_START(samples)

	MDRV_SPEAKER_STANDARD_STEREO("lspeaker", "rspeaker")

	MDRV_SOUND_ADD("samples1", SAMPLES, 0)
	MDRV_SOUND_CONFIG(m4_samples_interface)
	MDRV_SOUND_ROUTE(ALL_OUTPUTS, "lspeaker", 1)

	MDRV_SOUND_ADD("samples2", SAMPLES, 0)
	MDRV_SOUND_CONFIG(m4_samples_interface)
	MDRV_SOUND_ROUTE(ALL_OUTPUTS, "rspeaker", 1)
MACHINE_DRIVER_END


WRITE8_HANDLER( m4_audio_1_w )
{
	const device_config *samples0 = devtag_get_device(space->machine, "samples1");
	const device_config *samples1 = devtag_get_device(space->machine, "samples2");
	UINT8 rising_bits = data & ~port_1_last;

	/* D0 and D1 are not connected */

	coin_counter_w(0, (data >> 2) & 0x01);

	sound_global_enable((data >> 3) & 0x01);

	/* if (data & 0x10)  enable LEFT PLAYER SHOT sound (goes to left speaker) */
	if (rising_bits & 0x10) sample_start(samples0, 0, 0, 0);

	/* if (data & 0x20)  enable RIGHT PLAYER SHOT sound (goes to right speaker) */
	if (rising_bits & 0x20) sample_start(samples1, 0, 0, 0);

	/* if (data & 0x40)  enable LEFT PLAYER EXPLOSION sound via 300K res (goes to left speaker) */
	if (rising_bits & 0x40) sample_start(samples0, 1, 1, 0);

	/* if (data & 0x80)  enable RIGHT PLAYER EXPLOSION sound via 300K res (goes to right speaker) */
	if (rising_bits & 0x80) sample_start(samples1, 1, 1, 0);

	port_1_last = data;
}


WRITE8_HANDLER( m4_audio_2_w )
{
	const device_config *samples0 = devtag_get_device(space->machine, "samples1");
	const device_config *samples1 = devtag_get_device(space->machine, "samples2");
	UINT8 rising_bits = data & ~port_2_last;

	/* if (data & 0x01)  enable LEFT PLAYER EXPLOSION sound via 510K res (goes to left speaker) */
	if (rising_bits & 0x01) sample_start(samples0, 1, 1, 0);

	/* if (data & 0x02)  enable RIGHT PLAYER EXPLOSION sound via 510K res (goes to right speaker) */
	if (rising_bits & 0x02) sample_start(samples1, 1, 1, 0);

	/* if (data & 0x04)  enable LEFT TANK MOTOR sound (goes to left speaker) */

	/* if (data & 0x08)  enable RIGHT TANK MOTOR sound (goes to right speaker) */

	/* if (data & 0x10)  enable sound that is playing while the right plane is
                         flying.  Circuit not named on schematics  (goes to left speaker) */

	/* if (data & 0x20)  enable sound that is playing while the left plane is
                         flying.  Circuit not named on schematics  (goes to right speaker) */

	/* D6 and D7 are not connected */

	port_2_last = data;
}



/*************************************
 *
 *  Clowns
 *
 *  Discrete sound emulation: Mar 2005, D.R.
 *
 *************************************/

/* nodes - inputs */
#define CLOWNS_POP_BOTTOM_EN		NODE_01
#define CLOWNS_POP_MIDDLE_EN		NODE_02
#define CLOWNS_POP_TOP_EN			NODE_03
#define CLOWNS_SPRINGBOARD_HIT_EN	NODE_04
#define CLOWNS_SPRINGBOARD_MISS_EN	NODE_05

/* nodes - sounds */
#define CLOWNS_NOISE				NODE_06
#define CLOWNS_POP_SND				NODE_07
#define CLOWNS_SB_HIT_SND			NODE_08
#define CLOWNS_SB_MISS_SND			NODE_09

/* nodes - adjusters */
#define CLOWNS_MUSIC_ADJ			NODE_11


static const discrete_op_amp_tvca_info clowns_music_tvca_info =
{
	RES_M(3.3),				/* r502 */
	RES_K(10) + RES_K(680),	/* r505 + r506 */
	0,
	RES_K(680),				/* r503 */
	RES_K(10),				/* r500 */
	0,
	RES_K(680),				/* r501 */
	0,
	0,
	0,
	0,
	CAP_U(.001),			/* c500 */
	0,
	0,
	12,			/* v1 */
	0,			/* v2 */
	0,			/* v3 */
	12,			/* vP */
	DISC_OP_AMP_TRIGGER_FUNCTION_TRG0,
	DISC_OP_AMP_TRIGGER_FUNCTION_NONE,
	DISC_OP_AMP_TRIGGER_FUNCTION_TRG1,
	DISC_OP_AMP_TRIGGER_FUNCTION_NONE,
	DISC_OP_AMP_TRIGGER_FUNCTION_NONE,
	DISC_OP_AMP_TRIGGER_FUNCTION_NONE
};


static const discrete_op_amp_tvca_info clowns_pop_tvca_info =
{
	RES_M(2.7),		/* r304 */
	RES_K(680),		/* r303 */
	0,
	RES_K(680),		/* r305 */
	RES_K(1),		/* j3 */
	0,
	RES_K(470),		/* r300 */
	RES_K(1),		/* j3 */
	RES_K(510),		/* r301 */
	RES_K(1),		/* j3 */
	RES_K(680),		/* r302 */
	CAP_U(.015),	/* c300 */
	CAP_U(.1),		/* c301 */
	CAP_U(.082),	/* c302 */
	5,			/* v1 */
	5,			/* v2 */
	5,			/* v3 */
	12,			/* vP */
	DISC_OP_AMP_TRIGGER_FUNCTION_NONE,
	DISC_OP_AMP_TRIGGER_FUNCTION_NONE,
	DISC_OP_AMP_TRIGGER_FUNCTION_TRG0,
	DISC_OP_AMP_TRIGGER_FUNCTION_NONE,
	DISC_OP_AMP_TRIGGER_FUNCTION_TRG1,
	DISC_OP_AMP_TRIGGER_FUNCTION_TRG2
};


static const discrete_op_amp_osc_info clowns_sb_hit_osc_info =
{
	DISC_OP_AMP_OSCILLATOR_1 | DISC_OP_AMP_IS_NORTON | DISC_OP_AMP_OSCILLATOR_OUT_SQW,
	RES_K(820),		/* r200 */
	RES_K(33),		/* r203 */
	RES_K(150),		/* r201 */
	RES_K(240),		/* r204 */
	RES_M(1),		/* r202 */
	0,
	0,
	0,
	CAP_U(0.01),	/* c200 */
	12
};


static const discrete_op_amp_tvca_info clowns_sb_hit_tvca_info =
{
	RES_M(2.7),		/* r207 */
	RES_K(680),		/* r205 */
	0,
	RES_K(680),		/* r208 */
	RES_K(1),		/* j3 */
	0,
	RES_K(680),		/* r206 */
	0,0,0,0,
	CAP_U(1),		/* c201 */
	0,
	0,
	5,			/* v1 */
	0,			/* v2 */
	0,			/* v3 */
	12,			/* vP */
	DISC_OP_AMP_TRIGGER_FUNCTION_NONE,
	DISC_OP_AMP_TRIGGER_FUNCTION_NONE,
	DISC_OP_AMP_TRIGGER_FUNCTION_TRG0,
	DISC_OP_AMP_TRIGGER_FUNCTION_NONE,
	DISC_OP_AMP_TRIGGER_FUNCTION_NONE,
	DISC_OP_AMP_TRIGGER_FUNCTION_NONE
};


static const discrete_mixer_desc clowns_mixer =
{
	DISC_MIXER_IS_OP_AMP,
	{ RES_K(10),
	  RES_K(10),
	  RES_K(10) + 1.0 / (1.0 / RES_K(15) + 1.0 / RES_K(39)),
	  RES_K(1) },
	{ 0,
	  0,
	  0,
	  CLOWNS_MUSIC_ADJ },
	{ 0,
	  CAP_U(0.022),
	  0,
	  0 },
	0,
	RES_K(100),
	0,
	CAP_U(1),
	0,
	1
};


static DISCRETE_SOUND_START(clowns)

	/************************************************
     * Input register mapping
     ************************************************/
	DISCRETE_INPUT_LOGIC(CLOWNS_POP_BOTTOM_EN)
	DISCRETE_INPUT_LOGIC(CLOWNS_POP_MIDDLE_EN)
	DISCRETE_INPUT_LOGIC(CLOWNS_POP_TOP_EN)
	DISCRETE_INPUT_LOGIC(CLOWNS_SPRINGBOARD_HIT_EN)
	DISCRETE_INPUT_LOGIC(CLOWNS_SPRINGBOARD_MISS_EN)

	/* The low value of the pot is set to 7000.  A real 1M pot will never go to 0 anyways. */
	/* This will give the control more apparent volume range. */
	/* The music way overpowers the rest of the sounds anyways. */
	DISCRETE_ADJUSTMENT_TAG(CLOWNS_MUSIC_ADJ, RES_M(1), 7000, DISC_LOGADJ, "MUSIC_ADJ")

	/************************************************
     * Tone generator
     ************************************************/
	MIDWAY_TONE_GENERATOR(clowns_music_tvca_info)

	/************************************************
     * Balloon hit sounds
     * The LFSR is the same as boothill
     ************************************************/
	/* Noise clock was breadboarded and measured at 7700Hz */
	DISCRETE_LFSR_NOISE(CLOWNS_NOISE, 1, 1, 7700, 12.0, 0, 12.0/2, &midway_lfsr)

	DISCRETE_OP_AMP_TRIG_VCA(NODE_30, CLOWNS_POP_TOP_EN, CLOWNS_POP_MIDDLE_EN, CLOWNS_POP_BOTTOM_EN, CLOWNS_NOISE, 0, &clowns_pop_tvca_info)
	DISCRETE_RCFILTER(NODE_31, 1, NODE_30, RES_K(15), CAP_U(.01))
	DISCRETE_CRFILTER(NODE_32, 1, NODE_31, RES_K(15) + RES_K(39), CAP_U(.01))
	DISCRETE_GAIN(CLOWNS_POP_SND, NODE_32, RES_K(39)/(RES_K(15) + RES_K(39)))

	/************************************************
     * Springboard hit
     ************************************************/
	DISCRETE_OP_AMP_OSCILLATOR(NODE_40, 1, &clowns_sb_hit_osc_info)
	DISCRETE_OP_AMP_TRIG_VCA(NODE_41, CLOWNS_SPRINGBOARD_HIT_EN, 0, 0, NODE_40, 0, &clowns_sb_hit_tvca_info)
	/* The rest of the circuit is a filter.  The frequency response was calculated with SPICE. */
	DISCRETE_FILTER2(NODE_42, 1, NODE_41, 500, 1.0/.8, DISC_FILTER_LOWPASS)
	/* The filter has a gain of 0.5 */
	DISCRETE_GAIN(CLOWNS_SB_HIT_SND, NODE_42, 0.5)

	/************************************************
     * Springboard miss - INCOMPLETE
     ************************************************/
	DISCRETE_CONSTANT(CLOWNS_SB_MISS_SND, 0) /* Placeholder for incomplete sound */

	/************************************************
     * Combine all sound sources.
     ************************************************/
	DISCRETE_MIXER4(NODE_91, 1, CLOWNS_SB_HIT_SND, CLOWNS_SB_MISS_SND, CLOWNS_POP_SND, MIDWAY_TONE_SND, &clowns_mixer)

	DISCRETE_OUTPUT(NODE_91, 11000)
DISCRETE_SOUND_END


static const char *const clowns_sample_names[] =
{
	"*clowns",
	"miss.wav",
	0
};

static const samples_interface clowns_samples_interface =
{
	1,	/* 1 channel */
	clowns_sample_names
};


MACHINE_DRIVER_START( clowns_audio )
	MDRV_SOUND_START(samples)

	MDRV_SPEAKER_STANDARD_MONO("mono")

	MDRV_SOUND_ADD("samples", SAMPLES, 0)
	MDRV_SOUND_CONFIG(clowns_samples_interface)
	MDRV_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.70)

	MDRV_SOUND_ADD("discrete", DISCRETE, 0)
	MDRV_SOUND_CONFIG_DISCRETE(clowns)
	MDRV_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.25)
MACHINE_DRIVER_END


WRITE8_HANDLER( clowns_audio_1_w )
{
	coin_counter_w(0, (data >> 0) & 0x01);

	clowns_set_controller_select((data >> 1) & 0x01);

	/* D2-D7 are not connected */
}


WRITE8_DEVICE_HANDLER( clowns_audio_2_w )
{
	const device_config *samples = devtag_get_device(device->machine, "samples");
	UINT8 rising_bits = data & ~port_2_last;

	discrete_sound_w(device, CLOWNS_POP_BOTTOM_EN, (data >> 0) & 0x01);

	discrete_sound_w(device, CLOWNS_POP_MIDDLE_EN, (data >> 1) & 0x01);

	discrete_sound_w(device, CLOWNS_POP_TOP_EN, (data >> 2) & 0x01);

	sound_global_enable((data >> 3) & 0x01);

	discrete_sound_w(device, CLOWNS_SPRINGBOARD_HIT_EN, (data >> 4) & 0x01);

	if (rising_bits & 0x20) sample_start(samples, 0, 0, 0);  /* springboard miss */

	/* D6 and D7 are not connected */

	port_2_last = data;
}



/*************************************
 *
 *  Space Walk
 *
 *************************************/

MACHINE_DRIVER_START( spacwalk_audio )
	MDRV_SPEAKER_STANDARD_MONO("mono")
MACHINE_DRIVER_END

WRITE8_HANDLER( spacwalk_audio_1_w )
{
	coin_counter_w(0, (data >> 0) & 0x01);

	clowns_set_controller_select((data >> 1) & 0x01);

	// D2: ?
	// D3: ?
	// D4-7: unused?
	// if (data&0xfc) printf("%02x ",data);
}



/*************************************
 *
 *  Shuffleboard
 *
 *************************************/

/* Noise clock was breadboarded and measured at 1210Hz */


MACHINE_DRIVER_START( shuffle_audio )
	MDRV_SPEAKER_STANDARD_MONO("mono")
MACHINE_DRIVER_END


WRITE8_HANDLER( shuffle_audio_1_w )
{
	/* if (data & 0x01)  enable CLICK (balls collide) sound */

	/* if (data & 0x02)  enable SHUFFLE ROLLOVER sound */

	sound_global_enable((data >> 2) & 0x01);

	/* set SHUFFLE ROLLING sound((data >> 3) & 0x07)  0, if not rolling,
                                                      faster rolling = higher number */

	/* D6 and D7 are not connected */
}


WRITE8_HANDLER( shuffle_audio_2_w )
{
	/* if (data & 0x01)  enable FOUL sound */

	coin_counter_w(0, (data >> 1) & 0x01);

	/* D2-D7 are not connected */
}



/*************************************
 *
 *  Dog Patch
 *
 *  We don't have the schematics, so this is all questionable.
 *  This game is most likely stereo as well.
 *
 *************************************/

static const discrete_op_amp_tvca_info dogpatch_music_tvca_info =
{
	RES_M(3.3),
	RES_K(10) + RES_K(680),
	0,
	RES_K(680),
	RES_K(10),
	0,
	RES_K(680),
	0,
	0,
	0,
	0,
	CAP_U(.001),
	0,
	0,
	12,			/* v1 */
	0,			/* v2 */
	0,			/* v3 */
	12,			/* vP */
	DISC_OP_AMP_TRIGGER_FUNCTION_TRG0,
	DISC_OP_AMP_TRIGGER_FUNCTION_NONE,
	DISC_OP_AMP_TRIGGER_FUNCTION_TRG1,
	DISC_OP_AMP_TRIGGER_FUNCTION_NONE,
	DISC_OP_AMP_TRIGGER_FUNCTION_NONE,
	DISC_OP_AMP_TRIGGER_FUNCTION_NONE
};


static DISCRETE_SOUND_START(dogpatch)

	/************************************************
     * Tone generator
     ************************************************/
	MIDWAY_TONE_GENERATOR(dogpatch_music_tvca_info)

	/************************************************
     * Filter it to be AC.
     ************************************************/
	DISCRETE_CRFILTER(NODE_91, 1, MIDWAY_TONE_SND, RES_K(100), CAP_U(0.1))

	DISCRETE_OUTPUT(NODE_91, 5000)

DISCRETE_SOUND_END


MACHINE_DRIVER_START( dogpatch_audio )
	MDRV_SPEAKER_STANDARD_MONO("mono")
	MDRV_SOUND_ADD("discrete", DISCRETE, 0)
	MDRV_SOUND_CONFIG_DISCRETE(dogpatch)
	MDRV_SOUND_ROUTE(ALL_OUTPUTS, "mono", 1.3)
MACHINE_DRIVER_END


WRITE8_HANDLER( dogpatch_audio_w )
{
	/* D0 and D1 are most likely not used */

	coin_counter_w(0, (data >> 2) & 0x01);

	sound_global_enable((data >> 3) & 0x01);

	/* if (data & 0x10)  enable LEFT SHOOT sound */

	/* if (data & 0x20)  enable RIGHT SHOOT sound */

	/* if (data & 0x40)  enable CAN HIT sound */

	/* D7 is most likely not used */
}



/*************************************
 *
 *  Space Encounters
 *
 *  Discrete sound emulation:
 *  Apr 2007, D.R.
 *************************************/

static const sn76477_interface spcenctr_sn76477_interface =
{
	0,				/*  4 noise_res (N/C)        */
	0,				/*  5 filter_res (N/C)       */
	0,				/*  6 filter_cap (N/C)       */
	0,				/*  7 decay_res (N/C)        */
	0,				/*  8 attack_decay_cap (N/C) */
	RES_K(100), 	/* 10 attack_res             */
	RES_K(56),		/* 11 amplitude_res          */
	RES_K(10),		/* 12 feedback_res           */
	0,				/* 16 vco_voltage (N/C)      */
	CAP_U(0.047),	/* 17 vco_cap                */
	RES_K(56),		/* 18 vco_res                */
	5.0,			/* 19 pitch_voltage          */
	RES_K(150),		/* 20 slf_res                */
	CAP_U(1.0),		/* 21 slf_cap                */
	0,				/* 23 oneshot_cap (N/C)      */
	0,				/* 24 oneshot_res (N/C)      */
	1,				/* 22 vco                    */
	0,				/* 26 mixer A                */
	0,				/* 25 mixer B                */
	0,				/* 27 mixer C                */
	1,				/* 1  envelope 1             */
	0,				/* 28 envelope 2             */
	1				/* 9  enable (variable)      */
};


/* nodes - inputs */
#define SPCENCTR_ENEMY_SHIP_SHOT_EN		NODE_01
#define SPCENCTR_PLAYER_SHOT_EN			NODE_02
#define SPCENCTR_SCREECH_EN				NODE_03
#define SPCENCTR_CRASH_EN				NODE_04
#define SPCENCTR_EXPLOSION_EN			NODE_05
#define SPCENCTR_BONUS_EN				NODE_06
#define SPCENCTR_WIND_DATA				NODE_07

/* nodes - sounds */
#define SPCENCTR_NOISE					NODE_10
#define SPCENCTR_ENEMY_SHIP_SHOT_SND	NODE_11
#define SPCENCTR_PLAYER_SHOT_SND		NODE_12
#define SPCENCTR_SCREECH_SND			NODE_13
#define SPCENCTR_CRASH_SND				NODE_14
#define SPCENCTR_EXPLOSION_SND			NODE_15
#define SPCENCTR_BONUS_SND				NODE_16
#define SPCENCTR_WIND_SND				NODE_17


static const discrete_op_amp_info spcenctr_enemy_ship_shot_op_amp_E1 =
{
	DISC_OP_AMP_IS_NORTON,
	0,						/* no r1 */
	RES_K(510),				/* R100 */
	RES_M(2.2),				/* R101 */
	RES_M(2.2),				/* R102 */
	CAP_U(0.1),				/* C100 */
	0,						/* vN */
	12						/* vP */
};


static const discrete_op_amp_osc_info spcenctr_enemy_ship_shot_op_amp_osc =
{
	DISC_OP_AMP_OSCILLATOR_VCO_3 | DISC_OP_AMP_IS_NORTON | DISC_OP_AMP_OSCILLATOR_OUT_SQW,
	RES_K(560),		/* R103 */
	RES_K(7.5),		/* R118 */
	RES_K(22),		/* R104 */
	RES_K(47),		/* R106 */
	RES_K(100),		/* R105 */
	0,				/* no r6 */
	0,				/* no r7 */
	0,				/* no r8 */
	CAP_U(0.0022),	/* C101 */
	12,				/* vP */
};


static const discrete_op_amp_info spcenctr_enemy_ship_shot_op_amp_D1 =
{
	DISC_OP_AMP_IS_NORTON,
	RES_K(100),				/* R107 */
	RES_K(100),				/* R109 */
	RES_M(2.7),				/* R108 */
	RES_K(100),				/* R110 */
	0,						/* no c */
	0,						/* vN */
	12						/* vP */
};


static const discrete_op_amp_filt_info spcenctr_enemy_ship_shot_filt =
{
	RES_K(100),		/* R112 */
	RES_K(10),		/* R113 */
	RES_M(4.3),		/* r3 */
	0,				/* no r4 */
	RES_M(2.2),		/* R114 */
	CAP_U(0.001),	/* c1 */
	CAP_U(0.001),	/* c2 */
	0,				/* no c3 */
	0,				/* vRef */
	12,				/* vP */
	0				/* vN */
};


static const discrete_op_amp_1sht_info spcenctr_player_shot_1sht =
{
	DISC_OP_AMP_1SHT_1 | DISC_OP_AMP_IS_NORTON,
	RES_M(4.7),		/* R500 */
	RES_K(100),		/* R502 */
	RES_M(1),		/* R501 */
	RES_M(1),		/* R503 */
	RES_M(2.2),		/* R504 */
	CAP_U(1),		/* C500 */
	CAP_P(470),		/* C501 */
	0,				/* vN */
	12				/* vP */
};


static const discrete_op_amp_info spcenctr_player_shot_op_amp_E1 =
{
	DISC_OP_AMP_IS_NORTON,
	0,				/* no r1 */
	RES_K(10),		/* R505 */
	RES_M(1.5),		/* R506 */
	0,				/* no r4 */
	CAP_U(0.22),	/* C502 */
	0,				/* vN */
	12				/* vP */
};


static const discrete_op_amp_osc_info spcenctr_player_shot_op_amp_osc =
{
	DISC_OP_AMP_OSCILLATOR_VCO_3 | DISC_OP_AMP_IS_NORTON | DISC_OP_AMP_OSCILLATOR_OUT_SQW,
	1.0 / (1.0 / RES_M(1) + 1.0 / RES_K(330)) + RES_M(1.5),		/* R507||R509 + R508 */
	RES_M(1),		/* R513 */
	RES_K(560),		/* R512 */
	RES_M(2.7),		/* R516 */
	RES_M(1),		/* R515 */
	RES_M(4.7),		/* R510 */
	RES_M(3.3),		/* R511 */
	0,				/* no r8 */
	CAP_P(330),		/* C504 */
	12,				/* vP */
};


static const discrete_op_amp_info spcenctr_player_shot_op_amp_C1 =
{
	DISC_OP_AMP_IS_NORTON,
	RES_K(560),		/* R517 */
	RES_K(470),		/* R514 */
	RES_M(2.7),		/* R518 */
	RES_K(560),		/* R524 */
	0,				/* no c */
	0,				/* vN */
	12				/* vP */
};


static const discrete_op_amp_tvca_info spcenctr_player_shot_tvca =
{
	RES_M(2.7),							/* R522 */
	RES_K(560),							/* R521 */
	0,									/* no r3 */
	RES_K(560),							/* R560 */
	RES_K(1),							/* R42 */
	0,									/* no r6 */
	RES_K(560),							/* R523 */
	0,									/* no r8 */
	0,									/* no r9 */
	0,									/* no r10 */
	0,									/* no r11 */
	CAP_U(1),							/* C506 */
	0,									/* no c2 */
	0,									/* no c3 */
	12,									/* v1 */
	0,									/* no v2 */
	0,									/* no v3 */
	12,									/* vP */
	DISC_OP_AMP_TRIGGER_FUNCTION_NONE,	/* no f0 */
	DISC_OP_AMP_TRIGGER_FUNCTION_NONE,	/* no f1 */
	DISC_OP_AMP_TRIGGER_FUNCTION_TRG0,	/* f2 */
	DISC_OP_AMP_TRIGGER_FUNCTION_NONE,	/* no f3 */
	DISC_OP_AMP_TRIGGER_FUNCTION_NONE,	/* no f4 */
	DISC_OP_AMP_TRIGGER_FUNCTION_NONE	/* no f5 */
};


static const discrete_op_amp_tvca_info spcenctr_crash_tvca =
{
	RES_M(2.7),							/* R302 */
	RES_K(470),							/* R300 */
	0,									/* no r3 */
	RES_K(470),							/* R303 */
	RES_K(1),							/* R56 */
	0,									/* no r6 */
	RES_K(470),							/* R301 */
	0,									/* no r8 */
	0,									/* no r9 */
	0,									/* no r10 */
	0,									/* no r11 */
	CAP_U(2.2),							/* C304 */
	0,									/* no c2 */
	0,									/* no c3 */
	5,									/* v1 */
	0,									/* no v2 */
	0,									/* no v3 */
	12,									/* vP */
	DISC_OP_AMP_TRIGGER_FUNCTION_NONE,	/* no f0 */
	DISC_OP_AMP_TRIGGER_FUNCTION_NONE,	/* no f1 */
	DISC_OP_AMP_TRIGGER_FUNCTION_TRG0,	/* f2 */
	DISC_OP_AMP_TRIGGER_FUNCTION_NONE,	/* no f3 */
	DISC_OP_AMP_TRIGGER_FUNCTION_NONE,	/* no f4 */
	DISC_OP_AMP_TRIGGER_FUNCTION_NONE	/* no f5 */
};


static const discrete_op_amp_tvca_info spcenctr_explosion_tvca =
{
	RES_M(2.7),							/* R402 */
	RES_K(680),							/* R400 */
	0,									/* no r3 */
	RES_K(680),							/* R403 */
	RES_K(1),							/* R41 */
	0,									/* no r6 */
	RES_K(680),							/* R401 */
	0,									/* no r8 */
	0,									/* no r9 */
	0,									/* no r10 */
	0,									/* no r11 */
	CAP_U(2.2),							/* C400 */
	0,									/* no c2 */
	0,									/* no c3 */
	12,									/* v1 */
	0,									/* no v2 */
	0,									/* no v3 */
	12,									/* vP */
	DISC_OP_AMP_TRIGGER_FUNCTION_NONE,	/* no f0 */
	DISC_OP_AMP_TRIGGER_FUNCTION_NONE,	/* no f1 */
	DISC_OP_AMP_TRIGGER_FUNCTION_TRG0,	/* f2 */
	DISC_OP_AMP_TRIGGER_FUNCTION_NONE,	/* no f3 */
	DISC_OP_AMP_TRIGGER_FUNCTION_NONE,	/* no f4 */
	DISC_OP_AMP_TRIGGER_FUNCTION_NONE	/* no f5 */
};


static const discrete_555_desc spcenctr_555_bonus =
{
	DISC_555_OUT_SQW | DISC_555_OUT_DC,
	5,				/* B+ voltage of 555 */
	DEFAULT_555_VALUES
};


static const discrete_mixer_desc spcenctr_mixer =
{
	DISC_MIXER_IS_RESISTOR,		/* type */
	{ RES_K(15),				/* R117 */
	  RES_K(15),				/* R526 */
	  RES_K(22),				/* R211 */
	  RES_K(3.6),				/* R309 */
	  RES_K(1.8) +  RES_K(3.6) + RES_K(4.7),	/* R405 + R406 + R407 */
	  RES_K(27),				/* R715 */
	  RES_K(27)},				/* R51 */
	{0},						/* no rNode{} */
	{ 0,
	  CAP_U(0.001),				/* C505 */
	  CAP_U(0.1),				/* C202 */
	  CAP_U(1),					/* C303 */
	  0,
	  0,
	  CAP_U(10)},				/* C16 */
	0,							/* no rI */
	0,							/* no rF */
	0,							/* no cF */
	CAP_U(1),					/* C900 */
	0,							/* vRef = ground */
	1							/* gain */
};


static DISCRETE_SOUND_START(spcenctr)

	/************************************************
     * Input register mapping
     ************************************************/
	DISCRETE_INPUTX_LOGIC(SPCENCTR_ENEMY_SHIP_SHOT_EN, 12, 0, 0)
	DISCRETE_INPUTX_LOGIC(SPCENCTR_PLAYER_SHOT_EN, 12, 0, 0)
	DISCRETE_INPUT_LOGIC (SPCENCTR_SCREECH_EN)
	DISCRETE_INPUT_LOGIC (SPCENCTR_CRASH_EN)
	DISCRETE_INPUT_LOGIC (SPCENCTR_EXPLOSION_EN)
	DISCRETE_INPUT_LOGIC (SPCENCTR_BONUS_EN)
	DISCRETE_INPUT_DATA  (SPCENCTR_WIND_DATA)


	/************************************************
     * Noise Generator
     ************************************************/
	/* Noise clock was breadboarded and measured at 7515 */
	DISCRETE_LFSR_NOISE(SPCENCTR_NOISE,			/* IC A0, pin 10 */
					1,							/* ENAB */
					1,							/* no RESET */
					7515,						/* CLK in Hz */
					12,							/* p-p AMPL */
					0,							/* no FEED input */
					12.0/2,						/* dc BIAS */
					&midway_lfsr)


	/************************************************
     * Enemy Ship Shot
     ************************************************/
	DISCRETE_OP_AMP(NODE_20,							/* IC E1, pin 10 */
					1,									/* ENAB */
					0,									/* no IN0 */
					SPCENCTR_ENEMY_SHIP_SHOT_EN,		/* IN1 */
					&spcenctr_enemy_ship_shot_op_amp_E1)
	DISCRETE_OP_AMP_VCO1(NODE_21,						/* IC D1, pin 5 */
					1,									/* ENAB */
					NODE_20,							/* VMOD1 */
					&spcenctr_enemy_ship_shot_op_amp_osc)
	DISCRETE_OP_AMP(NODE_22,							/* IC D1, pin 9 */
					1,									/* ENAB */
					NODE_21,							/* IN0 */
					NODE_20,							/* IN1 */
					&spcenctr_enemy_ship_shot_op_amp_D1)
	DISCRETE_OP_AMP_FILTER(NODE_23,						/* IC D1, pin 10 */
					1,									/* ENAB */
					NODE_22,							/* INP0 */
					0,									/* no INP1 */
					DISC_OP_AMP_FILTER_IS_BAND_PASS_1M | DISC_OP_AMP_IS_NORTON,
					&spcenctr_enemy_ship_shot_filt)
	DISCRETE_CRFILTER(SPCENCTR_ENEMY_SHIP_SHOT_SND,
					1,									/* ENAB */
					NODE_23,							/* IN0 */
					RES_K(1.8),							/* R116 */
					CAP_U(0.1) )						/* C104 */


	/************************************************
     * Player Shot
     ************************************************/
	DISCRETE_OP_AMP_ONESHOT(NODE_30,					/* IC E1, pin 4 */
					SPCENCTR_PLAYER_SHOT_EN,			/* TRIG */
					&spcenctr_player_shot_1sht)			/* breadboarded and scoped at 325mS */
	DISCRETE_OP_AMP(NODE_31,							/* IC E1, pin 5 */
					1,									/* ENAB */
					0,									/* no IN0 */
					NODE_30,							/* IN1 */
					&spcenctr_player_shot_op_amp_E1)
	/* next 2 modules simulate the D502 voltage drop */
	DISCRETE_ADDER2(NODE_32,
					1,									/* ENAB */
					NODE_31,							/* IN0 */
					-0.5)								/* IN1 */
	DISCRETE_CLAMP(NODE_33,
					1,									/* ENAB */
					NODE_32,							/* IN0 */
					0,									/* MIN */
					12,									/* MAX */
					0)									/* CLAMP */
	DISCRETE_CRFILTER(NODE_34,
					1,									/* ENAB */
					SPCENCTR_NOISE,						/* IN0 */
					RES_M(1) + RES_K(330),				/* R507, R509 */
					CAP_U(0.1) )						/* C503 */
	DISCRETE_GAIN(NODE_35,
					NODE_34,							/* IN0 */
					RES_K(330)/(RES_M(1) + RES_K(330)))	/* GAIN - R507 : R509 */
	DISCRETE_OP_AMP_VCO2(NODE_36,						/* IC C1, pin 4 */
					1,									/* ENAB */
					NODE_35,							/* VMOD1 */
					NODE_33,							/* VMOD2 */
					&spcenctr_player_shot_op_amp_osc)
	DISCRETE_OP_AMP(NODE_37,							/* IC C1, pin 9 */
					1,									/* ENAB */
					NODE_36,							/* IN0 */
					NODE_33,							/* IN1 */
					&spcenctr_player_shot_op_amp_C1)
	DISCRETE_OP_AMP_TRIG_VCA(SPCENCTR_PLAYER_SHOT_SND,	/* IC C1, pin 10 */
					SPCENCTR_PLAYER_SHOT_EN,			/* TRG0 */
					0,									/* no TRG1 */
					0,									/* no TRG2 */
					NODE_37,							/* IN0 */
					0,									/* no IN1 */
					&spcenctr_player_shot_tvca)


	/************************************************
     *Screech - unemulated
     ************************************************/
	DISCRETE_CONSTANT(SPCENCTR_SCREECH_SND, 0)


	/************************************************
     * Crash
     ************************************************/
	DISCRETE_OP_AMP_TRIG_VCA(NODE_60,			/* IC C2, pin 4 */
					SPCENCTR_CRASH_EN,			/* TRG0 */
					0,							/* no TRG1 */
					0,							/* no TRG2 */
					SPCENCTR_NOISE,				/* IN0 */
					0,							/* no IN1 */
					&spcenctr_crash_tvca)
	/* The next 5 modules emulate the filter. */
	/* The DC level was breadboarded and the frequency response was SPICEd */
	DISCRETE_ADDER2(NODE_61,					/* center on filter DC level */
					1,							/* ENAB */
					NODE_60,					/* IN0 */
					-6.8)						/* IN1 */
	DISCRETE_FILTER2(NODE_62,
					1,							/* ENAB */
					NODE_61,					/* INP0 */
					130,						/* FREQ */
					1.0 / 8,					/* DAMP */
					DISC_FILTER_BANDPASS)
	DISCRETE_GAIN(NODE_63,
					NODE_62,					/* IN0 */
					6)							/* GAIN */
	DISCRETE_ADDER2(NODE_64,					/* center on filter DC level */
					1,							/* ENAB */
					NODE_63,					/* IN0 */
					6.8)						/* IN1 */
	DISCRETE_CLAMP(SPCENCTR_CRASH_SND,			/* IC C2, pin 5 */
					1,							/* ENAB */
					NODE_64,					/* IN0 */
					0,							/* MIN */
					12.0 - OP_AMP_NORTON_VBE,	/* MAX */
					0)							/* CLAMP */


	/************************************************
     * Explosion
     ************************************************/
	DISCRETE_OP_AMP_TRIG_VCA(NODE_70,			/* IC D2, pin 10 */
					SPCENCTR_EXPLOSION_EN,		/* TRG0 */
					0,							/* no TRG1 */
					0,							/* no TRG2 */
					SPCENCTR_NOISE,				/* IN0 */
					0,							/* no IN1 */
					&spcenctr_explosion_tvca)
	DISCRETE_RCFILTER(NODE_71,
					1,							/* ENAB */
					NODE_70,					/* IN0 */
					RES_K(1.8),					/* R405 */
					CAP_U(0.22) )				/* C401 */
	DISCRETE_RCFILTER(SPCENCTR_EXPLOSION_SND,
					1,							/* ENAB */
					NODE_71,					/* IN0 */
					RES_K(1.8) + RES_K(3.6),	/* R405 + R406 */
					CAP_U(0.22) )				/* C402 */


	/************************************************
     *Bonus
     ************************************************/
	DISCRETE_555_ASTABLE(NODE_80,				/* pin 5 */
					/* the pin 4 reset is not connected in schematic, but should be */
					SPCENCTR_BONUS_EN,			/* RESET */
					RES_K(1),					/* R710 */
					RES_K(27),					/* R711 */
					CAP_U(0.047),				/* C710 */
					&spcenctr_555_bonus)
	DISCRETE_555_ASTABLE(NODE_81,				/* pin 9 */
					SPCENCTR_BONUS_EN,			/* RESET pin 10 */
					RES_K(100),					/* R713 */
					RES_K(47),					/* R714 */
					CAP_U(1),					/* C713 */
					&spcenctr_555_bonus)
	DISCRETE_LOGIC_AND3(NODE_82,				/* IC C-D, pin 6 */
					1,							/* ENAB */
					NODE_80,					/* INP0 */
					NODE_81,					/* INP1 */
					SPCENCTR_BONUS_EN)			/* INP2 */
	DISCRETE_GAIN(SPCENCTR_BONUS_SND,			/* adjust from logic to TTL voltage level */
					NODE_82,					/* IN0 */
					DEFAULT_TTL_V_LOGIC_1)		/* GAIN */


	/************************************************
     *Wind - unemulated
     ************************************************/
	DISCRETE_CONSTANT(SPCENCTR_WIND_SND, 0)


	/************************************************
     * Final mix
     ************************************************/
	DISCRETE_MIXER7(NODE_91,
					1,								/* ENAB */
					SPCENCTR_ENEMY_SHIP_SHOT_SND,	/* IN0 */
					SPCENCTR_PLAYER_SHOT_SND,		/* IN1 */
					SPCENCTR_SCREECH_SND,			/* IN2 */
					SPCENCTR_CRASH_SND,				/* IN3 */
					SPCENCTR_EXPLOSION_SND,			/* IN4 */
					SPCENCTR_BONUS_SND,				/* IN5 */
					SPCENCTR_WIND_SND,				/* IN6 */
					&spcenctr_mixer)

	DISCRETE_OUTPUT(NODE_91, 20000)
DISCRETE_SOUND_END


MACHINE_DRIVER_START( spcenctr_audio )
	MDRV_SPEAKER_STANDARD_MONO("mono")

	MDRV_SOUND_ADD("sn", SN76477, 0)
	MDRV_SOUND_CONFIG(spcenctr_sn76477_interface)
	MDRV_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.20)

	MDRV_SOUND_ADD("discrete", DISCRETE, 0)
	MDRV_SOUND_CONFIG_DISCRETE(spcenctr)
	MDRV_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.45)
MACHINE_DRIVER_END



WRITE8_DEVICE_HANDLER( spcenctr_audio_1_w )
{
	sound_global_enable((data >> 0) & 0x01);

	/* D1 is marked as 'OPTIONAL SWITCH VIDEO FOR COCKTAIL',
       but it is never set by the software */

	discrete_sound_w(device, SPCENCTR_CRASH_EN, (data >> 2) & 0x01);

	/* D3-D7 are not connected */
}


WRITE8_DEVICE_HANDLER( spcenctr_audio_2_w )
{
	/* set WIND SOUND FREQ(data & 0x0f)  0, if no wind */

	discrete_sound_w(device, SPCENCTR_EXPLOSION_EN, (data >> 4) & 0x01);

	discrete_sound_w(device, SPCENCTR_PLAYER_SHOT_EN, (data >> 5) & 0x01);

	/* D6 and D7 are not connected */

	port_2_last = data;
}


WRITE8_DEVICE_HANDLER( spcenctr_audio_3_w )
{
	const device_config *sn = devtag_get_device(device->machine, "sn");

	/* if (data & 0x01)  enable SCREECH (hit the sides) sound */

	discrete_sound_w(device, SPCENCTR_ENEMY_SHIP_SHOT_EN, (data >> 1) & 0x01);

	spcenctr_set_strobe_state((data >> 2) & 0x01);

	output_set_value("LAMP", (data >> 3) & 0x01);

	discrete_sound_w(device, SPCENCTR_BONUS_EN, (data >> 4) & 0x01);

	sn76477_enable_w(sn, (data >> 5) & 0x01);	/* saucer sound */

	/* D6 and D7 are not connected */
}



/*************************************
 *
 *  Phantom II
 *
 *************************************/

static const char *const phantom2_sample_names[] =
{
	"*phantom2",
	"1.wav",	/* shot */
	"2.wav",	/* explosion */
	0
};


static const samples_interface phantom2_samples_interface =
{
	2,	/* 2 channels */
	phantom2_sample_names
};


MACHINE_DRIVER_START( phantom2_audio )
	MDRV_SOUND_START(samples)

	MDRV_SPEAKER_STANDARD_MONO("mono")
	MDRV_SOUND_ADD("samples", SAMPLES, 0)
	MDRV_SOUND_CONFIG(phantom2_samples_interface)
	MDRV_SOUND_ROUTE(ALL_OUTPUTS, "mono", 1)
MACHINE_DRIVER_END


WRITE8_HANDLER( phantom2_audio_1_w )
{
	const device_config *samples = devtag_get_device(space->machine, "samples");
	UINT8 rising_bits = data & ~port_1_last;

	/* if (data & 0x01)  enable PLAYER SHOT sound */
	if (rising_bits & 0x01) sample_start(samples, 0, 0, 0);

	/* if (data & 0x02)  enable ENEMY SHOT sound */

	sound_global_enable((data >> 2) & 0x01);

	coin_counter_w(0, (data >> 3) & 0x01);

	/* if (data & 0x10)  enable RADAR sound */

	/* D5-D7 are not connected */

	port_1_last = data;
}


WRITE8_HANDLER( phantom2_audio_2_w )
{
	const device_config *samples = devtag_get_device(space->machine, "samples");
	UINT8 rising_bits = data & ~port_2_last;

	/* D0-D2 are not connected */

	/* if (data & 0x08)  enable EXPLOSION sound */
	if (rising_bits & 0x08) sample_start(samples, 1, 1, 0);

	output_set_value("EXPLAMP", (data >> 4) & 0x01);

	/* set JET SOUND FREQ((data >> 5) & 0x07)  0, if no jet sound */

	port_2_last = data;
}



/*************************************
 *
 *  Bowling Alley
 *
 *  Discrete sound emulation:
 *  Apr 2007, D.R.
 *
 *************************************/

/* nodes - inputs */
#define BOWLER_FOWL_EN			NODE_01

/* nodes - sounds */
#define BOWLER_FOWL_SND			NODE_10


static const discrete_op_amp_tvca_info bowler_fowl_tvca =
{
	RES_M(2.7),							/* R1103 */
	RES_K(680),							/* R1102 */
	0,									/* no r3 */
	RES_K(680),							/* R1104 */
	RES_K(1),							/* SIP */
	0,									/* no r6 */
	RES_K(300),							/* R1101 */
	0,									/* no r8 */
	0,									/* no r9 */
	0,									/* no r10 */
	0,									/* no r11 */
	CAP_U(0.1),							/* C1050 */
	0,									/* no c2 */
	0,									/* no c3 */
	5,									/* v1 */
	0,									/* no v2 */
	0,									/* no v3 */
	12,									/* vP */
	DISC_OP_AMP_TRIGGER_FUNCTION_NONE,	/* no f0 */
	DISC_OP_AMP_TRIGGER_FUNCTION_NONE,	/* no f1 */
	DISC_OP_AMP_TRIGGER_FUNCTION_TRG0,	/* f2 */
	DISC_OP_AMP_TRIGGER_FUNCTION_NONE,	/* no f3 */
	DISC_OP_AMP_TRIGGER_FUNCTION_NONE,	/* no f4 */
	DISC_OP_AMP_TRIGGER_FUNCTION_NONE	/* no f5 */
};


static DISCRETE_SOUND_START(bowler)

	/************************************************
     * Input register mapping
     ************************************************/
	DISCRETE_INPUT_LOGIC(BOWLER_FOWL_EN)


	/************************************************
     * Explosion
     ************************************************/
	DISCRETE_SQUAREWFIX(NODE_20,
					1,							/* ENAB */
					180,						/* FREQ */
					DEFAULT_TTL_V_LOGIC_1,		/* p-p AMP */
					50,							/* DUTY */
					DEFAULT_TTL_V_LOGIC_1 / 2,	/* dc BIAS */
					0)							/* PHASE */
	DISCRETE_OP_AMP_TRIG_VCA(NODE_21,			/* IC P3, pin 9 */
					BOWLER_FOWL_EN,				/* TRG0 */
					0,							/* no TRG1 */
					0,							/* no TRG2 */
					NODE_20,					/* IN0 */
					0,							/* no IN1 */
					&bowler_fowl_tvca)
	DISCRETE_CRFILTER(BOWLER_FOWL_SND,
					1,							/* ENAB */
					NODE_21,					/* IN0 */
					RES_K(68),					/* R1120 */
					CAP_U(0.1) )				/* C1048 */

	DISCRETE_OUTPUT(BOWLER_FOWL_SND, 10000)
DISCRETE_SOUND_END


MACHINE_DRIVER_START( bowler_audio )
	MDRV_SPEAKER_STANDARD_MONO("mono")

	MDRV_SOUND_ADD("discrete", DISCRETE, 0)
	MDRV_SOUND_CONFIG_DISCRETE(bowler)
	MDRV_SOUND_ROUTE(ALL_OUTPUTS, "mono", 1)
MACHINE_DRIVER_END


WRITE8_DEVICE_HANDLER( bowler_audio_1_w )
{
	/* D0 - selects controller on the cocktail PCB */

	coin_counter_w(0, (data >> 1) & 0x01);

	sound_global_enable((data >> 2) & 0x01);

	discrete_sound_w(device, BOWLER_FOWL_EN, (data >> 3) & 0x01);

	/* D4 - appears to be a screen flip, but it's
            shown unconnected on the schematics for both the
            upright and cocktail PCB's */

	/* D5 - triggered on a 'strike', sound circuit not labeled */

	/* D6 and D7 are not connected */
}


WRITE8_HANDLER( bowler_audio_2_w )
{
	/* set BALL ROLLING SOUND FREQ(data & 0x0f)
       0, if no rolling, 0x08 used during ball return */

	/* D4 -  triggered when the ball crosses the foul line,
             sound circuit not labeled */

	/* D5 - triggered on a 'spare', sound circuit not labeled */

	/* D6 and D7 are not connected */
}


WRITE8_HANDLER( bowler_audio_3_w )
{
	/* regardless of the data, enable BALL HITS PIN 1 sound
       (top circuit on the schematics) */
}


WRITE8_HANDLER( bowler_audio_4_w )
{
	/* regardless of the data, enable BALL HITS PIN 2 sound
       (bottom circuit on the schematics) */
}


WRITE8_HANDLER( bowler_audio_5_w )
{
	/* not sure, appears to me trigerred alongside the two
       BALL HITS PIN sounds */
}


WRITE8_HANDLER( bowler_audio_6_w )
{
	/* D0 is not connected */

	/* D3 is not connected */

	/* D6 and D7 are not connected */

	/* D1, D2, D4 and D5 have something to do with a chime circuit.
       D1 and D4 are HI when a 'strike' happens, and D2 and D5 are
       HI on a 'spare' */
}



/*************************************
 *
 *  Space Invaders
 *
 *  Author      : Tormod Tjaberg
 *  Created     : 1997-04-09
 *  Description : Sound routines for the 'invaders' games
 *
 *  Note:
 *  The samples were taken from Michael Strutt's (mstrutt@pixie.co.za)
 *  excellent space invader emulator and converted to signed samples so
 *  they would work under SEAL. The port info was also gleaned from
 *  his emulator. These sounds should also work on all the invader games.
 *
 *  Discrete sound emulation:
 *  Apr 2007, D.R.
 *
 *************************************/

static const sn76477_interface invaders_sn76477_interface =
{
	0,			/*  4 noise_res (N/C)        */
	0,			/*  5 filter_res (N/C)       */
	0,			/*  6 filter_cap (N/C)       */
	0,			/*  7 decay_res (N/C)        */
	0,			/*  8 attack_decay_cap (N/C) */
	RES_K(100), /* 10 attack_res             */
	RES_K(56),	/* 11 amplitude_res          */
	RES_K(10),	/* 12 feedback_res           */
	0,			/* 16 vco_voltage (N/C)      */
	CAP_U(0.1),	/* 17 vco_cap                */
	RES_K(8.2),	/* 18 vco_res                */
	5.0,		/* 19 pitch_voltage          */
	RES_K(120),	/* 20 slf_res                */
	CAP_U(1.0),	/* 21 slf_cap                */
	0,			/* 23 oneshot_cap (N/C)      */
	0,			/* 24 oneshot_res (N/C)      */
	1,			/* 22 vco                    */
	0,			/* 26 mixer A                */
	0,			/* 25 mixer B                */
	0,			/* 27 mixer C                */
	1,			/* 1  envelope 1             */
	0,			/* 28 envelope 2             */
	1			/* 9  enable (variable)      */
};



static const char *const invaders_sample_names[] =
{
	"*invaders",
	"1.wav",		/* shot/missle */
	"2.wav",		/* base hit/explosion */
	"3.wav",		/* invader hit */
	"4.wav",		/* fleet move 1 */
	"5.wav",		/* fleet move 2 */
	"6.wav",		/* fleet move 3 */
	"7.wav",		/* fleet move 4 */
	"8.wav",		/* UFO/saucer hit */
	"9.wav",		/* bonus base */
	0
};


static const samples_interface invaders_samples_interface =
{
	6,	/* 6 channels */
	invaders_sample_names
};


/* left in for all games that hack into invaders samples for audio */
MACHINE_DRIVER_START( invaders_samples_audio )
	MDRV_SOUND_START(samples)

	MDRV_SPEAKER_STANDARD_MONO("mono")

	MDRV_SOUND_ADD("sn", SN76477, 0)
	MDRV_SOUND_CONFIG(invaders_sn76477_interface)
	MDRV_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.5)

	MDRV_SOUND_ADD("samples", SAMPLES, 0)
	MDRV_SOUND_CONFIG(invaders_samples_interface)
	MDRV_SOUND_ROUTE(ALL_OUTPUTS, "mono", 1.0)
MACHINE_DRIVER_END


/* nodes - inputs */
#define INVADERS_SAUCER_HIT_EN				01
#define INVADERS_FLEET_DATA					02
#define INVADERS_BONUS_MISSLE_BASE_EN		03
#define INVADERS_INVADER_HIT_EN				04
#define INVADERS_EXPLOSION_EN				05
#define INVADERS_MISSILE_EN					06

/* nodes - sounds */
#define INVADERS_NOISE						NODE_10
#define INVADERS_SAUCER_HIT_SND				11
#define INVADERS_FLEET_SND					12
#define INVADERS_BONUS_MISSLE_BASE_SND		13
#define INVADERS_INVADER_HIT_SND			14
#define INVADERS_EXPLOSION_SND				15
#define INVADERS_MISSILE_SND				16


static const discrete_op_amp_info invaders_saucer_hit_op_amp_B3_9 =
{
	DISC_OP_AMP_IS_NORTON,
	0,						/* no r1 */
	RES_K(100),				/* R72 */
	RES_M(1),				/* R71 */
	0,						/* no r4 */
	CAP_U(1),				/* C23 */
	0,						/* vN */
	12						/* vP */
};


static const discrete_op_amp_osc_info invaders_saucer_hit_osc =
{
	DISC_OP_AMP_OSCILLATOR_1 | DISC_OP_AMP_IS_NORTON | DISC_OP_AMP_OSCILLATOR_OUT_CAP,
	RES_M(1),		/* R70 */
	RES_K(470),		/* R64 */
	RES_K(100),		/* R61 */
	RES_K(120),		/* R63 */
	RES_M(1),		/* R62 */
	0,				/* no r6 */
	0,				/* no r7 */
	0,				/* no r8 */
	CAP_U(0.1),		/* C21 */
	12,				/* vP */
};

static const discrete_op_amp_osc_info invaders_saucer_hit_vco =
{
	DISC_OP_AMP_OSCILLATOR_VCO_1 | DISC_OP_AMP_IS_NORTON | DISC_OP_AMP_OSCILLATOR_OUT_SQW,
	RES_M(1),		/* R65 */
	RES_K(470),		/* R66 */
	RES_K(680),		/* R67 */
	RES_M(1),		/* R69*/
	RES_M(1),		/* R68 */
	0,				/* no r6 */
	0,				/* no r7 */
	0,				/* no r8 */
	CAP_P(470),		/* C22 */
	12,				/* vP */
};


static const discrete_op_amp_info invaders_saucer_hit_op_amp_B3_10 =
{
	DISC_OP_AMP_IS_NORTON,
	RES_K(680),				/* R73 */
	RES_K(680),				/* R77 */
	RES_M(2.7),				/* R74 */
	RES_K(680),				/* R75 */
	0,						/* no c */
	0,						/* vN */
	12						/* vP */
};


static const discrete_comp_adder_table invaders_thump_resistors =
{
	DISC_COMP_P_RESISTOR,
	0,							/* no cDefault */
	4,							/* length */
	{ RES_K(20) + RES_K(20),	/* R126 + R127 */
	  RES_K(68),				/* R128 */
	  RES_K(82),				/* R129 */
	  RES_K(100) }				/* R130 */
};


static const discrete_555_desc invaders_thump_555 =
{
	DISC_555_OUT_ENERGY | DISC_555_OUT_DC,
	5,
	5.0 - 0.6,				/* 5V - diode drop */
	DEFAULT_TTL_V_LOGIC_1	/* Output of F3 7411 buffer */
};


static const discrete_555_desc invaders_bonus_555 =
{
	DISC_555_OUT_SQW | DISC_555_OUT_DC,
	5.0,					/* 5V */
	DEFAULT_555_VALUES
};


static const discrete_op_amp_1sht_info invaders_invader_hit_1sht =
{
	DISC_OP_AMP_1SHT_1 | DISC_OP_AMP_IS_NORTON,
	RES_M(4.7),		/* R49 */
	RES_K(100),		/* R51 */
	RES_M(1),		/* R48 */
	RES_M(1),		/* R50 */
	RES_M(2.2),		/* R52 */
	CAP_U(0.1),		/* C18 */
	CAP_P(470),		/* C20 */
	0,				/* vN */
	12				/* vP */
};


static const discrete_op_amp_info invaders_invader_hit_op_amp_D3_10 =
{
	DISC_OP_AMP_IS_NORTON,
	0,				/* no r1 */
	RES_K(10),		/* R53 */
	RES_M(1),		/* R137 */
	0,				/* no r4 */
	CAP_U(0.47),	/* C19 */
	0,				/* vN */
	12				/* vP */
};


static const discrete_op_amp_osc_info invaders_invader_hit_osc =
{
	DISC_OP_AMP_OSCILLATOR_1 | DISC_OP_AMP_IS_NORTON | DISC_OP_AMP_OSCILLATOR_OUT_CAP,
	RES_M(1),		/* R37 */
	RES_K(10),		/* R41 */
	RES_K(100),		/* R38 */
	RES_K(120),		/* R40 */
	RES_M(1),		/* R39 */
	0,				/* no r6 */
	0,				/* no r7 */
	0,				/* no r8 */
	CAP_U(0.1),		/* C16 */
	12,				/* vP */
};


static const discrete_op_amp_osc_info invaders_invader_hit_vco =
{
	DISC_OP_AMP_OSCILLATOR_VCO_1 | DISC_OP_AMP_IS_NORTON | DISC_OP_AMP_OSCILLATOR_OUT_CAP,
	RES_M(1),		/* R42 */
	RES_K(470),		/* R43 */
	RES_K(680),		/* R44 */
	RES_M(1),		/* R46 */
	RES_M(1),		/* R45 */
	0,				/* no r6 */
	0,				/* no r7 */
	0,				/* no r8 */
	CAP_P(330),		/* C16 */
	12,				/* vP */
};


static const discrete_op_amp_info invaders_invader_hit_op_amp_D3_4 =
{
	DISC_OP_AMP_IS_NORTON,
	RES_K(470),		/* R55 */
	RES_K(680),		/* R54 */
	RES_M(2.7),		/* R56 */
	RES_K(680),		/* R57 */
	0,				/* no c */
	0,				/* vN */
	12				/* vP */
};


static const discrete_op_amp_1sht_info invaders_explosion_1sht =
{
	DISC_OP_AMP_1SHT_1 | DISC_OP_AMP_IS_NORTON,
	RES_M(4.7),		/* R90 */
	RES_K(100),		/* R88 */
	RES_M(1),		/* R91 */
	RES_M(1),		/* R89 */
	RES_M(2.2),		/* R92 */
	CAP_U(2.2),		/* C24 */
	CAP_P(470),		/* C25 */
	0,				/* vN */
	12				/* vP */
};


static const discrete_op_amp_tvca_info invaders_explosion_tvca =
{
	RES_M(2.7),							/* R80 */
	RES_K(680),							/* R79 */
	0,									/* no r3 */
	RES_K(680),							/* R82 */
	RES_K(10),							/* R93 */
	0,									/* no r6 */
	RES_K(680),							/* R83 */
	0,									/* no r8 */
	0,									/* no r9 */
	0,									/* no r10 */
	0,									/* no r11 */
	CAP_U(1),							/* C26 */
	0,									/* no c2 */
	0,									/* no c3 */
	12.0 - OP_AMP_NORTON_VBE,			/* v1 */
	0,									/* no v2 */
	0,									/* no v3 */
	12,									/* vP */
	DISC_OP_AMP_TRIGGER_FUNCTION_NONE,	/* no f0 */
	DISC_OP_AMP_TRIGGER_FUNCTION_NONE,	/* no f1 */
	DISC_OP_AMP_TRIGGER_FUNCTION_TRG0,	/* f2 */
	DISC_OP_AMP_TRIGGER_FUNCTION_NONE,	/* no f3 */
	DISC_OP_AMP_TRIGGER_FUNCTION_NONE,	/* no f4 */
	DISC_OP_AMP_TRIGGER_FUNCTION_NONE	/* no f5 */
};


static const discrete_op_amp_1sht_info invaders_missle_1sht =
{
	DISC_OP_AMP_1SHT_1 | DISC_OP_AMP_IS_NORTON,
	RES_M(4.7),		/* R32 */
	RES_K(100),		/* R30 */
	RES_M(1),		/* R31 */
	RES_M(1),		/* R33 */
	RES_M(2.2),		/* R34 */
	CAP_U(1),		/* C12 */
	CAP_P(470),		/* C15 */
	0,				/* vN */
	12				/* vP */
};


static const discrete_op_amp_info invaders_missle_op_amp_B3 =
{
	DISC_OP_AMP_IS_NORTON,
	0,				/* no r1 */
	RES_K(10),		/* R35 */
	RES_M(1.5),		/* R36 */
	0,				/* no r4 */
	CAP_U(0.22),	/* C13 */
	0,				/* vN */
	12				/* vP */
};


static const discrete_op_amp_osc_info invaders_missle_op_amp_osc =
{
	DISC_OP_AMP_OSCILLATOR_VCO_3 | DISC_OP_AMP_IS_NORTON | DISC_OP_AMP_OSCILLATOR_OUT_SQW,
	1.0 / (1.0 / RES_M(1) + 1.0 / RES_K(330)) + RES_M(1.5),		/* R29||R11 + R12 */
	RES_M(1),		/* R16 */
	RES_K(560),		/* R17 */
	RES_M(2.2),		/* R19 */
	RES_M(1),		/* R16 */
	RES_M(4.7),		/* R14 */
	RES_M(3.3),		/* R13 */
	0,				/* no r8 */
	CAP_P(330),		/* C58 */
	12,				/* vP */
};


static const discrete_op_amp_info invaders_missle_op_amp_A3 =
{
	DISC_OP_AMP_IS_NORTON,
	RES_K(560),		/* R22 */
	RES_K(470),		/* R15 */
	RES_M(2.7),		/* R20 */
	RES_K(560),		/* R21 */
	0,				/* no c */
	0,				/* vN */
	12				/* vP */
};


static const discrete_op_amp_tvca_info invaders_missle_tvca =
{
	RES_M(2.7),							/* R25 */
	RES_K(560),							/* R23 */
	0,									/* no r3 */
	RES_K(560),							/* R26 */
	RES_K(1),							/*  */
	0,									/* no r6 */
	RES_K(560),							/* R60 */
	0,									/* no r8 */
	0,									/* no r9 */
	0,									/* no r10 */
	0,									/* no r11 */
	CAP_U(0.1),							/* C14 */
	0,									/* no c2 */
	0,									/* no c3 */
	5,									/* v1 */
	0,									/* no v2 */
	0,									/* no v3 */
	12,									/* vP */
	DISC_OP_AMP_TRIGGER_FUNCTION_NONE,	/* no f0 */
	DISC_OP_AMP_TRIGGER_FUNCTION_NONE,	/* no f1 */
	DISC_OP_AMP_TRIGGER_FUNCTION_TRG0,	/* f2 */
	DISC_OP_AMP_TRIGGER_FUNCTION_NONE,	/* no f3 */
	DISC_OP_AMP_TRIGGER_FUNCTION_NONE,	/* no f4 */
	DISC_OP_AMP_TRIGGER_FUNCTION_NONE	/* no f5 */
};


static const discrete_mixer_desc invaders_mixer =
{
	DISC_MIXER_IS_OP_AMP,		/* type */
	{ RES_K(200),				/* R78 */
	  RES_K(10) + 100 + 100,	/* R134 + R133 + R132 */
	  RES_K(150),				/* R136 */
	  RES_K(200),				/* R59 */
	  RES_K(2) + RES_K(6.8) + RES_K(5.6),	/* R86 + R85 + R84 */
	  RES_K(150) },				/* R28 */
	{0},						/* no rNode{} */
	{ 0,
	  0,
	  0,
	  0,
	  0,
	  CAP_U(0.001) },			/* C11 */
	0,							/* no rI */
	RES_K(100),					/* R105 */
	0,							/* no cF */
	CAP_U(0.1),					/* C45 */
	0,							/* vRef = ground */
	1							/* gain */
};


/* sound board 1 or 2, for multi-board games */
#define INVADERS_NODE(_node, _board)	(NODE(_node + ((_board - 1) * 100)))

/************************************************
 * Noise Generator
 ************************************************/
/* Noise clock was breadboarded and measured at 7515 */
#define INVADERS_NOISE_GENERATOR												\
	DISCRETE_LFSR_NOISE(INVADERS_NOISE,					/* IC N5, pin 10 */     \
					1,									/* ENAB */              \
					1,									/* no RESET */          \
					7515,								/* CLK in Hz */         \
					12,									/* p-p AMPL */          \
					0,									/* no FEED input */     \
					12.0/2,								/* dc BIAS */           \
					&midway_lfsr)


/************************************************
 * Saucer Hit
 ************************************************/
#define INVADERS_SAUCER_HIT(_board)														\
	DISCRETE_INPUTX_LOGIC(INVADERS_NODE(INVADERS_SAUCER_HIT_EN, _board), 12, 0, 0)		\
	DISCRETE_OP_AMP(INVADERS_NODE(20, _board),						/* IC B3, pin 9 */  \
					1,												/* ENAB */          \
					0,												/* no IN0 */        \
					INVADERS_NODE(INVADERS_SAUCER_HIT_EN, _board),	/* IN1 */           \
					&invaders_saucer_hit_op_amp_B3_9)                                   \
	DISCRETE_OP_AMP_OSCILLATOR(INVADERS_NODE(21, _board),			/* IC A4, pin 5 */  \
					1,												/* ENAB */          \
					&invaders_saucer_hit_osc)               		                    \
	DISCRETE_OP_AMP_VCO1(INVADERS_NODE(22, _board),					/* IC A4, pin 9 */  \
					1,												/* ENAB */          \
					INVADERS_NODE(21, _board),						/* VMOD1 */         \
					&invaders_saucer_hit_vco)               		                    \
	DISCRETE_OP_AMP(INVADERS_NODE(INVADERS_SAUCER_HIT_SND, _board),	/* IC B3, pin 10 */ \
					1,												/* ENAB */          \
					INVADERS_NODE(22, _board),						/* IN0 */           \
					INVADERS_NODE(20, _board),						/* IN1 */           \
					&invaders_saucer_hit_op_amp_B3_10)


/************************************************
 * Fleet movement
 ************************************************/
#define INVADERS_FLEET(_board)															\
	DISCRETE_INPUT_DATA  (INVADERS_NODE(INVADERS_FLEET_DATA, _board))					\
	DISCRETE_COMP_ADDER(INVADERS_NODE(30, _board),                                      \
					INVADERS_NODE(INVADERS_FLEET_DATA, _board),		/* DATA */          \
					&invaders_thump_resistors)                                          \
	DISCRETE_555_ASTABLE(INVADERS_NODE(31, _board),					/* IC F3, pin 6 */  \
					1,												/* RESET */         \
					INVADERS_NODE(30, _board),						/* R1 */            \
					RES_K(75),										/* R131 */          \
					CAP_U(0.1),										/* C29 */           \
					&invaders_thump_555)                                                \
	DISCRETE_RCFILTER(INVADERS_NODE(32, _board),                                        \
					1,												/* ENAB */          \
					INVADERS_NODE(31, _board),						/* IN0 */           \
					100,											/* R132 */          \
					CAP_U(4.7) )									/* C31 */           \
	DISCRETE_RCFILTER(INVADERS_NODE(INVADERS_FLEET_SND, _board),                        \
					1,												/* ENAB */          \
					INVADERS_NODE(32, _board),						/* IN0 */           \
					100 + 100,										/* R132 + R133 */   \
					CAP_U(10) )										/* C32 */


/************************************************
 * Bonus Missle Base
 ************************************************/
#define INVADERS_BONUS_MISSLE_BASE(_board)																			\
	DISCRETE_INPUT_LOGIC (INVADERS_NODE(INVADERS_BONUS_MISSLE_BASE_EN, _board))										\
	DISCRETE_555_ASTABLE(INVADERS_NODE(40, _board),						/* IC F4, pin 9 */                          \
					INVADERS_NODE(INVADERS_BONUS_MISSLE_BASE_EN, _board),/* RESET */                                \
					RES_K(100),											/* R94 */                                   \
					RES_K(47),											/* R95 */                                   \
					CAP_U(1),											/* C34 */                                   \
					&invaders_bonus_555)                				                                            \
	DISCRETE_SQUAREWFIX(INVADERS_NODE(41, _board),       				                                            \
					1,													/* ENAB */                                  \
					480,												/* FREQ */                                  \
					1,													/* AMP */                                   \
					50,													/* DUTY */                                  \
					1.0/2,												/* BIAS */                                  \
					0)													/* PHASE */                                 \
	DISCRETE_LOGIC_AND3(INVADERS_NODE(42, _board),						/* IC F3, pin 12 */                         \
					1,													/* ENAB */                                  \
					INVADERS_NODE(INVADERS_BONUS_MISSLE_BASE_EN, _board),/* INP0 */                                 \
					INVADERS_NODE(41, _board),							/* INP1 */                                  \
					INVADERS_NODE(40, _board) )							/* INP2 */                                  \
	DISCRETE_GAIN(INVADERS_NODE(INVADERS_BONUS_MISSLE_BASE_SND, _board),/* adjust from logic to TTL voltage level */\
					INVADERS_NODE(42, _board),							/* IN0 */                                   \
					DEFAULT_TTL_V_LOGIC_1)								/* GAIN */


/************************************************
 * Invader Hit
 ************************************************/
#define INVADERS_INVADER_HIT(_board, _type)													\
	DISCRETE_INPUTX_LOGIC(INVADERS_NODE(INVADERS_INVADER_HIT_EN, _board), 5, 0, 0)			\
	DISCRETE_OP_AMP_ONESHOT(INVADERS_NODE(50, _board),					/* IC D3, pin 9 */  \
					INVADERS_NODE(INVADERS_INVADER_HIT_EN, _board),		/* TRIG */          \
					&_type##_invader_hit_1sht)                                              \
	DISCRETE_OP_AMP(INVADERS_NODE(51, _board),							/* IC D3, pin 10 */ \
					1,													/* ENAB */          \
					0,													/* no IN0 */        \
					INVADERS_NODE(50, _board),							/* IN1 */           \
					&invaders_invader_hit_op_amp_D3_10)                                     \
	DISCRETE_OP_AMP_OSCILLATOR(INVADERS_NODE(52, _board),				/* IC B4, pin 5 */  \
					1,													/* ENAB */          \
					&_type##_invader_hit_osc)                                               \
	DISCRETE_OP_AMP_VCO1(INVADERS_NODE(53, _board),						/* IC B4, pin 4 */  \
					1,													/* ENAB */          \
					INVADERS_NODE(52, _board),							/* VMOD1 */         \
					&invaders_invader_hit_vco)                                              \
	DISCRETE_OP_AMP(INVADERS_NODE(INVADERS_INVADER_HIT_SND,	_board),	/* IC D3, pin 4 */  \
					1,													/* ENAB */          \
					INVADERS_NODE(53, _board),							/* IN0 */           \
					INVADERS_NODE(51, _board),							/* IN1 */           \
					&invaders_invader_hit_op_amp_D3_4)


/************************************************
 * Explosion
 ************************************************/
#define INVADERS_EXPLOSION(_board)														\
	DISCRETE_INPUTX_LOGIC(INVADERS_NODE(INVADERS_EXPLOSION_EN, _board), 5, 0, 0)		\
	DISCRETE_OP_AMP_ONESHOT(INVADERS_NODE(60, _board),				/* IC D2, pin 10 */ \
					INVADERS_NODE(INVADERS_EXPLOSION_EN, _board),	/* TRIG */          \
					&invaders_explosion_1sht)                                           \
	DISCRETE_OP_AMP_TRIG_VCA(INVADERS_NODE(61, _board),				/* IC D2, pin 4 */  \
					INVADERS_NODE(60, _board),						/* TRG0 */          \
					0,												/* no TRG1 */       \
					0,												/* no TRG2 */       \
					INVADERS_NOISE,									/* IN0 */           \
					0,												/* no IN1 */        \
					&invaders_explosion_tvca)                                           \
	DISCRETE_RCFILTER(INVADERS_NODE(62, _board),                                        \
					1,												/* ENAB */          \
					INVADERS_NODE(61, _board),						/* IN0 */           \
					RES_K(5.6),										/* R84 */           \
					CAP_U(0.1) )									/* C27 */           \
	DISCRETE_RCFILTER(INVADERS_NODE(INVADERS_EXPLOSION_SND, _board),                    \
					1,												/* ENAB */          \
					INVADERS_NODE(62, _board),						/* IN0 */           \
					RES_K(5.6) + RES_K(6.8),						/* R84 + R85 */     \
					CAP_U(0.1) )									/* C28 */


/************************************************
 * Missle Sound
 ************************************************/
#define INVADERS_MISSILE(_board, _type)																\
	DISCRETE_INPUTX_LOGIC(INVADERS_NODE(INVADERS_MISSILE_EN, _board), 5, 0, 0)						\
	DISCRETE_OP_AMP_ONESHOT(INVADERS_NODE(70, _board),						/* IC B3, pin 4 */      \
					INVADERS_NODE(INVADERS_MISSILE_EN, _board),				/* TRIG */              \
					&_type##_missle_1sht)                               	                        \
	DISCRETE_OP_AMP(INVADERS_NODE(71, _board),								/* IC B3, pin 5 */      \
					1,														/* ENAB */              \
					0,														/* no IN0 */            \
					INVADERS_NODE(70, _board),								/* IN1 */               \
					&invaders_missle_op_amp_B3)                         	                        \
	/* next 2 modules simulate the D1 voltage drop */                   	                        \
	DISCRETE_ADDER2(INVADERS_NODE(72, _board),                          	                        \
					1,														/* ENAB */              \
					INVADERS_NODE(71, _board),								/* IN0 */               \
					-0.5)													/* IN1 */               \
	DISCRETE_CLAMP(INVADERS_NODE(73, _board),           					                        \
					1,														/* ENAB */              \
					INVADERS_NODE(72, _board),								/* IN0 */               \
					0,														/* MIN */               \
					12,														/* MAX */               \
					0)														/* CLAMP */             \
	DISCRETE_CRFILTER(INVADERS_NODE(74, _board),        					                        \
					1,														/* ENAB */              \
					INVADERS_NOISE,											/* IN0 */               \
					RES_M(1) + RES_K(330),									/* R29, R11 */          \
					CAP_U(0.1) )											/* C57 */               \
	DISCRETE_GAIN(INVADERS_NODE(75, _board),                            	                        \
					INVADERS_NODE(74, _board),								/* IN0 */               \
					RES_K(330)/(RES_M(1) + RES_K(330)))						/* GAIN - R29 : R11 */  \
	DISCRETE_OP_AMP_VCO2(INVADERS_NODE(76, _board),							/* IC C1, pin 4 */      \
					1,														/* ENAB */              \
					INVADERS_NODE(75, _board),								/* VMOD1 */             \
					INVADERS_NODE(73, _board),								/* VMOD2 */             \
					&invaders_missle_op_amp_osc)                        	                        \
	DISCRETE_OP_AMP(INVADERS_NODE(77, _board),								/* IC A3, pin 9 */      \
					1,														/* ENAB */              \
					INVADERS_NODE(76, _board),								/* IN0 */               \
					INVADERS_NODE(73, _board),								/* IN1 */               \
					&invaders_missle_op_amp_A3)	                                                    \
	DISCRETE_OP_AMP_TRIG_VCA(INVADERS_NODE(INVADERS_MISSILE_SND, _board),	/* IC A3, pin 10 */     \
					INVADERS_NODE(INVADERS_MISSILE_EN, _board),				/* TRG0 */              \
					0,														/* no TRG1 */           \
					0,														/* no TRG2 */           \
					INVADERS_NODE(77, _board),								/* IN0 */               \
					0,														/* no IN1 */            \
					&invaders_missle_tvca)


/************************************************
 * Final mix
 ************************************************/
#define INVADERS_MIXER(_board, _type)													\
	DISCRETE_MIXER6(INVADERS_NODE(90, _board),                                          \
					1,														/* ENAB */  \
					INVADERS_NODE(INVADERS_SAUCER_HIT_SND, _board),			/* IN0 */   \
					INVADERS_NODE(INVADERS_FLEET_SND, _board),				/* IN1 */   \
					INVADERS_NODE(INVADERS_BONUS_MISSLE_BASE_SND, _board),	/* IN2 */   \
					INVADERS_NODE(INVADERS_INVADER_HIT_SND, _board),		/* IN3 */   \
					INVADERS_NODE(INVADERS_EXPLOSION_SND, _board),			/* IN4 */   \
					INVADERS_NODE(INVADERS_MISSILE_SND, _board),			/* IN5 */   \
					&_type##_mixer)                                                     \
	DISCRETE_OUTPUT(INVADERS_NODE(90, _board), 2500)

/* Schematic M051-00739-A005 and M051-00739-B005 */
/* P.C.      A084-90700-B000 and A084-90700-C000 */
static DISCRETE_SOUND_START(invaders)
	INVADERS_NOISE_GENERATOR
	INVADERS_SAUCER_HIT(1)
	INVADERS_FLEET(1)
	INVADERS_BONUS_MISSLE_BASE(1)
	INVADERS_INVADER_HIT(1, invaders)
	INVADERS_EXPLOSION(1)
	INVADERS_MISSILE(1, invaders)
	INVADERS_MIXER(1, invaders)
DISCRETE_SOUND_END


MACHINE_DRIVER_START( invaders_audio )
	MDRV_SPEAKER_STANDARD_MONO("mono")

	MDRV_SOUND_ADD("sn", SN76477, 0)
	MDRV_SOUND_CONFIG(invaders_sn76477_interface)
	MDRV_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.5)

	MDRV_SOUND_ADD("discrete", DISCRETE, 0)
	MDRV_SOUND_CONFIG_DISCRETE(invaders)
	MDRV_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.5)
MACHINE_DRIVER_END


WRITE8_DEVICE_HANDLER( invaders_audio_1_w )
{
	const device_config *sn = devtag_get_device(device->machine, "sn");

	sn76477_enable_w(sn, (~data >> 0) & 0x01);	/* saucer sound */

	discrete_sound_w(device, INVADERS_NODE(INVADERS_MISSILE_EN, 1), data & 0x02);
	discrete_sound_w(device, INVADERS_NODE(INVADERS_EXPLOSION_EN, 1), data & 0x04);
	discrete_sound_w(device, INVADERS_NODE(INVADERS_INVADER_HIT_EN, 1), data & 0x08);
	discrete_sound_w(device, INVADERS_NODE(INVADERS_BONUS_MISSLE_BASE_EN, 1), data & 0x10);

	sound_global_enable(data & 0x20);

	/* D6 and D7 are not connected */
}


WRITE8_DEVICE_HANDLER( invaders_audio_2_w )
{
	discrete_sound_w(device, INVADERS_NODE(INVADERS_FLEET_DATA, 1), data & 0x0f);
	discrete_sound_w(device, INVADERS_NODE(INVADERS_SAUCER_HIT_EN, 1), data & 0x10);

	/* the flip screen line is only connected on the cocktail PCB */
	if (invaders_is_cabinet_cocktail(device->machine))
	{
		invaders_set_flip_screen((data >> 5) & 0x01);
	}

	/* D6 and D7 are not connected */
}



/*************************************
 *
 *  Blue Shark
 *
 *  Discrete sound emulation: Jan 2007, D.R.
 *
 *************************************/

/* Noise clock was breadboarded and measured at 7700Hz */


/* nodes - inputs */
#define BLUESHRK_OCTOPUS_EN		NODE_01
#define BLUESHRK_HIT_EN			NODE_02
#define BLUESHRK_SHARK_EN		NODE_03
#define BLUESHRK_SHOT_EN		NODE_04
#define BLUESHRK_GAME_ON_EN		NODE_05

/* nodes - sounds */
#define BLUESHRK_NOISE_1		NODE_11
#define BLUESHRK_NOISE_2		NODE_12
#define BLUESHRK_OCTOPUS_SND	NODE_13
#define BLUESHRK_HIT_SND		NODE_14
#define BLUESHRK_SHARK_SND		NODE_15
#define BLUESHRK_SHOT_SND		NODE_16


static const discrete_555_desc blueshrk_555_H1A =
{
	DISC_555_OUT_SQW | DISC_555_OUT_DC | DISC_555_TRIGGER_IS_LOGIC,
	5,				/* B+ voltage of 555 */
	DEFAULT_555_VALUES
};


static const discrete_555_desc blueshrk_555_H1B =
{
	DISC_555_OUT_ENERGY | DISC_555_OUT_DC,
	5,				/* B+ voltage of 555 */
	DEFAULT_555_CHARGE,
	12				/* the OC buffer H2 converts the output voltage to 12V. */
};


static const discrete_mixer_desc blueshrk_mixer =
{
	DISC_MIXER_IS_OP_AMP,
	{ RES_K(750),
	  1.0/(1.0/RES_K(510) + 1.0/RES_K(22)),
	  RES_M(1),
	  RES_K(56) },
	{ 0 },
	{ CAP_U(1),
	  CAP_U(1),
	  CAP_U(1),
	  CAP_U(1) },
	0,
	RES_K(100),
	0,
	CAP_U(0.1),
	0,		/* Vref */
	700		/* final gain */
};


static DISCRETE_SOUND_START(blueshrk)

	/************************************************
     * Input register mapping
     ************************************************/
	DISCRETE_INPUT_LOGIC(BLUESHRK_OCTOPUS_EN)
	DISCRETE_INPUT_LOGIC(BLUESHRK_HIT_EN)
	DISCRETE_INPUT_LOGIC(BLUESHRK_SHARK_EN)
	DISCRETE_INPUT_LOGIC(BLUESHRK_SHOT_EN)
	DISCRETE_INPUT_LOGIC(BLUESHRK_GAME_ON_EN)

	/************************************************
     * Octopus sound
     ************************************************/
	DISCRETE_CONSTANT(BLUESHRK_OCTOPUS_SND, 0)	/* placeholder for incomplete sound */

	/************************************************
     * Hit sound
     ************************************************/
	/* the 555 trigger is connected to the cap, so when reset goes high, the 555 is triggered */
	/* but the 555_MSTABLE does not currently allow connection of the trigger to the cap */
	/* so we will cheat and add a pulse 1 sample wide to trigger it */
	DISCRETE_ONESHOT(NODE_30, BLUESHRK_HIT_EN, 1, /* 1 sample wide */ 0, DISC_ONESHOT_REDGE | DISC_ONESHOT_NORETRIG | DISC_OUT_ACTIVE_LOW)
	DISCRETE_555_MSTABLE(NODE_31, BLUESHRK_HIT_EN, NODE_30, RES_K(47), CAP_U(2.2), &blueshrk_555_H1A)
	DISCRETE_LOGIC_INVERT(NODE_32, 1, BLUESHRK_HIT_EN)
	DISCRETE_COUNTER(NODE_33, 1, /*RST*/ NODE_32, /*CLK*/ NODE_31, 1, DISC_COUNT_UP, 0, DISC_CLK_ON_F_EDGE)
	DISCRETE_SWITCH(NODE_34, 1, NODE_33, CAP_U(0.015) + CAP_U(0.01), CAP_U(0.022))
	DISCRETE_555_ASTABLE(BLUESHRK_HIT_SND, BLUESHRK_HIT_EN, RES_K(22), RES_K(39), NODE_34, &blueshrk_555_H1B)

	/************************************************
     * Shark sound
     ************************************************/
	DISCRETE_CONSTANT(BLUESHRK_SHARK_SND, 0)	/* paceholder for incomplete sound */

	/************************************************
     * Shot sound
     ************************************************/
	DISCRETE_CONSTANT(BLUESHRK_SHOT_SND, 0)		/* placeholder for incomplete sound */

	/************************************************
     * Combine all sound sources.
     ************************************************/
	DISCRETE_MIXER4(NODE_91, BLUESHRK_GAME_ON_EN, BLUESHRK_OCTOPUS_SND, BLUESHRK_HIT_SND, BLUESHRK_SHARK_SND, BLUESHRK_SHOT_SND, &blueshrk_mixer)

	DISCRETE_OUTPUT(NODE_91, 1)
DISCRETE_SOUND_END


MACHINE_DRIVER_START( blueshrk_audio )
	MDRV_SPEAKER_STANDARD_MONO("mono")
	MDRV_SOUND_ADD("discrete", DISCRETE, 0)
	MDRV_SOUND_CONFIG_DISCRETE(blueshrk)
	MDRV_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.25)
MACHINE_DRIVER_END


WRITE8_DEVICE_HANDLER( blueshrk_audio_w )
{
	discrete_sound_w(device, BLUESHRK_GAME_ON_EN, (data >> 0) & 0x01);

	/* discrete_sound_w(device, BLUESHRK_SHOT_EN, (data >> 1) & 0x01); */

	discrete_sound_w(device, BLUESHRK_HIT_EN, (data >> 2) & 0x01);

	/* discrete_sound_w(device, BLUESHRK_SHARK_EN, (data >> 3) & 0x01); */

	/* if (data & 0x10)  enable KILLED DIVER sound, this circuit
       doesn't appear to be on the schematics */

	/* discrete_sound_w(device, BLUESHRK_OCTOPUS_EN, (data >> 5) & 0x01); */

	/* D6 and D7 are not connected */
}



/*************************************
 *
 *  Space Invaders II (cocktail)
 *
 *************************************/

static const sn76477_interface invad2ct_p1_sn76477_interface =
{
	0,			/*  4 noise_res (N/C)        */
	0,			/*  5 filter_res (N/C)       */
	0,			/*  6 filter_cap (N/C)       */
	0,			/*  7 decay_res (N/C)        */
	0,			/*  8 attack_decay_cap (N/C) */
	RES_K(100), /* 10 attack_res             */
	RES_K(56),	/* 11 amplitude_res          */
	RES_K(10),	/* 12 feedback_res           */
	0,			/* 16 vco_voltage (N/C)      */
	CAP_U(0.1),	/* 17 vco_cap                */
	RES_K(8.2),	/* 18 vco_res                */
	5.0,		/* 19 pitch_voltage          */
	RES_K(120),	/* 20 slf_res                */
	CAP_U(1.0),	/* 21 slf_cap                */
	0,			/* 23 oneshot_cap (N/C)      */
	0,			/* 24 oneshot_res (N/C)      */
	1,			/* 22 vco                    */
	0,			/* 26 mixer A                */
	0,			/* 25 mixer B                */
	0,			/* 27 mixer C                */
	1,			/* 1  envelope 1             */
	0,			/* 28 envelope 2             */
	1			/* 9  enable (variable)      */
};


static const sn76477_interface invad2ct_p2_sn76477_interface =
{
	0,			  /*  4 noise_res (N/C)        */
	0,			  /*  5 filter_res (N/C)       */
	0,			  /*  6 filter_cap (N/C)       */
	0,			  /*  7 decay_res (N/C)        */
	0,			  /*  8 attack_decay_cap (N/C) */
	RES_K(100),   /* 10 attack_res             */
	RES_K(56),	  /* 11 amplitude_res          */
	RES_K(10),	  /* 12 feedback_res           */
	0,			  /* 16 vco_voltage (N/C)      */
	CAP_U(0.047), /* 17 vco_cap                */
	RES_K(39),	  /* 18 vco_res                */
	5.0,		  /* 19 pitch_voltage          */
	RES_K(120),	  /* 20 slf_res                */
	CAP_U(1.0),	  /* 21 slf_cap                */
	0,			  /* 23 oneshot_cap (N/C)      */
	0,			  /* 24 oneshot_res (N/C)      */
	1,			  /* 22 vco                    */
	0,			  /* 26 mixer A                */
	0,			  /* 25 mixer B                */
	0,			  /* 27 mixer C                */
	1,			  /* 1  envelope 1             */
	0,			  /* 28 envelope 2             */
	1			  /* 9  enable (variable)      */
};


static const discrete_op_amp_1sht_info invad2ct_invader_hit_1sht =
{
	DISC_OP_AMP_1SHT_1 | DISC_OP_AMP_IS_NORTON,
	RES_M(4.7),		/* R49 */
	RES_K(100),		/* R51 */
	RES_M(1),		/* R48 */
	RES_M(1),		/* R50 */
	RES_M(2.2),		/* R52 */
	CAP_U(0.22),	/* C18 */
	CAP_P(470),		/* C20 */
	0,				/* vN */
	12				/* vP */
};


static const discrete_op_amp_osc_info invad2ct_invader_hit_osc =
{
	DISC_OP_AMP_OSCILLATOR_1 | DISC_OP_AMP_IS_NORTON | DISC_OP_AMP_OSCILLATOR_OUT_CAP,
	RES_M(1),		/* R37 */
	RES_K(10),		/* R41 */
	RES_K(100),		/* R38 */
	RES_K(120),		/* R40 */
	RES_M(1),		/* R39 */
	0,				/* no r6 */
	0,				/* no r7 */
	0,				/* no r8 */
	CAP_U(0.22),	/* C16 */
	12,				/* vP */
};


static const discrete_op_amp_1sht_info invad2ct_brd2_invader_hit_1sht =
{
	DISC_OP_AMP_1SHT_1 | DISC_OP_AMP_IS_NORTON,
	RES_M(4.7),		/* R49 */
	RES_K(100),		/* R51 */
	RES_M(1),		/* R48 */
	RES_M(1),		/* R50 */
	RES_M(2.2),		/* R52 */
	CAP_U(1),		/* C18 */
	CAP_P(470),		/* C20 */
	0,				/* vN */
	12				/* vP */
};


static const discrete_op_amp_osc_info invad2ct_brd2_invader_hit_osc =
{
	DISC_OP_AMP_OSCILLATOR_1 | DISC_OP_AMP_IS_NORTON | DISC_OP_AMP_OSCILLATOR_OUT_CAP,
	RES_M(1),		/* R37 */
	RES_K(10),		/* R41 */
	RES_K(100),		/* R38 */
	RES_K(120),		/* R40 */
	RES_M(1),		/* R39 */
	0,				/* no r6 */
	0,				/* no r7 */
	0,				/* no r8 */
	CAP_U(0.1),		/* C16 */
	12,				/* vP */
};


static const discrete_op_amp_1sht_info invad2ct_missle_1sht =
{
	DISC_OP_AMP_1SHT_1 | DISC_OP_AMP_IS_NORTON,
	RES_M(4.7),		/* R32 */
	RES_K(100),		/* R30 */
	RES_M(1),		/* R31 */
	RES_M(1),		/* R33 */
	RES_M(2.2),		/* R34 */
	CAP_U(0.22),	/* C12 */
	CAP_P(470),		/* C15 */
	0,				/* vN */
	12				/* vP */
};


static const discrete_mixer_desc invad2ct_mixer =
{
	DISC_MIXER_IS_OP_AMP,		/* type */
	{ RES_K(100),				/* R78 */
	  RES_K(15) + 100 + 100,	/* R134 + R133 + R132 */
	  RES_K(150),				/* R136 */
	  RES_K(150),				/* R59 */
	  RES_K(10) + RES_K(6.8) + RES_K(5.6),	/* R86 + R85 + R84 */
	  RES_K(150) },				/* R28 */
	{0},						/* no rNode{} */
	{ 0,
	  0,
	  0,
	  0,
	  0,
	  CAP_U(0.001) },			/* C11 */
	0,							/* no rI */
	RES_K(100),					/* R105 */
	0,							/* no cF */
	CAP_U(0.1),					/* C45 */
	0,							/* vRef = ground */
	1							/* gain */
};


static DISCRETE_SOUND_START(invad2ct)
	/* sound board 1 */
	/* P.C. A082-90700-A000 */
	/* Schematic M051-00851-A002 */
	INVADERS_NOISE_GENERATOR
	INVADERS_SAUCER_HIT(1)
	INVADERS_FLEET(1)
	INVADERS_BONUS_MISSLE_BASE(1)
	INVADERS_INVADER_HIT(1, invad2ct)
	INVADERS_EXPLOSION(1)
	INVADERS_MISSILE(1, invad2ct)
	INVADERS_MIXER(1, invad2ct)

	/* sound board 2 */
	/* P.C. A084-90901-C851 */
	/* Schematic M051-00851-A005 */
	INVADERS_SAUCER_HIT(2)
	INVADERS_FLEET(2)
	INVADERS_BONUS_MISSLE_BASE(2)
	INVADERS_INVADER_HIT(2, invad2ct_brd2)
	INVADERS_EXPLOSION(2)
	INVADERS_MISSILE(2, invaders)
	INVADERS_MIXER(2, invaders)
DISCRETE_SOUND_END


MACHINE_DRIVER_START( invad2ct_audio )
	MDRV_SPEAKER_STANDARD_STEREO("spk1", "spk2")

	MDRV_SOUND_ADD("discrete", DISCRETE, 0)
	MDRV_SOUND_CONFIG_DISCRETE(invad2ct)
	MDRV_SOUND_ROUTE(0, "spk1", 0.5)
	MDRV_SOUND_ROUTE(1, "spk2", 0.5)

	MDRV_SOUND_ADD("sn1", SN76477, 0)
	MDRV_SOUND_CONFIG(invad2ct_p1_sn76477_interface)
	MDRV_SOUND_ROUTE(ALL_OUTPUTS, "spk1", 0.3)

	MDRV_SOUND_ADD("sn2", SN76477, 0)
	MDRV_SOUND_CONFIG(invad2ct_p2_sn76477_interface)
	MDRV_SOUND_ROUTE(ALL_OUTPUTS, "spk2", 0.3)
MACHINE_DRIVER_END


WRITE8_DEVICE_HANDLER( invad2ct_audio_1_w )
{
	const device_config *sn = devtag_get_device(device->machine, "sn1");

	sn76477_enable_w(sn, (~data >> 0) & 0x01);	/* saucer sound */

	discrete_sound_w(device, INVADERS_NODE(INVADERS_MISSILE_EN, 1), data & 0x02);
	discrete_sound_w(device, INVADERS_NODE(INVADERS_EXPLOSION_EN, 1), data & 0x04);
	discrete_sound_w(device, INVADERS_NODE(INVADERS_INVADER_HIT_EN, 1), data & 0x08);
	discrete_sound_w(device, INVADERS_NODE(INVADERS_BONUS_MISSLE_BASE_EN, 1), data & 0x10);

	sound_global_enable(data & 0x20);

	/* D6 and D7 are not connected */
}


WRITE8_DEVICE_HANDLER( invad2ct_audio_2_w )
{
	discrete_sound_w(device, INVADERS_NODE(INVADERS_FLEET_DATA, 1), data & 0x0f);
	discrete_sound_w(device, INVADERS_NODE(INVADERS_SAUCER_HIT_EN, 1), data & 0x10);

	/* D5-D7 are not connected */
}


WRITE8_DEVICE_HANDLER( invad2ct_audio_3_w )
{
	const device_config *sn = devtag_get_device(device->machine, "sn2");

	sn76477_enable_w(sn, (~data >> 0) & 0x01);	/* saucer sound */

	discrete_sound_w(device, INVADERS_NODE(INVADERS_MISSILE_EN, 2), data & 0x02);
	discrete_sound_w(device, INVADERS_NODE(INVADERS_EXPLOSION_EN, 2), data & 0x04);
	discrete_sound_w(device, INVADERS_NODE(INVADERS_INVADER_HIT_EN, 2), data & 0x08);
	discrete_sound_w(device, INVADERS_NODE(INVADERS_BONUS_MISSLE_BASE_EN, 2), data & 0x10);

	/* D5-D7 are not connected */
}


WRITE8_DEVICE_HANDLER( invad2ct_audio_4_w )
{
	discrete_sound_w(device, INVADERS_NODE(INVADERS_FLEET_DATA, 2), data & 0x0f);
	discrete_sound_w(device, INVADERS_NODE(INVADERS_SAUCER_HIT_EN, 2), data & 0x10);

	/* D5-D7 are not connected */
}
