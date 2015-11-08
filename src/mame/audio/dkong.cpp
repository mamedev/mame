// license:BSD-3-Clause
// copyright-holders:Couriersud
#include "emu.h"
#include "cpu/mcs48/mcs48.h"
#include "sound/discrete.h"

#include "sound/tms5110.h"

#include "includes/dkong.h"


/****************************************************************
 *
 * Defines and Macros
 *
 ****************************************************************/

/* Set to 1 to disable DAC and post-mixer filters */
#define     DK_NO_FILTERS   (0)
/* Set to 1 to use faster custom mixer */
#define     DK_USE_CUSTOM   (1)

/* Issue surrounded by this define need to be analyzed and
 * reviewed at a later time.
 * Currently, the following issues exist:
 * - although not present on schematics, a 10K resistor is needed
 *   as RF in the mixer stage. Without this resistor, the DAC
 *   sound is completely overmodulated.
 */

/* FIXME: Review at a later time */
#define     DK_REVIEW       (1)


/****************************************************************
 *
 * Discrete Sound defines
 *
 ****************************************************************/

/* Discrete sound inputs */

#define DS_SOUND0_INV       NODE_01
#define DS_SOUND1_INV       NODE_02
#define DS_SOUND2_INV       NODE_03
#define DS_SOUND6_INV       NODE_04
#define DS_SOUND7_INV       NODE_05
#define DS_SOUND9_INV       NODE_06
#define DS_DAC              NODE_07
#define DS_DISCHARGE_INV    NODE_08

#define DS_SOUND0           NODE_208
#define DS_SOUND1           NODE_209
#define DS_SOUND6           NODE_210
#define DS_SOUND7           NODE_211
#define DS_SOUND9           NODE_212

#define DS_ADJ_DAC          NODE_240

#define DS_OUT_SOUND0       NODE_241
#define DS_OUT_SOUND1       NODE_242
#define DS_OUT_SOUND2       NODE_243
#define DS_OUT_SOUND6       NODE_247
#define DS_OUT_SOUND7       NODE_248
#define DS_OUT_SOUND9       NODE_249
#define DS_OUT_DAC          NODE_250

/* Input definitions for write handlers */

#define DS_SOUND0_INP       DS_SOUND0_INV
#define DS_SOUND1_INP       DS_SOUND1_INV
#define DS_SOUND2_INP       DS_SOUND2_INV
#define DS_SOUND6_INP       DS_SOUND6_INV
#define DS_SOUND7_INP       DS_SOUND7_INV
#define DS_SOUND9_INP       DS_SOUND9_INV

/* General defines */

#define DK_1N5553_V         0.4 /* from datasheet at 1mA */
#define DK_SUP_V            5.0
#define NE555_INTERNAL_R    RES_K(5)

#define R_SERIES(R1,R2)   ((R1)+(R2))

/****************************************************************
 *
 * Static declarations
 *
 ****************************************************************/



/****************************************************************
 *
 * Dkong Discrete Sound Interface
 *
 ****************************************************************/

/* Resistors */

#define DK_R1       RES_K(47)
#define DK_R2       RES_K(47)
#define DK_R3       RES_K(5.1)
#define DK_R4       RES_K(2)
#define DK_R5       750
#define DK_R6       RES_K(4.7)
#define DK_R7       RES_K(10)
#define DK_R8       RES_K(100)
#define DK_R9       RES_K(10)
#define DK_R10      RES_K(10)
#define DK_R14      RES_K(47)

#define DK_R15      RES_K(5.6)
#define DK_R16      RES_K(5.6)
#define DK_R17      RES_K(10)
#define DK_R18      RES_K(4.7)
#define DK_R20      RES_K(10)
#define DK_R21      RES_K(5.6)
#define DK_R22      RES_K(5.6)
#define DK_R24      RES_K(47)
#define DK_R25      RES_K(5.1)
#define DK_R26      RES_K(2)
#define DK_R27      150
#define DK_R28      RES_K(4.7)
#define DK_R29      RES_K(10)
#define DK_R30      RES_K(100)
#define DK_R31      RES_K(10)
#define DK_R32      RES_K(10)
#define DK_R35      RES_K(1)
#define DK_R36      RES_K(1)
#define DK_R38      RES_K(18)
#define DK_R39      RES_M(3.3)
#define DK_R49      RES_K(1.2)
#define DK_R44      RES_K(1.2)
#define DK_R45      RES_K(10)
#define DK_R46      RES_K(12)
#define DK_R47      RES_K(4.3)
#define DK_R48      RES_K(43)
#define DK_R50      RES_K(10)
#define DK_R51      RES_K(10)

/* Capacitors */

#define DK_C8       CAP_U(220)
#define DK_C12      CAP_U(1)
#define DK_C13      CAP_U(33)
#define DK_C16      CAP_U(1)
#define DK_C17      CAP_U(4.7)
#define DK_C18      CAP_U(1)
#define DK_C19      CAP_U(1)
#define DK_C20      CAP_U(3.3)
#define DK_C21      CAP_U(1)

#define DK_C23      CAP_U(4.7)
#define DK_C24      CAP_U(10)
#define DK_C25      CAP_U(3.3)
#define DK_C26      CAP_U(3.3)
#define DK_C29      CAP_U(3.3)
#define DK_C30      CAP_U(10)
#define DK_C32      CAP_U(10)
#define DK_C34      CAP_N(10)
#define DK_C159     CAP_N(100)


/*
 * The noice generator consists of three LS164 8+8+8
 * the output signal is taken after the xor, so
 * taking bit 0 is not exact
 */

static const discrete_lfsr_desc dkong_lfsr =
{
	DISC_CLK_IS_FREQ,
	24,                   /* Bit Length */
	0,                    /* Reset Value */
	10,                   /* Use Bit 10 (QC of second LS164) as F0 input 0 */
	23,                   /* Use Bit 23 (QH of third LS164) as F0 input 1 */
	DISC_LFSR_XOR,        /* F0 is XOR */
	DISC_LFSR_NOT_IN0,    /* F1 is inverted F0*/
	DISC_LFSR_REPLACE,    /* F2 replaces the shifted register contents */
	0x000001,             /* Everything is shifted into the first bit only */
	DISC_LFSR_FLAG_OUTPUT_F0, /* Output is result of F0 */
	0                     /* Output bit */
};

static const double dkong_diode_mix_table[2] = {DK_1N5553_V, DK_1N5553_V * 2};

#if !DK_USE_CUSTOM
static const discrete_mixer_desc dkong_rc_jump_desc =
{
	DISC_MIXER_IS_RESISTOR,
	{1, DK_R49+DK_R51,NE555_INTERNAL_R,2*NE555_INTERNAL_R},
	{NODE_26,0,0,0},
	{0,0,0,0},  /* no node capacitors */
	0, 0,
	DK_C24,
	0,
	0, 1
};

static const discrete_mixer_desc dkong_rc_walk_desc =
{
	DISC_MIXER_IS_RESISTOR,
	{1, DK_R45+DK_R44,NE555_INTERNAL_R,2*NE555_INTERNAL_R},
	{NODE_52,0,0,0},
	{0,0,0,0},  /* no node capacitors */
	0, 0,
	DK_C29,
	0,
	0, 1
};
#endif

static const discrete_mixer_desc dkong_mixer_desc =
{
	DISC_MIXER_IS_RESISTOR,
	{DK_R2, DK_R24, DK_R1, DK_R14},
	{0,0,0},  /* no variable resistors */
	{0,0,0},  /* no node capacitors */
#if DK_REVIEW
	0, RES_K(10),
#else
	0, 0,
#endif
	DK_C159,
	DK_C12,
	0, 1
};

/* There is no load on the output for the jump circuit
 * For the walk circuit, the voltage does not matter */

static const discrete_555_desc dkong_555_vco_desc =
{
	DISC_555_OUT_ENERGY | DISC_555_OUT_DC,
	DK_SUP_V,
	DEFAULT_555_CHARGE,
	DK_SUP_V - 0.5
};

static const discrete_dss_inverter_osc_node::description dkong_inverter_osc_desc_jump =
{
	DEFAULT_CD40XX_VALUES(DK_SUP_V),
	discrete_dss_inverter_osc_node::IS_TYPE1
};

static const discrete_dss_inverter_osc_node::description dkong_inverter_osc_desc_walk =
{
	DEFAULT_CD40XX_VALUES(DK_SUP_V),
	discrete_dss_inverter_osc_node::IS_TYPE2
};

static const discrete_op_amp_filt_info dkong_sallen_key_info =
{
	RES_K(5.6), RES_K(5.6), 0, 0, 0,
	CAP_N(22), CAP_N(10), 0
};

