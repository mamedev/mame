// license:BSD-3-Clause
// copyright-holders:Derrick Renaud, Couriersud
/*************************************************************************

    VIC Dual Game board

*************************************************************************/

#include "emu.h"
#include "includes/vicdual.h"


/************************************************************************
 * frogs Sound System Analog emulation
 * Oct 2004, Derrick Renaud
 ************************************************************************/


/* Discrete Sound Input Nodes */
#define FROGS_FLY_EN        NODE_01
#define FROGS_JUMP_EN       NODE_03
#define FROGS_HOP_EN        NODE_04
#define FROGS_TONGUE_EN     NODE_05
#define FROGS_CAPTURE_EN    NODE_06
#define FROGS_SPLASH_EN     NODE_08

/* Nodes - Sounds */
#define FROGS_BUZZZ_SND     NODE_11
#define FROGS_BOING_SND     NODE_13
#define FROGS_HOP_SND       NODE_14
#define FROGS_ZIP_SND       NODE_15
#define FROGS_CROAK_SND     NODE_16
#define FROGS_SPLASH_SND    NODE_18
/* VRs */
#define FROGS_R93           NODE_25

static const discrete_555_desc frogsZip555m =
{
	DISC_555_OUT_CAP | DISC_555_OUT_DC | DISC_555_TRIGGER_IS_LOGIC,
	12,     // B+ voltage of 555
	DEFAULT_555_VALUES
};

static const discrete_555_cc_desc frogsZip555cc =
{
	DISC_555_OUT_CAP | DISC_555_OUT_DC,
	12,     // B+ voltage of 555
	DEFAULT_555_VALUES,
	0.6     // Q13 Vbe
};

static const discrete_mixer_desc frogsMixer =
{
	DISC_MIXER_IS_OP_AMP,
	{RES_K(1), RES_K(5)},
	{FROGS_R93, 0},
	{CAP_U(0.01), CAP_U(0.01)},
	0, RES_K(56), 0, CAP_U(0.1), 0, 10000
};

static DISCRETE_SOUND_START(frogs)
	/************************************************
	 * Input register mapping for frogs
	 *
	 * All inputs are inverted by initial transistor.
	 ************************************************/
	DISCRETE_INPUT_LOGIC(FROGS_FLY_EN)
	DISCRETE_INPUT_NOT(FROGS_JUMP_EN)
	DISCRETE_INPUT_NOT(FROGS_HOP_EN)
	DISCRETE_INPUT_NOT(FROGS_TONGUE_EN)
	DISCRETE_INPUT_NOT(FROGS_CAPTURE_EN)
	DISCRETE_INPUT_NOT(FROGS_SPLASH_EN)

	DISCRETE_ADJUSTMENT(FROGS_R93, RES_M(1), RES_K(10), DISC_LOGADJ, "R93")

	DISCRETE_555_MSTABLE(NODE_30, 1, FROGS_TONGUE_EN, RES_K(100), CAP_U(1), &frogsZip555m)

	/* Q11 & Q12 transform the voltage from the oneshot U4, to what is
	 * needed by the 555CC circuit.  Vin to R29 must be > 1V for things
	 * to change.  <=1 then The Vout of this circuit is 12V.
	 * The Current through R28 equals current through R51. iR28 = iR51
	 * So when Vin>.5, iR51 = (Vin-.5)/39k.  =0 when Vin<=.5
	 * So the voltage drop across R28 is vR28 = iR51 * 22k.
	 * Finally the Vout = 12 - vR28.
	 * Note this formula only works when Vin < 39/(22+39)*12V+1.
	 * Which it always is, due to the 555 clamping to 12V*2/3.
	 * The Zip effect is hard to emulate 100% due to loading effects
	 * of the output stage on the charge stage.  So I added some values
	 * to get a similar waveshape to the breadboarded circuit.
	 */
	DISCRETE_TRANSFORM5(NODE_31, 12, NODE_30, .5, RES_K(22)/RES_K(39), 0, "012-P4>*3*-")

	DISCRETE_555_CC(NODE_32, 1, NODE_31, RES_K(1.1), CAP_U(0.14), 0, RES_K(100), 500, &frogsZip555cc)

	DISCRETE_MIXER2(NODE_90, 1, NODE_32, 0, &frogsMixer)

	DISCRETE_OUTPUT(NODE_90, 1)

DISCRETE_SOUND_END

static const char *const frogs_sample_names[] =
{
	"*frogs",
	"boing",
	"buzzz",
	"croak",
	"hop",
	"splash",
	"zip",
	nullptr
};


