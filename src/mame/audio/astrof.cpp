// license:BSD-3-Clause
// copyright-holders:Lee Taylor
/***************************************************************************

    Astro Fighter hardware

****************************************************************************/

#include "emu.h"
#include "includes/astrof.h"


/*************************************
 *
 *  Astro Fighter
 *
 *************************************/

#define SAMPLE_FIRE      0
#define SAMPLE_EKILLED   1
#define SAMPLE_WAVE      2
#define SAMPLE_BOSSFIRE  6
#define SAMPLE_FUEL      7
#define SAMPLE_DEATH     8
#define SAMPLE_BOSSHIT   9
#define SAMPLE_BOSSKILL  10

#define CHANNEL_FIRE      0
#define CHANNEL_EXPLOSION 1
#define CHANNEL_WAVE      2   /* background humm */
#define CHANNEL_BOSSFIRE  2   /* there is no background humm on the boss level */
#define CHANNEL_FUEL      3


WRITE8_MEMBER(astrof_state::astrof_audio_1_w)
{
	UINT8 rising_bits = data & ~m_port_1_last;

	if (m_astrof_death_playing)
		m_astrof_death_playing = m_samples->playing(CHANNEL_EXPLOSION);

	if (m_astrof_bosskill_playing)
		m_astrof_bosskill_playing = m_samples->playing(CHANNEL_EXPLOSION);

	/* D2 - explosion */
	if (rising_bits & 0x04)
	{
		/* I *know* that the effect select port will be written shortly
		   after this one, so this works */
		m_astrof_start_explosion = 1;
	}

	/* D0,D1,D3 - background noise */
	if ((data & 0x08) && (~m_port_1_last & 0x08))
	{
		int sample = SAMPLE_WAVE + (data & 3);
		m_samples->start(CHANNEL_WAVE, sample, 1);
	}

	if ((~data & 0x08) && (m_port_1_last & 0x08))
		m_samples->stop(CHANNEL_WAVE);

	/* D4 - boss laser */
	if ((rising_bits & 0x10) && !m_astrof_bosskill_playing)
		m_samples->start(CHANNEL_BOSSFIRE, SAMPLE_BOSSFIRE, 0);

	/* D5 - fire */
	if ((rising_bits & 0x20) && !m_astrof_bosskill_playing)
		m_samples->start(CHANNEL_FIRE, SAMPLE_FIRE, 0);

	/* D6 - don't know. Probably something to do with the explosion sounds */

	/* D7 - sound enable bit */
	machine().sound().system_enable(data & 0x80);

	m_port_1_last = data;
}


WRITE8_MEMBER(astrof_state::astrof_audio_2_w)
{
	UINT8 rising_bits = data & ~m_port_2_last;

	/* D0-D2 - explosion select (triggered by D2 of the other port */
	if (m_astrof_start_explosion)
	{
		/* this is really a compound effect, made up of I believe 3 sound
		   effects, but since our sample contains them all, disable playing
		   the other effects while the explosion is playing */

		logerror("Explosion: %x\n", data);
		if (data & 0x04)
		{
			if (!m_astrof_bosskill_playing)
			{
				m_samples->start(CHANNEL_EXPLOSION, SAMPLE_BOSSKILL, 0);
				m_astrof_bosskill_playing = 1;
			}
		}
		else if (data & 0x02)
			m_samples->start(CHANNEL_EXPLOSION, SAMPLE_BOSSHIT, 0);
		else if (data & 0x01)
			m_samples->start(CHANNEL_EXPLOSION, SAMPLE_EKILLED, 0);
		else
		{
			if (!m_astrof_death_playing)
			{
				m_samples->start(CHANNEL_EXPLOSION, SAMPLE_DEATH, 0);
				m_astrof_death_playing = 1;
			}
		}

		m_astrof_start_explosion = 0;
	}

	/* D3 - low fuel warning */
	if (rising_bits & 0x08)
		m_samples->start(CHANNEL_FUEL, SAMPLE_FUEL, 0);

	m_port_2_last = data;
}


static const char *const astrof_sample_names[] =
{
	"*astrof",
	"fire",
	"ekilled",
	"wave1",
	"wave2",
	"wave3",
	"wave4",
	"bossfire",
	"fuel",
	"death",
	"bosshit",
	"bosskill",
	nullptr
};

MACHINE_CONFIG_FRAGMENT( astrof_audio )
	MCFG_SPEAKER_STANDARD_MONO("mono")
	MCFG_SOUND_ADD("samples", SAMPLES, 0)
	MCFG_SAMPLES_CHANNELS(4)
	MCFG_SAMPLES_NAMES(astrof_sample_names)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.50)
MACHINE_CONFIG_END



/*************************************
 *
 *  Space Fighter
 *
 *************************************/

WRITE8_MEMBER(astrof_state::spfghmk2_audio_w)
{
	/* nothing yet */
}


MACHINE_CONFIG_FRAGMENT( spfghmk2_audio )
	/* nothing yet */
MACHINE_CONFIG_END



/*************************************
 *
 *  Tomahawk
 *
 *************************************/

WRITE8_MEMBER(astrof_state::tomahawk_audio_w)
{
	/* D0 - sonar */

	/* D1 - UFO explosion */

	/* D2 - morse */

	/* D3 - missile */

	/* D4 - UFO */

	/* D5 - UFO under water */
	m_sn->enable_w((~data >> 5) & 0x01);

	/* D6 - explosion */

	/* D7 - sound enable bit */
	machine().sound().system_enable(data & 0x80);
}


MACHINE_CONFIG_FRAGMENT( tomahawk_audio )
	MCFG_SPEAKER_STANDARD_MONO("mono")
	MCFG_SOUND_ADD("snsnd", SN76477, 0)
	MCFG_SN76477_NOISE_PARAMS(0, 0, 0)                   // noise + filter: N/C
	MCFG_SN76477_DECAY_RES(0)                            // decay_res N/C
	MCFG_SN76477_ATTACK_PARAMS(0, 0)                     // attack_decay_cap + attack_res: N/C
	MCFG_SN76477_AMP_RES(RES_K(47))                      // amplitude_res
	MCFG_SN76477_FEEDBACK_RES(RES_K(47))                 // feedback_res
	MCFG_SN76477_VCO_PARAMS(0, CAP_U(0.033), RES_K(33))  // VCO volt + cap + res
	MCFG_SN76477_PITCH_VOLTAGE(5.0)                      // pitch_voltage
	MCFG_SN76477_SLF_PARAMS(CAP_U(2.2), RES_K(47))       // slf caps + res
	MCFG_SN76477_ONESHOT_PARAMS(0, 0)                    // oneshot caps + res: N/C
	MCFG_SN76477_VCO_MODE(1)                             // VCO mode
	MCFG_SN76477_MIXER_PARAMS(0, 0, 0)                   // mixer A, B, C
	MCFG_SN76477_ENVELOPE_PARAMS(0, 0)                   // envelope 1, 2
	MCFG_SN76477_ENABLE(1)                               // enable
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 1.0)
MACHINE_CONFIG_END
