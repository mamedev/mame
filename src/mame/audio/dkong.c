#include "driver.h"
#include "cpu/i8039/i8039.h"
#include "cpu/m6502/m6502.h"
#include "sound/nes_apu.h"
#include "sound/samples.h"
#include "sound/discrete.h"
#include "sound/dac.h"

#include "sound/tms5110.h"
#include "sound/5110intf.h"

#include "includes/dkong.h"

/****************************************************************
 *
 * Defines and Macros
 *
 ****************************************************************/

/* Set to 1 to disable DAC and post-mixer filters */
#define		DK_NO_FILTERS	(0)

/* Issue surrounded by this define need to be analyzed and
 * reviewed at a lator time.
 * Currently, the following issues exist:
 * - although not present on schematics, a 10K resistor is needed
 *   as RF in the mixer stage. Without this resistor, the DAC
 *   sound is completely overmodulated.
 */

// FIXME: Review at a later time
#define		DK_REVIEW		(1)

#define ACTIVELOW_PORT_BIT(P,A,D)   (((P) & (~(1 << (A)))) | (((D) ^ 1) << (A)))

/* Needed for dkongjr ... FIXME */
//#define I8035_T_R(N) ((portT >> (N)) & 1)
#define I8035_T_R(M,N) ((soundlatch2_r(machine,0) >> (N)) & 1)
#define I8035_T_W_AL(M,N,D) do { state->portT = ACTIVELOW_PORT_BIT(state->portT,N,D); soundlatch2_w(machine, 0, state->portT); } while (0)

#define I8035_P1_R(M) (soundlatch3_r(M,0))
#define I8035_P2_R(M) (soundlatch4_r(M,0))
#define I8035_P1_W(M,D) soundlatch3_w(M,0,D)
#define I8035_P2_W(M,D) soundlatch4_w(M,0,D)

#define I8035_P1_W_AL(M,B,D) I8035_P1_W(M,ACTIVELOW_PORT_BIT(I8035_P1_R(M),B,(D)))
#define I8035_P2_W_AL(M,B,D) I8035_P2_W(M,ACTIVELOW_PORT_BIT(I8035_P2_R(M),B,(D)))


/****************************************************************
 *
 * Discrete Sound defines
 *
 ****************************************************************/

/* Discrete sound inputs */

#define DS_SOUND0_INV		NODE_01
#define DS_SOUND1_INV		NODE_02
#define DS_SOUND2_INV		NODE_03
#define DS_SOUND6_INV		NODE_04
#define DS_SOUND7_INV		NODE_05
#define DS_SOUND9_INV		NODE_06
#define DS_DAC				NODE_07
#define DS_DAC_DISCHARGE	NODE_08

#define DS_SOUND0			NODE_208
#define DS_SOUND1			NODE_209
#define DS_SOUND2			NODE_210
#define DS_SOUND6			NODE_211
#define DS_SOUND7			NODE_212
#define DS_SOUND9			NODE_213

#define DS_ADJ_DAC			NODE_240

#define DS_OUT_SOUND0		NODE_241
#define DS_OUT_SOUND1		NODE_242
#define DS_OUT_SOUND2		NODE_243
#define DS_OUT_SOUND6		NODE_247
#define DS_OUT_SOUND7		NODE_248
#define DS_OUT_SOUND9		NODE_249
#define DS_OUT_DAC			NODE_250

/* Input definitions for write handlers */

#define DS_SOUND0_INP		DS_SOUND0_INV
#define DS_SOUND1_INP		DS_SOUND1_INV
#define DS_SOUND2_INP		DS_SOUND2_INV
#define DS_SOUND6_INP		DS_SOUND6_INV
#define DS_SOUND7_INP		DS_SOUND7_INV
#define DS_SOUND9_INP		DS_SOUND9_INV

/* General defines */

#define DK_1N5553_V			0.4	/* from datasheet at 1mA */
#define DK_SUP_V			5.0
#define NE555_INTERNAL_R	RES_K(5)

#define R_PARALLEL(R1,R2) ((R1)*(R2)/((R1)+(R2)))
#define R_SERIE(R1,R2)	  ((R1)+(R2))

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

#define DK_R1 		RES_K(47)
#define DK_R2 		RES_K(47)
#define DK_R3		RES_K(5.1)
#define DK_R4		RES_K(2)
#define DK_R5		750
#define DK_R6		RES_K(4.7)
#define DK_R7		RES_K(10)
#define DK_R8		RES_K(100)
#define DK_R9		RES_K(10)
#define DK_R10 		RES_K(10)
#define DK_R14 		RES_K(47)

#define DK_R15		RES_K(5.6)
#define DK_R16		RES_K(5.6)
#define DK_R17		RES_K(10)
#define DK_R18		RES_K(4.7)
#define DK_R20		RES_K(10)
#define DK_R21		RES_K(5.6)
#define DK_R22		RES_K(5.6)
#define DK_R24 		RES_K(47)
#define DK_R25		RES_K(5.1)
#define DK_R26		RES_K(2)
#define DK_R27		150
#define DK_R28		RES_K(4.7)
#define DK_R29		RES_K(10)
#define DK_R30		RES_K(100)
#define DK_R31 		RES_K(10)
#define DK_R32 		RES_K(10)
#define DK_R35		RES_K(1)
#define DK_R36		RES_K(1)
#define DK_R38		RES_K(18)
#define DK_R39		RES_M(3.3)
#define	DK_R49		RES_K(1.2)
#define	DK_R44		RES_K(1.2)
#define DK_R45		RES_K(10)
#define DK_R46		RES_K(12)
#define DK_R47		RES_K(4.3)
#define DK_R48		RES_K(43)
#define DK_R50		RES_K(10)
#define DK_R51		RES_K(10)

/* Capacitors */

#define DK_C8		CAP_U(220)
#define DK_C12		CAP_U(1)
#define DK_C13		CAP_U(33)
#define DK_C16		CAP_U(1)
#define DK_C17		CAP_U(4.7)
#define DK_C18		CAP_U(1)
#define DK_C19		CAP_U(1)
#define DK_C20		CAP_U(3.3)
#define DK_C21		CAP_U(1)

#define DK_C23		CAP_U(4.7)
#define	DK_C24		CAP_U(10)
#define DK_C25		CAP_U(3.3)
#define	DK_C26		CAP_U(3.3)
#define	DK_C29		CAP_U(3.3)
#define DK_C30		CAP_U(10)
#define DK_C32		CAP_U(10)
#define DK_C34		CAP_N(10)
#define DK_C159		CAP_N(100)


/*
 * The noice generator consists of three LS164 8+8+8
 * the output signal is taken after the xor, so
 * taking bit 0 is not exact
 */

static const discrete_lfsr_desc dkong_lfsr =
{
	DISC_CLK_IS_FREQ,
	24,			          /* Bit Length */
	0,			          /* Reset Value */
	10,			          /* Use Bit 10 (QC of second LS164) as F0 input 0 */
	23,			          /* Use Bit 23 (QH of third LS164) as F0 input 1 */
	DISC_LFSR_XOR,		  /* F0 is XOR */
	DISC_LFSR_NOT_IN0,	  /* F1 is inverted F0*/
	DISC_LFSR_REPLACE,	  /* F2 replaces the shifted register contents */
	0x000001,		      /* Everything is shifted into the first bit only */
	DISC_LFSR_FLAG_OUTPUT_F0, /* Output is result of F0 */
	0			          /* Output bit */
};

