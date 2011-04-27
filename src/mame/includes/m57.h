class m57_state : public driver_device
{
public:
	m57_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag) { }

	/* memory pointers */
	UINT8 *              m_videoram;
	UINT8 *              m_spriteram;
	size_t               m_spriteram_size;

	UINT8 *              m_scrollram;

	/* video-related */
	tilemap_t*             m_bg_tilemap;
	int                  m_flipscreen;
};

/*----------- defined in video/m57.c -----------*/

WRITE8_HANDLER( m57_videoram_w );
WRITE8_HANDLER( m57_flipscreen_w );

PALETTE_INIT( m57 );
VIDEO_START( m57 );
SCREEN_UPDATE( m57 );
