// license:BSD-3-Clause
// copyright-holders:Derrick Renaud, Couriersud, Aaron Giles
/*************************************************************************

    VIC Dual Game board

*************************************************************************/

#include "emu.h"
#include "vicdual.h"

#include "nl_brdrline.h"
#include "nl_frogs.h"


/************************************************************************
 * headon Sound System Analog emulation
 * July 2007, couriersud
 ************************************************************************/

#define HEADON_HISPEED_CC_EN    NODE_01
#define HEADON_HISPEED_PC_EN    NODE_02
#define HEADON_CAR_ON_EN        NODE_03
#define HEADON_CRASH_EN         NODE_04
#define HEADON_SCREECH1_EN      NODE_05
#define HEADON_SCREECH2_EN      NODE_06
#define HEADON_BONUS_EN         NODE_07

#define HEADON_COMP_CAR_OUT     NODE_200
#define HEADON_PLAYER_CAR_OUT   NODE_201
#define HEADON_CRASH_OUT        NODE_202
#define HEADON_SCREECH1_OUT     NODE_203
#define HEADON_SCREECH2_OUT     NODE_204
#define HEADON_BONUS_OUT        NODE_205


static const discrete_mixer_desc headon_mixer =
{
	DISC_MIXER_IS_RESISTOR,
	{RES_K(130), RES_K(130), RES_K(100), RES_K(100), RES_K(100), RES_K(10)},   // 130 = 390/3, Bonus Res is dummy
	{0,0,0,0,0},    // no variable resistors
	{0,0,0,0,CAP_N(470),0},
	0, RES_K(100),
	0,
	CAP_U(1),       // not in schematics, used to suppress DC
	0, 1
};

static const discrete_mixer_desc headon_crash_mixer =
{
	DISC_MIXER_IS_OP_AMP,
	{RES_K(50), RES_K(10)},   // Resistors, in fact variable resistors (100k)
	{0,0,0,0,0},    // no variable resistors
	{CAP_N(100),CAP_U(1)},
	0, RES_K(100),
	0,
	CAP_U(1)*0,     // not in schematics, used to suppress DC
	0, 1
};

static const discrete_dss_inverter_osc_node::description headon_inverter_osc_1 =
{
	DEFAULT_CD40XX_VALUES(12),
	discrete_dss_inverter_osc_node::IS_TYPE4
};

static const discrete_dss_inverter_osc_node::description headon_inverter_osc_2 =
{
	DEFAULT_CD40XX_VALUES(12),
	discrete_dss_inverter_osc_node::IS_TYPE5 | discrete_dss_inverter_osc_node::OUT_IS_LOGIC
};

static const discrete_555_desc headon_555_bonus =
{
	DISC_555_OUT_ENERGY | DISC_555_OUT_DC,
	12,
	DEFAULT_555_CHARGE,
	12.0-0.5
};

static const discrete_555_desc headon_555_crash =
{
	DISC_555_OUT_SQW | DISC_555_OUT_DC | DISC_555_TRIGGER_IS_LOGIC,
	12,
	DEFAULT_555_CHARGE,
	12.0-0.5
};

static const discrete_555_cc_desc headon_555cc =
{
	DISC_555_OUT_SQW | DISC_555_OUT_DC,
	12,     // B+ voltage of 555
	DEFAULT_555_VALUES,
	0.6     // Q16, Q10 Vbe
};


/*
 * From : http://www.vego.nl/8/08/03/08_08_03.htm
 *
 *- voeding:  -7 V, clock-frequency:  2.267 Hz
 *- voeding:  -8 V, clock-frequency:  8.731 Hz
 *- voeding:  -9 V, clock-frequency: 16,38 kHz
 *- voeding: -10 V, clock-frequency: 23,53 kHz
 *- voeding: -11 V, clock-frequency: 32,56 kHz
 *- voeding: -12 V, clock-frequency: 38,34 kHz
 *- voeding: -13 V, clock-frequency: 40,00 kHz
 *- voeding: -14 V, clock-frequency: 37,80 kHz
 *- voeding: -15 V, clock-frequency: 33,17 kHz
 *
 *  However all other mame sources say 100kHz.
 */

#define MM5837_CLOCK_12V 100000

static const discrete_lfsr_desc mm5837_lfsr =
{
	DISC_CLK_IS_FREQ,
	17,                   /* Bit Length */
	0,                    /* Reset Value */
	13,                   /* Use Bit 14 as F0 input 0 */
	16,                   /* Use Bit 17 as F0 input 1 */
	DISC_LFSR_XOR,        /* F0 is XOR */
	DISC_LFSR_NOT_IN0,    /* F1 is inverted F0*/
	DISC_LFSR_REPLACE,    /* F2 replaces the shifted register contents */
	0x000001,             /* Everything is shifted into the first bit only */
	0,                    /* Flags */
	16                    /* Output bit */
};