static const discrete_mixer_desc dkong_rc_jump_desc =
	{DISC_MIXER_IS_RESISTOR,
		{1, DK_R49+DK_R51,NE555_INTERNAL_R,2*NE555_INTERNAL_R},
		{NODE_26,0,0,0},
		{0,0,0,0},  // no node capacitors
		0, 0,
		DK_C24,
		0,
		0, 1};

static const discrete_mixer_desc dkong_rc_walk_desc =
	{DISC_MIXER_IS_RESISTOR,
		{1, DK_R45+DK_R44,NE555_INTERNAL_R,2*NE555_INTERNAL_R},
		{NODE_52,0,0,0},
		{0,0,0,0},  // no node capacitors
		0, 0,
		DK_C29,
		0,
		0, 1};

static const discrete_mixer_desc dkong_mixer_desc =
	{DISC_MIXER_IS_RESISTOR,
		{DK_R2, DK_R24, DK_R1, DK_R14},
		{0,0,0},	// no variable resistors
		{0,0,0},  // no node capacitors
#if DK_REVIEW
		0, RES_K(10),
#else
		0, 0,
#endif
		DK_C159,
		DK_C12,
		0, 1};

/* There is no load on the output for the jump circuit
 * For the walk circuit, the voltage does not matter */

static const discrete_555_desc dkong_555_vco_desc =
	{DISC_555_OUT_DC | DISC_555_OUT_ENERGY,
		DK_SUP_V,
		DK_SUP_V-0.5,DK_SUP_V*0.66,DK_SUP_V*0.33
	};

static const discrete_inverter_osc_desc dkong_inverter_osc_desc_jump =
	{DEFAULT_CD40XX_VALUES(DK_SUP_V),
	DISC_OSC_INVERTER_IS_TYPE1
	};

static const discrete_inverter_osc_desc dkong_inverter_osc_desc_walk =
	{DEFAULT_CD40XX_VALUES(DK_SUP_V),
	DISC_OSC_INVERTER_IS_TYPE2
	};

static const discrete_op_amp_filt_info dkong_sallen_key_info =
	{ RES_K(5.6), RES_K(5.6), 0, 0, 0,
	  CAP_N(22), CAP_N(10), 0
	};

static DISCRETE_SOUND_START(dkong2b)

	/************************************************/
	/* Input register mapping for dkong             */
	/************************************************/

	// DISCRETE_INPUT_DATA
    DISCRETE_INPUT_NOT(DS_SOUND2_INV)
    DISCRETE_INPUT_NOT(DS_SOUND1_INV)
    DISCRETE_INPUT_NOT(DS_SOUND0_INV)
    DISCRETE_INPUT_LOGIC(DS_DAC_DISCHARGE)
    DISCRETE_INPUT_DATA(DS_DAC)
	// Mixing - DAC
	DISCRETE_ADJUSTMENT_TAG(DS_ADJ_DAC, 1, 0, 1, DISC_LINADJ, "VR2")

	/************************************************/
	/* SIGNALS                                      */
	/************************************************/

	DISCRETE_LOGIC_INVERT(DS_SOUND0,1,DS_SOUND0_INV)
	DISCRETE_LOGIC_INVERT(DS_SOUND1,1,DS_SOUND1_INV)
	DISCRETE_LOGIC_INVERT(DS_SOUND2,1,DS_SOUND2_INV)

	/************************************************/
	/* Stomp                                        */
	/************************************************/
	/* Noise */
	DISCRETE_LFSR_NOISE(NODE_11, 1, 1, CLOCK_2VF, 1.0, 0, 0.5, &dkong_lfsr)
	DISCRETE_COUNTER(NODE_12, 1, 0, NODE_11, 7, DISC_COUNT_UP, 0, DISC_CLK_ON_R_EDGE)	// LS161, IC 3J
	DISCRETE_TRANSFORM3(NODE_13,1,NODE_12,3,DK_SUP_V,"01>2*")

	/* Stomp */
	/* C21 is discharged via Q5 BE */
	DISCRETE_RCDISC_MODULATED(NODE_15,1,DS_SOUND2_INV,0,DK_R10,0,0,DK_R9,DK_C21,DK_SUP_V)
	/* Q5 */
    DISCRETE_TRANSFORM2(NODE_16, 1, NODE_15, 0.6, "01>")
    DISCRETE_RCDISC2(NODE_17,NODE_16,DK_SUP_V,DK_R8+DK_R7,0.0,DK_R7,DK_C20)

 	DISCRETE_DIODE_MIXER2(NODE_18, 1, DK_1N5553_V, NODE_13, NODE_13) // D3
	DISCRETE_DIODE_MIXER2(NODE_20, 1, DK_1N5553_V, NODE_17, NODE_18) // D1, D2

    DISCRETE_RCINTEGRATE(NODE_22,1,NODE_20,DK_R5, R_PARALLEL(DK_R4+DK_R3,DK_R6),0,DK_C19,DK_SUP_V,DISC_RC_INTEGRATE_TYPE1)
    DISCRETE_MULTIPLY(DS_OUT_SOUND0,1,NODE_22,DK_R3/R_SERIE(DK_R3,DK_R4))

	/************************************************/
	/* Jump                                         */
	/************************************************/

	DISCRETE_MULTIPLY(NODE_24,1,DS_SOUND1,DK_SUP_V)
	/* 4049B Inverter Oscillator build from 3 inverters */
	DISCRETE_INVERTER_OSC(NODE_25,1,0,DK_R38,DK_R39,DK_C26,0,&dkong_inverter_osc_desc_jump)

	DISCRETE_TRANSFORM3(NODE_26,1,DS_SOUND1,DK_R32,DK_R49+DK_R50,"01*2+")
	DISCRETE_MIXER4(NODE_28, 1, NODE_24, NODE_25, DK_SUP_V, 0,&dkong_rc_jump_desc)
    /* 555 Voltage controlled */
    DISCRETE_555_ASTABLE_CV(NODE_29, 1, RES_K(47), RES_K(27), CAP_N(47), NODE_28,
    						&dkong_555_vco_desc)

	/* Jump trigger */
	DISCRETE_RCDISC_MODULATED(NODE_33,1,DS_SOUND1_INV,0,DK_R32,0,0,DK_R31,DK_C18,DK_SUP_V)

    DISCRETE_TRANSFORM2(NODE_34, 1, NODE_33, 0.6, "01>")
    DISCRETE_RCDISC2(NODE_35, NODE_34,DK_SUP_V,R_SERIE(DK_R30,DK_R29),0.0,DK_R29,DK_C17)

 	DISCRETE_DIODE_MIXER2(NODE_36, 1, DK_1N5553_V, NODE_29, NODE_29)
 	DISCRETE_DIODE_MIXER2(NODE_38, 1, DK_1N5553_V, NODE_36, NODE_35)

    DISCRETE_RCINTEGRATE(NODE_39,1,NODE_38,DK_R27, R_PARALLEL(DK_R28,DK_R26+DK_R25),0,DK_C16,DK_SUP_V,DISC_RC_INTEGRATE_TYPE1)
    DISCRETE_MULTIPLY(DS_OUT_SOUND1,1,NODE_39,DK_R25/(DK_R26+DK_R25))

	/************************************************/
	/* Walk                                         */
	/************************************************/
	DISCRETE_MULTIPLY(NODE_50,1,DS_SOUND0,DK_SUP_V)
	DISCRETE_INVERTER_OSC(NODE_51,1,0,DK_R47,DK_R48,DK_C30,0,&dkong_inverter_osc_desc_walk)

	DISCRETE_TRANSFORM3(NODE_52,1,DS_SOUND0,DK_R46,R_SERIE(DK_R44,DK_R45),"01*2+")
	DISCRETE_MIXER4(NODE_54, 1, NODE_50, NODE_51, DK_SUP_V, 0,&dkong_rc_walk_desc)

    /* 555 Voltage controlled */
    DISCRETE_555_ASTABLE_CV(NODE_55, 1, RES_K(47), RES_K(27), CAP_N(33), NODE_54, &dkong_555_vco_desc)
	/* Trigger */
	DISCRETE_RCDISC_MODULATED(NODE_60,1,DS_SOUND0_INV,NODE_55,DK_R36,DK_R18,DK_R35,DK_R17,DK_C25,DK_SUP_V)
	/* Filter and divide - omitted C22 */
	DISCRETE_CRFILTER(NODE_61, 1, NODE_60, DK_R15+DK_R16, DK_C23)
	DISCRETE_MULTIPLY(DS_OUT_SOUND2, 1, NODE_61, DK_R15/(DK_R15+DK_R16))

	/************************************************/
	/* DAC                                          */
	/************************************************/

	/* Signal decay circuit Q7, R20, C32 */
	DISCRETE_RCDISC(NODE_70, DS_DAC_DISCHARGE, 1, DK_R20, DK_C32)
	DISCRETE_TRANSFORM4(NODE_71, 1, DS_DAC,  DK_SUP_V/256.0, NODE_70, DS_DAC_DISCHARGE, "01*3!2+*")

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
	DISCRETE_MULTIPLY(DS_OUT_DAC, 1, NODE_71, DS_ADJ_DAC)
