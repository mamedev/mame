// license:BSD-3-Clause
// copyright-holders:Nicola Salmoria, Tormod Tjaberg, Mirko Buffoni,Lee Taylor, Valerio Verrando, Zsolt Vasvari,Derrick Renaud
// thanks-to:Michael Strutts, Marco Cassili
/***************************************************************************

    Midway 8080-based black and white hardware

****************************************************************************/

#include "emu.h"
#include "mw8080bw_a.h"
#include "nl_gunfight.h"
#include "nl_280zzzap.h"

#include "mw8080bw.h"
#include "speaker.h"


namespace {

/*************************************
 *
 *  Implementation of the common
 *  noise circuits
 *
 *************************************/

discrete_lfsr_desc const midway_lfsr =
{
	DISC_CLK_IS_FREQ,
	17,                 // bit length
						// the RC network fed into pin 4 has the effect of presetting all bits high at power up
	0x1ffff,            // reset value
	4,                  // use bit 4 as XOR input 0
	16,                 // use bit 16 as XOR input 1
	DISC_LFSR_XOR,      // feedback stage1 is XOR
	DISC_LFSR_OR,       // feedback stage2 is just stage 1 output OR with external feed
	DISC_LFSR_REPLACE,  // feedback stage3 replaces the shifted register contents
	0x000001,           // everything is shifted into the first bit only
	0,                  // output is not inverted
	12                  // output bit
};


/*************************************
 *
 *  Shared by Space Invaders and
 *  Space Invaders II (cocktail)
 *
 *************************************/

// sound board 1 or 2, for multi-board games
#define INVADERS_NODE(_node, _board)    (NODE(_node + ((_board - 1) * 100)))

// nodes - inputs
#define INVADERS_SAUCER_HIT_EN                01
#define INVADERS_FLEET_DATA                   02
#define INVADERS_BONUS_MISSLE_BASE_EN         03
#define INVADERS_INVADER_HIT_EN               04
#define INVADERS_EXPLOSION_EN                 05
#define INVADERS_MISSILE_EN                   06

// nodes - sounds
#define INVADERS_NOISE                        NODE_10
#define INVADERS_SAUCER_HIT_SND               11
#define INVADERS_FLEET_SND                    12
#define INVADERS_BONUS_MISSLE_BASE_SND        13
#define INVADERS_INVADER_HIT_SND              14
#define INVADERS_EXPLOSION_SND                15
#define INVADERS_MISSILE_SND                  16


/************************************************
 * Noise Generator
 ************************************************/

// Noise clock was breadboarded and measured at 7515
#define INVADERS_NOISE_GENERATOR                                                \
		DISCRETE_LFSR_NOISE(INVADERS_NOISE,             /* IC N5, pin 10 */     \
				1,                                      /* ENAB */              \
				1,                                      /* no RESET */          \
				7515,                                   /* CLK in Hz */         \
				12,                                     /* p-p AMPL */          \
				0,                                      /* no FEED input */     \
				12.0/2,                                 /* dc BIAS */           \
				&midway_lfsr)


/************************************************
 * Saucer Hit
 ************************************************/

discrete_op_amp_info const invaders_saucer_hit_op_amp_B3_9 =
{
	DISC_OP_AMP_IS_NORTON,
	0,              // no r1
	RES_K(100),     // R72
	RES_M(1),       // R71
	0,              // no r4
	CAP_U(1),       // C23
	0,              // vN
	12              // vP
};


discrete_op_amp_osc_info const invaders_saucer_hit_osc =
{
	DISC_OP_AMP_OSCILLATOR_1 | DISC_OP_AMP_IS_NORTON | DISC_OP_AMP_OSCILLATOR_OUT_CAP,
	RES_M(1),       // R70
	RES_K(470),     // R64
	RES_K(100),     // R61
	RES_K(120),     // R63
	RES_M(1),       // R62
	0,              // no r6
	0,              // no r7
	0,              // no r8
	CAP_U(0.1),     // C21
	12,             // vP
};

discrete_op_amp_osc_info const invaders_saucer_hit_vco =
{
	DISC_OP_AMP_OSCILLATOR_VCO_1 | DISC_OP_AMP_IS_NORTON | DISC_OP_AMP_OSCILLATOR_OUT_CAP,
	RES_M(1),       // R65
	RES_K(470),     // R66
	RES_K(680),     // R67
	RES_M(1),       // R69
	RES_M(1),       // R68
	0,              // no r6
	0,              // no r7
	0,              // no r8
	CAP_P(470),     // C22
	12,             // vP
};

discrete_op_amp_info const invaders_saucer_hit_op_amp_B3_10 =
{
	DISC_OP_AMP_IS_NORTON,
	RES_K(680),     // R73
	RES_K(680),     // R77
	RES_M(2.7),     // R74
	RES_K(680),     // R75
	0,              // no c
	0,              // vN
	12              // vP
};

#define INVADERS_SAUCER_HIT(_board)                                                     \
		DISCRETE_INPUTX_LOGIC(INVADERS_NODE(INVADERS_SAUCER_HIT_EN, _board), 12, 0, 0)      \
		DISCRETE_OP_AMP(INVADERS_NODE(20, _board),                      /* IC B3, pin 9 */  \
				1,                                                      /* ENAB */          \
				0,                                                      /* no IN0 */        \
				INVADERS_NODE(INVADERS_SAUCER_HIT_EN, _board),          /* IN1 */           \
				&invaders_saucer_hit_op_amp_B3_9)                                   \
		DISCRETE_OP_AMP_OSCILLATOR(INVADERS_NODE(21, _board),           /* IC A4, pin 5 */  \
				1,                                                      /* ENAB */          \
				&invaders_saucer_hit_osc)                                           \
		DISCRETE_OP_AMP_VCO1(INVADERS_NODE(22, _board),                 /* IC A4, pin 9 */  \
				1,                                                      /* ENAB */          \
				INVADERS_NODE(21, _board),                              /* VMOD1 */         \
				&invaders_saucer_hit_vco)                                           \
		DISCRETE_OP_AMP(INVADERS_NODE(INVADERS_SAUCER_HIT_SND, _board), /* IC B3, pin 10 */ \
				1,                                                      /* ENAB */          \
				INVADERS_NODE(22, _board),                              /* IN0 */           \
				INVADERS_NODE(20, _board),                              /* IN1 */           \
				&invaders_saucer_hit_op_amp_B3_10)


/************************************************
 * Fleet movement
 ************************************************/

discrete_comp_adder_table const invaders_thump_resistors =
{
	DISC_COMP_P_RESISTOR,
	0,                          // no cDefault
	4,                          // length
	{ RES_K(20) + RES_K(20),    // R126 + R127
		RES_K(68),              // R128
		RES_K(82),              // R129
		RES_K(100) }            // R130
};

discrete_555_desc const invaders_thump_555 =
{
	DISC_555_OUT_ENERGY | DISC_555_OUT_DC,
	5,
	5.0 - 0.6,                  // 5V - diode drop
	DEFAULT_TTL_V_LOGIC_1       // Output of F3 7411 buffer
};

#define INVADERS_FLEET(_board)                                                              \
		DISCRETE_INPUT_DATA  (INVADERS_NODE(INVADERS_FLEET_DATA, _board))                   \
		DISCRETE_COMP_ADDER(INVADERS_NODE(30, _board),                                      \
				INVADERS_NODE(INVADERS_FLEET_DATA, _board),             /* DATA */          \
				&invaders_thump_resistors)                                                  \
		DISCRETE_555_ASTABLE(INVADERS_NODE(31, _board),                 /* IC F3, pin 6 */  \
				1,                                                      /* RESET */         \
				INVADERS_NODE(30, _board),                              /* R1 */            \
				RES_K(75),                                              /* R131 */          \
				CAP_U(0.1),                                             /* C29 */           \
				&invaders_thump_555)                                                        \
		DISCRETE_RCFILTER(INVADERS_NODE(32, _board),                                        \
				INVADERS_NODE(31, _board),                              /* IN0 */           \
				100,                                                    /* R132 */          \
				CAP_U(4.7) )                                            /* C31 */           \
		DISCRETE_RCFILTER(INVADERS_NODE(INVADERS_FLEET_SND, _board),                        \
				INVADERS_NODE(32, _board),                              /* IN0 */           \
				100 + 100,                                              /* R132 + R133 */   \
				CAP_U(10) )                                             /* C32 */


/************************************************
 * Bonus Missle Base
 ************************************************/

discrete_555_desc const invaders_bonus_555 =
{
	DISC_555_OUT_SQW | DISC_555_OUT_DC,
	5.0,                        // 5V
	DEFAULT_555_VALUES
};

#define INVADERS_BONUS_MISSLE_BASE(_board)                                                  \
		DISCRETE_INPUT_LOGIC (INVADERS_NODE(INVADERS_BONUS_MISSLE_BASE_EN, _board))         \
		DISCRETE_555_ASTABLE(INVADERS_NODE(40, _board),                 /* IC F4, pin 9 */  \
				INVADERS_NODE(INVADERS_BONUS_MISSLE_BASE_EN, _board),   /* RESET */         \
				RES_K(100),                                             /* R94 */           \
				RES_K(47),                                              /* R95 */           \
				CAP_U(1),                                               /* C34 */           \
				&invaders_bonus_555)                                                        \
		DISCRETE_SQUAREWFIX(INVADERS_NODE(41, _board),                                      \
				1,                                                      /* ENAB */          \
				480,                                                    /* FREQ */          \
				1,                                                      /* AMP */           \
				50,                                                     /* DUTY */          \
				1.0/2,                                                  /* BIAS */          \
				0)                                                      /* PHASE */         \
		DISCRETE_LOGIC_AND3(INVADERS_NODE(42, _board),                  /* IC F3, pin 12 */ \
				INVADERS_NODE(INVADERS_BONUS_MISSLE_BASE_EN, _board),    /* INP0 */         \
				INVADERS_NODE(41, _board),                              /* INP1 */          \
				INVADERS_NODE(40, _board) )                             /* INP2 */          \
		DISCRETE_GAIN(INVADERS_NODE(INVADERS_BONUS_MISSLE_BASE_SND, _board),/* adjust from logic to TTL voltage level */\
				INVADERS_NODE(42, _board),                              /* IN0 */           \
				DEFAULT_TTL_V_LOGIC_1)                                  /* GAIN */


/************************************************
 * Invader Hit
 ************************************************/

discrete_op_amp_info const invaders_invader_hit_op_amp_D3_10 =
{
	DISC_OP_AMP_IS_NORTON,
	0,                          // no r1
	RES_K(10),                  // R53
	RES_M(1),                   // R137
	0,                          // no r4
	CAP_U(0.47),                // C19
	0,                          // vN
	12                          // vP
};

discrete_op_amp_osc_info const invaders_invader_hit_vco =
{
	DISC_OP_AMP_OSCILLATOR_VCO_1 | DISC_OP_AMP_IS_NORTON | DISC_OP_AMP_OSCILLATOR_OUT_CAP,
	RES_M(1),                   // R42
	RES_K(470),                 // R43
	RES_K(680),                 // R44
	RES_M(1),                   // R46
	RES_M(1),                   // R45
	0,                          // no r6
	0,                          // no r7
	0,                          // no r8
	CAP_P(330),                 // C16
	12,                         // vP
};

discrete_op_amp_info const invaders_invader_hit_op_amp_D3_4 =
{
	DISC_OP_AMP_IS_NORTON,
	RES_K(470),                 // R55
	RES_K(680),                 // R54
	RES_M(2.7),                 // R56
	RES_K(680),                 // R57
	0,                          // no c
	0,                          // vN
	12                          // vP
};

#define INVADERS_INVADER_HIT(_board, _type)                                                 \
		DISCRETE_INPUTX_LOGIC(INVADERS_NODE(INVADERS_INVADER_HIT_EN, _board), 5, 0, 0)      \
		DISCRETE_OP_AMP_ONESHOT(INVADERS_NODE(50, _board),              /* IC D3, pin 9 */  \
				INVADERS_NODE(INVADERS_INVADER_HIT_EN, _board),         /* TRIG */          \
				&_type##_invader_hit_1sht)                                                  \
		DISCRETE_OP_AMP(INVADERS_NODE(51, _board),                      /* IC D3, pin 10 */ \
				1,                                                      /* ENAB */          \
				0,                                                      /* no IN0 */        \
				INVADERS_NODE(50, _board),                              /* IN1 */           \
				&invaders_invader_hit_op_amp_D3_10)                                         \
		DISCRETE_OP_AMP_OSCILLATOR(INVADERS_NODE(52, _board),           /* IC B4, pin 5 */  \
				1,                                                      /* ENAB */          \
				&_type##_invader_hit_osc)                                                   \
		DISCRETE_OP_AMP_VCO1(INVADERS_NODE(53, _board),                 /* IC B4, pin 4 */  \
				1,                                                      /* ENAB */          \
				INVADERS_NODE(52, _board),                              /* VMOD1 */         \
				&invaders_invader_hit_vco)                                                  \
		DISCRETE_OP_AMP(INVADERS_NODE(INVADERS_INVADER_HIT_SND, _board),/* IC D3, pin 4 */  \
				1,                                                      /* ENAB */          \
				INVADERS_NODE(53, _board),                              /* IN0 */           \
				INVADERS_NODE(51, _board),                              /* IN1 */           \
				&invaders_invader_hit_op_amp_D3_4)


/************************************************
 * Missle Sound
 ************************************************/

discrete_op_amp_1sht_info const invaders_missle_1sht =
{
	DISC_OP_AMP_1SHT_1 | DISC_OP_AMP_IS_NORTON,
	RES_M(4.7),                         // R32
	RES_K(100),                         // R30
	RES_M(1),                           // R31
	RES_M(1),                           // R33
	RES_M(2.2),                         // R34
	CAP_U(0.22),                        // C12, CAP_U(1) on Midway PCB
	CAP_P(470),                         // C15
	0,                                  // vN
	12                                  // vP
};

discrete_op_amp_info const invaders_missle_op_amp_B3 =
{
	DISC_OP_AMP_IS_NORTON,
	0,                                  // no r1
	RES_K(10),                          // R35
	RES_M(1.5),                         // R36
	0,                                  // no r4
	CAP_U(0.22),                        // C13
	0,                                  // vN
	12                                  // vP
};

discrete_op_amp_osc_info const invaders_missle_op_amp_osc =
{
	DISC_OP_AMP_OSCILLATOR_VCO_3 | DISC_OP_AMP_IS_NORTON | DISC_OP_AMP_OSCILLATOR_OUT_SQW, // DISC_OP_AMP_OSCILLATOR_VCO_3 doesn't fully represent the actual circuit on a Taito PCB, missile sound is off
	1.0 / (1.0 / RES_M(1) + 1.0 / RES_K(330)) + RES_M(1.5),     // R29||R11 + R12
	RES_M(1),                           // R16
	RES_K(560),                         // R17
	RES_M(2.2),                         // R19
	RES_M(1),                           // R16
	RES_M(4.7),                         // R14
	RES_M(3.3),                         // R13
	0,                                  // no r8
	CAP_P(330),                         // C10, CAP_P(300) on Taito PCB C58 (but it seems to different as real with the output sound)
	12,                                 // vP
};

discrete_op_amp_info const invaders_missle_op_amp_A3 =
{
	DISC_OP_AMP_IS_NORTON,
	RES_K(560),                         // R22
	RES_K(470),                         // R15
	RES_M(2.7),                         // R20
	RES_K(560),                         // R21
	0,                                  // no c
	0,                                  // vN
	12                                  // vP
};

discrete_op_amp_tvca_info const invaders_missle_tvca =
{
	RES_M(2.7),                         // R25
	RES_K(560),                         // R23
	0,                                  // no r3
	RES_K(560),                         // R26
	RES_K(1),                           //
	0,                                  // no r6
	RES_K(560),                         // R60
	0,                                  // no r8
	0,                                  // no r9
	0,                                  // no r10
	0,                                  // no r11
	CAP_U(0.1),                         // C14
	0,                                  // no c2
	0, 0,                               // no c3, c4
	5,                                  // v1
	0,                                  // no v2
	0,                                  // no v3
	12,                                 // vP
	DISC_OP_AMP_TRIGGER_FUNCTION_NONE,  // no f0
	DISC_OP_AMP_TRIGGER_FUNCTION_NONE,  // no f1
	DISC_OP_AMP_TRIGGER_FUNCTION_TRG0,  // f2
	DISC_OP_AMP_TRIGGER_FUNCTION_NONE,  // no f3
	DISC_OP_AMP_TRIGGER_FUNCTION_NONE,  // no f4
	DISC_OP_AMP_TRIGGER_FUNCTION_NONE   // no f5
};

#define INVADERS_MISSILE(_board, _type)                                                     \
		DISCRETE_INPUTX_LOGIC(INVADERS_NODE(INVADERS_MISSILE_EN, _board), 5, 0, 0)          \
		DISCRETE_OP_AMP_ONESHOT(INVADERS_NODE(70, _board),              /* IC B3, pin 4 */  \
				INVADERS_NODE(INVADERS_MISSILE_EN, _board),             /* TRIG */          \
				&_type##_missle_1sht)                                                       \
		DISCRETE_OP_AMP(INVADERS_NODE(71, _board),                      /* IC B3, pin 5 */  \
				1,                                                      /* ENAB */          \
				0,                                                      /* no IN0 */        \
				INVADERS_NODE(70, _board),                              /* IN1 */           \
				&invaders_missle_op_amp_B3)                                                 \
		/* next 2 modules simulate the D1 voltage drop */                                   \
		DISCRETE_ADDER2(INVADERS_NODE(72, _board),                                          \
				1,                                                      /* ENAB */          \
				INVADERS_NODE(71, _board),                              /* IN0 */           \
				-0.5)                                                   /* IN1 */           \
		DISCRETE_CLAMP(INVADERS_NODE(73, _board),                                           \
				INVADERS_NODE(72, _board),                              /* IN0 */           \
				0,                                                      /* MIN */           \
				12)                                                     /* MAX */           \
		DISCRETE_CRFILTER(INVADERS_NODE(74, _board),                                        \
				INVADERS_NOISE,                                         /* IN0 */           \
				RES_M(1) + RES_K(330),                                  /* R29, R11 */      \
				CAP_U(0.1) )                                            /* C57 */           \
		DISCRETE_GAIN(INVADERS_NODE(75, _board),                                            \
				INVADERS_NODE(74, _board),                              /* IN0 */           \
				RES_K(330)/(RES_M(1) + RES_K(330)))                     /* GAIN - R29 : R11 */  \
		DISCRETE_OP_AMP_VCO2(INVADERS_NODE(76, _board),                 /* IC C1, pin 4 */  \
				1,                                                      /* ENAB */          \
				INVADERS_NODE(75, _board),                              /* VMOD1 */         \
				INVADERS_NODE(73, _board),                              /* VMOD2 */         \
				&invaders_missle_op_amp_osc)                                                \
		DISCRETE_OP_AMP(INVADERS_NODE(77, _board),                      /* IC A3, pin 9 */  \
				1,                                                      /* ENAB */          \
				INVADERS_NODE(76, _board),                              /* IN0 */           \
				INVADERS_NODE(73, _board),                              /* IN1 */           \
				&invaders_missle_op_amp_A3)                                                 \
		DISCRETE_OP_AMP_TRIG_VCA(INVADERS_NODE(INVADERS_MISSILE_SND, _board),   /* IC A3, pin 10 */     \
				INVADERS_NODE(INVADERS_MISSILE_EN, _board),             /* TRG0 */          \
				0,                                                      /* no TRG1 */       \
				0,                                                      /* no TRG2 */       \
				INVADERS_NODE(77, _board),                              /* IN0 */           \
				0,                                                      /* no IN1 */        \
				&invaders_missle_tvca)


/************************************************
 * Explosion
 ************************************************/

discrete_op_amp_1sht_info const invaders_explosion_1sht =
{
	DISC_OP_AMP_1SHT_1 | DISC_OP_AMP_IS_NORTON,
	RES_M(4.7),                         // R90
	RES_K(100),                         // R88
	RES_M(1),                           // R91
	RES_M(1),                           // R89
	RES_M(2.2),                         // R92
	CAP_U(2.2),                         // C24
	CAP_P(470),                         // C25
	0,                                  // vN
	12                                  // vP
};

discrete_op_amp_tvca_info const invaders_explosion_tvca =
{
	RES_M(2.7),                         // R80
	RES_K(680),                         // R79
	0,                                  // no r3
	RES_K(680),                         // R82
	RES_K(10),                          // R93
	0,                                  // no r6
	RES_K(680),                         // R83
	0,                                  // no r8
	0,                                  // no r9
	0,                                  // no r10
	0,                                  // no r11
	CAP_U(1),                           // C26
	0,                                  // no c2
	0, 0,                               // no c3, c4
	12.0 - OP_AMP_NORTON_VBE,           // v1
	0,                                  // no v2
	0,                                  // no v3
	12,                                 // vP
	DISC_OP_AMP_TRIGGER_FUNCTION_NONE,  // no f0
	DISC_OP_AMP_TRIGGER_FUNCTION_NONE,  // no f1
	DISC_OP_AMP_TRIGGER_FUNCTION_TRG0,  // f2
	DISC_OP_AMP_TRIGGER_FUNCTION_NONE,  // no f3
	DISC_OP_AMP_TRIGGER_FUNCTION_NONE,  // no f4
	DISC_OP_AMP_TRIGGER_FUNCTION_NONE   // no f5
};

#define INVADERS_EXPLOSION(_board)                                                          \
		DISCRETE_INPUTX_LOGIC(INVADERS_NODE(INVADERS_EXPLOSION_EN, _board), 5, 0, 0)        \
		DISCRETE_OP_AMP_ONESHOT(INVADERS_NODE(60, _board),              /* IC D2, pin 10 */ \
				INVADERS_NODE(INVADERS_EXPLOSION_EN, _board),           /* TRIG */          \
				&invaders_explosion_1sht)                                                   \
		DISCRETE_OP_AMP_TRIG_VCA(INVADERS_NODE(61, _board),             /* IC D2, pin 4 */  \
				INVADERS_NODE(60, _board),                              /* TRG0 */          \
				0,                                                      /* no TRG1 */       \
				0,                                                      /* no TRG2 */       \
				INVADERS_NOISE,                                         /* IN0 */           \
				0,                                                      /* no IN1 */        \
				&invaders_explosion_tvca)                                                   \
		DISCRETE_RCFILTER(INVADERS_NODE(62, _board),                                        \
				INVADERS_NODE(61, _board),                              /* IN0 */           \
				RES_K(5.6),                                             /* R84 */           \
				CAP_U(0.1) )                                            /* C27 */           \
		DISCRETE_RCFILTER(INVADERS_NODE(INVADERS_EXPLOSION_SND, _board),                    \
				INVADERS_NODE(62, _board),                              /* IN0 */           \
				RES_K(5.6) + RES_K(6.8),                                /* R84 + R85 */     \
				CAP_U(0.1) )                                            /* C28 */


/************************************************
 * Final mix
 ************************************************/

discrete_mixer_desc const invaders_mixer =
{
	DISC_MIXER_IS_OP_AMP,               // type
	{ RES_K(200),                       // R78
		RES_K(10) + 100 + 100,          // R134 + R133 + R132
		RES_K(150),                     // R136
		RES_K(200),                     // R59
		RES_K(2) + RES_K(6.8) + RES_K(5.6), // R86 + R85 + R84
		RES_K(150) },                   // R28
	{0},                                // no rNode{}
	{ 0,
		0,
		0,
		0,
		0,
		CAP_U(0.001) },                 // C11
	0,                                  // no rI
	RES_K(100),                         // R105
	0,                                  // no cF
	CAP_U(0.1),                         // C45
	0,                                  // vRef = ground
	1                                   // gain
};

#define INVADERS_MIXER(_board, _type)                                                       \
		DISCRETE_MIXER6(INVADERS_NODE(90, _board),                                          \
				1,                                                      /* ENAB */          \
				INVADERS_NODE(INVADERS_SAUCER_HIT_SND, _board),         /* IN0 */           \
				INVADERS_NODE(INVADERS_FLEET_SND, _board),              /* IN1 */           \
				INVADERS_NODE(INVADERS_BONUS_MISSLE_BASE_SND, _board),  /* IN2 */           \
				INVADERS_NODE(INVADERS_INVADER_HIT_SND, _board),        /* IN3 */           \
				INVADERS_NODE(INVADERS_EXPLOSION_SND, _board),          /* IN4 */           \
				INVADERS_NODE(INVADERS_MISSILE_SND, _board),            /* IN5 */           \
				&_type##_mixer)                                                             \
		DISCRETE_OUTPUT(INVADERS_NODE(90, _board), 2500)

} // anonymous namespace


/*************************************
 *
 *  Device type globals
 *
 *************************************/

DEFINE_DEVICE_TYPE(SEAWOLF_AUDIO,  seawolf_audio_device,  "seawolf_audio",  "Midway Sea Wolf Audio")
DEFINE_DEVICE_TYPE(GUNFIGHT_AUDIO, gunfight_audio_device, "gunfight_audio", "Midway Gun Fight Audio")
DEFINE_DEVICE_TYPE(BOOTHILL_AUDIO, boothill_audio_device, "boothill_audio", "Midway Boot Hill Audio")
DEFINE_DEVICE_TYPE(DESERTGU_AUDIO, desertgu_audio_device, "desertgu_audio", "Midway Desert Gun Audio")
DEFINE_DEVICE_TYPE(DPLAY_AUDIO,    dplay_audio_device,    "dplay_audio",    "Midway Double Play Audio")
DEFINE_DEVICE_TYPE(GMISSILE_AUDIO, gmissile_audio_device, "gmissile_audio", "Midway Guided Missile Audio")
DEFINE_DEVICE_TYPE(M4_AUDIO,       m4_audio_device,       "m4_audio",       "Midway M-4 Audio")
DEFINE_DEVICE_TYPE(CLOWNS_AUDIO,   clowns_audio_device,   "clowns_audio",   "Midway Clowns Audio")
DEFINE_DEVICE_TYPE(SPACWALK_AUDIO, spacwalk_audio_device, "spacwalk_audio", "Midway Space Walk Audio")
DEFINE_DEVICE_TYPE(DOGPATCH_AUDIO, dogpatch_audio_device, "dogpatch_audio", "Midway Dog Patch Audio")
DEFINE_DEVICE_TYPE(SPCENCTR_AUDIO, spcenctr_audio_device, "spcenctr_audio", "Midway Space Encounters Audio")
DEFINE_DEVICE_TYPE(PHANTOM2_AUDIO, phantom2_audio_device, "phantom2_audio", "Midway Phantom 2 Audio")
DEFINE_DEVICE_TYPE(INVADERS_AUDIO, invaders_audio_device, "invaders_audio", "Taito Space Invaders Audio")
DEFINE_DEVICE_TYPE(INVAD2CT_AUDIO, invad2ct_audio_device, "invad2ct_audio", "Midway Space Invaders II Audio")
DEFINE_DEVICE_TYPE(ZZZAP_AUDIO,    zzzap_audio_device,    "zzzap_audio",    "Midway 280-ZZZAP Audio")
DEFINE_DEVICE_TYPE(LAGUNAR_AUDIO,  lagunar_audio_device,  "lagunar_audio",  "Midway Laguna Racer Audio")


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

// most common values based on clowns schematic
static discrete_op_amp_tvca_info const midway_music_tvca_info =
{
	RES_M(3.3),             // r502
	RES_K(10) + RES_K(680), // r505 + r506
	0,
	RES_K(680),             // r503
	RES_K(10),              // r500
	0,
	RES_K(680),             // r501
	0,
	0,
	0,
	0,
	CAP_U(.001),            // c500
	0,
	0, 0,
	12,                     // v1
	0,                      // v2
	0,                      // v3
	12,                     // vP
	DISC_OP_AMP_TRIGGER_FUNCTION_TRG0,
	DISC_OP_AMP_TRIGGER_FUNCTION_NONE,
	DISC_OP_AMP_TRIGGER_FUNCTION_TRG1,
	DISC_OP_AMP_TRIGGER_FUNCTION_NONE,
	DISC_OP_AMP_TRIGGER_FUNCTION_NONE,
	DISC_OP_AMP_TRIGGER_FUNCTION_NONE
};

midway_tone_generator_device_base::midway_tone_generator_device_base(machine_config const &mconfig, device_type type, char const *tag, device_t *owner, u32 clock) :
	device_t(mconfig, type, tag, owner, clock),
	m_discrete(*this, "discrete")
{
}

void midway_tone_generator_device_base::tone_generator_lo_w(u8 data)
{
	m_discrete->write(MIDWAY_TONE_EN, (data >> 0) & 0x01);

	m_discrete->write(MIDWAY_TONE_DATA_L, (data >> 1) & 0x1f);

	// D6 and D7 are not connected
}


void midway_tone_generator_device_base::tone_generator_hi_w(u8 data)
{
	m_discrete->write(MIDWAY_TONE_DATA_H, data & 0x3f);

	// D6 and D7 are not connected
}


/*************************************
 *
 *  Sea Wolf
 *
 *************************************/

seawolf_audio_device::seawolf_audio_device(machine_config const &mconfig, char const *tag, device_t *owner, u32 clock) :
	device_t(mconfig, SEAWOLF_AUDIO, tag, owner, clock),
	m_samples(*this, "samples"),
	m_prev(0U)
{
}

void seawolf_audio_device::write(u8 data)
{
	u8 const rising(data & ~m_prev);
	m_prev = data;

	if (BIT(rising, 0)) m_samples->start(0, 0); // SHIP HIT sound
	if (BIT(rising, 1)) m_samples->start(1, 1); // TORPEDO sound
	if (BIT(rising, 2)) m_samples->start(2, 2); // DIVE sound
	if (BIT(rising, 3)) m_samples->start(3, 3); // SONAR sound
	if (BIT(rising, 4)) m_samples->start(4, 4); // MINE HIT sound

	machine().bookkeeping().coin_counter_w(0, BIT(data, 5));

	// D6 and D7 are not connected
}

void seawolf_audio_device::device_add_mconfig(machine_config &config)
{
	static char const *const sample_names[] = {
			"*seawolf",
			"shiphit",
			"torpedo",
			"dive",
			"sonar",
			"minehit",
			nullptr };

	SPEAKER(config, "mono").front_center();

	SAMPLES(config, m_samples);
	m_samples->set_channels(5);
	m_samples->set_samples_names(sample_names);
	m_samples->add_route(ALL_OUTPUTS, "mono", 0.6);
}

void seawolf_audio_device::device_start()
{
	m_prev = 0U;

	save_item(NAME(m_prev));
}


/*************************************
 *
 *  Gun Fight
 *
 *************************************/

// Sound board volume potentiometers. By default, these are all set to their
// midpoint values.

static INPUT_PORTS_START(gunfight_audio)
	PORT_START("POT_1_LEFT_MASTER_VOL")
	PORT_ADJUSTER( 50, "Pot: Left Master Volume" )  NETLIST_ANALOG_PORT_CHANGED("sound_nl", "pot_left_master_vol")
	PORT_START("POT_2_RIGHT_MASTER_VOL")
	PORT_ADJUSTER( 50, "Pot: Right Master Volume" )  NETLIST_ANALOG_PORT_CHANGED("sound_nl", "pot_right_master_vol")
	PORT_START("POT_3_LEFT_SHOT_VOL")
	PORT_ADJUSTER( 50, "Pot: Left Shot Volume" )  NETLIST_ANALOG_PORT_CHANGED("sound_nl", "pot_left_shot_vol")
	PORT_START("POT_4_RIGHT_SHOT_VOL")
	PORT_ADJUSTER( 50, "Pot: Right Shot Volume" )  NETLIST_ANALOG_PORT_CHANGED("sound_nl", "pot_right_shot_vol")
	PORT_START("POT_5_LEFT_HIT_VOL")
	PORT_ADJUSTER( 50, "Pot: Left Hit Volume" )  NETLIST_ANALOG_PORT_CHANGED("sound_nl", "pot_left_hit_vol")
	PORT_START("POT_6_RIGHT_HIT_VOL")
	PORT_ADJUSTER( 50, "Pot: Right Hit Volume" )  NETLIST_ANALOG_PORT_CHANGED("sound_nl", "pot_right_hit_vol")
INPUT_PORTS_END

gunfight_audio_device::gunfight_audio_device(machine_config const &mconfig, char const *tag, device_t *owner, u32 clock) :
	device_t(mconfig, GUNFIGHT_AUDIO, tag, owner, clock),
	m_left_shot(*this, "sound_nl:left_shot"),
	m_right_shot(*this, "sound_nl:right_shot"),
	m_left_hit(*this, "sound_nl:left_hit"),
	m_right_hit(*this, "sound_nl:right_hit")
{
}

void gunfight_audio_device::write(u8 data)
{
	// D0 and D1 are just tied to 1k resistors

	machine().bookkeeping().coin_counter_w(0, BIT(data, 2));

	// the 74175 latches and inverts the top 4 bits
	switch ((~data >> 4) & 0x0f)
	{
	case 0x01: // LEFT SHOT sound (left speaker)
		m_left_shot->write_line(1);
		break;
	case 0x02: // RIGHT SHOT sound (right speaker)
		m_right_shot->write_line(1);
		break;
	case 0x03: // LEFT HIT sound (left speaker)
		m_left_hit->write_line(1);
		break;
	case 0x04: // RIGHT HIT sound (right speaker)
		m_right_hit->write_line(1);
		break;
	default:   // any other value will turn off the sound switches
		m_left_shot->write_line(0);
		m_right_shot->write_line(0);
		m_left_hit->write_line(0);
		m_right_hit->write_line(0);
		break;
	}
}

void gunfight_audio_device::device_add_mconfig(machine_config &config)
{
	SPEAKER(config, "lspeaker").front_left();
	SPEAKER(config, "rspeaker").front_right();

	netlist_mame_sound_device &nl_sound =
		NETLIST_SOUND(config, "sound_nl", 48000)
			.set_source(NETLIST_NAME(gunfight));
	nl_sound.add_route(0, "lspeaker", 0.5);
	nl_sound.add_route(1, "rspeaker", 0.5);

	NETLIST_LOGIC_INPUT(config, "sound_nl:left_shot",  "I_LEFT_SHOT.IN",  0);
	NETLIST_LOGIC_INPUT(config, "sound_nl:right_shot", "I_RIGHT_SHOT.IN",  0);
	NETLIST_LOGIC_INPUT(config, "sound_nl:left_hit",   "I_LEFT_HIT.IN",  0);
	NETLIST_LOGIC_INPUT(config, "sound_nl:right_hit",  "I_RIGHT_HIT.IN",  0);

	// With all the volume potentiometers at their default midpoint
	// settings, the highest output spikes are around +/- 3 volts, for an
	// extreme output swing of 6 volts. Gun Fight's audio power amplifiers
	// are configured with a voltage gain of 15 and have a single power
	// supply of about 22 volts, so they will definitely clip the highest
	// output peaks, but we don't model them. Instead, be cautious: scale
	// the outputs before the power amps so that the highest output spikes
	// of +/- 3 volts just reach the clipping limits for signed 16-bit
	// samples.
	NETLIST_STREAM_OUTPUT(config, "sound_nl:cout0", 0, "OUT_L").set_mult_offset(1.0 / 3.0, 0.0);
	NETLIST_STREAM_OUTPUT(config, "sound_nl:cout1", 1, "OUT_R").set_mult_offset(1.0 / 3.0, 0.0);

	// Netlist volume-potentiometer interfaces
	NETLIST_ANALOG_INPUT(config, "sound_nl:pot_left_master_vol", "R103.DIAL");
	NETLIST_ANALOG_INPUT(config, "sound_nl:pot_right_master_vol", "R203.DIAL");
	NETLIST_ANALOG_INPUT(config, "sound_nl:pot_left_shot_vol", "R123.DIAL");
	NETLIST_ANALOG_INPUT(config, "sound_nl:pot_right_shot_vol", "R223.DIAL");
	NETLIST_ANALOG_INPUT(config, "sound_nl:pot_left_hit_vol", "R110.DIAL");
	NETLIST_ANALOG_INPUT(config, "sound_nl:pot_right_hit_vol", "R210.DIAL");
}

ioport_constructor gunfight_audio_device::device_input_ports() const
{
	return INPUT_PORTS_NAME(gunfight_audio);
}

void gunfight_audio_device::device_start()
{
}


/*************************************
 *
 *  Boot Hill
 *
 *  Discrete sound emulation: Jan 2007, D.R.
 *
 *************************************/

// nodes - inputs
#define BOOTHILL_GAME_ON_EN         NODE_01
#define BOOTHILL_LEFT_SHOT_EN       NODE_02
#define BOOTHILL_RIGHT_SHOT_EN      NODE_03
#define BOOTHILL_LEFT_HIT_EN        NODE_04
#define BOOTHILL_RIGHT_HIT_EN       NODE_05

// nodes - sounds
#define BOOTHILL_NOISE              NODE_06
#define BOOTHILL_L_SHOT_SND         NODE_07
#define BOOTHILL_R_SHOT_SND         NODE_08
#define BOOTHILL_L_HIT_SND          NODE_09
#define BOOTHILL_R_HIT_SND          NODE_10

// nodes - adjusters
#define BOOTHILL_MUSIC_ADJ          NODE_11

static discrete_op_amp_tvca_info const boothill_tone_tvca_info =
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

static discrete_op_amp_tvca_info const boothill_shot_tvca_info =
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

static discrete_op_amp_tvca_info const boothill_hit_tvca_info =
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

static discrete_mixer_desc const boothill_l_mixer =
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
	7200            // final gain
};

static discrete_mixer_desc const boothill_r_mixer =
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
	7200            // final gain
};

static DISCRETE_SOUND_START(boothill_discrete)

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
	// Noise clock was breadboarded and measured at 7700Hz
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

	// Music is only added to the right channel per schematics
	// This should be verified on the real game
	DISCRETE_MIXER3(NODE_92, BOOTHILL_GAME_ON_EN, BOOTHILL_R_SHOT_SND, BOOTHILL_R_HIT_SND, MIDWAY_TONE_SND, &boothill_r_mixer)

	DISCRETE_OUTPUT(NODE_91, 1)
	DISCRETE_OUTPUT(NODE_92, 1)
DISCRETE_SOUND_END

static INPUT_PORTS_START(boothill_audio)
	PORT_START("MUSIC_ADJ")
	PORT_ADJUSTER( 35, "Music Volume" )
INPUT_PORTS_END

boothill_audio_device::boothill_audio_device(machine_config const &mconfig, char const *tag, device_t *owner, u32 clock) :
	midway_tone_generator_device_base(mconfig, BOOTHILL_AUDIO, tag, owner, clock)
{
}

void boothill_audio_device::write(u8 data)
{
	// D0 and D1 are not connected

	machine().bookkeeping().coin_counter_w(0, BIT(data, 2));

	m_discrete->write(BOOTHILL_GAME_ON_EN, BIT(data, 3));

	m_discrete->write(BOOTHILL_LEFT_SHOT_EN, BIT(data, 4));

	m_discrete->write(BOOTHILL_RIGHT_SHOT_EN, BIT(data, 5));

	m_discrete->write(BOOTHILL_LEFT_HIT_EN, BIT(data, 6));

	m_discrete->write(BOOTHILL_RIGHT_HIT_EN, BIT(data, 7));
}

void boothill_audio_device::device_add_mconfig(machine_config &config)
{
	SPEAKER(config, "lspeaker").front_left();
	SPEAKER(config, "rspeaker").front_right();

	DISCRETE(config, m_discrete, boothill_discrete);
	m_discrete->add_route(0, "lspeaker", 1.0);
	m_discrete->add_route(1, "rspeaker", 1.0);
}

ioport_constructor boothill_audio_device::device_input_ports() const
{
	return INPUT_PORTS_NAME(boothill_audio);
}

void boothill_audio_device::device_start()
{
}


/*************************************
 *
 *  Desert Gun
 *
 *  Discrete sound emulation: Jan 2007, D.R.
 *
 *************************************/

// nodes - inputs
#define DESERTGU_GAME_ON_EN                   NODE_01
#define DESERTGU_RIFLE_SHOT_EN                NODE_02
#define DESERTGU_BOTTLE_HIT_EN                NODE_03
#define DESERTGU_ROAD_RUNNER_HIT_EN           NODE_04
#define DESERTGU_CREATURE_HIT_EN              NODE_05
#define DESERTGU_ROADRUNNER_BEEP_BEEP_EN      NODE_06
#define DESERTGU_TRIGGER_CLICK_EN             NODE_07

// nodes - sounds
#define DESERTGU_NOISE                        NODE_08
#define DESERTGU_RIFLE_SHOT_SND               NODE_09
#define DESERTGU_BOTTLE_HIT_SND               NODE_10
#define DESERTGU_ROAD_RUNNER_HIT_SND          NODE_11
#define DESERTGU_CREATURE_HIT_SND             NODE_12
#define DESERTGU_ROADRUNNER_BEEP_BEEP_SND     NODE_13
#define DESERTGU_TRIGGER_CLICK_SND            DESERTGU_TRIGGER_CLICK_EN

// nodes - adjusters
#define DESERTGU_MUSIC_ADJ                    NODE_15

static discrete_op_amp_tvca_info const desertgu_rifle_shot_tvca_info =
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
	12,         // v1
	0,          // v2
	0,          // v3
	12,         // vP
	DISC_OP_AMP_TRIGGER_FUNCTION_NONE,
	DISC_OP_AMP_TRIGGER_FUNCTION_NONE,
	DISC_OP_AMP_TRIGGER_FUNCTION_TRG0,
	DISC_OP_AMP_TRIGGER_FUNCTION_NONE,
	DISC_OP_AMP_TRIGGER_FUNCTION_NONE,
	DISC_OP_AMP_TRIGGER_FUNCTION_NONE
};


static discrete_mixer_desc const desertgu_filter_mixer =
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


static discrete_op_amp_filt_info const desertgu_filter =
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
	6000        // final gain
};