static const discrete_op_amp_filt_info headon_sallen_key_info =
{
	RES_K(15), RES_K(15), 0, 0, 0,
	CAP_N(470), CAP_N(47), 0
};

static DISCRETE_SOUND_START(headon_discrete)
	/************************************************
	 * Input register mapping for headon
	 *
	 ************************************************/
	DISCRETE_INPUT_LOGIC(HEADON_HISPEED_CC_EN)
	DISCRETE_INPUT_LOGIC(HEADON_HISPEED_PC_EN)
	DISCRETE_INPUT_LOGIC(HEADON_CAR_ON_EN)
	DISCRETE_INPUT_LOGIC(HEADON_CRASH_EN)
	DISCRETE_INPUT_LOGIC(HEADON_SCREECH1_EN)
	DISCRETE_INPUT_LOGIC(HEADON_SCREECH2_EN)
	DISCRETE_INPUT_LOGIC(HEADON_BONUS_EN)

	/************************************************
	 * CAR Sound generation Player Car
	 * The ramp values are taken from a
	 * SWITCHER CAD III simulation of the
	 * respective circuit. Using ramps may not be
	 * 100% accurate but comes very close.
	 ************************************************/

	DISCRETE_RAMP(NODE_20, 1, HEADON_CAR_ON_EN, (12-10.8)/7, 12, 10.8, 12)
	DISCRETE_RAMP(NODE_21, 1, HEADON_HISPEED_PC_EN, 2.0 / 0.8, 0, -2, 0)
	DISCRETE_ADDER2(NODE_22, 1, NODE_20, NODE_21)

#define HO_R56      RES_K(10)
#define HO_R72      RES_K(1)
#define HO_C31      CAP_N(100)

	DISCRETE_555_CC(NODE_25, HEADON_CAR_ON_EN, NODE_22, HO_R56, HO_C31, 0, 0, HO_R72, &headon_555cc)
	DISCRETE_COUNTER(NODE_26, 1, 0, NODE_25, 0, 1, DISC_COUNT_UP, 0, DISC_CLK_ON_R_EDGE) //divide by 2
	DISCRETE_COUNTER(NODE_27, 1, 0, NODE_25, 0, 3, DISC_COUNT_UP, 0, DISC_CLK_ON_R_EDGE) //divide by 4
	DISCRETE_COUNTER(NODE_28, 1, 0, NODE_25, 0, 2, DISC_COUNT_UP, 0, DISC_CLK_ON_R_EDGE) //divide by 3
	DISCRETE_TRANSFORM5(NODE_29,NODE_26,NODE_27,NODE_28,1,2,"13>24=+0+")
	DISCRETE_MULTIPLY(HEADON_PLAYER_CAR_OUT, NODE_29, 12 / 3)

	/************************************************
	 * CAR Sound generation Computer Car
	 ************************************************/

	DISCRETE_RAMP(NODE_30, 1, HEADON_CAR_ON_EN, (12-10.8)/7, 12, 10.8, 12)
	DISCRETE_RAMP(NODE_31, 1, HEADON_HISPEED_CC_EN, 2.0 / 0.8, 0, -2, 0)
	DISCRETE_ADDER2(NODE_32, 1, NODE_30, NODE_31)

