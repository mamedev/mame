// license:BSD-3-Clause
// copyright-holders:Manuel Abadia, Peter Ferrie
#ifndef MAME_INCLUDES_THOOP2_H
#define MAME_INCLUDES_THOOP2_H

#pragma once

#include "machine/74259.h"

class thoop2_state : public driver_device
{
public:
	thoop2_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_pant{ nullptr, nullptr },
		m_maincpu(*this, "maincpu"),
		m_outlatch(*this, "outlatch"),
		m_gfxdecode(*this, "gfxdecode"),
		m_palette(*this, "palette"),
		m_videoram(*this, "videoram"),
		m_vregs(*this, "vregs"),
		m_spriteram(*this, "spriteram"),
		m_shareram(*this, "shareram")
	{ }

	DECLARE_WRITE8_MEMBER(OKIM6295_bankswitch_w);
	DECLARE_WRITE_LINE_MEMBER(coin1_lockout_w);
	DECLARE_WRITE_LINE_MEMBER(coin2_lockout_w);
	DECLARE_WRITE_LINE_MEMBER(coin1_counter_w);
	DECLARE_WRITE_LINE_MEMBER(coin2_counter_w);

	DECLARE_WRITE16_MEMBER(vram_w);
	DECLARE_WRITE8_MEMBER(shareram_w);
	DECLARE_READ8_MEMBER(shareram_r);

	TILE_GET_INFO_MEMBER(get_tile_info_screen0);
	TILE_GET_INFO_MEMBER(get_tile_info_screen1);
	uint32_t screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);

	void thoop2(machine_config &config);
	void mcu_hostmem_map(address_map &map);
	void oki_map(address_map &map);
	void thoop2_map(address_map &map);
protected:
	virtual void machine_start() override;
	virtual void video_start() override;

	void sort_sprites();
	void draw_sprites(bitmap_ind16 &bitmap, const rectangle &cliprect, int pri);

private:
	int m_sprite_count[5];
	std::unique_ptr<int[]> m_sprite_table[5];
	tilemap_t *m_pant[2];

	required_device<cpu_device> m_maincpu;
	required_device<ls259_device> m_outlatch;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<palette_device> m_palette;

	required_shared_ptr<uint16_t> m_videoram;
	required_shared_ptr<uint16_t> m_vregs;
	required_shared_ptr<uint16_t> m_spriteram;
	optional_shared_ptr<uint16_t> m_shareram;
};

#endif // MAME_INCLUDES_THOOP2_H
