/***************************************************************************

    Astro Fighter hardware

****************************************************************************/

#include "emu.h"
#include "sound/samples.h"
#include "sound/sn76477.h"
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
	0
};


static const samples_interface astrof_samples_interface =
{
	4,  /* 4 channels */
	astrof_sample_names
};



MACHINE_CONFIG_FRAGMENT( astrof_audio )
	MCFG_SPEAKER_STANDARD_MONO("mono")
	MCFG_SAMPLES_ADD("samples", astrof_samples_interface)
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
	sn76477_enable_w(m_sn, (~data >> 5) & 0x01);

	/* D6 - explosion */

	/* D7 - sound enable bit */
	machine().sound().system_enable(data & 0x80);
}


static const sn76477_interface tomahawk_sn76477_interface =
{
	0,              /*  4 noise_res (N/C)        */
	0,              /*  5 filter_res (N/C)       */
	0,              /*  6 filter_cap (N/C)       */
	0,              /*  7 decay_res (N/C)        */
	0,              /*  8 attack_decay_cap (N/C) */
	0,              /* 10 attack_res (N/C)       */
	RES_K(47),      /* 11 amplitude_res          */
	RES_K(47),      /* 12 feedback_res           */
	0,              /* 16 vco_voltage (N/C)      */
	CAP_U(0.033),   /* 17 vco_cap                */
	RES_K(33),      /* 18 vco_res                */
	5.0,            /* 19 pitch_voltage          */
	RES_K(47),      /* 20 slf_res                */
	CAP_U(2.2),     /* 21 slf_cap                */
	0,              /* 23 oneshot_cap (N/C)      */
	0,              /* 24 oneshot_res (N/C)      */
	1,              /* 22 vco                    */
	0,              /* 26 mixer A                */
	0,              /* 25 mixer B                */
	0,              /* 27 mixer C                */
	0,              /* 1  envelope 1             */
	0,              /* 28 envelope 2             */
	1               /* 9  enable (variable)      */
};


MACHINE_CONFIG_FRAGMENT( tomahawk_audio )
	MCFG_SPEAKER_STANDARD_MONO("mono")
	MCFG_SOUND_ADD("snsnd", SN76477, 0)
	MCFG_SOUND_CONFIG(tomahawk_sn76477_interface)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 1.0)
MACHINE_CONFIG_END
