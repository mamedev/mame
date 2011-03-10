class nova2001_state : public driver_device
{
public:
	nova2001_state(running_machine &machine, const driver_device_config_base &config)
		: driver_device(machine, config) { }

	UINT8 ninjakun_io_a002_ctrl;
	UINT8 *fg_videoram;
	UINT8 *bg_videoram;
	tilemap_t *bg_tilemap;
	tilemap_t *fg_tilemap;
};


/*----------- defined in video/nova2001.c -----------*/

extern WRITE8_HANDLER( nova2001_fg_videoram_w );
extern WRITE8_HANDLER( nova2001_bg_videoram_w );
extern WRITE8_HANDLER( ninjakun_bg_videoram_w );
extern READ8_HANDLER( ninjakun_bg_videoram_r );
extern WRITE8_HANDLER( nova2001_scroll_x_w );
extern WRITE8_HANDLER( nova2001_scroll_y_w );
extern WRITE8_HANDLER( nova2001_flipscreen_w );
extern WRITE8_HANDLER( pkunwar_flipscreen_w );
extern WRITE8_HANDLER( ninjakun_paletteram_w );

extern PALETTE_INIT( nova2001 );
extern VIDEO_START( nova2001 );
extern SCREEN_UPDATE( nova2001 );
extern VIDEO_START( ninjakun );
extern SCREEN_UPDATE( ninjakun );
extern VIDEO_START( pkunwar );
extern SCREEN_UPDATE( pkunwar );
extern VIDEO_START( raiders5 );
extern SCREEN_UPDATE( raiders5 );
