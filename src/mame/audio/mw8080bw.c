// license:???
// copyright-holders:Michael Strutts, Nicola Salmoria, Tormod Tjaberg, Mirko Buffoni,Lee Taylor, Valerio Verrando, Marco Cassili, Zsolt Vasvari,Derrick Renaud
/***************************************************************************

    Midway 8080-based black and white hardware

****************************************************************************/

#include "emu.h"
#include "includes/mw8080bw.h"


/*************************************
 *
 *  Audio setup
 *
 *************************************/

SOUND_START_MEMBER( mw8080bw_state, samples )
{
	/* setup for save states */
	save_item(NAME(m_port_1_last));
	save_item(NAME(m_port_2_last));
}


/*************************************
 *
 *  Implementation of tone generator used
 *  by a few of these games
 *
 *************************************/

#define MIDWAY_TONE_EN              NODE_100
#define MIDWAY_TONE_DATA_L          NODE_101
#define MIDWAY_TONE_DATA_H          NODE_102
#define MIDWAY_TONE_SND             NODE_103
#define MIDWAY_TONE_TRASFORM_OUT    NODE_104
#define MIDWAY_TONE_BEFORE_AMP_SND  NODE_105


#define MIDWAY_TONE_GENERATOR(discrete_op_amp_tvca_info) \
		/* bit 0 of tone data is always 0 */ \
		/* join the L & H tone bits */ \
		DISCRETE_INPUT_LOGIC(MIDWAY_TONE_EN) \
		DISCRETE_INPUT_DATA (MIDWAY_TONE_DATA_L) \
		DISCRETE_INPUT_DATA (MIDWAY_TONE_DATA_H) \
		DISCRETE_TRANSFORM4(MIDWAY_TONE_TRASFORM_OUT, MIDWAY_TONE_DATA_H, 0x40, MIDWAY_TONE_DATA_L, 0x02, "01*23*+") \
		DISCRETE_NOTE(MIDWAY_TONE_BEFORE_AMP_SND, 1, (double)MW8080BW_MASTER_CLOCK/10/2, MIDWAY_TONE_TRASFORM_OUT, 0xfff, 1, DISC_CLK_IS_FREQ) \
		DISCRETE_OP_AMP_TRIG_VCA(MIDWAY_TONE_SND, MIDWAY_TONE_BEFORE_AMP_SND, MIDWAY_TONE_EN, 0, 12, 0, &discrete_op_amp_tvca_info)


/* most common values based on clowns schematic */
static const discrete_op_amp_tvca_info midway_music_tvca_info =
{
	RES_M(3.3),             /* r502 */
	RES_K(10) + RES_K(680), /* r505 + r506 */
	0,
	RES_K(680),             /* r503 */
	RES_K(10),              /* r500 */
	0,
	RES_K(680),             /* r501 */
	0,
	0,
	0,
	0,
	CAP_U(.001),            /* c500 */
	0,
	0, 0,
	12,         /* v1 */
	0,          /* v2 */
	0,          /* v3 */
	12,         /* vP */
	DISC_OP_AMP_TRIGGER_FUNCTION_TRG0,
	DISC_OP_AMP_TRIGGER_FUNCTION_NONE,
	DISC_OP_AMP_TRIGGER_FUNCTION_TRG1,
	DISC_OP_AMP_TRIGGER_FUNCTION_NONE,
	DISC_OP_AMP_TRIGGER_FUNCTION_NONE,
	DISC_OP_AMP_TRIGGER_FUNCTION_NONE
};


WRITE8_MEMBER(mw8080bw_state::midway_tone_generator_lo_w)
{
	m_discrete->write(space, MIDWAY_TONE_EN, (data >> 0) & 0x01);

	m_discrete->write(space, MIDWAY_TONE_DATA_L, (data >> 1) & 0x1f);

	/* D6 and D7 are not connected */
}


WRITE8_MEMBER(mw8080bw_state::midway_tone_generator_hi_w)
{
	m_discrete->write(space, MIDWAY_TONE_DATA_H, data & 0x3f);

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
	17,                 /* bit length */
						/* the RC network fed into pin 4, has the effect
						   of presetting all bits high at power up */
	0x1ffff,            /* reset value */
	4,                  /* use bit 4 as XOR input 0 */
	16,                 /* use bit 16 as XOR input 1 */
	DISC_LFSR_XOR,      /* feedback stage1 is XOR */
	DISC_LFSR_OR,       /* feedback stage2 is just stage 1 output OR with external feed */
	DISC_LFSR_REPLACE,  /* feedback stage3 replaces the shifted register contents */
	0x000001,           /* everything is shifted into the first bit only */
	0,                  /* output is not inverted */
	12                  /* output bit */
};



/*************************************
 *
 *  Sea Wolf
 *
 *************************************/

static const char *const seawolf_sample_names[] =
{
	"*seawolf",
	"shiphit",
	"torpedo",
	"dive",
	"sonar",
	"minehit",
	0
};

MACHINE_CONFIG_FRAGMENT( seawolf_audio )
	MCFG_SOUND_START_OVERRIDE(mw8080bw_state, samples)

	MCFG_SPEAKER_STANDARD_MONO("mono")
	MCFG_SOUND_ADD("samples", SAMPLES, 0)
	MCFG_SAMPLES_CHANNELS(5)
	MCFG_SAMPLES_NAMES(seawolf_sample_names)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.6)
MACHINE_CONFIG_END


WRITE8_MEMBER(mw8080bw_state::seawolf_audio_w)
{
	UINT8 rising_bits = data & ~m_port_1_last;

	/* if (data & 0x01)  enable SHIP HIT sound */
	if (rising_bits & 0x01) m_samples->start(0, 0);

	/* if (data & 0x02)  enable TORPEDO sound */
	if (rising_bits & 0x02) m_samples->start(1, 1);

	/* if (data & 0x04)  enable DIVE sound */
	if (rising_bits & 0x04) m_samples->start(2, 2);

	/* if (data & 0x08)  enable SONAR sound */
	if (rising_bits & 0x08) m_samples->start(3, 3);

	/* if (data & 0x10)  enable MINE HIT sound */
	if (rising_bits & 0x10) m_samples->start(4, 4);

	coin_counter_w(machine(), 0, (data >> 5) & 0x01);

	/* D6 and D7 are not connected */

	m_port_1_last = data;
}



/*************************************
 *
 *  Gun Fight
 *
 *************************************/

static const char *const gunfight_sample_names[] =
{
	"*gunfight",
	"gunshot",
	"killed",
	0
};


MACHINE_CONFIG_FRAGMENT( gunfight_audio )
	MCFG_SOUND_START_OVERRIDE(mw8080bw_state, samples)

	MCFG_SPEAKER_STANDARD_STEREO("lspeaker", "rspeaker")

	MCFG_SOUND_ADD("samples1", SAMPLES, 0)
	MCFG_SAMPLES_CHANNELS(1)
	MCFG_SAMPLES_NAMES(gunfight_sample_names)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "lspeaker", 0.50)

	MCFG_SOUND_ADD("samples2", SAMPLES, 0)
	MCFG_SAMPLES_CHANNELS(1)
	MCFG_SAMPLES_NAMES(gunfight_sample_names)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "rspeaker", 0.50)
MACHINE_CONFIG_END


WRITE8_MEMBER(mw8080bw_state::gunfight_audio_w)
{
	/* D0 and D1 are just tied to 1k resistors */

	coin_counter_w(machine(), 0, (data >> 2) & 0x01);

	/* the 74175 latches and inverts the top 4 bits */
	switch ((~data >> 4) & 0x0f)
	{
	case 0x00:
		break;

	case 0x01:
		/* enable LEFT SHOOT sound (left speaker) */
		m_samples1->start(0, 0);
		break;

	case 0x02:
		/* enable RIGHT SHOOT sound (right speaker) */
		m_samples2->start(0, 0);
		break;

	case 0x03:
		/* enable LEFT HIT sound (left speaker) */
		m_samples1->start(0, 1);
		break;

	case 0x04:
		/* enable RIGHT HIT sound (right speaker) */
		m_samples2->start(0, 1);
		break;

	default:
		logerror("%04x:  Unknown sh port write %02x\n", space.device().safe_pc(), data);
		break;
	}
}



/*************************************
 *
 *  Tornado Baseball
 *
 *************************************/

#define TORNBASE_SQUAREW_240        NODE_01
#define TORNBASE_SQUAREW_960        NODE_02
#define TORNBASE_SQUAREW_120        NODE_03

#define TORNBASE_TONE_240_EN        NODE_04
#define TORNBASE_TONE_960_EN        NODE_05
#define TORNBASE_TONE_120_EN        NODE_06

#define TORNBASE_TONE_240_SND       NODE_07
#define TORNBASE_TONE_960_SND       NODE_08
#define TORNBASE_TONE_120_SND       NODE_09
#define TORNBASE_TONE_SND           NODE_10
#define TORNBASE_TONE_SND_FILT      NODE_11


static DISCRETE_SOUND_START(tornbase)

	/* the 3 enable lines coming out of the 74175 flip-flop at G5 */
	DISCRETE_INPUT_LOGIC(TORNBASE_TONE_240_EN)      /* pin 2 */
	DISCRETE_INPUT_LOGIC(TORNBASE_TONE_960_EN)      /* pin 7 */
	DISCRETE_INPUT_LOGIC(TORNBASE_TONE_120_EN)      /* pin 5 */

	/* 3 different freq square waves (240, 960 and 120Hz).
	   Originates from the CPU board via an edge connector.
	   The wave is in the 0/+1 range */
	DISCRETE_SQUAREWFIX(TORNBASE_SQUAREW_240, 1, 240, 1.0, 50.0, 1.0/2, 0)  /* pin X */
	DISCRETE_SQUAREWFIX(TORNBASE_SQUAREW_960, 1, 960, 1.0, 50.0, 1.0/2, 0)  /* pin Y */
	DISCRETE_SQUAREWFIX(TORNBASE_SQUAREW_120, 1, 120, 1.0, 50.0, 1.0/2, 0)  /* pin V */

	/* 7403 O/C NAND gate at G6.  3 of the 4 gates used with their outputs tied together */
	DISCRETE_LOGIC_NAND(TORNBASE_TONE_240_SND, TORNBASE_SQUAREW_240, TORNBASE_TONE_240_EN)  /* pins 4,5,6 */
	DISCRETE_LOGIC_NAND(TORNBASE_TONE_960_SND, TORNBASE_SQUAREW_960, TORNBASE_TONE_960_EN)  /* pins 2,1,3 */
	DISCRETE_LOGIC_NAND(TORNBASE_TONE_120_SND, TORNBASE_SQUAREW_120, TORNBASE_TONE_120_EN)  /* pins 13,12,11 */
	DISCRETE_LOGIC_AND3(TORNBASE_TONE_SND,     TORNBASE_TONE_240_SND, TORNBASE_TONE_960_SND, TORNBASE_TONE_120_SND)

	/* 47K resistor (R601) and 0.047uF capacitor (C601)
	   There is also a 50K pot acting as a volume control, but we output at
	   the maximum volume as MAME has its own volume adjustment */
	DISCRETE_CRFILTER(TORNBASE_TONE_SND_FILT, TORNBASE_TONE_SND, RES_K(47), CAP_U(0.047))

	/* amplify for output */
	DISCRETE_OUTPUT(TORNBASE_TONE_SND_FILT, 32767)

DISCRETE_SOUND_END


MACHINE_CONFIG_FRAGMENT( tornbase_audio )
	MCFG_SPEAKER_STANDARD_MONO("mono")
	MCFG_SOUND_ADD("discrete", DISCRETE, 0)
	MCFG_DISCRETE_INTF(tornbase)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 1)
MACHINE_CONFIG_END


WRITE8_MEMBER(mw8080bw_state::tornbase_audio_w)
{
	m_discrete->write(space, TORNBASE_TONE_240_EN, (data >> 0) & 0x01);

	m_discrete->write(space, TORNBASE_TONE_960_EN, (data >> 1) & 0x01);

	m_discrete->write(space, TORNBASE_TONE_120_EN, (data >> 2) & 0x01);

	/* if (data & 0x08)  enable SIREN sound */

	/* if (data & 0x10)  enable CHEER sound */

	if (tornbase_get_cabinet_type() == TORNBASE_CAB_TYPE_UPRIGHT_OLD)
	{
		/* if (data & 0x20)  enable WHISTLE sound */

		/* D6 is not connected on this cabinet type */
	}
	else
	{
		/* D5 is not connected on this cabinet type */

		/* if (data & 0x40)  enable WHISTLE sound */
	}

	coin_counter_w(machine(), 0, (data >> 7) & 0x01);
}



/*************************************
 *
 *  280 ZZZAP / Laguna Racer
 *
 *************************************/

MACHINE_CONFIG_FRAGMENT( zzzap_audio )
	MCFG_SPEAKER_STANDARD_MONO("mono")
MACHINE_CONFIG_END


WRITE8_MEMBER(mw8080bw_state::zzzap_audio_1_w)
{
	/* set ENGINE SOUND FREQ(data & 0x0f)  the value written is
	                                       the gas pedal position */

	/* if (data & 0x10)  enable HI SHIFT engine sound modifier */

	/* if (data & 0x20)  enable LO SHIFT engine sound modifier */

	/* D6 and D7 are not connected */
}


WRITE8_MEMBER(mw8080bw_state::zzzap_audio_2_w)
{
	/* if (data & 0x01)  enable BOOM sound */

	/* if (data & 0x02)  enable ENGINE sound (global) */

	/* if (data & 0x04)  enable CR 1 (screeching sound) */

	/* if (data & 0x08)  enable NOISE CR 2 (happens only after the car blows up, but
	                                        before it appears again, not sure what
	                                        it is supposed to sound like) */

	coin_counter_w(machine(), 0, (data >> 5) & 0x01);

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
#define MAZE_P1_DATA             NODE_01
#define MAZE_P2_DATA             NODE_02
#define MAZE_TONE_TIMING         NODE_03
#define MAZE_COIN                NODE_04

/* nodes - other */
#define MAZE_JOYSTICK_IN_USE     NODE_11
#define MAZE_AUDIO_ENABLE        NODE_12
#define MAZE_TONE_ENABLE         NODE_13
#define MAZE_GAME_OVER           NODE_14
#define MAZE_R305_306_308        NODE_15
#define MAZE_R303_309            NODE_16
#define MAZE_PLAYER_SEL          NODE_17

/* nodes - sounds */
#define MAZE_SND                 NODE_18


static const discrete_555_desc maze_555_F2 =
{
	DISC_555_OUT_SQW | DISC_555_OUT_DC | DISC_555_TRIGGER_IS_LOGIC | DISC_555_TRIGGER_DISCHARGES_CAP,
	5,              /* B+ voltage of 555 */
	DEFAULT_555_VALUES
};


static const double maze_74147_table[] =
{
	3, 3, 3, 3, 3, 3, 3, 3, 0, 0, 0, 0, 1, 1, 2, 3
};


static const discrete_comp_adder_table maze_r305_306_308 =
{
	DISC_COMP_P_RESISTOR,   /* type of circuit */
	RES_K(100),             /* R308 */
	2,                      /* length */
	{ RES_M(1.5),           /* R304 */
		RES_K(820) }            /* R304 */
};


static const discrete_comp_adder_table maze_r303_309 =
{
	DISC_COMP_P_RESISTOR,   /* type of circuit */
	RES_K(330),             /* R309 */
	1,                      /* length */
	{ RES_M(1) }            /* R303 */
};


static const discrete_op_amp_osc_info maze_op_amp_osc =
{
	DISC_OP_AMP_OSCILLATOR_1 | DISC_OP_AMP_IS_NORTON | DISC_OP_AMP_OSCILLATOR_OUT_SQW,  /* type */
	RES_M(1),           /* R306 */
	RES_K(430),         /* R307 */
	MAZE_R305_306_308,  /* R304, R305, R308 switchable circuit */
	MAZE_R303_309,      /* R303, R309 switchable circuit */
	RES_K(330),         /* R310 */
	0, 0, 0,            /* not used */
	CAP_P(3300),        /* C300 */
	5                   /* vP */
};


static DISCRETE_SOUND_START(maze)

	/************************************************
	 * Input register mapping
	 ************************************************/
	DISCRETE_INPUT_DATA (MAZE_P1_DATA)
	DISCRETE_INPUT_DATA (MAZE_P2_DATA)
	DISCRETE_INPUT_LOGIC(MAZE_TONE_TIMING)
	DISCRETE_INPUT_LOGIC(MAZE_COIN)
	DISCRETE_INPUT_LOGIC(MAZE_JOYSTICK_IN_USE)  /* IC D2, pin 8 */

	/* The following circuits control when audio is heard. */
	/* Basically there is sound for 30s after a coin is inserted. */
	/* This time is extended whenever a control is pressed. */
	/* After the 30s has expired, there is no sound until the next coin is inserted. */
	/* There is also sound for the first 30s after power up even without a coin. */
	DISCRETE_LOGIC_INVERT(NODE_20,              /* IC E2, pin 8 */
					MAZE_JOYSTICK_IN_USE)       /* IN0 */
	DISCRETE_555_MSTABLE(MAZE_GAME_OVER,        /* IC F2, pin 3 */
					1,                          /* RESET */
					NODE_20,                    /* TRIG */
					RES_K(270),                 /* R203 */
					CAP_U(100),                 /* C204 */
					&maze_555_F2)
	DISCRETE_LOGIC_JKFLIPFLOP(MAZE_AUDIO_ENABLE,/* IC F1, pin 5 */
					MAZE_COIN,                  /* RESET */
					1,                          /* SET */
					MAZE_GAME_OVER,             /* CLK */
					1,                          /* J */
					0)                          /* K */
	DISCRETE_LOGIC_INVERT(MAZE_TONE_ENABLE,     /* IC F1, pin 6 */
					MAZE_AUDIO_ENABLE)          /* IN0 */
	DISCRETE_LOGIC_AND3(NODE_21,
					MAZE_JOYSTICK_IN_USE,       /* INP0 */
					MAZE_TONE_ENABLE,           /* INP1 */
					MAZE_TONE_TIMING)           /* INP2 */

	/* The following circuits use the control info to generate a tone. */
	DISCRETE_LOGIC_JKFLIPFLOP(MAZE_PLAYER_SEL,  /* IC C1, pin 3 */
					1,                          /* RESET */
					1,                          /* SET */
					MAZE_TONE_TIMING,           /* CLK */
					1,                          /* J */
					1)                          /* K */
	DISCRETE_MULTIPLEX2(NODE_31,                /* IC D1 */
					MAZE_PLAYER_SEL,            /* ADDR */
					MAZE_P1_DATA,               /* INP0 */
					MAZE_P2_DATA)               /* INP1 */
	DISCRETE_LOOKUP_TABLE(NODE_32,              /* IC E1 */
					NODE_31,                    /* ADDR */
					16,                         /* SIZE */
					&maze_74147_table)
	DISCRETE_COMP_ADDER(MAZE_R305_306_308,      /* value of selected parallel circuit R305, R306, R308 */
					NODE_32,                    /* DATA */
					&maze_r305_306_308)
	DISCRETE_COMP_ADDER(MAZE_R303_309,          /* value of selected parallel circuit R303, R309 */
					MAZE_PLAYER_SEL,            /* DATA */
					&maze_r303_309)
	DISCRETE_OP_AMP_OSCILLATOR(NODE_36,         /* IC J1, pin 4 */
					1,                          /* ENAB */
					&maze_op_amp_osc)

	/* The following circuits remove DC poping noises when the tone is switched in/out. */
	DISCRETE_CRFILTER_VREF(NODE_40,
					NODE_36,                    /* IN0 */
					RES_K(250),                 /* R311, R312, R402, R403 in parallel */
					CAP_U(0.1),                 /* c301 */
					2.5)                        /* center voltage of R311, R312 */
	DISCRETE_SWITCH(NODE_41,                    /* IC H3, pin 10 */
					1,                          /* ENAB */
					NODE_21,                    /* switch */
					2.5,                        /* INP0 - center voltage of R402, R403 */
					NODE_40)                    /* INP1 */
	DISCRETE_CRFILTER(NODE_42,
					NODE_41,                    /* IN0 */
					RES_K(56 + 390),            /* R404 + R405 */
					CAP_P(0.01) )               /* C401 */
	DISCRETE_RCFILTER(NODE_43,
					NODE_42,                    /* IN0 */
					RES_K(56),                  /* R404 */
					CAP_P(4700) )               /* C400 */
	DISCRETE_SWITCH(MAZE_SND,                   /* H3 saturates op-amp J3 when enabled, disabling audio */
					1,                          /* ENAB */
					MAZE_AUDIO_ENABLE,          /* SWITCH */
					NODE_43,                    /* INP0 */
					0)                          /* INP1 */

	DISCRETE_OUTPUT(MAZE_SND, 96200)
DISCRETE_SOUND_END


MACHINE_CONFIG_FRAGMENT( maze_audio )
	MCFG_SPEAKER_STANDARD_MONO("mono")
	MCFG_SOUND_ADD("discrete", DISCRETE, 0)
	MCFG_DISCRETE_INTF(maze)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 1.0)
MACHINE_CONFIG_END


void mw8080bw_state::maze_write_discrete(UINT8 maze_tone_timing_state)
{
	/* controls need to be active low */
	int controls = ~ioport("IN0")->read() & 0xff;

	address_space &space = machine().driver_data()->generic_space();
	m_discrete->write(space, MAZE_TONE_TIMING, maze_tone_timing_state);
	m_discrete->write(space, MAZE_P1_DATA, controls & 0x0f);
	m_discrete->write(space, MAZE_P2_DATA, (controls >> 4) & 0x0f);
	m_discrete->write(space, MAZE_JOYSTICK_IN_USE, controls != 0xff);

	/* The coin line is connected directly to the discrete circuit. */
	/* We can't really do that, so updating it with the tone timing is close enough. */
	/* A better option might be to update it at vblank or set a timer to do it. */
	/* The only noticeable difference doing it here, is that the controls don't */
	/* immediately start making tones if pressed right after the coin is inserted. */
	m_discrete->write(space, MAZE_COIN, (~ioport("IN1")->read() >> 3) & 0x01);
}



/*************************************
 *
 *  Boot Hill
 *
 *  Discrete sound emulation: Jan 2007, D.R.
 *
 *************************************/

/* nodes - inputs */
#define BOOTHILL_GAME_ON_EN         NODE_01
#define BOOTHILL_LEFT_SHOT_EN       NODE_02
#define BOOTHILL_RIGHT_SHOT_EN      NODE_03
#define BOOTHILL_LEFT_HIT_EN        NODE_04
#define BOOTHILL_RIGHT_HIT_EN       NODE_05

