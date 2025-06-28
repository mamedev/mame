// license:BSD-3-Clause
// copyright-holders:Derrick Renaud,Hans Andersson
/***************************************************************************

    Sigma Spiders hardware

To Do: add filters

***************************************************************************/

#include "emu.h"
#include "spiders.h"

#include "machine/6821pia.h"
#include "sound/discrete.h"
#include "speaker.h"


/* Discrete Sound Input Nodes */
#define SPIDERS_WEB_SOUND_DATA      NODE_01
#define SPIDER_WEB_SOUND_MOD_DATA   NODE_02
#define SPIDERS_FIRE_EN             NODE_03
#define SPIDERS_EXP_EN              NODE_04
#define SPIDERS_SUPER_WEB_EN        NODE_05
#define SPIDERS_SUPER_WEB_EXPL_EN   NODE_06
#define SPIDERS_X_EN                NODE_07

#define SPIDERS_SOUND_CLK 6000000 /* 6 MHZ*/


/************************************************************************/
/* Spiders Sound System Analog emulation                                */
/* Written by Hans Andersson,  Aug 2005                                 */
/* Schematics by Don Maeby                                              */
/*                                                                      */
/* Todo - Add proper sound mixer                                        */
/************************************************************************/

static const discrete_dac_r1_ladder spiders_sound_dac =
{
	4,          // size of ladder
	{RES_K(68), RES_K(47), RES_K(33), RES_K(33)}, // R21, R31, R30, R50
	DEFAULT_TTL_V_LOGIC_1,
	0,          // no rBias
	0,          // no rGnd
	0           // no cap
};

/* The noise generator consists of two LS164 plus a LS74, so the length is 8+8+1 */
static const discrete_lfsr_desc spiders_lfsr =
{
	DISC_CLK_IS_FREQ,
	17,                   /* Bit Length */
	0,                    /* Reset Value */
	4,                    /* Use Bit 4 (QE of first LS164) as F0 input 0 */
	16,                   /* Use Bit 16 (LS74) as F0 input 1 */
	DISC_LFSR_XOR_INV_IN1,/* F0 is XOR with input 1 inverted */
	DISC_LFSR_IN0,        /* F1 is 1*F0*/
	DISC_LFSR_REPLACE,    /* F2 replaces the shifted register contents */
	0x000001,             /* Everything is shifted into the first bit only */
	1,                    /* Output is inverted Q of LS74*/
	16                    /* Output bit */
};


static const discrete_dac_r1_ladder spiders_fire_dac =
{
	1,
	{RES_K(10)},    // R29
	5,              // 555 Vcc
	RES_K(5),       // 555 internal
	RES_K(10),      // 555 internal
	CAP_N(100)      // C100
};

static const discrete_dac_r1_ladder spiders_web_exp_dac =
{
	1,
	{RES_K(10)},    // R44
	5,              // 555 Vcc
	RES_K((5*2.2)/(5+2.2)),     // 555 internal (5k) // R49 (2.2k)
	RES_K(10),      // 555 internal
	CAP_U(0.01)     // C25
};

// IC 10
static const discrete_555_desc spiders_fire_555a =
{
	DISC_555_OUT_SQW | DISC_555_OUT_DC,
	5,      // B+ voltage of 555
	DEFAULT_555_VALUES
};


static const discrete_555_desc spiders_super_web_555a =
{
	DISC_555_OUT_CAP | DISC_555_OUT_DC,
	5,      // B+ voltage of 555
	DEFAULT_555_VALUES
};


/* Nodes - Sounds */

#define SPIDERS_WEB_SOUND   NODE_08
#define SPIDERS_NOISE       NODE_09
#define SPIDERS_FIRE        NODE_10
#define SPIDERS_EXPL        NODE_11
#define SPIDERS_SW_EXPL     NODE_12
#define SPIDERS_SW          NODE_13
#define SPIDERS_X           NODE_14

