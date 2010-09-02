class solomon_state : public driver_device
{
public:
	solomon_state(running_machine &machine, const driver_device_config_base &config)
		: driver_device(machine, config) { }

	size_t spriteram_size;
	UINT8 *spriteram;
	UINT8 *videoram;
	UINT8 *colorram;
	UINT8 *videoram2;
	UINT8 *colorram2;

	tilemap_t *bg_tilemap;
	tilemap_t *fg_tilemap;
};


/*----------- defined in video/solomon.c -----------*/

WRITE8_HANDLER( solomon_videoram_w );
WRITE8_HANDLER( solomon_colorram_w );
WRITE8_HANDLER( solomon_videoram2_w );
WRITE8_HANDLER( solomon_colorram2_w );
WRITE8_HANDLER( solomon_flipscreen_w );

VIDEO_START( solomon );
VIDEO_UPDATE( solomon );
