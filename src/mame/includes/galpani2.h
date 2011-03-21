/*----------- defined in video/galpani2.c -----------*/

extern UINT16 *galpani2_bg8_0,         *galpani2_bg8_1;
extern UINT16 *galpani2_palette_0,     *galpani2_palette_1;
extern UINT16 *galpani2_bg8_0_scrollx, *galpani2_bg8_1_scrollx;
extern UINT16 *galpani2_bg8_0_scrolly, *galpani2_bg8_1_scrolly;

extern UINT16 *galpani2_bg15;

PALETTE_INIT( galpani2 );
VIDEO_START( galpani2 );
SCREEN_UPDATE( galpani2 );

WRITE16_HANDLER( galpani2_palette_0_w );
WRITE16_HANDLER( galpani2_palette_1_w );

WRITE16_HANDLER( galpani2_bg8_0_w );
WRITE16_HANDLER( galpani2_bg8_1_w );

WRITE16_HANDLER( galpani2_bg15_w );
