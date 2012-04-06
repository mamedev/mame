/*************************************************************************

    Momoko 120%

*************************************************************************/

class momoko_state : public driver_device
{
public:
	momoko_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag) { }

	/* memory pointers */
	UINT8 *        m_bg_scrollx;
	UINT8 *        m_bg_scrolly;
	UINT8 *        m_videoram;
	UINT8 *        m_spriteram;
//  UINT8 *        paletteram;    // currently this uses generic palette handling
	size_t         m_spriteram_size;
	size_t         m_videoram_size;

	/* video-related */
	UINT8          m_fg_scrollx;
	UINT8          m_fg_scrolly;
	UINT8          m_fg_select;
	UINT8          m_text_scrolly;
	UINT8          m_text_mode;
	UINT8          m_bg_select;
	UINT8          m_bg_priority;
	UINT8          m_bg_mask;
	UINT8          m_fg_mask;
	UINT8          m_flipscreen;
	DECLARE_WRITE8_MEMBER(momoko_bg_read_bank_w);
	DECLARE_WRITE8_MEMBER(momoko_fg_scrollx_w);
	DECLARE_WRITE8_MEMBER(momoko_fg_scrolly_w);
	DECLARE_WRITE8_MEMBER(momoko_fg_select_w);
	DECLARE_WRITE8_MEMBER(momoko_text_scrolly_w);
	DECLARE_WRITE8_MEMBER(momoko_text_mode_w);
	DECLARE_WRITE8_MEMBER(momoko_bg_scrollx_w);
	DECLARE_WRITE8_MEMBER(momoko_bg_scrolly_w);
	DECLARE_WRITE8_MEMBER(momoko_bg_select_w);
	DECLARE_WRITE8_MEMBER(momoko_bg_priority_w);
	DECLARE_WRITE8_MEMBER(momoko_flipscreen_w);
};


/*----------- defined in video/momoko.c -----------*/


SCREEN_UPDATE_IND16( momoko );
