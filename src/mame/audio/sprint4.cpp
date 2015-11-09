// license:???
// copyright-holders:Derrick Renaud
/*************************************************************************

    audio\sprint4.c

*************************************************************************/
#include "emu.h"
#include "sprint4.h"
#include "sound/discrete.h"


/************************************************************************
 * sprint4 Sound System Analog emulation
 *
 * R/C labels in the comments are from the player 1 circuits of Sprint 4.
 *
 * Mar 2007, D.R.
 ************************************************************************/

#define SPRINT4_2V  (15750.0/4)

static const discrete_lfsr_desc sprint4_lfsr =
{
	DISC_CLK_IS_FREQ,
	16,                 /* Bit Length */
	0,                  /* Reset Value */
	0,                  /* Use Bit 0 as XOR input 0 */
	14,                 /* Use Bit 14 as XOR input 1 */
	DISC_LFSR_XNOR,     /* Feedback stage1 is XNOR */
	DISC_LFSR_OR,       /* Feedback stage2 is just stage 1 output OR with external feed */
	DISC_LFSR_REPLACE,  /* Feedback stage3 replaces the shifted register contents */
	0x000001,           /* Everything is shifted into the first bit only */
	0,                  /* Output is not inverted */
	15                  /* Output bit */
};


static const discrete_dac_r1_ladder sprint4_motor_freq_DAC =
{
	4,                  /* size */
	{ RES_M(2.2),       /* R20 */
		RES_M(1),           /* R23 */
		RES_K(470),     /* R22 */
		RES_K(220) },       /* R21 */
	5.0 - 0.6,          /* 5V - CR1 junction voltage */
	RES_K(68),          /* R24 */
	0,                  /* no rGnd */
	CAP_U(2.2)          /* C28 */
};


#define SPRINT4_MOTOR_OUT_RES   (1.0 / (1.0 / RES_K(10) + 1.0 / RES_K(10) + 1.0 / RES_K(10)))
static const discrete_dac_r1_ladder sprint4_motor_out_DAC =
{
	4,                  /* size */
	{ RES_K(10),        /* R76 */
		0,              /* not connected */
		RES_K(10),      /* R77 */
		RES_K(10) },        /* R75 */
	0,                  /* no vBias */
	0,                  /* no rBias */
	0,                  /* no rGnd */
	CAP_U(0.1)          /* C47 */
};


#define SPRINT4_BANG_RES    (1.0 / (1.0 / RES_K(8.2) + 1.0 / RES_K(3.9) + 1.0 / RES_K(2.2) + 1.0 / RES_K(1)))
static const discrete_dac_r1_ladder sprint4_bang_DAC =
{
	4,                  /* size */
	{ RES_K(8.2),       /* R19 */
		RES_K(3.9),     /* R18 */
		RES_K(2.2),     /* R17 */
		RES_K(1) },     /* R16 */
	0,                  /* no vBias */
	0,                  /* no rBias */
	0,                  /* no rGnd */
	CAP_U(0.1)          /* C32 */
};


static const discrete_555_cc_desc sprint4_motor_vco =
{
	DISC_555_OUT_DC | DISC_555_OUT_SQW,
	5,                  // B+ voltage of 555
	DEFAULT_555_VALUES,
	0.7                 // Q1 junction voltage
};


/* this will make 4 slightly different screech circuits due to part tolerance */
/* otherwise they would all sound identical */
#define SPRINT4_SCREECH_CIRCUIT(_plr)                               \
static const discrete_schmitt_osc_desc sprint4_screech_osc_##_plr = \
{                                                                   \
	RES_K(1),                           /* R65 */                   \
	100,                                /* R64 */                   \
	CAP_U(10) + CAP_U((_plr-2) * .2),   /* C44 */                   \
	DEFAULT_7414_VALUES,                                            \
	DISC_SCHMITT_OSC_IN_IS_LOGIC | DISC_SCHMITT_OSC_ENAB_IS_AND     \
};
SPRINT4_SCREECH_CIRCUIT(1)
SPRINT4_SCREECH_CIRCUIT(2)
SPRINT4_SCREECH_CIRCUIT(3)
SPRINT4_SCREECH_CIRCUIT(4)


static const discrete_mixer_desc sprint4_mixer =
{
	DISC_MIXER_IS_RESISTOR,
	{ RES_K(10) + SPRINT4_MOTOR_OUT_RES,    /* R105 */
		RES_K(10) + SPRINT4_MOTOR_OUT_RES,  /* R106 */
		RES_K(10) + SPRINT4_BANG_RES,           /* R104 */
		RES_K(47),                          /* R102 */
		RES_K(47) },                            /* R103 */
	{ 0 },                                  /* no rNode{} */
	{ 0 },                                  /* no c{} */
	0,                                      /* no rI */
	RES_K(5),                               /* R109 */
	0,                                      /* no cF */
	CAP_U(0.1),                             /* C51 */
	0,                                      /* vRef = Gnd */
	1                                       /* gain */
};


