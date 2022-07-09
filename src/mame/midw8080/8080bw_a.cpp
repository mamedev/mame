// license:BSD-3-Clause
// copyright-holders:Nicola Salmoria, Tormod Tjaberg, Mirko Buffoni,Lee Taylor, Valerio Verrando, Zsolt Vasvari,Aaron Giles,Jonathan Gevaryahu,hap,Robbbert
// thanks-to:Michael Strutts, Marco Cassili
/* 8080bw.c *******************************************/

#include "emu.h"
#include "8080bw.h"

#include "sound/samples.h"
#include "sound/discrete.h"
#include "speaker.h"

#include <cmath>

//#define VERBOSE 1
#include "logmacro.h"


/*******************************************************/
/*                                                     */
/* Global state registration                           */
/*                                                     */
/*******************************************************/

MACHINE_START_MEMBER(_8080bw_state,extra_8080bw_sh)
{
	save_item(NAME(m_port_1_last_extra));
	save_item(NAME(m_port_2_last_extra));
}

/*************************************
 *
 *  Device type globals
 *
 *************************************/

DEFINE_DEVICE_TYPE(CANE_AUDIO,  cane_audio_device,  "cane_audio",  "Model Racing Cane Audio")


/*************************************
 *
 *  Space Invaders
 *
 *  Author      : Tormod Tjaberg
 *  Created     : 1997-04-09
 *  Description : Sound routines for the 'invaders' games
 *
 *  Note:
 *  The samples were taken from Michael Strutt's (mstrutt@pixie.co.za)
 *  excellent space invader emulator and converted to signed samples so
 *  they would work under SEAL. The port info was also gleaned from
 *  his emulator. These sounds should also work on all the invader games.
 *
 *************************************/

static const char *const invaders_sample_names[] =
{
	"*invaders",
	"1",        /* shot/missle */
	"2",        /* base hit/explosion */
	"3",        /* invader hit */
	"4",        /* fleet move 1 */
	"5",        /* fleet move 2 */
	"6",        /* fleet move 3 */
	"7",        /* fleet move 4 */
	"8",        /* UFO/saucer hit */
	"9",        /* bonus base */
	nullptr
};


/* left in for all games that hack into invaders samples for audio */
void invaders_clone_state::invaders_samples_audio(machine_config &config)
{
	SPEAKER(config, "mono").front_center();

	SN76477(config, m_sn);
	m_sn->set_noise_params(0, 0, 0);
	m_sn->set_decay_res(0);
	m_sn->set_attack_params(0, RES_K(100));
	m_sn->set_amp_res(RES_K(56));
	m_sn->set_feedback_res(RES_K(10));
	m_sn->set_vco_params(0, CAP_U(0.1), RES_K(8.2));
	m_sn->set_pitch_voltage(5.0);
	m_sn->set_slf_params(CAP_U(1.0), RES_K(120));
	m_sn->set_oneshot_params(0, 0);
	m_sn->set_vco_mode(1);
	m_sn->set_mixer_params(0, 0, 0);
	m_sn->set_envelope_params(1, 0);
	m_sn->set_enable(1);
	m_sn->add_route(ALL_OUTPUTS, "mono", 0.5);

	SAMPLES(config, m_samples);
	m_samples->set_channels(6);
	m_samples->set_samples_names(invaders_sample_names);
	m_samples->add_route(ALL_OUTPUTS, "mono", 1.0);
}



/*******************************************************/
/*                                                     */
/* Midway "Space Invaders Part II"                     */
/*                                                     */
/*******************************************************/

void _8080bw_state::invadpt2_sh_port_1_w(uint8_t data)
{
	uint8_t rising_bits = data & ~m_port_1_last_extra;

	m_sn->enable_w(!BIT(data, 0));                      // SAUCER SOUND

	if (BIT(rising_bits, 1)) m_samples->start(0, 0);    // MISSLE SOUND
	if (BIT(rising_bits, 2)) m_samples->start(1, 1);    // EXPLOSION
	if (BIT(rising_bits, 3)) m_samples->start(2, 2);    // INVADER HIT
	if (BIT(rising_bits, 4)) m_samples->start(5, 8);    // BONUS MISSILE BASE

	m_screen_red = data & 0x04;

	machine().sound().system_mute(!BIT(data, 5));

	m_port_1_last_extra = data;
}

void _8080bw_state::invadpt2_sh_port_2_w(uint8_t data)
{
	/* FLEET (movement)

	   DO = 20K + 20K
	   D1 = 68K
	   D2 = 82K
	   D3 = 100K */

	uint8_t rising_bits = data & ~m_port_2_last_extra;

	if (BIT(rising_bits, 0)) m_samples->start(4, 3);    // FLEET
	if (BIT(rising_bits, 1)) m_samples->start(4, 4);    // FLEET
	if (BIT(rising_bits, 2)) m_samples->start(4, 5);    // FLEET
	if (BIT(rising_bits, 3)) m_samples->start(4, 6);    // FLEET
	if (BIT(rising_bits, 4)) m_samples->start(3, 7);    // SAUCER HIT

	m_flip_screen = BIT(data, 5) & ioport(CABINET_PORT_TAG)->read();
	m_color_map = BIT(data, 5);

	m_port_2_last_extra = data;
}



/*******************************************************/
/*                                                     */
/* Space Ranger                                        */
/*                                                     */
/*******************************************************/

void _8080bw_state::spacerng_sh_port_2_w(uint8_t data)
{
	uint8_t rising_bits = data & ~m_port_2_last_extra;

	if (BIT(rising_bits, 0)) m_samples->start(4, 3);    // FLEET
	if (BIT(rising_bits, 1)) m_samples->start(4, 4);    // FLEET
	if (BIT(rising_bits, 2)) m_samples->start(4, 5);    // FLEET
	if (BIT(rising_bits, 3)) m_samples->start(4, 6);    // FLEET
	if (BIT(rising_bits, 4)) m_samples->start(3, 7);    // SAUCER HIT

	m_flip_screen = BIT(~data, 5) & ioport(CABINET_PORT_TAG)->read();

	m_port_2_last_extra = data;
}

/*******************************************************/
/*                                                     */
/* Sanritsu "Space War"                                */
/*                                                     */
/*******************************************************/

void _8080bw_state::spcewars_sh_port_w(uint8_t data)
{
	uint8_t rising_bits = data & ~m_port_1_last_extra;

	m_sn->enable_w(!BIT(data, 0));                      // Saucer Sound

	if (BIT(rising_bits, 1)) m_samples->start(0, 0);    // Shot Sound
	if (BIT(rising_bits, 2)) m_samples->start(1, 1);    // Base Hit
	if (BIT(rising_bits, 3)) m_samples->start(2, 2);    // Invader Hit

	m_speaker->level_w(BIT(data, 4));                   // Various bitstream tunes

	m_port_1_last_extra = data;
}



/*******************************************************/
/*                                                     */
/* lrescue, grescue, lrescuem, desterth                */
/*                                                     */
/*******************************************************/

const char *const lrescue_sample_names[] =
{
	"*lrescue",
	"alienexplosion",
	"rescueshipexplosion",
	"beamgun",
	"thrust",
	"bonus2",
	"bonus3",
	"shootingstar",
	"stepl",
	"steph",
	nullptr
};

void _8080bw_state::lrescue_sh_port_1_w(uint8_t data)
{
	uint8_t rising_bits = data & ~m_port_1_last_extra;

	if (BIT(rising_bits, 0)) m_samples->start(0, 3);    // Thrust
	if (BIT(rising_bits, 1)) m_samples->start(1, 2);    // Shot Sound
	if (BIT(rising_bits, 2)) m_samples->start(0, 1);    // Death
	if (BIT(rising_bits, 3)) m_samples->start(1, 0);    // Alien Hit
	if (BIT(rising_bits, 4)) m_samples->start(2, 5);    // Bonus Ship (not confirmed)

	machine().sound().system_mute(!BIT(data, 5));

	m_screen_red = data & 0x04;

	m_port_1_last_extra = data;
}

void _8080bw_state::lrescue_sh_port_2_w(uint8_t data)
{
	uint8_t rising_bits = data & ~m_port_2_last_extra;
	uint8_t falling_bits = ~data & m_port_2_last_extra;

	if (BIT(rising_bits, 0)) m_samples->start(1, 8);    // Footstep high tone
	if (BIT(rising_bits, 1)) m_samples->start(1, 7);    // Footstep low tone
	if (BIT(rising_bits, 2)) m_samples->start(1, 4);    // Bonus when counting men saved

	m_speaker->level_w(BIT(data, 3));                   // Bitstream tunes - endlevel and bonus1

	if (BIT(rising_bits, 4)) m_samples->start(3, 6);    // Shooting Star and Rescue Ship sounds
	if (BIT(falling_bits, 4)) m_samples->stop(3);       // This makes the rescue ship sound beep on and off

	m_flip_screen = BIT(data, 5) & ioport(CABINET_PORT_TAG)->read();

	m_port_2_last_extra = data;
}



/*******************************************************/
/*                                                     */
/* Cosmo                                               */
/*                                                     */
/*******************************************************/

void _8080bw_state::cosmo_sh_port_2_w(uint8_t data)
{
	/* inverted flip screen bit */
	invadpt2_sh_port_2_w(data ^ 0x20);
}



/*******************************************************/
/*                                                     */
/* Taito "Balloon Bomber"                              */
/*   The sounds are not the correct ones               */
/*                                                     */
/*******************************************************/

/*************************************
 *
 *  Discrete Sound
 * (copied from indianbt)
 *
 *************************************/

