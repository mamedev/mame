// license:BSD-3-Clause
// copyright-holders:Ernesto Corvi,Tim Lindquist,Carlos A. Lozano,Bryan McPhail,Jarek Parchanski,Nicola Salmoria,Tomasz Slanina,Phil Stroffolino,Acho A. Tang,Victor Trucco
// thanks-to:Marco Cassili

/*************************************************************************

    various SNK triple Z80 games

*************************************************************************/

#include "machine/gen_latch.h"

class snk_state : public driver_device
{
public:
	snk_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_audiocpu(*this, "audiocpu"),
		m_subcpu(*this, "sub"),
		m_gfxdecode(*this, "gfxdecode"),
		m_screen(*this, "screen"),
		m_palette(*this, "palette"),
		m_soundlatch(*this, "soundlatch"),
		m_spriteram(*this, "spriteram"),
		m_fg_videoram(*this, "fg_videoram"),
		m_bg_videoram(*this, "bg_videoram"),
		m_tx_videoram(*this, "tx_videoram") { }

	required_device<cpu_device> m_maincpu;
	required_device<cpu_device> m_audiocpu;
	required_device<cpu_device> m_subcpu;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<screen_device> m_screen;
	required_device<palette_device> m_palette;
	optional_device<generic_latch_8_device> m_soundlatch;

	required_shared_ptr<uint8_t> m_spriteram;
	optional_shared_ptr<uint8_t> m_fg_videoram;
	required_shared_ptr<uint8_t> m_bg_videoram;
	required_shared_ptr<uint8_t> m_tx_videoram;

	int m_countryc_trackball;
	int m_last_value[2];
	int m_cp_count[2];

	int m_marvins_sound_busy_flag;
	// FIXME this should be initialised on machine reset
	int m_sound_status;

	tilemap_t *m_tx_tilemap;
	tilemap_t *m_fg_tilemap;
	tilemap_t *m_bg_tilemap;
	int m_fg_scrollx;
	int m_fg_scrolly;
	int m_bg_scrollx;
	int m_bg_scrolly;
	int m_sp16_scrollx;
	int m_sp16_scrolly;
	int m_sp32_scrollx;
	int m_sp32_scrolly;
	uint8_t m_sprite_split_point;
	int m_num_sprites;
	int m_yscroll_mask;
	uint32_t m_bg_tile_offset;
	uint32_t m_tx_tile_offset;
	int m_is_psychos;