/* nodes - sounds */
#define BOOTHILL_NOISE              NODE_06
#define BOOTHILL_L_SHOT_SND         NODE_07
#define BOOTHILL_R_SHOT_SND         NODE_08
#define BOOTHILL_L_HIT_SND          NODE_09
#define BOOTHILL_R_HIT_SND          NODE_10

/* nodes - adjusters */
#define BOOTHILL_MUSIC_ADJ          NODE_11


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
	0, 0,
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
	0, 0,
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
	0, 0,
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
	7200    /* final gain */
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
	7200    /* final gain */
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
	DISCRETE_ADJUSTMENT(BOOTHILL_MUSIC_ADJ, RES_M(1), 75000, DISC_LOGADJ, "MUSIC_ADJ")

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
	DISCRETE_RCFILTER(NODE_31, NODE_30, RES_K(12), CAP_U(.01))
	DISCRETE_RCFILTER(BOOTHILL_L_SHOT_SND, NODE_31, RES_K(12) + RES_K(68), CAP_U(.0022))

	DISCRETE_OP_AMP_TRIG_VCA(NODE_35, BOOTHILL_RIGHT_SHOT_EN, 0, 0, BOOTHILL_NOISE, 0, &boothill_shot_tvca_info)
	DISCRETE_RCFILTER(NODE_36, NODE_35, RES_K(12), CAP_U(.01))
	DISCRETE_RCFILTER(BOOTHILL_R_SHOT_SND, NODE_36, RES_K(12) + RES_K(68), CAP_U(.0033))

	/************************************************
	 * Hit sounds
	 ************************************************/
	DISCRETE_OP_AMP_TRIG_VCA(NODE_40, BOOTHILL_LEFT_HIT_EN, 0, 0, BOOTHILL_NOISE, 0, &boothill_hit_tvca_info)
	DISCRETE_RCFILTER(NODE_41, NODE_40, RES_K(12), CAP_U(.033))
	DISCRETE_RCFILTER(BOOTHILL_L_HIT_SND, NODE_41, RES_K(12) + RES_K(100), CAP_U(.0033))

	DISCRETE_OP_AMP_TRIG_VCA(NODE_45, BOOTHILL_RIGHT_HIT_EN, 0, 0, BOOTHILL_NOISE, 0, &boothill_hit_tvca_info)
	DISCRETE_RCFILTER(NODE_46, NODE_45, RES_K(12), CAP_U(.0033))
	DISCRETE_RCFILTER(BOOTHILL_R_HIT_SND, NODE_46, RES_K(12) + RES_K(100), CAP_U(.0022))

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


MACHINE_CONFIG_FRAGMENT( boothill_audio )
	MCFG_SPEAKER_STANDARD_STEREO("lspeaker", "rspeaker")
	MCFG_SOUND_ADD("discrete", DISCRETE, 0)
	MCFG_DISCRETE_INTF(boothill)
	MCFG_SOUND_ROUTE(0, "lspeaker", 1.0)
	MCFG_SOUND_ROUTE(1, "rspeaker", 1.0)
MACHINE_CONFIG_END


WRITE8_MEMBER(mw8080bw_state::boothill_audio_w)
{
	/* D0 and D1 are not connected */

	coin_counter_w(machine(), 0, (data >> 2) & 0x01);

	m_discrete->write(space, BOOTHILL_GAME_ON_EN, (data >> 3) & 0x01);

	m_discrete->write(space, BOOTHILL_LEFT_SHOT_EN, (data >> 4) & 0x01);

	m_discrete->write(space, BOOTHILL_RIGHT_SHOT_EN, (data >> 5) & 0x01);

	m_discrete->write(space, BOOTHILL_LEFT_HIT_EN, (data >> 6) & 0x01);

	m_discrete->write(space, BOOTHILL_RIGHT_HIT_EN, (data >> 7) & 0x01);
}



/*************************************
 *
 *  Checkmate
 *
 *************************************/

/* nodes - inputs */
#define CHECKMAT_BOOM_EN            NODE_01
#define CHECKMAT_TONE_EN            NODE_02
#define CHECKMAT_TONE_DATA_45       NODE_03
#define CHECKMAT_TONE_DATA_67       NODE_04

/* nodes - other */
#define CHECKMAT_R401_402_400       NODE_06
#define CHECKMAT_R407_406_410       NODE_07

/* nodes - sounds */
#define CHECKMAT_BOOM_SND           NODE_10
#define CHECKMAT_TONE_SND           NODE_11
#define CHECKMAT_FINAL_SND          NODE_12

/* nodes - adjusters */
#define CHECKMAT_R309               NODE_15
#define CHECKMAT_R411               NODE_16


static const discrete_comp_adder_table checkmat_r401_402_400 =
{
	DISC_COMP_P_RESISTOR,   /* type of circuit */
	RES_K(100),             /* R401 */
	2,                      /* length */
	{ RES_M(1.5),           /* R402 */
		RES_K(820) }            /* R400 */
};


static const discrete_comp_adder_table checkmat_r407_406_410 =
{
	DISC_COMP_P_RESISTOR,   /* type of circuit */
	RES_K(330),             /* R407 */
	2,                      /* length */
	{ RES_M(1),             /* R406 */
		RES_K(510) }            /* R410 */
};


static const discrete_op_amp_osc_info checkmat_op_amp_osc =
{
	DISC_OP_AMP_OSCILLATOR_1 | DISC_OP_AMP_IS_NORTON | DISC_OP_AMP_OSCILLATOR_OUT_SQW,  /* type */
	RES_M(1),               /* R403 */
	RES_K(430),             /* R405 */
	CHECKMAT_R401_402_400,  /* R401, R402, R400 switchable circuit */
	CHECKMAT_R407_406_410,  /* R407, R406, R410 switchable circuit */
	RES_K(330),             /* R404 */
	0, 0, 0,                /* not used */
	CAP_P(3300),            /* C400 */
	5                       /* vP */
};


static const discrete_op_amp_tvca_info checkmat_op_amp_tvca =
{
	RES_M(1.2), /* R302 */
	RES_M(1),   /* R305 */
	0,          /* r3 - not used */
	RES_M(1.2), /* R304 */
	RES_K(1),   /* M4 */
	0,          /* r6 - not used */
	RES_M(1),   /* R303 */
	0,          /* r8 - not used */
	0,          /* r9 - not used */
	0,          /* r10 - not used */
	0,          /* r11 - not used */
	CAP_U(1),   /* C300 */
	0,          /* c2 - not used */
	0, 0,       /* c3, c4 - not used */
	5,          /* v1 */
	0,          /* v2 */
	0,          /* v3 */
	5,          /* vP */
	DISC_OP_AMP_TRIGGER_FUNCTION_NONE,  /* f0 - not used */
	DISC_OP_AMP_TRIGGER_FUNCTION_NONE,  /* f1 - not used */
	DISC_OP_AMP_TRIGGER_FUNCTION_TRG0,  /* f2 */
	DISC_OP_AMP_TRIGGER_FUNCTION_NONE,  /* f3 - not used */
	DISC_OP_AMP_TRIGGER_FUNCTION_NONE,  /* f4 - not used */
	DISC_OP_AMP_TRIGGER_FUNCTION_NONE,  /* f5 - not used */
};


static const discrete_mixer_desc checkmat_mixer =
{
	DISC_MIXER_IS_OP_AMP,   /* type */
	{ RES_K(100),       /* R308 - VERIFY - can't read schematic */
		RES_K(56 + 47) },   /* R412 + R408 */
	{ CHECKMAT_R309,    /* R309 */
		CHECKMAT_R411}, /* R411 */
	{ CAP_U(10),        /* C305 */
		CAP_U(0.01) },  /* C401 */
	0,                  /* rI - not used */
	RES_K(100),         /* R507 */
	0,                  /* cF - not used */
	CAP_U(1),           /* C505 */
	0,                  /* vRef - GND */
	1                   /* gain */
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
	DISCRETE_ADJUSTMENT(CHECKMAT_R309, RES_K(100), 1000, DISC_LOGADJ, "R309")
	DISCRETE_ADJUSTMENT(CHECKMAT_R411, RES_M(1), 1000, DISC_LOGADJ, "R411")

	/************************************************
	 * Boom Sound
	 *
	 * The zener diode noise source is hard to
	 * emulate.  Guess for now.
	 ************************************************/
	/* FIX - find noise freq and amplitude */
	DISCRETE_NOISE(NODE_20,
					1,                          /* ENAB */
					1500,                       /* FREQ */
					2,                          /* AMP */
					0)                      /* BIAS */
	DISCRETE_OP_AMP_TRIG_VCA(NODE_21,
					CHECKMAT_BOOM_EN,           /* TRG0 */
					0,                          /* TRG1 - not used */
					0,                          /* TRG2 - not used */
					NODE_20,                    /* IN0 */
					0,                          /* IN1 - not used */
					&checkmat_op_amp_tvca)
	/* The next 5 modules emulate the filter. */
	DISCRETE_FILTER2(NODE_23,
					1,                          /* ENAB */
					NODE_21,                    /* INP0 */
					35,                         /* FREQ */
					1.0 / 8,                    /* DAMP */
					DISC_FILTER_BANDPASS)
	DISCRETE_GAIN(NODE_24,
					NODE_23,                    /* IN0 */
					15)                         /* GAIN */
	DISCRETE_CLAMP(CHECKMAT_BOOM_SND,           /* IC Q2/3, pin 10 */
					NODE_24,                    /* IN0 */
					0 - 6,                      /* MIN */
					12.0 - OP_AMP_NORTON_VBE -6)/* MAX */

	/************************************************
	 * Tone generator
	 ************************************************/
	DISCRETE_COMP_ADDER(CHECKMAT_R401_402_400,  /* value of selected parallel circuit R401, R402, R400 */
					CHECKMAT_TONE_DATA_45,      /* DATA */
					&checkmat_r401_402_400)
	DISCRETE_COMP_ADDER(CHECKMAT_R407_406_410,  /* value of selected parallel circuit R407, R406, R410 */
					CHECKMAT_TONE_DATA_67,      /* DATA */
					&checkmat_r407_406_410)
	DISCRETE_OP_AMP_OSCILLATOR(NODE_30,         /* IC N3/4, pin 4 */
					1,                          /* ENAB */
					&checkmat_op_amp_osc)

	/* The following circuits remove DC poping noises when the tone is switched in/out. */
	DISCRETE_CRFILTER_VREF(NODE_31,
					NODE_30,                    /* IN0 */
					RES_K(250),                 /* R409, R415, R414, R413 in parallel */
					CAP_U(0.1),                 /* c401 */
					2.5)                        /* center voltage of R409, R415 */
	DISCRETE_SWITCH(NODE_32,                    /* IC R3/4, pin 9 */
					1,                          /* ENAB */
					CHECKMAT_TONE_EN,           /* switch */
					2.5,                        /* INP0 - center voltage of R413, R414 */
					NODE_31)                    /* INP1 */
	DISCRETE_CRFILTER(NODE_33,
					NODE_32,                    /* IN0 */
					RES_K(56 + 47 + 200),       /* R412 + R408 + part of R411 */
					CAP_P(0.01) )               /* C404 */
	DISCRETE_RCFILTER(CHECKMAT_TONE_SND,
					NODE_33,                    /* IN0 */
					RES_K(56),                  /* R412 */
					CAP_P(4700) )               /* C403 */

	/************************************************
	 * Final mix and output
	 ************************************************/
	DISCRETE_MIXER2(CHECKMAT_FINAL_SND,
					1,                          /* ENAB */
					CHECKMAT_BOOM_SND,          /* IN0 */
					CHECKMAT_TONE_SND,          /* IN1 */
					&checkmat_mixer)
	DISCRETE_OUTPUT(CHECKMAT_FINAL_SND, 300000)
DISCRETE_SOUND_END


MACHINE_CONFIG_FRAGMENT( checkmat_audio )
	MCFG_SPEAKER_STANDARD_MONO("mono")
	MCFG_SOUND_ADD("discrete", DISCRETE, 0)
	MCFG_DISCRETE_INTF(checkmat)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.4)
MACHINE_CONFIG_END


WRITE8_MEMBER(mw8080bw_state::checkmat_audio_w)
{
	m_discrete->write(space, CHECKMAT_TONE_EN, data & 0x01);

	m_discrete->write(space, CHECKMAT_BOOM_EN, (data >> 1) & 0x01);

	coin_counter_w(machine(), 0, (data >> 2) & 0x01);

	machine().sound().system_enable((data >> 3) & 0x01);

	m_discrete->write(space, CHECKMAT_TONE_DATA_45, (data >> 4) & 0x03);
	m_discrete->write(space, CHECKMAT_TONE_DATA_67, (data >> 6) & 0x03);
}



/*************************************
 *
 *  Desert Gun
 *
 *  Discrete sound emulation: Jan 2007, D.R.
 *
 *************************************/

/* nodes - inputs */
#define DESERTGU_GAME_ON_EN                   NODE_01
#define DESERTGU_RIFLE_SHOT_EN                NODE_02
#define DESERTGU_BOTTLE_HIT_EN                NODE_03
#define DESERTGU_ROAD_RUNNER_HIT_EN           NODE_04
#define DESERTGU_CREATURE_HIT_EN              NODE_05
#define DESERTGU_ROADRUNNER_BEEP_BEEP_EN      NODE_06
#define DESERTGU_TRIGGER_CLICK_EN             NODE_07

/* nodes - sounds */
#define DESERTGU_NOISE                        NODE_08
#define DESERTGU_RIFLE_SHOT_SND               NODE_09
#define DESERTGU_BOTTLE_HIT_SND               NODE_10
#define DESERTGU_ROAD_RUNNER_HIT_SND          NODE_11
#define DESERTGU_CREATURE_HIT_SND             NODE_12
#define DESERTGU_ROADRUNNER_BEEP_BEEP_SND     NODE_13
#define DESERTGU_TRIGGER_CLICK_SND            DESERTGU_TRIGGER_CLICK_EN

/* nodes - adjusters */
#define DESERTGU_MUSIC_ADJ                    NODE_15


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
	0, 0,
	12,         /* v1 */
	0,          /* v2 */
	0,          /* v3 */
	12,         /* vP */
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
	6000    /* final gain */
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
	DISCRETE_ADJUSTMENT(DESERTGU_MUSIC_ADJ, RES_M(1), 75000, DISC_LOGADJ, "MUSIC_ADJ")

	/************************************************
	 * Tone generator
	 ************************************************/
	MIDWAY_TONE_GENERATOR(midway_music_tvca_info)

	/************************************************
	 * Rifle shot sound
	 ************************************************/
	/* Noise clock was breadboarded and measured at 7515Hz */
	DISCRETE_LFSR_NOISE(DESERTGU_NOISE, 1, 1, 7515, 12.0, 0, 12.0/2, &midway_lfsr)

	DISCRETE_OP_AMP_TRIG_VCA(NODE_30, DESERTGU_RIFLE_SHOT_EN, 0, 0, DESERTGU_NOISE, 0, &desertgu_rifle_shot_tvca_info)
	DISCRETE_RCFILTER(NODE_31, NODE_30, RES_K(12), CAP_U(.01))
	DISCRETE_CRFILTER(DESERTGU_RIFLE_SHOT_SND, NODE_31, RES_K(12) + RES_K(68), CAP_U(.0022))

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


MACHINE_CONFIG_FRAGMENT( desertgu_audio )
	MCFG_SPEAKER_STANDARD_MONO("mono")
	MCFG_SOUND_ADD("discrete", DISCRETE, 0)
	MCFG_DISCRETE_INTF(desertgu)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.8)
MACHINE_CONFIG_END


WRITE8_MEMBER(mw8080bw_state::desertgu_audio_1_w)
{
	/* D0 and D1 are not connected */

	coin_counter_w(machine(), 0, (data >> 2) & 0x01);

	m_discrete->write(space, DESERTGU_GAME_ON_EN, (data >> 3) & 0x01);

	m_discrete->write(space, DESERTGU_RIFLE_SHOT_EN, (data >> 4) & 0x01);

	m_discrete->write(space, DESERTGU_BOTTLE_HIT_EN, (data >> 5) & 0x01);

	m_discrete->write(space, DESERTGU_ROAD_RUNNER_HIT_EN, (data >> 6) & 0x01);

	m_discrete->write(space, DESERTGU_CREATURE_HIT_EN, (data >> 7) & 0x01);
}


WRITE8_MEMBER(mw8080bw_state::desertgu_audio_2_w)
{
	m_discrete->write(space, DESERTGU_ROADRUNNER_BEEP_BEEP_EN, (data >> 0) & 0x01);

	m_discrete->write(space, DESERTGU_TRIGGER_CLICK_EN, (data >> 1) & 0x01);

	output_set_value("Player1_Gun_Recoil", (data >> 2) & 0x01);

	m_desertgun_controller_select = (data >> 3) & 0x01;

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
#define DPLAY_GAME_ON_EN      NODE_01
#define DPLAY_TONE_ON_EN      NODE_02
#define DPLAY_SIREN_EN        NODE_03
#define DPLAY_WHISTLE_EN      NODE_04
#define DPLAY_CHEER_EN        NODE_05

/* nodes - sounds */
#define DPLAY_NOISE           NODE_06
#define DPLAY_TONE_SND        NODE_07
#define DPLAY_SIREN_SND       NODE_08
#define DPLAY_WHISTLE_SND     NODE_09
#define DPLAY_CHEER_SND       NODE_10

/* nodes - adjusters */
#define DPLAY_MUSIC_ADJ       NODE_11


static const discrete_lfsr_desc dplay_lfsr =
{
	DISC_CLK_IS_FREQ,
	17,                 /* bit length */
						/* the RC network fed into pin 4, has the effect
						   of presetting all bits high at power up */
	0x1ffff,            /* reset value */
	4,                  /* use bit 4 as XOR input 0 */
	16,                 /* use bit 16 as XOR input 1 */
	DISC_LFSR_XOR,      /* feedback stage1 is XOR */
	DISC_LFSR_OR,       /* feedback stage2 is just stage 1 output OR with external feed */
	DISC_LFSR_REPLACE,  /* feedback stage3 replaces the shifted register contents */
	0x000001,           /* everything is shifted into the first bit only */
	0,                  /* output is not inverted */
	8                   /* output bit */
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
	DISC_OP_AMP_OSCILLATOR_VCO_2 | DISC_OP_AMP_IS_NORTON | DISC_OP_AMP_OSCILLATOR_OUT_SQW,  /* type */
	RES_K(390),     /* r1 */
	RES_M(5.6),     /* r2 */
	RES_M(1),       /* r3 */
	RES_M(1.5),     /* r4 */
	RES_M(3.3),     /* r5 */
	RES_K(56),      /* r6 */
	0,              /* no r7 */
	0,              /* no r8 */
	CAP_U(0.0022),  /* c */
	12              /* vP */
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
	DISC_OP_AMP_OSCILLATOR_VCO_2 | DISC_OP_AMP_IS_NORTON | DISC_OP_AMP_OSCILLATOR_OUT_SQW,  /* type */
	RES_K(510),     /* r1 */
	RES_M(5.6),     /* r2 */
	RES_M(1),       /* r3 */
	RES_M(1.5),     /* r4 */
	RES_M(3.3),     /* r5 */
	RES_K(300),     /* r6 */
	0,              /* no r7 */
	0,              /* no r8 */
	CAP_P(220),     /* c */
	12              /* vP */
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
	2000    /* final gain */
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
	DISCRETE_ADJUSTMENT(DPLAY_MUSIC_ADJ, RES_M(1), 1000, DISC_LOGADJ, "MUSIC_ADJ")

	/************************************************
	 * Music and Tone Generator
	 ************************************************/
	MIDWAY_TONE_GENERATOR(midway_music_tvca_info)

	DISCRETE_OP_AMP_TRIG_VCA(DPLAY_TONE_SND, MIDWAY_TONE_BEFORE_AMP_SND, DPLAY_TONE_ON_EN, 0, 12, 0, &midway_music_tvca_info)

	/************************************************
	 * Siren
	 ************************************************/
	DISCRETE_INTEGRATE(NODE_30,
					DPLAY_SIREN_EN,                 /* TRG0 */
					0           ,                   /* TRG1 */
					&dplay_siren_integrate_info)
	DISCRETE_OP_AMP_VCO1(DPLAY_SIREN_SND,
					1,                              /* ENAB */
					NODE_30,                        /* VMOD1 */
					&dplay_siren_osc)

	/************************************************
	 * Whistle
	 ************************************************/
	DISCRETE_INTEGRATE(NODE_40,
					DPLAY_WHISTLE_EN,               /* TRG0 */
					0           ,                   /* TRG1 */
					&dplay_whistle_integrate_info)
	DISCRETE_OP_AMP_VCO1(DPLAY_WHISTLE_SND,
					1,                              /* ENAB */
					NODE_40,                        /* VMOD1 */
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


MACHINE_CONFIG_FRAGMENT( dplay_audio )
	MCFG_SPEAKER_STANDARD_MONO("mono")
	MCFG_SOUND_ADD("discrete", DISCRETE, 0)
	MCFG_DISCRETE_INTF(dplay)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.8)
MACHINE_CONFIG_END


WRITE8_MEMBER(mw8080bw_state::dplay_audio_w)
{
	m_discrete->write(space, DPLAY_TONE_ON_EN, (data >> 0) & 0x01);

	m_discrete->write(space, DPLAY_CHEER_EN, (data >> 1) & 0x01);

	m_discrete->write(space, DPLAY_SIREN_EN, (data >> 2) & 0x01);

	m_discrete->write(space, DPLAY_WHISTLE_EN, (data >> 3) & 0x01);

	m_discrete->write(space, DPLAY_GAME_ON_EN, (data >> 4) & 0x01);

	coin_counter_w(machine(), 0, (data >> 5) & 0x01);

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
	"1",    /* missle */
	"2",    /* explosion */
	0
};

MACHINE_CONFIG_FRAGMENT( gmissile_audio )
	MCFG_SOUND_START_OVERRIDE(mw8080bw_state, samples)

	MCFG_SPEAKER_STANDARD_STEREO("lspeaker", "rspeaker")

	MCFG_SOUND_ADD("samples1", SAMPLES, 0)
	MCFG_SAMPLES_CHANNELS(1)
	MCFG_SAMPLES_NAMES(gmissile_sample_names)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "lspeaker", 1.9)

	MCFG_SOUND_ADD("samples2", SAMPLES, 0)
	MCFG_SAMPLES_CHANNELS(1)
	MCFG_SAMPLES_NAMES(gmissile_sample_names)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "rspeaker", 1.9)
MACHINE_CONFIG_END