static const discrete_dac_r1_ladder ballbomb_music_dac =
	{3, {0, RES_K(47), RES_K(12)}, 0, 0, 0, CAP_U(0.1)};

#define BALLBOMB_MUSIC_CLK      (3993.6*2*2*2)

/* Nodes - Inputs */
#define BALLBOMB_MUSIC_DATA     NODE_01
/* Nodes - Sounds */
#define BALLBOMB_MUSIC          NODE_11

DISCRETE_SOUND_START(ballbomb_discrete)

	DISCRETE_INPUT_DATA (BALLBOMB_MUSIC_DATA)

/******************************************************************************
 *
 * Music Generator
 *
 ******************************************************************************/
	DISCRETE_NOTE(NODE_20, 1, BALLBOMB_MUSIC_CLK, BALLBOMB_MUSIC_DATA, 255, 5, DISC_CLK_IS_FREQ)

	// Convert count to 7492 output
	DISCRETE_TRANSFORM2(NODE_21, NODE_20, 2, "01>0+")

	DISCRETE_DAC_R1(NODE_22, NODE_21, DEFAULT_TTL_V_LOGIC_1, &ballbomb_music_dac)

/******************************************************************************
 *
 * Final Mixing and Output
 *
 ******************************************************************************/
	DISCRETE_CRFILTER(NODE_90, NODE_22, RES_K(10), CAP_U(0.1))

	DISCRETE_OUTPUT(NODE_90, 21000)

DISCRETE_SOUND_END

void _8080bw_state::ballbomb_01_w(uint8_t data)
{
	m_discrete->write(BALLBOMB_MUSIC_DATA, data|0x80);
}

void _8080bw_state::ballbomb_sh_port_1_w(uint8_t data)
{
	uint8_t rising_bits = data & ~m_port_1_last_extra;

	if (BIT(rising_bits, 0)) m_samples->start(1, 2);    // Hit a balloon
	if (BIT(rising_bits, 1)) m_samples->start(2, 0);    // Shot Sound
	if (BIT(rising_bits, 2)) m_samples->start(2, 1);    // Base Hit
	if (BIT(rising_bits, 3)) m_samples->start(1, 7);    // Hit a Bomb
	if (BIT(rising_bits, 4)) m_samples->start(3, 8);    // Bonus Base at 1500 points

	machine().sound().system_mute(!BIT(data, 5));

	m_screen_red = data & 0x04;

	m_port_1_last_extra = data;
}

void _8080bw_state::ballbomb_sh_port_2_w(uint8_t data)
{
	uint8_t rising_bits = data & ~m_port_2_last_extra;

	if (BIT(data, 0)) m_samples->start(0, 7);           // Indicates plane will drop bombs
	if (BIT(data, 2)) m_samples->start(0, 4);           // Plane is dropping new balloons at start of level
	if (BIT(rising_bits, 4)) m_samples->start(2, 2);    // Balloon hit and bomb drops

	m_flip_screen = BIT(data, 5) & ioport(CABINET_PORT_TAG)->read();

	m_port_2_last_extra = data;
}



/*******************************************************/
/*                                                     */
/* Taito "Indian Battle"                               */
/* Sept 2005, D.R.                                     */
/*******************************************************/

static const discrete_dac_r1_ladder indianbt_music_dac =
	{3, {0, RES_K(47), RES_K(12)}, 0, 0, 0, CAP_U(0.1)};

#define INDIANBT_MUSIC_CLK      (7680.0*2*2*2)

/* Nodes - Inputs */
#define INDIANBT_MUSIC_DATA     NODE_01
/* Nodes - Sounds */
#define INDIANBT_MUSIC          NODE_11

DISCRETE_SOUND_START(indianbt_discrete)

	DISCRETE_INPUT_DATA (INDIANBT_MUSIC_DATA)

/******************************************************************************
 *
 * Music Generator
 *
 ******************************************************************************/
	DISCRETE_NOTE(NODE_20, 1, INDIANBT_MUSIC_CLK, INDIANBT_MUSIC_DATA, 255, 5, DISC_CLK_IS_FREQ)

	// Convert count to 7492 output
	DISCRETE_TRANSFORM2(NODE_21, NODE_20, 2, "01>0+")

	DISCRETE_DAC_R1(NODE_22, NODE_21, DEFAULT_TTL_V_LOGIC_1, &indianbt_music_dac)

/******************************************************************************
 *
 * Final Mixing and Output
 *
 ******************************************************************************/
	DISCRETE_CRFILTER(NODE_90, NODE_22, RES_K(10), CAP_U(0.1))

	DISCRETE_OUTPUT(NODE_90, 21000)

DISCRETE_SOUND_END

void _8080bw_state::indianbt_sh_port_1_w(uint8_t data)
{
	/* bit 4 occurs every 5.25 seconds during gameplay */
	uint8_t rising_bits = data & ~m_port_1_last_extra;

	if (BIT(rising_bits, 0)) m_samples->start(1, 7);    // Death
	if (BIT(rising_bits, 1)) m_samples->start(0, 1);    // Shot Sound
	if (BIT(rising_bits, 2)) m_samples->start(2, 3);    // Move
	if (BIT(rising_bits, 3)) m_samples->start(3, 2);    // Hit

	machine().sound().system_mute(!BIT(data, 5));

	m_port_1_last_extra = data;
}

void _8080bw_state::indianbt_sh_port_2_w(uint8_t data)
{
	uint8_t rising_bits = data & ~m_port_2_last_extra;

	if (BIT(rising_bits, 0)) m_samples->start(4, 0);    // Bird dropped an egg, Lasso used
	if (BIT(rising_bits, 1)) m_samples->start(4, 2);    // Egg hatches, egg shot
	if (BIT(rising_bits, 3)) m_samples->start(5, 0);    // Grabber, Lasso caught something
	if (BIT(rising_bits, 4)) m_samples->start(3, 7);    // Lasso sound

	m_flip_screen = BIT(data, 5) & ioport(CABINET_PORT_TAG)->read();

	m_port_2_last_extra = data;
}

void _8080bw_state::indianbt_sh_port_3_w(uint8_t data)
{
	m_discrete->write(INDIANBT_MUSIC_DATA, data);
}

void _8080bw_state::indianbtbr_sh_port_1_w(uint8_t data)
{
	uint8_t rising_bits = data & ~m_port_1_last_extra;

	if (BIT(rising_bits, 0)) m_samples->start(4, 7);    // Lasso
	if (BIT(rising_bits, 2)) m_samples->start(0, 1);    // Shot Sound
	if (BIT(rising_bits, 3)) m_samples->start(3, 2);    // Hit

	machine().sound().system_mute(!BIT(data, 5));

	m_port_1_last_extra = data;
}

void _8080bw_state::indianbtbr_sh_port_2_w(uint8_t data)
{
	uint8_t rising_bits = data & ~m_port_2_last_extra;

	if (BIT(rising_bits, 3)) m_samples->start(2, 3);    // Move
	if (BIT(rising_bits, 4)) m_samples->start(3, 7);    // Death

	m_flip_screen = BIT(data, 5) & ioport(CABINET_PORT_TAG)->read();

	m_port_2_last_extra = data;
}



/*******************************************************************/
/*                                                                 */
/* Taito "Polaris"                                                 */
/*                                                                 */
/* D.R.                                                            */
/* The R/C values in the schematic may have no bearing in reality. */
/* I have noted some differences from a real board.                */
/*                                                                 */
/*******************************************************************/

static const discrete_lfsr_desc polaris_lfsr={
	DISC_CLK_IS_FREQ,
	17,         /* Bit Length */
	0,          /* Reset Value */
	4,          /* Use Bit 4 as XOR input 0 */
	16,         /* Use Bit 16 as XOR input 1 */
	DISC_LFSR_XOR,      /* Feedback stage1 is XOR */
	DISC_LFSR_OR,       /* Feedback stage2 is just stage 1 output OR with external feed */
	DISC_LFSR_REPLACE,  /* Feedback stage3 replaces the shifted register contents */
	0x000001,       /* Everything is shifted into the first bit only */
	0,          /* Output is not inverted */
	12          /* Output bit */
};

static const discrete_dac_r1_ladder polaris_music_dac =
	{2, {RES_K(47), RES_K(12), 0,0,0,0,0,0}, 0, 0, 0, CAP_P(180)};

static const discrete_op_amp_filt_info polaris_music_op_amp_filt_info =
	{RES_K(580), 0, 0, RES_M(2.2), RES_M(1), CAP_U(.01), 0, 0, 0, 12, 0};

static const discrete_op_amp_filt_info polaris_nol_op_amp_filt_info =
	{560, RES_K(6.8), RES_K(1002), RES_M(2.2), RES_M(1), CAP_U(.22), CAP_U(.22), CAP_U(1), 0, 12, 0};

static const discrete_op_amp_filt_info polaris_noh_op_amp_filt_info =
	{560, RES_K(6.8), RES_K(1002), RES_M(2.2), RES_M(1), CAP_U(.001), CAP_U(.001), CAP_U(.01), 0, 12, 0};

static const discrete_op_amp_osc_info polaris_sonar_vco_info =
	{DISC_OP_AMP_OSCILLATOR_VCO_1 | DISC_OP_AMP_IS_NORTON | DISC_OP_AMP_OSCILLATOR_OUT_CAP, RES_M(1), RES_K(680), RES_K(680), RES_M(1), RES_M(1), RES_K(120), RES_M(1), 0, CAP_P(180), 12};

