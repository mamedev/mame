// license:BSD-3-Clause
// copyright-holders:Aaron Giles
/*************************************************************************

    Targ hardware

*************************************************************************/

/* Sound channel usage
   0 = CPU music,  Shoot
   1 = Crash
   2 = Spectar sound
   3 = Tone generator
*/

#include "emu.h"
#include "includes/exidy.h"



#define SPECTAR_MAXFREQ     525000
#define TARG_MAXFREQ        125000

static const INT16 sine_wave[32] =
{
		0x0f0f,  0x0f0f,  0x0f0f,  0x0606,  0x0606,  0x0909,  0x0909,  0x0606,  0x0606,  0x0909,  0x0606,  0x0d0d,  0x0f0f,  0x0f0f,  0x0d0d,  0x0000,
	-0x191a, -0x2122, -0x1e1f, -0x191a, -0x1314, -0x191a, -0x1819, -0x1819, -0x1819, -0x1314, -0x1314, -0x1314, -0x1819, -0x1e1f, -0x1e1f, -0x1819
};


/* some macros to make detecting bit changes easier */
#define RISING_EDGE(bit)  ( (data & bit) && !(m_port_1_last & bit))
#define FALLING_EDGE(bit) (!(data & bit) &&  (m_port_1_last & bit))


void exidy_state::adjust_sample(UINT8 freq)
{
	m_tone_freq = freq;

	if (!m_samples->playing(3))
	{
		m_samples->set_volume(3, 0);
		m_samples->start_raw(3, sine_wave, 32, 1000, true);
	}

	if ((m_tone_freq == 0xff) || (m_tone_freq == 0x00))
		m_samples->set_volume(3, 0);
	else
	{
		m_samples->set_frequency(3, 1.0 * m_max_freq / (0xff - m_tone_freq));
		m_samples->set_volume(3, m_tone_active);
	}
}


WRITE8_MEMBER( exidy_state::targ_audio_1_w )
{
	/* CPU music */
	if ((data & 0x01) != (m_port_1_last & 0x01))
		m_dac->write_unsigned8((data & 0x01) * 0xff);

	/* shot */
	if (FALLING_EDGE(0x02) && !m_samples->playing(0))  m_samples->start(0,1);
	if (RISING_EDGE(0x02)) m_samples->stop(0);

	/* crash */
	if (RISING_EDGE(0x20))
	{
		if (data & 0x40)
			m_samples->start(1,2);
		else
			m_samples->start(1,0);
	}

	/* Sspec */
	if (data & 0x10)
		m_samples->stop(2);
	else
	{
		if ((data & 0x08) != (m_port_1_last & 0x08))
		{
			if (data & 0x08)
				m_samples->start(2,3,true);
			else
				m_samples->start(2,4,true);
		}
	}

	/* Game (tone generator enable) */
	if (FALLING_EDGE(0x80))
	{
		m_tone_pointer = 0;
		m_tone_active = 0;

		adjust_sample(m_tone_freq);
	}

	if (RISING_EDGE(0x80))
		m_tone_active=1;

	m_port_1_last = data;
}


WRITE8_MEMBER( exidy_state::targ_audio_2_w )
{
	if ((data & 0x01) && !(m_port_2_last & 0x01))
	{
		UINT8 *prom = memregion("targ")->base();

		m_tone_pointer = (m_tone_pointer + 1) & 0x0f;

		adjust_sample(prom[((data & 0x02) << 3) | m_tone_pointer]);
	}

	m_port_2_last = data;
}


WRITE8_MEMBER( exidy_state::spectar_audio_2_w )
{
	adjust_sample(data);
}


static const char *const sample_names[] =
{
	"*targ",
	"expl",
	"shot",
	"sexpl",
	"spslow",
	"spfast",
	0
};


void exidy_state::common_audio_start(int freq)
{
	m_max_freq = freq;

	m_tone_freq = 0;
	m_tone_active = 0;

	/* start_raw can't be called here: chan.source will be set by
	samples_device::device_start and then nulled out by samples_device::device_reset
	at the soft_reset stage of init_machine() and will never be set again.
	Thus, I've moved it to exidy_state::adjust_sample() were it will be set after
	machine initialization. */
	//m_samples->set_volume(3, 0);
	//m_samples->start_raw(3, sine_wave, 32, 1000, true);

	save_item(NAME(m_port_1_last));
	save_item(NAME(m_port_2_last));
	save_item(NAME(m_tone_freq));
	save_item(NAME(m_tone_active));
}


SAMPLES_START_CB_MEMBER(exidy_state::spectar_audio_start)
{
	common_audio_start(SPECTAR_MAXFREQ);
}


SAMPLES_START_CB_MEMBER(exidy_state::targ_audio_start)
{
	common_audio_start(TARG_MAXFREQ);

	m_tone_pointer = 0;

	save_item(NAME(m_tone_pointer));
}


MACHINE_CONFIG_FRAGMENT( spectar_audio )

	MCFG_SPEAKER_STANDARD_MONO("mono")

	MCFG_SOUND_ADD("samples", SAMPLES, 0)
	MCFG_SAMPLES_CHANNELS(4)
	MCFG_SAMPLES_NAMES(sample_names)
	MCFG_SAMPLES_START_CB(exidy_state, spectar_audio_start)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.25)

	MCFG_DAC_ADD("dac")
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 1.0)
MACHINE_CONFIG_END


MACHINE_CONFIG_FRAGMENT( targ_audio )

	MCFG_SPEAKER_STANDARD_MONO("mono")

	MCFG_SOUND_ADD("samples", SAMPLES, 0)
	MCFG_SAMPLES_CHANNELS(4)
	MCFG_SAMPLES_NAMES(sample_names)
	MCFG_SAMPLES_START_CB(exidy_state, targ_audio_start)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.25)

	MCFG_DAC_ADD("dac")
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 1.0)
MACHINE_CONFIG_END
