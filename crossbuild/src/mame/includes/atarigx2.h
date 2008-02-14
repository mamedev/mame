/*************************************************************************

    Atari G42 hardware

*************************************************************************/

/*----------- defined in video/atarigx2.c -----------*/

extern UINT16 atarigx2_playfield_base;
extern UINT16 atarigx2_motion_object_base;
extern UINT16 atarigx2_motion_object_mask;

VIDEO_START( atarigx2 );
VIDEO_UPDATE( atarigx2 );

WRITE16_HANDLER( atarigx2_mo_control_w );

void atarigx2_scanline_update(running_machine *machine, int scrnum, int scanline);
