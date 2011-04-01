class tankbatt_state : public driver_device
{
public:
	tankbatt_state(running_machine &machine, const driver_device_config_base &config)
		: driver_device(machine, config) { }

	UINT8 *m_videoram;
	int m_nmi_enable;
	int m_sound_enable;
	UINT8 *m_bulletsram;
	size_t m_bulletsram_size;
	tilemap_t *m_bg_tilemap;
};


/*----------- defined in video/tankbatt.c -----------*/

WRITE8_HANDLER( tankbatt_videoram_w );

PALETTE_INIT( tankbatt );
VIDEO_START( tankbatt );
SCREEN_UPDATE( tankbatt );