#define HO_R43      RES_K(10)
#define HO_R35      RES_K(1)
#define HO_C20      CAP_N(100)

	DISCRETE_555_CC(NODE_35, HEADON_CAR_ON_EN, NODE_32, HO_R43, HO_C20, 0, 0, HO_R35, &headon_555cc)
	DISCRETE_COUNTER(NODE_36, 1, 0, NODE_35, 0, 1, DISC_COUNT_UP, 0, DISC_CLK_ON_R_EDGE) //divide by 2
	DISCRETE_COUNTER(NODE_37, 1, 0, NODE_35, 0, 3, DISC_COUNT_UP, 0, DISC_CLK_ON_R_EDGE) //divide by 4
	DISCRETE_COUNTER(NODE_38, 1, 0, NODE_35, 0, 2, DISC_COUNT_UP, 0, DISC_CLK_ON_R_EDGE) //divide by 3
	DISCRETE_TRANSFORM5(NODE_39,NODE_36,NODE_37,NODE_38,1,2,"13>24=+0+")
	DISCRETE_MULTIPLY(HEADON_COMP_CAR_OUT, NODE_39, 12 / 3)

	/************************************************
	 * Screech #1
	 ************************************************/

	DISCRETE_MULTIPLY(NODE_50,HEADON_SCREECH1_EN,12)
	DISCRETE_LFSR_NOISE(NODE_51, 1, 1, MM5837_CLOCK_12V, 12.0, 0, 6.0, &mm5837_lfsr)
	DISCRETE_INVERTER_OSC(HEADON_SCREECH1_OUT,NODE_50,NODE_51,RES_K(10),RES_K(100),CAP_N(47),RES_K(10),&headon_inverter_osc_1)

	/************************************************
	 * Screech #2
	 ************************************************/

	DISCRETE_MULTIPLY(NODE_60,HEADON_SCREECH2_EN,12)
	DISCRETE_INVERTER_OSC(HEADON_SCREECH2_OUT,NODE_60,NODE_51,RES_K(10),RES_K(100),CAP_N(57),RES_K(10),&headon_inverter_osc_1)

	/************************************************
	 * Bonus
	 ************************************************/

	DISCRETE_LOGIC_INVERT(NODE_70, HEADON_BONUS_EN)
	DISCRETE_MULTIPLY(NODE_71,NODE_70,12)
	DISCRETE_INVERTER_OSC(NODE_73,NODE_71,0,RES_K(22),RES_M(1),CAP_N(470),RES_M(10),&headon_inverter_osc_2)

	/* FIXME: the following is a bit of a hack
	 * The NE555 is operating at a frequency of 400Hz
	 * The output of the oscillator is connectred through a 150K resistor to
	 * the discharge pin.
	 * The simulation gives a frequency of roughly 600Hz if the osc output is high.
	 * This is equivalent to R1 being 47k || 150k = 35K
	 * The simulation gives a frequency of roughly 375Hz if the osc output is low.
	 * This is not emulated exactly. We will just use 200k for R1.
	 *
	 */
	DISCRETE_TRANSFORM3(NODE_74,NODE_73,200000,165000,"102*-")
	DISCRETE_555_ASTABLE(NODE_75, 1, NODE_74, RES_K(100), CAP_N(10), &headon_555_bonus)
	DISCRETE_MULTIPLY(HEADON_BONUS_OUT,NODE_75,HEADON_BONUS_EN)

	/************************************************
	 * Crash
	 * FIXME: Just a prototype several filter missing
	 ************************************************/

	DISCRETE_LOGIC_INVERT(NODE_80, HEADON_CRASH_EN)
	DISCRETE_555_MSTABLE(NODE_81, 1, NODE_80, RES_K(470), CAP_U(1), &headon_555_crash)
	// Mix with noise
	DISCRETE_MULTIPLY(NODE_84, NODE_81, NODE_51)
	// Taken from simulation
	// Center frequency is 500 Hz
	// roughly 6db per octave
	DISCRETE_FILTER1(NODE_85, 1, NODE_84, 500, DISC_FILTER_BANDPASS)


	DISCRETE_555_MSTABLE(NODE_86, 1, NODE_80, RES_K(470), CAP_U(2.2), &headon_555_crash)
	// Mix with noise
	DISCRETE_MULTIPLY(NODE_87, NODE_86, NODE_51)
	// Sallen Key filter ...
	// http://www.t-linespeakers.org/tech/filters/Sallen-Key.html
	// f = w / 2 / pi  = 1 / ( 2 * pi * 15k*sqrt(470n*47n)) = 71 Hz
	// Q = 1/2 * sqrt(470n/47n)= 1.58
	DISCRETE_SALLEN_KEY_FILTER(NODE_88, 1, NODE_87, DISC_SALLEN_KEY_LOW_PASS, &headon_sallen_key_info)

	DISCRETE_MIXER2(NODE_95, 1, NODE_85, NODE_88, &headon_crash_mixer)
	DISCRETE_TRANSFORM2(HEADON_CRASH_OUT, NODE_95, 12, "01/")

	/************************************************
	 * Mixer Stage
	 ************************************************/

	DISCRETE_MIXER6(NODE_210, 1, HEADON_PLAYER_CAR_OUT, HEADON_COMP_CAR_OUT,
					HEADON_SCREECH1_OUT, HEADON_SCREECH2_OUT,
					HEADON_BONUS_OUT, HEADON_CRASH_OUT, &headon_mixer)

	DISCRETE_OUTPUT(NODE_210, 37000.0 / 12.0)
	//DISCRETE_CSVLOG3(HEADON_CRASH_EN,NODE_81,NODE_80)

DISCRETE_SOUND_END

void vicdual_state::headon_audio(machine_config &config)
{
	DISCRETE(config, m_discrete, headon_discrete);
	m_discrete->add_route(ALL_OUTPUTS, "mono", 1.0);
}

