/*************************************************************************

    Capcom Baseball

*************************************************************************/

class cbasebal_state : public driver_device
{
public:
	cbasebal_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag) { }

	/* memory pointers */
	UINT8 *    m_spriteram;
//  UINT8 *    m_paletteram;    // currently this uses generic palette handling
	size_t     m_spriteram_size;

	/* video-related */
	tilemap_t    *m_fg_tilemap;
	tilemap_t    *m_bg_tilemap;
	UINT8      *m_textram;
	UINT8      *m_scrollram;
	UINT8      m_scroll_x[2];
	UINT8      m_scroll_y[2];
	int        m_tilebank;
	int        m_spritebank;
	int        m_text_on;
	int        m_bg_on;
	int        m_obj_on;
	int        m_flipscreen;

	/* misc */
	UINT8      m_rambank;
	DECLARE_WRITE8_MEMBER(cbasebal_bankswitch_w);
	DECLARE_READ8_MEMBER(bankedram_r);
	DECLARE_WRITE8_MEMBER(bankedram_w);
	DECLARE_WRITE8_MEMBER(cbasebal_coinctrl_w);
};

/*----------- defined in video/cbasebal.c -----------*/

WRITE8_HANDLER( cbasebal_textram_w );
READ8_HANDLER( cbasebal_textram_r );
WRITE8_HANDLER( cbasebal_scrollram_w );
READ8_HANDLER( cbasebal_scrollram_r );
WRITE8_HANDLER( cbasebal_gfxctrl_w );
WRITE8_HANDLER( cbasebal_scrollx_w );
WRITE8_HANDLER( cbasebal_scrolly_w );

VIDEO_START( cbasebal );
SCREEN_UPDATE_IND16( cbasebal );
