// license:BSD-3-Clause
// copyright-holders:Hans Andersson
/*************************************************************************

    audio\grchamp.c

*************************************************************************/
#include "emu.h"
#include "includes/grchamp.h"


/************************************************************************/
/* Grand Champion Analog emulation                                      */
/* Written by Hans Andersson,  dec 2005                                 */
/************************************************************************/

static const discrete_dac_r1_ladder grchamp_sound_dac1 =
{
	8,          // size of ladder
	{RES_K(680), RES_K(330), RES_K(150), RES_K(82), RES_K(39), RES_K(20), RES_K(10), RES_K(4.7)},
	0,
	0,          // no rBias
	0,          // no rGnd
	0           // no cap
};

static const discrete_dac_r1_ladder grchamp_sound_dac2 =
{
	8,          // size of ladder
	{RES_K(680), RES_K(330), RES_K(150), RES_K(82), RES_K(39), RES_K(20), RES_K(10), RES_K(4.7)},
	0,
	0,          // no rBias
	0,          // no rGnd
	0           // no cap
};

/* Nodes - Sounds */

DISCRETE_SOUND_START(grchamp)

	/************************************************/
	/* Input register mapping for grand champion    */
	/************************************************/

	DISCRETE_INPUT_LOGIC(GRCHAMP_ENGINE_CS_EN)
	DISCRETE_INPUT_DATA (GRCHAMP_SIFT_DATA)
	DISCRETE_INPUT_DATA (GRCHAMP_ATTACK_UP_DATA)
	DISCRETE_INPUT_NOT  (GRCHAMP_IDLING_EN)
	DISCRETE_INPUT_LOGIC(GRCHAMP_FOG_EN)
	DISCRETE_INPUT_DATA (GRCHAMP_PLAYER_SPEED_DATA)
	DISCRETE_INPUT_DATA (GRCHAMP_ATTACK_SPEED_DATA)
	DISCRETE_INPUT_DATA (GRCHAMP_A_DATA)
	DISCRETE_INPUT_DATA (GRCHAMP_B_DATA)

	DISCRETE_DAC_R1(NODE_20, GRCHAMP_A_DATA, DEFAULT_TTL_V_LOGIC_1, &grchamp_sound_dac1)

	DISCRETE_DAC_R1(NODE_21, GRCHAMP_B_DATA, DEFAULT_TTL_V_LOGIC_1, &grchamp_sound_dac2)

	DISCRETE_ADDER4(NODE_89, 1, 0, NODE_20, NODE_21, 0)

	DISCRETE_OUTPUT(NODE_89, 5000)

DISCRETE_SOUND_END
