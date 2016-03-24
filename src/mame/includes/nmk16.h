// license:BSD-3-Clause
// copyright-holders:Mirko Buffoni,Nicola Salmoria,Bryan McPhail,David Haywood,R. Belmont,Alex Marshall,Angelo Salese,Luca Elia
// thanks-to:Richard Bush
#include "machine/nmk112.h"
#include "sound/okim6295.h"
#include "machine/nmk004.h"

class nmk16_state : public driver_device
{
public:
	nmk16_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_audiocpu(*this, "audiocpu"),
		m_oki1(*this, "oki1"),
		m_oki2(*this, "oki2"),
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
		m_afega_scroll_1(*this, "afega_scroll_1"),
		m_gfxdecode(*this, "gfxdecode"),
		m_palette(*this, "palette"),
		m_nmk004(*this, "nmk004"),
		m_sprdma_base(0x8000)
	{}

	required_device<cpu_device> m_maincpu;
	optional_device<cpu_device> m_audiocpu;
	optional_device<okim6295_device> m_oki1;
	optional_device<okim6295_device> m_oki2;
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
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<palette_device> m_palette;
	optional_device<nmk004_device> m_nmk004;
	int m_sprdma_base;
	int mask[4*2];
	int m_simple_scroll;
	int m_redraw_bitmap;
	std::unique_ptr<UINT16[]> m_spriteram_old;
	std::unique_ptr<UINT16[]> m_spriteram_old2;
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
	std::unique_ptr<bitmap_ind16> m_background_bitmap;
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
	DECLARE_WRITE16_MEMBER(hachamf_mainram_w);
	DECLARE_WRITE16_MEMBER(tdragon_mainram_w);
	DECLARE_READ16_MEMBER(vandykeb_r);
	DECLARE_READ16_MEMBER(tdragonb_prot_r);
	DECLARE_READ16_MEMBER(afega_unknown_r);
	DECLARE_WRITE16_MEMBER(afega_scroll0_w);
	DECLARE_WRITE16_MEMBER(afega_scroll1_w);
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
	DECLARE_READ16_MEMBER(atombjt_unkr_r) {return 0x0000;}
	DECLARE_WRITE16_MEMBER(nmk16_x0016_w);
	DECLARE_WRITE16_MEMBER(nmk16_bioship_x0016_w);
	DECLARE_DRIVER_INIT(nmk);
	DECLARE_DRIVER_INIT(tharrier);
	DECLARE_DRIVER_INIT(vandykeb);
	DECLARE_DRIVER_INIT(tdragonb);
	DECLARE_DRIVER_INIT(ssmissin);
	DECLARE_DRIVER_INIT(hachamf_prot);
	DECLARE_DRIVER_INIT(redhawk);
	DECLARE_DRIVER_INIT(tdragon_prot);
	DECLARE_DRIVER_INIT(bubl2000);
	DECLARE_DRIVER_INIT(banked_audiocpu);
	DECLARE_DRIVER_INIT(grdnstrm);
	DECLARE_DRIVER_INIT(spec2k);
	DECLARE_DRIVER_INIT(redfoxwp2a);
	DECLARE_DRIVER_INIT(grdnstrmg);
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
	DECLARE_VIDEO_START(macross);
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
	TIMER_DEVICE_CALLBACK_MEMBER(tdragon_mcu_sim);
	TIMER_DEVICE_CALLBACK_MEMBER(hachamf_mcu_sim);
	TIMER_DEVICE_CALLBACK_MEMBER(nmk16_scanline);
	TIMER_DEVICE_CALLBACK_MEMBER(manybloc_scanline);
	void nmk16_video_init();
	inline void nmk16_draw_sprite(bitmap_ind16 &bitmap, const rectangle &cliprect, UINT16 *spr);
	inline void nmk16_draw_sprite_flipsupported(bitmap_ind16 &bitmap, const rectangle &cliprect, UINT16 *spr);
	void nmk16_draw_sprites_swap(bitmap_ind16 &bitmap, const rectangle &cliprect, int *bittbl);
	void nmk16_draw_sprites_swap_flipsupported(bitmap_ind16 &bitmap, const rectangle &cliprect, int *bittbl);
	void nmk16_draw_sprites(bitmap_ind16 &bitmap, const rectangle &cliprect);
	void nmk16_draw_sprites_flipsupported(bitmap_ind16 &bitmap, const rectangle &cliprect);
	int nmk16_bg_spr_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	int nmk16_bg_fg_spr_tx_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	int nmk16_bg_spr_tx_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	int nmk16_bg_sprflip_tx_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	int nmk16_bioshipbg_sprflip_tx_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	int nmk16_bg_sprswap_tx_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect, int bittbl[8]);
	int nmk16_bg_sprswapflip_tx_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect, int bittbl[8]);
	int nmk16_complexbg_sprswap_tx_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect, int bittbl[8]);
	void video_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect,int dsw_flipscreen,int xoffset, int yoffset,int attr_mask);
	void redhawki_video_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect );
	void mcu_run(UINT8 dsw_setting);
	UINT8 decode_byte(UINT8 src, const UINT8 *bitp);
	UINT32 bjtwin_address_map_bg0(UINT32 addr);
	UINT16 decode_word(UINT16 src, const UINT8 *bitp);
	UINT32 bjtwin_address_map_sprites(UINT32 addr);
	void decode_gfx();
	void decode_tdragonb();
	void decode_ssmissin();
};
