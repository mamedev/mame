/*************************************************************************

    Atari Shuuz hardware

*************************************************************************/

#include "machine/atarigen.h"

class shuuz_state
{
public:
	static void *alloc(running_machine &machine) { return auto_alloc_clear(&machine, shuuz_state(machine)); }

	shuuz_state(running_machine &machine) { }

	atarigen_state	atarigen;
};


/*----------- defined in video/shuuz.c -----------*/

VIDEO_START( shuuz );
VIDEO_UPDATE( shuuz );
