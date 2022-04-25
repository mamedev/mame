// license:BSD-3-Clause
// copyright-holders:Ernesto Corvi, Phil Stroffolino, Bryan McPhail
#ifndef MAME_INCLUDES_SHOOTOUT_H
#define MAME_INCLUDES_SHOOTOUT_H

#pragma once

#include "machine/gen_latch.h"
#include "emupal.h"
#include "tilemap.h"

class shootout_state : public driver_device
{
public:
	shootout_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_audiocpu(*this, "audiocpu"),
		m_gfxdecode(*this, "gfxdecode"),
		m_palette(*this, "palette"),
		m_soundlatch(*this, "soundlatch"),
		m_spriteram(*this, "spriteram"),
		m_textram(*this, "textram"),
		m_videoram(*this, "videoram")
	{ }

	void shootouj(machine_config &config);
	void shootouk(machine_config &config);
	void shootout(machine_config &config);

	void init_shootout();

	DECLARE_INPUT_CHANGED_MEMBER(coin_inserted);

private:
	required_device<cpu_device> m_maincpu;
	optional_device<cpu_device> m_audiocpu;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<palette_device> m_palette;
	required_device<generic_latch_8_device> m_soundlatch;

	required_shared_ptr<uint8_t> m_spriteram;
	required_shared_ptr<uint8_t> m_textram;
	required_shared_ptr<uint8_t> m_videoram;

	tilemap_t *m_background = nullptr;
	tilemap_t *m_foreground = nullptr;

	int m_ccnt_old_val = 0;

	void bankswitch_w(uint8_t data);
	uint8_t sound_cpu_command_r();
	void sound_cpu_command_w(uint8_t data);
	void flipscreen_w(uint8_t data);
	void coincounter_w(uint8_t data);
	void videoram_w(offs_t offset, uint8_t data);
	void textram_w(offs_t offset, uint8_t data);

	virtual void machine_reset() override;
	virtual void video_start() override;

	void shootout_palette(palette_device &palette) const;

	TILE_GET_INFO_MEMBER(get_bg_tile_info);
	TILE_GET_INFO_MEMBER(get_fg_tile_info);

	uint32_t screen_update_shootout(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	uint32_t screen_update_shootouj(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	void draw_sprites(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect, int bank_bits );

	void shootouj_map(address_map &map);
	void shootout_map(address_map &map);
	void shootout_sound_map(address_map &map);
};

#endif // MAME_INCLUDES_SHOOTOUT_H
