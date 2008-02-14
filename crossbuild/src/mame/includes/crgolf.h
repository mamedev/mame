/*************************************************************************

    Kitco Crowns Golf hardware

**************************************************************************/

/*----------- defined in video/crgolf.c -----------*/

extern UINT8 *crgolf_color_select;
extern UINT8 *crgolf_screen_flip;
extern UINT8 *crgolf_screen_select;
extern UINT8 *crgolf_screenb_enable;
extern UINT8 *crgolf_screena_enable;

WRITE8_HANDLER( crgolf_videoram_w );
READ8_HANDLER( crgolf_videoram_r );

MACHINE_DRIVER_EXTERN( crgolf_video );
