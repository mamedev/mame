/***************************************************************************

    Sun Electronics Kangaroo hardware

    driver by Ville Laitinen

***************************************************************************/

/*----------- defined in video/kangaroo.c -----------*/

extern UINT8 *kangaroo_video_control;
extern UINT8 *kangaroo_bank_select;
extern UINT8 *kangaroo_blitter;
extern UINT8 *kangaroo_scroll;

PALETTE_INIT( kangaroo );
VIDEO_START( kangaroo );
VIDEO_UPDATE( kangaroo );

WRITE8_HANDLER( kangaroo_blitter_w );
WRITE8_HANDLER( kangaroo_videoram_w );
WRITE8_HANDLER( kangaroo_video_control_w );
WRITE8_HANDLER( kangaroo_bank_select_w );
WRITE8_HANDLER( kangaroo_color_mask_w );