	uint8_t m_drawmode_table[16];
	uint8_t m_empty_tile[16*16];
	int m_hf_posy;
	int m_hf_posx;
	int m_tc16_posy;
	int m_tc16_posx;
	int m_tc32_posy;
	int m_tc32_posx;
	uint8_t snk_cpuA_nmi_trigger_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	void snk_cpuA_nmi_ack_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	uint8_t snk_cpuB_nmi_trigger_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	void snk_cpuB_nmi_ack_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void marvins_soundlatch_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	uint8_t marvins_soundlatch_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	uint8_t marvins_sound_nmi_ack_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	void sgladiat_soundlatch_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	uint8_t sgladiat_soundlatch_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	uint8_t sgladiat_sound_nmi_ack_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	uint8_t sgladiat_sound_irq_ack_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	void snk_soundlatch_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	uint8_t snk_sound_status_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	void snk_sound_status_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	uint8_t tnk3_cmdirq_ack_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	uint8_t tnk3_ymirq_ack_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	uint8_t tnk3_busy_clear_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	void hardflags_scrollx_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void hardflags_scrolly_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void hardflags_scroll_msb_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	uint8_t hardflags1_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	uint8_t hardflags2_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	uint8_t hardflags3_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	uint8_t hardflags4_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	uint8_t hardflags5_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	uint8_t hardflags6_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	uint8_t hardflags7_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	void turbocheck16_1_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void turbocheck16_2_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void turbocheck32_1_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void turbocheck32_2_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void turbocheck_msb_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	uint8_t turbocheck16_1_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	uint8_t turbocheck16_2_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	uint8_t turbocheck16_3_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	uint8_t turbocheck16_4_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	uint8_t turbocheck16_5_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	uint8_t turbocheck16_6_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	uint8_t turbocheck16_7_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	uint8_t turbocheck16_8_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	uint8_t turbocheck32_1_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	uint8_t turbocheck32_2_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	uint8_t turbocheck32_3_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	uint8_t turbocheck32_4_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	void athena_coin_counter_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void ikari_coin_counter_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void tdfever_coin_counter_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void countryc_trackball_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void snk_tx_videoram_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void marvins_fg_videoram_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void marvins_bg_videoram_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void snk_bg_videoram_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void snk_fg_scrollx_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void snk_fg_scrolly_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void snk_bg_scrollx_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void snk_bg_scrolly_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void snk_sp16_scrollx_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void snk_sp16_scrolly_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void snk_sp32_scrollx_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void snk_sp32_scrolly_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void snk_sprite_split_point_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void marvins_palette_bank_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void marvins_flipscreen_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void sgladiat_flipscreen_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void hal21_flipscreen_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void marvins_scroll_msb_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void jcross_scroll_msb_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void sgladiat_scroll_msb_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void aso_videoattrs_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void tnk3_videoattrs_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void aso_bg_bank_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void ikari_bg_scroll_msb_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void ikari_sp_scroll_msb_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void ikari_unknown_video_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void gwar_tx_bank_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void gwar_videoattrs_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void gwara_videoattrs_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void gwara_sp_scroll_msb_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void tdfever_sp_scroll_msb_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void tdfever_spriteram_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	ioport_value marvins_sound_busy(ioport_field &field, void *param);
	ioport_value snk_sound_busy(ioport_field &field, void *param);
	ioport_value gwar_rotary(ioport_field &field, void *param);
	ioport_value gwarb_rotary(ioport_field &field, void *param);
	ioport_value countryc_trackball_x(ioport_field &field, void *param);
	ioport_value countryc_trackball_y(ioport_field &field, void *param);
	ioport_value snk_bonus_r(ioport_field &field, void *param);
	void init_countryc();
	tilemap_memory_index marvins_tx_scan_cols(uint32_t col, uint32_t row, uint32_t num_cols, uint32_t num_rows);
	void marvins_get_tx_tile_info(tilemap_t &tilemap, tile_data &tileinfo, tilemap_memory_index tile_index);
	void ikari_get_tx_tile_info(tilemap_t &tilemap, tile_data &tileinfo, tilemap_memory_index tile_index);
	void gwar_get_tx_tile_info(tilemap_t &tilemap, tile_data &tileinfo, tilemap_memory_index tile_index);
	void marvins_get_fg_tile_info(tilemap_t &tilemap, tile_data &tileinfo, tilemap_memory_index tile_index);
	void marvins_get_bg_tile_info(tilemap_t &tilemap, tile_data &tileinfo, tilemap_memory_index tile_index);
	void aso_get_bg_tile_info(tilemap_t &tilemap, tile_data &tileinfo, tilemap_memory_index tile_index);
	void tnk3_get_bg_tile_info(tilemap_t &tilemap, tile_data &tileinfo, tilemap_memory_index tile_index);
	void ikari_get_bg_tile_info(tilemap_t &tilemap, tile_data &tileinfo, tilemap_memory_index tile_index);
	void gwar_get_bg_tile_info(tilemap_t &tilemap, tile_data &tileinfo, tilemap_memory_index tile_index);
	void video_start_marvins();
	void palette_init_tnk3(palette_device &palette);
	void video_start_jcross();
	void video_start_tnk3();
	void video_start_ikari();
	void video_start_gwar();
	void video_start_tdfever();
	void video_start_sgladiat();
	void video_start_hal21();
	void video_start_aso();
	void video_start_psychos();
	void video_start_snk_3bpp_shadow();
	void video_start_snk_4bpp_shadow();
	uint32_t screen_update_marvins(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	uint32_t screen_update_tnk3(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	uint32_t screen_update_ikari(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	uint32_t screen_update_gwar(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	uint32_t screen_update_tdfever(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	uint32_t screen_update_fitegolf2(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	void sgladiat_sndirq_update_callback(void *ptr, int32_t param);
	void sndirq_update_callback(void *ptr, int32_t param);
	void ymirq_callback_2(int state);
	void marvins_draw_sprites(bitmap_ind16 &bitmap, const rectangle &cliprect, const int scrollx, const int scrolly, const int from, const int to);
	void tnk3_draw_sprites(bitmap_ind16 &bitmap, const rectangle &cliprect, const int xscroll, const int yscroll);
	void ikari_draw_sprites(bitmap_ind16 &bitmap, const rectangle &cliprect, const int start, const int xscroll, const int yscroll, const uint8_t *source, const int gfxnum );
	void tdfever_draw_sprites(bitmap_ind16 &bitmap, const rectangle &cliprect,  const int xscroll, const int yscroll, const uint8_t *source, const int gfxnum, const int hw_xflip, const int from, const int to);
	int hardflags_check(int num);
	int hardflags_check8(int num);
	int turbofront_check(int small, int num);
	int turbofront_check8(int small, int num);
	void ymirq_callback_1(int state);
};
