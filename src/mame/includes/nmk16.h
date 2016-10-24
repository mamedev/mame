// license:BSD-3-Clause
// copyright-holders:Mirko Buffoni,Nicola Salmoria,Bryan McPhail,David Haywood,R. Belmont,Alex Marshall,Angelo Salese,Luca Elia
// thanks-to:Richard Bush

#include "machine/nmk112.h"
#include "sound/okim6295.h"
#include "machine/nmk004.h"
#include "machine/gen_latch.h"

class nmk16_state : public driver_device
{
public:
	nmk16_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_audiocpu(*this, "audiocpu"),
		m_oki1(*this, "oki1"),
		m_oki2(*this, "oki2"),
		m_gfxdecode(*this, "gfxdecode"),
		m_palette(*this, "palette"),
		m_nmk004(*this, "nmk004"),
		m_soundlatch(*this, "soundlatch"),
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
		m_sprdma_base(0x8000)
	{}

	required_device<cpu_device> m_maincpu;
	optional_device<cpu_device> m_audiocpu;
	optional_device<okim6295_device> m_oki1;
	optional_device<okim6295_device> m_oki2;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<palette_device> m_palette;
	optional_device<nmk004_device> m_nmk004;
	optional_device<generic_latch_8_device> m_soundlatch;

	required_shared_ptr<uint16_t> m_nmk_bgvideoram0;
	optional_shared_ptr<uint16_t> m_nmk_txvideoram;
	required_shared_ptr<uint16_t> m_mainram;
	optional_shared_ptr<uint16_t> m_gunnail_scrollram;
	optional_shared_ptr<uint8_t> m_spriteram;
	optional_shared_ptr<uint16_t> m_nmk_fgvideoram;
	optional_shared_ptr<uint16_t> m_gunnail_scrollramy;
	optional_shared_ptr<uint16_t> m_nmk_bgvideoram1;
	optional_shared_ptr<uint16_t> m_nmk_bgvideoram2;
	optional_shared_ptr<uint16_t> m_nmk_bgvideoram3;
	optional_shared_ptr<uint16_t> m_afega_scroll_0;
	optional_shared_ptr<uint16_t> m_afega_scroll_1;


