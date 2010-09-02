class pass_state : public driver_device
{
public:
	pass_state(running_machine &machine, const driver_device_config_base &config)
		: driver_device(machine, config) { }

	tilemap_t *bg_tilemap;
	tilemap_t *fg_tilemap;

	UINT16 *bg_videoram;
	UINT16 *fg_videoram;
};


/*----------- defined in video/pass.c -----------*/

WRITE16_HANDLER( pass_fg_videoram_w );
WRITE16_HANDLER( pass_bg_videoram_w );

VIDEO_START( pass );
VIDEO_UPDATE( pass );
