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
	"bonus",
	"sonar",
	nullptr
};


/* sample IDs - must match sample file name table above */
enum
{
	SND_LONGEXPL = 0,
	SND_SHRTEXPL,
	SND_SPRAY,
	SND_BONUS,
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

		// bonus sound on same line as sonar
		PLAY( m_samples, SND_BONUS, 0 );
	}
}


void vicdual_state::depthch_audio(machine_config &config)
{
	/* samples */
	SAMPLES(config, m_samples);
	m_samples->set_channels(5);
	m_samples->set_samples_names(depthch_sample_names);
	m_samples->add_route(ALL_OUTPUTS, "mono", 0.5);
}