#else
	DISCRETE_MULTIPLY(DS_OUT_DAC, 1, NODE_73, DS_ADJ_DAC)
#endif

	/************************************************/
	/* Amplifier                                    */
	/************************************************/

	DISCRETE_MIXER4(NODE_288, 1, DS_OUT_SOUND0, DS_OUT_SOUND1, DS_OUT_DAC, DS_OUT_SOUND2, &dkong_mixer_desc)

	// Amplifier: internal amplifier
	DISCRETE_ADDER2(NODE_289,1,NODE_288,5.0*43.0/(100.0+43.0))
    DISCRETE_RCINTEGRATE(NODE_294,1,NODE_289,0,150,1000, CAP_U(33),DK_SUP_V,DISC_RC_INTEGRATE_TYPE3)
    DISCRETE_CRFILTER(NODE_295,1,NODE_294, RES_K(50), DK_C13)
	//DISCRETE_CRFILTER(NODE_295,1,NODE_294, 1000, DK_C13)
	// EZV20 equivalent filter circuit ...
	DISCRETE_CRFILTER(NODE_296,1,NODE_295, RES_K(1), CAP_U(4.7))
#if DK_NO_FILTERS
	DISCRETE_OUTPUT(NODE_288, 32767.0/5.0 * 10)
#else
	DISCRETE_OUTPUT(NODE_296, 32767.0/5.0 * 2)
#endif

DISCRETE_SOUND_END

/****************************************************************
 *
 * radarscp Discrete Sound Interface
 *
 ****************************************************************/

#define RS_R1		RES_K(10)
#define RS_R2		RES_K(10)
#define RS_R3		RES_K(5.1)
#define RS_R4		RES_K(2)
#define RS_R5		750
#define RS_R6		RES_K(4.7)
#define RS_R7		RES_K(10)
#define RS_R8		RES_K(100)
#define RS_R9		RES_K(10)
#define RS_R14		RES_K(10)
#define RS_R15		RES_K(5.6)	// ????
#define RS_R16		RES_K(5.6)
#define RS_R18		RES_K(4.7)
#define RS_R22		RES_K(5.6)
#define RS_R23		RES_K(5.6)
#define RS_R25		RES_K(10)
#define RS_R26		RES_K(5.1)
#define RS_R27		RES_K(2) // 10k in schematics - but will oscillate
#define RS_R28		150
#define RS_R29		RES_K(4.7)
#define RS_R30		RES_K(10)
#define RS_R31		RES_K(100)
#define RS_R32		RES_K(10)
#define RS_R37		RES_K(1)
#define RS_R38		RES_K(1)
#define RS_R39		RES_K(1)
#define RS_R40		RES_K(10)
#define RS_R42		RES_K(10)
#define RS_R43		RES_K(5.1)
#define RS_R44		RES_K(3.9)
#define RS_R46		RES_K(1)
#define RS_R48		RES_K(18)
#define RS_R49		RES_M(3.3)
#define RS_R54		RES_K(1.2)
#define RS_R55		RES_K(10)
#define RS_R56		RES_K(12)
#define RS_R57		RES_K(4.3) // ??? 43
#define RS_R58		RES_K(43)
#define RS_R59		RES_K(1.2)
#define RS_R60		RES_K(10)
#define RS_R61		RES_K(20)
#define RS_R62		RES_K(2)
#define RS_R63		130

#define RS_R_NN01	RES_K(10)
#define RS_R_NN02	RES_K(10)

#define RS_C5		CAP_U(220)
#define RS_C18		CAP_U(1)
#define RS_C19		CAP_U(22)
#define RS_C20		CAP_U(1)
#define RS_C22		CAP_U(47)
#define RS_C29		CAP_U(1)
#define RS_C30		CAP_U(10)
#define RS_C31		CAP_U(1)
#define RS_C33		CAP_U(4.7)
#define RS_C38		CAP_N(10)
#define RS_C40		CAP_U(10)
#define RS_C45		CAP_U(22)
#define RS_C46		CAP_U(1)
#define RS_C47		CAP_U(22)
#define RS_C48		CAP_N(33)
#define RS_C49		CAP_N(10)
#define RS_C50		CAP_U(3.3)
#define RS_C51		CAP_U(3.3)
#define RS_C53		CAP_U(3.3)
#define RS_C54		CAP_U(1)

#define RS_VR2		RES_K(10)
#define RS_C2		CAP_U(1)


static const discrete_mixer_desc radarscp_mixer_desc =
	{DISC_MIXER_IS_RESISTOR,
		{RS_R14, RS_R25, RS_R2, RS_R42, RS_R1},
		{0,0,0,0,0},	// no variable resistors
		{0,0,0,0,0},  // no node capacitors
		0, RS_VR2,
		0,
		RS_C2,
		0, 1};

static const discrete_mixer_desc radarscp_mixer_desc_0 =
	{DISC_MIXER_IS_RESISTOR,
		{RS_R56+RS_R54,NE555_INTERNAL_R,R_PARALLEL(2*NE555_INTERNAL_R,RS_R55) },
		{0,0,0},
		{0,0,0,0},  // no node capacitors
		0, 0,
		RS_C51,
		0,
		0, 1};

static const discrete_mixer_desc radarscp_mixer_desc_7 =
	{DISC_MIXER_IS_RESISTOR,
		{RS_R63+RS_R59, NE555_INTERNAL_R,R_PARALLEL(2*NE555_INTERNAL_R,RS_R60)},
		{0,0,0},	// no variable resistors
		{0,0,0},  // no node capacitors
		0, 0,
		RS_C50,
		0,
		0, 1};

/* There is no load on the output for the jump circuit
 * For the walk circuit, the voltage does not matter */

