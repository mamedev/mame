// license:BSD-3-Clause
// copyright-holders:Aaron Giles
#ifndef MAME_INCLUDES_TP84
#define MAME_INCLUDES_TP84

#pragma once

#include "sound/flt_rc.h"
#include "emupal.h"
#include "screen.h"
#include "tilemap.h"

class tp84_state : public driver_device
{
public:
	tp84_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_maincpu(*this, "cpu1"),
		m_subcpu(*this, "sub"),
		m_audiocpu(*this, "audiocpu"),
		m_palette_bank(*this, "palette_bank"),
		m_scroll_x(*this, "scroll_x"),
		m_scroll_y(*this, "scroll_y"),
		m_bg_videoram(*this, "bg_videoram"),
		m_fg_videoram(*this, "fg_videoram"),
		m_bg_colorram(*this, "bg_colorram"),
		m_fg_colorram(*this, "fg_colorram"),
		m_spriteram(*this, "spriteram"),
		m_gfxdecode(*this, "gfxdecode"),
		m_screen(*this, "screen"),
		m_palette(*this, "palette"),
		m_filter(*this, "filter%u", 1U)
	{ }

	void tp84(machine_config &config);
	void tp84b(machine_config &config);

private:
	required_device<cpu_device> m_maincpu;
	required_device<cpu_device> m_subcpu;
	required_device<cpu_device> m_audiocpu;
	required_shared_ptr<uint8_t> m_palette_bank;
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
	required_device_array<filter_rc_device, 3> m_filter;
	tilemap_t *m_bg_tilemap = nullptr;
	tilemap_t *m_fg_tilemap = nullptr;
	bool m_flipscreen_x = false;
	bool m_flipscreen_y = false;

	bool m_irq_enable = false;
	bool m_sub_irq_mask = false;

	DECLARE_WRITE_LINE_MEMBER(irq_enable_w);
	DECLARE_WRITE_LINE_MEMBER(coin_counter_1_w);
	DECLARE_WRITE_LINE_MEMBER(coin_counter_2_w);
	DECLARE_WRITE_LINE_MEMBER(flip_screen_x_w);
	DECLARE_WRITE_LINE_MEMBER(flip_screen_y_w);
	uint8_t tp84_sh_timer_r();
	void tp84_filter_w(offs_t offset, uint8_t data);
	void tp84_sh_irqtrigger_w(uint8_t data);
	void sub_irq_mask_w(uint8_t data);
	void tp84_spriteram_w(offs_t offset, uint8_t data);
	uint8_t tp84_scanline_r();
	TILE_GET_INFO_MEMBER(get_bg_tile_info);
	TILE_GET_INFO_MEMBER(get_fg_tile_info);
	virtual void machine_start() override;
	virtual void video_start() override;
	void tp84_palette(palette_device &palette) const;
	uint32_t screen_update_tp84(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	DECLARE_WRITE_LINE_MEMBER(vblank_irq);
	void draw_sprites(bitmap_ind16 &bitmap, const rectangle &cliprect);

	void audio_map(address_map &map);
	void cpu2_map(address_map &map);
	void tp84_cpu1_map(address_map &map);
	void tp84b_cpu1_map(address_map &map);
};

#endif // MAME_INCLUDES_TP84
