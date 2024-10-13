// license:BSD-3-Clause
// copyright-holders:Eugene Sandulenko
#ifndef MAME_USSR_TIAMC1_H
#define MAME_USSR_TIAMC1_H

#pragma once

#include "machine/pit8253.h"
#include "sound/spkrdev.h"
#include "emupal.h"
#include "tilemap.h"

class tiamc1_state : public driver_device
{
public:
	tiamc1_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
		, m_gfxdecode(*this, "gfxdecode")
		, m_palette(*this, "palette")
		, m_speaker(*this, "speaker")
	{ }

	void kot(machine_config &config);
	void tiamc1(machine_config &config);

protected:
	virtual void machine_reset() override ATTR_COLD;
	virtual void video_start() override ATTR_COLD;

private:
	std::unique_ptr<uint8_t[]> m_videoram;
	uint8_t *m_tileram = nullptr;
	uint8_t *m_charram = nullptr;
	uint8_t *m_spriteram_x = nullptr;
	uint8_t *m_spriteram_y = nullptr;
	uint8_t *m_spriteram_a = nullptr;
	uint8_t *m_spriteram_n = nullptr;
	uint8_t *m_paletteram = nullptr;
	uint8_t m_layers_ctrl =0;
	uint8_t m_bg_vshift = 0;
	uint8_t m_bg_hshift = 0;
	uint8_t m_bg_bplctrl = 0;
	tilemap_t *m_bg_tilemap1 = nullptr;
	tilemap_t *m_bg_tilemap2 = nullptr;
	std::unique_ptr<rgb_t[]> m_palette_ptr;
	void tiamc1_control_w(uint8_t data);
	void tiamc1_videoram_w(offs_t offset, uint8_t data);
	void tiamc1_bankswitch_w(uint8_t data);
	void tiamc1_sprite_x_w(offs_t offset, uint8_t data);
	void tiamc1_sprite_y_w(offs_t offset, uint8_t data);
	void tiamc1_sprite_a_w(offs_t offset, uint8_t data);
	void tiamc1_sprite_n_w(offs_t offset, uint8_t data);
	void tiamc1_bg_vshift_w(uint8_t data);
	void tiamc1_bg_hshift_w(uint8_t data);
	void tiamc1_bg_bplctrl_w(uint8_t data);
	void tiamc1_palette_w(offs_t offset, uint8_t data);
	void kot_bankswitch_w(uint8_t data);
	void kot_videoram_w(offs_t offset, uint8_t data);
	void pit8253_2_w(int state);

	TILE_GET_INFO_MEMBER(get_bg1_tile_info);
	TILE_GET_INFO_MEMBER(get_bg2_tile_info);
	DECLARE_VIDEO_START(kot);
	void tiamc1_palette(palette_device &palette);
	uint32_t screen_update_tiamc1(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	uint32_t screen_update_kot(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	void draw_sprites(bitmap_ind16 &bitmap, const rectangle &cliprect);
	required_device<cpu_device> m_maincpu;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<palette_device> m_palette;

	void kotrybolov_io_map(address_map &map) ATTR_COLD;
	void kotrybolov_map(address_map &map) ATTR_COLD;
	void tiamc1_io_map(address_map &map) ATTR_COLD;
	void tiamc1_map(address_map &map) ATTR_COLD;

	optional_device<speaker_sound_device> m_speaker;
	void update_bg_palette();
};

#endif // MAME_USSR_TIAMC1_H