static const discrete_555_desc radarscp_555_vco_desc =
	{DISC_555_OUT_DC | DISC_555_OUT_ENERGY,
		DK_SUP_V,
		DK_SUP_V-0.5,DK_SUP_V*0.66,DK_SUP_V*0.33
	};

static const discrete_inverter_osc_desc radarscp_inverter_osc_desc_0 =
	{DEFAULT_CD40XX_VALUES(DK_SUP_V),
	DISC_OSC_INVERTER_IS_TYPE2
	};

static const discrete_inverter_osc_desc radarscp_inverter_osc_desc_7 =
	{DEFAULT_CD40XX_VALUES(DK_SUP_V),
	DISC_OSC_INVERTER_IS_TYPE3
	};

static DISCRETE_SOUND_START(radarscp)

	/************************************************/
	/* Input register mapping for radarscp          */
	/************************************************/

	// DISCRETE_INPUT_DATA
    DISCRETE_INPUT_NOT(DS_SOUND0_INV)
    DISCRETE_INPUT_NOT(DS_SOUND1_INV)
    DISCRETE_INPUT_NOT(DS_SOUND2_INV)
    DISCRETE_INPUT_NOT(DS_SOUND6_INV)
    DISCRETE_INPUT_NOT(DS_SOUND7_INV)
    DISCRETE_INPUT_LOGIC(DS_DAC_DISCHARGE)
    DISCRETE_INPUT_DATA(DS_DAC)

	// Mixing - DAC
	DISCRETE_ADJUSTMENT_TAG(DS_ADJ_DAC, 1, 0, 1, DISC_LINADJ, "VR2")

	/************************************************/
	/* SIGNALS                                      */
	/************************************************/

	DISCRETE_LOGIC_INVERT(DS_SOUND6,1,DS_SOUND6_INV)
	DISCRETE_LOGIC_INVERT(DS_SOUND7,1,DS_SOUND7_INV)

	/************************************************/
	/* Noise                                      */
	/************************************************/

	DISCRETE_LFSR_NOISE(NODE_11, 1, 1, CLOCK_2VF, 1.0, 0, 0.5, &dkong_lfsr)
	// Clear (1) from SOUND6
	DISCRETE_COUNTER(NODE_12, 1, DS_SOUND6_INV, NODE_11, 15, DISC_COUNT_UP, 0, DISC_CLK_ON_R_EDGE)	// LS161, IC 3J
	DISCRETE_TRANSFORM3(NODE_13,1,NODE_12,0x04,DK_SUP_V,"01&1=2*")  //QC => SND02
	DISCRETE_TRANSFORM3(NODE_14,1,NODE_12,0x02,DK_SUP_V,"01&1=2*")  //QB => SND01

	/************************************************/
	/* SOUND2                                       */
	/************************************************/

	/* C21 is discharged via Q5 BE */

	DISCRETE_RCDISC_MODULATED(NODE_16,1,DS_SOUND2_INV,0,RS_R_NN01,0,0,RS_R9*2,RS_C20,DK_SUP_V)
    DISCRETE_TRANSFORM2(NODE_17, 1, NODE_16, 0.6, "01>") // TR2
    DISCRETE_RCDISC2(NODE_18,NODE_17,DK_SUP_V,RS_R8+RS_R7,0.0,RS_R7,RS_C19)

 	DISCRETE_DIODE_MIXER2(NODE_19, 1, DK_1N5553_V, NODE_13, NODE_13) // D3
	DISCRETE_DIODE_MIXER2(NODE_20, 1, DK_1N5553_V, NODE_18, NODE_19) // D1, D2

    DISCRETE_RCINTEGRATE(NODE_22,1,NODE_20,RS_R5, R_PARALLEL(RS_R4+RS_R3,RS_R6),0,RS_C18,DK_SUP_V,DISC_RC_INTEGRATE_TYPE1)
    DISCRETE_MULTIPLY(DS_OUT_SOUND2,1,NODE_22,RS_R3/R_SERIE(RS_R3,RS_R4))

	/************************************************/
	/* SOUND1                                       */
	/************************************************/

	/* C21 is discharged via Q5 BE */

	DISCRETE_RCDISC_MODULATED(NODE_26,1,DS_SOUND1_INV,0,RS_R_NN02,0,0,RS_R32,RS_C31,DK_SUP_V)
    DISCRETE_TRANSFORM2(NODE_27, 1, NODE_26, 0.6, "01>") // TR5
    DISCRETE_RCDISC2(NODE_28,NODE_27,DK_SUP_V,RS_R31+RS_R30,0.0,RS_R30,RS_C30)

 	DISCRETE_DIODE_MIXER2(NODE_29, 1, DK_1N5553_V, NODE_14, NODE_14) // D3
	DISCRETE_DIODE_MIXER2(NODE_30, 1, DK_1N5553_V, NODE_28, NODE_29) // D1, D2

    DISCRETE_RCINTEGRATE(NODE_31,1,NODE_30,RS_R28, R_PARALLEL(RS_R27+RS_R26,RS_R29),0,RS_C29,DK_SUP_V,DISC_RC_INTEGRATE_TYPE1)
    DISCRETE_MULTIPLY(DS_OUT_SOUND1,1,NODE_31,RS_R26/R_SERIE(RS_R26,RS_R27))

	/************************************************/
	/* SOUND0                                       */
	/************************************************/

	DISCRETE_INVERTER_OSC(NODE_41,1,0,RS_R57,RS_R58,RS_C53,0,&radarscp_inverter_osc_desc_0)
	DISCRETE_MIXER3(NODE_42, 1, NODE_41, DK_SUP_V, 0,&radarscp_mixer_desc_0)

    /* 555 Voltage controlled */
    DISCRETE_555_ASTABLE_CV(NODE_43, DS_SOUND6, RES_K(47), RES_K(27), RS_C49, NODE_42, &radarscp_555_vco_desc)

	DISCRETE_RCDISC_MODULATED(NODE_44,1,DS_SOUND0_INV,NODE_43,RS_R39,RS_R18,RS_R37,RS_R38,RS_C22,DK_SUP_V)
	DISCRETE_CRFILTER(NODE_45, 1, NODE_44, RS_R15+RS_R16, RS_C33)
	DISCRETE_MULTIPLY(DS_OUT_SOUND0, 1, NODE_45, RS_R15/(RS_R15+RS_R16))

	/************************************************/
	/* SOUND7                                       */
	/************************************************/

	DISCRETE_INVERTER_OSC(NODE_51,1,0,RS_R62,RS_R61,RS_C54,0,&radarscp_inverter_osc_desc_0)
	/* inverter osc used as sine wave generator */
	DISCRETE_INVERTER_OSC(NODE_52,1,0,RS_R48,RS_R49,RS_C47,0,&radarscp_inverter_osc_desc_7)

	DISCRETE_MIXER3(NODE_53, 1, NODE_51, DK_SUP_V, 0,&radarscp_mixer_desc_7)
    /* 555 Voltage controlled */
    DISCRETE_555_ASTABLE_CV(NODE_54, DS_SOUND7, RES_K(47), RES_K(27), RS_C48, NODE_53, &radarscp_555_vco_desc)

    DISCRETE_RCINTEGRATE(NODE_55,1,NODE_52,RS_R46, RS_R46,0,RS_C45,DK_SUP_V,DISC_RC_INTEGRATE_TYPE1)
    DISCRETE_TRANSFORM4(NODE_56, 1, NODE_55, DS_SOUND7,NODE_54,2.5, "01*23<*")
	DISCRETE_CRFILTER(NODE_57, 1, NODE_56, RS_R43+RS_R44, RS_C46)
	DISCRETE_MULTIPLY(DS_OUT_SOUND7, 1, NODE_57, RS_R44/(RS_R43+RS_R44))

	/************************************************/
	/* DAC                                          */
	/************************************************/
	/* Signal decay circuit Q7, R20, C32 */
	DISCRETE_RCDISC(NODE_170, DS_DAC_DISCHARGE, 1, RS_R40, RS_C40)
	DISCRETE_TRANSFORM4(NODE_171, 1, DS_DAC,  DK_SUP_V/256.0, NODE_170, DS_DAC_DISCHARGE, "01*3!2+*")

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
	DISCRETE_MULTIPLY(DS_OUT_DAC, 1, NODE_173, DS_ADJ_DAC)

	/************************************************/
	/* Amplifier                                    */
	/************************************************/

	DISCRETE_MIXER5(NODE_288, 1, DS_OUT_SOUND0, DS_OUT_SOUND1, DS_OUT_SOUND2, DS_OUT_SOUND7, DS_OUT_DAC, &radarscp_mixer_desc)

	// Amplifier: internal amplifier
	DISCRETE_ADDER2(NODE_289,1,NODE_288,5.0*43.0/(100.0+43.0))
    DISCRETE_RCINTEGRATE(NODE_294,1,NODE_289,0,150,1000, CAP_U(33),DK_SUP_V,DISC_RC_INTEGRATE_TYPE3)
	DISCRETE_CRFILTER(NODE_295,1,NODE_294, 1000, DK_C13)
	DISCRETE_OUTPUT(NODE_295, 32767.0/5.0 * 3)

