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
	rallyx_state(const machine_config &mconfig, device_type type, std::string tag)
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
	required_shared_ptr<UINT8> m_videoram;
	required_shared_ptr<UINT8> m_radarattr;
	UINT8 *  m_spriteram;
	UINT8 *  m_spriteram2;
	UINT8 *  m_radarx;
	UINT8 *  m_radary;

	/* video-related */
	tilemap_t  *m_bg_tilemap;
	tilemap_t  *m_fg_tilemap;

	/* misc */
	int      m_last_bang;
	int      m_spriteram_base;
	int      m_stars_enable;
	int      m_total_stars;
	UINT8    m_drawmode_table[4];
	struct jungler_star m_stars[JUNGLER_MAX_STARS];

	/* devices */
	required_device<cpu_device> m_maincpu;
	optional_device<namco_device> m_namco_sound;
	optional_device<samples_device> m_samples;
	optional_device<timeplt_audio_device> m_timeplt_audio;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<palette_device> m_palette;

	UINT8    m_main_irq_mask;
	DECLARE_WRITE8_MEMBER(rallyx_interrupt_vector_w);
	DECLARE_WRITE8_MEMBER(rallyx_bang_w);
	DECLARE_WRITE8_MEMBER(rallyx_latch_w);
	DECLARE_WRITE8_MEMBER(locomotn_latch_w);
	DECLARE_WRITE8_MEMBER(rallyx_videoram_w);
	DECLARE_WRITE8_MEMBER(rallyx_scrollx_w);
	DECLARE_WRITE8_MEMBER(rallyx_scrolly_w);
	DECLARE_WRITE8_MEMBER(tactcian_starson_w);
	TILEMAP_MAPPER_MEMBER(fg_tilemap_scan);
	TILE_GET_INFO_MEMBER(rallyx_bg_get_tile_info);
	TILE_GET_INFO_MEMBER(rallyx_fg_get_tile_info);
	TILE_GET_INFO_MEMBER(locomotn_bg_get_tile_info);
	TILE_GET_INFO_MEMBER(locomotn_fg_get_tile_info);
	DECLARE_MACHINE_START(rallyx);
	DECLARE_MACHINE_RESET(rallyx);
	DECLARE_VIDEO_START(rallyx);
	DECLARE_PALETTE_INIT(rallyx);
	DECLARE_VIDEO_START(jungler);
	DECLARE_PALETTE_INIT(jungler);
	DECLARE_VIDEO_START(locomotn);
	DECLARE_VIDEO_START(commsega);
	UINT32 screen_update_rallyx(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	UINT32 screen_update_jungler(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	UINT32 screen_update_locomotn(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	INTERRUPT_GEN_MEMBER(rallyx_vblank_irq);
	INTERRUPT_GEN_MEMBER(jungler_vblank_irq);
	inline void rallyx_get_tile_info( tile_data &tileinfo, int tile_index, int ram_offs);
	inline void locomotn_get_tile_info(tile_data &tileinfo,int tile_index,int ram_offs);
	void calculate_star_field(  );
	void rallyx_video_start_common(  );
	void plot_star( bitmap_ind16 &bitmap, const rectangle &cliprect, int x, int y, int color );
	void draw_stars( bitmap_ind16 &bitmap, const rectangle &cliprect );
	void rallyx_draw_sprites( screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect );
	void locomotn_draw_sprites( screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect );
	void rallyx_draw_bullets( bitmap_ind16 &bitmap, const rectangle &cliprect, int transpen );
	void jungler_draw_bullets( bitmap_ind16 &bitmap, const rectangle &cliprect, int transpen );
	void locomotn_draw_bullets( bitmap_ind16 &bitmap, const rectangle &cliprect, int transpen );
};
