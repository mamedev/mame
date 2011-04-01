class ampoker2_state : public driver_device
{
public:
	ampoker2_state(running_machine &machine, const driver_device_config_base &config)
		: driver_device(machine, config) { }

	UINT8 *m_videoram;
	tilemap_t *m_bg_tilemap;
};


/*----------- defined in video/ampoker2.c -----------*/

WRITE8_HANDLER( ampoker2_videoram_w );
PALETTE_INIT( ampoker2 );
VIDEO_START( ampoker2 );
VIDEO_START( sigma2k );
SCREEN_UPDATE( ampoker2 );