WRITE8_MEMBER(mw8080bw_state::gmissile_audio_1_w)
{
	/* note that the schematics shows the left and right explosions
	   reversed (D5=R, D7=L), but the software confirms that
	   ours is right */

	UINT8 rising_bits = data & ~m_port_1_last;

	/* D0 and D1 are not connected */

	coin_counter_w(machine(), 0, (data >> 2) & 0x01);

	machine().sound().system_enable((data >> 3) & 0x01);

	/* if (data & 0x10)  enable RIGHT MISSILE sound (goes to right speaker) */
	if (rising_bits & 0x10) m_samples2->start(0, 0);

	/* if (data & 0x20)  enable LEFT EXPLOSION sound (goes to left speaker) */
	output_set_value("L_EXP_LIGHT", (data >> 5) & 0x01);
	if (rising_bits & 0x20) m_samples1->start(0, 1);

	/* if (data & 0x40)  enable LEFT MISSILE sound (goes to left speaker) */
	if (rising_bits & 0x40) m_samples1->start(0, 0);

	/* if (data & 0x80)  enable RIGHT EXPLOSION sound (goes to right speaker) */
	output_set_value("R_EXP_LIGHT", (data >> 7) & 0x01);
	if (rising_bits & 0x80) m_samples2->start(0, 1);

	m_port_1_last = data;
}


WRITE8_MEMBER(mw8080bw_state::gmissile_audio_2_w)
{
	/* set AIRPLANE/COPTER/JET PAN(data & 0x07) */

	/* set TANK PAN((data >> 3) & 0x07) */

	/* D6 and D7 are not connected */
}


WRITE8_MEMBER(mw8080bw_state::gmissile_audio_3_w)
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
	"1",    /* missle */
	"2",    /* explosion */
	0
};


MACHINE_CONFIG_FRAGMENT( m4_audio )
	MCFG_SOUND_START_OVERRIDE(mw8080bw_state, samples)

	MCFG_SPEAKER_STANDARD_STEREO("lspeaker", "rspeaker")

	MCFG_SOUND_ADD("samples1", SAMPLES, 0)
	MCFG_SAMPLES_CHANNELS(2)
	MCFG_SAMPLES_NAMES(m4_sample_names)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "lspeaker", 1)

	MCFG_SOUND_ADD("samples2", SAMPLES, 0)
	MCFG_SAMPLES_CHANNELS(2)
	MCFG_SAMPLES_NAMES(m4_sample_names)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "rspeaker", 1)
MACHINE_CONFIG_END


WRITE8_MEMBER(mw8080bw_state::m4_audio_1_w)
{
	UINT8 rising_bits = data & ~m_port_1_last;

	/* D0 and D1 are not connected */

	coin_counter_w(machine(), 0, (data >> 2) & 0x01);

	machine().sound().system_enable((data >> 3) & 0x01);

	/* if (data & 0x10)  enable LEFT PLAYER SHOT sound (goes to left speaker) */
	if (rising_bits & 0x10) m_samples1->start(0, 0);

	/* if (data & 0x20)  enable RIGHT PLAYER SHOT sound (goes to right speaker) */
	if (rising_bits & 0x20) m_samples2->start(0, 0);

	/* if (data & 0x40)  enable LEFT PLAYER EXPLOSION sound via 300K res (goes to left speaker) */
	if (rising_bits & 0x40) m_samples1->start(1, 1);

	/* if (data & 0x80)  enable RIGHT PLAYER EXPLOSION sound via 300K res (goes to right speaker) */
	if (rising_bits & 0x80) m_samples2->start(1, 1);

	m_port_1_last = data;
}


WRITE8_MEMBER(mw8080bw_state::m4_audio_2_w)
{
	UINT8 rising_bits = data & ~m_port_2_last;

	/* if (data & 0x01)  enable LEFT PLAYER EXPLOSION sound via 510K res (goes to left speaker) */
	if (rising_bits & 0x01) m_samples1->start(1, 1);

	/* if (data & 0x02)  enable RIGHT PLAYER EXPLOSION sound via 510K res (goes to right speaker) */
	if (rising_bits & 0x02) m_samples2->start(1, 1);

	/* if (data & 0x04)  enable LEFT TANK MOTOR sound (goes to left speaker) */

	/* if (data & 0x08)  enable RIGHT TANK MOTOR sound (goes to right speaker) */

	/* if (data & 0x10)  enable sound that is playing while the right plane is
	                     flying.  Circuit not named on schematics  (goes to left speaker) */

	/* if (data & 0x20)  enable sound that is playing while the left plane is
	                     flying.  Circuit not named on schematics  (goes to right speaker) */

	/* D6 and D7 are not connected */

	m_port_2_last = data;
}



/*************************************
 *
 *  Clowns
 *
 *  Discrete sound emulation: Mar 2005, D.R.
 *
 *************************************/

/* nodes - inputs */
#define CLOWNS_POP_BOTTOM_EN        NODE_01
#define CLOWNS_POP_MIDDLE_EN        NODE_02
#define CLOWNS_POP_TOP_EN           NODE_03
#define CLOWNS_SPRINGBOARD_HIT_EN   NODE_04
#define CLOWNS_SPRINGBOARD_MISS_EN  NODE_05

/* nodes - sounds */
#define CLOWNS_NOISE                NODE_06
#define CLOWNS_POP_SND              NODE_07
#define CLOWNS_SB_HIT_SND           NODE_08
#define CLOWNS_SB_MISS_SND          NODE_09

/* nodes - adjusters */
#define CLOWNS_R507_POT             NODE_11


static const discrete_op_amp_tvca_info clowns_pop_tvca_info =
{
	RES_M(2.7),     /* r304 */
	RES_K(680),     /* r303 */
	0,
	RES_K(680),     /* r305 */
	RES_K(1),       /* j3 */
	0,
	RES_K(470),     /* r300 */
	RES_K(1),       /* j3 */
	RES_K(510),     /* r301 */
	RES_K(1),       /* j3 */
	RES_K(680),     /* r302 */
	CAP_U(.015),    /* c300 */
	CAP_U(.1),      /* c301 */
	CAP_U(.082),    /* c302 */
	0,              /* no c4 */
	5,          /* v1 */
	5,          /* v2 */
	5,          /* v3 */
	12,         /* vP */
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
	RES_K(820),     /* r200 */
	RES_K(33),      /* r203 */
	RES_K(150),     /* r201 */
	RES_K(240),     /* r204 */
	RES_M(1),       /* r202 */
	0,
	0,
	0,
	CAP_U(0.01),    /* c200 */
	12
};


static const discrete_op_amp_tvca_info clowns_sb_hit_tvca_info =
{
	RES_M(2.7),     /* r207 */
	RES_K(680),     /* r205 */
	0,
	RES_K(680),     /* r208 */
	RES_K(1),       /* j3 */
	0,
	RES_K(680),     /* r206 */
	0,0,0,0,
	CAP_U(1),       /* c201 */
	0,
	0, 0,
	5,          /* v1 */
	0,          /* v2 */
	0,          /* v3 */
	12,         /* vP */
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
		CLOWNS_R507_POT },
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
	DISCRETE_ADJUSTMENT(CLOWNS_R507_POT, RES_M(1), 7000, DISC_LOGADJ, "R507")

	/************************************************
	 * Tone generator
	 ************************************************/
	MIDWAY_TONE_GENERATOR(midway_music_tvca_info)

	/************************************************
	 * Balloon hit sounds
	 * The LFSR is the same as boothill
	 ************************************************/
	/* Noise clock was breadboarded and measured at 7700Hz */
	DISCRETE_LFSR_NOISE(CLOWNS_NOISE, 1, 1, 7700, 12.0, 0, 12.0/2, &midway_lfsr)

	DISCRETE_OP_AMP_TRIG_VCA(NODE_30, CLOWNS_POP_TOP_EN, CLOWNS_POP_MIDDLE_EN, CLOWNS_POP_BOTTOM_EN, CLOWNS_NOISE, 0, &clowns_pop_tvca_info)
	DISCRETE_RCFILTER(NODE_31, NODE_30, RES_K(15), CAP_U(.01))
	DISCRETE_CRFILTER(NODE_32, NODE_31, RES_K(15) + RES_K(39), CAP_U(.01))
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
	"miss",
	0
};

MACHINE_CONFIG_FRAGMENT( clowns_audio )
	MCFG_SOUND_START_OVERRIDE(mw8080bw_state, samples)

	MCFG_SPEAKER_STANDARD_MONO("mono")

	MCFG_SOUND_ADD("samples", SAMPLES, 0)
	MCFG_SAMPLES_CHANNELS(1)
	MCFG_SAMPLES_NAMES(clowns_sample_names)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.70)

	MCFG_SOUND_ADD("discrete", DISCRETE, 0)
	MCFG_DISCRETE_INTF(clowns)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.25)
MACHINE_CONFIG_END


WRITE8_MEMBER(mw8080bw_state::clowns_audio_1_w)
{
	coin_counter_w(machine(), 0, (data >> 0) & 0x01);

	m_clowns_controller_select = (data >> 1) & 0x01;

	/* D2-D7 are not connected */
}


WRITE8_MEMBER(mw8080bw_state::clowns_audio_2_w)
{
	UINT8 rising_bits = data & ~m_port_2_last;

	m_discrete->write(space, CLOWNS_POP_BOTTOM_EN, (data >> 0) & 0x01);

	m_discrete->write(space, CLOWNS_POP_MIDDLE_EN, (data >> 1) & 0x01);

	m_discrete->write(space, CLOWNS_POP_TOP_EN, (data >> 2) & 0x01);

	machine().sound().system_enable((data >> 3) & 0x01);

	m_discrete->write(space, CLOWNS_SPRINGBOARD_HIT_EN, (data >> 4) & 0x01);

	if (rising_bits & 0x20) m_samples->start(0, 0);  /* springboard miss */

	/* D6 and D7 are not connected */

	m_port_2_last = data;
}



/*************************************
 *
 *  Space Walk
 *
 *  Discrete sound emulation: Oct 2009, D.R.
 *
 *************************************/

	/* Discrete Sound Input Nodes */
#define SPACWALK_TARGET_HIT_BOTTOM_EN         NODE_01
#define SPACWALK_TARGET_HIT_MIDDLE_EN         NODE_02
#define SPACWALK_TARGET_HIT_TOP_EN            NODE_03
#define SPACWALK_SPRINGBOARD_HIT1_EN          NODE_04
#define SPACWALK_SPRINGBOARD_HIT2_EN          NODE_05
#define SPACWALK_SPRINGBOARD_MISS_EN          NODE_06
#define SPACWALK_SPACE_SHIP_EN                NODE_07

/* Discrete Sound Output Nodes */
#define SPACWALK_NOISE                        NODE_10
#define SPACWALK_TARGET_HIT_SND               NODE_11
#define SPACWALK_SPRINGBOARD_HIT1_SND         NODE_12
#define SPACWALK_SPRINGBOARD_HIT2_SND         NODE_13
#define SPACWALK_SPRINGBOARD_MISS_SND         NODE_14
#define SPACWALK_SPACE_SHIP_SND               NODE_15

/* Adjusters */
#define SPACWALK_R507_POT                     NODE_19

/* Parts List - Resistors */
#define SPACWALK_R200       RES_K(820)
#define SPACWALK_R201       RES_K(150)
#define SPACWALK_R202       RES_M(1)
#define SPACWALK_R203       RES_K(82)
#define SPACWALK_R204       RES_K(240)
#define SPACWALK_R205       RES_K(220)
#define SPACWALK_R206       RES_K(120)
#define SPACWALK_R207       RES_M(1)
#define SPACWALK_R208       RES_K(300)
#define SPACWALK_R210       RES_K(56)
#define SPACWALK_R211       RES_K(100)
#define SPACWALK_R213       RES_K(300)
#define SPACWALK_R214       RES_K(27)
#define SPACWALK_R215       RES_K(51)
#define SPACWALK_R216       RES_K(30)
#define SPACWALK_R300       RES_K(270)
#define SPACWALK_R301       RES_K(300)
#define SPACWALK_R302       RES_K(330)
#define SPACWALK_R303       RES_K(680)
#define SPACWALK_R304       RES_M(1)
#define SPACWALK_R305       RES_K(3680)
#define SPACWALK_R307       RES_K(20)
#define SPACWALK_R308       RES_K(20)   /* not labeled but it's beside R307 */
#define SPACWALK_R400       RES_K(1)
#define SPACWALK_R401       RES_K(200)
#define SPACWALK_R403       RES_K(51)
#define SPACWALK_R404       RES_K(220)
#define SPACWALK_R406       RES_M(1)
#define SPACWALK_R407       RES_K(820)
#define SPACWALK_R410       RES_K(47)
#define SPACWALK_R411       RES_K(300)
#define SPACWALK_R412       RES_K(330)
#define SPACWALK_R413       RES_M(1)
#define SPACWALK_R414       RES_M(1)
#define SPACWALK_R416       RES_M(4.7)
#define SPACWALK_R417       RES_K(10)
#define SPACWALK_R418       RES_K(100)
#define SPACWALK_R419       RES_K(2.7)
#define SPACWALK_R420       RES_K(20)
#define SPACWALK_R421       RES_K(11)
#define SPACWALK_R422       RES_K(75)
#define SPACWALK_R507       RES_M(1)
#define SPACWALK_RJ3        RES_K(1)

/* Parts List - Capacitors */
#define SPACWALK_C200       CAP_U(0.0022)
#define SPACWALK_C201       CAP_U(3.3)
#define SPACWALK_C203       CAP_U(0.0033)
#define SPACWALK_C204       CAP_U(0.0033)
#define SPACWALK_C300       CAP_U(2.2)
#define SPACWALK_C301       CAP_U(2.2)
#define SPACWALK_C302       CAP_U(2.2)
#define SPACWALK_C303       CAP_U(0.0047)
#define SPACWALK_C304       CAP_U(0.0047)   /* not labeled but it's beside C303 */
#define SPACWALK_C401       CAP_U(1)
#define SPACWALK_C402       CAP_U(0.68)
#define SPACWALK_C403       CAP_U(0.0022)
#define SPACWALK_C451       CAP_U(0.001)
#define SPACWALK_C452       CAP_U(0.001)
#define SPACWALK_C453       CAP_U(0.001)
#define SPACWALK_C602       CAP_U(1)


static const discrete_op_amp_tvca_info spacwalk_hit_tvca_info =
{
	SPACWALK_R304,  SPACWALK_R303, 0, SPACWALK_R305,    /* r1, r2, r3, r4 */
	SPACWALK_RJ3, 0, SPACWALK_R300,                     /* r5, r6, r7 */
	SPACWALK_RJ3, SPACWALK_R301,                        /* r8, r9 */
	SPACWALK_RJ3, SPACWALK_R302,                        /* r10, r11 */
	SPACWALK_C300, SPACWALK_C301, SPACWALK_C302, 0,     /* c1, c2, c3, c4 */
	5, 5, 5, 12,                                        /* v1, v2, v3, vP */
	DISC_OP_AMP_TRIGGER_FUNCTION_NONE,
	DISC_OP_AMP_TRIGGER_FUNCTION_NONE,
	DISC_OP_AMP_TRIGGER_FUNCTION_TRG0,
	DISC_OP_AMP_TRIGGER_FUNCTION_NONE,
	DISC_OP_AMP_TRIGGER_FUNCTION_TRG1,
	DISC_OP_AMP_TRIGGER_FUNCTION_TRG2
};

static const discrete_op_amp_osc_info spacwalk_sb_hit_vco =
{
	DISC_OP_AMP_OSCILLATOR_VCO_3 | DISC_OP_AMP_IS_NORTON,
	RES_K(330 + 150 + 30), SPACWALK_R203, SPACWALK_R201, SPACWALK_R204, /* r1, r2, r3, r4 */
	SPACWALK_R202, 0, SPACWALK_R200, 0,                                 /* r5, r6, r7, r8*/
	SPACWALK_C200, 12                                                    /* c, vP*/
};

static const discrete_op_amp_tvca_info spacwalk_sb_hit_tvca_info =
{
	SPACWALK_R207,  SPACWALK_R205, 0, SPACWALK_R208,    /* r1, r2, r3, r4 */
	SPACWALK_RJ3, 0, SPACWALK_R206,                     /* r5, r6, r7 */
	0, 0, 0, 0                  ,                       /* r8, r9, r10, r11 */
	SPACWALK_C201, 0, 0, 0,                             /* c1, c2, c3, c4 */
	5, 0, 0, 12,                                        /* v1, v2, v3, vP */
	DISC_OP_AMP_TRIGGER_FUNCTION_NONE,
	DISC_OP_AMP_TRIGGER_FUNCTION_NONE,
	DISC_OP_AMP_TRIGGER_FUNCTION_TRG0,
	DISC_OP_AMP_TRIGGER_FUNCTION_NONE,
	DISC_OP_AMP_TRIGGER_FUNCTION_NONE,
	DISC_OP_AMP_TRIGGER_FUNCTION_NONE
};

static const discrete_integrate_info spacwalk_sb_miss_integrate =
{
	DISC_INTEGRATE_OP_AMP_1 | DISC_OP_AMP_IS_NORTON,
	SPACWALK_R406, SPACWALK_R401, 0, SPACWALK_C402, /* r1, r2, r3, c */
	12, 12, /* v1, vP */
	0, 0, 0 /* f0, f1, f2 */
};

static const discrete_op_amp_osc_info spacwalk_sb_miss_vco =
{
	DISC_OP_AMP_OSCILLATOR_VCO_3 | DISC_OP_AMP_IS_NORTON | DISC_OP_AMP_OSCILLATOR_OUT_SQW,
	SPACWALK_R407, SPACWALK_R412, SPACWALK_R410, SPACWALK_R411, SPACWALK_R413, 0, 0, 0, /* r1, r2, r3, r4, r5, r6, r7, r8 */
	SPACWALK_C403, 12                                               /* c, vP */
};

static const discrete_op_amp_filt_info spacwalk_sb_miss_filter =
{
	/* we use r1, not r2 because vref is taken into acount by the CRFILTER */
	SPACWALK_R417, 0, SPACWALK_R414, 0, SPACWALK_R416,  /* r1, r2, r3, r4, rF */
	SPACWALK_C451, SPACWALK_C452, 0,                    /* c1, c2, c3 */
	0, 12, 0                                            /* vRef, vP, vN */
};

static const discrete_op_amp_info spacwalk_sb_miss_amp =
{
	DISC_OP_AMP_IS_NORTON,
	SPACWALK_R418, SPACWALK_R404, 0, SPACWALK_R403, /* r1, r2, r3, r4 */
	0,  /* c */
	0, 12,  /* vN, vP */
};

static const discrete_op_amp_osc_info spacwalk_spaceship_osc =
{
	DISC_OP_AMP_OSCILLATOR_2 | DISC_OP_AMP_IS_NORTON | DISC_OP_AMP_OSCILLATOR_OUT_CAP,
	RES_K(75), RES_M(1), RES_M(6.8), RES_M(2.4), 0, 0, 0, 0,    /* r1, r2, r3, r4, r5, r6, r7, r8 */
	CAP_U(2.2), 12                                              /* c, vP */
};

static const discrete_op_amp_osc_info spacwalk_spaceship_vco =
{
	DISC_OP_AMP_OSCILLATOR_VCO_1 | DISC_OP_AMP_IS_NORTON | DISC_OP_AMP_OSCILLATOR_OUT_CAP,
	RES_K(680), RES_K(300), RES_K(100), RES_K(150), RES_K(120), 0, 0, 0,    /* r1, r2, r3, r4, r5, r6, r7, r8 */
	CAP_U(0.0012), 12                                                       /* c, vP */
};

static const discrete_mixer_desc spacwalk_mixer =
{
	DISC_MIXER_IS_RESISTOR,
	{SPACWALK_R422, SPACWALK_R422, RES_K(39 + 10 + 1), SPACWALK_R421, SPACWALK_R420, SPACWALK_R419},
	{0, 0, 0, 0, 0, SPACWALK_R507_POT},     /* r_nodes */
	{0}, 0, 0, 0, SPACWALK_C602, 0, 1       /* c, rI, rF, cF, cAmp, vRef, gain */
};


/************************************************
 * Springboard Hit Circuit   1 or 2
 ************************************************/
#define SPACWALK_SPRINGBOARD_HIT_CIRCUIT(_num)                                              \
DISCRETE_RCFILTER(NODE_RELATIVE(NODE_29, _num),                                             \
	SPACWALK_NOISE,                                 /* IN0 */                               \
	RES_K(330), CAP_U(.1))                                                                  \
DISCRETE_RCFILTER(NODE_RELATIVE(NODE_31, _num),                                             \
	NODE_RELATIVE(NODE_29, _num),                   /* IN0 */                               \
	RES_K(330) + RES_K(150), CAP_U(.1))                                                     \
DISCRETE_OP_AMP_VCO1(NODE_RELATIVE(NODE_33, _num),  /* IC M2-3, pin 5 */                    \
	1,                                              /* ENAB */                              \
	NODE_RELATIVE(NODE_31, _num),                   /* VMOD1 */                             \
	&spacwalk_sb_hit_vco)                                                                   \
DISCRETE_OP_AMP_TRIG_VCA(NODE_RELATIVE(NODE_35, _num),              /* IC M2-3, pin 9 */    \
	NODE_RELATIVE(SPACWALK_SPRINGBOARD_HIT1_EN, _num - 1), 0, 0,    /* TRG0, TRG1, TRG2 */  \
	NODE_RELATIVE(NODE_33, _num), 0,                                /* IN0, IN1 */          \
	&spacwalk_sb_hit_tvca_info)                                                             \
/* Wrong values.  Untested */                                                               \
/* The rest of the circuit is a filter. */                                                  \
DISCRETE_FILTER2(NODE_RELATIVE(NODE_37, _num),                                              \
	1,                                              /* ENAB */                              \
	NODE_RELATIVE(NODE_35, _num),                   /* INP0 */                              \
	2000.0 - _num * 500, 1.0/.8,                    /* FREQ, DAMP */                        \
	DISC_FILTER_LOWPASS)                                                                    \
