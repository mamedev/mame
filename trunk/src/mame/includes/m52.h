class m52_state : public driver_device
{
public:
	m52_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag) { }

	/* memory pointers */
	UINT8 *              m_videoram;
	UINT8 *              m_spriteram;
	size_t               m_spriteram_size;

	UINT8 *              m_colorram;

	/* video-related */
	tilemap_t*             m_bg_tilemap;
	UINT8                m_bg1xpos;
	UINT8                m_bg1ypos;
	UINT8                m_bg2xpos;
	UINT8                m_bg2ypos;
	UINT8                m_bgcontrol;
};

/*----------- defined in video/m52.c -----------*/

READ8_HANDLER( m52_protection_r );
WRITE8_HANDLER( m52_scroll_w );
WRITE8_HANDLER( m52_bg1xpos_w );
WRITE8_HANDLER( m52_bg1ypos_w );
WRITE8_HANDLER( m52_bg2xpos_w );
WRITE8_HANDLER( m52_bg2ypos_w );
WRITE8_HANDLER( m52_bgcontrol_w );
WRITE8_HANDLER( m52_flipscreen_w );
WRITE8_HANDLER( alpha1v_flipscreen_w );
WRITE8_HANDLER( m52_videoram_w );
WRITE8_HANDLER( m52_colorram_w );

PALETTE_INIT( m52 );
VIDEO_START( m52 );
SCREEN_UPDATE( m52 );
