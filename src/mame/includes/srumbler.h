class srumbler_state : public driver_device
{
public:
	srumbler_state(running_machine &machine, const driver_device_config_base &config)
		: driver_device(machine, config) { }

	UINT8 *m_backgroundram;
	UINT8 *m_foregroundram;
	tilemap_t *m_bg_tilemap;
	tilemap_t *m_fg_tilemap;
	int m_scroll[4];
};


/*----------- defined in video/srumbler.c -----------*/

WRITE8_HANDLER( srumbler_background_w );
WRITE8_HANDLER( srumbler_foreground_w );
WRITE8_HANDLER( srumbler_scroll_w );
WRITE8_HANDLER( srumbler_4009_w );

VIDEO_START( srumbler );
SCREEN_UPDATE( srumbler );
SCREEN_EOF( srumbler );
