// license:BSD-3-Clause
// copyright-holders:Mike Coates
/*******************************************************************************

  circus_a.cpp

  Functions to emulate the audio hardware of the machine.

*******************************************************************************/

#include "emu.h"
#include "circus.h"



/*******************************************************************************
    Circus
*******************************************************************************/

const char *const circus_sample_names[] =
{
	"*circus",
	"pop",
	"miss",
	"bounce",
	nullptr
};

// Nodes - Inputs
#define CIRCUS_MUSIC_BIT    NODE_01
// Nodes - Sounds
#define CIRCUS_MUSIC_SND    NODE_10

DISCRETE_SOUND_START(circus_discrete)
	/************************************************/
	/* Input register mapping for circus            */
	/************************************************/
	DISCRETE_INPUTX_NOT(CIRCUS_MUSIC_BIT, 20000, 0, 1)

	/************************************************/
	/* Music is just a 1 bit DAC                    */
	/************************************************/
	DISCRETE_CRFILTER(CIRCUS_MUSIC_SND, CIRCUS_MUSIC_BIT, RES_K(50), CAP_U(.1)) /* 50K is just an average value */

	DISCRETE_OUTPUT(CIRCUS_MUSIC_SND, 1)
DISCRETE_SOUND_END


void circus_state::sound_w(uint8_t data)
{
	// Bits 4-6 enable/disable trigger different events
	switch ((data & 0x70) >> 4)
	{
		case 0: // All Off
			m_discrete->write(CIRCUS_MUSIC_BIT, 0);
			break;

		case 1: // Music
			m_discrete->write(CIRCUS_MUSIC_BIT, 1);
			break;

		case 2: // Pop
			m_samples->start(0, 0);
			break;

		case 3: // Normal Video
			break;

		case 4: // Miss
			m_samples->start(1, 1);
			break;

		case 5: // Invert Video
			break;

		case 6: // Bounce
			m_samples->start(2, 2);
			break;

		case 7: // Unused
			break;
	}
}



/*******************************************************************************
    Robot Bowl
*******************************************************************************/

const char *const robotbwl_sample_names[] =
{
	"*robotbwl",
	"hit",
	"roll",
	"balldrop",
	"demerit",
	"reward",
	nullptr
};

// Nodes - Inputs
#define ROBOTBWL_MUSIC_BIT      NODE_01
// Nodes - Sounds
#define ROBOTBWL_MUSIC_SND      NODE_10

DISCRETE_SOUND_START(robotbwl_discrete)
	/************************************************/
	/* Input register mapping for robotbwl          */
	/************************************************/
	DISCRETE_INPUTX_LOGIC(ROBOTBWL_MUSIC_BIT, 30000, 0, 0)

	/************************************************/
	/* Music is just a 1 bit DAC                    */
	/************************************************/
	DISCRETE_RCFILTER(NODE_20, ROBOTBWL_MUSIC_BIT, RES_K(10), CAP_U(.47))
	DISCRETE_CRFILTER(ROBOTBWL_MUSIC_SND, NODE_20, RES_K(10) + RES_K(22), CAP_U(.1))

	DISCRETE_OUTPUT(ROBOTBWL_MUSIC_SND, 1)
DISCRETE_SOUND_END


void robotbwl_state::sound_w(uint8_t data)
{
	if (data & 0x40) // Hit
		m_samples->start(0, 0);

	if (data & 0x20) // Roll
		m_samples->start(1, 1);

	if (data & 0x10) // Ball Drop
		m_samples->start(2, 2);

	m_discrete->write(ROBOTBWL_MUSIC_BIT, data & 0x08); // Footsteps

	//if (data & 0x04) // Invert

	if (data & 0x02) // Demerit
		m_samples->start(3, 3);

	if (data & 0x01) // Reward
		m_samples->start(4, 4);
}



/*******************************************************************************
    Crash
*******************************************************************************/

const char *const crash_sample_names[] =
{
	"*crash",
	"crash",
	nullptr
};

static const discrete_mixer_desc crash_mixer =
{
	DISC_MIXER_IS_RESISTOR,
	{RES_K(22), RES_K(5)},
	{0},
	{CAP_U(.1), CAP_U(.1)},
	0, RES_K(100), 0, CAP_U(.1), 0, 10000
};

