/*----------- defined in video/tehkanwc.c -----------*/

extern UINT8 *tehkanwc_videoram;
extern UINT8 *tehkanwc_colorram;
extern UINT8 *tehkanwc_videoram2;

extern WRITE8_HANDLER( tehkanwc_videoram_w );
extern WRITE8_HANDLER( tehkanwc_colorram_w );
extern WRITE8_HANDLER( tehkanwc_videoram2_w );
extern WRITE8_HANDLER( tehkanwc_scroll_x_w );
extern WRITE8_HANDLER( tehkanwc_scroll_y_w );
extern WRITE8_HANDLER( tehkanwc_flipscreen_x_w );
extern WRITE8_HANDLER( tehkanwc_flipscreen_y_w );
extern WRITE8_HANDLER( gridiron_led0_w );
extern WRITE8_HANDLER( gridiron_led1_w );

extern VIDEO_START( tehkanwc );
extern VIDEO_UPDATE( tehkanwc );
