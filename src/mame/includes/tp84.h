/*----------- defined in video/tp84.c -----------*/

extern UINT8 *tp84_bg_videoram;
extern UINT8 *tp84_bg_colorram;
extern UINT8 *tp84_fg_videoram;
extern UINT8 *tp84_fg_colorram;
extern UINT8 *tp84_spriteram;
extern UINT8 *tp84_scroll_x;
extern UINT8 *tp84_scroll_y;
extern UINT8 *tp84_palette_bank;
extern UINT8 *tp84_flipscreen_x;
extern UINT8 *tp84_flipscreen_y;

WRITE8_HANDLER( tp84_spriteram_w );
READ8_HANDLER( tp84_scanline_r );

PALETTE_INIT( tp84 );
VIDEO_START( tp84 );
VIDEO_UPDATE( tp84 );
