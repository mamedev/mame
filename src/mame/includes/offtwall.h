/*************************************************************************

    Atari "Round" hardware

*************************************************************************/

#include "machine/atarigen.h"

class offtwall_state
{
public:
	static void *alloc(running_machine &machine) { return auto_alloc_clear(&machine, offtwall_state(machine)); }

	offtwall_state(running_machine &machine) { }

	atarigen_state	atarigen;
};


/*----------- defined in video/offtwall.c -----------*/

VIDEO_START( offtwall );
VIDEO_UPDATE( offtwall );
