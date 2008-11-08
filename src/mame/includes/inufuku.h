/*----------- defined in drivers/inufuku.c -----------*/

extern UINT16 *inufuku_spriteram1;
extern UINT16 *inufuku_spriteram2;
extern size_t inufuku_spriteram1_size;

extern UINT16 *inufuku_bg_videoram;
extern UINT16 *inufuku_bg_rasterram;
extern UINT16 *inufuku_text_videoram;


/*----------- defined in video/inufuku.c -----------*/

VIDEO_UPDATE( inufuku );
VIDEO_START( inufuku );

READ16_HANDLER( inufuku_bg_videoram_r );
WRITE16_HANDLER( inufuku_bg_videoram_w );
READ16_HANDLER( inufuku_text_videoram_r );
WRITE16_HANDLER( inufuku_text_videoram_w );
WRITE16_HANDLER( inufuku_palettereg_w );
WRITE16_HANDLER( inufuku_scrollreg_w );