	int m_sprdma_base;
	int mask[4*2];
	int m_simple_scroll;
	int m_redraw_bitmap;
	std::unique_ptr<uint16_t[]> m_spriteram_old;
	std::unique_ptr<uint16_t[]> m_spriteram_old2;
	int m_bgbank;
	int m_videoshift;
	int m_bioship_background_bank;
	uint8_t m_bioship_scroll[4];
	tilemap_t *m_bg_tilemap0;
	tilemap_t *m_bg_tilemap1;
	tilemap_t *m_bg_tilemap2;
	tilemap_t *m_bg_tilemap3;
	tilemap_t *m_tx_tilemap;
	tilemap_t *m_fg_tilemap;
	std::unique_ptr<bitmap_ind16> m_background_bitmap;
	int m_mustang_bg_xscroll;
	uint8_t m_scroll[4];
	uint8_t m_scroll_2[4];
	uint16_t m_vscroll[4];
	int m_prot_count;
	uint8_t m_input_pressed;
	uint8_t m_start_helper;
	uint8_t m_coin_count[2];
	uint8_t m_coin_count_frac[2];
	void nmk16_mainram_strange_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	void ssmissin_sound_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	void ssmissin_soundbank_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void tharrier_mcu_control_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	uint16_t tharrier_mcu_r(address_space &space, offs_t offset, uint16_t mem_mask = 0xffff);
	void macross2_sound_reset_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	void macross2_sound_command_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	void macross2_sound_bank_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void tharrier_oki6295_bankswitch_0_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void tharrier_oki6295_bankswitch_1_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void afega_soundlatch_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	void hachamf_mainram_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	void tdragon_mainram_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	uint16_t vandykeb_r(address_space &space, offs_t offset, uint16_t mem_mask = 0xffff);
	uint16_t tdragonb_prot_r(address_space &space, offs_t offset, uint16_t mem_mask = 0xffff);
	uint16_t afega_unknown_r(address_space &space, offs_t offset, uint16_t mem_mask = 0xffff);
	void afega_scroll0_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	void afega_scroll1_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	void nmk_bgvideoram0_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	void nmk_bgvideoram1_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	void nmk_bgvideoram2_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	void nmk_bgvideoram3_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	void nmk_fgvideoram_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	void nmk_txvideoram_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	void mustang_scroll_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	void bioshipbg_scroll_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	void nmk_scroll_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	void nmk_scroll_2_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	void vandyke_scroll_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	void vandykeb_scroll_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	void manybloc_scroll_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	void nmk_flipscreen_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	void nmk_tilebank_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	void bioship_scroll_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	void bioship_bank_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	void spec2k_oki1_banking_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void twinactn_oki_bank_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	uint16_t atombjt_unkr_r(address_space &space, offs_t offset, uint16_t mem_mask = 0xffff) {return 0x0000;}
	void nmk16_x0016_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	void nmk16_bioship_x0016_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	void init_nmk();
	void init_tharrier();
	void init_vandykeb();
	void init_tdragonb();
	void init_ssmissin();
	void init_hachamf_prot();
	void init_redhawk();
	void init_tdragon_prot();
	void init_bubl2000();
	void init_banked_audiocpu();
	void init_grdnstrm();
	void init_spec2k();
	void init_redfoxwp2a();
	void init_grdnstrmg();
	void init_bjtwin();
	tilemap_memory_index afega_tilemap_scan_pages(uint32_t col, uint32_t row, uint32_t num_cols, uint32_t num_rows);
	void macross_get_bg0_tile_info(tilemap_t &tilemap, tile_data &tileinfo, tilemap_memory_index tile_index);
	void macross_get_bg1_tile_info(tilemap_t &tilemap, tile_data &tileinfo, tilemap_memory_index tile_index);
	void macross_get_bg2_tile_info(tilemap_t &tilemap, tile_data &tileinfo, tilemap_memory_index tile_index);
	void macross_get_bg3_tile_info(tilemap_t &tilemap, tile_data &tileinfo, tilemap_memory_index tile_index);
	void strahl_get_fg_tile_info(tilemap_t &tilemap, tile_data &tileinfo, tilemap_memory_index tile_index);
	void macross_get_tx_tile_info(tilemap_t &tilemap, tile_data &tileinfo, tilemap_memory_index tile_index);
	void bjtwin_get_bg_tile_info(tilemap_t &tilemap, tile_data &tileinfo, tilemap_memory_index tile_index);
	void get_tile_info_0_8bit(tilemap_t &tilemap, tile_data &tileinfo, tilemap_memory_index tile_index);
	void video_start_macross();
	void video_start_bioship();
	void video_start_strahl();
	void video_start_gunnail();
	void video_start_macross2();
	void video_start_raphero();
	void video_start_bjtwin();
	void video_start_afega();
	void video_start_firehawk();
	void video_start_grdnstrm();
	uint32_t screen_update_tharrier(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	uint32_t screen_update_manybloc(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	uint32_t screen_update_macross(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	uint32_t screen_update_bioship(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	uint32_t screen_update_strahl(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	uint32_t screen_update_gunnail(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	uint32_t screen_update_tdragon2(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	uint32_t screen_update_bjtwin(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	uint32_t screen_update_afega(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	uint32_t screen_update_firehawk(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	uint32_t screen_update_redhawki(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	uint32_t screen_update_redhawkb(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	uint32_t screen_update_bubl2000(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	void tdragon_mcu_sim(timer_device &timer, void *ptr, int32_t param);
	void hachamf_mcu_sim(timer_device &timer, void *ptr, int32_t param);
	void nmk16_scanline(timer_device &timer, void *ptr, int32_t param);
	void manybloc_scanline(timer_device &timer, void *ptr, int32_t param);
	void nmk16_video_init();
	inline void nmk16_draw_sprite(bitmap_ind16 &bitmap, const rectangle &cliprect, uint16_t *spr);
	inline void nmk16_draw_sprite_flipsupported(bitmap_ind16 &bitmap, const rectangle &cliprect, uint16_t *spr);
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
	void mcu_run(uint8_t dsw_setting);
	uint8_t decode_byte(uint8_t src, const uint8_t *bitp);
	uint32_t bjtwin_address_map_bg0(uint32_t addr);
	uint16_t decode_word(uint16_t src, const uint8_t *bitp);
	uint32_t bjtwin_address_map_sprites(uint32_t addr);
	void decode_gfx();
	void decode_tdragonb();
	void decode_ssmissin();
};
