class shaolins_state : public driver_device
{
public:
	shaolins_state(running_machine &machine, const driver_device_config_base &config)
		: driver_device(machine, config) { }

	size_t spriteram_size;
	UINT8 *spriteram;
	UINT8 *videoram;
	UINT8 *colorram;
	int palettebank;

	tilemap_t *bg_tilemap;
	UINT8 nmi_enable;
};


/*----------- defined in video/sauro.c -----------*/

WRITE8_HANDLER( shaolins_videoram_w );
WRITE8_HANDLER( shaolins_colorram_w );
WRITE8_HANDLER( shaolins_palettebank_w );
WRITE8_HANDLER( shaolins_scroll_w );
WRITE8_HANDLER( shaolins_nmi_w );

PALETTE_INIT( shaolins );
VIDEO_START( shaolins );
VIDEO_UPDATE( shaolins );
