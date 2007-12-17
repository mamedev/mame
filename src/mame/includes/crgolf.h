/*************************************************************************

    Kitco Crowns Golf hardware

**************************************************************************/

/*----------- defined in video/crgolf.c -----------*/

extern UINT8 *crgolf_color_select;
extern UINT8 *crgolf_screen_flip;
extern UINT8 *crgolf_screen_select;
extern UINT8 *crgolf_screenb_enable;
extern UINT8 *crgolf_screena_enable;

WRITE8_HANDLER( crgolf_videoram_bit0_w );
WRITE8_HANDLER( crgolf_videoram_bit1_w );
WRITE8_HANDLER( crgolf_videoram_bit2_w );

READ8_HANDLER( crgolf_videoram_bit0_r );
READ8_HANDLER( crgolf_videoram_bit1_r );
READ8_HANDLER( crgolf_videoram_bit2_r );

PALETTE_INIT( crgolf );
VIDEO_START( crgolf );
VIDEO_UPDATE( crgolf );
