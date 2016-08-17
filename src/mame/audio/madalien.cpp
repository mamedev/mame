// license:BSD-3-Clause
// copyright-holders:Derrick Renaud
/************************************************************************
 * madalien Sound System Analog emulation
 * Aug 2008, Derrick Renaud
 ************************************************************************/

#include "emu.h"
#include "includes/madalien.h"
#include "sound/discrete.h"


/* Discrete Sound Input Nodes */
/* see also "madalien.h" */
#define MADALIEN_8910_PORTA_1       NODE_03
#define MADALIEN_8910_PORTA_2       NODE_04
#define MADALIEN_8910_PORTA_3       NODE_05
#define MADALIEN_8910_PORTA_4       NODE_06
#define MADALIEN_8910_PORTA_5       NODE_07
#define MADALIEN_8910_PORTA_6       NODE_08
#define MADALIEN_8910_PORTA_8       NODE_09

#define MADALIEN_8910_PORTB_1       NODE_10
#define MADALIEN_8910_PORTB_23      NODE_11
#define MADALIEN_8910_PORTB_45      NODE_12
#define MADALIEN_8910_PORTB_6       NODE_13
#define MADALIEN_8910_PORTB_7       NODE_14

#define MADALIEN_8910_PSG_A         NODE_15
#define MADALIEN_8910_PSG_B         NODE_16
#define MADALIEN_8910_PSG_C         NODE_17


static const discrete_op_amp_filt_info madalien_psg_a_filter =
{
	RES_K(1)+ 270, 0, RES_K(10), 0, RES_K(100), CAP_U(.1), CAP_U(.1), 0, 0, 5, -5
};

static const discrete_op_amp_filt_info madalien_psg_c_filter =
{
	RES_K(1)+ 270, 0, RES_K(10), 0, RES_K(100), CAP_U(.068), CAP_U(.068), 0, 0, 5, -5
};

static const discrete_mixer_desc madalien_psg_mix =
{
	DISC_MIXER_IS_RESISTOR,
	{RES_K(10), RES_K(10), RES_K(10)},
	{0}, {0}, 0, 0, 0, 0, 0, 1
};

static const discrete_555_desc madalien_555_1f =
{
	// the 555 will clock a 74161, which counts on rising edges
	DISC_555_TRIGGER_IS_LOGIC | DISC_555_OUT_DC | DISC_555_OUT_COUNT_R_X,
	5.0-.5,     // B+ voltage of 555 - diode drop
	DEFAULT_555_VALUES
};

static const discrete_555_desc madalien_555_1c =
{
	// the 555 will clock a 74161, which counts on rising edges
	DISC_555_TRIGGER_IS_LOGIC | DISC_555_OUT_DC | DISC_555_OUT_COUNT_R_X,
	5,      // B+ voltage of 555
	DEFAULT_555_VALUES
};

static const discrete_555_desc madalien_555_1l =
{
	DISC_555_TRIGGER_IS_LOGIC | DISC_555_OUT_DC | DISC_555_OUT_CAP,
	5,      // B+ voltage of 555
	DEFAULT_555_VALUES
};

static const discrete_comp_adder_table madalien_555_1f_r_select =
{
	DISC_COMP_P_RESISTOR, 0, 2,
	{RES_K(22+1), RES_K(15+1)}
};

static const discrete_comp_adder_table madalien_effect_1b_vol_r =
{
	DISC_COMP_P_RESISTOR, 0, 2,
	{270, RES_K(22) + 270}
};

static const discrete_dac_r1_ladder madalien_effect1a_dac =
{
	4,
	{RES_K(22), RES_K(22), 0, RES_K(22)},
	0, 0, 0, 0
};

static const discrete_dac_r1_ladder madalien_effect1b_dac =
{
	2,
	{RES_K(22), RES_K(22)},
	0, 0, 0, 0
};

static const discrete_dac_r1_ladder madalien_effect2_dac =
{
	4,
	{0, RES_K(33), RES_K(22), RES_K(33)},
	0, 0, 0, 0
};

// resistor effect of 555 internal resistors at pin 5
static const discrete_mixer_desc madalien_555_1c_cv =
{
	DISC_MIXER_IS_RESISTOR,
	{RES_K(1.5), RES_K(5)},
	{0}, {0}, 0, RES_K(10), 0, 0, 0, 1
};

