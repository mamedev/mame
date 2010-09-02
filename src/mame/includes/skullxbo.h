/*************************************************************************

    Atari Skull & Crossbones hardware

*************************************************************************/

#include "machine/atarigen.h"

class skullxbo_state : public atarigen_state
{
public:
	skullxbo_state(running_machine &machine, const driver_device_config_base &config)
		: atarigen_state(machine, config) { }
};


/*----------- defined in video/skullxbo.c -----------*/

WRITE16_HANDLER( skullxbo_playfieldlatch_w );
WRITE16_HANDLER( skullxbo_xscroll_w );
WRITE16_HANDLER( skullxbo_yscroll_w );
WRITE16_HANDLER( skullxbo_mobmsb_w );

VIDEO_START( skullxbo );
VIDEO_UPDATE( skullxbo );

void skullxbo_scanline_update(running_machine *machine, int param);