#if DK_USE_CUSTOM
/************************************************************************
 *
 * Custom dkong mixer
 *
 * input[0]    - In1 (Logic)
 * input[1]    - In2
 * input[2]    - R1
 * input[3]    - R2
 * input[4]    - R3
 * input[5]    - R4
 * input[6]    - C
 * input[7]    - B+
 *
 *              V (B+)                               V (B+)
 *                v                                    v
 *                |       Node Output <----.       .-- | -----
 *                Z                        |       |   Z
 *                Z R1                     |       |   Z 5k
 *                Z                        |       |   Z     555
 *          |\    |     R2          R4     |           |    internal
 *  In1 >---| >o--+---/\/\/\--+---/\/\/\---+-----------+ CV
 *          |/                |            |           |
 *                            |           ---      |   Z
 *             R3             |           --- C    |   Z 10k
 *  In2 >----/\/\/\-----------'            |       |   Z
 *                                         |       '-- | -----
 *                                        Gnd         Gnd
 *
 ************************************************************************/
#define DKONG_CUSTOM_IN1        DISCRETE_INPUT(0)
#define DKONG_CUSTOM_IN2        DISCRETE_INPUT(1)
#define DKONG_CUSTOM_R1         DISCRETE_INPUT(2)
#define DKONG_CUSTOM_R2         DISCRETE_INPUT(3)
#define DKONG_CUSTOM_R3         DISCRETE_INPUT(4)
#define DKONG_CUSTOM_R4         DISCRETE_INPUT(5)
#define DKONG_CUSTOM_C          DISCRETE_INPUT(6)
#define DKONG_CUSTOM_V          DISCRETE_INPUT(7)

DISCRETE_CLASS_STEP_RESET(dkong_custom_mixer, 1,
	double m_i_in1[2];
	double m_r_in[2];
	double m_r_total[2];
	double m_exp[2];
	double m_out_v;
);

DISCRETE_STEP( dkong_custom_mixer )
{
	int     in_1    = (int)DKONG_CUSTOM_IN1;

	/* start of with 555 current */
	double  i_total = DKONG_CUSTOM_V / RES_K(5);
	/* add in current from In1 */
	i_total += m_i_in1[in_1];
	/* add in current from In2 */
	i_total += DKONG_CUSTOM_IN2 / DKONG_CUSTOM_R3;
	/* charge cap */
	/* node->output is cap voltage, (i_total * m_r_total[in_1]) is current charge voltage */
	m_out_v += (i_total * m_r_total[in_1] - m_out_v) * m_exp[in_1];
	set_output(0, m_out_v);
}

#define NE555_CV_R      RES_2_PARALLEL(RES_K(5), RES_K(10))

DISCRETE_RESET( dkong_custom_mixer )
{
	/* everything is based on the input to the O.C. inverter */
	/* precalculate current from In1 */
	m_i_in1[0] = DKONG_CUSTOM_V / (DKONG_CUSTOM_R1 + DKONG_CUSTOM_R2);
	m_i_in1[1] = 0;
	/* precalculate total resistance for input circuit */
	m_r_in[0] = RES_2_PARALLEL((DKONG_CUSTOM_R1 + DKONG_CUSTOM_R2), DKONG_CUSTOM_R3);
	m_r_in[1] = RES_2_PARALLEL(DKONG_CUSTOM_R2, DKONG_CUSTOM_R3);
	/* precalculate total charging resistance */
	m_r_total[0] = RES_2_PARALLEL(m_r_in[0] + DKONG_CUSTOM_R4, NE555_CV_R);
	m_r_total[1] = RES_2_PARALLEL((m_r_in[1] + DKONG_CUSTOM_R4), NE555_CV_R);
	/* precalculate charging exponents */
	m_exp[0] = RC_CHARGE_EXP(m_r_total[0] * DKONG_CUSTOM_C);
	m_exp[1] = RC_CHARGE_EXP(m_r_total[1] * DKONG_CUSTOM_C);

	m_out_v = 0;
}

#endif

static DISCRETE_SOUND_START(dkong2b)

	/************************************************/
	/* Input register mapping for dkong             */
	/************************************************/

	/* DISCRETE_INPUT_DATA */
	DISCRETE_INPUT_NOT(DS_SOUND2_INV)
	DISCRETE_INPUT_NOT(DS_SOUND1_INV)   /* IC 6J, pin 12 */
	DISCRETE_INPUT_NOT(DS_SOUND0_INV)   /* IC 6J, pin 2 */
	DISCRETE_INPUT_NOT(DS_DISCHARGE_INV)
	//DISCRETE_INPUT_DATA(DS_DAC)

	/************************************************/
	/* Stomp                                        */
	/************************************************/
	/* Noise */
	DISCRETE_TASK_START(1)
	DISCRETE_LFSR_NOISE(NODE_11, 1, 1, CLOCK_2VF, 1.0, 0, 0.5, &dkong_lfsr)
	DISCRETE_COUNTER(NODE_12, 1, 0, NODE_11, 0, 7, DISC_COUNT_UP, 0, DISC_CLK_ON_R_EDGE)    /* LS161, IC 3J */
	DISCRETE_TRANSFORM3(NODE_13,NODE_12,3,DK_SUP_V,"01>2*")

	/* Stomp */
	/* C21 is discharged via Q5 BE */
	DISCRETE_RCDISC_MODULATED(NODE_15,DS_SOUND2_INV,0,DK_R10,0,0,DK_R9,DK_C21,DK_SUP_V)
	/* Q5 */
	DISCRETE_TRANSFORM2(NODE_16, NODE_15, 0.6, "01>")
	DISCRETE_RCDISC2(NODE_17,NODE_16,DK_SUP_V,DK_R8+DK_R7,0.0,DK_R7,DK_C20)

	DISCRETE_DIODE_MIXER2(NODE_20, NODE_17, NODE_13, &dkong_diode_mix_table) /* D1, D2 + D3 */

	DISCRETE_RCINTEGRATE(NODE_22,NODE_20,DK_R5, RES_2_PARALLEL(DK_R4+DK_R3,DK_R6),0,DK_C19,DK_SUP_V,DISC_RC_INTEGRATE_TYPE1)
	DISCRETE_MULTIPLY(DS_OUT_SOUND0,NODE_22,DK_R3/R_SERIES(DK_R3,DK_R4))
	DISCRETE_TASK_END()

	/************************************************/
	/* Jump                                         */
	/************************************************/
	/*  tt */
	/* 4049B Inverter Oscillator build from 3 inverters */
	DISCRETE_TASK_START(1)
	DISCRETE_INVERTER_OSC(NODE_25,1,0,DK_R38,DK_R39,DK_C26,0,&dkong_inverter_osc_desc_jump)

#if DK_USE_CUSTOM
	/* custom mixer for 555 CV voltage */
	DISCRETE_CUSTOM8(NODE_28, dkong_custom_mixer, DS_SOUND1_INV, NODE_25,
				DK_R32, DK_R50, DK_R51, DK_R49, DK_C24, DK_SUP_V, NULL)
#else
	DISCRETE_LOGIC_INVERT(DS_SOUND1,DS_SOUND1_INV)
	DISCRETE_MULTIPLY(NODE_24,DS_SOUND1,DK_SUP_V)
	DISCRETE_TRANSFORM3(NODE_26,DS_SOUND1,DK_R32,DK_R49+DK_R50,"01*2+")
	DISCRETE_MIXER4(NODE_28, 1, NODE_24, NODE_25, DK_SUP_V, 0,&dkong_rc_jump_desc)
#endif
	/* 555 Voltage controlled */
	DISCRETE_555_ASTABLE_CV(NODE_29, 1, RES_K(47), RES_K(27), CAP_N(47), NODE_28,
							&dkong_555_vco_desc)

	/* Jump trigger */
	DISCRETE_RCDISC_MODULATED(NODE_33,DS_SOUND1_INV,0,DK_R32,0,0,DK_R31,DK_C18,DK_SUP_V)

	DISCRETE_TRANSFORM2(NODE_34, NODE_33, 0.6, "01>")
	DISCRETE_RCDISC2(NODE_35, NODE_34,DK_SUP_V,R_SERIES(DK_R30,DK_R29),0.0,DK_R29,DK_C17)

	DISCRETE_DIODE_MIXER2(NODE_38, NODE_35, NODE_29, &dkong_diode_mix_table)

	DISCRETE_RCINTEGRATE(NODE_39,NODE_38,DK_R27, RES_2_PARALLEL(DK_R28,DK_R26+DK_R25),0,DK_C16,DK_SUP_V,DISC_RC_INTEGRATE_TYPE1)
	DISCRETE_MULTIPLY(DS_OUT_SOUND1,NODE_39,DK_R25/(DK_R26+DK_R25))
	DISCRETE_TASK_END()

	/************************************************/
	/* Walk                                         */
	/************************************************/
	DISCRETE_TASK_START(1)
	DISCRETE_INVERTER_OSC(NODE_51,1,0,DK_R47,DK_R48,DK_C30,0,&dkong_inverter_osc_desc_walk)

