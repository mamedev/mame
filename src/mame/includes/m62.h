// license:BSD-3-Clause
// copyright-holders:smf, David Haywood

#include "audio/irem.h"

class m62_state : public driver_device
{
public:
	m62_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		m_spriteram(*this, "spriteram"),
		m_m62_tileram(*this, "m62_tileram"),
		m_m62_textram(*this, "m62_textram"),
		m_scrollram(*this, "scrollram"),
		m_maincpu(*this, "maincpu"),
		m_gfxdecode(*this, "gfxdecode"),
		m_palette(*this, "palette"),
		m_audio(*this, "irem_audio")
	{ }

	/* memory pointers */
	required_shared_ptr<uint8_t> m_spriteram;

	required_shared_ptr<uint8_t> m_m62_tileram;
	optional_shared_ptr<uint8_t> m_m62_textram;
	optional_shared_ptr<uint8_t> m_scrollram;

	/* video-related */
	tilemap_t*             m_bg_tilemap;
	tilemap_t*             m_fg_tilemap;
	int                  m_flipscreen;

	const uint8_t          *m_sprite_height_prom;
	int32_t                m_m62_background_hscroll;
	int32_t                m_m62_background_vscroll;
	uint8_t                m_kidniki_background_bank;
	int32_t                m_kidniki_text_vscroll;
	int                  m_ldrun3_topbottom_mask;
	int32_t                m_spelunkr_palbank;

	/* misc */
	int                 m_ldrun2_bankswap;  //ldrun2
	int                 m_bankcontrol[2];   //ldrun2
	uint8_t ldrun2_bankswitch_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	void ldrun2_bankswitch_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	uint8_t ldrun3_prot_5_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	uint8_t ldrun3_prot_7_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	void ldrun4_bankswitch_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void kidniki_bankswitch_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void spelunkr_bankswitch_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void spelunk2_bankswitch_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void youjyudn_bankswitch_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void m62_flipscreen_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void m62_hscroll_low_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void m62_hscroll_high_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void m62_vscroll_low_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void m62_vscroll_high_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void m62_tileram_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void m62_textram_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void kungfum_tileram_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void ldrun3_topbottom_mask_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void kidniki_text_vscroll_low_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void kidniki_text_vscroll_high_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void kidniki_background_bank_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void spelunkr_palbank_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void spelunk2_gfxport_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void horizon_scrollram_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void init_youjyudn();
	void init_spelunkr();
	void init_ldrun2();
	void init_ldrun4();
	void init_spelunk2();
	void init_kidniki();
	void init_battroad();
	void get_kungfum_bg_tile_info(tilemap_t &tilemap, tile_data &tileinfo, tilemap_memory_index tile_index);
	void get_ldrun_bg_tile_info(tilemap_t &tilemap, tile_data &tileinfo, tilemap_memory_index tile_index);
	void get_ldrun2_bg_tile_info(tilemap_t &tilemap, tile_data &tileinfo, tilemap_memory_index tile_index);
	void get_battroad_bg_tile_info(tilemap_t &tilemap, tile_data &tileinfo, tilemap_memory_index tile_index);
	void get_battroad_fg_tile_info(tilemap_t &tilemap, tile_data &tileinfo, tilemap_memory_index tile_index);
	void get_ldrun4_bg_tile_info(tilemap_t &tilemap, tile_data &tileinfo, tilemap_memory_index tile_index);
	void get_lotlot_bg_tile_info(tilemap_t &tilemap, tile_data &tileinfo, tilemap_memory_index tile_index);
	void get_lotlot_fg_tile_info(tilemap_t &tilemap, tile_data &tileinfo, tilemap_memory_index tile_index);
	void get_kidniki_bg_tile_info(tilemap_t &tilemap, tile_data &tileinfo, tilemap_memory_index tile_index);
	void get_kidniki_fg_tile_info(tilemap_t &tilemap, tile_data &tileinfo, tilemap_memory_index tile_index);
	void get_spelunkr_bg_tile_info(tilemap_t &tilemap, tile_data &tileinfo, tilemap_memory_index tile_index);
	void get_spelunkr_fg_tile_info(tilemap_t &tilemap, tile_data &tileinfo, tilemap_memory_index tile_index);
	void get_spelunk2_bg_tile_info(tilemap_t &tilemap, tile_data &tileinfo, tilemap_memory_index tile_index);
	void get_youjyudn_bg_tile_info(tilemap_t &tilemap, tile_data &tileinfo, tilemap_memory_index tile_index);
	void get_youjyudn_fg_tile_info(tilemap_t &tilemap, tile_data &tileinfo, tilemap_memory_index tile_index);
	void get_horizon_bg_tile_info(tilemap_t &tilemap, tile_data &tileinfo, tilemap_memory_index tile_index);
	virtual void machine_start() override;
	virtual void machine_reset() override;
	virtual void video_start() override;
	void palette_init_m62(palette_device &palette);
	void video_start_kungfum();
	void video_start_battroad();
	void palette_init_battroad(palette_device &palette);
	void video_start_ldrun2();
	void video_start_ldrun4();
	void video_start_lotlot();
	void palette_init_lotlot(palette_device &palette);
	void video_start_kidniki();
	void video_start_spelunkr();
	void video_start_spelunk2();
	void palette_init_spelunk2(palette_device &palette);
	void video_start_youjyudn();
	void video_start_horizon();
	uint32_t screen_update_ldrun(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	uint32_t screen_update_kungfum(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	uint32_t screen_update_battroad(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	uint32_t screen_update_ldrun3(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	uint32_t screen_update_ldrun4(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	uint32_t screen_update_lotlot(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	uint32_t screen_update_kidniki(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	uint32_t screen_update_spelunkr(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	uint32_t screen_update_spelunk2(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	uint32_t screen_update_youjyudn(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	uint32_t screen_update_horizon(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	void m62_amplify_contrast(palette_t *palette, uint32_t numcolors);
	void register_savestate(  );
	void draw_sprites( bitmap_ind16 &bitmap, const rectangle &cliprect, int colormask, int prioritymask, int priority );
	void m62_start( tilemap_get_info_delegate tile_get_info, int rows, int cols, int x1, int y1, int x2, int y2 );
	void m62_textlayer( tilemap_get_info_delegate tile_get_info, int rows, int cols, int x1, int y1, int x2, int y2 );
	required_device<cpu_device> m_maincpu;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<palette_device> m_palette;
	required_device<irem_audio_device> m_audio;
};
