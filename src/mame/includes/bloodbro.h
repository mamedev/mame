/*----------- defined in video/bloodbro.c -----------*/

extern UINT16 *bloodbro_bgvideoram, *bloodbro_fgvideoram;
extern UINT16 *bloodbro_txvideoram;
extern UINT16 *bloodbro_scroll;

WRITE16_HANDLER( bloodbro_bgvideoram_w );
WRITE16_HANDLER( bloodbro_fgvideoram_w );
WRITE16_HANDLER( bloodbro_txvideoram_w );

SCREEN_UPDATE( bloodbro );
SCREEN_UPDATE( weststry );
SCREEN_UPDATE( skysmash );
VIDEO_START( bloodbro );
