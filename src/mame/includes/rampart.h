/*************************************************************************

    Atari Rampart hardware

*************************************************************************/


/*----------- defined in video/rampart.c -----------*/

VIDEO_START( rampart );
VIDEO_UPDATE( rampart );

void rampart_bitmap_render(running_machine *machine, bitmap_t *bitmap, const rectangle *cliprect);

extern UINT16 *rampart_bitmap;
