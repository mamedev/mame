class shisen_state : public driver_device
{
public:
	shisen_state(running_machine &machine, const driver_device_config_base &config)
		: driver_device(machine, config) { }

	int m_gfxbank;
	tilemap_t *m_bg_tilemap;
	UINT8 *m_paletteram;
	UINT8 *m_videoram;
};


/*----------- defined in video/shisen.c -----------*/

WRITE8_HANDLER( sichuan2_videoram_w );
WRITE8_HANDLER( sichuan2_bankswitch_w );
WRITE8_HANDLER( sichuan2_paletteram_w );

VIDEO_START( sichuan2 );
SCREEN_UPDATE( sichuan2 );