static const discrete_mixer_desc madalien_final_mix =
{
	DISC_MIXER_IS_RESISTOR,
	{RES_K(10), RES_K(10)/3, RES_K(10), RES_K(10),
		RES_K(22) + RES_K(22)/3,
		RES_K(22) + RES_K(22) + RES_K(22)/2,
		RES_K(33) + 1.0 / (1.0/RES_K(33) + 1.0/RES_K(22) + 1.0/RES_K(33))},
	{MADALIEN_8910_PORTA_1, 0, MADALIEN_8910_PORTA_5, MADALIEN_8910_PORTA_8, MADALIEN_8910_PORTB_1, NODE_59, 0},
	{0}, 0,
	RES_K(100),
	0, CAP_U(1), 0,
	32768.0/DEFAULT_TTL_V_LOGIC_1   // final gain
};

DISCRETE_SOUND_START( madalien )
	/************************************************
	 * Input register mapping
	 ************************************************/
	DISCRETE_INPUT_DATA(MADALIEN_8910_PORTA)
	DISCRETE_INPUT_DATA(MADALIEN_8910_PORTB)

	/************************************************
	 * Convert to individual bits
	 ************************************************/
	// Port A is used to turn filtering on/off.
	// 1 - player shot, siren, music
	// turn bit into 4066 Ron value of 270 ohms @ 5V
	DISCRETE_TRANSFORM3(MADALIEN_8910_PORTA_1, MADALIEN_8910_PORTA, 0x01, 270, "01&2*")
	// 2 - player explosion
	DISCRETE_TRANSFORM2(MADALIEN_8910_PORTA_2, MADALIEN_8910_PORTA, 0x02, "01&")
	// 3 - rub wall
	DISCRETE_TRANSFORM2(MADALIEN_8910_PORTA_3, MADALIEN_8910_PORTA, 0x04, "01&")
	// 4 - is it even triggered?
	DISCRETE_TRANSFORM2(MADALIEN_8910_PORTA_4, MADALIEN_8910_PORTA, 0x08, "01&")
	// 5 - siren, music (out of phase copy of 1 for echo effect)
	DISCRETE_TRANSFORM3(MADALIEN_8910_PORTA_5, MADALIEN_8910_PORTA, 0x10, 270, "01&2*")
	// 6 - enemy explosions
	DISCRETE_TRANSFORM2(MADALIEN_8910_PORTA_6, MADALIEN_8910_PORTA, 0x20, "01&")
	// 8 - enemy spin, beep
	DISCRETE_TRANSFORM3(MADALIEN_8910_PORTA_8, MADALIEN_8910_PORTA, 0x80, 270, "01&2*")

	//Port B controls motor sounds
	// 1,2,3 - Player motor volume
	DISCRETE_TRANSFORM3(MADALIEN_8910_PORTB_1,  MADALIEN_8910_PORTB, 0x01, 270, "01&2*")
	DISCRETE_TRANSFORM3(MADALIEN_8910_PORTB_23, MADALIEN_8910_PORTB, 0x06, 2, "01&2/")
	// 4,5 - Player motor speed
	DISCRETE_TRANSFORM3(MADALIEN_8910_PORTB_45, MADALIEN_8910_PORTB, 0x18, 8, "01&2/")
	// 6 - Enemy motor enable
	DISCRETE_TRANSFORM2(MADALIEN_8910_PORTB_6,  MADALIEN_8910_PORTB, 0x20, "01&")
	// 7 - Enemy motor speed
	// convert bit 7 to voltage level
	// 2 diodes reduce voltage by 1V.
	DISCRETE_TRANSFORM3(MADALIEN_8910_PORTB_7, MADALIEN_8910_PORTB, 0x40, (DEFAULT_TTL_V_LOGIC_1 - 1) / 0x40, "01&2*")

	/************************************************
	 * PSG input streams
	 ************************************************/
	// AY-3-8910 PSG have a 1Vpp level
	DISCRETE_INPUTX_STREAM(MADALIEN_8910_PSG_A, 0, 2.0/32768, .250)
	DISCRETE_INPUTX_STREAM(MADALIEN_8910_PSG_B, 1, 2.0/32768, .250)
	DISCRETE_INPUTX_STREAM(MADALIEN_8910_PSG_C, 2, 2.0/32768, .250)

	/************************************************
	 * AY-3-8910 filtering
	 ************************************************/
	// top op-amp
	DISCRETE_ONOFF(NODE_20, MADALIEN_8910_PORTA_2, MADALIEN_8910_PSG_A)
	DISCRETE_OP_AMP_FILTER(NODE_21, 1, NODE_20, 0, DISC_OP_AMP_FILTER_IS_BAND_PASS_1M, &madalien_psg_a_filter)

	// middle op-amp
	// pin 3
	DISCRETE_MULTIPLY(NODE_30, MADALIEN_8910_PSG_B, RES_K(3.3)/(RES_K(3.3)+RES_K(3.3)))
	DISCRETE_ONOFF(NODE_31, MADALIEN_8910_PORTA_4, NODE_30)
	// pin 2
	DISCRETE_TRANSFORM3(NODE_32, MADALIEN_8910_PSG_A, NODE_30, -RES_K(10)/RES_K(3.3), "01-2*1+")
	// pin 1
	DISCRETE_SWITCH(NODE_33, 1, MADALIEN_8910_PORTA_3, NODE_31, NODE_32)
	DISCRETE_CLAMP(NODE_34, NODE_33, -5, 5.0-1.5)

	// bottom op-amp
	DISCRETE_ONOFF(NODE_40, MADALIEN_8910_PORTA_6, MADALIEN_8910_PSG_B)
	DISCRETE_OP_AMP_FILTER(NODE_41, 1, NODE_40, 0, DISC_OP_AMP_FILTER_IS_BAND_PASS_1M, &madalien_psg_c_filter)

	DISCRETE_MIXER3(NODE_48, 1, NODE_21, NODE_33, NODE_41, &madalien_psg_mix)

	/************************************************
	 * Player motor
	 ************************************************/
	DISCRETE_555_ASTABLE(NODE_50,   // cap is buffered by op-amp 2D.
		1,                          // always enabled
		RES_K(4.7),
		RES_K(22),
		CAP_U(2.2),
		&madalien_555_1l)
	// The speed frequencies seem strange but the components have been GuruVerified
	// There is not much change in selected frequency.  99Hz, 110.6Hz, 124Hz
	DISCRETE_COMP_ADDER(NODE_51, MADALIEN_8910_PORTB_45, &madalien_555_1f_r_select)
	DISCRETE_555_ASTABLE_CV(NODE_52,    // IC 1F pin 3 out
		MADALIEN_8910_PORTB_45,         // enabled by gate O2 pin 13
		NODE_51,
		RES_K(10),                      // per actual board
		CAP_U(.22),
		NODE_50,                        // IC 1F pin 5 in
		&madalien_555_1f)
	// convert reset to active high for module use
	DISCRETE_LOGIC_INVERT(NODE_53, MADALIEN_8910_PORTB_45)
	DISCRETE_COUNTER(NODE_54, 1,
		NODE_53,                        // pin 7 in
		NODE_52,                        // pin 1 in
		0, 15, 1, 0, DISC_CLK_BY_COUNT) // 4-bit binary up counter
	DISCRETE_DAC_R1(NODE_55, NODE_54, DEFAULT_TTL_V_LOGIC_1, &madalien_effect1a_dac)
	DISCRETE_DAC_R1(NODE_56, NODE_54, DEFAULT_TTL_V_LOGIC_1, &madalien_effect1b_dac)
	DISCRETE_RCFILTER(NODE_57, NODE_56, RES_K(22)/2 + RES_K(22), CAP_U(.033))
	DISCRETE_COMP_ADDER(NODE_59, MADALIEN_8910_PORTB_23, &madalien_effect_1b_vol_r)

	/************************************************
	 * Enemy motor
	 ************************************************/
	DISCRETE_CRFILTER(NODE_60, MADALIEN_8910_PORTB_7, RES_K(100), CAP_U(4.7))
	// 2 diodes clamp it positive.
	DISCRETE_CLAMP(NODE_62, NODE_60, 0, 12)
	// the 0.047uF cap to ground just removes real world spikes.
	// it does not have to be simulated.
	DISCRETE_MIXER2(NODE_64, 1, NODE_62, 5, &madalien_555_1c_cv)
	DISCRETE_555_ASTABLE_CV(NODE_65,    // IC 1C pin 3 out
		MADALIEN_8910_PORTB_6,
		RES_K(47),
		RES_K(22),
		CAP_U(.033),
		NODE_64,                        // IC 1C pin 5 in
		&madalien_555_1c)
	// convert reset to active high for module use
	DISCRETE_LOGIC_INVERT(NODE_66, MADALIEN_8910_PORTB_6)
	DISCRETE_COUNTER(NODE_67, 1,
		NODE_66,                        // pin 7 in
		NODE_65,                        // pin 1 in
		0, 15, 1, 0, DISC_CLK_BY_COUNT) // 4-bit binary up counter
	DISCRETE_DAC_R1(NODE_68, NODE_67, DEFAULT_TTL_V_LOGIC_1, &madalien_effect2_dac)

	/************************************************
	 * Mixer
	 ************************************************/
	DISCRETE_MIXER7(NODE_90, 1, MADALIEN_8910_PSG_A, NODE_48, MADALIEN_8910_PSG_B, MADALIEN_8910_PSG_C, NODE_56, NODE_57, NODE_68, &madalien_final_mix)

	DISCRETE_OUTPUT(NODE_90, 1.1)

DISCRETE_SOUND_END
