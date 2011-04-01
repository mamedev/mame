class shootout_state : public driver_device
{
public:
	shootout_state(running_machine &machine, const driver_device_config_base &config)
		: driver_device(machine, config) { }

	tilemap_t *m_background;
	tilemap_t *m_foreground;
	UINT8 *m_spriteram;
	UINT8 *m_videoram;
	UINT8 *m_textram;
	int m_bFlicker;
};


/*----------- defined in video/shootout.c -----------*/

WRITE8_HANDLER( shootout_videoram_w );
WRITE8_HANDLER( shootout_textram_w );

PALETTE_INIT( shootout );
VIDEO_START( shootout );
SCREEN_UPDATE( shootout );
SCREEN_UPDATE( shootouj );