#if DK_USE_CUSTOM
	/* custom mixer for 555 CV voltage */
	DISCRETE_CUSTOM8(NODE_54, dkong_custom_mixer, DS_SOUND0_INV, NODE_51,
				DK_R36, DK_R45, DK_R46, DK_R44, DK_C29, DK_SUP_V, NULL)
#else
	DISCRETE_LOGIC_INVERT(DS_SOUND0,DS_SOUND0_INV)
	DISCRETE_MULTIPLY(NODE_50,DS_SOUND0,DK_SUP_V)
	DISCRETE_TRANSFORM3(NODE_52,DS_SOUND0,DK_R46,R_SERIES(DK_R44,DK_R45),"01*2+")
	DISCRETE_MIXER4(NODE_54, 1, NODE_50, NODE_51, DK_SUP_V, 0,&dkong_rc_walk_desc)
#endif

	/* 555 Voltage controlled */
	DISCRETE_555_ASTABLE_CV(NODE_55, 1, RES_K(47), RES_K(27), CAP_N(33), NODE_54, &dkong_555_vco_desc)
	/* Trigger */
	DISCRETE_RCDISC_MODULATED(NODE_60,DS_SOUND0_INV,NODE_55,DK_R36,DK_R18,DK_R35,DK_R17,DK_C25,DK_SUP_V)
	/* Filter and divide - omitted C22 */
	DISCRETE_CRFILTER(NODE_61, NODE_60, DK_R15+DK_R16, DK_C23)
	DISCRETE_MULTIPLY(DS_OUT_SOUND2, NODE_61, DK_R15/(DK_R15+DK_R16))
	DISCRETE_TASK_END()

	/************************************************/
	/* DAC                                          */
	/************************************************/

	DISCRETE_TASK_START(1)
	/* Mixing - DAC */
	DISCRETE_ADJUSTMENT(DS_ADJ_DAC, 0, 1, DISC_LINADJ, "VR2")

	/* Buffer DAC first to input stream 0 */
	DISCRETE_INPUT_BUFFER(DS_DAC, 0)
	//DISCRETE_INPUT_DATA(DS_DAC)
	/* Signal decay circuit Q7, R20, C32 */
	DISCRETE_RCDISC(NODE_70, DS_DISCHARGE_INV, 1, DK_R20, DK_C32)
	DISCRETE_TRANSFORM4(NODE_71, DS_DAC,  DK_SUP_V/256.0, NODE_70, DS_DISCHARGE_INV, "01*3!2+*")

	/* following the DAC are two opamps. The first is a current-to-voltage changer
	 * for the DAC08 which delivers a variable output current.
	 *
	 * The second one is a Sallen Key filter ...
	 * http://www.t-linespeakers.org/tech/filters/Sallen-Key.html
	 * f = w / 2 / pi  = 1 / ( 2 * pi * 5.6k*sqrt(22n*10n)) = 1916 Hz
	 * Q = 1/2 * sqrt(22n/10n)= 0.74
	 */
	DISCRETE_SALLEN_KEY_FILTER(NODE_73, 1, NODE_71, DISC_SALLEN_KEY_LOW_PASS, &dkong_sallen_key_info)

	/* Adjustment VR2 */
#if DK_NO_FILTERS
	DISCRETE_MULTIPLY(DS_OUT_DAC, NODE_71, DS_ADJ_DAC)
#else
	DISCRETE_MULTIPLY(DS_OUT_DAC, NODE_73, DS_ADJ_DAC)
#endif
	DISCRETE_TASK_END()

	/************************************************/
	/* Amplifier                                    */
	/************************************************/

	DISCRETE_TASK_START(2)

	DISCRETE_MIXER4(NODE_288, 1, DS_OUT_SOUND0, DS_OUT_SOUND1, DS_OUT_DAC, DS_OUT_SOUND2, &dkong_mixer_desc)

	/* Amplifier: internal amplifier */
	DISCRETE_ADDER2(NODE_289,1,NODE_288,5.0*43.0/(100.0+43.0))
	DISCRETE_RCINTEGRATE(NODE_294,NODE_289,0,150,1000, CAP_U(33),DK_SUP_V,DISC_RC_INTEGRATE_TYPE3)
	DISCRETE_CRFILTER(NODE_295,NODE_294, RES_K(50), DK_C13)
	/*DISCRETE_CRFILTER(NODE_295,1,NODE_294, 1000, DK_C13) */
	/* EZV20 equivalent filter circuit ... */
	DISCRETE_CRFILTER(NODE_296,NODE_295, RES_K(1), CAP_U(4.7))
#if DK_NO_FILTERS
	DISCRETE_OUTPUT(NODE_288, 32767.0/5.0 * 10)
#else
	DISCRETE_OUTPUT(NODE_296, 32767.0/5.0 * 3.41)
	/* Test */
	//DISCRETE_CSVLOG2(NODE_296, NODE_288)
	//DISCRETE_WAVELOG1(NODE_296, 32767.0/5.0 * 3.41)
#endif
	DISCRETE_TASK_END()

DISCRETE_SOUND_END

/****************************************************************
 *
 * radarscp Discrete Sound Interface
 *
 ****************************************************************/

#define RS_R1       RES_K(10)
#define RS_R2       RES_K(10)
#define RS_R3       RES_K(5.1)
#define RS_R4       RES_K(2)
#define RS_R5       750
#define RS_R6       RES_K(4.7)
#define RS_R7       RES_K(10)
#define RS_R8       RES_K(100)
#define RS_R9       RES_K(10)
#define RS_R14      RES_K(10)
#define RS_R15      RES_K(5.6)  /* ???? */
#define RS_R16      RES_K(5.6)
#define RS_R18      RES_K(4.7)
#define RS_R22      RES_K(5.6)
#define RS_R23      RES_K(5.6)
#define RS_R25      RES_K(10)
#define RS_R26      RES_K(5.1)
#define RS_R27      RES_K(2) /* 10k in schematics - but will oscillate */
#define RS_R28      150
#define RS_R29      RES_K(4.7)
#define RS_R30      RES_K(10)
#define RS_R31      RES_K(100)
#define RS_R32      RES_K(10)
#define RS_R37      RES_K(1)
#define RS_R38      RES_K(1)
#define RS_R39      RES_K(1)
#define RS_R40      RES_K(10)
#define RS_R42      RES_K(10)
#define RS_R43      RES_K(5.1)
#define RS_R44      RES_K(3.9)
#define RS_R46      RES_K(1)
#define RS_R48      RES_K(18)
#define RS_R49      RES_M(3.3)
#define RS_R54      RES_K(1.2)
#define RS_R55      RES_K(10)
#define RS_R56      RES_K(12)
#define RS_R57      RES_K(4.3) /* ??? 43 */
#define RS_R58      RES_K(43)
#define RS_R59      RES_K(1.2)
#define RS_R60      RES_K(10)
#define RS_R61      RES_K(20)
#define RS_R62      RES_K(2)
#define RS_R63      130

#define RS_R_NN01   RES_K(10)
#define RS_R_NN02   RES_K(10)

#define RS_C5       CAP_U(220)
#define RS_C18      CAP_U(1)
#define RS_C19      CAP_U(22)
#define RS_C20      CAP_U(1)
#define RS_C22      CAP_U(47)
#define RS_C29      CAP_U(1)
#define RS_C30      CAP_U(10)
#define RS_C31      CAP_U(1)
#define RS_C33      CAP_U(4.7)
#define RS_C38      CAP_N(10)
#define RS_C40      CAP_U(10)
#define RS_C45      CAP_U(22)
#define RS_C46      CAP_U(1)
#define RS_C47      CAP_U(22)
#define RS_C48      CAP_N(33)
#define RS_C49      CAP_N(10)
#define RS_C50      CAP_U(3.3)
#define RS_C51      CAP_U(3.3)
#define RS_C53      CAP_U(3.3)
#define RS_C54      CAP_U(1)

#define RS_VR2      RES_K(10)
#define RS_C2       CAP_U(1)


static const discrete_mixer_desc radarscp_mixer_desc =
	{DISC_MIXER_IS_RESISTOR,
		{RS_R14, RS_R25, RS_R2, RS_R42, RS_R1},
		{0,0,0,0,0},    /* no variable resistors */
		{0,0,0,0,0},  /* no node capacitors */
		0, RS_VR2,
		0,
		RS_C2,
		0, 1};

static const discrete_mixer_desc radarscp_mixer_desc_0 =
	{DISC_MIXER_IS_RESISTOR,
		{RS_R56+RS_R54,NE555_INTERNAL_R,RES_2_PARALLEL(2*NE555_INTERNAL_R,RS_R55) },
		{0,0,0},
		{0,0,0,0},  /* no node capacitors */
		0, 0,
		RS_C51,
		0,
		0, 1};