DISCRETE_SOUND_END

/****************************************************************
 *
 * DkongJR Discrete Sound Interface
 *
 ****************************************************************/

#define JR_R2		120
#define JR_R3		RES_K(100)
#define JR_R4		RES_K(47)
#define JR_R5		RES_K(150)
#define JR_R6		RES_K(20)
#define JR_R8		RES_K(47)
#define JR_R9		RES_K(47)
#define JR_R10		RES_K(10)
#define JR_R11		RES_K(20)
#define JR_R12		RES_K(10)
#define JR_R13		RES_K(47)
#define JR_R14		RES_K(30)
#define JR_R17		RES_K(47)
#define JR_R18		RES_K(100)
#define JR_R19		(100)
#define JR_R20		RES_K(10)
#define JR_R24		RES_K(4.7)
#define JR_R25		RES_K(47)
#define JR_R27		RES_K(10)
#define JR_R28		RES_K(100)


#define JR_C13		CAP_U(4.7)
#define JR_C14		CAP_U(4.7)
#define JR_C15		CAP_U(22)
#define JR_C16		CAP_U(3.3)
#define JR_C17		CAP_U(3.3) // ??? illegible
#define JR_C18		CAP_N(22)
#define JR_C19		CAP_N(4.7)
#define JR_C20		CAP_U(0.12)
#define JR_C21		CAP_N(56)
#define JR_C22		CAP_N(220)
#define JR_C23		CAP_U(0.47)
#define JR_C24		CAP_U(47)
#define JR_C25		CAP_U(1)
#define JR_C26		CAP_U(47)
#define JR_C27		CAP_U(22)
#define JR_C28		CAP_U(10)
#define JR_C29		CAP_U(10)
#define JR_C30		CAP_U(0.47)
#define JR_C32		CAP_U(10)
#define JR_C37		CAP_U(0.12)
#define JR_C39		CAP_U(0.47)
#define JR_C161		CAP_U(1)
#define JR_C155		CAP_U(0.01)

#define TTL_HIGH	(4)
#define GND			(0)


/* KT = 0.25 for diode circuit, 0.33 else */

#define DISCRETE_LS123(_N, _T, _R, _C) \
	DISCRETE_ONESHOTR(_N, 0, _T, TTL_HIGH, (0.25 * (_R) * (_C) * (1.0+700./(_R))), DISC_ONESHOT_RETRIG | DISC_ONESHOT_REDGE)
#define DISCRETE_LS123_INV(_N, _T, _R, _C) \
	DISCRETE_ONESHOTR(_N, 0, _T, TTL_HIGH, (0.25 * (_R) * (_C) * (1.0+700./(_R))), DISC_ONESHOT_RETRIG | DISC_ONESHOT_REDGE | DISC_OUT_ACTIVE_LOW)

#define DISCRETE_BITSET(_N, _N1, _B) DISCRETE_TRANSFORM3(_N, 1, _N1, 1 << ((_B)-1), 0, "01&2>")
#define DISCRETE_ENERGY_NAND(_N, _E, _N1, _N2) DISCRETE_TRANSFORM3(_N, _E, _N1, _N2, 1, "201*-")

static const discrete_mixer_desc dkongjr_mixer_desc =
	{DISC_MIXER_IS_RESISTOR,
		{JR_R5, JR_R3, JR_R6, JR_R4, JR_R25},
		{0,0,0,0,0},	// no variable resistors
		{0,0,0,0,0},  // no node capacitors
		0, 0,
		JR_C155,
		JR_C161,
		0, 1};

static const discrete_mixer_desc dkongjr_s1_mixer_desc =
	{DISC_MIXER_IS_RESISTOR,
		{JR_R13, JR_R12},
		{0,0},	// no variable resistors
		{0,0},  // no node capacitors
		0, RES_K(90), // Internal LS624 resistors ...
		JR_C24,
		0,
		0, 1};

static const discrete_lfsr_desc dkongjr_lfsr =
{
	DISC_CLK_BY_COUNT,
	16,			          /* Bit Length */
	0,			          /* Reset Value */
	2,			          /* Use Bit 2 (QC of first LS164) as F0 input 0 */
	15,			          /* Use Bit 15 (QH of secong LS164) as F0 input 1 */
	DISC_LFSR_XOR,		  /* F0 is XOR */
	DISC_LFSR_NOT_IN0,	  /* F1 is inverted F0*/
	DISC_LFSR_REPLACE,	  /* F2 replaces the shifted register contents */
	0x000001,		      /* Everything is shifted into the first bit only */
	DISC_LFSR_FLAG_OUTPUT_F0 | DISC_LFSR_FLAG_OUT_INVERT, /* Output is result of F0 */
	0			          /* Output bit */
};

