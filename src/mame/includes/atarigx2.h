/*************************************************************************

    Atari GX2 hardware

*************************************************************************/

#include "machine/atarigen.h"

class atarigx2_state
{
public:
	static void *alloc(running_machine &machine) { return auto_alloc_clear(&machine, atarigx2_state(machine)); }

	atarigx2_state(running_machine &machine) { }

	atarigen_state	atarigen;
	UINT16			playfield_base;
	UINT16			motion_object_base;
	UINT16			motion_object_mask;

	UINT32 *		mo_command;
	UINT32 *		protection_base;

	UINT16			current_control;
	UINT8			playfield_tile_bank;
	UINT8			playfield_color_bank;
	UINT16			playfield_xscroll;
	UINT16			playfield_yscroll;

	UINT16			last_write;
	UINT16			last_write_offset;
};


/*----------- defined in video/atarigx2.c -----------*/

VIDEO_START( atarigx2 );
VIDEO_UPDATE( atarigx2 );

WRITE16_HANDLER( atarigx2_mo_control_w );

void atarigx2_scanline_update(screen_device &screen, int scanline);
