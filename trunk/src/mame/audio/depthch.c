/*
 *  Depth Charge audio routines
 */

#include "emu.h"
#include "sound/samples.h"
#include "includes/vicdual.h"

/* output port 0x01 definitions - sound effect drive outputs */
#define OUT_PORT_1_LONGEXPL     0x01
#define OUT_PORT_1_SHRTEXPL     0x02
#define OUT_PORT_1_SPRAY        0x04
#define OUT_PORT_1_SONAR        0x08


#define PLAY(samp,id,loop)      samp->start( id, id, loop )
#define STOP(samp,id)           samp->stop( id )


/* sample file names */
static const char *const depthch_sample_names[] =
{
	"*depthch",
	"longex",
	"shortex",
	"spray",
	"sonar",
	0
};


static const samples_interface depthch_samples_interface =
{
	4,
	depthch_sample_names
};


MACHINE_CONFIG_FRAGMENT( depthch_audio )
	MCFG_SAMPLES_ADD("samples", depthch_samples_interface)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.5)
MACHINE_CONFIG_END


/* sample IDs - must match sample file name table above */
enum
{
	SND_LONGEXPL = 0,
	SND_SHRTEXPL,
	SND_SPRAY,
	SND_SONAR,
};


WRITE8_HANDLER( depthch_audio_w )
{
	static int port1State = 0;
	samples_device *samples = space->machine().device<samples_device>("samples");
	int bitsChanged;
	int bitsGoneHigh;
	int bitsGoneLow;


	bitsChanged  = port1State ^ data;
	bitsGoneHigh = bitsChanged & data;
	bitsGoneLow  = bitsChanged & ~data;

	port1State = data;

	if ( bitsGoneHigh & OUT_PORT_1_LONGEXPL )
	{
		PLAY( samples, SND_LONGEXPL, 0 );
	}

	if ( bitsGoneHigh & OUT_PORT_1_SHRTEXPL )
	{
		PLAY( samples, SND_SHRTEXPL, 0 );
	}

	if ( bitsGoneHigh & OUT_PORT_1_SPRAY )
	{
		PLAY( samples, SND_SPRAY, 0 );
	}

	if ( bitsGoneHigh & OUT_PORT_1_SONAR )
	{
		PLAY( samples, SND_SONAR, 1 );
	}
	if ( bitsGoneLow & OUT_PORT_1_SONAR )
	{
		STOP( samples, SND_SONAR );
	}
}
