/*************************************************************************

    World Grand Prix

*************************************************************************/

class wgp_state : public driver_device
{
public:
	wgp_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag) { }

	/* memory pointers */
	UINT16 *    m_spritemap;
	UINT16 *    m_spriteram;
	UINT16 *    m_pivram;
	UINT16 *    m_piv_ctrlram;
	UINT16 *    m_sharedram;
//  UINT16 *    m_paletteram;    // currently this uses generic palette handling
	size_t      m_sharedram_size;
	size_t      m_spritemap_size;
	size_t      m_spriteram_size;

	/* video-related */
	tilemap_t   *m_piv_tilemap[3];
	UINT16      m_piv_ctrl_reg;
	UINT16      m_piv_zoom[3];
	UINT16      m_piv_scrollx[3];
	UINT16      m_piv_scrolly[3];
	UINT16      m_rotate_ctrl[8];
	int         m_piv_xoffs;
	int         m_piv_yoffs;
	UINT8       m_dislayer[4];

	/* misc */
	UINT16      m_cpua_ctrl;
	UINT16      m_port_sel;
	INT32       m_banknum;

	/* devices */
	device_t *m_maincpu;
	device_t *m_audiocpu;
	device_t *m_subcpu;
	device_t *m_tc0100scn;
	device_t *m_tc0140syt;
	DECLARE_READ16_MEMBER(sharedram_r);
	DECLARE_WRITE16_MEMBER(sharedram_w);
	DECLARE_WRITE16_MEMBER(cpua_ctrl_w);
	DECLARE_READ16_MEMBER(lan_status_r);
	DECLARE_WRITE16_MEMBER(rotate_port_w);
	DECLARE_READ16_MEMBER(wgp_adinput_r);
	DECLARE_WRITE16_MEMBER(wgp_adinput_w);
	DECLARE_WRITE8_MEMBER(sound_bankswitch_w);
	DECLARE_WRITE16_MEMBER(wgp_sound_w);
	DECLARE_READ16_MEMBER(wgp_sound_r);
	DECLARE_READ16_MEMBER(wgp_pivram_word_r);
	DECLARE_WRITE16_MEMBER(wgp_pivram_word_w);
	DECLARE_READ16_MEMBER(wgp_piv_ctrl_word_r);
	DECLARE_WRITE16_MEMBER(wgp_piv_ctrl_word_w);
};


/*----------- defined in video/wgp.c -----------*/



VIDEO_START( wgp );
VIDEO_START( wgp2 );
SCREEN_UPDATE_IND16( wgp );