static DISCRETE_SOUND_START(desertgu_discrete)

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

	// The low value of the pot is set to 75000.  A real 1M pot will never go to 0 anyways.
	// This will give the control more apparent volume range.
	// The music way overpowers the rest of the sounds anyways.
	DISCRETE_ADJUSTMENT(DESERTGU_MUSIC_ADJ, RES_M(1), 75000, DISC_LOGADJ, "MUSIC_ADJ")

	/************************************************
	 * Tone generator
	 ************************************************/
	MIDWAY_TONE_GENERATOR(midway_music_tvca_info)

	/************************************************
	 * Rifle shot sound
	 ************************************************/
	// Noise clock was breadboarded and measured at 7515Hz
	DISCRETE_LFSR_NOISE(DESERTGU_NOISE, 1, 1, 7515, 12.0, 0, 12.0/2, &midway_lfsr)

	DISCRETE_OP_AMP_TRIG_VCA(NODE_30, DESERTGU_RIFLE_SHOT_EN, 0, 0, DESERTGU_NOISE, 0, &desertgu_rifle_shot_tvca_info)
	DISCRETE_RCFILTER(NODE_31, NODE_30, RES_K(12), CAP_U(.01))
	DISCRETE_CRFILTER(DESERTGU_RIFLE_SHOT_SND, NODE_31, RES_K(12) + RES_K(68), CAP_U(.0022))

	/************************************************
	 * Bottle hit sound
	 ************************************************/
	DISCRETE_CONSTANT(DESERTGU_BOTTLE_HIT_SND, 0)  // placeholder for incomplete sound

	/************************************************
	 * Road Runner hit sound
	 ************************************************/
	DISCRETE_CONSTANT(DESERTGU_ROAD_RUNNER_HIT_SND, 0)  // placeholder for incomplete sound

	/************************************************
	 * Creature hit sound
	 ************************************************/
	DISCRETE_CONSTANT(DESERTGU_CREATURE_HIT_SND, 0)  // placeholder for incomplete sound

	/************************************************
	 * Beep-Beep sound
	 ************************************************/
	DISCRETE_CONSTANT(DESERTGU_ROADRUNNER_BEEP_BEEP_SND, 0) // placeholder for incomplete sound

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

