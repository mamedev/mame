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


MACHINE_CONFIG_EXTERN( genpin_audio );


#endif /* GENPIN_H_ */