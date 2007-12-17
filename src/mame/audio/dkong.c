#include "driver.h"
#include "cpu/i8039/i8039.h"
#include "sound/samples.h"
#include "includes/dkong.h"
#include "sound/discrete.h"
#include "sound/dac.h"


/****************************************************************
 *
 * Diskrete Sound defines
 *
 ****************************************************************/

/* Discrete sound inputs */

#define DS_SOUND0_INV		NODE_01
#define DS_SOUND1_INV		NODE_02
#define DS_SOUND2_INV		NODE_03
#define DS_SOUND6_INV		NODE_04
#define DS_SOUND7_INV		NODE_05
#define DS_DAC				NODE_06
#define DS_DAC_DISCHARGE	NODE_07

#define DS_SOUND0			NODE_208
#define DS_SOUND1			NODE_209
#define DS_SOUND2			NODE_210
#define DS_SOUND6			NODE_211
#define DS_SOUND7			NODE_212

#define DS_ADJ_DAC			NODE_240

#define DS_OUT_SOUND0		NODE_241
#define DS_OUT_SOUND1		NODE_242
#define DS_OUT_SOUND2		NODE_243
#define DS_OUT_SOUND6		NODE_247
#define DS_OUT_SOUND7		NODE_248
#define DS_OUT_DAC			NODE_249

/* Input definitions for write handlers */

#define DS_SOUND0_INP		DS_SOUND0_INV
#define DS_SOUND1_INP		DS_SOUND1_INV
#define DS_SOUND2_INP		DS_SOUND2_INV
#define DS_SOUND6_INP		DS_SOUND6_INV
#define DS_SOUND7_INP		DS_SOUND7_INV

/* General defines */

#define DK_1N5553_V			0.4	// from datasheet at 1mA
#define DK_SUP_V			5.0
#define NE555_INTERNAL_R	RES_K(5)

#define R_PARALLEL(R1,R2) ((R1)*(R2)/((R1)+(R2)))
#define R_SERIE(R1,R2)	  ((R1)+(R2))

/****************************************************************
 *
 * Static declarations
 *
 ****************************************************************/

static UINT8 dkongjr_latch[10];
static UINT8 sh_climb_count;

static UINT8 has_discrete_interface = 0;

static UINT8 page,mcustatus;
static UINT8 p[8];
static UINT8 t[2];
static double envelope,tt;
static UINT8 decay;
static UINT8 sh1_count = 0;

enum {
	WAIT_CMD,
	WAIT_WRITE,
	WAIT_DONE1,
	WAIT_DONE2
} m58817_states;

static UINT8 m58817_state;
static UINT8 m58817_drq;
static UINT8 m58817_nibbles[4];
static INT32 m58817_count;
static INT32 m58817_address;

/****************************************************************
 *
 * Dkong Discrete Sound Interface
 *
 ****************************************************************/

/* Variable components */

#define DK_VR1		RES_K(10)

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

#define CAP_U_AGED(x) (1*CAP_U(x))

#define DK_C8		CAP_U_AGED(220)
#define DK_C12		CAP_U_AGED(1)
#define DK_C13		CAP_U(33)
#define DK_C16		CAP_U_AGED(1)
#define DK_C17		CAP_U_AGED(4.7)
#define DK_C18		CAP_U_AGED(1)
#define DK_C19		CAP_U_AGED(1)
#define DK_C20		CAP_U_AGED(3.3)
#define DK_C21		CAP_U_AGED(1)

#define DK_C23		CAP_U_AGED(4.7)
#define	DK_C24		CAP_U_AGED(10)
#define DK_C25		CAP_U_AGED(3.3)
#define	DK_C26		CAP_U_AGED(3.3)
#define	DK_C29		CAP_U_AGED(3.3)
#define DK_C30		CAP_U_AGED(10)
#define DK_C32		CAP_U_AGED(10)
#define DK_C34		CAP_N(100)
#define DK_C160		CAP_N(100)


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
		0, DK_VR1,
		DK_C160,
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

DISCRETE_SOUND_START(dkong)

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
	DISCRETE_MIXER4(NODE_54, 1, NODE_50, NODE_51, DK_SUP_V, 0,&dkong_rc_jump_desc)

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
     * The second opamp I tried to modell in the transform.
     */

    DISCRETE_RCFILTER(NODE_72,1,NODE_71,(DK_R21+DK_R22),DK_C34)
	DISCRETE_TRANSFORM4(NODE_73,1,NODE_72,NODE_71,-1,NODE_73,"012*3+-")

	/* Adjustment VR2 */
	DISCRETE_MULTIPLY(DS_OUT_DAC, 1, NODE_73, DS_ADJ_DAC)

	/************************************************/
	/* Amplifier                                    */
	/************************************************/

	DISCRETE_MIXER4(NODE_288, 1, DS_OUT_SOUND0, DS_OUT_SOUND1, DS_OUT_DAC, DS_OUT_SOUND2, &dkong_mixer_desc)