INPUT_PORTS_START(desertgu_audio)
	PORT_START("MUSIC_ADJ")  // 3
	PORT_ADJUSTER( 60, "Music Volume" )
INPUT_PORTS_END

desertgu_audio_device::desertgu_audio_device(machine_config const &mconfig, char const *tag, device_t *owner, u32 clock) :
	midway_tone_generator_device_base(mconfig, DESERTGU_AUDIO, tag, owner, clock),
	m_ctrl_sel_out(*this),
	m_recoil(*this, "Player1_Gun_Recoil"),
	m_p2(0U)
{
}

void desertgu_audio_device::p1_w(u8 data)
{
	// D0 and D1 are not connected

	machine().bookkeeping().coin_counter_w(0, BIT(data, 2));

	m_discrete->write(DESERTGU_GAME_ON_EN, (data >> 3) & 0x01);
	m_discrete->write(DESERTGU_RIFLE_SHOT_EN, (data >> 4) & 0x01);
	m_discrete->write(DESERTGU_BOTTLE_HIT_EN, (data >> 5) & 0x01);
	m_discrete->write(DESERTGU_ROAD_RUNNER_HIT_EN, (data >> 6) & 0x01);
	m_discrete->write(DESERTGU_CREATURE_HIT_EN, (data >> 7) & 0x01);
}

void desertgu_audio_device::p2_w(u8 data)
{
	u8 const changed(data ^ m_p2);
	m_p2 = data;

	m_discrete->write(DESERTGU_ROADRUNNER_BEEP_BEEP_EN, (data >> 0) & 0x01);
	m_discrete->write(DESERTGU_TRIGGER_CLICK_EN, (data >> 1) & 0x01);

	m_recoil = BIT(data, 2);

	if (BIT(changed, 3)) m_ctrl_sel_out(BIT(data, 3));

	// D4-D7 are not connected
}

void desertgu_audio_device::device_add_mconfig(machine_config &config)
{
	SPEAKER(config, "mono").front_center();

	DISCRETE(config, m_discrete, desertgu_discrete);
	m_discrete->add_route(ALL_OUTPUTS, "mono", 0.8);
}

ioport_constructor desertgu_audio_device::device_input_ports() const
{
	return INPUT_PORTS_NAME(desertgu_audio);
}

void desertgu_audio_device::device_start()
{
	m_recoil.resolve();

	m_p2 = 0U;

	save_item(NAME(m_p2));
}


/*************************************
 *
 *  Double Play / Extra Inning
 *
 *  Discrete sound emulation: Jan 2007, D.R.
 *
 *************************************/

// nodes - inputs
#define DPLAY_GAME_ON_EN      NODE_01
#define DPLAY_TONE_ON_EN      NODE_02
#define DPLAY_SIREN_EN        NODE_03
#define DPLAY_WHISTLE_EN      NODE_04
#define DPLAY_CHEER_EN        NODE_05

// nodes - sounds
#define DPLAY_NOISE           NODE_06
#define DPLAY_TONE_SND        NODE_07
#define DPLAY_SIREN_SND       NODE_08
#define DPLAY_WHISTLE_SND     NODE_09
#define DPLAY_CHEER_SND       NODE_10

// nodes - adjusters
#define DPLAY_MUSIC_ADJ       NODE_11

static discrete_lfsr_desc const dplay_lfsr =
{
	DISC_CLK_IS_FREQ,
	17,                 // bit length
						// the RC network fed into pin 4, has the effect of presetting all bits high at power up
	0x1ffff,            // reset value
	4,                  // use bit 4 as XOR input 0
	16,                 // use bit 16 as XOR input 1
	DISC_LFSR_XOR,      // feedback stage1 is XOR
	DISC_LFSR_OR,       // feedback stage2 is just stage 1 output OR with external feed
	DISC_LFSR_REPLACE,  // feedback stage3 replaces the shifted register contents
	0x000001,           // everything is shifted into the first bit only
	0,                  // output is not inverted
	8                   // output bit
};

static discrete_integrate_info const dplay_siren_integrate_info =
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

static discrete_op_amp_osc_info const dplay_siren_osc =
{
	DISC_OP_AMP_OSCILLATOR_VCO_2 | DISC_OP_AMP_IS_NORTON | DISC_OP_AMP_OSCILLATOR_OUT_SQW,  // type
	RES_K(390),         // r1
	RES_M(5.6),         // r2
	RES_M(1),           // r3
	RES_M(1.5),         // r4
	RES_M(3.3),         // r5
	RES_K(56),          // r6
	0,                  // no r7
	0,                  // no r8
	CAP_U(0.0022),      // c
	12                  // vP
};

static discrete_integrate_info const dplay_whistle_integrate_info =
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

static discrete_op_amp_osc_info const dplay_whistle_osc =
{
	DISC_OP_AMP_OSCILLATOR_VCO_2 | DISC_OP_AMP_IS_NORTON | DISC_OP_AMP_OSCILLATOR_OUT_SQW,  // type
	RES_K(510),         // r1
	RES_M(5.6),         // r2
	RES_M(1),           // r3
	RES_M(1.5),         // r4
	RES_M(3.3),         // r5
	RES_K(300),         // r6
	0,                  // no r7
	0,                  // no r8
	CAP_P(220),         // c
	12                  // vP
};

static discrete_integrate_info const dplay_cheer_integrate_info =
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

static discrete_op_amp_filt_info const dplay_cheer_filter =
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

static discrete_mixer_desc const dplay_mixer =
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

static DISCRETE_SOUND_START(dplay_discrete)
	/************************************************
	 * Input register mapping
	 ************************************************/
	DISCRETE_INPUT_LOGIC (DPLAY_GAME_ON_EN)
	DISCRETE_INPUT_LOGIC (DPLAY_TONE_ON_EN)
	DISCRETE_INPUTX_LOGIC(DPLAY_SIREN_EN, 5, 0, 0)
	DISCRETE_INPUTX_LOGIC(DPLAY_WHISTLE_EN, 12, 0, 0)
	DISCRETE_INPUTX_LOGIC(DPLAY_CHEER_EN, 5, 0, 0)

	// The low value of the pot is set to 1000.  A real 1M pot will never go to 0 anyways.
	// This will give the control more apparent volume range.
	// The music way overpowers the rest of the sounds anyways.
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
			DPLAY_SIREN_EN,                 // TRG0
			0,                              // TRG1
			&dplay_siren_integrate_info)
	DISCRETE_OP_AMP_VCO1(DPLAY_SIREN_SND,
			1,                              // ENAB
			NODE_30,                        // VMOD1
			&dplay_siren_osc)

	/************************************************
	 * Whistle
	 ************************************************/
	DISCRETE_INTEGRATE(NODE_40,
			DPLAY_WHISTLE_EN,               // TRG0
			0,                              // TRG1
			&dplay_whistle_integrate_info)
	DISCRETE_OP_AMP_VCO1(DPLAY_WHISTLE_SND,
			1,                              // ENAB
			NODE_40,                        // VMOD1
			&dplay_whistle_osc)

	/************************************************
	 * Cheer
	 ************************************************/
	// Noise clock was breadboarded and measured at 7700Hz
	DISCRETE_LFSR_NOISE(DPLAY_NOISE, 1, 1, 7700, 12.0, 0, 12.0/2, &dplay_lfsr)

	DISCRETE_INTEGRATE(NODE_50, DPLAY_CHEER_EN, 0, &dplay_cheer_integrate_info)
	DISCRETE_SWITCH(NODE_51, 1, DPLAY_NOISE, 0, NODE_50)
	DISCRETE_OP_AMP_FILTER(DPLAY_CHEER_SND, 1, NODE_51, 0, DISC_OP_AMP_FILTER_IS_BAND_PASS_1M, &dplay_cheer_filter)

	/************************************************
	 * Combine all sound sources.
	 ************************************************/
	DISCRETE_MIXER5(NODE_91, DPLAY_GAME_ON_EN, DPLAY_TONE_SND, DPLAY_SIREN_SND, DPLAY_WHISTLE_SND, DPLAY_CHEER_SND, MIDWAY_TONE_SND, &dplay_mixer)

	DISCRETE_OUTPUT(NODE_91, 1)
DISCRETE_SOUND_END