void vicdual_state::headon_audio_w(uint8_t data)
{
	if (m_discrete == nullptr)
		return;
	m_discrete->write(HEADON_HISPEED_PC_EN, data & 0x01);
	m_discrete->write(HEADON_SCREECH1_EN, data & 0x02);
	m_discrete->write(HEADON_CRASH_EN, data & 0x04);
	m_discrete->write(HEADON_HISPEED_CC_EN, data & 0x08);
	m_discrete->write(HEADON_SCREECH2_EN, data & 0x10);
	m_discrete->write(HEADON_BONUS_EN, data & 0x20);
	m_discrete->write(HEADON_CAR_ON_EN, data & 0x40);

}

void vicdual_state::invho2_audio_w(uint8_t data)
{
	if (m_discrete == nullptr)
		return;
	m_discrete->write(HEADON_HISPEED_PC_EN, data & 0x10);
	m_discrete->write(HEADON_SCREECH1_EN, data & 0x08);
	m_discrete->write(HEADON_CRASH_EN, data & 0x80);
	m_discrete->write(HEADON_HISPEED_CC_EN, data & 0x40);
	m_discrete->write(HEADON_SCREECH2_EN, data & 0x04);
	m_discrete->write(HEADON_BONUS_EN, data & 0x02);
	m_discrete->write(HEADON_CAR_ON_EN, data & 0x20);

}


/*************************************
 *
 *  Netlist-based Vic Dual Audio
 *
 *************************************/

vicdual_audio_device_base::vicdual_audio_device_base(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, u32 clock, u8 inputs_mask, void (*netlist)(netlist::nlparse_t &), double output_scale) :
	device_t(mconfig, type, tag, owner, clock),
	device_mixer_interface(mconfig, *this),
	m_input_line(*this, "sound_nl:in_%u", 0),
	m_inputs_mask(inputs_mask),
	m_netlist(netlist),
	m_output_scale(output_scale)
{
}

void vicdual_audio_device_base::device_add_mconfig(machine_config &config)
{
	NETLIST_SOUND(config, "sound_nl", 48000)
		.set_source(m_netlist)
		.add_route(ALL_OUTPUTS, *this, 1.0);

	if (BIT(m_inputs_mask, 0))
		NETLIST_LOGIC_INPUT(config, m_input_line[0], "I_SOUND_0.IN", 0);
	if (BIT(m_inputs_mask, 1))
		NETLIST_LOGIC_INPUT(config, m_input_line[1], "I_SOUND_1.IN", 0);
	if (BIT(m_inputs_mask, 2))
		NETLIST_LOGIC_INPUT(config, m_input_line[2], "I_SOUND_2.IN", 0);
	if (BIT(m_inputs_mask, 3))
		NETLIST_LOGIC_INPUT(config, m_input_line[3], "I_SOUND_3.IN", 0);
	if (BIT(m_inputs_mask, 4))
		NETLIST_LOGIC_INPUT(config, m_input_line[4], "I_SOUND_4.IN", 0);
	if (BIT(m_inputs_mask, 5))
		NETLIST_LOGIC_INPUT(config, m_input_line[5], "I_SOUND_5.IN", 0);
	if (BIT(m_inputs_mask, 6))
		NETLIST_LOGIC_INPUT(config, m_input_line[6], "I_SOUND_6.IN", 0);
	if (BIT(m_inputs_mask, 7))
		NETLIST_LOGIC_INPUT(config, m_input_line[7], "I_SOUND_7.IN", 0);

	NETLIST_STREAM_OUTPUT(config, "sound_nl:cout0", 0, "OUTPUT").set_mult_offset(m_output_scale, 0.0);
}

void vicdual_audio_device_base::device_start()
{
	save_item(NAME(m_input_state));
}

void vicdual_audio_device_base::write(u8 value)
{
	if (value != m_input_state)
	{
		m_input_state = value;
		for (int index = 0; index < 8; index++)
			if (m_input_line[index] != nullptr)
				m_input_line[index]->write_line(BIT(m_input_state, index));
	}
}


/*************************************
 *
 *  Borderline/Tranquilizer Gun
 *
 *************************************/

DEFINE_DEVICE_TYPE(BORDERLINE_AUDIO, borderline_audio_device, "borderline_audio", "Borderline Sound Board")

borderline_audio_device::borderline_audio_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock) :
	vicdual_audio_device_base(mconfig, BORDERLINE_AUDIO, tag, owner, clock, 0xff, NETLIST_NAME(brdrline), 1.0)
{
}



/*************************************
 *
 *  Frogs
 *
 *************************************/

DEFINE_DEVICE_TYPE(FROGS_AUDIO, frogs_audio_device, "frogs_audio", "Frogs Sound Board")

frogs_audio_device::frogs_audio_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock) :
	vicdual_audio_device_base(mconfig, FROGS_AUDIO, tag, owner, clock, 0xff, NETLIST_NAME(frogs), 1.0)
{
}