static const discrete_op_amp_tvca_info polaris_sonar_tvca_info =
	{ RES_M(2.7), RES_K(680), 0, RES_K(680), RES_K(1), RES_K(120), RES_K(560), 0, 0, 0, 0, CAP_U(1), 0, 0, 0, 12, 12, 12, 12, DISC_OP_AMP_TRIGGER_FUNCTION_NONE, DISC_OP_AMP_TRIGGER_FUNCTION_NONE, DISC_OP_AMP_TRIGGER_FUNCTION_TRG1, DISC_OP_AMP_TRIGGER_FUNCTION_TRG0_INV, DISC_OP_AMP_TRIGGER_FUNCTION_NONE, DISC_OP_AMP_TRIGGER_FUNCTION_NONE};

static const discrete_op_amp_osc_info polaris_boat_mod_info =
	{DISC_OP_AMP_OSCILLATOR_1 | DISC_OP_AMP_IS_NORTON | DISC_OP_AMP_OSCILLATOR_OUT_CAP, RES_M(1), RES_K(10), RES_K(100), RES_K(120), RES_M(1), 0, 0, 0, CAP_U(.22), 12};

static const discrete_op_amp_osc_info polaris_boat_vco_info =
	{DISC_OP_AMP_OSCILLATOR_VCO_1 | DISC_OP_AMP_IS_NORTON | DISC_OP_AMP_OSCILLATOR_OUT_CAP, RES_M(1), RES_K(680), RES_K(680), RES_M(1), RES_M(1), 0, 0, 0, CAP_P(180), 12};

static const discrete_op_amp_tvca_info polaris_shot_tvca_info =
	{ RES_M(2.7), RES_K(680), RES_K(680), RES_K(680), RES_K(1), 0, RES_K(680), 0, 0, 0, 0, CAP_U(1), 0, 0, 0, 12, 12, 12, 12, DISC_OP_AMP_TRIGGER_FUNCTION_TRG0_INV, DISC_OP_AMP_TRIGGER_FUNCTION_TRG0, DISC_OP_AMP_TRIGGER_FUNCTION_TRG0, DISC_OP_AMP_TRIGGER_FUNCTION_NONE, DISC_OP_AMP_TRIGGER_FUNCTION_NONE, DISC_OP_AMP_TRIGGER_FUNCTION_NONE};

static const discrete_op_amp_tvca_info polaris_shiphit_tvca_info =
	{ RES_M(2.7), RES_K(680), RES_K(680), RES_K(680), RES_K(1), 0, RES_K(680), 0, 0, 0, 0, CAP_U(1), 0, 0, 0, 12, 12, 12, 12, DISC_OP_AMP_TRIGGER_FUNCTION_TRG0_INV, DISC_OP_AMP_TRIGGER_FUNCTION_NONE, DISC_OP_AMP_TRIGGER_FUNCTION_TRG0, DISC_OP_AMP_TRIGGER_FUNCTION_NONE, DISC_OP_AMP_TRIGGER_FUNCTION_NONE, DISC_OP_AMP_TRIGGER_FUNCTION_NONE};

static const discrete_op_amp_tvca_info polaris_exp_tvca_info =
	{ RES_M(2.7), RES_K(680), 0, RES_K(680), RES_K(1), 0, RES_K(680), 0, 0, 0, 0, CAP_U(.33), 0, 0, 0, 12, 12, 12, 12, DISC_OP_AMP_TRIGGER_FUNCTION_NONE, DISC_OP_AMP_TRIGGER_FUNCTION_NONE, DISC_OP_AMP_TRIGGER_FUNCTION_TRG0, DISC_OP_AMP_TRIGGER_FUNCTION_NONE, DISC_OP_AMP_TRIGGER_FUNCTION_NONE, DISC_OP_AMP_TRIGGER_FUNCTION_NONE};

// The schematic shows a .22uF cap but Guru's board has a 1uF
static const discrete_op_amp_tvca_info polaris_hit_tvca_info =
	{ RES_M(2.7), RES_K(1360), RES_K(1360), RES_K(680), RES_K(1), 0, RES_K(680), 0, 0, 0, 0, CAP_U(1), 0, 0, 0, 12, 12, 12, 12, DISC_OP_AMP_TRIGGER_FUNCTION_TRG0, DISC_OP_AMP_TRIGGER_FUNCTION_TRG1, DISC_OP_AMP_TRIGGER_FUNCTION_TRG01_NAND, DISC_OP_AMP_TRIGGER_FUNCTION_NONE, DISC_OP_AMP_TRIGGER_FUNCTION_NONE, DISC_OP_AMP_TRIGGER_FUNCTION_NONE};

// The schematic shows a 1uF cap but Guru's board has a 2.2uF
static const discrete_integrate_info polaris_plane_integrate_info =
	{DISC_INTEGRATE_OP_AMP_2 | DISC_OP_AMP_IS_NORTON, RES_K(1001), RES_K(1001), RES_K(101), CAP_U(2.2), 12, 12, DISC_OP_AMP_TRIGGER_FUNCTION_TRG0, DISC_OP_AMP_TRIGGER_FUNCTION_TRG0_INV, DISC_OP_AMP_TRIGGER_FUNCTION_TRG1_INV};

// A bit of a cheat.  The schematic show the cap as 47p, but that makes the frequency too high.
// Guru's board has a .01 cap, which would make the freq sub-sonic using the other schematic values.
// I will use 2000p until the proper values can be confirmed.
static const discrete_op_amp_osc_info polaris_plane_vco_info =
	{DISC_OP_AMP_OSCILLATOR_VCO_1 | DISC_OP_AMP_IS_NORTON | DISC_OP_AMP_OSCILLATOR_OUT_CAP, RES_M(1), RES_K(680), RES_K(680), RES_M(1), RES_M(1), RES_K(100), RES_K(10), RES_K(100), CAP_U(0.002), 12};

static const discrete_mixer_desc polaris_mixer_vr1_desc =
	{DISC_MIXER_IS_RESISTOR,
		{RES_K(66), RES_K(43), RES_K(20), RES_K(43)},
		{0},    // no variable resistors
		{CAP_U(1), CAP_U(1), CAP_U(1), CAP_U(1)},
		0, RES_K(50), 0, 0, 0, 1};

static const discrete_mixer_desc polaris_mixer_vr2_desc =
	{DISC_MIXER_IS_RESISTOR,
		{RES_K(66), RES_K(110)},
		{0},    // no variable resistors
		{CAP_U(1), CAP_U(1)},
		0, RES_K(50), 0, 0, 0, 1};

// Note: the final gain leaves the explosions (SX3) at a level
// where they clip.  From the schematics, this is how they wanted it.
// This makes them have more bass and distortion.
static const discrete_mixer_desc polaris_mixer_vr4_desc =
	{DISC_MIXER_IS_RESISTOR,
		{RES_K(22), RES_K(20), RES_K(22), RES_K(22)},
		{0},    // no variable resistors
		{0, CAP_U(1), 0, 0},
		0, RES_K(50), 0, CAP_U(1), 0, 40000};

/* Nodes - Inputs */
#define POLARIS_MUSIC_DATA      NODE_01
#define POLARIS_SX0_EN          NODE_02
#define POLARIS_SX1_EN          NODE_03
#define POLARIS_SX2_EN          NODE_04
#define POLARIS_SX3_EN          NODE_05
#define POLARIS_SX5_EN          NODE_06
#define POLARIS_SX6_EN          NODE_07
#define POLARIS_SX7_EN          NODE_08
#define POLARIS_SX9_EN          NODE_09
#define POLARIS_SX10_EN         NODE_10
/* Nodes - Sounds */
#define POLARIS_MUSIC           NODE_11
#define POLARIS_NOISE_LO        NODE_12
#define POLARIS_NOISE_LO_FILT   NODE_13
#define POLARIS_NOISE_HI_FILT   NODE_14
#define POLARIS_SHOTSND         NODE_15
#define POLARIS_SHIP_HITSND     NODE_16
#define POLARIS_SHIPSND         NODE_17
#define POLARIS_EXPLOSIONSND    NODE_18
#define POLARIS_PLANESND        NODE_19
#define POLARIS_HITSND          NODE_20
#define POLARIS_SONARSND        NODE_21
/* Nodes - Adjust */
#define POLARIS_ADJ_VR1         NODE_23
#define POLARIS_ADJ_VR2         NODE_24
#define POLARIS_ADJ_VR3         NODE_25

DISCRETE_SOUND_START(polaris_discrete)

	/************************************************/
	/* Polaris sound system: 8 Sound Sources        */
	/*                                              */
	/* Relative volumes are adjustable              */
	/*                                              */
	/************************************************/

	/************************************************/
	/* Input register mapping for polaris           */
	/************************************************/
	DISCRETE_INPUT_DATA (POLARIS_MUSIC_DATA)
	DISCRETE_INPUT_LOGIC(POLARIS_SX0_EN)
	DISCRETE_INPUT_LOGIC(POLARIS_SX1_EN)
	DISCRETE_INPUT_LOGIC(POLARIS_SX2_EN)
	DISCRETE_INPUT_LOGIC(POLARIS_SX3_EN)
	DISCRETE_INPUT_LOGIC(POLARIS_SX5_EN)
	DISCRETE_INPUT_LOGIC(POLARIS_SX6_EN)
	DISCRETE_INPUT_LOGIC(POLARIS_SX7_EN)
	DISCRETE_INPUT_LOGIC(POLARIS_SX9_EN)
	DISCRETE_INPUT_LOGIC(POLARIS_SX10_EN)

	/* We will cheat and just use the controls to scale the amplitude. */
	/* It is the same as taking the (0 to 50k)/50k */
	DISCRETE_ADJUSTMENT(POLARIS_ADJ_VR1, 0, 1, DISC_LINADJ, "VR1")
	DISCRETE_ADJUSTMENT(POLARIS_ADJ_VR2, 0, 1, DISC_LINADJ, "VR2")
	/* Extra cheating for VR3.  We will include the resistor scaling. */
	DISCRETE_ADJUSTMENT(POLARIS_ADJ_VR3, 0, 0.5376, DISC_LINADJ, "VR3")

