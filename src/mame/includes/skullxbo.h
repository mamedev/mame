/*************************************************************************

    Atari Skull & Crossbones hardware

*************************************************************************/

#include "machine/atarigen.h"

class skullxbo_state
{
public:
	static void *alloc(running_machine &machine) { return auto_alloc_clear(&machine, skullxbo_state(machine)); }

	skullxbo_state(running_machine &machine) { }

	atarigen_state	atarigen;
};


/*----------- defined in video/skullxbo.c -----------*/

WRITE16_HANDLER( skullxbo_playfieldlatch_w );
WRITE16_HANDLER( skullxbo_xscroll_w );
WRITE16_HANDLER( skullxbo_yscroll_w );
WRITE16_HANDLER( skullxbo_mobmsb_w );

VIDEO_START( skullxbo );
VIDEO_UPDATE( skullxbo );

void skullxbo_scanline_update(running_machine *machine, int param);
