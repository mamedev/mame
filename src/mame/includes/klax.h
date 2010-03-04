/*************************************************************************

    Atari Klax hardware

*************************************************************************/

#include "machine/atarigen.h"

class klax_state
{
public:
	static void *alloc(running_machine &machine) { return auto_alloc_clear(&machine, klax_state(machine)); }

	klax_state(running_machine &machine) { }

	atarigen_state			atarigen;
};


/*----------- defined in video/klax.c -----------*/

WRITE16_HANDLER( klax_latch_w );

VIDEO_START( klax );
VIDEO_UPDATE( klax );
