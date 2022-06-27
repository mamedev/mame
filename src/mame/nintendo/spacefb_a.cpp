// license:BSD-3-Clause
// copyright-holders:Chris Hardy
/***************************************************************************

    Space Firebird hardware

****************************************************************************/

#include "emu.h"
#include "spacefb.h"

#include "cpu/mcs48/mcs48.h"
#include "sound/dac.h"
#include "sound/samples.h"
#include "speaker.h"


uint8_t spacefb_state::audio_p2_r()
{
	return (m_sound_latch & 0x18) << 1;
}


READ_LINE_MEMBER(spacefb_state::audio_t0_r)
{
	return BIT(m_sound_latch, 5);
}


READ_LINE_MEMBER(spacefb_state::audio_t1_r)
{
	return BIT(m_sound_latch, 2);
}


void spacefb_state::port_1_w(uint8_t data)
{
	m_audiocpu->set_input_line(0, (data & 0x02) ? CLEAR_LINE : ASSERT_LINE);

	/* enemy killed */
	if (!(data & 0x01) && (m_sound_latch & 0x01))  m_samples->start(0,0);

	/* ship fire */
	if (!(data & 0x40) && (m_sound_latch & 0x40))  m_samples->start(1,1);

	/*
	 *  Explosion Noise
	 *
	 *  Actual sample has a bit of attack at the start, but these doesn't seem to be an easy way
	 *  to play the attack part, then loop the middle bit until the sample is turned off
	 *  Fortunately it seems like the recorded sample of the spaceship death is the longest the sample plays for.
	 *  We loop it just in case it runs out
	 */
	if ((data & 0x80) != (m_sound_latch & 0x80))
	{
		if (data & 0x80)
			/* play decaying noise */
			m_samples->start(2,3);
		else
			/* start looping noise */
			m_samples->start(2,2, true);
	}

	m_sound_latch = data;
}


static const char *const spacefb_sample_names[] =
{
	"*spacefb",
	"ekilled",
	"shipfire",
	"explode1",
	"explode2",
	nullptr
};


void spacefb_state::spacefb_audio(machine_config &config)
{
	SPEAKER(config, "speaker").front_center();
	DAC_8BIT_R2R(config, "dac", 0).add_route(ALL_OUTPUTS, "speaker", 0.25); // unknown DAC

	SAMPLES(config, m_samples);
	m_samples->set_channels(3);
	m_samples->set_samples_names(spacefb_sample_names);
	m_samples->add_route(ALL_OUTPUTS, "speaker", 1.0);
}
