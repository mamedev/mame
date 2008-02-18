/*----------- defined in video/nova2001.c -----------*/

extern UINT8 *nova2001_fg_videoram, *nova2001_bg_videoram;

extern WRITE8_HANDLER( nova2001_fg_videoram_w );
extern WRITE8_HANDLER( nova2001_bg_videoram_w );
extern WRITE8_HANDLER( ninjakun_bg_videoram_w );
extern READ8_HANDLER( ninjakun_bg_videoram_r );
extern WRITE8_HANDLER( nova2001_scroll_x_w );
extern WRITE8_HANDLER( nova2001_scroll_y_w );
extern WRITE8_HANDLER( nova2001_flipscreen_w );
extern WRITE8_HANDLER( pkunwar_flipscreen_w );
extern WRITE8_HANDLER( ninjakun_paletteram_w );

extern PALETTE_INIT( nova2001 );
extern VIDEO_START( nova2001 );
extern VIDEO_UPDATE( nova2001 );
extern VIDEO_START( ninjakun );
extern VIDEO_UPDATE( ninjakun );
extern VIDEO_START( pkunwar );
extern VIDEO_UPDATE( pkunwar );
extern VIDEO_START( raiders5 );
extern VIDEO_UPDATE( raiders5 );
