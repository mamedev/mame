/*----------- defined in video/punchout.c -----------*/

extern UINT8 *punchout_bg_top_videoram;
extern UINT8 *punchout_bg_bot_videoram;
extern UINT8 *armwrest_fg_videoram;
extern UINT8 *punchout_spr1_videoram;
extern UINT8 *punchout_spr2_videoram;
extern UINT8 *punchout_spr1_ctrlram;
extern UINT8 *punchout_spr2_ctrlram;
extern UINT8 *punchout_palettebank;

WRITE8_HANDLER( punchout_bg_top_videoram_w );
WRITE8_HANDLER( punchout_bg_bot_videoram_w );
WRITE8_HANDLER( armwrest_fg_videoram_w );
WRITE8_HANDLER( punchout_spr1_videoram_w );
WRITE8_HANDLER( punchout_spr2_videoram_w );

VIDEO_START( punchout );
VIDEO_START( armwrest );
VIDEO_UPDATE( punchout );
VIDEO_UPDATE( armwrest );

DRIVER_INIT( punchout );
DRIVER_INIT( spnchout );
DRIVER_INIT( spnchotj );
DRIVER_INIT( armwrest );