#if 1
	/* This filter should simulate gain vs. frequency behaviour of MB3712 */
	//DISCRETE_FILTER1(NODE_294,1,NODE_288,80,DISC_FILTER_HIGHPASS)

	/* The following is the CR filter by the speaker and C8 */
	/* 4 Ohm is from MB3712 Spec Sheet */
	DISCRETE_CRFILTER(NODE_295,1,NODE_288, 4, DK_C8)
	DISCRETE_OUTPUT(NODE_295, 32767.0/5.0*30)
#else
	// Amplifier: internal amplifier
	DISCRETE_ADDER2(NODE_289,1,NODE_288,0.5+5.0*150.0/(150.0+1000.0))
    DISCRETE_RCINTEGRATE(NODE_294,1,NODE_289,0,150,1000, CAP_U(33),DK_SUP_V,DISC_RC_INTEGRATE_TYPE3)
	DISCRETE_CRFILTER(NODE_295,1,NODE_294, 50, DK_C13)
	DISCRETE_OUTPUT(NODE_295, 32767.0/5.0)
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
#define RS_R18		RES_K(4.7)    // ????
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
		RS_C51,
		0,
		0, 1};

/* There is no load on the output for the jump circuit
 * For the walk circuit, the voltage does not matter */

static const discrete_555_desc radarscp_555_vco_desc =
	{DISC_555_OUT_DC | DISC_555_OUT_SQW,
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

DISCRETE_SOUND_START(radarscp)

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
    DISCRETE_555_ASTABLE_CV(NODE_43, DS_SOUND6, RES_K(47), RES_K(27), CAP_N(10), NODE_42, &dkong_555_vco_desc)

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
    DISCRETE_555_ASTABLE_CV(NODE_54, DS_SOUND7, RES_K(47), RES_K(27), CAP_N(33), NODE_53, &dkong_555_vco_desc)

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
     * The second opamp I tried to modell in the transform.
     */

    DISCRETE_RCFILTER(NODE_172,1,NODE_171,(RS_R22+RS_R23),RS_C38)
	DISCRETE_TRANSFORM4(NODE_173,1,NODE_172,NODE_171,-1,NODE_173,"012*3+-")

	/* Adjustment VR3 */
	DISCRETE_MULTIPLY(DS_OUT_DAC, 1, NODE_173, DS_ADJ_DAC)

	/************************************************/
	/* Amplifier                                    */
	/************************************************/

	DISCRETE_MIXER5(NODE_288, 1, DS_OUT_SOUND0, DS_OUT_SOUND1, DS_OUT_SOUND2, DS_OUT_SOUND7, DS_OUT_DAC, &radarscp_mixer_desc)

#if 1
	/* This filter should simulate gain vs. frequency behaviour of MB3712 */
	//DISCRETE_FILTER1(NODE_291,1,NODE_289,80,DISC_FILTER_HIGHPASS)

	/* The following is the CR filter by the speaker and C8 */
	/* 4 Ohm is from MB3712 Spec Sheet */
	DISCRETE_CRFILTER(NODE_295,1,NODE_288, 4, RS_C5)
	DISCRETE_OUTPUT(NODE_295, 32767.0/5.0*10)
#else
	// Amplifier: internal amplifier
	DISCRETE_ADDER2(NODE_289,1,NODE_288,0.5+5.0*150.0/(150.0+1000.0))
    DISCRETE_RCINTEGRATE(NODE_294,1,NODE_289,0,150,1000, CAP_U(33),DK_SUP_V,DISC_RC_INTEGRATE_TYPE3)
	DISCRETE_CRFILTER(NODE_295,1,NODE_294, 50, DK_C13)
	DISCRETE_OUTPUT(NODE_295, 32767.0/5.0)
#endif

	//DISCRETE_CSVLOG3(DS_SOUND2,NODE_13,NODE_14)
	//DISCRETE_CSVLOG3(DS_SOUND0,NODE_54,NODE_51)
	//DISCRETE_CRFILTER(NODE_299,1,NODE_288, 50, DK_C13)
	//DISCRETE_WAVELOG2(NODE_295, 32767/5.0, NODE_299, 32767.0/5.0*20)
	//DISCRETE_WAVELOG2(NODE_57, 32767/5.0, NODE_56, 32767/5.0)
DISCRETE_SOUND_END


/****************************************************************
 *
 * Initialization
 *
 ****************************************************************/

SOUND_START( dkong )
{
	state_save_register_global(page);
	state_save_register_global(mcustatus);
	state_save_register_global_array(p);
	state_save_register_global_array(t);
	state_save_register_global(envelope);
	state_save_register_global(tt);
	state_save_register_global(decay);
	state_save_register_global(sh1_count);

	sh1_count = 0;
	mcustatus = 0;
	envelope = 0;
	tt = 0;
	decay = 0;

	page = 0;
	p[0] = p[1] = p[2] = p[3] = p[4] = p[5] = p[6] = p[7] = 255;
	t[0] = t[1] = 1;

	has_discrete_interface = 1;
}

SOUND_START( dkongjr )
{
	sound_start_dkong(machine);

	state_save_register_global(sh_climb_count);
	state_save_register_global_array(dkongjr_latch);

	sh_climb_count = 0;

	has_discrete_interface = 0;
}

SOUND_START( hunchbkd  )
{
	sound_start_dkong(machine);

	has_discrete_interface = 0;
}

static int dumpframe(int startbit);

SOUND_START( radarsc1  )
{
	sound_start_dkong(machine);

	m58817_state=WAIT_CMD;
	m58817_drq=0;
	m58817_count=0;
	m58817_address=0;

	state_save_register_global(m58817_state);
	state_save_register_global(m58817_drq);
	state_save_register_global_array(m58817_nibbles);
	state_save_register_global(m58817_count);
	state_save_register_global(m58817_address);

	has_discrete_interface = 1;
}

/****************************************************************
 *
 * I/O Handlers
 *
 ****************************************************************/


READ8_HANDLER( dkong_sh_p1_r )
{
	return p[1];
}

READ8_HANDLER( dkong_sh_p2_r )
{
	return p[2];
}

READ8_HANDLER( dkong_sh_t0_r )
{
	return t[0];
}

READ8_HANDLER( dkong_sh_t1_r )
{
	return t[1];
}

READ8_HANDLER( dkong_sh_tune_r )
{
	UINT8 *SND = memory_region(REGION_CPU2);
	if ((page & 0x40) && (offset==0x20))
		return soundlatch_r(0);
	else
		return (SND[2048+(page & 7)*256+offset]);
}

#define TSTEP 0.001

WRITE8_HANDLER( dkong_sh_p1_w )
{

	if (has_discrete_interface)
		discrete_sound_w(DS_DAC,data);
	else
	{
		envelope=exp(-tt);
		DAC_data_w(0,(int)(data*envelope));
		if (decay)
			tt+=TSTEP;
		else
			tt=0;
	}
}

READ8_HANDLER( radarsc1_sh_tune_r )
{
	// always offset 0x20
	return soundlatch_r(0);
}


/* @0x510, cpu2
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
 0: 14 16
 1: 14 18
 2: 14 1A
 3: 1C
 4: 1E 1E
 5: 10 10 10
 6: 12 12
 7: 20
*/

static UINT8 m58817_getbit(int bitnum)
{
	const UINT8 *table = memory_region(REGION_SOUND1);
	return (table[bitnum >> 3] >> (0x07 - (bitnum & 0x07))) & 1;
}

static UINT32 m58817_getbits(int startbit, int num)
{
	int i;
	UINT32 r=0;

	for (i=0;i<num;i++)
	{
		r = r << 1;
		r = r | m58817_getbit(startbit++);
	}
	return r;
}

static int decodeframe(int startbit)
{
	int energy;
	int pitch;
	int repeat;
	UINT32 kx;

	energy = m58817_getbits(startbit, 4);
	logerror("frame @ bit %04d (%02x):", startbit, startbit>>3);
	logerror(" energy = %d ", energy);
	startbit += 4;
	if (energy==0)
	{
		logerror(" - silence\n");
		return startbit;
	}
	if (energy==15)
	{
		logerror(" - stop\n");
		return -1; /* stop ... */
	}
	repeat = m58817_getbits(startbit, 1);
	startbit += 1;
	pitch = m58817_getbits(startbit, 6);
	startbit += 6;
	logerror(" pitch = %d ", pitch);
	if (repeat)
	{
		logerror(" - repeat\n");
		return startbit;
	}
	if (pitch == 0)
	{
		logerror(" - unvoiced skip %d\n", 18);
		kx = m58817_getbits(startbit, 18);
		startbit += 18;
		return startbit;
	}
	else
	{
		logerror(" - voiced skip %d\n", 37);
		kx = m58817_getbits(startbit, 30);
		startbit += 30;
		kx = m58817_getbits(startbit, 7);
		startbit += 7;
		return startbit;
	}
}

static int dumpframe(int startbit)
{
	while (startbit>=0)
		startbit=decodeframe(startbit);
	logerror("\n");
	return startbit;
}

static void m58817_state_loop(int data)
{
	int i;
	switch (m58817_state)
	{
		case WAIT_CMD:
			switch (data)
			{
				case 0x00: // reset ????
					m58817_count=0;
					break;
				case 0x02: // latch next nibbel
					m58817_state=WAIT_WRITE;
					break;
				case 0x08: // play ????
					m58817_state=WAIT_DONE1;
					break;
				default:
					logerror("m58817: unknown cmd : 0x%02x\n", data);
			}
			break;
		case WAIT_WRITE:
			m58817_nibbles[m58817_count++] = data & 0x0f;
			m58817_state=WAIT_CMD;
			break;
		case WAIT_DONE1:
			if (data != 0x0A)
				logerror("m58817: expected 0x0A got 0x%02x\n", data);
			m58817_address = 0;
			for (i=0;i<m58817_count;i++)
			{
				m58817_address |= (m58817_nibbles[i] << (i*4));
			}
			logerror("m58817: address: 0x%04x\n", m58817_address);
			dumpframe(m58817_address * 8);
			m58817_state=WAIT_CMD;
			break;
	}
}

static READ8_HANDLER( M58817_status_r)
{
	return !(m58817_state == WAIT_CMD);
}

static WRITE8_HANDLER( M58817_command_w)
{
	int drq = (data>>4) & 0x01; // FIXME 0x20 ??
	int dat = data & 0x0f;
	logerror("PA Write %x\n", data);
	if (!drq & m58817_drq)
		m58817_state_loop(dat);
	m58817_drq = drq;
}

READ8_HANDLER( radarsc1_sh_p1_r )
{
	int r;

	r = (p[1] & 0x80) | (M58817_status_r(0)<<6);
	//logerror("PA Ret %x\n", r);
	return r;
}

WRITE8_HANDLER( radarsc1_sh_p1_w )
{
	M58817_command_w(0,data);
}

WRITE8_HANDLER( radarsc1_sh_p2_w )
{
	/*   If P2.Bit7 -> external signal decay
     *   If P2.Bit6 -> not connected
     *   If P2.Bit5 -> Signal ANSN ==> Grid enable
     *   If P2.Bit4 -> status code to main cpu
     *   P2.Bit2-0  -> select the 256 byte bank for external ROM
     */

	if (has_discrete_interface)
		discrete_sound_w(DS_DAC_DISCHARGE, (data & 0x80) ? 0 : 1 );
	else
		decay = !(data & 0x80);
	page = (data & 0x07);
	mcustatus = ((~data & 0x10) >> 4);
	radarsc1_ansn_w(0, (data & 0x20) >> 5);
}

WRITE8_HANDLER( dkong_sh_p2_w )
{
	/*   If P2.Bit7 -> is apparently an external signal decay or other output control
     *   If P2.Bit6 -> activates the external compressed sample ROM
     *   If P2.Bit4 -> status code to main cpu
     *   P2.Bit2-0  -> select the 256 byte bank for external ROM
     */

	if (has_discrete_interface)
		discrete_sound_w(DS_DAC_DISCHARGE, (data & 0x80) ? 0 : 1 );
	else
		decay = !(data & 0x80);
	page = (data & 0x47);
	mcustatus = ((~data & 0x10) >> 4);
}

#define ACTIVELOW_PORT_BIT(P,A,D)   ((P & (~(1 << A))) | (((D) ^ 1) << A))

WRITE8_HANDLER( dkong_sh_tuneselect_w )
{
	soundlatch_w(offset,data ^ 0x0f);
}

WRITE8_HANDLER( dkongjr_sh_test6_w )
{
	p[2] = ACTIVELOW_PORT_BIT(p[2],6,data);
}

WRITE8_HANDLER( dkongjr_sh_test5_w )
{
	p[2] = ACTIVELOW_PORT_BIT(p[2],5,data);
}

WRITE8_HANDLER( dkongjr_sh_test4_w )
{
	p[2] = ACTIVELOW_PORT_BIT(p[2],4,data);
}

WRITE8_HANDLER( dkongjr_sh_tuneselect_w )
{
	soundlatch_w(offset,data);
}


WRITE8_HANDLER( dkong_sh_w )
{
	if (data)
		cpunum_set_input_line(1, 0, ASSERT_LINE);
	else
		cpunum_set_input_line(1, 0, CLEAR_LINE);
}

WRITE8_HANDLER( dkong_snd_disc_w )
{
	if (!has_discrete_interface && (offset<3))
	{
		logerror("dkong.c: Write to snd port %d (%d)\n", offset, data);
		return;
	}
	switch (offset)
	{
		case 0:
			discrete_sound_w(DS_SOUND0_INP,data & 1);
			break;
		case 1:
			discrete_sound_w(DS_SOUND1_INP,data & 1);
			break;
		case 2:
			discrete_sound_w(DS_SOUND2_INP,data & 1);
			radarscp_snd02_w(0, data & 1);
			break;
		case 3:
			p[2] = ACTIVELOW_PORT_BIT(p[2],5,data);
			break;
		case 4:
			t[1] = ~data & 1;
			break;
		case 5:
			t[0] = ~data & 1;
			break;
		case 6:
			if (has_discrete_interface)
				discrete_sound_w(DS_SOUND6_INP,data & 1);
			break;
		case 7:
			if (has_discrete_interface)
				discrete_sound_w(DS_SOUND7_INP,data & 1);
			break;
	}
	return;
}

WRITE8_HANDLER( radarsc1_snd_disc_w )
{
	if (!has_discrete_interface && (offset<3))
	{
		logerror("dkong.c: Write to snd port %d (%d)\n", offset, data);
		if (offset == 2)
			radarscp_snd02_w(0, data & 1);
		return;
	}
	switch (offset)
	{
		case 0:
			discrete_sound_w(DS_SOUND0_INP,data & 1);
			break;
		case 1:
			discrete_sound_w(DS_SOUND1_INP,data & 1);
			break;
		case 2:
			discrete_sound_w(DS_SOUND2_INP,data & 1);
			radarscp_snd02_w(0, data & 1);
			break;
		case 3:  //SOUND3 ==> PA7
			p[1] = ACTIVELOW_PORT_BIT(p[1],7,data & 1);
			break;
		case 4:
			t[1] = ~data & 1;
			break;
		case 5:
			t[0] = ~data & 1;
			break;
		case 6:
			if (has_discrete_interface)
				discrete_sound_w(DS_SOUND6_INP,data & 1);
			break;
		case 7:
			if (has_discrete_interface)
				discrete_sound_w(DS_SOUND7_INP,data & 1);
			break;
	}
	return;
}

READ8_HANDLER( dkong_in2_r )
{
	return input_port_2_r(offset) | (mcustatus << 6);
}

WRITE8_HANDLER( dkongjr_snd_w1 )
{
	static const int sample_order[7] = {1,2,1,2,0,1,0};

	if (dkongjr_latch[offset] != data)
	{
		switch (offset)
		{
			case 0:			/* climb */
				if (data && dkongjr_latch[7] == 0)
				{
					sample_start (3,sample_order[sh_climb_count]+3,0);
					sh_climb_count++;
					if (sh_climb_count == 7) sh_climb_count = 0;
				}
				else if (data && dkongjr_latch[7] == 1)
				{
					sample_start (3,sample_order[sh_climb_count]+8,0);
					sh_climb_count++;
					if (sh_climb_count == 7) sh_climb_count = 0;
				}
				break;
			case 1:			/* jump */
		if (data)
					sample_start (6,0,0);
				break;
			case 2:			/* land */
				if (data)
					sample_stop (7);
				sample_start (4,1,0);
				break;
			case 3:			/* roar */
				if (data)
					sample_start (7,2,0);
				break;
			case 4:			/* Port 4 write */
					t[1] = ~data & 1;
				break;
			case 5:			/* Port 5 write */
					t[0] = ~data & 1;
				break;
			case 6:			/* snapjaw */
				if (data)
					sample_stop (7);
				sample_start (4,11,0);
				break;
			case 7:			/* walk */
				//walk = data;
				break;
				}
		dkongjr_latch[offset] = data;
	}
}

WRITE8_HANDLER( dkongjr_snd_w2 )
{
	if (dkongjr_latch[offset+8] != data)
	{
		switch (offset)
		{
		case 0:			/* death */
			if (data)
				sample_stop (7);
			sample_start (6, 6, 0);
			break;
		case 1:			/* drop */
			if (data)
				sample_start (7, 7, 0);
			break;
		}
		dkongjr_latch[offset+8] = data;
	}
}
