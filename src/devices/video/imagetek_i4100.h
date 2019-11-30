// license:BSD-3-Clause
// copyright-holders:Luca Elia,David Haywood,Angelo Salese
/***************************************************************************

    Imagetek I4100 / I4220 / I4300 device files

***************************************************************************/

#ifndef MAME_VIDEO_I4100_H
#define MAME_VIDEO_I4100_H

#pragma once

#include "emupal.h"
#include "screen.h"
#include "video/bufsprite.h"


//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> i4100_device

class imagetek_i4100_device : public device_t,
							  public device_gfx_interface,
							  public device_video_interface
{
public:
	// construction/destruction
	imagetek_i4100_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);

	void map(address_map &map);

	void set_tmap_xoffsets(int x1, int x2, int x3) { m_tilemap_scrolldx[0] = x1; m_tilemap_scrolldx[1] = x2; m_tilemap_scrolldx[2] = x3; }
	void set_tmap_yoffsets(int y1, int y2, int y3) { m_tilemap_scrolldy[0] = y1; m_tilemap_scrolldy[1] = y2; m_tilemap_scrolldy[2] = y3; }

	auto blit_irq_cb() { return m_blit_irq_cb.bind(); }
	void set_spriteram_buffered(bool buffer) { m_spriteram_buffered = buffer; }

	u32 screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);
	void draw_foreground(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);
	DECLARE_WRITE_LINE_MEMBER(screen_eof);

protected:
	imagetek_i4100_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, u32 clock, bool has_ext_tiles);

	// device-level overrides
	//virtual void device_validity_check(validity_checker &valid) const override;
	virtual void device_add_mconfig(machine_config &config) override;
	virtual void device_start() override;
	virtual void device_reset() override;
	virtual void device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr) override;

	required_shared_ptr_array<u16, 3> m_vram;
	required_shared_ptr<u16> m_scratchram;
	required_shared_ptr<u16> m_blitter_regs;
	required_device<buffered_spriteram16_device> m_spriteram;
	required_shared_ptr<u16> m_tiletable;
	required_shared_ptr<u16> m_window;
	required_shared_ptr<u16> m_scroll;

	required_device<palette_device> m_palette;
	required_region_ptr<u8> m_gfxrom;

	std::unique_ptr<u8[]>   m_expanded_gfx1;

	devcb_write_line m_blit_irq_cb;

	struct sprite_t
	{
		int x, y;
		u32 gfxstart;
		int width, height;
		int flipx, flipy;
		u16 color;
		u32 zoom;
		int curr_pri;
	};

	std::unique_ptr<sprite_t []> m_spritelist;
	const sprite_t *m_sprite_end;

	u16 m_rombank;
	size_t m_gfxrom_size;
	bool m_crtc_unlock;
	u16 m_sprite_count;
	u16 m_sprite_priority;
	u16 m_sprite_xoffset,m_sprite_yoffset;
	u16 m_sprite_color_code;
	u8 m_layer_priority[3];
	u16 m_background_color;
	u16 m_screen_xoffset,m_screen_yoffset;
	bool m_layer_tile_select[3];
	bool m_screen_blank;
	bool m_screen_flip;
	const bool m_support_8bpp, m_support_16x16;
	int  m_tilemap_scrolldx[3];
	int  m_tilemap_scrolldy[3];
	bool m_spriteram_buffered;

	void blt_write(const int tmap, const offs_t offs, const u16 data, const u16 mask);

	enum
	{
		TIMER_BLIT_END = 1
	};

	emu_timer *m_blit_done_timer;

	// I/O operations
	inline u16 vram_r(offs_t offset, int layer) { return m_vram[layer][offset]; }
	inline void vram_w(offs_t offset, u16 data, u16 mem_mask, int layer) { COMBINE_DATA(&m_vram[layer][offset]); }
	DECLARE_READ16_MEMBER(vram_0_r);
	DECLARE_READ16_MEMBER(vram_1_r);
	DECLARE_READ16_MEMBER(vram_2_r);
	DECLARE_WRITE16_MEMBER(vram_0_w);
	DECLARE_WRITE16_MEMBER(vram_1_w);
	DECLARE_WRITE16_MEMBER(vram_2_w);
	DECLARE_READ16_MEMBER(rmw_vram_0_r);
	DECLARE_READ16_MEMBER(rmw_vram_1_r);
	DECLARE_READ16_MEMBER(rmw_vram_2_r);
	DECLARE_WRITE16_MEMBER(rmw_vram_0_w);
	DECLARE_WRITE16_MEMBER(rmw_vram_1_w);
	DECLARE_WRITE16_MEMBER(rmw_vram_2_w);
	DECLARE_READ16_MEMBER(scratchram_r);
	DECLARE_WRITE16_MEMBER(scratchram_w);
	DECLARE_READ16_MEMBER(spriteram_r);
	DECLARE_WRITE16_MEMBER(spriteram_w);
	DECLARE_READ16_MEMBER(tiletable_r);
	DECLARE_WRITE16_MEMBER(tiletable_w);
	DECLARE_READ16_MEMBER(sprite_count_r);
	DECLARE_WRITE16_MEMBER(sprite_count_w);
	DECLARE_READ16_MEMBER(sprite_priority_r);
	DECLARE_WRITE16_MEMBER(sprite_priority_w);
	DECLARE_READ16_MEMBER(sprite_xoffset_r);
	DECLARE_WRITE16_MEMBER(sprite_xoffset_w);
	DECLARE_READ16_MEMBER(sprite_yoffset_r);
	DECLARE_WRITE16_MEMBER(sprite_yoffset_w);
	DECLARE_READ16_MEMBER(sprite_color_code_r);
	DECLARE_WRITE16_MEMBER(sprite_color_code_w);
	DECLARE_READ16_MEMBER(layer_priority_r);
	DECLARE_WRITE16_MEMBER(layer_priority_w);
	DECLARE_READ16_MEMBER(background_color_r);
	DECLARE_WRITE16_MEMBER(background_color_w);

	DECLARE_READ16_MEMBER(screen_xoffset_r);
	DECLARE_WRITE16_MEMBER(screen_xoffset_w);
	DECLARE_READ16_MEMBER(screen_yoffset_r);
	DECLARE_WRITE16_MEMBER(screen_yoffset_w);

	DECLARE_READ16_MEMBER(window_r);
	DECLARE_WRITE16_MEMBER(window_w);
	DECLARE_READ16_MEMBER(scroll_r);
	DECLARE_WRITE16_MEMBER(scroll_w);


	DECLARE_READ16_MEMBER(gfxrom_r);
	DECLARE_WRITE16_MEMBER(crtc_vert_w);
	DECLARE_WRITE16_MEMBER(crtc_horz_w);
	DECLARE_WRITE16_MEMBER(crtc_unlock_w);
	DECLARE_WRITE16_MEMBER(blitter_w);
	DECLARE_WRITE16_MEMBER(screen_ctrl_w);
	DECLARE_WRITE16_MEMBER(rombank_w);

	void draw_layers(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect, int pri);
	inline u8 get_tile_pix(u16 code, u8 x, u8 y, bool const big, u32 &pix);
	void draw_tilemap(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect, u32 flags, u32 const pcode,
					int sx, int sy, int wx, int wy, bool const big, int const layer);
	void draw_spritegfx(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &clip,
					u32 const gfxstart, u16 const width, u16 const height,
					u16 color, int const flipx, int const flipy, int sx, int sy,
					u32 const scale, u8 const prival);
	void draw_sprites(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);
	void expand_gfx1();

