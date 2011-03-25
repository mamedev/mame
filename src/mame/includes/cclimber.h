class cclimber_state : public driver_device
{
public:
	cclimber_state(running_machine &machine, const driver_device_config_base &config)
		: driver_device(machine, config) { }

	UINT8 *videoram;
	UINT8 *colorram;
	UINT8 *spriteram;
	UINT8 *bigsprite_videoram;
	UINT8 *bigsprite_control;
	UINT8 *column_scroll;
	UINT8 *flip_screen;
	UINT8 *swimmer_background_color;
	UINT8 *swimmer_side_background_enabled;
	UINT8 *swimmer_palettebank;
	UINT8 *toprollr_bg_videoram;
	UINT8 *toprollr_bg_coloram;
	UINT8 yamato_p0;
	UINT8 yamato_p1;
	UINT8 toprollr_rombank;
	tilemap_t *pf_tilemap;
	tilemap_t *bs_tilemap;
	tilemap_t *toproller_bg_tilemap;
};


/*----------- defined in machine/cclimber.c -----------*/

DRIVER_INIT( cclimber );
DRIVER_INIT( cclimberj );
DRIVER_INIT( cannonb );
DRIVER_INIT( cannonb2 );
DRIVER_INIT( ckongb );

/*----------- defined in video/cclimber.c -----------*/

WRITE8_HANDLER( cclimber_colorram_w );
WRITE8_HANDLER( cannonb_flip_screen_w );

PALETTE_INIT( cclimber );
VIDEO_START( cclimber );
SCREEN_UPDATE( cclimber );

PALETTE_INIT( swimmer );
VIDEO_START( swimmer );
SCREEN_UPDATE( swimmer );

PALETTE_INIT( yamato );
SCREEN_UPDATE( yamato );

PALETTE_INIT( toprollr );
VIDEO_START( toprollr );
SCREEN_UPDATE( toprollr );