static DISCRETE_SOUND_START(dkongjr)

	/************************************************/
	/* Input register mapping for dkongjr           */
	/************************************************/

	/* DISCRETE_INPUT_DATA */
    DISCRETE_INPUT_NOT(DS_SOUND0_INV)
    DISCRETE_INPUT_NOT(DS_SOUND1_INV)
    DISCRETE_INPUT_NOT(DS_SOUND2_INV)
    DISCRETE_INPUT_NOT(DS_SOUND6_INV)
    DISCRETE_INPUT_NOT(DS_SOUND7_INV)
    DISCRETE_INPUT_NOT(DS_SOUND9_INV)
    DISCRETE_INPUT_LOGIC(DS_DAC_DISCHARGE)
    DISCRETE_INPUT_DATA(DS_DAC)

	/************************************************/
	/* SIGNALS                                      */
	/************************************************/

    DISCRETE_LOGIC_INVERT(DS_SOUND7,1,DS_SOUND7_INV)
	DISCRETE_LOGIC_INVERT(DS_SOUND9,1,DS_SOUND9_INV)

	/************************************************/
	/* SOUND1                                       */
	/************************************************/

	DISCRETE_LS123(NODE_10, DS_SOUND1_INV, JR_R9, JR_C15)
	DISCRETE_TRANSFORM2(NODE_11,1,NODE_104,TTL_HIGH,"0!1*")
	DISCRETE_LOGIC_INVERT(NODE_12,1,NODE_10)
	DISCRETE_MIXER2(NODE_13, 1, NODE_10, NODE_11, &dkongjr_s1_mixer_desc)
	DISCRETE_74LS624( NODE_14, 1, NODE_13, 0.98*DK_SUP_V, JR_C22, DISC_LS624_OUT_ENERGY)
	DISCRETE_RCDISC_MODULATED(NODE_15, 1, NODE_12, NODE_14, 120, JR_R27, RES_K(0.001), JR_R28, JR_C28, DK_SUP_V)
	/* The following circuit does not match 100%, however works.
     * To be exact, we need a C-R-C-R circuit, we actually do not have.
     */
	DISCRETE_CRFILTER_VREF(NODE_16, 1, NODE_15, JR_R4, JR_C23, 2.5)
	DISCRETE_RCFILTER(DS_OUT_SOUND1, 1, NODE_16, JR_R19, JR_C21)

	/************************************************/
	/* SOUND2                                       */
	/************************************************/

	DISCRETE_74LS624( NODE_20, 1, 0, 0.98*DK_SUP_V, JR_C20, DISC_LS624_OUT_COUNT_F)
	DISCRETE_LFSR_NOISE(NODE_21, 1, 1, NODE_20, 1.0, 0, 0.5, &dkongjr_lfsr)
	DISCRETE_LS123_INV(NODE_25, DS_SOUND2_INV, JR_R17, JR_C27)
	DISCRETE_RCDISC_MODULATED(NODE_26, 1, NODE_25, NODE_21, 120, JR_R24, RES_K(0.001), JR_R18, JR_C29, DK_SUP_V)
	/* The following circuit does not match 100%, however works.
     * To be exact, we need a C-R-C-R circuit, we actually do not have.
     */
	DISCRETE_CRFILTER_VREF(NODE_27, 1, NODE_26, JR_R6, JR_C30, 2.5)
	DISCRETE_RCFILTER(DS_OUT_SOUND2, 1, NODE_27, JR_R2, JR_C25)

	/************************************************/
	/* SOUND9                                       */
	/************************************************/

	DISCRETE_RCFILTER(NODE_90, 1, DS_SOUND9_INV, JR_R14, JR_C26)
	DISCRETE_MULTIPLY(NODE_91, 1, NODE_90, TTL_HIGH)
	DISCRETE_74LS624( NODE_92, 1, NODE_91, DK_SUP_V, JR_C37, DISC_LS624_OUT_ENERGY)
	DISCRETE_ENERGY_NAND(NODE_93, 1, NODE_92, DS_SOUND9)
	DISCRETE_MULTIPLY(DS_OUT_SOUND9, 1, NODE_93, TTL_HIGH)

	/************************************************/
	/* SOUND0 / SOUND7                              */
	/************************************************/

	DISCRETE_COUNTER(NODE_100,1,0,NODE_118,0xFFFF,DISC_COUNT_UP,0,DISC_CLK_BY_COUNT)

	DISCRETE_BITSET(NODE_101, NODE_100, 7) 	//LS157 2A
	DISCRETE_BITSET(NODE_102, NODE_100, 4) 	//LS157 2B
	DISCRETE_BITSET(NODE_103, NODE_100, 13) //LS157 3A
	DISCRETE_BITSET(NODE_104, NODE_100, 12) //LS157 3B

	/* LS157 Switches */
	DISCRETE_SWITCH(NODE_105, 1, DS_SOUND7_INV, GND, NODE_113) // Switch 1 from LS624
	DISCRETE_SWITCH(NODE_106, 1, DS_SOUND7_INV, NODE_101, NODE_102) // Switch 2
	DISCRETE_SWITCH(NODE_107, 1, DS_SOUND7_INV, NODE_103, NODE_104) // Switch 3

	DISCRETE_LS123(NODE_110, DS_SOUND0_INV, JR_R8, JR_C14)
	DISCRETE_TRANSFORM2(NODE_111, 1, TTL_HIGH, NODE_110, "01-")
	DISCRETE_RCFILTER(NODE_112, 1, NODE_111, JR_R10, JR_C17)
	DISCRETE_74LS624(NODE_113, 1, NODE_112, DK_SUP_V, JR_C18, DISC_LS624_OUT_ENERGY)

	DISCRETE_LOGIC_XOR(NODE_115, 1, NODE_105, NODE_106)

	DISCRETE_TRANSFORM2(NODE_116,1, NODE_107, TTL_HIGH, "0!1*")
	DISCRETE_RCFILTER(NODE_117, 1, NODE_116, JR_R11, JR_C16)
	DISCRETE_74LS624(NODE_118, 1, NODE_117, DK_SUP_V, JR_C19, DISC_LS624_OUT_COUNT_F)

	DISCRETE_LOGIC_NAND(NODE_120, 1, NODE_115, NODE_110)
	DISCRETE_MULTIPLY(DS_OUT_SOUND0, 1, NODE_120, TTL_HIGH)

	/************************************************/
	/* DAC                                          */
	/************************************************/
	/* Signal decay circuit Q7, R20, C32 */
	DISCRETE_RCDISC(NODE_170, DS_DAC_DISCHARGE, 1, JR_R20, JR_C32)
	DISCRETE_TRANSFORM4(NODE_171, 1, DS_DAC,  DK_SUP_V/256.0, NODE_170, DS_DAC_DISCHARGE, "01*3!2+*")

	/* following the DAC are two opamps. The first is a current-to-voltage changer
     * for the DAC08 which delivers a variable output current.
     *
     * The second one is a Sallen Key filter ...
     * http://www.t-linespeakers.org/tech/filters/Sallen-Key.html
     * f = w / 2 / pi  = 1 / ( 2 * pi * 5.6k*sqrt(22n*10n)) = 1916 Hz
     * Q = 1/2 * sqrt(22n/10n)= 0.74
     */

	DISCRETE_SALLEN_KEY_FILTER(DS_OUT_DAC, 1, NODE_171, DISC_SALLEN_KEY_LOW_PASS, &dkong_sallen_key_info)

	/************************************************/
	/* Amplifier                                    */
	/************************************************/

	DISCRETE_MIXER5(NODE_288, 1, DS_OUT_SOUND9, DS_OUT_SOUND0, DS_OUT_SOUND2, DS_OUT_SOUND1, DS_OUT_DAC, &dkongjr_mixer_desc)

	/* Amplifier: internal amplifier
     * Just a 1:n amplifier without filters - just the output filter
     */
	DISCRETE_CRFILTER(NODE_295,1,NODE_288, 1000, JR_C13)
	DISCRETE_OUTPUT(NODE_295, 32767.0/5.0 * 10)

DISCRETE_SOUND_END

/****************************************************************
 *
 * Initialization
 *
 ****************************************************************/

static SOUND_START( dkong )
{
	dkong_state *state = machine->driver_data;

	state_save_register_global(state->page);
	state_save_register_global(state->mcustatus);
	state_save_register_global(state->portT);
}

