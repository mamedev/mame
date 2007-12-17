/*----------- defined in video/lemmings.c -----------*/

VIDEO_START( lemmings );
VIDEO_EOF( lemmings );
VIDEO_UPDATE( lemmings );
WRITE16_HANDLER( lemmings_pixel_0_w );
WRITE16_HANDLER( lemmings_pixel_1_w );
WRITE16_HANDLER( lemmings_vram_w );

extern UINT16 *lemmings_pixel_0_data,*lemmings_pixel_1_data,*lemmings_vram_data,*lemmings_control_data;