/******************************************************************************
 *
 * Music Generator
 *
 * This is a simulation of the following circuit:
 * 555 Timer (Ra = 1k, Rb = 1k, C =.01uF) running at 48kHz.  Connected to a
 * 1 bit counter (/2) for 24kHz.  But I will use the frequency measured by Guru.
 * This is then sent to a preloadable 8 bit counter (4G & 4H), which loads the
 * value from Port 2 when overflowing from 0xFF to 0x00.  Therefore it divides
 * by 2 (Port 2 = FE) to 256 (Port 2 = 00).
 * This goes to a 2 bit counter (5H) which has a 47k resistor on Q0 and a 12k on Q1.
 * This creates a sawtooth ramp of: 0%, 12/59%, 47/59%, 100% then back to 0%
 *
 * Note that there is no music disable line.  When there is no music, the game
 * sets the oscillator to 0Hz.  (Port 2 = FF)
 *
 ******************************************************************************/
	DISCRETE_NOTE(NODE_30, 1, 23396, POLARIS_MUSIC_DATA, 255, 3, DISC_CLK_IS_FREQ)
	DISCRETE_DAC_R1(NODE_31, NODE_30, DEFAULT_TTL_V_LOGIC_1, &polaris_music_dac)
	DISCRETE_OP_AMP_FILTER(NODE_32, 1, NODE_31, 0, DISC_OP_AMP_FILTER_IS_HIGH_PASS_0 | DISC_OP_AMP_IS_NORTON, &polaris_music_op_amp_filt_info)
	DISCRETE_MULTIPLY(POLARIS_MUSIC, NODE_32, POLARIS_ADJ_VR3)

/******************************************************************************
 *
 * Background Sonar Sound
 *
 * This is a background sonar that plays at all times during the game.
 * It is a VCO triangle wave genarator, that uses the Low frequency filtered
 * noise source to modulate the frequency.
 * This is then amplitude modulated, by some fancy clocking scheme.
 * It is disabled during SX3.  (No sonar when you die.)
 *
 * 5L pin 6 divides 60Hz by 16.  This clocks the sonar.
 * 5K pin 9 is inverted by 5F, and then the 0.1uF;1M;270k;1S1588 diode circuit
 * makes a one shot pulse of approx. 15ms to keep the noise working.
 *
 ******************************************************************************/
	DISCRETE_SQUAREWFIX(NODE_40, 1, 60.0/16, 1, 50, 1.0/2, 0)   // IC 5L, pin 6
	DISCRETE_COUNTER(NODE_41, 1, 0, NODE_40, 0, 31, 1, 0, DISC_CLK_ON_F_EDGE)   // IC 5L & 5F
	DISCRETE_TRANSFORM2(NODE_42, NODE_41, 4, "01&")         // IC 5L, pin 9
	DISCRETE_TRANSFORM2(NODE_43, NODE_41, 16, "01&!")       // IC 5F, pin 8
	DISCRETE_ONESHOT(NODE_44, NODE_43, 1, 0.0015, DISC_ONESHOT_REDGE | DISC_ONESHOT_NORETRIG | DISC_OUT_ACTIVE_HIGH)

	DISCRETE_LOGIC_OR(NODE_45, NODE_42, POLARIS_SX3_EN)
	DISCRETE_LOGIC_DFLIPFLOP(NODE_46, 1, 1, NODE_40, NODE_45)

	DISCRETE_OP_AMP_VCO1(NODE_47, 1, POLARIS_NOISE_LO_FILT, &polaris_sonar_vco_info)
	DISCRETE_OP_AMP_TRIG_VCA(POLARIS_SONARSND, NODE_45, NODE_46, 0, NODE_47, 0, &polaris_sonar_tvca_info)

/******************************************************************************
 *
 * Noise sources
 *
 * The frequencies for the noise sources were Measured by Guru.
 *
 * The output of the shift register is buffered by an op-amp which limits
 * the output to 0V and (12V - OP_AMP_NORTON_VBE)
 *
 ******************************************************************************/
	DISCRETE_LFSR_NOISE(POLARIS_NOISE_LO, 1, 1, 800.8, (12.0 - OP_AMP_NORTON_VBE), NODE_44, (12.0 - OP_AMP_NORTON_VBE)/2, &polaris_lfsr)  // Unfiltered Lo noise. 7K pin 4.
	// Filtered Lo noise.  7K pin 5.
	DISCRETE_OP_AMP_FILTER(POLARIS_NOISE_LO_FILT, 1, POLARIS_NOISE_LO, 0, DISC_OP_AMP_FILTER_IS_BAND_PASS_0 | DISC_OP_AMP_IS_NORTON, &polaris_nol_op_amp_filt_info)

	DISCRETE_LFSR_NOISE(NODE_50, 1, 1, 23396, (12.0 - OP_AMP_NORTON_VBE), NODE_44, (12.0 - OP_AMP_NORTON_VBE)/2, &polaris_lfsr) // 7K pin 10
	// Filtered Hi noise.  6B pin 10. - This does not really do much.  Sample rates of 98k+ are needed for this high of filtering.
	DISCRETE_OP_AMP_FILTER(POLARIS_NOISE_HI_FILT, 1, NODE_50, 0, DISC_OP_AMP_FILTER_IS_BAND_PASS_0 | DISC_OP_AMP_IS_NORTON, &polaris_noh_op_amp_filt_info)

/******************************************************************************
 *
 * Shot - SX0 (When player shoots)
 *
 * When Enabled it makes a low frequency RC filtered noise.  As soon as it
 * disables, it switches to a high frequency RC filtered noise with the volume
 * decaying based on the RC values of 680k and 1uF.
 *
 ******************************************************************************/
	DISCRETE_OP_AMP_TRIG_VCA(POLARIS_SHOTSND, POLARIS_SX0_EN, 0, 0, POLARIS_NOISE_HI_FILT, POLARIS_NOISE_LO_FILT, &polaris_shot_tvca_info)

/******************************************************************************
 *
 * Ship Hit - SX1 (When sub is hit)
 *
 * When Enabled it makes a low frequency RC filtered noise.  As soon as it
 * disables, it's  volume starts decaying based on the RC values of 680k and
 * 1uF.  Also as it decays, a decaying high frequency RC filtered noise is
 * mixed in.
 *
 ******************************************************************************/
	DISCRETE_OP_AMP_TRIG_VCA(POLARIS_SHIP_HITSND, POLARIS_SX1_EN, 0, 0, POLARIS_NOISE_HI_FILT, POLARIS_NOISE_LO_FILT, &polaris_shiphit_tvca_info)

/******************************************************************************
 *
 * Ship - SX2 (When boat moves across screen)
 *
 * This uses a 5.75Hz |\|\ sawtooth to modulate the frequency of a
 * voltage controlled triangle wave oscillator. *
 *
 ******************************************************************************/
	DISCRETE_OP_AMP_OSCILLATOR(NODE_60, 1, &polaris_boat_mod_info)
	DISCRETE_OP_AMP_VCO1(POLARIS_SHIPSND, POLARIS_SX2_EN, NODE_60, &polaris_boat_vco_info)

/******************************************************************************
 *
 * Explosion - SX3 (When player, boat or boss plane is hit)
 *
 * When Enabled it makes a low frequency noise.  As soon as it disables, it's
 * volume starts decaying based on the RC values of 680k and 0.33uF.  The
 * final output is RC filtered.
 *
 * Note that when this is triggered, the sonar sound clock is disabled.
 *
 ******************************************************************************/
	DISCRETE_OP_AMP_TRIG_VCA(NODE_70, POLARIS_SX3_EN, 0, 0, POLARIS_NOISE_LO, 0, &polaris_exp_tvca_info)

	DISCRETE_RCFILTER(NODE_71, NODE_70, 560.0, CAP_U(.22))
	DISCRETE_RCFILTER(POLARIS_EXPLOSIONSND, NODE_71, RES_K(6.8), CAP_U(.22))

/******************************************************************************
 *
 * Plane Down - SX6
 * Plane Up   - SX7
 *
 * The oscillator is enabled when SX7 goes high. When SX6 is enabled the
 * frequency lowers.  When SX6 is disabled the frequency ramps back up.
 * Also some NOISE_HI_FILT is mixed in so the frequency varies some.
 *
 ******************************************************************************/
	DISCRETE_INTEGRATE(NODE_80, POLARIS_SX6_EN, POLARIS_SX7_EN, &polaris_plane_integrate_info)
	DISCRETE_OP_AMP_VCO2(POLARIS_PLANESND, POLARIS_SX7_EN, NODE_80, POLARIS_NOISE_HI_FILT, &polaris_plane_vco_info)

/******************************************************************************
 *
 * HIT - SX9 & SX10
 *
 * Following the schematic, 3 different sounds are produced.
 * SX10  SX9  Effect
 *  0     0   no sound
 *  0     1   NOISE_HI_FILT while enabled
 *  1     0   NOISE_LO_FILT while enabled (When a regular plane is hit)
 *  1     1   NOISE_HI_FILT & NOISE_LO_FILT decaying immediately @ 680k, 0.22uF
 *
 ******************************************************************************/
	DISCRETE_OP_AMP_TRIG_VCA(POLARIS_HITSND, POLARIS_SX10_EN, POLARIS_SX9_EN, 0, POLARIS_NOISE_LO_FILT, POLARIS_NOISE_HI_FILT, &polaris_hit_tvca_info)