static INPUT_PORTS_START(dplay_audio)
	PORT_START("MUSIC_ADJ")  // 3
	PORT_ADJUSTER( 60, "Music Volume" )
INPUT_PORTS_END

dplay_audio_device::dplay_audio_device(machine_config const &mconfig, char const *tag, device_t *owner, u32 clock) :
	midway_tone_generator_device_base(mconfig, DPLAY_AUDIO, tag, owner, clock)
{
}

void dplay_audio_device::write(u8 data)
{
	m_discrete->write(DPLAY_TONE_ON_EN, BIT(data, 0));

	m_discrete->write(DPLAY_CHEER_EN, BIT(data, 1));

	m_discrete->write(DPLAY_SIREN_EN, BIT(data, 2));

	m_discrete->write(DPLAY_WHISTLE_EN, BIT(data, 3));

	m_discrete->write(DPLAY_GAME_ON_EN, BIT(data, 4));

	machine().bookkeeping().coin_counter_w(0, BIT(data, 5));

	// D6 and D7 are not connected
}

void dplay_audio_device::device_add_mconfig(machine_config &config)
{
	SPEAKER(config, "mono").front_center();

	DISCRETE(config, m_discrete, dplay_discrete);
	m_discrete->add_route(ALL_OUTPUTS, "mono", 0.8);
}

ioport_constructor dplay_audio_device::device_input_ports() const
{
	return INPUT_PORTS_NAME(dplay_audio);
}

void dplay_audio_device::device_start()
{
}


/*************************************
 *
 *  Guided Missile
 *
 *************************************/

gmissile_audio_device::gmissile_audio_device(machine_config const &mconfig, char const *tag, device_t *owner, u32 clock) :
	device_t(mconfig, GMISSILE_AUDIO, tag, owner, clock),
	m_samples(*this, "samples%u", 1U),
	m_l_exp(*this, "L_EXP_LIGHT"),
	m_r_exp(*this, "R_EXP_LIGHT"),
	m_p1(0U)
{
}

void gmissile_audio_device::p1_w(u8 data)
{
	/* note that the schematics shows the left and right explosions
	   reversed (D5=R, D7=L), but the software confirms that
	   ours is right */

	u8 const rising(data & ~m_p1);
	m_p1 = data;

	// D0 and D1 are not connected

	machine().bookkeeping().coin_counter_w(0, BIT(data, 2));

	machine().sound().system_mute(!BIT(data, 3));

	if (BIT(rising, 4)) m_samples[1]->start(0, 0); // RIGHT MISSILE sound (goes to right speaker)

	m_l_exp = BIT(data, 5);
	if (BIT(rising, 5)) m_samples[0]->start(0, 1); // LEFT EXPLOSION sound (goes to left speaker)

	if (BIT(rising, 6)) m_samples[0]->start(0, 0); // LEFT MISSILE sound (goes to left speaker)

	m_r_exp = BIT(data, 7);
	if (BIT(rising, 7)) m_samples[1]->start(0, 1); // RIGHT EXPLOSION sound (goes to right speaker)
}

void gmissile_audio_device::p2_w(u8 data)
{
	// set AIRPLANE/COPTER/JET PAN(data & 0x07)

	// set TANK PAN((data >> 3) & 0x07)

	// D6 and D7 are not connected
}

void gmissile_audio_device::p3_w(u8 data)
{
	// if (data & 0x01)  enable AIRPLANE (bi-plane) sound (goes to AIRPLANE/COPTER/JET panning circuit)

	// if (data & 0x02)  enable TANK sound (goes to TANK panning circuit)

	// if (data & 0x04)  enable COPTER sound (goes to AIRPLANE/COPTER/JET panning circuit)

	// D3 and D4 are not connected

	// if (data & 0x20)  enable JET (3 fighter jets) sound (goes to AIRPLANE/COPTER/JET panning circuit)

	// D6 and D7 are not connected
}

void gmissile_audio_device::device_add_mconfig(machine_config &config)
{
	static char const *const sample_names[] = {
			"*gmissile",
			"1",        // missle
			"2",        // explosion
			nullptr };

	SPEAKER(config, "lspeaker").front_left();
	SPEAKER(config, "rspeaker").front_right();

	SAMPLES(config, m_samples[0]);
	m_samples[0]->set_channels(1);
	m_samples[0]->set_samples_names(sample_names);
	m_samples[0]->add_route(ALL_OUTPUTS, "lspeaker", 1.0);

	SAMPLES(config, m_samples[1]);
	m_samples[1]->set_channels(1);
	m_samples[1]->set_samples_names(sample_names);
	m_samples[1]->add_route(ALL_OUTPUTS, "rspeaker", 1.0);
}

void gmissile_audio_device::device_start()
{
	m_l_exp.resolve();
	m_r_exp.resolve();

	m_p1 = 0U;

	save_item(NAME(m_p1));
}


/*************************************
 *
 *  M-4
 *
 *************************************/

// Noise clock was breadboarded and measured at 3760Hz

m4_audio_device::m4_audio_device(machine_config const &mconfig, char const *tag, device_t *owner, u32 clock) :
	device_t(mconfig, M4_AUDIO, tag, owner, clock),
	m_samples(*this, "samples%u", 1U),
	m_p1(0U),
	m_p2(0U)
{
}

void m4_audio_device::p1_w(u8 data)
{
	u8 const rising(data & ~m_p1);
	m_p1 = data;

	// D0 and D1 are not connected

	machine().bookkeeping().coin_counter_w(0, BIT(data, 2));

	machine().sound().system_mute(!BIT(data, 3));

	if (BIT(rising, 4)) m_samples[0]->start(0, 0); // LEFT PLAYER SHOT sound (goes to left speaker)
	if (BIT(rising, 5)) m_samples[1]->start(0, 0); // RIGHT PLAYER SHOT sound (goes to right speaker)
	if (BIT(rising, 6)) m_samples[0]->start(1, 1); // LEFT PLAYER EXPLOSION sound via 300K res (goes to left speaker)
	if (BIT(rising, 7)) m_samples[1]->start(1, 1); // RIGHT PLAYER EXPLOSION sound via 300K res (goes to right speaker)
}


void m4_audio_device::p2_w(u8 data)
{
	u8 const rising(data & ~m_p2);
	m_p2 = data;

	if (BIT(rising, 0)) m_samples[0]->start(1, 1); // LEFT PLAYER EXPLOSION sound via 510K res (goes to left speaker)
	if (BIT(rising, 1)) m_samples[1]->start(1, 1); // RIGHT PLAYER EXPLOSION sound via 510K res (goes to right speaker)

	// if (data & 0x04)  enable LEFT TANK MOTOR sound (goes to left speaker)

	// if (data & 0x08)  enable RIGHT TANK MOTOR sound (goes to right speaker)

	// if (data & 0x10)  enable sound that is playing while the right plane is flying.  Circuit not named on schematics  (goes to left speaker)

	// if (data & 0x20)  enable sound that is playing while the left plane is flying.  Circuit not named on schematics  (goes to right speaker)

	// D6 and D7 are not connected
}

void m4_audio_device::device_add_mconfig(machine_config &config)
{
	static char const *const sample_names[] = {
			"*m4",
			"1",        // missle
			"2",        // explosion
			nullptr };

	SPEAKER(config, "lspeaker").front_left();
	SPEAKER(config, "rspeaker").front_right();

	SAMPLES(config, m_samples[0]);
	m_samples[0]->set_channels(2);
	m_samples[0]->set_samples_names(sample_names);
	m_samples[0]->add_route(ALL_OUTPUTS, "lspeaker", 1.0);

	SAMPLES(config, m_samples[1]);
	m_samples[1]->set_channels(2);
	m_samples[1]->set_samples_names(sample_names);
	m_samples[1]->add_route(ALL_OUTPUTS, "rspeaker", 1.0);
}

void m4_audio_device::device_start()
{
	m_p1 = 0U;
	m_p2 = 0U;

	save_item(NAME(m_p1));
	save_item(NAME(m_p2));
}


/*************************************
 *
 *  Clowns
 *
 *  Discrete sound emulation: Mar 2005, D.R.
 *
 *************************************/

// nodes - inputs
#define CLOWNS_POP_BOTTOM_EN        NODE_01
#define CLOWNS_POP_MIDDLE_EN        NODE_02
#define CLOWNS_POP_TOP_EN           NODE_03
#define CLOWNS_SPRINGBOARD_HIT_EN   NODE_04
#define CLOWNS_SPRINGBOARD_MISS_EN  NODE_05

// nodes - sounds
#define CLOWNS_NOISE                NODE_06
#define CLOWNS_POP_SND              NODE_07
#define CLOWNS_SB_HIT_SND           NODE_08
#define CLOWNS_SB_MISS_SND          NODE_09

// nodes - adjusters
#define CLOWNS_R507_POT             NODE_11

static discrete_op_amp_tvca_info const clowns_pop_tvca_info =
{
	RES_M(2.7),     // r304
	RES_K(680),     // r303
	0,
	RES_K(680),     // r305
	RES_K(1),       // j3
	0,
	RES_K(470),     // r300
	RES_K(1),       // j3
	RES_K(510),     // r301
	RES_K(1),       // j3
	RES_K(680),     // r302
	CAP_U(.015),    // c300
	CAP_U(.1),      // c301
	CAP_U(.082),    // c302
	0,              // no c4
	5,              // v1
	5,              // v2
	5,              // v3
	12,             // vP
	DISC_OP_AMP_TRIGGER_FUNCTION_NONE,
	DISC_OP_AMP_TRIGGER_FUNCTION_NONE,
	DISC_OP_AMP_TRIGGER_FUNCTION_TRG0,
	DISC_OP_AMP_TRIGGER_FUNCTION_NONE,
	DISC_OP_AMP_TRIGGER_FUNCTION_TRG1,
	DISC_OP_AMP_TRIGGER_FUNCTION_TRG2
};

static discrete_op_amp_osc_info const clowns_sb_hit_osc_info =
{
	DISC_OP_AMP_OSCILLATOR_1 | DISC_OP_AMP_IS_NORTON | DISC_OP_AMP_OSCILLATOR_OUT_SQW,
	RES_K(820),     // r200
	RES_K(33),      // r203
	RES_K(150),     // r201
	RES_K(240),     // r204
	RES_M(1),       // r202
	0,
	0,
	0,
	CAP_U(0.01),    // c200
	12
};

static discrete_op_amp_tvca_info const clowns_sb_hit_tvca_info =
{
	RES_M(2.7),     // r207
	RES_K(680),     // r205
	0,
	RES_K(680),     // r208
	RES_K(1),       // j3
	0,
	RES_K(680),     // r206
	0,0,0,0,
	CAP_U(1),       // c201
	0,
	0, 0,
	5,              // v1
	0,              // v2
	0,              // v3
	12,             // vP
	DISC_OP_AMP_TRIGGER_FUNCTION_NONE,
	DISC_OP_AMP_TRIGGER_FUNCTION_NONE,
	DISC_OP_AMP_TRIGGER_FUNCTION_TRG0,
	DISC_OP_AMP_TRIGGER_FUNCTION_NONE,
	DISC_OP_AMP_TRIGGER_FUNCTION_NONE,
	DISC_OP_AMP_TRIGGER_FUNCTION_NONE
};

static discrete_mixer_desc const clowns_mixer =
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

static DISCRETE_SOUND_START(clowns_discrete)

	/************************************************
	 * Input register mapping
	 ************************************************/
	DISCRETE_INPUT_LOGIC(CLOWNS_POP_BOTTOM_EN)
	DISCRETE_INPUT_LOGIC(CLOWNS_POP_MIDDLE_EN)
	DISCRETE_INPUT_LOGIC(CLOWNS_POP_TOP_EN)
	DISCRETE_INPUT_LOGIC(CLOWNS_SPRINGBOARD_HIT_EN)
	DISCRETE_INPUT_LOGIC(CLOWNS_SPRINGBOARD_MISS_EN)

	// The low value of the pot is set to 7000.  A real 1M pot will never go to 0 anyways.
	// This will give the control more apparent volume range.
	// The music way overpowers the rest of the sounds anyways.
	DISCRETE_ADJUSTMENT(CLOWNS_R507_POT, RES_M(1), 7000, DISC_LOGADJ, "R507")

	/************************************************
	 * Tone generator
	 ************************************************/
	MIDWAY_TONE_GENERATOR(midway_music_tvca_info)

	/************************************************
	 * Balloon hit sounds
	 * The LFSR is the same as boothill
	 ************************************************/
	// Noise clock was breadboarded and measured at 7700Hz
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
	// The rest of the circuit is a filter.  The frequency response was calculated with SPICE.
	DISCRETE_FILTER2(NODE_42, 1, NODE_41, 500, 1.0/.8, DISC_FILTER_LOWPASS)
	// The filter has a gain of 0.5
	DISCRETE_GAIN(CLOWNS_SB_HIT_SND, NODE_42, 0.5)

	/************************************************
	 * Springboard miss - INCOMPLETE
	 ************************************************/
	DISCRETE_CONSTANT(CLOWNS_SB_MISS_SND, 0) // Placeholder for incomplete sound

	/************************************************
	 * Combine all sound sources.
	 ************************************************/
	DISCRETE_MIXER4(NODE_91, 1, CLOWNS_SB_HIT_SND, CLOWNS_SB_MISS_SND, CLOWNS_POP_SND, MIDWAY_TONE_SND, &clowns_mixer)

	DISCRETE_OUTPUT(NODE_91, 11000)
DISCRETE_SOUND_END

static INPUT_PORTS_START(clowns_audio)
	PORT_START("R507")
	PORT_ADJUSTER( 40, "R507 - Music Volume" )
INPUT_PORTS_END

clowns_audio_device::clowns_audio_device(machine_config const &mconfig, char const *tag, device_t *owner, u32 clock) :
	midway_tone_generator_device_base(mconfig, CLOWNS_AUDIO, tag, owner, clock),
	m_samples(*this, "samples"),
	m_ctrl_sel_out(*this),
	m_p1(0U),
	m_p2(0U)
{
}

void clowns_audio_device::p1_w(u8 data)
{
	u8 const changed(data ^ m_p1);
	m_p1 = data;

	machine().bookkeeping().coin_counter_w(0, BIT(data, 0));

	if (BIT(changed, 1)) m_ctrl_sel_out(BIT(data, 1));

	// D2-D7 are not connected
}

void clowns_audio_device::p2_w(u8 data)
{
	u8 const rising(data & ~m_p2);
	m_p2 = data;

	m_discrete->write(CLOWNS_POP_BOTTOM_EN, BIT(data, 0));
	m_discrete->write(CLOWNS_POP_MIDDLE_EN, BIT(data, 1));
	m_discrete->write(CLOWNS_POP_TOP_EN, BIT(data, 2));

	machine().sound().system_mute(!BIT(data, 3));

	m_discrete->write(CLOWNS_SPRINGBOARD_HIT_EN, BIT(data, 4));

	if (BIT(rising, 5)) m_samples->start(0, 0);  // springboard miss

	// D6 and D7 are not connected
}

void clowns_audio_device::device_add_mconfig(machine_config &config)
{
	static char const *const sample_names[] = {
			"*clowns",
			"miss",
			nullptr };

	SPEAKER(config, "mono").front_center();

	SAMPLES(config, m_samples);
	m_samples->set_channels(1);
	m_samples->set_samples_names(sample_names);
	m_samples->add_route(ALL_OUTPUTS, "mono", 0.70);

	DISCRETE(config, m_discrete, clowns_discrete);
	m_discrete->add_route(ALL_OUTPUTS, "mono", 0.25);
}

ioport_constructor clowns_audio_device::device_input_ports() const
{
	return INPUT_PORTS_NAME(clowns_audio);
}

void clowns_audio_device::device_start()
{
	m_p1 = 0U;
	m_p2 = 0U;

	save_item(NAME(m_p1));
	save_item(NAME(m_p2));
}


/*************************************
 *
 *  Space Walk
 *
 *  Discrete sound emulation: Oct 2009, D.R.
 *
 *************************************/

// Discrete Sound Input Nodes
#define SPACWALK_TARGET_HIT_BOTTOM_EN         NODE_01
#define SPACWALK_TARGET_HIT_MIDDLE_EN         NODE_02
#define SPACWALK_TARGET_HIT_TOP_EN            NODE_03
#define SPACWALK_SPRINGBOARD_HIT1_EN          NODE_04
#define SPACWALK_SPRINGBOARD_HIT2_EN          NODE_05
#define SPACWALK_SPRINGBOARD_MISS_EN          NODE_06
#define SPACWALK_SPACE_SHIP_EN                NODE_07

// Discrete Sound Output Nodes
#define SPACWALK_NOISE                        NODE_10
#define SPACWALK_TARGET_HIT_SND               NODE_11
#define SPACWALK_SPRINGBOARD_HIT1_SND         NODE_12
#define SPACWALK_SPRINGBOARD_HIT2_SND         NODE_13
#define SPACWALK_SPRINGBOARD_MISS_SND         NODE_14
#define SPACWALK_SPACE_SHIP_SND               NODE_15

// Adjusters
#define SPACWALK_R507_POT                     NODE_19

// Parts List - Resistors
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
#define SPACWALK_R308       RES_K(20)       // not labeled but it's beside R307
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