MACHINE_CONFIG_FRAGMENT( frogs_audio )
	MCFG_SOUND_ADD("samples", SAMPLES, 0)
	MCFG_SAMPLES_CHANNELS(5)
	MCFG_SAMPLES_NAMES(frogs_sample_names)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.35)

	MCFG_SOUND_ADD("discrete", DISCRETE, 0)
	MCFG_DISCRETE_INTF(frogs)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 1.0)
MACHINE_CONFIG_END


TIMER_CALLBACK_MEMBER( vicdual_state::frogs_croak_callback )
{
	m_samples->stop(2);
}


MACHINE_START_MEMBER(vicdual_state,frogs_audio)
{
	m_frogs_croak_timer = machine().scheduler().timer_alloc(timer_expired_delegate(FUNC(vicdual_state::frogs_croak_callback), this));

	machine_start();
}


WRITE8_MEMBER( vicdual_state::frogs_audio_w )
{
	static int last_croak = 0;
	static int last_buzzz = 0;
	int new_croak = data & 0x08;
	int new_buzzz = data & 0x10;

//  m_discrete->write(space, FROGS_HOP_EN, data & 0x01);
//  m_discrete->write(space, FROGS_JUMP_EN, data & 0x02);
	m_discrete->write(space, FROGS_TONGUE_EN, data & 0x04);
//  m_discrete->write(space, FROGS_CAPTURE_EN, data & 0x08);
//  m_discrete->write(space, FROGS_FLY_EN, data & 0x10);
//  m_discrete->write(space, FROGS_SPLASH_EN, data & 0x80);

	if (data & 0x01)
		m_samples->start(3, 3);   // Hop
	if (data & 0x02)
		m_samples->start(0, 0);   // Boing
	if (new_croak)
		m_samples->start(2, 2);   // Croak
	else
	{
		if (last_croak)
		{
			/* The croak will keep playing until .429s after being disabled */
			m_frogs_croak_timer->adjust(attotime::from_double(1.1 * RES_K(390) * CAP_U(1)));
		}
	}
	if (new_buzzz)
	{
		/* The Buzzz sound starts off a little louder in volume then
		 * settles down to a steady buzzz.  Whenever the trigger goes
		 * low, the sound is disabled.  If it then goes high, the buzzz
		 * then starts off louder again.  The games does this every time
		 * the fly moves.
		 * So I made the sample start with the louder effect and then play
		 * for 12 seconds.  A fly should move before this.  If not the
		 * sample loops, adding the loud part as if the fly moved.
		 * This is obviously incorrect, but a fly never stands still for
		 * 12 seconds.
		 */
		if (!last_buzzz)
			m_samples->start(1, 1, true); // Buzzz
	}
	else
		m_samples->stop(1);
	if (data & 0x80)
		m_samples->start(4, 4);   // Splash

	last_croak = new_croak;
	last_buzzz = new_buzzz;
}



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

static DISCRETE_SOUND_START(headon)
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

MACHINE_CONFIG_FRAGMENT( headon_audio )

	MCFG_SOUND_ADD("discrete", DISCRETE, 0)
	MCFG_DISCRETE_INTF(headon)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 1.0)
MACHINE_CONFIG_END

WRITE8_MEMBER( vicdual_state::headon_audio_w )
{
	if (m_discrete == nullptr)
		return;
	m_discrete->write(space, HEADON_HISPEED_PC_EN, data & 0x01);
	m_discrete->write(space, HEADON_SCREECH1_EN, data & 0x02);
	m_discrete->write(space, HEADON_CRASH_EN, data & 0x04);
	m_discrete->write(space, HEADON_HISPEED_CC_EN, data & 0x08);
	m_discrete->write(space, HEADON_SCREECH2_EN, data & 0x10);
	m_discrete->write(space, HEADON_BONUS_EN, data & 0x20);
	m_discrete->write(space, HEADON_CAR_ON_EN, data & 0x40);

}

WRITE8_MEMBER( vicdual_state::invho2_audio_w )
{
	if (m_discrete == nullptr)
		return;
	m_discrete->write(space, HEADON_HISPEED_PC_EN, data & 0x10);
	m_discrete->write(space, HEADON_SCREECH1_EN, data & 0x08);
	m_discrete->write(space, HEADON_CRASH_EN, data & 0x80);
	m_discrete->write(space, HEADON_HISPEED_CC_EN, data & 0x40);
	m_discrete->write(space, HEADON_SCREECH2_EN, data & 0x04);
	m_discrete->write(space, HEADON_BONUS_EN, data & 0x02);
	m_discrete->write(space, HEADON_CAR_ON_EN, data & 0x20);

}

/************************************************************************
 * brdrline Sound System Analog emulation
 * May 2006, Derrick Renaud
 ************************************************************************/
