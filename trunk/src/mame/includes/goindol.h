/*************************************************************************

    Goindol

*************************************************************************/

class goindol_state : public driver_device
{
public:
	goindol_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag) ,
		m_ram(*this, "ram"),
		m_fg_scrolly(*this, "fg_scrolly"),
		m_fg_scrollx(*this, "fg_scrollx"),
		m_spriteram(*this, "spriteram"),
		m_bg_videoram(*this, "bg_videoram"),
		m_spriteram2(*this, "spriteram2"),
		m_fg_videoram(*this, "fg_videoram"){ }

	/* memory pointers */
	required_shared_ptr<UINT8> m_ram;
	required_shared_ptr<UINT8> m_fg_scrolly;
	required_shared_ptr<UINT8> m_fg_scrollx;
	required_shared_ptr<UINT8> m_spriteram;
	required_shared_ptr<UINT8> m_bg_videoram;
	required_shared_ptr<UINT8> m_spriteram2;
	required_shared_ptr<UINT8> m_fg_videoram;

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
