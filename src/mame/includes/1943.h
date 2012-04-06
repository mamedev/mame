/***************************************************************************

    1943

***************************************************************************/

class _1943_state : public driver_device
{
public:
	_1943_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag) { }

	/* memory pointers */
	UINT8 * m_videoram;
	UINT8 * m_colorram;
	UINT8 * m_spriteram;
	UINT8 * m_scrollx;
	UINT8 * m_scrolly;
	UINT8 * m_bgscrollx;
	size_t  m_spriteram_size;

	/* video-related */
	tilemap_t *m_fg_tilemap;
	tilemap_t *m_bg_tilemap;
	tilemap_t *m_bg2_tilemap;
	int     m_char_on;
	int     m_obj_on;
	int     m_bg1_on;
	int     m_bg2_on;
	DECLARE_READ8_MEMBER(c1943_protection_r);
	DECLARE_READ8_MEMBER(_1943b_c007_r);
	DECLARE_WRITE8_MEMBER(c1943_videoram_w);
	DECLARE_WRITE8_MEMBER(c1943_colorram_w);
	DECLARE_WRITE8_MEMBER(c1943_c804_w);
	DECLARE_WRITE8_MEMBER(c1943_d806_w);
};



/*----------- defined in video/1943.c -----------*/


extern PALETTE_INIT( 1943 );
extern VIDEO_START( 1943 );
extern SCREEN_UPDATE_IND16( 1943 );
