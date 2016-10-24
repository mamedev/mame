// license:BSD-3-Clause
// copyright-holders:Aaron Giles
class tp84_state : public driver_device
{
public:
	tp84_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		m_audiocpu(*this, "audiocpu"),
		m_palette_bank(*this, "palette_bank"),
		m_flipscreen_x(*this, "flipscreen_x"),
		m_flipscreen_y(*this, "flipscreen_y"),
		m_scroll_x(*this, "scroll_x"),
		m_scroll_y(*this, "scroll_y"),
		m_bg_videoram(*this, "bg_videoram"),
		m_fg_videoram(*this, "fg_videoram"),
		m_bg_colorram(*this, "bg_colorram"),
		m_fg_colorram(*this, "fg_colorram"),
		m_spriteram(*this, "spriteram"),
		m_gfxdecode(*this, "gfxdecode"),
		m_screen(*this, "screen"),
		m_palette(*this, "palette") { }

	required_device<cpu_device> m_audiocpu;
	required_shared_ptr<uint8_t> m_palette_bank;
	required_shared_ptr<uint8_t> m_flipscreen_x;
	required_shared_ptr<uint8_t> m_flipscreen_y;
	required_shared_ptr<uint8_t> m_scroll_x;
	required_shared_ptr<uint8_t> m_scroll_y;
	required_shared_ptr<uint8_t> m_bg_videoram;
	required_shared_ptr<uint8_t> m_fg_videoram;
	required_shared_ptr<uint8_t> m_bg_colorram;
	required_shared_ptr<uint8_t> m_fg_colorram;
	required_shared_ptr<uint8_t> m_spriteram;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<screen_device> m_screen;
	required_device<palette_device> m_palette;
	tilemap_t *m_bg_tilemap;
	tilemap_t *m_fg_tilemap;

	uint8_t m_sub_irq_mask;
	uint8_t tp84_sh_timer_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	void tp84_filter_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void tp84_sh_irqtrigger_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void sub_irq_mask_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void tp84_spriteram_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	uint8_t tp84_scanline_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	void get_bg_tile_info(tilemap_t &tilemap, tile_data &tileinfo, tilemap_memory_index tile_index);
	void get_fg_tile_info(tilemap_t &tilemap, tile_data &tileinfo, tilemap_memory_index tile_index);
	virtual void machine_start() override;
	virtual void video_start() override;
	void palette_init_tp84(palette_device &palette);
	uint32_t screen_update_tp84(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	void sub_vblank_irq(device_t &device);
	void draw_sprites(bitmap_ind16 &bitmap, const rectangle &cliprect);
};