static const discrete_mixer_desc radarscp_mixer_desc_7 =
	{DISC_MIXER_IS_RESISTOR,
		{RS_R63+RS_R59, NE555_INTERNAL_R,RES_2_PARALLEL(2*NE555_INTERNAL_R,RS_R60)},
		{0,0,0},    /* no variable resistors */
		{0,0,0},  /* no node capacitors */
		0, 0,
		RS_C50,
		0,
		0, 1};

/* There is no load on the output for the jump circuit
 * For the walk circuit, the voltage does not matter */

#define radarscp_555_vco_desc  dkong_555_vco_desc

static const discrete_dss_inverter_osc_node::description radarscp_inverter_osc_desc_0 =
	{DEFAULT_CD40XX_VALUES(DK_SUP_V),
	discrete_dss_inverter_osc_node::IS_TYPE2
	};

static const discrete_dss_inverter_osc_node::description radarscp_inverter_osc_desc_7 =
	{DEFAULT_CD40XX_VALUES(DK_SUP_V),
	discrete_dss_inverter_osc_node::IS_TYPE3
	};

static DISCRETE_SOUND_START(radarscp)

	/************************************************/
	/* Input register mapping for radarscp          */
	/************************************************/

	/* DISCRETE_INPUT_DATA */
	DISCRETE_INPUT_NOT(DS_SOUND0_INV)
	DISCRETE_INPUT_NOT(DS_SOUND1_INV)
	DISCRETE_INPUT_NOT(DS_SOUND2_INV)
	DISCRETE_INPUT_NOT(DS_SOUND6_INV)
	DISCRETE_INPUT_NOT(DS_SOUND7_INV)
	DISCRETE_INPUT_NOT(DS_DISCHARGE_INV)

	/* Must be in task if tasks added */
	DISCRETE_INPUT_BUFFER(DS_DAC, 0)
	//DISCRETE_INPUT_DATA(DS_DAC)

	/* Mixing - DAC */
	DISCRETE_ADJUSTMENT(DS_ADJ_DAC, 0, 1, DISC_LINADJ, "VR2")

	/************************************************/
	/* SIGNALS                                      */
	/************************************************/

	DISCRETE_LOGIC_INVERT(DS_SOUND6,DS_SOUND6_INV)
	DISCRETE_LOGIC_INVERT(DS_SOUND7,DS_SOUND7_INV)

	/************************************************/
	/* Noise                                      */
	/************************************************/

	DISCRETE_LFSR_NOISE(NODE_11, 1, 1, CLOCK_2VF, 1.0, 0, 0.5, &dkong_lfsr)
	/* Clear (1) from SOUND6 */
	DISCRETE_COUNTER(NODE_12, 1, DS_SOUND6_INV, NODE_11, 0, 15, DISC_COUNT_UP, 0, DISC_CLK_ON_R_EDGE)   /* LS161, IC 3J */
	DISCRETE_TRANSFORM3(NODE_13,NODE_12,0x04,DK_SUP_V,"01&1=2*")  /*QC => SND02 */
	DISCRETE_TRANSFORM3(NODE_14,NODE_12,0x02,DK_SUP_V,"01&1=2*")  /*QB => SND01 */

	/************************************************/
	/* SOUND2                                       */
	/************************************************/

	/* C21 is discharged via Q5 BE */

	DISCRETE_RCDISC_MODULATED(NODE_16,DS_SOUND2_INV,0,RS_R_NN01,0,0,RS_R9*2,RS_C20,DK_SUP_V)
	DISCRETE_TRANSFORM2(NODE_17, NODE_16, 0.6, "01>") /* TR2 */
	DISCRETE_RCDISC2(NODE_18,NODE_17,DK_SUP_V,RS_R8+RS_R7,0.0,RS_R7,RS_C19)

	DISCRETE_DIODE_MIXER2(NODE_20, NODE_18, NODE_13, &dkong_diode_mix_table) /* D1, D2 + D3 */

	DISCRETE_RCINTEGRATE(NODE_22,NODE_20,RS_R5, RES_2_PARALLEL(RS_R4+RS_R3,RS_R6),0,RS_C18,DK_SUP_V,DISC_RC_INTEGRATE_TYPE1)
	DISCRETE_MULTIPLY(DS_OUT_SOUND2,NODE_22,RS_R3/R_SERIES(RS_R3,RS_R4))

	/************************************************/
	/* SOUND1                                       */
	/************************************************/

	/* C21 is discharged via Q5 BE */

	DISCRETE_RCDISC_MODULATED(NODE_26,DS_SOUND1_INV,0,RS_R_NN02,0,0,RS_R32,RS_C31,DK_SUP_V)
	DISCRETE_TRANSFORM2(NODE_27, NODE_26, 0.6, "01>") /* TR5 */
	DISCRETE_RCDISC2(NODE_28,NODE_27,DK_SUP_V,RS_R31+RS_R30,0.0,RS_R30,RS_C30)

	DISCRETE_DIODE_MIXER2(NODE_30, NODE_28, NODE_14, &dkong_diode_mix_table) /* D1, D2 + D3 */

	DISCRETE_RCINTEGRATE(NODE_31,NODE_30,RS_R28, RES_2_PARALLEL(RS_R27+RS_R26,RS_R29),0,RS_C29,DK_SUP_V,DISC_RC_INTEGRATE_TYPE1)
	DISCRETE_MULTIPLY(DS_OUT_SOUND1,NODE_31,RS_R26/R_SERIES(RS_R26,RS_R27))

	/************************************************/
	/* SOUND0                                       */
	/************************************************/

	DISCRETE_INVERTER_OSC(NODE_41,1,0,RS_R57,RS_R58,RS_C53,0,&radarscp_inverter_osc_desc_0)
	DISCRETE_MIXER3(NODE_42, 1, NODE_41, DK_SUP_V, 0,&radarscp_mixer_desc_0)

	/* 555 Voltage controlled */
	DISCRETE_555_ASTABLE_CV(NODE_43, DS_SOUND6, RES_K(47), RES_K(27), RS_C49, NODE_42, &radarscp_555_vco_desc)

	DISCRETE_RCDISC_MODULATED(NODE_44,DS_SOUND0_INV,NODE_43,RS_R39,RS_R18,RS_R37,RS_R38,RS_C22,DK_SUP_V)
	DISCRETE_CRFILTER(NODE_45, NODE_44, RS_R15+RS_R16, RS_C33)
	DISCRETE_MULTIPLY(DS_OUT_SOUND0, NODE_45, RS_R15/(RS_R15+RS_R16))

	/************************************************/
	/* SOUND7                                       */
	/************************************************/

	DISCRETE_INVERTER_OSC(NODE_51,1,0,RS_R62,RS_R61,RS_C54,0,&radarscp_inverter_osc_desc_0)
	/* inverter osc used as sine wave generator */
	DISCRETE_INVERTER_OSC(NODE_52,1,0,RS_R48,RS_R49,RS_C47,0,&radarscp_inverter_osc_desc_7)

	DISCRETE_MIXER3(NODE_53, 1, NODE_51, DK_SUP_V, 0,&radarscp_mixer_desc_7)
	/* 555 Voltage controlled */
	DISCRETE_555_ASTABLE_CV(NODE_54, DS_SOUND7, RES_K(47), RES_K(27), RS_C48, NODE_53, &radarscp_555_vco_desc)

	DISCRETE_RCINTEGRATE(NODE_55,NODE_52,RS_R46, RS_R46,0,RS_C45,DK_SUP_V,DISC_RC_INTEGRATE_TYPE1)
	DISCRETE_TRANSFORM4(NODE_56, NODE_55, DS_SOUND7,NODE_54,2.5, "01*23<*")
	DISCRETE_CRFILTER(NODE_57, NODE_56, RS_R43+RS_R44, RS_C46)
	DISCRETE_MULTIPLY(DS_OUT_SOUND7, NODE_57, RS_R44/(RS_R43+RS_R44))

	/************************************************/
	/* DAC                                          */
	/************************************************/
	/* Signal decay circuit Q7, R20, C32 */
	DISCRETE_RCDISC(NODE_170, DS_DISCHARGE_INV, 1, RS_R40, RS_C40)
	DISCRETE_TRANSFORM4(NODE_171, DS_DAC,  DK_SUP_V/256.0, NODE_170, DS_DISCHARGE_INV, "01*3!2+*")

	/* following the DAC are two opamps. The first is a current-to-voltage changer
	 * for the DAC08 which delivers a variable output current.
	 *
	 * The second one is a Sallen Key filter ...
	 * http://www.t-linespeakers.org/tech/filters/Sallen-Key.html
	 * f = w / 2 / pi  = 1 / ( 2 * pi * 5.6k*sqrt(22n*10n)) = 1916 Hz
	 * Q = 1/2 * sqrt(22n/10n)= 0.74
	 */
	DISCRETE_SALLEN_KEY_FILTER(NODE_173, 1, NODE_171, DISC_SALLEN_KEY_LOW_PASS, &dkong_sallen_key_info)

	/* Adjustment VR3 */
	DISCRETE_MULTIPLY(DS_OUT_DAC, NODE_173, DS_ADJ_DAC)

	/************************************************/
	/* Amplifier                                    */
	/************************************************/

	DISCRETE_MIXER5(NODE_288, 1, DS_OUT_SOUND0, DS_OUT_SOUND1, DS_OUT_SOUND2, DS_OUT_SOUND7, DS_OUT_DAC, &radarscp_mixer_desc)

	/* Amplifier: internal amplifier */
	DISCRETE_ADDER2(NODE_289,1,NODE_288,5.0*43.0/(100.0+43.0))
	DISCRETE_RCINTEGRATE(NODE_294,NODE_289,0,150,1000, CAP_U(33),DK_SUP_V,DISC_RC_INTEGRATE_TYPE3)
	DISCRETE_CRFILTER(NODE_295,NODE_294, 1000, DK_C13)
	DISCRETE_OUTPUT(NODE_295, 32767.0/5.0 * 3)