/******************************************************************************
 *
 * Final Mixing and Output
 *
 ******************************************************************************/
	DISCRETE_MIXER4(NODE_90, 1, POLARIS_SHOTSND, POLARIS_SONARSND, POLARIS_HITSND, POLARIS_PLANESND, &polaris_mixer_vr1_desc)
	DISCRETE_MULTIPLY(NODE_91, NODE_90, POLARIS_ADJ_VR1)
	DISCRETE_MIXER2(NODE_92, 1, POLARIS_SHIPSND, POLARIS_SHIP_HITSND, &polaris_mixer_vr2_desc)
	DISCRETE_MULTIPLY(NODE_93, NODE_92, POLARIS_ADJ_VR2)
	DISCRETE_MIXER4(NODE_94, POLARIS_SX5_EN, NODE_91, POLARIS_EXPLOSIONSND, NODE_93, POLARIS_MUSIC, &polaris_mixer_vr4_desc)

	DISCRETE_OUTPUT(NODE_94, 1)

DISCRETE_SOUND_END

void _8080bw_state::polaris_sh_port_1_w(uint8_t data)
{
	m_discrete->write(POLARIS_MUSIC_DATA, data);
}

void _8080bw_state::polaris_sh_port_2_w(uint8_t data)
{
	/* 0x01 - SX0 - Shot */
	m_discrete->write(POLARIS_SX0_EN, data & 0x01);

	/* 0x02 - SX1 - Ship Hit (Sub) */
	m_discrete->write(POLARIS_SX1_EN, data & 0x02);

	/* 0x04 - SX2 - Ship */
	m_discrete->write(POLARIS_SX2_EN, data & 0x04);

	/* 0x08 - SX3 - Explosion */
	m_discrete->write(POLARIS_SX3_EN, data & 0x08);

	/* 0x10 - SX4 */

	/* 0x20 - SX5 - Sound Enable */
	m_discrete->write(POLARIS_SX5_EN, data & 0x20);
}

void _8080bw_state::polaris_sh_port_3_w(uint8_t data)
{
	machine().bookkeeping().coin_lockout_global_w(data & 0x04);  /* SX8 */

	m_flip_screen = BIT(data, 5) & BIT(ioport("IN2")->read(), 2); /* SX11 */

	/* 0x01 - SX6 - Plane Down */
	m_discrete->write(POLARIS_SX6_EN, data & 0x01);

	/* 0x02 - SX7 - Plane Up */
	m_discrete->write(POLARIS_SX7_EN, data & 0x02);

	/* 0x08 - SX9 - Hit */
	m_discrete->write(POLARIS_SX9_EN, data & 0x08);

	/* 0x10 - SX10 - Hit */
	m_discrete->write(POLARIS_SX10_EN, data & 0x10);
}



/*******************************************************/
/*                                                     */
/* Taito "Space Chaser"                                */
/*                                                     */
/* The SN76477 still needs to be routed to the         */
/* discrete system for filtering.                      */
/*******************************************************/

/*
 *  The dot sound is a square wave clocked by either the
 *  the 8V or 4V signals
 *
 *  The frequencies are (for the 8V signal):
 *
 *  19.968 MHz crystal / 2 (Qa of 74160 #10) -> 9.984MHz
 *                     / 2 (7474 #14) -> 4.992MHz
 *                     / 256+16 (74161 #5 and #8) -> 18352.94Hz
 *                     / 8 (8V) -> 2294.12 Hz
 *                     / 2 the final freq. is 2 toggles -> 1147.06Hz
 *
 *  for 4V, it's double at 2294.12Hz
 */
#define SCHASER_HSYNC   18352.94
#define SCHASER_4V      SCHASER_HSYNC /2 /4
#define SCHASER_8V      SCHASER_HSYNC /2 /8


/* Nodes - Inputs */
#define SCHASER_DOT_EN      NODE_01
#define SCHASER_DOT_SEL     NODE_02
#define SCHASER_EXP_STREAM  NODE_03
#define SCHASER_MUSIC_BIT   NODE_04
#define SCHASER_SND_EN      NODE_05
/* Nodes - Adjusters */
#define SCHASER_VR1         NODE_07
#define SCHASER_VR2         NODE_08
#define SCHASER_VR3         NODE_09
/* Nodes - Sounds */
#define SCHASER_DOT_SND     NODE_10
#define SCHASER_EXP_SND     NODE_11
#define SCHASER_MUSIC_SND   NODE_12

DISCRETE_SOUND_START(schaser_discrete)
	/************************************************/
	/* Input register mapping for schaser           */
	/************************************************/
	DISCRETE_INPUT_LOGIC  (SCHASER_DOT_EN)
	DISCRETE_INPUT_LOGIC  (SCHASER_DOT_SEL)
	// scale to 0-3.5V
	DISCRETE_INPUTX_STREAM(SCHASER_EXP_STREAM, 0, 1.0/14100,             2.323)
	DISCRETE_INPUTX_LOGIC (SCHASER_MUSIC_BIT,    DEFAULT_TTL_V_LOGIC_1,  0,      0.0)
	DISCRETE_INPUT_LOGIC  (SCHASER_SND_EN)

	/************************************************/
	/* Volume adjusters.                            */
	/* We will set them to adjust the realitive     */
	/* gains.                                       */
	/************************************************/
	DISCRETE_ADJUSTMENT(SCHASER_VR1, 0, RES_K(50)/(RES_K(50) + RES_K(470)), DISC_LINADJ, "VR1")
	DISCRETE_ADJUSTMENT(SCHASER_VR2, 0, RES_K(50)/(RES_K(50) + 560 + RES_K(6.8) + RES_K(2)), DISC_LINADJ, "VR2")
	DISCRETE_ADJUSTMENT(SCHASER_VR3, 0, RES_K(50)/(RES_K(50) + 560 + RES_K(6.8) + RES_K(10)), DISC_LINADJ, "VR3")

	/************************************************/
	/* Dot selection just selects between 4V and 8V */
	/************************************************/
	DISCRETE_SQUAREWFIX(NODE_20, 1, SCHASER_4V, DEFAULT_TTL_V_LOGIC_1, 50, 0, 0)
	DISCRETE_SQUAREWFIX(NODE_21, 1, SCHASER_8V, DEFAULT_TTL_V_LOGIC_1, 50, 0, 0)
	DISCRETE_SWITCH(NODE_22, SCHASER_DOT_EN, SCHASER_DOT_SEL, NODE_20, NODE_21)
	DISCRETE_RCFILTER(NODE_23, NODE_22, 560, CAP_U(.1))
	DISCRETE_RCFILTER(NODE_24, NODE_23, RES_K(6.8) + 560, CAP_U(.1))
	DISCRETE_MULTIPLY(SCHASER_DOT_SND, NODE_24, SCHASER_VR3)

	/************************************************/
	/* Explosion/Effect filtering                   */
	/************************************************/
	DISCRETE_RCFILTER(NODE_30, SCHASER_EXP_STREAM, 560, CAP_U(.1))
	DISCRETE_RCFILTER(NODE_31, NODE_30, RES_K(6.8) + 560, CAP_U(.1))
	DISCRETE_CRFILTER(NODE_32, NODE_31, RES_K(6.8) + 560 + RES_K(2) + RES_K(50), CAP_U(1))
	DISCRETE_MULTIPLY(SCHASER_EXP_SND, NODE_32, SCHASER_VR2)

	/************************************************/
	/* Music is just a 1 bit DAC                    */
	/************************************************/
	DISCRETE_CRFILTER(NODE_40, SCHASER_MUSIC_BIT, RES_K(470) + RES_K(50), CAP_U(.01))
	DISCRETE_MULTIPLY(SCHASER_MUSIC_SND, NODE_40, SCHASER_VR1)

	/************************************************/
	/* Final mix with gain                          */
	/************************************************/
	DISCRETE_ADDER3(NODE_90, SCHASER_SND_EN, SCHASER_DOT_SND, SCHASER_EXP_SND, SCHASER_MUSIC_SND)

	DISCRETE_OUTPUT(NODE_90, 33080)
DISCRETE_SOUND_END

static const double schaser_effect_rc[8] =
{
	0,
	(RES_K(15) + RES_K(20)) * CAP_U(1),
	(RES_K(39) + RES_K(20)) * CAP_U(1),
	(1.0/ (1.0/RES_K(15) + 1.0/RES_K(39)) + RES_K(20)) * CAP_U(1),
	(RES_K(82) + RES_K(20)) * CAP_U(1),
	(1.0/ (1.0/RES_K(15) + 1.0/RES_K(82)) + RES_K(20)) * CAP_U(1),
	(1.0/ (1.0/RES_K(15) + 1.0/RES_K(82)) + RES_K(20)) * CAP_U(1),
	(1.0/ (1.0/RES_K(15) + 1.0/RES_K(39) + 1.0/RES_K(82)) + RES_K(20)) * CAP_U(1)
};

