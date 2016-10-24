// license:BSD-3-Clause
// copyright-holders:Jarek Parchanski
#include "machine/bankdev.h"
#include "video/jalblend.h"

class psychic5_state : public driver_device
{
public:
	psychic5_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_audiocpu(*this, "audiocpu"),
		m_gfxdecode(*this, "gfxdecode"),
		m_palette(*this, "palette"),
		m_vrambank(*this, "vrambank"),
		m_blend(*this, "blend"),
		m_spriteram(*this, "spriteram"),
		m_fg_videoram(*this, "fg_videoram"),
		m_bg_videoram(*this, "bg_videoram"),
		m_bg_control(*this, "bg_control"),
		m_ps5_palette_ram_bg(*this, "palette_ram_bg"),
		m_ps5_palette_ram_sp(*this, "palette_ram_sp"),
		m_ps5_palette_ram_tx(*this, "palette_ram_tx")

	{ }

	required_device<cpu_device> m_maincpu;
	required_device<cpu_device> m_audiocpu;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<palette_device> m_palette;
	optional_device<address_map_bank_device> m_vrambank;
	optional_device<jaleco_blend_device> m_blend;

	required_shared_ptr<uint8_t> m_spriteram;
	required_shared_ptr<uint8_t> m_fg_videoram;
	required_shared_ptr<uint8_t> m_bg_videoram;
	required_shared_ptr<uint8_t> m_bg_control;
	required_shared_ptr<uint8_t> m_ps5_palette_ram_bg;
	required_shared_ptr<uint8_t> m_ps5_palette_ram_sp;
	required_shared_ptr<uint8_t> m_ps5_palette_ram_tx;

	uint8_t m_bank_latch;
	uint8_t m_ps5_vram_page;
	uint8_t m_bg_clip_mode;
	uint8_t m_title_screen;
	tilemap_t *m_bg_tilemap;
	tilemap_t *m_fg_tilemap;
	uint16_t m_palette_intensity;
	uint8_t m_bombsa_unknown;
	int m_sx1;
	int m_sy1;
	int m_sy2;

	uint8_t bankselect_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	uint8_t vram_page_select_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	void vram_page_select_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void fg_videoram_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void bg_videoram_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void sprite_col_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void bg_col_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void tx_col_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);

	/* psychic5 specific */
	void psychic5_coin_counter_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void psychic5_bankselect_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void psychic5_title_screen_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);

	/* bombsa specific */
	void bombsa_bankselect_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void bombsa_flipscreen_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void bombsa_unknown_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);

	void get_bg_tile_info(tilemap_t &tilemap, tile_data &tileinfo, tilemap_memory_index tile_index);
	void get_fg_tile_info(tilemap_t &tilemap, tile_data &tileinfo, tilemap_memory_index tile_index);

	virtual void machine_reset() override;
	void machine_start_psychic5();
	void machine_start_bombsa();
	virtual void video_start() override;
	void video_start_psychic5();
	void video_start_bombsa();
	void video_reset_psychic5();

	void scanline(timer_device &timer, void *ptr, int32_t param);

	uint32_t screen_update_psychic5(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);
	uint32_t screen_update_bombsa(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);
	void change_palette(int offset, uint8_t* palram, int palbase);
	void change_bg_palette(int color, int lo_offs, int hi_offs);
	void set_background_palette_intensity();
	void draw_sprites(bitmap_rgb32 &bitmap, const rectangle &cliprect);
	void draw_background(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect); //only used by psychic5
};
