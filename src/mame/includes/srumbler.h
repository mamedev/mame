class srumbler_state : public driver_device
{
public:
	srumbler_state(running_machine &machine, const driver_device_config_base &config)
		: driver_device(machine, config) { }

	UINT8 *backgroundram;
	UINT8 *foregroundram;
	tilemap_t *bg_tilemap;
	tilemap_t *fg_tilemap;
	int scroll[4];
};


/*----------- defined in video/srumbler.c -----------*/

WRITE8_HANDLER( srumbler_background_w );
WRITE8_HANDLER( srumbler_foreground_w );
WRITE8_HANDLER( srumbler_scroll_w );
WRITE8_HANDLER( srumbler_4009_w );

VIDEO_START( srumbler );
VIDEO_UPDATE( srumbler );
VIDEO_EOF( srumbler );
