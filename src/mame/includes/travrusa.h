class travrusa_state : public driver_device
{
public:
	travrusa_state(running_machine &machine, const driver_device_config_base &config)
		: driver_device(machine, config) { }

	/* memory pointers */
	UINT8 *              videoram;
	UINT8 *              spriteram;
	size_t               spriteram_size;

	/* video-related */
	tilemap_t*             bg_tilemap;
	int                  scrollx[2];
};

/*----------- defined in video/travrusa.c -----------*/

WRITE8_HANDLER( travrusa_videoram_w );
WRITE8_HANDLER( travrusa_scroll_x_low_w );
WRITE8_HANDLER( travrusa_scroll_x_high_w );
WRITE8_HANDLER( travrusa_flipscreen_w );

PALETTE_INIT( travrusa );
PALETTE_INIT( shtrider );
VIDEO_START( travrusa );
VIDEO_UPDATE( travrusa );