/* The filter has a gain of 0.5 */                                                          \
DISCRETE_GAIN(NODE_RELATIVE(SPACWALK_SPRINGBOARD_HIT1_SND, _num - 1),                       \
	NODE_RELATIVE(NODE_37, _num), 0.5)

	static DISCRETE_SOUND_START(spacwalk)

	/************************************************
	 * Input register mapping
	 ************************************************/
	DISCRETE_INPUT_LOGIC(SPACWALK_TARGET_HIT_BOTTOM_EN)
	DISCRETE_INPUT_LOGIC(SPACWALK_TARGET_HIT_MIDDLE_EN)
	DISCRETE_INPUT_LOGIC(SPACWALK_TARGET_HIT_TOP_EN)
	DISCRETE_INPUT_LOGIC(SPACWALK_SPRINGBOARD_HIT1_EN)
	DISCRETE_INPUT_LOGIC(SPACWALK_SPRINGBOARD_HIT2_EN)
	DISCRETE_INPUT_LOGIC(SPACWALK_SPRINGBOARD_MISS_EN)
	DISCRETE_INPUT_LOGIC(SPACWALK_SPACE_SHIP_EN)

	/* The low value of the pot is set to 7000.  A real 1M pot will never go to 0 anyways. */
	/* This will give the control more apparent volume range. */
	/* The music way overpowers the rest of the sounds anyways. */
	DISCRETE_ADJUSTMENT(SPACWALK_R507_POT, SPACWALK_R507, 7000, DISC_LOGADJ, "R507")

	/************************************************
	 * Tone generator
	 ************************************************/
	MIDWAY_TONE_GENERATOR(midway_music_tvca_info)

	/************************************************
	 * Target hit sounds
	 * The LFSR is the same as boothill
	 ************************************************/
	/* Noise clock was breadboarded and measured at 7700Hz */
	DISCRETE_LFSR_NOISE(SPACWALK_NOISE,         /* IC L4, pin 10 */
		1, 1,                                   /* ENAB, RESET */
		7700, 12.0, 0, 12.0/2, &midway_lfsr)    /* CLK,AMPL,FEED,BIAS,LFSRTB */

	DISCRETE_OP_AMP_TRIG_VCA(NODE_20,           /* IC K3, pin 9 */
		SPACWALK_TARGET_HIT_TOP_EN, SPACWALK_TARGET_HIT_MIDDLE_EN, SPACWALK_TARGET_HIT_BOTTOM_EN,
		SPACWALK_NOISE, 0,                      /* IN0, IN1 */
		&spacwalk_hit_tvca_info)
	DISCRETE_RCFILTER(NODE_21,
		NODE_20,                                /* IN0 */
		SPACWALK_R307, SPACWALK_C303)
	DISCRETE_RCFILTER(SPACWALK_TARGET_HIT_SND,
		NODE_21,                                /* IN0 */
		SPACWALK_R307 + SPACWALK_R308, SPACWALK_C304)

	/************************************************
	 * Springboard hit sounds
	 ************************************************/
	/* Nodes 30 - 40 */
	SPACWALK_SPRINGBOARD_HIT_CIRCUIT(1)
	SPACWALK_SPRINGBOARD_HIT_CIRCUIT(2)

	/************************************************
	 * Springboard miss sound
	 ************************************************/
	DISCRETE_RCDISC2(NODE_50,                   /* voltage on C401 */
		SPACWALK_SPRINGBOARD_MISS_EN,           /* SWITCH */
		OP_AMP_NORTON_VBE, RES_2_PARALLEL(SPACWALK_R401, SPACWALK_R407),    /* INP0,RVAL0 */
		12.0 - .5, SPACWALK_R400,               /* INP1,RVAL1 */
		SPACWALK_C401)
	DISCRETE_INTEGRATE(NODE_51,                 /* IC K4, pin 9 */
		NODE_50, 0,                             /* TRG0,TRG1*/
		&spacwalk_sb_miss_integrate)
	DISCRETE_OP_AMP_VCO1(NODE_52,               /* IC K4, pin 5 */
		1,                                      /* ENAB */
		NODE_50,                                /* VMOD1 */
		&spacwalk_sb_miss_vco)
	DISCRETE_CRFILTER(NODE_53,
		NODE_52,                                /* IN0 */
		SPACWALK_R417, SPACWALK_C453)
	/* this filter type probably does not work right. I need to test it. */
	DISCRETE_OP_AMP_FILTER(NODE_54,             /* IC K3, pin 5 */
		1,                                      /* ENAB */
		NODE_53, 0,                             /* INP0,INP1 */
		DISC_OP_AMP_FILTER_IS_BAND_PASS_1M | DISC_OP_AMP_IS_NORTON, &spacwalk_sb_miss_filter)
	DISCRETE_OP_AMP(SPACWALK_SPRINGBOARD_MISS_SND,  /* IC K4, pin 10 */
		1,                                      /* ENAB */
		NODE_54, NODE_51,                       /* IN0,IN1 */
		&spacwalk_sb_miss_amp)

	/************************************************
	 * Space ship sound
	 ************************************************/
	DISCRETE_OP_AMP_OSCILLATOR(NODE_60,         /* voltage on 2.2uF cap near IC JK-2 */
		1,                                      /* ENAB */
		&spacwalk_spaceship_osc)
	DISCRETE_OP_AMP_VCO1(NODE_61,               /* IC JK-2, pin 5 */
		SPACWALK_SPACE_SHIP_EN,                 /* ENAB */
		NODE_60,                                /* VMOD1*/
		&spacwalk_spaceship_vco)
	DISCRETE_RCFILTER(NODE_62,
		NODE_61,                                /* IN0 */
		RES_K(1), CAP_U(0.15))
	DISCRETE_RCFILTER(SPACWALK_SPACE_SHIP_SND,
		NODE_62,                                /* IN0 */
		RES_K(1) + RES_K(10), CAP_U(0.015))

	/************************************************
	 * Combine all sound sources.
	 ************************************************/
	DISCRETE_MIXER6(NODE_90,
		1,                                      /* ENAB */
		SPACWALK_SPRINGBOARD_HIT1_SND,
		SPACWALK_SPRINGBOARD_HIT2_SND,
		SPACWALK_SPACE_SHIP_SND,
		SPACWALK_SPRINGBOARD_MISS_SND,
		SPACWALK_TARGET_HIT_SND,
		MIDWAY_TONE_SND,
		&spacwalk_mixer)
	DISCRETE_OUTPUT(NODE_90, 11000)
DISCRETE_SOUND_END

MACHINE_CONFIG_FRAGMENT( spacwalk_audio )
	MCFG_SPEAKER_STANDARD_MONO("mono")

	MCFG_SOUND_ADD("discrete", DISCRETE, 0)
	MCFG_DISCRETE_INTF(spacwalk)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 1.0)
MACHINE_CONFIG_END

WRITE8_MEMBER(mw8080bw_state::spacwalk_audio_1_w)
{
	coin_counter_w(machine(), 0, (data >> 0) & 0x01);

	m_clowns_controller_select = (data >> 1) & 0x01;

	machine().sound().system_enable((data >> 2) & 0x01);

	m_discrete->write(space, SPACWALK_SPACE_SHIP_EN, (data >> 3) & 0x01);
}

WRITE8_MEMBER(mw8080bw_state::spacwalk_audio_2_w)
{
	m_discrete->write(space, SPACWALK_TARGET_HIT_BOTTOM_EN, (data >> 0) & 0x01);

	m_discrete->write(space, SPACWALK_TARGET_HIT_MIDDLE_EN, (data >> 1) & 0x01);

	m_discrete->write(space, SPACWALK_TARGET_HIT_TOP_EN, (data >> 2) & 0x01);

	m_discrete->write(space, SPACWALK_SPRINGBOARD_HIT1_EN, (data >> 3) & 0x01);

	m_discrete->write(space, SPACWALK_SPRINGBOARD_HIT2_EN, (data >> 4) & 0x01);

	m_discrete->write(space, SPACWALK_SPRINGBOARD_MISS_EN, (data >> 5) & 0x01);
}



/*************************************
 *
 *  Shuffleboard
 *
 *  Discrete sound emulation: Oct 2009, D.R.
 *
 *************************************/

	/* Discrete Sound Input Nodes */
#define SHUFFLE_ROLLING_1_EN        NODE_01
#define SHUFFLE_ROLLING_2_EN        NODE_02
#define SHUFFLE_ROLLING_3_EN        NODE_03
#define SHUFFLE_FOUL_EN             NODE_04
#define SHUFFLE_ROLLOVER_EN         NODE_05
#define SHUFFLE_CLICK_EN            NODE_06

/* Discrete Sound Output Nodes */
#define SHUFFLE_NOISE               NODE_10
#define SHUFFLE_ROLLING_SND         NODE_11
#define SHUFFLE_FOUL_SND            NODE_12
#define SHUFFLE_ROLLOVER_SND        NODE_13
#define SHUFFLE_CLICK_SND           NODE_14

/* Parts List - Resistors */
#define SHUFFLE_R300    RES_K(33)
#define SHUFFLE_R400    RES_K(200)
#define SHUFFLE_R401    RES_K(3)
#define SHUFFLE_R402    RES_K(5.6)
#define SHUFFLE_R403    RES_K(5.6)
#define SHUFFLE_R404    RES_M(1)
#define SHUFFLE_R406    RES_K(300)
#define SHUFFLE_R407    RES_K(680)
#define SHUFFLE_R408    RES_K(680)
#define SHUFFLE_R409    RES_K(680)
#define SHUFFLE_R410    RES_K(680)
#define SHUFFLE_R411    RES_K(680)
#define SHUFFLE_R412    RES_M(2.7)
#define SHUFFLE_R500    RES_K(300)
#define SHUFFLE_R503    RES_M(2.7)
#define SHUFFLE_R504    RES_K(680)
#define SHUFFLE_R505    RES_K(680)
#define SHUFFLE_R506    RES_K(100)
#define SHUFFLE_R507    RES_K(47)
#define SHUFFLE_R508    RES_K(47)
#define SHUFFLE_R509    RES_K(100)
#define SHUFFLE_R511    RES_M(2)
#define SHUFFLE_R512    RES_M(5.6)
#define SHUFFLE_R513    RES_K(680)
#define SHUFFLE_R514    RES_M(1.5)
#define SHUFFLE_R515    RES_M(1)
#define SHUFFLE_R516    RES_K(510)

/* Parts List - Capacitors */
#define SHUFFLE_C300    CAP_U(0.1)
#define SHUFFLE_C400    CAP_U(0.1)
#define SHUFFLE_C401    CAP_U(1)
#define SHUFFLE_C402    CAP_U(1)
#define SHUFFLE_C403    CAP_U(1)
#define SHUFFLE_C404    CAP_U(0.1)
#define SHUFFLE_C405    CAP_U(0.1)
#define SHUFFLE_C500    CAP_U(0.1)
#define SHUFFLE_C503    CAP_U(0.0022)
#define SHUFFLE_C504    CAP_U(0.0022)
#define SHUFFLE_C505    CAP_U(0.33)
#define SHUFFLE_C506    CAP_U(1)
#define SHUFFLE_C507    CAP_U(1)
#define SHUFFLE_C508    CAP_U(1)


static const discrete_op_amp_tvca_info shuffle_rolling_tvca =
{
	SHUFFLE_R512, 0, 0, SHUFFLE_R511,                   /* r1, r2, r3, r4 */
	RES_K(10), 0, SHUFFLE_R516,                         /* r5, r6, r7 */
	RES_K(10), SHUFFLE_R515,                            /* r8, r9 */
	RES_K(10), SHUFFLE_R514,                            /* r10, r11 */
	SHUFFLE_C508, SHUFFLE_C507, SHUFFLE_C506, SHUFFLE_C505,     /* c1, c2, c3, c4 */
	12, 12, 12, 12,                                     /* v1, v2, v3, vP */
	0, 0, DISC_OP_AMP_TRIGGER_FUNCTION_TRG0, 0,         /* f0, f1, f2, f3 */
	DISC_OP_AMP_TRIGGER_FUNCTION_TRG1,                  /* f4 */
	DISC_OP_AMP_TRIGGER_FUNCTION_TRG2                   /* f5 */
};

static const discrete_op_amp_info shuffle_rolling_amp =
{
	DISC_OP_AMP_IS_NORTON,
	SHUFFLE_R513, SHUFFLE_R505, SHUFFLE_R503, SHUFFLE_R504, /* r1, r2, r3, r4 */
	0,                      /* c */
	0, 12,                  /* vN, vP */
};

static const discrete_op_amp_tvca_info shuffle_foul_tvca =
{
	SHUFFLE_R412, SHUFFLE_R411, 0, SHUFFLE_R408,        /* r1, r2, r3, r4 */
	RES_K(1), 0, SHUFFLE_R406,                          /* r5, r6, r7 */
	0, 0, 0, 0,                                         /* r8, r9, r10, r11 */
	SHUFFLE_C404, 0, 0, 0,                              /* c1, c2, c3, c4 */
	5, 0, 0, 12,                                        /* v1, v2, v3, vP */
	0, 0, DISC_OP_AMP_TRIGGER_FUNCTION_TRG0, 0, 0, 0    /* f0, f1, f2, f3, f4, f5 */
};

static const discrete_op_amp_tvca_info shuffle_rollover_tvca =
{
	SHUFFLE_R404, SHUFFLE_R410, 0, SHUFFLE_R407,        /* r1, r2, r3, r4 */
	RES_K(10), 0, SHUFFLE_R409,                         /* r5, r6, r7 */
	0, 0, 0, 0,                                         /* r8, r9, r10, r11 */
	SHUFFLE_C405, 0, 0, 0,                              /* c1, c2, c3, c4 */
	12, 0, 0, 12,                                       /* v1, v2, v3, vP */
	0, 0, DISC_OP_AMP_TRIGGER_FUNCTION_TRG0, 0, 0, 0    /* f0, f1, f2, f3, f4, f5 */
};

static const discrete_mixer_desc shuffle_mixer =
{
	DISC_MIXER_IS_RESISTOR,
	{SHUFFLE_R500, SHUFFLE_R400, SHUFFLE_R403 + SHUFFLE_R402 + SHUFFLE_R401, SHUFFLE_R300},
	{0},        /* r_nodes */
	{SHUFFLE_C500, SHUFFLE_C400, SHUFFLE_C401, SHUFFLE_C300},
	0, 0, 0, 0, 0 ,1        /* rI, rF, cF, cAmp, vRef, gain */
};


static DISCRETE_SOUND_START(shuffle)
	DISCRETE_INPUT_LOGIC(SHUFFLE_ROLLING_1_EN)
	DISCRETE_INPUT_LOGIC(SHUFFLE_ROLLING_2_EN)
	DISCRETE_INPUT_LOGIC(SHUFFLE_ROLLING_3_EN)
	DISCRETE_INPUT_LOGIC(SHUFFLE_FOUL_EN)
	DISCRETE_INPUT_LOGIC(SHUFFLE_ROLLOVER_EN)
	DISCRETE_INPUTX_LOGIC(SHUFFLE_CLICK_EN, 11.5, 0, 0)

	/* Noise clock was breadboarded and measured at 1210Hz */
	DISCRETE_LFSR_NOISE(SHUFFLE_NOISE,          /* IC N5, pin 10 */
		1, 1,                                   /* ENAB, RESET */
		1210, 12.0, 0, 12.0 / 2, &midway_lfsr)  /* CLK,AMPL,FEED,BIAS,LFSRTB */

	/************************************************
	 * Shuffle rolling
	 ************************************************/
	DISCRETE_OP_AMP_TRIG_VCA(NODE_20,           /* IC P3-4, pin 5 */
		SHUFFLE_ROLLING_1_EN, SHUFFLE_ROLLING_2_EN, SHUFFLE_ROLLING_3_EN,   /* TRG0,TRG1,TRG2 */
		0, 0,                                   /*IN0,IN1 */
		&shuffle_rolling_tvca)
	DISCRETE_OP_AMP(NODE_21,                    /* IC P3-4, pin 4 */
		1,                                      /* ENAB */
		SHUFFLE_NOISE, NODE_20,                 /* IN0,IN1 */
		&shuffle_rolling_amp)
	/* filter not accurate */
	DISCRETE_FILTER1(NODE_22, 1, NODE_21, 800, DISC_FILTER_LOWPASS)
	DISCRETE_GAIN(SHUFFLE_ROLLING_SND, NODE_22, .2)

	/************************************************
	 * Foul
	 ************************************************/
	DISCRETE_SQUAREWFIX(NODE_30,                /* Connected to edge connector V - 120Hz */
		1, 120, DEFAULT_TTL_V_LOGIC_1, 50, DEFAULT_TTL_V_LOGIC_1 / 2, 0)    /* ENAB,FREQ,AMP,DUTY,BIAS,PHASE */
	DISCRETE_OP_AMP_TRIG_VCA(SHUFFLE_FOUL_SND,  /* IC M3-4, pin 5 */
		SHUFFLE_FOUL_EN, 0, 0,                  /* TRG0,TRG1,TRG2 */
		NODE_30, 0,                             /*IN0,IN1 */
		&shuffle_foul_tvca)

	/************************************************
	 * Shuffle rollover
	 ************************************************/
	DISCRETE_OP_AMP_TRIG_VCA(NODE_40,           /* IC M3-4, pin 4 */
		SHUFFLE_ROLLOVER_EN, 0, 0,              /* TRG0,TRG1,TRG2 */
		SHUFFLE_NOISE, 0,                       /*IN0,IN1 */
		&shuffle_rollover_tvca)
	DISCRETE_RCFILTER(NODE_41,
		NODE_40,                                /* IN0 */
		SHUFFLE_R403, SHUFFLE_C403)
	DISCRETE_RCFILTER(SHUFFLE_ROLLOVER_SND,
		NODE_41,                                /* IN0 */
		SHUFFLE_R403 + SHUFFLE_R402, SHUFFLE_C402)

	/************************************************
	 * Click
	 ************************************************/
	/* filter not accurate */
	DISCRETE_FILTER1(NODE_50, 1, SHUFFLE_CLICK_EN, 300, DISC_FILTER_LOWPASS)
	DISCRETE_GAIN(SHUFFLE_CLICK_SND, NODE_50, .3)

	/************************************************
	 * Combine all sound sources.
	 ************************************************/
	DISCRETE_MIXER4(NODE_90,
		1,                                      /* ENAB */
		SHUFFLE_ROLLING_SND,
		SHUFFLE_FOUL_SND,
		SHUFFLE_ROLLOVER_SND,
		SHUFFLE_CLICK_SND,
		&shuffle_mixer)
	DISCRETE_OUTPUT(NODE_90, 59200)
DISCRETE_SOUND_END


MACHINE_CONFIG_FRAGMENT( shuffle_audio )
	MCFG_SPEAKER_STANDARD_MONO("mono")

	MCFG_SOUND_ADD("discrete", DISCRETE, 0)
	MCFG_DISCRETE_INTF(shuffle)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 1.0)
MACHINE_CONFIG_END


WRITE8_MEMBER(mw8080bw_state::shuffle_audio_1_w)
{
	m_discrete->write(space, SHUFFLE_CLICK_EN, (data >> 0) & 0x01);

	m_discrete->write(space, SHUFFLE_ROLLOVER_EN, (data >> 1) & 0x01);

	machine().sound().system_enable((data >> 2) & 0x01);

	m_discrete->write(space, NODE_29, (data >> 3) & 0x07);

	m_discrete->write(space, SHUFFLE_ROLLING_3_EN, (data >> 3) & 0x01);
	m_discrete->write(space, SHUFFLE_ROLLING_2_EN, (data >> 4) & 0x01);
	m_discrete->write(space, SHUFFLE_ROLLING_1_EN, (data >> 5) & 0x01);

	/* D6 and D7 are not connected */
}


WRITE8_MEMBER(mw8080bw_state::shuffle_audio_2_w)
{
	m_discrete->write(space, SHUFFLE_FOUL_EN, (data >> 0) & 0x01);

	coin_counter_w(machine(), 0, (data >> 1) & 0x01);

	/* D2-D7 are not connected */
}



/*************************************
 *
 *  Dog Patch
 *
 *  Discrete sound emulation:
 *   Sept 2011, D.R.
 *
 *************************************/

/* nodes - inputs */
#define DOGPATCH_GAME_ON_EN         NODE_01
#define DOGPATCH_LEFT_SHOT_EN       NODE_02
#define DOGPATCH_RIGHT_SHOT_EN      NODE_03
#define DOGPATCH_HIT_EN             NODE_04
#define DOGPATCH_PAN_DATA           NODE_05

/* nodes - sounds */
#define DOGPATCH_NOISE              NODE_06
#define DOGPATCH_L_SHOT_SND         NODE_07
#define DOGPATCH_R_SHOT_SND         NODE_08
#define DOGPATCH_HIT_SND            NODE_09
#define DOGPATCH_L_HIT_SND          NODE_10
#define DOGPATCH_R_HIT_SND          NODE_11


static const discrete_op_amp_tvca_info dogpatch_shot_tvca_info =
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
	0, 0,
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


static const discrete_mixer_desc dogpatch_l_mixer =
{
	DISC_MIXER_IS_OP_AMP,
	{ RES_K(12) + RES_K(68) + RES_K(33),
		RES_K(33) },
	{ 0 },
	{ 0 },
	0,
	RES_K(100),
	0,
	CAP_U(0.1),
	0,
	1   /* final gain */
};


static const discrete_mixer_desc dogpatch_r_mixer =
{
	DISC_MIXER_IS_OP_AMP,
	{ RES_K(12) + RES_K(68) + RES_K(33),
		RES_K(33),
		RES_K(510) + RES_K(33) },
	{ 0 },
	{ 0 },
	0,
	RES_K(100),
	0,
	CAP_U(0.1),
	0,
	1   /* final gain */
};


static DISCRETE_SOUND_START(dogpatch)
	/************************************************
	 * Input register mapping
	 ************************************************/
	DISCRETE_INPUT_LOGIC(DOGPATCH_GAME_ON_EN)
	DISCRETE_INPUT_LOGIC(DOGPATCH_LEFT_SHOT_EN)
	DISCRETE_INPUT_LOGIC(DOGPATCH_RIGHT_SHOT_EN)
	DISCRETE_INPUT_LOGIC(DOGPATCH_HIT_EN)

	/************************************************
	 * Tone generator
	 ************************************************/
	MIDWAY_TONE_GENERATOR(midway_music_tvca_info)

	/* Noise clock was breadboarded and measured at 7700Hz */
	DISCRETE_LFSR_NOISE(DOGPATCH_NOISE, 1, 1, 7700, 12.0, 0, 12.0/2, &midway_lfsr)

	/************************************************
	 * Shot sounds
	 ************************************************/
	DISCRETE_OP_AMP_TRIG_VCA(NODE_20, DOGPATCH_LEFT_SHOT_EN, 0, 0, DOGPATCH_NOISE, 0, &dogpatch_shot_tvca_info)
	DISCRETE_RCFILTER(NODE_21, NODE_20, RES_K(12), CAP_U(.01))
	DISCRETE_RCFILTER(DOGPATCH_L_SHOT_SND, NODE_21, RES_K(12) + RES_K(68), CAP_U(.0022))

	DISCRETE_OP_AMP_TRIG_VCA(NODE_30, DOGPATCH_RIGHT_SHOT_EN, 0, 0, DOGPATCH_NOISE, 0, &dogpatch_shot_tvca_info)
	DISCRETE_RCFILTER(NODE_31, NODE_30, RES_K(12), CAP_U(.01))
	DISCRETE_RCFILTER(DOGPATCH_R_SHOT_SND, NODE_31, RES_K(12) + RES_K(68), CAP_U(.0033))

	/************************************************
	 * Target hit sounds
	 ************************************************/
	DISCRETE_CONSTANT(DOGPATCH_L_HIT_SND, 0)
	DISCRETE_CONSTANT(DOGPATCH_R_HIT_SND, 0)

	/************************************************
	 * Combine all sound sources.
	 ************************************************/
	/* There is a 1uF cap on the input to the amp that I was too lazy to simulate.
	 * It is just a DC blocking cap needed by the Norton amp.  Doing the extra
	 * work to simulate it is not going to make a difference to the waveform
	 * or to how it sounds.  Also I use a regular amp in place of the Norton
	 * for the same reasons.  Ease of coding/simulation. */

	DISCRETE_MIXER2(NODE_91, DOGPATCH_GAME_ON_EN, DOGPATCH_L_SHOT_SND, DOGPATCH_L_HIT_SND, &dogpatch_l_mixer)

	/* Music is only added to the right channel per schematics */
	/* This should be verified on the real game */
	DISCRETE_MIXER3(NODE_92, DOGPATCH_GAME_ON_EN, DOGPATCH_R_SHOT_SND, DOGPATCH_R_HIT_SND, MIDWAY_TONE_SND, &dogpatch_r_mixer)

	DISCRETE_OUTPUT(NODE_91, 32760.0 / 5.8)
	DISCRETE_OUTPUT(NODE_92, 32760.0 / 5.8)

