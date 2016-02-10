// license:BSD-3-Clause
// copyright-holders:Nicola Salmoria,Derrick Renaud
/***************************************************************************

    Taito Crazy Balloon hardware

    Analog emulation - Jan 2006, Derrick Renaud

***************************************************************************/

#include "emu.h"
#include "includes/crbaloon.h"


/* timing sources */
#define CRBALOON_16H            (CRBALOON_MASTER_XTAL/2/2/16)

/* enables */
#define CRBALOON_LAUGH_EN       NODE_01
#define CRBALOON_MUSIC_EN       NODE_02
#define CRBALOON_MUSIC_DATA     NODE_03

/* nodes - adjusters */
#define CRBALOON_VR2            NODE_05
#define CRBALOON_VR3            NODE_06

/* nodes - sounds */
#define CRBALOON_LAUGH_SND      NODE_80
#define CRBALOON_MUSIC_SND      NODE_81



WRITE8_MEMBER(crbaloon_state::crbaloon_audio_set_music_freq)
{
	m_discrete->write(space, CRBALOON_MUSIC_DATA, data);
}


WRITE8_MEMBER(crbaloon_state::crbaloon_audio_set_music_enable)
{
	m_discrete->write(space, CRBALOON_MUSIC_EN, data);
}


void crbaloon_state::crbaloon_audio_set_explosion_enable(int enabled)
{
	m_sn->enable_w(enabled);
}


void crbaloon_state::crbaloon_audio_set_breath_enable(int enabled)
{
	/* changes slf_res to 10k (middle of two 10k resistors)
	   it also puts a tantal capacitor against GND on the output,
	   but this section of the schematics is not readable. */
	m_sn->slf_res_w(enabled ? RES_K(10) : RES_K(20) );
}


void crbaloon_state::crbaloon_audio_set_appear_enable(int enabled)
{
	/* APPEAR is connected to MIXER B */
	m_sn->mixer_b_w(enabled);
}


WRITE8_MEMBER(crbaloon_state::crbaloon_audio_set_laugh_enable)
{
	m_discrete->write(space, CRBALOON_LAUGH_EN, data);
}



static const discrete_555_desc desc_crbaloon_laugh_osc =
{
	DISC_555_OUT_SQW | DISC_555_OUT_DC,
	5,              // B+ voltage of 555
	DEFAULT_555_VALUES
};


static const discrete_dac_r1_ladder desc_crbaloon_music_dac =
{
	3,
	{0, RES_K(470), RES_K(120)},
	5, RES_K(470), 0, 0
};



static DISCRETE_SOUND_START(crbaloon)

	/************************************************
	* Input register mapping
	************************************************/
	DISCRETE_INPUT_LOGIC(CRBALOON_LAUGH_EN)
	DISCRETE_INPUT_LOGIC(CRBALOON_MUSIC_EN)
	DISCRETE_INPUT_DATA (CRBALOON_MUSIC_DATA)

	DISCRETE_ADJUSTMENT(CRBALOON_VR2, 0, 0.5, DISC_LINADJ, "VR2")
	DISCRETE_ADJUSTMENT(CRBALOON_VR3, 0, 1,   DISC_LINADJ, "VR3")

	/************************************************
	* Laugh is a VCO modulated by a constant
	* square wave.
	************************************************/
	DISCRETE_555_ASTABLE(NODE_10, 1, RES_K(10), RES_K(100), CAP_U(1), &desc_crbaloon_laugh_osc)
	DISCRETE_CRFILTER_VREF(NODE_11, NODE_10,
		1.0/(1.0/RES_K(5) + 1.0/RES_K(10) + 1.0/RES_K(100)), // 5k & 10k are 555 internal
		CAP_U(10),
		/* The following will calculate the reference control voltage with no signal applied to the cap. */
		5.0* (1.0/(1.0/RES_K(10) + 1.0/RES_K(100))) / (RES_K(5)+(1.0/(1.0/RES_K(10) + 1.0/RES_K(100)))) )
	DISCRETE_555_ASTABLE_CV(NODE_12, CRBALOON_LAUGH_EN, RES_K(1), RES_K(22), CAP_U(.1), NODE_11, &desc_crbaloon_laugh_osc)
	DISCRETE_MULTIPLY(NODE_13, NODE_12, CRBALOON_VR2)
	DISCRETE_CRFILTER(CRBALOON_LAUGH_SND, NODE_13, RES_K(20), CAP_U(1))

	/************************************************
	* Music Generator is amplitude modulated by a
	* linear ramp.
	************************************************/
	/* TO BE FIXED - needs proper modulation */
	DISCRETE_NOTE(NODE_20, 1, CRBALOON_16H / 2, CRBALOON_MUSIC_DATA, 255, 7, DISC_CLK_IS_FREQ)
	DISCRETE_DAC_R1(NODE_21, NODE_20, DEFAULT_TTL_V_LOGIC_1, &desc_crbaloon_music_dac)
	DISCRETE_ONOFF(NODE_22, CRBALOON_MUSIC_EN, NODE_21)
//  DISCRETE_RAMP(NODE_21, 1, RAMP, GRAD, MIN, MAX, CLAMP)
	DISCRETE_MULTIPLY(NODE_23, NODE_22, CRBALOON_VR3)
	DISCRETE_CRFILTER(CRBALOON_MUSIC_SND, NODE_23, RES_K(50), CAP_U(1))

	/************************************************
	* Final mix and output.
	************************************************/
	DISCRETE_ADDER2(NODE_90, 1, CRBALOON_LAUGH_SND, CRBALOON_MUSIC_SND)
	DISCRETE_CRFILTER(NODE_91, NODE_90, RES_K(100), CAP_U(1))

	DISCRETE_OUTPUT(NODE_91, 65000.0/12)

DISCRETE_SOUND_END


MACHINE_CONFIG_FRAGMENT( crbaloon_audio )

	MCFG_SPEAKER_STANDARD_MONO("mono")

	MCFG_SOUND_ADD("snsnd", SN76477, 0)
	MCFG_SN76477_NOISE_PARAMS(RES_K(47), RES_K(330), CAP_P(470)) // noise + filter
	MCFG_SN76477_DECAY_RES(RES_K(220))                   // decay_res
	MCFG_SN76477_ATTACK_PARAMS(CAP_U(1.0), RES_K(4.7))   // attack_decay_cap + attack_res
	MCFG_SN76477_AMP_RES(RES_M(1))                       // amplitude_res
	MCFG_SN76477_FEEDBACK_RES(RES_K(200))                // feedback_res
	MCFG_SN76477_VCO_PARAMS(5.0, CAP_P(470), RES_K(330)) // VCO volt + cap + res
	MCFG_SN76477_PITCH_VOLTAGE(5.0)                      // pitch_voltage
	MCFG_SN76477_SLF_PARAMS(CAP_P(420), RES_K(20))       // slf caps + res
	MCFG_SN76477_ONESHOT_PARAMS(CAP_U(1.0), RES_K(47))   // oneshot caps + res
	MCFG_SN76477_VCO_MODE(0)                             // VCO mode
	MCFG_SN76477_MIXER_PARAMS(0, 0, 1)                   // mixer A, B, C
	MCFG_SN76477_ENVELOPE_PARAMS(1, 0)                   // envelope 1, 2
	MCFG_SN76477_ENABLE(0)                               // enable
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 2.0)

	MCFG_SOUND_ADD("discrete", DISCRETE, 0)
	MCFG_DISCRETE_INTF(crbaloon)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 1.0)
MACHINE_CONFIG_END
