// license:BSD-3-Clause
// copyright-holders:Eugene Sandulenko
#ifndef MAME_INCLUDES_TIAMC1_H
#define MAME_INCLUDES_TIAMC1_H

#pragma once

#include "machine/pit8253.h"
#include "sound/spkrdev.h"
#include "emupal.h"

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
	virtual void machine_reset() override;
	virtual void video_start() override;

private:
	std::unique_ptr<uint8_t[]> m_videoram;
	uint8_t *m_tileram;
	uint8_t *m_charram;
	uint8_t *m_spriteram_x;
	uint8_t *m_spriteram_y;
	uint8_t *m_spriteram_a;
	uint8_t *m_spriteram_n;
	uint8_t *m_paletteram;
	uint8_t m_layers_ctrl;
	uint8_t m_bg_vshift;
	uint8_t m_bg_hshift;
	uint8_t m_bg_bplctrl;
	tilemap_t *m_bg_tilemap1;
	tilemap_t *m_bg_tilemap2;
	std::unique_ptr<rgb_t[]> m_palette_ptr;
	DECLARE_WRITE8_MEMBER(tiamc1_control_w);
	DECLARE_WRITE8_MEMBER(tiamc1_videoram_w);
	DECLARE_WRITE8_MEMBER(tiamc1_bankswitch_w);
	DECLARE_WRITE8_MEMBER(tiamc1_sprite_x_w);
	DECLARE_WRITE8_MEMBER(tiamc1_sprite_y_w);
	DECLARE_WRITE8_MEMBER(tiamc1_sprite_a_w);
	DECLARE_WRITE8_MEMBER(tiamc1_sprite_n_w);
	DECLARE_WRITE8_MEMBER(tiamc1_bg_vshift_w);
	DECLARE_WRITE8_MEMBER(tiamc1_bg_hshift_w);
	DECLARE_WRITE8_MEMBER(tiamc1_bg_bplctrl_w);
	DECLARE_WRITE8_MEMBER(tiamc1_palette_w);
	DECLARE_WRITE8_MEMBER(kot_bankswitch_w);
	DECLARE_WRITE8_MEMBER(kot_videoram_w);
	DECLARE_WRITE_LINE_MEMBER(pit8253_2_w);

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

	void kotrybolov_io_map(address_map &map);
	void kotrybolov_map(address_map &map);
	void tiamc1_io_map(address_map &map);
	void tiamc1_map(address_map &map);

	optional_device<speaker_sound_device> m_speaker;
	void update_bg_palette();
};

#endif // MAME_INCLUDES_TIAMC1_H
