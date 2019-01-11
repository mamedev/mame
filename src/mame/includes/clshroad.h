// license:BSD-3-Clause
// copyright-holders:Luca Elia
#ifndef MAME_INCLUDES_CLSHROAD_H
#define MAME_INCLUDES_CLSHROAD_H

#pragma once

#include "emupal.h"

class clshroad_state : public driver_device
{
public:
	clshroad_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_audiocpu(*this, "audiocpu"),
		m_gfxdecode(*this, "gfxdecode"),
		m_palette(*this, "palette"),
		m_spriteram(*this, "spriteram"),
		m_vram_0(*this, "vram_0"),
		m_vram_1(*this, "vram_1"),
		m_vregs(*this, "vregs")
	{ }

	required_device<cpu_device> m_maincpu;
	required_device<cpu_device> m_audiocpu;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<palette_device> m_palette;

	required_shared_ptr<uint8_t> m_spriteram;
	required_shared_ptr<uint8_t> m_vram_0;
	required_shared_ptr<uint8_t> m_vram_1;
	required_shared_ptr<uint8_t> m_vregs;

	uint8_t m_main_irq_mask;
	uint8_t m_sound_irq_mask;

	tilemap_t *m_tilemap_0a;
	tilemap_t *m_tilemap_0b;
	tilemap_t *m_tilemap_1;

	DECLARE_READ8_MEMBER(input_r);
	DECLARE_WRITE_LINE_MEMBER(main_irq_mask_w);
	DECLARE_WRITE_LINE_MEMBER(sound_irq_mask_w);
	DECLARE_WRITE_LINE_MEMBER(flipscreen_w);
	DECLARE_WRITE8_MEMBER(vram_0_w);
	DECLARE_WRITE8_MEMBER(vram_1_w);

	TILE_GET_INFO_MEMBER(get_tile_info_0a);
	TILE_GET_INFO_MEMBER(get_tile_info_0b);
	TILEMAP_MAPPER_MEMBER(tilemap_scan_rows_extra);
	TILE_GET_INFO_MEMBER(get_tile_info_fb1);
	TILE_GET_INFO_MEMBER(get_tile_info_1);

	void init_firebatl();
	virtual void machine_reset() override;
	DECLARE_VIDEO_START(firebatl);
	void firebatl_palette(palette_device &palette) const;
	DECLARE_VIDEO_START(clshroad);
	void clshroad_palette(palette_device &palette) const;
	uint32_t screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	void draw_sprites(bitmap_ind16 &bitmap, const rectangle &cliprect);

	INTERRUPT_GEN_MEMBER(vblank_irq);
	INTERRUPT_GEN_MEMBER(sound_timer_irq);
	void firebatl(machine_config &config);
	void clshroad(machine_config &config);
	void clshroad_map(address_map &map);
	void clshroad_sound_map(address_map &map);
};

#endif // MAME_INCLUDES_CLSHROAD_H
