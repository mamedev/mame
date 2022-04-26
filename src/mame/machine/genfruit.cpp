// license:BSD-3-Clause
// copyright-holders:SomeRandomGuyIdk
/*********************************************************************************

Mechanical sounds for fruit machines, same concept as genpin.cpp

**********************************************************************************/

#include "emu.h"
#include "genfruit.h"
#include "speaker.h"

void genfruit_class::genfruit_audio(machine_config &config)
{
	SPEAKER(config, "mechvol").front_center();
	SAMPLES(config, m_samples);
	m_samples->set_channels(6);
	m_samples->set_samples_names(genfruit_sample_names);
	m_samples->add_route(ALL_OUTPUTS, "mechvol", 1.0);
}
