// license:BSD-3-Clause
// copyright-holders:Zsolt Vasvari
/*
 *  Tranquillizer Gun audio routines
 */

#include "emu.h"
#include "includes/vicdual.h"

/* output port 0x01 definitions - sound effect drive outputs */
#define OUT_PORT_1_STUFF1       0x01
#define OUT_PORT_1_STUFF2       0x02


#define PLAY(samp,id,loop)      samp->start( id, id, loop )
#define STOP(samp,id)           samp->stop( id )


/* sample file names */
static const char *const tranqgun_sample_names[] =
{
	"*tranqgun",
	"stuff1",
	"stuff2",
	nullptr
};


/* sample IDs - must match sample file name table above */
enum
{
	SND_STUFF1 = 0,
	SND_STUFF2
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

	if ( bitsGoneHigh & OUT_PORT_1_STUFF1 )
	{
		PLAY( m_samples, SND_STUFF1, 0 );
	}
	if ( bitsGoneLow & OUT_PORT_1_STUFF1 )
	{
		STOP( m_samples, SND_STUFF1 );
	}
}


MACHINE_CONFIG_START( tranqgun_audio )

	/* samples */
	MCFG_SOUND_ADD("samples", SAMPLES, 0)
	MCFG_SAMPLES_CHANNELS(2)
	MCFG_SAMPLES_NAMES(tranqgun_sample_names)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.5)
MACHINE_CONFIG_END
