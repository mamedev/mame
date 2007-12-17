/*************************************************************************

    Atari Rampart hardware

*************************************************************************/


/*----------- defined in video/rampart.c -----------*/

WRITE16_HANDLER( rampart_bitmap_w );

VIDEO_START( rampart );
VIDEO_UPDATE( rampart );

void rampart_bitmap_init(running_machine *machine, int _xdim, int _ydim);
void rampart_bitmap_render(running_machine *machine, mame_bitmap *bitmap, const rectangle *cliprect);

extern UINT16 *rampart_bitmap;