DISCRETE_SOUND_END


MACHINE_CONFIG_FRAGMENT( dogpatch_audio )
	MCFG_SPEAKER_STANDARD_STEREO("lspeaker", "rspeaker")
	MCFG_SOUND_ADD("discrete", DISCRETE, 0)
	MCFG_DISCRETE_INTF(dogpatch)
	MCFG_SOUND_ROUTE(0, "lspeaker", 1.0)
	MCFG_SOUND_ROUTE(1, "rspeaker", 1.0)
MACHINE_CONFIG_END


WRITE8_MEMBER(mw8080bw_state::dogpatch_audio_w)
{
	/* D0, D1 and D7 are not used */

	coin_counter_w(machine(), 0, (data >> 2) & 0x01);

	machine().sound().system_enable((data >> 3) & 0x01);
	m_discrete->write(space, DOGPATCH_GAME_ON_EN, (data >> 3) & 0x01);

	m_discrete->write(space, DOGPATCH_LEFT_SHOT_EN, (data >> 4) & 0x01);

	m_discrete->write(space, DOGPATCH_RIGHT_SHOT_EN, (data >> 5) & 0x01);

	m_discrete->write(space, DOGPATCH_HIT_EN, (data >> 6) & 0x01);
}



/*************************************
 *
 *  Space Encounters
 *
 *  Discrete sound emulation:
 *  Apr 2007, D.R.
 *************************************/

/* nodes - inputs */
#define SPCENCTR_ENEMY_SHIP_SHOT_EN       NODE_01
#define SPCENCTR_PLAYER_SHOT_EN           NODE_02
#define SPCENCTR_SCREECH_EN               NODE_03
#define SPCENCTR_CRASH_EN                 NODE_04
#define SPCENCTR_EXPLOSION_EN             NODE_05
#define SPCENCTR_BONUS_EN                 NODE_06
#define SPCENCTR_WIND_DATA                NODE_07

/* nodes - sounds */
#define SPCENCTR_NOISE                    NODE_10
#define SPCENCTR_ENEMY_SHIP_SHOT_SND      NODE_11
#define SPCENCTR_PLAYER_SHOT_SND          NODE_12
#define SPCENCTR_SCREECH_SND              NODE_13
#define SPCENCTR_CRASH_SND                NODE_14
#define SPCENCTR_EXPLOSION_SND            NODE_15
#define SPCENCTR_BONUS_SND                NODE_16
#define SPCENCTR_WIND_SND                 NODE_17


static const discrete_op_amp_info spcenctr_enemy_ship_shot_op_amp_E1 =
{
	DISC_OP_AMP_IS_NORTON,
	0,                      /* no r1 */
	RES_K(510),             /* R100 */
	RES_M(2.2),             /* R101 */
	RES_M(2.2),             /* R102 */
	CAP_U(0.1),             /* C100 */
	0,                      /* vN */
	12                      /* vP */
};


static const discrete_op_amp_osc_info spcenctr_enemy_ship_shot_op_amp_osc =
{
	DISC_OP_AMP_OSCILLATOR_VCO_3 | DISC_OP_AMP_IS_NORTON | DISC_OP_AMP_OSCILLATOR_OUT_SQW,
	RES_K(560),     /* R103 */
	RES_K(7.5),     /* R118 */
	RES_K(22),      /* R104 */
	RES_K(47),      /* R106 */
	RES_K(100),     /* R105 */
	0,              /* no r6 */
	0,              /* no r7 */
	0,              /* no r8 */
	CAP_U(0.0022),  /* C101 */
	12,             /* vP */
};


static const discrete_op_amp_info spcenctr_enemy_ship_shot_op_amp_D1 =
{
	DISC_OP_AMP_IS_NORTON,
	RES_K(100),             /* R107 */
	RES_K(100),             /* R109 */
	RES_M(2.7),             /* R108 */
	RES_K(100),             /* R110 */
	0,                      /* no c */
	0,                      /* vN */
	12                      /* vP */
};


static const discrete_op_amp_filt_info spcenctr_enemy_ship_shot_filt =
{
	RES_K(100),     /* R112 */
	RES_K(10),      /* R113 */
	RES_M(4.3),     /* r3 */
	0,              /* no r4 */
	RES_M(2.2),     /* R114 */
	CAP_U(0.001),   /* c1 */
	CAP_U(0.001),   /* c2 */
	0,              /* no c3 */
	0,              /* vRef */
	12,             /* vP */
	0               /* vN */
};


static const discrete_op_amp_1sht_info spcenctr_player_shot_1sht =
{
	DISC_OP_AMP_1SHT_1 | DISC_OP_AMP_IS_NORTON,
	RES_M(4.7),     /* R500 */
	RES_K(100),     /* R502 */
	RES_M(1),       /* R501 */
	RES_M(1),       /* R503 */
	RES_M(2.2),     /* R504 */
	CAP_U(1),       /* C500 */
	CAP_P(470),     /* C501 */
	0,              /* vN */
	12              /* vP */
};


static const discrete_op_amp_info spcenctr_player_shot_op_amp_E1 =
{
	DISC_OP_AMP_IS_NORTON,
	0,              /* no r1 */
	RES_K(10),      /* R505 */
	RES_M(1.5),     /* R506 */
	0,              /* no r4 */
	CAP_U(0.22),    /* C502 */
	0,              /* vN */
	12              /* vP */
};


static const discrete_op_amp_osc_info spcenctr_player_shot_op_amp_osc =
{
	DISC_OP_AMP_OSCILLATOR_VCO_3 | DISC_OP_AMP_IS_NORTON | DISC_OP_AMP_OSCILLATOR_OUT_SQW,
	1.0 / (1.0 / RES_M(1) + 1.0 / RES_K(330)) + RES_M(1.5),     /* R507||R509 + R508 */
	RES_M(1),       /* R513 */
	RES_K(560),     /* R512 */
	RES_M(2.7),     /* R516 */
	RES_M(1),       /* R515 */
	RES_M(4.7),     /* R510 */
	RES_M(3.3),     /* R511 */
	0,              /* no r8 */
	CAP_P(330),     /* C504 */
	12,             /* vP */
};


static const discrete_op_amp_info spcenctr_player_shot_op_amp_C1 =
{
	DISC_OP_AMP_IS_NORTON,
	RES_K(560),     /* R517 */
	RES_K(470),     /* R514 */
	RES_M(2.7),     /* R518 */
	RES_K(560),     /* R524 */
	0,              /* no c */
	0,              /* vN */
	12              /* vP */
};


static const discrete_op_amp_tvca_info spcenctr_player_shot_tvca =
{
	RES_M(2.7),                         /* R522 */
	RES_K(560),                         /* R521 */
	0,                                  /* no r3 */
	RES_K(560),                         /* R560 */
	RES_K(1),                           /* R42 */
	0,                                  /* no r6 */
	RES_K(560),                         /* R523 */
	0,                                  /* no r8 */
	0,                                  /* no r9 */
	0,                                  /* no r10 */
	0,                                  /* no r11 */
	CAP_U(1),                           /* C506 */
	0,                                  /* no c2 */
	0, 0,                               /* no c3, c4 */
	12,                                 /* v1 */
	0,                                  /* no v2 */
	0,                                  /* no v3 */
	12,                                 /* vP */
	DISC_OP_AMP_TRIGGER_FUNCTION_NONE,  /* no f0 */
	DISC_OP_AMP_TRIGGER_FUNCTION_NONE,  /* no f1 */
	DISC_OP_AMP_TRIGGER_FUNCTION_TRG0,  /* f2 */
	DISC_OP_AMP_TRIGGER_FUNCTION_NONE,  /* no f3 */
	DISC_OP_AMP_TRIGGER_FUNCTION_NONE,  /* no f4 */
	DISC_OP_AMP_TRIGGER_FUNCTION_NONE   /* no f5 */
};


static const discrete_op_amp_tvca_info spcenctr_crash_tvca =
{
	RES_M(2.7),                         /* R302 */
	RES_K(470),                         /* R300 */
	0,                                  /* no r3 */
	RES_K(470),                         /* R303 */
	RES_K(1),                           /* R56 */
	0,                                  /* no r6 */
	RES_K(470),                         /* R301 */
	0,                                  /* no r8 */
	0,                                  /* no r9 */
	0,                                  /* no r10 */
	0,                                  /* no r11 */
	CAP_U(2.2),                         /* C304 */
	0,                                  /* no c2 */
	0, 0,                               /* no c3, c4 */
	5,                                  /* v1 */
	0,                                  /* no v2 */
	0,                                  /* no v3 */
	12,                                 /* vP */
	DISC_OP_AMP_TRIGGER_FUNCTION_NONE,  /* no f0 */
	DISC_OP_AMP_TRIGGER_FUNCTION_NONE,  /* no f1 */
	DISC_OP_AMP_TRIGGER_FUNCTION_TRG0,  /* f2 */
	DISC_OP_AMP_TRIGGER_FUNCTION_NONE,  /* no f3 */
	DISC_OP_AMP_TRIGGER_FUNCTION_NONE,  /* no f4 */
	DISC_OP_AMP_TRIGGER_FUNCTION_NONE   /* no f5 */
};


static const discrete_op_amp_tvca_info spcenctr_explosion_tvca =
{
	RES_M(2.7),                         /* R402 */
	RES_K(680),                         /* R400 */
	0,                                  /* no r3 */
	RES_K(680),                         /* R403 */
	RES_K(1),                           /* R41 */
	0,                                  /* no r6 */
	RES_K(680),                         /* R401 */
	0,                                  /* no r8 */
	0,                                  /* no r9 */
	0,                                  /* no r10 */
	0,                                  /* no r11 */
	CAP_U(2.2),                         /* C400 */
	0,                                  /* no c2 */
	0, 0,                               /* no c3, c4 */
	12,                                 /* v1 */
	0,                                  /* no v2 */
	0,                                  /* no v3 */
	12,                                 /* vP */
	DISC_OP_AMP_TRIGGER_FUNCTION_NONE,  /* no f0 */
	DISC_OP_AMP_TRIGGER_FUNCTION_NONE,  /* no f1 */
	DISC_OP_AMP_TRIGGER_FUNCTION_TRG0,  /* f2 */
	DISC_OP_AMP_TRIGGER_FUNCTION_NONE,  /* no f3 */
	DISC_OP_AMP_TRIGGER_FUNCTION_NONE,  /* no f4 */
	DISC_OP_AMP_TRIGGER_FUNCTION_NONE   /* no f5 */
};


static const discrete_555_desc spcenctr_555_bonus =
{
	DISC_555_OUT_SQW | DISC_555_OUT_DC,
	5,              /* B+ voltage of 555 */
	DEFAULT_555_VALUES
};


static const discrete_mixer_desc spcenctr_mixer =
{
	DISC_MIXER_IS_RESISTOR,     /* type */
	{ RES_K(15),                /* R117 */
		RES_K(15),              /* R526 */
		RES_K(22),              /* R211 */
		RES_K(3.6),             /* R309 */
		RES_K(1.8) +  RES_K(3.6) + RES_K(4.7),  /* R405 + R406 + R407 */
		RES_K(27),              /* R715 */
		RES_K(27)},             /* R51 */
	{0},                        /* no rNode{} */
	{ 0,
		CAP_U(0.001),               /* C505 */
		CAP_U(0.1),             /* C202 */
		CAP_U(1),                   /* C303 */
		0,
		0,
		CAP_U(10)},             /* C16 */
	0,                          /* no rI */
	0,                          /* no rF */
	0,                          /* no cF */
	CAP_U(1),                   /* C900 */
	0,                          /* vRef = ground */
	1                           /* gain */
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
	DISCRETE_LFSR_NOISE(SPCENCTR_NOISE,         /* IC A0, pin 10 */
					1,                          /* ENAB */
					1,                          /* no RESET */
					7515,                       /* CLK in Hz */
					12,                         /* p-p AMPL */
					0,                          /* no FEED input */
					12.0/2,                     /* dc BIAS */
					&midway_lfsr)


	/************************************************
	 * Enemy Ship Shot
	 ************************************************/
	DISCRETE_OP_AMP(NODE_20,                            /* IC E1, pin 10 */
					1,                                  /* ENAB */
					0,                                  /* no IN0 */
					SPCENCTR_ENEMY_SHIP_SHOT_EN,        /* IN1 */
					&spcenctr_enemy_ship_shot_op_amp_E1)
	DISCRETE_OP_AMP_VCO1(NODE_21,                       /* IC D1, pin 5 */
					1,                                  /* ENAB */
					NODE_20,                            /* VMOD1 */
					&spcenctr_enemy_ship_shot_op_amp_osc)
	DISCRETE_OP_AMP(NODE_22,                            /* IC D1, pin 9 */
					1,                                  /* ENAB */
					NODE_21,                            /* IN0 */
					NODE_20,                            /* IN1 */
					&spcenctr_enemy_ship_shot_op_amp_D1)
	DISCRETE_OP_AMP_FILTER(NODE_23,                     /* IC D1, pin 10 */
					1,                                  /* ENAB */
					NODE_22,                            /* INP0 */
					0,                                  /* no INP1 */
					DISC_OP_AMP_FILTER_IS_BAND_PASS_1M | DISC_OP_AMP_IS_NORTON,
					&spcenctr_enemy_ship_shot_filt)
	DISCRETE_CRFILTER(SPCENCTR_ENEMY_SHIP_SHOT_SND,
					NODE_23,                            /* IN0 */
					RES_K(1.8),                         /* R116 */
					CAP_U(0.1) )                        /* C104 */


	/************************************************
	 * Player Shot
	 ************************************************/
	DISCRETE_OP_AMP_ONESHOT(NODE_30,                    /* IC E1, pin 4 */
					SPCENCTR_PLAYER_SHOT_EN,            /* TRIG */
					&spcenctr_player_shot_1sht)         /* breadboarded and scoped at 325mS */
	DISCRETE_OP_AMP(NODE_31,                            /* IC E1, pin 5 */
					1,                                  /* ENAB */
					0,                                  /* no IN0 */
					NODE_30,                            /* IN1 */
					&spcenctr_player_shot_op_amp_E1)
	/* next 2 modules simulate the D502 voltage drop */
	DISCRETE_ADDER2(NODE_32,
					1,                                  /* ENAB */
					NODE_31,                            /* IN0 */
					-0.5)                               /* IN1 */
	DISCRETE_CLAMP(NODE_33,
					NODE_32,                            /* IN0 */
					0,                                  /* MIN */
					12)                                 /* MAX */
	DISCRETE_CRFILTER(NODE_34,
					SPCENCTR_NOISE,                     /* IN0 */
					RES_M(1) + RES_K(330),              /* R507, R509 */
					CAP_U(0.1) )                        /* C503 */
	DISCRETE_GAIN(NODE_35,
					NODE_34,                            /* IN0 */
					RES_K(330)/(RES_M(1) + RES_K(330))) /* GAIN - R507 : R509 */
	DISCRETE_OP_AMP_VCO2(NODE_36,                       /* IC C1, pin 4 */
					1,                                  /* ENAB */
					NODE_35,                            /* VMOD1 */
					NODE_33,                            /* VMOD2 */
					&spcenctr_player_shot_op_amp_osc)
	DISCRETE_OP_AMP(NODE_37,                            /* IC C1, pin 9 */
					1,                                  /* ENAB */
					NODE_36,                            /* IN0 */
					NODE_33,                            /* IN1 */
					&spcenctr_player_shot_op_amp_C1)
	DISCRETE_OP_AMP_TRIG_VCA(SPCENCTR_PLAYER_SHOT_SND,  /* IC C1, pin 10 */
					SPCENCTR_PLAYER_SHOT_EN,            /* TRG0 */
					0,                                  /* no TRG1 */
					0,                                  /* no TRG2 */
					NODE_37,                            /* IN0 */
					0,                                  /* no IN1 */
					&spcenctr_player_shot_tvca)


	/************************************************
	 *Screech - unemulated
	 ************************************************/
	DISCRETE_CONSTANT(SPCENCTR_SCREECH_SND, 0)


	/************************************************
	 * Crash
	 ************************************************/
	DISCRETE_OP_AMP_TRIG_VCA(NODE_60,           /* IC C2, pin 4 */
					SPCENCTR_CRASH_EN,          /* TRG0 */
					0,                          /* no TRG1 */
					0,                          /* no TRG2 */
					SPCENCTR_NOISE,             /* IN0 */
					0,                          /* no IN1 */
					&spcenctr_crash_tvca)
	/* The next 5 modules emulate the filter. */
	/* The DC level was breadboarded and the frequency response was SPICEd */
	DISCRETE_ADDER2(NODE_61,                    /* center on filter DC level */
					1,                          /* ENAB */
					NODE_60,                    /* IN0 */
					-6.8)                       /* IN1 */
	DISCRETE_FILTER2(NODE_62,
					1,                          /* ENAB */
					NODE_61,                    /* INP0 */
					130,                        /* FREQ */
					1.0 / 8,                    /* DAMP */
					DISC_FILTER_BANDPASS)
	DISCRETE_GAIN(NODE_63,
					NODE_62,                    /* IN0 */
					6)                          /* GAIN */
	DISCRETE_ADDER2(NODE_64,                    /* center on filter DC level */
					1,                          /* ENAB */
					NODE_63,                    /* IN0 */
					6.8)                        /* IN1 */
	DISCRETE_CLAMP(SPCENCTR_CRASH_SND,          /* IC C2, pin 5 */
					NODE_64,                    /* IN0 */
					0,                          /* MIN */
					12.0 - OP_AMP_NORTON_VBE)   /* MAX */


	/************************************************
	 * Explosion
	 ************************************************/
	DISCRETE_OP_AMP_TRIG_VCA(NODE_70,           /* IC D2, pin 10 */
					SPCENCTR_EXPLOSION_EN,      /* TRG0 */
					0,                          /* no TRG1 */
					0,                          /* no TRG2 */
					SPCENCTR_NOISE,             /* IN0 */
					0,                          /* no IN1 */
					&spcenctr_explosion_tvca)
	DISCRETE_RCFILTER(NODE_71,
					NODE_70,                    /* IN0 */
					RES_K(1.8),                 /* R405 */
					CAP_U(0.22) )               /* C401 */
	DISCRETE_RCFILTER(SPCENCTR_EXPLOSION_SND,
					NODE_71,                    /* IN0 */
					RES_K(1.8) + RES_K(3.6),    /* R405 + R406 */
					CAP_U(0.22) )               /* C402 */


	/************************************************
	 *Bonus
	 ************************************************/
	DISCRETE_555_ASTABLE(NODE_80,               /* pin 5 */
					/* the pin 4 reset is not connected in schematic, but should be */
					SPCENCTR_BONUS_EN,          /* RESET */
					RES_K(1),                   /* R710 */
					RES_K(27),                  /* R711 */
					CAP_U(0.047),               /* C710 */
					&spcenctr_555_bonus)
	DISCRETE_555_ASTABLE(NODE_81,               /* pin 9 */
					SPCENCTR_BONUS_EN,          /* RESET pin 10 */
					RES_K(100),                 /* R713 */
					RES_K(47),                  /* R714 */
					CAP_U(1),                   /* C713 */
					&spcenctr_555_bonus)
	DISCRETE_LOGIC_AND3(NODE_82,                /* IC C-D, pin 6 */
					NODE_80,                    /* INP0 */
					NODE_81,                    /* INP1 */
					SPCENCTR_BONUS_EN)          /* INP2 */
	DISCRETE_GAIN(SPCENCTR_BONUS_SND,           /* adjust from logic to TTL voltage level */
					NODE_82,                    /* IN0 */
					DEFAULT_TTL_V_LOGIC_1)      /* GAIN */


	/************************************************
	 *Wind - unemulated
	 ************************************************/
	DISCRETE_CONSTANT(SPCENCTR_WIND_SND, 0)


	/************************************************
	 * Final mix
	 ************************************************/
	DISCRETE_MIXER7(NODE_91,
					1,                              /* ENAB */
					SPCENCTR_ENEMY_SHIP_SHOT_SND,   /* IN0 */
					SPCENCTR_PLAYER_SHOT_SND,       /* IN1 */
					SPCENCTR_SCREECH_SND,           /* IN2 */
					SPCENCTR_CRASH_SND,             /* IN3 */
					SPCENCTR_EXPLOSION_SND,         /* IN4 */
					SPCENCTR_BONUS_SND,             /* IN5 */
					SPCENCTR_WIND_SND,              /* IN6 */
					&spcenctr_mixer)

	DISCRETE_OUTPUT(NODE_91, 20000)
DISCRETE_SOUND_END


MACHINE_CONFIG_FRAGMENT( spcenctr_audio )
	MCFG_SPEAKER_STANDARD_MONO("mono")

	MCFG_SOUND_ADD("snsnd", SN76477, 0)
	MCFG_SN76477_NOISE_PARAMS(0, 0, 0)                  // noise + filter: N/C
	MCFG_SN76477_DECAY_RES(0)                           // decay_res: N/C
	MCFG_SN76477_ATTACK_PARAMS(0, RES_K(100))           // attack_decay_cap + attack_res
	MCFG_SN76477_AMP_RES(RES_K(56))                     // amplitude_res
	MCFG_SN76477_FEEDBACK_RES(RES_K(10))                // feedback_res
	MCFG_SN76477_VCO_PARAMS(0, CAP_U(0.047), RES_K(56)) // VCO volt + cap + res
	MCFG_SN76477_PITCH_VOLTAGE(5.0)                     // pitch_voltage
	MCFG_SN76477_SLF_PARAMS(CAP_U(1.0), RES_K(150))     // slf caps + res
	MCFG_SN76477_ONESHOT_PARAMS(0, 0)                   // oneshot caps + res: N/C
	MCFG_SN76477_VCO_MODE(1)                            // VCO mode
	MCFG_SN76477_MIXER_PARAMS(0, 0, 0)                  // mixer A, B, C
	MCFG_SN76477_ENVELOPE_PARAMS(1, 0)                  // envelope 1, 2
	MCFG_SN76477_ENABLE(1)                              // enable
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.20)

	MCFG_SOUND_ADD("discrete", DISCRETE, 0)
	MCFG_DISCRETE_INTF(spcenctr)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.45)
