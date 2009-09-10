/************************************************************************
 * norautp Sound System Analog emulation
 * Sept 2009, Derrick Renaud
 ************************************************************************/

#include "driver.h"
#include "norautp.h"


/* Discrete Sound Input Nodes */
/* defined in norautp.h */

/* Parts List - Resistors */
#define NORAUTP_R1		RES_K(120)
#define NORAUTP_R2		RES_K(2.2)

/* Parts List - Capacitors */
#define NORAUTP_C1		CAP_U(.009)		/* The real value is .01, but using .009 the tone for this line is accurate */
#define NORAUTP_C2		CAP_U(.022)
#define NORAUTP_C3		CAP_U(.047)
#define NORAUTP_C4		CAP_U(.01)


static const discrete_555_desc desc_norautp_555 =
{
	DISC_555_OUT_ENERGY | DISC_555_OUT_AC,
	5,				// B+ voltage of 555
	DEFAULT_555_VALUES
};

static const discrete_comp_adder_table desc_norautp_caps =
{
	DISC_COMP_P_CAPACITOR, NORAUTP_C4, 3,
	{
		NORAUTP_C3, NORAUTP_C2, NORAUTP_C1
	}
};


DISCRETE_SOUND_START( norautp )
	/************************************************
     * Input register mapping
     ************************************************/
	DISCRETE_INPUT_LOGIC(NORAUTP_SND_EN)
	DISCRETE_INPUT_DATA (NORAUTP_FREQ_DATA)

	/************************************************
     * Tone Generator
     ************************************************/
	DISCRETE_COMP_ADDER(NODE_20, NORAUTP_FREQ_DATA, &desc_norautp_caps)
	DISCRETE_555_ASTABLE(NODE_21,
						 NORAUTP_SND_EN,		/* RESET */
						 NORAUTP_R2, NORAUTP_R1, NODE_20, &desc_norautp_555)

	DISCRETE_OUTPUT(NODE_21, 65000.0/3.8)

DISCRETE_SOUND_END