DISCRETE_SOUND_END

/****************************************************************
 *
 * DkongJR Discrete Sound Interface
 *
 ****************************************************************/

#define JR_R2       120
#define JR_R3       RES_K(100)
#define JR_R4       RES_K(47)
#define JR_R5       RES_K(150)
#define JR_R6       RES_K(20)
#define JR_R8       RES_K(47)
#define JR_R9       RES_K(47)
#define JR_R10      RES_K(10)
#define JR_R11      RES_K(20)
#define JR_R12      RES_K(10)
#define JR_R13      RES_K(47)
#define JR_R14      RES_K(30)
#define JR_R17      RES_K(47)
#define JR_R18      RES_K(100)
#define JR_R19      (100)
#define JR_R20      RES_K(10)
#define JR_R24      RES_K(4.7)
#define JR_R25      RES_K(47)
#define JR_R27      RES_K(10)
#define JR_R28      RES_K(100)
#define JR_R33      RES_K(1)
#define JR_R34      RES_K(1)
#define JR_R35      RES_K(1)


#define JR_C13      CAP_U(4.7)
#define JR_C14      CAP_U(4.7)
#define JR_C15      CAP_U(22)
#define JR_C16      CAP_U(3.3)
#define JR_C17      CAP_U(3.3)
#define JR_C18      CAP_N(22)
#define JR_C19      CAP_N(4.7)
#define JR_C20      CAP_U(0.12)
#define JR_C21      CAP_N(56)
#define JR_C22      CAP_N(220)
#define JR_C23      CAP_U(0.47)
#define JR_C24      CAP_U(47)
#define JR_C25      CAP_U(1)
#define JR_C26      CAP_U(47)
#define JR_C27      CAP_U(22)
#define JR_C28      CAP_U(10)
#define JR_C29      CAP_U(10)
#define JR_C30      CAP_U(0.47)
#define JR_C32      CAP_U(10)
#define JR_C37      CAP_U(0.12)
#define JR_C39      CAP_U(0.47)
#define JR_C161     CAP_U(1)
#define JR_C155     CAP_U(0.01)

#define TTL_HIGH    (4)
#define GND         (0)


/* KT = 0.25 for diode circuit, 0.33 else */

#define DISCRETE_LS123(_N, _T, _R, _C) \
	DISCRETE_ONESHOTR(_N, 0, _T, TTL_HIGH, (0.25 * (_R) * (_C) * (1.0+700./(_R))), DISC_ONESHOT_RETRIG | DISC_ONESHOT_REDGE)
#define DISCRETE_LS123_INV(_N, _T, _R, _C) \
	DISCRETE_ONESHOTR(_N, 0, _T, TTL_HIGH, (0.25 * (_R) * (_C) * (1.0+700./(_R))), DISC_ONESHOT_RETRIG | DISC_ONESHOT_REDGE | DISC_OUT_ACTIVE_LOW)

static const discrete_mixer_desc dkongjr_mixer_desc =
	{DISC_MIXER_IS_RESISTOR,
		{JR_R5, JR_R3, JR_R6, JR_R4, JR_R25},
		{0,0,0,0,0},    /* no variable resistors */
		{0,0,0,0,0},    /* no node capacitors */
		0, 0,
		JR_C155,        /* cF */
		JR_C161,        /* cAmp */
		0, 1};

static const discrete_mixer_desc dkongjr_s1_mixer_desc =
{
	DISC_MIXER_IS_RESISTOR,
	{JR_R13, JR_R12},
	{0}, {0}, 0, 0, JR_C24, 0, 0, 1     /* r_node{}, c{}, rI, rF, cF, cAmp, vRef, gain */
};

static const discrete_lfsr_desc dkongjr_lfsr =
{
	DISC_CLK_IS_FREQ,
	16,                   /* Bit Length */
	0,                    /* Reset Value */
	2,                    /* Use Bit 2 (QC of first LS164) as F0 input 0 */
	15,                   /* Use Bit 15 (QH of secong LS164) as F0 input 1 */
	DISC_LFSR_XOR,        /* F0 is XOR */
	DISC_LFSR_NOT_IN0,    /* F1 is inverted F0*/
	DISC_LFSR_REPLACE,    /* F2 replaces the shifted register contents */
	0x000001,             /* Everything is shifted into the first bit only */
	DISC_LFSR_FLAG_OUTPUT_F0 | DISC_LFSR_FLAG_OUT_INVERT, /* Output is result of F0 */
	0                     /* Output bit */
};

#define DS_SOUND9_EN    DS_SOUND9_INV

static DISCRETE_SOUND_START(dkongjr)

	/************************************************/
	/* Input register mapping for dkongjr           */
	/************************************************/

	/* DISCRETE_INPUT_DATA */
	DISCRETE_INPUT_NOT(DS_SOUND0_INV)       /* IC 6J, pin 2 */
	DISCRETE_INPUT_NOT(DS_SOUND1_INV)       /* IC 6J, pin 12 */
	DISCRETE_INPUT_NOT(DS_SOUND2_INV)       /* IC 6J, pin 4 */
	DISCRETE_INPUT_NOT(DS_SOUND6_INV)       /* unused */
	DISCRETE_INPUT_NOT(DS_SOUND7_INV)       /* IC 5J, pin 12 */
	DISCRETE_INPUT_LOGIC(DS_SOUND9_EN)      /* IC 7N pin 10 from IC 5J, pin 4 */
	DISCRETE_INPUT_NOT(DS_DISCHARGE_INV)    /* IC 7H, pin 38 */

	/************************************************
	 * SOUND0 - walking
	 ************************************************/