void _8080bw_state::schaser_sh_port_1_w(uint8_t data)
{
	int effect;

	/* bit 0 - Dot Sound Enable (SX0)
	   bit 1 - Dot Sound Pitch (SX1)
	   bit 2 - Effect Sound A (SX2)
	   bit 3 - Effect Sound B (SX3)
	   bit 4 - Effect Sound C (SX4)
	   bit 5 - Explosion (SX5)

	    Note that the schematic has SX2 and SX4 the wrong way around.
	    See MT 2662 for video proof. */

	m_discrete->write(SCHASER_DOT_EN, data & 0x01);
	m_discrete->write(SCHASER_DOT_SEL, data & 0x02);

	/* The effect is a variable rate 555 timer.  A diode/resistor array is used to
	 * select the frequency.  Because of the diode voltage drop, we can not use the
	 * standard 555 time formulas.  Also, when effect=0, the charge resistor
	 * is disconnected.  This causes the charge on the cap to slowly bleed off, but
	 * but the bleed time is so long, that we can just cheat and put the time on hold
	 * when effect = 0. */
	effect = (data >> 2) & 0x07;
	if (m_schaser_last_effect != effect)
	{
		if (effect)
		{
			if (m_schaser_effect_555_time_remain != attotime::zero)
			{
				/* timer re-enabled, use up remaining 555 high time */
				m_schaser_effect_555_timer->adjust(m_schaser_effect_555_time_remain, effect);
			}
			else if (!m_schaser_effect_555_is_low)
			{
				/* set 555 high time */
				attotime new_time = attotime(0, ATTOSECONDS_PER_SECOND * .8873 * schaser_effect_rc[effect]);
				m_schaser_effect_555_timer->adjust(new_time, effect);
			}
		}
		else
		{
			/* disable effect - stops at end of low cycle */
			if (!m_schaser_effect_555_is_low)
			{
				m_schaser_effect_555_time_remain = m_schaser_effect_555_timer->time_left();
				m_schaser_effect_555_time_remain_savable = m_schaser_effect_555_time_remain.as_double();
				m_schaser_effect_555_timer->adjust(attotime::never);
			}
		}
		m_schaser_last_effect = effect;
	}

	m_schaser_explosion = BIT(data, 5);
	if (m_schaser_explosion)
	{
		m_sn->amplitude_res_w(1.0 / (1.0/RES_K(200) + 1.0/RES_K(68)));
	}
	else
	{
		m_sn->amplitude_res_w(RES_K(200));
	}
	m_sn->enable_w(!(m_schaser_effect_555_is_low || m_schaser_explosion));
	m_sn->one_shot_cap_voltage_w(!(m_schaser_effect_555_is_low || m_schaser_explosion) ? 0 : sn76477_device::EXTERNAL_VOLTAGE_DISCONNECT);
	m_sn->mixer_b_w(m_schaser_explosion);
}

void _8080bw_state::schaser_sh_port_2_w(uint8_t data)
{
	/* bit 0 - Music (DAC) (SX6)
	   bit 1 - Sound Enable (SX7)
	   bit 2 - Coin Lockout (SX8)
	   bit 3 - Field Control A (SX9)
	   bit 4 - Field Control B (SX10)
	   bit 5 - Flip Screen */

	m_discrete->write(SCHASER_MUSIC_BIT, BIT(data, 0));

	m_discrete->write(SCHASER_SND_EN, BIT(data, 1));
	machine().sound().system_mute(!BIT(data, 1));

	machine().bookkeeping().coin_lockout_global_w(BIT(data, 2));

	m_schaser_background_disable = BIT(data, 3);
	m_schaser_background_select = BIT(data, 4);

	m_flip_screen = BIT(data, 5) & BIT(ioport("IN2")->read(), 6);

	m_port_2_last_extra = data;
}


TIMER_DEVICE_CALLBACK_MEMBER(_8080bw_state::schaser_effect_555_cb)
{
	int effect = param;
	attotime new_time;

	/* Toggle 555 output */
	m_schaser_effect_555_is_low = !m_schaser_effect_555_is_low;
	m_schaser_effect_555_time_remain = attotime::zero;
	m_schaser_effect_555_time_remain_savable = m_schaser_effect_555_time_remain.as_double();

	if (m_schaser_effect_555_is_low)
		new_time = PERIOD_OF_555_ASTABLE(0, RES_K(20), CAP_U(1)) / 2;
	else
	{
		if (effect)
			new_time = attotime(0, ATTOSECONDS_PER_SECOND * .8873 * schaser_effect_rc[effect]);
		else
			new_time = attotime::never;
	}
	m_schaser_effect_555_timer->adjust(new_time, effect);
	m_sn->enable_w(!(m_schaser_effect_555_is_low || m_schaser_explosion));
	m_sn->one_shot_cap_voltage_w(!(m_schaser_effect_555_is_low || m_schaser_explosion) ? 0 : sn76477_device::EXTERNAL_VOLTAGE_DISCONNECT);
}


void _8080bw_state::schaser_reinit_555_time_remain()
{
	m_schaser_effect_555_time_remain = attotime::from_double(m_schaser_effect_555_time_remain_savable);
	schaser_sh_port_2_w(m_port_2_last_extra);
}


MACHINE_START_MEMBER(_8080bw_state,schaser_sh)
{
	save_item(NAME(m_schaser_explosion));
	save_item(NAME(m_schaser_effect_555_is_low));
	save_item(NAME(m_schaser_effect_555_time_remain_savable));
	save_item(NAME(m_port_2_last_extra));
	machine().save().register_postload(save_prepost_delegate(FUNC(_8080bw_state::schaser_reinit_555_time_remain), this));
}


MACHINE_RESET_MEMBER(_8080bw_state,schaser_sh)
{
	m_schaser_effect_555_is_low = 0;
	m_schaser_effect_555_timer->adjust(attotime::never);
	schaser_sh_port_1_w(0);
	schaser_sh_port_2_w(0);
	m_schaser_effect_555_time_remain = attotime::zero;
	m_schaser_effect_555_time_remain_savable = m_schaser_effect_555_time_remain.as_double();
}



/*******************************************************/
/*                                                     */
/* Zenitone Microsec "Invaders Revenge"                */
/*                                                     */
/*******************************************************/

void invrvnge_state::port03_w(uint8_t data)
{
	m_sound_data = data;
}

void invrvnge_state::port05_w(uint8_t data)
{
	/*
	    00 - normal play
	    0A, 0E - alternate during play/attract at about once per second (invrvngegw only) purpose unknown
	    01 - briefly at boot time
	    10 - different colour map (or screen red) when you die
	    20 - flip screen */

	m_screen_red = BIT(data, 4);
	m_flip_screen = BIT(data, 5) & ioport(CABINET_PORT_TAG)->read();

	// no sound-related writes?
}

// The timer frequency controls the speed of the sounds
TIMER_DEVICE_CALLBACK_MEMBER(invrvnge_state::nmi_timer)
{
	m_timer_state ^= 1;
	m_audiocpu->set_input_line(INPUT_LINE_NMI, m_timer_state ? ASSERT_LINE : CLEAR_LINE );
}


/****************************************************/
/* Rolling Crash / Moon Base                        */
/* - Moon Base uses same ports and bits as invaders */
/* - Press Left or Right to choose game to play     */
/****************************************************/

void rollingc_state::rollingc_sh_port_w(uint8_t data)
{
	uint8_t rising_bits = data & ~m_port_3_last;

	if (BIT(rising_bits, 1)) m_samples->start(4, 0); // Steering
	if (BIT(rising_bits, 2)) m_samples->start(0, 1); // Collision
	if (BIT(rising_bits, 4)) m_samples->start(1, 8); // Computer car is starting to move

	m_port_3_last = data;
}



/*****************************************/
/* Lupin III preliminary sound           */
/* Correct samples not available         */
/*****************************************/

const char *const lupin3_sample_names[] =
{
	"*lupin3",
	"cap",      // go to jail
	"bark",     // dog barking
	"walk1",    // walk, get money
	"walk2",    // walk, get money
	"warp",     // translocate, deposit money
	"extend",   // bonus man
	"kick",     // lands on top of building, wife kicks man
	nullptr
};

void  _8080bw_state::lupin3_00_w (uint8_t data)
{
	m_discrete->write(INDIANBT_MUSIC_DATA, data);
}

void _8080bw_state::lupin3_sh_port_1_w(uint8_t data)
{
	uint8_t rising_bits = data & ~m_port_1_last_extra;
	static uint8_t lupin3_step = 2;

	if (BIT(rising_bits, 0))
	{
		m_samples->start(0, lupin3_step);               // Walking, steal money
		lupin3_step ^= 1;
	}

	m_sn->enable_w(BIT(~data, 1));                      // Helicopter

	if (BIT(rising_bits, 2)) m_samples->start(1, 4);     // Translocate
	if (BIT(rising_bits, 3)) m_samples->start(0, 0);     // Jail
	if (BIT(rising_bits, 4)) m_samples->start(2, 5);     // Bonus Man

	//machine().sound().system_mute(!BIT(data, 5));

	//machine().bookkeeping().coin_lockout_global_w(BIT(data, 7));

	m_port_1_last_extra = data;
}

void _8080bw_state::lupin3_sh_port_2_w(uint8_t data)
{
	uint8_t rising_bits = data & ~m_port_2_last_extra;

	if (rising_bits & 0x01) m_samples->start(0, 6);     /* Lands on top of building, wife kicks man */
	//if (rising_bits & 0x02) m_samples->start(3, 7);       /* deposit money, start intermission, end game */
	//if (rising_bits & 0x04) m_samples->start(4, 7);       /* deposit money, start intermission, Slides down rope */
	//if (rising_bits & 0x08) m_samples->start(5, 7);       /* start intermission, end game */
	if (rising_bits & 0x10) m_samples->start(3, 1);        /* Dog barking */

	m_color_map = data & 0x40;

	m_flip_screen = BIT(data, 5) & BIT(ioport("IN2")->read(), 2);

	m_port_2_last_extra = data;
}



/*****************************************/
/* Space Chaser (CV) preliminary sound   */
/* Much more work needs to be done       */
/*****************************************/

