/*----------- defined in video/taito_b.c -----------*/

extern UINT16 *taitob_scroll;
extern UINT16 *TC0180VCU_ram;
extern UINT16 *taitob_spriteram;
extern UINT16 *taitob_pixelram;

VIDEO_START( taitob_color_order0 );
VIDEO_START( taitob_color_order1 );
VIDEO_START( taitob_color_order2 );
VIDEO_START( hitice );
VIDEO_EOF( taitob );

VIDEO_RESET( hitice );

VIDEO_UPDATE( taitob );

WRITE16_HANDLER( TC0180VCU_word_w );
READ16_HANDLER ( TC0180VCU_word_r );

READ16_HANDLER( TC0180VCU_framebuffer_word_r );
WRITE16_HANDLER( TC0180VCU_framebuffer_word_w );

WRITE16_HANDLER( taitob_v_control_w );
READ16_HANDLER ( taitob_v_control_r );

WRITE16_HANDLER( hitice_pixelram_w );
WRITE16_HANDLER( hitice_pixel_scroll_w );