static SOUND_RESET( dkong )
{
	dkong_state *state = machine->driver_data;

	state->mcustatus = 0;
	state->page = 0x47;
	state->portT = 0;

	I8035_T_W_AL(machine,0,0);
	I8035_T_W_AL(machine,1,0);
	I8035_P1_W(machine,0xFF);
	I8035_P2_W(machine,0xFF);
	soundlatch_w(machine,0,0x0F);
}

static SOUND_RESET( dkongjr )
{
	SOUND_RESET_CALL(dkong);
	soundlatch_w(machine,0,0x00);
}

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
 3: 1C          You'll notice
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

    static const char *const radarsc1_sample_names[] =
    {
        "*radarsc1",
        "10.wav",
        "12.wav",
        "14.wav",
        "16.wav",
        "18.wav",
        "1A.wav",
        "1C.wav",
        "1E.wav",
        "20.wav",
        0
    };

    static const struct Samplesinterface radarsc1_samples_interface =
    {
        8,
        radarsc1_sample_names
    };

*/

static WRITE8_HANDLER( M58817_command_w )
{
	logerror("PA Write %x\n", data);

	tms5110_CTL_w(machine, 0, data & 0x0f);
	tms5110_PDC_w(machine, 0, (data>>4) & 0x01);
	// FIXME 0x20 is CS
}

/****************************************************************
 *
 * I/O Handlers - static
 *
 ****************************************************************/

static READ8_HANDLER( dkong_sh_p1_r )
{
	return I8035_P1_R(machine);
}

static READ8_HANDLER( dkong_sh_p2_r )
{
	return I8035_P2_R(machine);
}

static READ8_HANDLER( dkong_sh_t0_r )
{
	return I8035_T_R(machine,0);
}

static READ8_HANDLER( dkong_sh_t1_r )
{
	return I8035_T_R(machine,1);
}

static WRITE8_HANDLER( dkong_voice_w )
{
	/* only provided for documentation purposes
     * not actually used
     */
	logerror("dkong_speech_w: 0x%02x\n", data);
}

static READ8_HANDLER( dkong_voice_status_r )
{
	/* only provided for documentation purposes
     * not actually used
     */
	return 0;
}

static READ8_HANDLER( dkong_sh_tune_r )
{
	dkong_state *state = machine->driver_data;
	UINT8 *SND = memory_region(machine, "sound");

	if ( state->page & 0x40 )
	{
		return (soundlatch_r(machine,0) & 0x0F) | (dkong_voice_status_r(machine,0)<<4);
	}
	else
	{
		/* printf("rom access at pc = %4x\n",activecpu_get_pc()); */
		return (SND[0x1000+(state->page & 7)*256+offset]);
	}
}

static READ8_HANDLER( dkongjr_sh_tune_r )
{
	return soundlatch_r(machine,0) & 0x01F;
}

static WRITE8_HANDLER( dkong_sh_p1_w )
{
	discrete_sound_w(machine,DS_DAC,data);
}


static READ8_HANDLER( radarsc1_sh_p1_r )
{
	int r;

	r = (I8035_P1_R(machine) & 0x80) | (tms5110_status_r(machine,0)<<6);
	return r;
}

static WRITE8_HANDLER( dkong_sh_p2_w )
{
	dkong_state *state = machine->driver_data;

	/*   If P2.Bit7 -> is apparently an external signal decay or other output control
     *   If P2.Bit6 -> activates the external compressed sample ROM (not radarsc1)
     *   If P2.Bit5 -> Signal ANSN ==> Grid enable (radarsc1)
     *   If P2.Bit4 -> status code to main cpu
     *   P2.Bit2-0  -> select the 256 byte bank for external ROM
     */

	state->mcustatus = ((~data & 0x10) >> 4);
	radarsc1_ansn_w(machine, 0, (data & 0x20) >> 5);
	state->page = (data & 0x47);

	discrete_sound_w(machine,DS_DAC_DISCHARGE, (data & 0x80) ? 0 : 1 );
}


/****************************************************************
 *
 * I/O Handlers - global
 *
 ****************************************************************/

WRITE8_HANDLER( dkongjr_sh_test6_w )
{
	I8035_P2_W_AL(machine,6,data & 1);
}


WRITE8_HANDLER( dkongjr_sh_tuneselect_w )
{
	soundlatch_w(machine,offset,data);
}

READ8_HANDLER( dkong_audio_status_r )
{
	dkong_state *state = machine->driver_data;

	return state->mcustatus;
}

WRITE8_HANDLER( dkong_audio_irq_w )
{
	if (data)
		cpunum_set_input_line(machine, 1, 0, ASSERT_LINE);
	else
		cpunum_set_input_line(machine, 1, 0, CLEAR_LINE);
}

WRITE8_HANDLER( dkong_snd_disc_w )
{
	dkong_state *state = machine->driver_data;

	switch (offset)
	{
		case 0:
			discrete_sound_w(machine,DS_SOUND0_INP,data & 1);
			break;
		case 1:
			discrete_sound_w(machine,DS_SOUND1_INP,data & 1);
			break;
		case 2:
			discrete_sound_w(machine,DS_SOUND2_INP,data & 1);
			radarscp_snd02_w(machine, 0, data & 1);
			break;
		case 3:
			if (state->hardware_type == HARDWARE_TRS01)
				//SOUND3 ==> PA7
				I8035_P1_W_AL(machine,7,data & 1);
			else
				I8035_P2_W_AL(machine,5,data & 1);
			break;
		case 4:
			I8035_T_W_AL(machine, 1, data & 1);
			break;
		case 5:
			I8035_T_W_AL(machine, 0, data & 1);
			break;
		case 6:
			discrete_sound_w(machine,DS_SOUND6_INP,data & 1);
			break;
		case 7:
			discrete_sound_w(machine,DS_SOUND7_INP,data & 1);
			break;
 	}
	return;
}

WRITE8_HANDLER( dkong_sh_tuneselect_w )
{
	soundlatch_w(machine,offset,data ^ 0x0f);
}

WRITE8_HANDLER( dkongjr_snd_w1 )
{
	dkong_state *state = machine->driver_data;

	switch (offset)
	{
		case 0:			/* climb */
			discrete_sound_w(machine,DS_SOUND0_INP,data & 1);
			break;
		case 1:			/* jump */
			discrete_sound_w(machine,DS_SOUND1_INP,data & 1);
			break;
		case 2:			/* land */
			discrete_sound_w(machine,DS_SOUND2_INP,data & 1);
			break;
		case 3: /* Port 3 write ==> PB 5 */
			I8035_P2_W_AL(machine,5,data & 1);
			break;
		case 4:			/* Port 4 write */
			I8035_T_W_AL(machine,1, data & 1);
			break;
		case 5:			/* Port 5 write */
			I8035_T_W_AL(machine,0, data & 1);
			break;
		case 6:			/* Port 6 write ==> PB 4 */
			I8035_P2_W_AL(machine,4,data & 1);
			break;
		case 7:			/* walk */
			discrete_sound_w(machine,DS_SOUND7_INP,data & 1);
			break;
	}
}

WRITE8_HANDLER( dkongjr_snd_w2 )
{
	switch (offset)
	{
	case 0:			/* S9 - drop */
		discrete_sound_w(machine,DS_SOUND9_INP,data & 1);
		break;
	}
}

/*************************************
 *
 *  Sound CPU memory handlers
 *
 *************************************/

static ADDRESS_MAP_START( dkong_sound_map, ADDRESS_SPACE_PROGRAM, 8 )
	AM_RANGE(0x0000, 0x0fff) AM_ROM
