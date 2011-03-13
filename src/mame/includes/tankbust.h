class tankbust_state : public driver_device
{
public:
	tankbust_state(running_machine &machine, const driver_device_config_base &config)
		: driver_device(machine, config) { }

	int latch;
	UINT32 timer1;
	int e0xx_data[8];
	UINT8 variable_data;
	UINT8 *txtram;
	UINT8 *videoram;
	UINT8 *colorram;
	tilemap_t *bg_tilemap;
	tilemap_t *txt_tilemap;
	UINT8 xscroll[2];
	UINT8 yscroll[2];
	UINT8 *spriteram;
	size_t spriteram_size;
};


/*----------- defined in video/tankbust.c -----------*/

VIDEO_START( tankbust );
SCREEN_UPDATE( tankbust );

WRITE8_HANDLER( tankbust_background_videoram_w );
READ8_HANDLER( tankbust_background_videoram_r );
WRITE8_HANDLER( tankbust_background_colorram_w );
READ8_HANDLER( tankbust_background_colorram_r );
WRITE8_HANDLER( tankbust_txtram_w );
READ8_HANDLER( tankbust_txtram_r );

WRITE8_HANDLER( tankbust_xscroll_w );
WRITE8_HANDLER( tankbust_yscroll_w );