DISCRETE_TASK_START(1)
	DISCRETE_COUNTER(NODE_100,                  /* IC 6L */
		1, 0,                                   /* ENAB; RESET */
		NODE_118,                               /* CLK - IC 6L, pin 10 */
		0, 0x3FFF, DISC_COUNT_UP, 0, DISC_CLK_BY_COUNT | DISC_OUT_HAS_XTIME)

	DISCRETE_BIT_DECODE(NODE_101,               /* IC 6L, pin 6 */
		NODE_100,  6, 0)                        /* output x_time logic */
	DISCRETE_BIT_DECODE(NODE_102,               /* IC 6L, pin 7 */
		NODE_100,  3, 0)                        /* output x_time logic */
	DISCRETE_BIT_DECODE(NODE_103,               /* IC 6L, pin 2 */
		NODE_100, 12, 0)                        /* output x_time logic */
	DISCRETE_BIT_DECODE(NODE_104,               /* IC 6L, pin 1 */
		NODE_100, 11, 0)                        /* output x_time logic */

	/* LS157 Switches - IC 6K */
	DISCRETE_SWITCH(NODE_106,                       /* IC 6K, pin 7 */
		1, DS_SOUND7_INV,                           /* ENAB; IC 6K, pin 1 */
		NODE_101, NODE_102)                         /* IC 6K, pin 5; pin 6 */
	DISCRETE_SWITCH(NODE_107,                       /* IC 6K, pin 9 */
		1, DS_SOUND7_INV,                           /* ENAB; IC 6K, pin 1 */
		NODE_103, NODE_104)                         /* IC 6K, pin 11; pin 10 */

	DISCRETE_LS123(NODE_110,                        /* IC 4K, pin 5 */
		DS_SOUND0_INV,                              /* IC 4K, pin 10 */
		JR_R8, JR_C14)
	DISCRETE_SWITCH(NODE_111,                       /* IC 4F, pin 10 (inverter) */
		1, NODE_110,                                /* ENAB; IC 4F, pin 11 */
		4.14, 0.151)                                /* INP0; INP1 (measured) */

	/* Breadboarded measurements IC 5K, pin 7
	   D.R. Oct 2010
	    V       Hz
	    0.151   3139
	    0.25    2883
	    0.5     2820
	    0.75    3336
	    1       3805
	    2       6498
	    3       9796
	    4       13440
	    4.14    13980
	*/

	DISCRETE_74LS624(NODE_113,                      /* IC 5K, pin 7 */
		1,                                          /* ENAB */
		NODE_111, DK_SUP_V,                         /* VMOD - IC 5K, pin 2; VRNG */
		JR_C18, JR_R10, JR_C17, JR_R33,             /* C; R_FREQ_IN; C_FREQ_IN; R_RNG_IN */
		DISC_LS624_OUT_LOGIC_X)
	DISCRETE_SWITCH(NODE_105,                       /* IC 6K, pin 4 */
		1,                                          /* ENAB */
		DS_SOUND7_INV,                              /* SWITCH, IC 6K, pin 1 */
		GND, NODE_113)                              /* IC 6K, pin 2; pin 3 */

	DISCRETE_XTIME_XOR(NODE_115,                    /* IC 6N, pin 3 */
		NODE_105, NODE_106,                         /* IC 6N, pin 1; pin 2 */
		0, 0)                                       /* use x_time logic */

	DISCRETE_XTIME_INVERTER(NODE_116,               /* IC 5J, pin 8 */
		NODE_107,                                   /* IC 5J, pin 9 */
		0.135, 4.15)                                /* measured Low/High */

	/* Breadboarded measurements IC 5K, pin 10
	   D.R. Oct 2010
	    V       Hz
	    0.135   14450 - measured 74LS04 low
	    0.25    13320
	    0.5     12980
	    0.75    15150
	    1       17270
	    2       28230
	    3       41910
	    4       56950
	    4.15    59400 - measured 74LS04 high
	*/

	DISCRETE_74LS624(NODE_118,                      /* IC 5K, pin 10 */
		1,                                          /* ENAB */
		NODE_116, DK_SUP_V,                         /* VMOD - IC 5K, pin 1; VRNG */
		JR_C19, JR_R11, JR_C16, JR_R33,             /* C; R_FREQ_IN; C_FREQ_IN; R_RNG_IN */
		DISC_LS624_OUT_COUNT_F_X)
	DISCRETE_SWITCH(NODE_119, 1, NODE_110, 0, 1)    /* convert from voltage to x_time logic */
	DISCRETE_XTIME_NAND(DS_OUT_SOUND0,              /* IC 5N, pin 11 */
		NODE_119, NODE_115,                         /* IC 5N, pin 13; pin 12 */
		0.2, 4.9)                                   /* LOW; HIGH (1k pullup to 5V) */
DISCRETE_TASK_END()

	/************************************************
	 * SOUND1  - Jump
	 ************************************************/

DISCRETE_TASK_START(2)
	/* needs NODE_104 from TASK(1) ready */
	DISCRETE_LS123(NODE_10,                         /* IC 4K, pin 13 */
		DS_SOUND1_INV,                              /* IC 4K, pin 8 */
		JR_R9, JR_C15)
	DISCRETE_SWITCH(NODE_11,                        /* IC 7N, pin 6 */
		1, NODE_10,                                 /* ENAB; SWITCH - IC 7N, pin 5 */
		0.151, 4.14)                                /* measured Low/High */
	DISCRETE_XTIME_INVERTER(NODE_12,                /* IC 7N, pin 4 */
		NODE_104,                                   /* IC 7N, pin 3 */
		0.151, 4.14)                                /* measured Low/High */
	DISCRETE_MIXER2(NODE_13, 1, NODE_11, NODE_12, &dkongjr_s1_mixer_desc)

	/* Breadboarded measurements IC 8L, pin 10
	   D.R. Oct 2010
	    V       Hz
	    0.151   313
	    0.25    288
	    0.5     275
	    0.75    324
	    1       370
	    2       635
	    3       965
	    4       1325
	    4.14    1378
	*/

	DISCRETE_74LS624(NODE_14,                       /* IC 8L, pin 10 */
		1,                                          /* ENAB */
		NODE_13, DK_SUP_V,                          /* VMOD - IC 8L, pin 1, VRNG */
		/* C_FREQ_IN is taken care of by the NODE_13 mixer */
		JR_C22, RES_2_PARALLEL(JR_R13, JR_R12), 0, JR_R35,  /* C; R_FREQ_IN; C_FREQ_IN; R_RNG_IN */
		DISC_LS624_OUT_ENERGY)

	DISCRETE_LOGIC_INVERT(NODE_15, NODE_10)         /* fake invert for NODE_16 */
	DISCRETE_RCDISC_MODULATED(NODE_16,              /* Q3, collector */
		NODE_15, NODE_14, 120, JR_R27, RES_K(0.001), JR_R28, JR_C28, DK_SUP_V)
	/* The following circuit does not match 100%, however works.
	 * To be exact, we need a C-R-C-R circuit, we actually do not have.
	 */
	DISCRETE_CRFILTER_VREF(NODE_17, NODE_16, JR_R4, JR_C23, 2.5)
	DISCRETE_RCFILTER(DS_OUT_SOUND1, NODE_17, JR_R19, JR_C21)
DISCRETE_TASK_END()

	/************************************************
	 * SOUND2 - climbing
	 ************************************************/

DISCRETE_TASK_START(1)
	/* the noise source clock is a 74LS629 IC 7P, pin 10.
	 * using JR_C20 as the timing cap, with Freq Control tied to 0V
	 * and Range tied to 5V.  This creates a fixed frequency of 710Hz.
	 * So for speed, I breadboarded and measured the frequency.
	 * Oct 2009, D.R.
	 */
	DISCRETE_LFSR_NOISE(NODE_21, 1, 1, 710, 1.0, 0, 0.5, &dkongjr_lfsr)     /* IC 3J & 4J */
	DISCRETE_LS123_INV(NODE_25,                     /* IC 8N, pin 13 (fake inverted for use by NODE_26) */
		DS_SOUND2_INV,                              /* IC 8N, pin 8 */
		JR_R17, JR_C27)
	DISCRETE_RCDISC_MODULATED(NODE_26,              /* Q2, collector */
		NODE_25, NODE_21, 120, JR_R24, RES_K(0.001), JR_R18, JR_C29, DK_SUP_V)
	/* The following circuit does not match 100%, however works.
	 * To be exact, we need a C-R-C-R circuit, we actually do not have.
	 */
	DISCRETE_CRFILTER_VREF(NODE_27, NODE_26, JR_R6, JR_C30, 2.5)
	DISCRETE_RCFILTER(DS_OUT_SOUND2, NODE_27, JR_R2, JR_C25)
DISCRETE_TASK_END()

	/************************************************
	 * SOUND9 - Falling
	 ************************************************/

DISCRETE_TASK_START(1)
	DISCRETE_XTIME_INVERTER(NODE_90,        /* IC 7N, pin 8 */
		DS_SOUND9_EN,                       /* IC 7N, pin 9 */
		0.134, 4.16)                        /* measured Low/High */

	/* Breadboarded measurements IC 7P, pin 7
	   D.R. Oct 2010
	    V       Hz
	    0.134   570
	    0.25    538
	    0.5     489
	    0.75    560
	    1       636
	    2       1003
	    3       1484
	    4       2016
	    4.16    2111
	*/
	DISCRETE_74LS624(NODE_91,               /* IC 7P, pin 7 */
		1,                                  /* ENAB */
		NODE_90, DK_SUP_V,                  /* VMOD - IC 7P, pin 2, VRNG */
		JR_C37, JR_R14, JR_C26, JR_R34,     /* C; R_FREQ_IN; C_FREQ_IN; R_RNG_IN */
		DISC_LS624_OUT_LOGIC_X)
	DISCRETE_XTIME_NAND(DS_OUT_SOUND9,      /* IC 5N, pin 8 */
		DS_SOUND9_EN,                       /* IC 5N, pin 9 */
		NODE_91,                            /* IC 5N, pin 10 */
		0.2, 4.9)                           /* LOW, HIGH (1k pullup to 5V) */
DISCRETE_TASK_END()

	/************************************************
	 * DAC
	 ************************************************/