ADDRESS_MAP_END

static ADDRESS_MAP_START( dkong_sound_io_map, ADDRESS_SPACE_IO, 8 )
	AM_RANGE(0x00, 0xff) AM_READWRITE(dkong_sh_tune_r, dkong_voice_w)
	AM_RANGE(I8039_bus, I8039_bus) AM_READWRITE(dkong_sh_tune_r, dkong_voice_w)
	AM_RANGE(I8039_p1, I8039_p1) AM_READWRITE(dkong_sh_p1_r, dkong_sh_p1_w)
	AM_RANGE(I8039_p2, I8039_p2) AM_READWRITE(dkong_sh_p2_r, dkong_sh_p2_w)
	AM_RANGE(I8039_t0, I8039_t0) AM_READ(dkong_sh_t0_r)
	AM_RANGE(I8039_t1, I8039_t1) AM_READ(dkong_sh_t1_r)
ADDRESS_MAP_END

static ADDRESS_MAP_START( dkongjr_sound_io_map, ADDRESS_SPACE_IO, 8 )
	AM_RANGE(0x00, 0xff) AM_READ(dkongjr_sh_tune_r)
	AM_RANGE(I8039_p1, I8039_p1) AM_READWRITE(dkong_sh_p1_r, dkong_sh_p1_w)
	AM_RANGE(I8039_p2, I8039_p2) AM_READWRITE(dkong_sh_p2_r, dkong_sh_p2_w)
	AM_RANGE(I8039_t0, I8039_t0) AM_READ(dkong_sh_t0_r)
	AM_RANGE(I8039_t1, I8039_t1) AM_READ(dkong_sh_t1_r)
ADDRESS_MAP_END

static ADDRESS_MAP_START( radarsc1_sound_io_map, ADDRESS_SPACE_IO, 8 )
	AM_RANGE(0x00, 0xff) AM_READ(soundlatch_r)
	AM_RANGE(0x00, 0xff) AM_WRITE(dkong_sh_p1_w) // DAC here
	AM_RANGE(I8039_p1, I8039_p1) AM_READWRITE(radarsc1_sh_p1_r, M58817_command_w)
	AM_RANGE(I8039_p2, I8039_p2) AM_READWRITE(dkong_sh_p2_r, dkong_sh_p2_w)
	AM_RANGE(I8039_t0, I8039_t0) AM_READ(dkong_sh_t0_r)
	AM_RANGE(I8039_t1, I8039_t1) AM_READ(dkong_sh_t1_r)
ADDRESS_MAP_END

static ADDRESS_MAP_START( dkong3_sound1_map, ADDRESS_SPACE_PROGRAM, 8 )
	AM_RANGE(0x0000, 0x01ff) AM_RAM
	AM_RANGE(0x4016, 0x4016) AM_READ(soundlatch_r)		// overwrite default
	AM_RANGE(0x4017, 0x4017) AM_READ(soundlatch2_r)
	AM_RANGE(0x4000, 0x4017) AM_READ(NESPSG_0_r)
	AM_RANGE(0x4000, 0x4017) AM_WRITE(NESPSG_0_w)
	AM_RANGE(0xe000, 0xffff) AM_ROM
ADDRESS_MAP_END

static ADDRESS_MAP_START( dkong3_sound2_map, ADDRESS_SPACE_PROGRAM, 8 )
	AM_RANGE(0x0000, 0x01ff) AM_RAM
	AM_RANGE(0x4016, 0x4016) AM_READ(soundlatch3_r)		// overwrite default
	AM_RANGE(0x4000, 0x4017) AM_READ(NESPSG_1_r)
	AM_RANGE(0x4000, 0x4017) AM_WRITE(NESPSG_1_w)
	AM_RANGE(0xe000, 0xffff) AM_ROM
ADDRESS_MAP_END

/*************************************
 *
 *  Sound interfaces
 *
 *************************************/

static const struct NESinterface nes_interface_1 = { "n2a03a" };
static const struct NESinterface nes_interface_2 = { "n2a03b" };

/*************************************
 *
 *  Machine driver
 *
 *************************************/

MACHINE_DRIVER_START( dkong2b_audio )

	MDRV_CPU_ADD("sound", I8035,I8035_CLOCK)
	MDRV_CPU_PROGRAM_MAP(dkong_sound_map,0)
	MDRV_CPU_IO_MAP(dkong_sound_io_map, 0)

	MDRV_SOUND_START(dkong)
	MDRV_SOUND_RESET(dkong)

	MDRV_SPEAKER_STANDARD_MONO("mono")
	MDRV_SOUND_ADD("discrete", DISCRETE, 0)
	MDRV_SOUND_CONFIG_DISCRETE(dkong2b)
	MDRV_SOUND_ROUTE(ALL_OUTPUTS, "mono", 1.0)

MACHINE_DRIVER_END

MACHINE_DRIVER_START( radarscp_audio )

	MDRV_IMPORT_FROM( dkong2b_audio )
	MDRV_SOUND_MODIFY("discrete")
	MDRV_SOUND_CONFIG_DISCRETE(radarscp)
	MDRV_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.7)

MACHINE_DRIVER_END


MACHINE_DRIVER_START( radarsc1_audio )

	MDRV_IMPORT_FROM( radarscp_audio )
	MDRV_CPU_MODIFY("sound")
	MDRV_CPU_IO_MAP(radarsc1_sound_io_map, 0)

	MDRV_SOUND_ADD("tms", M58817, XTAL_640kHz)
	MDRV_SOUND_ROUTE(ALL_OUTPUTS, "mono", 1.0)

MACHINE_DRIVER_END

MACHINE_DRIVER_START( dkongjr_audio )

	MDRV_CPU_ADD("sound", I8035,I8035_CLOCK)
	MDRV_CPU_PROGRAM_MAP(dkong_sound_map,0)
	MDRV_CPU_IO_MAP(dkongjr_sound_io_map, 0)

	MDRV_SOUND_START(dkong)
	MDRV_SOUND_RESET(dkongjr)

	MDRV_SPEAKER_STANDARD_MONO("mono")

	MDRV_SOUND_ADD("discrete", DISCRETE, 0)
	MDRV_SOUND_CONFIG_DISCRETE(dkongjr)
	MDRV_SOUND_ROUTE(ALL_OUTPUTS, "mono", 1.0)

MACHINE_DRIVER_END

MACHINE_DRIVER_START( dkong3_audio )

	MDRV_CPU_ADD("n2a03a", N2A03,N2A03_DEFAULTCLOCK)
	MDRV_CPU_PROGRAM_MAP(dkong3_sound1_map, 0)
	MDRV_CPU_VBLANK_INT("main", nmi_line_pulse)

	MDRV_CPU_ADD("n2a03b", N2A03,N2A03_DEFAULTCLOCK)
	MDRV_CPU_PROGRAM_MAP(dkong3_sound2_map, 0)
	MDRV_CPU_VBLANK_INT("main", nmi_line_pulse)

	MDRV_SPEAKER_STANDARD_MONO("mono")
	MDRV_SOUND_ADD("nes1", NES, N2A03_DEFAULTCLOCK)
	MDRV_SOUND_CONFIG(nes_interface_1)
	MDRV_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.50)

	MDRV_SOUND_ADD("nes2", NES, N2A03_DEFAULTCLOCK)
	MDRV_SOUND_CONFIG(nes_interface_2)
	MDRV_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.50)

MACHINE_DRIVER_END