// Parts List - Capacitors
#define SPACWALK_C200       CAP_U(0.0022)
#define SPACWALK_C201       CAP_U(3.3)
#define SPACWALK_C203       CAP_U(0.0033)
#define SPACWALK_C204       CAP_U(0.0033)
#define SPACWALK_C300       CAP_U(2.2)
#define SPACWALK_C301       CAP_U(2.2)
#define SPACWALK_C302       CAP_U(2.2)
#define SPACWALK_C303       CAP_U(0.0047)
#define SPACWALK_C304       CAP_U(0.0047)   // not labeled but it's beside C303
#define SPACWALK_C401       CAP_U(1)
#define SPACWALK_C402       CAP_U(0.68)
#define SPACWALK_C403       CAP_U(0.0022)
#define SPACWALK_C451       CAP_U(0.001)
#define SPACWALK_C452       CAP_U(0.001)
#define SPACWALK_C453       CAP_U(0.001)
#define SPACWALK_C602       CAP_U(1)

static discrete_op_amp_tvca_info const spacwalk_hit_tvca_info =
{
	SPACWALK_R304, SPACWALK_R303, 0, SPACWALK_R305,                     // r1, r2, r3, r4
	SPACWALK_RJ3, 0, SPACWALK_R300,                                     // r5, r6, r7
	SPACWALK_RJ3, SPACWALK_R301,                                        // r8, r9
	SPACWALK_RJ3, SPACWALK_R302,                                        // r10, r11
	SPACWALK_C300, SPACWALK_C301, SPACWALK_C302, 0,                     // c1, c2, c3, c4
	5, 5, 5, 12,                                                        // v1, v2, v3, vP
	DISC_OP_AMP_TRIGGER_FUNCTION_NONE,
	DISC_OP_AMP_TRIGGER_FUNCTION_NONE,
	DISC_OP_AMP_TRIGGER_FUNCTION_TRG0,
	DISC_OP_AMP_TRIGGER_FUNCTION_NONE,
	DISC_OP_AMP_TRIGGER_FUNCTION_TRG1,
	DISC_OP_AMP_TRIGGER_FUNCTION_TRG2
};

static discrete_op_amp_osc_info const spacwalk_sb_hit_vco =
{
	DISC_OP_AMP_OSCILLATOR_VCO_3 | DISC_OP_AMP_IS_NORTON,
	RES_K(330 + 150 + 30), SPACWALK_R203, SPACWALK_R201, SPACWALK_R204, // r1, r2, r3, r4
	SPACWALK_R202, 0, SPACWALK_R200, 0,                                 // r5, r6, r7, r8
	SPACWALK_C200, 12                                                   // c, vP
};

static discrete_op_amp_tvca_info const spacwalk_sb_hit_tvca_info =
{
	SPACWALK_R207, SPACWALK_R205, 0, SPACWALK_R208,                     // r1, r2, r3, r4
	SPACWALK_RJ3, 0, SPACWALK_R206,                                     // r5, r6, r7
	0, 0, 0, 0                  ,                                       // r8, r9, r10, r11
	SPACWALK_C201, 0, 0, 0,                                             // c1, c2, c3, c4
	5, 0, 0, 12,                                                        // v1, v2, v3, vP
	DISC_OP_AMP_TRIGGER_FUNCTION_NONE,
	DISC_OP_AMP_TRIGGER_FUNCTION_NONE,
	DISC_OP_AMP_TRIGGER_FUNCTION_TRG0,
	DISC_OP_AMP_TRIGGER_FUNCTION_NONE,
	DISC_OP_AMP_TRIGGER_FUNCTION_NONE,
	DISC_OP_AMP_TRIGGER_FUNCTION_NONE
};

static discrete_integrate_info const spacwalk_sb_miss_integrate =
{
	DISC_INTEGRATE_OP_AMP_1 | DISC_OP_AMP_IS_NORTON,
	SPACWALK_R406, SPACWALK_R401, 0, SPACWALK_C402,                     // r1, r2, r3, c
	12, 12,                                                             // v1, vP
	0, 0, 0                                                             // f0, f1, f2
};

static discrete_op_amp_osc_info const spacwalk_sb_miss_vco =
{
	DISC_OP_AMP_OSCILLATOR_VCO_3 | DISC_OP_AMP_IS_NORTON | DISC_OP_AMP_OSCILLATOR_OUT_SQW,
	SPACWALK_R407, SPACWALK_R412, SPACWALK_R410, SPACWALK_R411, SPACWALK_R413, 0, 0, 0, // r1, r2, r3, r4, r5, r6, r7, r8
	SPACWALK_C403, 12                                                   // c, vP
};

static discrete_op_amp_filt_info const spacwalk_sb_miss_filter =
{
	// we use r1, not r2 because vref is taken into acount by the CRFILTER
	SPACWALK_R417, 0, SPACWALK_R414, 0, SPACWALK_R416,                  // r1, r2, r3, r4, rF
	SPACWALK_C451, SPACWALK_C452, 0,                                    // c1, c2, c3
	0, 12, 0                                                            // vRef, vP, vN
};

static discrete_op_amp_info const spacwalk_sb_miss_amp =
{
	DISC_OP_AMP_IS_NORTON,
	SPACWALK_R418, SPACWALK_R404, 0, SPACWALK_R403, /* r1, r2, r3, r4 */
	0,  /* c */
	0, 12,  /* vN, vP */
};

static discrete_op_amp_osc_info const spacwalk_spaceship_osc =
{
	DISC_OP_AMP_OSCILLATOR_2 | DISC_OP_AMP_IS_NORTON | DISC_OP_AMP_OSCILLATOR_OUT_CAP,
	RES_K(75), RES_M(1), RES_M(6.8), RES_M(2.4), 0, 0, 0, 0,            // r1, r2, r3, r4, r5, r6, r7, r8
	CAP_U(2.2), 12                                                      // c, vP
};

static discrete_op_amp_osc_info const spacwalk_spaceship_vco =
{
	DISC_OP_AMP_OSCILLATOR_VCO_1 | DISC_OP_AMP_IS_NORTON | DISC_OP_AMP_OSCILLATOR_OUT_CAP,
	RES_K(680), RES_K(300), RES_K(100), RES_K(150), RES_K(120), 0, 0, 0,    // r1, r2, r3, r4, r5, r6, r7, r8
	CAP_U(0.0012), 12                                                   // c, vP
};

static discrete_mixer_desc const spacwalk_mixer =
{
	DISC_MIXER_IS_RESISTOR,
	{SPACWALK_R422, SPACWALK_R422, RES_K(39 + 10 + 1), SPACWALK_R421, SPACWALK_R420, SPACWALK_R419},
	{0, 0, 0, 0, 0, SPACWALK_R507_POT},                                 // r_nodes
	{0}, 0, 0, 0, SPACWALK_C602, 0, 1                                   // c, rI, rF, cF, cAmp, vRef, gain
};


/************************************************
 * Springboard Hit Circuit   1 or 2
 ************************************************/
#define SPACWALK_SPRINGBOARD_HIT_CIRCUIT(_num)                                                          \
		DISCRETE_RCFILTER(NODE_RELATIVE(NODE_29, _num),                                                 \
				SPACWALK_NOISE,                                                 /* IN0 */               \
				RES_K(330), CAP_U(.1))                                                                  \
		DISCRETE_RCFILTER(NODE_RELATIVE(NODE_31, _num),                                                 \
				NODE_RELATIVE(NODE_29, _num),                                   /* IN0 */               \
				RES_K(330) + RES_K(150), CAP_U(.1))                                                     \
		DISCRETE_OP_AMP_VCO1(NODE_RELATIVE(NODE_33, _num),                      /* IC M2-3, pin 5 */    \
				1,                                                              /* ENAB */              \
				NODE_RELATIVE(NODE_31, _num),                                   /* VMOD1 */             \
				&spacwalk_sb_hit_vco)                                                                   \
		DISCRETE_OP_AMP_TRIG_VCA(NODE_RELATIVE(NODE_35, _num),                  /* IC M2-3, pin 9 */    \
				NODE_RELATIVE(SPACWALK_SPRINGBOARD_HIT1_EN, _num - 1), 0, 0,    /* TRG0, TRG1, TRG2 */  \
				NODE_RELATIVE(NODE_33, _num), 0,                                /* IN0, IN1 */          \
				&spacwalk_sb_hit_tvca_info)                                                             \
		/* Wrong values.  Untested */                                                                   \
		/* The rest of the circuit is a filter. */                                                      \
		DISCRETE_FILTER2(NODE_RELATIVE(NODE_37, _num),                                                  \
				1,                                                              /* ENAB */              \
				NODE_RELATIVE(NODE_35, _num),                                   /* INP0 */              \
				2000.0 - _num * 500, 1.0/.8,                                    /* FREQ, DAMP */        \
				DISC_FILTER_LOWPASS)                                                                    \
		/* The filter has a gain of 0.5 */                                                              \
		DISCRETE_GAIN(NODE_RELATIVE(SPACWALK_SPRINGBOARD_HIT1_SND, _num - 1),                           \
				NODE_RELATIVE(NODE_37, _num), 0.5)

static DISCRETE_SOUND_START(spacwalk_discrete)

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

	// The low value of the pot is set to 7000.  A real 1M pot will never go to 0 anyways.
	// This will give the control more apparent volume range.
	// The music way overpowers the rest of the sounds anyways.
	DISCRETE_ADJUSTMENT(SPACWALK_R507_POT, SPACWALK_R507, 7000, DISC_LOGADJ, "R507")

	/************************************************
	 * Tone generator
	 ************************************************/
	MIDWAY_TONE_GENERATOR(midway_music_tvca_info)

	/************************************************
	 * Target hit sounds
	 * The LFSR is the same as boothill
	 ************************************************/
	// Noise clock was breadboarded and measured at 7700Hz
	DISCRETE_LFSR_NOISE(SPACWALK_NOISE,             // IC L4, pin 10
			1, 1,                                   // ENAB, RESET
			7700, 12.0, 0, 12.0/2, &midway_lfsr)    // CLK,AMPL,FEED,BIAS,LFSRTB

	DISCRETE_OP_AMP_TRIG_VCA(NODE_20,               // IC K3, pin 9
			SPACWALK_TARGET_HIT_TOP_EN, SPACWALK_TARGET_HIT_MIDDLE_EN, SPACWALK_TARGET_HIT_BOTTOM_EN,
			SPACWALK_NOISE, 0,                      // IN0, IN1
			&spacwalk_hit_tvca_info)
	DISCRETE_RCFILTER(NODE_21,
			NODE_20,                                // IN0
			SPACWALK_R307, SPACWALK_C303)
	DISCRETE_RCFILTER(SPACWALK_TARGET_HIT_SND,
			NODE_21,                                // IN0
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
	DISCRETE_RCDISC2(NODE_50,                       // voltage on C401
			SPACWALK_SPRINGBOARD_MISS_EN,           // SWITCH
			OP_AMP_NORTON_VBE, RES_2_PARALLEL(SPACWALK_R401, SPACWALK_R407),    // INP0,RVAL0
			12.0 - .5, SPACWALK_R400,               // INP1,RVAL1
			SPACWALK_C401)
	DISCRETE_INTEGRATE(NODE_51,                     // IC K4, pin 9
			NODE_50, 0,                             // TRG0,TRG1
			&spacwalk_sb_miss_integrate)
	DISCRETE_OP_AMP_VCO1(NODE_52,                   // IC K4, pin 5
			1,                                      // ENAB
			NODE_50,                                // VMOD1
			&spacwalk_sb_miss_vco)
	DISCRETE_CRFILTER(NODE_53,
			NODE_52,                                // IN0
			SPACWALK_R417, SPACWALK_C453)
	// this filter type probably does not work right. I need to test it.
	DISCRETE_OP_AMP_FILTER(NODE_54,                 // IC K3, pin 5
			1,                                      // ENAB
			NODE_53, 0,                             // INP0,INP1
			DISC_OP_AMP_FILTER_IS_BAND_PASS_1M | DISC_OP_AMP_IS_NORTON, &spacwalk_sb_miss_filter)
	DISCRETE_OP_AMP(SPACWALK_SPRINGBOARD_MISS_SND,  // IC K4, pin 10
			1,                                      // ENAB
			NODE_54, NODE_51,                       // IN0,IN1
			&spacwalk_sb_miss_amp)

	/************************************************
	 * Space ship sound
	 ************************************************/
	DISCRETE_OP_AMP_OSCILLATOR(NODE_60,             // voltage on 2.2uF cap near IC JK-2
			1,                                      // ENAB
			&spacwalk_spaceship_osc)
	DISCRETE_OP_AMP_VCO1(NODE_61,                   // IC JK-2, pin 5
			SPACWALK_SPACE_SHIP_EN,                 // ENAB
			NODE_60,                                // VMOD1
			&spacwalk_spaceship_vco)
	DISCRETE_RCFILTER(NODE_62,
			NODE_61,                                // IN0
			RES_K(1), CAP_U(0.15))
	DISCRETE_RCFILTER(SPACWALK_SPACE_SHIP_SND,
			NODE_62,                                // IN0
			RES_K(1) + RES_K(10), CAP_U(0.015))

	/************************************************
	 * Combine all sound sources.
	 ************************************************/
	DISCRETE_MIXER6(NODE_90,
			1,                                      // ENAB
			SPACWALK_SPRINGBOARD_HIT1_SND,
			SPACWALK_SPRINGBOARD_HIT2_SND,
			SPACWALK_SPACE_SHIP_SND,
			SPACWALK_SPRINGBOARD_MISS_SND,
			SPACWALK_TARGET_HIT_SND,
			MIDWAY_TONE_SND,
			&spacwalk_mixer)
	DISCRETE_OUTPUT(NODE_90, 11000)
DISCRETE_SOUND_END

static INPUT_PORTS_START(spacwalk_audio)
	PORT_START("R507")
	PORT_ADJUSTER( 40, "R507 - Music Volume" )
INPUT_PORTS_END

spacwalk_audio_device::spacwalk_audio_device(machine_config const &mconfig, char const *tag, device_t *owner, u32 clock) :
	midway_tone_generator_device_base(mconfig, SPACWALK_AUDIO, tag, owner, clock),
	m_ctrl_sel_out(*this),
	m_p1(0U)
{
}

void spacwalk_audio_device::p1_w(u8 data)
{
	u8 const changed(data ^ m_p1);
	m_p1 = data;

	machine().bookkeeping().coin_counter_w(0, BIT(data, 0));

	if (BIT(changed, 1)) m_ctrl_sel_out(BIT(data, 1));

	machine().sound().system_mute(!BIT(data, 2));

	m_discrete->write(SPACWALK_SPACE_SHIP_EN, (data >> 3) & 0x01);
}

void spacwalk_audio_device::p2_w(u8 data)
{
	m_discrete->write(SPACWALK_TARGET_HIT_BOTTOM_EN, (data >> 0) & 0x01);
	m_discrete->write(SPACWALK_TARGET_HIT_MIDDLE_EN, (data >> 1) & 0x01);
	m_discrete->write(SPACWALK_TARGET_HIT_TOP_EN, (data >> 2) & 0x01);
	m_discrete->write(SPACWALK_SPRINGBOARD_HIT1_EN, (data >> 3) & 0x01);
	m_discrete->write(SPACWALK_SPRINGBOARD_HIT2_EN, (data >> 4) & 0x01);
	m_discrete->write(SPACWALK_SPRINGBOARD_MISS_EN, (data >> 5) & 0x01);
}

void spacwalk_audio_device::device_add_mconfig(machine_config &config)
{
	SPEAKER(config, "mono").front_center();

	DISCRETE(config, m_discrete, spacwalk_discrete);
	m_discrete->add_route(ALL_OUTPUTS, "mono", 1.0);
}

ioport_constructor spacwalk_audio_device::device_input_ports() const
{
	return INPUT_PORTS_NAME(spacwalk_audio);
}

void spacwalk_audio_device::device_start()
{
	m_p1 = 0U;

	save_item(NAME(m_p1));
}


/*************************************
 *
 *  Dog Patch
 *
 *  Discrete sound emulation:
 *   Sept 2011, D.R.
 *
 *************************************/

// nodes - inputs
#define DOGPATCH_GAME_ON_EN         NODE_01
#define DOGPATCH_LEFT_SHOT_EN       NODE_02
#define DOGPATCH_RIGHT_SHOT_EN      NODE_03
#define DOGPATCH_HIT_EN             NODE_04
#define DOGPATCH_PAN_DATA           NODE_05

// nodes - sounds
#define DOGPATCH_NOISE              NODE_06
#define DOGPATCH_L_SHOT_SND         NODE_07
#define DOGPATCH_R_SHOT_SND         NODE_08
#define DOGPATCH_HIT_SND            NODE_09
#define DOGPATCH_L_HIT_SND          NODE_10
#define DOGPATCH_R_HIT_SND          NODE_11

static discrete_op_amp_tvca_info const dogpatch_shot_tvca_info =
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

static discrete_mixer_desc const dogpatch_l_mixer =
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
	1               // final gain
};

static discrete_mixer_desc const dogpatch_r_mixer =
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
	1               // final gain
};

static DISCRETE_SOUND_START(dogpatch_discrete)
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

	// Noise clock was breadboarded and measured at 7700Hz
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

	// Music is only added to the right channel per schematics
	// This should be verified on the real game
	DISCRETE_MIXER3(NODE_92, DOGPATCH_GAME_ON_EN, DOGPATCH_R_SHOT_SND, DOGPATCH_R_HIT_SND, MIDWAY_TONE_SND, &dogpatch_r_mixer)

	DISCRETE_OUTPUT(NODE_91, 32760.0 / 5.8)
	DISCRETE_OUTPUT(NODE_92, 32760.0 / 5.8)
DISCRETE_SOUND_END

dogpatch_audio_device::dogpatch_audio_device(machine_config const &mconfig, char const *tag, device_t *owner, u32 clock) :
	midway_tone_generator_device_base(mconfig, DOGPATCH_AUDIO, tag, owner, clock)
{
}

