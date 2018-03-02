// license:BSD-3-Clause
// copyright-holders:Luca Elia, Hau

#include "machine/timer.h"
#include "screen.h"

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
		m_spriteregs(*this, "spriteregs"),
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

	DECLARE_READ16_MEMBER(irq_cause_r);
	DECLARE_WRITE16_MEMBER(irq_cause_w);
	DECLARE_WRITE16_MEMBER(subcpu_control_w);
	DECLARE_READ16_MEMBER(hyprduel_cpusync_trigger1_r);
	DECLARE_WRITE16_MEMBER(hyprduel_cpusync_trigger1_w);
	DECLARE_READ16_MEMBER(hyprduel_cpusync_trigger2_r);
	DECLARE_WRITE16_MEMBER(hyprduel_cpusync_trigger2_w);
	DECLARE_READ16_MEMBER(bankedrom_r);
	DECLARE_WRITE16_MEMBER(blitter_w);
	DECLARE_WRITE16_MEMBER(paletteram_w);
	DECLARE_WRITE16_MEMBER(vram_0_w);
	DECLARE_WRITE16_MEMBER(vram_1_w);
	DECLARE_WRITE16_MEMBER(vram_2_w);
	DECLARE_WRITE16_MEMBER(window_w);
	DECLARE_WRITE16_MEMBER(scrollreg_w);
	DECLARE_WRITE16_MEMBER(scrollreg_init_w);
	DECLARE_DRIVER_INIT(magerror);
	DECLARE_DRIVER_INIT(hyprduel);
	TILE_GET_INFO_MEMBER(get_tile_info_0_8bit);
	TILE_GET_INFO_MEMBER(get_tile_info_1_8bit);
	TILE_GET_INFO_MEMBER(get_tile_info_2_8bit);
	DECLARE_MACHINE_START(hyprduel);
	DECLARE_VIDEO_START(hyprduel_14220);
	DECLARE_MACHINE_START(magerror);
	DECLARE_VIDEO_START(magerror_14220);
	DECLARE_VIDEO_START(common_14220);
	uint32_t screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	TIMER_CALLBACK_MEMBER(vblank_end_callback);
	TIMER_CALLBACK_MEMBER(magerror_irq_callback);
	TIMER_CALLBACK_MEMBER(blit_done);
	TIMER_DEVICE_CALLBACK_MEMBER(interrupt);

	void magerror(machine_config &config);
	void hyprduel(machine_config &config);
	void hyprduel_map(address_map &map);
	void hyprduel_map2(address_map &map);
	void magerror_map(address_map &map);
	void magerror_map2(address_map &map);
protected:
	virtual void machine_reset() override;

private:
	/* memory pointers */
	required_shared_ptr<uint16_t> m_vram_0;
	required_shared_ptr<uint16_t> m_vram_1;
	required_shared_ptr<uint16_t> m_vram_2;
	required_shared_ptr<uint16_t> m_paletteram;
	required_shared_ptr<uint16_t> m_spriteram;
	required_shared_ptr<uint16_t> m_spriteregs;
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
	std::unique_ptr<uint8_t[]>   m_expanded_gfx1;

	/* misc */
	emu_timer *m_magerror_irq_timer;
	emu_timer *m_vblank_end_timer;
	emu_timer *m_blit_done_timer;
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

	void postload();
	inline void get_tile_info( tile_data &tileinfo, int tile_index, int layer, uint16_t *vram);
	inline void get_tile_info_8bit( tile_data &tileinfo, int tile_index, int layer, uint16_t *vram );
	inline void get_tile_info_16x16_8bit( tile_data &tileinfo, int tile_index, int layer, uint16_t *vram );
	inline void vram_w( offs_t offset, uint16_t data, uint16_t mem_mask, int layer, uint16_t *vram );
	void alloc_empty_tiles(  );
	void expand_gfx1(  );
	void draw_sprites( screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect );
	void draw_layers( screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect, int pri, int layers_ctrl );
	void dirty_tiles( int layer, uint16_t *vram );
	void update_irq_state(  );
	inline int blt_read( const uint8_t *ROM, const int offs );
	void blt_write( address_space &space, const int tmap, const offs_t offs, const uint16_t data, const uint16_t mask );
};
