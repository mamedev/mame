// license:BSD-3-Clause
// copyright-holders:Pierpaolo Prazzoli, Bryan McPhail
#ifndef MAME_INCLUDES_TRYOUT_H
#define MAME_INCLUDES_TRYOUT_H

#pragma once

#include "emupal.h"
#include "tilemap.h"

class tryout_state : public driver_device
{
public:
	tryout_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_audiocpu(*this, "audiocpu"),
		m_gfxdecode(*this, "gfxdecode"),
		m_palette(*this, "palette"),
		m_videoram(*this, "videoram"),
		m_spriteram(*this, "spriteram%u", 1U),
		m_gfx_control(*this, "gfx_control"),
		m_vram(*this, "vram", 8 * 0x800, ENDIANNESS_LITTLE),
		m_vram_gfx(*this, "vram_gfx", 0x6000, ENDIANNESS_LITTLE),
		m_rombank(*this, "rombank")
	{ }

	void tryout(machine_config &config);

	DECLARE_INPUT_CHANGED_MEMBER(coin_inserted);

protected:
	virtual void machine_start() override;
	virtual void video_start() override;

private:
	required_device<cpu_device> m_maincpu;
	required_device<cpu_device> m_audiocpu;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<palette_device> m_palette;

	required_shared_ptr<uint8_t> m_videoram;
	required_shared_ptr_array<uint8_t, 2> m_spriteram;
	required_shared_ptr<uint8_t> m_gfx_control;
	memory_share_creator<uint8_t> m_vram;
	memory_share_creator<uint8_t> m_vram_gfx;

	required_memory_bank m_rombank;

	tilemap_t *m_fg_tilemap = nullptr;
	tilemap_t *m_bg_tilemap = nullptr;
	uint8_t m_vram_bank = 0;

	void nmi_ack_w(uint8_t data);
	void sound_irq_ack_w(uint8_t data);
	void bankswitch_w(uint8_t data);
	uint8_t vram_r(offs_t offset);
	void videoram_w(offs_t offset, uint8_t data);
	void vram_w(offs_t offset, uint8_t data);
	void vram_bankswitch_w(uint8_t data);
	void flipscreen_w(uint8_t data);

	TILE_GET_INFO_MEMBER(get_fg_tile_info);
	TILE_GET_INFO_MEMBER(get_bg_tile_info);
	TILEMAP_MAPPER_MEMBER(get_fg_memory_offset);
	TILEMAP_MAPPER_MEMBER(get_bg_memory_offset);

	void palette(palette_device &palette) const;

	uint32_t screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	void draw_sprites(bitmap_ind16 &bitmap,const rectangle &cliprect);

	void main_cpu(address_map &map);
	void sound_cpu(address_map &map);
};

#endif // MAME_INCLUDES_TRYOUT_H
