/*************************************************************************

    Street Fighter

*************************************************************************/

class sf_state : public driver_device
{
public:
	sf_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag) ,
		m_videoram(*this, "videoram"),
		m_objectram(*this, "objectram"){ }

	/* memory pointers */
	required_shared_ptr<UINT16> m_videoram;
	required_shared_ptr<UINT16> m_objectram;
//  UINT16 *    m_paletteram;    // currently this uses generic palette handling

	/* video-related */
	tilemap_t     *m_bg_tilemap;
	tilemap_t     *m_fg_tilemap;
	tilemap_t     *m_tx_tilemap;
	int         m_sf_active;
	UINT16      m_bgscroll;
	UINT16      m_fgscroll;

	/* devices */
	cpu_device *m_maincpu;
	cpu_device *m_audiocpu;
	DECLARE_READ16_MEMBER(dummy_r);
	DECLARE_WRITE16_MEMBER(sf_coin_w);
	DECLARE_WRITE16_MEMBER(soundcmd_w);
	DECLARE_WRITE16_MEMBER(protection_w);
	DECLARE_READ16_MEMBER(button1_r);
	DECLARE_READ16_MEMBER(button2_r);
	DECLARE_WRITE8_MEMBER(sound2_bank_w);
	DECLARE_WRITE16_MEMBER(sf_videoram_w);
	DECLARE_WRITE16_MEMBER(sf_bg_scroll_w);
	DECLARE_WRITE16_MEMBER(sf_fg_scroll_w);
	DECLARE_WRITE16_MEMBER(sf_gfxctrl_w);
	DECLARE_WRITE8_MEMBER(msm1_5205_w);
	DECLARE_WRITE8_MEMBER(msm2_5205_w);
	TILE_GET_INFO_MEMBER(get_bg_tile_info);
	TILE_GET_INFO_MEMBER(get_fg_tile_info);
	TILE_GET_INFO_MEMBER(get_tx_tile_info);
	virtual void machine_start();
	virtual void machine_reset();
	virtual void video_start();
};


/*----------- defined in video/sf.c -----------*/



SCREEN_UPDATE_IND16( sf );
