/*----------- defined in video/xxmissio.c -----------*/

extern UINT8 *xxmissio_bgram;
extern UINT8 *xxmissio_fgram;
extern UINT8 *xxmissio_spriteram;

VIDEO_START( xxmissio );
VIDEO_UPDATE( xxmissio );

WRITE8_DEVICE_HANDLER( xxmissio_scroll_x_w );
WRITE8_DEVICE_HANDLER( xxmissio_scroll_y_w );
WRITE8_HANDLER( xxmissio_flipscreen_w );

READ8_HANDLER( xxmissio_bgram_r );
WRITE8_HANDLER( xxmissio_bgram_w );

WRITE8_HANDLER( xxmissio_paletteram_w );
