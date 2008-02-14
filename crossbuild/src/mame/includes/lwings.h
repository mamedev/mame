/*----------- defined in video/lwings.c -----------*/

extern UINT8 *lwings_fgvideoram;
extern UINT8 *lwings_bg1videoram;

WRITE8_HANDLER( lwings_fgvideoram_w );
WRITE8_HANDLER( lwings_bg1videoram_w );
WRITE8_HANDLER( lwings_bg1_scrollx_w );
WRITE8_HANDLER( lwings_bg1_scrolly_w );
WRITE8_HANDLER( trojan_bg2_scrollx_w );
WRITE8_HANDLER( trojan_bg2_image_w );
VIDEO_START( lwings );
VIDEO_START( trojan );
VIDEO_START( avengers );
VIDEO_UPDATE( lwings );
VIDEO_UPDATE( trojan );
VIDEO_EOF( lwings );