// A 2048 x 2048 virtual tilemap
	static constexpr u32 BIG_NX = (0x100);
	static constexpr u32 BIG_NY = (0x100);

// A smaller 512 x 256 window defines the actual tilemap
	static constexpr u32 WIN_NX = (0x40);
	static constexpr u32 WIN_NY = (0x20);

	bool m_inited_hack;
	DECLARE_GFXDECODE_MEMBER(gfxinfo);
	DECLARE_GFXDECODE_MEMBER(gfxinfo_ext);
};

class imagetek_i4220_device : public imagetek_i4100_device
{
public:
	// construction/destruction
	imagetek_i4220_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);

	// needed by Blazing Tornado / Grand Striker 2 for mixing with PSAC
	// (it's unknown how the chip enables external sync)
	u32 get_background_pen() { return m_palette->pen(m_background_color); };

	void v2_map(address_map &map);
};

class imagetek_i4300_device : public imagetek_i4100_device
{
public:
	// construction/destruction
	imagetek_i4300_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);

	void v3_map(address_map &map);
};

// device type definition
DECLARE_DEVICE_TYPE(I4100, imagetek_i4100_device)
DECLARE_DEVICE_TYPE(I4220, imagetek_i4220_device)
DECLARE_DEVICE_TYPE(I4300, imagetek_i4300_device)



//**************************************************************************
//  GLOBAL VARIABLES
//**************************************************************************


#endif // MAME_VIDEO_I4100_H
