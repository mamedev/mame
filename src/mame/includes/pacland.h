/*----------- defined in video/pacland.c -----------*/

extern UINT8 *pacland_videoram,*pacland_videoram2,*pacland_spriteram;

WRITE8_HANDLER( pacland_videoram_w );
WRITE8_HANDLER( pacland_videoram2_w );
WRITE8_HANDLER( pacland_scroll0_w );
WRITE8_HANDLER( pacland_scroll1_w );
WRITE8_HANDLER( pacland_bankswitch_w );

PALETTE_INIT( pacland );
VIDEO_START( pacland );
VIDEO_UPDATE( pacland );
