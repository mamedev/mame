/*----------- defined in video/tecmo16.c -----------*/

extern UINT16 *tecmo16_videoram;
extern UINT16 *tecmo16_colorram;
extern UINT16 *tecmo16_videoram2;
extern UINT16 *tecmo16_colorram2;
extern UINT16 *tecmo16_charram;

WRITE16_HANDLER( tecmo16_videoram_w );
WRITE16_HANDLER( tecmo16_colorram_w );
WRITE16_HANDLER( tecmo16_videoram2_w );
WRITE16_HANDLER( tecmo16_colorram2_w );
WRITE16_HANDLER( tecmo16_charram_w );
WRITE16_HANDLER( tecmo16_flipscreen_w );

WRITE16_HANDLER( tecmo16_scroll_x_w );
WRITE16_HANDLER( tecmo16_scroll_y_w );
WRITE16_HANDLER( tecmo16_scroll2_x_w );
WRITE16_HANDLER( tecmo16_scroll2_y_w );
WRITE16_HANDLER( tecmo16_scroll_char_x_w );
WRITE16_HANDLER( tecmo16_scroll_char_y_w );

VIDEO_START( fstarfrc );
VIDEO_START( ginkun );
VIDEO_START( riot );
VIDEO_UPDATE( tecmo16 );