DISCRETE_TASK_START(1)
	DISCRETE_INPUT_BUFFER(DS_DAC, 0)
	/* Signal decay circuit Q7, R20, C32 */
	DISCRETE_RCDISC(NODE_170, DS_DISCHARGE_INV, 1, JR_R20, JR_C32)
	DISCRETE_TRANSFORM4(NODE_171, DS_DAC,  DK_SUP_V/256.0, NODE_170, DS_DISCHARGE_INV, "01*3!2+*")

	/* following the DAC are two opamps. The first is a current-to-voltage changer
	 * for the DAC08 which delivers a variable output current.
	 *
	 * The second one is a Sallen Key filter ...
	 * http://www.t-linespeakers.org/tech/filters/Sallen-Key.html
	 * f = w / 2 / pi  = 1 / ( 2 * pi * 5.6k*sqrt(22n*10n)) = 1916 Hz
	 * Q = 1/2 * sqrt(22n/10n)= 0.74
	 */

	DISCRETE_SALLEN_KEY_FILTER(DS_OUT_DAC, 1, NODE_171, DISC_SALLEN_KEY_LOW_PASS, &dkong_sallen_key_info)
DISCRETE_TASK_END()

	/************************************************
	 * Amplifier
	 ************************************************/

DISCRETE_TASK_START(3)
	DISCRETE_MIXER5(NODE_288, 1, DS_OUT_SOUND9, DS_OUT_SOUND0, DS_OUT_SOUND2, DS_OUT_SOUND1, DS_OUT_DAC, &dkongjr_mixer_desc)

	/* Amplifier: internal amplifier
	 * Just a 1:n amplifier without filters - just the output filter
	 */
	DISCRETE_CRFILTER(NODE_295, NODE_288, 1000, JR_C13)
	/* approx -1.805V to 2.0V when playing, but turn on sound peaks at 2.36V */
	/* we will set the full wav range to 1.18V which will cause clipping on the turn on
	 * sound and explosions.  The real game would do this when the volume is turned up too.
	 * Reducing MAME's master volume to 50% will provide full unclipped volume.
	 */
	DISCRETE_OUTPUT(NODE_295, 32767.0/1.18)
DISCRETE_TASK_END()

DISCRETE_SOUND_END

/****************************************************************
 *
 * M58817 Speech
 *
 ****************************************************************/

/*

http://www.freepatentsonline.com/4633500.html


Addresses found at @0x510, cpu2

 10: 0000 00 00000000 ... 50 53 01010000 01010011 "scramble"
 12: 007a 44 01000100 ... 00 0f 00000000 00001111 "all pilots climb up"
 14: 018b 13 00010011 ... dc f0 11011100 11110000
 16: 0320 91 10010001 ... 00 f0 00000000 11110000
 18: 036c 42 01000010 ... 00 3C 00000000 00111100
 1A: 03c4 32 00110010 ... 03 C0 00000011 11000000
 1C: 041c 34 00110100 ... 07 80 00000111 10000000
 1E: 0520 52 01010010 ... 07 80 81 00000111 10000000 10000001
 20: 063e a3 10100011 ... 03 C0 00000011 11000000

 sample length ...

 122
 273
 405
 76
 88
 88
 260
 286
 271

 Samples
 0: 14 16       ... checkpoint charlie
 1: 14 18       ... checkpoint bravo
 2: 14 1A       ... checkpoint alpha
 3: 1C          Use Caution (sounds kinda like 'You'll notice')
 4: 1E 1E       Complete attack mission
 5: 10 10 10    trouble, trouble, trouble
 6: 12 12       all pilots climb up
 7: 20          engine trouble

 PA5   ==> CS 28
 PA4   ==> PDC 2
 PA0   ==> CTL1 25
 PA1   ==> CTL2 23
 PA2   ==> CTL4 20
 PA3   ==> CTL8 27
 M1 19 ==> PA6         M1 on TMS5100

 12,13 Speaker
  7,8  Xin, Xout  (5100: RC-OSC, T11)
  24 A0 (5100: ADD1)
  22 A1 (5100: ADD2)
  22 A2 (5100: ADD4)
  26 A3 (5100: ADD8)
  16 C0 (5100: NC)
  18 C1 (5100: NC)
  3  CLK  (5100: ROM-CK)

    For documentation purposes:

    Addresses
        { 0x0000, 0x007a, 0x018b, 0x0320, 0x036c, 0x03c4, 0x041c, 0x0520, 0x063e }
    and related samples interface

    static const char *const radarscp1_sample_names[] =
    {
        "*radarscp1",
        "10",
        "12",
        "14",
        "16",
        "18",
        "1A",
        "1C",
        "1E",
        "20",
        0
    };

    static const samples_interface radarscp1_samples_interface =
    {
        8,
        radarscp1_sample_names
    };

*/

WRITE8_MEMBER(dkong_state::M58817_command_w)
{
	m58817_device *m58817 = machine().device<m58817_device>("tms");
	m58817->ctl_w(space, 0, data & 0x0f);
	m58817->pdc_w((data>>4) & 0x01);
	/* FIXME 0x20 is CS */
}

READ8_MEMBER(dkong_state::M58817_status_r)
{
	m58817_device *m58817 = machine().device<m58817_device>("tms");
	return m58817->status_r(space, offset, mem_mask);
}


/****************************************************************
 *
 * I/O Handlers - static
 *
 ****************************************************************/

WRITE8_MEMBER(dkong_state::dkong_voice_w)
{
	/* only provided for documentation purposes
	 * not actually used
	 */
	logerror("dkong_speech_w: 0x%02x\n", data);
}

READ8_MEMBER(dkong_state::dkong_voice_status_r)
{
	/* only provided for documentation purposes
	 * not actually used
	 */
	return 0;
}

READ8_MEMBER(dkong_state::dkong_tune_r)
{
	latch8_device *m_ls175_3d = machine().device<latch8_device>("ls175.3d");
	UINT8 page = m_dev_vp2->read(space, 0) & 0x47;

	if ( page & 0x40 )
	{
		return (m_ls175_3d->read(space, 0) & 0x0F) | (dkong_voice_status_r(space, 0) << 4);
	}
	else
	{
		/* printf("%s:rom access\n",machine().describe_context()); */
		return (m_snd_rom[0x1000 + (page & 7) * 256 + offset]);
	}
}

WRITE8_MEMBER(dkong_state::dkong_p1_w)
{
	m_discrete->write(space,DS_DAC,data);
}


/****************************************************************
 *
 * I/O Handlers - global
 *
 ****************************************************************/

WRITE8_MEMBER(dkong_state::dkong_audio_irq_w)
{
	if (data)
		m_soundcpu->set_input_line(0, ASSERT_LINE);
	else
		m_soundcpu->set_input_line(0, CLEAR_LINE);
}


/*************************************
 *
 *  Sound CPU memory handlers
 *
 *************************************/

static ADDRESS_MAP_START( dkong_sound_map, AS_PROGRAM, 8, dkong_state )
	AM_RANGE(0x0000, 0x0fff) AM_ROM
ADDRESS_MAP_END

static ADDRESS_MAP_START( dkong_sound_io_map, AS_IO, 8, dkong_state )
	AM_RANGE(0x00, 0xFF) AM_READ(dkong_tune_r)
							AM_WRITE(dkong_voice_w)
	AM_RANGE(MCS48_PORT_BUS, MCS48_PORT_BUS) AM_READ(dkong_tune_r)
									AM_WRITE(dkong_voice_w)
	AM_RANGE(MCS48_PORT_P1, MCS48_PORT_P1) AM_WRITE(dkong_p1_w) /* only write to dac */
	AM_RANGE(MCS48_PORT_P2, MCS48_PORT_P2) AM_LATCH8_READWRITE("virtual_p2")
	AM_RANGE(MCS48_PORT_T0, MCS48_PORT_T0) AM_LATCH8_READBIT("ls259.6h", 5)
	AM_RANGE(MCS48_PORT_T1, MCS48_PORT_T1) AM_LATCH8_READBIT("ls259.6h", 4)
ADDRESS_MAP_END

static ADDRESS_MAP_START( dkongjr_sound_io_map, AS_IO, 8, dkong_state )
	AM_RANGE(0x00, 0x00) AM_MIRROR(0xff) AM_LATCH8_READ("ls174.3d")
	AM_RANGE(MCS48_PORT_P1, MCS48_PORT_P1) AM_WRITE(dkong_p1_w) /* only write to dac */
	AM_RANGE(MCS48_PORT_P2, MCS48_PORT_P2) AM_LATCH8_READWRITE("virtual_p2")
	AM_RANGE(MCS48_PORT_T0, MCS48_PORT_T0) AM_LATCH8_READBIT("ls259.6h", 5)
	AM_RANGE(MCS48_PORT_T1, MCS48_PORT_T1) AM_LATCH8_READBIT("ls259.6h", 4)
ADDRESS_MAP_END

