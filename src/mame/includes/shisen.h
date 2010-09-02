class shisen_state : public driver_device
{
public:
	shisen_state(running_machine &machine, const driver_device_config_base &config)
		: driver_device(machine, config) { }

	int gfxbank;
	tilemap_t *bg_tilemap;
	UINT8 *paletteram;
	UINT8 *videoram;
};


/*----------- defined in video/shisen.c -----------*/

WRITE8_HANDLER( sichuan2_videoram_w );
WRITE8_HANDLER( sichuan2_bankswitch_w );
WRITE8_HANDLER( sichuan2_paletteram_w );

VIDEO_START( sichuan2 );
VIDEO_UPDATE( sichuan2 );
