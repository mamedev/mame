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
	DECLARE_WRITE8_MEMBER(m52_scroll_w);
	DECLARE_WRITE8_MEMBER(m52_videoram_w);
	DECLARE_WRITE8_MEMBER(m52_colorram_w);
	DECLARE_READ8_MEMBER(m52_protection_r);
	DECLARE_WRITE8_MEMBER(m52_bg1ypos_w);
	DECLARE_WRITE8_MEMBER(m52_bg1xpos_w);
	DECLARE_WRITE8_MEMBER(m52_bg2xpos_w);
	DECLARE_WRITE8_MEMBER(m52_bg2ypos_w);
	DECLARE_WRITE8_MEMBER(m52_bgcontrol_w);
	DECLARE_WRITE8_MEMBER(m52_flipscreen_w);
	DECLARE_WRITE8_MEMBER(alpha1v_flipscreen_w);
};

/*----------- defined in video/m52.c -----------*/


PALETTE_INIT( m52 );
VIDEO_START( m52 );
SCREEN_UPDATE_IND16( m52 );
