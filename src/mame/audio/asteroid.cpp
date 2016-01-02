// license:BSD-3-Clause
// copyright-holders:K.Wilkins
/*****************************************************************************
 *
 * Asteroids Analog Sound system interface
 *
 *****************************************************************************/

#include "emu.h"
#include "sound/discrete.h"
#include "includes/asteroid.h"

/************************************************************************/
/* Asteroids Sound System Analog emulation by K.Wilkins Nov 2000        */
/* Questions/Suggestions to mame@esplexo.co.uk                          */
/************************************************************************/

static const discrete_lfsr_desc asteroid_lfsr =
{
	DISC_CLK_IS_FREQ,
	16,         /* Bit Length */
	0,          /* Reset Value */
	6,          /* Use Bit 6 as XOR input 0 */
	14,         /* Use Bit 14 as XOR input 1 */
	DISC_LFSR_XNOR,     /* Feedback stage1 is XNOR */
	DISC_LFSR_OR,       /* Feedback stage2 is just stage 1 output OR with external feed */
	DISC_LFSR_REPLACE,  /* Feedback stage3 replaces the shifted register contents */
	0x000001,       /* Everything is shifted into the first bit only */
	0,          /* Output is already inverted by XNOR */
	16          /* Output bit is feedback bit */
};

static const discrete_dac_r1_ladder asteroid_thump_dac1 =
{
	4,          // size of ladder
	{RES_K(220), RES_K(100), RES_K(47), RES_K(22)}, //R44-R47
	4.3,        // 5v - diode junction
	RES_K(6.8), // R49
	RES_K(47),  // R44
	CAP_U(0.01) // C27
};

static const discrete_555_cc_desc asteroid_thump_555cc =
{
	DISC_555_OUT_SQW | DISC_555_OUT_AC | DISCRETE_555_CC_TO_CAP,
	5,      // B+ voltage of 555
	DEFAULT_555_VALUES,
	0.8     // VBE 2N3906 (Si)
};

#define ASTEROID_SAUCER_SND_EN      NODE_01
#define ASTEROID_SAUCER_FIRE_EN     NODE_02
#define ASTEROID_SAUCER_SEL         NODE_03
#define ASTEROID_THRUST_EN          NODE_04
#define ASTEROID_SHIP_FIRE_EN       NODE_05
#define ASTEROID_LIFE_EN            NODE_06
#define ASTEROID_NOISE_RESET        NODE_07

#define ASTEROID_THUMP_EN           NODE_08
#define ASTEROID_THUMP_DATA         NODE_09
#define ASTEROID_EXPLODE_DATA       NODE_10
#define ASTEROID_EXPLODE_PITCH      NODE_11

#define ASTEROID_NOISE              NODE_20
#define ASTEROID_THUMP_SND          NODE_21
#define ASTEROID_SAUCER_SND         NODE_22
#define ASTEROID_LIFE_SND           NODE_23
#define ASTEROID_SAUCER_FIRE_SND    NODE_24
#define ASTEROID_SHIP_FIRE_SND      NODE_25
#define ASTEROID_EXPLODE_SND        NODE_26
#define ASTEROID_THRUST_SND         NODE_27

