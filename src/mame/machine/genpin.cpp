// license:BSD-3-Clause
// copyright-holders:Robbbert
/*********************************************************************************

This is for common pinball machine coding.

**********************************************************************************/

#include "emu.h"
#include "genpin.h"
#include "speaker.h"


MACHINE_CONFIG_START(genpin_class::genpin_audio)
	SPEAKER(config, "mechvol").front_center();
	MCFG_DEVICE_ADD("samples", SAMPLES)
	MCFG_SAMPLES_CHANNELS(6)
	MCFG_SAMPLES_NAMES(genpin_sample_names)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mechvol", 1.0)
MACHINE_CONFIG_END
