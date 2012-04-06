class punchout_state : public driver_device
{
public:
	punchout_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag) { }

	int m_rp5c01_mode_sel;
	int m_rp5c01_mem[16*4];
	UINT8 *m_bg_top_videoram;
	UINT8 *m_bg_bot_videoram;
	UINT8 *m_armwrest_fg_videoram;
	UINT8 *m_spr1_videoram;
	UINT8 *m_spr2_videoram;
	UINT8 *m_spr1_ctrlram;
	UINT8 *m_spr2_ctrlram;
	UINT8 *m_palettebank;
	tilemap_t *m_bg_top_tilemap;
	tilemap_t *m_bg_bot_tilemap;
	tilemap_t *m_fg_tilemap;
	tilemap_t *m_spr1_tilemap;
	tilemap_t *m_spr1_tilemap_flipx;
	tilemap_t *m_spr2_tilemap;
	int m_palette_reverse_top;
	int m_palette_reverse_bot;

	UINT8 m_nmi_mask;
	DECLARE_WRITE8_MEMBER(punchout_2a03_reset_w);
	DECLARE_READ8_MEMBER(spunchout_rp5c01_r);
	DECLARE_WRITE8_MEMBER(spunchout_rp5c01_w);
	DECLARE_READ8_MEMBER(spunchout_exp_r);
	DECLARE_WRITE8_MEMBER(spunchout_exp_w);
	DECLARE_WRITE8_MEMBER(nmi_mask_w);
	DECLARE_WRITE8_MEMBER(punchout_bg_top_videoram_w);
	DECLARE_WRITE8_MEMBER(punchout_bg_bot_videoram_w);
	DECLARE_WRITE8_MEMBER(armwrest_fg_videoram_w);
	DECLARE_WRITE8_MEMBER(punchout_spr1_videoram_w);
	DECLARE_WRITE8_MEMBER(punchout_spr2_videoram_w);
};


/*----------- defined in video/punchout.c -----------*/


VIDEO_START( punchout );
VIDEO_START( armwrest );
SCREEN_UPDATE_IND16( punchout_top );
SCREEN_UPDATE_IND16( punchout_bottom );
SCREEN_UPDATE_IND16( armwrest_top );
SCREEN_UPDATE_IND16( armwrest_bottom );

DRIVER_INIT( punchout );
DRIVER_INIT( spnchout );
DRIVER_INIT( spnchotj );
DRIVER_INIT( armwrest );
