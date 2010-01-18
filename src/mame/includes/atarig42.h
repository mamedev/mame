/*************************************************************************

    Atari G42 hardware

*************************************************************************/

#include "machine/atarigen.h"

typedef struct _atarig42_state atarig42_state;
struct _atarig42_state
{
	atarigen_state	atarigen;
	UINT16			playfield_base;
	UINT16			motion_object_base;
	UINT16			motion_object_mask;

	UINT16			current_control;
	UINT8			playfield_tile_bank;
	UINT8			playfield_color_bank;
	UINT16			playfield_xscroll;
	UINT16			playfield_yscroll;

	UINT8			analog_data;
	UINT16 *		mo_command;

	int 			sloop_bank;
	int 			sloop_next_bank;
	int 			sloop_offset;
	int 			sloop_state;
	UINT16 *		sloop_base;
};


/*----------- defined in video/atarig42.c -----------*/

VIDEO_START( atarig42 );
VIDEO_UPDATE( atarig42 );

WRITE16_HANDLER( atarig42_mo_control_w );

void atarig42_scanline_update(running_device *screen, int scanline);

