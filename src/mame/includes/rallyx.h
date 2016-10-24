// license:BSD-3-Clause
// copyright-holders:Nicola Salmoria
#include "audio/timeplt.h"
#include "sound/namco.h"
#include "sound/samples.h"

struct jungler_star
{
	int x, y, color;
};

#define JUNGLER_MAX_STARS 1000

class rallyx_state : public driver_device
{
public:
	rallyx_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		m_videoram(*this, "videoram"),
		m_radarattr(*this, "radarattr"),
		m_maincpu(*this, "maincpu"),
		m_namco_sound(*this, "namco"),
		m_samples(*this, "samples"),
		m_timeplt_audio(*this, "timeplt_audio"),
		m_gfxdecode(*this, "gfxdecode"),
		m_palette(*this, "palette") { }

	/* memory pointers */
	required_shared_ptr<uint8_t> m_videoram;
	required_shared_ptr<uint8_t> m_radarattr;
	uint8_t *  m_spriteram;
	uint8_t *  m_spriteram2;
	uint8_t *  m_radarx;
	uint8_t *  m_radary;

	/* video-related */
	tilemap_t  *m_bg_tilemap;
	tilemap_t  *m_fg_tilemap;

	/* misc */
	int      m_last_bang;
	int      m_spriteram_base;
	int      m_stars_enable;
	int      m_total_stars;
	uint8_t    m_drawmode_table[4];
	struct jungler_star m_stars[JUNGLER_MAX_STARS];

	/* devices */
	required_device<cpu_device> m_maincpu;
	optional_device<namco_device> m_namco_sound;
	optional_device<samples_device> m_samples;
	optional_device<timeplt_audio_device> m_timeplt_audio;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<palette_device> m_palette;

	uint8_t    m_main_irq_mask;
	void rallyx_interrupt_vector_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void rallyx_bang_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void rallyx_latch_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void locomotn_latch_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void rallyx_videoram_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void rallyx_scrollx_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void rallyx_scrolly_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void tactcian_starson_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	tilemap_memory_index fg_tilemap_scan(uint32_t col, uint32_t row, uint32_t num_cols, uint32_t num_rows);
	void rallyx_bg_get_tile_info(tilemap_t &tilemap, tile_data &tileinfo, tilemap_memory_index tile_index);
	void rallyx_fg_get_tile_info(tilemap_t &tilemap, tile_data &tileinfo, tilemap_memory_index tile_index);
	void locomotn_bg_get_tile_info(tilemap_t &tilemap, tile_data &tileinfo, tilemap_memory_index tile_index);
	void locomotn_fg_get_tile_info(tilemap_t &tilemap, tile_data &tileinfo, tilemap_memory_index tile_index);
	void machine_start_rallyx();
	void machine_reset_rallyx();
	void video_start_rallyx();
	void palette_init_rallyx(palette_device &palette);
	void video_start_jungler();
	void palette_init_jungler(palette_device &palette);
	void video_start_locomotn();
	void video_start_commsega();
	uint32_t screen_update_rallyx(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	uint32_t screen_update_jungler(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	uint32_t screen_update_locomotn(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	void rallyx_vblank_irq(device_t &device);
	void jungler_vblank_irq(device_t &device);
	inline void rallyx_get_tile_info( tile_data &tileinfo, int tile_index, int ram_offs);
	inline void locomotn_get_tile_info(tile_data &tileinfo,int tile_index,int ram_offs);
	void calculate_star_field(  );
	void rallyx_video_start_common(  );
	void plot_star( bitmap_ind16 &bitmap, const rectangle &cliprect, int x, int y, int color );
	void draw_stars( bitmap_ind16 &bitmap, const rectangle &cliprect );
	void rallyx_draw_sprites( screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect );
	void locomotn_draw_sprites( screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect );
	void rallyx_draw_bullets( bitmap_ind16 &bitmap, const rectangle &cliprect, bool transpen );
	void jungler_draw_bullets( bitmap_ind16 &bitmap, const rectangle &cliprect, bool transpen );
	void locomotn_draw_bullets( bitmap_ind16 &bitmap, const rectangle &cliprect, bool transpen );
};
