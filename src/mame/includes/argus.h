/*----------- defined in video/argus.c -----------*/

extern UINT8 *argus_paletteram;
extern UINT8 *argus_txram;
extern UINT8 *argus_bg0_scrollx;
extern UINT8 *argus_bg0_scrolly;
extern UINT8 *argus_bg1ram;
extern UINT8 *argus_bg1_scrollx;
extern UINT8 *argus_bg1_scrolly;
extern UINT8 *butasan_bg1ram;

VIDEO_START( argus );
VIDEO_START( valtric );
VIDEO_START( butasan );
VIDEO_RESET( argus );
VIDEO_RESET( valtric );
VIDEO_RESET( butasan );
VIDEO_UPDATE( argus );
VIDEO_UPDATE( valtric );
VIDEO_UPDATE( butasan );

READ8_HANDLER( argus_txram_r );
READ8_HANDLER( argus_bg1ram_r );
READ8_HANDLER( argus_paletteram_r );

WRITE8_HANDLER( argus_txram_w );
WRITE8_HANDLER( argus_bg1ram_w );
WRITE8_HANDLER( argus_bg0_scrollx_w );
WRITE8_HANDLER( argus_bg0_scrolly_w );
WRITE8_HANDLER( argus_bg1_scrollx_w );
WRITE8_HANDLER( argus_bg1_scrolly_w );
WRITE8_HANDLER( argus_bg_status_w );
WRITE8_HANDLER( argus_flipscreen_w );
WRITE8_HANDLER( argus_paletteram_w );

WRITE8_HANDLER( valtric_bg_status_w );
WRITE8_HANDLER( valtric_paletteram_w );
WRITE8_HANDLER( valtric_mosaic_w );
WRITE8_HANDLER( valtric_unknown_w );

READ8_HANDLER( butasan_pagedram_r );
READ8_HANDLER( butasan_bg1ram_r );
WRITE8_HANDLER( butasan_pageselect_w );
WRITE8_HANDLER( butasan_pagedram_w );
WRITE8_HANDLER( butasan_bg1ram_w );
WRITE8_HANDLER( butasan_bg0_status_w );
WRITE8_HANDLER( butasan_paletteram_w );
WRITE8_HANDLER( butasan_bg1_status_w );
WRITE8_HANDLER( butasan_unknown_w );
