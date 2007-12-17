/*************************************************************************

    Raster Elite Tickee Tickats hardware

**************************************************************************/

/*----------- defined in drivers/tickee.c -----------*/

extern UINT16 *tickee_control;


/*----------- defined in video/tickee.c -----------*/

extern UINT16 *tickee_vram;

VIDEO_START( tickee );

void tickee_scanline_update(running_machine *machine, int screen, mame_bitmap *bitmap, int scanline, const tms34010_display_params *params);