void dogpatch_audio_device::write(u8 data)
{
	// D0, D1 and D7 are not used

	machine().bookkeeping().coin_counter_w(0, BIT(data, 2));

	machine().sound().system_mute(!BIT(data, 3));
	m_discrete->write(DOGPATCH_GAME_ON_EN, BIT(data, 3));

	m_discrete->write(DOGPATCH_LEFT_SHOT_EN, BIT(data, 4));

	m_discrete->write(DOGPATCH_RIGHT_SHOT_EN, BIT(data, 5));

	m_discrete->write(DOGPATCH_HIT_EN, BIT(data, 6));
}

void dogpatch_audio_device::device_add_mconfig(machine_config &config)
{
	SPEAKER(config, "lspeaker").front_left();
	SPEAKER(config, "rspeaker").front_right();

	DISCRETE(config, m_discrete, dogpatch_discrete);
	m_discrete->add_route(0, "lspeaker", 1.0);
	m_discrete->add_route(1, "rspeaker", 1.0);
}

void dogpatch_audio_device::device_start()
{
}


/*************************************
 *
 *  Space Encounters
 *
 *  Discrete sound emulation:
 *  Apr 2007, D.R.
 *************************************/

// nodes - inputs
#define SPCENCTR_ENEMY_SHIP_SHOT_EN       NODE_01
#define SPCENCTR_PLAYER_SHOT_EN           NODE_02
#define SPCENCTR_SCREECH_EN               NODE_03
#define SPCENCTR_CRASH_EN                 NODE_04
#define SPCENCTR_EXPLOSION_EN             NODE_05
#define SPCENCTR_BONUS_EN                 NODE_06
#define SPCENCTR_WIND_DATA                NODE_07

// nodes - sounds
#define SPCENCTR_NOISE                    NODE_10
#define SPCENCTR_ENEMY_SHIP_SHOT_SND      NODE_11
#define SPCENCTR_PLAYER_SHOT_SND          NODE_12
#define SPCENCTR_SCREECH_SND              NODE_13
#define SPCENCTR_CRASH_SND                NODE_14
#define SPCENCTR_EXPLOSION_SND            NODE_15
#define SPCENCTR_BONUS_SND                NODE_16
#define SPCENCTR_WIND_SND                 NODE_17

static discrete_op_amp_info const spcenctr_enemy_ship_shot_op_amp_E1 =
{
	DISC_OP_AMP_IS_NORTON,
	0,                      // no r1
	RES_K(510),             // R100
	RES_M(2.2),             // R101
	RES_M(2.2),             // R102
	CAP_U(0.1),             // C100
	0,                      // vN
	12                      // vP
};

static discrete_op_amp_osc_info const spcenctr_enemy_ship_shot_op_amp_osc =
{
	DISC_OP_AMP_OSCILLATOR_VCO_3 | DISC_OP_AMP_IS_NORTON | DISC_OP_AMP_OSCILLATOR_OUT_SQW,
	RES_K(560),             // R103
	RES_K(7.5),             // R118
	RES_K(22),              // R104
	RES_K(47),              // R106
	RES_K(100),             // R105
	0,                      // no r6
	0,                      // no r7
	0,                      // no r8
	CAP_U(0.0022),          // C101
	12,                     // vP
};


static discrete_op_amp_info const spcenctr_enemy_ship_shot_op_amp_D1 =
{
	DISC_OP_AMP_IS_NORTON,
	RES_K(100),             // R107
	RES_K(100),             // R109
	RES_M(2.7),             // R108
	RES_K(100),             // R110
	0,                      // no c
	0,                      // vN
	12                      // vP
};

static discrete_op_amp_filt_info const spcenctr_enemy_ship_shot_filt =
{
	RES_K(100),             // R112
	RES_K(10),              // R113
	RES_M(4.3),             // r3
	0,                      // no r4
	RES_M(2.2),             // R114
	CAP_U(0.001),           // c1
	CAP_U(0.001),           // c2
	0,                      // no c3
	0,                      // vRef
	12,                     // vP
	0                       // vN
};

static discrete_op_amp_1sht_info const spcenctr_player_shot_1sht =
{
	DISC_OP_AMP_1SHT_1 | DISC_OP_AMP_IS_NORTON,
	RES_M(4.7),             // R500
	RES_K(100),             // R502
	RES_M(1),               // R501
	RES_M(1),               // R503
	RES_M(2.2),             // R504
	CAP_U(1),               // C500
	CAP_P(470),             // C501
	0,                      // vN
	12                      // vP
};

static discrete_op_amp_info const spcenctr_player_shot_op_amp_E1 =
{
	DISC_OP_AMP_IS_NORTON,
	0,                      // no r1
	RES_K(10),              // R505
	RES_M(1.5),             // R506
	0,                      // no r4
	CAP_U(0.22),            // C502
	0,                      // vN
	12                      // vP
};

static discrete_op_amp_osc_info const spcenctr_player_shot_op_amp_osc =
{
	DISC_OP_AMP_OSCILLATOR_VCO_3 | DISC_OP_AMP_IS_NORTON | DISC_OP_AMP_OSCILLATOR_OUT_SQW,
	1.0 / (1.0 / RES_M(1) + 1.0 / RES_K(330)) + RES_M(1.5),     // R507||R509 + R508
	RES_M(1),               // R513
	RES_K(560),             // R512
	RES_M(2.7),             // R516
	RES_M(1),               // R515
	RES_M(4.7),             // R510
	RES_M(3.3),             // R511
	0,                      // no r8
	CAP_P(330),             // C504
	12,                     // vP
};

static discrete_op_amp_info const spcenctr_player_shot_op_amp_C1 =
{
	DISC_OP_AMP_IS_NORTON,
	RES_K(560),             // R517
	RES_K(470),             // R514
	RES_M(2.7),             // R518
	RES_K(560),             // R524
	0,                      // no c
	0,                      // vN
	12                      // vP
};

static discrete_op_amp_tvca_info const spcenctr_player_shot_tvca =
{
	RES_M(2.7),                         // R522
	RES_K(560),                         // R521
	0,                                  // no r3
	RES_K(560),                         // R560
	RES_K(1),                           // R42
	0,                                  // no r6
	RES_K(560),                         // R523
	0,                                  // no r8
	0,                                  // no r9
	0,                                  // no r10
	0,                                  // no r11
	CAP_U(1),                           // C506
	0,                                  // no c2
	0, 0,                               // no c3, c4
	12,                                 // v1
	0,                                  // no v2
	0,                                  // no v3
	12,                                 // vP
	DISC_OP_AMP_TRIGGER_FUNCTION_NONE,  // no f0
	DISC_OP_AMP_TRIGGER_FUNCTION_NONE,  // no f1
	DISC_OP_AMP_TRIGGER_FUNCTION_TRG0,  // f2
	DISC_OP_AMP_TRIGGER_FUNCTION_NONE,  // no f3
	DISC_OP_AMP_TRIGGER_FUNCTION_NONE,  // no f4
	DISC_OP_AMP_TRIGGER_FUNCTION_NONE   // no f5
};

static discrete_op_amp_tvca_info const spcenctr_crash_tvca =
{
	RES_M(2.7),                         // R302
	RES_K(470),                         // R300
	0,                                  // no r3
	RES_K(470),                         // R303
	RES_K(1),                           // R56
	0,                                  // no r6
	RES_K(470),                         // R301
	0,                                  // no r8
	0,                                  // no r9
	0,                                  // no r10
	0,                                  // no r11
	CAP_U(2.2),                         // C304
	0,                                  // no c2
	0, 0,                               // no c3, c4
	5,                                  // v1
	0,                                  // no v2
	0,                                  // no v3
	12,                                 // vP
	DISC_OP_AMP_TRIGGER_FUNCTION_NONE,  // no f0
	DISC_OP_AMP_TRIGGER_FUNCTION_NONE,  // no f1
	DISC_OP_AMP_TRIGGER_FUNCTION_TRG0,  // f2
	DISC_OP_AMP_TRIGGER_FUNCTION_NONE,  // no f3
	DISC_OP_AMP_TRIGGER_FUNCTION_NONE,  // no f4
	DISC_OP_AMP_TRIGGER_FUNCTION_NONE   // no f5
};

static discrete_op_amp_tvca_info const spcenctr_explosion_tvca =
{
	RES_M(2.7),                         // R402
	RES_K(680),                         // R400
	0,                                  // no r3
	RES_K(680),                         // R403
	RES_K(1),                           // R41
	0,                                  // no r6
	RES_K(680),                         // R401
	0,                                  // no r8
	0,                                  // no r9
	0,                                  // no r10
	0,                                  // no r11
	CAP_U(2.2),                         // C400
	0,                                  // no c2
	0, 0,                               // no c3, c4
	12,                                 // v1
	0,                                  // no v2
	0,                                  // no v3
	12,                                 // vP
	DISC_OP_AMP_TRIGGER_FUNCTION_NONE,  // no f0
	DISC_OP_AMP_TRIGGER_FUNCTION_NONE,  // no f1
	DISC_OP_AMP_TRIGGER_FUNCTION_TRG0,  // f2
	DISC_OP_AMP_TRIGGER_FUNCTION_NONE,  // no f3
	DISC_OP_AMP_TRIGGER_FUNCTION_NONE,  // no f4
	DISC_OP_AMP_TRIGGER_FUNCTION_NONE   // no f5
};

static const discrete_555_desc spcenctr_555_bonus =
{
	DISC_555_OUT_SQW | DISC_555_OUT_DC,
	5,                      // B+ voltage of 555
	DEFAULT_555_VALUES
};

static const discrete_mixer_desc spcenctr_mixer =
{
	DISC_MIXER_IS_RESISTOR, // type
	{ RES_K(15),            // R117
		RES_K(15),          // R526
		RES_K(22),          // R211
		RES_K(3.6),         // R309
		RES_K(1.8) +  RES_K(3.6) + RES_K(4.7),  // R405 + R406 + R407
		RES_K(27),          // R715
		RES_K(27)},         // R51
	{0},                    // no rNode{}
	{ 0,
		CAP_U(0.001),       // C505
		CAP_U(0.1),         // C202
		CAP_U(1),           // C303
		0,
		0,
		CAP_U(10)},         // C16
	0,                      // no rI
	0,                      // no rF
	0,                      // no cF
	CAP_U(1),               // C900
	0,                      // vRef = ground
	1                       // gain
};

static DISCRETE_SOUND_START(spcenctr_discrete)

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
	// Noise clock was breadboarded and measured at 7515
	DISCRETE_LFSR_NOISE(SPCENCTR_NOISE,                 // IC A0, pin 10
			1,                                          // ENAB
			1,                                          // no RESET
			7515,                                       // CLK in Hz
			12,                                         // p-p AMPL
			0,                                          // no FEED input
			12.0/2,                                     // dc BIAS
			&midway_lfsr)

	/************************************************
	 * Enemy Ship Shot
	 ************************************************/
	DISCRETE_OP_AMP(NODE_20,                            // IC E1, pin 10
			1,                                          // ENAB
			0,                                          // no IN0
			SPCENCTR_ENEMY_SHIP_SHOT_EN,                // IN1
			&spcenctr_enemy_ship_shot_op_amp_E1)
	DISCRETE_OP_AMP_VCO1(NODE_21,                       // IC D1, pin 5
			1,                                          // ENAB
			NODE_20,                                    // VMOD1
			&spcenctr_enemy_ship_shot_op_amp_osc)
	DISCRETE_OP_AMP(NODE_22,                            // IC D1, pin 9
			1,                                          // ENAB
			NODE_21,                                    // IN0
			NODE_20,                                    // IN1
			&spcenctr_enemy_ship_shot_op_amp_D1)
	DISCRETE_OP_AMP_FILTER(NODE_23,                     // IC D1, pin 10
			1,                                          // ENAB
			NODE_22,                                    // INP0
			0,                                          // no INP1
			DISC_OP_AMP_FILTER_IS_BAND_PASS_1M | DISC_OP_AMP_IS_NORTON,
			&spcenctr_enemy_ship_shot_filt)
	DISCRETE_CRFILTER(SPCENCTR_ENEMY_SHIP_SHOT_SND,
			NODE_23,                                    // IN0
			RES_K(1.8),                                 // R116
			CAP_U(0.1) )                                // C104

	/************************************************
	 * Player Shot
	 ************************************************/
	DISCRETE_OP_AMP_ONESHOT(NODE_30,                    // IC E1, pin 4
			SPCENCTR_PLAYER_SHOT_EN,                    // TRIG
			&spcenctr_player_shot_1sht)                 // breadboarded and scoped at 325mS
	DISCRETE_OP_AMP(NODE_31,                            // IC E1, pin 5
			1,                                          // ENAB
			0,                                          // no IN0
			NODE_30,                                    // IN1
			&spcenctr_player_shot_op_amp_E1)
	// next 2 modules simulate the D502 voltage drop
	DISCRETE_ADDER2(NODE_32,
			1,                                          // ENAB
			NODE_31,                                    // IN0
			-0.5)                                       // IN1
	DISCRETE_CLAMP(NODE_33,
			NODE_32,                                    // IN0
			0,                                          // MIN
			12)                                         // MAX
	DISCRETE_CRFILTER(NODE_34,
			SPCENCTR_NOISE,                             // IN0
			RES_M(1) + RES_K(330),                      // R507, R509
			CAP_U(0.1) )                                // C503
	DISCRETE_GAIN(NODE_35,
			NODE_34,                                    // IN0
			RES_K(330)/(RES_M(1) + RES_K(330)))         // GAIN - R507 : R509
	DISCRETE_OP_AMP_VCO2(NODE_36,                       // IC C1, pin 4
			1,                                          // ENAB
			NODE_35,                                    // VMOD1
			NODE_33,                                    // VMOD2
					&spcenctr_player_shot_op_amp_osc)
	DISCRETE_OP_AMP(NODE_37,                            // IC C1, pin 9
			1,                                          // ENAB
			NODE_36,                                    // IN0
			NODE_33,                                    // IN1
			&spcenctr_player_shot_op_amp_C1)
	DISCRETE_OP_AMP_TRIG_VCA(SPCENCTR_PLAYER_SHOT_SND,  // IC C1, pin 10
			SPCENCTR_PLAYER_SHOT_EN,                    // TRG0
			0,                                          // no TRG1
			0,                                          // no TRG2
			NODE_37,                                    // IN0
			0,                                          // no IN1
			&spcenctr_player_shot_tvca)

	/************************************************
	 *Screech - unemulated
	 ************************************************/
	DISCRETE_CONSTANT(SPCENCTR_SCREECH_SND, 0)

	/************************************************
	 * Crash
	 ************************************************/
	DISCRETE_OP_AMP_TRIG_VCA(NODE_60,                   // IC C2, pin 4
			SPCENCTR_CRASH_EN,                          // TRG0
			0,                                          // no TRG1
			0,                                          // no TRG2
			SPCENCTR_NOISE,                             // IN0
			0,                                          // no IN1
			&spcenctr_crash_tvca)
	// The next 5 modules emulate the filter.
	// The DC level was breadboarded and the frequency response was SPICEd
	DISCRETE_ADDER2(NODE_61,                            // center on filter DC level
			1,                                          // ENAB
			NODE_60,                                    // IN0
			-6.8)                                       // IN1
	DISCRETE_FILTER2(NODE_62,
			1,                                          // ENAB
			NODE_61,                                    // INP0
			130,                                        // FREQ
			1.0 / 8,                                    // DAMP
			DISC_FILTER_BANDPASS)
	DISCRETE_GAIN(NODE_63,
			NODE_62,                                    // IN0
			6)                                          // GAIN
	DISCRETE_ADDER2(NODE_64,                                    // center on filter DC level
			1,                                          // ENAB
			NODE_63,                                    // IN0
			6.8)                                        // IN1
	DISCRETE_CLAMP(SPCENCTR_CRASH_SND,                          // IC C2, pin 5
			NODE_64,                                    // IN0
			0,                                          // MIN
			12.0 - OP_AMP_NORTON_VBE)                   // MAX

	/************************************************
	 * Explosion
	 ************************************************/
	DISCRETE_OP_AMP_TRIG_VCA(NODE_70,                   // IC D2, pin 10
			SPCENCTR_EXPLOSION_EN,                      // TRG0
			0,                                          // no TRG1
			0,                                          // no TRG2
			SPCENCTR_NOISE,                             // IN0
			0,                                          // no IN1
			&spcenctr_explosion_tvca)
	DISCRETE_RCFILTER(NODE_71,
			NODE_70,                                    // IN0
			RES_K(1.8),                                 // R405
			CAP_U(0.22) )                               // C401
	DISCRETE_RCFILTER(SPCENCTR_EXPLOSION_SND,
			NODE_71,                                    // IN0
			RES_K(1.8) + RES_K(3.6),                    // R405 + R406
			CAP_U(0.22) )                               // C402

	/************************************************
	 *Bonus
	 ************************************************/
	DISCRETE_555_ASTABLE(NODE_80,                       // pin 5
			// the pin 4 reset is not connected in schematic, but should be
			SPCENCTR_BONUS_EN,                          // RESET
			RES_K(1),                                   // R710
			RES_K(27),                                  // R711
			CAP_U(0.047),                               // C710
			&spcenctr_555_bonus)
	DISCRETE_555_ASTABLE(NODE_81,                       // pin 9
			SPCENCTR_BONUS_EN,                          // RESET pin 10
			RES_K(100),                                 // R713
			RES_K(47),                                  // R714
			CAP_U(1),                                   // C713
			&spcenctr_555_bonus)
	DISCRETE_LOGIC_AND3(NODE_82,                        // IC C-D, pin 6
			NODE_80,                                    // INP0
			NODE_81,                                    // INP1
			SPCENCTR_BONUS_EN)                          // INP2
	DISCRETE_GAIN(SPCENCTR_BONUS_SND,                   // adjust from logic to TTL voltage level
			NODE_82,                                    // IN0
			DEFAULT_TTL_V_LOGIC_1)                      // GAIN

	/************************************************
	 *Wind - unemulated
	 ************************************************/
	DISCRETE_CONSTANT(SPCENCTR_WIND_SND, 0)

	/************************************************
	 * Final mix
	 ************************************************/
	DISCRETE_MIXER7(NODE_91,
			1,                                          // ENAB
			SPCENCTR_ENEMY_SHIP_SHOT_SND,               // IN0
			SPCENCTR_PLAYER_SHOT_SND,                   // IN1
			SPCENCTR_SCREECH_SND,                       // IN2
			SPCENCTR_CRASH_SND,                         // IN3
			SPCENCTR_EXPLOSION_SND,                     // IN4
			SPCENCTR_BONUS_SND,                         // IN5
			SPCENCTR_WIND_SND,                          // IN6
			&spcenctr_mixer)

	DISCRETE_OUTPUT(NODE_91, 20000)
