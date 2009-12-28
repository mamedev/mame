/*----------- defined in video/taito_b.c -----------*/

extern UINT16 *taitob_spriteram;
extern UINT16 *taitob_pixelram;

VIDEO_START( taitob_color_order0 );
VIDEO_START( taitob_color_order1 );
VIDEO_START( taitob_color_order2 );
VIDEO_START( hitice );
VIDEO_EOF( taitob );

VIDEO_RESET( hitice );

VIDEO_UPDATE( taitob );

READ16_HANDLER( tc0180vcu_framebuffer_word_r );
WRITE16_HANDLER( tc0180vcu_framebuffer_word_w );

WRITE16_HANDLER( hitice_pixelram_w );
WRITE16_HANDLER( hitice_pixel_scroll_w );
