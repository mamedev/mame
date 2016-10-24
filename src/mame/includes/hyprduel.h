// license:BSD-3-Clause
// copyright-holders:Luca Elia, Hau
#define RASTER_LINES 262
#define FIRST_VISIBLE_LINE 0
#define LAST_VISIBLE_LINE 223

class hyprduel_state : public driver_device
{
public:
	hyprduel_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		m_vram_0(*this, "vram_0"),
		m_vram_1(*this, "vram_1"),
		m_vram_2(*this, "vram_2"),
		m_paletteram(*this, "paletteram"),
		m_spriteram(*this, "spriteram"),
		m_tiletable(*this, "tiletable"),
		m_blitter_regs(*this, "blitter_regs"),
		m_window(*this, "window"),
		m_scroll(*this, "scroll"),
		m_irq_enable(*this, "irq_enable"),
		m_rombank(*this, "rombank"),
		m_screenctrl(*this, "screenctrl"),
		m_videoregs(*this, "videoregs"),
		m_sharedram1(*this, "sharedram1"),
		m_sharedram3(*this, "sharedram3"),
		m_maincpu(*this, "maincpu"),
		m_subcpu(*this, "sub"),
		m_gfxdecode(*this, "gfxdecode"),
		m_screen(*this, "screen"),
		m_palette(*this, "palette") { }

	/* memory pointers */
	required_shared_ptr<uint16_t> m_vram_0;
	required_shared_ptr<uint16_t> m_vram_1;
	required_shared_ptr<uint16_t> m_vram_2;
	required_shared_ptr<uint16_t> m_paletteram;
	required_shared_ptr<uint16_t> m_spriteram;
	required_shared_ptr<uint16_t> m_tiletable;
	required_shared_ptr<uint16_t> m_blitter_regs;
	required_shared_ptr<uint16_t> m_window;
	required_shared_ptr<uint16_t> m_scroll;
	required_shared_ptr<uint16_t> m_irq_enable;
	required_shared_ptr<uint16_t> m_rombank;
	required_shared_ptr<uint16_t> m_screenctrl;
	required_shared_ptr<uint16_t> m_videoregs;
	required_shared_ptr<uint16_t> m_sharedram1;
	required_shared_ptr<uint16_t> m_sharedram3;
	std::unique_ptr<uint16_t[]>  m_tiletable_old;

	/* video-related */
	tilemap_t   *m_bg_tilemap[3];
	std::unique_ptr<uint8_t[]>    m_empty_tiles;
	std::unique_ptr<uint8_t[]>     m_dirtyindex;
	int       m_sprite_xoffs;
	int       m_sprite_yoffs;
	int       m_sprite_yoffs_sub;
	std::unique_ptr<uint8_t[]>   m_expanded_gfx1;

	/* misc */
	emu_timer *m_magerror_irq_timer;
	int       m_blitter_bit;
	int       m_requested_int;
	int       m_subcpu_resetline;
	int       m_cpu_trigger;
	int       m_int_num;

	/* devices */
	required_device<cpu_device> m_maincpu;
	required_device<cpu_device> m_subcpu;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<screen_device> m_screen;
	required_device<palette_device> m_palette;

	uint16_t hyprduel_irq_cause_r(address_space &space, offs_t offset, uint16_t mem_mask = 0xffff);
	void hyprduel_irq_cause_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	void hyprduel_subcpu_control_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	uint16_t hyprduel_cpusync_trigger1_r(address_space &space, offs_t offset, uint16_t mem_mask = 0xffff);
	void hyprduel_cpusync_trigger1_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	uint16_t hyprduel_cpusync_trigger2_r(address_space &space, offs_t offset, uint16_t mem_mask = 0xffff);
	void hyprduel_cpusync_trigger2_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	uint16_t hyprduel_bankedrom_r(address_space &space, offs_t offset, uint16_t mem_mask = 0xffff);
	void hyprduel_blitter_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	void hyprduel_paletteram_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	void hyprduel_vram_0_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	void hyprduel_vram_1_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	void hyprduel_vram_2_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	void hyprduel_window_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	void hyprduel_scrollreg_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	void hyprduel_scrollreg_init_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	void blt_write( address_space &space, const int tmap, const offs_t offs, const uint16_t data, const uint16_t mask );
	void init_magerror();
	void init_hyprduel();
	void get_tile_info_0_8bit(tilemap_t &tilemap, tile_data &tileinfo, tilemap_memory_index tile_index);
	void get_tile_info_1_8bit(tilemap_t &tilemap, tile_data &tileinfo, tilemap_memory_index tile_index);
	void get_tile_info_2_8bit(tilemap_t &tilemap, tile_data &tileinfo, tilemap_memory_index tile_index);
	virtual void machine_reset() override;
	void machine_start_hyprduel();
	void video_start_hyprduel_14220();
	void machine_start_magerror();
	void video_start_magerror_14220();
	void video_start_common_14220();
	uint32_t screen_update_hyprduel(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	void vblank_end_callback(void *ptr, int32_t param);
	void magerror_irq_callback(void *ptr, int32_t param);
	void hyprduel_blit_done(void *ptr, int32_t param);
	void hyprduel_interrupt(timer_device &timer, void *ptr, int32_t param);
	void hyprduel_postload();
	inline void get_tile_info( tile_data &tileinfo, int tile_index, int layer, uint16_t *vram);
	inline void get_tile_info_8bit( tile_data &tileinfo, int tile_index, int layer, uint16_t *vram );
	inline void get_tile_info_16x16_8bit( tile_data &tileinfo, int tile_index, int layer, uint16_t *vram );
	inline void hyprduel_vram_w( offs_t offset, uint16_t data, uint16_t mem_mask, int layer, uint16_t *vram );
	void alloc_empty_tiles(  );
	void expand_gfx1(hyprduel_state &state);
	void draw_sprites( screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect );
	void draw_layers( screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect, int pri, int layers_ctrl );
	void dirty_tiles( int layer, uint16_t *vram );
	void update_irq_state(  );
	inline int blt_read( const uint8_t *ROM, const int offs );
};