DISCRETE_SOUND_END

spcenctr_audio_device::spcenctr_audio_device(machine_config const &mconfig, char const *tag, device_t *owner, u32 clock) :
	device_t(mconfig, SPCENCTR_AUDIO, tag, owner, clock),
	m_sn(*this, "snsnd"),
	m_discrete(*this, "discrete"),
	m_lamp(*this, "LAMP"),
	m_strobe(*this, "STROBE"),
	m_strobe_timer(nullptr),
	m_strobe_enable(0U)
{
}

void spcenctr_audio_device::p1_w(u8 data)
{
	machine().sound().system_mute(!BIT(data, 0));

	// D1 is marked as 'OPTIONAL SWITCH VIDEO FOR COCKTAIL', but it is never set by the software

	m_discrete->write(SPCENCTR_CRASH_EN, BIT(data, 2));

	// D3-D7 are not connected
}

void spcenctr_audio_device::p2_w(u8 data)
{
	// set WIND SOUND FREQ(data & 0x0f)  0, if no wind

	m_discrete->write(SPCENCTR_EXPLOSION_EN, BIT(data, 4));
	m_discrete->write(SPCENCTR_PLAYER_SHOT_EN, BIT(data, 5));

	// D6 and D7 are not connected
}


void spcenctr_audio_device::p3_w(u8 data)
{
	// if (data & 0x01)  enable SCREECH (hit the sides) sound

	m_discrete->write(SPCENCTR_ENEMY_SHIP_SHOT_EN, BIT(data, 1));

	m_strobe_enable = BIT(data, 2);

	m_lamp = BIT(data, 3);

	m_discrete->write(SPCENCTR_BONUS_EN, BIT(data, 4));

	m_sn->enable_w(BIT(data, 5));

	// D6 and D7 are not connected
}

void spcenctr_audio_device::device_add_mconfig(machine_config &config)
{
	SPEAKER(config, "mono").front_center();

	SN76477(config, m_sn);
	m_sn->set_noise_params(0, 0, 0);
	m_sn->set_decay_res(0);
	m_sn->set_attack_params(0, RES_K(100));
	m_sn->set_amp_res(RES_K(56));
	m_sn->set_feedback_res(RES_K(10));
	m_sn->set_vco_params(0, CAP_U(0.047), RES_K(56));
	m_sn->set_pitch_voltage(5.0);
	m_sn->set_slf_params(CAP_U(1.0), RES_K(150));
	m_sn->set_oneshot_params(0, 0);
	m_sn->set_vco_mode(1);
	m_sn->set_mixer_params(0, 0, 0);
	m_sn->set_envelope_params(1, 0);
	m_sn->set_enable(1);
	m_sn->add_route(ALL_OUTPUTS, "mono", 0.20);

	DISCRETE(config, m_discrete, spcenctr_discrete);
	m_discrete->add_route(ALL_OUTPUTS, "mono", 0.45);
}

void spcenctr_audio_device::device_start()
{
	m_lamp.resolve();
	m_strobe.resolve();

	m_strobe_timer = timer_alloc(FUNC(spcenctr_audio_device::strobe_callback), this);

	m_strobe_enable = 0U;

	save_item(NAME(m_strobe_enable));

	strobe_callback(0);
}

TIMER_CALLBACK_MEMBER(spcenctr_audio_device::strobe_callback)
{
	constexpr double STROBE_FREQ = 9.00;    // Hz - calculated from the 555 timer
	constexpr u32 STROBE_DUTY_CYCLE = 95;   // %

	m_strobe = (param && m_strobe_enable) ? 1 : 0;
	m_strobe_timer->adjust(
			attotime::from_hz(STROBE_FREQ) * (param ? (100 - STROBE_DUTY_CYCLE) : STROBE_DUTY_CYCLE) / 100,
			param ? 0 : 1);
}


/*************************************
 *
 *  Phantom II
 *
 *************************************/

phantom2_audio_device::phantom2_audio_device(machine_config const &mconfig, char const *tag, device_t *owner, u32 clock) :
	device_t(mconfig, PHANTOM2_AUDIO, tag, owner, clock),
	m_samples(*this, "samples"),
	m_exp(*this, "EXPLAMP"),
	m_p1(0U),
	m_p2(0U)
{
}

void phantom2_audio_device::p1_w(u8 data)
{
	u8 const rising(data & ~m_p1);
	m_p1 = data;

	if (BIT(rising, 0)) m_samples->start(0, 0); // PLAYER SHOT sound

	// if (data & 0x02)  enable ENEMY SHOT sound

	// previously, code did this - system_mute and system_enable controlled the same thing, so bit 5 was ignored
	//machine().sound().system_mute(!BIT(data, 5));
	//machine().sound().system_enable(BIT(data, 2));
	machine().sound().system_mute(!BIT(data, 5) && !BIT(data, 2));

	machine().bookkeeping().coin_counter_w(0, BIT(data, 3));

	// if (data & 0x10)  enable RADAR sound

	// D5-D7 are not connected
}

void phantom2_audio_device::p2_w(u8 data)
{
	u8 const rising(data & ~m_p2);
	m_p2 = data;

	// D0-D2 are not connected

	if (BIT(rising, 3)) m_samples->start(1, 1); // enable EXPLOSION sound

	m_exp = BIT(data, 4);

	// set JET SOUND FREQ((data >> 5) & 0x07)  0, if no jet sound
}

void phantom2_audio_device::device_add_mconfig(machine_config &config)
{
	static char const *const sample_names[] = {
			"*phantom2",
			"1",        // shot
			"2",        // explosion
			nullptr };

	SPEAKER(config, "mono").front_center();

	SAMPLES(config, m_samples);
	m_samples->set_channels(2);
	m_samples->set_samples_names(sample_names);
	m_samples->add_route(ALL_OUTPUTS, "mono", 1.0);
}

void phantom2_audio_device::device_start()
{
	m_exp.resolve();

	m_p1 = 0U;
	m_p2 = 0U;

	save_item(NAME(m_p1));
	save_item(NAME(m_p2));
}


/*************************************
 *
 *  Space Invaders
 *
 *  Discrete sound emulation:
 *  Apr 2007, D.R.
 *
 *************************************/

static discrete_op_amp_1sht_info const invaders_invader_hit_1sht =
{
	DISC_OP_AMP_1SHT_1 | DISC_OP_AMP_IS_NORTON,
	RES_M(4.7),     // R49
	RES_K(100),     // R51
	RES_M(1),       // R48
	RES_M(1),       // R50
	RES_M(2.2),     // R52
	CAP_U(0.1),     // C18
	CAP_P(470),     // C20
	0,              // vN
	12              // vP
};

static const discrete_op_amp_osc_info invaders_invader_hit_osc =
{
	DISC_OP_AMP_OSCILLATOR_1 | DISC_OP_AMP_IS_NORTON | DISC_OP_AMP_OSCILLATOR_OUT_CAP,
	RES_M(1),       // R37
	RES_K(10),      // R41
	RES_K(100),     // R38
	RES_K(120),     // R40
	RES_M(1),       // R39
	0,              // no r6
	0,              // no r7
	0,              // no r8
	CAP_U(0.22),    // C16, CAP_U(0.1) on Midway PCB C21
	12,             // vP
};

// Schematic M051-00739-A005 and M051-00739-B005
// P.C.      A084-90700-B000 and A084-90700-C000
static DISCRETE_SOUND_START(invaders_discrete)
	INVADERS_NOISE_GENERATOR
	INVADERS_SAUCER_HIT(1)
	INVADERS_FLEET(1)
	INVADERS_BONUS_MISSLE_BASE(1)
	INVADERS_INVADER_HIT(1, invaders)
	INVADERS_EXPLOSION(1)
	INVADERS_MISSILE(1, invaders)
	INVADERS_MIXER(1, invaders)
DISCRETE_SOUND_END

invaders_audio_device::invaders_audio_device(machine_config const &mconfig, char const *tag, device_t *owner, u32 clock) :
	device_t(mconfig, INVADERS_AUDIO, tag, owner, clock),
	m_sn(*this, "snsnd"),
	m_discrete(*this, "discrete"),
	m_flip_screen_out(*this),
	m_p2(0U)
{
}

void invaders_audio_device::p1_w(u8 data)
{
	m_sn->enable_w(BIT(~data, 0));    // saucer sound

	m_discrete->write(INVADERS_NODE(INVADERS_MISSILE_EN, 1), data & 0x02);
	m_discrete->write(INVADERS_NODE(INVADERS_EXPLOSION_EN, 1), data & 0x04);
	m_discrete->write(INVADERS_NODE(INVADERS_INVADER_HIT_EN, 1), data & 0x08);
	m_discrete->write(INVADERS_NODE(INVADERS_BONUS_MISSLE_BASE_EN, 1), data & 0x10);

	machine().sound().system_mute(!BIT(data, 5));

	// D6 and D7 are not connected
}


void invaders_audio_device::p2_w(u8 data)
{
	u8 const changed(data ^ m_p2);
	m_p2 = data;

	m_discrete->write(INVADERS_NODE(INVADERS_FLEET_DATA, 1), data & 0x0f);
	m_discrete->write(INVADERS_NODE(INVADERS_SAUCER_HIT_EN, 1), data & 0x10);

	if (BIT(changed, 5)) m_flip_screen_out(BIT(data, 5));

	// D6 and D7 are not connected
}

void invaders_audio_device::device_add_mconfig(machine_config &config)
{
	SPEAKER(config, "mono").front_center();

	SN76477(config, m_sn);
	m_sn->set_noise_params(0, 0, 0);
	m_sn->set_decay_res(0);
	m_sn->set_attack_params(0, RES_K(100));
	m_sn->set_amp_res(RES_K(150));
	m_sn->set_feedback_res(RES_K(10));
	m_sn->set_vco_params(0, CAP_U(0.1), RES_K(8.2));
	m_sn->set_pitch_voltage(5.0);
	m_sn->set_slf_params(CAP_U(1.0), RES_K(120));
	m_sn->set_oneshot_params(0, 0);
	m_sn->set_vco_mode(1);
	m_sn->set_mixer_params(0, 0, 0);
	m_sn->set_envelope_params(1, 0);
	m_sn->set_enable(1);
	m_sn->add_route(ALL_OUTPUTS, "mono", 0.5);

	DISCRETE(config, m_discrete, invaders_discrete);
	m_discrete->add_route(ALL_OUTPUTS, "mono", 0.5);
}

void invaders_audio_device::device_start()
{
	m_p2 = 0U;

	save_item(NAME(m_p2));
}


/*************************************
 *
 *  Space Invaders II (cocktail)
 *
 *************************************/

static discrete_op_amp_1sht_info const invad2ct_invader_hit_1sht =
{
	DISC_OP_AMP_1SHT_1 | DISC_OP_AMP_IS_NORTON,
	RES_M(4.7),                 // R49
	RES_K(100),                 // R51
	RES_M(1),                   // R48
	RES_M(1),                   // R50
	RES_M(2.2),                 // R52
	CAP_U(0.22),                // C18
	CAP_P(470),                 // C20
	0,                          // vN
	12                          // vP
};

static discrete_op_amp_osc_info const invad2ct_invader_hit_osc =
{
	DISC_OP_AMP_OSCILLATOR_1 | DISC_OP_AMP_IS_NORTON | DISC_OP_AMP_OSCILLATOR_OUT_CAP,
	RES_M(1),                   // R37
	RES_K(10),                  // R41
	RES_K(100),                 // R38
	RES_K(120),                 // R40
	RES_M(1),                   // R39
	0,                          // no r6
	0,                          // no r7
	0,                          // no r8
	CAP_U(0.22),                // C16
	12,                         // vP
};

static discrete_op_amp_1sht_info const invad2ct_brd2_invader_hit_1sht =
{
	DISC_OP_AMP_1SHT_1 | DISC_OP_AMP_IS_NORTON,
	RES_M(4.7),                 // R49
	RES_K(100),                 // R51
	RES_M(1),                   // R48
	RES_M(1),                   // R50
	RES_M(2.2),                 // R52
	CAP_U(1),                   // C18
	CAP_P(470),                 // C20
	0,                          // vN
	12                          // vP
};

static discrete_op_amp_osc_info const invad2ct_brd2_invader_hit_osc =
{
	DISC_OP_AMP_OSCILLATOR_1 | DISC_OP_AMP_IS_NORTON | DISC_OP_AMP_OSCILLATOR_OUT_CAP,
	RES_M(1),                   // R37
	RES_K(10),                  // R41
	RES_K(100),                 // R38
	RES_K(120),                 // R40
	RES_M(1),                   // R39
	0,                          // no r6
	0,                          // no r7
	0,                          // no r8
	CAP_U(0.1),                 // C16
	12,                         // vP
};

static discrete_op_amp_1sht_info const invad2ct_missle_1sht =
{
	DISC_OP_AMP_1SHT_1 | DISC_OP_AMP_IS_NORTON,
	RES_M(4.7),                 // R32
	RES_K(100),                 // R30
	RES_M(1),                   // R31
	RES_M(1),                   // R33
	RES_M(2.2),                 // R34
	CAP_U(0.22),                // C12
	CAP_P(470),                 // C15
	0,                          // vN
	12                          // vP
};

static discrete_mixer_desc const invad2ct_mixer =
{
	DISC_MIXER_IS_OP_AMP,       // type
	{ RES_K(100),               // R78
		RES_K(15) + 100 + 100,  // R134 + R133 + R132
		RES_K(150),             // R136
		RES_K(150),             // R59
		RES_K(10) + RES_K(6.8) + RES_K(5.6),    // R86 + R85 + R84
		RES_K(150) },           // R28
	{0},                        // no rNode{}
	{ 0,
		0,
		0,
		0,
		0,
		CAP_U(0.001) },         // C11
	0,                          // no rI
	RES_K(100),                 // R105
	0,                          // no cF
	CAP_U(0.1),                 // C45
	0,                          // vRef = ground
	1                           // gain
};

static DISCRETE_SOUND_START(invad2ct_discrete)
	// sound board 1
	// P.C. A082-90700-A000
	// Schematic M051-00851-A002
	INVADERS_NOISE_GENERATOR
	INVADERS_SAUCER_HIT(1)
	INVADERS_FLEET(1)
	INVADERS_BONUS_MISSLE_BASE(1)
	INVADERS_INVADER_HIT(1, invad2ct)
	INVADERS_EXPLOSION(1)
	INVADERS_MISSILE(1, invad2ct)
	INVADERS_MIXER(1, invad2ct)

	// sound board 2
	// P.C. A084-90901-C851
	// Schematic M051-00851-A005
	INVADERS_SAUCER_HIT(2)
	INVADERS_FLEET(2)
	INVADERS_BONUS_MISSLE_BASE(2)
	INVADERS_INVADER_HIT(2, invad2ct_brd2)
	INVADERS_EXPLOSION(2)
	INVADERS_MISSILE(2, invaders)
	INVADERS_MIXER(2, invaders)
DISCRETE_SOUND_END

invad2ct_audio_device::invad2ct_audio_device(machine_config const &mconfig, char const *tag, device_t *owner, u32 clock) :
	device_t(mconfig, INVAD2CT_AUDIO, tag, owner, clock),
	m_discrete(*this, "discrete"),
	m_sn(*this, "sn%u", 1U)
{
}

void invad2ct_audio_device::p1_w(u8 data)
{
	m_sn[0]->enable_w(BIT(~data, 0));   // saucer sound

	m_discrete->write(INVADERS_NODE(INVADERS_MISSILE_EN, 1), data & 0x02);
	m_discrete->write(INVADERS_NODE(INVADERS_EXPLOSION_EN, 1), data & 0x04);
	m_discrete->write(INVADERS_NODE(INVADERS_INVADER_HIT_EN, 1), data & 0x08);
	m_discrete->write(INVADERS_NODE(INVADERS_BONUS_MISSLE_BASE_EN, 1), data & 0x10);

	machine().sound().system_mute(!BIT(data, 5));

	// D6 and D7 are not connected
}

void invad2ct_audio_device::p2_w(u8 data)
{
	m_discrete->write(INVADERS_NODE(INVADERS_FLEET_DATA, 1), data & 0x0f);
	m_discrete->write(INVADERS_NODE(INVADERS_SAUCER_HIT_EN, 1), data & 0x10);

	// D5-D7 are not connected
}

