class m57_state : public driver_device
{
public:
	m57_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag) ,
		m_videoram(*this, "videoram"),
		m_scrollram(*this, "scrollram"),
		m_spriteram(*this, "spriteram"){ }

	/* memory pointers */
	required_shared_ptr<UINT8> m_videoram;
	required_shared_ptr<UINT8> m_scrollram;
	required_shared_ptr<UINT8> m_spriteram;

	/* video-related */
	tilemap_t*             m_bg_tilemap;
	int                  m_flipscreen;
	DECLARE_WRITE8_MEMBER(m57_videoram_w);
	DECLARE_WRITE8_MEMBER(m57_flipscreen_w);
};

/*----------- defined in video/m57.c -----------*/


PALETTE_INIT( m57 );
VIDEO_START( m57 );
SCREEN_UPDATE_IND16( m57 );