void _8080bw_state::schasercv_sh_port_1_w(uint8_t data)
{
	/* bit 2 = 2nd speedup
	   bit 3 = 1st speedup
	   Death is a stream of ff's with some fe's thrown in */

	uint8_t rising_bits = data & ~m_port_1_last_extra;

	if (BIT(rising_bits, 1)) m_samples->start(1, 6);    // Ran over a dot
	if (BIT(rising_bits, 4)) m_samples->start(0, 1);    // Death

	m_port_1_last_extra = data;
}

void _8080bw_state::schasercv_sh_port_2_w(uint8_t data)
{
	m_speaker->level_w(BIT(data, 0));      // End-of-Level

	machine().sound().system_mute(!BIT(data, 4));

	m_flip_screen = BIT(data, 5) & ioport(CABINET_PORT_TAG)->read();
}



/*****************************************/
/* Crash Road preliminary sound          */
/* Much more work needs to be done       */
/*****************************************/

void _8080bw_state::crashrd_port03_w(uint8_t data)
{
	/* bit 0 - Dot Sound Pitch (SX1)
	   bit 2 - Explosion (SX5)
	   bit 4 - Dot Sound Enable (SX0)
	   bit 5 - Effect Sound C (SX4) */

	m_discrete->write(SCHASER_SND_EN, BIT(data, 5));
	machine().sound().system_mute(!BIT(data, 5));
	m_discrete->write(SCHASER_DOT_EN, BIT(data, 4));
	m_discrete->write(SCHASER_DOT_SEL, BIT(data, 0));
	m_schaser_explosion = BIT(data, 2);

	if (m_schaser_explosion)
		m_sn->amplitude_res_w(1.0 / (1.0/RES_K(200) + 1.0/RES_K(68)));
	else
		m_sn->amplitude_res_w(RES_K(200));

	m_sn->enable_w(!m_schaser_explosion);
	m_sn->one_shot_cap_voltage_w(!m_schaser_explosion ? 0 : sn76477_device::EXTERNAL_VOLTAGE_DISCONNECT);
	m_sn->mixer_b_w(m_schaser_explosion);
}

void _8080bw_state::crashrd_port05_w(uint8_t data)
{
	// bit 0 = bitstream audio
	// bit 4 = indicates when enemy goes faster toward end of level
	// bit 5 = pulses once when flipscreen needed
	m_discrete->write(SCHASER_MUSIC_BIT, BIT(data, 0));
}



/*******************************************************************/
/* Yosakdon preliminary sound                                      */
/* No information available as what the correct sounds are         */
/*******************************************************************/

void yosakdon_state::sh_port_1_w(uint8_t data)
{
	uint8_t rising_bits = data & ~m_port_1_last;

	if (BIT(rising_bits, 0)) m_samples->start(0, 3);        //  Game Over
	if (BIT(rising_bits, 1)) m_samples->start(2, 0);        //  Bird dead
	if (BIT(rising_bits, 2)) m_samples->start(0, 1);        //  Rifle being fired
	if (BIT(rising_bits, 3)) m_samples->start(1, 2);        //  Man dead
	if (BIT(rising_bits, 4)) m_samples->start(5, 8);        //  Bonus Man?

	machine().sound().system_mute(!BIT(data, 5));

	m_port_1_last = data;
}

void yosakdon_state::sh_port_2_w(uint8_t data)
{
	uint8_t rising_bits = data & ~m_port_2_last;

	if (BIT(rising_bits, 0)) m_samples->start(1, 6);        // Ready? , Game Over
	if (BIT(rising_bits, 2)) m_samples->start(3, 7);        // Big bird dead

	m_sn->enable_w(BIT(~data, 3));                          // Big bird

	if (BIT(rising_bits, 4)) m_samples->start(2, 7);        // Game Over

	m_flip_screen = BIT(data, 5) & ioport(CABINET_PORT_TAG)->read();

	m_port_2_last = data;
}



/*****************************************/
/* shuttlei preliminary sound            */
/* Proper samples are unavailable        */
/*****************************************/

void shuttlei_state::sh_port_1_w(uint8_t data)
{
	/* bit 3 is high while you are alive and playing */
	uint8_t rising_bits = data & ~m_port_1_last;

	if (rising_bits & 0x01) m_samples->start(4, 4);         /* Fleet move */
	if (rising_bits & 0x02) m_samples->start(5, 8);         /* Extra Tank */

	m_sn->enable_w(data & 0x04 ? 0:1);              /* UFO */

	m_port_1_last = data;
}

void shuttlei_state::sh_port_2_w(uint8_t data)
{
	switch (data)
	{
		case 0x23:
			m_samples->start(2, 2);             /* Hit */
			break;

		case 0x2b:
			m_samples->start(0, 0);             /* Shoot */
			break;

		case 0xa3:
			m_samples->start(3, 7);             /* Hit UFO */
			break;

		case 0xab:
			m_samples->start(1, 1);             /* Death */
			break;
	}
}


/*****************************************/
/* "Darth Vader" preliminary sound       */
/* Proper samples are unavailable        */
/*****************************************/

void darthvdr_state::darthvdr_00_w(uint8_t data)
{
	m_flip_screen = BIT(data, 0) & ioport(CABINET_PORT_TAG)->read();
}

void darthvdr_state::darthvdr_08_w(uint8_t data)
{
	uint8_t rising_bits = data & ~m_port_1_last;

	machine().sound().system_mute(!BIT(data, 0));

	if (BIT(rising_bits, 1)) m_samples->start(0, 0);    // Shoot
	if (BIT(rising_bits, 2)) m_samples->start(3, 7);    // Hit UFO
	if (BIT(rising_bits, 4)) m_samples->start(5, 8);    // Bonus

	m_sn->enable_w(BIT(~data, 5));                      // UFO

	if (BIT(rising_bits, 6)) m_samples->start(1, 1);    // Death
	if (BIT(rising_bits, 7)) m_samples->start(2, 2);    // Hit

	if (BIT(rising_bits, 3))
	{
		m_samples->start(4, m_fleet_step);          // Fleet move in 4 steps
		m_fleet_step++;
		if (m_fleet_step > 6) m_fleet_step = 3;
	}

	m_port_1_last = data;
}


/*********************************************************/
/*                                                       */
/* Model Racing "Cane" (Slightly based on Claybuster hw) */
/*                                                       */
/*********************************************************/
#define CANE_CLOCK   (19968000.0)
#define CANE_H64     CANE_CLOCK /2 /2 /4

/* Nodes - Sound enable */
#define CANE_SND_EN        NODE_05

/* Nodes - Adjusters */
#define CANE_VR1           NODE_07  // Gain for 76477
#define CANE_VR2           NODE_08  // VR attached to the output of the TOS
#define CANE_VR3           NODE_09  // VR for SFX generated by the 555

/* Nodes - sn76477 Sounds */
#define CANE_EXP_STREAM    NODE_03
#define CANE_EXP_SND       NODE_11

/* Nodes - BGM */
#define CANE_MUSIC_DATA    NODE_06
#define CANE_MUSIC_NOTE    NODE_40
#define CANE_MUSIC_NOTE_PF NODE_01
#define CANE_MUSIC_SND     NODE_12

/* Nodes - 555 sfx */
#define CANE_76477_PIN6    NODE_13
#define CANE_555_CLAMPED   NODE_14
#define CANE_555_ONESHOT   NODE_15
#define CANE_555_EN        NODE_16
#define CANE_TMP_SND       NODE_17
#define CANE_SFX_SND       NODE_18

/* Node output */
#define CANE_SOUND_OUT     NODE_90

static INPUT_PORTS_START( cane_audio )
	PORT_START("VR1")
	PORT_ADJUSTER( 80, "VR1 - SFX from 76477" )

	PORT_START("VR2")
	PORT_ADJUSTER( 90, "VR2 - TOS music" )

	PORT_START("VR3")
	PORT_ADJUSTER( 70, "VR3 - Shoot SFX from 555" )
INPUT_PORTS_END

cane_audio_device::cane_audio_device(machine_config const &mconfig, char const *tag, device_t *owner, u32 clock) :
	device_t(mconfig, CANE_AUDIO, tag, owner, clock),
	m_vco_timer(*this, "vco_timer"),
	m_sn(*this, "snsnd"),
	m_discrete(*this, "discrete"),
	m_vco_rc_chargetime(attotime::never)
{
}

void cane_audio_device::device_add_mconfig(machine_config &config)
{
	TIMER(config, m_vco_timer).configure_generic(FUNC(cane_audio_device::vco_voltage_timer));

	SPEAKER(config, "mono").front_center();

	SN76477(config, m_sn);
	// Amplitude res in the schematic is connected to a 470K potentiometer, so from the schematic is impossible to know the real res.
	// This parameter drives the amp just before the audio output pin 13.
	m_sn->set_amp_res(100+RES_K(20));
	m_sn->set_noise_params(RES_K(39), RES_K(1), CAP_P(1000));
	m_sn->set_decay_res(RES_M(1));
	m_sn->set_attack_params(CAP_U(1.0), RES_K(47));
	m_sn->set_feedback_res(RES_K(4.7));
	m_sn->set_vco_params(0, CAP_P(3300), RES_K(100));
	m_sn->set_pitch_voltage(5.0);
	m_sn->set_slf_params(CAP_U(1.0), RES_K(33));
	m_sn->set_oneshot_params(CAP_U(10), RES_K(100));
	m_sn->set_vco_mode(0);
	m_sn->set_mixer_params(0, 0, 0);
	m_sn->set_envelope_params(1, 0);
	m_sn->set_enable(0);
	m_sn->add_route(0, "discrete", 1.0, 0);

	DISCRETE(config, m_discrete, cane_discrete);
	m_discrete->add_route(ALL_OUTPUTS, "mono", 1.0);
}

