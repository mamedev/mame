// license:BSD-3-Clause
// copyright-holders:Zsolt Vasvari
/*
 *  Pulsar sound routines
 *
 *  TODO: change heart rate based on bit 7 of Port 1
 *
 */

#include "emu.h"
#include "includes/vicdual.h"


/* output port 0x01 definitions - sound effect drive outputs */
#define OUT_PORT_1_CLANG        0x01
#define OUT_PORT_1_KEY          0x02
#define OUT_PORT_1_ALIENHIT     0x04
#define OUT_PORT_1_PHIT         0x08
#define OUT_PORT_1_ASHOOT       0x10
#define OUT_PORT_1_PSHOOT       0x20
#define OUT_PORT_1_BONUS        0x40
#define OUT_PORT_1_HBEAT_RATE   0x80    /* currently not used */

/* output port 0x02 definitions - sound effect drive outputs */
#define OUT_PORT_2_SIZZLE       0x01
#define OUT_PORT_2_GATE         0x02
#define OUT_PORT_2_BIRTH        0x04
#define OUT_PORT_2_HBEAT        0x08
#define OUT_PORT_2_MOVMAZE      0x10


#define PLAY(samp,id,loop)           samp->start( id, id, loop )
#define STOP(samp,id)                samp->stop( id )


/* sample file names */
static const char *const pulsar_sample_names[] =
{
	"*pulsar",
	"clang",
	"key",
	"alienhit",
	"phit",
	"ashoot",
	"pshoot",
	"bonus",
	"sizzle",
	"gate",
	"birth",
	"hbeat",
	"movmaze",
	0
};


MACHINE_CONFIG_FRAGMENT( pulsar_audio )
	MCFG_SOUND_ADD("samples", SAMPLES, 0)
	MCFG_SAMPLES_CHANNELS(12)
	MCFG_SAMPLES_NAMES(pulsar_sample_names)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.5)
MACHINE_CONFIG_END


/* sample IDs - must match sample file name table above */
enum
{
	SND_CLANG = 0,
	SND_KEY,
	SND_ALIENHIT,
	SND_PHIT,
	SND_ASHOOT,
	SND_PSHOOT,
	SND_BONUS,
	SND_SIZZLE,
	SND_GATE,
	SND_BIRTH,
	SND_HBEAT,
	SND_MOVMAZE
};


WRITE8_MEMBER( vicdual_state::pulsar_audio_1_w )
{
	int bitsChanged;
	//int bitsGoneHigh;
	int bitsGoneLow;


	bitsChanged  = m_port1State ^ data;
	//bitsGoneHigh = bitsChanged & data;
	bitsGoneLow  = bitsChanged & ~data;

	m_port1State = data;

	if ( bitsGoneLow & OUT_PORT_1_CLANG )
	{
		PLAY( m_samples, SND_CLANG, 0 );
	}

	if ( bitsGoneLow & OUT_PORT_1_KEY )
	{
		PLAY( m_samples, SND_KEY, 0 );
	}

	if ( bitsGoneLow & OUT_PORT_1_ALIENHIT )
	{
		PLAY( m_samples, SND_ALIENHIT, 0 );
	}

	if ( bitsGoneLow & OUT_PORT_1_PHIT )
	{
		PLAY( m_samples, SND_PHIT, 0 );
	}

	if ( bitsGoneLow & OUT_PORT_1_ASHOOT )
	{
		PLAY( m_samples, SND_ASHOOT, 0 );
	}

	if ( bitsGoneLow & OUT_PORT_1_PSHOOT )
	{
		PLAY( m_samples, SND_PSHOOT, 0 );
	}

	if ( bitsGoneLow & OUT_PORT_1_BONUS )
	{
		PLAY( m_samples, SND_BONUS, 0 );
	}
}


WRITE8_MEMBER( vicdual_state::pulsar_audio_2_w )
{
	static int port2State = 0;
	int bitsChanged;
	int bitsGoneHigh;
	int bitsGoneLow;


	bitsChanged  = port2State ^ data;
	bitsGoneHigh = bitsChanged & data;
	bitsGoneLow  = bitsChanged & ~data;

	port2State = data;

	if ( bitsGoneLow & OUT_PORT_2_SIZZLE )
	{
		PLAY( m_samples, SND_SIZZLE, 0 );
	}

	if ( bitsGoneLow & OUT_PORT_2_GATE )
	{
		m_samples->start(SND_CLANG, SND_GATE);
	}
	if ( bitsGoneHigh & OUT_PORT_2_GATE )
	{
		STOP( m_samples, SND_CLANG );
	}

	if ( bitsGoneLow & OUT_PORT_2_BIRTH )
	{
		PLAY( m_samples, SND_BIRTH, 0 );
	}

	if ( bitsGoneLow & OUT_PORT_2_HBEAT )
	{
		PLAY( m_samples, SND_HBEAT, 1 );
	}
	if ( bitsGoneHigh & OUT_PORT_2_HBEAT )
	{
		STOP( m_samples, SND_HBEAT );
	}

	if ( bitsGoneLow & OUT_PORT_2_MOVMAZE )
	{
		PLAY( m_samples, SND_MOVMAZE, 1 );
	}
	if ( bitsGoneHigh & OUT_PORT_2_MOVMAZE )
	{
		STOP( m_samples, SND_MOVMAZE );
	}
}
