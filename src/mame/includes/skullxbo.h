/*************************************************************************

    Atari Skull & Crossbones hardware

*************************************************************************/

#include "machine/atarigen.h"

class skullxbo_state : public atarigen_state
{
public:
	skullxbo_state(const machine_config &mconfig, device_type type, const char *tag)
		: atarigen_state(mconfig, type, tag) { }
};


/*----------- defined in video/skullxbo.c -----------*/

WRITE16_HANDLER( skullxbo_playfieldlatch_w );
WRITE16_HANDLER( skullxbo_xscroll_w );
WRITE16_HANDLER( skullxbo_yscroll_w );
WRITE16_HANDLER( skullxbo_mobmsb_w );

VIDEO_START( skullxbo );
SCREEN_UPDATE( skullxbo );

void skullxbo_scanline_update(running_machine &machine, int param);
