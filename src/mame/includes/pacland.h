class pacland_state : public driver_device
{
public:
	pacland_state(running_machine &machine, const driver_device_config_base &config)
		: driver_device(machine, config) { }

	UINT8 *videoram;
	UINT8 *videoram2;
	UINT8 *spriteram;
	UINT8 palette_bank;
	const UINT8 *color_prom;
	tilemap_t *bg_tilemap;
	tilemap_t *fg_tilemap;
	bitmap_t *fg_bitmap;
	UINT32 *transmask[3];
	UINT16 scroll0;
	UINT16 scroll1;
};


/*----------- defined in video/pacland.c -----------*/

WRITE8_HANDLER( pacland_videoram_w );
WRITE8_HANDLER( pacland_videoram2_w );
WRITE8_HANDLER( pacland_scroll0_w );
WRITE8_HANDLER( pacland_scroll1_w );
WRITE8_HANDLER( pacland_bankswitch_w );

PALETTE_INIT( pacland );
VIDEO_START( pacland );
SCREEN_UPDATE( pacland );
