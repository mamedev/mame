/*----------- defined in video/baraduke.c -----------*/

extern UINT8 *baraduke_textram, *baraduke_videoram, *baraduke_spriteram;

VIDEO_START( baraduke );
VIDEO_UPDATE( baraduke );
VIDEO_EOF( baraduke );
READ8_HANDLER( baraduke_videoram_r );
WRITE8_HANDLER( baraduke_videoram_w );
READ8_HANDLER( baraduke_textram_r );
WRITE8_HANDLER( baraduke_textram_w );
WRITE8_HANDLER( baraduke_scroll0_w );
WRITE8_HANDLER( baraduke_scroll1_w );
READ8_HANDLER( baraduke_spriteram_r );
WRITE8_HANDLER( baraduke_spriteram_w );
PALETTE_INIT( baraduke );
