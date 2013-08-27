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
	0
};


static const samples_interface invinco_samples_interface =
{
	8,
	invinco_sample_names
};


MACHINE_CONFIG_FRAGMENT( invinco_audio )
	MCFG_SAMPLES_ADD("samples", invinco_samples_interface)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.5)
MACHINE_CONFIG_END


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
	static int port2State = 0;
	int bitsChanged;
	//int bitsGoneHigh;
	int bitsGoneLow;


	bitsChanged  = port2State ^ data;
	//bitsGoneHigh = bitsChanged & data;
	bitsGoneLow  = bitsChanged & ~data;

	port2State = data;

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

#if 0
	logerror("Went LO: %02X  %04X\n", bitsGoneLow, space.device().safe_pc());
#endif
}
