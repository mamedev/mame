// license:BSD-3-Clause
// copyright-holders:Lee Taylor
/***************************************************************************

    Astro Fighter hardware

****************************************************************************/

#include "emu.h"
#include "astrof.h"
#include "speaker.h"


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


void astrof_state::astrof_audio_1_w(uint8_t data)
{
	uint8_t rising_bits = data & ~m_port_1_last;

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
	machine().sound().system_mute(!BIT(data, 7));

	m_port_1_last = data;
}


void astrof_state::astrof_audio_2_w(uint8_t data)
{
	uint8_t rising_bits = data & ~m_port_2_last;

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

void astrof_state::astrof_audio(machine_config &config)
{
	SPEAKER(config, "mono").front_center();
	SAMPLES(config, m_samples);
	m_samples->set_channels(4);
	m_samples->set_samples_names(astrof_sample_names);
	m_samples->add_route(ALL_OUTPUTS, "mono", 0.50);
}



/*************************************
 *
 *  Space Fighter
 *
 *************************************/

void astrof_state::spfghmk2_audio_w(uint8_t data)
{
	/* nothing yet */
}


void astrof_state::spfghmk2_audio(machine_config &config)
{
	/* nothing yet */
}



/*************************************
 *
 *  Tomahawk
 *
 *************************************/

void astrof_state::tomahawk_audio_w(uint8_t data)
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
	machine().sound().system_mute(!BIT(data, 7));
}


void astrof_state::tomahawk_audio(machine_config &config)
{
	SPEAKER(config, "mono").front_center();
	SN76477(config, m_sn);
	m_sn->set_noise_params(0, 0, 0);
	m_sn->set_decay_res(0);
	m_sn->set_attack_params(0, 0);
	m_sn->set_amp_res(RES_K(47));
	m_sn->set_feedback_res(RES_K(47));
	m_sn->set_vco_params(0, CAP_U(0.033), RES_K(33));
	m_sn->set_pitch_voltage(5.0);
	m_sn->set_slf_params(CAP_U(2.2), RES_K(47));
	m_sn->set_oneshot_params(0, 0);
	m_sn->set_vco_mode(1);
	m_sn->set_mixer_params(0, 0, 0);
	m_sn->set_envelope_params(0, 0);
	m_sn->set_enable(1);
	m_sn->add_route(ALL_OUTPUTS, "mono", 1.0);
}