DISCRETE_SOUND_START(asteroid)
	/************************************************/
	/* Asteroid Effects Relataive Gain Table        */
	/*                                              */
	/* Effect       V-ampIn   Gain ratio  Relative  */
	/* Thump         5        1/47          131.6   */
	/* Saucer        2.4      1/39           76.1   */
	/* Life          3.8      1/47          100.0   */
	/* Saucer Fire   5-0.6    1/(100+10)     49.5   */
	/* Ship Fire     5-0.6    1/(100+2.7)    53.0   */
	/* Explode       3.8      1/4.7        1000.0   */
	/* Thrust        3.8      1/4.7         600.0   */
	/*  NOTE: Thrust gain has to be tweaked, due to */
	/*        the filter stage.                     */
	/************************************************/

	/*                        NODE                 GAIN        OFFSET  INIT */
	DISCRETE_INPUT_LOGIC (ASTEROID_SAUCER_SND_EN)
	DISCRETE_INPUT_LOGIC (ASTEROID_SAUCER_FIRE_EN)
	DISCRETE_INPUT_LOGIC (ASTEROID_SAUCER_SEL)
	DISCRETE_INPUT_LOGIC (ASTEROID_THRUST_EN)
	DISCRETE_INPUT_LOGIC (ASTEROID_SHIP_FIRE_EN)
	DISCRETE_INPUT_LOGIC (ASTEROID_LIFE_EN)
	DISCRETE_INPUT_PULSE (ASTEROID_NOISE_RESET, 1)

	DISCRETE_INPUT_LOGIC (ASTEROID_THUMP_EN)
	DISCRETE_INPUT_DATA  (ASTEROID_THUMP_DATA)

	DISCRETE_INPUTX_DATA (ASTEROID_EXPLODE_DATA, 1000.0/15.0,   0.0,   0)
	DISCRETE_INPUTX_DATA (ASTEROID_EXPLODE_PITCH, 1,            0,     12)

	/************************************************/
	/* Thump circuit is based on a VCO with the     */
	/* VCO control fed from the 4 low order bits    */
	/* from /THUMP bit 4 controls the osc enable.   */
	/* A resistor ladder network is used to convert */
	/* the 4 bit value to an analog value.          */
	/*                                              */
	/* The VCO is implemented with a 555 timer and  */
	/* an RC filter to perform smoothing on the     */
	/* output                                       */
	/************************************************/
	DISCRETE_DAC_R1(NODE_30, ASTEROID_THUMP_DATA, 3.5, &asteroid_thump_dac1) // CMOS
	DISCRETE_555_CC(NODE_31, ASTEROID_THUMP_EN, NODE_30, RES_K(22), CAP_U(0.22), 0, 0, RES_K(18), &asteroid_thump_555cc)
	DISCRETE_RCFILTER(NODE_32, NODE_31, RES_K(3.3), CAP_U(0.1))
	DISCRETE_GAIN(ASTEROID_THUMP_SND, NODE_32, 30)

	/************************************************/
	/* The SAUCER sound is based on two VCOs, a     */
	/* slow VCO feed the input to a higher freq VCO */
	/* with the SAUCERSEL switch being used to move */
	/* the frequency ranges of both VCOs            */
	/*                                              */
	/* The slow VCO is implemented with a 555 timer */
	/* and a 566 is used for the higher VCO.        */
	/*                                              */
	/* The sound can be tweaked with the gain and   */
	/* adder constants in the 2 lines below         */
	/************************************************/
	// SAUCER_SEL = 1 - larger saucer
	DISCRETE_MULTADD(NODE_40, ASTEROID_SAUCER_SEL, -2.5, 8.25)  // Saucer Warble rate (5.75 or 8.25Hz)
	DISCRETE_TRIANGLEWAVE(NODE_41, 1, NODE_40, 920.0, 920.0/2, 0)   // (amount of warble)

	DISCRETE_TRANSFORM4(NODE_42, ASTEROID_SAUCER_SEL, -250, NODE_41, 750, "01*2+3+")    // Large saucer is 250hz lower

	DISCRETE_TRIANGLEWAVE(NODE_43, ASTEROID_SAUCER_SND_EN, NODE_42, 76.1, 0, 0)
	DISCRETE_RCFILTER(ASTEROID_SAUCER_SND, NODE_43, 1, 1.0e-5)

	/************************************************/
	/* The Ship Fire sound is produced by a 555     */
	/* based VCO where the frequency rapidly decays */
	/* with time.                                   */
	/************************************************/
	DISCRETE_RAMP(NODE_50, ASTEROID_SHIP_FIRE_EN, ASTEROID_SHIP_FIRE_EN, (820.0-110.0)/0.28, 820.0, 110.0, 0)   /* Decay - Freq */
	DISCRETE_RCDISC(NODE_51, ASTEROID_SHIP_FIRE_EN, 53.0-7.0, 2700.0*3, 1e-5)   /* Decay - Amplitude */
	DISCRETE_ADDER2(NODE_52, 1, NODE_51, 7.0)   /* Amplitude */
	DISCRETE_TRANSFORM3(NODE_53, 4500, NODE_50, 67, "01/2+")    /* Duty */
	DISCRETE_SQUAREWAVE(NODE_54, ASTEROID_SHIP_FIRE_EN, NODE_50, NODE_52, NODE_53, 0, 0)
	DISCRETE_RCFILTER(ASTEROID_SHIP_FIRE_SND, NODE_54, 1, 1.0e-5)

	/************************************************/
	/* The Saucer Fire sound is produced by a 555   */
	/* based VCO where the frequency rapidly decays */
	/* with time.                                   */
	/************************************************/
	DISCRETE_RAMP(NODE_60, ASTEROID_SAUCER_FIRE_EN, ASTEROID_SAUCER_FIRE_EN, (830.0-630.0)/0.28, 830.0, 630.0, 0)   /* Decay - Freq */
	DISCRETE_RCDISC(NODE_61, ASTEROID_SAUCER_FIRE_EN, 49.5-7.0, 10000.0*3, 1e-5)    /* Decay - Amplitude */
	DISCRETE_ADDER2(NODE_62, 1, NODE_61, 7.0)   /* Amplitude */
	DISCRETE_TRANSFORM3(NODE_63, 4500, NODE_60, 67, "01/2+")    /* Duty */
	DISCRETE_SQUAREWAVE(NODE_64, ASTEROID_SAUCER_FIRE_EN, NODE_60, NODE_62, NODE_63, 0, 0)
	DISCRETE_RCFILTER(ASTEROID_SAUCER_FIRE_SND, NODE_64, 2, 1.0e-5)

	/************************************************/
	/* Thrust noise is a gated noise source         */
	/* fed into a filter network                    */
	/* It is an RC lowpass, followed by a           */
	/* Sallen-Key bandpass, followed by an active   */
	/* lowpass.                                     */
	/************************************************/
	DISCRETE_LFSR_NOISE(ASTEROID_NOISE, ASTEROID_NOISE_RESET, ASTEROID_NOISE_RESET, 12000.0, 1.0, 0, 0, &asteroid_lfsr)

	DISCRETE_GAIN(NODE_70, ASTEROID_NOISE, 600.0*7.6)
	DISCRETE_RCFILTER(NODE_71, NODE_70, 2200, 1e-6)
	DISCRETE_MULTIPLY(NODE_72, NODE_71, ASTEROID_THRUST_EN)
	/* TBD - replace this line with a Sallen-Key Bandpass macro */
	DISCRETE_FILTER2(NODE_73, 1, NODE_72, 89.5, (1.0 / 7.6), DISC_FILTER_BANDPASS)
	/* TBD - replace this line with a Active Lowpass macro */
	DISCRETE_FILTER1(ASTEROID_THRUST_SND, 1, NODE_73, 160, DISC_FILTER_LOWPASS)

	/************************************************/
	/* Explosion generation circuit, pitch and vol  */
	/* are variable.                                */
	/* The pitch divides using an overflow counter. */
	/* Meaning the duty cycle varies.  The on time  */
	/* is always the same (one 12Khz cycle).  But   */
	/* the off time varies.  /12 = 11 off cycles    */
	/* Then it is ANDed with the 12kHz to make a    */
	/* shorter pulse.  There is no real reason to   */
	/* do this, as the D-Latch already triggers on  */
	/* the rising edge.  So we won't add the extra  */
	/* nodes.                                       */
	/************************************************/
	DISCRETE_DIVIDE(NODE_80, 1, 12000, ASTEROID_EXPLODE_PITCH)      /* Frequency */
	DISCRETE_DIVIDE(NODE_81, 1, 100, ASTEROID_EXPLODE_PITCH)        /* Duty */
	DISCRETE_SQUAREWFIX(NODE_82, 1, NODE_80, 1.0, NODE_81, 1.0/2, 0)    /* Pitch clock */
	DISCRETE_SAMPLHOLD(NODE_83, ASTEROID_NOISE, NODE_82, DISC_SAMPHOLD_REDGE)
	DISCRETE_MULTIPLY(NODE_84, NODE_83, ASTEROID_EXPLODE_DATA)
	DISCRETE_RCFILTER(ASTEROID_EXPLODE_SND, NODE_84, 3042, 1e-6)

	/************************************************/
	/* Life enable is just 3KHz tone from the clock */
	/* generation cct according to schematics       */
	/************************************************/
	DISCRETE_SQUAREWFIX(ASTEROID_LIFE_SND, ASTEROID_LIFE_EN, 3000, 100.0, 50.0, 0, 0)

	/************************************************/
	/* Combine all 7 sound sources with a double    */
	/* adder circuit                                */
	/************************************************/
	DISCRETE_ADDER4(NODE_90, 1, ASTEROID_THUMP_SND, ASTEROID_SAUCER_SND, ASTEROID_SHIP_FIRE_SND, ASTEROID_SAUCER_FIRE_SND)
	DISCRETE_ADDER4(NODE_91, 1, ASTEROID_THRUST_SND, ASTEROID_EXPLODE_SND, ASTEROID_LIFE_SND, NODE_90)

	DISCRETE_OUTPUT(NODE_91, 65534.0 / (131.6+76.1+49.5+53.0+1000.0+600.0))     // Take the output from the mixer
