// license:BSD-3-Clause
// copyright-holders:Frank Palazzolo

#include "emu.h"
#include "includes/blockade.h"
#include "sound/discrete.h"
#include "sound/samples.h"

#define BLOCKADE_LOG 0

/*
 * This still needs the noise generator stuff,
 * along with proper mixing and volume control
 */

#define BLOCKADE_NOTE_DATA      NODE_01
#define BLOCKADE_NOTE           NODE_02

DISCRETE_SOUND_START(blockade)
	DISCRETE_INPUT_DATA  (BLOCKADE_NOTE_DATA)

	/************************************************/
	/* Note sound is created by a divider circuit.  */
	/* The master clock is the 93681.5 Hz, from the */
	/* 555 oscillator.  This is then sent to a      */
	/* preloadable 8 bit counter, which loads the   */
	/* value from OUT02 when overflowing from 0xFF  */
	/* to 0x00.  Therefore it divides by 2 (OUT02   */
	/* = FE) to 256 (OUT02 = 00).                   */
	/* There is also a final /2 stage.              */
	/* Note that there is no music disable line.    */
	/* When there is no music, the game sets the    */
	/* oscillator to 0Hz.  (OUT02 = FF)             */
	/************************************************/
	DISCRETE_NOTE(BLOCKADE_NOTE, 1, 93681.5, BLOCKADE_NOTE_DATA, 255, 1, DISC_CLK_IS_FREQ | DISC_OUT_IS_ENERGY)
	DISCRETE_CRFILTER(NODE_10, BLOCKADE_NOTE, RES_K(35), CAP_U(.01))

	DISCRETE_OUTPUT(NODE_10, 7500)
DISCRETE_SOUND_END

WRITE8_MEMBER(blockade_state::blockade_sound_freq_w)
{
	m_discrete->write(space,BLOCKADE_NOTE_DATA, data);
	return;
}

WRITE8_MEMBER(blockade_state::blockade_env_on_w)
{
	if (BLOCKADE_LOG) osd_printf_debug("Boom Start\n");
	m_samples->start(0,0);
	return;
}

WRITE8_MEMBER(blockade_state::blockade_env_off_w)
{
	if (BLOCKADE_LOG) osd_printf_debug("Boom End\n");
	return;
}

const char *const blockade_sample_names[] =
{
	"*blockade",
	"boom",
	0
};
