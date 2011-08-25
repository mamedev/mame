class nova2001_state : public driver_device
{
public:
	nova2001_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag) { }

	UINT8 m_ninjakun_io_a002_ctrl;
	UINT8 *m_fg_videoram;
	UINT8 *m_bg_videoram;
	tilemap_t *m_bg_tilemap;
	tilemap_t *m_fg_tilemap;
	UINT8 *m_spriteram;
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
