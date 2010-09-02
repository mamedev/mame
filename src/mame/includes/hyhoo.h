/*----------- defined in video/hyhoo.c -----------*/

extern UINT8 *hyhoo_clut;

VIDEO_UPDATE( hyhoo );
VIDEO_START( hyhoo );

WRITE8_HANDLER( hyhoo_blitter_w );
WRITE8_HANDLER( hyhoo_romsel_w );
