class m62_state : public driver_device
{
public:
	m62_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag) { }

	/* memory pointers */
	UINT8 *              m_spriteram;
	size_t               m_spriteram_size;

	UINT8 *              m_m62_tileram;
	UINT8 *              m_m62_textram;
	UINT8 *              m_scrollram;

	/* video-related */
	tilemap_t*             m_bg_tilemap;
	tilemap_t*             m_fg_tilemap;
	int                  m_flipscreen;

	const UINT8          *m_sprite_height_prom;
	INT32                m_m62_background_hscroll;
	INT32                m_m62_background_vscroll;
	UINT8                m_kidniki_background_bank;
	INT32                m_kidniki_text_vscroll;
	int                  m_ldrun3_topbottom_mask;
	INT32                m_spelunkr_palbank;

	/* misc */
	int                 m_ldrun2_bankswap;	//ldrun2
	int                 m_bankcontrol[2];	//ldrun2
	DECLARE_READ8_MEMBER(ldrun2_bankswitch_r);
	DECLARE_WRITE8_MEMBER(ldrun2_bankswitch_w);
	DECLARE_READ8_MEMBER(ldrun3_prot_5_r);
	DECLARE_READ8_MEMBER(ldrun3_prot_7_r);
	DECLARE_WRITE8_MEMBER(ldrun4_bankswitch_w);
	DECLARE_WRITE8_MEMBER(kidniki_bankswitch_w);
	DECLARE_WRITE8_MEMBER(spelunkr_bankswitch_w);
	DECLARE_WRITE8_MEMBER(spelunk2_bankswitch_w);
	DECLARE_WRITE8_MEMBER(youjyudn_bankswitch_w);
	DECLARE_WRITE8_MEMBER(m62_flipscreen_w);
	DECLARE_WRITE8_MEMBER(m62_hscroll_low_w);
	DECLARE_WRITE8_MEMBER(m62_hscroll_high_w);
	DECLARE_WRITE8_MEMBER(m62_vscroll_low_w);
	DECLARE_WRITE8_MEMBER(m62_vscroll_high_w);
	DECLARE_WRITE8_MEMBER(m62_tileram_w);
	DECLARE_WRITE8_MEMBER(m62_textram_w);
	DECLARE_WRITE8_MEMBER(kungfum_tileram_w);
	DECLARE_WRITE8_MEMBER(ldrun3_topbottom_mask_w);
	DECLARE_WRITE8_MEMBER(kidniki_text_vscroll_low_w);
	DECLARE_WRITE8_MEMBER(kidniki_text_vscroll_high_w);
	DECLARE_WRITE8_MEMBER(kidniki_background_bank_w);
	DECLARE_WRITE8_MEMBER(spelunkr_palbank_w);
	DECLARE_WRITE8_MEMBER(spelunk2_gfxport_w);
	DECLARE_WRITE8_MEMBER(horizon_scrollram_w);
};


/*----------- defined in video/m62.c -----------*/



PALETTE_INIT( m62 );
PALETTE_INIT( lotlot );
PALETTE_INIT( battroad );
PALETTE_INIT( spelunk2 );

VIDEO_START( battroad );
VIDEO_START( horizon );
VIDEO_START( kidniki );
VIDEO_START( kungfum );
VIDEO_START( ldrun );
VIDEO_START( ldrun2 );
VIDEO_START( ldrun4 );
VIDEO_START( lotlot );
VIDEO_START( spelunkr );
VIDEO_START( spelunk2 );
VIDEO_START( youjyudn );

SCREEN_UPDATE_IND16( battroad );
SCREEN_UPDATE_IND16( horizon );
SCREEN_UPDATE_IND16( kidniki );
SCREEN_UPDATE_IND16( kungfum );
SCREEN_UPDATE_IND16( ldrun );
SCREEN_UPDATE_IND16( ldrun3 );
SCREEN_UPDATE_IND16( ldrun4 );
SCREEN_UPDATE_IND16( lotlot );
SCREEN_UPDATE_IND16( spelunkr );
SCREEN_UPDATE_IND16( spelunk2 );
SCREEN_UPDATE_IND16( youjyudn );
