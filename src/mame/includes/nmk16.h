class nmk16_state : public driver_device
{
public:
	nmk16_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag) ,
		m_nmk_bgvideoram0(*this, "nmk_bgvideoram0"),
		m_nmk_txvideoram(*this, "nmk_txvideoram"),
		m_mainram(*this, "mainram"),
		m_gunnail_scrollram(*this, "scrollram"),
		m_spriteram(*this, "spriteram"),
		m_nmk_fgvideoram(*this, "nmk_fgvideoram"),
		m_gunnail_scrollramy(*this, "scrollramy"),
		m_nmk_bgvideoram1(*this, "nmk_bgvideoram1"),
		m_nmk_bgvideoram2(*this, "nmk_bgvideoram2"),
		m_nmk_bgvideoram3(*this, "nmk_bgvideoram3"),
		m_afega_scroll_0(*this, "afega_scroll_0"),
		m_afega_scroll_1(*this, "afega_scroll_1"){ }

	int mask[4*2];
	required_shared_ptr<UINT16> m_nmk_bgvideoram0;
	optional_shared_ptr<UINT16> m_nmk_txvideoram;
	required_shared_ptr<UINT16> m_mainram;
	optional_shared_ptr<UINT16> m_gunnail_scrollram;
	optional_shared_ptr<UINT8> m_spriteram;
	optional_shared_ptr<UINT16> m_nmk_fgvideoram;
	optional_shared_ptr<UINT16> m_gunnail_scrollramy;
	optional_shared_ptr<UINT16> m_nmk_bgvideoram1;
	optional_shared_ptr<UINT16> m_nmk_bgvideoram2;
	optional_shared_ptr<UINT16> m_nmk_bgvideoram3;
	optional_shared_ptr<UINT16> m_afega_scroll_0;
	optional_shared_ptr<UINT16> m_afega_scroll_1;
	int m_simple_scroll;
	int m_redraw_bitmap;
	UINT16 *m_spriteram_old;
	UINT16 *m_spriteram_old2;
	int m_bgbank;
	int m_videoshift;
	int m_bioship_background_bank;
	UINT8 m_bioship_scroll[4];
	tilemap_t *m_bg_tilemap0;
	tilemap_t *m_bg_tilemap1;
	tilemap_t *m_bg_tilemap2;
	tilemap_t *m_bg_tilemap3;
	tilemap_t *m_tx_tilemap;
	tilemap_t *m_fg_tilemap;
	bitmap_ind16 *m_background_bitmap;
	int m_mustang_bg_xscroll;
	UINT8 m_scroll[4];
	UINT8 m_scroll_2[4];
	UINT16 m_vscroll[4];
	int m_prot_count;
	UINT8 m_input_pressed;
	UINT8 m_start_helper;
	UINT8 m_coin_count[2];
	UINT8 m_coin_count_frac[2];
	DECLARE_WRITE16_MEMBER(nmk16_mainram_strange_w);
	DECLARE_WRITE16_MEMBER(ssmissin_sound_w);
	DECLARE_WRITE8_MEMBER(ssmissin_soundbank_w);
	DECLARE_WRITE16_MEMBER(tharrier_mcu_control_w);
	DECLARE_READ16_MEMBER(tharrier_mcu_r);
	DECLARE_WRITE16_MEMBER(macross2_sound_reset_w);
	DECLARE_WRITE16_MEMBER(macross2_sound_command_w);
	DECLARE_WRITE8_MEMBER(macross2_sound_bank_w);
	DECLARE_WRITE8_MEMBER(tharrier_oki6295_bankswitch_0_w);
	DECLARE_WRITE8_MEMBER(tharrier_oki6295_bankswitch_1_w);
	DECLARE_WRITE16_MEMBER(afega_soundlatch_w);
	DECLARE_READ16_MEMBER(mcu_shared_r);
	DECLARE_WRITE16_MEMBER(hachamf_mainram_w);
	DECLARE_WRITE16_MEMBER(tdragon_mainram_w);
	DECLARE_WRITE8_MEMBER(okibank_w);
	DECLARE_WRITE8_MEMBER(raphero_sound_rombank_w);
	DECLARE_READ16_MEMBER(vandykeb_r);
	DECLARE_READ16_MEMBER(afega_unknown_r);
	DECLARE_WRITE16_MEMBER(afega_scroll0_w);
	DECLARE_WRITE16_MEMBER(afega_scroll1_w);
	DECLARE_WRITE16_MEMBER(twinactn_scroll0_w);
	DECLARE_WRITE16_MEMBER(twinactn_scroll1_w);
	DECLARE_WRITE16_MEMBER(twinactn_flipscreen_w);
	DECLARE_WRITE16_MEMBER(nmk_bgvideoram0_w);
	DECLARE_WRITE16_MEMBER(nmk_bgvideoram1_w);
	DECLARE_WRITE16_MEMBER(nmk_bgvideoram2_w);
	DECLARE_WRITE16_MEMBER(nmk_bgvideoram3_w);
	DECLARE_WRITE16_MEMBER(nmk_fgvideoram_w);
	DECLARE_WRITE16_MEMBER(nmk_txvideoram_w);
	DECLARE_WRITE16_MEMBER(mustang_scroll_w);
	DECLARE_WRITE16_MEMBER(bioshipbg_scroll_w);
	DECLARE_WRITE16_MEMBER(nmk_scroll_w);
	DECLARE_WRITE16_MEMBER(nmk_scroll_2_w);
	DECLARE_WRITE16_MEMBER(vandyke_scroll_w);
	DECLARE_WRITE16_MEMBER(vandykeb_scroll_w);
	DECLARE_WRITE16_MEMBER(manybloc_scroll_w);
	DECLARE_WRITE16_MEMBER(nmk_flipscreen_w);
	DECLARE_WRITE16_MEMBER(nmk_tilebank_w);
	DECLARE_WRITE16_MEMBER(bioship_scroll_w);
	DECLARE_WRITE16_MEMBER(bioship_bank_w);
	DECLARE_WRITE8_MEMBER(spec2k_oki1_banking_w);
	DECLARE_WRITE8_MEMBER(twinactn_oki_bank_w);
	DECLARE_DRIVER_INIT(nmk);
	DECLARE_DRIVER_INIT(vandykeb);
	DECLARE_DRIVER_INIT(tdragonb);
	DECLARE_DRIVER_INIT(ssmissin);
	DECLARE_DRIVER_INIT(hachamf);
	DECLARE_DRIVER_INIT(redhawk);
	DECLARE_DRIVER_INIT(tdragon);
	DECLARE_DRIVER_INIT(bubl2000);
	DECLARE_DRIVER_INIT(grdnstrm);
	DECLARE_DRIVER_INIT(spec2k);
	DECLARE_DRIVER_INIT(bjtwin);
	TILEMAP_MAPPER_MEMBER(afega_tilemap_scan_pages);
	TILE_GET_INFO_MEMBER(macross_get_bg0_tile_info);
	TILE_GET_INFO_MEMBER(macross_get_bg1_tile_info);
	TILE_GET_INFO_MEMBER(macross_get_bg2_tile_info);
	TILE_GET_INFO_MEMBER(macross_get_bg3_tile_info);
	TILE_GET_INFO_MEMBER(strahl_get_fg_tile_info);
	TILE_GET_INFO_MEMBER(macross_get_tx_tile_info);
	TILE_GET_INFO_MEMBER(bjtwin_get_bg_tile_info);
	TILE_GET_INFO_MEMBER(get_tile_info_0_8bit);
	DECLARE_MACHINE_RESET(mustang_sound);
	DECLARE_VIDEO_START(macross);
	DECLARE_MACHINE_RESET(NMK004);
	DECLARE_VIDEO_START(bioship);
	DECLARE_VIDEO_START(strahl);
	DECLARE_VIDEO_START(gunnail);
	DECLARE_VIDEO_START(macross2);
	DECLARE_VIDEO_START(raphero);
	DECLARE_VIDEO_START(bjtwin);
	DECLARE_VIDEO_START(afega);
	DECLARE_VIDEO_START(firehawk);
	DECLARE_VIDEO_START(grdnstrm);
	UINT32 screen_update_tharrier(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	UINT32 screen_update_manybloc(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	UINT32 screen_update_macross(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	UINT32 screen_update_bioship(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	UINT32 screen_update_strahl(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	UINT32 screen_update_gunnail(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	UINT32 screen_update_tdragon2(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	UINT32 screen_update_bjtwin(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	UINT32 screen_update_afega(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	UINT32 screen_update_firehawk(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	UINT32 screen_update_redhawki(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	UINT32 screen_update_redhawkb(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	UINT32 screen_update_bubl2000(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	void screen_eof_nmk(screen_device &screen, bool state);
	void screen_eof_strahl(screen_device &screen, bool state);
};


/*----------- defined in video/nmk16.c -----------*/





























