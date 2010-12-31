/*----------- defined in video/pk8000.c -----------*/

READ8_HANDLER(pk8000_video_color_r);
WRITE8_HANDLER(pk8000_video_color_w);
READ8_HANDLER(pk8000_text_start_r);
WRITE8_HANDLER(pk8000_text_start_w);
READ8_HANDLER(pk8000_chargen_start_r);
WRITE8_HANDLER(pk8000_chargen_start_w);
READ8_HANDLER(pk8000_video_start_r);
WRITE8_HANDLER(pk8000_video_start_w);
READ8_HANDLER(pk8000_color_start_r);
WRITE8_HANDLER(pk8000_color_start_w);
READ8_HANDLER(pk8000_color_r);
WRITE8_HANDLER(pk8000_color_w);

extern UINT8 pk8000_video_mode;
extern UINT8 pk8000_video_enable;

PALETTE_INIT( pk8000 );

UINT32 pk8000_video_update(device_t *screen, bitmap_t *bitmap, const rectangle *cliprect, UINT8 *videomem);
