class punchout_state : public driver_device
{
public:
	punchout_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag) ,
		m_bg_top_videoram(*this, "bg_top_videoram"),
		m_spr1_ctrlram(*this, "spr1_ctrlram"),
		m_spr2_ctrlram(*this, "spr2_ctrlram"),
		m_palettebank(*this, "palettebank"),
		m_spr1_videoram(*this, "spr1_videoram"),
		m_spr2_videoram(*this, "spr2_videoram"),
		m_bg_bot_videoram(*this, "bg_bot_videoram"),
		m_armwrest_fg_videoram(*this, "armwrest_fgram"){ }

	int m_rp5c01_mode_sel;
	int m_rp5c01_mem[16*4];
	required_shared_ptr<UINT8> m_bg_top_videoram;
	required_shared_ptr<UINT8> m_spr1_ctrlram;
	required_shared_ptr<UINT8> m_spr2_ctrlram;
	required_shared_ptr<UINT8> m_palettebank;
	required_shared_ptr<UINT8> m_spr1_videoram;
	required_shared_ptr<UINT8> m_spr2_videoram;
	required_shared_ptr<UINT8> m_bg_bot_videoram;
	optional_shared_ptr<UINT8> m_armwrest_fg_videoram;
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
	DECLARE_CUSTOM_INPUT_MEMBER(punchout_vlm5030_busy_r);
	DECLARE_WRITE8_MEMBER(punchout_speech_reset_w);
	DECLARE_WRITE8_MEMBER(punchout_speech_st_w);
	DECLARE_WRITE8_MEMBER(punchout_speech_vcu_w);
	DECLARE_DRIVER_INIT(armwrest);
	DECLARE_DRIVER_INIT(spnchotj);
	DECLARE_DRIVER_INIT(punchout);
	DECLARE_DRIVER_INIT(spnchout);
	TILE_GET_INFO_MEMBER(top_get_info);
	TILE_GET_INFO_MEMBER(armwrest_top_get_info);
	TILE_GET_INFO_MEMBER(bot_get_info);
	TILE_GET_INFO_MEMBER(armwrest_bot_get_info);
	TILE_GET_INFO_MEMBER(bs1_get_info);
	TILE_GET_INFO_MEMBER(bs2_get_info);
	TILE_GET_INFO_MEMBER(armwrest_fg_get_info);
	TILEMAP_MAPPER_MEMBER(armwrest_bs1_scan);
	TILEMAP_MAPPER_MEMBER(armwrest_bs1_scan_flipx);
	virtual void machine_reset();
	virtual void video_start();
	DECLARE_VIDEO_START(armwrest);
	UINT32 screen_update_punchout_top(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	UINT32 screen_update_punchout_bottom(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	UINT32 screen_update_armwrest_top(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	UINT32 screen_update_armwrest_bottom(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
};
