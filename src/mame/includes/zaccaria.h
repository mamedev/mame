/*----------- defined in video/zaccaria.c -----------*/

extern UINT8 *zaccaria_videoram,*zaccaria_attributesram;

PALETTE_INIT( zaccaria );
VIDEO_START( zaccaria );
WRITE8_HANDLER( zaccaria_videoram_w );
WRITE8_HANDLER( zaccaria_attributes_w );
WRITE8_HANDLER( zaccaria_flip_screen_x_w );
WRITE8_HANDLER( zaccaria_flip_screen_y_w );
VIDEO_UPDATE( zaccaria );
