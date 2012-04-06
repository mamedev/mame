/*************************************************************************

    Street Fighter

*************************************************************************/

class sf_state : public driver_device
{
public:
	sf_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag) { }

	/* memory pointers */
	UINT16 *    m_videoram;
	UINT16 *    m_objectram;
//  UINT16 *    m_paletteram;    // currently this uses generic palette handling
	size_t      m_videoram_size;

	/* video-related */
	tilemap_t     *m_bg_tilemap;
	tilemap_t     *m_fg_tilemap;
	tilemap_t     *m_tx_tilemap;
	int         m_sf_active;
	UINT16      m_bgscroll;
	UINT16      m_fgscroll;

	/* devices */
	device_t *m_maincpu;
	device_t *m_audiocpu;
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
};


/*----------- defined in video/sf.c -----------*/


VIDEO_START( sf );
SCREEN_UPDATE_IND16( sf );