void invad2ct_audio_device::p3_w(u8 data)
{
	m_sn[1]->enable_w(BIT(~data, 0));   // saucer sound

	m_discrete->write(INVADERS_NODE(INVADERS_MISSILE_EN, 2), data & 0x02);
	m_discrete->write(INVADERS_NODE(INVADERS_EXPLOSION_EN, 2), data & 0x04);
	m_discrete->write(INVADERS_NODE(INVADERS_INVADER_HIT_EN, 2), data & 0x08);
	m_discrete->write(INVADERS_NODE(INVADERS_BONUS_MISSLE_BASE_EN, 2), data & 0x10);

	// D5-D7 are not connected
}

void invad2ct_audio_device::p4_w(u8 data)
{
	m_discrete->write(INVADERS_NODE(INVADERS_FLEET_DATA, 2), data & 0x0f);
	m_discrete->write(INVADERS_NODE(INVADERS_SAUCER_HIT_EN, 2), data & 0x10);

	// D5-D7 are not connected
}

void invad2ct_audio_device::device_add_mconfig(machine_config &config)
{
	SPEAKER(config, "spk1").front_left();
	SPEAKER(config, "spk2").front_right();

	DISCRETE(config, m_discrete, invad2ct_discrete);
	m_discrete->add_route(0, "spk1", 0.5);
	m_discrete->add_route(1, "spk2", 0.5);

	SN76477(config, m_sn[0]);
	m_sn[0]->set_noise_params(0, 0, 0);
	m_sn[0]->set_decay_res(0);
	m_sn[0]->set_attack_params(0, RES_K(100));
	m_sn[0]->set_amp_res(RES_K(150));
	m_sn[0]->set_feedback_res(RES_K(10));
	m_sn[0]->set_vco_params(0, CAP_U(0.1), RES_K(8.2));
	m_sn[0]->set_pitch_voltage(5.0);
	m_sn[0]->set_slf_params(CAP_U(1.0), RES_K(120));
	m_sn[0]->set_oneshot_params(0, 0);
	m_sn[0]->set_vco_mode(1);
	m_sn[0]->set_mixer_params(0, 0, 0);
	m_sn[0]->set_envelope_params(1, 0);
	m_sn[0]->set_enable(1);
	m_sn[0]->add_route(ALL_OUTPUTS, "spk1", 0.3);

	SN76477(config, m_sn[1]);
	m_sn[1]->set_noise_params(0, 0, 0);
	m_sn[1]->set_decay_res(0);
	m_sn[1]->set_attack_params(0, RES_K(100));
	m_sn[1]->set_amp_res(RES_K(150));
	m_sn[1]->set_feedback_res(RES_K(10));
	m_sn[1]->set_vco_params(0, CAP_U(0.047),  RES_K(39));
	m_sn[1]->set_pitch_voltage(5.0);
	m_sn[1]->set_slf_params(CAP_U(1.0), RES_K(120));
	m_sn[1]->set_oneshot_params(0, 0);
	m_sn[1]->set_vco_mode(1);
	m_sn[1]->set_mixer_params(0, 0, 0);
	m_sn[1]->set_envelope_params(1, 0);
	m_sn[1]->set_enable(1);
	m_sn[1]->add_route(ALL_OUTPUTS, "spk2", 0.3);
}

void invad2ct_audio_device::device_start()
{
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


static DISCRETE_SOUND_START(tornbase_discrete)

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


void mw8080bw_state::tornbase_audio(machine_config &config)
{
	SPEAKER(config, "mono").front_center();
	DISCRETE(config, m_discrete, tornbase_discrete);
	m_discrete->add_route(ALL_OUTPUTS, "mono", 1);
}


void mw8080bw_state::tornbase_audio_w(uint8_t data)
{
	m_discrete->write(TORNBASE_TONE_240_EN, (data >> 0) & 0x01);

	m_discrete->write(TORNBASE_TONE_960_EN, (data >> 1) & 0x01);

	m_discrete->write(TORNBASE_TONE_120_EN, (data >> 2) & 0x01);

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

	machine().bookkeeping().coin_counter_w(0, (data >> 7) & 0x01);
}



/*************************************
 *
 *  280 ZZZAP / Laguna Racer
 *
 *************************************/

// Sound board volume potentiometer, set to its midpoint value by default.

static INPUT_PORTS_START(zzzap_audio)
	PORT_START("POT_MASTER_VOL")
	PORT_ADJUSTER( 50, "Pot: Master Volume" )  NETLIST_ANALOG_PORT_CHANGED("sound_nl", "pot_master_vol")
INPUT_PORTS_END

zzzap_common_audio_device::zzzap_common_audio_device(machine_config const &mconfig, device_type type, char const *tag, device_t *owner, u32 clock, void (*netlist)(netlist::nlparse_t &)) :
	device_t(mconfig, type, tag, owner, clock),
	m_netlist(netlist),
	m_pedal_bit0(*this, "sound_nl:pedal_bit0"),
	m_pedal_bit1(*this, "sound_nl:pedal_bit1"),
	m_pedal_bit2(*this, "sound_nl:pedal_bit2"),
	m_pedal_bit3(*this, "sound_nl:pedal_bit3"),
	m_hi_shift(*this, "sound_nl:hi_shift"),
	m_lo_shift(*this, "sound_nl:lo_shift"),
	m_boom(*this, "sound_nl:boom"),
	m_engine_sound_off(*this, "sound_nl:engine_sound_off"),
	m_noise_cr_1(*this, "sound_nl:noise_cr_1"),
	m_noise_cr_2(*this, "sound_nl:noise_cr_2")
{
}


void zzzap_common_audio_device::p1_w(u8 data)
{
	// **** Output pins from 74174 latch at F5 ****

	// Bits 0-3 (PEDAL_BIT0 to PEDAL_BIT3): accelerator pedal position
	// Sets the frequency and volume of the engine sound oscillators.
	m_pedal_bit0->write_line(BIT(data, 0));
	m_pedal_bit1->write_line(BIT(data, 1));
	m_pedal_bit2->write_line(BIT(data, 2));
	m_pedal_bit3->write_line(BIT(data, 3));

	// Bit 4 (HI SHIFT): set when gearshift is in high gear
	// Modifies the engine sound to be lower pitched at a given speed and
	// to change more slowly.
	m_hi_shift->write_line(BIT(data, 4));

	// Bit 5 (LO SHIFT): set when gearshift is in low gear
	// Modifies the engine sound to be higher pitched at a given speed and
	// to change faster.
	m_lo_shift->write_line(BIT(data, 5));

	// Bits 6-7 (D6, D7): not connected.
}


void zzzap_common_audio_device::p2_w(u8 data)
{
	// **** Output pins from 74174 latch at F4 ****

	// Bit 0 (BOOM): Set to activate boom sound for a crash. Cleared to
	// terminate boom.
	m_boom->write_line(BIT(data, 0));

	// Bit 1 (ENGINE SOUND OFF): Set to turn *off* engine sound.
	// Used in a crash or when game is not running.
	m_engine_sound_off->write_line(BIT(data, 1));

	// Bit 2 (NOISE CR 1): tire squealing sound
	// Set to activate "tire squeal" noise from noise generator.
	m_noise_cr_1->write_line(BIT(data, 2));

	// Bit 3 (NOISE CR 2): post-crash noise
	// Set to activate screeching noise that follows BOOM (the car blowing
	// up). This sounds like a generic high-pitched screeching hiss, and
	// it is unclear what it was meant to represent. It's just as
	// ambiguous in the real game.
	m_noise_cr_2->write_line(BIT(data, 3));

	// Bit 5 is for the coin counter.
	machine().bookkeeping().coin_counter_w(0, (data >> 5) & 0x01);

	// Bits 4, 6-7 (D4, D6, D7): not connected.
}


void zzzap_common_audio_device::device_add_mconfig(machine_config &config)
{
	SPEAKER(config, "mono").front_center();

	if (m_netlist != nullptr) {

		NETLIST_SOUND(config, "sound_nl", 48000)
			.set_source(m_netlist)
			.add_route(ALL_OUTPUTS, "mono", 1.0);

		NETLIST_LOGIC_INPUT(config, "sound_nl:pedal_bit0",
					"I_PEDAL_BIT0", 0);
		NETLIST_LOGIC_INPUT(config, "sound_nl:pedal_bit1",
					"I_PEDAL_BIT1", 0);
		NETLIST_LOGIC_INPUT(config, "sound_nl:pedal_bit2",
					"I_PEDAL_BIT2", 0);
		NETLIST_LOGIC_INPUT(config, "sound_nl:pedal_bit3",
					"I_PEDAL_BIT3", 0);
		NETLIST_LOGIC_INPUT(config, "sound_nl:hi_shift",
					"I_HI_SHIFT", 0);
		NETLIST_LOGIC_INPUT(config, "sound_nl:lo_shift",
					"I_LO_SHIFT", 0);
		NETLIST_LOGIC_INPUT(config, "sound_nl:boom", "I_BOOM", 0);
		NETLIST_LOGIC_INPUT(config, "sound_nl:engine_sound_off",
					"I_ENGINE_SOUND_OFF", 0);
		NETLIST_LOGIC_INPUT(config, "sound_nl:noise_cr_1",
					"I_NOISE_CR_1", 0);
		NETLIST_LOGIC_INPUT(config, "sound_nl:noise_cr_2",
					"I_NOISE_CR_2", 0);

		// The audio output is taken from an LM3900 op-amp whose
		// output has a peak-to-peak range of about 5 volts, centered
		// on 2.5 volts. With the master volume potentiometer at its
		// default midpoint setting, this range is cut in half, to 2.5
		// volts peak to peak. In the real machine, the audio power
		// amps might clip the highest output peaks, but I don't model
		// this. Instead, I take the easy way out: assume the output
		// at midpoint volume will just avoid clipping the extreme
		// peaks, and scale and offset it so that those peaks will
		// just reach the clipping limits for signed 16-bit samples.
		// So turning the volume up much higher than the default will
		// give clipped output.
		NETLIST_STREAM_OUTPUT(config, "sound_nl:cout0", 0, "OUTPUT").set_mult_offset(1.0 / 1.25, -(1.0 / 1.25) * 2.50);

		// Netlist volume-potentiometer interface
		NETLIST_ANALOG_INPUT(config, "sound_nl:pot_master_vol", "R70.DIAL");
	}
}

ioport_constructor zzzap_common_audio_device::device_input_ports() const
{
	return INPUT_PORTS_NAME(zzzap_audio);
}

void zzzap_common_audio_device::device_start()
{
}


zzzap_audio_device::zzzap_audio_device(machine_config const &mconfig, char const *tag, device_t *owner, u32 clock) :
		zzzap_common_audio_device(mconfig, ZZZAP_AUDIO, tag, owner, clock, NETLIST_NAME(280zzzap))
{
}


lagunar_audio_device::lagunar_audio_device(machine_config const &mconfig, char const *tag, device_t *owner, u32 clock) :
		zzzap_common_audio_device(mconfig, LAGUNAR_AUDIO, tag, owner, clock, NETLIST_NAME(lagunar))
{
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


static DISCRETE_SOUND_START(maze_discrete)

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


void mw8080bw_state::maze_audio(machine_config &config)
{
	SPEAKER(config, "mono").front_center();
	DISCRETE(config, m_discrete, maze_discrete);
	m_discrete->add_route(ALL_OUTPUTS, "mono", 1.0);
}


void mw8080bw_state::maze_write_discrete(uint8_t maze_tone_timing_state)
{
	/* controls need to be active low */
	int controls = ~ioport("IN0")->read() & 0xff;

	m_discrete->write(MAZE_TONE_TIMING, maze_tone_timing_state);
	m_discrete->write(MAZE_P1_DATA, controls & 0x0f);
	m_discrete->write(MAZE_P2_DATA, (controls >> 4) & 0x0f);
	m_discrete->write(MAZE_JOYSTICK_IN_USE, controls != 0xff);

	/* The coin line is connected directly to the discrete circuit. */
	/* We can't really do that, so updating it with the tone timing is close enough. */
	/* A better option might be to update it at vblank or set a timer to do it. */
	/* The only noticeable difference doing it here, is that the controls don't */
	/* immediately start making tones if pressed right after the coin is inserted. */
	m_discrete->write(MAZE_COIN, (~ioport("IN1")->read() >> 3) & 0x01);
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

static DISCRETE_SOUND_START(checkmat_discrete)

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


void mw8080bw_state::checkmat_audio(machine_config &config)
{
	SPEAKER(config, "mono").front_center();
	DISCRETE(config, m_discrete, checkmat_discrete);
	m_discrete->add_route(ALL_OUTPUTS, "mono", 0.4);
}


void mw8080bw_state::checkmat_audio_w(uint8_t data)
{
	m_discrete->write(CHECKMAT_TONE_EN, data & 0x01);

	m_discrete->write(CHECKMAT_BOOM_EN, (data >> 1) & 0x01);

	machine().bookkeeping().coin_counter_w(0, (data >> 2) & 0x01);

	machine().sound().system_mute(!BIT(data, 3));

	m_discrete->write(CHECKMAT_TONE_DATA_45, (data >> 4) & 0x03);
	m_discrete->write(CHECKMAT_TONE_DATA_67, (data >> 6) & 0x03);
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


static DISCRETE_SOUND_START(shuffle_discrete)
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


void mw8080bw_state::shuffle_audio(machine_config &config)
{
	SPEAKER(config, "mono").front_center();

	DISCRETE(config, m_discrete, shuffle_discrete);
	m_discrete->add_route(ALL_OUTPUTS, "mono", 1.0);
}


void mw8080bw_state::shuffle_audio_1_w(uint8_t data)
{
	m_discrete->write(SHUFFLE_CLICK_EN, (data >> 0) & 0x01);

	m_discrete->write(SHUFFLE_ROLLOVER_EN, (data >> 1) & 0x01);

	machine().sound().system_mute(!BIT(data, 2));

	m_discrete->write(NODE_29, (data >> 3) & 0x07);

	m_discrete->write(SHUFFLE_ROLLING_3_EN, (data >> 3) & 0x01);
	m_discrete->write(SHUFFLE_ROLLING_2_EN, (data >> 4) & 0x01);
	m_discrete->write(SHUFFLE_ROLLING_1_EN, (data >> 5) & 0x01);

	/* D6 and D7 are not connected */
}


void mw8080bw_state::shuffle_audio_2_w(uint8_t data)
{
	m_discrete->write(SHUFFLE_FOUL_EN, (data >> 0) & 0x01);

	machine().bookkeeping().coin_counter_w(0, (data >> 1) & 0x01);

	/* D2-D7 are not connected */
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


static DISCRETE_SOUND_START(bowler_discrete)

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


void bowler_state::audio(machine_config &config)
{
	SPEAKER(config, "mono").front_center();

	DISCRETE(config, m_discrete, bowler_discrete);
	m_discrete->add_route(ALL_OUTPUTS, "mono", 1);
}


void bowler_state::audio_1_w(uint8_t data)
{
	/* D0 - selects controller on the cocktail PCB */

	machine().bookkeeping().coin_counter_w(0, (data >> 1) & 0x01);

	machine().sound().system_mute(!BIT(data, 2));

	m_discrete->write(BOWLER_FOWL_EN, (data >> 3) & 0x01);

	/* D4 - appears to be a screen flip, but it's
	        shown unconnected on the schematics for both the
	        upright and cocktail PCB's */

	/* D5 - triggered on a 'strike', sound circuit not labeled */

	/* D6 and D7 are not connected */
}


void bowler_state::audio_2_w(uint8_t data)
{
	/* set BALL ROLLING SOUND FREQ(data & 0x0f)
	   0, if no rolling, 0x08 used during ball return */

	/* D4 -  triggered when the ball crosses the foul line,
	         sound circuit not labeled */

	/* D5 - triggered on a 'spare', sound circuit not labeled */

	/* D6 and D7 are not connected */
}


void bowler_state::audio_3_w(uint8_t data)
{
	/* regardless of the data, enable BALL HITS PIN 1 sound
	   (top circuit on the schematics) */
}


void bowler_state::audio_4_w(uint8_t data)
{
	/* regardless of the data, enable BALL HITS PIN 2 sound
	   (bottom circuit on the schematics) */
}


void bowler_state::audio_5_w(uint8_t data)
{
	/* not sure, appears to me triggered alongside the two
	   BALL HITS PIN sounds */
}


void bowler_state::audio_6_w(uint8_t data)
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

static DISCRETE_SOUND_START(blueshrk_discrete)

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


void mw8080bw_state::blueshrk_audio(machine_config &config)
{
	SPEAKER(config, "mono").front_center();
	DISCRETE(config, m_discrete, blueshrk_discrete);
	m_discrete->add_route(ALL_OUTPUTS, "mono", 0.25);
}


void mw8080bw_state::blueshrk_audio_w(uint8_t data)
{
	m_discrete->write(BLUESHRK_GAME_ON_EN, (data >> 0) & 0x01);

	m_discrete->write(BLUESHRK_SHOT_EN, (data >> 1) & 0x01);

	m_discrete->write(BLUESHRK_HIT_EN, (data >> 2) & 0x01);

	m_discrete->write(BLUESHRK_SHARK_EN, (data >> 3) & 0x01);

	/* if (data & 0x10)  enable KILLED DIVER sound, this circuit
	   doesn't appear to be on the schematics */

	m_discrete->write(BLUESHRK_OCTOPUS_EN, (data >> 5) & 0x01);

	/* D6 and D7 are not connected */
}
