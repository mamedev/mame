class snookr10_state : public driver_device
{
public:
	snookr10_state(running_machine &machine, const driver_device_config_base &config)
		: driver_device(machine, config) { }

	int outportl;
	int outporth;
	int bit0;
	int bit1;
	int bit2;
	int bit3;
	int bit4;
	int bit5;
	UINT8 *videoram;
	UINT8 *colorram;
	tilemap_t *bg_tilemap;
};


/*----------- defined in video/snookr10.c -----------*/

WRITE8_HANDLER( snookr10_videoram_w );
WRITE8_HANDLER( snookr10_colorram_w );
PALETTE_INIT( snookr10 );
PALETTE_INIT( apple10 );
VIDEO_START( snookr10 );
VIDEO_START( apple10 );
SCREEN_UPDATE( snookr10 );

