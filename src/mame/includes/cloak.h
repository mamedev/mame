// license:BSD-3-Clause
// copyright-holders:Dan Boris, Mirko Buffoni
/*************************************************************************

    Atari Cloak & Dagger hardware

*************************************************************************/
#ifndef MAME_INCLUDES_CLOAK_H
#define MAME_INCLUDES_CLOAK_H

#pragma once

#include "emupal.h"
#include "screen.h"
#include "tilemap.h"

class cloak_state : public driver_device
{
public:
	cloak_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		m_videoram(*this, "videoram"),
		m_spriteram(*this, "spriteram"),
		m_maincpu(*this, "maincpu"),
		m_slave(*this, "slave"),
		m_gfxdecode(*this, "gfxdecode"),
		m_screen(*this, "screen"),
		m_palette(*this, "palette")
	{ }

	void cloak(machine_config &config);

protected:
	DECLARE_WRITE_LINE_MEMBER(coin_counter_l_w);
	DECLARE_WRITE_LINE_MEMBER(coin_counter_r_w);
	DECLARE_WRITE8_MEMBER(cloak_custom_w);
	DECLARE_WRITE8_MEMBER(cloak_irq_reset_0_w);
	DECLARE_WRITE8_MEMBER(cloak_irq_reset_1_w);
	DECLARE_WRITE8_MEMBER(cloak_nvram_enable_w);
	DECLARE_WRITE8_MEMBER(cloak_paletteram_w);
	DECLARE_WRITE8_MEMBER(cloak_clearbmp_w);
	DECLARE_READ8_MEMBER(graph_processor_r);
	DECLARE_WRITE8_MEMBER(graph_processor_w);
	DECLARE_WRITE8_MEMBER(cloak_videoram_w);
	DECLARE_WRITE_LINE_MEMBER(cocktail_w);
	void set_current_bitmap_videoram_pointer();
	void adjust_xy(int offset);
	TILE_GET_INFO_MEMBER(get_bg_tile_info);
	virtual void video_start() override;
	uint32_t screen_update_cloak(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	void set_pen(int i);
	void draw_bitmap(bitmap_ind16 &bitmap, const rectangle &cliprect);
	void draw_sprites(bitmap_ind16 &bitmap, const rectangle &cliprect);
	void master_map(address_map &map);
	void slave_map(address_map &map);

private:
	required_shared_ptr<uint8_t> m_videoram;
	required_shared_ptr<uint8_t> m_spriteram;
	int m_nvram_enabled;
	uint8_t m_bitmap_videoram_selected;
	uint8_t m_bitmap_videoram_address_x;
	uint8_t m_bitmap_videoram_address_y;
	std::unique_ptr<uint8_t[]> m_bitmap_videoram1;
	std::unique_ptr<uint8_t[]> m_bitmap_videoram2;
	uint8_t *m_current_bitmap_videoram_accessed;
	uint8_t *m_current_bitmap_videoram_displayed;
	std::unique_ptr<uint16_t[]>  m_palette_ram;
	tilemap_t *m_bg_tilemap;

	required_device<cpu_device> m_maincpu;
	required_device<cpu_device> m_slave;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<screen_device> m_screen;
	required_device<palette_device> m_palette;
};

#endif // MAME_INCLUDES_CLOAK_H
