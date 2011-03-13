class pingpong_state : public driver_device
{
public:
	pingpong_state(running_machine &machine, const driver_device_config_base &config)
		: driver_device(machine, config) { }

	int intenable;
	int question_addr_high;
	UINT8 *videoram;
	UINT8 *colorram;
	tilemap_t *bg_tilemap;
	UINT8 *spriteram;
	size_t spriteram_size;
};


/*----------- defined in video/pingpong.c -----------*/

WRITE8_HANDLER( pingpong_videoram_w );
WRITE8_HANDLER( pingpong_colorram_w );

PALETTE_INIT( pingpong );
VIDEO_START( pingpong );
SCREEN_UPDATE( pingpong );
