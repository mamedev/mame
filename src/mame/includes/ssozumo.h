class ssozumo_state : public driver_device
{
public:
	ssozumo_state(running_machine &machine, const driver_device_config_base &config)
		: driver_device(machine, config) { }

	size_t spriteram_size;
	UINT8 *spriteram;
	UINT8 *paletteram;
	UINT8 *videoram;
	UINT8 *colorram;
	UINT8 *videoram2;
	UINT8 *colorram2;

	tilemap_t *bg_tilemap;
	tilemap_t *fg_tilemap;
};


/*----------- defined in video/ssozumo.c -----------*/

WRITE8_HANDLER( ssozumo_videoram_w );
WRITE8_HANDLER( ssozumo_colorram_w );
WRITE8_HANDLER( ssozumo_videoram2_w );
WRITE8_HANDLER( ssozumo_colorram2_w );
WRITE8_HANDLER( ssozumo_paletteram_w );
WRITE8_HANDLER( ssozumo_scroll_w );
WRITE8_HANDLER( ssozumo_flipscreen_w );

PALETTE_INIT( ssozumo );
VIDEO_START( ssozumo );
VIDEO_UPDATE( ssozumo );
