/*********************************************************************************

This is for common pinball machine coding.

**********************************************************************************/



#include "genpin.h"

MACHINE_CONFIG_FRAGMENT( genpin_audio )
	MCFG_SPEAKER_STANDARD_MONO("mechmono")
	MCFG_SAMPLES_ADD("samples", genpin_samples_intf)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mechmono", 1.0)
MACHINE_CONFIG_END