DISCRETE_SOUND_END


DISCRETE_SOUND_START(astdelux)
	/************************************************/
	/* Asteroid delux sound hardware is mostly done */
	/* in the Pokey chip except for the thrust and  */
	/* explosion sounds that are a direct lift of   */
	/* the asteroids hardware hence is a clone of   */
	/* the circuit above apart from gain scaling.   */
	/*                                              */
	/* Note that the thrust enable signal is invert */
	/************************************************/
	/*                         NODE                GAIN        OFFSET  INIT */
	DISCRETE_INPUT_LOGIC (ASTEROID_THRUST_EN)
	DISCRETE_INPUT_PULSE (ASTEROID_NOISE_RESET, 1)

	DISCRETE_INPUTX_DATA (ASTEROID_EXPLODE_DATA, 1000.0/15.0,   0.0,   0)
	DISCRETE_INPUTX_DATA (ASTEROID_EXPLODE_PITCH, 1,            0,     12)

	/************************************************/
	/* Thrust noise is a gated noise source         */
	/* fed into a filter network                    */
	/* It is an RC lowpass, followed by a           */
	/* Sallen-Key bandpass, followed by an active   */
	/* lowpass.                                     */
	/************************************************/
	DISCRETE_LFSR_NOISE(ASTEROID_NOISE, ASTEROID_NOISE_RESET, ASTEROID_NOISE_RESET, 12000.0, 1.0, 0, 0, &asteroid_lfsr)

	DISCRETE_GAIN(NODE_70, ASTEROID_NOISE, 1000.0*7.6)
	DISCRETE_RCFILTER(NODE_71, NODE_70, 2200, 1e-6)
	DISCRETE_MULTIPLY(NODE_72, NODE_71, ASTEROID_THRUST_EN)
	/* TBD - replace this line with a Sallen-Key Bandpass macro */
	DISCRETE_FILTER2(NODE_73, 1, NODE_72, 89.5, (1.0 / 7.6), DISC_FILTER_BANDPASS)
	/* TBD - replace this line with a Active Lowpass macro */
	DISCRETE_FILTER1(ASTEROID_THRUST_SND, 1, NODE_73, 160, DISC_FILTER_LOWPASS)

	/************************************************/
	/* Explosion generation circuit, pitch and vol  */
	/* are variable.                                */
	/* The pitch divides using an overflow counter. */
	/* Meaning the duty cycle varies.  The on time  */
	/* is always the same (one 12Khz cycle).  But   */
	/* the off time varies.  /12 = 11 off cycles    */
	/* Then it is ANDed with the 12kHz to make a    */
	/* shorter pulse.  There is no real reason to   */
	/* do this, as the D-Latch already triggers on  */
	/* the rising edge.  So we won't add the extra  */
	/* nodes.                                       */
	/************************************************/
	DISCRETE_DIVIDE(NODE_80, 1, 12000, ASTEROID_EXPLODE_PITCH)      /* Frequency */
	DISCRETE_DIVIDE(NODE_81, 1, 100, ASTEROID_EXPLODE_PITCH)        /* Duty */
	DISCRETE_SQUAREWFIX(NODE_82, 1, NODE_80, 1.0, NODE_81, 1.0/2, 0)    /* Pitch clock */
	DISCRETE_SAMPLHOLD(NODE_83, ASTEROID_NOISE, NODE_82, DISC_SAMPHOLD_REDGE)
	DISCRETE_MULTIPLY(NODE_84, NODE_83, ASTEROID_EXPLODE_DATA)
	DISCRETE_RCFILTER(ASTEROID_EXPLODE_SND, NODE_84, 3042, 1e-6)

	/************************************************/
	/* Combine all 7 sound sources with a double    */
	/* adder circuit                                */
	/************************************************/
	DISCRETE_ADDER2(NODE_90, 1, ASTEROID_THRUST_SND, ASTEROID_EXPLODE_SND)

	DISCRETE_OUTPUT(NODE_90, 65534.0/(1000+600))    // Take the output from the mixer
