/*************************************************************************

    Goindol

*************************************************************************/

class goindol_state : public driver_device
{
public:
	goindol_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag) { }

	/* memory pointers */
	UINT8 *    m_bg_videoram;
	UINT8 *    m_fg_videoram;
	UINT8 *    m_fg_scrollx;
	UINT8 *    m_fg_scrolly;
	UINT8 *    m_ram;
	UINT8 *    m_spriteram;
	UINT8 *    m_spriteram2;
	size_t     m_fg_videoram_size;
	size_t     m_bg_videoram_size;
	size_t     m_spriteram_size;

	/* video-related */
	tilemap_t     *m_bg_tilemap;
	tilemap_t     *m_fg_tilemap;
	UINT16      m_char_bank;

	/* misc */
	int         m_prot_toggle;
	DECLARE_WRITE8_MEMBER(goindol_bankswitch_w);
	DECLARE_READ8_MEMBER(prot_f422_r);
	DECLARE_WRITE8_MEMBER(prot_fc44_w);
	DECLARE_WRITE8_MEMBER(prot_fd99_w);
	DECLARE_WRITE8_MEMBER(prot_fc66_w);
	DECLARE_WRITE8_MEMBER(prot_fcb0_w);
	DECLARE_WRITE8_MEMBER(goindol_fg_videoram_w);
	DECLARE_WRITE8_MEMBER(goindol_bg_videoram_w);
};



/*----------- defined in video/goindol.c -----------*/


VIDEO_START( goindol );
SCREEN_UPDATE_IND16( goindol );
