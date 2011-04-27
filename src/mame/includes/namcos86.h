class namcos86_state : public driver_device
{
public:
	namcos86_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag) { }

	UINT8 *m_spriteram;
	int m_wdog;
	UINT8 *m_rthunder_videoram1;
	UINT8 *m_rthunder_videoram2;
	UINT8 *m_rthunder_spriteram;
	int m_tilebank;
	int m_xscroll[4];
	int m_yscroll[4];
	tilemap_t *m_bg_tilemap[4];
	int m_backcolor;
	const UINT8 *m_tile_address_prom;
	int m_copy_sprites;
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
