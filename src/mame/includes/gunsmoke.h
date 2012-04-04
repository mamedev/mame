/*************************************************************************

    Gun.Smoke

*************************************************************************/

class gunsmoke_state : public driver_device
{
public:
	gunsmoke_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag) { }

	/* memory pointers */
	UINT8 *    m_videoram;
	UINT8 *    m_colorram;
	UINT8 *    m_spriteram;
	UINT8 *    m_scrollx;
	UINT8 *    m_scrolly;
	size_t     m_spriteram_size;

	/* video-related */
	tilemap_t    *m_bg_tilemap;
	tilemap_t    *m_fg_tilemap;
	UINT8      m_chon;
	UINT8      m_objon;
	UINT8      m_bgon;
	UINT8      m_sprite3bank;
	DECLARE_READ8_MEMBER(gunsmoke_protection_r);
};


/*----------- defined in video/gunsmoke.c -----------*/

WRITE8_HANDLER( gunsmoke_c804_w );
WRITE8_HANDLER( gunsmoke_d806_w );
WRITE8_HANDLER( gunsmoke_videoram_w );
WRITE8_HANDLER( gunsmoke_colorram_w );

PALETTE_INIT( gunsmoke );
VIDEO_START( gunsmoke );
SCREEN_UPDATE_IND16( gunsmoke );

