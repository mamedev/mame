/*************************************************************************

    Atari G1 hardware

*************************************************************************/

#include "machine/atarigen.h"

typedef struct _atarig1_state atarig1_state;
struct _atarig1_state
{
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

void atarig1_scanline_update(const device_config *screen, int scanline);
