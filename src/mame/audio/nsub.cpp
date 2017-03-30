// license:BSD-3-Clause
// copyright-holders:Zsolt Vasvari, MASH
/*
 *  N-Sub (upright) sound routines
 */

#include "emu.h"
#include "includes/vicdual.h"

/* output port 0x02 definitions - sound effect drive outputs */
#define OUT_PORT_2_ALERT        0x01
#define OUT_PORT_2_SONAR        0x02
#define OUT_PORT_2_FIRE         0x04
#define OUT_PORT_2_EXPLOSION    0x08
#define OUT_PORT_2_EXPLOSION2   0x10
#define OUT_PORT_2_BONUS        0x20
#define OUT_PORT_2_SIGNAL       0x40
#define OUT_PORT_2_SIREN        0x80


#define PLAY(samp,id,loop)      samp->start( id, id, loop )
#define STOP(samp,id)           samp->stop( id )


/* sample file names */
static const char *const nsub_sample_names[] =
{
	"*nsub",
	"alert",
	"sonar",
	"fire",
	"explosion",
	"explosion2",
	"bonus",
	"signal",
	"siren",
	nullptr
};


/* sample IDs - must match sample file name table above */
enum
{
	SND_ALERT = 0,
	SND_SONAR,
	SND_FIRE,
	SND_EXPLOSION,
	SND_EXPLOSION2,
	SND_BONUS,
	SND_SIGNAL,
	SND_SIREN
};


WRITE8_MEMBER( vicdual_state::nsub_audio_w )
{
	int bitsChanged;
	//int bitsGoneHigh;
	int bitsGoneLow;

	bitsChanged  = m_port2State ^ data;
	//bitsGoneHigh = bitsChanged & data;
	bitsGoneLow  = bitsChanged & ~data;

	m_port2State = data;

	if ( bitsGoneLow & OUT_PORT_2_ALERT )
	{
		PLAY( m_samples, SND_ALERT, 0 );
	}

	if ( bitsGoneLow & OUT_PORT_2_SONAR )
	{
		PLAY( m_samples, SND_SONAR, 1 );
	}

	if ( bitsGoneLow & OUT_PORT_2_FIRE )
	{
		PLAY( m_samples, SND_FIRE, 0 );
	}

	if ( bitsGoneLow & OUT_PORT_2_EXPLOSION )
	{
		PLAY( m_samples, SND_EXPLOSION, 0 );
	}

	if ( bitsGoneLow & OUT_PORT_2_EXPLOSION2 )
	{
		PLAY( m_samples, SND_EXPLOSION2, 0 );
	}

	if ( bitsGoneLow & OUT_PORT_2_BONUS )
	{
		PLAY( m_samples, SND_BONUS, 0 );
	}

	if ( bitsGoneLow & OUT_PORT_2_SIGNAL )
	{
		PLAY( m_samples, SND_SIGNAL, 0 );
	}

	if ( bitsGoneLow & OUT_PORT_2_SIREN )
	{
		PLAY( m_samples, SND_SIREN, 0 );
	}
}


MACHINE_CONFIG_FRAGMENT( nsub_audio )

	/* samples */
	MCFG_SOUND_ADD("samples", SAMPLES, 0)
	MCFG_SAMPLES_CHANNELS(8)
	MCFG_SAMPLES_NAMES(nsub_sample_names)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.5)
MACHINE_CONFIG_END