MACHINE_CONFIG_END



WRITE8_MEMBER(mw8080bw_state::spcenctr_audio_1_w)
{
	machine().sound().system_enable((data >> 0) & 0x01);

	/* D1 is marked as 'OPTIONAL SWITCH VIDEO FOR COCKTAIL',
	   but it is never set by the software */

	m_discrete->write(space, SPCENCTR_CRASH_EN, (data >> 2) & 0x01);

	/* D3-D7 are not connected */
}


WRITE8_MEMBER(mw8080bw_state::spcenctr_audio_2_w)
{
	/* set WIND SOUND FREQ(data & 0x0f)  0, if no wind */

	m_discrete->write(space, SPCENCTR_EXPLOSION_EN, (data >> 4) & 0x01);

	m_discrete->write(space, SPCENCTR_PLAYER_SHOT_EN, (data >> 5) & 0x01);

	/* D6 and D7 are not connected */

	m_port_2_last = data;
}


WRITE8_MEMBER(mw8080bw_state::spcenctr_audio_3_w)
{
	/* if (data & 0x01)  enable SCREECH (hit the sides) sound */

	m_discrete->write(space, SPCENCTR_ENEMY_SHIP_SHOT_EN, (data >> 1) & 0x01);

	m_spcenctr_strobe_state = (data >> 2) & 0x01;

	output_set_value("LAMP", (data >> 3) & 0x01);

	m_discrete->write(space, SPCENCTR_BONUS_EN, (data >> 4) & 0x01);

	m_sn->enable_w((data >> 5) & 0x01); /* saucer sound */

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
	"1",    /* shot */
	"2",    /* explosion */
	0
};

MACHINE_CONFIG_FRAGMENT( phantom2_audio )
	MCFG_SOUND_START_OVERRIDE(mw8080bw_state, samples)

	MCFG_SPEAKER_STANDARD_MONO("mono")
	MCFG_SOUND_ADD("samples", SAMPLES, 0)
	MCFG_SAMPLES_CHANNELS(2)
	MCFG_SAMPLES_NAMES(phantom2_sample_names)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 1)
MACHINE_CONFIG_END


WRITE8_MEMBER(mw8080bw_state::phantom2_audio_1_w)
{
	UINT8 rising_bits = data & ~m_port_1_last;

	/* if (data & 0x01)  enable PLAYER SHOT sound */
	if (rising_bits & 0x01) m_samples->start(0, 0);

	/* if (data & 0x02)  enable ENEMY SHOT sound */

	machine().sound().system_mute(!(data & 0x20));
	machine().sound().system_enable((data >> 2) & 0x01);

	coin_counter_w(machine(), 0, (data >> 3) & 0x01);

	/* if (data & 0x10)  enable RADAR sound */

	/* D5-D7 are not connected */

	m_port_1_last = data;
}


WRITE8_MEMBER(mw8080bw_state::phantom2_audio_2_w)
{
	UINT8 rising_bits = data & ~m_port_2_last;

	/* D0-D2 are not connected */

	/* if (data & 0x08)  enable EXPLOSION sound */
	if (rising_bits & 0x08) m_samples->start(1, 1);

	output_set_value("EXPLAMP", (data >> 4) & 0x01);

	/* set JET SOUND FREQ((data >> 5) & 0x07)  0, if no jet sound */

	m_port_2_last = data;
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
#define BOWLER_FOWL_EN            NODE_01

/* nodes - sounds */
#define BOWLER_FOWL_SND           NODE_10


static const discrete_op_amp_tvca_info bowler_fowl_tvca =
{
	RES_M(2.7),                         /* R1103 */
	RES_K(680),                         /* R1102 */
	0,                                  /* no r3 */
	RES_K(680),                         /* R1104 */
	RES_K(1),                           /* SIP */
	0,                                  /* no r6 */
	RES_K(300),                         /* R1101 */
	0,                                  /* no r8 */
	0,                                  /* no r9 */
	0,                                  /* no r10 */
	0,                                  /* no r11 */
	CAP_U(0.1),                         /* C1050 */
	0,                                  /* no c2 */
	0, 0,                               /* no c3, c4 */
	5,                                  /* v1 */
	0,                                  /* no v2 */
	0,                                  /* no v3 */
	12,                                 /* vP */
	DISC_OP_AMP_TRIGGER_FUNCTION_NONE,  /* no f0 */
	DISC_OP_AMP_TRIGGER_FUNCTION_NONE,  /* no f1 */
	DISC_OP_AMP_TRIGGER_FUNCTION_TRG0,  /* f2 */
	DISC_OP_AMP_TRIGGER_FUNCTION_NONE,  /* no f3 */
	DISC_OP_AMP_TRIGGER_FUNCTION_NONE,  /* no f4 */
	DISC_OP_AMP_TRIGGER_FUNCTION_NONE   /* no f5 */
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
					1,                          /* ENAB */
					180,                        /* FREQ */
					DEFAULT_TTL_V_LOGIC_1,      /* p-p AMP */
					50,                         /* DUTY */
					DEFAULT_TTL_V_LOGIC_1 / 2,  /* dc BIAS */
					0)                          /* PHASE */
	DISCRETE_OP_AMP_TRIG_VCA(NODE_21,           /* IC P3, pin 9 */
					BOWLER_FOWL_EN,             /* TRG0 */
					0,                          /* no TRG1 */
					0,                          /* no TRG2 */
					NODE_20,                    /* IN0 */
					0,                          /* no IN1 */
					&bowler_fowl_tvca)
	DISCRETE_CRFILTER(BOWLER_FOWL_SND,
					NODE_21,                    /* IN0 */
					RES_K(68),                  /* R1120 */
					CAP_U(0.1) )                /* C1048 */

	DISCRETE_OUTPUT(BOWLER_FOWL_SND, 10000)
DISCRETE_SOUND_END


MACHINE_CONFIG_FRAGMENT( bowler_audio )
	MCFG_SPEAKER_STANDARD_MONO("mono")

	MCFG_SOUND_ADD("discrete", DISCRETE, 0)
	MCFG_DISCRETE_INTF(bowler)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 1)
MACHINE_CONFIG_END


WRITE8_MEMBER(mw8080bw_state::bowler_audio_1_w)
{
	/* D0 - selects controller on the cocktail PCB */

	coin_counter_w(machine(), 0, (data >> 1) & 0x01);

	machine().sound().system_enable((data >> 2) & 0x01);

	m_discrete->write(space, BOWLER_FOWL_EN, (data >> 3) & 0x01);

	/* D4 - appears to be a screen flip, but it's
	        shown unconnected on the schematics for both the
	        upright and cocktail PCB's */

	/* D5 - triggered on a 'strike', sound circuit not labeled */

	/* D6 and D7 are not connected */
}


WRITE8_MEMBER(mw8080bw_state::bowler_audio_2_w)
{
	/* set BALL ROLLING SOUND FREQ(data & 0x0f)
	   0, if no rolling, 0x08 used during ball return */

	/* D4 -  triggered when the ball crosses the foul line,
	         sound circuit not labeled */

	/* D5 - triggered on a 'spare', sound circuit not labeled */

	/* D6 and D7 are not connected */
}


WRITE8_MEMBER(mw8080bw_state::bowler_audio_3_w)
{
	/* regardless of the data, enable BALL HITS PIN 1 sound
	   (top circuit on the schematics) */
}


WRITE8_MEMBER(mw8080bw_state::bowler_audio_4_w)
{
	/* regardless of the data, enable BALL HITS PIN 2 sound
	   (bottom circuit on the schematics) */
}


WRITE8_MEMBER(mw8080bw_state::bowler_audio_5_w)
{
	/* not sure, appears to me trigerred alongside the two
	   BALL HITS PIN sounds */
}


WRITE8_MEMBER(mw8080bw_state::bowler_audio_6_w)
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

static const char *const invaders_sample_names[] =
{
	"*invaders",
	"1",        /* shot/missle */
	"2",        /* base hit/explosion */
	"3",        /* invader hit */
	"4",        /* fleet move 1 */
	"5",        /* fleet move 2 */
	"6",        /* fleet move 3 */
	"7",        /* fleet move 4 */
	"8",        /* UFO/saucer hit */
	"9",        /* bonus base */
	0
};


/* left in for all games that hack into invaders samples for audio */
MACHINE_CONFIG_FRAGMENT( invaders_samples_audio )
	MCFG_SOUND_START_OVERRIDE(mw8080bw_state, samples)

	MCFG_SPEAKER_STANDARD_MONO("mono")

	MCFG_SOUND_ADD("snsnd", SN76477, 0)
	MCFG_SN76477_NOISE_PARAMS(0, 0, 0)                  // noise + filter: N/C
	MCFG_SN76477_DECAY_RES(0)                           // decay_res: N/C
	MCFG_SN76477_ATTACK_PARAMS(0, RES_K(100))           // attack_decay_cap + attack_res
	MCFG_SN76477_AMP_RES(RES_K(56))                     // amplitude_res
	MCFG_SN76477_FEEDBACK_RES(RES_K(10))                // feedback_res
	MCFG_SN76477_VCO_PARAMS(0, CAP_U(0.1), RES_K(8.2))  // VCO volt + cap + res
	MCFG_SN76477_PITCH_VOLTAGE(5.0)                     // pitch_voltage
	MCFG_SN76477_SLF_PARAMS(CAP_U(1.0), RES_K(120))     // slf caps + res
	MCFG_SN76477_ONESHOT_PARAMS(0, 0)                   // oneshot caps + res: N/C
	MCFG_SN76477_VCO_MODE(1)                            // VCO mode
	MCFG_SN76477_MIXER_PARAMS(0, 0, 0)                  // mixer A, B, C
	MCFG_SN76477_ENVELOPE_PARAMS(1, 0)                  // envelope 1, 2
	MCFG_SN76477_ENABLE(1)                              // enable
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.5)

	MCFG_SOUND_ADD("samples", SAMPLES, 0)
	MCFG_SAMPLES_CHANNELS(6)
	MCFG_SAMPLES_NAMES(invaders_sample_names)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 1.0)
MACHINE_CONFIG_END


/* nodes - inputs */
#define INVADERS_SAUCER_HIT_EN                01
#define INVADERS_FLEET_DATA                   02
#define INVADERS_BONUS_MISSLE_BASE_EN         03
#define INVADERS_INVADER_HIT_EN               04
#define INVADERS_EXPLOSION_EN                 05
#define INVADERS_MISSILE_EN                   06

/* nodes - sounds */
#define INVADERS_NOISE                        NODE_10
#define INVADERS_SAUCER_HIT_SND               11
#define INVADERS_FLEET_SND                    12
#define INVADERS_BONUS_MISSLE_BASE_SND        13
#define INVADERS_INVADER_HIT_SND              14
#define INVADERS_EXPLOSION_SND                15
#define INVADERS_MISSILE_SND                  16


static const discrete_op_amp_info invaders_saucer_hit_op_amp_B3_9 =
{
	DISC_OP_AMP_IS_NORTON,
	0,                      /* no r1 */
	RES_K(100),             /* R72 */
	RES_M(1),               /* R71 */
	0,                      /* no r4 */
	CAP_U(1),               /* C23 */
	0,                      /* vN */
	12                      /* vP */
};


static const discrete_op_amp_osc_info invaders_saucer_hit_osc =
{
	DISC_OP_AMP_OSCILLATOR_1 | DISC_OP_AMP_IS_NORTON | DISC_OP_AMP_OSCILLATOR_OUT_CAP,
	RES_M(1),       /* R70 */
	RES_K(470),     /* R64 */
	RES_K(100),     /* R61 */
	RES_K(120),     /* R63 */
	RES_M(1),       /* R62 */
	0,              /* no r6 */
	0,              /* no r7 */
	0,              /* no r8 */
	CAP_U(0.1),     /* C21 */
	12,             /* vP */
};

static const discrete_op_amp_osc_info invaders_saucer_hit_vco =
{
	DISC_OP_AMP_OSCILLATOR_VCO_1 | DISC_OP_AMP_IS_NORTON | DISC_OP_AMP_OSCILLATOR_OUT_SQW,
	RES_M(1),       /* R65 */
	RES_K(470),     /* R66 */
	RES_K(680),     /* R67 */
	RES_M(1),       /* R69*/
	RES_M(1),       /* R68 */
	0,              /* no r6 */
	0,              /* no r7 */
	0,              /* no r8 */
	CAP_P(470),     /* C22 */
	12,             /* vP */
};


static const discrete_op_amp_info invaders_saucer_hit_op_amp_B3_10 =
{
	DISC_OP_AMP_IS_NORTON,
	RES_K(680),             /* R73 */
	RES_K(680),             /* R77 */
	RES_M(2.7),             /* R74 */
	RES_K(680),             /* R75 */
	0,                      /* no c */
	0,                      /* vN */
	12                      /* vP */
};


static const discrete_comp_adder_table invaders_thump_resistors =
{
	DISC_COMP_P_RESISTOR,
	0,                          /* no cDefault */
	4,                          /* length */
	{ RES_K(20) + RES_K(20),    /* R126 + R127 */
		RES_K(68),              /* R128 */
		RES_K(82),              /* R129 */
		RES_K(100) }                /* R130 */
};


static const discrete_555_desc invaders_thump_555 =
{
	DISC_555_OUT_ENERGY | DISC_555_OUT_DC,
	5,
	5.0 - 0.6,              /* 5V - diode drop */
	DEFAULT_TTL_V_LOGIC_1   /* Output of F3 7411 buffer */
};


static const discrete_555_desc invaders_bonus_555 =
{
	DISC_555_OUT_SQW | DISC_555_OUT_DC,
	5.0,                    /* 5V */
	DEFAULT_555_VALUES
};


static const discrete_op_amp_1sht_info invaders_invader_hit_1sht =
{
	DISC_OP_AMP_1SHT_1 | DISC_OP_AMP_IS_NORTON,
	RES_M(4.7),     /* R49 */
	RES_K(100),     /* R51 */
	RES_M(1),       /* R48 */
	RES_M(1),       /* R50 */
	RES_M(2.2),     /* R52 */
	CAP_U(0.1),     /* C18 */
	CAP_P(470),     /* C20 */
	0,              /* vN */
	12              /* vP */
};


static const discrete_op_amp_info invaders_invader_hit_op_amp_D3_10 =
{
	DISC_OP_AMP_IS_NORTON,
	0,              /* no r1 */
	RES_K(10),      /* R53 */
	RES_M(1),       /* R137 */
	0,              /* no r4 */
	CAP_U(0.47),    /* C19 */
	0,              /* vN */
	12              /* vP */
};


static const discrete_op_amp_osc_info invaders_invader_hit_osc =
{
	DISC_OP_AMP_OSCILLATOR_1 | DISC_OP_AMP_IS_NORTON | DISC_OP_AMP_OSCILLATOR_OUT_CAP,
	RES_M(1),       /* R37 */
	RES_K(10),      /* R41 */
	RES_K(100),     /* R38 */
	RES_K(120),     /* R40 */
	RES_M(1),       /* R39 */
	0,              /* no r6 */
	0,              /* no r7 */
	0,              /* no r8 */
	CAP_U(0.1),     /* C16 */
	12,             /* vP */
};


static const discrete_op_amp_osc_info invaders_invader_hit_vco =
{
	DISC_OP_AMP_OSCILLATOR_VCO_1 | DISC_OP_AMP_IS_NORTON | DISC_OP_AMP_OSCILLATOR_OUT_CAP,
	RES_M(1),       /* R42 */
	RES_K(470),     /* R43 */
	RES_K(680),     /* R44 */
	RES_M(1),       /* R46 */
	RES_M(1),       /* R45 */
	0,              /* no r6 */
	0,              /* no r7 */
	0,              /* no r8 */
	CAP_P(330),     /* C16 */
	12,             /* vP */
};


static const discrete_op_amp_info invaders_invader_hit_op_amp_D3_4 =
{
	DISC_OP_AMP_IS_NORTON,
	RES_K(470),     /* R55 */
	RES_K(680),     /* R54 */
	RES_M(2.7),     /* R56 */
	RES_K(680),     /* R57 */
	0,              /* no c */
	0,              /* vN */
	12              /* vP */
};


static const discrete_op_amp_1sht_info invaders_explosion_1sht =
{
	DISC_OP_AMP_1SHT_1 | DISC_OP_AMP_IS_NORTON,
	RES_M(4.7),     /* R90 */
	RES_K(100),     /* R88 */
	RES_M(1),       /* R91 */
	RES_M(1),       /* R89 */
	RES_M(2.2),     /* R92 */
	CAP_U(2.2),     /* C24 */
	CAP_P(470),     /* C25 */
	0,              /* vN */
	12              /* vP */
};


static const discrete_op_amp_tvca_info invaders_explosion_tvca =
{
	RES_M(2.7),                         /* R80 */
	RES_K(680),                         /* R79 */
	0,                                  /* no r3 */
	RES_K(680),                         /* R82 */
	RES_K(10),                          /* R93 */
	0,                                  /* no r6 */
	RES_K(680),                         /* R83 */
	0,                                  /* no r8 */
	0,                                  /* no r9 */
	0,                                  /* no r10 */
	0,                                  /* no r11 */
	CAP_U(1),                           /* C26 */
	0,                                  /* no c2 */
	0, 0,                               /* no c3, c4 */
	12.0 - OP_AMP_NORTON_VBE,           /* v1 */
	0,                                  /* no v2 */
	0,                                  /* no v3 */
	12,                                 /* vP */
	DISC_OP_AMP_TRIGGER_FUNCTION_NONE,  /* no f0 */
	DISC_OP_AMP_TRIGGER_FUNCTION_NONE,  /* no f1 */
	DISC_OP_AMP_TRIGGER_FUNCTION_TRG0,  /* f2 */
	DISC_OP_AMP_TRIGGER_FUNCTION_NONE,  /* no f3 */
	DISC_OP_AMP_TRIGGER_FUNCTION_NONE,  /* no f4 */
	DISC_OP_AMP_TRIGGER_FUNCTION_NONE   /* no f5 */
};


static const discrete_op_amp_1sht_info invaders_missle_1sht =
{
	DISC_OP_AMP_1SHT_1 | DISC_OP_AMP_IS_NORTON,
	RES_M(4.7),     /* R32 */
	RES_K(100),     /* R30 */
	RES_M(1),       /* R31 */
	RES_M(1),       /* R33 */
	RES_M(2.2),     /* R34 */
	CAP_U(1),       /* C12 */
	CAP_P(470),     /* C15 */
	0,              /* vN */
	12              /* vP */
};


static const discrete_op_amp_info invaders_missle_op_amp_B3 =
{
	DISC_OP_AMP_IS_NORTON,
	0,              /* no r1 */
	RES_K(10),      /* R35 */
	RES_M(1.5),     /* R36 */
	0,              /* no r4 */
	CAP_U(0.22),    /* C13 */
	0,              /* vN */
	12              /* vP */
};


static const discrete_op_amp_osc_info invaders_missle_op_amp_osc =
{
	DISC_OP_AMP_OSCILLATOR_VCO_3 | DISC_OP_AMP_IS_NORTON | DISC_OP_AMP_OSCILLATOR_OUT_SQW,
	1.0 / (1.0 / RES_M(1) + 1.0 / RES_K(330)) + RES_M(1.5),     /* R29||R11 + R12 */
	RES_M(1),       /* R16 */
	RES_K(560),     /* R17 */
	RES_M(2.2),     /* R19 */
	RES_M(1),       /* R16 */
	RES_M(4.7),     /* R14 */
	RES_M(3.3),     /* R13 */
	0,              /* no r8 */
	CAP_P(330),     /* C58 */
	12,             /* vP */
};


static const discrete_op_amp_info invaders_missle_op_amp_A3 =
{
	DISC_OP_AMP_IS_NORTON,
	RES_K(560),     /* R22 */
	RES_K(470),     /* R15 */
	RES_M(2.7),     /* R20 */
	RES_K(560),     /* R21 */
	0,              /* no c */
	0,              /* vN */
	12              /* vP */
};


static const discrete_op_amp_tvca_info invaders_missle_tvca =
{
	RES_M(2.7),                         /* R25 */
	RES_K(560),                         /* R23 */
	0,                                  /* no r3 */
	RES_K(560),                         /* R26 */
	RES_K(1),                           /*  */
	0,                                  /* no r6 */
	RES_K(560),                         /* R60 */
	0,                                  /* no r8 */
	0,                                  /* no r9 */
	0,                                  /* no r10 */
	0,                                  /* no r11 */
	CAP_U(0.1),                         /* C14 */
	0,                                  /* no c2 */
	0, 0,                               /* no c3, c4 */
	5,                                  /* v1 */
	0,                                  /* no v2 */
	0,                                  /* no v3 */
	12,                                 /* vP */
	DISC_OP_AMP_TRIGGER_FUNCTION_NONE,  /* no f0 */
	DISC_OP_AMP_TRIGGER_FUNCTION_NONE,  /* no f1 */
	DISC_OP_AMP_TRIGGER_FUNCTION_TRG0,  /* f2 */
	DISC_OP_AMP_TRIGGER_FUNCTION_NONE,  /* no f3 */
	DISC_OP_AMP_TRIGGER_FUNCTION_NONE,  /* no f4 */
	DISC_OP_AMP_TRIGGER_FUNCTION_NONE   /* no f5 */
};


static const discrete_mixer_desc invaders_mixer =
{
	DISC_MIXER_IS_OP_AMP,       /* type */
	{ RES_K(200),               /* R78 */
		RES_K(10) + 100 + 100,  /* R134 + R133 + R132 */
		RES_K(150),             /* R136 */
		RES_K(200),             /* R59 */
		RES_K(2) + RES_K(6.8) + RES_K(5.6), /* R86 + R85 + R84 */
		RES_K(150) },               /* R28 */
	{0},                        /* no rNode{} */
	{ 0,
		0,
		0,
		0,
		0,
		CAP_U(0.001) },         /* C11 */
	0,                          /* no rI */
	RES_K(100),                 /* R105 */
	0,                          /* no cF */
	CAP_U(0.1),                 /* C45 */
	0,                          /* vRef = ground */
	1                           /* gain */
};


/* sound board 1 or 2, for multi-board games */
#define INVADERS_NODE(_node, _board)    (NODE(_node + ((_board - 1) * 100)))

/************************************************
 * Noise Generator
 ************************************************/
