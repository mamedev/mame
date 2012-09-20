#ifndef GENPIN_H_
#define GENPIN_H_


#include "emu.h"
#include "sound/samples.h"



static const char *const genpin_sample_names[] =
{
	"*genpin",
	"bumper",
	"chime1",
	"chime2",
	"chime3",
	"coinin",
	"hole",
	"knocker",
	0	/* end of array */
};

static const samples_interface genpin_samples_intf =
{
	4, // channels
	genpin_sample_names
};

MACHINE_CONFIG_FRAGMENT( genpin_audio )
	MCFG_SPEAKER_STANDARD_MONO("mono")
	MCFG_SAMPLES_ADD("samples", genpin_samples_intf)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 1.0)
MACHINE_CONFIG_END


#endif /* GENPIN_H_ */