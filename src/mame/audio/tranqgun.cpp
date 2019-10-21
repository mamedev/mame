// license:BSD-3-Clause
// copyright-holders:Zsolt Vasvari, Jim Hernandez
/*
 *  Tranquillizer Gun audio routines
 */

#include "emu.h"
#include "includes/vicdual.h"

/* output port 0x01 definitions - sound effect drive outputs */
#define OUT_PORT_1_ANIMAL    0x01
#define OUT_PORT_1_CRY       0x02
#define OUT_PORT_1_WALK      0x04
#define OUT_PORT_1_EMAR      0x08
#define OUT_PORT_1_ANIMALHIT 0x10
#define OUT_PORT_1_POINT     0x20
#define OUT_PORT_1_JEEP      0x40
#define OUT_PORT_1_GUN       0x80


#define PLAY(samp,id,loop)      samp->start( id, id, loop )
#define STOP(samp,id)           samp->stop( id )


/* sample file names */
static const char *const tranqgun_sample_names[] =
{
	"*tranqgun",
	"cry",
	"walk",
	"emar",
	"animalhit",
	"point",
	"jeep",
	"gun",
	"animal",
	nullptr
};


/* sample IDs - must match sample file name table above */
enum
{
	SND_CRY = 0,
	SND_WALK,
	SND_EMAR,
	SND_ANIMALHIT,
	SND_POINT,
	SND_JEEP,
	SND_GUN,
	SND_ANIMAL,
};


WRITE8_MEMBER( vicdual_state::tranqgun_audio_w )
{
	int bitsChanged;
	int bitsGoneHigh;
	int bitsGoneLow;

	bitsChanged  = m_port1State ^ data;
	bitsGoneHigh = bitsChanged & data;
	bitsGoneLow  = bitsChanged & ~data;

	m_port1State = data;

	if ( bitsGoneHigh & OUT_PORT_1_ANIMAL )
	{
		PLAY( m_samples, SND_ANIMAL, 0 );
	}
	if ( bitsGoneLow & OUT_PORT_1_ANIMAL )
	{
		STOP( m_samples, SND_ANIMAL );
	}

	if ( bitsGoneHigh & OUT_PORT_1_CRY )
	{
		PLAY( m_samples, SND_CRY, 0 );
	}
	if ( bitsGoneLow & OUT_PORT_1_CRY )
	{
		STOP( m_samples, SND_CRY );
	}

	if ( bitsGoneHigh & OUT_PORT_1_WALK )
	{
		PLAY( m_samples, SND_WALK, 0 );
	}
	if ( bitsGoneLow & OUT_PORT_1_WALK )
	{
		STOP( m_samples, SND_WALK );
	}

	if ( bitsGoneLow & OUT_PORT_1_ANIMAL )
	{
		PLAY( m_samples, SND_ANIMAL,0 );
	}

	if ( bitsGoneHigh & OUT_PORT_1_ANIMALHIT )
	{
		PLAY( m_samples, SND_ANIMALHIT, 0 );
	}
	if ( bitsGoneLow & OUT_PORT_1_ANIMALHIT )
	{
		STOP( m_samples, SND_ANIMALHIT );
	}

	if ( bitsGoneHigh & OUT_PORT_1_POINT )
	{
		PLAY( m_samples, SND_POINT, 0 );
	}
	if ( bitsGoneLow & OUT_PORT_1_POINT )
	{
		STOP( m_samples, SND_POINT );
	}

	if ( bitsGoneLow & OUT_PORT_1_JEEP )
	{
		PLAY( m_samples, SND_JEEP, 1 );

	}
	if ( bitsGoneHigh & OUT_PORT_1_JEEP )
	{
		STOP( m_samples, SND_JEEP );
	}

	if ( bitsGoneLow & OUT_PORT_1_EMAR )
	{
		PLAY( m_samples, SND_EMAR, 0 );
	}

	if ( bitsGoneHigh & OUT_PORT_1_GUN )
	{
		PLAY( m_samples, SND_GUN, 0 );
	}
	if ( bitsGoneLow & OUT_PORT_1_GUN )
	{
		STOP( m_samples, SND_GUN );
	}
}


void vicdual_state::tranqgun_audio(machine_config &config)
{
	/* samples */
	SAMPLES(config, m_samples);
	m_samples->set_channels(8);
	m_samples->set_samples_names(tranqgun_sample_names);
	m_samples->add_route(ALL_OUTPUTS, "mono", 0.5);
}
