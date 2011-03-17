class namcos86_state : public driver_device
{
public:
	namcos86_state(running_machine &machine, const driver_device_config_base &config)
		: driver_device(machine, config) { }

	UINT8 *spriteram;
	int wdog;
	UINT8 *rthunder_videoram1;
	UINT8 *rthunder_videoram2;
	UINT8 *rthunder_spriteram;
	int tilebank;
	int xscroll[4];
	int yscroll[4];
	tilemap_t *bg_tilemap[4];
	int backcolor;
	const UINT8 *tile_address_prom;
	int copy_sprites;
};


/*----------- defined in video/namcos86.c -----------*/

PALETTE_INIT( namcos86 );
VIDEO_START( namcos86 );
SCREEN_UPDATE( namcos86 );
SCREEN_EOF( namcos86 );

READ8_HANDLER( rthunder_videoram1_r );
WRITE8_HANDLER( rthunder_videoram1_w );
READ8_HANDLER( rthunder_videoram2_r );
WRITE8_HANDLER( rthunder_videoram2_w );
WRITE8_HANDLER( rthunder_scroll0_w );
WRITE8_HANDLER( rthunder_scroll1_w );
WRITE8_HANDLER( rthunder_scroll2_w );
WRITE8_HANDLER( rthunder_scroll3_w );
WRITE8_HANDLER( rthunder_backcolor_w );
WRITE8_HANDLER( rthunder_tilebank_select_w );
READ8_HANDLER( rthunder_spriteram_r );
WRITE8_HANDLER( rthunder_spriteram_w );
