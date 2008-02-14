/***************************************************************************

    Kyugo hardware games

***************************************************************************/

/*----------- defined in drivers/kyugo.c -----------*/

MACHINE_RESET( kyugo );

WRITE8_HANDLER( kyugo_sub_cpu_control_w );

/*----------- defined in video/kyugo.c -----------*/

extern UINT8 *kyugo_fgvideoram;
extern UINT8 *kyugo_bgvideoram;
extern UINT8 *kyugo_bgattribram;
extern UINT8 *kyugo_spriteram_1;
extern UINT8 *kyugo_spriteram_2;

READ8_HANDLER( kyugo_spriteram_2_r );

WRITE8_HANDLER( kyugo_fgvideoram_w );
WRITE8_HANDLER( kyugo_bgvideoram_w );
WRITE8_HANDLER( kyugo_bgattribram_w );
WRITE8_HANDLER( kyugo_scroll_x_lo_w );
WRITE8_HANDLER( kyugo_gfxctrl_w );
WRITE8_HANDLER( kyugo_scroll_y_w );
WRITE8_HANDLER( kyugo_flipscreen_w );

VIDEO_START( kyugo );

VIDEO_UPDATE( kyugo );