/* Noise clock was breadboarded and measured at 7515 */
#define INVADERS_NOISE_GENERATOR                                                \
	DISCRETE_LFSR_NOISE(INVADERS_NOISE,                 /* IC N5, pin 10 */     \
					1,                                  /* ENAB */              \
					1,                                  /* no RESET */          \
					7515,                               /* CLK in Hz */         \
					12,                                 /* p-p AMPL */          \
					0,                                  /* no FEED input */     \
					12.0/2,                             /* dc BIAS */           \
					&midway_lfsr)


/************************************************
 * Saucer Hit
 ************************************************/
#define INVADERS_SAUCER_HIT(_board)                                                     \
	DISCRETE_INPUTX_LOGIC(INVADERS_NODE(INVADERS_SAUCER_HIT_EN, _board), 12, 0, 0)      \
	DISCRETE_OP_AMP(INVADERS_NODE(20, _board),                      /* IC B3, pin 9 */  \
					1,                                              /* ENAB */          \
					0,                                              /* no IN0 */        \
					INVADERS_NODE(INVADERS_SAUCER_HIT_EN, _board),  /* IN1 */           \
					&invaders_saucer_hit_op_amp_B3_9)                                   \
	DISCRETE_OP_AMP_OSCILLATOR(INVADERS_NODE(21, _board),           /* IC A4, pin 5 */  \
					1,                                              /* ENAB */          \
					&invaders_saucer_hit_osc)                                           \
	DISCRETE_OP_AMP_VCO1(INVADERS_NODE(22, _board),                 /* IC A4, pin 9 */  \
					1,                                              /* ENAB */          \
					INVADERS_NODE(21, _board),                      /* VMOD1 */         \
					&invaders_saucer_hit_vco)                                           \
	DISCRETE_OP_AMP(INVADERS_NODE(INVADERS_SAUCER_HIT_SND, _board), /* IC B3, pin 10 */ \
					1,                                              /* ENAB */          \
					INVADERS_NODE(22, _board),                      /* IN0 */           \
					INVADERS_NODE(20, _board),                      /* IN1 */           \
					&invaders_saucer_hit_op_amp_B3_10)


/************************************************
 * Fleet movement
 ************************************************/
#define INVADERS_FLEET(_board)                                                          \
	DISCRETE_INPUT_DATA  (INVADERS_NODE(INVADERS_FLEET_DATA, _board))                   \
	DISCRETE_COMP_ADDER(INVADERS_NODE(30, _board),                                      \
					INVADERS_NODE(INVADERS_FLEET_DATA, _board),     /* DATA */          \
					&invaders_thump_resistors)                                          \
	DISCRETE_555_ASTABLE(INVADERS_NODE(31, _board),                 /* IC F3, pin 6 */  \
					1,                                              /* RESET */         \
					INVADERS_NODE(30, _board),                      /* R1 */            \
					RES_K(75),                                      /* R131 */          \
					CAP_U(0.1),                                     /* C29 */           \
					&invaders_thump_555)                                                \
	DISCRETE_RCFILTER(INVADERS_NODE(32, _board),                                        \
					INVADERS_NODE(31, _board),                      /* IN0 */           \
					100,                                            /* R132 */          \
					CAP_U(4.7) )                                    /* C31 */           \
	DISCRETE_RCFILTER(INVADERS_NODE(INVADERS_FLEET_SND, _board),                        \
					INVADERS_NODE(32, _board),                      /* IN0 */           \
					100 + 100,                                      /* R132 + R133 */   \
					CAP_U(10) )                                     /* C32 */


/************************************************
 * Bonus Missle Base
 ************************************************/
#define INVADERS_BONUS_MISSLE_BASE(_board)                                                                          \
	DISCRETE_INPUT_LOGIC (INVADERS_NODE(INVADERS_BONUS_MISSLE_BASE_EN, _board))                                     \
	DISCRETE_555_ASTABLE(INVADERS_NODE(40, _board),                     /* IC F4, pin 9 */                          \
					INVADERS_NODE(INVADERS_BONUS_MISSLE_BASE_EN, _board),/* RESET */                                \
					RES_K(100),                                         /* R94 */                                   \
					RES_K(47),                                          /* R95 */                                   \
					CAP_U(1),                                           /* C34 */                                   \
					&invaders_bonus_555)                                                                            \
	DISCRETE_SQUAREWFIX(INVADERS_NODE(41, _board),                                                                  \
					1,                                                  /* ENAB */                                  \
					480,                                                /* FREQ */                                  \
					1,                                                  /* AMP */                                   \
					50,                                                 /* DUTY */                                  \
					1.0/2,                                              /* BIAS */                                  \
					0)                                                  /* PHASE */                                 \
	DISCRETE_LOGIC_AND3(INVADERS_NODE(42, _board),                      /* IC F3, pin 12 */                         \
					INVADERS_NODE(INVADERS_BONUS_MISSLE_BASE_EN, _board),/* INP0 */                                 \
					INVADERS_NODE(41, _board),                          /* INP1 */                                  \
					INVADERS_NODE(40, _board) )                         /* INP2 */                                  \
	DISCRETE_GAIN(INVADERS_NODE(INVADERS_BONUS_MISSLE_BASE_SND, _board),/* adjust from logic to TTL voltage level */\
					INVADERS_NODE(42, _board),                          /* IN0 */                                   \
					DEFAULT_TTL_V_LOGIC_1)                              /* GAIN */


/************************************************
 * Invader Hit
 ************************************************/