/* discrete sound output nodes */
#define SPRINT4_MOTOR_SND_1         NODE_11
#define SPRINT4_MOTOR_SND_2         NODE_12
#define SPRINT4_MOTOR_SND_3         NODE_13
#define SPRINT4_MOTOR_SND_4         NODE_14
#define SPRINT4_SCREECH_SND_1       NODE_15
#define SPRINT4_SCREECH_SND_2       NODE_16
#define SPRINT4_SCREECH_SND_3       NODE_17
#define SPRINT4_SCREECH_SND_4       NODE_18
#define SPRINT4_BANG_SND            NODE_19
#define SPRINT4_NOISE               NODE_20
#define SPRINT4_FINAL_MIX_1_2       NODE_21
#define SPRINT4_FINAL_MIX_3_4       NODE_22

#define SPRINT4_ATTRACT_INV         NODE_23


/* setup the attract input and it's inverse */
#define SPRINT4_ATTRACT                                                 \
	DISCRETE_INPUT_NOT(SPRINT4_ATTRACT_EN)                              \
	DISCRETE_LOGIC_INVERT(SPRINT4_ATTRACT_INV, SPRINT4_ATTRACT_EN)


/* port tags used for the discrete adjusters */
#define SPRINT4_MOTOR_TAG_1     "MOTOR1"
#define SPRINT4_MOTOR_TAG_2     "MOTOR2"
#define SPRINT4_MOTOR_TAG_3     "MOTOR3"
#define SPRINT4_MOTOR_TAG_4     "MOTOR4"


/* used to offset the motor nodes based on the player so the nodes do not overlap */
/* _plr must be 1, 2, 3, or 4 */
/* so it uses NODES_30 to NODE_69 */
#define SPRINT4_PLAYER_MOTOR_NODE(_node, _plr)      NODE(20 + 10 * _plr + _node)


/************************************************
 * Motor sound
 *
 * complete motor effects for each player
 * _plr must be 1, 2, 3, or 4
 ************************************************/