static DISCRETE_SOUND_START(spiders_discrete)

	/************************************************/
	/* Input register mapping for spiders           */
	/************************************************/

	DISCRETE_INPUT_DATA (SPIDERS_WEB_SOUND_DATA)
	DISCRETE_INPUT_DATA (SPIDER_WEB_SOUND_MOD_DATA)
	DISCRETE_INPUT_LOGIC(SPIDERS_FIRE_EN)
	DISCRETE_INPUT_LOGIC(SPIDERS_EXP_EN)
	DISCRETE_INPUT_LOGIC(SPIDERS_SUPER_WEB_EXPL_EN)
	DISCRETE_INPUT_LOGIC(SPIDERS_SUPER_WEB_EN)
	DISCRETE_INPUT_LOGIC(SPIDERS_X_EN)

	/* Web sound */
	DISCRETE_NOTE(NODE_20, 1, SPIDERS_SOUND_CLK/21, SPIDERS_WEB_SOUND_DATA, 255, 15, DISC_CLK_IS_FREQ)
	DISCRETE_TRANSFORM2(NODE_21, NODE_20, SPIDER_WEB_SOUND_MOD_DATA, "01&")
	DISCRETE_DAC_R1(NODE_22, NODE_21, DEFAULT_TTL_V_LOGIC_1, &spiders_sound_dac)

	/* Noise */
	/* LFSR NOISE Should be 3 MHz, but setting it to 6KHz should make no difference (except wasting less CPU cycles) since we sample it
	at 3.75 kHz */
	DISCRETE_LFSR_NOISE(NODE_30, 1, 1, SPIDERS_SOUND_CLK/(1000), 1, 0, 0.5, &spiders_lfsr)
	DISCRETE_SQUAREWFIX(NODE_31, 1, SPIDERS_SOUND_CLK/(10*10*16),1, 50.0, 0.5, 0)   /* 3.75 kHz */
	DISCRETE_SAMPLHOLD(NODE_32,NODE_30,NODE_31,DISC_SAMPHOLD_REDGE)

	/* Fire */
	DISCRETE_DAC_R1(NODE_40, NODE_32, DEFAULT_TTL_V_LOGIC_1, &spiders_fire_dac)

	// Fire envelope
	DISCRETE_MULTIPLY(NODE_41, DEFAULT_TTL_V_LOGIC_1, SPIDERS_FIRE_EN)
	DISCRETE_RCFILTER(NODE_42, NODE_41,RES_K(2.2),CAP_U(47)) //R11, C11
	DISCRETE_TRANSFORM4(NODE_43, NODE_40, NODE_42, 2.5, 24.5,"012*3/-")

	DISCRETE_555_ASTABLE_CV(NODE_44, 1, RES_K(10), RES_K(22), CAP_N(100), NODE_43, &spiders_fire_555a) //R10, R18, C12
	DISCRETE_MULTIPLY(NODE_45, DEFAULT_TTL_V_LOGIC_1-0.7, SPIDERS_FIRE_EN) // Diode drop D2
	DISCRETE_ASWITCH(NODE_46, NODE_44, NODE_45, DISC_CD4066_THRESHOLD)

	/* Explosion */
	DISCRETE_MULTIPLY(NODE_50, DEFAULT_TTL_V_LOGIC_1, SPIDERS_EXP_EN)
	DISCRETE_RCDISC5(NODE_51, 1, NODE_50, RES_M(1),CAP_U(0.47)) //R51, C8
	DISCRETE_MULTIPLY(NODE_52, DEFAULT_TTL_V_LOGIC_1, NODE_32) // Noise 0/1 -> TTL
	DISCRETE_ASWITCH(NODE_53, NODE_52, NODE_51, DISC_CD4066_THRESHOLD)

	/*Super web explosion*/
	DISCRETE_DAC_R1(NODE_60, NODE_32, DEFAULT_TTL_V_LOGIC_1, &spiders_web_exp_dac)
	DISCRETE_555_ASTABLE_CV(NODE_61, 1, RES_K(10), RES_K(22), CAP_N(22), NODE_60, &spiders_fire_555a)  //R48, R47, C24
	DISCRETE_MULTIPLY(NODE_62, DEFAULT_TTL_V_LOGIC_1, SPIDERS_SUPER_WEB_EXPL_EN)
	DISCRETE_RCDISC5(NODE_63, 1, NODE_62, RES_M(1),CAP_U(0.47)) //R9, C9
	DISCRETE_ASWITCH(NODE_64, NODE_61, NODE_63, DISC_CD4066_THRESHOLD)

	/*Super web */
	DISCRETE_555_ASTABLE_CV(NODE_70, 1, RES_K(330), 0, CAP_U(1), -1, &spiders_super_web_555a) //R46, C23
	DISCRETE_MULTIPLY(NODE_71, 57/47, NODE_70) // R27, R28
	DISCRETE_555_ASTABLE_CV(NODE_72, SPIDERS_SUPER_WEB_EN, RES_K(100), RES_K(330), CAP_U(0.01), NODE_71, &spiders_super_web_555a) //R22, R33, C16
	DISCRETE_555_ASTABLE_CV(NODE_73, SPIDERS_X_EN, RES_K(100), RES_K(470), CAP_N(47), NODE_71, &spiders_super_web_555a) // R23, R32, C15

	DISCRETE_ADDER4(NODE_89, 1, NODE_22, NODE_46, NODE_53, NODE_64)
	DISCRETE_ADDER3(NODE_90, 1, NODE_72, NODE_73, NODE_89)

	DISCRETE_OUTPUT(NODE_90, 10000)

DISCRETE_SOUND_END



void spiders_state::spiders_audio_command_w(uint8_t data)
{
	m_pia[3]->porta_w(data & 0xf8);
	m_pia[3]->ca1_w(BIT(data, 7));
}


void spiders_state::spiders_audio_a_w(uint8_t data)
{
	m_discrete->write(SPIDER_WEB_SOUND_MOD_DATA, 1 + (data & 4) * 8 + (data & 2) * 4 + (data & 1) * 2);
}

void spiders_state::spiders_audio_b_w(uint8_t data)
{
	m_discrete->write(SPIDERS_WEB_SOUND_DATA, data);
}


void spiders_state::spiders_audio_ctrl_w(uint8_t data)
{
	m_discrete->write(SPIDERS_FIRE_EN, data & 0x10 ? 1 : 0);
	m_discrete->write(SPIDERS_EXP_EN, data & 0x08 ? 1 : 0);
	m_discrete->write(SPIDERS_SUPER_WEB_EXPL_EN, data & 0x04 ? 1 : 0);
	m_discrete->write(SPIDERS_SUPER_WEB_EN, data & 0x02 ? 1 : 0);
	m_discrete->write(SPIDERS_X_EN, data & 0x01 ? 1 : 0);
}


void spiders_state::spiders_audio(machine_config &config)
{
	SPEAKER(config, "mono").front_center();
	DISCRETE(config, m_discrete, spiders_discrete).add_route(ALL_OUTPUTS, "mono", 1.0);
}