ioport_constructor cane_audio_device::device_input_ports() const
{
	return INPUT_PORTS_NAME(cane_audio);
}

void cane_audio_device::device_start()
{
	// provare a commentare
	m_vco_rc_chargetime = attotime::never;

	save_item(NAME(m_vco_rc_chargetime));
}

void cane_audio_device::sh_port_1_w(u8 data)
{
	/*
	    bit 0 - SX0 - Sound enable on mixer
	    bit 1 - SX1 - SN76477 - Mixer select C - pin 27
	    bit 2 - SX2 - SN76477 - Mixer select A - pin 26
	    bit 3 - SX3 - SN76477 - Mixer select B - pin 25
	    bit 4 - SX4 - NE555 - Trigger (Step, high output level for 1.1*RC = 1.1*100K*0.47u = 51.7 ms)
	*/

	m_discrete->write(CANE_SND_EN, data & 0x01); // BIT(data, 0) - bit 0 - SX0 - Sound enable on mixer
	m_discrete->write(CANE_555_EN, data & 0x10); // BIT(data, 4) - bit 4 - SX4 - NE555 - Trigger

	// 76477 enable bit is connected to the select line of the out port 3 (inverted).
	m_sn->enable_w(1);
	m_sn->set_mixer_params(BIT(data, 2), BIT(data, 3), BIT(data, 1));

	m_vco_timer->adjust(attotime::zero, m_vco_timer->param(), attotime::from_hz(1000));
	m_vco_rc_chargetime = m_vco_timer->start_time();

	// Little hack...
	// To be precise I should enable the 76477 every time the CPU reads or write to a port different from port 3
	// and disable it every time the CPU read/write from/to port 3.
	// Actually this can not be done easily so I decided to enable it preemptively here after every port 3 access
	m_sn->enable_w(0);
}

void cane_audio_device::music_w(u8 data)
{
	m_sn->enable_w(1);
	m_discrete->write(CANE_MUSIC_DATA, data);
}

void cane_audio_device::sn76477_en_w(u8 data)
{
	m_sn->enable_w(0);
}

void cane_audio_device::sn76477_dis_w(u8 data)
{
	m_sn->enable_w(1);
}

/*******************************************************************************************************************************************************/
/* Cane discrete implementation, slightly based on Claybuster hw.                                                                                      */
/* This implementation doesn't pretend to be accurate thus trying to be at least functionally similar.                                                 */
/*                                                                                                                                                     */
/*                                                                       Port 1 - SX4              CANE_VR3                       Port 1 - SX0         */
/*                                                                            |                        |                               |               */
/*                                                                       CANE_555_EN                   |                               |               */
/*                                                                            |                        |                               |               */
/*                                                                  +------------------+               |                               |               */
/*                                                                  | CANE_555_ONESHOT |               |                               |               */
/*                                                                  +------------------+               |                               |               */
/*                                                                            |                        |                               |               */
/*                                                                            v                        v                               |               */
/*                                    sn76477 pin 6   +------------------+  +---+  +--------------+  +---+  +--------------+           |               */
/*                                     CLAMP(0, 5V) ->| CANE_555_CLAMPED |->| * |->| CANE_TMP_SND |->| * |->| CANE_SFX_SND |       CANE_SND_EN         */
/*                                 (DISCRETE_NOISE)   +------------------+  +---+  +--------------+  +---+  +--------------+           |               */
/*                                                                                                                       |             |               */
/*                                                                                          CANE_VR1                     |             |               */
/*                                                                                              |                        |             |               */
/*                                                                                              v                        v             v               */
/*                                                      sn76477 output   +-----------------+  +---+  +--------------+  +---+  +----------------+       */
/*                                              DISCRETE_INPUTX_STREAM ->| CANE_EXP_STREAM |->| * |->| CANE_EXP_SND |->| + |->| CANE_SOUND_OUT |->OUT  */
/*                                                                       +-----------------+  +---+  +--------------+  +---+  +----------------+       */
/*                                                                                                                       ^                             */
/*                                                                                                                       |                             */
/*                   CANE_MUSIC_DATA   +--------------------+  +-----------+  +-----------------+  +---+  +----------------+                           */
/* Port 5 data---DISCRETE_INPUT_DATA ->| CANE_MUSIC_NOTE_PF |->| CR_FILTER |->| CANE_MUSIC_NOTE |->| * |->| CANE_MUSIC_SND |                           */
/*                     DISCRETE_NOTE   +--------------------+  +-----------+  +-----------------+  +---+  +----------------+                           */
/*                                                                RES_K(10)                          ^                                                 */
/*                                                               CAP_U(0.1)                          |                                                 */
/*                                                                                               CANE_VR2                                              */
/*                                                                                                                                                     */
/*                                                                                                                                                     */
/*******************************************************************************************************************************************************/
DISCRETE_SOUND_START(cane_discrete)
	/************************************************/
	/* Input register mapping for cane           */
	/************************************************/
	DISCRETE_INPUT_DATA (CANE_MUSIC_DATA)

	DISCRETE_INPUT_LOGIC  (CANE_555_EN)

	// scale to 0-2.5V
	DISCRETE_INPUTX_STREAM(CANE_EXP_STREAM, 0, 0.5, 0)

	DISCRETE_INPUT_LOGIC  (CANE_SND_EN)

	/************************************************/
	/* Volume adjusters.                            */
	/* We will set them to adjust the realitive     */
	/* gains.                                       */
	/************************************************/
	DISCRETE_ADJUSTMENT(CANE_VR1, 0, 0.33*6, DISC_LINADJ, "VR1")      // Gain for 76477
	DISCRETE_ADJUSTMENT(CANE_VR2, 0, 0.33*60000, DISC_LINADJ, "VR2")  // VR attached to the output of the TOS
	DISCRETE_ADJUSTMENT(CANE_VR3, 0, 0.33*60000, DISC_LINADJ, "VR3")  // VR for SFX generated by the 555

	/************************************************/
	/* From 555                                     */
	/************************************************/
	/* TODO: find real noise freq and amplitude */
	/* width was simulated with ltspice using Claybuster schematic as a source and it's value is about 51ms */
	DISCRETE_NOISE(CANE_76477_PIN6,
				1,                          /* ENAB */
				1280,                       /* FREQ - Guessed */
				1,                          /* AMP  */
				0)                          /* BIAS - fake AC is fine*/
	DISCRETE_CLAMP(CANE_555_CLAMPED,
				CANE_76477_PIN6,            /* input node */
				0.0,                        /* minimum */
				5.0)                        /* maximum */
	DISCRETE_ONESHOT(CANE_555_ONESHOT,
					 CANE_555_EN,           /* trigger node */
					 1,                     /* amplitude node or static value */
					 0.05,                  /* width (in seconds) node or static value - 50 ms*/
					 DISC_ONESHOT_FEDGE | DISC_ONESHOT_RETRIG)  /* type of oneshot static value */

	DISCRETE_MULTIPLY(CANE_TMP_SND, CANE_555_CLAMPED, CANE_555_ONESHOT)
	DISCRETE_MULTIPLY(CANE_SFX_SND, CANE_TMP_SND, CANE_VR3)

/*****************************************************************************
*
* Music Generator (TOS)
*
* Values for this section of the sound hardware where derived from comments
* in the source code and the analysis of TOS.ED sources.
*
* For further info look at the relevant comments reported into
* drivers/8080bw.cpp
*
******************************************************************************/
	DISCRETE_NOTE(CANE_MUSIC_NOTE_PF, 1, CANE_H64, CANE_MUSIC_DATA, 255, 1, DISC_CLK_IS_FREQ)
	DISCRETE_CRFILTER(CANE_MUSIC_NOTE, CANE_MUSIC_NOTE_PF, RES_K(10), CAP_U(0.1))  // high pass filter
	DISCRETE_MULTIPLY(CANE_MUSIC_SND, CANE_MUSIC_NOTE, CANE_VR2)

/******************************************************************************
*
* From 76477 output
*
******************************************************************************/
	DISCRETE_MULTIPLY(CANE_EXP_SND, CANE_EXP_STREAM, CANE_VR1)

/******************************************************************************
*
* Final Mixing and Output
*
******************************************************************************/
	DISCRETE_ADDER3(CANE_SOUND_OUT, CANE_SND_EN, CANE_SFX_SND, CANE_EXP_SND, CANE_MUSIC_SND)
	DISCRETE_OUTPUT(CANE_SOUND_OUT, 1)

//LOG
/*
    DISCRETE_WAVLOG1(CANE_EXP_STREAM, 1)
    DISCRETE_WAVLOG1(CANE_EXP_SND, 1)
    DISCRETE_WAVLOG1(CANE_TMP_SND, 1)
    DISCRETE_WAVLOG1(CANE_SFX_SND, 1)
    DISCRETE_WAVLOG1(CANE_MUSIC_NOTE, 1)
    DISCRETE_WAVLOG1(CANE_MUSIC_SND, 1)
    DISCRETE_WAVLOG1(CANE_SOUND_OUT, 1)
*/
DISCRETE_SOUND_END

TIMER_DEVICE_CALLBACK_MEMBER(cane_audio_device::vco_voltage_timer)
{
	const double delta = (m_vco_timer->fire_time() - m_vco_rc_chargetime).as_double();
	const double voltage = 5 * (1 - std::exp(-delta / 47));

	LOG("t = %d\n", delta);
	LOG("vco_voltage = %d\n", voltage);

	m_sn->vco_voltage_w(voltage);
}