#define SPRINT4_PLAYER_MOTOR(_plr)                                                                  \
	DISCRETE_INPUTX_DATA(SPRINT4_MOTOR_DATA_##_plr,                                                 \
					-1, 0x0f, 0)    /* latch IC D8 inverts the data */                              \
	DISCRETE_DAC_R1(SPRINT4_PLAYER_MOTOR_NODE(1, _plr),                                             \
					SPRINT4_MOTOR_DATA_##_plr,                  /* DATA */                          \
					DEFAULT_TTL_V_LOGIC_1,                      /* VDATA */                         \
					&sprint4_motor_freq_DAC)                    /* LADDER */                        \
	DISCRETE_ADJUSTMENT(SPRINT4_PLAYER_MOTOR_NODE(2, _plr),                                     \
					RES_K(10) + RES_K(250),                     /* MIN */                           \
					RES_K(10),                                  /* MAX */                           \
					DISC_LOGADJ,                                /* LOGLIN */                        \
					SPRINT4_MOTOR_TAG_##_plr)                   /* PORT TAG */                      \
	DISCRETE_555_CC(SPRINT4_PLAYER_MOTOR_NODE(3, _plr),         /* IC D8, pin 3 */                  \
					1,                                          /* RESET */                         \
					SPRINT4_PLAYER_MOTOR_NODE(1, _plr),         /* VIN */                           \
					SPRINT4_PLAYER_MOTOR_NODE(2, _plr),         /* R */                             \
					CAP_U(0.01),                                /* C34 */                           \
					RES_M(3.3),                                 /* R42 */                           \
					0,                                          /* no RGND */                       \
					0,                                          /* no RDIS */                       \
					&sprint4_motor_vco)                         /* OPTIONS */                       \
	DISCRETE_COUNTER_7492(SPRINT4_PLAYER_MOTOR_NODE(4, _plr),   /* IC D9, pins 11,9,8 */            \
					1,                                          /* ENAB */                          \
					SPRINT4_ATTRACT_EN,                         /* RESET */                         \
					SPRINT4_PLAYER_MOTOR_NODE(3, _plr),         /* CLK */                           \
					DISC_CLK_ON_F_EDGE)                                                             \
	DISCRETE_TRANSFORM3(SPRINT4_PLAYER_MOTOR_NODE(5, _plr),     /* IC B10, pin 3 */                 \
					SPRINT4_PLAYER_MOTOR_NODE(4, _plr),         /* INP0 */                          \
					0x01,                                       /* INP1 */                          \
					0x04,                                       /* INP2 */                          \
					"01&02&2/^")                                /* XOR QA and QC */                 \
	DISCRETE_COUNTER(SPRINT4_PLAYER_MOTOR_NODE(6, _plr),        /* IC D9, pin 12 */                 \
					1,                                          /* ENAB */                          \
					SPRINT4_ATTRACT_EN,                         /* RESET */                         \
					SPRINT4_PLAYER_MOTOR_NODE(5, _plr),         /* CLK */                           \
					0, 1,                                       /* MIN, MAX */                      \
					DISC_COUNT_UP,                              /* DIR */                           \
					0,                                          /* INIT0 */                         \
					DISC_CLK_ON_F_EDGE)                         /* CLKTYPE */                       \
	DISCRETE_TRANSFORM3(SPRINT4_PLAYER_MOTOR_NODE(7, _plr),     /*  */                              \
					SPRINT4_PLAYER_MOTOR_NODE(4, _plr),         /* INP0 */                          \
					SPRINT4_PLAYER_MOTOR_NODE(6, _plr),         /* INP1 */                          \
					0x08,                                       /* INP2 */                          \
					"012*+")                                    /* join 7492 bits together */       \
	DISCRETE_DAC_R1(SPRINT4_MOTOR_SND_##_plr,                                                       \
					SPRINT4_PLAYER_MOTOR_NODE(7, _plr),         /* DATA */                          \
					DEFAULT_TTL_V_LOGIC_1,                      /* VDATA */                         \
					&sprint4_motor_out_DAC)                     /* LADDER */


/************************************************
 * Bang sound
 ************************************************/
#define SPRINT4_BANG                                                    \
	DISCRETE_LFSR_NOISE(SPRINT4_NOISE,                                  \
					1,                                  /* ENAB */      \
					SPRINT4_ATTRACT_INV,                /* RESET */     \
					SPRINT4_2V,                         /* CLK */       \
					1,                                  /* AMPL */      \
					0,                                  /* FEED */      \
					1.0/2,                              /* BIAS */      \
					&sprint4_lfsr)                      /* LFSRTB */    \
	DISCRETE_INPUT_DATA(SPRINT4_BANG_DATA)                              \
	DISCRETE_ONOFF(NODE_70,                             /* IC F7 */     \
					SPRINT4_NOISE,                      /* ENAB */      \
					SPRINT4_BANG_DATA)                  /* INP0 */      \
	DISCRETE_DAC_R1(SPRINT4_BANG_SND,                                   \
					NODE_70,                            /* DATA */      \
					DEFAULT_TTL_V_LOGIC_1,              /* VDATA */     \
					&sprint4_bang_DAC)                  /* LADDER */


/************************************************
 * Screech sound
 *
 * Complete screech effects for each player.
 * This must follow the Bang sound code because
 * it uses the bang noise source.
 * _plr must be 1, 2, 3, or 4
 ************************************************/
#define SPRINT4_PLAYER_SCREECH(_plr)                                    \
	DISCRETE_INPUT_LOGIC(SPRINT4_SCREECH_EN_##_plr)                     \
	DISCRETE_SCHMITT_OSCILLATOR(SPRINT4_SCREECH_SND_##_plr,             \
					SPRINT4_SCREECH_EN_##_plr,          /* ENAB */      \
					SPRINT4_NOISE,                      /* INP0 */      \
					DEFAULT_TTL_V_LOGIC_1,              /* AMPL */      \
					&sprint4_screech_osc_##_plr)        /* TABLE */


/************************************************
 * Final mixer
 * _plr_a must be 1 and _plr_b must be 2
 * or
 * _plr_a must be 3 and _plr_b must be 4
 ************************************************/
#define SPRINT4_MIXER(_plr_a, _plr_b, _gain)                            \
	DISCRETE_MIXER5(SPRINT4_FINAL_MIX_##_plr_a##_##_plr_b,              \
					1,                                  /* ENAB */      \
					SPRINT4_MOTOR_SND_##_plr_a,         /* IN0 */       \
					SPRINT4_MOTOR_SND_##_plr_b,         /* IN1 */       \
					SPRINT4_BANG_SND,                   /* IN2 */       \
					SPRINT4_SCREECH_SND_##_plr_a,       /* IN3 */       \
					SPRINT4_SCREECH_SND_##_plr_b,       /* IN4 */       \
					&sprint4_mixer)                     /* INFO */      \
	DISCRETE_OUTPUT(SPRINT4_FINAL_MIX_##_plr_a##_##_plr_b, _gain)




DISCRETE_SOUND_START(sprint4)
	SPRINT4_ATTRACT
	SPRINT4_PLAYER_MOTOR(1)
	SPRINT4_PLAYER_MOTOR(2)
	SPRINT4_PLAYER_MOTOR(3)
	SPRINT4_PLAYER_MOTOR(4)
	SPRINT4_BANG
	SPRINT4_PLAYER_SCREECH(1)
	SPRINT4_PLAYER_SCREECH(2)
	SPRINT4_PLAYER_SCREECH(3)
	SPRINT4_PLAYER_SCREECH(4)
	SPRINT4_MIXER(1, 2, 30000)
	SPRINT4_MIXER(3, 4, 30000)
DISCRETE_SOUND_END


DISCRETE_SOUND_START(ultratnk)
	SPRINT4_ATTRACT
	SPRINT4_PLAYER_MOTOR(1)
	SPRINT4_PLAYER_MOTOR(2)
	SPRINT4_BANG
	SPRINT4_PLAYER_SCREECH(1)
	SPRINT4_PLAYER_SCREECH(2)
	SPRINT4_MIXER(1, 2, 30000)
DISCRETE_SOUND_END
