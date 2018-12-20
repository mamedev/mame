// license:BSD-3-Clause
// copyright-holders:Zsolt Vasvari
/*
 *  Invinco sound routines
 */

#include "emu.h"
#include "includes/vicdual.h"

/* output port 0x02 definitions - sound effect drive outputs */
#define OUT_PORT_2_SAUCER       0x04
#define OUT_PORT_2_MOVE1        0x08
#define OUT_PORT_2_MOVE2        0x10
#define OUT_PORT_2_FIRE         0x20
#define OUT_PORT_2_INVHIT       0x40
#define OUT_PORT_2_SHIPHIT      0x80


#define PLAY(samp,id,loop)      samp->start( id, id, loop )
#define STOP(samp,id)           samp->stop( id )


/* sample file names */
static const char *const invinco_sample_names[] =
{
	"*invinco",
	"saucer",
	"move1",
	"move2",
	"fire",
	"invhit",
	"shiphit",
	"move3",    /* currently not used */
	"move4",    /* currently not used */
	nullptr
};


/* sample IDs - must match sample file name table above */
enum
{
	SND_SAUCER = 0,
	SND_MOVE1,
	SND_MOVE2,
	SND_FIRE,
	SND_INVHIT,
	SND_SHIPHIT,
	SND_MOVE3,
	SND_MOVE4
};


WRITE8_MEMBER( vicdual_state::invinco_audio_w )
{
	int bitsChanged;
	//int bitsGoneHigh;
	int bitsGoneLow;

	bitsChanged  = m_port2State ^ data;
	//bitsGoneHigh = bitsChanged & data;
	bitsGoneLow  = bitsChanged & ~data;

	m_port2State = data;

	if ( bitsGoneLow & OUT_PORT_2_SAUCER )
	{
		PLAY( m_samples, SND_SAUCER, 0 );
	}

	if ( bitsGoneLow & OUT_PORT_2_MOVE1 )
	{
		PLAY( m_samples, SND_MOVE1, 0 );
	}

	if ( bitsGoneLow & OUT_PORT_2_MOVE2 )
	{
		PLAY( m_samples, SND_MOVE2, 0 );
	}

	if ( bitsGoneLow & OUT_PORT_2_FIRE )
	{
		PLAY( m_samples, SND_FIRE, 0 );
	}

	if ( bitsGoneLow & OUT_PORT_2_INVHIT )
	{
		PLAY( m_samples, SND_INVHIT, 0 );
	}

	if ( bitsGoneLow & OUT_PORT_2_SHIPHIT )
	{
		PLAY( m_samples, SND_SHIPHIT, 0 );
	}
}


void vicdual_state::invinco_audio(machine_config &config)
{
	/* samples */
	SAMPLES(config, m_samples);
	m_samples->set_channels(8);
	m_samples->set_samples_names(invinco_sample_names);
	m_samples->add_route(ALL_OUTPUTS, "mono", 0.5);
}
