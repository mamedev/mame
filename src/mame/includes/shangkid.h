/*----------- defined in video/shangkid.c -----------*/

extern UINT8 *shangkid_videoreg;
extern int shangkid_gfx_type;

VIDEO_START( shangkid );
VIDEO_UPDATE( shangkid );
WRITE8_HANDLER( shangkid_videoram_w );

PALETTE_INIT( dynamski );
VIDEO_UPDATE( dynamski );

