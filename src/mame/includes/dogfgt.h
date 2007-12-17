/*----------- defined in video/dogfgt.c -----------*/

extern UINT8 *dogfgt_bgvideoram;

WRITE8_HANDLER( dogfgt_plane_select_w );
READ8_HANDLER( dogfgt_bitmapram_r );
WRITE8_HANDLER( dogfgt_bitmapram_w );
WRITE8_HANDLER( dogfgt_bgvideoram_w );
WRITE8_HANDLER( dogfgt_scroll_w );
WRITE8_HANDLER( dogfgt_1800_w );

PALETTE_INIT( dogfgt );
VIDEO_START( dogfgt );
VIDEO_UPDATE( dogfgt );
