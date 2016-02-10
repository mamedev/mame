// license:BSD-3-Clause
// copyright-holders:Zsolt Vasvari
/*
 *  Depth Charge audio routines
 */

#include "emu.h"
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
	nullptr
};


/* sample IDs - must match sample file name table above */
enum
{
	SND_LONGEXPL = 0,
	SND_SHRTEXPL,
	SND_SPRAY,
	SND_SONAR
};


WRITE8_MEMBER( vicdual_state::depthch_audio_w )
{
	int bitsChanged;
	int bitsGoneHigh;
	int bitsGoneLow;

	bitsChanged  = m_port1State ^ data;
	bitsGoneHigh = bitsChanged & data;
	bitsGoneLow  = bitsChanged & ~data;

	m_port1State = data;

	if ( bitsGoneHigh & OUT_PORT_1_LONGEXPL )
	{
		PLAY( m_samples, SND_LONGEXPL, 0 );
	}

	if ( bitsGoneHigh & OUT_PORT_1_SHRTEXPL )
	{
		PLAY( m_samples, SND_SHRTEXPL, 0 );
	}

	if ( bitsGoneHigh & OUT_PORT_1_SPRAY )
	{
		PLAY( m_samples, SND_SPRAY, 0 );
	}

	if ( bitsGoneHigh & OUT_PORT_1_SONAR )
	{
		PLAY( m_samples, SND_SONAR, 1 );
	}
	if ( bitsGoneLow & OUT_PORT_1_SONAR )
	{
		STOP( m_samples, SND_SONAR );
	}
}


MACHINE_CONFIG_FRAGMENT( depthch_audio )

	/* samples */
	MCFG_SOUND_ADD("samples", SAMPLES, 0)
	MCFG_SAMPLES_CHANNELS(4)
	MCFG_SAMPLES_NAMES(depthch_sample_names)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.5)
MACHINE_CONFIG_END
