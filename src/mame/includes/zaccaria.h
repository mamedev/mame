class zaccaria_state : public driver_device
{
public:
	zaccaria_state(running_machine &machine, const driver_device_config_base &config)
		: driver_device(machine, config) { }

	int dsw;
	int active_8910;
	int port0a;
	int acs;
	int last_port0b;
	int toggle;
	UINT8 *videoram;
	UINT8 *attributesram;
	tilemap_t *bg_tilemap;
};


/*----------- defined in video/zaccaria.c -----------*/

PALETTE_INIT( zaccaria );
VIDEO_START( zaccaria );
WRITE8_HANDLER( zaccaria_videoram_w );
WRITE8_HANDLER( zaccaria_attributes_w );
WRITE8_HANDLER( zaccaria_flip_screen_x_w );
WRITE8_HANDLER( zaccaria_flip_screen_y_w );
SCREEN_UPDATE( zaccaria );
