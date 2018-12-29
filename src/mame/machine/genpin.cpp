// license:BSD-3-Clause
// copyright-holders:Robbbert
/*********************************************************************************

This is for common pinball machine coding.

**********************************************************************************/

#include "emu.h"
#include "genpin.h"
#include "speaker.h"

void genpin_class::genpin_audio(machine_config &config)
{
	SPEAKER(config, "mechvol").front_center();
	SAMPLES(config, m_samples);
	m_samples->set_channels(6);
	m_samples->set_samples_names(genpin_sample_names);
	m_samples->add_route(ALL_OUTPUTS, "mechvol", 1.0);
}