#define INVADERS_INVADER_HIT(_board, _type)                                                 \
	DISCRETE_INPUTX_LOGIC(INVADERS_NODE(INVADERS_INVADER_HIT_EN, _board), 5, 0, 0)          \
	DISCRETE_OP_AMP_ONESHOT(INVADERS_NODE(50, _board),                  /* IC D3, pin 9 */  \
					INVADERS_NODE(INVADERS_INVADER_HIT_EN, _board),     /* TRIG */          \
					&_type##_invader_hit_1sht)                                              \
	DISCRETE_OP_AMP(INVADERS_NODE(51, _board),                          /* IC D3, pin 10 */ \
					1,                                                  /* ENAB */          \
					0,                                                  /* no IN0 */        \
					INVADERS_NODE(50, _board),                          /* IN1 */           \
					&invaders_invader_hit_op_amp_D3_10)                                     \
	DISCRETE_OP_AMP_OSCILLATOR(INVADERS_NODE(52, _board),               /* IC B4, pin 5 */  \
					1,                                                  /* ENAB */          \
					&_type##_invader_hit_osc)                                               \
	DISCRETE_OP_AMP_VCO1(INVADERS_NODE(53, _board),                     /* IC B4, pin 4 */  \
					1,                                                  /* ENAB */          \
					INVADERS_NODE(52, _board),                          /* VMOD1 */         \
					&invaders_invader_hit_vco)                                              \
	DISCRETE_OP_AMP(INVADERS_NODE(INVADERS_INVADER_HIT_SND, _board),    /* IC D3, pin 4 */  \
					1,                                                  /* ENAB */          \
					INVADERS_NODE(53, _board),                          /* IN0 */           \
					INVADERS_NODE(51, _board),                          /* IN1 */           \
					&invaders_invader_hit_op_amp_D3_4)


/************************************************
 * Explosion
 ************************************************/
#define INVADERS_EXPLOSION(_board)                                                      \
	DISCRETE_INPUTX_LOGIC(INVADERS_NODE(INVADERS_EXPLOSION_EN, _board), 5, 0, 0)        \
	DISCRETE_OP_AMP_ONESHOT(INVADERS_NODE(60, _board),              /* IC D2, pin 10 */ \
					INVADERS_NODE(INVADERS_EXPLOSION_EN, _board),   /* TRIG */          \
					&invaders_explosion_1sht)                                           \
	DISCRETE_OP_AMP_TRIG_VCA(INVADERS_NODE(61, _board),             /* IC D2, pin 4 */  \
					INVADERS_NODE(60, _board),                      /* TRG0 */          \
					0,                                              /* no TRG1 */       \
					0,                                              /* no TRG2 */       \
					INVADERS_NOISE,                                 /* IN0 */           \
					0,                                              /* no IN1 */        \
					&invaders_explosion_tvca)                                           \
	DISCRETE_RCFILTER(INVADERS_NODE(62, _board),                                        \
					INVADERS_NODE(61, _board),                      /* IN0 */           \
					RES_K(5.6),                                     /* R84 */           \
					CAP_U(0.1) )                                    /* C27 */           \
	DISCRETE_RCFILTER(INVADERS_NODE(INVADERS_EXPLOSION_SND, _board),                    \
					INVADERS_NODE(62, _board),                      /* IN0 */           \
					RES_K(5.6) + RES_K(6.8),                        /* R84 + R85 */     \
					CAP_U(0.1) )                                    /* C28 */


/************************************************
 * Missle Sound
 ************************************************/
#define INVADERS_MISSILE(_board, _type)                                                             \
	DISCRETE_INPUTX_LOGIC(INVADERS_NODE(INVADERS_MISSILE_EN, _board), 5, 0, 0)                      \
	DISCRETE_OP_AMP_ONESHOT(INVADERS_NODE(70, _board),                      /* IC B3, pin 4 */      \
					INVADERS_NODE(INVADERS_MISSILE_EN, _board),             /* TRIG */              \
					&_type##_missle_1sht)                                                           \
	DISCRETE_OP_AMP(INVADERS_NODE(71, _board),                              /* IC B3, pin 5 */      \
					1,                                                      /* ENAB */              \
					0,                                                      /* no IN0 */            \
					INVADERS_NODE(70, _board),                              /* IN1 */               \
					&invaders_missle_op_amp_B3)                                                     \
	/* next 2 modules simulate the D1 voltage drop */                                               \
	DISCRETE_ADDER2(INVADERS_NODE(72, _board),                                                      \
					1,                                                      /* ENAB */              \
					INVADERS_NODE(71, _board),                              /* IN0 */               \
					-0.5)                                                   /* IN1 */               \
	DISCRETE_CLAMP(INVADERS_NODE(73, _board),                                                       \
					INVADERS_NODE(72, _board),                              /* IN0 */               \
					0,                                                      /* MIN */               \
					12)                                                     /* MAX */               \
	DISCRETE_CRFILTER(INVADERS_NODE(74, _board),                                                    \
					INVADERS_NOISE,                                         /* IN0 */               \
					RES_M(1) + RES_K(330),                                  /* R29, R11 */          \
					CAP_U(0.1) )                                            /* C57 */               \
	DISCRETE_GAIN(INVADERS_NODE(75, _board),                                                        \
					INVADERS_NODE(74, _board),                              /* IN0 */               \
					RES_K(330)/(RES_M(1) + RES_K(330)))                     /* GAIN - R29 : R11 */  \
	DISCRETE_OP_AMP_VCO2(INVADERS_NODE(76, _board),                         /* IC C1, pin 4 */      \
					1,                                                      /* ENAB */              \
					INVADERS_NODE(75, _board),                              /* VMOD1 */             \
					INVADERS_NODE(73, _board),                              /* VMOD2 */             \
					&invaders_missle_op_amp_osc)                                                    \
	DISCRETE_OP_AMP(INVADERS_NODE(77, _board),                              /* IC A3, pin 9 */      \
					1,                                                      /* ENAB */              \
					INVADERS_NODE(76, _board),                              /* IN0 */               \
					INVADERS_NODE(73, _board),                              /* IN1 */               \
					&invaders_missle_op_amp_A3)                                                     \
	DISCRETE_OP_AMP_TRIG_VCA(INVADERS_NODE(INVADERS_MISSILE_SND, _board),   /* IC A3, pin 10 */     \
					INVADERS_NODE(INVADERS_MISSILE_EN, _board),             /* TRG0 */              \
					0,                                                      /* no TRG1 */           \
					0,                                                      /* no TRG2 */           \
					INVADERS_NODE(77, _board),                              /* IN0 */               \
					0,                                                      /* no IN1 */            \
					&invaders_missle_tvca)


/************************************************
 * Final mix
 ************************************************/
#define INVADERS_MIXER(_board, _type)                                                   \
	DISCRETE_MIXER6(INVADERS_NODE(90, _board),                                          \
					1,                                                      /* ENAB */  \
					INVADERS_NODE(INVADERS_SAUCER_HIT_SND, _board),         /* IN0 */   \
					INVADERS_NODE(INVADERS_FLEET_SND, _board),              /* IN1 */   \
					INVADERS_NODE(INVADERS_BONUS_MISSLE_BASE_SND, _board),  /* IN2 */   \
					INVADERS_NODE(INVADERS_INVADER_HIT_SND, _board),        /* IN3 */   \
					INVADERS_NODE(INVADERS_EXPLOSION_SND, _board),          /* IN4 */   \
					INVADERS_NODE(INVADERS_MISSILE_SND, _board),            /* IN5 */   \
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


MACHINE_CONFIG_FRAGMENT( invaders_audio )
	MCFG_SPEAKER_STANDARD_MONO("mono")

	MCFG_SOUND_ADD("snsnd", SN76477, 0)
	MCFG_SN76477_NOISE_PARAMS(0, 0, 0)                  // noise + filter: N/C
	MCFG_SN76477_DECAY_RES(0)                           // decay_res: N/C
	MCFG_SN76477_ATTACK_PARAMS(0, RES_K(100))           // attack_decay_cap + attack_res
	MCFG_SN76477_AMP_RES(RES_K(56))                     // amplitude_res
	MCFG_SN76477_FEEDBACK_RES(RES_K(10))                // feedback_res
	MCFG_SN76477_VCO_PARAMS(0, CAP_U(0.1), RES_K(8.2))  // VCO volt + cap + res
	MCFG_SN76477_PITCH_VOLTAGE(5.0)                     // pitch_voltage
	MCFG_SN76477_SLF_PARAMS(CAP_U(1.0), RES_K(120))     // slf caps + res
	MCFG_SN76477_ONESHOT_PARAMS(0, 0)                   // oneshot caps + res: N/C
	MCFG_SN76477_VCO_MODE(1)                            // VCO mode
	MCFG_SN76477_MIXER_PARAMS(0, 0, 0)                  // mixer A, B, C
	MCFG_SN76477_ENVELOPE_PARAMS(1, 0)                  // envelope 1, 2
	MCFG_SN76477_ENABLE(1)                              // enable
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.5)

	MCFG_SOUND_ADD("discrete", DISCRETE, 0)
	MCFG_DISCRETE_INTF(invaders)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.5)
MACHINE_CONFIG_END


WRITE8_MEMBER(mw8080bw_state::invaders_audio_1_w)
{
	m_sn->enable_w((~data >> 0) & 0x01);    /* saucer sound */

	m_discrete->write(space, INVADERS_NODE(INVADERS_MISSILE_EN, 1), data & 0x02);
	m_discrete->write(space, INVADERS_NODE(INVADERS_EXPLOSION_EN, 1), data & 0x04);
	m_discrete->write(space, INVADERS_NODE(INVADERS_INVADER_HIT_EN, 1), data & 0x08);
	m_discrete->write(space, INVADERS_NODE(INVADERS_BONUS_MISSLE_BASE_EN, 1), data & 0x10);

	machine().sound().system_enable(data & 0x20);

	/* D6 and D7 are not connected */
}


WRITE8_MEMBER(mw8080bw_state::invaders_audio_2_w)
{
	m_discrete->write(space, INVADERS_NODE(INVADERS_FLEET_DATA, 1), data & 0x0f);
	m_discrete->write(space, INVADERS_NODE(INVADERS_SAUCER_HIT_EN, 1), data & 0x10);

	/* the flip screen line is only connected on the cocktail PCB */
	if (invaders_is_cabinet_cocktail())
	{
		m_flip_screen = (data >> 5) & 0x01;
	}

	/* D6 and D7 are not connected */
}



/*************************************
 *
 *  Blue Shark
 *
 *  Discrete sound emulation:
 *   Jan 2007, D.R.
 *   Oct 2009, D.R.
 *
 *************************************/

/* nodes - inputs */
#define BLUESHRK_OCTOPUS_EN         NODE_01
#define BLUESHRK_HIT_EN             NODE_02
#define BLUESHRK_SHARK_EN           NODE_03
#define BLUESHRK_SHOT_EN            NODE_04
#define BLUESHRK_GAME_ON_EN         NODE_05

/* nodes - sounds */
#define BLUESHRK_NOISE_1            NODE_11
#define BLUESHRK_NOISE_2            NODE_12
#define BLUESHRK_OCTOPUS_SND        NODE_13
#define BLUESHRK_HIT_SND            NODE_14
#define BLUESHRK_SHARK_SND          NODE_15
#define BLUESHRK_SHOT_SND           NODE_16

/* Parts List - Resistors */
#define BLUESHRK_R300   RES_M(1)
#define BLUESHRK_R301   RES_K(100)
#define BLUESHRK_R302   RES_M(1)
#define BLUESHRK_R303   RES_K(33)
#define BLUESHRK_R304   RES_K(120)
#define BLUESHRK_R305   RES_M(1)
#define BLUESHRK_R306   RES_K(470)
#define BLUESHRK_R307   RES_K(680)
#define BLUESHRK_R308   RES_M(1)
#define BLUESHRK_R309   RES_M(1)
#define BLUESHRK_R310   RES_K(680)
#define BLUESHRK_R311   RES_K(1)
#define BLUESHRK_R312   RES_K(100)
#define BLUESHRK_R313   RES_M(1)
#define BLUESHRK_R314   RES_M(1)
#define BLUESHRK_R315   RES_M(4.7)
#define BLUESHRK_R316   RES_M(2.2)
#define BLUESHRK_R317   RES_K(10)
#define BLUESHRK_R318   RES_M(1)
#define BLUESHRK_R319   RES_K(680)
#define BLUESHRK_R320   RES_M(2.7)
#define BLUESHRK_R321   RES_K(680)
#define BLUESHRK_R324   RES_K(750)
#define BLUESHRK_R520   RES_K(510)
#define BLUESHRK_R521   RES_K(22)
#define BLUESHRK_R529   RES_K(33)
#define BLUESHRK_R601   RES_K(47)
#define BLUESHRK_R602   RES_K(22)
#define BLUESHRK_R603   RES_K(39)
#define BLUESHRK_R604   RES_K(1)
#define BLUESHRK_R605   RES_M(1)
#define BLUESHRK_R700   RES_K(68)
#define BLUESHRK_R701   RES_K(470)
#define BLUESHRK_R702   RES_M(1.2)
#define BLUESHRK_R703   RES_M(1.5)
#define BLUESHRK_R704   RES_K(22)
#define BLUESHRK_R705   RES_K(100)
#define BLUESHRK_R706   RES_K(470)
#define BLUESHRK_R707   RES_M(1.2)
#define BLUESHRK_R708   RES_M(1.5)
#define BLUESHRK_R709   RES_K(22)
#define BLUESHRK_R710   RES_K(470)
#define BLUESHRK_R711   RES_K(39)
#define BLUESHRK_R712   RES_M(1.2)
#define BLUESHRK_R713   RES_M(1.5)
#define BLUESHRK_R714   RES_K(22)
#define BLUESHRK_R715   RES_K(47)
#define BLUESHRK_R716   RES_K(75)
#define BLUESHRK_R717   RES_M(1.5)
#define BLUESHRK_R718   RES_M(2.2)
#define BLUESHRK_R719   RES_K(560)
#define BLUESHRK_R720   RES_M(1.5)
#define BLUESHRK_R721   RES_M(2.2)
#define BLUESHRK_R722   RES_M(2.2)
#define BLUESHRK_R723   RES_K(560)
#define BLUESHRK_R724   RES_K(12)
#define BLUESHRK_R725   RES_K(68)
#define BLUESHRK_R726   RES_K(330)
#define BLUESHRK_R727   RES_M(2.2)
#define BLUESHRK_R728   RES_M(1)
#define BLUESHRK_R730   RES_K(56)
#define BLUESHRK_R1000  RES_K(1)

/* Parts List - Capacitors */
#define BLUESHRK_C300   CAP_U(0.1)
#define BLUESHRK_C301   CAP_P(470)
#define BLUESHRK_C302   CAP_P(470)
#define BLUESHRK_C303   CAP_U(0.47)
#define BLUESHRK_C304   CAP_U(1)
#define BLUESHRK_C305   CAP_U(1)
#define BLUESHRK_C508   CAP_U(1)
#define BLUESHRK_C600   CAP_U(2.2)
#define BLUESHRK_C602   CAP_U(0.022)
#define BLUESHRK_C603   CAP_U(0.01)
#define BLUESHRK_C604   CAP_U(0.015)
#define BLUESHRK_C606   CAP_U(1)
#define BLUESHRK_C700   CAP_U(22)
#define BLUESHRK_C701   CAP_U(22)
#define BLUESHRK_C702   CAP_U(10)
#define BLUESHRK_C703   CAP_U(0.033)
#define BLUESHRK_C704   CAP_U(0.015)
#define BLUESHRK_C705   CAP_U(0.015)
#define BLUESHRK_C706   CAP_U(0.033)
#define BLUESHRK_C707   CAP_U(2.2)
#define BLUESHRK_C708   CAP_U(1)
#define BLUESHRK_C900   CAP_U(10)


static const discrete_op_amp_osc_info blueshrk_octopus_osc =
{
	DISC_OP_AMP_OSCILLATOR_1 | DISC_OP_AMP_IS_NORTON | DISC_OP_AMP_OSCILLATOR_OUT_CAP,
	BLUESHRK_R300, BLUESHRK_R303, BLUESHRK_R301, BLUESHRK_R304, BLUESHRK_R302, 0, 0, 0, /* r1, r2, r3, r4, r5, r6, r7, r8 */
	BLUESHRK_C300, 12               /*c, vP */
};

static const discrete_op_amp_osc_info blueshrk_octopus_vco =
{
	DISC_OP_AMP_OSCILLATOR_VCO_1 | DISC_OP_AMP_IS_NORTON | DISC_OP_AMP_OSCILLATOR_OUT_SQW,
	BLUESHRK_R305, BLUESHRK_R306, BLUESHRK_R307, BLUESHRK_R309, BLUESHRK_R308, 0, 0, 0, /* r1, r2, r3, r4, r5, r6, r7, r8 */
	BLUESHRK_C301, 12               /*c, vP */
};

static const discrete_op_amp_1sht_info blueshrk_octopus_oneshot =
{
	DISC_OP_AMP_1SHT_1 | DISC_OP_AMP_IS_NORTON,
	BLUESHRK_R315, BLUESHRK_R312, BLUESHRK_R314, BLUESHRK_R313, BLUESHRK_R316,  /* r1, r2, r3, r4, r5 */
	BLUESHRK_C303, BLUESHRK_C302,                                               /* c1, c2 */
	0, 12                                                                       /* vN, vP */
};

static const discrete_integrate_info blueshrk_octopus_integrate =
{
	DISC_INTEGRATE_OP_AMP_1 | DISC_OP_AMP_IS_NORTON,
	BLUESHRK_R318, BLUESHRK_R317, 0, BLUESHRK_C304,     /* r1, r2, r3, c */
	12, 12,                                             /* v1, vP */
	0, 0, 0                                             /* f0, f1, f2 */
};

static const discrete_op_amp_info blueshrk_octopus_amp =
{
	DISC_OP_AMP_IS_NORTON,
	BLUESHRK_R310, BLUESHRK_R319, BLUESHRK_R320, BLUESHRK_R321, /* r1, r2, r3, r4 */
	0, 0, 12                                                        /* c, vN, vP */
};

static const discrete_lfsr_desc blueshrk_lfsr =
{
	DISC_CLK_IS_FREQ,
	17,                 /* bit length */
						/* the RC network fed into pin 4, has the effect
						   of presetting all bits high at power up */
	0x1ffff,            /* reset value */
	4,                  /* use bit 4 as XOR input 0 */
	16,                 /* use bit 16 as XOR input 1 */
	DISC_LFSR_XOR,      /* feedback stage1 is XOR */
	DISC_LFSR_OR,       /* feedback stage2 is just stage 1 output OR with external feed */
	DISC_LFSR_REPLACE,  /* feedback stage3 replaces the shifted register contents */
	0x000001,           /* everything is shifted into the first bit only */
	DISC_LFSR_FLAG_OUTPUT_SR_SN1,       /* output is not inverted */
	12                  /* output bit */
};

static const discrete_555_desc blueshrk_555_H1B =
{
	DISC_555_OUT_ENERGY | DISC_555_OUT_DC,
	5,              /* B+ voltage of 555 */
	DEFAULT_555_CHARGE,
	12              /* the OC buffer H2 converts the output voltage to 12V. */
};

static const discrete_op_amp_osc_info blueshrk_shark_osc1 =
{
	DISC_OP_AMP_OSCILLATOR_2 | DISC_OP_AMP_IS_NORTON | DISC_OP_AMP_OSCILLATOR_OUT_ENERGY,
	0, BLUESHRK_R701, BLUESHRK_R703, BLUESHRK_R702, 0, BLUESHRK_R700, 0, 0, /* r1, r2, r3, r4, r5, r6, r7, r8 */
	BLUESHRK_C700, 12               /*c, vP */
};

static const discrete_op_amp_osc_info blueshrk_shark_osc2 =
{
	DISC_OP_AMP_OSCILLATOR_2 | DISC_OP_AMP_IS_NORTON | DISC_OP_AMP_OSCILLATOR_OUT_ENERGY,
	0, BLUESHRK_R706, BLUESHRK_R708, BLUESHRK_R707, 0, BLUESHRK_R705, 0, 0, /* r1, r2, r3, r4, r5, r6, r7, r8 */
	BLUESHRK_C700, 12               /*c, vP */
};

static const discrete_op_amp_osc_info blueshrk_shark_osc3 =
{
	DISC_OP_AMP_OSCILLATOR_2 | DISC_OP_AMP_IS_NORTON | DISC_OP_AMP_OSCILLATOR_OUT_ENERGY,
	0, BLUESHRK_R711, BLUESHRK_R713, BLUESHRK_R712, 0, BLUESHRK_R710, 0, 0, /* r1, r2, r3, r4, r5, r6, r7, r8 */
	BLUESHRK_C700, 12               /*c, vP */
};

static const discrete_mixer_desc blueshrk_shark_mixer =
{
	DISC_MIXER_IS_RESISTOR,
	{BLUESHRK_R704, BLUESHRK_R709, BLUESHRK_R714},
	{0}, {0}, 0, 0, 0, 0, 0, 1  /* r_node, c, rI, rF, cF, cAmp, vRef, gain */
};

static const discrete_op_amp_info blueshrk_shark_amp_m3 =
{
	DISC_OP_AMP_IS_NORTON,
	0, BLUESHRK_R715 + RES_3_PARALLEL(BLUESHRK_R704, BLUESHRK_R709, BLUESHRK_R714), BLUESHRK_R716, 0,       /* r1, r2, r3, r4 */
	0, 0, 12                                /* c, vN, vP */
};

static const discrete_op_amp_osc_info blueshrk_shark_vco =
{
	DISC_OP_AMP_OSCILLATOR_VCO_3 | DISC_OP_AMP_IS_NORTON | DISC_OP_AMP_OSCILLATOR_OUT_ENERGY,
	BLUESHRK_R717, BLUESHRK_R722, BLUESHRK_R719, BLUESHRK_R721, BLUESHRK_R720,  /* r1, r2, r3, r4, r5 */
	0, 0, BLUESHRK_R718,    /* r6, r7, r8 */
	BLUESHRK_C703, 12                   /*c, vP */
};

static const discrete_op_amp_info blueshrk_shark_amp_k3 =
{
	DISC_OP_AMP_IS_NORTON,
	BLUESHRK_R724 + BLUESHRK_R725 + BLUESHRK_R726,      /* r1 */
	BLUESHRK_R723 , BLUESHRK_R727, BLUESHRK_R728,       /* r2, r3, r4 */
	0, 0, 12                                            /* c, vN, vP */
};

static const discrete_mixer_desc blueshrk_mixer =
{
	DISC_MIXER_IS_RESISTOR,
	{BLUESHRK_R324, RES_2_PARALLEL(BLUESHRK_R520, BLUESHRK_R521) + BLUESHRK_R529, BLUESHRK_R604 + BLUESHRK_R605, BLUESHRK_R730},
	{0},    /* r_node */
	{BLUESHRK_C305, BLUESHRK_C508, BLUESHRK_C606, BLUESHRK_C708},
	0, 0, 0, BLUESHRK_C900, 0, 1    /* rI, rF, cF, cAmp, vRef, gain */
};

static DISCRETE_SOUND_START(blueshrk)

	/************************************************
	 * Input register mapping
	 ************************************************/
	DISCRETE_INPUTX_LOGIC(BLUESHRK_OCTOPUS_EN, 12, 0, 0)
	DISCRETE_INPUT_LOGIC(BLUESHRK_HIT_EN)
	DISCRETE_INPUT_LOGIC(BLUESHRK_SHARK_EN)
	DISCRETE_INPUT_LOGIC(BLUESHRK_SHOT_EN)
	DISCRETE_INPUT_LOGIC(BLUESHRK_GAME_ON_EN)

	/************************************************
	 * Octopus sound
	 ************************************************/
	DISCRETE_OP_AMP_OSCILLATOR(NODE_20,         /* IC M5, pin 5 */
		1,                                      /* ENAB */
		&blueshrk_octopus_osc)
	DISCRETE_OP_AMP_VCO1(NODE_21,               /* IC M5, pin 10 */
		1,                                      /* ENAB */
		NODE_20,                                /* VMOD1 */
		&blueshrk_octopus_vco)
	DISCRETE_OP_AMP_ONESHOT(NODE_22,            /* IC J5, pin 10 */
		BLUESHRK_OCTOPUS_EN, &blueshrk_octopus_oneshot)
	DISCRETE_INTEGRATE(NODE_23,                 /* IC J5, pin 5 */
		NODE_22, 0,                             /* TRG0,TRG1 */
		&blueshrk_octopus_integrate)
	DISCRETE_OP_AMP(BLUESHRK_OCTOPUS_SND,       /* IC J5, pin 4 */
		1,                                      /* ENAB */
		NODE_21, NODE_23,                       /* IN0,IN1 */
		&blueshrk_octopus_amp)

	/************************************************
	 * Noise
	 ************************************************/
	/* Noise clock was breadboarded and measured at 7700Hz */
	DISCRETE_LFSR_NOISE(BLUESHRK_NOISE_1,           /* IC N5, pin 10 (NODE_11) */
		1, 1,                                       /* ENAB, RESET */
		7700, 12.0, 0, 12.0 / 2, &blueshrk_lfsr)    /* CLK,AMPL,FEED,BIAS,LFSRTB */
	DISCRETE_BIT_DECODE(BLUESHRK_NOISE_2,           /* IC N5, pin 13 */
		NODE_SUB(BLUESHRK_NOISE_1, 1), 8, 12)       /* INP,BIT_N,VOUT */

	/************************************************
	 * Shot sound
	 ************************************************/
	DISCRETE_CONSTANT(BLUESHRK_SHOT_SND, 0)     /* placeholder for incomplete sound */

	/************************************************
	 * Hit sound
	 ************************************************/
	DISCRETE_COUNTER(NODE_40,                           /* IC H3, pin 5 */
		1, BLUESHRK_HIT_EN,                             /* ENAB,RESET */
		FREQ_OF_555(BLUESHRK_R601, 0, BLUESHRK_C600),   /* CLK - IC H1, pin 9 */
		0,1, DISC_COUNT_UP, 0,                          /* MIN,MAX,DIR,INIT0 */
		DISC_CLK_IS_FREQ)
	DISCRETE_SWITCH(NODE_41,                    /* value of toggled caps */
		1,                                      /* ENAB */
		NODE_40,                                /* SWITCH */
		BLUESHRK_C602 + BLUESHRK_C603,          /* INP0 - IC H3, pin 5 low */
		BLUESHRK_C604)                          /* INP1 - IC H3, pin 6 low  */
	DISCRETE_555_ASTABLE(BLUESHRK_HIT_SND,      /* IC H2, pin 2 */
		BLUESHRK_HIT_EN,                        /* RESET */
		BLUESHRK_R602, BLUESHRK_R603, NODE_41,  /* R1,R2,C */
		&blueshrk_555_H1B)

	/************************************************
	 * Shark sound
	 ************************************************/
	DISCRETE_OP_AMP_OSCILLATOR(NODE_50,         /* IC M3, pin 4 */
		1,                                      /* ENAB */
		&blueshrk_shark_osc1)
	DISCRETE_OP_AMP_OSCILLATOR(NODE_51,         /* IC M3, pin 5 */
		1,                                      /* ENAB */
		&blueshrk_shark_osc2)
	DISCRETE_OP_AMP_OSCILLATOR(NODE_52,         /* IC M3, pin 9 */
		1,                                      /* ENAB */
		&blueshrk_shark_osc3)
	DISCRETE_MIXER3(NODE_53,
		1,                                      /* ENAB */
		NODE_50, NODE_51, NODE_52, &blueshrk_shark_mixer)
	/* threshold detector */
	/* if any of the above oscillators are low, then the output is low */
	DISCRETE_OP_AMP(NODE_54,                    /* IC M3, pin 10 */
		1,                                      /* ENAB */
		0, NODE_53,                             /* IN0,IN1 */
		&blueshrk_shark_amp_m3)
	DISCRETE_ADDER2(NODE_55,                    /* diode drops voltage */
		1, NODE_54, -0.7)                       /* ENAB,IN0,IN1 */
	DISCRETE_CLAMP(NODE_56, NODE_55, 0, 12)     /* IN0,MIN,MAX */
	/* VCO disabled if any of the above oscillators or enable are low */
	DISCRETE_OP_AMP_VCO1(NODE_57,               /* IC K3, pin 5 */
		BLUESHRK_SHARK_EN, NODE_56,             /* ENAB,VMOD1 */
		&blueshrk_shark_vco)
	DISCRETE_RCFILTER(NODE_58,
		BLUESHRK_NOISE_1,                       /* IN0 */
		BLUESHRK_R724, BLUESHRK_C704)
	DISCRETE_RCFILTER(NODE_59,
		NODE_58,                                /* IN0 */
		BLUESHRK_R724 + BLUESHRK_R725, BLUESHRK_C704)
	DISCRETE_RCFILTER(NODE_60,
		NODE_59,                                /* IN0 */
		BLUESHRK_R724 + BLUESHRK_R725 + BLUESHRK_R726, BLUESHRK_C704)
	DISCRETE_OP_AMP(NODE_61,                    /* IC K3, pin 10 */
		1,                                      /* ENAB */
		NODE_60, NODE_57,                       /* IN0,IN1 */
		&blueshrk_shark_amp_k3)
	/* the opamp output is connected directly to a capacitor */
	/* we will simulate this using a 1 ohm resistor */
	DISCRETE_RCFILTER(BLUESHRK_SHARK_SND,
		NODE_61,                                /* IN0 */
		1, BLUESHRK_C707)

	/************************************************
	 * Combine all sound sources.
	 ************************************************/
	DISCRETE_MIXER4(NODE_91,
		BLUESHRK_GAME_ON_EN,
		BLUESHRK_OCTOPUS_SND,
		BLUESHRK_SHOT_SND,
		BLUESHRK_HIT_SND,
		BLUESHRK_SHARK_SND,
		&blueshrk_mixer)

	DISCRETE_OUTPUT(NODE_91, 90000)
DISCRETE_SOUND_END


MACHINE_CONFIG_FRAGMENT( blueshrk_audio )
	MCFG_SPEAKER_STANDARD_MONO("mono")
	MCFG_SOUND_ADD("discrete", DISCRETE, 0)
	MCFG_DISCRETE_INTF(blueshrk)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.25)
MACHINE_CONFIG_END


WRITE8_MEMBER(mw8080bw_state::blueshrk_audio_w)
{
	m_discrete->write(space, BLUESHRK_GAME_ON_EN, (data >> 0) & 0x01);

	m_discrete->write(space, BLUESHRK_SHOT_EN, (data >> 1) & 0x01);

	m_discrete->write(space, BLUESHRK_HIT_EN, (data >> 2) & 0x01);

	m_discrete->write(space, BLUESHRK_SHARK_EN, (data >> 3) & 0x01);

	/* if (data & 0x10)  enable KILLED DIVER sound, this circuit
	   doesn't appear to be on the schematics */

	m_discrete->write(space, BLUESHRK_OCTOPUS_EN, (data >> 5) & 0x01);

	/* D6 and D7 are not connected */
}



/*************************************
 *
 *  Space Invaders II (cocktail)
 *
 *************************************/

static const discrete_op_amp_1sht_info invad2ct_invader_hit_1sht =
{
	DISC_OP_AMP_1SHT_1 | DISC_OP_AMP_IS_NORTON,
	RES_M(4.7),     /* R49 */
	RES_K(100),     /* R51 */
	RES_M(1),       /* R48 */
	RES_M(1),       /* R50 */
	RES_M(2.2),     /* R52 */
	CAP_U(0.22),    /* C18 */
	CAP_P(470),     /* C20 */
	0,              /* vN */
	12              /* vP */
};


static const discrete_op_amp_osc_info invad2ct_invader_hit_osc =
{
	DISC_OP_AMP_OSCILLATOR_1 | DISC_OP_AMP_IS_NORTON | DISC_OP_AMP_OSCILLATOR_OUT_CAP,
	RES_M(1),       /* R37 */
	RES_K(10),      /* R41 */
	RES_K(100),     /* R38 */
	RES_K(120),     /* R40 */
	RES_M(1),       /* R39 */
	0,              /* no r6 */
	0,              /* no r7 */
	0,              /* no r8 */
	CAP_U(0.22),    /* C16 */
	12,             /* vP */
};


static const discrete_op_amp_1sht_info invad2ct_brd2_invader_hit_1sht =
{
	DISC_OP_AMP_1SHT_1 | DISC_OP_AMP_IS_NORTON,
	RES_M(4.7),     /* R49 */
	RES_K(100),     /* R51 */
	RES_M(1),       /* R48 */
	RES_M(1),       /* R50 */
	RES_M(2.2),     /* R52 */
	CAP_U(1),       /* C18 */
	CAP_P(470),     /* C20 */
	0,              /* vN */
	12              /* vP */
};


static const discrete_op_amp_osc_info invad2ct_brd2_invader_hit_osc =
{
	DISC_OP_AMP_OSCILLATOR_1 | DISC_OP_AMP_IS_NORTON | DISC_OP_AMP_OSCILLATOR_OUT_CAP,
	RES_M(1),       /* R37 */
	RES_K(10),      /* R41 */
	RES_K(100),     /* R38 */
	RES_K(120),     /* R40 */
	RES_M(1),       /* R39 */
	0,              /* no r6 */
	0,              /* no r7 */
	0,              /* no r8 */
	CAP_U(0.1),     /* C16 */
	12,             /* vP */
};


static const discrete_op_amp_1sht_info invad2ct_missle_1sht =
{
	DISC_OP_AMP_1SHT_1 | DISC_OP_AMP_IS_NORTON,
	RES_M(4.7),     /* R32 */
	RES_K(100),     /* R30 */
	RES_M(1),       /* R31 */
	RES_M(1),       /* R33 */
	RES_M(2.2),     /* R34 */
	CAP_U(0.22),    /* C12 */
	CAP_P(470),     /* C15 */
	0,              /* vN */
	12              /* vP */
};


static const discrete_mixer_desc invad2ct_mixer =
{
	DISC_MIXER_IS_OP_AMP,       /* type */
	{ RES_K(100),               /* R78 */
		RES_K(15) + 100 + 100,  /* R134 + R133 + R132 */
		RES_K(150),             /* R136 */
		RES_K(150),             /* R59 */
		RES_K(10) + RES_K(6.8) + RES_K(5.6),    /* R86 + R85 + R84 */
		RES_K(150) },               /* R28 */
	{0},                        /* no rNode{} */
	{ 0,
		0,
		0,
		0,
		0,
		CAP_U(0.001) },         /* C11 */
	0,                          /* no rI */
	RES_K(100),                 /* R105 */
	0,                          /* no cF */
	CAP_U(0.1),                 /* C45 */
	0,                          /* vRef = ground */
	1                           /* gain */
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


MACHINE_CONFIG_FRAGMENT( invad2ct_audio )
	MCFG_SPEAKER_STANDARD_STEREO("spk1", "spk2")

	MCFG_SOUND_ADD("discrete", DISCRETE, 0)
	MCFG_DISCRETE_INTF(invad2ct)
	MCFG_SOUND_ROUTE(0, "spk1", 0.5)
	MCFG_SOUND_ROUTE(1, "spk2", 0.5)

	MCFG_SOUND_ADD("sn1", SN76477, 0)
	MCFG_SN76477_NOISE_PARAMS(0, 0, 0)                  // noise + filter: N/C
	MCFG_SN76477_DECAY_RES(0)                           // decay_res: N/C
	MCFG_SN76477_ATTACK_PARAMS(0, RES_K(100))           // attack_decay_cap + attack_res
	MCFG_SN76477_AMP_RES(RES_K(56))                     // amplitude_res
	MCFG_SN76477_FEEDBACK_RES(RES_K(10))                // feedback_res
	MCFG_SN76477_VCO_PARAMS(0, CAP_U(0.1), RES_K(8.2))  // VCO volt + cap + res
	MCFG_SN76477_PITCH_VOLTAGE(5.0)                     // pitch_voltage
	MCFG_SN76477_SLF_PARAMS(CAP_U(1.0), RES_K(120))     // slf caps + res
	MCFG_SN76477_ONESHOT_PARAMS(0, 0)                   // oneshot caps + res: N/C
	MCFG_SN76477_VCO_MODE(1)                            // VCO mode
	MCFG_SN76477_MIXER_PARAMS(0, 0, 0)                  // mixer A, B, C
	MCFG_SN76477_ENVELOPE_PARAMS(1, 0)                  // envelope 1, 2
	MCFG_SN76477_ENABLE(1)                              // enable
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "spk1", 0.3)

	MCFG_SOUND_ADD("sn2", SN76477, 0)
	MCFG_SN76477_NOISE_PARAMS(0, 0, 0)                  // noise + filter: N/C
	MCFG_SN76477_DECAY_RES(0)                           // decay_res: N/C
	MCFG_SN76477_ATTACK_PARAMS(0, RES_K(100))           // attack_decay_cap + attack_res
	MCFG_SN76477_AMP_RES(RES_K(56))                     // amplitude_res
	MCFG_SN76477_FEEDBACK_RES(RES_K(10))                // feedback_res
	MCFG_SN76477_VCO_PARAMS(0, CAP_U(0.047),  RES_K(39))  // VCO volt + cap + res
	MCFG_SN76477_PITCH_VOLTAGE(5.0)                     // pitch_voltage
	MCFG_SN76477_SLF_PARAMS(CAP_U(1.0), RES_K(120))     // slf caps + res
	MCFG_SN76477_ONESHOT_PARAMS(0, 0)                   // oneshot caps + res: N/C
	MCFG_SN76477_VCO_MODE(1)                            // VCO mode
	MCFG_SN76477_MIXER_PARAMS(0, 0, 0)                  // mixer A, B, C
	MCFG_SN76477_ENVELOPE_PARAMS(1, 0)                  // envelope 1, 2
	MCFG_SN76477_ENABLE(1)                              // enable
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "spk2", 0.3)
MACHINE_CONFIG_END


WRITE8_MEMBER(mw8080bw_state::invad2ct_audio_1_w)
{
	m_sn1->enable_w((~data >> 0) & 0x01);   /* saucer sound */

	m_discrete->write(space, INVADERS_NODE(INVADERS_MISSILE_EN, 1), data & 0x02);
	m_discrete->write(space, INVADERS_NODE(INVADERS_EXPLOSION_EN, 1), data & 0x04);
	m_discrete->write(space, INVADERS_NODE(INVADERS_INVADER_HIT_EN, 1), data & 0x08);
	m_discrete->write(space, INVADERS_NODE(INVADERS_BONUS_MISSLE_BASE_EN, 1), data & 0x10);

	machine().sound().system_enable(data & 0x20);

	/* D6 and D7 are not connected */
}


WRITE8_MEMBER(mw8080bw_state::invad2ct_audio_2_w)
{
	m_discrete->write(space, INVADERS_NODE(INVADERS_FLEET_DATA, 1), data & 0x0f);
	m_discrete->write(space, INVADERS_NODE(INVADERS_SAUCER_HIT_EN, 1), data & 0x10);

	/* D5-D7 are not connected */
}


WRITE8_MEMBER(mw8080bw_state::invad2ct_audio_3_w)
{
	m_sn2->enable_w((~data >> 0) & 0x01);   /* saucer sound */

	m_discrete->write(space, INVADERS_NODE(INVADERS_MISSILE_EN, 2), data & 0x02);
	m_discrete->write(space, INVADERS_NODE(INVADERS_EXPLOSION_EN, 2), data & 0x04);
	m_discrete->write(space, INVADERS_NODE(INVADERS_INVADER_HIT_EN, 2), data & 0x08);
	m_discrete->write(space, INVADERS_NODE(INVADERS_BONUS_MISSLE_BASE_EN, 2), data & 0x10);

	/* D5-D7 are not connected */
}


WRITE8_MEMBER(mw8080bw_state::invad2ct_audio_4_w)
{
	m_discrete->write(space, INVADERS_NODE(INVADERS_FLEET_DATA, 2), data & 0x0f);
	m_discrete->write(space, INVADERS_NODE(INVADERS_SAUCER_HIT_EN, 2), data & 0x10);

	/* D5-D7 are not connected */
}
