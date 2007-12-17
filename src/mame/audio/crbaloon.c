#include "driver.h"
#include "crbaloon.h"


/************************************************************************
* crbaloon Sound System Analog emulation
* Jan 2006, Derrick Renaud
************************************************************************/


/* Timing Sources */
#define CRBALOON_16H			9987000.0/2/2/16

/* Nodes - Adjusters */
#define CRBALOON_VR2			NODE_05
#define CRBALOON_VR3			NODE_06

/* Nodes - Sounds */
#define CRBALOON_LAUGH_SND		NODE_80
#define CRBALOON_MUSIC_SND		NODE_81

static const discrete_555_desc desc_crbaloon_laugh_osc =
{
	DISC_555_OUT_SQW | DISC_555_OUT_DC,
	5,				// B+ voltage of 555
	DEFAULT_555_VALUES
};

static const discrete_dac_r1_ladder desc_crbaloon_music_dac =
{
	3,
	{0, RES_K(470), RES_K(120)},
	5, RES_K(470), 0, 0
};

DISCRETE_SOUND_START(crbaloon)

	/************************************************
    * Input register mapping
    ************************************************/
	DISCRETE_INPUT_LOGIC(CRBALOON_LAUGH_EN)
	DISCRETE_INPUT_LOGIC(CRBALOON_MUSIC_EN)
	DISCRETE_INPUT_DATA (CRBALOON_MUSIC_DATA)

	DISCRETE_ADJUSTMENT_TAG(CRBALOON_VR2, 1, 0, 0.5, DISC_LINADJ, "VR2")
	DISCRETE_ADJUSTMENT_TAG(CRBALOON_VR3, 1, 0, 1,   DISC_LINADJ, "VR3")

	/************************************************
    * Laugh is a VCO modulated by a constant
    * square wave.
    ************************************************/
	DISCRETE_555_ASTABLE(NODE_10, 1, RES_K(10), RES_K(100), CAP_U(1), &desc_crbaloon_laugh_osc)
	DISCRETE_CRFILTER_VREF(NODE_11, 1, NODE_10,
		1.0/(1.0/RES_K(5) + 1.0/RES_K(10) + 1.0/RES_K(100)), // 5k & 10k are 555 internal
		CAP_U(10),
		/* The following will calculate the reference control voltage with no signal applied to the cap. */
		5.0* (1.0/(1.0/RES_K(10) + 1.0/RES_K(100))) / (RES_K(5)+(1.0/(1.0/RES_K(10) + 1.0/RES_K(100)))) )
	DISCRETE_555_ASTABLE_CV(NODE_12, CRBALOON_LAUGH_EN, RES_K(1), RES_K(22), CAP_U(.1), NODE_11, &desc_crbaloon_laugh_osc)
	DISCRETE_MULTIPLY(NODE_13, 1, NODE_12, CRBALOON_VR2)
	DISCRETE_CRFILTER(CRBALOON_LAUGH_SND, 1, NODE_13, RES_K(20), CAP_U(1))

	/************************************************
    * Music Generator is amplitude modulated by a
    * linear ramp.
    ************************************************/
	DISCRETE_NOTE(NODE_20, 1, CRBALOON_16H / 2, CRBALOON_MUSIC_DATA, 255, 7, DISC_CLK_IS_FREQ)
	DISCRETE_DAC_R1(NODE_21, CRBALOON_MUSIC_EN, NODE_20, DEFAULT_TTL_V_LOGIC_1, &desc_crbaloon_music_dac)
//  DISCRETE_RAMP(NODE_21, 1, RAMP, GRAD, MIN, MAX, CLAMP)
	DISCRETE_MULTIPLY(NODE_22, 1, NODE_21, CRBALOON_VR3)
	DISCRETE_CRFILTER(CRBALOON_MUSIC_SND, 1, NODE_22, RES_K(50), CAP_U(1))

	/************************************************
    * Final mix and output.
    ************************************************/
	DISCRETE_ADDER2(NODE_90, 1, CRBALOON_LAUGH_SND, CRBALOON_MUSIC_SND)
	DISCRETE_CRFILTER(NODE_91, 1, NODE_90, RES_K(100), CAP_U(1))

	DISCRETE_OUTPUT(NODE_91, 65000.0/12)

DISCRETE_SOUND_END
