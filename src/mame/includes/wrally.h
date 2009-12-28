/*----------- defined in machine/wrally.c -----------*/

WRITE16_HANDLER( wrally_vram_w );
WRITE16_HANDLER( wrally_flipscreen_w );
WRITE16_HANDLER( OKIM6295_bankswitch_w );
WRITE16_HANDLER( wrally_coin_counter_w );
WRITE16_HANDLER( wrally_coin_lockout_w );

/*----------- defined in video/wrally.c -----------*/

extern tilemap_t *wrally_pant[2];

extern UINT16 *wrally_vregs;
extern UINT16 *wrally_videoram;
extern UINT16 *wrally_spriteram;

VIDEO_START( wrally );
VIDEO_UPDATE( wrally );

