/*************************************************************************

    Atari G1 hardware

*************************************************************************/

#include "machine/atarigen.h"

class atarig1_state
{
public:
	static void *alloc(running_machine &machine) { return auto_alloc_clear(&machine, atarig1_state(machine)); }

	atarig1_state(running_machine &machine) { }

	atarigen_state	atarigen;
	UINT8			is_pitfight;

	UINT8			which_input;
	UINT16 *		mo_command;

	UINT16 *		bslapstic_base;
	void *			bslapstic_bank0;
	UINT8			bslapstic_bank;
	UINT8			bslapstic_primed;

	int 			pfscroll_xoffset;
	UINT16			current_control;
	UINT8			playfield_tile_bank;
	UINT16			playfield_xscroll;
	UINT16			playfield_yscroll;
};


/*----------- defined in video/atarig1.c -----------*/

WRITE16_HANDLER( atarig1_mo_control_w );

VIDEO_START( atarig1 );
VIDEO_UPDATE( atarig1 );

void atarig1_scanline_update(screen_device &screen, int scanline);
