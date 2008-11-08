/*----------- defined in video/armedf.c -----------*/

extern UINT16 armedf_vreg;
extern UINT16 *armedf_bg_videoram;
extern UINT16 *armedf_fg_videoram;
extern UINT16 *terraf_text_videoram;
extern UINT16 *legion_cmd;
extern tilemap *armedf_tx_tilemap;

void armedf_setgfxtype( int type );

VIDEO_UPDATE( armedf );
VIDEO_EOF( armedf );
VIDEO_START( armedf );

WRITE16_HANDLER( armedf_bg_videoram_w );
WRITE16_HANDLER( armedf_fg_videoram_w );
WRITE16_HANDLER( armedf_text_videoram_w );
WRITE16_HANDLER( terraf_fg_scrollx_w );
WRITE16_HANDLER( terraf_fg_scrolly_w );
WRITE16_HANDLER( terraf_fg_scroll_msb_arm_w );
WRITE16_HANDLER( armedf_fg_scrollx_w );
WRITE16_HANDLER( armedf_fg_scrolly_w );
WRITE16_HANDLER( armedf_bg_scrollx_w );
WRITE16_HANDLER( armedf_bg_scrolly_w );
WRITE16_HANDLER( armedf_mcu_cmd );
