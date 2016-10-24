// license:BSD-3-Clause
// copyright-holders:David Haywood,Bryan McPhail

#include "machine/gen_latch.h"
#include "video/decospr.h"
#include "sound/okim6295.h"

class tumbleb_state : public driver_device
{
public:
	tumbleb_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		m_mainram(*this, "mainram"),
		m_spriteram(*this, "spriteram"),
		m_pf1_data(*this, "pf1_data"),
		m_pf2_data(*this, "pf2_data"),
		m_control(*this, "control"),
		m_maincpu(*this, "maincpu"),
		m_audiocpu(*this, "audiocpu"),
		m_oki(*this, "oki"),
		m_gfxdecode(*this, "gfxdecode"),
		m_palette(*this, "palette"),
		m_sprgen(*this, "spritegen"),
		m_soundlatch(*this, "soundlatch")
	{ }

	/* memory pointers */
	optional_shared_ptr<uint16_t> m_mainram;
	required_shared_ptr<uint16_t> m_spriteram;
	required_shared_ptr<uint16_t> m_pf1_data;
	required_shared_ptr<uint16_t> m_pf2_data;
	optional_shared_ptr<uint16_t> m_control;

	/* misc */
	int         m_music_command;
	int         m_music_bank;
	int         m_music_is_playing;

	/* video-related */
	tilemap_t   *m_pf1_tilemap;
	tilemap_t   *m_pf1_alt_tilemap;
	tilemap_t   *m_pf2_tilemap;
	tilemap_t   *m_pf2_alt_tilemap;
	uint16_t      m_control_0[8];
	uint16_t      m_tilebank;

	/* devices */
	required_device<cpu_device> m_maincpu;
	optional_device<cpu_device> m_audiocpu;
	required_device<okim6295_device> m_oki;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<palette_device> m_palette;
	optional_device<decospr_device> m_sprgen;
	optional_device<generic_latch_8_device> m_soundlatch;

	uint8_t m_semicom_prot_offset;
	uint16_t m_protbase;
	void tumblepb_oki_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	uint16_t tumblepb_prot_r(address_space &space, offs_t offset, uint16_t mem_mask = 0xffff);
	uint16_t tumblepopb_controls_r(address_space &space, offs_t offset, uint16_t mem_mask = 0xffff);
	uint16_t semibase_unknown_r(address_space &space, offs_t offset, uint16_t mem_mask = 0xffff);
	void jumpkids_sound_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	void semicom_soundcmd_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	void oki_sound_bank_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void jumpkids_oki_bank_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	uint8_t prot_io_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	void prot_io_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	uint16_t bcstory_1a0_read(address_space &space, offs_t offset, uint16_t mem_mask = 0xffff);
	void bcstory_tilebank_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	void chokchok_tilebank_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	void wlstar_tilebank_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	void suprtrio_tilebank_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	void tumblepb_pf1_data_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	void tumblepb_pf2_data_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	void fncywld_pf1_data_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	void fncywld_pf2_data_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	void tumblepb_control_0_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	void pangpang_pf1_data_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	void pangpang_pf2_data_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	void tumbleb2_soundmcu_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	void init_dquizgo();
	void init_jumpkids();
	void init_htchctch();
	void init_wlstar();
	void init_suprtrio();
	void init_tumblepb();
	void init_tumblepba();
	void init_bcstory();
	void init_wondl96();
	void init_tumbleb2();
	void init_chokchok();
	void init_fncywld();
	void init_carket();
	tilemap_memory_index tumblep_scan(uint32_t col, uint32_t row, uint32_t num_cols, uint32_t num_rows);
	void get_bg1_tile_info(tilemap_t &tilemap, tile_data &tileinfo, tilemap_memory_index tile_index);
	void get_bg2_tile_info(tilemap_t &tilemap, tile_data &tileinfo, tilemap_memory_index tile_index);
	void get_fg_tile_info(tilemap_t &tilemap, tile_data &tileinfo, tilemap_memory_index tile_index);
	void get_fncywld_bg1_tile_info(tilemap_t &tilemap, tile_data &tileinfo, tilemap_memory_index tile_index);
	void get_fncywld_bg2_tile_info(tilemap_t &tilemap, tile_data &tileinfo, tilemap_memory_index tile_index);
	void get_fncywld_fg_tile_info(tilemap_t &tilemap, tile_data &tileinfo, tilemap_memory_index tile_index);
	void pangpang_get_bg1_tile_info(tilemap_t &tilemap, tile_data &tileinfo, tilemap_memory_index tile_index);
	void pangpang_get_bg2_tile_info(tilemap_t &tilemap, tile_data &tileinfo, tilemap_memory_index tile_index);
	void pangpang_get_fg_tile_info(tilemap_t &tilemap, tile_data &tileinfo, tilemap_memory_index tile_index);
	void machine_start_tumbleb();
	void machine_reset_tumbleb();
	void video_start_tumblepb();
	void video_start_fncywld();
	void machine_reset_htchctch();
	void video_start_suprtrio();
	void video_start_pangpang();
	void video_start_sdfight();
	uint32_t screen_update_tumblepb(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	uint32_t screen_update_jumpkids(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	uint32_t screen_update_fncywld(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	uint32_t screen_update_semicom(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	uint32_t screen_update_suprtrio(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	uint32_t screen_update_pangpang(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	uint32_t screen_update_semicom_altoffsets(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	uint32_t screen_update_bcstory(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	uint32_t screen_update_semibase(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	uint32_t screen_update_sdfight(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	void tumbleb2_interrupt(device_t &device);
	void tumbleb_tilemap_redraw();
	inline void get_bg_tile_info( tile_data &tileinfo, int tile_index, int gfx_bank, uint16_t *gfx_base);
	inline void get_fncywld_bg_tile_info( tile_data &tileinfo, int tile_index, int gfx_bank, uint16_t *gfx_base);
	inline void pangpang_get_bg_tile_info( tile_data &tileinfo, int tile_index, int gfx_bank, uint16_t *gfx_base );
	inline void pangpang_get_bg2x_tile_info( tile_data &tileinfo, int tile_index, int gfx_bank, uint16_t *gfx_base );
	void tumbleb_draw_common(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect, int pf1x_offs, int pf1y_offs, int pf2x_offs, int pf2y_offs);
	void tumbleb2_set_music_bank( int bank );
	void tumbleb2_play_sound( okim6295_device *oki, int data );
	void process_tumbleb2_music_command( okim6295_device *oki, int data );
	void tumblepb_gfx_rearrange(int rgn);
	void suprtrio_decrypt_code();
	void suprtrio_decrypt_gfx();
};