DISCRETE_SOUND_END


WRITE8_MEMBER(asteroid_state::asteroid_explode_w)
{
	m_discrete->write(space,ASTEROID_EXPLODE_DATA,(data&0x3c)>>2);                // Volume
	/* We will modify the pitch data to send the divider value. */
	switch ((data&0xc0))
	{
		case 0x00:
			data = 12;
			break;
		case 0x40:
			data = 6;
			break;
		case 0x80:
			data = 3;
			break;
		case 0xc0:
			data = 5;
			break;
	}
	m_discrete->write(space, ASTEROID_EXPLODE_PITCH, data);
}

WRITE8_MEMBER(asteroid_state::asteroid_thump_w)
{
	m_discrete->write(space, ASTEROID_THUMP_EN,   data & 0x10);
	m_discrete->write(space, ASTEROID_THUMP_DATA, data & 0x0f);
}

WRITE8_MEMBER(asteroid_state::asteroid_sounds_w)
{
	m_discrete->write(space, NODE_RELATIVE(ASTEROID_SAUCER_SND_EN, offset), data & 0x80);
}

WRITE8_MEMBER(asteroid_state::astdelux_sounds_w)
{
	/* Only ever activates the thrusters in Astdelux */
	m_discrete->write(space, ASTEROID_THRUST_EN, data & 0x80);
}

WRITE8_MEMBER(asteroid_state::asteroid_noise_reset_w)
{
	m_discrete->write(space, ASTEROID_NOISE_RESET, 0);
}
