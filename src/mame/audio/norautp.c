// license:???
// copyright-holders:Derrick Renaud, Roberto Fresca
/************************************************************************
 * Noraut/Kimble/Kramer Poker Sound System Analog emulation
 * Sept 2009, Derrick Renaud & Roberto Fresca.
 ************************************************************************/

#include "emu.h"
#include "includes/norautp.h"


/* Discrete Sound Input Nodes */
/* defined in norautp.h */


static const discrete_555_desc desc_norautp_555 =
{
	DISC_555_OUT_ENERGY | DISC_555_OUT_AC,
	5,              // B+ voltage of 555
	DEFAULT_555_VALUES
};


#define NORAUTP_SOUND_CIRCUIT(_name, _r1, _r2, _c1, _c2, _c3, _c4)          \
static const discrete_comp_adder_table desc_##_name##_caps =                \
{                                                                           \
	DISC_COMP_P_CAPACITOR, _c4, 3,                                          \
	{                                                                       \
		_c3, _c2, _c1                                                       \
	}                                                                       \
};                                                                          \
																			\
DISCRETE_SOUND_START( _name )                                               \
	/************************************************                       \
	 * Input register mapping                                               \
	 ************************************************/                      \
	DISCRETE_INPUT_LOGIC(NORAUTP_SND_EN)                                    \
	DISCRETE_INPUT_DATA (NORAUTP_FREQ_DATA)                                 \
																			\
	/************************************************                       \
	 * Tone Generator                                                       \
	 ************************************************/                      \
	DISCRETE_COMP_ADDER(NODE_20, NORAUTP_FREQ_DATA, &desc_##_name##_caps)   \
	DISCRETE_555_ASTABLE(NODE_21,                                           \
							NORAUTP_SND_EN,                 /* RESET */     \
							_r2, _r1, NODE_20, &desc_norautp_555)              \
																			\
	DISCRETE_OUTPUT(NODE_21, 65000.0/3.8)                                   \
DISCRETE_SOUND_END


/***** Noraut Poker *****/

/* Parts List - Resistors */
#define NORAUTP_R1      RES_K(120)
#define NORAUTP_R2      RES_K(2.2)

/* Parts List - Capacitors */
#define NORAUTP_C1      CAP_U(.01)
#define NORAUTP_C2      CAP_U(.022)
#define NORAUTP_C3      CAP_U(.047)
#define NORAUTP_C4      CAP_U(.01)

NORAUTP_SOUND_CIRCUIT(norautp,
	NORAUTP_R1, NORAUTP_R2,
	NORAUTP_C1, NORAUTP_C2, NORAUTP_C3, NORAUTP_C4)


/***** Draw Poker HI-LO *****/

/* Parts List - Resistors */
#define DPHL_R1     RES_K(120)
#define DPHL_R2     RES_K(1)

/* Parts List - Capacitors */
#define DPHL_C1     CAP_U(.01)
#define DPHL_C2     CAP_U(.022)
#define DPHL_C3     CAP_U(.05)
#define DPHL_C4     CAP_U(.01)

NORAUTP_SOUND_CIRCUIT(dphl,
	DPHL_R1, DPHL_R2,
	DPHL_C1, DPHL_C2, DPHL_C3, DPHL_C4)


/***** Kimble Double HI-LO *****/

/* Parts List - Resistors */
#define KIMBLE_R1       RES_K(100)
#define KIMBLE_R2       RES_K(1)

/* Parts List - Capacitors */
#define KIMBLE_C1       CAP_U(.01)
#define KIMBLE_C2       CAP_U(.022)
#define KIMBLE_C3       CAP_U(.047)
#define KIMBLE_C4       CAP_U(.01)

NORAUTP_SOUND_CIRCUIT(kimble,
	KIMBLE_R1, KIMBLE_R2,
	KIMBLE_C1, KIMBLE_C2, KIMBLE_C3, KIMBLE_C4)