static ADDRESS_MAP_START( radarscp1_sound_io_map, AS_IO, 8, dkong_state )
	AM_RANGE(0x00, 0x00) AM_MIRROR(0xff) AM_DEVREAD("ls175.3d", latch8_device, read)
	AM_RANGE(0x00, 0xff) AM_WRITE(dkong_p1_w) /* DAC here */
	AM_RANGE(MCS48_PORT_P1, MCS48_PORT_P1) AM_LATCH8_READ("virtual_p1")
									AM_WRITE(M58817_command_w)
	AM_RANGE(MCS48_PORT_P2, MCS48_PORT_P2) AM_LATCH8_WRITE("virtual_p2")
	AM_RANGE(MCS48_PORT_T0, MCS48_PORT_T0) AM_LATCH8_READBIT("ls259.6h", 5)
	AM_RANGE(MCS48_PORT_T1, MCS48_PORT_T1) AM_LATCH8_READBIT("ls259.6h", 4)
ADDRESS_MAP_END

static ADDRESS_MAP_START( dkong3_sound1_map, AS_PROGRAM, 8, dkong_state )
	AM_RANGE(0x0000, 0x01ff) AM_RAM
	AM_RANGE(0x4016, 0x4016) AM_LATCH8_READ("latch1")       /* overwrite default */
	AM_RANGE(0x4017, 0x4017) AM_LATCH8_READ("latch2")
	AM_RANGE(0xe000, 0xffff) AM_ROM
ADDRESS_MAP_END

static ADDRESS_MAP_START( dkong3_sound2_map, AS_PROGRAM, 8, dkong_state )
	AM_RANGE(0x0000, 0x01ff) AM_RAM
	AM_RANGE(0x4016, 0x4016) AM_LATCH8_READ("latch3")       /* overwrite default */
	AM_RANGE(0xe000, 0xffff) AM_ROM
ADDRESS_MAP_END

/*************************************
 *
 *  Machine driver
 *
 *************************************/

MACHINE_CONFIG_FRAGMENT( dkong2b_audio )

	/* sound latches */

	MCFG_LATCH8_ADD("ls175.3d") /* sound cmd latch */
	MCFG_LATCH8_MASKOUT(0xf0)
	MCFG_LATCH8_INVERT(0x0F)

	MCFG_LATCH8_ADD("ls259.6h")
	MCFG_LATCH8_WRITE_0(DEVWRITE8("discrete", discrete_device, write),DS_SOUND0_INP)
	MCFG_LATCH8_WRITE_1(DEVWRITE8("discrete", discrete_device, write),DS_SOUND1_INP)
	MCFG_LATCH8_WRITE_2(DEVWRITE8("discrete", discrete_device, write),DS_SOUND2_INP)
	MCFG_LATCH8_WRITE_6(DEVWRITE8("discrete", discrete_device, write),DS_SOUND6_INP)
	MCFG_LATCH8_WRITE_7(DEVWRITE8("discrete", discrete_device, write),DS_SOUND7_INP)

	/*   If P2.Bit7 -> is apparently an external signal decay or other output control
	 *   If P2.Bit6 -> activates the external compressed sample ROM (not radarscp1)
	 *   If P2.Bit5 -> Signal ANSN ==> Grid enable (radarscp1)
	 *   If P2.Bit4 -> status code to main cpu
	 *   P2.Bit2-0  -> select the 256 byte bank for external ROM
	 */

	MCFG_LATCH8_ADD( "virtual_p2" ) /* virtual latch for port B */
	MCFG_LATCH8_INVERT( 0x20 )      /* signal is inverted       */
	MCFG_LATCH8_READ_5(DEVREAD8("ls259.6h", latch8_device, read), 3)
	MCFG_LATCH8_WRITE_7(DEVWRITE8("discrete", discrete_device, write), DS_DISCHARGE_INV)

	MCFG_CPU_ADD("soundcpu", MB8884, I8035_CLOCK)
	MCFG_CPU_PROGRAM_MAP(dkong_sound_map)
	MCFG_CPU_IO_MAP(dkong_sound_io_map)

	MCFG_SPEAKER_STANDARD_MONO("mono")
	MCFG_DISCRETE_ADD("discrete", 0, dkong2b)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 1.0)

MACHINE_CONFIG_END

MACHINE_CONFIG_DERIVED( radarscp_audio, dkong2b_audio )
	MCFG_DISCRETE_REPLACE("discrete", 0, radarscp)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.7)

MACHINE_CONFIG_END


MACHINE_CONFIG_DERIVED( radarscp1_audio, radarscp_audio )
	MCFG_CPU_MODIFY("soundcpu")
	MCFG_CPU_IO_MAP(radarscp1_sound_io_map)

	/* virtual_p2 is not read -see memory map-, all bits are output bits */
	MCFG_LATCH8_ADD( "virtual_p1" ) /* virtual latch for port A */
	MCFG_LATCH8_INVERT( 0x80 )      /* signal is inverted       */
	MCFG_LATCH8_READ_7(DEVREAD8("ls259.6h", latch8_device, read), 3)
	MCFG_LATCH8_READ_6(READ8(dkong_state,M58817_status_r), 0)

	/* tms memory controller */
	MCFG_DEVICE_ADD("m58819", M58819, 0)

	MCFG_SOUND_ADD("tms", M58817, XTAL_640kHz)
	MCFG_TMS5110_M0_CB(DEVWRITELINE("m58819", tms6100_device, tms6100_m0_w))
	MCFG_TMS5110_M1_CB(DEVWRITELINE("m58819", tms6100_device, tms6100_m1_w))
	MCFG_TMS5110_ADDR_CB(DEVWRITE8("m58819", tms6100_device, tms6100_addr_w))
	MCFG_TMS5110_DATA_CB(DEVREADLINE("m58819", tms6100_device, tms6100_data_r))
	MCFG_TMS5110_ROMCLK_CB(DEVWRITELINE("m58819", tms6100_device, tms6100_romclock_w))
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 1.0)
MACHINE_CONFIG_END

MACHINE_CONFIG_FRAGMENT( dkongjr_audio )

	/* sound latches */

	MCFG_LATCH8_ADD("ls174.3d")
	MCFG_LATCH8_MASKOUT(0xE0)

	MCFG_LATCH8_ADD( "ls259.6h")
	MCFG_LATCH8_WRITE_0(DEVWRITE8("discrete", discrete_device, write), DS_SOUND0_INP)
	MCFG_LATCH8_WRITE_1(DEVWRITE8("discrete", discrete_device, write), DS_SOUND1_INP)
	MCFG_LATCH8_WRITE_2(DEVWRITE8("discrete", discrete_device, write), DS_SOUND2_INP)
	MCFG_LATCH8_WRITE_7(DEVWRITE8("discrete", discrete_device, write), DS_SOUND7_INP)

	MCFG_LATCH8_ADD( "ls259.5h")
	MCFG_LATCH8_WRITE_1(DEVWRITE8("discrete", discrete_device, write), DS_SOUND9_INP)

	MCFG_LATCH8_ADD( "ls259.4h")

	MCFG_LATCH8_ADD( "virtual_p2" ) /* virtual latch for port B */
	MCFG_LATCH8_INVERT( 0x70 )      /* all signals are inverted */
	MCFG_LATCH8_READ_6(DEVREAD8("ls259.4h", latch8_device, read), 1)
	MCFG_LATCH8_READ_5(DEVREAD8("ls259.6h", latch8_device, read), 3)
	MCFG_LATCH8_READ_4(DEVREAD8("ls259.6h", latch8_device, read), 6)
	MCFG_LATCH8_WRITE_7(DEVWRITE8("discrete", discrete_device, write), DS_DISCHARGE_INV)

	MCFG_CPU_ADD("soundcpu", MB8884, I8035_CLOCK)
	MCFG_CPU_PROGRAM_MAP(dkong_sound_map)
	MCFG_CPU_IO_MAP(dkongjr_sound_io_map)

	MCFG_SPEAKER_STANDARD_MONO("mono")

	MCFG_DISCRETE_ADD("discrete", 0, dkongjr)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 1.0)

MACHINE_CONFIG_END

MACHINE_CONFIG_FRAGMENT( dkong3_audio )

	MCFG_CPU_ADD("n2a03a", N2A03,N2A03_DEFAULTCLOCK)
	MCFG_CPU_PROGRAM_MAP(dkong3_sound1_map)
	MCFG_CPU_VBLANK_INT_DRIVER("screen", dkong_state,  nmi_line_pulse)

	MCFG_CPU_ADD("n2a03b", N2A03,N2A03_DEFAULTCLOCK)
	MCFG_CPU_PROGRAM_MAP(dkong3_sound2_map)
	MCFG_CPU_VBLANK_INT_DRIVER("screen", dkong_state,  nmi_line_pulse)

	/* sound latches */
	MCFG_LATCH8_ADD( "latch1")
	MCFG_LATCH8_ADD( "latch2")
	MCFG_LATCH8_ADD( "latch3")

	MCFG_SPEAKER_STANDARD_MONO("mono")
MACHINE_CONFIG_END