#if 0


/* Discrete Sound Input Nodes */
#define BRDRLINE_GUN_TRG_EN         NODE_01
#define BRDRLINE_JEEP_ON_EN         NODE_02
#define BRDRLINE_POINT_TRG_EN       NODE_03
#define BRDRLINE_HIT_TRG_EN         NODE_04
#define BRDRLINE_ANIMAL_TRG_EN      NODE_05
#define BRDRLINE_EMAR_TRG_EN        NODE_06
#define BRDRLINE_WALK_TRG_EN        NODE_07
#define BRDRLINE_CRY_TRG_EN         NODE_08

/* Nodes - Sounds */
#define BRDRLINE_GUN_TRG_SND        NODE_91
#define BRDRLINE_JEEP_ON_SND        NODE_92
#define BRDRLINE_POINT_TRG_SND      NODE_93
#define BRDRLINE_HIT_TRG_SND        NODE_94
#define BRDRLINE_ANIMAL_TRG_SND     NODE_95
#define BRDRLINE_EMAR_TRG_SND       NODE_96
#define BRDRLINE_WALK_TRG_SND       NODE_97
#define BRDRLINE_CRY_TRG_SND        NODE_98

DISCRETE_SOUND_START(brdrline)
	/************************************************
	 * Input register mapping
	 ************************************************/
	DISCRETE_INPUT_LOGIC(BRDRLINE_GUN_TRG_EN)
	DISCRETE_INPUT_LOGIC(BRDRLINE_JEEP_ON_EN)
	DISCRETE_INPUT_LOGIC(BRDRLINE_POINT_TRG_EN)
	DISCRETE_INPUT_LOGIC(BRDRLINE_HIT_TRG_EN)
	DISCRETE_INPUT_LOGIC(BRDRLINE_ANIMAL_TRG_EN)
	DISCRETE_INPUT_LOGIC(BRDRLINE_EMAR_TRG_EN)
	DISCRETE_INPUT_LOGIC(BRDRLINE_WALK_TRG_EN)
	DISCRETE_INPUT_LOGIC(BRDRLINE_CRY_TRG_EN)

	/************************************************
	 * GUN TRG
	 ************************************************/
	DISCRETE_LFSR_NOISE(NODE_10, 1, 1,CLK,AMPL,FEED,BIAS,LFSRTB)
	DISCRETE_MIXER2(NODE_11, 1, NODE_10,IN1,INFO)
	DISCRETE_FILTER2(NODE_12, 1, NODE_11,FREQ,DAMP,TYPE)
	DISCRETE_ONESHOT(NODE_13, BRDRLINE_GUN_TRG_EN, DEFAULT_TTL_V_LOGIC_1,
		TIME_OF_74LS123(RES_K(47), CAP_U(1)),   // R155, C73
		DISC_ONESHOT_FEDGE | DISC_ONESHOT_RETRIG | DISC_OUT_ACTIVE_LOW)
	DISCRETE_RCDISC4(NODE_14, 1, NODE_13,RVAL0,RVAL1,RVAL2,CVAL,VP,TYPE)
	DISCRETE_VCA(BRDRLINE_GUN_TRG_SND, 1, NODE_12, NODE_14,TYPE)

	/************************************************
	 * JEEP ON
	 ************************************************/
	DISCRETE_555_ASTABLE(NODE_20, BRDRLINE_JEEP_ON_EN,
		RES_K(1),   // R150
		RES_K(33),  // R153
		CAP_U(.1),  // C72
		OPTIONS)
	DISCRETE_COUNTER(NODE_21, 1, 1, NODE_20,MIN,MAX,DIR,INIT0, DISC_CLK_BY_COUNT)
	DISCRETE_COUNTER(NODE_22, 1, 1, NODE_20,MIN,MAX,DIR,INIT0, DISC_CLK_BY_COUNT)
	DISCRETE_TRANSFORM3(NODE,INP0,INP1,INP2,FUNCT)
	DISCRETE_DAC_R1(NODE,DATA,VDATA,LADDER)

	/************************************************
	 * POINT TRG
	 ************************************************/

	/************************************************
	 * HIT TRG
	 ************************************************/

	/************************************************
	 * ANIMAL TRG
	 ************************************************/

	/************************************************
	 * EMAR TRG
	 ************************************************/

	/************************************************
	 * WALK TRG
	 ************************************************/

	/************************************************
	 * CRY TRG
	 ************************************************/

	/************************************************
	 * Mixer
	 ************************************************/

	DISCRETE_OUTPUT(NODE_90, 1)

DISCRETE_SOUND_END
#endif
