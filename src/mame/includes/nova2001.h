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
	DECLARE_WRITE8_MEMBER(ninjakun_cpu1_io_A002_w);
	DECLARE_WRITE8_MEMBER(ninjakun_cpu2_io_A002_w);
	DECLARE_WRITE8_MEMBER(ninjakun_paletteram_w);
	DECLARE_WRITE8_MEMBER(nova2001_fg_videoram_w);
	DECLARE_WRITE8_MEMBER(nova2001_bg_videoram_w);
	DECLARE_WRITE8_MEMBER(ninjakun_bg_videoram_w);
	DECLARE_READ8_MEMBER(ninjakun_bg_videoram_r);
	DECLARE_WRITE8_MEMBER(nova2001_scroll_x_w);
	DECLARE_WRITE8_MEMBER(nova2001_scroll_y_w);
	DECLARE_WRITE8_MEMBER(nova2001_flipscreen_w);
	DECLARE_WRITE8_MEMBER(pkunwar_flipscreen_w);
};


/*----------- defined in video/nova2001.c -----------*/


extern PALETTE_INIT( nova2001 );
extern VIDEO_START( nova2001 );
extern SCREEN_UPDATE_IND16( nova2001 );
extern VIDEO_START( ninjakun );
extern SCREEN_UPDATE_IND16( ninjakun );
extern VIDEO_START( pkunwar );
extern SCREEN_UPDATE_IND16( pkunwar );
extern VIDEO_START( raiders5 );
extern SCREEN_UPDATE_IND16( raiders5 );
