class tankbatt_state : public driver_device
{
public:
	tankbatt_state(running_machine &machine, const driver_device_config_base &config)
		: driver_device(machine, config) { }

	UINT8 *videoram;
	int nmi_enable;
	int sound_enable;
	UINT8 *bulletsram;
	size_t bulletsram_size;
	tilemap_t *bg_tilemap;
};


/*----------- defined in video/tankbatt.c -----------*/

WRITE8_HANDLER( tankbatt_videoram_w );

PALETTE_INIT( tankbatt );
VIDEO_START( tankbatt );
SCREEN_UPDATE( tankbatt );
