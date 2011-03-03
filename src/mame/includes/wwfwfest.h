class wwfwfest_state : public driver_device
{
public:
	wwfwfest_state(running_machine &machine, const driver_device_config_base &config)
		: driver_device(machine, config) { }

	UINT16 *fg0_videoram;
	UINT16 *bg0_videoram;
	UINT16 *bg1_videoram;
	UINT16 pri;
	UINT16 bg0_scrollx;
	UINT16 bg0_scrolly;
	UINT16 bg1_scrollx;
	UINT16 bg1_scrolly;
	tilemap_t *fg0_tilemap;
	tilemap_t *bg0_tilemap;
	tilemap_t *bg1_tilemap;
	UINT16 sprite_xoff;
	UINT16 bg0_dx;
	UINT16 bg1_dx[2];
};


/*----------- defined in video/wwfwfest.c -----------*/

VIDEO_START( wwfwfest );
VIDEO_START( wwfwfstb );
SCREEN_UPDATE( wwfwfest );
WRITE16_HANDLER( wwfwfest_fg0_videoram_w );
WRITE16_HANDLER( wwfwfest_bg0_videoram_w );
WRITE16_HANDLER( wwfwfest_bg1_videoram_w );
