class sauro_state : public driver_device
{
public:
	sauro_state(running_machine &machine, const driver_device_config_base &config)
		: driver_device(machine, config) { }

	size_t spriteram_size;
	UINT8 *spriteram;
	UINT8 *videoram;
	UINT8 *colorram;
	UINT8 *videoram2;
	UINT8 *colorram2;

	tilemap_t *bg_tilemap, *fg_tilemap;
	UINT8 palette_bank;
};


/*----------- defined in video/sauro.c -----------*/

WRITE8_HANDLER( tecfri_videoram_w );
WRITE8_HANDLER( tecfri_colorram_w );
WRITE8_HANDLER( tecfri_videoram2_w );
WRITE8_HANDLER( tecfri_colorram2_w );
WRITE8_HANDLER( tecfri_scroll_bg_w );
WRITE8_HANDLER( sauro_scroll_fg_w );
WRITE8_HANDLER( sauro_palette_bank_w );

VIDEO_START( sauro );
VIDEO_START( trckydoc );

VIDEO_UPDATE( sauro );
VIDEO_UPDATE( trckydoc );
