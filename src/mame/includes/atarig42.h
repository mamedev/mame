/*************************************************************************

    Atari G42 hardware

*************************************************************************/

/*----------- defined in video/atarig42.c -----------*/

extern UINT16 atarig42_playfield_base;
extern UINT16 atarig42_motion_object_base;
extern UINT16 atarig42_motion_object_mask;

VIDEO_START( atarig42 );
VIDEO_UPDATE( atarig42 );

WRITE16_HANDLER( atarig42_mo_control_w );

void atarig42_scanline_update(running_machine *machine, int scrnum, int scanline);

