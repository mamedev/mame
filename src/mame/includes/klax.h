/*************************************************************************

    Atari Klax hardware

*************************************************************************/

#include "machine/atarigen.h"

typedef struct _klax_state klax_state;
struct _klax_state
{
	atarigen_state			atarigen;
};


/*----------- defined in video/klax.c -----------*/

WRITE16_HANDLER( klax_latch_w );

VIDEO_START( klax );
VIDEO_UPDATE( klax );
