class tp84_state : public driver_device
{
public:
	tp84_state(running_machine &machine, const driver_device_config_base &config)
		: driver_device(machine, config) { }

	cpu_device *audiocpu;
	UINT8 *bg_videoram;
	UINT8 *bg_colorram;
	UINT8 *fg_videoram;
	UINT8 *fg_colorram;
	UINT8 *spriteram;
	UINT8 *scroll_x;
	UINT8 *scroll_y;
	UINT8 *palette_bank;
	UINT8 *flipscreen_x;
	UINT8 *flipscreen_y;
	tilemap_t *bg_tilemap;
	tilemap_t *fg_tilemap;
};


/*----------- defined in video/tp84.c -----------*/

WRITE8_HANDLER( tp84_spriteram_w );
READ8_HANDLER( tp84_scanline_r );

PALETTE_INIT( tp84 );
VIDEO_START( tp84 );
SCREEN_UPDATE( tp84 );
