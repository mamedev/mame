/*************************************************************************

    Atari GX2 hardware

*************************************************************************/

#include "machine/atarigen.h"

typedef struct _atarigx2_state atarigx2_state;
struct _atarigx2_state
{
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
};


/*----------- defined in video/atarigx2.c -----------*/

VIDEO_START( atarigx2 );
VIDEO_UPDATE( atarigx2 );

WRITE16_HANDLER( atarigx2_mo_control_w );

void atarigx2_scanline_update(const device_config *screen, int scanline);
