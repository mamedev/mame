/*----------- defined in video/superqix.c -----------*/

extern UINT8 *superqix_videoram;
extern UINT8 *superqix_bitmapram,*superqix_bitmapram2;
extern int pbillian_show_power;

WRITE8_HANDLER( superqix_videoram_w );
WRITE8_HANDLER( superqix_bitmapram_w );
WRITE8_HANDLER( superqix_bitmapram2_w );
WRITE8_HANDLER( pbillian_0410_w );
WRITE8_HANDLER( superqix_0410_w );

VIDEO_START( pbillian );
VIDEO_UPDATE( pbillian );
VIDEO_START( superqix );
VIDEO_UPDATE( superqix );