static const discrete_555_desc crash_beeper_555m =
{
	DISC_555_OUT_SQW | DISC_555_OUT_DC | DISC_555_TRIGGER_IS_LOGIC,
	5, // B+ voltage of 555
	DEFAULT_555_VALUES
};

static const discrete_555_desc crash_beeper_555a =
{
	DISC_555_OUT_SQW | DISC_555_OUT_DC,
	5, // B+ voltage of 555
	DEFAULT_555_VALUES
};

// Nodes - Inputs
#define CRASH_MUSIC_BIT     NODE_01
#define CRASH_BEEPER_EN     NODE_02
// Nodes - Adjusters
#define CRASH_R63           NODE_10
#define CRASH_R39           NODE_11
// Nodes - Sounds
#define CRASH_MUSIC_SND     NODE_20
#define CRASH_BEEPER_SND    NODE_21

DISCRETE_SOUND_START(crash_discrete)
	/************************************************/
	/* Input register mapping for crash             */
	/************************************************/
	DISCRETE_INPUT_LOGIC(CRASH_MUSIC_BIT)
	DISCRETE_INPUT_PULSE(CRASH_BEEPER_EN, 1)

	DISCRETE_ADJUSTMENT(CRASH_R63, 0, 5.0*RES_K(100)/(RES_K(47+100))-0.5, DISC_LINADJ, "R63")
	DISCRETE_ADJUSTMENT(CRASH_R39, 0, 1, DISC_LINADJ, "R39")

	/************************************************/
	/* Music is just a 1 bit DAC                    */
	/************************************************/
	DISCRETE_MULTADD(CRASH_MUSIC_SND, CRASH_MUSIC_BIT, CRASH_R63, 0.5)

	/************************************************/
	/* Beeper - oneshot gates tone                  */
	/************************************************/
	DISCRETE_555_MSTABLE(NODE_30, 1, CRASH_BEEPER_EN, RES_K(22), CAP_U(.47), &crash_beeper_555m)
	DISCRETE_555_ASTABLE(NODE_31, NODE_30, RES_K(4.7), RES_K(4.7), CAP_U(.1), &crash_beeper_555a)
	DISCRETE_MULTIPLY(CRASH_BEEPER_SND, NODE_31, CRASH_R39)

	/************************************************/
	/* Final mix with gain                          */
	/************************************************/
	DISCRETE_MIXER2(NODE_90, 1, CRASH_MUSIC_SND, CRASH_BEEPER_SND, &crash_mixer)

	DISCRETE_OUTPUT(NODE_90, 1)
DISCRETE_SOUND_END


void crash_state::sound_w(uint8_t data)
{
	// Bits 4-6 enable/disable trigger different events
	switch ((data & 0x70) >> 4)
	{
		case 0: // All Off
			m_discrete->write(CRASH_MUSIC_BIT, 0);
			break;

		case 1: // Music
			m_discrete->write(CRASH_MUSIC_BIT, 1);
			break;

		case 2: // Crash
			m_samples->start(0, 0);
			break;

		case 3: // Normal Video and Beep
			m_discrete->write(CRASH_BEEPER_EN, 0);
			break;

		case 4: // Skid
			break;

		case 5: // Invert Video and Beep
			m_discrete->write(CRASH_BEEPER_EN, 0);
			break;

		case 6: // Hi Motor
			break;

		case 7: // Low Motor
			break;
	}
}



/*******************************************************************************
    Rip Cord
*******************************************************************************/

const char *const ripcord_sample_names[] =
{
	"*ripcord",
	"splash",
	"scream",
	"chute",
	"whistle",
	nullptr
};

void ripcord_state::sound_w(uint8_t data)
{
	// Bits 4-6 enable/disable trigger different events
	switch ((data & 0x70) >> 4)
	{
		case 0: // All Off
			m_discrete->write(CIRCUS_MUSIC_BIT, 0);
			break;

		case 1: // Music
			m_discrete->write(CIRCUS_MUSIC_BIT, 1);
			break;

		case 2: // Splash
			m_samples->start(0, 0);
			break;

		case 3: // Normal Video
			break;

		case 4: // Scream
			m_samples->start(1, 1);
			break;

		case 5: // Invert Video
			break;

		case 6: // Chute Open
			m_samples->start(2, 2);
			break;

		case 7: // Whistle
			m_samples->start(3, 3);
			break;
	}
}
